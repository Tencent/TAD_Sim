// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "common/macros.h"
#include "structs/map_structs.h"
#include "types/map_defs.h"
#include "types/map_types.h"

#include <stdint.h>
#include <string>
#include <vector>

namespace hadmap {
class TXSIMMAP_API dbOperation {
 protected:
  uint32_t srs;

 public:
  explicit dbOperation(uint32_t s) : srs(s) {}

  virtual ~dbOperation() {}

 public:
  // database connect and initialize

  // connect database
  virtual bool connect() = 0;

  // initialize
  virtual bool initialize() = 0;

  // check is tadsim sqlite
  virtual bool isTadsimSqlite() = 0;

 public:
  //
  //  select function
  //

  // get all task info
  virtual int getTasks(hadmap::tx_task_vec& tasks) = 0;

  // get task info by name
  virtual int getTask(const std::string& name, hadmap::tx_task_t& task) = 0;

  // ------------------------------------------------------------- //

  // get total map data from db
  virtual int getMap(hadmap::tx_map_t& map) = 0;

  // get map data by envelope
  // envelope [ leftbottom, righttop ]
  virtual int getMap(const hadmap::Points2d& envelope, hadmap::tx_map_t& map) = 0;

  // ------------------------------------------------------------- //

  // get total roads data from db
  virtual int getRoads(hadmap::tx_road_vec& roads) = 0;

  // get roads data by envelope
  // envelope [ leftbottom, righttop ]
  virtual int getRoads(const hadmap::Points2d& envelope, hadmap::tx_road_vec& roads) = 0;

  // get road data by id
  virtual int getRoad(const roadpkid& road_pkid, hadmap::tx_road_t& road) = 0;

  // get road max id
  virtual int getRoadMaxId(roadpkid& roadMaxId) = 0;

  // ------------------------------------------------------------- //

  // get lane data
  // get nearest lane
  virtual int getLane(const hadmap::Point2d& point, hadmap::tx_lane_t& lane, double radius = 5) = 0;

  // get lane data by envelope
  // envelope [ leftbottom, righttop ]
  virtual int getLanes(const hadmap::Points2d& envelope, hadmap::tx_lane_vec& lanes) = 0;

  // get lane data by id
  virtual int getLanes(const roadpkid& road_pkid, const sectionpkid& section_id, const lanepkid& lane_id,
                       hadmap::tx_lane_vec& lanes) = 0;

  // get lane data by boundary id
  virtual int getLanes(const laneboundarypkid& boundary_pkid, hadmap::tx_lane_vec& lanes) = 0;

  // ------------------------------------------------------------- //

  // get lane boundary by id
  virtual int getLaneBoundary(const laneboundarypkid& boundary_pkid, hadmap::tx_laneboundary_t& boundary) = 0;

  // get boundary by envelope
  // envelope [ leftbottom, righttop ]
  virtual int getLaneBoundaries(const hadmap::Points2d& envelope, hadmap::tx_laneboundary_vec& boundaries) = 0;

  // get boundary by ids
  // if ids is empty, then get whole boundaries
  virtual int getLaneBoundaries(const std::vector<laneboundarypkid>& ids, hadmap::tx_laneboundary_vec& boundaries) = 0;

  // get boundary max id
  virtual int getLaneBoundaryMaxId(laneboundarypkid& boundaryMaxId) = 0;

  // ------------------------------------------------------------- //
  // get nearest lanelink by locations
  virtual int getLaneLink(const Point2d& point, tx_lanelink_t& link) = 0;

  // get lane link from db by id
  virtual int getLaneLink(const lanelinkpkid& linkId, hadmap::tx_lanelink_t& link) = 0;

  // get total lane links from db
  virtual int getLaneLinks(hadmap::tx_lanelink_vec& lanelinks) = 0;

  // get lane links by envelope
  virtual int getLaneLinks(const hadmap::Points2d& envelope, hadmap::tx_lanelink_vec& lanelinks) = 0;

  // get lane links by road
  // road_pkid < 0 : road_pkid invalid
  // section_id < 0 : section_id invalid
  // lane_id = 0 : lane_id invalid
  virtual int getLaneLinks(const roadpkid& from_road_pkid, const sectionpkid& from_section_id,
                           const lanepkid& from_lane_id, const roadpkid& to_road_pkid, const sectionpkid& to_section_id,
                           const lanepkid& to_lane_id, hadmap::tx_lanelink_vec& lanelinks) = 0;

