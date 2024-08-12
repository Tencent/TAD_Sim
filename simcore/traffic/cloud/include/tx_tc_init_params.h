// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include "nlohmann/json.hpp"
#include "structs/base_struct.h"
#include "tx_header.h"
#include "tx_map_info.h"
#include "tx_path_utils.h"
#include "tx_string_utils.h"
#include "tx_time_utils.h"
#include "tx_tc_gflags.h"
#include "tx_tc_marco.h"
#include "tx_tc_plugin_micro.h"
TX_NAMESPACE_OPEN(Scene)

using json = nlohmann::json;
#if __pingsn__
using map_range_t = Base::map_range_t;

struct parallel_simulation_t {
  /*
  "parallel_simulation" : {
    "map_file" : "D:/0.vt/microsim/pingsn.sqlite",
    "config_file" : "D:/0.vt/microsim/config.xml",
    "init_state_file" : "D:/0.vt/microsim/state.pb",
    "traffic_light_file" : "D:/0.vt/microsim/TrafficInfo.xml",
    "vehicles_file" : "D:/0.vt/microsim/vehicles_pingsn.pb",
    "traffic_statistics" : "D:/0.vt/microsim/traffic_stat.pb"
  },
  */
  Base::txString map_file;
  Base::txString config_file;
  Base::txString init_state_string;
  Base::txString traffic_light_string;
};
#endif /*__pingsn__*/
struct traffic_config_t {
  Base::txInt id;
  Base::txString module_bin_path;
  Base::txString module_type;
  Base::txString name;
  Base::txInt stepTime;
  Base::txInt timeout;

  /**
   * @brief 输出结构体的字符串表示
   *
   * @return Base::txString 结构化的字符串
   */
  Base::txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << "{" << TX_VARS(id) << TX_VARS(module_bin_path) << TX_VARS(module_type) << TX_VARS(name) << TX_VARS(stepTime)
        << TX_VARS(timeout) << "}";
    return oss.str();
  }
  friend std::ostream& operator<<(std::ostream& os, const traffic_config_t& v) {
    os << v.Str();
    return os;
  }
};

struct map_data_t {
  Base::txString data_path;
  Base::txInt map_id;
  map_range_t map_range;
  hadmap::txPoint origin_gps;
  Base::txString config_path;
#if __pingsn__
  parallel_simulation_t parallel_simulation_info;
#endif /*__pingsn__*/
  Base::txString hadmapFilter_path;
  /**
   * @brief 输出结构体的字符串表示
   *
   * @return Base::txString 结构化的字符串
   */
  Base::txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << "{" << TX_VARS(data_path) << TX_VARS(map_id) << TX_VARS(map_range)
        << TX_VARS_NAME(origin_gps, Utils::ToString(origin_gps)) << TX_VARS(config_path) << TX_VARS(hadmapFilter_path)
        << "}";
    return oss.str();
  }
  friend std::ostream& operator<<(std::ostream& os, const map_data_t& v) {
    os << v.Str();
    return os;
  }
};

extern std::ostream& operator<<(std::ostream& os, const sim_msg::MapPosition& pos);

struct surrounding_maps_t {
  Base::txInt surrounding_traffic_id;
  sim_msg::MapPosition surrounding_traffic_pos;
  map_range_t surrounding_traffic_map_range;

  /**
   * @brief 输出结构体的字符串表示
   *
   * @return Base::txString 结构化的字符串
   */
  Base::txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << "{" << TX_VARS(surrounding_traffic_id) << TX_VARS(surrounding_traffic_pos)
        << TX_VARS(surrounding_traffic_map_range) << "}";
    return oss.str();
  }

  friend std::ostream& operator<<(std::ostream& os, const surrounding_maps_t& v) {
    os << v.Str();
    return os;
  }
};

using SurroundingTrafficRangeMap = std::map<Base::txInt /*sim_msg::MapPosition*/, surrounding_maps_t>;

struct random_conf_t {
  Base::txBool valid = false;
  Base::txInt obs_row = 50;
  Base::txInt obs_col = 50;
  Base::txFloat obs_valid_radius = 500.0;
  Base::txInt obs_rnd_seed = 55;
  Base::txFloat obs_valid_section_length = 100.0;
  Base::txFloat pedestrian_valid_section_length = 10.0;

