// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_signal_element.h"
#include "HdMap/tx_hashed_road.h"
#include "tx_signal_utils.h"
TX_NAMESPACE_OPEN(Base)

ISignalLightElement::KineticsInfo_t ISignalLightElement::GetKineticsInfo(Base::TimeParamManager const &timeMgr) const
    TX_NOEXCEPT {
  KineticsInfo_t retInfo;
  // 设置info信息
  retInfo.m_elemId = Id();
  retInfo.m_compute_velocity = GetVelocity();
  retInfo.m_acc = GetAcc();
  retInfo.m_show_velocity = GetShowVelocity();
  return retInfo;
}

void ISignalLightElement::GenerateSignalByLaneCache_Internal(
    const hadmap::txLanes &lanes, const RoadId2SignalIdSet &map_roadid2signalId) TX_NOEXCEPT {
#if _CloseControlSignal_

  for (const auto curLanePtr : lanes) {
    std::set<Base::txLaneUId> curLaneUidSet;
    std::set<Base::txLaneID> curLinkIdSet;
    Utils::GetPreLaneAndLinkAndSelfLaneFromLane(curLanePtr, curLaneUidSet, curLinkIdSet);
    LOG_IF(WARNING, FLAGS_LogLevel_Signal)
        << TX_VARS(Id()) << TX_VARS(curLaneUidSet.size()) << TX_VARS(curLinkIdSet.size());
    std::copy(std::begin(curLaneUidSet), std::end(curLaneUidSet),
              std::inserter(m_ControlLaneSet, std::end(m_ControlLaneSet)));
    std::copy(std::begin(curLinkIdSet), std::end(curLinkIdSet),
              std::inserter(m_ControlLaneLinkSet, std::end(m_ControlLaneLinkSet)));
  }

  // MapManager::getInstance().SearchPrevLanesAndLinks(lanes, FLAGS_Signal_Stop_Dist, m_ControlLaneSet,
  // m_ControlLaneLinkSet);
  const Base::txVec2 signal_dir_2d = Utils::Vec3_Vec2(signal_dir);
  for (auto itr = m_ControlLaneSet.begin(); itr != m_ControlLaneSet.end();) {
    const Base::txLaneUId laneUid = *itr;
    Base::txVec2 laneDir2d = Utils::Vec3_Vec2(HdMap::HadmapCacheConCurrent::GetLaneDir(laneUid, 0.0));
    if (CallFail(Math::IsSynchronicity(signal_dir_2d, laneDir2d))) {
      itr = m_ControlLaneSet.erase(itr);
      LOG(WARNING) << TX_VARS_NAME(erase_lane, Utils::ToString(laneUid));
    } else {
      // ++itr;
      const Base::txFloat fAngle = Math::VectorRadian2D(signal_dir_2d, laneDir2d);
      if (fAngle < 15.0) {
        ++itr;
      } else {
        itr = m_ControlLaneSet.erase(itr);
      }
    }
  }

  for (auto itr = m_ControlLaneLinkSet.begin(); itr != m_ControlLaneLinkSet.end();) {
    const Base::txLaneLinkID linkId = *itr;
    Base::txVec2 laneDir2d = Utils::Vec3_Vec2(HdMap::HadmapCacheConCurrent::GetLaneLinkDir(linkId, 0.0));
    if (CallFail(Math::IsSynchronicity(signal_dir_2d, laneDir2d))) {
      itr = m_ControlLaneLinkSet.erase(itr);
      LOG(WARNING) << TX_VARS_NAME(erase_link, (linkId));
    } else {
      // ++itr;
      const Base::txFloat fAngle = Math::VectorRadian2D(signal_dir_2d, laneDir2d);
      if (fAngle < 15.0) {
        ++itr;
      } else {
        itr = m_ControlLaneLinkSet.erase(itr);
      }
    }
  }

  if (FLAGS_LogLevel_Signal) {
    for (const auto &laneUid : m_ControlLaneSet) {
      LOG(WARNING) << "[signal_light_by_lane_result] " << TX_VARS(Id())
                   << ", control lane : " << Utils::ToString(laneUid);
    }

    for (const auto &laneLinkId : m_ControlLaneLinkSet) {
      LOG(WARNING) << "[signal_light_by_lane_result] " << TX_VARS(Id()) << ", control lanelink : " << laneLinkId;
    }
  }
#else  /*_CloseControlSignal_*/

  // 初始化忽略的道路ID集合
  std::set<Base::txRoadID> ignoreRoadIdSet /*almost size = 1*/;
  // 遍历lanes中的车道
  for (const auto &curLanePtr : lanes) {
    // 如果curLanePtr不为空
    if (NonNull_Pointer(curLanePtr)) {
      ignoreRoadIdSet.insert(curLanePtr->getRoadId());
    }
  }
  // 创建一个SignalControlLaneOp对象，用于检查道路ID是否在ignoreRoadIdSet中
  SignalControlLaneOp checkOp(map_roadid2signalId, ignoreRoadIdSet);
  using std::placeholders::_1;
  std::function<Base::txBool(const Base::txRoadID)> check_func = std::bind(&SignalControlLaneOp::check, checkOp, _1);
  // 清空mControlOrthogonalNodeMap
  mControlOrthogonalNodeMap.clear();
  // 遍历lanes中的车道
  for (const auto &curLanePtr : lanes) {
    // 如果curLanePtr不为空
    if (NonNull_Pointer(curLanePtr)) {
      // 获取curLanePtr的车道ID
      const auto curLaneUid = curLanePtr->getTxLaneId();
      const HdMap::txLaneInfoInterfacePtr geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(curLaneUid);
      // 计算起始位置的倒车道长度
      const Base::txFloat start_inv_s = geom_ptr->GetLength() - 0.1;
      // 生成一个HashedLaneInfo对象，表示起始位置
      HashedLaneInfo start_hash_node =
          Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, start_inv_s, Base::Info_Lane_t(curLaneUid));
      // 获取起始位置的正交列表
      HashedLaneInfoOrthogonalListPtr start_Orthogonal_ptr =
          HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(start_hash_node);
      // 搜索正交列表，找到控制车道
      HashedLaneInfoOrthogonalList::SearchOrthogonalListPost(start_Orthogonal_ptr, FLAGS_SignalMaxVisionDistance,
                                                             mControlOrthogonalNodeMap, check_func);
    }
  }
  m_ControlLaneHashSet.clear();
  // 遍历mControlOrthogonalNodeMap中的节点
  for (const auto &refNode : mControlOrthogonalNodeMap) {
    m_ControlLaneHashSet.insert(refNode.first.LaneInfo());
  }

  // 如果启用了日志记录
  if (FLAGS_LogLevel_Signal) {
    // 遍历mControlOrthogonalNodeMap中的节点
    for (const auto &ref_node_ptr : mControlOrthogonalNodeMap) {
      LOG(WARNING) << "[signal_light_by_lane_result] " << TX_VARS(Id())
                   << " control laneinfo : " << ref_node_ptr.first.LaneInfo();
    }

    // 遍历m_ControlLaneHashSet中的车道信息
    for (const auto &locInfo : m_ControlLaneHashSet) {
      LOG(WARNING) << "[signal_light_by_lane_result] " << TX_VARS(Id()) << ", control lane : " << locInfo;
    }
  }
  m_control_lane_set_from_conf.clear();
  // 遍历m_ControlLaneHashSet中的车道信息
  for (const auto &locInfo : m_ControlLaneHashSet) {
    if (locInfo.IsOnLane()) {
      // 将locInfo的车道ID和信号灯的位置插入到m_control_lane_set_from_conf中
      m_control_lane_set_from_conf.push_back(std::make_pair(locInfo.onLaneUid, GetLocation().WGS84()));
    }
  }
