// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "mapengine/remote_search.h"
#include "mapengine/hadmap_codes.h"
#include "transmission/CJsonObject.hpp"
#include "transmission/json_parser.h"
#include "transmission/post_param.h"
#include "transmission/url_gen.h"

#include "common/map_util.h"
#include "structs/hadmap_object.h"
#include "structs/hadmap_objgeom.h"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <set>
#include <unordered_map>

namespace hadmap {
RemoteSearch::RemoteSearch(MAP_DATA_TYPE type, const std::string& name)
    : dataType(type), dbName(name), pTrans(new TransInterface) {
  optInfo = "Connect Success";
}

RemoteSearch::~RemoteSearch() {
  optInfo = "Disconnect";
  if (pTrans != NULL) delete pTrans;
}

int RemoteSearch::getTopoLanes(txLanePtr curLanePtr, const txPoint& tagPoint, const int& topoFlag, txLanes& topoLanes) {
  PointVec envelope;
  double offset = 1.0 / 111000.0;
  envelope.push_back(txPoint(tagPoint.x - offset, tagPoint.y - offset, 0.0));
  envelope.push_back(txPoint(tagPoint.x + offset, tagPoint.y + offset, 0.0));

  txLanes lanes;
  if (TX_HADMAP_DATA_OK == getLanes(envelope, lanes)) {
    topoLanes.clear();
    for (auto& lanePtr : lanes) {
      if (lanePtr->getRoadId() == curLanePtr->getRoadId() && lanePtr->getSectionId() == curLanePtr->getSectionId()) {
        continue;
      }
      if (lanePtr->getGeometry() == NULL || lanePtr->getGeometry()->empty()) continue;
      txPoint startP;
      if (lanePtr->getId() > 0) {
        if (topoFlag == 1) {
          startP = lanePtr->getGeometry()->getEnd();
        } else {
          startP = lanePtr->getGeometry()->getStart();
        }
      } else {
        if (topoFlag == 1) {
          startP = lanePtr->getGeometry()->getStart();
        } else {
          startP = lanePtr->getGeometry()->getEnd();
        }
      }

      Point3d a(tagPoint.x, tagPoint.y, 0.0);
      Point3d b(startP.x, startP.y, 0.0);
      if (map_util::distanceBetweenPoints(a, b, true) < 0.5) topoLanes.push_back(lanePtr);
    }
  }
  return topoLanes.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

bool RemoteSearch::getRoads(const std::string& url, const bool& wholeData, txRoads& roads) {
  std::string jsonData;
  if (pTrans->get(url, jsonData)) {
    if (JsonParser::parseRoad(jsonData, roads)) {
      if (wholeData) {
        txSections sections;
        if (JsonParser::parseSection(jsonData, sections)) {
          std::unordered_map<roadpkid, txRoadPtr> roadMap;
          for (auto& curRoadPtr : roads) roadMap.insert(std::make_pair(curRoadPtr->getId(), curRoadPtr));
          for (auto& curSecPtr : sections) roadMap[curSecPtr->getRoadId()]->addSection(curSecPtr);
          for (auto itr = roadMap.begin(); itr != roadMap.end(); ++itr) itr->second->bindRoadPtr();
          return true;
        } else {
          return false;
        }
        /*
        for ( auto& roadPtr : roads )
        {
                txSections sections;
                txSectionId sectionId( roadPtr->getId(), SECT_PKID_INVALID );
                if ( getSections( sectionId, sections ) )
                {
                        roadPtr->setSections( sections );
                        roadPtr->bindRoadPtr();
                }
                else
                        return false;}
        }
        */
      }
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool RemoteSearch::connSuccess() { return true; }

bool RemoteSearch::saveMap(const int type, std::string filePath) { return false; }

std::string RemoteSearch::getLastOptInfo() { return optInfo; }

int RemoteSearch::getLane(const hadmap::txPoint& loc, double radius, hadmap::txLanePtr& lanePtr) {
  std::string baseUrl = pTrans->location();
  std::string url = UrlGen(baseUrl).db(dbName).getLane().center(loc.x, loc.y).url();
  std::string jsonData;
  if (pTrans->get(url, jsonData)) {
    txLanes lanes;
    if (JsonParser::parseLane(jsonData, lanes)) {
      lanePtr = lanes.front();
      return TX_HADMAP_DATA_OK;
    } else {
      return TX_HADMAP_DATA_EMPTY;
    }
  }

  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getLanes(const hadmap::txLaneId& laneId, hadmap::txLanes& lanes) {
  std::string baseUrl = pTrans->location();
  std::string url = UrlGen(baseUrl)
                        .db(dbName)
                        .getLane()
                        .roadId(laneId.roadId)
                        .sectionId(laneId.sectionId)
                        .laneId(laneId.laneId)
                        .url();

  std::string jsonData;
  if (pTrans->get(url, jsonData)) {
    lanes.clear();

    JsonParser::parseLane(jsonData, lanes);

    return lanes.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
  }

  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getLanes(const hadmap::PointVec& envelope, hadmap::txLanes& lanes) {
  std::string baseUrl = pTrans->location();
  std::string url =
      UrlGen(baseUrl).db(dbName).getLane().envelope(envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y).url();
  std::string jsonData;
  if (pTrans->get(url, jsonData)) {
    lanes.clear();
    JsonParser::parseLane(jsonData, lanes);

    if (lanes.empty()) {
      return TX_HADMAP_DATA_EMPTY;
    } else {
      return TX_HADMAP_DATA_OK;
    }
  } else {
    return TX_HADMAP_DATA_ERROR;
  }
}

int RemoteSearch::getNextLanes(txLanePtr curLanePtr, txLanes& nextLanes) {
  if (curLanePtr->getGeometry() == NULL || curLanePtr->getGeometry()->empty()) return TX_HADMAP_PARAM_ERROR;

  txPoint endP;
  if (curLanePtr->getId() < 0) {
    endP = curLanePtr->getGeometry()->getEnd();
  } else {
    endP = curLanePtr->getGeometry()->getStart();
  }

  return getTopoLanes(curLanePtr, endP, 1, nextLanes);
}

int RemoteSearch::getPrevLanes(txLanePtr curLanePtr, txLanes& prevLanes) {
  if (curLanePtr->getGeometry() == NULL || curLanePtr->getGeometry()->empty()) return TX_HADMAP_PARAM_ERROR;

  txPoint startP;
  if (curLanePtr->getId() < 0) {
    startP = curLanePtr->getGeometry()->getStart();
  } else {
    startP = curLanePtr->getGeometry()->getEnd();
  }

  return getTopoLanes(curLanePtr, startP, -1, prevLanes);
}

int RemoteSearch::getLeftLane(txLanePtr curLanePtr, txLanePtr& leftLanePtr) {
  txLaneId curLaneId = curLanePtr->getTxLaneId();
  if (curLaneId.laneId > 0) {
    curLaneId.laneId -= 1;
    if (curLaneId.laneId == 0) curLaneId.laneId = -1;
  } else {
    curLaneId.laneId += 1;
    if (curLaneId.laneId == 0) curLaneId.laneId = 1;
  }

  txLanes lanes;
  if (TX_HADMAP_DATA_OK == getLanes(curLaneId, lanes)) {
    leftLanePtr = lanes.front();
    return TX_HADMAP_DATA_OK;
  } else {
    return TX_HADMAP_DATA_EMPTY;
  }
}

int RemoteSearch::getRightLane(txLanePtr curLanePtr, txLanePtr& rightLanePtr) {
  txLaneId curLaneId = curLanePtr->getTxLaneId();
  if (curLaneId.laneId > 0) {
    curLaneId.laneId += 1;
  } else {
    curLaneId.laneId -= 1;
  }

  txLanes lanes;
  if (TX_HADMAP_DATA_OK == getLanes(curLaneId, lanes)) {
    rightLanePtr = lanes.front();
    return TX_HADMAP_DATA_OK;
  } else {
    return TX_HADMAP_DATA_EMPTY;
  }
}

int RemoteSearch::getBoundaryMaxId(hadmap::laneboundarypkid& boundaryMaxId) {
  std::string baseUrl = pTrans->location();
  std::string url = UrlGen(baseUrl).db(dbName).getBoundaryMaxId().url();

  std::string jsonData;
  if (pTrans->get(url, jsonData)) {
    neb::CJsonObject jsonParser;
    jsonParser.Parse(jsonData);

    std::string id;
    jsonParser.Get("maxId", id);

    boundaryMaxId = laneboundarypkid(std::stoull(id));
    return TX_HADMAP_DATA_OK;
  }

  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getBoundaries(const std::vector<hadmap::laneboundarypkid>& ids,
                                hadmap::txLaneBoundaries& boundaries) {
  std::string baseUrl = pTrans->location();
  std::string url = UrlGen(baseUrl).db(dbName).getBoundary().url();

  std::vector<PostParamPtr> params;
  for (auto& id : ids) params.push_back(PostParamPtr(new BoundaryIdParam(id)));
  std::string postJson;
  generatePostJson(params, postJson);

  std::string jsonData;
  if (pTrans->post(url, postJson, jsonData)) {
    boundaries.clear();

    JsonParser::parseBoundary(jsonData, boundaries);

    return boundaries.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
  }

  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getSections(const hadmap::txSectionId& sectionId, hadmap::txSections& sections) {
  std::string baseUrl = pTrans->location();
  std::string url =
      UrlGen(baseUrl).db(dbName).getSection().roadId(sectionId.roadId).sectionId(sectionId.sectionId).url();

  std::string jsonData;
  if (pTrans->get(url, jsonData)) {
    sections.clear();

    if (JsonParser::parseSection(jsonData, sections)) {
      return TX_HADMAP_DATA_OK;
    } else {
      return TX_HADMAP_DATA_EMPTY;
    }

    /*
    txLanes lanes;
    txLaneBoundaries boundaries;
    if ( JsonParser::parseLane( jsonData, lanes ) && \
                    JsonParser::parseBoundary( jsonData, boundaries ) )
    {
            std::unordered_map< laneboundarypkid, size_t > boundaryMap;
            for ( size_t i = 0; i < boundaries.size(); ++ i )
                    boundaryMap.insert( std::make_pair( boundaries[i]->getId(), i ) );

            std::map< sectionpkid, std::vector< txLanePtr > > laneMap;
            for ( auto& curLanePtr : lanes )
            {
                    if ( laneMap.find( curLanePtr->getSectionId() ) == laneMap.end() ){
                            laneMap.insert( std::make_pair( curLanePtr->getSectionId(), std::vector< txLanePtr >() ) );}
                    curLanePtr->setLeftBoundary( boundaries[ boundaryMap[ curLanePtr->getLeftBoundaryId() ] ] );
                    curLanePtr->setRightBoundary( boundaries[ boundaryMap[ curLanePtr->getRightBoundaryId() ] ] );
                    laneMap[ curLanePtr->getSectionId() ].push_back( curLanePtr );
            }

            for ( auto itr = laneMap.begin(); itr != laneMap.end(); ++ itr )
            {
                    std::sort( itr->second.begin(), itr->second.end(), \
                                    []( txLanePtr a, txLanePtr b )->bool{ return a->getId() > b->getId(); } );

                    txSectionPtr secPtr( new txSection );
                    secPtr->setId( itr->first )
                               .setRoadId( itr->second.front()->getRoadId() )
                               .setLanes( itr->second );
                    sections.push_back( secPtr );
            }
    }

    return sections.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
    */
  }

  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getRoadMaxId(roadpkid& roadMaxId) {
  std::string baseUrl = pTrans->location();
  std::string url = UrlGen(baseUrl).db(dbName).getRoadMaxId().url();

  std::string jsonData;
  if (pTrans->get(url, jsonData)) {
    neb::CJsonObject jsonParser;
    jsonParser.Parse(jsonData);

    std::string id;
    jsonParser.Get("maxId", id);
    roadMaxId = roadpkid(std::stoull(id));

    return TX_HADMAP_DATA_OK;
  }

  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getRoad(const roadpkid& roadId, const bool& wholeData, hadmap::txRoadPtr& roadPtr) {
  std::string baseUrl = pTrans->location();
  std::string url = UrlGen(baseUrl).db(dbName).getRoad().roadId(roadId).url();
  if (wholeData) url = UrlGen(baseUrl).db(dbName).getRoad().roadId(roadId).wholeData().url();

  txRoads roads;
  if (getRoads(url, wholeData, roads)) {
    if (roads.empty()) return TX_HADMAP_DATA_EMPTY;

    roadPtr = roads.front();
    return TX_HADMAP_DATA_OK;
  }
  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getRoads(const hadmap::PointVec& envelope, const bool& wholeData, hadmap::txRoads& roads) {
  std::string baseUrl = pTrans->location();
  std::string url =
      UrlGen(baseUrl).db(dbName).getRoad().envelope(envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y).url();
  if (wholeData)
    url = UrlGen(baseUrl)
              .db(dbName)
              .getRoad()
              .envelope(envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y)
              .wholeData()
              .url();

  if (getRoads(url, wholeData, roads)) {
    return roads.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
  } else {
    return TX_HADMAP_DATA_ERROR;
  }
}

int RemoteSearch::getRoads(const std::vector<roadpkid>& rids, const bool& wholeData, txRoads& roads) {
  std::string baseUrl = pTrans->location();
  std::string url = UrlGen(baseUrl).db(dbName).getRoad().url();
  if (wholeData) url = UrlGen(baseUrl).db(dbName).getRoad().wholeData().url();

  std::vector<PostParamPtr> params;
  for (auto& id : rids) params.push_back(PostParamPtr(new RoadIdParam(id)));
  std::string postJson;
  generatePostJson(params, postJson);

  std::string jsonData;
  roads.clear();
  if (pTrans->post(url, postJson, jsonData)) {
    if (JsonParser::parseRoad(jsonData, roads)) {
      if (wholeData) {
        txSections sections;
        if (JsonParser::parseSection(jsonData, sections)) {
          std::unordered_map<roadpkid, txRoadPtr> roadMap;
          for (auto& curRoadPtr : roads) roadMap.insert(std::make_pair(curRoadPtr->getId(), curRoadPtr));
          for (auto& curSecPtr : sections) roadMap[curSecPtr->getRoadId()]->addSection(curSecPtr);
          for (auto itr = roadMap.begin(); itr != roadMap.end(); ++itr) itr->second->bindRoadPtr();
        }
      }
    }
  }
  return roads.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
}

int RemoteSearch::getRoads(const bool& wholeData, hadmap::txRoads& roads) {
  std::string baseUrl = pTrans->location();
  std::string url = UrlGen(baseUrl).db(dbName).getAllRoad().url();
  if (wholeData) url = UrlGen(baseUrl).db(dbName).getAllRoad().wholeData().url();

  if (getRoads(url, wholeData, roads)) {
    return roads.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
  } else {
    return TX_HADMAP_DATA_ERROR;
  }
}

int RemoteSearch::getJuncInfo(const roadpkid& entraceRoadId, const roadpkid& exitRoadId, hadmap::txRoads& roads,
                              hadmap::txLaneLinks& links) {
  return TX_HADMAP_HANDLE_ERROR;
}

int RemoteSearch::getJuncInfo(const lanelinkpkid& linkId, hadmap::txRoads& roads, hadmap::txLaneLinks& links) {
  return TX_HADMAP_HANDLE_ERROR;
}

int RemoteSearch::getLaneLinks(const roadpkid& fromRoadId, const roadpkid& toRoadId, hadmap::txLaneLinks& lanelinks) {
  std::string baseUrl = pTrans->location();
  std::string url = UrlGen(baseUrl)
                        .db(dbName)
                        .getLink()
                        .topoId(txLaneId(fromRoadId, SECT_PKID_INVALID, LANE_PKID_INVALID),
                                txLaneId(toRoadId, SECT_PKID_INVALID, LANE_PKID_INVALID))
                        .url();

  std::string jsonData;
  if (pTrans->get(url, jsonData)) {
    lanelinks.clear();

    JsonParser::parseLink(jsonData, lanelinks);

    return lanelinks.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
  }

  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getLaneLinks(const hadmap::PointVec& envelope, hadmap::txLaneLinks& lanelinks) {
  std::string baseUrl = pTrans->location();
  std::string url =
      UrlGen(baseUrl).db(dbName).getLink().envelope(envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y).url();

  std::string jsonData;
  if (pTrans->get(url, jsonData)) {
    lanelinks.clear();

    JsonParser::parseLink(jsonData, lanelinks);

    if (lanelinks.empty()) {
      return TX_HADMAP_DATA_EMPTY;
    } else {
      return TX_HADMAP_DATA_OK;
    }
  } else {
    return TX_HADMAP_DATA_ERROR;
  }
}

int RemoteSearch::getLaneLinks(const std::vector<roadpkid>& roadIds, hadmap::txLaneLinks& lanelinks) {
  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getLaneLinkMaxId(lanelinkpkid& linkMaxId) {
  std::string baseUrl = pTrans->location();
  std::string url = UrlGen(baseUrl).db(dbName).getLinkMaxId().url();

  std::string jsonData;
  if (pTrans->get(url, jsonData)) {
    neb::CJsonObject jsonParser;
    jsonParser.Parse(jsonData);

    std::string id;
    jsonParser.Get("maxId", id);
    linkMaxId = lanelinkpkid(std::stoull(id));

    return TX_HADMAP_DATA_OK;
  }

  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getObjects(const hadmap::PointVec& envelope, const std::vector<hadmap::OBJECT_TYPE>& types,
                             hadmap::txObjects& objects) {
  std::string baseUrl = pTrans->location();
  std::string url =
      UrlGen(baseUrl).db(dbName).getObject().envelope(envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y).url();

  std::vector<PostParamPtr> params;
  for (auto& objType : types) params.push_back(PostParamPtr(new ObjTypeParam(objType)));
  std::string paramJson;
  generatePostJson(params, paramJson);

  std::string jsonData;
  if (pTrans->post(url, paramJson, jsonData)) {
    objects.clear();

    JsonParser::parseObj(jsonData, objects);

    return objects.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
  }

  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getObjects(const std::vector<txLaneId>& laneIds, const std::vector<OBJECT_TYPE>& types,
                             txObjects& objects) {
  std::string baseUrl = pTrans->location();
  std::string url = UrlGen(baseUrl).db(dbName).getObject().url();

  std::vector<PostParamPtr> params;
  for (auto& laneId : laneIds) params.push_back(PostParamPtr(new LaneIdParam(laneId)));
  for (auto& objType : types) params.push_back(PostParamPtr(new ObjTypeParam(objType)));
  std::string paramJson;
  generatePostJson(params, paramJson);

  std::string jsonData;
  if (pTrans->post(url, paramJson, jsonData)) {
    objects.clear();

    JsonParser::parseObj(jsonData, objects);

    return objects.empty() ? TX_HADMAP_DATA_EMPTY : TX_HADMAP_DATA_OK;
  }

  return TX_HADMAP_DATA_ERROR;
}

int RemoteSearch::getObjects(const PointVec& envelope, const std::vector<OBJECT_SUB_TYPE>& types, txObjects& objects) {
  return TX_HADMAP_DATA_EMPTY;
}
int RemoteSearch::getObjects(const std::vector<txLaneId>& laneIds, const std::vector<OBJECT_SUB_TYPE>& types,
                             txObjects& objects) {
  return TX_HADMAP_DATA_EMPTY;
}
int RemoteSearch::getJunctions(hadmap::txJunctions& junctions) { return TX_HADMAP_DATA_EMPTY; }
int RemoteSearch::getJunction(junctionpkid jid, hadmap::txJunctionPtr& junction) { return TX_HADMAP_DATA_EMPTY; }
int RemoteSearch::insertRoads(const hadmap::txRoads& roads) { return TX_HADMAP_DATA_NOT_SUPPORT; }
int RemoteSearch::updateRoads(const hadmap::txRoads& roads) { return TX_HADMAP_DATA_NOT_SUPPORT; }
int RemoteSearch::deleteRoads(const hadmap::txRoads& roads) { return TX_HADMAP_DATA_NOT_SUPPORT; }
int RemoteSearch::insertLaneLinks(const hadmap::txLaneLinks& lanelinks) { return TX_HADMAP_DATA_NOT_SUPPORT; }
int RemoteSearch::updateLaneLinks(const hadmap::txLaneLinks& lanelinks) { return TX_HADMAP_DATA_NOT_SUPPORT; }
int RemoteSearch::deleteLaneLinks(const hadmap::txLaneLinks& lanelinks) { return TX_HADMAP_DATA_NOT_SUPPORT; }
int RemoteSearch::getRoadLink(const hadmap::roadpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr) {
  return TX_HADMAP_DATA_NOT_SUPPORT;
}
hadmap::txPoint RemoteSearch::getRefPoint() { return hadmap::txPoint(); }
int RemoteSearch::getHeader(hadmap::txOdHeaderPtr& headerPtr) { return TX_HADMAP_DATA_NOT_SUPPORT; }
int RemoteSearch::insertObjects(const hadmap::txObjects& objects) { return TX_HADMAP_DATA_NOT_SUPPORT; }
int RemoteSearch::updateObjects(const hadmap::txObjects& objects) { return TX_HADMAP_DATA_NOT_SUPPORT; }
int RemoteSearch::deleteObjects(const hadmap::txObjects& objects) { return TX_HADMAP_DATA_NOT_SUPPORT; }
}  // namespace hadmap
