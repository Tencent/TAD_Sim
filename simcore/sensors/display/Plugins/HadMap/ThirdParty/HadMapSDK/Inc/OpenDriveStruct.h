// Copyright 2024 Tencent Inc. All rights reserved.
// Project:  SimMapsdk
// @author   longerding@tencent
// Modify history:
//

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../../datamodel/include/structs/base_struct.h"
#include "tinyxml2.h"
#include "types/map_defs.h"
#include "types/map_types.h"

namespace hadmap {
using AreaPointVec = std::vector<std::tuple<hadmap::txPoint, hadmap::txPoint>>;

class ODXYZ {
 public:
  // Constructor with default values
  explicit ODXYZ(double _x = 0, double _y = 0, double _z = 0) : x(_x), y(_y), z(_z) {}

  // Overload the += operator
  ODXYZ& operator+=(const ODXYZ& rhs);

  // Overload the + operator
  ODXYZ operator+(const ODXYZ& rhs) const;

  double x = 0, y = 0, z = 0;
};

class ODXYZWH : public ODXYZ {
 public:
  double w = 0, h = 0;
};

class ODXYZWHP : public ODXYZWH {
 public:
  double l = 0;
  double p = 0;
};

class ODRPY {
 public:
  // Constructor with default values
  explicit ODRPY(double h = 0, double r = 0, double p = 0) : hdg(h), roll(r), pitch(p) {}

  double hdg = 0, roll = 0, pitch = 0;

  // Overload the * operator
  ODXYZ operator*(const ODXYZ& p) const;
};

class ODNode {
 public:
  // Constructor with node name
  explicit ODNode(const std::string& n) : nodeName(n) {}

  // Destructor
  virtual ~ODNode() {}

 public:
  // Check the element
  void elementCheck(tinyxml2::XMLElement* element);

  // Get the node name
  std::string getName() const { return nodeName; }

 protected:
  std::string nodeName;
};

class ODHeader : public ODNode {
 public:
  // Constructor
  ODHeader() : ODNode("header"), north(0.0), south(0.0), east(0.0), west(0.0) {}

  // Destructor
  ~ODHeader() {}

 public:
  // Parse the element
  bool parse(tinyxml2::XMLElement* element);

  // Get the bounding box
  void getBBox(double& north, double& south, double& east, double& west);

  // Get the geo reference
  std::string getGeoRef() const;

  // Get the vendor
  std::string getVendor() const;

  // Get the date
  std::string getDate() const;

  // Get the major version
  uint8_t getRemajor() const;

  // Get the minor version
  uint8_t getReminor() const;

 private:
  double north;
  double south;
  double east;
  double west;
  std::string coord_str;
  uint8_t remajor = 1;
  uint8_t reminor = 4;
  std::string vendor;
  std::string date;
};

class ODLink : public ODNode {
 public:
  // Enum for link type
  enum ODLinkType { ROAD = 1, JUNC = 2 };

  // Enum for connection type
  enum ODConType { START = 1, END = 2 };

 public:
  // Constructor
  ODLink() : ODNode("link"), from_rid(0), from_type(ROAD), from_cont(END), to_rid(0), to_type(ROAD), to_cont(START) {}

  // Destructor
  ~ODLink() {}

 public:
  // Parse the element
  bool parse(tinyxml2::XMLElement* element);

  // Get the from road ID
  pkid_t fromRoadId() const;

  // Clear the from road
  void clearFromRoad();

  // Get the from road type
  ODLinkType fromRoadType() const;

  // Get the to road ID
  pkid_t toRoadId() const;

  // Clear the to road
  void clearToRoad();

  // Get the to road type
  ODLinkType toRoadType() const;

  // Get the from road connection type
  ODLink::ODConType fromRoadCont() const;

  // Get the to road connection type
  ODLink::ODConType toRoadCont() const;

 private:
  pkid_t from_rid;
  ODLinkType from_type;
  ODConType from_cont;
  pkid_t to_rid;
  ODLinkType to_type;
  ODConType to_cont;
};

// Using declaration for ODLinkPtr
using ODLinkPtr = std::shared_ptr<ODLink>;

class ODConnection : public ODNode {
 public:
  // Constructor
  ODConnection() : ODNode("connection") {}

