// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (lohasye)
//
// 外部接入自定义指标示范
//

#pragma once

#include "eval/eval_base.h"

namespace eval {

class EvalExKpiExample : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类（视情况，上升沿和下降沿选一个即可）
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-1e6);
  /**
   * @brief 用下降沿检测模版类（视情况，上升沿和下降沿选一个即可）
   */
  // FallingDetection<double> _fall_detector = MakeFallingDetection<double>(-1e6);

  /**
   * @brief 在这放一些自定义的评测内用的变量
   */
  bool _is_collision;

  /**
   * @brief 在这放一些自定义的评测内用的成员函数
   */
  bool CheckIsCollision(const RectCorners &corners, const double time);

 public:
  /**
   * @brief 指标构造函数
   */
  EvalExKpiExample();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalExKpiExample() {}

 public:
  /**
   * @brief 指标Init方法
   * @param helper 评测辅助类Init类
   * @param actor_mgr 交通参与者管理者
   * @param map_mgr 地图管理器
   * @param msg_mgr 仿真消息管理器
   * @return true 初始化成功
   * @return false 初始化失败
   */
  virtual bool Init(eval::EvalInit &helper, ActorManager *actor_mgr, MapManager *map_mgr, MsgManager *msg_mgr);
  /**
   * @brief 指标Step方法
   * @param helper 评测辅助类Step类
   * @param actor_mgr 交通参与者管理者
   * @param map_mgr 地图管理器
   * @param msg_mgr 仿真消息管理器
   * @return true 单步执行成功
   * @return false 单步执行失败
   */
  virtual bool Step(eval::EvalStep &helper, ActorManager *actor_mgr, MapManager *map_mgr, MsgManager *msg_mgr);
  /**
   * @brief 指标Stop方法
   * @param helper 评测辅助类Stop类
   * @param actor_mgr 交通参与者管理者
   * @param map_mgr 地图管理器
   * @param msg_mgr 仿真消息管理器
   * @return true 停止成功
   * @return false 停止失败
   */
  virtual bool Stop(eval::EvalStop &helper, ActorManager *actor_mgr, MapManager *map_mgr, MsgManager *msg_mgr);
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
  static sim_msg::TestReport_XYPlot _s_external_kpi_example_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalExKpiExample>(); }
};
}  // namespace eval
