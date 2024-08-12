/**
 * @file TestUtility.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "test_utility.h"
#include <boost/algorithm/string.hpp>
#include <exception>
#include <fstream>
#include "boost/filesystem.hpp"
#include "glog/logging.h"
#include "opencv2/imgcodecs.hpp"

namespace test {
ImageList CTestUtility::GetInputImages(const std::string image_folder) {
  ImageList image_list;
  boost::filesystem::path img_root_dir(image_folder);

  if (boost::filesystem::is_directory(img_root_dir)) {
    _image_folder = image_folder;

    for (auto const &entry : boost::filesystem::recursive_directory_iterator(img_root_dir)) {
      if (boost::filesystem::is_regular_file(entry) && entry.path().extension() == ".jpg") {
        auto filename = entry.path().filename().string();
        std::vector<std::string> strs;
        boost::split(strs, filename, boost::is_any_of("."));
        if (strs.size() == 2 && strs[1] == "jpg") {
          std::vector<std::string> strs_;
          boost::split(strs_, strs[0], boost::is_any_of("_"));
          if (strs_.size() == 3) {
            int index = std::stoi(strs_[2]);
            image_list[index] = filename;
          }
        }
      }
    }
  }

  return image_list;
}

void CTestUtility::LoadImage(const ImageList &img_list, const int &id, PixelPoints &pixel_points) {
  std::string image_file = _image_folder + "/" + img_list.at(id);
  pixel_points = cv::imread(image_file, cv::IMREAD_COLOR);

  LOG(INFO) << "test | load image from " << image_file << " done. pixel points size:" << pixel_points.size << "\n";
}

void CTestUtility::SaveImage(const std::string &image_out_folder, const std::string &img_file_name,
                             const PixelPoints &pixel_points) {
  std::string img_file = image_out_folder + "/" + img_file_name;
  cv::imwrite(img_file, pixel_points);
  LOG(INFO) << "test | image saved into " << img_file << "\n";
}

void CTestUtility::SavePCD(const std::string &pcd_out_folder, const std::string &pcd_file_name,
                           const SamplePoints &sample_points) {
  // otuput pcd file name
  std::string pcd_file = pcd_out_folder + "/" + pcd_file_name;

  // check if data valid
  if (sample_points.size() <= 0) return;

  // pcd data
  test::PCDData pcd_data;

  // set pcd data header
  pcd_data._pcd_headers["width"].second = std::to_string(sample_points.size());
  pcd_data._pcd_headers["points_size"].second = std::to_string(sample_points.size());

  // set pcd data
  for (auto point : sample_points) {
    std::string pcd_point =
        std::to_string(point(0)) + " " + std::to_string(point(1)) + " " + std::to_string(point(2)) + "\n";
    pcd_data._pcd_data.push_back(pcd_point);
  }

  // open output stream
  std::ofstream pcd_out_stream(pcd_file, std::ios::out);

  // save header
  if (pcd_out_stream.is_open()) {
    for (auto header_item : pcd_data._header_items) {
      pcd_out_stream << pcd_data._pcd_headers[header_item].first << " " << pcd_data._pcd_headers[header_item].second
                     << "\n";
    }
  }

  // save pcd data
  if (pcd_out_stream.is_open()) {
    for (auto point_data : pcd_data._pcd_data) {
      pcd_out_stream << point_data;
    }
  }

  pcd_out_stream.close();

  LOG(INFO) << "test | pcd file saved into " << pcd_file << "\n";
}

void CTestUtility::ShowImageList(const ImageList &image_list) {
  for (auto image : image_list) {
    LOG(INFO) << "test | image id:" << image.first << ", file name:" << image.second << "\n";
  }
}

IMURigidDataSet CTestUtility::GetIMURigid(const std::string imurigid_csv) {
  IMURigidDataSet imurigid_dataset;
  boost::filesystem::path csv_path(imurigid_csv);

  if (boost::filesystem::is_regular_file(csv_path)) {
    std::ifstream in_stream;
    std::string line;

    in_stream.open(imurigid_csv, std::ios::in);

    // discard first line
    std::getline(in_stream, line);

    // read imurigid data
    while (std::getline(in_stream, line) && in_stream.good()) {
      std::vector<std::string> strs;
      boost::split(strs, line, boost::is_any_of(","));
      if (strs.size() > 0) {
        try {
          test::IMURigid imurigid;
          imurigid._Pos_GI = {std::stod(strs[1]), std::stod(strs[2]), std::stod(strs[3])};
          imurigid._Pos_GI_WGS84 = {std::stod(strs[11]), std::stod(strs[12]), std::stod(strs[13])};
          imurigid._Quat_GI = {std::stod(strs[7]), std::stod(strs[8]), std::stod(strs[9]), std::stod(strs[10])};
          imurigid_dataset.push_back(imurigid);
        } catch (const std::exception &e) {
          LOG(ERROR) << "test | error, " << e.what() << "\n";
        }
      }
    }

    in_stream.close();
  }

  return imurigid_dataset;
}

void CTestUtility::ShowIMURigidDataSet(const IMURigidDataSet &imu_dataset) {
  for (auto imurigid : imu_dataset) {
    auto quat = imurigid._Quat_GI.coeffs();
    LOG(INFO) << "test | position:(" << imurigid._Pos_GI(0) << ", " << imurigid._Pos_GI(1) << ", "
              << imurigid._Pos_GI(2) << ")\n";
    LOG(INFO) << "test | quaternion:(" << quat(0) << ", " << quat(1) << ", " << quat(2) << ", " << quat(3) << ")\n";
  }
}

void CTestUtility::ShowEigenVector(const std::string &prefix, const Eigen::Vector3d &vector_in) {
  LOG(INFO) << prefix << ":" << vector_in(0) << ", " << vector_in(1) << ", " << vector_in(2) << "\n";
}

void CTestUtility::ShowEigenMatrix(const std::string &prefix, const Eigen::Matrix3d &mat_in) {
  for (auto i = 0; i < 3; ++i) CTestUtility::ShowEigenVector(prefix, mat_in.row(i));
}
}  // namespace test
