/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "tx_hadmap_cache.h"
#include <common/coord_trans.h>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <set>
CHadmap::CHadmap() {}

CHadmap::~CHadmap() {}

Base::txInt CHadmap::ParseMap(const char* hadmapPath) {
  if (!hadmapPath) {
    LogWarn << ("map file name is null");
    return -1;
  }
  boost::filesystem::path pathMapFile = hadmapPath;
  if (!boost::filesystem::exists(pathMapFile)) {
    LogWarn << ("map file %s not exist!", hadmapPath);
    return -1;
  }
  std::string strExt = pathMapFile.extension().string();
  int nMapType = hadmap::SQLITE;
  if (boost::algorithm::iequals(".sqlite", strExt)) {
    nMapType = hadmap::SQLITE;
  } else if (boost::algorithm::iequals(".xodr", strExt)) {
    nMapType = hadmap::OPENDRIVE;
  } else {
    LogWarn << ("unknown map type: %s!", hadmapPath);
    return -1;
  }
  // use mapSdk load map
  m_pMapHandler = NULL;
  int nRet = 0;
  nRet = hadmap::hadmapConnect(hadmapPath, static_cast<hadmap::MAP_DATA_TYPE>(nMapType), &m_pMapHandler);
  if (nRet != TX_HADMAP_HANDLE_OK) {
    LogWarn << ("hadmap file %s open failed!", hadmapPath);
    return -1;
  }
  m_dataType = static_cast<hadmap::MAP_DATA_TYPE>(nMapType);
  return 0;
}

Base::txInt CHadmap::CreateMap(const char* hadmapPath, const std::stringstream& cmd_json) {
  if (!hadmapPath) {
    LogWarn << ("map file name is null");
    return -1;
  }
  boost::filesystem::path pathMapFile = hadmapPath;
  if (boost::filesystem::exists(pathMapFile)) {
    boost::filesystem::remove(pathMapFile);
  }
  std::string strExt = pathMapFile.extension().string();
  int nMapType = hadmap::SQLITE;
  m_pMapHandler = NULL;
  int nRet = 0;
  nRet = hadmap::hadmapConnect(hadmapPath, static_cast<hadmap::MAP_DATA_TYPE>(nMapType), &m_pMapHandler);
  if (nRet != TX_HADMAP_HANDLE_OK) {
    LogWarn << ("hadmap file %s create failed!", hadmapPath);
    return -1;
  }
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  std::string strJson = cmd_json.str();
  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson.c_str());
  if (!reader->parse(strJson.c_str(), strJson.c_str() + nLen, &root, &errs)) {
    LogWarn << ("parser json %s failed: ", strJson);
    return -1;
  }
  JsonToMap(root);
  return 0;
}

Base::txInt CHadmap::ModifyMap(const std::stringstream& cmd_json) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  std::string strJson = cmd_json.str();
  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson.c_str());
  if (!reader->parse(strJson.c_str(), strJson.c_str() + nLen, &root, &errs)) {
    LogWarn << ("parser json %s failed: ", strJson.c_str());
    return -1;
  }
  // parse datas
  std::vector<ModifyInfo> modifyInfoVec;
  for (auto it : root["datas"]) {
    ModifyInfo info;
    info.strCmd = formatJosnValue(it["cmd"]);
    info.strType = formatJosnValue(it["type"]);
    info.strId = formatJosnValue(it["id"]);
    modifyInfoVec.push_back(info);
  }
  // parse roads;
  Id2RoadMap roadmap;
  int ret = ParseRoads(root["roads"], roadmap);
  if (ret != 0) {
    return -1;
  }
  // start modifyinfo
  for (auto it : modifyInfoVec) {
    if (it.strType == "road") {
      std::string id = it.strId;
      if (it.strCmd == "add") {
        if (roadmap.find(id) != roadmap.end()) {
          hadmap::txRoads roads;
          roads.push_back(roadmap[id]);
          int InsertRet = hadmap::insertRoads(m_pMapHandler, roads);
          if (InsertRet != 0) {
            LogWarn << "InsertRet:" << InsertRet;
          }
        }
      } else if (it.strCmd == "update") {
        if (roadmap.find(id) != roadmap.end()) {
          hadmap::txRoads roads;
          roads.push_back(roadmap[id]);
          int UpdateRet = hadmap::upDateRoads(m_pMapHandler, roads);
          if (UpdateRet != 0) {
            LogWarn << "InsertRet:" << UpdateRet;
          }
        }
      } else if (it.strCmd == "delete") {
        hadmap::txRoads roads;
        roads.push_back(roadmap[id]);
        int DeleteRet = hadmap::deleteRoads(m_pMapHandler, roads);
        if (DeleteRet != 0) {
          LogWarn << "DeleteRet:" << DeleteRet;
        }
      }
    }
  }
  return 0;
}

