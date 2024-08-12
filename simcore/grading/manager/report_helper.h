// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <fstream>
#include <thread>
#include <vector>

#include "boost/interprocess/managed_shared_memory.hpp"
#include "boost/interprocess/sync/named_mutex.hpp"
#include "google/protobuf/util/json_util.h"
#include "grading_kpi.pb.h"
#include "grading_report.pb.h"
#include "scene.pb.h"
#include "utils/eval_utils.h"

#ifdef _USE_BOOST_UUID_ENABLED_
#  include "boost/uuid/uuid.hpp"
#  include "boost/uuid/uuid_generators.hpp"
#  include "boost/uuid/uuid_io.hpp"

#else
#  include <random>
#endif

namespace eval {

/**
 * @brief generate a UUID
 *
 * @return std::string
 */
std::string UUID();

/**
 * @brief "报告辅助类", report manager, manage all report behaviors.
 */
class ReportHelper {
 public:
  // set report info
  static void SetReportInfo(sim_msg::TestReport &report, const std::string &scene_name, const std::string &scene_desc,
                            const std::string &scene_id, const std::string &scene_version,
                            const std::string &date_version);

  // set case info, case can be used as one kpi
  static void SetCaseInfo(sim_msg::TestReport::Case &case_, const EvalCfg &eval_cfg);

  // set case info, case can be used as one kpi
  static void SetCaseInfo(sim_msg::TestReport::Case &case_, const sim_msg::GradingKpiGroup_GradingKpi &kpi);

  // set start time of test
  static void SetStartSecond(sim_msg::TestReport &report, double second);

  // set end time of test
  static void SetEndSecond(sim_msg::TestReport &report, double second);

  // set total mileage
  static void SetTotalMileage(sim_msg::TestReport &report, double km);

  // set case test result info
  static void SetTestResult(sim_msg::TestReport_TestResult &result, const sim_msg::TestReport_TestState &state,
                            const std::string &reason, double score = 0.0, double grade = 0.0);

  // post process test report
  static void ReportPostProcess(sim_msg::TestReport &report, bool eval_pass, const ModuleConfig &module_cfg,
                                const sim_msg::Scene &scene, const std::string &reason, double score = 0.0,
                                double grade = 0.0);

  /**
   * @brief calculate score of one case
   * @param case_: test case
   * @param usingPassCondition: calculate score by passcondition if true, otherwise using threshold score map defined in
   * grading_kpi.proto
   */
  static void CalScoreOfCase(const sim_msg::GradingKpiGroup_GradingKpi &kpi, sim_msg::TestReport::Case &case_,
                             bool usingPassCondition = true);

  /**
   * @brief look up score from ScoreMap1d
   * return INT32_MIN if fail
   */
  static double lookupScoreFromMap1d(const sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D &scoreMap,
                                     double u);

  /**
   * @brief Get the previous report file of previous ego's grading
   * @param[out] previous_report: previous ego's grading report
   * @param[in] report_dir: directory of report files
   * @param[in] file_name: file name of report
   * @return sim_msg::TestReport_TestState. whether previous grading is pass or not. If return
   * TestReport_TestState_SKIPPED, it means failed.
   */
  static sim_msg::TestReport_TestState GetPreviousReport(sim_msg::TestReport *previous_report,
                                                         const std::string &report_dir, const std::string &file_name);

  /**
   * @brief Merge previous report with current report
   * @param[in] previous_report: previous ego's grading report
   * @param[out] current_report: current ego's grading report
   */
  static void MergeReport(sim_msg::TestReport &&previous_report, sim_msg::TestReport &current_report);

  /**
   * @brief dump json to file
   * @param[in] report: grading report message unique ptr
   * @param[in] report_dir: the path to save the report file
   * @param[in] file_name: report file name
   * @param[in] report_mutex: mutex pointer to lock report file
   * @param[in] sync: whether to use another thread to dump json to file. true for not use, false for use
   * @param[in] pre_result: previous grading result
   */
  static void DumpReportJson2File(std::unique_ptr<sim_msg::TestReport> report, const std::string &report_dir,
                                  const std::string &file_name,
                                  std::unique_ptr<boost::interprocess::named_mutex> report_mutex, bool sync = true,
                                  sim_msg::TestReport_TestState pre_result = sim_msg::TestReport_TestState_UN_DEFINED);

  // dump json to file thread func, parameter pass by value
  static void DumpJsonFunc(const std::string &json_path, std::unique_ptr<sim_msg::TestReport> report,
                           std::unique_ptr<boost::interprocess::named_mutex> report_mutex);

  // set pair data
  static void SetPairData(sim_msg::TestReport_PairData &pair_, const std::string &key_, const std::string &value_);

 public:
  // add case to ego_case
  static void AddCase2EgoCase(sim_msg::TestReport_EgoCase &ego_case, const sim_msg::TestReport::Case &case_);
  // add step to case
  static void AddStep2Case(sim_msg::TestReport_Case &case_, const sim_msg::TestReport_Step &step);
  // set step info
  static void AddAttach2Step(sim_msg::TestReport::Step &step, const sim_msg::TestReport::Attach &attach);

