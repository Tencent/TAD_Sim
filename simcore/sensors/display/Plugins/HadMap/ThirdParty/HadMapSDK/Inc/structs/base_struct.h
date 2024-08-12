// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <stdint.h>
#include <string.h>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "common/macros.h"
#include "types/map_defs.h"
#include "types/map_types.h"

namespace hadmap {
// Enum for coordinate types
enum TXSIMMAP_API CoordType {
  COORD_XYZ = 0,    // Global coordinates (X, Y, Z)
  COORD_WGS84 = 1,  // WGS84 coordinates (longitude, latitude, elevation)
  COORD_ENU = 2     // Local East-North-Up coordinates relative to a reference point
};

// Point class
class TXSIMMAP_API txPoint {
 public:
  // Coordinates
  double x;  // Global X or longitude
  double y;  // Global Y or latitude
  double z;  // Global Z or elevation

  // Additional attributes
  double w;
  double h;
  double hdg;  // Heading or direction

 public:
  // Constructors
  txPoint() : x(0.0), y(0.0), z(0.0), w(0.0), h(0.0) {}
  txPoint(double px, double py, double pz) : x(px), y(py), z(pz), w(0.0), h(0.0) {}
  txPoint(double px, double py, double pz, double pw, double ph) : x(px), y(py), z(pz), w(pw), h(ph) {}
  txPoint(const txPoint& p) : x(p.x), y(p.y), z(p.z), w(p.w), h(p.h), hdg(p.hdg) {}

  // Assignment operator
  txPoint& operator=(const txPoint& p) {
    x = p.x;
    y = p.y;
    z = p.z;
    w = p.w;
    h = p.h;
    hdg = p.hdg;
    return *this;
  }

  // Equality operator
  bool operator==(const txPoint& p) {
    return fabs(x - p.x) < 1e-7 && fabs(y - p.y) < 1e-7 && fabs(z - p.z) < 1e-7 && fabs(w - p.w) < 1e-7 &&
           fabs(h - p.h) < 1e-7;
  }

  // Arithmetic operators
  const txPoint operator+(const txPoint& point) const {
    txPoint ret = *this;
    ret.x += point.x;
    ret.y += point.y;
    return ret;
  }
  const txPoint operator-(const txPoint& point) const {
    txPoint ret = *this;
    ret.x -= point.x;
    ret.y -= point.y;
    return ret;
  }
  const txPoint operator*(const double d) const {
    txPoint point = *this;
    point.x *= d;
    point.y *= d;
    return point;
  }
  const txPoint operator/(const double d) const {
    if (d != 0) {
      txPoint point = *this;
      point.x /= d;
      point.y /= d;
      return point;
    }
    return *this;
  }
};

// Vector of points
typedef std::vector<txPoint> PointVec;

// Base class for curves
class TXSIMMAP_API txCurve {
 protected:
  CoordType coordType;  // Coordinate type of the curve

 public:
  explicit txCurve(CoordType ct = COORD_XYZ) : coordType(ct) {}
  virtual ~txCurve() {}

  // Set coordinate type
  void setCoordType(CoordType ct) { coordType = ct; }

  // Get coordinate type
  CoordType getCoordType() const { return coordType; }

  // Reverse the curve
  virtual void reverse() = 0;

  // Transfer the geometry center
  // This function is used when coordType is COORD_XYZ and the geometry is in ENU
  // Set the old and new ENU centers, and all geometries will be transferred to the new coordinates
  virtual bool transfer(const txPoint& oldEnuC, const txPoint& newEnuC) = 0;

  // Transfer geometry from WGS84 to ENU
  // Only used when coordType is WGS84
  virtual bool transfer2ENU(const txPoint& enuCenter) = 0;

 public:
  // Functional operations

  // Check if the curve is empty
  virtual bool empty() const = 0;

  // Get the length of the curve
  virtual double getLength() const = 0;

  // Get a point on the curve at a specified distance
  // If distance < 0, the point is the first one
  // If distance > length, the point is the last one
  virtual txPoint getPoint(double distance) const = 0;

  // Get the start point of the curve
  virtual txPoint getStart() const = 0;

  // Get the end point of the curve
  virtual txPoint getEnd() const = 0;

