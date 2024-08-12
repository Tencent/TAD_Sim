// Copyright 2024 Tencent Inc. All rights reserved.
//
// 第三十八条　机动车信号灯和非机动车信号灯表示:
// (一) 绿灯亮时, 准许车辆通行, 但转弯的车辆不得妨碍被放行的直行车辆/行人通行.
// (二) 黄灯亮时, 已越过停止线的车辆可以继续通行.
// (三) 红灯亮时, 禁止车辆通行.
// 在未设置非机动车信号灯和人行横道信号灯的路口, 非机动车和行人应当按照机动车信号灯的表示通行.
// 红灯亮时, 右转弯的车辆在不妨碍被放行的车辆/行人通行的情况下, 可以通行.
//
// \par References:
// [1] http://www.gov.cn/zhengce/2020-12/27/content_5574617.htm
//

#pragma once

#include "eval/eval_base.h"

namespace eval {
class EvalRule38 : public EvalBase {
 private:
  bool _cond_on_red_light;
  bool _event_rush_red_light;
  bool _cond_on_right_lane;
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-INT32_MAX);

 public:
  /**
   * @brief 指标构造函数
   */
  EvalRule38();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalRule38() {}

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
  static sim_msg::TestReport_XYPlot s_rule38_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalRule38>(); }
};
}  // namespace eval