Base::txInt CHadmap::SaveMap(const char* hadmapPath) {
  if (!hadmapPath) {
    LogWarn << ("map file name is null");
    return -1;
  }
  boost::filesystem::path pathMapFile = hadmapPath;
  if (boost::filesystem::exists(pathMapFile)) {
    LogWarn << ("map file name is exist");
    return -2;
  }
  std::string strExt = pathMapFile.extension().string();
  int nMapType = hadmap::SQLITE;
  if (boost::algorithm::iequals(".sqlite", strExt)) {
    nMapType = hadmap::SQLITE;
  } else {
    return -3;
  }
  return hadmap::hadmapOutput(hadmapPath, (hadmap::MAP_DATA_TYPE)nMapType, m_pMapHandler);
}

hadmap::MAP_DATA_TYPE CHadmap::getHadmapDataType() { return this->m_dataType; }
//
Base::txInt CHadmap::ParseDataToJson(Base::txStringStream& jsonValue) {
  // get headers
  m_refPoint = hadmap::getRefPoint(m_pMapHandler);
  coord_trans_api::lonlat2mercator(m_refPoint.x, m_refPoint.y);

  // get roads
  bool bWholeData = true;
  int nRet = 0;
  hadmap::txRoads roads;
  nRet = hadmap::getRoads(m_pMapHandler, &bWholeData, roads);
  if (nRet != TX_HADMAP_DATA_OK && nRet != TX_HADMAP_DATA_EMPTY) {
    LogWarn << ("hadmap::getRoads failed!");
    return -1;
  }
  // get refpoint (left_bottom point)

  Json::Value root_json;
  // road to json
  RoadsToJson(roads, root_json);
  // get lanelinks
  hadmap::txLaneLinks lanelinks;
  hadmap::roadpkid fromRoadId = ROAD_PKID_INVALID;
  hadmap::roadpkid toRoadId = ROAD_PKID_INVALID;
  nRet = hadmap::getLaneLinks(m_pMapHandler, fromRoadId, toRoadId, lanelinks);
  if (nRet != TX_HADMAP_DATA_OK && nRet != TX_HADMAP_DATA_EMPTY) {
    LogWarn << ("hadmap::getLaneLinks failed!");
    return -1;
  }
  // json_laneLink;
  LaneLinkToJson(lanelinks, root_json);
  Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "";
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream ss;
  writer->write(root_json, &ss);
  jsonValue << ss.str();
  return 0;
}