  // Destructor
  ~ODConnection() {}

 public:
  // Parse the element
  bool parse(tinyxml2::XMLElement* element);

  // Get the size of lane links
  size_t lanelinkSize() const;

  // Get the from road ID
  roadpkid fromRoadId() const;

  // Get the to road ID
  roadpkid toRoadId() const;

  // Get the lane link by index
  std::pair<lanepkid, lanepkid> lanelink(const size_t& index) const;

 private:
  roadpkid from_rid;
  roadpkid to_rid;
  std::vector<std::pair<lanepkid, lanepkid>> lane_links;
};

// Using declaration for ODConnectionPtr
using ODConnectionPtr = std::shared_ptr<ODConnection>;

class ODController : public ODNode {
 public:
  // Constructor
  ODController() : ODNode("controller") {}

  // Destructor
  ~ODController() {}

 public:
  // Parse the element
  bool parse(tinyxml2::XMLElement* element);

  // Get the controller ID
  controllerkid id() const;

  // Get the signal IDs
  const std::vector<objectpkid>& getSignsals() const;

 private:
  controllerkid cid;
  std::vector<objectpkid> signalIds;
};

// Using declaration for ODControllerPtr
using ODControllerPtr = std::shared_ptr<ODController>;

class ODJunction : public ODNode {
 public:
  // Constructor
  ODJunction() : ODNode("junction"), jid(0) {}

  // Destructor
  ~ODJunction() {}

 public:
  // Parse the element
  bool parse(tinyxml2::XMLElement* element);

  // Get the junction ID
  junctionpkid id() const;

  // Get the size of connections
  size_t connectionSize() const;

  // Get the connection by index
  ODConnectionPtr connection(const size_t& index);

  // Get the controller IDs
  const std::vector<controllerkid>& getControllers() const;

 private:
  junctionpkid jid;
  std::vector<ODConnectionPtr> connections;
  std::vector<controllerkid> controllers;
};

// Using declaration for ODJunctionPtr
using ODJunctionPtr = std::shared_ptr<ODJunction>;

class Polynomial3D : public ODNode {
 public:
  // Constructor
  explicit Polynomial3D(const std::string& node_name) : ODNode(node_name), s(0.0), a(0.0), b(0.0), c(0.0), d(0.0) {}

  // Destructor
  ~Polynomial3D() {}

 public:
  // Parse the element
  bool parse(tinyxml2::XMLElement* element);

  // Sample the value at the given distances
  bool sampleValue(const std::vector<double>& dis, std::vector<double>& value) const;

  // Get the value at the given distance
  double value(const double& dis) const;

  // Get the slope at the given distance
  double slope(const double& dis) const;

  // Get the start distance
  double offset() const { return s; }

  // Get the parameters
  void getParam(double& _a, double& _b, double& _c, double& _d, double& _s) const;

 private:
  double s;
  double a;
  double b;
  double c;
  double d;
};

class ODLane : public ODNode {
 public:
  // Constructor
  ODLane()
      : ODNode("lane"),
        lid(0),
        from_lid(0),
        to_lid(0),
        lane_mark(LANE_MARK_None),
        laneboundary_width(0.0),
        laned_interval(0),
        speed_limit(0),
        lane_type(LANE_TYPE_None),
        lane_surface(LANE_SURFACE_OTHER),
        lane_friction(0.0) {}

  // Destructor
  ~ODLane() {}

 public:
  // Parse the element
  bool parse(tinyxml2::XMLElement* element);

  // Get the lane ID
  lanepkid id() const;

  // Get the from lane ID
  lanepkid fromLId() const;

  // Get the to lane ID
  lanepkid toLId() const;

  // Get the lane mark
  LANE_MARK mark() const;

  // Get the lane type
  LANE_TYPE type() const;

  // Get the lane surface
  LANE_SURFACE surface() const;

  // Get the lane friction
  double friction() const;

  // Get the speed limit
  int speedlimit() const;

  // Get the lane width
  double lanewidth() const;

