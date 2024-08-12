// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "HdMap/tx_hashed_road_area.h"
#include "HdMap/tx_hashed_road.h"
#include "tx_spatial_query.h"
#if USE_HashedRoadNetwork
TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(SpatialQuery)

Base::txBool txHashedRoadArea::Initialize(const Base::txSysId _id, Coord::txWGS84 endPt) TX_NOEXCEPT {
  _area_id = _id;
  Base::txFloat distanceCurve = 0.0;
  hadmap::txLanePtr _EndPtLanePtr = Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().GetLaneNearBy(
      endPt.Lon(), endPt.Lat(), distanceCurve);
  if (NonNull_Pointer(_EndPtLanePtr)) {
    // 获取终点车道的Uid
    const Base::txLaneUId _deadline_LaneUid = _EndPtLanePtr->getTxLaneId();
    // 创建一个Info_Lane_t对象，包含终点车道的Uid
    Base::Info_Lane_t locInfo(_deadline_LaneUid);
    // 获取终点车道的几何信息
    HdMap::txLaneInfoInterfacePtr geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(_deadline_LaneUid);
    // 生成一个HashedLaneInfo对象，包含终点车道的几何信息和距离曲线的长度
    HashedLaneInfo curHashedLaneInfo = Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, distanceCurve, locInfo);
    // 获取HashedLaneInfo的实际长度
    const Base::txFloat rawNodeLength = curHashedLaneInfo.RealLength();
    // 如果当前HashedLaneInfo是最后一个子路段，且其实际长度小于最小终点长度
    if (curHashedLaneInfo.IsLastSection() && curHashedLaneInfo.RealLength() < FLAGS_deadline_minimum_length) {
      // 获取前方HashedLaneInfo列表
      std::list<HashedLaneInfoPtr> front_nodes;
      Geometry::SpatialQuery::GetFrontHashedLaneInfoList(curHashedLaneInfo, front_nodes, 1);
      // 如果前方HashedLaneInfo列表非空，且第一个元素在车道上，且其实际长度大于当前HashedLaneInfo的实际长度
      if (_NonEmpty_(front_nodes) && CallSucc(front_nodes.front()->m_laneinfo.IsOnLane()) &&
          (front_nodes.front()->RealLength() > rawNodeLength)) {
        LOG(INFO) << "[shift_deadline][front]" << TX_VARS_NAME(from_node, curHashedLaneInfo)
                  << TX_VARS_NAME(to_node, *front_nodes.front())
                  << TX_VARS_NAME(deadline_minimum_length, FLAGS_deadline_minimum_length)
                  << TX_VARS_NAME(from_node_length, curHashedLaneInfo.RealLength())
                  << TX_VARS_NAME(to_node_length, (*front_nodes.front()).RealLength());
        // 更新当前HashedLaneInfo为前方HashedLaneInfo列表中的第一个元素
        curHashedLaneInfo = *(front_nodes.front());
      } else {
        // 清空前方HashedLaneInfo列表
        front_nodes.clear();
        // 获取后方HashedLaneInfo列表
        std::list<HashedLaneInfoPtr>& back_nodes = front_nodes;
        Geometry::SpatialQuery::GetBackHashedLaneInfoList(curHashedLaneInfo, back_nodes, 1);
        // 如果后方HashedLaneInfo列表非空，且第一个元素在车道上，且其实际长度大于当前HashedLaneInfo的实际长度
        if (_NonEmpty_(back_nodes) && CallSucc(back_nodes.front()->m_laneinfo.IsOnLane()) &&
            (back_nodes.front()->RealLength() > rawNodeLength)) {
          // 输出日志信息
          LOG(INFO) << "[shift_deadline][back]" << TX_VARS_NAME(from_node, curHashedLaneInfo)
                    << TX_VARS_NAME(to_node, *back_nodes.front())
                    << TX_VARS_NAME(deadline_minimum_length, FLAGS_deadline_minimum_length)
                    << TX_VARS_NAME(from_node_length, curHashedLaneInfo.RealLength())
                    << TX_VARS_NAME(to_node_length, (*back_nodes.front()).RealLength());
          // 更新当前HashedLaneInfo为后方HashedLaneInfo列表中的第一个元素
          curHashedLaneInfo = *(back_nodes.front());
        } else {
          LOG(INFO) << "[raw_deadline][front.back.invalid] " << curHashedLaneInfo;
        }
      }
    } else {
      LOG(INFO) << "[raw_deadline][normal] " << curHashedLaneInfo;
    }

    // 清空_vec_OrthogonalListPtr列表
    _vec_OrthogonalListPtr.clear();
    // 预留10个空间
    _vec_OrthogonalListPtr.reserve(10);

    // 查询当前HashedLaneInfo的正交列表
    HashedLaneInfoOrthogonalListPtr srcOrthogonalListPtr =
        HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(curHashedLaneInfo);
    // 如果正交列表非空
    if (NonNull_Pointer(srcOrthogonalListPtr)) {
      // 将当前正交列表添加到_vec_OrthogonalListPtr列表中
      _vec_OrthogonalListPtr.emplace_back(srcOrthogonalListPtr);
      // 获取当前正交列表的左侧正交列表
      HashedLaneInfoOrthogonalListPtr start_orthogonal_list_ptr = srcOrthogonalListPtr->GetLeftNode();
      // 如果左侧正交列表非空
      while (NonNull_Pointer(start_orthogonal_list_ptr)) {
        // 将左侧正交列表添加到_vec_OrthogonalListPtr列表中
        _vec_OrthogonalListPtr.emplace_back(start_orthogonal_list_ptr);
        // 获取下一个左侧正交列表
        start_orthogonal_list_ptr = start_orthogonal_list_ptr->GetLeftNode();
      }

      // 获取当前正交列表的右侧正交列表
      start_orthogonal_list_ptr = srcOrthogonalListPtr->GetRightNode();
      // 如果右侧正交列表非空
      while (NonNull_Pointer(start_orthogonal_list_ptr)) {
        // 将右侧正交列表添加到_vec_OrthogonalListPtr列表中
        _vec_OrthogonalListPtr.emplace_back(start_orthogonal_list_ptr);
        // 获取下一个右侧正交列表
        start_orthogonal_list_ptr = start_orthogonal_list_ptr->GetRightNode();
      }

      _valid = true;

      // 输出日志信息
      std::ostringstream oss;
      oss << "[area] " << TX_VARS(AreaId());
      for (const auto& orthogonal_list_ptr : _vec_OrthogonalListPtr) {
        oss << orthogonal_list_ptr->GetSelfHashedInfo().subSectionStartWGS84.StrWGS84() << ", ";
      }
      LOG(INFO) << "create area success: " << oss.str();
    } else {
      // 输出日志信息
      LOG(WARNING) << "cannot query Orthogonal List Ptr : " << curHashedLaneInfo;
    }
  } else {
    LOG(WARNING) << "[deadline] create failure. location on lanelink. " << endPt;
  }
  return _valid;
}

