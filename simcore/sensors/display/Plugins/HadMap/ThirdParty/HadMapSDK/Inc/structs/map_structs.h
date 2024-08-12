// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <stdint.h>
#include <string.h>
#include <vector>
#include "base_struct.h"
#include "types/map_defs.h"

namespace hadmap {
typedef double float64;
typedef float float32;
// Represents a 5-dimensional point with x, y, z, w, and h coordinates.
template <typename T>
struct Point5 {
  T x, y, z, w, h;

  Point5() : x(0.0), y(0), z(0), w(0), h(0) {}
  Point5(T ax, T ay, T az, T aw, T ah) : x(ax), y(ay), z(az), w(aw), h(ah) {}
  bool operator==(const Point5& other) {
    return this->x == other.x && this->y == other.y && this->z == other.z && this->w == other.w && this->h == other.h;
  }
  bool operator!=(const Point5& other) { return !operator==(other); }
};
typedef Point5<double> Point5d;
typedef Point5<float> Point5f;
typedef std::vector<Point5d> Points5d;
typedef std::vector<Point5f> Points5f;
// Represents a 4-dimensional point with x, y, z, w, and h coordinates.
template <typename T>
struct Point4 {
  T x, y, z, l;

  Point4() : x(0.0), y(0.0), z(0.0), l(0.0) {}
  Point4(T _x, T _y, T _z, T _l) : x(_x), y(_y), z(_z), l(_l) {}
  bool operator==(const Point4& other) {
    return this->x == other.x && this->y == other.y && this->z == other.z && this->l == other.l;
  }
  bool operator!=(const Point4& other) { return !operator==(other); }
};
typedef Point4<double> Point4d;
typedef Point4<float> Point4f;
typedef std::vector<Point4d> Points4d;
typedef std::vector<Point4f> Points4f;

// Represents a 3-dimensional point with x, y, z, w, and h coordinates.
template <typename T>
struct Point3 {
  T x, y, z;

  Point3() : x(0.0), y(0.0), z(0.0) {}

  Point3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
  bool operator==(const Point3& other) { return this->x == other.x && this->y == other.y && this->z == other.z; }
  bool operator!=(const Point3& other) { return !operator==(other); }
};
typedef Point3<double> Point3d;
typedef Point3<float> Point3f;
typedef std::vector<Point3d> Points3d;
typedef std::vector<Point3f> Points3f;

// Represents a 2-dimensional point with x, y, z, w, and h coordinates.
template <typename T>
struct Point2 {
  T x, y;

  Point2() : x(0.0), y(0.0) {}

  Point2(T _x, T _y) : x(_x), y(_y) {}
  bool operator==(const Point2& other) { return this->x == other.x && this->y == other.y; }
  bool operator!=(const Point2& other) { return !operator==(other); }
};
typedef Point2<double> Point2d;
typedef Point2<float> Point2f;
typedef std::vector<Point2d> Points2d;
typedef std::vector<Point2f> Points2f;

struct tx_task_t {
  taskpkid pkid;
  char name[18];
  uint32_t srs;
  float64 x;
  float64 y;
  float64 z;
  float64 longitude_ref;
  float64 latitude_ref;
  float64 heading;
  float64 pitch;
  float64 roll;
  tx_task_t() {
    memset(this, 0, sizeof(tx_task_t));
    pkid = 1;
  }
};
typedef std::vector<tx_task_t> tx_task_vec;
typedef std::vector<txCurvature> tx_Curvature_vec;
struct tx_road_t {
  roadpkid pkid;
  taskpkid task_pkid;
  char id[32];
  char name[32];
  uint32_t srs;
  junctionpkid junction_pkid;
  double ground;
  uint32_t speed_limit;
  uint32_t section_num;
  uint32_t road_type;
  uint8_t direction;
  uint8_t toll;
  uint8_t func_class;
  bool urban;
  bool paved;
  uint8_t transition_type;
  bool lane_mark;
  uint32_t material;
  Points3d curvature;
  Points3d slope;
  Points3d heading;
  Points3d super_elevation;
  Points3d geom;
  //
  tx_Curvature_vec curvaturesss;
  txSlopeVec slopevec;
  txRoadLink preLink;
  txRoadLink endLink;
  txControlPoint controlPoint;
  txControlPoint eleControlPoint;
  txOpenCrgVec opencrg;
  tx_road_t() {
    memset(this, 0, reinterpret_cast<char*>(&curvature) - reinterpret_cast<char*>(this));
    pkid = ROAD_PKID_INVALID;
    task_pkid = 1;
  }
  tx_road_t(const tx_road_t& other) {
    pkid = other.pkid;
    task_pkid = other.task_pkid;
    memcpy(id, other.id, strlen(other.id) + 1);
    memcpy(name, other.name, strlen(other.name) + 1);
    srs = other.srs;
    junction_pkid = other.junction_pkid;
    ground = other.ground;
    speed_limit = other.speed_limit;
    section_num = other.section_num;
    road_type = other.road_type;
    direction = other.direction;
    toll = other.toll;
    func_class = other.func_class;
    urban = other.urban;
    paved = other.paved;
    transition_type = other.transition_type;
    lane_mark = other.lane_mark;
    material = other.material;
    curvaturesss = other.curvaturesss;
    slopevec = other.slopevec;
    preLink = other.preLink;
    endLink = other.endLink;
    opencrg = other.opencrg;
    controlPoint = other.controlPoint;
    eleControlPoint = other.eleControlPoint;
    geom.assign(other.geom.begin(), other.geom.end());
  }
};
typedef std::vector<tx_road_t> tx_road_vec;

struct tx_lane_t {
  roadpkid road_pkid;
  sectionpkid section_id;
  lanepkid lane_id;
  char name[32];
  uint32_t srs;
  laneboundarypkid left_boundary;
  laneboundarypkid right_boundary;
  uint32_t lane_type;
  uint32_t lane_arrow;
  uint32_t speed_limit;

