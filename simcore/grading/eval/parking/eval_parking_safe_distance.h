// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (mama)
//
// 1. 对于每辆交通车,逐个计算交通车的四个角和主车框的最短距离,逐个计算主车四个角和交通车框的最短距离;
// 2. 对于每个动态障碍物,逐个计算障碍物位置和主车框的最短距离;
// 3. 对于每个静态障碍物,逐个计算障碍物位置和主车框的最短距离;
// 4. 取以上所有计算的最短距离的最小值,作为"主车与障碍物的距离",并与阈值进行比较.
// 若PARKING_STATE的msg为空,则"主车与障碍物的距离"为浮点型数值最大值(输出null).

#pragma once

#include "eval/eval_base.h"

namespace eval {
class EvalParkingSafeDistance : public EvalBase {
 private:
  /**
   * @brief 用下降沿检测模版类
   */
  FallingDetection<double> _detector = MakeFallingDetection<double>(1e6);

  double _parking_safe_distance;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalParkingSafeDistance();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalParkingSafeDistance() {}

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
   * @brief 指标结果附加信息，评测报告用
   */
  static sim_msg::TestReport_PairData s_parking_safe_distance_pair;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalParkingSafeDistance>(); }
};
}  // namespace eval
