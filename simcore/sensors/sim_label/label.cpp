/**
 * @file label.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "label.h"
#include <stdio.h>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <regex>
#include "camera_sensor.h"
#include "catalog.h"
#include "fisheye_sensor.h"
#include "image_label.h"
#include "lidar_sensor.h"
#include "visable_calculate.h"
#include "common/coord_trans.h"
#include "google/protobuf/util/json_util.h"
#include "json/json.h"
#include "osi_datarecording.pb.h"
#include "scene.pb.h"
#include "sensor_raw.pb.h"


/**
 * @brief define FS_TRY
 * @param f : the function to try
 */
#define FS_TRY(f)                       \
  try {                                 \
    f;                                  \
  } catch (const std::exception &e) {   \
    std::cout << e.what() << std::endl; \
  }

/**
 * @brief Construct a new sim label::sim label object
 *
 */
sim_label::sim_label() {}

/**
 * @brief Destroy the sim label::sim label object
 *
 */
sim_label::~sim_label() {}

/**
 * @brief init the label
 *
 * @param helper : the helper of tx_sim
 */
void sim_label::Init(tx_sim::InitHelper &helper) {
  // subscribe sensor truth
  helper.Subscribe("TXSIM_SENSOR_OBJECT");

  // choose the device ids to recoard. default all devices are selected.
  device = helper.GetParameter("-device");
  if (device == "all") {
    device.clear();
  }

  // set the num of displays
  // default is 30
  // we will subcribe the topic "DISPLAYSENSOR_{id}"
  auto NumOfDisplay = helper.GetParameter("NumOfDisplay");
  if (!NumOfDisplay.empty()) {
    disNum = std::atoi(NumOfDisplay.c_str());
  }
  // set the path of the data saved.
  if (!helper.GetParameter("DataSavePath").empty()) savePathBase = helper.GetParameter("DataSavePath");

  // subcribe display topic from 0 to NumOfDisplay
  for (int i = 0; i < disNum; i++) {
    helper.Subscribe(std::string("DISPLAYSENSOR_") + std::to_string(i));
    helper.Subscribe(std::string("DISPLAYPOSE_") + std::to_string(i));
  }
  // need create scenario dir or not?
  auto tmp = helper.GetParameter("CreateScenarioDir");
  if (tmp == "0" || tmp == "false" || tmp == "disable") saveScenarioDir = false;

  tmp = helper.GetParameter("FullBox");
  if (tmp == "1" || tmp == "true" || tmp == "enable") fullBox = true;

  tmp = helper.GetParameter("DebugFiles");
  if (tmp == "1" || tmp == "true" || tmp == "enable") debugFiles = true;
  config_dir = helper.GetParameter(tx_sim::constant::kInitKeyModuleSharedLibDirectory);
  // write path to logs
  // std::cout << "savePathBase=" << savePathBase << std::endl;
}

/**
 * @brief reset the label
 *
 * @param helper the helper of tx_sim
 */
void sim_label::Reset(tx_sim::ResetHelper &helper) {
  // Load the sensor configuration
  getSensorConfig(helper.scene_pb(), helper.group_name());
  // Output the number of cameras and lidars loaded
  std::cout << "Load " << semantics.size() << " semantic." << std::endl;
  std::cout << "Load " << cameras.size() << " cameras." << std::endl;
  std::cout << "Load " << lidars.size() << " lidars." << std::endl;
  std::cout << "Load " << fisheyes.size() << " fisheyes." << std::endl;

  // Initialize the save paths
  savePath = savePathBase;
  if (savePath.back() == '/' || savePath.back() == '\\') {
    savePath.pop_back();
  }
  // If the saveScenarioDir flag is set, create a new subdirectory for the
  // scenario
  if (saveScenarioDir) {
    auto scenario = boost::filesystem::path(helper.scenario_file_path()).stem().string();
    savePath += "/";
    savePath += scenario;
  }
  // if (boost::filesystem::exists(savePath)) {
  //   FS_TRY(boost::filesystem::rename(savePath,
  //                                    savePath + "_" +
  //                                    std::to_string(rand())));
  // }
  // create ego dir
  savePath += "/";
  savePath += helper.group_name();
  // Create the directories for storing data in the savePath
  FS_TRY(boost::filesystem::create_directories(savePath + "/lidar/pcd"));
  FS_TRY(boost::filesystem::create_directories(savePath + "/camera/jpg"));
  FS_TRY(boost::filesystem::create_directories(savePath + "/semantic/png"));
  FS_TRY(boost::filesystem::create_directories(savePath + "/fisheye/jpg"));

  // Output the savePath
  std::cout << "savePath=" << savePath << std::endl;

  // Initialize the thread pool and data queue
  threads = std::make_shared<ThreadPool>(8);
  queues = std::make_shared<DataQueue>();
  queues->setImageCallback(std::bind(&sim_label::saveImageLabel, this, std::placeholders::_1));

  queues->setPcdCallback(std::bind(&sim_label::savePcdLabel, this, std::placeholders::_1));

  if (!semantics.empty()) {
    saveMaskJson(savePath + "/semantic/mask.json");
  }

  ego_id = std::atoi(helper.group_name().substr(helper.group_name().length() - 3).c_str());
  sim_msg::Scene scene;
  scene.ParseFromString(helper.scene_pb());
  // std::cout << scene.DebugString();
  Catalog::getInstance().init(scene);
  Catalog::getInstance().load_contour(config_dir);
}

