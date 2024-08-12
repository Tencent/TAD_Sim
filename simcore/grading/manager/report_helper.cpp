// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "report_helper.h"
#include "utils/proto_helper.h"

namespace eval {

std::string UUID() {
#ifdef _USE_BOOST_UUID_ENABLED_
  boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
  return boost::uuids::to_string(a_uuid);
#else
  std::default_random_engine engine;
  std::uniform_int_distribution<unsigned> distribution(0, 9);

  char fake_12[13];
  std::string uuid = "550e8400-e29b-41d4-a716-";

  for (auto i = 0; i < 12; ++i) {
    fake_12[i] = static_cast<char>('0' + distribution(engine));
  }
  fake_12[12] = '\0';

  return uuid + fake_12;
#endif
}

// set report info
void ReportHelper::SetReportInfo(sim_msg::TestReport &report, const std::string &scene_name,
                                 const std::string &scene_desc, const std::string &scene_id,
                                 const std::string &scene_version, const std::string &date_version) {
  report.mutable_report()->mutable_info()->set_report_id(UUID());
  report.mutable_report()->mutable_info()->set_scene_name(scene_name);
  report.mutable_report()->mutable_info()->set_scene_desc(scene_desc);
  report.mutable_report()->mutable_info()->set_scene_id(scene_id);
  // set report version
  report.mutable_report()->mutable_info()->set_report_version("2.1");

  sim_msg::TestReport_PairData *scene_version_ptr = report.mutable_report()->mutable_info()->add_additionals();
  scene_version_ptr->set_key("scene_version");
  scene_version_ptr->set_value(scene_version.c_str());

  sim_msg::TestReport_PairData *date_version_ptr = report.mutable_report()->mutable_info()->add_additionals();
  date_version_ptr->set_key("date_version");
  date_version_ptr->set_value(date_version.c_str());
}

// set case info, case can be used as one kpi
void ReportHelper::SetCaseInfo(sim_msg::TestReport::Case &case_, const EvalCfg &eval_cfg) {
  std::string indicator_name;
  auto indicator_pair = eval_cfg._pairs.find("IndicatorName");
  if (indicator_pair != eval_cfg._pairs.end()) {
    indicator_name = indicator_pair->second;
  }
  if (indicator_name.size() > 0)
    case_.mutable_info()->set_name((indicator_name).c_str());
  else
    case_.mutable_info()->set_name((eval_cfg._kpi_name).c_str());
  case_.mutable_info()->set_desc(eval_cfg._definition_value);
  case_.mutable_info()->mutable_category()->set_level1(eval_cfg._algorithm_name.c_str());
  case_.mutable_info()->mutable_category()->set_level2(eval_cfg._category_value.c_str());
  case_.mutable_info()->set_thresh(eval_cfg._thresh_value);
}

// set case info, case can be used as one kpi
void ReportHelper::SetCaseInfo(sim_msg::TestReport::Case &case_, const sim_msg::GradingKpiGroup_GradingKpi &kpi) {
  case_.mutable_info()->set_name((kpi.parameters().indicatorname()).c_str());
  case_.mutable_info()->set_desc(kpi.parameters().definition().c_str());

  case_.mutable_info()->mutable_category()->set_level1(kpi.category().features().at(0));
  std::string labels;
  for (const auto &label : kpi.category().labels()) {
    labels = label + ";" + labels;
  }
  if (labels.size() > 0) {
    labels.pop_back();
  }
  case_.mutable_info()->mutable_category()->set_level2(labels.c_str());

  // grading_kpi
  case_.mutable_info()->mutable_grading_kpi()->CopyFrom(kpi);

  // thresholdInfo
  auto thresholdInfoPtr = case_.mutable_info()->add_additionals();
  thresholdInfoPtr->set_key("thresholdInfo");
  std::string thresholdInfoStr;
  if (kpi.parameters().thresholds_size() > 0) {
    double val = 0.0;
    try {
      val = std::atof(kpi.parameters().thresholds().at(0).threshold().c_str());
    } catch (const std::exception &e) {
      val = 0.0;
      LOG_ERROR << "fail to convert first threshold to double in " << kpi.name() << e.what() << "\n";
    }
    case_.mutable_info()->set_thresh(val);

    for (const auto &threshold : kpi.parameters().thresholds()) {
      std::string thresholdValue = threshold.threshold();
      if (threshold.thresholdtype() == sim_msg::GradingKpiGroup_ThresholdType_ThresholdType_BOOL)
        thresholdValue = threshold.threshold() == "0" ? "false" : "true";
      std::string thresholdunit = threshold.thresholdunit() == "N/A" ? "" : threshold.thresholdunit();
      thresholdInfoStr += (threshold.thresholdname() + ":" + thresholdValue + thresholdunit + ";");
    }
  }
  if (thresholdInfoStr.size() > 0) {
    thresholdInfoStr.pop_back();
  }
  thresholdInfoPtr->set_value(thresholdInfoStr);

  // passCondition
  auto passConditionPtr = case_.mutable_info()->add_additionals();
  passConditionPtr->set_key("passCondition");
  std::string relation = "\\u5c0f\\u4e8e";  // 小于.
  if (kpi.passcondition().relation() == ">") {
    relation = "\\u5927\\u4e8e";  // 大于.
  } else if (kpi.passcondition().relation() == "<=") {
    relation = "\\u5c0f\\u4e8e\\u7b49\\u4e8e";  // 小于等于.
  } else if (kpi.passcondition().relation() == ">=") {
    relation = "\\u5927\\u4e8e\\u7b49\\u4e8e";  // 大于等于.
  }

  std::string passDesc =
      "\\u68c0\\u6d4b\\u6b21\\u6570" + relation + std::to_string(kpi.passcondition().value()) + "\\u6b21";  // 次.
  passConditionPtr->set_value(passDesc);

  // finishCondition
  auto finishConditionPtr = case_.mutable_info()->add_additionals();
  finishConditionPtr->set_key("finishCondition");
  finishConditionPtr->set_value(std::to_string(kpi.finishcondition().value()));
}

// set start time of test
void ReportHelper::SetStartSecond(sim_msg::TestReport &report, double second) {
  report.mutable_report()->mutable_info()->mutable_date()->set_start_second(second);
}

// set end time of test
void ReportHelper::SetEndSecond(sim_msg::TestReport &report, double second) {
  report.mutable_report()->mutable_info()->mutable_date()->set_end_second(second);
  double duration = second - report.report().info().date().start_second();
  report.mutable_report()->mutable_info()->mutable_date()->set_duration_second(duration);
}

// set total mileage
void ReportHelper::SetTotalMileage(sim_msg::TestReport &report, double km) {
  report.mutable_report()->mutable_info()->set_mileage(km);
}

// set case test result info
void ReportHelper::SetTestResult(sim_msg::TestReport_TestResult &result, const sim_msg::TestReport_TestState &state,
                                 const std::string &reason, double score, double grade) {
  result.set_state(state);
  result.set_reason(reason);
  result.set_score(score);
  result.set_grade(grade);
}

// post process test report
void ReportHelper::ReportPostProcess(sim_msg::TestReport &report, bool eval_pass, const ModuleConfig &module_cfg,
                                     const sim_msg::Scene &scene, const std::string &reason, double score,
                                     double grade) {
  auto summary = report.mutable_report()->mutable_summary();
  if (report.report().ego_cases_size() != 1) {
    VLOG_0 << "the size of report().ego_cases_size() is " << report.report().ego_cases_size()
           << ", which we expect is 1.\n";
    return;
  }

  // add ego module config
  {
    sim_msg::TestReport::XlsxSheet *module_cfg_sheet = report.mutable_report()->mutable_info()->add_module_cfg();
    ConfigXLSXSheet(*module_cfg_sheet, getMyEgoGroupName(), getMyEgoGroupName() + "-配置列表",
                    sim_msg::TestReport_XlsxSheet_SheetDataType_Column_Sheet);

    sim_msg::TestReport_XlsxSheet_SheetData kpi_name_col, kpi_category_col, kpi_elf_col;
    sim_msg::TestReport_XlsxSheet_SheetData avg_steptime_col, expected_avg_steptime_col, steptime_col, dutycycle_col,
        overrun_col;

    // set xlsx sheet data
    {
      kpi_name_col.set_header("算法名称");
      kpi_category_col.set_header("算法分类");

      avg_steptime_col.set_header("平均单步耗时(s)");
      expected_avg_steptime_col.set_header("期望平均单步耗时(s)");
      steptime_col.set_header("算法步长(s)");
      dutycycle_col.set_header("平均单步耗时占比(%)");
      overrun_col.set_header("超时次数");

      kpi_elf_col.set_header("算法文件");
      for (auto i = 0; i < module_cfg.m_module_names.size(); ++i) {
        std::string module_name = module_cfg.m_module_names.at(i);
        // 若模块名小于主车名，忽略
        if (module_name.size() < getMyEgoGroupName().size()) continue;
        // 若模块名的前缀不是主车名，忽略
        if (module_name.compare(0, getMyEgoGroupName().size(), getMyEgoGroupName()) != 0) continue;
        kpi_name_col.add_data()->assign(module_name.c_str());
        kpi_category_col.add_data()->assign(std::to_string(module_cfg.m_module_types.at(i)));
        kpi_elf_col.add_data()->assign(module_cfg.m_module_paths.at(i).c_str());
        avg_steptime_col.add_data()->assign(module_cfg.m_avg_steptime.find(module_name) ==
                                                    module_cfg.m_avg_steptime.end()
                                                ? "-1.000000"
                                                : module_cfg.m_avg_steptime.at(module_name).c_str());
        expected_avg_steptime_col.add_data()->assign(module_cfg.m_expected_avg_steptime.find(module_name) ==
                                                             module_cfg.m_expected_avg_steptime.end()
                                                         ? "-1.000000"
                                                         : module_cfg.m_expected_avg_steptime.at(module_name).c_str());
        steptime_col.add_data()->assign(module_cfg.m_steptime.find(module_name) == module_cfg.m_steptime.end()
                                            ? "-1.000000"
                                            : module_cfg.m_steptime.at(module_name).c_str());
        dutycycle_col.add_data()->assign(module_cfg.m_dutycycle.find(module_name) == module_cfg.m_dutycycle.end()
                                             ? "-1.000000"
                                             : module_cfg.m_dutycycle.at(module_name).c_str());
        overrun_col.add_data()->assign(module_cfg.m_overrun_count.find(module_name) == module_cfg.m_overrun_count.end()
                                           ? "-1.000000"
                                           : module_cfg.m_overrun_count.at(module_name).c_str());
      }
    }

    AddSheetData2XLSX(*module_cfg_sheet, kpi_name_col);
    AddSheetData2XLSX(*module_cfg_sheet, kpi_category_col);

    AddSheetData2XLSX(*module_cfg_sheet, avg_steptime_col);
    AddSheetData2XLSX(*module_cfg_sheet, expected_avg_steptime_col);
    AddSheetData2XLSX(*module_cfg_sheet, steptime_col);
    AddSheetData2XLSX(*module_cfg_sheet, dutycycle_col);
    AddSheetData2XLSX(*module_cfg_sheet, overrun_col);

    AddSheetData2XLSX(*module_cfg_sheet, kpi_elf_col);
  }

  // add global module config
  {
    sim_msg::TestReport::XlsxSheet *module_cfg_sheet = report.mutable_report()->mutable_info()->add_module_cfg();
    ConfigXLSXSheet(*module_cfg_sheet, "global", "global-配置列表",
                    sim_msg::TestReport_XlsxSheet_SheetDataType_Column_Sheet);

    sim_msg::TestReport_XlsxSheet_SheetData kpi_name_col, kpi_category_col, kpi_elf_col;
    sim_msg::TestReport_XlsxSheet_SheetData avg_steptime_col, expected_avg_steptime_col, steptime_col, dutycycle_col,
        overrun_col;

    // set xlsx sheet data
    {
      kpi_name_col.set_header("算法名称");
      kpi_category_col.set_header("算法分类");

      avg_steptime_col.set_header("平均单步耗时(s)");
      expected_avg_steptime_col.set_header("期望平均单步耗时(s)");
      steptime_col.set_header("算法步长(s)");
      dutycycle_col.set_header("平均单步耗时占比(%)");
      overrun_col.set_header("超时次数");

      kpi_elf_col.set_header("算法文件");
      for (auto i = 0; i < module_cfg.m_module_names.size(); ++i) {
        std::string module_name = module_cfg.m_module_names.at(i);
        if ("Ego" == module_name.substr(0, 3)) continue;
        kpi_name_col.add_data()->assign(module_cfg.m_module_names.at(i).c_str());
        kpi_category_col.add_data()->assign(std::to_string(module_cfg.m_module_types.at(i)));
        kpi_elf_col.add_data()->assign(module_cfg.m_module_paths.at(i).c_str());
        avg_steptime_col.add_data()->assign(module_cfg.m_avg_steptime.find(module_name) ==
                                                    module_cfg.m_avg_steptime.end()
                                                ? "-1.000000"
                                                : module_cfg.m_avg_steptime.at(module_name).c_str());
        expected_avg_steptime_col.add_data()->assign(module_cfg.m_expected_avg_steptime.find(module_name) ==
                                                             module_cfg.m_expected_avg_steptime.end()
                                                         ? "-1.000000"
                                                         : module_cfg.m_expected_avg_steptime.at(module_name).c_str());
        steptime_col.add_data()->assign(module_cfg.m_steptime.find(module_name) == module_cfg.m_steptime.end()
                                            ? "-1.000000"
                                            : module_cfg.m_steptime.at(module_name).c_str());
        dutycycle_col.add_data()->assign(module_cfg.m_dutycycle.find(module_name) == module_cfg.m_dutycycle.end()
                                             ? "-1.000000"
                                             : module_cfg.m_dutycycle.at(module_name).c_str());
        overrun_col.add_data()->assign(module_cfg.m_overrun_count.find(module_name) == module_cfg.m_overrun_count.end()
                                           ? "-1.000000"
                                           : module_cfg.m_overrun_count.at(module_name).c_str());
      }
    }

    AddSheetData2XLSX(*module_cfg_sheet, kpi_name_col);
    AddSheetData2XLSX(*module_cfg_sheet, kpi_category_col);

    AddSheetData2XLSX(*module_cfg_sheet, avg_steptime_col);
    AddSheetData2XLSX(*module_cfg_sheet, expected_avg_steptime_col);
    AddSheetData2XLSX(*module_cfg_sheet, steptime_col);
    AddSheetData2XLSX(*module_cfg_sheet, dutycycle_col);
    AddSheetData2XLSX(*module_cfg_sheet, overrun_col);

    AddSheetData2XLSX(*module_cfg_sheet, kpi_elf_col);
  }

  // add ego config info
  {
    sim_msg::TestReport::XlsxSheet *ego_cfg_sheet = report.mutable_report()->mutable_info()->add_ego_cfg();
    ConfigXLSXSheet(*ego_cfg_sheet, getMyEgoGroupName(), getMyEgoGroupName() + "-主车信息",
                    sim_msg::TestReport_XlsxSheet_SheetDataType_Column_Sheet);

    sim_msg::TestReport_XlsxSheet_SheetData config_name, config_value;
    config_name.set_header("名称");
    config_value.set_header("值");

    for (int i = 0; i < scene.egos_size(); ++i) {
      if (getMyEgoGroupName() != scene.egos(i).group()) continue;
      sim_msg::Ego ego_cfg = scene.egos(i);
      config_name.add_data("主车ID");
      config_value.add_data(getMyEgoGroupName());

      config_name.add_data("主车模型");
      config_value.add_data(ego_cfg.name());

      config_name.add_data("主车控制器名称");
      config_value.add_data(ego_cfg.initial().assign().controller_name());
    }

    AddSheetData2XLSX(*ego_cfg_sheet, config_name);
    AddSheetData2XLSX(*ego_cfg_sheet, config_value);
  }

  // fill summary info
  {
    int case_size = report.report().ego_cases(0).cases_size();
    int undefined = 0, pass = 0, skipped = 0, failed = 0;

    for (auto i = 0; i < report.report().ego_cases(0).cases_size(); ++i) {
      const sim_msg::TestReport::Case &case_ = report.report().ego_cases(0).cases().at(i);
      if (case_.info().result().state() == sim_msg::TestReport_TestState_UN_DEFINED) undefined++;
      if (case_.info().result().state() == sim_msg::TestReport_TestState_PASS) pass++;
      if (case_.info().result().state() == sim_msg::TestReport_TestState_SKIPPED) skipped++;
      if (case_.info().result().state() == sim_msg::TestReport_TestState_FAIL) failed++;
    }

    summary->set_case_size(case_size);
    summary->set_case_undefined_size(undefined);
    summary->set_case_pass_size(pass);
    summary->set_case_skipped_size(skipped);
    summary->set_case_fail_size(failed);
  }

  // add kpi detail table
  {
    sim_msg::TestReport::XlsxSheet *kpi_sheet = summary->add_case_summary();
    ConfigXLSXSheet(*kpi_sheet, getMyEgoGroupName(), getMyEgoGroupName() + "-指标列表",
                    sim_msg::TestReport_XlsxSheet_SheetDataType_Column_Sheet);

    sim_msg::TestReport_XlsxSheet_SheetData kpi_name_col, kpi_res_col, kpi_category_col, kpi_thresh_col, kpi_detect_col;

    // set xlsx sheet data
    {
      kpi_name_col.set_header("指标名称");
      kpi_res_col.set_header("评测结果");
      // kpi_thresh_col.set_header("\\u9608\\u503c");
      kpi_thresh_col.set_header("\\u8bc4\\u6d4b\\u9608\\u503c");    // 评测阈值.
      kpi_category_col.set_header("\\u5206\\u7c7b\\u6807\\u7b7e");  // 分类标签.
      kpi_detect_col.set_header(
          "\\u5b9e\\u9645\\u503c/\\u901a\\u8fc7\\u6761\\u4ef6\\u9608\\u503c");  // 实际值/通过条件阈值.
      for (auto i = 0; i < report.report().ego_cases(0).cases_size(); ++i) {
        const sim_msg::TestReport_Case &case_ = report.report().ego_cases(0).cases().at(i);
        std::string category = case_.info().category().level1() + "-" + case_.info().category().level2();

        kpi_name_col.add_data()->assign((case_.info().name()).c_str());
        kpi_res_col.add_data()->assign(sim_msg::TestReport_TestState_Name(case_.info().result().state()).c_str());
        kpi_category_col.add_data()->assign(category.c_str());
        std::string threshold_info_str;
        for (const auto &pd : case_.info().additionals()) {
          if (pd.key() == "thresholdInfo") threshold_info_str = pd.value();
        }
        // kpi_thresh_col.add_data()->assign(std::to_string(case_.info().thresh()).c_str());
        kpi_thresh_col.add_data()->assign(threshold_info_str);
        std::string detect_str = std::to_string(case_.info().detected_count()) + "/" +
                                 std::to_string(case_.info().grading_kpi().passcondition().value()) + "\t\\u6b21";
        kpi_detect_col.add_data()->assign(detect_str);
      }
    }

    AddSheetData2XLSX(*kpi_sheet, kpi_name_col);
    AddSheetData2XLSX(*kpi_sheet, kpi_res_col);
    AddSheetData2XLSX(*kpi_sheet, kpi_thresh_col);
    AddSheetData2XLSX(*kpi_sheet, kpi_category_col);
    AddSheetData2XLSX(*kpi_sheet, kpi_detect_col);
  }

  // set report result info
  {
    sim_msg::TestReport_TestResult &result = *report.mutable_report()->mutable_info()->mutable_result();
    sim_msg::TestReport_TestState state =
        eval_pass ? sim_msg::TestReport_TestState_PASS : sim_msg::TestReport_TestState_FAIL;
    SetTestResult(result, state, reason, score, grade);
  }

  // add version
  {
    auto addition = report.mutable_report()->mutable_info()->add_additionals();
    SetPairData(*addition, "version", "2.0");
  }
}

sim_msg::TestReport_TestState ReportHelper::GetPreviousReport(sim_msg::TestReport *previous_report,
                                                              const std::string &report_dir,
                                                              const std::string &file_name) {
  previous_report->Clear();

  // don't know previous report is pass or fail, so we need prepare three options.
  std::string pre_report_path_pass = report_dir + "/P." + file_name;
  std::string pre_report_path_fail = report_dir + "/F." + file_name;
  std::string pre_report_path_unknown = report_dir + "/U." + file_name;
  std::string exist_report_path = "";

  // check which previous report is not empty
  sim_msg::TestReport_TestState pre_result = sim_msg::TestReport_TestState_SKIPPED;
  if (CFileUtils::IsFileExist(pre_report_path_fail)) {
    exist_report_path = pre_report_path_fail;
    pre_result = sim_msg::TestReport_TestState_FAIL;
  } else if (CFileUtils::IsFileExist(pre_report_path_pass)) {
    exist_report_path = pre_report_path_pass;
    pre_result = sim_msg::TestReport_TestState_PASS;
  } else if (CFileUtils::IsFileExist(pre_report_path_unknown)) {
    exist_report_path = pre_report_path_unknown;
    pre_result = sim_msg::TestReport_TestState_UN_DEFINED;
  } else {
    VLOG_0 << "previous report not found. It may be the first report.\n";
    VLOG_0 << "desired pre_report_path_pass: " << pre_report_path_pass << std::endl;
    VLOG_0 << "desired pre_report_path_fail: " << pre_report_path_fail << std::endl;
    VLOG_0 << "desired pre_report_path_unknown: " << pre_report_path_unknown << std::endl;
    pre_result = sim_msg::TestReport_TestState_SKIPPED;
  }
  VLOG_0 << "previous report path is " << exist_report_path << "\n";

  if (exist_report_path.empty()) return sim_msg::TestReport_TestState_SKIPPED;

  // load content from file
  std::string pre_report_content = eval::loadContentFromFile(exist_report_path);
  if (pre_report_content.size() == 0) {
    VLOG_0 << "previous report file exists but contains nothing.\n";
    return sim_msg::TestReport_TestState_SKIPPED;
  }

  // convert content to proto
  if (!eval::jsonToProto(pre_report_content, *previous_report)) {
    VLOG_0 << "fail to convert previous report content to sim_msg::TestReport. pre_report_content: "
           << pre_report_content << "\n";
    previous_report->Clear();
    return sim_msg::TestReport_TestState_SKIPPED;
  }

  return pre_result;
}

void ReportHelper::MergeReport(sim_msg::TestReport &&previous_report, sim_msg::TestReport &current_report) {
  CElapsedTime e1("MergeReport");
  // std::cout << "previous report: " << previous_report.Utf8DebugString() << std::endl;
  // std::cout << "current report: " << current_report.Utf8DebugString() << std::endl;

  // add mileage
  double pre_mileage = previous_report.report().info().mileage();
  double cur_mileage = current_report.report().info().mileage();
  previous_report.mutable_report()->mutable_info()->set_mileage(pre_mileage + cur_mileage);

  // add module config (only ego's module)
  previous_report.mutable_report()->mutable_info()->add_module_cfg()->CopyFrom(
      current_report.report().info().module_cfg(0));

  // add ego config
  previous_report.mutable_report()->mutable_info()->mutable_ego_cfg()->MergeFrom(
      current_report.report().info().ego_cfg());

  // merge grading result state
  sim_msg::TestReport_TestState pre_state = previous_report.report().info().result().state();
  sim_msg::TestReport_TestState cur_state = current_report.report().info().result().state();
  if (pre_state == sim_msg::TestReport_TestState_FAIL || cur_state == sim_msg::TestReport_TestState_FAIL) {
    previous_report.mutable_report()->mutable_info()->mutable_result()->set_state(sim_msg::TestReport_TestState_FAIL);
  } else {
    previous_report.mutable_report()->mutable_info()->mutable_result()->set_state(sim_msg::TestReport_TestState_PASS);
  }

  // add additional of current report to previous report
  previous_report.mutable_report()->mutable_info()->mutable_result()->mutable_additional()->MergeFrom(
      current_report.report().info().result().additional());

  // add report->summary->case_summary
  previous_report.mutable_report()->mutable_summary()->mutable_case_summary()->MergeFrom(
      current_report.report().summary().case_summary());
  // add current report->summary->case_size to previous
  int32_t pre_case_size = previous_report.report().summary().case_size();
  int32_t cur_case_size = current_report.report().summary().case_size();
  previous_report.mutable_report()->mutable_summary()->set_case_size(pre_case_size + cur_case_size);
  // add current report->summary->case_undefined_size to previous
  int32_t pre_case_undefined_size = previous_report.report().summary().case_undefined_size();
  int32_t cur_case_undefined_size = current_report.report().summary().case_undefined_size();
  previous_report.mutable_report()->mutable_summary()->set_case_undefined_size(pre_case_undefined_size +
                                                                               cur_case_undefined_size);
  // add current report->summary->case_pass_size to previous
  int32_t pre_case_pass_size = previous_report.report().summary().case_pass_size();
  int32_t cur_case_pass_size = current_report.report().summary().case_pass_size();
  previous_report.mutable_report()->mutable_summary()->set_case_pass_size(pre_case_pass_size + cur_case_pass_size);
  // add current report->summary->case_skipped_size to previous
  int32_t pre_case_skipped_size = previous_report.report().summary().case_skipped_size();
  int32_t cur_case_skipped_size = current_report.report().summary().case_skipped_size();
  previous_report.mutable_report()->mutable_summary()->set_case_skipped_size(pre_case_skipped_size +
                                                                             cur_case_skipped_size);
  // add current report->summary->case_fail_size to previous
  int32_t pre_case_fail_size = previous_report.report().summary().case_fail_size();
  int32_t cur_case_fail_size = current_report.report().summary().case_fail_size();
  previous_report.mutable_report()->mutable_summary()->set_case_fail_size(pre_case_fail_size + cur_case_fail_size);

  // merge report->ego_cases
  previous_report.mutable_report()->mutable_ego_cases()->MergeFrom(current_report.report().ego_cases());

  // merge meta
  previous_report.mutable_ego_meta()->MergeFrom(current_report.ego_meta());

  current_report = std::move(previous_report);
}

void ReportHelper::DumpReportJson2File(std::unique_ptr<sim_msg::TestReport> report, const std::string &report_dir,
                                       const std::string &file_name,
                                       std::unique_ptr<boost::interprocess::named_mutex> report_mutex, bool sync,
                                       sim_msg::TestReport_TestState pre_result) {
  if (report.get() == nullptr) {
    VLOG_0 << "report pointer is nullptr. save nothing.\n";
    return;
  }

  std::string report_prefix = "U.";

  if (report->report().info().result().state() == sim_msg::TestReport_TestState_PASS) report_prefix = "P.";
  if (report->report().info().result().state() == sim_msg::TestReport_TestState_FAIL ||
      pre_result == sim_msg::TestReport_TestState_FAIL) {
    report_prefix = "F.";
  }

  // if current report is fail and previous report is pass,
  // delete previous report file because the file name will be different
  if (report->report().info().result().state() == sim_msg::TestReport_TestState_FAIL &&
      pre_result == sim_msg::TestReport_TestState_PASS) {
    std::string file_to_delete = report_dir + "/P." + file_name;
    VLOG_0 << "need to delete previous report file. previous report file is " << file_to_delete << "\n";
    if (CFileUtils::DeleteFile(file_to_delete)) {
      VLOG_0 << "delete previous report file success.\n";
    } else {
      VLOG_0 << "delete previous report file failed.\n";
    }
  }

  // get json output full path
  std::string json_path = report_dir + "/" + report_prefix + file_name;
  VLOG_0 << "output json file path is " << json_path << "\n";

  // write to file sync or async
  if (sync) {
    VLOG_0 << "sync mode is true, use a function call to save report.\n";
    DumpJsonFunc(json_path, std::move(report), std::move(report_mutex));
  } else {
    VLOG_0 << "sync mode is false, use a detached thread to save report.\n";
    auto handle = std::thread(ReportHelper::DumpJsonFunc, json_path, std::move(report), std::move(report_mutex));
    handle.detach();
  }
}

// dump json to file thread func, parameter pass by value
void ReportHelper::DumpJsonFunc(const std::string &json_path, std::unique_ptr<sim_msg::TestReport> report,
                                std::unique_ptr<boost::interprocess::named_mutex> report_mutex) {
  if (report.get() == nullptr) {
    VLOG_0 << "report pointer is nullptr. save nothing.\n";
    return;
  }

  // dump protobuf to json
  std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

  std::string report_json;
  google::protobuf::util::JsonOptions json_opt;
  json_opt.add_whitespace = false;
  json_opt.always_print_primitive_fields = true;
  google::protobuf::util::MessageToJsonString(*(report.get()), &report_json, json_opt);

  std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
  double duration_s = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() * 0.001;
  VLOG_0 << "report protobuf message to json costs[second]:" << duration_s << "\n";

  // write to file
  t1 = std::chrono::high_resolution_clock::now();
  std::ofstream json_out_stream;
  json_out_stream.open(json_path, std::ios::out);
  if (json_out_stream.is_open()) {
    json_out_stream << report_json;
    json_out_stream.flush();
    json_out_stream.close();
  }

  // show report finished num
  VLOG_0 << "report_finished_num: " << report->report().info().ego_cfg_size() << std::endl;
  // unlock named_mutex
  report_mutex->unlock();
  // remove named_mutex according to report finished num
  if (report->report().info().ego_cfg_size() >= getTotalEgoNum()) {
    if (report->report().info().ego_cfg_size() > getTotalEgoNum()) {
      VLOG_0 << "warning: report finished num is larger than total ego number. report finished num: "
             << report->report().info().ego_cfg_size() << ", total ego number: " << getTotalEgoNum() << std::endl;
    }
    boost::interprocess::named_mutex::remove("grading_named_mutex");
    boost::interprocess::shared_memory_object::remove("grading_shared_memory");
    VLOG_0 << "grading_named_mutex and grading_shared_memory are removed." << std::endl;
  }

  t2 = std::chrono::high_resolution_clock::now();
  duration_s = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() * 0.001;
  VLOG_0 << "report dump to disk costs[second]:" << duration_s << "\n";
}

// set pair data
void ReportHelper::SetPairData(sim_msg::TestReport_PairData &pair_, const std::string &key_,
                               const std::string &value_) {
  pair_.set_key(key_);
  pair_.set_value(value_);
}

// add case to report
void ReportHelper::AddCase2EgoCase(sim_msg::TestReport_EgoCase &ego_case, const sim_msg::TestReport::Case &case_) {
  ego_case.add_cases()->CopyFrom(case_);
}
// add step to case
void ReportHelper::AddStep2Case(sim_msg::TestReport_Case &case_, const sim_msg::TestReport_Step &step) {
  case_.add_steps()->CopyFrom(step);
}
// set step info
void ReportHelper::AddAttach2Step(sim_msg::TestReport::Step &step, const sim_msg::TestReport::Attach &attach) {
  step.add_attach()->CopyFrom(attach);
}

// add pair data to attach
void ReportHelper::AddPairData2Attach(sim_msg::TestReport_Attach &attach,
                                      const sim_msg::TestReport_PairData &pair_data) {
  attach.add_pair_data()->CopyFrom(pair_data);
}
// add xy-plot to attach
void ReportHelper::AddXYPlot2Attach(sim_msg::TestReport_Attach &attach, const sim_msg::TestReport_XYPlot &xy_plot) {
  attach.add_xyplot()->CopyFrom(xy_plot);
}
// add xlsx sheet to attach
void ReportHelper::AddXLSXSheet2Attach(sim_msg::TestReport_Attach &attach, const sim_msg::TestReport_XlsxSheet &sheet) {
  attach.add_sheet_data()->CopyFrom(sheet);
}
// add pair to attach
void ReportHelper::AddPair2Attach(sim_msg::TestReport_Attach &attach, const sim_msg::TestReport_PairData &pair_) {
  attach.mutable_pair_data()->Add()->CopyFrom(pair_);
}
// add xlsx sheet data to xlsx sheet
void ReportHelper::AddSheetData2XLSX(sim_msg::TestReport_XlsxSheet &sheet,
                                     const sim_msg::TestReport_XlsxSheet_SheetData &sheet_data) {
  sheet.add_sheet_data()->CopyFrom(sheet_data);
}
// add xy-plot to meta
void ReportHelper::AddXYPlot2Meta(sim_msg::TestReport_EgoMeta &ego_meta, const sim_msg::TestReport_XYPlot &xy_plot) {
  ego_meta.add_meta()->CopyFrom(xy_plot);
}

// add ego_meta to report
void ReportHelper::AddEgoMeta2Report(sim_msg::TestReport &report, const sim_msg::TestReport_EgoMeta &ego_meta) {
  report.add_ego_meta()->CopyFrom(ego_meta);
}

// add ego_case to report
void ReportHelper::AddEgoCase2Report(sim_msg::TestReport &report, const sim_msg::TestReport_EgoCase &ego_case) {
  report.mutable_report()->add_ego_cases()->CopyFrom(ego_case);
}

// config xy-plot info
void ReportHelper::ConfigXYPlot(sim_msg::TestReport_XYPlot &xy_plot, const std::string &name, const std::string &desc,
                                const std::string &x_name, const std::string &x_unit,
                                const std::vector<std::string> &y_names, const std::vector<std::string> &y_units,
                                size_t y_axis_num) {
  xy_plot.Clear();
  xy_plot.set_name(name);
  xy_plot.set_description(desc);

  xy_plot.mutable_x_axis()->set_name(x_name);
  xy_plot.mutable_x_axis()->set_unit(x_unit);

  for (auto i = 0; i < y_axis_num; ++i) {
    auto y_axis = xy_plot.add_y_axis();
    y_axis->set_name(y_names.at(i));
    y_axis->set_unit(y_units.at(i));
    // add theshold
    sim_msg::TestReport_ThesholdInfo theshold;
    theshold.set_unit(y_units.at(i));
    theshold.add_value(INT32_MAX);
    y_axis->mutable_threshold_lower()->CopyFrom(theshold);
    y_axis->mutable_threshold_upper()->CopyFrom(theshold);
  }
  if (xy_plot.y_axis_size() == 0) {
    auto y_axis = xy_plot.add_y_axis();
    y_axis->set_name("default");
    y_axis->set_unit("null");
  }
}

/**
 * @brief Set Threshold after ConfigXYPlot
 * @param    xy_plot         TestReport_XYPlot
 * @param    upper_desc      the desc of upper threshold.
 * @param    upper_space     the IntervalSpace of upper threshold.
 * @param    upper_type      the IntervalType of upper threshold.
 * @param    upper_value     the value of upper threshold.
 * @param    lower_desc      the desc of lower threshold.
 * @param    lower_space     the IntervalSpace of lower threshold.
 * @param    lower_type      the IntervalType of lower threshold.
 * @param    lower_value     the value of lower threshold.
 * @param    y_axis_id       the id  of y axis. default 0
 */
void ReportHelper::ConfigXYPlotThreshold(sim_msg::TestReport_XYPlot &xy_plot, const std::string &upper_desc,
                                         size_t upper_space, size_t upper_type, double upper_value,
                                         const std::string &lower_desc, size_t lower_space, size_t lower_type,
                                         double lower_value, size_t y_axis_id) {
  if (y_axis_id >= xy_plot.y_axis_size()) {
    VLOG_0 << "[ERROR] XYPlot y_axis_size <= " << y_axis_id << "\n";
    return;
  }
  xy_plot.mutable_y_axis()->at(y_axis_id).mutable_threshold_upper()->set_desc(upper_desc);
  xy_plot.mutable_y_axis()->at(y_axis_id).mutable_threshold_upper()->set_space(
      sim_msg::TestReport_IntervalSpace(upper_space));
  xy_plot.mutable_y_axis()->at(y_axis_id).mutable_threshold_upper()->set_type(
      sim_msg::TestReport_IntervalType(upper_type));

  xy_plot.mutable_y_axis()->at(y_axis_id).mutable_threshold_upper()->clear_value();
  xy_plot.mutable_y_axis()->at(y_axis_id).mutable_threshold_upper()->add_value(upper_value);

  xy_plot.mutable_y_axis()->at(y_axis_id).mutable_threshold_lower()->set_desc(lower_desc);
  xy_plot.mutable_y_axis()->at(y_axis_id).mutable_threshold_lower()->set_space(
      sim_msg::TestReport_IntervalSpace(lower_space));
  xy_plot.mutable_y_axis()->at(y_axis_id).mutable_threshold_lower()->set_type(
      sim_msg::TestReport_IntervalType(lower_type));
  xy_plot.mutable_y_axis()->at(y_axis_id).mutable_threshold_lower()->clear_value();
  xy_plot.mutable_y_axis()->at(y_axis_id).mutable_threshold_lower()->add_value(lower_value);
}

/**
 * @brief [Optional] Clear threshold value if threshold is not constants
 * @param xy_plot         TestReport_XYPlot
 * @param y_axis_ids      y_axis id list to clear
 * @param upper_clear     whether clear upper threshold
 * @param lower_clear     whether clear lower threshold
 */
void ReportHelper::ConfigXYPlotThresholdClearValue(sim_msg::TestReport_XYPlot &xy_plot,
                                                   const std::vector<size_t> &y_axis_ids, bool upper_clear,
                                                   bool lower_clear) {
  for (auto id : y_axis_ids) {
    if (upper_clear) xy_plot.mutable_y_axis()->at(id).mutable_threshold_upper()->clear_value();
    if (lower_clear) xy_plot.mutable_y_axis()->at(id).mutable_threshold_lower()->clear_value();
  }
}
// config xlsx sheet info
void ReportHelper::ConfigXLSXSheet(sim_msg::TestReport_XlsxSheet &sheet, const std::string &sheet_name,
                                   const std::string &desc,
                                   const sim_msg::TestReport_XlsxSheet_SheetDataType &sheet_type) {
  sheet.Clear();
  sheet.set_name(sheet_name);
  sheet.set_description(desc);
  sheet.set_type(sheet_type);
}

// config xlsx sheet data
void ReportHelper::ConfigSheetData(sim_msg::TestReport_XlsxSheet_SheetData &sheet_data, const std::string &header) {
  sheet_data.Clear();
  sheet_data.set_header(header.c_str());
}

// config pair data
void ReportHelper::ConfigPairData(sim_msg::TestReport_PairData &pair_) { pair_.Clear(); }

// calculate variance of an axis data
double ReportHelper::CalVariance(const sim_msg::TestReport_Axis &axis) {
  double average = CalAverage(axis);
  double variance = 0.0;
  const int &count = axis.axis_data_size();

  if (count > 0) {
    for (auto i = 0; i < axis.axis_data_size(); ++i) {
      variance += std::pow(axis.axis_data(i) - average, 2);
    }
    variance /= count;
  }

  return variance;
}

// calculate average value of an axis data
double ReportHelper::CalAverage(const sim_msg::TestReport_Axis &axis) {
  double average = 0.0;
  const int &count = axis.axis_data_size();
  if (count > 0) {
    // calculate average value
    for (auto i = 0; i < axis.axis_data_size(); ++i) {
      average += axis.axis_data(i);
    }
    // calculate variance
    average /= count;
  }
  return average;
}

/**
 * @brief calculate score of one case
 * @param case_: test case
 * @param usingPassCondition: calculate score by passcondition if true, otherwise using threshold score map defined in
 * grading_kpi.proto
 */
void ReportHelper::CalScoreOfCase(const sim_msg::GradingKpiGroup_GradingKpi &kpi, sim_msg::TestReport::Case &case_,
                                  bool usingPassCondition) {
  if (usingPassCondition) {
    CalScoreOfCase_ByPassCondition(kpi, case_);
  } else {
    CalScoreOfCase_ByThreshold(kpi, case_);
  }
}

// lookup score from score map1d
double ReportHelper::lookupScoreFromMap1d(const sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D &scoreMap,
                                          double u) {
  // pre check
  if (scoreMap.u().size() < 2 || scoreMap.u().size() != scoreMap.y().size()) {
    return INT32_MIN;
  }

  // pre check
  double preU = INT32_MIN;
  for (const auto &u : scoreMap.u()) {
    if (u > preU) {
      preU = u;
    } else {
      LOG_ERROR << "u of scoreMap1d should be increase order:" << scoreMap.DebugString() << ".\n";
      return INT32_MIN;
    }
  }

  // do lookup
  int left_i = 0, right_i = scoreMap.y().size() - 1;

  // pre quit if u is out of boundry of scoreMap u axis
  if (u < scoreMap.u().at(left_i)) {
    return scoreMap.y().at(left_i);
  } else if (u > scoreMap.u().at(right_i)) {
    return scoreMap.y().at(right_i);
  }

  // binary search
  while (left_i + 1 < right_i) {
    int mid = (left_i + right_i) / 2;
    const auto &midU = scoreMap.u().at(mid);

    if (u == midU) {
      // bingo, here will quit while loop
      left_i = mid;
      right_i = left_i + 1;
    } else if (u > midU) {
      // mid is less than u, move left to mid
      left_i = mid;
    } else {
      // mid is bigger than u, move right to mid
      right_i = mid;
    }
  }

  const auto &u0 = scoreMap.u().at(left_i);
  const auto &u1 = scoreMap.u().at(right_i);
  const auto &y0 = scoreMap.y().at(left_i);
  const auto &y1 = scoreMap.y().at(right_i);

  // linear interplot
  return (y1 - y0) * (u - u0) / (u1 - u0) + y0;
}

// calculate score of one case by pass condition
void ReportHelper::CalScoreOfCase_ByPassCondition(const sim_msg::GradingKpiGroup_GradingKpi &kpi,
                                                  sim_msg::TestReport::Case &case_) {
  const auto &scoreMap = kpi.parameters().scoremap1d();
  auto detectedCount = case_.info().detected_count();
  double score = lookupScoreFromMap1d(kpi.parameters().scoremap1d(), detectedCount);
  case_.mutable_info()->mutable_result()->set_score(score);
}

// calculate score of one case by threshold
void ReportHelper::CalScoreOfCase_ByThreshold(const sim_msg::GradingKpiGroup_GradingKpi &kpi,
                                              sim_msg::TestReport::Case &case_) {
  LOG_ERROR << "do not use this CalScoreOfCase_ByThreshold function, not implemented yet.\n";
}
}  // namespace eval
