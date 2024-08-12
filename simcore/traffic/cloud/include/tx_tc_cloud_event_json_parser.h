// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_time_utils.h"
TX_NAMESPACE_OPEN(Utils)
class EventInflunceLaneRule {
 public:
  EventInflunceLaneRule() = default;
  virtual ~EventInflunceLaneRule() = default;

 public:
  /**
   * @brief 获取对象的字符串输出格式
   *
   * @return std::string
   */
  std::string Str() const TX_NOEXCEPT;
  friend std::ostream& operator<<(std::ostream& oss, const EventInflunceLaneRule& v) TX_NOEXCEPT {
    oss << v.Str();
    return oss;
  }

 public:
  int laneId;
  bool turn_left;
  bool turn_right;
  int upstream_block;
};

class VirtualCityEvent {
 public:
  VirtualCityEvent() = default;
  virtual ~VirtualCityEvent() = default;

 public:
  std::string event_type;
  double event_location_lat;
  double event_location_lon;
  double event_location_longitudinal_offset = 0.0;
  std::vector<int64_t> event_influnce_lane;
  int64_t event_id;
  std::vector<int64_t> event_influnce_roads;
  double event_influnce_roads_speed;

  double event_speed_limit_start_lat;
  double event_speed_limit_start_lon;
  double event_speed_limit_end_lat;
  double event_speed_limit_end_lon;
  std::vector<EventInflunceLaneRule> event_influnce_lane_rule;
  int64_t event_duration = INT_MAX;

 public:
  /**
   * @brief 获取事件类型
   * @return 返回事件类型字符串
   *
   * 获取当前事件的事件类型字符串，例如："ack", "heartbeat"等。
   */
  const std::string& get_event_type() const { return event_type; }

  /**
   * @brief 获取可修改的事件类型
   * @return 返回可修改的事件类型字符串引用
   *
   * 获取当前事件的可修改事件类型字符串引用，可用于修改事件类型。
   */
  std::string& get_mutable_event_type() { return event_type; }

  /**
   * @brief 设置事件类型
   * @param value 要设置的事件类型字符串
   *
   * 设置当前事件的事件类型字符串，用于存储事件类型信息。
   */
  void set_event_type(const std::string& value) { this->event_type = value; }

  /**
   * @brief 获取事件的纬度
   * @return const double& 返回纬度
   *
   * 获取事件的纬度信息，返回纬度数值。
   */
  const double& get_event_location_lat() const { return event_location_lat; }

  /**
   * @brief 获取可修改的事件纬度
   * @return double& 返回可修改的事件纬度
   *
   * 获取可修改的事件纬度信息。
   */
  double& get_mutable_event_location_lat() { return event_location_lat; }

  /**
   * @brief 设置事件纬度
   * @param value 要设置的纬度值
   *
   * 设置事件纬度，使其变为指定值。
   */
  void set_event_location_lat(const double& value) { this->event_location_lat = value; }

  /**
   * @brief 获取事件经度
   * @return const double & 返回事件的经度值的const引用
   *
   * 获取事件的经度，返回一个const double类型的引用，使得可以直接读取或修改事件的经度。
   */
  const double& get_event_location_lon() const { return event_location_lon; }

  /**
   * @brief 获取事件经度
   * @return double & 返回事件的经度值的引用
   *
   * 获取事件的经度，返回一个double类型的引用，使得可以直接读取或修改事件的经度。
   */
  double& get_mutable_event_location_lon() { return event_location_lon; }

  /**
   * @brief 设置事件的经度
   * @param value 经度值，double类型
   * @return 无
   *
   * 设置事件的经度值，传入double类型的参数值。
   */
  void set_event_location_lon(const double& value) { this->event_location_lon = value; }

  /**
   * @brief 获取事件的经度偏移
   * @return 双精度浮点数，表示事件的经度偏移
   *
   * 获取事件的经度偏移值，返回一个双精度浮点数。
   */
  const double& get_event_location_longitudinal_offset() const { return event_location_longitudinal_offset; }

  /**
   * @brief 获取事件的沿着线的经度偏移
   * @return 双精度浮点数，表示事件的沿着线的经度偏移
   *
   * 获取事件的沿着线的经度偏移值，返回一个双精度浮点数。
   */
  double& get_mutable_event_location_longitudinal_offset() { return event_location_longitudinal_offset; }

  /**
   * @brief 设置事件的沿着线的经度偏移
   * @param value 双精度浮点数，表示事件的沿着线的经度偏移
   * @return 无返回值
   *
   * 设置事件的沿着线的经度偏移值。
   */
  void set_event_location_longitudinal_offset(const double& value) { this->event_location_longitudinal_offset = value; }

