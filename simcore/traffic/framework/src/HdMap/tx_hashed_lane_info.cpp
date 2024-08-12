// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "HdMap/tx_hashed_lane_info.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_component.h"
#include "tx_hadmap_utils.h"
TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(SpatialQuery)

#if USE_HashedRoadNetwork
Base::txBool GetPrevHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo,
                                   HashedLaneInfo& retHashedLaneInfo) TX_NOEXCEPT {
  static const Base::txFloat sScopeLen = HashedLaneInfo::HashScopeLen();
  // 如果当前路段不是最后一个路段
  if (CallFail(srcHashedLaneInfo.IsLastSection())) {
    TX_MARK("not last section.");

    Base::txFloat sStep = sScopeLen;
    // 如果当前距离加上步长大于等于几何体的长度
    if ((srcHashedLaneInfo.DistanceOnCurve() + sStep) >= srcHashedLaneInfo.GeomLength()) {
      // 更新步长为几何体长度减去当前范围的一半加上当前范围减去当前距离
      sStep = ((srcHashedLaneInfo.GeomLength() - (srcHashedLaneInfo.EndScope())) * 0.5) +
              ((srcHashedLaneInfo.EndScope()) - srcHashedLaneInfo.DistanceOnCurve());
    }
    // 根据新的距离生成新的HashedLaneInfo
    retHashedLaneInfo = Geometry::SpatialQuery::GenerateHashedLaneInfo(
        srcHashedLaneInfo.GeomPtr(), (srcHashedLaneInfo.DistanceOnCurve() + sStep), srcHashedLaneInfo.LaneInfo());
    return true;
  } else {
    TX_MARK("last section.");
    // 如果当前路段在车道链接上
    if (srcHashedLaneInfo.LaneInfo().IsOnLaneLink()) {
      TX_MARK("on lanelink");
      // 获取下一个车道的信息
      HdMap::txLaneInfoInterfacePtr geom_ptr =
          HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(srcHashedLaneInfo.LaneInfo().onLinkToLaneUid);
      Base::Info_Lane_t newLaneInfo(srcHashedLaneInfo.LaneInfo().onLinkToLaneUid);
      // 生成新的HashedLaneInfo
      retHashedLaneInfo = Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (0.1), newLaneInfo);
      return true;
    } else {
      TX_MARK("on lane");
      // 获取下一个车道的集合
      auto nextLaneUidSet =
          HdMap::HadmapCacheConCurrent::GetNextLaneSetByLaneUid(srcHashedLaneInfo.LaneInfo().onLaneUid);
      // 如果下一个车道的集合非空
      if (_NonEmpty_(nextLaneUidSet)) {
        // 如果下一个车道的集合只有一个元素
        if (1 == nextLaneUidSet.size()) {
          const auto nextLaneUid = *(nextLaneUidSet.begin());
          // 获取下一个车道的信息
          HdMap::txLaneInfoInterfacePtr geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(nextLaneUid);
          Base::Info_Lane_t newLaneInfo(nextLaneUid);
          // 生成新的HashedLaneInfo
          retHashedLaneInfo = Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (0.1), newLaneInfo);
          return true;
        } else {
          LOG(WARNING) << TX_VARS(srcHashedLaneInfo.LaneInfo()) << " get next lane has multi results.";
          return false;
        }
      } else {
        TX_MARK("next geometry is lanelink.");
        // 获取下一个车道链接的集合
        auto nextLaneLinkIdSet =
            HdMap::HadmapCacheConCurrent::GetLaneNextLinkSet(srcHashedLaneInfo.LaneInfo().onLaneUid);
        // 如果下一个车道链接的集合非空
        if (_NonEmpty_(nextLaneLinkIdSet)) {
          // 如果下一个车道链接的集合只有一个元素
          if (1 == nextLaneLinkIdSet.size()) {
            const Base::Info_Lane_t nextLaneLink_locInfo = *(nextLaneLinkIdSet.begin());
            // 获取下一个车道链接的信息
            HdMap::txLaneInfoInterfacePtr geom_ptr =
                HdMap::HadmapCacheConCurrent::GetGeomInfoById(nextLaneLink_locInfo);
            auto linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(nextLaneLink_locInfo.onLinkFromLaneUid,
                                                                          nextLaneLink_locInfo.onLinkToLaneUid);
            Base::Info_Lane_t newLaneInfo(nextLaneLink_locInfo.onLinkId_without_equal, linkPtr->fromTxLaneId(),
                                          linkPtr->toTxLaneId());
            // 生成新的HashedLaneInfo
            retHashedLaneInfo = Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (0.1), newLaneInfo);
            return true;
          } else {
            LOG(WARNING) << TX_VARS(srcHashedLaneInfo.m_laneinfo) << " get next lane-link has multi results.";
            return false;
          }
        } else {
          LOG(WARNING) << TX_VARS(srcHashedLaneInfo.m_laneinfo) << " get next lane-link has zero results.";
          return false;
        }
      }
    }
  }
  return false;
}

