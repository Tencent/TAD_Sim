/**
 * @file ImageLabel.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-30
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once

#include <vector>
#include "data_queue.h"
#include "json/json.h"
#include "sensor_raw.pb.h"

class ImageLabel {
 public:
  explicit ImageLabel(const ImagePackage&);
  ~ImageLabel();

  void init(double minArea, double maxDistance, double completeness, bool fullBox);
  Json::Value label(std::string& dir0, std::string& dir1);

 private:
  struct TempObject {
    int t;
    sim_msg::DisplayPose::Object obj;
    std::vector<std::pair<double, double>> outline;
  };
  std::vector<TempObject> detection_objects;
  const ImagePackage& info;
};
