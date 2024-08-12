// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

// ----------------------------------------------------------------------------
#define TXST_NAMESPACE_OPEN(_NAME_) namespace _NAME_ {
// ----------------------------------------------------------------------------
#define TXST_NAMESPACE_CLOSE(_UNUSED_) }
// ----------------------------------------------------------------------------
// #define TXST_USING_NAMESPACE(_NAME_) using namespace _NAME_
// ----------------------------------------------------------------------------
#define TXST_FLAGS(_NAME_) #_NAME_ " = " << FLAGS_##_NAME_ << ", "
// ----------------------------------------------------------------------------
#define TXST_TRACE_VARIABLES(_NAME_) " " #_NAME_ " = " << _NAME_ << ", "
#define TX_VARS(_VAR_) " " #_VAR_ " = " << _VAR_ << ", "
// ----------------------------------------------------------------------------
#define TXST_TRACE_VARIABLES_NAME(_NAME_, _VALUE_) " " #_NAME_ " = " << (_VALUE_) << ", "
// ----------------------------------------------------------------------------
#define TXST_TRACE_COND_NAME(_NAME_, _VALUE_) " " #_NAME_ " = " << ((_VALUE_) ? "TRUE" : "FALSE") << ", "
// ----------------------------------------------------------------------------
/*Debug Tag, with no real function*/
#define TXSTMARK(_UNUSED_)
// ----------------------------------------------------------------------------
/*Debug Tag, with no real function*/
#define __TX_Mark__(_UNUSED_) (1)
// ----------------------------------------------------------------------------
#define __TX_Removed__ (0)
// ----------------------------------------------------------------------------
#define OVERRIDE override
// ----------------------------------------------------------------------------
#define NOEXCEPT noexcept
// ----------------------------------------------------------------------------
#define DEFAULT = default
// ----------------------------------------------------------------------------
#define txDELETE = delete
// ----------------------------------------------------------------------------
#define ErrExitCode (-1)
// ----------------------------------------------------------------------------
#define SuccExitCode (0)
// ----------------------------------------------------------------------------
#define CallSucc(__cond__) (true == __cond__)
// ----------------------------------------------------------------------------
#define CallFail(__cond__) (false == __cond__)
// ----------------------------------------------------------------------------
#define Null_Pointer(__ptr__) (nullptr == __ptr__)
// ----------------------------------------------------------------------------
#define NonNull_Pointer(__ptr__) (nullptr != __ptr__)
// ----------------------------------------------------------------------------
#define UseDouble (1)
// ----------------------------------------------------------------------------
#define FloatPrecision (14)
// ----------------------------------------------------------------------------
#define _StreamFormat_ std::setprecision(FloatPrecision) << std::fixed
// ----------------------------------------------------------------------------
#define _FootPrint_ ClassName() << " " << __func__
// ----------------------------------------------------------------------------
#define _FormatFoot_ ClassName() << " " << __func__ << std::setprecision(FloatPrecision) << std::fixed
// ----------------------------------------------------------------------------
#define bool2lpsz(bflag) ((bflag) ? "true" : "false")
// ----------------------------------------------------------------------------
#define TODO(_UNUSED_)
// ----------------------------------------------------------------------------
#define TAG(_UNUSED_)
// ----------------------------------------------------------------------------
#define TXST_CLASSNAME(classname) \
 public:                          \
  Base::txLpsz ClassName() const NOEXCEPT { return #classname; }
// ----------------------------------------------------------------------------
#define _AI_TRAFFIC_ "[AI_TRAFFIC]_"
// ----------------------------------------------------------------------------
#define QUESTION(_UNUSED_)

#if defined(_WIN32) || defined(WIN32)
#  define txPAUSE system("pause")
#  define txAssert(condition)                                                                           \
    do {                                                                                                \
      if (!(condition)) {                                                                               \
        LOG(WARNING) << "Assertion failed at " << __FILE__ << ":" << __LINE__ << " inside " << __func__ \
                     << "Condition: " << #condition;                                                    \
        system("pause");                                                                                \
      }                                                                                                 \
    } while (false)
#  define txMsg(__MESSAGE__)                                                                            \
    do {                                                                                                \
      {                                                                                                 \
        LOG(WARNING) << "Assertion failed at " << __FILE__ << ":" << __LINE__ << " inside " << __func__ \
                     << "Message: " << #__MESSAGE__;                                                    \
        system("pause");                                                                                \
      }                                                                                                 \
    } while (false)
#else
#  define txPAUSE
#  define txAssert(condition)
#  define txMsg(__MESSAGE__)
#endif
// ----------------------------------------------------------------------------
#define __ST(x) (#x)
#define _ST(x) __ST(x)
#define STR_COMMIT_ID _ST(GIT_COMMIT_HASH)
#define STR_GIT_BRANCH _ST(GIT_BRANCH)
// ----------------------------------------------------------------------------
#define USE_TBB (1)
// ----------------------------------------------------------------------------
#define __TX_LOCALCOORD__
// ----------------------------------------------------------------------------
#define USE_ThreadLog (0)
#define USE_LaneCacheOnInit (0)
#define USE_RTree (1)
#define LeftOnENU (90.0)
#define LeftIndex (0)
#define RightOnENU (-90.0)
#define RightIndex (1)
// ----------------------------------------------------------------------------
#define USE_SignalByLane (1)
// ----------------------------------------------------------------------------
#define USE_3rd_Party_TrafficFlow (0)
#if USE_3rd_Party_TrafficFlow
// ----------------------------------------------------------------------------
#  define Use_XiDianTrafficFlow (1)
// ----------------------------------------------------------------------------
#else
// ----------------------------------------------------------------------------
#  define Use_XiDianTrafficFlow (0)
// ----------------------------------------------------------------------------
#endif /*USE_3rd_Party_TrafficFlow*/
// ----------------------------------------------------------------------------
#define Use_TrailerEgo (0)
#define TrailerEgoStr ("LOCATION_TRAILER")
#define Use_TruckEgo (0)
#define Use_SignalPhase (1)

#define USE_GetLaneNearBy (1)
#define USE_Component (1)
#define USE_VehicleKinectInfo (0)
#define USE_Ego_RPY_Yaw (1)

#define InvalidId (-66)

#define Use_ManualVehicle (0)