void CHadmap::RoadsToJson(const hadmap::txRoads roads, Json::Value& json_value) {
  for (auto it : roads) {
    Json::Value jsonRoad;
    jsonRoad["id"] = formatIntValue(it->getId());
    jsonRoad["length"] = it->getLength();
    jsonRoad["type"] = it->getRoadType();
    const hadmap::txCurve* curvePtr = it->getGeometry();
    hadmap::txLanes lanes;
    //

    if (NonNull_Pointer(curvePtr)) {
      double start_s = 0.0;
      uint64_t startIndex = 0;
      uint64_t endIndex = 0;
      double last_length_sum = 0;
      for (size_t i = 0; i < it->getSections().size(); i++) {
        hadmap::PointVec points;
        Json::Value jsonSection;
        jsonSection["id"] = formatIntValue(it->getSections().at(i)->getId());
        double lengthSection = it->getSections().at(i)->getLength();
        CurveToPoints(curvePtr, points, start_s, start_s + lengthSection);
        endIndex = startIndex + points.size() - 1;
        jsonSection["start"] = startIndex;
        jsonSection["end"] = endIndex;
        jsonSection["length"] = lengthSection;
        jsonSection["startPercent"] = last_length_sum / it->getLength();
        last_length_sum += it->getSections().at(i)->getLength();
        jsonSection["endPercent"] = last_length_sum / it->getLength();
        for (size_t j = 0; j < points.size(); j++) {
          Json::Value jsonPoint;
          jsonPoint["x"] = points.at(j).x;
          jsonPoint["y"] = points.at(j).y;
          jsonPoint["z"] = points.at(j).z;
          jsonRoad["samplePoints"].append(jsonPoint);
        }
        startIndex = (endIndex + 1);
        start_s += lengthSection;
        hadmap::txLanes tmpLanes = it->getSections().at(i)->getLanes();
        //
        std::set<hadmap::laneboundarypkid> boundaryIdset;
        for (size_t j = 0; j < tmpLanes.size(); j++) {
          Json::Value laneJson;
          laneJson["id"] = formatIntValue(tmpLanes.at(j)->getId());
          laneJson["lbid"] = formatIntValue(tmpLanes.at(j)->getLeftBoundaryId());
          laneJson["rbid"] = formatIntValue(tmpLanes.at(j)->getRightBoundaryId());
          laneJson["type"] = tmpLanes.at(j)->getLaneType();
          laneJson["speedlimit"] = tmpLanes.at(j)->getSpeedLimit();
          const hadmap::txCurve* laneCurve = tmpLanes.at(j)->getGeometry();
          hadmap::PointVec lanePoints;
          CurveToPoints(laneCurve, lanePoints);
          for (size_t m = 0; m < lanePoints.size(); m++) {
            Json::Value jsonPoint;
            jsonPoint["x"] = lanePoints.at(m).x;
            jsonPoint["y"] = lanePoints.at(m).y;
            jsonPoint["z"] = lanePoints.at(m).z;
            laneJson["samplePoints"].append(jsonPoint);
          }
          // left boundary
          Json::Value leftBoudaryJson;
          hadmap::PointVec leftBoudaryPoints;
          hadmap::txCurve* leftBoudary = (hadmap::txCurve*)tmpLanes.at(j)->getLeftBoundary()->getGeometry();
          CurveToPoints(leftBoudary, leftBoudaryPoints);
          for (size_t m = 0; m < leftBoudaryPoints.size(); m++) {
            Json::Value jsonPoint;
            jsonPoint["x"] = leftBoudaryPoints.at(m).x;
            jsonPoint["y"] = leftBoudaryPoints.at(m).y;
            jsonPoint["z"] = leftBoudaryPoints.at(m).z;
            leftBoudaryJson["samplePoints"].append(jsonPoint);
          }
          leftBoudaryJson["mark"] = tmpLanes.at(j)->getLeftBoundary()->getLaneMark();
          leftBoudaryJson["id"] = formatIntValue(tmpLanes.at(j)->getLeftBoundary()->getId());
          if (boundaryIdset.find(tmpLanes.at(j)->getLeftBoundary()->getId()) == boundaryIdset.end()) {
            jsonSection["boundarys"].append(leftBoudaryJson);
            boundaryIdset.insert(tmpLanes.at(j)->getLeftBoundary()->getId());
          }
          // right boundary
          Json::Value rightBoudaryJson;
          hadmap::PointVec rightBoudaryPoints;
          const hadmap::txCurve* rightBoudary = tmpLanes.at(j)->getRightBoundary()->getGeometry();
          CurveToPoints(rightBoudary, rightBoudaryPoints);
          for (size_t m = 0; m < rightBoudaryPoints.size(); m++) {
            Json::Value jsonPoint;
            jsonPoint["x"] = rightBoudaryPoints.at(m).x;
            jsonPoint["y"] = rightBoudaryPoints.at(m).y;
            jsonPoint["z"] = rightBoudaryPoints.at(m).z;
            rightBoudaryJson["samplePoints"].append(jsonPoint);
          }
          rightBoudaryJson["mark"] = tmpLanes.at(j)->getRightBoundary()->getLaneMark();
          rightBoudaryJson["id"] = formatIntValue(tmpLanes.at(j)->getRightBoundary()->getId());
          if (boundaryIdset.find(tmpLanes.at(j)->getRightBoundary()->getId()) == boundaryIdset.end()) {
            jsonSection["boundarys"].append(rightBoudaryJson);
            boundaryIdset.insert(tmpLanes.at(j)->getRightBoundary()->getId());
          }
          jsonSection["lanes"].append(laneJson);
        }
        jsonRoad["sections"].append(jsonSection);
      }
    }
    json_value["roads"].append(jsonRoad);
  }
}

