// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <mapengine/hadmap_codes.h>
#include <mapengine/hadmap_engine.h>
#include "json/json.h"
#include "structs/hadmap_header.h"
#include "tx_od_header.h"

using Id2RoadMap = std::map<std::string, hadmap::txRoadPtr>;
using Id2LaneLinkMap = std::map<std::string, hadmap::txLaneLinkPtr>;
using Id2Objects = std::map<std::string, hadmap::txObjectPtr>;
using Id2Junctions = std::map<std::string, hadmap::txJunctionPtr>;

namespace ODR {
struct ModifyInfo {
  std::string strCmd;
  std::string strType;
  std::string strId;
};

class CHadmap {
 public:
  static std::map<std::string, std::string> SplitToMap(const std::string&, const std::string&, const std::string&);

  CHadmap();

  ~CHadmap();

  Base::txInt Open(const char* path, bool close, std::stringstream& ss);

  Base::txInt Save(const char* path, bool close, const std::stringstream& ss);

  Base::txInt ParseMap(const char* hadmapPath);

  Base::txInt CreateMap(const char* hadmapPath, const std::stringstream& cmd_json);

  Base::txInt ModifyMap(const std::stringstream& cmd_json);

  Base::txInt ModifyMap(Json::Value& root);

  Base::txInt SaveMap(const char* hadmapPath);

  Base::txInt ParseDataToJson(Base::txStringStream& jsonValue);

  hadmap::MAP_DATA_TYPE getHadmapDataType();

  Base::txInt CloseMap();

 private:
  template <typename ExecutorMap, typename DataMap>
  void ModifyMapImpl(const ModifyInfo& info, const ExecutorMap& executor_map, const DataMap& data_map) {
    if (executor_map.count(info.strCmd) <= 0) {
      return;
    }

    auto iter = data_map.find(info.strId);
    if (iter == data_map.end() || !iter->second) {
      LOG(WARNING) << "Not find " << info.strType << " id: " << info.strId;
      return;
    }
    int ret = executor_map.at(info.strCmd)(m_pMapHandler, {iter->second});
    LOG_IF(WARNING, ret != 0) << info.strCmd << " " << info.strType << " ret: " << ret;
  }

  void RoadsToJson(const hadmap::txRoads roads, Json::Value& json_value);

  void LaneLinkToJson(const hadmap::txRoads roads, const hadmap::txLaneLinks links, Json::Value& json_value);

  void ObjectsToJson(const hadmap::txObjects& objects, Json::Value& json_value);

  void CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points);

  void CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points, int nPointsSize);

  void CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points, const double start_s, const double end_s,
                     bool isEnd = false);

  std::string formatIntValue(int64_t data);

  int JsonToMap(Json::Value& json_roads);

  int ParseRoads(Json::Value& roads, Id2RoadMap& map);

  int ParseLanelinks(Json::Value& lanelinks, Id2LaneLinkMap& map);

  int ParseObjects(Json::Value& objects, Id2Objects& map);

  bool makeInitXodr(const char* filePath, std::string maptype, std::string mapversion);
  // int ParseObjects(Json::Value & objects,);

  hadmap::txMapHandle* m_pMapHandler = nullptr;

 private:
  // hadmap::txPoint m_refPoint;

  void getObjectType(std::string name, std::string strType, std::string strSubType, hadmap::OBJECT_TYPE nType,
                     hadmap::OBJECT_SUB_TYPE);

  int GetStartAndEndType(const hadmap::txPoint& point, const hadmap::txLanePtr& ptr);

  std::string formatJosnValue(Json::Value& value);

  void roadSt2xy(int roadid, int lanelinkid, double s, double t, double& lon, double& lat);

  void lonlat2xy(std::vector<hadmap::txPoint>& points);

  hadmap::MAP_DATA_TYPE m_dataType;

  void lonlat2xy(double& lon, double& lat);

  void xy2lonlat(double& x, double& y);

  void replace_str(std::string& str, const std::string& before, const std::string& after);

  // std::vector<ModifyInfo> m_ModifyInfoVec;
  // hadmap::txRoads m_roads;
  // hadmap::txLanes m_lanes;

 private:
  static const std::map<std::string, hadmap::OBJECT_COLOR> kObjectStrColorDict;
};

}  // namespace ODR
