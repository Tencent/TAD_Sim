/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "parser_hadmap_json.h"

#include <common/coord_trans.h>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <cstring>
#include <sstream>
#include "common/engine/math/vec3.h"
#include "common/log/system_logger.h"
#include "cross.h"
#include "curve_road.h"
#include "entity_link.h"
#include "group_entity.h"
#include "map_scene.h"
#include "straight_road.h"

CMapScene* CParserHadmapJson::Parse(const char* strJson) {
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
    // LOG(ERROR) << "parser map json error!";
    SYSTEM_LOGGER_ERROR("parser map json error!");
    return nullptr;
  }

  CMapScene* pScene = new CMapScene();

  // setting
  Json::Value& jSetting = root["setting"];

  double dRefLon = jSetting["m_refLon"].asDouble();
  double dRefLat = jSetting["m_refLat"].asDouble();
  double dRefAlt = jSetting["m_refAlt"].asDouble();

  pScene->SetRef(dRefLon, dRefLat, dRefAlt);

  // straights
  Json::Value& jStraights = root["straights"];
  if (!jStraights.isNull()) {
    Json::ValueIterator sItr = jStraights.begin();

    for (; sItr != jStraights.end(); ++sItr) {
      CStraightRoad* pRoad = new CStraightRoad();
      pRoad->m_id = (*sItr)["m_ID"].asInt();
      pRoad->m_name = (*sItr)["m_name"].asString();
      pRoad->m_height = (*sItr)["m_height"].asInt();
      pRoad->m_length = (*sItr)["m_length"].asInt();
      pRoad->m_width = (*sItr)["m_width"].asInt();
      pRoad->m_maxSpeed = (*sItr)["m_maxSpeed"].asInt();
      pRoad->m_numberOfLanes = (*sItr)["m_numberOfLanes"].asInt();
      pRoad->m_widthOfLane = (*sItr)["m_laneWidth"].asDouble();
      pRoad->m_roadThickness = (*sItr)["m_roadThickness"].asDouble();
      pRoad->m_roadFriction = (*sItr)["m_friction"].asDouble();
      pRoad->m_roadLaneMark = (*sItr)["m_roadLaneMark"].asInt();

      Json::Value& pos = (*sItr)["m_position"];
      Json::ValueIterator itr = pos.begin();
      int idx = 0;
      for (; itr != pos.end(); ++itr) {
        pRoad->m_position[idx] = (*itr).asDouble();
        idx++;
      }

      Json::Value& rot = (*sItr)["m_rotation"];
      itr = rot.begin();
      idx = 0;
      for (; itr != rot.end(); ++itr) {
        pRoad->m_rotation[idx] = (*itr).asDouble();
        idx++;
      }

      Json::Value& mat = (*sItr)["m_matrix"];
      itr = mat.begin();
      idx = 0;
      for (; itr != mat.end(); ++itr) {
        pRoad->m_matrix[idx] = (*itr).asDouble();
        idx++;
      }

      Json::Value& start = (*sItr)["m_start"];
      itr = start.begin();
      idx = 0;
      for (; itr != start.end(); ++itr) {
        pRoad->m_start[idx] = (*itr).asDouble();
        idx++;
      }

      Json::Value& end = (*sItr)["m_end"];
      itr = end.begin();
      idx = 0;
      for (; itr != end.end(); ++itr) {
        pRoad->m_end[idx] = (*itr).asDouble();
        idx++;
      }

      pRoad->m_geometry = (*sItr)["m_geometry"].asString();
      hadmap::txRoadPtr pTXRoad(new hadmap::txRoad());
      ParseRoad(pRoad->m_geometry.c_str(), pTXRoad, pScene->TXLaneBoundaries(), dRefLon, dRefLat, dRefAlt);
      pRoad->m_txRoad = pTXRoad;
      pScene->TXRoads().push_back(pTXRoad);
      pScene->AddStraight(pRoad);
    }
  }

  // curves
  Json::Value& jCurves = root["curves"];
  if (!jCurves.isNull()) {
    Json::ValueIterator cItr = jCurves.begin();

    for (; cItr != jCurves.end(); ++cItr) {
      CCurveRoad* pRoad = new CCurveRoad();
      pRoad->m_id = (*cItr)["m_ID"].asInt();
      pRoad->m_name = (*cItr)["m_name"].asString();
      pRoad->m_height = (*cItr)["m_height"].asInt();
      pRoad->m_length = (*cItr)["m_length"].asInt();
      pRoad->m_width = (*cItr)["m_width"].asInt();
      pRoad->m_maxSpeed = (*cItr)["m_maxSpeed"].asInt();
      pRoad->m_numberOfLanes = (*cItr)["m_numberOfLanes"].asInt();
      pRoad->m_widthOfLane = (*cItr)["m_laneWidth"].asDouble();
      pRoad->m_roadThickness = (*cItr)["m_roadThickness"].asDouble();
      pRoad->m_roadFriction = (*cItr)["m_friction"].asDouble();
      pRoad->m_roadLaneMark = (*cItr)["m_roadLaneMark"].asInt();

      Json::Value& pos = (*cItr)["m_position"];
      Json::ValueIterator itr = pos.begin();
      int idx = 0;
      for (; itr != pos.end(); ++itr) {
        pRoad->m_position[idx] = (*itr).asDouble();
        idx++;
      }

      Json::Value& rot = (*cItr)["m_rotation"];
      itr = rot.begin();
      idx = 0;
      for (; itr != rot.end(); ++itr) {
        pRoad->m_rotation[idx] = (*itr).asDouble();
        idx++;
      }

      Json::Value& mat = (*cItr)["m_matrix"];
      itr = mat.begin();
      idx = 0;
      for (; itr != mat.end(); ++itr) {
        pRoad->m_matrix[idx] = (*itr).asDouble();
        idx++;
      }

      Json::Value& jControlPoints = (*cItr)["m_controlPoints"];
      itr = jControlPoints.begin();
      for (; itr != jControlPoints.end(); ++itr) {
        Json::Value vec = (*itr)["vec"];
        double dX = vec["0"].asDouble();
        double dY = vec["1"].asDouble();
        double dZ = vec["2"].asDouble();

        CVector3 point;
        point.Set(dX, dY, dZ);
        pRoad->m_controlPoints.push_back(point);
      }

      pRoad->m_geometry = (*cItr)["m_geometry"].asString();
      hadmap::txRoadPtr pTXRoad(new hadmap::txRoad());
      ParseRoad(pRoad->m_geometry.c_str(), pTXRoad, pScene->TXLaneBoundaries(), dRefLon, dRefLat, dRefAlt);
      pRoad->m_txRoad = pTXRoad;
      pScene->TXRoads().push_back(pTXRoad);
      pScene->AddCurve(pRoad);
    }
  }

  // crosses
  Json::Value& jCrosses = root["crosses"];
  if (!jCrosses.isNull()) {
    Json::ValueIterator cItr = jCrosses.begin();

    for (; cItr != jCrosses.end(); ++cItr) {
      CCross* pCross = new CCross();
      pCross->m_id = (*cItr)["m_ID"].asInt();
      pCross->m_name = (*cItr)["m_name"].asString();
      pCross->m_height = (*cItr)["m_height"].asInt();
      pCross->m_length = (*cItr)["m_length"].asInt();
      pCross->m_width = (*cItr)["m_width"].asInt();
      pCross->m_maxSpeed = (*cItr)["m_maxSpeed"].asInt();
      /*pCross->m_numberOfLanes = (*sItr)["m_numberOfLanes"].asInt();
      pCross->m_widthOfLane = (*sItr)["m_laneWidth"].asDouble();
      pCross->m_roadThickness = (*sItr)["m_roadThickness"].asDouble();
      pCross->m_roadFriction = (*sItr)["m_friction"].asDouble();
      pCross->m_roadLaneMark = (*sItr)["m_roadLaneMark"].asInt();*/

      Json::Value& pos = (*cItr)["m_position"];
      Json::ValueIterator itr = pos.begin();
      int idx = 0;
      for (; itr != pos.end(); ++itr) {
        pCross->m_position[idx] = (*itr).asDouble();
        idx++;
      }

      Json::Value& rot = (*cItr)["m_rotation"];
      itr = rot.begin();
      idx = 0;
      for (; itr != rot.end(); ++itr) {
        pCross->m_rotation[idx] = (*itr).asDouble();
        idx++;
      }

      Json::Value& mat = (*cItr)["m_matrix"];
      itr = mat.begin();
      idx = 0;
      for (; itr != mat.end(); ++itr) {
        pCross->m_matrix[idx] = (*itr).asDouble();
        idx++;
      }

      pCross->m_geometry = (*cItr)["m_geometry"].asString();
      hadmap::txRoadPtr pTXRoad(new hadmap::txRoad());
      ParseCross(pCross, pCross->m_geometry.c_str(), dRefLon, dRefLat, dRefAlt);
      pScene->AddCross(pCross);
    }
  }

  // groups
  Json::Value& jGroups = root["groups"];
  Json::ValueIterator gItr = jGroups.begin();
  for (; gItr != jGroups.end(); ++gItr) {
    CGroupEntity* pGroup = new CGroupEntity();
    pGroup->SetScene(pScene);
    Json::Value& children = (*gItr)["m_children"];
    Json::ValueIterator itr = children.begin();
    for (; itr != children.end(); ++itr) {
      pGroup->AddChild((*itr).asInt());
    }

    Json::Value& links = (*gItr)["m_links"];
    itr = links.begin();
    for (; itr != links.end(); ++itr) {
      CLinkEntity* pLink = new CLinkEntity();

      pLink->m_from_id = (*itr)["m_fromID"].asInt();
      pLink->m_from_end = (*itr)["m_fromEnd"].asInt();
      pLink->m_to_id = (*itr)["m_toID"].asInt();
      pLink->m_to_end = (*itr)["m_toEnd"].asInt();
      pGroup->AddLink(pLink);

      CMapElement* pRoadFrom = pScene->FindElement(pLink->m_from_id);
      CMapElement* pRoadTo = pScene->FindElement(pLink->m_to_id);
      if (!pRoadTo || !pRoadFrom) {
        SYSTEM_LOGGER_ERROR("parse scene format error!");
        continue;
      }
      hadmap::txRoadPtr pTXRoadFrom = nullptr;
      hadmap::txRoadPtr pTXRoadTo = nullptr;
      if (pRoadFrom->m_type == HET_Straight) {
        CStraightRoad* pStraightFrom = static_cast<CStraightRoad*>(pRoadFrom);
        pTXRoadFrom = pStraightFrom->m_txRoad;
      } else if (pRoadFrom->m_type == HET_Curve) {
        CCurveRoad* pCurveFrom = static_cast<CCurveRoad*>(pRoadFrom);
        pTXRoadFrom = pCurveFrom->m_txRoad;
      } else if (pRoadFrom->m_type == HET_Cross) {
        pTXRoadFrom = nullptr;
      } else {
        SYSTEM_LOGGER_ERROR("Parse link type error!");
      }
      if (pRoadTo->m_type == HET_Straight) {
        CStraightRoad* pStraightTo = static_cast<CStraightRoad*>(pRoadTo);
        pTXRoadTo = pStraightTo->m_txRoad;
      } else if (pRoadTo->m_type == HET_Curve) {
        CCurveRoad* pCurveTo = static_cast<CCurveRoad*>(pRoadTo);
        pTXRoadTo = pCurveTo->m_txRoad;
      } else if (pRoadTo->m_type == HET_Cross) {
        pTXRoadTo = nullptr;
      } else {
        SYSTEM_LOGGER_ERROR("Parse link type error!");
      }
      if (!pTXRoadFrom || !pTXRoadTo) {
        continue;
      }
      /*if (pRoadFrom->m_type == HET_Straight && pRoadTo->m_type == HET_Straight)
      {
              CStraightRoad* pStraightFrom = static_cast<CStraightRoad*>(pRoadFrom);
              CStraightRoad* pStraightTo = static_cast<CStraightRoad*>(pRoadTo);

              pTXRoadFrom = pStraightFrom->m_txRoad;
              pTXRoadTo = pStraightTo->m_txRoad;
      }*/
      hadmap::txSections secsFrom = pTXRoadFrom->getSections();
      hadmap::txSections secsTo = pTXRoadTo->getSections();
      if (secsFrom.size() < 1 || secsTo.size() < 1) {
        SYSTEM_LOGGER_ERROR("parse scene from error: section wrong");
        continue;
      }
      for (int i = -2; i < 3; ++i) {
        if (i == 0) {
          continue;
        }
        hadmap::txLaneLinkPtr linkPtr(new hadmap::txLaneLink());
        if (i < 0) {
          linkPtr->setFromRoadId(pTXRoadFrom->getId());
          linkPtr->setToRoadId(pTXRoadTo->getId());
        } else {
          linkPtr->setToRoadId(pTXRoadFrom->getId());
          linkPtr->setFromRoadId(pTXRoadTo->getId());
        }

        linkPtr->setFromSectionId(secsFrom[0]->getId());
        linkPtr->setFromLaneId(i);
        linkPtr->setToSectionId(secsTo[0]->getId());
        linkPtr->setToLaneId(i);
        linkPtr->setGeometry(nullptr);
        SYSTEM_LOGGER_INFO("from:(%d, %d, %d) to(%d, %d, %d)", linkPtr->fromRoadId(), linkPtr->fromSectionId(),
                           linkPtr->fromLaneId(), linkPtr->toRoadId(), linkPtr->toSectionId(), linkPtr->toLaneId());
        pScene->TXLaneLinks().push_back(linkPtr);
      }
      //}
    }
    pGroup->GenerateCrossLink();
    pScene->AddGroup(pGroup);
  }

  return pScene;
}

