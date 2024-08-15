// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <set>
#include <unordered_set>
#include "structs/base_struct.h"
#include "structs/hadmap_header.h"
#include "structs/hadmap_junction.h"
#include "structs/hadmap_lanelink.h"
#include "structs/hadmap_mapinterface.h"
#include "structs/hadmap_road.h"

namespace hadmap {
class txMap;
using txMapPtr = std::shared_ptr<txMap>;

class TXSIMMAP_API txMap : public txMapInterface {
 private:
  struct MapData;
  std::unique_ptr<MapData> instancePtr;

 private:
  txMap(const txMap&) = delete;

  txMap& operator=(const txMap&) = delete;

 public:
  // if type == COORD_XYZ -> all geom and rtree is in Cartesian
  //            -> used for perception map generation
  // if type == COORD_WGS84
  //    if useLocalCoord is true -> geom data is in enu coord, rtree is in mercator
  //    if useLocalCoord is false -> geom data is in wgs84, rtree is in mercator
  explicit txMap(CoordType type = COORD_ENU, const txPoint& centerLL = txPoint(0.0, 0.0, 0.0));

  ~txMap();

  // Update the map center location
  // The coordinate of the map is ENU
  // Call this function in the main loop
  bool updateCenter(const txPoint& lonlatele);

  // Insert a reference point
  bool indertRefPoint(const hadmap::txPoint& point);

  // Get the center ID
  size_t centerId();

  // Get the center point
  const txPoint& getCenter();

  // Interface
  CoordType getGeomCoordType();

  // Get the projection
  txProjectionConstPtr getProjection();

  // Get a road by its ID
  txRoadPtr getRoadById(const roadpkid& roadId);

  // Get a lane by its ID
  txLanePtr getLaneById(const txLaneId& laneId);

  // Get the lanes within a given radius of a position
  txLanes getLanes(const txPoint& pos, const double& radius);

  // Get the nearest lane to a given position within a given radius
  txLanePtr nearestLane(const txPoint& pos, double radius);

  // Get the nearest lane to a given position within a given radius and yaw
  txLanePtr nearestLane(const txPoint& pos, double radius, const double& yaw);

  // Get the nearest lane link to a given position within a given radius
  txLaneLinkPtr nearestLaneLink(const txPoint& pos, double radius);

  // Get the next lanes of a given lane
  txLanes getNextLanes(const txLanePtr& lanePtr);

  // Get the previous lanes of a given lane
  txLanes getPreLanes(const txLanePtr& lanePtr);

  // Get the left lane of a given lane
  txLanePtr getLeftLane(const txLanePtr& lanePtr);

  // Get the right lane of a given lane
  txLanePtr getRightLane(const txLanePtr& lanePtr);

  // Get the lane boundaries at a given position, yaw, and range
  txLaneBoundaries getLaneBoundaries(const txPoint& pos, const double& yaw, const std::pair<double, double>& range);

  // Get an object by its ID
  txObjectPtr getObject(const objectpkid& objId);

  // Get an object of a given type at a given position
  txObjectPtr getObject(const txPoint& pos, const OBJECT_TYPE& obj_type);

  // Get objects within a given envelope and of given types
  txObjects getObjects(const PointVec& envelope, const std::vector<OBJECT_TYPE>& objTypes);

  // Get objects within a given envelope, yaw, and of given types
  txObjects getObjects(const PointVec& envelope, const double& yaw, const std::vector<OBJECT_TYPE>& objTypes);

  // Get objects within a given range, yaw, and of given types
  txObjects getObjects(const txPoint& pos, const double& yaw, const std::pair<double, double>& range,
                       const std::vector<OBJECT_TYPE>& objTypes);

  // Get objects of given types related to given lane IDs
  txObjects getObjects(const std::vector<OBJECT_TYPE>& objTypes, const std::vector<txLaneId>& relIds);

