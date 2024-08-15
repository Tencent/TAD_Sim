// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "mapengine/engine_defs.h"
#include "mapengine/search_interface.h"

#include "db_codes.h"
#include "db_operation.h"

#include <unordered_set>

namespace hadmap {
class LocalSearch : public SearchInterface {
  // The LocalSearch class provides functions for searching and saving map data
 public:
  // Constructor
  // @param type: The type of map data
  // @param dbPath: The path to the map database
  LocalSearch(MAP_DATA_TYPE type, const std::string& dbPath);

  // Destructor
  ~LocalSearch();

 public:
  // Check if the connection to the map database is successful
  // @return: True if the connection is successful, false otherwise
  bool connSuccess();

  // Save the map data to a file
  // @param type: The type of map data
  // @param filePath: The path to the file where the map data will be saved
  // @return: True if the map data is saved successfully, false otherwise
  bool saveMap(const int type, std::string filePath);

  // save current map
  bool save() { return true; }

  // Get the last operation information
  // @return: The last operation information
  std::string getLastOptInfo();

  // Get lane data by a WGS84 longitude and latitude point
  // @param loc: The longitude and latitude point
  // @param radius: The search radius
  // @param lanePtr: The returned lane data
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getLane(const hadmap::txPoint& loc, double radius, hadmap::txLanePtr& lanePtr);

  // Get lanes data by lane ID
  // @param laneId: The lane ID
  // @param lanes: The returned lanes data
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getLanes(const hadmap::txLaneId& laneId, hadmap::txLanes& lanes);

  // Get lanes data in an envelope
  // @param envelope: The envelope, represented by two points: [0] left bottom, [1] right top
  // @param lanes: The returned lanes data
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getLanes(const hadmap::PointVec& envelope, hadmap::txLanes& lanes);

  // Get the next lanes of the current lane
  // @param curLanePtr: The current lane
  // @param nextLanes: The returned next lanes
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getNextLanes(txLanePtr curLanePtr, txLanes& nextLanes);

  // Get the previous lanes of the current lane
  // @param curLanePtr: The current lane
  // @param prevLanes: The returned previous lanes
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getPrevLanes(txLanePtr curLanePtr, txLanes& prevLanes);

  // Get the left lane of the current lane
  // @param curLanePtr: The current lane
  // @param leftLanePtr: The returned left lane
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getLeftLane(txLanePtr curLanePtr, txLanePtr& leftLanePtr);

  // Get the right lane of the current lane
  // @param curLanePtr: The current lane
  // @param rightLanePtr: The returned right lane
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getRightLane(txLanePtr curLanePtr, txLanePtr& rightLanePtr);

  // Get the maximum ID of lane boundaries
  // @param boundaryMaxId: The returned maximum ID of lane boundaries
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getBoundaryMaxId(hadmap::laneboundarypkid& boundaryMaxId);

  // Get lane boundaries data by ID
  // @param ids: The IDs of the lane boundaries
  // @param boundaries: The returned lane boundaries data
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getBoundaries(const std::vector<hadmap::laneboundarypkid>& ids, hadmap::txLaneBoundaries& boundaries);

  // Get section data by road ID
  // @param sectionId: The section ID
  // @param sections: The returned sections data
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getSections(const hadmap::txSectionId& sectionId, hadmap::txSections& sections);

  // Get the maximum ID of roads
  // @param roadMaxId: The returned maximum ID of roads
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getRoadMaxId(roadpkid& roadMaxId);

  // Get road data by road ID
  // @param roadId: The road ID
  // @param wholeData: Whether to get the whole data, including sections and lanes
  // @param roadPtr: The returned road data
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getRoad(const roadpkid& roadId, const bool& wholeData, hadmap::txRoadPtr& roadPtr);

  // Get roads data by envelope
  // @param envelope: The envelope, represented by two points: [0] left bottom, [1] right top
  // @param wholeData: Whether to get the whole data, including sections and lanes
  // @param roads: The returned roads data
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getRoads(const hadmap::PointVec& envelope, const bool& wholeData, hadmap::txRoads& roads);

  // Get roads data by ID
  // @param rids: The IDs of the roads
  // @param wholeData: Whether to get the whole data, including sections and lanes
  // @param roads: The returned roads data
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getRoads(const std::vector<roadpkid>& rids, const bool& wholeData, hadmap::txRoads& roads);

  // Get all roads in db
  // wholeData = true : get sections and lanes data belong to this road
  // wholeData = false : only road data
  int getRoads(const bool& wholeData, hadmap::txRoads& roads);

