/**
 * @file radar.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "radar.h"
#include <google/protobuf/util/json_util.h>
#include <stdio.h>
#include <Eigen/Core>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include "Eigen/Geometry"
#include "object_size.h"
#include "coord_trans.h"
#include "location.pb.h"
#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"
#include "osi_common.pb.h"
#include "osi_featuredata.pb.h"
#include "traffic.pb.h"
#include "union.pb.h"

#define TOPIC std::string("OSI_RADAR")
#define DYN_ID0 100000
#define STA_ID0 200000
#define MAP_ID0 300000

/**
 * @brief Construct a new Sim Radar:: Sim Radar object
 *
 */
SimRadar::SimRadar() {}

/**
 * @brief Destroy the Sim Radar:: Sim Radar object
 *
 */
SimRadar::~SimRadar() {}

/**
 * @brief radar init
 *
 * @param helper helper of txsim
 */
void SimRadar::Init(tx_sim::InitHelper &helper) {
  // get debug dir
  debugDir = helper.GetParameter("DebugDir");
  // get ignore z
  ignoreZ = helper.GetParameter("IgnoreZ") == "ON";
  if (ignoreZ) {
    std::cout << "Ignore Z" << std::endl;
  }
  // get tadsim path
  tadsimPath = helper.GetParameter("tadsim_path");
  std::cout << "tadsimPath: " << tadsimPath << std::endl;
  // get device id
  device = helper.GetParameter("-device");
  if (device == "all") {
    device.clear();
  }
  helper.Publish(TOPIC + (device.empty() ? "" : "_") + device);
  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Subscribe(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation);
}

/**
 * @brief radar reset
 *
 * @param helper helper of txsim
 */
void SimRadar::Reset(tx_sim::ResetHelper &helper) {
  tmpdatas.clear();
  ls_traffic.clear();
  objects.clear();
  bpFlann.Clear();
  map_ori = helper.map_local_origin();
  // load radar sensors
  if (!LoadRadarSensors(helper.scene_pb(), helper.group_name())) std::cout << "Cannot load radar" << std::endl;

  // load hdmap
  std::string hdfile = helper.map_file_path();
  if (!tadsimPath.empty()) {
    auto hdname = boost::filesystem::path(hdfile).filename();
    hdfile = (boost::filesystem::path(tadsimPath) / "data/scenario/hadmap" / hdname).string();
  }
  LoadMap(hdfile);

  max_delay = 0;
  // handle all radar
  for (const auto &radar : radarSensors) {
    max_delay = std::max(max_delay, radar.second->Delay());
    // cannot handle enough radar
    if (radar.first >= MAX_RAD_NUM) {
      std::cout << "radar id out of range " << MAX_RAD_NUM << std::endl;
      continue;
    }
    // radar signal init
    radar_bit::output initO = radar_bit::Radar_Initialization(radar.second->Para(), radar.first);

    // get the signal output
    initOut.delta_R[radar.first] = initO.delta_R[radar.first];
    initOut.delta_V[radar.first] = initO.delta_V[radar.first];
    initOut.hfov[radar.first] = initO.hfov[radar.first];
    initOut.delta_A[radar.first] = initO.delta_A[radar.first];
    initOut.R_max[radar.first] = initO.R_max[radar.first];
    initOut.sigma_R[radar.first] = initO.sigma_R[radar.first];
    initOut.sigma_V[radar.first] = initO.sigma_V[radar.first];
    initOut.sigma_A[radar.first] = initO.sigma_A[radar.first];

    radar.second->MaxDistance() = initO.R_max[radar.first];

    // print init param
    std::cout << "Radar " << radar.first << "`s initOut.delta_R is: " << initOut.delta_R[radar.first] << std::endl;
    std::cout << "Radar " << radar.first << "`s initOut.delta_V is: " << initOut.delta_V[radar.first] << std::endl;
    std::cout << "Radar " << radar.first << "`s initOut.hfov is: " << initOut.hfov[radar.first] << std::endl;
    std::cout << "Radar " << radar.first << "`s initOut.delta_A is: " << initOut.delta_A[radar.first] << std::endl;
    std::cout << "Radar " << radar.first << "`s initOut.R_max is: " << initOut.R_max[radar.first] << std::endl;
    std::cout << "Radar " << radar.first << "`s initOut.sigma_R is: " << initOut.sigma_R[radar.first] << std::endl;
    std::cout << "Radar " << radar.first << "`s initOut.sigma_V is: " << initOut.sigma_V[radar.first] << std::endl;
    std::cout << "Radar " << radar.first << "`s initOut.sigma_A is: " << initOut.sigma_A[radar.first] << std::endl;
  }
}