  // Get objects of given subtypes related to given lane IDs
  txObjects getObjects(const std::vector<OBJECT_SUB_TYPE>& objTypes, const std::vector<txLaneId>& relIds);

  // Get a junction by its ID
  txJunctionPtr getJunction(const junctionpkid& jId);

  // Get all junctions
  txJunctions getJunctions();

  // Compare the priority of two junction roads
  PRIORITY_TYPE priorityCmp(const txJunctionRoadPtr& curPtr, const txJunctionRoadPtr& cmpPtr);

  // Set the header
  void setHeader(const txOdHeaderPtr& header);

  // Get the header
  txOdHeaderPtr getHeader();

  // Get the reference point
  txPoint getRefPoint();

 public:
  // get info
  // call in main loop

  // get whole roads
  bool getRoads(txRoads& roads);

  // get whole lanes
  bool getLanes(txLanes& lanes);

  // get whole boundaries
  bool getBoundaries(txLaneBoundaries& boundaries);

  // get whole links
  bool getLaneLinks(txLaneLinks& links);

  // get whole objects
  bool getObjects(txObjects& objects);

  // get whole specified objects
  bool getObjects(OBJECT_TYPE type, txObjects& objects);

  bool getJunctions(txJunctions& juncs);

 public:
  // search map by spatialindex
  // call in main loop

  // search nearest lane by loc
  // loc coord is same as map geom coord
  // radius -> meter
  bool nearestLane(const txPoint& loc, double radius, txLanePtr& lanePtr);

  // search lanes by envelope
  // envelope -> [ leftbottom, righttop ]
  // loc coord is same as map geom coord
  bool searchLanes(const PointVec& envelope, txLanes& lanes);

  // search boundaries by envelope
  // envelope -> [ leftbottom, righttop ]
  // loc coord is same as map geom coord
  bool searchBoundaries(const PointVec& envelope, txLaneBoundaries& boundaries);

  // get boundaries by distance
  // dis > 0, get boundaries in front of cur pos
  // dis < 0, get boundaries behind cur pos
  bool getBoundaries(const txPoint& pos, const double& yaw, const double& dis, txLaneBoundaries& boundaries);

  // search roads by envelope
  // envelope -> [ leftbottom, righttop ]
  // loc coord is same as map geom coord
  bool searchRoads(const PointVec& envelope, txRoads& roads);

  // search objects by envelope
  bool searchObjects(const PointVec& envelope, const std::set<OBJECT_TYPE>& types, const txLaneId& relLaneId,
                     txObjects& objects);

  bool searchObjects(const PointVec& envelope, const std::set<OBJECT_SUB_TYPE>& types, const txLaneId& relLaneId,
                     txObjects& objects);

  // search specified lane width
  bool laneWidth(const txPoint& loc, double& dis2Left, double& dis2Right);

  // search specified road width
  bool roadWidth(const txPoint& loc, double& dis2Left, double& dis2Right);

 public:
  // search map by index
  // call in main loop

  // search next lanes
  bool nextLanes(const txLaneId& curLaneId, txLanes& lanes);

  // search specified lanes
  // roadId, sectionId, laneId != INVALID, then get specified lane
  // roadId, sectionId != INVALID, then get specified lanes in specified section
  // roadId != INVALID, then get specified lanes in specified road
  // else return false
  bool specLanes(const txLaneId& specLaneId, txLanes& lanes);

  // search boundaries by id
  bool specBoundaries(const std::vector<laneboundarypkid>& ids, txLaneBoundaries& boundaries);

  // search lanelinks by id
  // fromRoadId != ROAD_PKID_INVALID && toRoadId != ROAD_PKID_INVALID : get lanelinks between fromRoad and toRoad
  // fromRoadId != ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID : get all lanelinks from fromRoad
  // fromRoadId == ROAD_PKID_INVALID && toRoadId != ROAD_PKID_INVALID : get all lanelinks to toRoad
  // fromRoadId & toRoadId == ROAD_PKID_INVALID: return false
  bool specLinks(const roadpkid& fromRoadId, const roadpkid& toRoadId, txLaneLinks& links);