  Base::txInt pedestrian_show = 0;
  Base::txFloat pedestrian_velocity = 1.2;
  Base::txFloat pedestrian_occurrence_dist = 30.0;
};

struct InitConfigure_t {
  Base::txInt TX_MARK("id: region id") region_id;
  Base::txInt TX_MARK("log_level: 日志等级") log_level;
  Base::txString TX_MARK("log_dir: 日志文件夹路径，绝对路径") log_dir;
  Base::txInt time_step_in_ms;
  Base::txFloat perception_radius;
  Base::txFloat aggressiveness;
  std::list<Base::txString> conditions;
  TX_MARK("\"pedestrian\", \"obstacle\", \"laneChange\", \"acceleration\", \"runLight\"");
  Base::txFloat interval;
  map_data_t map_data;
  map_range_t map_range;
  // traffic_config_t traffic_config;
  SurroundingTrafficRangeMap surrounding_maps;
  Base::txInt sim_type = -1;
  TX_MARK("0:virtual city; 1: parallel simulation");
  Base::txInt max_vehicle_size = -1;
  std::vector<Base::txString> vec_loglevel;
  std::vector<Base::map_range_t> m_regions_range;

  /**
   * @brief 输出结构体的字符串表示
   *
   * @return Base::txString 结构化的字符串
   */
  Base::txString Str() const TX_NOEXCEPT {
    std::ostringstream ossConditions;
    { std::copy(conditions.begin(), conditions.end(), std::ostream_iterator<Base::txString>(ossConditions, ", ")); }

    std::ostringstream ossSurroundingMaps;
    {
      for (const auto& refmapRange : surrounding_maps) {
        ossSurroundingMaps << refmapRange.second;
      }
    }

    std::ostringstream ossLogLevel;
    {
      for (const auto& flags : vec_loglevel) {
        ossLogLevel << flags << ",";
      }
    }
    std::ostringstream ossRegionsRange;
    for (const auto& refRegionsRange : m_regions_range) {
      ossRegionsRange << refRegionsRange.Str();
    }
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(id, region_id) << TX_VARS(log_level) << TX_VARS(log_dir)
        << TX_VARS_NAME(t, time_step_in_ms) << TX_VARS_NAME(perception_radius, perception_radius)
        << TX_VARS(aggressiveness) << TX_VARS_NAME(conditions, ossConditions.str()) << TX_VARS(interval)
        << TX_VARS(map_data) << TX_VARS(map_range) << TX_VARS_NAME(surrounding_maps, ossSurroundingMaps.str())
        << TX_VARS(sim_type) << TX_VARS(max_vehicle_size) << TX_VARS_NAME(LogLevels, ossLogLevel.str())
        << TX_VARS_NAME(RegionsRange, ossRegionsRange.str()) << "}";
    return oss.str();
  }
  friend std::ostream& operator<<(std::ostream& os, const InitConfigure_t& v) {
    os << v.Str();
    return os;
  }
};

/**
 * @brief 将 JSON 对象转换为 surrounding_maps_t 结构
 *
 * 该函数用于将 JSON 对象转换为 surrounding_maps_t 结构。
 *
 * @param j 输入的 JSON 对象
 * @param surrounding_map 存储转换结果的 surrounding_maps_t 结构
 */
void from_json(const json& j, surrounding_maps_t& surrounding_map);

/**
 * @brief 从 JSON 对象转换为 traffic_config_t 结构体
 *
 * 该函数将输入的 JSON 对象转换为 traffic_config_t 结构体，并将转换结果存储在传入的参数中。
 *
 * @param j 输入的 JSON 对象
 * @param _traffic_config 存储转换结果的 traffic_config_t 结构体
 */
void from_json(const json& j, traffic_config_t& _traffic_config);

/**
 * @brief 从 JSON 对象解析参数
 *
 * 解析给定的 JSON 对象，并将解析后的参数存储到传入的 map_range_t 参数中。
 *
 * @param j        要解析的 JSON 对象
 * @param _map_range 存储解析结果的 map_range_t 参数
 */
void from_json(const json& j, Base::map_range_t& _map_range);