/**
 * @brief radar setp
 *
 * @param helper helper of txsim
 */
void SimRadar::Step(tx_sim::StepHelper &helper) {
  double time_stamp = helper.timestamp();
  std::cout << "[" << time_stamp << "]: ";

  // 2. get messages we subscribed.
  // get traffic
  std::string payload_, payload2_;
  helper.GetSubscribedMessage(tx_sim::topic::kTraffic, payload_);
  helper.GetSubscribedMessage(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation, payload2_);
  ls_traffic.push_back(std::make_pair(helper.timestamp(), std::make_pair(0, std::make_pair(payload_, payload2_))));

  // get location
  helper.GetSubscribedMessage(tx_sim::topic::kLocation, payload_);
  sim_msg::Location loc;
  loc.ParseFromString(payload_);
  Eigen::Vector3d egop(loc.position().x(), loc.position().y(), ignoreZ ? 0 : loc.position().z());
  coord_trans_api::lonlat2enu(egop.x(), egop.y(), egop.z(), map_ori.x, map_ori.y, map_ori.z);
  Eigen::Vector3d egov(loc.velocity().x(), loc.velocity().y(), loc.velocity().z());

  // get weather
  auto weather = getWeather(helper.timestamp());
  // get radar info
  osi3::FeatureData ftdatapb;
  for (auto &radar : radarSensors) {
    // handle by frequency
    if (radar.second->Frequency() > 0 &&
        (time_stamp - radar.second->Timestamp()) * 0.001 < 1. / radar.second->Frequency()) {
      continue;
    }
    radar.second->Timestamp() = time_stamp;

    // handle delay
    double target_timestamp = helper.timestamp() - radar.second->Delay() + 1e-4;
    std::pair<size_t, std::pair<std::string, std::string>> *traffic_buf = nullptr;
    for (auto &buf : ls_traffic) {
      if (buf.first > target_timestamp) {
        break;
      }
      traffic_buf = &buf.second;
    }
    if (!traffic_buf) {
      continue;
    }
    traffic_buf->first += 1;
    // parse traffic
    sim_msg::Traffic traffic;
    traffic.ParseFromString(traffic_buf->second.first);
    sim_msg::Union union_location;
    union_location.ParseFromString(traffic_buf->second.second);

    if (radar.first >= MAX_RAD_NUM) {
      continue;
    }
    // set radar position and rotation
    radar.second->setCarPosition(egop);
    radar.second->setCarRoatation(loc.rpy().x(), loc.rpy().y(), loc.rpy().z());
    Eigen::Vector3d senser_postion = radar.second->LocalVector(Eigen::Vector3d(0, 0, 0));
    Eigen::Vector3d senser_rotation = radar.second->LocalRotator(Eigen::Vector3d(0, 0, 0));
    Eigen::Vector3d senser_postion_gps = senser_postion;
    coord_trans_api::enu2lonlat(senser_postion_gps.x(), senser_postion_gps.y(), senser_postion_gps.z(), map_ori.x,
                                map_ori.y, map_ori.z);

    std::vector<radar_bit::tar_info_in> infoArry =
        handle_traffic(*radar.second, senser_postion, egov, union_location, traffic);

    // find out nearby objects
    std::vector<size_t> indexs;
    std::vector<double> diss;
    double p[2] = {egop.x(), egop.y()};
    if (bpFlann.SearchRadius(p, indexs, diss, initOut.R_max[radar.first] * initOut.R_max[radar.first])) {
      for (auto i : indexs) {
        const auto &object = objects[i];
        Eigen::Vector3d pos(object.cenx, object.ceny, ignoreZ ? 0 : object.cenz);
        Eigen::Vector3d c = radar.second->Vector(pos);
        if (c.x() < 0 || c.norm() > initOut.R_max[radar.first] * 1.4) {
          continue;
        }
        if (!radar.second->inFov(object.getBboxPts(pos, object.angle))) {
          continue;
        }
        radar_bit::tar_info_in info;
        info.tar_ID = object.id + MAP_ID0;
        info.tar_range = c.norm();
        info.tar_speed = (pos - senser_postion).normalized().dot(-egov);
        info.tar_angle = -std::atan2(c.y(), c.x()) * 180. / EIGEN_PI;
        info.tar_rcs = object.rcs;
        infoArry.push_back(info);

        if (info.tar_range > 1e-4) tmpdatas[object.id + MAP_ID0].vert_angle = std::asin(c.z() / info.tar_range);
        tmpdatas[object.id + MAP_ID0].size = std::max(0.1, std::max(object.len, object.wid));
      }
    }

    // sort by distance
    std::sort(infoArry.begin(), infoArry.end(),
              [](const radar_bit::tar_info_in &rhs1, const radar_bit::tar_info_in &rhs2) {
                return rhs1.tar_range < rhs2.tar_range;
              });

    // signal handle
    radar_bit::tar_info_out OutInfo =
        radar_bit::Rar_signal_pro(0, infoArry.data(), infoArry.size(), initOut, radar.first, weather);

    osi3::RadarDetectionData *radarpb = ftdatapb.add_radar_sensor();
    // write time stamp
    int64_t ts = (int64_t)std::floor(time_stamp);
    radarpb->mutable_header()->mutable_measurement_time()->set_seconds(ts / 1000);
    radarpb->mutable_header()->mutable_measurement_time()->set_nanos((ts % 1000) * 1000000);
    // write id
    radarpb->mutable_header()->mutable_sensor_id()->set_value(radar.first);

    // write position
    radarpb->mutable_header()->mutable_mounting_position()->mutable_position()->set_x(senser_postion_gps.x());
    radarpb->mutable_header()->mutable_mounting_position()->mutable_position()->set_y(senser_postion_gps.y());
    radarpb->mutable_header()->mutable_mounting_position()->mutable_position()->set_z(senser_postion_gps.z());
    // write orientation
    radarpb->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_roll(senser_rotation.x());
    radarpb->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_pitch(senser_rotation.y());
    radarpb->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_yaw(senser_rotation.z());

    std::cout << "radar(" << radar.first << ")=" << OutInfo.tar_num << "/" << infoArry.size() << "  ";

    // default insert
    auto customFault = radar.second->getCustomFault();
    auto fault_fd = customFault.upper_bound(time_stamp);
    // insert fault
    if (!customFault.empty() && fault_fd != customFault.begin()) {
      fault_fd--;
      for (size_t i = 0; i < OutInfo.tar_num; i++) {
        auto ft = fault_fd->second.find(OutInfo.tar_ID[i]);
        if (ft != fault_fd->second.end()) {
          if (ft->second.distance != FAULT_INVALID) {
            OutInfo.tar_range[i] = ft->second.distance;
          }
          if (ft->second.speed != FAULT_INVALID) {
            OutInfo.tar_speed[i] = ft->second.speed;
          }
          if (ft->second.angle != FAULT_INVALID) {
            OutInfo.tar_angle[i] = ft->second.angle;
          }
          if (ft->second.rcs != FAULT_INVALID) {
            OutInfo.tar_rcs[i] = ft->second.rcs;
          }
          if (ft->second.ar != FAULT_INVALID) {
            OutInfo.tar_Ar[i] = ft->second.ar;
          }
        }
      }
    }

    // add detection
    for (size_t i = 0; i < OutInfo.tar_num; i++) {
      osi3::RadarDetection *obj = radarpb->add_detection();
      obj->mutable_object_id()->set_value(OutInfo.tar_ID[i]);
      obj->set_radial_velocity(OutInfo.tar_speed[i]);
      obj->set_rcs(OutInfo.tar_rcs[i]);
      obj->mutable_position()->set_azimuth(OutInfo.tar_angle[i] * EIGEN_PI / 180.);
      obj->mutable_position()->set_distance(OutInfo.tar_range[i]);
    }
  }
  // publish message
  std::string buf;
  if (ftdatapb.SerializeToString(&buf)) helper.PublishMessage(TOPIC + (device.empty() ? "" : "_") + device, buf);

  // print debug message
  if (!debugDir.empty()) {
    std::ofstream debug(debugDir + "/radar_" + std::to_string(helper.timestamp()) + ".json");
    std::string json;
    google::protobuf::util::MessageToJsonString(ftdatapb, &json);
    debug << json;
    debug.close();
  }

  double farest_timestamp = helper.timestamp() - max_delay - 1e-4;
  for (auto ls = ls_traffic.begin(); ls != ls_traffic.end();) {
    if (ls->first < max_delay) {
      ls = ls_traffic.erase(ls);
    } else {
      ls++;
    }
  }

  std::cout << "\n";
}

