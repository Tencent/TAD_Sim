// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_hadmap_cache.h"

#include <ctime>
#include <fstream>
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
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>

#include "common/coord_trans.h"

namespace ODR {

constexpr const char* kXodrTpl = R"(
<?xml version="1.0" encoding="UTF-8"?>
<OpenDRIVE>
  <header revMajor="1" revMinor="$revMinor$" name="" version="1.0" date="$date$"
north="0" south="0" west="0" east="0" vendor="$version$">
    <geoReference>
     <![CDATA[+proj=tmerc +ellps=WGS84 +datum=WGS84 +k_0=1 +lon_0=113.329232210708 +lat_0=23.23445 +x_0=0 +y_0=0 +units=m]]>
    </geoReference>
  </header>
</OpenDRIVE>
)";

constexpr const char* kRevMinorPlaceholder = "$revMinor$";

constexpr const char* kDatePlaceholder = "$date$";

constexpr const char* kVersionPlaceholder = "$version$";

const std::map<std::string, hadmap::OBJECT_COLOR> CHadmap::kObjectStrColorDict = {
    {"white", hadmap::OBJECT_COLOR_White},
    {"yellow", hadmap::OBJECT_COLOR_Yellow},
    {"blue", hadmap::OBJECT_COLOR_Blue},
    {"green", hadmap::OBJECT_COLOR_Green},
    {"red", hadmap::OBJECT_COLOR_Red}};

std::map<std::string, std::string> CHadmap::SplitToMap(const std::string& in, const std::string& s1,
                                                       const std::string& s2) {
  std::map<std::string, std::string> res;

  std::vector<std::string> tokens;
  boost::algorithm::split(tokens, in, boost::is_any_of(s1));
  for (const auto& token : tokens) {
    std::vector<std::string> kv;
    boost::algorithm::split(kv, token, boost::is_any_of(s2));
    if (kv.empty() || kv[0].empty()) {
      continue;
    }
    res[kv[0]] = kv.size() > 1 ? kv[1] : "";
  }

  return res;
}

CHadmap::CHadmap() {}

CHadmap::~CHadmap() { CloseMap(); }

Base::txInt CHadmap::Open(const char* path, bool close, std::stringstream& ss) {
  // Close the map
  CloseMap();

  // Initialize the return value
  int ret = 0;

  // Check if the map handler is empty and if the path is not empty
  if (!m_pMapHandler && path) {
    // Parse the map
    ret = ParseMap(path);
  }

  // Check if the map handler is empty or if the return value is not 0
  if (!m_pMapHandler || ret != 0) {
    return -1;
  }

  // Parse the data to JSON
  ret = ParseDataToJson(ss);

  // If the close flag is set, close the map
  if (close) {
    CloseMap();
  }

  // Return the result
  return ret;
}

Base::txInt CHadmap::Save(const char* path, bool close, const std::stringstream& ss) {
  // Close the last map
  CloseMap();

  // Initialize the JSON reader
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  std::string strJson = ss.str();
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson.c_str());

  // Parse the JSON data
  if (!reader->parse(strJson.c_str(), strJson.c_str() + nLen, &root, &errs)) {
    LogWarn << ("parser json %s failed: ", strJson.c_str());
    return -1;
  }

  // Check if the JSON data contains the "data" key
  if (!root.isMember("data")) {
    LogWarn << "invalid param: key `data` is missing";
    return -1;
  }

  // Parse the map
  if (ParseMap(path) < 0) {
    LogWarn << ("invalid hadmap: load map fail , path = ", path);
  }

  // Modify the map
  int modify_ret = ModifyMap(root["data"]);
  if (modify_ret != 1) {
    if (close || !!modify_ret) {
      CloseMap();
    }
    return modify_ret;
  }

  // If is not modify, create the map
  int ret = CreateMap(path, ss);
  if (close) {
    CloseMap();
  }
  return ret;
}

Base::txInt CHadmap::ParseMap(const char* hadmapPath) {
  // Check if the input parameter is empty
  if (!hadmapPath) {
    LogWarn << ("map file name is null");
    return -1;
  }
  // Get the map file path
  boost::filesystem::path pathMapFile = hadmapPath;
  // Check if the map file exists
  if (!boost::filesystem::exists(pathMapFile)) {
    LogWarn << "map file " << hadmapPath << " not exist!";
    return -1;
  }
  // Get the map file extension
  std::string strExt = pathMapFile.extension().string();
  // Set the map type
  int nMapType = hadmap::SQLITE;
  if (boost::algorithm::iequals(".sqlite", strExt)) {
    nMapType = hadmap::SQLITE;
  } else if (boost::algorithm::iequals(".xodr", strExt)) {
    nMapType = hadmap::OPENDRIVE;
  } else {
    LogWarn << ("unknown map type: %s!", hadmapPath);
    return -1;
  }
  // Load the map using the map SDK
  m_pMapHandler = NULL;
  int nRet = 0;
  nRet = hadmap::hadmapConnect(hadmapPath, static_cast<hadmap::MAP_DATA_TYPE>(nMapType), &m_pMapHandler, false);
  // Check if the map file was loaded successfully
  if (nRet != TX_HADMAP_HANDLE_OK) {
    LogWarn << ("hadmap file %s open failed!", hadmapPath);
    return -1;
  }
  // Set the map data type
  m_dataType = static_cast<hadmap::MAP_DATA_TYPE>(nMapType);
  return 0;
}

Base::txInt CHadmap::CreateMap(const char* hadmapPath, const std::stringstream& cmd_json) {
  // Check if the input parameter is empty
  if (!hadmapPath) {
    LogWarn << ("map file name is null");
    return -1;
  }
  // Get the map file path
  boost::filesystem::path pathMapFile = hadmapPath;
  // Check if the map file already exists and delete it if it does
  if (boost::filesystem::exists(pathMapFile)) {
    try {
      int bRet = boost::filesystem::remove(hadmapPath);
    } catch (const std::exception& e) {
      std::string str = e.what();
      LogWarn << ("remove file error=%s", str.c_str());
    }
  }
  LogWarn << "start create file " << pathMapFile.c_str();
  // Get the map file extension
  std::string strExt = pathMapFile.extension().string();
  // Set the map type
  int nMapType = hadmap::SQLITE;
  if (strExt == ".sqlite") {
    nMapType = hadmap::SQLITE;
    this->m_dataType = hadmap::SQLITE;
  } else {
    nMapType = hadmap::OPENDRIVE;
    this->m_dataType = hadmap::OPENDRIVE;
  }
  // Initialize the map handler
  m_pMapHandler = NULL;
  int nRet = 0;
  // If the map type is SQLite
  if (nMapType == hadmap::SQLITE) {
    nRet = hadmap::hadmapConnect(hadmapPath, static_cast<hadmap::MAP_DATA_TYPE>(nMapType), &m_pMapHandler);
    if (nRet != TX_HADMAP_HANDLE_OK) {
      LogWarn << ("hadmap file %s create failed!", hadmapPath);
      return -1;
    }
    LogInfo << "hadmap connect sucess";
    // Parse the JSON data and convert it to map data
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING errs;
    std::string strJson = cmd_json.str();
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    int nLen = strlen(strJson.c_str());
    if (!reader->parse(strJson.c_str(), strJson.c_str() + nLen, &root, &errs)) {
      LogWarn << ("parser json error");
      return -1;
    }
    if (!root["data"].empty() && JsonToMap(root["data"]) != 0) {
      LogWarn << ("Json to map error");
      return -1;
    }
    LogInfo << "Hadmap connect sucess";
  } else if ((nMapType == hadmap::OPENDRIVE)) {
    // If the map type is OpenDRIVE
    // Parse the JSON data and convert it to map data
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING errs;
    std::string strJson = cmd_json.str();
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    LogInfo << ("start output file");
    int nLen = strlen(strJson.c_str());
    if (!reader->parse(strJson.c_str(), strJson.c_str() + nLen, &root, &errs) || !root.isObject() ||
        !root.isMember("data")) {
      LogWarn << ("parser json error, or missing key `data` in object");
      return -1;
    }
    // Initialize the OpenDRIVE map
    Json::Value header = root["data"].get("header", Json::Value(Json::objectValue));
    std::string map_type = header.get("map_type", "").asString();
    std::string map_version = header.get("version", "").asString();
    MakeInitXodr(hadmapPath, map_type, map_version);
    // Connect the map
    nRet = hadmap::hadmapConnect(hadmapPath, static_cast<hadmap::MAP_DATA_TYPE>(nMapType), &m_pMapHandler, false);
    if (!root["data"].empty() && JsonToMap(root["data"]) != 0) {
      LogWarn << ("Json to map error");
      return -1;
    }
    LogInfo << ("output file end");
  }
  return 0;
}