/**
 * @brief 解析 JSON 对象并填充 map_data_t 结构体
 *
 * 此函数根据传入的 JSON 对象，解析其中的数据，并将解析后的数据填充至传入的 map_data_t 结构体中。
 *
 * @param j       要解析的 JSON 对象
 * @param _map_data 存储解析结果的 map_data_t 结构体
 */
void from_json(const json& j, map_data_t& _map_data);

/**
 * @brief 将 JSON 对象转换为初始化配置结构体
 *
 * 此函数将从给定的 JSON 对象中解析所需的数据，并填充初始化配置结构体。
 *
 * @param j       输入的 JSON 对象
 * @param _input_params 存储解析结果的初始化配置结构体
 */
void from_json(const json& j, InitConfigure_t& _input_params);

class InitInfoWrap {
  using txULong = Base::txULong;
  using txUInt = Base::txUInt;
  using txFloat = Base::txFloat;
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txString = Base::txString;
  enum { eVirtualCity = 0, eParallelSimulation = 1 };

 public:
  struct map_range_info_t {
    map_range_t map_range;
    txFloat perception_radius;
  };

 public:
  InitInfoWrap() TX_DEFAULT;
  ~InitInfoWrap() TX_DEFAULT;

  /**
   * @brief Reset the traffic cloud's initial parameters
   *
   * This function is used to reset the traffic cloud's initial parameters.
   *
   * @param[in] _new Initial parameters of the traffic cloud
   *
   * @retval None
   */
  void Reset(const TrafficCloud::TrafficManager::InitInfo& _new) TX_NOEXCEPT {
    configfile_path = _new.configfile_path;
    if (CallSucc(exists(Utils::FilePath(configfile_path)))) {
      std::ifstream in_json(configfile_path);
      json re_parse_json = json::parse(in_json);
      initConf = re_parse_json;

      if (initConf.vec_loglevel.size() > 1) {
        std::string merge_loglevel;
        for (const auto& refLogLevet : initConf.vec_loglevel) {
          LOG(INFO) << refLogLevet << " ";
          merge_loglevel += refLogLevet;
          merge_loglevel += " ";
        }
        initConf.vec_loglevel.clear();
        initConf.vec_loglevel.emplace_back(merge_loglevel);
      }

      if (1 == initConf.vec_loglevel.size()) {
        fresh_log_level(initConf.vec_loglevel.front());
      } else {
        std::ostringstream oss;
        oss << "command params error. " << TX_VARS(initConf.vec_loglevel.size());
        for (const auto& refParams : initConf.vec_loglevel) {
          oss << refParams;
        }
        LOG(WARNING) << oss.str();
      }
      LOG(INFO) << TX_VARS_NAME(InParams, initConf);

      mValid = true;
    } else {
      mValid = false;
      LOG(WARNING) << " Init Conf File do not exist. " << TX_VARS(configfile_path);
    }
  }

  /**
   * @brief 根据Simulation生成的交通信息，创建具有指定路径和场景原点的Traffic场景
   *
   * @param str_traffic_xml_path    交通信息XML文件路径
   * @param scene_origin_pgs        场景原点的位置坐标，以PGS(Pixel Grid System)为单位
   * @param str_scene_hdmap_path    场景地图文件路径，可根据需要指定
   *
   * @return 无
   *
   */
  void CreateInfoFromSim(const Base::txString& str_traffic_xml_path, const hadmap::txPoint& scene_origin_pgs,
                         const Base::txString& str_scene_hdmap_path) TX_NOEXCEPT {
    initConf.region_id = 66;
    initConf.log_level = 0;
    initConf.log_dir = "";
    initConf.time_step_in_ms = 20;
    initConf.map_data.config_path = str_traffic_xml_path;
    initConf.map_data.origin_gps = scene_origin_pgs;
    initConf.map_data.data_path = str_scene_hdmap_path;
    initConf.perception_radius = FLT_MAX;
    initConf.map_range.center = scene_origin_pgs;
    initConf.map_range.bottom_left =
        hadmap::txPoint(__Lon__(scene_origin_pgs) - 0.5, __Lat__(scene_origin_pgs) - 0.5, 0.0);
    initConf.map_range.top_right =
        hadmap::txPoint(__Lon__(scene_origin_pgs) + 0.5, __Lat__(scene_origin_pgs) + 0.5, 0.0);
    initConf.sim_type = eVirtualCity;
    initConf.max_vehicle_size = FLAGS_MAX_VEHICLE_LIMIT_COUNT; /*2000*/
  }