Base::txBool txHashedRoadArea::Initialize(const Base::txSysId _id, Coord::txWGS84 endPt,
                                          std::set<Base::txLaneID> cover) TX_NOEXCEPT {
  _area_id = _id;
  Base::txFloat distanceCurve = 0.0;
  hadmap::txLanePtr _EndPtLanePtr = Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().GetLaneNearBy(
      endPt.Lon(), endPt.Lat(), distanceCurve);
  if (NonNull_Pointer(_EndPtLanePtr)) {
    // 获取终点车道的Uid
    const Base::txLaneUId _deadline_LaneUid = _EndPtLanePtr->getTxLaneId();
    // 创建一个Info_Lane_t对象，包含终点车道的Uid
    Base::Info_Lane_t locInfo(_deadline_LaneUid);
    // 获取终点车道的几何信息
    HdMap::txLaneInfoInterfacePtr geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(_deadline_LaneUid);
    // 生成一个HashedLaneInfo对象，包含终点车道的几何信息和距离曲线的长度
    HashedLaneInfo curHashedLaneInfo = Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, distanceCurve, locInfo);
    // 获取HashedLaneInfo的实际长度
    const Base::txFloat rawNodeLength = curHashedLaneInfo.RealLength();
    // 如果当前HashedLaneInfo是最后一个子路段，且其实际长度小于最小终点长度
    if (curHashedLaneInfo.IsLastSection() && curHashedLaneInfo.RealLength() < FLAGS_deadline_minimum_length) {
      // 获取前方HashedLaneInfo列表
      std::list<HashedLaneInfoPtr> front_nodes;
      Geometry::SpatialQuery::GetFrontHashedLaneInfoList(curHashedLaneInfo, front_nodes, 1);
      // 如果前方HashedLaneInfo列表非空，且第一个元素在车道上，且其实际长度大于当前HashedLaneInfo的实际长度
      if (_NonEmpty_(front_nodes) && CallSucc(front_nodes.front()->m_laneinfo.IsOnLane()) &&
          (front_nodes.front()->RealLength() > rawNodeLength)) {
        // 输出日志信息
        LOG(INFO) << "[shift_deadline][front]" << TX_VARS_NAME(from_node, curHashedLaneInfo)
                  << TX_VARS_NAME(to_node, *front_nodes.front())
                  << TX_VARS_NAME(deadline_minimum_length, FLAGS_deadline_minimum_length)
                  << TX_VARS_NAME(from_node_length, curHashedLaneInfo.RealLength())
                  << TX_VARS_NAME(to_node_length, (*front_nodes.front()).RealLength());
        // 更新当前HashedLaneInfo为前方HashedLaneInfo列表中的第一个元素
        curHashedLaneInfo = *(front_nodes.front());
      } else {
        // 清空前方HashedLaneInfo列表
        front_nodes.clear();
        // 获取后方HashedLaneInfo列表
        std::list<HashedLaneInfoPtr>& back_nodes = front_nodes;
        Geometry::SpatialQuery::GetBackHashedLaneInfoList(curHashedLaneInfo, back_nodes, 1);
        // 如果后方HashedLaneInfo列表非空，且第一个元素在车道上，且其实际长度大于当前HashedLaneInfo的实际长度
        if (_NonEmpty_(back_nodes) && CallSucc(back_nodes.front()->m_laneinfo.IsOnLane()) &&
            (back_nodes.front()->RealLength() > rawNodeLength)) {
          LOG(INFO) << "[shift_deadline][back]" << TX_VARS_NAME(from_node, curHashedLaneInfo)
                    << TX_VARS_NAME(to_node, *back_nodes.front())
                    << TX_VARS_NAME(deadline_minimum_length, FLAGS_deadline_minimum_length)
                    << TX_VARS_NAME(from_node_length, curHashedLaneInfo.RealLength())
                    << TX_VARS_NAME(to_node_length, (*back_nodes.front()).RealLength());
          // 更新当前HashedLaneInfo为后方HashedLaneInfo列表中的第一个元素
          curHashedLaneInfo = *(back_nodes.front());
        } else {
          LOG(INFO) << "[raw_deadline][front.back.invalid] " << curHashedLaneInfo;
        }
      }
    } else {
      LOG(INFO) << "[raw_deadline][normal] " << curHashedLaneInfo;
    }

    _vec_OrthogonalListPtr.clear();
    _vec_OrthogonalListPtr.reserve(10);

    // 查询当前HashedLaneInfo的正交列表
    HashedLaneInfoOrthogonalListPtr srcOrthogonalListPtr =
        HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(curHashedLaneInfo);
    // 如果正交列表非空
    if (NonNull_Pointer(srcOrthogonalListPtr)) {
      // 如果当前正交列表的车道在覆盖范围内
      if (cover.count(srcOrthogonalListPtr->GetSelfHashedInfo().LaneInfo().onLaneUid.laneId)) {
        // 将当前正交列表添加到_vec_OrthogonalListPtr列表中
        _vec_OrthogonalListPtr.emplace_back(srcOrthogonalListPtr);
      }
      // 获取当前正交列表的左侧正交列表
      HashedLaneInfoOrthogonalListPtr start_orthogonal_list_ptr = srcOrthogonalListPtr->GetLeftNode();
      // 如果左侧正交列表非空
      while (NonNull_Pointer(start_orthogonal_list_ptr)) {
        // 如果左侧正交列表的车道在覆盖范围内
        if (cover.count(start_orthogonal_list_ptr->GetSelfHashedInfo().LaneInfo().onLaneUid.laneId)) {
          // 将左侧正交列表添加到_vec_OrthogonalListPtr列表中
          _vec_OrthogonalListPtr.emplace_back(start_orthogonal_list_ptr);
        }
        // 获取下一个左侧正交列表
        start_orthogonal_list_ptr = start_orthogonal_list_ptr->GetLeftNode();
      }

      // 获取当前正交列表的右侧正交列表
      start_orthogonal_list_ptr = srcOrthogonalListPtr->GetRightNode();
      // 如果右侧正交列表非空
      while (NonNull_Pointer(start_orthogonal_list_ptr)) {
        // 如果右侧正交列表的车道在覆盖范围内
        if (cover.count(start_orthogonal_list_ptr->GetSelfHashedInfo().LaneInfo().onLaneUid.laneId)) {
          _vec_OrthogonalListPtr.emplace_back(start_orthogonal_list_ptr);
        }
        // 获取下一个右侧正交列表
        start_orthogonal_list_ptr = start_orthogonal_list_ptr->GetRightNode();
      }

      // 如果_vec_OrthogonalListPtr列表非空，设置_valid为true
      if (_vec_OrthogonalListPtr.size() > 0) {
        _valid = true;
      }

      // 输出日志信息
      std::ostringstream oss;
      oss << "[area] " << TX_VARS(AreaId());
      for (const auto& orthogonal_list_ptr : _vec_OrthogonalListPtr) {
        oss << orthogonal_list_ptr->GetSelfHashedInfo().subSectionStartWGS84.StrWGS84() << ", ";
      }
      LOG(INFO) << "create area success: " << oss.str();
    } else {
      LOG(WARNING) << "cannot query Orthogonal List Ptr : " << curHashedLaneInfo;
    }
  } else {
    LOG(WARNING) << "[deadline] create failure. location on lanelink. " << endPt;
  }
  return _valid;
}

