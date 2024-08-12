// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_init_params.h"
#include "tx_path_utils.h"
#include "tx_tc_gflags.h"
TX_NAMESPACE_OPEN(Scene)

#if __pingsn__
void from_json(const json& j, parallel_simulation_t& _ps_info) {
  _ps_info.map_file = j.at("map_file");
  _ps_info.config_file = j.at("config_file");
  _ps_info.init_state_string = j.at("init_state_file");
  _ps_info.traffic_light_string = j.at("traffic_light_file");
}
#endif /*__pingsn__*/

void from_json(const json& j, surrounding_maps_t& _surrounding_map) {
  _surrounding_map.surrounding_traffic_id = j.at("id");
  _surrounding_map.surrounding_traffic_pos = j.at("pos");
  _surrounding_map.surrounding_traffic_map_range = j.at("map_range");
}

void from_json(const json& j, traffic_config_t& _traffic_config) {
  _traffic_config.id = j.at("id");
  _traffic_config.module_bin_path = j.at("module_bin_path");
  _traffic_config.module_type = j.at("module_type");
  _traffic_config.name = j.at("name");
  _traffic_config.stepTime = j.at("stepTime");
  _traffic_config.timeout = j.at("timeout");
}

// void from_json(const json& j, Base::map_range_t& _map_range)
// {
//     const auto& j_bottom_left = j.at("bottom_left");
//     _map_range.bottom_left = hadmap::txPoint(j_bottom_left.at("lon"), j_bottom_left.at("lat"), 0.0);
//     const auto& j_top_right = j.at("top_right");
//     _map_range.top_right = hadmap::txPoint(j_top_right.at("lon"), j_top_right.at("lat"), 0.0);
// }

void from_json(const json& j, map_data_t& _map_data) {
  _map_data.data_path = j.at("data_path");
  _map_data.map_id = j.at("map_id");
  _map_data.map_range = j.at("map_range");
  _map_data.origin_gps = hadmap::txPoint(j.at("origin_x"), j.at("origin_y"), 0.0);
  _map_data.config_path = j.at("config_path");
#if __pingsn__
  if (j.end() != j.find("parallel_simulation")) {
    _map_data.parallel_simulation_info = j.at("parallel_simulation");
  }
#endif /*__pingsn__*/

  if (j.end() != j.find("hadmap_filter_path")) {
    _map_data.hadmapFilter_path = j.at("hadmap_filter_path");
  }
}

void from_json(const json& j, InitConfigure_t& _input_params) {
  _input_params.sim_type = j.at("sim_type");
  _input_params.region_id = j.at("id");
  _input_params.log_level = j.at("log_level");
  _input_params.log_dir = j.at("log_dir");
  _input_params.time_step_in_ms = j.at("t");
  _input_params.perception_radius = j.at("perception_radius");
  _input_params.aggressiveness = j.at("aggressiveness");
  const json& conditions = j.at("conditions");
  if (conditions.size() > 0) {
    _input_params.conditions.resize(conditions.size());
    std::copy(conditions.begin(), conditions.end(), _input_params.conditions.begin());
  } else {
    _input_params.conditions.clear();
  }
  _input_params.interval = j.at("interval");
  _input_params.map_data = j.at("map_data");
  _input_params.map_range = j.at("map_range");
  // _input_params.traffic_config = j.at("traffic_config");

  const json& surrounding_maps = j.at("surrounding_maps");
  if (surrounding_maps.size() > 0) {
    for (auto itr = surrounding_maps.begin(); itr != surrounding_maps.end(); ++itr) {
      _input_params.surrounding_maps[(*itr).at("pos")] = (*itr);
    }
  } else {
    _input_params.surrounding_maps.clear();
  }
  if (j.end() != j.find("max_vehicle_size")) {
    _input_params.max_vehicle_size = j.at("max_vehicle_size");
  }
  if (j.end() != j.find("LogLevel")) {
    const json& loglevel_json = j.at("LogLevel");
    for (auto itr = loglevel_json.begin(); itr != loglevel_json.end(); ++itr) {
      _input_params.vec_loglevel.emplace_back(*itr);
    }
  }

  if (j.end() != j.find("regions_range")) {
    const auto& ref_regions_range = j.at("regions_range");
    for (auto itr = ref_regions_range.begin(); itr != ref_regions_range.end(); ++itr) {
      Base::map_range_t cur_range;
      Base::from_json(*itr, cur_range);
      _input_params.m_regions_range.emplace_back(cur_range);
    }
  }
}

InitInfoWrap::map_range_info_t InitInfoWrap::get_map_manager_range_info() const noexcept {
  map_range_info_t retNode;
  retNode.map_range = initConf.map_range;
  /*retNode.bottom_left = initConf.map_range.bottom_left;
  retNode.top_right = initConf.map_range.top_right;*/
  retNode.perception_radius = initConf.perception_radius;
  return retNode;
}

std::ostream& operator<<(std::ostream& os, const sim_msg::MapPosition& pos) {
  switch (pos) {
    case sim_msg::MapPosition::MAP_POSITION_TOP: {
      os << "MAP_POSITION_TOP";
      break;
    }
    case sim_msg::MapPosition::MAP_POSITION_TOP_LEFT: {
      os << "MAP_POSITION_TOP_LEFT";
      break;
    }
    case sim_msg::MapPosition::MAP_POSITION_LEFT: {
      os << "MAP_POSITION_LEFT";
      break;
    }
    case sim_msg::MapPosition::MAP_POSITION_BOTTOM_LEFT: {
      os << "MAP_POSITION_BOTTOM_LEFT";
      break;
    }
    case sim_msg::MapPosition::MAP_POSITION_BOTTOM: {
      os << "MAP_POSITION_BOTTOM";
      break;
    }
    case sim_msg::MapPosition::MAP_POSITION_BOTTOM_RIGHT: {
      os << "MAP_POSITION_BOTTOM_RIGHT";
      break;
    }
    case sim_msg::MapPosition::MAP_POSITION_RIGHT: {
      os << "MAP_POSITION_RIGHT";
      break;
    }
    case sim_msg::MapPosition::MAP_POSITION_TOP_RIGHT: {
      os << "MAP_POSITION_TOP_RIGHT";
      break;
    }
    default: {
      os << "MAP_POSITION_UNKNOWN";
      break;
    }
  }
  return os;
}

char* convert(const std::string& s) {
  char* pc = new char[s.size() + 1];
  std::strcpy(pc, s.c_str());
  return pc;
}

void InitInfoWrap::fresh_log_level(Base::txString flags_params) const TX_NOEXCEPT {
  auto paramsVec = Utils::SpliteStringVector(flags_params, Base::txString(" "));
  if (_NonEmpty_(paramsVec)) {
    std::string strAppInfo;
    Utils::GetAppPath(Base::txString("micro_traffic_cloud"), strAppInfo);
    paramsVec.insert(paramsVec.begin(), strAppInfo);
    int argc = paramsVec.size();
    char** argv = new char*[argc + 1];
    for (int idx = 0; idx < argc; ++idx) {
      Utils::trim(paramsVec[idx]);
      argv[idx] = convert(paramsVec[idx]);
    }
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true TX_MARK("erase gflags item."));
    PrintFlagsValuesTrafficCloud();
  }
  return;
}
TX_NAMESPACE_CLOSE(Scene)
