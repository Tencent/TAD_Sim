// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routingmap/routing_map.h"

#include <atomic>
#include <chrono>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

#include "common/coord_trans.h"
#include "common/log.h"
#include "common/map_util.h"
#include "routingmap/blocking_queue.hpp"
#include "routingmap/driving_track.h"
#include "routingmap/id_generator.hpp"
#include "routingmap/loading_info.h"
#include "routingmap/mapdata_load.h"
#include "routingmap/routing_log.h"
#include "routingmap/routing_request.h"
#include "routingmap/routing_utils.h"
#include "routingmap/txmap_proxy.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_junctionroad.h"

namespace hadmap {
#define LOADING_CENTER "LOADING_CENTER"
#define UPDATE_CENTER "UPDATE_CENTER"

#define LOADING_DIS 100.0
#define UPDATE_DIS 1000.0
#define MAP_RADIUS 1000.0

#define AREA_LOADING

// #define DRIVING_TRACE_ON

struct RoutingMap::RoutingMapData {
  std::shared_ptr<txMapProxy> mapProxyPtr;

  IdGeneratorPtr idGenPtr;

  std::shared_ptr<RoutingRequest> routingReqPtr;

  std::shared_ptr<MapDataLoad> mapLoadPtr;

  txMapHandle* pHandle;

  txPoint curPos;

  double upCenDis;

  double loadDis;

  BlockingQueue<LoadingInfoPtr> loadingQueue;

  std::thread searchT;

  pkid_t routingId;

  DrivingTrackPtr dtPtr;

  bool routingOn;

  size_t frameId;

  bool driving_trace_on_;

  RoutingMapData(CoordType mapGeomCoordType, const std::string& dbPath)
      : pHandle(NULL),
        upCenDis(0.0),
        loadDis(0.0),
        routingId(0),
        routingOn(false),
        frameId(0),
        driving_trace_on_(false) {
    MAP_DATA_TYPE map_data_type = SQLITE;
    if (dbPath.find(".xodr") != std::string::npos) map_data_type = OPENDRIVE;

    // TODO(undefined) resource detected by HMI
    std::ifstream fin;
    fin.open(dbPath);
    if (!fin.is_open()) {
      throw std::runtime_error(dbPath + " not exists");
    }
    fin.close();

    if (TX_HADMAP_HANDLE_OK == hadmapConnect(dbPath.c_str(), map_data_type, &pHandle)) {
      // if ( TX_HADMAP_HANDLE_OK == hadmapConnect( dbPath.c_str(), REMOTE, &pHandle ) )
#ifdef DRIVING_TRACE_ON
      driving_trace_on_ = true;
#endif
      mapProxyPtr.reset(new txMapProxy(mapGeomCoordType));

      idGenPtr.reset(new IdGenerator(pHandle));

      routingReqPtr.reset(new RoutingRequest(idGenPtr));
      routingReqPtr->init(pHandle, dbPath);

      mapLoadPtr.reset(new MapDataLoad(pHandle, idGenPtr));

      dtPtr.reset(new DrivingTrack(mapProxyPtr));

      searchT = std::thread(&RoutingMapData::loadDataProc, this);

      RoutingLog::getInstance()->init("routing.log");
    } else {
      std::cout << dbPath << std::endl;
      pHandle = NULL;
    }
  }

  ~RoutingMapData() {
    loadingQueue.push(std::make_shared<ExitInfo>());
    if (searchT.joinable()) searchT.join();
  }

