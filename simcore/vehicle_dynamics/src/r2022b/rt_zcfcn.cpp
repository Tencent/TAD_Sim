/*
 * rt_zcfcn.cpp
 *
 * Code generation for model "DynamicSteer".
 *
 * Model version              : 2.21
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Thu Apr 13 10:52:25 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "rt_zcfcn.h"
#include "rtwtypes.h"
#include "solver_zc.h"
#include "zero_crossing_types.h"

extern "C" {
/* Detect zero crossings events. */
ZCEventType rt_ZCFcn(ZCDirection zcDir, ZCSigState *prevZc, real_T currValue) {
  slZcEventType zcsDir;
  slZcEventType tempEv;
  ZCEventType zcEvent{NO_ZCEVENT}; /* assume */

  /* zcEvent matrix */
  static const slZcEventType eventMatrix[4][4]{
      /*          ZER              POS              NEG              UNK */
      {SL_ZCS_EVENT_NUL, SL_ZCS_EVENT_Z2P, SL_ZCS_EVENT_Z2N, SL_ZCS_EVENT_NUL}, /* ZER */

      {SL_ZCS_EVENT_P2Z, SL_ZCS_EVENT_NUL, SL_ZCS_EVENT_P2N, SL_ZCS_EVENT_NUL}, /* POS */

      {SL_ZCS_EVENT_N2Z, SL_ZCS_EVENT_N2P, SL_ZCS_EVENT_NUL, SL_ZCS_EVENT_NUL}, /* NEG */

      {SL_ZCS_EVENT_NUL, SL_ZCS_EVENT_NUL, SL_ZCS_EVENT_NUL, SL_ZCS_EVENT_NUL} /* UNK */
  };

  /* get prevZcEvent and prevZcSign from prevZc */
  const slZcEventType prevEv{(slZcEventType)(((uint8_T)(*prevZc)) >> 2)};

  const slZcSignalSignType prevSign{(slZcSignalSignType)(((uint8_T)(*prevZc)) & (uint8_T)0x03)};

  /* get current zcSignal sign from current zcSignal value */
  const slZcSignalSignType currSign{
      (slZcSignalSignType)((currValue) > 0.0 ? SL_ZCS_SIGN_POS
                                             : ((currValue) < 0.0 ? SL_ZCS_SIGN_NEG : SL_ZCS_SIGN_ZERO))};

  /* get current zcEvent based on prev and current zcSignal value */
  slZcEventType currEv{eventMatrix[prevSign][currSign]};

  /* get slZcEventType from ZCDirection */
  switch (zcDir) {
    case ANY_ZERO_CROSSING:
      zcsDir = SL_ZCS_EVENT_ALL;
      break;

    case FALLING_ZERO_CROSSING:
      zcsDir = SL_ZCS_EVENT_ALL_DN;
      break;

    case RISING_ZERO_CROSSING:
      zcsDir = SL_ZCS_EVENT_ALL_UP;
      break;

    default:
      zcsDir = SL_ZCS_EVENT_NUL;
      break;
  }

  /*had event, check if double zc happend remove double detection. */
  if (slZcHadEvent(currEv, zcsDir)) {
    currEv = (slZcEventType)(slZcUnAliasEvents(prevEv, currEv));
  } else {
    currEv = SL_ZCS_EVENT_NUL;
  }

  /* Update prevZc */
  tempEv = (slZcEventType)(currEv << 2); /* shift left by 2 bits */
  *prevZc = (ZCSigState)((currSign) | (tempEv));
  if ((currEv & SL_ZCS_EVENT_ALL_DN) != 0) {
    zcEvent = FALLING_ZCEVENT;
  } else if ((currEv & SL_ZCS_EVENT_ALL_UP) != 0) {
    zcEvent = RISING_ZCEVENT;
  } else {
    zcEvent = NO_ZCEVENT;
  }

  return zcEvent;
} /* rt_ZCFcn */
}