Base::txInt CHadmap::ModifyMap(const std::stringstream& cmd_json) {
  // Initialize the JSON reader
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  std::string strJson = cmd_json.str();
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson.c_str());

  // Parse the JSON data
  if (!reader->parse(strJson.c_str(), strJson.c_str() + nLen, &root, &errs)) {
    LogWarn << ("parser json %s failed: ", strJson.c_str());
    return -1;
  }

  // Modify the map using the parsed JSON data
  return ModifyMap(root);
}

Base::txInt CHadmap::ModifyMap(Json::Value& root) {
  // Check if the root is a JSON object
  if (!root.isObject()) {
    LogWarn << "invalid param, must be json object";
    return -1;
  }

  // Check if the root contains the "datas" member
  if (!root.isMember("datas")) {
    LogWarn << "not found member `datas`, is not modify save";
    return 1;
  }

  // Parse the "datas" member
  std::vector<ModifyInfo> modifyInfoVec;
  for (auto it : root["datas"]) {
    ModifyInfo info;
    info.strCmd = FormatJosnValue(it["cmd"]);
    info.strType = FormatJosnValue(it["type"]);
    info.strId = FormatJosnValue(it["id"]);
    modifyInfoVec.push_back(info);
  }

  // Check if the parsed data is empty
  if (modifyInfoVec.empty()) {
    LogWarn << "found member `datas`, but no change";
    return 0;
  }

  // Check if the map is opened
  if (!m_pMapHandler) {
    LogWarn << "the map is not opened, stop saving";
    return -1;
  }

  // Parse roads, lanelinks, and objects
  Id2RoadMap roadmap;
  Id2LaneLinkMap linkmap;
  Id2Objects objectmap;
  if (ParseRoads(root["roads"], roadmap) || ParseLanelinks(root["lanelinks"], linkmap) ||
      ParseObjects(root["objects"], objectmap)) {
    return -1;
  }
  LogInfo << "road, lanelink, object size: " << roadmap.size() << ", " << linkmap.size() << ", " << objectmap.size();

  // Define lambda functions for modifying the map
  auto modify_map_road_impl = [this](const ModifyInfo& it, Id2RoadMap& data_map) {
    using ExecutorType = std::function<int(hadmap::txMapHandle*, hadmap::txRoads)>;
    std::map<std::string, ExecutorType> executor_map = {
        {"add", hadmap::insertRoads}, {"update", hadmap::updateRoads}, {"delete", hadmap::deleteRoads}};

    if (it.strCmd == "delete") {
      hadmap::getRoad(m_pMapHandler, std::atoi(it.strId.c_str()), true, data_map[it.strId]);
    }
    ModifyMapImpl(it, executor_map, data_map);
  };

  auto modify_map_lanelink_impl = [this](const ModifyInfo& it, Id2LaneLinkMap& data_map) {
    using ExecutorType = std::function<int(hadmap::txMapHandle*, hadmap::txLaneLinks)>;
    std::map<std::string, ExecutorType> executor_map = {
        {"add", hadmap::insertLaneLinks}, {"update", hadmap::updateLaneLinks}, {"delete", hadmap::deleteLaneLinks}};

    if (it.strCmd == "delete") {
      hadmap::getLaneLink(m_pMapHandler, std::atoi(it.strId.c_str()), data_map[it.strId]);
    }
    ModifyMapImpl(it, executor_map, data_map);
  };

  auto modify_map_object_impl = [this](const ModifyInfo& it, Id2Objects& data_map) {
    using ExecutorType = std::function<int(hadmap::txMapHandle*, hadmap::txObjects)>;
    std::map<std::string, ExecutorType> executor_map = {
        {"add", hadmap::insertObjects}, {"update", hadmap::updateObjects}, {"delete", hadmap::deleteObjects}};

    if (it.strCmd == "delete") {
      hadmap::txObjects objects;
      hadmap::getObjects(m_pMapHandler, std::vector<hadmap::txLaneId>{}, {}, objects);
      for (auto object : objects) {
        if (object->getId() != std::atoi(it.strId.c_str())) {
          continue;
        }
        data_map[it.strId] = object;
        break;
      }
    }
    ModifyMapImpl(it, executor_map, data_map);
  };
  // Iterate through the modifyInfoVec and apply the appropriate modification
  for (auto it : modifyInfoVec) {
    if (it.strType == "road") {
      modify_map_road_impl(it, roadmap);
    } else if (it.strType == "lanelink") {
      modify_map_lanelink_impl(it, linkmap);
    } else if (it.strType == "object") {
      modify_map_object_impl(it, objectmap);
    }
  }
  hadmap::hadmapSave(m_pMapHandler);
  return 0;
}

Base::txInt CHadmap::SaveMap(const char* hadmapPath) {
  // Check if the map path is empty
  if (!hadmapPath) {
    LogWarn << ("map file name is null");
    return -1;
  }

  // Create a boost filesystem path object
  boost::filesystem::path pathMapFile = hadmapPath;

  // Check if the map file already exists
  if (boost::filesystem::exists(pathMapFile)) {
    LogWarn << ("map file name is exist %s") << hadmapPath;
    return -2;
  }

  // Get the file extension
  std::string strExt = pathMapFile.extension().string();

  // Determine the map type based on the file extension
  int nMapType = hadmap::SQLITE;
  if (boost::algorithm::iequals(".sqlite", strExt)) {
    nMapType = hadmap::SQLITE;
  } else if (boost::algorithm::iequals(".xodr", strExt)) {
    nMapType = hadmap::OPENDRIVE;
  } else {
    return -3;
  }

  // Connect to the map database
  return hadmap::hadmapConnect(hadmapPath, (hadmap::MAP_DATA_TYPE)nMapType, &m_pMapHandler, false);
}

Base::txInt CHadmap::CloseMap() {
  // Check if the map path is open
  if (m_pMapHandler) {
    hadmap::hadmapClose(&m_pMapHandler);
    m_pMapHandler = NULL;
  }
  return 0;
}

Base::txInt CHadmap::ParseDataToJson(Base::txStringStream& jsonValue) {
  bool bWholeData = true;
  int nRet = 0;
  hadmap::txRoads roads;
  nRet = hadmap::getRoads(m_pMapHandler, &bWholeData, roads);
  if (nRet != TX_HADMAP_DATA_OK && nRet != TX_HADMAP_DATA_EMPTY) {
    LogWarn << ("hadmap::getRoads failed!");
    return -1;
  }
  m_refPoint.x = 0;
  m_refPoint.y = 0;
  if (m_dataType == hadmap::MAP_DATA_TYPE::SQLITE) {
    double minx = 1e10;
    double maxx = -1e10;
    double miny = 1e10;
    double maxy = -1e10;
    for (auto& it : roads) {
      if (it->getGeometry()) {
        hadmap::txPoint start = it->getGeometry()->getStart();
        if (start.x > maxx) {
          maxx = start.x;
        }
        if (start.x < minx) {
          minx = start.x;
        }
        if (start.y > maxy) {
          maxy = start.y;
        }
        if (start.y < miny) {
          miny = start.y;
        }
      }
    }
    m_refPoint.x = (minx + maxx) / 2.0;
    m_refPoint.y = (miny + maxy) / 2.0;
    LonLatToXY(m_refPoint.x, m_refPoint.y);
  } else {
    double minx = 1e10;
    double maxx = -1e10;
    double miny = 1e10;
    double maxy = -1e10;
    for (auto& it : roads) {
      if (it->getGeometry()) {
        hadmap::txPoint start = it->getGeometry()->getStart();
        if (start.x > maxx) {
          maxx = start.x;
        }
        if (start.x < minx) {
          minx = start.x;
        }
        if (start.y > maxy) {
          maxy = start.y;
        }
        if (start.y < miny) {
          miny = start.y;
        }
      }
    }
    double ref_lon = (minx + maxx) / 2.0;
    double ref_lat = (miny + maxy) / 2.0;
    LonLatToXY(ref_lon, ref_lat);
    if (ref_lon > 1e5 || ref_lat > 1e5) {
      m_refPoint.x = ref_lon;
      m_refPoint.y = ref_lat;
    }
  }
  // start make json
  Json::Value root_json;

  // Convert header to json
  hadmap::txOdHeaderPtr header = NULL;
  hadmap::getHeader(m_pMapHandler, header);
  if (header != NULL) {
    Json::Value headerJson;
    if (header->getVendor() == "tadsim" || header->getVendor() == "tadsim v1.0") {
      headerJson["version"] = "tadsim v1.0";
    } else if (header->getVendor() == "tadsim v2.0") {
      headerJson["version"] = "tadsim v2.0";
    } else {
      headerJson["version"] = "three party";
    }
    if (header->getRevMinor() == 5) {
      headerJson["map_type"] = "xodr 1.5";
    } else {
      headerJson["map_type"] = "xodr 1.4";
    }
    root_json["header"] = headerJson;
  } else {
    Json::Value headerJson;
    headerJson["version"] = "tadsim v2.0";
    headerJson["map_type"] = "sqlite";
    root_json["header"] = headerJson;
  }

  // 1)road to json
  LogWarn << "road to json";
  RoadsToJson(roads, root_json);

  // Get lane links
  hadmap::txLaneLinks lanelinks;
  hadmap::roadpkid fromRoadId = ROAD_PKID_INVALID;
  hadmap::roadpkid toRoadId = ROAD_PKID_INVALID;
  nRet = hadmap::getLaneLinks(m_pMapHandler, fromRoadId, toRoadId, lanelinks);
  if (nRet != TX_HADMAP_DATA_OK && nRet != TX_HADMAP_DATA_EMPTY) {
    LogWarn << ("hadmap::getLaneLinks failed!");
    return -1;
  }
  LogWarn << "road to json finished";

  // 2)lanelink to json;
  LogWarn << "lanelinks to json";
  LaneLinkToJson(roads, lanelinks, root_json);
  LogWarn << "lanelinks to json finshed";
  const std::vector<hadmap::txLaneId> laneIds;
  const std::vector<hadmap::OBJECT_TYPE> types;
  // 3)objects to json
  LogWarn << "objects to json";
  hadmap::txObjects objects;
  nRet = hadmap::getObjects(m_pMapHandler, laneIds, types, objects);
  ObjectsToJson(objects, root_json);
  LogWarn << "objects to json finshed";
  // 4)json to stringstream
  Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "";
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream ss;
  writer->write(root_json, &ss);
  jsonValue << ss.str();

  // Return success
  return 0;
}