Base::txBool GetPostHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo,
                                   HashedLaneInfo& retHashedLaneInfo) TX_NOEXCEPT {
  static const Base::txFloat sScopeLen = HashedLaneInfo::HashScopeLen();
  // 如果当前路段不是第一个路段
  if (srcHashedLaneInfo.m_subSectionIdx > 0) {
    // 根据新的距离生成新的HashedLaneInfo
    retHashedLaneInfo = Geometry::SpatialQuery::GenerateHashedLaneInfo(
        srcHashedLaneInfo.GeomPtr(), (srcHashedLaneInfo.DistanceOnCurve() - sScopeLen), srcHashedLaneInfo.LaneInfo());
    return true;
  } else if (0 == srcHashedLaneInfo.m_subSectionIdx) {
    if (srcHashedLaneInfo.LaneInfo().isOnLaneLink) {
      TX_MARK("on lanelink");
      HdMap::txLaneInfoInterfacePtr geom_ptr =
          HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(srcHashedLaneInfo.LaneInfo().onLinkFromLaneUid);
      Base::Info_Lane_t newLaneInfo;
      newLaneInfo.FromLane(srcHashedLaneInfo.m_laneinfo.onLinkFromLaneUid);
      retHashedLaneInfo =
          Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (geom_ptr->GetLength() - 0.5), newLaneInfo);
      return true;
    } else {  // 在车道上时
      TX_MARK("on lane");
      // 获取前一个车道的集合
      auto preLaneUidSet = HdMap::HadmapCacheConCurrent::GetPreLaneSetByLaneUid(srcHashedLaneInfo.LaneInfo().onLaneUid);
      // 如果前一个车道的集合非空
      if (_NonEmpty_(preLaneUidSet)) {
        // 如果前一个车道的集合只有一个元素
        if (1 == preLaneUidSet.size()) {
          // 获取前一个车道的Uid
          const auto postLaneUid = *(preLaneUidSet.begin());
          // 获取前一个车道的信息
          HdMap::txLaneInfoInterfacePtr geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(postLaneUid);
          Base::Info_Lane_t newLaneInfo;
          newLaneInfo.FromLane(postLaneUid);
          // 生成新的HashedLaneInfo
          retHashedLaneInfo =
              Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (geom_ptr->GetLength() - 0.5), newLaneInfo);
          return true;
        } else {
          // 初始化一个迭代器指向前一个车道的集合的开始
          auto itr = preLaneUidSet.begin();
          // 获取前一个车道的Uid
          Base::txLaneUId innerLaneId = *(itr++);
          // 遍历前一个车道的集合，找到最大的车道Uid
          for (; itr != preLaneUidSet.end(); ++itr) {
            if (innerLaneId.laneId < (*itr).laneId) {
              innerLaneId = *itr;
            }
          }
          const auto postLaneUid = innerLaneId;
          // 获取前一个车道的信息
          HdMap::txLaneInfoInterfacePtr geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(postLaneUid);
          Base::Info_Lane_t newLaneInfo;
          newLaneInfo.FromLane(postLaneUid);
          // 生成新的HashedLaneInfo
          retHashedLaneInfo =
              Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (geom_ptr->GetLength() - 0.5), newLaneInfo);
          return true;
          /*LOG(WARNING) << TX_VARS(srcHashedLaneInfo.m_laneinfo) << " get post lane has multi results.";
          return false;*/
        }
      } else {
        // 获取当前车道的前一个车道连接集合
        auto preLaneLinkIdSet =
            HdMap::HadmapCacheConCurrent::GetLanePrevLinkSet(srcHashedLaneInfo.LaneInfo().onLaneUid);
        // 如果前一个车道连接集合非空
        if (_NonEmpty_(preLaneLinkIdSet)) {
          // 如果前一个车道连接集合只有一个元素
          if (1 == preLaneLinkIdSet.size()) {
            const Base::Info_Lane_t postLaneLink_locInfo = *(preLaneLinkIdSet.begin());
            // 根据元素获取对应的车道信息
            HdMap::txLaneInfoInterfacePtr geom_ptr =
                HdMap::HadmapCacheConCurrent::GetGeomInfoById(postLaneLink_locInfo);
            // 获取车道连接指针
            auto linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(postLaneLink_locInfo.onLinkFromLaneUid,
                                                                          postLaneLink_locInfo.onLinkToLaneUid);
            // 创建新的车道信息
            Base::Info_Lane_t newLaneInfo;
            newLaneInfo.FromLaneLink(postLaneLink_locInfo.onLinkId_without_equal, linkPtr->fromTxLaneId(),
                                     linkPtr->toTxLaneId());
            // 生成新的HashedLaneInfo
            retHashedLaneInfo =
                Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (geom_ptr->GetLength() - 0.5), newLaneInfo);
            return true;
          } else {
            // 如果前一个车道连接集合有多个元素，记录警告信息
            LOG(WARNING) << TX_VARS(srcHashedLaneInfo.LaneInfo()) << " get post lane-link has multi results.";
            return false;
          }
        } else {
          LOG(WARNING) << TX_VARS(srcHashedLaneInfo.LaneInfo())
                       << " get post lane/lane link has zero. may be map boundary.";
          return false;
        }
      }
    }
  } else {
    LOG(FATAL) << "GetPostHashedLaneInfo ERROR.";
  }
  return false;
}

