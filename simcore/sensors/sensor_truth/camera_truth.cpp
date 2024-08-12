/**
 * @file CameraTruth.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "camera_truth.h"
#include <google/protobuf/util/json_util.h>
#include <ctime>
#include <iostream>
#include "camera_sensor.h"
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
 * @brief cameratruth init function
 *
 * @param helper helper of txsim
 */
void CameraTruth::Init(tx_sim::InitHelper &helper) {
  // get user defined initiation parameters.
  // if we defined the parameters in TADSim UI, override the default values
  // here.

  // get max distance
  std::string value = helper.GetParameter("CT_MaxDistance");
  if (!value.empty()) {
    maxDist = atof(value.c_str());
  }
  // get completeness
  value = helper.GetParameter("CT_Completeness");
  if (!value.empty()) {
    completeness = atof(value.c_str());
  }
  // get min area
  value = helper.GetParameter("CT_MinArea");
  if (!value.empty()) {
    minArea = std::max(1.0, atof(value.c_str()));
  }
  std::cout << "CT_MaxDistance = " << maxDist << std::endl;
  std::cout << "CT_Completeness = " << completeness << std::endl;
  std::cout << "CT_MinArea = " << minArea << std::endl;

  // by subscribe our interested topics, we expect that the two corresponding
  // messages which defined by traffic.proto and location.proto would received
  // in every Step callback.
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Subscribe(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation);
}

/**
 * @brief cameratruth step function
 *
 * @param helper helper of txsim
 */
void CameraTruth::Step(tx_sim::StepHelper &helper) {
  if (!osiMaker) return;
  if (cameras.empty()) return;

  // 2. get messages we subscribed.
  sim_msg::Location location = getLocation(helper);
  // Convert coordinates from lon-lat to ENU
  Eigen::Vector3d locEnu(location.position().x(), location.position().y(), location.position().z());
  coord_trans_api::lonlat2enu(locEnu.x(), locEnu.y(), locEnu.z(), map_ori.x, map_ori.y, map_ori.z);

  std::cout << "[CameraTruth(" << helper.timestamp() << ")]: ";
  sim_msg::Union union_location = getUnion(helper);
  // get traffic
  sim_msg::Traffic traffic = getTraffic(helper);
  for (auto cam : cameras) {
    // do every camera 'sruth calculation
    // Add a new camera feature into OSIMaker
    auto sensor_feature = osiMaker->AddCamera();
    // Set header information about the camera
    sensor_feature->mutable_header()->mutable_sensor_id()->set_value(cam.first);
    // Eigen::Vector3d slot_p, slot_a;
    // cam.second->getPosition(slot_p);
    // cam.second->getRotation(slot_a);
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_position()->set_x(slot_p.x());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_position()->set_y(slot_p.y());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_position()->set_z(slot_p.z());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_roll(slot_a.x());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_pitch(slot_a.y());
    // sensor_feature->mutable_header()->mutable_mounting_position()->mutable_orientation()->set_yaw(slot_a.z());

    // set the car position and rotation
    cam.second->setCarPosition(locEnu);
    cam.second->setCarRoatation(location.rpy().x(), location.rpy().y(), location.rpy().z());

    // setup visable buffers
    std::vector<VisableBuffer> vbs;
    handle_ego(*cam.second, union_location, locEnu, vbs);
    handle_traffic(*cam.second, traffic, locEnu, vbs);

    // calculate visible
    ReVisable(vbs);
    int tagcount[5] = {0};
    for (const auto &vb : vbs) {
      if (minArea > 0 && completeness > 0) {
        bool visable = (vb.area >= minArea) && (vb.area0 > 0 && (vb.area / vb.area0) > completeness);
        if (!visable) {
          continue;
        }
      }
      int64_t id;
      if (vb.tag < 0) {
        tagcount[0]++;
        // handle the ego
        id = -vb.tag;
        osiMaker->Add(*static_cast<const sim_msg::Location *>(vb.rawPtr), id);
      } else if (vb.tag == 2) {
        tagcount[3]++;
        // handle the static obstacle
        const sim_msg::StaticObstacle &sstatic = *static_cast<const sim_msg::StaticObstacle *>(vb.rawPtr);
        id = getUUID(sstatic);
        osiMaker->Add(sstatic);
      } else if (vb.tag == 1) {
        tagcount[2]++;
        // handle the dynamic obstacle
        const sim_msg::DynamicObstacle &obs = *static_cast<const sim_msg::DynamicObstacle *>(vb.rawPtr);
        osiMaker->Add(obs);
        id = getUUID(obs);
      } else if (vb.tag == 0) {
        tagcount[1]++;
        // handle the car
        const sim_msg::Car &car = *static_cast<const sim_msg::Car *>(vb.rawPtr);
        osiMaker->Add(car);
        id = getUUID(car);
      } else {
        continue;
      }
      // succeed to detect
      auto detection = sensor_feature->add_detection();
      detection->mutable_object_id()->set_value(id);
      detection->set_image_shape_type(osi3::CameraDetection_ImageShapeType_IMAGE_SHAPE_TYPE_POLYGON);
      detection->set_first_point_index(sensor_feature->point_size());
      // calculate the polygon hull
      polygon ply;
      boost::geometry::convex_hull(vb.bounds, ply);
      for (const auto &p : ply.outer()) {
        int x = static_cast<int>(p.get<0>());
        int y = static_cast<int>(p.get<1>());
        auto fp = sensor_feature->add_point();
        fp->set_existence_probability(1);
        fp->mutable_point()->set_azimuth(x);
        fp->mutable_point()->set_elevation(y);
      }
      detection->set_number_of_points(ply.outer().size());
    }

    // handle the traffic lights
    for (const auto &light : traffic.trafficlights()) {
      Eigen::Vector3d pos(light.x(), light.y(), light.z());
      coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
      if (!cam.second->inFov(pos, maxDist)) {
        continue;
      }
      osiMaker->Add(light);
      tagcount[4]++;
    }

    std::cout << "camera[" << cam.first << "] with " << tagcount[0] << " ego, " << tagcount[1] << " car, "
              << tagcount[2] << " dyn_objects, " << tagcount[3] << " stationary_objects. " << tagcount[4]
              << " trafficlights.";
  }
  std::cout << "\n";
}