#endif /*_CloseControlSignal_*/
}

Base::txBool ISignalLightElement::GenerateSignalByLaneCache(Base::txString control_lane,
                                                            const Base::txLaneUId signalLoc, hadmap::txLanes &lanes,
                                                            Coord::txWGS84 &signalPosition) TX_NOEXCEPT {
  lanes.clear();
  if (Base::txString("ALL") == control_lane) {
    // 获取信号灯所在路段下的所有车道
    lanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(std::make_pair(signalLoc.roadId, signalLoc.sectionId));
  } else {
    // 创建一个txLaneID类型的向量，用于存储车道ID
    std::vector<Base::txLaneID> laneIdVec;
    Utils::trim(control_lane);
    // 如果control_lane不为空
    if (!control_lane.empty()) {
      // 创建一个txString类型的向量，用于存储分割后的字符串
      std::vector<Base::txString> results_pairs;
      boost::algorithm::split(results_pairs, control_lane, boost::is_any_of(";"));
      // 遍历分割后的字符串
      for (auto s : results_pairs) {
        // 去除字符串两端的空格
        Utils::trim(s);
        if (!s.empty()) {
          laneIdVec.push_back(std::stol(s));
        }
      }
    }

    // 预留lanes向量的空间，大小为laneIdVec的大小加1
    lanes.reserve(laneIdVec.size() + 1);
    // 遍历laneIdVec中的车道ID
    for (const auto laneId : laneIdVec) {
      lanes.push_back(
          HdMap::HadmapCacheConCurrent::GetTxLanePtr(Base::txLaneUId(signalLoc.roadId, signalLoc.sectionId, laneId)));
    }
  }

  // // 遍历lanes中的车道
  // for (auto itr = std::begin(lanes); itr != std::end(lanes);) {
  //   // 如果车道无效
  //   if (CallFail(Utils::IsLaneValidDriving(*itr))) {
  //     LOG_IF(INFO, FLAGS_LogLevel_Signal)
  //         << TX_VARS_NAME(erase_lane, Utils::ToString((*itr)->getTxLaneId())) << TX_VARS((*itr)->getLaneType());
  //     // 从lanes中删除无效车道
  //     itr = lanes.erase(itr);
  //   } else {
  //     ++itr;
  //   }
  // }
  m_control_lane_set_from_conf.clear();
  // 初始化signalLocationOnENU为零向量
  Base::txVec3 signalLocationOnENU;
  signalLocationOnENU.setZero();
  // 初始化车道计数器nLaneCnt为0
  Base::txInt nLaneCnt = 0;
  // 初始化信号灯方向向量signal_dir
  Base::txVec3 signal_dir;
  // 遍历lanes中的车道
  for (const auto &l : lanes) {
    // 如果车道指针非空
    if (NonNull_Pointer(l)) {
      const auto &refLaneUid = l->getTxLaneId();
      // 如果车道ID小于0
      if (refLaneUid.laneId < 0) {
        ++nLaneCnt;
        signal_dir = HdMap::HadmapCacheConCurrent::GetLaneDir(refLaneUid, l->getLength() - 0.5);
        // 获取车道长度
        const auto laneLength = l->getGeometry()->getLength();
        hadmap::txPoint signal_loc_by_lane = l->getGeometry()->getPoint(laneLength - 1.0);
        // 将车道ID和信号灯位置添加到m_control_lane_set_from_conf中
        m_control_lane_set_from_conf.push_back(std::make_pair(refLaneUid, signal_loc_by_lane));
        Coord::txENU curSignalLoc;
        curSignalLoc.FromWGS84(signal_loc_by_lane);
        // 将信号灯位置累加到signalLocationOnENU中
        signalLocationOnENU += curSignalLoc.ENU();
        LOG_IF(WARNING, FLAGS_LogLevel_Signal)
            << "[signal_light_by_lane][pos] " << TX_VARS(Id()) << ", control lane : " << Utils::ToString(refLaneUid);
      }
    }
  }

  if (nLaneCnt > 0) {
    // 计算信号灯位置的平均值
    signalLocationOnENU = signalLocationOnENU / nLaneCnt;
    // 将信号灯位置从ENU坐标系转换为WGS84坐标系
    signalPosition.FromENU(signalLocationOnENU);
    LOG_IF(WARNING, FLAGS_LogLevel_Signal) << "[signal_light_by_lane] compute average position " << signalPosition;
    return true;
  } else {
    LOG_IF(WARNING, FLAGS_LogLevel_Signal) << "[signal_light_by_lane] valid signal lane is zero ";
    return false;
  }
}