/**
 * @brief timestamp to string
 *
 * @param timestamp The timestamp to be converted
 */
template <class T>
std::string timeStarmString(T timestamp) {
  // Create a string stream object to store the result
  std::stringstream ss;
  ss << std::setw(10) << std::setfill('0') << (std::int64_t)(timestamp);
  return ss.str();
}

/**
 * @brief Step method implementation for SimLabel class
 *
 * @param helper helper of txsim
 */
void sim_label::Step(tx_sim::StepHelper &helper) {
  // Initialize a map to keep track of timestamps for displaying objects
  static std::map<int, double> display_timstamp;
  if (helper.timestamp() == 0) {
    display_timstamp.clear();
  }

  // Print current simulation timestamp followed by a colon
  std::cout << helper.timestamp() << ": ";

  // Check if truth mode is enabled
  // Get subscribed messages from various sources
  // Process Display Pose messages
  sim_msg::DisplayPose trafficPose_all;
  std::set<std::int64_t> egoid, carid, staid, dynid;
  // Iterate over DISPLAYPOSE_* channels
  for (int i = 0; i < disNum; i++) {
    std::string payload_;
    helper.GetSubscribedMessage(std::string("DISPLAYPOSE_") + std::to_string(i), payload_);

    // Deserialize incoming message
    sim_msg::DisplayPose trafficPose;
    if (payload_.empty() || !trafficPose.ParseFromString(payload_)) continue;
    // Update the timestamp field
    trafficPose_all.set_timestamp(trafficPose.timestamp());
    // Extract unique EGO IDs,
    for (const auto &obj : trafficPose.egos()) {
      if (obj.id() == ego_id) continue;
      if (egoid.find(obj.id()) == egoid.end()) {
        *trafficPose_all.add_egos() = obj;
        egoid.insert(obj.id());
      }
    }
    // Extract unique CAR IDs,
    for (const auto &obj : trafficPose.cars()) {
      if (carid.find(obj.id()) == carid.end()) {
        *trafficPose_all.add_cars() = obj;
        carid.insert(obj.id());
      }
    }
    // Extract unique STATIONARY OBJECT IDs
    for (const auto &obj : trafficPose.staticobstacles()) {
      if (staid.find(obj.id()) == staid.end()) {
        *trafficPose_all.add_staticobstacles() = obj;
        staid.insert(obj.id());
      }
    }
    // Extract unique DYNAMIC OBJECT IDs
    for (const auto &obj : trafficPose.dynamicobstacles()) {
      if (dynid.find(obj.id()) == dynid.end()) {
        *trafficPose_all.add_dynamicobstacles() = obj;
        dynid.insert(obj.id());
      }
    }
  }
  // Send processed Display Pose message back out via queue
  std::cout << "[" << trafficPose_all.timestamp() << ": " << trafficPose_all.egos_size() << " ego | "
            << trafficPose_all.cars_size() << " car | " << trafficPose_all.staticobstacles_size() << " static | "
            << trafficPose_all.dynamicobstacles_size() << " dynamic] ";
  queues->addObject(trafficPose_all);

  // Handle updates to displayed objects' ids
  std::map<int, std::set<std::int64_t>> display_ids;
  for (int i = 0; i < disNum; i++) {
    std::string payload_;
    helper.GetSubscribedMessage(std::string("DISPLAYSENSOR_") + std::to_string(i), payload_);
    sim_msg::SensorRaw sensorraw;
    if (payload_.empty() || !sensorraw.ParseFromString(payload_)) continue;

    // time is not now
    if (sensorraw.timestamp() == display_timstamp[i]) {
      continue;
    }
    display_timstamp[i] = sensorraw.timestamp();

    // Output channel index and timestamp
    std::cout << "[" << i << ", " << sensorraw.timestamp() << ": ";
    // Process raw sensor data
    for (const auto &sensor : sensorraw.sensor()) {
      // Add parsed image information to queue
      if (sensor.type() == sim_msg::SensorRaw::TYPE_CAMERA) {
        ImageInfo info;
        if (parseImage(sensor.raw(), info)) {
          std::cout << "camera(" << info.id << "," << info.timestamp << ")=" << info.size;
          queues->addCamera(info);
        }
      } else if (sensor.type() == sim_msg::SensorRaw::TYPE_SEMANTIC) {
        ImageInfo info;
        if (parseImage(sensor.raw(), info)) {
          std::cout << "semantic(" << info.id << "," << info.timestamp << ")=" << info.size;
          queues->addSenmantic(info);
        }
      } else if (sensor.type() == sim_msg::SensorRaw::TYPE_FISHEYE) {
        ImageInfo info;
        if (parseImage(sensor.raw(), info)) {
          std::cout << "fisheye(" << info.id << "," << info.timestamp << ")=" << info.size;
          queues->addFisheye(info);
        }
      } else if (sensor.type() == sim_msg::SensorRaw::TYPE_LIDAR) {
        PcInfo info;
        if (parseLidar(sensor.raw(), info)) {
          std::cout << "lidar(" << info.id << "," << info.timestamp << ")=" << info.size;
          queues->addLidar(info);
        }
      }
      std::cout << ", ";
    }
    // Close square brackets and comma after processing sensors
    std::cout << "], ";
  }
  queues->update();
  // Clear the output buffer and print newline character
  std::cout << "\n";
}

