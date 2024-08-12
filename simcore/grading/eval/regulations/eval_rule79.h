// Copyright 2024 Tencent Inc. All rights reserved.
//
// 第七十九条
// 机动车从匝道驶入高速公路, 应当开启左转向灯, 在不妨碍已在高速公路内的机动车正常行驶的情况下驶入车道;
// 机动车驶离高速公路时, 应当开启右转向灯, 驶入减速车道, 降低车速后驶离;
//
// \par References:
// [1] http://www.gov.cn/zhengce/2020-12/27/content_5575017.htm
//

#pragma once

#include "eval/eval_base.h"

namespace eval {
class EvalRule79 : public EvalBase {
 private:
  bool _cond_on_enter_expressway;
  bool _cond_on_exit_expressway;
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
  EvalRule79();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalRule79() {}

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
  static sim_msg::TestReport_XYPlot s_rule79_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalRule79>(); }
};
}  // namespace eval
