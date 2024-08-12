// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "sqlite_operation.h"
#include <float.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_set>

#include "common/map_block.h"
#include "common/map_util.h"
#include "db_codes.h"
#include "sqlite3.h"
#include "sqlite_sql.h"
#include "sqlite_transaction.h"

#include "spatialite.h"
#include "spatialite/gaiageo.h"

#define USE_RTREE

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wformat"
#endif

namespace hadmap {
static const double g_lane_width = 3.75;
static const double g_meter_to_deg = 1.0 / 111000.0;
static const double g_box_buffer = g_lane_width * g_meter_to_deg * 3;
static const uint16_t id_max_len = 22;
#define kIdMaxLen (id_max_len + 1)
static bool isFileExists(const std::string& filePath) {
  std::ifstream fin(filePath.c_str());
  if (!fin) {
    std::cerr << filePath << " not exists" << std::endl;
    return false;
  }
  return true;
}

struct sqlite3_blob_data {
  unsigned char* data;
  int size;
  sqlite3_blob_data() : data(NULL), size(0) {}
  ~sqlite3_blob_data() {}

  // free manually
  // no need to free data from sqlite3_column_blob
  void release() {
    if (data != NULL) {
      gaiaFree(data);
      data = NULL;
      size = 0;
    }
  }
};

static bool createBlob(const hadmap::Points3d& geom, const int srs, sqlite3_blob_data& blob) {
  if (geom.empty()) return false;
  gaiaGeomCollPtr coll = gaiaAllocGeomColl();
  coll->Srid = srs;

  gaiaLinestringPtr pline = 0;
  coll->DimensionModel = GAIA_XY_Z;
  pline = gaiaAllocLinestringXYZ(static_cast<int>(geom.size()));

  for (size_t i = 0; i < geom.size(); ++i) gaiaSetPointXYZ(pline->Coords, i, geom[i].x, geom[i].y, geom[i].z);

  gaiaInsertLinestringInGeomColl(coll, pline);
  gaiaToSpatiaLiteBlobWkb(coll, &blob.data, &blob.size);
  gaiaFreeGeomColl(coll);

  return true;
}

static bool parseBlob(const sqlite3_blob_data& blob, hadmap::Points3d& geom) {
  geom.clear();
  if (blob.size == 0) return true;
  gaiaGeomCollPtr coll = gaiaFromSpatiaLiteBlobWkb(blob.data, blob.size);
  gaiaLinestringPtr pline = coll->FirstLinestring;

  geom.resize(pline->Points);
  for (int i = 0; i < pline->Points; i++) gaiaGetPointXYZ(pline->Coords, i, &geom[i].x, &geom[i].y, &geom[i].z);
  gaiaFreeGeomColl(coll);
  return true;
}

sqliteOperation::sqliteOperation(const std::string& path, uint32_t srs)
    : dbOperation(srs), dbPtr(NULL), dbCache(NULL), dbPath(path), divValid(false) {}

sqliteOperation::~sqliteOperation() { close(); }

bool sqliteOperation::connect() {
  if (dbPtr != NULL) {
    std::cerr << "Connection exists" << std::endl;
    return false;
  }

  int flags = SQLITE_OPEN_READWRITE;
  if (!isFileExists(dbPath)) flags |= SQLITE_OPEN_CREATE;

  if (SQLITE_OK != sqlite3_open_v2(dbPath.c_str(), &dbPtr, flags, 0)) return false;

  dbCache = spatialite_alloc_connection();
  spatialite_init_ex(dbPtr, dbCache, 0);

  sqlitePackage::execSql(dbPtr, "PRAGMA foreign_keys = ON;");
  return true;
}

bool sqliteOperation::initialize() {
  if (NULL == dbPtr) {
    std::cerr << "Create connection first" << std::endl;
    return false;
  }

  // create spatial metadata
  if (!sqlitePackage::initSpatialMeta(dbPtr)) return false;

  // create temp table
  // attention mutil process situation
  /*
  if ( !sqlitePackage::isTableExists( dbPtr, TEMP_TABLE ) )
  {
          if ( !sqlitePackage::execSql( dbPtr, SQL_CREATE_TEMP_TABLE ) )
          {
                  if ( !sqlitePackage::isTableExists( dbPtr, TEMP_TABLE ) ){
                          return false;}
          }
  }
  */

  bool ret = initializeTables();
  return ret;
}

bool sqliteOperation::initializeTables() {
  // check division table
  if (sqlitePackage::isTableExists(dbPtr, LANE_GEOM_DIV_TABLE) &&
      sqlitePackage::isTableExists(dbPtr, OBJECT_GEOM_DIV_TABLE)) {
    divValid = true;
  } else {
    divValid = false;
    // std::cout << "Division table not exists" << std::endl;
  }
  // check task table
  if (!sqlitePackage::isTableExists(dbPtr, TASK_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_TASK_TABLE)) return false;
  }

  if (!sqlitePackage::isTableExists(dbPtr, SVID_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_SVID_TABLE)) return false;
  }

  if (!sqlitePackage::isTableExists(dbPtr, JUNCTION_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_JUNCTION_TABLE) ||
        !sqlitePackage::addGeomColumn(dbPtr, JUNCTION_TABLE, "geom", srs, "POLYGON", "XY"))
      return false;
  }

  if (!sqlitePackage::isTableExists(dbPtr, ROAD_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_ROAD_TABLE) ||
        !sqlitePackage::addGeomColumn(dbPtr, ROAD_TABLE, "geom", srs, "LINESTRINGZ", "XYZ"))
      return false;
  }

  if (!sqlitePackage::isTableExists(dbPtr, LANE_BOUNDARY_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_LANE_BOUNDARY_TABLE) ||
        !sqlitePackage::addGeomColumn(dbPtr, LANE_BOUNDARY_TABLE, "geom", srs, "LINESTRINGZ", "XYZ"))
      return false;
  }

  if (!sqlitePackage::isTableExists(dbPtr, LANE_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_LANE_TABLE) ||
        !sqlitePackage::addGeomColumn(dbPtr, LANE_TABLE, "geom", srs, "LINESTRINGZ", "XYZ"))
      return false;
  }

  if (!sqlitePackage::isTableExists(dbPtr, LANE_LINK_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_LANE_LINK_TABLE) ||
        !sqlitePackage::addGeomColumn(dbPtr, LANE_LINK_TABLE, "geom", srs, "LINESTRINGZ", "XYZ"))
      return false;
  }

  // create object tables
  if (!sqlitePackage::isTableExists(dbPtr, OBJECT_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_OBJECT_TABLE(PK))) return false;
  }

  if (!sqlitePackage::isTableExists(dbPtr, OBJECT_GEOM_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_OBJECT_GEOM_TABLE) ||
        !sqlitePackage::addGeomColumn(dbPtr, OBJECT_GEOM_TABLE, "geom", srs, "LINESTRINGZ", "XYZ"))
      return false;
  }

  if (!sqlitePackage::isTableExists(dbPtr, OBJECT_GEOM_REL_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_OBJECT_GEOM_REL_TABLE(PK))) return false;
  }

  if (!sqlitePackage::isTableExists(dbPtr, OBJECT_REL_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_OBJECT_REL_TABLE(PK))) return false;
  }
  if (!sqlitePackage::isTableExists(dbPtr, OBJECT_OD_TABLE)) {
    if (!sqlitePackage::execSql(dbPtr, SQL_CREATE_OBJECT_OD_TABLE)) return false;
  }
  return true;
}

bool sqliteOperation::close() {
  if (NULL != dbPtr) {
    if (sqlitePackage::isTableExists(dbPtr, TEMP_TABLE)) sqlitePackage::dropTable(dbPtr, TEMP_TABLE);
    sqlite3_close(dbPtr);
    dbPtr = NULL;
  }

  if (NULL != dbCache) {
    spatialite_cleanup_ex(dbCache);
    dbCache = NULL;
  }

  return true;
}

bool sqliteOperation::isTadsimSqlite() {
  hadmap::tx_task_t task;
  int nRet = getTask("TadsimXodr", task);
  char sql[512];
  tx_lanelink_vec links;
  if (nRet != HADMAP_DATA_OK) {
    return false;
  }
  return true;
}

// pkid, task name, srs, x, y, z, heading, pitch, roll
int sqliteOperation::internalGetTasks(const std::string& sql, hadmap::tx_task_vec& tasks) {
  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get tasks" << std::endl;
    return HADMAP_DATA_ERROR;
  }

  tasks.clear();
  int code = HADMAP_DATA_EMPTY;
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    return HADMAP_DATA_OK;
    hadmap::tx_task_t task;
    task.pkid = sqlite3_column_int64(stmt, 0);
    strcpy(task.name, (const char*)sqlite3_column_text(stmt, 1));
    task.srs = sqlite3_column_int(stmt, 2);
    task.x = sqlite3_column_double(stmt, 3);
    task.y = sqlite3_column_double(stmt, 4);
    task.z = sqlite3_column_double(stmt, 5);
    task.heading = sqlite3_column_double(stmt, 6);
    task.pitch = sqlite3_column_double(stmt, 7);
    task.roll = sqlite3_column_double(stmt, 8);
    tasks.push_back(task);
    code = HADMAP_DATA_OK;
  }
  return code;
}