  txRoadPtr getRoad(const txRouteNode& routeNodeInfo) {
    if (mapProxyPtr) {
      txRoadPtr roadPtr = mapProxyPtr->getRoadById(routeNodeInfo.getId());
      if (roadPtr == NULL) {
        /*
        std::vector< RoadIdInfoPtr > infoPtrAry;
        if ( routeNodeInfo.getRouteType() == txRouteNode::ROAD_ROUTE )
        {
                RoadIdInfoPtr infoPtr = std::make_shared< RoadIdInfo >( routeNodeInfo.getId() );
                infoPtrAry.push_back( infoPtr );
        }
        else
        {
                infoPtrAry.push_back( std::make_shared< RoadIdInfo >( routeNodeInfo.getPreId() ) );
                infoPtrAry.push_back( std::make_shared< RoadIdInfo >( routeNodeInfo.getNextId() ) );
        }
        std::unique_lock< std::mutex > lck( searchPosAryMutex );
        for ( auto& infoPtr : infoPtrAry )
        {
                bool flag = false;
                for ( size_t i = 0; i < searchPosAry.size(); ++ i )
                {
                        if ( searchPosAry[i]->type == LOADING_ROADID && \
                                        infoPtr->rid == std::dynamic_pointer_cast< RoadIdInfo >( searchPosAry[i] )->rid
        )
                        {
                                flag = true;
                                if ( i == 0 ){
                                        break;}
                                std::swap( searchPosAry[0], searchPosAry[i] );
                                break;
                        }
                }
                if ( !flag ){
                        searchPosAry.insert( searchPosAry.begin(), infoPtr );}
        }
        */
        txlog::debug("getRoad:: getRoadById return null.");
        return NULL;
      } else {
        if (routeNodeInfo.getRouteType() == txRouteNode::ROAD_ROUTE) {
          txlog::debug("getRoad:: return road.");
          return roadPtr;
        } else {
          txJunctionRoadPtr juncPtr = std::dynamic_pointer_cast<txJunctionRoad>(roadPtr);
          if (juncPtr == NULL) {
            txlog::debug("getRoad:: cast roadptr falid.");
            return NULL;
          }
          // txJunctionRoadPtr juncPtr( new txJunctionRoad( *roadPtr ) );
          juncPtr->setTurnDir((DIR_TYPE)(routeNodeInfo.getAttr() & 0xf));
          juncPtr->setLightFlag(static_cast<bool>(routeNodeInfo.getAttr() & 16));
          // juncPtr->setId( roadPtr->getId() );
          // juncPtr->bindRoadPtr();

          txlog::debug("getRoad:: return junction.");
          return juncPtr;
        }
      }
    } else {
      txlog::debug("getRoad:: mapProxyPtr is null.");
      return NULL;
    }
    txlog::debug("getRoad:: return null.");
    return NULL;
  }

  RoutingStatus getRoute(txRoute& route) {
    if (routingReqPtr == NULL) {
      return ROUTING_FAIL;
    } else {
      return routingReqPtr->getRoute(route);
    }
  }

  bool generateLoadingRoadAryUnSafe(const txRoute& route, const txPoint& curPos, const bool& init) {
    size_t startIndex;
    double startRatio;
    if (init) {
      startIndex = 0;
      startRatio = route.front().getStartRange();
    } else {
      txLanePtr nLanePtr = mapProxyPtr->nearestLane(curPos, 5);
    }
    return false;
  }

  // TODO(undefined) need fix bug, may delete cur road
  bool generateRoadLoadingList(LoadingInfoPtr& loading_ptr) {
    std::vector<roadpkid> rids;
    txRouteNode front_route_node;
    if (dtPtr->frontRouteNode(front_route_node)) {
      if (dtPtr->passedDisInCurNode() < 50.0) {
        if (front_route_node.getRouteType() == txRouteNode::JUNCTION_ROUTE) {
          rids.push_back(idGenPtr->getOriginRoadId(front_route_node.getPreId()));
        } else {
          rids.push_back(idGenPtr->getOriginRoadId(front_route_node.getId()));
        }
      }
    }
    txRoute loading_route;
    dtPtr->routeAhead(500.0, loading_route);

    // std::vector< roadpkid > lrids;
    for (size_t i = 0; i < loading_route.size(); ++i) {
      // lrids.push_back( loading_route[i].getId() );
      if (i == 0 && loading_route[i].getRouteType() == txRouteNode::JUNCTION_ROUTE) {
        rids.push_back(front_route_node.getId());
      }
      if (loading_route[i].getRouteType() == txRouteNode::ROAD_ROUTE) {
        rids.push_back(idGenPtr->getOriginRoadId(loading_route[i].getId()));
      }
    }
    /*
    for ( auto& node : loading_route )
    {
            if ( node.getRouteType() == txRouteNode::ROAD_ROUTE ){
                    rids.push_back( idGenPtr->getOriginRoadId( node.getId() ) );}
    }
    */
    if (rids.empty()) return false;

    loading_ptr.reset(new RoadListInfo(rids));
    return true;
  }

