// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "OpenDriveOutput.h"
#include "mapengine/engine_defs.h"
#include "mapengine/search_interface.h"
#include "structs/hadmap_header.h"
#include "structs/hadmap_map.h"
namespace hadmap {
using AreaPointVec = std::vector<std::tuple<hadmap::txPoint, hadmap::txPoint>>;
class MemSearch : public SearchInterface {
 public:
  // Constructor
  explicit MemSearch(MAP_DATA_TYPE type);

  // Destructor
  virtual ~MemSearch();

 public:
  // Check if the connection to the map database is successful
  // @return: True if the connection is successful, false otherwise
  bool connSuccess();

  // Init file
  bool initFile(const std::string& filePath);

  // Save the map data to a file
  // @param type: The type of map data
  // @param filePath: The path to the file where the map data will be saved
  // @return: True if the map data is saved successfully, false otherwise
  bool saveMap(const int type, std::string filePath);

  // save current map
  bool save();

  // Get the last operation information
  // @return: The last operation information
  std::string getLastOptInfo();

  // Get lane data by a WGS84 longitude and latitude point
  // @param loc: The longitude and latitude point
  // @param radius: The search radius
  // @param lanePtr: The returned lane data
  // @return: The result of the function execution, 0 indicates success, and other values indicate errors
  int getLane(const hadmap::txPoint& loc, double radius, hadmap::txLanePtr& lanePtr);

  // get lanes data by lane id
  // road id == ROAD_PKID_INVALID -> invalid
  // section id == SECT_PKID_INVALID -> invalid
  // lane id == LANE_PKID_INVALID -> invalid
  // without boundary data
  int getLanes(const hadmap::txLaneId& laneId, hadmap::txLanes& lanes);

  // get lanes data in envelope
  // without boundary data
  // envelope -> [0] leftbottom, [1] righttop
  // x : lon, y : lat
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

  // get lane boundaries data by id
  // if ids is empty, then return whole boundary data
  int getBoundaries(const std::vector<hadmap::laneboundarypkid>& ids, hadmap::txLaneBoundaries& boundaries);

  // get section data by road id
  // if roadId == ROAD_PKID_INVALID, return TX_HADMAP_PARAM_ERROR
  // if sectionId == SECT_PKID_INVALID : get whole sections in specified road
  // else : get specified section
  int getSections(const hadmap::txSectionId& sectionId, hadmap::txSections& sections);

  // get road max id
  int getRoadMaxId(roadpkid& roadMaxId);

  // get road data by road id
  // if roadId == ROAD_PKID_INVALID, return TX_HADMAP_PARAM_ERROR
  // wholeData = true : get sections and lanes data belong to this road
  // wholeData = false : only road data
  int getRoad(const roadpkid& roadId, const bool& wholeData, hadmap::txRoadPtr& roadPtr);

  // get roads data by envelope
  // envelope -> [0] leftbottom, [1] righttop
  // x : lon, y : lat
  // wholeData = true : get sections and lanes data belong to this road
  // wholeData = false : only road data
  int getRoads(const hadmap::PointVec& envelope, const bool& wholeData, hadmap::txRoads& roads);

  // get roads by id
  // if rids is empty, then return all roads in db
  // wholeData = true : get sections and lanes data belong to this road
  // wholeData = false : only road data
  int getRoads(const std::vector<roadpkid>& rids, const bool& wholeData, hadmap::txRoads& roads);

  // get all roads in db
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

  // get lanelinks by road id
  // fromRoadId != ROAD_PKID_INVALID && toRoadId != ROAD_PKID_INVALID : get lanelinks between fromRoad and toRoad
  // fromRoadId != ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID : get all lanelinks from fromRoad
  // fromRoadId == ROAD_PKID_INVALID && toRoadId != ROAD_PKID_INVALID : get all lanelinks to toRoad
  // fromRoadId == ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID : get all lanelinks
  int getLaneLinks(const roadpkid& fromRoadId, const roadpkid& toRoadId, hadmap::txLaneLinks& lanelinks);

  // get lanelinks by envelope
  // envelope -> [0] leftbottom, [1] righttop
  // x : lon, y : lat
  int getLaneLinks(const hadmap::PointVec& envelope, hadmap::txLaneLinks& lanelinks);

  // get lanelinks by road
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

  hadmap::txPoint getRefPoint();

  // initialize
  virtual void init() = 0;

 private:
  void intersectionRoads(const std::vector<std::pair<roadpkid, lanepkid>>& initEntranceIds,
                         const std::vector<std::pair<roadpkid, lanepkid>>& initExitIds,
                         std::unordered_set<roadpkid>& entranceIds, std::unordered_set<roadpkid>& exitIds);

 protected:
  /// Member variables for the class
  // The type of the map data
  MAP_DATA_TYPE dataType;
  // The path to the map file
  std::string mapfilePath;
  // A PointVec object representing the current envelope
  PointVec curEnvelope;
  // Additional information for the operation
  std::string optInfo;
  // A pointer to a txMap object
  txMapPtr mapPtr;
  // Opendrive output
  std::shared_ptr<hadmap::OpenDriveOutput> mOutPtr = NULL;
};
}  // namespace hadmap