  /**
   * @brief 获取事件影响的车道
   * @return 返回一个包含所有影响该事件的车道编号的整数向量
   *
   * 获取该事件影响的车道的编号，返回一个包含所有影响该事件的车道编号的整数向量。
   */
  const std::vector<int64_t>& get_event_influnce_lane() const { return event_influnce_lane; }

  /**
   * @brief 获取可修改的事件影响车道数组
   * @return 返回一个包含所有可修改的事件影响车道编号的整数向量的引用
   *
   * 获取该事件影响的车道的编号，返回一个包含所有可修改的事件影响车道编号的整数向量的引用。通过这个引用，用户可以修改事件的影响车道数组。
   */
  std::vector<int64_t>& get_mutable_event_influnce_lane() { return event_influnce_lane; }

  /**
   * @brief 设置事件影响车道数组
   * @param value 包含所有可修改的事件影响车道编号的整数向量
   * @return 无返回值
   *
   * 用户可以通过这个函数设置事件影响车道的编号，传入的参数是一个包含所有可修改的事件影响车道编号的整数向量。通过这个函数，用户可以修改事件的影响车道数组。
   */
  void set_event_influnce_lane(const std::vector<int64_t>& value) { this->event_influnce_lane = value; }

  /**
   * @brief 获取事件ID
   * @return 返回事件ID的int64_t引用
   *
   * 该函数用于获取事件的ID，返回值为一个int64_t类型的引用。
   */
  const int64_t& get_event_id() const { return event_id; }

  /**
   * @brief 获取可修改的事件ID
   * @return 返回可修改的事件ID的int64_t引用
   *
   * 该函数用于获取事件的ID，返回值为一个int64_t类型的引用，允许用户修改事件ID。
   */
  int64_t& get_mutable_event_id() { return event_id; }

  /**
   * @brief 设置事件ID
   * @param value 事件ID的值，类型为int64_t
   * @return 无返回值
   *
   * 该函数用于设置事件的ID，通过传入的value参数，将事件ID设置为value的值。
   */
  void set_event_id(const int64_t& value) { this->event_id = value; }

  /**
   * @brief 获取事件影响的道路ID列表
   * @return 返回事件影响的道路ID列表，类型为std::vector<int64_t>
   *
   * 该函数用于获取事件影响的道路ID列表，返回一个包含所有道路ID的std::vector<int64_t>类型的列表。
   */
  const std::vector<int64_t>& get_event_influnce_roads() const { return event_influnce_roads; }

  /**
   * @brief 获取可变的事件影响的道路ID列表
   * @return 返回可变的事件影响的道路ID列表，类型为std::vector<int64_t>&
   *
   * 该函数用于获取可变的事件影响的道路ID列表，返回一个包含所有道路ID的std::vector<int64_t>&类型的列表。
   */
  std::vector<int64_t>& get_mutable_event_influnce_roads() { return event_influnce_roads; }

  /**
   * @brief 设置事件影响的道路ID列表
   * @param value 包含所有道路ID的 std::vector<int64_t> 类型的值
   * @return 无返回值
   *
   * 该函数用于设置事件影响的道路ID列表，传入参数为包含所有道路ID的 std::vector<int64_t> 类型的值。
   */
  void set_event_influnce_roads(const std::vector<int64_t>& value) { this->event_influnce_roads = value; }

  /**
   * @brief 获取事件影响的道路速度
   * @return 返回事件影响的道路速度，类型为 double 类型的引用
   *
   * 该函数用于获取事件影响的道路速度，返回包含道路速度的 double 类型引用。
   */
  const double& get_event_influnce_roads_speed() const { return event_influnce_roads_speed; }

  /**
   * @brief 获取可变的事件影响的道路速度
   * @return 返回可变的事件影响的道路速度，类型为 double 类型的引用
   *
   * 该函数用于获取可变的事件影响的道路速度，返回包含道路速度的 double 类型引用。
   */
  double& get_mutable_event_influnce_roads_speed() { return event_influnce_roads_speed; }

  /**
   * @brief 设置可变的事件影响的道路速度
   * @param value 包含道路速度的 double 类型的值
   * @return 无返回值
   *
   * 该函数用于设置可变的事件影响的道路速度，接收一个包含道路速度的 double 类型的值。
   */
  void set_event_influnce_roads_speed(const double& value) { this->event_influnce_roads_speed = value; }

  /**
   * @brief 获取事件开始位置的纬度
   * @return 返回事件开始位置的纬度，类型为 double 引用
   *
   * 此函数用于获取事件开始位置的纬度，并以 double 类型的引用形式返回。
   */
  const double& get_event_speed_limit_start_lat() const { return event_speed_limit_start_lat; }