void ISignalLightElement::PublicSignals(const Base::TimeParamManager &) TX_NOEXCEPT {
  // 如果信号灯元素存活
  if (IsAlive()) {
    // 创建一个HashedLaneInfoOrthogonalList::VehicleContainer类型的变量，用于存储车辆信息
    HashedLaneInfoOrthogonalList::VehicleContainer refVehicleContainer;
    for (const auto &ref_node_ptr : mControlOrthogonalNodeMap) {
      // 创建一个HashedLaneInfoOrthogonalList::VehicleContainer类型的变量，用于存储车辆信息
      HashedLaneInfoOrthogonalList::VehicleContainer resContainer;
      ref_node_ptr.second->GetRegisterVehicles(resContainer);
      // 将车辆信息插入到refVehicleContainer中
      refVehicleContainer.insert(resContainer.begin(), resContainer.end());
    }
    // 获取信号灯元素的位置
    const Base::txVec3 signalPos3d = GetLocation().ToENU().ENU();
    Base::ISignalLightElementPtr self_signal_ptr = std::dynamic_pointer_cast<Base::ISignalLightElement>(GetBasePtr());
    // 遍历refVehicleContainer中的车辆信息
    for (auto &ref_id_elemPtr : refVehicleContainer) {
      const Base::txVec3 vehiclePos3d = ref_id_elemPtr.second->StablePositionOnLane().ToENU().ENU();
      const Base::txFloat half_veh_length = ref_id_elemPtr.second->GetLength() * 0.5;
      // 计算信号灯元素和车辆之间的方向向量
      Base::txVec3 vDir = signalPos3d - vehiclePos3d;
      // 计算信号灯元素和车辆之间的距离
      const Base::txFloat curDistance = vDir.norm();
      vDir.normalize();
      // 计算信号灯元素和车辆之间的角度
      const Base::txFloat fAngle = Math::VectorRadian3D(ref_id_elemPtr.second->StableLaneDir(), vDir);
      // 计算车辆到信号灯元素的距离减去车辆长度的一半
      const Base::txFloat distance_sub_half_length = curDistance * sin(Math::PI * 0.5 - fAngle) - half_veh_length;
      TX_MARK("One signal controls multiple lanes.");
      // if (Null_Pointer(ref_id_elemPtr.second->GetCurTargetSignLightPtr()) ||
      // (NonNull_Pointer(ref_id_elemPtr.second->GetCurTargetSignLightPtr()) &&
      // ref_id_elemPtr.second->GetCurTargetSignLightPtr()->Id() > Id())) {
      // ref_id_elemPtr.second->SetCurTargetSignLightPtr(self_signal_ptr);
      // ref_id_elemPtr.second->SetCurDistanceToTargetSignLight(distance_sub_half_length);
      ref_id_elemPtr.second->SetCurTargetSignLight(self_signal_ptr, distance_sub_half_length);
      // }
      // LOG(WARNING) << TX_VARS_NAME(elemID, ref_id_elemPtr.second->Id()) << TX_VARS(distance_sub_half_length);
    }
  }
}
TX_NAMESPACE_CLOSE(Base)
