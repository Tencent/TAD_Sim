// Copyright 2024 Tencent Inc. All rights reserved.
//
// 绿灯应尽快通行
//
// 1. 主车前方无车,且处于等红灯状态,红灯变绿灯,主车速度从0加速至0.1m/s的时间是否在阈值内;
// 2. 主车经过路口绿灯,前方10m内无车,主车存在刹停行为不通过

#pragma once

#include "eval/eval_base.h"

namespace eval {
class RunWhenTrafficLightIsGreen : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _duration_detector = MakeRiseUpDetection<double>(-1e6);
  /**
   * @brief 用下降沿检测模版类
   */
  FallingDetection<double> _transit_detector = MakeFallingDetection<double>(1e6);
  double _start_duration;
  bool _wait_red_light;

 public:
  /**
   * @brief 指标构造函数
   */
  RunWhenTrafficLightIsGreen();
  /**
   * @brief 指标析构函数
   */
  virtual ~RunWhenTrafficLightIsGreen() {}

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
   * @brief 指标plot图定义，评测报告用
   */
  static sim_msg::TestReport_XYPlot s_RunWhenTrafficLightIsGreen_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<RunWhenTrafficLightIsGreen>(); }
};
}  // namespace eval