  // Get the lane interval
  double lanedinterval() const;

  // Get the offset
  double offset() const;

  // Get the width at the given distances
  void width(const std::vector<double>& ds, std::vector<double>& width) const;

  //
  lanepkid lid;
  lanepkid from_lid;
  lanepkid to_lid;
  LANE_MARK lane_mark;
  double laneboundary_width;
  double laned_interval;
  double laned_space;
  double laned_soffset;
  double laned_len;
  int speed_limit;
  LANE_TYPE lane_type;
  LANE_SURFACE lane_surface;
  double lane_material_sOffset;

  double lane_friction;
  // Lane parameters
  struct GeomParam {
    double s;
    double a;
    double b;
    double c;
    double d;
  };
  std::vector<GeomParam> params;
};

// Using declaration for ODLanePtr
using ODLanePtr = std::shared_ptr<ODLane>;

// Using declaration for ODLaneSection
using ODLaneSection = std::vector<ODLanePtr>;

class ODGeometry : public ODNode {
 public:
  // Constructor
  ODGeometry() : ODNode("geometry") {}

  // Destructor
  virtual ~ODGeometry() {}

 public:
  // Parse the element
  virtual bool parse(tinyxml2::XMLElement* element);

  // Get the offset
  double offset() const;

  // Get the start X coordinate
  double startX() const;

  // Get the start Y coordinate
  double startY() const;

  // Get the heading
  double heading() const;

  // Get the length
  double length() const;

  // Set the length
  void setLength(double length);

  // Get the geometry points
  virtual void getGeom(std::vector<std::pair<double, double>>& points) const;

  // Get the tangents
  virtual void getTangent(std::vector<std::pair<double, double>>& tangents) const;

  // Get the normals
  virtual void getNormal(std::vector<std::pair<double, double>>& normals) const;

  // Get the intervals
  virtual void getInterval(std::vector<double>& intervals) const = 0;

  // Get the curvature
  virtual double getCurvature(double soffset) const = 0;

  // Get the geometry point at the given s coordinate
  virtual void getGeom(double s, double& x, double& y) const = 0;

  // Get the heading at the given s coordinate
  virtual double getHeading(double s) const = 0;

  // Get the normal at the given s coordinate
  virtual void getNormal(double s, double& x, double& y) const;

  // Get the tangent at the given s coordinate
  virtual void getTangent(double s, double& x, double& y) const;

 protected:
  double _s;
  double _x;
  double _y;
  double _hdg;
  double _len;

  const double pi = 3.1415926535897932384626433832795;

  // Rotate the coordinates
  void rotate(double angle, double& x, double& y) const;
};

// Using declaration for ODGeomPtr
using ODGeomPtr = std::shared_ptr<ODGeometry>;

class ODParamLine : public ODGeometry {
 public:
  // Constructor
  ODParamLine() : ODGeometry() {
    nodeName = "line";
    min_len = 1.0;
  }

  // Destructor
  ~ODParamLine() {}

 public:
  // Get the curvature
  virtual double getCurvature(double soffset) const;

  // Get the intervals
  void getInterval(std::vector<double>& intervals) const;

  // Get the geometry point at the given s coordinate
  virtual void getGeom(double s, double& x, double& y) const;

  // Get the heading at the given s coordinate
  virtual double getHeading(double s) const;

 private:
  double min_len = 1;
};

// Using declaration for ODParamLinePtr
using ODParamLinePtr = std::shared_ptr<ODParamLine>;

class ODGeomSpiral : public ODGeometry {
 public:
  // Constructor
  ODGeomSpiral() : ODGeometry() {
    nodeName = "spiral";
    min_len = 1.0;
    min_angle = 0.314159265;  // 18°
  }

  // Destructor
  ~ODGeomSpiral() {}

 public:
  // Parse the element
  bool parse(tinyxml2::XMLElement* element);

  // Get the intervals
  void getInterval(std::vector<double>& intervals) const;

  // Get the curvature
  virtual double getCurvature(double soffset) const;

  // Get the geometry point at the given s coordinate
  virtual void getGeom(double s, double& x, double& y) const;

