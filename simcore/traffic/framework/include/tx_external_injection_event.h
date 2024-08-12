// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <cfloat>
#include <set>
#include <unordered_set>
#include "HdMap/tx_hashed_lane_info.h"
#include "tx_hash_utils.h"
#include "tx_header.h"
#include "tx_sim_point.h"
#if USE_DITW_Event
TX_NAMESPACE_OPEN(Base)

class ITrafficElement;
using ITrafficElementPtr = std::shared_ptr<ITrafficElement>;

class txInjectionEvent {
 public:
  enum class EventType : txInt {
    eSpeedLimitRoad,
    eSpeedLimitLane,
    eCollision,
    eCrashInfluneLane,
    eCloseLane,
    eVirtualCityCrash,
    eTadReplay,
    eUndef
  };

  class txAffectedRoads {
   public:
    std::set<Base::txRoadID> roadIdSet;
    Base::txFloat affected_speed_ratio = 0.0;
    Base::txFloat affected_speed_ratio_upper = 0.0;
  };

  class txAffectedLanes {
   public:
    std::unordered_set<Base::txLaneUId, Utils::LaneUIdHashCompare> laneUidSet;
    Base::txFloat affected_speed = 0;
  };

 public:
  txInjectionEvent() TX_DEFAULT;

  /**
   * @brief 构造一个新的txInjectionEvent对象
   *
   * @param _eId 事件ID
   * @param _roadIdSet 受影响的道路集合
   * @param _speedRatio 初始速度比例
   * @param _speedRatioUpper 最大速度比例
   * @param startTime 事件开始时间
   * @param endTime 事件结束时间
   */
  txInjectionEvent(const txSysId _eId, const std::set<Base::txRoadID>& _roadIdSet, const txFloat _speedRatio,
                   const txFloat _speedRatioUpper, const txFloat startTime, const txFloat endTime)
      : mValid(true), mStartingTime(startTime), mEndTime(endTime), mEventId(_eId) {
    mAffectedRoads.affected_speed_ratio = _speedRatio;
    mAffectedRoads.affected_speed_ratio_upper = _speedRatioUpper;
    mAffectedRoads.roadIdSet = _roadIdSet;
    mEventType = EventType::eSpeedLimitRoad;
  }

  ~txInjectionEvent() TX_DEFAULT;

  /**
   * @brief 事件是否激活
   *
   * @return txBool
   */
  txBool IsValid() const TX_NOEXCEPT { return mValid; }

  /**
   * @brief 获取事件id
   *
   * @return txSysId 返回事件id
   */
  txSysId EventId() const TX_NOEXCEPT { return mEventId; }

  /**
   * @brief 获取事件类型
   *
   * @return EventType 事件类型
   */
  EventType GetEventType() const TX_NOEXCEPT { return mEventType; }

  /**
   * @brief 获取事件的开始时间
   *
   * @return txFloat 事件的开始时间
   */
  txFloat StartTime() const TX_NOEXCEPT { return mStartingTime; }

  /**
   * @brief 获取事件的结束时间
   *
   * @return txFloat 结束时间
   */
  txFloat EndTime() const TX_NOEXCEPT { return mEndTime; }

  /**
   * @brief 表示该函数检查当前时间戳是否可用，无异常
   *
   * @param curTimeStamp 类型，表示当前时间戳
   * @return txBool 返回：表示函数是否可用的布尔值
   */
  virtual txBool CheckTime(const txFloat curTimeStamp) TX_NOEXCEPT;

  /**
   * @brief UpdateSimulationEvent 更新模拟事件
   * 根据给定的当前时间戳和车辆指针向量，更新模拟事件
   * @param curTimeStamp 当前时间戳，单位秒
   * @param vecVehiclePtr 一个包含 Base::ITrafficElementPtr 类型的指针的向量，表示车辆指针向量
   * @return txBool 更新模拟事件是否成功
   */
  virtual txBool UpdateSimulationEvent(const txFloat curTimeStamp,
                                       std::vector<Base::ITrafficElementPtr> vecVehiclePtr) TX_NOEXCEPT;