/**
 * @brief Stop method implementation for SimLabel class
 *
 * @param helper helper of tx_sim
 */
void sim_label::Stop(tx_sim::StopHelper &helper) {
  // Reset thread pool and queues
  queues.reset();
  threads.reset();

  // Acquire save mutex before clearing saved_files vector
  {
    std::unique_lock<std::mutex> lock(save_mutex);
    saved_files.clear();
  }

  // Log stop event
  std::cout << "stop.\n";
}

/**
 * @brief Parse image sensor data into appropriate format
 *
 * @param prefix Prefix of image file name
 * @param buf Raw sensor data
 * @return true on success
 * @return false
 */
bool sim_label::parseImage(const std::string &buf, ImageInfo &info) {
  // sensor proto
  sim_msg::CameraRaw camera;
  if (!camera.ParseFromString(buf)) {
    std::cout << "camera error";
    return false;
  }
  // the path to write
  auto ffnm = timeStarmString(camera.timestamp()) + "_" + std::to_string(camera.id());
  if (camera.type() == "JPEG") {
    ffnm += ".jpg";
  } else if (camera.type() == "PNG") {
    ffnm += ".png";
  } else if (camera.type() == "EXR") {
    return false;
    ffnm += ".exr";
  } else {
    return false;
  }

  // the info of image
  info.id = camera.id();
  info.fpath = ffnm;
  info.size = camera.image_data().size();
  info.timestamp = static_cast<std::int64_t>(camera.timestamp());
  info.height = camera.height();
  info.width = camera.width();
  info.buffer = camera.image_data();
  info.pose.x = camera.pose().longitude();
  info.pose.y = camera.pose().latitude();
  info.pose.z = camera.pose().altitude();
  info.pose.roll = camera.pose().roll();
  info.pose.pitch = camera.pose().pitch();
  info.pose.yaw = camera.pose().yaw();
  return true;
}

/**
 * @brief lidar from pb string
 *
 * @param prefix Prefix of lidar file name
 * @param buf Raw sensor data
 * @return true
 * @return false
 */
