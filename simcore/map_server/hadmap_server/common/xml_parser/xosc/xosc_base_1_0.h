/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <set>
#include <string>

#ifdef PROJ_NEW_API
#  include <proj.h>
#else
#  ifndef ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#    define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#  endif
#  include "proj_api.h"
#endif

#include "common/map_data/map_query.h"
#include "common/utils/misc.h"

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLString.hpp>

#define XOSC_TADSIM "TAD Sim"
#define XOSC_TADSIM_2_0 "TAD Sim V2.0"
#define XOSC_TADSIM_2_1 "TAD Sim V2.1"
#define XOSC_TADSIM_2_2 "TAD Sim V2.2"

class CTraffic;
class CSimulation;
class CRoute;
#ifndef PI
#  define PI 3.14159265358979323846
#endif
enum XOSCErrorCode {
  OtherError = -1,
  Error_MapFormat = -2,
  Error_MapLost = -3,
};

enum Object_Type {
  Object_VehicleType,
  Object_ObstacleType,
  Object_PedestrianType,
};

enum XOSC_VERSION {
  XOSC_1_0_VERSION_TAD_SIM,
  XOSC_1_0_VERSION_TAD_SIM_2_0,
  XOSC_1_0_VERSION_TAD_SIM_2_1,
  XOSC_1_0_VERSION_OTHER,
};

enum XOSC_MAP_TYPE {
  MAP_XODR = 1,
  MAP_SQLITE = 2,
};

enum RoutingActionType { AssignRouteAction_Type, FollowTrajectoryAction_Type, AcquirePositionAction_Type };
// catalog路径
struct CatalogPath {
  std::string strVechileCatalog;
  std::string strControllerCatalog;
  std::string strPedestrianCatalog;
  std::string strMisObjectCatalog;
  std::string strEnvironmentCatalog;
  std::string strManeuverCatalog;
  std::string strTrajectoryCatalog;
  std::string strRouteCatalog;
};

// 时间触发
struct TimeTraggerData {
  std::string strTraggerTime;
  std::string strVelocity;
  std::string strAcc;
  std::string strEndConditionType;
  std::string strEndConditionValue;
  std::string strMerge;
  std::string strOffset;
  std::string strOffsetOverTime;

  TimeTraggerData() {
    strTraggerTime = "null";
    strVelocity = "null";
    strAcc = "null";
    strEndConditionType = "null";
    strEndConditionValue = "null";
    strMerge = "null";
    strOffset = "null";
    strOffsetOverTime = "null";
  }
};

// 条件触发
struct ConditionTraggerData {
  std::string strCondition_3to1;

  std::string strTraggerType;
  std::string strDisModle;
  std::string strTraggerValue;
  std::string strCount;
  std::string strVelocity;
  std::string strAcc;
  std::string strEndConditionType;
  std::string strEndConditionValue;
  std::string strMerge;
  std::string strOffset;
  std::string strOffsetOverTime;

  ConditionTraggerData() {
    strCondition_3to1 = "null";
    strTraggerType = "null";
    strDisModle = "null";
    strTraggerValue = "null";
    strCount = "null";
    strVelocity = "null";
    strAcc = "null";
    strEndConditionType = "null";
    strEndConditionValue = "null";
    strMerge = "null";
    strOffset = "null";
    strOffsetOverTime = "null";
  }
};

struct doublestring {
  std::string _str1;
  std::string _str2;
  doublestring(std::string str1, std::string str2) : _str1(str1), _str2(str2) {}
};

struct Type3_Value {
  std::string _parameterType;
  std::string _vaule;
};

struct sTagQueryInfoResult {
  std::string strType;
  int64_t roadID;
  int64_t sectionID;
  int64_t laneID;
  int64_t lanelinkID;
  double dLon;
  double dLat;
  double dAlt;
  double dShift;
  double dOffset;
  double dsrcLon;
  double dsrcLat;
  double yaw = 0.0;
};

using xercesc::Attributes;
using xercesc::XMLString;

class ParameterDeclarationHandler : public xercesc::DefaultHandler {
 private:
  class XStr {
   public:
    explicit XStr(const char* const str) { data = XMLString::transcode(str); }
    ~XStr() { XMLString::release(&data); }
    const XMLCh* Data() const { return data; }

   private:
    XMLCh* data;
  };

 public:
  explicit ParameterDeclarationHandler(std::map<std::string, std::string>* parameter_map)
      : parameter_map_(parameter_map) {}

