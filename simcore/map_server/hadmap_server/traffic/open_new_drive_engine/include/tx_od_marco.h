// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
//----------------------------------------------------------------------------
#define TX_NAMESPACE_OPEN(_NAME_) namespace _NAME_ {
//----------------------------------------------------------------------------
#define TX_NAMESPACE_CLOSE(_UNUSED_) }
//----------------------------------------------------------------------------
#define TX_FLAGS(_NAME_) #_NAME_ " = " << FLAGS_##_NAME_ << ", "
//----------------------------------------------------------------------------
#define TX_VARS(_VAR_) " " #_VAR_ " = " << _VAR_ << ", "
//----------------------------------------------------------------------------
#define TX_VARS_NAME(_NAME_, _VALUE_) " " #_NAME_ " = " << (_VALUE_) << ", "
//----------------------------------------------------------------------------
#define TX_COND(_VALUE_) " " #_VALUE_ " = " << ((_VALUE_) ? "TRUE" : "FALSE") << ", "
#define TX_COND_NAME(_NAME_, _VALUE_) " " #_NAME_ " = " << ((_VALUE_) ? "TRUE" : "FALSE") << ", "
//----------------------------------------------------------------------------
/*Debug Tag, with no actual function*/
#define TX_MARK(_UNUSED_)
//----------------------------------------------------------------------------
/*Debug Tag, with no actual function*/
#define __TX_Mark__(_UNUSED_) (1)
//----------------------------------------------------------------------------
#define __TX_Removed__ (0)
//----------------------------------------------------------------------------
#define TX_OVERRIDE override
//----------------------------------------------------------------------------
#define TX_NOEXCEPT noexcept
//----------------------------------------------------------------------------
#define TX_FINAL final
//----------------------------------------------------------------------------
#define TX_DEFAULT = default
//----------------------------------------------------------------------------
#define TX_DELETE = delete
//----------------------------------------------------------------------------
#define ErrExitCode (-1)
//----------------------------------------------------------------------------
#define SuccExitCode (0)
//----------------------------------------------------------------------------
#define CallSucc(__cond__) (true == (__cond__))
//----------------------------------------------------------------------------
#define CallFail(__cond__) (false == (__cond__))
//----------------------------------------------------------------------------
#define _NonEmpty_(__std_container__) (false == (__std_container__).empty())
//----------------------------------------------------------------------------
#define _Contain_(_stl_obj_, _elem_) (_stl_obj_.end() != _stl_obj_.find(_elem_))
//----------------------------------------------------------------------------
#define _NotContain_(_stl_obj_, _elem_) (_stl_obj_.end() == _stl_obj_.find(_elem_))
//----------------------------------------------------------------------------
#define Null_Pointer(__ptr__) (nullptr == (__ptr__))
//----------------------------------------------------------------------------
#define NonNull_Pointer(__ptr__) (nullptr != (__ptr__))
//----------------------------------------------------------------------------
#define __Lon__(gps) (gps.x)
#define __Lat__(gps) (gps.y)
#define __Alt__(gps) (gps.z)
#define __East__(enu) (enu.x())
#define __North__(enu) (enu.y())
#define __Horizontal__(vDir) (vDir.x())
#define __Vertical__(vDir) (vDir.y())
#define __S__(sl) (sl.x())
#define __L__(sl) (sl.y())
//----------------------------------------------------------------------------
#define __rid_idx__ (0)
#define __sid_idx__ (1)
#define __lid_idx__ (2)
#define __rid__(tuple_id) (std::get<__rid_idx__>(tuple_id))
#define __sid__(tuple_id) (std::get<__sid_idx__>(tuple_id))
#define __lid__(tuple_id) (std::get<__lid_idx__>(tuple_id))
//----------------------------------------------------------------------------
#define UseDouble (1)
//----------------------------------------------------------------------------
#define FloatPrecision (14)
//----------------------------------------------------------------------------
#define _StreamPrecision_ std::setprecision(FloatPrecision) << std::fixed
//----------------------------------------------------------------------------
#define bool2lpsz(bflag) ((bflag) ? "true" : "false")
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#define __ST(x) (#x)
#define _ST(x) __ST(x)
#define STR_COMMIT_ID _ST(GIT_COMMIT_HASH)
#define STR_GIT_BRANCH _ST(GIT_BRANCH)
//----------------------------------------------------------------------------
#define USE_TBB (1)
//----------------------------------------------------------------------------
#define LeftOnENU (90.0)
#define LeftIndex (0)
#define RightOnENU (-90.0)
#define RightIndex (1)
//----------------------------------------------------------------------------
#define WeakIsValid(weakPtr) (false == weakPtr.expired())
#define WeakIsNull(weakPtr) (true == weakPtr.expired())
#define Weak2SharedPtr(weakPtr) (weakPtr.expired()) ? (nullptr) : (weakPtr.lock())
#define ForceWeak2SharedPtr(weakPtr) (weakPtr.lock())
//----------------------------------------------------------------------------
#define AppName ("tx_od_server")

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#  define __tx_windows__
#elif defined(linux) || defined(__linux) || defined(__linux__)
#  define __tx_linux__
#endif

#define _TO_DZL_(_MSG_)
#define TODO(_MSG_)
#ifndef M_PI
#  define M_PI 3.141592653589793
#endif
