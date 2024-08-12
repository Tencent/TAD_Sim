// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "hadmap.h"
#include "mapengine/engine_defs.h" /*hadmap::MAP_DATA_TYPE*/
#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"
#include "tx_header.h"
#include "tx_path_utils.h"
#include "tx_units.h"

TX_NAMESPACE_OPEN(Utils)

/**
 * @brief 检查给定的道路类型是否为行驶道路
 * @param laneType 要检查的道路类型
 * @return 如果道路类型为行驶道路则返回 true，否则返回 false
 */
inline Base::txBool IsDrivingLane(const hadmap::LANE_TYPE laneType) TX_NOEXCEPT {
  return (laneType == hadmap::LANE_TYPE_Driving || laneType == hadmap::LANE_TYPE_TollBooth ||
          laneType == hadmap::LANE_TYPE_ENTRY || laneType == hadmap::LANE_TYPE_EXIT);
}

/**
 * @brief 判断给定的车道是否有效
 * @param _pLane 待判断的车道指针
 * @return 如果车道有效则返回 true，否则返回 false
 */
inline Base::txBool IsLaneValid(const hadmap::txLanePtr _pLane) TX_NOEXCEPT {
  return ((_pLane) && (_pLane->getGeometry()) && (_pLane->getGeometry()->getLength() > 0.0) &&
          // (IsDrivingLane(_pLane->getLaneType())) &&
          (!(_pLane->getGeometry()->empty())));
}

/**
 * @brief 检查给定的车道是否为可行驾驶车道
 * @param _pLane 待检查的车道指针
 * @return 如果车道为可行驾驶车道则返回 true，否则返回 false
 */
inline Base::txBool IsLaneValidDriving(const hadmap::txLanePtr _pLane) TX_NOEXCEPT {
  return ((_pLane) && (_pLane->getGeometry()) && (_pLane->getGeometry()->getLength() > 0.0) &&
          (IsDrivingLane(_pLane->getLaneType())) && (!(_pLane->getGeometry()->empty())));
}

/**
 * @brief 判断给定的车道是否为可行驾驶车道
 * @param _pLane 待检查的车道指针
 * @param strLog 日志字符串
 * @return 如果车道为可行驾驶车道则返回 true，否则返回 false
 */
inline Base::txBool IsLaneValidDriving(const hadmap::txLanePtr _pLane, Base::txString& strLog) TX_NOEXCEPT {
  std::stringstream ss;
  ss << TX_COND(NonNull_Pointer(_pLane)) << TX_COND(NonNull_Pointer(_pLane->getGeometry()))
     << TX_COND((_pLane->getGeometry()->getLength() > 0.0)) << TX_COND((IsDrivingLane(_pLane->getLaneType())))
     << TX_VARS_NAME(laneType, _pLane->getLaneType()) << TX_COND((!(_pLane->getGeometry()->empty())));
  strLog = ss.str();
  return IsLaneValidDriving(_pLane);
}

/**
 * @brief 判断给定的车道连接是否有效
 * @param _pLaneLink 待检查的车道连接指针
 * @return 如果车道连接有效则返回 true，否则返回 false
 */
inline Base::txBool IsLaneLinkValid(const hadmap::txLaneLinkPtr _pLaneLink) TX_NOEXCEPT {
  return (NonNull_Pointer(_pLaneLink)) && (NonNull_Pointer(_pLaneLink->getGeometry())) &&
         (_pLaneLink->getGeometry()->getLength() > 0.0) && (!(_pLaneLink->getGeometry()->empty()));
}

/**
 * @brief 检查给定的车道连接是否在一条道路中有效
 * @param _pLaneLink 待检查的车道连接指针
 * @return 如果车道连接在道路中有效则返回 true，否则返回 false
 */
inline Base::txBool IsLaneLinkValid_InRoad(const hadmap::txLaneLinkPtr _pLaneLink) TX_NOEXCEPT {
  return (NonNull_Pointer(_pLaneLink));
}

