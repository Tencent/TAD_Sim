// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include "test_common.h"

namespace test {
class CTestUtility {
 private:
  std::string _image_folder;
  std::string _image_output_folder;

 public:
  ImageList GetInputImages(const std::string image_folder);
  IMURigidDataSet GetIMURigid(const std::string imurigid_csv);
  void LoadImage(const ImageList &img_list, const int &id, PixelPoints &pixel_points);
  void SaveImage(const std::string &image_out_folder, const std::string &img_file_name,
                 const PixelPoints &pixel_points);
  void SavePCD(const std::string &pcd_out_folder, const std::string &pcd_file_name, const SamplePoints &sample_points);

  static void ShowImageList(const ImageList &image_list);
  static void ShowIMURigidDataSet(const IMURigidDataSet &imu_dataset);
  static void ShowEigenVector(const std::string &prefix, const Eigen::Vector3d &vector_in);
  static void ShowEigenMatrix(const std::string &prefix, const Eigen::Matrix3d &mat_in);
};
}  // namespace test