  uint32_t lane_surface;
  double lane_friction;
  double lane_material_soffset = 0;
  Points3d geom;
  tx_lane_t() {
    memset(this, 0, reinterpret_cast<char*>(&geom) - reinterpret_cast<char*>(this));
    road_pkid = ROAD_PKID_INVALID;
    section_id = SECT_PKID_INVALID;
    lane_id = LANE_PKID_INVALID;
    left_boundary = BOUN_PKID_INVALID;
    right_boundary = BOUN_PKID_INVALID;
  }

  tx_lane_t(const tx_lane_t& other) {
    road_pkid = other.road_pkid;
    section_id = other.section_id;
    lane_id = other.lane_id;
    memcpy(name, other.name, strlen(other.name) + 1);
    srs = other.srs;
    left_boundary = other.left_boundary;
    right_boundary = other.right_boundary;
    lane_type = other.lane_type;
    lane_arrow = other.lane_arrow;
    speed_limit = other.speed_limit;
    lane_surface = other.lane_surface;
    lane_friction = other.lane_friction;
    lane_material_soffset = other.lane_material_soffset;
    geom.assign(other.geom.begin(), other.geom.end());
  }

  tx_lane_t& operator=(const tx_lane_t& other) {
    road_pkid = other.road_pkid;
    section_id = other.section_id;
    lane_id = other.lane_id;
    memcpy(name, other.name, strlen(other.name) + 1);
    srs = other.srs;
    left_boundary = other.left_boundary;
    right_boundary = other.right_boundary;
    lane_type = other.lane_type;
    lane_arrow = other.lane_arrow;
    speed_limit = other.speed_limit;
    lane_surface = other.lane_surface;
    lane_friction = other.lane_friction;
    lane_material_soffset = other.lane_material_soffset;

    geom.assign(other.geom.begin(), other.geom.end());
    return *this;
  }
};
typedef std::vector<tx_lane_t> tx_lane_vec;

struct tx_laneboundary_t {
  laneboundarypkid pkid;
  uint32_t srs;
  uint32_t lane_mark;
  double lane_width;
  double double_interval;
  double dot_space;
  double dot_offset;
  double dot_len;
  Points3d geom;
  tx_laneboundary_t() {
    memset(this, 0, reinterpret_cast<char*>(&geom) - reinterpret_cast<char*>(this));
    pkid = BOUN_PKID_INVALID;
  }

  tx_laneboundary_t(const tx_laneboundary_t& other) {
    pkid = other.pkid;
    srs = other.srs;
    lane_mark = other.lane_mark;
    lane_width = other.lane_width;
    double_interval = other.double_interval;
    dot_space = other.dot_space;
    dot_offset = other.dot_offset;
    dot_len = other.dot_len;

    geom.assign(other.geom.begin(), other.geom.end());
  }

  tx_laneboundary_t& operator=(const tx_laneboundary_t& other) {
    pkid = other.pkid;
    srs = other.srs;
    lane_mark = other.lane_mark;
    lane_width = other.lane_width;
    double_interval = other.double_interval;
    dot_space = other.dot_space;
    dot_offset = other.dot_offset;
    dot_len = other.dot_len;

    geom.assign(other.geom.begin(), other.geom.end());
    return *this;
  }
};
typedef std::vector<tx_laneboundary_t> tx_laneboundary_vec;

struct tx_junction_t {
  junctionpkid pkid;
  char name[32];
  uint32_t srs;
  float altitude;
  Points3d geom;
  tx_junction_t() {
    memset(this, 0, reinterpret_cast<char*>(&geom) - reinterpret_cast<char*>(this));
    pkid = JUNC_PKID_INVALID;
  }
};
typedef std::vector<tx_junction_t> tx_junction_vec;

struct tx_lane_division_t {
  lanedivisionpkid pkid;
  char division[32];
  roadpkid road_pkid;
  sectionpkid section_id;
  tx_lane_division_t() { memset(this, 0, sizeof(tx_lane_division_t)); }

