// Copyright 2024 Tencent Inc. All rights reserved.
//
// 第八十条
// 机动车在高速公路上行驶, 车速超过每小时100公里时, 应当与同车道前车保持100米以上的距离,
// 车速低于每小时100公里时, 与同车道前车距离可以适当缩短, 但最小距离不得少于50米.
//
// \par References:
// [1] http://www.gov.cn/zhengce/2020-12/27/content_5574617.htm
//

#pragma once

#include "eval/eval_base.h"

namespace eval {
class EvalRule80 : public EvalBase {
 private:
  bool _cond_on_enter_expressway;
  double _result_distance;
  double _threshold_lower_distance;
  /**
   * @brief 用下降沿检测模版类
   */
  FallingDetection<double> _detector_lower_distance = MakeFallingDetection<double>(1e12);

 public:
  /**
   * @brief 指标构造函数
   */
  EvalRule80();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalRule80() {}

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
  static sim_msg::TestReport_XYPlot s_rule80_plot;
  /**
   * @brief 指标结果附加信息，评测报告用
   */
  static sim_msg::TestReport_PairData s_distance_variance_pair;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalRule80>(); }
};
}  // namespace eval
