// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "xml_util.h"
#include <cmath>
#include <fstream>
#include <regex>
#include <sstream>
#ifdef PROJ_NEW_API
#  include <proj.h>
#else
#  ifndef ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#    define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#  endif
#  include "proj_api.h"
#endif


#include "boost/algorithm/string.hpp"
#include "glog/logging.h"


using namespace tinyxml2;
namespace tx_sim {
namespace utils {

// using explict namespace prefix to avoid error C2872: 'XMLDocument': ambiguous symbol on Windows(msxml.h).
//!
//! @brief 函数名：GetXMLRoot
//! @details 函数功能：获取XML文件的根节点
//!
//! @param[in] path XML文件路径
//! @param[in] doc XML文档对象
//!
XMLElement* GetXMLRoot(const std::string& path, tinyxml2::XMLDocument& doc) {
  if (doc.LoadFile(path.c_str()) != XML_SUCCESS)
    throw std::invalid_argument("cannot open file: " + path + " " + doc.ErrorStr());
  XMLElement* root = doc.RootElement();
  if (root == nullptr) throw std::invalid_argument("no root element found in " + path);
  return root;
}

//!
//! @brief 函数名：GetChildNode
//! @details 函数功能：获取子节点
//!
//! @param[in] f 父节点
//! @param[in] name 子节点名称
//!
const XMLElement* GetChildNode(const XMLElement* f, const char* name) {
  const XMLElement* child = f->FirstChildElement(name);
  if (child == nullptr) throw std::invalid_argument("no " + std::string(name) + " element found");
  return child;
}

//!
//! @brief 函数名：GetChildNode
//! @details 函数功能：获取具有指定属性值的子节点
//!
//! @param[in] f 父节点
//! @param[in] name 子节点名称
//! @param[in] key 属性名称
//! @param[in] value 属性值
//!
const XMLElement* GetChildNode(const XMLElement* f, const char* name, const char* key, const char* value) {
  const XMLElement* pNode = f->FirstChildElement(name);
  while (pNode) {
    std::string sValue = GetNodeAttributeWithDefault(pNode, key, "");
    if (sValue == value) return pNode;
    pNode = pNode->NextSiblingElement();
  }
  return nullptr;
}

//!
//! @brief 函数名：TryGetChildNode
//! @details 函数功能：尝试获取子节点，如果不存在则返回nullptr
//!
//! @param[in] f 父节点
//! @param[in] name 子节点名称
//!
const XMLElement* TryGetChildNode(const XMLElement* f, const char* name) {
  if (f == nullptr) return nullptr;
  return f->FirstChildElement(name);
}

//!
//! @brief 函数名：GetNodeAttribute
//! @details 函数功能：获取节点属性值
//!
//! @param[in] node 节点对象
//! @param[in] attr_name 属性名称
//! @param[in] node_name 节点名称
//!
std::string GetNodeAttribute(const XMLElement* node, const char* attr_name, const std::string& node_name) {
  const char* p = node->Attribute(attr_name);
  if (p == nullptr)
    throw std::invalid_argument("no " + std::string(attr_name) + " attribute on node " + node_name + " specified.");
  return p;
}

//!
//! @brief 函数名：GetNodeAttributeWithDefault
//! @details 函数功能：获取节点属性值，如果不存在则返回默认值
//!
//! @param[in] node 节点对象
//! @param[in] attr_name 属性名称
//! @param[in] defaultValue 默认值
//!
std::string GetNodeAttributeWithDefault(const XMLElement* node, const char* attr_name,
                                        const std::string& defaultValue) {
  const char* value = node->Attribute(attr_name);
  std::string ret;
  if (value == nullptr) {
    ret = defaultValue;
  } else {
    ret = value;
  }
  return ret;
}

//!
//! @brief 函数名：GetNodeText
//! @details 函数功能：获取节点文本内容
//!
//! @param[in] node 节点对象
//! @param[in] node_name 节点名称
//! @param[in] trim_copy 是否去除文本内容的空白字符
//!
std::string GetNodeText(const XMLElement* node, const std::string& node_name, bool trim_copy) {
  const char* p = node->GetText();
  if (p == nullptr) throw std::invalid_argument("no text on node " + node_name + " specified.");
  return trim_copy ? boost::algorithm::trim_copy(std::string(p)) : p;
}

//!
//! @brief 函数名：TryGetNodeWithAttributeInList
//! @details 函数功能：尝试在节点列表中获取具有指定属性值的节点，如果不存在则返回nullptr
//!
//! @param[in] current_node 当前节点
//! @param[in] attr_name 属性名称
//! @param[in] attr_value 属性值
//!
const XMLElement* TryGetNodeWithAttributeInList(const XMLElement* current_node, const char* attr_name,
                                                const char* attr_value) {
  const char* node_name = current_node->Name();
  do {
    if (GetNodeAttribute(current_node, attr_name, node_name) == attr_value) return current_node;
    current_node = current_node->NextSiblingElement(node_name);
  } while (current_node);
  return nullptr;
}

//!
//! @brief 函数名：GetNodeWithAttributeInList
//! @details 函数功能：在节点列表中获取具有指定属性值的节点
//!
//! @param[in] current_node 当前节点
//! @param[in] attr_name 属性名称
//! @param[in] attr_value 属性值
//! @param[in] log_name 日志名称
//!
const XMLElement* GetNodeWithAttributeInList(const XMLElement* current_node, const char* attr_name,
                                             const char* attr_value, const std::string& log_name) {
  auto ret = TryGetNodeWithAttributeInList(current_node, attr_name, attr_value);
  if (ret == nullptr) {
    std::ostringstream ss;
    ss << "there's no " << log_name << " with attribute " << attr_name << "=" << attr_value << " found.";
    throw std::invalid_argument(ss.str());
  }
  return ret;
}

//!
//! @brief 函数名：GetGrandChildInList
//! @details 函数功能：获取子节点列表中的孙子节点
//!
//! @param[in] current_node 当前节点
//! @param[in] child_name 子节点名称
//! @param[in] log_name 日志名称
//!
const XMLElement* GetGrandChildInList(const XMLElement* current_node, const char* child_name,
                                      const std::string& log_name) {
  const char* node_name = current_node->Name();
  bool found = (current_node->FirstChildElement(child_name) != nullptr);
  while (!found) {
    current_node = current_node->NextSiblingElement(node_name);
    if (!current_node) break;
    found = (current_node->FirstChildElement(child_name) != nullptr);
  }
  if (!found) {
    std::ostringstream ss;
    ss << "there's no " << log_name << " with first child named " << child_name << " found.";
    throw std::invalid_argument(ss.str());
  }
  return current_node->FirstChildElement(child_name);
}

//!
//! @brief 函数名：ParsePosStr
//! @details 函数功能：解析位置字符串
//!
//! @param[in] s 位置字符串
//! @param[out] ret 解析后的位置数据
//!
void ParsePosStr(const std::string& s, std::vector<std::string>& ret) {
  ret.clear();
  boost::split(ret, s, boost::is_any_of(","));
  if (ret.size() == 2 || ret.size() == 3) return;
  throw std::invalid_argument("invalid format of path points, a point should be a pair/triple.");
}

//!
//! @brief 函数名：TransformEgoPathCoordinates
//! @details 函数功能：转换自车路径坐标
//!
//! @param[in] od_map_path 路径地图文件路径
//! @param[out] ego_path 自车路径数据
//!
void TransformEgoPathCoordinates(const std::string& od_map_path,
                                 std::vector<std::tuple<double, double, double>>& ego_path) {
  tinyxml2::XMLDocument doc;
  const XMLElement* root = GetXMLRoot(od_map_path, doc);
  const XMLElement* header = GetChildNode(root, "header");
  double offset_west = std::stod(GetNodeAttribute(header, "west", "header")),
         offset_south = std::stod(GetNodeAttribute(header, "south", "header"));
  std::string od_geo_ref = GetNodeText(GetChildNode(header, "geoReference"), "geoReference"),
              target_geo_ref("+proj=longlat +datum=WGS84 +no_defs"), od_proj;
  std::regex rgx("\\+proj=(\\w+)");
  std::smatch matches;
  if (std::regex_search(od_geo_ref, matches, rgx) && matches.size() > 1) od_proj = matches[1].str();
  else
    throw std::runtime_error("no proj defined in OpenDrive map file.");
  if (od_proj == "longlat") return;  // no need to perform transforming.
#ifdef PROJ_NEW_API
  PJ_CONTEXT* C = proj_context_create();
  PJ* P = proj_create_crs_to_crs(C, od_geo_ref.c_str(), target_geo_ref.c_str(), /*  EPSG:4326 */
      NULL);
  for (auto& p : ego_path) {
      double x = std::get<0>(p), y = std::get<1>(p), z = std::get<2>(p);
      x += offset_west, y += offset_south;
      PJ_COORD a = proj_coord(x, y, z, 0);
      PJ_COORD b = proj_trans(P, PJ_FWD, a);
      x = b.xyz.x, y = b.xyz.x, z = b.xyz.z;
      std::get<0>(p) = x, std::get<1>(p) = y, std::get<2>(p) = z;
  }
  proj_destroy(P);
  proj_context_destroy(C);
#else
  projPJ pj_od = pj_init_plus(od_geo_ref.c_str());
  projPJ pj_lonlat = pj_init_plus(target_geo_ref.c_str());
  for (auto& p : ego_path) {
      double x = std::get<0>(p), y = std::get<1>(p), z = std::get<2>(p);
      x += offset_west, y += offset_south;
      int ec = pj_transform(pj_od, pj_lonlat, 1, 1, &x, &y, &z);
      if (ec) throw std::runtime_error("transforming points in xosc error: " + std::string(pj_strerrno(ec)));
      x *= RAD_TO_DEG, y *= RAD_TO_DEG;
      std::get<0>(p) = x, std::get<1>(p) = y, std::get<2>(p) = z;
  }
#endif /*PROJ_NEW_API*/
}

//!
//! @brief 函数名：VehicleXMLNode2GeometryMsg
//! @details 函数功能：将车辆XML节点转换为几何体消息
//!
//! @param[in] root 车辆XML节点
//! @param[out] geometry 几何体消息对象
//! @param[in] offset2front_x 前轴偏移量x
//! @param[in] offset2front_z 前轴偏移量z
//!
void VehicleXMLNode2GeometryMsg(const tinyxml2::XMLElement* root, sim_msg::VehicleGeometory* geometry,
                                double offset2front_x, double offset2front_z) {
  typedef sim_msg::VehicleGeometory VG;

  if (root->FirstChildElement("ReferencePoint")) {
    const std::string ref_origin =
        GetNodeAttribute(GetChildNode(root, "ReferencePoint"), "origin", "Catalog.Vehicle.Reference");
    if (ref_origin == VG::VehicleCoordType_Name(VG::FrontAxle_SnappedToGround))
      geometry->set_coord_type(VG::FrontAxle_SnappedToGround);
    else if (ref_origin == VG::VehicleCoordType_Name(VG::WheelBaseCenter_SnappedToGround))
      geometry->set_coord_type(VG::WheelBaseCenter_SnappedToGround);
    else if (ref_origin == VG::VehicleCoordType_Name(VG::RearAxle_SnappedToGround))
      geometry->set_coord_type(VG::RearAxle_SnappedToGround);
    else if (ref_origin == VG::VehicleCoordType_Name(VG::TrailerHitch_SnappedToGround))
      geometry->set_coord_type(VG::TrailerHitch_SnappedToGround);
    else
      throw std::runtime_error(std::string("unknown vehicle coordinate type: ") + ref_origin);
  }

  VG::VehicleGeometoryScalar* scalar = geometry->mutable_vehicle_geometory();
  VG::PointInVehicleCoord* coord = geometry->mutable_vehicle_coord();
  VG::WheelFour* wheel_max_steering = geometry->mutable_max_steering_radius();
  VG::WheelFour* wheel_diameter = geometry->mutable_wheel_diameter();

  const auto *bounding_box_node = GetChildNode(root, "BoundingBox"),
             *box_center_node = GetChildNode(bounding_box_node, "Center"),
             *box_dimension_node = GetChildNode(bounding_box_node, "Dimensions");
  const auto* axles_node = GetChildNode(root, "Axles");
  const auto* front_axle_node = axles_node->FirstChildElement("FrontAxle");
  const auto* rear_axle_node = axles_node->FirstChildElement("RearAxle");

  scalar->set_length(
      std::stod(GetNodeAttribute(box_dimension_node, "length", "Catalog.Vehicle.BoundingBox.Dimensions")));
  scalar->set_width(std::stod(GetNodeAttribute(box_dimension_node, "width", "Catalog.Vehicle.BoundingBox.Dimensions")));
  scalar->set_height(
      std::stod(GetNodeAttribute(box_dimension_node, "height", "Catalog.Vehicle.BoundingBox.Dimensions")));

  sim_msg::Vec3* v = coord->mutable_bounding_box_center();
  v->set_x(std::stod(GetNodeAttribute(box_center_node, "x", "Catalog.Vehicle.BoundingBox.Center")));
  v->set_y(std::stod(GetNodeAttribute(box_center_node, "y", "Catalog.Vehicle.BoundingBox.Center")));
  v->set_z(std::stod(GetNodeAttribute(box_center_node, "z", "Catalog.Vehicle.BoundingBox.Center")));

  sim_msg::Vec3* pCombination_offset = coord->mutable_combination_offset();
  pCombination_offset->set_x(offset2front_x);
  pCombination_offset->set_z(offset2front_z);

  v = coord->mutable_front_axle();
  if (front_axle_node) {
    v->set_x(std::stod(GetNodeAttribute(front_axle_node, "positionX", "Catalog.Vehicle.Axles.FrontAxle")));
    v->set_z(std::stod(GetNodeAttribute(front_axle_node, "positionZ", "Catalog.Vehicle.Axles.FrontAxle")));
  }
  v = coord->mutable_rear_axle();
  if (rear_axle_node) {
    v->set_x(std::stod(GetNodeAttribute(rear_axle_node, "positionX", "Catalog.Vehicle.Axles.RearAxle")));
    v->set_z(std::stod(GetNodeAttribute(rear_axle_node, "positionZ", "Catalog.Vehicle.Axles.RearAxle")));
  }

  if (front_axle_node) {
    scalar->set_ref_to_front_axle(std::fabs(coord->front_axle().x()));
    scalar->set_front_axle_to_front(scalar->length() / 2.0 + coord->bounding_box_center().x() -
                                    scalar->ref_to_front_axle());
    scalar->set_wheel_base(coord->front_axle().x() - coord->rear_axle().x());
    scalar->set_track_width_front(
        std::stod(GetNodeAttribute(front_axle_node, "trackWidth", "Catalog.Vehicle.Axles.FrontAxle")));
  }
  scalar->set_ref_to_rear_axle(std::fabs(coord->rear_axle().x()));
  scalar->set_rear_axle_to_rear(scalar->length() / 2.0 - coord->bounding_box_center().x() - scalar->ref_to_rear_axle());
  if (rear_axle_node) {
    scalar->set_track_width_rear(
        std::stod(GetNodeAttribute(rear_axle_node, "trackWidth", "Catalog.Vehicle.Axles.RearAxle")));
  }

  v = coord->mutable_wheel_base_center();
  if (front_axle_node) {
    v->set_x((coord->front_axle().x() + coord->rear_axle().x()) / 2.0);
    v->set_z((coord->front_axle().z() + coord->rear_axle().z()) / 2.0);
  } else {
    v->set_x(coord->rear_axle().x());
    v->set_z(coord->rear_axle().z());
  }
  v = coord->mutable_front();
  v->set_x(coord->bounding_box_center().x() + scalar->length() / 2.0);
  v = coord->mutable_rear();
  v->set_x(coord->bounding_box_center().x() - scalar->length() / 2.0);
  v = coord->mutable_front_left();
  v->set_x(coord->front().x());
  v->set_y(coord->bounding_box_center().y() + scalar->width() / 2.0);
  v = coord->mutable_front_right();
  v->set_x(coord->front().x());
  v->set_y(coord->bounding_box_center().y() - scalar->width() / 2.0);
  v = coord->mutable_rear_left();
  v->set_x(coord->rear().x());
  v->set_y(coord->bounding_box_center().y() + scalar->width() / 2.0);
  v = coord->mutable_rear_right();
  v->set_x(coord->rear().x());
  v->set_y(coord->bounding_box_center().y() - scalar->width() / 2.0);

  if (front_axle_node) {
    wheel_max_steering->set_front_left(
        std::stod(GetNodeAttribute(front_axle_node, "maxSteering", "Catalog.Vehicle.Axles.FrontAxle")));
    wheel_max_steering->set_front_right(wheel_max_steering->front_left());
    wheel_diameter->set_front_left(
        std::stod(GetNodeAttribute(front_axle_node, "wheelDiameter", "Catalog.Vehicle.Axles.FrontAxle")));
    wheel_diameter->set_front_right(wheel_diameter->front_right());
  }
  if (rear_axle_node) {
    wheel_max_steering->set_rear_left(
        std::stod(GetNodeAttribute(rear_axle_node, "maxSteering", "Catalog.Vehicle.Axles.FrontAxle")));
    wheel_max_steering->set_rear_right(wheel_max_steering->rear_left());
    wheel_diameter->set_rear_left(
        std::stod(GetNodeAttribute(rear_axle_node, "wheelDiameter", "Catalog.Vehicle.Axles.FrontAxle")));
    wheel_diameter->set_rear_right(wheel_diameter->rear_right());
  }
}

//!
//! @brief 函数名：GetRule
//! @details 函数功能：获取规则
//!
//! @param[in] sRule 规则字符串
//!
sim_msg::Rule GetRule(std::string sRule) {
  if (sRule == "eq") return sim_msg::Rule::RULE_EQUAL_TO;
  else if (sRule == "gt")
    return sim_msg::Rule::RULE_GREATER_THEN;
  else if (sRule == "gte")
    return sim_msg::Rule::RULE_GREATER_THEN_EQUAL;
  else if (sRule == "lt")
    return sim_msg::Rule::RULE_LESS_THEN;
  else if (sRule == "lte")
    return sim_msg::Rule::RULE_LESS_THEN_EQUAL;
  else
    return sim_msg::Rule::RULE_NOT_EQUAL_TO;
}

//!
//! @brief 函数名：SplitStrTrigger
//! @details 函数功能：分割触发器字符串
//!
//! @param[in] pNode 触发器XML节点
//! @param[out] pTrigger 触发器消息对象
//! @param[in] index 索引
//!
void SplitStrTrigger(const tinyxml2::XMLElement* pNode, sim_msg::Trigger* pTrigger, int index) {
  std::string typeKey("type");
  std::string conditionKey("condition");
  if (index > 0) {
    typeKey += std::to_string(index);
    conditionKey += std::to_string(index);
  }
  const char* pTypeKey = pNode->Attribute(typeKey.c_str());
  const char* pConditionKey = pNode->Attribute(conditionKey.c_str());
  if (!pTypeKey || !pConditionKey) return;

  std::string sCondition(pConditionKey);
  sim_msg::Condition* pCondition = pTrigger->add_conditions();

  std::vector<std::string> vecStr;
  std::unordered_map<std::string, std::string> mapValue;
  boost::split(vecStr, sCondition, boost::is_any_of(","));
  for (const std::string& item : vecStr) {
    std::string::size_type pos = item.find_first_of(':');
    if (pos != std::string::npos) {
      std::string key = item.substr(0, pos);
      std::string value = item.substr(pos + 1);
      mapValue[key] = value;
    }
  }
  std::string condition_boundary_value = mapValue["condition_boundary"];
  sim_msg::ConditionEdge conditionEdge;
  // none / rising /falling / both
  if (condition_boundary_value == "none") {
    conditionEdge = sim_msg::ConditionEdge::CONDITION_EDGE_NONE;
  } else if (condition_boundary_value == "rising") {
    conditionEdge = sim_msg::ConditionEdge::CONDITION_EDGE_RISING;
  } else if (condition_boundary_value == "falling") {
    conditionEdge = sim_msg::ConditionEdge::CONDITION_EDGE_FALLING;
  } else if (condition_boundary_value == "both") {
    conditionEdge = sim_msg::ConditionEdge::CONDITION_EDGE_BOTH;
  }
  if (!condition_boundary_value.empty()) { pCondition->set_edge(conditionEdge); }
  std::string trigger_count_value = mapValue["trigger_count"];
  if (!trigger_count_value.empty()) { pCondition->set_count(std::stod(trigger_count_value)); }
  std::string sRule = mapValue["equation_op"];
  sim_msg::Rule rule = GetRule(sRule);
  std::string sType(pTypeKey);
  if (sType == "velocity_trigger") {
    std::string speedType = mapValue["speed_type"];
    if (speedType == "absolute") {
      sim_msg::ConditionSpeed* pSpeed = pCondition->mutable_speed();
      pSpeed->set_rule(rule);
      std::string sValue = mapValue["speed_threshold"];
      pSpeed->set_value(stod(sValue));
    } else if (speedType == "relative") {
      sim_msg::ConditionRelativeSpeed* pSpeed = pCondition->mutable_relative_speed();
      pSpeed->set_entity_ref(mapValue["target_element"]);
      pSpeed->set_rule(rule);
      std::string sValue = mapValue["speed_threshold"];
      pSpeed->set_value(stod(sValue));
    }
  } else if (sType == "reach_abs_position") {
    std::string sPositionType = mapValue["position_type"];
    if (sPositionType == "absolute_position") {
      sim_msg::ConditionReachPosition* pPosition = pCondition->mutable_reach_position();
      std::string sValue = mapValue["radius"];
      pPosition->set_radius(stod(sValue));
      std::string sLon = mapValue["lon"];
      std::string sLat = mapValue["lat"];
      std::string sAlt = mapValue["alt"];
      sim_msg::PositionWorld* pWorld = pPosition->mutable_position()->mutable_world();
      pWorld->set_x(stod(sLon));
      pWorld->set_y(stod(sLat));
      pWorld->set_z(stod(sAlt));
    }

  } else if (sType == "reach_abs_lane") {
    std::string sPositionType = mapValue["position_type"];
    if (sPositionType == "absolute_lane") {
      sim_msg::ConditionReachPosition* pPosition = pCondition->mutable_reach_position();
      std::string sValue = mapValue["radius"];
      pPosition->set_radius(stod(sValue));
      std::string sLon = mapValue["lon"];
      std::string sLat = mapValue["lat"];
      std::string sAlt = mapValue["alt"];
      sim_msg::PositionWorld* pWorld = pPosition->mutable_position()->mutable_world();
      pWorld->set_x(stod(sLon));
      pWorld->set_y(stod(sLat));
      pWorld->set_z(stod(sAlt));
    }
  } else if (sType == "time_trigger") {
    sim_msg::ConditionSimulationTime* pConditionSimulationTime = pCondition->mutable_simulation_time();
    pConditionSimulationTime->set_value(std::stod(mapValue["trigger_time"]));
  } else if (sType == "ttc_trigger") {
    sim_msg::ConditionTimeToCollision* pttc = pCondition->mutable_ttc();
    pttc->set_entity_ref(mapValue["target_element"]);
    sim_msg::DistanceType distanceType;
    if (mapValue["distance_type"] == "euclideandistance") {
      distanceType = sim_msg::DistanceType::DISTANCE_TYPE_EUCLIDEAN;
    } else if (mapValue["distance_type"] == "laneprojection") {
      distanceType = sim_msg::DistanceType::DISTANCE_TYPE_LANEPROJECTION;
    }
    pttc->set_distance_type(distanceType);
    pttc->set_rule(rule);
    std::string sValue = mapValue["ttc_threshold"];
    pttc->set_value(stod(sValue));

  } else if (sType == "distance_trigger") {
    sim_msg::ConditionRelativeDistance* pdistance = pCondition->mutable_relative_distance();
    pdistance->set_entity_ref(mapValue["target_element"]);
    sim_msg::DistanceType distanceType;
    if (mapValue["distance_type"] == "euclideandistance") {
      distanceType = sim_msg::DistanceType::DISTANCE_TYPE_EUCLIDEAN;
    } else if (mapValue["distance_type"] == "laneprojection") {
      distanceType = sim_msg::DistanceType::DISTANCE_TYPE_LANEPROJECTION;
    }
    pdistance->set_distance_type(distanceType);
    pdistance->set_rule(rule);
    pdistance->set_value(stod(mapValue["distance_threshold"]));

  } else if (sType == "ego_attach_laneid_custom") {
    sim_msg::ConditionEgoAttachLaneidCustom* pConditionEgoAttachLaneidCustom =
        pCondition->mutable_ego_attach_laneid_custom();
    pConditionEgoAttachLaneidCustom->set_lane_id(std::stol(mapValue["lane_id"]));
    pConditionEgoAttachLaneidCustom->set_road_id(std::stoul(mapValue["road_it"]));
    pConditionEgoAttachLaneidCustom->set_section_id(std::stoul(mapValue["section_id"]));
  } else if (sType == "timeheadway_trigger") {
    sim_msg::ConditionTimeHeadway* pConditionTimeHeadway = pCondition->mutable_time_headway();
    std::string sEgo = mapValue["target_element"];
    std::string sValue = mapValue["value"];
    std::string sFressSpace = mapValue["freespace"];
    std::string sAlongRoute = mapValue["alongroute"];
    pConditionTimeHeadway->set_entity_ref(sEgo);
    pConditionTimeHeadway->set_value(stod(sValue));
    pConditionTimeHeadway->set_freespace(sFressSpace == "true" ? true : false);
    pConditionTimeHeadway->set_along_route(sAlongRoute == "true" ? true : false);
    pConditionTimeHeadway->set_rule(rule);
  } else if (sType == "element_state") {
    sim_msg::ConditionStoryboardElementState* pConditionStoryboardElementState =
        pCondition->mutable_storyboard_element_state();
    std::string sElementType = mapValue["element_type"];
    std::string sElementRef = mapValue["element_ref"];
    std::string sState = mapValue["state"];
    // story, act, maneuverGroup, maneuver, event,action
    if (sElementType == "action") {
      pConditionStoryboardElementState->set_type(sim_msg::StoryboardElementType::STORYBOARD_ELEMENT_TYPE_ACTION);
    } else if (sElementType == "story") {
      pConditionStoryboardElementState->set_type(sim_msg::StoryboardElementType::STORYBOARD_ELEMENT_TYPE_STORY);
    } else if (sElementType == "act") {
      pConditionStoryboardElementState->set_type(sim_msg::StoryboardElementType::STORYBOARD_ELEMENT_TYPE_ACT);
    } else if (sElementType == "maneuverGroup") {
      pConditionStoryboardElementState->set_type(
          sim_msg::StoryboardElementType::STORYBOARD_ELEMENT_TYPE_MANEUVER_GROUP);
    } else if (sElementType == "maneuver") {
      pConditionStoryboardElementState->set_type(sim_msg::StoryboardElementType::STORYBOARD_ELEMENT_TYPE_MANEUVER);
    } else if (sElementType == "event") {
      pConditionStoryboardElementState->set_type(sim_msg::StoryboardElementType::STORYBOARD_ELEMENT_TYPE_EVENT);
    }

    pConditionStoryboardElementState->set_storyboard_ref(sElementRef);

    if (sState == "runningState") {
      pConditionStoryboardElementState->set_state(
          sim_msg::StoryboardElementState::STORYBOARD_ELEMENT_STATE_RUNNING_STATE);
    } else if (sState == "completeState") {
      pConditionStoryboardElementState->set_state(
          sim_msg::StoryboardElementState::STORYBOARD_ELEMENT_STATE_COMPLETE_STATE);
    }

  } else {
    LOG(ERROR) << "SplitStrTrigger unknow type:" << sType;
  }
}

//!
//! @brief 函数名：BuildStatus
//! @details 函数功能：构建状态
//!
//! @param[out] pStatus 状态消息对象
//! @param[in] sSubType 子类型
//! @param[in] sValue 值
//! @param[in] sLaneOffSet 车道偏移量
//!
void BuildStatus(sim_msg::Status* pStatus, std::string sSubType, std::string sValue, std::string sLaneOffSet) {
  sim_msg::OnOff onoffValue = GetOnOff(sValue);
  if (sSubType == "emergency_brake") {
    pStatus->set_emergency_brake(onoffValue);
  } else if (sSubType == "vehicle_lost_control") {
    pStatus->set_vehicle_lost_control(onoffValue);
  } else if (sSubType == "hands_on_steeringwheel") {
    pStatus->set_hands_on_steeringwheel(onoffValue);
  } else if (sSubType == "eyes_on_road") {
    pStatus->set_eyes_on_road(onoffValue);
  } else if (sSubType == "lidar") {
    pStatus->set_lidar(onoffValue);
  } else if (sSubType == "radar") {
    pStatus->set_radar(onoffValue);
  } else if (sSubType == "ultrasonic") {
    pStatus->set_ultrasonic(onoffValue);
  } else if (sSubType == "camera") {
    pStatus->set_camera(onoffValue);
  } else if (sSubType == "gnss") {
    pStatus->set_gnss(onoffValue);
  } else if (sSubType == "imu") {
    pStatus->set_imu(onoffValue);
  } else if (sSubType == "localization") {
    pStatus->set_localization(onoffValue);
  } else if (sSubType == "beam") {
    sim_msg::BeamSts bsts;
    if (sLaneOffSet == "off") bsts = sim_msg::BeamSts::BEAM_STS_OFF;
    else if (sLaneOffSet == "auto")
      bsts = sim_msg::BeamSts::BEAM_STS_AUTO;
    else if (sLaneOffSet == "low")
      bsts = sim_msg::BeamSts::BEAM_STS_LOW;
    else if (sLaneOffSet == "high")
      bsts = sim_msg::BeamSts::BEAM_STS_HIGH;
    pStatus->set_beam(bsts);
  } else if (sSubType == "brake_light") {
    pStatus->set_brake_light(onoffValue);
  } else if (sSubType == "hazard_light") {
    pStatus->set_hazard_light(onoffValue);
  } else if (sSubType == "left_turn_light") {
    pStatus->set_left_turn_light(onoffValue);
  } else if (sSubType == "right_turn_light") {
    pStatus->set_right_turn_light(onoffValue);
  } else if (sSubType == "driver_seatbelt") {
    pStatus->set_driver_seatbelt(onoffValue);
  } else if (sSubType == "passenger_seatbelt") {
    pStatus->set_passenger_seatbelt(onoffValue);
  } else if (sSubType == "driver_door") {
    pStatus->set_driver_door(onoffValue);
  } else if (sSubType == "passenger_door") {
    pStatus->set_passenger_door(onoffValue);
  } else if (sSubType == "hood") {
    pStatus->set_hood(onoffValue);
  } else if (sSubType == "trunk") {
    pStatus->set_trunk(onoffValue);
  } else if (sSubType == "parkingbrake") {
    sim_msg::ParkingBrakeSts sts;
    if (sLaneOffSet == "released") sts = sim_msg::ParkingBrakeSts::PARKING_BRAKE_STS_RELEASED;
    else if (sLaneOffSet == "releasing")
      sts = sim_msg::ParkingBrakeSts::PARKING_BRAKE_STS_RELEASING;
    else if (sLaneOffSet == "applied")
      sts = sim_msg::ParkingBrakeSts::PARKING_BRAKE_STS_APPLIED;
    else if (sLaneOffSet == "applying")
      sts = sim_msg::ParkingBrakeSts::PARKING_BRAKE_STS_APPLYING;
    pStatus->set_parkingbrake(sts);
  } else if (sSubType == "wiper") {
    sim_msg::WiperSts sts;
    if (sLaneOffSet == "off") sts = sim_msg::WiperSts::WIPER_STS_OFF;
    else if (sLaneOffSet == "low")
      sts = sim_msg::WiperSts::WIPER_STS_LOW;
    else if (sLaneOffSet == "med")
      sts = sim_msg::WiperSts::WIPER_STS_MED;
    else if (sLaneOffSet == "high")
      sts = sim_msg::WiperSts::WIPER_STS_HIGH;
    pStatus->set_wiper(sts);
  } else if (sSubType == "gear") {
    sim_msg::GearSts sts;
    if (sLaneOffSet == "d") sts = sim_msg::GearSts::GEAR_STS_D;
    else if (sLaneOffSet == "p")
      sts = sim_msg::GearSts::GEAR_STS_P;
    else if (sLaneOffSet == "r")
      sts = sim_msg::GearSts::GEAR_STS_R;
    else if (sLaneOffSet == "n")
      sts = sim_msg::GearSts::GEAR_STS_N;
    else if (sLaneOffSet == "m1")
      sts = sim_msg::GearSts::GEAR_STS_M1;
    else if (sLaneOffSet == "m2")
      sts = sim_msg::GearSts::GEAR_STS_M2;
    else if (sLaneOffSet == "m3")
      sts = sim_msg::GearSts::GEAR_STS_M3;
    else if (sLaneOffSet == "m4")
      sts = sim_msg::GearSts::GEAR_STS_M4;
    else if (sLaneOffSet == "m5")
      sts = sim_msg::GearSts::GEAR_STS_M5;
    else if (sLaneOffSet == "m6")
      sts = sim_msg::GearSts::GEAR_STS_M6;
    else if (sLaneOffSet == "m7")
      sts = sim_msg::GearSts::GEAR_STS_M7;
    pStatus->set_gear(sts);
  } else if (sSubType == "key") {
    sim_msg::KeySts sts;
    if (sLaneOffSet == "off") sts = sim_msg::KeySts::KEY_STS_OFF;
    else if (sLaneOffSet == "ign")
      sts = sim_msg::KeySts::KEY_STS_IGN;
    else if (sLaneOffSet == "acc")
      sts = sim_msg::KeySts::KEY_STS_ACC;
    else if (sLaneOffSet == "on")
      sts = sim_msg::KeySts::KEY_STS_ON;
    pStatus->set_key(sts);
  } else if (sSubType == "userdefine") {
    pStatus->set_user_define(sValue.c_str());
  }
}

//!
//! @brief 函数名：AddWayPointByWorld
//! @details 函数功能：根据世界坐标添加路径点
//!
//! @param[in] sPoint 路径点字符串
//! @param[out] pWayPoint 路径点消息对象
//!
void AddWayPointByWorld(std::string sPoint, sim_msg::Waypoint* pWayPoint) {
  if (sPoint.empty()) return;
  sim_msg::Position* pPositon = pWayPoint->mutable_position();
  sim_msg::PositionWorld* pPositonWorld = pPositon->mutable_world();
  std::vector<std::string> point_triple;
  ParsePosStr(sPoint, point_triple);
  pPositonWorld->set_x(std::stod(point_triple[0]));
  pPositonWorld->set_y(std::stod(point_triple[1]));
}

//!
//! @brief 函数名：ParseEndCondition
//! @details 函数功能：解析结束条件字符串
//!
//! @param[in] sEndCondition 结束条件字符串
//!
std::vector<std::tuple<std::string, std::string>> ParseEndCondition(const std::string& sEndCondition) {
  std::vector<std::tuple<std::string, std::string>> ret;
  std::vector<std::string> vecOrigins;
  std::string sType;
  std::string sValue;
  std::vector<std::string> vectypeValueOrigin;
  std::vector<std::string> vecTypeValue;


  boost::split(vecOrigins, sEndCondition, boost::is_any_of(";"));
  for (auto& itemOrigin : vecOrigins) {
    vectypeValueOrigin.clear();
    boost::split(vectypeValueOrigin, itemOrigin, boost::is_any_of(","));
    vecTypeValue.clear();
    boost::split(vecTypeValue, vectypeValueOrigin[0], boost::is_any_of(":"));
    sType = vecTypeValue[1];

    vecTypeValue.clear();
    boost::split(vecTypeValue, vectypeValueOrigin[1], boost::is_any_of(":"));
    sValue = vecTypeValue[1];

    ret.emplace_back(std::make_tuple(sType, sValue));
  }
  return ret;
}

//!
//! @brief 函数名：GetEndType
//! @details 函数功能：获取结束类型
//!
//! @param[in] sType 结束类型字符串
//!
sim_msg::ActionAccel::EndType GetEndType(std::string sType) {
  if (sType == "time") return sim_msg::ActionAccel_EndType_END_TYPE_TIME;
  else if (sType == "speed")
    return sim_msg::ActionAccel_EndType_END_TYPE_SPEED;
  return sim_msg::ActionAccel_EndType_END_TYPE_NONE;
}

sim_msg::OnOff GetOnOff(std::string sOnOff) {
  return sOnOff == "true" ? sim_msg::OnOff::ONOFF_ON : sim_msg::OnOff::ONOFF_OFF;
}
}  // namespace utils
}  // namespace tx_sim
