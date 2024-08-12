/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/scene_event.h"
#include <json/reader.h>
#include <json/writer.h>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <fstream>
#include <regex>
#include <sstream>
#include "common/log/system_logger.h"
#include "common/utils/misc.h"
SceneEvent::SceneEvent() {
  m_id = "";
  strName = "";
  // strCondition = "";
  strEndCondition = "";
  strAction = "";
  isUserDefined = false;
}

SceneEvent::~SceneEvent() {}

bool SceneEvent::fromJson2Xml(const Json::Value& json, const std::string& ver) {
  auto set_without_empty = [](const std::string& input, std::string& output) {
    if (!input.empty()) {
      output = input;
    }
  };
  this->m_id = json["id"].asString();
  this->strName = json["name"].asString();
  this->strInfo = json["info"].asString();
  // condition
  if (ver == "1.0.0.0" || ver == "1.1.0.0") {
    SceneCondition tmp;
    tmp.strType = json.get("type", "").asString();
    tmp.strTriggerTime = json.get("triggertime", "").asString();
    tmp.strThreshold = json.get("threshold", "").asString();
    tmp.strEquationOp = json.get("equationop", "").asString();
    tmp.strDisType = json.get("distype", "").asString();
    tmp.strBoundary = json.get("boundary", "").asString();
    tmp.strCount = json.get("count", "").asString();
    tmp.strSpeedUnit = json.get("unit", "").asString();
    tmp.strSpeedType = json.get("speedtype", "").asString();
    tmp.strTargetElement = json.get("targetelement", "").asString();
    tmp.strSourceElement = json.get("sourceelement", "").asString();
    tmp.strPostionType = json.get("positiontype", "").asString();
    tmp.strLon = json.get("lon", "").asString();
    tmp.strLat = json.get("lat", "").asString();
    tmp.strAlt = json.get("alt", "").asString();
    tmp.strRadius = json.get("radius", "").asString();
    tmp.strVariate = json.get("variate", "").asString();
    tmp.strRoadId = json.get("roadid", "").asString();
    tmp.strSectionId = json.get("sectionid", "").asString();
    tmp.strLaneId = json.get("laneid", "").asString();
    tmp.strLateralOffset = json.get("lateraloffset", "").asString();
    tmp.strLongitudinalOffset = json.get("longitudinaloffset", "").asString();
    tmp.strTolerance = json.get("tolerance", "").asString();
    tmp.strElementType = json.get("element_type", "").asString();
    tmp.strElementRef = json.get("element_ref", "").asString();
    tmp.strState = json.get("state", "").asString();
    this->ConditionVec.push_back(tmp);
  } else {
    Json::Value conditions = json.get("conditions", Json::Value(Json::arrayValue));
    for (auto _it : conditions) {
      SceneCondition tmp;
      tmp.strType = _it.get("type", "").asString();
      tmp.strTriggerTime = _it.get("triggertime", "").asString();
      tmp.strThreshold = _it.get("threshold", "").asString();
      tmp.strEquationOp = _it.get("equationop", "").asString();
      tmp.strDisType = _it.get("distype", "").asString();
      tmp.strBoundary = _it.get("boundary", "").asString();
      tmp.strCount = _it.get("count", "").asString();
      tmp.strSpeedUnit = _it.get("unit", "").asString();
      tmp.strSpeedType = _it.get("speedtype", "").asString();
      tmp.strTargetElement = _it.get("targetelement", "").asString();
      tmp.strSourceElement = _it.get("sourceelement", "").asString();
      tmp.strPostionType = _it.get("positiontype", "").asString();
      tmp.strLon = _it.get("lon", "").asString();
      tmp.strLat = _it.get("lat", "").asString();
      tmp.strAlt = _it.get("alt", "").asString();
      tmp.strRadius = _it.get("radius", "").asString();
      tmp.strVariate = _it.get("variate", "").asString();
      tmp.strRoadId = _it.get("roadid", "").asString();
      tmp.strSectionId = _it.get("sectionid", "").asString();
      tmp.strLaneId = _it.get("laneid", "").asString();
      tmp.strLateralOffset = _it.get("lateraloffset", "").asString();
      tmp.strLongitudinalOffset = _it.get("longitudinaloffset", "").asString();
      tmp.strTolerance = _it.get("tolerance", "").asString();
      tmp.strElementType = _it.get("element_type", "").asString();
      tmp.strElementRef = _it.get("element_ref", "").asString();
      tmp.strState = _it.get("state", "").asString();
      this->ConditionVec.push_back(tmp);
    }
  }
  // action
  if (!json["action"].isNull()) {
    int index = 0;
    for (auto endItr = json["action"].begin(); endItr != json["action"].end(); ++endItr) {
      Scene_Action temp;
      temp.strType = endItr->get("actiontype", "none").asString();
      set_without_empty(endItr->get("actionvalue", "0.0").asString(), temp.strValue);
      set_without_empty(endItr->get("subtype", "none").asString(), temp.strSubType);
      set_without_empty(endItr->get("offset", "0").asString(), temp.strLaneOffset);
      set_without_empty(endItr->get("actionid", "-1").asString(), temp.strId);
      if (temp.strId == "-1") {
        temp.strId = std::to_string(index++);
      }
      if (!(*endItr)["multi"].isNull()) {
        Scene_Action_multi& multi = temp.sActionMulti;
        Json::Value jsonMulti = (*endItr)["multi"];
        if (temp.strType == "assign") {
          multi.flag = jsonMulti["isValid"].asBool();
          if ((*endItr)["multi"]["isValid"].asBool()) {
            if (true == temp.sActionMulti.flag) {
              multi.sAssign.s_resume_sw = bool2String(jsonMulti["resume_sw"].asBool());
              multi.sAssign.s_cancel_sw = bool2String(jsonMulti["cancel_sw"].asBool());
              multi.sAssign.s_speed_inc_sw = bool2String(jsonMulti["speed_inc_sw"].asBool());
              multi.sAssign.s_speed_dec_sw = bool2String(jsonMulti["speed_dec_sw"].asBool());
              multi.sAssign.s_set_timegap = (jsonMulti["set_timegap"].asString());
              multi.sAssign.s_set_speed = (jsonMulti["set_speed"].asString());
            }
          }
        } else if (temp.strType == "override") {
          multi.sOveride.s_throttle = jsonMulti["throttle"].asString();
          multi.sOveride.s_brake = jsonMulti["brake"].asString();
          multi.sOveride.s_steering_wheel = jsonMulti["steering_wheel"].asString();
          multi.sOveride.s_parking_brake = jsonMulti["parking_brake"].asString();
          multi.sOveride.s_clutch = jsonMulti["clutch"].asString();
          multi.sOveride.s_gear = jsonMulti["gear"].asString();
          multi.flag = true;
        } else if (temp.strType == "lateralDistance") {
          multi.sLateralDistance.s_target_element = jsonMulti.get("target_element", "").asString();
          multi.sLateralDistance.s_maxAcc = jsonMulti.get("maxAcc", "").asString();
          multi.sLateralDistance.s_maxDec = jsonMulti.get("maxDec", "").asString();
          multi.sLateralDistance.s_maxSpeed = jsonMulti.get("maxSpeed", "").asString();
          multi.sLateralDistance.s_continuous = bool2String(jsonMulti.get("continuous", "").asBool());
          multi.flag = true;
        }
      }
      temp.strChecked = endItr->get("checked", "true").asString();
      this->ActionVer.push_back(temp);
    }
  }
  // endCondition in v1.0
  Json::Value end_conditions = json.get("endCondition", Json::Value(Json::arrayValue));
  if (end_conditions.empty()) {
    end_conditions = json.get("action", Json::Value(Json::arrayValue));
  }
  for (auto&& condition : end_conditions) {
    Scene_EndCondition one;
    set_without_empty(condition.get("endconditiontype", "none").asString(), one.strType);
    set_without_empty(condition.get("endconditionvalue", "0.0").asString(), one.strValue);
    this->EndConditionVer.emplace_back(one);
  }
  if (ver == "1.0.0.0") {
    this->MergeString1_0();
  } else {
    this->MergeString();
  }
  return true;
}

