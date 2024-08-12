// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_serialization.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_AI_StateMachine)
#define LogWarn LOG(WARNING)
#define TXST_STATE_CHANGE(_ID_, _FROM_STATE_, _TO_STATE_)                                       \
  " Id = " << _ID_ << ", From " << (__enum2lpsz__(TAD_VehicleState_AI, _FROM_STATE_)) << " To " \
           << (__enum2lpsz__(TAD_VehicleState_AI, _TO_STATE_));
// #define SM_ASSERT(_Condition_, _Msg_) do { if (CallFail(_Condition_)) { LOG(WARNING) << _Msg_; system("pause"); } }
// while (false)
#define SM_ASSERT(_Condition_, _Msg_)
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/
TX_NAMESPACE_OPEN(TrafficFlow)
TX_NAMESPACE_OPEN(SM)

class txAITrafficState {
  using StateType = Base::Enums::TAD_VehicleState_AI;
#if 0
    eIdle = 0, eStart, eStop, eKill,
        eLaneKeep,
        eTurnLeft_Start, eTurnLeft_Ing, eTurnLeft_Finish, eTurnLeft_Abort,
        eTurnRight_Start, eTurnRight_Ing, eTurnRight_Finish, eTurnRight_Abort
#endif

 public:
  txAITrafficState() TX_DEFAULT;
  virtual ~txAITrafficState() TX_DEFAULT;

  /**
   * @brief 初始化核心逻辑部分
   *
   * 该函数用于初始化核心逻辑部分，包括设置当前行驶的车辆的ID，通常是车辆的唯一标识符。如果初始化成功，则返回真值；如果初始化失败，则返回假值。在继承的类中重写此方法以提供特定场景下的初始化行为。
   *
   * @param _ai_vehicle_id 当前行驶的车辆的ID
   * @return txBool 初始化结果，真值表示成功，假值表示失败
   */
  virtual Base::txBool Initialize(const Base::txSysId _ai_vehicle_id) TX_NOEXCEPT {
    mId = _ai_vehicle_id;
    return true;
  }

  /**
   * @brief 获取当前行驶的车辆ID
   *
   * @return Base::txSysId
   */
  Base::txSysId VehicleId() const TX_NOEXCEPT { return mId; }

