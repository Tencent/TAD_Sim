// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_cloud_event_json_parser.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include "nlohmann/json.hpp"
TX_NAMESPACE_OPEN(Utils)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EventInflunceLaneRule, laneId, turn_left, turn_right, upstream_block);
std::string EventInflunceLaneRule::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_VARS(laneId) << TX_COND(turn_left) << TX_COND(turn_right) << TX_VARS(upstream_block);
  return oss.str();
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(VirtualCityEvent, event_type, event_location_lat, event_location_lon,
                                                event_location_longitudinal_offset, event_influnce_lane, event_id,
                                                event_duration, event_influnce_roads, event_influnce_roads_speed,
                                                event_speed_limit_start_lat, event_speed_limit_start_lon,
                                                event_speed_limit_end_lat, event_speed_limit_end_lon,
                                                event_influnce_lane_rule);

std::string VirtualCityEvent::Str() const TX_NOEXCEPT {
  std::ostringstream oss, oss_lane, oss_roads, oss_lane_rules;
  std::copy(event_influnce_lane.begin(), event_influnce_lane.end(), std::ostream_iterator<int64_t>(oss_lane, " "));
  std::copy(event_influnce_roads.begin(), event_influnce_roads.end(), std::ostream_iterator<int64_t>(oss_roads, " "));
  for (const auto& refRule : event_influnce_lane_rule) {
    oss_lane_rules << refRule.Str() << " ";
  }

  oss << _StreamPrecision_ << std::endl
      << TX_VARS(event_id) << std::endl
      << TX_VARS(event_duration) << std::endl
      << TX_VARS(event_type) << std::endl
      << TX_VARS(event_location_lon) << std::endl
      << TX_VARS(event_location_lat) << std::endl
      << TX_VARS(event_location_longitudinal_offset) << std::endl
      << TX_VARS(event_influnce_roads_speed) << std::endl
      << TX_VARS_NAME(event_influnce_lane, oss_lane.str()) << std::endl
      << TX_VARS_NAME(event_influnce_roads, oss_roads.str()) << std::endl
      << TX_VARS(event_speed_limit_start_lon) << std::endl
      << TX_VARS(event_speed_limit_start_lat) << std::endl
      << TX_VARS(event_speed_limit_end_lon) << std::endl
      << TX_VARS(event_speed_limit_end_lat) << std::endl
      << TX_VARS_NAME(event_influnce_lane_rule, oss_lane_rules.str()) << std::endl;
  return oss.str();
}

VirtualCityEvent parser_virtual_city_event(const Base::txString json_content) TX_NOEXCEPT {
  VirtualCityEvent ret_json;
  nlohmann::json re_parse_json = nlohmann::json::parse(json_content);
  from_json(re_parse_json, ret_json);
  return ret_json;
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(InfluenceRuleTemplate, name, threshold_ms, speed_limit_value_ms,
                                                speed_limit_factor);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(InfluenceRange, upstream_m, downstream_m);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EventLaneUid, rid, sid, lid);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(InfluenceAction, turn_left, turn_right);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(TrafficInjectEvent, event_type, event_id, event_influence_roads,
                                                event_start_time_stamp_s, event_duration_s, event_influence_rule,
                                                event_influence_rule_template, event_location_lat, event_location_lon,
                                                event_influence_roads_speed_ms, event_influence_range,
                                                event_influence_lanes, event_influence_action,
                                                event_influence_lanes_speed_ms);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(TrafficInjectEventBatch, batch_job_id, user_id, weather_enable,
                                                event_enable, control_enable, event_list);

TrafficInjectEventBatch parser_inject_event(const Base::txString json_content) TX_NOEXCEPT {
  TrafficInjectEventBatch ret_json;
  try {
    nlohmann::json re_parse_json = nlohmann::json::parse(json_content);
    from_json(re_parse_json, ret_json);
  } catch (std::exception* e) {
    std::cout << (e->what());
  }
  return ret_json;
}

TX_NAMESPACE_CLOSE(Utils)
