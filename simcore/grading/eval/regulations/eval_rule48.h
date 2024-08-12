// Copyright 2024 Tencent Inc. All rights reserved.
//
// 第四十八条
// 在没有中心隔离设施或者没有中心线的道路上, 机动车遇相对方向来车时应当遵守下列规定:
// (一) 减速靠右行驶, 并与其他车辆/行人保持必要的安全距离;
// (二) 在有障碍的路段, 无障碍的一方先行;但有障碍的一方已驶入障碍路段而无障碍的一方未驶入时, 有障碍的一方先行;
// (三) 在狭窄的坡路, 上坡的一方先行;但下坡的一方已行至中途而上坡的一方未上坡时, 下坡的一方先行;
// (四) 在狭窄的山路, 不靠山体的一方先行;
// (五) 夜间会车应当在距相对方向来车150米以外改用近光灯, 在窄路/窄桥与非机动车会车时应当使用近光灯.
//
// \par References:
// [1] http://www.gov.cn/zhengce/2020-12/27/content_5574617.htm
//

#pragma once

#include "eval/eval_base.h"

namespace eval {
class EvalRule48 : public EvalBase {
 private:
  bool _cond_night;
  bool _cond_no_center_lane;
  bool _cond_on_narrow_lane;
  bool _cond_on_bridge;
  bool _actual_beam_proper_used;
  bool _actual_keep_safe_distance;
  const double _const_min_safe_distance = 30.0;
  double _result;
  const double _threshold = 0.5;
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-INT32_MAX);

 public:
  /**
   * @brief 指标构造函数
   */
  EvalRule48();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalRule48() {}

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
  static sim_msg::TestReport_XYPlot s_rule48_plot;
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
  static EvalBasePtr Build() { return std::make_shared<EvalRule48>(); }
};
}  // namespace eval
