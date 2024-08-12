// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_math.h"

namespace eval {
/**********************/
EVector3d::EVector3d(double x, double y, double z, CoordType coord_type) { SetValues(x, y, z, coord_type); }

void EVector3d::SetValues(double x, double y, double z, CoordType coord_type) {
  _pt = {x, y, z};
  _coord_type = coord_type;
}

EVector3d CEvalMath::Sub(const EVector3d &pt1, const EVector3d &pt2) {
  return EVector3d(pt1.GetX() - pt2.GetX(), pt1.GetY() - pt2.GetY(), pt1.GetZ() - pt2.GetZ());
}

EVector3d CEvalMath::Add(const EVector3d &pt1, const EVector3d &pt2) {
  return EVector3d(pt1.GetX() + pt2.GetX(), pt1.GetY() + pt2.GetY(), pt1.GetZ() + pt2.GetZ());
}

/**********************/
double CEvalMath::Distance3D(const EVector3d &pt1, const EVector3d &pt2) {
  auto pt = CEvalMath::Sub(pt1, pt2);
  return pt.GetNormal();
}
double CEvalMath::Distance2D(const EVector3d &pt1, const EVector3d &pt2) {
  return std::hypot(pt1.GetX() - pt2.GetX(), pt1.GetY() - pt2.GetY());
}
double CEvalMath::AbsoluteDistance2D(const EVector3d &pt1, const EVector3d &pt2) {
  return std::abs(pt1.GetX() - pt2.GetX()) + std::abs(pt1.GetY() - pt2.GetY());
}
Eigen::Matrix3d CEvalMath::ToRotMatrix(double roll, double pitch, double yaw) {
  Eigen::AngleAxisd yaw_rot(yaw, Eigen::Vector3d::UnitZ());
  Eigen::AngleAxisd pitch_rot(pitch, Eigen::Vector3d::UnitY());
  Eigen::AngleAxisd roll_rot(roll, Eigen::Vector3d::UnitX());

  // Combine rotations in yaw-pitch-roll order into a quaternion.
  Eigen::Quaterniond Q = yaw_rot * pitch_rot * roll_rot;
  Q.normalize();

  return Q.toRotationMatrix();
}

Eigen::Matrix3d CEvalMath::ToRotMatrix(double yaw) {
  Eigen::AngleAxisd yaw_rot(yaw, Eigen::Vector3d::UnitZ());
  return yaw_rot.toRotationMatrix();
}

CEuler CEvalMath::RotMatrix2Euler(const Eigen::Matrix3d &rot) {
  CEuler euler;
  Eigen::Vector3d euler_ = rot.eulerAngles(2, 1, 0);
  euler.SetValues(euler_[2], euler_[1], euler_[0]);
  return euler;
}

Eigen::Matrix3d CEvalMath::OrientationTransXB(const Eigen::Matrix3d &R_GX, const Eigen::Matrix3d &R_GB) {
  return R_GX.inverse() * R_GB;
}

// get nearest point index between given_loc and locs(limitted by max_dist), return -1 if no such location
int CEvalMath::GetNearestPointIndex(const EvalPoints &locs, const CLocation &given_loc, const double &max_dist) {
  int index = -1;
  double min_dist = static_cast<double>(LONG_MAX), tmp_dist = 0.0;

  for (auto i = 0; i < locs.size(); ++i) {
    tmp_dist = CEvalMath::Distance2D(locs.at(i).GetPosition(), given_loc.GetPosition());
    if (tmp_dist < max_dist && tmp_dist < min_dist) {
      min_dist = tmp_dist;
      index = i;
    }
  }

  return index;
}

// calculate lateral dist between given_loc and locs
double CEvalMath::GetLateralDist(const CLocation &loc_base, const CLocation &loc) {
  Eigen::Matrix3d &&R_BX = CEvalMath::ToRotMatrix(loc_base.GetEuler().GetYaw()).inverse();

  // vector from loc_base to loc
  Eigen::Vector3d &&Vec_X_BM = CEvalMath::Sub(loc.GetPosition(), loc_base.GetPosition()).GetPoint();

  Eigen::Vector3d &&Vec_BM = R_BX * Vec_X_BM;

  return Vec_BM[1];
}

double CEvalMath::GetVerticalDist(const CLocation &loc_base, const CLocation &loc) {
  Eigen::Matrix3d &&R_BX = CEvalMath::ToRotMatrix(loc_base.GetEuler().GetYaw()).inverse();

  // vector from loc_base to loc
  Eigen::Vector3d &&Vec_X_BM = CEvalMath::Sub(loc.GetPosition(), loc_base.GetPosition()).GetPoint();

  Eigen::Vector3d &&Vec_BM = R_BX * Vec_X_BM;

  return Vec_BM[0];
}

// calculate lateral yaw between given_loc and locs
double CEvalMath::GetLateralYaw(const CLocation &loc_base, const CLocation &loc) {
  return YawDiff(loc_base.GetEuler(), loc.GetEuler());
}

double CEvalMath::YawDiff(const CEuler &euler1, const CEuler &euler2) {
  Eigen::Vector3d axis_x = {1.0, 0.0, 0.0};
  Eigen::Vector3d rotated_axis_x = euler1.GetRotMatrix().inverse() * euler2.GetRotMatrix() * axis_x;
  return std::atan2(rotated_axis_x[1], rotated_axis_x[0]);
}

// calculate yaw of each location
void CEvalMath::CalYaws(EvalPoints &locs) {}

eval::CLocation CEvalMath::CalLocationXB(const eval::CLocation &GX_loc, const eval::CLocation &GB_loc) {
  // CPosition &&G_B2X = CEvalMath::Sub(GX_loc.GetPosition(), GB_loc.GetPosition());
  CPosition &&G_X2B = CEvalMath::Sub(GB_loc.GetPosition(), GX_loc.GetPosition());
  Eigen::Matrix3d &&R_XG = GX_loc.GetRotMatrix().inverse();
  Eigen::Matrix3d &&R_XB = R_XG * GB_loc.GetRotMatrix();
  Eigen::Vector3d &&pos = R_XG * G_X2B.GetPoint();
  CLocation XB_loc;
  XB_loc.MutablePosition()->SetValues(pos[0], pos[1], pos[2]);
  CEuler &&euler = CEvalMath::RotMatrix2Euler(R_XB);
  XB_loc.MutableEuler()->SetValues(euler.GetRoll(), euler.GetPitch(), euler.GetYaw());
  return XB_loc;
}

/**********************/
CEuler::CEuler(double roll, double pitch, double yaw) { SetValues(roll, pitch, yaw); }

void CEuler::SetValues(double roll, double pitch, double yaw) {
  _roll = roll;
  _pitch = pitch;
  _yaw = yaw;
  _rot_matrix = CEvalMath::ToRotMatrix(_roll, _pitch, _yaw);
}

/**********************/
CLocation::CLocation(double x, double y, double z, double roll, double pitch, double yaw, CoordType coord_type) {
  _pos = CPosition(x, y, z, coord_type);
  _rpy = CEuler(roll, pitch, yaw);
}

void CLocation::DebugShow(const std::string &prefix) const {
  VLOG_2 << prefix << "x,y,z:" << _pos.GetX() << ", " << _pos.GetY() << ", " << _pos.GetZ()
         << ", roll,pitch,yaw:" << _rpy.GetRoll() << ", " << _rpy.GetPitch() << ", " << _rpy.GetYaw() << "\n";
}
}  // namespace eval