HashedLaneInfo GenerateHashedLaneInfo(const HdMap::txLaneInfoInterfacePtr cur_geom_ptr, const Base::txFloat distOnCurce,
                                      const Base::Info_Lane_t& laneLocInfo) TX_NOEXCEPT {
  // 获取车道的长度
  const Base::txFloat curveLen = cur_geom_ptr->GetLength();
  // 初始化子路段索引
  Base::txSize subSectionIdx = 0;
  // 初始化子路段范围
  HashedLaneInfo::ScopeType subSectionScope;
  // 计算子路段索引和范围
  std::tie(subSectionIdx, subSectionScope) = HashedLaneInfo::ComputeSubSectionIdx(distOnCurce, curveLen);
  // 获取子路段起始点的ENU坐标
  Coord::txENU subSectionStartPt(cur_geom_ptr->GetLocalPos(std::get<0>(subSectionScope)));
  return HashedLaneInfo(laneLocInfo, subSectionIdx, subSectionScope, curveLen, subSectionStartPt.ToWGS84(), distOnCurce,
                        cur_geom_ptr);
}

// 检查HashedLaneInfo对象的子路段索引是否有效
Base::txBool CheckHashedValid(const Base::txSize subSectionIdx, Base::txFloat _curveLength) TX_NOEXCEPT {
  // 获取子路段长度
  static const Base::txFloat scopeLen = HashedLaneInfo::HashScopeLen();
  // 计算子路段起始点的距离
  const Base::txFloat scopeLen_subSectionIdx = (scopeLen * (subSectionIdx));
  // 判断子路段起始点的距离是否小于车道长度，如果是则返回true，否则返回false
  return ((scopeLen_subSectionIdx < _curveLength) ? (true) : (false));
}

