// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_route_path_manager.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Routing)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

RoutePathManager::RoutePathManager() { Reset(); }

void RoutePathManager::Reset() TX_NOEXCEPT { m_curPathPlanIndex = std::begin(roadInfoArray); }

void RoutePathManager::Clear() TX_NOEXCEPT {
  roadInfoArray.clear();
  Reset();
}

Base::txSize RoutePathManager::Size() const TX_NOEXCEPT { return roadInfoArray.size(); }

Base::txDiff_t RoutePathManager::Index() const TX_NOEXCEPT { return m_curPathPlanIndex - std::begin(roadInfoArray); }

Base::txDiff_t RoutePathManager::Relax() const TX_NOEXCEPT { return roadInfoArray.end() - m_curPathPlanIndex; }
Base::txBool RoutePathManager::IsEmpty() const TX_NOEXCEPT { return 0 == Size(); }
Base::txBool RoutePathManager::IsValid() const TX_NOEXCEPT {
  return m_curPathPlanIndex != std::end(roadInfoArray) && _isValid;
}
Base::txBool RoutePathManager::HasNextRoad(Base::txDiff_t const idx) TX_NOEXCEPT {
  // LogInfo << TXST_TRACE_COND_NAME(IsValid(), IsValid()) << ", std::distance(std::begin(roadInfoArray),
  // m_curPathPlanIndex) = " << (std::distance(std::begin(roadInfoArray), m_curPathPlanIndex));
  return (IsValid()) && ((std::distance(std::begin(roadInfoArray), m_curPathPlanIndex) + idx) < Size());
}

Base::txSignedRoadID RoutePathManager::CurRoadId() const TX_NOEXCEPT {
  if (IsValid()) {
    return (*m_curPathPlanIndex).roadId;
  } else {
    return 0;
  }
}

Base::txSignedRoadID TX_MARK("+") RoutePathManager::NextRoadId(Base::txDiff_t const idx) TX_NOEXCEPT {
  if (HasNextRoad(idx)) {
    /*LogInfo << "(*std::next(m_curPathPlanIndex, idx)).roadId = " << ((*std::next(m_curPathPlanIndex, idx)).roadId);*/
    return (*std::next(m_curPathPlanIndex, idx)).roadId TX_MARK("+,eg:48,49");
  } else {
    return 0;
  }
}

planner::RoadInfoArray::iterator RoutePathManager::Next(Base::txDiff_t const idx) TX_NOEXCEPT {
  if (HasNextRoad(idx)) {
    return (m_curPathPlanIndex = std::next(m_curPathPlanIndex, idx));
  } else {
    return m_curPathPlanIndex;
  }
}

Base::txString RoutePathManager::RoadInfoStr(const planner::RoadInfo& refRoadInfo) const TX_NOEXCEPT {
  if (refRoadInfo.roadId > 0) {
    return (boost::format("(roadId=%1%, fromRId=%2%, toRId=%3%) ") % refRoadInfo.roadId % -1 % -1).str();
  } else {
    return (boost::format("(roadId=%1%, fromRId=%2%, toRId=%3%) ") % refRoadInfo.roadId % refRoadInfo.fromRId %
            refRoadInfo.toRId)
        .str();
  }
}

Base::txString RoutePathManager::Str() const TX_NOEXCEPT {
  std::stringstream ss;
  // 格式化拼接字符串
  ss << "RoutePathManager {";
  ss << "m_curPathPlanIndex = " << (m_curPathPlanIndex - std::begin(roadInfoArray)) << " ";
  for (const auto& ref : roadInfoArray) {
    ss << RoadInfoStr(ref);
  }
  ss << ", m_iGoalLaneId = " << m_iGoalLaneId;
  ss << "}";
  return ss.str();
}

WayPoints PrevPointArea(const std::vector<WayPoints>& vec_waypoint_area, const int idx) TX_NOEXCEPT {
  // 返回第一个点区域对象
  for (int i = idx; i >= 0; --i) {
    if (vec_waypoint_area[i].IsPointArea()) {
      return vec_waypoint_area[i];
    }
  }
  return vec_waypoint_area[0];
}

