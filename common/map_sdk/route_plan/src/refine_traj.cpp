// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routeplan/refine_traj.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

#include "common/log.h"
#include "routeplan/node_opt.h"
#include "routeplan/route_utils.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_lanelink.h"

using namespace hadmap;

namespace planner {
bool isJunction(const txLaneLinks& lanelinks) {
  if (lanelinks.empty()) return false;
  for (size_t i = 0; i < lanelinks.size(); ++i) {
    if (lanelinks[i]->getGeometry() != NULL) {
      const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(lanelinks[i]->getGeometry());
      if (NULL == curvePtr) return false;
      double dis = curvePtr->getLength();
      if (dis < 2.0) return false;
    }
  }
  return true;
}

void judgeAdjRoadLinkTypeByYaw(RoadInfoArray& traj) {
  if (traj.empty()) {
    txlog::error("trajectory empty");
    return;
  }

  for (auto trajItr = traj.begin(); trajItr != (traj.end() - 1); ++trajItr) {
    hadmap::roadpkid fromRoadId = (*trajItr).roadId;
    hadmap::roadpkid toRoadId = (*(trajItr + 1)).roadId;

    ROAD_LINK_TYPE road_link_type = DEFAULT;

    ROAD_TYPE fromRoadType, toRoadType;
    txRoadPtr fromRoadPtr, toRoadPtr;
    NodeOpt::getInstance()->getRoad(NodeId(fromRoadId), false, fromRoadPtr);
    NodeOpt::getInstance()->getRoad(NodeId(toRoadId), false, toRoadPtr);
    fromRoadType = fromRoadPtr->getRoadType();
    toRoadType = toRoadPtr->getRoadType();

    if (fromRoadType == ROAD_TYPE_Roundabout || toRoadType == ROAD_TYPE_Roundabout) {
      if (fromRoadType == ROAD_TYPE_Roundabout && toRoadType == ROAD_TYPE_Roundabout) {
        road_link_type = STRAIGHT;
      } else if (fromRoadType == ROAD_TYPE_Roundabout) {
        road_link_type = TURN_RIGHT;
      } else {
        road_link_type = TURN_LEFT;
      }
    } else if (fromRoadType != ROAD_TYPE_GenerapRamp && toRoadType == ROAD_TYPE_GenerapRamp) {
      road_link_type = TURN_RIGHT;
    } else if (fromRoadType == ROAD_TYPE_GenerapRamp && toRoadType != ROAD_TYPE_GenerapRamp) {
      road_link_type = TURN_LEFT;
    } else {
      txLaneLinks laneLinks;
      if (NodeOpt::getInstance()->getLaneLinks(fromRoadId, toRoadId, laneLinks)) {
        if (laneLinks.empty() || !isJunction(laneLinks)) {
          /*
          char info[128];
          sprintf( info, "no lanelinks from %lld to %lld", fromRoadId, toRoadId );
          txlog::info( info );
          */
        } else {
          hadmap::txObjects trafficLights;
          bool hasTrafficLight = NodeOpt::getInstance()->getTrafficLights(
              laneLinks.front()->fromRoadId(), laneLinks.front()->fromSectionId(), laneLinks.front()->fromLaneId(),
              trafficLights);

          const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(laneLinks.front()->getGeometry());
          double angle = RouteUtil::angleWithDir(curvePtr);

          const double angleOffset = 15.0;
          const double uturnAngle = 150.0;
          if (fromRoadId == toRoadId || fabs(angle) > uturnAngle) {
            txlog::info("U-Turn");
            road_link_type = hasTrafficLight ? U_TURN_WITH_TRAFFIC_LIGHT : U_TURN;
          } else if (angle < -angleOffset) {
            txlog::info("Left");
            road_link_type = hasTrafficLight ? LEFT_WITH_TRAFFIC_LIGHT : TURN_LEFT;
          } else if (-angleOffset <= angle && angle <= angleOffset) {
            txlog::info("Straight");
            road_link_type = hasTrafficLight ? STRAIGHT_WITH_TRAFFIC_LIGHT : STRAIGHT;
          } else {
            txlog::info("Right");
            road_link_type = TURN_RIGHT;
          }
        }
      }
    }
    trajItr->attribute = road_link_type;
  }
}

void TrajRefiner::mockJunctionToRoad(RoadInfoArray& traj) {
  if (traj.size() <= 1) {
    return;
  } else {
    for (auto itr = traj.begin(); itr != traj.end() - 1; ++itr) {
      hadmap::roadpkid curRoadId = (*itr).roadId;
      hadmap::roadpkid nextRoadId = (*(std::next(itr))).roadId;

      txLaneLinks lanelinks;
      if (NodeOpt::getInstance()->getLaneLinks(curRoadId, nextRoadId, lanelinks)) {
        if (isJunction(lanelinks)) {
          // static int64_t mock_road_id = -1;
          RoadInfo mockRoad;
          mockRoad.roadId = -1;
          mockRoad.fromRId = curRoadId;
          mockRoad.toRId = nextRoadId;

          double l = 0.0;
          for (auto& linkPtr : lanelinks) l += linkPtr->getGeometry()->getLength();
          mockRoad.length = l / lanelinks.size();

          itr = traj.insert(std::next(itr), mockRoad);
        }
      }
    }
  }
}

/*
 *@ Description: process with reverse road, the \
 *      geom (including section order, line boundaryordr, line .etc) of \
 *      which should be processed(reverse).
 */
void TrajRefiner::roadProc(RoadInfoArray& traj) {
  if (traj.size() <= 0) {
    txlog::error("trajectory empty, failed!");
    return;
  }
  if (traj.size() == 1) return;
  for (size_t i = 0; i < traj.size(); ++i) {
    if (i == traj.size() - 1) {
      hadmap::roadpkid frontRoadId = traj[i - 1].roadId;
      hadmap::roadpkid curRoadId = traj[i].roadId;
      txLaneLinks curLinks;
      if (NodeOpt::getInstance()->getLaneLinks(frontRoadId, curRoadId, curLinks)) {
        int flag = 1;
        if (curLinks.front()->toLaneId() > 0) {
          flag = -1;
          traj[i].reverseFlag = true;
        }
        std::set<hadmap::lanepkid> enableLaneId;
        for (size_t j = 0; j < curLinks.size(); ++j) enableLaneId.insert(curLinks[j]->toLaneId() * flag);
        for (auto itr = enableLaneId.rbegin(); itr != enableLaneId.rend(); ++itr)
          traj[i].endEnableLanes.push_back(*itr);
      } else {
        throw std::runtime_error("roadProc Error, can not get links");
      }
    } else {
      hadmap::roadpkid curRoadId = traj[i].roadId;
      hadmap::roadpkid nextRoadId = traj[i + 1].roadId;
      txLaneLinks curLinks;
      if (NodeOpt::getInstance()->getLaneLinks(curRoadId, nextRoadId, curLinks)) {
        int flag = 1;
        if (curLinks.front()->fromLaneId() > 0) {
          flag = -1;
          traj[i].reverseFlag = true;
        }
        std::set<hadmap::lanepkid> enableLaneId;
        for (size_t j = 0; j < curLinks.size(); ++j) enableLaneId.insert(curLinks[j]->fromLaneId() * flag);
        for (auto itr = enableLaneId.rbegin(); itr != enableLaneId.rend(); ++itr)
          traj[i].endEnableLanes.push_back(*itr);
      } else {
        throw std::runtime_error("roadProc Error, can not get links");
      }
    }
  }
}

void TrajRefiner::cutRoad(const txPoint& start, const txPoint& end, RoadInfoArray& traj) {
  if (traj.size() == 0) return;

  // cut begin road geom
  hadmap::roadpkid startRoadId = traj.front().roadId;
  txRoadPtr curRoadPtr;
  if (NodeOpt::getInstance()->getRoad(startRoadId, false, curRoadPtr)) {
    // reverse road data if necessary
    txLaneLinks lanelinks;
    if (traj.size() > 1) NodeOpt::getInstance()->getLaneLinks(startRoadId, traj[1].roadId, lanelinks);
    bool needReverse = false;
    for (size_t i = 0; i < lanelinks.size(); ++i) {
      if (lanelinks[i]->fromLaneId() > 0) {
        needReverse = true;
        break;
      }
    }
    if (needReverse) curRoadPtr->reverse();

    const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(curRoadPtr->getGeometry());
    if (NULL == curvePtr) {
      traj.front().begin = 1.0;
    } else {
      int startNearestIndex = RouteUtil::nearestPoint(curvePtr, start);
      // startNearestIndex = startNearestIndex > 10 ? startNearestIndex - 10 : 0;
      txLineCurve cutCurve(COORD_WGS84);
      for (size_t i = startNearestIndex; i < curvePtr->size(); ++i)
        cutCurve.addPoint(curvePtr->getPoint(i), i == (curvePtr->size() - 1));
      // set traj start node length
      traj.front().begin = 1.0 - cutCurve.getLength() / curvePtr->getLength();
    }
  }

  // cut end road geom
  hadmap::roadpkid endRoadId = traj.back().roadId;
  if (NodeOpt::getInstance()->getRoad(endRoadId, false, curRoadPtr)) {
    // reverse road data if necessary
    txLaneLinks lanelinks;
    if (traj.size() > 1) NodeOpt::getInstance()->getLaneLinks(traj[traj.size() - 2].roadId, endRoadId, lanelinks);
    bool needReverse = false;
    for (size_t i = 0; i < lanelinks.size(); ++i) {
      if (lanelinks[i]->toLaneId() > 0) {
        needReverse = true;
        break;
      }
    }
    if (needReverse) curRoadPtr->reverse();

    const txLineCurve* curvePtr = dynamic_cast<const txLineCurve*>(curRoadPtr->getGeometry());
    if (NULL == curvePtr) {
      traj.back().end = 0.0;
    } else {
      int endNearestIndex = RouteUtil::nearestPoint(curvePtr, end);
      // endNearestIndex = endNearestIndex < int( curvePtr->size() ) - 10 ? endNearestIndex : curvePtr->size() - 1;
      txLineCurve endCutCurve(COORD_WGS84);
      for (size_t i = 0; i <= (size_t)endNearestIndex; ++i)
        endCutCurve.addPoint(curvePtr->getPoint(i), i == endNearestIndex);
      traj.back().end = endCutCurve.getLength() / curvePtr->getLength();
    }
  }
}

void TrajRefiner::judgeAdjRoadLinkType(RoadInfoArray& traj) { judgeAdjRoadLinkTypeByYaw(traj); }

void TrajRefiner::setJunctionLinkType(RoadInfoArray& traj) {
  if (traj.empty()) {
    txlog::error("trajectory empty");
    return;
  }
  /*
  for ( auto trajItr = traj.begin(); trajItr != ( traj.end() - 1 ); ++ trajItr )
  {
          txRoad::txRoadPtr &pCurRoad = *trajItr;
          txRoad::txRoadPtr &pNextRoad = *( trajItr + 1 );

          // pNextRoad is junction
          if ( pNextRoad->getId() < 0 )
          {
             pNextRoad->setAttribute( pCurRoad->getAttribute() );
          }
  }
  */
  for (size_t i = 0; i < traj.size() - 1; ++i) {
    if (traj[i + 1].roadId < 0) traj[i + 1].attribute = traj[i].attribute;
  }
}

}  // namespace planner
