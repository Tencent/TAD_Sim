// Copyright 2024 Tencent Inc. All rights reserved.
// Project:  SimMapsdk
// @author   longerding@tencent
// Modify history:
//

#pragma once
#include "DataParse.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_header.h"
#include "structs/hadmap_junction.h"
#include "structs/hadmap_lanelink.h"
#include "tinyxml2.h"
#include "types/map_types.h"
#ifndef M_PI
#  define M_PI 3.1415926
#endif  // !M_PI
struct PlanView {
  std::string type;
  double hdg;
  double length;
  double s;
  double x;
  double y;
  // arc double
  double arc;
  int startIndex;
  int endIndex;
  // param ploy
  double aU = 0;
  double bU = 0;
  double cU = 0;
  double dU = 0;
  double aV = 0;
  double bV = 0;
  double cV = 0;
  double dV = 0;
};

struct ele {
  double s = 0;
  double a = 0;
  double b = 0;
  double c = 0;
  double d = 0;
};

namespace hadmap {
struct LaneMark {
  std::string type;
  std::string color;
  std::string laneChange;
};
// OpenDriveOutput class
class TXSIMMAP_API OpenDriveOutput {
 public:
  // Initialize the file path
  bool initFilePath(const char* filePath, bool replace = false);

  // Insert functions
  bool insertHeader(const hadmap::txPoint refPoint);
  bool insertRoads(const hadmap::txRoads& roads);
  bool insertObjects(const hadmap::txObjects& objects, const hadmap::txRoads& roads);
  bool insertLaneLinks(const hadmap::txLaneLinks& lanelinks);

  // Update functions
  bool updateRoads(const hadmap::txRoads& roads);
  bool updateLaneLinks(const hadmap::txLaneLinks& lanelinks);
  bool updateObjects(const hadmap::txObjects& lanelinks, const hadmap::txRoads& roads);

  // Delete functions
  bool deleteRoads(const hadmap::txRoads& roads);
  bool deleteLanelinks(const hadmap::txLaneLinks& lanelinks);
  bool deleteObjects(const hadmap::txObjects& objects);

  // Split laneLinks
  bool specLaneLinks(const roadpkid roadid, hadmap::txLaneLinks& lanelinks);

  // Check is init
  bool getIsInit() { return m_isInit; }

  // Save map file
  int saveFile(const char* filePath);

 private:
  // Insert one road
  tinyxml2::XMLElement* insertOneRoad(const hadmap::txRoadPtr& it);

  // Insert one lane link
  tinyxml2::XMLElement* insertOneLaneLink(const hadmap::txLaneLinkPtr& it);

  // Insert opencrg
  tinyxml2::XMLElement* insertCrgs(const hadmap::txOpenCrgVec& crgs, double roadlength);

  // insert one lane
  tinyxml2::XMLElement* insertOneLane(const hadmap::txLanePtr lanePtr);

  // get lane width
  double GetLaneWidth(const hadmap::txLanePtr& lane);

  // Convert road type to string
  const char* fromatRoadType(const hadmap::ROAD_TYPE& type);

  // Convert curve to points
  void CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points);

  // Calculate the distance between two points
  double distanceTwoPoint(txPoint a, txPoint b);

  // Fit the center of the points using least squares
  void FitCenterByLeastSquares(std::vector<hadmap::txPoint> mapPoint, double& radius,
                               std::vector<double>& radiusVec) const;

  // Get elevation from points
  double getEle(const PointVec vec, std::vector<ele>& elevec);

  // Get elevation from control points
  int getEleFromControl(const PointVec vec, std::vector<ele>& elevec);

  // Get plan view link from points
  bool getPlanViewLink(const std::vector<hadmap::txPoint> points, std::vector<PlanView>& planViewVec);

  // Get plan view link from control point and points
  bool getPlanViewLink(const txControlPoint controlPoint, const std::vector<hadmap::txPoint> points,
                       std::vector<PlanView>& planViewVec);

  // Get plan view from points
  bool getPlanView(const std::vector<hadmap::txPoint> mapPoint, std::vector<PlanView>& planviewVec);

  // Get arc and length from headings and length
  void getArcAndLength(double hdg1, double hdg2, double nextHdg, double length, double& arc, double& arcLength);

  // Convert longitude and latitude to x and y
  void lonlat2xy(double& x, double& y);

  // Transform angle using center point and radius
  txPoint transformAngle(const txPoint centerPoint, const double rad, const txPoint point);

  // Rotate points by angle
  void rotate(double angle, double& x, double& y);

  // Convert control points to polyline
  void controlPointsToPloy3(const std::vector<txPoint> pts, ele& _ele);

  // Convert control points to parametric polyline
  void controlPointsToParamPloy(const std::vector<hadmap::txPoint> pts, PlanView& plan);

  // Get heading from two points
  double getHdg(const hadmap::txPoint frontPoint, const hadmap::txPoint nextPoint);

  // XML document
  tinyxml2::XMLDocument* doc;

  // Reference point
  txPoint m_refPoint;

  // File path
  const char* m_filePath;

  // Init member
  bool m_isInit = false;
};
}  // namespace hadmap
