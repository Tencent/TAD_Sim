// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routingmap/driving_track.h"

#include <exception>
#include <iomanip>
#include <iostream>

#include "routingmap/routing_log.h"
#include "routingmap/routing_utils.h"
#include "structs/hadmap_junctionroad.h"
#include "structs/hadmap_road.h"
#include "structs/hadmap_section.h"

namespace hadmap {
DrivingTrack::DrivingTrack(txMapInterfacePtr imp)
    : iMapPtr(imp), lastPos(0.0, 0.0, 0.0), curNodeIndex(0), curNodePassedDis(0.0) {}

DrivingTrack::~DrivingTrack() {}

bool DrivingTrack::setRoute(const txRoute& r, const txPoint& startPos) {
  route.assign(r.begin(), r.end());
  lastPos = startPos;
  curNodeIndex = 0;
  curNodePassedDis = route.front().getStartRange() * route.front().getLength();
  return true;
}

void DrivingTrack::updateTrack(const txPoint& pos) {
  txMapInterface::txProjectionConstPtr proj_ptr = iMapPtr->getProjection();
  txPoint enu_pos;
  proj_ptr->lonlat2enu(pos, enu_pos);
  if (cur_lane_ptr_ == NULL) {
    txLanes lanes = iMapPtr->getLanes(enu_pos, 5.0);
    double l = 1000.0;
    for (auto& lane_ptr : lanes) {
      if (lane_ptr->getRoadId() == route[curNodeIndex].getId() && lane_ptr->getId() < 0) {
        double ts, tl;
        if (lane_ptr->xy2sl(enu_pos.x, enu_pos.y, ts, tl)) {
          if (l > fabs(tl)) {
            cur_lane_ptr_ = lane_ptr;
            l = tl;
          }
        }
      }
    }
    if (cur_lane_ptr_ == NULL) throw std::runtime_error("cur pos error, can not locate on map");
  } else {
    double s, l;
    if (cur_lane_ptr_->xy2sl(enu_pos.x, enu_pos.y, s, l)) {
      if (l > 1.5) {
        txLanePtr left_lane_ptr = iMapPtr->getLeftLane(cur_lane_ptr_);
        if (left_lane_ptr == NULL) {
          if (l > 3.0) throw std::runtime_error("can not find left lane");
        } else {
          double ts, tl;
          if (left_lane_ptr->xy2sl(enu_pos.x, enu_pos.y, ts, tl) && fabs(tl) < fabs(l)) cur_lane_ptr_ = left_lane_ptr;
        }
      } else if (l < -1.5) {
        txLanePtr right_lane_ptr = iMapPtr->getRightLane(cur_lane_ptr_);
        if (right_lane_ptr == NULL) {
          if (l < -3.0) throw std::runtime_error("can not find right lane");
        } else {
          double ts, tl;
          if (right_lane_ptr->xy2sl(enu_pos.x, enu_pos.y, ts, tl) && fabs(tl) < fabs(l)) cur_lane_ptr_ = right_lane_ptr;
        }
      }
    }
    if (!cur_lane_ptr_->xy2sl(enu_pos.x, enu_pos.y, s, l) || s > cur_lane_ptr_->getLength()) {
      txLanes next_lanes = iMapPtr->getNextLanes(cur_lane_ptr_);
      if (next_lanes.empty()) throw std::runtime_error("get next lanes error");
      bool yaw_flag = true;
      for (auto& next_lane_ptr : next_lanes) {
        if (next_lane_ptr->getRoadId() == cur_lane_ptr_->getRoadId()) {
          cur_lane_ptr_ = next_lane_ptr;
          yaw_flag = false;
          break;
        } else if (next_lane_ptr->getRoadId() == route[curNodeIndex + 1].getId()) {
          cur_lane_ptr_ = next_lane_ptr;
          yaw_flag = false;
          curNodeIndex += 1;
          break;
        }
      }
      if (yaw_flag) throw std::runtime_error("already yaw");
    }
    updatePassedDisInCurNode(cur_lane_ptr_, enu_pos);
  }
}

bool DrivingTrack::setPos(const txPoint& pos) {
  std::cout << std::setprecision(10) << pos.x << "," << pos.y << " " << curNodeIndex << std::endl;
  double dis = RoutingUtils::pointsDisWGS84(lastPos, pos);
  for (auto itr = markDis.begin(); itr != markDis.end(); ++itr) itr->second += dis;
  lastPos = pos;

  updateTrack(pos);
  return true;
  /*
  txMapInterface::txProjectionConstPtr projPtr = iMapPtr->getProjection();
  txPoint enuPos;
  projPtr->lonlat2enu( pos, enuPos );
  txLanes lanes = iMapPtr->getLanes( enuPos, 6.0 );
  if ( lanes.empty() ){
          throw std::runtime_error( "cur pos error, cant locate on map" );}
  else
  {
          txLanePtr curLanePtr, nextLanePtr;
          double s, l = 1000.0;
          for ( size_t i = 0; i < lanes.size(); ++ i )
          {
                  if ( lanes[i]->getRoadId() == route[ curNodeIndex ].getId() && \
                                  lanes[i]->getId() < 0 )
                  {
                          double ts, tl;
                          if ( lanes[i]->xy2sl( enuPos.x, enuPos.y, ts, tl ) )
                          {
                                  if ( l > fabs( tl ) )
                                  {
                                          if ( curLanePtr != NULL && \
                                                          curLanePtr->getSectionId() == lanes[i]->getSectionId() )
                                          {
                                                  l = fabs( tl );
                                                  s = ts;
                                                  curLanePtr = lanes[i];
                                                  continue;
                                          }
                                          if ( curLanePtr != NULL && ts > lanes[i]->getLength() ){
                                                  continue;}
                                          l = fabs( tl );
                                          s = ts;
                                          curLanePtr = lanes[i];
                                  }
                          }
                  }
                  if ( curNodeIndex + 1 < route.size() && \
                                  lanes[i]->getRoadId() == route[ curNodeIndex + 1 ].getId() && \
                                          lanes[i]->getId() < 0 )
                          nextLanePtr = lanes[i];
          }
          if ( curLanePtr == NULL ){
                  throw std::runtime_error( "already yaw" );}
          else
          {
                  if ( s > curLanePtr->getLength() )
                  {
                          txLanes nextLanes = iMapPtr->getNextLanes( curLanePtr );
                          if ( !nextLanePtr && nextLanes.front()->getRoadId() == curLanePtr->getRoadId() )
                          {
                                  curLanePtr = nextLanes.front();
                                  passedDisInCurNode( curLanePtr, enuPos );
                                  lastPos = pos;
                                  return true;
                          }
                          else
                          {
                                  if ( nextLanePtr )
                                  {
                                          curNodeIndex += 1;
                                          passedDisInCurNode( nextLanePtr, enuPos );
                                          lastPos = pos;
                                          return true;
                                  }
                                  else
                                          return false;}
                          }
                  }
                  else
                  {
                          passedDisInCurNode( curLanePtr, enuPos );
                          lastPos = pos;
                          return true;
                  }
          }
  }
  */
}

void DrivingTrack::updatePassedDisInCurNode(txLanePtr curLanePtr, const txPoint& enuPos) {
  double s, l;
  if (!curLanePtr->xy2sl(enuPos.x, enuPos.y, s, l)) s = 0.0;
  curNodePassedDis = 0.0;
  txRoadPtr curRoadPtr = curLanePtr->getSection()->getRoad();
  txSections& curSecs = curRoadPtr->getSections();
  for (auto& secPtr : curSecs) {
    if (secPtr->getId() == curLanePtr->getSectionId()) {
      break;
    } else {
      curNodePassedDis += secPtr->getLength();
    }
  }
  curNodePassedDis += s;
}

bool DrivingTrack::markPassedCenter(const std::string& targetStr) {
  if (markDis.find(targetStr) == markDis.end()) {
    markDis.insert(std::make_pair(targetStr, 0.0));
  } else {
    markDis[targetStr] = 0.0;
  }
  return true;
}

txRouteNode DrivingTrack::curRouteNode() {
  if (curNodeIndex < route.size()) {
    return route[curNodeIndex];
  } else {
    throw std::runtime_error("node index error");
  }
}

bool DrivingTrack::frontRouteNode(txRouteNode& front_route_node) {
  if (curNodeIndex == 0) {
    return false;
  } else {
    front_route_node = route[curNodeIndex - 1];
    return true;
  }
}

bool DrivingTrack::backRouteNode(txRouteNode& back_route_node) {
  if (curNodeIndex == route.size() - 1) {
    return false;
  } else {
    back_route_node = route[curNodeIndex + 1];
    return true;
  }
}

void DrivingTrack::routeAhead(double distance, txRoute& r) {
  if (distance < 0.0) return;
  r.clear();
  for (size_t i = curNodeIndex; i < route.size(); ++i) {
    r.push_back(route[i]);
    if (i == curNodeIndex) {
      distance -= (route[i].getEndRange() * route[i].getLength() - curNodePassedDis);
    } else {
      distance -= (route[i].getEndRange() - route[i].getStartRange()) * route[i].getLength();
    }
    if (distance < 0.0) {
      break;
    }
  }
}

void DrivingTrack::routeBehind(double distance, txRoute& r) {
  if (distance < 0.0) return;
  r.clear();
  if (curNodeIndex == 0) {
    return;
  } else {
    distance -= curNodePassedDis;
    int i = curNodeIndex - 1;
    while (i >= 0 && distance > 0.0) {
      r.push_back(route[i]);
      distance -= (route[i].getEndRange() - route[i].getStartRange()) * route[i].getLength();
      i--;
    }
  }
}

txPoint DrivingTrack::posAhead(double distance) {
  if (distance < 1.0) {
    return lastPos;
  } else {
    txPoint tagP = lastPos;
    txMapInterface::txProjectionConstPtr projPtr = iMapPtr->getProjection();
    for (size_t i = curNodeIndex; i < route.size(); ++i) {
      txRoadPtr curRoadPtr = iMapPtr->getRoadById(route[i].getId());
      if (!curRoadPtr) {
        break;
      } else {
        double offset = distance;
        if (i == curNodeIndex) {
          distance -= route[i].getEndRange() * route[i].getLength() - curNodePassedDis;
        } else {
          distance -= (route[i].getEndRange() - route[i].getStartRange()) * route[i].getLength();
        }
        if (distance > 0.0) {
          if (i + 1 == route.size()) {
            txPoint p;
            curRoadPtr->getGeometry()->sl2xy(route[i].getEndRange() * route[i].getLength(), 0.0, p.x, p.y, p.z);
            p.z = curRoadPtr->getGeometry()->getEnd().z;
            projPtr->enu2lonlat(p, tagP);
          } else {
            txPoint p = curRoadPtr->getGeometry()->getEnd();
            projPtr->enu2lonlat(p, tagP);
          }
        } else {
          if (i == curNodeIndex) offset += curNodePassedDis;
          txPoint p;
          curRoadPtr->getGeometry()->sl2xy(offset, 0.0, p.x, p.y, p.z);
          p.z = curRoadPtr->getGeometry()->getEnd().z;
          projPtr->enu2lonlat(p, tagP);
          break;
        }
      }
    }
    return tagP;
  }
}

double DrivingTrack::passedDisInCurNode() { return curNodePassedDis; }

double DrivingTrack::passedDis(const std::string& targetStr) {
  if (markDis.find(targetStr) == markDis.end()) throw std::runtime_error("get passed dis error, target str not mark");
  return markDis[targetStr];
}

double DrivingTrack::disToRoad(const roadpkid& rId) {
  if (route[curNodeIndex].getId() == rId) return 0.0;
  double dis = route[curNodeIndex].getLength() - curNodePassedDis;
  for (size_t i = curNodeIndex + 1; i < route.size(); ++i) {
    if (route[i].getId() == rId) {
      return dis;
    } else {
      dis += route[i].getLength();
    }
  }
  // unable to find specified road on left route
  return -1.0;
}

bool DrivingTrack::nextJunc(junctionpkid& jId, roadpkid& jrId) {
  for (size_t i = curNodeIndex + 1; i < route.size(); ++i) {
    if (route[i].getRouteType() == txRouteNode::JUNCTION_ROUTE) {
      txRoadPtr roadPtr = iMapPtr->getRoadById(route[i].getId());
      if (roadPtr) {
        txJunctionRoadPtr juncRoadPtr = std::dynamic_pointer_cast<txJunctionRoad>(roadPtr);
        jId = juncRoadPtr->getJuncId();
        jrId = juncRoadPtr->getId();
        return true;
      } else {
        return false;
      }
    }
  }
  return false;
}
}  // namespace hadmap
