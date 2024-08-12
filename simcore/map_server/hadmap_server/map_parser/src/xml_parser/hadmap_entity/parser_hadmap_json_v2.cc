/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "parser_hadmap_json_v2.h"

#include <common/coord_trans.h>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <cstring>
#include <sstream>
#include "common/engine/math/vec3.h"
#include "common/log/system_logger.h"
#include "cross.h"
#include "curve_road.h"
#include "entity_link.h"
#include "entity_link2.h"
#include "group_entity.h"
#include "map_scene_v2.h"
#include "straight_road.h"

CMapSceneV2* CParserHadmapJsonV2::ParseV2(const char* strJson) {
  if (!strJson) {
    // LOG(ERROR) << "Parse Json error!";
    SYSTEM_LOGGER_ERROR("Parse Json error!");
    return nullptr;
  }

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parser map json error!");
    return nullptr;
  }

  CMapSceneV2* pScene = new CMapSceneV2();

  // name
  Json::Value& jName = root["name"];
  if (!jName.isNull()) {
    std::string strName = jName.asString();
    pScene->SetName(strName);
  }

  // data
  Json::Value& jData = root["data"];
  if (!jData.isNull()) {
    // std::string strData = jData.asString();
    std::string strData = jData.toStyledString();
    pScene->SetData(strData);
  }

  // geometry
  Json::Value& jGeometry = root["geometry"];
  if (!jGeometry.isNull()) {
    Json::Value& jSetting = jGeometry["setting"];

    double dRefLon = jSetting["refLon"].asDouble();
    double dRefLat = jSetting["refLat"].asDouble();
    double dRefAlt = jSetting["refAlt"].asDouble();

    pScene->SetRef(dRefLon, dRefLat, dRefAlt);

    // elements
    Json::Value& jElements = jGeometry["elements"];

    Json::ValueIterator elemItr = jElements.begin();

    for (; elemItr != jElements.end(); ++elemItr) {
      std::string strType = (*elemItr)["type"].asString();
      std::string strData = (*elemItr)["data"].toStyledString();
      if (boost::iequals("road", strType)) {
        hadmap::txRoadPtr pTXRoad(new hadmap::txRoad());
        ParseRoad(strData.c_str(), pTXRoad, pScene->TXLaneBoundaries(), dRefLon, dRefLat, dRefAlt);
        pScene->TXRoads().push_back(pTXRoad);

      } else if (boost::iequals("cross", strType)) {
        CCross* pCross = new CCross();
        ParseCross(pCross, pScene->TXLaneLinks(), strData.c_str(), dRefLon, dRefLat, dRefAlt);
        pScene->AddCross(pCross);
      } else if (boost::iequals("link", strType)) {
        CLinkEntityV2* pLink = new CLinkEntityV2();
        hadmap::txLaneLinkPtr linkPtr(new hadmap::txLaneLink());
        int nRet = ParseLink(pLink, linkPtr, strData.c_str(), dRefLon, dRefLat, dRefAlt);
        if (nRet == 0) {
          pScene->TXLaneLinks().push_back(linkPtr);
        } else {
          SYSTEM_LOGGER_ERROR("parse link error!");
        }
      }
    }
  }

  return pScene;
}

int CParserHadmapJsonV2::ParseCross(CCross* pCross, hadmap::txLaneLinks& laneLinks, const char* strJson, double dRefLon,
                                    double dRefLat, double dRefAlt) {
  if (!strJson || !pCross) {
    SYSTEM_LOGGER_ERROR("ParseCross error!");
    return -1;
  }

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse cross json error!");
    return -1;
  }

  // links
  Json::Value& jLinks = root["m_links"];
  Json::ValueIterator lsItr = jLinks.begin();
  int i = 0;
  pCross->m_links.clear();
  for (; lsItr != jLinks.end(); ++lsItr) {
    Json::Value jEnd = (*lsItr);
    Json::ValueIterator leItr = jEnd.begin();
    int j = 0;
    CrossLinkLinesByFromRoad roadFork;
    for (; leItr != jEnd.end(); ++leItr) {
      Json::Value jLink = (*leItr);
      Json::ValueIterator llItr = jLink.begin();
      int k = 0;
      CrossFromToLinkLines fromToLinks;
      for (; llItr != jLink.end(); ++llItr) {
        Json::Value jLinkInfo = (*llItr);

        CLinkEntityV2 le;
        hadmap::txLaneLinkPtr linkPtr(new hadmap::txLaneLink());
        int nRet = ExtractOneLinkData(&le, linkPtr, jLinkInfo, dRefLon, dRefLat, dRefAlt);
        if (nRet != 0) {
          k++;
          SYSTEM_LOGGER_INFO("cross road fork : %d to %d link %d is null", i, j, k);
          continue;
        }
        laneLinks.push_back(linkPtr);
        fromToLinks.push_back(le);

        k++;
        SYSTEM_LOGGER_INFO("cross road fork : %d to %d link %d finished", i, j, k);
      }
      roadFork.push_back(fromToLinks);
      j++;
    }
    i++;
    pCross->m_links.push_back(roadFork);
  }
  return 0;
}