  /**
   * @brief 设置状态为启动
   *
   * 该函数用于将状态更改为启动状态。在一般情况下，调用此函数将通知AI开始执行，然后调用FSM::Step()函数以响应State变化。如果当前状态已经是启动状态或者队列中存在正在处理的任务，则函数将返回false。
   *
   * @return 如果设置成功，则返回true，否则返回false。
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
   * @brief 设置状态为车道保持
   *
   * 该函数用于将状态更改为车道保持状态。在一般情况下，调用此函数将通知AI在车道保持模式下执行，然后调用FSM::Step()函数以响应State变化。如果当前状态已经是车道保持状态，则函数将返回false。
   *
   * @return 如果设置成功，则返回true，否则返回false。
   */
  virtual Base::txBool SetLaneKeep() TX_NOEXCEPT {
    switch (mState) {
      case (_plus_(StateType::eStart)):
      case (_plus_(StateType::eLaneKeep)):
      case (_plus_(StateType::eTurnLeft_Abort)):
      case (_plus_(StateType::eTurnLeft_Finish)):
      case (_plus_(StateType::eTurnRight_Abort)):
      case (_plus_(StateType::eTurnRight_Finish)): {
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
   * @brief 设置机器人开始向左转
   *
   * 此函数用于设置机器人开始向左转，并返回转向结果。
   *
   * @return 返回值表示函数执行的成功与否，true为成功，false为失败。
   */
  virtual Base::txBool StartTurnLeft() TX_NOEXCEPT { return SetTurnLeft_Start(); }

  /**
   * @brief 设置机器人开始向左转
   *
   * 此函数用于设置机器人开始向左转，并返回转向结果。
   *
   * @return 返回值表示函数执行的成功与否，true为成功，false为失败。
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
   * @brief 设置机器人开始向左转中
   *
   * 此函数用于设置机器人开始向左转，并返回转向结果。
   *
   * @return 返回值表示函数执行的成功与否，true为成功，false为失败。
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
   * @brief 设置机器人中止左转状态
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
   * @brief 设置机器人左转结束状态
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
   * @brief 设置机器人开始向右转
   *
   * @return Base::txBool
   */
  virtual Base::txBool StartTurnRight() TX_NOEXCEPT { return SetTurnRight_Start(); }

  /**
   * @brief 设置机器人开始向右转状态
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
   * @brief 设置机器人处于右转状态中
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
   * @brief 设置机器人右转中止状态
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
   * @brief 设置机器人右转结束状态
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
   * @brief 获取当前状态
   *
   * @return StateType
   */
  virtual StateType State() const TX_NOEXCEPT { return mState; }

  /**
   * @brief 状态是否处于左转
   *
   * @param _type 传入的状态
   * @return Base::txBool 返回是否处于左转
   */
  virtual Base::txBool IsInTurnLeft(const StateType _type) const TX_NOEXCEPT {
    switch (_type) {
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
   * @brief 当前是否处于左转状态
   *
   * @return Base::txBool 当前是否处于左转状态
   */
  virtual Base::txBool IsInTurnLeft() const TX_NOEXCEPT {
    return IsInTurnLeft(State());
    /*switch (State()) {
    case StateType::eTurnLeft_Start:
    case StateType::eTurnLeft_Ing:
    case StateType::eTurnLeft_Finish:
    case StateType::eTurnLeft_Abort: { return true; }
    default: {return false; }
    }*/
  }

  /**
   * @brief 判断是否右转状态
   *
   * @param _type 传入的状态
   * @return Base::txBool 判断是否右转状态
   */
  virtual Base::txBool IsInTurnRight(const StateType _type) const TX_NOEXCEPT {
    switch (_type) {
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
   * @brief 当前是否处于右转状态
   *
   * @return Base::txBool 当前是否处于右转状态
   */
  virtual Base::txBool IsInTurnRight() const TX_NOEXCEPT {
    return IsInTurnRight(State());
    /*switch (State()) {
    case StateType::eTurnRight_Start:
    case StateType::eTurnRight_Ing:
    case StateType::eTurnRight_Finish:
    case StateType::eTurnRight_Abort: { return true; }
    default: {return false; }
    }*/
  }

  /**
   * @brief 是否处于左转或右转状态
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
      case StateType::eTurnRight_Abort:
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
      case StateType::eTurnRight_Start:
      case StateType::eTurnLeft_Start: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 从Start状态到Ing状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool ChaneFromStartToIng() TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnRight_Start: {
        return SetTurnRight_Ing();
      }
      case StateType::eTurnLeft_Start: {
        return SetTurnLeft_Ing();
      }
      default: {
        SM_ASSERT(false, __func__);
        return false;
      }
    }
  }

  /**
   * @brief 从Start到Abort状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool ChaneFromStartToAbort() TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnRight_Start: {
        return SetTurnRight_Ing();
      }
      case StateType::eTurnLeft_Start: {
        return SetTurnLeft_Ing();
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
      case StateType::eTurnRight_Ing:
      case StateType::eTurnLeft_Ing: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 从Ing状态到Finish状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool ChaneFromIngToFinish() TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnRight_Ing: {
        return SetTurnRight_Finish();
      }
      case StateType::eTurnLeft_Ing: {
        return SetTurnLeft_Finish();
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 是否LaneKeep
   *
   * @param _type 传入的状态
   * @return Base::txBool
   */
  virtual Base::txBool IsLaneKeep(const StateType _type) const TX_NOEXCEPT {
    return ((_plus_(StateType::eLaneKeep)) == _type);
  }

  /**
   * @brief 当前是否处于LaneKeep状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsLaneKeep() const TX_NOEXCEPT {
    return IsLaneKeep(State());
    /*return ((_plus_(StateType::eLaneKeep)) == State());*/
  }

  /**
   * @brief 当前是否处于LaneChange状态
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
      case StateType::eTurnRight_Abort: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 是否中止车道保持状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInAbortLaneChange() const TX_NOEXCEPT { return IsInTurn_Abort(); }

  /**
   * @brief 是否换道开始状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsInLaneChange_StartIng() const TX_NOEXCEPT {
    switch (State()) {
      case StateType::eTurnLeft_Start:
      case StateType::eTurnLeft_Ing:

      case StateType::eTurnRight_Start:
      case StateType::eTurnRight_Ing: {
        return true;
      }
      default: {
        return false;
      }
    }
  }

  /**
   * @brief 设置转向中止状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool SetTurnAbort() TX_NOEXCEPT {
    if (IsInTurnRight()) {
      return SetTurnRight_Abort();
    } else if (IsInTurnLeft()) {
      return SetTurnLeft_Abort();
    }
    SM_ASSERT(false, __func__);
    return false;
  }

  /**
   * @brief 保存上一次状态
   *
   */
  virtual void SaveLastFSMStatus() TX_NOEXCEPT { mLastState = mState; }
  virtual StateType LastFSMState() const TX_NOEXCEPT { return mLastState; }

 public:
  /**
   * @brief 序列化模板函数
   *
   * @tparam Archive
   * @param ar
   */
  template <class Archive>
  void serialize(Archive& ar) {
    SerializeInfo("txAITrafficState");
    ar(_MAKE_NVP_("Id", mId));
    ar(_MAKE_NVP_("State", mState));
    ar(_MAKE_NVP_("LastState", mLastState));
  }

 protected:
  StateType mState = StateType::eIdle;
  StateType mLastState = StateType::eIdle;
  Base::txSysId mId = -1;
};

TX_NAMESPACE_CLOSE(SM)
TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
#undef TXST_STATE_CHANGE
#undef SM_ASSERT
#undef SerializeInfo
