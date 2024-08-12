// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_math.h"
#include "tx_serialization.h"
TX_NAMESPACE_OPEN(TrafficFlow)

struct TAD_BehaviorTimeManager {
 public:
  enum class DurationType { Undefine, Normal, Short };

 public:
  /**
   * @brief 获取当前行为持续时间
   *
   * @return Base::txFloat
   */
  Base::txFloat Duration() const TX_NOEXCEPT { return m_Duration; }
  /**
   * @brief 获取当前行为执行进度
   *
   * @return Base::txFloat
   */
  Base::txFloat Procedure() const TX_NOEXCEPT { return m_Procedure; }
  /**
   * @brief 获取当前行为最小取消时间
   *
   * @return Base::txFloat
   */
  Base::txFloat Min_Cancel_Duration() const TX_NOEXCEPT { return m_Min_Cancel_Duration; }
  /**
   * @brief 设置当前行为持续时间
   *
   * @param _dr 持续时间
   */
  void SetDuration(Base::txFloat const _dr) TX_NOEXCEPT { m_Duration = _dr; }
  /**
   * @brief 设置当前行为执行进度
   *
   * @param _pd 执行进度
   */
  void SetProcedure(Base::txFloat const _pd) TX_NOEXCEPT { m_Procedure = _pd; }
  /**
   * @brief 设置当前行为最小取消时间
   *
   * @param _min_cancel_d 最小取消时间
   */
  void SetMinCancelDuration(Base::txFloat _min_cancel_d) TX_NOEXCEPT { m_Min_Cancel_Duration = _min_cancel_d; }

  Base::txFloat Step(Base::txFloat const _interval) TX_NOEXCEPT {
    m_Procedure -= _interval;
    if (Math::isZero(m_Procedure, 1e-9) || m_Procedure < 0.0 TX_MARK("400ms")) {
      m_Procedure = 0.0;
    }

    return m_Procedure;
  }
  Base::txFloat Experience() const TX_NOEXCEPT { return (m_Duration - m_Procedure); }

  /**
   * @brief 判断是否是未定义的时长类型
   *
   * @return Base::txBool
   */
  Base::txBool IsUndefineType() const TX_NOEXCEPT { return DurationType::Undefine == m_DurationType; }
  /**
   * @brief 判断是否Normal类型
   *
   * @return Base::txBool
   */
  Base::txBool IsNormalType() const TX_NOEXCEPT { return DurationType::Normal == m_DurationType; }
  /**
   * @brief 判断是否Short类型
   *
   * @return Base::txBool
   */
  Base::txBool IsShortType() const TX_NOEXCEPT { return DurationType::Short == m_DurationType; }
  /**
   * @brief 设置当前行为时长类型
   *
   * @param _type
   */
  void SetDurationType(DurationType const _type) TX_NOEXCEPT { m_DurationType = _type; }

  /**
   * @brief 获取换道时间的平均值
   *
   * @return Base::txFloat
   */
  Base::txFloat Average_LaneChanging_Duration() const TX_NOEXCEPT { return FLAGS_Average_LaneChanging_Duration; }
  /**
   * @brief 获取换道时间的方差
   *
   * @return Base::txFloat
   */
  Base::txFloat Variance_LaneChanging_Duration() const TX_NOEXCEPT { return FLAGS_Variance_LaneChanging_Duration; }
  // Function to get the average lane changing duration for short distances
  Base::txFloat Average_LaneChanging_Duration_Short() const TX_NOEXCEPT {
    return FLAGS_Average_LaneChanging_Duration_Short;
  }
  /**
   * @brief Function to get the variance of lane changing duration for short distances
   *
   * @return Base::txFloat
   */
  Base::txFloat Variance_LaneChanging_Duration_Short() const TX_NOEXCEPT {
    return FLAGS_Variance_LaneChanging_Duration_Short;
  }