WayPoints NextPointArea(const std::vector<WayPoints>& vec_waypoint_area, const int idx) TX_NOEXCEPT {
  // 返回idx之后的第一个点区域对象
  for (int i = idx; i < vec_waypoint_area.size(); ++i) {
    if (vec_waypoint_area[i].IsPointArea()) {
      return vec_waypoint_area[i];
    }
  }
  return vec_waypoint_area[vec_waypoint_area.size() - 1];
}

Base::txBool cmp(const std::pair<Base::txInt, hadmap::txLaneLinkPtr>& a,
                 const std::pair<Base::txInt, hadmap::txLaneLinkPtr>& b) TX_NOEXCEPT {
  return a.first < b.first;
}

void RoutePathManager::SetWayPoints(const std::vector<hadmap::txPoint>& pointList) TX_NOEXCEPT {
  if (pointList.size() > 1) {
    // 先清空
    m_vec_waypoint_area.clear();
    LogWarn << "call SetWayPoints.";
    for (int i = 0; i < roadInfoArray.size(); ++i) {
      const auto curRoadId = roadInfoArray[i].roadId;
      // 若当前道路ID大于0
      if (curRoadId > 0) {
        TX_MARK("lane");
        const auto curRoadPtr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(curRoadId);
        const int nSectionSize = curRoadPtr->getSections().size();
        // 遍历sections并添加waypoint
        for (int s = 0; s < nSectionSize; ++s) {
          WayPoints wpMgr;
          wpMgr.Initialize(curRoadId, s);
          m_vec_waypoint_area.push_back(wpMgr);
        }
      } else {
        // lanlink则另外添加
        TX_MARK("lane link");
        if (m_vec_waypoint_area.size() > 0) {
          WayPoints wpMgr;
          wpMgr.Initialize(-1, Base::txLaneUId(), Base::txLaneUId());
          m_vec_waypoint_area.push_back(wpMgr);
        } else {
          _isValid = false;
        }
      }
    }

    std::vector<WayPoints> tmp_vec_waypoint_area;

    for (const auto& refWayPoint : pointList) {
      WayPoints wayPointArea;
      Coord::txWGS84 wayPt(__2d_Alt__(refWayPoint));
      wayPointArea.Initialize(wayPt);
      tmp_vec_waypoint_area.push_back(wayPointArea);
      LogWarn << "way point " << Utils::ToString(wayPointArea.GetLocLaneUid());
    }
    {
      TX_MARK("erase prev start point road.section");
      const Base::txLaneUId& refWayPointStartPtUid = tmp_vec_waypoint_area[0].GetLocLaneUid();
      LogWarn << "[start_point_road_section] " << Utils::ToString(refWayPointStartPtUid);
      auto itr = m_vec_waypoint_area.begin();
      // 遍历路径点集合，按照路径点位置信息进行匹配和删除操作
      for (; itr != m_vec_waypoint_area.end();) {
        const Base::txLaneUId& refRouteLaneUid = (*itr).GetLocLaneUid();
        // 判断当前路径点是否为起始点，如果不是则删除该路径点
        if (!(refRouteLaneUid.roadId == refWayPointStartPtUid.roadId &&
              refRouteLaneUid.sectionId == refWayPointStartPtUid.sectionId)) {
          itr = m_vec_waypoint_area.erase(itr);
        } else {
          break;
        }
      }
    }
    {
      TX_MARK("erase next end point road.section");
      // 获取临时向量中的最后一个点的位置信息，用于后续路径删除操作
      const Base::txLaneUId& refWayPointEndPtUid =
          tmp_vec_waypoint_area[tmp_vec_waypoint_area.size() - 1].GetLocLaneUid();
      LogWarn << "[end_point_road_section] " << Utils::ToString(refWayPointEndPtUid);
      // 遍历路径点向量，按照路径点位置信息进行匹配和删除操作
      auto itr = m_vec_waypoint_area.begin();
      Base::txBool bEndFlag = false;
      for (; itr != m_vec_waypoint_area.end();) {
        // 如果bEndFlag为true
        if (bEndFlag) {
          itr = m_vec_waypoint_area.erase(itr);
        } else {
          // 获取当前路径区域的车道ID
          const Base::txLaneUId& refRouteLaneUid = (*itr).GetLocLaneUid();
          // 如果当前路径区域的车道ID等于refWayPointEndPtUid.roadId和refWayPointEndPtUid.sectionId
          if ((refRouteLaneUid.roadId == refWayPointEndPtUid.roadId &&
               refRouteLaneUid.sectionId == refWayPointEndPtUid.sectionId)) {
            bEndFlag = true;
          }
          ++itr;
        }
      }
    }
#if 1
    std::map<std::pair<Base::txRoadID, Base::txSectionID>, std::vector<WayPoints> > tmp_pointAreasInSameSection;
    // 提取同一道路和同一区域的路线点
    for (const auto& refWayPoint : tmp_vec_waypoint_area) {
      // 遍历m_vec_waypoint_area
      for (int i = 0; i < m_vec_waypoint_area.size(); ++i) {
        if (!m_vec_waypoint_area[i].IsLaneLinkArea()) {
          // 获取当前路径区域的车道ID
          const Base::txLaneUId& refRouteLaneUid = m_vec_waypoint_area[i].GetLocLaneUid();
          const Base::txLaneUId& refWayPointLaneUid = refWayPoint.GetLocLaneUid();
          // 如果当前路径区域的车道ID等于refWayPoint的车道ID
          if (refRouteLaneUid.roadId == refWayPointLaneUid.roadId &&
              refRouteLaneUid.sectionId == refWayPointLaneUid.sectionId) {
            tmp_pointAreasInSameSection[std::make_pair(refRouteLaneUid.roadId, refRouteLaneUid.sectionId)].push_back(
                refWayPoint);
          }
        }
      }
    }

    // 对已有路线点进行合并
    for (const auto& refPointAreaVec : tmp_pointAreasInSameSection) {
      Base::txRoadID _roadId = 0;
      Base::txSectionID _sectionId = 0;
      std::tie(_roadId, _sectionId) = refPointAreaVec.first;
      // 遍历m_vec_waypoint_area
      for (int i = 0; i < m_vec_waypoint_area.size(); ++i) {
        // 如果当前路径区域不是车道连接区域
        if (!m_vec_waypoint_area[i].IsLaneLinkArea()) {
          const Base::txLaneUId& refRouteLaneUid = m_vec_waypoint_area[i].GetLocLaneUid();
          if (refRouteLaneUid.roadId == _roadId && refRouteLaneUid.sectionId == _sectionId) {
            auto insert_itr = std::begin(m_vec_waypoint_area) + i;
            // 在插入位置插入refPointAreaVec.second中的元素
            m_vec_waypoint_area.insert(insert_itr, std::begin(refPointAreaVec.second),
                                       std::end(refPointAreaVec.second));
            auto delete_itr = std::begin(m_vec_waypoint_area) + i + refPointAreaVec.second.size();
            m_vec_waypoint_area.erase(delete_itr);
            break;
          }
        }
      }
    }
#endif
    if (m_vec_waypoint_area.size() > 0) {
      // 如果首尾节点为点类型
      if (m_vec_waypoint_area[0].IsPointArea() && m_vec_waypoint_area[m_vec_waypoint_area.size() - 1].IsPointArea()) {
        for (int i = 0; i < m_vec_waypoint_area.size(); ++i) {
          // 如果当前节点为LaneLink类型
          if (m_vec_waypoint_area[i].IsLaneLinkArea()) {
            const WayPoints prevPointArea = PrevPointArea(m_vec_waypoint_area, i);
            const WayPoints nextPointArea = NextPointArea(m_vec_waypoint_area, i);
            // 获取前后车道id
            const Base::txLaneUId prevTxLaneUid = prevPointArea.GetLocLaneUid();
            const Base::txLaneUId nextTxLaneUid = nextPointArea.GetLocLaneUid();
            // 获取道路前后id
            const Base::txRoadID fromRoadId = m_vec_waypoint_area[i - 1].GetLocLaneUid().roadId;
            const Base::txRoadID toRoadId = m_vec_waypoint_area[i + 1].GetLocLaneUid().roadId;
            const Base::LocInfoSet set_lanelink =
                HdMap::HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(fromRoadId, toRoadId);
            // 如果获取到LaneLink数据
            if (set_lanelink.size() > 0) {
              std::vector<std::pair<Base::txInt, hadmap::txLaneLinkPtr> > vec_pair_int_linkPtr;
              for (const Base::Info_Lane_t& refLink_locInfo : set_lanelink) {
                // 获取对应的LaneLink指针
                hadmap::txLaneLinkPtr linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(
                    refLink_locInfo.onLinkFromLaneUid, refLink_locInfo.onLinkToLaneUid);
                const int nFromDiff = std::abs(linkPtr->fromLaneId() - prevTxLaneUid.laneId);
                const int nToDiff = std::abs(linkPtr->toLaneId() - nextTxLaneUid.laneId);
                vec_pair_int_linkPtr.push_back(std::make_pair(/*nFromDiff +*/ nToDiff, linkPtr));
                LogWarn << "[vec_pair_int_linkPtr] " << TX_VARS(linkPtr->fromLaneId()) << TX_VARS(prevTxLaneUid.laneId)
                        << TX_VARS(std::abs(linkPtr->fromLaneId() - prevTxLaneUid.laneId))
                        << TX_VARS(linkPtr->toLaneId()) << TX_VARS(nextTxLaneUid.laneId)
                        << TX_VARS(std::abs(linkPtr->toLaneId() - nextTxLaneUid.laneId));
              }

              std::sort(vec_pair_int_linkPtr.begin(), vec_pair_int_linkPtr.end(), cmp);
              hadmap::txLaneLinkPtr selectedLinkPtr = vec_pair_int_linkPtr[0].second;
              LogWarn << "[selected_lanelink] " << TX_VARS_NAME(lanelinkId, selectedLinkPtr->getId())
                      << TX_VARS_NAME(from_uid, Utils::ToString(selectedLinkPtr->fromTxLaneId()))
                      << TX_VARS_NAME(to_uid, Utils::ToString(selectedLinkPtr->toTxLaneId()));
              m_vec_waypoint_area[i].Initialize(selectedLinkPtr->getId(), selectedLinkPtr->fromTxLaneId(),
                                                selectedLinkPtr->toTxLaneId());
            } else {
              _isValid = false;
              return;
            }
          }
        }
      } else {
        _isValid = false;
      }
    } else {
      _isValid = false;
    }

    LogWarn << "[Routing_Result] start.";
    // 遍历m_vec_waypoint_area
    for (int i = 0; i < m_vec_waypoint_area.size(); ++i) {
      if (m_vec_waypoint_area[i].IsPointArea()) {
        LogWarn << "[PointArea] " << m_vec_waypoint_area[i].GetEndPt()
                << Utils::ToString(m_vec_waypoint_area[i].GetLocLaneUid());
      } else if (m_vec_waypoint_area[i].IsRoadSectionArea()) {  // 如果当前路径区域是道路区域
        LogWarn << "[RoadSectionArea] " << Utils::ToString(m_vec_waypoint_area[i].GetLocLaneUid());
      } else if (m_vec_waypoint_area[i].IsLaneLinkArea()) {  // 如果当前路径区域是车道连接区域
        LogWarn << "[LaneLinkArea] " << TX_VARS_NAME(lanelinkId, m_vec_waypoint_area[i].GetLaneLinkId())
                << TX_VARS_NAME(fromUid, Utils::ToString(m_vec_waypoint_area[i].GetLaneLinkFromUid()))
                << TX_VARS_NAME(toUid, Utils::ToString(m_vec_waypoint_area[i].GetLaneLinkToUid()));
      }
    }
    LogWarn << "[Routing_Result] end.";
  } else {
    _isValid = false;
  }
  if (m_vec_waypoint_area.size() > 0) {
    m_iGoalLaneId = m_vec_waypoint_area[0].GetLocLaneUid().laneId;
  }

  LogWarn << "call SetWayPoints End.";
}