// pkid, task_pkid, id, name, srs, junction_pkid, ground, section_num, road_type,
// direction, toll, func_class, urban, paved, transition_type, lane_mark, material,
// curvature, slope, heading, super_elevation, geom
int sqliteOperation::internalGetRoads(const std::string& sql, hadmap::tx_road_vec& roads) {
  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get road" << std::endl;
    return HADMAP_DATA_ERROR;
  }

  int code = HADMAP_DATA_EMPTY;
  sqlite3_blob_data blob;
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    hadmap::tx_road_t road;
    road.pkid = sqlite3_column_int64(stmt, 0);
    road.task_pkid = sqlite3_column_int64(stmt, 1);
    strcpy(road.id, (const char*)sqlite3_column_text(stmt, 2));
    strcpy(road.name, (const char*)sqlite3_column_text(stmt, 3));
    road.srs = sqlite3_column_int(stmt, 4);
    road.junction_pkid = sqlite3_column_int64(stmt, 5);
    road.ground = sqlite3_column_double(stmt, 6);
    road.section_num = sqlite3_column_int(stmt, 7);
    road.road_type = sqlite3_column_int(stmt, 8);
    road.direction = sqlite3_column_int(stmt, 9);
    road.toll = sqlite3_column_int(stmt, 10);
    road.func_class = sqlite3_column_int(stmt, 11);
    road.urban = static_cast<bool>(sqlite3_column_int(stmt, 12));
    road.paved = static_cast<bool>(sqlite3_column_int(stmt, 13));
    road.transition_type = sqlite3_column_int(stmt, 14);
    road.lane_mark = static_cast<bool>(sqlite3_column_int(stmt, 15));
    road.material = sqlite3_column_int(stmt, 16);

    blob.data = (unsigned char*)sqlite3_column_blob(stmt, 17);
    blob.size = sqlite3_column_bytes(stmt, 17);
    parseBlob(blob, road.curvature);

    blob.data = (unsigned char*)sqlite3_column_blob(stmt, 18);
    blob.size = sqlite3_column_bytes(stmt, 18);
    parseBlob(blob, road.slope);

    blob.data = (unsigned char*)sqlite3_column_blob(stmt, 19);
    blob.size = sqlite3_column_bytes(stmt, 19);
    parseBlob(blob, road.heading);

    blob.data = (unsigned char*)sqlite3_column_blob(stmt, 20);
    blob.size = sqlite3_column_bytes(stmt, 20);
    parseBlob(blob, road.super_elevation);

    blob.data = (unsigned char*)sqlite3_column_blob(stmt, 21);
    blob.size = sqlite3_column_bytes(stmt, 21);
    parseBlob(blob, road.geom);

    roads.push_back(road);
    code = HADMAP_DATA_OK;
  }

  return code;
}

// road_pkid, section_id, lane_id, name, srs, left_boundary, right_boundary, lane_type,
// lane_arrow, speed_limit, geom
int sqliteOperation::internalGetLanes(const std::string& sql, hadmap::tx_lane_vec& lanes) {
  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get lanes" << std::endl;
    return HADMAP_DATA_ERROR;
  }

  int code = HADMAP_DATA_EMPTY;
  sqlite3_blob_data blob;
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    hadmap::tx_lane_t lane;
    lane.road_pkid = sqlite3_column_int64(stmt, 0);
    lane.section_id = sqlite3_column_int(stmt, 1);
    lane.lane_id = sqlite3_column_int(stmt, 2);
    strcpy(lane.name, (const char*)sqlite3_column_text(stmt, 3));
    lane.srs = sqlite3_column_int(stmt, 4);
    lane.left_boundary = sqlite3_column_int64(stmt, 5);
    lane.right_boundary = sqlite3_column_int64(stmt, 6);
    lane.lane_type = sqlite3_column_int(stmt, 7);
    lane.lane_arrow = sqlite3_column_int(stmt, 8);
    lane.speed_limit = sqlite3_column_int(stmt, 9);

    blob.data = (unsigned char*)sqlite3_column_blob(stmt, 10);
    blob.size = sqlite3_column_bytes(stmt, 10);
    parseBlob(blob, lane.geom);

    lanes.push_back(lane);
    code = HADMAP_DATA_OK;
  }

  // sort lane data
  map_util::sortLaneData(lanes);

  return code;
}

// pkid, srs, lane_mark, geom
int sqliteOperation::internalGetLaneBoundaries(const std::string& sql, hadmap::tx_laneboundary_vec& boundaries) {
  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get lane boundaries" << std::endl;
    return HADMAP_DATA_ERROR;
  }

  int code = HADMAP_DATA_EMPTY;
  sqlite3_blob_data blob;
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    hadmap::tx_laneboundary_t boundary;
    boundary.pkid = sqlite3_column_int64(stmt, 0);
    boundary.srs = sqlite3_column_int(stmt, 1);
    boundary.lane_mark = sqlite3_column_int(stmt, 2);

    blob.data = reinterpret_cast<uint8_t*>(const_cast<void*>(sqlite3_column_blob(stmt, 3)));
    blob.size = sqlite3_column_bytes(stmt, 3);
    parseBlob(blob, boundary.geom);

    boundaries.push_back(boundary);
    code = HADMAP_DATA_OK;
  }

  return code;
}

// pkid, from_road_pkid, from_section_id, from_lane_id, to_road_pkid, to_section_id, to_lane_id, geom
int sqliteOperation::internalGetLaneLinks(const std::string& sql, hadmap::tx_lanelink_vec& lanelinks) {
  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get lane links" << std::endl;
    return HADMAP_DATA_ERROR;
  }

  int code = HADMAP_DATA_EMPTY;
  sqlite3_blob_data blob;
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    hadmap::tx_lanelink_t lanelink;
    lanelink.pkid = sqlite3_column_int64(stmt, 0);
    lanelink.from_road_pkid = sqlite3_column_int64(stmt, 1);
    lanelink.from_section_id = (uint32_t)sqlite3_column_int(stmt, 2);
    lanelink.from_lane_id = sqlite3_column_int(stmt, 3);
    int preData = sqlite3_column_int(stmt, 4);
    lanelink.preContact = (txContactType)preData;
    lanelink.to_road_pkid = sqlite3_column_int64(stmt, 5);
    lanelink.to_section_id = (uint32_t)sqlite3_column_int64(stmt, 6);
    lanelink.to_lane_id = sqlite3_column_int(stmt, 7);
    int succData = sqlite3_column_int(stmt, 8);
    lanelink.succContact = (txContactType)succData;
    lanelink.junction_id = sqlite3_column_int(stmt, 9);
    // cut geom need to be removed
    blob.data = (unsigned char*)sqlite3_column_blob(stmt, 10);
    blob.size = sqlite3_column_bytes(stmt, 10);
    parseBlob(blob, lanelink.geom);
    lanelinks.push_back(lanelink);
    code = HADMAP_DATA_OK;
  }

  return code;
}

int sqliteOperation::internalGetLaneOldLinks(const std::string& sql, hadmap::tx_lanelink_vec& lanelinks) {
  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get lane links" << std::endl;
    return HADMAP_DATA_ERROR;
  }

  int code = HADMAP_DATA_EMPTY;
  sqlite3_blob_data blob;
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    hadmap::tx_lanelink_t lanelink;
    lanelink.pkid = sqlite3_column_int64(stmt, 0);
    lanelink.from_road_pkid = sqlite3_column_int64(stmt, 1);
    lanelink.from_section_id = (uint32_t)sqlite3_column_int(stmt, 2);
    lanelink.from_lane_id = sqlite3_column_int(stmt, 3);
    lanelink.to_road_pkid = sqlite3_column_int64(stmt, 4);
    lanelink.to_section_id = (uint32_t)sqlite3_column_int64(stmt, 5);
    lanelink.to_lane_id = sqlite3_column_int(stmt, 6);
    // cut geom need to be removed
    blob.data = (unsigned char*)sqlite3_column_blob(stmt, 7);
    blob.size = sqlite3_column_bytes(stmt, 7);
    lanelink.junction_id = sqlite3_column_int64(stmt, 8);
    parseBlob(blob, lanelink.geom);
    lanelinks.push_back(lanelink);
    code = HADMAP_DATA_OK;
  }

  return code;
}

// object pkid, object type, name, attribute
// geom pkid, geom type, srs, dimension, object style, color, text, size( w, h ), geom( x, y, z )
int sqliteOperation::internalGetObjects(const std::string& sql, hadmap::tx_object_vec& objects,
                                        hadmap::tx_object_geom_vec& geoms) {
  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get objects" << std::endl;
    return HADMAP_DATA_ERROR;
  }

  int code = HADMAP_DATA_EMPTY;
  sqlite3_blob_data sizeBlob, geomBlob;
  std::vector<double> sizes;
  hadmap::Points3d linestring;
  std::unordered_set<objectpkid> obj_ids;
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    hadmap::tx_object_t object;
    hadmap::tx_object_geom_t geom;

    // get object data
    object.pkid = (uint64_t)sqlite3_column_int64(stmt, 0);
    object.type = (uint32_t)sqlite3_column_int(stmt, 1);
    strcpy(object.name, (const char*)sqlite3_column_text(stmt, 2));
    object.attrLen = (size_t)sqlite3_column_bytes(stmt, 3);
    memcpy(object.attribute, (const char*)sqlite3_column_blob(stmt, 3), object.attrLen);

    // get geom data
    geom.pkid = (uint64_t)sqlite3_column_int64(stmt, 4);
    geom.type = (uint32_t)sqlite3_column_int(stmt, 5);
    geom.srs = (uint32_t)sqlite3_column_int(stmt, 6);
    geom.dimension = (int8_t)sqlite3_column_int(stmt, 7);
    geom.style = (uint32_t)sqlite3_column_int(stmt, 8);
    geom.color = (uint32_t)sqlite3_column_int(stmt, 9);
    strcpy(geom.text, (const char*)sqlite3_column_text(stmt, 10));

    sizeBlob.size = static_cast<int>(sqlite3_column_bytes(stmt, 11));
    sizeBlob.data = (unsigned char*)sqlite3_column_blob(stmt, 11);
    size_t num = sizeBlob.size / (sizeof(double) * 2);
    if (sizeBlob.size % (sizeof(double) * 2) != 0) {
      std::cout << "Object size data error" << std::endl;
      continue;
    }
    sizes.resize(sizeBlob.size / sizeof(double));
    if (sizes.size() > 0) memcpy(&sizes[0], sizeBlob.data, sizeBlob.size);

    geomBlob.size = static_cast<int>(sqlite3_column_bytes(stmt, 12));
    geomBlob.data = (unsigned char*)sqlite3_column_blob(stmt, 12);
    parseBlob(geomBlob, linestring);

    geom.geom.resize(std::min(num, linestring.size()));
    for (size_t k = 0; k < geom.geom.size(); ++k) {
      geom.geom[k].x = linestring[k].x;
      geom.geom[k].y = linestring[k].y;
      geom.geom[k].z = linestring[k].z;
      geom.geom[k].w = sizes[k * 2];
      geom.geom[k].h = sizes[k * 2 + 1];
    }
    if (obj_ids.find(object.pkid) == obj_ids.end()) {
      obj_ids.insert(object.pkid);
      objects.push_back(object);
      code = HADMAP_DATA_OK;
    }
    geoms.push_back(geom);
  }
  return code;
}

int sqliteOperation::getTasks(hadmap::tx_task_vec& tasks) {
  char sql[512];
  sprintf(sql, "SELECT pkid, name, srs, x, y, z, heading, pitch, roll FROM %s", TASK_TABLE);
  return internalGetTasks(sql, tasks);
}

