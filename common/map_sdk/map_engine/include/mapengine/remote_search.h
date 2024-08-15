// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "mapengine/engine_defs.h"
#include "mapengine/search_interface.h"

#include "transmission/trans_interface.h"

#include <string>

namespace hadmap {
class RemoteSearch : public SearchInterface {
 public:
  RemoteSearch(MAP_DATA_TYPE type, const std::string& dbName);

  ~RemoteSearch();

 public:
  bool connSuccess();
  //
  bool saveMap(const int type, std::string filePath);

  // save current map
  bool save() { return true; }
  // get last opt info
  std::string getLastOptInfo();

  // get lane data by lonlat point
  // loc with WGS84
  // without boundary data
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

  // get next lanes
  int getNextLanes(txLanePtr curLanePtr, txLanes& nextLanes);

  // get prev lanes
  int getPrevLanes(txLanePtr curLanePtr, txLanes& prevLanes);

  // get left lane
  int getLeftLane(txLanePtr curLanePtr, txLanePtr& leftLanePtr);

  // get right lane
  int getRightLane(txLanePtr curLanePtr, txLanePtr& rightLanePtr);

  // get lane boundary
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

  int getJuncInfo(const roadpkid& entraceRoadId, const roadpkid& exitRoadId, hadmap::txRoads& roads,
                  hadmap::txLaneLinks& links);

  int getJuncInfo(const lanelinkpkid& linkId, hadmap::txRoads& roads, hadmap::txLaneLinks& links);

  int getLaneLink(const hadmap::txPoint& loc, double radius, hadmap::txLaneLinkPtr& lanelinkPtr) {
    // TBD
    return 3;  // TX_HADMAP_DATA_ERROR
  }

  // get LaneLink data by lanelinkpkid id
  // if id == PKID_INVALID, return TX_HADMAP_PARAM_ERROR
  int getLaneLink(const hadmap::lanelinkpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr) {
    // TBD
    return 3;  // TX_HADMAP_DATA_ERROR
  }

  // get prev lanelinks of current lane by txLaneId
  // if current lane doesn't have  prev lanelinks in mapdata
  // return TX_HADMAP_DATA_EMPTY and lanelinks is empty
  int getPrevLaneLinks(const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks) {
    // TBD
    return 3;  // TX_HADMAP_DATA_ERROR
  }

  // get Next lanelinks of current lane  by txLaneId
  // if current lane  doesn't have  Next lanelinks in mapdata
  // return TX_HADMAP_DATA_EMPTY and lanelinks is empty
  int getNextLaneLinks(const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks) {
    // TBD
    return 3;  // TX_HADMAP_DATA_ERROR
  }

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

  int getObjects(const PointVec& envelope, const std::vector<OBJECT_SUB_TYPE>& types, txObjects& objects);

  int getObjects(const std::vector<txLaneId>& laneIds, const std::vector<OBJECT_SUB_TYPE>& types, txObjects& objects);

  // get all jucntuions
  int getJunctions(hadmap::txJunctions& junctions);

  int getJunction(junctionpkid jid, hadmap::txJunctionPtr& junction);
  // header
  int getHeader(hadmap::txOdHeaderPtr& headerPtr);

  int insertObjects(const hadmap::txObjects& objects);

  int updateObjects(const hadmap::txObjects& objects);

  int deleteObjects(const hadmap::txObjects& objects);

  //
  int insertRoads(const hadmap::txRoads& roads);

  int updateRoads(const hadmap::txRoads& roads);

  int deleteRoads(const hadmap::txRoads& roads);

  int insertLaneLinks(const hadmap::txLaneLinks& lanelinks);

  int updateLaneLinks(const hadmap::txLaneLinks& lanelinks);

  int deleteLaneLinks(const hadmap::txLaneLinks& lanelinks);

  int getRoadLink(const hadmap::roadpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr);

  hadmap::txPoint getRefPoint();

 private:
  bool getRoads(const std::string& url, const bool& wholeData, txRoads& roads);

  int getTopoLanes(txLanePtr curLanePtr, const txPoint& tagPoint, const int& topoFlag, txLanes& lanes);

 private:
  std::string dbName;
  MAP_DATA_TYPE dataType;
  std::string optInfo;
  TransInterface* pTrans;
};
}  // namespace hadmap
