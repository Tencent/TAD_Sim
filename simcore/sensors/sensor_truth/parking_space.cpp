/**
 * @file ParkingSpace.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "parking_space.h"
#include <google/protobuf/util/json_util.h>
#include <Eigen/Geometry>
#include <boost/property_tree/json_parser.hpp>
#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include "object_size.h"
#include "ultrasonic_sensor.h"
#include "user_topic.h"
#include "common/coord_trans.h"
#include "location.pb.h"
#include "mapengine/hadmap_codes.h"
#include "parking_space.pb.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_object.h"
#include "traffic.pb.h"
#include "union.pb.h"

#ifndef DBL_MAX
#  define DBL_MAX 1.7976931348623158e+308
#endif  // !

/**
 * @brief parkingspace init
 *
 * @param helper helper of txsim
 */
void ParkingSpace::Init(tx_sim::InitHelper &helper) {
  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Subscribe(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation);

  helper.Publish(std::string(tx_sim::user_topic::kParkingSpace) + (device.empty() ? "" : "_") + device);
}

/**
 * @brief parkingspace reset
 *
 * @param helper helper of txsim
 */
void ParkingSpace::Reset(tx_sim::ResetHelper &helper) {
  obs_swap.clear();
  parking_swap.clear();
  roadPtr.reset();
  InitMap();
}

/**
 * @brief parkingspace step
 *
 * @param helper helper of txsim
 */
