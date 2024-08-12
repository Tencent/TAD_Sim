// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_serialization.h"
TX_NAMESPACE_OPEN(Base)

class txCountDownTimer {
 public:
  txCountDownTimer() TX_DEFAULT;
  virtual ~txCountDownTimer() TX_DEFAULT;

  /**
   * @brief 初始化CountDownTimer类
   *
   * @param _id 初始化时使用的系统ID
   * @param rnd_factor 随机因子，用于影响countdown时间
   */
  virtual void Initialize(const Base::txSysId _id, const txFloat rnd_factor) TX_NOEXCEPT {
    mId = _id;
    mRndFactor = rnd_factor;
  }

  /**
   * @brief ResetCounter 重置计数器
   *
   * @param _laneKeep 车辆保持车道的值
   * @param _aggress 车辆进行随机车道变更的概率
   * @param bExitHighWay 是否在高速路段结束后退出高速路段，默认为 false
   */
  virtual void ResetCounter(const txFloat _laneKeep, const txFloat /*_aggress*/,
                            const txBool bExitHighWay = false) TX_NOEXCEPT {
    TX_MARK("(!bHurryToExitState) ? (LaneKeep + (1.0 - m_aggress)) : (LaneKeep / 2.0)");
    if (bExitHighWay) {
      mDuration = _laneKeep / 2.0;
    } else {
      mDuration = a() * _laneKeep + b() * (1.0 - mRndFactor);
    }
    // LOG(INFO) << "ResetCounter : " << TX_VARS(mId) << TX_VARS(Duration()) << TX_VARS(b() * (1.0 - mRndFactor));
  }

  /**
   * @brief 设置随机持续时间
   * @param[in] _d 新的随机持续时间
   */
  void SetRndDuration(const txFloat _d) TX_NOEXCEPT { mDuration = _d /** b()*/; }

  txFloat a() const TX_NOEXCEPT { return FLAGS_LK_A; }
  txFloat b() const TX_NOEXCEPT { return FLAGS_LK_B; }

  /**
   * @brief 获取持续时长
   *
   * @return txFloat
   */
  txFloat Duration() const TX_NOEXCEPT { return mDuration; }

  /**
   * @brief 计时器递减函数
   *
   * 对计时器的时长进行递减操作，同时返回递减后的时长值。
   * 递减操作由传入的参数 _time_step 指定，即每次递减的时长。
   * 当计时器的时长递减至 0 时，计时器被清空，并将时长恢复为初始值。
   *
   * @param _time_step 递减的时长值
   * @return 递减后的时长值
   */
  virtual txFloat Decrement(const txFloat _time_step) TX_NOEXCEPT {
    mDuration -= _time_step;
    return mDuration;
  }
  virtual txBool Expired() const TX_NOEXCEPT { return (mDuration <= 0.0); }

  /**
   * @brief 清除计时器数据
   *
   * 清除当前计时器的数据，使其恢复到初始状态。
   * 当前计时器的设定时长及随机因子都会被清除。
   *
   */
  virtual void Clear() TX_NOEXCEPT { mDuration = 0.0; /*mRndFactor = 0.5;*/ }

  /**
   * @brief 获取随机因子
   * @return 随机因子
   */
  virtual txFloat RndFactor() const TX_NOEXCEPT { return mRndFactor; }

 protected:
  txFloat mDuration = 0.0;
  txFloat mRndFactor = 0.5;
  Base::txSysId mId;

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("Duration", mDuration), _MAKE_NVP_("RndFactor", mRndFactor), _MAKE_NVP_("Id", mId));
  }
};

TX_NAMESPACE_CLOSE(Base)
