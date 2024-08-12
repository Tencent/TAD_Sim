// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (mama)
//
// 1. 创建一个"已识别到车位集合",创建一个"真实存在的车位集合";
// 2. 接收到PARKING_STATE的msg不为空,且PARKING_SPACE的msg不为空,获取到主车处于"泊车搜索状态";
// 3. 获取到PARKING_STATE中识别到的车位,不重复的加入"已识别到车位集合";
// 4. 获取到PARKING_SPACE中的真实车位,不重复的加入"真实存在的车位集合";
// 5. 根据"已识别车位集合大小"/"真实存在的车位集合大小"获取识别成功率,并计算出漏识率,与阈值进行比较;
// 6. 最终仿真结束时输出"漏识率".

#pragma once

#include "eval/eval_base.h"
#include "parking_state.pb.h"

namespace eval {
class EvalParkingMissRate : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-1e6);

  std::vector<const sim_msg::Parking *> _parking_spaces;
  std::vector<const sim_msg::Parking *> _t_parking_spaces;
  double _search_range;
  double _parking_miss_rate;
  bool isContainParkingSpace(const sim_msg::Parking *p);
  bool isContainParkingSpaceById(const sim_msg::Parking *p);
  // bool searchTruthParkingSpaceWithRange(const sim_msg::Parking *p);

 public:
  /**
   * @brief 指标构造函数
   */
  EvalParkingMissRate();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalParkingMissRate() {}

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
  static sim_msg::TestReport_PairData s_parking_miss_pair;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalParkingMissRate>(); }
};
}  // namespace eval