/**
 * @brief stop
 *
 * @param helper helper of txsim
 */
void CameraTruth::Stop(tx_sim::StopHelper &helper) {}

// calculate the box points of the object
points CameraTruth::VbufferInit(const ObjectBBox &bbox, double heading, const Eigen::Vector3d &pos,
                                const CameraSensor &cam) {
  points uvs;
  const double sample_box = 1.0;
  for (const auto &p : bbox.getBboxPts(pos, heading, sample_box, 0.9)) {
    auto uv = cam.world2pixel(p);
    if (uv.x() > 0 && uv.x() < cam.W() && uv.y() > 0 && uv.y() < cam.H()) {
      uvs.push_back(point(uv.x(), uv.y()));
    }
  }
  return uvs;
}

/// @brief
/// @param cam
/// @param union_location
/// @param loc0
/// @param vbs
void CameraTruth::handle_ego(const CameraSensor &cam, const sim_msg::Union &union_location, const Eigen::Vector3d &loc0,
                             std::vector<VisableBuffer> &vbs) {
  // handle other ego
  for (const auto &msg : union_location.messages()) {
    std::int64_t eid = std::atoi(msg.groupname().substr(msg.groupname().length() - 3).c_str());
    if (eid == ego_id) continue;
    sim_msg::Location location;
    if (location.ParseFromString(msg.content())) {
      Eigen::Vector3d posEnu(location.position().x(), location.position().y(), location.position().z());
      coord_trans_api::lonlat2enu(posEnu.x(), posEnu.y(), posEnu.z(), map_ori.x, map_ori.y, map_ori.z);
      double distance = (posEnu - loc0).norm();
      // out of range
      if (distance > maxDist) {
        continue;
      }
      auto &objbb = ego_Bboxes[eid];
      if (objbb.category < 0) {
        std::cout << "Cannot find ego type: " << eid << std::endl;
      }
      VisableBuffer vb;
      // init the buffer
      if (!vb.init(VbufferInit(objbb, location.rpy().z(), posEnu, cam))) {
        continue;
      }
      vb.tag = -eid;
      vb.rawPtr = static_cast<void *>(&location);
      vb.distance = distance;
      vbs.push_back(vb);
    }
  }
}

void CameraTruth::handle_traffic(const CameraSensor &cam, const sim_msg::Traffic &traffic,
                                 const Eigen::Vector3d &locPosition, std::vector<VisableBuffer> &vbs) {
  // handle the cars
  for (const auto &car : traffic.cars()) {
    Eigen::Vector3d pos(car.x(), car.y(), car.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);

    double distance = (pos - locPosition).norm();
    // out of range
    if (distance > maxDist) {
      continue;
    }
    // update the size
    auto &objbb = car_Bboxes[car.type()];
    if (objbb.category < 0) {
      std::cout << "Cannot find car type " << car.type() << std::endl;
      objbb.len = car.length();
      objbb.wid = car.width();
      objbb.hei = car.height();
    }
    VisableBuffer vb;
    // init the buffer
    if (!vb.init(VbufferInit(objbb, car.heading(), pos, cam))) {
      continue;
    }
    vb.tag = 0;
    vb.rawPtr = static_cast<const void *>(&car);
    vb.distance = distance;
    vbs.push_back(vb);
  }
  // handle the dynamic obstacles
  for (const auto &obs : traffic.dynamicobstacles()) {
    Eigen::Vector3d pos(obs.x(), obs.y(), obs.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    double distance = (pos - locPosition).norm();
    // out of range
    if (distance > maxDist) {
      continue;
    }
    auto objbb = dyn_Bboxes[obs.type()];
    if (objbb.category < 0) {
      objbb = car_Bboxes[obs.type()];
    }
    if (objbb.category < 0) {
      std::cout << "Cannot find dynamic obstracle type " << obs.type() << std::endl;
      objbb.len = obs.length();
      objbb.wid = obs.width();
      objbb.hei = obs.height();
    }
    VisableBuffer vb;
    // init the buffer
    if (!vb.init(VbufferInit(objbb, obs.heading(), pos, cam))) {
      continue;
    }
    vb.tag = 1;
    vb.rawPtr = static_cast<const void *>(&obs);
    vb.distance = distance;
    vbs.push_back(vb);
  }
  // handle the static obstacles
  for (const auto &sta : traffic.staticobstacles()) {
    if (sta.height() < 0.01 && sta.width() < 0.01 && sta.length() < 0.01) {
      continue;
    }
    Eigen::Vector3d pos(sta.x(), sta.y(), sta.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    double distance = (pos - locPosition).norm();
    // out of range
    if (distance > maxDist) {
      continue;
    }
    auto &objbb = sta_Bboxes[sta.type()];
    if (objbb.category < 0) {
      std::cout << "Cannot find static obstracle type " << sta.type() << std::endl;
      objbb.len = sta.length();
      objbb.wid = sta.width();
      objbb.hei = sta.height();
    }
    VisableBuffer vb;
    // init the buffer
    if (!vb.init(VbufferInit(objbb, sta.heading(), pos, cam))) {
      continue;
    }
    vb.tag = 2;
    vb.rawPtr = static_cast<const void *>(&sta);
    vb.distance = distance;
    vbs.push_back(vb);
  }
}
