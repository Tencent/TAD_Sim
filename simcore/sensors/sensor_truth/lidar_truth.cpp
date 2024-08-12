/**
 * @file LidarTruth.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "lidar_truth.h"
#include <google/protobuf/util/json_util.h>
#include <boost/geometry.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "lidar_sensor.h"
#include "object_size.h"
#include "osi_util.h"
#include "user_topic.h"
#include "visable_calculate.h"
#include "common/coord_trans.h"
#include "location.pb.h"
#include "osi_datarecording.pb.h"
#include "traffic.pb.h"
#include "union.pb.h"

/**
 * @brief lidar truth init
 *
 * @param helper helper of txsim
 */
void LidarTruth::Init(tx_sim::InitHelper &helper) {
  // get user defined initiation parameters.
  // if we defined the parameters in TADSim UI, override the default values
  // here.

  // get min hit num
  std::string value = helper.GetParameter("LT_MinHitNum");
  if (!value.empty()) {
    minHitNum = atof(value.c_str());
  }
  // get completeness
  value = helper.GetParameter("LT_Completeness");
  if (!value.empty()) {
    completeness = atof(value.c_str());
  }
  std::cout << "LT_MinHitNum = " << minHitNum << std::endl;
  std::cout << "LT_Completeness = " << completeness << std::endl;

  // by subscribe our interested topics, we expect that the two corresponding
  // messages which defined by traffic.proto and location.proto would received
  // in every Step callback.
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Subscribe(tx_sim::topic::kLocation);
}

/**
 * @brief lidar truth step
 *
 * @param helper helper of txsim
 */
void LidarTruth::Step(tx_sim::StepHelper &helper) {
  if (lidars.empty()) {
    return;
  }
  if (!osiMaker) {
    return;
  }

  // get traffic message
  sim_msg::Traffic traffic = getTraffic(helper);
  // get location
  sim_msg::Location loc = getLocation(helper);

  std::cout << "[LidarTruth(" << helper.timestamp() << ")]: ";

  Eigen::Vector3d locPosition(loc.position().x(), loc.position().y(), loc.position().z());
  // to enu
  coord_trans_api::lonlat2enu(locPosition.x(), locPosition.y(), locPosition.z(), map_ori.x, map_ori.y, map_ori.z);

  // 2. get messages we subscribed.
  sim_msg::Union union_location = getUnion(helper);
  for (auto lid : lidars) {
    auto sensor_feature = osiMaker->AddLidar();
    sensor_feature->mutable_header()->mutable_sensor_id()->set_value(lid.first);
    // Eigen::Vector3d slot_p, slot_a;
    // lid.second->getPosition(slot_p);
    // lid.second->getRotation(slot_a);
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_position()->set_x(slot_p.x());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_position()->set_y(slot_p.y());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_position()->set_z(slot_p.z());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_roll(slot_a.x());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_pitch(slot_a.y());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_yaw(slot_a.z());

    // set the car position and rotation
    lid.second->setCarPosition(locPosition);
    lid.second->setCarRoatation(loc.rpy().x(), loc.rpy().y(), loc.rpy().z());

    // setup visable buffers
    std::vector<VisableBuffer> vbs;
    handle_traffic(*lid.second, union_location, traffic, locPosition, vbs);

    // calculate visible
    ReVisable(vbs);
    int ccc[5] = {0};
    for (const auto &vb : vbs) {
      if (minHitNum > 0 && completeness > 0) {
        bool visable = (vb.area >= minHitNum) && (vb.area0 > 0 && (vb.area / vb.area0) > completeness);
        if (!visable) {
          continue;
        }
      }
      int64_t id = 0;
      if (vb.tag < 0) {
        // handle the ego
        const sim_msg::Location &car = *static_cast<const sim_msg::Location *>(vb.rawPtr);
        id = -vb.tag;
        osiMaker->Add(car, id);
        ccc[0]++;
      } else if (vb.tag == 0) {
        // handle the car
        const sim_msg::Car &car = *static_cast<const sim_msg::Car *>(vb.rawPtr);
        osiMaker->Add(car);
        id = getUUID(car);
        ccc[1]++;
      } else if (vb.tag == 1) {
        // handle the dynamic obstacle
        const sim_msg::DynamicObstacle &obs = *static_cast<const sim_msg::DynamicObstacle *>(vb.rawPtr);
        osiMaker->Add(obs);
        id = getUUID(obs);
        ccc[2]++;
      } else if (vb.tag == 2) {
        // handle the static obstacle
        const sim_msg::StaticObstacle &sta = *static_cast<const sim_msg::StaticObstacle *>(vb.rawPtr);
        osiMaker->Add(sta);
        id = getUUID(sta);
        ccc[3]++;
      } else {
        continue;
      }
      auto detection = sensor_feature->add_detection();
      detection->mutable_object_id()->set_value(id);
    }

    // handle the traffic lights
    for (const auto &light : traffic.trafficlights()) {
      Eigen::Vector3d pos(light.x(), light.y(), light.z());
      coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
      Eigen::Vector2d uv;
      if (!lid.second->world2pixel(pos, uv)) {
        continue;
      }
      osiMaker->Add(light);
      ccc[4]++;
    }
    std::cout << "lidar[" << lid.first << "] with " << ccc[0] << " ego, " << ccc[1] << " car, " << ccc[2]
              << " dyn_objects, " << ccc[3] << " stationary_objects. " << ccc[4] << " trafficlights.";
  }
  std::cout << "\n";
}

