// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (xuanwang)
//
// 检测主车的违规次数和扣分分数.
//
// 规则1:闯红灯
//  判别红灯状态下:
//  1. 主车停止状态v<0.56,前轮越过停止线,后轮没越过,不违规,检测通过
//  2. 主车停止状态v<0.56,后轮越过停止线,违规不按规定停车,扣2分,检测不通过
//  3. 主车越过对方车道停止线,违规闯红灯,扣6分,检测不通过
//
// 规则2:不按指定车道行驶.
//  进入路口前:
//  1. 当前为左转车道,主车直行或右转,违章扣2分,检测不通过
//  2. 当前为直行车道,主车左转或右转,违章扣2分,检测不通过
//  3. 当前为右转车道,主车直行或左转,违章扣2分,检测不通过
//
// 规则3:超速
//  首次超速超过1s持续时间少于5s记一次,5s后每累积5s继续计一次
//  获取当前道路的限速,获取主车当前速度.
//  1. 超速30% 扣3分
//  2. 超速 30%~50% 扣6分
//  3. 超速 >50% 扣12分
//
// 规则4:压实线行驶
//  检测主车是否压线,压线扣2分

#pragma once

#include "eval/eval_base.h"

namespace eval {

class EvalCompliance : public EvalBase {
  enum OVER_SPEED { NO_OVER = 0, OVER0 = 3, OVER30 = 6, OVER50 = 12 };

 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-1e6);
  int _violate_times;
  int _rush_score;
  int _speed_limit_score;
  int _error_lane_score;
  int _solid_line_score;

  bool _event_overspeed, _event_solid_line, _event_err_lane, _event_red_light;

  double _overspeed_time;
  OVER_SPEED _cur_over;
  int over_speed_times;
  bool _overspeed_score_flag;
  bool _errlane_score_flag;
  bool option_link;

  double m_speed_limit_from_scene;
  eval::SignalLightColor cur_light;
  std::vector<int> lane_phase;
  std::vector<int> before_lane_phase;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalCompliance();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalCompliance() {}

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
  static sim_msg::TestReport_XYPlot _s_compliance_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalCompliance>(); }
};
}  // namespace eval
