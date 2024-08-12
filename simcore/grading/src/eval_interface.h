// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "manager/eval_manager.h"
#include "txsim_logger.h"
#include "txsim_module.h"

namespace eval {
/**
 * @brief "评测模块总接口", finally inherit from SimModule. It's the interface class for eval module.
 */
class EvalInterface final : public tx_sim::SimModule {
 private:
  uint32_t _enable_grading;
  uint32_t _enable_pblogger;

  EvalManagerPtr _eval_mgr;
  pblog::txSimLogger _data_logger;

  std::string _ego_groupname;
  double _t_ms;
  bool m_sync_save_report, m_is_simcity;
  std::string _payload;
  std::string _cloud_dir, _log_folder;
  std::vector<std::string> _sub_topics;
  std::vector<std::string> _pub_topics;
  bool mEvalValid;
  std::string _external_eval_lib_file;

 public:
  EvalInterface();
  virtual ~EvalInterface();

  void Init(tx_sim::InitHelper& helper);
  void Reset(tx_sim::ResetHelper& helper);
  void Step(tx_sim::StepHelper& helper);
  void Stop(tx_sim::StopHelper& helper);
};
}  // namespace eval