  // add pair data to attach
  static void AddPairData2Attach(sim_msg::TestReport_Attach &attach, const sim_msg::TestReport_PairData &pair_data);
  // add xy-plot to attach
  static void AddXYPlot2Attach(sim_msg::TestReport_Attach &attach, const sim_msg::TestReport_XYPlot &xy_plot);
  // add xlsx sheet to attach
  static void AddXLSXSheet2Attach(sim_msg::TestReport_Attach &attach, const sim_msg::TestReport_XlsxSheet &sheet);
  // add pair to attach
  static void AddPair2Attach(sim_msg::TestReport_Attach &attach, const sim_msg::TestReport_PairData &pair_);
  // add xlsx sheet data to xlsx sheet
  static void AddSheetData2XLSX(sim_msg::TestReport_XlsxSheet &sheet,
                                const sim_msg::TestReport_XlsxSheet_SheetData &sheet_data);
  // add xy-plot to meta
  static void AddXYPlot2Meta(sim_msg::TestReport_EgoMeta &ego_meta, const sim_msg::TestReport_XYPlot &xy_plot);
  // add ego_meta to report
  static void AddEgoMeta2Report(sim_msg::TestReport &report, const sim_msg::TestReport_EgoMeta &ego_meta);
  // add ego_case to report
  static void AddEgoCase2Report(sim_msg::TestReport &report, const sim_msg::TestReport_EgoCase &ego_case);

 public:
  // config xy-plot info
  static void ConfigXYPlot(sim_msg::TestReport_XYPlot &xy_plot, const std::string &name, const std::string &desc,
                           const std::string &x_name, const std::string &x_unit,
                           const std::vector<std::string> &y_names, const std::vector<std::string> &y_units,
                           size_t y_axis_num = 1);

  /**
   * @brief Set Threshold after ConfigXYPlot
   * @param    xy_plot         TestReport_XYPlot
   * @param    upper_desc      the desc of upper threshold.
   * @param    upper_space     the IntervalSpace of upper threshold. 0:up, 1:down
   * @param    upper_type      the IntervalType of upper threshold. 0:open, 1:close
   * @param    upper_value     the value of upper threshold.
   * @param    lower_desc      the desc of lower threshold.
   * @param    lower_space     the IntervalSpace of lower threshold. 0:up, 1:down
   * @param    lower_type      the IntervalType of lower threshold. 0:open, 1:close
   * @param    lower_value     the value of lower threshold.
   * @param    y_axis_id       the id  of y axis. default 0
   */
  static void ConfigXYPlotThreshold(sim_msg::TestReport_XYPlot &xy_plot, const std::string &upper_desc = "",
                                    size_t upper_space = 0, size_t upper_type = 1, double upper_value = INT32_MAX,
                                    const std::string &lower_desc = "", size_t lower_space = 1, size_t lower_type = 1,
                                    double lower_value = INT32_MIN, size_t y_axis_id = 0);

  /**
   * @brief [Optional] Clear threshold value if threshold is not constants
   * @param xy_plot         TestReport_XYPlot
   * @param y_axis_ids      y_axis id list to clear
   * @param upper_clear     whether clear upper threshold
   * @param lower_clear     whether clear lower threshold
   */
  static void ConfigXYPlotThresholdClearValue(sim_msg::TestReport_XYPlot &xy_plot,
                                              const std::vector<size_t> &y_axis_ids, bool upper_clear,
                                              bool lower_clear);
  // config xlsx sheet info
  static void ConfigXLSXSheet(sim_msg::TestReport_XlsxSheet &sheet, const std::string &sheet_name,
                              const std::string &desc = "",
                              const sim_msg::TestReport_XlsxSheet_SheetDataType &sheet_type =
                                  sim_msg::TestReport_XlsxSheet_SheetDataType_Column_Sheet);

  // config xlsx sheet data
  static void ConfigSheetData(sim_msg::TestReport_XlsxSheet_SheetData &sheet_data, const std::string &header);
  // config pair data
  static void ConfigPairData(sim_msg::TestReport_PairData &pair_);

 public:
  // calculate variance of an axis data
  static double CalVariance(const sim_msg::TestReport_Axis &axis);

  // calculate average value of an axis data
  static double CalAverage(const sim_msg::TestReport_Axis &axis);

 private:
  // calculate score of one case by pass condition
  static void CalScoreOfCase_ByPassCondition(const sim_msg::GradingKpiGroup_GradingKpi &kpi,
                                             sim_msg::TestReport::Case &case_);

  // calculate score of one case by threshold
  static void CalScoreOfCase_ByThreshold(const sim_msg::GradingKpiGroup_GradingKpi &kpi,
                                         sim_msg::TestReport::Case &case_);
};
}  // namespace eval