/**
 * @brief 从文件路径中获取地图数据类型
 * @param strMapFile 地图文件路径
 * @return 地图数据类型
 */
inline hadmap::MAP_DATA_TYPE ComputeMapType(const Base::txString& strMapFile) TX_NOEXCEPT {
  hadmap::MAP_DATA_TYPE retType = hadmap::MAP_DATA_TYPE::SQLITE;
  Base::txString strExtension;
  if (Utils::GetFileExtension(strMapFile, strExtension)) {
    boost::algorithm::to_lower(strExtension);
    if (Base::txString(".xodr") == strExtension) {
      retType = hadmap::MAP_DATA_TYPE::OPENDRIVE;
    }
  }
  return retType;
}

/**
 * @brief 检查返回代码是否正确
 *
 * 检查给定的返回代码是否正确。
 *
 * @param optCode 要检查的返回代码
 * @return true 代码正确
 * @return false 代码错误
 */
inline Base::txBool CheckRetCode(const Base::txInt optCode) TX_NOEXCEPT {
  if (optCode != TX_HADMAP_DATA_OK && optCode != TX_HADMAP_DATA_EMPTY && optCode != TX_HADMAP_HANDLE_OK &&
      optCode != TX_HADMAP_OK) {
    return false;
  } else {
    return true;
  }
}

/**
 * @brief 检查两个laneId是否相等
 * @param Left_txLaneId 要比较的第一个laneId
 * @param Right_txLaneId 要比较的第二个laneId
 * @return true 两个laneId相等
 * @return false 两个laneId不相等
 */
inline Base::txBool IsEqual(const hadmap::txLaneId Left_txLaneId, const hadmap::txLaneId Right_txLaneId) TX_NOEXCEPT {
  return (Left_txLaneId.roadId == Right_txLaneId.roadId) && (Left_txLaneId.sectionId == Right_txLaneId.sectionId) &&
         (Left_txLaneId.laneId == Right_txLaneId.laneId);
}

/**
 * @brief 检查两个道路是否相等
 *
 * @param pLeftLane 要比较的第一个道路指针
 * @param pRightLane 要比较的第二个道路指针
 * @return true 两个道路相等
 * @return false 两个道路不相等
 */
inline Base::txBool IsEqual(const hadmap::txLanePtr pLeftLane, const hadmap::txLanePtr pRightLane) TX_NOEXCEPT {
  if (pLeftLane && pRightLane) {
    return IsEqual(pLeftLane->getTxLaneId(), pRightLane->getTxLaneId());
  } else {
    return false;
  }
}

/**
 * @brief 检查两个道路是否不相等
 *
 * @param pLeftLane 要比较的第一个道路指针
 * @param pRightLane 要比较的第二个道路指针
 * @return true 两个道路不相等
 * @return false 两个道路相等
 */
inline Base::txBool IsNotEqual(const hadmap::txLanePtr pLeftLane, const hadmap::txLanePtr pRightLane) TX_NOEXCEPT {
  return !IsEqual(pLeftLane, pRightLane);
}

/**
 * @brief 计算给定方向向量在ENU坐标系中对应的车道角度
 *
 * 计算给定方向向量在ENU坐标系中的车道角度，方向向量应为沿东北方向的单位向量。
 *
 * @param vDir 给定方向向量，应为沿东北方向的单位向量
 * @return 车道角度，范围为[0, 360)，单位为度
 */
inline Unit::txDegree GetLaneAngleFromVectorOnENU(const Base::txVec3& vDir) TX_NOEXCEPT {
  return Unit::txRadian::MakeRadian(Math::Atan2(__North__(vDir), __East__(vDir))).ToDegree();
}