int sqliteOperation::getTask(const std::string& name, hadmap::tx_task_t& task) {
  char sql[512];
  sprintf(sql, "SELECT pkid FROM %s WHERE name='%s'", TASK_TABLE, name.c_str());
  hadmap::tx_task_vec tasks;
  return internalGetTasks(sql, tasks);
}

int sqliteOperation::getMap(hadmap::tx_map_t& map) { return HADMAP_DATA_ERROR; }

int sqliteOperation::getMap(const hadmap::Points2d& envelope, hadmap::tx_map_t& map) {
  if ((getRoads(envelope, map.roads) == HADMAP_DATA_OK) && (getLanes(envelope, map.lanes) == HADMAP_DATA_OK) &&
      (getLaneBoundaries(envelope, map.boundaries) == HADMAP_DATA_OK) &&
      (getLaneLinks(envelope, map.lanelinks) == HADMAP_DATA_OK)) {
    return HADMAP_DATA_OK;
  } else {
    return HADMAP_DATA_ERROR;
  }
}

int sqliteOperation::getRoads(hadmap::tx_road_vec& roads) {
  char sql[512];
  sprintf(sql, SQL_SELECT_ROAD);
  return internalGetRoads(sql, roads);
}

int sqliteOperation::getRoads(const hadmap::Points2d& envelope, hadmap::tx_road_vec& roads) {
  char sql[512];
  sprintf(sql, SQL_SELECT_ROAD " WHERE MBRIntersects(BuildMBR(%lf,%lf,%lf,%lf), geom)", envelope[0].x, envelope[0].y,
          envelope[1].x, envelope[1].y);
  return internalGetRoads(sql, roads);
}

int sqliteOperation::getRoad(const roadpkid& road_pkid, hadmap::tx_road_t& road) {
  char sql[512];
  sprintf(sql, SQL_SELECT_ROAD " WHERE pkid = %llu", road_pkid);
  hadmap::tx_road_vec roads;
  int code = internalGetRoads(sql, roads);
  if (HADMAP_DATA_OK == code) {
    road = roads[0];
  }
  return code;
}

int sqliteOperation::getRoadMaxId(roadpkid& roadMaxId) {
  char sql[512];
  sprintf(sql, "SELECT MAX(pkid) FROM TX_ROAD");

  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get road max id" << std::endl;
    return HADMAP_DATA_ERROR;
  }

  int code = HADMAP_DATA_EMPTY;
  sqlite3_blob_data blob;
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    roadMaxId = sqlite3_column_int64(stmt, 0);
    code = HADMAP_DATA_OK;
  }

  return code;
}

int sqliteOperation::getLane(const hadmap::Point2d& point, hadmap::tx_lane_t& lane, double radius) {
  hadmap::Points2d envelope;
  envelope.push_back(hadmap::Point2d(point.x - radius * g_meter_to_deg, point.y - radius * g_meter_to_deg));
  envelope.push_back(hadmap::Point2d(point.x + radius * g_meter_to_deg, point.y + radius * g_meter_to_deg));
  hadmap::tx_lane_vec lanes;
  int code = getLanes(envelope, lanes);
  if (code == HADMAP_DATA_OK) {
    // need to be opt
    if (lanes.size() == 1) {
      lane = lanes[0];
    } else {
      double dis = DBL_MAX;
      size_t index = 0;
      for (size_t i = 0; i < lanes.size(); ++i) {
        double z = lanes[i].geom[0].z;
        double curDis = map_util::calcPoint2PolylineDis(hadmap::Point3d(point.x, point.y, z), lanes[i].geom,
                                                        true /*,countZ is false*/);

        double disToLaneStart = map_util::distanceBetweenPoints(
            hadmap::Point3d(lanes[i].geom[0].x, lanes[i].geom[0].y, 0), hadmap::Point3d(point.x, point.y, 0), true);

        if (curDis < 0.001 /*1 mm*/ && disToLaneStart < 0.001) {
          dis = curDis;
          index = i;
          break;
        }
        if (curDis < dis) {
          dis = curDis;
          index = i;
        }
      }
      lane = lanes[index];
    }
  }
  return code;
}