  /**
   * @brief 释放初始化参数对象
   *
   * 在使用完初始化参数对象后，使用此方法释放资源
   *
   */
  void Release() TX_NOEXCEPT { mValid = false; }

  /**
   * @brief 获取初始化参数对象是否有效
   *
   * 检查初始化参数对象是否已正确初始化，如果返回值为 true，则该对象有效；否则，该对象无效。
   *
   * @return 返回 true 表示对象有效，false 表示对象无效。
   */
  txBool IsValid() const TX_NOEXCEPT { return mValid; }

  /**
   * @brief 返回初始化参数字符串
   *
   * 返回一个包含初始化参数信息的字符串，可用于调试或打印输出.
   *
   * @return 返回一个包含初始化参数信息的字符串.
   */
  txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << "{" << TX_COND_NAME(mValid, mValid) << TX_VARS(configfile_path) << TX_VARS_NAME(configfile_context, initConf)
        << "}";
    return oss.str();
  }
  friend std::ostream& operator<<(std::ostream& os, const InitInfoWrap& v) {
    os << "InitInfoWrap : " << v.Str();
    return os;
  }

 public:
  /**
   * @brief 获取初始化参数中的区域ID
   *
   * 返回一个整数，表示初始化参数中的区域ID.
   *
   * @return 返回区域ID.
   */
  txInt get_region_id() const TX_NOEXCEPT { return initConf.region_id; }

  /**
   * @brief 获取初始化参数中的日志级别
   *
   * 返回一个整数，表示初始化参数中的日志级别.
   *
   * @return 返回日志级别.
   */
  txInt get_log_level() const TX_NOEXCEPT { return initConf.log_level; }

  /**
   * @brief 获取日志目录
   *
   * @return txString
   */
  txString get_log_dir() const TX_NOEXCEPT { return initConf.log_dir; }

  /**
   * @brief 获取初始化参数中的时间步长（单位：毫秒）
   *
   * 返回一个整数，表示初始化参数中的时间步长，单位为毫秒。
   *
   * @return 返回时间步长，单位为毫秒.
   */
  txInt get_time_step_in_ms() const TX_NOEXCEPT { return initConf.time_step_in_ms; }

  /**
   * @brief 获取初始化参数中的时间步长（单位：秒）
   *
   * 返回一个浮点数，表示初始化参数中的时间步长，单位为秒。
   *
   * @return 返回时间步长，单位为秒.
   */
  txFloat get_time_step_in_s() const TX_NOEXCEPT { return Utils::MillisecondToSecond((txFloat)get_time_step_in_ms()); }

  /**
   * @brief 获取初始化参数中的配置文件路径
   *
   * 返回一个字符串，表示初始化参数中的配置文件路径.
   *
   * @return 返回配置文件路径
   */
  txString get_configfile_path() const TX_NOEXCEPT { return configfile_path; }

  /**
   * @brief 获取初始化参数中的配置文件路径
   *
   * 返回一个字符串，表示初始化参数中的配置文件路径。
   *
   * @return 返回配置文件路径
   */
  txString get_traffic_xml_path() const TX_NOEXCEPT { return initConf.map_data.config_path; }

  /**
   * @brief 获取初始化参数中的场景原点坐标
   *
   * 返回一个包含场景原点的地理坐标的对象，以经度为 x 轴，纬度为 y 轴，高度为 z 轴表示。
   *
   * @return 返回场景原点的地理坐标对象
   */
  hadmap::txPoint get_scene_origin_pgs() const TX_NOEXCEPT { return initConf.map_data.origin_gps; }

  /**
   * @brief 获取场景hadmap地图路径
   *
   * 返回一个字符串，表示场景中使用的hadmap地图数据路径。
   *
   * @return 返回场景hadmap地图数据路径字符串
   */
  txString get_scene_hdmap_path() const TX_NOEXCEPT { return initConf.map_data.data_path; }

  /**
   * @brief 获取场景hadmap地图过滤器路径
   *
   * 返回一个字符串，表示场景中使用的hadmap地图数据过滤器路径。
   *
   * @return 返回场景hadmap地图过滤器路径字符串
   */
  txString get_scene_hdmap_filter_path() const TX_NOEXCEPT { return initConf.map_data.hadmapFilter_path; }

  /**
   * @brief 获取地图管理器的范围信息
   *
   * 返回一个 map_range_info_t 类型的结构体，包含地图管理器中使用的范围信息。
   *
   * @return map_range_info_t 类型的结构体，包含地图管理器中使用的范围信息
   */
  map_range_info_t get_map_manager_range_info() const TX_NOEXCEPT;

  /**
   * @brief 是否是虚拟城市类型
   *
   * @return txBool
   */
  txBool isVirtualCity() const TX_NOEXCEPT { return eVirtualCity == initConf.sim_type; }

  /**
   * @brief 是否为并行模拟
   *
   * 如果初始化配置中的模拟类型是并行模拟，则返回true，否则返回false。
   *
   * @return txBool 类型的标志，表示模拟类型是否为并行模拟
   */
  txBool isParallelSimulation() const TX_NOEXCEPT { return eParallelSimulation == initConf.sim_type; }
