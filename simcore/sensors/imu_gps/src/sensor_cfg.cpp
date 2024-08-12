/**
 * @file sensor_cfg.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "sensor_cfg.h"
#include "chrono/core/ChQuaternion.h"
#include "pugixml.hpp"
#include "scene.pb.h"
#include "sensor_core.h"

namespace sensor {
CSensorCfg::CSensorCfg() {}

// split string
void CSensorCfg::stringSplit(const std::string strIn, const std::string& spliter,
                             std::vector<std::string>& splitedStrs) {
  splitedStrs.clear();

  std::string::size_type pos1 = 0, pos2 = strIn.find(spliter);

  while (std::string::npos != pos2) {
    splitedStrs.push_back(strIn.substr(pos1, pos2 - pos1));

    pos1 = pos2 + spliter.size();
    pos2 = strIn.find(spliter, pos1);
  }

  if (pos1 != strIn.length()) splitedStrs.push_back(strIn.substr(pos1));
}

void splitString(const std::string& content, std::vector<std::string>& splitted, const std::string& spliter) {
  splitted.reserve(3);
  size_t p1 = 0, p2 = 0;

  do {
    p2 = content.find(spliter, p1);
    if (p2 != content.npos) {
      if (p2 > p1) splitted.push_back(content.substr(p1, p2 - p1));
      p1 = p2 + spliter.size();
    }
  } while (p2 != content.npos);

  if (p1 < content.size()) {
    splitted.push_back(content.substr(p1));
  }
}

// parse imu config from json dict
bool CSensorCfg::parseIMUConfig(const std::string& buffer, const std::string& gname, osi3::imu_IMUCfg& imuCfg) {
  sim_msg::Scene scene_;
  if (!scene_.ParseFromString(buffer)) {
    std::cout << "parse scene faild.";
    return false;
  }
  int ego_id = std::atoi(gname.substr(gname.length() - 3).c_str());
  for (const auto& ego : scene_.egos()) {
    int64_t eiid = std::atoi(ego.group().substr(ego.group().length() - 3).c_str());
    if (eiid != ego_id) continue;
    for (const auto& sensor : ego.sensor_group().sensors()) {
      if (sensor.type() == sim_msg::SENSOR_TYPE_IMU) {
        // get position and orientation ptr of imu sensor
        auto orientPtr = imuCfg.mutable_orientation();
        auto eulerPtr = imuCfg.mutable_euler();

        auto posPtr = imuCfg.mutable_position();
        posPtr->set_x(sensor.extrinsic().locationx() * 0.01);
        posPtr->set_y(sensor.extrinsic().locationy() * 0.01);
        posPtr->set_z(sensor.extrinsic().locationz() * 0.01);

        LOG(INFO) << "imu | position, x:" << imuCfg.position().x() << ", y:" << imuCfg.position().y()
                  << ", z:" << imuCfg.position().z() << "\n";

        auto& params = sensor.intrinsic().params();
        if (params.find("Quaternion") != params.end()) {
          std::string quatStr = params.find("Quaternion")->second;
          std::vector<std::string> quatList;

          stringSplit(quatStr, ",", quatList);
          if (quatList.size() != 4) {
            LOG(ERROR) << "wrong quaternion format, " << quatStr << "\n";
            return false;
          }

          orientPtr->set_qx(std::stod(quatList[0]));
          orientPtr->set_qy(std::stod(quatList[1]));
          orientPtr->set_qz(std::stod(quatList[2]));
          orientPtr->set_qw(std::stod(quatList[3]));

          LOG(INFO) << "imu | imu orentation,w,x,y,z:" << imuCfg.orientation().qw() << ", " << imuCfg.orientation().qx()
                    << ", " << imuCfg.orientation().qy() << ", " << imuCfg.orientation().qz() << "\n";

          // get euler
          chrono::ChQuaternion<double> imuQ(imuCfg.orientation().qw(), imuCfg.orientation().qx(),
                                            imuCfg.orientation().qy(), imuCfg.orientation().qz());
          chrono::ChVector<double> rpy = imuQ.Q_to_Euler123();
          eulerPtr->set_roll(rpy[0]);
          eulerPtr->set_pitch(rpy[1]);
          eulerPtr->set_yaw(rpy[2]);

          LOG(INFO) << "imu | imu orentation, roll, pitch, yaw[rad]:" << eulerPtr->roll() << ", " << eulerPtr->pitch()
                    << ", yaw:" << eulerPtr->yaw() << "\n";
        }
      }
    }

    break;
  }

  return true;
}

// parse gps config from json dict
bool CSensorCfg::parseGPSConfig(const std::string& buffer, const std::string& groupname, osi3::GPS& gps) {
  sim_msg::Scene scene;
  if (!scene.ParseFromString(buffer)) {
    std::cout << "parse scene faild.";
    return false;
  }
  int ego_id = std::atoi(groupname.substr(groupname.length() - 3).c_str());
  for (const auto& ego : scene.egos()) {
    int64_t eid = std::atoi(ego.group().substr(ego.group().length() - 3).c_str());
    if (eid != ego_id) continue;
    for (const auto& sensor : ego.sensor_group().sensors()) {
      if (sensor.type() == sim_msg::SENSOR_TYPE_GPS) {
        // get position and orientation ptr of imu sensor
        auto posPtr = gps.mutable_mount_pos()->mutable_position();
        posPtr->set_x(sensor.extrinsic().locationx() * 0.01);
        posPtr->set_y(sensor.extrinsic().locationy() * 0.01);
        posPtr->set_z(sensor.extrinsic().locationz() * 0.01);

        LOG(INFO) << "gps | relative position, x:" << gps.mount_pos().position().x()
                  << ", y:" << gps.mount_pos().position().y() << ", z:" << gps.mount_pos().position().z() << "\n";
      }
    }
    break;
  }
  return true;
}

}  // namespace sensor