HashedLaneInfo GenerateHashedLaneInfo(const Base::Info_Lane_t& laneLocInfo,
                                      const HdMap::txLaneInfoInterfacePtr cur_geom_ptr,
                                      const Base::txSize subSectionIdx, Base::txFloat curveLen) TX_NOEXCEPT {
  // 获取子路段长度
  static const Base::txFloat scopeLen = HashedLaneInfo::HashScopeLen();
  static const Base::txFloat half_scopeLen = 0.5 * HashedLaneInfo::HashScopeLen();

  // 计算子路段起始点的距离
  const Base::txFloat scopeLen_subSectionIdx = (scopeLen * (subSectionIdx));
  Base::txFloat distOnCurce = (scopeLen_subSectionIdx);
#  if 0
    if ((scopeLen_subSectionIdx + scopeLen) <= curveLen) {
        distOnCurce += (half_scopeLen);
    } else {
        distOnCurce += ((curveLen - (scopeLen_subSectionIdx)) * 0.5);
    }
#  endif

  // 根据子路段索引和车道长度计算距离
  if ((scopeLen_subSectionIdx < curveLen) && ((scopeLen_subSectionIdx + scopeLen) <= curveLen)) {
    distOnCurce += (half_scopeLen);
  } else if ((scopeLen_subSectionIdx < curveLen) && ((scopeLen_subSectionIdx + scopeLen) > curveLen)) {
    distOnCurce += ((curveLen - (scopeLen_subSectionIdx)) * 0.5);
  } else {
    return HashedLaneInfo();
  }

  // 计算子路段索引和范围
  HashedLaneInfo::ScopeType subSectionScope;
  Base::txSize resSubSectionIdx = 0;
  std::tie(resSubSectionIdx, subSectionScope) = HashedLaneInfo::ComputeSubSectionIdx(distOnCurce, curveLen);

  // 检查子路段索引和距离是否有效
  assert(distOnCurce <= curveLen && subSectionIdx == resSubSectionIdx);
  // 获取子路段起始点的ENU坐标
  Coord::txENU subSectionStartPt(cur_geom_ptr->GetLocalPos(std::get<0>(subSectionScope)));
  // 创建并返回HashedLaneInfo对象
  return HashedLaneInfo(laneLocInfo, subSectionIdx, subSectionScope, curveLen, subSectionStartPt.ToWGS84(), distOnCurce,
                        cur_geom_ptr);
}

