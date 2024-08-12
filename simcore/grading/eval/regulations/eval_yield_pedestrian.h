// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (xuanwang)
//
// 检测主车在驾驶过程中是否礼让行人.
// 1.  检测主车前进方向半圆区域内,最大行人距离内是否有行人.
// 2.  检测主车与行人距离是否在距离区间内.
// 3.  检测主车是否停车.
// 4.  检测主车与行人距离是否变大,若是则记录此刻到主车启动时的启动时间.
//
// 情况1: 是否停车礼让.
// 情况2: 礼让停车时,与行人距离是否符合距离区间.
// 情况3: 行人远离后,主车启动时长是否超过阈值.
//
// 输出:具体情况+检测结果

#pragma once

#include "eval/eval_base.h"

namespace eval {
/**
 * @brief 指标plot图定义，评测报告用
 */
extern sim_msg::TestReport_XYPlot _s_behav_pedestrian_plot;

class EvalYieldPedestrian : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-INT32_MAX);
  bool _event_yield;
  double min_dist_to_person, max_dist_to_person, max_starting_time, stopped_distance;
  double restart_time;
  std::string _msg;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalYieldPedestrian();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalYieldPedestrian() {}

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
   * @brief 指标结果附加信息，评测报告用
   */
  static sim_msg::TestReport_PairData _g_yield_pair;
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalYieldPedestrian>(); }
};
}  // namespace eval
