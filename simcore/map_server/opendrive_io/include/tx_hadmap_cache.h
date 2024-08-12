// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "json/json.h"
#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"
#include "structs/hadmap_header.h"
#include "tx_od_header.h"

using Id2RoadMap = std::map<std::string, hadmap::txRoadPtr>;
using Id2LaneLinkMap = std::map<std::string, hadmap::txLaneLinkPtr>;
using Id2Objects = std::map<std::string, hadmap::txObjectPtr>;
using Id2Junctions = std::map<std::string, hadmap::txJunctionPtr>;

namespace ODR {
/*
 * Map modification information structure
 */
struct ModifyInfo {
  std::string strCmd;   // Modification command: add/update/delete
  std::string strType;  // Modified object data type: road/object/lanelink
  std::string strId;    // Modified object ID
};
class CHadmap {
 public:
  static std::map<std::string, std::string> SplitToMap(const std::string&, const std::string&, const std::string&);

  CHadmap();

  ~CHadmap();
  /*
   *Open map (used in cloud)
   * @param [in] const char* path: Full path of the map;
   * @param [in] bool close: Whether to close the map after opening it;
   * @param [out] std::stringstream ss: map data
   * @return Status code
   */
  Base::txInt Open(const char* path, bool close, std::stringstream& ss);

  /*
   * Save map used in cloud
   *
   * @param [in] const char* path: Full path of the map.
   * @param [in] bool close: Whether to close the map after opening it.
   * @param [in] std::stringstream ss: map data
   * @return Status code
   */
  Base::txInt Save(const char* path, bool close, const std::stringstream& ss);

  /*
   * parse map
   *
   * @param [in]  const char* hadmapPath:Full path of the map.
   * @return Status code
   */
  Base::txInt ParseMap(const char* hadmapPath);

  /*
   * Create map
   *
   * @param [in]  const char* hadmapPath:Full path of the map.
   * @param [in]  const std::stringstream& cmd_json:map data json
   * @return Status code
   */
  Base::txInt CreateMap(const char* hadmapPath, const std::stringstream& cmd_json);

  /*
   * modify map
   *
   * @param [in]  const std::stringstream& cmd_json /Json::Value& root : map data json
   * @return Status code
   */
  Base::txInt ModifyMap(const std::stringstream& cmd_json);
  Base::txInt ModifyMap(Json::Value& root);

  /*
   * Save the current map
   *
   * @param [in]  const char* hadmapPath :Full path of the map.
   * @return Status code
   */
  Base::txInt SaveMap(const char* hadmapPath);

  /*
   * To convert parsed data into JSON data
   *
   * @param [out]  Base::txStringStream& jsonValue : map data json
   * @return Status code
   */
  Base::txInt ParseDataToJson(Base::txStringStream& jsonValue);

  /*
   * Get the current map type
   *
   * @return MAP_DATA_TYPE
   */
  hadmap::MAP_DATA_TYPE getHadmapDataType() { return this->m_dataType; }

  /*
   * Close the currently opened map
   *
   * @return Status Code
   */
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
  /*
   * Generate JSON data from hadmap::txRoads
   */
  void RoadsToJson(const hadmap::txRoads roads, Json::Value& json_value);

  /*
   * Generate JSON data from hadmap::txLaneLinks
   */
  void LaneLinkToJson(const hadmap::txRoads roads, const hadmap::txLaneLinks links, Json::Value& json_value);

  /*
   * Generate JSON data from hadmap::txObjects
   */
  void ObjectsToJson(const hadmap::txObjects& objects, Json::Value& json_value);

  /*
   * Sample points from a HAD map curve
   */
  void CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points);

  void CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points, int nPointsSize);

  void CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points, const double start_s, const double end_s,
                     bool isEnd = false);
  /*
   * Generate HD Map from JSON data
   */
  int JsonToMap(Json::Value& json_roads);

  /*
   * Generate hadmap road data from JSON data
   */
  int ParseRoads(Json::Value& roads, Id2RoadMap& map);

  /*
   * Generate hadmap lanelink data from JSON data
   */
  int ParseLanelinks(Json::Value& lanelinks, Id2LaneLinkMap& map);

  /*
   * Generate hadmap object data from JSON data
   */
  int ParseObjects(Json::Value& objects, Id2Objects& map);

  /*
   * Generate initial map data (OpenDrive contains only header)
   */
  bool MakeInitXodr(const char* filePath, std::string maptype, std::string mapversion);

 private:
  int GetStartAndEndType(const hadmap::txPoint& point, const hadmap::txLanePtr& ptr);
  /*
   * Convert road ST to XY coordinates
   */
  void RoadSt2XY(int roadid, int lanelinkid, double s, double t, double& lon, double& lat);

  /*
   * Convert lon/lat points to XY coordinates
   */
  void LonLatToXY(std::vector<hadmap::txPoint>& points);

  /*
   * Convert lon/lat to XY coordinates
   */
  void LonLatToXY(double& lon, double& lat);

  /*
   * Convert XY to lon/lat coordinates
   */
  void XY2LonLat(double& x, double& y);

  void ReplaceStr(std::string& str, const std::string& before, const std::string& after);

  /*
   * Convert Json value to String
   */
  Base::txString FormatJosnValue(Json::Value& value) {
    if (value.isString()) return value.asString();
    return Base::txString();
  }
  /*
   * Convert Int value to String
   */
  Base::txString FormatIntValue(Base::txInt data) { return std::to_string(data); }

  Base::txFloat FormatDoubleValue(Base::txFloat value) { return std::round(value * 1000) / 1000; }

 private:
  hadmap::txMapHandle* m_pMapHandler = nullptr;

  hadmap::MAP_DATA_TYPE m_dataType;

  // std::vector<ModifyInfo> m_ModifyInfoVec;
  // hadmap::txRoads m_roads;
  // hadmap::txLanes m_lanes;
  hadmap::txPoint m_refPoint = {0, 0, 0};

 private:
  static const std::map<std::string, hadmap::OBJECT_COLOR> kObjectStrColorDict;
};

}  // namespace ODR