  // Get the heading at the given s coordinate
  virtual double getHeading(double s) const;

 private:
  double curve_start;
  double curve_end;
  double min_len;
  double min_angle;

  // Evaluate the spiral function
  void odrSpiral(double s, double cDot, double* x, double* y, double* t) const;

  // Evaluate the Fresnel integral
  void fresnel(double xxa, double* ssa, double* cca) const;

  // Evaluate the polynomial
  double p1evl(double x, double* coef, int n) const;

  // Evaluate the polynomial
  double polevl(double x, double* coef, int n) const;
};

// Using declaration for ODParamSpiralPtr
using ODParamSpiralPtr = std::shared_ptr<ODGeomSpiral>;

class ODParamArc : public ODGeometry {
 public:
  // Constructor
  ODParamArc() : ODGeometry() {
    nodeName = "arc";
    min_len = 1.0;
    min_angle = 0.314159265;  // 18°
  }

  // Destructor
  ~ODParamArc() {}

 public:
  // Parse the element
  bool parse(tinyxml2::XMLElement* element);

  // Get the curvature
  virtual double getCurvature(double soffset) const;

  // Get the intervals
  void getInterval(std::vector<double>& intervals) const;

  // Get the geometry point at the given s coordinate
  virtual void getGeom(double s, double& x, double& y) const;

  // Get the heading at the given s coordinate
  virtual double getHeading(double s) const;

 private:
  double curvature;
  double min_len;
  double min_angle;

  // Get the interval step
  double getIntervalStep() const;
};

// Using declaration for ODParamArcPtr
using ODParamArcPtr = std::shared_ptr<ODParamArc>;
// ODGeomPoly3 class represents a 3D polynomial geometry
class ODGeomPoly3 : public ODGeometry {
 public:
  // Default constructor
  ODGeomPoly3() : ODGeometry() {
    nodeName = "poly3";
    min_len = 1.0;
    min_angle = 0.314159265;  // 18 degrees
  }

  // Destructor
  ~ODGeomPoly3() {}

 public:
  // Parse the XML element and set the attributes
  bool parse(tinyxml2::XMLElement* element);

  // Get the intervals for the geometry
  void getInterval(std::vector<double>& intervals) const;

  // Get the curvature at the given offset
  virtual double getCurvature(double soffset) const;

  // Get the geometry at the given offset
  virtual void getGeom(double s, double& x, double& y) const;

  // Get the heading at the given offset
  virtual double getHeading(double s) const;

 private:
  double a;
  double b;
  double c;
  double d;
  double min_angle;
  double min_len;
};
using ODODGeomPoly3Ptr = std::shared_ptr<ODGeomPoly3>;

// ODParamPoly3 class represents a 3D parametric polynomial geometry
class ODParamPoly3 : public ODGeometry {
 public:
  // Default constructor
  ODParamPoly3() : ODGeometry() {
    nodeName = "paramPoly3";
    min_len = 1.0;
    min_angle = 0.314159265;  // 18 degrees
    normalized = true;
  }

  // Destructor
  ~ODParamPoly3() {}

 public:
  // Parse the XML element and set the attributes
  bool parse(tinyxml2::XMLElement* element);

  // Get the intervals for the geometry
  void getInterval(std::vector<double>& intervals) const;

  // Get the curvature at the given offset
  virtual double getCurvature(double soffset) const;

  // Get the geometry at the given offset
  virtual void getGeom(double s, double& x, double& y) const;

  // Get the heading at the given offset
  virtual double getHeading(double s) const;

 private:
  // Fit the center by least squares
  void FitCenterByLeastSquares(std::vector<std::pair<double, double>> mapPoint, double& radius) const;

 private:
  double aU;
  double bU;
  double cU;
  double dU;
  double aV;
  double bV;
  double cV;
  double dV;
  bool normalized;
  double min_angle;
  double min_len;
};
using ODParamPoly3Ptr = std::shared_ptr<ODParamPoly3>;

// ODCorner class represents a corner in the road network
class ODCorner : public ODNode {
 public:
  // Constructor
  explicit ODCorner(const std::string& n) : ODNode(n) {}