void CHadmap::RoadsToJson(const hadmap::txRoads roads, Json::Value& json_value) {
  // Iterate through each element in 'roads'
  for (auto it : roads) {
    Json::Value jsonRoad;
    jsonRoad["id"] = FormatIntValue(it->getId());
    jsonRoad["length"] = it->getLength();
    jsonRoad["type"] = it->getRoadType();
    const hadmap::txCurve* curvePtr = it->getGeometry();
    hadmap::txLanes lanes;

    // Check if the curve pointer is not null and if the first section has at least one lane
    if (NonNull_Pointer(curvePtr) && it->getSections().size() > 0 && it->getSections().at(0)->getLanes().size() > 0) {
      double start_s = 0.0;
      uint64_t startIndex = 0;
      uint64_t endIndex = 0;
      double last_length_sum = 0;

      // Iterate through each section in the road
      for (size_t i = 0; i < it->getSections().size(); i++) {
        hadmap::PointVec points;
        Json::Value jsonSection;
        hadmap::txSectionPtr sectionPtr = it->getSections().at(i);
        jsonSection["id"] = FormatIntValue(sectionPtr->getId());
        double lengthSection = 0.0;

        // Get the length of the section
        if (NonNull_Pointer(sectionPtr->get(-1))) {
          lengthSection = sectionPtr->get(-1)->getLeftBoundary()->getGeometry()->getLength();
        } else {
          lengthSection = it->getSections().at(i)->getLength();
        }

        // Sample points along the curve
        bool isEndSection = false;
        if (i == it->getSections().size() - 1) isEndSection = true;
        CurveToPoints(curvePtr, points, start_s, start_s + lengthSection, isEndSection);

        int currentPointSize = points.size();
        endIndex = startIndex + points.size() - 1;
        jsonSection["start"] = startIndex;
        jsonSection["end"] = endIndex;
        jsonSection["length"] = lengthSection;

        // Calculate start and end percentages
        jsonSection["startPercent"] = last_length_sum / it->getLength();
        last_length_sum += it->getSections().at(i)->getLength();
        jsonSection["endPercent"] = last_length_sum / it->getLength();

        // Set the end percentage to 1.0 for the last section of the road
        if (i == it->getSections().size() - 1) {
          jsonSection["endPercent"] = 1.0;
        }

        // Add sample points to the JSON section
        for (size_t j = 0; j < points.size(); j++) {
          Json::Value jsonPoint;
          jsonPoint["x"] = FormatDoubleValue(points.at(j).x);
          jsonPoint["y"] = FormatDoubleValue(points.at(j).y);
          jsonPoint["z"] = FormatDoubleValue(points.at(j).z);
          jsonRoad["samplePoints"].append(jsonPoint);
        }

        startIndex = (endIndex + 1);
        start_s += lengthSection;
        hadmap::txLanes tmpLanes = it->getSections().at(i)->getLanes();

        // Set of boundary IDs to avoid duplicates
        std::set<hadmap::laneboundarypkid> boundaryIdset;

        // Iterate through each lane in the section
        for (size_t j = 0; j < tmpLanes.size(); j++) {
          Json::Value laneJson;
          laneJson["id"] = FormatIntValue(tmpLanes.at(j)->getId());
          laneJson["lbid"] = FormatIntValue(tmpLanes.at(j)->getLeftBoundaryId());
          laneJson["rbid"] = FormatIntValue(tmpLanes.at(j)->getRightBoundaryId());
          laneJson["type"] = tmpLanes.at(j)->getLaneType();
          laneJson["speedlimit"] = tmpLanes.at(j)->getSpeedLimit();
          laneJson["friction"] = tmpLanes.at(j)->getLaneFriction();
          // Round to three decimal places
          laneJson["sOffset"] = std::round(tmpLanes.at(j)->getMaterialOffset() * 1000) / 1000;
          const hadmap::txCurve* laneCurve = tmpLanes.at(j)->getGeometry();
          hadmap::PointVec lanePoints;

          // Convert the lane curve to points
          CurveToPoints(laneCurve, lanePoints, currentPointSize);

          // Add sample points to the JSON lane
          for (size_t m = 0; m < lanePoints.size(); m++) {
            Json::Value jsonPoint;
            jsonPoint["x"] = FormatDoubleValue(lanePoints.at(m).x);
            jsonPoint["y"] = FormatDoubleValue(lanePoints.at(m).y);
            jsonPoint["z"] = FormatDoubleValue(lanePoints.at(m).z);
            laneJson["samplePoints"].append(jsonPoint);
          }

          // Convert the left boundary curve to points
          Json::Value leftBoudaryJson;
          hadmap::PointVec leftBoudaryPoints;
          hadmap::txCurve* leftBoudary = (hadmap::txCurve*)tmpLanes.at(j)->getLeftBoundary()->getGeometry();
          CurveToPoints(leftBoudary, leftBoudaryPoints, currentPointSize);

          // Add sample points to the JSON left boundary
          for (size_t m = 0; m < leftBoudaryPoints.size(); m++) {
            Json::Value jsonPoint;
            jsonPoint["x"] = FormatDoubleValue(leftBoudaryPoints.at(m).x);
            jsonPoint["y"] = FormatDoubleValue(leftBoudaryPoints.at(m).y);
            jsonPoint["z"] = FormatDoubleValue(leftBoudaryPoints.at(m).z);
            leftBoudaryJson["samplePoints"].append(jsonPoint);
          }

          leftBoudaryJson["mark"] = tmpLanes.at(j)->getLeftBoundary()->getLaneMark();
          leftBoudaryJson["id"] = FormatIntValue(tmpLanes.at(j)->getLeftBoundary()->getId());

          // Add the left boundary to the JSON section if it's not a duplicate
          if (boundaryIdset.find(tmpLanes.at(j)->getLeftBoundary()->getId()) == boundaryIdset.end()) {
            jsonSection["boundarys"].append(leftBoudaryJson);
            boundaryIdset.insert(tmpLanes.at(j)->getLeftBoundary()->getId());
          }

          // Convert the right boundary curve to points
          Json::Value rightBoudaryJson;
          hadmap::PointVec rightBoudaryPoints;
          const hadmap::txCurve* rightBoudary = tmpLanes.at(j)->getRightBoundary()->getGeometry();
          CurveToPoints(rightBoudary, rightBoudaryPoints, currentPointSize);

          // Add sample points to the JSON right boundary
          for (size_t m = 0; m < rightBoudaryPoints.size(); m++) {
            Json::Value jsonPoint;
            jsonPoint["x"] = FormatDoubleValue(rightBoudaryPoints.at(m).x);
            jsonPoint["y"] = FormatDoubleValue(rightBoudaryPoints.at(m).y);
            jsonPoint["z"] = FormatDoubleValue(rightBoudaryPoints.at(m).z);
            rightBoudaryJson["samplePoints"].append(jsonPoint);
          }

          rightBoudaryJson["mark"] = tmpLanes.at(j)->getRightBoundary()->getLaneMark();
          rightBoudaryJson["id"] = FormatIntValue(tmpLanes.at(j)->getRightBoundary()->getId());

          // Add the right boundary to the JSON section if it's not a duplicate
          if (boundaryIdset.find(tmpLanes.at(j)->getRightBoundary()->getId()) == boundaryIdset.end()) {
            jsonSection["boundarys"].append(rightBoudaryJson);
            boundaryIdset.insert(tmpLanes.at(j)->getRightBoundary()->getId());
          }

          jsonSection["lanes"].append(laneJson);
        }

        jsonRoad["sections"].append(jsonSection);
      }

      // Convert the control points to JSON
      hadmap::txControlPoint controlPoint = it->getControlPoint();
      jsonRoad["controlType"] = controlPoint.m_type;
      for (auto itPt : controlPoint.m_points) {
        Json::Value pt1;
        pt1["x"] = FormatDoubleValue(itPt.x);
        pt1["y"] = FormatDoubleValue(itPt.y);
        pt1["z"] = FormatDoubleValue(itPt.z);
        pt1["hdg"] = std::to_string(itPt.hdg);
        jsonRoad["controlPoints"].append(pt1);
      }

      // Convert the elevation points to JSON
      hadmap::txControlPoint eleControlPoint = it->getElePoint();
      if (eleControlPoint.m_points.size() > 0) {
        for (auto itPt : eleControlPoint.m_points) {
          Json::Value pt1;
          pt1["s"] = itPt.x;
          pt1["h"] = itPt.y;
          pt1["slope"] = std::atan(itPt.h);
          jsonRoad["elevation"].append(pt1);
        }
      }

      // Convert the opencrg data to JSON
      hadmap::txOpenCrgVec _crgs = it->getCrgVec();
      for (auto itCrg : _crgs) {
        Json::Value crg;
        crg["file"] = itCrg.m_file;
        crg["orientation"] = itCrg.m_orientation;
        crg["mode"] = itCrg.m_mode;
        crg["purpose"] = itCrg.m_purpose;
        crg["sOffset"] = itCrg.m_soffset;
        crg["tOffset"] = itCrg.m_toffset;
        crg["zOffset"] = itCrg.m_zoffset;
        crg["zScale"] = itCrg.m_zscale;
        crg["hOffset"] = itCrg.m_hoffset;
        jsonRoad["opencrgs"].append(crg);
      }
    } else {
      continue;
    }

    json_value["roads"].append(jsonRoad);
  }
}

