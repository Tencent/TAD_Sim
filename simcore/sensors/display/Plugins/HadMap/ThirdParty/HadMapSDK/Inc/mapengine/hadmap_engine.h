// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "common/macros.h"
#include "structs/hadmap_lane.h"
#include "structs/hadmap_lanelink.h"
#include "structs/hadmap_object.h"
#include "structs/hadmap_road.h"
#include "structs/hadmap_section.h"
#include "types/map_defs.h"
#include "types/map_types.h"
// #include "structs/hadmap_header.h"

#include "mapengine/engine_defs.h"

namespace hadmap {
// database operation handle
typedef struct txMapHandle txMapHandle;

// connect hadmap database
// type = SQLITE | MYSQL -> fileInfo is dbPath
// type = REMOTE -> fileInfo is dbName
TXSIMMAP_API int hadmapConnect(const char* fileInfo, const MAP_DATA_TYPE& type, txMapHandle** ppHandle);

// connect hadmap database
// type = SQLITE | MYSQL -> fileInfo is dbPath
// type = REMOTE -> fileInfo is dbName
TXSIMMAP_API int hadmapConnect(const char* fileInfo, const MAP_DATA_TYPE& type, txMapHandle** ppHandle,
                               bool doubleRoad);

// Connect to the HD map
// @param fileInfo: File information, including the database path and file name
// @param map_range_list: A list of map ranges, containing the left bottom and right top coordinates of multiple map
// ranges
// @param type: The type of map
// @param ppHandle: The returned map handle, used for subsequent operations
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
TXSIMMAP_API int hadmapConnect(
    const char* fileInfo,
    std::vector<std::tuple<hadmap::txPoint /*left_bottom*/, hadmap::txPoint /*right_top*/>> map_range_list,
    const MAP_DATA_TYPE& type, txMapHandle** ppHandle);

// Map edit to save map
TXSIMMAP_API int hadmapSave(txMapHandle* ppHandle);

// close connection
TXSIMMAP_API int hadmapClose(txMapHandle** ppHandle);

// create a new hd map
// fileInfo is outfile path
TXSIMMAP_API int hadmapOutput(const char* fileInfo, const MAP_DATA_TYPE& type, txMapHandle* ppHandle);

// get sdk version
TXSIMMAP_API std::string version();

// get last opt info
TXSIMMAP_API std::string getLastOptInfo(txMapHandle* pHandle);

// get lane data by lonlat point
// loc with WGS84, ignore Z when z<-9999
// without boundary data
TXSIMMAP_API int getLane(txMapHandle* pHandle, const hadmap::txPoint& loc, hadmap::txLanePtr& lanePtr,
                         double radius = 5.0);

// get lane data by lane id
// road id, section id, lane id
// if id == PKID_INVALID, return TX_HADMAP_PARAM_ERROR
// without boundary data
TXSIMMAP_API int getLane(txMapHandle* pHandle, const hadmap::txLaneId& laneId, hadmap::txLanePtr& lanePtr);

// get lanes data by lane id
// road id == ROAD_PKID_INVALID -> invalid
// section id == SECT_PKID_INVALID -> invalid
// lane id == LANE_PKID_INVALID -> invalid
// without boundary data
TXSIMMAP_API int getLanes(txMapHandle* pHandle, const hadmap::txLaneId& laneId, hadmap::txLanes& lanes);

// get lanes data in envelope
// without boundary data
// envelope -> [0] leftbottom, [1] righttop
// x : lon, y : lat
TXSIMMAP_API int getLanes(txMapHandle* pHandle, const hadmap::PointVec& envelope, hadmap::txLanes& lanes);

// get lanes data in specified area
// without boundary data
// center -> WGS84
// radius -> km
TXSIMMAP_API int getLanes(txMapHandle* pHandle, const hadmap::txPoint& center, const double& radius,
                          hadmap::txLanes& lanes);

// get topo of lanes

// get next lanes
// if cur lane is in the last section and related to junction
// return TX_HADMAP_DATA_EMPTY and nextLanes is emtpy
TXSIMMAP_API int getNextLanes(txMapHandle* pHandle, txLanePtr curLanePtr, txLanes& nextLanes);

// get prev lanes
// if cur lane is in the first section and related to junction
// return TX_HADMAP_DATA_EMPTY and prevLanes is empty
TXSIMMAP_API int getPrevLanes(txMapHandle* pHandle, txLanePtr curLanePtr, txLanes& prevLanes);

// get left lane
// if cur lane is the most left lane, then return TX_HADMAP_DATA_EMPTY
// if curroad is biodirection, cur lane id is -1, then left lane id is 1
// if curroad is biodirection, cur lane id is 1, then left lane id is -1
TXSIMMAP_API int getLeftLane(txMapHandle* pHandle, txLanePtr curLanePtr, txLanePtr& leftLanePtr);

// get right lane
// if cur lane is the most right lane, then return TX_HADMAP_DATA_EMPTY
TXSIMMAP_API int getRightLane(txMapHandle* pHandle, txLanePtr curLanePtr, txLanePtr& rightLanePtr);

// get lane boundary
TXSIMMAP_API int getBoundaryMaxId(txMapHandle* pHandle, hadmap::laneboundarypkid& boundaryMaxId);

// get lane boundary data by boundary id
// if boundaryId == BOUN_PKID_INVALID, return TX_HADMAP_PARAM_ERROR
TXSIMMAP_API int getBoundary(txMapHandle* pHandle, const laneboundarypkid& boundaryId,
                             hadmap::txLaneBoundaryPtr& boundaryPtr);

// get lane boundaries data by id
// if ids is empty, then return whole boundary data
TXSIMMAP_API int getBoundaries(txMapHandle* pHandle, const std::vector<uint64_t>& ids,
                               hadmap::txLaneBoundaries& boundaries);

// get section data by road id
// if roadId == ROAD_PKID_INVALID, return TX_HADMAP_PARAM_ERROR
// if sectionId == SECT_PKID_INVALID : get whole sections in specified road
// else : get specified section
TXSIMMAP_API int getSections(txMapHandle* pHandle, const hadmap::txSectionId& sectionId, hadmap::txSections& sections);

// get road max id
TXSIMMAP_API int getRoadMaxId(txMapHandle* pHandle, roadpkid& roadMaxId);

// get road data by road id
// if roadId == ROAD_PKID_INVALID, return TX_HADMAP_PARAM_ERROR
// wholeData = true : get sections and lanes data belong to this road
// wholeData = false : only road data
TXSIMMAP_API int getRoad(txMapHandle* pHandle, const roadpkid& roadId, const bool& wholeData,
                         hadmap::txRoadPtr& roadPtr);

// get roads data by envelope
// envelope -> [0] leftbottom, [1] righttop
// x : lon, y : lat
// wholeData = true : get sections and lanes data belong to this road
// wholeData = false : only road data
TXSIMMAP_API int getRoads(txMapHandle* pHandle, const hadmap::PointVec& envelope, const bool& wholeData,
                          hadmap::txRoads& roads);

// get roads by id
// if rids is empty, then return all roads in db
// wholeData = true : get sections and lanes data belong to this road
// wholeData = false : only road data
TXSIMMAP_API int getRoads(txMapHandle* pHandle, const std::vector<roadpkid>& rids, const bool& wholeData,
                          hadmap::txRoads& roads);

// get all roads in db
// wholeData = true : get sections and lanes data belong to this road
// wholeData = false : only road data
TXSIMMAP_API int getRoads(txMapHandle* pHandle, const bool& wholeData, hadmap::txRoads& roads);

// get roads and links in specified junction
// entranceRoadId : road enter into junction
// exitRoadId : road exit from junction
// entranceRoadId != ROAD_PKID_INVALID && exitRoadId == ROAD_PKID_INVALID : junction is specified by entranceRoadId
// entranceRoadId == ROAD_PKID_INVALID && exitRoadId != ROAD_PKID_INVALID : junction is specified by exitRoadId
// entranceRoadId != ROAD_PKID_INVALID && exitRoadId != ROAD_PKID_INVALID : junction is specified by both
//                                      if
// roads are not around the same junction, then return TX_HADMAP_PARAM_ERROR
// entranceRoadId == ROAD_PKID_INVALID && exitRoadId == ROAD_PKID_INVALID : return TX_HADMAP_PARAM_ERROR
TXSIMMAP_API int getJuncInfo(txMapHandle* pHandle, const roadpkid& entraceRoadId, const roadpkid& exitRoadId,
                             hadmap::txRoads& roads, hadmap::txLaneLinks& links);

// get roads and links in specified junction
// if linkId == LINK_PKID_INVALID or not in db : return TX_HADMAP_PARAM_ERROR
TXSIMMAP_API int getJuncInfo(txMapHandle* pHandle, const lanelinkpkid& linkId, hadmap::txRoads& roads,
                             hadmap::txLaneLinks& links);

// get lanelink (having geometry) data by lonlat point
// Search radius： 3 m
// loc with WGS84, ignore Z when z<-9999

TXSIMMAP_API int getLaneLink(txMapHandle* pHandle, const hadmap::txPoint& loc, hadmap::txLaneLinkPtr& lanelinkPtr,
                             double radius = 5.0);

// get LaneLink data by lanelinkpkid id
// if id == PKID_INVALID, return TX_HADMAP_PARAM_ERROR
TXSIMMAP_API int getRoadLaneLink(txMapHandle* pHandle, const hadmap::roadpkid& roadpkid,
                                 hadmap::txLaneLinkPtr& lanelinkPtr);

// get LaneLink data by lanelinkpkid id
// if id == PKID_INVALID, return TX_HADMAP_PARAM_ERROR
TXSIMMAP_API int getLaneLink(txMapHandle* pHandle, const hadmap::lanelinkpkid& lanelinkId,
                             hadmap::txLaneLinkPtr& lanelinkPtr);

// get prev lanelinks of current lane by txLaneId
// if current lane doesn't have  prev lanelinks in mapdata
// return TX_HADMAP_DATA_EMPTY and lanelinks is empty
TXSIMMAP_API int getPrevLaneLinks(txMapHandle* pHandle, const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks);

// get Next lanelinks of current lane  by txLaneId
// if current lane  doesn't have  Next lanelinks in mapdata
// return TX_HADMAP_DATA_EMPTY and lanelinks is empty
TXSIMMAP_API int getNextLaneLinks(txMapHandle* pHandle, const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks);

// get lanelinks by road id
// fromRoadId != ROAD_PKID_INVALID && toRoadId != ROAD_PKID_INVALID : get lanelinks between fromRoad and toRoad
// fromRoadId != ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID : get all lanelinks from fromRoad
// fromRoadId == ROAD_PKID_INVALID && toRoadId != ROAD_PKID_INVALID : get all lanelinks to toRoad
// fromRoadId == ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID : get all lanelinks
TXSIMMAP_API int getLaneLinks(txMapHandle* pHandle, const roadpkid& fromRoadId, const roadpkid& toRoadId,
                              hadmap::txLaneLinks& lanelinks);

// get lanelinks by envelope
// envelope -> [0] leftbottom, [1] righttop
// x : lon, y : lat
TXSIMMAP_API int getLaneLinks(txMapHandle* pHandle, const hadmap::PointVec& envelope, hadmap::txLaneLinks& lanelinks);

// get lanelinks by roads
// from specified road or to specified road
// if roadIds is empty, return TX_HADMAP_PARAM_ERROR
TXSIMMAP_API int getLaneLinks(txMapHandle* pHandle, const std::vector<roadpkid>& roadIds,
                              hadmap::txLaneLinks& lanelinks);

// get lanelink max id
TXSIMMAP_API int getLaneLinkMaxId(txMapHandle* pHandle, lanelinkpkid& linkMaxId);

// get objects data in envelope
// envelope -> [0] leftbottom, [1] righttop
// x : lon, y : lat
// types is empty -> get whole objects with any types
TXSIMMAP_API int getObjects(txMapHandle* pHandle, const hadmap::PointVec& envelope,
                            const std::vector<hadmap::OBJECT_TYPE>& types, hadmap::txObjects& objects);

// get objects data which is related to specified lanes
// road id == ROAD_PKID_INVALID -> invalid
// section id == SECT_PKID_INVALID -> invalid
// lane id == LANE_PKID_INVALID -> invalid
// laneIs is empty -> get whole objects with specified types
// types is empty -> get whole objects with specified lanes
TXSIMMAP_API int getObjects(txMapHandle* pHandle, const std::vector<hadmap::txLaneId>& laneIds,
                            const std::vector<hadmap::OBJECT_TYPE>& types, hadmap::txObjects& objects);

// get objects data in envelope
// envelope -> [0] leftbottom, [1] righttop  x : lon, y : lat
// subtypes is empty -> get whole objects with any types
TXSIMMAP_API int getSubObjects(txMapHandle* pHandle, const hadmap::PointVec& envelope,
                               const std::vector<hadmap::OBJECT_SUB_TYPE>& subtypes, hadmap::txObjects& objects);

// get objects data which is related to specified lanes
// laneIs is empty -> get whole objects with specified types
// subtypes is empty -> get whole objects with specified lanes
TXSIMMAP_API int getSubObjects(txMapHandle* pHandle, const std::vector<hadmap::txLaneId>& laneIds,
                               const std::vector<hadmap::OBJECT_SUB_TYPE>& subtypes, hadmap::txObjects& objects);

// get all junction
// only support opendrive
TXSIMMAP_API int getJunctions(txMapHandle* pHandle, hadmap::txJunctions& junctions);

// get all junction
// only support opendrive
TXSIMMAP_API int getJunction(txMapHandle* pHandle, junctionpkid jid, hadmap::txJunctionPtr& junction);

// get path points frome p to faraway
// p: lon lat atl
// dis: distance of faraway. negative num means going back
// sampleDis: sampling returned pts.
// laneIds: return passing lane ids. when the lane`s sec and lane id is invalid, it means link id.
TXSIMMAP_API int getForwardPoints(txMapHandle* pHandle, const hadmap::txPoint& p, double dis,
                                  std::vector<std::pair<double, hadmap::PointVec>>& pointArray, double sampleDis = 0,
                                  std::vector<std::vector<hadmap::txLaneId>>* laneIds = nullptr);

// Get the header information of the map
// @param pHandle: The map handle
// @param headerPtr: The returned header information
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
TXSIMMAP_API int getHeader(txMapHandle* pHandle, txOdHeaderPtr& headerPtr);

// Insert roads into the map
// @param pHandle: The map handle
// @param roads: The roads to be inserted
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
TXSIMMAP_API int insertRoads(txMapHandle* pHandle, const hadmap::txRoads& roads);

// Update roads in the map (deprecated, please use updateRoads() instead)
// @param pHandle: The map handle
// @param roads: The roads to be updated
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
[[deprecated("For naming rules, please use updateRoads() instead.")]] TXSIMMAP_API int upDateRoads(
    txMapHandle* pHandle, const hadmap::txRoads& roads);

// Update roads in the map
// @param pHandle: The map handle
// @param roads: The roads to be updated
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
TXSIMMAP_API int updateRoads(txMapHandle* pHandle, const hadmap::txRoads& roads);

// Delete roads from the map
// @param pHandle: The map handle
// @param roads: The roads to be deleted
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
TXSIMMAP_API int deleteRoads(txMapHandle* pHandle, const hadmap::txRoads& roads);

// Insert lane links into the map
// @param pHandle: The map handle
// @param lanelinks: The lane links to be inserted
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
TXSIMMAP_API int insertLaneLinks(txMapHandle* pHandle, const hadmap::txLaneLinks& lanelinks);

// Update lane links in the map
// @param pHandle: The map handle
// @param lanelinks: The lane links to be updated
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
TXSIMMAP_API int updateLaneLinks(txMapHandle* pHandle, const hadmap::txLaneLinks& lanelinks);

// Delete lane links from the map
// @param pHandle: The map handle
// @param lanelinks: The lane links to be deleted
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
TXSIMMAP_API int deleteLaneLinks(txMapHandle* pHandle, const hadmap::txLaneLinks& lanelinks);

// Insert objects into the map
// @param pHandle: The map handle
// @param objects: The objects to be inserted
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
TXSIMMAP_API int insertObjects(txMapHandle* pHandle, const hadmap::txObjects& objects);

// Update objects in the map
// @param pHandle: The map handle
// @param objects: The objects to be updated
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
TXSIMMAP_API int updateObjects(txMapHandle* pHandle, const hadmap::txObjects& objects);

// Delete objects from the map
// @param pHandle: The map handle
// @param objects: The objects to be deleted
// @return: The result of the function execution, 0 indicates success, and other values indicate errors
TXSIMMAP_API int deleteObjects(txMapHandle* pHandle, const hadmap::txObjects& objects);

// Get the reference point of the map
// @param pHandle: The map handle
// @return: The reference point of the map
TXSIMMAP_API hadmap::txPoint getRefPoint(txMapHandle* pHandle);
}  // namespace hadmap
