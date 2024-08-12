// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <cmath>
#include "eigen3/Eigen/Dense"
#include "eval_common.h"

namespace eval {
/**
 * @brief point class
 */
class EVector3d {
 private:
  Eigen::Vector3d _pt;
  CoordType _coord_type;

 public:
  EVector3d(double x, double y, double z, CoordType coord_type = Coord_ENU);
  EVector3d() : EVector3d(0.0, 0.0, 0.0, Coord_ENU) {}
  EVector3d(const EVector3d &in) {
    _pt = in.GetPoint();
    _coord_type = in.GetCoordType();
  }
  EVector3d &operator=(const EVector3d &in) {
    _pt = in.GetPoint();
    _coord_type = in.GetCoordType();
    return *this;
  }
  EVector3d(const EVector3d &&in) {
    _pt = in.GetPoint();
    _coord_type = in.GetCoordType();
  }
  EVector3d &operator=(const EVector3d &&in) {
    _pt = in.GetPoint();
    _coord_type = in.GetCoordType();
    return *this;
  }
  virtual ~EVector3d() {}

 public:
  // set values by x, y, z and coordinate type. coord_type is default to Coord_ENU.
  void SetValues(double x, double y, double z, CoordType coord_type = Coord_ENU);

 public:
  // Get the x value of the point
  inline double GetX() const { return _pt[0]; }
  // Get the y value of the point
  inline double GetY() const { return _pt[1]; }
  // Get the z value of the point
  inline double GetZ() const { return _pt[2]; }
  // Get the norm of the point
  inline double GetNormal() const { return _pt.norm(); }
  // Get the norm of the point in 2D (x and y)
  inline double GetNormal2D() const { return sqrt(_pt[0] * _pt[0] + _pt[1] * _pt[1]); }
  // Normalize the point
  inline void Normalize() { _pt.normalize(); }
  // Get the coordinate type of the point
  inline CoordType GetCoordType() const { return _coord_type; }
  // Get the origin point
  inline Eigen::Vector3d GetPoint() const { return _pt; }
};

/**
 * @brief euler class
 */
class CEuler {
 private:
  double _roll, _pitch, _yaw;
  Eigen::Matrix3d _rot_matrix;

 public:
  explicit CEuler(double roll = 0.0, double pitch = 0.0, double yaw = 0.0);
  virtual ~CEuler() {}
  CEuler(const CEuler &in) {
    _roll = in.GetRoll();
    _pitch = in.GetPitch();
    _yaw = in.GetYaw();
    _rot_matrix = in.GetRotMatrix();
  }
  CEuler &operator=(const CEuler &in) {
    _roll = in.GetRoll();
    _pitch = in.GetPitch();
    _yaw = in.GetYaw();
    _rot_matrix = in.GetRotMatrix();
    return *this;
  }
  CEuler(CEuler &&in) {
    _roll = in.GetRoll();
    _pitch = in.GetPitch();
    _yaw = in.GetYaw();
    _rot_matrix = in.GetRotMatrix();
  }
  CEuler &operator=(CEuler &&in) {
    _roll = in.GetRoll();
    _pitch = in.GetPitch();
    _yaw = in.GetYaw();
    _rot_matrix = in.GetRotMatrix();
    return *this;
  }

  // set values by roll, pitch and yaw
  void SetValues(double roll, double pitch, double yaw);

 public:
  // get the roll value of the euler
  inline double GetRoll() const { return _roll; }
  // get the pitch value of the euler
  inline double GetPitch() const { return _pitch; }
  // get the yaw value of the euler
  inline double GetYaw() const { return _yaw; }
  // get the rotation matrix of the euler
  inline Eigen::Matrix3d GetRotMatrix() const { return _rot_matrix; }
};

/**
 * @brief location class, contains position (EVector3d) and euler
 */
class CLocation {
 private:
  CPosition _pos;
  CEuler _rpy;

