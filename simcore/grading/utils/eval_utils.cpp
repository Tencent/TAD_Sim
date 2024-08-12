// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "utils/eval_utils.h"
#include "boost/filesystem.hpp"
#include "utils/proto_helper.h"

#include <algorithm>

namespace eval {
bool EvalCfg::GetValueAsDouble(const std::string &name, double &out) const {
  std::string value_str;
  GetValueAsString(name, value_str);
  if (!value_str.empty()) {
    try {
      out = std::atof(value_str.c_str());
      return true;
    } catch (const std::exception &e) {
      LOG_ERROR << "conversion failed, " << e.what() << "\n";
    }
  }
  return false;
}
bool EvalCfg::GetValueAsInt(const std::string &name, int &out) const {
  std::string value_str;
  GetValueAsString(name, value_str);
  if (!value_str.empty()) {
    try {
      out = std::atoi(value_str.c_str());
      return true;
    } catch (const std::exception &e) {
      LOG_ERROR << "conversion failed, " << e.what() << "\n";
    }
  }
  return false;
}
bool EvalCfg::GetValueAsString(const std::string &name, std::string &out) const {
  auto iter = _pairs.find(name);
  if (iter != _pairs.end() && !iter->second.empty()) {
    out = iter->second;
    return true;
  }
  return false;
}

void EvalCfg::DebugShow() const {
  for (auto iter = _pairs.begin(); iter != _pairs.end(); ++iter) {
    VLOG_2 << "key:" << iter->first << ", value:" << iter->second;
  }
  VLOG_2 << "\n";
}

// load xml
bool EvalCfgLoader::LoadXMLFile(const std::string &xml_file) {
  pugi::xml_parse_result parse_result = xml_doc.load_file(xml_file.c_str());
  m_xml_path = xml_file;
  _xml_valid = parse_result.status == pugi::xml_parse_status::status_ok;
  return _xml_valid;
}

bool EvalCfgLoader::LoadEvalCfgs(EvalCfgMap &eval_cfgs) {
  // clear
  eval_cfgs.clear();

  // root node
  pugi::xml_node node_grading = xml_doc.child("Grading");

  for (auto algorithm_node : node_grading.children()) {
    // skip if empty
    if (algorithm_node.empty()) {
      continue;
    }

    for (auto kpi_node : algorithm_node.children()) {
      if (kpi_node.empty()) {
        continue;
      }

      // one configuration of one KPI
      EvalCfg eval_cfg;

      eval_cfg._algorithm_name = algorithm_node.name();

      auto kpi_name_node = kpi_node.attribute(eval_cfg._kpi_attrib.c_str());
      if (!kpi_name_node.empty()) eval_cfg._kpi_name = kpi_name_node.as_string();

      // keep all attributes
      auto kpi_attributes = kpi_node.attributes();
      for (auto iter = kpi_attributes.begin(); iter != kpi_attributes.end(); ++iter) {
        std::string key(iter->name());
        std::string value(iter->value());
        if (key.size() > 0) eval_cfg._pairs[key] = value;
      }

      // thresh, eval value, stop value
      auto thresh_node = kpi_node.attribute(eval_cfg._thresh_name.c_str());
      if (!thresh_node.empty()) eval_cfg._thresh_value = thresh_node.as_double();
      auto eval_node = kpi_node.attribute(eval_cfg._eval_name.c_str());
      if (!eval_node.empty()) eval_cfg._eval_value = eval_node.as_int();
      auto stop_node = kpi_node.attribute(eval_cfg._stop_name.c_str());
      if (!stop_node.empty()) eval_cfg._stop_value = stop_node.as_int();

      eval_cfg._radius_value = 2.5;
      auto radius_attrib = kpi_node.attribute(eval_cfg._radius_name.c_str());
      if (!radius_attrib.empty()) eval_cfg._radius_value = radius_attrib.as_double();

      eval_cfg._need_parking_value = 0;
      auto parking_attrib = kpi_node.attribute(eval_cfg._need_parking_name.c_str());
      if (!parking_attrib.empty()) eval_cfg._need_parking_value = parking_attrib.as_int();

      eval_cfg._definition_value = "";
      auto definition_attrib = kpi_node.attribute(eval_cfg._definition_name.c_str());
      if (!definition_attrib.empty()) eval_cfg._definition_value = definition_attrib.as_string();

      eval_cfg._category_value = "";
      auto category_attrib = kpi_node.attribute(eval_cfg._category_name.c_str());
      if (!category_attrib.empty()) eval_cfg._category_value = category_attrib.as_string();

      eval_cfg._enabled = true;

      // only enable simcity eval kpis
      const auto &evalParams = getEvalParams();
      if (evalParams.m_is_simcity) {
        // default is not enable
        eval_cfg._enabled = false;

        // only enable specific kpi
        for (const auto &kpi : SimCity::constSimCityKPIs) {
          if (kpi == eval_cfg._kpi_name) eval_cfg._enabled = true;
        }
      }

      eval_cfgs[eval_cfg._kpi_name] = eval_cfg;

      eval_cfg.DebugShow();
    }
  }
  return true;
}

bool EvalCfgLoader::ParseFromSim(std::string &grading_file, ScenarioInfo &scenario_info) {
  // root node
  pugi::xml_node node_sim = xml_doc.child("simulation");

  if (!node_sim.empty()) {
    auto version_attrib = node_sim.attribute("version");
    if (!version_attrib.empty()) scenario_info.m_scene_version = version_attrib.as_string();

    auto id_attrib = node_sim.attribute("sim_id");
    if (!id_attrib.empty()) scenario_info.m_scene_id = id_attrib.as_string();

    auto date_version_attrib = node_sim.attribute("date_version");
    if (!date_version_attrib.empty()) scenario_info.m_date_version = date_version_attrib.as_string();

    auto node_info = node_sim.child("info");
    if (!node_info.empty()) {
      scenario_info.m_scene_description = node_info.text().as_string();
    }

    auto node_grading = node_sim.child("grading");
    if (!node_grading.empty()) {
      grading_file = CFileUtils::JoinPath(CFileUtils::DirName(m_xml_path), node_grading.text().as_string());
      LOG(INFO) << "grading file in .sim file:" << grading_file << "\n";
    } else {
      LOG_WARNING << "node grading not in sim file.\n";
      return false;
    }
  } else {
    LOG_WARNING << "node simulation empty.\n";
    return false;
  }
  return true;
}

/**
 * @brief load grading kpi group from grading.json
 * @param gradingKpiFile
 */
void EvalCfgLoader::loadGradingKpiGroup(const std::string &gradingKpiFile) {
  // clear first
  m_KpiGroup.Clear();

  // load json content from grading.json
  std::string content = eval::loadContentFromFile(gradingKpiFile);

  if (content.size() == 0) {
    LOG_ERROR << "eval file contains nothing. " << gradingKpiFile << "\n";
    return;
  }

  if (!eval::jsonToProto(content, m_KpiGroup)) {
    LOG_ERROR << "fail to convert eval configuration json to sim_msg::GradingKpiGroup. " << content << "\n";
    m_KpiGroup.Clear();
  }

  VLOG_1 << "grading| load grading kpi group = " << m_KpiGroup.DebugString();
}

/**
 * @brief get grading kpi group
 * @return
 */
const sim_msg::GradingKpiGroup &EvalCfgLoader::getGradingKpiGroup() { return m_KpiGroup; }

/**
 * @brief get grading kpi from kpi group by kpi name
 * @param kpiName
 * @return true if bingo, otherwise false
 */
bool EvalCfgLoader::getGradingKpiByName(const std::string &kpiName, sim_msg::GradingKpiGroup_GradingKpi &kpiOut) {
  for (const auto &kpi : m_KpiGroup.kpi()) {
    if (kpi.name() == kpiName) {
      kpiOut.CopyFrom(kpi);
      return true;
    }
  }
  return false;
}

std::string CFileUtils::AbsPath(const std::string &file) {
  boost::filesystem::path abs_path = boost::filesystem::absolute(file);
  return abs_path.string();
}
std::string CFileUtils::BaseName(const std::string &file) {
  boost::filesystem::path file_path(file);
  return file_path.filename().string();
}
std::string CFileUtils::DirName(const std::string &abs_file) {
  boost::filesystem::path dir_path(abs_file);

  if (boost::filesystem::is_directory(abs_file)) {
    return dir_path.string();
  }

  return dir_path.parent_path().string();
}
std::string CFileUtils::ParentDir(const std::string &one_path) {
  boost::filesystem::path dir_path(one_path);
  return dir_path.parent_path().string();
}
std::string CFileUtils::JoinPath(const std::string &path1, const std::string &path2) {
  boost::filesystem::path path_1(path1), path_2(path2);
  return (path_1 / path_2).string();
}
bool CFileUtils::IsFileExist(const std::string &file) {
  return !boost::filesystem::is_directory(file) && boost::filesystem::exists(file);
}
bool CFileUtils::Exist(const std::string &one_path) { return boost::filesystem::exists(one_path); }

bool CFileUtils::DeleteFile(const std::string &file) {
  if (!IsFileExist(file)) return false;
  return boost::filesystem::remove(file);
}

void StringSplit(const std::string &str_in, const std::string &spliter, std::vector<std::string> &out) {
  out.clear();

  if (str_in.empty() || spliter.empty()) return;
  size_t pre_pos = 0, cur_pos = 0;
  while (cur_pos != str_in.npos) {
    cur_pos = str_in.find(spliter, pre_pos);
    if (cur_pos != str_in.npos) {
      std::string sub_str = str_in.substr(pre_pos, cur_pos);
      if (sub_str.size() > 0) out.emplace_back(sub_str.c_str());
      pre_pos = cur_pos + spliter.size();
    }
  }

  if (pre_pos < str_in.size()) out.emplace_back(str_in.substr(pre_pos).c_str());
}

// time point, for debug
void CElapsedTime::MarkStartTimePoint() { g_start_t = std::chrono::high_resolution_clock::now(); }

void CElapsedTime::MarkEndTimePoint() { g_end_t = std::chrono::high_resolution_clock::now(); }

float CElapsedTime::PrintElapsedTime(const std::string &prefix_string) {
  float elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(g_end_t - g_start_t).count() / 1000.0;
  LOG(INFO) << prefix_string << ", time elapsed:" << elapsed_time << " milliseconds.\n";

  return elapsed_time;
}

CElapsedTime::CElapsedTime(const std::string &prefix_string) {
  mPrefixString = prefix_string;
  MarkStartTimePoint();
}

CElapsedTime::~CElapsedTime() {
  MarkEndTimePoint();
  PrintElapsedTime(mPrefixString);
}

}  // namespace eval
