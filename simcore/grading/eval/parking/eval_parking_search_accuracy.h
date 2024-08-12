// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (mama)
//
// 1. 接收到PARKING_STATE的msg不为空,并且PARKING_SPACE的msg不为空, 获取到主车处于"泊车搜索状态";
// 2. 分别对"垂直车位","平行车位","斜列车位"建立对应的"识别到的车位数"和"真值中实际存在的车位数"计数器;
// 3.
// 遍历PARKING_STATE中所有的停车位,根据未识别过的停车位的四个角在PARKING_SPACE中的真值停车位中查找是否存在,若存在,则步骤2中对应类型的车位计数器都加1,否则仅对"识别到的车位数"加1;
// 4. 取对应车位类型的计数器,"真值中实际存在的车位数" /
// "识别到的车位数"得到各个车位类型对应的识别准确率,对停车位整体的识别准确率与阈值进行比较;
// 5.在最终仿真结束时输出各个车位类型的识别准确率和总的车位识别准确率

#pragma once

#include "eval/eval_base.h"
#include "parking_state.pb.h"

namespace eval {
class EvalParkingSearchAccuracy : public EvalBase {
 private:
  /**
   * @brief 用下降沿检测模版类
   */
  FallingDetection<double> _detector = MakeFallingDetection<double>(1e6);
  std::vector<const sim_msg::Parking *> _parking_spaces;
  int _parallel_identified_num;
  int _vertical_identified_num;
  int _slanted_identified_num;
  int _parallel_total_num;
  int _vertical_total_num;
  int _slanted_total_num;

  double _parallel_accuracy;
  double _vertical_accuracy;
  double _slanted_accuracy;
  double _total_accuracy;
  bool isContainParkingSpace(const sim_msg::Parking *p);

 public:
  /**
   * @brief 指标构造函数
   */
  EvalParkingSearchAccuracy();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalParkingSearchAccuracy() {}

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

  // public:
  //  sim_msg::Parking getClosestParkingSpace(Eigen::Vector3d ego_vec, double radius = 100);

 public:
  /**
   * @brief 指标结果附加信息，评测报告用
   */
  static sim_msg::TestReport_PairData s_parking_search_parallel_accuracy_pair;
  static sim_msg::TestReport_PairData s_parking_search_vertical_accuracy_pair;
  static sim_msg::TestReport_PairData s_parking_search_slanted_accuracy_pair;
  static sim_msg::TestReport_PairData s_parking_search_total_accuracy_pair;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalParkingSearchAccuracy>(); }
};
}  // namespace eval