void ParkingSpace::Step(tx_sim::StepHelper &helper) {
  // std::chrono::steady_clock::time_point t1, t2;
  // t1 = std::chrono::steady_clock::now();
  //  load location message and parse it into a protobuf object
  std::string payload_;
  helper.GetSubscribedMessage(tx_sim::topic::kLocation, payload_);
  sim_msg::Location loc;
  loc.ParseFromString(payload_);

  if (fovfilters.empty()) {
    return;
  }
  auto &fov = *fovfilters.begin()->second;

  sim_msg::ParkingSpace tParkingspace;
  tParkingspace.set_t(helper.timestamp());

  // ego

  // find out road of the ego vehicle in current frame
  {
    hadmap::txLanePtr lanePtr;
    if (GetEgoLane(hadmap::txPoint(loc.position().x(), loc.position().y(), loc.position().z()), lanePtr)) {
      egoLaneid = lanePtr->getTxLaneId();
    }

    if (!roadPtr.get() || roadPtr->getId() != egoLaneid.roadId)
      hadmap::getRoad(hMapHandle, egoLaneid.roadId, true, roadPtr);
  }
  Eigen::Vector3d locPosition(loc.position().x(), loc.position().y(), loc.position().z());
  // set ego world coordinate
  cal_wd(locPosition, tParkingspace.mutable_world()->mutable_ego()->mutable_pos());
  // set ego st coordinate
  cal_st(locPosition, tParkingspace.mutable_st()->mutable_ego()->mutable_pos());
  // set ego heading
  tParkingspace.mutable_world()->mutable_ego()->set_heading(loc.rpy().z());
  tParkingspace.mutable_st()->mutable_ego()->set_heading(loc.rpy().z());

  // t2 = std::chrono::steady_clock::now();
  // std::cout
  //     << "preapre took "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t2 -
  //     t1).count()
  //     << " ms.";
  // t1 = std::chrono::steady_clock::now();
  //  parking
  //  find out all parkingspace nearby to the ego car
  coord_trans_api::lonlat2enu(locPosition.x(), locPosition.y(), locPosition.z(), map_ori.x, map_ori.y, map_ori.z);
  std::vector<size_t> indexs;
  std::vector<double> diss;
  double p[2] = {locPosition.x(), locPosition.y()};
  if (bpFlann.SearchRadius(p, indexs, diss, fov.getRange() * fov.getRange())) {
    for (auto i : indexs) {
      const auto &parking = parkings[i];
      // if the parking is not in the fov, skip it
      if (!fov.inFov(parking.pos)) {
        continue;
      }
      // if the parkingspace is not calculated before
      if (parking_swap.find(parking.id) == parking_swap.end()) {
        ParkingSwap tparking;
        tparking.world.set_type((sim_msg::ParkingType)parking.type);

        // set parkingspace size
        tparking.world.set_id(parking.id);
        tparking.world.set_length(parking.length);
        tparking.world.set_depth(parking.depth);
        tparking.world.set_yaw(parking.yaw);
        // set parkingspace center
        cal_wd(parking.pos_84, tparking.world.mutable_center());
        // set parkingspace left and right, bottom and top
        cal_wd(parking.p_84[0], tparking.world.mutable_lb());
        cal_wd(parking.p_84[1], tparking.world.mutable_lt());
        cal_wd(parking.p_84[2], tparking.world.mutable_rt());
        cal_wd(parking.p_84[3], tparking.world.mutable_rb());

        // we do same thing in st coordinate
        tparking.roadid = parking.roadid;
        tparking.st.set_type(tparking.world.type());
        tparking.st.set_id(tparking.world.id());
        tparking.st.set_length(tparking.world.length());
        tparking.st.set_depth(tparking.world.depth());
        tparking.st.set_yaw(tparking.world.yaw());

        // set parkingspace center
        cal_st(parking.pos_84, tparking.st.mutable_center());
        // set parkingspace left and right, bottom and top
        cal_st(parking.p_84[0], tparking.st.mutable_lb());
        cal_st(parking.p_84[1], tparking.st.mutable_lt());
        cal_st(parking.p_84[2], tparking.st.mutable_rt());
        cal_st(parking.p_84[3], tparking.st.mutable_rb());

        // save to memory to use it directly next time
        parking_swap.insert(std::make_pair(parking.id, std::move(tparking)));
      }
      // using from memory
      if (parking_swap.find(parking.id) != parking_swap.end()) {
        const auto &oswap = parking_swap.at(parking.id);
        *(tParkingspace.mutable_world()->add_parking()) = oswap.world;
        if (roadPtr && egoLaneid.roadId == oswap.roadid) {
          *(tParkingspace.mutable_st()->add_parking()) = oswap.st;
        }
      }
    }
  }

  // t2 = std::chrono::steady_clock::now();
  // std::cout
  //     << "parking took "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t2 -
  //     t1).count()
  //     << " ms.";
  // t1 = std::chrono::steady_clock::now();
  //  fov: obstacle,traffic
  //  set fov
  fov.setCarPosition(locPosition);
  fov.setCarRoatation(loc.rpy().x(), loc.rpy().y(), loc.rpy().z());

  // egos
  helper.GetSubscribedMessage(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation, payload_);
  sim_msg::Union union_location;
  union_location.ParseFromString(payload_);
  handle_ego(union_location, tParkingspace, fov);

  // get traffic message
  helper.GetSubscribedMessage(tx_sim::topic::kTraffic, payload_);
  sim_msg::Traffic traffic;
  traffic.ParseFromString(payload_);
  handle_traffic(traffic, tParkingspace, fov);

  //  UltrasonicSensor
  for (auto &ult : ultrasonics) {
    // set sensor location and rotation
    ult.second->setCarPosition(locPosition);
    ult.second->setCarRoatation(loc.rpy().x(), loc.rpy().y(), loc.rpy().z());
  }
  handle_uss(traffic, union_location, tParkingspace);

  // t2 = std::chrono::steady_clock::now();
  // std::cout
  //     << "ultrasonics took "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t2 -
  //     t1).count()
  //     << " ms.";
  // t1 = std::chrono::steady_clock::now();
  std::cout << "[ParkingSpace(" << helper.timestamp() << ")]: " << tParkingspace.world().parking_size()
            << " parkingspace, " << tParkingspace.world().traffic_size() << " cars, "
            << tParkingspace.world().obstacle_size() << " obstacles, " << tParkingspace.ultrasonic_size() << " uss\n";
  // publish message
  std::string buf;
  tParkingspace.SerializeToString(&buf);
  helper.PublishMessage(std::string(tx_sim::user_topic::kParkingSpace) + (device.empty() ? "" : "_") + device, buf);

  // debug print
  if (!debugDir.empty()) {
    std::ofstream debug(debugDir + "/parking_space_" + std::to_string(helper.timestamp()) + ".json");
    std::string json;
    google::protobuf::util::MessageToJsonString(tParkingspace, &json);
    debug << json;
  }

  // t2 = std::chrono::steady_clock::now();
  // std::cout
  //     << "publish took "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t2 -
  //     t1).count()
  //     << " ms.";
}

