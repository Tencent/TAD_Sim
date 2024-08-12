// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_ex_kpi_example.h"

namespace eval {
// _kpi_name即本指标的名称，需要和external_grading_kpi.json的"kpi"数组里相应指标的"name"字段一致
const char EvalExKpiExample::_kpi_name[] = "ExKpiExample";

// 指标检测结果的xy plot图，最终会被画入评测报告中
sim_msg::TestReport_XYPlot EvalExKpiExample::_s_external_kpi_example_plot;

EvalExKpiExample::EvalExKpiExample() { VLOG_0 << "eval algorithm " << _kpi_name << " constructed.\n"; }

bool EvalExKpiExample::Init(eval::EvalInit &helper, ActorManager *actor_mgr, MapManager *map_mgr, MsgManager *msg_mgr) {
  VLOG_0 << _kpi_name << " Init.\n";
  // 判断入参指针是否有内容。 如果有效，将检查 kpi 是否已启用并获取相关阈值。
  if (actor_mgr && map_mgr && msg_mgr) {
    m_KpiEnabled = helper.getGradingKpiByName(_kpi_name, m_Kpi);
    m_defaultThreshDouble = getThreshValueByID_Double(m_Kpi, m_KpiEnabled);
    _is_collision = false;
    // 输出指标配置详细信息以供调试，仅在主程序_log_level配置参数是>=2时输出
    DebugShowKpi();
  }

  // 在simcity=0，即非虚拟城市模式下，isReportEnabled()都是true，下同
  if (isReportEnabled()) {
    ReportHelper::SetCaseInfo(_case, m_Kpi);
    // 设置一些plot图的名称和单位
    ReportHelper::ConfigXYPlot(_s_external_kpi_example_plot, "collision check", "check collision", "t", "s",
                               {"if_collision"}, {"N/A"}, 1);
    // 设置一些plot图的阈值及其类型
    ReportHelper::ConfigXYPlotThreshold(_s_external_kpi_example_plot, "", 0, 1, 1, "", 1, 0, INT32_MIN, 0);
  }
  return true;
}

bool EvalExKpiExample::CheckIsCollision(const RectCorners &corners, const double time) {
  // 根据自己的需要实现所需的功能，本函数仅作示例
  return static_cast<int64_t>(time) % 10 == 9 ? true : false;
}

bool EvalExKpiExample::Step(eval::EvalStep &helper, ActorManager *actor_mgr, MapManager *map_mgr, MsgManager *msg_mgr) {
  VLOG_0 << _kpi_name << " Step.\n";
  // 判断入参指针是否有内容和 kpi 是否已启用
  if (actor_mgr && map_mgr && msg_mgr && m_KpiEnabled) {
    // 获取主车指针，并检查指针是否为空
    auto ego_front = actor_mgr->GetEgoFrontActorPtr();
    if (ego_front) {
      // 在这里写入评测逻辑。
      _is_collision = CheckIsCollision(ego_front->TransCorners2BaseCoord(), helper.GetSimTime());
      // 使用上升沿检测进行检测，会统计其超过0.5的上升沿次数
      _detector.Detect(_is_collision, 0.5);
      // 记录日志的示例
      VLOG_0 << "if collision : " << _is_collision << "\n";

      if (isReportEnabled()) {
        // x是时间，y是数据，要一一对应
        _s_external_kpi_example_plot.mutable_x_axis()->add_axis_data(helper.GetSimTime());
        _s_external_kpi_example_plot.mutable_y_axis()->at(0).add_axis_data(_is_collision);
      }
    } else {
      LOG_ERROR << "ego actor missing.\n";
      return false;
    }
  } else {
    VLOG_0 << _kpi_name << " not enabled.\n";
    return false;
  }
  return true;
}
bool EvalExKpiExample::Stop(eval::EvalStop &helper, ActorManager *actor_mgr, MapManager *map_mgr, MsgManager *msg_mgr) {
  VLOG_0 << _kpi_name << " Stop.\n";
  // add report
  if (isReportEnabled()) {
    // 将plot图添加到_case的相应地方
    auto attach = _case.add_steps()->add_attach();
    ReportHelper::AddXYPlot2Attach(*attach, _s_external_kpi_example_plot);
  }
  return true;
}

void EvalExKpiExample::SetGradingMsg(sim_msg::Grading &msg) {
  // 在msg填写本指标在这一帧是否不通过
  EvalHelper::SetDetectedEvent(msg, _detector, _kpi_name);
}

EvalResult EvalExKpiExample::IsEvalPass() {
  if (m_KpiEnabled) {
    // 在评测报告case填写检测结果
    _case.mutable_info()->set_detected_count(_detector.GetCount());

    // 若检测次数大于通过阈值且阈值大于0，则返回不通过
    if (_detector.GetCount() >= m_Kpi.passcondition().value() && m_Kpi.passcondition().value() >= 0.5) {
      return EvalResult(sim_msg::TestReport_TestState_FAIL, "collision happens");
    } else {
      return EvalResult(sim_msg::TestReport_TestState_PASS, "collision not happens");
    }
  }

  return EvalResult(sim_msg::TestReport_TestState_PASS, "collision check pass");
}

bool EvalExKpiExample::ShouldStopScenario(std::string &reason) {
  // 若检测次数大于结束场景阈值且阈值大于0，则返回结果设置为true
  auto ret = _detector.GetCount() >= m_Kpi.finishcondition().value() && m_Kpi.finishcondition().value() >= 0.5;
  // 若结果是true，填写结束场景原因
  if (ret) reason = "collision";
  // 设置结束场景标志位
  _case.mutable_info()->set_request_stop(ret);
  return ret;
}
}  // namespace eval
