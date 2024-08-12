// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_signal_phase_period.h"
TX_NAMESPACE_OPEN(Scene)

class TAD_SignalPhasePeriod : public Base::ISignalPhasePeriod {
 public:
  /**
   * @brief 信号相位初始化
   *
   * @param initParam 初始化参数
   * @return Base::txBool 初始化是否成功
   */
  virtual Base::txBool Initialize(const InitParam_t& initParam) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief ComputeColor 计算颜色
   *
   * @param passTime 单位为秒的浮点数
   * @return Base::txBool 返回计算结果，成功时返回true，失败时返回false
   */
  virtual Base::txBool ComputeColor(const Base::txFloat passTime) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief TAD_SignalPhasePeriod::Update 更新信号周期的阶段
   *
   * @param timeStamp 当前时间戳，单位为秒
   * @return Base::txBool 更新是否成功，成功为 true，失败为 false
   *
   * 根据给定的时间戳，更新信号周期的阶段。
   */
  virtual Base::txBool Update(const Base::txFloat timeStamp) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief TAD_SignalPhasePeriod::GetPhase 获取信号相位类型
   * @return SIGN_LIGHT_COLOR_TYPE 信号相位类型
   *
   * 获取当前信号相位类型。
   */
  virtual SIGN_LIGHT_COLOR_TYPE GetPhase() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取下一个信号阶段的颜色类型
   *
   * @return SIGN_LIGHT_COLOR_TYPE 下一个信号阶段的颜色类型
   *
   * 根据当前的信号阶段及时间，获取下一个信号阶段的颜色类型。
   */
  virtual SIGN_LIGHT_COLOR_TYPE GetNextPhase() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取信号周期
   * @return Base::txInt 信号周期
   *
   */
  virtual Base::txInt GetAge() const TX_NOEXCEPT { return _age[0]; }

  /**
   * @brief 获取下一个信号周期
   *
   * @return Base::txInt 信号周期
   */
  virtual Base::txInt GetNextAge() const TX_NOEXCEPT { return _age[1]; }

  /**
   * @brief 获取字符串表示
   *
   * @return Base::txString
   */
  virtual Base::txString Str() const TX_NOEXCEPT;

 protected:
  struct ColorInterval_t {
    ColorInterval_t(const Base::txFloat l = 0.0, const Base::txFloat r = 0.0,
                    const SIGN_LIGHT_COLOR_TYPE c = SIGN_LIGHT_COLOR_TYPE::eGreen)
        : leftTimeClosed(l), rightTimeOpen(r), color(c) {}
    Base::txFloat leftTimeClosed;
    Base::txFloat rightTimeOpen;
    SIGN_LIGHT_COLOR_TYPE color;

    /**
     * @brief Comparision 比较时间段内的时间。
     * 判断给定的时间是否在给定的时间段内。
     *
     * @param timeInterval 给定的时间段
     * @param passTime 要比较的时间
     * @return true 时间在时间段内，false 时间不在时间段内
     */
    static bool Comparision(const ColorInterval_t& timeInterval, const Base::txFloat passTime) TX_NOEXCEPT {
      if (timeInterval.leftTimeClosed <= passTime && passTime < timeInterval.rightTimeOpen) {
        return true;
      } else {
        return false;
      }
    }
  };

 protected:
  Base::txFloat _start_t = 0.0;
  Base::txFloat _period[3] = {0.0, 0.0, 0.0};
  Base::txFloat _totalPeriod = 0.0;
  SIGN_LIGHT_COLOR_TYPE _current_Phase[2];
  std::vector<ColorInterval_t> _colorIntervalVec;
  Base::txInt _age[2];
  /*SIGN_LIGHT_COLOR_TYPE _next_Phase = SIGN_LIGHT_COLOR_TYPE::eGreen;
  Base::txInt _next_age = 0;*/
};

TX_NAMESPACE_CLOSE(Scene)
