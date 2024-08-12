// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eval/eval_base.h"
#include "manager/actor_manager.h"
#include "manager/eval_helper.h"
#include "manager/msg_manager.h"
#include "utils/eval_common.h"

#include "txsim_module.h"

namespace eval {
enum C_EvalAlgorithmType { No_Result_Check, Need_Result_Check, External_Algorithm };
using EvalMaps =
    std::unordered_map<C_EvalAlgorithmType, EvalAlgorithmMap, std::hash<uint32_t>, std::equal_to<C_EvalAlgorithmType>>;

/**
 * @brief "评测管理器", eval manager, manages all eval behavior. It's a key member of eval interface.
 */
class EvalManager : public NoneCopyable {
 public:
  EvalManager();
  virtual ~EvalManager();

 public:
  virtual bool Init(tx_sim::ResetHelper &helper, const EvalParams &params);
  virtual bool Step(tx_sim::StepHelper &helper);
  virtual bool Stop(tx_sim::StopHelper &helper);

 public:
  // legacy grading message
  sim_msg::Grading GetGradingMsg();
  sim_msg::Grading_Statistics GetGradingStatMsg();

 protected:
  // create eval algorithms according to grading.xml
  void CreateEvalAlgorithms(const EvalCfgMap &eval_cfgs);

  // init eval algorithms
  void InitEvalAlgorithms(EvalAlgorithmMap &eval_map, tx_sim::ResetHelper &helper, bool is_external_kpi = false);

  // update eval algorithms
  void UpdateEvalAlgorithms(tx_sim::StepHelper &helper, bool &should_stop, std::string &reason);

  // collect eval result
  void StopEvalAlgorithms(tx_sim::StopHelper &helper, bool &is_pass, std::string &reason);

 protected:
  // check if actor/map/msg/heler module valid
  inline bool IsModuleValid() { return _actor_mgr && _map_mgr && _msg_mgr; }

  // get grading.json file path and .sim config, active after 20230528
  std::string GetEvalCfgFromSim(tx_sim::ResetHelper &helper, ScenarioInfo &scenario_info);

  // generate report json file
  std::string GenReportJsonFileName(const std::string &scene_name, const std::string &date);

  // release
  void Release();

 private:
  ActorManagerPtr _actor_mgr;
  MapManagerPtr _map_mgr;
  MsgManagerPtr _msg_mgr;

  EvalInit m_initHelper;
  EvalStep m_stepHelper;
  EvalStop m_stopHelper;

  EvalMaps _eval_maps;  // eval algorithms map

  double _cur_t_ms, m_pre_t_ms;
  std::string _start_system_t;
  std::string _scenario_name;
  std::string _grading_cfg_path;

  EvalParams m_params;
  bool m_executed;

  // external customized eval algorithm library pointer
  void *_external_lib;
};
}  // namespace eval