  tx_lane_division_t(const tx_lane_division_t& other) {
    memset(this, 0, sizeof(tx_lane_division_t));
    memcpy(this, &other, sizeof(tx_lane_division_t));
  }

  tx_lane_division_t& operator=(const tx_lane_division_t& other) {
    if (this != &other) {
      memset(this, 0, sizeof(tx_lane_division_t));
      memcpy(this, &other, sizeof(tx_lane_division_t));
    }
    return *this;
  }
};
typedef std::vector<tx_lane_division_t> tx_lane_division_vec;

struct tx_lanelink_t {
  lanelinkpkid pkid;
  roadpkid from_road_pkid;
  sectionpkid from_section_id;
  lanepkid from_lane_id;
  roadpkid to_road_pkid;
  sectionpkid to_section_id;
  lanepkid to_lane_id;
  junctionpkid junction_id;
  roadpkid odr_road_id;
  Points3d geom;
  txCurvatureVec curvature;
  txSlopeVec slope;
  txContactType preContact;
  txContactType succContact;
  txControlPoint controlPoint;
  txControlPoint eleControlPoint;
  tx_lanelink_t()
      : pkid(LINK_PKID_INVALID),
        from_road_pkid(ROAD_PKID_INVALID),
        from_section_id(SECT_PKID_INVALID),
        from_lane_id(LANE_PKID_INVALID),
        to_road_pkid(ROAD_PKID_INVALID),
        to_section_id(SECT_PKID_INVALID),
        to_lane_id(LANE_PKID_INVALID),
        junction_id(JUNC_PKID_INVALID),
        odr_road_id(ROAD_PKID_INVALID) {}
};
typedef std::vector<tx_lanelink_t> tx_lanelink_vec;

struct tx_object_t {
  // sqlite data
  objectpkid pkid;
  uint32_t type;
  uint32_t subtype;
  double length, width, height;
  double roll, pitch, yaw;
  double x, y, z;
  double s, t;  // add 20221130
  char name[64];
  char attribute[1024];
  char rawtype[1024];
  char rawsubtype[1024];

  size_t attrLen;  // the length of attribute
  // xodr
  int roadid;
  tx_object_t() {
    memset(this, 0, sizeof(tx_object_t));
    pkid = OBJ_PKID_INVALID;
  }

  tx_object_t(const tx_object_t& other)
      : pkid(other.pkid),
        type(other.type),
        subtype(other.subtype),
        length(other.length),
        width(other.width),
        height(other.height),
        roll(other.roll),
        pitch(other.pitch),
        yaw(other.yaw),
        x(other.x),
        y(other.y),
        z(other.z),
        s(other.s),
        t(other.t),
        attrLen(other.attrLen),
        roadid(other.roadid) {
    memcpy(name, other.name, sizeof(name));
    memcpy(attribute, other.attribute, sizeof(attrLen));
    memcpy(rawtype, other.rawtype, sizeof(rawtype));
    memcpy(rawsubtype, other.rawsubtype, sizeof(rawsubtype));
  }

  tx_object_t& operator=(const tx_object_t& other) {
    pkid = other.pkid;
    type = other.type;
    subtype = other.subtype;
    length = other.length;
    width = other.width;
    height = other.height;
    roll = other.roll;
    pitch = other.pitch;
    yaw = other.yaw;
    x = other.x;
    y = other.y;
    z = other.z;
    s = other.s;
    t = other.t;
    roadid = other.roadid;
    memcpy(name, other.name, sizeof(name));
    memcpy(attribute, other.attribute, sizeof(attrLen));
    memcpy(rawtype, other.rawtype, sizeof(rawtype));
    memcpy(rawsubtype, other.rawsubtype, sizeof(rawsubtype));
    attrLen = other.attrLen;
    return *this;
  }
};
typedef std::vector<tx_object_t> tx_object_vec;

struct tx_object_division_t {
  objectdivisionpkid pkid;
  char division[32];
  objectpkid object_pkid;
  uint32_t object_type;
  tx_object_division_t() { memset(this, 0, sizeof(tx_object_division_t)); }

  tx_object_division_t(const tx_object_division_t& other) {
    memset(this, 0, sizeof(tx_object_division_t));
    memcpy(this, &other, sizeof(tx_object_division_t));
  }

