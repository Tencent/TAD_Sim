// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (xuanwang)
//
// 检测车辆在指示标志牌前的行为是否正确.
//
// 1. 获取主车前方100m指示标志牌信息
// 2. 获取主车速度/加速度和主车行驶状态
// 3. 判断是否符合指示牌限制
//
// 情况1. 公交车站标志牌(ego acc x>0.2则不通过)
// 情况2. 靠右行驶标志牌(未靠右行驶则不通过)
// 情况3. 靠左行驶标志牌(未靠左行驶则不通过
// 情况4. 向右转弯标志牌 (未向右转则不通过
// 情况5. 向左转弯标
// 情况6. 直行标志牌
// 情况7. 直行向左转弯标志牌
// 情况8. 直行向右转弯标志牌
// 情况9. 向左向右转弯标志牌
// 情况10. 环岛标志牌 (ego_speed x >0.2则不通过)
// 情况11. 隧道标志牌(ego_speed>30km/h或ego_acc_x>0.2则不通过)
// 情况12. 自行车道标志牌(ego speed>30km/h或ego acc x>0.2则不通过)
// 情况13. 行人通道标志牌(ego_speed>30km/h或ego_acc_x>0.2则不通过)
// 情况14. 公交车道标志牌(egoacc x>0.2则不通过)
//
// 输出:具体情况 + 检测结果
//
// 注:"公交车站"/"隧道"/"公交车道"标志牌暂不支持. "行人通道"暂不支持,目前支持"步行"标志

#pragma once

#include "eval/eval_base.h"

namespace eval {
/**
 * @brief 指标plot图定义，评测报告用
 */
extern sim_msg::TestReport_XYPlot _s_behav_instruct_sign_plot;

class EvalBehavInstructSign : public EvalBase {
 private:
  /**
   * @brief 用下降沿检测模版类
   */
  FallingDetection<double> _fall_detector = MakeFallingDetection<double>(INT32_MAX);
  hadmap::OBJECT_SUB_TYPE subtype;
  bool _event_instruct;
  std::set<std::string> set_signs;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalBehavInstructSign();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalBehavInstructSign() {}

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
  static EvalBasePtr Build() { return std::make_shared<EvalBehavInstructSign>(); }
};
}  // namespace eval