Base::txBool GetFrontHashedLaneInfoList(const HashedLaneInfo& srcHashedLaneInfo,
                                        std::list<HashedLaneInfoPtr>& retHashedLaneInfoList,
                                        const Base::txInt nStep /* 3*16=48m */) TX_NOEXCEPT {
  // 如果步长小于等于0，直接返回true
  if (nStep <= 0) {
    return true;
  }

  // 获取子路段长度
  static const Base::txFloat sScopeLen = HashedLaneInfo::HashScopeLen();
  // 如果当前HashedLaneInfo不是最后一个子路段
  if (CallFail(srcHashedLaneInfo.IsLastSection())) {
    TX_MARK("not last section.");

    // 生成下一个子路段的HashedLaneInfo对象
    auto res = Geometry::SpatialQuery::GenerateHashedLaneInfo(srcHashedLaneInfo.LaneInfo(), srcHashedLaneInfo.GeomPtr(),
                                                              srcHashedLaneInfo.m_subSectionIdx + 1,
                                                              srcHashedLaneInfo.GeomPtr()->GetLength());
    HashedLaneInfoPtr resPtr = std::make_shared<HashedLaneInfo>(res);
    // 将新生成的HashedLaneInfo对象添加到结果列表中
    retHashedLaneInfoList.push_back(resPtr);
    // 递归调用，获取下一个子路段的前方HashedLaneInfo列表
    return GetFrontHashedLaneInfoList(res, retHashedLaneInfoList, nStep - 1);
  } else {
    TX_MARK("last section.");
    // 如果当前HashedLaneInfo在车道连接上
    if (srcHashedLaneInfo.m_laneinfo.IsOnLaneLink()) {
      TX_MARK("on lanelink");
      // 获取下一个车道的几何信息
      HdMap::txLaneInfoInterfacePtr geom_ptr =
          HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(srcHashedLaneInfo.LaneInfo().onLinkToLaneUid);
      // 如果下一个车道的几何信息有效
      if (NonNull_Pointer(geom_ptr)) {
        Base::Info_Lane_t newLaneInfo(srcHashedLaneInfo.LaneInfo().onLinkToLaneUid);
        // 生成下一个车道的HashedLaneInfo对象
        auto res = Geometry::SpatialQuery::GenerateHashedLaneInfo(newLaneInfo, geom_ptr, 0, geom_ptr->GetLength());
        HashedLaneInfoPtr resPtr = std::make_shared<HashedLaneInfo>(res);
        // 将新生成的HashedLaneInfo对象添加到结果列表中
        retHashedLaneInfoList.push_back(resPtr);
        // 递归调用，获取下一个车道的前方HashedLaneInfo列表
        return GetFrontHashedLaneInfoList(res, retHashedLaneInfoList, nStep - 1);
      } else {
        LOG(WARNING) << TX_VARS(srcHashedLaneInfo.m_laneinfo.onLinkFromLaneUid)
                     << TX_VARS(srcHashedLaneInfo.m_laneinfo.onLinkToLaneUid);
        return false;
      }
    } else {
      TX_MARK("on lane");
      // 获取下一个车道的集合
      auto nextLaneUidSet =
          HdMap::HadmapCacheConCurrent::GetNextLaneSetByLaneUid(srcHashedLaneInfo.m_laneinfo.onLaneUid);
      // 如果下一个车道的集合非空
      if (_NonEmpty_(nextLaneUidSet)) {
        for (const auto nextLaneUid : nextLaneUidSet) {
          HdMap::txLaneInfoInterfacePtr geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(nextLaneUid);
          if (NonNull_Pointer(geom_ptr)) {
            Base::Info_Lane_t newLaneInfo(nextLaneUid);
            // 生成下一个车道的HashedLaneInfo对象
            auto res = Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (0.1), newLaneInfo);
            HashedLaneInfoPtr resPtr = std::make_shared<HashedLaneInfo>(res);
            // 将新生成的HashedLaneInfo对象添加到结果列表中
            retHashedLaneInfoList.push_back(resPtr);
            // 递归调用，获取下一个车道的前方HashedLaneInfo列表
            GetFrontHashedLaneInfoList(res, retHashedLaneInfoList, nStep - 1);
          }
        }
      }
      // else
      {
        TX_MARK("next geometry is lanelink.");
        auto nextLaneLinkIdSet =
            HdMap::HadmapCacheConCurrent::GetLaneNextLinkSet(srcHashedLaneInfo.m_laneinfo.onLaneUid);
        // 如果下一个车道连接的集合非空
        if (_NonEmpty_(nextLaneLinkIdSet)) {
          for (const Base::Info_Lane_t nextLaneLink_locInfo : nextLaneLinkIdSet) {
            HdMap::txLaneInfoInterfacePtr geom_ptr =
                HdMap::HadmapCacheConCurrent::GetGeomInfoById(nextLaneLink_locInfo);
            if (NonNull_Pointer(geom_ptr)) {
              auto linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(nextLaneLink_locInfo.onLinkFromLaneUid,
                                                                            nextLaneLink_locInfo.onLinkToLaneUid);
              Base::Info_Lane_t newLaneInfo(nextLaneLink_locInfo.onLinkId_without_equal, linkPtr->fromTxLaneId(),
                                            linkPtr->toTxLaneId());
              // 生成下一个车道连接的HashedLaneInfo对象
              auto res = Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (0.1), newLaneInfo);
              HashedLaneInfoPtr resPtr = std::make_shared<HashedLaneInfo>(res);
              // 将新生成的HashedLaneInfo对象添加到结果列表中
              retHashedLaneInfoList.push_back(resPtr);
              // 递归调用，获取下一个车道连接的前方HashedLaneInfo列表
              GetFrontHashedLaneInfoList(res, retHashedLaneInfoList, nStep - 1);
            }
          }
          return true;
        }
        /*else {
            LOG(WARNING) << TX_VARS(srcHashedLaneInfo.m_laneinfo) << " get next lanelink has zero results.";
            return false;
        }*/
      }
    }
  }
  return false;
}

