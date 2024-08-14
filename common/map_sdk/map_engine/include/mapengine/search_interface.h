// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/hadmap_header.h"
#include "structs/hadmap_lane.h"
#include "structs/hadmap_laneboundary.h"
#include "structs/hadmap_lanelink.h"
#include "structs/hadmap_road.h"

#include <memory>

namespace hadmap {
class SearchInterface;
using ISearchPtr = std::shared_ptr<SearchInterface>;

class SearchInterface {
 public:
  SearchInterface() {}

  virtual ~SearchInterface() {}

 public:
  //
  virtual bool saveMap(const int type, std::string filePath) = 0;

  // connect success flag
  virtual bool connSuccess() = 0;

  // get last opt info
  virtual std::string getLastOptInfo() = 0;

  //
  virtual bool save() = 0;

  // get lane data by lonlat point
  // loc with WGS84
  // without boundary data
  virtual int getLane(const hadmap::txPoint& loc, double radius, hadmap::txLanePtr& lanePtr) = 0;

  // get lanes data by lane id
  // road id == ROAD_PKID_INVALID -> invalid
  // section id == SECT_PKID_INVALID -> invalid
  // lane id == LANE_PKID_INVALID -> invalid
  // without boundary data
  virtual int getLanes(const hadmap::txLaneId& laneId, hadmap::txLanes& lanes) = 0;

  // get lanes data in envelope
  // without boundary data
  // envelope -> [0] leftbottom, [1] righttop
  // x : lon, y : lat
  virtual int getLanes(const hadmap::PointVec& envelope, hadmap::txLanes& lanes) = 0;

  // get next lanes
  virtual int getNextLanes(txLanePtr curLanePtr, txLanes& nextLanes) = 0;

  // get prev lanes
  virtual int getPrevLanes(txLanePtr curLanePtr, txLanes& prevLanes) = 0;

  // get left lane
  virtual int getLeftLane(txLanePtr curLanePtr, txLanePtr& leftLanePtr) = 0;

  // get right lane
  virtual int getRightLane(txLanePtr curLanePtr, txLanePtr& rightLanePtr) = 0;

  // get lane boundary
  virtual int getBoundaryMaxId(hadmap::laneboundarypkid& boundaryMaxId) = 0;

  // get lane boundaries data by id
  // if ids is empty, then return whole boundary data
  virtual int getBoundaries(const std::vector<uint64_t>& ids, hadmap::txLaneBoundaries& boundaries) = 0;

  // get section data by road id
  // if roadId == ROAD_PKID_INVALID, return TX_HADMAP_PARAM_ERROR
  // if sectionId == SECT_PKID_INVALID : get whole sections in specified road
  // else : get specified section
  virtual int getSections(const hadmap::txSectionId& sectionId, hadmap::txSections& sections) = 0;

  // get road max id
  virtual int getRoadMaxId(roadpkid& roadMaxId) = 0;

  // get road data by road id
  // if roadId == ROAD_PKID_INVALID, return TX_HADMAP_PARAM_ERROR
  // wholeData = true : get sections and lanes data belong to this road
  // wholeData = false : only road data
  virtual int getRoad(const roadpkid& roadId, const bool& wholeData, hadmap::txRoadPtr& roadPtr) = 0;

  // get roads data by envelope
  // envelope -> [0] leftbottom, [1] righttop
  // x : lon, y : lat
  // wholeData = true : get sections and lanes data belong to this road
  // wholeData = false : only road data
  virtual int getRoads(const hadmap::PointVec& envelope, const bool& wholeData, hadmap::txRoads& roads) = 0;

  // get roads by id
  // if rids is empty, then return all roads in db
  // wholeData = true : get sections and lanes data belong to this road
  // wholeData = false : only road data
  virtual int getRoads(const std::vector<roadpkid>& rids, const bool& wholeData, hadmap::txRoads& roads) = 0;

  // get all roads in db
  // wholeData = true : get sections and lanes data belong to this road
  // wholeData = false : only road data
  virtual int getRoads(const bool& wholeData, hadmap::txRoads& roads) = 0;

  // get roads around specified junction
  // entranceRoadId : road enter into junction
  // exitRoadId : road exit from junction
  // entranceRoadId != ROAD_PKID_INVALID && exitRoadId == ROAD_PKID_INVALID : junction is specified by entranceRoadId
  // entranceRoadId == ROAD_PKID_INVALID && exitRoadId != ROAD_PKID_INVALID : junction is specified by exitRoadId
  // entranceRoadId != ROAD_PKID_INVALID && exitRoadId != ROAD_PKID_INVALID : junction is specified by both
  //                                      if
  // roads are not around the same junction, then return TX_HADMAP_PARAM_ERROR
  // entranceRoadId == ROAD_PKID_INVALID && exitRoadId == ROAD_PKID_INVALID : return TX_HADMAP_PARAM_ERROR
  virtual int getJuncInfo(const roadpkid& entraceRoadId, const roadpkid& exitRoadId, hadmap::txRoads& roads,
                          hadmap::txLaneLinks& links) = 0;

  virtual int getJuncInfo(const lanelinkpkid& linkId, hadmap::txRoads& roads, hadmap::txLaneLinks& links) = 0;

  // get lanelink (having geometry) data by lonlat point
  // Search radius： 3.75*3 m
  // loc with WGS84

