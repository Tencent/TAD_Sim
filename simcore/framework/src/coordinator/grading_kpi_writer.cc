// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "grading_kpi_writer.h"

#include "boost/filesystem/path.hpp"
#include "glog/logging.h"
#include "tinyxml2.h"

#include "utils/constant.h"
#include "utils/json_helper.h"

using namespace tinyxml2;
using namespace tx_sim::impl;
using namespace tx_sim::utils;
namespace fs = boost::filesystem;

namespace tx_sim {
namespace coordinator {

//! @brief 构造函数
//! @details 初始化GradingKpiXmlWriter对象，设置输出目录
//! @param[in] output_directory 输出目录
GradingKpiXmlWriter::GradingKpiXmlWriter(const std::string& output_directory) : grading_kpi_dir_(output_directory) {
  LOG(INFO) << "grading KPI config file directory: " << grading_kpi_dir_;
}

//! @brief 函数名：WriteGradingKpiConfigFile
//! @details 函数功能：将评分KPI配置写入XML文件
//! @param[in] kpi_config 一个包含评分KPI配置信息的GradingKpiList对象
//! @return 一个字符串，表示输出的评分KPI XML文件名
std::string GradingKpiXmlWriter::WriteGradingKpiConfigFile(const GradingKpiList& kpi_config) {
  std::string output_file_path = GetGradingKpiFilePath(kpi_config.group_id);
  LOG(INFO) << "writing grading KPI file to " << output_file_path;

  tinyxml2::XMLDocument doc;
  XMLElement* root = doc.NewElement("Grading");
  root->SetAttribute("version", "1.0");
  doc.InsertFirstChild(root);

  Json::Value jsonDocRoot;
  Json::Value& kpi_node = jsonDocRoot["kpi"];
  kpi_node = Json::arrayValue;  // to explicitly construct a empty [] when there's no module configs.

  const std::vector<GradingKpi>& kpis = kpi_config.kpis;
  for (const GradingKpi& kpi : kpis) {
    Json::Value& arrayNode = kpi_node.append(Json::Value());
    kpi.Encode(arrayNode);
    arrayNode.removeMember("id");
  }
  WriteJsonDocToFile(jsonDocRoot, output_file_path);
  LOG(INFO) << "writing grading KPI file done.";

  return fs::path(output_file_path).filename().string();
}

//! @brief 函数名：GetGradingKpiFilePath
//! @details 函数功能：获取评分KPI文件路径
//! @param[in] group_id 一个整数，表示评分KPI文件的组ID
//! @return 一个字符串，表示评分KPI文件路径
std::string GradingKpiXmlWriter::GetGradingKpiFilePath(int64_t group_id) {
  fs::path p(grading_kpi_dir_);
  p /= (kGradingKpiFilePrefix + std::to_string(group_id) + ".json");
  return p.string();
}

}  // namespace coordinator
}  // namespace tx_sim
