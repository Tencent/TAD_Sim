// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routingmap/mapdata_load.h"
#include "routingmap/routing_utils.h"

#include <float.h>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <set>
#include <unordered_set>

#include "structs/hadmap_curve.h"
#include "structs/hadmap_junctionroad.h"

#define SINGLE_PASS_ON

namespace hadmap {
MapDataLoad::MapDataLoad(txMapHandle* pH, IdGeneratorPtr idPtr) : pHandle(pH), idGenPtr(idPtr) {}

MapDataLoad::~MapDataLoad() {
  if (pHandle != NULL) hadmap::hadmapClose(&pHandle);
}

bool MapDataLoad::updateRoads(const hadmap::txRoads& curLoadRoads) {
  curAddedRoads.clear();
  curRemovedRoads.clear();
  std::unordered_set<roadpkid> curLoadRoadIds;

  // confirm added roads
  for (auto roadPtr : curLoadRoads) {
    curLoadRoadIds.insert(roadPtr->getId());
    if (curRoads.find(roadPtr->getId()) == curRoads.end()) {
      curAddedRoads.push_back(roadPtr);
      curRoads.insert(std::make_pair(roadPtr->getId(), roadPtr));
    }
  }

  // confirm removed roads
  for (auto itr = curRoads.begin(); itr != curRoads.end(); ++itr) {
    if (curLoadRoadIds.find(itr->first) == curLoadRoadIds.end() && routeRoadId.find(itr->first) == routeRoadId.end()) {
      // if ( curLoadRoadIds.find( itr->first ) == curLoadRoadIds.end() )
      curRemovedRoads.push_back(itr->second);
    }
  }

  // remove old roads
  for (auto roadPtr : curRemovedRoads) curRoads.erase(roadPtr->getId());

  return true;
}

bool MapDataLoad::updateLinks(const hadmap::txLaneLinks& curLoadLinks) {
  curAddedLinks.clear();
  curRemovedLinks.clear();
  std::unordered_set<lanelinkpkid> curLoadLinkIds;

  for (auto linkPtr : curLoadLinks) {
    curLoadLinkIds.insert(linkPtr->getId());
    if (curLinks.find(linkPtr->getId()) == curLinks.end()) {
      curAddedLinks.push_back(linkPtr);
      curLinks.insert(std::make_pair(linkPtr->getId(), linkPtr));
    }
  }

  for (auto itr = curLinks.begin(); itr != curLinks.end(); ++itr) {
    if (curLoadLinkIds.find(itr->first) == curLoadLinkIds.end() &&
        (routeRoadId.find(itr->second->fromRoadId()) == routeRoadId.end() ||
         routeRoadId.find(itr->second->toRoadId()) == routeRoadId.end())) {
      // if ( curLoadLinkIds.find( itr->first ) == curLoadLinkIds.end() )
      curRemovedLinks.push_back(itr->second);
    }
  }

  for (auto linkPtr : curRemovedLinks) curLinks.erase(linkPtr->getId());

  return true;
}

bool MapDataLoad::updateJunctions(const hadmap::txJunctions& curLoadJunctions) {
  curAddedJunctions.clear();
  curRemovedJunctions.clear();
  std::unordered_set<junctionpkid> curLoadJuncIds;

  for (auto juncPtr : curLoadJunctions) {
    curLoadJuncIds.insert(juncPtr->getId());
    if (curJunctions.find(juncPtr->getId()) == curJunctions.end()) {
      curAddedJunctions.push_back(juncPtr);
      curJunctions.insert(std::make_pair(juncPtr->getId(), juncPtr));
    }
  }

  for (auto itr = curJunctions.begin(); itr != curJunctions.end(); ++itr) {
    if (curLoadJuncIds.find(itr->first) == curLoadJuncIds.end()) curRemovedJunctions.push_back(itr->second);
  }

  for (auto juncPtr : curRemovedJunctions) curJunctions.erase(juncPtr->getId());
  return true;
}

bool MapDataLoad::updateObjects(const hadmap::txObjects& curLoadObjects) {
  curAddedObjects.clear();
  curRemovedObjects.clear();
  std::unordered_set<objectpkid> curLoadObjIds;

  for (auto& objPtr : curLoadObjects) {
    curLoadObjIds.insert(objPtr->getId());
    if (curObjects.find(objPtr->getId()) == curObjects.end()) {
      curAddedObjects.push_back(objPtr);
      curObjects.insert(std::make_pair(objPtr->getId(), objPtr));
    }
  }

  for (auto itr = curObjects.begin(); itr != curObjects.end(); ++itr) {
    if (curLoadObjIds.find(itr->first) == curLoadObjIds.end()) curRemovedObjects.push_back(itr->second);
  }

  for (auto objPtr : curRemovedObjects) curObjects.erase(objPtr->getId());
  return true;
}

bool MapDataLoad::isJuncLink(const txLaneLinks& links) {
  bool r = false;
  for (auto linkPtr : links) {
    if (linkPtr->getGeometry() != NULL && linkPtr->getGeometry()->getLength() > 2.0) r = true;
  }
  return r;
}

bool MapDataLoad::loadLinksByEnvelope(const PointVec& envelope,
                                      std::unordered_map<std::string, txLaneLinks>& directLinks,
                                      std::unordered_map<std::string, txLaneLinks>& junctionLinks) {
  txLaneLinks links;
  if (TX_HADMAP_DATA_OK == getLaneLinks(pHandle, envelope, links)) {
    for (auto& curLinkPtr : links) {
      /*
      roadpkid fromRoadId = curLinkPtr->fromRoadId();
      if ( curLinkPtr->fromLaneId() > 0 )
              fromRoadId = idGenPtr->generateReverseRoadId( RoutingUtils::reverseRoadIndex( fromRoadId ) );
      roadpkid toRoadId = curLinkPtr->toRoadId();
      if ( curLinkPtr->toLaneId() > 0 )
              toRoadId = idGenPtr->generateReverseRoadId( RoutingUtils::reverseRoadIndex( toRoadId ) );
      std::string index = RoutingUtils::linkIndex( fromRoadId, toRoadId );
      */
      std::string index = RoutingUtils::linkIndex(curLinkPtr->fromRoadId(), curLinkPtr->toRoadId());
      if (directLinks.find(index) == directLinks.end()) directLinks.insert(std::make_pair(index, txLaneLinks()));
      directLinks[index].push_back(curLinkPtr);
    }
    for (auto itr = directLinks.begin(); itr != directLinks.end(); ++itr) {
      if (isJuncLink(itr->second)) junctionLinks.insert(std::make_pair(itr->first, itr->second));
    }
    for (auto itr = junctionLinks.begin(); itr != junctionLinks.end(); ++itr) directLinks.erase(itr->first);
  }
  return !directLinks.empty() || !junctionLinks.empty();
}

bool MapDataLoad::loadLinksByRoad(const txRoads& roads, std::unordered_map<std::string, txLaneLinks>& directLinks,
                                  std::unordered_map<std::string, txLaneLinks>& junctionLinks) {
  /*
  std::unordered_set< lanelinkpkid > uselessLinkIds;
  for ( auto roadPtr : roads )
  {
          txLaneLinks links;
          txLaneLinks toLinks;
          getLaneLinks( pHandle, roadPtr->getId(), ROAD_PKID_INVALID, links );
          getLaneLinks( pHandle, ROAD_PKID_INVALID, roadPtr->getId(), toLinks );
          links.insert( links.end(), toLinks.begin(), toLinks.end() );
          for ( auto linkPtr : links )
          {
                  if ( uselessLinkIds.find( linkPtr->getId() ) == uselessLinkIds.end() )
                          uselessLinkIds.insert( linkPtr->getId() );
                  else
                          continue;
                  std::string index = RoutingUtils::linkIndex( linkPtr->fromRoadId(), linkPtr->toRoadId() );
                  if ( directLinks.find( index ) == directLinks.end() )
                          directLinks.insert( std::make_pair( index, txLaneLinks() ) );
                  directLinks[ index ].push_back( linkPtr );
          }
  }
  */
  std::vector<roadpkid> roadIds;
  for (auto& roadPtr : roads) roadIds.push_back(roadPtr->getId());
  txLaneLinks links;
  if (TX_HADMAP_DATA_OK == getLaneLinks(pHandle, roadIds, links)) {
    for (auto& linkPtr : links) {
      std::string index = RoutingUtils::linkIndex(linkPtr->fromRoadId(), linkPtr->toRoadId());
      if (directLinks.find(index) == directLinks.end()) directLinks.insert(std::make_pair(index, txLaneLinks()));
      directLinks[index].push_back(linkPtr);
    }
  }
  for (auto itr = directLinks.begin(); itr != directLinks.end(); ++itr) {
    if (isJuncLink(itr->second)) junctionLinks.insert(std::make_pair(itr->first, itr->second));
  }
  for (auto itr = junctionLinks.begin(); itr != junctionLinks.end(); ++itr) directLinks.erase(itr->first);
  return !directLinks.empty() || !junctionLinks.empty();
}

bool MapDataLoad::isMockRoadExisted(const std::string& mockRoadIndex) {
  std::unique_lock<std::mutex> lck(mockRoadIdMapMutex);
  if (mockRoadIdMap.find(mockRoadIndex) == mockRoadIdMap.end()) return false;
  if (curRoads.find(mockRoadIdMap[mockRoadIndex]) == curRoads.end()) return false;
  return true;
}

std::vector<size_t> MapDataLoad::minDisLaneLinks(const std::set<lanepkid>& fromLaneIds,
                                                 std::set<lanepkid>& usedFromLaneIds,
                                                 const std::set<lanepkid>& toLaneIds, std::set<lanepkid>& usedToLaneIds,
                                                 const txLaneLinks& lanelinks, std::set<size_t>& usedLinkIndex) {
  bool complete = true;
  for (auto itr = fromLaneIds.begin(); itr != fromLaneIds.end(); ++itr)
    if (usedFromLaneIds.find(*itr) == usedFromLaneIds.end()) complete = false;
  for (auto itr = toLaneIds.begin(); itr != toLaneIds.end(); ++itr)
    if (usedToLaneIds.find(*itr) == usedToLaneIds.end()) complete = false;
  if (complete) {
    return std::vector<size_t>();
  } else {
    double dis = DBL_MAX;
    size_t minIndex = 0;
    for (size_t i = 0; i < lanelinks.size(); ++i) {
      if (usedLinkIndex.find(i) != usedLinkIndex.end()) continue;
      hadmap::lanepkid fromId = lanelinks[i]->fromLaneId();
      hadmap::lanepkid toId = lanelinks[i]->toLaneId();
      if (fromLaneIds.find(fromId) == fromLaneIds.end() || toLaneIds.find(toId) == toLaneIds.end()) continue;
      if (dis > lanelinks[i]->getGeometry()->getLength()) {
        dis = lanelinks[i]->getGeometry()->getLength();
        minIndex = i;
      }
    }
    hadmap::lanepkid fromId = lanelinks[minIndex]->fromLaneId();
    hadmap::lanepkid toId = lanelinks[minIndex]->toLaneId();
    std::set<hadmap::lanepkid> leftFromLaneIds, leftToLaneIds, rightFromLaneIds, rightToLaneIds;
    leftFromLaneIds.insert(fromId);
    leftToLaneIds.insert(toId);
    rightFromLaneIds.insert(fromId);
    rightToLaneIds.insert(toId);
    for (auto itr = fromLaneIds.begin(); itr != fromLaneIds.end(); ++itr) {
      if (*itr > 0) {
        if (*itr < fromId) {
          leftFromLaneIds.insert(*itr);
        } else {
          rightFromLaneIds.insert(*itr);
        }
      }
      if (*itr < 0) {
        if (*itr > fromId) {
          leftFromLaneIds.insert(*itr);
        } else {
          rightFromLaneIds.insert(*itr);
        }
      }
    }
    for (auto itr = toLaneIds.begin(); itr != toLaneIds.end(); ++itr) {
      if (*itr > 0) {
        if (*itr < toId) {
          leftToLaneIds.insert(*itr);
        } else {
          rightToLaneIds.insert(*itr);
        }
      }
      if (*itr < 0) {
        if (*itr > toId) {
          leftToLaneIds.insert(*itr);
        } else {
          rightToLaneIds.insert(*itr);
        }
      }
    }
    usedFromLaneIds.insert(fromId);
    usedToLaneIds.insert(toId);
    usedLinkIndex.insert(minIndex);
    std::vector<size_t> leftIndex =
        minDisLaneLinks(leftFromLaneIds, usedFromLaneIds, leftToLaneIds, usedToLaneIds, lanelinks, usedLinkIndex);
    std::vector<size_t> rightIndex =
        minDisLaneLinks(rightFromLaneIds, usedFromLaneIds, rightToLaneIds, usedToLaneIds, lanelinks, usedLinkIndex);
    leftIndex.push_back(minIndex);
    leftIndex.insert(leftIndex.end(), rightIndex.begin(), rightIndex.end());
    return leftIndex;
  }
}

bool MapDataLoad::generateMockRoad(const txRoadPtr& fromRoadPtr, const txRoadPtr& toRoadPtr,
                                   const txLaneLinks& lanelinks, txRoadPtr& roadPtr) {
  if (lanelinks.empty() || fromRoadPtr == NULL || toRoadPtr == NULL) {
    return false;
  } else {
    std::set<lanepkid> fromLaneIds, toLaneIds, fromUsedIds, toUsedIds;
    std::set<size_t> usedLinkIndex;
    for (size_t i = 0; i < lanelinks.size(); ++i) {
      fromLaneIds.insert(lanelinks[i]->fromLaneId());
      toLaneIds.insert(lanelinks[i]->toLaneId());
    }
    std::vector<size_t> targetIndex;

#ifdef SINGLE_PASS_ON
    targetIndex = minDisLaneLinks(fromLaneIds, fromUsedIds, toLaneIds, toUsedIds, lanelinks, usedLinkIndex);

    // clear non-unique link
    std::unordered_map<hadmap::lanepkid, size_t> fromLaneCount, toLaneCount;
    std::vector<int> flag(targetIndex.size(), 0);
    int count = targetIndex.size();
    while (count > 0) {
      for (size_t i = 0; i < targetIndex.size(); ++i) {
        if (flag[i] != 0) continue;
        txLaneLinkPtr linkPtr = lanelinks[targetIndex[i]];
        if (fromLaneCount.find(linkPtr->fromLaneId()) == fromLaneCount.end()) {
          fromLaneCount.insert(std::make_pair(linkPtr->fromLaneId(), 1));
        } else {
          fromLaneCount[linkPtr->fromLaneId()] += 1;
        }
        if (toLaneCount.find(linkPtr->toLaneId()) == toLaneCount.end()) {
          toLaneCount.insert(std::make_pair(linkPtr->toLaneId(), 1));
        } else {
          toLaneCount[linkPtr->toLaneId()] += 1;
        }
      }
      for (size_t i = 0; i < targetIndex.size(); ++i) {
        if (flag[i] != 0) continue;
        txLaneLinkPtr linkPtr = lanelinks[targetIndex[i]];
        if (fromLaneCount[linkPtr->fromLaneId()] == 1 || toLaneCount[linkPtr->toLaneId()] == 1) {
          flag[i] = 1;
          count--;
        }
      }
      if (count == 0) break;

      double maxDis = 0;
      size_t maxIndex = 0;
      for (size_t i = 0; i < targetIndex.size(); ++i)
        if (flag[i] == 0) {
          double len = lanelinks[targetIndex[i]]->getGeometry()->getLength();
          if (len > maxDis) {
            maxDis = len;
            maxIndex = i;
          }
        }
      flag[maxIndex] = -1;
      count--;

      fromLaneCount.clear();
      toLaneCount.clear();
    }
#else
    struct LinkIndex {
      size_t index;
      lanepkid from_lid;
      lanepkid to_lid;
      LinkIndex(size_t i, lanepkid f_lid, lanepkid t_lid) : index(i), from_lid(f_lid), to_lid(t_lid) {}
    };
    std::vector<LinkIndex> link_indexs;
    for (size_t i = 0; i < lanelinks.size(); ++i)
      link_indexs.push_back(LinkIndex(i, lanelinks[i]->fromLaneId(), lanelinks[i]->toLaneId()));
    sort(link_indexs.begin(), link_indexs.end(), [](const LinkIndex& a, const LinkIndex& b) {
      if (a.from_lid == b.from_lid) {
        return a.to_lid > b.to_lid;
      } else {
        return a.from_lid > b.from_lid;
      }
    });

    // TODO(undefined) only for hdjx
    if (link_indexs.size() == 2 && link_indexs[0].from_lid == link_indexs[1].from_lid &&
        link_indexs[0].to_lid == link_indexs[1].to_lid)
      std::swap(link_indexs[0], link_indexs[1]);

    for (auto& link_index : link_indexs) targetIndex.push_back(link_index.index);
#endif

    txSectionPtr sectionPtr(new txSection);
    sectionPtr->setId(0);
    std::vector<double> _lane_width;
    for (int i = 0, index = 0; i < static_cast<int>(targetIndex.size()); ++i) {
#ifdef SINGLE_PASS_ON
      if (flag[i] != 1) continue;
#endif
      txLaneLinkPtr linkPtr = lanelinks[targetIndex[i]];

      txLanePtr fromLanePtr, toLanePtr;
      roadpkid frid = linkPtr->fromRoadId();
      fromLanePtr = fromRoadPtr->getLane(linkPtr->fromSectionId(), linkPtr->fromLaneId());
      toLanePtr = toRoadPtr->getLane(linkPtr->toSectionId(), linkPtr->toLaneId());

      txLanePtr lanePtr(new txLane);
      lanePtr->setId(0 - index - 1);
      // char label[8];
      // sprintf( label, "%d", 0-i-1 );
      std::string laneName = "MockLane" + std::to_string(fromLanePtr->getRoadId()) + "." +
                             std::to_string(fromLanePtr->getSectionId()) + "." + std::to_string(fromLanePtr->getId()) +
                             "_" + std::to_string(toLanePtr->getRoadId()) + "." +
                             std::to_string(toLanePtr->getSectionId()) + "." + std::to_string(toLanePtr->getId());
      lanePtr->setName(laneName);
      index++;

      lanePtr->setGeometry(linkPtr->getGeometry());

      // set lane type & speed limit
      lanePtr->setRoadId(-1);
      lanePtr->setLaneType(LANE_TYPE_Driving);
      lanePtr->setLaneArrow(LANE_ARROW_None);
      lanePtr->setSpeedLimit(toLanePtr->getSpeedLimit());
      double fromLaneWidth = fromLanePtr->getLaneWidth();
      double toLaneWidth = toLanePtr->getLaneWidth();
      lanePtr->setLaneWidth((fromLaneWidth + toLaneWidth) / 2);

      if (i == 0) {
        double from_left_dis = RoutingUtils::pointsDisWGS84(fromLanePtr->getGeometry()->getEnd(),
                                                            fromLanePtr->getLeftBoundary()->getGeometry()->getEnd());
        double to_left_dis = RoutingUtils::pointsDisWGS84(toLanePtr->getGeometry()->getStart(),
                                                          toLanePtr->getLeftBoundary()->getGeometry()->getStart());
        _lane_width.push_back(from_left_dis);
        _lane_width.push_back(to_left_dis);
      }
      if (i == targetIndex.size() - 1) {
        double from_right_dis = RoutingUtils::pointsDisWGS84(fromLanePtr->getGeometry()->getEnd(),
                                                             fromLanePtr->getRightBoundary()->getGeometry()->getEnd());
        double to_right_dis = RoutingUtils::pointsDisWGS84(toLanePtr->getGeometry()->getStart(),
                                                           toLanePtr->getRightBoundary()->getGeometry()->getStart());
        _lane_width.push_back(from_right_dis);
        _lane_width.push_back(to_right_dis);
      }

      sectionPtr->add(lanePtr);
    }

    // construct road
    roadPtr.reset(new txRoad);
    roadPtr->addSection(sectionPtr);
    roadPtr->setGeometry(roadPtr->getSections()[0]->getLanes()[0]->getGeometry());
    roadPtr->setId(idGenPtr->generateMockRoadId(RoutingUtils::linkIndex(fromRoadPtr->getId(), toRoadPtr->getId())));
    roadPtr->setRoadType(fromRoadPtr->getRoadType());

    // create lane boundary
    txLanePtr leftLanePtr = roadPtr->getSections()[0]->getLanes().front();
    txLanePtr rightLanePtr = roadPtr->getSections()[0]->getLanes().back();

    double _left_from_width = _lane_width[0];
    double _left_to_width = _lane_width[1];
    double _right_from_width = _lane_width[_lane_width.size() - 2];
    double _right_to_width = _lane_width[_lane_width.size() - 1];

    hadmap::PointVec leftLaneGeom, leftBoundaryGeom, rightLaneGeom, rightBoundaryGeom;
    dynamic_cast<const hadmap::txLineCurve*>(leftLanePtr->getGeometry())->sample(2.0, leftLaneGeom);
    dynamic_cast<const hadmap::txLineCurve*>(rightLanePtr->getGeometry())->sample(2.0, rightLaneGeom);
    RoutingUtils::createOffsetGeom(leftLaneGeom, _left_from_width * -1, _left_to_width * -1, leftBoundaryGeom);
    RoutingUtils::createOffsetGeom(rightLaneGeom, _right_from_width * 1, _right_to_width * 1, rightBoundaryGeom);
    //  RoutingUtils::createOffsetGeom( leftLaneGeom, leftLanePtr->getLaneWidth() * -0.5, leftLanePtr->getLaneWidth() *
    // -0.5, leftBoundaryGeom );   RoutingUtils::createOffsetGeom( rightLaneGeom, rightLanePtr->getLaneWidth() *
    // 0.5, rightLanePtr->getLaneWidth() * 0.5, rightBoundaryGeom );

    hadmap::txLaneBoundaryPtr leftBoundaryPtr(new hadmap::txLaneBoundary), rightBoundaryPtr(new hadmap::txLaneBoundary);
    leftBoundaryPtr->setGeometry(leftBoundaryGeom, COORD_WGS84);
    leftBoundaryPtr->setId(idGenPtr->generateBoundaryId());
    rightBoundaryPtr->setGeometry(rightBoundaryGeom, COORD_WGS84);
    rightBoundaryPtr->setId(idGenPtr->generateBoundaryId());

    leftLanePtr->setLeftBoundary(leftBoundaryPtr);
    rightLanePtr->setRightBoundary(rightBoundaryPtr);

    return true;
  }
}

bool MapDataLoad::addRoadsByLinks(txRoads& roads, std::unordered_map<std::string, txLaneLinks>& links) {
  std::unordered_set<roadpkid> curLoadedRoadInfo;
  for (auto roadPtr : roads) curLoadedRoadInfo.insert(roadPtr->getId());
  std::vector<roadpkid> missingRoadIds;
  for (auto itr = links.begin(); itr != links.end(); ++itr) {
    roadpkid fromRId = itr->second[0]->fromRoadId();
    roadpkid toRId = itr->second[0]->toRoadId();
    if (curLoadedRoadInfo.find(fromRId) == curLoadedRoadInfo.end()) {
      curLoadedRoadInfo.insert(fromRId);
      missingRoadIds.push_back(fromRId);
    }
    if (curLoadedRoadInfo.find(toRId) == curLoadedRoadInfo.end()) {
      curLoadedRoadInfo.insert(toRId);
      missingRoadIds.push_back(toRId);
    }
  }
  if (missingRoadIds.empty()) return true;
  txRoads missingRoads;
  if (TX_HADMAP_DATA_OK == getRoads(pHandle, missingRoadIds, true, missingRoads)) {
    txRoads revRoads;
    generateReverseRoads(missingRoads, revRoads);
    roads.insert(roads.end(), missingRoads.begin(), missingRoads.end());
    roads.insert(roads.end(), revRoads.begin(), revRoads.end());
  }
  /*
  std::unordered_map< roadpkid, txRoadPtr > curLoadedRoadInfo;
  for ( auto roadPtr : roads )
          curLoadedRoadInfo.insert( std::make_pair( roadPtr->getId(), roadPtr ) );
  for ( auto itr = links.begin(); itr != links.end(); ++ itr )
  {
          roadpkid fromRId = itr->second[0]->fromRoadId();
          roadpkid toRId = itr->second[0]->toRoadId();
          if ( curLoadedRoadInfo.find( fromRId ) == curLoadedRoadInfo.end() )
          {
                  txRoadPtr roadPtr;
                  if ( TX_HADMAP_DATA_OK == getRoad( pHandle, fromRId, true, roadPtr ) )
                  {
                          curLoadedRoadInfo.insert( std::make_pair( fromRId, roadPtr ) );
                          roads.push_back( roadPtr );
                  }
          }
          if ( curLoadedRoadInfo.find( toRId ) == curLoadedRoadInfo.end() )
          {
                  txRoadPtr roadPtr;
                  if ( TX_HADMAP_DATA_OK == getRoad( pHandle, toRId, true, roadPtr ) )
                  {
                          curLoadedRoadInfo.insert( std::make_pair( toRId, roadPtr ) );
                          roads.push_back( roadPtr );
                  }
          }
  }
  */
  return true;
}

bool MapDataLoad::generateMockLinks(const txRoadPtr& fromRoadPtr, const txRoadPtr& toRoadPtr, txLaneLinks& mockLinks) {
  if (fromRoadPtr == NULL || toRoadPtr == NULL) return false;
  txLanes& fromLanes = fromRoadPtr->getSections().back()->getLanes();
  txLanes& toLanes = toRoadPtr->getSections().front()->getLanes();

  mockLinks.clear();
  for (auto fromLanePtr : fromLanes) {
    for (auto toLanePtr : toLanes) {
      if (RoutingUtils::geomConnectDirectly(fromLanePtr->getGeometry(), toLanePtr->getGeometry())) {
        txLaneLinkPtr linkPtr(new txLaneLink);
        linkPtr
            ->setId(
                idGenPtr->generateLinkId(RoutingUtils::linkIndex(fromLanePtr->getTxLaneId(), toLanePtr->getTxLaneId())))
            .setFromRoadId(fromLanePtr->getRoadId())
            .setFromSectionId(fromLanePtr->getSectionId())
            .setFromLaneId(fromLanePtr->getId())
            .setToRoadId(toLanePtr->getRoadId())
            .setToSectionId(toLanePtr->getSectionId())
            .setToLaneId(toLanePtr->getId());
        mockLinks.push_back(linkPtr);
      }
    }
  }
  return !mockLinks.empty();
}

bool MapDataLoad::generateReverseRoads(const txRoads& roads, txRoads& reverseRoads) {
  reverseRoads.clear();
  for (auto& roadPtr : roads) {
    if (roadPtr->isBidirectional()) {
      roadpkid rid = roadPtr->getId();
      txRoadPtr revRoadPtr(new txRoad(*roadPtr));
      revRoadPtr->reverse();
      revRoadPtr->bindRoadPtr();

      roadpkid revRid = idGenPtr->generateReverseRoadId(RoutingUtils::reverseRoadIndex(roadPtr->getId()));
      revRoadPtr->setId(revRid);

      mockRoad2RealRoad[revRid] = roadPtr->getId();

      std::unordered_map<laneboundarypkid, laneboundarypkid> boundaryIdMap;
      auto& sections = revRoadPtr->getSections();
      for (auto& secPtr : sections) {
        auto& lanes = secPtr->getLanes();
        for (auto& lanePtr : lanes) {
          laneboundarypkid leftBoundaryId = lanePtr->getLeftBoundaryId();
          if (boundaryIdMap.find(leftBoundaryId) == boundaryIdMap.end()) {
            boundaryIdMap[leftBoundaryId] = idGenPtr->generateBoundaryId();
          }

          laneboundarypkid rightBoundaryId = lanePtr->getRightBoundaryId();
          if (boundaryIdMap.find(rightBoundaryId) == boundaryIdMap.end()) {
            boundaryIdMap[rightBoundaryId] = idGenPtr->generateBoundaryId();
          }

          lanePtr->getLeftBoundary()->setId(boundaryIdMap[leftBoundaryId]);
          lanePtr->setLeftBoundaryId(boundaryIdMap[leftBoundaryId]);

          lanePtr->getRightBoundary()->setId(boundaryIdMap[rightBoundaryId]);
          lanePtr->setRightBoundaryId(boundaryIdMap[rightBoundaryId]);
        }
      }
      reverseRoads.push_back(revRoadPtr);
    }
  }
  return true;
}

bool MapDataLoad::updateRoadIdInLinks(std::unordered_map<roadpkid, txRoadPtr>& roadMap, txLaneLinks& links) {
  for (auto& linkPtr : links) {
    roadpkid fromRid = linkPtr->fromRoadId();
    sectionpkid fromSid = linkPtr->fromSectionId();
    lanepkid fromLid = linkPtr->fromLaneId();
    if (fromLid > 0) {
      roadpkid nfromRid = idGenPtr->generateReverseRoadId(RoutingUtils::reverseRoadIndex(fromRid));
      mockRoad2RealRoad[nfromRid] = fromRid;
      if (roadMap.find(nfromRid) == roadMap.end()) throw std::runtime_error("can not find road id in road map");
      fromSid = roadMap[nfromRid]->getSections().size() - fromSid - 1;
      fromLid *= -1;
      linkPtr->setFromRoadId(nfromRid);
      linkPtr->setFromSectionId(fromSid);
      linkPtr->setFromLaneId(fromLid);
    }

    roadpkid toRid = linkPtr->toRoadId();
    sectionpkid toSid = linkPtr->toSectionId();
    lanepkid toLid = linkPtr->toLaneId();
    if (toLid > 0) {
      roadpkid ntoRid = idGenPtr->generateReverseRoadId(RoutingUtils::reverseRoadIndex(toRid));
      mockRoad2RealRoad[ntoRid] = toRid;
      if (roadMap.find(ntoRid) == roadMap.end()) throw std::runtime_error("can not findn road id in road map");
      toSid = roadMap[ntoRid]->getSections().size() - toSid - 1;
      toLid *= -1;
      linkPtr->setToRoadId(ntoRid);
      linkPtr->setToSectionId(toSid);
      linkPtr->setToLaneId(toLid);
    }
  }
  return true;
}

bool MapDataLoad::updateLinkMap(std::unordered_map<std::string, txLaneLinks>& linkMap) {
  std::unordered_set<std::string> removeIndex;
  std::unordered_map<std::string, txLaneLinks> newLinkMap;
  for (auto itr = linkMap.begin(); itr != linkMap.end(); ++itr) {
    std::string index = RoutingUtils::linkIndex(itr->second.front()->fromRoadId(), itr->second.front()->toRoadId());
    if (itr->first != index) {
      newLinkMap.insert(std::make_pair(index, itr->second));
      removeIndex.insert(itr->first);
    }
  }
  for (auto itr = removeIndex.begin(); itr != removeIndex.end(); ++itr) linkMap.erase(*itr);
  linkMap.insert(newLinkMap.begin(), newLinkMap.end());
  return true;
}

/*
bool MapDataLoad::loadData( const hadmap::txPoint& center, double radius )
{
        if ( pHandle == NULL )
                return false;

        double offset = radius / 111000.0;
        hadmap::PointVec envelope;
        envelope.push_back( hadmap::txPoint( center.x - offset, center.y - offset, center.z ) );
        envelope.push_back( hadmap::txPoint( center.x + offset, center.y + offset, center.z ) );

        hadmap::txRoads roads;
        if ( TX_HADMAP_DATA_OK == getRoads( pHandle, envelope, true, roads ) )
        {
                txRoads revRoads;
                generateReverseRoads( roads, revRoads );
                roads.insert( roads.end(), revRoads.begin(), revRoads.end() );

                std::unordered_map< std::string, txLaneLinks > directLinks, juncLinks;
                txLaneLinks links;
                // if ( loadLinksByRoad( roads, directLinks, juncLinks ) )
                if ( loadLinksByEnvelope( envelope, directLinks, juncLinks ) )
                {
                        std::unordered_map< roadpkid, txRoadPtr > curLoadedRoadInfo;
                        for ( auto roadPtr : roads )
                                curLoadedRoadInfo.insert( std::make_pair( roadPtr->getId(), roadPtr ) );

                        removeUselessLinks( curLoadedRoadInfo, directLinks );
                        removeUselessLinks( curLoadedRoadInfo, juncLinks );

                        // update road id in links
                        for ( auto itr = directLinks.begin(); itr != directLinks.end(); ++ itr )
                                updateRoadIdInLinks( curLoadedRoadInfo, itr->second );
                        for ( auto itr = juncLinks.begin(); itr != juncLinks.end(); ++ itr )
                                updateRoadIdInLinks( curLoadedRoadInfo, itr->second );

                        updateLinkMap( directLinks );
                        updateLinkMap( juncLinks );


                }
        }

}
*/
bool MapDataLoad::removeUselessLinks(const std::unordered_map<roadpkid, txRoadPtr>& roads,
                                     std::unordered_map<std::string, txLaneLinks>& links) {
  std::unordered_set<std::string> _useless_link_index;
  for (auto _itr = links.begin(); _itr != links.end(); ++_itr) {
    roadpkid _from_r_id = _itr->second.front()->fromRoadId();
    roadpkid _to_r_id = _itr->second.front()->toRoadId();
    if (roads.find(_from_r_id) != roads.end() && roads.find(_to_r_id) != roads.end()) continue;
    _useless_link_index.insert(_itr->first);
  }

  for (auto& _index : _useless_link_index) links.erase(_index);
  return true;
}

bool MapDataLoad::loadDataByRoads(hadmap::txRoads& roads) {
  if (roads.size() < 1) {
    return false;
  } else {
    txRoads revRoads;
    generateReverseRoads(roads, revRoads);
    roads.insert(roads.end(), revRoads.begin(), revRoads.end());
    std::unordered_map<std::string, txLaneLinks> directLinks, juncLinks;
    txLaneLinks links;
    // if ( loadLinksByEnvelope( envelope, directLinks, juncLinks ) )
    if (loadLinksByRoad(roads, directLinks, juncLinks)) {
      //  for ( auto itr = directLinks.begin(); itr != directLinks.end(); ++ itr )
      //    links.insert( links.end(), itr->second.begin(), itr->second.end() );

      // add roads by links
      // addRoadsByLinks( roads, directLinks );
      // addRoadsByLinks( roads, juncLinks );

      std::unordered_map<roadpkid, txRoadPtr> curLoadedRoadInfo;
      for (auto roadPtr : roads) curLoadedRoadInfo.insert(std::make_pair(roadPtr->getId(), roadPtr));

      removeUselessLinks(curLoadedRoadInfo, directLinks);
      removeUselessLinks(curLoadedRoadInfo, juncLinks);

      for (auto itr = directLinks.begin(); itr != directLinks.end(); ++itr)
        links.insert(links.end(), itr->second.begin(), itr->second.end());

      // update road id in links
      for (auto itr = directLinks.begin(); itr != directLinks.end(); ++itr)
        updateRoadIdInLinks(curLoadedRoadInfo, itr->second);
      for (auto itr = juncLinks.begin(); itr != juncLinks.end(); ++itr)
        updateRoadIdInLinks(curLoadedRoadInfo, itr->second);

      updateLinkMap(directLinks);
      updateLinkMap(juncLinks);

      std::unordered_map<junctionpkid, std::unordered_set<std::string> > juncInfo;
      generateJunction(juncLinks, roads, juncInfo);

      // update cur loaded road info
      for (auto& road_ptr : roads) {
        if (curLoadedRoadInfo.find(road_ptr->getId()) == curLoadedRoadInfo.end()) {
          curLoadedRoadInfo.insert(std::make_pair(road_ptr->getId(), road_ptr));
        }
      }

      // generate junction data
      std::unordered_map<junctionpkid, txJunctionPtr> juncMap;
      for (auto juncInfoP : juncInfo)
        juncMap.insert(std::make_pair(juncInfoP.first, std::make_shared<txJunction>(juncInfoP.first)));

      // generate mock roads and mock links
      txRoads mockRoads;
      txLaneLinks mockLinks;
      for (auto itr = juncLinks.begin(); itr != juncLinks.end(); ++itr) {
        roadpkid fromRId = itr->second[0]->fromRoadId();
        roadpkid toRId = itr->second[0]->toRoadId();

        bool uselessFlag = false;
        for (auto curLinkPtr : itr->second) {
          if (curLinkPtr->getGeometry() == NULL || curLinkPtr->getGeometry()->empty()) {
            uselessFlag = true;

            // direct links

            if (directLinks.find(itr->first) == directLinks.end()) {
              directLinks[itr->first] = itr->second;
              links.insert(links.end(), itr->second.begin(), itr->second.end());
            }

            break;
          }
        }
        if (uselessFlag) continue;

        std::string juncLinkIndex = RoutingUtils::linkIndex(fromRId, toRId);
        junctionpkid jId;
        for (auto juncInfoP : juncInfo) {
          if (juncInfoP.second.find(juncLinkIndex) != juncInfoP.second.end()) jId = juncInfoP.first;
        }

        txRoadPtr mockRoadPtr;
        if (generateMockRoad(curLoadedRoadInfo[fromRId], curLoadedRoadInfo[toRId], itr->second, mockRoadPtr)) {
          txLaneLinks fromLinks, toLinks;
          generateMockLinks(curLoadedRoadInfo[fromRId], mockRoadPtr, fromLinks);
          generateMockLinks(mockRoadPtr, curLoadedRoadInfo[toRId], toLinks);
          mockLinks.insert(mockLinks.end(), fromLinks.begin(), fromLinks.end());
          mockLinks.insert(mockLinks.end(), toLinks.begin(), toLinks.end());

          txJunctionRoadPtr juncPtr(new txJunctionRoad(*mockRoadPtr));
          juncPtr->setId(mockRoadPtr->getId());
          juncPtr->setFromRoadId(fromRId);
          juncPtr->setToRoadId(toRId);
          juncPtr->bindRoadPtr();
          mockRoads.push_back(juncPtr);
          juncPtr->setJuncId(jId);

          juncMap[jId]->insertJuncRoad(juncPtr);
        }
      }

      roads.insert(roads.end(), mockRoads.begin(), mockRoads.end());
      links.insert(links.end(), mockLinks.begin(), mockLinks.end());

      updateLinks(links);

      txJunctions juncs;
      for (auto juncInfoP : juncMap) juncs.push_back(juncInfoP.second);
      updateJunctions(juncs);
    }
    updateRoads(roads);

    std::vector<OBJECT_TYPE> objTypes;
    objTypes.push_back(OBJECT_TYPE_Stop);
    objTypes.push_back(OBJECT_TYPE_TrafficLights);
    objTypes.push_back(OBJECT_TYPE_CrossWalk);
    objTypes.push_back(OBJECT_TYPE_Pole);
    objTypes.push_back(OBJECT_TYPE_LaneBoundary);
    objTypes.push_back(OBJECT_TYPE_ParkingSpace);
    objTypes.push_back(OBJECT_TYPE_LineSegment_PL);
    // loadObjDataByRoads( objTypes );
    loadObjDataByEnvelope(objTypes, cur_envelope_);

    return true;
  }
}

bool MapDataLoad::loadObjDataByRoads(const std::vector<OBJECT_TYPE>& types) {
  hadmap::txObjects objects;
  std::vector<txLaneId> lane_ids;
  //  for ( auto& road_ptr : curAddedRoads )
  // //    lane_ids.push_back( txLaneId( road_ptr->getId(), SECT_PKID_INVALID, LANE_PKID_INVALID ) );
  for (auto itr = curRoads.begin(); itr != curRoads.end(); ++itr)
    lane_ids.push_back(txLaneId(itr->second->getId(), SECT_PKID_INVALID, LANE_PKID_INVALID));

  if (TX_HADMAP_DATA_OK == getObjects(pHandle, lane_ids, types, objects)) {
    return updateObjects(objects);
  } else {
    return false;
  }
}

bool MapDataLoad::loadObjDataByEnvelope(const std::vector<OBJECT_TYPE>& types, const PointVec& envelope) {
  hadmap::txObjects objects;
  int r = getObjects(pHandle, envelope, types, objects);
  if (TX_HADMAP_DATA_OK == r) {
    return updateObjects(objects);
  } else {
    return r == TX_HADMAP_DATA_EMPTY;
  }
}

bool MapDataLoad::loadData(const hadmap::txPoint& center, double radius) {
  if (pHandle == NULL) return false;

  double offset = radius / 111000.0;
  cur_envelope_.clear();
  cur_envelope_.push_back(hadmap::txPoint(center.x - offset, center.y - offset, center.z));
  cur_envelope_.push_back(hadmap::txPoint(center.x + offset, center.y + offset, center.z));

  hadmap::txRoads roads;
  if (TX_HADMAP_DATA_OK == getRoads(pHandle, cur_envelope_, true, roads)) {
    if (loadDataByRoads(roads)) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool MapDataLoad::loadData(const std::vector<roadpkid>& roadIds) {
  if (pHandle == NULL || roadIds.empty()) {
    return false;
  } else {
    hadmap::txRoads roads;
    cur_envelope_.clear();
    if (TX_HADMAP_DATA_OK == getRoads(pHandle, roadIds, true, roads)) {
      genEnvelope(roads);
      return loadDataByRoads(roads);
    } else {
      return false;
    }
  }
}

bool MapDataLoad::getMapData(hadmap::txRoads& roads, hadmap::txLaneLinks& links, hadmap::txObjects& objects) {
  roads.resize(curRoads.size());
  size_t i = 0;
  for (auto roadPair : curRoads) roads[i++] = roadPair.second;

  links.resize(curLinks.size());
  i = 0;
  for (auto linkPair : curLinks) links[i++] = linkPair.second;

  objects.resize(objects.size());
  i = 0;
  for (auto objectPair : curObjects) objects[i++] = objectPair.second;

  return (!roads.empty()) || (!links.empty()) || (!objects.empty());
}

bool MapDataLoad::getAddedMapData(hadmap::txRoads& roads, hadmap::txLaneLinks& links, hadmap::txObjects& objects) {
  roads.assign(curAddedRoads.begin(), curAddedRoads.end());

  links.assign(curAddedLinks.begin(), curAddedLinks.end());

  objects.assign(curAddedObjects.begin(), curAddedObjects.end());

  return (!roads.empty()) || (!links.empty()) || (!objects.empty());
}

bool MapDataLoad::getAddedJuncData(hadmap::txJunctions& juncs) {
  juncs.assign(curAddedJunctions.begin(), curAddedJunctions.end());
  return !juncs.empty();
}

bool MapDataLoad::getRemovedMapData(hadmap::txRoads& roads, hadmap::txLaneLinks& links, hadmap::txObjects& objects) {
  roads.assign(curRemovedRoads.begin(), curRemovedRoads.end());

  links.assign(curRemovedLinks.begin(), curRemovedLinks.end());

  objects.assign(curRemovedObjects.begin(), curRemovedObjects.end());

  return (!roads.empty()) || (!links.empty()) || (!objects.empty());
}

bool MapDataLoad::getRemovedJuncData(hadmap::txJunctions& juncs) {
  juncs.assign(curRemovedJunctions.begin(), curRemovedJunctions.end());
  return !juncs.empty();
}

void MapDataLoad::setRouteInfo(const txRoute& route) {
  {
    std::unique_lock<std::mutex> lock(routeRoadMutex);
    routeRoadId.clear();
    for (auto& routeNode : route) routeRoadId.insert(routeNode.getId());
  }
  {
    std::unique_lock<std::mutex> lck(mockRoadIdMapMutex);
    for (auto& routeNode : route) {
      if (routeNode.getRouteType() == txRouteNode::JUNCTION_ROUTE) {
        std::string index = RoutingUtils::linkIndex(routeNode.getPreId(), routeNode.getNextId());
        if (mockRoadIdMap.find(index) == mockRoadIdMap.end()) {
          mockRoadIdMap.insert(std::make_pair(index, routeNode.getId()));
        } else {
          mockRoadIdMap[index] = routeNode.getId();
        }
      }
    }
  }
}

bool MapDataLoad::generateJunction(std::unordered_map<std::string, txLaneLinks>& links, txRoads& roads,
                                   std::unordered_map<junctionpkid, std::unordered_set<std::string> >& juncInfo) {
  std::unordered_map<std::string, txLaneLinks> addedLinks;
  std::unordered_set<std::string> usedLinksIndex;
  std::unordered_map<roadpkid, txRoadPtr> usedRoads;
  txRoads addedRoads;
  for (auto& r : roads) usedRoads.insert(std::make_pair(r->getId(), r));
  for (auto itr = links.begin(); itr != links.end(); ++itr) {
    if (usedLinksIndex.find(itr->first) != usedLinksIndex.end()) {
      continue;
    } else {
      txRoads juncRoads;
      txLaneLinks juncLinks;

      /*
      roadpkid entranceId = itr->second.front()->fromRoadId();
      if ( mockRoad2RealRoad.find( entranceId ) != mockRoad2RealRoad.end() )
              entranceId = mockRoad2RealRoad[ entranceId ];
      */

      if (TX_HADMAP_DATA_OK == getJuncInfo(pHandle, itr->second.front()->getId(), juncRoads, juncLinks)) {
        for (auto& r : juncRoads) {
          if (usedRoads.find(r->getId()) != usedRoads.end()) continue;
          usedRoads.insert(std::make_pair(r->getId(), r));
          addedRoads.push_back(r);
        }
        std::vector<lanelinkpkid> juncLinksIndex;
        for (auto& l : juncLinks) juncLinksIndex.push_back(l->getId());
        junctionpkid juncId = idGenPtr->generateJuncId(juncLinksIndex);
        juncInfo.insert(std::make_pair(juncId, std::unordered_set<std::string>()));
        for (auto& l : juncLinks) {
          roadpkid curFromRId = l->fromRoadId();
          if (l->fromLaneId() > 0) {
            curFromRId = idGenPtr->generateReverseRoadId(RoutingUtils::reverseRoadIndex(curFromRId));
            mockRoad2RealRoad[curFromRId] = l->fromRoadId();

            txRoadPtr _from_r_ptr = usedRoads[l->fromRoadId()];
            l->setFromRoadId(curFromRId);
            l->setFromSectionId(_from_r_ptr->getSections().size() - 1);
            l->setFromLaneId(l->fromLaneId() * -1);
          }
          roadpkid curToRId = l->toRoadId();
          if (l->toLaneId() > 0) {
            curToRId = idGenPtr->generateReverseRoadId(RoutingUtils::reverseRoadIndex(curToRId));
            mockRoad2RealRoad[curToRId] = l->toRoadId();

            txRoadPtr _to_r_ptr = usedRoads[l->toRoadId()];
            l->setToRoadId(curToRId);
            l->setToSectionId(0);
            l->setToLaneId(l->toLaneId() * -1);
          }
          std::string linkIndex = RoutingUtils::linkIndex(curFromRId, curToRId);
          if (links.find(linkIndex) == links.end()) {
            if (addedLinks.find(linkIndex) == addedLinks.end()) {
              addedLinks.insert(std::make_pair(linkIndex, txLaneLinks()));
            }
            addedLinks[linkIndex].push_back(l);
          }
          usedLinksIndex.insert(linkIndex);
          juncInfo[juncId].insert(linkIndex);
        }
      }
    }
  }
  txRoads revRoads;
  generateReverseRoads(addedRoads, revRoads);
  roads.insert(roads.end(), addedRoads.begin(), addedRoads.end());
  roads.insert(roads.end(), revRoads.begin(), revRoads.end());
  links.insert(addedLinks.begin(), addedLinks.end());
  return true;
}

bool MapDataLoad::genEnvelope(const txRoads& roads) {
  if (roads.empty()) return false;

  cur_envelope_.push_back(txPoint(200.0, 90.0, 0.0));
  cur_envelope_.push_back(txPoint(0.0, 0.0, 0.0));

  for (auto& road_ptr : roads) {
    const txLineCurve* curve_ptr = dynamic_cast<const txLineCurve*>(road_ptr->getGeometry());
    PointVec points;
    curve_ptr->getPoints(points);
    for (auto& p : points) {
      cur_envelope_[0].x = std::min(cur_envelope_[0].x, p.x);
      cur_envelope_[0].y = std::min(cur_envelope_[0].y, p.y);

      cur_envelope_[1].x = std::max(cur_envelope_[1].x, p.x);
      cur_envelope_[1].y = std::max(cur_envelope_[1].y, p.y);
    }
  }

  const double METER_PER_DEG = 111000.0;
  const double OFFSET = 20.0 / METER_PER_DEG;
  cur_envelope_[0].x -= OFFSET;
  cur_envelope_[0].y -= OFFSET;
  cur_envelope_[1].x += OFFSET;
  cur_envelope_[1].y += OFFSET;

  return true;
}
}  // namespace hadmap
