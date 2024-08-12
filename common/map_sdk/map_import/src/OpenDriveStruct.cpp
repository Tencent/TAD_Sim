// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "OpenDriveStruct.h"

#include <assert.h>
#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include "../../datamodel/include/common/map_util.h"
using namespace tinyxml2;

#define MAX_LENGTH_ERROR 30
namespace hadmap {
#define ELE_CKECK elementCheck(element)
std::string StrCheckEmpty(const char* s) {
  if (s == nullptr) {
    return std::string();
  }
  std::string ns = s;
  bool hasChn = false;
  for (const auto& c : ns) {
    if (c < 0) {
      hasChn = true;
      break;
    }
  }
  if (hasChn) {
    setlocale(LC_ALL, "en_US.utf8");
    int len = mbstowcs(nullptr, ns.c_str(), ns.size());
    if (len > 0) {
      std::wstring wstr;
      wstr.resize(len, 0);
      mbstowcs(const_cast<wchar_t*>(wstr.data()), ns.c_str(), len);
      setlocale(LC_ALL, "");
      len = wcstombs(nullptr, wstr.c_str(), wstr.size());
      if (len > 0) {
        ns.clear();
        ns.resize(len, 0);
        wcstombs(const_cast<char*>(ns.data()), wstr.c_str(), len);
      }
    }
  }
  return ns;
}

ODXYZ& ODXYZ::operator+=(const ODXYZ& rhs) {
  // TODO(undefined): insert return statement here
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  return *this;
}

ODXYZ ODXYZ::operator+(const ODXYZ& rhs) const { return ODXYZ(x + rhs.x, y + rhs.y, z + rhs.z); }

ODXYZ ODRPY::operator*(const ODXYZ& p) const {
  double s = p.x * std::cos(hdg) - p.y * std::sin(hdg);
  double t = p.x * std::sin(hdg) + p.y * std::cos(hdg);
  return ODXYZ(s, t, p.z);
}

/* ODNode */
void ODNode::elementCheck(XMLElement* element) {
  if (element == NULL || nodeName != std::string(element->Name())) {
    throw std::runtime_error("OpenDrive element parse error, no matching element");
  }
}

/*ODHeader*/
bool ODHeader::parse(XMLElement* element) {
  ELE_CKECK;

  north = element->DoubleAttribute("north");
  south = element->DoubleAttribute("south");
  east = element->DoubleAttribute("east");
  west = element->DoubleAttribute("west");
  if (element->Attribute("vendor")) {
    vendor = element->Attribute("vendor");
  }
  if (element->Attribute("revMajor")) {
    remajor = element->Int64Attribute("revMajor");
  }
  if (element->Attribute("revMinor")) {
    reminor = element->Int64Attribute("revMinor");
  }
  if (element->Attribute("date")) {
    date = element->Attribute("date");
  }
  XMLElement* geo_ref = element->FirstChildElement("geoReference");
  if (geo_ref) coord_str = geo_ref->GetText();

  return true;
}

void ODHeader::getBBox(double& n, double& s, double& e, double& w) {
  n = north;
  s = south;
  e = east;
  w = west;
}

std::string ODHeader::getGeoRef() const { return coord_str; }

std::string ODHeader::getVendor() const { return vendor; }

std::string ODHeader::getDate() const { return date; }

uint8_t ODHeader::getRemajor() const { return remajor; }

uint8_t ODHeader::getReminor() const { return reminor; }

/* ODLink */
bool ODLink::parse(XMLElement* element) {
  ELE_CKECK;

  if (element->FirstChildElement("predecessor")) {
    std::string type_str = StrCheckEmpty(element->FirstChildElement("predecessor")->Attribute("elementType"));
    from_type = type_str == "road" ? ROAD : JUNC;
    from_rid = element->FirstChildElement("predecessor")->Int64Attribute("elementId");
    type_str = StrCheckEmpty(element->FirstChildElement("predecessor")->Attribute("contactPoint"));
    from_cont = type_str == "start" ? START : END;

  } else {
    from_rid = ROAD_PKID_INVALID;
  }

  if (element->FirstChildElement("successor")) {
    std::string type_str = StrCheckEmpty(element->FirstChildElement("successor")->Attribute("elementType"));
    to_type = type_str == "road" ? ROAD : JUNC;
    to_rid = element->FirstChildElement("successor")->Int64Attribute("elementId");
    type_str = StrCheckEmpty(element->FirstChildElement("successor")->Attribute("contactPoint"));
    to_cont = type_str == "end" ? END : START;
  } else {
    to_rid = ROAD_PKID_INVALID;
  }

  return true;
}

roadpkid ODLink::fromRoadId() const { return from_rid; }

void ODLink::clearFromRoad() { from_rid = ROAD_PKID_INVALID; }

roadpkid ODLink::toRoadId() const { return to_rid; }

void ODLink::clearToRoad() { to_rid = ROAD_PKID_INVALID; }

ODLink::ODLinkType ODLink::fromRoadType() const { return from_type; }

ODLink::ODLinkType ODLink::toRoadType() const { return to_type; }

ODLink::ODConType ODLink::fromRoadCont() const { return from_cont; }

ODLink::ODConType ODLink::toRoadCont() const { return to_cont; }

/* ODConnection */
bool ODConnection::parse(XMLElement* element) {
  ELE_CKECK;

  from_rid = element->Int64Attribute("incomingRoad");
  to_rid = element->Int64Attribute("connectingRoad");

  XMLElement* link_elem = element->FirstChildElement("laneLink");
  while (link_elem) {
    std::pair<lanepkid, lanepkid> link_pair;
    link_pair.first = link_elem->Int64Attribute("from");
    link_pair.second = link_elem->Int64Attribute("to");
    lane_links.push_back(link_pair);
    link_elem = link_elem->NextSiblingElement("laneLink");
  }
  return true;
}

size_t ODConnection::lanelinkSize() const { return lane_links.size(); }

roadpkid ODConnection::fromRoadId() const { return from_rid; }

roadpkid ODConnection::toRoadId() const { return to_rid; }

std::pair<lanepkid, lanepkid> ODConnection::lanelink(const size_t& index) const {
  if (index >= lane_links.size()) throw std::exception();
  return lane_links[index];
}

/* ODJunction */
bool ODJunction::parse(XMLElement* element) {
  ELE_CKECK;

  jid = element->Int64Attribute("id");
  XMLElement* conn_elem = element->FirstChildElement("connection");
  while (conn_elem) {
    ODConnectionPtr conn_ptr(new ODConnection);
    conn_ptr->parse(conn_elem);
    connections.push_back(conn_ptr);
    conn_elem = conn_elem->NextSiblingElement("connection");
  }
  XMLElement* ctl_elem = element->FirstChildElement("controller");
  while (ctl_elem) {
    controllerkid id = ctl_elem->Int64Attribute("id");
    controllers.push_back(id);
    ctl_elem = ctl_elem->NextSiblingElement("controller");
  }
  return true;
}

junctionpkid ODJunction::id() const { return jid; }

size_t ODJunction::connectionSize() const { return connections.size(); }

ODConnectionPtr ODJunction::connection(const size_t& index) {
  if (index >= connections.size()) return ODConnectionPtr();
  return connections[index];
}

const std::vector<controllerkid>& ODJunction::getControllers() const { return controllers; }

/* Poly3D */
bool Polynomial3D::parse(XMLElement* element) {
  ELE_CKECK;

  s = element->DoubleAttribute("s");
  a = element->DoubleAttribute("a");
  b = element->DoubleAttribute("b");
  c = element->DoubleAttribute("c");
  d = element->DoubleAttribute("d");

  return true;
}

bool Polynomial3D::sampleValue(const std::vector<double>& dis, std::vector<double>& value) const {
  value.resize(dis.size());
  for (size_t i = 0; i < dis.size(); ++i) {
    value[i] = a + b * dis[i] + c * dis[i] * dis[i] + d * dis[i] * dis[i] * dis[i];
  }
  return true;
}

double Polynomial3D::value(const double& dis) const { return a + b * dis + c * dis * dis + d * dis * dis * dis; }

double Polynomial3D::slope(const double& dis) const { return b + 2 * c * dis + 3 * d * dis * dis; }

void Polynomial3D::getParam(double& _a, double& _b, double& _c, double& _d, double& _s) const {
  _s = s;
  _a = a;
  _b = b;
  _c = c;
  _d = d;
}

/* ODLane */
bool ODLane::parse(XMLElement* element) {
  ELE_CKECK;

  // parse lane id
  lid = element->Int64Attribute("id");

  // parse lane type
  std::string typeStr = StrCheckEmpty(element->Attribute("type"));
  if (typeStr == "none") {
    lane_type = LANE_TYPE_None;
  } else if (typeStr == "HOV") {
    lane_type = LANE_TYPE_HOV;
  } else if (typeStr == "bidirectional") {
    lane_type = LANE_TYPE_BIDIRECTIONAL;
  } else if (typeStr == "biking") {
    lane_type = LANE_TYPE_Biking;
  } else if (typeStr == "border") {
    lane_type = LANE_TYPE_Border;
  } else if (typeStr == "busonly") {
    lane_type = LANE_TYPE_BusOnly;
  } else if (typeStr == "bus") {
    lane_type = LANE_TYPE_BusOnly;
  } else if (typeStr == "connectingRamp") {
    lane_type = LANE_TYPE_CONNECTINGRAMP;
  } else if (typeStr == "curb") {
    lane_type = LANE_TYPE_CURB;
  } else if (typeStr == "driving") {
    lane_type = LANE_TYPE_Driving;
  } else if (typeStr == "entry") {
    lane_type = LANE_TYPE_ENTRY;
  } else if (typeStr == "exit") {
    lane_type = LANE_TYPE_EXIT;
  } else if (typeStr == "median") {
    lane_type = LANE_TYPE_MEDIAN;
  } else if (typeStr == "mwyEntry") {
    lane_type = LANE_TYPE_MwyEntry;
  } else if (typeStr == "mwyExit") {
    lane_type = LANE_TYPE_MwyExit;
  } else if (typeStr == "offRamp") {
    lane_type = LANE_TYPE_OFFRAMP;
  } else if (typeStr == "onRamp") {
    lane_type = LANE_TYPE_ONRAMP;
  } else if (typeStr == "parking") {
    lane_type = LANE_TYPE_Parking;
  } else if (typeStr == "rail") {
    lane_type = LANE_TYPE_RAIL;
  } else if (typeStr == "restricted") {
    lane_type = LANE_TYPE_Restricted;
  } else if (typeStr == "roadWorks") {
    lane_type = LANE_TYPE_ROADWORKS;
  } else if (typeStr == "shoulder") {
    lane_type = LANE_TYPE_Shoulder;
  } else if (typeStr == "sidewalk") {
    lane_type = LANE_TYPE_Sidewalk;
  } else if (typeStr == "stop") {
    lane_type = LANE_TYPE_Stop;
  } else if (typeStr == "taxi") {
    lane_type = LANE_TYPE_TAXI;
  } else if (typeStr == "tram") {
    lane_type = LANE_TYPE_TRAM;
  } else if (typeStr == "reversible") {
    lane_type = LANE_TYPE_Reversible;
  } else {
    lane_type = LANE_TYPE_Others;
  }

  // parse link info
  if (element->FirstChildElement("link")) {
    if (element->FirstChildElement("link")->FirstChildElement("predecessor")) {
      from_lid = element->FirstChildElement("link")->FirstChildElement("predecessor")->Int64Attribute("id");
    } else {
      from_lid = LANE_PKID_INVALID;
    }

    if (element->FirstChildElement("link")->FirstChildElement("successor")) {
      to_lid = element->FirstChildElement("link")->FirstChildElement("successor")->Int64Attribute("id");
    } else {
      to_lid = LANE_PKID_INVALID;
    }
  } else {
    from_lid = LANE_PKID_INVALID;
    to_lid = LANE_PKID_INVALID;
  }

  std::string markStr = "none";
  std::string colorStr = "standard";
  laneboundary_width = 0;
  laned_interval = 0;
  laned_space = 0;
  laned_soffset = 0;
  laned_len = 0;

  // parse lane mark
  auto roadmark = element->FirstChildElement("roadMark");
  if (roadmark) {
    markStr = StrCheckEmpty(roadmark->Attribute("type"));
    colorStr = StrCheckEmpty(roadmark->Attribute("color"));
    // parse lane boundary width
    laneboundary_width = roadmark->DoubleAttribute("width");
    // parse line
    auto xtype = roadmark->FirstChildElement("type");
    while (xtype) {
      if (StrCheckEmpty(xtype->Attribute("name")) == markStr) {
        auto line1 = xtype->FirstChildElement("line");
        if (line1) {
          laned_space = line1->DoubleAttribute("space");
          laned_soffset = line1->DoubleAttribute("sOffset");
          laned_len = line1->DoubleAttribute("length");
          auto line2 = line1->NextSiblingElement("line");
          if (line2) {
            laned_interval = std::abs(line1->DoubleAttribute("tOffset") - line2->DoubleAttribute("tOffset"));

            laned_space = std::max(laned_space, line2->DoubleAttribute("space"));
            laned_soffset = std::max(laned_soffset, line2->DoubleAttribute("sOffset"));
            laned_len = std::max(laned_len, line2->DoubleAttribute("length"));
          }
        }
        break;
      }
      xtype = xtype->NextSiblingElement("type");
    }
  }

  if (markStr == "solid") {
    lane_mark = LANE_MARK_Solid;
  } else if (markStr == "broken") {
    lane_mark = LANE_MARK_Broken;
  } else if (markStr == "solid solid") {
    lane_mark = LANE_MARK_SolidSolid;
  } else if (markStr == "solid broken") {
    lane_mark = LANE_MARK_SolidBroken;
  } else if (markStr == "broken solid") {
    lane_mark = LANE_MARK_BrokenSolid;
  } else if (markStr == "broken broken") {
    lane_mark = LANE_MARK_BrokenBroken;
  } else if (markStr == "curb") {
    lane_mark = LANE_MARK_Curb;
  } else {
    lane_mark = LANE_MARK_None;
  }

  if (lane_mark != LANE_MARK_None) {
    if (colorStr == "yellow") {
      lane_mark = (LANE_MARK)(static_cast<int>(lane_mark) | LANE_MARK_Yellow);
      // if (lane_mark & LANE_MARK_Solid2 || lane_mark & LANE_MARK_Broken2)
      // {
      //  lane_mark = (LANE_MARK)((int)lane_mark | LANE_MARK_Yellow2);
      // }
    } else if (colorStr == "blue") {
      lane_mark = (LANE_MARK)(static_cast<int>(lane_mark) | LANE_MARK_Blue);
    } else if (colorStr == "green") {
      lane_mark = (LANE_MARK)(static_cast<int>(lane_mark) | LANE_MARK_Green);
    } else if (colorStr == "red") {
      lane_mark = (LANE_MARK)(static_cast<int>(lane_mark) | LANE_MARK_Red);
    }
    // else if (colorStr == "")
  }

  // parse speed limit
  // if (lane_type != LANE_TYPE_Driving){
  speed_limit = 0;
  if (element->FirstChildElement("speed")) {
    double speed_max = element->FirstChildElement("speed")->DoubleAttribute("max");
    auto unit = StrCheckEmpty(element->FirstChildElement("speed")->Attribute("unit"));

    if (unit == "km/h") {
      speed_max *= 0.277778;
    } else if (unit == "mph") {
      speed_max *= 0.44704;
    }
    speed_limit = std::max(0, static_cast<int>(std::round(speed_max * 3.6)));
  }

  // parse s
  XMLElement* width_elem = element->FirstChildElement("width");
  while (width_elem) {
    GeomParam gp;
    gp.s = width_elem->DoubleAttribute("sOffset");
    gp.a = width_elem->DoubleAttribute("a");
    if (std::isinf(gp.a) || std::isnan(gp.a)) {
      gp.a = 0;
    }
    gp.b = width_elem->DoubleAttribute("b");
    if (std::isinf(gp.b) || std::isnan(gp.b)) {
      gp.b = 0;
    }
    gp.c = width_elem->DoubleAttribute("c");
    if (std::isinf(gp.c) || std::isnan(gp.c)) {
      gp.c = 0;
    }
    gp.d = width_elem->DoubleAttribute("d");
    if (std::isinf(gp.d) || std::isnan(gp.d)) {
      gp.d = 0;
    }
    params.push_back(gp);
    width_elem = width_elem->NextSiblingElement("width");
  }
  /*
  s = element->FirstChildElement( "width" )->DoubleAttribute( "sOffset" );

  // parse width param
  XMLElement* widthElem = element->FirstChildElement( "width" );
  if ( widthElem != NULL )
  {
          s = widthElem->DoubleAttribute( "sOffset" );
          a = widthElem->DoubleAttribute( "a" );
          b = widthElem->DoubleAttribute( "b" );
          c = widthElem->DoubleAttribute( "c" );
          d = widthElem->DoubleAttribute( "d" );
  }
  */
  // parse  material
  auto material = element->FirstChildElement("material");
  if (material) {
    if (material->Attribute("surface")) {
      std::string strsurface = material->Attribute("surface");
      if (strsurface == "cement") {
        this->lane_surface = LANE_SURFACE_CEMENT;
      } else if (strsurface == "asphalt") {
        this->lane_surface = LANE_SURFACE_ASPHALT;
      } else if (strsurface == "cobblestone") {
        this->lane_surface = LANE_SURFACE_COBBLESTONE;
      } else if (strsurface == "dirt") {
        this->lane_surface = LANE_SURFACE_DIRT;
      } else {
        this->lane_surface = LANE_SURFACE_OTHER;
      }
    }
    if (material->Attribute("friction")) {
      this->lane_friction = std::atof(material->Attribute("friction"));
    }
    lane_material_sOffset = 0;
    if (material->Attribute("sOffset")) {
      lane_material_sOffset = material->DoubleAttribute("sOffset");
    }
  }
  return true;
  return !params.empty() || typeStr == "virtual";
}

lanepkid ODLane::id() const { return lid; }

lanepkid ODLane::fromLId() const { return from_lid; }

lanepkid ODLane::toLId() const { return to_lid; }

LANE_MARK ODLane::mark() const { return lane_mark; }

LANE_TYPE ODLane::type() const { return lane_type; }

LANE_SURFACE ODLane::surface() const { return lane_surface; }

double ODLane::friction() const { return lane_friction; }

int ODLane::speedlimit() const { return speed_limit; }

double ODLane::lanewidth() const { return laneboundary_width; }

double ODLane::lanedinterval() const { return laned_interval; }

double ODLane::offset() const {
  if (params.empty()) return 0.0;
  return params.front().s;
}

void ODLane::width(const std::vector<double>& ds, std::vector<double>& width) const {
  width.resize(ds.size(), 0.0);
  size_t cur_i = 0;
  for (size_t i = 0; i < ds.size(); ++i) {
    while ((cur_i + 1) < params.size() && ds[i] > params[cur_i + 1].s) cur_i++;
    if (cur_i >= params.size()) {
      break;
    }
    double cur_d = ds[i] - params[cur_i].s;
    double a = params[cur_i].a;
    double b = params[cur_i].b;
    double c = params[cur_i].c;
    double d = params[cur_i].d;
    width[i] = a + b * cur_d + c * cur_d * cur_d + d * cur_d * cur_d * cur_d;
  }
}

/* ODGeometry */
bool ODGeometry::parse(tinyxml2::XMLElement* element) {
  _s = element->DoubleAttribute("s");
  _x = element->DoubleAttribute("x");
  _y = element->DoubleAttribute("y");
  _hdg = element->DoubleAttribute("hdg");
  _len = std::abs(element->DoubleAttribute("length"));

  return true;
}

double ODGeometry::offset() const { return _s; }

double ODGeometry::startX() const { return _x; }

double ODGeometry::startY() const { return _y; }

double ODGeometry::heading() const { return _hdg; }

double ODGeometry::length() const { return _len; }

void ODGeometry::setLength(double length) { _len = length; }

void ODGeometry::getGeom(std::vector<std::pair<double, double>>& points) const {
  if (length() < 1e-6) {
    return;
  }
  std::vector<double> intervals;
  getInterval(intervals);
  points.resize(intervals.size());
  for (size_t i = 0; i < intervals.size(); i++) {
    double x = 0, y = 0;
    getGeom(intervals[i] + offset(), x, y);
    points[i] = std::make_pair(x - startX(), y - startY());
  }
}

void ODGeometry::getTangent(std::vector<std::pair<double, double>>& tangents) const {
  if (length() < 1e-6) {
    return;
  }
  std::vector<double> intervals;
  getInterval(intervals);
  tangents.resize(intervals.size());
  for (size_t i = 0; i < intervals.size(); i++) {
    double h = getHeading(intervals[i] + offset());
    tangents[i] = std::make_pair(std::cos(h), std::sin(h));
  }
}

void ODGeometry::getNormal(std::vector<std::pair<double, double>>& normals) const {
  if (length() < 1e-6) {
    return;
  }
  getTangent(normals);
  for (auto& _np : normals) {
    std::swap(_np.first, _np.second);
    _np.second *= -1;
  }
}

void ODGeometry::getNormal(double s, double& x, double& y) const {
  double alpha = getHeading(s) + pi * 0.5;
  x = std::cos(alpha);
  y = std::sin(alpha);
}

void ODGeometry::getTangent(double s, double& x, double& y) const {
  double alpha = getHeading(s);
  x = std::cos(alpha);
  y = std::sin(alpha);
}

void ODGeometry::rotate(double angle, double& x, double& y) const {
  const double cos_a = std::cos(angle);
  const double sin_a = std::sin(angle);
  double xx = x * cos_a - y * sin_a;
  double yy = y * cos_a + x * sin_a;
  x = xx;
  y = yy;
}

// line

double ODParamLine::getCurvature(double soffset) const { return 0.0; }

void ODParamLine::getInterval(std::vector<double>& intervals) const {
  if (length() < 1e-6) {
    return;
  }
  intervals.clear();
  int sp = static_cast<int>(std::ceil(length() / min_len));
  double itv = length() / sp;
  for (int i = 0; i <= sp; i++) {
    intervals.push_back(itv * i);
  }
}

void ODParamLine::getGeom(double s, double& x, double& y) const {
  if (length() < 1e-6) {
    return;
  }
  x = startX() + (s - offset()) * std::cos(heading());
  y = startY() + (s - offset()) * std::sin(heading());
}

double ODParamLine::getHeading(double s) const { return heading(); }

/*ODGeomSpiral*/
bool ODGeomSpiral::parse(tinyxml2::XMLElement* element) {
  ODGeometry::parse(element);
  element = element->FirstChildElement("spiral");
  ELE_CKECK;
  curve_start = element->DoubleAttribute("curvStart");
  curve_end = element->DoubleAttribute("curvEnd");
  return true;
}
void ODGeomSpiral::getInterval(std::vector<double>& intervals) const {
  if (length() < 1e-6) {
    return;
  }
  intervals.clear();

  const double curve_dot = (curve_end - curve_start) / length();
  if (std::abs(curve_dot) < 1e-6) {
    double a = min_angle / (std::abs(curve_end) * pi);
    a = std::min(a, min_len);
    double interval = std::min(1.0, a / length());
    for (double _p = 0.0; _p < 1.001; _p += interval) {
      intervals.push_back(_p * length());
    }
  } else {
    const double s_o = curve_start / curve_dot;
    double dt = std::max(1e-4, std::abs(curve_dot * s_o));
    double s = 0;
    while (s < length()) {
      intervals.push_back(s);
      s += std::min(min_len, std::abs(min_angle / dt));
      dt = std::max(1e-4, std::abs(curve_dot * (s_o + s)));
    }
    intervals.push_back(length());
  }
}
double ODGeomSpiral::getCurvature(double soffset) const { return 0.0; }
void ODGeomSpiral::getGeom(double s, double& x, double& y) const {
  if (length() < 1e-6) {
    return;
  }
  const double dist = s - offset();

  const double curve_dot = (curve_end - curve_start) / length();
  if (std::abs(curve_dot) < 1e-6) {
    s -= offset();
    double r = 1. / std::abs(curve_end);
    double tag = (curve_end > 0) ? 1. : (-1.);
    double hdg = heading() + tag * pi / 2;
    x = r * std::cos(hdg);
    y = r * std::sin(hdg);
    hdg += pi;
    double alpha = hdg + s * curve_end;
    x += r * std::cos(alpha);
    y += r * std::sin(alpha);
    x += startX();
    y += startY();
  } else {
    const double s_o = curve_start / curve_dot;
    s = s_o + dist;

    odrSpiral(s, curve_dot, &x, &y, nullptr);
    double x_o;
    double y_o;
    double t_o;
    odrSpiral(s_o, curve_dot, &x_o, &y_o, &t_o);
    x = x - x_o;
    y = y - y_o;
    rotate(heading() - t_o, x, y);
    x += startX();
    y += startY();
  }
}
double ODGeomSpiral::getHeading(double s) const {
  if (length() < 1e-6) {
    return 0;
  }
  const double dist = s - offset();
  const double curve_dot = (curve_end - curve_start) / length();
  if (std::abs(curve_dot) < 1e-6) {
    return heading() + (s - offset()) * curve_end;
  } else {
    const double s_o = curve_start / curve_dot;
    s = s_o + dist;

    double t = 0;
    double t_o = 0;
    odrSpiral(s, curve_dot, nullptr, nullptr, &t);
    odrSpiral(s_o, curve_dot, nullptr, nullptr, &t_o);
    t = t - t_o;
    return heading() + t;
  }
}

// copy from opendrive
void ODGeomSpiral::odrSpiral(double s, double cDot, double* x, double* y, double* t) const {
  if (x && y) {
    double a = 1.0 / sqrt(fabs(cDot));
    a *= sqrt(pi);
    fresnel(s / a, y, x);
    *x *= a;
    *y *= a;
    if (cDot < 0.0) *y *= -1.0;
  }

  if (t) *t = s * s * cDot * 0.5;
}

void ODGeomSpiral::fresnel(double xxa, double* ssa, double* cca) const {
  /* S(x) for small x */
  static double sn[6] = {
      -2.99181919401019853726E3, 7.08840045257738576863E5,   -6.29741486205862506537E7,
      2.54890880573376359104E9,  -4.42979518059697779103E10, 3.18016297876567817986E11,
  };
  static double sd[6] = {
      /* 1.00000000000000000000E0,*/
      2.81376268889994315696E2, 4.55847810806532581675E4,  5.17343888770096400730E6,
      4.19320245898111231129E8, 2.24411795645340920940E10, 6.07366389490084639049E11,
  };

  /* C(x) for small x */
  static double cn[6] = {
      -4.98843114573573548651E-8, 9.50428062829859605134E-6,  -6.45191435683965050962E-4,
      1.88843319396703850064E-2,  -2.05525900955013891793E-1, 9.99999999999999998822E-1,
  };
  static double cd[7] = {
      3.99982968972495980367E-12, 9.15439215774657478799E-10, 1.25001862479598821474E-7, 1.22262789024179030997E-5,
      8.68029542941784300606E-4,  4.12142090722199792936E-2,  1.00000000000000000118E0,
  };

  /* Auxiliary function f(x) */
  static double fn[10] = {
      4.21543555043677546506E-1,  1.43407919780758885261E-1,  1.15220955073585758835E-2,  3.45017939782574027900E-4,
      4.63613749287867322088E-6,  3.05568983790257605827E-8,  1.02304514164907233465E-10, 1.72010743268161828879E-13,
      1.34283276233062758925E-16, 3.76329711269987889006E-20,
  };
  static double fd[10] = {
      /*  1.00000000000000000000E0,*/
      7.51586398353378947175E-1,  1.16888925859191382142E-1,  6.44051526508858611005E-3,  1.55934409164153020873E-4,
      1.84627567348930545870E-6,  1.12699224763999035261E-8,  3.60140029589371370404E-11, 5.88754533621578410010E-14,
      4.52001434074129701496E-17, 1.25443237090011264384E-20,
  };

  /* Auxiliary function g(x) */
  static double gn[11] = {
      5.04442073643383265887E-1,  1.97102833525523411709E-1,  1.87648584092575249293E-2,  6.84079380915393090172E-4,
      1.15138826111884280931E-5,  9.82852443688422223854E-8,  4.45344415861750144738E-10, 1.08268041139020870318E-12,
      1.37555460633261799868E-15, 8.36354435630677421531E-19, 1.86958710162783235106E-22,
  };
  static double gd[11] = {
      /*  1.00000000000000000000E0,*/
      1.47495759925128324529E0,   3.37748989120019970451E-1,  2.53603741420338795122E-2,  8.14679107184306179049E-4,
      1.27545075667729118702E-5,  1.04314589657571990585E-7,  4.60680728146520428211E-10, 1.10273215066240270757E-12,
      1.38796531259578871258E-15, 8.39158816283118707363E-19, 1.86958710162783236342E-22,
  };

  static double pi = 3.1415926535897932384626433832795;

  double f, g, cc, ss, c, s, t, u;
  double x, x2;

  x = fabs(xxa);
  x2 = x * x;

  if (x2 < 2.5625) {
    t = x2 * x2;
    ss = x * x2 * polevl(t, sn, 5) / p1evl(t, sd, 6);
    cc = x * polevl(t, cn, 5) / polevl(t, cd, 6);
  } else if (x > 36974.0) {
    cc = 0.5;
    ss = 0.5;
  } else {
    x2 = x * x;
    t = pi * x2;
    u = 1.0 / (t * t);
    t = 1.0 / t;
    f = 1.0 - u * polevl(u, fn, 9) / p1evl(u, fd, 10);
    g = t * polevl(u, gn, 10) / p1evl(u, gd, 11);

    t = pi * 0.5 * x2;
    c = cos(t);
    s = sin(t);
    t = pi * x;
    cc = 0.5 + (f * s - g * c) / t;
    ss = 0.5 - (f * c + g * s) / t;
  }

  if (xxa < 0.0) {
    cc = -cc;
    ss = -ss;
  }

  *cca = cc;
  *ssa = ss;
}

double ODGeomSpiral::p1evl(double x, double* coef, int n) const {
  double ans;
  double* p = coef;
  int i;

  ans = x + *p++;
  i = n - 1;

  do {
    ans = ans * x + *p++;
  } while (--i);

  return ans;
}

inline double ODGeomSpiral::polevl(double x, double* coef, int n) const {
  double ans;
  double* p = coef;
  int i;

  ans = *p++;
  i = n;

  do {
    ans = ans * x + *p++;
  } while (--i);

  return ans;
}

/* ODParamArc */
bool ODParamArc::parse(XMLElement* element) {
  ODGeometry::parse(element);

  element = element->FirstChildElement("arc");
  ELE_CKECK;

  curvature = element->DoubleAttribute("curvature");
  return true;
}

double ODParamArc::getCurvature(double soffset) const { return this->curvature; }

void ODParamArc::getInterval(std::vector<double>& intervals) const {
  if (length() < 1e-6) {
    return;
  }
  intervals.clear();
  if (std::abs(curvature) < 1e-6) {
    int sp = static_cast<int>(std::ceil(length() / min_len));
    double itv = length() / sp;
    for (int i = 0; i <= sp; i++) {
      intervals.push_back(itv * i);
    }
  } else {
    double interval = getIntervalStep();
    for (double _p = 0.0; _p < 1.001; _p += interval) {
      intervals.push_back(_p * length());
    }
  }
}

void ODParamArc::getGeom(double s, double& x, double& y) const {
  if (length() < 1e-6) {
    return;
  }
  if (std::abs(curvature) < 1e-6) {
    x = startX() + (s - offset()) * std::cos(heading());
    y = startY() + (s - offset()) * std::sin(heading());
  } else {
    s -= offset();
    double r = 1. / std::abs(curvature);
    double tag = (curvature > 0) ? 1. : (-1.);
    double hdg = heading() + tag * pi / 2;
    x = r * std::cos(hdg);
    y = r * std::sin(hdg);
    hdg += pi;
    double alpha = hdg + s * curvature;
    x += r * std::cos(alpha);
    y += r * std::sin(alpha);
    x += startX();
    y += startY();
  }
}

double ODParamArc::getHeading(double s) const {
  if (length() < 1e-6) {
    return 0;
  }
  return heading() + (s - offset()) * curvature;
}

double ODParamArc::getIntervalStep() const {
  if (length() < 1e-6) {
    return 0;
  }
  double a = min_angle / (std::abs(curvature) * pi);
  a = std::min(a, min_len);
  return std::min(1.0, a / length());
}

/* ODGeomPoly3 */
bool ODGeomPoly3::parse(tinyxml2::XMLElement* element) {
  ODGeometry::parse(element);

  element = element->FirstChildElement("poly3");
  ELE_CKECK;
  a = element->DoubleAttribute("a");
  b = element->DoubleAttribute("b");
  c = element->DoubleAttribute("c");
  d = element->DoubleAttribute("d");
  return true;
}

void ODGeomPoly3::getInterval(std::vector<double>& intervals) const {
  if (length() < 1e-6) {
    return;
  }
  intervals.clear();
  double dt = 1e-4;
  double s = 0;
  while (s < length()) {
    intervals.push_back(s);
    s += std::min(min_len, min_angle / dt);
    dt = std::abs(2 * c + 6 * d * s);
    dt = std::max(std::atan(dt), 1e-4);
  }
  intervals.push_back(length());
}

double ODGeomPoly3::getCurvature(double soffset) const {
  const double SampleDistance = 30.0;
  int i = 0;
  while (SampleDistance * i < this->length()) {
    if (soffset < SampleDistance * i) {
      double length = SampleDistance;
      if (SampleDistance * i > this->length()) {
        length = this->length() - SampleDistance * (i - 1);
      }
      std::vector<std::pair<double, double>> mapPoint;
      for (double n = 0; n < length; n += min_len) {
        double x = 0;
        double y = 0;
        this->getGeom(soffset + this->offset(), x, y);
        mapPoint.push_back(std::make_pair(x, y));
      }
      // this->FitCenterByLeastSquares
    }
    i++;
  }
  return 0.0;
}

void ODGeomPoly3::getGeom(double s, double& x, double& y) const {
  if (length() < 1e-6) {
    return;
  }
  x = s - offset();
  y = a + b * x + c * x * x + d * x * x * x;
  rotate(heading(), x, y);
  x += startX();
  y += startY();
}

double ODGeomPoly3::getHeading(double s) const {
  if (length() < 1e-6) {
    return 0;
  }
  s -= offset();
  double v = a + b * s + c * s * s + d * s * s * s;
  rotate(heading(), s, v);
  return std::atan2(v, s);
}

/* ODParamPoly3 */
bool ODParamPoly3::parse(XMLElement* element) {
  ODGeometry::parse(element);

  element = element->FirstChildElement("paramPoly3");
  ELE_CKECK;

  aU = element->DoubleAttribute("aU");
  bU = element->DoubleAttribute("bU");
  cU = element->DoubleAttribute("cU");
  dU = element->DoubleAttribute("dU");
  aV = element->DoubleAttribute("aV");
  bV = element->DoubleAttribute("bV");
  cV = element->DoubleAttribute("cV");
  dV = element->DoubleAttribute("dV");
  auto szPRange = StrCheckEmpty(element->Attribute("pRange"));
  if (std::string("arcLength") == szPRange) {
    normalized = false;
  } else {
    normalized = true;
  }

  return true;
}

void ODParamPoly3::getInterval(std::vector<double>& intervals) const {
  if (length() < 1e-6) {
    return;
  }
  intervals.clear();
  double dt = 1e-4;
  double s = 0;
  while (s < length()) {
    intervals.push_back(s);
    s += std::min(min_len, min_angle / dt);
    double p = normalized ? (s / length()) : s;
    double du = bU + 2 * cU * p + 3 * dU * p * p;
    double dv = bV + 2 * cV * p + 3 * dV * p * p;
    double duu = std::abs(2 * cU + 6 * dU * p);
    double dvv = std::abs(2 * cV + 6 * dV * p);

    dt = std::abs(du) < 1e-8 ? 1e8 : ((dvv * dvv * du - dv * duu * duu) / (du * du * du));
    dt = std::max(std::atan(std::abs(dt)), 1e-4);
  }
  intervals.push_back(length());
}

double ODParamPoly3::getCurvature(double soffset) const {
  const double SampleDistance = 30.0;
  int i = 0;
  while (SampleDistance * i < this->length()) {
    if (soffset <= SampleDistance * i) {
      double length = SampleDistance;
      if (SampleDistance * i > this->length() - SampleDistance) {
        length = this->length() - soffset;
      }
      std::vector<std::pair<double, double>> mapPoint;
      for (double n = 0; n < length; n += min_len) {
        double x = 0;
        double y = 0;
        this->getGeom(SampleDistance * (i - 1) + this->offset() + n, x, y);
        mapPoint.push_back(std::make_pair(x, y));
      }
      double radius = 0;
      FitCenterByLeastSquares(mapPoint, radius);
      if (radius < 5e-5) {
        radius = 0;
      }
      return radius;
    }
    i++;
  }
  return 0.0;
}

void ODParamPoly3::getGeom(double s, double& x, double& y) const {
  if (length() < 1e-6) {
    return;
  }
  s -= offset();
  double p = normalized ? (s / length()) : s;
  x = aU + bU * p + cU * p * p + dU * p * p * p;
  y = aV + bV * p + cV * p * p + dV * p * p * p;
  rotate(heading(), x, y);
  x += startX();
  y += startY();
}

double ODParamPoly3::getHeading(double s) const {
  if (length() < 1e-6) {
    return 0;
  }
  s -= offset();
  double p = normalized ? (s / length()) : s;
  double u = bU + 2 * cU * p + 3 * dU * p * p;
  double v = bV + 2 * cV * p + 3 * dV * p * p;
  rotate(heading(), u, v);
  return std::atan2(v, u);
}

void ODParamPoly3::FitCenterByLeastSquares(std::vector<std::pair<double, double>> mapPoint, double& radius) const {
  if (mapPoint.size() < 3) {
    radius = 0.0;
    return;
  }
  double radiusVecSum = 0.0;

  for (int i = 0; i < mapPoint.size() - 2; i++) {
    struct point {
      double x;
      double y;
    };
    point P1{mapPoint.at(i).first, mapPoint.at(i).second};
    point P2{mapPoint.at(i + 1).first, mapPoint.at(i + 1).second};
    point P3{mapPoint.at(i + 2).first, mapPoint.at(i + 2).second};

    if (P1.x == P2.x == P3.x) {  // 三点横坐标相同，即共线，直接标记曲率为0
      radius = 0;
    } else {
      double dis1, dis2, dis3;
      double cosA, sinA, dis;
      dis1 = sqrt((P1.x - P2.x) * (P1.x - P2.x) + (P1.y - P2.y) * (P1.y - P2.y));
      dis2 = sqrt((P1.x - P3.x) * (P1.x - P3.x) + (P1.y - P3.y) * (P1.y - P3.y));
      dis3 = sqrt((P2.x - P3.x) * (P2.x - P3.x) + (P2.y - P3.y) * (P2.y - P3.y));
      dis = dis1 * dis1 + dis3 * dis3 - dis2 * dis2;

      if (dis1 == 0 || dis2 == 0 || dis3 == 0 || dis == 0) {
        radius = 0;
      } else {
        cosA = dis / (2 * dis1 * dis3);  // 余弦定理求角度
        if (std::abs(1 - cosA * cosA) < 1e-6 || std::abs(dis2) < 1e-6) {
          radius = 0;
        } else {
          sinA = sqrt(1 - cosA * cosA);  // 求正弦
          radius = 0.5 * dis2 / sinA;    // 正弦定理求外接圆半径
          radius = 1 / radius;           // 半径的倒数是曲率，半径越小曲率越大
        }
      }
    }
    radiusVecSum += radius;
  }
  radius = radiusVecSum / static_cast<double>(mapPoint.size() - 2);

  // double sumX = 0, sumY = 0;

  // double sumXX = 0, sumYY = 0, sumXY = 0;

  // double sumXXX = 0, sumXXY = 0, sumXYY = 0, sumYYY = 0;

  // int pCount = mapPoint.size();

  // for (int i = 0; i< pCount;i++)
  // {

  //  sumX += mapPoint.at(i).first;

  //  sumY += mapPoint.at(i).second;

  //  sumXX += pow(mapPoint.at(i).first, 2);

  //  sumYY += pow(mapPoint.at(i).second, 2);

  //  sumXY += mapPoint.at(i).first * mapPoint.at(i).second;

  //  sumXXX += pow(mapPoint.at(i).first, 3);

  //  sumXXY += pow(mapPoint.at(i).first, 2) * mapPoint.at(i).second;

  //  sumXYY += mapPoint.at(i).first * pow(mapPoint.at(i).second, 2);

  //  sumYYY += pow(mapPoint.at(i).second, 3);

  // }

  // double M1 = pCount * sumXY - sumX * sumY;

  // double M2 = pCount * sumXX - sumX * sumX;

  // double M3 = pCount * (sumXXX + sumXYY) - sumX * (sumXX + sumYY);

  // double M4 = pCount * sumYY - sumY * sumY;

  // double M5 = pCount * (sumYYY + sumXXY) - sumY * (sumXX + sumYY);

  // double a = (M1 * M5 - M3 * M4) / (M2 * M4 - M1 * M1);

  // double b = (M1 * M3 - M2 * M5) / (M2 * M4 - M1 * M1);

  // double c = -(a * sumX + b * sumY + sumXX + sumYY) / pCount;

  // //圆心XY 半径

  // double xCenter = -0.5 * a;

  // double yCenter = -0.5 * b;

  // radius = 0.5 * sqrt(a * a + b * b - 4 * c);
}

bool ODOutline::parse(tinyxml2::XMLElement* element) {
  id = element->Int64Attribute("id");
  closed = element->BoolAttribute("closed");
  tinyxml2::XMLElement* _corner = element->FirstChildElement("cornerRoad");
  while (_corner) {
    ODCornerRoadPtr corner_ptr = std::make_shared<ODCornerRoad>();
    if (corner_ptr->parse(_corner)) {
      corners.push_back(corner_ptr);
    }
    _corner = _corner->NextSiblingElement("cornerRoad");
  }
  _corner = element->FirstChildElement("cornerLocal");
  while (_corner) {
    ODCornerLocalPtr corner_ptr = std::make_shared<ODCornerLocal>();
    if (corner_ptr->parse(_corner)) {
      corners.push_back(corner_ptr);
    }
    _corner = _corner->NextSiblingElement("cornerLocal");
  }

  return true;
}

void ODOutline::getGeom(std::vector<std::pair<bool, ODXYZ>>& points) {
  points.clear();
  for (const auto& corner : corners) {
    points.push_back(std::make_pair(corner->isLocal(), corner->getxyz()));
  }
}

bool ODCornerRoad::parse(tinyxml2::XMLElement* element) {
  id = element->Int64Attribute("id");
  s = element->DoubleAttribute("s");
  t = element->DoubleAttribute("t");
  dz = element->DoubleAttribute("dz");
  height = element->DoubleAttribute("height");
  return true;
}

void ODCornerRoad::setXyz(ODXYZ data) {
  s = data.x;
  t = data.y;
  dz = data.z;
}

ODXYZ ODCornerRoad::getxyz() { return ODXYZ(s, t, dz); }

bool ODCornerLocal::parse(tinyxml2::XMLElement* element) {
  id = element->Int64Attribute("id");
  u = element->DoubleAttribute("u");
  v = element->DoubleAttribute("v");
  z = element->DoubleAttribute("z");
  height = element->DoubleAttribute("height");
  return true;
}

ODXYZ ODCornerLocal::getxyz() { return ODXYZ(u, v, z); }

bool ODParkingSpace::parse(tinyxml2::XMLElement* element) {
  access = StrCheckEmpty(element->Attribute("access"));
  restrictions = StrCheckEmpty(element->Attribute("restrictions"));
  XMLElement* elem = element->FirstChildElement("marking");
  while (elem) {
    Marking marking;
    marking.side = StrCheckEmpty(elem->Attribute("side"));
    marking.type = StrCheckEmpty(elem->Attribute("type"));
    marking.width = elem->DoubleAttribute("width");
    marking.color = StrCheckEmpty(elem->Attribute("color"));
    markings.push_back(marking);
    elem = elem->NextSiblingElement("marking");
  }
  return true;
}

bool ODParkingSpace::parseMarks(tinyxml2::XMLElement* element) { return false; }

bool ODRepeat::parse(tinyxml2::XMLElement* element) {
  s = element->DoubleAttribute("s");
  length = element->DoubleAttribute("length");
  distance = element->DoubleAttribute("distance");
  tStart = element->DoubleAttribute("tStart");
  tEnd = element->DoubleAttribute("tEnd");
  widthStart = element->DoubleAttribute("widthStart");
  widthEnd = element->DoubleAttribute("widthEnd");
  heightStart = element->DoubleAttribute("heightStart");
  heightEnd = element->DoubleAttribute("heightEnd");
  zOffsetStart = element->DoubleAttribute("zOffsetStart");
  zOffsetEnd = element->DoubleAttribute("zOffsetEnd");
  lengthStart = element->DoubleAttribute("lengthStart");
  lengthEnd = element->DoubleAttribute("lengthEnd");
  return true;
}

bool ODObject::parse(tinyxml2::XMLElement* element) {
  type = StrCheckEmpty(element->Attribute("type"));
  subtype = StrCheckEmpty(element->Attribute("subtype"));
  name = StrCheckEmpty(element->Attribute("name"));
  id = StrCheckEmpty(element->Attribute("id"));
  s = element->DoubleAttribute("s");
  t = element->DoubleAttribute("t");
  zoffset = element->DoubleAttribute("zOffset");
  hdg = element->DoubleAttribute("hdg");
  pitch = element->DoubleAttribute("pitch");
  roll = element->DoubleAttribute("roll");

  length = element->DoubleAttribute("length");
  width = element->DoubleAttribute("width");
  height = element->DoubleAttribute("height");
  radius = element->DoubleAttribute("radius");

  tinyxml2::XMLElement* _repeat = element->FirstChildElement("repeat");
  while (_repeat) {
    ODRepeatPtr trepeat = std::make_shared<ODRepeat>();
    if (trepeat->parse(_repeat)) {
      repeats.push_back(trepeat);
    }
    _repeat = _repeat->NextSiblingElement("repeat");
  }

  tinyxml2::XMLElement* _parkspace = element->FirstChildElement("parkingSpace");
  if (_parkspace) {
    ODParkingSpacePtr parkspace = std::make_shared<ODParkingSpace>();
    if (parkspace->parse(_parkspace)) {
      parkingSpace = parkspace;
    }
  }

  // opendrive 1.4
  tinyxml2::XMLElement* _outline = element->FirstChildElement("outline");
  if (_outline) {
    ODOutlinePtr outline_ptr = std::make_shared<ODOutline>();
    if (outline_ptr->parse(_outline)) {
      outlines.push_back(outline_ptr);
    }
  }
  //
  tinyxml2::XMLElement* user = element->FirstChildElement("userData");
  while (user) {
    std::string key = StrCheckEmpty(user->Attribute("code"));
    std::string value = StrCheckEmpty(user->Attribute("value"));
    userData.insert(std::make_pair(key, value));
    user = user->NextSiblingElement("userData");
  }
  if (userData.find("sample_point") != userData.end()) {
    getPointFromUserData(userData["sample_point"], m_points);
  }
  //
  tinyxml2::XMLElement* _markings = element->FirstChildElement("markings");
  while (_markings) {
    this->markings.reset(new ODMarkings);
    markings->parse(_markings);
    _markings = _markings->NextSiblingElement("_markings");
  }

  // opendrive 1.5
  tinyxml2::XMLElement* _outlines = element->FirstChildElement("outlines");
  if (_outlines) {
    tinyxml2::XMLElement* _outline = _outlines->FirstChildElement("outline");
    while (_outline) {
      ODOutlinePtr outline_ptr = std::make_shared<ODOutline>();
      if (outline_ptr->parse(_outline)) {
        outlines.push_back(outline_ptr);
      }
      _outline = _outline->NextSiblingElement("outline");
    }
    _outline = _outlines->NextSiblingElement("outlines");
  }
  XMLElement* validity = element->FirstChildElement("validity");
  while (validity) {
    std::string fid = StrCheckEmpty(validity->Attribute("fromLane"));
    std::string tid = StrCheckEmpty(validity->Attribute("toLane"));
    if (!fid.empty() && !tid.empty()) {
      int64_t from = std::stod(fid.c_str());
      int64_t to = std::stod(tid.c_str());
      if (from > to) {
        std::swap(from, to);
      }
      for (int i = from; i <= to; i++) {
        validityLane.push_back(i);
      }
    }
    validity = validity->NextSiblingElement("validity");
  }
  //
  return true;
}

bool ODObject::parseTunnel(tinyxml2::XMLElement* element) {
  s = element->DoubleAttribute("s");
  t = 0;
  id = StrCheckEmpty(element->Attribute("id"));
  type = StrCheckEmpty(element->Attribute("type"));
  name = StrCheckEmpty(element->Attribute("name"));
  length = element->DoubleAttribute("length");
  tinyxml2::XMLElement* user = element->FirstChildElement("userData");
  while (user) {
    std::string key = StrCheckEmpty(user->Attribute("code"));
    std::string value = StrCheckEmpty(user->Attribute("value"));
    userData.insert(std::make_pair(key, value));
    user = user->NextSiblingElement("userData");
  }
  return true;
}

void ODObject::getGeom(std::vector<std::pair<bool, std::vector<ODXYZ>>>& points,
                       std::function<void(double, double, ODXYZ&, double&)> getP) const {
  if (!outlines.empty()) {
    ODXYZ P;
    double T = 0;
    getP(s, t, P, T);
    for (const auto& outline : outlines) {
      points.push_back(std::make_pair(outline->isClosed(), std::vector<ODXYZ>()));
      std::vector<std::pair<bool, ODXYZ>> pts;
      outline->getGeom(pts);
      points.back().second.reserve(pts.size());
      for (const auto& p : pts) {
        ODXYZ np;
        if (p.first) {
          np = ODRPY(T) * (ODRPY(hdg, roll, pitch) * p.second) + P + ODXYZ(0, 0, zoffset);
        } else {
          double T0 = 0;
          getP(p.second.x, p.second.y, np, T0);
        }
        points.back().second.push_back(np);
      }
    }
  } else {
    if (m_points.size() > 0) {
      points.resize(1);
      for (auto it : m_points) {
        ODXYZ P;
        double T = 0;
        getP(it.x, it.y, P, T);
        points.back().first = false;
        points.back().second.push_back(P);
      }
    } else {
      ODXYZ P;
      double T = 0;
      getP(s, t, P, T);
      P.z += zoffset;
      points.resize(1);
      points.back().first = false;
      points.back().second.push_back(P);
    }
  }
}

void ODObject::getRepeat(std::vector<ODXYZWH>& points,
                         std::function<void(double s, double t, ODXYZ& P, double& T)> getP) const {
  if (!repeats.empty()) {
    for (const auto& repeat : repeats) {
      if (repeat->length <= 0.0001) {
        return;
      }
      double ms = 0.5;
      if (repeat->distance > 0.0001) {
        ms = repeat->distance;
      }
      double s0 = 0;
      while (s0 <= (repeat->length + 0.001)) {
        double s = repeat->s + s0;
        double t = repeat->tStart + s0 * (repeat->tEnd - repeat->tStart) / repeat->length;
        double w = repeat->widthStart + s0 * (repeat->widthEnd - repeat->widthStart) / repeat->length;
        double h = repeat->heightStart + s0 * (repeat->heightEnd - repeat->heightStart) / repeat->length;
        double zoffset = repeat->zOffsetStart + s0 * (repeat->zOffsetEnd - repeat->zOffsetStart) / repeat->length;
        ODXYZ p;
        double T = 0;
        getP(s, t, p, T);
        ODXYZWH px;
        px.x = p.x;
        px.y = p.y;
        px.z = p.z + zoffset;
        px.w = w;
        px.h = h;
        points.push_back(px);
        s0 += ms;
      }
    }
  }
}

void ODObject::getBoundary(std::vector<ODXYZ>& points,
                           std::function<void(double s, double t, ODXYZ& P, double& T)> getP) const {
  points.clear();
  ODXYZ P;
  double T = 0;
  getP(s, t, P, T);
  points.reserve(4);
  std::vector<ODXYZ> pts;
  if (width < 1e-6 && length < 1e-6) {
    pts.push_back(ODXYZ(0, 0));
    if (height > 1e-6) {
      pts.push_back(ODXYZ(0, 0, height));
    }
  } else if (width > 1e-6 && length < 1e-6) {
    pts.push_back(ODXYZ(0, -width * 0.5, 0));
    pts.push_back(ODXYZ(0, width * 0.5, 0));
  } else if (width < 1e-6 && length > 1e-6) {
    pts.push_back(ODXYZ(-length * 0.5, 0));
    pts.push_back(ODXYZ(length * 0.5, 0));
  } else {
    pts.push_back(ODXYZ(-length * 0.5, -width * 0.5));
    pts.push_back(ODXYZ(length * 0.5, -width * 0.5));
    pts.push_back(ODXYZ(length * 0.5, width * 0.5));
    pts.push_back(ODXYZ(-length * 0.5, width * 0.5));
  }

  for (const auto& p : pts) {
    ODXYZ np = ODRPY(T) * (ODRPY(hdg, roll, pitch) * p) + P + ODXYZ(0, 0, zoffset);
    points.push_back(np);
  }
}

void ODObject::getRepeateBoundarys(std::vector<std::vector<ODXYZ>>& points,
                                   std::function<void(double s, double t, ODXYZ& P, double& T)> getP) {
  std::vector<ODXYZWHP> _tmpPoints;
  if (!repeats.empty()) {
    for (const auto& repeat : repeats) {
      if (repeat->length <= 0.0001) {
        return;
      }
      double ms = 0.5;
      if (repeat->distance > 0.0001) {
        ms = repeat->distance;
      }
      double s0 = 0;
      while (s0 <= (repeat->length + 0.001)) {
        double s = repeat->s + s0;
        double t = repeat->tStart + s0 * (repeat->tEnd - repeat->tStart) / repeat->length;
        double w = repeat->widthStart + s0 * (repeat->widthEnd - repeat->widthStart) / repeat->length;
        double h = repeat->heightStart + s0 * (repeat->heightEnd - repeat->heightStart) / repeat->length;
        double zoffset = repeat->zOffsetStart + s0 * (repeat->zOffsetEnd - repeat->zOffsetStart) / repeat->length;
        ODXYZ p;
        double T = 0;
        getP(s, t, p, T);
        ODXYZWHP px;
        px.x = p.x;
        px.y = p.y;
        px.z = p.z + zoffset;
        px.w = w;
        px.h = h;
        px.p = T;
        px.l = (repeat->distance);
        _tmpPoints.push_back(px);
        s0 += ms;
      }
    }
  }
  for (auto it : _tmpPoints) {
    std::vector<ODXYZ> outPutPoints;
    outPutPoints.clear();
    ODXYZ P = it;
    double T = it.p;
    outPutPoints.reserve(4);
    std::vector<ODXYZ> pts;
    if (width < 1e-6 && length < 1e-6) {
      pts.push_back(ODXYZ(0, 0));
      if (height > 1e-6) {
        pts.push_back(ODXYZ(0, 0, height));
      }
    } else if (width > 1e-6 && length < 1e-6) {
      pts.push_back(ODXYZ(0, -width * 0.5, 0));
      pts.push_back(ODXYZ(0, width * 0.5, 0));
    } else if (width < 1e-6 && length > 1e-6) {
      pts.push_back(ODXYZ(-length * 0.5, 0));
      pts.push_back(ODXYZ(length * 0.5, 0));
    } else {
      pts.push_back(ODXYZ(-it.l * 0.5, -width * 0.5));
      pts.push_back(ODXYZ(it.l * 0.5, -width * 0.5));
      pts.push_back(ODXYZ(it.l * 0.5, width * 0.5));
      pts.push_back(ODXYZ(-it.l * 0.5, width * 0.5));
    }
    for (const auto& p : pts) {
      ODXYZ np = ODRPY(T) * (ODRPY(hdg, roll, pitch) * p) + P + ODXYZ(0, 0, zoffset);
      outPutPoints.push_back(np);
    }
    points.push_back(outPutPoints);
  }
}
void ODObject::getRepeateFromOutlines() {
  ODOutlinePtr _outLine = this->outlines.front();
  std::vector<std::pair<bool, ODXYZ>> points;
  _outLine->getGeom(points);
  double center_s = 0;
  double center_t = 0;
  for (auto it : points) {
    center_s += it.second.x;
    center_t += it.second.y;
  }
  center_s /= points.size();
  center_t /= points.size();

  int index = 1;
  if (!repeats.empty()) {
    for (const auto& repeat : repeats) {
      if (repeat->length <= 0.0001) {
        return;
      }
      double ms = 0.5;
      if (repeat->distance > 0.0001) {
        ms = repeat->distance;
      }
      while (repeat->distance * index <= repeat->length) {
        ODOutlinePtr tmpOutLine = std::make_shared<ODOutline>();
        for (auto it : points) {
          ODCornerRoadPtr corner_ptr = std::make_shared<ODCornerRoad>();
          ODXYZ tmp = it.second;
          tmp.x = tmp.x + repeat->distance * index;
          corner_ptr->setXyz(tmp);
          tmpOutLine->insertCorner(corner_ptr);
        }
        outlines.push_back(tmpOutLine);
        index++;
      }
    }
  }
}
void ODObject::getPointFromUserData(std::string str, std::vector<ODXYZ>& points) {
  std::string tmp = str;
  std::vector<std::string> _point_string;
  while (tmp.find("[") != tmp.npos) {
    tmp.erase(tmp.find("["), 1);
  }
  while (tmp.find("]") != tmp.npos) {
    tmp.erase(tmp.find("]"), 1);
  }
  while (tmp.find("#") != tmp.npos) {
    tmp.erase(tmp.find("#"), 1);
  }
  std::string::size_type pos;
  std::vector<std::string> result;
  tmp += ",";
  int size = tmp.size();
  for (int i = 0; i < size; i++) {
    pos = tmp.find("},", i);
    if (pos < size) {
      std::string s = tmp.substr(i, pos - i + 1);
      if (!s.empty()) result.push_back(s);
      i = pos + 1;
    }
  }
  for (auto its : result) {
    while (its.find("{") != its.npos) {
      its = its.erase(its.find("{"), 1);
    }
    while (its.find("}") != its.npos) {
      its = its.erase(its.find("}"), 1);
    }
    its += ",";
    std::string::size_type pos;
    std::vector<std::string> _tmp_result;
    int size = its.size();
    for (int i = 0; i < size; i++) {
      pos = its.find(",", i);
      if (pos < size) {
        std::string _s = its.substr(i, pos - i);
        if (!_s.empty()) _tmp_result.push_back(_s);
        i = pos;
      }
    }
    std::map<std::string, std::string> repeatdataVec;
    for (auto it : _tmp_result) {
      int pos = it.find(":");
      std::string key = it.substr(0, pos);
      std::string value = it.substr(pos + 1, it.size() - pos - 1);
      repeatdataVec[key] = value;
    }
    double _d_s = std::atof(repeatdataVec["s"].c_str());
    double _d_t = std::atof(repeatdataVec["t"].c_str());
    ODXYZ _pt;
    _pt.x = _d_s;
    _pt.y = _d_t;
    _pt.z = 0;
    points.push_back(_pt);
  }
}

bool ODSignal::parse(tinyxml2::XMLElement* element) {
  type = StrCheckEmpty(element->Attribute("type"));
  subtype = StrCheckEmpty(element->Attribute("subtype"));
  name = StrCheckEmpty(element->Attribute("name"));
  id = StrCheckEmpty(element->Attribute("id"));
  dynamic = StrCheckEmpty(element->Attribute("dynamic"));
  country = StrCheckEmpty(element->Attribute("country"));
  text = StrCheckEmpty(element->Attribute("text"));
  s = element->DoubleAttribute("s");
  t = element->DoubleAttribute("t");
  zoffset = element->DoubleAttribute("zOffset");
  hOffset = element->DoubleAttribute("hOffset");
  pitch = element->DoubleAttribute("pitch");
  roll = element->DoubleAttribute("roll");
  hdg = element->DoubleAttribute("hdg");
  value = element->DoubleAttribute("value");
  width = element->DoubleAttribute("width");
  height = element->DoubleAttribute("height");

  tinyxml2::XMLElement* user = element->FirstChildElement("userData");
  while (user) {
    std::string key = StrCheckEmpty(user->Attribute("code"));
    std::string value = StrCheckEmpty(user->Attribute("value"));
    userData.insert(std::make_pair(key, value));
    user = user->NextSiblingElement("userData");
  }
  XMLElement* validity = element->FirstChildElement("validity");
  while (validity) {
    std::string fid = StrCheckEmpty(validity->Attribute("fromLane"));
    std::string tid = StrCheckEmpty(validity->Attribute("toLane"));
    if (!fid.empty() && !tid.empty()) {
      int from = std::atoi(fid.c_str());
      int to = std::atoi(tid.c_str());
      if (from > to) {
        std::swap(from, to);
      }
      for (int i = from; i <= to; i++) {
        validityLane.push_back(i);
      }
    }
    validity = validity->NextSiblingElement("validity");
  }

  return true;
}

void ODSignal::getGeom(std::vector<std::pair<bool, std::vector<ODXYZ>>>& points,
                       std::function<void(double, double, ODXYZ&, double&)> getP) const {
  ODXYZ P;
  double T = 0;
  getP(s, t, P, T);
  P.z += zoffset;
  points.resize(1);
  points.back().first = false;
  points.back().second.push_back(P);
}

void ODSignal::getBoundary(std::vector<ODXYZ>& points,
                           std::function<void(double s, double t, ODXYZ& P, double& T)> getP) const {
  points.clear();
  ODXYZ P;
  double T = 0;
  getP(s, t, P, T);
  points.reserve(4);
  std::vector<ODXYZ> pts;
  if (width > 1e-6 && height < 1e-6) {
    pts.push_back(ODXYZ(0, -width * 0.5, 0));
    pts.push_back(ODXYZ(0, width * 0.5, 0));
  } else if (width < 1e-6 && height > 1e-6) {
    pts.push_back(ODXYZ(0, 0, -height * 0.5));
    pts.push_back(ODXYZ(0, 0, height * 0.5));
  } else if (width > 1e-6 && height > 1e-6) {
    pts.push_back(ODXYZ(0, -width * 0.5, -height * 0.5));
    pts.push_back(ODXYZ(0, width * 0.5, -height * 0.5));
    pts.push_back(ODXYZ(0, width * 0.5, height * 0.5));
    pts.push_back(ODXYZ(0, -width * 0.5, height * 0.5));
  }

  for (const auto& p : pts) {
    ODXYZ np = ODRPY(T) * (ODRPY(hOffset, roll, pitch) * p) + P + ODXYZ(0, 0, zoffset);
    points.push_back(np);
  }
}

/* ODRoad */
bool ODRoad::parse(XMLElement* element) {
  ELE_CKECK;

  // parse road id
  rid = element->Int64Attribute("id");

  // parse length
  len = element->DoubleAttribute("length");

  // parse junction id
  jid = element->Int64Attribute("junction") == -1 ? 0 : element->Int64Attribute("junction");

  // parse speed limit
  speed_limit = 0;
  if (element->FirstChildElement("type") != NULL) {
    if (element->FirstChildElement("type")->Attribute("type")) {
      road_type = element->FirstChildElement("type")->Attribute("type");
    }
    if (element->FirstChildElement("type")->FirstChildElement("speed") != NULL) {
      double speed_max = element->FirstChildElement("type")->FirstChildElement("speed")->DoubleAttribute("max");
      auto unit = StrCheckEmpty(element->FirstChildElement("type")->FirstChildElement("speed")->Attribute("unit"));
      if (unit == "km/h") {
        speed_max *= 0.277778;
      } else if (unit == "mph") {
        speed_max *= 0.44704;
      }

      speed_limit = std::max(0, static_cast<int>(std::round(speed_max * 3.6)));
    }
  }

  // parse link info
  XMLElement* _link = element->FirstChildElement("link");
  if (_link != NULL) {
    link_ptr.reset(new ODLink);
    link_ptr->parse(_link);
  } else {
    // std::cout << "rid" << rid<<std::endl;
  }
  // parse ref line info
  XMLElement* _plan_view = element->FirstChildElement("planView");
  if (_plan_view) {
    ref_line.clear();
    XMLElement* _geometry = _plan_view->FirstChildElement("geometry");
    double _sum_length = 0;
    while (_geometry) {
      ODGeomPtr _geom_ptr;
      if (_geometry->FirstChildElement("line")) {
        _geom_ptr.reset(new ODParamLine);
      } else if (_geometry->FirstChildElement("arc")) {
        _geom_ptr.reset(new ODParamArc);
      } else if (_geometry->FirstChildElement("spiral")) {
        _geom_ptr.reset(new ODGeomSpiral);
      } else if (_geometry->FirstChildElement("poly3")) {
        _geom_ptr.reset(new ODGeomPoly3);
      } else if (_geometry->FirstChildElement("paramPoly3")) {
        _geom_ptr.reset(new ODParamPoly3);
      } else {
        return false;
      }
      _geom_ptr->parse(_geometry);
      // ref_line.push_back(_geom_ptr);
      double length = _geom_ptr->length();
      if (length < 0.001) {
        _sum_length += length;
      } else {
        _geom_ptr->setLength(_geom_ptr->length() + _sum_length);
        _sum_length = 0;
        ref_line.push_back(_geom_ptr);
      }
      _geometry = _geometry->NextSiblingElement("geometry");
    }
  }

  // parse elevation
  XMLElement* _elevation_profile = element->FirstChildElement("elevationProfile");
  if (_elevation_profile) {
    elevation.clear();
    XMLElement* _elevation = _elevation_profile->FirstChildElement("elevation");
    while (_elevation) {
      Polynomial3D ele("elevation");
      ele.parse(_elevation);
      elevation.push_back(ele);
      _elevation = _elevation->NextSiblingElement("elevation");
    }
  }

  // parse lanes
  XMLElement* _lanes = element->FirstChildElement("lanes");
  if (_lanes) {
    XMLElement* _lane_offset = _lanes->FirstChildElement("laneOffset");
    lane_offset.clear();
    while (_lane_offset) {
      Polynomial3D poly3d("laneOffset");
      poly3d.parse(_lane_offset);
      // if (poly3d.offset() >= len)
      //  break;
      lane_offset.push_back(poly3d);
      _lane_offset = _lane_offset->NextSiblingElement("laneOffset");
    }

    sections.clear();
    XMLElement* _section = _lanes->FirstChildElement("laneSection");
    int sid = 0;
    while (_section) {
      // parse section offset
      double _s = _section->DoubleAttribute("s");
      if (_s >= len) {
        break;
      }
      // parse lane info in cur section
      ODLaneSection _cur_lanes;
      XMLElement* _left_group = _section->FirstChildElement("left");
      if (_left_group != NULL) {
        XMLElement* _lane = _left_group->FirstChildElement("lane");

        while (_lane) {
          ODLanePtr _cur_lane_ptr(new ODLane);
          if (_cur_lane_ptr->parse(_lane)) _cur_lanes.push_back(_cur_lane_ptr);
          _lane = _lane->NextSiblingElement("lane");
        }
      }
      XMLElement* _cen_group = _section->FirstChildElement("center");
      if (_cen_group != NULL) {
        XMLElement* _lane = _cen_group->FirstChildElement("lane");
        if (_lane) {
          ODLanePtr _cur_lane_ptr(new ODLane);
          _cur_lane_ptr->parse(_lane);
          _cur_lanes.push_back(_cur_lane_ptr);
        } else {
          ODLanePtr _cur_lane_ptr(new ODLane);
          _cur_lanes.push_back(_cur_lane_ptr);
        }

      } else {
        ODLanePtr _cur_lane_ptr(new ODLane);
        _cur_lanes.push_back(_cur_lane_ptr);
      }
      XMLElement* _right_group = _section->FirstChildElement("right");
      if (_right_group != NULL) {
        XMLElement* _lane = _right_group->FirstChildElement("lane");

        while (_lane) {
          ODLanePtr _cur_lane_ptr(new ODLane);
          if (_cur_lane_ptr->parse(_lane)) _cur_lanes.push_back(_cur_lane_ptr);
          _lane = _lane->NextSiblingElement("lane");
        }
      }
      /*else
  {
  std::cout <<"ERROR can't find right lanes: roadid="<<rid<<" section="<< sid <<std::endl;
  }*/

      sort(_cur_lanes.begin(), _cur_lanes.end(),
           [](const ODLanePtr& a, const ODLanePtr& b) { return a->id() > b->id(); });
      for (int ix = _cur_lanes.size() - 1; ix > 0; ix--) {
        if (_cur_lanes.at(ix)->id() == _cur_lanes.at(ix - 1)->id()) {
          _cur_lanes.erase(_cur_lanes.begin() + ix);
        }
      }

      sections.push_back(std::make_pair(_s, _cur_lanes));

      _section = _section->NextSiblingElement("laneSection");
      sid++;
    }
  }

  // parse objects
  XMLElement* _objects = element->FirstChildElement("objects");
  if (_objects) {
    objects.clear();
    XMLElement* _object = _objects->FirstChildElement("object");
    while (_object) {
      ODObjectPtr object_ptr = std::make_shared<ODObject>();
      if (object_ptr->parse(_object)) objects.push_back(object_ptr);
      _object = _object->NextSiblingElement("object");
    }
    XMLElement* _tunnel = _objects->FirstChildElement("tunnel");
    while (_tunnel) {
      ODObjectPtr object_ptr = std::make_shared<ODObject>();
      if (object_ptr->parseTunnel(_tunnel)) objects.push_back(object_ptr);
      _tunnel = _tunnel->NextSiblingElement("object");
    }
  }

  // parse signals
  XMLElement* _signals = element->FirstChildElement("signals");
  if (_signals) {
    signals.clear();
    XMLElement* _signal = _signals->FirstChildElement("signal");
    while (_signal) {
      ODSignalPtr signal_ptr = std::make_shared<ODSignal>();
      if (signal_ptr->parse(_signal)) signals.push_back(signal_ptr);
      _signal = _signal->NextSiblingElement("signal");
    }

    signalReferences.clear();
    XMLElement* reference = _signals->FirstChildElement("signalReference");
    while (reference) {
      SignalRef sref;
      sref.id = StrCheckEmpty(reference->Attribute("id"));
      std::string s = StrCheckEmpty(reference->Attribute("s"));
      std::string t = StrCheckEmpty(reference->Attribute("t"));
      if (!s.empty()) sref.s = std::atof(s.c_str());
      if (!t.empty()) sref.t = std::atof(t.c_str());
      XMLElement* validity = reference->FirstChildElement("validity");
      while (validity) {
        std::string fid = StrCheckEmpty(validity->Attribute("fromLane"));
        std::string tid = StrCheckEmpty(validity->Attribute("toLane"));
        if (!fid.empty() && !tid.empty()) {
          int from = std::atoi(fid.c_str());
          int to = std::atoi(tid.c_str());
          if (from > to) {
            std::swap(from, to);
          }
          for (int i = from; i <= to; i++) {
            sref.laneids.push_back(i);
          }
        }
        validity = validity->NextSiblingElement("validity");
      }
      signalReferences[sref.id] = sref;
      reference = reference->NextSiblingElement("signalReference");
    }
  }

  // parse surface
  XMLElement* _surface = element->FirstChildElement("surface");
  if (_surface) {
    crgs.clear();
    XMLElement* CRG = _surface->FirstChildElement("CRG");
    while (CRG) {
      ODCrg _crgNode;
      _crgNode.parse(CRG);
      crgs.push_back(_crgNode);
      CRG = CRG->NextSiblingElement("CRG");
    }
  }
  return true;
}

bool ODRoad::parse(tinyxml2::XMLElement* element, double _west, double _south, std::vector<std::vector<txPoint>> area) {
  // parse length
  len = element->DoubleAttribute("length");
  int id = element->Int64Attribute("id");
  // getStartPoint
  Point2d startPoint;
  tinyxml2::XMLElement* geometry = element->FirstChildElement("planView")->FirstChildElement("geometry");
  while (geometry) {
    if (geometry->Int64Attribute("s") == 0) {
      startPoint.x = geometry->DoubleAttribute("x");
      startPoint.y = geometry->DoubleAttribute("y");
      for (auto iter : area) {
        // txPoint xx
        int ret = map_util::pointToRectDistance(startPoint, Point2d(iter.at(0).x - _west, iter.at(0).y - _south),
                                                Point2d(iter.at(2).x - _west, iter.at(2).y - _south));
        if (ret == 0) {
          return parse(element);
        } else if (ret < len + MAX_LENGTH_ERROR) {
          return parse(element);
        }
      }
    }
    geometry = geometry->NextSiblingElement("geometry");
  }
  return false;
}
roadpkid ODRoad::id() const { return rid; }

junctionpkid ODRoad::junctionId() const { return jid; }

size_t ODRoad::getSectionIdx(double s) {
  size_t ii = 0;
  for (size_t i = 0; i < sections.size(); i++) {
    if (sections[i].first > s) {
      break;
    }
    ii = i;
  }
  return ii;
}

std::vector<ODCrg> ODRoad::getCrgs() const { return this->crgs; }

double ODRoad::length() const { return len; }

int ODRoad::speedlimit() const { return speed_limit; }

void ODRoad::roadGeom(std::vector<std::pair<double, double>>& geom, const std::vector<double>* interval) const {
  geom.clear();
  if (interval && !interval->empty()) {
    geom.resize(interval->size());
    size_t index = 0;
    for (size_t i = 0; i < interval->size();) {
      if (index + 1 < ref_line.size() && interval->at(i) > ref_line[index + 1]->offset()) {
        index++;
      } else {
        double x = 0, y = 0;
        ref_line[index]->getGeom(interval->at(i), x, y);
        assert(!std::isnan(x));
        assert(!std::isnan(y));
        geom[i] = std::make_pair(x, y);
        i++;
      }
    }
  } else {
    for (auto& _geom_ptr : ref_line) {
      double _x = _geom_ptr->startX();
      double _y = _geom_ptr->startY();
      std::vector<std::pair<double, double>> _cur_geom;
      _geom_ptr->getGeom(_cur_geom);
      if (_cur_geom.empty()) {
        continue;
      }
      for (auto& _point : _cur_geom) _point.first += _x, _point.second += _y;
      if (geom.empty()) {
        geom.assign(_cur_geom.begin(), _cur_geom.end());
      } else {
        geom.insert(geom.end(), _cur_geom.begin() + 1, _cur_geom.end());
      }
    }
  }
}

void ODRoad::roadTangent(std::vector<std::pair<double, double>>& tangent, const std::vector<double>* interval) const {
  tangent.clear();
  if (interval && !interval->empty()) {
    tangent.resize(interval->size());
    size_t index = 0;
    for (size_t i = 0; i < interval->size();) {
      if (index + 1 < ref_line.size() && interval->at(i) > ref_line[index + 1]->offset()) {
        index++;
      } else {
        double x = 0, y = 0;
        ref_line[index]->getTangent(interval->at(i), x, y);
        tangent[i] = std::make_pair(x, y);
        i++;
      }
    }
  } else {
    for (auto& _geom_ptr : ref_line) {
      std::vector<std::pair<double, double>> _cur_tangent;
      _geom_ptr->getTangent(_cur_tangent);
      if (_cur_tangent.empty()) {
        continue;
      }
      if (tangent.empty()) {
        tangent.assign(_cur_tangent.begin(), _cur_tangent.end());
      } else {
        tangent.insert(tangent.end(), _cur_tangent.begin() + 1, _cur_tangent.end());
      }
    }
  }
}

void ODRoad::roadNormal(std::vector<std::pair<double, double>>& normal, const std::vector<double>* interval) const {
  normal.clear();
  if (interval && !interval->empty()) {
    normal.resize(interval->size());
    size_t index = 0;
    for (size_t i = 0; i < interval->size();) {
      if (index + 1 < ref_line.size() && interval->at(i) >= ref_line[index + 1]->offset()) {
        index++;
      } else {
        double x = 0, y = 0;
        ref_line[index]->getNormal(interval->at(i), x, y);
        normal[i] = std::make_pair(x, y);
        i++;
      }
    }
  } else {
    for (auto& _geom_ptr : ref_line) {
      std::vector<std::pair<double, double>> _cur_normal;
      _geom_ptr->getNormal(_cur_normal);
      if (_cur_normal.empty()) {
        continue;
      }
      if (normal.empty()) {
        normal.assign(_cur_normal.begin(), _cur_normal.end());
      } else {
        normal.insert(normal.end(), _cur_normal.begin() + 1, _cur_normal.end());
      }
    }
  }
}

void ODRoad::roadInterval(std::vector<double>& interval) const {
  interval.clear();

  for (auto& _geom_ptr : ref_line) {
    std::vector<double> _cur_interval;
    _geom_ptr->getInterval(_cur_interval);
    if (_cur_interval.empty()) {
      continue;
    }
    for (auto& v : _cur_interval) v += _geom_ptr->offset();

    if (interval.empty()) {
      interval.insert(interval.end(), _cur_interval.begin(), _cur_interval.end());
    } else {
      interval.insert(interval.end(), _cur_interval.begin() + 1, _cur_interval.end());
    }
  }
}

void ODRoad::roadCurvature(std::vector<double>& startx, std::vector<double>& length, std::vector<double>& radios) {
  startx.clear();
  length.clear();
  radios.clear();
  for (ODGeomPtr& _geom_ptr : ref_line) {
    if (_geom_ptr->getName() == "line") {
      startx.push_back(_geom_ptr->offset());
      length.push_back(_geom_ptr->length());
      radios.push_back(_geom_ptr->getCurvature(0.0));
    } else if (_geom_ptr->getName() == "arc") {
      startx.push_back(_geom_ptr->offset());
      length.push_back(_geom_ptr->length());
      radios.push_back(_geom_ptr->getCurvature(0.0));
    } else if (_geom_ptr->getName() == "spiral") {
    } else if (_geom_ptr->getName() == "paramPoly3") {
      const double SampleDistance = 30.0;
      int i = 0;
      double last_radios = 0.0;
      double startx_tmp = 0.0;
      double length_tmp = 0.0;
      while (SampleDistance * i < _geom_ptr->length()) {
        double radius = _geom_ptr->getCurvature(SampleDistance * i);
        startx_tmp = SampleDistance * i;
        length_tmp = SampleDistance;
        if (SampleDistance * i > _geom_ptr->length() - SampleDistance) {
          length_tmp = _geom_ptr->length() - SampleDistance * (i);
        }
        startx.push_back(_geom_ptr->offset() + startx_tmp);
        length.push_back(length_tmp);
        radios.push_back(radius);

        i++;
      }
    }
  }
}

void ODRoad::roadRef(std::vector<std::pair<double, double>>& ref, const std::vector<double>* intervals) const {
  ref.clear();

  if (lane_offset.empty()) {
    roadGeom(ref, intervals);
    return;
  }
  std::vector<double> interval;
  if (intervals && !intervals->empty()) {
    interval = *intervals;
  } else {
    roadInterval(interval);
  }

  std::vector<double> offset;
  size_t index = 0;
  for (size_t i = 0; i < interval.size();) {
    if (index + 1 < lane_offset.size() && interval[i] > lane_offset[index + 1].offset()) {
      index++;
    } else {
      double dis = interval[i] - lane_offset[index].offset();
      offset.push_back(lane_offset[index].value(dis));
      i++;
    }
  }

  std::vector<std::pair<double, double>> geom, normal;
  roadGeom(geom, &interval);
  roadNormal(normal, &interval);

  ref.resize(geom.size());
  for (size_t i = 0; i < geom.size(); ++i) {
    ref[i].first = geom[i].first + offset[i] * normal[i].first;
    ref[i].second = geom[i].second + offset[i] * normal[i].second;
  }
}

void ODRoad::roadEle(std::vector<double>& ele, const std::vector<double>* intervals) const {
  ele.clear();

  if (elevation.empty()) {
    return;
  } else {
    std::vector<double> interval;
    if (intervals && !intervals->empty()) {
      interval = *intervals;
    } else {
      roadInterval(interval);
    }

    size_t index = 0;
    for (size_t i = 0; i < interval.size();) {
      if (index + 1 < elevation.size() && interval[i] > elevation[index + 1].offset()) {
        index++;
      } else {
        double dis = interval[i] - elevation[index].offset();
        ele.push_back(elevation[index].value(dis));
        i++;
      }
    }
  }
}

void ODRoad::roadEleControlPoint(std::vector<std::vector<double>>& ele, double length, std::string mapversion) {
  ele.clear();
  double _a = 0;
  double _b = 0;
  double _c = 0;
  double _d = 0;
  double _s = 0;
  int index = 0;
  for (auto it : elevation) {
    it.getParam(_a, _b, _c, _d, _s);
    double elelength = 0;
    if (index == elevation.size() - 1) {
      elelength = length - it.offset();
    } else {
      elelength = elevation.at(index + 1).offset() - _s;
    }
    if ((std::abs(_c) < 1e-6 && std::abs(_d) < 1e-6) || mapversion == "tadsim v2.0") {
      std::vector<double> tmp;
      tmp.push_back(_s);
      tmp.push_back(it.value(0));
      tmp.push_back(it.slope(0));
      ele.push_back(tmp);
    } else {
      for (int i = 0; i < 4; i++) {
        double percent = static_cast<double>(i) / 5.0 * elelength;
        std::vector<double> tmp;
        tmp.push_back(percent + _s);
        tmp.push_back(it.value(percent));
        tmp.push_back(it.slope(percent));
        ele.push_back(tmp);
      }
    }
    if (index == elevation.size() - 1) {
      std::vector<double> tmp;
      tmp.push_back(length);
      tmp.push_back(it.value(elelength));
      tmp.push_back(it.slope(elelength));
      ele.push_back(tmp);
    }
    index++;
  }
}

void ODRoad::roadSlope(std::vector<double>& startx, std::vector<double>& length, std::vector<double>& slope) {
  for (size_t i = 0; i < elevation.size(); i++) {
    Polynomial3D& eleItr = elevation.at(i);
    double _a, _b, _c, _d, _s;
    eleItr.getParam(_a, _b, _c, _d, _s);
    double _length = 0;
    if (i != elevation.size() - 1) {
      double _aa, _bb, _cc, _dd, _ss;
      elevation.at(i + 1).getParam(_aa, _bb, _cc, _dd, _ss);
      _length = _ss - _s;
    } else {
      _length = this->length() - _s;
    }
    if (std::abs(_c) < 1e-6 && std::abs(_d) < 1e-6) {
      startx.push_back(_s);
      length.push_back(_length);
      slope.push_back(_b);
      continue;
    } else {
      const double SampleDistance = 30.0;
      int i = 0;
      double startx_tmp = 0.0;
      double length_tmp = 0.0;
      while (SampleDistance * i < _length) {
        std::vector<double> dis;
        startx_tmp = SampleDistance * i;
        length_tmp = SampleDistance;
        if (SampleDistance * i > _length - SampleDistance) {
          length_tmp = _length - SampleDistance * i;
        }
        dis.push_back(startx_tmp);
        dis.push_back(startx_tmp + length_tmp);

        std::vector<double> value;
        eleItr.sampleValue(dis, value);
        double slope_tmp = (value[1] - value[0]) / length_tmp;
        startx.push_back(_s + startx_tmp);
        length.push_back(length_tmp);
        slope.push_back(slope_tmp);
        i++;
      }
    }
  }
}

void ODRoad::roadControlPoint(std::string& controltype, std::vector<std::vector<double>>& points) {
  for (ODGeomPtr& _geom_ptr : ref_line) {
    if (_geom_ptr->getName() == "line") {
      std::vector<double> pt1;
      pt1.push_back(_geom_ptr->startX());
      pt1.push_back(_geom_ptr->startY());
      pt1.push_back(_geom_ptr->getHeading(0));
      points.push_back(pt1);
      std::vector<double> pt2;
      double x;
      double y;
      _geom_ptr->getGeom(_geom_ptr->length(), x, y);
      pt2.push_back(x);
      pt2.push_back(y);
      pt2.push_back(_geom_ptr->getHeading(_geom_ptr->length()));
      points.push_back(pt2);
      controltype = "line";
    } else if (_geom_ptr->getName() == "arc") {
      std::vector<double> pt1;
      pt1.push_back(_geom_ptr->startX());
      pt1.push_back(_geom_ptr->startY());
      pt1.push_back(_geom_ptr->getHeading(0));
      points.push_back(pt1);
      std::vector<double> pt2;
      double x;
      double y;
      double s = _geom_ptr->length() + _geom_ptr->offset();
      _geom_ptr->getGeom(s, x, y);
      pt2.push_back(x);
      pt2.push_back(y);
      pt2.push_back(_geom_ptr->getHeading(s));
      points.push_back(pt2);
      controltype = "arc";
    } else if (_geom_ptr->getName() == "paramPoly3") {
      if (points.size() == 0) {
        std::vector<double> pt1;
        pt1.push_back(_geom_ptr->startX());
        pt1.push_back(_geom_ptr->startY());
        pt1.push_back(_geom_ptr->getHeading(0));
        points.push_back(pt1);
      }
      std::vector<double> pt2;
      double x;
      double y;
      double s = _geom_ptr->length() + _geom_ptr->offset();
      _geom_ptr->getGeom(s, x, y);
      pt2.push_back(x);
      pt2.push_back(y);
      pt2.push_back(_geom_ptr->getHeading(s));
      points.push_back(pt2);
      controltype = "catmullrom";
    } else {
      controltype = "none";
    }
  }
  if (controltype == "catmullrom") {
    //
    std::vector<double> pt1;
    pt1.push_back(2 * points.at(0).at(0) - points.at(1).at(0));
    pt1.push_back(2 * points.at(0).at(1) - points.at(1).at(1));
    pt1.push_back(points.at(0).at(2));
    points.insert(points.begin(), pt1);
    //
    std::vector<double> pt2;
    pt2.push_back(2 * points.at(points.size() - 1).at(0) - points.at(points.size() - 2).at(0));
    pt2.push_back(2 * points.at(points.size() - 1).at(1) - points.at(points.size() - 2).at(1));
    pt2.push_back(points.at(points.size() - 1).at(2));
    points.push_back(pt2);
  }
}

void ODRoad::predecessor(std::vector<std::pair<lanepkid, lanepkid>>& links) const {
  if (link_ptr == NULL) return;
  if (sections.empty()) return;
  auto& _front_sec = sections.front().second;
  roadpkid _from_rid = link_ptr->fromRoadId();

  if (_from_rid == ROAD_PKID_INVALID) return;

  links.clear();
  for (auto& _lane_ptr : _front_sec) {
    if (_lane_ptr->fromLId() == LANE_PKID_INVALID) {
      continue;
    } else {
      links.push_back(std::make_pair(_lane_ptr->fromLId(), _lane_ptr->id()));
    }
  }
}

void ODRoad::getT(double x, double y, double s, double& t) {
  double ref_x = 0;
  double ref_y = 0;
  roadGeom(s, ref_x, ref_y);
  hadmap::Point3d p1(ref_x, ref_y, 0);
  hadmap::Point3d p2(x, y, 0);
  t = map_util::distanceBetweenPoints(p1, p2, false);
}

std::string ODRoad::getRoadType() const { return road_type; }

void ODRoad::successor(std::vector<std::pair<lanepkid, lanepkid>>& links) const {
  if (link_ptr == NULL) return;
  if (sections.empty()) return;
  auto& _back_sec = sections.back().second;
  roadpkid _to_rid = link_ptr->toRoadId();

  if (_to_rid == ROAD_PKID_INVALID) return;

  links.clear();
  for (auto& _lane_ptr : _back_sec) {
    if (_lane_ptr->toLId() == LANE_PKID_INVALID) {
      continue;
    } else {
      links.push_back(std::make_pair(_lane_ptr->id(), _lane_ptr->toLId()));
    }
  }
}

void ODRoad::links(std::vector<std::vector<lanepkid>>& links) const {
  if (link_ptr == NULL) return;
  if (sections.empty()) return;
  if (link_ptr->toRoadId() == ROAD_PKID_INVALID || link_ptr->fromRoadId() == ROAD_PKID_INVALID) return;
  auto& _front_sec = sections.front().second;
  for (auto& _lane_ptr : _front_sec) {
    if (_lane_ptr->fromLId() == LANE_PKID_INVALID || _lane_ptr->toLId() == LANE_PKID_INVALID) {
      continue;
    }
    std::vector<lanepkid> link;
    link.push_back(_lane_ptr->fromLId());
    link.push_back(_lane_ptr->id());
    link.push_back(_lane_ptr->toLId());

    for (size_t i = 1; i < sections.size(); i++) {
      auto& nx_sec = sections[i].second;
      for (auto& _nxlane_ptr : nx_sec) {
        if (_nxlane_ptr->fromLId() == LANE_PKID_INVALID || _nxlane_ptr->toLId() == LANE_PKID_INVALID) {
          continue;
        }
        if (_nxlane_ptr->fromLId() == link.at(link.size() - 2) && _nxlane_ptr->id() == link.back()) {
          link.push_back(_nxlane_ptr->toLId());
          break;
        }
      }
    }
    if (link.size() == sections.size() + 2) {
      links.push_back(link);
    }
  }
}

size_t ODRoad::sectionSize() const { return sections.size(); }

void ODRoad::getSectionRange(const size_t& index, double& begin, double& end) const {
  if (index >= sections.size()) throw std::runtime_error("get section range error, index range error");

  begin = sections[index].first;
  if (index + 1 < sections.size()) {
    end = sections[index + 1].first;
  } else {
    end = len;
  }
}

void ODRoad::getSection(const size_t& index, ODLaneSection& section) const {
  if (index >= sections.size()) throw std::runtime_error("get section error, index range error");
  section = sections[index].second;
}

roadpkid ODRoad::preRoadId() const {
  if (link_ptr == nullptr) return ROAD_PKID_INVALID;
  return link_ptr->fromRoadId();
}

void ODRoad::clearPreRoadI() {
  if (link_ptr == nullptr) return;
  link_ptr->clearFromRoad();
}

ODLink::ODLinkType ODRoad::preRoadType() const {
  if (link_ptr == nullptr) return ODLink::ODLinkType::ROAD;
  return link_ptr->fromRoadType();
}

ODLink::ODConType ODRoad::preRoadCont() const {
  if (link_ptr == nullptr) return ODLink::ODConType::START;
  return link_ptr->fromRoadCont();
}

roadpkid ODRoad::succRoadId() const {
  if (link_ptr == nullptr) return ROAD_PKID_INVALID;
  return link_ptr->toRoadId();
}

void ODRoad::clearSuccRoadI() {
  if (link_ptr == nullptr) return;
  link_ptr->clearToRoad();
}

ODLink::ODLinkType ODRoad::succRoadType() const {
  if (link_ptr == nullptr) return ODLink::ODLinkType::ROAD;
  return link_ptr->toRoadType();
}

ODLink::ODConType ODRoad::succRoadCont() const {
  if (link_ptr == nullptr) return ODLink::ODConType::START;
  return link_ptr->toRoadCont();
}

const ODGeomPtr ODRoad::getRefGeo(double s) const {
  if (ref_line.empty()) {
    return ODGeomPtr(nullptr);
  }
  for (const auto& _geom_ptr : ref_line) {
    if (s < (_geom_ptr->offset() + _geom_ptr->length())) {
      return _geom_ptr;
    }
  }
  return ref_line.back();
}

void ODRoad::roadGeom(double s, double& x, double& y) const {
  auto _geom_ptr = getRefGeo(s);
  if (_geom_ptr) {
    _geom_ptr->getGeom(s, x, y);
  }
}

double ODRoad::getHeading(double s) const {
  auto _geom_ptr = getRefGeo(s);
  if (_geom_ptr) {
    return _geom_ptr->getHeading(s);
  }
  return 0.0;
}

void ODRoad::roadNormal(double s, double& x, double& y) const {
  auto _geom_ptr = getRefGeo(s);
  if (_geom_ptr) {
    _geom_ptr->getNormal(s, x, y);
  }
}

double ODRoad::getEle(double s) const {
  if (elevation.empty()) return 0.0;
  size_t index = 0;
  for (size_t i = 1; i < elevation.size(); i++) {
    if (s < elevation[i].offset()) {
      break;
    }
    index = i;
  }
  double dis = s - elevation[index].offset();
  return elevation[index].value(dis);
}

void ODRoad::getObjects(std::vector<Object>& objs) const {
  auto getP = [&](double s, double t, ODXYZ& P, double& T) {
    double x = 0, y = 0, z = 0, dx = 0, dy = 0;
    roadGeom(s, x, y);
    roadNormal(s, dx, dy);
    T = getHeading(s);
    P.z = getEle(s);
    P.x = x + t * dx;
    P.y = y + t * dy;
  };
  objs.clear();
  for (const auto& obj_ptr : objects) {
    Object obj;
    obj.st = Object::OBJECT;
    {
      ODXYZ P;
      double T = 0;
      getP(obj_ptr->getS(), obj_ptr->getT(), P, T);
      P.z += obj_ptr->getZoffset();
      obj.x = P.x;
      obj.y = P.y;
      obj.z = P.z;
    }
    obj.id = obj_ptr->getId();
    obj.s = obj_ptr->getS();
    obj.t = obj_ptr->getT();
    obj.len = obj_ptr->getLength();
    obj.wid = obj_ptr->getWidth();
    obj.hei = obj_ptr->getHeight();
    obj.pitch = obj_ptr->getPitch();
    obj.roll = obj_ptr->getRoll();
    obj.yaw = obj_ptr->getHdg() + getHeading(obj_ptr->getS());
    obj.hdg = obj_ptr->getHdg();
    obj.name = obj_ptr->getName2();
    obj.type = obj_ptr->getType();
    obj.subtype = obj_ptr->getSubType();
    obj.zoffset = obj_ptr->getZoffset();

    obj_ptr->getGeom(obj.geom, getP);
    obj_ptr->getRepeat(obj.repeat, getP);

    // obj_ptr->getBoundary(obj.boundary, getP);
    if (obj_ptr->getRepeatData().size() > 0) {
      ODRepeatPtr tmp = obj_ptr->getRepeatData().front();
      obj.repetaData = "";
      obj.repetaData += ("s:" + std::to_string(tmp->s) + ";");
      obj.repetaData += ("length:" + std::to_string(tmp->length) + ";");
      obj.repetaData += ("distance:" + std::to_string(tmp->distance) + ";");
      obj.repetaData += ("tStart:" + std::to_string(tmp->tStart) + ";");
      obj.repetaData += ("tEnd:" + std::to_string(tmp->tEnd) + ";");
      obj.repetaData += ("widthStart:" + std::to_string(tmp->widthStart) + ";");
      obj.repetaData += ("widthEnd:" + std::to_string(tmp->widthEnd) + ";");
      obj.repetaData += ("heightStart:" + std::to_string(tmp->heightStart) + ";");
      obj.repetaData += ("heightEnd:" + std::to_string(tmp->heightEnd) + ";");
      obj.repetaData += ("zOffsetStart:" + std::to_string(tmp->zOffsetStart) + ";");
      obj.repetaData += ("zOffsetEnd:" + std::to_string(tmp->zOffsetEnd) + ";");
      obj.repetaData += ("lengthStart:" + std::to_string(tmp->lengthStart) + ";");
      obj.repetaData += ("lengthEnd:" + std::to_string(tmp->lengthEnd) + ";");
    }
    if (obj_ptr->getMarking() && obj_ptr->getMarking()->markings.size() > 0) {
      obj.markwidth = obj_ptr->getMarking()->markings.at(0).width;
      obj.markcolor = obj_ptr->getMarking()->markings.at(0).color;
    }

    auto park = obj_ptr->getParkingSpace();
    if (park) {
      obj.st = Object::PARKING;
      obj.access = park->access;
      obj.restrictions = park->restrictions;
      for (const auto& mark : park->markings) {
        Object::ParkingMark pmark;
        pmark.side = mark.side;
        pmark.type = mark.type;
        pmark.width = mark.width;
        pmark.color = mark.color;
        obj.parkingmark.push_back(pmark);
      }
      if (obj_ptr->getIsOutLine() == true) {
        if (obj.repeat.size() > 0) {
          obj_ptr->getRepeateFromOutlines();
          obj.geom.clear();
          obj_ptr->getGeom(obj.geom, getP);
        }
      }
      // opendrive 1.4 to geom
      if (obj_ptr->getIsOutLines() == false) {
        if (obj.repeat.size() > 0) {
          std::vector<std::vector<ODXYZ>> geomDatas;
          obj_ptr->getRepeateBoundarys(geomDatas, getP);
          obj.geom.clear();
          for (auto it : geomDatas) {
            obj.geom.push_back(std::make_pair(false, it));
          }
        } else {
          std::vector<ODXYZ> boundary;
          obj_ptr->getBoundary(boundary, getP);
          obj.geom.clear();
          obj.geom.push_back(std::make_pair(false, boundary));
        }
      }
    }
    obj.userData = obj_ptr->getUserData();
    obj.validityLane = obj_ptr->validityLane;

    objs.push_back(obj);
  }
  for (const auto& sign_ptr : signals) {
    Object obj;
    obj.st = sign_ptr->dynamic == "yes" ? Object::SIGNAL_DYNAMIC : Object::SIGNAL;
    obj.id = sign_ptr->id;
    obj.wid = sign_ptr->width;
    obj.hei = sign_ptr->height;
    obj.pitch = sign_ptr->pitch;
    obj.roll = sign_ptr->roll;
    obj.yaw = sign_ptr->hOffset + getHeading(sign_ptr->s);
    obj.s = sign_ptr->s;
    obj.t = sign_ptr->t;
    obj.zoffset = sign_ptr->zoffset;
    obj.hdg = sign_ptr->hOffset;
    if (sign_ptr->orientation == "-") {
      obj.yaw += 3.1415926535897932384626433832795;
    }
    obj.name = sign_ptr->name;
    obj.type = sign_ptr->type;
    obj.subtype = sign_ptr->subtype;
    sign_ptr->getGeom(obj.geom, getP);
    // sign_ptr->getBoundary(obj.boundary, getP);
    {
      ODXYZ P;
      double T = 0;
      getP(sign_ptr->s, sign_ptr->t, P, T);
      P.z += sign_ptr->zoffset;
      obj.x = P.x;
      obj.y = P.y;
      obj.z = P.z;
    }
    obj.userData = sign_ptr->userData;
    obj.validityLane = sign_ptr->validityLane;
    objs.push_back(obj);
  }
}

bool ODController::parse(tinyxml2::XMLElement* element) {
  cid = element->Int64Attribute("id");
  XMLElement* ctl_elem = element->FirstChildElement("control");
  while (ctl_elem) {
    controllerkid id = ctl_elem->Int64Attribute("signalId");
    signalIds.push_back(id);
    ctl_elem = ctl_elem->NextSiblingElement("control");
  }
  return true;
}

controllerkid ODController::id() const { return cid; }

const std::vector<objectpkid>& ODController::getSignsals() const { return signalIds; }

bool ODMarkings::parse(tinyxml2::XMLElement* element) {
  tinyxml2::XMLElement* marking = element->FirstChildElement("marking");
  while (marking) {
    Marking tmp;
    tmp.width = marking->DoubleAttribute("width");
    tmp.color = marking->Attribute("color");
    this->markings.push_back(tmp);
    marking = marking->NextSiblingElement("marking");
  }
  return true;
}

bool ODCrg::parse(tinyxml2::XMLElement* element) {
  const char* p = element->Attribute("file");
  if (p) file = p;

  p = element->Attribute("orientation");
  if (p) orientation = p;

  p = element->Attribute("mode");
  if (p) mode = p;

  p = element->Attribute("purpose");
  if (p) purpose = p;

  p = element->Attribute("sOffset");
  if (p) soffset = p;

  p = element->Attribute("sOffset");
  if (p) toffset = p;

  p = element->Attribute("zOffset");
  if (p) zoffset = p;

  p = element->Attribute("hOffset");
  if (p) hoffset = p;

  p = element->Attribute("zScale");
  if (p) zscale = p;

  return true;
}

std::string ODCrg::getFile() { return this->file; }

std::string ODCrg::getOrientation() { return this->orientation; }

std::string ODCrg::getMode() { return this->mode; }

std::string ODCrg::getPurpose() { return this->purpose; }

std::string ODCrg::getSOffset() { return this->soffset; }

std::string ODCrg::getTOffset() { return this->toffset; }

std::string ODCrg::getZOffset() { return this->zoffset; }

std::string ODCrg::getZScale() { return this->zscale; }

std::string ODCrg::getHOffset() { return this->hoffset; }

}  // namespace hadmap
