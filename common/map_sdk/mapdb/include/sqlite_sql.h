// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#define PK " PRIMARY KEY AUTOINCREMENT "
#define NO_PK " "

#define TEMP_TABLE "TX_TEMP"
#define SQL_CREATE_TEMP_TABLE \
  "CREATE TABLE " TEMP_TABLE  \
  " (\
string_column VARCHAR(256), \
integer_column INTEGER)"

#define TASK_TABLE "TX_TASK"
#define SQL_CREATE_TASK_TABLE \
  "CREATE TABLE " TASK_TABLE  \
  " (\
pkid INTEGER PRIMARY KEY AUTOINCREMENT, \
name VARCHAR(18), \
srs INTEGER, \
x REAL, \
y REAL, \
z REAL, \
longitude_ref REAL, \
latitude_ref REAL, \
heading REAL, \
pitch REAL, \
roll REAL)"

#define SVID_TABLE "TX_SVID"
#define SQL_CREATE_SVID_TABLE \
  "CREATE TABLE " SVID_TABLE  \
  " (\
pkid INTEGER PRIMARY KEY AUTOINCREMENT, \
task_pkid INTEGER, \
svid VARCHAR(24), \
longitude REAL, \
latitude REAL, \
longitude_encrypt REAL, \
latitude_encrypt REAL, \
altitude REAL, \
heading REAL, \
pitch REAL, \
roll REAL, \
speed REAL, \
FOREIGN KEY(task_pkid) REFERENCES " TASK_TABLE "(pkid) ON DELETE CASCADE)"

#define ROAD_TABLE "TX_ROAD"
#define SQL_CREATE_ROAD_TABLE \
  "CREATE TABLE " ROAD_TABLE  \
  "(\
pkid INTEGER PRIMARY KEY AUTOINCREMENT, \
task_pkid INTEGER, \
id VARCHAR(32), \
name VARCHAR(32), \
srs INTEGER, \
junction_pkid INTEGER, \
ground REAL, \
section_num INTEGER, \
road_type INTEGER, \
direction INTEGER, \
toll INTEGER, \
func_class INTEGER, \
urban INTEGER, \
paved INTEGER, \
transition_type INTEGER, \
lane_mark INTEGER, \
material INTEGER, \
curvature BLOB, \
slope BLOB, \
heading BLOB, \
super_elevation BLOB)"
// FOREIGN KEY(task_pkid) REFERENCES " TASK_TABLE "(pkid) ON DELETE CASCADE)
#define SQL_INSERT_ROAD                                                                           \
  "INSERT INTO " ROAD_TABLE                                                                       \
  " (task_pkid, id, name, srs, junction_pkid, ground, section_num, road_type, direction, toll, \
    func_class, urban, paved, transition_type, lane_mark, material, curvature, slope, heading, super_elevation, geom)" \
  " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
#define SQL_SELECT_ROAD \
  "SELECT pkid, task_pkid, id, name, srs, junction_pkid, ground, section_num, road_type, \
         direction, toll, func_class, urban, paved, transition_type, lane_mark, material, \
         curvature, slope, heading, super_elevation, geom FROM " ROAD_TABLE
#define SQL_DELETE_ROAD "DELETE FROM " ROAD_TABLE " WHERE id='%s'"

#define LANE_BOUNDARY_TABLE "TX_LANE_BOUNDARY"
#define SQL_CREATE_LANE_BOUNDARY_TABLE \
  "CREATE TABLE " LANE_BOUNDARY_TABLE  \
  " (\
pkid INTEGER PRIMARY KEY AUTOINCREMENT, \
srs INTEGER, \
lane_mark INTEGER)"
#define SQL_SELECT_LANE_BOUNDARY "SELECT pkid, srs, lane_mark, geom FROM " LANE_BOUNDARY_TABLE

