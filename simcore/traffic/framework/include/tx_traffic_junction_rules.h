// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_serialization.h"
#include "tx_signal_phase_period.h"
#include "tx_sim_time.h"
#include "tx_vehicle_element.h"
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/
TX_NAMESPACE_OPEN(Base)

// @brief 路口规则基类
class ITrafficJunctionRules {
 public:
  using SIGN_LIGHT_COLOR_TYPE = Base::ISignalPhasePeriod::SIGN_LIGHT_COLOR_TYPE;

 public:
  virtual ~ITrafficJunctionRules() TX_DEFAULT;

  /**
   * @brief 根据车辆的行驶距离、速度、安全距离和最大减速度，计算汽车应该使用的减速度。
   *
   * 此函数根据车辆行驶的距离、速度、安全距离和最大减速度，计算出一个合理的减速度值。
   *
   * @param Distance 车辆行驶的距离。单位为米。
   * @param Speed 车辆当前的速度。单位为米/秒。
   * @param Safe_Distance 安全距离。单位为米。
   * @param maxDeceleration 最大减速度。单位为米/秒^2。
   * @return 返回车辆应该使用的减速度。单位为米/秒^2。
   */
  virtual txFloat Deceleration(const txFloat Distance, const txFloat Speed, const txFloat Safe_Distance,
                               const txFloat maxDeceleration) const TX_NOEXCEPT {
    TX_MARK("在距离障碍还有Safe_Distance的地方停下来所需要的减速度 这里认为Distance是前bumper到前面的距离");
    if (Distance < Safe_Distance) {
      return maxDeceleration;
    }
    const Base::txFloat Dec = Speed * Speed / (-2 * (Distance - Safe_Distance));
    if (Dec < maxDeceleration) {
      TX_MARK("maxDeceleration = -10.0");
      TX_MARK("不超过最大减速度");
      return maxDeceleration;  //
    } else {
      return Dec;
    }
  }

  /**
   * @brief 检查信号灯的状态
   * @param singalId 信号灯ID
   * @param distance 离信号灯的距离
   * @param lightType 信号灯的颜色类型
   * @return 信号灯的状态，如果返回值为-1表示该信号灯未被触发，否则表示该信号灯触发了
   * @note 该函数的返回值为-1表示该信号灯未被触发，否则表示该信号灯触发了
   */
  virtual txFloat CheckSignLight(const txSysId singalId, const txFloat distance,
                                 const SIGN_LIGHT_COLOR_TYPE lightType) TX_NOEXCEPT = 0;

  /**
   * @brief 驾驶员在向左转时，对遇到的与行驶方向相反的交通元素进行操作
   * @param pOpposingVeh 指向交通元素的指针
   * @param distance2signal 驾驶员与交通元素之间的距离
   * @param acceleration 驾驶员所需的加速度
   * @return 返回驾驶员在向左转时，对遇到的与行驶方向相反的交通元素的操作结果
   */
  virtual txFloat TurnLeft_vs_GoStraight(const Base::IVehicleElementPtr pOpposingVeh, const txFloat distance2signal,
                                         const txFloat acceleration) const TX_NOEXCEPT = 0;

  /**
   * @brief 汽车向右转时，遇到方向相反的汽车时的操作结果
   * @param pOpposingVeh 指向相反方向的交通元素的指针
   * @param pLeftApproachingVeh 指向左侧逼近的交通元素的指针
   * @param distance2signal 汽车与相反方向交通元素之间的距离
   * @param acceleration 汽车所需的加速度
   * @return 返回汽车在向右转时，遇到方向相反的汽车时的操作结果
   */
  virtual txFloat TurnRight_vs_TurnLeft_GoStraight(const Base::IVehicleElementPtr pOpposingVeh,
                                                   const Base::IVehicleElementPtr pLeftApproachingVeh,
                                                   const txFloat distance2signal,
                                                   const txFloat acceleration) const TX_NOEXCEPT = 0;

 protected:
  const txInt Number_Of_Opposing_Lane = 1;
  const txFloat Critical_Gap_LeftTurn =
      4.5 + Number_Of_Opposing_Lane *
                0.5;  // 单位：秒。 对向直行车距离本左转车的时间距离,对向车道每多一条，则所需时间增加0.5秒
  const txFloat LYTOpposingBuffer =
      42;  // 单位：米。1230 HD before=25
           // 对向车过了对向停止线之后深入十字路口中心部分的距离;用于左转让直行时，直行车进入十字路口的距离
  const txFloat RYLOpposingBuffer =
      42;  // 单位：米。1230 HD before=30
           // 对向车过了对向停止线之后深入十字路口中心部分的距离;用于右转让左转时，左转车进入十字路口的距离
  const txFloat ApproachingBuffer =
      45;  // 单位：米。 左侧的直行车过了左侧停止线之后深入十字路口中心部分的距离 1230 HD before=25
  const txFloat LeftYielding_Reaction_Gap = 30;  // 单位：米。 本车(左转)在此范围内对对向直行的车进行避让反应
  const txFloat RightYielding_Reaction_Gap =
      35;  // 单位：米。 本车（右转）在此范围内对对向左转和左侧直行的车进行避让反应
  // int LeftTurnWaitingBuffer = pTraffic->LeftTurnWaitingBuffer();//单位：米。1230 HD before=20
  // 左转车过了停止线之后深入十字路口中心部分的距离，相当于待转区域 int RightTurnWaitingBuffer =
  // pTraffic->RightTurnWaitingBuffer();//单位：米。1230 HD before=20
  // 右转车过了停止线之后深入十字路口中心部分的距离，相当于待转区域
  // ////////////////////1230 HD/////////////////////////////////////////////////////
  const txFloat LeftTurnWaitingBuffer =
      FLAGS_LeftTurnWaitingBuffer;  // 单位：米。 左转车过了停止线之后深入十字路口中心部分的距离，相当于待转区域
  const txFloat RightTurnWaitingBuffer =
      FLAGS_RightTurnWaitingBuffer;  // 单位：米。 右转车过了停止线之后深入十字路口中心部分的距离，相当于待转区域

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("ITrafficJunctionRules");
  }
};

using ITrafficJunctionRulesPtr = std::shared_ptr<ITrafficJunctionRules>;

TX_NAMESPACE_CLOSE(Base)
#undef SerializeInfo
