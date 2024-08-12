// Copyright 2024 Tencent Inc. All rights reserved.
//
// 第五十三条
// 机动车遇有前方交叉路口交通阻塞时, 应当依次停在路口以外等候, 不得进入路口.
//
// 机动车在遇有前方机动车停车排队等候或者缓慢行驶时, 应当依次排队,
// 不得从前方车辆两侧穿插或者超越行驶, 不得在人行横道/网状线区域内停车等候.
//
// 机动车在车道减少的路口/路段, 遇有前方机动车停车排队等候或者缓慢行驶的,
// 应当每车道一辆依次交替驶入车道减少后的路口/路段.
//
// \par References:
// [1] http://www.gov.cn/zhengce/2020-12/27/content_5575017.htm
//

#pragma once

#include "eval/eval_base.h"

namespace eval {
class EvalRule53 : public EvalBase {
 private:
  bool _cond_on_junction;
  bool _cond_on_crosswalk;
  bool _cond_on_control_access;
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
  EvalRule53();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalRule53() {}

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
  static sim_msg::TestReport_XYPlot s_rule53_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalRule53>(); }
};
}  // namespace eval
