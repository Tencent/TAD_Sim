// Copyright 2024 Tencent Inc. All rights reserved.
//
// 第六十三条
// 机动车在道路上临时停车, 应当遵守下列规定:
// (一) 在设有禁停标志/标线的路段, 在机动车道与非机动车道/人行道之间设有隔离设施的路段以及人行横道/施工地段, 不得停车;
// (二) 交叉路口/铁路道口/急弯路/宽度不足4米的窄路/桥梁/陡坡/隧道以及距离上述地点50米以内的路段, 不得停车;
// (三) 公共汽车站/急救站/加油站/消防栓或者消防队(站)门前以及距离上述地点30米以内的路段, 除使用上述设施的以外, 不得停车;
// (四) 车辆停稳前不得开车门和上下人员, 开关车门不得妨碍其他车辆和行人通行;
// (五) 路边停车应当紧靠道路右侧, 机动车驾驶人不得离车, 上下人员或者装卸物品后, 立即驶离;
// (六) 城市公共汽车不得在站点以外的路段停车上下乘客.
//
// \par References:
// [1] http://www.gov.cn/zhengce/2020-12/27/content_5575017.htm
//
// 条件1: 当前主车行驶在窄路以及距离阈值范围以内
// 判别: 条件1时,主车是否停车

#pragma once

#include "eval/eval_base.h"

namespace eval {
class NoStopWhenNarrowRoad : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-INT32_MAX);

 public:
  /**
   * @brief 指标构造函数
   */
  NoStopWhenNarrowRoad();
  /**
   * @brief 指标析构函数
   */
  virtual ~NoStopWhenNarrowRoad() {}

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
  static sim_msg::TestReport_XYPlot s_no_stop_when_narrow_road_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<NoStopWhenNarrowRoad>(); }
};
}  // namespace eval
