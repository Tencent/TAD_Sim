// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include <iostream>
#include <mutex>
#include <thread>

#include "boost/filesystem.hpp"
#include "eigen3/Eigen/Dense"

#include "mapengine/hadmap_engine.h"

#include "test_common.h"

namespace test {
enum HADMAPState { MAP_NOT_OK, MAP_OK };

class CTestMap {
 private:
  hadmap::txMapHandle *_map_handle;
  std::string _map_path;
  hadmap::MAP_DATA_TYPE _map_type;

  double _sample_step;  // unit m
  HADMAPState _state;   // ok or not ok
  double _radius;       // unit m

  hadmap::txPoint _map_origin;
  hadmap::txPoint _cur_location_enu;
  hadmap::txPoint _cur_location_wgs84;

  test::CLineObjects _line_objects;

 private:
  bool CalNearByLineObjects(const hadmap::txLanes &nearby_lanes);

 public:
  CTestMap() {
    _state = HADMAPState::MAP_NOT_OK;
    _map_handle = nullptr;
    _map_type = hadmap::MAP_DATA_TYPE::SQLITE;
    _radius = 150.0;
    _sample_step = 0.1;
  }
  virtual ~CTestMap() { ReleaseHADMap(); }

  // connect and disconnect with hadmap
  bool InitHADMap(const std::string &map_path, const double &ori_long, const double &ori_lat, const double &ori_alt);
  void ReleaseHADMap();

 public:
  // get nearby lanes
  void ToEigenPoints(const hadmap::PointVec &pts, std::vector<Eigen::Vector3d> &sample_points);
  bool CalNearByObjects(const double &radius);

  // update current location
  void SetCurLocENU(const double &x, const double &y, const double &z);

  // coordinate trans
  Eigen::Vector3d LonToloc(double _Lon, double _Lat, double _Alt);
  Eigen::Vector3d LocToLon(const double &x, const double &y, const double &z);

  void SetRadius(const double &radius);

 public:
  hadmap::txPoint GetCurrentLocationWGS84();
  Eigen::Vector3d GetCurrentLocationENU();
  double GetRadius();
  test::CLineObjects GetLineObjects();
};
}  // namespace test