/**
 * @brief parkingspace stop
 *
 * @param helper helper of txsim
 */
void ParkingSpace::Stop(tx_sim::StopHelper &helper) {}

/**
 * @brief calculate world coordinate
 *
 * @param pt the point
 * @param pp the parking point
 */
void ParkingSpace::cal_wd(const Eigen::Vector3d &pt, sim_msg::ParkingPoint *pp) {
  pp->set_x(pt.x());
  pp->set_y(pt.y());
  pp->set_z(pt.z());
}

/**
 * @brief calculate st coordinate
 *
 * @param pt the point
 * @param pp the parking point
 */
bool ParkingSpace::cal_st(const Eigen::Vector3d &pt, sim_msg::ParkingPoint *pp) {
  if (roadPtr.get() && roadPtr->getGeometry()) {
    double s = 0, l = 0, yaw = 0;
    if (roadPtr->getGeometry()->xy2sl(pt.x(), pt.y(), s, l, yaw)) {
      pp->set_x(s);
      pp->set_y(l);
      return true;
    }
  }
  return false;
}

/// @brief
/// @param union_location
/// @param tParkingspace
/// @param fov
void ParkingSpace::handle_ego(const sim_msg::Union &union_location, sim_msg::ParkingSpace &tParkingspace,
                              const FovFilterSensor &fov) {
  for (const auto &msg : union_location.messages()) {
    std::int64_t eid = std::atoi(msg.groupname().substr(msg.groupname().length() - 3).c_str());
    if (eid == ego_id) continue;
    sim_msg::Location uloc;
    if (uloc.ParseFromString(msg.content())) {
      Eigen::Vector3d pos_84(uloc.position().x(), uloc.position().y(), uloc.position().z());
      Eigen::Vector3d pos = pos_84;
      coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
      // if the car is not in the fov, skip it
      if (!fov.inFov(pos)) {
        continue;
      }
      // calculate the car info
      sim_msg::TrafficVehicle &tfc = *(tParkingspace.mutable_world()->add_traffic());
      tfc.set_id(-eid);
      // calculate position
      cal_wd(pos_84, tfc.mutable_pos());
      // calculate vehicle position
      cal_wd(fov.FovVector(pos), tfc.mutable_vehicle_pos());
      tfc.set_heading(uloc.rpy().z());
      double vv = uloc.velocity().x() * uloc.velocity().x() + uloc.velocity().y() * uloc.velocity().y() +
                  uloc.velocity().z() * uloc.velocity().z();
      tfc.set_v(vv > 0.001 ? std::sqrt(vv) : 0);
      double cc = uloc.velocity().x() * uloc.velocity().x() + uloc.velocity().y() * uloc.velocity().y() +
                  uloc.velocity().z() * uloc.velocity().z();
      tfc.set_acc(cc > 0.001 ? std::sqrt(cc) : 0);
      // if this char is on the road that same as ego
      // calculate the car info
      if (uloc.ego_lane().roadpkid() == egoLaneid.roadId && roadPtr) {
        sim_msg::TrafficVehicle &stfc = *(tParkingspace.mutable_st()->add_traffic());
        stfc.set_id(tfc.id());
        cal_st(pos_84, stfc.mutable_pos());
        stfc.set_heading(tfc.heading());
        stfc.set_v(tfc.v());
        stfc.set_acc(tfc.acc());
      }
    }
  }
}
/// @brief
/// @param traffic
/// @param tParkingspace
/// @param fov
void ParkingSpace::handle_traffic(const sim_msg::Traffic &traffic, sim_msg::ParkingSpace &tParkingspace,
                                  const FovFilterSensor &fov) {
  // handle car
  for (const auto &car : traffic.cars()) {
    Eigen::Vector3d pos_84(car.x(), car.y(), car.z());
    Eigen::Vector3d pos = pos_84;
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    // if the car is not in the fov, skip it
    if (!fov.inFov(pos)) {
      continue;
    }
    // calculate the car info
    sim_msg::TrafficVehicle &tfc = *(tParkingspace.mutable_world()->add_traffic());
    tfc.set_id(car.id());
    // calculate position
    cal_wd(pos_84, tfc.mutable_pos());
    // calculate vehicle position
    cal_wd(fov.FovVector(pos), tfc.mutable_vehicle_pos());
    tfc.set_heading(car.heading());
    tfc.set_v(car.v());
    tfc.set_acc(car.acc());
    // if this char is on the road that same as ego
    // calculate the car info
    if (car.tx_road_id() == egoLaneid.roadId && roadPtr) {
      sim_msg::TrafficVehicle &stfc = *(tParkingspace.mutable_st()->add_traffic());
      stfc.set_id(car.id());
      cal_st(pos_84, stfc.mutable_pos());
      stfc.set_heading(car.heading());
      stfc.set_v(car.v());
      stfc.set_acc(car.acc());
    }
  }
  // t2 = std::chrono::steady_clock::now();
  // std::cout
  //     << "cars took "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t2 -
  //     t1).count()
  //     << " ms.";
  // t1 = std::chrono::steady_clock::now();
  //  handle dynamic obstacles
  for (const auto &obs : traffic.dynamicobstacles()) {
    Eigen::Vector3d pos_84(obs.x(), obs.y(), obs.z());
    Eigen::Vector3d pos = pos_84;
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);

    // if the pos in not in fov, skip it
    if (!fov.inFov(pos)) {
      continue;
    }
    // calculate the dynamic info
    sim_msg::Obstacle &osc = *(tParkingspace.mutable_world()->add_obstacle());
    osc.set_id(obs.id());
    osc.set_heading(obs.heading());
    // calculate position
    cal_wd(pos_84, osc.mutable_pos());

    // get this road
    hadmap::txLanePtr lane;
    if (roadPtr && hadmap::getLane(hMapHandle, hadmap::txPoint(obs.x(), obs.y(), obs.z()), lane) == TX_HADMAP_DATA_OK) {
      // if this char is on the road that same as ego
      // calculate the dynamic object info
      if (lane->getRoadId() == egoLaneid.roadId) {
        sim_msg::Obstacle &sosc = *(tParkingspace.mutable_st()->add_obstacle());
        sosc.set_id(osc.id());
        osc.set_heading(obs.heading());
        cal_st(pos_84, sosc.mutable_pos());
      }
    }
  }
  // t2 = std::chrono::steady_clock::now();
  // std::cout
  //     << "dynamicobstacles took "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t2 -
  //     t1).count()
  //     << " ms.";
  // t1 = std::chrono::steady_clock::now();
  //  handle static obstacles
  for (const auto &obs : traffic.staticobstacles()) {
    Eigen::Vector3d pos_84(obs.x(), obs.y(), obs.z());
    Eigen::Vector3d pos = pos_84;
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);

    // if the pos in not in fov, skip it
    if (!fov.inFov(pos)) {
      continue;
    }
    int id = obs.id() + 10000;
    // if the info is not calculated before.
    //
    if (obs_swap.find(id) == obs_swap.end()) {
      ObsSwap oswap;
      // calculate the static object info
      oswap.world.set_id(id);
      oswap.world.set_heading(obs.heading());
      // calculate position
      cal_wd(pos_84, oswap.world.mutable_pos());

      hadmap::txLanePtr lane;
      if (hadmap::getLane(hMapHandle, hadmap::txPoint(obs.x(), obs.y(), obs.z()), lane) == TX_HADMAP_DATA_OK) {
        oswap.roadid = lane->getRoadId();

        oswap.st.set_id(id);
        oswap.st.set_heading(obs.heading());
        cal_st(pos_84, oswap.st.mutable_pos());
      }
      // save to memory to use it directly next time
      obs_swap.insert(std::make_pair(id, std::move(oswap)));
    }
    // using from memory
    if (obs_swap.find(id) != obs_swap.end()) {
      const auto &oswap = obs_swap.at(id);
      *(tParkingspace.mutable_world()->add_obstacle()) = oswap.world;
      if (roadPtr && egoLaneid.roadId == oswap.roadid) {
        *(tParkingspace.mutable_st()->add_obstacle()) = oswap.st;
      }
    }
  }
  // t2 = std::chrono::steady_clock::now();
  // std::cout
  //     << "staticobstacles took "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t2 -
  //     t1).count()
  //     << " ms.";
  // t1 = std::chrono::steady_clock::now();
}

