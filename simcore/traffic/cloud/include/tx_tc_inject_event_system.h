// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <list>
#include <set>
#include "HdMap/tx_hashed_lane_info.h"
#include "tx_element_manager_base.h"
#include "tx_header.h"
#include "tx_sim_time.h"
#include "tx_tc_cloud_event_json_parser.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class InjectEventAction {
 public:
  InjectEventAction() TX_DEFAULT;
  virtual ~InjectEventAction() TX_DEFAULT;

 public:
  /**
   * @brief 事件行为初始化
   *
   * @param _raw_event_info 原始事件信息
   * @return Base::txBool 初始化成功返回true
   */
  virtual Base::txBool Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT;

  /**
   * @brief 是否需要释放
   *
   * @param timeMgr 时间参数管理器
   * @return Base::txBool 需要释放返回true
   */
  virtual Base::txBool NeedRelease(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT;

  /**
   * @brief 检查事件是否需要完成处理
   *
   * 当事件需要进行事件处理但需要用户等待用户的操作时，使用该函数检查事件是否已经处理完成。
   *
   * @param timeMgr 当前的时间管理器
   * @return Base::txBool 如果事件完成则返回 true，否则返回 false
   */
  virtual Base::txBool NeedDone(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT;

  /**
   * @brief 检查当前事件是否已经完成处理
   *
   * 当需要用户操作来处理事件但还未完成时，使用该函数检查事件是否已经完成处理。
   *
   * @param laneInfo 当前事件的相关信息
   * @return Base::txBool 如果事件已完成处理则返回 true，否则返回 false
   */
  virtual Base::txBool NeedDone(const Base::Info_Lane_t& laneInfo) const TX_NOEXCEPT { return false; }

  /**
   * @brief 获取事件ID
   *
   * 获取当前事件ID，通常在事件处理前调用。
   *
   * @return Base::txSize 返回当前事件ID
   */
  virtual Base::txSize EventId() const TX_NOEXCEPT { return raw_event_info.Event_id(); }

  /**
   * @brief 处理事件完成时的操作
   *
   * @param[in] timeMgr 时间管理器
   * @param[in] elemMgr 元素管理器
   */
  virtual void Done(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT = 0;

  /**
   * @brief PostDone 处理事件完成时的操作
   * @param timeMgr 时间管理器
   * @param elemMgr 元素管理器
   */
  virtual void PostDone(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT;

  /**
   * @brief 释放资源
   *
   * 当此函数被调用时，会释放与此事件相关的资源。
   *
   * @param timeMgr 时间管理器
   * @param elemMgr 元素管理器
   * @return 无
   */
  virtual void Release(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {}

 protected:
  Utils::TrafficInjectEvent raw_event_info;
};
using InjectEventActionPtr = std::shared_ptr<InjectEventAction>;

class WeatherEvent : public InjectEventAction {
 public:
  using ParentClass = InjectEventAction;

 public:
  /**
   * @brief 初始化函数
   *
   * 初始化函数用于初始化类的相关变量和数据结构。
   *
   * @param _raw_event_info 一个包含原始事件信息的数据结构
   * @return Base::txBool 函数执行成功返回true，否则返回false
   */
  virtual Base::txBool Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 事件执行
   *
   * @param timeMgr 时间管理器
   * @param elemMgr 元素管理器
   */
  virtual void Done(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 判断当前Lane是否需要调用Done()方法
   *
   * 如果当前Lane正处于链路上，并且所在的道路ID位于`set_event_influence_roads`集合中，则返回true，表示需要调用Done()方法；否则返回false。
   *
   * @param laneInfo 当前Lane的信息
   * @return Base::txBool 当前Lane是否需要调用Done()方法
   */
  virtual Base::txBool NeedDone(const Base::Info_Lane_t& laneInfo) const TX_NOEXCEPT TX_OVERRIDE {
    return (laneInfo.IsOnLane()) && (set_event_influence_roads.count(laneInfo.onLaneUid.roadId) > 0);
  }

 protected:
  std::set<Base::txRoadID> set_event_influence_roads;
  Utils::InfluenceRuleTemplate weather_influence_template;
};

class LanesSpeedLimitControlEvent : public InjectEventAction {
 public:
  using ParentClass = InjectEventAction;

 public:
  /**
   * @brief 初始化函数，传入原始事件信息。
   *
   * 此函数用于初始化事件对象的属性，包括事件ID、数据包大小等信息。
   *
   * @param _raw_event_info 原始事件信息
   * @return Base::txBool 如果初始化成功则返回true，否则返回false
   */
  virtual Base::txBool Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 处理事件完成时的操作
   *
   * @param[in] timeMgr 时间管理器
   * @param[in] elemMgr 元素管理器
   */
  virtual void Done(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 判断当前Lane是否需要调用Done()方法
   *
   * 如果当前Lane正处于链路上，并且所在的道路ID位于`set_event_influence_roads`集合中，则返回true，表示需要调用Done()方法；否则返回false。
   *
   * @param laneInfo 当前Lane的信息
   * @return Base::txBool 当前Lane是否需要调用Done()方法
   */
  virtual Base::txBool NeedDone(const Base::Info_Lane_t& laneInfo) const TX_NOEXCEPT TX_OVERRIDE {
    return (laneInfo.IsOnLane()) && (set_speed_limit_laneUids.count(laneInfo.onLaneUid) > 0);
  }

  /**
   * @brief 获取事件对应的影响车道速度
   *
   * 此函数用于获取事件对应的影响车道速度信息。
   *
   * @return 返回影响车道速度（单位：毫秒）
   */
  virtual Base::txFloat Event_influence_lanes_speed_ms() const TX_NOEXCEPT {
    return raw_event_info.Event_influence_lanes_speed_ms();
  }

 protected:
  std::unordered_set<Base::txLaneUId, Utils::LaneUIdHashCompare> set_speed_limit_laneUids;
};

class RoadsSpeedLimitControlEvent : public InjectEventAction {
 public:
  using ParentClass = InjectEventAction;

 public:
  /**
   * @brief 初始化函数，传入原始事件信息。
   *
   * 此函数用于初始化事件对象的属性，包括事件ID、数据包大小等信息。
   *
   * @param _raw_event_info 原始事件信息
   * @return Base::txBool 如果初始化成功则返回true，否则返回false
   */
  virtual Base::txBool Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 处理事件完成时的操作
   *
   * @param[in] timeMgr 时间管理器
   * @param[in] elemMgr 元素管理器
   */
  virtual void Done(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 判断当前Lane是否需要调用Done()方法
   *
   * 如果当前Lane正处于链路上，并且所在的道路ID位于`set_event_influence_roads`集合中，则返回true，表示需要调用Done()方法；否则返回false。
   *
   * @param laneInfo 当前Lane的信息
   * @return Base::txBool 当前Lane是否需要调用Done()方法
   */
  virtual Base::txBool NeedDone(const Base::Info_Lane_t& laneInfo) const TX_NOEXCEPT TX_OVERRIDE {
    return (laneInfo.IsOnLane()) && (set_speed_limit_roads.count(laneInfo.onLaneUid.roadId) > 0);
  }

  /**
   * @brief 获取事件对应的影响道路速度
   *
   * 此函数用于获取事件对应的影响道路速度信息。
   *
   * @return 返回影响道路速度（单位：毫秒）
   */
  virtual Base::txFloat Event_influence_roads_speed_ms() const TX_NOEXCEPT {
    return raw_event_info.Event_influence_roads_speed_ms();
  }

 protected:
  std::set<Base::txRoadID> set_speed_limit_roads;
};

class TAD_Virtual_Obstacle_VehicleElement;
using TAD_Virtual_Obstacle_VehicleElementPtr = std::shared_ptr<TAD_Virtual_Obstacle_VehicleElement>;

class RoadClosureControlEvent : public InjectEventAction {
 public:
  using ParentClass = InjectEventAction;

 public:
  /**
   * @brief 初始化函数，传入原始事件信息。
   *
   * 此函数用于初始化事件对象的属性，包括事件ID、数据包大小等信息。
   *
   * @param _raw_event_info 原始事件信息
   * @return Base::txBool 如果初始化成功则返回true，否则返回false
   */
  virtual Base::txBool Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 处理事件完成时的操作
   *
   * @param[in] timeMgr 时间管理器
   * @param[in] elemMgr 元素管理器
   */
  virtual void Done(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放事件
   *
   * 此函数用于在事件结束时释放事件系统。
   *
   * @param timeMgr 时间管理器，用于获取当前时间
   * @param elemMgr 元素管理器，用于管理和操作元素
   * @return 无
   */
  virtual void Release(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief PostDone 函数
   *
   * 该函数用于完成元素事件处理。
   *
   * @param timeMgr 时间管理器，用于获取当前时间
   * @param elemMgr 元素管理器，用于管理和操作元素
   * @return 无
   */
  virtual void PostDone(Base::TimeParamManager const& timeMgr,
                        Base::IElementManagerPtr elemMgr) TX_NOEXCEPT TX_OVERRIDE {}

 protected:
  /**
   * @brief Clear 函数清除set_closure_roads，has_trigger和m_vec_event_obs。
   *
   * 这个函数用于清除这些变量，使得触发后的闭包事件逻辑可重复执行。
   *
   * @param void
   * @return void
   */
  virtual void Clear() TX_NOEXCEPT {
    set_closure_roads.clear();
    has_trigger = false;
    m_vec_event_obs.clear();
  }

 protected:
  std::set<Base::txRoadID> set_closure_roads;
  Base::txBool has_trigger = false;
  std::vector<TAD_Virtual_Obstacle_VehicleElementPtr> m_vec_event_obs;
};

class LaneClosureControlEvent : public InjectEventAction {
 public:
  using ParentClass = InjectEventAction;

 public:
  /**
   * @brief 初始化函数，传入原始事件信息。
   *
   * 此函数用于初始化事件对象的属性，包括事件ID、数据包大小等信息。
   *
   * @param _raw_event_info 原始事件信息
   * @return Base::txBool 如果初始化成功则返回true，否则返回false
   */
  virtual Base::txBool Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 处理事件完成时的操作
   *
   * @param[in] timeMgr 时间管理器
   * @param[in] elemMgr 元素管理器
   */
  virtual void Done(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放事件
   *
   * 此函数用于在事件结束时释放事件系统。
   *
   * @param timeMgr 时间管理器，用于获取当前时间
   * @param elemMgr 元素管理器，用于管理和操作元素
   * @return 无
   */
  virtual void Release(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief PostDone 函数
   *
   * 该函数用于完成元素事件处理。
   *
   * @param timeMgr 时间管理器，用于获取当前时间
   * @param elemMgr 元素管理器，用于管理和操作元素
   * @return 无
   */
  virtual void PostDone(Base::TimeParamManager const& timeMgr,
                        Base::IElementManagerPtr elemMgr) TX_NOEXCEPT TX_OVERRIDE {}

 protected:
  /**
   * @brief 清除闭包、触发器和事件观察者。
   *
   * 这个函数清除 set_closure_lanes、has_trigger 和 m_vec_event_obs 变量。
   *
   * @param 无
   * @return 无
   */
  virtual void Clear() TX_NOEXCEPT {
    set_closure_lanes.clear();
    has_trigger = false;
    m_vec_event_obs.clear();
  }

 protected:
  std::unordered_set<Base::txLaneUId, Utils::LaneUIdHashCompare> set_closure_lanes;
  Base::txBool has_trigger = false;
  std::vector<TAD_Virtual_Obstacle_VehicleElementPtr> m_vec_event_obs;
};

class TrafficIncidentEvent : public InjectEventAction {
 public:
  using ParentClass = InjectEventAction;
  using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;

 public:
  /**
   * @brief 初始化函数，传入原始事件信息。
   *
   * 此函数用于初始化事件对象的属性，包括事件ID、数据包大小等信息。
   *
   * @param _raw_event_info 原始事件信息
   * @return Base::txBool 如果初始化成功则返回true，否则返回false
   */
  virtual Base::txBool Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 处理事件完成时的操作
   *
   * @param[in] timeMgr 时间管理器
   * @param[in] elemMgr 元素管理器
   */
  virtual void Done(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放事件
   *
   * 此函数用于在事件结束时释放事件系统。
   *
   * @param timeMgr 时间管理器，用于获取当前时间
   * @param elemMgr 元素管理器，用于管理和操作元素
   * @return 无
   */
  virtual void Release(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT TX_OVERRIDE;
  // virtual void PostDone(Base::TimeParamManager const&  timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT
  // TX_OVERRIDE {}

 protected:
  /**
   * @brief 清除闭包、触发器和事件观察者。
   *
   * 这个函数清除 set_closure_lanes、has_trigger 和 m_vec_event_obs 变量。
   *
   * @param 无
   * @return 无
   */
  virtual void Clear() TX_NOEXCEPT {
    m_list_influence_hashednode.clear();
    has_trigger = false;
    m_vec_event_obs.clear();
  }

 protected:
  std::list<HashedLaneInfo> m_list_influence_hashednode;
  Base::txBool has_trigger = false;
  std::vector<TAD_Virtual_Obstacle_VehicleElementPtr> m_vec_event_obs;
  Coord::txWGS84 m_event_point;
};

class InjectEventHandler {
 public:
  InjectEventHandler() TX_DEFAULT;
  virtual ~InjectEventHandler() TX_DEFAULT;

 public:
  /**
   * @brief 向系统注入事件
   *
   * 该函数向当前的事件系统注入事件，需要输入JSON格式的事件数据。
   *
   * @param _event_json JSON格式的事件数据
   * @return Base::txBool 操作成功返回true，失败返回false
   */
  virtual Base::txBool InjectTrafficEvent(const std::string& _event_json) TX_NOEXCEPT;

  /**
   * @brief 注入流量事件的处理函数
   *
   * 根据给定的时间参数和元素管理器，该函数处理相应的流量事件。
   *
   * @param timeMgr 时间参数管理器，包含了一系列时间参数
   * @param elemMgr 元素管理器，包含了当前系统的所有元素信息
   * @return 无
   */
  virtual void InjectTrafficEventHandler(Base::TimeParamManager const& timeMgr,
                                         Base::IElementManagerPtr elemMgr) TX_NOEXCEPT;

  /**
   * @brief 在事件系统后执行的流量注入处理函数
   *
   * 这个函数会在事件系统运行后执行，以处理已注入流量的相关操作。
   *
   * @param timeMgr 时间参数管理器，包含了一系列时间参数
   * @param elemMgr 元素管理器，包含了当前系统的所有元素信息
   */
  virtual void InjectTrafficEventHandlerPost(Base::TimeParamManager const& timeMgr,
                                             Base::IElementManagerPtr elemMgr) TX_NOEXCEPT;

  /**
   * @brief 返回事件行为的数量大小
   *
   * @return Base::txSize 返回事件行为的大小
   */
  virtual Base::txSize ActionSize() const TX_NOEXCEPT { return m_vec_event_actions.size(); }

 protected:
  std::vector<InjectEventActionPtr> m_vec_event_actions;
};

TX_NAMESPACE_CLOSE(TrafficFlow)
