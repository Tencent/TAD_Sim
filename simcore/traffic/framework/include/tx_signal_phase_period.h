// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_enum_def.h"
#include "tx_header.h"
TX_NAMESPACE_OPEN(Base)

class ISignalPhasePeriod {
 public:
  using SIGN_LIGHT_COLOR_TYPE = Base::Enums::SIGN_LIGHT_COLOR_TYPE;
  /*enum SIGN_LIGHT_COLOR_TYPE
  {
      SIGN_LIGHT_COLOR_GREEN = 0,
      SIGN_LIGHT_COLOR_YELLOW,
      SIGN_LIGHT_COLOR_RED,
      SIGN_LIGHT_COLOR_GREY,
      SIGN_LIGHT_COLOR_COUNT
  };*/

  struct InitParam_t {
    txFloat _start_t = 0.0;
    txFloat _period[3] = {0.0, 0.0, 0.0};
  };

 public:
  virtual ~ISignalPhasePeriod() TX_DEFAULT;

  /**
   * @brief 初始化
   *
   * 初始化函数，在调用其他函数之前需要先调用此函数进行初始化。
   *
   * @param initParam 初始化参数
   * @return 返回是否成功初始化的标志，如果成功则返回 true，否则返回 false
   */
  virtual txBool Initialize(const InitParam_t& initParam) TX_NOEXCEPT = 0;

  /**
   * @brief 计算颜色
   *
   * 该函数在传递的时间间隔内计算信号颜色。
   *
   * @param passTime 时间间隔
   * @return 如果计算成功，则返回 true，否则返回 false
   */
  virtual txBool ComputeColor(const txFloat passTime) TX_NOEXCEPT = 0;

  /**
   * @brief 更新信号阶段周期
   *
   * 更新信号阶段周期，并返回更新是否成功。
   *
   * @param timeStamp 时间戳
   * @return 更新成功返回 true，否则返回 false
   */
  virtual txBool Update(const txFloat timeStamp) TX_NOEXCEPT = 0;

  /**
   * @brief 获取信号阶段的颜色类型
   *
   * 获取信号阶段的颜色类型，包括红绿灯、亮绿、灭绿等。
   *
   * @return 返回信号阶段的颜色类型，具体见 SIGN_LIGHT_COLOR_TYPE 枚举类型
   */
  virtual SIGN_LIGHT_COLOR_TYPE GetPhase() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取下一个信号阶段的颜色
   *
   * 获取下一个信号阶段的颜色，包括红绿灯、亮绿、灭绿等。
   *
   * @return 返回下一个信号阶段的颜色，具体见 SIGN_LIGHT_COLOR_TYPE 枚举类型
   */
  virtual SIGN_LIGHT_COLOR_TYPE GetNextPhase() const TX_NOEXCEPT = 0;
};

TX_NAMESPACE_CLOSE(Base)