/// @brief
/// @param traffic
/// @param union_location
/// @param tParkingspace
void ParkingSpace::handle_uss(const sim_msg::Traffic &traffic, const sim_msg::Union &union_location,
                              sim_msg::ParkingSpace &tParkingspace) {
  //  UltrasonicSensor
  for (auto &ult : ultrasonics) {
    int type = 0;
    double mindis = ult.second->getRange() + 100;
    // handle other ego
    for (const auto &msg : union_location.messages()) {
      std::int64_t eid = std::atoi(msg.groupname().substr(msg.groupname().length() - 3).c_str());
      if (eid == ego_id) continue;
      sim_msg::Location uloc;
      if (uloc.ParseFromString(msg.content())) {
        auto &objbb = ego_Bboxes[eid];
        if (objbb.category < 0) {
          std::cout << "Cannot find ego type: " << eid << std::endl;
          continue;
        }
        Eigen::Vector3d pos(uloc.position().x(), uloc.position().y(), uloc.position().z());
        coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
        double d = ult.second->distance(pos, 0, 0, uloc.rpy().z(), objbb.len, objbb.wid, objbb.hei);
        // get the closest car
        if (d > 0.01 && d < mindis) {
          mindis = d;
          type = -1;
        }
      }
    }
    // handle car
    // get the minmum distance
    for (const auto &car : traffic.cars()) {
      Eigen::Vector3d pos(car.x(), car.y(), car.z());
      coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
      double d = ult.second->distance(pos, 0, 0, car.heading(), car.length(), car.width(), car.height());
      if (d > 0.01 && d < mindis) {
        mindis = d;
        type = 0;
      }
    }
    // handle dynamic obstacles
    // get the minmum distance
    for (const auto &obs : traffic.dynamicobstacles()) {
      Eigen::Vector3d pos(obs.x(), obs.y(), obs.z());
      coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
      double d = ult.second->distance(pos, 0, 0, obs.heading(), obs.length(), obs.width(), obs.height());
      if (d > 0.01 && d < mindis) {
        mindis = d;
        type = 1;
      }
    }
    // handle static obstacles
    // get the minmum distance
    for (const auto &obs : traffic.staticobstacles()) {
      Eigen::Vector3d pos(obs.x(), obs.y(), obs.z());
      coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
      double d = ult.second->distance(pos, 0, 0, obs.heading(), obs.length(), obs.width(), obs.height());
      if (d > 0.01 && d < mindis) {
        mindis = d;
        type = 2;
      }
    }
    // if minminum distance less than range
    if (mindis < ult.second->getRange()) {
      sim_msg::Ultrasonic &utsc = *(tParkingspace.add_ultrasonic());
      utsc.set_id(ult.first);
      utsc.set_distance(mindis);
      utsc.set_source_type(type);
    }
  }
}