bool sim_label::parseLidar(const std::string &buf, PcInfo &info) {
  sim_msg::LidarRaw lidar;
  if (!lidar.ParseFromString(buf)) {
    std::cout << "lidar error";
    return false;
  }
  std::int64_t timestamp = static_cast<std::int64_t>((lidar.timestamp_begin() + lidar.timestamp_end()) * 0.5);
  // the path of pcd file to be saved
  auto ffnm = timeStarmString(timestamp) + "_" + std::to_string(lidar.id()) + ".pcd";

  size_t num = lidar.count();

  std::stringstream pcdbuf;
  // write the header of pcd format file
  pcdbuf << "# .PCD v.7 - Point Cloud Data file format\n";
  pcdbuf << "VERSION 0.7\n";
  pcdbuf << "FIELDS x y z intensity label\n";
  pcdbuf << "SIZE 4 4 4 4 4\n";
  pcdbuf << "TYPE F F F F I\n";
  pcdbuf << "COUNT 1 1 1 1 1\n";
  pcdbuf << "WIDTH " << num << "\n";
  pcdbuf << "HEIGHT 1\n";
  pcdbuf << "VIEWPOINT 0 0 0 1 -1 0 0\n";
  pcdbuf << "POINTS " << num << "\n";
  pcdbuf << "DATA binary\n";

  // write point raw
  if (lidar.point_lists().size() > 0) {
    pcdbuf.write(lidar.point_lists().data(), lidar.point_lists().size());
  } else {
    // or write x y z i by order
#pragma pack(push, 4)
    struct TMP {
      float x = 0;
      float y = 0;
      float z = 0;
      float i = 0;          // intensity
      std::uint32_t t = 0;  // type
    };
#pragma pack(pop)

    std::vector<TMP> datas;
    datas.reserve(num);
    for (const auto &p : lidar.points()) {
      TMP tmp;
      tmp.x = p.x();
      tmp.y = p.y();
      tmp.z = p.z();
      tmp.i = p.i();
      tmp.t = p.t();
      datas.push_back(tmp);
    }
    pcdbuf.write(reinterpret_cast<const char *>(datas.data()), 5 * num * sizeof(float));
  }

  // the info of point
  info.id = lidar.id();
  info.fpath = ffnm;
  info.size = pcdbuf.str().length();
  info.timestamp = timestamp;
  info.fpath = ffnm;
  info.count = lidar.count();
  info.buffer = pcdbuf.str();
  info.coord_type = lidar.coord_type();
  info.pose.x = lidar.pose_last().longitude();
  info.pose.y = lidar.pose_last().latitude();
  info.pose.z = lidar.pose_last().altitude();
  info.pose.roll = lidar.pose_last().roll();
  info.pose.pitch = lidar.pose_last().pitch();
  info.pose.yaw = lidar.pose_last().yaw();
  info.timestamp_bg = lidar.timestamp_begin();
  info.timestamp_ed = lidar.timestamp_end();
  return true;
}

/**
 * @brief Save file method implementation for SimLabel class
 *
 * @param fname The name of the file to be saved
 * @param buf The binary data to be saved
 * @return true
 * @return false
 */
bool sim_label::saveFile(const std::string &fname, const std::string &buf) {
  {
    // Lock save mutex before checking if filename already exists in saved files
    // set
    std::unique_lock<std::mutex> lock(save_mutex);
    if (saved_files.find(fname) != saved_files.end()) {
      return true;
    }
    // Update saved files set after successfully saving file
    saved_files.insert(fname);
  }

  std::cout << "save " << fname;
  // Open output stream for writing binary data to disk
  std::fstream of(savePath + "/" + fname, std::ios::out | std::ios::binary);
  if (of.is_open()) {
    of.write(buf.c_str(), buf.size());
    of.close();
    std::cout << " done.";
  } else {
    std::cout << " fail.";
  }

  // Log successful save operation
  std::cout << std::endl;
  return true;
}

/**
 * @brief This code snippet demonstrates how to use C++ Boost libraries to
 * extract relevant information from an image package, generate necessary
 * metadata, and enqueue tasks to save both JPEG and PNG files alongside their
 * respective JSON files. The main steps include extracting timestamps, getting
 * UTC dates, binding functions to save files, defining temporary structures,
 * checking input types, iterating over sensors, processing detected objects'
 * outlines, calculating UVs for visible pixels within bounding boxes,
 * initializing visible buffer with calculated UV coordinates, calling
 * `ReVisable` method to update visibility status, generating convex hull
 * polygons using Boost Geometry library, creating JSON metadata, enqueuing
 * tasks to save JPEG and PNG files, constructing JSON source object, appending
 * related files to dataset, writing JSON output streams, and finally testing
 * the functionality by exporting GeoJSON format of extracted objects.
 *
 * @param info The image package to be processed
 */
