/**
 * @file ObjectSize.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "object_size.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include "truth_base.h"
#include "scene.pb.h"

/// @brief car bounding box
std::map<int64_t, ObjectBBox> car_Bboxes;
/// @brief ego bounding box
std::map<int64_t, ObjectBBox> ego_Bboxes;
/// @brief dynamic bounding box
std::map<int64_t, ObjectBBox> dyn_Bboxes;
/// @brief static bounding box
std::map<int64_t, ObjectBBox> sta_Bboxes;

/**
 * @brief load all objects
 *
 * @param dir directory of the scenario
 * @return true on success
 * @return false on failure
 */
bool LoadObjectBBox(const std::string &buffer) {
  car_Bboxes.clear();
  dyn_Bboxes.clear();
  sta_Bboxes.clear();
  if (buffer.empty()) {
    return false;
  }
  // parse the protobuf message into a Scene instance
  sim_msg::Scene scene;
  if (!scene.ParseFromString(buffer)) {
    std::cout << "parse scene faild.";
    return false;
  }
  auto gen_bbox = [](const sim_msg::BoundingBox bbx) {
    ObjectBBox sz;
    sz.ceny = bbx.center().y();
    sz.cenz = bbx.center().z();
    sz.hei = bbx.higth();
    sz.wid = bbx.width();
    sz.len = bbx.length();
    return sz;
  };

  for (const auto &obj : scene.egos()) {
    if (obj.physicles().empty()) continue;
    auto &bbx = obj.physicles()[0].common();
    ObjectBBox sz = gen_bbox(bbx.bounding_box());
    sz.category = obj.category();
    if (obj.physicles().size() > 1) {
      std::cout << "not support now." << std::endl;
    }
    int id = std::atoi(obj.group().substr(obj.group().length() - 3).c_str());
    ego_Bboxes[id] = sz;
  }
  for (const auto &obj : scene.vehicles()) {
    auto &bbx = obj.physicle().common().bounding_box();
    ObjectBBox sz = gen_bbox(bbx);
    sz.category = obj.category();
    car_Bboxes[obj.physicle().common().model_id()] = sz;
  }

  for (const auto &obj : scene.vrus()) {
    auto &bbx = obj.physicle().common().bounding_box();
    ObjectBBox sz = gen_bbox(bbx);
    sz.category = obj.category();
    dyn_Bboxes[obj.physicle().common().model_id()] = sz;
  }
  for (const auto &obj : scene.miscs()) {
    auto &bbx = obj.physicle().bounding_box();
    ObjectBBox sz = gen_bbox(bbx);
    sz.category = obj.category();
    sta_Bboxes[obj.physicle().model_id()] = sz;
  }
  return true;
}

/**
 * @brief get bounding box center
 *
 * @param pos object position
 * @param heading object heading
 * @return object 's center
 */
Eigen::Vector3d ObjectBBox::getBboxCen(const Eigen::Vector3d &pos, double heading) {
  double x = cenx;
  double y = ceny;
  double z = cenz;
  rotate(x, y, heading);
  return pos + Eigen::Vector3d(x, y, z);
}

/**
 * @brief get bounding box points
 *
 * @param pos object position
 * @param heading object heading
 * @param scale scale factor
 * @return points in box
 */
std::vector<Eigen::Vector3d> ObjectBBox::getBboxPts(const Eigen::Vector3d &pos, double heading, double scale) const {
  std::vector<Eigen::Vector3d> bpts;

  const double offx[8] = {-0.5, 0.5, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5};
  const double offy[8] = {-0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5};
  const double offz[8] = {-0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5};
  double l = len * scale, w = wid * scale, h = hei * scale;
  for (int i = 0; i < 8; i++) {
    double x = offx[i] * l + cenx;
    double y = offy[i] * w + ceny;
    double z = offz[i] * h + cenz;
    rotate(x, y, heading);
    bpts.push_back(pos + Eigen::Vector3d(x, y, z));
  }
  return bpts;
}

/**
 * @brief get bounding box points
 *
 * @param pos object position
 * @param heading object heading
 * @param sample sample scale
 * @param scale scale factor
 * @param igore_inside igore inside points
 * @return points in box
 */
std::vector<Eigen::Vector3d> ObjectBBox::getBboxPts(const Eigen::Vector3d &pos, double heading, double sample,
                                                    double scale, bool igore_inside) const {
  std::vector<Eigen::Vector3d> bpts;
  double l = len * scale, w = wid * scale, h = hei * scale;
  for (int i = 0, is = std::ceil(h / sample); i <= is; ++i) {
    for (int j = 0, js = std::ceil(l / sample); j <= js; ++j) {
      for (int k = 0, ks = std::ceil(w / sample); k <= ks; ++k) {
        if (igore_inside && i != 0 && i != is && j != 0 && j != js && k != 0 && k != ks) {
          continue;
        }

        double dx = l * j / js - l * 0.5 + cenx;
        double dy = w * k / ks - w * 0.5 + ceny;
        double dz = h * i / is - h * 0.5 + cenz;

        rotate(dx, dy, heading);
        bpts.push_back(pos + Eigen::Vector3d(dx, dy, dz));
      }
    }
  }
  return bpts;
}