/**
 * @brief radar stop
 *
 * @param helper helper of txsim
 */
void SimRadar::Stop(tx_sim::StopHelper &helper) {}

/**
 * @brief load map
 *
 * @param fname path to the map file
 * @return true on success
 * @return false on failure
 */
bool SimRadar::LoadMap(const std::string &fname) {
  using namespace hadmap;
  txMapHandle *hMapHandle = 0;
  auto ftype = NONE;
  if (fname.substr(fname.size() - 5) == ".xodr") {
    // hanle opendrive
    ftype = OPENDRIVE;
  } else if (fname.substr(fname.size() - 7) == ".sqlite") {
    // hanle sqlite
    ftype = SQLITE;
  } else {
    std::cout << "Load map falid. Unsupport file type." << fname << std::endl;
    return false;
  }
  if (TX_HADMAP_HANDLE_OK != hadmapConnect(fname.c_str(), ftype, &hMapHandle)) {
    std::cout << "Load map falid." << fname << std::endl;
    return false;
  }
  // get all objects
  std::vector<OBJECT_TYPE> objtypes;
  objtypes.push_back(OBJECT_TYPE_Pole);
  objtypes.push_back(OBJECT_TYPE_SpeedBump);
  objtypes.push_back(OBJECT_TYPE_TrafficBarrier);
  txObjects mapObjects;
  getObjects(hMapHandle, std::vector<hadmap::txLaneId>(), objtypes, mapObjects);

  bpFlann.Clear();
  objects.clear();
  for (const auto &mapObj : mapObjects) {
    Object obj;
    obj.id = mapObj->getId();
    obj.cenx = mapObj->getPos().x;
    obj.ceny = mapObj->getPos().y;
    obj.cenz = mapObj->getPos().z;
    coord_trans_api::enu2lonlat(obj.cenx, obj.ceny, obj.cenz, map_ori.x, map_ori.y, map_ori.z);

    // get object info: angle and size
    double roll = 0, pitch = 0;
    mapObj->getRPY(roll, pitch, obj.angle);
    mapObj->getLWH(obj.len, obj.wid, obj.hei);
    auto objType = mapObj->getObjectType();
    if (objType == OBJECT_TYPE_Pole) {
      obj.rcs = 1;
    } else if (objType == OBJECT_TYPE_SpeedBump) {
      obj.rcs = 1;
    } else if (objType == OBJECT_TYPE_TrafficBarrier) {
      obj.rcs = 1;
    }
    objects.push_back(obj);
  }
  for (size_t i = 0; i < objects.size(); i++) {
    double p[2] = {objects[i].cenx, objects[i].ceny};
    bpFlann.Add(p, i);
    /*
    std::cout << "A " << parkings[i].p[0].x() << " " << parkings[i].p[0].y()
    << " " << parkings[i].p[0].z() << std::endl; std::cout << "B " <<
    parkings[i].p[1].x() << " " << parkings[i].p[1].y() << " " <<
    parkings[i].p[1].z() << std::endl; std::cout << "C " << parkings[i].p[2].x()
    << " " << parkings[i].p[2].y() << " " << parkings[i].p[2].z() << std::endl;
    std::cout << "D " << parkings[i].p[3].x() << " " << parkings[i].p[3].y() <<
    " " << parkings[i].p[3].z() << std::endl;
    */
  }
  // build flann
  if (bpFlann.Build()) {
    std::cout << "Flann success." << std::endl;
  } else {
    std::cout << "Flann faild." << std::endl;
  }

  hadmapClose(&hMapHandle);
  return true;
}
/**
 * @brief get car rcs
 *
 * @param a angle of car
 */
