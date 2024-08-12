// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (dongyuanhu)
//
// 未来1秒钟纵向位置预测最大误差小于阈值

#pragma once

#include <cstdint>
#include <queue>
#include "eval/eval_base.h"
#include "eval/prediction/prediction_base.h"

namespace eval {
class EvalPredVerticalMax : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-1e6);
  FellowPredManager m_fellow_mgr;  // hold prediction and fellow trajectory within 1 second
  double m_timewindow;             // unit second
  std::string m_pred_payload, m_fellow_payload;
  size_t m_index;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalPredVerticalMax();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalPredVerticalMax() {}

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
   * @brief 指标结果表格，评测报告用
   */
  static sim_msg::TestReport_XlsxSheet s_lateral_sheet;
  static sim_msg::TestReport_XlsxSheet_SheetData s_index, s_t, s_fellow_id, s_error;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalPredVerticalMax>(); }
};
}  // namespace eval