/**
 * @brief calcalate the parking space
 *
 */
bool ParkingSpace::cal_parking(parkingbuf &pkb, double &hdg) {
  const double sin0 = 1e-6;
  Eigen::Vector3d st[4];
  for (int i = 0; i < 4; i++) {
    st[i] = pkb.p[i] - pkb.pos;
    st[i].z() = 0;
    rotate(st[i].x(), st[i].y(), hdg - pkb.yaw);
  }
  // -------road---------
  // -------road---------
  //    p0------p1
  //
  //   p3------p2
  //
  // step1: find the p with highest t
  int highest_t = 0;
  for (int i = 1; i < 4; i++) {
    if (st[i].y() > st[highest_t].y()) {
      highest_t = i;
    }
  }
  // step2: the p is p0 or p1 and set p to p0
  Eigen::Vector3d a = st[(highest_t + 3) % 4] - st[highest_t];
  Eigen::Vector3d b = st[(highest_t + 1) % 4] - st[highest_t];
  if (a.squaredNorm() < 1e-6 || b.squaredNorm() < 1e-6) {
    std::cout << "some point of parking is so closed, id = " << pkb.id << std::endl;
    return false;
  }
  a.normalize();
  b.normalize();
  if (a.x() < -b.x()) {
    highest_t = (highest_t + 3) % 4;
  }
  auto rollrect = [](Eigen::Vector3d p4[], int highest_t) {
    Eigen::Vector3d tmp[4];
    tmp[0] = p4[0];
    tmp[1] = p4[1];
    tmp[2] = p4[2];
    tmp[3] = p4[3];
    for (int i = 0; i < 4; i++) p4[i] = tmp[(i + highest_t) % 4];
  };
  rollrect(pkb.p, highest_t);
  rollrect(st, highest_t);

  // setp3
  a = st[3] - st[0];
  b = st[1] - st[0];
  double al = a.norm();
  double bl = b.norm();
  a /= al;
  b /= bl;
  if (std::abs(a.dot(b)) > sin0) {  // PARALLEL RECT
    pkb.type = 2;
  }

  if (std::abs(b.y()) < sin0) {
    double dep = pkb.depth;
    double len = pkb.length;
    if (al > bl) {
      if (pkb.type == 0) pkb.type = 1;
      pkb.length = std::min(dep, len);
      pkb.depth = std::max(dep, len);
    } else {
      pkb.length = std::max(dep, len);
      pkb.depth = std::min(dep, len);
    }
  } else {
    pkb.type = 2;

    if (std::abs(a.y()) < 1e-6) {
      std::cout << "a.y is zero: " << pkb.id << std::endl;
      return false;
    }

    double exl = bl * b.y() / a.y();
    Eigen::Vector3d n = (pkb.p[3] - pkb.p[0]).normalized();
    if (b.y() > 0) {
      // exl < 0
      pkb.p[0] += exl * n;
      pkb.p[2] += -exl * n;
    } else {
      // exl > 0
      pkb.p[1] += -exl * n;
      pkb.p[3] += exl * n;
    }
    pkb.depth = (pkb.p[3] - pkb.p[0]).norm();
    pkb.length = (pkb.p[1] - pkb.p[0]).norm();
  }
  if (pkb.depth > pkb.length) {
    pkb.yaw = std::atan2(pkb.p[3].y() - pkb.p[0].y(), pkb.p[3].x() - pkb.p[0].x());
  } else {
    pkb.yaw = std::atan2(pkb.p[1].y() - pkb.p[0].y(), pkb.p[1].x() - pkb.p[0].x());
  }
  return true;
}