  // Function to get junction information given entrance and exit road IDs
  // @param entraceRoadId: The ID of the entrance road
  // @param exitRoadId: The ID of the exit road
  // @param roads: A reference to a txRoads object to store the retrieved road information
  // @param links: A reference to a txLaneLinks object to store the retrieved lane link information
  // @return: An integer indicating success (0) or failure (non-zero)
  int getJuncInfo(const roadpkid& entraceRoadId, const roadpkid& exitRoadId, hadmap::txRoads& roads,
                  hadmap::txLaneLinks& links);

  // Function to get junction information given a lane link ID
  // @param linkId: The ID of the lane link
  // @param roads: A reference to a txRoads object to store the retrieved road information
  // @param links: A reference to a txLaneLinks object to store the retrieved lane link information
  // @return: An integer indicating success (0) or failure (non-zero)
  int getJuncInfo(const lanelinkpkid& linkId, hadmap::txRoads& roads, hadmap::txLaneLinks& links);

  // Function to get the nearest lane link given a location and radius
  // @param loc: The location (txPoint) to search for the nearest lane link
  // @param radius: The search radius
  // @param lanelinkPtr: A reference to a txLaneLinkPtr object to store the retrieved lane link information
  // @return: An integer indicating success (0) or failure (non-zero)
  int getLaneLink(const hadmap::txPoint& loc, double radius, hadmap::txLaneLinkPtr& lanelinkPtr);

  // get LaneLink data by lanelinkpkid id
  // if id == PKID_INVALID, return TX_HADMAP_PARAM_ERROR
  int getLaneLink(const hadmap::lanelinkpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr);

  int getRoadLink(const hadmap::roadpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr);

  // get prev lanelinks of current lane by txLaneId
  // if current lane doesn't have  prev lanelinks in mapdata
  // return TX_HADMAP_DATA_EMPTY and lanelinks is empty
  int getPrevLaneLinks(const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks);

  // get Next lanelinks of current lane  by txLaneId
  // if current lane  doesn't have  Next lanelinks in mapdata
  // return TX_HADMAP_DATA_EMPTY and lanelinks is empty
  int getNextLaneLinks(const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks);

  // Get lanelinks by road id
  // fromRoadId != ROAD_PKID_INVALID && toRoadId != ROAD_PKID_INVALID : get lanelinks between fromRoad and toRoad
  // fromRoadId != ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID : get all lanelinks from fromRoad
  // fromRoadId == ROAD_PKID_INVALID && toRoadId != ROAD_PKID_INVALID : get all lanelinks to toRoad
  // fromRoadId == ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID : get all lanelinks
  int getLaneLinks(const roadpkid& fromRoadId, const roadpkid& toRoadId, hadmap::txLaneLinks& lanelinks);

  // get lanelinks by envelope
  // envelope -> [0] leftbottom, [1] righttop
  // x : lon, y : lat
  int getLaneLinks(const hadmap::PointVec& envelope, hadmap::txLaneLinks& lanelinks);

  // get lanelinks by roads
  int getLaneLinks(const std::vector<roadpkid>& roadIds, hadmap::txLaneLinks& lanelinks);

  // get lanelink max id
  int getLaneLinkMaxId(lanelinkpkid& linkMaxId);

  // get objects data in envelope
  // envelope -> [0] leftbottom, [1] righttop
  // x : lon, y : lat
  // types is empty -> get whole objects with any types
  int getObjects(const hadmap::PointVec& envelope, const std::vector<hadmap::OBJECT_TYPE>& types,
                 hadmap::txObjects& objects);

  // get objects data which is related to specified lanes
  // road id == ROAD_PKID_INVALID -> invalid
  // section id == SECT_PKID_INVALID -> invalid
  // lane id == LANE_PKID_INVALID -> invalid
  // laneIs is empty -> get whole objects with specified types
  // types is empty -> get whole objects with specified lanes
  int getObjects(const std::vector<hadmap::txLaneId>& laneIds, const std::vector<hadmap::OBJECT_TYPE>& types,
                 hadmap::txObjects& objects);

  // Function to get objects within a specified envelope and object types
  // @param envelope: A PointVec object representing the envelope to search for objects
  // @param types: A vector of OBJECT_SUB_TYPE objects representing the object types to search for
  // @param objects: A reference to a txObjects object to store the retrieved object information
  // @return: An integer indicating success (0) or failure (non-zero)
  int getObjects(const hadmap::PointVec& envelope, const std::vector<hadmap::OBJECT_SUB_TYPE>& types,
                 hadmap::txObjects& objects);