int CParserHadmapJson::JsonToXml(const char* strPath, const char* strFilename, const char* strJson) {
  if (!strPath || !strFilename || !strJson) {
    // LOG(ERROR) << "Json to Xml error!";
    SYSTEM_LOGGER_ERROR("Json to Xml error!");
    return -1;
  }

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    // LOG(ERROR) << "parser map json error!";
    SYSTEM_LOGGER_ERROR("parser map json error!");
    return -1;
  }

  /*
  // straights
  Json::Value& jStraights = root["straights"];
  */
  // groups
  return 0;
}

int CParserHadmapJson::ParseRoad(const char* strJson, hadmap::txRoadPtr& road, hadmap::txLaneBoundaries& vecBoundaries,
                                 double dRefLon, double dRefLat, double dRefAlt) {
  if (!strJson) {
    // LOG(ERROR) << "Json to Xml error!";
    SYSTEM_LOGGER_ERROR("ParseRoad error!");
    return -1;
  }

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse road json error!");
    return -1;
  }

  std::unordered_map<int, hadmap::txLaneBoundaryPtr> boundaries;

  // txRoad
  // hadmap::txRoad road;

  Json::Value& jPoints = root["m_points"];
  if (!jPoints.isNull()) {
    hadmap::txLineCurve curve;
    Json::ValueIterator itr = jPoints.begin();
    for (; itr != jPoints.end(); ++itr) {
      Json::Value vec = (*itr)["vec"];
      double dX = vec["0"].asDouble();
      double dY = vec["1"].asDouble();
      double dZ = vec["2"].asDouble();
      // double dX = -1 * vec["1"].asDouble();
      // double dY = vec["0"].asDouble();
      // double dZ = vec["2"].asDouble();

      coord_trans_api::local2lonlat(dX, dY, dZ, dRefLon, dRefLat, dRefAlt);

      hadmap::txPoint pt;
      pt.x = dX;
      pt.y = dY;
      pt.z = dZ;
      curve.addPoint(pt);
    }

    road->setGeometry(curve);
  }

  road->setId(root["m_roadID"].asInt());
  road->setName(root["m_name"].asString());

  Json::Value& jSections = root["m_sections"];
  if (!jSections.isNull()) {
    Json::ValueIterator itr = jSections.begin();
    for (; itr != jSections.end(); ++itr) {
      hadmap::txSectionPtr pSec(new hadmap::txSection());
      pSec->setRoadId((*itr)["m_roadID"].asInt());
      pSec->setId((*itr)["m_sectionID"].asInt());

      // lane boundaries
      Json::Value& jBoundaries = (*itr)["m_laneBoundaries"];
      Json::ValueIterator bItr = jBoundaries.begin();
      for (; bItr != jBoundaries.end(); ++bItr) {
        hadmap::txLaneBoundaryPtr pBound(new hadmap::txLaneBoundary());
        pBound->setId((*bItr)["m_id"].asInt());
        Json::Value jBoundaryPoints = (*bItr)["m_points"];
        hadmap::txLineCurve curve;
        Json::ValueIterator bpItr = jBoundaryPoints.begin();
        for (; bpItr != jBoundaryPoints.end(); ++bpItr) {
          Json::Value vec = (*bpItr)["vec"];
          double dX = vec["0"].asDouble();
          double dY = vec["1"].asDouble();
          double dZ = vec["2"].asDouble();
          // double dX = -1 * vec["1"].asDouble();
          // double dY = vec["0"].asDouble();
          // double dZ = vec["2"].asDouble();

          coord_trans_api::local2lonlat(dX, dY, dZ, dRefLon, dRefLat, dRefAlt);

          hadmap::txPoint pt;
          pt.x = dX;
          pt.y = dY;
          pt.z = dZ;
          curve.addPoint(pt);
        }

        int nLaneMark = (*bItr)["m_laneMarkType"].asInt();

        if (nLaneMark == 0) {
          pBound->setLaneMark(hadmap::LANE_MARK_None);
        } else if (nLaneMark == 1) {
          pBound->setLaneMark(hadmap::LANE_MARK_Solid);
        } else if (nLaneMark == 2) {
          pBound->setLaneMark(hadmap::LANE_MARK_Broken);
        }

        pBound->setGeometry(curve);
        boundaries.insert(std::make_pair(pBound->getId(), pBound));
        vecBoundaries.push_back(pBound);
      }

      // lanes
      Json::Value& jLanes = (*itr)["m_lanes"];
      Json::ValueIterator lItr = jLanes.begin();
      for (; lItr != jLanes.end(); ++lItr) {
        hadmap::txLanePtr pLane(new hadmap::txLane());
        pLane->setRoadId((*lItr)["m_roadID"].asUInt64());
        pLane->setSectionId((*lItr)["m_sectionID"].asUInt64());
        pLane->setId((*lItr)["m_laneID"].asInt64());
        Json::Value jLanePoints = (*lItr)["m_points"];
        hadmap::txLineCurve curve;
        Json::ValueIterator lpItr = jLanePoints.begin();
        for (; lpItr != jLanePoints.end(); ++lpItr) {
          Json::Value vec = (*lpItr)["vec"];
          double dX = vec["0"].asDouble();
          double dY = vec["1"].asDouble();
          double dZ = vec["2"].asDouble();
          // double dX = -1 * vec["1"].asDouble();
          // double dY = vec["0"].asDouble();
          // double dZ = vec["2"].asDouble();

          coord_trans_api::local2lonlat(dX, dY, dZ, dRefLon, dRefLat, dRefAlt);

          hadmap::txPoint pt;
          pt.x = dX;
          pt.y = dY;
          pt.z = dZ;
          curve.addPoint(pt);
        }
        uint64_t lBound = (*lItr)["m_leftBoundaryID"].asUInt64();
        std::unordered_map<int, hadmap::txLaneBoundaryPtr>::iterator bitr = boundaries.find(lBound);
        if (bitr == boundaries.end()) {
          SYSTEM_LOGGER_ERROR("find lane %u left boundary %u error!", pLane->getId(), lBound);
          continue;
        }
        pLane->setLeftBoundary(bitr->second);

        uint64_t rBound = (*lItr)["m_rightBoundaryID"].asUInt64();
        bitr = boundaries.find(rBound);
        if (bitr == boundaries.end()) {
          SYSTEM_LOGGER_ERROR("find lane %u right boundary %u error!", pLane->getId(), rBound);
          continue;
        }
        pLane->setRightBoundary(bitr->second);
        double laneWidth = (*lItr)["m_laneWidth"].asDouble();
        // pLane->setLaneWidth(laneWidth);

        pLane->setGeometry(curve);
        pSec->add(pLane);
      }

      road->addSection(hadmap::txSectionPtr(pSec));
    }
  }

  return 0;
}

