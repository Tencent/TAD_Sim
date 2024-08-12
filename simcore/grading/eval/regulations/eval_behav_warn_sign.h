// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (xuanwang)
//
// 检测车辆在警告标志牌前的行为是否正确.
//
// 1. 获取主车前方100m警告标志牌信息
// 2. 获取主车速度/加速度和主车行驶状态
// 3. 判断是否符合指示牌限制
//
// 情况1.停车标志牌 (ego speed>0.1m/s则不通过)
// 情况2.小心危险标志牌(ego_speed>30km/h或ego acc x>0.2则不通过)
// 情况3.小心转弯标志牌(ego speed>30km/h或ego acc x>0.2则不通过)
// 情况4.小心连续转弯标志牌(ego speed>30km/h或ego acc x>0.2则不通过
// 情况5.小心道路不平标志牌(ego speed>30km/h或egoacc x>02则不通过
// 情况6.小心路滑标志牌(ego speed>30km/h或ego acc x>0.2则不通过)
// 情况7.小心道路变窄标志牌(egospeed>30km/h或ego acc x>0.2则不通过
// 情况8.小心强侧风标志牌(ego speed>30km/h或ego acc x>0.2则不通过
// 情况9.小心施工处标志牌(ego speed>30km/h或ego acc x>0.2则不通过
// 情况10.小心容易发生塞车标志牌(ego speed>30km/h或ego acc x>0.2则不通过
// 情况11.小心行人标志牌(ego speed>30km/h或ego acc x>0.2则不通过)
// 情况12.小心自行车标志牌(ego speed>30km/h或ego acc x>0.2则不通过
// 情况13.小心野生动物标志牌(ego_speed>30km/h或ego_acc_x>0.2则不通过
// 情况14.小心前方火车通过标志牌(ego speed>30km/h或ego acc x>0.2则不通过)
// 情况15.小心前方有红绿灯标志牌(ego_speed>30km/h或ego_acc_x>0.2则不通过
// 情况16.小心人行横道标志牌(egospeed>30km/h或ego acc_x>0.2则不通过
// 情况17.小心雨雪标志牌(ego speed>30km/h或ego acc x>0.2则不通过)
// 情况18.小心坡道标志牌(egospeed>30km/h或ego acc x>0.2则不通过)
//
// 输出:具体情况 + 检测结果
//
// 注:"停车标志牌"/"小心人行横道标志牌"暂不支持

#pragma once

#include "eval/eval_base.h"

namespace eval {
/**
 * @brief 指标plot图定义，评测报告用
 */
extern sim_msg::TestReport_XYPlot _s_behav_warn_sign_plot;

class EvalBehavWarnSign : public EvalBase {
 private:
  /**
   * @brief 用下降沿检测模版类
   */
  FallingDetection<double> _detector = MakeFallingDetection<double>(1e6);
  std::set<std::string> set_msgs;
  std::unordered_map<int, std::string> warn_signs = {
      {6002, "turn-left"},    {6003, "turn-right"},         {6008, "uphill"},          {6009, "construction"},
      {6010, "narrow-left"},  {6011, "narrow-both"},        {6012, "care-railway"},    {6031, "curve_ahead"},
      {6032, "long-descent"}, {6033, "rough-road"},         {6034, "snow-rain"},       {6036, "animals"},
      {6042, "downhill"},     {6043, "queues-likely"},      {6048, "narrow-right"},    {6049, "care-non-motor"},
      {6050, "slippery"},     {6051, "care-traffic-light"}, {6057, "care-cross-wind"}, {6060, "care-danger"},
      {6149, "care-people"}};

 public:
  /**
   * @brief 指标构造函数
   */
  EvalBehavWarnSign();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalBehavWarnSign() {}

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
  static EvalBasePtr Build() { return std::make_shared<EvalBehavWarnSign>(); }
};
}  // namespace eval