  /**
   * @brief 获取事件开始位置的纬度
   * @return 返回事件开始位置的纬度，类型为 double 引用
   *
   * 此函数用于获取事件开始位置的纬度，并以 double 类型的引用形式返回。
   */
  double& get_event_speed_limit_start_lat() { return event_speed_limit_start_lat; }

  /**
   * @brief 设置事件开始位置的纬度
   * @param value 事件开始位置的纬度
   *
   * 此函数用于设置事件开始位置的纬度。
   */
  void set_event_speed_limit_start_lat(const double& value) { this->event_speed_limit_start_lat = value; }

  /**
   * @brief 获取事件开始位置的经度
   * @return 事件开始位置的经度，类型为 const double &
   *
   * 此函数用于获取事件开始位置的经度。
   */
  const double& get_event_speed_limit_start_lon() const { return event_speed_limit_start_lon; }

  /**
   * @brief 获取事件开始位置的经度
   * @return 事件开始位置的经度，类型为 double &
   *
   * 此函数用于获取事件开始位置的经度。
   */
  double& get_event_speed_limit_start_lon() { return event_speed_limit_start_lon; }

  /**
   * @brief 设置事件开始位置的经度
   * @param value 事件开始位置的经度
   *
   * 此函数用于设置事件开始位置的经度。
   */
  void set_event_speed_limit_start_lon(const double& value) { this->event_speed_limit_start_lon = value; }

  /**
   * @brief 获取事件结束位置的纬度
   * @return 事件结束位置的纬度，类型为 const double &
   *
   * 此函数用于获取事件结束位置的纬度。
   */
  const double& get_event_speed_limit_end_lat() const { return event_speed_limit_end_lat; }

  /**
   * @brief 获取事件结束位置的纬度
   * @return 事件结束位置的纬度，类型为 double &
   *
   * 此函数用于获取事件结束位置的纬度。
   */
  double& get_event_speed_limit_end_lat() { return event_speed_limit_end_lat; }

  /**
   * @brief 设置事件结束位置的纬度
   * @param value 事件结束位置的纬度
   *
   * 此函数用于设置事件结束位置的纬度。
   */
  void set_event_speed_limit_end_lat(const double& value) { this->event_speed_limit_end_lat = value; }

  /**
   * @brief 获取事件结束位置的经度
   * @return 事件结束位置的经度，类型为 const double &
   *
   * 此函数用于获取事件结束位置的经度。
   */
  const double& get_event_speed_limit_end_lon() const { return event_speed_limit_end_lon; }

  /**
   * @brief 获取事件结束位置的经度
   * @return 事件结束位置的经度，类型为 double &
   *
   * 此函数用于获取事件结束位置的经度。
   */
  double& get_event_speed_limit_end_lon() { return event_speed_limit_end_lon; }

  /**
   * @brief 设置事件结束位置的经度
   * @param value 事件结束位置的经度，类型为 const double &
   *
   * 此函数用于设置事件结束位置的经度。
   */
  void set_event_speed_limit_end_lon(const double& value) { this->event_speed_limit_end_lon = value; }

  /**
   * @brief 获取事件持续时间
   * @return int64_t 事件持续时间，单位为微秒
   */
  const int64_t get_event_duration() const { return event_duration; }

  /**
   * @brief 获取事件持续时间
   * @return int64_t 事件持续时间，单位为微秒
   */
  int64_t& get_event_duration() { return event_duration; }

  /**
   * @brief 设置事件持续时间
   * @param [in] _d 事件持续时间，单位为微秒
   */
  void set_event_duration(const int64_t _d) { this->event_duration = _d; }

  /**
   * @brief 将当前数据对象转换为字符串
   * @return 返回包含当前数据对象的字符串
   */
  std::string Str() const TX_NOEXCEPT;
};

extern VirtualCityEvent parser_virtual_city_event(const Base::txString json_content) TX_NOEXCEPT;

class InfluenceRuleTemplate {
 public:
  InfluenceRuleTemplate() = default;
  ~InfluenceRuleTemplate() = default;

 public:
  using txString = Base::txString;
  using txFloat = Base::txFloat;
  enum class InflunceRule : Base::txInt { light, moderate, heavy };

 public:
  txString name;
  txFloat threshold_ms;
  txFloat speed_limit_value_ms;
  txFloat speed_limit_factor;

 public:
  /**
   * @brief 将当前对象的 Name() 属性转换为对应的 InflunceRule
   * @return InflunceRule 对象类型的值
   */
  InflunceRule eName() const TX_NOEXCEPT { return s2e(Name()); }