#if __pingsn__

  /**
   * @brief 获取并行模拟的地图文件名称
   *
   * 返回一个字符串，包含当前初始化配置中的并行模拟信息的地图文件名称。
   *
   * @return txString 类型的地图文件名称
   */
  txString get_ps_map_file() const noexcept { return initConf.map_data.parallel_simulation_info.map_file; }

  /**
   * @brief 获取并行模拟的配置文件名称
   *
   * 返回一个字符串，包含当前初始化配置中的并行模拟信息的配置文件名称。
   *
   * @return txString 类型的配置文件名称
   */
  txString get_ps_config_file() const noexcept { return initConf.map_data.parallel_simulation_info.config_file; }

  /**
   * @brief 获取并行模拟的初始状态字符串
   *
   * 返回一个字符串，表示当前初始化配置中的并行模拟信息的初始状态字符串。
   *
   * @return txString 类型的初始状态字符串
   */
  txString get_ps_init_state_string() const noexcept {
    return initConf.map_data.parallel_simulation_info.init_state_string;
  }

  /**
   * @brief 获取并行模拟的红绿灯信息字符串
   *
   * 返回一个字符串，表示当前初始化配置中的并行模拟信息的红绿灯信息字符串。
   *
   * @return txString 类型的红绿灯信息字符串
   */
  txString get_ps_traffic_light_string() const noexcept {
    return initConf.map_data.parallel_simulation_info.traffic_light_string;
  }
#endif /*__pingsn__*/

  /**
   * @brief 配置初始化
   *
   * 使用此函数配置初始化参数。
   *
   * @param[in] _t 要配置的初始化参数
   * @return true 如果参数配置成功
   * @return false 如果参数配置失败
   */
  txBool set_InitConfigure(const InitConfigure_t& _t) TX_NOEXCEPT {
    initConf = _t;
    mValid = true;
    return mValid;
  }

  /**
   * @brief 获取最大车辆数量
   *
   * 使用此函数获取最大车辆数量。
   *
   * @return txInt 最大车辆数量
   */
  txInt get_max_vehicle_size() const TX_NOEXCEPT { return initConf.max_vehicle_size; }

  /**
   * @brief 用于更新日志等级的函数
   *
   * 这个函数用于更新日志等级设置。输入参数是一个包含日志等级更新信息的字符串。
   *
   * @param flags_params 包含日志等级更新信息的字符串
   */
  void fresh_log_level(Base::txString flags_params) const TX_NOEXCEPT;

  /**
   * @brief 返回各个预测区域的范围信息
   *
   * 使用此函数获取整个区域的范围信息列表。
   *
   * @return std::vector<Base::map_range_t> 各个预测区域的范围信息列表
   */
  std::vector<Base::map_range_t> regions_range() const TX_NOEXCEPT { return initConf.m_regions_range; }

 protected:
  txBool mValid = false;
  txString configfile_path;
  InitConfigure_t initConf;
};

TX_NAMESPACE_CLOSE(Scene)