Base::txLaneID RoutePathManager::FindNextTargentLaneId(const Base::txSize nextStartIndex) const TX_NOEXCEPT {
  // 从nextStartIndex开始遍历m_vec_waypoint_area
  for (Base::txSize i = nextStartIndex; i < m_vec_waypoint_area.size(); ++i) {
    auto& curWayPointArea = m_vec_waypoint_area[i];
    // 如果当前路径区域是点区域
    if (curWayPointArea.IsPointArea()) {
      return curWayPointArea.GetLocLaneUid().laneId;
    }
    // 如果当前路径区域是车道连接区域
    if (curWayPointArea.IsLaneLinkArea()) {
      return curWayPointArea.GetLaneLinkFromUid().laneId;
    }
  }
  return 0;
}

void RoutePathManager::CheckWayPoint(Coord::txWGS84 curPos, const Base::Info_Lane_t& laneInfo) TX_NOEXCEPT {
  Base::txLaneID lastGoalLaneId = m_iGoalLaneId;
  if (laneInfo.isOnLaneLink) {
    // LogWarn << "[CheckWayPoint] isOnLaneLink";
    auto itr = m_vec_waypoint_area.begin();
    for (; itr != m_vec_waypoint_area.end(); ++itr) {
      auto& waypointArea = *itr;
      // 如果是LaneLinkArea
      if (waypointArea.IsLaneLinkArea()) {
        // 已经到达了PointArea
        if (waypointArea.ArriveAtWayPointArea(laneInfo.onLinkId_without_equal, laneInfo.onLinkFromLaneUid,
                                              laneInfo.onLinkToLaneUid)) {
          /*LogWarn << "[CheckWayPoint] isOnLaneLink " << TX_VARS(laneInfo.onLinkId)
              << TX_VARS_NAME(onLinkFromLaneUid, Utils::ToString(laneInfo.onLinkFromLaneUid))
              << TX_VARS_NAME(onLinkToLaneUid, Utils::ToString(laneInfo.onLinkToLaneUid));*/
          waypointArea.SetInValid();
          auto& nextWayPointArea = *(itr + 1);
          _nextWayPointAreaIndex = std::distance(std::begin(m_vec_waypoint_area), (itr + 1));
          // LogWarn << TX_VARS(_nextWayPointAreaIndex);
          if (nextWayPointArea.IsPointArea()) {
            m_iGoalLaneId = nextWayPointArea.GetLocLaneUid().laneId;
            LogWarn << "[multi_waypoint] change goal lane id from " << lastGoalLaneId << " to " << m_iGoalLaneId;
          } else {
            const Base::txLaneID res = FindNextTargentLaneId(_nextWayPointAreaIndex);
            if (res < 0) {
              m_iGoalLaneId = res;
              LogWarn << "[multi_waypoint] change goal lane id from " << lastGoalLaneId << " to " << m_iGoalLaneId;
            } else {
              m_iGoalLaneId = waypointArea.GetLaneLinkToUid().laneId;
              LogWarn << "[multi_waypoint] change goal lane id from " << lastGoalLaneId << " to " << m_iGoalLaneId;
            }
          }
          break;
        }
      }
    }
  } else {
    /*LogWarn << "[CheckWayPoint] isOnLane";*/
    const Base::txSize nWayPointSize = m_vec_waypoint_area.size();
    for (Base::txSize i = 0; i < nWayPointSize; ++i) {
      auto& waypointArea = m_vec_waypoint_area[i];
      if (CallFail(waypointArea.IsValid())) {
        continue;
      }
      if (waypointArea.IsRoadSectionArea() || waypointArea.IsPointArea()) {
        if ((waypointArea.IsRoadSectionArea() &&
             waypointArea.ArriveAtWayPointArea(laneInfo.onLaneUid.roadId, laneInfo.onLaneUid.sectionId)) ||
            (waypointArea.IsPointArea() && waypointArea.ArriveAtWayPointArea(laneInfo.onLaneUid, curPos.ToENU()))) {
          // LogWarn << "[CheckWayPoint] isOnLane " << Utils::ToString(laneInfo.onLaneUid);
          waypointArea.SetInValid();
          if ((i + 1) < nWayPointSize) {
            auto& nextWayPointArea = m_vec_waypoint_area[i + 1];
            _nextWayPointAreaIndex = i + 1;
            // LogWarn << TX_VARS(_nextWayPointAreaIndex);
            // 如果是point区域
            if (nextWayPointArea.IsPointArea()) {
              m_iGoalLaneId = nextWayPointArea.GetLocLaneUid().laneId;
              LogWarn << "[multi_waypoint] change goal lane id from " << lastGoalLaneId << " to " << m_iGoalLaneId;
            } else if (nextWayPointArea.IsRoadSectionArea()) {  // 如果是road section区域
              const Base::txLaneID res = FindNextTargentLaneId(_nextWayPointAreaIndex);
              if (res < 0) {
                m_iGoalLaneId = res;
                LogWarn << "[multi_waypoint] change goal lane id from " << lastGoalLaneId << " to " << m_iGoalLaneId;
              } else {
                // LogWarn << "[multi_waypoint] re>= 0, " << TX_VARS(_nextWayPointAreaIndex);
              }
            } else if (nextWayPointArea.IsLaneLinkArea()) {  // 如果是lane link区域
              m_iGoalLaneId = nextWayPointArea.GetLaneLinkFromUid().laneId;
              LogWarn << "[multi_waypoint] change goal lane id from " << lastGoalLaneId << " to " << m_iGoalLaneId;
            }
          }
          break;
        }
      }
    }
  }
}

