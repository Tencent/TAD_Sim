/**
 * @file TestMap.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include <iostream>
#include <mutex>
#include <thread>

#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "eigen3/Eigen/Dense"

#include "common/coord_trans.h"
#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"

#include "glog/logging.h"

#include "test_map.h"
#include "test_utility.h"

namespace test {
Eigen::Vector3d CTestMap::LonToloc(double _Lon, double _Lat, double _Alt) {
  Eigen::Vector3d Location;

  double x = _Lon;
  double y = _Lat;
  double z = _Alt;

  coord_trans_api::lonlat2enu(x, y, z, _map_origin.x, _map_origin.y, _map_origin.z);
  Location(0) = x;
  Location(1) = y;
  Location(2) = z;

  return Location;
}

Eigen::Vector3d CTestMap::LocToLon(const double &x, const double &y, const double &z) {
  Eigen::Vector3d longlat = {0.0, 0.0, 0.0};

  double x_loc = x, y_loc = y, z_loc = z;
  coord_trans_api::enu2lonlat(x_loc, y_loc, z_loc, _map_origin.x, _map_origin.y, _map_origin.z);

  longlat[0] = x_loc;
  longlat[1] = y_loc;
  longlat[2] = z_loc;

  return longlat;
}

bool CTestMap::InitHADMap(const std::string &map_path, const double &ori_long, const double &ori_lat,
                          const double &ori_alt) {
  // release hadmap
  ReleaseHADMap();

  // check hadmap file path
  {
    boost::filesystem::path hadmap_file(map_path);
    std::string map_extension;

    if (boost::filesystem::exists(hadmap_file)) {
      map_extension = boost::algorithm::to_lower_copy(hadmap_file.extension().string());
      if (map_extension == ".sqlite") {
        LOG(INFO) << "map type is sqlite.\n";
        _map_type = hadmap::MAP_DATA_TYPE::SQLITE;
      } else if (map_extension == ".xodr") {
        LOG(INFO) << "map type is xodr.\n";
        _map_type = hadmap::MAP_DATA_TYPE::OPENDRIVE;
      } else {
        LOG(ERROR) << "hadmap type do not support. " << hadmap_file.string() << ", use .sqlite or .xodr.\n";
        _state = HADMAPState::MAP_NOT_OK;
        return false;
      }
    } else {
      LOG(ERROR) << "hadmap file do not exist. " << hadmap_file.string() << "\n";
      _state = HADMAPState::MAP_NOT_OK;
      return false;
    }
  }

  // connect hadmap
  {
    if (hadmap::hadmapConnect(map_path.c_str(), _map_type, &_map_handle) == TX_HADMAP_HANDLE_OK) {
      LOG(INFO) << "hadmap connectted. " << map_path << "\n";
    } else {
      LOG(ERROR) << "hadmap connect failed. " << map_path << "\n";
      _state = HADMAPState::MAP_NOT_OK;
      return false;
    }
  }

  // set map origin and current location
  {
    _map_origin.x = ori_long;
    _map_origin.y = ori_lat;
    _map_origin.z = ori_alt;
  }

  _state = HADMAPState::MAP_OK;

  return true;
}

void CTestMap::ReleaseHADMap() {
  if (_map_handle != nullptr) {
    hadmap::hadmapClose(&_map_handle);
    _map_handle = nullptr;
  }
}

bool CTestMap::CalNearByObjects(const double &radius) {
  Eigen::Vector3d center = GetCurrentLocationENU();
  hadmap::txLanes nearby_lanes;

  nearby_lanes.clear();
  _line_objects.clear();

  if (_map_handle == nullptr) return false;

  // envelope
  hadmap::PointVec envelop;

  Eigen::Vector3d bottom_left(center(0) - GetRadius(), center(1) - GetRadius(), center(2));
  Eigen::Vector3d top_right(center(0) + GetRadius(), center(1) + GetRadius(), center(2));

  test::CTestUtility::ShowEigenVector("map | envelop ct", center);
  test::CTestUtility::ShowEigenVector("map | envelop bl", bottom_left);
  test::CTestUtility::ShowEigenVector("map | envelop tr", top_right);

  bottom_left = LocToLon(bottom_left(0), bottom_left(1), bottom_left(2));
  top_right = LocToLon(top_right(0), top_right(1), top_right(2));

  envelop.push_back(hadmap::txPoint(bottom_left(0), bottom_left(1), bottom_left(2)));
  envelop.push_back(hadmap::txPoint(top_right(0), top_right(1), top_right(2)));

  // get nearby lanes
  if (hadmap::getLanes(_map_handle, envelop, nearby_lanes) == TX_HADMAP_DATA_OK) {
    return CalNearByLineObjects(nearby_lanes);
  } else {
    LOG(ERROR) << "fail to get nearby lanes.\n";
    return false;
  }

  return true;
}

bool CTestMap::CalNearByLineObjects(const hadmap::txLanes &nearby_lanes) {
  bool line_objects_retrived = false;

  // clear line objects
  _line_objects.clear();

  // get line objects
  for (auto lane_iter = nearby_lanes.begin(); lane_iter != nearby_lanes.end(); ++lane_iter) {
    // get lane ptr
    const hadmap::txLanePtr &lane_ptr = *lane_iter;

    // print lane information
    // LOG(INFO)<<"map | road id:"<<lane_ptr->getRoadId()<<", section id:"<<lane_ptr->getSectionId()<<", lane
    // id:"<<lane_ptr->getId()<<"\n";

    if (lane_ptr.get() == nullptr) continue;

    // get lane boundries
    std::vector<hadmap::laneboundarypkid> boundry_ids;
    boundry_ids.push_back(lane_ptr->getLeftBoundaryId());
    boundry_ids.push_back(lane_ptr->getRightBoundaryId());

    for (auto boundry_id : boundry_ids) {
      // boundry ptr
      hadmap::txLaneBoundaryPtr boundry_ptr;

      /* query lane by lane*/
      if (hadmap::getBoundary(_map_handle, boundry_id, boundry_ptr) == TX_HADMAP_DATA_OK) {
        // one line object
        test::CLineObject line_object;

        line_object.lane_ptr = lane_ptr;

        const hadmap::txCurve *curve_ptr = dynamic_cast<const hadmap::txCurve *>(boundry_ptr->getGeometry());

        if (curve_ptr != nullptr) {
          hadmap::PointVec sample_pts;

          curve_ptr->sample(_sample_step, sample_pts);
          ToEigenPoints(sample_pts, line_object.sample_points);

          _line_objects.push_back(line_object);

          line_objects_retrived = true;
        }
      } else {
        LOG(ERROR) << "fail to get lane boundry objects from lane id:" << lane_ptr->getTxLaneId().laneId
                   << ", section id:" << lane_ptr->getTxLaneId().sectionId
                   << ", road id:" << lane_ptr->getTxLaneId().roadId << "\n";
      }
    }
  }
  return line_objects_retrived;
}

