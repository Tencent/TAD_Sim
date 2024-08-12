// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (dongyuanhu)
//
// 计算主车行驶舒适性,计算方法:
//
// 1. T取20帧为一个时步区间
// 2. 每一个时步区间,取主车 横/纵向加速度/横摆角速度/纵向加加速度jerk均方根
// 3. 根据公式I(rt,comfort)计算本区间的指标值
// 4. 取本区间及之前所有区间的指标值的平均值

#pragma once

#include "eval/eval_base.h"

namespace eval {

/**
 * @brief 指标plot图定义，评测报告用
 */
extern sim_msg::TestReport_XYPlot _s_drive_comfort_plot;

class EvalDrivingComfort : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-1e6);
  int frames_numbers;
  int time_cloth_sum;
  double i_comfort_sum;
  double lateral_acc2_sum;
  double long_acc2_sum;
  double angle2_sum;
  double jerk2_sum;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalDrivingComfort();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalDrivingComfort() {}

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
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalDrivingComfort>(); }
};
}  // namespace eval
