/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include "common/xml_parser/entity/catalogbase.h"
#include "common/xml_parser/entity/route.h"
#include "common/xml_parser/entity/scene_event.h"
class CPlannerParam;
struct sEgoVelocity {
  float m_fStartVelocityStart;
  float m_fStartVelocityEnd;
  float m_fStartVelocitySep;
};

struct sControlPathPoint {
  double m_dLon;
  double m_dLat;
  double m_dAlt;
  double m_dVelocity;
  int m_nGear;

  std::string m_strLon;
  std::string m_strLat;
  std::string m_strAlt;
  std::string m_strVelocity;
  std::string m_strGear;
  std::string m_accs;
  std::string m_heading;
  std::string m_frontwheel;

  sControlPathPoint();

  void Reset();
  void ConvertToStr();
  void ConvertToValue();
};

struct sControlPath {
  double m_dSampleInterval;
  std::string m_strSampleInterval;
  std::vector<sControlPathPoint> m_vPoints;
  std::string m_strPoints;
  sControlPath();
  void Reset();
  void pushPoint(sControlPathPoint& pp);
  void ConvertToStr();
  void ConvertToValue();
  // set controlPoint
  void setControlPathPoint(std::string strPoints);
  // set control Point lon/lat/alt
  void setControlPointLonLat(const sPath& inputpath);
  // get control path
  std::string getControlPath();
  // get TrajectoryTracking
  std::string getTrajectoryTracking();
};

using SceneEventMap = std::map<std::string, SceneEvent>;
class CEgoInfo {
 public:
  CEgoInfo();

  bool IsEmpty();
  void Reset();
  void ConvertToValue();
  void ConvertToStr();
  void AdjustTypeAndName(const std::string& catalog_file = "", const char* raw_type = nullptr,
                         const char* raw_name = nullptr);

 public:
  uint64_t m_ID;
  float m_fStartVelocity;
  float m_fStartAngle;
  float m_fMaxVelocity;
  double m_dStartAlt;
  double m_dEndAlt;
  int m_nType;
  std::string m_strGroupName;  // 唯一标识
  std::string m_strStartVelocity;
  std::string m_strTheta ;
  std::string m_strVelocityMax;
  std::string m_strStartAlt;
  std::string m_strEndAlt;
  CRoute m_route;
  std::string m_strType;
  // std::string m_ScenarioName;
  std::string m_strName;
  std::string m_sensorId;
  sPath m_inputPath;
  sControlPath m_controlPath;
  int m_nUnrealLevel;
  std::string m_strUnrealLevel;
  // add
  std::string m_controlType;
  std::string m_controlLongitudinal;
  std::string m_controlLateral;
  std::string m_accelerationMax;
  std::string m_decelerationMax;
  std::string m_trajectoryEnabled;
  CBoundingBox m_boundingBox;
  // scene_event
  SceneEventMap m_scenceEvents;
  // scene param
  std::vector<std::string> m_paramSceneStartV;
};

class CPlanner {
 public:
  CPlanner();
  void Reset();
  void ConvertFirstEgoParam();
  bool isContainsTrajectoryEnable();
  std::map<std::string, CEgoInfo> m_egos;

 public:
  uint64_t m_ID;
  float m_fStartVelocity;
  float m_fStartAngle;
  float m_fMaxVelocity;
  double m_dStartAlt;
  double m_dEndAlt;
  int m_nType;
  std::string m_strStartVelocity;
  std::string m_strTheta;
  std::string m_strVelocityMax;
  std::string m_strStartAlt;
  std::string m_strEndAlt;
  CRoute m_route;
  std::string m_strType;
  std::string m_ScenarioName;
  std::string m_strName;
  std::string m_sensorId;
  sPath m_inputPath;
  sControlPath m_controlPath;
  int m_nUnrealLevel;
  std::string m_strUnrealLevel;
  // add
  std::string m_controlType;
  std::string m_controlLongitudinal;
  std::string m_controlLateral;
  std::string m_accelerationMax;
  std::string m_decelerationMax;
  std::string m_trajectoryEnabled;
  // scene_event
  SceneEventMap m_scenceEvents;
};

class CPlannerParam : public CPlanner {
 public:
  CPlannerParam& operator=(const CPlanner& p);

  int ComputeSingleNumber(std::string key) {
    size_t nNumber = 0;
    if (m_mapEgoGroup2Velocity.find(key) != m_mapEgoGroup2Velocity.end()) {
      auto it = m_mapEgoGroup2Velocity.find(key)->second;
      nNumber = std::ceil((it.m_fStartVelocityEnd - it.m_fStartVelocityStart) / it.m_fStartVelocitySep);
    }
    return nNumber;
  }

  int GenerateSingleEgoParam();

  int GenerateCount();

  void GenerateSets(std::vector<std::vector<std::string>>& arrays, std::vector<std::string>& currentSet, int index,
                    std::vector<std::vector<std::string>>& result) {
    if (index == arrays.size()) {
      result.push_back(currentSet);
      return;
    }

    for (int i = 0; i < arrays[index].size(); ++i) {
      currentSet.push_back(arrays[index][i]);
      GenerateSets(arrays, currentSet, index + 1, result);
      currentSet.pop_back();
    }
  }

 public:
  std::map<std::string, sEgoVelocity> m_mapEgoGroup2Velocity;
  std::vector<std::vector<std::string>> m_paramsVec;
};
