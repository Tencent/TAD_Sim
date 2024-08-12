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
  // second
  bool load_contour(const std::string &dir);

  std::vector<Eigen::Vector3d> getBboxPts(std::pair<int, int> type, const Eigen::Vector3d &pos, double roll,
                                          double pitch, double yaw) const;

 private:
  struct Cata {
    struct Model {
      Eigen::Vector3d cen;
      double w = 0, h = 0, l = 0;
      std::string model;
    };
    std::vector<Model> models;
    std::string name;
    std::vector<Eigen::Vector3d> contour;
  };

  std::map<std::pair<int, int>, Cata> _objects;
  std::vector<Eigen::Vector3d> SamplingBboxPts(const Eigen::Vector3d &off, double len, double wid, double hei,
                                               double sample = 0.3, double scale = 1, bool igore_inside = true) const;
};

std::string findTypeFromUE(const std::string &type);
