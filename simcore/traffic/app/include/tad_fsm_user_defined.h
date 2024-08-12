// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_serialization.h"

TX_NAMESPACE_OPEN(TrafficFlow)
TX_NAMESPACE_OPEN(SM)
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_UserDefine_StateMachine)
#define LogWarn LOG(WARNING)
#define TXST_STATE_CHANGE(_ID_, _FROM_STATE_, _TO_STATE_)                                                \
  " Id = " << _ID_ << ", From " << (__enum2lpsz__(TAD_VehicleState_UserDefined, _FROM_STATE_)) << " To " \
           << (__enum2lpsz__(TAD_VehicleState_UserDefined, _TO_STATE_));
// #define SM_ASSERT(_Condition_, _Msg_) do { if (CallFail(_Condition_)) { LOG(WARNING) << _Msg_; system("pause"); } }
// while (false)
#define SM_ASSERT(_Condition_, _Msg_)
class txUserDefinedState {
  using StateType = Base::Enums::TAD_VehicleState_UserDefined;
#if 0
    eIdle = 0, eStart, eStop, eKill,
        eLaneKeep,
        eTurnLeft_Start, eTurnLeft_Ing, eTurnLeft_Finish, eTurnLeft_Abort,
        eTurnLeft_InLane_Start, eTurnLeft_InLane_Ing, eTurnLeft_InLane_Finish, eTurnLeft_InLane_Abort,
        eTurnRight_Start, eTurnRight_Ing, eTurnRight_Finish, eTurnRight_Abort,
        eTurnRight_InLane_Start, eTurnRight_InLane_Ing, eTurnRight_InLane_Finish, eTurnRight_InLane_Abort,
        eLateral_Action
#endif

 public:
  txUserDefinedState() TX_DEFAULT;
  virtual ~txUserDefinedState() TX_DEFAULT;

  /**
   * @brief 初始化
   *
   * @param _userdefined_vehicle_id 车辆ID
   * @return Base::txBool
   */
  virtual Base::txBool Initialize(const Base::txSysId _userdefined_vehicle_id) TX_NOEXCEPT {
    mId = _userdefined_vehicle_id;
    return true;
  }

  /**
   * @brief 获取vehicle id
   *
   * @return Base::txSysId
   */
  Base::txSysId VehicleId() const TX_NOEXCEPT { return mId; }

  /**
   * @brief 设置启动状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetStart() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eIdle)):
      case (_plus_(StateType::eStart)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eStart)));
        mState = StateType::eStart;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 设置lane keep状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetLaneKeep() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eStart)):
      case (_plus_(StateType::eLaneKeep)):
      case (_plus_(StateType::eTurnLeft_Abort)):
      case (_plus_(StateType::eTurnLeft_Finish)):
      case (_plus_(StateType::eTurnRight_Abort)):
      case (_plus_(StateType::eTurnRight_Finish)):
      case (_plus_(StateType::eTurnLeft_InLane_Abort)):
      case (_plus_(StateType::eTurnLeft_InLane_Finish)):
      case (_plus_(StateType::eTurnRight_InLane_Abort)):
      case (_plus_(StateType::eTurnRight_InLane_Finish)):
      case (_plus_(StateType::eLateral_Action)): {
        // LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (+StateType::eLaneKeep));
        mState = StateType::eLaneKeep;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 开始左转
   *
   * @return Base::txBool
   */
  virtual Base::txBool StartTurnLeft() TX_NOEXCEPT { return SetTurnLeft_Start(); }