Base::txBool GetBackHashedLaneInfoList(const HashedLaneInfo& srcHashedLaneInfo,
                                       std::list<HashedLaneInfoPtr>& retHashedLaneInfoList,
                                       const Base::txInt nStep /* 3*16=48m */) TX_NOEXCEPT {
  const Base::txFloat step = 0.0;
  if (nStep <= 0) {
    return true;
  }

  // 如果当前HashedLaneInfo不是最后一个子路段
  if (srcHashedLaneInfo.m_subSectionIdx > 0) {
    // 生成上一个子路段的HashedLaneInfo对象
    auto res = Geometry::SpatialQuery::GenerateHashedLaneInfo(srcHashedLaneInfo.LaneInfo(), srcHashedLaneInfo.GeomPtr(),
                                                              srcHashedLaneInfo.m_subSectionIdx - 1,
                                                              srcHashedLaneInfo.GeomPtr()->GetLength());
    HashedLaneInfoPtr resPtr = std::make_shared<HashedLaneInfo>(res);
    // 将新生成的HashedLaneInfo对象添加到结果列表中
    retHashedLaneInfoList.push_back(resPtr);
    // 递归调用，获取上一个子路段的后方HashedLaneInfo列表
    return GetBackHashedLaneInfoList(res, retHashedLaneInfoList, nStep - 1);
  } else if (0 == srcHashedLaneInfo.m_subSectionIdx) {
    if (srcHashedLaneInfo.m_laneinfo.isOnLaneLink) {
      TX_MARK("on lanelink");
      // 获取下一个车道的几何信息
      HdMap::txLaneInfoInterfacePtr geom_ptr =
          HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(srcHashedLaneInfo.m_laneinfo.onLinkFromLaneUid);
      // 如果下一个车道的几何信息有效
      if (NonNull_Pointer(geom_ptr)) {
        Base::Info_Lane_t newLaneInfo(srcHashedLaneInfo.m_laneinfo.onLinkFromLaneUid);
        // 生成下一个车道的HashedLaneInfo对象
        auto res =
            Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (geom_ptr->GetLength() - step), newLaneInfo);
        HashedLaneInfoPtr resPtr = std::make_shared<HashedLaneInfo>(res);
        // 将新生成的HashedLaneInfo对象添加到结果列表中
        retHashedLaneInfoList.push_back(resPtr);
        // 递归调用，获取下一个车道的后方HashedLaneInfo列表
        return GetBackHashedLaneInfoList(res, retHashedLaneInfoList, nStep - 1);
      } else {
        auto linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(srcHashedLaneInfo.m_laneinfo.onLinkFromLaneUid,
                                                                      srcHashedLaneInfo.m_laneinfo.onLinkToLaneUid);
        // 如果linkPtr指针非空
        if (NonNull_Pointer(linkPtr)) {
          // 输出警告日志，包含当前HashedLaneInfo的onLinkFromLaneUid和onLinkToLaneUid信息
          LOG(WARNING) << TX_VARS(srcHashedLaneInfo.m_laneinfo.onLinkFromLaneUid)
                       << TX_VARS(srcHashedLaneInfo.m_laneinfo.onLinkToLaneUid);
        }
        return true;
      }
    } else {
      TX_MARK("on lane");
      auto preLaneUidSet = HdMap::HadmapCacheConCurrent::GetPreLaneSetByLaneUid(srcHashedLaneInfo.m_laneinfo.onLaneUid);
      // 如果前方车道的集合非空
      if (_NonEmpty_(preLaneUidSet)) {
        // 遍历前方车道的集合
        for (const auto postLaneUid : preLaneUidSet) {
          // 获取前方车道的几何信息
          HdMap::txLaneInfoInterfacePtr geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(postLaneUid);
          // 如果前方车道的几何信息有效
          if (NonNull_Pointer(geom_ptr)) {
            Base::Info_Lane_t newLaneInfo(postLaneUid);
            // 生成前方车道的HashedLaneInfo对象
            auto res =
                Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (geom_ptr->GetLength() - step), newLaneInfo);
            HashedLaneInfoPtr resPtr = std::make_shared<HashedLaneInfo>(res);
            // 将新生成的HashedLaneInfo对象添加到结果列表中
            retHashedLaneInfoList.push_back(resPtr);
            // 递归调用，获取前方车道的后方HashedLaneInfo列表
            GetBackHashedLaneInfoList(res, retHashedLaneInfoList, nStep - 1);
          }
        }
        // return true;
      }
      // else
      {
        // 获取前方车道连接的集合
        auto preLaneLinkIdSet =
            HdMap::HadmapCacheConCurrent::GetLanePrevLinkSet(srcHashedLaneInfo.m_laneinfo.onLaneUid);
        // 如果前方车道连接的集合非空
        if (_NonEmpty_(preLaneLinkIdSet)) {
          // 遍历前方车道连接的集合
          for (const Base::Info_Lane_t& postLaneLink_locInfo : preLaneLinkIdSet) {
            TX_MARK("filter out lanes with multiple lanelinks at one end. Fixed simulation consistency issues. ");
            auto nextLaneLinkIdSet =
                HdMap::HadmapCacheConCurrent::GetLaneNextLinkSet(postLaneLink_locInfo.onLinkFromLaneUid);
            if (_NonEmpty_(nextLaneLinkIdSet) && nextLaneLinkIdSet.size() > 1) {
              continue;
            }
            // 获取前方车道连接的几何信息
            HdMap::txLaneInfoInterfacePtr geom_ptr =
                HdMap::HadmapCacheConCurrent::GetGeomInfoById(postLaneLink_locInfo);

            if (NonNull_Pointer(geom_ptr)) {
              auto linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(postLaneLink_locInfo.onLinkFromLaneUid,
                                                                            postLaneLink_locInfo.onLinkToLaneUid);
              Base::Info_Lane_t newLaneInfo;
              newLaneInfo.FromLaneLink(postLaneLink_locInfo.onLinkId_without_equal, linkPtr->fromTxLaneId(),
                                       linkPtr->toTxLaneId());
              // 生成前方车道连接的HashedLaneInfo对象
              auto res =
                  Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, (geom_ptr->GetLength() - step), newLaneInfo);
              HashedLaneInfoPtr resPtr = std::make_shared<HashedLaneInfo>(res);
              // 将新生成的HashedLaneInfo对象添加到结果列表中
              retHashedLaneInfoList.push_back(resPtr);
              // 递归调用，获取前方车道连接的后方HashedLaneInfo列表
              GetBackHashedLaneInfoList(res, retHashedLaneInfoList, nStep - 1);
            }
          }

          return true;
        }
        /*else {
            LOG(WARNING) << TX_VARS(srcHashedLaneInfo.m_laneinfo) << " get post lane/lane link has zero. may be map
        boundary."; return true;
        }*/
      }
    }
  } else {
    LOG(FATAL) << "GetPostHashedLaneInfo ERROR.";
  }
  return true;
}