// 获取下一个连接的路口，当前路口的信息
// 给定路口ID和部分ID，返回下一个连接的指针
hadmap::txLaneLinkPtr RoutePathManager::GetNextLaneLinkPtr(const Base::txRoadID _roadId,
                                                           const Base::txSectionID _sectionId) const TX_NOEXCEPT {
  hadmap::txLaneLinkPtr retLinkPtr = nullptr;
  // 遍历所有的沿线点和路段区域
  for (int i = 0; i < m_vec_waypoint_area.size(); ++i) {
    const auto& refWayPointArea = m_vec_waypoint_area[i];
    // 如果当前区域是沿线点或路段区域
    if (refWayPointArea.IsPointArea() || refWayPointArea.IsRoadSectionArea()) {
      const Base::txLaneUId waypoint_laneUid = refWayPointArea.GetLocLaneUid();
      // 如果当前沿线点或路段与给定的沿线点相匹配
      if (_roadId == waypoint_laneUid.roadId && _sectionId == waypoint_laneUid.sectionId) {
        // 遍历当前区域的后继连接
        for (int next_link_idx = i + 1; next_link_idx < m_vec_waypoint_area.size(); ++next_link_idx) {
          const auto& ref_Next_WayPointArea = m_vec_waypoint_area[next_link_idx];
          // 如果当前的后继区域是连接区域
          if (ref_Next_WayPointArea.IsLaneLinkArea()) {
            // 通过Uid查询对应的连接指针
            retLinkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(ref_Next_WayPointArea.GetLaneLinkFromUid(),
                                                                        ref_Next_WayPointArea.GetLaneLinkToUid());
          }
        }
        // 返回查询到的连接指针
        return retLinkPtr;
      }
    }
  }
  // 如果没有找到匹配的连接，则返回空指针
  return retLinkPtr;
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