  /**
   * @brief 设置左转开始状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnLeft_Start() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eLaneKeep)):
      case (_plus_(StateType::eTurnLeft_Start)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnLeft_Start)));
        mState = StateType::eTurnLeft_Start;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 设置左转进行中状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnLeft_Ing() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnLeft_Start)):
      case (_plus_(StateType::eTurnLeft_Ing)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnLeft_Ing)));
        mState = StateType::eTurnLeft_Ing;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 设置左转中止状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnLeft_Abort() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnLeft_Start)):
      case (_plus_(StateType::eTurnLeft_Abort)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnLeft_Abort)));
        mState = StateType::eTurnLeft_Abort;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 设置左转结束状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnLeft_Finish() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnLeft_Ing)):
      case (_plus_(StateType::eTurnLeft_Finish)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnLeft_Finish)));
        mState = StateType::eTurnLeft_Finish;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 开始右转
   *
   * @return Base::txBool
   */
  virtual Base::txBool StartTurnRight() TX_NOEXCEPT { return SetTurnRight_Start(); }

  /**
   * @brief 设置右转开始状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnRight_Start() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eLaneKeep)):
      case (_plus_(StateType::eTurnRight_Start)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnRight_Start)));
        mState = StateType::eTurnRight_Start;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief set右转进行中状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnRight_Ing() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnRight_Start)):
      case (_plus_(StateType::eTurnRight_Ing)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnRight_Ing)));
        mState = StateType::eTurnRight_Ing;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief set右转中止状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnRight_Abort() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnRight_Start)):
      case (_plus_(StateType::eTurnRight_Abort)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnRight_Abort)));
        mState = StateType::eTurnRight_Abort;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief set右转结束状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnRight_Finish() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnRight_Ing)):
      case (_plus_(StateType::eTurnRight_Finish)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnRight_Finish)));
        mState = StateType::eTurnRight_Finish;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 开始车道内左转向
   *
   * @return Base::txBool
   */
  virtual Base::txBool StartTurnLeftInLane() TX_NOEXCEPT { return SetTurnLeft_InLane_Start(); }

  /**
   * @brief set车道内左转开始状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnLeft_InLane_Start() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eLaneKeep)):
      case (_plus_(StateType::eTurnLeft_InLane_Start)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnLeft_InLane_Start)));
        mState = StateType::eTurnLeft_InLane_Start;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief set车道内左转进行中状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnLeft_InLane_Ing() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnLeft_InLane_Start)):
      case (_plus_(StateType::eTurnLeft_InLane_Ing)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnLeft_InLane_Ing)));
        mState = StateType::eTurnLeft_InLane_Ing;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief set车道内左转中止状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnLeft_InLane_Abort() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnLeft_InLane_Start)):
      case (_plus_(StateType::eTurnLeft_InLane_Abort)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnLeft_InLane_Abort)));
        mState = StateType::eTurnLeft_InLane_Abort;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 设置车道内左转结束状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnLeft_InLane_Finish() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnLeft_InLane_Ing)):
      case (_plus_(StateType::eTurnLeft_InLane_Finish)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnLeft_InLane_Finish)));
        mState = StateType::eTurnLeft_InLane_Finish;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 开始车道内右转向
   *
   * @return Base::txBool
   */
  virtual Base::txBool StartTurnRightInLane() TX_NOEXCEPT { return SetTurnRight_InLane_Start(); }

  /**
   * @brief 设置车道内右转向开始状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnRight_InLane_Start() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eLaneKeep)):
      case (_plus_(StateType::eTurnRight_InLane_Start)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnRight_InLane_Start)));
        mState = StateType::eTurnRight_InLane_Start;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 设置车道内右转向进行中状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnRight_InLane_Ing() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnRight_InLane_Start)):
      case (_plus_(StateType::eTurnRight_InLane_Ing)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnRight_InLane_Ing)));
        mState = StateType::eTurnRight_InLane_Ing;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 设置车道内右转向中止状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnRight_InLane_Abort() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnRight_InLane_Start)):
      case (_plus_(StateType::eTurnRight_InLane_Abort)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnRight_InLane_Abort)));
        mState = StateType::eTurnRight_InLane_Abort;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 设置车道内右转向结束状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnRight_InLane_Finish() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eTurnRight_InLane_Ing)):
      case (_plus_(StateType::eTurnRight_InLane_Finish)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eTurnRight_InLane_Finish)));
        mState = StateType::eTurnRight_InLane_Finish;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 获取当前状态
   *
   * @return StateType
   */
  virtual StateType State() const TX_NOEXCEPT { return mState; }

