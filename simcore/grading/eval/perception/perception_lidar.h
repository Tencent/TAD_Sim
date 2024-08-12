// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (dongyuanhu)
//
// 激光感知数据记录, 记录感知目标的yaw与真值目标的差
// 激光感知数据记录, 记录感知目标的yaw,同时找到并显示真值目标的yaw
// 激光感知数据记录, 记录感知目标的ay与真值目标的差
// 激光感知数据记录, 记录感知目标的ay,同时找到并显示真值目标的ay
// 激光感知数据记录, 记录感知目标的ax与真值目标的差
// 激光感知数据记录, 记录感知目标的ax,同时找到并显示真值目标的ax
// 激光感知数据记录, 记录感知目标的vy与真值目标的差
// 激光感知数据记录, 记录感知目标的vy数据,同时找到并显示真值目标的vy
// 激光感知数据记录, 记录感知目标的vx与真值目标的差
// 激光感知数据记录, 记录感知目标的vx数据,同时找到并显示真值目标的vx
// 激光感知数据记录, 记录感知目标的y与真值目标的差
// 激光感知数据记录, 记录感知目标的y位置数据,同时找到并显示真值目标的y位置
// 激光感知数据记录, 记录感知目标的x与真值目标的差
// 激光感知数据记录, 记录感知目标的x位置数据,同时找到并显示真值目标的x位置
// 激光感知数据记录, 记录感知目标的位置数据,同时找到并显示真值目标的位置

#pragma once

#include "eval/eval_base.h"
#include "perception_base.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace eval {
class EvalPerceptionLidar : public EvalBase {
 public:
  /**
   * @brief 指标构造函数
   */
  EvalPerceptionLidar();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalPerceptionLidar() {}

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

 public:
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalPerceptionLidar>(); }

 private:
  std::unordered_map<uint64_t, TrackedObjPlotPtr> m_capture;

  const std::string PERCEPTION_TOPIC = "PERCEPTION_LIDAR";
};
}  // namespace eval
