// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: (dongyuanhu)
//
// 一/主车与机动车碰撞
// 追尾碰撞:
// 在(主车v＞0,后退)或(主车v＝0)情况下:
// 1．主车和交通车的夹角[-75,0],主车右前角或(交通右后角)碰撞(-1.308996,0)
// 2．主车和交通车夹角[0,75],主车左前角或(交通左后角)碰撞
//
// 被追尾碰撞:
// 在(主车v＞0,前进)或(主车v＝0)情况下:
// 1．判断主车和交通车的夹角[-75,0],主车左后角或(交通左前角)碰撞
// 2．主车和交通车夹角[0,75],主车右后角或(交通右前角)碰撞
//
// 垂直碰撞:
// 在(主车v＞＝0,交通车v＞0,前进)情况下:
// 1．主车与交通车夹角[75,90],主车右后角或(交通前角)碰撞(1.308996,1.570796)
// 2．主车与交通车夹角[90,105],(交通右前角)碰撞(1.570796,1.832595)
// 3．主车与交通车夹角[-90,-75],主车左后角或(交通前角)碰撞
// 4．主车与交通车夹角[-105,-90],(交通左前角)碰撞
// 在(主车v＞＝0,交通车v＞0,后退)情况下:
// 1．主车与交通车夹角[75,90],主车左前角或(交通后角)碰撞
// 2．主车与交通车夹角[90,105],主车右前角或(交通前角)碰撞
// 3．主车与交通车夹角[-90,-75],(交通右前角)碰撞
// 4．主车与交通车夹角[-105,-90],主车左前角或(交通前角)碰撞
//
// 正面碰撞:
// 在(主车v＞0,后退)(v＝0)情况下:
// 1．主车与交通车夹角[105,180],主车右前角或(交通左前角)碰撞(1.832595,3.141592)
// 2．主车与交通车夹角[-180,-105],主车左前角或(交通右前角)碰撞
//
// 斜角碰撞:
// 在(主车v＞＝0,交通车v＞0,前进)情况下:
// 1．主车与交通车夹角[0,75],交通左前角碰撞
// 2．主车与交通车夹角[105,180],交通左后角或右前角碰撞
// 3．主车与交通车夹角[-75,0],交通右前角碰撞
// 4．主车与交通车夹角[-180,-105],交通左前角或右后角碰撞
// 在(主车v＞＝0,交通车v＞0,后退)情况下:
// 1．主车与交通车夹角[0,75],交通右后角碰撞
// 2．主车与交通车夹角[105,180],交通左后角或右前角碰撞
// 3．主车与交通车夹角[-75,0],交通左后角碰撞
// 4．主车与交通车夹角[-180,-105],交通左前角或右后角碰撞
//
// 二/主车与行人碰撞
// 在(主车v＞＝0,后退)的情况下:
// 1．发生碰撞时,主车的左右前角碰撞(主车v＞＝0,前进)
// 2．发生碰撞时,主车的左右后角在行人轮廓内,视为主动碰撞.
//
// 三/主车与非机动车碰撞
// 在(主车v＞0,前进)(主车v＝0)情况下:
// 1．主车的左或右后角在非机动车轮廓内,视为被动碰撞
// 2．主车与非机动车的夹角[-90,90],若非机动车的左或右前角发生碰撞,则视为被动碰撞
// 在(主车v＞0,后退)(主车v＝0)情况下:
// 1．主车的左或右前角在非机动车轮廓内,视为被动碰撞
// 2．主车与非机动车夹角[-180,-90],[90,180],机动车的左或右前角发生碰撞,视为被动碰撞
//
// 特别说明:
// 主车与非机动车发生前进行驶追尾/或倒车时碰撞,无法使用主车四角验证碰撞的发生具体情况,而是使用非机动车的四角是否在主车轮廓内进行判断
//

#pragma once

#include "eval/eval_base.h"

namespace eval {

class EvalCollisionPassive : public EvalBase {
 private:
  /**
   * @brief 用上升沿检测模版类
   */
  RiseUpDetection<double> _detector = MakeRiseUpDetection<double>(-1e6);
  std::string _collision_type;
  bool _event_collision;
  bool is_collision;

 public:
  /**
   * @brief 指标构造函数
   */
  EvalCollisionPassive();
  /**
   * @brief 指标析构函数
   */
  virtual ~EvalCollisionPassive() {}

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
  bool CheckPointInPolygon(const eval::RectCorners &corners, Eigen::Vector3d &point);

 public:
  /**
   * @brief 指标plot图定义，评测报告用
   */
  static sim_msg::TestReport_XYPlot _s_passive_collision_plot;
  /**
   * @brief 指标名称定义，在factory中注册时的参数，且与json中"name"一一对应
   */
  static const char _kpi_name[];
  /**
   * @brief  指标Build方法，在factory中注册时传入的方法
   * @return EvalBasePtr 指标指针
   */
  static EvalBasePtr Build() { return std::make_shared<EvalCollisionPassive>(); }
};
}  // namespace eval
