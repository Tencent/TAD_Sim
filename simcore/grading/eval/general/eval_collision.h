// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (dongyuanhu)
//
// 碰撞检测包含:
// 1. 车车碰撞检测(追尾/被追尾/正面对碰/垂直角度/斜角侧碰)
// 2. 车与障碍物碰撞(行人/自行车/静态障碍物/摩托车/坑洼道路)
// 3. 车与周边设施碰撞(电线杆/房屋/树木/绿化植被/交通标志)
//
// 响应检测包含:
// 1. 减速响应检测(全部情况):判断主车减速度是否大于1m/s^2
// 2. 转向响应检测 (车车正面碰撞/车车垂直碰撞/车车斜角侧碰/静态障碍物碰撞/坑洼道路碰撞):判断主车转向灯是否开启
//
// 计算步骤:
// 1. 获取主车位置/轮廓.
// 2. 获取主车10m范围内的object 列表.
// 3. 逐一访问object 列表元素,获取元素的位置/轮廓,类型.
// 4. 逐一访问过程中,通过判断主车与object之间轮廓的重合情况,判断是否碰撞,及碰撞角度和碰撞类型.
// 5. 逐一访问过程中,通过object的类型和碰撞类型,进行响应的检测,包括减速响应检测和转向检测(部分情况需要).
// 6. 记录碰撞和响应结果,并最终输出.
//
// 当前地图元素说明:
//    交通标志和电线杆无法区分;树木和绿化植被无法区分;坑洼与其他地面元素无法区分(井盖/凸起/裂缝等).

#pragma once

#include "eval/eval_base.h"

namespace eval {
/**
 * @brief 指标plot图定义，评测报告用
 */
extern sim_msg::TestReport_XYPlot s_collision_plot;

class EvalCollision : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<int> _detector = MakeRiseUpDetection<int>(0);
  CStaticActorPtr _collision_actor;
  bool _turn_response;
  bool _reduce_response;
  bool _is_collision = false;
  std::string _collision_type;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalCollision();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalCollision() {}

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
  /**
   * @brief 指标评测结果写入方法
   * @param msg Grading评测消息
   */
  virtual void SetGradingMsg(sim_msg::Grading &msg);
  /**
   * @brief 指标评测旧版报告写入方法
   * @param msg Grading统计信息消息，用于旧版报告
   */
  virtual void SetLegacyReport(sim_msg::Grading_Statistics &msg);

 private:
  void CheckCollisonTurnResponse(CEgoActorPtr ego_ptr);
  void CheckCollisonReduceResponse(CEgoActorPtr ego_ptr);

 public:
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalCollision>(); }
};
}  // namespace eval