  // Function to get objects within specified lanes and object types
  // @param laneIds: A vector of txLaneId objects representing the lane IDs to search for objects
  // @param types: A vector of OBJECT_SUB_TYPE objects representing the object types to search for
  // @param objects: A reference to a txObjects object to store the retrieved object information
  // @return: An integer indicating success (0) or failure (non-zero)
  int getObjects(const std::vector<txLaneId>& laneIds, const std::vector<OBJECT_SUB_TYPE>& types, txObjects& objects);

  // Function to get all junctions
  // @param junctions: A reference to a txJunctions object to store the retrieved junction information
  // @return: An integer indicating success (0) or failure (non-zero)
  int getJunctions(hadmap::txJunctions& junctions);

  // Function to get a junction by its ID
  // @param jid: The ID of the junction to retrieve
  // @param junction: A reference to a txJunctionPtr object to store the retrieved junction information
  // @return: An integer indicating success (0) or failure (non-zero)
  int getJunction(junctionpkid jid, hadmap::txJunctionPtr& junction);

  // Function to get the header information
  // @param headerPtr: A reference to a txOdHeaderPtr object to store the retrieved header information
  // @return: An integer indicating success (0) or failure (non-zero)
  int getHeader(hadmap::txOdHeaderPtr& headerPtr);

  // Function to insert roads into the map
  // @param roads: A const reference to a txRoads object containing the road information to insert
  // @return: An integer indicating success (0) or failure (non-zero)
  int insertRoads(const hadmap::txRoads& roads);

  // Function to update roads in the map
  // @param roads: A const reference to a txRoads object containing the road information to update
  // @return: An integer indicating success (0) or failure (non-zero)
  int updateRoads(const hadmap::txRoads& roads);

  // Function to delete roads from the map
  // @param roads: A const reference to a txRoads object containing the road information to delete
  // @return: An integer indicating success (0) or failure (non-zero)
  int deleteRoads(const hadmap::txRoads& roads);

  // Function to insert lane links into the map
  // @param lanelinks: A const reference to a txLaneLinks object containing the lane link information to insert
  // @return: An integer indicating success (0) or failure (non-zero)
  int insertLaneLinks(const hadmap::txLaneLinks& lanelinks);

  // Function to update lane links in the map
  // @param lanelinks: A const reference to a txLaneLinks object containing the lane link information to update
  // @return: An integer indicating success (0) or failure (non-zero)
  int updateLaneLinks(const hadmap::txLaneLinks& lanelinks);

  // Function to delete lane links from the map
  // @param lanelinks: A const reference to a txLaneLinks object containing the lane link information to delete
  // @return: An integer indicating success (0) or failure (non-zero)
  int deleteLaneLinks(const hadmap::txLaneLinks& lanelinks);

  // Function to insert objects into the map
  // @param objects: A const reference to a txObjects object containing the object information to insert
  // @return: An integer indicating success (0) or failure (non-zero)
  int insertObjects(const hadmap::txObjects& objects);

  // Function to update objects in the map
  // @param objects: A const reference to a txObjects object containing the object information to update
  // @return: An integer indicating success (0) or failure (non-zero)
  int updateObjects(const hadmap::txObjects& objects);

  // Function to delete objects from the map
  // @param objects: A const reference to a txObjects object containing the object information to delete
  // @return: An integer indicating success (0) or failure (non-zero)
  int deleteObjects(const hadmap::txObjects& objects);
  // get refPoint
  hadmap::txPoint getRefPoint();

 private:
  // get pre | nex lanes
  // topoFlag = 1, get nex lanes
  // topoFlag = -1, get pre lanes
  int getTopoLanes(txLanePtr curLanePtr, const txPoint& tagPoint, const int& topoFlag, txLanes& lanes);

  // Function to get road IDs from the same intersections by seed
  // @param initEntranceIds: A vector of pairs containing roadpkid and lanepkid objects representing the initial
  // entrance IDs
  // @param initExitIds: A vector of pairs containing roadpkid and lanepkid objects representing the initial exit IDs
  // @param entranceIds: A reference to an unordered_set of roadpkid objects to store the retrieved entrance IDs
  // @param exitIds: A reference to an unordered_set of roadpkid objects to store the retrieved exit IDs
  void intersectionRoads(const std::vector<std::pair<roadpkid, lanepkid> >& initEntranceIds,
                         const std::vector<std::pair<roadpkid, lanepkid> >& initExitIds,
                         std::unordered_set<roadpkid>& entranceIds, std::unordered_set<roadpkid>& exitIds);

 private:
  // The path to the database
  std::string dbPath;
  // A pointer to a dbOperation object
  dbOperation* dbPtr;
  // The type of the map data
  MAP_DATA_TYPE dbType;
  // Additional information for the operation
  std::string optInfo;
};
}  // namespace hadmap
