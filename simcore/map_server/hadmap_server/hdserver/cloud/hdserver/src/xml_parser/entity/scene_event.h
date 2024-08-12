/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <map>
#include <string>
#include <vector>
enum Tragger_type {
  Time_Trigger_Type,
  Velocity_Trigger_Type,
  Reach_Abs_Position_Type,
  Reach_Abs_Lane_Type,
  TTC_Trigger_Type,
  Distance_Trigger_Type
};

struct Scene_EndCondition {
  std::string strType;
  std::string strValue;
  Scene_EndCondition() {
    strType = "none";
    strValue = "0.0";
  }
};

struct Scene_Action {
  std::string strType;
  std::string strValue;
  std::string strSubType;
  std::string strLaneOffset;
  std::string strChecked;
  Scene_Action() {
    strType = "none";
    strValue = "0.0";
    strSubType = "none";
    strLaneOffset = "0.0";
    strChecked = "false";
  }
};

class SceneEvent {
 public:
  SceneEvent();
  ~SceneEvent();
  bool isUserDefined;  // 我们目前自定义的只有 acc  merge  或者 velocity   但是标准里还是有其它的行为的 比如轨迹跟踪
                       // 先用这个变量去约束    除了这三个为true   其它都为false  不通过"事件"传给前端
  std::string m_id;
  std::string strName;
  std::string strType;

  std::string strCondition;
  std::string strEndCondition;
  std::string strAction;
  std::string strInfo;

  void SegmentString();  // 拆分给前端用
  void MergeString();    // 合并给后端用

  /////
  // condition time_trigger
  std::string strTriggerTime;

  // condition velocity_trigger
  std::string strSpeedType;
  std::string strThreshold;
  std::string strEquationOp;
  std::string strSourceElement;
  std::string strTargetElement;
  std::string strDisType;
  std::string strBoundary;
  std::string strCount;
  std::string strSpeedUnit;
  // condition

  // condition
  std::string strPostionType;
  std::string strLon;
  std::string strLat;
  std::string strAlt;
  std::string strRadius;
  // condition
  std::string strRoadId;
  std::string strLaneId;
  std::string strLateralOffset;
  std::string strLongitudinalOffset;
  std::string strTolerance;
  // condition
  std::string strSectionId;
  std::string strVariate;

  // end condition   目前是三个元素
  // std::string strEndConditionType;
  // std::string strEndConditionValue;
  // std::vector<std::string> strEndConditionType;
  // std::vector<std::string> strEndConditionValue;
  std::vector<Scene_EndCondition> EndConditionVer;

  // Action  目前是三个元素
  // std::string strActionType;
  // std::string strActionValue;
  // std::vector<std::string> strActionType;
  // std::vector<std::string> strActionValue;
  // std::vector<std::string> strActionSubType;
  std::vector<Scene_Action> ActionVer;
};