 protected:
  txAffectedRoads mAffectedRoads;
  txAffectedLanes mAffectedLanes;
  Base::txSysId mEventId = -1;
  EventType mEventType = EventType::eUndef;
  Base::txFloat mStartingTime = 0.0;
  Base::txFloat mEndTime = 0.0;
  Base::txBool mValid = false;
};
using txInjectionEventPtr = std::shared_ptr<txInjectionEvent>;
#  if USE_HashedRoadNetwork
class txInjectionEventCrashInflunce : public txInjectionEvent {
 public:
  struct InjectionEventInfo {
    Base::txSysId event_id;
    Base::txString event_type;
    Base::txFloat event_start_time_in_second = 0.0;
    Base::txFloat event_end_time_in_second = FLT_MAX;
    Base::txFloat event_location_lon;
    Base::txFloat event_location_lat;
    std::vector<Base::txLaneID> event_influnce_lane;
    std::vector<Base::txFloat> event_influnce_range_alone_lane;
    Base::txString record_file_path;
    Base::txFloat l2w_switch_time = 0.0;
  };
  friend std::ostream& operator<<(std::ostream& os, const InjectionEventInfo& v) TX_NOEXCEPT;
  static Base::txBool LoadEventCfg(const Base::txString& _cfg_full_path, InjectionEventInfo& ret_cfg) TX_NOEXCEPT;

 public:
  txInjectionEventCrashInflunce() TX_DEFAULT;

  /**
   * @brief 初始化外部嵌入事件
   *
   * 设置外部嵌入事件的相关信息，例如事件ID、崩溃地点、影响道路ID、前置距离和后置距离等。
   *
   * @param _eId 事件ID
   * @param _crash_location 崩溃地点（WGS84坐标系）
   * @param influnceLaneIdVec 影响道路ID向量
   * @param pre_dist 前置距离
   * @param post_dist 后置距离
   * @param startTimeStamp 事件开始时间戳
   * @param endTimeStamp 事件结束时间戳
   * @return txBool 初始化是否成功
   */
  Base::txBool Initialize(const txSysId _eId, const Coord::txWGS84 _crash_location,
                          const std::vector<Base::txLaneID>& influnceLaneIdVec, const Base::txFloat pre_dist,
                          const Base::txFloat post_dist, const txFloat startTimeStamp,
                          const txFloat endTimeStamp) TX_NOEXCEPT;

  /**
   * @brief CheckTime 检查时间是否合法
   * @param curTimeStamp 当前时间戳
   * @return 返回检查结果，若合法返回 true, 否则返回 false
   */
  virtual txBool CheckTime(const txFloat curTimeStamp) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief UpdateSimulationEvent 更新模拟事件
   * 根据给定的当前时间戳和车辆指针向量，更新模拟事件
   * @param curTimeStamp 当前时间戳，单位秒
   * @param vecVehiclePtr 一个包含 Base::ITrafficElementPtr 类型的指针的向量，表示车辆指针向量
   * @return txBool 更新模拟事件是否成功
   */
  virtual txBool UpdateSimulationEvent(const txFloat curTimeStamp,
                                       std::vector<Base::ITrafficElementPtr> vecVehiclePtr) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  Coord::txWGS84 mEventLocation;
  Base::Info_Lane_t mEventLaneInfo;
  std::vector<Geometry::SpatialQuery::HashedLaneInfo> mInflunceHashedLaneInfo;
};
using txInjectionEventCrashInfluncePtr = std::shared_ptr<txInjectionEventCrashInflunce>;

#  endif /*USE_HashedRoadNetwork*/
TX_NAMESPACE_CLOSE(Base)
#endif /*USE_DITW_Event*/