int CParserHadmapJsonV2::ParseLink(CLinkEntityV2* pLink, hadmap::txLaneLinkPtr& ptrLaneLink, const char* strJson,
                                   double dRefLon, double dRefLat, double dRefAlt) {
  if (!strJson || !pLink) {
    SYSTEM_LOGGER_ERROR("ParseLink error!");
    return -1;
  }

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse link json error!");
    return -1;
  }

  return ExtractOneLinkData(pLink, ptrLaneLink, root, dRefLon, dRefLat, dRefAlt);
}

std::string CParserHadmapJsonV2::ToJson(CMapSceneV2* pScene) {
  if (!pScene) {
    SYSTEM_LOGGER_ERROR("ToJson error!");
    return "";
  }

  Json::Value root;
  // name
  { root["name"] = pScene->Name(); }

  // data
  { root["data"] = pScene->Data(); }

  // std::string strStyledResult = root.toStyledString();
  // Json::FastWriter writer;
  Json::StreamWriterBuilder swbuilder;
  std::string strResult = Json::writeString(swbuilder, root);

  Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "";
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream ss;
  writer->write(root, &ss);
  std::string strUnstyledResult = ss.str();
  // return strStyledResult;
  return strUnstyledResult;
}

int CParserHadmapJsonV2::ExtractOneLinkData(CLinkEntityV2* pLink, hadmap::txLaneLinkPtr& ptrLaneLink, Json::Value& root,
                                            double dRefLon, double dRefLat, double dRefAlt) {
  // from to
  Json::Value& jFromRoadID = root["from_road_id"];
  if (!jFromRoadID.isNull()) {
    pLink->m_from_road_id = root["from_road_id"].asUInt64();
    pLink->m_from_section_id = root["from_section_id"].asUInt64();
    pLink->m_from_lane_id = root["from_lane_id"].asInt64();

    pLink->m_to_road_id = root["to_road_id"].asUInt64();
    pLink->m_to_section_id = root["to_section_id"].asUInt64();
    pLink->m_to_lane_id = root["to_lane_id"].asInt64();

    ptrLaneLink->setFromRoadId(pLink->m_from_road_id);
    ptrLaneLink->setFromSectionId(pLink->m_from_section_id);
    ptrLaneLink->setFromLaneId(pLink->m_from_lane_id);
    ptrLaneLink->setToRoadId(pLink->m_to_road_id);
    ptrLaneLink->setToSectionId(pLink->m_to_section_id);
    ptrLaneLink->setToLaneId(pLink->m_to_lane_id);

    SYSTEM_LOGGER_INFO("from:(%d, %d, %d) to(%d, %d, %d)", ptrLaneLink->fromRoadId(), ptrLaneLink->fromSectionId(),
                       ptrLaneLink->fromLaneId(), ptrLaneLink->toRoadId(), ptrLaneLink->toSectionId(),
                       ptrLaneLink->toLaneId());
  } else {
    return -1;
  }

  // points
  Json::Value& jPoints = root["points"];
  if (!jPoints.isNull()) {
    hadmap::txLineCurve curve;
    Json::ValueIterator ptsItr = jPoints.begin();
    for (; ptsItr != jPoints.end(); ++ptsItr) {
      Json::Value vec = (*ptsItr)["vec"];
      double dX = vec["0"].asDouble();
      double dY = vec["1"].asDouble();
      double dZ = vec["2"].asDouble();
      /*
      double dX = -1 * vec["1"].asDouble();
      double dY = vec["0"].asDouble();
      double dZ = vec["2"].asDouble();
      */

      /*
      double dX = vec["1"].asDouble();
      double dY = vec["0"].asDouble();
      double dZ = vec["2"].asDouble();
      */

      coord_trans_api::local2lonlat(dX, dY, dZ, dRefLon, dRefLat, dRefAlt);
      CPoint3D p(dX, dY, dZ);
      pLink->m_points.push_back(p);

      hadmap::txPoint pt;
      pt.x = dX;
      pt.y = dY;
      pt.z = dZ;
      curve.addPoint(pt);
    }
    ptrLaneLink->setGeometry(curve);
  } else {
    ptrLaneLink->setGeometry(nullptr);
  }

  return 0;
}