void sim_label::saveImageLabel(const ImagePackage &info) {
  // Get timestamp string and UTC date/time string from image package
  std::string tss = timeStarmString(info.image.timestamp);
  std::string utc = getUTC();
  // Bind a function that saves file with given key-value pair
  auto sfun = std::bind(&sim_label::saveFile, this, std::placeholders::_1, std::placeholders::_2);

  // Define temporary object structure used in saving data

  ImageLabel label(info);
  label.init(minArea, maxDistance, completeness, fullBox);

  std::string dir0, dir1;
  // Construct JSON source object
  Json::Value source = label.label(dir0, dir1);

  const auto &jpginfo = info.image;
  threads->enqueue(sfun, dir1 + "/" + jpginfo.fpath, jpginfo.buffer);
  // save json object as string
  Json::StreamWriterBuilder builder;
  builder["commentStyle"] = "None";
  builder["indentation"] = "";
  const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream oss;
  writer->write(source, &oss);
  // save the file in queue
  threads->enqueue(sfun, dir0 + "/" + tss + "_" + std::to_string(jpginfo.id) + ".json", oss.str());

  // save geojson for debug
  // you can drop geojson in QGIS for a look
  if (debugFiles) {
    Json::Value geojson;
    geojson["type"] = "FeatureCollection";
    for (const auto &key : source["openlabel"]["objects"].getMemberNames()) {
      const Json::Value &obj = source["openlabel"]["objects"][key];
      const Json::Value &odata = obj["object_data"]["poly2d"][0]["val"];
      Json::Value feature;
      feature["type"] = "Feature";
      feature["geometry"]["type"] = "LineString";
      auto pn = odata.size() / 2;
      for (std::uint32_t i = 0; i < pn; i++) {
        Json::Value jp;
        jp.append(odata[i * 2].asDouble());
        jp.append(-(odata[i * 2 + 1].asDouble()));
        feature["geometry"]["coordinates"].append(jp);
      }
      geojson["features"].append(feature);
    }
    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::stringstream oss;
    writer->write(geojson, &oss);
    threads->enqueue(sfun, dir1 + "/" + jpginfo.fpath + ".json", oss.str());
  }
}

/**
 * @brief save pcd label
 *
 * @param info pcd info
 */
