// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include "test_common.h"
#include "test_map.h"

#define _DISTORTION_

namespace test {
struct CTestCameraConfig {
  Eigen::Vector3d _Pos_IC, _Pos_Init_VI;  // position, camera in IMU coordinate
  Eigen::Quaterniond _Quat_IC;            // quaternion, camera in IMU coordinate
  Eigen::Matrix3d _Rot_IC, _Rot_CI;

  double focal;
  int ccd_height, ccd_width;                          // width and height of on pixel
  unsigned int _camera_horizontal, _camera_vertical;  // camera resolution
  double _fov_horizontal, _fov_vertical;              // camera fov
  Eigen::Matrix3d _intrinsic_matrix;
  double _k1, _k2, _k3, _p1, _p2;  // Distortion_Parameters

  CTestCameraConfig() {
    _Pos_IC = {-0.05243217, 2.279668, 0.87131715};
    _Pos_Init_VI = {-1.541, 0.0, 0.58};
    _Quat_IC = Eigen::Quaterniond(0.701021382888812, -0.713133786050885, 0.001062423979092, -0.002839004706153);
    _Rot_IC = _Quat_IC.toRotationMatrix();
    _Rot_CI = _Rot_IC.inverse();

    focal = 10.0;

    ccd_height = 10;
    ccd_width = 10;

    _camera_horizontal = 1920;
    _camera_vertical = 1208;

    _fov_horizontal = 60.0;
    _fov_vertical = 36.28;

    _intrinsic_matrix << 1924.2032686463117, -4.7557312192168055, 981.9631626946548, 0.0, 1930.7846151103831,
        591.9702091631682, 0.0, 0.0, 1.0;

    _k1 = -0.1093679313749193;
    _k2 = 0.08769005663523627;
    _k3 = 0.18457095950838645;
    _p1 = 0.002186133055337269;
    _p2 = -4.92474274260547e-5;
  }
};

class CTestCamera {
 private:
  CTestCameraConfig _cam_cfg;   // camera configuration
  Eigen::Vector3d _Pos_GI;      // imu position in global coordinate
  Eigen::Quaterniond _Quat_GI;  // imu quaternion in global coordinate
  Eigen::Matrix4d _extrinsic_matrix;
  Eigen::Vector3d _Pos_GC;  // camera position in globale coordinate
  Eigen::Matrix3d _Rot_CG, _Rot_GC, _Rot_GI;
  const Eigen::Quaterniond _Quat_Init_VI = {0.70710678118, 0.0, 0.0, -0.70710678118};

 public:
  CTestCamera();
  virtual ~CTestCamera() {}

 public:
  // set postion of imu rigid in global
  void SetPos_GI(const double &x, const double &y, const double &z);

  // set quaternion of imu rigid in global
  void SetQuat_GI(const Eigen::Quaterniond &Quat_GI);

  // calculate camera postion in global
  void CalPos_GC();

  // calculate rotation of camera expressed in global
  void CalRot_CG();

  // calculate rotation of global expressed in camera
  void CalRot_GC();

  // calculate extrinsic matrix
  void CalExtrinsicMatrix();

  // world points to camera coordinate
  void WorldPointsToCamera(const CameraPoints &points_world, CameraPoints &Pts_CG);

  // camera coordinate points to image coordinate
  void CameraToPicture(const CameraPoints &pts_c, PixelPoints &pixel_points);

 public:
  Eigen::Vector3d GetPos_GC();
};
}  // namespace test