/**
 * @brief 从给定方向向量计算沿ENU坐标系的车道角度
 *
 * 计算沿ENU坐标系的车道角度，方向向量应该是沿东北方向的单位向量。
 *
 * @param vDir 给定方向向量，应该是沿东北方向的单位向量
 * @return 车道角度，范围为[0, 360)，单位为度
 */
inline Unit::txDegree DisplayerGetLaneAngleFromVector(const Base::txVec3& vDir) TX_NOEXCEPT {
  return GetLaneAngleFromVectorOnENU(vDir);
}

/**
 * @brief 根据给定的laneID计算出对应的索引
 *
 * 此函数将输入的laneID转换为与之对应的索引。
 *
 * @param _goalLaneId 给定的laneID
 * @return 返回与输入laneID对应的索引
 */
inline Base::txLaneID ComputeLaneIndex(const Base::txLaneID _goalLaneId) TX_NOEXCEPT {
  /*
  -1 -> 0
  -2 -> 1
  -3 -> 2
  */
  return (_goalLaneId + 1) * -1;
}

/**
 * @brief 检查给定的车道ID是否代表第一个section
 *
 * 该函数检查给定的车道ID是否具有第一个section的特征。
 *
 * @param laneUid 要检查的车道ID
 * @return 如果给定的车道ID代表第一个section，则返回true，否则返回false
 */
inline Base::txBool isFirstSection(const Base::txLaneUId& laneUid) TX_NOEXCEPT { return (0 == laneUid.sectionId); }

/**
 * @brief 检查给定的车道ID是否为最后一个section
 *
 * 该函数检查给定的车道ID是否具有最后一个section的特征。
 *
 * @param laneUid 要检查的车道ID
 * @param nSectionSize 车道分为多少个部分
 * @return 如果给定的车道ID是最后一个section，则返回true，否则返回false
 */
inline Base::txBool isLastSection(const Base::txLaneUId& laneUid, const Base::txSize nSectionSize) TX_NOEXCEPT {
  return ((nSectionSize - 1) == laneUid.sectionId);
}

/**
 * @brief 获取下一个section id
 *
 * @param _secId 当前的section id
 * @return Base::txSectionID
 */
inline Base::txSectionID NextSectionId(const Base::txSectionID _secId) TX_NOEXCEPT { return (_secId + 1); }

/**
 * @brief 获取前一个section id
 *
 * @param _secId 当前的section id
 * @return Base::txSectionID
 */
inline Base::txSectionID PrevSectionId(const Base::txSectionID _secId) TX_NOEXCEPT { return (_secId - 1); }

/**
 * @brief 是否到达道路的最后一个路段
 *
 * @param lanePtr 车道指针
 * @return Base::txBool 到达最后一个路段返回true
 */
