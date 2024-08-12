/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "xml_parser/entity/scene_event.h"
#include <boost/algorithm/string.hpp>

SceneEvent::SceneEvent() {
  strSpeedType = "m_s";
  m_id = "";
  strName = "";
  strType = "";

  strCondition = "";
  strEndCondition = "";
  strAction = "";
  isUserDefined = false;
}
SceneEvent::~SceneEvent() {}
// 拆分
void SceneEvent::SegmentString() {
  std::vector<std::string> strCondition_K_V;
  boost::algorithm::split(strCondition_K_V, strCondition, boost::algorithm::is_any_of(","));

  std::map<std::string, std::string> map_keyvalue;
  for (int i = 0; i < strCondition_K_V.size(); i++) {
    std::vector<std::string> strK_V;
    boost::algorithm::split(strK_V, strCondition_K_V[i], boost::algorithm::is_any_of(":"));
    if (strK_V.size() > 1) map_keyvalue.insert(std::make_pair(strK_V[0], strK_V[1]));
  }

  if (strType == "time_trigger") {
    auto itr = map_keyvalue.find("trigger_time");
    if (itr != map_keyvalue.end()) strTriggerTime = itr->second;
  } else if (strType == "ttc_trigger") {
    auto itr = map_keyvalue.find("ttc_threshold");
    if (itr != map_keyvalue.end()) strThreshold = itr->second;

    itr = map_keyvalue.find("equation_op");
    if (itr != map_keyvalue.end()) strEquationOp = itr->second;

    itr = map_keyvalue.find("target_element");
    if (itr != map_keyvalue.end()) strTargetElement = itr->second;

    itr = map_keyvalue.find("distance_type");
    if (itr != map_keyvalue.end()) strDisType = itr->second;

    itr = map_keyvalue.find("condition_boundary");
    if (itr != map_keyvalue.end()) strBoundary = itr->second;

    itr = map_keyvalue.find("trigger_count");
    if (itr != map_keyvalue.end()) strCount = itr->second;

  } else if (strType == "distance_trigger") {
    auto itr = map_keyvalue.find("distance_threshold");
    if (itr != map_keyvalue.end()) strThreshold = itr->second;

    itr = map_keyvalue.find("equation_op");
    if (itr != map_keyvalue.end()) strEquationOp = itr->second;

    itr = map_keyvalue.find("target_element");
    if (itr != map_keyvalue.end()) strTargetElement = itr->second;

    itr = map_keyvalue.find("distance_type");
    if (itr != map_keyvalue.end()) strDisType = itr->second;

    itr = map_keyvalue.find("condition_boundary");
    if (itr != map_keyvalue.end()) strBoundary = itr->second;

    itr = map_keyvalue.find("trigger_count");
    if (itr != map_keyvalue.end()) strCount = itr->second;
  } else if (strType == "velocity_trigger") {
    auto itr = map_keyvalue.find("speed_threshold");
    if (itr != map_keyvalue.end()) strThreshold = itr->second;

    itr = map_keyvalue.find("equation_op");
    if (itr != map_keyvalue.end()) strEquationOp = itr->second;

    itr = map_keyvalue.find("source_element");
    if (itr != map_keyvalue.end()) strSourceElement = itr->second;

    itr = map_keyvalue.find("target_element");
    if (itr != map_keyvalue.end()) strTargetElement = itr->second;

    itr = map_keyvalue.find("distance_type");
    if (itr != map_keyvalue.end()) strDisType = itr->second;

    itr = map_keyvalue.find("condition_boundary");
    if (itr != map_keyvalue.end()) strBoundary = itr->second;

    itr = map_keyvalue.find("trigger_count");
    if (itr != map_keyvalue.end()) strCount = itr->second;

    itr = map_keyvalue.find("speed_type");
    if (itr != map_keyvalue.end()) strSpeedType = itr->second;

    itr = map_keyvalue.find("speed_unit");
    if (itr != map_keyvalue.end()) strSpeedUnit = itr->second;

  } else if (strType == "reach_abs_position") {
    auto itr = map_keyvalue.find("position_type");
    if (itr != map_keyvalue.end()) strPostionType = itr->second;

    itr = map_keyvalue.find("equation_op");
    if (itr != map_keyvalue.end()) strEquationOp = itr->second;

    itr = map_keyvalue.find("lon");
    if (itr != map_keyvalue.end()) strLon = itr->second;

    itr = map_keyvalue.find("lat");
    if (itr != map_keyvalue.end()) strLat = itr->second;

    itr = map_keyvalue.find("alt");
    if (itr != map_keyvalue.end()) strAlt = itr->second;

    itr = map_keyvalue.find("radius");
    if (itr != map_keyvalue.end()) strRadius = itr->second;

    itr = map_keyvalue.find("target_element");
    if (itr != map_keyvalue.end()) strTargetElement = itr->second;

    itr = map_keyvalue.find("condition_boundary");
    if (itr != map_keyvalue.end()) strBoundary = itr->second;

    itr = map_keyvalue.find("trigger_count");
    if (itr != map_keyvalue.end()) strCount = itr->second;
  } else if (strType == "reach_abs_lane") {
    auto itr = map_keyvalue.find("position_type");
    if (itr != map_keyvalue.end()) strPostionType = itr->second;

    itr = map_keyvalue.find("road_id");
    if (itr != map_keyvalue.end()) strRoadId = itr->second;

    itr = map_keyvalue.find("lane_id");
    if (itr != map_keyvalue.end()) strLaneId = itr->second;

    itr = map_keyvalue.find("target_element");
    if (itr != map_keyvalue.end()) strTargetElement = itr->second;

    itr = map_keyvalue.find("lateral_offset");
    if (itr != map_keyvalue.end()) strLateralOffset = itr->second;

    itr = map_keyvalue.find("longitudinal_offset");
    if (itr != map_keyvalue.end()) strLongitudinalOffset = itr->second;

    itr = map_keyvalue.find("tolerance");
    if (itr != map_keyvalue.end()) strTolerance = itr->second;

    itr = map_keyvalue.find("condition_boundary");
    if (itr != map_keyvalue.end()) strBoundary = itr->second;

    itr = map_keyvalue.find("trigger_count");
    if (itr != map_keyvalue.end()) strCount = itr->second;
  } else if (strType == "ego_attach_laneid_custom") {
    auto itr = map_keyvalue.find("variate");
    if (itr != map_keyvalue.end()) strVariate = itr->second;

    itr = map_keyvalue.find("road_id");
    if (itr != map_keyvalue.end()) strRoadId = itr->second;

    itr = map_keyvalue.find("section_id");
    if (itr != map_keyvalue.end()) strSectionId = itr->second;

    itr = map_keyvalue.find("lane_id");
    if (itr != map_keyvalue.end()) strLaneId = itr->second;

    itr = map_keyvalue.find("trigger_count");
    if (itr != map_keyvalue.end()) strCount = itr->second;

    // itr = map_keyvalue.find("condition_boundary");
    // if (itr != map_keyvalue.end())   strBoundary = itr->second;
  } else {
  }

  std::vector<std::string> strEndCondition_K_V;
  boost::algorithm::split(strEndCondition_K_V, strEndCondition, boost::algorithm::is_any_of(";"));
  for (int i = 0; i < strEndCondition_K_V.size(); i++) {
    std::vector<std::string> strSingle;
    boost::algorithm::split(strSingle, strEndCondition_K_V[i], boost::algorithm::is_any_of(","));

    std::map<std::string, std::string> mapKV;
    for (int j = 0; j < strSingle.size(); j++) {
      std::vector<std::string> strK_V;
      boost::algorithm::split(strK_V, strSingle[j], boost::algorithm::is_any_of(":"));
      if (strK_V.size() > 1) mapKV.insert(std::make_pair(strK_V[0], strK_V[1]));
    }
    Scene_EndCondition temp;
    auto itr = mapKV.find("type");
    if (itr != mapKV.end()) temp.strType = itr->second;
    itr = mapKV.find("value");
    if (itr != mapKV.end()) temp.strValue = itr->second;

    EndConditionVer.push_back(temp);
  }

  std::vector<std::string> strAction_K_V;
  boost::algorithm::split(strAction_K_V, strAction, boost::algorithm::is_any_of(";"));
  for (int i = 0; i < strAction_K_V.size(); i++) {
    std::vector<std::string> strSingle;
    boost::algorithm::split(strSingle, strAction_K_V[i], boost::algorithm::is_any_of(","));

    std::map<std::string, std::string> mapKV;
    for (int j = 0; j < strSingle.size(); j++) {
      std::vector<std::string> strK_V;
      boost::algorithm::split(strK_V, strSingle[j], boost::algorithm::is_any_of(":"));
      if (strK_V.size() > 1) mapKV.insert(std::make_pair(strK_V[0], strK_V[1]));
    }
    Scene_Action temp;
    auto itr = mapKV.find("type");
    if (itr != mapKV.end()) temp.strType = itr->second;
    itr = mapKV.find("value");
    if (itr != mapKV.end()) temp.strValue = itr->second;
    itr = mapKV.find("subtype");
    if (itr != mapKV.end()) temp.strSubType = itr->second;
    itr = mapKV.find("laneoffset");
    if (itr != mapKV.end()) temp.strLaneOffset = itr->second;
    itr = mapKV.find("checked");
    if (itr != mapKV.end()) temp.strChecked = itr->second;

    ActionVer.push_back(temp);
  }
}
void SceneEvent::MergeString() {
  if (strType == "time_trigger") {
    strCondition = "trigger_time:";
    strCondition.append(strTriggerTime);
  } else if (strType == "ttc_trigger") {
    strCondition = "ttc_threshold:";
    strCondition.append(strThreshold);
    strCondition.append(",equation_op:");
    strCondition.append(strEquationOp);
    strCondition.append(",target_element:");
    strCondition.append(strTargetElement);
    strCondition.append(",distance_type:");
    strCondition.append(strDisType);
    strCondition.append(",condition_boundary:");
    strCondition.append(strBoundary);
    strCondition.append(",trigger_count:");
    strCondition.append(strCount);

  } else if (strType == "distance_trigger") {
    strCondition = "distance_threshold:";
    strCondition.append(strThreshold);
    strCondition.append(",equation_op:");
    strCondition.append(strEquationOp);
    strCondition.append(",target_element:");
    strCondition.append(strTargetElement);
    strCondition.append(",distance_type:");
    strCondition.append(strDisType);
    strCondition.append(",condition_boundary:");
    strCondition.append(strBoundary);
    strCondition.append(",trigger_count:");
    strCondition.append(strCount);
  } else if (strType == "velocity_trigger") {
    strCondition = "speed_type:";
    strCondition.append(strSpeedType);
    strCondition.append(",speed_unit:");
    strCondition.append(strSpeedUnit);
    strCondition.append(",speed_threshold:");
    strCondition.append(strThreshold);
    strCondition.append(",equation_op:");
    strCondition.append(strEquationOp);
    strCondition.append(",source_element:");
    strCondition.append(strSourceElement);
    strCondition.append(",target_element:");
    strCondition.append(strTargetElement);
    strCondition.append(",distance_type:");
    strCondition.append(strDisType);
    strCondition.append(",condition_boundary:");
    strCondition.append(strBoundary);
    strCondition.append(",trigger_count:");
    strCondition.append(strCount);

  } else if (strType == "reach_abs_position") {
    strCondition = "position_type:";
    strCondition.append(strPostionType);
    strCondition.append(",lon:");
    strCondition.append(strLon);
    strCondition.append(",lat:");
    strCondition.append(strLat);
    strCondition.append(",alt:");
    strCondition.append(strAlt);
    strCondition.append(",radius:");
    strCondition.append(strRadius);
    strCondition.append(",target_element:");
    strCondition.append(strTargetElement);
    strCondition.append(",condition_boundary:");
    strCondition.append(strBoundary);
    strCondition.append(",trigger_count:");
    strCondition.append(strCount);
  } else if (strType == "reach_abs_lane") {
    strCondition = "position_type:";
    strCondition.append(strPostionType);
    strCondition.append(",road_id:");
    strCondition.append(strRoadId);
    strCondition.append(",lane_id:");
    strCondition.append(strLaneId);
    strCondition.append(",target_element:");
    strCondition.append(strTargetElement);
    strCondition.append(",lateral_offset:");
    strCondition.append(strLateralOffset);
    strCondition.append(",longitudinal_offset:");
    strCondition.append(strLongitudinalOffset);
    strCondition.append(",tolerance:");
    strCondition.append(strTolerance);
    strCondition.append(",condition_boundary:");
    strCondition.append(strBoundary);
    strCondition.append(",trigger_count:");
    strCondition.append(strCount);
  } else if (strType == "ego_attach_laneid_custom") {
    strCondition = "variate:";
    strCondition.append(strVariate);
    strCondition.append(",road_id:");
    strCondition.append(strRoadId);
    strCondition.append(",section_id:");
    strCondition.append(strSectionId);
    strCondition.append(",lane_id:");
    strCondition.append(strLaneId);
    // strCondition.append(",condition_boundary:");
    // strCondition.append(strBoundary);
    strCondition.append(",trigger_count:");
    strCondition.append(strCount);
  } else {
  }

  strEndCondition = "";
  for (int i = 0; i < EndConditionVer.size(); i++) {
    strEndCondition.append("type:");
    strEndCondition.append(EndConditionVer[i].strType);
    strEndCondition.append(",value:");
    strEndCondition.append(EndConditionVer[i].strValue);
    if (i < EndConditionVer.size() - 1) {
      strEndCondition.append(";");
    }
  }

  strAction = "";
  for (int i = 0; i < ActionVer.size(); i++) {
    strAction.append("type:");
    strAction.append(ActionVer[i].strType);
    strAction.append(",value:");
    strAction.append(ActionVer[i].strValue);
    strAction.append(",subtype:");
    strAction.append(ActionVer[i].strSubType);
    strAction.append(",laneoffset:");
    strAction.append(ActionVer[i].strLaneOffset);
    strAction.append(",checked:");
    strAction.append(ActionVer[i].strChecked);
    if (i < ActionVer.size() - 1) {
      strAction.append(";");
    }
  }
}