  bool routingSync(const PointVec& poiLocs, txRoute& route) {
    PointVec newpoiLocs;
    newpoiLocs.reserve(poiLocs.size());
    for (const auto& p : poiLocs) {
      txLanePtr lanePtr;
      if (TX_HADMAP_DATA_OK == getLane(pHandle, p, lanePtr)) {
        newpoiLocs.push_back(p);
      }
    }
    if (routingReqPtr->routingSync(newpoiLocs, ++routingId, route)) {
      dtPtr->setRoute(route, newpoiLocs.front());
      dtPtr->markPassedCenter(UPDATE_CENTER);
      dtPtr->markPassedCenter(LOADING_CENTER);
      mapProxyPtr->updateCenter(newpoiLocs.front());
      mapLoadPtr->setRouteInfo(route);
      {
        std::vector<roadpkid> rids;
        for (const auto& node : route) {
          if (node.getRouteType() == txRouteNode::ROAD_ROUTE) {
            rids.push_back(node.getId());
          }
        }
        if (mapLoadPtr->loadData(rids)) {
          txRoads addedRoads;
          txLaneLinks addedLinks;
          txObjects addedObjs;
          if (mapLoadPtr->getAddedMapData(addedRoads, addedLinks, addedObjs)) {
            mapProxyPtr->insertRoads(addedRoads);
            mapProxyPtr->insertLaneLinks(addedLinks);
            mapProxyPtr->insertObjects(addedObjs);
          }
          txJunctions addedJunctions;
          if (mapLoadPtr->getAddedJuncData(addedJunctions)) mapProxyPtr->insertJunctions(addedJunctions);
        }
      }
      {
#ifdef AREA_LOADING
        loadingQueue.push(std::make_shared<CoordInfo>(newpoiLocs.front()));
#else
        LoadingInfoPtr loading_ptr;
        if (generateRoadLoadingList(loading_ptr)) loadingQueue.push(loading_ptr);
#endif
        loadingQueue.push(std::make_shared<PauseInfo>(0));
        // searchPosAry.push_back( std::make_shared< PauseInfo >( 0 ) );
      }
      /*
      size_t size = 2;

      while ( size > 0 )
      {
              std::unique_lock< std::mutex > lck( searchPosAryMutex );
              size = searchPosAry.size();
      }
      */
      // while (loadingQueue.size() > 0);

      routingOn = true;
      return true;
    } else {
      return false;
    }
  }