  // Sample the curve at a specified interval
  virtual void sample(double interval, PointVec& points) const = 0;

  // Sample the curve with a specified angle limit and angle limit length
  virtual void sampleV2(double angleLimit, double angleLimitLen, PointVec& points) const = 0;

  // Get the pedal point on the curve at a specified point
  // Also return the distance from the starting point to the pedal point
  // coordType == COORD_XYZ -> specifiedPoint, pedalPoint: local/global
  // coordType == COORD_WGS84 -> specifiedPoint, pedalPoint: latitude, elevation, pedalZ: elevation
  virtual double getPassedDistance(const txPoint& specifiedPoint, txPoint& pedalPoint,
                                   const CoordType& ct = COORD_XYZ) const = 0;

  // Convert XY coordinates to SL (s, l) coordinates
  // XY coordinates are the same as the curve's coordinates
  // Yaw is in the range [0, 360] degrees
  virtual bool xy2sl(const double& x, const double& y, double& s, double& l, double& yaw) const = 0;

  // Convert SL (s, l) coordinates to XY coordinates
  // XY coordinates are the same as the curve's coordinates
  // Yaw is in the range [0, 360] degrees
  virtual bool sl2xy(const double& s, const double& l, double& x, double& y, double& yaw) const = 0;

  // Get the yaw angle at a specified distance along the curve
  // Yaw is in the range [0, 360] degrees
  virtual double getYaw(const double& s) const = 0;
};

// Link object class
class txLinkObject;
typedef std::shared_ptr<txLinkObject> txLinkObjectPtr;
typedef std::vector<txLinkObjectPtr> txLinkObjects;

// Class for extended data
class TXSIMMAP_API ExData {
 public:
  virtual ~ExData() {}
};

// Shared pointer for extended data
typedef std::shared_ptr<ExData> ExDataPtr;

// Enum for contact types
enum txContactType { START = 1, END = 2, UN_KNOW = 3 };

// Link object class
class TXSIMMAP_API txLinkObject : public std::enable_shared_from_this<txLinkObject> {
 public:
  txLinkObject();
  virtual ~txLinkObject() { data.reset(); }

 public:
  // Get the link type
  virtual LINK_TYPE getLinkType() const = 0;

  // Get the previous links
  bool getPrev(std::vector<std::string>& prevIds) const;

  // Get the next links
  bool getNext(std::vector<std::string>& nextIds) const;

  // Clear the previous links
  void clearPrev() { prev.clear(); }

  // Clear the next links
  void clearNext() { next.clear(); }

  // Add a previous link
  void addPrev(const std::string& preId);

  // Add a next link
  void addNext(const std::string& nextId);

  // Set extended data for the link object
  void putExData(const ExDataPtr& dataPtr) { data = dataPtr; }

  // Get the extended data for the link object
  ExDataPtr& getExData() { return data; }

  // Get the unique id
  virtual std::string getUniqueId() const;

 protected:
  std::vector<std::string> prev;  // Previous links
  std::vector<std::string> next;  // Next links

 private:
  std::shared_ptr<ExData> data;  // Extended data
};

// Curve curvature structure
struct txCurvature {
  double m_starts;     // Start distance
  double m_length;     // Length of the curve segment
  double m_curvature;  // Curvature value
};

// Vector of curve curvatures
using txCurvatureVec = std::vector<txCurvature>;

// Curve control point structure
struct txControlPoint {
  std::string m_type = "none";
  std::vector<hadmap::txPoint> m_points;
};

// OpenCRG structure
struct txOpenCrg {
  std::string m_file;
  std::string m_orientation;
  std::string m_mode;
  std::string m_purpose;
  std::string m_soffset;
  std::string m_toffset;
  std::string m_zoffset;
  std::string m_zscale;
  std::string m_hoffset;
};

// Vector of OpenCRG structures
using txOpenCrgVec = std::vector<txOpenCrg>;

// Curve slope structure
struct txSlope {
  double m_starts;  // Start distance
  double m_length;  // Length of the slope segment
  double m_slope;   // Slope value
};

// Vector of curve slopes
using txSlopeVec = std::vector<txSlope>;

// Road link structure
struct txRoadLink {
  std::string Id;
  std::string Type;
  std::string ContactPoint;
};
}  // namespace hadmap