int CParserHadmapJsonV2::ParseGISImageRenameParams(const char* strJson, sTagGISImageRenameParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse gis image rename param json failed: ", strJson);
    return -1;
  }

  if (!root.isNull()) {
    param.m_strOldName = root["name"].asString();
    param.m_strNewName = root["newName"].asString();
  }

  return 0;
}

int CParserHadmapJsonV2::ParseGISImageDeleteParams(const char* strJson, sTagGISImageDeleteParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse gis image delete param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jFiles = root;

  if (!jFiles.isNull()) {
    Json::ValueIterator fItr = jFiles.begin();
    for (; fItr != jFiles.end(); ++fItr) {
      std::string strFileName = (*fItr)["name"].asString();
      param.m_imageNames.push_back(strFileName);
    }
  }

  return 0;
}

int CParserHadmapJsonV2::ParseGISImageUploadParams(const char* strJson, sTagGISImageUploadParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse upload image param json failed: ", strJson);
    return -1;
  }

  // source
  param.m_strType = root["type"].asString();
  if (boost::iequals(param.m_strType, "dir")) {
    param.m_strSourceDir = root["dir"].asString();
  } else if (boost::iequals(param.m_strType, "files")) {
    param.m_strSourceDir = root["dir"].asString();
    // files
    Json::Value& jFiles = root["sources"];

    if (!jFiles.isNull()) {
      Json::ValueIterator fItr = jFiles.begin();
      for (; fItr != jFiles.end(); ++fItr) {
        std::string strFileName = (*fItr)["name"].asString();
        param.m_imageNames.push_back(strFileName);
      }
    }
  } else {
    SYSTEM_LOGGER_ERROR("upload gis images param error");
    return -1;
  }

  return 0;
}

int CParserHadmapJsonV2::ParseGISModelRenameParams(const char* strJson, sTagGISModelRenameParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse gis model rename param json failed: ", strJson);
    return -1;
  }

  if (!root.isNull()) {
    param.m_strOldName = root["name"].asString();
    param.m_strNewName = root["newName"].asString();
  }

  return 0;
}

int CParserHadmapJsonV2::ParseGISModelDeleteParams(const char* strJson, sTagGISModelDeleteParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse gis model delete param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jFiles = root;

  if (!jFiles.isNull()) {
    Json::ValueIterator fItr = jFiles.begin();
    for (; fItr != jFiles.end(); ++fItr) {
      std::string strFileName = (*fItr)["name"].asString();
      param.m_modelNames.push_back(strFileName);
    }
  }

  return 0;
}

int CParserHadmapJsonV2::ParseGISModelUploadParams(const char* strJson, sTagGISModelUploadParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse upload model param json failed: ", strJson);
    return -1;
  }

  // source
  param.m_strType = root["type"].asString();
  if (boost::iequals(param.m_strType, "dir")) {
    param.m_strSourceDir = root["dir"].asString();
  } else if (boost::iequals(param.m_strType, "files")) {
    param.m_strSourceDir = root["dir"].asString();
    // files
    Json::Value& jFiles = root["sources"];

    if (!jFiles.isNull()) {
      Json::ValueIterator fItr = jFiles.begin();
      for (; fItr != jFiles.end(); ++fItr) {
        std::string strFileName = (*fItr)["name"].asString();
        param.m_modelNames.push_back(strFileName);
      }
    }
  } else {
    SYSTEM_LOGGER_ERROR("upload gis model param error");
    return -1;
  }

  return 0;
}
