// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

// ----------------------------------------------------------------------------
#define TX_NAMESPACE_OPEN(_NAME_) namespace _NAME_ {
// ----------------------------------------------------------------------------
#define TX_NAMESPACE_CLOSE(_UNUSED_) }
// ----------------------------------------------------------------------------
// #define TX_USING_NAMESPACE(_NAME_) using namespace _NAME_;
// ----------------------------------------------------------------------------
#define TX_FLAGS(_NAME_) #_NAME_ " = " << FLAGS_##_NAME_ << ", "
// ----------------------------------------------------------------------------
#define TX_VARS(_VAR_) " " #_VAR_ " = " << _VAR_ << ", "
// ----------------------------------------------------------------------------
#define TX_VARS_NAME(_NAME_, _VALUE_) " " #_NAME_ " = " << (_VALUE_) << ", "
// ----------------------------------------------------------------------------
#define TX_COND(_VALUE_) " " #_VALUE_ " = " << ((_VALUE_) ? "TRUE" : "FALSE") << ", "
#define TX_COND_NAME(_NAME_, _VALUE_) " " #_NAME_ " = " << ((_VALUE_) ? "TRUE" : "FALSE") << ", "
// ----------------------------------------------------------------------------
/*Debug Tag, with no real function*/
#define TX_MARK(_UNUSED_)
// ----------------------------------------------------------------------------
/*Debug Tag, with no real function*/
#define __TX_Mark__(_UNUSED_) (1)
// ----------------------------------------------------------------------------
#define __TX_Removed__ (0)
// ----------------------------------------------------------------------------
#define TX_OVERRIDE override
// ----------------------------------------------------------------------------
#define TX_NOEXCEPT noexcept
// ----------------------------------------------------------------------------
#define TX_FINAL final
// ----------------------------------------------------------------------------
#define TX_DEFAULT = default
// ----------------------------------------------------------------------------
#define TX_DELETE = delete
// ----------------------------------------------------------------------------
#define ErrExitCode (-1)
// ----------------------------------------------------------------------------
#define SuccExitCode (0)
// ----------------------------------------------------------------------------
#define CallSucc(__cond__) (true == (__cond__))
// ----------------------------------------------------------------------------
#define CallFail(__cond__) (false == (__cond__))
// ----------------------------------------------------------------------------
#define _NonEmpty_(__std_container__) (false == (__std_container__).empty())
// ----------------------------------------------------------------------------
#define _Empty_(__std_container__) (true == (__std_container__).empty())
// ----------------------------------------------------------------------------
#define _Contain_(_stl_obj_, _elem_) (_stl_obj_.end() != _stl_obj_.find(_elem_))
// ----------------------------------------------------------------------------
#define _NotContain_(_stl_obj_, _elem_) (_stl_obj_.end() == _stl_obj_.find(_elem_))
// ----------------------------------------------------------------------------
#define Null_Pointer(__ptr__) (nullptr == (__ptr__))
// ----------------------------------------------------------------------------
#define NonNull_Pointer(__ptr__) (nullptr != (__ptr__))
// ----------------------------------------------------------------------------
#define TX_PTR_FUNC(_ptr_, _func_, _default_v_) (NonNull_Pointer(_ptr_) ? (_ptr_->_func_) : (_default_v_))
// ----------------------------------------------------------------------------
#define __Lon__(gps) (gps.x)
#define __Lat__(gps) (gps.y)
#define __Alt__(gps) (gps.z)
#define __East__(enu) (enu.x())
#define __North__(enu) (enu.y())
#define __Vertical__(vDir) (vDir.x())
#define __Horizontal__(vDir) (vDir.y())
// ----------------------------------------------------------------------------
#define UseDouble (1)
// ----------------------------------------------------------------------------
#define FloatPrecision (14)
// ----------------------------------------------------------------------------
#define _StreamPrecision_ std::setprecision(FloatPrecision) << std::fixed
// ----------------------------------------------------------------------------
#define bool2lpsz(bflag) ((bflag) ? "true" : "false")
// ----------------------------------------------------------------------------
#define TODO(_UNUSED_)
// ----------------------------------------------------------------------------
#define TAG(_UNUSED_)
// ----------------------------------------------------------------------------
#define _AI_TRAFFIC_ "[AI_TRAFFIC]"
// ----------------------------------------------------------------------------

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
#define USE_LaneCacheOnInit (1)
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
#define WeakIsValid(weakPtr) (false == weakPtr.expired())
#define WeakIsNull(weakPtr) (true == weakPtr.expired())
#define Weak2SharedPtr(weakPtr) (weakPtr.expired()) ? (nullptr) : (weakPtr.lock())
#define ForceWeak2SharedPtr(weakPtr) (weakPtr.lock())
// ----------------------------------------------------------------------------

#define Use_TrailerEgo (1)
#define Use_TruckEgo (1)
#define Use_SignalPhase (1)

#define USE_GetLaneNearBy (1)
#define USE_Component (1)
#define USE_VehicleKinectInfo (1)
#define USE_Ego_RPY_Yaw (1)
#define Use_ManualVehicle (1)
#define USE_NearestObjInfoForVisualizer (0)
#define USE_SUDOKU_GRID (1)
#define USE_HashedRoadNetwork (1)

#define USE_HdMapCacheConcurrent (1)
#define txMapHdr (HdMap::HadmapCacheConCurrent::GetMapHandler())

#define InvalidId (-1)
#define USE_DITW_Event (1)

#define GeneralCheck (0)
#if GeneralCheck
#  define GeneralCheckBool (false)
#  define LogSurroundInfo (true)
#else
#  define GeneralCheckBool (false)
#  define LogSurroundInfo (false)
#endif

#define USE_CustomModelImport (1)
#define USE_EgoGroup (1)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#  define __tx_windows__ (1)
#  define __tx_linux__ (0)
#else
#  define __tx_windows__ (0)
#  define __tx_linux__ (1)
#endif

#define __education_version__ (1)

#define _OutputSerialLog_ (0)

#define __HYPOT__(x, y) (std::hypot(x, y))

#if defined(GNUC) || defined(clang) || __tx_linux__
#  define TX_ALWAYS_INLINE inline
#elif defined(_MSC_VER) && !defined(clang)
#  define TX_ALWAYS_INLINE __forceinline
#endif

#define __ClearHashLanePerStep__ (1)

#define __Meeting__ (1)
#define __FocusVehicle__ (1)
#define _EraseRtree2d_ (1)
#define __UseProj__ (0)
#define __JunctionYield__ (1)
#define __SecenEventVersion_1_2_0_0__ (1)
#define __Control_V2__ (1)
#define __Consistency__ (0)
#define __SafeElemId__(__ptr__) ((NonNull_Pointer(__ptr__)) ? (__ptr__->Id()) : (-1))
