// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_control.h"

#include "trajectory.pb.h"
#include "trajectoryFollowing.pb.h"

#include <algorithm>
#include <numeric>

namespace eval {
const char EvalControl::_kpi_name[] = "MaxPosError_H";
const char g_MaxAveragePosError_H[] = "MaxAveragePosError_H";
const char g_MaxPosError_H_L[] = "MaxPosError_H_L";
const char g_MaxPosError_H_R[] = "MaxPosError_H_R";

sim_msg::TestReport_XYPlot s_lateral_offset_plot;
sim_msg::TestReport_PairData s_average_lateral_offset;
sim_msg::TestReport_PairData s_left_lateral_offset;
sim_msg::TestReport_PairData s_right_lateral_offset;

EvalControl::EvalControl() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

int EvalControl::GetNearestPointByPos(const CTrajectory &traj, CEgoActorPtr ego) {
  int index_nearest = -1;

  if (ego) {
    double min_dist = 1e12;

    for (size_t i = 0; i < traj.size(); i++) {
      double dist = CEvalMath::Sub(traj[i].GetPosition(), ego->GetLocation().GetPosition()).GetNormal2D();
      if (dist < min_dist) {
        min_dist = dist;
        index_nearest = i;
      }
    }
  }

  return index_nearest;
}

double EvalControl::CalPosError_H(const CTrajectory &traj, CEgoActorPtr ego) {
  int index_nearest = GetNearestPointByPos(traj, ego);

  if (ego && index_nearest >= 0) {
    double cur_x = ego->GetLocation().GetPosition().GetX();
    double cur_y = ego->GetLocation().GetPosition().GetY();
    double tar_x = traj[index_nearest].GetPosition().GetX();
    double tar_y = traj[index_nearest].GetPosition().GetY();
    double tar_yaw = traj[index_nearest].GetEuler().GetYaw();

    double X0 = tar_x, Y0 = tar_y;
    double X1 = tar_x + 1.0 * cos(tar_yaw);
    double Y1 = tar_y + 1.0 * sin(tar_yaw);
    double X2 = cur_x, Y2 = cur_y;

    double value = (X1 - X0) * (Y2 - Y0) - (X2 - X0) * (Y1 - Y0);
    double length = hypot(X0 - X1, Y0 - Y1) + 0.01;

    double pos_error = value / length;

    return pos_error;
  }

  return 0.0;
}

bool EvalControl::Init(eval::EvalInit &helper) {
  // determine whether the module is valid. If valid, check if kpi is enabled and get the threshold values.
  if (IsModuleValid()) {
    // get accompany kpi first
    if (helper.getGradingKpiByName(g_MaxAveragePosError_H, m_Kpi)) {
      auto case_ = std::make_shared<AccompanyCase>(g_MaxAveragePosError_H, m_Kpi, true);
      m_accompanyCases.push_back(case_);
      if (isReportEnabled()) {
        ReportHelper::SetCaseInfo(case_->m_reportCase, m_Kpi);
      }
    }
    if (helper.getGradingKpiByName(g_MaxPosError_H_L, m_Kpi)) {
      auto case_ = std::make_shared<AccompanyCase>(g_MaxPosError_H_L, m_Kpi, true);
      m_accompanyCases.push_back(case_);
      if (isReportEnabled()) {
        ReportHelper::SetCaseInfo(case_->m_reportCase, m_Kpi);
      }
    }
    if (helper.getGradingKpiByName(g_MaxPosError_H_R, m_Kpi)) {
      auto case_ = std::make_shared<AccompanyCase>(g_MaxPosError_H_R, m_Kpi, true);
      m_accompanyCases.push_back(case_);
      if (isReportEnabled()) {
        ReportHelper::SetCaseInfo(case_->m_reportCase, m_Kpi);
      }
    }

    // get main kpi
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    // output indicator configuration details
    DebugShowKpi();
  }
  _pos_error_h = 0.0;
  _averagePosError_H = 0.0;
  _maxPosError_H = 0.0;
  _maxPosError_H_L = 0.0;
  _maxPosError_H_R = 0.0;
  _pos_error_arr.clear();

  // set report info
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    ReportHelper::ConfigXYPlot(s_lateral_offset_plot, "lateral offset", "", "t", "s", {"lateral offset"}, {"m"}, 1);
    ReportHelper::ConfigXYPlotThreshold(s_lateral_offset_plot, "thresh upper", 0, 1, m_defaultThreshDouble,
                                        "thresh lower", 1, 1, -m_defaultThreshDouble);
    ReportHelper::ConfigPairData(s_average_lateral_offset);
    ReportHelper::ConfigPairData(s_left_lateral_offset);
    ReportHelper::ConfigPairData(s_right_lateral_offset);
  }