  // Parse the XML element and set the attributes
  virtual bool parse(tinyxml2::XMLElement* element) = 0;

  // Get the XYZ coordinates of the corner
  virtual ODXYZ getxyz() = 0;

  // Check if the corner is local
  virtual bool isLocal() const = 0;
};
using ODCornerPtr = std::shared_ptr<ODCorner>;
using ODCorners = std::vector<ODCornerPtr>;

// ODCornerRoad class represents a corner in the road network
class ODCornerRoad : public ODCorner {
 public:
  // Default constructor
  ODCornerRoad() : ODCorner("cornerRoad") {}

  // Parse the XML element and set the attributes
  bool parse(tinyxml2::XMLElement* element);

  // Get the XYZ coordinates of the corner
  virtual ODXYZ getxyz();

  // Set the XYZ coordinates of the corner
  void setXyz(ODXYZ data);

  // Check if the corner is local
  bool isLocal() const { return false; }

 private:
  uint64_t id;
  double s;
  double t;
  double dz;
  double height;
};
using ODCornerRoadPtr = std::shared_ptr<ODCornerRoad>;

// ODCornerLocal class represents a local corner in the road network
class ODCornerLocal : public ODCorner {
 public:
  // Default constructor
  ODCornerLocal() : ODCorner("cornerLocal") {}

  // Parse the XML element and set the attributes
  bool parse(tinyxml2::XMLElement* element);

  // Get the XYZ coordinates of the corner
  virtual ODXYZ getxyz();

  // Check if the corner is local
  bool isLocal() const { return true; }

 private:
  uint64_t id;
  double u;
  double v;
  double z;
  double height;
};
using ODCornerLocalPtr = std::shared_ptr<ODCornerLocal>;

// ODOutline class represents an outline in the road network
class ODOutline : public ODNode {
 public:
  // Default constructor
  ODOutline() : ODNode("outline") {}

 public:
  // Parse the XML element and set the attributes
  bool parse(tinyxml2::XMLElement* element);

  // Get the geometry of the outline
  void getGeom(std::vector<std::pair<bool, ODXYZ>>& points);

  // Check if the outline is closed
  bool isClosed() const { return closed; }

  // Insert a corner into the outline
  bool insertCorner(ODCornerPtr tmp) {
    corners.push_back(tmp);
    return true;
  }

 private:
  uint64_t id;
  bool closed;
  ODCorners corners;
};
using ODOutlinePtr = std::shared_ptr<ODOutline>;
using ODOutlines = std::vector<ODOutlinePtr>;

// ODRepeat class represents a repeat element in the road network
class ODRepeat : public ODNode {
 public:
  // Default constructor
  ODRepeat() : ODNode("repeat") {}

  double s;
  double length;
  double distance;
  double tStart;
  double tEnd;
  double widthStart;
  double widthEnd;
  double heightStart;
  double heightEnd;
  double zOffsetStart;
  double zOffsetEnd;
  double lengthStart;
  double lengthEnd;

  // Parse the XML element and set the attributes
  bool parse(tinyxml2::XMLElement* element);
};
using ODRepeatPtr = std::shared_ptr<ODRepeat>;

class ODParkingSpace : public ODNode {
 public:
  // Default constructor
  ODParkingSpace() : ODNode("parkingSpace") {}

  // Parse the parking space information from the XML element
  bool parse(tinyxml2::XMLElement* element);

  // Parse the parking space markings from the XML element
  bool parseMarks(tinyxml2::XMLElement* element);

  // Access type
  std::string access;

  // Restrictions
  std::string restrictions;

  // Parking space marking structure
  struct Marking {
    std::string side;
    std::string type;
    double width;
    std::string color;
  };

  // Vector of parking space markings
  std::vector<Marking> markings;
};

// Shared pointer for ODParkingSpace
using ODParkingSpacePtr = std::shared_ptr<ODParkingSpace>;

class ODMarkings : public ODNode {
 public:
  // Default constructor
  ODMarkings() : ODNode("markings") {}

  // Parse the markings information from the XML element
  bool parse(tinyxml2::XMLElement* element);