  // get max lane link id
  virtual int getLaneLinkMaxId(lanelinkpkid& linkMaxId) = 0;

  // ------------------------------------------------------------- //

  // get objects by envelope and objtype
  virtual int getObjects(const hadmap::Points2d& envelope, const std::vector<hadmap::OBJECT_TYPE>& obj_types,
                         hadmap::tx_object_vec& objects, hadmap::tx_object_geom_vec& obj_geoms) = 0;

  virtual int getObjects(const hadmap::Points2d& envelope, const std::vector<hadmap::OBJECT_TYPE>& obj_types,
                         hadmap::tx_object_vec& objects, hadmap::tx_object_geom_vec& obj_geoms,
                         hadmap::tx_object_rel_vec& obj_rels) = 0;

  // get objects by name
  virtual int getObjects(const std::vector<std::string>& names, hadmap::tx_object_vec& objects,
                         hadmap::tx_object_geom_vec& obj_geoms) = 0;

  // get objects by relation
  virtual int getObjects(const roadpkid& road_pkid, const sectionpkid& section_id, const lanepkid& lane_id,
                         const std::vector<hadmap::OBJECT_TYPE>& obj_types, hadmap::tx_object_vec& objects,
                         hadmap::tx_object_geom_vec& obj_geoms) = 0;

  virtual int getOdObjects(hadmap::tx_object_vec& objects, hadmap::tx_od_object_vec& od_objects) = 0;

  virtual int getObjectRel(const objectpkid& objectpkid, hadmap::tx_object_geom_rel_vec& obj_rels) = 0;

 public:
  //
  // insert function
  //

  // insert tasks
  virtual bool insertTasks(hadmap::tx_task_vec& tasks) = 0;

  // insert roads
  virtual bool insertRoads(hadmap::tx_road_vec& roads) = 0;

  // insert lanes
  virtual bool insertLanes(hadmap::tx_lane_vec& lanes) = 0;

  // insert lane boundaries
  virtual bool insertLaneBoundaries(hadmap::tx_laneboundary_vec& boundaries) = 0;

  // insert lane links
  virtual bool insertLaneLinks(hadmap::tx_lanelink_vec& lanelinks) = 0;

  // insert objects
  // get object pkid after insert data
  virtual bool insertObjects(hadmap::tx_object_vec& objects) = 0;

  // get object geom pkid after insert data
  virtual bool insertObjectGeoms(hadmap::tx_object_geom_vec& geoms) = 0;

  //
  virtual bool insertObjectGeomRels(hadmap::tx_object_geom_rel_vec& geom_rels) = 0;

  //
  virtual bool insertObjectRels(hadmap::tx_object_rel_vec& rels) = 0;

  //
  virtual bool insertOdObjects(hadmap::tx_od_object_vec& od_object) = 0;

 public:
  //
  // update function
  //

  // update road
  virtual bool updateRoad(const hadmap::tx_road_t& road) = 0;

  // update lane
  virtual bool updateLane(const hadmap::tx_lane_t& lane) = 0;

  // update lane boundary
  virtual bool updateLaneBoundary(const hadmap::tx_laneboundary_t& boundary) = 0;

  // update lane link
  virtual bool updateLaneLink(const hadmap::tx_lanelink_t& lanelink) = 0;

  //
  virtual bool updateOdObject(const hadmap::tx_od_object_t& object) = 0;

  virtual bool updateObjectGEOM(const hadmap::tx_object_geom_t& objectgeom) = 0;

 public:
  //
  // delete function
  //

  // delete road
  virtual bool deleteRoad(const uint64_t& road_pkid) = 0;

  // delete lane
  virtual bool deleteLane(const uint64_t& road_pkid, const uint32_t& section_id, const int8_t& lane_id) = 0;

  // delete lane boundary
  virtual bool deleteLaneBoundary(const uint64_t& boundary_pkid) = 0;

  // delete lane link
  virtual bool deleteLaneLink(const uint64_t& link_pkid) = 0;

  //
  virtual bool deleteObject(const uint64_t& object_pkid) = 0;
};
}  // namespace hadmap