  /**
   * @brief 获取当前对象的名称
   * @return 名称
   */
  txString Name() const TX_NOEXCEPT { return name; }

  /**
   * @brief 获取阈值(ms)
   * @return 返回浮点数类型的阈值
   */
  txFloat Threshold_ms() const TX_NOEXCEPT { return threshold_ms; }

  /**
   * @brief 获取速度限制值(ms)
   * @return 返回浮点数类型的速度限制值
   */
  txFloat Speed_limit_value_ms() const TX_NOEXCEPT { return speed_limit_value_ms; }

  /**
   * @brief 获取速度限制系数
   * @return 返回浮点数类型的速度限制系数
   */
  txFloat Speed_limit_factor() const TX_NOEXCEPT { return speed_limit_factor; }

  /**
   * @brief 将输入的字符串转换为InflunceRule结构
   *
   * @param str 输入的字符串，包含InflunceRule的信息
   * @return InflunceRule 转换后的InflunceRule结构
   *
   * @details 此函数用于将输入的字符串转换为InflunceRule结构。输入字符串应遵循特定的格式，该格式在函数文档中进行了说明。
   */
  static InflunceRule s2e(const txString str) TX_NOEXCEPT {
    if ("light" == str) {
      return InflunceRule::light;
    }
    if ("moderate" == str) {
      return InflunceRule::moderate;
    }
    if ("heavy" == str) {
      return InflunceRule::heavy;
    }
    return InflunceRule::light;
  }

  /**
   * @brief 返回当前对象的字符串格式输出
   *
   * @return std::string
   */
  std::string Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(Name, Name()) << TX_VARS_NAME(Threshold_ms, Threshold_ms())
        << TX_VARS_NAME(Speed_limit_value_ms, Speed_limit_value_ms())
        << TX_VARS_NAME(Speed_limit_factor, Speed_limit_factor()) << "}";
    // LOG(WARNING) << oss.str();
    return oss.str();
  }
  friend std::ostream& operator<<(std::ostream& os, const InfluenceRuleTemplate& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }
};

class InfluenceRange {
 public:
  InfluenceRange() = default;
  ~InfluenceRange() = default;

 public:
  Base::txFloat upstream_m = 0.0;
  Base::txFloat downstream_m = 0.0;

 public:
  /**
   * @brief 获取上游带宽
   *
   * @return float 上游带宽
   */
  Base::txFloat Upstream_m() const TX_NOEXCEPT { return upstream_m; }

  /**
   * @brief 获取下游带宽
   *
   * @return float 下游带宽
   */
  Base::txFloat Downstream_m() const TX_NOEXCEPT { return downstream_m; }

  /**
   * @brief 获取字符串形式的事件信息
   * @return std::string 事件信息的字符串形式
   */
  std::string Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(upstream_m, Upstream_m()) << TX_VARS_NAME(downstream_m, Downstream_m()) << "}";
    // LOG(WARNING) << oss.str();
    return oss.str();
  }
  friend std::ostream& operator<<(std::ostream& os, const InfluenceRange& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }
};

class EventLaneUid {
 public:
  EventLaneUid() = default;
  ~EventLaneUid() = default;

 public:
  Base::txRoadID rid;
  Base::txSectionID sid;
  Base::txLaneID lid;

 public:
  /**
   * @brief 获取road id
   *
   * @return Base::txRoadID
   */
  Base::txRoadID Rid() const TX_NOEXCEPT { return rid; }

  /**
   * @brief 获取section id
   *
   * @return Base::txSectionID
   */
  Base::txSectionID Sid() const TX_NOEXCEPT { return sid; }

  /**
   * @brief 获取lane id
   *
   * @return Base::txLaneID
   */
  Base::txLaneID Lid() const TX_NOEXCEPT { return lid; }

  /**
   * @brief 获取当前事件的Lane UID
   * @return Base::txLaneUId 当前事件的Lane UID
   */
  Base::txLaneUId LaneUid() const TX_NOEXCEPT { return Base::txLaneUId(Rid(), Sid(), Lid()); }

  /**
   * @brief 对象的字符串输出
   *
   * @return std::string
   */
  std::string Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(rid, Rid()) << TX_VARS_NAME(sid, Sid()) << TX_VARS_NAME(lid, Lid()) << "}";
    // LOG(WARNING) << oss.str();
    return oss.str();
  }
  friend std::ostream& operator<<(std::ostream& os, const EventLaneUid& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }
};

class InfluenceAction {
 public:
  InfluenceAction() = default;
  ~InfluenceAction() = default;