void CTestMap::ToEigenPoints(const hadmap::PointVec &pts, std::vector<Eigen::Vector3d> &sample_points) {
  for (auto pt_iter = pts.begin(); pt_iter != pts.end(); pt_iter++) {
    Eigen::Vector3d point = LonToloc(pt_iter->x, pt_iter->y, pt_iter->z);

    if (test::math::Distance3d(point, GetCurrentLocationENU()) <= GetRadius()) {
      sample_points.push_back(point);
    }
  }
}

void CTestMap::SetCurLocENU(const double &x, const double &y, const double &z) {
  _cur_location_enu.x = x;
  _cur_location_enu.y = y;
  _cur_location_enu.z = z;

  Eigen::Vector3d loc_wgs84 = LocToLon(x, y, z);
  _cur_location_wgs84.x = loc_wgs84[0];
  _cur_location_wgs84.y = loc_wgs84[1];
  _cur_location_wgs84.z = loc_wgs84[2];
}

void CTestMap::SetRadius(const double &radius) {
  if (radius > 0.0) {
    _radius = radius;
  }
}

hadmap::txPoint CTestMap::GetCurrentLocationWGS84() {
  hadmap::txPoint pt;

  pt = _cur_location_wgs84;

  return pt;
}

Eigen::Vector3d CTestMap::GetCurrentLocationENU() {
  Eigen::Vector3d pt;

  pt[0] = _cur_location_enu.x;
  pt[1] = _cur_location_enu.y;
  pt[2] = _cur_location_enu.z;

  return pt;
}

double CTestMap::GetRadius() { return _radius; }

test::CLineObjects CTestMap::GetLineObjects() { return _line_objects; }
}  // namespace test