#define LANE_TABLE "TX_LANE"
#define SQL_CREATE_LANE_TABLE                               \
  "CREATE TABLE " LANE_TABLE                                \
  "(\
road_pkid INTEGER, \
section_id INTEGER, \
lane_id INTEGER, \
name VARCHAR(32), \
srs INTEGER, \
left_boundary INTEGER, \
right_boundary INTEGER, \
lane_type INTEGER, \
lane_arrow INTEGER, \
speed_limit INTEGER, \
PRIMARY KEY(road_pkid, section_id, lane_id), \
FOREIGN KEY(road_pkid) REFERENCES " ROAD_TABLE              \
  "(pkid) ON DELETE CASCADE, \
FOREIGN KEY(left_boundary) REFERENCES " LANE_BOUNDARY_TABLE \
  "(pkid) ON DELETE CASCADE, \
FOREIGN KEY(right_boundary) REFERENCES " LANE_BOUNDARY_TABLE "(pkid) ON DELETE CASCADE)"
#define SQL_INSERT_LANE                                                                                              \
  "INSERT INTO " LANE_TABLE                                                                                          \
  " (road_pkid, section_id, lane_id, name, srs, left_boundary, right_boundary, lane_type, lane_arrow, speed_limit, " \
  "geom)"                                                                                                            \
  " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
#define SQL_SELECT_LANE                                                                                      \
  "SELECT road_pkid, section_id, lane_id, name, srs, left_boundary, right_boundary, lane_type, lane_arrow, " \
  "speed_limit, geom FROM " LANE_TABLE
#define SQL_DELETE_LANE "DELETE FROM " LANE_TABLE "  WHERE road_pkid=%d AND section_id=%d AND lane_id=%d"

#define JUNCTION_TABLE "TX_JUNCTION"
#define SQL_CREATE_JUNCTION_TABLE \
  "CREATE TABLE " JUNCTION_TABLE  \
  "(\
pkid INTEGER PRIMARY KEY AUTOINCREMENT, \
name VARCHAR(32), \
srs INTEGER, \
altitude REAL)"

#define SQL_INSERT_JUNCTION      \
  "INSERT INTO " JUNCTION_TABLE  \
  " (name, srs, altitude, geom)" \
  " VALUES (?, ?, ?, ?)"

#define LANE_LINK_TABLE "TX_LANE_LINK"
#define SQL_CREATE_LANE_LINK_TABLE                                                 \
  "CREATE TABLE " LANE_LINK_TABLE                                                  \
  "(\
pkid INTEGER PRIMARY KEY AUTOINCREMENT, \
from_road_pkid INTEGER, \
from_section_id INTEGER, \
from_lane_id INTEGER, \
from_type INTEGER, \
to_road_pkid INTEGER, \
to_section_id INTEGER, \
to_lane_id INTEGER, \
to_type INTEGER, \
junction_id INTEGER, \
FOREIGN KEY(from_road_pkid, from_section_id, from_lane_id) REFERENCES " LANE_TABLE \
  "(road_pkid, section_id, lane_id) ON DELETE CASCADE, \
FOREIGN KEY(to_road_pkid, to_section_id, to_lane_id) REFERENCES " LANE_TABLE       \
  "(road_pkid, section_id, lane_id) ON DELETE CASCADE)"

#define SQL_INSERT_LANE_LINK_(pkid, value)                                                                       \
  "INSERT INTO " LANE_LINK_TABLE " (" pkid                                                                       \
  "from_road_pkid, from_section_id, from_lane_id, from_type, to_road_pkid, to_section_id, to_lane_id, to_type, " \
  "junction_id, geom)"                                                                                           \
  " VALUES (" value "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
// OLD SQLITE
#define SQL_SELECT_LANE_LINK_OLD                                                                                \
  "SELECT pkid, from_road_pkid, from_section_id, from_lane_id, to_road_pkid, to_section_id, to_lane_id, geom, " \
  "junction "                                                                                                   \
  "FROM " LANE_LINK_TABLE

// Map Edit SQLITE
#define SQL_INSERT_LANE_LINK SQL_INSERT_LANE_LINK_("", "")
#define SQL_INSERT_LANE_LINK_PKID SQL_INSERT_LANE_LINK_("pkid,", "?,")
#define SQL_SELECT_LANE_LINK                                                                                          \
  "SELECT pkid, from_road_pkid, from_section_id, from_lane_id,  from_type, to_road_pkid, to_section_id, to_lane_id, " \
  "to_type, junction_id, geom FROM " LANE_LINK_TABLE

#define OBJECT_TABLE "TX_OBJECT"
#define SQL_CREATE_OBJECT_TABLE(pk) \
  "CREATE TABLE " OBJECT_TABLE      \
  "(\
pkid INTEGER " pk                   \
  ", \
type INTEGER, \
name VARCHAR(32), \
attribute VARCHAR(1024))"