void CHadmap::LaneLinkToJson(const hadmap::txRoads roads, const hadmap::txLaneLinks links, Json::Value& json_value) {
  // Iterate through all the lane links in the provided 'links' object.
  for (auto it : links) {
    // Create a JSON object 'link_json' to store the lane link's attributes.
    Json::Value link_json;
    if (FormatIntValue(it->getJunctionId()) == "-1") continue;
    // Populate the 'link_json' object with the link's attributes.
    link_json["fid"] = FormatIntValue(it->fromLaneId());
    link_json["frid"] = FormatIntValue(it->fromRoadId());
    link_json["fsid"] = FormatIntValue(it->fromSectionId());
    link_json["id"] = FormatIntValue(it->getId());
    link_json["junctionid"] = FormatIntValue(it->getJunctionId());
    link_json["length"] = it->getGeometry()->getLength();
    link_json["tid"] = FormatIntValue(it->toLaneId());
    link_json["trid"] = FormatIntValue(it->toRoadId());
    link_json["tsid"] = FormatIntValue(it->toSectionId());
    link_json["roadid"] = FormatIntValue(it->getOdrRoadId());

    // Convert the lane link's curve geometry into a series of points and store them in 'laneLinkPoints'.
    const hadmap::txCurve* laneLinkCurve = it->getGeometry();
    hadmap::PointVec laneLinkPoints;
    CurveToPoints(laneLinkCurve, laneLinkPoints);

    // Iterate through the 'laneLinkPoints' and add each point's coordinates (x, y, z) to the 'link_json' object.
    for (size_t m = 0; m < laneLinkPoints.size(); m++) {
      Json::Value jsonPoint;
      jsonPoint["x"] = laneLinkPoints.at(m).x;
      jsonPoint["y"] = laneLinkPoints.at(m).y;
      jsonPoint["z"] = laneLinkPoints.at(m).z;
      link_json["samplePoints"].append(jsonPoint);
    }
    // get start or end;
    if (m_dataType != hadmap::MAP_DATA_TYPE::SQLITE) {
      hadmap::txContactType preContact = it->getPreContact();
      hadmap::txContactType succContact = it->getSuccContact();
      if (preContact == hadmap::txContactType::START) {
        link_json["ftype"] = "start";
      } else {
        link_json["ftype"] = "end";
      }
      if (succContact == hadmap::txContactType::START) {
        link_json["ttype"] = "start";
      } else {
        link_json["ttype"] = "end";
      }
    } else {
      link_json["ftype"] = "end";
      link_json["ttype"] = "start";
    }

    // Get the control points and store them in the 'link_json' object.
    hadmap::txControlPoint controlPoint = it->getControlPoint();
    link_json["controlType"] = controlPoint.m_type;
    for (auto itPt : controlPoint.m_points) {
      Json::Value pt1;
      pt1["x"] = itPt.x;
      pt1["y"] = itPt.y;
      pt1["z"] = itPt.z;
      pt1["hdg"] = std::to_string(itPt.hdg);
      link_json["controlPoints"].append(pt1);
    }

    // Get the elevation points and store them in the 'link_json' object.
    hadmap::txControlPoint eleControlPoint = it->getEleControlPoint();
    if (eleControlPoint.m_points.size() > 0) {
      for (auto itPt : eleControlPoint.m_points) {
        Json::Value pt1;
        pt1["s"] = itPt.x;
        pt1["h"] = itPt.y;
        pt1["slope"] = std::atan(itPt.h);
        link_json["elevation"].append(pt1);
      }
    }
    json_value["lanelinks"].append(link_json);
  }
}

void CHadmap::ObjectsToJson(const hadmap::txObjects& objects, Json::Value& json_value) {
  for (auto it : objects) {
    Json::Value object_json;
    hadmap::tx_od_object_t tmp = it->getOdData();
    std::string _name = it->getName();
    std::string _type;
    std::string _subtype;
    std::string _aname;
    it->getObjectFromatType(_aname, _type, _subtype);
    object_json["type"] = _type;
    object_json["subtype"] = _subtype;
    object_json["name"] = _aname;
    object_json["id"] = std::to_string(tmp.object_pkid);
    object_json["zOffset"] = std::to_string(tmp.zoffset);
    object_json["validlength"] = std::to_string(tmp.validlength);
    if (tmp.orientation > 0) {
      object_json["orientation"] = "+";
    } else {
      object_json["orientation"] = "-";
    }
    object_json["radius"] = std::to_string(tmp.radius);
    object_json["length"] = std::to_string(tmp.length);
    object_json["width"] = std::to_string(tmp.width);
    object_json["height"] = std::to_string(tmp.height);
    object_json["hdg"] = std::to_string(tmp.hdg);
    object_json["pitch"] = std::to_string(tmp.pitch);
    object_json["roll"] = std::to_string(tmp.roll);
    object_json["roadid"] = std::to_string(tmp.roadid);
    object_json["lanelinkid"] = std::to_string(tmp.lanelinkid);
    object_json["s"] = std::to_string(tmp.s);
    object_json["t"] = std::to_string(tmp.t);
    if (tmp.lanelinkid > 0) {
      hadmap::txLaneLinkPtr lanelinkPtr = NULL;
      hadmap::getLaneLink(this->m_pMapHandler, tmp.lanelinkid, lanelinkPtr);
      if (lanelinkPtr && lanelinkPtr->getGeometry()) {
        double s = 0.0;
        double t = 0.0;
        double yaw = 0.0;
        lanelinkPtr->getGeometry()->xy2sl(it->getPos().x, it->getPos().y, s, t, yaw);
        object_json["s"] = std::to_string(s);
        object_json["t"] = std::to_string(t);
      }
    }
    XY2LonLat(tmp.s, tmp.t);
    hadmap::txPoint point(tmp.s, tmp.t, 0);
    hadmap::txLanePtr lanePtr;
    hadmap::txLaneLinkPtr laneLinkPtr;
    double radios = 20;
    std::map<std::string, std::string> _userData;
    it->getUserData(_userData);
    std::string origialName = it->getName();
    int apos = origialName.find("-");
    if (apos < origialName.size()) {
      std::string key = origialName.substr(0, apos);
      std::string value = origialName.substr(apos + 1, origialName.size() - apos - 1);
      if (key == "pole" || key == "sensor") {
        _userData.insert(std::make_pair("pole_id", value));
      }
    }
    for (auto itData : _userData) {
      Json::Value _jsonItData;
      _jsonItData["code"] = itData.first;
      _jsonItData["value"] = itData.second;
      object_json["userdata"].append(_jsonItData);
    }
    std::map<std::string, std::string> repeatdataVec = SplitToMap(tmp.repeatdata, ";", ":");
    if (repeatdataVec.size() > 0) {
      Json::Value repeat;
      repeat["s"] = repeatdataVec["s"];
      repeat["length"] = repeatdataVec["length"];
      repeat["distance"] = repeatdataVec["distance"];
      repeat["tStart"] = repeatdataVec["tStart"];
      repeat["tEnd"] = repeatdataVec["tEnd"];
      repeat["widthStart"] = repeatdataVec["widthStart"];
      repeat["widthEnd"] = repeatdataVec["widthEnd"];
      repeat["heightStart"] = repeatdataVec["heightStart"];
      repeat["zOffsetStart"] = repeatdataVec["zOffsetStart"];
      repeat["zOffsetEnd"] = repeatdataVec["zOffsetEnd"];
      repeat["lengthStart"] = repeatdataVec["lengthStart"];
      repeat["lengthEnd"] = repeatdataVec["lengthEnd"];
      object_json["repeat"] = repeat;
    }
    if (_type == std::string("parkingSpace")) {
      Json::Value Markings;
      std::string strcolor;
      if (it->getGeom()->getColor() == hadmap::OBJECT_COLOR_White) {
        strcolor = "white";
      } else if (it->getGeom()->getColor() == hadmap::OBJECT_COLOR_Yellow) {
        strcolor = "yellow";
      } else if (it->getGeom()->getColor() == hadmap::OBJECT_COLOR_Red) {
        strcolor = "red";
      } else if (it->getGeom()->getColor() == hadmap::OBJECT_COLOR_Blue) {
        strcolor = "blue";
      } else if (it->getGeom()->getColor() == hadmap::OBJECT_COLOR_Green) {
        strcolor = "green";
      }
      Markings["width"] = std::to_string(tmp.markWidth);
      Markings["color"] = strcolor;
      Markings["cornerreferenceid"] = "0_1";
      object_json["markings"].append(Markings);
    }
    json_value["objects"].append(object_json);
  }
}

