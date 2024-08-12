// Copyright 2024 Tencent Inc. All rights reserved.
//
// 第八十二条
// 机动车在高速公路上行驶,不得有下列行为:
// (一) 倒车,逆行,穿越中央分隔带掉头或者在车道内停车;
// (二) 在匝道,加速车道或者减速车道上超车;
// (三) 骑,轧车行道分界线或者在路肩上行驶;
// (四) 非紧急情况时在应急车道行驶或者停车;
// (五) 试车或者学习驾驶机动车.
//
// \par References:
// [1] http://www.gov.cn/zhengce/2020-12/27/content_5574617.htm
//
// 条件1: 当前主车行驶路段为高速公路
// 条件2: 当前主车行驶在减速段内
// 判别: 条件1&2时,主车是否超车

#pragma once

#include "eval/eval_base.h"

namespace eval {
class NoOvertakingWhenExpresswayDecelerationSection : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-INT32_MAX);

 public:
  /**
   * @brief 指标构造函数
   */
  NoOvertakingWhenExpresswayDecelerationSection();
  /**
   * @brief 指标析构函数
   */
  virtual ~NoOvertakingWhenExpresswayDecelerationSection() {}

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
  static sim_msg::TestReport_XYPlot s_no_overtaking_when_expressway_deceleration_section_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<NoOvertakingWhenExpresswayDecelerationSection>(); }
};
}  // namespace eval