  // Marking structure
  struct Marking {
    std::string side;
    std::string type;
    double width;
    std::string color;
  };

  // Vector of markings
  std::vector<Marking> markings;
};

// Shared pointer for ODMarkings
using ODMarkingsPtr = std::shared_ptr<ODMarkings>;

class ODObject : public ODNode {
 public:
  // Default constructor
  ODObject() : ODNode("object") {}

  // Parse the object information from the XML element
  bool parse(tinyxml2::XMLElement* element);

  // Parse the tunnel information from the XML element
  bool parseTunnel(tinyxml2::XMLElement* element);

  // Get the object geometry
  void getGeom(std::vector<std::pair<bool, std::vector<ODXYZ>>>& points,
               std::function<void(double s, double t, ODXYZ& P, double& T)> getP) const;

  // Get the object repeat information
  void getRepeat(std::vector<ODXYZWH>& points, std::function<void(double s, double t, ODXYZ& P, double& T)> getP) const;

  // Get the object boundary
  void getBoundary(std::vector<ODXYZ>& points, std::function<void(double s, double t, ODXYZ& P, double& T)> getP) const;

  // Get the repeated boundary
  void getRepeateBoundarys(std::vector<std::vector<ODXYZ>>& points,
                           std::function<void(double s, double t, ODXYZ& P, double& T)> getP);

  // Get the object type
  const std::string& getType() const { return type; }

  // Get the object subtype
  const std::string& getSubType() const { return subtype; }

  // Get the object name
  const std::string& getName2() const { return name; }

  // Get the object ID
  const std::string& getId() const { return id; }

  // Get the object heading
  double getHdg() const { return hdg; }

  // Get the object pitch
  double getPitch() const { return pitch; }

  // Get the object roll
  double getRoll() const { return roll; }

  // Get the object length
  double getLength() const { return length; }

  // Get the object width
  double getWidth() const { return width; }

  // Get the object height
  double getHeight() const { return height; }

  // Get the object s offset
  double getS() const { return s; }

  // Get the object t offset
  double getT() const { return t; }

  // Get the object z offset
  double getZoffset() const { return zoffset; }

  // Check if the object has outlines
  bool getIsOutLines() const { return !outlines.empty(); }

  // Check if the object has a single outline
  bool getIsOutLine() const { return outlines.size() == 1; }

  // Get the repeated outlines
  void getRepeateFromOutlines();

  // Get the object repeat data
  std::vector<ODRepeatPtr> getRepeatData() const { return repeats; }

  // Get the marks
  ODMarkingsPtr getMarking() const { return markings; }

  // Get the object parking space
  ODParkingSpacePtr getParkingSpace() const { return parkingSpace; }

  // Get the object user data
  const std::map<std::string, std::string>& getUserData() const { return userData; }

  // Vector of valid lane IDs
  std::vector<int> validityLane;

 private:
  // Get points from user data
  void getPointFromUserData(std::string str, std::vector<ODXYZ>& points);

  // Object type
  std::string type;

  // Object subtype
  std::string subtype;

  // Object name
  std::string name;

  // Object ID
  std::string id;

  // Object s offset
  double s{0};

  // Object t offset
  double t{0};

  // Object z offset
  double zoffset{0};

  // Object heading
  double hdg{0};

  // Object pitch
  double pitch{0};

  // Object roll
  double roll{0};

  // Object length
  double length{0};

  // Object width
  double width{0};

  // Object height
  double height{0};

  // Object radius
  double radius{0};

  // Flag indicating if the object has outlines
  bool isOutLines;

  // Object markings
  ODMarkingsPtr markings;

  // Vector of object repeats
  std::vector<ODRepeatPtr> repeats;

  // Object outlines
  ODOutlines outlines;

  // Object parking space
  ODParkingSpacePtr parkingSpace;

  // Vector of object points
  std::vector<ODXYZ> m_points;

  // Object user data
  std::map<std::string, std::string> userData;
};

// Shared pointer for ODObject
using ODObjectPtr = std::shared_ptr<ODObject>;

// Vector of ODObject shared pointers
using ODObjects = std::vector<ODObjectPtr>;

class ODSignal : public ODNode {
 public:
  // Default constructor
  ODSignal() : ODNode("signal") {}