int sqliteOperation::getLanes(const hadmap::Points2d& envelope, hadmap::tx_lane_vec& lanes) {
  std::set<std::string> blockIds;
  map_block::GenerateBlockIds(envelope, blockIds);
  if (blockIds.empty()) return HADMAP_PARAM_ERROR;
  std::string blockStr = map_block::GenerateBlockStr(blockIds);

  char sql[512];
#ifdef USE_RTREE
  char rowid_by_vsi[256];
  sprintf(rowid_by_vsi, ROWID_BY_VSI(LANE_TABLE), envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
  sprintf(sql,
          "SELECT road_pkid, section_id, lane_id, name, srs, left_boundary, "
          "right_boundary, lane_type, lane_arrow, speed_limit, geom "
          "FROM %s WHERE rowid in (%s) and geom is not NULL",
          LANE_TABLE, rowid_by_vsi);
#else
  if (divValid == false) {
    sprintf(sql,
            "SELECT road_pkid, section_id, lane_id, name, srs, left_boundary, "
            "right_boundary, lane_type, lane_arrow, speed_limit, geom "
            "FROM %s WHERE Intersects(BuildMBR(%lf,%lf,%lf,%lf), geom) and geom is not NULL",
            LANE_TABLE, envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
  } else {
    sprintf(sql,
            "SELECT L.road_pkid, L.section_id, L.lane_id, L.name, L.srs, L.left_boundary, "
            "L.right_boundary, L.lane_type, L.lane_arrow, L.speed_limit, L.geom FROM " LANE_TABLE
            " AS L, "
            "( SELECT road_pkid, section_id FROM " LANE_GEOM_DIV_TABLE
            " WHERE division in ( %s ) ) AS G "
            "WHERE L.road_pkid == G.road_pkid AND L.section_id == G.section_id AND "
            "Intersects(BuildMBR( %lf, %lf, %lf, %lf ), L.geom ) AND L.geom is not NULL",
            blockStr.c_str(), envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
  }
#endif
  return internalGetLanes(sql, lanes);
}

int sqliteOperation::getLanes(const roadpkid& road_pkid, const sectionpkid& section_id, const lanepkid& lane_id,
                              hadmap::tx_lane_vec& lanes) {
  char sql[512];
  sprintf(sql,
          "SELECT road_pkid, section_id, lane_id, name, srs, left_boundary, \
         right_boundary, lane_type, lane_arrow, speed_limit, geom FROM %s",
          LANE_TABLE);
  std::string baseSql(sql);
  if (road_pkid != ROAD_PKID_INVALID || section_id != SECT_PKID_INVALID || lane_id != LANE_PKID_INVALID) {
    baseSql += " WHERE ";
    memset(sql, 0, 512);
    if (road_pkid != ROAD_PKID_INVALID) {
      sprintf(sql, "road_pkid = %lld", road_pkid);
      baseSql += std::string(sql);
    }
    if (section_id != SECT_PKID_INVALID) {
      if (strlen(sql) != 0) baseSql += " AND ";
      sprintf(sql, "section_id = %d", section_id);
      baseSql += std::string(sql);
    }
    if (lane_id != LANE_PKID_INVALID) {
      if (strlen(sql) != 0) baseSql += " AND ";
      sprintf(sql, "lane_id = %d", lane_id);
      baseSql += std::string(sql);
    }
  }
  return internalGetLanes(baseSql, lanes);
}

int sqliteOperation::getLanes(const uint64_t& boundary_pkid, hadmap::tx_lane_vec& lanes) {
  char sql[512];
  sprintf(sql,
          "SELECT road_pkid, section_id, lane_id, name, srs, \
            left_boundary, right_boundary, lane_type, lane_arrow, speed_limit, geom FROM %s \
            WHERE left_boundary = %lld or right_boundary = %lld",
          LANE_TABLE, boundary_pkid, boundary_pkid);
  return internalGetLanes(sql, lanes);
}

int sqliteOperation::getLaneBoundary(const uint64_t& boundary_pkid, hadmap::tx_laneboundary_t& boundary) {
  char sql[512];
  sprintf(sql, "SELECT pkid, srs, lane_mark, geom FROM %s WHERE pkid=%llu", LANE_BOUNDARY_TABLE, boundary_pkid);
  hadmap::tx_laneboundary_vec boundaries;
  int code = internalGetLaneBoundaries(sql, boundaries);
  if (HADMAP_DATA_OK == code) {
    boundary = boundaries[0];
  }
  return code;
}

int sqliteOperation::getLaneBoundaries(const hadmap::Points2d& envelope, hadmap::tx_laneboundary_vec& boundaries) {
  char sql[512];
#ifdef USE_RTREE
  char rowid_by_vsi[256];
  sprintf(rowid_by_vsi, ROWID_BY_VSI(LANE_BOUNDARY_TABLE), envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
  sprintf(sql,
          "SELECT pkid, srs, lane_mark, geom FROM %s "
          "WHERE rowid in ( %s )",
          LANE_BOUNDARY_TABLE, rowid_by_vsi);
#else
  sprintf(sql,
          "SELECT pkid, srs, lane_mark, geom FROM %s \
            WHERE MBRIntersects(BuildMBR(%lf, %lf, %lf, %lf), geom)",
          LANE_BOUNDARY_TABLE, envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
#endif
  return internalGetLaneBoundaries(sql, boundaries);
}

int sqliteOperation::getLaneBoundaries(const std::vector<laneboundarypkid>& ids,
                                       hadmap::tx_laneboundary_vec& boundaries) {
  char sql[5000];
  sprintf(sql, "SELECT pkid, srs, lane_mark, geom FROM %s", LANE_BOUNDARY_TABLE);
  if (!ids.empty()) {
    strcat(sql, " WHERE pkid in (");
    char tmp[32];
    for (size_t i = 0; i < ids.size(); ++i) {
      if (i > 0) {
        sprintf(tmp, ",%llu", ids[i]);
      } else {
        sprintf(tmp, "%llu", ids[i]);
      }
      strcat(sql, tmp);
    }
    strcat(sql, ")");
  }
  return internalGetLaneBoundaries(sql, boundaries);
}

int sqliteOperation::getLaneBoundaryMaxId(laneboundarypkid& boundaryMaxId) {
  char sql[512];
  sprintf(sql, "SELECT MAX(pkid) FROM TX_LANE_BOUNDARY");

  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get lane boundary max id" << std::endl;
    return HADMAP_DATA_ERROR;
  }

  int code = HADMAP_DATA_EMPTY;
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    boundaryMaxId = sqlite3_column_int64(stmt, 0);
    code = HADMAP_DATA_OK;
  }

  return code;
}
/**
 * get nearest lanelink by loc
 */
int sqliteOperation::getLaneLink(const Point2d& point, tx_lanelink_t& link) {
  hadmap::Points2d envelope;
  envelope.push_back(hadmap::Point2d(point.x - g_box_buffer, point.y - g_box_buffer));
  envelope.push_back(hadmap::Point2d(point.x + g_box_buffer, point.y + g_box_buffer));
  hadmap::tx_lanelink_vec lanelinkVec;
  int code = getLaneLinks(envelope, lanelinkVec);
  if (code == HADMAP_DATA_OK) {
    if (lanelinkVec.size() == 1) {
      link = lanelinkVec[0];
    } else {
      double dis = DBL_MAX;
      size_t index = 0;
      for (size_t i = 0; i < lanelinkVec.size() && lanelinkVec[i].geom.size() > 0; ++i) {
        double z = lanelinkVec[i].geom[0].z;
        double curDis = map_util::calcPoint2PolylineDis(hadmap::Point3d(point.x, point.y, z), lanelinkVec[i].geom,
                                                        true /*,countZ is false*/);

        double disToLaneStart =
            map_util::distanceBetweenPoints(hadmap::Point3d(lanelinkVec[i].geom[0].x, lanelinkVec[i].geom[0].y, 0),
                                            hadmap::Point3d(point.x, point.y, 0), true);

        if (curDis < 0.001 /*1 mm*/ && disToLaneStart < 0.001) {
          dis = curDis;
          index = i;
          break;
        }
        if (curDis < dis) {
          dis = curDis;
          index = i;
        }
      }
      link = lanelinkVec[index];
    }
  }
  return code;
}

int sqliteOperation::getLaneLink(const lanelinkpkid& linkId, tx_lanelink_t& link) {
  hadmap::tx_task_t task;
  int nRet = getTask("TadsimXodr", task);
  char sql[512];
  tx_lanelink_vec links;
  if (nRet != HADMAP_DATA_OK) {
    sprintf(sql, SQL_SELECT_LANE_LINK_OLD " WHERE pkid=%s", std::to_string(linkId).c_str());
    int r = internalGetLaneOldLinks(sql, links);
    if (!links.empty()) link = links.front();
    return r;
  } else {
    sprintf(sql, SQL_SELECT_LANE_LINK " WHERE pkid=%s", std::to_string(linkId).c_str());
    int r = internalGetLaneLinks(sql, links);
    if (!links.empty()) link = links.front();
    return r;
  }
}

int sqliteOperation::getLaneLinks(hadmap::tx_lanelink_vec& lanelinks) {
  hadmap::tx_task_t task;
  int nRet = getTask("TadsimXodr", task);
  char sql[512];
  if (nRet != HADMAP_DATA_OK) {
    sprintf(sql, SQL_SELECT_LANE_LINK_OLD);
    int r = internalGetLaneOldLinks(sql, lanelinks);
    return r;
  } else {
    sprintf(sql, SQL_SELECT_LANE_LINK);
    int r = internalGetLaneLinks(sql, lanelinks);
    return r;
  }
}

int sqliteOperation::getLaneLinks(const hadmap::Points2d& envelope, hadmap::tx_lanelink_vec& lanelinks) {
  bool bFlagIsOld = false;
  char sql[512];
  hadmap::tx_task_t task;
  int nRet = getTask("TadsimXodr", task);
#ifdef USE_RTREE
  char rowid_by_vsi[256];
  sprintf(rowid_by_vsi, ROWID_BY_VSI(LANE_LINK_TABLE), envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
  if (nRet != HADMAP_DATA_OK) {
    sprintf(sql, SQL_SELECT_LANE_LINK_OLD " WHERE rowid in (%s)", rowid_by_vsi);
    bFlagIsOld = true;
  } else {
    sprintf(sql, SQL_SELECT_LANE_LINK " WHERE rowid in (%s)", rowid_by_vsi);
    bFlagIsOld = false;
  }
#else
  sprintf(sql, SQL_SELECT_LANE_LINK " WHERE MBRIntersects(BuildMBR(%lf,%lf,%lf,%lf), geom)", envelope[0].x,
          envelope[0].y, envelope[1].x, envelope[1].y);
#endif
  if (bFlagIsOld == true) {
    return internalGetLaneOldLinks(sql, lanelinks);
  } else {
    return internalGetLaneLinks(sql, lanelinks);
  }
}

int sqliteOperation::getLaneLinks(const roadpkid& from_road_pkid, const sectionpkid& from_section_id,
                                  const lanepkid& from_lane_id, const roadpkid& to_road_pkid,
                                  const sectionpkid& to_section_id, const lanepkid& to_lane_id,
                                  hadmap::tx_lanelink_vec& lanelinks) {
  char sql[1024];
  char csql[128];
  sprintf(sql, SQL_SELECT_LANE_LINK);
  hadmap::tx_task_t task;
  int nRet = getTask("TadsimXodr", task);
  bool bFlagIsOld = false;
  if (nRet != HADMAP_DATA_OK) {
    sprintf(sql, SQL_SELECT_LANE_LINK_OLD);
    bFlagIsOld = true;
  } else {
    sprintf(sql, SQL_SELECT_LANE_LINK);
    bFlagIsOld = false;
  }

  bool conditionFlag = false;
  if (from_road_pkid != ROAD_PKID_INVALID) {
    sprintf(csql, " WHERE from_road_pkid = %lld ", from_road_pkid);
    conditionFlag = true;
    strcat(sql, csql);
  }
  if (from_section_id != SECT_PKID_INVALID) {
    if (conditionFlag) {
      sprintf(csql, " AND from_section_id = %d ", from_section_id);
    } else {
      sprintf(csql, " WHERE from_section_id = %d ", from_section_id);
    }
    conditionFlag = true;
    strcat(sql, csql);
  }
  if (from_lane_id != LANE_PKID_INVALID) {
    if (conditionFlag) {
      sprintf(csql, " AND from_lane_id = %d ", from_lane_id);
    } else {
      sprintf(csql, " WHERE from_lane_id = %d ", from_lane_id);
    }
    conditionFlag = true;
    strcat(sql, csql);
  }
  if (to_road_pkid != ROAD_PKID_INVALID) {
    if (conditionFlag) {
      sprintf(csql, " AND to_road_pkid = %lld ", to_road_pkid);
    } else {
      sprintf(csql, " WHERE to_road_pkid = %lld ", to_road_pkid);
    }
    conditionFlag = true;
    strcat(sql, csql);
  }
  if (to_section_id != SECT_PKID_INVALID) {
    if (conditionFlag) {
      sprintf(csql, " AND to_section_id = %d ", to_section_id);
    } else {
      sprintf(csql, " WHERE to_section_id = %d ", to_section_id);
    }
    conditionFlag = true;
    strcat(sql, csql);
  }
  if (to_lane_id != LANE_PKID_INVALID) {
    if (conditionFlag) {
      sprintf(csql, " AND to_lane_id = %d ", to_lane_id);
    } else {
      sprintf(csql, " WHERE to_lane_id = %d ", to_lane_id);
    }
    conditionFlag = true;
    strcat(sql, csql);
  }
  if (bFlagIsOld == true) {
    return internalGetLaneOldLinks(sql, lanelinks);
  } else {
    return internalGetLaneLinks(sql, lanelinks);
  }
}

int sqliteOperation::getLaneLinkMaxId(lanelinkpkid& linkMaxId) {
  char sql[512];
  sprintf(sql, "SELECT MAX(pkid) FROM TX_LANE_LINK");

  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get link max id" << std::endl;
    return HADMAP_DATA_ERROR;
  }

  int code = HADMAP_DATA_EMPTY;
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    linkMaxId = sqlite3_column_int64(stmt, 0);
    code = HADMAP_DATA_OK;
  }

  return code;
}

int sqliteOperation::getObjects(const hadmap::Points2d& envelope, const std::vector<hadmap::OBJECT_TYPE>& obj_types,
                                hadmap::tx_object_vec& objects, hadmap::tx_object_geom_vec& obj_geoms,
                                hadmap::tx_object_rel_vec& obj_rels) {
  if (HADMAP_DATA_OK == getObjects(envelope, obj_types, objects, obj_geoms)) {
    if (objects.empty()) return HADMAP_DATA_ERROR;
    char* sql = new char[id_max_len * objects.size() + 128]();
    sprintf(sql,
            "SELECT pkid, object_pkid, road_pkid, section_id, lane_id FROM TX_OBJECT_REL "
            "WHERE object_pkid in ( ");
    for (size_t i = 0; i < objects.size(); ++i) {
      char id[kIdMaxLen];
      sprintf(id, "%llu,", objects[i].pkid);
      strcat(sql, id);
    }
    size_t l = strlen(sql);
    sql[l - 1] = ')';

    sqliteStmtPtr stmt(dbPtr, sql);
    if (!stmt.status()) {
      std::cerr << "Failed to get obj rels" << std::endl;
      delete[] sql;
      return HADMAP_DATA_ERROR;
    }

    int code = HADMAP_DATA_EMPTY;
    while (SQLITE_ROW == sqlite3_step(stmt)) {
      hadmap::tx_object_rel_t objRel;
      objRel.pkid = sqlite3_column_int64(stmt, 0);
      objRel.object_pkid = sqlite3_column_int64(stmt, 1);
      objRel.road_pkid = sqlite3_column_int64(stmt, 2);
      objRel.section_id = sqlite3_column_int64(stmt, 3);
      objRel.lane_id = sqlite3_column_int64(stmt, 4);

      obj_rels.push_back(objRel);
    }
    delete[] sql;
    return HADMAP_DATA_OK;
  }

  return HADMAP_DATA_ERROR;
}

int sqliteOperation::getOdObjects(hadmap::tx_object_vec& objects, hadmap::tx_od_object_vec& od_objects) {
  char* sql = new char[id_max_len * objects.size() + 256]();
  sprintf(
      sql,
      "SELECT pkid, object_pkid, type, subtype, name, s, t, validIength, orientation, radius, length, width, height, "
      "hdg, pitch, roll, zOffset, roadpkid, lanelinkpkid, userdata, repeatdata, markwidth, outlines FROM TX_OD_OBJECT "
      "WHERE object_pkid in ( ");
  for (size_t i = 0; i < objects.size(); ++i) {
    char id[kIdMaxLen];
    sprintf(id, "%llu,", objects[i].pkid);
    strcat(sql, id);
  }
  size_t l = strlen(sql);
  sql[l - 1] = ')';
  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get od objects" << std::endl;
    return HADMAP_DATA_ERROR;
  }
  int code = HADMAP_DATA_EMPTY;
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    hadmap::tx_od_object_t _od_object;
    _od_object.pkid = sqlite3_column_int64(stmt, 0);
    _od_object.object_pkid = sqlite3_column_int64(stmt, 1);
    _od_object.type = sqlite3_column_int64(stmt, 2);
    _od_object.subtype = sqlite3_column_int64(stmt, 3);
    strcpy(_od_object.name, (const char*)sqlite3_column_text(stmt, 4));
    _od_object.s = sqlite3_column_double(stmt, 5);
    _od_object.t = sqlite3_column_double(stmt, 6);
    _od_object.validlength = sqlite3_column_double(stmt, 7);
    _od_object.orientation = sqlite3_column_int64(stmt, 8);
    _od_object.radius = sqlite3_column_double(stmt, 9);
    _od_object.length = sqlite3_column_double(stmt, 10);
    _od_object.width = sqlite3_column_double(stmt, 11);
    _od_object.height = sqlite3_column_double(stmt, 12);
    _od_object.hdg = sqlite3_column_double(stmt, 13);
    _od_object.pitch = sqlite3_column_double(stmt, 14);
    _od_object.roll = sqlite3_column_double(stmt, 15);
    _od_object.zoffset = sqlite3_column_double(stmt, 16);
    _od_object.roadid = sqlite3_column_int64(stmt, 17);
    _od_object.lanelinkid = sqlite3_column_int64(stmt, 18);
    strcpy(_od_object.userdata, (const char*)sqlite3_column_text(stmt, 19));
    strcpy(_od_object.repeatdata, (const char*)sqlite3_column_text(stmt, 20));
    _od_object.markWidth = sqlite3_column_double(stmt, 21);
    strcpy(_od_object.outlines, (const char*)sqlite3_column_text(stmt, 22));
    od_objects.push_back(_od_object);
  }
  delete[] sql;
  return HADMAP_DATA_OK;
  return 0;
}

