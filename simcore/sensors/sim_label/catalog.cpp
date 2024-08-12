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
#include <regex>

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

/**
 * @brief load contour from files
 *
 * @param dir dir to contour files
 * @return true
 * @return false
 */
bool Catalog::load_contour(const std::string &dir) {
  for (auto &obj : _objects) {
    for (auto &model : obj.second.models) {
      std::string path = dir + "/models/" + model.model + ".smp";
      std::ifstream in(path, std::ios::binary);
      if (in.is_open()) {
        in.seekg(0, std::ios::end);
        size_t num_points = in.tellg() / (3 * sizeof(float));
        in.seekg(0, std::ios::beg);
        std::vector<float> values(3 * num_points);
        in.read(reinterpret_cast<char *>(values.data()), 3 * num_points * sizeof(float));
        in.close();
        // test geometry center
        // calculate bbox
        float min_x = std::numeric_limits<float>::max();
        float max_x = std::numeric_limits<float>::min();
        float min_y = std::numeric_limits<float>::max();
        float max_y = std::numeric_limits<float>::min();
        float min_z = std::numeric_limits<float>::max();
        float max_z = std::numeric_limits<float>::min();
        for (size_t i = 0; i < num_points; i++) {
          // y-z flip
          float x = values[i * 3];
          float y = -values[i * 3 + 2];
          float z = values[i * 3 + 1];
          min_x = std::min(min_x, x);
          max_x = std::max(max_x, x);
          min_y = std::min(min_y, y);
          max_y = std::max(max_y, y);
          min_z = std::min(min_z, z);
          max_z = std::max(max_z, z);
        }
        Eigen::Vector3d off = model.cen - Eigen::Vector3d(min_x + max_x, min_y + max_y, min_z + max_z) * 0.5;
        for (size_t i = 0; i < num_points; i++) {
          // y-z flip
          obj.second.contour.push_back(off + Eigen::Vector3d(values[i * 3], -values[i * 3 + 2], values[i * 3 + 1]));
        }
      } else {
        std::cout << obj.second.name << " smp faild: using bbox\n";
        auto pts = SamplingBboxPts(model.cen, model.l, model.w, model.h);
        obj.second.contour.insert(obj.second.contour.end(), pts.begin(), pts.end());
      }

      /*
      std::ofstream test(path + ".txt");
      for (const auto &p : obj.second.contour) {
        test << p.x() << " " << p.y() << " " << p.z() << std::endl;
      }
      //*/
    }
  }
  return true;
}

/**
 * @brief Generates bounding box points around a specified origin point based
 * on
 *
 * @param pos position of the object
 * @param rot rotation of the object
 * @return std::vector<Eigen::Vector3d>
 */
std::vector<Eigen::Vector3d> Catalog::getBboxPts(std::pair<int, int> type, const Eigen::Vector3d &pos, double roll,
                                                 double pitch, double yaw) const {
  if (_objects.find(type) == _objects.end()) {
    return std::vector<Eigen::Vector3d>();
  }
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  Eigen::Matrix3d R = q.toRotationMatrix();
  std::vector<Eigen::Vector3d> bpts;
  const auto &obj = _objects.at(type);
  for (const auto &p : obj.contour) {
    Eigen::Vector3d p_ = R * p;
    p_ += pos;
    bpts.push_back(p_);
  }
  return bpts;
}

/**
 * @brief generates bounding box points around a specified origin point based
 * on length, width, height, heading angle, sampling distance, scaling factor,
 * and optional flag to ignore points inside the bbox.It utilizes the Eigen
 * library for matrix operations and vector math.
 *
 * @param len length of the object
 * @param wid width of the object
 * @param hei height of the object
 * @param sample sample rate in meters per step along each axis
 * @param scale scaling factor of the object
 * @param igore_inside ignore points inside the bbox
 * @return std::vector<Eigen::Vector3d> inside the bounding box
 */
std::vector<Eigen::Vector3d> Catalog::SamplingBboxPts(const Eigen::Vector3d &off, double len, double wid, double hei,
                                                      double sample, double scale, bool igore_inside) const {
  std::vector<Eigen::Vector3d> bpts;
  double l = len * scale, w = wid * scale, h = hei * scale;
  for (int i = 0, is = std::ceil(h / sample); i <= is; ++i) {
    for (int j = 0, js = std::ceil(l / sample); j <= js; ++j) {
      for (int k = 0, ks = std::ceil(w / sample); k <= ks; ++k) {
        if (igore_inside && i != 0 && i != is && j != 0 && j != js && k != 0 && k != ks) {
          continue;
        }
        double dx = l * j / js - l * 0.5;
        double dy = w * k / ks - w * 0.5;
        double dz = h * i / is - h * 0.5;
        bpts.push_back(off + Eigen::Vector3d(dx, dy, dz));
      }
    }
  }
  return bpts;
}

/**
 * @brief type in ue to type
 *
 *
 * @param type
 * @return std::string
 */
std::string findTypeFromUE(const std::string &type) {
  const std::map<std::string, std::string> typeNameMap = {
      {"transport/Sedan", "car"},        {"transport/SUV", "car"},
      {"transport/Car", "car"},          {"transport/MPV", "car"},
      {"transport/Truck", "truck"},      {"transport/AIV", "truck"},
      {"transport/Bus", "bus"},          {"creature/pedestrian.", "pedestrian"},
      {"creature/bike", "bike"},         {"creature/elecBike", "bike"},
      {"creature/tricycle", "bike"},     {"creature/moto", "bike"},
      {"creature/Honda", "bike"},        {"creature/Animal", "animal"},
      {"creature/vendingCar", "car"},    {"obstacle.Sedan", "car"},
      {"obstacle.Person", "pedestrian"}, {"obstacle.box", "box"},
      {"obstacle.Cone", "cone"}};

  for (const auto &tn : typeNameMap) {
    std::regex reg(tn.first);
    if (std::regex_search(type, reg)) {
      return tn.second;
    }
  }
  return std::string("other");
}
