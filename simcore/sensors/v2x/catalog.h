// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <Eigen/Core>
#include <string>
#include "scene.pb.h"

class Catalog {
 public:
  Catalog();
  ~Catalog();

  static Catalog &getInstance();

  // first
  bool init(const sim_msg::Scene &);

  struct Cata {
    struct Model {
      Eigen::Vector3d cen;
      double w = 0, h = 0, l = 0;
      std::string model;
    };
    std::vector<Model> models;
    std::string name;
  };

  std::map<std::pair<int, int>, Cata> _objects;
};