  // Parse the signal information from the XML element
  bool parse(tinyxml2::XMLElement* element);

  // Get the signal geometry
  void getGeom(std::vector<std::pair<bool, std::vector<ODXYZ>>>& points,
               std::function<void(double, double, ODXYZ&, double&)> getP) const;

  // Get the signal boundary
  void getBoundary(std::vector<ODXYZ>& points, std::function<void(double s, double t, ODXYZ& P, double& T)> getP) const;

  // Signal type
  std::string type;

  // Signal subtype
  std::string subtype;

  // Signal name
  std::string name;

  // Signal ID
  std::string id;

  // Signal dynamic property
  std::string dynamic;

  // Signal country
  std::string country;

  // Signal orientation
  std::string orientation;

  // Signal text
  std::string text;

  // Signal s offset
  double s{0};

  // Signal t offset
  double t{0};

  // Signal z offset
  double zoffset{0};

  // Signal heading offset
  double hOffset{0};

  // Signal pitch
  double pitch{0};

  // Signal roll
  double roll{0};

  // Signal heading
  double hdg{0};

  // Signal value
  double value{0};

  // Signal width
  double width{0};

  // Signal height
  double height{0};

  // Signal user data
  std::map<std::string, std::string> userData;

  // Vector of valid lane IDs
  std::vector<int> validityLane;
};
using ODSignalPtr = std::shared_ptr<ODSignal>;
using ODSignals = std::vector<ODSignalPtr>;

class ODCrg : public ODNode {
 public:
  // Default constructor
  ODCrg() : ODNode("CRG") {}

  // Destructor
  ~ODCrg() {}

  // Parse the CRG information from the XML element
  bool parse(tinyxml2::XMLElement* element);

  // Get the CRG file
  std::string getFile();

  // Get the CRG orientation
  std::string getOrientation();

  // Get the CRG mode
  std::string getMode();

  // Get the CRG purpose
  std::string getPurpose();

  // Get the CRG s offset
  std::string getSOffset();

  // Get the CRG t offset
  std::string getTOffset();

  // Get the CRG z offset
  std::string getZOffset();

  // Get the CRG z scale
  std::string getZScale();

  // Get the CRG h offset
  std::string getHOffset();

 private:
  std::string file;
  std::string orientation;
  std::string mode;
  std::string purpose;
  std::string soffset;
  std::string toffset;
  std::string zoffset;
  std::string zscale;
  std::string hoffset;
};

class ODRoad : public ODNode {
 public:
  // Default constructor
  ODRoad() : ODNode("road"), speed_limit(0) {}

  // Destructor
  ~ODRoad() {}

 public:
  // Parse the road information from the XML element
  bool parse(tinyxml2::XMLElement* element);

  // Parse the road information from the XML element with additional parameters
  bool parse(tinyxml2::XMLElement* element, double _west, double _south, std::vector<std::vector<txPoint>> area);

  // Get the road ID
  roadpkid id() const;

  // Get the road length
  double length() const;

  // Get the road speed limit
  int speedlimit() const;

  // Get the road geometry
  void roadGeom(std::vector<std::pair<double, double>>& geom, const std::vector<double>* interval = nullptr) const;

  // Get the road tangent
  void roadTangent(std::vector<std::pair<double, double>>& tangent,
                   const std::vector<double>* interval = nullptr) const;

  // Get the road normal
  void roadNormal(std::vector<std::pair<double, double>>& normal, const std::vector<double>* interval = nullptr) const;

  // Get the road interval
  void roadInterval(std::vector<double>& interval) const;

  // Get the road curvature
  void roadCurvature(std::vector<double>& startx, std::vector<double>& length, std::vector<double>& radios);

  // Get the road reference line
  void roadRef(std::vector<std::pair<double, double>>& ref, const std::vector<double>* interval = nullptr) const;

  // Get the road elevation
  void roadEle(std::vector<double>& ele, const std::vector<double>* interval = nullptr) const;

  // Get the road elevation control points
  void roadEleControlPoint(std::vector<std::vector<double>>& ele, double length, std::string map_version);

