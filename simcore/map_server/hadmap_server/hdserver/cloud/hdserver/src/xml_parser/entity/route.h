/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <string>
#include "common/map_data/map_data_predef.h"

struct sPathPoint {
  double m_dLon;
  double m_dLat;
  double m_dAlt;

  std::string m_strLon;
  std::string m_strLat;
  std::string m_strAlt;
  sPathPoint();
  void Reset();
  void ConvertToStr();
  void ConvertToValue();
};

struct sPath {
  std::vector<sPathPoint> m_vPoints;
  std::string m_strPoints;
  sPath();
  void Reset();
  void pushPoint(double dLon, double dLat, double dAlt);
  void pushPoint(sPathPoint& pp);
  void ConvertToStr();
  void ConvertToValue();
};

struct sRouteControlPath {
  std::string strLon;
  std::string strLat;
  std::string strAlt;
  std::string strSpeed_m_s;
  std::string strGear;
};

class CRoute {
 public:
  enum {
    ROUTE_TYPE_INVALID = -1,
    ROUTE_TYPE_START_END = 0,
    ROUTE_TYPE_ROAD_ID,
  };

  CRoute();
  CRoute& operator=(const CRoute& other);
  bool operator<(const CRoute& other);

  void ConvertToValue();

  void RemoveStrAlt();

  void FromStr();

  void ExtractPath();

  void Reset();

  void Set(uint64_t id);

  void Set(double dStartLon, double dStartLat);

  void Set(double dStartLon, double dStartLat, double dEndLon, double dEndLat);

  void Set(double dStartLon, double dStartLat, double dMidLon, double dMidLat, double dEndLon, double dEndLat);

  void Set(roadpkid roadID, sectionpkid sectionID);

  void Set(std::string strInfo);

  int Type() { return m_nType; }

  void SegmentString();  // 拆分m_strControlPath  给前端用
  void MergeString();    // 合并m_strControlPath   给后端用
  void MergeXOSCString();  // 通过m_strTrajectoryTracking_XOSC  和  m_path  计算得到m_strControlPath和m_ControlPathVer

 public:
  std::string m_strID;
  std::string m_strType;
  std::string m_strStart;
  std::string m_strStartLon;
  std::string m_strStartLat;
  std::string m_strStartAlt;
  std::string m_strMids;
  std::string m_strMid;
  std::string m_strMidLon;
  std::string m_strMidLat;
  std::string m_strMidAlt;
  std::string m_strEnd;
  std::string m_strEndLon;
  std::string m_strEndLat;
  std::string m_strEndAlt;
  std::string m_strRoadID;
  std::string m_strSectionID;
  std::string m_strInfo;

  std::string m_strControlPath;                     // 交通车 跟踪数据
  std::vector<sRouteControlPath> m_ControlPathVer;  // 交通车 跟踪数据
  std::string m_strTrajectoryTracking_XOSC;  // xosc格式场景中  没有与之对应的speed_m_s 和 gear 那么就把它们单独拎出来
                                             // 放在属性中

  sPath m_path;

  uint64_t m_ID;
  int m_nType;

  double m_dStartLon;
  double m_dStartLat;
  double m_dStartAlt;
  // 通过m_dMidLon和m_dMidLat的值是否大于0来确定文件中是否设置了mid点
  double m_dMidLon;
  double m_dMidLat;
  double m_dMidAlt;
  // 通过m_dEndLon和m_dEndLat的值是否大于0来确定文件中是否设置了end点
  double m_dEndLon;
  double m_dEndLat;
  double m_dEndAlt;

  roadpkid m_roadID;
  sectionpkid m_sectionID;
};