Json::Value SceneEvent::saveJson(const std::string& ver) {
  Json::Value onesceneevent;
  onesceneevent["id"] = this->m_id;
  onesceneevent["name"] = this->strName;
  // conditions
  if (ver == "1.1.0.0" || ver == "1.0.0.0") {
    for (auto it : this->ConditionVec) {
      onesceneevent["type"] = it.strType;
      onesceneevent["triggertime"] = it.strTriggerTime;
      onesceneevent["threshold"] = it.strThreshold;
      onesceneevent["equationop"] = it.strEquationOp;
      onesceneevent["distype"] = it.strDisType;
      onesceneevent["boundary"] = it.strBoundary;
      onesceneevent["count"] = it.strCount;
      onesceneevent["unit"] = it.strSpeedUnit;
      onesceneevent["targetelement"] = it.strTargetElement;
      onesceneevent["sourceelement"] = it.strSourceElement;
      onesceneevent["positiontype"] = it.strPostionType;
      onesceneevent["speedtype"] = it.strSpeedType;
      onesceneevent["lon"] = it.strLon;
      onesceneevent["lat"] = it.strLat;
      onesceneevent["alt"] = it.strAlt;
      onesceneevent["radius"] = it.strRadius;
      onesceneevent["variate"] = it.strVariate;
      onesceneevent["roadid"] = it.strRoadId;
      onesceneevent["sectionid"] = it.strSectionId;
      onesceneevent["laneid"] = it.strLaneId;
      onesceneevent["lateraloffset"] = it.strLateralOffset;
      onesceneevent["longitudinaloffset"] = it.strLongitudinalOffset;
      onesceneevent["tolerance"] = it.strTolerance;
      onesceneevent["element_type"] = it.strElementType;
      onesceneevent["element_ref"] = it.getElementRef();
      onesceneevent["state"] = it.strState;
    }
    onesceneevent["info"] = this->strInfo;
  } else {  // version  >= 1.2.0
    for (auto it : this->ConditionVec) {
      Json::Value condition;
      condition["type"] = it.strType;
      condition["triggertime"] = it.strTriggerTime;
      condition["threshold"] = it.strThreshold;
      condition["equationop"] = it.strEquationOp;
      condition["distype"] = it.strDisType;
      condition["boundary"] = it.strBoundary;
      condition["count"] = it.strCount;
      condition["unit"] = it.strSpeedUnit;
      condition["targetelement"] = it.getTargetElement();
      condition["sourceelement"] = it.strSourceElement;
      condition["positiontype"] = it.strPostionType;
      condition["speedtype"] = it.strSpeedType;
      condition["lon"] = it.strLon;
      condition["lat"] = it.strLat;
      condition["alt"] = it.strAlt;
      condition["radius"] = it.strRadius;
      condition["variate"] = it.strVariate;
      condition["roadid"] = it.strRoadId;
      condition["sectionid"] = it.strSectionId;
      condition["laneid"] = it.strLaneId;
      condition["lateraloffset"] = it.strLateralOffset;
      condition["longitudinaloffset"] = it.strLongitudinalOffset;
      condition["tolerance"] = it.strTolerance;
      condition["element_type"] = it.strElementType;
      condition["element_ref"] = it.getElementRef();
      condition["state"] = it.strState;
      onesceneevent["conditions"].append(condition);
    }
  }
  // actions
  Json::Value act(Json::arrayValue);
  int actionIndex = 0;
  for (int i = 0; i < this->ActionVer.size(); i++) {
    if (ver == "1.0.0.0" && this->ActionVer[i].strChecked == "false") continue;
    Json::Value n;
    if (this->ActionVer[i].strId == "-1") {
      n["actionid"] = std::to_string(actionIndex++);
    } else {
      n["actionid"] = this->ActionVer[i].strId;
    }
    n["actiontype"] = this->ActionVer[i].strType;
    n["actionvalue"] = this->ActionVer[i].strValue;
    n["subtype"] = this->ActionVer[i].strSubType;
    n["offset"] = this->ActionVer[i].strLaneOffset;
    if (this->EndConditionVer.size() > i) {
      if (this->EndConditionVer[i].strType == "") this->EndConditionVer[i].strType = "none";
      n["endconditiontype"] = this->EndConditionVer[i].strType;
      if (this->EndConditionVer[i].strValue == "") this->EndConditionVer[i].strValue = "0.0";
      n["endconditionvalue"] = this->EndConditionVer[i].strValue;
    }
    Json::Value mulit;
    if (this->ActionVer[i].strType == "assign") {
      mulit["isValid"] = this->ActionVer[i].sActionMulti.flag;
      mulit["resume_sw"] =
          static_cast<bool>((this->ActionVer[i].sActionMulti.sAssign.s_resume_sw == "true") ? true : false);
      mulit["cancel_sw"] =
          static_cast<bool>((this->ActionVer[i].sActionMulti.sAssign.s_cancel_sw == "true") ? true : false);
      mulit["speed_inc_sw"] =
          static_cast<bool>((this->ActionVer[i].sActionMulti.sAssign.s_speed_inc_sw == "true") ? true : false);
      mulit["speed_dec_sw"] =
          static_cast<bool>((this->ActionVer[i].sActionMulti.sAssign.s_speed_dec_sw == "true") ? true : false);
      if (this->ActionVer[i].sActionMulti.sAssign.s_set_timegap == "null") {
        this->ActionVer[i].sActionMulti.sAssign.s_set_timegap = "";
      }
      mulit["set_timegap"] = this->ActionVer[i].sActionMulti.sAssign.s_set_timegap;
      if (this->ActionVer[i].sActionMulti.sAssign.s_set_speed == "null") {
        this->ActionVer[i].sActionMulti.sAssign.s_set_speed = "";
      }
      mulit["set_speed"] = this->ActionVer[i].sActionMulti.sAssign.s_set_speed;
      n["multi"] = mulit;
    } else if (this->ActionVer[i].strType == "override") {
      mulit["isValid"] = this->ActionVer[i].sActionMulti.flag;
      mulit["throttle"] = this->ActionVer[i].sActionMulti.sOveride.s_throttle;
      mulit["brake"] = this->ActionVer[i].sActionMulti.sOveride.s_brake;
      mulit["steering_wheel"] = this->ActionVer[i].sActionMulti.sOveride.s_steering_wheel;
      mulit["parking_brake"] = this->ActionVer[i].sActionMulti.sOveride.s_parking_brake;
      mulit["clutch"] = this->ActionVer[i].sActionMulti.sOveride.s_clutch;
      mulit["gear"] = this->ActionVer[i].sActionMulti.sOveride.s_gear;
      n["multi"] = mulit;
    } else if (this->ActionVer[i].strType == "lateralDistance") {
      // n["actiontype"] = "LateralDistance";
      mulit["isValid"] = "true";
      mulit["target_element"] = this->ActionVer[i].sActionMulti.sLateralDistance.s_target_element;
      mulit["maxAcc"] = this->ActionVer[i].sActionMulti.sLateralDistance.s_maxAcc;
      mulit["maxDec"] = this->ActionVer[i].sActionMulti.sLateralDistance.s_maxDec;
      mulit["maxSpeed"] = this->ActionVer[i].sActionMulti.sLateralDistance.s_maxSpeed;
      mulit["continuous"] = ((this->ActionVer[i].sActionMulti.sLateralDistance.s_continuous == "false") ? false : true);
      n["multi"] = mulit;
    } else {
      Json::Value mulitvalue;
      mulitvalue["isValid"] = false;
      n["multi"] = mulitvalue;
    }
    act.append(n);
  }
  onesceneevent["action"] = act;
  if (ver == "1.0.0.0") {
    for (auto i = 0; i < onesceneevent["action"].size(); i++) {
      auto& action = onesceneevent["action"][i];
      if (i >= this->ActionVer.size() || !action.isObject()) {
        continue;
      }
      action["checked"] = this->ActionVer[i].strChecked;
      action.removeMember("multi");
      action.removeMember("endconditiontype");
      action.removeMember("endconditionvalue");
    }
    for (auto&& condition : this->EndConditionVer) {
      Json::Value n;
      n["endconditiontype"] = condition.strType;
      n["endconditionvalue"] = condition.strValue;
      onesceneevent["endCondition"].append(n);
    }
  }

  return onesceneevent;
}

bool SceneEvent::SplitEndCondition() {
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
  return true;
}
// 拆分
void SceneEvent::SegmentString() {
  // split endcondition
  SplitEndCondition();
  // split action version 1.1.0.0
  std::string actionstring = "{action:" + strAction + "}";
  Json::Value actionJson = EncodeActionToJson(strAction);
  if (!actionJson.isNull()) {
    for (Json::ValueIterator endItr = actionJson.begin(); endItr != actionJson.end(); ++endItr) {
      Scene_Action temp;
      temp.strId = (*endItr).get("actionid", "-1").asString();
      temp.strType = (*endItr)["type"].asString();
      temp.strValue = (*endItr).get("value", "0").asString();
      temp.strSubType = (*endItr).get("subtype", "").asString();
      temp.strLaneOffset = (*endItr).get("offset", "0").asString();
      if (temp.strType == "assign") {
        temp.sActionMulti.SplitAssignValue((*endItr)["multi"]);
      } else if (temp.strType == "override") {
        temp.sActionMulti.SplitOveridValue((*endItr)["multi"]);
      } else if (temp.strType == "lateralDistance") {
        temp.strValue = (*endItr).get("distance", "6.0").asString();
        temp.sActionMulti.sLateralDistance.s_target_element = (*endItr).get("target_element", "").asString();
        temp.sActionMulti.sLateralDistance.s_maxAcc = (*endItr).get("maxAcc", "-1").asString();
        temp.sActionMulti.sLateralDistance.s_maxDec = (*endItr).get("maxDec", "-1").asString();
        temp.sActionMulti.sLateralDistance.s_maxSpeed = (*endItr).get("maxSpeed", "-1").asString();
        temp.sActionMulti.sLateralDistance.s_continuous = (*endItr).get("continuous", "false").asString();
      } else {
        temp.sActionMulti.flag = false;
      }
      this->ActionVer.push_back(temp);
    }
    SYSTEM_LOGGER_DEBUG("this->ActionVer size %d", this->ActionVer.size());
  }
}

bool SceneEvent::SegmentString1_0() {
  SYSTEM_LOGGER_INFO("SegmentString1_0");

  // split EndCondition
  SplitEndCondition();
  // split action version 1.0.0.0
  ActionVer.clear();
  std::vector<std::string> actionList;
  boost::algorithm::split(actionList, strAction, boost::algorithm::is_any_of(";"));
  std::vector<Scene_EndCondition> tmpCondition;
  for (size_t i = 0; i < actionList.size(); i++) {
    std::string it = actionList.at(i);
    std::vector<std::string> strSingleValue;
    boost::algorithm::split(strSingleValue, it, boost::algorithm::is_any_of(","));
    //
    std::map<std::string, std::string> mapKV;
    Scene_Action temp;
    for (auto it2 : strSingleValue) {
      std::vector<std::string> strK_V;
      boost::algorithm::split(strK_V, it2, boost::algorithm::is_any_of(":"));
      if (strK_V.size() > 1) mapKV.insert(std::make_pair(strK_V[0], strK_V[1]));
    }
    temp.strChecked = "false";
    auto itr = mapKV.find("checked");
    if (itr != mapKV.end()) temp.strChecked = itr->second;
    tmpCondition.push_back(EndConditionVer.at(i));
    itr = mapKV.find("type");
    if (itr != mapKV.end()) temp.strType = itr->second;
    itr = mapKV.find("value");
    if (itr != mapKV.end()) temp.strValue = itr->second;
    itr = mapKV.find("subtype");
    if (itr != mapKV.end()) temp.strSubType = itr->second;
    itr = mapKV.find("laneoffset");
    if (itr != mapKV.end()) temp.strLaneOffset = itr->second;
    ActionVer.push_back(temp);
  }
  EndConditionVer.clear();
  EndConditionVer = tmpCondition;
  return true;
}

void SceneEvent::MergeString() {  // 合并给后端用
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
  // merge json
  strAction = "[";

  for (int i = 0; i < ActionVer.size(); i++) {
    strAction.append("{actionid:");
    strAction.append(ActionVer[i].strId);
    strAction.append(",type:");
    strAction.append(ActionVer[i].strType);
    if (ActionVer[i].strType == "lateralDistance") {
      strAction.append(",target_element:");
      strAction.append(ActionVer[i].sActionMulti.sLateralDistance.s_target_element);
      strAction.append(",distance:");
      strAction.append(ActionVer[i].strValue);
      strAction.append(",freespace:false");
      strAction.append(",continuous:");
      strAction.append(ActionVer[i].sActionMulti.sLateralDistance.s_continuous);
      strAction.append(",maxAcc:");
      strAction.append(ActionVer[i].sActionMulti.sLateralDistance.s_maxAcc);
      strAction.append(",maxDec:");
      strAction.append(ActionVer[i].sActionMulti.sLateralDistance.s_maxDec);
      strAction.append(",maxSpeed:");
      strAction.append(ActionVer[i].sActionMulti.sLateralDistance.s_maxSpeed);
    } else {
      strAction.append(",value:");
      strAction.append(ActionVer[i].strValue);
      strAction.append(",subtype:");
      strAction.append(ActionVer[i].strSubType);
      strAction.append(",offset:");
      strAction.append(ActionVer[i].strLaneOffset);
      strAction.append(",multi:");
      if (ActionVer[i].strType == "assign") {
        strAction.append(ActionVer[i].sActionMulti.MergeAssignValue());
      } else if (ActionVer[i].strType == "override") {
        strAction.append(ActionVer[i].sActionMulti.MergeOveridValue());
      } else {
        strAction.append("{}");
      }
    }
    if (i < ActionVer.size() - 1) {
      strAction.append("},");
    } else {
      strAction.append("}");
    }
  }
  strAction.append("]");
}