 public:
  Base::txString turn_left;
  Base::txString turn_right;
  enum class ActionType : Base::txInt { eAuto, eLeft, eRight };

 public:
  /**
   * @brief 根据字符串获取对应的ActionType
   *
   * @param str 字符串
   * @return ActionType 返回的类型
   */
  static ActionType s2e(const Base::txString str) TX_NOEXCEPT {
    if ("auto" == str) {
      return ActionType::eAuto;
    }
    if ("left" == str) {
      return ActionType::eLeft;
    }
    if ("right" == str) {
      return ActionType::eRight;
    }
    return ActionType::eAuto;
  }

  /**
   * @brief 左转
   * @return ActionType 返回左转动作类型
   */
  ActionType Turn_left() const TX_NOEXCEPT { return s2e(turn_left); }

  /**
   * @brief 获取右转动作类型
   * @return ActionType 返回右转动作类型
   */
  ActionType Turn_right() const TX_NOEXCEPT { return s2e(turn_right); }

  /**
   * @brief 获取对象的字符串输出
   *
   * @return std::string
   */
  std::string Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(turn_left, turn_left) << TX_VARS_NAME(turn_right, turn_right) << "}";
    // LOG(WARNING) << oss.str();
    return oss.str();
  }
  friend std::ostream& operator<<(std::ostream& os, const InfluenceAction& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }
};

class TrafficInjectEvent {
 public:
  TrafficInjectEvent() = default;
  ~TrafficInjectEvent() = default;

 public:
  using txString = Base::txString;
  using txBool = Base::txBool;
  using txSize = Base::txSize;
  using txFloat = Base::txFloat;
  enum class InjectEventType : Base::txInt {
    eUnknow,
    sim_fog,
    sim_snowfall,
    sim_rainfall,
    sim_accident,
    sim_construction,
    sim_road_speed_limit,
    sim_lane_speed_limit,
    sim_road_closure,
    sim_lane_closure
  };

 public:
  txString event_type;
  txSize event_id;
  std::vector<Base::txRoadID> event_influence_roads;
  txSize event_start_time_stamp_s;
  txSize event_duration_s;
  txString event_influence_rule;
  std::vector<InfluenceRuleTemplate> event_influence_rule_template;

  txFloat event_location_lat;
  txFloat event_location_lon;
  txFloat event_influence_roads_speed_ms;
  InfluenceRange event_influence_range;
  std::vector<EventLaneUid> event_influence_lanes;
  InfluenceAction event_influence_action;
  txFloat event_influence_lanes_speed_ms;

 public:
  /**
   * @brief 检查当前事件是否为天气相关事件
   * @return txBool 如果当前事件是天气相关事件，返回txTrue，否则返回txFalse
   */
  txBool IsWeather() const TX_NOEXCEPT {
    const InjectEventType e = eEventType();
    return (InjectEventType::sim_fog == e) || (InjectEventType::sim_snowfall == e) ||
           (InjectEventType::sim_rainfall == e);
  }

  /**
   * @brief 判断当前事件是否为特定事件
   * @return txBool 如果当前事件是特定事件，则返回txTrue，否则返回txFalse
   */
  txBool IsEvent() const TX_NOEXCEPT {
    const InjectEventType e = eEventType();
    return (InjectEventType::sim_accident == e) || (InjectEventType::sim_construction == e);
  }

  /**
   * @brief 检查当前事件是否为控制事件
   * @return txBool 如果当前事件是控制事件，则返回txTrue，否则返回txFalse
   */
  txBool IsControl() const TX_NOEXCEPT {
    const InjectEventType e = eEventType();
    return (InjectEventType::sim_road_speed_limit == e) || (InjectEventType::sim_lane_speed_limit == e) ||
           (InjectEventType::sim_road_closure == e) || (InjectEventType::sim_lane_closure == e);
  }

  /**
   * @brief 获取事件类型
   * @return InjectEventType 事件类型
   */
  InjectEventType eEventType() const TX_NOEXCEPT { return s2e(event_type); }

  /**
   * @brief 根据字符串类型转换成对应的事件类型
   *
   * @param str 字符串类型
   * @return 对应的事件类型
   */
  static InjectEventType s2e(const txString str) TX_NOEXCEPT {
    if ("sim_fog" == str) {
      return InjectEventType::sim_fog;
    }
    if ("sim_snowfall" == str) {
      return InjectEventType::sim_snowfall;
    }
    if ("sim_rainfall" == str) {
      return InjectEventType::sim_rainfall;
    }
    if ("sim_accident" == str) {
      return InjectEventType::sim_accident;
    }
    if ("sim_construction" == str) {
      return InjectEventType::sim_construction;
    }
    if ("sim_road_speed_limit" == str) {
      return InjectEventType::sim_road_speed_limit;
    }
    if ("sim_lane_speed_limit" == str) {
      return InjectEventType::sim_lane_speed_limit;
    }
    if ("sim_road_closure" == str) {
      return InjectEventType::sim_road_closure;
    }
    if ("sim_lane_closure" == str) {
      return InjectEventType::sim_lane_closure;
    }
    return InjectEventType::eUnknow;
  }

