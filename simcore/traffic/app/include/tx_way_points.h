// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <set>
#include "tx_container_defs.h"
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_locate_info.h"
#include "tx_sim_point.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class WayPoints {
 public:
  using WayPointType = Base::Enums::WayPointType;

 public:
  WayPoints() TX_DEFAULT;
  ~WayPoints() TX_DEFAULT;

  /**
   * @brief 根据经纬度类型的结束点进行初始化
   *
   * @param endPt
   * @return Base::txBool
   */
  Base::txBool Initialize(Coord::txWGS84& endPt) TX_NOEXCEPT;

  /**
   * @brief 根据roadId和sectionId进行初始化
   *
   * @param _roadId 道路id
   * @param _sectionId section id
   * @return Base::txBool
   */
  Base::txBool Initialize(const Base::txRoadID _roadId, const Base::txSectionID _sectionId) TX_NOEXCEPT {
    _type = WayPointType::Road_Section;
    _deadline_LaneUid.roadId = _roadId;
    _deadline_LaneUid.sectionId = _sectionId;
    _valid = true;
    return _valid;
  }

  /**
   * @brief 根据linkId和fromLaneId和toLaneUid进行初始化
   *
   * @param _linkId 路口连接线id
   * @param fromLaneId 起始车道id
   * @param toLaneUid  结束车道id
   * @return Base::txBool
   */
  Base::txBool Initialize(const Base::txLaneLinkID _linkId, const Base::txLaneUId fromLaneId,
                          const Base::txLaneUId toLaneUid) TX_NOEXCEPT {
    _type = WayPointType::LaneLink;
    _link_locInfo.FromLaneLink(_linkId, fromLaneId, toLaneUid);
    _valid = true;
    return _valid;
  }

  /**
   * @brief 根据位置信息集合进行初始化
   *
   * @param _linkIdSet
   * @return Base::txBool
   */
  Base::txBool Initialize(const Base::LocInfoSet& _linkIdSet) TX_NOEXCEPT {
    _type = WayPointType::LaneLink;
    _all_link_set.clear();
    _all_link_set.insert(_linkIdSet.begin(), _linkIdSet.end());
    _valid = true;
    return _valid;
  }

  /**
   * @brief 判断元素是否到达预设目标点区域
   *
   * @param elemLaneUid 元素所在的道路的唯一标识
   * @param elementPt 元素在当前道路上的点坐标
   * @return true 当前元素到达预设目标点区域
   * @return false 当前元素未到达预设目标点区域
   */
  Base::txBool ArriveAtWayPointArea(const Base::txLaneUId& elemLaneUid,
                                    const Coord::txENU& elementPt) const TX_NOEXCEPT {
    if (_valid && _plus_(WayPointType::Point) == _type) {
      if (elemLaneUid.roadId == _deadline_LaneUid.roadId && elemLaneUid.sectionId == _deadline_LaneUid.sectionId) {
        const Base::txFloat distToEnd = Coord::txENU::EuclideanDistanceLocal_2D(elementPt, _EndPos);
        if (distToEnd < FLAGS_Finish_Radius) {
          return true;
        }
      }
    }
    return false;
  }

  /**
   * @brief 判断当前位置是否在指定道路上的指定区域内
   *
   * @param _roadId 道路的唯一ID
   * @param _sectionId 区域ID
   * @return Base::txBool true 当前位置在指定道路上的指定区域内
   */
  Base::txBool ArriveAtWayPointArea(const Base::txRoadID _roadId,
                                    const Base::txSectionID _sectionId) const TX_NOEXCEPT {
    if (_valid /*&& WayPointType::Road_Section == _type*/) {
      if (_roadId == _deadline_LaneUid.roadId && _sectionId == _deadline_LaneUid.sectionId) {
        return true;
      }
    }
    return false;
  }

  /**
   * @brief 判断当前位置是否在指定的车道连接区域内
   * @param _linkId 车道连接ID
   * @param fromLaneId 起始车道ID
   * @param toLaneUid 结束车道ID
   * @return true 当前位置在指定的车道连接区域内
   * @return false 当前位置不在指定的车道连接区域内
   */
  Base::txBool ArriveAtWayPointArea(const Base::txLaneLinkID _linkId, const Base::txLaneUId fromLaneId,
                                    const Base::txLaneUId toLaneUid) const TX_NOEXCEPT {
    if (_valid && _plus_(WayPointType::LaneLink) == _type) {
      if (fromLaneId == _link_locInfo.onLinkFromLaneUid && toLaneUid == _link_locInfo.onLinkToLaneUid) {
        return true;
      }
    }
    return false;
  }

  /**
   * @brief 判断当前点是否属于指定的点区域
   * @return true 当前点在指定的点区域内
   */
  Base::txBool IsPointArea() const TX_NOEXCEPT { return _plus_(WayPointType::Point) == _type; }

  /**
   * @brief 判断当前点是否属于路段区域
   *
   * @return Base::txBool
   */
  Base::txBool IsRoadSectionArea() const TX_NOEXCEPT { return _plus_(WayPointType::Road_Section) == _type; }

  /**
   * @brief 判断当前点是否属于车道连接区域
   *
   * @return Base::txBool
   */
  Base::txBool IsLaneLinkArea() const TX_NOEXCEPT { return _plus_(WayPointType::LaneLink) == _type; }

  /**
   * @brief 设置当前点无效
   *
   */
  void SetInValid() TX_NOEXCEPT { _valid = false; }

  /**
   * @brief 获取当前点是否有效
   *
   * @return Base::txBool
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return _valid; }

  /**
   * @brief 获取当前位置所在的车道ID
   * @return 当前位置所在的车道ID
   */
  Base::txLaneUId GetLocLaneUid() const TX_NOEXCEPT { return _deadline_LaneUid; }

  /**
   * @brief 获取结束点位置
   *
   * @return Coord::txENU
   */
  Coord::txENU GetEndPt() const TX_NOEXCEPT { return _EndPos; }

  /**
   * @brief 获取当前位置所在的车道连接ID
   * @return 当前位置所在的车道连接ID
   */
  Base::txLaneLinkID GetLaneLinkId() const TX_NOEXCEPT { return _link_locInfo.onLinkId_without_equal; }

  /**
   * @brief 获取当前位置所在的车道连接ID
   * @return 当前位置所在的车道连接ID
   */
  Base::txLaneUId GetLaneLinkFromUid() const TX_NOEXCEPT { return _link_locInfo.onLinkFromLaneUid; }

  /**
   * @brief 获取当前位置所在的车道连接的结束车道ID
   * @return 当前位置所在的车道连接的结束车道ID
   */
  Base::txLaneUId GetLaneLinkToUid() const TX_NOEXCEPT { return _link_locInfo.onLinkToLaneUid; }

  /**
   * @brief 获取当前位置所在的车道连接的结束车道ID
   * @return 当前位置所在的车道连接的结束车道ID
   */
  Base::LocInfoSet GetAllLinkSet() const TX_NOEXCEPT { return _all_link_set; }

  /**
   * @brief 获取当前位置所在的车道连接信息
   * @return 当前位置所在的车道连接信息
   */
  Base::txString Str() const TX_NOEXCEPT;
  friend std::ostream& operator<<(std::ostream& os, const WayPoints& v) TX_NOEXCEPT {
    os << "{" << v.Str() << "}";
    return os;
  }

 protected:
  hadmap::txLanePtr _EndPtLanePtr = nullptr;
  Base::txBool _valid = false;
  Base::txLaneUId _deadline_LaneUid;
  Coord::txENU _EndPos;

  Base::Info_Lane_t _link_locInfo;
  Base::LocInfoSet _all_link_set;
  WayPointType _type = WayPointType::Road_Section;
};

TX_NAMESPACE_CLOSE(TrafficFlow)