void ParkingSpace::obj2pk(const hadmap::txObject *obj) {
  // read size
  double len = 0, wid = 0, hei = 0;
  obj->getLWH(len, wid, hei);
  // read rpy
  double roll = 0, pitch = 0, yaw = 0;
  obj->getRPY(roll, pitch, yaw);

  // get the road of parkingspace
  size_t roadid = obj->getRoadId();
  double hdg = 0;
  if (roadid == ROAD_PKID_INVALID) {
    hadmap::txLanePtr lane;
    if (hadmap::getLane(hMapHandle, obj->getPos(), lane, 50) == TX_HADMAP_DATA_OK) {
      roadid = lane->getRoadId();
    }
  }
  // get the hading of parkingspace
  if (roadid != ROAD_PKID_INVALID) {
    hadmap::txRoadPtr road;
    hadmap::getRoad(hMapHandle, roadid, false, road);
    if (road.get() && road->getGeometry()) {
      auto pos = obj->getPos();
      double s = 0, l = 0, ya = 0;
      if (road->getGeometry()->xy2sl(pos.x, pos.y, s, l, ya)) {
        hdg = yaw - ya * EIGEN_PI / 180.0;
        hdg = std::atan2(std::sin(hdg), std::cos(hdg));
        if (hdg < 0) {
          hdg += EIGEN_PI * 2;
        }
      }
    }
  }

  // if the parikingspace has geometry,
  bool hasgeo = false;

  for (size_t i = 0, is = obj->getGeomSize(); i < is; i++) {
    auto geo_ptr = obj->getGeom(i);
    hadmap::PointVec pts;
    dynamic_cast<const hadmap::txLineCurve *>(geo_ptr->getGeometry())->getPoints(pts);

    if (pts.size() == 4) {
      // set info
      parkingbuf pkb;
      pkb.id = obj->getId() * 100 + i;
      pkb.type = 0;
      pkb.roadid = roadid;
      pkb.depth = wid;
      pkb.length = len;
      pkb.yaw = yaw;
      // tadsim parking
      hasgeo = true;
      std::vector<Eigen::Vector3d> epts;
      pkb.pos = Eigen::Vector3d(0, 0, 0);

      // culcate the center position
      for (size_t j = 0; j < 4; ++j) {
        int jj = (3 - j) % 4;
        pkb.p[j] = Eigen::Vector3d(pts[jj].x, pts[jj].y, pts[jj].z);
        pkb.p_84[j] = pkb.p[j];
        coord_trans_api::lonlat2enu(pkb.p[j].x(), pkb.p[j].y(), pkb.p[j].z(), map_ori.x, map_ori.y, map_ori.z);

        pkb.pos += pkb.p[j];
      }
      pkb.pos *= 0.25;
      pkb.pos_84 = pkb.pos;
      coord_trans_api::enu2lonlat(pkb.pos_84.x(), pkb.pos_84.y(), pkb.pos_84.z(), map_ori.x, map_ori.y, map_ori.z);
      if (cal_parking(pkb, hdg)) parkings.push_back(pkb);
    }
  }

  // has no geometry
  if (!hasgeo) {
    if (len > 0 && wid > 0) {
      Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                             Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                             Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());

      hadmap::PointVec pts;
      if (obj->getRepeat()) {
        obj->getRepeat()->getPoints(pts);
      }
      if (pts.empty()) {
        pts.push_back(obj->getPos());
      }

      // set info
      parkingbuf pkb;
      pkb.id = obj->getId();
      pkb.type = 0;
      pkb.roadid = roadid;
      pkb.depth = wid;
      pkb.length = len;
      pkb.yaw = yaw;

      for (auto &pos : pts) {
        pkb.depth = wid;
        pkb.length = len;
        pkb.pos = Eigen::Vector3d(pos.x, pos.y, pos.z);
        pkb.pos_84 = pkb.pos;
        coord_trans_api::lonlat2enu(pkb.pos.x(), pkb.pos.y(), pkb.pos.z(), map_ori.x, map_ori.y, map_ori.z);

        // calculate corner by size
        double skew = pkb.depth * 0.5 * std::tan(hdg * EIGEN_PI / 180.);
        pkb.p[0] = pkb.pos + q * Eigen::Vector3d(-len * 0.5 - skew, wid * 0.5, 0);
        pkb.p[1] = pkb.pos + q * Eigen::Vector3d(len * 0.5 - skew, wid * 0.5, 0);
        pkb.p[2] = pkb.pos + q * Eigen::Vector3d(len * 0.5 + skew, -wid * 0.5, 0);
        pkb.p[3] = pkb.pos + q * Eigen::Vector3d(-len * 0.5 + skew, -wid * 0.5, 0);
        // recover to wgs84
        for (size_t j = 0; j < 4; ++j) {
          pkb.p_84[j] = pkb.p[j];
          coord_trans_api::enu2lonlat(pkb.p_84[j].x(), pkb.p_84[j].y(), pkb.p_84[j].z(), map_ori.x, map_ori.y,
                                      map_ori.z);
        }
        if (cal_parking(pkb, hdg)) parkings.push_back(pkb);
      }
    }
  }
}
/**
 * @brief init map
 *
 * @return true on success
 * @return false on failure
 */
