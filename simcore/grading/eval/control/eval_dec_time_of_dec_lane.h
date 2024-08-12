// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (dongyuanhu)
//
// 1. 检测主车当前是否处于高速减速车道,进入减速车道时开始计时当前系统时间 start_time
// 2. 检测主车的加速度是否> -0.01m/s^2,若大于则开始计时为未减速的系统时间.
// 3. 检测主车加速度<=-0.01m/s^2,若符合,结束减速状态并计入总的未减速时间.
// 4. 检测主车驶出减速车道,驶出则计算主车在减速车道内总时间
//
// 计算未减速时间与总时间的比值,比值大于1/3不通过

#pragma once

#include "eval/eval_base.h"

namespace eval {

class EvalDecTimeOfDecLane : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-1e6);
  bool _on_dec_lane;
  double all_time, ctime;
  double _md_step_time;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalDecTimeOfDecLane();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalDecTimeOfDecLane() {}

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
  static sim_msg::TestReport_XYPlot _s_behav_dec_lane_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalDecTimeOfDecLane>(); }
};
}  // namespace eval
