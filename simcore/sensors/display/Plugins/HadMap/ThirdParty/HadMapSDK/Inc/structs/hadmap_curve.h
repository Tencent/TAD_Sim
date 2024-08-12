// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/base_struct.h"

namespace hadmap {
// real curve struct

// sampling curve
// geometry is discrete points
class TXSIMMAP_API txLineCurve : public txCurve {
 private:
  struct CurveData;
  std::unique_ptr<CurveData> instancePtr;

 public:
  explicit txLineCurve(CoordType ct = COORD_ENU);

  ~txLineCurve();

 public:
  // inherit function
  bool empty() const;

  // get geometry length
  double getLength() const;

  // get specified point by distance
  txPoint getPoint(double distance) const;

  // get specified point index by distance
  int getPointIndex(double distance) const;

  // get start point
  txPoint getStart() const;

  // get end point
  txPoint getEnd() const;

  // sample
  void sample(double interval, PointVec& points) const;

  // sample
  void sample(const double interval, const double start_s, const double end_s, PointVec& points) const;

  // sample by angle limit
  void sampleV2(double angleLimit, double angleLimitLen, PointVec& points) const;

  // cut
  // begin < 0 : from first point to end
  // end < 0 : from begin to last point
  bool cut(const double& begin, const double& end, PointVec& points) const;

  // get pedal point and the distance from beginning
  // get distance between starting point to pedal point
  // ct == COORD_XYZ -> specifiedPoint, pedalPoint : local / global
  // ct == COORD_WGS84 -> specifiedPoint, pedalPoint : lat, z,pz : ele
  double getPassedDistance(const txPoint& specifiedPoint, txPoint& pedalPoint, const CoordType& ct = COORD_ENU) const;

  // xy -> sl
  // yaw -> [ 0, 360 ]
  bool xy2sl(const double& x, const double& y, double& s, double& l, double& yaw) const;

  // sl -> xy
  // yaw -> [ 0, 360 ]
  bool sl2xy(const double& s, const double& l, double& x, double& y, double& yaw) const;

  // s -> yaw
  double getYaw(const double& s) const;

 public:
  // get geometry size
  size_t size() const;

  // get point by index
  txPoint getPoint(size_t index) const;

  // get yaw
  // yaw -> [ 0, 360 ]
  double yaw(size_t index) const;

  // get all points
  void getPoints(PointVec& points) const;

 public:
  // add point to the end
  void addPoint(const txPoint& point, bool last_update = true);

  // set curve coordinates
  void setCoords(const PointVec& points);

  // clone data from curve ptr | curve instance
  void clone(const txCurve& curveIns);

  void clone(const txCurve* curvePtr);

  void clone(const txLineCurve* lineCurvePtr);

  // splice data to the end
  void splice(const txCurve* curvePtr);

  void splice(const txCurve& curveIns);

  // reverse coord data
  void reverse();

  // transfer geom
  bool transfer(const txPoint& oldEnuC, const txPoint& newEnuC);

  // transfer to enu
  bool transfer2ENU(const txPoint& enuCenter);

 private:
  // gen interval point
  // start + 1 = end
  txPoint genIntervalP(const size_t& start, const size_t& end, const double& offset) const;
};
}  // namespace hadmap