int sqliteOperation::getObjects(const hadmap::Points2d& envelope, const std::vector<hadmap::OBJECT_TYPE>& obj_types,
                                hadmap::tx_object_vec& objects, hadmap::tx_object_geom_vec& obj_geoms) {
  std::set<std::string> blockIds;
  map_block::GenerateBlockIds(envelope, blockIds);
  if (blockIds.empty()) return false;
  std::string blockStr = map_block::GenerateBlockStr(blockIds);

  char sql[4096];
#ifdef USE_RTREE
  char rowid_by_vsi[256];
  sprintf(rowid_by_vsi, ROWID_BY_VSI(OBJECT_GEOM_TABLE), envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
  sprintf(sql,
          "SELECT o.pkid, o.type, o.name, o.attribute, "
          "g.pkid, g.type, g.srs, g.dimension, g.style, g.color, g.text, g.size, g.geom "
          "FROM " OBJECT_TABLE " AS o INNER JOIN " OBJECT_GEOM_REL_TABLE
          " AS r ON o.pkid = r.object_pkid "
          "INNER JOIN ( SELECT * FROM " OBJECT_GEOM_TABLE
          " "
          "WHERE rowid in (%s) ) AS g ON r.geom_pkid = g.pkid ",
          rowid_by_vsi);
  if (!obj_types.empty()) {
    std::string objTypeCondition = "WHERE o.type in ( ";
    for (size_t i = 0; i < obj_types.size(); ++i) {
      char typeContent[32];
      sprintf(typeContent, " %d,", obj_types[i]);
      objTypeCondition += std::string(typeContent);
    }
    objTypeCondition[objTypeCondition.length() - 1] = ')';
    strcat(sql, objTypeCondition.c_str());
  }
#else
  if (divValid == false) {
    sprintf(sql,
            "SELECT o.pkid, o.type, o.name, o.attribute, "
            "g.pkid, g.type, g.srs, g.dimension, g.style, g.color, g.text, g.size, g.geom "
            "FROM " OBJECT_TABLE " AS o INNER JOIN " OBJECT_GEOM_REL_TABLE
            " AS r ON o.pkid = r.object_pkid "
            "INNER JOIN ( SELECT * FROM " OBJECT_GEOM_TABLE
            " "
            "WHERE MBRIntersects(BuildMBR(%lf,%lf,%lf,%lf), geom)) AS g ON r.geom_pkid = g.pkid ",
            envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
    if (!obj_types.empty()) {
      std::string objTypeCondition = "WHERE o.type in ( ";
      for (size_t i = 0; i < obj_types.size(); ++i) {
        char typeContent[32];
        sprintf(typeContent, " %d,", obj_types[i]);
        objTypeCondition += std::string(typeContent);
      }
      objTypeCondition[objTypeCondition.length() - 1] = ')';
      strcat(sql, objTypeCondition.c_str());
    }
  } else {
    std::string objTypeCondition = "";
    if (!obj_types.empty()) {
      objTypeCondition = " object_type in ( ";
      for (size_t i = 0; i < obj_types.size(); ++i) {
        char typeContent[32];
        sprintf(typeContent, " %d,", obj_types[i]);
        objTypeCondition += std::string(typeContent);
      }
      objTypeCondition[objTypeCondition.length() - 1] = ')';
    }
    sprintf(sql,
            "SELECT O.pkid, O.type, O.name, O.attribute, "
            "G.pkid, G.type, G.srs, G.dimension, G.style, G.color, G.text, G.size, G.geom "
            "FROM " OBJECT_TABLE " AS O, " OBJECT_GEOM_REL_TABLE
            " AS R, "
            "( SELECT * FROM TX_OBJECT_GEOM WHERE pkid IN "
            "( SELECT object_pkid FROM " OBJECT_GEOM_DIV_TABLE
            " WHERE %s and division in ( %s ) ) AND "
            "MBRIntersects(BuildMBR( %.7lf, %.7lf, %.7lf, %.7lf ), geom ) "
            ") AS G "
            "WHERE O.pkid = R.object_pkid AND R.geom_pkid = G.pkid",
            objTypeCondition.c_str(), blockStr.c_str(), envelope[0].x, envelope[0].y, envelope[1].x, envelope[1].y);
  }
#endif
  return internalGetObjects(sql, objects, obj_geoms);
}

int sqliteOperation::getObjects(const std::vector<std::string>& names, hadmap::tx_object_vec& objects,
                                hadmap::tx_object_geom_vec& obj_geoms) {
  char sql[4096];
  sprintf(sql,
          "SELECT o.pkid, o.type, o.name, o.attribute, "
          "g.pkid, g.type, g.srs, g.dimension, g.style, g.color, g.text, g.size, g.geom "
          "FROM " OBJECT_TABLE " AS o INNER JOIN " OBJECT_GEOM_REL_TABLE
          " AS r ON o.pkid = r.object_pkid "
          "INNER JOIN " OBJECT_GEOM_TABLE " AS g ON r.geom_pkid = g.pkid");
  if (!names.empty()) {
    strcat(sql, " WHERE o.name IN (");
    char tmp[128];
    for (size_t i = 0; i < names.size(); ++i) {
      if (i == 0) {
        sprintf(tmp, "\'%s\'", names[i].c_str());
      } else {
        sprintf(tmp, ",\'%s\'", names[i].c_str());
      }
      strcat(sql, tmp);
    }
    strcat(sql, ")");
  }
  return internalGetObjects(sql, objects, obj_geoms);
}

int sqliteOperation::getObjects(const roadpkid& road_pkid, const sectionpkid& section_id, const lanepkid& lane_id,
                                const std::vector<hadmap::OBJECT_TYPE>& obj_types, hadmap::tx_object_vec& objects,
                                hadmap::tx_object_geom_vec& obj_geoms) {
  char sql[10000];
  char relCondition[128];
  char whereCondition[16];
  memset(relCondition, 0, 128);
  memset(whereCondition, 0, 16);
  if (road_pkid != ROAD_PKID_INVALID || section_id != SECT_PKID_INVALID || lane_id != LANE_PKID_INVALID) {
    sprintf(relCondition, "INNER JOIN " OBJECT_REL_TABLE " AS objr ON objr.object_pkid = o.pkid");
  }
  if (road_pkid != ROAD_PKID_INVALID || section_id != SECT_PKID_INVALID || lane_id != LANE_PKID_INVALID ||
      obj_types.size() > 0) {
    sprintf(whereCondition, "WHERE");
  }
  sprintf(sql,
          "SELECT o.pkid, o.type, o.name, o.attribute, "
          "g.pkid, g.type, g.srs, g.dimension, g.style, g.color, g.text, g.size, g.geom "
          "FROM " OBJECT_TABLE " AS o INNER JOIN " OBJECT_GEOM_REL_TABLE
          " AS r ON o.pkid = r.object_pkid "
          "INNER JOIN " OBJECT_GEOM_TABLE
          " AS g ON r.geom_pkid = g.pkid "
          "%s "
          "%s ",
          relCondition, whereCondition);
  std::string objTypeCondition = " o.type ";
  if (!obj_types.empty()) {
    objTypeCondition += "in ( ";
    for (size_t i = 0; i < obj_types.size(); ++i) {
      char typeContent[32];
      sprintf(typeContent, " %d,", obj_types[i]);
      objTypeCondition += std::string(typeContent);
    }
    objTypeCondition[objTypeCondition.length() - 1] = ')';
  } else {
    objTypeCondition += " >= 0";
  }
  if (obj_types.size() > 0) {
    strcat(sql, objTypeCondition.c_str());
  }

  if (road_pkid != ROAD_PKID_INVALID) {
    char tmp[128];
    sprintf(tmp, " AND objr.road_pkid = %lld", road_pkid);
    strcat(sql, tmp);
  }
  if (section_id != SECT_PKID_INVALID) {
    char tmp[128];
    sprintf(tmp, " AND objr.section_id = %d", section_id);
    strcat(sql, tmp);
  }
  if (lane_id != LANE_PKID_INVALID) {
    char tmp[128];
    sprintf(tmp, " AND objr.lane_id = %d", lane_id);
    strcat(sql, tmp);
  }
  return internalGetObjects(sql, objects, obj_geoms);
}

int sqliteOperation::getObjectRel(const objectpkid& objectpkid, hadmap::tx_object_geom_rel_vec& obj_rels) {
  char sql[512];
  sprintf(sql,
          "SELECT pkid, object_pkid,geom_pkid FROM TX_OBJECT_GEOM_REL "
          "WHERE object_pkid = %llu ",
          objectpkid);
  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "Failed to get objects" << std::endl;
    return HADMAP_DATA_ERROR;
  }
  while (SQLITE_ROW == sqlite3_step(stmt)) {
    tx_object_geom_rel_t rel_object;
    rel_object.pkid = sqlite3_column_int64(stmt, 0);
    rel_object.object_pkid = sqlite3_column_int64(stmt, 1);
    rel_object.geom_pkid = sqlite3_column_int64(stmt, 2);
    obj_rels.push_back(rel_object);
  }
  return HADMAP_DATA_OK;
}

bool sqliteOperation::insertTasks(hadmap::tx_task_vec& tasks) {
  const char* sql = "INSERT INTO " TASK_TABLE
                    " (name, srs, x, y,z,longitude_ref,latitude_ref, heading, pitch, roll)"
                    " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
  sqliteStmtPtr stmt(dbPtr, sql);
  bool complete = true;
  for (size_t i = 0; i < tasks.size(); i++) {
    const hadmap::tx_task_t& task = tasks[i];
    sqlite3_bind_text(stmt, 1, task.name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, task.srs);
    sqlite3_bind_double(stmt, 3, task.x);
    sqlite3_bind_double(stmt, 4, task.y);
    sqlite3_bind_double(stmt, 5, task.z);
    sqlite3_bind_double(stmt, 6, task.longitude_ref);
    sqlite3_bind_double(stmt, 7, task.latitude_ref);
    sqlite3_bind_double(stmt, 8, task.heading);
    sqlite3_bind_double(stmt, 9, task.pitch);
    sqlite3_bind_double(stmt, 10, task.roll);
    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) {
      std::cerr << "Insert Task Error, Code " << r << std::endl;
      complete = false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);

    tasks[i].pkid = sqlitePackage::getLastInsertedRowid(dbPtr);
  }
  return complete;
}

