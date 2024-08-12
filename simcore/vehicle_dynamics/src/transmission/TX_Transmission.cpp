/*
 * TX_Transmission.cpp
 *
 * Code generation for model "TX_Transmission".
 *
 * Model version              : 1.210
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Dec  6 16:43:43 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TX_Transmission.h"
#include <cmath>
#include "TX_Transmission_private.h"
#include "rtwtypes.h"
#include "zero_crossing_types.h"

extern "C" {

#include "rt_nonfinite.h"
}

/* Named constants for Chart: '<S2>/hold' */
const uint8_T TX_Transmission_IN_Hold{1U};

const uint8_T TX_Transmission_IN_NO_ACTIVE_CHILD{0U};

const uint8_T TX_Transmission_IN_Normal{2U};

/* Named constants for Chart: '<S59>/TCM Shift Controller' */
const uint8_T TX_Transmission_IN_DownShifting{1U};

const uint8_T TX_Transmission_IN_G1{1U};

const uint8_T TX_Transmission_IN_G2{2U};

const uint8_T TX_Transmission_IN_G3{3U};

const uint8_T TX_Transmission_IN_G4{4U};

const uint8_T TX_Transmission_IN_G5{5U};

const uint8_T TX_Transmission_IN_G6{6U};

const uint8_T TX_Transmission_IN_G7{7U};

const uint8_T TX_Transmission_IN_G8{8U};

const uint8_T TX_Transmission_IN_Neutral{9U};

const uint8_T TX_Transmission_IN_R{10U};

const uint8_T TX_Transmission_IN_SteadyState{2U};

const uint8_T TX_Transmission_IN_UpShifting{3U};

const uint8_T TX_Transmission_IN_preDownShifting{4U};

const uint8_T TX_Transmission_IN_preUpShifting{5U};

const int32_T TX_Transmission_event_Down{0};

const int32_T TX_Transmission_event_Up{1};

real_T look2_binlcapw(real_T u0, real_T u1, const real_T bp0[], const real_T bp1[], const real_T table[],
                      const uint32_T maxIndex[], uint32_T stride) {
  real_T fractions[2];
  real_T frac;
  real_T y;
  real_T yL_0d0;
  uint32_T bpIndices[2];
  uint32_T bpIdx;
  uint32_T iLeft;
  uint32_T iRght;

  /* Column-major Lookup 2-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = 0.0;
  } else if (u0 < bp0[maxIndex[0U]]) {
    /* Binary Search */
    bpIdx = maxIndex[0U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[0U];
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex[0U];
    frac = 0.0;
  }

  fractions[0U] = frac;
  bpIndices[0U] = iLeft;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u1 <= bp1[0U]) {
    iLeft = 0U;
    frac = 0.0;
  } else if (u1 < bp1[maxIndex[1U]]) {
    /* Binary Search */
    bpIdx = maxIndex[1U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[1U];
    while (iRght - iLeft > 1U) {
      if (u1 < bp1[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u1 - bp1[iLeft]) / (bp1[iLeft + 1U] - bp1[iLeft]);
  } else {
    iLeft = maxIndex[1U];
    frac = 0.0;
  }

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Overflow mode: 'portable wrapping'
   */
  bpIdx = iLeft * stride + bpIndices[0U];
  if (bpIndices[0U] == maxIndex[0U]) {
    y = table[bpIdx];
  } else {
    yL_0d0 = table[bpIdx];
    y = (table[bpIdx + 1U] - yL_0d0) * fractions[0U] + yL_0d0;
  }

  if (iLeft == maxIndex[1U]) {
  } else {
    bpIdx += stride;
    if (bpIndices[0U] == maxIndex[0U]) {
      yL_0d0 = table[bpIdx];
    } else {
      yL_0d0 = table[bpIdx];
      yL_0d0 += (table[bpIdx + 1U] - yL_0d0) * fractions[0U];
    }

    y += (yL_0d0 - y) * frac;
  }

  return y;
}

uint32_T plook_u32d_binckan(real_T u, const real_T bp[], uint32_T maxIndex) {
  uint32_T bpIndex;

  /* Prelookup - Index only
     Index Search method: 'binary'
     Interpolation method: 'Use nearest'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u <= bp[0U]) {
    bpIndex = 0U;
  } else if (u < bp[maxIndex]) {
    bpIndex = binsearch_u32d(u, bp, maxIndex >> 1U, maxIndex);
    if ((bpIndex < maxIndex) && (bp[bpIndex + 1U] - u <= u - bp[bpIndex])) {
      bpIndex++;
    }
  } else {
    bpIndex = maxIndex;
  }

  return bpIndex;
}

uint32_T binsearch_u32d(real_T u, const real_T bp[], uint32_T startIndex, uint32_T maxIndex) {
  uint32_T bpIdx;
  uint32_T bpIndex;
  uint32_T iRght;

  /* Binary Search */
  bpIdx = startIndex;
  bpIndex = 0U;
  iRght = maxIndex;
  while (iRght - bpIndex > 1U) {
    if (u < bp[bpIdx]) {
      iRght = bpIdx;
    } else {
      bpIndex = bpIdx;
    }

    bpIdx = (iRght + bpIndex) >> 1U;
  }

  return bpIndex;
}

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void TX_Transmission::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
  time_T t{rtsiGetT(si)};

  time_T tnew{rtsiGetSolverStopTime(si)};

  time_T h{rtsiGetStepSize(si)};

  real_T *x{rtsiGetContStates(si)};

  ODE4_IntgData *id{static_cast<ODE4_IntgData *>(rtsiGetSolverData(si))};

  real_T *y{id->y};

  real_T *f0{id->f[0]};

  real_T *f1{id->f[1]};

  real_T *f2{id->f[2]};

  real_T *f3{id->f[3]};

  real_T temp;
  int_T i;
  int_T nXc{8};

  rtsiSetSimTimeStep(si, MINOR_TIME_STEP);

  /* Save the state values at time t in y, we'll use x as ynew. */
  (void)std::memcpy(y, x, static_cast<uint_T>(nXc) * sizeof(real_T));

  /* Assumes that rtsiSetT and ModelOutputs are up-to-date */
  /* f0 = f(t,y) */
  rtsiSetdX(si, f0);
  TX_Transmission_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  TX_Transmission_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  TX_Transmission_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  TX_Transmission_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Function for Chart: '<S59>/TCM Shift Controller' */
void TX_Transmission::TX_Transmission_GearCmd(void) {
  switch (TX_Transmission_DW.is_GearCmd) {
    case TX_Transmission_IN_G1:
      if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Up) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G2;
        TX_Transmission_B.GearCmd = 2;
      } else if (TX_Transmission_B.LogicalOperator1) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_Neutral;
        TX_Transmission_B.GearCmd = 0;
      }
      break;

    case TX_Transmission_IN_G2:
      if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Up) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G3;
        TX_Transmission_B.GearCmd = 3;
      } else if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Down) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G1;
        TX_Transmission_B.GearCmd = 1;
      }
      break;

    case TX_Transmission_IN_G3:
      if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Up) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G4;
        TX_Transmission_B.GearCmd = 4;
      } else if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Down) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G2;
        TX_Transmission_B.GearCmd = 2;
      }
      break;

    case TX_Transmission_IN_G4:
      if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Up) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G5;
        TX_Transmission_B.GearCmd = 5;
      } else if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Down) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G3;
        TX_Transmission_B.GearCmd = 3;
      }
      break;

    case TX_Transmission_IN_G5:
      if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Up) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G6;
        TX_Transmission_B.GearCmd = 6;
      } else if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Down) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G4;
        TX_Transmission_B.GearCmd = 4;
      }
      break;

    case TX_Transmission_IN_G6:
      if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Up) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G7;
        TX_Transmission_B.GearCmd = 7;
      } else if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Down) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G5;
        TX_Transmission_B.GearCmd = 5;
      }
      break;

    case TX_Transmission_IN_G7:
      if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Up) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G8;
        TX_Transmission_B.GearCmd = 8;
      } else if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Down) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G6;
        TX_Transmission_B.GearCmd = 6;
      }
      break;

    case TX_Transmission_IN_G8:
      if (TX_Transmission_DW.sfEvent == TX_Transmission_event_Down) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G7;
        TX_Transmission_B.GearCmd = 7;
      }
      break;

    case TX_Transmission_IN_Neutral:
      if (TX_Transmission_B.gearMode) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G1;
        TX_Transmission_B.GearCmd = 1;
      } else if (TX_Transmission_B.gearMode_i) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_R;
        TX_Transmission_B.GearCmd = -1;
      }
      break;

    case TX_Transmission_IN_R:
      if (TX_Transmission_B.LogicalOperator1) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_Neutral;
        TX_Transmission_B.GearCmd = 0;
      }
      break;

    default:
      /* Unreachable state, for coverage only */
      TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_NO_ACTIVE_CHILD;
      break;
  }
}

