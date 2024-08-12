// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (mama)
//
// 1. 初始化上次档位为"NO_CONTROL"
// 2. 接收到PARKING_STATE的msg不为空,获取到主车处于"泊出状态". 否则泊出揉库次数置0;
// 3. "上次档位不为‘REVERSE’,但本次档位为REVERSE" 或 "上次档位为‘REVERSE’,但本次档位不为‘REVERSE’",对泊出揉库次数加1;
// 4. 将泊出揉库次数和阈值进行比较,更新上次的档位状态,并更新泊出揉库次数的最大值;
// 5. 最终仿真结束时输出记录的最大泊出揉库次数.

#pragma once

#include "eval/eval_base.h"

namespace eval {
class EvalParkingOutAccessTimes : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-1e6);

  int access_times;
  int max_access_times;
  VehicleState::GearMode last_gear_mode;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalParkingOutAccessTimes();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalParkingOutAccessTimes() {}

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
  static sim_msg::TestReport_PairData s_parking_out_access_times_pair;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalParkingOutAccessTimes>(); }
};
}  // namespace eval
