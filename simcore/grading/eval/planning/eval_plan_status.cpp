// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval/planning/eval_plan_status.h"

#include "planStatus.pb.h"

namespace eval {
const char EvalPlanStatus::_kpi_name[] = "Planning_PlanStatus";

sim_msg::TestReport_XYPlot EvalPlanStatus::s_acc_plot;
sim_msg::TestReport_XYPlot EvalPlanStatus::s_vel_plot;
sim_msg::TestReport_XYPlot EvalPlanStatus::s_lanechange_plot;

EvalPlanStatus::EvalPlanStatus() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }
bool EvalPlanStatus::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    // output indicator configuration details
    DebugShowKpi();
  }

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_acc_plot, "plan acc", "plan acc and actual acc", "time", "s",
                               {"expect acc", "actual acc"}, {"m|s2", "m|s2"}, 2);
    ReportHelper::ConfigXYPlot(s_vel_plot, "plan velocity", "plan velocity and actual velocity", "time", "s",
                               {"expect velocity", "actual velocity"}, {"m|s", "m|s"}, 2);
    ReportHelper::ConfigXYPlot(s_lanechange_plot, "lane change state", "plan lane change state", "time", "s",
                               {"lc_state"}, {"N/A"}, 1);
  }

  return true;
}
bool EvalPlanStatus::Step(eval::EvalStep &helper) {
  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    EvalMsg msg = _msg_mgr->Get(m_Const_Topic);
    const std::string &payload = msg.GetPayload();

    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front == nullptr) {
      VLOG_1 << "ego missing";
      return false;
    }

    if (payload.size() > 0) {
      sim_msg::PlanStatus plan_status;

      if (plan_status.ParseFromString(payload)) {
        double acc_expect = 0.0, velocity_expect = 0.0;
        acc_expect = plan_status.expect_acc().acc();
        velocity_expect = plan_status.expect_speed().speed();

        /*
                enum BehaviorType {
                        INVALID = 0;                     // ��Ч��Ϊ
                        LANE_KEEP = 1;                   // ��������
                        BREAKTHROUGH = 20;               // ����
                        LEFT_LANE_CHANGE_PREPARE = 31;   // �󻻵�׼��
                        RIGHT_LANE_CHANGE_PREPARE = 32;  // �һ���׼��
                        LEFT_LANE_CHANGE = 41;           // �󻻵���
                        RIGHT_LANE_CHANGE = 42;          // �һ�����
                        LEFT_LANE_CHANGE_HOLD = 51;      // �󻻵�����
                        RIGHT_LANE_CHANGE_HOLD = 52;     // �һ�������
                        LEFT_LANE_CHANGE_CANCEL = 61;    // �󻻵�����
                        RIGHT_LANE_CHANGE_CANCEL = 62;   // �һ�������
                  }
        */
        uint32_t lc_state = 0;
        const sim_msg::PlanClassification &lc_type = plan_status.class_();

        switch (lc_type) {
          case sim_msg::PLAN_GO_STRAIGHT: {
            lc_state = 1;
            break;
          }
          case sim_msg::PLAN_BREAK_THROUGH: {
            lc_state = 20;
            break;
          }
          case sim_msg::PLAN_CHANGE_LANE_LEFT_PREPARE: {
            lc_state = 31;
            break;
          }
          case sim_msg::PLAN_CHANGE_LANE_RIGHT_PREPARE: {
            lc_state = 32;
            break;
          }
          case sim_msg::PLAN_CHANGE_LANE_LEFT: {
            lc_state = 41;
            break;
          }
          case sim_msg::PLAN_CHANGE_LANE_RIGHT: {
            lc_state = 42;
            break;
          }
          case sim_msg::PLAN_CHANGE_LANE_LEFT_HOLD: {
            lc_state = 51;
            break;
          }
          case sim_msg::PLAN_CHANGE_LANE_RIGHT_HOLD: {
            lc_state = 52;
            break;
          }
          case sim_msg::PLAN_CHANGE_LANE_LEFT_CANCEL: {
            lc_state = 61;
            break;
          }
          case sim_msg::PLAN_CHANGE_LANE_RIGHT_CANCEL: {
            lc_state = 62;
            break;
          }
          default: {
            lc_state = 0;
            break;
          }
        }

        if (isReportEnabled()) {
          s_acc_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
          s_acc_plot.mutable_y_axis()->at(0).add_axis_data(acc_expect);
          s_acc_plot.mutable_y_axis()->at(1).add_axis_data(ego_front->GetAcc().GetX());

          s_vel_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
          s_vel_plot.mutable_y_axis()->at(0).add_axis_data(velocity_expect);
          s_vel_plot.mutable_y_axis()->at(1).add_axis_data(ego_front->GetSpeed().GetNormal2D());

          s_lanechange_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
          s_lanechange_plot.mutable_y_axis()->at(0).add_axis_data(lc_state);
        }
      } else {
        VLOG_1 << "fail to parse payload on topic " << m_Const_Topic << " with sim_msg::PlanStatus";
        return false;
      }
    } else {
      VLOG_1 << "payload on topic " << m_Const_Topic << " is empty";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return true;
  }

  return true;
}
bool EvalPlanStatus::Stop(eval::EvalStop &helper) {
  // add report
  if (isReportEnabled()) {
    auto attach1 = _case.add_steps()->add_attach();
    attach1->set_name("Ego PlanStatus");
    ReportHelper::AddXYPlot2Attach(*attach1, s_acc_plot);
    ReportHelper::AddXYPlot2Attach(*attach1, s_vel_plot);
    ReportHelper::AddXYPlot2Attach(*attach1, s_lanechange_plot);
  }

  return true;
}

void EvalPlanStatus::SetGradingMsg(sim_msg::Grading &msg) {
  // set detected event
  // EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalPlanStatus::IsEvalPass() {
  return EvalResult(sim_msg::TestReport_TestState_PASS, "Ego PlanStatus eval finished");
}
bool EvalPlanStatus::ShouldStopScenario(std::string &reason) { return false; }
}  // namespace eval
