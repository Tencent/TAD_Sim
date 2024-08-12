// Copyright (c) 2016 Tencent, Inc. All Rights Reserved
// @author elkejiang@tencent.com
// @date 2018/04/03
// @brief utils functions

#pragma once

#include <set>
#include <string>

#include "routeplan/traj_node.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_lanelink.h"
#include "structs/hadmap_road.h"

namespace planner {

class RouteUtil {
 public:
  static int nearestPoint(const hadmap::txLineCurve* curvePtr, const hadmap::txPoint& point);

  static double pointsDistance(const hadmap::txPoint& p0, const hadmap::txPoint& p1);

  static double pointsLonLatDis(const hadmap::txPoint& p0, const hadmap::txPoint& p1);

  static double angleWithDir(const hadmap::txLineCurve* curvePtr);

  static int nearestSectionIndex(const hadmap::txRoadPtr& roadPtr, const double& prog);

  static void copyRoadBaseInfo(const hadmap::txRoadPtr& oriPtr, hadmap::txRoadPtr& dstPtr);

  static std::vector<size_t> minDisLaneLinks(const std::set<hadmap::lanepkid>& fromLaneIds,
                                             std::set<hadmap::lanepkid>& usedFromLaneIds,
                                             const std::set<hadmap::lanepkid>& toLaneIds,
                                             std::set<hadmap::lanepkid>& usedToLaneIds,
                                             const hadmap::txLaneLinks& lanelinks, std::set<size_t>& usedLinkIndex);

  static void getJunctionLinks(const hadmap::txPoint& center, hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId,
                               std::vector<std::pair<hadmap::roadpkid, hadmap::roadpkid> >& linkIds,
                               std::vector<std::pair<hadmap::sectionpkid, hadmap::sectionpkid> >& linkSecIds,
                               std::vector<std::pair<hadmap::lanepkid, hadmap::lanepkid> >& linkLaneIds);

  static std::string linkIndex(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId);

  static std::string rsLinkIndex(hadmap::roadpkid fromRoadId, hadmap::roadpkid toRoadId, bool road2sec);

  static std::string secIndex(hadmap::roadpkid roadId, hadmap::sectionpkid secId);

  static std::string laneIndex(hadmap::roadpkid roadId, hadmap::sectionpkid secId, hadmap::lanepkid laneId);

  static bool buildSecRelation(hadmap::txSectionPtr& fromSecPtr, hadmap::txSectionPtr& toSecPtr);

  // vertical right -> x+, vertical left -> x-
  // horizontal forward -> y+
  static bool createOffsetGeom(const hadmap::PointVec& originGeom, double startOffset, double endOffset,
                               hadmap::PointVec& geom);

  // cut setion geom( lane geom and boundary geom )
  static bool cutSectionGeom(const hadmap::txPoint& startPoint, const hadmap::txPoint& endPoint,
                             hadmap::txSectionPtr secPtr);

  static bool isPointInPolygon(const hadmap::txPoint& p, const hadmap::PointVec& polygonGeoms);

  // intersection judgement between line and polygon
  static bool hasIntersection(const hadmap::txLineCurve* linePtr, const hadmap::txLineCurve* polygonPtr);
};
};  // namespace planner
