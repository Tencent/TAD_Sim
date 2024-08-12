// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "scenario_parser.h"

#include <cmath>
#include <fstream>
#include <regex>

#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "glog/logging.h"
#include "json/json.h"

#include "location.pb.h"
#include "scene_pb_facade.h"
#include "traffic.pb.h"
#include "vehicle_geometry.pb.h"
#include "xml_util.h"

using namespace tinyxml2;
namespace fs = boost::filesystem;


using namespace tx_sim::utils;


namespace tx_sim {
namespace utils {

//! @brief 函数名：ParseEgoMeasurement
//! @details 函数功能：解析Ego测量数据
//!
//! @param[in] root 一个包含tinyxml2::XMLElement对象的常量指针
//! @param[in] catalog_name 一个包含目录名称的字符串
//! @param[in] entry_name 一个包含条目名称的字符串
//! @param[in] ego_type 一个包含Ego类型的字符串
//! @param[in] ego_measurement_msg 一个包含zmq::message_t对象的引用
bool ParseEgoMeasurement(const std::string& catalogpath, const char* catalog_name, const char* entry_name,
                         const char* ego_type, zmq::message_t& ego_measurement_msg) {
  LOG(INFO) << "find catalog_name:" << catalog_name << " sEgoType:" << ego_type << " sEgoName :" << entry_name
            << " in catalog path " << catalogpath;
  tinyxml2::XMLDocument doc;
  const XMLElement* catalog_root = GetXMLRoot(catalogpath, doc);
  const XMLElement* catalog_node = GetChildNode(catalog_root, "Catalog");
  const XMLElement* target_catalog_node = TryGetNodeWithAttributeInList(catalog_node, "name", catalog_name);
  if (target_catalog_node == nullptr)  // using the first catalog node if the specified not found.
    target_catalog_node = catalog_node;
  const XMLElement* vehicle_node = GetChildNode(target_catalog_node, "Vehicle");
  const XMLElement* target_vehicle_node = TryGetNodeWithAttributeInList(vehicle_node, "name", ego_type);

  if (!target_vehicle_node) {
    LOG(ERROR) << "vehicle node can't find ego_type " << ego_type;
    return false;
  }
  const char* node_name = target_vehicle_node->Name();
  LOG(INFO) << "catalog_name:" << catalog_name;
  LOG(INFO) << "ego_type:" << ego_type;

  do {
    std::string sNameValue = target_vehicle_node->Attribute("name");
    std::string sVehicleCategoryValue = target_vehicle_node->Attribute("vehicleCategory");
    if (ego_type == sNameValue) {
      LOG(INFO) << "find node with name:" << sNameValue;
      break;
    }
    target_vehicle_node = target_vehicle_node->NextSiblingElement(node_name);
  } while (target_vehicle_node);

  if (target_vehicle_node == nullptr) {
    if (entry_name == kDefaultCatalogEntryName) throw std::runtime_error("there's no Vehicle catalog node named ego.");
    target_vehicle_node = GetNodeWithAttributeInList(vehicle_node, "name", "ego", "Catalog.Vehicle");
  }

  const std::string vehicle_category = GetNodeAttribute(target_vehicle_node, "vehicleCategory", "Catalog.Vehicle");
  sim_msg::VehicleGeometoryList vgl;
  if (vehicle_category == "combination") {
    std::string tractor_name =
        GetNodeAttribute(GetChildNode(target_vehicle_node, "Tractor"), "name", "Catalog.Vehicle.Tractor");
    VehicleXMLNode2GeometryMsg(
        GetNodeWithAttributeInList(vehicle_node, "name", tractor_name.c_str(), "Catalog.Vehicle"), vgl.mutable_front());
    const XMLElement* trailer_index_node = GetChildNode(target_vehicle_node, "Trailer");
    while (trailer_index_node) {
      std::string trailer_name = GetNodeAttribute(trailer_index_node, "name", "Catalog.Vehicle.Trailer");
      double offset_x = std::stod(GetNodeAttribute(trailer_index_node, "offsetX", "Catalog.Vehicle.Trailer")),
             offset_z = std::stod(GetNodeAttribute(trailer_index_node, "offsetZ", "Catalog.Vehicle.Trailer"));
      VehicleXMLNode2GeometryMsg(
          GetNodeWithAttributeInList(vehicle_node, "name", trailer_name.c_str(), "Catalog.Vehicle"), vgl.add_trailer(),
          offset_x, offset_z);
      trailer_index_node = trailer_index_node->NextSiblingElement("Trailer");
    }
  } else {
    VehicleXMLNode2GeometryMsg(target_vehicle_node, vgl.mutable_front());
  }

  ego_measurement_msg.rebuild(vgl.ByteSizeLong());
  vgl.SerializeToArray(ego_measurement_msg.data(), ego_measurement_msg.size());
}

//! @brief 函数名：ParseSimCloudCityVehicle
//! @details 函数功能：解析模拟CloudCity车辆
//!
//! @param[in] scene_pb 一个包含zmq::message_t对象的引用
//! @param[in] catalogPath 一个包含目录路径的字符串
void ParseSimCloudCityVehicle(zmq::message_t& scene_pb, const std::string& catalogSysPath,
                              const std::string& catalogDataPath) {
  sim_msg::Scene simScene;
  try {
    tx_sim::coordinator::CScenePbFacade scenePbFacade(
        "", (fs::path(catalogSysPath) / kDefaultCatalogFileRelativePath).lexically_normal().make_preferred().string(),
        (fs::path(catalogSysPath) / kVehicleCatalogFileRelativePath).lexically_normal().make_preferred().string(),
        (fs::path(catalogSysPath) / kMiscObjectCatalogFileRelativePath).lexically_normal().make_preferred().string(),
        (fs::path(catalogSysPath) / kPedestrianCatalogFileRelativePath).lexically_normal().make_preferred().string(),
        (fs::path(catalogDataPath) / kDefaultCatalogFileRelativePath).lexically_normal().make_preferred().string(),
        (fs::path(catalogDataPath) / kVehicleCatalogFileRelativePath).lexically_normal().make_preferred().string(),
        (fs::path(catalogDataPath) / kMiscObjectCatalogFileRelativePath).lexically_normal().make_preferred().string(),
        (fs::path(catalogDataPath) / kPedestrianCatalogFileRelativePath).lexically_normal().make_preferred().string(),
        (fs::path(catalogDataPath) / kModel3dCatalogFileRelativePath).lexically_normal().make_preferred().string());
    if (scenePbFacade.BuildCloudCityScenePb(simScene)) {
      LOG(INFO) << "build scene pb success";
    } else {
      LOG(INFO) << "build scene pb failed";
    }
  } catch (const std::exception& e) { LOG(ERROR) << "build scene pb error: " << e.what(); }
  scene_pb.rebuild(simScene.ByteSizeLong());
  simScene.SerializeToArray(scene_pb.data(), scene_pb.size());
}

}  // namespace utils
}  // namespace tx_sim