  // search lanelinks by txLaneId
  // fromLaneId.roadId != ROAD_PKID_INVALID && toLaneId.roadId != ROAD_PKID_INVALID :
  //      get lanelinks exectly between fromLaneId(roadId,sectionId,laneId) and toLaneId ( roadId,sectionId,laneId)
  // fromLaneId.roadId != ROAD_PKID_INVALID &&  toLaneId.roadId  == ROAD_PKID_INVALID :
  //    get all lanelinks from  fromLaneId(roadId,sectionId,laneId)
  // fromLaneId.roadId == ROAD_PKID_INVALID && toLaneId.roadId  != ROAD_PKID_INVALID :
  // get all lanelinks to toLaneId ( roadId,sectionId,laneId)
  bool specLinks(const txLaneId& fromLaneId, const txLaneId& toLaneId, txLaneLinks& links);

  // search lanelinks by link id
  bool specLinks(const std::vector<lanelinkpkid>& link_ids, txLaneLinks& links);

  bool specRoadLink(const roadpkid roadid, txLaneLinkPtr& link);

  bool specRoadLinks(const roadpkid roadid, txLaneLinks& links);
  // search objects by relation
  bool relObjects(const txLaneId& curLaneId, const std::set<OBJECT_TYPE>& types, txObjects& objects);

  // get junction info by id
  bool specJunction(const junctionpkid& jId, txJunctionPtr& juncPtr);

 public:
  // set info
  // if map coord type is COORD_XYZ, then all the data of geom must be transformed into COORD_XYZ before insertion
  // if map coord type is COORD_WGS84, then all the data of geom must be transformed into COORD_WGS84 before insertion

  // insert road data
  // with section info, lane info, laneboundary info
  bool insertRoad(const txRoadPtr roadPtr);

  // insert roads data
  bool insertRoads(const txRoads& roads);

  // insert lanelink
  bool insertLaneLink(const txLaneLinkPtr linkPtr);

  // insert lanelinks
  bool insertLaneLinks(const txLaneLinks& links);

  // insert object
  bool insertObject(const txObjectPtr objectPtr);

  // insert objects
  bool insertObjects(const txObjects& objects);

  // insert junction info
  bool insertJunction(const txJunctionPtr juncPtr);

  // insert junctions
  bool insertJunctions(const txJunctions& juncs);

 public:
  // update info
  // update road data
  bool updateRoad(const txRoadPtr& road);

  // update roads data
  bool updateRoads(const txRoads& roads);

  // update lane link data
  bool updateLaneLink(const txLaneLinkPtr& lanelinks);

  // update lane links data
  bool updateLaneLinks(const txLaneLinks& lanelinks);

 public:
  // remove map data

  // remove road by id
  // remove lanelinks and objects related to this road
  bool removeRoad(roadpkid roadId);

  // remove roads by ids
  // remove lanelinks and objects related to these roads
  bool removeRoads(const std::vector<roadpkid>& roadIds);

  // remove lanelinks by lanelinkid
  bool removeLinks(const std::vector<lanelinkpkid>& lanelinkid);

  // remove lanelinks by id
  // specified fromRoadId and toRoadId, remove all links between
  // fromRoadId == ROAD_PKID_INVALID, remove all links to toRoadId
  // toRaodId == ROAD_PKID_INVALID, remove all links from fromRoadId
  // fromRoadId == ROAD_PKID_INVALID and toRoadId == ROAD_PKID_INVALID, remove all links in map
  bool removeLinks(roadpkid fromRoadId, roadpkid toRoadId);

  // remove object by id
  bool removeObject(objectpkid objectId);

  // remove objects by ids
  bool removeObjects(const std::vector<objectpkid>& objectIds);

  // remove junction by id
  bool removeJunction(junctionpkid jId);
};
}  // namespace hadmap
