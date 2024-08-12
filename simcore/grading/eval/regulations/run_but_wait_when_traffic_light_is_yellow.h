// Copyright 2024 Tencent Inc. All rights reserved.
//
// 黄灯在可通过路口前提下准许通行
//
// \par References:
// [1] https://baike.baidu.com/item/%E9%BB%84%E7%81%AF%E6%97%B6%E9%95%BF/10757866
//
// 条件1:主车外缘最前方在停止线上
// 条件2:无停止线且主车外缘最前方在junction上时
// 判别:在条件1或2的情况下, 判断当前车道红绿灯的灯态
// 黄灯时间参考下方链接
// https://baike.baidu.com/item/%E9%BB%84%E7%81%AF%E6%97%B6%E9%95%BF/10757866

#pragma once

#include "eval/eval_base.h"

namespace eval {
class RunButWaitWhenTrafficLightIsYellow : public EvalBase {
 private:
  double _count;
  bool _cond_has_close_vehicle_ahead;
  bool _cond_on_stop_lane;
  bool _cond_on_junction;
  bool _cond_on_yellow_light;
  double _result;
  double wait_timer;
  const double _threshold = 0.5;
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-INT32_MAX);

 public:
  /**
   * @brief 指标构造函数
   */
  RunButWaitWhenTrafficLightIsYellow();
  /**
   * @brief 指标析构函数
   */
  virtual ~RunButWaitWhenTrafficLightIsYellow() {}

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
  static sim_msg::TestReport_XYPlot s_RunButWaitWhenTrafficLightIsYellow_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<RunButWaitWhenTrafficLightIsYellow>(); }
};
}  // namespace eval