void CHadmap::CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points) {
  // Check if the provided curve object is a null pointer.
  if (Null_Pointer(curve)) {
    return;
  }

  // Calculate the length of the curve.
  double fLen = curve->getLength();

  // Determine the sampling interval based on the length of the curve.
  double fInterval = 10;
  if (fLen < 50) {
    fInterval = fLen / 10;
  } else if (fLen >= 50 && fLen < 100) {
    fInterval = fLen / 20;
  } else if (fLen >= 100 && fLen < 300) {
    fInterval = fLen / 30;
  }

  // If the curve length is less than 0.1, set the sampling interval to 1.
  if (fLen < 0.1) {
    fInterval = 1;
  }

  // Sample the curve at the specified interval and store the resulting points in the 'pts' vector.
  hadmap::PointVec pts;
  curve->sample(fInterval, pts);

  // transform the points from the original coordinate system to the XY coordinate system.
  if (pts.size() > 0) {
    hadmap::PointVec::iterator itr = pts.begin();
    for (; itr != pts.end(); ++itr) {
      double x = itr->x;
      double y = itr->y;
      double z = itr->z;
      itr->x = x;
      itr->y = y;
      itr->z = z;
      points.push_back(*itr);
    }
    // Convert the points from the lonlat coordinate system to the XY coordinate system.
    LonLatToXY(points);
  }
  for (auto& it : points) {
    it.x -= m_refPoint.x;
    it.y -= m_refPoint.y;
  }
  return;
}

void CHadmap::CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points, int nPointsSize) {
  // Check if the provided curve object is a null pointer.
  if (Null_Pointer(curve)) {
    return;
  }

  // Calculate the length of the curve.
  double fLen = curve->getLength();

  // Calculate the sampling interval based on the desired number of points.
  double fInterval = fLen / nPointsSize;

  // Sample the curve at the specified interval and store the resulting points in the 'pts' vector.
  hadmap::PointVec pts;
  curve->sample(fInterval, pts);

  //  transform the points from the original coordinate system to the ENUcoordinate system.
  if (pts.size() > 0) {
    hadmap::PointVec::iterator itr = pts.begin();
    for (; itr != pts.end(); ++itr) {
      double x = itr->x;
      double y = itr->y;
      double z = itr->z;
      itr->x = x;
      itr->y = y;
      itr->z = z;
      points.push_back(*itr);
    }
    // Convert the points from the lonlat coordinate system to the XY coordinate system.
    LonLatToXY(points);
  }
  for (auto& it : points) {
    it.x -= m_refPoint.x;
    it.y -= m_refPoint.y;
  }
  return;
}

void CHadmap::CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points, const double start_s,
                            const double end_s, bool isEnd) {
  points.clear();
  if (Null_Pointer(curve)) {
    return;
  }
  double fLen = end_s - start_s;

  double fInterval = 10;
  if (fLen < 50) {
    fInterval = fLen / 10;
  } else if (fLen >= 50 && fLen < 100) {
    fInterval = fLen / 20;
  } else if (fLen >= 100 && fLen < 300) {
    fInterval = fLen / 30;
  }
  if (fLen < 0.01) {
    fInterval = 1;
  }
  if (fLen >= 0.0) fLen += fInterval;
  for (int i = 0; i * fInterval < fLen; ++i) {
    hadmap::txPoint pt = curve->getPoint(start_s + i * fInterval);
    double x = pt.x;
    double y = pt.y;
    double z = pt.z;
    pt.x = x;
    pt.y = y;
    pt.z = z;
    points.push_back(pt);
  }
  if (curve->getCoordType() == hadmap::COORD_WGS84) {
    LonLatToXY(points);
  }
  for (auto& it : points) {
    it.x -= m_refPoint.x;
    it.y -= m_refPoint.y;
  }
  return;
}

int CHadmap::JsonToMap(Json::Value& json_root) {
  // Check if the JSON object contains the "roads" key.
  if (json_root["roads"].empty()) {
    return -1;
  }

  // Parse the roads from the JSON object and store them in the 'map' object.
  hadmap::txRoads roads;
  Id2RoadMap map;
  ParseRoads(json_root["roads"], map);
  for (auto it : map) {
    roads.push_back(it.second);
  }

  // Insert the roads into the HD Map.
  int nret = hadmap::insertRoads(m_pMapHandler, roads);

  // Check if the road insertion was successful.
  if (nret != 0) {
    LogWarn << "insert roads error";
    return nret;
  }

  // Add the lane links from the 'mapLinks' object to the 'laneLinks' vector.
  hadmap::txLaneLinks laneLinks;
  Id2LaneLinkMap mapLinks;
  ParseLanelinks(json_root["lanelinks"], mapLinks);
  for (auto it : mapLinks) {
    laneLinks.push_back(it.second);
  }

  // Adjust the lane link road IDs if necessary.
  std::set<int> lanelinkRoadId;
  for (auto& itL : mapLinks) {
    int odrRoadId = itL.second->getOdrRoadId();
    if (lanelinkRoadId.find(odrRoadId) != lanelinkRoadId.end()) {
      itL.second->setOdrRoadId((*lanelinkRoadId.rbegin()) + 1);
    }
    lanelinkRoadId.insert(itL.second->getOdrRoadId());
  }

  // Insert the lane links into the HD Map.
  nret = hadmap::insertLaneLinks(m_pMapHandler, laneLinks);

  // Add the objects from the 'mapObjects' object to the 'objects' vector.
  hadmap::txObjects objects;
  Id2Objects mapObjects;
  ParseObjects(json_root["objects"], mapObjects);
  for (auto it : mapObjects) {
    objects.push_back(it.second);
  }

  // Insert the objects into the HD Map.
  nret = hadmap::insertObjects(m_pMapHandler, objects);

  return hadmap::hadmapSave(m_pMapHandler);
}