 public:
  CLocation(double x = 0.0, double y = 0.0, double z = 0.0, double roll = 0.0, double pitch = 0.0, double yaw = 0.0,
            CoordType coord_type = Coord_ENU);
  CLocation(const CPosition &pos, const CEuler &euler) : _pos(pos), _rpy(euler) {}
  CLocation(const CLocation &loc) {
    _pos = loc.GetPosition();
    _rpy = loc.GetEuler();
  }
  CLocation &operator=(const CLocation &loc) {
    _pos = loc.GetPosition();
    _rpy = loc.GetEuler();
    return *this;
  }
  CLocation(CLocation &&loc) {
    _pos = loc.GetPosition();
    _rpy = loc.GetEuler();
  }
  CLocation &operator=(CLocation &&loc) {
    _pos = loc.GetPosition();
    _rpy = loc.GetEuler();
    return *this;
  }
  virtual ~CLocation() {}

 public:
  // return a constant reference to the position, ensuring read-only access
  inline const CPosition &GetPosition() const { return _pos; }
  // return a constant reference to the euler, ensuring read-only access
  inline const CEuler &GetEuler() const { return _rpy; }
  // return a constant reference to the rotation matrix of the euler, ensuring read-only access
  inline Eigen::Matrix3d GetRotMatrix() const { return _rpy.GetRotMatrix(); }

  // return a pointer to the position, allowing modification of its value.
  inline CPosition *MutablePosition() { return &_pos; }
  // return a pointer to the euler, allowing modification of its value.
  inline CEuler *MutableEuler() { return &_rpy; }

 public:
  void DebugShow(const std::string &prefix) const;
};

/**
 * @brief a vector of CLocation
 */
class EvalPoints final : public std::vector<CLocation> {
 public:
  EvalPoints() { reserve(const_MSPS); }
};

/**
 * @brief all kinds of mathematical methods used in eval
 */
class CEvalMath {
 public:
  // subtract the coordinates of 'pt2' from 'pt1' and returns the result as a new EVector3d object.
  static EVector3d Sub(const EVector3d &pt1, const EVector3d &pt2);
  // add the coordinates of 'pt2' from 'pt1' and returns the result as a new EVector3d object.
  static EVector3d Add(const EVector3d &pt1, const EVector3d &pt2);
  // calculate the distance of 'pt1' and 'pt2' in x, y, z.
  static double Distance3D(const EVector3d &pt1, const EVector3d &pt2);
  // calculate the distance of 'pt1' and 'pt2' in x, y.
  static double Distance2D(const EVector3d &pt1, const EVector3d &pt2);
  // calculate the manhattan distance of 'pt1' and 'pt2' in x, y.
  static double AbsoluteDistance2D(const EVector3d &pt1, const EVector3d &pt2);
  // get the Matrix3d by using roll, pitch, yaw
  static Eigen::Matrix3d ToRotMatrix(double roll, double pitch, double yaw);
  // get the Matrix3d by using yaw
  static Eigen::Matrix3d ToRotMatrix(double yaw);

  // z-y-x order, angle value range:[0:pi]x[-pi:pi]x[-pi:pi]
  static CEuler RotMatrix2Euler(const Eigen::Matrix3d &rot);

  // get yaw diff of oren_2 in oren_1, [-pi,pi]
  static double YawDiff(const CEuler &oren_1, const CEuler &oren_2);

  // return R_XB according to R_GX and R_GB
  static Eigen::Matrix3d OrientationTransXB(const Eigen::Matrix3d &R_GX, const Eigen::Matrix3d &R_GB);

  // return XB_Loc according to GX_loc and GB_loc
  static eval::CLocation CalLocationXB(const eval::CLocation &GX_loc, const eval::CLocation &GB_loc);

 public:
  // get nearest point index between given_loc and locs(limitted by max_dist), return -1 if no such location
  static int GetNearestPointIndex(const EvalPoints &locs, const CLocation &given_loc, const double &max_dist = 4.0);

  // calculate lateral dist between loc_base and loc
  static double GetLateralDist(const CLocation &loc_base, const CLocation &loc);

  // calculate vertical dist between loc_base and loc
  static double GetVerticalDist(const CLocation &loc_base, const CLocation &loc);

  // calculate lateral yaw between loc_base and loc
  static double GetLateralYaw(const CLocation &loc_base, const CLocation &loc);

  // calculate yaw of each location, not implement yet, do not use
  static void CalYaws(EvalPoints &locs);
};
}  // namespace eval
