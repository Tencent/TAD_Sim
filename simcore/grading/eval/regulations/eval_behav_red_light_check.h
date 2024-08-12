// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (xuanwang)
//
// 检测主车在行驶方向为红灯状态下的行为是否正确.
// 1. 检测主车行驶方向交通灯状态是否是红灯.
// 2. 检测主车车头中点与停止线的距离是否在阈值内.
// 3. 检测主车是否停车.
//
// 若在红灯状态下,主车超出停止线最小距离未停车,则输出: 红灯时非右转通行检测-未通过
// 若在红灯状态下,主车停车超出最大停止线距离或小于最小停止线距离

#pragma once

#include "eval/eval_base.h"

namespace eval {

class EvalBehavRedLightCheck : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-1e6);
  bool _run_traffic_light, _event_run_red;
  double min_dist, max_dist;
  std::string msg_res;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalBehavRedLightCheck();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalBehavRedLightCheck() {}

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

 public:
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief 指标plot图定义，评测报告用
   */
  static sim_msg::TestReport_XYPlot _s_behav_red_light_plot;
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalBehavRedLightCheck>(); }
};
}  // namespace eval
