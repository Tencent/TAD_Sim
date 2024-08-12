/**
 * @file TestCamera.cpp
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include <algorithm>
#include <cmath>

#include "test_camera.h"
#include "test_utility.h"

#include "glog/logging.h"

namespace test {
CTestCamera::CTestCamera() {
  // camera coordinate, x to right, y to down, z to front
  // _camera_cfg._Quat_IC is already what we need
  // do anything else that we need
  Eigen::Matrix3d Quat_Init_VC = _Quat_Init_VI.toRotationMatrix() * _cam_cfg._Quat_IC.toRotationMatrix();
  Eigen::Vector3d Euler_GC = Quat_Init_VC.eulerAngles(2, 0, 1);

  // calculate init camera position
  Eigen::Vector3d Pos_Init_VC = _cam_cfg._Pos_Init_VI + _Quat_Init_VI.toRotationMatrix() * _cam_cfg._Pos_IC;

  test::CTestUtility::ShowEigenVector("camera | Euler_Init_GC(2,0,1)", Euler_GC * _2_DEG);
  test::CTestUtility::ShowEigenVector("camera | Pos_Init_VC", Pos_Init_VC);
  test::CTestUtility::ShowEigenMatrix("camera | intrinsic matrix", _cam_cfg._intrinsic_matrix);
}

void CTestCamera::SetPos_GI(const double &x, const double &y, const double &z) {
  _Pos_GI[0] = x;
  _Pos_GI[1] = y;
  _Pos_GI[2] = z;
  test::CTestUtility::ShowEigenVector("camera | _Pos_GI", _Pos_GI);
}

void CTestCamera::SetQuat_GI(const Eigen::Quaterniond &Quat_GI) {
  _Quat_GI = Quat_GI;
  _Rot_GI = _Quat_GI.toRotationMatrix();
  auto &coeffs = _Quat_GI.coeffs();
  LOG(INFO) << "camera | quat(w,x,y,z):" << coeffs(3) << ", " << coeffs(0) << ", " << coeffs(1) << ", " << coeffs(2)
            << "\n";
}

// calculate camera postion in global
void CTestCamera::CalPos_GC() {
  // camera position in global coordinate
  _Pos_GC = _Pos_GI + _Rot_GI * _cam_cfg._Pos_IC;
  test::CTestUtility::ShowEigenVector("camera | _Pos_GC", _Pos_GC);
}

// calculate rotation of camera expressed in global
void CTestCamera::CalRot_CG() {
  Eigen::Matrix3d _Rot_IG = _Rot_GI.inverse();

  test::CTestUtility::ShowEigenVector("camera | Pos_IC", _Rot_IG * (_Pos_GC - _Pos_GI));

  // global coordinate expressed in camera coordinate
  _Rot_CG = _cam_cfg._Rot_CI * _Rot_IG;
}

// calculate rotation of global expressed in camera
void CTestCamera::CalRot_GC() { _Rot_GC = _Rot_GI * _cam_cfg._Quat_IC.toRotationMatrix(); }

// calculate extrinsic matrix
void CTestCamera::CalExtrinsicMatrix() {
  // cal extrinsic matrix
  _extrinsic_matrix = Eigen::Matrix4d::Identity();

  for (auto r = 0; r < 3; ++r) {
    for (auto c = 0; c < 3; ++c) {
      _extrinsic_matrix(r, c) = _Rot_GC(r, c);
    }
    _extrinsic_matrix(r, 3) = _Pos_GC(r);
  }

  _extrinsic_matrix = _extrinsic_matrix.inverse();
}

// world points to camera coordinate
void CTestCamera::WorldPointsToCamera(const CameraPoints &pt_worlds, CameraPoints &Pts_CG) {
  // clear
  Pts_CG.clear();

  try {
    // convert
    for (auto pt_world : pt_worlds) {
      Eigen::Vector4d pt_world_ext(pt_world(0), pt_world(1), pt_world(2), 1.0);

      Eigen::Vector4d Pos_CG = _extrinsic_matrix * pt_world_ext;
      Eigen::Vector3d pt_camera(Pos_CG(0), Pos_CG(1), Pos_CG(2));

      if (pt_camera[2] > 1e-12) {
        Pts_CG.push_back(pt_camera);
        // test::CTestUtility::ShowEigenVector("camera | pt_camera",pt_camera);
      }
    }
  } catch (const std::exception &e) {
    LOG(ERROR) << "error | " << e.what() << "\n";
  }
}

// camera coordinate points to image coordinate
void CTestCamera::CameraToPicture(const CameraPoints &pts_c, PixelPoints &pixel_points) {
  try {
    for (auto pt_camera : pts_c) {
      if (pt_camera(2) > 1e-12) {
        double x = pt_camera(0) / pt_camera(2), y = pt_camera(1) / pt_camera(2);
        double x_ = x, y_ = y;

// distortion
#ifdef _DISTORTION_
        double r = x * x + y * y;
        double ratio = 1.0 + _cam_cfg._k1 * r + _cam_cfg._k2 * r * r + _cam_cfg._k3 * r * r * r;
        x_ = x * ratio + 2 * _cam_cfg._p1 * x * y + _cam_cfg._p2 * (r + 2 * x * x);
        y_ = y * ratio + _cam_cfg._p1 * (r + 2 * y * y) + 2 * _cam_cfg._p2 * x * y;
#else
        // do nothing
#endif

        Eigen::Vector3d pt_c(x_, y_, 1.0);
        Eigen::Vector3d pt_pixel = _cam_cfg._intrinsic_matrix * pt_c;

        // test::CTestUtility::ShowEigenVector("pixel | uv",pt_pixel);

        int u = pt_pixel(0), v = pt_pixel(1);

        if (u >= 0 && u < _cam_cfg._camera_horizontal && v >= 0 && v < _cam_cfg._camera_vertical) {
          pixel_points.at<cv::Vec3b>(v, u) = test::_const_color_red;
        }
      }
    }

    // LOG(INFO)<<"camera | camera coordinate to picture coordinate done.\n";
  } catch (const std::exception &e) {
    LOG(ERROR) << "error | " << e.what() << "\n";
  }
}

Eigen::Vector3d CTestCamera::GetPos_GC() { return _Pos_GC; }
}  // namespace test
