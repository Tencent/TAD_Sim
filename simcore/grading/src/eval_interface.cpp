// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "src/eval_interface.h"
#include "utils/eval_log.h"

#define SPLIT_LINE "========================================\n"

namespace eval {
EvalInterface::EvalInterface() {
  _enable_grading = 1;
  _enable_pblogger = 1;
  _t_ms = 0;
  _cloud_dir = "";
  FLAGS_v = 0;
  _log_folder = "";
  _cloud_dir = "";
  m_sync_save_report = true;
  m_is_simcity = false;
  mEvalValid = true;
  _eval_mgr.reset();
}
EvalInterface::~EvalInterface() {}

void EvalInterface::Init(tx_sim::InitHelper& helper) {
  // set vlog level
  std::string glog_level = helper.GetParameter("_log_level");
  if (!glog_level.empty()) {
    FLAGS_v = std::atoi(glog_level.c_str());
  }

  VLOG_1 << SPLIT_LINE;

  // if enable grading
  std::string par_str = helper.GetParameter("enableGrading");
  if (!par_str.empty()) {
    _enable_grading = static_cast<uint32_t>(std::stoi(par_str));
  }
  LOG(INFO) << "grading | grading module enable state is " << _enable_grading << "\n";

  // if enable data logger
  par_str = helper.GetParameter("enablePBLogger");
  if (!par_str.empty()) {
    _enable_pblogger = static_cast<uint32_t>(std::stoi(par_str));
  }
  LOG(INFO) << "grading | pblogger module enable state is " << _enable_pblogger << "\n";

  // get cloud dir
  std::string cloud_dir_str = helper.GetParameter("DataDir");
  if (!cloud_dir_str.empty()) {
    _cloud_dir = cloud_dir_str;
    LOG(INFO) << "grading | cloud data dir is " << _cloud_dir << "\n";
  }

  // get log foler
  std::string log_folder = helper.GetParameter("log-folder");
  if (!log_folder.empty()) {
    _log_folder = log_folder;
    LOG(INFO) << "grading | log dir is " << _log_folder << "\n";
  }

  // get sub topics
  std::string sub_topics = helper.GetParameter("channels");
  // additional sub topics
  sub_topics += "||" + std::string(topic::PREDICTIONS) + "||" + std::string(topic::LOCATION_UNION);
  // add topics to _sub_topics
  if (!sub_topics.empty()) {
    pblog::CPBLogCommon::StringSplit(sub_topics, "||", _sub_topics);
    for (auto topic : _sub_topics) {
      helper.Subscribe(topic);
    }
    VLOG_0 << "grading | subscribed to topics: " << sub_topics;
  }
  // additional sub topics but not added to _sub_topics
  helper.Subscribe(topic::MODULE_CONFIG);

  // sync save report mode. Warning: must be true when multiple egos.
  std::string sync_save = helper.GetParameter("sync");
  m_sync_save_report = true;
  if (!sync_save.empty()) {
    m_sync_save_report = static_cast<bool>(std::stoi(sync_save.c_str()));
  }

  // check if is simcity
  std::string simcity = helper.GetParameter("simcity");
  m_is_simcity = false;
  if (!simcity.empty()) {
    m_is_simcity = static_cast<bool>(std::stoi(simcity.c_str()));
  }
  VLOG_0 << "simcity is " << m_is_simcity << "\n";

  // try to get step time
  std::string stepTime = helper.GetParameter("stepTime");
  double moduleStepTime = 0.01;
  if (stepTime.empty()) {
    moduleStepTime = std::stof(helper.GetParameter(tx_sim::constant::kInitKeyModuleExecutePeriod)) / 1000.0;
    VLOG_0 << "stepTime is not at Parameter list. use kInitKeyModuleExecutePeriod.\n";
  } else {
    moduleStepTime = std::atof(stepTime.c_str()) / 1000.0;
    VLOG_0 << "stepTime is at Parameter list. use it.\n";
  }
  setModuleStepTime(moduleStepTime);
  VLOG_0 << "moduleStepTime[s]:" << moduleStepTime << "\n";

  // try to get external eval lib file parameter
  _external_eval_lib_file = helper.GetParameter("exEvalLib");
  VLOG_0 << "_external_eval_lib_file: " << _external_eval_lib_file << "\n";

  // pub topics
  helper.Publish(topic::GRADING);
  helper.Publish(topic::GRADING_STATISTICS);

  if (_enable_pblogger) _data_logger.Init(helper);
}
void EvalInterface::Reset(tx_sim::ResetHelper& helper) {
  VLOG_1 << SPLIT_LINE;

  if (_enable_grading) {
    // reset eval managers
    _eval_mgr = std::make_shared<EvalManager>();

    // get my own ego groupname
    _ego_groupname = helper.group_name();

    // init eval manager
    EvalParams params = {_ego_groupname,         _cloud_dir, _sub_topics, _log_folder, m_sync_save_report, m_is_simcity,
                         _external_eval_lib_file};

    mEvalValid = _eval_mgr->Init(helper, params);
  }
  if (_enable_pblogger) _data_logger.Reset(helper);
}
void EvalInterface::Step(tx_sim::StepHelper& helper) {
  VLOG_1 << SPLIT_LINE;

  _t_ms = helper.timestamp();

  if (_enable_grading && mEvalValid) {
    // update eval one step
    _eval_mgr->Step(helper);

    // publish grading msg
    const auto& gradingMsg = _eval_mgr->GetGradingMsg();
    gradingMsg.SerializeToString(&_payload);
    helper.PublishMessage(topic::GRADING, _payload);
    VLOG_3 << "grading msg:" << gradingMsg.DebugString() << "\n";
  }

  if (_enable_pblogger) {
    // save grading msg
    _eval_mgr->GetGradingMsg().SerializeToString(&_payload);
    _data_logger.ProducePBLogEvent(topic::GRADING, _payload, _t_ms);

    // update data logger one step
    _data_logger.Step(helper);
  }
}
void EvalInterface::Stop(tx_sim::StopHelper& helper) {
  VLOG_1 << SPLIT_LINE;

  if (_enable_grading) {
    _eval_mgr->Stop(helper);
  }

  if (_enable_pblogger) {
    _eval_mgr->GetGradingStatMsg().SerializeToString(&_payload);
    _data_logger.ProducePBLogEvent(topic::GRADING_STATISTICS, _payload, _t_ms);
    _data_logger.Stop(helper);
  }

  _eval_mgr.reset();
}
}  // namespace eval

TXSIM_MODULE(eval::EvalInterface)
