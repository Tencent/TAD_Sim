// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "location.pb.h"
#include "scene.pb.h"
#include "tinyxml2.h"
#include "traffic.pb.h"
#include "vehicle_geometry.pb.h"
#include "xml_util.h"

using namespace tinyxml2;

namespace tx_sim {
namespace utils {
//!
//! @brief 函数名：GetXMLRoot
//! @details 函数功能：获取XML文件的根节点
//!
//! @param[in] path XML文件路径
//! @param[in] doc XML文档对象
//!
XMLElement* GetXMLRoot(const std::string& path, tinyxml2::XMLDocument& doc);

//!
//! @brief 函数名：GetChildNode
//! @details 函数功能：获取子节点
//!
//! @param[in] f 父节点
//! @param[in] name 子节点名称
//!
const XMLElement* GetChildNode(const XMLElement* f, const char* name);

//!
//! @brief 函数名：GetChildNode
//! @details 函数功能：获取具有指定属性值的子节点
//!
//! @param[in] f 父节点
//! @param[in] name 子节点名称
//! @param[in] key 属性名称
//! @param[in] value 属性值
//!
const XMLElement* GetChildNode(const XMLElement* f, const char* name, const char* key, const char* value);

//!
//! @brief 函数名：TryGetChildNode
//! @details 函数功能：尝试获取子节点，如果不存在则返回nullptr
//!
//! @param[in] f 父节点
//! @param[in] name 子节点名称
//!
const XMLElement* TryGetChildNode(const XMLElement* f, const char* name);

//!
//! @brief 函数名：GetNodeAttribute
//! @details 函数功能：获取节点属性值
//!
//! @param[in] node 节点对象
//! @param[in] attr_name 属性名称
//! @param[in] node_name 节点名称
//!
std::string GetNodeAttribute(const XMLElement* node, const char* attr_name, const std::string& node_name);

//!
//! @brief 函数名：GetNodeAttributeWithDefault
//! @details 函数功能：获取节点属性值，如果不存在则返回默认值
//!
//! @param[in] node 节点对象
//! @param[in] attr_name 属性名称
//! @param[in] defaultValue 默认值
//!
std::string GetNodeAttributeWithDefault(const XMLElement* node, const char* attr_name, const std::string& defaultValue);

//!
//! @brief 函数名：GetNodeText
//! @details 函数功能：获取节点文本内容
//!
//! @param[in] node 节点对象
//! @param[in] node_name 节点名称
//! @param[in] trim_copy 是否去除文本内容的空白字符
//!
std::string GetNodeText(const XMLElement* node, const std::string& node_name, bool trim_copy = false);

//!
//! @brief 函数名：TryGetNodeWithAttributeInList
//! @details 函数功能：尝试在节点列表中获取具有指定属性值的节点，如果不存在则返回nullptr
//!
//! @param[in] current_node 当前节点
//! @param[in] attr_name 属性名称
//! @param[in] attr_value 属性值
//!
const XMLElement* TryGetNodeWithAttributeInList(const XMLElement* current_node, const char* attr_name,
                                                const char* attr_value);

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
                                             const char* attr_value, const std::string& log_name);

//!
//! @brief 函数名：GetGrandChildInList
//! @details 函数功能：获取子节点列表中的孙子节点
//!
//! @param[in] current_node 当前节点
//! @param[in] child_name 子节点名称
//! @param[in] log_name 日志名称
//!
const XMLElement* GetGrandChildInList(const XMLElement* current_node, const char* child_name,
                                      const std::string& log_name);

//!
//! @brief 函数名：ParsePosStr
//! @details 函数功能：解析位置字符串
//!
//! @param[in] s 位置字符串
//! @param[out] ret 解析后的位置数据
//!
void ParsePosStr(const std::string& s, std::vector<std::string>& ret);

//!
//! @brief 函数名：TransformEgoPathCoordinates
//! @details 函数功能：转换自车路径坐标
//!
//! @param[in] od_map_path 路径地图文件路径
//! @param[out] ego_path 自车路径数据
//!
void TransformEgoPathCoordinates(const std::string& od_map_path,
                                 std::vector<std::tuple<double, double, double>>& ego_path);

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
                                double offset2front_x = 0, double offset2front_z = 0);

//!
//! @brief 函数名：GetRule
//! @details 函数功能：获取规则
//!
//! @param[in] sRule 规则字符串
//!
sim_msg::Rule GetRule(std::string sRule);

//!
//! @brief 函数名：SplitStrTrigger
//! @details 函数功能：分割触发器字符串
//!
//! @param[in] pNode 触发器XML节点
//! @param[out] pTrigger 触发器消息对象
//! @param[in] index 索引
//!
void SplitStrTrigger(const tinyxml2::XMLElement* pNode, sim_msg::Trigger* pTrigger, int index = 0);

//!
//! @brief 函数名：BuildStatus
//! @details 函数功能：构建状态
//!
//! @param[out] pStatus 状态消息对象
//! @param[in] sSubType 子类型
//! @param[in] sValue 值
//! @param[in] sLaneOffSet 车道偏移量
//!
void BuildStatus(sim_msg::Status* pStatus, std::string sSubType, std::string sValue, std::string sLaneOffSet);

//!
//! @brief 函数名：GetOnOff
//! @details 函数功能：获取开关状态
//!
//! @param[in] sOnOff 开关状态字符串
//!
sim_msg::OnOff GetOnOff(std::string sOnOff);

//!
//! @brief 函数名：AddWayPointByWorld
//! @details 函数功能：根据世界坐标添加路径点
//!
//! @param[in] sPoint 路径点字符串
//! @param[out] pWayPoint 路径点消息对象
//!
void AddWayPointByWorld(std::string sPoint, sim_msg::Waypoint* pWayPoint);

//!
//! @brief 函数名：ParseEndCondition
//! @details 函数功能：解析结束条件字符串
//!
//! @param[in] sEndCondition 结束条件字符串
//!
std::vector<std::tuple<std::string, std::string>> ParseEndCondition(const std::string& sEndCondition);

//!
//! @brief 函数名：GetEndType
//! @details 函数功能：获取结束类型
//!
//! @param[in] sType 结束类型字符串
//!
sim_msg::ActionAccel::EndType GetEndType(std::string sType);
}  // namespace utils
}  // namespace tx_sim