void sim_label::savePcdLabel(const PcdPackage &info) {
  // Get timestamp string and UTC date/time string from lidar package
  std::string tss = timeStarmString(info.lidar.timestamp);
  std::string utc = getUTC();

  // Define temporary object structure used in saving data
  auto sfun = std::bind(&sim_label::saveFile, this, std::placeholders::_1, std::placeholders::_2);

  // find out these object in lidar view
  struct TempObject {
    int t;
    sim_msg::DisplayPose::Object obj;
    double x, y, z;
    double roll, pitch, yaw;
  };
  std::vector<TempObject> detect_objects;

  // read object
  const auto &obj_ops = info.obj;
  // hand object from display
  if (lidars.find(info.lidar.id) == lidars.end()) {
    std::cout << "cannot find lidar in sensor config. id = " << info.lidar.id << std::endl;
  } else {
    // found out these visable object
    auto &lidar = lidars[info.lidar.id];
    lidar->setCarPosition(Eigen::Vector3d::Zero());
    lidar->setCarRotation(info.lidar.pose.roll, info.lidar.pose.pitch, info.lidar.pose.yaw);
    std::vector<std::pair<int, sim_msg::DisplayPose::Object>> dobjects;
    for (const auto &car : obj_ops.egos()) {
      dobjects.push_back(std::make_pair(-1, car));
    }
    for (const auto &car : obj_ops.cars()) {
      dobjects.push_back(std::make_pair(0, car));
    }
    for (const auto &sta : obj_ops.staticobstacles()) {
      dobjects.push_back(std::make_pair(1, sta));
    }
    for (const auto &dyn : obj_ops.dynamicobstacles()) {
      dobjects.push_back(std::make_pair(2, dyn));
    }

    for (const auto &dobj : dobjects) {
      Eigen::Vector3d pos(dobj.second.pose().longitude(), dobj.second.pose().latitude(), dobj.second.pose().altitude());
      coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), info.lidar.pose.x, info.lidar.pose.y, info.lidar.pose.z);
      double distance = pos.norm();
      // filter by distance
      if (distance > maxDistance) {
        continue;
      }
      // filter by fov
      if (!lidar->inFov(Catalog::getInstance().getBboxPts(
              std::make_pair(dobj.first, dobj.first == -1 ? dobj.second.id() : dobj.second.raw_type()), pos,
              dobj.second.pose().roll(), dobj.second.pose().pitch(), dobj.second.pose().yaw()))) {
        continue;
      }
      pos = lidar->FovVectorOnlyCar(pos);
      TempObject tobj;
      tobj.t = dobj.first;
      tobj.obj = dobj.second;
      tobj.x = pos.x();
      tobj.y = pos.y();
      tobj.z = pos.z();
      tobj.roll = dobj.second.pose().roll();
      tobj.pitch = dobj.second.pose().pitch();
      tobj.yaw = dobj.second.pose().yaw();
      lidar->FovRotator(tobj.roll, tobj.pitch, tobj.yaw);
      detect_objects.push_back(tobj);
    }
  }

  // save pcd file
  const auto &pcdinfo = info.lidar;
  threads->enqueue(sfun, "lidar/pcd/" + pcdinfo.fpath, pcdinfo.buffer);
  auto pcdname = boost::filesystem::path(pcdinfo.fpath).filename().string();

  // write base info
  Json::Value source;
  Json::Value &label = source["openlabel"];
  // metadata
  label["metadata"]["schema_version"] = "1.0.0";
  // coordinate_systems
  std::string sensor_cs = "lidar";
  sensor_cs += std::to_string(pcdinfo.id);
  label["coordinate_systems"]["geospatial-wgs84"]["type"] = "geo";
  label["coordinate_systems"]["geospatial-wgs84"]["parent"] = "";
  label["coordinate_systems"]["geospatial-wgs84"]["children"].append(sensor_cs);
  Json::Value &cs = label["coordinate_systems"][sensor_cs];
  cs["type"] = "geo";
  cs["parent"] = "geospatial-wgs84";
  cs["children"].resize(0);
  // euler_angles
  cs["pose_wrt_parent"]["euler_angles"].append(pcdinfo.pose.roll);
  cs["pose_wrt_parent"]["euler_angles"].append(pcdinfo.pose.pitch);
  cs["pose_wrt_parent"]["euler_angles"].append(pcdinfo.pose.yaw);
  // translation
  cs["pose_wrt_parent"]["translation"].append(pcdinfo.pose.x);
  cs["pose_wrt_parent"]["translation"].append(pcdinfo.pose.y);
  cs["pose_wrt_parent"]["translation"].append(pcdinfo.pose.z);
  // order
  cs["pose_wrt_parent"]["sequence"] = "zyx";
  // streams
  Json::Value &stream = label["streams"][sensor_cs];
  stream["type"] = "camera";
  stream["description"] = sensor_cs;
  stream["uri"] = "lidar/pcd/" + pcdinfo.fpath;

  // objects
  Json::Value &objects = label["objects"];
  for (const auto &detection : detect_objects) {
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
    obj["object_data"]["cuboid"].resize(1);
    Json::Value &odata = obj["object_data"]["cuboid"][0];
    odata["name"] = "bbox3d";
    // box
    odata["val"].append(detection.x);
    odata["val"].append(detection.y);
    odata["val"].append(detection.z);
    odata["val"].append(detection.yaw);
    odata["val"].append(detection.pitch);
    odata["val"].append(detection.roll);
    odata["val"].append(detection.obj.length());
    odata["val"].append(detection.obj.width());
    odata["val"].append(detection.obj.height());
  }

  Json::StreamWriterBuilder builder;
  builder["commentStyle"] = "None";
  builder["indentation"] = "";
  const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream oss;
  writer->write(source, &oss);
  threads->enqueue(sfun, "lidar/" + tss + "_" + std::to_string(pcdinfo.id) + ".json", oss.str());

  // test
  // write obj file
  // you can drop it into cloudcampare and watch it
  if (debugFiles) {
    std::stringstream test;
    int nn = 0;
    for (const auto &detection : detect_objects) {
      auto &info = detection;
      Eigen::Quaterniond q(Eigen::AngleAxisd(info.yaw, Eigen::Vector3d::UnitZ()));
      Eigen::Affine3d tfs = Eigen::Translation3d(info.x, info.y, info.z) * q.toRotationMatrix();

      const double offx[8] = {-0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5};
      const double offy[8] = {-0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5};
      const double offz[8] = {-0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5};
      for (int i = 0; i < 8; i++) {
        double x = offx[i] * info.obj.length();
        double y = offy[i] * info.obj.width();
        double z = offz[i] * info.obj.height();

        Eigen::Vector3d ep = tfs * Eigen::Vector3d(x, y, z);
        test << "v " << ep.x() << " " << ep.y() << " " << ep.z() << "\n";
      }

      test << "l " << nn + 1 << " " << nn + 2 << " " << nn + 3 << " " << nn + 4 << " " << nn + 1 << "\n";
      test << "l " << nn + 5 << " " << nn + 6 << " " << nn + 7 << " " << nn + 8 << " " << nn + 5 << "\n";
      test << "l " << nn + 1 << " " << nn + 5 << "\n";
      test << "l " << nn + 2 << " " << nn + 6 << "\n";
      test << "l " << nn + 3 << " " << nn + 7 << "\n";
      test << "l " << nn + 4 << " " << nn + 8 << "\n";
      nn += 8;
    }

    threads->enqueue(sfun, "lidar/pcd/" + pcdinfo.fpath + ".obj", test.str());
  }
}

