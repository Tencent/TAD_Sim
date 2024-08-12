// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (dongyuanhu)
//
// 判断主车当前行驶的车道类型是否属于正常可通行的车道.
// 计算步骤:
// 判断当前行驶的车道类型是否属于以下条件:
// 条件1. (LANE_TYPE_Driving) 驾驶driving:一条"正常"可供行驶/不属于其他类型的道路
// 条件2. (LANE_TYPE_Parking) 泊车parking:带停车位的车道.
// 条件3. (LANE_TYPE_BIDIRECTIONAL) 连续双向左转车道.
// 条件4. (LANE_TYPE_CONNECTINGRAMP) 连接匝道:连接两条高速公路的匝道. 例如高速公路路口.
// 条件5. (LANE_TYPE_ENTRY) 入口entry:描述了用于平行于主路路段的车道. 主要用于加速. .
// 条件6. (LANE_TYPE_EXIT) 出口exit:描述了用于平行于主路路段的车道. 主要用于减速.
// 条件7. (LANE_TYPE_MEDIAN) 分隔带median:描述了位于不同方向车道间的车道. 在城市中通常用来分隔大型道路上不同方向的交通.
// 条件8. (LANE_TYPE_OFFRAMP) 减速车道offRamp:驶出高速公路,驶向乡村或城市道路所需的匝道.
// 条件9. (LANE_TYPE_ONRAMP) 加速车道onramp:由乡村或城市道路引向高速公路的匝道.
// 若不属于,则判定为不通过.

#pragma once

#include "eval/eval_base.h"

namespace eval {

class EvalOnRoad : public EvalBase {
 private:
  /**
   * @brief 用下降沿检测模版类
   */
  FallingDetection<double> _detector = MakeFallingDetection<double>(1e6);
  bool _is_on_lane;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalOnRoad();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalOnRoad() {}

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
  static EvalBasePtr Build() { return std::make_shared<EvalOnRoad>(); }
  /**
   * @brief 指标plot图定义，评测报告用
   */
  static sim_msg::TestReport_XYPlot _s_on_lane_plot;
};
}  // namespace eval