  bool updateLocation(const txPoint& pos) {
    frameId++;
    if (routingOn && driving_trace_on_) {
      dtPtr->setPos(pos);
      upCenDis = dtPtr->passedDis(UPDATE_CENTER);
      loadDis = dtPtr->passedDis(LOADING_CENTER);
      if (loadDis > LOADING_DIS) {
        dtPtr->markPassedCenter(LOADING_CENTER);
        /*
        std::unique_lock< std::mutex > lck(searchPosAryMutex);
        txPoint loadPos = dtPtr->posAhead( 150.0 );

        searchPosAry.push_back(std::make_shared< CoordInfo >( loadPos ) );
        */

#ifdef AREA_LOADING
        txPoint loadPos = dtPtr->posAhead(150.0);
        loadingQueue.push(std::make_shared<CoordInfo>(loadPos));
        ROUTING_LOG("Search Pos");
        ROUTING_LOG(loadPos);
#else
        ROUTING_LOG("Search Pos");
        ROUTING_LOG(pos);
        LoadingInfoPtr loading_ptr;
        if (generateRoadLoadingList(loading_ptr)) loadingQueue.push(loading_ptr);
#endif
        loadDis = 0.0;
      }
      if (upCenDis > UPDATE_DIS) {
        dtPtr->markPassedCenter(UPDATE_CENTER);
        mapProxyPtr->updateCenter(pos);
        upCenDis = 0.0;
      }
    } else {
      if (frameId == 1) {
        curPos = pos;
        mapProxyPtr->updateCenter(pos);
        {
          /*
          std::unique_lock< std::mutex > lck( searchPosAryMutex );
          searchPosAry.push_back( std::make_shared< CoordInfo >( curPos ) );
          searchPosAry.push_back( std::make_shared< PauseInfo >( 0 ) );
          */
          loadingQueue.push(std::make_shared<CoordInfo>(curPos));
          loadingQueue.push(std::make_shared<PauseInfo>(0));
        }

        /*
        size_t size = 2;
        while ( size > 0 )
        {
                std::unique_lock< std::mutex > lck( searchPosAryMutex );
                size = searchPosAry.size();
        }
        */
        // while (loadingQueue.size() > 0);
      } else {
        double dis = RoutingUtils::pointsDisWGS84(curPos, pos);
        curPos = pos;
        upCenDis += dis;
        loadDis += dis;
        if (loadDis > LOADING_DIS) {
          /*
          std::unique_lock< std::mutex > lck( searchPosAryMutex );
          searchPosAry.push_back( std::make_shared< CoordInfo >( curPos ) );
          */

          loadingQueue.push(std::make_shared<CoordInfo>(curPos));

          loadDis = 0.0;
        }
        if (upCenDis > UPDATE_DIS) {
          mapProxyPtr->updateCenter(curPos);
          upCenDis = 0.0;
        }
      }
    }
    return true;
  }

  void loadDataProc() {
    while (true) {
      LoadingInfoPtr curInfoPtr;
      if (!loadingQueue.pop(curInfoPtr)) {
        break;
      } else {
        bool r = false;
        if (curInfoPtr->type == LOADING_COORD) {
          txPoint curPos = std::dynamic_pointer_cast<CoordInfo>(curInfoPtr)->coord;
          r = mapLoadPtr->loadData(curPos, MAP_RADIUS);
        } else if (curInfoPtr->type == LOADING_ROADID) {
          continue;
          //  roadpkid rid = std::dynamic_pointer_cast< RoadIdInfo >( curInfoPtr )->rid;
          //  r = mapLoadPtr->loadData( std::vector< roadpkid >( 1, rid ) );
        } else if (curInfoPtr->type == LOADING_PAUSE) {
          size_t curInterval = std::dynamic_pointer_cast<PauseInfo>(curInfoPtr)->pauseMS;
          if (curInterval > 0) std::this_thread::sleep_for(std::chrono::milliseconds(curInterval));
          continue;
        } else if (curInfoPtr->type == LOADING_EXIT) {
          break;
        } else if (curInfoPtr->type == LOADING_ROADLIST) {
          std::vector<roadpkid> rid_list = std::dynamic_pointer_cast<RoadListInfo>(curInfoPtr)->rids;
          r = mapLoadPtr->loadData(rid_list);
        } else {
          std::cout << "Loading Param Invalid" << std::endl;
          continue;
        }
        if (r) {
          txRoads addedRoads, removedRoads;
          txLaneLinks addedLinks, removedLinks;
          txObjects addedObjs, removedObjs;
          if (mapLoadPtr->getAddedMapData(addedRoads, addedLinks, addedObjs)) {
            mapProxyPtr->insertRoads(addedRoads);
            mapProxyPtr->insertLaneLinks(addedLinks);
            mapProxyPtr->insertObjects(addedObjs);
          }
          if (mapLoadPtr->getRemovedMapData(removedRoads, removedLinks, removedObjs)) {
            for (auto& roadPtr : removedRoads) mapProxyPtr->removeRoad(roadPtr->getId());
            for (auto& objPtr : removedObjs) mapProxyPtr->removeObject(objPtr->getId());
          }

          txJunctions addedJunctions, removedJunctions;
          if (mapLoadPtr->getAddedJuncData(addedJunctions)) mapProxyPtr->insertJunctions(addedJunctions);
          if (mapLoadPtr->getRemovedJuncData(removedJunctions)) {
            for (auto& juncPtr : removedJunctions) mapProxyPtr->removeJunction(juncPtr->getId());
          }

          std::vector<roadpkid> rids(addedRoads.size(), 0);
          for (size_t i = 0; i < rids.size(); ++i) rids[i] = addedRoads[i]->getId();
          ROUTING_LOG("Add");
          ROUTING_LOG(rids);

          rids.resize(removedRoads.size(), 0);
          for (size_t i = 0; i < rids.size(); ++i) rids[i] = removedRoads[i]->getId();
          ROUTING_LOG("Removed");
          ROUTING_LOG(rids);

          ROUTING_LOG("AddJuncs " + std::to_string(addedJunctions.size()));
          for (auto& juncPtr : addedJunctions) {
            ROUTING_LOG("Junc " + std::to_string(juncPtr->getId()));
            rids.clear();
            juncPtr->getEntranceRoads(rids);
            ROUTING_LOG("Entrance");
            ROUTING_LOG(rids);

            rids.clear();
            juncPtr->getExitRoads(rids);
            ROUTING_LOG("Exit");
            ROUTING_LOG(rids);

            rids.clear();
            juncPtr->getJuncRoads(rids);
            ROUTING_LOG("JuncRoad");
            ROUTING_LOG(rids);
          }

          ROUTING_LOG("RemoveJuncs " + std::to_string(removedJunctions.size()));
          for (auto& juncPtr : removedJunctions) ROUTING_LOG("Junc " + juncPtr->getId());

        } else {
          std::cout << "Load Data Failed" << std::endl;
        }
      }
    }
  }