inline Base::txBool IsReachedRoadLastSection(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT {
  if (NonNull_Pointer(lanePtr)) {
    const Base::txLaneUId laneUid = lanePtr->getTxLaneId();
    const Base::txSize roadHasSectionCnt = HdMap::HadmapCacheConCurrent::GetSectionCountOnRoad(laneUid.roadId);
    return (laneUid.sectionId == (roadHasSectionCnt - 1));
  }
  return false;
}

/**
 * @brief 当前车道是否右左车道
 *
 * @param curLaneUid 当前车道id
 * @return Base::txBool 存在左车道返回true
 */
inline Base::txBool HasLeftLane(const Base::txLaneUId& curLaneUid) TX_NOEXCEPT { return (curLaneUid.laneId < -1); }

/**
 * @brief 获取当前车道的左侧车道
 *
 * 通过调用Hadmap缓存库中的QueryTrick_L函数，来获取当前车道的左侧车道。如果函数调用成功，则返回对应的左侧车道ID；否则，根据当前车道ID计算出左侧车道ID。
 *
 * @param curLaneUid 当前车道ID
 * @return 返回当前车道的左侧车道ID
 */
inline Base::txLaneUId LeftLaneUid(const Base::txLaneUId& curLaneUid) TX_NOEXCEPT {
  Base::txLaneUId leftLaneUid(curLaneUid);
  if (CallSucc(HdMap::HadmapCacheConCurrent::QueryTrick_L(curLaneUid, leftLaneUid))) {
    return std::move(leftLaneUid);
  }
  return Base::txLaneUId(curLaneUid.roadId, curLaneUid.sectionId, curLaneUid.laneId + 1);
}

/**
 * @brief 获取当前车道的左侧车道
 *
 * 该函数接收一个车道ID作为输入，并返回该车道的左侧车道。如果输入车道有效，返回对应的左侧车道；否则，根据输入车道ID计算出左侧车道。
 *
 * @param curLaneUid 当前车道ID
 * @return 返回当前车道的左侧车道ID
 */
inline Base::txLaneUId LeftLaneUid_UnCheck(const Base::txLaneUId& curLaneUid) TX_NOEXCEPT {
  return LeftLaneUid(curLaneUid);
}

/**
 * @brief 判断当前车道是否有右侧车道
 *
 * 判断当前车道的ID是否小于车道总数，如果是则表示当前车道有右侧车道；否则表示当前车道没有右侧车道。
 *
 * @param curLaneUid 当前车道的ID
 * @param szLane 车道总数
 * @return 如果当前车道有右侧车道，返回true；否则返回false
 */
inline Base::txBool HasRightLane(const Base::txLaneUId& curLaneUid, const Base::txSize szLane) TX_NOEXCEPT {
  return ((-1 * curLaneUid.laneId) < szLane);
}

/**
 * @brief 获取当前车道的右侧车道ID
 *
 * 通过调用HdMap库中的QueryTrick_R函数，获取当前车道的右侧车道ID。如果成功获取到右侧车道ID，则返回右侧车道ID；否则返回当前车道ID的左侧车道ID。
 *
 * @param curLaneUid 当前车道的ID
 * @return 返回当前车道的右侧车道ID，如果无法获取则返回左侧车道ID
 */
inline Base::txLaneUId RightLaneUid(const Base::txLaneUId& curLaneUid) TX_NOEXCEPT {
  Base::txLaneUId rightLaneUid(curLaneUid);
  if (CallSucc(HdMap::HadmapCacheConCurrent::QueryTrick_R(curLaneUid, rightLaneUid))) {
    return std::move(rightLaneUid);
  }
  return Base::txLaneUId(curLaneUid.roadId, curLaneUid.sectionId, curLaneUid.laneId - 1);
}

/**
 * @brief 获取当前车道的右侧车道ID
 *
 * 通过调用HdMap库中的QueryTrick_R函数，获取当前车道的右侧车道ID。如果成功获取到右侧车道ID，则返回右侧车道ID；否则返回当前车道ID的左侧车道ID。
 *
 * @param curLaneUid 当前车道的ID
 * @return 返回当前车道的右侧车道ID，如果无法获取则返回左侧车道ID
 */
inline Base::txLaneUId RightLaneUid_UnCheck(const Base::txLaneUId& curLaneUid) TX_NOEXCEPT {
  return RightLaneUid(curLaneUid);
}

/**
 * @brief 检查给定的WGS84坐标是否有效
 *
 * 判断给定的WGS84坐标是否有效，只需判断经度和纬度是否不为零即可。
 *
 * @param _wgs84 待检查的WGS84坐标
 * @return true 如果经度和纬度都不为零，则返回true
 * @return false 如果经度或纬度任意一个为零，则返回false
 */
inline Base::txBool IsGPSValid(const hadmap::txPoint& _wgs84) TX_NOEXCEPT {
  return true;
  return Math::isNotZero(__Lon__(_wgs84)) && Math::isNotZero(__Lat__(_wgs84));
}

/**
 * @brief 判断给定的道路ID是否有效
 *
 * 判断给定的道路ID是否有效，有效则返回true，否则返回false。
 *
 * @param _road_id 待检查的道路ID
 * @return true 如果道路ID有效，则返回true
 * @return false 如果道路ID无效，则返回false
 */
inline Base::txBool IsRoadValid(const /*hadmap::roadpkid*/ TX_MARK("route result may be -1")
                                    Base::txSysId _road_id) TX_NOEXCEPT {
  return (_road_id >= 0);
}

/**
 * @brief 获取道路的左右停车线信息
 *
 * 获取道路的左右停车线信息，包括停车线的坐标点。如果成功获取，则返回true，否则返回false。
 *
 * @param road_ptr 待获取的道路指针
 * @param left_pt 存储左停车线坐标点的变量
 * @param right_pt 存储右停车线坐标点的变量
 * @return true 如果成功获取，则返回true
 * @return false 如果未成功获取，则返回false
 */
inline Base::txBool GetRoadStopLine(const hadmap::txRoadPtr road_ptr, Coord::txWGS84& left_pt,
                                    Coord::txWGS84& right_pt) TX_NOEXCEPT {
  if (NonNull_Pointer(road_ptr) && CallSucc(road_ptr->getLength() > 0.0) && _NonEmpty_(road_ptr->getSections()) &&
      NonNull_Pointer(road_ptr->getSections().back()) && _NonEmpty_(road_ptr->getSections().back()->getLanes())) {
    const auto& refLanes = road_ptr->getSections().back()->getLanes();
    if (NonNull_Pointer(refLanes.front()->getLeftBoundary()) &&
        NonNull_Pointer(refLanes.front()->getLeftBoundary()->getGeometry()) &&
        NonNull_Pointer(refLanes.back()->getRightBoundary()) &&
        NonNull_Pointer(refLanes.back()->getRightBoundary()->getGeometry())) {
      left_pt.FromWGS84(refLanes.front()->getLeftBoundary()->getGeometry()->getEnd());
      right_pt.FromWGS84(refLanes.back()->getRightBoundary()->getGeometry()->getEnd());
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

/**
 * @brief 检查两个道路ID是否在同一道路段上
 *
 * 检查两个道路ID是否在同一道路段上，如果不在同一道路段上则返回true，否则返回false。
 *
 * @param fromRid 第一个道路ID
 * @param toRid 第二个道路ID
 * @return true 如果两个道路ID不在同一道路段上，则返回true
 * @return false 如果两个道路ID在同一道路段上，则返回false
 */
inline Base::txBool IsOnLink(const Base::txRoadID fromRid, const Base::txRoadID toRid) TX_NOEXCEPT {
  return (fromRid != toRid);
}

/**
 * @brief 检查两个道路ID是否在同一车道上
 *
 * 检查两个道路ID是否在同一车道上，如果不在同一车道上则返回true，否则返回false。
 *
 * @param fromRid 第一个道路ID
 * @param toRid 第二个道路ID
 * @return true 如果两个道路ID不在同一车道上，则返回true
 * @return false 如果两个道路ID在同一车道上，则返回false
 */
inline Base::txBool IsOnLane(const Base::txRoadID fromRid, const Base::txRoadID toRid) TX_NOEXCEPT {
  return !(IsOnLink(fromRid, toRid));
}

/**
 * @brief 将车道编号转换为道路段编号
 *
 * 将给定的车道编号转换为道路段编号，即将道路编号转换为其所属的道路段编号。
 *
 * @param laneUid 要转换的道路编号
 * @return 转换后的道路段编号
 */
inline Base::txSectionUId LaneUid2SectionUid(const Base::txLaneUId& laneUid) TX_NOEXCEPT {
  return std::make_pair(laneUid.roadId, laneUid.sectionId);
}

/**
 * @brief 将道路编号和道路段编号组合成道路段编号
 *
 * 使用给定的道路编号和道路段编号创建一个新的道路段编号。
 *
 * @param rid 道路编号
 * @param sid 道路段编号
 * @return 新的道路段编号
 */
inline Base::txSectionUId MakeSectionUid(const Base::txRoadID rid, const Base::txSectionID sid) TX_NOEXCEPT {
  return std::make_pair(rid, sid);
}

/**
 * @brief MaxMin 类的实现，用于获取某向量数组中的最大值和最小值
 *
 * 类的功能是通过输入向量数组获取其最大值和最小值，
 * 使用tbb中的tbb::split实现并行处理
 */
class MaxMin {
 public:
  using txFloat = Base::txFloat;
  using txVec3 = Base::txVec3;
  using EnuPointVec = std::vector<txVec3>;

 protected:
  const EnuPointVec& mArray;
  txVec3 m_max;
  txVec3 m_min;

 public:
  /**
   * @brief 获取最大向量
   *
   * @return txVec3
   */
  txVec3 Max() const TX_NOEXCEPT { return m_max; }

  /**
   * @brief 获取最小向量
   *
   * @return txVec3
   */
  txVec3 Min() const TX_NOEXCEPT { return m_min; }
  void operator()(const tbb::blocked_range<size_t>& r) TX_NOEXCEPT {
    txVec3 cur_max = txVec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    txVec3 cur_min = txVec3(FLT_MAX, FLT_MAX, FLT_MAX);
    for (size_t i = r.begin(); i != r.end(); ++i) {
      const auto& curPt = mArray[i];
      std::tie(cur_max, cur_min) = join(cur_max, cur_min, curPt);
    }
    m_max = cur_max;
    m_min = cur_min;
  }

  /**
   * @brief 创建一个新的 MaxMin 类的实例，通过调用构造函数创建并行处理任务。
   *
   * 该函数接收一个 MaxMin 类的引用，通过 tbb::split 实现并行处理任务。
   *
   * @param x 需要创建并行任务的 MaxMin 类的实例
   * @param split 通过 tbb::split 实现并行处理任务
   */
  MaxMin(MaxMin& x, tbb::split) : mArray(x.mArray) {
    m_max = txVec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    m_min = txVec3(FLT_MAX, FLT_MAX, FLT_MAX);
  }
  static std::tuple<txVec3, txVec3> join(txVec3 spMax, txVec3 spMin, const txVec3& spPoint) TX_NOEXCEPT {
    if (spMax[0] < spPoint[0]) {
      spMax[0] = spPoint[0];
    }
    if (spMax[1] < spPoint[1]) {
      spMax[1] = spPoint[1];
    }
    if (spMax[2] < spPoint[2]) {
      spMax[2] = spPoint[2];
    }

    if (spMin[0] > spPoint[0]) {
      spMin[0] = spPoint[0];
    }
    if (spMin[1] > spPoint[1]) {
      spMin[1] = spPoint[1];
    }
    if (spMin[2] > spPoint[2]) {
      spMin[2] = spPoint[2];
    }
    return std::make_tuple(spMax, spMin);
  }

  /**
   * @brief 将两个MaxMin实例进行并行处理
   *
   * 此函数将两个MaxMin实例进行并行处理，并将结果合并到当前MaxMin实例。
   *
   * @param y 要进行并行处理的MaxMin实例
   */
  void join(const MaxMin& y) TX_NOEXCEPT {
    std::tie(m_max, m_min) = join(m_max, m_min, y.m_max);
    std::tie(m_max, m_min) = join(m_max, m_min, y.m_min);
  }

  /**
   * @brief MaxMin 构造函数，用于根据输入点的集合计算最大值和最小值
   *
   * @param a 一个 EnuPointVec 类型的对象，表示输入点的集合
   */
  explicit MaxMin(EnuPointVec a) : mArray(a) {
    m_max = txVec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    m_min = txVec3(FLT_MAX, FLT_MAX, FLT_MAX);
  }
};

TX_NAMESPACE_CLOSE(Utils)
