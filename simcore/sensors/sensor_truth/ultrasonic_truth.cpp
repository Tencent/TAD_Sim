/**
 * @file UltrasonicTruth.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "ultrasonic_truth.h"
#include <google/protobuf/util/json_util.h>
#include <boost/geometry.hpp>
#include <boost/preprocessor.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "object_size.h"
#include "osi_util.h"
#include "ultrasonic_sensor.h"
#include "user_topic.h"
#include "visable_calculate.h"
#include "common/coord_trans.h"
#include "location.pb.h"
#include "osi_datarecording.pb.h"
#include "traffic.pb.h"
#include "union.pb.h"

/**
 * @brief ultrasonic truth init
 *
 * @param helper helper of txsim
 */
void UltrasonicTruth::Init(tx_sim::InitHelper &helper) {
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Subscribe(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation);
  helper.Subscribe(tx_sim::topic::kLocation);
}

/**
 * @brief ultrasonic truth step
 *
 * @param helper helper of txsim
 */
void UltrasonicTruth::Step(tx_sim::StepHelper &helper) {
  if (ultrasonics.empty() || !osiMaker) {
    return;
  }

  // 2. get messages we subscribed.
  // parse traffic message and location message into protobuf object
  sim_msg::Location loc84 = getLocation(helper);
  sim_msg::Union ulocation = getUnion(helper);
  sim_msg::Traffic traffic = getTraffic(helper);
  // to enu
  Eigen::Vector3d loc_dke(loc84.position().x(), loc84.position().y(), loc84.position().z());
  coord_trans_api::lonlat2enu(loc_dke.x(), loc_dke.y(), loc_dke.z(), map_ori.x, map_ori.y, map_ori.z);

  std::cout << "[UltrasonicTruth(" << helper.timestamp() << ")]: ";
  // 3. calculate distance
  for (auto &ult : ultrasonics) {
    auto sensor_feature = osiMaker->AddUltrasonic();
    sensor_feature->mutable_header()->mutable_sensor_id()->set_value(ult.first);
    // Eigen::Vector3d slot_p, slot_a;
    // ult.second->getPosition(slot_p);
    // ult.second->getRotation(slot_a);
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_position()->set_x(slot_p.x());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_position()->set_y(slot_p.y());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_position()->set_z(slot_p.z());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_roll(slot_a.x());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_pitch(slot_a.y());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_yaw(slot_a.z());

    // set position
    ult.second->setCarPosition(loc_dke);
    // set roatation
    ult.second->setCarRoatation(loc84.rpy().x(), loc84.rpy().y(), loc84.rpy().z());
    double mindis = ult.second->getRange() + 100;
    int type = 0;
    const void *rawPtr = handle_traffic(*ult.second, ulocation, traffic, loc_dke, type, mindis);

    // get closest object
    if (mindis < ult.second->getRange()) {
      int64_t id = 0;
      // handle the ego
      if (type < 0) {
        const sim_msg::Location *car = static_cast<const sim_msg::Location *>(rawPtr);
        id = -type;
        osiMaker->Add(*car, id);
      } else if (type == 0) {
        // handle car
        const sim_msg::Car *car = static_cast<const sim_msg::Car *>(rawPtr);
        osiMaker->Add(*car);
        id = getUUID(*car);
      } else if (type == 1) {
        // handle dynamic obstacle
        const sim_msg::DynamicObstacle *obs = static_cast<const sim_msg::DynamicObstacle *>(rawPtr);
        osiMaker->Add(*obs);
        id = getUUID(*obs);
      } else if (type == 2) {
        // handle static obstacle
        const sim_msg::StaticObstacle *sta = static_cast<const sim_msg::StaticObstacle *>(rawPtr);
        osiMaker->Add(*sta);
        id = getUUID(*sta);
      } else {
        continue;
      }
      // set detection
      auto detection = sensor_feature->add_detection();
      detection->mutable_object_id()->set_value(id);
      detection->set_distance(mindis);

      std::cout << "ultrasonic[" << ult.first << "] with " << type << "- " << mindis << ", ";
    }
  }
  std::cout << "\n";
}

const void *UltrasonicTruth::handle_traffic(const UltrasonicSensor &ult, const sim_msg::Union &ulocation,
                                            const sim_msg::Traffic &traffic, const Eigen::Vector3d &locPosition,
                                            int &type, double &mindis) {
  const void *rawPtr = 0;
  // handle other ego
  for (const auto &msg : ulocation.messages()) {
    std::int64_t eid = std::atoi(msg.groupname().substr(msg.groupname().length() - 3).c_str());
    if (eid == ego_id) continue;
    sim_msg::Location egolocation;
    if (egolocation.ParseFromString(msg.content())) {
      auto &ego_bbx = ego_Bboxes[eid];
      if (ego_bbx.category < 0) {
        std::cout << "Cannot find ego type: " << eid << std::endl;
        continue;
      }
      Eigen::Vector3d pos_ee(egolocation.position().x(), egolocation.position().y(), egolocation.position().z());
      coord_trans_api::lonlat2enu(pos_ee.x(), pos_ee.y(), pos_ee.z(), map_ori.x, map_ori.y, map_ori.z);
      double d = ult.distance(pos_ee, 0, 0, egolocation.rpy().z(), ego_bbx.len, ego_bbx.wid, ego_bbx.hei);
      // get the closest car
      if (d > 0.01 && d < mindis) {
        mindis = d;
        type = -eid;
        rawPtr = static_cast<const void *>(&egolocation);
      }
    }
  }
  // handle car
  for (const auto &car : traffic.cars()) {
    Eigen::Vector3d pos(car.x(), car.y(), car.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    double d = ult.distance(pos, 0, 0, car.heading(), car.length(), car.width(), car.height());
    // get the closest car
    if (d > 0.01 && d < mindis) {
      mindis = d;
      type = 0;
      rawPtr = static_cast<const void *>(&car);
    }
  }
  // handle dynamic obstacle
  for (const auto &dynobj : traffic.dynamicobstacles()) {
    Eigen::Vector3d pos_enu(dynobj.x(), dynobj.y(), dynobj.z());
    coord_trans_api::lonlat2enu(pos_enu.x(), pos_enu.y(), pos_enu.z(), map_ori.x, map_ori.y, map_ori.z);
    double d = ult.distance(pos_enu, 0, 0, dynobj.heading(), dynobj.length(), dynobj.width(), dynobj.height());
    // get the cloest dynamic obstacle
    if (d > 0.01 && d < mindis) {
      mindis = d;
      type = 1;
      rawPtr = static_cast<const void *>(&dynobj);
    }
  }
  // handle static obstacle
  for (const auto &obs : traffic.staticobstacles()) {
    if (obs.height() < 0.01 && obs.width() < 0.01 && obs.length() < 0.01) {
      continue;
    }
    Eigen::Vector3d pos(obs.x(), obs.y(), obs.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    double d = ult.distance(pos, 0, 0, obs.heading(), obs.length(), obs.width(), obs.height());
    // get the cloest static obstacle
    if (d > 0.01 && d < mindis) {
      mindis = d;
      type = 2;
      rawPtr = static_cast<const void *>(&obs);
    }
  }
  return rawPtr;
}