  /**
   * @brief 获取事件类型
   * @return 事件类型字符串
   */
  txString Event_type() const TX_NOEXCEPT { return event_type; }

  /**
   * @brief 获取事件ID
   * @return 事件ID
   */
  txSize Event_id() const TX_NOEXCEPT { return event_id; }

  /**
   * @brief 获取影响道路列表
   * @return 影响道路列表
   */
  std::vector<Base::txRoadID> Event_influence_roads() const TX_NOEXCEPT { return event_influence_roads; }

  /**
   * @brief 获取事件开始时间戳
   * @return 事件开始时间戳（单位：秒）
   */
  txSize Event_start_time_stamp_s() const TX_NOEXCEPT { return event_start_time_stamp_s; }

  /**
   * @brief 获取事件持续时间（单位：秒）
   * @return 事件持续时间（单位：秒）
   */
  txSize Event_duration_s() const TX_NOEXCEPT { return event_duration_s; }

  /**
   * @brief 获取事件结束时间戳（单位：秒）
   * @return 事件结束时间戳（单位：秒）
   */
  txSize Event_end_time_stamp_s() const TX_NOEXCEPT { return (Event_start_time_stamp_s()) + (Event_duration_s()); }

  /**
   * @brief 获取事件开始时间戳（单位：毫秒）
   * @return 事件开始时间戳（单位：毫秒）
   */
  txFloat Event_start_time_stamp_ms() const TX_NOEXCEPT {
    return Utils::SecondToMillisecond(Event_start_time_stamp_s());
  }

  /**
   * @brief 获取事件结束时间戳（单位：毫秒）
   * @return 事件结束时间戳（单位：毫秒）
   */
  txFloat Event_end_time_stamp_ms() const TX_NOEXCEPT { return Utils::SecondToMillisecond(Event_end_time_stamp_s()); }

  /**
   * @brief 获取事件影响规则
   * @return 事件影响规则
   */
  txString Event_influence_rule() const TX_NOEXCEPT { return event_influence_rule; }

  /**
   * @brief 获取事件影响规则模板集合
   * @return 事件影响规则模板集合
   */
  std::vector<InfluenceRuleTemplate> Event_influence_rule_template() const TX_NOEXCEPT {
    return event_influence_rule_template;
  }

  /**
   * @brief 获取事件纬度
   * @return 事件纬度
   */
  txFloat Event_location_lat() const TX_NOEXCEPT { return event_location_lat; }

  /**
   * @brief 获取事件经度
   * @return 事件经度
   */
  txFloat Event_location_lon() const TX_NOEXCEPT { return event_location_lon; }

  /**
   * @brief 获取事件影响道路速度（ms）
   * @return 事件影响道路速度（ms）
   */
  txFloat Event_influence_roads_speed_ms() const TX_NOEXCEPT { return event_influence_roads_speed_ms; }

  /**
   * @brief 获取事件影响范围
   *
   * @return 事件影响范围
   */
  InfluenceRange Event_influence_range() const TX_NOEXCEPT { return event_influence_range; }

  /**
   * @brief 获取事件影响的车道
   *
   * @return 事件影响的车道列表
   */
  std::vector<EventLaneUid> Event_influence_lanes() const TX_NOEXCEPT { return event_influence_lanes; }

  /**
   * @brief 获取事件影响的行为
   *
   * @return 事件影响的行为
   */
  InfluenceAction Event_influence_action() const TX_NOEXCEPT { return event_influence_action; }

  /**
   * @brief 获取事件影响的车道速度
   *
   * @return 事件影响的车道速度
   */
  txFloat Event_influence_lanes_speed_ms() const TX_NOEXCEPT { return event_influence_lanes_speed_ms; }

