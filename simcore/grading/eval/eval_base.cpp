// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval/eval_base.h"
#include "utils/proto_helper.h"

namespace eval {
const char EvalBase::_kpi_name[] = "DoNotExist";
const EvalResult constEvalPass(sim_msg::TestReport_TestState_PASS, "");

/**
 * @brief accompany test case of this main test case
 */
AccompanyCase::AccompanyCase() {
  m_KpiID = "";
  m_kpi.Clear();
  m_kpiEnabled = false;
  m_reportCase.Clear();
}

AccompanyCase::AccompanyCase(const std::string& kpiID, const EvalKpi& kpi, const bool& enabled) {
  m_KpiID = kpiID;
  m_kpi.CopyFrom(kpi);
  m_kpiEnabled = enabled;
  m_reportCase.Clear();
}

void AccompanyCaseList::emplace_back(const std::string& kpiID, const EvalKpi& kpi, const bool& enabled) {
  auto casePtr = std::make_shared<AccompanyCase>(kpiID, kpi, enabled);
  push_back(casePtr);
}

sim_msg::TestReport::Case* AccompanyCaseList::getReportByKpiID(const std::string& kpiID) {
  sim_msg::TestReport::Case* caseReportPtr = nullptr;

  for (auto i = 0; i < size(); ++i) {
    auto& testCase = at(i);
    if (testCase.get() != nullptr && testCase->m_KpiID == kpiID) {
      caseReportPtr = &(testCase->m_reportCase);
      return caseReportPtr;
    }
  }

  return caseReportPtr;
}

AccompanyCasePtr AccompanyCaseList::getTestCaseByKpiID(const std::string& kpiID) {
  AccompanyCasePtr casePtr;

  for (auto i = 0; i < size(); ++i) {
    auto& testCase = at(i);
    if (testCase.get() != nullptr && testCase->m_KpiID == kpiID) {
      return testCase;
    }
  }

  return casePtr;
}

EvalBase::EvalBase() {
  _actor_mgr = CActorManager::GetInstance();
  _map_mgr = CMapManager::GetInstance();
  _msg_mgr = CMsgManager::GetInstance();
  m_KpiEnabled = false;
  m_defaultThreshBool = false;
  m_defaultThreshDouble = 0.0;
  m_defaultThreshString = "";
}

EvalBase::~EvalBase() {}

bool EvalBase::IsModuleValid() {
  return _actor_mgr.get() != nullptr && _map_mgr.get() != nullptr && _msg_mgr.get() != nullptr;
}

bool EvalBase::Init(eval::EvalInit& helper) { return true; }

bool EvalBase::Step(eval::EvalStep& helper) {
  // pre-condition check
  if (!IsModuleValid() || !_eval_cfg._enabled) {
    return false;
  }

  /* add your eval code here */

  return true;
}

bool EvalBase::Stop(eval::EvalStop& helper) { return true; }

bool EvalBase::Init(eval::EvalInit& helper, ActorManager* actor_mgr, MapManager* map_mgr, MsgManager* msg_mgr) {
  return true;
}

bool EvalBase::Step(eval::EvalStep& helper, ActorManager* actor_mgr, MapManager* map_mgr, MsgManager* msg_mgr) {
  return true;
}

bool EvalBase::Stop(eval::EvalStop& helper, ActorManager* actor_mgr, MapManager* map_mgr, MsgManager* msg_mgr) {
  return true;
}

// grading information in each step, such as event detection used in simcity
void EvalBase::SetGradingMsg(sim_msg::Grading& msg) {}

void EvalBase::DebugShowKpi() {
  std::string content;
  eval::protoToJson(m_Kpi, content);
  VLOG_2 << content << "\n";
}

bool EvalBase::getThreshValueByID_Boolean(const EvalKpi& kpi, bool& kpiEnabled, const std::string& threshID) {
  bool threshValue = false;

  if (!kpiEnabled) return threshValue;

  for (const auto& thresh : kpi.parameters().thresholds()) {
    if (threshID == thresh.thresholdid()) {
      if (thresh.thresholdtype() == sim_msg::GradingKpiGroup_ThresholdType_ThresholdType_BOOL) {
        int val = 0;
        try {
          val = std::atoi(thresh.threshold().c_str());
        } catch (const std::exception& e) {
          LOG_ERROR << "fail to convert thresh " << e.what() << ", in kpi " << m_Kpi.name();
          val = 0;
          kpiEnabled = false;
        }
        threshValue = static_cast<bool>(val);
        return threshValue;
      }
    }
  }

  // disable kpi
  kpiEnabled = false;

  LOG_ERROR << "fail to get thresh from " << kpi.name() << " by ID of " << threshID << ", kpi disabled.\n";

  return threshValue;
}

double EvalBase::getThreshValueByID_Double(const EvalKpi& kpi, bool& kpiEnabled, const std::string& threshID) {
  double threshValue = 0.0;

  if (!kpiEnabled) return threshValue;

  for (const auto& thresh : kpi.parameters().thresholds()) {
    if (threshID == thresh.thresholdid()) {
      // if (thresh.thresholdtype() == sim_msg::GradingKpiGroup_ThresholdType_ThresholdType_DOUBLE) {
      double val = 0;
      try {
        val = std::atof(thresh.threshold().c_str());
      } catch (const std::exception& e) {
        LOG_ERROR << "fail to convert thresh " << e.what() << ", in kpi " << m_Kpi.name();
        val = 0.0;
        kpiEnabled = false;
      }

      threshValue = val;

      return threshValue;
      //}
    }
  }

  // disable kpi
  kpiEnabled = false;
  LOG_ERROR << "fail to get thresh from " << kpi.name() << " by ID of " << threshID << ", kpi disabled.\n";

  return threshValue;
}

std::string EvalBase::getThreshValueByID_String(const EvalKpi& kpi, bool& kpiEnabled, const std::string& threshID) {
  std::string threshValue = "";

  if (!kpiEnabled) return threshValue;

  for (const auto& thresh : kpi.parameters().thresholds()) {
    if (threshID == thresh.thresholdid()) {
      if (thresh.thresholdtype() == sim_msg::GradingKpiGroup_ThresholdType_ThresholdType_STRING) {
        threshValue = thresh.threshold();
        return threshValue;
      }
    }
  }

  // disable kpi
  kpiEnabled = false;

  LOG_ERROR << "fail to get thresh from " << kpi.name() << " by ID of " << threshID << ", kpi disabled.\n";

  return threshValue;
}
}  // namespace eval