int CHadmap::ParseRoads(Json::Value& roads, Id2RoadMap& map) {
  // Iterate through the roads in the JSON object.
  for (auto it : roads) {
    // Create a shared pointer for the road.
    hadmap::txRoadPtr roadPtr = std::make_shared<hadmap::txRoad>();

    // Set the road attributes from the JSON object.
    roadPtr->setId(atoi(it["id"].asString().c_str()));
    roadPtr->setRoadType((hadmap::ROAD_TYPE)it["type"].asInt());

    // Parse the sample points for the road.
    hadmap::PointVec points;
    for (auto point : it["samplePoints"]) {
      hadmap::txPoint tPoint;
      double x = point["x"].asDouble();
      double y = point["y"].asDouble();
      double z = point["z"].asDouble();
      XY2LonLat(x, y);
      tPoint.x = x;
      tPoint.y = y;
      tPoint.z = z;
      points.push_back(tPoint);
    }

    // Parse the sections for the road.
    hadmap::txSections sections;
    for (auto section : it["sections"]) {
      hadmap::txSectionPtr sectionPtr = std::make_shared<hadmap::txSection>();
      sectionPtr->setId(atoi(section["id"].asString().c_str()));

      // Parse the boundary for the section.
      std::map<int, hadmap::txLaneBoundaryPtr> id2BoundaryMap;
      hadmap::txLaneBoundaries boundaries;
      for (auto laneBoudary : section["boundarys"]) {
        hadmap::txLaneBoundaryPtr boundaryPtr = std::make_shared<hadmap::txLaneBoundary>();
        boundaryPtr->setId(atoi(laneBoudary["id"].asString().c_str()));
        boundaryPtr->setLaneMark(hadmap::LANE_MARK(laneBoudary["mark"].asInt()));

        // Parse the sample points for the boundary.
        hadmap::PointVec points;
        for (auto point : laneBoudary["samplePoints"]) {
          hadmap::txPoint tPoint;
          double x = point["x"].asDouble();
          double y = point["y"].asDouble();
          double z = point["z"].asDouble();
          XY2LonLat(x, y);
          tPoint.x = x;
          tPoint.y = y;
          tPoint.z = z;

          points.push_back(tPoint);
        }
        boundaryPtr->setGeometry(points, hadmap::CoordType::COORD_WGS84);
        id2BoundaryMap[boundaryPtr->getId()] = boundaryPtr;
      }

      // Parse the lanes for the section.
      hadmap::txLanes lanes;
      for (auto lane : section["lanes"]) {
        hadmap::txLanePtr lanePtr = std::make_shared<hadmap::txLane>();
        lanePtr->setId(atoi(lane["id"].asString().c_str()));
        lanePtr->setRoadId(roadPtr->getId());
        lanePtr->setSectionId(sectionPtr->getId());
        lanePtr->setLeftBoundaryId(atoi(lane["lbid"].asString().c_str()));
        lanePtr->setLeftBoundary(id2BoundaryMap[atoi(lane["lbid"].asString().c_str())]);
        lanePtr->setRightBoundaryId(atoi(lane["rbid"].asString().c_str()));
        lanePtr->setRightBoundary(id2BoundaryMap[lanePtr->getRightBoundaryId()]);
        lanePtr->setLaneType(hadmap::LANE_TYPE(lane["type"].asInt()));
        lanePtr->setSpeedLimit(lane["speedlimit"].asInt());
        lanePtr->setLaneFriction(lane.get("friction", "0").asDouble());
        if (lane.isMember("sOffset") && lane["sOffset"].isInt())
          lanePtr->setMaterialOffset(lane.get("sOffset", "0").asInt());
        else if (lane.isMember("sOffset") && lane["sOffset"].isDouble())
          lanePtr->setMaterialOffset(lane.get("sOffset", "0").asDouble());
        // Parse the sample points for the lane.
        hadmap::PointVec points;
        for (auto point : lane["samplePoints"]) {
          hadmap::txPoint tPoint;
          double x = point["x"].asDouble();
          double y = point["y"].asDouble();
          double z = point["z"].asDouble();
          XY2LonLat(x, y);
          tPoint.x = x;
          tPoint.y = y;
          tPoint.z = z;
          points.push_back(tPoint);
        }
        lanePtr->setGeometry(points, hadmap::CoordType::COORD_WGS84);
        lanes.push_back(lanePtr);
      }
      sectionPtr->setLanes(lanes);
      sections.push_back(sectionPtr);
    }

    // Set the geometry and sections for the road.
    roadPtr->setGeometry(points, hadmap::CoordType::COORD_WGS84);
    roadPtr->setSections(sections);

    // Parse the control points for the road.
    hadmap::txControlPoint _cpoint;
    _cpoint.m_type = it.get("controlType", "none").asString();
    for (auto itPt : it["controlPoints"]) {
      hadmap::txPoint pt;
      pt.x = itPt.get("x", "0").asDouble();
      pt.y = itPt.get("y", "0").asDouble();
      pt.z = itPt.get("z", "0").asDouble();
      pt.hdg = std::atof(itPt.get("hdg", "0").asString().c_str());
      _cpoint.m_points.push_back(pt);
    }
    roadPtr->setControlPoint(_cpoint);

    // Parse the opencrgs for the road.
    hadmap::txOpenCrgVec _crgs;
    for (auto itCrg : it["opencrgs"]) {
      hadmap::txOpenCrg crg;
      crg.m_file = itCrg.get("file", "").asString();
      crg.m_orientation = itCrg.get("orientation", "").asString();
      crg.m_mode = itCrg.get("mode", "").asString();
      crg.m_purpose = itCrg.get("purpose", "").asString();
      crg.m_soffset = itCrg.get("sOffset", "").asString();
      crg.m_toffset = itCrg.get("tOffset", "").asString();
      crg.m_zoffset = itCrg.get("zOffset", "").asString();
      crg.m_zscale = itCrg.get("zScale", "").asString();
      crg.m_hoffset = itCrg.get("hOffset", "").asString();
      _crgs.push_back(crg);
    }
    roadPtr->setCrgs(_crgs);

    // Parse the elevation points for the road.
    if (it["elevation"]) {
      hadmap::txControlPoint _elePoints;
      for (auto itPt : it["elevation"]) {
        hadmap::txPoint pt;
        pt.x = itPt.get("s", "0").asDouble();
        pt.y = itPt.get("h", "0").asDouble();
        _elePoints.m_points.push_back(pt);
      }
      if (_elePoints.m_points.size() > 1) {
        _elePoints.m_type = "catmullrom";
      }
      roadPtr->setElePoint(_elePoints);
    }

    // Add the road to the map.
    map[it["id"].asString()] = roadPtr;
  }
  return 0;
}

int CHadmap::ParseLanelinks(Json::Value& lanelinks, Id2LaneLinkMap& map) {
  hadmap::txPoint pointref = hadmap::getRefPoint(m_pMapHandler);
  for (auto it : lanelinks) {
    hadmap::txLaneLinkPtr lanelink = std::make_shared<hadmap::txLaneLink>();
    lanelink->setId(atoi(it["id"].asString().c_str()));
    lanelink->setFromRoadId(atoi(it["frid"].asString().c_str()));
    lanelink->setFromSectionId(atoi(it["fsid"].asString().c_str()));
    lanelink->setFromLaneId(atoi(it["fid"].asString().c_str()));
    lanelink->setToRoadId(atoi(it["trid"].asString().c_str()));
    lanelink->setToSectionId(atoi(it["tsid"].asString().c_str()));
    lanelink->setToLaneId(atoi(it["tid"].asString().c_str()));
    lanelink->setJunctionId(atoi(it["junctionid"].asString().c_str()));
    lanelink->setOdrRoadId(atoi(it["roadid"].asString().c_str()));
    if (it["ftype"].asString() == std::string("start")) {
      lanelink->setPreContact(hadmap::txContactType::START);
    } else {
      lanelink->setPreContact(hadmap::txContactType::END);
    }
    if (it["ttype"].asString() == std::string("start")) {
      lanelink->setSuccContact(hadmap::txContactType::START);
    } else {
      lanelink->setSuccContact(hadmap::txContactType::END);
    }
    // Parse the samplePoints.
    hadmap::PointVec points;
    for (auto point : it["samplePoints"]) {
      hadmap::txPoint tPoint;
      double x = point["x"].asDouble();
      double y = point["y"].asDouble();
      double z = point["z"].asDouble();
      XY2LonLat(x, y);
      tPoint.x = x;
      tPoint.y = y;
      tPoint.z = z;
      points.push_back(tPoint);
    }
    lanelink->setGeometry(points, hadmap::CoordType::COORD_WGS84);
    // Parse the controlPoints.
    hadmap::txControlPoint _cpoint;
    _cpoint.m_type = it.get("controlType", "none").asString();
    for (auto itPt : it["controlPoints"]) {
      hadmap::txPoint pt;
      pt.x = itPt.get("x", "0").asDouble();
      pt.y = itPt.get("y", "0").asDouble();
      pt.z = itPt.get("z", "0").asDouble();
      pt.hdg = std::atof(itPt.get("hdg", "0").asString().c_str());
      _cpoint.m_points.push_back(pt);
    }
    lanelink->setControlPoint(_cpoint);
    // Parse the elevation.
    if (it["elevation"]) {
      hadmap::txControlPoint _elePoints;
      for (auto itPt : it["elevation"]) {
        hadmap::txPoint pt;
        pt.x = itPt.get("s", "0").asDouble();
        pt.y = itPt.get("h", "0").asDouble();
        _elePoints.m_points.push_back(pt);
      }
      if (_elePoints.m_points.size() > 1) {
        _elePoints.m_type = "catmullrom";
      }
      lanelink->setEleControlPoint(_elePoints);
    }

    map[it["id"].asString()] = lanelink;
  }
  return 0;
}

