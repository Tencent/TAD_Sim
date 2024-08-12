/*
 * rt_zcfcn.h
 *
 * Code generation for model "DriveLineATFrontDrive".
 *
 * Model version              : 1.51
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Thu Jun 23 10:34:26 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_rt_zcfcn_h_
#define RTW_HEADER_rt_zcfcn_h_
#include "rtwtypes.h"
#include "solver_zc.h"
#include "zero_crossing_types.h"
#ifndef slZcHadEvent
#  define slZcHadEvent(ev, zcsDir) (((ev) & (zcsDir)) != 0x00)
#endif

#ifndef slZcUnAliasEvents
#  define slZcUnAliasEvents(evL, evR)                                                          \
    ((((slZcHadEvent((evL), (SL_ZCS_EVENT_N2Z)) && slZcHadEvent((evR), (SL_ZCS_EVENT_Z2P))) || \
       (slZcHadEvent((evL), (SL_ZCS_EVENT_P2Z)) && slZcHadEvent((evR), (SL_ZCS_EVENT_Z2N))))   \
          ? (SL_ZCS_EVENT_NUL)                                                                 \
          : (evR)))
#endif

#ifdef __cplusplus

extern "C" {

#endif

extern ZCEventType rt_ZCFcn(ZCDirection zcDir, ZCSigState *prevZc, real_T currValue);

#ifdef __cplusplus

} /* extern "C" */
#endif
#endif /* RTW_HEADER_rt_zcfcn_h_ */