  /**
   * @brief 获取天气相关信息
   *
   * @return 天气相关信息字符串
   */
  txString WeatherStr() const TX_NOEXCEPT {
    std::ostringstream oss_rids;
    std::copy(std::begin(event_influence_roads), std::end(event_influence_roads),
              std::ostream_iterator<Base::txRoadID>(oss_rids, ","));
    std::ostringstream oss_rule_template;
    std::copy(std::begin(event_influence_rule_template), std::end(event_influence_rule_template),
              std::ostream_iterator<InfluenceRuleTemplate>(oss_rule_template, ","));

    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(event_type, Event_type()) << TX_VARS_NAME(event_id, Event_id())
        << TX_VARS_NAME(event_influence_roads, oss_rids.str())
        << TX_VARS_NAME(event_start_time_stamp_s, Event_start_time_stamp_s())
        << TX_VARS_NAME(event_duration_s, Event_duration_s())
        << TX_VARS_NAME(event_influence_rule, Event_influence_rule())
        << TX_VARS_NAME(event_influence_rule_template, oss_rule_template.str()) << "}";
    return oss.str();
  }

  /**
   * @brief accident 字符串输出
   *
   * @return txString
   */
  txString AccidentStr() const TX_NOEXCEPT {
    std::ostringstream oss_lanes;
    std::copy(std::begin(event_influence_lanes), std::end(event_influence_lanes),
              std::ostream_iterator<EventLaneUid>(oss_lanes, ","));
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(event_type, Event_type()) << TX_VARS_NAME(event_id, Event_id())
        << TX_VARS_NAME(event_location_lat, Event_location_lat())
        << TX_VARS_NAME(event_location_lon, Event_location_lon())
        << TX_VARS_NAME(event_start_time_stamp_s, Event_start_time_stamp_s())
        << TX_VARS_NAME(event_duration_s, Event_duration_s())
        << TX_VARS_NAME(event_influence_roads_speed_ms, Event_influence_roads_speed_ms())
        << TX_VARS_NAME(event_influence_range, Event_influence_range())
        << TX_VARS_NAME(event_influence_lanes, oss_lanes.str())
        << TX_VARS_NAME(event_influence_action, Event_influence_action()) << "}";
    return oss.str();
  }

  /**
   * @brief 获取模拟道路速度限制
   * @return 模拟道路速度限制字符串
   */
  txString str_sim_road_speed_limit() const TX_NOEXCEPT {
    std::ostringstream oss_rids;
    std::copy(std::begin(event_influence_roads), std::end(event_influence_roads),
              std::ostream_iterator<Base::txRoadID>(oss_rids, ","));
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(event_type, Event_type()) << TX_VARS_NAME(event_id, Event_id())
        << TX_VARS_NAME(event_influence_roads, oss_rids.str())
        << TX_VARS_NAME(event_start_time_stamp_s, Event_start_time_stamp_s())
        << TX_VARS_NAME(event_duration_s, Event_duration_s())
        << TX_VARS_NAME(event_influence_roads_speed_ms, Event_influence_roads_speed_ms()) << "}";
    return oss.str();
  }

  /**
   * @brief 获取模拟车道速度限制字符串
   * @return 模拟车道速度限制字符串
   */
  txString str_sim_lane_speed_limit() const TX_NOEXCEPT {
    std::ostringstream oss_lanes;
    std::copy(std::begin(event_influence_lanes), std::end(event_influence_lanes),
              std::ostream_iterator<EventLaneUid>(oss_lanes, ","));
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(event_type, Event_type()) << TX_VARS_NAME(event_id, Event_id())
        << TX_VARS_NAME(event_influence_lanes, oss_lanes.str())
        << TX_VARS_NAME(event_start_time_stamp_s, Event_start_time_stamp_s())
        << TX_VARS_NAME(event_duration_s, Event_duration_s())
        << TX_VARS_NAME(event_influence_lanes_speed_ms, Event_influence_lanes_speed_ms()) << "}";
    return oss.str();
  }

  /**
   * @brief 获取模拟道路关闭事件字符串
   * @return 模拟道路关闭事件字符串
   */
  txString str_sim_road_closure() const TX_NOEXCEPT {
    std::ostringstream oss_rids;
    std::copy(std::begin(event_influence_roads), std::end(event_influence_roads),
              std::ostream_iterator<Base::txRoadID>(oss_rids, ","));
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(event_type, Event_type()) << TX_VARS_NAME(event_id, Event_id())
        << TX_VARS_NAME(event_influence_roads, oss_rids.str())
        << TX_VARS_NAME(event_start_time_stamp_s, Event_start_time_stamp_s())
        << TX_VARS_NAME(event_duration_s, Event_duration_s()) << "}";
    return oss.str();
  }

