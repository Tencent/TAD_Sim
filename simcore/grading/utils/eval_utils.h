// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <memory>
#include <mutex>

#include "utils/detection.h"
#include "utils/eval_common.h"

#include "grading_kpi.pb.h"
#include "pugixml.hpp"

namespace eval {
// map for all kpis
class EvalCfg;
typedef std::unordered_map<std::string, EvalCfg> EvalCfgMap;

// KPI configuration, default is not enabled
class EvalCfg {
 public:
  std::string _root_node = "Grading";
  std::string _algorithm_name;

  std::string _kpi_attrib = "Name";
  std::string _kpi_name;

  std::string _eval_name = "EvalValue";
  int _eval_value = 0;

  std::string _stop_name = "StopValue";
  int _stop_value = 0;

  std::string _thresh_name = "Threshold";
  double _thresh_value = 0.0;

  std::string _radius_name = "Radius";
  double _radius_value = 2.5;

  std::string _need_parking_name = "NeedParking";
  int _need_parking_value = 0;

  std::string _definition_name = "Definition";
  std::string _definition_value;

  std::string _category_name = "Category";
  std::string _category_value;

  // unordered map to keep all attributes of kpi in string format
  std::unordered_map<std::string, std::string> _pairs;

  bool _enabled = false;

  void DebugShow() const;
  bool GetValueAsDouble(const std::string &name, double &out) const;
  bool GetValueAsInt(const std::string &name, int &out) const;
  bool GetValueAsString(const std::string &name, std::string &out) const;
};

// scenario information loaded from .sim file
struct ScenarioInfo {
 public:
  CPosition m_end_point_enu;
  std::string m_cloud_dir;
  std::string m_eval_cfg_file;
  std::string m_report_dir;
  std::string m_map_file;
  std::string m_start_location;
  std::string m_scene_id;
  std::string m_scene_version;
  std::string m_scene_description;
  double m_ego_speed_limit;
  EvalPoints m_waypoints;
  std::string m_date_version;

  ScenarioInfo() {
    m_waypoints.reserve(eval::const_MSPS);
    m_ego_speed_limit = 22.3;
  }
};

// executed modules in this simulation
struct ModuleConfig {
  std::vector<std::string> m_module_names;
  std::vector<int> m_module_types;
  std::vector<std::string> m_module_paths;
  std::unordered_map<std::string, std::string> m_avg_steptime;           // unit second
  std::unordered_map<std::string, std::string> m_expected_avg_steptime;  // unit second
  std::unordered_map<std::string, std::string> m_steptime;               // unit second
  std::unordered_map<std::string, std::string> m_dutycycle;              // unit second
  std::unordered_map<std::string, std::string> m_overrun_count;

  ModuleConfig() { Clear(); }

  void Clear() {
    m_module_names.clear();
    m_module_types.clear();
    m_module_paths.clear();
    m_avg_steptime.clear();
    m_expected_avg_steptime.clear();
    m_steptime.clear();
    m_dutycycle.clear();
    m_overrun_count.clear();

    m_module_names.reserve(32);
    m_module_types.reserve(32);
    m_module_paths.reserve(32);
    m_avg_steptime.reserve(32);
    m_expected_avg_steptime.reserve(32);
    m_steptime.reserve(32);
    m_dutycycle.reserve(32);
    m_overrun_count.reserve(32);
  }
};

/**
 * @brief eval configuration load function.
 * It will load data from .sim file.
 * It will load kpis from grading.xml(active before 20230528).
 * It will load kpis from grading.json(active after 20230528).
 */
class EvalCfgLoader {
 public:
  virtual ~EvalCfgLoader() {}
  inline bool XMLValid() { return _xml_valid; }

  /**
   * @brief open and load xml file
   *
   * @param xml_file
   * @return true
   * @return false
   */
  bool LoadXMLFile(const std::string &xml_file);

  /**
   * @brief parse KPIs from xml file, KPIs defined in KPI::constSimCityKPIs will be enabled if in simcity mode
   *
   * @param eval_cfgs, output variable, all kpi will be loaded into this variable
   * @return true
   * @return false
   */
  bool LoadEvalCfgs(EvalCfgMap &eval_cfgs);

  /**
   * @brief parse scenario info from .sim file, and save info into ScenarioInfo.
   * ScenarioInfo will be dumped into test report.
   * grading.xml file path will also be extracted from .sim file if grading node is defined in .sim file.
   *
   * @param grading_file
   * @param scenario_info
   * @return true
   * @return false
   */
  bool ParseFromSim(std::string &grading_file, ScenarioInfo &scenario_info);

  /**
   * @brief load grading kpi group from grading.json
   * @param gradingKpiFile
   */
  void loadGradingKpiGroup(const std::string &gradingKpiFile);

  /**
   * @brief get grading kpi group
   * @return
   */
  const sim_msg::GradingKpiGroup &getGradingKpiGroup();

  /**
   * @brief get grading kpi from kpi group by kpi name
   * @param kpiName
   * @return true if bingo, otherwise false
   */
  bool getGradingKpiByName(const std::string &kpiName, sim_msg::GradingKpiGroup_GradingKpi &kpiOut);

 private:
  pugi::xml_document xml_doc;
  bool _xml_valid = false;
  std::string m_xml_path;

  sim_msg::GradingKpiGroup m_KpiGroup;
};

/**
 * @brief file utils
 */
class CFileUtils {
 public:
  static std::string AbsPath(const std::string &file);
  static std::string DirName(const std::string &abs_file);
  static std::string ParentDir(const std::string &one_path);
  static std::string JoinPath(const std::string &path1, const std::string &path2);
  static std::string BaseName(const std::string &file);
  static bool IsFileExist(const std::string &file);
  static bool Exist(const std::string &one_path);
  static bool DeleteFile(const std::string &file);
};

void StringSplit(const std::string &str_in, const std::string &spliter, std::vector<std::string> &out);

/**
 * @brief timing class, for debug. Start timing when the object is constructed, end timing and print when the object is
 * destroyed
 */
class CElapsedTime {
 public:
  explicit CElapsedTime(const std::string &prefix_string);
  ~CElapsedTime();

 private:
  void MarkStartTimePoint();

  void MarkEndTimePoint();

  float PrintElapsedTime(const std::string &prefix_string);

  std::chrono::high_resolution_clock::time_point g_start_t, g_end_t;
  std::string mPrefixString;
};
}  // namespace eval