  bool disToNextJunc(double& dis, junctionpkid& jId) {
    if (routingOn) {
      roadpkid jrId;
      if (dtPtr->nextJunc(jId, jrId)) {
        dis = dtPtr->disToRoad(jrId);
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  bool lightsInNextJunc(txObjects& lights) {
    if (routingOn) {
      junctionpkid jId;
      roadpkid jrId;
      if (dtPtr->nextJunc(jId, jrId)) {
        txRoadPtr juncRoadPtr = mapProxyPtr->getRoadById(jrId);
        txLanes juncLanes = juncRoadPtr->getSections()[0]->getLanes();
        std::vector<txLaneId> preLaneIds;
        std::unordered_set<lanepkid> laneIds;
        for (auto& lanePtr : juncLanes) {
          txLanes curPreLanes = mapProxyPtr->getPreLanes(lanePtr);
          for (auto& preLanePtr : curPreLanes) {
            if (laneIds.find(preLanePtr->getId()) == laneIds.end()) {
              preLaneIds.push_back(preLanePtr->getTxLaneId());
              laneIds.insert(preLanePtr->getId());
            }
          }
        }
        lights = mapProxyPtr->getObjects(std::vector<OBJECT_TYPE>(1, OBJECT_TYPE_TrafficLights), preLaneIds);
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
};

RoutingMap::RoutingMap(CoordType mapGeomCoordType, const std::string& dbPath)
    : ins(new RoutingMapData(mapGeomCoordType, dbPath)) {}

RoutingMap::~RoutingMap() {}

txMapHandle* RoutingMap::getMapHadle() { return ins->pHandle; }

bool RoutingMap::routingSync(const PointVec& poiLocs, txRoute& route) { return ins->routingSync(poiLocs, route); }

bool RoutingMap::routingAsync(const PointVec& poiLocs) { return false; }
RoutingMap::ParkingSpaceInfo::ParkingSpaceInfo(const ParkingSpaceInfo& p) {
  this->id = p.id;
  this->lane_id = p.lane_id;
  this->road_id = p.road_id;
  this->section_id = p.section_id;
  this->heading = p.heading;
  this->polygon = p.polygon;
}

bool RoutingMap::ParkingSpaceInfo::setValue(const txObjectPtr& objPtr) {
  if (objPtr != NULL) {
    std::vector<txLaneId> related_ids;
    objPtr->getReliedLaneIds(related_ids);

    this->polygon.clear();
    const txLineCurve* linePtr = dynamic_cast<const txLineCurve*>(objPtr->getGeom()->getGeometry());
    linePtr->getPoints(this->polygon);

    if (related_ids.size() > 0 && this->polygon.size() > 3) {
      this->id = objPtr->getId();
      // this->heading = dynamic_cast< const txLineCurve* >( objPtr->getGeom()->getGeometry() )->yaw(3);
      this->road_id = related_ids[0].roadId;
      this->section_id = related_ids[0].sectionId;
      this->lane_id = related_ids[0].laneId;

      Point2d dir;
      size_t b_index = size_t(0);                 // first point
      size_t e_index = this->polygon.size() - 1;  // end point,in current data: parking space is not as closed python
      txPoint b = this->polygon[b_index];
      txPoint e = this->polygon[e_index];

      hadmap::CoordType cdtype = linePtr->getCoordType();

      if (linePtr->getCoordType() == COORD_WGS84) {
        coord_trans_api::lonlat2enu(b.x, b.y, b.z, e.x, e.y, e.z);

      } else if (linePtr->getCoordType() == COORD_ENU) {
        b.x -= e.x;
        b.y -= e.y;
      } else {
        b.x -= e.x;
        b.y -= e.y;
      }

      dir.x = b.x;
      dir.y = b.y;

      bool isEnu = true;
      if (linePtr->getCoordType() == COORD_WGS84 || linePtr->getCoordType() == COORD_ENU) {
        isEnu = true;
      } else {
        isEnu = false;
      }
      // dir2yaw out angle in degree and reference east counter-clock
      double ang_deg = map_util::dir2yaw(dir, isEnu);
      // the out heading should in  [ -PI, PI ), east -> 0, north -> PI/2
      this->heading = map_util::angle2radian(ang_deg);  // ref to the east counter-clock
      return true;
    }
  }
  return false;
}

bool RoutingMap::getParkingSpaceInfo(const objectpkid& objId, ParkingSpaceInfo& parkInfo) {
  if (objId > 0) {
    txObjectPtr objPtr = ins->mapProxyPtr->getObject(objId);
    if (objPtr != NULL && parkInfo.setValue(objPtr)) {
      return true;
    }
  }
  return false;
}

bool RoutingMap::getParkingSpaceInfo(const txPoint& pos, ParkingSpaceInfo& parkInfo) {
  txObjectPtr objPtr = ins->mapProxyPtr->getObject(pos, hadmap::OBJECT_TYPE_ParkingSpace);
  if (objPtr != NULL && parkInfo.setValue(objPtr)) {
    return true;
  }
  return false;
}

bool RoutingMap::getParkingSpaceInfos(const roadpkid& road_id, std::vector<ParkingSpaceInfo>& parkInfos) {
  parkInfos.clear();
  txObjects objs =
      ins->mapProxyPtr->getObjects(std::vector<OBJECT_TYPE>(1, hadmap::OBJECT_TYPE_ParkingSpace),
                                   std::vector<txLaneId>(1, txLaneId(road_id, SECT_PKID_INVALID, LANE_PKID_INVALID)));
  for (auto& objPtr : objs) {
    ParkingSpaceInfo park;
    park.setValue(objPtr);
    parkInfos.push_back(park);
  }

  if (parkInfos.size() > 0) {
    return true;
  } else {
    return false;
  }
}

RoutingStatus RoutingMap::getRoute(txRoute& route) { return ins->getRoute(route); }

bool RoutingMap::updateLocation(const txPoint& pos) { return ins->updateLocation(pos); }

txRoadPtr RoutingMap::getRoad(const txRouteNode& routeNodeInfo) { return ins->getRoad(routeNodeInfo); }

txMapInterfacePtr RoutingMap::getMapInterface() { return ins->mapProxyPtr; }

bool RoutingMap::disToNextJunc(double& dis, junctionpkid& jId) {
  bool r = ins->disToNextJunc(dis, jId);
  return dis < 500.0;
}

bool RoutingMap::lightsInNextJunc(txObjects& lights) { return ins->lightsInNextJunc(lights); }
}  // namespace hadmap
