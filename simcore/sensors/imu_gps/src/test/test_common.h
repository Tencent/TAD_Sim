// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include <chrono>
#include <iostream>
#include <map>
#include <vector>

#include "eigen3/Eigen/Dense"
#include "glog/logging.h"
#include "mapengine/hadmap_engine.h"
#include "opencv2/core.hpp"

#define _2_RAD 1.0 / 180.0 * M_PI
#define _2_DEG 1.0 / M_PI * 180.0

namespace test {
// points in camera coordinate
typedef std::vector<Eigen::Vector3d> CameraPoints;

// picture points
typedef cv::Mat PixelPoints;

// color red
static const cv::Vec3b _const_color_red(0, 0, 0xFF);

// first is image timestamp, second is image file name
typedef std::pair<int, std::string> TestImage;
typedef std::map<int, std::string> ImageList;

// imu rigid
struct IMURigid {
  Eigen::Vector3d _Pos_GI;  // enu coordiante
  Eigen::Vector3d _Pos_GI_WGS84;
  Eigen::Quaterniond _Quat_GI;
};

typedef std::vector<IMURigid> IMURigidDataSet;
typedef std::vector<Eigen::Vector3d> SamplePoints;

// expressed in ENU coordinate
struct CLineObject {
  hadmap::txLanePtr lane_ptr;
  SamplePoints sample_points;
};

typedef std::vector<CLineObject> CLineObjects;

// time point, for debug
class CElapsedTime {
 private:
  std::chrono::high_resolution_clock::time_point g_start_t, g_end_t;

 public:
  void MarkStartTimePoint() { g_start_t = std::chrono::high_resolution_clock::now(); }

  void MarkEndTimePoint() { g_end_t = std::chrono::high_resolution_clock::now(); }

  float PrintElapsedTime(const std::string &prefix_string) {
    float elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(g_end_t - g_start_t).count() / 1000.0;
    LOG(INFO) << prefix_string << ", time elapsed:" << elapsed_time << " milliseconds.\n";

    return elapsed_time;
  }
};

// math
namespace math {
double Distance3d(const Eigen::Vector3d &pt1, const Eigen::Vector3d &pt2);
}

// pcd data
typedef std::pair<std::string, std::string> PCDHeaderPair;

struct PCDData {
  const std::vector<std::string> _header_items = {"comment_header", "version",     "fields",   "size",
                                                  "type",           "count",       "width",    "height",
                                                  "viewport",       "points_size", "data_type"};

  std::map<std::string, PCDHeaderPair> _pcd_headers;
  std::vector<std::string> _pcd_data;

  PCDData() {
    _pcd_headers["comment_header"] = std::make_pair("# .PCD v.7 - Point Cloud Data file format", "");
    _pcd_headers["version"] = std::make_pair("VERSION", ".7");
    _pcd_headers["fields"] = std::make_pair("FIELDS", "x y z");
    _pcd_headers["size"] = std::make_pair("SIZE", "4 4 4");
    _pcd_headers["type"] = std::make_pair("TYPE", "F F F");
    _pcd_headers["count"] = std::make_pair("COUNT", "1 1 1");
    _pcd_headers["width"] = std::make_pair("WIDTH", "1");
    _pcd_headers["height"] = std::make_pair("HEIGHT", "1");
    _pcd_headers["viewport"] = std::make_pair("VIEWPOINT", "0 0 0 1 0 0 0");
    _pcd_headers["points_size"] = std::make_pair("POINTS", "1");
    _pcd_headers["data_type"] = std::make_pair("DATA", "ascii");
  }
};
}  // namespace test