void CHadmap::LaneLinkToJson(const hadmap::txLaneLinks links, Json::Value& json_value) {
  for (auto it : links) {
    Json::Value link_json;
    link_json["fid"] = formatIntValue(it->fromLaneId());
    link_json["frid"] = formatIntValue(it->fromRoadId());
    link_json["fsid"] = formatIntValue(it->fromSectionId());
    link_json["id"] = formatIntValue(it->getId());
    link_json["junctionid"] = formatIntValue(it->getJunctionId());
    link_json["length"] = it->getGeometry()->getLength();
    link_json["tid"] = formatIntValue(it->toLaneId());
    link_json["trid"] = formatIntValue(it->toRoadId());
    link_json["tsid"] = formatIntValue(it->toSectionId());
    const hadmap::txCurve* laneLinkCurve = it->getGeometry();
    hadmap::PointVec laneLinkPoints;
    CurveToPoints(laneLinkCurve, laneLinkPoints);
    for (size_t m = 0; m < laneLinkPoints.size(); m++) {
      Json::Value jsonPoint;
      jsonPoint["x"] = laneLinkPoints.at(m).x;
      jsonPoint["y"] = laneLinkPoints.at(m).y;
      jsonPoint["z"] = laneLinkPoints.at(m).z;
      link_json["samplePoints"].append(jsonPoint);
    }
    json_value["lanelinks"].append(link_json);
  }
}

void CHadmap::CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points) {
  if (Null_Pointer(curve)) {
    return;
  }
  double fLen = curve->getLength();
  double fInterval = 10;
  if (fLen < 50) {
    fInterval = fLen / 10;
  } else if (fLen >= 50 && fLen < 100) {
    fInterval = fLen / 20;
  } else if (fLen >= 100 && fLen < 300) {
    fInterval = fLen / 30;
  }
  if (fLen < 0.1) {
    fInterval = 1;
  }
  hadmap::PointVec pts;
  curve->sample(fInterval, pts);
  if (pts.size() > 0) {
    hadmap::PointVec::iterator itr = pts.begin();
    // points.push_back(*itr);
    // itr++;
    for (; itr != pts.end(); ++itr) {
      double x = itr->x;
      double y = itr->y;
      double z = itr->z;
      // coord_trans_api::lonlat2enu(x, y, z,pts.at(0).x, pts.at(0).y, pts.at(0).z);
      coord_trans_api::lonlat2mercator(x, y);
      itr->x = x - m_refPoint.x;
      itr->y = y - m_refPoint.y;
      itr->z = z;
      // points.push_back(*itr);
      points.push_back(*itr);
    }
    // points.pop_back();
  }
}

void CHadmap::CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points, const double start_s,
                            const double end_s) {
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
  for (int i = 0; i * fInterval < fLen + fInterval; ++i) {
    hadmap::txPoint pt = curve->getPoint(start_s + i * fInterval);
    double x = pt.x;
    double y = pt.y;
    double z = pt.z;
    coord_trans_api::lonlat2mercator(x, y);
    // coord_trans_api::lonlat2global(x, y, z);
    pt.x = x - m_refPoint.x;
    pt.y = y - m_refPoint.y;
    pt.z = z;
    points.push_back(pt);
  }
}

std::string CHadmap::formatIntValue(uint64_t data) { return std::to_string(data); }

int CHadmap::JsonToMap(Json::Value& json_root) {
  if (json_root["roads"].empty()) {
    return -1;
  }
  hadmap::txRoads roads;

  hadmap::insertRoads(m_pMapHandler, roads);
}