// calculate the box points of the object
points LidarTruth::VbufferInit(const ObjectBBox &bbox, double heading, const Eigen::Vector3d &pos,
                               const LidarSensor &lid) {
  points uvs;
  const double sample_box = 1.0;
  for (const auto &p : bbox.getBboxPts(pos, heading, sample_box, 0.9)) {
    Eigen::Vector2d uv;
    if (lid.world2pixel(p, uv)) {
      uvs.push_back(point(uv.x(), uv.y()));
    }
  }
  return uvs;
}

/// @brief
/// @param cam
/// @param union_location
/// @param locPosition
/// @param vbs
void LidarTruth::handle_traffic(const LidarSensor &lid, const sim_msg::Union &union_location,
                                const sim_msg::Traffic &traffic, const Eigen::Vector3d &locPosition,
                                std::vector<VisableBuffer> &vbs) {
  // handle other ego
  for (const auto &msg : union_location.messages()) {
    std::int64_t eid = std::atoi(msg.groupname().substr(msg.groupname().length() - 3).c_str());
    if (eid == ego_id) continue;
    sim_msg::Location uloc;
    if (uloc.ParseFromString(msg.content())) {
      Eigen::Vector3d pos(uloc.position().x(), uloc.position().y(), uloc.position().z());
      coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
      double distance = (pos - locPosition).norm();
      // out of range
      if (distance > lid.getRange()) {
        continue;
      }
      // update the size
      auto &objbb = ego_Bboxes[eid];
      if (objbb.category < 0) {
        std::cout << "Cannot find ego type: " << eid << std::endl;
        continue;
      }
      VisableBuffer vb;
      // init the buffer
      if (!vb.init(VbufferInit(objbb, uloc.rpy().z(), pos, lid))) {
        continue;
      }
      vb.tag = -1;
      vb.rawPtr = static_cast<const void *>(&uloc);
      vb.distance = distance;
      vbs.push_back(vb);
    }
  }

  // handle the cars
  for (const auto &car : traffic.cars()) {
    Eigen::Vector3d carpos(car.x(), car.y(), car.z());
    coord_trans_api::lonlat2enu(carpos.x(), carpos.y(), carpos.z(), map_ori.x, map_ori.y, map_ori.z);

    double d_car = (carpos - locPosition).norm();
    // out of range
    if (d_car > lid.getRange()) {
      continue;
    }
    // update the size
    auto &objbb = car_Bboxes[car.type()];
    VisableBuffer vb;
    // init the buffer
    if (!vb.init(VbufferInit(objbb, car.heading(), carpos, lid))) {
      continue;
    }
    vb.tag = 0;
    vb.rawPtr = static_cast<const void *>(&car);
    vb.distance = d_car;
    vbs.push_back(vb);
  }
  // handle the dynamic obstacles
  for (const auto &obs : traffic.dynamicobstacles()) {
    Eigen::Vector3d pos(obs.x(), obs.y(), obs.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    double distance = (pos - locPosition).norm();
    // out of range
    if (distance > lid.getRange()) {
      continue;
    }
    auto objbb = dyn_Bboxes[obs.type()];
    if (objbb.category < 0) {
      objbb = car_Bboxes[obs.type()];
    }
    // update the size
    if (objbb.category < 0) {
      std::cout << "Cannot find dynamic obstracle type " << obs.type() << std::endl;
      continue;
    }
    VisableBuffer vb;
    // init the buffer
    if (!vb.init(VbufferInit(objbb, obs.heading(), pos, lid))) {
      continue;
    }
    vb.tag = 1;
    vb.rawPtr = static_cast<const void *>(&obs);
    vb.distance = distance;
    vbs.push_back(vb);
  }
  // handle the dynamic obstacles
  for (const auto &sta : traffic.staticobstacles()) {
    if (sta.height() < 0.01 && sta.width() < 0.01 && sta.length() < 0.01) {
      continue;
    }
    Eigen::Vector3d pos(sta.x(), sta.y(), sta.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    double distance = (pos - locPosition).norm();
    // out of range
    if (distance > lid.getRange()) {
      continue;
    }
    // update the size
    auto &objbb = sta_Bboxes[sta.type()];
    VisableBuffer vb;
    // init the buffer
    if (!vb.init(VbufferInit(objbb, sta.heading(), pos, lid))) {
      continue;
    }
    vb.tag = 2;
    vb.rawPtr = static_cast<const void *>(&sta);
    vb.distance = distance;
    vbs.push_back(vb);
  }
}