bool sqliteOperation::insertRoads(hadmap::tx_road_vec& roads) {
  char sql[1024];
  sprintf(
      sql,
      "INSERT INTO '%s' (pkid, task_pkid, id, name, srs, junction_pkid, ground, section_num, road_type, direction, \
           toll, func_class, urban, paved, transition_type, lane_mark, material, geom) \
           VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
      ROAD_TABLE);

  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "failed to prepare insert road." << std::endl;
    return false;
  }
  sqlite3_exec(dbPtr, "begin;", 0, 0, 0);

  int index_base = 1;
  bool complete = true;
  sqlite3_blob_data curvature, slope, heading, super_elevation, geom;
  for (hadmap::tx_road_vec::iterator iter = roads.begin(); iter != roads.end(); ++iter) {
    sqlite3_bind_int64(stmt, index_base, iter->pkid);
    sqlite3_bind_int64(stmt, index_base + 1, iter->task_pkid);
    sqlite3_bind_text(stmt, index_base + 2, iter->id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index_base + 3, iter->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, index_base + 4, iter->srs);
    sqlite3_bind_int64(stmt, index_base + 5, iter->junction_pkid);
    sqlite3_bind_double(stmt, index_base + 6, iter->ground);
    sqlite3_bind_int(stmt, index_base + 7, iter->section_num);
    sqlite3_bind_int(stmt, index_base + 8, iter->road_type);
    sqlite3_bind_int(stmt, index_base + 9, iter->direction);
    sqlite3_bind_int(stmt, index_base + 10, iter->toll);
    sqlite3_bind_int(stmt, index_base + 11, iter->func_class);
    sqlite3_bind_int(stmt, index_base + 12, iter->urban);
    sqlite3_bind_int(stmt, index_base + 13, iter->paved);
    sqlite3_bind_int(stmt, index_base + 14, iter->transition_type);
    sqlite3_bind_int(stmt, index_base + 15, iter->lane_mark);
    sqlite3_bind_int(stmt, index_base + 16, iter->material);

    /*
    createBlob( iter->curvature, iter->srs, curvature );
    sqlite3_bind_blob(stmt, index_base+17, curvature.data, curvature.size, SQLITE_TRANSIENT);

    createBlob( iter->slope, iter->srs, slope );
    sqlite3_bind_blob(stmt, index_base+18, slope.data, slope.size, SQLITE_TRANSIENT);

    createBlob( iter->heading, iter->srs, heading );
    sqlite3_bind_blob(stmt, index_base+19, heading.data, heading.size, SQLITE_TRANSIENT);

    createBlob( iter->super_elevation, iter->srs, super_elevation );
    sqlite3_bind_blob(stmt, index_base+20, super_elevation.data, super_elevation.size, SQLITE_TRANSIENT);
    */

    createBlob(iter->geom, iter->srs, geom);
    sqlite3_bind_blob(stmt, index_base + 17, geom.data, geom.size, SQLITE_TRANSIENT);

    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) {
      std::cerr << "Insert Road Error, Id " << iter->id << ", Code " << r << std::endl;
      std::cerr << sqlite3_errmsg(dbPtr) << std::endl;
      complete = false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);

    // iter->pkid = sqlitePackage::getLastInsertedRowid( dbPtr );

    curvature.release();
    slope.release();
    heading.release();
    super_elevation.release();
    geom.release();
  }
  sqlite3_exec(dbPtr, "commit;", 0, 0, 0);
  return complete;
}

bool sqliteOperation::insertLanes(hadmap::tx_lane_vec& lanes) {
  char sql[1024];
  sprintf(sql,
          "INSERT INTO %s(road_pkid, section_id, lane_id, name, srs, left_boundary, right_boundary, lane_type, \
           lane_arrow, speed_limit, geom) \
           VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
          LANE_TABLE);

  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "failed to prepare insert lane." << std::endl;
    return false;
  }

  sqlite3_exec(dbPtr, "begin;", 0, 0, 0);
  bool complete = true;
  sqlite3_blob_data geom;
  for (hadmap::tx_lane_vec::iterator iter = lanes.begin(); iter != lanes.end(); ++iter) {
    sqlite3_bind_int64(stmt, 1, iter->road_pkid);
    sqlite3_bind_int(stmt, 2, iter->section_id);
    sqlite3_bind_int(stmt, 3, iter->lane_id);
    sqlite3_bind_text(stmt, 4, iter->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, iter->srs);
    sqlite3_bind_int64(stmt, 6, iter->left_boundary);
    sqlite3_bind_int64(stmt, 7, iter->right_boundary);
    sqlite3_bind_int(stmt, 8, iter->lane_type);
    sqlite3_bind_int(stmt, 9, iter->lane_arrow);
    sqlite3_bind_int(stmt, 10, iter->speed_limit);

    createBlob(iter->geom, iter->srs, geom);
    sqlite3_bind_blob(stmt, 11, geom.data, geom.size, SQLITE_TRANSIENT);

    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) {
      std::cerr << "Insert Lane Error, Id " << iter->road_pkid << "-" << iter->section_id << "-" << iter->lane_id
                << ", Code " << r << std::endl;
      std::cerr << sqlite3_errmsg(dbPtr) << std::endl;
      complete = false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);

    geom.release();
  }
  sqlite3_exec(dbPtr, "commit;", 0, 0, 0);
  stmt.finalize();

  return complete;
}

bool sqliteOperation::insertLaneBoundaries(hadmap::tx_laneboundary_vec& boundaries) {
  char sql[1024];
  sprintf(sql, "INSERT INTO '%s' (pkid, srs, lane_mark, geom) VALUES (?, ?, ?, ?)", LANE_BOUNDARY_TABLE);

  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "failed to prepare insert boundary." << std::endl;
    return false;
  }
  sqlite3_exec(dbPtr, "begin;", 0, 0, 0);
  bool complete = true;
  int index_base = 1;
  sqlite3_blob_data geom;
  for (hadmap::tx_laneboundary_vec::iterator iter = boundaries.begin(); iter != boundaries.end(); ++iter) {
    sqlite3_bind_int(stmt, index_base, iter->pkid);
    sqlite3_bind_int(stmt, index_base + 1, iter->srs);
    sqlite3_bind_int(stmt, index_base + 2, iter->lane_mark);

    createBlob(iter->geom, iter->srs, geom);
    sqlite3_bind_blob(stmt, index_base + 3, geom.data, geom.size, SQLITE_TRANSIENT);

    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) {
      std::cerr << "Insert Boundary Error, Code " << r << std::endl;
      complete = false;
    }

    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);

    // iter->pkid = sqlitePackage::getLastInsertedRowid( dbPtr );

    geom.release();
  }
  sqlite3_exec(dbPtr, "commit;", 0, 0, 0);
  return complete;
}

bool sqliteOperation::insertLaneLinks(hadmap::tx_lanelink_vec& lanelinks) {
  char sql[1024];
  sprintf(sql,
          "INSERT INTO '%s' (pkid, from_road_pkid, from_section_id, from_lane_id, from_type, to_road_pkid, "
          "to_section_id, to_lane_id, to_type, junction_id, geom) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
          LANE_LINK_TABLE);
  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "failed to prepare insert lane link." << std::endl;
    return false;
  }
  sqlite3_exec(dbPtr, "begin;", 0, 0, 0);
  bool complete = true;
  sqlite3_blob_data blob;
  int index_base = 1;
  for (hadmap::tx_lanelink_vec::iterator iter = lanelinks.begin(); iter != lanelinks.end(); ++iter) {
    sqlite3_bind_int(stmt, index_base, iter->pkid);
    sqlite3_bind_int64(stmt, index_base + 1, iter->from_road_pkid);
    sqlite3_bind_int(stmt, index_base + 2, iter->from_section_id);
    sqlite3_bind_int(stmt, index_base + 3, iter->from_lane_id);
    sqlite3_bind_int(stmt, index_base + 4, iter->preContact);
    sqlite3_bind_int64(stmt, index_base + 5, iter->to_road_pkid);
    sqlite3_bind_int(stmt, index_base + 6, iter->to_section_id);
    sqlite3_bind_int(stmt, index_base + 7, iter->to_lane_id);
    sqlite3_bind_int(stmt, index_base + 8, iter->succContact);
    sqlite3_bind_int(stmt, index_base + 9, iter->junction_id);

    createBlob(iter->geom, 4326, blob);
    sqlite3_bind_blob(stmt, index_base + 10, (const void*)blob.data, blob.size, SQLITE_TRANSIENT);

    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) {
      /// std::cerr << "Insert Lane Link Error, Code " << r <<"x:"<< iter->geom.at(0).x<< std::endl;
      std::cerr << sqlite3_errmsg(dbPtr) << std::endl;
      complete = false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    blob.release();
  }
  sqlite3_exec(dbPtr, "commit;", 0, 0, 0);
  stmt.finalize();

  return complete;
}