int CParserHadmapJson::ParseCross(CCross* pCross, const char* strJson, double dRefLon, double dRefLat, double dRefAlt) {
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
        Json::Value jLinkPoints = (*llItr);
        Json::ValueIterator lpItr = jLinkPoints.begin();

        // CrossLinkLine ll;
        CLinkEntityV2 ll;
        for (; lpItr != jLinkPoints.end(); ++lpItr) {
          Json::Value vec = (*lpItr)["vec"];
          double dX = vec["0"].asDouble();
          double dY = vec["1"].asDouble();
          double dZ = vec["2"].asDouble();
          // double dX = -1 * vec["1"].asDouble();
          // double dY = vec["0"].asDouble();
          // double dZ = vec["2"].asDouble();

          coord_trans_api::local2lonlat(dX, dY, dZ, dRefLon, dRefLat, dRefAlt);

          CPoint3D pt(dX, dY, dZ);
          ll.m_points.push_back(pt);
          // pCross->m_links[i][j][k].push_back(pt);
        }
        fromToLinks.push_back(ll);
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

std::string CParserHadmapJson::ToJson(CMapScene* pScene) {
  if (!pScene) {
    SYSTEM_LOGGER_ERROR("ToJson error!");
    return "";
  }

  Json::Value root;
  // settings
  {
    Json::Value jSettings;
    jSettings["lon"] = pScene->RefLon();
    jSettings["lat"] = pScene->RefLat();
    jSettings["alt"] = pScene->RefAlt();
    root["settings"] = jSettings;
  }

  // striaghts
  CMapScene::StragihtRoads& straights = pScene->Straights();
  if (straights.size() > 0) {
    Json::Value jStraights;
    for (int i = 0; i < straights.size(); ++i) {
      Json::Value jStraight;
      jStraight["m_ID"] = straights[i]->m_id;
      jStraight["m_name"] = straights[i]->m_name;
      jStraight["m_height"] = straights[i]->m_height;
      jStraight["m_length"] = straights[i]->m_length;
      jStraight["m_width"] = straights[i]->m_width;
      jStraight["m_maxSpeed"] = straights[i]->m_maxSpeed;
      jStraight["m_laneWidth"] = straights[i]->m_widthOfLane;
      jStraight["m_numberOfLanes"] = straights[i]->m_numberOfLanes;
      jStraight["m_roadThickness"] = straights[i]->m_roadThickness;

      // Json::Value jPosition;
      for (int j = 0; j < 3; ++j) {
        // jPosition.append(straights[i]->m_position[i]);
        jStraight["m_position"].append(straights[i]->m_position[j]);
      }
      // jStraight["m_position"] = jPosition;

      // Json::Value jRotation;
      for (int j = 0; j < 3; ++j) {
        jStraight["m_rotation"].append(straights[i]->m_rotation[j]);
      }
      // jStraight["m_rotation"] = jRotation;

      // Json::Value jStart;
      for (int j = 0; j < 3; ++j) {
        jStraight["m_start"].append(straights[i]->m_start[j]);
      }
      // jStraight["m_start"] = jStart;

      // Json::Value jEnd;
      for (int j = 0; j < 3; ++j) {
        jStraight["m_end"].append(straights[i]->m_end[j]);
      }
      // jStraight["m_end"] = jEnd;

      // Json::Value jMatrix;
      for (int j = 0; j < 16; ++j) {
        jStraight["m_matrix"].append(straights[i]->m_matrix[j]);
      }
      // jStraight["m_matrix"] = jMatrix;

      jStraights.append(jStraight);
    }

    root["straights"] = jStraights;
  }

  // curves
  CMapScene::CurveRoads& curves = pScene->Curves();
  if (curves.size() > 0) {
    Json::Value jCurves;
    for (int i = 0; i < curves.size(); ++i) {
      Json::Value jCurve;
      jCurve["m_ID"] = curves[i]->m_id;
      jCurve["m_name"] = curves[i]->m_name;
      jCurve["m_height"] = curves[i]->m_height;
      jCurve["m_length"] = curves[i]->m_length;
      jCurve["m_width"] = curves[i]->m_width;
      jCurve["m_maxSpeed"] = curves[i]->m_maxSpeed;
      jCurve["m_laneWidth"] = curves[i]->m_widthOfLane;
      jCurve["m_numberOfLanes"] = curves[i]->m_numberOfLanes;
      jCurve["m_roadThickness"] = curves[i]->m_roadThickness;

      // Json::Value jPosition;
      for (int j = 0; j < 3; ++j) {
        // jPosition.append(straights[i]->m_position[i]);
        jCurve["m_position"].append(curves[i]->m_position[j]);
      }
      // jStraight["m_position"] = jPosition;

      // Json::Value jRotation;
      for (int j = 0; j < 3; ++j) {
        jCurve["m_rotation"].append(curves[i]->m_rotation[j]);
      }

      for (int j = 0; j < 16; ++j) {
        jCurve["m_matrix"].append(curves[i]->m_matrix[j]);
      }

      {
        for (int j = 0; j < curves[i]->m_controlPoints.size(); ++j) {
          Json::Value jCP;
          jCP.append(curves[i]->m_controlPoints[j].X());
          jCP.append(curves[i]->m_controlPoints[j].Y());
          jCP.append(curves[i]->m_controlPoints[j].Z());
          /*jCP["0"] = (curves[i]->m_controlPoints[j].X());
          jCP["1"] = (curves[i]->m_controlPoints[j].Y());
          jCP["2"] = (curves[i]->m_controlPoints[j].Z());
          Json::Value jVec;
          jVec["vec"] = jCP;*/
          jCurve["m_controlPoints"].append(jCP);
        }
      }

      jCurves.append(jCurve);
    }

    root["curves"] = jCurves;
  }

  // crosses
  CMapScene::Crosses& crosses = pScene->AllCrosses();
  if (crosses.size() > 0) {
    Json::Value jCrosses;
    for (int i = 0; i < crosses.size(); ++i) {
      Json::Value jCross;
      jCross["m_ID"] = crosses[i]->m_id;
      jCross["m_name"] = crosses[i]->m_name;
      jCross["m_height"] = crosses[i]->m_height;
      jCross["m_length"] = crosses[i]->m_length;
      jCross["m_width"] = crosses[i]->m_width;
      jCross["m_maxSpeed"] = crosses[i]->m_maxSpeed;
      /*jCross["m_laneWidth"] = crosses[i]->m_widthOfLane;
      jCross["m_numberOfLanes"] = crosses[i]->m_numberOfLanes;
      jCross["m_roadThickness"] = crosses[i]->m_roadThickness;*/

      // Json::Value jPosition;
      for (int j = 0; j < 3; ++j) {
        // jPosition.append(straights[i]->m_position[i]);
        jCross["m_position"].append(crosses[i]->m_position[j]);
      }
      // jStraight["m_position"] = jPosition;

      // Json::Value jRotation;
      for (int j = 0; j < 3; ++j) {
        jCross["m_rotation"].append(crosses[i]->m_rotation[j]);
      }
      // jStraight["m_rotation"] = jRotation;

      // Json::Value jMatrix;
      for (int j = 0; j < 16; ++j) {
        jCross["m_matrix"].append(crosses[i]->m_matrix[j]);
      }
      // jStraight["m_matrix"] = jMatrix;

      jCrosses.append(jCross);
    }

    root["crosses"] = jCrosses;
  }

  CMapScene::Groups& groups = pScene->AllGroups();
  if (groups.size() > 0) {
    Json::Value jGroups;
    for (int i = 0; i < groups.size(); ++i) {
      Json::Value jGroup;

      CGroupEntity::GroupChildren& childrens = groups[i]->Children();
      if (childrens.size() > 0) {
        Json::Value jChildren;
        for (int j = 0; j < childrens.size(); ++j) {
          jChildren.append(childrens[j]);
        }
        jGroup["m_children"] = jChildren;
      }

      CGroupEntity::LinkEntities& links = groups[i]->Links();
      if (links.size() > 0) {
        Json::Value jLinks;
        for (int j = 0; j < links.size(); ++j) {
          Json::Value jLink;
          jLink["m_fromID"] = links[j]->m_from_id;
          jLink["m_fromEnd"] = links[j]->m_from_end;
          jLink["m_toID"] = links[j]->m_to_id;
          jLink["m_toEnd"] = links[j]->m_to_end;

          jLinks.append(jLink);
        }

        jGroup["m_links"] = jLinks;
      }

      jGroups.append(jGroup);
    }
    root["groups"] = jGroups;
  }

  std::string strStyledResult = root.toStyledString();
  // Json::FastWriter writer;
  Json::StreamWriterBuilder swbuilder;
  std::string strResult = Json::writeString(swbuilder, root);

  Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "";
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream ss;
  writer->write(root, &ss);
  std::string strUnstyledResult = ss.str();
  return strStyledResult;
  // return strUnstyledResult;
}
