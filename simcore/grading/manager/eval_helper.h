// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "utils/eval_common.h"
#include "utils/eval_math.h"
#include "utils/eval_shape.h"
#include "utils/eval_unit.h"
#include "utils/eval_utils.h"

#include "manager/actor_manager.h"
#include "manager/map_manager.h"
#include "manager/msg_manager.h"

#include "stateflow/veh_state_flow.h"

#include "grading.pb.h"
#include "grading_report.pb.h"

#include <functional>
#include <memory>

namespace eval {
/**
 * @brief "评测辅助类基类", base class for all eval helpers, provides some important helper methods or api.
 */
class EvalHelper : public NoneCopyable {
  friend class EvalManager;

 public:
  EvalHelper();
  virtual ~EvalHelper() {}

 public:
  // calculate current system time, use y-m-d-h-m-s
  static std::string GetCurrentSystemTime(const int64_t unix_time = -1, const bool detail_time = true);

  // get current system time, unit s
  static double GetSystemUnixTime();

  // check if module valid
  inline bool IsModuleValid() { return _actor_mgr && _map_mgr && _msg_mgr; }

  // set module config
  void SetModuleCfg(const std::string &module_cfg);
  void SetModuleStatus(const std::string &module_status);

  // get module step time, use second
  inline CSimTime GetStepTime() { return CSimTime(getModuleStepTime() * 1000.0); }

  // get module config
  inline const ModuleConfig &GetModuleCfg() { return m_module_cfg; }

  /**
   * @brief helper functions for detected event
   * @tparam T: int/float/double/bool etc.
   * @param msg: grading msg
   * @param detector: detector
   */
  template <class T>
  static void SetDetectedEvent(sim_msg::Grading &msg, const RiseUpDetection<T> &detector, const std::string &kpiID) {
    // add detected event if has
    auto &eventMap = *msg.mutable_event_detector()->mutable_detectedevents();

    if (!detector.GetLatestState() && !detector.GetLatestEdgeState()) {
      // try to erase if no event detected for now
      eventMap.erase(kpiID);  // nothing will happen if map contains no key
    } else {
      // add event to map
      auto detectedEvent = sim_msg::Grading_EventDetector_DetectedEvent();
      detectedEvent.set_state(sim_msg::Grading_EventDetector_EventState_EventDetected);
      detectedEvent.set_edge(static_cast<sim_msg::Grading_EventDetector_EventState>(detector.GetLatestEdgeState()));
      eventMap[kpiID] = detectedEvent;

      // update flag of event map
      if (detector.GetLatestEdgeState()) {
        msg.mutable_event_detector()->set_hasedge(sim_msg::Grading_EventDetector_EventState_EventDetected);
      }
    }
  }

  /**
   * @brief helper functions for detected event
   * @tparam T: int/float/double/bool etc.
   * @param msg: grading msg
   * @param detector: detector
   */
  template <class T>
  static void SetDetectedEvent(sim_msg::Grading &msg, const FallingDetection<T> &detector, const std::string &kpiID) {
    // add detected event if has
    auto &eventMap = *msg.mutable_event_detector()->mutable_detectedevents();

    if (!detector.GetLatestState() && !detector.GetLatestEdgeState()) {
      // try to erase if no event detected for now
      eventMap.erase(kpiID);  // nothing will happen if map contains no key
    } else {
      // add event to map
      auto detectedEvent = sim_msg::Grading_EventDetector_DetectedEvent();
      detectedEvent.set_state(sim_msg::Grading_EventDetector_EventState_EventDetected);
      detectedEvent.set_edge(static_cast<sim_msg::Grading_EventDetector_EventState>(detector.GetLatestEdgeState()));
      eventMap[kpiID] = detectedEvent;

      // update flag of event map
      if (detector.GetLatestEdgeState()) {
        msg.mutable_event_detector()->set_hasedge(sim_msg::Grading_EventDetector_EventState_EventDetected);
      }
    }
  }

 protected:
  bool Init() { return true; }

 protected:
  // manager
  ActorManagerPtr _actor_mgr;
  MapManagerPtr _map_mgr;
  MsgManagerPtr _msg_mgr;
  ModuleConfig m_module_cfg;
};

/**
 * @brief "评测辅助类Init类", init class eval helper, provides additional init methods or api than EvalHelper.
 */
class EvalInit final : public EvalHelper {
  friend class EvalManager;

 public:
  // get scenario info
  inline const ScenarioInfo &GetScenarioInfo() const { return m_scenario_info; }

  // get eval config by kpi name
  EvalCfg GetEvalCfg(const std::string &kpi_name) const;

  // get all eval configs
  inline const EvalCfgMap &GetEvalCfgs() { return _eval_cfgs; }

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

 protected:
  // reset function
  bool Init(const ScenarioInfo &scene_info);

  // calculate report dir
  void CalReportDir();

 protected:
  // scenario info
  ScenarioInfo m_scenario_info;