namespace tx_sim {
namespace coordinator {
//! @brief 函数名：CheckIsOpenScenario
//! @details 函数功能：检查是否为OpenScenario
//!
//! @param[in] path 一个包含路径的字符串
//!
//! @return 返回一个字符串，表示是否为OpenScenario
std::string ScenarioXmlParser::CheckIsOpenScenario(const std::string& path) {
  fs::path scenario_file(path);
  std::string ext = scenario_file.extension().string();
  std::string tmpPath = path;
  if (ext == ".sim") return path;
  else if (ext == ".xosc") {
    size_t nPos = path.find_last_of(".");
    if (nPos != std::string::npos) {
      tmpPath.replace(nPos, 5, "_convert.sim");
      return tmpPath;
    }
    return "";
  } else
    throw std::invalid_argument("unknown scenario file extension: " + ext);
}

//! @brief 函数名：GetEgoType
//! @details 函数功能：获取Ego类型
//!
//! @param[in] path 一个包含路径的字符串
//!
//! @return 返回一个字符串，表示Ego类型
std::string ScenarioXmlParser::GetEgoType(const std::string& path) {
  tinyxml2::XMLDocument doc;
  const XMLElement* root = GetXMLRoot(path, doc);
  const XMLElement* planner_node = GetChildNode(root, "planner");
  const XMLElement* egoNode = TryGetChildNode(planner_node, "ego");
  std::string sEgoType = egoNode ? egoNode->Attribute("type") : "";
  LOG(INFO) << "parse sim type: " << sEgoType;
  return sEgoType;
}

//! @brief 函数名：ExistSimRec
//! @details 函数功能：检查是否存在模拟记录
//!
//! @param[in] path 一个包含路径的字符串
//!
//! @return 返回一个布尔值，表示是否存在模拟记录
bool ScenarioXmlParser::ExistSimRec(const std::string& path) {
  tinyxml2::XMLDocument doc;
  const XMLElement* root = GetXMLRoot(path, doc);
  const XMLElement* traffic_node = root->FirstChildElement("traffic");
  if (traffic_node == nullptr) return false;

  fs::path simrec_path(path);
  simrec_path = simrec_path.parent_path() / GetNodeText(traffic_node, "traffic", true);
  if (simrec_path.extension() == ".simrec") return true;
  return false;
}


//! @brief 函数名：ParseScenario
//! @details 函数功能：解析场景
//!
//! @param[in] path 一个包含路径的字符串
//! @param[in] req 一个包含tx_sim::impl::ModuleResetRequest对象的引用
//!
//! @return 返回一个布尔值，表示解析是否成功
bool ScenarioXmlParser::ParseScenario(const std::string& path, tx_sim::impl::ModuleResetRequest& req) {
  std::string tmpPath = CheckIsOpenScenario(path);
  fs::path scenario_file(tmpPath);
  req.scenario_path = tmpPath;
  return ParseSimScenario(tmpPath, req);
}

//! @brief 函数名：ParseLogsimEvents
//! @details 函数功能：解析Logsim事件
//!
//! @param[in] path 一个包含路径的字符串
//! @param[in] event_info 一个包含LogsimEventInfo对象的引用
void ScenarioXmlParser::ParseLogsimEvents(const std::string& path, LogsimEventInfo& event_info) {
  tinyxml2::XMLDocument doc;
  const XMLElement* root = GetXMLRoot(path, doc);

  const XMLElement* traffic_node = root->FirstChildElement("traffic");
  if (traffic_node == nullptr) throw std::runtime_error("no traffic node found for parsing simrec info");

  fs::path simrec_path(path);
  simrec_path = simrec_path.parent_path() / GetNodeText(traffic_node, "traffic", true);
  if (simrec_path.extension() != ".simrec") return;

  if (!fs::is_regular_file(simrec_path))
    throw std::runtime_error(std::string(".simrec file not found: ") + simrec_path.string());

  sim_msg::TrafficRecords4Logsim tfc_log;
  std::ifstream in(simrec_path.string(), std::ios::binary);
  if (!tfc_log.ParseFromIstream(&in))
    throw std::runtime_error(std::string("parsing proto binary file failed: ") + simrec_path.string());

  if (tfc_log.ego_location_size() > 0) event_info.start_timestamp_ms = tfc_log.ego_location(0).t();
  event_info.total_duration_ms = tfc_log.duration_ms();
  event_info.events.clear();
  for (const auto& e : tfc_log.disengage_events()) {
    event_info.events.emplace_back();
    auto& i = event_info.events.back();
    i.timestamp_ms = e.timestamp_ms();
    i.type = e.type();
  }
}

//! @brief 函数名：ParseLog2WorldConfig
//! @details 函数功能：解析Log2World配置
//!
//! @param[in] path 一个包含路径的字符串
//! @param[in] config 一个包含Log2WorldPlayConfig对象的引用
void ScenarioXmlParser::ParseLog2WorldConfig(const std::string& path, Log2WorldPlayConfig& config) {
  tinyxml2::XMLDocument doc;
  const XMLElement* root = GetXMLRoot(path, doc);

  const XMLElement* log2world_node = root->FirstChildElement("log2world");
  if (log2world_node == nullptr) return;

  config.switch_time_ms = log2world_node->IntAttribute("switch_time", 0);
  config.ego_switch = log2world_node->BoolAttribute("ego_switch", false);
  config.traffic_switch = log2world_node->BoolAttribute("traffic_switch", false);
  config.switch_type = tx_sim::impl::Int2Log2WorldSwitchType(log2world_node->IntAttribute("switch_type", 1));
}

//! @brief 函数名：CacheLog2WorldConfig
//! @details 函数功能：缓存Log2World配置
//!
//! @param[in] path 一个包含路径的字符串
//! @param[in] config 一个包含Log2WorldPlayConfig对象的引用
void ScenarioXmlParser::CacheLog2WorldConfig(const std::string& path, Log2WorldPlayConfig& config) {
  tinyxml2::XMLDocument doc;
  XMLElement* root = GetXMLRoot(path, doc);

  XMLElement* log2world_node = root->FirstChildElement("log2world");
  if (log2world_node == nullptr) {
    log2world_node = doc.NewElement("log2world");
    root->InsertEndChild(log2world_node);
  }
  log2world_node->SetAttribute("switch_time", config.switch_time_ms);
  log2world_node->SetAttribute("ego_switch", config.ego_switch);
  log2world_node->SetAttribute("traffic_switch", config.traffic_switch);
  log2world_node->SetAttribute("switch_type", static_cast<int16_t>(config.switch_type));

  if (doc.SaveFile(path.c_str()) != XML_SUCCESS)
    throw std::runtime_error(std::string("save scenario file error: ") + path);
}

void GetCatalogPath(const fs::path& scenario_dir, fs::path& data_catalog_path, fs::path& sys_catalog_path) {
  data_catalog_path = scenario_dir;
  // if the scenario directory is in a desktop data dir, switch it to the sys dir.
  const std::string dir_name(data_catalog_path.parent_path().parent_path().stem().string());
  if (dir_name == tx_sim::impl::kPathUserDirSys)  //.sim file find from local \sys\scenario\scene
  {
    data_catalog_path = data_catalog_path.parent_path().parent_path().parent_path() / tx_sim::impl::kPathUserDirData /
                        data_catalog_path.parent_path().stem() / data_catalog_path.stem();
    data_catalog_path = data_catalog_path.parent_path();

    sys_catalog_path = data_catalog_path.parent_path().parent_path().parent_path() / tx_sim::impl::kPathUserDirSys /
                       data_catalog_path.stem();
  } else if (dir_name == tx_sim::impl::kPathUserDirData)  //.sim file find from local \data\scenario\scene
  {
    data_catalog_path = data_catalog_path.parent_path();
    sys_catalog_path =
        data_catalog_path.parent_path().parent_path() / tx_sim::impl::kPathUserDirSys / data_catalog_path.stem();
  } else if (dir_name == "sim" ||  // .sim file find from cloud  /home/sim/data/scene/
             dir_name == "tests")  // gtest dir name only use in test case
  {
    data_catalog_path = data_catalog_path.parent_path();
  } else {
    LOG(ERROR) << "can't find sim cata log catalog_path:" << data_catalog_path << " with dir name " << dir_name;
  }
}

bool ScenarioXmlParser::ParseSimEgo(const std::string& scenario, const XMLElement* ego_node,
                                    tx_sim::impl::ModuleEgoInfo& egoinfo) {
  fs::path scenarioPath(scenario);
  fs::path scenario_dir = scenarioPath.parent_path();
  std::vector<std::string> point_triple;
  // using std::stod instead of tinyxml2's DoubleText to enforce exception throwing.
  egoinfo.ego_initial_velocity = std::stod(GetNodeText(GetChildNode(ego_node, "start_v"), "start_v"));
  egoinfo.ego_speed_limit = std::stod(GetNodeText(GetChildNode(ego_node, "Velocity_Max"), "Velocity_Max"));
  egoinfo.ego_initial_theta = std::stod(GetNodeText(GetChildNode(ego_node, "theta"), "theta"));

  // if there is "InputPath" node, parsing it as the ego path.
  const XMLElement* path_node = ego_node->FirstChildElement("InputPath");
  if (path_node) {
    std::vector<std::string> path_points;
    boost::split(path_points, GetNodeAttribute(path_node, "points", "InputPath"), boost::is_any_of(";"));
    if (path_points.empty())
      throw std::invalid_argument("invalid format of InputPath points, at least 1 point(start) should be given.");
    for (const std::string& p : path_points) {
      ParsePosStr(p, point_triple);
      egoinfo.ego_path.emplace_back(std::stod(point_triple[0]), std::stod(point_triple[1]),
                                    point_triple.size() == 3 ? std::stod(point_triple[2]) : 0);
    }
  } else {  // fallback to the old start/end/altitude node.
    const XMLElement* route_node = GetChildNode(ego_node, "route");
    ParsePosStr(GetNodeAttribute(route_node, "start", "route"), point_triple);
    egoinfo.ego_path.emplace_back(std::stod(point_triple[0]), std::stod(point_triple[1]), 0);
    ParsePosStr(GetNodeAttribute(route_node, "end", "route"), point_triple);
    egoinfo.ego_path.emplace_back(std::stod(point_triple[0]), std::stod(point_triple[1]), 0);
    // if there is "altitude" node, parsing it.
    const XMLElement* alt_node = ego_node->FirstChildElement("altitude");
    if (alt_node) {
      std::get<2>(egoinfo.ego_path[0]) = std::stod(GetNodeAttribute(alt_node, "start", "altitude"));
      std::get<2>(egoinfo.ego_path[1]) = std::stod(GetNodeAttribute(alt_node, "end", "altitude"));
    }
  }

  // try to get the measurement info of the ego car(currently only support one ego car).
  const char* ego_id_str = ego_node->Attribute("id");
  int64_t ego_id = ego_id_str == nullptr ? 1 : (std::stoll(ego_id_str));
  const XMLElement* catalog_node = ego_node->FirstChildElement("catalog");
  const char* catalog_file_path_str = kDefaultCatalogFileRelativePath.c_str();
  std::string catalog_name = kDefaultCatalogName.c_str();
  const char* catalog_entry_name = kDefaultCatalogEntryName.c_str();
  if (catalog_node) {
    catalog_file_path_str = catalog_node->Attribute("filePath");
    catalog_name = catalog_node->Attribute("name");
    catalog_entry_name = catalog_node->Attribute("entryName");
  }

  fs::path data_catalog_path;
  fs::path sys_catalog_path;
  GetCatalogPath(scenario_dir, data_catalog_path, sys_catalog_path);
  data_catalog_path /= catalog_file_path_str;
  data_catalog_path = data_catalog_path.lexically_normal().make_preferred();

  sys_catalog_path /= catalog_file_path_str;
  sys_catalog_path = sys_catalog_path.lexically_normal().make_preferred();
  try {
    const XMLElement* egoNode = TryGetChildNode(ego_node, "ego");
    std::string sEgoType = egoNode ? GetNodeAttributeWithDefault(egoNode, "type", "suv") : "suv";
    std::string sEgoName = egoNode ? GetNodeAttributeWithDefault(egoNode, "name", "") : "";
    std::string sGroup =
        egoNode ? GetNodeAttributeWithDefault(egoNode, "group", "") : tx_sim::impl::kDefaultEgoGroupName;

    if (sEgoType.empty() || sEgoType == "suv" || sEgoType == "car") {
      sEgoType = "suv";
      catalog_name = "CarCatalog";
    } else if (sEgoType == "truck" || sEgoType == "combination") {
      sEgoType = "truck";
      catalog_name = "TruckCatalog";
    } else {
      LOG(INFO) << "catalog_name:" << catalog_name << " sEgoType:" << sEgoType << " sEgoName :" << sEgoName
                << " group :" << sGroup;
    }
    egoinfo.ego_group_name = sGroup;

    if (!sEgoName.empty()) { sEgoType = sEgoName; }
    egoinfo.ego_vehicle_measurement.emplace_back();
    egoinfo.ego_vehicle_measurement.back().first = ego_id;


    if (!tx_sim::utils::ParseEgoMeasurement(data_catalog_path.string(), catalog_name.c_str(), catalog_entry_name,
                                            sEgoType.c_str(), std::get<1>(egoinfo.ego_vehicle_measurement.back()))) {
      LOG(ERROR) << "can't find sEgoType in catalog";
      throw std::runtime_error("vehicle node can't find ego_type: " + std::string(sEgoType));
    }
  } catch (const std::exception& e) {
    LOG(WARNING) << "trying parsing the measurement info of the ego vehicle error: " << e.what();
    return false;
  }
}

bool ScenarioXmlParser::ParseSimScenario(const std::string& scenario, tx_sim::impl::ModuleResetRequest& req) {
  // get had-map file path.
  tinyxml2::XMLDocument doc;
  const XMLElement* root = GetXMLRoot(scenario, doc);
  fs::path scenarioPath(scenario);
  fs::path scenario_dir = scenarioPath.parent_path();

  const XMLElement* map_node = GetChildNode(root, "mapfile");
  fs::path map_path(scenario_dir);
  map_path /= GetNodeText(map_node, "mapfile", true);
  std::string hadmapPath = map_path.lexically_normal().make_preferred().string();

  req.map_path = hadmapPath;
  // get origin point.
  req.map_local_origin[0] = std::stod(GetNodeAttribute(map_node, "lon", "mapfile"));
  req.map_local_origin[1] = std::stod(GetNodeAttribute(map_node, "lat", "mapfile"));
  req.map_local_origin[2] = std::stod(GetNodeAttribute(map_node, "alt", "mapfile"));

  // get geo fence if specified.
  const XMLElement* geofence_node = root->FirstChildElement("geofence");
  if (geofence_node != nullptr) {
    std::vector<std::string> geo_points;
    boost::split(geo_points, GetNodeText(geofence_node, "geofence"), boost::is_any_of(";"));
    for (const std::string& p : geo_points) {
      size_t pos = p.find_first_of(',');
      req.geo_fence.emplace_back(std::stod(p.substr(0, pos)), std::stod(p.substr(pos + 1)));
    }
  }
  // get planner initial parameters.
  const XMLElement* planner_node = GetChildNode(root, "planner");
  std::string sim_version = GetNodeAttributeWithDefault(planner_node, "version", "1.0");
  LOG(INFO) << "planner node version:" << sim_version;
  if (!sim_version.empty() && sim_version == "2.0") {
    const XMLElement* egolist_node = GetChildNode(planner_node, "egolist");
    for (const XMLElement* ego_node = egolist_node->FirstChildElement("egoinfo"); ego_node != nullptr;
         ego_node = ego_node->NextSiblingElement("egoinfo")) {
      req.ego_infos.emplace_back();
      ParseSimEgo(scenario, ego_node, req.ego_infos.back());
    }
  } else {
    req.ego_infos.emplace_back();
    ParseSimEgo(scenario, planner_node, req.ego_infos.back());
  }
  // const XMLElement* scene_group_node = GetChildNode(planner_node, "scene_group");
  // const XMLElement* scene_controller_node = GetChildNode(root, "controller");
  const char* catalog_file_path_str = kDefaultCatalogFileRelativePath.c_str();
  fs::path data_catalog_path;
  fs::path sys_catalog_path;
  GetCatalogPath(scenario_dir, data_catalog_path, sys_catalog_path);
  fs::path vehicleCatalogDataPath = data_catalog_path / kVehicleCatalogFileRelativePath;
  fs::path miscCatalogDataPath = data_catalog_path / kMiscObjectCatalogFileRelativePath;
  fs::path pedestrianCatalogDataPath = data_catalog_path / kPedestrianCatalogFileRelativePath;
  fs::path mapModel3dXMLDataPath = data_catalog_path / kModel3dCatalogFileRelativePath;

  std::string model3d_pathdir = (data_catalog_path / "models").lexically_normal().make_preferred().string();
  data_catalog_path /= catalog_file_path_str;
  data_catalog_path = data_catalog_path.lexically_normal().make_preferred();

  sys_catalog_path = sys_catalog_path.lexically_normal().make_preferred();
  fs::path vehicleCatalogSysPath = sys_catalog_path / kVehicleCatalogFileRelativePath;
  fs::path miscCatalogSysPath = sys_catalog_path / kMiscObjectCatalogFileRelativePath;
  fs::path pedestrianCatalogSysPath = sys_catalog_path / kPedestrianCatalogFileRelativePath;
  sys_catalog_path /= catalog_file_path_str;

  sim_msg::Scene simScene;
  try {
    CScenePbFacade scenePbFacade(
        scenario, sys_catalog_path.string(), vehicleCatalogSysPath.lexically_normal().make_preferred().string(),
        miscCatalogSysPath.lexically_normal().make_preferred().string(),
        pedestrianCatalogSysPath.lexically_normal().make_preferred().string(), data_catalog_path.string(),
        vehicleCatalogDataPath.lexically_normal().make_preferred().string(),
        miscCatalogDataPath.lexically_normal().make_preferred().string(),
        pedestrianCatalogDataPath.lexically_normal().make_preferred().string(),
        mapModel3dXMLDataPath.lexically_normal().make_preferred().string());
    if (scenePbFacade.BuildScenePb(simScene)) {
      LOG(INFO) << "build scene pb success";
    } else {
      LOG(INFO) << "build scene pb failed";
    }
  } catch (const std::exception& e) { LOG(ERROR) << "build scene pb error: " << e.what(); }
  std::string sLabels = req.m_labels;
  std::vector<std::string> vecLabel;
  LOG(INFO) << "labels:" << sLabels;
  if (!sLabels.empty()) {
    boost::split(vecLabel, sLabels, boost::is_any_of(","));
    for (const std::string& item : vecLabel) { simScene.mutable_setting()->add_scenario_labels(item); }
  }
  simScene.mutable_setting()->set_model3d_pathdir(model3d_pathdir);
#ifdef _DEBUG
  LOG(INFO) << "scene pb:" << simScene.DebugString();
#endif

  req.m_setting_pb.rebuild(simScene.setting().ByteSizeLong());
  simScene.setting().SerializeToArray(req.m_setting_pb.data(), req.m_setting_pb.size());

  req.m_scene_pb.rebuild(simScene.ByteSizeLong());
  simScene.SerializeToArray(req.m_scene_pb.data(), req.m_scene_pb.size());
  return true;
}

// the parsing logic for Open Scenario format is not supported for further updates.
// it is the responsibility of scenario editor to converting Open scenario file to .sim file.
//! @brief 函数名：ParseOpenScenario
//! @details 函数功能：解析OpenScenario
//!
//! @param[in] root 一个包含tinyxml2::XMLElement对象的指针
//! @param[in] scenario_dir 一个包含boost::filesystem::path对象的引用
//! @param[in] req 一个包含tx_sim::impl::ModuleResetRequest对象的引用
void ScenarioXmlParser::ParseOpenScenario(const XMLElement* root, fs::path scenario_dir,
                                          tx_sim::impl::ModuleResetRequest& req) {
  // get had-map file path.
  const XMLElement* map_node = GetChildNode(GetChildNode(root, "RoadNetwork"), "LogicFile");
  fs::path map_path(scenario_dir);
  map_path /= GetNodeAttribute(map_node, "filepath", "RoadNetwork.LogicFile");
  req.map_path = map_path.lexically_normal().make_preferred().string();
  req.ego_infos.emplace_back();
  // get planner initial parameters.
  std::unordered_map<std::string, double> way_points;
  bool start_theta_found = false, start_v_found = false, max_v_found = false;
  // 1. read and save some interested parameter declaration values for ego car.
  const XMLElement* param_decl =
      TryGetChildNode(TryGetChildNode(root, "ParameterDeclarations"), "ParameterDeclaration");
  while (param_decl) {
    std::string param_name = GetNodeAttribute(param_decl, "name", "ParameterDeclaration");
    std::string param_value = GetNodeAttribute(param_decl, "value", "ParameterDeclaration");
    if (param_name == "$HeadingAngle_Ego") {
      req.ego_infos[0].ego_initial_theta = std::stod(param_value);
      start_theta_found = true;
    } else if (param_name == "$Speed_Ini_Ego") {
      req.ego_infos[0].ego_initial_velocity = std::stod(param_value);
      start_v_found = true;
    } else if (param_name == "$Speed_Max_Ego") {
      req.ego_infos[0].ego_speed_limit = std::stod(param_value);
      max_v_found = true;
    } else if (boost::algorithm::starts_with(param_name, "$Ego_WayPoint_")) {
      way_points[param_name] = std::stod(param_value);
    }
    param_decl = param_decl->NextSiblingElement("ParameterDeclaration");
  };
  // 2. read ego path from Storyboard -> Init -> Actions -> <Private entityRef="Ego">
  //    -> PrivateAction -> RoutingAction -> AssignRouteAction -> <Route name="EgoRoute">
  //    -> Waypoint -> Position -> WorldPosition
  // 2.1. locating/validating to the Waypoint xml element.
  const XMLElement* ego_private_node = GetNodeWithAttributeInList(
      GetChildNode(GetChildNode(GetChildNode(GetChildNode(root, "Storyboard"), "Init"), "Actions"), "Private"),
      "entityRef", "Ego", "Storyboard.Init.Actions.Private");
  const XMLElement* routing_action_node =
      GetGrandChildInList(GetChildNode(ego_private_node, "PrivateAction"), "RoutingAction",
                          "PrivateAction.RoutingAction of ego private node");
  const XMLElement* waypoint_node =
      GetChildNode(GetChildNode(GetChildNode(routing_action_node, "AssignRouteAction"), "Route"), "Waypoint");
  // 2.2. read ego path x/y points.
  auto parse_waypoint_func = [&way_points](const XMLElement* node, const char* attr) {
    std::string param_name = GetNodeAttribute(node, attr, "WorldPosition");
    if (param_name.empty())
      throw std::invalid_argument(std::string("parameter value ") + std::string(attr) +
                                  " of WorldPosition is empty(xosc scenario file)");
    if (param_name.at(0) != '$')  // it is a direct value instead of a declared parameter.
      return std::stod(param_name);
    auto it = way_points.find(param_name);
    if (it == way_points.end())
      throw std::invalid_argument("no " + param_name +
                                  " in WorldPosition defined in ParameterDeclarations(xosc scenario file)");
    return it->second;
  };
  do {
    const XMLElement* pos_node = GetChildNode(GetChildNode(waypoint_node, "Position"), "WorldPosition");
    req.ego_infos[0].ego_path.emplace_back(parse_waypoint_func(pos_node, "x"), parse_waypoint_func(pos_node, "y"), 0);
    waypoint_node = waypoint_node->NextSiblingElement("Waypoint");
  } while (waypoint_node);
  // 2.3. transform points in ego path vector from OpenDrive defined proj to lon-lat.
  TransformEgoPathCoordinates(req.map_path, req.ego_infos[0].ego_path);
  // using start point of the Ego car as the map origin.
  const auto& ego_start_point = req.ego_infos[0].ego_path[0];
  req.map_local_origin[0] = std::get<0>(ego_start_point);
  req.map_local_origin[1] = std::get<1>(ego_start_point);
  req.map_local_origin[2] = std::get<2>(ego_start_point);
  // 3. optionally, try parsing the TeleportAction in Storyboard(initial theta) and Properties in Ego
  //    Entities(initial/max velocity), if there are any.
  if (!start_theta_found)
    req.ego_infos[0].ego_initial_theta = std::stod(GetNodeAttribute(
        GetChildNode(GetChildNode(GetGrandChildInList(GetChildNode(ego_private_node, "PrivateAction"), "TeleportAction",
                                                      "PrivateAction.TeleportAction of ego private node"),
                                  "Position"),
                     "WorldPosition"),
        "h", "TeleportAction.Position.WorldPosition"));
  if (!start_v_found || !max_v_found) {
    const XMLElement* ego_properties_node = GetChildNode(
        GetChildNode(GetNodeWithAttributeInList(GetChildNode(GetChildNode(root, "Entities"), "ScenarioObject"), "name",
                                                "Ego", "Entities.ScenarioObject"),
                     "Vehicle"),
        "Properties");
    if (!start_v_found)
      req.ego_infos[0].ego_initial_velocity = std::stod(
          GetNodeAttribute(GetNodeWithAttributeInList(GetChildNode(ego_properties_node, "Property"), "name", "StartV",
                                                      "Entities.ScenarioObject.Vehicle.Properties.Property"),
                           "value", "Ego ScenarioObject StartV Property"));
    if (!max_v_found)
      req.ego_infos[0].ego_speed_limit = std::stod(
          GetNodeAttribute(GetNodeWithAttributeInList(GetChildNode(ego_properties_node, "Property"), "name", "MaxV",
                                                      "Entities.ScenarioObject.Vehicle.Properties.Property"),
                           "value", "Ego ScenarioObject MaxV Property"));
  }
}


// assembling location pb msg for old API compitability.
//! @brief 函数名：AssembleInitialLocation
//! @details 函数功能：组装初始位置
//!
//! @param[in] req 一个包含tx_sim::impl::ModuleResetRequest对象的引用
void AssembleInitialLocation(tx_sim::impl::ModuleResetRequest& req) {
  for (auto& itemInfo : req.ego_infos) {
    sim_msg::Location loc;
    sim_msg::Vec3* pos = loc.mutable_position();
    pos->set_x(std::get<0>(itemInfo.ego_path[0]));
    pos->set_y(std::get<1>(itemInfo.ego_path[0]));
    pos->set_z(std::get<2>(itemInfo.ego_path[0]));
    loc.mutable_rpy()->set_z(itemInfo.ego_initial_theta);
    sim_msg::Vec3* v = loc.mutable_velocity();
    v->set_x(itemInfo.ego_initial_velocity * std::cos(itemInfo.ego_initial_theta));
    v->set_y(itemInfo.ego_initial_velocity * std::sin(itemInfo.ego_initial_theta));
    size_t byte_size = loc.ByteSizeLong();
    uint8_t* byte_array = new uint8_t[byte_size];
    loc.SerializeToArray(byte_array, byte_size);
    itemInfo.initial_location.rebuild(byte_array, byte_size,
                                      [](void* data, void* hint) { delete[] static_cast<uint8_t*>(data); });
  }
}

}  // namespace coordinator
}  // namespace tx_sim
