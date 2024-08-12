/**
 * @file OsiUtil.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <functional>
#include <map>
#include "age.h"
#include "fov_filter_sensor.h"
#include "noise.h"
#include "truth_base.h"
#include "osi_datarecording.pb.h"
#include "traffic.pb.h"

#define DYN_ID0 100000
#define STA_ID0 200000
#define EGO_ID0 300000

class OsiSensorData : public TruthBase {
 public:
  void Init(tx_sim::InitHelper &helper);
  void Reset(tx_sim::ResetHelper &helper);
  void Step(tx_sim::StepHelper &helper);

  void Add(const sim_msg::Car &car);
  void Add(const sim_msg::Location &car, std::int64_t id);
  void Add(const sim_msg::DynamicObstacle &dob);
  void Add(const sim_msg::StaticObstacle &sob);
  void Add(const sim_msg::TrafficLight &lig);
  osi3::LidarDetectionData *AddLidar();
  osi3::CameraDetectionData *AddCamera();
  osi3::UltrasonicDetectionData *AddUltrasonic();

 protected:
  OsiSensorData *osiMaker = 0;

 private:
  osi3::SensorData sensordata;

  FovFilterSensor fov{0};
  friend class Truth;

  std::map<int64_t, sim_msg::Location> egoBuf;
  std::map<int64_t, sim_msg::Car> carBuf;
  std::map<int64_t, sim_msg::DynamicObstacle> dynBuf;
  std::map<int64_t, sim_msg::StaticObstacle> staBuf;
  std::map<int64_t, sim_msg::TrafficLight> ligBuf;

  Age ages[4];

  Noise noise;
};

int64_t getUUID(std::int64_t ego_id);
int64_t getUUID(const sim_msg::Car &obj);
int64_t getUUID(const sim_msg::DynamicObstacle &obj);
int64_t getUUID(const sim_msg::StaticObstacle &obj);

bool Txsim2OsiDynamicObject(
    const sim_msg::Location &car, std::int64_t id, osi3::DetectedMovingObject *mov_object, ValueBlock age,
    std::function<void(double &, double &, double &)> ts_pt = [](double &, double &, double &) {},
    std::function<void(double &, double &, double &)> ts_rpy = [](double &, double &, double &) {});
bool Txsim2OsiDynamicObject(
    const sim_msg::Car &car, osi3::DetectedMovingObject *mov_object, ValueBlock age,
    std::function<void(double &, double &, double &)> ts_pt = [](double &, double &, double &) {},
    std::function<void(double &, double &, double &)> ts_rpy = [](double &, double &, double &) {});
bool Txsim2OsiDynamicObject(
    const sim_msg::DynamicObstacle &dob, osi3::DetectedMovingObject *mov_object, ValueBlock age,
    std::function<void(double &, double &, double &)> ts_pt = [](double &, double &, double &) {},
    std::function<void(double &, double &, double &)> ts_rpy = [](double &, double &, double &) {});
bool Txsim2OsiStationaryObject(
    const sim_msg::StaticObstacle &sob, osi3::DetectedStationaryObject *sta_object,
    std::function<void(double &, double &, double &)> ts_pt = [](double &, double &, double &) {},
    std::function<void(double &, double &, double &)> ts_rpy = [](double &, double &, double &) {});
bool Txsim2OsiTrafficLightObject(const sim_msg::TrafficLight &lig, osi3::DetectedTrafficLight *lig_object,
                                 std::function<void(double &, double &, double &)> ts_pt,
                                 std::function<void(double &, double &, double &)> ts_rpy);

bool MovingNoise(osi3::DetectedMovingObject *mov_object, double tstamp, Noise &noise);
bool StationaryNoise(osi3::DetectedStationaryObject *sta_object, double tstamp, Noise &noise);