int CHadmap::ParseObjects(Json::Value& objects, Id2Objects& map) {
  // Define a lambda function to convert string color to OBJECT_COLOR enum
  auto str_to_color = [&](const std::string& strcolor) {
    auto iter = CHadmap::kObjectStrColorDict.find(strcolor);
    return iter != CHadmap::kObjectStrColorDict.end() ? iter->second : hadmap::OBJECT_COLOR_White;
  };

  // Iterate through all objects in the input JSON
  for (auto it : objects) {
    // Create a new txObjectPtr instance
    hadmap::txObjectPtr pObject(new hadmap::txObject);
    double lon = 0.0;
    double lat = 0.0;
    double yaw = 0.0;
    double height = 0.0;
    int roadid = atoi(it["roadid"].asString().c_str());
    int lanelinkid = atoi(it["lanelinkid"].asString().c_str());
    double s = atof(it["s"].asString().c_str());
    double t = atof(it["t"].asString().c_str());

    // If roadid is -1, get lanelink
    if (lanelinkid != -1) {
      hadmap::lanelinkpkid lanelinkPid = atoi(it["lanelinkid"].asString().c_str());
      hadmap::txLaneLinkPtr laneLinkPtr;
      hadmap::getLaneLink(m_pMapHandler, lanelinkPid, laneLinkPtr);
      if (laneLinkPtr) {
        laneLinkPtr->getGeometry()->sl2xy(s, t, lon, lat, yaw);
        hadmap::txPoint point = laneLinkPtr->getGeometry()->getPoint(s);
        height = point.z;
      }
    } else {
      hadmap::roadpkid _roadId = roadid;
      hadmap::txRoadPtr txRoad;
      hadmap::getRoad(m_pMapHandler, _roadId, false, txRoad);
      if (txRoad) {
        txRoad->getGeometry()->sl2xy(s, t, lon, lat, yaw);
        hadmap::txPoint point = txRoad->getGeometry()->getPoint(s);
        height = point.z;
      }
    }

    // Set object data
    hadmap::tx_od_object_t od_road = pObject->getOdData();
    od_road.pkid = std::atoi(it["id"].asString().c_str());
    od_road.s = atof(it["s"].asString().c_str());
    od_road.t = atof(it["t"].asString().c_str());
    od_road.orientation = (std::string(it["orientation"].asString()) == "-" ? 0 : 1);
    od_road.zoffset = atof(it["zOffset"].asString().c_str());
    od_road.hdg = atof(it["hdg"].asString().c_str());
    strcpy(od_road.name, it["name"].asString().c_str());
    od_road.lanelinkid = atoi(it["lanelinkid"].asString().c_str());
    std::string strtype = std::string(it["type"].asString());

    // If object type is "parkingSpace"
    if (strtype == "parkingSpace") {
      hadmap::PointVec curve;
      std::string strcolor = it["markings"].begin()->get("color", "white").asString();
      hadmap::OBJECT_GEOMETRY_TYPE type(hadmap::OBJECT_GEOMETRY_TYPE_Polygon);
      hadmap::OBJECT_STYLE style(hadmap::OBJECT_STYLE_None);
      hadmap::OBJECT_COLOR color = str_to_color(strcolor);

      int index = 1;
      for (auto& itPoint : it["outlines"]) {
        double s = std::atof(itPoint.get("s", 0.0).asString().c_str());
        double t = std::atof(itPoint.get("t", 0.0).asString().c_str());
        double lon = 0.0;
        double lat = 0.0;
        RoadSt2XY(roadid, od_road.lanelinkid, s, t, lon, lat);
        curve.push_back(hadmap::txPoint(lon, lat, 0.0));
        if (++index % 4 == 0) {
          // Get color
          hadmap::txObjGeomPtr geomPtr(new hadmap::txObjectGeom);
          geomPtr->setGeometry(curve, hadmap::COORD_WGS84);
          geomPtr->setStyle(style);
          geomPtr->setColor(color);
          geomPtr->setType(type);
          pObject->addGeom(geomPtr);
          curve.clear();
          LogInfo << "strcolor: " << strcolor << ", color: " << color;
        }
      }
    } else {
      hadmap::PointVec curve;
      curve.push_back(hadmap::txPoint(lon, lat, height));
      hadmap::OBJECT_GEOMETRY_TYPE type(hadmap::OBJECT_GEOMETRY_TYPE_Point);
      hadmap::OBJECT_STYLE style(hadmap::OBJECT_STYLE_None);
      hadmap::OBJECT_COLOR color(hadmap::OBJECT_COLOR_White);
      hadmap::txObjGeomPtr geomPtr(new hadmap::txObjectGeom);
      geomPtr->setGeometry(curve, hadmap::COORD_WGS84);
      geomPtr->setStyle(style);
      geomPtr->setColor(color);
      geomPtr->setType(type);
      pObject->addGeom(geomPtr);
    }

    // Set marking width
    if (it["markings"]) {
      od_road.markWidth = std::atof(it["markings"].begin()->get("width", "0.1").asString().c_str());
    }

    // Set position
    pObject->setPos(lon, lat, height);
    LonLatToXY(lon, lat);

    // If "repeat" data exists
    if (it["repeat"]) {
      std::string _tmpData;
      _tmpData += ("s:" + it["repeat"]["s"].asString() + ";");
      _tmpData += ("length:" + it["repeat"]["length"].asString() + ";");
      _tmpData += ("distance:" + it["repeat"]["distance"].asString() + ";");
      _tmpData += ("tStart:" + it["repeat"]["tStart"].asString() + ";");
      _tmpData += ("tEnd:" + it["repeat"]["tEnd"].asString() + ";");
      _tmpData += ("widthStart:" + it["repeat"]["widthStart"].asString() + ";");
      _tmpData += ("widthEnd:" + it["repeat"]["widthEnd"].asString() + ";");
      _tmpData += ("heightStart:" + it["repeat"]["heightStart"].asString() + ";");
      _tmpData += ("heightEnd:" + it["repeat"]["heightEnd"].asString() + ";");
      _tmpData += ("zOffsetStart:" + it["repeat"]["zOffsetStart"].asString() + ";");
      _tmpData += ("zOffsetEnd:" + it["repeat"]["zOffsetEnd"].asString() + ";");
      _tmpData += ("lengthStart:" + it["repeat"]["lengthStart"].asString() + ";");
      _tmpData += ("lengthEnd:" + it["repeat"]["lengthEnd"].asString() + ";");
      memcpy(od_road.repeatdata, _tmpData.c_str(), strlen(_tmpData.c_str()) + 1);
    }

    // If "outlines" data exists
    if (it["outlines"]) {
      std::string _tmpData;
      for (auto itPoint : it["outlines"]) {
        _tmpData += (itPoint.get("s", 0.0).asString());
        _tmpData += ",";
        _tmpData += (itPoint.get("t", 0.0).asString());
        _tmpData += ";";
      }
      memcpy(od_road.outlines, _tmpData.c_str(), strlen(_tmpData.c_str()) + 1);
    }

    // Set txOdData
    pObject->setTxOdData(od_road);

    // If "userdata" exists
    if (it["userdata"]) {
      std::map<std::string, std::string> _userData;
      for (auto itJson : it["userdata"]) {
        std::string key = itJson.get("code", "").asString();
        std::string value = itJson.get("value", "").asString();
        if (!key.empty()) {
          _userData[key] = value;
        }
      }
      pObject->setUserData(_userData);
    }

    // Set object ID, type, and dimensions
    pObject->setId(atoi(it["id"].asString().c_str()));
    pObject->setObjectType(it["name"].asString(), it["type"].asString(), it["subtype"].asString());
    pObject->setLWH(atof(it["length"].asString().c_str()), atof(it["width"].asString().c_str()),
                    atof(it["height"].asString().c_str()));
    pObject->setRawTypeString(it["type"].asString(), it["subtype"].asString());

    // Set odRoadId
    pObject->setOdRoadId(roadid);

    // Transform object data to odData
    pObject->transform2OdData();

    // Add object to map
    std::string id = it["id"].asString();
    map[id] = pObject;
  }

  return 0;
}

bool CHadmap::MakeInitXodr(const char* filePath, std::string maptype, std::string mapversion) {
  // Initialize a temporary string with the predefined XODR template
  std::string tmp = kXodrTpl;

  // Replace the revision minor placeholder in the template based on the map type
  if (maptype == "xodr 1.5") {
    ReplaceStr(tmp, kRevMinorPlaceholder, "5");
  } else {
    ReplaceStr(tmp, kRevMinorPlaceholder, "4");
  }

  // Replace the version placeholder in the template based on the map version
  if (mapversion == "tadsim v1.0") {
    ReplaceStr(tmp, kVersionPlaceholder, "tadsim v1.0");
  } else {
    ReplaceStr(tmp, kVersionPlaceholder, "tadsim v2.0");
  }

  // Get the current time and format it as a string to replace the date placeholder in the template
  char buf[64] = {0};
  std::time_t now = std::time(nullptr);
  std::size_t size = std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&now));
  ReplaceStr(tmp, kDatePlaceholder, std::string(buf, size));

  // Create an output file stream object and open the file specified by the file path
  std::ofstream ofs;
  ofs.open(filePath, std::ios::out);

  // Write the temporary string to the file and close it
  ofs << tmp.c_str();
  ofs.close();

  return true;
}

