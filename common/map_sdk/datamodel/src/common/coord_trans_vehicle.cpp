// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "common/coord_trans_vehicle.h"
#include <stdio.h>
#include <algorithm>
#include <boost/property_tree/json_parser.hpp>
#include <cmath>
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include "common/coord_trans.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_object.h"
#include "structs/hadmap_road.h"
#include "types/map_defs.h"

using namespace std;
using namespace coord_trans_api;
using namespace hadmap;

// Set the position of the vehicle in the global coordinate system
void Vehicle::setCarPosition(double x, double y, double z) {
  // If the reference longitude, latitude, and altitude are not set, set them to the current position
  if (refLon == 0 && refLat == 0 && refAlt == 0) {
    refLon = x;
    refLat = y;
    refAlt = z;
  }
  // Convert the position from longitude, latitude, and altitude to the local ENU (East, North, Up) coordinate system
  lonlat2enu(x, y, z, refLon, refLat, refAlt);
  // Set the translation vector T and its inverse Ti
  T = -Eigen::Vector3d(x, y, z);
  Ti = -T;
}

// Set the rotation of the vehicle in the global coordinate system
void Vehicle::setCarRoatation(double roll, double pitch, double yaw) {
  // Create a quaternion representing the rotation
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  // Set the rotation matrix Ri and its inverse R
  Ri = q.toRotationMatrix();
  R = Ri.inverse();
}

// Transform a vector from the global coordinate system to the local vehicle coordinate system
void Vehicle::Vector(double& x, double& y, double& z) const {
  // Convert the position from longitude, latitude, and altitude to the local ENU (East, North, Up) coordinate system
  lonlat2enu(x, y, z, refLon, refLat, refAlt);
  // Transform the vector to the local vehicle coordinate system
  Eigen::Vector3d p = R * (Eigen::Vector3d(x, y, z) + T);
  x = p.x();
  y = p.y();
  z = p.z();
}

// Transform a vector from the local vehicle coordinate system to the global coordinate system
void Vehicle::Vector_inv(double& x, double& y, double& z) const {
  // Transform the vector to the global coordinate system
  Eigen::Vector3d p = Ri * Eigen::Vector3d(x, y, z) + Ti;
  // Convert the position from the local ENU (East, North, Up) coordinate system to longitude, latitude, and altitude
  enu2lonlat(p.x(), p.y(), p.z(), refLon, refLat, refAlt);
  x = p.x();
  y = p.y();
  z = p.z();
}

// Transform a velocity vector from the local vehicle coordinate system to the global coordinate system
void coord_trans_api::Vehicle::Velocity(double& Vx, double& Vy, double heading) const {
  double v = std::sqrt(Vx * Vx + Vy * Vy);
  Eigen::Vector3d vv(v * std::cos(heading), v * std::sin(heading), 0);
  Eigen::Vector3d vp = R * vv;
  Vx = vp.x();
  Vy = vp.y();
}

// Transform an acceleration vector from the local vehicle coordinate system to the global coordinate system
void coord_trans_api::Vehicle::Acceleration(double& Ax, double& Ay, double heading) const {
  double v = std::sqrt(Ax * Ax + Ay * Ay);
  Eigen::Vector3d vv(v * std::cos(heading), v * std::sin(heading), 0);
  Eigen::Vector3d vp = R * vv;
  Ax = vp.x();
  Ay = vp.y();
}

// Transform a rotational vector from the local vehicle coordinate system to the global coordinate system
void coord_trans_api::Vehicle::Rotator(double& Nx, double& Ny, double& Nz, double roll, double pitch,
                                       double yaw) const {
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  Eigen::Vector3d m = R * (q.toRotationMatrix()) * Eigen::Vector3d(Nx, Ny, Nz);
  Nx = m.x();
  Ny = m.y();
  Nz = m.z();
}

// Set the installation position of the sensor in the vehicle coordinate system
void Sensor::setInstallPosition(double x, double y, double z) {
  // Set the translation vector Tl and its inverse Tli
  Tl = -Eigen::Vector3d(x, y, z);
  Tli = -Tl;
}

// Set the installation rotation of the sensor in the vehicle coordinate system
void Sensor::setInstallRotation(double roll, double pitch, double yaw) {
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  // Set the rotation matrix Rli and its inverse Rl
  Rli = q.toRotationMatrix();
  Rl = Rli.inverse();
}

void Sensor::Vector(double& x, double& y, double& z) const {
  Vehicle::Vector(x, y, z);
  Eigen::Vector3d p = Rl * (Eigen::Vector3d(x, y, z) + Tl);
  x = p.x();
  y = p.y();
  z = p.z();
}

void Sensor::Vector_inv(double& x, double& y, double& z) const {
  Eigen::Vector3d p = Rli * Eigen::Vector3d(x, y, z) + Tli;
  x = p.x();
  y = p.y();
  z = p.z();
  Vehicle::Vector_inv(x, y, z);
}