double SimRadar::car_rcs(double a) {
  double angle = std::abs(a);
  return (1.f / 5075.) * angle * angle - (2. / 225.) * angle + 3.2;
}
/**
 * @brief get man rcs
 *
 * @param a angle of man
 */
double SimRadar::man_rcs(double a) {
  double angle = std::abs(a);
  return -(3. / 81000.) * angle * angle - (1. / 900.) * angle + 1.;
}
/**
 * @brief get object rcs
 *
 * @param a angle of object
 */
double SimRadar::obj_rcs(double a) { return 1.; }

std::vector<radar_bit::tar_info_in> SimRadar::handle_traffic(const Sensor &radar, const Eigen::Vector3d &senser_postion,
                                                             const Eigen::Vector3d &egov,
                                                             const sim_msg::Union &union_location,
                                                             const sim_msg::Traffic &traffic) {
  std::vector<radar_bit::tar_info_in> infoArry;
  // handle other ego
  for (const auto &msg : union_location.messages()) {
    std::int64_t eid = std::atoi(msg.groupname().substr(msg.groupname().length() - 3).c_str());
    if (eid == ego_id) continue;
    sim_msg::Location uloc;
    if (uloc.ParseFromString(msg.content())) {
      Eigen::Vector3d pos(uloc.position().x(), uloc.position().y(), uloc.position().z());
      coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
      Eigen::Vector3d c = radar.Vector(pos);
      if (c.x() < 0 || c.norm() > initOut.R_max[radar.ID()] * 1.4) {
        // out of radar range
        continue;
      }
      // update size of car
      auto &objbb = ego_Bboxes[eid];
      // judge if in radar fov
      if (!radar.inFov(objbb.getBboxPts(pos, uloc.rpy().z()))) {
        continue;
      }
      // fill signale info
      radar_bit::tar_info_in info;
      info.tar_ID = -eid;
      info.tar_range = c.norm();
      double carv = uloc.velocity().x() * uloc.velocity().x() + uloc.velocity().y() * uloc.velocity().y();
      if (carv > 0.0001) carv = std::sqrt(carv);
      info.tar_speed =
          (pos - senser_postion)
              .normalized()
              .dot((Eigen::Vector3d(carv * std::cos(uloc.rpy().z()), carv * std::sin(uloc.rpy().z()), 0) - egov));
      info.tar_angle = -std::atan2(c.y(), c.x()) * 180. / EIGEN_PI;
      info.tar_rcs = car_rcs(info.tar_angle);
      infoArry.push_back(info);

      if (info.tar_range > 1e-4)
        tmpdatas[-eid].vert_angle = std::acos(std::abs(c.z()) / info.tar_range) * 180. / EIGEN_PI;

      // fill temdates
      Eigen::Vector3d sita = radar.Rotator(Eigen::Vector3d(std::cos(uloc.rpy().z()), std::sin(uloc.rpy().z()), 0));
      tmpdatas[-eid].hor_sita = std::atan2(sita.y(), sita.x()) * 180. / EIGEN_PI;
      tmpdatas[-eid].vert_sita = std::asin(sita.z()) * 180. / EIGEN_PI;
      tmpdatas[-eid].size = std::max(0.1, objbb.wid);
    }
  }
  // handle car
  for (const auto &car : traffic.cars()) {
    Eigen::Vector3d pos(car.x(), car.y(), ignoreZ ? 0 : car.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    Eigen::Vector3d c = radar.Vector(pos);
    if (c.x() < 0 || c.norm() > initOut.R_max[radar.ID()] * 1.4) {
      // out of radar range
      continue;
    }
    // update size of car
    auto &objbb = car_Bboxes[car.type()];
    objbb.len = car.length();
    objbb.wid = car.width();
    objbb.hei = car.height();
    // judge if in radar fov
    if (!radar.inFov(objbb.getBboxPts(pos, car.heading()))) {
      continue;
    }
    // fill signale info
    radar_bit::tar_info_in info;
    info.tar_ID = car.id();
    info.tar_range = c.norm();
    double carv = std::sqrt(car.v() * car.v() + car.vl() * car.vl());
    info.tar_speed =
        (pos - senser_postion)
            .normalized()
            .dot((Eigen::Vector3d(carv * std::cos(car.heading()), carv * std::sin(car.heading()), 0) - egov));
    info.tar_angle = -std::atan2(c.y(), c.x()) * 180. / EIGEN_PI;
    info.tar_rcs = car_rcs(info.tar_angle);
    infoArry.push_back(info);

    if (info.tar_range > 1e-4)
      tmpdatas[car.id()].vert_angle = std::acos(std::abs(c.z()) / info.tar_range) * 180. / EIGEN_PI;

    // fill temdates
    Eigen::Vector3d sita = radar.Rotator(Eigen::Vector3d(std::cos(car.heading()), std::sin(car.heading()), 0));
    tmpdatas[car.id()].hor_sita = std::atan2(sita.y(), sita.x()) * 180. / EIGEN_PI;
    tmpdatas[car.id()].vert_sita = std::asin(sita.z()) * 180. / EIGEN_PI;
    tmpdatas[car.id()].size = std::max(0.1, car.width());
  }
  // handle static obstacles
  for (const auto &sta : traffic.staticobstacles()) {
    Eigen::Vector3d pos(sta.x(), sta.y(), ignoreZ ? 0 : sta.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    Eigen::Vector3d c = radar.Vector(pos);
    if (c.x() < 0 || c.norm() > initOut.R_max[radar.ID()] * 1.4) {
      // out of radar range
      continue;
    }
    // update size
    auto &objbb = sta_Bboxes[sta.type()];
    objbb.len = sta.length();
    objbb.wid = sta.width();
    objbb.hei = sta.height();
    // judge if in radar fov
    if (!radar.inFov(objbb.getBboxPts(pos, sta.heading()))) {
      continue;
    }
    // fill signale info
    radar_bit::tar_info_in info;
    info.tar_ID = sta.id() + STA_ID0;
    info.tar_range = c.norm();
    info.tar_speed = (pos - senser_postion).normalized().dot(-egov);
    info.tar_angle = -std::atan2(c.y(), c.x()) * 180. / EIGEN_PI;
    info.tar_rcs = obj_rcs(info.tar_angle);
    infoArry.push_back(info);

    // fill tmpdatas
    if (info.tar_range > 1e-4) tmpdatas[sta.id() + STA_ID0].vert_angle = std::asin(c.z() / info.tar_range);
    tmpdatas[sta.id() + STA_ID0].size = std::max(0.1, sta.width());
  }
  // handle dynamic obstacles
  for (const auto &dyn : traffic.dynamicobstacles()) {
    Eigen::Vector3d pos(dyn.x(), dyn.y(), ignoreZ ? 0 : dyn.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    Eigen::Vector3d c = radar.Vector(pos);
    if (c.x() < 0 || c.norm() > initOut.R_max[radar.ID()] * 1.4) {
      // out of radar range
      continue;
    }
    // update size
    auto &objbb = dyn_Bboxes[dyn.type()];
    objbb.len = dyn.length();
    objbb.wid = dyn.width();
    objbb.hei = dyn.height();
    // judge if in radar fov
    if (!radar.inFov(objbb.getBboxPts(pos, dyn.heading()))) {
      continue;
    }
    // fill signale info
    radar_bit::tar_info_in info;
    info.tar_ID = DYN_ID0 - dyn.id();
    info.tar_range = c.norm();
    info.tar_speed =
        (pos - senser_postion)
            .normalized()
            .dot((Eigen::Vector3d(dyn.v() * std::cos(dyn.heading()), dyn.v() * std::sin(dyn.heading()), 0) - egov));
    info.tar_angle = -std::atan2(c.y(), c.x()) * 180. / EIGEN_PI;
    info.tar_rcs = man_rcs(info.tar_angle);
    infoArry.push_back(info);

    if (info.tar_range > 1e-4)
      tmpdatas[dyn.id()].vert_angle = std::acos(std::abs(c.z()) / info.tar_range) * 180. / EIGEN_PI;

    // fill tmpdata
    Eigen::Vector3d sita = radar.Rotator(Eigen::Vector3d(std::cos(dyn.heading()), std::sin(dyn.heading()), 0));
    tmpdatas[dyn.id()].hor_sita = std::atan2(sita.y(), sita.x()) * 180. / EIGEN_PI;
    tmpdatas[dyn.id()].vert_sita = std::asin(sita.z()) * 180. / EIGEN_PI;
    tmpdatas[dyn.id()].size = std::max(0.1, dyn.width());
  }
  // obj
  return infoArry;
}

TXSIM_MODULE(SimRadar)