bool sqliteOperation::insertObjects(hadmap::tx_object_vec& objects) {
  char sql[512];
  sprintf(sql, "INSERT INTO '%s' (pkid, type, name, attribute) VALUES (?, ?, ?, ?)", OBJECT_TABLE);

  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "failed to prepare insert objects." << std::endl;
    return false;
  }
  sqlite3_exec(dbPtr, "begin;", 0, 0, 0);
  bool complete = true;
  for (hadmap::tx_object_vec::iterator iter = objects.begin(); iter != objects.end(); ++iter) {
    sqlite3_bind_int(stmt, 1, iter->pkid);
    sqlite3_bind_int(stmt, 2, iter->type);
    sqlite3_bind_text(stmt, 3, iter->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 4, iter->attribute, iter->attrLen, SQLITE_TRANSIENT);
    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) {
      std::cerr << "Insert Object Error, Name " << iter->name << ", Code " << r << std::endl;
      complete = false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_exec(dbPtr, "commit;", 0, 0, 0);
  // insert tx od object
  return complete;
}

bool sqliteOperation::insertObjectGeoms(hadmap::tx_object_geom_vec& geoms) {
  char sql[512];
  sprintf(sql,
          "INSERT INTO %s(type, srs, dimension, style, color, text, size, image, geom) \
                 VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)",
          OBJECT_GEOM_TABLE);

  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "failed to prepare insert object geometries." << std::endl;
    return false;
  }

  std::vector<double> sizes;
  hadmap::Points3d linestring;
  sqlite3_blob_data blob;
  sqlite3_exec(dbPtr, "begin;", 0, 0, 0);
  bool complete = true;
  for (hadmap::tx_object_geom_vec::iterator iter = geoms.begin(); iter != geoms.end(); ++iter) {
    int index = 0;
    sqlite3_bind_int(stmt, index + 1, iter->type);
    iter->srs = 4326;
    sqlite3_bind_int(stmt, index + 2, iter->srs);
    sqlite3_bind_int(stmt, index + 3, iter->dimension);
    sqlite3_bind_int(stmt, index + 4, iter->style);
    sqlite3_bind_int(stmt, index + 5, iter->color);
    sqlite3_bind_text(stmt, index + 6, iter->text, -1, SQLITE_TRANSIENT);
    if (iter->geom.size() > 0) {
      linestring.resize(iter->geom.size());
      sizes.resize(iter->geom.size() * 2);
      for (size_t k = 0; k < iter->geom.size(); ++k) {
        linestring[k].x = iter->geom[k].x;
        linestring[k].y = iter->geom[k].y;
        linestring[k].z = iter->geom[k].z;
        sizes[k * 2] = iter->geom[k].w;
        sizes[k * 2 + 1] = iter->geom[k].h;
      }

      sqlite3_bind_blob(stmt, index + 7, &sizes[0], sizeof(double) * sizes.size(), SQLITE_TRANSIENT);
      sqlite3_bind_blob(stmt, index + 8, "", 0, SQLITE_TRANSIENT);
      createBlob(linestring, iter->srs, blob);
      sqlite3_bind_blob(stmt, index + 9, blob.data, blob.size, SQLITE_TRANSIENT);
    }
    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) {
      std::cerr << "Insert Object Geom Error, Code " << r << std::endl;
      complete = false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    iter->pkid = sqlitePackage::getLastInsertedRowid(dbPtr);
  }
  sqlite3_exec(dbPtr, "commit;", 0, 0, 0);

  return true;
}

bool sqliteOperation::insertObjectGeomRels(hadmap::tx_object_geom_rel_vec& geom_rels) {
  const char* sql = SQL_INSERT_OBJECT_GEOM_REL;

  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "failed to prepare insert object-geometry relations." << std::endl;
    return false;
  }

  sqlite3_exec(dbPtr, "begin;", 0, 0, 0);
  bool complete = true;
  for (hadmap::tx_object_geom_rel_vec::iterator iter = geom_rels.begin(); iter != geom_rels.end(); ++iter) {
    sqlite3_bind_int64(stmt, 1, iter->object_pkid);
    sqlite3_bind_int64(stmt, 2, iter->geom_pkid);
    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) {
      std::cerr << "Insert Object Geom Rel Error, Code " << r << std::endl;
      complete = false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    iter->pkid = sqlitePackage::getLastInsertedRowid(dbPtr);
  }
  sqlite3_exec(dbPtr, "commit;", 0, 0, 0);

  return complete;
}

bool sqliteOperation::insertObjectRels(hadmap::tx_object_rel_vec& rels) {
  const char* sql = SQL_INSERT_OBJECT_REL;

  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "failed to prepare insert object-road relations." << std::endl;
    return false;
  }

  bool complete = true;
  for (hadmap::tx_object_rel_vec::iterator iter = rels.begin(); iter != rels.end(); ++iter) {
    sqlite3_bind_int64(stmt, 1, iter->object_pkid);
    sqlite3_bind_int64(stmt, 2, iter->road_pkid);
    sqlite3_bind_int(stmt, 3, iter->section_id);
    sqlite3_bind_int(stmt, 4, iter->lane_id);
    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) {
      std::cerr << "Insert Object Rel Error, Code " << r << std::endl;
      complete = false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    iter->pkid = sqlitePackage::getLastInsertedRowid(dbPtr);
  }

  return complete;
}