/// <summary>
/// todo
/// </summary>
/// <param name="obj"></param>
/// <returns></returns>
// ParkingSpace class
bool ParkingSpace::setObject(const hadmap::txObject& obj) {
  // Check if the object type is not parking or RR_Parking
  if (obj.getObjectType() != OBJECT_TYPE_ParkingSpace && obj.getObjectType() != OBJECT_TYPE_RR_Parking) {
    std::cout << "object type is not parking.\n";
    return false;
  }

  // Get the length, width, and height of the object
  double len = 0, wid = 0, hei = 0;
  double angle = 0;
  obj.getLWH(len, wid, hei);

  // Get the roll, pitch, and yaw of the object
  double roll = 0, pitch = 0, yaw = 0;
  obj.getRPY(roll, pitch, yaw);

  // Initialize variables
  Eigen::Vector3d PO;
  bool hasgeo = false;

  // Iterate through the object's geometries
  for (size_t i = 0, is = obj.getGeomSize(); i < is; i++) {
    auto geo_ptr = obj.getGeom(i);
    hadmap::PointVec pts;
    dynamic_cast<const txLineCurve*>(geo_ptr->getGeometry())->getPoints(pts);

    // Check if the number of points is 4
    if (pts.size() == 4) {
      Eigen::Vector3d pos(0, 0, 0);
      std::vector<Eigen::Vector3d> epts;
      for (size_t j = 0; j < 4; ++j) {
        pos += Eigen::Vector3d(pts[j].x, pts[j].y, pts[j].z);
      }
      pos *= 0.25;

      size_t bestj = 4;
      double bestv = -10000000;
      for (size_t j = 0; j < 4; ++j) {
        double x = pts[j].x;
        double y = pts[j].y;
        double z = pts[j].z;
        lonlat2enu(x, y, z, pos.x(), pos.y(), pos.z());
        double s = x * std::cos(-yaw) - y * std::sin(-yaw);
        double t = x * std::sin(-yaw) + y * std::cos(-yaw);
        double v = Eigen::Vector3d(s, t, 0).dot(Eigen::Vector3d(-1, 1, 0));
        if (v > bestv) {
          bestv = v;
          bestj = j;
        }
      }
      if (bestj < 4) {
        PO = Eigen::Vector3d(pts[bestj].x, pts[bestj].y, pts[bestj].z);
        hasgeo = true;
      }
    }
  }

  // If no geometry was found, check if length and width are valid
  if (!hasgeo) {
    if (len > 0 && wid > 0) {
      // Calculate the quaternion based on the roll, pitch, and yaw
      Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                             Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                             Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());

      hadmap::PointVec pts;
      if (obj.getRepeat()) {
        obj.getRepeat()->getPoints(pts);
      }
      if (pts.empty()) {
        pts.push_back(obj.getPos());
      } else {
        wid = pts.front().w;
        len /= pts.size();
      }

      // Get user data from the object
      std::map<std::string, std::string> userData;
      obj.getUserData(userData);
      if (userData.find("userData") != userData.end()) {
        std::stringstream user = std::stringstream(userData.at("userData"));
        boost::property_tree::ptree pt;
        boost::property_tree::json_parser::read_json(user, pt);
        if (pt.find("park") != pt.not_found()) {
          auto pk = pt.get_child("park");
          if (pk.get<double>("length", 0) > 0) {
            len = pk.get<double>("length") + 0.1;
          }
          if (pk.get<double>("angle", 0) > 0) {
            angle = pk.get<double>("angle");
          }
          if (pk.get<double>("width", 0) > 0) {
            wid = pk.get<double>("width") + 0.1;
          }
        }
      }

      // Calculate the skew and the position of the parking space
      double skew = wid * 0.5 * std::tan(angle * EIGEN_PI / 180.);
      PO = q * Eigen::Vector3d(-len * 0.5 - skew, wid * 0.5, 0);
      enu2lonlat(PO.x(), PO.y(), PO.z(), pts[0].x, pts[0].y, pts[0].z);
      hasgeo = true;
    }
  }

  // If no geometry was found, return false
  if (!hasgeo) {
    std::cout << "Faild to get corner of pariking.\n";
    return false;
  }

  // Set the object using the calculated position and yaw
  return setObject(PO.x(), PO.y(), yaw);
}

bool ParkingSpace::setObject(double x, double y, double yaw) {
  refLon = 0;
  refLat = 0;
  refAlt = 0;
  setCarPosition(x, y, 0);
  setCarRoatation(0, 0, yaw);

  // Set the installation position and rotation
  setInstallPosition(-10, 6, 0);
  setInstallRotation(0, 0, 0);
  return true;
}

void ParkingSpace::ParkingVector(double& x, double& y, double& z) const { Vehicle::Vector(x, y, z); }

void ParkingSpace::ParkingVector_inv(double& x, double& y, double& z) const { Vehicle::Vector_inv(x, y, z); }

void ParkingSpace::GridVector(double& x, double& y, double& z) const {
  Sensor::Vector(x, y, z);
  y *= -1;
}

void ParkingSpace::GridVector_inv(double& x, double& y, double& z) const {
  y *= -1;
  Sensor::Vector_inv(x, y, z);
}

// Road class
bool Road::setRoad(const hadmap::txRoad& road) {
  curve = road.getGeometry();
  return true;
}

bool Road::Vector(double& x, double& y) const {
  double s = 0, l = 0, yaw = 0;
  if (curve && curve->xy2sl(x, y, s, l, yaw)) {
    x = s;
    y = l;
    return true;
  }
  return false;
}

bool Road::Vector_inv(double& s, double& t) const {
  double x = 0, y = 0, yaw = 0;
  if (curve && curve->sl2xy(s, t, x, y, yaw)) {
    s = x;
    t = y;
    return true;
  }
  return false;
}
