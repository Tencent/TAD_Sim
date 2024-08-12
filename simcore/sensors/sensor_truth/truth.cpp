/**
 * @file Truth.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "truth.h"
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include "fov_filter_sensor.h"
#include "hmap.h"
#include "object_size.h"
#include "location.pb.h"
#include "scene.pb.h"

#ifdef USE_SENSOR_TRUTH
#  include "camera_sensor.h"
#  include "camera_truth.h"
#  include "free_space.h"
#  include "lane_truth.h"
#  include "lidar_sensor.h"
#  include "lidar_truth.h"
#  include "osi_util.h"
#  include "parking_space.h"
#  include "sensor_truth.h"
#  include "ultrasonic_sensor.h"
#  include "ultrasonic_truth.h"
#else
#  ifdef USE_TRAFFIC_FILTER
#  endif
#  include "traffic_filter.h"
#endif
#include "settings.pb.h"

// since the example would process the two types of message, we need include the
// two corresponding protobuf headers here. if you need process other type of
// protobuf message that TADSim defines, include them too.

#ifdef _WIN32
#  define MODULE_API __declspec(dllexport)
#else
#  define MODULE_API
#endif  // _WIN32

/**
 * @brief Construct a new Truth:: Truth object
 *
 */
Truth::Truth() {}

/**
 * @brief Destroy the Truth:: Truth object
 *
 */
Truth::~Truth() { std::cout << "Truth destroyed." << std::endl; }

/**
 * @brief truth init
 *
 * @param helper helper of txsim
 */
void Truth::Init(tx_sim::InitHelper &helper) {
  std::string debugDir = helper.GetParameter("DebugDir");
  device = helper.GetParameter("-device");
  if (device == "all") {
    device.clear();
  }
#ifdef USE_SENSOR_TRUTH
  // CameraTruth=ON
  if (helper.GetParameter("CameraTruth") == "ON") sensors.push_back(std::make_shared<CameraTruth>());
  // LidarTruth=ON
  if (helper.GetParameter("LidarTruth") == "ON") sensors.push_back(std::make_shared<LidarTruth>());
  // UltrasonicTruth=ON
  if (helper.GetParameter("UltrasonicTruth") == "ON") sensors.push_back(std::make_shared<UltrasonicTruth>());

  if (!sensors.empty()) {
    auto osiMaker = std::make_shared<OsiSensorData>();
    for (auto &sens : sensors) {
      // set osi pointer
      dynamic_cast<OsiSensorData *>(sens.get())->osiMaker = osiMaker.get();
    }
    sensors.push_back(osiMaker);
  }

  // SensorTruth=ON
  if (helper.GetParameter("SensorTruth") == "ON") sensors.push_back(std::make_shared<SensorTruth>());
  // ParkingSpace=ON
  if (helper.GetParameter("ParkingSpace") == "ON") sensors.push_back(std::make_shared<ParkingSpace>());
  if (helper.GetParameter("FreeSpace") == "ON") sensors.push_back(std::make_shared<FreeSpace>());
  if (helper.GetParameter("LaneTruth") == "ON") sensors.push_back(std::make_shared<LaneTruth>());
#else
#  ifdef USE_TRAFFIC_FILTER
  sensors.push_back(std::make_shared<TrafficFilter>());
#  endif
#endif

  for (auto &sens : sensors) {
    sens->ego_id = ego_id;
    sens->device = device;
    sens->debugDir = debugDir;
    sens->Init(helper);
  }

  auto config_dir = helper.GetParameter(tx_sim::constant::kInitKeyModuleSharedLibDirectory);
  // auto config_dir = helper.GetParameter("_cwd");
  // loadObjectSize(config_dir);
}

/**
 * @brief truth reset
 *
 * @param helper helper of txsim
 */
void Truth::Reset(tx_sim::ResetHelper &helper) {
  // load bbox
  LoadObjectBBox(helper.scene_pb());

  // load sensor
  getSensorConfig(helper.scene_pb(), helper.group_name());
#ifdef USE_SENSOR_TRUTH
  std::cout << "Load " << cameras.size() << " cameras." << std::endl;
  std::cout << "Load " << lidars.size() << " lidars." << std::endl;
  std::cout << "Load " << ultrasonics.size() << " ultrasonics." << std::endl;
#endif
  std::cout << "Load " << fovfilters.size() << " fovs." << std::endl;
  // load map
  LoadMap(helper.map_file_path());

  // if (fovfilters.empty())
  // {
  // std::cout << "Create default Fov: hfov = 120, vfov = 60, distance =
  // 100m\n";  AddDefaultFov(0, 60, 120, 100);
  //}

  for (auto &sens : sensors) {
    sens->ego_id = ego_id;
    sens->map_ori = helper.map_local_origin();
    sens->Reset(helper);
  }
}

/**
 * @brief truth step
 *
 * @param helper helper of txsim
 */
void Truth::Step(tx_sim::StepHelper &helper) {
  for (auto &sens : sensors) {
    sens->Step(helper);
  }
}

/**
 * @brief truth stop
 *
 * @param helper helper of txsim
 */
void Truth::Stop(tx_sim::StopHelper &helper) {
  for (auto &sens : sensors) {
    sens->Stop(helper);
  }
}

/**
 * @brief get sensor config
 *
 * @param buffer scene buffer
 * @return true on success
 * @return false on failure
 */
bool Truth::getSensorConfig(const std::string &buffer, const std::string &groupname) {
  if (buffer.empty()) {
    return false;
  }

  ego_id = std::atoi(groupname.substr(groupname.length() - 3).c_str());
  // parse the protobuf message into a Scene instance
  sim_msg::Scene scene;
  if (!scene.ParseFromString(buffer)) {
    std::cout << "parse scene faild.";
    return false;
  }
  // read sensor config

  fovfilters.clear();
#ifdef USE_SENSOR_TRUTH
  cameras.clear();
  lidars.clear();
  ultrasonics.clear();
#endif
  for (const auto &ego : scene.egos()) {
    int64_t eid = std::atoi(ego.group().substr(ego.group().length() - 3).c_str());
    if (eid != ego_id) continue;
    for (const auto &sensor : ego.sensor_group().sensors()) {
      std::cout << sensor.DebugString();
      if (sensor.type() == sim_msg::SENSOR_TYPE_TRUTH) {
        // handle truth
        LoadFovFilter(sensor, device);
      } else if (sensor.type() == sim_msg::SENSOR_TYPE_CAMERA) {
        // handle camera
#ifdef USE_SENSOR_TRUTH
        LoadCamera(sensor, device);
#endif
      } else if (sensor.type() == sim_msg::SENSOR_TYPE_TRADITIONAL_LIDAR) {
        // handle lidar
#ifdef USE_SENSOR_TRUTH
        LoadLidar(sensor, device);
#endif
      } else if (sensor.type() == sim_msg::SENSOR_TYPE_ULTRASONIC) {
        // handle ultrasonic
#ifdef USE_SENSOR_TRUTH
        LoadUltrasonic(sensor, device);
#endif
      }
    }
    break;
  }
  return true;
}

TXSIM_MODULE(Truth)