  /**
   * @brief 返回当前状态是否为 Lateral Action（向左行动）
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInLateralAction() const TX_NOEXCEPT { return (_plus_(StateType::eLateral_Action) == State()); }

  /**
   * @brief 是否处于laneChange状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInLaneChange() const TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnLeft_Start:
      case StateType::eTurnLeft_Ing:
      case StateType::eTurnLeft_Finish:
      case StateType::eTurnLeft_Abort:

      case StateType::eTurnRight_Start:
      case StateType::eTurnRight_Ing:
      case StateType::eTurnRight_Finish:
      case StateType::eTurnRight_Abort:

      case StateType::eTurnLeft_InLane_Start:
      case StateType::eTurnLeft_InLane_Ing:
      case StateType::eTurnLeft_InLane_Finish:
      case StateType::eTurnLeft_InLane_Abort:

      case StateType::eTurnRight_InLane_Start:
      case StateType::eTurnRight_InLane_Ing:
      case StateType::eTurnRight_InLane_Finish:
      case StateType::eTurnRight_InLane_Abort: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 是否处于TurnLeft状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInTurnLeft() const TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnLeft_Start:
      case StateType::eTurnLeft_Ing:
      case StateType::eTurnLeft_Finish:
      case StateType::eTurnLeft_Abort: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 是否处于TurnRight状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInTurnRight() const TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnRight_Start:
      case StateType::eTurnRight_Ing:
      case StateType::eTurnRight_Finish:
      case StateType::eTurnRight_Abort: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 是否处于车道内左转向状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInTurnLeft_InLane() const TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnLeft_InLane_Start:
      case StateType::eTurnLeft_InLane_Ing:
      case StateType::eTurnLeft_InLane_Finish:
      case StateType::eTurnLeft_InLane_Abort: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 是否处于车道内右转向状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInTurnRight_InLane() const TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnRight_InLane_Start:
      case StateType::eTurnRight_InLane_Ing:
      case StateType::eTurnRight_InLane_Finish:
      case StateType::eTurnRight_InLane_Abort: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 是否处于车道内左转向或右转向状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInTurn_LeftRight_InLane() const TX_NOEXCEPT {
    return (IsInTurnRight_InLane() || IsInTurnLeft_InLane());
  }

  /**
   * @brief 是否处于左转或右转向状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInTurn_LeftRight() const TX_NOEXCEPT { return (IsInTurnRight() || IsInTurnLeft()); }

  /**
   * @brief 是否处于转向中止状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInTurn_Abort() const TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnRight_InLane_Abort:
      case StateType::eTurnRight_Abort:
      case StateType::eTurnLeft_InLane_Abort:
      case StateType::eTurnLeft_Abort: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 是否处于转向开始状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInTurn_Start() const TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnRight_InLane_Start:
      case StateType::eTurnRight_Start:
      case StateType::eTurnLeft_InLane_Start:
      case StateType::eTurnLeft_Start: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 从start状态到ing状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool ChaneFromStartToIng() TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnRight_Start: {
        return SetTurnRight_Ing();
      }
      case StateType::eTurnRight_InLane_Start: {
        return SetTurnRight_InLane_Ing();
      }
      case StateType::eTurnLeft_Start: {
        return SetTurnLeft_Ing();
      }
      case StateType::eTurnLeft_InLane_Start: {
        return SetTurnLeft_InLane_Ing();
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 是否处于转向中状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInTurn_Ing() const TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnRight_InLane_Ing:
      case StateType::eTurnRight_Ing:
      case StateType::eTurnLeft_InLane_Ing:
      case StateType::eTurnLeft_Ing: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 从ing状态到finish状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool ChaneFromIngToFinish() TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnRight_Ing: {
        return SetTurnRight_Finish();
      }
      case StateType::eTurnRight_InLane_Ing: {
        return SetTurnRight_InLane_Finish();
      }
      case StateType::eTurnLeft_Ing: {
        return SetTurnLeft_Finish();
      }
      case StateType::eTurnLeft_InLane_Ing: {
        return SetTurnLeft_InLane_Finish();
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 是否车道保持状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsLaneKeep() const TX_NOEXCEPT { return ((_plus_(StateType::eLaneKeep)) == State()); }

  /**
   * @brief 设置为eLateral_Action状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetLateralAction() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eLaneKeep)): {
        LogInfo << TXST_STATE_CHANGE(VehicleId(), mState, (_plus_(StateType::eLateral_Action)));
        mState = StateType::eLateral_Action;
        return true;
        break;
      }
      default:
        break;
    }
    SM_ASSERT(false, __func__);
    return false;
  }

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("Id", mId), _MAKE_NVP_("State", mState));
  }

 protected:
  StateType mState = StateType::eIdle;
  Base::txSysId mId = -1;
};

#if 0
struct UserDefinedStateContext {
  Base::txInt mUnUse = 0;
};
/*eStart = 0, eStop, eKill,
    eLaneKeep,
    eTurnLeft_Start, eTurnLeft_Ing, eTurnLeft_Finish, eTurnLeft_Abort,
    eTurnLeft_InLane_Start, eTurnLeft_InLane_Ing, eTurnLeft_InLane_Finish, eTurnLeft_InLane_Abort,
    eTurnRight_Start, eTurnRight_Ing, eTurnRight_Finish, eTurnRight_Abort,
    eTurnRight_InLane_Start, eTurnRight_InLane_Ing, eTurnRight_InLane_Finish, eTurnRight_InLane_Abort*/
struct UserDefinedState : public impl_ctx<UserDefinedState, UserDefinedStateContext> {
  using TAD_VehicleState = Base::Enums::TAD_VehicleState;
  virtual void Start() TX_NOEXCEPT {}
  virtual void Stop() TX_NOEXCEPT {}
  virtual void Kill() TX_NOEXCEPT {}
  virtual void LaneKeep() TX_NOEXCEPT {}

