#include "sensor.h"
#include <stdio.h>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include "sensor_raw.pb.h"

sim_sensor::sim_sensor() : threads(16) {}

sim_sensor::~sim_sensor() {}

void sim_sensor::Init(tx_sim::InitHelper& helper) {
  auto NumOfDisplay = helper.GetParameter("NumOfDisplay");
  if (!NumOfDisplay.empty()) {
    disNum = std::atoi(NumOfDisplay.c_str());
  }
  savePath = helper.GetParameter("SavePath");
  for (int i = 0; i < disNum; i++) {
    helper.Subscribe(std::string("DISPLAYSENSOR_") + std::to_string(i));
  }
  if (!boost::filesystem::exists(savePath)) {
    boost::filesystem::create_directory(savePath);
  }
}

void sim_sensor::Reset(tx_sim::ResetHelper& helper) { std::cout << "reset\n"; }

void saveJpg(const std::string fname, const std::string buf) {
  sim_msg::CameraRaw camera;
  if (!camera.ParseFromString(buf)) {
    std::cout << "camera error";
  }
  if (camera.type() == "JPEG") {
    std::fstream of(fname, std::ios::out | std::ios::binary);
    of.write(camera.image_data().c_str(), camera.image_data().size());
    of.close();
  }
}

bool savePcd(const std::string fname, const std::string buf) {
  sim_msg::LidarRaw lidar;
  if (!lidar.ParseFromString(buf)) std::cout << "lidar error";

  size_t num = lidar.count();

  std::ofstream of(fname);
  if (!of.good()) {
    return false;
  }
  of << "# .PCD v.7 - Point Cloud Data file format\n";
  of << "VERSION 0.7\n";
  of << "FIELDS x y z intensity label\n";
  of << "SIZE 4 4 4 4 4\n";
  of << "TYPE F F F F I\n";
  of << "COUNT 1 1 1 1 1\n";
  of << "WIDTH " << num << "\n";
  of << "HEIGHT 1\n";
  of << "VIEWPOINT 0 0 0 1 -1 0 0\n";
  of << "POINTS " << num << "\n";
  of << "DATA binary\n";

  if (lidar.point_lists().size() > 0) {
    of.write((char*)lidar.point_lists().data(), lidar.point_lists().size());
  } else {
#pragma pack(push, 4)
    struct TMP {
      float x = 0;
      float y = 0;
      float z = 0;
      float i = 0;          // intensity
      std::uint32_t t = 0;  // type
    };
#pragma pack(pop)

    std::vector<TMP> datas;
    datas.reserve(num);
    for (const auto& p : lidar.points()) {
      TMP tmp;
      tmp.x = p.x();
      tmp.y = p.y();
      tmp.z = p.z();
      tmp.i = p.i();
      tmp.t = p.t();
      datas.push_back(tmp);
    }
    of.write((char*)datas.data(), 5 * num * sizeof(float));
  }

  of.close();
  return true;
}

void sim_sensor::Step(tx_sim::StepHelper& helper) {
  std::cout << helper.timestamp() << ": ";

  for (int i = 0; i < disNum; i++) {
    std::string payload_;
    helper.GetSubscribedMessage(std::string("DISPLAYSENSOR_") + std::to_string(i), payload_);
    sim_msg::SensorRaw sensorraw;
    if (payload_.empty() || !sensorraw.ParseFromString(payload_)) continue;

    std::string baseName = savePath + "/" + std::to_string(helper.timestamp());
    std::cout << "[" << i << ": ";
    for (const auto& sensor : sensorraw.sensor()) {
      std::cout << "(";

      if (sensor.type() == sim_msg::SensorRaw::TYPE_CAMERA) {
        threads.enqueue(saveJpg, baseName + "_camera_" + std::to_string(sensor.id()) + ".jpg", sensor.raw());
        std::cout << "camera=" << sensor.raw().size();
      } else if (sensor.type() == sim_msg::SensorRaw::TYPE_LIDAR) {
        threads.enqueue(savePcd, baseName + "_lidar_" + std::to_string(sensor.id()) + ".pcd", sensor.raw());
        std::cout << "lidar=" << sensor.raw().size();
      }

      std::cout << "), ";
    }
    std::cout << "], ";
  }

  std::cout << "\n";
}

void sim_sensor::Stop(tx_sim::StopHelper& helper) { std::cout << "stop.\n"; }

TXSIM_MODULE(sim_sensor)