  /**
   * @brief 获取模拟车道关闭事件
   * @return 模拟车道关闭事件字符串
   */
  txString str_sim_lane_closure() const TX_NOEXCEPT {
    std::ostringstream oss_lanes;
    std::copy(std::begin(event_influence_lanes), std::end(event_influence_lanes),
              std::ostream_iterator<EventLaneUid>(oss_lanes, ","));
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(event_type, Event_type()) << TX_VARS_NAME(event_id, Event_id())
        << TX_VARS_NAME(event_influence_lanes, oss_lanes.str())
        << TX_VARS_NAME(event_start_time_stamp_s, Event_start_time_stamp_s())
        << TX_VARS_NAME(event_duration_s, Event_duration_s()) << "}";
    return oss.str();
  }

  /**
   * @brief 根据事件的类型返回对应的字符串表示
   *
   * @return std::string
   */
  std::string Str() const TX_NOEXCEPT {
    switch (eEventType()) {
      case InjectEventType::sim_fog:
      case InjectEventType::sim_rainfall:
      case InjectEventType::sim_snowfall:
        return WeatherStr();
      case InjectEventType::sim_accident:
      case InjectEventType::sim_construction:
        return AccidentStr();
      case InjectEventType::sim_road_speed_limit:
        return str_sim_road_speed_limit();
      case InjectEventType::sim_lane_speed_limit:
        return str_sim_lane_speed_limit();
      case InjectEventType::sim_road_closure:
        return str_sim_road_closure();
      case InjectEventType::sim_lane_closure:
        return str_sim_lane_closure();
      default:
        break;
    }
    return "";
  }
  friend std::ostream& operator<<(std::ostream& os, const TrafficInjectEvent& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }
};

class TrafficInjectEventBatch {
 public:
  TrafficInjectEventBatch() = default;
  ~TrafficInjectEventBatch() = default;

 public:
  using txString = Base::txString;
  using txBool = Base::txBool;
  using txSize = Base::txSize;

 public:
  txSize batch_job_id;
  txString user_id;
  txBool weather_enable = false;
  txBool event_enable = false;
  txBool control_enable = false;
  std::vector<TrafficInjectEvent> event_list;

 public:
  /**
   * @brief 获取批次作业ID
   *
   * 该函数返回当前事件对应的批次作业ID，作为一个常量值。
   *
   * @return txSize 当前事件对应的批次作业ID
   */
  txSize Batch_job_id() const TX_NOEXCEPT { return batch_job_id; }

  /**
   * @brief 获取用户ID
   *
   * 该函数返回当前事件对应的用户ID，作为一个常量字符串。
   *
   * @return txString 当前事件对应的用户ID
   */
  txString User_id() const TX_NOEXCEPT { return user_id; }

  /**
   * @brief 是否启用天气功能
   *
   * @return txBool 如果启用天气功能则返回 true，否则返回 false
   */
  txBool Weather_enable() const TX_NOEXCEPT { return weather_enable; }

  /**
   * @brief 获取事件启用状态
   *
   * 该函数返回一个布尔值，表示当前是否启用了事件功能。
   *
   * @return 如果启用了事件功能，则返回 true；否则返回 false。
   */
  txBool Event_enable() const TX_NOEXCEPT { return event_enable; }

  /**
   * @brief 获取控制启用状态
   *
   * 该函数返回一个布尔值，表示当前是否启用了控制功能。
   *
   * @return 如果启用了控制功能，则返回 true；否则返回 false。
   */
  txBool Control_enable() const TX_NOEXCEPT { return control_enable; }

  /**
   * @brief 获取事件列表
   *
   * 返回一个包含所有事件的列表。
   *
   * @return 返回一个包含所有事件的列表。
   */
  std::vector<TrafficInjectEvent> Event_list() const TX_NOEXCEPT { return event_list; }

  /**
   * @brief 获取字符串形式的事件信息
   *
   * 返回一个包含事件信息的字符串。
   *
   * @return 返回一个包含事件信息的字符串。
   */
  std::string Str() const TX_NOEXCEPT {
    std::ostringstream oss_eventList;
    std::copy(std::begin(event_list), std::end(event_list),
              std::ostream_iterator<TrafficInjectEvent>(oss_eventList, ","));
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(batch_job_id, Batch_job_id()) << TX_VARS_NAME(user_id, User_id())
        << TX_VARS_NAME(weather_enable, bool2lpsz(Weather_enable()))
        << TX_VARS_NAME(event_enable, bool2lpsz(Event_enable()))
        << TX_VARS_NAME(control_enable, bool2lpsz(Control_enable())) << TX_VARS_NAME(event_list, oss_eventList.str())
        << "}";
    return oss.str();
  }
  friend std::ostream& operator<<(std::ostream& os, const TrafficInjectEventBatch& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }
};

extern TrafficInjectEventBatch parser_inject_event(const Base::txString json_content) TX_NOEXCEPT;
TX_NAMESPACE_CLOSE(Utils)
