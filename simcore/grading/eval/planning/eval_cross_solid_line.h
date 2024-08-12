// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (dongyuanhu)
//
// 检测主车在驾驶过程中是否压到实线.
//
// 1. 检测是否压到实线.
// 2. 在接下来10s内检测此次压线是否是换道行为.
//
// 压实线且未换道情况下,输出: 未换道但压到实线检测-未通过
// 压实线且换道情况下,输出:违规实线换道检测-未通过

#pragma once

#include "eval/eval_base.h"

namespace eval {
/**
 * @brief 指标plot图定义，评测报告用
 */
extern sim_msg::TestReport_XYPlot s_on_solidline_plot;

class EvalCrossSolidLine : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<int> _detector = MakeRiseUpDetection<int>(-INT32_MAX);
  double _c_time;
  bool _event_solid;
  int _lane_id_on_solid;
  std::string tip_msg;
  bool _cross_solid_line;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalCrossSolidLine();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalCrossSolidLine() {}

 public:
  /**
   * @brief 指标Init方法
   * @param helper 评测辅助类Init类
   * @return true 初始化成功
   * @return false 初始化失败
   */
  virtual bool Init(eval::EvalInit &helper);
  /**
   * @brief 指标Step方法
   * @param helper 评测辅助类Step类
   * @return true 单步执行成功
   * @return false 单步执行失败
   */
  virtual bool Step(eval::EvalStep &helper);
  /**
   * @brief 指标Stop方法
   * @param helper 评测辅助类Stop类
   * @return true 停止成功
   * @return false 停止失败
   */
  virtual bool Stop(eval::EvalStop &helper);

  /**
   * @brief 指标评测结果写入方法
   * @param msg Grading评测消息
   */
  virtual void SetGradingMsg(sim_msg::Grading &msg) override;

  /**
   * @brief 评测通过条件，判断指标是否通过
   * @return EvalResult 判断结果
   */
  virtual EvalResult IsEvalPass();
  /**
   * @brief 结束场景条件，判断指标是否应该立即停止场景
   * @param[out] reason 如果需要终止场景的原因
   * @return true 结束场景
   * @return false 不结束场景
   */
  virtual bool ShouldStopScenario(std::string &reason);
  /**
   * @brief 指标评测旧版报告写入方法
   * @param msg Grading统计信息消息，用于旧版报告
   */
  virtual void SetLegacyReport(sim_msg::Grading_Statistics &msg);

 public:
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalCrossSolidLine>(); }
};
}  // namespace eval