int CHadmap::ParseRoads(Json::Value& roads, Id2RoadMap& map) {
  hadmap::txPoint pointref = hadmap::getRefPoint(m_pMapHandler);
  coord_trans_api::lonlat2mercator(pointref.x, pointref.y);
  for (auto it : roads) {
    hadmap::txRoadPtr roadPtr = std::make_shared<hadmap::txRoad>();
    roadPtr->setId(atoi(it["id"].asString().c_str()));
    roadPtr->setRoadType((hadmap::ROAD_TYPE)it["type"].asInt());
    hadmap::PointVec points;
    for (auto point : it["samplePoints"]) {
      hadmap::txPoint tPoint;
      double x = point["x"].asDouble() + pointref.x;
      double y = point["y"].asDouble() + pointref.y;
      double z = point["z"].asDouble();
      coord_trans_api::mercator2lonlat(x, y);
      tPoint.x = x;
      tPoint.y = y;
      tPoint.z = z;
      points.push_back(tPoint);
    }
    hadmap::txSections sections;
    for (auto section : it["sections"]) {
      hadmap::txSectionPtr sectionPtr = std::make_shared<hadmap::txSection>();
      sectionPtr->setId(atoi(section["id"].asString().c_str()));
      std::map<int, hadmap::txLaneBoundaryPtr> id2BoundaryMap;
      hadmap::txLaneBoundaries boundaries;
      for (auto laneBoudary : section["boundarys"]) {
        hadmap::txLaneBoundaryPtr boundaryPtr = std::make_shared<hadmap::txLaneBoundary>();
        boundaryPtr->setId(atoi(laneBoudary["id"].asString().c_str()));
        boundaryPtr->setLaneMark(hadmap::LANE_MARK(laneBoudary["mark"].asInt()));
        hadmap::PointVec points;
        for (auto point : laneBoudary["samplePoints"]) {
          hadmap::txPoint tPoint;
          double x = point["x"].asDouble() + pointref.x;
          double y = point["y"].asDouble() + pointref.y;
          double z = point["z"].asDouble();
          coord_trans_api::mercator2lonlat(x, y);
          tPoint.x = x;
          tPoint.y = y;
          tPoint.z = z;
          points.push_back(tPoint);
        }
        boundaryPtr->setGeometry(points, hadmap::CoordType::COORD_WGS84);
        id2BoundaryMap[boundaryPtr->getId()] = boundaryPtr;
      }
      hadmap::txLanes lanes;
      for (auto lane : section["lanes"]) {
        hadmap::txLanePtr lanePtr = std::make_shared<hadmap::txLane>();
        lanePtr->setId(atoi(lane["id"].asString().c_str()));
        lanePtr->setRoadId(roadPtr->getId());
        lanePtr->setSectionId(sectionPtr->getId());
        lanePtr->setLeftBoundaryId(atoi(lane["lbid"].asString().c_str()));
        lanePtr->setLeftBoundary(id2BoundaryMap[atoi(lane["lbid"].asString().c_str())]);
        LogWarn << "1111111111" << lanePtr->getLeftBoundary();
        lanePtr->setRightBoundaryId(atoi(lane["rbid"].asString().c_str()));
        lanePtr->setRightBoundary(id2BoundaryMap[lanePtr->getRightBoundaryId()]);
        lanePtr->setLaneType(hadmap::LANE_TYPE(lane["type"].asInt()));
        lanes.push_back(lanePtr);
        hadmap::PointVec points;
        for (auto point : lane["samplePoints"]) {
          hadmap::txPoint tPoint;
          double x = point["x"].asDouble() + pointref.x;
          double y = point["y"].asDouble() + pointref.y;
          double z = point["z"].asDouble();
          coord_trans_api::mercator2lonlat(x, y);
          tPoint.x = x;
          tPoint.y = y;
          tPoint.z = z;
          points.push_back(tPoint);
        }
        lanePtr->setGeometry(points, hadmap::CoordType::COORD_WGS84);
      }
      sections.push_back(sectionPtr);
      sectionPtr->setLanes(lanes);
    }
    roadPtr->setGeometry(points, hadmap::CoordType::COORD_WGS84);
    roadPtr->setSections(sections);
    map[it["id"].asString()] = roadPtr;
  }
  return 0;
}

int CHadmap::ParseLanelinks(Json::Value& lanelinks, Id2LaneLinkMap& map) {
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
    hadmap::PointVec points;
    for (auto point : it["samplePoints"]) {
      hadmap::txPoint tPoint;
      double x = point["x"].asDouble();
      double y = point["y"].asDouble();
      double z = point["z"].asDouble();
      coord_trans_api::mercator2lonlat(x, y);
      tPoint.x = x;
      tPoint.y = y;
      tPoint.z = z;
      points.push_back(tPoint);
    }

    lanelink->setGeometry(points, hadmap::CoordType::COORD_WGS84);
    map[it["id"].asString()] = lanelink;
  }
}

std::string CHadmap::formatJosnValue(Json::Value& value) {
  if (value.isString()) return value.asString();
  return std::string();
}
