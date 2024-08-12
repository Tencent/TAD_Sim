// Copyright 2024 Tencent Inc. All rights reserved.
//

// this module can build local map auto & supply interfaces for all upper applications
// before calling any interfaces, init function should be called first
// this module is multithread
// if coordType is ENU, then all the interfaces should be called in main loop

#pragma once
#include <memory>
#include <string>

#include "structs/base_struct.h"
#include "structs/hadmap_mapinterface.h"
#include "structs/hadmap_route.h"

#include "common/macros.h"
#include "routingmap/routingmap_types.h"

namespace hadmap {
// database operation handle
typedef struct txMapHandle txMapHandle;
class TXSIMMAP_API RoutingMap {
 public:
  class TXSIMMAP_API ParkingSpaceInfo {
   public:
    objectpkid id;
    double heading; /* yaw -> [ -PI, PI ), east -> 0, north -> PI/2 */
    roadpkid road_id;
    sectionpkid section_id;
    lanepkid lane_id;
    std::vector<txPoint> polygon;
    bool setValue(const txObjectPtr& objPtr);
    ParkingSpaceInfo(const ParkingSpaceInfo& p);
    ParkingSpaceInfo() : id(0), heading(0.0), road_id(0), section_id(0), lane_id(0) {}
  };

  // default construction
  // if routingmap.cfg is not in the default location, set it
  explicit RoutingMap(CoordType mapGeomCoordType, const std::string& dbPath = "");

  ~RoutingMap();

  RoutingMap(const RoutingMap&) = delete;

  txMapHandle* getMapHadle();

 public:
  // search path from hadmap
  // poiLocs are in WGS84 coord
  // this function will return after route complete and current road build complete
  bool routingSync(const PointVec& poiLocs, txRoute& route);

  // search path from hadmap
  // poiLocs are in WGS84 coord
  // this function will return immediately
  // get routing info by getRoute func
  bool routingAsync(const PointVec& poiLocs);
  // get ParkingSpaceInfo by objectId
  bool getParkingSpaceInfo(const objectpkid& objId, ParkingSpaceInfo& parkInfo);
  // get nearest ParkingSpaceInfo by position
  bool getParkingSpaceInfo(const txPoint& pos, ParkingSpaceInfo& parkInfo);
  // get ParkingSpaceInfos by types and road_id
  bool getParkingSpaceInfos(const roadpkid& road_id, std::vector<ParkingSpaceInfo>& parkInfos);

  // after routing function being called, you can get newest routing info by this function
  RoutingStatus getRoute(txRoute& route);

  // update center point
  // this function should be called each frame
  // routing map will update local map based on center point
  // center point should be in wgs84 coord
  bool updateLocation(const txPoint& pos);

  // get route road info
  // road data may be cutted by node info
  // if there is no road match node info, then return null
  txRoadPtr getRoad(const txRouteNode& routeNodeInfo);

  // distance to next junction
  // if there is no junction of dis > 500m, return false
  // else return distance and junction pkid
  bool disToNextJunc(double& dis, junctionpkid& jId);

  // traffic lights in next junction
  // if there is no juntion, return false
  // if no traffic lights, return true, but lights is empty
  bool lightsInNextJunc(txObjects& lights);

  // get map query interface
  txMapInterfacePtr getMapInterface();

 private:
  struct RoutingMapData;
  std::unique_ptr<RoutingMapData> ins;
};
}  // namespace hadmap