#define OBJECT_OD_TABLE "TX_OD_OBJECT"
#define SQL_CREATE_OBJECT_OD_TABLE \
  "CREATE TABLE " OBJECT_OD_TABLE  \
  "(\
pkid INTEGER PRIMARY KEY AUTOINCREMENT, \
object_pkid INTEGER, \
type INTEGER, \
subtype INTEGER, \
name  VARCHAR(32), \
s REAL, \
t REAL, \
validIength REAL, \
orientation INTEGER, \
radius REAL, \
length REAL, \
width REAL, \
height REAL, \
hdg REAL, \
pitch REAL, \
roll REAL, \
zOffset REAL, \
roadpkid int, \
lanelinkpkid int, \
userdata VARCHAR(1024), \
repeatdata VARCHAR(1024), \
markwidth REAL, \
outlines VARCHAR(4096), \
FOREIGN KEY(object_pkid) REFERENCES " OBJECT_TABLE "(pkid) ON DELETE CASCADE)"

#define SQL_INSERT_OBJECT_OD_TABLE \
  "INSERT INTO " OBJECT_OD_TABLE   \
  " (object_pkid, type, subtype, name, s, t, validIength, orientation, radius, length, width, height, hdg, pitch, \
                   roll, zOffset, roadpkid, lanelinkpkid, userdata, repeatdata, markwidth, outlines) VALUES \
                   (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"

#define OBJECT_GEOM_TABLE "TX_OBJECT_GEOM"
#define SQL_CREATE_OBJECT_GEOM_TABLE \
  "CREATE TABLE " OBJECT_GEOM_TABLE  \
  "(\
pkid INTEGER PRIMARY KEY AUTOINCREMENT, \
type INTEGER, \
srs INTEGER, \
dimension INTEGER, \
style INTEGER, \
color INTEGER, \
text VARCHAR(32), \
size BLOB, \
image BLOB)"

#define OBJECT_GEOM_DIV_TABLE "TX_OBJECT_DIVISION"
#define LANE_GEOM_DIV_TABLE "TX_LANE_DIVISION"

/** An object may contains several geometry components. */
#define OBJECT_GEOM_REL_TABLE "TX_OBJECT_GEOM_REL"
#define SQL_CREATE_OBJECT_GEOM_REL_TABLE(pk)       \
  "CREATE TABLE " OBJECT_GEOM_REL_TABLE            \
  "(\
pkid INTEGER " pk                                  \
  ", \
object_pkid INTEGER, \
geom_pkid INTEGER, \
FOREIGN KEY(object_pkid) REFERENCES " OBJECT_TABLE \
  "(pkid) ON DELETE CASCADE, \
FOREIGN KEY(geom_pkid) REFERENCES " OBJECT_GEOM_TABLE "(pkid) ON DELETE CASCADE)"
#define SQL_INSERT_OBJECT_GEOM_REL "INSERT INTO " OBJECT_GEOM_REL_TABLE " (object_pkid, geom_pkid) VALUES (?, ?)"

/** An object may links to a road or a lane. */
#define OBJECT_REL_TABLE "TX_OBJECT_REL"
#define SQL_CREATE_OBJECT_REL_TABLE(pk)            \
  "CREATE TABLE " OBJECT_REL_TABLE                 \
  "(\
pkid INTEGER " pk                                  \
  ", \
object_pkid INTEGER, \
road_pkid INTEGER, \
section_id INTEGER, \
lane_id INTEGER, \
FOREIGN KEY(object_pkid) REFERENCES " OBJECT_TABLE \
  "(pkid) ON DELETE CASCADE, \
FOREIGN KEY(road_pkid) REFERENCES " ROAD_TABLE "(pkid) )"
#define SQL_INSERT_OBJECT_REL \
  "INSERT INTO " OBJECT_REL_TABLE " (object_pkid, road_pkid, section_id, lane_id) VALUES (?, ?, ?, ?)"

// VirtualSpatialIndex
#define ROWID_BY_VSI(table) \
  "SELECT ROWID FROM SpatialIndex WHERE \
f_table_name = '" table "' and f_geometry_column = 'geom' and search_frame = BuildMBR(%lf, %lf, %lf, %lf)"