  void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname,
                    const Attributes& attrs) override {
    if (!parameter_map_ || !XMLString::equals(localname, tag_name_)) {
      return;
    }
    current_element_ = localname;
    std::string name{"$"};
    std::string value;
    for (int32_t i = 0; i < attrs.getLength(); i++) {
      if (XMLString::equals(attrs.getLocalName(i), XStr("name").Data())) {
        name.append(XMLString::transcode(attrs.getValue(i)));
      } else if (XMLString::equals(attrs.getLocalName(i), XStr("value").Data())) {
        value = XMLString::transcode(attrs.getValue(i));
      }
    }
    (*parameter_map_)[name] = value;
  }

  void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) override {
    current_element_ = nullptr;
    last_element_ = localname;
  }

  void characters(const XMLCh* const chars, const XMLSize_t length) override {
    if (!current_element_ || !XMLString::equals(current_element_, XStr("ParameterDeclaration").Data())) {
      return;
    }
  }

 private:
  const XMLCh* tag_name_ = XMLString::transcode("ParameterDeclaration");
  const XMLCh* current_element_ = nullptr;
  const XMLCh* last_element_ = nullptr;

  std::map<std::string, std::string>* parameter_map_;
};

class XOSCBase_1_0 {
 public:
  XOSCBase_1_0();
  ~XOSCBase_1_0();

  static void ParameterDeclarationPreprocess(const std::string& input);

  static bool IsLatLongPj(const std::string& s);

  int CanSaveXOSC(CSimulation& simulation);
  int InitMapQuery(CSimulation& simulation, bool enable_load_hadmap = true);
  int InitMapQueryCloud(CSimulation& simulation, std::string strMapFilePath) { return 0; }
  const char* XODRCoordinator(CSimulation& simulation);
  const char* XODRCoordinator(std::string strMapFile);
  const char* XODRCoordinatorCloud(std::string strMapFile);

  const char* ConvertLonLat2XODR(double& lon, double& lat);
  const char* ConvertXODR2LonLat(double& x, double& y);

  int ConvertSimPosition2LonLat(double dLon, double dLat, lanepkid laneID, double dShift, double dOffset,
                                double& dFinalLon, double& dFinalLat);
  int ConvertSimPosition2Local(double dLon, double dLat, lanepkid laneID, double dShift, double dOffset, double& dX,
                               double& dY);

  int ConvertSimPosition2Local(double dLon, double dLat, std::string strlaneID, std::string strShift,
                               std::string strOffset, double& dX, double& dY);

  int ConvertLonLat2Local(double& dX, double& dY);
  int ConvertLonLat2SimPosition(double dLon, double dLat, sTagQueryInfoResult& result);
  int ConvertLonLat2SimPosition(roadpkid roadID, /*sectionpkid secID,*/ lanepkid laneID, double dShift, double dOffset,
                                sTagQueryInfoResult& result);
  int ConvertLonLat2SimPosition(roadpkid roadID, sectionpkid secID, lanepkid laneID, double dShift, double dOffset,
                                sTagQueryInfoResult& result);
  int ConvertLonLat2SimPosition(roadpkid roadID, double dShift, double dOffset, sTagQueryInfoResult& result);

  int ExtractPosition(CTraffic& traffic, std::string strRouteID, lanepkid laneID, double dShift, double Offset,
                      double& dFinalLon, double& dFinalLat, double& dFinalYaw, bool isLonLat = true);
  int ExtractPosition(CTraffic& traffic, std::string strRouteID, std::string strlaneID, std::string strShift,
                      std::string strOffset, double& dFinalLon, double& dFinalLat, double& dFinalYaw,
                      bool isLonLat = true);

  int ExtractPosition(CRoute& r, lanepkid laneID, double dShift, double Offset, double& dFinalLon, double& dFinalLat,
                      double& dFinalYaw, bool isLonLat = true);
  int ExtractPosition(CRoute& r, std::string strlaneID, std::string strShift, std::string strOffset, double& dFinalLon,
                      double& dFinalLat, double& dFinalYaw, bool isLonLat = true);

  int GetYaw(double dLon, double dLat, lanepkid laneID, double dShift, double dOffset, double& dFinalYaw);

  int GetNearByLaneFromStart(double dLon, double dLat, hadmap::txLanePtr& pLane);
  int GetNearByLaneFromEnd(double dLon, double dLat, hadmap::txLanePtr& pLane);
  //
  std::string m_coordnator_str;
  XOSC_MAP_TYPE m_mapType = MAP_XODR;
  std::string m_actualMap = "";
  double m_dSourth;
  double m_dWest;
  double m_dEast;
  double m_dNorth;

  CMapQuery* m_query;

  bool m_delete_query;
#ifdef PROJ_NEW_API
  // xodr to lon lat cs2cs
  PJ* m_PJ = NULL;
  // lonlat to xodr cs2cs
  PJ* m_PJ2 = NULL;
  PJ_CONTEXT* m_C = NULL;
#else
  std::map<std::string, projPJ> pj_map_;
#endif

  int GetTrafficLightInfo(const std::string strId, std::string& routeId, std::string& phase, std::string& lane,
                          std::string& junction, std::string& roadId, hadmap::txPoint& endp);

  std::string DoubleToString(double& DD, int nlen = 12);

  static constexpr const char* s_Lonlat_Coordinator_Str = "+proj=longlat +datum=WGS84 +no_defs";

  static std::set<std::string> m_set_PedestrianType;
};