Base::txBool txHashedRoadArea::QueryVehicles(txHashedRoadArea::VehicleContainer& refVehicleContainer) TX_NOEXCEPT {
  // 输出日志信息
  if (IsValid()) {
    // 清空refVehicleContainer列表
    refVehicleContainer.clear();
    // 遍历_vec_OrthogonalListPtr列表中的正交列表
    for (auto orthogonal_list_ptr : _vec_OrthogonalListPtr) {
      // 创建一个临时的VehicleContainer对象
      VehicleContainer resContainer;
      // 获取当前正交列表中的车辆
      orthogonal_list_ptr->GetRegisterVehicles(resContainer);
      // 将当前正交列表中的车辆添加到refVehicleContainer列表中
      refVehicleContainer.insert(resContainer.begin(), resContainer.end());
    }
  }
  return IsValid();
}

Base::txBool txHashedRoadArea::ArriveAtArea(const HashedLaneInfo& objHashedNode) const TX_NOEXCEPT {
  // 遍历_vec_OrthogonalListPtr列表中的正交列表
  for (auto orthogonal_list_ptr : _vec_OrthogonalListPtr) {
    // 如果给定的HashedLaneInfo等于当前正交列表的HashedLaneInfo，返回true
    if (objHashedNode == (orthogonal_list_ptr->GetSelfHashedInfo())) {
      return true;
    }
  }
  return false;
}

Base::txString txHashedRoadArea::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_VARS(AreaId()) << TX_COND(IsValid()) << TX_VARS_NAME(Location, GetEndPos());
  for (auto orthogonal_list_ptr : _vec_OrthogonalListPtr) {
    oss << (orthogonal_list_ptr->GetSelfHashedInfo()) << ", ";
  }
  return oss.str();
}

TX_NAMESPACE_CLOSE(SpatialQuery)
TX_NAMESPACE_CLOSE(Geometry)
#endif /*USE_HashedRoadNetwork*/
