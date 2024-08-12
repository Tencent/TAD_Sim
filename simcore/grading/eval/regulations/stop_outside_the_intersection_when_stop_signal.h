// Copyright 2024 Tencent Inc. All rights reserved.
//
// 第五十一条
// 机动车通过有交通信号灯控制的交叉路口,应当按照下列规定通行:
// (一)在划有导向车道的路口,按所需行进方向驶入导向车道;
// (二)准备进入环形路口的让已在路口内的机动车先行;
// (三)向左转弯时,靠路口中心点左侧转弯.转弯时开启转向灯,夜间行驶开启近光灯;
// (四)遇放行信号时,依次通过;
// (五)遇停止信号时,依次停在停止线以外.没有停止线的,停在路口以外;
// (六)向右转弯遇有同车道前车正在等候放行信号时,依次停车等候;
// (七)在没有方向指示信号灯的交叉路口,转弯的机动车让直行的车辆/行人先行.相对方向行驶的右转弯机动车让左转弯车辆先行.
//
// \par References:
// [1] http://www.gov.cn/zhengce/2020-12/27/content_5575017.htm
//
// 条件1: 当前主车行驶车道红绿灯为红灯
// 条件2: 当前主车行驶车道不存在停止线
// 判别: 条件1&2时,主车外缘最前方是否在junction以外

#pragma once

#include "eval/eval_base.h"

namespace eval {
class StopOutsideTheIntersectionWhenStopSignal : public EvalBase {
 private:
  bool _cond_on_junction;
  bool _cond_has_traffic_light;
  bool _cond_on_red_light;
  bool _cond_on_stop_line;
  bool _has_stop_line;
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
  StopOutsideTheIntersectionWhenStopSignal();
  /**
   * @brief 指标析构函数
   */
  virtual ~StopOutsideTheIntersectionWhenStopSignal() {}

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
  static sim_msg::TestReport_XYPlot s_stop_outside_the_intersection_when_stop_signal_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<StopOutsideTheIntersectionWhenStopSignal>(); }
};
}  // namespace eval
