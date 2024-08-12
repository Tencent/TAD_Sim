// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_serialization.h"
#include "tx_sim_time.h"
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/
TX_NAMESPACE_OPEN(Base)

// @brief 行驶换道接口
class IDrivingChangeLane {
 public:
  enum class LaneChangeIntention : txInt { eYes = 0, eNo = 1 };
  enum class PedestrianSearchType : txInt { eFront = 0, eLeft = 1, eRight = 2 };
  enum class ExitHighWayType : txInt { eFollower = 0, eAcc = 1, eDec = 2 };
  friend std::ostream& operator<<(std::ostream& os, const PedestrianSearchType& v) TX_NOEXCEPT {
    switch (v) {
      case PedestrianSearchType::eFront: {
        os << "{PedestrianSearchType : eFront}";
        break;
      }
      case PedestrianSearchType::eLeft: {
        os << "{PedestrianSearchType : eLeft}";
        break;
      }
      case PedestrianSearchType::eRight: {
        os << "{PedestrianSearchType : eRight}";
        break;
      }
    }
    return os;
  }

 public:
  virtual ~IDrivingChangeLane() TX_DEFAULT;

  /**
   * @brief 行驶换道
   *
   * @param timeMgr
   * @return txFloat
   */
  virtual txFloat DrivingChangeLane(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;

  /**
   * @brief 获取换道时长
   *
   * @return txFloat 换道时长
   */
  virtual txFloat LcDuration() const TX_NOEXCEPT { return mLcDuration; }

  /**
   * @brief 设置换道时长
   *
   * @param duration 设置的换道时长
   * @return txFloat 返回设置过的换道时长
   */
  virtual txFloat SetLcDuration(const txFloat duration) TX_NOEXCEPT {
    mLcDuration = duration;
    return LcDuration();
  }

 protected:
  txFloat mLcDuration = 2.0;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("IDrivingChangeLane");
    archive(_MAKE_NVP_("LcDuration", mLcDuration));
  }
};

using IDrivingChangeLanePtr = std::shared_ptr<IDrivingChangeLane>;

TX_NAMESPACE_CLOSE(Base)
#undef SerializeInfo
