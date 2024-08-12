// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (dongyuanhu)
//
// 1. 主车是否处于换道状态
// 2. 取侧向加速度检测的最大加速度是否小于最大阈值
// 3. 取换道持续时间的最短时间是否大于最小阈值
// 4. 取换道持续时间的最长时间是否小于最大阈值
// 条件1下,满足条件2&3&4 评测通过

#pragma once

#include "eval/eval_base.h"

namespace eval {

extern sim_msg::TestReport_XlsxSheet s_ego_lc_duration;

class EvalLaneChangeDuration : public EvalBase {
 private:
  /**
   * @brief 用下降沿检测模版类
   */
  FallingDetection<double> _fall_detector = MakeFallingDetection<double>(1e6);
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-1e6);
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _acc_detector = MakeRiseUpDetection<double>(-1e6);
  sim_msg::TestReport_XlsxSheet_SheetData m_sim_t, m_duration;
  double max_duration, min_duration;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalLaneChangeDuration();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalLaneChangeDuration() {}

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
  static sim_msg::TestReport_XYPlot s_ego_lc_plot;
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalLaneChangeDuration>(); }
};
}  // namespace eval