bool sqliteOperation::insertOdObjects(hadmap::tx_od_object_vec& od_object) {
  const char* sql = SQL_INSERT_OBJECT_OD_TABLE;
  sqliteStmtPtr stmt(dbPtr, sql);
  if (!stmt.status()) {
    std::cerr << "failed to prepare insert object od table" << std::endl;
    return false;
  }
  sqlite3_exec(dbPtr, "begin;", 0, 0, 0);
  // int index = sqlitePackage::getLastInsertedRowid(dbPtr);;
  int index = 0;
  bool complete = true;
  for (auto iter = od_object.begin(); iter != od_object.end(); ++iter) {
    // sqlite3_bind_int(stmt, 0,  index++);
    sqlite3_bind_int(stmt, 1, iter->object_pkid);
    sqlite3_bind_int(stmt, 2, iter->type);
    sqlite3_bind_int(stmt, 3, iter->subtype);
    sqlite3_bind_text(stmt, 4, iter->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, iter->s);
    sqlite3_bind_double(stmt, 6, iter->t);
    sqlite3_bind_double(stmt, 7, iter->validlength);
    sqlite3_bind_int(stmt, 8, iter->orientation);
    sqlite3_bind_double(stmt, 9, iter->radius);
    sqlite3_bind_double(stmt, 10, iter->length);
    sqlite3_bind_double(stmt, 11, iter->width);
    sqlite3_bind_double(stmt, 12, iter->height);
    sqlite3_bind_double(stmt, 13, iter->hdg);
    sqlite3_bind_double(stmt, 14, iter->pitch);
    sqlite3_bind_double(stmt, 15, iter->roll);
    sqlite3_bind_double(stmt, 16, iter->zoffset);
    sqlite3_bind_int(stmt, 17, iter->roadid);
    sqlite3_bind_int(stmt, 18, iter->lanelinkid);
    sqlite3_bind_text(stmt, 19, iter->userdata, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 20, iter->repeatdata, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 21, iter->markWidth);
    sqlite3_bind_text(stmt, 22, iter->outlines, -1, SQLITE_TRANSIENT);
    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) {
      std::cerr << "Insert OdObject Error, Name " << iter->name << ", Code " << r << std::endl;
      complete = false;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_exec(dbPtr, "commit;", 0, 0, 0);
  return complete;
}

bool sqliteOperation::updateRoad(const hadmap::tx_road_t& road) {
  char sql[512];
  sprintf(sql,
          "UPDATE " ROAD_TABLE
          " "
          "SET id = ?, name = ?, srs = ?, junction_pkid = ?, ground = ?, section_num = ?, "
          "road_type = ?, direction = ?, toll = ?, func_class = ?, urban = ?, paved = ?, transition_type = ?, "
          "lane_mark = ?, material = ?, curvature = ?, slope = ?, heading = ?, super_elevation = ?, geom = ? "
          "WHERE pkid = %llu",
          road.pkid);

  sqliteStmtPtr stmt(dbPtr, sql);

  sqlite3_bind_text(stmt, 1, road.id, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, road.name, -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 3, road.srs);
  sqlite3_bind_int64(stmt, 4, road.junction_pkid);
  sqlite3_bind_double(stmt, 5, road.ground);
  sqlite3_bind_int(stmt, 6, road.section_num);
  sqlite3_bind_int(stmt, 7, road.road_type);
  sqlite3_bind_int(stmt, 8, road.direction);
  sqlite3_bind_int(stmt, 9, road.toll);
  sqlite3_bind_int(stmt, 10, road.func_class);
  sqlite3_bind_int(stmt, 11, road.urban);
  sqlite3_bind_int(stmt, 12, road.paved);
  sqlite3_bind_int(stmt, 13, road.transition_type);
  sqlite3_bind_int(stmt, 14, road.lane_mark);
  sqlite3_bind_int(stmt, 15, road.material);

  sqlite3_blob_data curvature, slope, heading, super_elevation, geom;

  createBlob(road.curvature, road.srs, curvature);
  sqlite3_bind_blob(stmt, 16, curvature.data, curvature.size, SQLITE_TRANSIENT);

  createBlob(road.slope, road.srs, slope);
  sqlite3_bind_blob(stmt, 17, slope.data, slope.size, SQLITE_TRANSIENT);

  createBlob(road.heading, road.srs, heading);
  sqlite3_bind_blob(stmt, 18, heading.data, heading.size, SQLITE_TRANSIENT);

  createBlob(road.super_elevation, road.srs, super_elevation);
  sqlite3_bind_blob(stmt, 19, super_elevation.data, super_elevation.size, SQLITE_TRANSIENT);

  createBlob(road.geom, road.srs, geom);
  sqlite3_bind_blob(stmt, 20, geom.data, geom.size, SQLITE_TRANSIENT);

  int r = sqlite3_step(stmt);
  if (SQLITE_DONE != r) std::cerr << "Update Road Error, Code " << r << std::endl;
  sqlite3_reset(stmt);

  return r == SQLITE_DONE;
}

bool sqliteOperation::updateLane(const hadmap::tx_lane_t& lane) {
  char sql[512];

  sprintf(sql,
          "UPDATE " LANE_TABLE
          " SET name = ?, srs = ?, left_boundary = ?, "
          "right_boundary = ?, lane_type = ?, lane_arrow = ?, speed_limit = ?, geom = ? "
          "WHERE road_pkid = %llu AND section_id = %u AND lane_id = %d",
          lane.road_pkid, lane.section_id, lane.lane_id);

  sqliteStmtPtr stmt(dbPtr, sql);

  sqlite3_bind_text(stmt, 1, lane.name, -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 2, lane.srs);
  sqlite3_bind_int64(stmt, 3, lane.left_boundary);
  sqlite3_bind_int64(stmt, 4, lane.right_boundary);
  sqlite3_bind_int(stmt, 5, lane.lane_type);
  sqlite3_bind_int(stmt, 6, lane.lane_arrow);
  sqlite3_bind_int(stmt, 7, lane.speed_limit);

  sqlite3_blob_data blob;
  createBlob(lane.geom, lane.srs, blob);
  sqlite3_bind_blob(stmt, 8, blob.data, blob.size, SQLITE_TRANSIENT);

  int r = sqlite3_step(stmt);
  if (SQLITE_DONE != r) std::cerr << "Update Lane Error, Code " << r << std::endl;
  sqlite3_reset(stmt);

  return r == SQLITE_DONE;
}

bool sqliteOperation::updateLaneBoundary(const hadmap::tx_laneboundary_t& boundary) {
  char sql[512];

  sprintf(sql,
          "UPDATE " LANE_BOUNDARY_TABLE
          " SET srs = ?, lane_mark = ?, geom = ? "
          "WHERE pkid = %llu",
          boundary.pkid);

  sqliteStmtPtr stmt(dbPtr, sql);

  sqlite3_bind_int(stmt, 1, boundary.srs);
  sqlite3_bind_int(stmt, 2, boundary.lane_mark);

  sqlite3_blob_data blob;
  createBlob(boundary.geom, boundary.srs, blob);
  sqlite3_bind_blob(stmt, 3, blob.data, blob.size, SQLITE_TRANSIENT);

  int r = sqlite3_step(stmt);
  if (SQLITE_DONE != r) std::cerr << "Update Boundary Error, Code " << r << std::endl;
  sqlite3_reset(stmt);

  return r == SQLITE_DONE;
}

bool sqliteOperation::updateLaneLink(const hadmap::tx_lanelink_t& lanelink) {
  char sql[512];
  sprintf(sql, "UPDATE " LANE_LINK_TABLE " SET geom = ? WHERE pkid = %llu", lanelink.pkid);
  sqliteStmtPtr stmt(dbPtr, sql);

  sqlite3_blob_data blob;
  createBlob(lanelink.geom, 4326, blob);
  sqlite3_bind_blob(stmt, 1, blob.data, blob.size, SQLITE_TRANSIENT);
  int r = sqlite3_step(stmt);
  if (SQLITE_DONE != r) std::cerr << "Update Lane Link Error, Code " << r << std::endl;
  sqlite3_reset(stmt);

  return (r == SQLITE_DONE);
}

bool sqliteOperation::updateOdObject(const hadmap::tx_od_object_t& object) {
  char sql[512];
  sprintf(
      sql,
      "UPDATE " OBJECT_OD_TABLE
      " SET s = ? , t = ?, validIength = ?, orientation = ?, radius = ?, length = ?, width = ?, height = ?, hdg = ?, \
                         pitch = ? , roll = ?, zOffset = ?,  roadpkid = ?,  lanelinkpkid = ?, userdata = ?, \
                         repeatdata = ? , markwidth = ?, outlines =? , name =? WHERE pkid = %llu",
      object.pkid);
  sqliteStmtPtr stmt(dbPtr, sql);
  sqlite3_blob_data blob;
  sqlite3_bind_double(stmt, 1, object.s);
  sqlite3_bind_double(stmt, 2, object.t);
  sqlite3_bind_double(stmt, 3, object.validlength);
  sqlite3_bind_int(stmt, 4, object.orientation);
  sqlite3_bind_double(stmt, 5, object.radius);
  sqlite3_bind_double(stmt, 6, object.length);
  sqlite3_bind_double(stmt, 7, object.width);
  sqlite3_bind_double(stmt, 8, object.height);
  sqlite3_bind_double(stmt, 9, object.hdg);
  sqlite3_bind_double(stmt, 10, object.pitch);
  sqlite3_bind_double(stmt, 11, object.roll);
  sqlite3_bind_double(stmt, 12, object.zoffset);
  sqlite3_bind_int(stmt, 13, object.roadid);
  sqlite3_bind_int(stmt, 14, object.lanelinkid);
  sqlite3_bind_text(stmt, 15, object.userdata, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 16, object.repeatdata, -1, SQLITE_TRANSIENT);
  sqlite3_bind_double(stmt, 17, object.markWidth);
  sqlite3_bind_text(stmt, 18, object.outlines, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 19, object.name, -1, SQLITE_TRANSIENT);

  int r = sqlite3_step(stmt);
  if (SQLITE_DONE != r) std::cerr << "Update object od error, Code " << r << std::endl;
  sqlite3_reset(stmt);
  return (r == SQLITE_DONE);
  return false;
}

bool sqliteOperation::updateObjectGEOM(const hadmap::tx_object_geom_t& objectgeom) {
  char sql[512];
  sprintf(sql, "UPDATE " OBJECT_GEOM_TABLE " SET type = ? , style = ?, color = ?, size = ?, geom = ? WHERE pkid = %llu",
          objectgeom.pkid);
  sqliteStmtPtr stmt(dbPtr, sql);
  sqlite3_blob_data blob;
  sqlite3_bind_int(stmt, 1, objectgeom.type);
  sqlite3_bind_int(stmt, 2, objectgeom.style);
  sqlite3_bind_int(stmt, 3, objectgeom.color);
  if (objectgeom.geom.size() > 0) {
    hadmap::Points3d linestring;
    sqlite3_blob_data blob;
    std::vector<double> sizes;
    linestring.resize(objectgeom.geom.size());
    sizes.resize(objectgeom.geom.size() * 2);
    for (size_t k = 0; k < objectgeom.geom.size(); ++k) {
      linestring[k].x = objectgeom.geom[k].x;
      linestring[k].y = objectgeom.geom[k].y;
      linestring[k].z = objectgeom.geom[k].z;
      sizes[k * 2] = objectgeom.geom[k].w;
      sizes[k * 2 + 1] = objectgeom.geom[k].h;
    }
    sqlite3_bind_blob(stmt, 4, &sizes[0], sizeof(double) * sizes.size(), SQLITE_TRANSIENT);
    createBlob(linestring, objectgeom.srs, blob);
    sqlite3_bind_blob(stmt, 5, blob.data, blob.size, SQLITE_TRANSIENT);
  }
  int r = sqlite3_step(stmt);
  if (SQLITE_DONE != r) std::cerr << "Update Lane Link Error, Code " << r << std::endl;
  sqlite3_reset(stmt);
  return (r == SQLITE_DONE);
  return false;
}

bool sqliteOperation::deleteRoad(const uint64_t& road_pkid) {
  char sql[512];
  sprintf(sql, "DELETE FROM " ROAD_TABLE " WHERE pkid = %llu", road_pkid);
  return sqlitePackage::execSql(dbPtr, sql);
}

bool sqliteOperation::deleteLane(const uint64_t& road_pkid, const uint32_t& section_id, const int8_t& lane_id) {
  char sql[512];
  sprintf(sql,
          "DELETE FROM " LANE_TABLE
          " WHERE road_pkid = %llu AND "
          "section_id = %d AND lane_id = %d",
          road_pkid, section_id, lane_id);
  return sqlitePackage::execSql(dbPtr, sql);
}

bool sqliteOperation::deleteLaneBoundary(const uint64_t& boundary_pkid) {
  char sql[512];
  sprintf(sql, "DELETE FROM " LANE_BOUNDARY_TABLE " WHERE pkid = %llu", boundary_pkid);
  return sqlitePackage::execSql(dbPtr, sql);
}

bool sqliteOperation::deleteLaneLink(const uint64_t& link_pkid) {
  char sql[512];
  sprintf(sql, "DELETE FROM " LANE_LINK_TABLE " WHERE pkid = %llu", link_pkid);
  sqliteStmtPtr stmt(dbPtr, sql);
  int r = sqlite3_step(stmt);
  if (SQLITE_DONE != r) std::cerr << "delete Lane Link Error, Code " << r << std::endl;
  sqlite3_reset(stmt);
  return true;
}

bool sqliteOperation::deleteObject(const uint64_t& object_pkid) {
  tx_object_geom_rel_vec rel;
  getObjectRel(object_pkid, rel);
  // delete geom_rel
  for (auto it : rel) {
    char sql[512];
    sprintf(sql, "DELETE FROM " OBJECT_GEOM_REL_TABLE " WHERE pkid = %llu", it.pkid);
    sqliteStmtPtr stmt(dbPtr, sql);
    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) std::cerr << "delete Lane Link Error, Code " << r << std::endl;
    sqlite3_reset(stmt);
  }
  // delete geom
  for (auto it : rel) {
    char sql[512];
    sprintf(sql, "DELETE FROM " OBJECT_GEOM_TABLE " WHERE pkid = %llu", it.geom_pkid);
    sqliteStmtPtr stmt(dbPtr, sql);
    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) std::cerr << "delete Lane Link Error, Code " << r << std::endl;
    sqlite3_reset(stmt);
  }
  // delete od_object
  for (auto it : rel) {
    char sql[512];
    sprintf(sql, "DELETE FROM " OBJECT_OD_TABLE " WHERE object_pkid = %llu", it.object_pkid);
    sqliteStmtPtr stmt(dbPtr, sql);
    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) std::cerr << "delete Lane Link Error, Code " << r << std::endl;
    sqlite3_reset(stmt);
  }
  // delete object
  for (auto it : rel) {
    char sql[512];
    sprintf(sql, "DELETE FROM " OBJECT_TABLE "   WHERE pkid = %llu", it.object_pkid);
    sqliteStmtPtr stmt(dbPtr, sql);
    int r = sqlite3_step(stmt);
    if (SQLITE_DONE != r) std::cerr << "delete Lane Link Error, Code " << r << std::endl;
    sqlite3_reset(stmt);
  }
  return false;
}
}  // namespace hadmap

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif
