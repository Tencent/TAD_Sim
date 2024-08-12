/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <json/value.h>
#include <map>
#include <string>
#include <vector>
#ifndef SCENE_EVENT_VERSION
#  define SCENE_EVENT_VERSION ("1.2.0.0")
#endif

enum Tragger_type {
  Time_Trigger_Type,
  Velocity_Trigger_Type,
  Reach_Abs_Position_Type,
  Reach_Abs_Lane_Type,
  TTC_Trigger_Type,
  Distance_Trigger_Type
};

struct Scene_EndCondition {
  std::string strType = "none";
  std::string strValue = "0.0";
};

struct Scene_Action_Assign {
  std::string s_resume_sw = "false";
  std::string s_cancel_sw = "false";
  std::string s_speed_inc_sw = "false";
  std::string s_speed_dec_sw = "false";
  std::string s_set_timegap = "0.0";
  std::string s_set_speed = "0.0";
};

struct Scene_Action_Override {
  std::string s_throttle;
  std::string s_brake;
  std::string s_steering_wheel;
  std::string s_parking_brake;
  std::string s_clutch;
  std::string s_gear;

  std::string activate(std::string str);
  std::string value(std::string str);
};

struct Scene_Action_LateralDistance {
  std::string s_target_element = "";
  std::string s_maxAcc = "-1";
  std::string s_maxDec = "-1";
  std::string s_maxSpeed = "-1";
  std::string s_continuous = "false";
};

enum Scene_Action_Command {
  COM_LANECHANGE = 1,
  COM_OVERTAKING = 2,
  COM_EMERGENCY_STOP = 3,
  COM_SPEED_TO_LEFT = 4,
  COM_SPEED_TO_RIGHT = 5,
  COM_ACCEL_TO_LEFT = 6,
  COM_ACCEL_TO_RIGHT = 7,
  COM_UNDERDEFINE = 99
};

struct Scene_Action_multi {
  bool flag;
  Scene_Action_Assign sAssign;
  Scene_Action_Override sOveride;
  Scene_Action_LateralDistance sLateralDistance;

 public:
  std::string MergeAssignValue();
  std::string MergeOveridValue();
  bool SplitAssignValue(Json::Value multivalue);
  bool SplitOveridValue(Json::Value multivalue);
  std::string multiMerge;
  Scene_Action_multi() {
    flag = "false";
    // Assign
    sAssign.s_resume_sw = "false";
    sAssign.s_cancel_sw = "false";
    sAssign.s_speed_inc_sw = "false";
    sAssign.s_speed_dec_sw = "false";
    sAssign.s_set_timegap = "2.0";
    sAssign.s_set_speed = "60.0";
    // override
    sOveride.s_throttle = "false;0";
    sOveride.s_brake = "false;0";
    sOveride.s_steering_wheel = "false;0";
    sOveride.s_parking_brake = "false;0";
    sOveride.s_clutch = "false;0";
    sOveride.s_gear = "false;0";
  }
};

struct Scene_Action {
  std::string strId = "-1";
  std::string strType = "none";
  std::string strValue = "0.0";
  std::string strSubType = "none";
  std::string strLaneOffset = "0.0";
  std::string strChecked = "false";  // for v1.0
  std::string strName = "";
  Scene_Action_multi sActionMulti;
};

struct SceneCondition {
 public:
  bool loadCondition(const std::string& condition);

  std::string mergeCondition();

  std::string getTargetElement();

  std::string getElementRef();

  std::string strType;
  /// conditions
  /// condition time_trigger
  std::string strTriggerTime;
  /// condition velocity_trigger
  std::string strSpeedType;
  std::string strThreshold;
  std::string strEquationOp;
  std::string strSourceElement;
  std::string strTargetElement;
  std::string strDisType;
  std::string strBoundary;
  std::string strCount = "1";
  std::string strSpeedUnit = "m_s";
  /// condition reach_abs_position
  std::string strPostionType;
  std::string strLon;
  std::string strLat;
  std::string strAlt;
  std::string strRadius;
  /// condition reach_abs_lane
  std::string strRoadId;
  std::string strLaneId;
  std::string strLateralOffset;
  std::string strLongitudinalOffset;
  std::string strTolerance;
  /// condition ego_attach_laneid_custom
  std::string strSectionId;
  std::string strVariate;
  /// storybroad
  std::string strElementType;
  std::string strElementRef;
  std::string strState;
};

class SceneEvent {
 public:
  SceneEvent();

  ~SceneEvent();
  // from json to xml value
  bool fromJson2Xml(const Json::Value& json, const std::string& ver = SCENE_EVENT_VERSION);
  //
  Json::Value saveJson(const std::string& ver = SCENE_EVENT_VERSION);

  // bool loadprofile(std::string strprofile, int type);
  // std::string strCondition;
  bool isUserDefined;  //
  std::string m_id;
  std::string strName;
  std::string strEndCondition;
  std::string strAction;
  std::string strInfo;
  bool SplitEndCondition();
  void SegmentString();  // 拆分给前端用
  bool SegmentString1_0();
  void MergeString();  // 合并给后端用
  void MergeString1_0();
  // end conditon
  std::vector<Scene_EndCondition> EndConditionVer;
  // action
  std::vector<Scene_Action> ActionVer;
  // condition
  std::vector<SceneCondition> ConditionVec;

  static Json::Value EncodeActionToJson(const std::string& str);

 private:
  Json::Value MakeJson(std::string str);
  std::string bool2String(bool value);

  static constexpr int kUserDefinePatternSubMatchSize = 12;
  static constexpr const char* kUserDefinePattern =
      R"(\{(actionid):(\d+),(type):(status|command),(value):(.*?),(subtype):(userdefine),(offset):(0.0),(multi):(\{\})\})";
};