/**
 * @brief This code snippet demonstrates a simple helper function that returns
 * the current UTC timestamp represented as a string. It uses the Chrono library
 * provided by C++ standard library to obtain the current system clock time,
 * converts it to Unix Epoch time, multiplies by 1000 to make it milliseconds
 * accurate, and then formats it as a string. Helper function to get current UTC
 * timestamp as a string
 *
 * @return std::string of UTC timestamp
 */
std::string sim_label::getUTC() {
  auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  return std::to_string(now * 1000);
}

/**
 * @brief get config from protobuf message
 *
 * @param buffer txsim protobuf message
 * @return true on success
 * @return false on faild
 */
bool sim_label::getSensorConfig(const std::string &buffer, const std::string &groupname) {
  if (buffer.empty()) {
    return false;
  }
  // Deserialize protobuf message
  sim_msg::Scene scene;
  if (!scene.ParseFromString(buffer)) {
    std::cout << "parse scene faild.";
    return false;
  }
  // Clear existing camera/LiDAR lists
  cameras.clear();
  lidars.clear();
  semantics.clear();
  // Iterate through egos and process sensors
  for (const auto &ego : scene.egos()) {
    if (ego.group() != groupname) continue;
    for (const auto &sensor : ego.sensor_group().sensors()) {
      // Handle camera case
      if (sensor.type() == sim_msg::SENSOR_TYPE_CAMERA || sensor.type() == sim_msg::SENSOR_TYPE_SEMANTIC) {
        LoadCamera(sensor, device);
      } else if (sensor.type() == sim_msg::SENSOR_TYPE_FISHEYE) {
        // Handle FISHEYE case
        LoadFisheye(sensor, device);
      } else if (sensor.type() == sim_msg::SENSOR_TYPE_TRADITIONAL_LIDAR) {
        // Handle LiDAR case
        LoadLidar(sensor, device);
      }
    }
  }
  return true;
}

/**
 * @brief SAVE mask json file
 *
 * @param fpath
 * @return true
 * @return false
 */
bool sim_label::saveMaskJson(const std::string &fpath) {
  Json::Value root;
  root["maskMapping"]["ground"] = 16;
  root["maskMapping"]["road"] = 12;
  root["maskMapping"]["tree"] = 150;
  root["maskMapping"]["man"] = 90;
  root["maskMapping"]["car"] = 100;
  root["maskMapping"]["build"] = 50;
  root["maskMapping"]["pole"] = 27;
  root["maskMapping"]["trafficlight"] = 28;
  root["maskMapping"]["trafficsign"] = 20;
  Json::StreamWriterBuilder builder;
  builder["commentStyle"] = "None";
  builder["indentation"] = "";
  const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream oss;
  writer->write(root, &oss);

  std::fstream of(savePath + "/" + fpath, std::ios::out | std::ios::binary);
  of.write(oss.str().c_str(), oss.str().size());
  of.close();
  return true;
}

TXSIM_MODULE(sim_label)