  // eval configurations
  EvalCfgLoader _eval_cfg_loader;
  EvalCfgMap _eval_cfgs;
};

/**
 * @brief "评测辅助类Step类", step class eval helper, provides additional step methods or api than EvalHelper.
 */
class EvalStep final : public EvalHelper {
  friend class EvalManager;

 public:
  EvalStep() {}
  virtual ~EvalStep();

 public:
  // get legacy grading message
  inline sim_msg::Grading &GetGradingMsg() { return _grading; }

  // get sim time, unit second
  inline double GetSimTime() { return _sim_t_s; }

  // get leading vehicle pointer
  inline CVehicleActorPtr GetLeadingVehicle() { return _leading_vehicle; }

  // get vehicle state-flow
  const VehFlow &GetVehStateFlow() { return m_veh_flow; }

  // find collision actor
  template <class ActorPtrTy>
  static ActorPtrTy FindCollisionFellow(CEgoActorPtr ego_ptr, ActorList<ActorPtrTy> &fellows) {
    for (auto fellow : fellows) {
      if (CollisionDetection::IsCollision(ego_ptr, fellow)) return fellow;
    }
    return nullptr;
  }

  // find collision object
  static bool FindCollisionObject(CEgoActorPtr ego_ptr, CActorMapObjects &fellows) {
    for (auto fellow : fellows) {
      if (CollisionDetection::IsCollision(ego_ptr, fellow.get())) return true;
    }
    return false;
  }

  // get vehicle actors which are in front of ego and adjacent to lane of ego
  ActorList<CVehicleActorPtr> FindFrontAndAdjacentVehicleActor(CEgoActorPtr ego, double range = 50.0);

  // get vehicle actors which are in front of ego and adjacent to lane of ego
  ActorList<CVehicleActorPtr> FindSurroundingVehicleActor(CEgoActorPtr ego, double range = 200.0);

  // get static actors which are in front of ego
  ActorList<CStaticActorPtr> FindFrontStaticActor(CEgoActorPtr ego, double range = 50.0);

  // get dynamic actors which are in front of ego
  std::vector<ActorList<CDynamicActorPtr>> FindFrontDynamicActor(CEgoActorPtr ego,
                                                                 std::vector<eval::DynamicActorType> types,
                                                                 double range = 50.0);

  DynamicActorType GetDynamicActorType(CDynamicActorPtr actor);

 protected:
  // reset function
  bool Init();

  // execute
  bool Update(double t_ms);

 private:
  // set sim time, unit second
  inline void SetSimTime(double t_s) { _sim_t_s = t_s; }

  // calculate leading vehicle
  void CalLeadingActor(const ActorManagerPtr &actor_mgr);

  // update ego state flow
  void UpdateEgoStateFlow();

 protected:
  // legacy grading messages
  sim_msg::Grading _grading;

  // simulation time
  double _sim_t_s;

  // leading actor pointer
  CVehicleActorPtr _leading_vehicle;

  // vehicle state-flow
  VehFlow m_veh_flow;
  CEgoAssemble m_ego_assemble;
};

/**
 * @brief "评测辅助类Stop类", stop class eval helper, provides additional stop methods or api than EvalHelper.
 */
class EvalStop final : public EvalHelper {
  friend class EvalManager;

 public:
  EvalStop() : _report(nullptr), _ego_meta(nullptr), _ego_cases(nullptr) {}
  virtual ~EvalStop() {}

 public:
  // get legacy grading message
  inline sim_msg::Grading_Statistics &GetGradingStatistics() { return _grading_stat; }

  // get grading report message
  sim_msg::TestReport *GetReport() { return _report.get(); }

  // get grading ego meta
  sim_msg::TestReport_EgoMeta *GetEgoMeta() { return _ego_meta.get(); }

  // get grading ego case
  sim_msg::TestReport_EgoCase *GetEgoCase() { return _ego_cases.get(); }

  // set feedback
  inline void SetFeedback(const std::string &key, const std::string &val) { m_feedback[key] = val; }

  // get feedback
  const std::unordered_map<std::string, std::string> &GetFeedback() { return m_feedback; }

 public:
  // get grading report unique ptr, do call this function before saving report to disk
  std::unique_ptr<sim_msg::TestReport> GetReportUniPtr() { return std::move(_report); }

 protected:
  // init function
  bool Init();

 protected:
  // legacy grading messages
  sim_msg::Grading_Statistics _grading_stat;

  // eval report
  // sim_msg::TestReport _report;
  std::unique_ptr<sim_msg::TestReport> _report;
  // sim_msg::TestReport_EgoMeta _ego_meta;
  std::unique_ptr<sim_msg::TestReport_EgoMeta> _ego_meta;
  // sim_msg::TestReport_EgoCase _ego_cases;
  std::unique_ptr<sim_msg::TestReport_EgoCase> _ego_cases;

  std::unordered_map<std::string, std::string> m_feedback;
};
}  // namespace eval
