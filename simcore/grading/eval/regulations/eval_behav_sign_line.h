// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (xuanwang)
//
// 检测车辆在具体道路标识线上的行为是否正确.
// 1. 获取主车前方100m禁止标志牌信息
// 2. 获取主车属性和主车行驶状态
// 3. 判断是否符合指示牌限制
//
// 情况1. 斑马线(ego acc_x>0.2 && ego_speed math.sqrt(2*6*5))*36 >30 则不通过
// 情况2. 红绿灯停止线 (ego speed > math.sqrt(2*6*5)则不通过)
// 情况3. 直行标志线车道(向左或向右转则不通过)
// 情况4. 左转标志线车道(直行或向右转则不通过)
// 情况5. 右转标志线车道(直行或向左转则不通过)
// 输出:具体情况 + 检测结果

#pragma once

#include "eval/eval_base.h"

namespace eval {
/**
 * @brief 指标plot图定义，评测报告用
 */
extern sim_msg::TestReport_XYPlot _s_behav_road_markline_plot;

class EvalBehavSignLine : public EvalBase {
 private:
  /**
   * @brief 用下降沿检测模版类
   */
  FallingDetection<double> _fall_detector = MakeFallingDetection<double>(INT32_MAX);
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _stop_line_detector = MakeRiseUpDetection<double>(-INT32_MAX);
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _cross_walk_detector = MakeRiseUpDetection<double>(-INT32_MAX);
  hadmap::OBJECT_SUB_TYPE subtype;
  bool _event_markline;
  std::string _msg_roadmark;
  std::set<std::string> set_msgs;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalBehavSignLine();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalBehavSignLine() {}

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
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalBehavSignLine>(); }
};
}  // namespace eval
