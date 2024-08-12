// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "config.h"

namespace tx_sim {
namespace coordinator {

//! @brief 类名：IGradingKpiWriter
//! @details 类功能：定义了一个接口，用于写入评分关键性能指标（KPI）配置文件
class IGradingKpiWriter {
 public:
  // returns the output grading xml file name.
  //! @brief 函数名：WriteGradingKpiConfigFile
  //! @details 函数功能：将评分KPI配置写入XML文件
  //! @param[in] kpi_config 一个包含评分KPI配置信息的GradingKpiList对象
  //! @return 一个字符串，表示输出的评分KPI XML文件名
  virtual std::string WriteGradingKpiConfigFile(const GradingKpiList& kpi_config) = 0;

  //! @brief 函数名：GetGradingKpiFilePath
  //! @details 函数功能：获取评分KPI文件路径
  //! @param[in] group_id 一个整数，表示评分KPI文件的组ID
  //! @return 一个字符串，表示评分KPI文件路径
  virtual std::string GetGradingKpiFilePath(int64_t group_id) = 0;
};

//! @brief 类名：GradingKpiXmlWriter
//! @details 类功能：实现了IGradingKpiWriter接口，用于将评分KPI配置写入XML文件
class GradingKpiXmlWriter final : public IGradingKpiWriter {
 public:
  //! @brief 构造函数
  //! @details 初始化GradingKpiXmlWriter对象，设置输出目录
  //! @param[in] output_directory 输出目录
  GradingKpiXmlWriter(const std::string& output_directory);

  //! @brief 函数名：WriteGradingKpiConfigFile
  //! @details 函数功能：将评分KPI配置写入XML文件
  //! @param[in] kpi_config 一个包含评分KPI配置信息的GradingKpiList对象
  //! @return 一个字符串，表示输出的评分KPI XML文件名
  std::string WriteGradingKpiConfigFile(const GradingKpiList& kpi_config) override;

  //! @brief 函数名：GetGradingKpiFilePath
  //! @details 函数功能：获取评分KPI文件路径
  //! @param[in] group_id 一个整数，表示评分KPI文件的组ID
  //! @return 一个字符串，表示评分KPI文件路径
  std::string GetGradingKpiFilePath(int64_t group_id) override;

 private:
  //! @brief 成员变量：grading_kpi_dir_
  //! @details 存储输出目录
  const std::string grading_kpi_dir_;
};

}  // namespace coordinator
}  // namespace tx_sim
