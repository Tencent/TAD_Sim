// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (mama)
//
// 1. 接收到PARKING_STATE的msg不为空,且接收到的PARKING_SPACE的msg不为空,获取到主车处于"泊车完成"状态;
// 2. 根据给定的停车位ID获取到PARKING_SPACE中的真值停车位信息;
// 3. 根据停车位的长和宽判断车位的横纵方向;
// 4. 分别取左前/左后车轮距左侧车位线的距离与对应的阈值进行比较;
// 5. 分别取右前/右后车轮距右侧车位线的距离与对应的距离进行比较
// 6. 取两个前车轮中点距前方车位线的距离与阈值进行比较,取两个后车轮中点距后方车位线的距离与阈值进行比较;
// 7. 取泊车完成后,主车车身与停车位的摆角与对应阈值进行比较.
// 8. 更新步骤4/5/6/7中涉及距离和角度的最大值;
// 9. 最终仿真结束时,输出每个距离和角度的最大值.

#pragma once

#include "eval/eval_base.h"

namespace eval {
class EvalParkingPrecision : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector_fl = MakeRiseUpDetection<double>(-1e6);
  RiseUpDetection<double> _detector_fr = MakeRiseUpDetection<double>(-1e6);
  RiseUpDetection<double> _detector_rl = MakeRiseUpDetection<double>(-1e6);
  RiseUpDetection<double> _detector_rr = MakeRiseUpDetection<double>(-1e6);
  RiseUpDetection<double> _detector_f = MakeRiseUpDetection<double>(-1e6);
  RiseUpDetection<double> _detector_r = MakeRiseUpDetection<double>(-1e6);
  RiseUpDetection<double> _detector_a = MakeRiseUpDetection<double>(-1e6);

  int32_t _parking_id;
  double _lateral_fl_dist;
  double _lateral_fr_dist;
  double _lateral_rl_dist;
  double _lateral_rr_dist;
  double _vertical_f_dist;
  double _vertical_r_dist;
  double _lateral_angle;

  double _lateral_thresh_value;
  double _vertical_thresh_value;
  double _angle_thresh_value;
  int _detect_count;
  // std::shared_ptr<sim_msg::Parking> _parking;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalParkingPrecision();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalParkingPrecision() {}

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
  // bool InParkingSpace(Eigen::Vector3d corner, std::shared_ptr<sim_msg::Parking> parking_ptr);

 public:
  /**
   * @brief 指标结果附加信息，评测报告用
   */
  static sim_msg::TestReport_PairData s_parking_lateral_fl_dist_pair;
  static sim_msg::TestReport_PairData s_parking_lateral_fr_dist_pair;
  static sim_msg::TestReport_PairData s_parking_lateral_rl_dist_pair;
  static sim_msg::TestReport_PairData s_parking_lateral_rr_dist_pair;
  static sim_msg::TestReport_PairData s_parking_vertical_f_pair;
  static sim_msg::TestReport_PairData s_parking_vertical_r_pair;
  static sim_msg::TestReport_PairData s_parking_body_angle_pair;

  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalParkingPrecision>(); }
};
}  // namespace eval