int CHadmap::GetStartAndEndType(const hadmap::txPoint& point, const hadmap::txLanePtr& ptr) {
  const hadmap::txCurve* curvePtr = ptr->getGeometry();
  if (Null_Pointer(curvePtr)) return -1;
  hadmap::txPoint startPoint = curvePtr->getStart();
  hadmap::txPoint endPoint = curvePtr->getEnd();
  // compare distance
  double distanceStart =
      (point.x - startPoint.x) * (point.x - startPoint.x) + (point.y - startPoint.y) * (point.y - startPoint.y);
  double distanceEnd =
      (point.x - endPoint.x) * (point.x - endPoint.x) + (point.y - endPoint.y) * (point.y - endPoint.y);
  if (distanceStart > distanceEnd)
    return 1;  // end
  else
    return 0;  // start
}

void CHadmap::RoadSt2XY(int roadid, int lanelinkid, double s, double t, double& lon, double& lat) {
  double yaw = 0;
  // Check if the lanelinkid is not -1, indicating that the conversion is for a lane link
  if (lanelinkid != -1) {
    hadmap::lanelinkpkid lanelinkPid = lanelinkid;
    hadmap::txLaneLinkPtr laneLinkPtr;
    hadmap::getLaneLink(m_pMapHandler, lanelinkPid, laneLinkPtr);
    // If laneLinkPtr is not empty, convert the s and t coordinates to longitude and latitude coordinates
    if (laneLinkPtr) {
      laneLinkPtr->getGeometry()->sl2xy(s, t, lon, lat, yaw);
      hadmap::txPoint point = laneLinkPtr->getGeometry()->getPoint(s);
    }
  } else {
    hadmap::roadpkid _roadId = roadid;
    hadmap::txRoadPtr txRoad;
    hadmap::getRoad(m_pMapHandler, _roadId, false, txRoad);
    if (txRoad) {
      txRoad->getGeometry()->sl2xy(s, t, lon, lat, yaw);
      hadmap::txPoint point = txRoad->getGeometry()->getPoint(s);
    }
  }
}

void CHadmap::LonLatToXY(std::vector<hadmap::txPoint>& points) {
  hadmap::txOdHeaderPtr headerPtr;
  if (m_pMapHandler) {
    hadmap::getHeader(m_pMapHandler, headerPtr);
    if (headerPtr) {
      double sourth = headerPtr->getSouth();
      double west = headerPtr->getWest();
      std::string georeference = headerPtr->getGeoReference();
#ifdef PROJ_NEW_API
      PJ_CONTEXT* C = proj_context_create();
      PJ* P = proj_create_crs_to_crs(C, "+proj=longlat +datum=WGS84 +no_defs", georeference.c_str(), NULL);
      if (P) {
        for (auto& p : points) {
          PJ_COORD a = proj_coord(p.x, p.y, p.z, 0);
          PJ_COORD b = proj_trans(P, PJ_FWD, a);
          p.x = b.xyz.x;
          p.y = b.xyz.y;
          p.x -= west;
          p.y -= sourth;
          p.x -= m_refPoint.x;
          p.y -= m_refPoint.y;
        }
        return;
      } else {
        for (auto& it : points) {
          coord_trans_api::lonlat2mercator(it.x, it.y);
          it.x -= west;
          it.y -= sourth;
          it.x -= m_refPoint.x;
          it.y -= m_refPoint.y;
        }
        return;
      }
#else
      projPJ pj_src = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs");
      projPJ pj_dst = pj_init_plus(georeference.c_str());
      int off = 1;

      if (pj_src && pj_dst) {
        int off = 1;
        if (points.size() > 1) {
          off = (&points[1].x) - (&points[0].x);
        }
        for (auto& p : points) {
          p.x /= RAD_TO_DEG;
          p.y /= RAD_TO_DEG;
        }
        int code = pj_transform(pj_src, pj_dst, points.size(), off, &points[0].x, &points[0].y, 0);
        for (auto& p : points) {
          p.x -= west;
          p.y -= sourth;
          p.x -= m_refPoint.x;
          p.y -= m_refPoint.y;
        }
        return;
      } else {
        for (auto& it : points) {
          coord_trans_api::lonlat2mercator(it.x, it.y);
          it.y -= sourth;
          it.x -= west;
          it.x -= m_refPoint.x;
          it.y -= m_refPoint.y;
        }
        return;
      }
#endif
    }
  }
  for (auto& it : points) {
    coord_trans_api::lonlat2mercator(it.x, it.y);
    it.x -= m_refPoint.x;
    it.y -= m_refPoint.y;
  }

  return;
}

void CHadmap::LonLatToXY(double& lon, double& lat) {
  hadmap::txOdHeaderPtr headerPtr;
  if (m_pMapHandler) {
    hadmap::getHeader(m_pMapHandler, headerPtr);
    if (headerPtr) {
      double sourth = headerPtr->getSouth();
      double west = headerPtr->getWest();
      std::string georeference = headerPtr->getGeoReference();
#ifdef PROJ_NEW_API
      PJ_CONTEXT* C = proj_context_create();
      PJ* P = proj_create_crs_to_crs(C, "+proj=longlat +datum=WGS84 +no_defs", georeference.c_str(), NULL);
      if (P) {
        PJ_COORD a = proj_coord(lon, lat, 0, 0);
        PJ_COORD b = proj_trans(P, PJ_FWD, a);
        lon = b.xyz.x;
        lat = b.xyz.y;
        lon -= west;
        lat -= sourth;
        lon -= m_refPoint.x;
        lat -= m_refPoint.y;
        return;
      } else {
        coord_trans_api::lonlat2mercator(lon, lat);
        lat -= sourth;
        lon -= west;
        lon -= m_refPoint.x;
        lat -= m_refPoint.y;
        return;
      }
#else
      projPJ pj_src = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs");
      projPJ pj_dst = pj_init_plus(georeference.c_str());
      if (pj_src && pj_dst) {
        lon /= RAD_TO_DEG;
        lat /= RAD_TO_DEG;
        int code = pj_transform(pj_src, pj_dst, 1, 0, &lon, &lat, 0);
        lat -= sourth;
        lon -= west;
        lon -= m_refPoint.x;
        lat -= m_refPoint.y;
        return;
      } else {
        coord_trans_api::lonlat2mercator(lon, lat);
        lat -= sourth;
        lon -= west;
        lon -= m_refPoint.x;
        lat -= m_refPoint.y;
        return;
      }
#endif
    }
  }
  coord_trans_api::lonlat2mercator(lon, lat);
  lon -= m_refPoint.x;
  lat -= m_refPoint.y;
  return;
}

void CHadmap::XY2LonLat(double& x, double& y) {
  hadmap::txOdHeaderPtr headerPtr;
  if (m_pMapHandler) {
    hadmap::getHeader(m_pMapHandler, headerPtr);
    if (headerPtr) {
      double sourth = headerPtr->getSouth();
      double west = headerPtr->getWest();
      x += west;
      y += sourth;
      x += m_refPoint.x;
      y += m_refPoint.y;
      std::string georeference = headerPtr->getGeoReference();
#ifdef PROJ_NEW_API
      PJ_CONTEXT* C = proj_context_create();
      PJ* P = proj_create_crs_to_crs(C, georeference.c_str(), "+proj=longlat +datum=WGS84 +no_defs", NULL);
      if (P) {
        PJ_COORD a = proj_coord(x, y, 0, 0);
        PJ_COORD b = proj_trans(P, PJ_FWD, a);
        x = b.xyz.x;
        y = b.xyz.y;
        return;
      } else {
        coord_trans_api::mercator2lonlat(x, y);
        return;
      }
#else
      projPJ pj_src = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs");
      projPJ pj_dst = pj_init_plus(georeference.c_str());
      if (pj_src && pj_dst) {
        int code = pj_transform(pj_dst, pj_src, 1, 0, &x, &y, 0);
        x *= RAD_TO_DEG;
        y *= RAD_TO_DEG;
        return;
      } else {
        coord_trans_api::mercator2lonlat(x, y);
        return;
      }
#endif
    }
  }
  coord_trans_api::mercator2lonlat(x, y);
}

void CHadmap::ReplaceStr(std::string& str, const std::string& before, const std::string& after) {
  for (std::string::size_type pos(0); pos != std::string::npos; pos += after.length()) {
    pos = str.find(before, pos);
    if (pos != std::string::npos)
      str.replace(pos, before.length(), after);
    else
      break;
  }
}
}  // namespace ODR