  tx_object_division_t& operator=(const tx_object_division_t& other) {
    if (this != &other) {
      memset(this, 0, sizeof(tx_object_division_t));
      memcpy(this, &other, sizeof(tx_object_division_t));
    }
    return *this;
  }
};
typedef std::vector<tx_object_division_t> tx_object_division_vec;

struct tx_object_geom_t {
  objectgeompkid pkid;
  uint32_t type;  // OBJECT_GEOMETRY_TYPE
  uint32_t srs;
  uint32_t dimension;
  uint32_t style;  // OBJECT_STYLE
  uint32_t color;
  char text[64];
  Points5d geom;
  std::vector<uint8_t> image;
  tx_object_geom_t() {
    memset(this, 0, reinterpret_cast<char*>(&geom) - reinterpret_cast<char*>(this));
    pkid = OBJ_GEOM_PKID_INVALID;
  }

  tx_object_geom_t(const tx_object_geom_t& other)
      : pkid(other.pkid),
        type(other.type),
        srs(other.srs),
        dimension(other.dimension),
        style(other.style),
        color(other.color),
        geom(other.geom) {
    memcpy(text, other.text, strlen(other.text) + 1);
  }

  tx_object_geom_t& operator=(const tx_object_geom_t& other) {
    pkid = other.pkid;
    type = other.type;
    srs = other.srs;
    dimension = other.dimension;
    style = other.style;
    color = other.color;
    memcpy(text, other.text, strlen(other.text) + 1);

    geom.assign(other.geom.begin(), other.geom.end());
    return *this;
  }
};
typedef std::vector<tx_object_geom_t> tx_object_geom_vec;

struct tx_object_geom_rel_t {
  objectgeomrelpkid pkid;
  objectpkid object_pkid;
  objectgeompkid geom_pkid;
  tx_object_geom_rel_t() { memset(this, 0, sizeof(tx_object_geom_rel_t)); }
};
typedef std::vector<tx_object_geom_rel_t> tx_object_geom_rel_vec;

struct tx_object_rel_t {
  objectrelpkid pkid;
  objectpkid object_pkid;
  roadpkid road_pkid;
  sectionpkid section_id;
  lanepkid lane_id;
  tx_object_rel_t() { memset(this, 0, sizeof(tx_object_rel_t)); }
};
typedef std::vector<tx_object_rel_t> tx_object_rel_vec;

struct tx_map_t {
  tx_road_vec roads;
  tx_lane_vec lanes;
  tx_laneboundary_vec boundaries;
  tx_lanelink_vec lanelinks;
};

struct tx_lane_id_t {
  roadpkid road_pkid;
  sectionpkid section_id;
  lanepkid lane_id;

  tx_lane_id_t() {}
  tx_lane_id_t(roadpkid rid, sectionpkid sid, lanepkid lid) : road_pkid(rid), section_id(sid), lane_id(lid) {}

  std::string ToString() const {
    return std::to_string(road_pkid) + "," + std::to_string(section_id) + "," + std::to_string(lane_id);
  }
};

struct tx_section_id_t {
  roadpkid road_pkid;
  sectionpkid section_id;

  tx_section_id_t() {}
  tx_section_id_t(roadpkid rid, sectionpkid sid) : road_pkid(rid), section_id(sid) {}

  std::string ToString() const { return std::to_string(road_pkid) + "," + std::to_string(section_id); }
};
struct tx_od_object_t {
  uint64_t pkid;
  uint64_t object_pkid;
  uint32_t type;
  uint32_t subtype;
  char name[64];
  double s;
  double t;
  double validlength;
  uint32_t orientation;
  double radius;
  double length;
  double width;
  double height;
  double hdg;
  double pitch;
  double roll;
  double zoffset;
  int64_t roadid;
  int64_t lanelinkid;
  double markWidth;
  char userdata[1024];
  char repeatdata[1024];
  char outlines[4096];
  tx_od_object_t() { memset(this, 0, sizeof(tx_od_object_t)); }
  tx_od_object_t& operator=(const tx_od_object_t& other) {
    pkid = other.pkid;
    object_pkid = other.object_pkid;
    type = other.type;
    subtype = other.subtype;
    s = other.s;
    t = other.t;
    validlength = other.validlength;
    orientation = other.orientation;
    length = other.length;
    width = other.width;
    height = other.height;
    hdg = other.hdg;
    roll = other.roll;
    pitch = other.pitch;
    radius = other.radius;
    zoffset = other.zoffset;
    roadid = other.roadid;
    lanelinkid = other.lanelinkid;
    markWidth = other.markWidth;
    memcpy(name, other.name, sizeof(name));
    memcpy(userdata, other.userdata, sizeof(userdata));
    memcpy(repeatdata, other.repeatdata, sizeof(repeatdata));
    memcpy(outlines, other.outlines, sizeof(outlines));
    return *this;
  }
};
typedef std::vector<tx_od_object_t> tx_od_object_vec;
}  // namespace hadmap
