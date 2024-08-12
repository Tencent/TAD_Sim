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

// @brief 行驶跟随接口
class IDrivingFollower {
 public:
  enum class DrivingFollowType : txInt { e99 = 0, e74 = 1 };
  enum class DRIVING_FOLLOW_STATE : txInt { N = 0, A = 1, B = 2, F = 3, W = 4 };

  // @brief 跟随状态结构
  struct follower_status {
    Base::txFloat dx = 0.0;
    Base::txFloat dv = 0.0;
    Base::txFloat sdxc = 0.0;
    Base::txFloat sdxv = 0.0;
    Base::txFloat sdxo = 0.0;
    Base::txFloat sdvc = 0.0;
    Base::txFloat sdvo = 0.0;
    DRIVING_FOLLOW_STATE status = DRIVING_FOLLOW_STATE::N;
    DRIVING_FOLLOW_STATE code = DRIVING_FOLLOW_STATE::N;
    friend std::ostream& operator<<(std::ostream& os, const follower_status& v) TX_NOEXCEPT {
      os << "{" << TX_VARS_NAME(dx, v.dx) << ", " << TX_VARS_NAME(dv, v.dv) << ", " << TX_VARS_NAME(sdxc, v.sdxc)
         << ", " << TX_VARS_NAME(sdxv, v.sdxv) << ", " << TX_VARS_NAME(sdxo, v.sdxo) << ", "
         << TX_VARS_NAME(sdvc, v.sdvc) << ", " << TX_VARS_NAME(sdvo, v.sdvo) << ", "
         << TX_VARS_NAME(status, Utils::to_underlying(v.status)) << ", "
         << TX_VARS_NAME(code, Utils::to_underlying(v.code)) << "}";
      return os;
    }
  };

  /**
   * @brief 获取跟随状态
   *
   * @param _c 枚举
   * @return Base::txString
   */
  static Base::txString toString(const DRIVING_FOLLOW_STATE _c) TX_NOEXCEPT {
    switch (_c) {
      case DRIVING_FOLLOW_STATE::N:
        return "N";
      case DRIVING_FOLLOW_STATE::A:
        return "A";
      case DRIVING_FOLLOW_STATE::B:
        return "B";
      case DRIVING_FOLLOW_STATE::F:
        return "F";
      case DRIVING_FOLLOW_STATE::W:
        return "W";
    }
    return "unknow";
  }

 public:
  virtual ~IDrivingFollower() TX_DEFAULT;

  /**
   * @brief 行驶跟随
   *
   * @param timeMgr 时间管理器
   * @return txFloat
   */
  virtual txFloat DrivingFollow(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;
  virtual void ConfigureFollowStrategy() TX_NOEXCEPT {
    if (FLAGS_Use_Arterial) {
      mDrivingFollowStrategy = DrivingFollowType::e74;
    }
  }

 protected:
  DrivingFollowType mDrivingFollowStrategy = DrivingFollowType::e99;
  follower_status m_follower_status;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("IDrivingFollower");
    archive(_MAKE_NVP_("DrivingFollowStrategy", mDrivingFollowStrategy));
  }
};
using IDrivingFollowerPtr = std::shared_ptr<IDrivingFollower>;

TX_NAMESPACE_CLOSE(Base)
#undef SerializeInfo
