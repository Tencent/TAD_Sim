// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//

#pragma once
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include "boost/variant.hpp"
#include "osi_datarecording.pb.h"
#include "sensor_raw.pb.h"

struct FileInfo {
  std::int64_t id = 0;
  std::int64_t timestamp = 0;
  size_t size = 0;
  std::string buffer;
  std::string fpath;
};

struct Pose {
  double x = 0, y = 0, z = 0;
  double roll = 0, pitch = 0, yaw = 0;
};

struct ImageInfo : FileInfo {
  int width = 0;
  int height = 0;
  Pose pose;
};

struct PcInfo : FileInfo {
  std::uint32_t count = 0;
  int coord_type = 0;
  std::int64_t timestamp_bg = 0;
  std::int64_t timestamp_ed = 0;
  Pose pose;
};

struct ImagePackage {
  std::uint64_t frame_c = 0;
  ImageInfo image;
  sim_msg::SensorRaw_Type type;
  sim_msg::DisplayPose obj;
};
struct PcdPackage {
  std::uint64_t frame_c = 0;
  sim_msg::DisplayPose obj;
  PcInfo lidar;
};

class DataQueue {
 public:
  DataQueue();
  ~DataQueue();

  void setImageCallback(const std::function<void(const ImagePackage &)> &callback);
  void setPcdCallback(const std::function<void(const PcdPackage &)> &callback);

  void addCamera(const ImageInfo &info);
  void addFisheye(const ImageInfo &info);
  void addSenmantic(const ImageInfo &info);
  void addLidar(const PcInfo &info);
  void addObject(const sim_msg::DisplayPose &info);

  /// @brief
  /// @param
  void update();

 private:
  std::map<std::int64_t, std::map<std::int64_t, ImageInfo>> _cameras;    // id - [timestmp - info]
  std::map<std::int64_t, std::map<std::int64_t, ImageInfo>> _semantics;  // id - [timestmp - info]
  std::map<std::int64_t, std::map<std::int64_t, ImageInfo>> _fisheyes;   // id - [timestmp - info]
  std::map<std::int64_t, std::map<std::int64_t, PcInfo>> _lidars;        // id - [timestmp - info]
  std::map<std::int64_t, sim_msg::DisplayPose> _objects;                 // timestmp - info
  std::function<void(const ImagePackage &)> _callback_image;
  std::function<void(const PcdPackage &)> _callback_pcd;
  bool _stop = false;
  std::mutex _mutex;
  std::thread _job_worker;
  std::condition_variable _job_condition;
};
