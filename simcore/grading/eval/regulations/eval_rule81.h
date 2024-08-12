// Copyright 2024 Tencent Inc. All rights reserved.
//
// 第八十一条
// 机动车在高速公路上行驶,遇有雾、雨、雪、沙尘、冰雹等低能见度气象条件时,应当遵守下列规定:
// (一) 能见度小于200米时,开启雾灯、近光灯、示廓灯和前后位灯,车速不得超过每小时60公里,与同车道前车保持100米以上的距离;
// (二)
// 能见度小于100米时,开启雾灯、近光灯、示廓灯、前后位灯和危险报警闪光灯,车速不得超过每小时40公里,与同车道前车保持50米以上的距离;
// (三)
// 能见度小于50米时,开启雾灯、近光灯、示廓灯、前后位灯和危险报警闪光灯,车速不得超过每小时20公里,并从最近的出口尽快驶离高速公路.
//
// \par References:
// [1] http://www.gov.cn/zhengce/2020-12/27/content_5574617.htm
//

#pragma once

#include "eval/eval_base.h"

namespace eval {
class EvalRule81 : public EvalBase {
 private:
  double _cond_visibility;
  bool _cond_on_enter_expressway;
  double _result_speed_m_s;
  double _result_distance;
  double _threshold_lower_distance;
  double _threshold_upper_speed_m_s;
  /**
   * @brief 用下降沿检测模版类
   */
  FallingDetection<double> _detector_lower_distance = MakeFallingDetection<double>(1e12);
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector_upper_speed_m_s = MakeRiseUpDetection<double>(-INT32_MAX);

 public:
  /**
   * @brief 指标构造函数
   */
  EvalRule81();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalRule81() {}

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
  static sim_msg::TestReport_XYPlot s_rule81_plot;
  /**
   * @brief 指标结果附加信息，评测报告用
   */
  static sim_msg::TestReport_PairData s_distance_variance_pair;
  /**
   * @brief 指标结果附加信息，评测报告用
   */
  static sim_msg::TestReport_PairData s_speed_variance_pair;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalRule81>(); }
};
}  // namespace eval
