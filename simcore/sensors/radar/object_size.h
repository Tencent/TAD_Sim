/**
 * @file ObjectSize.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <map>
#include <string>
#include "Eigen/Eigen"

struct ObjectBBox {
  int64_t id = 0;
  double cenx = 0;
  double ceny = 0;
  double cenz = 0;
  double len = 5;
  double wid = 2;
  double hei = 2;
  int category = -1;

  Eigen::Vector3d getBboxCen(const Eigen::Vector3d &pos, double heading);
  std::vector<Eigen::Vector3d> getBboxPts(const Eigen::Vector3d &pos, double heading, double scale = 1) const;
  std::vector<Eigen::Vector3d> getBboxPts(const Eigen::Vector3d &pos, double heading, double sample, double scale = 1,
                                          bool igore_inside = true) const;
};

extern std::map<int64_t, ObjectBBox> ego_Bboxes;
extern std::map<int64_t, ObjectBBox> car_Bboxes;
extern std::map<int64_t, ObjectBBox> dyn_Bboxes;
extern std::map<int64_t, ObjectBBox> sta_Bboxes;

// Catalogs dir
bool LoadObjectBBox(const std::string &buffer);