bool ParkingSpace::InitMap() {
  using namespace hadmap;
  // static hadmap::txMapHandle* maphandle = nullptr;
  // if (maphandle == hMapHandle)
  //{
  // return true;
  // }
  // if (!hMapHandle)
  //{
  // return false;
  // }
  // maphandle = hMapHandle;
  bpFlann.Clear();
  parkings.clear();

  // some threshold
  const double d90 = EIGEN_PI * 0.5;
  const double d45 = EIGEN_PI * 0.25;
  const double d5 = EIGEN_PI * 5.0 / 180.0;
  const double sin5 = std::sin(EIGEN_PI * 5.0 / 180.0);
  const double tan5 = std::tan(EIGEN_PI * 5.0 / 180.0);

  // read all object.
  // filter by parkingspace
  for (auto &obj : mapObjects) {
    if (obj->getObjectType() != OBJECT_TYPE_ParkingSpace && obj->getObjectType() != OBJECT_TYPE_RR_Parking) {
      continue;
    }

    obj2pk(obj.get());
  }
  std::cout << "Get " << parkings.size() << " parkings from map." << std::endl;

  // print all corner pos of all parkingspace
  for (size_t i = 0; i < parkings.size(); i++) {
    double p[2] = {parkings[i].pos.x(), parkings[i].pos.y()};
    bpFlann.Add(p, i);
    //*
    std::cout << "A" << parkings[i].id << "_" << parkings[i].type << " " << parkings[i].p[0].x() << " "
              << parkings[i].p[0].y() << " " << parkings[i].p[0].z() << std::endl;
    std::cout << "B"
              << "_" << parkings[i].length << " " << parkings[i].p[1].x() << " " << parkings[i].p[1].y() << " "
              << parkings[i].p[1].z() << std::endl;
    std::cout << "C"
              << "_" << parkings[i].depth << " " << parkings[i].p[2].x() << " " << parkings[i].p[2].y() << " "
              << parkings[i].p[2].z() << std::endl;
    std::cout << "D"
              << "_" << parkings[i].yaw << " " << parkings[i].p[3].x() << " " << parkings[i].p[3].y() << " "
              << parkings[i].p[3].z() << std::endl;  //*/
  }
  // build flann
  if (bpFlann.Build()) {
    std::cout << "Init map success in ParkingSpace." << std::endl;
  } else {
    std::cout << "Init map faild in ParkingSpace." << std::endl;
  }

  return true;
}