  // Get the road slope
  void roadSlope(std::vector<double>& startx, std::vector<double>& length, std::vector<double>& slope);

  // Get the road control points
  void roadControlPoint(std::string& controltype, std::vector<std::vector<double>>& points);

  // Get the predecessor road links
  void predecessor(std::vector<std::pair<lanepkid, lanepkid>>& links) const;

  // Get the predecessor road ID
  roadpkid preRoadId() const;

  // Clear the predecessor road ID
  void clearPreRoadI();

  // Get the predecessor road type
  ODLink::ODLinkType preRoadType() const;

  // Get the predecessor road connection type
  ODLink::ODConType preRoadCont() const;

  // Get the successor road links
  void successor(std::vector<std::pair<lanepkid, lanepkid>>& links) const;

  // Get the successor road ID
  roadpkid succRoadId() const;

  // Clear the successor road ID
  void clearSuccRoadI();

  // Get the successor road type
  ODLink::ODLinkType succRoadType() const;

  // Get the successor road connection type
  ODLink::ODConType succRoadCont() const;

  // Get the reference geometry at a specific s value
  const ODGeomPtr getRefGeo(double s) const;

  // Get the number of road sections
  size_t sectionSize() const;

  // Get the road type
  std::string getRoadType() const;

  // Get the road section range
  void getSectionRange(const size_t& index, double& begin, double& end) const;

  // Get the road section
  void getSection(const size_t& index, ODLaneSection& section) const;

  // Get the junction ID
  junctionpkid junctionId() const;

  // Get the section index for a specific s value
  size_t getSectionIdx(double s);

  // Get the t value for a specific x, y, and s value
  void getT(double x, double y, double s, double& t);

  // Get the CRGs
  std::vector<ODCrg> getCrgs() const;

  // Get links
  void links(std::vector<std::vector<lanepkid>>& links) const;

  // Struct to store road object information
  struct Object {
    // Enum for object source type
    enum SourceType { OBJECT = 0, SIGNAL, SIGNAL_DYNAMIC, PARKING };
    SourceType st;
    std::string type;
    std::string subtype;
    std::string name;
    std::string id;
    double s = 0, t = 0;
    double len = 0, wid = 0, hei = 0;
    double roll = 0, pitch = 0, yaw = 0;
    double hdg = 0;
    double zoffset = 0;
    double x = 0, y = 0, z = 0;
    double markwidth = 0;
    std::string markcolor = "";
    std::vector<std::pair<bool, std::vector<ODXYZ>>> geom;
    std::vector<ODXYZWH> repeat;
    std::vector<ODXYZ> boundary;
    std::string access;
    std::string restrictions;
    std::string repetaData;

    // Struct to store parking mark information
    struct ParkingMark {
      std::string side;
      std::string type;
      double width;
      std::string color;
    };
    std::vector<ParkingMark> parkingmark;

    std::map<std::string, std::string> userData;
    std::vector<int> validityLane;
  };

  // Get the road objects
  void getObjects(std::vector<Object>& objs) const;

  // Struct to store signal reference information
  struct SignalRef {
    std::string id;
    double s = 0, t = 0;
    std::vector<int> laneids;
  };
  std::map<std::string, SignalRef> signalReferences;

 private:
  ODLinkPtr link_ptr;
  std::vector<ODGeomPtr> ref_line;
  std::vector<Polynomial3D> lane_offset;
  std::vector<Polynomial3D> elevation;
  std::vector<std::pair<double, ODLaneSection>> sections;
  std::vector<ODCrg> crgs;
  ODObjects objects;
  ODSignals signals;
  std::string road_type;
  roadpkid rid;
  double len;
  int speed_limit;
  junctionpkid jid;

  // Helper functions for road geometry calculations
  void roadGeom(double s, double& x, double& y) const;
  double getHeading(double s) const;
  void roadNormal(double s, double& x, double& y) const;
  double getEle(double s) const;
};

using ODRoadPtr = std::shared_ptr<ODRoad>;
using ODRoads = std::vector<ODRoadPtr>;

}  // namespace hadmap