  return true;
}
bool EvalControl::Step(eval::EvalStep &helper) {
  _pos_error_h = 0.0;

  // check whether the module is valid and whether the indicator is enabled
  if (IsModuleValid() && m_KpiEnabled) {
    // get the ego pointer and check whether the pointer is null
    auto ego_front = _actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      sim_msg::Trajectory traj;
      // sim_msg::TrajectoryFollow traj_follow;
      CTrajectory traj_local;

      EvalMsg eval_msg = _msg_mgr->Get(topic::TRAJECTORY);

      if (eval_msg.GetPayload().size() > 0) {
        traj.ParseFromString(eval_msg.GetPayload());
        for (auto pt : traj.point()) {
          CLocation loc;
          loc.MutablePosition()->SetValues(pt.x(), pt.y(), pt.z(), Coord_WGS84);
          _map_mgr->WGS84ToENU(*loc.MutablePosition());
          loc.MutableEuler()->SetValues(0.0, 0.0, pt.theta());
          traj_local.push_back(loc);
        }
        _pos_error_h = CalPosError_H(traj_local, ego_front);
      }

      _detector.Detect(std::abs(_pos_error_h), m_defaultThreshDouble);

      // add data to xy-pot
      if (isReportEnabled()) {
        s_lateral_offset_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        s_lateral_offset_plot.mutable_y_axis()->at(0).add_axis_data(_pos_error_h);
      }

      VLOG_1 << "lateral position error " << _pos_error_h << " m.\n";
    } else {
      VLOG_1 << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_1 << _kpi_name << " not enabled.\n";
    return false;
  }

  _pos_error_arr.push_back(_pos_error_h);

  return true;
}
bool EvalControl::Stop(eval::EvalStop &helper) {
  if (_pos_error_arr.size() > 0) {
    _maxPosError_H_L = *std::max_element(_pos_error_arr.begin(), _pos_error_arr.end());
    _maxPosError_H_R = *std::min_element(_pos_error_arr.begin(), _pos_error_arr.end());
    _maxPosError_H = std::max(std::abs(_maxPosError_H_L), std::abs(_maxPosError_H_R));
    _averagePosError_H = std::accumulate(_pos_error_arr.begin(), _pos_error_arr.end(), 0.0);
    _averagePosError_H = _averagePosError_H / _pos_error_arr.size();
  }

  helper.SetFeedback("MaxPosError_H", std::to_string(_maxPosError_H));
  helper.SetFeedback("MaxPosError_H_L", std::to_string(_maxPosError_H_L));
  helper.SetFeedback("MaxPosError_H_R", std::to_string(std::abs(_maxPosError_H_R)));

  // add report
  if (isReportEnabled()) {
    auto attach = _case.add_steps()->add_attach();

    ReportHelper::SetPairData(s_average_lateral_offset, "average lateral offset", std::to_string(_averagePosError_H));
    ReportHelper::SetPairData(s_left_lateral_offset, "max left lateral offset", std::to_string(_maxPosError_H_L));
    ReportHelper::SetPairData(s_right_lateral_offset, "max right lateral offset", std::to_string(_maxPosError_H_R));

    ReportHelper::AddXYPlot2Attach(*attach, s_lateral_offset_plot);
    ReportHelper::AddPair2Attach(*attach, s_average_lateral_offset);
    ReportHelper::AddPair2Attach(*attach, s_left_lateral_offset);
    ReportHelper::AddPair2Attach(*attach, s_right_lateral_offset);

    auto setAccompanyReport = [this](const std::string &kpiID, const sim_msg::TestReport_PairData &pairData) {
      auto casePtr = m_accompanyCases.getReportByKpiID(kpiID);
      if (casePtr != nullptr) {
        auto attach = casePtr->add_steps()->add_attach();
        ReportHelper::AddPair2Attach(*attach, pairData);
      }
    };

    setAccompanyReport(g_MaxAveragePosError_H, s_average_lateral_offset);
    setAccompanyReport(g_MaxPosError_H_L, s_left_lateral_offset);
    setAccompanyReport(g_MaxPosError_H_R, s_right_lateral_offset);
  }

  return true;
}

EvalResult EvalControl::IsEvalPass() {
  if (m_KpiEnabled) {
    _case.mutable_info()->set_detected_count(_detector.GetCount());
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "above max lateral position error");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "max lateral position error check pass");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "max lateral position error check skipped");
}
bool EvalControl::ShouldStopScenario(std::string &reason) {
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  if (ret) reason = "above max lateral position error";
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
void EvalControl::SetGradingMsg(sim_msg::Grading &msg) {
  msg.mutable_control()->set_threshold(m_defaultThreshDouble);
  msg.mutable_control()->set_poserror_h(_pos_error_h);
  msg.mutable_control()->set_state(sim_msg::GradingPosErrorState::GRADING_PosError_NORMAL);

  if (_detector.GetLatestState()) {
    msg.mutable_control()->set_state(sim_msg::GradingPosErrorState::GRADING_PosError_EXCEED);
  }

  // set detected event
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}
void EvalControl::SetLegacyReport(sim_msg::Grading_Statistics &msg) {
  msg.mutable_detail()->mutable_count_overmaxposerror_h()->set_eval_value(_detector.GetCount());
  msg.mutable_detail()->mutable_count_overmaxposerror_h()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_count_overmaxposerror_h()->set_is_pass(IsEvalPass()._state ==
                                                                       sim_msg::TestReport_TestState_PASS);

  msg.mutable_detail()->mutable_average_poserror_h()->set_eval_value(_averagePosError_H);
  msg.mutable_detail()->mutable_average_poserror_h()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_average_poserror_h()->set_is_pass(IsEvalPass()._state ==
                                                                  sim_msg::TestReport_TestState_PASS);

  msg.mutable_detail()->mutable_max_poserror_h()->set_eval_value(_maxPosError_H);
  msg.mutable_detail()->mutable_max_poserror_h()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_max_poserror_h()->set_is_pass(IsEvalPass()._state ==
                                                              sim_msg::TestReport_TestState_PASS);

  msg.mutable_detail()->mutable_max_poserror_h_l()->set_eval_value(_maxPosError_H_L);
  msg.mutable_detail()->mutable_max_poserror_h_l()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_max_poserror_h_l()->set_is_pass(IsEvalPass()._state ==
                                                                sim_msg::TestReport_TestState_PASS);

  msg.mutable_detail()->mutable_max_poserror_h_r()->set_eval_value(_maxPosError_H_R);
  msg.mutable_detail()->mutable_max_poserror_h_r()->set_is_enabled(m_KpiEnabled);
  msg.mutable_detail()->mutable_max_poserror_h_r()->set_is_pass(IsEvalPass()._state ==
                                                                sim_msg::TestReport_TestState_PASS);
}
}  // namespace eval
