// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <Eigen/Core>
#include <map>
#include <memory>
#include <vector>
#include "../sensor_objects_def.h"
#include "camera_sensor.h"
#include "lidar_sensor.h"
#include "radar_sensor.h"
#include "ini.h"
#include "traffic.pb.h"
#include "v2x_asn1_2020.pb.h"

struct RsuConfig {
  int FrequencySPAT = 10;
  int FrequencyRSM = 10;
  int FrequencyRSI = 10;
  int FrequencyMAP = 10;
  double DistanceCommu = 300;
  int Band = 2;  // 5905-5915，5915-5925，5905-5925
  double Mbps = 100;
  double MaxDelay = 100;
  double MapRadius = 1000;
  std::vector<std::int64_t> JunctionIDs;
  bool TriggerImmediately = true;
  bool CongestionRegulation = true;
  bool V2N = false;
  bool V2I = false;
  std::string PreRsu;
};

class RsuSensor {
 public:
  RsuSensor();
  virtual ~RsuSensor();

  RsuConfig config;
  std::map<std::int64_t, std::shared_ptr<CameraSensor>> cameras;
  std::map<std::int64_t, std::shared_ptr<LidarSensor>> lidars;
  std::map<std::int64_t, std::shared_ptr<RadarSensor>> radars;

  Eigen::Vector3d map_ori;
  Eigen::Vector3d position;
  virtual bool Init();

  std::vector<sim_msg::Car> FilterRsuCars(const sim_msg::Traffic &traffic) const;
  std::vector<sim_msg::DynamicObstacle> FilterRsuDynamicObjects(const sim_msg::Traffic &traffic) const;
  std::vector<hadmap::objectpkid> FilterRsuEvents() const;
  std::vector<hadmap::junctionpkid> FilterRsuJuncs() const;

 private:
  std::vector<hadmap::junctionpkid> juncNearby;
  /// @brief 用camera、Lidar，radar过滤车辆
  /// @param traffic
  /// @return
  std::vector<sim_msg::Car> FilterCar(const sim_msg::Traffic &traffic) const;
  std::vector<sim_msg::DynamicObstacle> FilterObstacle(const sim_msg::Traffic &traffic) const;
};
