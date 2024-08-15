// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "db_operation.h"

#include <memory>

struct sqlite3;

namespace hadmap {
class sqliteOperation : public dbOperation {
 private:
  sqlite3* dbPtr;
  void* dbCache;
  std::string dbPath;
  bool divValid;  // division table available

 public:
  explicit sqliteOperation(const std::string& path, uint32_t srs = 4326);

  ~sqliteOperation();

 public:
  bool connect();

  bool initialize();

  bool close();

  bool isTadsimSqlite();

 public:
  // get all task info
  int getTasks(hadmap::tx_task_vec& tasks);

  // get task info by name
  int getTask(const std::string& name, hadmap::tx_task_t& task);

  // ------------------------------------------------------------- //

  // get total map data from db
  int getMap(hadmap::tx_map_t& map);

  // get map data by envelope
  // envelope [ leftbottom, righttop ]
  int getMap(const hadmap::Points2d& envelope, hadmap::tx_map_t& map);

  // ------------------------------------------------------------- //

  // get total roads data from db
  int getRoads(hadmap::tx_road_vec& roads);

  // get roads data by envelope
  // envelope [ leftbottom, righttop ]
  int getRoads(const hadmap::Points2d& envelope, hadmap::tx_road_vec& roads);

  // get road by id
  int getRoad(const roadpkid& road_pkid, hadmap::tx_road_t& road);

  // get road max id
  int getRoadMaxId(roadpkid& roadMaxId);

  // ------------------------------------------------------------- //

  // get lane data
  // get nearest lane
  int getLane(const hadmap::Point2d& point, hadmap::tx_lane_t& lane, double radius = 5);

  // get lane data by envelope
  // envelope [ leftbottom, righttop ]
  int getLanes(const hadmap::Points2d& envelope, hadmap::tx_lane_vec& lanes);

  // get lane data by id
  int getLanes(const roadpkid& road_pkid, const sectionpkid& section_id, const lanepkid& lane_id,
               hadmap::tx_lane_vec& lanes);

  // get lane data by boundary id
  int getLanes(const laneboundarypkid& boundary_pkid, hadmap::tx_lane_vec& lanes);

  // ------------------------------------------------------------- //

  // get lane boundary by id
  int getLaneBoundary(const laneboundarypkid& boundary_pkid, hadmap::tx_laneboundary_t& boundary);

  // get boundary by envelope
  // envelope [ leftbottom, righttop ]
  int getLaneBoundaries(const hadmap::Points2d& envelope, hadmap::tx_laneboundary_vec& boundaries);

  // get boundary by ids
  int getLaneBoundaries(const std::vector<laneboundarypkid>& ids, hadmap::tx_laneboundary_vec& boundaries);

  // get boundary max id
  int getLaneBoundaryMaxId(laneboundarypkid& boundaryMaxId);

  // ------------------------------------------------------------- //
  // get nearest lanelink by location
  int getLaneLink(const Point2d& point, tx_lanelink_t& link);

  // get lane link by id
  int getLaneLink(const lanelinkpkid& linkId, tx_lanelink_t& link);

  // get total lane links from db
  int getLaneLinks(hadmap::tx_lanelink_vec& lanelinks);

  // get lane links by envelope
  int getLaneLinks(const hadmap::Points2d& envelope, hadmap::tx_lanelink_vec& lanelinks);

  // get lane links by road
  int getLaneLinks(const roadpkid& from_road_pkid, const sectionpkid& from_section_id, const lanepkid& from_lane_id,
                   const roadpkid& to_road_pkid, const sectionpkid& to_section_id, const lanepkid& to_lane_id,
                   hadmap::tx_lanelink_vec& lanelinks);

  // get lane link max id
  int getLaneLinkMaxId(lanelinkpkid& linkMaxId);

  // ------------------------------------------------------------- //

  // get objects by envelope and objtype
  int getObjects(const hadmap::Points2d& envelope, const std::vector<hadmap::OBJECT_TYPE>& obj_types,
                 hadmap::tx_object_vec& objects, hadmap::tx_object_geom_vec& obj_geoms);