/* Model step function */
void TX_Transmission::step() {
  /* local block i/o variables */
  int32_T rtb_DataTypeConversion;
  real_T CalculateDownshiftThreshold;
  real_T CalculateUpshiftThreshold;
  real_T rtb_Add_h;
  real_T rtb_Integrator;
  real_T rtb_Merge;
  real_T rtb_Switch_h;
  real_T rtb_Switch_l;
  int8_T rtAction;
  int8_T rtPrevAction;
  ZCEventType zcEvent;
  if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
    /* set solver stop time */
    if (!((&TX_Transmission_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&TX_Transmission_M)->solverInfo, (((&TX_Transmission_M)->Timing.clockTickH0 + 1) *
                                                                (&TX_Transmission_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(
          &(&TX_Transmission_M)->solverInfo,
          (((&TX_Transmission_M)->Timing.clockTick0 + 1) * (&TX_Transmission_M)->Timing.stepSize0 +
           (&TX_Transmission_M)->Timing.clockTickH0 * (&TX_Transmission_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&TX_Transmission_M))) {
    (&TX_Transmission_M)->Timing.t[0] = rtsiGetT(&(&TX_Transmission_M)->solverInfo);
  }

  if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
    /* Memory: '<S15>/Memory' */
    TX_Transmission_B.Memory = TX_Transmission_DW.Memory_PreviousInput;

    /* Constant: '<S13>/Constant1' */
    TX_Transmission_B.Constant1 = TX_Transmission_P.init_gear_num;
  }

  /* Integrator: '<S15>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_Transmission_M)->solverInfo)) {
    zcEvent = rt_ZCFcn(RISING_ZERO_CROSSING, &TX_Transmission_PrevZCX.Integrator_Reset_ZCE, (TX_Transmission_B.Memory));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_Transmission_DW.Integrator_IWORK != 0)) {
      TX_Transmission_X.Integrator_CSTATE = TX_Transmission_B.Constant1;
    }
  }

  rtb_Integrator = TX_Transmission_X.Integrator_CSTATE;

  /* Rounding: '<S13>/Round' incorporates:
   *  Integrator: '<S15>/Integrator'
   */
  TX_Transmission_B.Round = std::round(TX_Transmission_X.Integrator_CSTATE);

  /* Outport: '<Root>/TransGear[]' */
  TX_Transmission_Y.TransGear = TX_Transmission_B.Round;
  if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
    /* Memory: '<S12>/Memory' */
    TX_Transmission_B.Memory_d = TX_Transmission_DW.Memory_PreviousInput_h;

    /* Constant: '<S8>/Constant' */
    TX_Transmission_B.Constant = TX_Transmission_P.VEH.InitialLongVel / TX_Transmission_P.StatLdWhlR[0] *
                                 (TX_Transmission_P.ratio_diff_front + TX_Transmission_P.ratio_diff_rear) / 2.0 *
                                 TX_Transmission_P.init_gear_ratio;
  }

  /* Integrator: '<S9>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_Transmission_M)->solverInfo)) {
    zcEvent =
        rt_ZCFcn(RISING_ZERO_CROSSING, &TX_Transmission_PrevZCX.Integrator_Reset_ZCE_e, (TX_Transmission_B.Memory_d));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_Transmission_DW.Integrator_IWORK_o != 0)) {
      TX_Transmission_X.Integrator_CSTATE_h = TX_Transmission_B.Constant;
    }
  }

  rtb_Switch_h = TX_Transmission_X.Integrator_CSTATE_h;

  /* End of Integrator: '<S9>/Integrator' */

  /* Saturate: '<S8>/Saturation' */
  rtb_Merge = TX_Transmission_P.EngIdleSpd * 2.0 * 3.1415926 / 60.0;
  if (TX_Transmission_X.Integrator_CSTATE_h > TX_Transmission_P.Saturation_UpperSat_g) {
    rtb_Merge = TX_Transmission_P.Saturation_UpperSat_g;
  } else if (!(TX_Transmission_X.Integrator_CSTATE_h < rtb_Merge)) {
    rtb_Merge = TX_Transmission_X.Integrator_CSTATE_h;
  }

  /* End of Saturate: '<S8>/Saturation' */

  /* Outport: '<Root>/EngSpeed[rad|s]' */
  TX_Transmission_Y.EngSpeedrads = rtb_Merge;
  if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
    /* Memory: '<S52>/Memory' */
    TX_Transmission_B.Memory_c = TX_Transmission_DW.Memory_PreviousInput_o;

    /* Constant: '<S51>/domega_o' */
    TX_Transmission_B.domega_o = TX_Transmission_P.PropShaft_domega_o;
  }

  /* Integrator: '<S52>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_Transmission_M)->solverInfo)) {
    zcEvent =
        rt_ZCFcn(RISING_ZERO_CROSSING, &TX_Transmission_PrevZCX.Integrator_Reset_ZCE_c, (TX_Transmission_B.Memory_c));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_Transmission_DW.Integrator_IWORK_i != 0)) {
      TX_Transmission_X.Integrator_CSTATE_c = TX_Transmission_B.domega_o;
    }
  }

  rtb_Switch_l = TX_Transmission_X.Integrator_CSTATE_c;

  /* Sum: '<S51>/Subtract1' incorporates:
   *  Gain: '<S51>/Gain1'
   *  Gain: '<S51>/Gain2'
   *  Integrator: '<S51>/Integrator'
   *  Integrator: '<S52>/Integrator'
   */
  rtb_Add_h = TX_Transmission_P.PropShaft_b * TX_Transmission_X.Integrator_CSTATE_c +
              TX_Transmission_P.PropShaft_k * TX_Transmission_X.Integrator_CSTATE_hz;

  /* Outport: '<Root>/DrvShfTrq[Nm]' */
  TX_Transmission_Y.DrvShfTrqNm = rtb_Add_h;

  /* Integrator: '<S64>/Integrator1' */
  TX_Transmission_B.Integrator1 = TX_Transmission_X.Integrator1_CSTATE;

  /* Logic: '<S60>/Logical Operator' incorporates:
   *  Constant: '<S65>/Constant'
   *  Constant: '<S66>/Constant'
   *  Inport: '<Root>/DriveLineIn'
   *  RelationalOperator: '<S65>/Compare'
   *  RelationalOperator: '<S66>/Compare'
   */
  TX_Transmission_B.gearMode =
      ((TX_Transmission_U.DriveLineIn.gear_cmd_driver > TX_Transmission_P.CompareToConstant1_const) &&
       (TX_Transmission_B.Integrator1 >= TX_Transmission_P.CompareToConstant_const));

  /* Logic: '<S62>/Logical Operator' incorporates:
   *  Constant: '<S68>/Constant'
   *  Constant: '<S69>/Constant'
   *  Inport: '<Root>/DriveLineIn'
   *  RelationalOperator: '<S68>/Compare'
   *  RelationalOperator: '<S69>/Compare'
   */
  TX_Transmission_B.gearMode_i =
      ((TX_Transmission_U.DriveLineIn.gear_cmd_driver < TX_Transmission_P.CompareToConstant1_const_m) &&
       (TX_Transmission_B.Integrator1 <= TX_Transmission_P.CompareToConstant_const_h));

  /* Gain: '<S4>/rpm' */
  rtb_Merge *= TX_Transmission_P.rpm_Gain;

  /* Logic: '<S61>/Logical Operator1' incorporates:
   *  Constant: '<S61>/Constant'
   *  Constant: '<S61>/Constant1'
   *  Constant: '<S61>/Constant2'
   *  Constant: '<S67>/Constant'
   *  Inport: '<Root>/DriveLineIn'
   *  Inport: '<Root>/SoftECUOut'
   *  Logic: '<S61>/Logical Operator'
   *  RelationalOperator: '<S61>/Relational Operator'
   *  RelationalOperator: '<S61>/Relational Operator1'
   *  RelationalOperator: '<S61>/Relational Operator2'
   *  RelationalOperator: '<S67>/Compare'
   */
  TX_Transmission_B.LogicalOperator1 =
      ((TX_Transmission_U.DriveLineIn.gear_cmd_driver == TX_Transmission_P.CompareToConstant_const_p) ||
       ((TX_Transmission_U.SoftECUOut.throttle_01 <= TX_Transmission_P.Constant1_Value_hx) &&
        (rtb_Merge <= TX_Transmission_P.Constant_Value_e)) ||
       (rtb_Merge <= TX_Transmission_P.Constant2_Value));
  if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
    /* Chart: '<S59>/TCM Shift Controller' incorporates:
     *  Constant: '<S59>/init_gear_num'
     */
    if (TX_Transmission_DW.temporalCounter_i1 < MAX_uint32_T) {
      TX_Transmission_DW.temporalCounter_i1++;
    }

    TX_Transmission_DW.sfEvent = -1;
    if (TX_Transmission_DW.is_active_c4_TX_Transmission == 0U) {
      TX_Transmission_DW.is_active_c4_TX_Transmission = 1U;
      TX_Transmission_DW.is_active_GearCmd = 1U;
      if (TX_Transmission_P.init_gear_num == -1.0) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_R;
        TX_Transmission_B.GearCmd = -1;
      } else if (TX_Transmission_P.init_gear_num == 0.0) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_Neutral;
        TX_Transmission_B.GearCmd = 0;
      } else if (TX_Transmission_P.init_gear_num == 1.0) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G1;
        TX_Transmission_B.GearCmd = 1;
      } else if (TX_Transmission_P.init_gear_num == 2.0) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G2;
        TX_Transmission_B.GearCmd = 2;
      } else if (TX_Transmission_P.init_gear_num == 3.0) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G3;
        TX_Transmission_B.GearCmd = 3;
      } else if (TX_Transmission_P.init_gear_num == 4.0) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G4;
        TX_Transmission_B.GearCmd = 4;
      } else if (TX_Transmission_P.init_gear_num == 6.0) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G5;
        TX_Transmission_B.GearCmd = 5;
      } else if (TX_Transmission_P.init_gear_num == 3.0) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G6;
        TX_Transmission_B.GearCmd = 6;
      } else if (TX_Transmission_P.init_gear_num == 7.0) {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G7;
        TX_Transmission_B.GearCmd = 7;
      } else {
        TX_Transmission_DW.is_GearCmd = TX_Transmission_IN_G8;
        TX_Transmission_B.GearCmd = 8;
      }

      TX_Transmission_DW.is_active_SelectionState = 1U;
      TX_Transmission_DW.is_SelectionState = TX_Transmission_IN_SteadyState;
    } else {
      if (TX_Transmission_DW.is_active_GearCmd != 0U) {
        TX_Transmission_GearCmd();
      }

      if (TX_Transmission_DW.is_active_SelectionState != 0U) {
        /* Outputs for Function Call SubSystem: '<S63>/calc_up' */
        /* Lookup_n-D: '<S72>/Calculate Upshift Threshold' incorporates:
         *  Inport: '<Root>/SoftECUOut'
         */
        CalculateUpshiftThreshold = look2_binlcapw(
            TX_Transmission_U.SoftECUOut.throttle_01, static_cast<real_T>(TX_Transmission_B.GearCmd),
            TX_Transmission_P.Pedal_Positions_UpShift, TX_Transmission_P.Up_Gears, TX_Transmission_P.Upshift_Speeds,
            TX_Transmission_P.CalculateUpshiftThreshold_maxIndex, m_upshift_accload_points_num);

        /* End of Outputs for SubSystem: '<S63>/calc_up' */

        /* Outputs for Function Call SubSystem: '<S63>/calc_down' */
        /* Lookup_n-D: '<S71>/Calculate  Downshift Threshold' incorporates:
         *  Inport: '<Root>/SoftECUOut'
         */
        CalculateDownshiftThreshold = look2_binlcapw(
            TX_Transmission_U.SoftECUOut.throttle_01, static_cast<real_T>(TX_Transmission_B.GearCmd),
            TX_Transmission_P.Pedal_Positions_DnShift, TX_Transmission_P.Dn_Gears, TX_Transmission_P.Downshift_Speeds,
            TX_Transmission_P.CalculateDownshiftThreshold_maxIndex, m_downshift_accload_points_num);

        /* End of Outputs for SubSystem: '<S63>/calc_down' */
        switch (TX_Transmission_DW.is_SelectionState) {
          case TX_Transmission_IN_DownShifting: {
            int32_T b_previousEvent;
            b_previousEvent = TX_Transmission_DW.sfEvent;
            TX_Transmission_DW.sfEvent = TX_Transmission_event_Down;
            if (TX_Transmission_DW.is_active_GearCmd != 0U) {
              TX_Transmission_GearCmd();
            }

            TX_Transmission_DW.sfEvent = b_previousEvent;
            TX_Transmission_DW.is_SelectionState = TX_Transmission_IN_SteadyState;
          } break;

          case TX_Transmission_IN_SteadyState:
            if ((TX_Transmission_B.Integrator1 < CalculateDownshiftThreshold) && (TX_Transmission_B.GearCmd > 1)) {
              TX_Transmission_DW.is_SelectionState = TX_Transmission_IN_preDownShifting;
              TX_Transmission_DW.temporalCounter_i1 = 0U;
            } else if ((TX_Transmission_B.Integrator1 > CalculateUpshiftThreshold) &&
                       (TX_Transmission_B.GearCmd < TX_Transmission_P.max_gear_num)) {
              TX_Transmission_DW.is_SelectionState = TX_Transmission_IN_preUpShifting;
              TX_Transmission_DW.temporalCounter_i1 = 0U;
            }
            break;

          case TX_Transmission_IN_UpShifting: {
            int32_T b_previousEvent;
            b_previousEvent = TX_Transmission_DW.sfEvent;
            TX_Transmission_DW.sfEvent = TX_Transmission_event_Up;
            if (TX_Transmission_DW.is_active_GearCmd != 0U) {
              TX_Transmission_GearCmd();
            }

            TX_Transmission_DW.sfEvent = b_previousEvent;
            TX_Transmission_DW.is_SelectionState = TX_Transmission_IN_SteadyState;
          } break;

          case TX_Transmission_IN_preDownShifting:
            if ((TX_Transmission_DW.temporalCounter_i1 >=
                 static_cast<uint32_T>(std::ceil(TX_Transmission_P.pre_shift_wait_time * 1000.0))) &&
                (TX_Transmission_B.Integrator1 < CalculateDownshiftThreshold - 2.0)) {
              TX_Transmission_DW.is_SelectionState = TX_Transmission_IN_DownShifting;
              TX_Transmission_B.GearCmd = static_cast<int8_T>(static_cast<real_T>(TX_Transmission_B.GearCmd) - 0.5);
            } else if (TX_Transmission_B.Integrator1 >= CalculateDownshiftThreshold) {
              TX_Transmission_DW.is_SelectionState = TX_Transmission_IN_SteadyState;
            }
            break;

          case TX_Transmission_IN_preUpShifting:
            if ((TX_Transmission_DW.temporalCounter_i1 >=
                 static_cast<uint32_T>(std::ceil(TX_Transmission_P.pre_shift_wait_time * 1000.0))) &&
                (TX_Transmission_B.Integrator1 > CalculateUpshiftThreshold)) {
              TX_Transmission_DW.is_SelectionState = TX_Transmission_IN_UpShifting;
              TX_Transmission_B.GearCmd = static_cast<int8_T>(static_cast<real_T>(TX_Transmission_B.GearCmd) + 0.5);
            } else if (TX_Transmission_B.Integrator1 <= CalculateUpshiftThreshold) {
              TX_Transmission_DW.is_SelectionState = TX_Transmission_IN_SteadyState;
            }
            break;

          default:
            /* Unreachable state, for coverage only */
            TX_Transmission_DW.is_SelectionState = TX_Transmission_IN_NO_ACTIVE_CHILD;
            break;
        }
      }
    }

    /* End of Chart: '<S59>/TCM Shift Controller' */

    /* DataTypeConversion: '<S2>/Data Type Conversion' */
    rtb_DataTypeConversion = TX_Transmission_B.GearCmd;

    /* Chart: '<S2>/hold' incorporates:
     *  Constant: '<S2>/HoldSecond'
     *  RelationalOperator: '<S6>/FixPt Relational Operator'
     *  UnitDelay: '<S6>/Delay Input1'
     */
    if (TX_Transmission_DW.temporalCounter_i1_e < MAX_uint32_T) {
      TX_Transmission_DW.temporalCounter_i1_e++;
    }

    if (TX_Transmission_DW.is_active_c3_TX_Transmission == 0U) {
      TX_Transmission_DW.is_active_c3_TX_Transmission = 1U;
      TX_Transmission_DW.is_c3_TX_Transmission = TX_Transmission_IN_Normal;
      TX_Transmission_B.y = TX_Transmission_B.GearCmd;
    } else if (TX_Transmission_DW.is_c3_TX_Transmission == TX_Transmission_IN_Hold) {
      if (TX_Transmission_DW.temporalCounter_i1_e >=
          static_cast<uint32_T>(std::ceil(TX_Transmission_P.HoldSecond_Value * 1000.0))) {
        TX_Transmission_DW.is_c3_TX_Transmission = TX_Transmission_IN_Normal;
        TX_Transmission_B.y = TX_Transmission_B.GearCmd;
      }

      /* case IN_Normal: */
    } else if (rtb_DataTypeConversion != TX_Transmission_DW.DelayInput1_DSTATE) {
      TX_Transmission_DW.is_c3_TX_Transmission = TX_Transmission_IN_Hold;
      TX_Transmission_DW.temporalCounter_i1_e = 0U;
      TX_Transmission_B.y = TX_Transmission_B.GearCmd;
    }

    /* End of Chart: '<S2>/hold' */
  }

  /* DeadZone: '<S13>/Dead Zone' incorporates:
   *  Integrator: '<S15>/Integrator'
   */
  if (TX_Transmission_X.Integrator_CSTATE > TX_Transmission_P.DeadZone_End) {
    /* DeadZone: '<S13>/Dead Zone' */
    TX_Transmission_B.DeadZone = TX_Transmission_X.Integrator_CSTATE - TX_Transmission_P.DeadZone_End;
  } else if (TX_Transmission_X.Integrator_CSTATE >= TX_Transmission_P.DeadZone_Start) {
    /* DeadZone: '<S13>/Dead Zone' */
    TX_Transmission_B.DeadZone = 0.0;
  } else {
    /* DeadZone: '<S13>/Dead Zone' */
    TX_Transmission_B.DeadZone = TX_Transmission_X.Integrator_CSTATE - TX_Transmission_P.DeadZone_Start;
  }

  /* End of DeadZone: '<S13>/Dead Zone' */

  /* Switch: '<S5>/Switch' incorporates:
   *  Abs: '<S5>/Abs'
   *  Constant: '<S5>/Constant'
   *  Constant: '<S73>/Constant'
   *  Constant: '<S74>/Constant'
   *  Gain: '<S5>/speed'
   *  Inport: '<Root>/DriveLineIn'
   *  Logic: '<S5>/lowSpeedAndNegativeTorque'
   *  RelationalOperator: '<S73>/Compare'
   *  RelationalOperator: '<S74>/Compare'
   *  Sum: '<S5>/Sum of Elements'
   */
  if ((TX_Transmission_U.DriveLineIn.engine_trq_Nm < TX_Transmission_P.Constant_Value_pi) &&
      ((((std::abs(TX_Transmission_U.DriveLineIn.wheel_spd_rad_s[0]) +
          std::abs(TX_Transmission_U.DriveLineIn.wheel_spd_rad_s[1])) +
         std::abs(TX_Transmission_U.DriveLineIn.wheel_spd_rad_s[2])) +
        std::abs(TX_Transmission_U.DriveLineIn.wheel_spd_rad_s[3])) *
           (TX_Transmission_P.StatLdWhlR[0] / 4.0) <
       TX_Transmission_P.CompareToConstant_const_i)) {
    CalculateUpshiftThreshold = TX_Transmission_P.Constant_Value_b;
  } else {
    CalculateUpshiftThreshold = TX_Transmission_U.DriveLineIn.engine_trq_Nm;
  }

  /* End of Switch: '<S5>/Switch' */
  if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
    /* InitialCondition: '<S20>/IC' */
    if (TX_Transmission_DW.IC_FirstOutputTime) {
      TX_Transmission_DW.IC_FirstOutputTime = false;

      /* InitialCondition: '<S20>/IC' */
      TX_Transmission_B.IC = TX_Transmission_P.IC_Value;
    } else {
      /* InitialCondition: '<S20>/IC' incorporates:
       *  Constant: '<S20>/Constant1'
       */
      TX_Transmission_B.IC = TX_Transmission_P.Constant1_Value_c;
    }

    /* End of InitialCondition: '<S20>/IC' */

    /* InitialCondition: '<S18>/IC' */
    if (TX_Transmission_DW.IC_FirstOutputTime_p) {
      TX_Transmission_DW.IC_FirstOutputTime_p = false;

      /* InitialCondition: '<S18>/IC' */
      TX_Transmission_B.IC_i = TX_Transmission_P.IC_Value_d;
    } else {
      /* InitialCondition: '<S18>/IC' incorporates:
       *  Constant: '<S18>/Constant1'
       */
      TX_Transmission_B.IC_i = TX_Transmission_P.Constant1_Value_o;
    }

    /* End of InitialCondition: '<S18>/IC' */
  }

  /* Switch: '<S20>/Switch' */
  if (TX_Transmission_B.IC > TX_Transmission_P.Switch_Threshold_i) {
    /* Switch: '<S20>/Switch' */
    TX_Transmission_B.Switch = TX_Transmission_X.w;
  } else {
    /* Switch: '<S20>/Switch' incorporates:
     *  Constant: '<S20>/Constant'
     */
    TX_Transmission_B.Switch = TX_Transmission_P.VEH.InitialLongVel / TX_Transmission_P.StatLdWhlR[0] *
                               (TX_Transmission_P.ratio_diff_front + TX_Transmission_P.ratio_diff_rear) / 2.0 *
                               TX_Transmission_P.init_gear_ratio;
  }

  /* End of Switch: '<S20>/Switch' */

  /* Switch: '<S18>/Switch' */
  if (TX_Transmission_B.IC_i > TX_Transmission_P.Switch_Threshold_d) {
    /* Switch: '<S18>/Switch' */
    TX_Transmission_B.Switch_h = TX_Transmission_X.w;
  } else {
    /* Switch: '<S18>/Switch' incorporates:
     *  Constant: '<S18>/Constant'
     */
    TX_Transmission_B.Switch_h = TX_Transmission_P.EngIdleSpd / 60.0 * 2.0 * 3.1415926;
  }

  /* End of Switch: '<S18>/Switch' */
  if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
    /* InitialCondition: '<S16>/IC' */
    if (TX_Transmission_DW.IC_FirstOutputTime_o) {
      TX_Transmission_DW.IC_FirstOutputTime_o = false;

      /* InitialCondition: '<S16>/IC' */
      TX_Transmission_B.IC_e = TX_Transmission_P.IC_Value_n;
    } else {
      /* InitialCondition: '<S16>/IC' incorporates:
       *  Constant: '<S16>/Constant1'
       */
      TX_Transmission_B.IC_e = TX_Transmission_P.Constant1_Value_e;
    }

    /* End of InitialCondition: '<S16>/IC' */
  }

  /* Switch: '<S16>/Switch' */
  if (TX_Transmission_B.IC_e > TX_Transmission_P.Switch_Threshold_o) {
    /* Switch: '<S16>/Switch' */
    TX_Transmission_B.Switch_j = TX_Transmission_X.we;
  } else {
    /* Switch: '<S16>/Switch' incorporates:
     *  Constant: '<S16>/Constant'
     */
    TX_Transmission_B.Switch_j = TX_Transmission_P.VEH.InitialLongVel / TX_Transmission_P.StatLdWhlR[0] *
                                 (TX_Transmission_P.ratio_diff_front + TX_Transmission_P.ratio_diff_rear) / 2.0 *
                                 TX_Transmission_P.init_gear_ratio;
  }

  /* End of Switch: '<S16>/Switch' */

  /* If: '<S13>/If' */
  rtPrevAction = TX_Transmission_DW.If_ActiveSubsystem;
  if (rtsiIsModeUpdateTimeStep(&(&TX_Transmission_M)->solverInfo)) {
    rtAction = static_cast<int8_T>(!(TX_Transmission_B.DeadZone != 0.0));
    TX_Transmission_DW.If_ActiveSubsystem = rtAction;
  } else {
    rtAction = TX_Transmission_DW.If_ActiveSubsystem;
  }

  switch (rtAction) {
    case 0: {
      real_T rtb_Product6;
      real_T rtb_Sum_a;
      real_T rtb_SumofElements;
      real_T tmp;
      if (rtAction != rtPrevAction) {
        /* InitializeConditions for IfAction SubSystem: '<S13>/Locked' incorporates:
         *  ActionPort: '<S17>/Action'
         */
        /* InitializeConditions for If: '<S13>/If' incorporates:
         *  Integrator: '<S17>/x'
         */
        if (rtmIsFirstInitCond((&TX_Transmission_M))) {
          TX_Transmission_X.w = 0.0;
        }

        TX_Transmission_DW.x_IWORK = 1;

        /* End of InitializeConditions for SubSystem: '<S13>/Locked' */
      }

      /* Outputs for IfAction SubSystem: '<S13>/Locked' incorporates:
       *  ActionPort: '<S17>/Action'
       */
      /* Lookup_n-D: '<S31>/Gear2Ratios' incorporates:
       *  Integrator: '<S15>/Integrator'
       */
      rtb_Merge = look1_binlxpw(TX_Transmission_X.Integrator_CSTATE, TX_Transmission_P.Trans.GearNums,
                                TX_Transmission_P.Trans.GearRatios, 8U);

      /* Abs: '<S17>/Abs' */
      CalculateDownshiftThreshold = std::abs(rtb_Merge);

      /* Lookup_n-D: '<S40>/Eta 1D' incorporates:
       *  Integrator: '<S15>/Integrator'
       */
      rtb_SumofElements = look1_binlcpw(TX_Transmission_X.Integrator_CSTATE, TX_Transmission_P.Trans.GearNums,
                                        TX_Transmission_P.Trans.Efficiencies, 8U);

      /* Integrator: '<S17>/x' */
      if (TX_Transmission_DW.x_IWORK != 0) {
        TX_Transmission_X.w = TX_Transmission_B.Switch_j;
        rtsiSetContTimeOutputInconsistentWithStateAtMajorStep(&(&TX_Transmission_M)->solverInfo, true);
      }

      /* Abs: '<S32>/Abs' incorporates:
       *  Abs: '<S34>/Abs'
       *  Integrator: '<S17>/x'
       */
      rtb_Product6 = std::abs(TX_Transmission_X.w);

      /* Gain: '<S33>/Gain' incorporates:
       *  Abs: '<S32>/Abs'
       *  Constant: '<S33>/Constant1'
       *  Gain: '<S33>/Gain1'
       *  Gain: '<S33>/Gain2'
       *  Sum: '<S33>/Subtract1'
       *  Sum: '<S33>/Subtract2'
       *  Trigonometry: '<S33>/Trigonometric Function'
       */
      rtb_Sum_a = (std::tanh((TX_Transmission_P.Gain1_Gain * rtb_Product6 - TX_Transmission_P.Constant1_Value) *
                             TX_Transmission_P.Gain2_Gain) +
                   TX_Transmission_P.Constant1_Value) *
                  TX_Transmission_P.Gain_Gain;

      /* Product: '<S17>/Product5' incorporates:
       *  UnaryMinus: '<S51>/Unary Minus'
       */
      rtb_Add_h = -rtb_Add_h / rtb_Merge;

      /* Gain: '<S35>/Gain' incorporates:
       *  Constant: '<S35>/Constant1'
       *  Gain: '<S35>/Gain1'
       *  Gain: '<S35>/Gain2'
       *  Sum: '<S35>/Subtract1'
       *  Sum: '<S35>/Subtract2'
       *  Trigonometry: '<S35>/Trigonometric Function'
       */
      rtb_Product6 = (std::tanh((TX_Transmission_P.Gain1_Gain_o * rtb_Product6 - TX_Transmission_P.Constant1_Value_h) *
                                TX_Transmission_P.Gain2_Gain_j) +
                      TX_Transmission_P.Constant1_Value_h) *
                     TX_Transmission_P.Gain_Gain_i;

      /* Switch: '<S28>/Switch' incorporates:
       *  Constant: '<S28>/Constant'
       *  Integrator: '<S17>/x'
       *  Product: '<S28>/Product1'
       */
      if (CalculateUpshiftThreshold * TX_Transmission_X.w > TX_Transmission_P.Switch_Threshold) {
        tmp = rtb_SumofElements;
      } else {
        tmp = TX_Transmission_P.Constant_Value;
      }

      /* Switch: '<S29>/Switch' incorporates:
       *  Constant: '<S29>/Constant'
       *  Integrator: '<S17>/x'
       *  Product: '<S29>/Product1'
       */
      if (!(rtb_Add_h * TX_Transmission_X.w > TX_Transmission_P.Switch_Threshold_b)) {
        rtb_SumofElements = TX_Transmission_P.Constant_Value_p;
      }

      /* Product: '<S17>/Product8' incorporates:
       *  Constant: '<S32>/Constant'
       *  Constant: '<S33>/Constant1'
       *  Constant: '<S34>/Constant'
       *  Constant: '<S35>/Constant1'
       *  Integrator: '<S15>/Integrator'
       *  Integrator: '<S17>/x'
       *  Lookup_n-D: '<S31>/Gear2damping'
       *  Lookup_n-D: '<S31>/Gear2inertias'
       *  Product: '<S17>/Product1'
       *  Product: '<S17>/Product3'
       *  Product: '<S17>/Product6'
       *  Product: '<S28>/Product4'
       *  Product: '<S29>/Product4'
       *  Product: '<S32>/Product3'
       *  Product: '<S32>/Product5'
       *  Product: '<S34>/Product3'
       *  Product: '<S34>/Product5'
       *  Sum: '<S17>/Sum'
       *  Sum: '<S17>/Sum1'
       *  Sum: '<S32>/Subtract1'
       *  Sum: '<S33>/Subtract'
       *  Sum: '<S34>/Subtract1'
       *  Sum: '<S35>/Subtract'
       *  Switch: '<S28>/Switch'
       *  Switch: '<S29>/Switch'
       */
      TX_Transmission_B.Product8_m =
          ((((TX_Transmission_P.Constant1_Value - rtb_Sum_a) * TX_Transmission_P.Constant_Value_k + tmp * rtb_Sum_a) *
                CalculateUpshiftThreshold +
            ((TX_Transmission_P.Constant1_Value_h - rtb_Product6) * TX_Transmission_P.Constant_Value_m +
             rtb_SumofElements * rtb_Product6) *
                rtb_Add_h) -
           1.0 / CalculateDownshiftThreshold / CalculateDownshiftThreshold *
               look1_binlxpw(rtb_Integrator, TX_Transmission_P.Trans.GearNums, TX_Transmission_P.Trans.DampingCoeffs,
                             8U) *
               TX_Transmission_X.w) *
          (1.0 /
           (look1_binlxpw(rtb_Integrator, TX_Transmission_P.Trans.GearNums, TX_Transmission_P.Trans.Inertias, 8U) /
            CalculateDownshiftThreshold / CalculateDownshiftThreshold));

      /* SignalConversion: '<S17>/Signal Conversion2' incorporates:
       *  Integrator: '<S17>/x'
       */
      rtb_Add_h = TX_Transmission_X.w;

      /* SignalConversion: '<S17>/Signal Conversion3' incorporates:
       *  Integrator: '<S17>/x'
       *  Product: '<S17>/Product2'
       */
      rtb_Merge = TX_Transmission_X.w / rtb_Merge;

      /* End of Outputs for SubSystem: '<S13>/Locked' */
    } break;

    case 1: {
      real_T rtb_Sum_a;
      real_T rtb_SumofElements;
      uint32_T Product4_tmp;
      if (rtAction != rtPrevAction) {
        /* InitializeConditions for IfAction SubSystem: '<S13>/Unlocked' incorporates:
         *  ActionPort: '<S19>/Action'
         */
        /* InitializeConditions for If: '<S13>/If' incorporates:
         *  Integrator: '<S19>/xe'
         *  Integrator: '<S19>/xv'
         */
        if (rtmIsFirstInitCond((&TX_Transmission_M))) {
          TX_Transmission_X.we = 0.0;
          TX_Transmission_X.wv = 0.0;
        }

        TX_Transmission_DW.xe_IWORK = 1;
        TX_Transmission_DW.xv_IWORK = 1;

        /* End of InitializeConditions for SubSystem: '<S13>/Unlocked' */
      }

      /* Outputs for IfAction SubSystem: '<S13>/Unlocked' incorporates:
       *  ActionPort: '<S19>/Action'
       */
      /* Lookup_n-D: '<S43>/Gear2Ratios' incorporates:
       *  Integrator: '<S15>/Integrator'
       */
      rtb_Merge = look1_binlxpw(TX_Transmission_X.Integrator_CSTATE, TX_Transmission_P.Trans.GearNums,
                                TX_Transmission_P.Trans.GearRatios, 8U);

      /* Abs: '<S19>/Abs' */
      CalculateDownshiftThreshold = std::abs(rtb_Merge);

      /* Integrator: '<S19>/xe' */
      if (TX_Transmission_DW.xe_IWORK != 0) {
        TX_Transmission_X.we = TX_Transmission_B.Switch;
        rtsiSetContTimeOutputInconsistentWithStateAtMajorStep(&(&TX_Transmission_M)->solverInfo, true);
      }

      /* Integrator: '<S19>/xv' */
      if (TX_Transmission_DW.xv_IWORK != 0) {
        TX_Transmission_X.wv = TX_Transmission_B.Switch_h;
      }

      /* Product: '<S19>/Product5' incorporates:
       *  UnaryMinus: '<S51>/Unary Minus'
       */
      rtb_Add_h = -rtb_Add_h / rtb_Merge;

      /* Switch: '<S41>/Switch' incorporates:
       *  Constant: '<S41>/Constant'
       *  Integrator: '<S15>/Integrator'
       *  Integrator: '<S19>/xv'
       *  Lookup_n-D: '<S50>/Eta 1D'
       *  Product: '<S41>/Product1'
       */
      if (rtb_Add_h * TX_Transmission_X.wv > TX_Transmission_P.Switch_Threshold_l) {
        rtb_SumofElements =
            look1_binlcpw(rtb_Integrator, TX_Transmission_P.Trans.GearNums, TX_Transmission_P.Trans.Efficiencies, 8U);
      } else {
        rtb_SumofElements = TX_Transmission_P.Constant_Value_i;
      }

      /* End of Switch: '<S41>/Switch' */

      /* Gain: '<S45>/Gain' incorporates:
       *  Abs: '<S44>/Abs'
       *  Constant: '<S45>/Constant1'
       *  Gain: '<S45>/Gain1'
       *  Gain: '<S45>/Gain2'
       *  Integrator: '<S19>/xv'
       *  Sum: '<S45>/Subtract1'
       *  Sum: '<S45>/Subtract2'
       *  Trigonometry: '<S45>/Trigonometric Function'
       */
      rtb_Sum_a = (std::tanh((TX_Transmission_P.Gain1_Gain_m * std::abs(TX_Transmission_X.wv) -
                              TX_Transmission_P.Constant1_Value_l) *
                             TX_Transmission_P.Gain2_Gain_o) +
                   TX_Transmission_P.Constant1_Value_l) *
                  TX_Transmission_P.Gain_Gain_oj;

      /* Lookup_n-D: '<S19>/Gear2inertias' incorporates:
       *  Constant: '<S19>/Neutral'
       *  Lookup_n-D: '<S19>/Gear2damping'
       */
      Product4_tmp = plook_u32d_binckan(TX_Transmission_P.Neutral_Value, TX_Transmission_P.Trans.GearNums, 8U);

      /* Product: '<S19>/Product4' incorporates:
       *  Constant: '<S19>/First'
       *  Integrator: '<S19>/xe'
       *  Lookup_n-D: '<S19>/Gear2damping'
       *  Lookup_n-D: '<S19>/Gear2inertias'
       *  Lookup_n-D: '<S19>/Gear2inertias1'
       *  Product: '<S19>/Product7'
       *  Sum: '<S19>/Subtract'
       *  Sum: '<S19>/Sum1'
       */
      TX_Transmission_B.Product4 =
          1.0 /
          (TX_Transmission_P.Trans
               .Inertias[plook_u32d_binckan(TX_Transmission_P.First_Value, TX_Transmission_P.Trans.GearNums, 8U)] -
           TX_Transmission_P.Trans.Inertias[Product4_tmp]) *
          (CalculateUpshiftThreshold - TX_Transmission_P.Trans.DampingCoeffs[Product4_tmp] * TX_Transmission_X.we);

      /* Product: '<S19>/Product8' incorporates:
       *  Constant: '<S44>/Constant'
       *  Constant: '<S45>/Constant1'
       *  Integrator: '<S15>/Integrator'
       *  Integrator: '<S19>/xv'
       *  Lookup_n-D: '<S43>/Gear2damping'
       *  Lookup_n-D: '<S43>/Gear2inertias'
       *  Product: '<S19>/Product1'
       *  Product: '<S19>/Product3'
       *  Product: '<S19>/Product6'
       *  Product: '<S41>/Product4'
       *  Product: '<S44>/Product3'
       *  Product: '<S44>/Product5'
       *  Sum: '<S19>/Sum'
       *  Sum: '<S44>/Subtract1'
       *  Sum: '<S45>/Subtract'
       */
      TX_Transmission_B.Product8 =
          (((TX_Transmission_P.Constant1_Value_l - rtb_Sum_a) * TX_Transmission_P.Constant_Value_f +
            rtb_SumofElements * rtb_Sum_a) *
               rtb_Add_h -
           1.0 / CalculateDownshiftThreshold / CalculateDownshiftThreshold *
               look1_binlxpw(rtb_Integrator, TX_Transmission_P.Trans.GearNums, TX_Transmission_P.Trans.DampingCoeffs,
                             8U) *
               TX_Transmission_X.wv) *
          (1.0 /
           (look1_binlxpw(rtb_Integrator, TX_Transmission_P.Trans.GearNums, TX_Transmission_P.Trans.Inertias, 8U) /
            CalculateDownshiftThreshold / CalculateDownshiftThreshold));

      /* SignalConversion: '<S19>/Signal Conversion' incorporates:
       *  Integrator: '<S19>/xv'
       *  Product: '<S19>/Product2'
       */
      rtb_Merge = TX_Transmission_X.wv / rtb_Merge;

      /* SignalConversion: '<S19>/Signal Conversion1' incorporates:
       *  Integrator: '<S19>/xe'
       */
      rtb_Add_h = TX_Transmission_X.we;

      /* End of Outputs for SubSystem: '<S13>/Unlocked' */
    } break;
  }

  /* End of If: '<S13>/If' */

  /* Switch: '<S12>/Switch' */
  if (!(TX_Transmission_B.Memory_d != 0.0)) {
    rtb_Switch_h = TX_Transmission_B.Constant;
  }

  /* Product: '<S9>/Divide' incorporates:
   *  Constant: '<S9>/Constant'
   *  Sum: '<S9>/Sum'
   *  Switch: '<S12>/Switch'
   */
  TX_Transmission_B.Divide = (rtb_Add_h - rtb_Switch_h) * TX_Transmission_P.ContLPFIC_wc;
  if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
    /* Gain: '<S13>/2*pi' incorporates:
     *  Constant: '<S13>/Constant'
     *  Product: '<S13>/Product'
     */
    TX_Transmission_B.upi = 1.0 / TX_Transmission_P.Trans.ShiftTau * TX_Transmission_P.upi_Gain;

    /* Saturate: '<S13>/Saturation' */
    if (TX_Transmission_B.y > TX_Transmission_P.Saturation_UpperSat_k) {
      /* Saturate: '<S13>/Saturation' */
      TX_Transmission_B.Saturation = TX_Transmission_P.Saturation_UpperSat_k;
    } else if (TX_Transmission_B.y < TX_Transmission_P.Saturation_LowerSat_e) {
      /* Saturate: '<S13>/Saturation' */
      TX_Transmission_B.Saturation = TX_Transmission_P.Saturation_LowerSat_e;
    } else {
      /* Saturate: '<S13>/Saturation' */
      TX_Transmission_B.Saturation = TX_Transmission_B.y;
    }

    /* End of Saturate: '<S13>/Saturation' */
  }

  /* Switch: '<S15>/Switch' */
  if (!(TX_Transmission_B.Memory != 0.0)) {
    rtb_Integrator = TX_Transmission_B.Constant1;
  }

  /* Product: '<S15>/Product' incorporates:
   *  Sum: '<S15>/Sum'
   *  Switch: '<S15>/Switch'
   */
  TX_Transmission_B.Product =
      (static_cast<real_T>(TX_Transmission_B.Saturation) - rtb_Integrator) * TX_Transmission_B.upi;

  /* Sum: '<S51>/Subtract' incorporates:
   *  Inport: '<Root>/PropShftSpd'
   */
  TX_Transmission_B.Subtract = rtb_Merge - TX_Transmission_U.PropShftSpd;

  /* Switch: '<S52>/Switch' */
  if (!(TX_Transmission_B.Memory_c != 0.0)) {
    rtb_Switch_l = TX_Transmission_B.domega_o;
  }

  /* Product: '<S52>/Product' incorporates:
   *  Constant: '<S51>/omega_c'
   *  Sum: '<S52>/Sum'
   *  Switch: '<S52>/Switch'
   */
  TX_Transmission_B.Product_i = (TX_Transmission_B.Subtract - rtb_Switch_l) * TX_Transmission_P.PropShaft_omega_c;

  /* Product: '<S64>/Divide' incorporates:
   *  Constant: '<S64>/Constant'
   *  Gain: '<S59>/Gain'
   *  Gain: '<S59>/kph'
   *  Inport: '<Root>/DriveLineIn'
   *  Sum: '<S59>/Add'
   *  Sum: '<S64>/Sum'
   */
  TX_Transmission_B.Divide_f =
      ((((TX_Transmission_U.DriveLineIn.wheel_spd_rad_s[0] + TX_Transmission_U.DriveLineIn.wheel_spd_rad_s[1]) +
         TX_Transmission_U.DriveLineIn.wheel_spd_rad_s[2]) +
        TX_Transmission_U.DriveLineIn.wheel_spd_rad_s[3]) *
           (TX_Transmission_P.StatLdWhlR[0] / 4.0) * TX_Transmission_P.kph_Gain -
       TX_Transmission_B.Integrator1) *
      TX_Transmission_P.Valve_wc;
  if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
    if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
      /* Update for Memory: '<S15>/Memory' incorporates:
       *  Constant: '<S15>/Reset'
       */
      TX_Transmission_DW.Memory_PreviousInput = TX_Transmission_P.Reset_Value_j;

      /* Update for Memory: '<S12>/Memory' incorporates:
       *  Constant: '<S12>/Reset'
       */
      TX_Transmission_DW.Memory_PreviousInput_h = TX_Transmission_P.Reset_Value;
    }

    /* Update for Integrator: '<S15>/Integrator' */
    TX_Transmission_DW.Integrator_IWORK = 0;

    /* Update for Integrator: '<S9>/Integrator' */
    TX_Transmission_DW.Integrator_IWORK_o = 0;
    if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
      /* Update for Memory: '<S52>/Memory' incorporates:
       *  Constant: '<S52>/Reset'
       */
      TX_Transmission_DW.Memory_PreviousInput_o = TX_Transmission_P.Reset_Value_l;

      /* Update for UnitDelay: '<S6>/Delay Input1' */
      TX_Transmission_DW.DelayInput1_DSTATE = rtb_DataTypeConversion;
    }

    /* Update for Integrator: '<S52>/Integrator' */
    TX_Transmission_DW.Integrator_IWORK_i = 0;

    /* Update for If: '<S13>/If' */
    switch (TX_Transmission_DW.If_ActiveSubsystem) {
      case 0:
        /* Update for IfAction SubSystem: '<S13>/Locked' incorporates:
         *  ActionPort: '<S17>/Action'
         */
        /* Update for Integrator: '<S17>/x' */
        TX_Transmission_DW.x_IWORK = 0;

        /* End of Update for SubSystem: '<S13>/Locked' */
        break;

      case 1:
        /* Update for IfAction SubSystem: '<S13>/Unlocked' incorporates:
         *  ActionPort: '<S19>/Action'
         */
        /* Update for Integrator: '<S19>/xe' */
        TX_Transmission_DW.xe_IWORK = 0;

        /* Update for Integrator: '<S19>/xv' */
        TX_Transmission_DW.xv_IWORK = 0;

        /* End of Update for SubSystem: '<S13>/Unlocked' */
        break;
    }

    /* End of Update for If: '<S13>/If' */

    /* ContTimeOutputInconsistentWithStateAtMajorOutputFlag is set, need to run a minor output */
    if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
      if (rtsiGetContTimeOutputInconsistentWithStateAtMajorStep(&(&TX_Transmission_M)->solverInfo)) {
        rtsiSetSimTimeStep(&(&TX_Transmission_M)->solverInfo, MINOR_TIME_STEP);
        rtsiSetContTimeOutputInconsistentWithStateAtMajorStep(&(&TX_Transmission_M)->solverInfo, false);
        TX_Transmission::step();
        rtsiSetSimTimeStep(&(&TX_Transmission_M)->solverInfo, MAJOR_TIME_STEP);
      }
    }
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&TX_Transmission_M))) {
    rt_ertODEUpdateContinuousStates(&(&TX_Transmission_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&TX_Transmission_M)->Timing.clockTick0)) {
      ++(&TX_Transmission_M)->Timing.clockTickH0;
    }

    (&TX_Transmission_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&TX_Transmission_M)->solverInfo);

    {
      /* Update absolute timer for sample time: [0.001s, 0.0s] */
      /* The "clockTick1" counts the number of times the code of this task has
       * been executed. The resolution of this integer timer is 0.001, which is the step size
       * of the task. Size of "clockTick1" ensures timer will not overflow during the
       * application lifespan selected.
       * Timer of this task consists of two 32 bit unsigned integers.
       * The two integers represent the low bits Timing.clockTick1 and the high bits
       * Timing.clockTickH1. When the low bit overflows to 0, the high bits increment.
       */
      (&TX_Transmission_M)->Timing.clockTick1++;
      if (!(&TX_Transmission_M)->Timing.clockTick1) {
        (&TX_Transmission_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void TX_Transmission::TX_Transmission_derivatives() {
  XDot_TX_Transmission_T *_rtXdot;
  _rtXdot = ((XDot_TX_Transmission_T *)(&TX_Transmission_M)->derivs);

  /* Derivatives for Integrator: '<S15>/Integrator' */
  _rtXdot->Integrator_CSTATE = TX_Transmission_B.Product;

  /* Derivatives for Integrator: '<S9>/Integrator' */
  _rtXdot->Integrator_CSTATE_h = TX_Transmission_B.Divide;

  /* Derivatives for Integrator: '<S52>/Integrator' */
  _rtXdot->Integrator_CSTATE_c = TX_Transmission_B.Product_i;

  /* Derivatives for Integrator: '<S51>/Integrator' */
  _rtXdot->Integrator_CSTATE_hz = TX_Transmission_B.Subtract;

  /* Derivatives for Integrator: '<S64>/Integrator1' */
  _rtXdot->Integrator1_CSTATE = TX_Transmission_B.Divide_f;

  /* Derivatives for If: '<S13>/If' */
  ((XDot_TX_Transmission_T *)(&TX_Transmission_M)->derivs)->w = 0.0;

  {
    real_T *dx;
    int_T i;
    dx = &(((XDot_TX_Transmission_T *)(&TX_Transmission_M)->derivs)->we);
    for (i = 0; i < 2; i++) {
      dx[i] = 0.0;
    }
  }

  switch (TX_Transmission_DW.If_ActiveSubsystem) {
    case 0:
      /* Derivatives for IfAction SubSystem: '<S13>/Locked' incorporates:
       *  ActionPort: '<S17>/Action'
       */
      /* Derivatives for Integrator: '<S17>/x' */
      _rtXdot->w = TX_Transmission_B.Product8_m;

      /* End of Derivatives for SubSystem: '<S13>/Locked' */
      break;

    case 1:
      /* Derivatives for IfAction SubSystem: '<S13>/Unlocked' incorporates:
       *  ActionPort: '<S19>/Action'
       */
      /* Derivatives for Integrator: '<S19>/xe' */
      _rtXdot->we = TX_Transmission_B.Product4;

      /* Derivatives for Integrator: '<S19>/xv' */
      _rtXdot->wv = TX_Transmission_B.Product8;

      /* End of Derivatives for SubSystem: '<S13>/Unlocked' */
      break;
  }

  /* End of Derivatives for If: '<S13>/If' */
}

/* Model initialize function */
void TX_Transmission::initialize() {
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* non-finite (run-time) assignments */
  TX_Transmission_P.Saturation1_LowerSat = rtMinusInf;
  TX_Transmission_P.Saturation2_UpperSat = rtInf;
  TX_Transmission_P.Saturation1_LowerSat_j = rtMinusInf;
  TX_Transmission_P.Saturation2_UpperSat_l = rtInf;

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&TX_Transmission_M)->solverInfo, &(&TX_Transmission_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&TX_Transmission_M)->solverInfo, &rtmGetTPtr((&TX_Transmission_M)));
    rtsiSetStepSizePtr(&(&TX_Transmission_M)->solverInfo, &(&TX_Transmission_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&TX_Transmission_M)->solverInfo, &(&TX_Transmission_M)->derivs);
    rtsiSetContStatesPtr(&(&TX_Transmission_M)->solverInfo, (real_T **)&(&TX_Transmission_M)->contStates);
    rtsiSetNumContStatesPtr(&(&TX_Transmission_M)->solverInfo, &(&TX_Transmission_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&TX_Transmission_M)->solverInfo,
                                    &(&TX_Transmission_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&TX_Transmission_M)->solverInfo,
                                       &(&TX_Transmission_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&TX_Transmission_M)->solverInfo,
                                      &(&TX_Transmission_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&TX_Transmission_M)->solverInfo, (&rtmGetErrorStatus((&TX_Transmission_M))));
    rtsiSetRTModelPtr(&(&TX_Transmission_M)->solverInfo, (&TX_Transmission_M));
  }

  rtsiSetSimTimeStep(&(&TX_Transmission_M)->solverInfo, MAJOR_TIME_STEP);
  (&TX_Transmission_M)->intgData.y = (&TX_Transmission_M)->odeY;
  (&TX_Transmission_M)->intgData.f[0] = (&TX_Transmission_M)->odeF[0];
  (&TX_Transmission_M)->intgData.f[1] = (&TX_Transmission_M)->odeF[1];
  (&TX_Transmission_M)->intgData.f[2] = (&TX_Transmission_M)->odeF[2];
  (&TX_Transmission_M)->intgData.f[3] = (&TX_Transmission_M)->odeF[3];
  (&TX_Transmission_M)->contStates = ((X_TX_Transmission_T *)&TX_Transmission_X);
  rtsiSetSolverData(&(&TX_Transmission_M)->solverInfo, static_cast<void *>(&(&TX_Transmission_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&TX_Transmission_M)->solverInfo, false);
  rtsiSetSolverName(&(&TX_Transmission_M)->solverInfo, "ode4");
  rtmSetTPtr((&TX_Transmission_M), &(&TX_Transmission_M)->Timing.tArray[0]);
  (&TX_Transmission_M)->Timing.stepSize0 = 0.001;
  rtmSetFirstInitCond((&TX_Transmission_M), 1);

  /* Start for Constant: '<S13>/Constant1' */
  TX_Transmission_B.Constant1 = TX_Transmission_P.init_gear_num;

  /* Start for Constant: '<S8>/Constant' */
  TX_Transmission_B.Constant = TX_Transmission_P.VEH.InitialLongVel / TX_Transmission_P.StatLdWhlR[0] *
                               (TX_Transmission_P.ratio_diff_front + TX_Transmission_P.ratio_diff_rear) / 2.0 *
                               TX_Transmission_P.init_gear_ratio;

  /* Start for Constant: '<S51>/domega_o' */
  TX_Transmission_B.domega_o = TX_Transmission_P.PropShaft_domega_o;

  /* Start for InitialCondition: '<S20>/IC' */
  TX_Transmission_DW.IC_FirstOutputTime = true;

  /* Start for InitialCondition: '<S18>/IC' */
  TX_Transmission_DW.IC_FirstOutputTime_p = true;

  /* Start for InitialCondition: '<S16>/IC' */
  TX_Transmission_DW.IC_FirstOutputTime_o = true;

  /* Start for If: '<S13>/If' */
  TX_Transmission_DW.If_ActiveSubsystem = -1;
  TX_Transmission_PrevZCX.Integrator_Reset_ZCE = UNINITIALIZED_ZCSIG;
  TX_Transmission_PrevZCX.Integrator_Reset_ZCE_e = UNINITIALIZED_ZCSIG;
  TX_Transmission_PrevZCX.Integrator_Reset_ZCE_c = UNINITIALIZED_ZCSIG;

  /* InitializeConditions for Memory: '<S15>/Memory' */
  TX_Transmission_DW.Memory_PreviousInput = TX_Transmission_P.Memory_InitialCondition;

  /* InitializeConditions for Integrator: '<S15>/Integrator' incorporates:
   *  Integrator: '<S9>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_Transmission_M))) {
    TX_Transmission_X.Integrator_CSTATE = 0.0;
    TX_Transmission_X.Integrator_CSTATE_h = 388.26121428571429;
  }

  TX_Transmission_DW.Integrator_IWORK = 1;

  /* End of InitializeConditions for Integrator: '<S15>/Integrator' */

  /* InitializeConditions for Memory: '<S12>/Memory' */
  TX_Transmission_DW.Memory_PreviousInput_h = TX_Transmission_P.Memory_InitialCondition_d;

  /* InitializeConditions for Integrator: '<S9>/Integrator' */
  TX_Transmission_DW.Integrator_IWORK_o = 1;

  /* InitializeConditions for Memory: '<S52>/Memory' */
  TX_Transmission_DW.Memory_PreviousInput_o = TX_Transmission_P.Memory_InitialCondition_o;

  /* SystemInitialize for IfAction SubSystem: '<S13>/Locked' */
  /* InitializeConditions for Integrator: '<S52>/Integrator' incorporates:
   *  Integrator: '<S17>/x'
   */
  if (rtmIsFirstInitCond((&TX_Transmission_M))) {
    TX_Transmission_X.Integrator_CSTATE_c = 0.0;
    TX_Transmission_X.w = 0.0;
  }

  /* End of SystemInitialize for SubSystem: '<S13>/Locked' */
  TX_Transmission_DW.Integrator_IWORK_i = 1;

  /* End of InitializeConditions for Integrator: '<S52>/Integrator' */

  /* InitializeConditions for Integrator: '<S51>/Integrator' */
  TX_Transmission_X.Integrator_CSTATE_hz = TX_Transmission_P.PropShaft_theta_o;

  /* InitializeConditions for Integrator: '<S64>/Integrator1' */
  TX_Transmission_X.Integrator1_CSTATE = TX_Transmission_P.Integrator1_IC;

  /* InitializeConditions for UnitDelay: '<S6>/Delay Input1' */
  TX_Transmission_DW.DelayInput1_DSTATE = TX_Transmission_P.GearCmdChange_vinit;

  /* SystemInitialize for Chart: '<S59>/TCM Shift Controller' */
  TX_Transmission_DW.sfEvent = -1;

  /* SystemInitialize for IfAction SubSystem: '<S13>/Locked' */
  /* InitializeConditions for Integrator: '<S17>/x' */
  TX_Transmission_DW.x_IWORK = 1;

  /* End of SystemInitialize for SubSystem: '<S13>/Locked' */

  /* SystemInitialize for IfAction SubSystem: '<S13>/Unlocked' */
  /* InitializeConditions for Integrator: '<S19>/xe' incorporates:
   *  Integrator: '<S19>/xv'
   */
  if (rtmIsFirstInitCond((&TX_Transmission_M))) {
    TX_Transmission_X.we = 0.0;
    TX_Transmission_X.wv = 0.0;
  }

  TX_Transmission_DW.xe_IWORK = 1;

  /* End of InitializeConditions for Integrator: '<S19>/xe' */

  /* InitializeConditions for Integrator: '<S19>/xv' */
  TX_Transmission_DW.xv_IWORK = 1;

  /* End of SystemInitialize for SubSystem: '<S13>/Unlocked' */

  /* set "at time zero" to false */
  if (rtmIsFirstInitCond((&TX_Transmission_M))) {
    rtmSetFirstInitCond((&TX_Transmission_M), 0);
  }
}

/* Model terminate function */
void TX_Transmission::terminate() { /* (no terminate code required) */
}

/* Constructor */
TX_Transmission::TX_Transmission()
    : TX_Transmission_U(),
      TX_Transmission_Y(),
      TX_Transmission_B(),
      TX_Transmission_DW(),
      TX_Transmission_X(),
      TX_Transmission_PrevZCX(),
      TX_Transmission_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
TX_Transmission::~TX_Transmission() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_TX_Transmission_T *TX_Transmission::getRTM() { return (&TX_Transmission_M); }