  /**
   * @brief SetChangeInLaneAction sets the flag that indicates whether the vehicle is allowed to change lanes
   *
   * @param flag 是否允许换道
   */
  void SetChangeInLaneAction(Base::txBool flag) TX_NOEXCEPT { m_ChangeInLane = flag; }
  /**
   * @brief isChangeInLane returns the current state of the ChangeInLane flag
   *
   * @return Base::txBool 获取是否允许换道
   */
  Base::txBool isChangeInLane() const TX_NOEXCEPT { return m_ChangeInLane; }
  /**
   * @brief SetChangeInLaneOffset sets the change in lane offset
   *
   * @param _offset
   */
  void SetChangeInLaneOffset(const Base::txFloat _offset) TX_NOEXCEPT {
    m_ChangeInLaneOffset = _offset;
    m_ChangeInLaneOffsetProcedure = m_ChangeInLaneOffset;
  }
  /**
   * @brief GetChangeInLaneOffset returns the current change in lane offset
   *
   * @return Base::txFloat
   */
  Base::txFloat GetChangeInLaneOffset() const TX_NOEXCEPT { return m_ChangeInLaneOffset; }
  /**
   * @brief GetChangeInLaneOffsetProcedure returns the current change in lane offset procedure
   *
   * @return Base::txFloat
   */
  Base::txFloat GetChangeInLaneOffsetProcedure() const TX_NOEXCEPT { return m_ChangeInLaneOffsetProcedure; }
  /**
   * @brief ChangeInLaneOffsetProcedure adjusts the change in lane offset procedure by a given step
   *
   * @param _step
   * @return Base::txFloat
   */
  Base::txFloat ChangeInLaneOffsetProcedure(const Base::txFloat _step) TX_NOEXCEPT {
    m_ChangeInLaneOffsetProcedure -= _step;
    if (m_ChangeInLaneOffsetProcedure < 0.0) {
      m_ChangeInLaneOffsetProcedure = 0.0;
    }
    LOG(WARNING) << TX_VARS(m_ChangeInLaneOffsetProcedure) << TX_VARS(_step);
    return m_ChangeInLaneOffsetProcedure;
  }

 protected:
  DurationType m_DurationType = DurationType::Undefine;
  Base::txFloat m_Duration = 0.0;
  Base::txFloat m_Procedure = 0.0;
  Base::txFloat m_Min_Cancel_Duration = 0.0;
  Base::txBool m_ChangeInLane = false;
  Base::txFloat m_ChangeInLaneOffset;
  Base::txFloat m_ChangeInLaneOffsetProcedure;

 public:
  friend std::ostream& operator<<(std::ostream& os, const DurationType& v) TX_NOEXCEPT {
    switch (v) {
      case DurationType::Undefine: {
        os << "Undefine";
        break;
      }
      case DurationType::Normal: {
        os << "Normal";
        break;
      }
      case DurationType::Short: {
        os << "Short";
        break;
      }
    }
    return os;
  }
  friend std::ostream& operator<<(std::ostream& os, const TAD_BehaviorTimeManager& v) TX_NOEXCEPT {
    os << "BehaviorTimeManager {" << TX_VARS(v.m_DurationType) << TX_VARS(v.m_Duration) << TX_VARS(v.m_Procedure)
       << TX_VARS(v.m_Min_Cancel_Duration) << TX_COND_NAME(ChangeInLane, v.m_ChangeInLane) << "}";
    return os;
  }

 public:
  /**
   * @brief 序列化模板函数
   *
   * @tparam Archive 类型
   * @param archive 目标对象
   */
  template <class Archive>
  void serialize(Archive& archive) {
    archive(m_DurationType);
    archive(m_Duration);
    archive(m_Procedure);
    archive(m_Min_Cancel_Duration);
    archive(m_ChangeInLane);
    archive(m_ChangeInLaneOffset);
    archive(m_ChangeInLaneOffsetProcedure);
  }
};

TX_NAMESPACE_CLOSE(TrafficFlow)