  int getObjects(const hadmap::Points2d& envelope, const std::vector<hadmap::OBJECT_TYPE>& obj_types,
                 hadmap::tx_object_vec& objects, hadmap::tx_object_geom_vec& obj_geoms,
                 hadmap::tx_object_rel_vec& obj_rels);

  int getOdObjects(hadmap::tx_object_vec& objects, hadmap::tx_od_object_vec& od_objects);

  // get objects by name
  int getObjects(const std::vector<std::string>& names, hadmap::tx_object_vec& objects,
                 hadmap::tx_object_geom_vec& obj_geoms);

  // get objects by relation
  int getObjects(const roadpkid& road_pkid, const sectionpkid& section_id, const lanepkid& lane_id,
                 const std::vector<hadmap::OBJECT_TYPE>& obj_types, hadmap::tx_object_vec& objects,
                 hadmap::tx_object_geom_vec& obj_geoms);

  // get objectGeomRel
  int getObjectRel(const objectpkid& objectpkid, hadmap::tx_object_geom_rel_vec& obj_rels);

 public:
  //
  // insert function
  //
  // insert tasks
  bool insertTasks(hadmap::tx_task_vec& tasks);

  // insert roads
  bool insertRoads(hadmap::tx_road_vec& roads);

  // insert lanes
  bool insertLanes(hadmap::tx_lane_vec& lanes);

  // insert lane boundaries
  bool insertLaneBoundaries(hadmap::tx_laneboundary_vec& boundaries);

  // insert lane links
  bool insertLaneLinks(hadmap::tx_lanelink_vec& lanelinks);

  // insert objects
  bool insertObjects(hadmap::tx_object_vec& objects);

  bool insertObjectGeoms(hadmap::tx_object_geom_vec& geoms);

  bool insertObjectGeomRels(hadmap::tx_object_geom_rel_vec& geom_rels);

  bool insertObjectRels(hadmap::tx_object_rel_vec& rels);

  bool insertOdObjects(hadmap::tx_od_object_vec& od_object);

 public:
  //
  // update function
  //

  // update road
  bool updateRoad(const hadmap::tx_road_t& road);

  // update lane
  bool updateLane(const hadmap::tx_lane_t& lane);

  // update lane boundary
  bool updateLaneBoundary(const hadmap::tx_laneboundary_t& boundary);

  // update lane link
  bool updateLaneLink(const hadmap::tx_lanelink_t& lanelink);

  // update objects
  bool updateOdObject(const hadmap::tx_od_object_t& object);

  //
  bool updateObjectGEOM(const hadmap::tx_object_geom_t& objectgeom);

 public:
  //
  // delete function
  //

  // delete road
  bool deleteRoad(const uint64_t& road_pkid);

  // delete lane
  bool deleteLane(const uint64_t& road_pkid, const uint32_t& section_id, const int8_t& lane_id);

  // delete lane boundary
  bool deleteLaneBoundary(const uint64_t& boundary_pkid);

  // delete lane link
  bool deleteLaneLink(const uint64_t& link_pkid);

  // delete object
  bool deleteObject(const uint64_t& object_pkid);

 private:
  // initialzie tables
  bool initializeTables();

  // get task info
  int internalGetTasks(const std::string& sql, hadmap::tx_task_vec& tasks);

  // get roads by sql
  int internalGetRoads(const std::string& sql, hadmap::tx_road_vec& roads);

  // get lanes by sql
  int internalGetLanes(const std::string& sql, hadmap::tx_lane_vec& lanes);

  // get lanelinks by sql
  int internalGetLaneLinks(const std::string& sql, hadmap::tx_lanelink_vec& lanelinks);

  // get lanelinks by sql
  int internalGetLaneOldLinks(const std::string& sql, hadmap::tx_lanelink_vec& lanelinks);

  // get lane boundaries by sql
  int internalGetLaneBoundaries(const std::string& sql, hadmap::tx_laneboundary_vec& boundaries);

  // get objects by sql
  int internalGetObjects(const std::string& sql, hadmap::tx_object_vec& objects, hadmap::tx_object_geom_vec& geoms);
};
}  // namespace hadmap