  virtual void TurnLeft_Start() TX_NOEXCEPT {}
  virtual void TurnLeft_Ing() TX_NOEXCEPT {}
  virtual void TurnLeft_Finish() TX_NOEXCEPT {}
  virtual void TurnLeft_Abort() TX_NOEXCEPT {}

  virtual void TurnLeft_InLane_Start() TX_NOEXCEPT {}
  virtual void TurnLeft_InLane_Ing() TX_NOEXCEPT {}
  virtual void TurnLeft_InLane_Finish() TX_NOEXCEPT {}
  virtual void TurnLeft_InLane_Abort() TX_NOEXCEPT {}

  virtual void TurnRight_Start() TX_NOEXCEPT {}
  virtual void TurnRight_Ing() TX_NOEXCEPT {}
  virtual void TurnRight_Finish() TX_NOEXCEPT {}
  virtual void TurnRight_Abort() TX_NOEXCEPT {}

  virtual void TurnRight_InLane_Start() TX_NOEXCEPT {}
  virtual void TurnRight_InLane_Ing() TX_NOEXCEPT {}
  virtual void TurnRight_InLane_Finish() TX_NOEXCEPT {}
  virtual void TurnRight_InLane_Abort() TX_NOEXCEPT {}

  virtual TAD_VehicleState CurState() const TX_NOEXCEPT { return mCurState; }

 protected:
  TAD_VehicleState mCurState = TAD_VehicleState::eIdle;
};

struct UserDefined_Start : public impl_state<UserDefined_Start, UserDefinedState> {
  virtual void Stop() TX_NOEXCEPT TX_OVERRIDE {}
  virtual void Kill() TX_NOEXCEPT TX_OVERRIDE {}
  virtual void LaneKeep() TX_NOEXCEPT TX_OVERRIDE {}
};
#endif
TX_NAMESPACE_CLOSE(SM)
TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
#undef TXST_STATE_CHANGE
#undef SM_ASSERT
