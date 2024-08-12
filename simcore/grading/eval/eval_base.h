// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "utils/eval_common.h"
#include "utils/eval_math.h"
#include "utils/eval_shape.h"
#include "utils/eval_unit.h"
#include "utils/eval_utils.h"

#include "manager/actor_manager.h"
#include "manager/eval_helper.h"
#include "manager/map_manager.h"
#include "manager/msg_manager.h"

#include "manager/report_helper.h"

#include "grading.pb.h"
#include "grading_kpi.pb.h"
#include "grading_report.pb.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace eval {
struct EvalResult {
 public:
  sim_msg::TestReport_TestState _state;
  std::string _reason;
  EvalResult(sim_msg::TestReport_TestState state, const std::string& reason) : _state(state), _reason(reason) {}
  EvalResult() : EvalResult(sim_msg::TestReport_TestState::TestReport_TestState_PASS, "") {}
};

// alias for GradingKPI definition
using EvalKpi = sim_msg::GradingKpiGroup_GradingKpi;

/**
 * @brief accompany test case of this main test case
 */
struct AccompanyCase {
 public:
  std::string m_KpiID;
  EvalKpi m_kpi;
  bool m_kpiEnabled;
  sim_msg::TestReport::Case m_reportCase;

  AccompanyCase();
  AccompanyCase(const std::string& kpiID, const EvalKpi& kpi, const bool& enabled);
};

using AccompanyCasePtr = std::shared_ptr<AccompanyCase>;

class AccompanyCaseList : public std::vector<AccompanyCasePtr> {
 public:
  void emplace_back(const std::string& kpiID, const EvalKpi& kpi, const bool& enabled);
  sim_msg::TestReport::Case* getReportByKpiID(const std::string& kpiID);
  AccompanyCasePtr getTestCaseByKpiID(const std::string& kpiID);
};

// default eval result
const extern EvalResult constEvalPass;

/**
 * @brief "评测指标基础类", eval base algorithm, all eval indicators should inherit this class.
 */
class EvalBase : public NoneCopyable {
 public:
  // constructor, destructor
  EvalBase();
  virtual ~EvalBase();

 public:
  // init operation
  virtual bool Init(eval::EvalInit& helper);
  // step operation
  virtual bool Step(eval::EvalStep& helper);
  // stop operation
  virtual bool Stop(eval::EvalStop& helper);

  // init operation for external eval algorithm
  virtual bool Init(eval::EvalInit& helper, ActorManager* actor_mgr, MapManager* map_mgr, MsgManager* msg_mgr);
  // step operation for external eval algorithm
  virtual bool Step(eval::EvalStep& helper, ActorManager* actor_mgr, MapManager* map_mgr, MsgManager* msg_mgr);
  // stop operation for external eval algorithm
  virtual bool Stop(eval::EvalStop& helper, ActorManager* actor_mgr, MapManager* map_mgr, MsgManager* msg_mgr);

  // grading information in each step, such as event detection used in simcity
  virtual void SetGradingMsg(sim_msg::Grading& msg);

  // grading report, legacy, not used anymore
  virtual void SetLegacyReport(sim_msg::Grading_Statistics& msg) {}

  // used to check eval result
  virtual EvalResult IsEvalPass() { return constEvalPass; }
  virtual bool ShouldStopScenario(std::string& reason) { return false; }

  // general grading report
  virtual inline sim_msg::TestReport::Case& GetReportCase() { return _case; }

  // accompany report cases
  virtual AccompanyCaseList& GetAccompanyCases() { return m_accompanyCases; }

 public:
  /**
   * @brief get thresh value by threshold id from kpi
   * @param threshID
   * @param kpi
   * @return true if bingo
   */
  bool getThreshValueByID_Boolean(const EvalKpi& kpi, bool& kpiEnabled,
                                  const std::string& threshID = "Default_Thresh_ID");

  /**
   * @brief get thresh value by threshold id from kpi
   * @param threshID
   * @param kpi
   * @return
   */
  double getThreshValueByID_Double(const EvalKpi& kpi, bool& kpiEnabled,
                                   const std::string& threshID = "Default_Thresh_ID");

  /**
   * @brief get thresh value by threshold id from kpi
   * @param threshID
   * @param kpi
   * @return
   */
  std::string getThreshValueByID_String(const EvalKpi& kpi, bool& kpiEnabled,
                                        const std::string& threshID = "Default_Thresh_ID");

 public:
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalBase>(); }

 protected:
  bool IsModuleValid();
  void DebugShowKpi();

 protected:
  ActorManagerPtr _actor_mgr;  // actor manager
  MapManagerPtr _map_mgr;      // map manager
  MsgManagerPtr _msg_mgr;      // simulation message manager

  EvalCfg _eval_cfg;  // kpi configuration, @dhu delete this value
  EvalKpi m_Kpi;      // grading kpi
  bool m_KpiEnabled;  // default is false

  bool m_defaultThreshBool;           // default thresh bool
  double m_defaultThreshDouble;       // default thresh double
  std::string m_defaultThreshString;  // default thresh string

  sim_msg::TestReport::Case _case;     // for report
  AccompanyCaseList m_accompanyCases;  // accompany cases
};

/**
 * @brief exception for kpi threshold value error
 */
class ThesholdValueError : public std::exception {
 public:
  /** Constructor (C strings).
   *  @param message C-style string error message.
   *                 The string contents are copied upon construction.
   *                 Hence, responsibility for deleting the char* lies
   *                 with the caller.
   */
  explicit ThesholdValueError(const char* message) : msg_(message) {}

  /** Constructor (C++ STL strings).
   *  @param message The error message.
   */
  explicit ThesholdValueError(const std::string& message) : msg_(message) {}

  /** Destructor.
   * Virtual to allow for subclassing.
   */
  virtual ~ThesholdValueError() noexcept {}

  /** Returns a pointer to the (constant) error description.
   *  @return A pointer to a const char*. The underlying memory
   *          is in posession of the ThesholdValueError object. Callers must
   *          not attempt to free the memory.
   */
  virtual const char* what() const noexcept { return msg_.c_str(); }

 protected:
  /** Error message.
   */
  std::string msg_;
};
}  // namespace eval