Base::txBool GetLeftHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo,
                                   HashedLaneInfoPtr& retHashedLaneInfoPtr) TX_NOEXCEPT {
  retHashedLaneInfoPtr = nullptr;
  // 如果当前HashedLaneInfo在车道上
  if (srcHashedLaneInfo.m_laneinfo.IsOnLane()) {
    // 如果当前车道有左侧车道
    if (Utils::HasLeftLane(srcHashedLaneInfo.m_laneinfo.onLaneUid)) {
      // 获取左侧车道的Uid
      auto leftLaneUid = Utils::LeftLaneUid(srcHashedLaneInfo.m_laneinfo.onLaneUid);
      Base::Info_Lane_t newLaneInfo(leftLaneUid);
      // 获取左侧车道的几何信息
      HdMap::txLaneInfoInterfacePtr left_geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(leftLaneUid);
      // 如果左侧车道的几何信息有效
      if (NonNull_Pointer(left_geom_ptr)) {
        // 如果当前HashedLaneInfo的子路段索引有效
        if (Geometry::SpatialQuery::CheckHashedValid(srcHashedLaneInfo.m_subSectionIdx, left_geom_ptr->GetLength())) {
          // 生成左侧车道的HashedLaneInfo对象
          auto res = Geometry::SpatialQuery::GenerateHashedLaneInfo(
              newLaneInfo, left_geom_ptr, srcHashedLaneInfo.m_subSectionIdx, left_geom_ptr->GetLength());
          retHashedLaneInfoPtr = std::make_shared<HashedLaneInfo>(res);
          return true;
        } else if (srcHashedLaneInfo.m_subSectionIdx > 0) {  // 如果当前HashedLaneInfo的子路段索引无效，但大于0
          // 生成左侧车道的HashedLaneInfo对象
          auto res = Geometry::SpatialQuery::GenerateHashedLaneInfo(
              newLaneInfo, left_geom_ptr, srcHashedLaneInfo.m_subSectionIdx - 1, left_geom_ptr->GetLength());
          retHashedLaneInfoPtr = std::make_shared<HashedLaneInfo>(res);
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool GetRightHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo,
                                    HashedLaneInfoPtr& retHashedLaneInfoPtr) TX_NOEXCEPT {
  retHashedLaneInfoPtr = nullptr;
  // 如果当前HashedLaneInfo在车道上
  if (srcHashedLaneInfo.m_laneinfo.IsOnLane()) {
    // 如果当前车道有右侧车道
    if (Utils::HasRightLane(
            srcHashedLaneInfo.m_laneinfo.onLaneUid,
            HdMap::HadmapCacheConCurrent::GetLaneCountOnSection(std::make_pair(
                srcHashedLaneInfo.m_laneinfo.onLaneUid.roadId, srcHashedLaneInfo.m_laneinfo.onLaneUid.sectionId)))) {
      // 获取右侧车道的Uid
      auto rightLaneUid = Utils::RightLaneUid(srcHashedLaneInfo.m_laneinfo.onLaneUid);
      Base::Info_Lane_t newLaneInfo(rightLaneUid);
      // 获取右侧车道的几何信息
      HdMap::txLaneInfoInterfacePtr right_geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(rightLaneUid);
      // 如果右侧车道的几何信息有效
      if (NonNull_Pointer(right_geom_ptr)) {
        // 如果当前HashedLaneInfo的子路段索引有效
        if (Geometry::SpatialQuery::CheckHashedValid(srcHashedLaneInfo.m_subSectionIdx, right_geom_ptr->GetLength())) {
          // 生成右侧车道的HashedLaneInfo对象
          auto res = Geometry::SpatialQuery::GenerateHashedLaneInfo(
              newLaneInfo, right_geom_ptr, srcHashedLaneInfo.m_subSectionIdx, right_geom_ptr->GetLength());
          retHashedLaneInfoPtr = std::make_shared<HashedLaneInfo>(res);
          return true;
        } else if (srcHashedLaneInfo.m_subSectionIdx > 0) {  // 如果当前HashedLaneInfo的子路段索引无效，但大于0
          // 生成右侧车道的HashedLaneInfo对象
          auto res = Geometry::SpatialQuery::GenerateHashedLaneInfo(
              newLaneInfo, right_geom_ptr, srcHashedLaneInfo.m_subSectionIdx - 1, right_geom_ptr->GetLength());
          retHashedLaneInfoPtr = std::make_shared<HashedLaneInfo>(res);
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool MoveFrontHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo, const Base::txFloat distance,
                                     HashedLaneInfoPtr& retHashedLaneInfoPtr) TX_NOEXCEPT {
  return false;
}

Base::txBool MoveBackHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo, const Base::txFloat distance,
                                    HashedLaneInfoPtr& retHashedLaneInfoPtr) TX_NOEXCEPT {
  return false;
}

HashedLaneInfo::~HashedLaneInfo() {
  // LOG(WARNING) << "[~] HashedLaneInfo.";
}
#endif /*USE_HashedRoadNetwork*/

TX_NAMESPACE_CLOSE(SpatialQuery)
TX_NAMESPACE_CLOSE(Geometry)