void SceneEvent::MergeString1_0() {
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

Json::Value SceneEvent::MakeJson(std::string str) {
  Json::Value ret;
  try {
    std::regex ex(R"([\w\-.]+)");
    std::string jsonStr = regex_replace(str, ex, "\"$&\"");
    SYSTEM_LOGGER_INFO("json %s", jsonStr.c_str());
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING errs;
    Json::CharReader* reader = builder.newCharReader();
    Json::Value root;
    int nLen = strlen(jsonStr.c_str());
    if (!reader->parse(jsonStr.c_str(), jsonStr.c_str() + nLen, &root, &errs)) {
      SYSTEM_LOGGER_ERROR("parse scenario create param json failed: ", jsonStr.c_str());
      return Json::Value();
    }
    return root;
  } catch (const std::regex_error& e) {
    SYSTEM_LOGGER_INFO("regex_error caught: %s", e.what());
    if (e.code() == std::regex_constants::error_brack) {
      SYSTEM_LOGGER_INFO("The code was error_brack");
    }
    return Json::Value();
  }
  return ret;
}

Json::Value SceneEvent::EncodeActionToJson(const std::string& s) {
  Json::Value actions(Json::arrayValue);
  std::string s_without_user_define_action = s;

  boost::regex re(SceneEvent::kUserDefinePattern);
  for (boost::sregex_iterator it(s.begin(), s.end(), re), end; it != end; ++it) {
    boost::smatch ms = *it;
    if (ms.size() != SceneEvent::kUserDefinePatternSubMatchSize + 1) {
      continue;
    }
    Json::Value one;
    for (auto i = 1; i < ms.size(); i += 2) {
      one[ms[i].str()] = ms[i + 1].str();
    }
    actions.append(one);
    s_without_user_define_action.replace(s_without_user_define_action.find(ms[0].str()), ms[0].str().size(), "{}");
  }

  s_without_user_define_action =
      boost::regex_replace(s_without_user_define_action, boost::regex(R"([\w\-.]+)"), R"("$&")");

  Json::Value other_actions = StringToJson(s_without_user_define_action);
  for (auto&& e : other_actions) {
    if (!e.empty()) {
      actions.append(e);
    }
  }

  return actions;
}

std::string SceneEvent::bool2String(bool value) {
  if (value == true) {
    return "true";
  } else {
    return "false";
  }
}

std::string Scene_Action_multi::MergeAssignValue() {
  if (this->flag) {
    std::string assignValue;
    assignValue += ("{resume_sw:" + this->sAssign.s_resume_sw + ",");
    assignValue += ("cancel_sw:" + this->sAssign.s_cancel_sw + ",");
    assignValue += ("speed_inc_sw:" + this->sAssign.s_speed_inc_sw + ",");
    assignValue += ("speed_dec_sw:" + this->sAssign.s_speed_dec_sw + ",");
    if (this->sAssign.s_set_timegap == "") {
      this->sAssign.s_set_timegap = "null";
    }
    assignValue += ("set_timegap:" + this->sAssign.s_set_timegap + ",");
    if (this->sAssign.s_set_speed == "") {
      this->sAssign.s_set_speed = "null";
    }
    assignValue += ("set_speed:" + this->sAssign.s_set_speed + "}");
    return assignValue;
  }
  return "{}";
}

std::string Scene_Action_multi::MergeOveridValue() {
  std::string assignValue;
  std::string tmp = this->sOveride.s_throttle;
  std::vector<std::string> strTmpVec;
  boost::algorithm::split(strTmpVec, tmp, boost::algorithm::is_any_of(";"));
  if (strTmpVec.size() > 1) {
    assignValue += ("{throttle:{");
    assignValue += ("activate:" + strTmpVec.at(0) + ",");
    assignValue += ("value:" + strTmpVec.at(1));
    assignValue += ("},");
  }
  tmp = this->sOveride.s_brake;
  strTmpVec.clear();
  boost::algorithm::split(strTmpVec, tmp, boost::algorithm::is_any_of(";"));
  if (strTmpVec.size() > 1) {
    assignValue += ("brake:{");
    assignValue += ("activate:" + strTmpVec.at(0) + ",");
    assignValue += ("value:" + strTmpVec.at(1));
    assignValue += ("},");
  }

  tmp = this->sOveride.s_steering_wheel;
  strTmpVec.clear();
  boost::algorithm::split(strTmpVec, tmp, boost::algorithm::is_any_of(";"));
  if (strTmpVec.size() > 1) {
    assignValue += ("steering_wheel:{");
    assignValue += ("activate:" + strTmpVec.at(0) + ",");
    assignValue += ("value:" + strTmpVec.at(1));
    assignValue += ("},");
  }

  tmp = this->sOveride.s_parking_brake;
  strTmpVec.clear();
  boost::algorithm::split(strTmpVec, tmp, boost::algorithm::is_any_of(";"));
  if (strTmpVec.size() > 1) {
    assignValue += ("parking_brake:{");
    assignValue += ("activate:" + strTmpVec.at(0) + ",");
    assignValue += ("value:" + strTmpVec.at(1));
    assignValue += ("},");
  }

  tmp = this->sOveride.s_clutch;
  strTmpVec.clear();
  boost::algorithm::split(strTmpVec, tmp, boost::algorithm::is_any_of(";"));
  if (strTmpVec.size() > 1) {
    assignValue += ("clutch:{");
    assignValue += ("activate:" + strTmpVec.at(0) + ",");
    assignValue += ("value:" + strTmpVec.at(1));
    assignValue += ("},");
  }

  tmp = this->sOveride.s_gear;
  strTmpVec.clear();
  boost::algorithm::split(strTmpVec, tmp, boost::algorithm::is_any_of(";"));
  if (strTmpVec.size() > 1) {
    assignValue += ("gear:{");
    assignValue += ("activate:" + strTmpVec.at(0) + ",");
    assignValue += ("value:" + strTmpVec.at(1));
    assignValue += ("}}");
  }

  return assignValue;
}

bool Scene_Action_multi::SplitAssignValue(Json::Value multivalue) {
  Scene_Action_Assign temp;
  if (multivalue["resume_sw"].isNull()) {
    this->flag = false;
    return true;
  }
  temp.s_resume_sw = multivalue["resume_sw"].asString();
  temp.s_cancel_sw = multivalue["cancel_sw"].asString();
  temp.s_speed_inc_sw = multivalue["speed_inc_sw"].asString();
  temp.s_speed_dec_sw = multivalue["speed_dec_sw"].asString();
  temp.s_set_timegap = multivalue["set_timegap"].asString();
  temp.s_set_speed = multivalue["set_speed"].asString();
  this->sAssign = temp;
  this->flag = true;
  return true;
}

bool Scene_Action_multi::SplitOveridValue(Json::Value multivalue) {
  Scene_Action_Override temp;
  std::string activate = multivalue["throttle"]["activate"].asString();
  std::string value = multivalue["throttle"]["value"].asString();
  temp.s_throttle = activate + ";" + value;

  activate = multivalue["brake"]["activate"].asString();
  value = multivalue["brake"]["value"].asString();
  temp.s_brake = activate + ";" + value;

  activate = multivalue["steering_wheel"]["activate"].asString();
  value = multivalue["steering_wheel"]["value"].asString();
  temp.s_steering_wheel = activate + ";" + value;

  activate = multivalue["parking_brake"]["activate"].asString();
  value = multivalue["parking_brake"]["value"].asString();
  temp.s_parking_brake = activate + ";" + value;

  activate = multivalue["clutch"]["activate"].asString();
  value = multivalue["clutch"]["value"].asString();
  temp.s_clutch = activate + ";" + value;

  activate = multivalue["gear"]["activate"].asString();
  value = multivalue["gear"]["value"].asString();
  temp.s_gear = activate + ";" + value;

  this->sOveride = temp;
  this->flag = true;
  return true;
}

std::string Scene_Action_Override::activate(std::string str) {
  std::string tmp;
  if (str == "throttle") {
    tmp = s_throttle;
  } else if (str == "brake") {
    tmp = s_brake;
  } else if (str == "steering_wheel") {
    tmp = s_steering_wheel;
  } else if (str == "parking_brake") {
    tmp = s_parking_brake;
  } else if (str == "clutch") {
    tmp = s_clutch;
  } else if (str == "gear") {
    tmp = s_gear;
  }
  std::vector<std::string> strTmpVec;
  boost::algorithm::split(strTmpVec, tmp, boost::algorithm::is_any_of(";"));
  if (strTmpVec.size() > 1) {
    return strTmpVec.at(0);
  } else {
    return "";
  }
}

std::string Scene_Action_Override::value(std::string str) {
  std::string tmp;
  if (str == "throttle") {
    tmp = s_throttle;
  } else if (str == "brake") {
    tmp = s_brake;
  } else if (str == "steering_wheel") {
    tmp = s_steering_wheel;
  } else if (str == "parking_brake") {
    tmp = s_parking_brake;
  } else if (str == "clutch") {
    tmp = s_clutch;
  } else if (str == "gear") {
    tmp = s_gear;
  }
  std::vector<std::string> strTmpVec;
  boost::algorithm::split(strTmpVec, tmp, boost::algorithm::is_any_of(";"));
  if (strTmpVec.size() > 1) {
    return strTmpVec.at(1);
  } else {
    return "";
  }
}

// SceneCondition
bool SceneCondition::loadCondition(const std::string& condition) {
  std::vector<std::string> strCondition_K_V;
  boost::algorithm::split(strCondition_K_V, condition, boost::algorithm::is_any_of(","));
  std::map<std::string, std::string> map_keyvalue;
  for (int i = 0; i < strCondition_K_V.size(); i++) {
    std::vector<std::string> strK_V;
    boost::algorithm::split(strK_V, strCondition_K_V[i], boost::algorithm::is_any_of(":"));
    if (strK_V.size() > 1) {
      map_keyvalue.insert(std::make_pair(strK_V[0], strK_V[1]));
    } else {
      SYSTEM_LOGGER_WARN("Condition value is error: %s", condition);
      return false;
    }
  }
  if (strType == "time_trigger") {
    auto itr = map_keyvalue.find("trigger_time");
    if (itr != map_keyvalue.end()) strTriggerTime = itr->second;
    itr = map_keyvalue.find("equation_op");
    if (itr != map_keyvalue.end()) strEquationOp = itr->second;
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
  } else if (strType == "timeheadway_trigger") {
    auto itr = map_keyvalue.find("target_element");
    if (itr != map_keyvalue.end()) strTargetElement = itr->second;

    itr = map_keyvalue.find("value");
    if (itr != map_keyvalue.end()) strThreshold = itr->second;

    itr = map_keyvalue.find("equation_op");
    if (itr != map_keyvalue.end()) strEquationOp = itr->second;

    itr = map_keyvalue.find("condition_boundary");
    if (itr != map_keyvalue.end()) strBoundary = itr->second;

    itr = map_keyvalue.find("trigger_count");
    if (itr != map_keyvalue.end()) strCount = itr->second;
  } else if (strType == "element_state") {
    auto itr = map_keyvalue.find("element_type");
    if (itr != map_keyvalue.end()) strElementType = itr->second;
    itr = map_keyvalue.find("element_ref");
    if (itr != map_keyvalue.end()) strElementRef = itr->second;
    itr = map_keyvalue.find("state");
    if (itr != map_keyvalue.end()) strState = itr->second;
  } else {
    SYSTEM_LOGGER_WARN("Condition type is error: %s", strType);
    return false;
  }
  return true;
}

std::string SceneCondition::mergeCondition() {
  std::string strCondition;
  if (strType == "time_trigger") {
    strCondition = "trigger_time:";
    strCondition.append(strTriggerTime);
    strCondition.append(",equation_op:");
    strCondition.append(strEquationOp);
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
    strCondition.append(",trigger_count:");
    strCondition.append(strCount);
  } else if (strType == "timeheadway_trigger") {
    strCondition = "target_element:";
    strCondition.append(strTargetElement);
    strCondition.append(",value:");
    strCondition.append(strThreshold);
    strCondition.append(",freespace:");
    strCondition.append("false");
    strCondition.append(",alongroute:");
    strCondition.append("false");
    strCondition.append(",equation_op:");
    strCondition.append(strEquationOp);
    strCondition.append(",condition_boundary:");
    strCondition.append(strBoundary);
    strCondition.append(",trigger_count:");
    strCondition.append(strCount);
  } else if (strType == "element_state") {
    strCondition = "element_type:";
    strCondition.append(strElementType);
    strCondition.append(",element_ref:");
    strCondition.append(strElementRef);
    strCondition.append(",state:");
    strCondition.append(strState);
  } else {
    SYSTEM_LOGGER_WARN("condition type is error : %s", strType);
  }
  return strCondition;
}

std::string SceneCondition::getTargetElement() {
  // multiple main vehicle versions compatible with older versions
  if (0 == strTargetElement.compare("ego")) {
    return "ego_1";
  }
  return strTargetElement;
}

std::string SceneCondition::getElementRef() {
  // multiple main vehicle versions compatible with older versions
  if (0 == strElementRef.compare("ego")) {
    return "ego_1";
  }
  return strElementRef;
}
