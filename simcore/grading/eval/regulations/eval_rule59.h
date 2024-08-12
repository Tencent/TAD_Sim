// Copyright 2024 Tencent Inc. All rights reserved.
//
// 第五十九条
// 机动车在夜间通过急弯/坡路/拱桥/人行横道或者没有交通信号灯控制的路口时, 应当交替使用远近光灯示意.
// 机动车驶近急弯/坡道顶端等影响安全视距的路段以及超车或者遇有紧急情况时, 应当减速慢行, 并鸣喇叭示意.
//
// \par References:
// [1] http://www.gov.cn/zhengce/2020-12/27/content_5574617.htm
//

#pragma once

#include "eval/eval_base.h"

namespace eval {
class EvalRule59 : public EvalBase {
 private:
  bool _cond_night;
  bool _cond_has_traffic_light;
  bool _cond_on_crosswalk;
  bool _cond_on_junction;
  bool _cond_on_bridge;
  bool _cond_on_sharp_curve;
  bool _cond_on_steep_slope;
  bool _actual_beam_proper_used;
  bool _actual_horn_proper_used;
  double _result;
  const double _threshold = 0.5;
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-INT32_MAX);

 public:
  /**
   * @brief 指标构造函数
   */
  EvalRule59();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalRule59() {}

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
  static sim_msg::TestReport_XYPlot s_rule59_plot;
  // static sim_msg::TestReport_XYPlot s_beam_proper_used_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalRule59>(); }
};
}  // namespace eval
