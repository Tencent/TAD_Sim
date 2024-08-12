/**
 * @file ImageLabel.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-30
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "image_label.h"
#include <Eigen/Eigen>
#include "camera_sensor.h"
#include "catalog.h"
#include "fisheye_sensor.h"
#include "visable_calculate.h"
#include "common/coord_trans.h"

ImageLabel::ImageLabel(const ImagePackage &_info) : info(_info) {}

ImageLabel::~ImageLabel() {}

void ImageLabel::init(double minArea, double maxDistance, double completeness, bool fullBox) {
  std::shared_ptr<CameraSensor> camerasensor;
  if (info.type == sim_msg::SensorRaw_Type_TYPE_CAMERA && cameras.find(info.image.id) != cameras.end()) {
    camerasensor = cameras[info.image.id];
  } else if (info.type == sim_msg::SensorRaw_Type_TYPE_SEMANTIC && semantics.find(info.image.id) != semantics.end()) {
    camerasensor = semantics[info.image.id];
  } else if (info.type == sim_msg::SensorRaw_Type_TYPE_FISHEYE && fisheyes.find(info.image.id) != fisheyes.end()) {
    camerasensor = fisheyes[info.image.id];
  }

  // calculate uv of objects
  if (camerasensor) {
    auto &dobj = info.obj;
    // Update position and rotation of the camera based on pose message
    camerasensor->setPosition(Eigen::Vector3d::Zero());
    camerasensor->setRotation(info.image.pose.roll, info.image.pose.pitch, info.image.pose.yaw);

    // Convert display objects into visible buffers
    std::vector<std::pair<int, sim_msg::DisplayPose::Object>> dobjects;
    for (const auto &car : dobj.egos()) {
      dobjects.push_back(std::make_pair(-1, car));
    }
    for (const auto &car : dobj.cars()) {
      dobjects.push_back(std::make_pair(0, car));
    }
    for (const auto &sta : dobj.staticobstacles()) {
      dobjects.push_back(std::make_pair(1, sta));
    }
    for (const auto &dyn : dobj.dynamicobstacles()) {
      dobjects.push_back(std::make_pair(2, dyn));
    }
    // Create visible buffer containers
    std::vector<VisableBuffer> vbs;
    // For each display object, calculate its bounding box vertices
    for (const auto &dobj : dobjects) {
      Eigen::Vector3d pos(dobj.second.pose().longitude(), dobj.second.pose().latitude(), dobj.second.pose().altitude());
      coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), info.image.pose.x, info.image.pose.y, info.image.pose.z);
      // Filter out objects outside maximum range
      double distance = pos.norm();
      if (distance > maxDistance) {
        continue;
      }

      // Calculate UVs for visible pixels within bounding box
      points uvs;
      auto pts = Catalog::getInstance().getBboxPts(
          std::make_pair(dobj.first, dobj.first == -1 ? dobj.second.id() : dobj.second.raw_type()), pos,
          dobj.second.pose().roll(), dobj.second.pose().pitch(), dobj.second.pose().yaw());
      for (const auto &p : pts) {
        if (!camerasensor->inFov(p)) continue;
        auto uv = camerasensor->world2pixel(p);
        if (uv.x() > 0 && uv.x() < camerasensor->W() && uv.y() > 0 && uv.y() < camerasensor->H()) {
          uvs.push_back(point(uv.x(), uv.y()));
        }
      }
      if (uvs.size() * 10 < pts.size()) {
        continue;
      }

      // Initialize visible buffer with calculated UV coordinates
      VisableBuffer vb;
      if (vb.init(uvs)) {
        vb.tag = 0;
        vb.rawPtr = static_cast<const void *>(&dobj);
        vb.distance = distance;
        vbs.push_back(vb);
      }
    }
    // Call re-visible method to update visibility status
    ReVisable(vbs);
    // Save detection results only when area condition satisfied
    for (const auto &vb : vbs) {
      if (minArea > 0. && completeness > 0.) {
        bool visable = (vb.area >= minArea) && (vb.area0 > 0 && (vb.area / vb.area0) > completeness);
        if (!visable) {
          continue;
        }
      }

      const std::pair<int, sim_msg::DisplayPose::Object> &dobj =
          *static_cast<const std::pair<int, sim_msg::DisplayPose::Object> *>(vb.rawPtr);

      TempObject tobj;
      tobj.t = dobj.first;
      tobj.obj = dobj.second;
      // Generate convex hull polygons using Boost Geometry library
      polygon ply;
      if (fullBox)
        ply = vb.bounds0;
      else
        boost::geometry::convex_hull(vb.bounds, ply);
      // Add all vertex pairs into outline vector
      for (const auto &p : ply.outer()) {
        tobj.outline.push_back(std::make_pair(p.get<0>(), p.get<1>()));
      }
      // Push processed outline back into detection_objects container
      detection_objects.push_back(tobj);
    }
  }
}

Json::Value ImageLabel::label(std::string &dir0, std::string &dir1) {
  const auto &jpginfo = info.image;
  // Enqueue tasks to save JPEG and PNG files
  dir0 = "camera";
  dir1 = "camera/jpg";
  if (info.type == sim_msg::SensorRaw_Type_TYPE_SEMANTIC) {
    dir0 = "semantic";
    dir1 = "semantic/png";
  } else if (info.type == sim_msg::SensorRaw_Type_TYPE_FISHEYE) {
    dir0 = "fisheye";
    dir1 = "fisheye/jpg";
  }

  Json::Value source;
  Json::Value &label = source["openlabel"];
  // metadata
  label["metadata"]["schema_version"] = "1.0.0";
  // coordinate_systems
  std::string sensor_cs = dir0 + std::to_string(info.image.id);
  label["coordinate_systems"]["geospatial-wgs84"]["type"] = "geo";
  label["coordinate_systems"]["geospatial-wgs84"]["parent"] = "";
  label["coordinate_systems"]["geospatial-wgs84"]["children"].append(sensor_cs);
  Json::Value &cs = label["coordinate_systems"][sensor_cs];
  cs["type"] = "geo";
  cs["parent"] = "geospatial-wgs84";
  cs["children"].resize(0);
  // euler_angles
  cs["pose_wrt_parent"]["euler_angles"].append(info.image.pose.roll);
  cs["pose_wrt_parent"]["euler_angles"].append(info.image.pose.pitch);
  cs["pose_wrt_parent"]["euler_angles"].append(info.image.pose.yaw);
  // translation
  cs["pose_wrt_parent"]["translation"].append(info.image.pose.x);
  cs["pose_wrt_parent"]["translation"].append(info.image.pose.y);
  cs["pose_wrt_parent"]["translation"].append(info.image.pose.z);
  // order
  cs["pose_wrt_parent"]["sequence"] = "zyx";
  // streams
  Json::Value &stream = label["streams"][sensor_cs];
  stream["type"] = "camera";
  stream["description"] = sensor_cs;
  stream["uri"] = dir1 + "/" + jpginfo.fpath;

  // objects
  Json::Value &objects = label["objects"];
  for (const auto &detection : detection_objects) {
    int id = detection.obj.id();
    std::string name = "car";
    if (detection.t == 1) {
      id += 10000;
      name = "static obstacle";
    } else if (detection.t == 2) {
      id = 20000 - id;
      name = "dynamic obstacle";
    }
    Json::Value &obj = objects[std::to_string(id)];
    obj["type"] = findTypeFromUE(detection.obj.type());
    obj["name"] = name;
    obj["coordinate_system"] = sensor_cs;
    obj["object_data"]["poly2d"].resize(1);
    Json::Value &odata = obj["object_data"]["poly2d"][0];
    odata["closed"] = true;
    odata["name"] = "";
    odata["mode"] = "";
    // outline
    for (const auto &p : detection.outline) {
      odata["val"].append(p.first);
      odata["val"].append(p.second);
    }
  }

  return source;
}