  int virtual getLaneLink(const hadmap::txPoint& loc, double radius, hadmap::txLaneLinkPtr& lanelinkPtr) = 0;

  // get LaneLink data by lanelinkpkid id
  // if id == PKID_INVALID, return TX_HADMAP_PARAM_ERROR
  int virtual getLaneLink(const hadmap::lanelinkpkid& lanelinkId, hadmap::txLaneLinkPtr& lanelinkPtr) = 0;

  // get prev lanelinks of current lane by txLaneId
  // if current lane doesn't have  prev lanelinks in mapdata
  // return TX_HADMAP_DATA_EMPTY and lanelinks is empty
  int virtual getPrevLaneLinks(const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks) = 0;

  int virtual getRoadLink(const hadmap::roadpkid& roadid, hadmap::txLaneLinkPtr& lanelinkPtr) = 0;

  // get Next lanelinks of current lane  by txLaneId
  // if current lane  doesn't have  Next lanelinks in mapdata
  // return TX_HADMAP_DATA_EMPTY and lanelinks is empty
  int virtual getNextLaneLinks(const hadmap::txLaneId& laneId, hadmap::txLaneLinks& lanelinks) = 0;

  // get lanelinks by road id
  // fromRoadId != ROAD_PKID_INVALID && toRoadId != ROAD_PKID_INVALID : get lanelinks between fromRoad and toRoad
  // fromRoadId != ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID : get all lanelinks from fromRoad
  // fromRoadId == ROAD_PKID_INVALID && toRoadId != ROAD_PKID_INVALID : get all lanelinks to toRoad
  // fromRoadId == ROAD_PKID_INVALID && toRoadId == ROAD_PKID_INVALID : get all lanelinks
  virtual int getLaneLinks(const roadpkid& fromRoadId, const roadpkid& toRoadId, hadmap::txLaneLinks& lanelinks) = 0;

  // get lanelinks by envelope
  // envelope -> [0] leftbottom, [1] righttop
  // x : lon, y : lat
  virtual int getLaneLinks(const hadmap::PointVec& envelope, hadmap::txLaneLinks& lanelinks) = 0;

  // get lanelinks by roads
  virtual int getLaneLinks(const std::vector<roadpkid>& roadIds, hadmap::txLaneLinks& lanelinks) = 0;

  // get lanelink max id
  virtual int getLaneLinkMaxId(lanelinkpkid& linkMaxId) = 0;

  // get objects data in envelope
  // envelope -> [0] leftbottom, [1] righttop
  // x : lon, y : lat
  // types is empty -> get whole objects with any types
  virtual int getObjects(const hadmap::PointVec& envelope, const std::vector<hadmap::OBJECT_TYPE>& types,
                         hadmap::txObjects& objects) = 0;

  // get objects data which is related to specified lanes
  // road id == ROAD_PKID_INVALID -> invalid
  // section id == SECT_PKID_INVALID -> invalid
  // lane id == LANE_PKID_INVALID -> invalid
  // laneIs is empty -> get whole objects with specified types
  // types is empty -> get whole objects with specified lanes
  virtual int getObjects(const std::vector<hadmap::txLaneId>& laneIds, const std::vector<hadmap::OBJECT_TYPE>& types,
                         hadmap::txObjects& objects) = 0;
  // get objects data in envelope
  // envelope -> [0] leftbottom, [1] righttop  x : lon, y : lat
  // subtypes is empty -> get whole objects with any types
  virtual int getObjects(const hadmap::PointVec& envelope, const std::vector<hadmap::OBJECT_SUB_TYPE>& subtypes,
                         hadmap::txObjects& objects) = 0;
  // 2）通过车道查询
  //  get objects data which is related to specified lanes
  //  laneIs is empty -> get whole objects with specified types
  //  subtypes is empty -> get whole objects with specified lanes
  virtual int getObjects(const std::vector<hadmap::txLaneId>& laneIds,
                         const std::vector<hadmap::OBJECT_SUB_TYPE>& subtypes, hadmap::txObjects& objects) = 0;
  // get all junctions
  virtual int getJunctions(hadmap::txJunctions& junctions) = 0;

  virtual int getJunction(junctionpkid jid, hadmap::txJunctionPtr& junction) = 0;
  // get header
  virtual int getHeader(hadmap::txOdHeaderPtr& headerPtr) = 0;

  virtual int insertRoads(const hadmap::txRoads& roads) = 0;

  virtual int updateRoads(const hadmap::txRoads& roads) = 0;

  virtual int deleteRoads(const hadmap::txRoads& roads) = 0;

  virtual int insertLaneLinks(const hadmap::txLaneLinks& lanelinks) = 0;

  virtual int updateLaneLinks(const hadmap::txLaneLinks& lanelinks) = 0;

  virtual int deleteLaneLinks(const hadmap::txLaneLinks& lanelinks) = 0;

  virtual int insertObjects(const hadmap::txObjects& objects) = 0;

  virtual int updateObjects(const hadmap::txObjects& objects) = 0;

  virtual int deleteObjects(const hadmap::txObjects& objects) = 0;

  virtual hadmap::txPoint getRefPoint() = 0;

  static constexpr const char* kTadsimXodr = "TadsimXodr";
};
}  // namespace hadmap
