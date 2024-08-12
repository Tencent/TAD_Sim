/**
 * @file TestMain.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "test_camera.h"
#include "test_common.h"
#include "test_map.h"
#include "test_utility.h"

int main(int argc, char **argv) {
  // hadmap path
  const std::string hadmap_path = "/home/dhu/.config/tadsim/scenario/hadmap/1002-1-015-190521.sqlite";

  if (argc == 4) {
    // get params
    std::string image_src_folder = argv[1];
    std::string image_out_folder = argv[2];
    std::string imurigid_csv_file = argv[3];

    LOG(INFO) << "test | input image folder " << image_src_folder << "\n";
    LOG(INFO) << "test | output image folder " << image_out_folder << "\n";
    LOG(INFO) << "test | imu rigid csv " << imurigid_csv_file << "\n";

    // prepare objects
    test::CTestUtility test_utility;
    test::CTestCamera test_camera;
    test::CTestMap test_map;

    // init objects
    test::ImageList image_list = test_utility.GetInputImages(image_src_folder);
    test::IMURigidDataSet imu_dataset = test_utility.GetIMURigid(imurigid_csv_file);

    LOG(INFO) << "main | image size:" << image_list.size() << "\n";
    LOG(INFO) << "main | imu size:" << imu_dataset.size() << "\n";

    // init hadmap
    test_map.InitHADMap(hadmap_path, 121.175972, 31.271755, 0.0);

    // utility
    test::CElapsedTime elasped_time;

    // projection
    for (auto image : image_list) {
      elasped_time.MarkStartTimePoint();

      LOG(INFO) << "======================================================\n";

      // print current image id
      LOG(INFO) << "test | current image id is " << image.first << "\n";

      // load image
      test::PixelPoints pixel_points;
      test_utility.LoadImage(image_list, image.first, pixel_points);

      // get imu index
      int imu_index = 4 * image.first / 40;

      LOG(INFO) << "test | current imu index*40 is " << imu_index * 40 << "\n";

      if (imu_index >= imu_dataset.size()) continue;

      // update imu data
      test::IMURigid &imu_data = imu_dataset.at(imu_index);

      // camera location and translation
      test_camera.SetPos_GI(imu_data._Pos_GI(0), imu_data._Pos_GI(1), imu_data._Pos_GI(2));
      test_camera.SetQuat_GI(imu_data._Quat_GI);
      test_camera.CalPos_GC();
      test_camera.CalRot_GC();

      // update camera location
      auto pos_gc = test_camera.GetPos_GC();
      test_map.SetCurLocENU(pos_gc(0), pos_gc(1), pos_gc(2));

      // get lines and projection
      if (test_map.CalNearByObjects(test_map.GetRadius())) {
        // get lane line
        test::CLineObjects line_objects = test_map.GetLineObjects();

        // sample points, used to be saved into pcd
        test::SamplePoints sample_points;

        // peojection line objects to camera
        for (auto line_object : line_objects) {
          test::CameraPoints camera_points;

          // calculate extrinsic matrix
          test_camera.CalExtrinsicMatrix();

          // world to camera
          test_camera.WorldPointsToCamera(line_object.sample_points, camera_points);

          // camera to pixel
          test_camera.CameraToPicture(camera_points, pixel_points);

          for (auto point : line_object.sample_points) sample_points.push_back(point);
        }

        // save image
        test_utility.SaveImage(image_out_folder, image.second, pixel_points);
        test_utility.SavePCD(image_out_folder, image.second + ".pcd", sample_points);
      }

      elasped_time.MarkEndTimePoint();

      elasped_time.PrintElapsedTime("main | one step of projection");
    }

    test_map.ReleaseHADMap();
  } else {
    LOG(ERROR) << "test | test_imu image_src_folder image_output_folder imurigid_csv\n";
  }
  return 0;
}
