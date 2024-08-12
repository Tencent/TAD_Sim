/**
 * @file Catalog.cpp
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-04-19
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */

#include "catalog.h"
#include <Eigen/Geometry>
#include <fstream>
#include <iostream>

Catalog::Catalog(/* args */) {}

Catalog::~Catalog() {}

Catalog &Catalog::getInstance() {
  static Catalog instance;
  return instance;
}

/**
 * @brief initialize the catalog from scene
 *
 * @param scene scene from reset
 * @return true
 * @return false
 */
bool Catalog::init(const sim_msg::Scene &scene) {
  _objects.clear();

  auto gen_cata = [](const sim_msg::PhysicleCommon &bbx) {
    Cata sz;
    sz.models.resize(1);
    sz.models[0].cen = Eigen::Vector3d(bbx.bounding_box().center().x(), bbx.bounding_box().center().y(),
                                       bbx.bounding_box().center().z());
    sz.models[0].h = bbx.bounding_box().higth();
    sz.models[0].l = bbx.bounding_box().length();
    sz.models[0].w = bbx.bounding_box().width();
    sz.models[0].model = bbx.model_3d();
    return sz;
  };

  for (const auto &obj : scene.egos()) {
    if (obj.physicles().empty()) continue;
    auto &bbx = obj.physicles()[0].common();
    Cata sz = gen_cata(bbx);
    sz.name = obj.name();
    if (obj.physicles().size() > 1) {
      for (size_t i = 1; i < obj.physicles().size(); i++) {
        auto &bbx2 = obj.physicles()[i];
        Cata::Model md;
        md.model = bbx2.common().model_3d();
        md.cen = Eigen::Vector3d(bbx2.common().bounding_box().center().x(), bbx2.common().bounding_box().center().y(),
                                 bbx2.common().bounding_box().center().z());
        md.h = bbx2.common().bounding_box().higth();
        md.l = bbx2.common().bounding_box().length();
        md.w = bbx2.common().bounding_box().width();
        md.cen += Eigen::Vector3d(bbx2.geometory().vehicle_coord().combination_offset().x(),
                                  bbx2.geometory().vehicle_coord().combination_offset().y(),
                                  bbx2.geometory().vehicle_coord().combination_offset().z());

        sz.models.push_back(md);
      }
    }
    int id = std::atoi(obj.group().substr(obj.group().length() - 3).c_str());
    _objects[std::make_pair(-1, id)] = sz;
  }
  for (const auto &obj : scene.vehicles()) {
    auto &bbx = obj.physicle().common();
    Cata sz = gen_cata(bbx);
    sz.name = obj.objectname();
    _objects[std::make_pair<int, int>(0, obj.physicle().common().model_id())] = sz;
  }

  for (const auto &obj : scene.vrus()) {
    auto &bbx = obj.physicle().common();
    Cata sz = gen_cata(bbx);
    sz.name = obj.objectname();
    _objects[std::make_pair<int, int>(2, obj.physicle().common().model_id())] = sz;
  }
  for (const auto &obj : scene.miscs()) {
    auto &bbx = obj.physicle();
    Cata sz = gen_cata(bbx);
    sz.name = obj.objectname();
    _objects[std::make_pair<int, int>(1, obj.physicle().model_id())] = sz;
  }
  return true;
}
