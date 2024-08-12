/*
 * Chassis.cpp
 *
 * Code generation for model "Chassis".
 *
 * Model version              : 2.33
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Thu Feb 16 17:07:18 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "Chassis.h"
#include <emmintrin.h>
#include <cmath>
#include <cstring>
#include "Chassis_private.h"
#include "rt_defines.h"
#include "rtwtypes.h"
#include "zero_crossing_types.h"

extern "C" {

#include "rt_nonfinite.h"
}

/* Named constants for Chart: '<S241>/Clutch' */
const uint8_T Chassis_IN_Locked{1U};

const uint8_T Chassis_IN_Slipping{2U};

real_T look1_binlcpw(real_T u0, const real_T bp0[], const real_T table[], uint32_T maxIndex) {
  real_T frac;
  real_T yL_0d0;
  uint32_T iLeft;

  /* Column-major Lookup 1-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = 0.0;
  } else if (u0 < bp0[maxIndex]) {
    uint32_T bpIdx;
    uint32_T iRght;

    /* Binary Search */
    bpIdx = maxIndex >> 1U;
    iLeft = 0U;
    iRght = maxIndex;
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
    iLeft = maxIndex - 1U;
    frac = 1.0;
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'portable wrapping'
   */
  yL_0d0 = table[iLeft];
  return (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
}

real_T look1_binlxpw(real_T u0, const real_T bp0[], const real_T table[], uint32_T maxIndex) {
  real_T frac;
  real_T yL_0d0;
  uint32_T iLeft;

  /* Column-major Lookup 1-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Linear'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Linear'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = (u0 - bp0[0U]) / (bp0[1U] - bp0[0U]);
  } else if (u0 < bp0[maxIndex]) {
    uint32_T bpIdx;
    uint32_T iRght;

    /* Binary Search */
    bpIdx = maxIndex >> 1U;
    iLeft = 0U;
    iRght = maxIndex;
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
    iLeft = maxIndex - 1U;
    frac = (u0 - bp0[maxIndex - 1U]) / (bp0[maxIndex] - bp0[maxIndex - 1U]);
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'portable wrapping'
   */
  yL_0d0 = table[iLeft];
  return (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
}

real_T look1_pbinlcapw(real_T u0, const real_T bp0[], const real_T table[], uint32_T prevIndex[], uint32_T maxIndex) {
  real_T frac;
  real_T y;
  uint32_T bpIdx;

  /* Column-major Lookup 1-D
     Search method: 'binary'
     Use previous index: 'on'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    bpIdx = 0U;
    frac = 0.0;
  } else if (u0 < bp0[maxIndex]) {
    uint32_T found;
    uint32_T iLeft;
    uint32_T iRght;

    /* Binary Search using Previous Index */
    bpIdx = prevIndex[0U];
    iLeft = 0U;
    iRght = maxIndex;
    found = 0U;
    while (found == 0U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u0 < bp0[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + iRght) + 1U) >> 1U;
      }
    }

    frac = (u0 - bp0[bpIdx]) / (bp0[bpIdx + 1U] - bp0[bpIdx]);
  } else {
    bpIdx = maxIndex;
    frac = 0.0;
  }

  prevIndex[0U] = bpIdx;

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Overflow mode: 'portable wrapping'
   */
  if (bpIdx == maxIndex) {
    y = table[bpIdx];
  } else {
    real_T yL_0d0;
    yL_0d0 = table[bpIdx];
    y = (table[bpIdx + 1U] - yL_0d0) * frac + yL_0d0;
  }

  return y;
}

real_T look2_pbinlcpw(real_T u0, real_T u1, const real_T bp0[], const real_T bp1[], const real_T table[],
                      uint32_T prevIndex[], const uint32_T maxIndex[], uint32_T stride) {
  real_T fractions[2];
  real_T frac;
  real_T yL_0d0;
  real_T yL_0d1;
  uint32_T bpIndices[2];
  uint32_T bpIdx;
  uint32_T found;
  uint32_T iLeft;
  uint32_T iRght;

  /* Column-major Lookup 2-D
     Search method: 'binary'
     Use previous index: 'on'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    bpIdx = 0U;
    frac = 0.0;
  } else if (u0 < bp0[maxIndex[0U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[0U];
    iLeft = 0U;
    iRght = maxIndex[0U];
    found = 0U;
    while (found == 0U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u0 < bp0[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + iRght) + 1U) >> 1U;
      }
    }

    frac = (u0 - bp0[bpIdx]) / (bp0[bpIdx + 1U] - bp0[bpIdx]);
  } else {
    bpIdx = maxIndex[0U] - 1U;
    frac = 1.0;
  }

  prevIndex[0U] = bpIdx;
  fractions[0U] = frac;
  bpIndices[0U] = bpIdx;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u1 <= bp1[0U]) {
    bpIdx = 0U;
    frac = 0.0;
  } else if (u1 < bp1[maxIndex[1U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[1U];
    iLeft = 0U;
    iRght = maxIndex[1U];
    found = 0U;
    while (found == 0U) {
      if (u1 < bp1[bpIdx]) {
        iRght = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u1 < bp1[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + iRght) + 1U) >> 1U;
      }
    }

    frac = (u1 - bp1[bpIdx]) / (bp1[bpIdx + 1U] - bp1[bpIdx]);
  } else {
    bpIdx = maxIndex[1U] - 1U;
    frac = 1.0;
  }

  prevIndex[1U] = bpIdx;

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'portable wrapping'
   */
  iLeft = bpIdx * stride + bpIndices[0U];
  yL_0d0 = table[iLeft];
  yL_0d0 += (table[iLeft + 1U] - yL_0d0) * fractions[0U];
  iLeft += stride;
  yL_0d1 = table[iLeft];
  return (((table[iLeft + 1U] - yL_0d1) * fractions[0U] + yL_0d1) - yL_0d0) * frac + yL_0d0;
}

/* State reduction function */
void local_stateReduction(real_T *x, int_T *p, int_T n, real_T *r) {
  int_T i, j;
  for (i = 0, j = 0; i < n; ++i, ++j) {
    int_T k{p[i]};

    real_T lb{r[j++]};

    real_T xk{x[k] - lb};

    real_T rk{r[j] - lb};

    int_T q{(int_T)std::floor(xk / rk)};

    if (q) {
      x[k] = xk - q * rk + lb;
    }
  }
}

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void Chassis::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  int_T nXc{65};

  rtsiSetSimTimeStep(si, MINOR_TIME_STEP);

  /* Save the state values at time t in y, we'll use x as ynew. */
  (void)std::memcpy(y, x, static_cast<uint_T>(nXc) * sizeof(real_T));

  /* Assumes that rtsiSetT and ModelOutputs are up-to-date */
  /* f0 = f(t,y) */
  rtsiSetdX(si, f0);
  Chassis_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  Chassis_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  Chassis_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  Chassis_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  local_stateReduction(rtsiGetContStates(si), rtsiGetPeriodicContStateIndices(si), 3,
                       rtsiGetPeriodicContStateRanges(si));
  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Function for Chart: '<S241>/Clutch' */
boolean_T Chassis::Chassis_detectSlip(real_T Tout, real_T Tfmaxs, B_Clutch_Chassis_T *localB) {
  localB->Tout = Tout;
  localB->Tfmaxs = Tfmaxs;

  /* Outputs for Function Call SubSystem: '<S242>/detectSlip' */
  /* RelationalOperator: '<S254>/Relational Operator' incorporates:
   *  Abs: '<S254>/Abs'
   */
  localB->RelationalOperator = (std::abs(localB->Tout) >= localB->Tfmaxs);

  /* End of Outputs for SubSystem: '<S242>/detectSlip' */
  return localB->RelationalOperator;
}

/* Function for Chart: '<S241>/Clutch' */
boolean_T Chassis::Chassis_detectLockup(real_T Tout, real_T Tfmaxs, real_T rtp_br, B_Clutch_Chassis_T *localB,
                                        DW_Clutch_Chassis_T *localDW, P_Clutch_Chassis_T *localP) {
  real_T rtb_Abs_l;
  localB->Tout_p = Tout;
  localB->Tfmaxs_h = Tfmaxs;

  /* Outputs for Function Call SubSystem: '<S242>/detectLockup' */
  /* Gain: '<S251>/Output Damping' incorporates:
   *  Constant: '<S245>/Constant'
   */
  rtb_Abs_l = rtp_br * localP->Constant_Value;

  /* CombinatorialLogic: '<S250>/Combinatorial  Logic' incorporates:
   *  Abs: '<S248>/Abs'
   *  Abs: '<S253>/Abs'
   *  RelationalOperator: '<S248>/Relational Operator'
   *  RelationalOperator: '<S253>/Relational Operator'
   *  Sum: '<S251>/Sum1'
   *  Sum: '<S251>/Sum2'
   *  UnaryMinus: '<S252>/Unary Minus'
   *  UnitDelay: '<S250>/Unit Delay'
   */
  localB->CombinatorialLogic =
      localP->CombinatorialLogic_table[(((std::abs(((0.0 - localB->Tout_p) - rtb_Abs_l) + rtb_Abs_l) >=
                                          localB->Tfmaxs_h) +
                                         (static_cast<uint32_T>(std::abs(-localB->Tout_p) <= localB->Tfmaxs_h) << 1))
                                        << 1) +
                                       localDW->UnitDelay_DSTATE];

  /* Update for UnitDelay: '<S250>/Unit Delay' */
  localDW->UnitDelay_DSTATE = localB->CombinatorialLogic;

  /* End of Outputs for SubSystem: '<S242>/detectLockup' */
  return localB->CombinatorialLogic;
}

/* System initialize for atomic system: '<S241>/Clutch' */
void Chassis::Chassis_Clutch_Init(real_T rtp_omegao, B_Clutch_Chassis_T *localB, DW_Clutch_Chassis_T *localDW,
                                  P_Clutch_Chassis_T *localP, X_Clutch_Chassis_T *localX) {
  /* SystemInitialize for Function Call SubSystem: '<S242>/detectSlip' */
  /* SystemInitialize for RelationalOperator: '<S254>/Relational Operator' incorporates:
   *  Outport: '<S246>/yn'
   */
  localB->RelationalOperator = localP->yn_Y0;

  /* End of SystemInitialize for SubSystem: '<S242>/detectSlip' */

  /* SystemInitialize for Function Call SubSystem: '<S242>/detectLockup' */
  /* InitializeConditions for UnitDelay: '<S250>/Unit Delay' */
  localDW->UnitDelay_DSTATE = localP->UnitDelay_InitialCondition;

  /* SystemInitialize for CombinatorialLogic: '<S250>/Combinatorial  Logic' incorporates:
   *  Outport: '<S245>/yn'
   */
  localB->CombinatorialLogic = localP->yn_Y0_d;

  /* End of SystemInitialize for SubSystem: '<S242>/detectLockup' */

  /* SystemInitialize for IfAction SubSystem: '<S242>/Slipping' */
  /* InitializeConditions for Integrator: '<S244>/omega wheel' */
  localX->omegaWheel = rtp_omegao;

  /* End of SystemInitialize for SubSystem: '<S242>/Slipping' */
}

/* Outputs for atomic system: '<S241>/Clutch' */
void Chassis::Chassis_Clutch(real_T rtu_Tout, real_T rtu_Tfmaxs, real_T rtu_Tfmaxk, real_T rtp_omegao, real_T rtp_br,
                             real_T rtp_Iyy, real_T rtp_OmegaTol, B_Clutch_Chassis_T *localB,
                             DW_Clutch_Chassis_T *localDW, P_Clutch_Chassis_T *localP, X_Clutch_Chassis_T *localX) {
  real_T rtb_OutputSum;
  if (rtsiIsModeUpdateTimeStep(&(&Chassis_M)->solverInfo)) {
    localDW->lastMajorTime = (&Chassis_M)->Timing.t[0];

    /* Chart: '<S241>/Clutch' */
    if (localDW->is_active_c8_autolibshared == 0U) {
      localDW->is_active_c8_autolibshared = 1U;
      localX->omegaWheel = rtp_omegao;
      localDW->is_c8_autolibshared = Chassis_IN_Slipping;

      /* Gain: '<S244>/Output Damping' incorporates:
       *  Integrator: '<S244>/omega wheel'
       */
      rtb_OutputSum = rtp_br * localX->omegaWheel;

      /* Merge: '<S242>/ Merge 3' incorporates:
       *  SignalConversion generated from: '<S244>/Myb'
       */
      localB->Myb = rtb_OutputSum;

      /* Merge: '<S242>/ Merge ' incorporates:
       *  Integrator: '<S244>/omega wheel'
       *  SignalConversion generated from: '<S244>/Omega'
       */
      localB->Omega = localX->omegaWheel;

      /* Gain: '<S244>/Output Inertia' incorporates:
       *  Gain: '<S244>/-4'
       *  Integrator: '<S244>/omega wheel'
       *  Product: '<S244>/Max Dynamic Friction Torque1'
       *  Sum: '<S244>/Output Sum'
       *  Trigonometry: '<S244>/Trigonometric Function'
       */
      localB->OutputInertia =
          ((std::tanh(localP->u_Gain * localX->omegaWheel) * rtu_Tfmaxk - rtu_Tout) - rtb_OutputSum) * (1.0 / rtp_Iyy);

      /* Merge: '<S242>/ Merge 1' incorporates:
       *  SignalConversion generated from: '<S244>/Omegadot'
       */
      localB->Omegadot = localB->OutputInertia;
      localDW->Slipping_entered = true;
      rtsiSetBlockStateForSolverChangedAtMajorStep(&(&Chassis_M)->solverInfo, true);
    } else if (localDW->is_c8_autolibshared == Chassis_IN_Locked) {
      if (Chassis_detectSlip(rtu_Tout, rtu_Tfmaxs, localB)) {
        localX->omegaWheel = 0.0;
        localDW->Locked_entered = false;
        localDW->is_c8_autolibshared = Chassis_IN_Slipping;

        /* Gain: '<S244>/Output Damping' incorporates:
         *  Integrator: '<S244>/omega wheel'
         */
        rtb_OutputSum = rtp_br * localX->omegaWheel;

        /* Merge: '<S242>/ Merge 3' incorporates:
         *  SignalConversion generated from: '<S244>/Myb'
         */
        localB->Myb = rtb_OutputSum;

        /* Merge: '<S242>/ Merge ' incorporates:
         *  Integrator: '<S244>/omega wheel'
         *  SignalConversion generated from: '<S244>/Omega'
         */
        localB->Omega = localX->omegaWheel;

        /* Gain: '<S244>/Output Inertia' incorporates:
         *  Gain: '<S244>/-4'
         *  Integrator: '<S244>/omega wheel'
         *  Product: '<S244>/Max Dynamic Friction Torque1'
         *  Sum: '<S244>/Output Sum'
         *  Trigonometry: '<S244>/Trigonometric Function'
         */
        localB->OutputInertia =
            ((std::tanh(localP->u_Gain * localX->omegaWheel) * rtu_Tfmaxk - rtu_Tout) - rtb_OutputSum) *
            (1.0 / rtp_Iyy);

        /* Merge: '<S242>/ Merge 1' incorporates:
         *  SignalConversion generated from: '<S244>/Omegadot'
         */
        localB->Omegadot = localB->OutputInertia;
        localDW->Slipping_entered = true;
        rtsiSetBlockStateForSolverChangedAtMajorStep(&(&Chassis_M)->solverInfo, true);
      }

      /* case IN_Slipping: */
    } else if (Chassis_detectLockup(rtu_Tout, rtu_Tfmaxs, rtp_br, localB, localDW, localP) &&
               (std::abs(localB->Omega) <= rtp_OmegaTol)) {
      localDW->Slipping_entered = false;
      localDW->is_c8_autolibshared = Chassis_IN_Locked;
      if (rtmIsMajorTimeStep((&Chassis_M))) {
        /* Merge: '<S242>/ Merge ' incorporates:
         *  Constant: '<S243>/Constant'
         *  SignalConversion generated from: '<S243>/Omega'
         */
        localB->Omega = localP->Constant_Value_l;

        /* Merge: '<S242>/ Merge 1' incorporates:
         *  Constant: '<S243>/Constant1'
         *  SignalConversion generated from: '<S243>/Omegadot'
         */
        localB->Omegadot = localP->Constant1_Value;

        /* Merge: '<S242>/ Merge 3' incorporates:
         *  Constant: '<S243>/Constant2'
         *  SignalConversion generated from: '<S243>/Myb'
         */
        localB->Myb = localP->Constant2_Value;
      }

      localDW->Locked_entered = true;
      rtsiSetBlockStateForSolverChangedAtMajorStep(&(&Chassis_M)->solverInfo, true);
    }

    /* End of Chart: '<S241>/Clutch' */
  }

  if (localDW->is_c8_autolibshared == Chassis_IN_Locked) {
    if (!localDW->Locked_entered) {
      if (rtmIsMajorTimeStep((&Chassis_M))) {
        /* Merge: '<S242>/ Merge ' incorporates:
         *  Constant: '<S243>/Constant'
         *  SignalConversion generated from: '<S243>/Omega'
         */
        localB->Omega = localP->Constant_Value_l;

        /* Merge: '<S242>/ Merge 1' incorporates:
         *  Constant: '<S243>/Constant1'
         *  SignalConversion generated from: '<S243>/Omegadot'
         */
        localB->Omegadot = localP->Constant1_Value;

        /* Merge: '<S242>/ Merge 3' incorporates:
         *  Constant: '<S243>/Constant2'
         *  SignalConversion generated from: '<S243>/Myb'
         */
        localB->Myb = localP->Constant2_Value;
      }
    } else {
      localDW->Locked_entered = false;
    }

    /* case IN_Slipping: */
  } else if (!localDW->Slipping_entered) {
    /* Gain: '<S244>/Output Damping' incorporates:
     *  Integrator: '<S244>/omega wheel'
     */
    rtb_OutputSum = rtp_br * localX->omegaWheel;

    /* Merge: '<S242>/ Merge 3' incorporates:
     *  SignalConversion generated from: '<S244>/Myb'
     */
    localB->Myb = rtb_OutputSum;

    /* Merge: '<S242>/ Merge ' incorporates:
     *  Integrator: '<S244>/omega wheel'
     *  SignalConversion generated from: '<S244>/Omega'
     */
    localB->Omega = localX->omegaWheel;

    /* Gain: '<S244>/Output Inertia' incorporates:
     *  Gain: '<S244>/-4'
     *  Integrator: '<S244>/omega wheel'
     *  Product: '<S244>/Max Dynamic Friction Torque1'
     *  Sum: '<S244>/Output Sum'
     *  Trigonometry: '<S244>/Trigonometric Function'
     */
    localB->OutputInertia =
        ((std::tanh(localP->u_Gain * localX->omegaWheel) * rtu_Tfmaxk - rtu_Tout) - rtb_OutputSum) * (1.0 / rtp_Iyy);

    /* Merge: '<S242>/ Merge 1' incorporates:
     *  SignalConversion generated from: '<S244>/Omegadot'
     */
    localB->Omegadot = localB->OutputInertia;
  } else {
    localDW->Slipping_entered = false;
  }
}

/* Derivatives for atomic system: '<S241>/Clutch' */
void Chassis::Chassis_Clutch_Deriv(B_Clutch_Chassis_T *localB, DW_Clutch_Chassis_T *localDW,
                                   XDot_Clutch_Chassis_T *localXdot) {
  localXdot->omegaWheel = 0.0;
  if (localDW->is_c8_autolibshared == Chassis_IN_Slipping) {
    /* Derivatives for Integrator: '<S244>/omega wheel' */
    localXdot->omegaWheel = localB->OutputInertia;
  }

  if (localDW->is_c8_autolibshared == Chassis_IN_Locked) {
    if (localDW->Locked_entered) {
      localDW->Locked_entered = false;
    }

    /* case IN_Slipping: */
  } else if (localDW->Slipping_entered) {
    localDW->Slipping_entered = false;
  }
}

real_T rt_powd_snf(real_T u0, real_T u1) {
  real_T y;
  if (std::isnan(u0) || std::isnan(u1)) {
    y = (rtNaN);
  } else {
    real_T tmp;
    real_T tmp_0;
    tmp = std::abs(u0);
    tmp_0 = std::abs(u1);
    if (std::isinf(u1)) {
      if (tmp == 1.0) {
        y = 1.0;
      } else if (tmp > 1.0) {
        if (u1 > 0.0) {
          y = (rtInf);
        } else {
          y = 0.0;
        }
      } else if (u1 > 0.0) {
        y = 0.0;
      } else {
        y = (rtInf);
      }
    } else if (tmp_0 == 0.0) {
      y = 1.0;
    } else if (tmp_0 == 1.0) {
      if (u1 > 0.0) {
        y = u0;
      } else {
        y = 1.0 / u0;
      }
    } else if (u1 == 2.0) {
      y = u0 * u0;
    } else if ((u1 == 0.5) && (u0 >= 0.0)) {
      y = std::sqrt(u0);
    } else if ((u0 < 0.0) && (u1 > std::floor(u1))) {
      y = (rtNaN);
    } else {
      y = std::pow(u0, u1);
    }
  }

  return y;
}

real_T rt_atan2d_snf(real_T u0, real_T u1) {
  real_T y;
  if (std::isnan(u0) || std::isnan(u1)) {
    y = (rtNaN);
  } else if (std::isinf(u0) && std::isinf(u1)) {
    int32_T tmp;
    int32_T tmp_0;
    if (u0 > 0.0) {
      tmp = 1;
    } else {
      tmp = -1;
    }

    if (u1 > 0.0) {
      tmp_0 = 1;
    } else {
      tmp_0 = -1;
    }

    y = std::atan2(static_cast<real_T>(tmp), static_cast<real_T>(tmp_0));
  } else if (u1 == 0.0) {
    if (u0 > 0.0) {
      y = RT_PI / 2.0;
    } else if (u0 < 0.0) {
      y = -(RT_PI / 2.0);
    } else {
      y = 0.0;
    }
  } else {
    y = std::atan2(u0, u1);
  }

  return y;
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_div0protect(const real_T u[4], real_T tol, real_T y[4], real_T yabs[4]) {
  real_T c_data[4];
  real_T x;
  int32_T b_size_idx_0;
  int32_T k;
  int8_T b_data[4];
  yabs[0] = std::abs(u[0]);
  yabs[1] = std::abs(u[1]);
  yabs[2] = std::abs(u[2]);
  yabs[3] = std::abs(u[3]);
  k = 0;
  if (yabs[0] < tol) {
    k = 1;
  }

  if (yabs[1] < tol) {
    k++;
  }

  if (yabs[2] < tol) {
    k++;
  }

  if (yabs[3] < tol) {
    k++;
  }

  b_size_idx_0 = k;
  k = 0;
  if (yabs[0] < tol) {
    b_data[0] = 1;
    k = 1;
  }

  if (yabs[1] < tol) {
    b_data[k] = 2;
    k++;
  }

  if (yabs[2] < tol) {
    b_data[k] = 3;
    k++;
  }

  if (yabs[3] < tol) {
    b_data[k] = 4;
  }

  x = 2.0 * tol;
  for (k = 0; k < b_size_idx_0; k++) {
    real_T varargin_1;
    varargin_1 = yabs[b_data[k] - 1] / tol;
    c_data[k] = x / (3.0 - varargin_1 * varargin_1);
  }

  k = 0;
  x = yabs[0];
  if (yabs[0] < tol) {
    x = c_data[0];
    k = 1;
  }

  yabs[0] = x;
  x = yabs[1];
  if (yabs[1] < tol) {
    x = c_data[k];
    k++;
  }

  yabs[1] = x;
  x = yabs[2];
  if (yabs[2] < tol) {
    x = c_data[k];
    k++;
  }

  yabs[2] = x;
  x = yabs[3];
  if (yabs[3] < tol) {
    x = c_data[k];
  }

  yabs[3] = x;
  y[0] = yabs[0];
  if (u[0] < 0.0) {
    y[0] = -yabs[0];
  }

  y[1] = yabs[1];
  if (u[1] < 0.0) {
    y[1] = -yabs[1];
  }

  y[2] = yabs[2];
  if (u[2] < 0.0) {
    y[2] = -yabs[2];
  }

  y[3] = x;
  if (u[3] < 0.0) {
    y[3] = -x;
  }
}

void Chassis::Chassis_binary_expand_op(real_T in1_data[], int32_T in1_size[2], real_T in2, int32_T in3, real_T in4,
                                       int32_T in5, real_T in6, int32_T in7) {
  real_T in2_data;
  int32_T in2_size_idx_1;
  int32_T loop_ub;

  /* MATLAB Function: '<S229>/Magic Tire Const Input' */
  in2_size_idx_1 = in7 == 1 ? in5 == 1 ? in3 : in5 : in7;
  loop_ub = in7 == 1 ? in5 == 1 ? in3 : in5 : in7;
  for (int32_T i{0}; i < loop_ub; i++) {
    in2_data = in2 * in4 / in6;
  }

  in1_size[0] = 1;
  in1_size[1] = in2_size_idx_1;
  for (int32_T i{0}; i < in2_size_idx_1; i++) {
    in1_data[0] = in2_data * in2_data;
  }

  /* End of MATLAB Function: '<S229>/Magic Tire Const Input' */
}

void Chassis::Chassis_binary_expand_op_b3i5z2tw(real_T in1_data[], int32_T in1_size[2], real_T in2, int32_T in3,
                                                real_T in4, int32_T in5, real_T in6, int32_T in7, real_T in8,
                                                int32_T in9) {
  real_T in2_data;
  int32_T in2_size_idx_1;
  int32_T loop_ub;

  /* MATLAB Function: '<S229>/Magic Tire Const Input' */
  in2_size_idx_1 = in7 == 1 ? in5 == 1 ? in3 : in5 : in7;
  loop_ub = in7 == 1 ? in5 == 1 ? in3 : in5 : in7;
  for (int32_T i{0}; i < loop_ub; i++) {
    in2_data = in2 * in4 / in6;
  }

  in1_size[0] = 1;
  in1_size[1] = in9 == 1 ? in2_size_idx_1 : in9;
  loop_ub = in9 == 1 ? in2_size_idx_1 : in9;
  for (int32_T i{0}; i < loop_ub; i++) {
    in1_data[i] = in2_data * in2_data - 4.0 * in8;
  }

  /* End of MATLAB Function: '<S229>/Magic Tire Const Input' */
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_sqrt(real_T x_data[], const int32_T x_size[2]) {
  int32_T b;
  b = x_size[1];
  for (int32_T k{0}; k < b; k++) {
    x_data[0] = std::sqrt(x_data[0]);
  }
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_abs(const real_T x_data[], const int32_T *x_size, real_T y_data[], int32_T *y_size) {
  int32_T b;
  *y_size = *x_size;
  b = *x_size;
  for (int32_T k{0}; k < b; k++) {
    y_data[k] = std::abs(x_data[k]);
  }
}

void Chassis::Chassis_binary_expand_op_b3i5z2t(real_T in1_data[], int32_T *in1_size, const real_T in2[4],
                                               const int8_T in3_data[], const int32_T *in3_size, const real_T in4[4],
                                               const real_T in5_data[], const int32_T *in5_size, const real_T in6[108],
                                               const int8_T in7_data[], const int32_T *in7_size) {
  real_T in4_data[4];
  real_T tmp_data[4];
  int32_T loop_ub;
  int32_T stride_0_0;
  int32_T stride_1_0;
  int32_T stride_2_0;
  int32_T stride_3_0;
  int32_T tmp_size;

  /* MATLAB Function: '<S229>/Magic Tire Const Input' incorporates:
   *  Concatenate: '<S216>/Vector Concatenate'
   */
  loop_ub = *in3_size;
  for (tmp_size = 0; tmp_size < loop_ub; tmp_size++) {
    in4_data[tmp_size] = in4[in3_data[tmp_size] - 1];
  }

  Chassis_abs(in4_data, in3_size, tmp_data, &tmp_size);
  *in1_size = *in7_size == 1 ? *in5_size == 1 ? tmp_size == 1 ? *in3_size : tmp_size : *in5_size : *in7_size;
  stride_0_0 = (*in3_size != 1);
  stride_1_0 = (tmp_size != 1);
  stride_2_0 = (*in5_size != 1);
  stride_3_0 = (*in7_size != 1);
  loop_ub = *in7_size == 1 ? *in5_size == 1 ? tmp_size == 1 ? *in3_size : tmp_size : *in5_size : *in7_size;
  for (tmp_size = 0; tmp_size < loop_ub; tmp_size++) {
    in1_data[tmp_size] =
        (in2[in3_data[tmp_size * stride_0_0] - 1] / tmp_data[tmp_size * stride_1_0] + in5_data[tmp_size * stride_2_0]) *
        in6[(in7_data[tmp_size * stride_3_0] - 1) * 27 + 22];
  }

  /* End of MATLAB Function: '<S229>/Magic Tire Const Input' */
}

void Chassis::Chassis_binary_expand_op_b3i5z2(real_T in1_data[], int32_T *in1_size, const real_T in2[4],
                                              const int8_T in3_data[], const int32_T *in3_size, const real_T in4[4],
                                              const int8_T in5_data[], const int32_T *in5_size, real_T in6) {
  real_T in4_data[4];
  real_T tmp_data[4];
  int32_T loop_ub;
  int32_T stride_0_0;
  int32_T stride_1_0;
  int32_T tmp_size;

  /* MATLAB Function: '<S229>/Magic Tire Const Input' */
  loop_ub = *in5_size;
  for (tmp_size = 0; tmp_size < loop_ub; tmp_size++) {
    in4_data[tmp_size] = in4[in5_data[tmp_size] - 1];
  }

  Chassis_abs(in4_data, in5_size, tmp_data, &tmp_size);
  *in1_size = tmp_size == 1 ? *in3_size : tmp_size;
  stride_0_0 = (*in3_size != 1);
  stride_1_0 = (tmp_size != 1);
  loop_ub = tmp_size == 1 ? *in3_size : tmp_size;
  for (tmp_size = 0; tmp_size < loop_ub; tmp_size++) {
    in1_data[tmp_size] = in2[in3_data[tmp_size * stride_0_0] - 1] * (tmp_data[tmp_size * stride_1_0] / in6);
  }

  /* End of MATLAB Function: '<S229>/Magic Tire Const Input' */
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_div0protect_p(const real_T u[4], real_T y[4]) {
  real_T c_data[4];
  real_T yabs[4];
  real_T varargin_1;
  int32_T b_size_idx_0;
  int32_T k;
  int8_T b_data[4];
  yabs[0] = std::abs(u[0]);
  yabs[1] = std::abs(u[1]);
  yabs[2] = std::abs(u[2]);
  yabs[3] = std::abs(u[3]);
  k = 0;
  if (yabs[0] < 0.0001) {
    k = 1;
  }

  if (yabs[1] < 0.0001) {
    k++;
  }

  if (yabs[2] < 0.0001) {
    k++;
  }

  if (yabs[3] < 0.0001) {
    k++;
  }

  b_size_idx_0 = k;
  k = 0;
  if (yabs[0] < 0.0001) {
    b_data[0] = 1;
    k = 1;
  }

  if (yabs[1] < 0.0001) {
    b_data[k] = 2;
    k++;
  }

  if (yabs[2] < 0.0001) {
    b_data[k] = 3;
    k++;
  }

  if (yabs[3] < 0.0001) {
    b_data[k] = 4;
  }

  for (k = 0; k < b_size_idx_0; k++) {
    varargin_1 = yabs[b_data[k] - 1] / 0.0001;
    c_data[k] = 0.0002 / (3.0 - varargin_1 * varargin_1);
  }

  k = 0;
  varargin_1 = yabs[0];
  if (yabs[0] < 0.0001) {
    varargin_1 = c_data[0];
    k = 1;
  }

  yabs[0] = varargin_1;
  varargin_1 = yabs[1];
  if (yabs[1] < 0.0001) {
    varargin_1 = c_data[k];
    k++;
  }

  yabs[1] = varargin_1;
  varargin_1 = yabs[2];
  if (yabs[2] < 0.0001) {
    varargin_1 = c_data[k];
    k++;
  }

  yabs[2] = varargin_1;
  varargin_1 = yabs[3];
  if (yabs[3] < 0.0001) {
    varargin_1 = c_data[k];
  }

  y[0] = yabs[0];
  if (u[0] < 0.0) {
    y[0] = -yabs[0];
  }

  y[1] = yabs[1];
  if (u[1] < 0.0) {
    y[1] = -yabs[1];
  }

  y[2] = yabs[2];
  if (u[2] < 0.0) {
    y[2] = -yabs[2];
  }

  y[3] = varargin_1;
  if (u[3] < 0.0) {
    y[3] = -varargin_1;
  }
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
boolean_T Chassis::Chassis_any(const boolean_T x[4]) {
  int32_T k;
  boolean_T exitg1;
  boolean_T y;
  y = false;
  k = 0;
  exitg1 = false;
  while ((!exitg1) && (k < 4)) {
    if (x[k]) {
      y = true;
      exitg1 = true;
    } else {
      k++;
    }
  }

  return y;
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_atan(real_T x_data[], const int32_T *x_size) {
  int32_T b;
  b = *x_size;
  for (int32_T k{0}; k < b; k++) {
    x_data[k] = std::atan(x_data[k]);
  }
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_cos(real_T x_data[], const int32_T *x_size) {
  int32_T b;
  b = *x_size;
  for (int32_T k{0}; k < b; k++) {
    x_data[k] = std::cos(x_data[k]);
  }
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_sqrt_c(real_T x_data[], const int32_T *x_size) {
  int32_T b;
  int32_T scalarLB;
  int32_T vectorUB;
  b = *x_size;
  scalarLB = (*x_size / 2) << 1;
  vectorUB = scalarLB - 2;
  for (int32_T k{0}; k <= vectorUB; k += 2) {
    __m128d tmp;
    tmp = _mm_loadu_pd(&x_data[k]);
    _mm_storeu_pd(&x_data[k], _mm_sqrt_pd(tmp));
  }

  for (int32_T k{scalarLB}; k < b; k++) {
    x_data[k] = std::sqrt(x_data[k]);
  }
}

void Chassis::Chassis_binary_expand_op_b3i5z(real_T in1_data[], int32_T *in1_size, real_T in2, real_T in3,
                                             const real_T in4[4], const real_T in5[4], const int8_T in6_data[],
                                             const int32_T *in6_size, real_T in7, const real_T in8[4],
                                             const int8_T in9_data[], const int32_T *in9_size, real_T in10,
                                             const real_T in11_data[], const int32_T *in11_size) {
  real_T in8_data[4];
  real_T tmp_data[4];
  int32_T loop_ub;
  int32_T stride_0_0;
  int32_T stride_1_0;
  int32_T stride_2_0;
  int32_T tmp_size;

  /* MATLAB Function: '<S229>/Magic Tire Const Input' */
  loop_ub = *in9_size;
  for (tmp_size = 0; tmp_size < loop_ub; tmp_size++) {
    in8_data[tmp_size] = in8[in9_data[tmp_size] - 1];
  }

  Chassis_abs(in8_data, in9_size, tmp_data, &tmp_size);
  *in1_size = (*in11_size == 1 ? tmp_size : *in11_size) == 1 ? *in6_size : *in11_size == 1 ? tmp_size : *in11_size;
  stride_0_0 = (*in6_size != 1);
  stride_1_0 = (tmp_size != 1);
  stride_2_0 = (*in11_size != 1);
  loop_ub = (*in11_size == 1 ? tmp_size : *in11_size) == 1 ? *in6_size : *in11_size == 1 ? tmp_size : *in11_size;
  for (tmp_size = 0; tmp_size < loop_ub; tmp_size++) {
    int32_T tmp;
    tmp = in6_data[tmp_size * stride_0_0] - 1;
    in1_data[tmp_size] = (in4[tmp] * in3 + 1.0) * in2 * in5[tmp] *
                         (tmp_data[tmp_size * stride_1_0] * in7 + in11_data[tmp_size * stride_2_0] * in10);
  }

  /* End of MATLAB Function: '<S229>/Magic Tire Const Input' */
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_magicsin(const real_T D[4], real_T C, const real_T B[4], real_T E, const real_T u[4],
                               real_T y[4]) {
  real_T y_0;
  y_0 = B[0] * u[0];
  y[0] = std::sin(std::atan(y_0 - (y_0 - std::atan(y_0)) * E) * C) * D[0];
  y_0 = B[1] * u[1];
  y[1] = std::sin(std::atan(y_0 - (y_0 - std::atan(y_0)) * E) * C) * D[1];
  y_0 = B[2] * u[2];
  y[2] = std::sin(std::atan(y_0 - (y_0 - std::atan(y_0)) * E) * C) * D[2];
  y_0 = B[3] * u[3];
  y[3] = std::sin(std::atan(y_0 - (y_0 - std::atan(y_0)) * E) * C) * D[3];
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_magiccos(real_T C, const real_T B[4], const real_T E[4], const real_T u[4], real_T y[4]) {
  real_T y_0;
  y_0 = B[0] * u[0];
  y[0] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[0]) * C);
  y_0 = B[1] * u[1];
  y[1] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[1]) * C);
  y_0 = B[2] * u[2];
  y[2] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[2]) * C);
  y_0 = B[3] * u[3];
  y[3] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[3]) * C);
}

void Chassis::Chassis_binary_expand_op_b3i5(real_T in1[36], const int8_T in2_data[], const int32_T *in2_size,
                                            const real_T in3[4], const int8_T in4_data[], const int32_T *in4_size,
                                            const real_T in5[4], const int8_T in6_data[], const int32_T *in6_size,
                                            const real_T in7[4]) {
  int32_T stride_0_1;
  int32_T stride_1_1;
  int32_T tmp;

  /* MATLAB Function: '<S229>/Magic Tire Const Input' */
  tmp = *in2_size;
  stride_0_1 = (*in4_size != 1);
  stride_1_1 = (*in6_size != 1);
  for (int32_T i{0}; i < tmp; i++) {
    int32_T tmp_0;
    tmp_0 = in6_data[i * stride_1_1] - 1;
    in1[9 * (in2_data[i] - 1) + 4] = (in3[in4_data[i * stride_0_1] - 1] + 1.0) - in5[tmp_0] / in7[tmp_0];
  }

  /* End of MATLAB Function: '<S229>/Magic Tire Const Input' */
}

void Chassis::Chassis_binary_expand_op_b3i(real_T in1_data[], int32_T *in1_size, const real_T in2[4],
                                           const real_T in3[4], const real_T in4[4], const int8_T in5_data[],
                                           const int32_T *in5_size, const real_T in6[4], const int8_T in7_data[],
                                           const int32_T *in7_size, const real_T in8[4]) {
  real_T in6_data[4];
  real_T tmp_data[4];
  int32_T loop_ub;
  int32_T stride_0_0;
  int32_T stride_1_0;
  int32_T stride_2_0;
  int32_T tmp_size;

  /* MATLAB Function: '<S229>/Magic Tire Const Input' */
  loop_ub = *in7_size;
  for (tmp_size = 0; tmp_size < loop_ub; tmp_size++) {
    in6_data[tmp_size] = in6[in7_data[tmp_size] - 1];
  }

  Chassis_abs(in6_data, in7_size, tmp_data, &tmp_size);
  *in1_size = (*in5_size == 1 ? tmp_size : *in5_size) == 1 ? *in5_size : *in5_size == 1 ? tmp_size : *in5_size;
  stride_0_0 = (*in5_size != 1);
  stride_1_0 = (tmp_size != 1);
  stride_2_0 = (*in5_size != 1);
  loop_ub = (*in5_size == 1 ? tmp_size : *in5_size) == 1 ? *in5_size : *in5_size == 1 ? tmp_size : *in5_size;
  for (tmp_size = 0; tmp_size < loop_ub; tmp_size++) {
    int32_T tmp;
    tmp = in5_data[tmp_size * stride_0_0] - 1;
    in1_data[tmp_size] = in2[tmp] * 2.0 / 3.1415926535897931 * in3[tmp] * in4[tmp] /
                         (in8[in5_data[tmp_size * stride_2_0] - 1] * 0.0001 - tmp_data[tmp_size * stride_1_0]);
  }

  /* End of MATLAB Function: '<S229>/Magic Tire Const Input' */
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_acos(real_T x_data[], const int32_T *x_size) {
  int32_T b;
  b = *x_size;
  for (int32_T k{0}; k < b; k++) {
    x_data[k] = std::acos(x_data[k]);
  }
}

void Chassis::Chassis_binary_expand_op_b3(real_T in1_data[], int32_T *in1_size, const real_T in2_data[],
                                          const int32_T *in2_size, const real_T in3[4], const int8_T in4_data[],
                                          const int32_T *in4_size, const real_T in5[4], const int8_T in6_data[],
                                          const int32_T *in6_size, const real_T in7[4]) {
  int32_T loop_ub;
  int32_T stride_0_0;
  int32_T stride_1_0;
  int32_T stride_2_0;

  /* MATLAB Function: '<S229>/Magic Tire Const Input' */
  *in1_size = *in6_size == 1 ? *in4_size == 1 ? *in2_size : *in4_size : *in6_size;
  stride_0_0 = (*in2_size != 1);
  stride_1_0 = (*in4_size != 1);
  stride_2_0 = (*in6_size != 1);
  loop_ub = *in6_size == 1 ? *in4_size == 1 ? *in2_size : *in4_size : *in6_size;
  for (int32_T i{0}; i < loop_ub; i++) {
    int32_T tmp;
    tmp = in6_data[i * stride_2_0] - 1;
    in1_data[i] = (in3[in4_data[i * stride_1_0] - 1] + in2_data[i * stride_0_0]) - in5[tmp] / in7[tmp];
  }

  /* End of MATLAB Function: '<S229>/Magic Tire Const Input' */
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_magiccos_n(real_T C, const real_T B[4], const real_T E[4], real_T y[4]) {
  y[0] = std::cos(std::atan(B[0] * 0.0 - (B[0] * 0.0 - std::atan(B[0] * 0.0)) * E[0]) * C);
  y[1] = std::cos(std::atan(B[1] * 0.0 - (B[1] * 0.0 - std::atan(B[1] * 0.0)) * E[1]) * C);
  y[2] = std::cos(std::atan(B[2] * 0.0 - (B[2] * 0.0 - std::atan(B[2] * 0.0)) * E[2]) * C);
  y[3] = std::cos(std::atan(B[3] * 0.0 - (B[3] * 0.0 - std::atan(B[3] * 0.0)) * E[3]) * C);
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_magicsin_g(const real_T D[4], const real_T C[4], const real_T B[4], const real_T E[4],
                                 const real_T u[4], real_T y[4]) {
  real_T y_0;
  y_0 = B[0] * u[0];
  y[0] = std::sin(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[0]) * C[0]) * D[0];
  y_0 = B[1] * u[1];
  y[1] = std::sin(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[1]) * C[1]) * D[1];
  y_0 = B[2] * u[2];
  y[2] = std::sin(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[2]) * C[2]) * D[2];
  y_0 = B[3] * u[3];
  y[3] = std::sin(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[3]) * C[3]) * D[3];
}

void Chassis::Chassis_binary_expand_op_b(real_T in1_data[], int32_T *in1_size, const real_T in2_data[],
                                         const int32_T *in2_size, const real_T in3[4], real_T in4, real_T in5,
                                         const real_T in6[4], real_T in7, const real_T in8[4], const real_T in9[108],
                                         const int8_T in10_data[], const int32_T *in10_size, const real_T in11[4],
                                         const real_T in12[4], const int8_T in13_data[], const int32_T *in13_size,
                                         const real_T in14[4], const real_T in15[36], const int8_T in16_data[],
                                         const int32_T *in16_size, const int8_T in17_data[], const int32_T *in17_size) {
  int32_T loop_ub;
  int32_T stride_0_0;
  int32_T stride_1_0;
  int32_T stride_2_0;
  int32_T stride_3_0;
  int32_T stride_4_0;
  int32_T stride_5_0;

  /* MATLAB Function: '<S229>/Magic Tire Const Input' incorporates:
   *  Concatenate: '<S216>/Vector Concatenate'
   */
  *in1_size = *in17_size == 1
                  ? (*in16_size == 1 ? *in13_size == 1 ? *in13_size == 1 ? *in10_size : *in13_size : *in13_size
                                     : *in16_size) == 1
                        ? *in2_size
                    : *in16_size == 1 ? *in13_size == 1 ? *in13_size == 1 ? *in10_size : *in13_size : *in13_size
                                      : *in16_size
                  : *in17_size;
  stride_0_0 = (*in2_size != 1);
  stride_1_0 = (*in10_size != 1);
  stride_2_0 = (*in13_size != 1);
  stride_3_0 = (*in13_size != 1);
  stride_4_0 = (*in16_size != 1);
  stride_5_0 = (*in17_size != 1);
  loop_ub = *in17_size == 1
                ? (*in16_size == 1 ? *in13_size == 1 ? *in13_size == 1 ? *in10_size : *in13_size : *in13_size
                                   : *in16_size) == 1
                      ? *in2_size
                  : *in16_size == 1 ? *in13_size == 1 ? *in13_size == 1 ? *in10_size : *in13_size : *in13_size
                                    : *in16_size
                : *in17_size;
  for (int32_T i{0}; i < loop_ub; i++) {
    int32_T tmp;
    tmp = in10_data[i * stride_1_0] - 1;
    in1_data[i] = (((in6[tmp] * in5 + in4) * in3[tmp] * (in8[tmp] * in7 + 1.0) * in9[tmp * 27 + 14] * in11[tmp] -
                    in12[in13_data[i * stride_2_0] - 1]) /
                       in14[in13_data[i * stride_3_0] - 1] * in15[(in16_data[i * stride_4_0] - 1) * 9] +
                   in2_data[i * stride_0_0]) +
                  in15[(in17_data[i * stride_5_0] - 1) * 9 + 4];
  }

  /* End of MATLAB Function: '<S229>/Magic Tire Const Input' */
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_rollingMoment(const real_T Fx[4], const real_T Vcx[4], const real_T Fz[4], const real_T press[4],
                                    const real_T b_gamma[4], real_T Vo, real_T Ro, real_T Fzo, real_T pio,
                                    real_T b_QSY1, real_T b_QSY2, real_T b_QSY3, real_T b_QSY4, real_T b_QSY5,
                                    real_T b_QSY6, real_T b_QSY7, real_T b_QSY8, const real_T lam_My[4], real_T My[4]) {
  real_T My_0;
  real_T Ro_0;
  real_T x_tmp;
  real_T y;
  Ro_0 = -Ro * Fzo;
  x_tmp = Vcx[0] / Vo;
  My_0 = std::abs(x_tmp);
  y = rt_powd_snf(x_tmp, 4.0);
  x_tmp = Fz[0] / Fzo;
  My[0] = ((((Fx[0] / Fzo * b_QSY2 + b_QSY1) + b_QSY3 * My_0) + b_QSY4 * y) +
           (b_QSY6 * x_tmp + b_QSY5) * b_gamma[0] * b_gamma[0]) *
          (Ro_0 * lam_My[0]) * (rt_powd_snf(press[0] / pio, b_QSY8) * rt_powd_snf(x_tmp, b_QSY7));
  x_tmp = Vcx[1] / Vo;
  My_0 = std::abs(x_tmp);
  y = rt_powd_snf(x_tmp, 4.0);
  x_tmp = Fz[1] / Fzo;
  My[1] = ((((Fx[1] / Fzo * b_QSY2 + b_QSY1) + b_QSY3 * My_0) + b_QSY4 * y) +
           (b_QSY6 * x_tmp + b_QSY5) * b_gamma[1] * b_gamma[1]) *
          (Ro_0 * lam_My[1]) * (rt_powd_snf(press[1] / pio, b_QSY8) * rt_powd_snf(x_tmp, b_QSY7));
  x_tmp = Vcx[2] / Vo;
  My_0 = std::abs(x_tmp);
  y = rt_powd_snf(x_tmp, 4.0);
  x_tmp = Fz[2] / Fzo;
  My[2] = ((((Fx[2] / Fzo * b_QSY2 + b_QSY1) + b_QSY3 * My_0) + b_QSY4 * y) +
           (b_QSY6 * x_tmp + b_QSY5) * b_gamma[2] * b_gamma[2]) *
          (Ro_0 * lam_My[2]) * (rt_powd_snf(press[2] / pio, b_QSY8) * rt_powd_snf(x_tmp, b_QSY7));
  x_tmp = Vcx[3] / Vo;
  My_0 = std::abs(x_tmp);
  y = rt_powd_snf(x_tmp, 4.0);
  x_tmp = Fz[3] / Fzo;
  My[3] = ((((Fx[3] / Fzo * b_QSY2 + b_QSY1) + b_QSY3 * My_0) + b_QSY4 * y) +
           (b_QSY6 * x_tmp + b_QSY5) * b_gamma[3] * b_gamma[3]) *
          (Ro_0 * lam_My[3]) * (rt_powd_snf(press[3] / pio, b_QSY8) * rt_powd_snf(x_tmp, b_QSY7));
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_magiccos_ngy(const real_T D[4], real_T C, const real_T B[4], const real_T E[4], const real_T u[4],
                                   real_T y[4]) {
  real_T y_0;
  y_0 = B[0] * u[0];
  y[0] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[0]) * C) * D[0];
  y_0 = B[1] * u[1];
  y[1] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[1]) * C) * D[1];
  y_0 = B[2] * u[2];
  y[2] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[2]) * C) * D[2];
  y_0 = B[3] * u[3];
  y[3] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * E[3]) * C) * D[3];
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_magiccos_ng(const real_T D[4], const real_T C[4], const real_T B[4], const real_T u[4],
                                  real_T y[4]) {
  real_T y_0;
  y_0 = B[0] * u[0];
  y[0] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * 0.0) * C[0]) * D[0];
  y_0 = B[1] * u[1];
  y[1] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * 0.0) * C[1]) * D[1];
  y_0 = B[2] * u[2];
  y[2] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * 0.0) * C[2]) * D[2];
  y_0 = B[3] * u[3];
  y[3] = std::cos(std::atan(y_0 - (y_0 - std::atan(y_0)) * 0.0) * C[3]) * D[3];
}

/* Function for MATLAB Function: '<S229>/Magic Tire Const Input' */
void Chassis::Chassis_vdyncsmtire(
    const real_T Omega[4], const real_T Vx[4], const real_T Vy[4], const real_T psidot[4], real_T b_gamma[4],
    real_T press[4], const real_T scaleFactors[108], const real_T rhoz[4], real_T b_turnslip, real_T b_PRESMAX,
    real_T b_PRESMIN, real_T b_FZMAX, real_T b_FZMIN, real_T b_VXLOW, real_T b_KPUMAX, real_T b_KPUMIN, real_T b_ALPMAX,
    real_T b_ALPMIN, real_T b_CAMMIN, real_T b_CAMMAX, real_T b_LONGVL, real_T b_UNLOADED_RADIUS, real_T b_RIM_RADIUS,
    real_T b_NOMPRES, real_T b_FNOMIN, real_T b_VERTICAL_STIFFNESS, real_T b_DREFF, real_T b_BREFF, real_T b_FREFF,
    real_T b_Q_RE0, real_T b_Q_V1, real_T b_Q_V2, real_T b_Q_FZ1, real_T b_Q_FZ2, real_T b_Q_FCX, real_T b_Q_FCY,
    real_T b_PFZ1, real_T b_Q_FCY2, real_T b_BOTTOM_OFFST, real_T b_BOTTOM_STIFF, real_T b_PCX1, real_T b_PDX1,
    real_T b_PDX2, real_T b_PDX3, real_T b_PEX1, real_T b_PEX2, real_T b_PEX3, real_T b_PEX4, real_T b_PKX1,
    real_T b_PKX2, real_T b_PKX3, real_T b_PHX1, real_T b_PHX2, real_T b_PVX1, real_T b_PVX2, real_T b_PPX1,
    real_T b_PPX2, real_T b_PPX3, real_T b_PPX4, real_T b_RBX1, real_T b_RBX2, real_T b_RBX3, real_T b_RCX1,
    real_T b_REX1, real_T b_REX2, real_T b_QSX2, real_T b_QSX3, real_T b_QSX4, real_T b_QSX5, real_T b_QSX6,
    real_T b_QSX7, real_T b_QSX8, real_T b_QSX9, real_T b_QSX10, real_T b_QSX11, real_T b_PPMX1, real_T b_PCY1,
    real_T b_PDY1, real_T b_PDY2, real_T b_PDY3, real_T b_PEY1, real_T b_PEY2, real_T b_PEY4, real_T b_PEY5,
    real_T b_PKY1, real_T b_PKY2, real_T b_PKY3, real_T b_PKY4, real_T b_PKY5, real_T b_PKY6, real_T b_PKY7,
    real_T b_PVY3, real_T b_PVY4, real_T b_PPY1, real_T b_PPY2, real_T b_PPY3, real_T b_PPY4, real_T b_PPY5,
    real_T b_RBY1, real_T b_RBY2, real_T b_RBY4, real_T b_RCY1, real_T b_REY1, real_T b_REY2, real_T b_RHY1,
    real_T b_RHY2, real_T b_RVY3, real_T b_RVY4, real_T b_RVY5, real_T b_RVY6, real_T b_QSY1, real_T b_QSY2,
    real_T b_QSY3, real_T b_QSY4, real_T b_QSY5, real_T b_QSY6, real_T b_QSY7, real_T b_QSY8, real_T b_QBZ1,
    real_T b_QBZ2, real_T b_QBZ3, real_T b_QBZ5, real_T b_QBZ6, real_T b_QBZ9, real_T b_QBZ10, real_T b_QCZ1,
    real_T b_QDZ1, real_T b_QDZ2, real_T b_QDZ4, real_T b_QDZ8, real_T b_QDZ9, real_T b_QDZ10, real_T b_QDZ11,
    real_T b_QEZ1, real_T b_QEZ2, real_T b_QEZ3, real_T b_QEZ5, real_T b_QHZ3, real_T b_QHZ4, real_T b_PPZ1,
    real_T b_PPZ2, real_T b_SSZ2, real_T b_SSZ3, real_T b_SSZ4, real_T b_PDXP1, real_T b_PDXP2, real_T b_PDXP3,
    real_T b_PKYP1, real_T b_PDYP1, real_T b_PDYP2, real_T b_PDYP3, real_T b_PDYP4, real_T b_PHYP1, real_T b_PHYP2,
    real_T b_PHYP3, real_T b_PHYP4, real_T b_PECP1, real_T b_PECP2, real_T b_QDTP1, real_T b_QCRP1, real_T b_QCRP2,
    real_T b_QBRP1, real_T b_QDRP1, real_T b_QDRP2, real_T b_WIDTH, real_T b_Q_RA1, real_T b_Q_RA2, real_T b_Q_RB1,
    real_T b_Q_RB2, real_T b_QSX12, real_T b_QSX13, real_T b_QSX14, real_T b_Q_FZ3, real_T b_LONGITUDINAL_STIFFNESS,
    real_T b_LATERAL_STIFFNESS, real_T b_PCFX1, real_T b_PCFX2, real_T b_PCFX3, real_T b_PCFY1, real_T b_PCFY2,
    real_T b_PCFY3, const real_T Fx_ext[4], const real_T Fy_ext[4], real_T Fx[4], real_T Fy[4], real_T Fz[4],
    real_T Mx[4], real_T My[4], real_T Mz[4], real_T Re[4], real_T kappa[4], real_T alpha[4], real_T sig_x[4],
    real_T sig_y[4], real_T patch_a[4], real_T patch_b[4]) {
  real_T zeta[36];
  real_T Bxalpha[4];
  real_T Bykappa[4];
  real_T Drphi[4];
  real_T Eykappa[4];
  real_T FzUnSat[4];
  real_T Kyalpha[4];
  real_T Mzphiinf[4];
  real_T SVygamma[4];
  real_T Vs[4];
  real_T Vsy[4];
  real_T b_x[4];
  real_T dfz[4];
  real_T dpi[4];
  real_T e_data[4];
  real_T epsilon_gamma[4];
  real_T gamma_star[4];
  real_T j_data[4];
  real_T k_data[4];
  real_T kappa_x[4];
  real_T lam_Cz[4];
  real_T phi[4];
  real_T phi_t[4];
  real_T rhoz_data[4];
  real_T tmpDrphiVar_data[4];
  real_T Bxalpha_0;
  real_T Bxalpha_tmp;
  real_T Dx_tmp;
  real_T Dx_tmp_0;
  real_T Fx_0;
  real_T Fx_tmp;
  real_T Fx_tmp_0;
  real_T Fz_0;
  real_T Fz_tmp;
  real_T Fz_tmp_0;
  real_T Fz_tmp_1;
  real_T Fz_tmp_2;
  real_T Fzo_prime_idx_0;
  real_T Fzo_prime_idx_1;
  real_T Fzo_prime_idx_2;
  real_T Re_0;
  real_T SHy_idx_0;
  real_T SHy_idx_1;
  real_T SHy_idx_2;
  real_T SHy_idx_3;
  real_T Vs_0;
  real_T Vs_1;
  real_T Vs_2;
  real_T Vs_tmp;
  real_T Vsy_0;
  real_T b_gamma_0;
  real_T b_idx_0;
  real_T c_y_idx_1;
  real_T dfz_0;
  real_T e_y;
  real_T lam_Cz_0;
  real_T lam_Cz_tmp;
  real_T lam_Cz_tmp_0;
  real_T lam_muy;
  real_T lam_muy_idx_0;
  real_T lam_muy_idx_1;
  real_T lam_muy_idx_2;
  real_T lam_muy_prime;
  real_T lam_muy_prime_idx_0;
  real_T lam_muy_prime_idx_1;
  real_T lam_muy_prime_idx_2;
  real_T lam_muy_prime_idx_3;
  real_T mu_y;
  real_T mu_y_idx_0;
  real_T mu_y_idx_1;
  real_T mu_y_idx_2;
  real_T mu_y_idx_2_tmp;
  real_T mu_y_idx_2_tmp_0;
  real_T mu_y_idx_2_tmp_1;
  real_T mu_y_idx_2_tmp_2;
  real_T mu_y_tmp;
  real_T press_0;
  real_T tb_data;
  real_T tmp_data;
  int32_T tb_size[2];
  int32_T tmp_size[2];
  int32_T tmp_size_0[2];
  int32_T b_i;
  int32_T h_i;
  int32_T o_i;
  int32_T o_size;
  int32_T s_i;
  int32_T tmpDrphiVar_size;
  int32_T tmp_size_1;
  int32_T tmp_size_2;
  int32_T w_size;
  int8_T ab_data[4];
  int8_T bb_data[4];
  int8_T cb_data[4];
  int8_T db_data[4];
  int8_T eb_data[4];
  int8_T fb_data[4];
  int8_T gb_data[4];
  int8_T hb_data[4];
  int8_T ib_data[4];
  int8_T jb_data[4];
  int8_T kb_data[4];
  int8_T lb_data[4];
  int8_T mb_data[4];
  int8_T n_data[4];
  int8_T nb_data[4];
  int8_T o_data[4];
  int8_T ob_data[4];
  int8_T p_data[4];
  int8_T pb_data[4];
  int8_T q_data[4];
  int8_T qb_data[4];
  int8_T r_data[4];
  int8_T rb_data[4];
  int8_T s_data[4];
  int8_T t_data[4];
  int8_T u_data[4];
  int8_T v_data[4];
  int8_T w_data[4];
  int8_T x_data[4];
  int8_T y_data[4];
  int8_T s;
  boolean_T isLowSpeed[4];
  boolean_T isLowSpeed_0[4];
  Chassis_div0protect(Vx, b_VXLOW, Drphi, Vsy);
  press_0 = press[0];
  b_gamma_0 = b_gamma[0];
  Fx[0] = scaleFactors[1];
  lam_muy_idx_0 = scaleFactors[2];
  if (b_gamma[0] < b_CAMMIN) {
    b_gamma_0 = b_CAMMIN;
  }

  if (b_gamma_0 > b_CAMMAX) {
    b_gamma_0 = b_CAMMAX;
  }

  if (press[0] < b_PRESMIN) {
    press_0 = b_PRESMIN;
  }

  if (press_0 > b_PRESMAX) {
    press_0 = b_PRESMAX;
  }

  dpi[0] = (press_0 - b_NOMPRES) / b_NOMPRES;
  if (scaleFactors[1] <= 0.0) {
    Fx[0] = 2.2204460492503131E-16;
  }

  if (scaleFactors[2] <= 0.0) {
    lam_muy_idx_0 = 2.2204460492503131E-16;
  }

  b_gamma[0] = b_gamma_0;
  press[0] = press_0;
  press_0 = press[1];
  b_gamma_0 = b_gamma[1];
  Fx[1] = scaleFactors[28];
  lam_muy_idx_1 = scaleFactors[29];
  if (b_gamma[1] < b_CAMMIN) {
    b_gamma_0 = b_CAMMIN;
  }

  if (b_gamma_0 > b_CAMMAX) {
    b_gamma_0 = b_CAMMAX;
  }

  if (press[1] < b_PRESMIN) {
    press_0 = b_PRESMIN;
  }

  if (press_0 > b_PRESMAX) {
    press_0 = b_PRESMAX;
  }

  dpi[1] = (press_0 - b_NOMPRES) / b_NOMPRES;
  if (scaleFactors[28] <= 0.0) {
    Fx[1] = 2.2204460492503131E-16;
  }

  if (scaleFactors[29] <= 0.0) {
    lam_muy_idx_1 = 2.2204460492503131E-16;
  }

  b_gamma[1] = b_gamma_0;
  press[1] = press_0;
  press_0 = press[2];
  b_gamma_0 = b_gamma[2];
  Fx[2] = scaleFactors[55];
  lam_muy_idx_2 = scaleFactors[56];
  if (b_gamma[2] < b_CAMMIN) {
    b_gamma_0 = b_CAMMIN;
  }

  if (b_gamma_0 > b_CAMMAX) {
    b_gamma_0 = b_CAMMAX;
  }

  if (press[2] < b_PRESMIN) {
    press_0 = b_PRESMIN;
  }

  if (press_0 > b_PRESMAX) {
    press_0 = b_PRESMAX;
  }

  dpi[2] = (press_0 - b_NOMPRES) / b_NOMPRES;
  if (scaleFactors[55] <= 0.0) {
    Fx[2] = 2.2204460492503131E-16;
  }

  if (scaleFactors[56] <= 0.0) {
    lam_muy_idx_2 = 2.2204460492503131E-16;
  }

  b_gamma[2] = b_gamma_0;
  press[2] = press_0;
  press_0 = press[3];
  b_gamma_0 = b_gamma[3];
  Fx_0 = scaleFactors[82];
  lam_muy = scaleFactors[83];
  if (b_gamma[3] < b_CAMMIN) {
    b_gamma_0 = b_CAMMIN;
  }

  if (b_gamma_0 > b_CAMMAX) {
    b_gamma_0 = b_CAMMAX;
  }

  if (press[3] < b_PRESMIN) {
    press_0 = b_PRESMIN;
  }

  if (press_0 > b_PRESMAX) {
    press_0 = b_PRESMAX;
  }

  dpi[3] = (press_0 - b_NOMPRES) / b_NOMPRES;
  if (scaleFactors[82] <= 0.0) {
    Fx_0 = 2.2204460492503131E-16;
  }

  if (scaleFactors[83] <= 0.0) {
    lam_muy = 2.2204460492503131E-16;
  }

  b_gamma[3] = b_gamma_0;
  press[3] = press_0;
  if (b_Q_FZ1 == 0.0) {
    boolean_T tempInds;
    press_0 = b_VERTICAL_STIFFNESS * b_UNLOADED_RADIUS / b_FNOMIN;
    tempInds = (press_0 * press_0 - 4.0 * b_Q_FZ2 < 0.0);
    b_idx_0 = b_Q_FZ1;
    b_i = 0;
    if (tempInds) {
      b_i = 1;
    }

    if (b_i - 1 >= 0) {
      b_idx_0 = b_Q_FZ2 * 0.0;
    }

    b_i = 0;
    h_i = 0;
    if (!tempInds) {
      b_i = 1;
    }

    o_i = 0;
    if (!tempInds) {
      h_i = 1;
    }

    s_i = 0;
    if (!tempInds) {
      o_i = 1;
      s_i = 1;
    }

    if ((b_i != h_i) || ((b_i == 1 ? h_i : b_i) != o_i)) {
      Chassis_binary_expand_op(&tmp_data, tmp_size, b_VERTICAL_STIFFNESS, b_i, b_UNLOADED_RADIUS, h_i, b_FNOMIN, o_i);
    }

    Chassis_binary_expand_op(&lam_muy_prime_idx_3, tmp_size_0, b_VERTICAL_STIFFNESS, b_i, b_UNLOADED_RADIUS, h_i,
                             b_FNOMIN, o_i);
    if ((b_i == h_i) && ((b_i == 1 ? h_i : b_i) == o_i) && (tmp_size_0[1] == s_i)) {
      tb_size[0] = 1;
      tb_size[1] = b_i;
      if (h_i - 1 >= 0) {
        tb_data = press_0 * press_0 - 4.0 * b_Q_FZ2;
      }
    } else {
      Chassis_binary_expand_op_b3i5z2tw(&tb_data, tb_size, b_VERTICAL_STIFFNESS, b_i, b_UNLOADED_RADIUS, h_i, b_FNOMIN,
                                        o_i, b_Q_FZ2, s_i);
    }

    Chassis_sqrt(&tb_data, tb_size);
    if (!tempInds) {
      b_idx_0 = tb_data;
    }

    b_Q_FZ1 = b_idx_0;
  }

  tmp_data = b_RIM_RADIUS + b_BOTTOM_OFFST;
  b_i = 0;
  press_0 = scaleFactors[0] * b_FNOMIN;
  lam_muy_prime_idx_3 = b_Q_FCX * Fx_ext[0] / b_FNOMIN;
  tb_data = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
  lam_muy_prime_idx_3 = b_Q_FCY * Fy_ext[0] / b_FNOMIN;
  e_y = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
  lam_muy_prime_idx_3 = rhoz[0] / b_UNLOADED_RADIUS;
  Fz_tmp = std::abs(Omega[0]);
  Fz[0] = std::fmax((((b_Q_V2 * Fz_tmp * b_UNLOADED_RADIUS / b_LONGVL + 1.0) - tb_data) - e_y) *
                        ((b_Q_FZ3 * b_gamma[0] * b_gamma[0] + b_Q_FZ1) * rhoz[0] / b_UNLOADED_RADIUS +
                         lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * b_Q_FZ2) *
                        (b_PFZ1 * dpi[0] + 1.0) * press_0,
                    (tmp_data - (b_UNLOADED_RADIUS + rhoz[0]) / std::cos(b_gamma[0])) * b_BOTTOM_STIFF);
  patch_b[0] = b_VERTICAL_STIFFNESS * scaleFactors[22];
  if (rhoz[0] > 0.0) {
    b_i = 1;
  }

  Fzo_prime_idx_0 = press_0;
  press_0 = scaleFactors[27] * b_FNOMIN;
  lam_muy_prime_idx_3 = b_Q_FCX * Fx_ext[1] / b_FNOMIN;
  tb_data = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
  lam_muy_prime_idx_3 = b_Q_FCY * Fy_ext[1] / b_FNOMIN;
  e_y = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
  lam_muy_prime_idx_3 = rhoz[1] / b_UNLOADED_RADIUS;
  Fz_tmp_0 = std::abs(Omega[1]);
  Fz[1] = std::fmax((((b_Q_V2 * Fz_tmp_0 * b_UNLOADED_RADIUS / b_LONGVL + 1.0) - tb_data) - e_y) *
                        ((b_Q_FZ3 * b_gamma[1] * b_gamma[1] + b_Q_FZ1) * rhoz[1] / b_UNLOADED_RADIUS +
                         lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * b_Q_FZ2) *
                        (b_PFZ1 * dpi[1] + 1.0) * press_0,
                    (tmp_data - (b_UNLOADED_RADIUS + rhoz[1]) / std::cos(b_gamma[1])) * b_BOTTOM_STIFF);
  patch_b[1] = b_VERTICAL_STIFFNESS * scaleFactors[49];
  if (rhoz[1] > 0.0) {
    b_i++;
  }

  Fzo_prime_idx_1 = press_0;
  press_0 = scaleFactors[54] * b_FNOMIN;
  lam_muy_prime_idx_3 = b_Q_FCX * Fx_ext[2] / b_FNOMIN;
  tb_data = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
  lam_muy_prime_idx_3 = b_Q_FCY * Fy_ext[2] / b_FNOMIN;
  e_y = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
  lam_muy_prime_idx_3 = rhoz[2] / b_UNLOADED_RADIUS;
  Fz_tmp_1 = std::abs(Omega[2]);
  Fz[2] = std::fmax((((b_Q_V2 * Fz_tmp_1 * b_UNLOADED_RADIUS / b_LONGVL + 1.0) - tb_data) - e_y) *
                        ((b_Q_FZ3 * b_gamma[2] * b_gamma[2] + b_Q_FZ1) * rhoz[2] / b_UNLOADED_RADIUS +
                         lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * b_Q_FZ2) *
                        (b_PFZ1 * dpi[2] + 1.0) * press_0,
                    (tmp_data - (b_UNLOADED_RADIUS + rhoz[2]) / std::cos(b_gamma[2])) * b_BOTTOM_STIFF);
  patch_b[2] = b_VERTICAL_STIFFNESS * scaleFactors[76];
  if (rhoz[2] > 0.0) {
    b_i++;
  }

  Fzo_prime_idx_2 = press_0;
  press_0 = scaleFactors[81] * b_FNOMIN;
  lam_muy_prime_idx_3 = b_Q_FCX * Fx_ext[3] / b_FNOMIN;
  tb_data = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
  lam_muy_prime_idx_3 = b_Q_FCY * Fy_ext[3] / b_FNOMIN;
  e_y = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
  lam_muy_prime_idx_3 = rhoz[3] / b_UNLOADED_RADIUS;
  Fz_tmp_2 = std::abs(Omega[3]);
  Fz[3] = std::fmax((((b_Q_V2 * Fz_tmp_2 * b_UNLOADED_RADIUS / b_LONGVL + 1.0) - tb_data) - e_y) *
                        ((b_Q_FZ3 * b_gamma_0 * b_gamma_0 + b_Q_FZ1) * rhoz[3] / b_UNLOADED_RADIUS +
                         lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * b_Q_FZ2) *
                        (b_PFZ1 * dpi[3] + 1.0) * press_0,
                    (tmp_data - (b_UNLOADED_RADIUS + rhoz[3]) / std::cos(b_gamma_0)) * b_BOTTOM_STIFF);
  patch_b[3] = b_VERTICAL_STIFFNESS * scaleFactors[103];
  if (rhoz[3] > 0.0) {
    b_i++;
  }

  h_i = b_i;
  b_i = 0;
  if (rhoz[0] > 0.0) {
    n_data[0] = 1;
    b_i = 1;
  }

  if (rhoz[1] > 0.0) {
    n_data[b_i] = 2;
    b_i++;
  }

  if (rhoz[2] > 0.0) {
    n_data[b_i] = 3;
    b_i++;
  }

  if (rhoz[3] > 0.0) {
    n_data[b_i] = 4;
  }

  b_i = 0;
  if (rhoz[0] > 0.0) {
    b_i = 1;
  }

  if (rhoz[1] > 0.0) {
    b_i++;
  }

  if (rhoz[2] > 0.0) {
    b_i++;
  }

  if (rhoz[3] > 0.0) {
    b_i++;
  }

  tmpDrphiVar_size = b_i;
  b_i = 0;
  if (rhoz[0] > 0.0) {
    tmpDrphiVar_data[0] = b_Q_FCY2 * Fy_ext[0] / b_FNOMIN;
    b_i = 1;
  }

  if (rhoz[1] > 0.0) {
    tmpDrphiVar_data[b_i] = b_Q_FCY2 * Fy_ext[1] / b_FNOMIN;
    b_i++;
  }

  if (rhoz[2] > 0.0) {
    tmpDrphiVar_data[b_i] = b_Q_FCY2 * Fy_ext[2] / b_FNOMIN;
    b_i++;
  }

  if (rhoz[3] > 0.0) {
    tmpDrphiVar_data[b_i] = b_Q_FCY2 * Fy_ext[3] / b_FNOMIN;
  }

  b_i = 0;
  if (rhoz[0] > 0.0) {
    b_i = 1;
  }

  if (rhoz[1] > 0.0) {
    b_i++;
  }

  if (rhoz[2] > 0.0) {
    b_i++;
  }

  if (rhoz[3] > 0.0) {
    b_i++;
  }

  o_size = b_i;
  b_i = 0;
  if (rhoz[0] > 0.0) {
    o_data[0] = 1;
    b_i = 1;
  }

  if (rhoz[1] > 0.0) {
    o_data[b_i] = 2;
    b_i++;
  }

  if (rhoz[2] > 0.0) {
    o_data[b_i] = 3;
    b_i++;
  }

  if (rhoz[3] > 0.0) {
    o_data[b_i] = 4;
  }

  for (b_i = 0; b_i < h_i; b_i++) {
    rhoz_data[b_i] = rhoz[n_data[b_i] - 1];
  }

  for (b_i = 0; b_i < h_i; b_i++) {
    Kyalpha[b_i] = rhoz[n_data[b_i] - 1];
  }

  b_i = h_i;
  for (b_i = 0; b_i < h_i; b_i++) {
    phi_t[b_i] = rhoz[n_data[b_i] - 1];
  }

  Chassis_abs(phi_t, &h_i, Bxalpha, &tmp_size_1);
  Chassis_abs(rhoz_data, &h_i, Bxalpha, &b_i);
  Chassis_abs(Kyalpha, &h_i, Bxalpha, &tmp_size_2);
  if ((h_i == b_i) && ((h_i == 1 ? tmp_size_2 : h_i) == tmpDrphiVar_size) &&
      (((h_i == 1 ? tmp_size_1 : h_i) == 1 ? tmpDrphiVar_size
        : h_i == 1                         ? tmp_size_1
                                           : h_i) == o_size)) {
    for (b_i = 0; b_i < h_i; b_i++) {
      rhoz_data[b_i] = rhoz[n_data[b_i] - 1];
    }

    Chassis_abs(rhoz_data, &h_i, Bxalpha, &tmp_size_1);
    tmp_size_1 = h_i;
    for (b_i = 0; b_i < h_i; b_i++) {
      e_data[b_i] =
          (Fz[n_data[b_i] - 1] / Bxalpha[b_i] + tmpDrphiVar_data[b_i]) * scaleFactors[(o_data[b_i] - 1) * 27 + 22];
    }
  } else {
    Chassis_binary_expand_op_b3i5z2t(e_data, &tmp_size_1, Fz, n_data, &h_i, rhoz, tmpDrphiVar_data, &tmpDrphiVar_size,
                                     scaleFactors, o_data, &o_size);
  }

  b_i = 0;
  Chassis_div0protect(Vx, b_VXLOW, phi_t, Drphi);
  tmp_data = patch_b[0];
  Fz_0 = Fz[0];
  if (rhoz[0] > 0.0) {
    tmp_data = e_data[0];
    b_i = 1;
  }

  FzUnSat[0] = Fz[0];
  if (Fz[0] < b_FZMIN) {
    Fz_0 = b_FZMIN;
  }

  if (Fz_0 > b_FZMAX) {
    Fz_0 = b_FZMAX;
  }

  dfz_0 = (Fz_0 - Fzo_prime_idx_0) / Fzo_prime_idx_0;
  epsilon_gamma[0] = (b_PECP2 * dfz_0 + 1.0) * b_PECP1;
  lam_muy_prime_idx_3 = Omega[0] * b_UNLOADED_RADIUS / b_LONGVL;
  lam_Cz_0 = Fz[0] / b_FNOMIN;
  Re_0 = (lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * b_Q_V1 + b_Q_RE0) * b_UNLOADED_RADIUS -
         (std::atan(b_BREFF * lam_Cz_0) * b_DREFF + b_FREFF * lam_Cz_0) * (b_FNOMIN / tmp_data);
  if (Re_0 < 0.001) {
    Re_0 = 0.001;
  }

  tmp_data = Fz[0] / (tmp_data * b_UNLOADED_RADIUS);
  patch_a[0] = (b_Q_RA2 * tmp_data + b_Q_RA1 * std::sqrt(tmp_data)) * b_UNLOADED_RADIUS;
  patch_b[0] = (b_Q_RB2 * tmp_data + b_Q_RB1 * rt_powd_snf(tmp_data, 0.33333333333333331)) * b_WIDTH;
  Bxalpha_0 = Re_0 * Omega[0] - Vx[0];
  tmp_data = Bxalpha_0 / Vsy[0];
  if (tmp_data < b_KPUMIN) {
    tmp_data = b_KPUMIN;
  }

  if (tmp_data > b_KPUMAX) {
    tmp_data = b_KPUMAX;
  }

  kappa_x[0] = (b_PHX2 * dfz_0 + b_PHX1) * scaleFactors[10] + tmp_data;
  e_y = rt_atan2d_snf(Vy[0], Vsy[0]);
  if (e_y < b_ALPMIN) {
    e_y = b_ALPMIN;
  }

  if (e_y > b_ALPMAX) {
    e_y = b_ALPMAX;
  }

  SHy_idx_0 = std::sin(b_gamma[0]);
  lam_muy_prime_idx_3 = -Vsy[0] * tmp_data;
  Vsy_0 = -Vsy[0] * std::tan(e_y);
  Vs_tmp = Vsy_0 * Vsy_0;
  Vs_0 = lam_muy_prime_idx_3 * lam_muy_prime_idx_3 + Vs_tmp;
  Fz[0] = Fz_0;
  dfz[0] = dfz_0;
  Re[0] = Re_0;
  lam_Cz[0] = std::sqrt(Vx[0] * Vx[0] + Vs_tmp);
  tb_data = Bxalpha_0 * Bxalpha_0;
  Vsy[0] = Vsy_0;
  kappa[0] = tmp_data;
  alpha[0] = e_y;
  gamma_star[0] = SHy_idx_0;
  tmp_data = patch_b[1];
  Fz_0 = Fz[1];
  if (rhoz[1] > 0.0) {
    tmp_data = e_data[b_i];
    b_i++;
  }

  FzUnSat[1] = Fz[1];
  if (Fz[1] < b_FZMIN) {
    Fz_0 = b_FZMIN;
  }

  if (Fz_0 > b_FZMAX) {
    Fz_0 = b_FZMAX;
  }

  dfz_0 = (Fz_0 - Fzo_prime_idx_1) / Fzo_prime_idx_1;
  epsilon_gamma[1] = (b_PECP2 * dfz_0 + 1.0) * b_PECP1;
  lam_muy_prime_idx_3 = Omega[1] * b_UNLOADED_RADIUS / b_LONGVL;
  lam_Cz_0 = Fz[1] / b_FNOMIN;
  Re_0 = (lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * b_Q_V1 + b_Q_RE0) * b_UNLOADED_RADIUS -
         (std::atan(b_BREFF * lam_Cz_0) * b_DREFF + b_FREFF * lam_Cz_0) * (b_FNOMIN / tmp_data);
  if (Re_0 < 0.001) {
    Re_0 = 0.001;
  }

  tmp_data = Fz[1] / (tmp_data * b_UNLOADED_RADIUS);
  patch_a[1] = (b_Q_RA2 * tmp_data + b_Q_RA1 * std::sqrt(tmp_data)) * b_UNLOADED_RADIUS;
  patch_b[1] = (b_Q_RB2 * tmp_data + b_Q_RB1 * rt_powd_snf(tmp_data, 0.33333333333333331)) * b_WIDTH;
  Bxalpha_0 = Re_0 * Omega[1] - Vx[1];
  tmp_data = Bxalpha_0 / Vsy[1];
  if (tmp_data < b_KPUMIN) {
    tmp_data = b_KPUMIN;
  }

  if (tmp_data > b_KPUMAX) {
    tmp_data = b_KPUMAX;
  }

  kappa_x[1] = (b_PHX2 * dfz_0 + b_PHX1) * scaleFactors[37] + tmp_data;
  e_y = rt_atan2d_snf(Vy[1], Vsy[1]);
  if (e_y < b_ALPMIN) {
    e_y = b_ALPMIN;
  }

  if (e_y > b_ALPMAX) {
    e_y = b_ALPMAX;
  }

  SHy_idx_1 = std::sin(b_gamma[1]);
  lam_muy_prime_idx_3 = -Vsy[1] * tmp_data;
  Vsy_0 = -Vsy[1] * std::tan(e_y);
  Vs_tmp = Vsy_0 * Vsy_0;
  Vs_1 = lam_muy_prime_idx_3 * lam_muy_prime_idx_3 + Vs_tmp;
  Fz[1] = Fz_0;
  dfz[1] = dfz_0;
  Re[1] = Re_0;
  lam_Cz[1] = std::sqrt(Vx[1] * Vx[1] + Vs_tmp);
  c_y_idx_1 = Bxalpha_0 * Bxalpha_0;
  Vsy[1] = Vsy_0;
  kappa[1] = tmp_data;
  alpha[1] = e_y;
  gamma_star[1] = SHy_idx_1;
  tmp_data = patch_b[2];
  Fz_0 = Fz[2];
  if (rhoz[2] > 0.0) {
    tmp_data = e_data[b_i];
    b_i++;
  }

  FzUnSat[2] = Fz[2];
  if (Fz[2] < b_FZMIN) {
    Fz_0 = b_FZMIN;
  }

  if (Fz_0 > b_FZMAX) {
    Fz_0 = b_FZMAX;
  }

  dfz_0 = (Fz_0 - Fzo_prime_idx_2) / Fzo_prime_idx_2;
  epsilon_gamma[2] = (b_PECP2 * dfz_0 + 1.0) * b_PECP1;
  lam_muy_prime_idx_3 = Omega[2] * b_UNLOADED_RADIUS / b_LONGVL;
  lam_Cz_0 = Fz[2] / b_FNOMIN;
  Re_0 = (lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * b_Q_V1 + b_Q_RE0) * b_UNLOADED_RADIUS -
         (std::atan(b_BREFF * lam_Cz_0) * b_DREFF + b_FREFF * lam_Cz_0) * (b_FNOMIN / tmp_data);
  if (Re_0 < 0.001) {
    Re_0 = 0.001;
  }

  tmp_data = Fz[2] / (tmp_data * b_UNLOADED_RADIUS);
  patch_a[2] = (b_Q_RA2 * tmp_data + b_Q_RA1 * std::sqrt(tmp_data)) * b_UNLOADED_RADIUS;
  patch_b[2] = (b_Q_RB2 * tmp_data + b_Q_RB1 * rt_powd_snf(tmp_data, 0.33333333333333331)) * b_WIDTH;
  Bxalpha_0 = Re_0 * Omega[2] - Vx[2];
  tmp_data = Bxalpha_0 / Vsy[2];
  if (tmp_data < b_KPUMIN) {
    tmp_data = b_KPUMIN;
  }

  if (tmp_data > b_KPUMAX) {
    tmp_data = b_KPUMAX;
  }

  kappa_x[2] = (b_PHX2 * dfz_0 + b_PHX1) * scaleFactors[64] + tmp_data;
  e_y = rt_atan2d_snf(Vy[2], Vsy[2]);
  if (e_y < b_ALPMIN) {
    e_y = b_ALPMIN;
  }

  if (e_y > b_ALPMAX) {
    e_y = b_ALPMAX;
  }

  SHy_idx_2 = std::sin(b_gamma[2]);
  lam_muy_prime_idx_3 = -Vsy[2] * tmp_data;
  Vsy_0 = -Vsy[2] * std::tan(e_y);
  Vs_tmp = Vsy_0 * Vsy_0;
  Vs_2 = lam_muy_prime_idx_3 * lam_muy_prime_idx_3 + Vs_tmp;
  Fz[2] = Fz_0;
  dfz[2] = dfz_0;
  Re[2] = Re_0;
  lam_Cz[2] = std::sqrt(Vx[2] * Vx[2] + Vs_tmp);
  Vs_tmp = Bxalpha_0 * Bxalpha_0;
  Vsy[2] = Vsy_0;
  kappa[2] = tmp_data;
  alpha[2] = e_y;
  gamma_star[2] = SHy_idx_2;
  tmp_data = patch_b[3];
  Fz_0 = Fz[3];
  if (rhoz[3] > 0.0) {
    tmp_data = e_data[b_i];
  }

  FzUnSat[3] = Fz[3];
  if (Fz[3] < b_FZMIN) {
    Fz_0 = b_FZMIN;
  }

  if (Fz_0 > b_FZMAX) {
    Fz_0 = b_FZMAX;
  }

  dfz_0 = (Fz_0 - press_0) / press_0;
  epsilon_gamma[3] = (b_PECP2 * dfz_0 + 1.0) * b_PECP1;
  lam_muy_prime_idx_3 = Omega[3] * b_UNLOADED_RADIUS / b_LONGVL;
  lam_Cz_0 = Fz[3] / b_FNOMIN;
  Re_0 = (lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * b_Q_V1 + b_Q_RE0) * b_UNLOADED_RADIUS -
         (std::atan(b_BREFF * lam_Cz_0) * b_DREFF + b_FREFF * lam_Cz_0) * (b_FNOMIN / tmp_data);
  if (Re_0 < 0.001) {
    Re_0 = 0.001;
  }

  tmp_data = Fz[3] / (tmp_data * b_UNLOADED_RADIUS);
  patch_a[3] = (b_Q_RA2 * tmp_data + b_Q_RA1 * std::sqrt(tmp_data)) * b_UNLOADED_RADIUS;
  patch_b[3] = (b_Q_RB2 * tmp_data + b_Q_RB1 * rt_powd_snf(tmp_data, 0.33333333333333331)) * b_WIDTH;
  Bxalpha_0 = Re_0 * Omega[3] - Vx[3];
  tmp_data = Bxalpha_0 / Vsy[3];
  if (tmp_data < b_KPUMIN) {
    tmp_data = b_KPUMIN;
  }

  if (tmp_data > b_KPUMAX) {
    tmp_data = b_KPUMAX;
  }

  kappa_x[3] = (b_PHX2 * dfz_0 + b_PHX1) * scaleFactors[91] + tmp_data;
  e_y = rt_atan2d_snf(Vy[3], Vsy[3]);
  if (e_y < b_ALPMIN) {
    e_y = b_ALPMIN;
  }

  if (e_y > b_ALPMAX) {
    e_y = b_ALPMAX;
  }

  SHy_idx_3 = std::sin(b_gamma_0);
  lam_muy_prime_idx_3 = -Vsy[3] * tmp_data;
  lam_Cz_0 = std::tan(e_y);
  Vsy_0 = -Vsy[3] * lam_Cz_0;
  Fz[3] = Fz_0;
  dfz[3] = dfz_0;
  Re[3] = Re_0;
  mu_y_idx_1 = Vsy_0 * Vsy_0;
  lam_Cz[3] = std::sqrt(Vx[3] * Vx[3] + mu_y_idx_1);
  kappa[3] = tmp_data;
  alpha[3] = e_y;
  gamma_star[3] = SHy_idx_3;
  Chassis_div0protect(lam_Cz, b_VXLOW, Mz, Drphi);
  isLowSpeed[0] = (std::sqrt(Vsy[0] * Vsy[0] + tb_data) < b_VXLOW);
  Vs_0 = scaleFactors[3] * std::sqrt(Vs_0) / b_LONGVL + 1.0;
  tb_data = Fx[0] / Vs_0;
  Vs[0] = Vs_0;
  isLowSpeed[1] = (std::sqrt(Vsy[1] * Vsy[1] + c_y_idx_1) < b_VXLOW);
  Vs_0 = scaleFactors[30] * std::sqrt(Vs_1) / b_LONGVL + 1.0;
  Vsy_0 = Fx[1] / Vs_0;
  Vs[1] = Vs_0;
  isLowSpeed[2] = (std::sqrt(Vsy[2] * Vsy[2] + Vs_tmp) < b_VXLOW);
  Vs_0 = scaleFactors[57] * std::sqrt(Vs_2) / b_LONGVL + 1.0;
  Fx_tmp = Fx[2] / Vs_0;
  Vs[2] = Vs_0;
  isLowSpeed[3] = (std::sqrt(Bxalpha_0 * Bxalpha_0 + mu_y_idx_1) < b_VXLOW);
  Vs_0 = std::sqrt(lam_muy_prime_idx_3 * lam_muy_prime_idx_3 + mu_y_idx_1) * scaleFactors[84] / b_LONGVL + 1.0;
  Fx_tmp_0 = Fx_0 / Vs_0;
  Fx_0 = dpi[0] * dpi[0];
  Vs_2 = lam_muy_idx_0 / Vs[0];
  lam_muy_prime_idx_0 = Vs_2 * 10.0 / (9.0 * Vs_2 + 1.0);
  Bxalpha_0 = SHy_idx_0 * SHy_idx_0;
  c_y_idx_1 =
      ((b_PPY3 * dpi[0] + 1.0) + Fx_0 * b_PPY4) * (b_PDY2 * dfz[0] + b_PDY1) * (1.0 - Bxalpha_0 * b_PDY3) * Vs_2;
  lam_muy_idx_0 = Vs_2;
  phi_t[0] = -psidot[0] / phi_t[0] * std::cos(alpha[0]);
  Vs_tmp = dpi[1] * dpi[1];
  Vs_2 = lam_muy_idx_1 / Vs[1];
  lam_muy_prime_idx_1 = Vs_2 * 10.0 / (9.0 * Vs_2 + 1.0);
  mu_y_tmp = SHy_idx_1 * SHy_idx_1;
  Re_0 = ((b_PPY3 * dpi[1] + 1.0) + Vs_tmp * b_PPY4) * (b_PDY2 * dfz[1] + b_PDY1) * (1.0 - mu_y_tmp * b_PDY3) * Vs_2;
  lam_muy_idx_1 = Vs_2;
  phi_t[1] = -psidot[1] / phi_t[1] * std::cos(alpha[1]);
  Dx_tmp = dpi[2] * dpi[2];
  Vs_2 = lam_muy_idx_2 / Vs[2];
  lam_muy_prime_idx_2 = Vs_2 * 10.0 / (9.0 * Vs_2 + 1.0);
  Vs_1 = SHy_idx_2 * SHy_idx_2;
  mu_y = ((b_PPY3 * dpi[2] + 1.0) + Dx_tmp * b_PPY4) * (b_PDY2 * dfz[2] + b_PDY1) * (1.0 - Vs_1 * b_PDY3) * Vs_2;
  lam_muy_idx_2 = Vs_2;
  phi_t[2] = -psidot[2] / phi_t[2] * std::cos(alpha[2]);
  Dx_tmp_0 = dpi[3] * dpi[3];
  Vs_2 = lam_muy / Vs_0;
  lam_muy_prime_idx_3 = Vs_2 * 10.0 / (9.0 * Vs_2 + 1.0);
  mu_y_idx_0 = SHy_idx_3 * SHy_idx_3;
  lam_muy =
      ((b_PPY3 * dpi[3] + 1.0) + Dx_tmp_0 * b_PPY4) * (b_PDY2 * dfz_0 + b_PDY1) * (1.0 - mu_y_idx_0 * b_PDY3) * Vs_2;
  phi_t[3] = -psidot[3] / phi_t[3] * std::cos(e_y);
  b_i = 0;
  if (isLowSpeed[0]) {
    b_i = 1;
  }

  if (isLowSpeed[1]) {
    b_i++;
  }

  if (isLowSpeed[2]) {
    b_i++;
  }

  if (isLowSpeed[3]) {
    b_i++;
  }

  h_i = b_i;
  b_i = 0;
  if (isLowSpeed[0]) {
    p_data[0] = 1;
    b_i = 1;
  }

  if (isLowSpeed[1]) {
    p_data[b_i] = 2;
    b_i++;
  }

  if (isLowSpeed[2]) {
    p_data[b_i] = 3;
    b_i++;
  }

  if (isLowSpeed[3]) {
    p_data[b_i] = 4;
  }

  b_i = 0;
  if (isLowSpeed[0]) {
    b_i = 1;
  }

  if (isLowSpeed[1]) {
    b_i++;
  }

  if (isLowSpeed[2]) {
    b_i++;
  }

  if (isLowSpeed[3]) {
    b_i++;
  }

  o_size = b_i;
  b_i = 0;
  if (isLowSpeed[0]) {
    q_data[0] = 1;
    b_i = 1;
  }

  if (isLowSpeed[1]) {
    q_data[b_i] = 2;
    b_i++;
  }

  if (isLowSpeed[2]) {
    q_data[b_i] = 3;
    b_i++;
  }

  if (isLowSpeed[3]) {
    q_data[b_i] = 4;
  }

  for (b_i = 0; b_i < h_i; b_i++) {
    rhoz_data[b_i] = Vx[p_data[b_i] - 1];
  }

  Chassis_abs(rhoz_data, &h_i, Bxalpha, &tmp_size_1);
  if (o_size == tmp_size_1) {
    for (b_i = 0; b_i < h_i; b_i++) {
      rhoz_data[b_i] = Vx[p_data[b_i] - 1];
    }

    Chassis_abs(rhoz_data, &h_i, Bxalpha, &tmp_size_1);
    tmpDrphiVar_size = o_size;
    for (b_i = 0; b_i < o_size; b_i++) {
      tmpDrphiVar_data[b_i] = phi_t[q_data[b_i] - 1] * (Bxalpha[b_i] / b_VXLOW);
    }
  } else {
    Chassis_binary_expand_op_b3i5z2(tmpDrphiVar_data, &tmpDrphiVar_size, phi_t, q_data, &o_size, Vx, p_data, &h_i,
                                    b_VXLOW);
  }

  b_i = 0;
  Chassis_div0protect_p(lam_Cz, Kyalpha);
  if (isLowSpeed[0]) {
    phi_t[0] = tmpDrphiVar_data[0];
    b_i = 1;
  }

  phi[0] = (psidot[0] - (1.0 - epsilon_gamma[0]) * Fz_tmp * SHy_idx_0) * -(1.0 / Kyalpha[0]);
  if (isLowSpeed[1]) {
    phi_t[1] = tmpDrphiVar_data[b_i];
    b_i++;
  }

  phi[1] = (psidot[1] - (1.0 - epsilon_gamma[1]) * Fz_tmp_0 * SHy_idx_1) * -(1.0 / Kyalpha[1]);
  if (isLowSpeed[2]) {
    phi_t[2] = tmpDrphiVar_data[b_i];
    b_i++;
  }

  phi[2] = (psidot[2] - (1.0 - epsilon_gamma[2]) * Fz_tmp_1 * SHy_idx_2) * -(1.0 / Kyalpha[2]);
  if (isLowSpeed[3]) {
    phi_t[3] = tmpDrphiVar_data[b_i];
  }

  phi[3] = (psidot[3] - (1.0 - epsilon_gamma[3]) * Fz_tmp_2 * SHy_idx_3) * -(1.0 / Kyalpha[3]);
  if (b_turnslip == 1.0) {
    isLowSpeed[0] = (std::abs(phi[0]) > 0.01);
    isLowSpeed[1] = (std::abs(phi[1]) > 0.01);
    isLowSpeed[2] = (std::abs(phi[2]) > 0.01);
    isLowSpeed[3] = (std::abs(phi[3]) > 0.01);
  } else {
    isLowSpeed[0] = false;
    isLowSpeed[1] = false;
    isLowSpeed[2] = false;
    isLowSpeed[3] = false;
  }

  Vsy[0] = b_RHY2 * dfz[0] + b_RHY1;
  Fz_tmp = b_REY2 * dfz[0] + b_REY1;
  Eykappa[0] = Fz_tmp;
  if (Fz_tmp > 1.0) {
    Eykappa[0] = 1.0;
  }

  mu_y_idx_1 = (Bxalpha_0 * b_RBY4 + b_RBY1) * std::cos(std::atan(b_RBY2 * alpha[0])) * scaleFactors[19];
  if (mu_y_idx_1 < 0.0) {
    mu_y_idx_1 = 0.0;
  }

  Bykappa[0] = mu_y_idx_1;
  Vsy[1] = b_RHY2 * dfz[1] + b_RHY1;
  Fz_tmp = b_REY2 * dfz[1] + b_REY1;
  Eykappa[1] = Fz_tmp;
  if (Fz_tmp > 1.0) {
    Eykappa[1] = 1.0;
  }

  mu_y_idx_1 = (mu_y_tmp * b_RBY4 + b_RBY1) * std::cos(std::atan(b_RBY2 * alpha[1])) * scaleFactors[46];
  if (mu_y_idx_1 < 0.0) {
    mu_y_idx_1 = 0.0;
  }

  Bykappa[1] = mu_y_idx_1;
  Vsy[2] = b_RHY2 * dfz[2] + b_RHY1;
  Fz_tmp = b_REY2 * dfz[2] + b_REY1;
  Eykappa[2] = Fz_tmp;
  if (Fz_tmp > 1.0) {
    Eykappa[2] = 1.0;
  }

  mu_y_idx_1 = (Vs_1 * b_RBY4 + b_RBY1) * std::cos(std::atan(b_RBY2 * alpha[2])) * scaleFactors[73];
  if (mu_y_idx_1 < 0.0) {
    mu_y_idx_1 = 0.0;
  }

  Bykappa[2] = mu_y_idx_1;
  Vsy[3] = b_RHY2 * dfz_0 + b_RHY1;
  Fz_tmp = b_REY2 * dfz_0 + b_REY1;
  Eykappa[3] = Fz_tmp;
  if (Fz_tmp > 1.0) {
    Eykappa[3] = 1.0;
  }

  mu_y_idx_1 = (mu_y_idx_0 * b_RBY4 + b_RBY1) * std::cos(std::atan(b_RBY2 * e_y)) * scaleFactors[100];
  if (mu_y_idx_1 < 0.0) {
    mu_y_idx_1 = 0.0;
  }

  Bykappa[3] = mu_y_idx_1;
  for (b_i = 0; b_i < 36; b_i++) {
    zeta[b_i] = 1.0;
  }

  b_i = 0;
  if (isLowSpeed[0]) {
    b_i = 1;
  }

  if (isLowSpeed[1]) {
    b_i++;
  }

  if (isLowSpeed[2]) {
    b_i++;
  }

  if (isLowSpeed[3]) {
    b_i++;
  }

  o_size = b_i;
  b_i = 0;
  if (isLowSpeed[0]) {
    r_data[0] = 1;
    b_i = 1;
  }

  if (isLowSpeed[1]) {
    r_data[b_i] = 2;
    b_i++;
  }

  if (isLowSpeed[2]) {
    r_data[b_i] = 3;
    b_i++;
  }

  if (isLowSpeed[3]) {
    r_data[b_i] = 4;
  }

  for (b_i = 0; b_i < o_size; b_i++) {
    zeta[9 * (r_data[b_i] - 1)] = 0.0;
  }

  if (Chassis_any(isLowSpeed)) {
    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    b_x[0] = std::cos(std::atan(b_PDXP3 * kappa[0]));
    SVygamma[0] = std::cos(std::atan(b_PDYP3 * std::tan(alpha[0])));
    if (isLowSpeed[1]) {
      b_i++;
    }

    b_x[1] = std::cos(std::atan(b_PDXP3 * kappa[1]));
    SVygamma[1] = std::cos(std::atan(b_PDYP3 * std::tan(alpha[1])));
    if (isLowSpeed[2]) {
      b_i++;
    }

    b_x[2] = std::cos(std::atan(b_PDXP3 * kappa[2]));
    SVygamma[2] = std::cos(std::atan(b_PDYP3 * std::tan(alpha[2])));
    if (isLowSpeed[3]) {
      b_i++;
    }

    b_x[3] = std::cos(std::atan(b_PDXP3 * tmp_data));
    SVygamma[3] = std::cos(std::atan(b_PDYP3 * lam_Cz_0));
    o_size = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      s_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      s_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      s_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      s_data[b_i] = 4;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    h_i = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      t_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      t_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      t_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      t_data[b_i] = 4;
    }

    for (b_i = 0; b_i < o_size; b_i++) {
      s = s_data[b_i];
      tmpDrphiVar_data[b_i] = (dfz[s - 1] * b_PDXP2 + 1.0) * b_PDXP1 * b_x[s - 1] * b_UNLOADED_RADIUS * phi[s - 1];
    }

    Chassis_atan(tmpDrphiVar_data, &o_size);
    Chassis_cos(tmpDrphiVar_data, &o_size);
    for (b_i = 0; b_i < h_i; b_i++) {
      zeta[9 * (t_data[b_i] - 1) + 1] = tmpDrphiVar_data[b_i];
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    tmp_size_2 = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      u_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      u_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      u_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      u_data[b_i] = 4;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    h_i = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      v_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      v_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      v_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      v_data[b_i] = 4;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    w_size = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      w_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      w_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      w_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      w_data[b_i] = 4;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    o_size = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      x_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      x_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      x_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      x_data[b_i] = 4;
    }

    for (b_i = 0; b_i < h_i; b_i++) {
      rhoz_data[b_i] = phi[v_data[b_i] - 1];
    }

    Chassis_abs(rhoz_data, &h_i, Bxalpha, &tmp_size_1);
    tmpDrphiVar_size = tmp_size_1;
    o_i = (tmp_size_1 / 2) << 1;
    s_i = o_i - 2;
    for (b_i = 0; b_i <= s_i; b_i += 2) {
      __m128d tmp;
      tmp = _mm_loadu_pd(&Bxalpha[b_i]);
      _mm_storeu_pd(&tmpDrphiVar_data[b_i], _mm_mul_pd(_mm_set1_pd(b_UNLOADED_RADIUS), tmp));
    }

    for (b_i = o_i; b_i < tmp_size_1; b_i++) {
      tmpDrphiVar_data[b_i] = b_UNLOADED_RADIUS * Bxalpha[b_i];
    }

    Chassis_sqrt_c(tmpDrphiVar_data, &tmp_size_1);
    for (b_i = 0; b_i < tmp_size_2; b_i++) {
      rhoz_data[b_i] = phi[u_data[b_i] - 1];
    }

    for (b_i = 0; b_i < tmp_size_2; b_i++) {
      Kyalpha[b_i] = phi[u_data[b_i] - 1];
    }

    Chassis_abs(Kyalpha, &tmp_size_2, Bxalpha, &tmp_size_1);
    Chassis_abs(rhoz_data, &tmp_size_2, Bxalpha, &b_i);
    if ((b_i == tmpDrphiVar_size) && ((tmp_size_1 == 1 ? tmpDrphiVar_size : tmp_size_1) == w_size)) {
      for (b_i = 0; b_i < tmp_size_2; b_i++) {
        rhoz_data[b_i] = phi[u_data[b_i] - 1];
      }

      Chassis_abs(rhoz_data, &tmp_size_2, Bxalpha, &tmp_size_1);
      tmp_size_1 = w_size;
      for (b_i = 0; b_i < w_size; b_i++) {
        s = w_data[b_i];
        e_data[b_i] = (dfz[s - 1] * b_PDYP2 + 1.0) * b_PDYP1 * SVygamma[s - 1] *
                      (b_UNLOADED_RADIUS * Bxalpha[b_i] + b_PDYP4 * tmpDrphiVar_data[b_i]);
      }
    } else {
      Chassis_binary_expand_op_b3i5z(e_data, &tmp_size_1, b_PDYP1, b_PDYP2, dfz, SVygamma, w_data, &w_size,
                                     b_UNLOADED_RADIUS, phi, u_data, &tmp_size_2, b_PDYP4, tmpDrphiVar_data,
                                     &tmpDrphiVar_size);
    }

    Chassis_atan(e_data, &tmp_size_1);
    Chassis_cos(e_data, &tmp_size_1);
    for (b_i = 0; b_i < o_size; b_i++) {
      zeta[9 * (x_data[b_i] - 1) + 2] = e_data[b_i];
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    o_size = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      y_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      y_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      y_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      y_data[b_i] = 4;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    h_i = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      ab_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      ab_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      ab_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      ab_data[b_i] = 4;
    }

    Fz_tmp = b_UNLOADED_RADIUS * b_UNLOADED_RADIUS * b_PKYP1;
    tmpDrphiVar_size = o_size;
    for (b_i = 0; b_i < o_size; b_i++) {
      Fz_tmp_0 = phi[y_data[b_i] - 1];
      tmpDrphiVar_data[b_i] = Fz_tmp_0 * Fz_tmp_0 * Fz_tmp;
    }

    Chassis_atan(tmpDrphiVar_data, &o_size);
    Chassis_cos(tmpDrphiVar_data, &o_size);
    for (b_i = 0; b_i < h_i; b_i++) {
      zeta[9 * (ab_data[b_i] - 1) + 3] = tmpDrphiVar_data[b_i];
    }
  }

  dfz_0 = 1.0 - b_PKY3 * 0.0;
  lam_Cz_0 = Fz[0] / Fzo_prime_idx_0;
  Vs_0 = (b_PPY1 * dpi[0] + 1.0) * (b_PKY1 * Fzo_prime_idx_0);
  mu_y_idx_1 = std::sin(std::atan(lam_Cz_0 / (Bxalpha_0 * b_PKY5 + b_PKY2) / (b_PPY2 * dpi[0] + 1.0)) * b_PKY4);
  Fz_tmp = (1.0 - b_PKY3 * std::abs(SHy_idx_0)) * Vs_0 * mu_y_idx_1 * zeta[3] * scaleFactors[5];
  Vs[0] = Vs_0 * dfz_0 * mu_y_idx_1 * zeta[3] * scaleFactors[5];
  Kyalpha[0] = Fz_tmp;
  Mzphiinf[0] = lam_Cz_0;
  lam_Cz_0 = Fz[1] / Fzo_prime_idx_1;
  Vs_0 = (b_PPY1 * dpi[1] + 1.0) * (b_PKY1 * Fzo_prime_idx_1);
  mu_y_idx_1 = std::sin(std::atan(lam_Cz_0 / (mu_y_tmp * b_PKY5 + b_PKY2) / (b_PPY2 * dpi[1] + 1.0)) * b_PKY4);
  Fz_tmp_0 = (1.0 - b_PKY3 * std::abs(SHy_idx_1)) * Vs_0 * mu_y_idx_1 * zeta[12] * scaleFactors[32];
  Vs[1] = Vs_0 * dfz_0 * mu_y_idx_1 * zeta[12] * scaleFactors[32];
  Kyalpha[1] = Fz_tmp_0;
  Mzphiinf[1] = lam_Cz_0;
  lam_Cz_0 = Fz[2] / Fzo_prime_idx_2;
  Vs_0 = (b_PPY1 * dpi[2] + 1.0) * (b_PKY1 * Fzo_prime_idx_2);
  mu_y_idx_1 = std::sin(std::atan(lam_Cz_0 / (Vs_1 * b_PKY5 + b_PKY2) / (b_PPY2 * dpi[2] + 1.0)) * b_PKY4);
  Fz_tmp_1 = (1.0 - b_PKY3 * std::abs(SHy_idx_2)) * Vs_0 * mu_y_idx_1 * zeta[21] * scaleFactors[59];
  Vs[2] = Vs_0 * dfz_0 * mu_y_idx_1 * zeta[21] * scaleFactors[59];
  Kyalpha[2] = Fz_tmp_1;
  Mzphiinf[2] = lam_Cz_0;
  lam_Cz_0 = Fz_0 / press_0;
  Vs_0 = (b_PPY1 * dpi[3] + 1.0) * (b_PKY1 * press_0);
  mu_y_idx_1 = std::sin(std::atan(lam_Cz_0 / (mu_y_idx_0 * b_PKY5 + b_PKY2) / (b_PPY2 * dpi[3] + 1.0)) * b_PKY4);
  Fz_tmp_2 = (1.0 - b_PKY3 * std::abs(SHy_idx_3)) * Vs_0 * mu_y_idx_1 * zeta[30] * scaleFactors[86];
  Vs[3] = Vs_0 * dfz_0 * mu_y_idx_1 * zeta[30] * scaleFactors[86];
  Kyalpha[3] = Fz_tmp_2;
  Chassis_div0protect(Kyalpha, 0.0001, b_x, lam_Cz);
  Chassis_div0protect(Vs, 0.0001, Bxalpha, lam_Cz);
  b_idx_0 = b_PHYP1;
  if (b_PHYP1 < 0.0) {
    b_idx_0 = 0.0;
  }

  mu_y_idx_1 = std::tanh(Vx[0]);
  lam_Cz[0] = (b_PHYP3 * dfz[0] + b_PHYP2) * mu_y_idx_1;
  lam_Cz_tmp_0 = std::tanh(Vx[1]);
  lam_Cz[1] = (b_PHYP3 * dfz[1] + b_PHYP2) * lam_Cz_tmp_0;
  mu_y_idx_2 = std::tanh(Vx[2]);
  lam_Cz[2] = (b_PHYP3 * dfz[2] + b_PHYP2) * mu_y_idx_2;
  dfz_0 = std::tanh(Vx[3]);
  lam_Cz_tmp = (b_PHYP3 * dfz[3] + b_PHYP2) * dfz_0;
  lam_Cz[3] = lam_Cz_tmp;
  Vs_0 = b_PHYP4;
  if (b_PHYP4 > 1.0) {
    Vs_0 = 1.0;
  }

  rhoz_data[0] = (b_PKY7 * dfz[0] + b_PKY6) * Fz[0] * (b_PPY5 * dpi[0] + 1.0) * scaleFactors[14] /
                 (1.0 - epsilon_gamma[0]) / (b_idx_0 * lam_Cz[0] * Bxalpha[0]);
  e_data[0] = b_UNLOADED_RADIUS * phi[0];
  rhoz_data[1] = (b_PKY7 * dfz[1] + b_PKY6) * Fz[1] * (b_PPY5 * dpi[1] + 1.0) * scaleFactors[41] /
                 (1.0 - epsilon_gamma[1]) / (b_idx_0 * lam_Cz[1] * Bxalpha[1]);
  e_data[1] = b_UNLOADED_RADIUS * phi[1];
  rhoz_data[2] = (b_PKY7 * dfz[2] + b_PKY6) * Fz[2] * (b_PPY5 * dpi[2] + 1.0) * scaleFactors[68] /
                 (1.0 - epsilon_gamma[2]) / (b_idx_0 * lam_Cz[2] * Bxalpha[2]);
  e_data[2] = b_UNLOADED_RADIUS * phi[2];
  rhoz_data[3] = (b_PKY7 * dfz[3] + b_PKY6) * Fz_0 * (b_PPY5 * dpi[3] + 1.0) * scaleFactors[95] /
                 (1.0 - epsilon_gamma[3]) / (lam_Cz_tmp * b_idx_0 * Bxalpha[3]);
  e_data[3] = b_UNLOADED_RADIUS * phi[3];
  Chassis_magicsin(lam_Cz, b_idx_0, rhoz_data, Vs_0, e_data, Kyalpha);
  Vs[0] = Kyalpha[0] * mu_y_idx_1;
  SVygamma[0] = (b_PVY4 * dfz[0] + b_PVY3) * Fz[0] * SHy_idx_0 * zeta[2] * scaleFactors[14] * lam_muy_prime_idx_0;
  SHy_idx_0 = 0.0;
  Drphi[0] = kappa[0] + Vsy[0];
  Vs[1] = Kyalpha[1] * lam_Cz_tmp_0;
  SVygamma[1] = (b_PVY4 * dfz[1] + b_PVY3) * Fz[1] * SHy_idx_1 * zeta[11] * scaleFactors[41] * lam_muy_prime_idx_1;
  SHy_idx_1 = 0.0;
  Drphi[1] = kappa[1] + Vsy[1];
  Vs[2] = Kyalpha[2] * mu_y_idx_2;
  SVygamma[2] = (b_PVY4 * dfz[2] + b_PVY3) * Fz[2] * SHy_idx_2 * zeta[20] * scaleFactors[68] * lam_muy_prime_idx_2;
  SHy_idx_2 = 0.0;
  Drphi[2] = kappa[2] + Vsy[2];
  Vs[3] = Kyalpha[3] * dfz_0;
  SVygamma[3] = (b_PVY4 * dfz[3] + b_PVY3) * Fz_0 * SHy_idx_3 * zeta[29] * scaleFactors[95] * lam_muy_prime_idx_3;
  SHy_idx_3 = 0.0;
  Drphi[3] = tmp_data + Vsy[3];
  Chassis_magiccos(b_RCY1, Bykappa, Eykappa, Drphi, Kyalpha);
  Chassis_magiccos(b_RCY1, Bykappa, Eykappa, Vsy, rhoz_data);
  Vs_0 = Kyalpha[0] / rhoz_data[0];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  Fy[0] = Vs_0;
  Vs_0 = Kyalpha[1] / rhoz_data[1];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  Fy[1] = Vs_0;
  Vs_0 = Kyalpha[2] / rhoz_data[2];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  Fy[2] = Vs_0;
  Vs_0 = Kyalpha[3] / rhoz_data[3];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  Fy[3] = Vs_0;
  if (Chassis_any(isLowSpeed)) {
    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    h_i = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      bb_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      bb_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      bb_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      bb_data[b_i] = 4;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    tmpDrphiVar_size = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      cb_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      cb_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      cb_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      cb_data[b_i] = 4;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    tmp_size_1 = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      db_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      db_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      db_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      db_data[b_i] = 4;
    }

    if (tmpDrphiVar_size == h_i) {
      for (b_i = 0; b_i < tmp_size_1; b_i++) {
        s = bb_data[b_i];
        zeta[9 * (db_data[b_i] - 1) + 4] = (Vs[cb_data[b_i] - 1] + 1.0) - SVygamma[s - 1] / b_x[s - 1];
      }
    } else {
      Chassis_binary_expand_op_b3i5(zeta, db_data, &tmp_size_1, Vs, cb_data, &tmpDrphiVar_size, SVygamma, bb_data, &h_i,
                                    b_x);
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    o_size = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      eb_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      eb_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      eb_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      eb_data[b_i] = 4;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    h_i = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      fb_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      fb_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      fb_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      fb_data[b_i] = 4;
    }

    Vs_0 = b_QDTP1 * b_UNLOADED_RADIUS;
    for (b_i = 0; b_i < o_size; b_i++) {
      tmpDrphiVar_data[b_i] = phi[eb_data[b_i] - 1] * Vs_0;
    }

    Chassis_atan(tmpDrphiVar_data, &o_size);
    Chassis_cos(tmpDrphiVar_data, &o_size);
    for (b_i = 0; b_i < h_i; b_i++) {
      zeta[9 * (fb_data[b_i] - 1) + 5] = tmpDrphiVar_data[b_i];
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    o_size = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      gb_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      gb_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      gb_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      gb_data[b_i] = 4;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    h_i = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      hb_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      hb_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      hb_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      hb_data[b_i] = 4;
    }

    Vs_0 = b_QBRP1 * b_UNLOADED_RADIUS;
    tmpDrphiVar_size = o_size;
    for (b_i = 0; b_i < o_size; b_i++) {
      tmpDrphiVar_data[b_i] = phi[gb_data[b_i] - 1] * Vs_0;
    }

    Chassis_atan(tmpDrphiVar_data, &o_size);
    Chassis_cos(tmpDrphiVar_data, &o_size);
    for (b_i = 0; b_i < h_i; b_i++) {
      zeta[9 * (hb_data[b_i] - 1) + 6] = tmpDrphiVar_data[b_i];
    }

    Vs_0 = b_QCRP2 * b_UNLOADED_RADIUS;
    mu_y_idx_1 = b_QCRP1 * c_y_idx_1 * b_UNLOADED_RADIUS * Fz[0] * std::sqrt(Mzphiinf[0]) * scaleFactors[26];
    if (mu_y_idx_1 < 0.0) {
      mu_y_idx_1 = 1.0E-6;
    }

    Mzphiinf[0] = mu_y_idx_1;
    Kyalpha[0] = std::atan(Vs_0 * std::abs(phi_t[0]));
    mu_y_idx_1 = b_QCRP1 * Re_0 * b_UNLOADED_RADIUS * Fz[1] * std::sqrt(Mzphiinf[1]) * scaleFactors[53];
    if (mu_y_idx_1 < 0.0) {
      mu_y_idx_1 = 1.0E-6;
    }

    Mzphiinf[1] = mu_y_idx_1;
    Kyalpha[1] = std::atan(Vs_0 * std::abs(phi_t[1]));
    mu_y_idx_1 = b_QCRP1 * mu_y * b_UNLOADED_RADIUS * Fz[2] * std::sqrt(Mzphiinf[2]) * scaleFactors[80];
    if (mu_y_idx_1 < 0.0) {
      mu_y_idx_1 = 1.0E-6;
    }

    Mzphiinf[2] = mu_y_idx_1;
    Kyalpha[2] = std::atan(Vs_0 * std::abs(phi_t[2]));
    mu_y_idx_1 = b_QCRP1 * lam_muy * b_UNLOADED_RADIUS * Fz_0 * std::sqrt(lam_Cz_0) * scaleFactors[107];
    if (mu_y_idx_1 < 0.0) {
      mu_y_idx_1 = 1.0E-6;
    }

    Mzphiinf[3] = mu_y_idx_1;
    Kyalpha[3] = std::atan(Vs_0 * std::abs(phi_t[3]));
    b_idx_0 = b_QDRP1;
    Vs_0 = b_QDRP2;
    if (b_QDRP1 < 0.0) {
      b_idx_0 = 0.0;
    }

    if (b_QDRP2 < 0.0) {
      Vs_0 = 0.0;
    }

    mu_y_idx_2 = std::sin(1.5707963267948966 * b_idx_0);
    lam_Cz_0 = Mzphiinf[0] / mu_y_idx_2;
    if (std::isnan(1.0 - epsilon_gamma[0])) {
      dfz_0 = (rtNaN);
    } else if (1.0 - epsilon_gamma[0] < 0.0) {
      dfz_0 = -1.0;
    } else {
      dfz_0 = (1.0 - epsilon_gamma[0] > 0.0);
    }

    rhoz_data[0] = ((b_QDZ11 * dfz[0] + b_QDZ10) * std::abs(b_gamma[0]) + (b_QDZ9 * dfz[0] + b_QDZ8)) *
                   (Fz[0] * b_UNLOADED_RADIUS) * scaleFactors[15] /
                   (b_idx_0 * lam_Cz_0 * (1.0 - epsilon_gamma[0]) + 0.0001 * dfz_0);
    e_data[0] = -b_UNLOADED_RADIUS * phi[0];
    lam_Cz[0] = lam_Cz_0;
    lam_Cz_0 = Mzphiinf[1] / mu_y_idx_2;
    if (std::isnan(1.0 - epsilon_gamma[1])) {
      dfz_0 = (rtNaN);
    } else if (1.0 - epsilon_gamma[1] < 0.0) {
      dfz_0 = -1.0;
    } else {
      dfz_0 = (1.0 - epsilon_gamma[1] > 0.0);
    }

    rhoz_data[1] = ((b_QDZ11 * dfz[1] + b_QDZ10) * std::abs(b_gamma[1]) + (b_QDZ9 * dfz[1] + b_QDZ8)) *
                   (Fz[1] * b_UNLOADED_RADIUS) * scaleFactors[42] /
                   (b_idx_0 * lam_Cz_0 * (1.0 - epsilon_gamma[1]) + 0.0001 * dfz_0);
    e_data[1] = -b_UNLOADED_RADIUS * phi[1];
    lam_Cz[1] = lam_Cz_0;
    lam_Cz_0 = Mzphiinf[2] / mu_y_idx_2;
    if (std::isnan(1.0 - epsilon_gamma[2])) {
      dfz_0 = (rtNaN);
    } else if (1.0 - epsilon_gamma[2] < 0.0) {
      dfz_0 = -1.0;
    } else {
      dfz_0 = (1.0 - epsilon_gamma[2] > 0.0);
    }

    rhoz_data[2] = ((b_QDZ11 * dfz[2] + b_QDZ10) * std::abs(b_gamma[2]) + (b_QDZ9 * dfz[2] + b_QDZ8)) *
                   (Fz[2] * b_UNLOADED_RADIUS) * scaleFactors[69] /
                   (b_idx_0 * lam_Cz_0 * (1.0 - epsilon_gamma[2]) + 0.0001 * dfz_0);
    e_data[2] = -b_UNLOADED_RADIUS * phi[2];
    lam_Cz[2] = lam_Cz_0;
    lam_Cz_0 = mu_y_idx_1 / mu_y_idx_2;
    if (std::isnan(1.0 - epsilon_gamma[3])) {
      dfz_0 = (rtNaN);
    } else if (1.0 - epsilon_gamma[3] < 0.0) {
      dfz_0 = -1.0;
    } else {
      dfz_0 = (1.0 - epsilon_gamma[3] > 0.0);
    }

    rhoz_data[3] = ((b_QDZ11 * dfz[3] + b_QDZ10) * std::abs(b_gamma_0) + (b_QDZ9 * dfz[3] + b_QDZ8)) *
                   (Fz_0 * b_UNLOADED_RADIUS) * scaleFactors[96] /
                   (b_idx_0 * lam_Cz_0 * (1.0 - epsilon_gamma[3]) + 0.0001 * dfz_0);
    e_data[3] = -b_UNLOADED_RADIUS * phi[3];
    lam_Cz[3] = lam_Cz_0;
    Chassis_magicsin(lam_Cz, b_idx_0, rhoz_data, Vs_0, e_data, Drphi);
    b_i = 0;
    if (std::isnan(Drphi[0])) {
      lam_Cz_0 = (rtNaN);
    } else if (Drphi[0] < 0.0) {
      lam_Cz_0 = -1.0;
    } else {
      lam_Cz_0 = (Drphi[0] > 0.0);
    }

    if (lam_Cz_0 == 0.0) {
      lam_Cz_0 = 1.0;
    }

    if (isLowSpeed[0]) {
      b_i = 1;
    }

    lam_Cz[0] = lam_Cz_0;
    if (std::isnan(Drphi[1])) {
      lam_Cz_0 = (rtNaN);
    } else if (Drphi[1] < 0.0) {
      lam_Cz_0 = -1.0;
    } else {
      lam_Cz_0 = (Drphi[1] > 0.0);
    }

    if (lam_Cz_0 == 0.0) {
      lam_Cz_0 = 1.0;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    lam_Cz[1] = lam_Cz_0;
    if (std::isnan(Drphi[2])) {
      lam_Cz_0 = (rtNaN);
    } else if (Drphi[2] < 0.0) {
      lam_Cz_0 = -1.0;
    } else {
      lam_Cz_0 = (Drphi[2] > 0.0);
    }

    if (lam_Cz_0 == 0.0) {
      lam_Cz_0 = 1.0;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    lam_Cz[2] = lam_Cz_0;
    if (std::isnan(Drphi[3])) {
      lam_Cz_0 = (rtNaN);
    } else if (Drphi[3] < 0.0) {
      lam_Cz_0 = -1.0;
    } else {
      lam_Cz_0 = (Drphi[3] > 0.0);
    }

    if (lam_Cz_0 == 0.0) {
      lam_Cz_0 = 1.0;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    lam_Cz[3] = lam_Cz_0;
    h_i = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      ib_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      ib_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      ib_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      ib_data[b_i] = 4;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    o_size = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      kb_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      kb_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      kb_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      kb_data[b_i] = 4;
    }

    for (b_i = 0; b_i < h_i; b_i++) {
      e_data[b_i] = Drphi[ib_data[b_i] - 1];
    }

    for (b_i = 0; b_i < h_i; b_i++) {
      rhoz_data[b_i] = Drphi[ib_data[b_i] - 1];
    }

    Chassis_abs(rhoz_data, &h_i, Bxalpha, &tmp_size_1);
    Chassis_abs(e_data, &h_i, Bxalpha, &b_i);
    if ((b_i == o_size) && ((tmp_size_1 == 1 ? o_size : tmp_size_1) == o_size)) {
      for (b_i = 0; b_i < h_i; b_i++) {
        e_data[b_i] = Drphi[ib_data[b_i] - 1];
      }

      Chassis_abs(e_data, &h_i, Bxalpha, &tmp_size_1);
      tmpDrphiVar_size = o_size;
      for (b_i = 0; b_i < o_size; b_i++) {
        s = kb_data[b_i];
        tmpDrphiVar_data[b_i] = Mzphiinf[s - 1] * 2.0 / 3.1415926535897931 * Kyalpha[s - 1] * Fy[s - 1] /
                                (lam_Cz[s - 1] * 0.0001 - Bxalpha[b_i]);
      }
    } else {
      Chassis_binary_expand_op_b3i(tmpDrphiVar_data, &tmpDrphiVar_size, Mzphiinf, Kyalpha, Fy, kb_data, &o_size, Drphi,
                                   ib_data, &h_i, lam_Cz);
    }

    for (h_i = 0; h_i < tmpDrphiVar_size; h_i++) {
      Vs_0 = tmpDrphiVar_data[h_i];
      if (Vs_0 > 1.0) {
        Vs_0 = 1.0;
      }

      tmpDrphiVar_data[h_i] = Vs_0;
    }

    for (h_i = 0; h_i < tmpDrphiVar_size; h_i++) {
      Vs_0 = tmpDrphiVar_data[h_i];
      if (Vs_0 < -1.0) {
        Vs_0 = -1.0;
      }

      tmpDrphiVar_data[h_i] = Vs_0;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    o_size = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      nb_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      nb_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      nb_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      nb_data[b_i] = 4;
    }

    Chassis_acos(tmpDrphiVar_data, &tmpDrphiVar_size);
    for (b_i = 0; b_i < o_size; b_i++) {
      zeta[9 * (nb_data[b_i] - 1) + 7] = 0.63661977236758138 * tmpDrphiVar_data[b_i];
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    o_size = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      ob_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      ob_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      ob_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      ob_data[b_i] = 4;
    }

    for (b_i = 0; b_i < o_size; b_i++) {
      s = ob_data[b_i];
      zeta[9 * (s - 1) + 8] = Drphi[s - 1] + 1.0;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    h_i = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      j_data[0] = 0.0 * dfz[0] * scaleFactors[11];
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      j_data[b_i] = 0.0 * dfz[1] * scaleFactors[38];
      b_i++;
    }

    if (isLowSpeed[2]) {
      j_data[b_i] = 0.0 * dfz[2] * scaleFactors[65];
      b_i++;
    }

    if (isLowSpeed[3]) {
      j_data[b_i] = 0.0 * dfz[3] * scaleFactors[92];
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    tmp_size_1 = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      qb_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      qb_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      qb_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      qb_data[b_i] = 4;
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      b_i++;
    }

    if (isLowSpeed[2]) {
      b_i++;
    }

    if (isLowSpeed[3]) {
      b_i++;
    }

    o_size = b_i;
    b_i = 0;
    if (isLowSpeed[0]) {
      rb_data[0] = 1;
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      rb_data[b_i] = 2;
      b_i++;
    }

    if (isLowSpeed[2]) {
      rb_data[b_i] = 3;
      b_i++;
    }

    if (isLowSpeed[3]) {
      rb_data[b_i] = 4;
    }

    if ((h_i == tmp_size_1) && ((h_i == 1 ? tmp_size_1 : h_i) == o_size)) {
      tmpDrphiVar_size = h_i;
      for (b_i = 0; b_i < h_i; b_i++) {
        s = rb_data[b_i];
        tmpDrphiVar_data[b_i] = (Vs[qb_data[b_i] - 1] + j_data[b_i]) - SVygamma[s - 1] / b_x[s - 1];
      }
    } else {
      Chassis_binary_expand_op_b3(tmpDrphiVar_data, &tmpDrphiVar_size, j_data, &h_i, Vs, qb_data, &tmp_size_1, SVygamma,
                                  rb_data, &o_size, b_x);
    }

    b_i = 0;
    if (isLowSpeed[0]) {
      SHy_idx_0 = tmpDrphiVar_data[0];
      b_i = 1;
    }

    if (isLowSpeed[1]) {
      SHy_idx_1 = tmpDrphiVar_data[b_i];
      b_i++;
    }

    if (isLowSpeed[2]) {
      SHy_idx_2 = tmpDrphiVar_data[b_i];
      b_i++;
    }

    if (isLowSpeed[3]) {
      SHy_idx_3 = tmpDrphiVar_data[b_i];
    }
  }

  lam_Cz_0 = b_PCX1 * scaleFactors[6];
  if (lam_Cz_0 < 0.0) {
    lam_Cz_0 = 0.0;
  }

  dfz_0 = b_gamma[0] * b_gamma[0];
  Vs_0 = ((b_PPX3 * dpi[0] + 1.0) + b_PPX4 * Fx_0) * (b_PDX2 * dfz[0] + b_PDX1) * (1.0 - dfz_0 * b_PDX3) * tb_data *
         Fz[0] * zeta[1];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  mu_y_idx_2_tmp = dfz[0] * dfz[0];
  mu_y_idx_2 = ((b_PEX2 * dfz[0] + b_PEX1) + mu_y_idx_2_tmp * b_PEX3) * (1.0 - std::tanh(10.0 * kappa_x[0]) * b_PEX4) *
               scaleFactors[8];
  if (mu_y_idx_2 > 1.0) {
    mu_y_idx_2 = 1.0;
  }

  phi[0] = lam_Cz_0 * Vs_0;
  Bykappa[0] = lam_Cz_0;
  epsilon_gamma[0] = Vs_0;
  Eykappa[0] = mu_y_idx_2;
  phi_t[0] = (b_PKX2 * dfz[0] + b_PKX1) * Fz[0] * std::exp(b_PKX3 * dfz[0]) *
             ((b_PPX1 * dpi[0] + 1.0) + Fx_0 * b_PPX2) * scaleFactors[4];
  lam_Cz_0 = b_PCX1 * scaleFactors[33];
  if (lam_Cz_0 < 0.0) {
    lam_Cz_0 = 0.0;
  }

  Fx_0 = b_gamma[1] * b_gamma[1];
  Vs_0 = ((b_PPX3 * dpi[1] + 1.0) + b_PPX4 * Vs_tmp) * (b_PDX2 * dfz[1] + b_PDX1) * (1.0 - Fx_0 * b_PDX3) * Vsy_0 *
         Fz[1] * zeta[10];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  mu_y_idx_2_tmp_0 = dfz[1] * dfz[1];
  mu_y_idx_2 = ((b_PEX2 * dfz[1] + b_PEX1) + mu_y_idx_2_tmp_0 * b_PEX3) *
               (1.0 - std::tanh(10.0 * kappa_x[1]) * b_PEX4) * scaleFactors[35];
  if (mu_y_idx_2 > 1.0) {
    mu_y_idx_2 = 1.0;
  }

  phi[1] = lam_Cz_0 * Vs_0;
  Bykappa[1] = lam_Cz_0;
  epsilon_gamma[1] = Vs_0;
  Eykappa[1] = mu_y_idx_2;
  phi_t[1] = (b_PKX2 * dfz[1] + b_PKX1) * Fz[1] * std::exp(b_PKX3 * dfz[1]) *
             ((b_PPX1 * dpi[1] + 1.0) + Vs_tmp * b_PPX2) * scaleFactors[31];
  lam_Cz_0 = b_PCX1 * scaleFactors[60];
  if (lam_Cz_0 < 0.0) {
    lam_Cz_0 = 0.0;
  }

  Vs_tmp = b_gamma[2] * b_gamma[2];
  Vs_0 = ((b_PPX3 * dpi[2] + 1.0) + b_PPX4 * Dx_tmp) * (b_PDX2 * dfz[2] + b_PDX1) * (1.0 - Vs_tmp * b_PDX3) * Fx_tmp *
         Fz[2] * zeta[19];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  mu_y_idx_2_tmp_1 = dfz[2] * dfz[2];
  mu_y_idx_2 = ((b_PEX2 * dfz[2] + b_PEX1) + mu_y_idx_2_tmp_1 * b_PEX3) *
               (1.0 - std::tanh(10.0 * kappa_x[2]) * b_PEX4) * scaleFactors[62];
  if (mu_y_idx_2 > 1.0) {
    mu_y_idx_2 = 1.0;
  }

  phi[2] = lam_Cz_0 * Vs_0;
  Bykappa[2] = lam_Cz_0;
  epsilon_gamma[2] = Vs_0;
  Eykappa[2] = mu_y_idx_2;
  phi_t[2] = (b_PKX2 * dfz[2] + b_PKX1) * Fz[2] * std::exp(b_PKX3 * dfz[2]) *
             ((b_PPX1 * dpi[2] + 1.0) + Dx_tmp * b_PPX2) * scaleFactors[58];
  lam_Cz_0 = b_PCX1 * scaleFactors[87];
  if (lam_Cz_0 < 0.0) {
    lam_Cz_0 = 0.0;
  }

  Dx_tmp = b_gamma_0 * b_gamma_0;
  Vs_0 = ((b_PPX3 * dpi[3] + 1.0) + b_PPX4 * Dx_tmp_0) * (b_PDX2 * dfz[3] + b_PDX1) * (1.0 - Dx_tmp * b_PDX3) *
         Fx_tmp_0 * Fz_0 * zeta[28];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  mu_y_idx_2_tmp_2 = dfz[3] * dfz[3];
  mu_y_idx_2 = ((b_PEX2 * dfz[3] + b_PEX1) + mu_y_idx_2_tmp_2 * b_PEX3) *
               (1.0 - std::tanh(10.0 * kappa_x[3]) * b_PEX4) * scaleFactors[89];
  if (mu_y_idx_2 > 1.0) {
    mu_y_idx_2 = 1.0;
  }

  Dx_tmp_0 = (b_PKX2 * dfz[3] + b_PKX1) * Fz_0 * std::exp(b_PKX3 * dfz[3]) *
             ((b_PPX1 * dpi[3] + 1.0) + Dx_tmp_0 * b_PPX2) * scaleFactors[85];
  phi[3] = lam_Cz_0 * Vs_0;
  Bykappa[3] = lam_Cz_0;
  epsilon_gamma[3] = Vs_0;
  Eykappa[3] = mu_y_idx_2;
  Chassis_div0protect(phi, 0.0001, Drphi, lam_Cz);
  mu_y_idx_1 = b_REX2 * dfz[0] + b_REX1;
  lam_Cz[0] = mu_y_idx_1;
  if (mu_y_idx_1 > 1.0) {
    lam_Cz[0] = 1.0;
  }

  Bxalpha_0 = (Bxalpha_0 * b_RBX3 + b_RBX1) * std::cos(std::atan(b_RBX2 * kappa[0])) * scaleFactors[18];
  if (Bxalpha_0 < 0.0) {
    Bxalpha_0 = 0.0;
  }

  Bxalpha[0] = Bxalpha_0;
  mu_y_idx_1 = b_REX2 * dfz[1] + b_REX1;
  lam_Cz[1] = mu_y_idx_1;
  if (mu_y_idx_1 > 1.0) {
    lam_Cz[1] = 1.0;
  }

  Bxalpha_0 = (mu_y_tmp * b_RBX3 + b_RBX1) * std::cos(std::atan(b_RBX2 * kappa[1])) * scaleFactors[45];
  if (Bxalpha_0 < 0.0) {
    Bxalpha_0 = 0.0;
  }

  Bxalpha[1] = Bxalpha_0;
  mu_y_idx_1 = b_REX2 * dfz[2] + b_REX1;
  lam_Cz[2] = mu_y_idx_1;
  if (mu_y_idx_1 > 1.0) {
    lam_Cz[2] = 1.0;
  }

  Bxalpha_0 = (Vs_1 * b_RBX3 + b_RBX1) * std::cos(std::atan(b_RBX2 * kappa[2])) * scaleFactors[72];
  if (Bxalpha_0 < 0.0) {
    Bxalpha_0 = 0.0;
  }

  Bxalpha[2] = Bxalpha_0;
  mu_y_idx_1 = b_REX2 * dfz[3] + b_REX1;
  lam_Cz[3] = mu_y_idx_1;
  if (mu_y_idx_1 > 1.0) {
    lam_Cz[3] = 1.0;
  }

  Bxalpha_0 = (mu_y_idx_0 * b_RBX3 + b_RBX1) * std::cos(std::atan(b_RBX2 * tmp_data)) * scaleFactors[99];
  if (Bxalpha_0 < 0.0) {
    Bxalpha_0 = 0.0;
  }

  Bxalpha[3] = Bxalpha_0;
  Chassis_magiccos(b_RCX1, Bxalpha, lam_Cz, alpha, Kyalpha);
  Chassis_magiccos_n(b_RCX1, Bxalpha, lam_Cz, rhoz_data);
  Vs_0 = Kyalpha[0] / rhoz_data[0];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  Mzphiinf[0] = phi_t[0] / Drphi[0];
  Vs[0] = Vs_0;
  Vs_0 = Kyalpha[1] / rhoz_data[1];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  Mzphiinf[1] = phi_t[1] / Drphi[1];
  Vs[1] = Vs_0;
  Vs_0 = Kyalpha[2] / rhoz_data[2];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  Mzphiinf[2] = phi_t[2] / Drphi[2];
  Vs[2] = Vs_0;
  Vs_0 = Kyalpha[3] / rhoz_data[3];
  if (Vs_0 < 0.0) {
    Vs_0 = 0.0;
  }

  Mzphiinf[3] = Dx_tmp_0 / Drphi[3];
  Chassis_magicsin_g(epsilon_gamma, Bykappa, Mzphiinf, Eykappa, kappa_x, Kyalpha);
  mu_y_tmp = b_PCY1 * scaleFactors[7];
  if (mu_y_tmp < 0.0) {
    mu_y_tmp = 0.0;
  }

  c_y_idx_1 *= Fz[0];
  Vs_1 = c_y_idx_1 * zeta[2];
  Mzphiinf[0] = mu_y_tmp * Vs_1;
  Fx[0] = ((b_PVX2 * dfz[0] + b_PVX1) * Fz[0] * (tb_data * 10.0 / (9.0 * tb_data + 1.0)) * scaleFactors[12] * zeta[1] +
           Kyalpha[0]) *
          Vs[0];
  Vs[0] = Vs_1;
  epsilon_gamma[0] = mu_y_tmp;
  mu_y_idx_0 = c_y_idx_1;
  mu_y_tmp = b_PCY1 * scaleFactors[34];
  if (mu_y_tmp < 0.0) {
    mu_y_tmp = 0.0;
  }

  c_y_idx_1 = Re_0 * Fz[1];
  Vs_1 = c_y_idx_1 * zeta[11];
  Mzphiinf[1] = mu_y_tmp * Vs_1;
  Fx[1] = ((b_PVX2 * dfz[1] + b_PVX1) * Fz[1] * (Vsy_0 * 10.0 / (9.0 * Vsy_0 + 1.0)) * scaleFactors[39] * zeta[10] +
           Kyalpha[1]) *
          Vs[1];
  Vs[1] = Vs_1;
  epsilon_gamma[1] = mu_y_tmp;
  mu_y_idx_1 = c_y_idx_1;
  mu_y_tmp = b_PCY1 * scaleFactors[61];
  if (mu_y_tmp < 0.0) {
    mu_y_tmp = 0.0;
  }

  c_y_idx_1 = mu_y * Fz[2];
  Vs_1 = c_y_idx_1 * zeta[20];
  Mzphiinf[2] = mu_y_tmp * Vs_1;
  Fx[2] = ((b_PVX2 * dfz[2] + b_PVX1) * Fz[2] * (Fx_tmp * 10.0 / (9.0 * Fx_tmp + 1.0)) * scaleFactors[66] * zeta[19] +
           Kyalpha[2]) *
          Vs[2];
  Vs[2] = Vs_1;
  epsilon_gamma[2] = mu_y_tmp;
  mu_y_idx_2 = c_y_idx_1;
  tb_data =
      ((b_PVX2 * dfz[3] + b_PVX1) * Fz_0 * (Fx_tmp_0 * 10.0 / (9.0 * Fx_tmp_0 + 1.0)) * scaleFactors[93] * zeta[28] +
       Kyalpha[3]) *
      Vs_0;
  mu_y_tmp = b_PCY1 * scaleFactors[88];
  if (mu_y_tmp < 0.0) {
    mu_y_tmp = 0.0;
  }

  c_y_idx_1 = lam_muy * Fz_0;
  Vs_1 = c_y_idx_1 * zeta[29];
  Mzphiinf[3] = mu_y_tmp * Vs_1;
  Fx[3] = tb_data;
  Vs[3] = Vs_1;
  epsilon_gamma[3] = mu_y_tmp;
  Chassis_div0protect(Mzphiinf, 0.0001, phi, lam_Cz);
  Fz_0 = Fz_tmp / phi[0];
  isLowSpeed_0[0] = !isLowSpeed[0];
  phi[0] = Fz_0;
  Fx_tmp = Fz_tmp_0 / phi[1];
  isLowSpeed_0[1] = !isLowSpeed[1];
  phi[1] = Fx_tmp;
  Fx_tmp_0 = Fz_tmp_1 / phi[2];
  isLowSpeed_0[2] = !isLowSpeed[2];
  phi[2] = Fx_tmp_0;
  Vs_1 = Fz_tmp_2 / phi[3];
  isLowSpeed_0[3] = !isLowSpeed[3];
  phi[3] = Vs_1;
  if (Chassis_any(isLowSpeed_0)) {
    b_i = 0;
    if (!isLowSpeed[0]) {
      b_i = 1;
    }

    if (!isLowSpeed[1]) {
      b_i++;
    }

    if (!isLowSpeed[2]) {
      b_i++;
    }

    if (!isLowSpeed[3]) {
      b_i++;
    }

    h_i = b_i;
    b_i = 0;
    if (!isLowSpeed[0]) {
      jb_data[0] = 1;
      b_i = 1;
    }

    if (!isLowSpeed[1]) {
      jb_data[b_i] = 2;
      b_i++;
    }

    if (!isLowSpeed[2]) {
      jb_data[b_i] = 3;
      b_i++;
    }

    if (!isLowSpeed[3]) {
      jb_data[b_i] = 4;
    }

    b_i = 0;
    if (!isLowSpeed[0]) {
      b_i = 1;
    }

    if (!isLowSpeed[1]) {
      b_i++;
    }

    if (!isLowSpeed[2]) {
      b_i++;
    }

    if (!isLowSpeed[3]) {
      b_i++;
    }

    tmp_size_1 = b_i;
    b_i = 0;
    if (!isLowSpeed[0]) {
      lb_data[0] = 1;
      b_i = 1;
    }

    if (!isLowSpeed[1]) {
      lb_data[b_i] = 2;
      b_i++;
    }

    if (!isLowSpeed[2]) {
      lb_data[b_i] = 3;
      b_i++;
    }

    if (!isLowSpeed[3]) {
      lb_data[b_i] = 4;
    }

    b_i = 0;
    if (!isLowSpeed[0]) {
      b_i = 1;
    }

    if (!isLowSpeed[1]) {
      b_i++;
    }

    if (!isLowSpeed[2]) {
      b_i++;
    }

    if (!isLowSpeed[3]) {
      b_i++;
    }

    o_size = b_i;
    b_i = 0;
    if (!isLowSpeed[0]) {
      k_data[0] = 0.0 * dfz[0] * scaleFactors[11];
      b_i = 1;
    }

    if (!isLowSpeed[1]) {
      k_data[b_i] = 0.0 * dfz[1] * scaleFactors[38];
      b_i++;
    }

    if (!isLowSpeed[2]) {
      k_data[b_i] = 0.0 * dfz[2] * scaleFactors[65];
      b_i++;
    }

    if (!isLowSpeed[3]) {
      k_data[b_i] = 0.0 * dfz[3] * scaleFactors[92];
    }

    b_i = 0;
    if (!isLowSpeed[0]) {
      b_i = 1;
    }

    if (!isLowSpeed[1]) {
      b_i++;
    }

    if (!isLowSpeed[2]) {
      b_i++;
    }

    if (!isLowSpeed[3]) {
      b_i++;
    }

    tmp_size_2 = b_i;
    b_i = 0;
    if (!isLowSpeed[0]) {
      mb_data[0] = 1;
      b_i = 1;
    }

    if (!isLowSpeed[1]) {
      mb_data[b_i] = 2;
      b_i++;
    }

    if (!isLowSpeed[2]) {
      mb_data[b_i] = 3;
      b_i++;
    }

    if (!isLowSpeed[3]) {
      mb_data[b_i] = 4;
    }

    b_i = 0;
    if (!isLowSpeed[0]) {
      b_i = 1;
    }

    if (!isLowSpeed[1]) {
      b_i++;
    }

    if (!isLowSpeed[2]) {
      b_i++;
    }

    if (!isLowSpeed[3]) {
      b_i++;
    }

    w_size = b_i;
    b_i = 0;
    if (!isLowSpeed[0]) {
      pb_data[0] = 1;
      b_i = 1;
    }

    if (!isLowSpeed[1]) {
      pb_data[b_i] = 2;
      b_i++;
    }

    if (!isLowSpeed[2]) {
      pb_data[b_i] = 3;
      b_i++;
    }

    if (!isLowSpeed[3]) {
      pb_data[b_i] = 4;
    }

    if ((h_i == tmp_size_1) && ((h_i == 1 ? tmp_size_1 : h_i) == tmp_size_1) &&
        (((h_i == 1 ? tmp_size_1 : h_i) == 1 ? tmp_size_1
          : h_i == 1                         ? tmp_size_1
                                             : h_i) == tmp_size_2) &&
        ((((h_i == 1 ? tmp_size_1 : h_i) == 1 ? tmp_size_1
           : h_i == 1                         ? tmp_size_1
                                              : h_i) == 1
              ? tmp_size_2
          : (h_i == 1 ? tmp_size_1 : h_i) == 1 ? tmp_size_1
          : h_i == 1                           ? tmp_size_1
                                               : h_i) == o_size) &&
        ((o_size == 1 ? ((h_i == 1 ? tmp_size_1 : h_i) == 1 ? tmp_size_1
                         : h_i == 1                         ? tmp_size_1
                                                            : h_i) == 1
                            ? tmp_size_2
                        : (h_i == 1 ? tmp_size_1 : h_i) == 1 ? tmp_size_1
                        : h_i == 1                           ? tmp_size_1
                                                             : h_i
                      : o_size) == w_size)) {
      for (b_i = 0; b_i < o_size; b_i++) {
        int8_T jb;
        s = lb_data[b_i];
        jb = jb_data[b_i];
        tmpDrphiVar_data[b_i] = (((dfz[jb - 1] * b_PKY7 + b_PKY6) * Fz[jb - 1] * (dpi[jb - 1] * b_PPY5 + 1.0) *
                                      scaleFactors[(jb - 1) * 27 + 14] * gamma_star[jb - 1] -
                                  SVygamma[s - 1]) /
                                     b_x[s - 1] * zeta[(mb_data[b_i] - 1) * 9] +
                                 k_data[b_i]) +
                                zeta[(pb_data[b_i] - 1) * 9 + 4];
      }
    } else {
      Chassis_binary_expand_op_b(tmpDrphiVar_data, &tmpDrphiVar_size, k_data, &o_size, Fz, b_PKY6, b_PKY7, dfz, b_PPY5,
                                 dpi, scaleFactors, jb_data, &h_i, gamma_star, SVygamma, lb_data, &tmp_size_1, b_x,
                                 zeta, mb_data, &tmp_size_2, pb_data, &w_size);
    }

    b_i = 0;
    if (!isLowSpeed[0]) {
      SHy_idx_0 = tmpDrphiVar_data[0] - 1.0;
      b_i = 1;
    }

    if (!isLowSpeed[1]) {
      SHy_idx_1 = tmpDrphiVar_data[b_i] - 1.0;
      b_i++;
    }

    if (!isLowSpeed[2]) {
      SHy_idx_2 = tmpDrphiVar_data[b_i] - 1.0;
      b_i++;
    }

    if (!isLowSpeed[3]) {
      SHy_idx_3 = tmpDrphiVar_data[b_i] - 1.0;
    }
  }

  lam_Cz_0 = alpha[0] + SHy_idx_0;
  if (std::isnan(lam_Cz_0)) {
    Vs_0 = (rtNaN);
  } else if (lam_Cz_0 < 0.0) {
    Vs_0 = -1.0;
  } else {
    Vs_0 = (lam_Cz_0 > 0.0);
  }

  if (Vs_0 == 0.0) {
    Vs_0 = 1.0;
  }

  Vsy_0 = gamma_star[0] * gamma_star[0];
  Bxalpha_0 = ((Vsy_0 * b_PEY5 + 1.0) - b_PEY4 * gamma_star[0] * Vs_0) * (b_PEY2 * dfz[0] + b_PEY1) * scaleFactors[9];
  if (Bxalpha_0 > 1.0) {
    Bxalpha_0 = 1.0;
  }

  Re_0 = (0.0 * dfz[0] + b_RVY3 * gamma_star[0]) * mu_y_idx_0 * std::cos(std::atan(b_RVY4 * alpha[0])) * zeta[2] *
         std::sin(std::atan(b_RVY6 * kappa[0]) * b_RVY5) * scaleFactors[20];
  lam_muy_prime_idx_0 = 0.0 * dfz[0] * Fz[0] * scaleFactors[13] * lam_muy_prime_idx_0 * zeta[2] + SVygamma[0];
  lam_Cz[0] = lam_Cz_0;
  Bxalpha[0] = Bxalpha_0;
  lam_Cz_0 = alpha[1] + SHy_idx_1;
  if (std::isnan(lam_Cz_0)) {
    Vs_0 = (rtNaN);
  } else if (lam_Cz_0 < 0.0) {
    Vs_0 = -1.0;
  } else {
    Vs_0 = (lam_Cz_0 > 0.0);
  }

  if (Vs_0 == 0.0) {
    Vs_0 = 1.0;
  }

  Bxalpha_tmp = gamma_star[1] * gamma_star[1];
  Bxalpha_0 =
      ((Bxalpha_tmp * b_PEY5 + 1.0) - b_PEY4 * gamma_star[1] * Vs_0) * (b_PEY2 * dfz[1] + b_PEY1) * scaleFactors[36];
  if (Bxalpha_0 > 1.0) {
    Bxalpha_0 = 1.0;
  }

  mu_y = (0.0 * dfz[1] + b_RVY3 * gamma_star[1]) * mu_y_idx_1 * std::cos(std::atan(b_RVY4 * alpha[1])) * zeta[11] *
         std::sin(std::atan(b_RVY6 * kappa[1]) * b_RVY5) * scaleFactors[47];
  lam_muy_prime_idx_1 = 0.0 * dfz[1] * Fz[1] * scaleFactors[40] * lam_muy_prime_idx_1 * zeta[11] + SVygamma[1];
  lam_Cz[1] = lam_Cz_0;
  Bxalpha[1] = Bxalpha_0;
  lam_Cz_0 = alpha[2] + SHy_idx_2;
  if (std::isnan(lam_Cz_0)) {
    Vs_0 = (rtNaN);
  } else if (lam_Cz_0 < 0.0) {
    Vs_0 = -1.0;
  } else {
    Vs_0 = (lam_Cz_0 > 0.0);
  }

  if (Vs_0 == 0.0) {
    Vs_0 = 1.0;
  }

  b_idx_0 = gamma_star[2] * gamma_star[2];
  Bxalpha_0 =
      ((b_idx_0 * b_PEY5 + 1.0) - b_PEY4 * gamma_star[2] * Vs_0) * (b_PEY2 * dfz[2] + b_PEY1) * scaleFactors[63];
  if (Bxalpha_0 > 1.0) {
    Bxalpha_0 = 1.0;
  }

  lam_muy = (0.0 * dfz[2] + b_RVY3 * gamma_star[2]) * mu_y_idx_2 * std::cos(std::atan(b_RVY4 * alpha[2])) * zeta[20] *
            std::sin(std::atan(b_RVY6 * kappa[2]) * b_RVY5) * scaleFactors[74];
  lam_muy_prime_idx_2 = 0.0 * dfz[2] * Fz[2] * scaleFactors[67] * lam_muy_prime_idx_2 * zeta[20] + SVygamma[2];
  lam_Cz[2] = lam_Cz_0;
  Bxalpha[2] = Bxalpha_0;
  lam_muy_prime = 0.0 * dfz[3] * Fz[3] * scaleFactors[94] * lam_muy_prime_idx_3 * zeta[29] + SVygamma[3];
  lam_Cz_0 = e_y + SHy_idx_3;
  if (std::isnan(lam_Cz_0)) {
    Vs_0 = (rtNaN);
  } else if (lam_Cz_0 < 0.0) {
    Vs_0 = -1.0;
  } else {
    Vs_0 = (lam_Cz_0 > 0.0);
  }

  if (Vs_0 == 0.0) {
    Vs_0 = 1.0;
  }

  mu_y_idx_2 = gamma_star[3] * gamma_star[3];
  Bxalpha_0 =
      ((mu_y_idx_2 * b_PEY5 + 1.0) - b_PEY4 * gamma_star[3] * Vs_0) * (b_PEY2 * dfz[3] + b_PEY1) * scaleFactors[90];
  if (Bxalpha_0 > 1.0) {
    Bxalpha_0 = 1.0;
  }

  c_y_idx_1 = (0.0 * dfz[3] + b_RVY3 * gamma_star[3]) * c_y_idx_1 * std::cos(std::atan(b_RVY4 * e_y)) * zeta[29] *
              std::sin(std::atan(b_RVY6 * tmp_data) * b_RVY5) * scaleFactors[101];
  lam_Cz[3] = lam_Cz_0;
  Bxalpha[3] = Bxalpha_0;
  Chassis_magicsin_g(Vs, epsilon_gamma, phi, Bxalpha, lam_Cz, Kyalpha);
  Bxalpha_0 = (Kyalpha[0] + lam_muy_prime_idx_0) * Fy[0] + Re_0;
  Mzphiinf[0] = scaleFactors[25];
  Fy[0] = Bxalpha_0;
  My[0] = std::tanh(10.0 * Omega[0]);
  mu_y_idx_0 = (Kyalpha[1] + lam_muy_prime_idx_1) * Fy[1] + mu_y;
  Mzphiinf[1] = scaleFactors[52];
  Fy[1] = mu_y_idx_0;
  My[1] = std::tanh(10.0 * Omega[1]);
  lam_Cz_tmp_0 = (Kyalpha[2] + lam_muy_prime_idx_2) * Fy[2] + lam_muy;
  Mzphiinf[2] = scaleFactors[79];
  Fy[2] = lam_Cz_tmp_0;
  My[2] = std::tanh(10.0 * Omega[2]);
  lam_Cz_tmp = (Kyalpha[3] + lam_muy_prime) * Fy[3] + c_y_idx_1;
  Mzphiinf[3] = scaleFactors[106];
  Fy[3] = lam_Cz_tmp;
  My[3] = std::tanh(10.0 * Omega[3]);
  Chassis_rollingMoment(Fx, Vx, FzUnSat, press, b_gamma, b_LONGVL, b_UNLOADED_RADIUS, b_FNOMIN, b_NOMPRES, b_QSY1,
                        b_QSY2, b_QSY3, b_QSY4, b_QSY5, b_QSY6, b_QSY7, b_QSY8, Mzphiinf, Kyalpha);
  lam_muy_prime_idx_3 = b_QSX6 * Fz[0] / b_FNOMIN;
  mu_y_idx_1 = b_UNLOADED_RADIUS * Fz[0];
  lam_Cz_0 = std::abs(b_gamma[0]);
  Mx[0] = (((((0.0 * scaleFactors[24] - (b_PPMX1 * dpi[0] + 1.0) * (b_QSX2 * b_gamma[0])) -
              b_QSX12 * b_gamma[0] * lam_Cz_0) +
             b_QSX3 * Bxalpha_0 / b_FNOMIN) +
            std::sin(std::atan(b_QSX9 * Bxalpha_0 / b_FNOMIN) * b_QSX8 + b_QSX7 * b_gamma[0]) *
                (std::cos(std::atan(lam_muy_prime_idx_3 * lam_muy_prime_idx_3) * b_QSX5) * b_QSX4)) +
           std::atan(b_QSX11 * Fz[0] / b_FNOMIN) * b_QSX10 * b_gamma[0]) *
              (mu_y_idx_1 * scaleFactors[23]) +
          b_UNLOADED_RADIUS * Bxalpha_0 * scaleFactors[23] * (b_QSX14 * lam_Cz_0 + b_QSX13);
  Drphi[0] = ((b_QHZ4 * dfz[0] + b_QHZ3) * gamma_star[0] + 0.0 * dfz[0]) + alpha[0];
  Vs_0 = 0.0 * b_gamma[0] + 1.0;
  Vsy_0 = ((b_QBZ2 * dfz[0] + b_QBZ1) + mu_y_idx_2_tmp * b_QBZ3) * ((b_QBZ5 * lam_Cz_0 + Vs_0) + Vsy_0 * b_QBZ6) *
          scaleFactors[5] / lam_muy_idx_0;
  if (Vsy_0 < 0.0) {
    Vsy_0 = 0.0;
  }

  My[0] *= Kyalpha[0];
  Bykappa[0] = mu_y_idx_1;
  Mz[0] = Vx[0] / Mz[0];
  SHy_idx_0 = (lam_muy_prime_idx_0 / b_x[0] + SHy_idx_0) + alpha[0];
  Vsy[0] = Vsy_0;
  Vs[0] = Vs_0;
  lam_muy_prime_idx_3 = b_QSX6 * Fz[1] / b_FNOMIN;
  mu_y_idx_1 = b_UNLOADED_RADIUS * Fz[1];
  lam_Cz_0 = std::abs(b_gamma[1]);
  Mx[1] = (((((0.0 * scaleFactors[51] - (b_PPMX1 * dpi[1] + 1.0) * (b_QSX2 * b_gamma[1])) -
              b_QSX12 * b_gamma[1] * lam_Cz_0) +
             b_QSX3 * mu_y_idx_0 / b_FNOMIN) +
            std::sin(std::atan(b_QSX9 * mu_y_idx_0 / b_FNOMIN) * b_QSX8 + b_QSX7 * b_gamma[1]) *
                (std::cos(std::atan(lam_muy_prime_idx_3 * lam_muy_prime_idx_3) * b_QSX5) * b_QSX4)) +
           std::atan(b_QSX11 * Fz[1] / b_FNOMIN) * b_QSX10 * b_gamma[1]) *
              (mu_y_idx_1 * scaleFactors[50]) +
          b_UNLOADED_RADIUS * mu_y_idx_0 * scaleFactors[50] * (b_QSX14 * lam_Cz_0 + b_QSX13);
  Drphi[1] = ((b_QHZ4 * dfz[1] + b_QHZ3) * gamma_star[1] + 0.0 * dfz[1]) + alpha[1];
  Vs_0 = 0.0 * b_gamma[1] + 1.0;
  Vsy_0 = ((b_QBZ2 * dfz[1] + b_QBZ1) + mu_y_idx_2_tmp_0 * b_QBZ3) *
          ((b_QBZ5 * lam_Cz_0 + Vs_0) + Bxalpha_tmp * b_QBZ6) * scaleFactors[32] / lam_muy_idx_1;
  if (Vsy_0 < 0.0) {
    Vsy_0 = 0.0;
  }

  My[1] *= Kyalpha[1];
  Bykappa[1] = mu_y_idx_1;
  Mz[1] = Vx[1] / Mz[1];
  SHy_idx_1 = (lam_muy_prime_idx_1 / b_x[1] + SHy_idx_1) + alpha[1];
  Vsy[1] = Vsy_0;
  Vs[1] = Vs_0;
  lam_muy_prime_idx_3 = b_QSX6 * Fz[2] / b_FNOMIN;
  mu_y_idx_1 = b_UNLOADED_RADIUS * Fz[2];
  lam_Cz_0 = std::abs(b_gamma[2]);
  Mx[2] = (((((0.0 * scaleFactors[78] - (b_PPMX1 * dpi[2] + 1.0) * (b_QSX2 * b_gamma[2])) -
              b_QSX12 * b_gamma[2] * lam_Cz_0) +
             b_QSX3 * lam_Cz_tmp_0 / b_FNOMIN) +
            std::sin(std::atan(b_QSX9 * lam_Cz_tmp_0 / b_FNOMIN) * b_QSX8 + b_QSX7 * b_gamma[2]) *
                (std::cos(std::atan(lam_muy_prime_idx_3 * lam_muy_prime_idx_3) * b_QSX5) * b_QSX4)) +
           std::atan(b_QSX11 * Fz[2] / b_FNOMIN) * b_QSX10 * b_gamma[2]) *
              (mu_y_idx_1 * scaleFactors[77]) +
          b_UNLOADED_RADIUS * lam_Cz_tmp_0 * scaleFactors[77] * (b_QSX14 * lam_Cz_0 + b_QSX13);
  Drphi[2] = ((b_QHZ4 * dfz[2] + b_QHZ3) * gamma_star[2] + 0.0 * dfz[2]) + alpha[2];
  Vs_0 = 0.0 * b_gamma[2] + 1.0;
  Vsy_0 = ((b_QBZ2 * dfz[2] + b_QBZ1) + mu_y_idx_2_tmp_1 * b_QBZ3) * ((b_QBZ5 * lam_Cz_0 + Vs_0) + b_idx_0 * b_QBZ6) *
          scaleFactors[59] / lam_muy_idx_2;
  if (Vsy_0 < 0.0) {
    Vsy_0 = 0.0;
  }

  My[2] *= Kyalpha[2];
  Bykappa[2] = mu_y_idx_1;
  Mz[2] = Vx[2] / Mz[2];
  SHy_idx_2 = (lam_muy_prime_idx_2 / b_x[2] + SHy_idx_2) + alpha[2];
  Vsy[2] = Vsy_0;
  Vs[2] = Vs_0;
  lam_muy_prime_idx_3 = b_QSX6 * Fz[3] / b_FNOMIN;
  mu_y_idx_1 = b_UNLOADED_RADIUS * Fz[3];
  lam_Cz_0 = std::abs(b_gamma_0);
  Mx[3] = (((((0.0 * scaleFactors[105] - (b_PPMX1 * dpi[3] + 1.0) * (b_QSX2 * b_gamma_0)) -
              b_QSX12 * b_gamma_0 * lam_Cz_0) +
             b_QSX3 * lam_Cz_tmp / b_FNOMIN) +
            std::sin(std::atan(b_QSX9 * lam_Cz_tmp / b_FNOMIN) * b_QSX8 + b_QSX7 * b_gamma_0) *
                (std::cos(std::atan(lam_muy_prime_idx_3 * lam_muy_prime_idx_3) * b_QSX5) * b_QSX4)) +
           std::atan(b_QSX11 * Fz[3] / b_FNOMIN) * b_QSX10 * b_gamma_0) *
              (mu_y_idx_1 * scaleFactors[104]) +
          b_UNLOADED_RADIUS * lam_Cz_tmp * scaleFactors[104] * (b_QSX14 * lam_Cz_0 + b_QSX13);
  lam_muy_prime_idx_2 = Vx[3] / Mz[3];
  Drphi[3] = ((b_QHZ4 * dfz[3] + b_QHZ3) * gamma_star[3] + 0.0 * dfz[3]) + e_y;
  lam_muy_prime_idx_3 = (lam_muy_prime / b_x[3] + SHy_idx_3) + e_y;
  Vs_0 = 0.0 * b_gamma_0 + 1.0;
  Vsy_0 = ((b_QBZ2 * dfz[3] + b_QBZ1) + mu_y_idx_2_tmp_2 * b_QBZ3) *
          ((b_QBZ5 * lam_Cz_0 + Vs_0) + mu_y_idx_2 * b_QBZ6) * scaleFactors[86] / Vs_2;
  if (Vsy_0 < 0.0) {
    Vsy_0 = 0.0;
  }

  My[3] *= Kyalpha[3];
  Vsy[3] = Vsy_0;
  b_idx_0 = b_QCZ1;
  if (b_QCZ1 < 0.0) {
    b_idx_0 = 0.0;
  }

  mu_y_idx_2 = (b_QEZ5 * gamma_star[0] * 2.0 / 3.1415926535897931 * std::atan(Vsy[0] * b_idx_0 * Drphi[0]) + 1.0) *
               ((b_QEZ2 * dfz[0] + b_QEZ1) + mu_y_idx_2_tmp * b_QEZ3);
  if (mu_y_idx_2 > 1.0) {
    mu_y_idx_2 = 1.0;
  }

  SHy_idx_3 = std::tan(SHy_idx_0);
  e_y = phi_t[0] / b_x[0];
  Bxalpha_tmp = e_y * e_y * (kappa[0] * kappa[0]);
  e_y = std::sqrt(SHy_idx_3 * SHy_idx_3 + Bxalpha_tmp);
  SHy_idx_3 = std::tan(Drphi[0]);
  SVygamma[0] = (b_QDZ2 * dfz[0] + b_QDZ1) * (1.0 - b_PPZ1 * dpi[0]) * (dfz_0 * b_QDZ4 + Vs[0]) * Fz[0] *
                (b_UNLOADED_RADIUS / Fzo_prime_idx_0) * scaleFactors[16] * zeta[5];
  Mzphiinf[0] = std::atan(std::sqrt(SHy_idx_3 * SHy_idx_3 + Bxalpha_tmp)) * std::tanh(1000.0 * Drphi[0]);
  phi[0] = ((((b_QDZ9 * dfz[0] + b_QDZ8) * (b_PPZ2 * dpi[0] + 1.0) +
              (b_QDZ11 * dfz[0] + b_QDZ10) * std::abs(gamma_star[0])) *
                 gamma_star[0] * scaleFactors[15] * zeta[0] +
             0.0 * dfz[0] * scaleFactors[17] * zeta[2]) *
                Bykappa[0] * lam_muy_idx_0 * std::tanh(10.0 * Vx[0]) * Mz[0] +
            zeta[8]) -
           1.0;
  FzUnSat[0] = zeta[7];
  kappa_x[0] = (b_QBZ9 * scaleFactors[5] / lam_muy_idx_0 + b_QBZ10 * Fz_0 * epsilon_gamma[0]) * zeta[6];
  lam_Cz[0] = std::tanh(1000.0 * SHy_idx_0) * std::atan(e_y);
  Eykappa[0] = mu_y_idx_2;
  mu_y_idx_2 = (b_QEZ5 * gamma_star[1] * 2.0 / 3.1415926535897931 * std::atan(b_idx_0 * Vsy[1] * Drphi[1]) + 1.0) *
               ((b_QEZ2 * dfz[1] + b_QEZ1) + mu_y_idx_2_tmp_0 * b_QEZ3);
  if (mu_y_idx_2 > 1.0) {
    mu_y_idx_2 = 1.0;
  }

  SHy_idx_3 = std::tan(SHy_idx_1);
  e_y = phi_t[1] / b_x[1];
  Bxalpha_tmp = e_y * e_y * (kappa[1] * kappa[1]);
  e_y = std::sqrt(SHy_idx_3 * SHy_idx_3 + Bxalpha_tmp);
  SHy_idx_3 = std::tan(Drphi[1]);
  SVygamma[1] = (b_QDZ2 * dfz[1] + b_QDZ1) * (1.0 - b_PPZ1 * dpi[1]) * (Fx_0 * b_QDZ4 + Vs[1]) * Fz[1] *
                (b_UNLOADED_RADIUS / Fzo_prime_idx_1) * scaleFactors[43] * zeta[14];
  Mzphiinf[1] = std::atan(std::sqrt(SHy_idx_3 * SHy_idx_3 + Bxalpha_tmp)) * std::tanh(1000.0 * Drphi[1]);
  phi[1] = ((((b_QDZ9 * dfz[1] + b_QDZ8) * (b_PPZ2 * dpi[1] + 1.0) +
              (b_QDZ11 * dfz[1] + b_QDZ10) * std::abs(gamma_star[1])) *
                 gamma_star[1] * scaleFactors[42] * zeta[9] +
             0.0 * dfz[1] * scaleFactors[44] * zeta[11]) *
                Bykappa[1] * lam_muy_idx_1 * std::tanh(10.0 * Vx[1]) * Mz[1] +
            zeta[17]) -
           1.0;
  FzUnSat[1] = zeta[16];
  kappa_x[1] = (b_QBZ9 * scaleFactors[32] / lam_muy_idx_1 + b_QBZ10 * Fx_tmp * epsilon_gamma[1]) * zeta[15];
  lam_Cz[1] = std::tanh(1000.0 * SHy_idx_1) * std::atan(e_y);
  Eykappa[1] = mu_y_idx_2;
  mu_y_idx_2 = (b_QEZ5 * gamma_star[2] * 2.0 / 3.1415926535897931 * std::atan(b_idx_0 * Vsy[2] * Drphi[2]) + 1.0) *
               ((b_QEZ2 * dfz[2] + b_QEZ1) + mu_y_idx_2_tmp_1 * b_QEZ3);
  if (mu_y_idx_2 > 1.0) {
    mu_y_idx_2 = 1.0;
  }

  SHy_idx_3 = std::tan(SHy_idx_2);
  e_y = phi_t[2] / b_x[2];
  Bxalpha_tmp = e_y * e_y * (kappa[2] * kappa[2]);
  e_y = std::sqrt(SHy_idx_3 * SHy_idx_3 + Bxalpha_tmp);
  SHy_idx_3 = std::tan(Drphi[2]);
  SVygamma[2] = (b_QDZ2 * dfz[2] + b_QDZ1) * (1.0 - b_PPZ1 * dpi[2]) * (Vs_tmp * b_QDZ4 + Vs[2]) * Fz[2] *
                (b_UNLOADED_RADIUS / Fzo_prime_idx_2) * scaleFactors[70] * zeta[23];
  Mzphiinf[2] = std::atan(std::sqrt(SHy_idx_3 * SHy_idx_3 + Bxalpha_tmp)) * std::tanh(1000.0 * Drphi[2]);
  phi[2] = ((((b_QDZ9 * dfz[2] + b_QDZ8) * (b_PPZ2 * dpi[2] + 1.0) +
              (b_QDZ11 * dfz[2] + b_QDZ10) * std::abs(gamma_star[2])) *
                 gamma_star[2] * scaleFactors[69] * zeta[18] +
             0.0 * dfz[2] * scaleFactors[71] * zeta[20]) *
                Bykappa[2] * lam_muy_idx_2 * std::tanh(10.0 * Vx[2]) * Mz[2] +
            zeta[26]) -
           1.0;
  FzUnSat[2] = zeta[25];
  kappa_x[2] = (b_QBZ9 * scaleFactors[59] / lam_muy_idx_2 + b_QBZ10 * Fx_tmp_0 * epsilon_gamma[2]) * zeta[24];
  lam_Cz[2] = std::tanh(1000.0 * SHy_idx_2) * std::atan(e_y);
  Eykappa[2] = mu_y_idx_2;
  mu_y_idx_2 = (b_QEZ5 * gamma_star[3] * 2.0 / 3.1415926535897931 * std::atan(b_idx_0 * Vsy_0 * Drphi[3]) + 1.0) *
               ((b_QEZ2 * dfz[3] + b_QEZ1) + mu_y_idx_2_tmp_2 * b_QEZ3);
  if (mu_y_idx_2 > 1.0) {
    mu_y_idx_2 = 1.0;
  }

  SHy_idx_3 = std::tan(lam_muy_prime_idx_3);
  e_y = Dx_tmp_0 / b_x[3];
  Bxalpha_tmp = e_y * e_y * (tmp_data * tmp_data);
  e_y = std::sqrt(SHy_idx_3 * SHy_idx_3 + Bxalpha_tmp);
  SHy_idx_3 = std::tan(Drphi[3]);
  SVygamma[3] = (b_QDZ2 * dfz[3] + b_QDZ1) * (1.0 - b_PPZ1 * dpi[3]) * (Dx_tmp * b_QDZ4 + Vs_0) * Fz[3] *
                (b_UNLOADED_RADIUS / press_0) * scaleFactors[97] * zeta[32];
  Mzphiinf[3] = std::atan(std::sqrt(SHy_idx_3 * SHy_idx_3 + Bxalpha_tmp)) * std::tanh(1000.0 * Drphi[3]);
  phi[3] = ((((b_QDZ9 * dfz[3] + b_QDZ8) * (b_PPZ2 * dpi[3] + 1.0) +
              (b_QDZ11 * dfz[3] + b_QDZ10) * std::abs(gamma_star[3])) *
                 gamma_star[3] * scaleFactors[96] * zeta[27] +
             0.0 * dfz[3] * scaleFactors[98] * zeta[29]) *
                mu_y_idx_1 * Vs_2 * std::tanh(10.0 * Vx[3]) * lam_muy_prime_idx_2 +
            zeta[35]) -
           1.0;
  FzUnSat[3] = zeta[34];
  kappa_x[3] = (b_QBZ9 * scaleFactors[86] / Vs_2 + b_QBZ10 * Vs_1 * mu_y_tmp) * zeta[33];
  lam_Cz[3] = std::tanh(1000.0 * lam_muy_prime_idx_3) * std::atan(e_y);
  Eykappa[3] = mu_y_idx_2;
  Chassis_magiccos_ngy(SVygamma, b_idx_0, Vsy, Eykappa, Mzphiinf, Kyalpha);
  Chassis_magiccos_ng(phi, FzUnSat, kappa_x, lam_Cz, rhoz_data);
  sig_x[0] = std::abs(phi_t[0] / (((b_PCFX1 * dfz[0] + 1.0) + mu_y_idx_2_tmp * b_PCFX2) * b_LONGITUDINAL_STIFFNESS *
                                  (b_PCFX3 * dpi[0] + 1.0)));
  sig_y[0] = std::abs(Fz_tmp / (((b_PCFY1 * dfz[0] + 1.0) + mu_y_idx_2_tmp * b_PCFY2) * b_LATERAL_STIFFNESS *
                                (b_PCFY3 * dpi[0] + 1.0)));
  Mz[0] = ((b_SSZ4 * dfz[0] + b_SSZ3) * gamma_star[0] + b_SSZ2 * Bxalpha_0 / b_FNOMIN) * b_UNLOADED_RADIUS *
              scaleFactors[21] * Fx[0] +
          (-(Kyalpha[0] * Mz[0] * scaleFactors[0]) * (Bxalpha_0 - Re_0) + rhoz_data[0]);
  sig_x[1] = std::abs(phi_t[1] / (((b_PCFX1 * dfz[1] + 1.0) + mu_y_idx_2_tmp_0 * b_PCFX2) * b_LONGITUDINAL_STIFFNESS *
                                  (b_PCFX3 * dpi[1] + 1.0)));
  sig_y[1] = std::abs(Fz_tmp_0 / (((b_PCFY1 * dfz[1] + 1.0) + mu_y_idx_2_tmp_0 * b_PCFY2) * b_LATERAL_STIFFNESS *
                                  (b_PCFY3 * dpi[1] + 1.0)));
  Mz[1] = ((b_SSZ4 * dfz[1] + b_SSZ3) * gamma_star[1] + b_SSZ2 * mu_y_idx_0 / b_FNOMIN) * b_UNLOADED_RADIUS *
              scaleFactors[48] * Fx[1] +
          (-(Kyalpha[1] * Mz[1] * scaleFactors[27]) * (mu_y_idx_0 - mu_y) + rhoz_data[1]);
  sig_x[2] = std::abs(phi_t[2] / (((b_PCFX1 * dfz[2] + 1.0) + mu_y_idx_2_tmp_1 * b_PCFX2) * b_LONGITUDINAL_STIFFNESS *
                                  (b_PCFX3 * dpi[2] + 1.0)));
  sig_y[2] = std::abs(Fz_tmp_1 / (((b_PCFY1 * dfz[2] + 1.0) + mu_y_idx_2_tmp_1 * b_PCFY2) * b_LATERAL_STIFFNESS *
                                  (b_PCFY3 * dpi[2] + 1.0)));
  Mz[2] = ((b_SSZ4 * dfz[2] + b_SSZ3) * gamma_star[2] + b_SSZ2 * lam_Cz_tmp_0 / b_FNOMIN) * b_UNLOADED_RADIUS *
              scaleFactors[75] * Fx[2] +
          (-(Kyalpha[2] * Mz[2] * scaleFactors[54]) * (lam_Cz_tmp_0 - lam_muy) + rhoz_data[2]);
  sig_x[3] = std::abs(Dx_tmp_0 / (((b_PCFX1 * dfz[3] + 1.0) + mu_y_idx_2_tmp_2 * b_PCFX2) * b_LONGITUDINAL_STIFFNESS *
                                  (b_PCFX3 * dpi[3] + 1.0)));
  sig_y[3] = std::abs(Fz_tmp_2 / (((b_PCFY1 * dfz[3] + 1.0) + mu_y_idx_2_tmp_2 * b_PCFY2) * b_LATERAL_STIFFNESS *
                                  (b_PCFY3 * dpi[3] + 1.0)));
  Mz[3] = ((b_SSZ4 * dfz[3] + b_SSZ3) * gamma_star[3] + b_SSZ2 * lam_Cz_tmp / b_FNOMIN) * b_UNLOADED_RADIUS *
              scaleFactors[102] * tb_data +
          (-(Kyalpha[3] * lam_muy_prime_idx_2 * scaleFactors[81]) * (lam_Cz_tmp - c_y_idx_1) + rhoz_data[3]);
}

void rt_mrdivide_U1d1x3_U2d3x3_Yd1x3_snf(const real_T u0[3], const real_T u1[9], real_T y[3]) {
  real_T A[9];
  real_T a21;
  real_T maxval;
  int32_T r1;
  int32_T r2;
  int32_T r3;
  std::memcpy(&A[0], &u1[0], 9U * sizeof(real_T));
  r1 = 0;
  r2 = 1;
  r3 = 2;
  maxval = std::abs(u1[0]);
  a21 = std::abs(u1[1]);
  if (a21 > maxval) {
    maxval = a21;
    r1 = 1;
    r2 = 0;
  }

  if (std::abs(u1[2]) > maxval) {
    r1 = 2;
    r2 = 1;
    r3 = 0;
  }

  A[r2] = u1[r2] / u1[r1];
  A[r3] /= A[r1];
  A[r2 + 3] -= A[r1 + 3] * A[r2];
  A[r3 + 3] -= A[r1 + 3] * A[r3];
  A[r2 + 6] -= A[r1 + 6] * A[r2];
  A[r3 + 6] -= A[r1 + 6] * A[r3];
  if (std::abs(A[r3 + 3]) > std::abs(A[r2 + 3])) {
    int32_T rtemp;
    rtemp = r2 + 1;
    r2 = r3;
    r3 = rtemp - 1;
  }

  A[r3 + 3] /= A[r2 + 3];
  A[r3 + 6] -= A[r3 + 3] * A[r2 + 6];
  y[r1] = u0[0] / A[r1];
  y[r2] = u0[1] - A[r1 + 3] * y[r1];
  y[r3] = u0[2] - A[r1 + 6] * y[r1];
  y[r2] /= A[r2 + 3];
  y[r3] -= A[r2 + 6] * y[r2];
  y[r3] /= A[r3 + 6];
  y[r2] -= A[r3 + 3] * y[r3];
  y[r1] -= y[r3] * A[r3];
  y[r1] -= y[r2] * A[r2];
}

/* Model step function */
void Chassis::step() {
  /* local scratch DWork variables */
  int32_T ForEach_itr;
  int32_T ForEach_itr_o;
  int32_T ForEach_itr_k;
  int32_T ForEach_itr_l;
  int32_T ForEach_itr_n;
  static const int8_T e[9]{1, -1, -1, -1, 1, -1, -1, -1, 1};

  static const int8_T f_b[9]{1, 0, 0, 0, 1, 0, 0, 0, 1};

  static const int8_T c[3]{-1, 1, -1};

  __m128d tmp_2;
  __m128d tmp_3;
  __m128d tmp_4;
  __m128d tmp_5;
  __m128d tmp_6;
  real_T rtb_VectorConcatenate_d[108];
  real_T Imat[72];
  real_T Itemp[72];
  real_T zeta[36];
  real_T rtb_ImpAsg_InsertedFor_Info_at_inport_0[32];
  real_T R[24];
  real_T d_b[24];
  real_T b_b[21];
  real_T rtb_MatrixConcatenation[18];
  real_T rtb_AngVel_g[12];
  real_T rtb_ImpAsg_InsertedFor_FsB_at_inport_0[12];
  real_T rtb_ImpAsg_InsertedFor_VehM_at_inport_0[12];
  real_T rtb_ImpAsg_InsertedFor_WhlAng_at_inport_0[12];
  real_T rtb_VectorConcatenate1_g[12];
  real_T rtb_VectorConcatenate2[12];
  real_T rtb_VectorConcatenate8[12];
  real_T rtb_Ibar[9];
  real_T rtb_Transpose_n[9];
  real_T rtb_VectorConcatenate[9];
  real_T rtb_VectorConcatenate_b_0[9];
  real_T rtb_VectorConcatenate_i[9];
  real_T rtb_MatrixConcatenate2[8];
  real_T rtb_MatrixConcatenate3[8];
  real_T rtb_MatrixConcatenate4[8];
  real_T rtb_Abs1[6];
  real_T rtb_ElementProduct[6];
  real_T tmp[6];
  real_T tmp_0[6];
  real_T Drphi[4];
  real_T Dx[4];
  real_T FzUnSat[4];
  real_T Gykappa[4];
  real_T Kyalpha[4];
  real_T Mzphiinf[4];
  real_T SVygamma[4];
  real_T Vc[4];
  real_T Vs[4];
  real_T b_x[4];
  real_T cosprimealpha[4];
  real_T gamma_star[4];
  real_T k_data[4];
  real_T lam_Cz[4];
  real_T lam_Hx[4];
  real_T lam_muV[4];
  real_T lam_mux[4];
  real_T lam_ykappa[4];
  real_T phi_t[4];
  real_T rtb_Add_j[4];
  real_T rtb_Alpha[4];
  real_T rtb_ImpAsg_InsertedFor_Fx_at_inport_0[4];
  real_T rtb_ImpAsg_InsertedFor_Fy_at_inport_0[4];
  real_T rtb_ImpAsg_InsertedFor_Mx_at_inport_0[4];
  real_T rtb_ImpAsg_InsertedFor_My_at_inport_0[4];
  real_T rtb_ImpAsg_InsertedFor_Mz_at_inport_0[4];
  real_T rtb_ImpAsg_InsertedFor_VehFz_at_inport_0[4];
  real_T rtb_ImpAsg_InsertedFor_WhlFz_at_inport_0[4];
  real_T rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[4];
  real_T rtb_ImpAsg_InsertedFor_ydotWheel_at_inport_0[4];
  real_T rtb_Integrator[4];
  real_T rtb_IntegratorSecondOrder_o1[4];
  real_T rtb_Integrator_c[4];
  real_T rtb_Integrator_g[4];
  real_T rtb_Kappa[4];
  real_T rtb_Mx[4];
  real_T rtb_My[4];
  real_T rtb_Saturation[4];
  real_T rtb_Saturation_h2[4];
  real_T rtb_Sum6[4];
  real_T rtb_Switch[4];
  real_T rtb_UnaryMinus[4];
  real_T rtb_VectorConcatenate_pe[4];
  real_T rtb_a[4];
  real_T rtb_b[4];
  real_T rtb_sig_x[4];
  real_T rtb_sig_y[4];
  real_T rtb_xdot[4];
  real_T tmpDrphiVar_data[4];
  real_T tmp_data[4];
  real_T rtb_Add[3];
  real_T rtb_Add1_h[3];
  real_T rtb_Add1_j[3];
  real_T rtb_Add1_l[3];
  real_T rtb_Add1_p[3];
  real_T rtb_Add_cm[3];
  real_T rtb_Add_ct[3];
  real_T rtb_Add_d1[3];
  real_T rtb_Add_fb[3];
  real_T rtb_Add_ms[3];
  real_T rtb_Divide3[3];
  real_T rtb_Fg_B[3];
  real_T rtb_Product[3];
  real_T rtb_Product_fe[3];
  real_T rtb_Sum_eb[3];
  real_T rtb_Sum_hy[3];
  real_T rtb_UnaryMinus1[3];
  real_T rtb_UnaryMinus_b[3];
  real_T rtb_UnitConversion3[3];
  real_T rtb_V_wb[3];
  real_T rtb_pqr[3];
  real_T rtb_sincos_o1_d[3];
  real_T rtb_sincos_o2_a[3];
  real_T rtb_VectorConcatenate_b[2];
  real_T Drphi_0;
  real_T Fzo_prime_idx_2;
  real_T Itemp_tmp_0;
  real_T SHy_idx_3;
  real_T lam_mux_0;
  real_T lam_muy_prime_idx_0;
  real_T lam_muy_prime_idx_3;
  real_T rtb_Add1_b_idx_0;
  real_T rtb_Add1_b_idx_1;
  real_T rtb_Add1_b_idx_2;
  real_T rtb_Add_f_tmp;
  real_T rtb_Add_n;
  real_T rtb_Alpha_b;
  real_T rtb_Beta;
  real_T rtb_Cg_h_Vel_Ydot;
  real_T rtb_Cg_h_Vel_Zdot;
  real_T rtb_FrntAxl_Rght_Vel_Ydot;
  real_T rtb_FrntAxl_Rght_Vel_Zdot;
  real_T rtb_Gain2_e;
  real_T rtb_Hitch_Disp_Y;
  real_T rtb_Hitch_Disp_Z;
  real_T rtb_Hitch_Vel_Xdot;
  real_T rtb_Hitch_Vel_Ydot;
  real_T rtb_Hitch_Vel_Zdot;
  real_T rtb_Integrator1_h_idx_0;
  real_T rtb_Integrator1_h_idx_1;
  real_T rtb_Integrator1_h_idx_2;
  real_T rtb_Integrator1_h_idx_3;
  real_T rtb_Integrator1_idx_0;
  real_T rtb_Integrator1_idx_0_tmp;
  real_T rtb_Integrator1_idx_1;
  real_T rtb_IntegratorSecondOrder_o1_k;
  real_T rtb_Lft_c_Vel_Xdot;
  real_T rtb_Lft_c_Vel_Ydot;
  real_T rtb_Product1_h;
  real_T rtb_Product1_l;
  real_T rtb_Product2_c;
  real_T rtb_Product2_l;
  real_T rtb_Product_c;
  real_T rtb_Product_ju;
  real_T rtb_Subtract1_idx_0;
  real_T rtb_Subtract1_idx_1;
  real_T rtb_Subtract1_idx_1_tmp;
  real_T rtb_Subtract1_idx_2;
  real_T rtb_UnaryMinus_d_tmp;
  real_T rtb_UnaryMinus_h_idx_0;
  real_T rtb_UnaryMinus_h_idx_1;
  real_T rtb_UnaryMinus_h_idx_2;
  real_T rtb_UnaryMinus_i;
  real_T rtb_UnitConversion3_h;
  real_T rtb_VectorConcatenate_b_tmp;
  real_T rtb_VectorConcatenate_b_tmp_0;
  real_T rtb_VectorConcatenate_b_tmp_1;
  real_T rtb_VectorConcatenate_b_tmp_2;
  real_T rtb_VectorConcatenate_b_tmp_3;
  real_T rtb_VectorConcatenate_tmp;
  real_T rtb_Vel_Xdot;
  real_T rtb_Vel_Ydot;
  real_T rtb_angles_idx_0;
  real_T rtb_angles_idx_1;
  real_T rtb_ixk;
  real_T rtb_jxi;
  real_T rtb_kxj;
  real_T rtb_phidot;
  real_T rtb_pqr_a;
  real_T rtb_pqr_tmp;
  real_T rtb_pqr_tmp_0;
  real_T rtb_psidot;
  real_T rtb_sig_x_f;
  real_T rtb_thetadot;
  real_T rtb_thetadot_tmp;
  real_T rtb_xdot_o;
  real_T rtb_xdot_p;
  real_T rtb_xdot_tmp;
  real_T rtb_z_idx_0;
  real_T rtb_z_idx_1;
  real_T rtb_z_idx_2;
  real_T u0;
  real_T u0_0;
  int32_T tb_size[2];
  int32_T tmp_size[2];
  int32_T tmp_size_0[2];
  int32_T Itemp_tmp;
  int32_T ibmat;
  int32_T iy;
  int32_T n_size;
  int32_T o_size;
  int32_T rtb_phidot_tmp;
  int32_T tmp_size_1;
  int32_T w_size;
  int8_T c_b[21];
  int8_T ab_data[4];
  int8_T bb_data[4];
  int8_T cb_data[4];
  int8_T db_data[4];
  int8_T eb_data[4];
  int8_T fb_data[4];
  int8_T gb_data[4];
  int8_T hb_data[4];
  int8_T ib_data[4];
  int8_T jb_data[4];
  int8_T kb_data[4];
  int8_T lb_data[4];
  int8_T mb_data[4];
  int8_T n_data[4];
  int8_T nb_data[4];
  int8_T o_data[4];
  int8_T ob_data[4];
  int8_T p_data[4];
  int8_T pb_data[4];
  int8_T q_data[4];
  int8_T qb_data[4];
  int8_T r_data[4];
  int8_T rb_data[4];
  int8_T s_data[4];
  int8_T t_data[4];
  int8_T u_data[4];
  int8_T v_data[4];
  int8_T w_data[4];
  int8_T x_data[4];
  int8_T y_data[4];
  boolean_T isLowSpeed[4];
  boolean_T isLowSpeed_0[4];
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* set solver stop time */
    if (!((&Chassis_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&Chassis_M)->solverInfo,
                            (((&Chassis_M)->Timing.clockTickH0 + 1) * (&Chassis_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(&(&Chassis_M)->solverInfo,
                            (((&Chassis_M)->Timing.clockTick0 + 1) * (&Chassis_M)->Timing.stepSize0 +
                             (&Chassis_M)->Timing.clockTickH0 * (&Chassis_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&Chassis_M))) {
    (&Chassis_M)->Timing.t[0] = rtsiGetT(&(&Chassis_M)->solverInfo);
  }

  /* Integrator: '<S115>/phi theta psi' */
  Chassis_B.phithetapsi[0] = Chassis_X.phithetapsi_CSTATE[0];
  Chassis_B.phithetapsi[1] = Chassis_X.phithetapsi_CSTATE[1];
  Chassis_B.phithetapsi[2] = Chassis_X.phithetapsi_CSTATE[2];

  /* Trigonometry: '<S123>/sincos' incorporates:
   *  SignalConversion generated from: '<S123>/sincos'
   */
  rtb_UnitConversion3[0] = std::cos(Chassis_B.phithetapsi[2]);
  rtb_pqr_a = std::sin(Chassis_B.phithetapsi[2]);

  /* Integrator: '<S109>/ub,vb,wb' */
  Chassis_B.ubvbwb[0] = Chassis_X.ubvbwb_CSTATE[0];

  /* Trigonometry: '<S123>/sincos' incorporates:
   *  SignalConversion generated from: '<S123>/sincos'
   */
  rtb_Subtract1_idx_0 = std::cos(Chassis_B.phithetapsi[1]);
  rtb_Subtract1_idx_1 = std::sin(Chassis_B.phithetapsi[1]);

  /* Integrator: '<S109>/ub,vb,wb' */
  Chassis_B.ubvbwb[1] = Chassis_X.ubvbwb_CSTATE[1];

  /* Trigonometry: '<S123>/sincos' incorporates:
   *  SignalConversion generated from: '<S123>/sincos'
   */
  rtb_Subtract1_idx_1_tmp = std::cos(Chassis_B.phithetapsi[0]);
  rtb_Integrator1_idx_0_tmp = std::sin(Chassis_B.phithetapsi[0]);

  /* Integrator: '<S109>/ub,vb,wb' */
  Chassis_B.ubvbwb[2] = Chassis_X.ubvbwb_CSTATE[2];

  /* Fcn: '<S123>/Fcn11' incorporates:
   *  Trigonometry: '<S123>/sincos'
   */
  rtb_VectorConcatenate[0] = rtb_UnitConversion3[0] * rtb_Subtract1_idx_0;

  /* Fcn: '<S123>/Fcn21' incorporates:
   *  Fcn: '<S123>/Fcn22'
   *  Trigonometry: '<S123>/sincos'
   */
  rtb_Subtract1_idx_2 = rtb_Subtract1_idx_1 * rtb_Integrator1_idx_0_tmp;
  rtb_VectorConcatenate[1] = rtb_Subtract1_idx_2 * rtb_UnitConversion3[0] - rtb_pqr_a * rtb_Subtract1_idx_1_tmp;

  /* Fcn: '<S123>/Fcn31' incorporates:
   *  Fcn: '<S123>/Fcn32'
   *  Trigonometry: '<S123>/sincos'
   */
  rtb_VectorConcatenate_tmp = rtb_Subtract1_idx_1 * rtb_Subtract1_idx_1_tmp;
  rtb_VectorConcatenate[2] = rtb_VectorConcatenate_tmp * rtb_UnitConversion3[0] + rtb_pqr_a * rtb_Integrator1_idx_0_tmp;

  /* Fcn: '<S123>/Fcn12' incorporates:
   *  Trigonometry: '<S123>/sincos'
   */
  rtb_VectorConcatenate[3] = rtb_pqr_a * rtb_Subtract1_idx_0;

  /* Fcn: '<S123>/Fcn22' incorporates:
   *  Trigonometry: '<S123>/sincos'
   */
  rtb_VectorConcatenate[4] = rtb_Subtract1_idx_2 * rtb_pqr_a + rtb_UnitConversion3[0] * rtb_Subtract1_idx_1_tmp;

  /* Fcn: '<S123>/Fcn32' incorporates:
   *  Trigonometry: '<S123>/sincos'
   */
  rtb_VectorConcatenate[5] = rtb_VectorConcatenate_tmp * rtb_pqr_a - rtb_UnitConversion3[0] * rtb_Integrator1_idx_0_tmp;

  /* Fcn: '<S123>/Fcn13' incorporates:
   *  Trigonometry: '<S123>/sincos'
   */
  rtb_VectorConcatenate[6] = -rtb_Subtract1_idx_1;

  /* Fcn: '<S123>/Fcn23' incorporates:
   *  Trigonometry: '<S123>/sincos'
   */
  rtb_VectorConcatenate[7] = rtb_Subtract1_idx_0 * rtb_Integrator1_idx_0_tmp;

  /* Fcn: '<S123>/Fcn33' incorporates:
   *  Trigonometry: '<S123>/sincos'
   */
  // added by dongyuanhu
  rtb_VectorConcatenate[8] = rtb_Subtract1_idx_0 * rtb_Subtract1_idx_1_tmp;
  real_T saturaion_ub_my = Chassis_B.ubvbwb[0], saturaion_vb_my = Chassis_B.ubvbwb[1],
         saturaion_wb_my = Chassis_B.ubvbwb[2];

  saturaion_ub_my = std::abs(saturaion_ub_my) < 0.01 ? 0.0 : saturaion_ub_my;
  saturaion_vb_my = std::abs(saturaion_vb_my) < 0.001 ? 0.0 : saturaion_vb_my;
  /*
  saturaion_wb_my =  std::abs(saturaion_wb_my) < 0.01 ? 0.0 : saturaion_wb_my;
  */

  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    /* Product: '<S122>/Product' incorporates:
     *  Concatenate: '<S125>/Vector Concatenate'
     *  Integrator: '<S109>/ub,vb,wb'
     *  Math: '<S109>/Transpose'
     */
    Chassis_B.Product[rtb_phidot_tmp] = 0.0;
    Chassis_B.Product[rtb_phidot_tmp] += rtb_VectorConcatenate[3 * rtb_phidot_tmp] * saturaion_ub_my;
    Chassis_B.Product[rtb_phidot_tmp] += rtb_VectorConcatenate[3 * rtb_phidot_tmp + 1] * saturaion_vb_my;
    Chassis_B.Product[rtb_phidot_tmp] += rtb_VectorConcatenate[3 * rtb_phidot_tmp + 2] * saturaion_wb_my;
  }

  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Memory: '<S141>/Memory1' */
    Chassis_B.Memory1[0] = Chassis_DW.Memory1_PreviousInput[0];
    Chassis_B.Memory1[1] = Chassis_DW.Memory1_PreviousInput[1];
    Chassis_B.Memory1[2] = Chassis_DW.Memory1_PreviousInput[2];
  }

  /* RelationalOperator: '<S142>/Compare' incorporates:
   *  Abs: '<S141>/Abs'
   *  Constant: '<S142>/Constant'
   *  Sum: '<S141>/Subtract'
   */
  Chassis_B.Compare[0] =
      (std::abs(Chassis_B.phithetapsi[0] - Chassis_B.Memory1[0]) >= Chassis_P.CompareToConstant_const);
  Chassis_B.Compare[1] =
      (std::abs(Chassis_B.phithetapsi[1] - Chassis_B.Memory1[1]) >= Chassis_P.CompareToConstant_const);
  Chassis_B.Compare[2] =
      (std::abs(Chassis_B.phithetapsi[2] - Chassis_B.Memory1[2]) >= Chassis_P.CompareToConstant_const);
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Outputs for Triggered SubSystem: '<S141>/Function-Call Subsystem' incorporates:
     *  TriggerPort: '<S143>/function'
     */
    if (rtsiIsModeUpdateTimeStep(&(&Chassis_M)->solverInfo)) {
      if ((Chassis_B.Compare[0] && (Chassis_PrevZCX.FunctionCallSubsystem_Trig_ZCE[0] != POS_ZCSIG)) ||
          (Chassis_B.Compare[1] && (Chassis_PrevZCX.FunctionCallSubsystem_Trig_ZCE[1] != POS_ZCSIG)) ||
          (Chassis_B.Compare[2] && (Chassis_PrevZCX.FunctionCallSubsystem_Trig_ZCE[2] != POS_ZCSIG))) {
        /* Signum: '<S143>/Sign' */
        if (std::isnan(Chassis_B.phithetapsi[0])) {
          Drphi_0 = (rtNaN);
        } else if (Chassis_B.phithetapsi[0] < 0.0) {
          Drphi_0 = -1.0;
        } else {
          Drphi_0 = (Chassis_B.phithetapsi[0] > 0.0);
        }

        /* Sum: '<S143>/Sum' incorporates:
         *  Constant: '<S143>/Constant'
         *  Product: '<S143>/Product'
         *  Signum: '<S143>/Sign'
         *  UnitDelay: '<S143>/Unit Delay'
         */
        Chassis_B.Sum_j[0] = Chassis_DW.UnitDelay_DSTATE[0] - Drphi_0 * Chassis_P.Constant_Value;

        /* Update for UnitDelay: '<S143>/Unit Delay' */
        Chassis_DW.UnitDelay_DSTATE[0] = Chassis_B.Sum_j[0];

        /* Signum: '<S143>/Sign' */
        if (std::isnan(Chassis_B.phithetapsi[1])) {
          Drphi_0 = (rtNaN);
        } else if (Chassis_B.phithetapsi[1] < 0.0) {
          Drphi_0 = -1.0;
        } else {
          Drphi_0 = (Chassis_B.phithetapsi[1] > 0.0);
        }

        /* Sum: '<S143>/Sum' incorporates:
         *  Constant: '<S143>/Constant'
         *  Product: '<S143>/Product'
         *  Signum: '<S143>/Sign'
         *  UnitDelay: '<S143>/Unit Delay'
         */
        Chassis_B.Sum_j[1] = Chassis_DW.UnitDelay_DSTATE[1] - Drphi_0 * Chassis_P.Constant_Value;

        /* Update for UnitDelay: '<S143>/Unit Delay' */
        Chassis_DW.UnitDelay_DSTATE[1] = Chassis_B.Sum_j[1];

        /* Signum: '<S143>/Sign' */
        if (std::isnan(Chassis_B.phithetapsi[2])) {
          Drphi_0 = (rtNaN);
        } else if (Chassis_B.phithetapsi[2] < 0.0) {
          Drphi_0 = -1.0;
        } else {
          Drphi_0 = (Chassis_B.phithetapsi[2] > 0.0);
        }

        /* Sum: '<S143>/Sum' incorporates:
         *  Constant: '<S143>/Constant'
         *  Product: '<S143>/Product'
         *  Signum: '<S143>/Sign'
         *  UnitDelay: '<S143>/Unit Delay'
         */
        Chassis_B.Sum_j[2] = Chassis_DW.UnitDelay_DSTATE[2] - Drphi_0 * Chassis_P.Constant_Value;

        /* Update for UnitDelay: '<S143>/Unit Delay' */
        Chassis_DW.UnitDelay_DSTATE[2] = Chassis_B.Sum_j[2];
      }

      Chassis_PrevZCX.FunctionCallSubsystem_Trig_ZCE[0] = Chassis_B.Compare[0];
      Chassis_PrevZCX.FunctionCallSubsystem_Trig_ZCE[1] = Chassis_B.Compare[1];
      Chassis_PrevZCX.FunctionCallSubsystem_Trig_ZCE[2] = Chassis_B.Compare[2];
    }

    /* End of Outputs for SubSystem: '<S141>/Function-Call Subsystem' */
  }

  /* Sum: '<S141>/Subtract1' */
  rtb_Subtract1_idx_0 = Chassis_B.Sum_j[0] + Chassis_B.phithetapsi[0];
  rtb_Subtract1_idx_1 = Chassis_B.Sum_j[1] + Chassis_B.phithetapsi[1];
  rtb_Subtract1_idx_2 = Chassis_B.Sum_j[2] + Chassis_B.phithetapsi[2];

  /* BusCreator generated from: '<Root>/VehBody' incorporates:
   *  Integrator: '<S109>/xe,ye,ze'
   */
  rtb_pqr_a = Chassis_X.xeyeze_CSTATE[0];
  rtb_VectorConcatenate_tmp = Chassis_X.xeyeze_CSTATE[1];
  rtb_Subtract1_idx_1_tmp = Chassis_X.xeyeze_CSTATE[2];
  rtb_Integrator1_idx_0_tmp = Chassis_B.Product[0];
  rtb_Cg_h_Vel_Ydot = Chassis_B.Product[1];
  rtb_Cg_h_Vel_Zdot = Chassis_B.Product[2];

  /* SignalConversion generated from: '<S159>/sincos' */
  rtb_UnitConversion3[0] = rtb_Subtract1_idx_2;
  rtb_UnitConversion3[1] = rtb_Subtract1_idx_1;
  rtb_UnitConversion3[2] = rtb_Subtract1_idx_0;

  /* Outputs for Iterator SubSystem: '<S93>/Susp2Body' incorporates:
   *  ForEach: '<S97>/For Each'
   */
  /* Trigonometry: '<S159>/sincos' incorporates:
   *  SignalConversion generated from: '<S159>/sincos'
   *  Trigonometry: '<S139>/sincos'
   *  Trigonometry: '<S176>/sincos'
   *  Trigonometry: '<S98>/sincos'
   */
  rtb_pqr_tmp_0 = std::sin(rtb_Subtract1_idx_2);
  rtb_Add_f_tmp = std::cos(rtb_Subtract1_idx_2);

  /* End of Outputs for SubSystem: '<S93>/Susp2Body' */
  rtb_Integrator1_idx_0 = std::sin(rtb_Subtract1_idx_1);
  rtb_Integrator1_idx_1 = std::cos(rtb_Subtract1_idx_1);
  rtb_pqr_tmp = std::sin(rtb_Subtract1_idx_0);
  rtb_UnaryMinus_h_idx_0 = std::cos(rtb_Subtract1_idx_0);

  /* Fcn: '<S159>/Fcn11' incorporates:
   *  Fcn: '<S176>/Fcn11'
   *  Trigonometry: '<S159>/sincos'
   */
  Drphi_0 = rtb_Add_f_tmp * rtb_Integrator1_idx_1;
  rtb_VectorConcatenate_i[0] = Drphi_0;

  /* Fcn: '<S159>/Fcn21' incorporates:
   *  Fcn: '<S176>/Fcn21'
   *  Trigonometry: '<S159>/sincos'
   */
  rtb_thetadot_tmp = rtb_Integrator1_idx_0 * rtb_pqr_tmp;
  rtb_VectorConcatenate_b_tmp = rtb_pqr_tmp_0 * rtb_UnaryMinus_h_idx_0;
  rtb_VectorConcatenate_i[1] = rtb_thetadot_tmp * rtb_Add_f_tmp - rtb_VectorConcatenate_b_tmp;

  /* Fcn: '<S159>/Fcn31' incorporates:
   *  Fcn: '<S176>/Fcn31'
   *  Trigonometry: '<S159>/sincos'
   */
  rtb_UnaryMinus_d_tmp = rtb_Integrator1_idx_0 * rtb_UnaryMinus_h_idx_0;
  rtb_VectorConcatenate_b_tmp_0 = rtb_pqr_tmp_0 * rtb_pqr_tmp;
  rtb_VectorConcatenate_i[2] = rtb_UnaryMinus_d_tmp * rtb_Add_f_tmp + rtb_VectorConcatenate_b_tmp_0;

  /* Fcn: '<S159>/Fcn12' incorporates:
   *  Fcn: '<S176>/Fcn12'
   *  Trigonometry: '<S159>/sincos'
   */
  rtb_phidot = rtb_pqr_tmp_0 * rtb_Integrator1_idx_1;
  rtb_VectorConcatenate_i[3] = rtb_phidot;

  /* Fcn: '<S159>/Fcn22' incorporates:
   *  Fcn: '<S159>/Fcn21'
   *  Fcn: '<S176>/Fcn22'
   *  Trigonometry: '<S159>/sincos'
   */
  rtb_Beta = rtb_Add_f_tmp * rtb_UnaryMinus_h_idx_0;
  rtb_VectorConcatenate_i[4] = rtb_thetadot_tmp * rtb_pqr_tmp_0 + rtb_Beta;

  /* Fcn: '<S159>/Fcn32' incorporates:
   *  Fcn: '<S159>/Fcn31'
   *  Fcn: '<S176>/Fcn32'
   *  Trigonometry: '<S159>/sincos'
   */
  rtb_VectorConcatenate_b_tmp_1 = rtb_Add_f_tmp * rtb_pqr_tmp;
  rtb_VectorConcatenate_i[5] = rtb_UnaryMinus_d_tmp * rtb_pqr_tmp_0 - rtb_VectorConcatenate_b_tmp_1;

  /* Fcn: '<S159>/Fcn13' incorporates:
   *  Trigonometry: '<S159>/sincos'
   */
  rtb_VectorConcatenate_i[6] = -rtb_Integrator1_idx_0;

  /* Fcn: '<S159>/Fcn23' incorporates:
   *  Fcn: '<S176>/Fcn23'
   *  Trigonometry: '<S159>/sincos'
   */
  rtb_VectorConcatenate_b_tmp_2 = rtb_Integrator1_idx_1 * rtb_pqr_tmp;
  rtb_VectorConcatenate_i[7] = rtb_VectorConcatenate_b_tmp_2;

  /* Fcn: '<S159>/Fcn33' incorporates:
   *  Fcn: '<S176>/Fcn33'
   *  Trigonometry: '<S159>/sincos'
   */
  rtb_VectorConcatenate_b_tmp_3 = rtb_Integrator1_idx_1 * rtb_UnaryMinus_h_idx_0;
  rtb_VectorConcatenate_i[8] = rtb_VectorConcatenate_b_tmp_3;

  /* Math: '<S158>/Transpose1' incorporates:
   *  Concatenate: '<S101>/Vector Concatenate'
   *  Math: '<S220>/Transpose'
   */
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    rtb_Transpose_n[3 * rtb_phidot_tmp] = rtb_VectorConcatenate_i[rtb_phidot_tmp];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 1] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 3];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 2] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 6];
  }

  /* End of Math: '<S158>/Transpose1' */
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* MATLAB Function: '<S94>/vehdyncginert' */
    rtb_pqr[0] = Chassis_P.VEH.FrontAxlePositionfromCG;
    rtb_pqr[1] = -Chassis_P.VehicleBody6DOF_d;
    rtb_pqr[2] = Chassis_P.VEH.HeightCG;
    for (iy = 0; iy < 3; iy++) {
      ibmat = iy * 7;
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 7; rtb_phidot_tmp++) {
        b_b[ibmat + rtb_phidot_tmp] = rtb_pqr[iy];
      }
    }

    for (iy = 0; iy < 3; iy++) {
      ibmat = iy * 7;
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 7; rtb_phidot_tmp++) {
        c_b[ibmat + rtb_phidot_tmp] = c[iy];
      }

      R[iy << 3] = 0.0;
    }

    R[1] = Chassis_P.VehicleBody6DOF_z1R[0] * static_cast<real_T>(c_b[0]) + b_b[0];
    R[2] = Chassis_P.VehicleBody6DOF_z2R[0] * static_cast<real_T>(c_b[1]) + b_b[1];
    R[3] = Chassis_P.VehicleBody6DOF_z3R[0] * static_cast<real_T>(c_b[2]) + b_b[2];
    R[4] = Chassis_P.VehicleBody6DOF_z4R[0] * static_cast<real_T>(c_b[3]) + b_b[3];
    R[5] = Chassis_P.VehicleBody6DOF_z5R[0] * static_cast<real_T>(c_b[4]) + b_b[4];
    R[6] = Chassis_P.VehicleBody6DOF_z6R[0] * static_cast<real_T>(c_b[5]) + b_b[5];
    R[7] = Chassis_P.VehicleBody6DOF_z7R[0] * static_cast<real_T>(c_b[6]) + b_b[6];
    R[9] = Chassis_P.VehicleBody6DOF_z1R[1] * static_cast<real_T>(c_b[7]) + b_b[7];
    R[10] = Chassis_P.VehicleBody6DOF_z2R[1] * static_cast<real_T>(c_b[8]) + b_b[8];
    R[11] = Chassis_P.VehicleBody6DOF_z3R[1] * static_cast<real_T>(c_b[9]) + b_b[9];
    R[12] = Chassis_P.VehicleBody6DOF_z4R[1] * static_cast<real_T>(c_b[10]) + b_b[10];
    R[13] = Chassis_P.VehicleBody6DOF_z5R[1] * static_cast<real_T>(c_b[11]) + b_b[11];
    R[14] = Chassis_P.VehicleBody6DOF_z6R[1] * static_cast<real_T>(c_b[12]) + b_b[12];
    R[15] = Chassis_P.VehicleBody6DOF_z7R[1] * static_cast<real_T>(c_b[13]) + b_b[13];
    R[17] = Chassis_P.VehicleBody6DOF_z1R[2] * static_cast<real_T>(c_b[14]) + b_b[14];
    R[18] = Chassis_P.VehicleBody6DOF_z2R[2] * static_cast<real_T>(c_b[15]) + b_b[15];
    R[19] = Chassis_P.VehicleBody6DOF_z3R[2] * static_cast<real_T>(c_b[16]) + b_b[16];
    R[20] = Chassis_P.VehicleBody6DOF_z4R[2] * static_cast<real_T>(c_b[17]) + b_b[17];
    R[21] = Chassis_P.VehicleBody6DOF_z5R[2] * static_cast<real_T>(c_b[18]) + b_b[18];
    R[22] = Chassis_P.VehicleBody6DOF_z6R[2] * static_cast<real_T>(c_b[19]) + b_b[19];
    R[23] = Chassis_P.VehicleBody6DOF_z7R[2] * static_cast<real_T>(c_b[20]) + b_b[20];
    // modified by dongyuanhu
    rtb_MatrixConcatenate4[0] = Chassis_P.VEH.SprungMass;
    rtb_MatrixConcatenate4[1] = Chassis_P.VehicleBody6DOF_z1m;
    rtb_MatrixConcatenate4[2] = Chassis_P.VehicleBody6DOF_z2m;
    rtb_MatrixConcatenate4[3] = Chassis_P.VehicleBody6DOF_z3m;
    rtb_MatrixConcatenate4[4] = Chassis_P.VehicleBody6DOF_z4m;
    rtb_MatrixConcatenate4[5] = Chassis_P.VehicleBody6DOF_z5m;
    rtb_MatrixConcatenate4[6] = Chassis_P.VehicleBody6DOF_z6m;
    rtb_MatrixConcatenate4[7] = Chassis_P.VehicleBody6DOF_z7m;
    for (ibmat = 0; ibmat < 9; ibmat++) {
      Imat[ibmat] = Chassis_P.VehicleBody6DOF_Iveh[ibmat];
      Imat[ibmat + 9] = Chassis_P.VehicleBody6DOF_z1I[ibmat];
      Imat[ibmat + 18] = Chassis_P.VehicleBody6DOF_z2I[ibmat];
      Imat[ibmat + 27] = Chassis_P.VehicleBody6DOF_z3I[ibmat];
      Imat[ibmat + 36] = Chassis_P.VehicleBody6DOF_z4I[ibmat];
      Imat[ibmat + 45] = Chassis_P.VehicleBody6DOF_z5I[ibmat];
      Imat[ibmat + 54] = Chassis_P.VehicleBody6DOF_z6I[ibmat];
      Imat[ibmat + 63] = Chassis_P.VehicleBody6DOF_z7I[ibmat];
    }

    for (iy = 0; iy < 3; iy++) {
      std::memcpy(&d_b[iy << 3], &rtb_MatrixConcatenate4[0], sizeof(real_T) << 3U);
    }
    // modified by dongyuanhu
    rtb_Vel_Xdot = Chassis_P.VEH.SprungMass;
    for (iy = 0; iy < 7; iy++) {
      rtb_Vel_Xdot += rtb_MatrixConcatenate4[iy + 1];
    }

    for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= 22; rtb_phidot_tmp += 2) {
      /* MATLAB Function: '<S94>/vehdyncginert' */
      tmp_5 = _mm_loadu_pd(&d_b[rtb_phidot_tmp]);
      tmp_6 = _mm_loadu_pd(&R[rtb_phidot_tmp]);
      _mm_storeu_pd(&d_b[rtb_phidot_tmp], _mm_div_pd(_mm_mul_pd(tmp_5, tmp_6), _mm_set1_pd(rtb_Vel_Xdot)));
    }

    /* MATLAB Function: '<S94>/vehdyncginert' */
    for (iy = 0; iy < 3; iy++) {
      ibmat = iy << 3;
      Chassis_B.Rbar[iy] = d_b[ibmat];
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 7; rtb_phidot_tmp++) {
        Chassis_B.Rbar[iy] += d_b[(ibmat + rtb_phidot_tmp) + 1];
      }
    }
    // modified by dongyuanhu
    rtb_Vel_Xdot = Chassis_P.VEH.SprungMass;
    for (iy = 0; iy < 7; iy++) {
      rtb_Vel_Xdot += rtb_MatrixConcatenate4[iy + 1];
    }

    for (iy = 0; iy < 3; iy++) {
      ibmat = iy << 3;
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 8; rtb_phidot_tmp++) {
        d_b[ibmat + rtb_phidot_tmp] = Chassis_B.Rbar[iy];
      }
    }

    for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= 22; rtb_phidot_tmp += 2) {
      /* MATLAB Function: '<S94>/vehdyncginert' */
      tmp_5 = _mm_loadu_pd(&R[rtb_phidot_tmp]);
      tmp_6 = _mm_loadu_pd(&d_b[rtb_phidot_tmp]);
      _mm_storeu_pd(&R[rtb_phidot_tmp], _mm_sub_pd(tmp_5, tmp_6));
    }

    /* MATLAB Function: '<S94>/vehdyncginert' */
    for (iy = 0; iy < 8; iy++) {
      rtb_pqr[0] = R[iy];
      rtb_pqr[1] = R[iy + 8];
      rtb_pqr[2] = R[iy + 16];
      rtb_Vel_Ydot = (rtb_pqr[0] * rtb_pqr[0] + rtb_pqr[1] * rtb_pqr[1]) + rtb_pqr[2] * rtb_pqr[2];
      rtb_Add1_b_idx_2 = rtb_MatrixConcatenate4[iy];
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
        Itemp_tmp = 3 * rtb_phidot_tmp + 9 * iy;
        Itemp_tmp_0 = R[(rtb_phidot_tmp << 3) + iy];
        Itemp[Itemp_tmp] = (static_cast<real_T>(f_b[3 * rtb_phidot_tmp]) * rtb_Vel_Ydot - Itemp_tmp_0 * rtb_pqr[0]) *
                               rtb_Add1_b_idx_2 * static_cast<real_T>(e[3 * rtb_phidot_tmp]) +
                           Imat[Itemp_tmp];
        ibmat = 3 * rtb_phidot_tmp + 1;
        Itemp[Itemp_tmp + 1] = (static_cast<real_T>(f_b[ibmat]) * rtb_Vel_Ydot - Itemp_tmp_0 * rtb_pqr[1]) *
                                   rtb_Add1_b_idx_2 * static_cast<real_T>(e[ibmat]) +
                               Imat[Itemp_tmp + 1];
        ibmat = 3 * rtb_phidot_tmp + 2;
        Itemp[Itemp_tmp + 2] = (static_cast<real_T>(f_b[ibmat]) * rtb_Vel_Ydot - Itemp_tmp_0 * rtb_pqr[2]) *
                                   rtb_Add1_b_idx_2 * static_cast<real_T>(e[ibmat]) +
                               Imat[Itemp_tmp + 2];
      }
    }

    std::memcpy(&rtb_Ibar[0], &Itemp[0], 9U * sizeof(real_T));
    for (iy = 0; iy < 7; iy++) {
      ibmat = (iy + 1) * 9;
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= 6; rtb_phidot_tmp += 2) {
        tmp_5 = _mm_loadu_pd(&Itemp[ibmat + rtb_phidot_tmp]);
        tmp_6 = _mm_loadu_pd(&rtb_Ibar[rtb_phidot_tmp]);
        _mm_storeu_pd(&rtb_Ibar[rtb_phidot_tmp], _mm_add_pd(tmp_5, tmp_6));
      }

      for (rtb_phidot_tmp = 8; rtb_phidot_tmp < 9; rtb_phidot_tmp++) {
        rtb_Ibar[rtb_phidot_tmp] += Itemp[ibmat + rtb_phidot_tmp];
      }
    }

    Chassis_B.Mbar = rtb_Vel_Xdot;
    Chassis_B.Xbar[0] = Chassis_P.VEH.FrontAxlePositionfromCG - Chassis_B.Rbar[0];
    Chassis_B.Xbar[1] = Chassis_P.VEH.RearAxlePositionfromCG + Chassis_B.Rbar[0];
    Chassis_B.Xbar[2] = Chassis_P.VEH.HeightCG - Chassis_B.Rbar[2];
    rtb_Add1_b_idx_2 = Chassis_P.VEH.TrackWidth / 2.0;
    rtb_Vel_Xdot = rtb_Add1_b_idx_2 + Chassis_B.Rbar[1];
    Chassis_B.Wbar[0] = rtb_Vel_Xdot;
    rtb_Add1_b_idx_2 -= Chassis_B.Rbar[1];
    Chassis_B.Wbar[1] = rtb_Add1_b_idx_2;
    Chassis_B.Wbar[2] = rtb_Vel_Xdot;
    Chassis_B.Wbar[3] = rtb_Add1_b_idx_2;
    Chassis_B.HPbar[0] = Chassis_B.Xbar[0];
    Chassis_B.HPbar[3] = Chassis_B.Xbar[0];
    Chassis_B.HPbar[6] = -Chassis_B.Xbar[1];
    Chassis_B.HPbar[9] = -Chassis_B.Xbar[1];
    Chassis_B.HPbar[1] = -Chassis_B.Wbar[0];
    Chassis_B.HPbar[4] = Chassis_B.Wbar[1];
    Chassis_B.HPbar[7] = -Chassis_B.Wbar[2];
    Chassis_B.HPbar[10] = Chassis_B.Wbar[3];
    Chassis_B.HPbar[2] = Chassis_B.Xbar[2];
    Chassis_B.HPbar[5] = Chassis_B.Xbar[2];
    Chassis_B.HPbar[8] = Chassis_B.Xbar[2];
    Chassis_B.HPbar[11] = Chassis_B.Xbar[2];

    /* Selector: '<S152>/Selector1' */
    Chassis_B.Selector1[0] = Chassis_B.HPbar[0];
    Chassis_B.Selector1[1] = Chassis_B.HPbar[1];
    Chassis_B.Selector1[2] = Chassis_B.HPbar[2];
  }

  /* Sum: '<S162>/Sum' incorporates:
   *  Integrator: '<S109>/p,q,r '
   *  Product: '<S164>/i x j'
   *  Product: '<S164>/j x k'
   *  Product: '<S164>/k x i'
   *  Product: '<S165>/i x k'
   *  Product: '<S165>/j x i'
   *  Product: '<S165>/k x j'
   */
  rtb_Add1_b_idx_0 =
      Chassis_X.pqr_CSTATE[1] * Chassis_B.Selector1[2] - Chassis_B.Selector1[1] * Chassis_X.pqr_CSTATE[2];
  rtb_Add1_b_idx_1 =
      Chassis_B.Selector1[0] * Chassis_X.pqr_CSTATE[2] - Chassis_X.pqr_CSTATE[0] * Chassis_B.Selector1[2];
  rtb_Add1_b_idx_2 =
      Chassis_X.pqr_CSTATE[0] * Chassis_B.Selector1[1] - Chassis_B.Selector1[0] * Chassis_X.pqr_CSTATE[1];
  for (iy = 0; iy < 3; iy++) {
    /* Product: '<S161>/Product' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S160>/Product'
     */
    rtb_Vel_Xdot = rtb_Transpose_n[iy + 3];
    rtb_Vel_Ydot = rtb_Transpose_n[iy + 6];

    /* Sum: '<S158>/Add' incorporates:
     *  Integrator: '<S109>/xe,ye,ze'
     *  Math: '<S220>/Transpose'
     *  Product: '<S161>/Product'
     *  Selector: '<S152>/Selector1'
     */
    rtb_Add_ct[iy] = ((rtb_Vel_Xdot * Chassis_B.Selector1[1] + rtb_Transpose_n[iy] * Chassis_B.Selector1[0]) +
                      rtb_Vel_Ydot * Chassis_B.Selector1[2]) +
                     Chassis_X.xeyeze_CSTATE[iy];

    /* Integrator: '<S109>/p,q,r ' */
    rtb_pqr[iy] = Chassis_X.pqr_CSTATE[iy];

    /* Trigonometry: '<S167>/sincos' */
    rtb_UnitConversion3_h = rtb_UnitConversion3[iy];

    /* Sum: '<S158>/Add4' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S160>/Product'
     *  Sum: '<S158>/Add1'
     */
    rtb_V_wb[iy] =
        ((rtb_Vel_Xdot * rtb_Add1_b_idx_1 + rtb_Transpose_n[iy] * rtb_Add1_b_idx_0) + rtb_Vel_Ydot * rtb_Add1_b_idx_2) +
        Chassis_B.Product[iy];

    /* Trigonometry: '<S167>/sincos' */
    rtb_Add1_h[iy] = std::sin(rtb_UnitConversion3_h);
    rtb_Add_fb[iy] = std::cos(rtb_UnitConversion3_h);
  }

  /* Fcn: '<S167>/Fcn11' */
  rtb_VectorConcatenate_i[0] = rtb_Add_fb[0] * rtb_Add_fb[1];

  /* Fcn: '<S167>/Fcn21' incorporates:
   *  Fcn: '<S167>/Fcn22'
   */
  rtb_thetadot = rtb_Add1_h[1] * rtb_Add1_h[2];
  rtb_VectorConcatenate_i[1] = rtb_thetadot * rtb_Add_fb[0] - rtb_Add1_h[0] * rtb_Add_fb[2];

  /* Fcn: '<S167>/Fcn31' incorporates:
   *  Fcn: '<S167>/Fcn32'
   */
  rtb_UnaryMinus_i = rtb_Add1_h[1] * rtb_Add_fb[2];
  rtb_VectorConcatenate_i[2] = rtb_UnaryMinus_i * rtb_Add_fb[0] + rtb_Add1_h[0] * rtb_Add1_h[2];

  /* Fcn: '<S167>/Fcn12' */
  rtb_VectorConcatenate_i[3] = rtb_Add1_h[0] * rtb_Add_fb[1];

  /* Fcn: '<S167>/Fcn22' */
  rtb_VectorConcatenate_i[4] = rtb_thetadot * rtb_Add1_h[0] + rtb_Add_fb[0] * rtb_Add_fb[2];

  /* Fcn: '<S167>/Fcn32' */
  rtb_VectorConcatenate_i[5] = rtb_UnaryMinus_i * rtb_Add1_h[0] - rtb_Add_fb[0] * rtb_Add1_h[2];

  /* Fcn: '<S167>/Fcn13' */
  rtb_VectorConcatenate_i[6] = -rtb_Add1_h[1];

  /* Fcn: '<S167>/Fcn23' */
  rtb_VectorConcatenate_i[7] = rtb_Add_fb[1] * rtb_Add1_h[2];

  /* Fcn: '<S167>/Fcn33' */
  rtb_VectorConcatenate_i[8] = rtb_Add_fb[1] * rtb_Add_fb[2];

  /* Math: '<S166>/Transpose1' incorporates:
   *  Concatenate: '<S101>/Vector Concatenate'
   *  Math: '<S220>/Transpose'
   */
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    rtb_Transpose_n[3 * rtb_phidot_tmp] = rtb_VectorConcatenate_i[rtb_phidot_tmp];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 1] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 3];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 2] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 6];
  }

  /* End of Math: '<S166>/Transpose1' */
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Selector: '<S153>/Selector1' */
    Chassis_B.Selector1_n[0] = Chassis_B.HPbar[3];
    Chassis_B.Selector1_n[1] = Chassis_B.HPbar[4];
    Chassis_B.Selector1_n[2] = Chassis_B.HPbar[5];
  }

  /* Sum: '<S170>/Sum' incorporates:
   *  Integrator: '<S109>/p,q,r '
   *  Product: '<S172>/i x j'
   *  Product: '<S172>/j x k'
   *  Product: '<S172>/k x i'
   *  Product: '<S173>/i x k'
   *  Product: '<S173>/j x i'
   *  Product: '<S173>/k x j'
   */
  rtb_Add1_h[0] =
      Chassis_X.pqr_CSTATE[1] * Chassis_B.Selector1_n[2] - Chassis_B.Selector1_n[1] * Chassis_X.pqr_CSTATE[2];
  rtb_Add1_h[1] =
      Chassis_B.Selector1_n[0] * Chassis_X.pqr_CSTATE[2] - Chassis_X.pqr_CSTATE[0] * Chassis_B.Selector1_n[2];
  rtb_Add1_h[2] =
      Chassis_X.pqr_CSTATE[0] * Chassis_B.Selector1_n[1] - Chassis_B.Selector1_n[0] * Chassis_X.pqr_CSTATE[1];
  for (iy = 0; iy < 3; iy++) {
    /* Product: '<S169>/Product' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S168>/Product'
     */
    rtb_Vel_Xdot = rtb_Transpose_n[iy + 3];
    rtb_Vel_Ydot = rtb_Transpose_n[iy + 6];

    /* Sum: '<S166>/Add' incorporates:
     *  Integrator: '<S109>/xe,ye,ze'
     *  Math: '<S220>/Transpose'
     *  Product: '<S169>/Product'
     *  Selector: '<S153>/Selector1'
     */
    rtb_Add_fb[iy] = ((rtb_Vel_Xdot * Chassis_B.Selector1_n[1] + rtb_Transpose_n[iy] * Chassis_B.Selector1_n[0]) +
                      rtb_Vel_Ydot * Chassis_B.Selector1_n[2]) +
                     Chassis_X.xeyeze_CSTATE[iy];

    /* Trigonometry: '<S195>/sincos' */
    rtb_UnitConversion3_h = rtb_UnitConversion3[iy];

    /* Sum: '<S166>/Add4' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S168>/Product'
     *  Sum: '<S166>/Add1'
     */
    rtb_Sum_hy[iy] =
        ((rtb_Vel_Xdot * rtb_Add1_h[1] + rtb_Transpose_n[iy] * rtb_Add1_h[0]) + rtb_Vel_Ydot * rtb_Add1_h[2]) +
        Chassis_B.Product[iy];

    /* Trigonometry: '<S195>/sincos' */
    rtb_Add1_j[iy] = std::sin(rtb_UnitConversion3_h);
    rtb_Add_d1[iy] = std::cos(rtb_UnitConversion3_h);
  }

  /* BusCreator generated from: '<Root>/VehBody' */
  Itemp_tmp_0 = rtb_Sum_hy[0];
  rtb_FrntAxl_Rght_Vel_Ydot = rtb_Sum_hy[1];
  rtb_FrntAxl_Rght_Vel_Zdot = rtb_Sum_hy[2];

  /* Fcn: '<S195>/Fcn11' */
  rtb_VectorConcatenate_i[0] = rtb_Add_d1[0] * rtb_Add_d1[1];

  /* Fcn: '<S195>/Fcn21' incorporates:
   *  Fcn: '<S195>/Fcn22'
   */
  rtb_thetadot = rtb_Add1_j[1] * rtb_Add1_j[2];
  rtb_VectorConcatenate_i[1] = rtb_thetadot * rtb_Add_d1[0] - rtb_Add1_j[0] * rtb_Add_d1[2];

  /* Fcn: '<S195>/Fcn31' incorporates:
   *  Fcn: '<S195>/Fcn32'
   */
  rtb_UnaryMinus_i = rtb_Add1_j[1] * rtb_Add_d1[2];
  rtb_VectorConcatenate_i[2] = rtb_UnaryMinus_i * rtb_Add_d1[0] + rtb_Add1_j[0] * rtb_Add1_j[2];

  /* Fcn: '<S195>/Fcn12' */
  rtb_VectorConcatenate_i[3] = rtb_Add1_j[0] * rtb_Add_d1[1];

  /* Fcn: '<S195>/Fcn22' */
  rtb_VectorConcatenate_i[4] = rtb_thetadot * rtb_Add1_j[0] + rtb_Add_d1[0] * rtb_Add_d1[2];

  /* Fcn: '<S195>/Fcn32' */
  rtb_VectorConcatenate_i[5] = rtb_UnaryMinus_i * rtb_Add1_j[0] - rtb_Add_d1[0] * rtb_Add1_j[2];

  /* Fcn: '<S195>/Fcn13' */
  rtb_VectorConcatenate_i[6] = -rtb_Add1_j[1];

  /* Fcn: '<S195>/Fcn23' */
  rtb_VectorConcatenate_i[7] = rtb_Add_d1[1] * rtb_Add1_j[2];

  /* Fcn: '<S195>/Fcn33' */
  rtb_VectorConcatenate_i[8] = rtb_Add_d1[1] * rtb_Add_d1[2];

  /* Math: '<S194>/Transpose1' incorporates:
   *  Concatenate: '<S101>/Vector Concatenate'
   *  Math: '<S220>/Transpose'
   */
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    rtb_Transpose_n[3 * rtb_phidot_tmp] = rtb_VectorConcatenate_i[rtb_phidot_tmp];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 1] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 3];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 2] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 6];
  }

  /* End of Math: '<S194>/Transpose1' */
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Selector: '<S156>/Selector1' */
    Chassis_B.Selector1_o[0] = Chassis_B.HPbar[6];
    Chassis_B.Selector1_o[1] = Chassis_B.HPbar[7];
    Chassis_B.Selector1_o[2] = Chassis_B.HPbar[8];
  }

  /* Sum: '<S198>/Sum' incorporates:
   *  Integrator: '<S109>/p,q,r '
   *  Product: '<S200>/i x j'
   *  Product: '<S200>/j x k'
   *  Product: '<S200>/k x i'
   *  Product: '<S201>/i x k'
   *  Product: '<S201>/j x i'
   *  Product: '<S201>/k x j'
   */
  rtb_Add1_j[0] =
      Chassis_X.pqr_CSTATE[1] * Chassis_B.Selector1_o[2] - Chassis_B.Selector1_o[1] * Chassis_X.pqr_CSTATE[2];
  rtb_Add1_j[1] =
      Chassis_B.Selector1_o[0] * Chassis_X.pqr_CSTATE[2] - Chassis_X.pqr_CSTATE[0] * Chassis_B.Selector1_o[2];
  rtb_Add1_j[2] =
      Chassis_X.pqr_CSTATE[0] * Chassis_B.Selector1_o[1] - Chassis_B.Selector1_o[0] * Chassis_X.pqr_CSTATE[1];
  for (iy = 0; iy < 3; iy++) {
    /* Product: '<S197>/Product' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S196>/Product'
     */
    rtb_Vel_Xdot = rtb_Transpose_n[iy + 3];
    rtb_Vel_Ydot = rtb_Transpose_n[iy + 6];

    /* Sum: '<S194>/Add' incorporates:
     *  Integrator: '<S109>/xe,ye,ze'
     *  Math: '<S220>/Transpose'
     *  Product: '<S197>/Product'
     *  Selector: '<S156>/Selector1'
     */
    rtb_Add_d1[iy] = ((rtb_Vel_Xdot * Chassis_B.Selector1_o[1] + rtb_Transpose_n[iy] * Chassis_B.Selector1_o[0]) +
                      rtb_Vel_Ydot * Chassis_B.Selector1_o[2]) +
                     Chassis_X.xeyeze_CSTATE[iy];

    /* Trigonometry: '<S203>/sincos' */
    rtb_UnitConversion3_h = rtb_UnitConversion3[iy];

    /* Sum: '<S194>/Add4' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S196>/Product'
     *  Sum: '<S194>/Add1'
     */
    rtb_UnaryMinus_b[iy] =
        ((rtb_Vel_Xdot * rtb_Add1_j[1] + rtb_Transpose_n[iy] * rtb_Add1_j[0]) + rtb_Vel_Ydot * rtb_Add1_j[2]) +
        Chassis_B.Product[iy];

    /* Trigonometry: '<S203>/sincos' */
    rtb_Add1_p[iy] = std::sin(rtb_UnitConversion3_h);
    rtb_Add_cm[iy] = std::cos(rtb_UnitConversion3_h);
  }

  /* BusCreator generated from: '<Root>/VehBody' */
  rtb_Lft_c_Vel_Xdot = rtb_UnaryMinus_b[0];
  rtb_Lft_c_Vel_Ydot = rtb_UnaryMinus_b[1];

  /* Fcn: '<S203>/Fcn11' */
  rtb_VectorConcatenate_i[0] = rtb_Add_cm[0] * rtb_Add_cm[1];

  /* Fcn: '<S203>/Fcn21' incorporates:
   *  Fcn: '<S203>/Fcn22'
   */
  rtb_thetadot = rtb_Add1_p[1] * rtb_Add1_p[2];
  rtb_VectorConcatenate_i[1] = rtb_thetadot * rtb_Add_cm[0] - rtb_Add1_p[0] * rtb_Add_cm[2];

  /* Fcn: '<S203>/Fcn31' incorporates:
   *  Fcn: '<S203>/Fcn32'
   */
  rtb_UnaryMinus_i = rtb_Add1_p[1] * rtb_Add_cm[2];
  rtb_VectorConcatenate_i[2] = rtb_UnaryMinus_i * rtb_Add_cm[0] + rtb_Add1_p[0] * rtb_Add1_p[2];

  /* Fcn: '<S203>/Fcn12' */
  rtb_VectorConcatenate_i[3] = rtb_Add1_p[0] * rtb_Add_cm[1];

  /* Fcn: '<S203>/Fcn22' */
  rtb_VectorConcatenate_i[4] = rtb_thetadot * rtb_Add1_p[0] + rtb_Add_cm[0] * rtb_Add_cm[2];

  /* Fcn: '<S203>/Fcn32' */
  rtb_VectorConcatenate_i[5] = rtb_UnaryMinus_i * rtb_Add1_p[0] - rtb_Add_cm[0] * rtb_Add1_p[2];

  /* Fcn: '<S203>/Fcn13' */
  rtb_VectorConcatenate_i[6] = -rtb_Add1_p[1];

  /* Fcn: '<S203>/Fcn23' */
  rtb_VectorConcatenate_i[7] = rtb_Add_cm[1] * rtb_Add1_p[2];

  /* Fcn: '<S203>/Fcn33' */
  rtb_VectorConcatenate_i[8] = rtb_Add_cm[1] * rtb_Add_cm[2];

  /* Math: '<S202>/Transpose1' incorporates:
   *  Concatenate: '<S101>/Vector Concatenate'
   *  Math: '<S220>/Transpose'
   */
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    rtb_Transpose_n[3 * rtb_phidot_tmp] = rtb_VectorConcatenate_i[rtb_phidot_tmp];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 1] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 3];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 2] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 6];
  }

  /* End of Math: '<S202>/Transpose1' */
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Selector: '<S157>/Selector1' */
    Chassis_B.Selector1_l[0] = Chassis_B.HPbar[9];
    Chassis_B.Selector1_l[1] = Chassis_B.HPbar[10];
    Chassis_B.Selector1_l[2] = Chassis_B.HPbar[11];
  }

  /* Sum: '<S206>/Sum' incorporates:
   *  Integrator: '<S109>/p,q,r '
   *  Product: '<S208>/i x j'
   *  Product: '<S208>/j x k'
   *  Product: '<S208>/k x i'
   *  Product: '<S209>/i x k'
   *  Product: '<S209>/j x i'
   *  Product: '<S209>/k x j'
   */
  rtb_Add1_p[0] =
      Chassis_X.pqr_CSTATE[1] * Chassis_B.Selector1_l[2] - Chassis_B.Selector1_l[1] * Chassis_X.pqr_CSTATE[2];
  rtb_Add1_p[1] =
      Chassis_B.Selector1_l[0] * Chassis_X.pqr_CSTATE[2] - Chassis_X.pqr_CSTATE[0] * Chassis_B.Selector1_l[2];
  rtb_Add1_p[2] =
      Chassis_X.pqr_CSTATE[0] * Chassis_B.Selector1_l[1] - Chassis_B.Selector1_l[0] * Chassis_X.pqr_CSTATE[1];
  for (iy = 0; iy < 3; iy++) {
    /* Product: '<S205>/Product' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S204>/Product'
     */
    rtb_Vel_Xdot = rtb_Transpose_n[iy + 3];
    rtb_Vel_Ydot = rtb_Transpose_n[iy + 6];

    /* Sum: '<S202>/Add' incorporates:
     *  Integrator: '<S109>/xe,ye,ze'
     *  Math: '<S220>/Transpose'
     *  Product: '<S205>/Product'
     *  Selector: '<S157>/Selector1'
     */
    rtb_Add_cm[iy] = ((rtb_Vel_Xdot * Chassis_B.Selector1_l[1] + rtb_Transpose_n[iy] * Chassis_B.Selector1_l[0]) +
                      rtb_Vel_Ydot * Chassis_B.Selector1_l[2]) +
                     Chassis_X.xeyeze_CSTATE[iy];

    /* Trigonometry: '<S187>/sincos' */
    rtb_UnitConversion3_h = rtb_UnitConversion3[iy];

    /* Sum: '<S202>/Add4' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S204>/Product'
     *  Sum: '<S202>/Add1'
     */
    rtb_UnaryMinus1[iy] =
        ((rtb_Vel_Xdot * rtb_Add1_p[1] + rtb_Transpose_n[iy] * rtb_Add1_p[0]) + rtb_Vel_Ydot * rtb_Add1_p[2]) +
        Chassis_B.Product[iy];

    /* Trigonometry: '<S187>/sincos' */
    rtb_Sum_eb[iy] = std::sin(rtb_UnitConversion3_h);
    rtb_Add1_l[iy] = std::cos(rtb_UnitConversion3_h);
  }

  /* Fcn: '<S187>/Fcn11' */
  rtb_VectorConcatenate_i[0] = rtb_Add1_l[0] * rtb_Add1_l[1];

  /* Fcn: '<S187>/Fcn21' incorporates:
   *  Fcn: '<S187>/Fcn22'
   */
  rtb_thetadot = rtb_Sum_eb[1] * rtb_Sum_eb[2];
  rtb_VectorConcatenate_i[1] = rtb_thetadot * rtb_Add1_l[0] - rtb_Sum_eb[0] * rtb_Add1_l[2];

  /* Fcn: '<S187>/Fcn31' incorporates:
   *  Fcn: '<S187>/Fcn32'
   */
  rtb_UnaryMinus_i = rtb_Sum_eb[1] * rtb_Add1_l[2];
  rtb_VectorConcatenate_i[2] = rtb_UnaryMinus_i * rtb_Add1_l[0] + rtb_Sum_eb[0] * rtb_Sum_eb[2];

  /* Fcn: '<S187>/Fcn12' */
  rtb_VectorConcatenate_i[3] = rtb_Sum_eb[0] * rtb_Add1_l[1];

  /* Fcn: '<S187>/Fcn22' */
  rtb_VectorConcatenate_i[4] = rtb_thetadot * rtb_Sum_eb[0] + rtb_Add1_l[0] * rtb_Add1_l[2];

  /* Fcn: '<S187>/Fcn32' */
  rtb_VectorConcatenate_i[5] = rtb_UnaryMinus_i * rtb_Sum_eb[0] - rtb_Add1_l[0] * rtb_Sum_eb[2];

  /* Fcn: '<S187>/Fcn13' */
  rtb_VectorConcatenate_i[6] = -rtb_Sum_eb[1];

  /* Fcn: '<S187>/Fcn23' */
  rtb_VectorConcatenate_i[7] = rtb_Add1_l[1] * rtb_Sum_eb[2];

  /* Fcn: '<S187>/Fcn33' */
  rtb_VectorConcatenate_i[8] = rtb_Add1_l[1] * rtb_Add1_l[2];

  /* Math: '<S186>/Transpose1' incorporates:
   *  Concatenate: '<S101>/Vector Concatenate'
   *  Math: '<S220>/Transpose'
   */
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    rtb_Transpose_n[3 * rtb_phidot_tmp] = rtb_VectorConcatenate_i[rtb_phidot_tmp];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 1] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 3];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 2] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 6];
  }

  /* End of Math: '<S186>/Transpose1' */
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Sum: '<S149>/Sum4' */
    rtb_Product2_c = Chassis_B.Rbar[0];

    /* UnaryMinus: '<S155>/Unary Minus2' incorporates:
     *  Concatenate: '<S155>/Vector Concatenate'
     *  Sum: '<S149>/Sum4'
     */
    Chassis_B.VectorConcatenate[0] = -Chassis_B.Rbar[0];

    /* Sum: '<S149>/Sum3' incorporates:
     *  Constant: '<S149>/Constant'
     */
    rtb_Product1_l = (0.0 - Chassis_P.VehicleBody6DOF_d) - Chassis_B.Rbar[1];

    /* SignalConversion generated from: '<S155>/Vector Concatenate' incorporates:
     *  Concatenate: '<S155>/Vector Concatenate'
     */
    Chassis_B.VectorConcatenate[1] = rtb_Product1_l;

    /* Sum: '<S149>/Sum1' incorporates:
     *  Constant: '<S149>/Constant1'
     */
    rtb_Product_c = Chassis_P.VEH.HeightCG - Chassis_B.Rbar[2];

    /* SignalConversion generated from: '<S155>/Vector Concatenate' incorporates:
     *  Concatenate: '<S155>/Vector Concatenate'
     */
    Chassis_B.VectorConcatenate[2] = rtb_Product_c;
  }

  for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= 0; rtb_phidot_tmp += 2) {
    /* Product: '<S189>/Product' incorporates:
     *  Math: '<S220>/Transpose'
     */
    tmp_5 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp]);
    tmp_6 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp + 3]);
    tmp_3 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp + 6]);

    /* Integrator: '<S109>/xe,ye,ze' incorporates:
     *  Product: '<S189>/Product'
     */
    tmp_4 = _mm_loadu_pd(&Chassis_X.xeyeze_CSTATE[rtb_phidot_tmp]);

    /* Sum: '<S186>/Add' incorporates:
     *  Concatenate: '<S155>/Vector Concatenate'
     *  Product: '<S189>/Product'
     */
    _mm_storeu_pd(
        &rtb_Add1_l[rtb_phidot_tmp],
        _mm_add_pd(tmp_4,
                   _mm_add_pd(_mm_mul_pd(tmp_3, _mm_set1_pd(Chassis_B.VectorConcatenate[2])),
                              _mm_add_pd(_mm_mul_pd(tmp_6, _mm_set1_pd(Chassis_B.VectorConcatenate[1])),
                                         _mm_add_pd(_mm_mul_pd(tmp_5, _mm_set1_pd(Chassis_B.VectorConcatenate[0])),
                                                    _mm_set1_pd(0.0))))));
  }

  for (rtb_phidot_tmp = 2; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    /* Sum: '<S186>/Add' incorporates:
     *  Concatenate: '<S155>/Vector Concatenate'
     *  Integrator: '<S109>/xe,ye,ze'
     *  Math: '<S220>/Transpose'
     *  Product: '<S189>/Product'
     */
    rtb_Add1_l[rtb_phidot_tmp] = ((rtb_Transpose_n[rtb_phidot_tmp + 3] * Chassis_B.VectorConcatenate[1] +
                                   rtb_Transpose_n[rtb_phidot_tmp] * Chassis_B.VectorConcatenate[0]) +
                                  rtb_Transpose_n[rtb_phidot_tmp + 6] * Chassis_B.VectorConcatenate[2]) +
                                 Chassis_X.xeyeze_CSTATE[rtb_phidot_tmp];
  }

  /* BusCreator generated from: '<Root>/VehBody' */
  rtb_UnitConversion3_h = rtb_Add1_l[0];
  rtb_Hitch_Disp_Y = rtb_Add1_l[1];
  rtb_Hitch_Disp_Z = rtb_Add1_l[2];

  /* Sum: '<S190>/Sum' incorporates:
   *  Integrator: '<S109>/p,q,r '
   *  Product: '<S192>/i x j'
   *  Product: '<S192>/j x k'
   *  Product: '<S192>/k x i'
   *  Product: '<S193>/i x k'
   *  Product: '<S193>/j x i'
   *  Product: '<S193>/k x j'
   */
  rtb_Add1_l[0] = Chassis_X.pqr_CSTATE[1] * Chassis_B.VectorConcatenate[2] -
                  Chassis_B.VectorConcatenate[1] * Chassis_X.pqr_CSTATE[2];
  rtb_Add1_l[1] = Chassis_B.VectorConcatenate[0] * Chassis_X.pqr_CSTATE[2] -
                  Chassis_X.pqr_CSTATE[0] * Chassis_B.VectorConcatenate[2];
  rtb_Add1_l[2] = Chassis_X.pqr_CSTATE[0] * Chassis_B.VectorConcatenate[1] -
                  Chassis_B.VectorConcatenate[0] * Chassis_X.pqr_CSTATE[1];
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= 0; rtb_phidot_tmp += 2) {
    /* Product: '<S188>/Product' incorporates:
     *  Math: '<S220>/Transpose'
     */
    tmp_5 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp]);
    tmp_6 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp + 3]);
    tmp_3 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp + 6]);

    /* Sum: '<S186>/Add4' incorporates:
     *  Product: '<S188>/Product'
     *  Sum: '<S186>/Add1'
     */
    tmp_4 = _mm_loadu_pd(&Chassis_B.Product[rtb_phidot_tmp]);
    _mm_storeu_pd(&rtb_Sum_eb[rtb_phidot_tmp],
                  _mm_add_pd(tmp_4, _mm_add_pd(_mm_mul_pd(tmp_3, _mm_set1_pd(rtb_Add1_l[2])),
                                               _mm_add_pd(_mm_mul_pd(tmp_6, _mm_set1_pd(rtb_Add1_l[1])),
                                                          _mm_add_pd(_mm_mul_pd(tmp_5, _mm_set1_pd(rtb_Add1_l[0])),
                                                                     _mm_set1_pd(0.0))))));
  }

  for (rtb_phidot_tmp = 2; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    /* Sum: '<S186>/Add4' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S188>/Product'
     *  Sum: '<S186>/Add1'
     */
    rtb_Sum_eb[rtb_phidot_tmp] =
        ((rtb_Transpose_n[rtb_phidot_tmp + 3] * rtb_Add1_l[1] + rtb_Transpose_n[rtb_phidot_tmp] * rtb_Add1_l[0]) +
         rtb_Transpose_n[rtb_phidot_tmp + 6] * rtb_Add1_l[2]) +
        Chassis_B.Product[rtb_phidot_tmp];
  }

  /* BusCreator generated from: '<Root>/VehBody' */
  rtb_Hitch_Vel_Xdot = rtb_Sum_eb[0];
  rtb_Hitch_Vel_Ydot = rtb_Sum_eb[1];
  rtb_Hitch_Vel_Zdot = rtb_Sum_eb[2];

  /* Fcn: '<S176>/Fcn11' */
  rtb_VectorConcatenate_i[0] = Drphi_0;

  /* Fcn: '<S176>/Fcn21' */
  rtb_VectorConcatenate_i[1] = rtb_thetadot_tmp * rtb_Add_f_tmp - rtb_VectorConcatenate_b_tmp;

  /* Fcn: '<S176>/Fcn31' */
  rtb_VectorConcatenate_i[2] = rtb_UnaryMinus_d_tmp * rtb_Add_f_tmp + rtb_VectorConcatenate_b_tmp_0;

  /* Fcn: '<S176>/Fcn12' */
  rtb_VectorConcatenate_i[3] = rtb_phidot;

  /* Fcn: '<S176>/Fcn22' */
  rtb_VectorConcatenate_i[4] = rtb_thetadot_tmp * rtb_pqr_tmp_0 + rtb_Beta;

  /* Fcn: '<S176>/Fcn32' */
  rtb_VectorConcatenate_i[5] = rtb_UnaryMinus_d_tmp * rtb_pqr_tmp_0 - rtb_VectorConcatenate_b_tmp_1;

  /* Fcn: '<S176>/Fcn13' */
  rtb_VectorConcatenate_i[6] = -rtb_Integrator1_idx_0;

  /* Fcn: '<S176>/Fcn23' */
  rtb_VectorConcatenate_i[7] = rtb_VectorConcatenate_b_tmp_2;

  /* Fcn: '<S176>/Fcn33' */
  rtb_VectorConcatenate_i[8] = rtb_VectorConcatenate_b_tmp_3;

  /* Math: '<S174>/Transpose1' incorporates:
   *  Concatenate: '<S101>/Vector Concatenate'
   *  Math: '<S220>/Transpose'
   */
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    rtb_Transpose_n[3 * rtb_phidot_tmp] = rtb_VectorConcatenate_i[rtb_phidot_tmp];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 1] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 3];
    rtb_Transpose_n[3 * rtb_phidot_tmp + 2] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 6];
  }

  /* End of Math: '<S174>/Transpose1' */
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Sum: '<S154>/Subtract' incorporates:
     *  Constant: '<S154>/latOff'
     *  Constant: '<S154>/longOff'
     *  Constant: '<S154>/vertOff '
     */
    Chassis_B.Subtract[0] = Chassis_P.VehicleBody6DOF_longOff + Chassis_B.Rbar[0];
    Chassis_B.Subtract[1] = Chassis_P.VehicleBody6DOF_latOff + Chassis_B.Rbar[1];
    Chassis_B.Subtract[2] = Chassis_P.VehicleBody6DOF_vertOff + Chassis_B.Rbar[2];
  }

  /* Sum: '<S179>/Sum' incorporates:
   *  Integrator: '<S109>/p,q,r '
   *  Product: '<S184>/i x j'
   *  Product: '<S184>/j x k'
   *  Product: '<S184>/k x i'
   *  Product: '<S185>/i x k'
   *  Product: '<S185>/j x i'
   *  Product: '<S185>/k x j'
   */
  rtb_Sum_eb[0] = Chassis_X.pqr_CSTATE[1] * Chassis_B.Subtract[2] - Chassis_B.Subtract[1] * Chassis_X.pqr_CSTATE[2];
  rtb_Sum_eb[1] = Chassis_B.Subtract[0] * Chassis_X.pqr_CSTATE[2] - Chassis_X.pqr_CSTATE[0] * Chassis_B.Subtract[2];
  rtb_Sum_eb[2] = Chassis_X.pqr_CSTATE[0] * Chassis_B.Subtract[1] - Chassis_B.Subtract[0] * Chassis_X.pqr_CSTATE[1];
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= 0; rtb_phidot_tmp += 2) {
    /* Product: '<S178>/Product' incorporates:
     *  Math: '<S220>/Transpose'
     */
    tmp_5 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp]);
    tmp_6 = _mm_set1_pd(0.0);
    tmp_3 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp + 3]);
    tmp_4 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp + 6]);

    /* Integrator: '<S109>/xe,ye,ze' incorporates:
     *  Product: '<S178>/Product'
     */
    tmp_2 = _mm_loadu_pd(&Chassis_X.xeyeze_CSTATE[rtb_phidot_tmp]);

    /* Sum: '<S174>/Add' incorporates:
     *  Product: '<S178>/Product'
     *  Sum: '<S154>/Subtract'
     */
    _mm_storeu_pd(
        &rtb_Add[rtb_phidot_tmp],
        _mm_add_pd(tmp_2,
                   _mm_add_pd(_mm_mul_pd(tmp_4, _mm_set1_pd(Chassis_B.Subtract[2])),
                              _mm_add_pd(_mm_mul_pd(tmp_3, _mm_set1_pd(Chassis_B.Subtract[1])),
                                         _mm_add_pd(_mm_mul_pd(tmp_5, _mm_set1_pd(Chassis_B.Subtract[0])), tmp_6)))));

    /* Product: '<S178>/Product' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S177>/Product'
     */
    tmp_5 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp]);
    tmp_3 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp + 3]);
    tmp_4 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp + 6]);

    /* Sum: '<S174>/Add4' incorporates:
     *  Product: '<S177>/Product'
     *  Product: '<S178>/Product'
     *  Sum: '<S154>/Subtract'
     *  Sum: '<S179>/Sum'
     */
    tmp_2 = _mm_loadu_pd(&Chassis_B.Product[rtb_phidot_tmp]);
    _mm_storeu_pd(
        &rtb_UnitConversion3[rtb_phidot_tmp],
        _mm_add_pd(tmp_2, _mm_add_pd(_mm_mul_pd(tmp_4, _mm_set1_pd(rtb_Sum_eb[2])),
                                     _mm_add_pd(_mm_mul_pd(tmp_3, _mm_set1_pd(rtb_Sum_eb[1])),
                                                _mm_add_pd(_mm_mul_pd(tmp_5, _mm_set1_pd(rtb_Sum_eb[0])), tmp_6)))));
  }

  for (rtb_phidot_tmp = 2; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    /* Product: '<S178>/Product' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S177>/Product'
     */
    rtb_Vel_Xdot = rtb_Transpose_n[rtb_phidot_tmp + 3];
    rtb_Vel_Ydot = rtb_Transpose_n[rtb_phidot_tmp + 6];

    /* Sum: '<S174>/Add' incorporates:
     *  Integrator: '<S109>/xe,ye,ze'
     *  Math: '<S220>/Transpose'
     *  Product: '<S178>/Product'
     *  Sum: '<S154>/Subtract'
     */
    rtb_Add[rtb_phidot_tmp] =
        ((rtb_Vel_Xdot * Chassis_B.Subtract[1] + rtb_Transpose_n[rtb_phidot_tmp] * Chassis_B.Subtract[0]) +
         rtb_Vel_Ydot * Chassis_B.Subtract[2]) +
        Chassis_X.xeyeze_CSTATE[rtb_phidot_tmp];

    /* Sum: '<S174>/Add4' incorporates:
     *  Math: '<S220>/Transpose'
     *  Product: '<S177>/Product'
     *  Sum: '<S179>/Sum'
     */
    rtb_UnitConversion3[rtb_phidot_tmp] =
        ((rtb_Vel_Xdot * rtb_Sum_eb[1] + rtb_Transpose_n[rtb_phidot_tmp] * rtb_Sum_eb[0]) +
         rtb_Vel_Ydot * rtb_Sum_eb[2]) +
        Chassis_B.Product[rtb_phidot_tmp];
  }

  /* BusCreator generated from: '<Root>/VehBody' */
  rtb_Vel_Xdot = rtb_UnitConversion3[0];
  rtb_Vel_Ydot = rtb_UnitConversion3[1];
  rtb_thetadot_tmp = rtb_UnitConversion3[2];

  /* BusCreator generated from: '<Root>/VehBody' */
  rtb_VectorConcatenate_b_tmp = Chassis_B.ubvbwb[0];
  rtb_UnaryMinus_d_tmp = Chassis_B.ubvbwb[1];
  rtb_VectorConcatenate_b_tmp_0 = Chassis_B.ubvbwb[2];

  /* Switch: '<S144>/Switch' incorporates:
   *  Abs: '<S144>/Abs'
   *  Constant: '<S145>/Constant'
   *  Constant: '<S146>/Constant'
   *  Fcn: '<S144>/Fcn'
   *  Logic: '<S144>/Logical Operator'
   *  RelationalOperator: '<S145>/Compare'
   *  RelationalOperator: '<S146>/Compare'
   */
  if ((Chassis_B.ubvbwb[0] >= -Chassis_P.VehicleBody6DOF_xdot_tol) &&
      (Chassis_B.ubvbwb[0] <= Chassis_P.VehicleBody6DOF_xdot_tol)) {
    rtb_phidot = 0.2 / (3.0 - rt_powd_snf(Chassis_B.ubvbwb[0] / 0.1, 2.0));
  } else {
    rtb_phidot = std::abs(Chassis_B.ubvbwb[0]);
  }

  /* End of Switch: '<S144>/Switch' */

  /* Trigonometry: '<S138>/Trigonometric Function' incorporates:
   *  Product: '<S138>/Divide'
   */
  rtb_Beta = std::atan(Chassis_B.ubvbwb[1] / rtb_phidot);

  /* BusCreator generated from: '<Root>/VehBody' incorporates:
   *  Integrator: '<S109>/p,q,r '
   */
  rtb_VectorConcatenate_b_tmp_1 = Chassis_X.pqr_CSTATE[0];
  rtb_VectorConcatenate_b_tmp_2 = Chassis_X.pqr_CSTATE[1];
  rtb_VectorConcatenate_b_tmp_3 = Chassis_X.pqr_CSTATE[2];

  /* Fcn: '<S139>/thetadot' incorporates:
   *  Integrator: '<S109>/p,q,r '
   */
  rtb_phidot = rtb_UnaryMinus_h_idx_0 * Chassis_X.pqr_CSTATE[1] - rtb_pqr_tmp * Chassis_X.pqr_CSTATE[2];

  /* Fcn: '<S139>/psidot' incorporates:
   *  Fcn: '<S139>/phidot'
   *  Integrator: '<S109>/p,q,r '
   */
  rtb_pqr_tmp = rtb_pqr_tmp * Chassis_X.pqr_CSTATE[1] + rtb_UnaryMinus_h_idx_0 * Chassis_X.pqr_CSTATE[2];
  rtb_psidot = rtb_pqr_tmp / rtb_Integrator1_idx_1;

  /* Fcn: '<S139>/phidot' incorporates:
   *  Integrator: '<S109>/p,q,r '
   */
  rtb_ixk = rtb_Integrator1_idx_0 / rtb_Integrator1_idx_1 * rtb_pqr_tmp + Chassis_X.pqr_CSTATE[0];

  /* Sum: '<S140>/Sum' incorporates:
   *  Product: '<S147>/i x j'
   *  Product: '<S147>/j x k'
   *  Product: '<S147>/k x i'
   *  Product: '<S148>/i x k'
   *  Product: '<S148>/j x i'
   *  Product: '<S148>/k x j'
   */
  rtb_UnitConversion3[0] = rtb_phidot * Chassis_B.ubvbwb[2] - rtb_psidot * Chassis_B.ubvbwb[1];
  rtb_UnitConversion3[1] = rtb_psidot * Chassis_B.ubvbwb[0] - rtb_ixk * Chassis_B.ubvbwb[2];
  rtb_UnitConversion3[2] = rtb_ixk * Chassis_B.ubvbwb[1] - rtb_phidot * Chassis_B.ubvbwb[0];

  /* Integrator: '<S95>/Integrator1' */
  rtb_Integrator1_idx_0 = Chassis_X.Integrator1_CSTATE[0];

  /* Gain: '<S93>/Unsprung Masses' incorporates:
   *  Concatenate: '<S93>/Vector Concatenate'
   *  Integrator: '<S95>/Integrator1'
   */
  Chassis_B.VectorConcatenate_p[0] = Chassis_P.UnsprungMasses_Gain * Chassis_X.Integrator1_CSTATE[0];

  /* Integrator: '<S95>/Integrator1' */
  rtb_Integrator1_idx_1 = Chassis_X.Integrator1_CSTATE[1];

  /* Gain: '<S93>/Unsprung Masses' incorporates:
   *  Concatenate: '<S93>/Vector Concatenate'
   *  Integrator: '<S95>/Integrator1'
   */
  Chassis_B.VectorConcatenate_p[1] = Chassis_P.UnsprungMasses_Gain * Chassis_X.Integrator1_CSTATE[1];
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Constant: '<S93>/Vertical' incorporates:
     *  Concatenate: '<S93>/Vector Concatenate'
     */
    Chassis_B.VectorConcatenate_p[2] = Chassis_P.Vertical_Value;

    /* Product: '<S111>/Product' incorporates:
     *  Constant: '<S111>/0'
     */
    rtb_pqr_tmp = Chassis_P.u_Value * Chassis_B.Mbar;

    /* Product: '<S111>/Product' incorporates:
     *  Constant: '<S111>/g'
     */
    Chassis_B.Fg_I[0] = rtb_pqr_tmp;
    Chassis_B.Fg_I[1] = rtb_pqr_tmp;
    Chassis_B.Fg_I[2] = Chassis_P.VehicleBody6DOF_g * Chassis_B.Mbar;
  }

  /* UnaryMinus: '<S93>/Unary Minus' */
  rtb_UnaryMinus_h_idx_0 = -Chassis_B.VectorConcatenate_p[0];
  rtb_UnaryMinus_h_idx_1 = -Chassis_B.VectorConcatenate_p[1];
  rtb_UnaryMinus_h_idx_2 = -Chassis_B.VectorConcatenate_p[2];
  for (Itemp_tmp = 0; Itemp_tmp < 3; Itemp_tmp++) {
    /* Product: '<S111>/Inertial to Body' incorporates:
     *  Concatenate: '<S125>/Vector Concatenate'
     */
    rtb_pqr_tmp = rtb_VectorConcatenate[Itemp_tmp];

    /* Sum: '<S136>/Add1' incorporates:
     *  Inport: '<Root>/Env'
     *  Product: '<S135>/Inertial to Body'
     */
    Drphi_0 = rtb_pqr_tmp * Chassis_U.Env.Wind[0];

    /* Product: '<S111>/Inertial to Body' incorporates:
     *  Concatenate: '<S125>/Vector Concatenate'
     */
    rtb_Integrator1_h_idx_0 = rtb_pqr_tmp * Chassis_B.Fg_I[0];
    rtb_pqr_tmp = rtb_VectorConcatenate[Itemp_tmp + 3];

    /* Sum: '<S136>/Add1' incorporates:
     *  Inport: '<Root>/Env'
     *  Product: '<S135>/Inertial to Body'
     */
    Drphi_0 += rtb_pqr_tmp * Chassis_U.Env.Wind[1];

    /* Product: '<S111>/Inertial to Body' incorporates:
     *  Concatenate: '<S125>/Vector Concatenate'
     */
    rtb_Integrator1_h_idx_0 += rtb_pqr_tmp * Chassis_B.Fg_I[1];
    rtb_pqr_tmp = rtb_VectorConcatenate[Itemp_tmp + 6];

    /* Sum: '<S136>/Add1' incorporates:
     *  Inport: '<Root>/Env'
     *  Product: '<S135>/Inertial to Body'
     */
    Fzo_prime_idx_2 = Chassis_B.ubvbwb[Itemp_tmp] - (rtb_pqr_tmp * Chassis_U.Env.Wind[2] + Drphi_0);

    /* Trigonometry: '<S136>/Tanh' incorporates:
     *  Gain: '<S136>/4'
     */
    rtb_sincos_o2_a[Itemp_tmp] = std::tanh(Chassis_P.u_Gain[Itemp_tmp] * Fzo_prime_idx_2);

    /* Product: '<S136>/Product' */
    rtb_Product[Itemp_tmp] = Fzo_prime_idx_2 * Fzo_prime_idx_2;

    /* Sum: '<S136>/Add1' incorporates:
     *  Product: '<S135>/Inertial to Body'
     */
    rtb_Add_ms[Itemp_tmp] = Fzo_prime_idx_2;

    /* Product: '<S111>/Inertial to Body' */
    rtb_Fg_B[Itemp_tmp] = rtb_pqr_tmp * Chassis_B.Fg_I[2] + rtb_Integrator1_h_idx_0;
  }

  /* Sqrt: '<S136>/Sqrt' incorporates:
   *  Sum: '<S136>/Sum of Elements'
   */
  rtb_phidot = std::sqrt((rtb_Product[0] + rtb_Product[1]) + rtb_Product[2]);

  /* Product: '<S136>/Product2' */
  rtb_thetadot = rtb_phidot * rtb_phidot;
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Constant: '<S136>/Constant' incorporates:
     *  Concatenate: '<S136>/Vector Concatenate'
     */
    Chassis_B.VectorConcatenate_k[0] = Chassis_P.VehicleBody6DOF_Cd;
  }

  /* Trigonometry: '<S136>/Trigonometric Function' */
  rtb_phidot = rt_atan2d_snf(rtb_Add_ms[1], rtb_Add_ms[0]);

  /* Lookup_n-D: '<S136>/Cs' incorporates:
   *  Concatenate: '<S136>/Vector Concatenate'
   *  Fcn: '<S124>/phidot'
   */
  Chassis_B.VectorConcatenate_k[1] =
      look1_binlcpw(rtb_phidot, Chassis_P.VehicleBody6DOF_beta_w, Chassis_P.VehicleBody6DOF_Cs, m_beta_w_cs_maxIndex);
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Constant: '<S136>/Constant1' incorporates:
     *  Concatenate: '<S136>/Vector Concatenate'
     */
    Chassis_B.VectorConcatenate_k[2] = Chassis_P.VehicleBody6DOF_Cl;
  }

  /* Lookup_n-D: '<S136>/Crm' incorporates:
   *  Concatenate: '<S136>/Vector Concatenate'
   *  Fcn: '<S124>/phidot'
   */
  Chassis_B.VectorConcatenate_k[3] = look1_binlxpw(rtb_phidot, Chassis_P.Crm_bp01Data, Chassis_P.Crm_tableData, 1U);

  /* Product: '<S136>/Product5' incorporates:
   *  Concatenate: '<S136>/Vector Concatenate'
   *  Constant: '<S136>/Constant2'
   */
  Chassis_B.VectorConcatenate_k[4] = rtb_sincos_o2_a[0] * Chassis_P.VehicleBody6DOF_Cpm;

  /* Lookup_n-D: '<S136>/Cym' incorporates:
   *  Concatenate: '<S136>/Vector Concatenate'
   *  Fcn: '<S124>/phidot'
   */
  Chassis_B.VectorConcatenate_k[5] =
      look1_binlxpw(rtb_phidot, Chassis_P.VehicleBody6DOF_beta_w, Chassis_P.VehicleBody6DOF_Cym, m_beta_w_cym_maxIndex);

  /* Gain: '<S136>/.5.*A.*Pabs.  // R.  // T' */
  rtb_UnaryMinus_i = 0.5 * Chassis_P.VEH.FrontalArea * Chassis_P.VehicleBody6DOF_Pabs / Chassis_P.DragForce_R;
  for (Itemp_tmp = 0; Itemp_tmp <= 4; Itemp_tmp += 2) {
    /* Product: '<S136>/Product1' incorporates:
     *  Gain: '<S136>/.5.*A.*Pabs.//R.//T'
     */
    tmp_5 = _mm_loadu_pd(&Chassis_B.VectorConcatenate_k[Itemp_tmp]);

    /* Gain: '<S136>/.5.*A.*Pabs.//R.//T' incorporates:
     *  Constant: '<S94>/AirTempConstant'
     *  Product: '<S136>/Product1'
     */
    _mm_storeu_pd(&rtb_Abs1[Itemp_tmp], _mm_mul_pd(_mm_div_pd(_mm_mul_pd(_mm_set1_pd(rtb_thetadot), tmp_5),
                                                              _mm_set1_pd(Chassis_P.VehicleBody6DOF_Tair)),
                                                   _mm_set1_pd(rtb_UnaryMinus_i)));
  }

  /* Product: '<S136>/Product3' incorporates:
   *  Constant: '<S136>/Constant4'
   *  Sum: '<S136>/Add2'
   */
  rtb_Product[0] = (rtb_sincos_o2_a[0] - Chassis_P.Constant4_Value[0]) * rtb_Abs1[0];
  rtb_Product[1] = (rtb_sincos_o2_a[1] - Chassis_P.Constant4_Value[1]) * rtb_Abs1[1];
  rtb_Product[2] = (rtb_sincos_o2_a[2] - Chassis_P.Constant4_Value[2]) * rtb_Abs1[2];

  /* SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  rtb_Switch[0] = Chassis_X.IntegratorSecondOrder_CSTATE[4];

  /* UnaryMinus: '<S217>/Unary Minus1' incorporates:
   *  SecondOrderIntegrator: '<S230>/Integrator, Second-Order'
   */
  rtb_z_idx_0 = -Chassis_X.IntegratorSecondOrder_CSTATE[0];

  /* Integrator: '<S232>/Integrator' */
  rtb_Integrator[0] = Chassis_X.Integrator_CSTATE[0];

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' incorporates:
   *  Integrator: '<S232>/Integrator'
   */
  rtb_VectorConcatenate8[0] = Chassis_X.Integrator_CSTATE[0];

  /* Integrator: '<S233>/Integrator' */
  rtb_Integrator_g[0] = Chassis_X.Integrator_CSTATE_c[0];

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' incorporates:
   *  Integrator: '<S233>/Integrator'
   */
  rtb_VectorConcatenate8[4] = Chassis_X.Integrator_CSTATE_c[0];

  /* Integrator: '<S210>/Integrator1' */
  rtb_Integrator1_h_idx_0 = Chassis_X.Integrator1_CSTATE_h[0];

  /* Saturate: '<S4>/Saturation' incorporates:
   *  Integrator: '<S210>/Integrator1'
   */
  if (Chassis_X.Integrator1_CSTATE_h[0] > Chassis_P.Saturation_UpperSat) {
    rtb_pqr_tmp = Chassis_P.Saturation_UpperSat;
  } else if (Chassis_X.Integrator1_CSTATE_h[0] < Chassis_P.Saturation_LowerSat) {
    rtb_pqr_tmp = Chassis_P.Saturation_LowerSat;
  } else {
    rtb_pqr_tmp = Chassis_X.Integrator1_CSTATE_h[0];
  }

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' */
  rtb_VectorConcatenate8[8] = rtb_pqr_tmp;

  /* SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  rtb_IntegratorSecondOrder_o1[0] = Chassis_X.IntegratorSecondOrder_CSTATE[0];

  /* Saturate: '<S4>/Saturation' */
  rtb_Saturation_h2[0] = rtb_pqr_tmp;

  /* SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  rtb_Switch[1] = Chassis_X.IntegratorSecondOrder_CSTATE[5];

  /* UnaryMinus: '<S217>/Unary Minus1' incorporates:
   *  SecondOrderIntegrator: '<S230>/Integrator, Second-Order'
   */
  rtb_z_idx_1 = -Chassis_X.IntegratorSecondOrder_CSTATE[1];

  /* Integrator: '<S232>/Integrator' */
  rtb_Integrator[1] = Chassis_X.Integrator_CSTATE[1];

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' incorporates:
   *  Integrator: '<S232>/Integrator'
   */
  rtb_VectorConcatenate8[1] = Chassis_X.Integrator_CSTATE[1];

  /* Integrator: '<S233>/Integrator' */
  rtb_Integrator_g[1] = Chassis_X.Integrator_CSTATE_c[1];

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' incorporates:
   *  Integrator: '<S233>/Integrator'
   */
  rtb_VectorConcatenate8[5] = Chassis_X.Integrator_CSTATE_c[1];

  /* Integrator: '<S210>/Integrator1' */
  rtb_Integrator1_h_idx_1 = Chassis_X.Integrator1_CSTATE_h[1];

  /* Saturate: '<S4>/Saturation' incorporates:
   *  Integrator: '<S210>/Integrator1'
   */
  if (Chassis_X.Integrator1_CSTATE_h[1] > Chassis_P.Saturation_UpperSat) {
    rtb_pqr_tmp = Chassis_P.Saturation_UpperSat;
  } else if (Chassis_X.Integrator1_CSTATE_h[1] < Chassis_P.Saturation_LowerSat) {
    rtb_pqr_tmp = Chassis_P.Saturation_LowerSat;
  } else {
    rtb_pqr_tmp = Chassis_X.Integrator1_CSTATE_h[1];
  }

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' */
  rtb_VectorConcatenate8[9] = rtb_pqr_tmp;

  /* SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  rtb_IntegratorSecondOrder_o1[1] = Chassis_X.IntegratorSecondOrder_CSTATE[1];

  /* Saturate: '<S4>/Saturation' */
  rtb_Saturation_h2[1] = rtb_pqr_tmp;

  /* SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  rtb_Switch[2] = Chassis_X.IntegratorSecondOrder_CSTATE[6];

  /* UnaryMinus: '<S217>/Unary Minus1' incorporates:
   *  SecondOrderIntegrator: '<S230>/Integrator, Second-Order'
   */
  rtb_z_idx_2 = -Chassis_X.IntegratorSecondOrder_CSTATE[2];

  /* Integrator: '<S232>/Integrator' */
  rtb_Integrator[2] = Chassis_X.Integrator_CSTATE[2];

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' incorporates:
   *  Integrator: '<S232>/Integrator'
   */
  rtb_VectorConcatenate8[2] = Chassis_X.Integrator_CSTATE[2];

  /* Integrator: '<S233>/Integrator' */
  rtb_Integrator_g[2] = Chassis_X.Integrator_CSTATE_c[2];

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' incorporates:
   *  Integrator: '<S233>/Integrator'
   */
  rtb_VectorConcatenate8[6] = Chassis_X.Integrator_CSTATE_c[2];

  /* Integrator: '<S210>/Integrator1' */
  rtb_Integrator1_h_idx_2 = Chassis_X.Integrator1_CSTATE_h[2];

  /* Saturate: '<S4>/Saturation' incorporates:
   *  Integrator: '<S210>/Integrator1'
   */
  if (Chassis_X.Integrator1_CSTATE_h[2] > Chassis_P.Saturation_UpperSat) {
    rtb_pqr_tmp = Chassis_P.Saturation_UpperSat;
  } else if (Chassis_X.Integrator1_CSTATE_h[2] < Chassis_P.Saturation_LowerSat) {
    rtb_pqr_tmp = Chassis_P.Saturation_LowerSat;
  } else {
    rtb_pqr_tmp = Chassis_X.Integrator1_CSTATE_h[2];
  }

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' */
  rtb_VectorConcatenate8[10] = rtb_pqr_tmp;

  /* SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  rtb_IntegratorSecondOrder_o1[2] = Chassis_X.IntegratorSecondOrder_CSTATE[2];

  /* Saturate: '<S4>/Saturation' */
  rtb_Saturation_h2[2] = rtb_pqr_tmp;

  /* SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  rtb_IntegratorSecondOrder_o1_k = Chassis_X.IntegratorSecondOrder_CSTATE[3];
  rtb_Switch[3] = Chassis_X.IntegratorSecondOrder_CSTATE[7];

  /* Integrator: '<S232>/Integrator' */
  rtb_Integrator[3] = Chassis_X.Integrator_CSTATE[3];

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' incorporates:
   *  Integrator: '<S232>/Integrator'
   */
  rtb_VectorConcatenate8[3] = Chassis_X.Integrator_CSTATE[3];

  /* Integrator: '<S233>/Integrator' */
  rtb_Integrator_g[3] = Chassis_X.Integrator_CSTATE_c[3];

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' incorporates:
   *  Integrator: '<S233>/Integrator'
   */
  rtb_VectorConcatenate8[7] = Chassis_X.Integrator_CSTATE_c[3];

  /* Integrator: '<S210>/Integrator1' */
  rtb_Integrator1_h_idx_3 = Chassis_X.Integrator1_CSTATE_h[3];

  /* Saturate: '<S4>/Saturation' incorporates:
   *  Integrator: '<S210>/Integrator1'
   */
  if (Chassis_X.Integrator1_CSTATE_h[3] > Chassis_P.Saturation_UpperSat) {
    rtb_pqr_tmp = Chassis_P.Saturation_UpperSat;
  } else if (Chassis_X.Integrator1_CSTATE_h[3] < Chassis_P.Saturation_LowerSat) {
    rtb_pqr_tmp = Chassis_P.Saturation_LowerSat;
  } else {
    rtb_pqr_tmp = Chassis_X.Integrator1_CSTATE_h[3];
  }

  /* SignalConversion generated from: '<S213>/Vector Concatenate8' */
  rtb_VectorConcatenate8[11] = rtb_pqr_tmp;

  /* SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  rtb_IntegratorSecondOrder_o1[3] = Chassis_X.IntegratorSecondOrder_CSTATE[3];

  /* Saturate: '<S4>/Saturation' */
  rtb_Saturation_h2[3] = rtb_pqr_tmp;

  /* MATLAB Function: '<S93>/Estimated Unsprung Mass Orientation' incorporates:
   *  SecondOrderIntegrator: '<S230>/Integrator, Second-Order'
   *  UnaryMinus: '<S217>/Unary Minus1'
   */
  rtb_angles_idx_0 =
      std::atan(((-Chassis_X.IntegratorSecondOrder_CSTATE[0] - (-Chassis_X.IntegratorSecondOrder_CSTATE[1])) +
                 (-Chassis_X.IntegratorSecondOrder_CSTATE[2] - (-Chassis_X.IntegratorSecondOrder_CSTATE[3]))) /
                Chassis_P.VEH.TrackWidth / 2.0);
  rtb_angles_idx_1 =
      std::atan(((-Chassis_X.IntegratorSecondOrder_CSTATE[0] - (-Chassis_X.IntegratorSecondOrder_CSTATE[2])) +
                 (-Chassis_X.IntegratorSecondOrder_CSTATE[1] - (-Chassis_X.IntegratorSecondOrder_CSTATE[3]))) /
                (Chassis_P.VEH.FrontAxlePositionfromCG + Chassis_P.VEH.RearAxlePositionfromCG) / 2.0);

  /* Sum: '<S230>/Sum6' incorporates:
   *  Inport: '<Root>/Env'
   *  SecondOrderIntegrator: '<S230>/Integrator, Second-Order'
   */
  rtb_Sum6[0] = Chassis_U.Env.Gnd.z.G_FL_z - Chassis_X.IntegratorSecondOrder_CSTATE[0];
  rtb_Sum6[1] = Chassis_U.Env.Gnd.z.G_FR_z - Chassis_X.IntegratorSecondOrder_CSTATE[1];
  rtb_Sum6[2] = Chassis_U.Env.Gnd.z.G_RL_z - Chassis_X.IntegratorSecondOrder_CSTATE[2];
  rtb_Sum6[3] = Chassis_U.Env.Gnd.z.G_RR_z - Chassis_X.IntegratorSecondOrder_CSTATE[3];

  /* Unit Conversion - from: Pa to: bar
     Expression: output = (1e-05*input) + (0) */
  for (Itemp_tmp = 0; Itemp_tmp < 4; Itemp_tmp++) {
    /* Math: '<S213>/Math Function' incorporates:
     *  Concatenate: '<S213>/Vector Concatenate8'
     */
    Chassis_B.MathFunction[3 * Itemp_tmp] = rtb_VectorConcatenate8[Itemp_tmp];
    Chassis_B.MathFunction[3 * Itemp_tmp + 1] = rtb_VectorConcatenate8[Itemp_tmp + 4];
    Chassis_B.MathFunction[3 * Itemp_tmp + 2] = rtb_VectorConcatenate8[Itemp_tmp + 8];

    /* Integrator: '<S235>/Integrator' */
    rtb_Integrator_c[Itemp_tmp] = Chassis_X.Integrator_CSTATE_a[Itemp_tmp];

    /* Saturate: '<S230>/Saturation' */
    u0 = rtb_Sum6[Itemp_tmp];
    if (u0 > Chassis_P.Saturation_UpperSat_c) {
      u0 = Chassis_P.Saturation_UpperSat_c;
    } else if (u0 < Chassis_P.Saturation_LowerSat_j) {
      u0 = Chassis_P.Saturation_LowerSat_j;
    }

    /* Sum: '<S231>/Add2' incorporates:
     *  Constant: '<S231>/Constant9'
     */
    u0_0 = Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS - u0;

    /* Saturate: '<S231>/Saturation' */
    if (u0_0 > Chassis_P.Saturation_UpperSat_g) {
      u0_0 = Chassis_P.Saturation_UpperSat_g;
    } else if (u0_0 < Chassis_P.Saturation_LowerSat_g) {
      u0_0 = Chassis_P.Saturation_LowerSat_g;
    }

    /* Sum: '<S231>/Add1' incorporates:
     *  Gain: '<S215>/Gain4'
     *  Inport: '<Root>/DriveLineInput'
     *  Integrator: '<S232>/Integrator'
     *  Integrator: '<S235>/Integrator'
     *  Product: '<S231>/Product3'
     *  Saturate: '<S231>/Saturation'
     */
    Chassis_B.Add1[Itemp_tmp] = (Chassis_X.Integrator_CSTATE[Itemp_tmp] * u0_0 -
                                 Chassis_P.Gain4_Gain[Itemp_tmp] * Chassis_U.DriveLineInput.AxlTrq[Itemp_tmp]) -
                                Chassis_X.Integrator_CSTATE_a[Itemp_tmp];

    /* UnitConversion: '<S240>/Unit Conversion' incorporates:
     *  Inport: '<Root>/DriveLineInput'
     */
    rtb_xdot[Itemp_tmp] = 1.0E-5 * Chassis_U.DriveLineInput.BrkPrs[Itemp_tmp];

    /* Saturate: '<S230>/Saturation' */
    rtb_Saturation[Itemp_tmp] = u0;
  }

  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* UnitConversion: '<S240>/Unit Conversion1' incorporates:
     *  Memory: '<S239>/Memory'
     */
    /* Unit Conversion - from: rad/s to: rpm
       Expression: output = (9.5493*input) + (0) */
    Chassis_B.UnitConversion1[0] = 9.5492965855137211 * Chassis_DW.Memory_PreviousInput[0];
    Chassis_B.UnitConversion1[1] = 9.5492965855137211 * Chassis_DW.Memory_PreviousInput[1];
    Chassis_B.UnitConversion1[2] = 9.5492965855137211 * Chassis_DW.Memory_PreviousInput[2];
    Chassis_B.UnitConversion1[3] = 9.5492965855137211 * Chassis_DW.Memory_PreviousInput[3];
  }

  /* Lookup_n-D: '<S240>/2-D Lookup Table' */
  rtb_xdot_o =
      look2_pbinlcpw(rtb_xdot[0], Chassis_B.UnitConversion1[0], Chassis_P.CombinedSlipWheel2DOF_brake_p_bpt,
                     Chassis_P.CombinedSlipWheel2DOF_brake_n_bpt, Chassis_P.CombinedSlipWheel2DOF_f_brake_t,
                     &Chassis_DW.m_bpIndex[0], Chassis_P.uDLookupTable_maxIndex, m_mapped_brake_pressure_maxIndex + 1);
  u0 = look2_pbinlcpw(rtb_xdot[1], Chassis_B.UnitConversion1[1], Chassis_P.CombinedSlipWheel2DOF_brake_p_bpt,
                      Chassis_P.CombinedSlipWheel2DOF_brake_n_bpt, Chassis_P.CombinedSlipWheel2DOF_f_brake_t,
                      &Chassis_DW.m_bpIndex[2], Chassis_P.uDLookupTable_maxIndex, m_mapped_brake_pressure_maxIndex + 1);
  u0_0 =
      look2_pbinlcpw(rtb_xdot[2], Chassis_B.UnitConversion1[2], Chassis_P.CombinedSlipWheel2DOF_brake_p_bpt,
                     Chassis_P.CombinedSlipWheel2DOF_brake_n_bpt, Chassis_P.CombinedSlipWheel2DOF_f_brake_t,
                     &Chassis_DW.m_bpIndex[4], Chassis_P.uDLookupTable_maxIndex, m_mapped_brake_pressure_maxIndex + 1);
  rtb_xdot_p =
      look2_pbinlcpw(rtb_xdot[3], Chassis_B.UnitConversion1[3], Chassis_P.CombinedSlipWheel2DOF_brake_p_bpt,
                     Chassis_P.CombinedSlipWheel2DOF_brake_n_bpt, Chassis_P.CombinedSlipWheel2DOF_f_brake_t,
                     &Chassis_DW.m_bpIndex[6], Chassis_P.uDLookupTable_maxIndex, m_mapped_brake_pressure_maxIndex + 1);

  /* Gain: '<S238>/Ratio of static to kinetic' */
  rtb_UnaryMinus_i = Chassis_P.CombinedSlipWheel2DOF_mu_static / Chassis_P.CombinedSlipWheel2DOF_mu_kinetic;

  /* Saturate: '<S240>/Disallow Negative Brake Torque' */
  if (rtb_xdot_o > Chassis_P.DisallowNegativeBrakeTorque_UpperSat) {
    rtb_xdot_o = Chassis_P.DisallowNegativeBrakeTorque_UpperSat;
  } else if (rtb_xdot_o < Chassis_P.DisallowNegativeBrakeTorque_LowerSat) {
    rtb_xdot_o = Chassis_P.DisallowNegativeBrakeTorque_LowerSat;
  }

  /* Gain: '<S238>/Ratio of static to kinetic' */
  Chassis_B.Ratioofstatictokinetic[0] = rtb_UnaryMinus_i * rtb_xdot_o;

  /* Saturate: '<S240>/Disallow Negative Brake Torque' */
  rtb_xdot[0] = rtb_xdot_o;
  if (u0 > Chassis_P.DisallowNegativeBrakeTorque_UpperSat) {
    rtb_xdot_o = Chassis_P.DisallowNegativeBrakeTorque_UpperSat;
  } else if (u0 < Chassis_P.DisallowNegativeBrakeTorque_LowerSat) {
    rtb_xdot_o = Chassis_P.DisallowNegativeBrakeTorque_LowerSat;
  } else {
    rtb_xdot_o = u0;
  }

  /* Gain: '<S238>/Ratio of static to kinetic' */
  Chassis_B.Ratioofstatictokinetic[1] = rtb_UnaryMinus_i * rtb_xdot_o;

  /* Saturate: '<S240>/Disallow Negative Brake Torque' */
  rtb_xdot[1] = rtb_xdot_o;
  if (u0_0 > Chassis_P.DisallowNegativeBrakeTorque_UpperSat) {
    rtb_xdot_o = Chassis_P.DisallowNegativeBrakeTorque_UpperSat;
  } else if (u0_0 < Chassis_P.DisallowNegativeBrakeTorque_LowerSat) {
    rtb_xdot_o = Chassis_P.DisallowNegativeBrakeTorque_LowerSat;
  } else {
    rtb_xdot_o = u0_0;
  }

  /* Gain: '<S238>/Ratio of static to kinetic' */
  Chassis_B.Ratioofstatictokinetic[2] = rtb_UnaryMinus_i * rtb_xdot_o;

  /* Saturate: '<S240>/Disallow Negative Brake Torque' */
  rtb_xdot[2] = rtb_xdot_o;
  if (rtb_xdot_p > Chassis_P.DisallowNegativeBrakeTorque_UpperSat) {
    rtb_xdot_o = Chassis_P.DisallowNegativeBrakeTorque_UpperSat;
  } else if (rtb_xdot_p < Chassis_P.DisallowNegativeBrakeTorque_LowerSat) {
    rtb_xdot_o = Chassis_P.DisallowNegativeBrakeTorque_LowerSat;
  } else {
    rtb_xdot_o = rtb_xdot_p;
  }

  /* Gain: '<S238>/Ratio of static to kinetic' */
  Chassis_B.Ratioofstatictokinetic[3] = rtb_UnaryMinus_i * rtb_xdot_o;

  /* Gain: '<S238>/Ratio of static to kinetic1' */
  rtb_UnaryMinus_i = Chassis_P.CombinedSlipWheel2DOF_mu_kinetic / Chassis_P.CombinedSlipWheel2DOF_mu_kinetic;

  /* Gain: '<S238>/Ratio of static to kinetic1' */
  Chassis_B.Ratioofstatictokinetic1[0] = rtb_UnaryMinus_i * rtb_xdot[0];
  Chassis_B.Ratioofstatictokinetic1[1] = rtb_UnaryMinus_i * rtb_xdot[1];
  Chassis_B.Ratioofstatictokinetic1[2] = rtb_UnaryMinus_i * rtb_xdot[2];
  Chassis_B.Ratioofstatictokinetic1[3] = rtb_UnaryMinus_i * rtb_xdot_o;

  /* Outputs for Iterator SubSystem: '<S237>/Clutch Scalar Parameters' incorporates:
   *  ForEach: '<S241>/For Each'
   */
  for (ForEach_itr_o = 0; ForEach_itr_o < 4; ForEach_itr_o++) {
    /* Chart: '<S241>/Clutch' incorporates:
     *  ForEachSliceSelector generated from: '<S241>/Tfmaxk'
     *  ForEachSliceSelector generated from: '<S241>/Tfmaxs'
     *  ForEachSliceSelector generated from: '<S241>/Tout'
     */
    Chassis_Clutch(Chassis_B.Add1[ForEach_itr_o], Chassis_B.Ratioofstatictokinetic[ForEach_itr_o],
                   Chassis_B.Ratioofstatictokinetic1[ForEach_itr_o],
                   Chassis_P.VEH.InitialLongVel / Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS,
                   Chassis_P.CombinedSlipWheel2DOF_br, Chassis_P.CombinedSlipWheel2DOF_IYY,
                   Chassis_P.CoreSubsys_d0.Clutch_OmegaTol, &Chassis_B.CoreSubsys_d0[ForEach_itr_o].sf_Clutch,
                   &Chassis_DW.CoreSubsys_d0[ForEach_itr_o].sf_Clutch, &Chassis_P.CoreSubsys_d0.sf_Clutch,
                   &Chassis_X.CoreSubsys_d0[ForEach_itr_o].sf_Clutch);

    /* ForEachSliceAssignment generated from: '<S241>/Omega' */
    Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[ForEach_itr_o] =
        Chassis_B.CoreSubsys_d0[ForEach_itr_o].sf_Clutch.Omega;
  }

  /* End of Outputs for SubSystem: '<S237>/Clutch Scalar Parameters' */

  /* Sum: '<S158>/Add1' */
  rtb_Add1_b_idx_0 += Chassis_B.ubvbwb[0];

  /* Sum: '<S166>/Add1' */
  u0 = rtb_Add1_h[0] + Chassis_B.ubvbwb[0];

  /* Sum: '<S194>/Add1' */
  u0_0 = rtb_Add1_j[0] + Chassis_B.ubvbwb[0];

  /* Sum: '<S202>/Add1' */
  rtb_xdot_p = rtb_Add1_p[0] + Chassis_B.ubvbwb[0];

  /* Reshape: '<S12>/Reshape2' incorporates:
   *  Integrator: '<S211>/Integrator1'
   */
  rtb_VectorConcatenate8[0] = rtb_Add1_b_idx_0;
  rtb_VectorConcatenate8[3] = u0;
  rtb_VectorConcatenate8[6] = u0_0;
  rtb_VectorConcatenate8[9] = rtb_xdot_p;

  /* Outport: '<Root>/VehBody' */
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Lft.Vel.xdot = rtb_Add1_b_idx_0;

  /* Sum: '<S166>/Add1' */
  rtb_Add1_h[0] = u0;

  /* Sum: '<S194>/Add1' */
  rtb_Add1_j[0] = u0_0;

  /* Sum: '<S202>/Add1' */
  rtb_Add1_p[0] = rtb_xdot_p;

  /* Sum: '<S158>/Add1' */
  rtb_Add1_b_idx_0 = rtb_Add1_b_idx_1 + Chassis_B.ubvbwb[1];

  /* Sum: '<S166>/Add1' */
  u0 = rtb_Add1_h[1] + Chassis_B.ubvbwb[1];

  /* Sum: '<S194>/Add1' */
  u0_0 = rtb_Add1_j[1] + Chassis_B.ubvbwb[1];

  /* Sum: '<S202>/Add1' */
  rtb_xdot_p = rtb_Add1_p[1] + Chassis_B.ubvbwb[1];

  /* Concatenate: '<S4>/Matrix Concatenate' incorporates:
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S5>/Selector1'
   *  Selector: '<S5>/Selector2'
   *  UnaryMinus: '<S217>/Unary Minus2'
   */
  Chassis_B.MatrixConcatenate[0] = rtb_VectorConcatenate8[0];
  Chassis_B.MatrixConcatenate[1] = rtb_Add1_b_idx_0;
  Chassis_B.MatrixConcatenate[2] = -rtb_Switch[0];

  /* Concatenate: '<S256>/Vector Concatenate3' incorporates:
   *  Constant: '<S256>/Constant3'
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S4>/Selector2'
   *  Selector: '<S4>/Selector3'
   */
  Chassis_B.VectorConcatenate3[0] = Chassis_X.Integrator1_CSTATE_p[0];
  Chassis_B.VectorConcatenate3[1] = Chassis_P.Constant3_Value[0];
  Chassis_B.VectorConcatenate3[2] = Chassis_X.Integrator1_CSTATE_p[2];

  /* Concatenate: '<S4>/Matrix Concatenate' incorporates:
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S5>/Selector1'
   *  Selector: '<S5>/Selector2'
   *  UnaryMinus: '<S217>/Unary Minus2'
   */
  Chassis_B.MatrixConcatenate[3] = rtb_VectorConcatenate8[3];
  Chassis_B.MatrixConcatenate[4] = u0;
  Chassis_B.MatrixConcatenate[5] = -rtb_Switch[1];

  /* Concatenate: '<S256>/Vector Concatenate3' incorporates:
   *  Constant: '<S256>/Constant3'
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S4>/Selector2'
   *  Selector: '<S4>/Selector3'
   */
  Chassis_B.VectorConcatenate3[3] = Chassis_X.Integrator1_CSTATE_p[3];
  Chassis_B.VectorConcatenate3[4] = Chassis_P.Constant3_Value[1];
  Chassis_B.VectorConcatenate3[5] = Chassis_X.Integrator1_CSTATE_p[5];

  /* Concatenate: '<S4>/Matrix Concatenate' incorporates:
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S5>/Selector1'
   *  Selector: '<S5>/Selector2'
   *  UnaryMinus: '<S217>/Unary Minus2'
   */
  Chassis_B.MatrixConcatenate[6] = rtb_VectorConcatenate8[6];
  Chassis_B.MatrixConcatenate[7] = u0_0;
  Chassis_B.MatrixConcatenate[8] = -rtb_Switch[2];

  /* Concatenate: '<S256>/Vector Concatenate3' incorporates:
   *  Constant: '<S256>/Constant3'
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S4>/Selector2'
   *  Selector: '<S4>/Selector3'
   */
  Chassis_B.VectorConcatenate3[6] = Chassis_X.Integrator1_CSTATE_p[6];
  Chassis_B.VectorConcatenate3[7] = Chassis_P.Constant3_Value[2];
  Chassis_B.VectorConcatenate3[8] = Chassis_X.Integrator1_CSTATE_p[8];

  /* Concatenate: '<S4>/Matrix Concatenate' incorporates:
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S5>/Selector1'
   *  Selector: '<S5>/Selector2'
   *  UnaryMinus: '<S217>/Unary Minus2'
   */
  Chassis_B.MatrixConcatenate[9] = rtb_VectorConcatenate8[9];
  Chassis_B.MatrixConcatenate[10] = rtb_xdot_p;
  Chassis_B.MatrixConcatenate[11] = -rtb_Switch[3];

  /* Concatenate: '<S256>/Vector Concatenate3' incorporates:
   *  Constant: '<S256>/Constant3'
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S4>/Selector2'
   *  Selector: '<S4>/Selector3'
   */
  Chassis_B.VectorConcatenate3[9] = Chassis_X.Integrator1_CSTATE_p[9];
  Chassis_B.VectorConcatenate3[10] = Chassis_P.Constant3_Value[3];
  Chassis_B.VectorConcatenate3[11] = Chassis_X.Integrator1_CSTATE_p[11];

  /* Outputs for Iterator SubSystem: '<S215>/Wheel to Body Transform' incorporates:
   *  ForEach: '<S257>/For Each'
   */
  for (ForEach_itr = 0; ForEach_itr < 4; ForEach_itr++) {
    /* ForEachSliceSelector generated from: '<S257>/WheelAngles' incorporates:
     *  ForEachSliceSelector generated from: '<S257>/VelVeh'
     *  Product: '<S257>/Divide1'
     *  SignalConversion generated from: '<S258>/sincos'
     */
    Itemp_tmp = 3 * ForEach_itr + 2;

    /* SignalConversion generated from: '<S258>/sincos' incorporates:
     *  Concatenate: '<S256>/Vector Concatenate3'
     *  ForEachSliceSelector generated from: '<S257>/WheelAngles'
     */
    rtb_sincos_o2_a[0] = Chassis_B.VectorConcatenate3[Itemp_tmp];

    /* ForEachSliceSelector generated from: '<S257>/WheelAngles' incorporates:
     *  ForEachSliceSelector generated from: '<S257>/VelVeh'
     *  Product: '<S257>/Divide1'
     *  SignalConversion generated from: '<S258>/sincos'
     */
    iy = 3 * ForEach_itr + 1;

    /* SignalConversion generated from: '<S258>/sincos' incorporates:
     *  Concatenate: '<S256>/Vector Concatenate3'
     *  ForEachSliceSelector generated from: '<S257>/WheelAngles'
     */
    rtb_sincos_o2_a[1] = Chassis_B.VectorConcatenate3[iy];
    rtb_sincos_o2_a[2] = Chassis_B.VectorConcatenate3[3 * ForEach_itr];

    /* Trigonometry: '<S258>/sincos' */
    rtb_Add_ms[0] = std::cos(rtb_sincos_o2_a[0]);
    rtb_Add1_b_idx_1 = std::sin(rtb_sincos_o2_a[0]);
    rtb_Add_ms[1] = std::cos(rtb_sincos_o2_a[1]);
    Fzo_prime_idx_2 = std::sin(rtb_sincos_o2_a[1]);
    rtb_Add_ms[2] = std::cos(rtb_sincos_o2_a[2]);
    rtb_ixk = std::sin(rtb_sincos_o2_a[2]);

    /* Fcn: '<S258>/Fcn11' */
    rtb_VectorConcatenate_i[0] = rtb_Add_ms[0] * rtb_Add_ms[1];

    /* Fcn: '<S258>/Fcn21' incorporates:
     *  Fcn: '<S258>/Fcn22'
     */
    rtb_thetadot = Fzo_prime_idx_2 * rtb_ixk;
    rtb_VectorConcatenate_i[1] = rtb_thetadot * rtb_Add_ms[0] - rtb_Add1_b_idx_1 * rtb_Add_ms[2];

    /* Fcn: '<S258>/Fcn31' incorporates:
     *  Fcn: '<S258>/Fcn32'
     */
    rtb_UnaryMinus_i = Fzo_prime_idx_2 * rtb_Add_ms[2];
    rtb_VectorConcatenate_i[2] = rtb_UnaryMinus_i * rtb_Add_ms[0] + rtb_Add1_b_idx_1 * rtb_ixk;

    /* Fcn: '<S258>/Fcn12' */
    rtb_VectorConcatenate_i[3] = rtb_Add1_b_idx_1 * rtb_Add_ms[1];

    /* Fcn: '<S258>/Fcn22' */
    rtb_VectorConcatenate_i[4] = rtb_thetadot * rtb_Add1_b_idx_1 + rtb_Add_ms[0] * rtb_Add_ms[2];

    /* Fcn: '<S258>/Fcn32' */
    rtb_VectorConcatenate_i[5] = rtb_UnaryMinus_i * rtb_Add1_b_idx_1 - rtb_Add_ms[0] * rtb_ixk;

    /* Fcn: '<S258>/Fcn13' */
    rtb_VectorConcatenate_i[6] = -Fzo_prime_idx_2;

    /* Fcn: '<S258>/Fcn23' */
    rtb_VectorConcatenate_i[7] = rtb_Add_ms[1] * rtb_ixk;

    /* Fcn: '<S258>/Fcn33' */
    rtb_VectorConcatenate_i[8] = rtb_Add_ms[1] * rtb_Add_ms[2];
    for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= 0; rtb_phidot_tmp += 2) {
      /* Product: '<S257>/Divide1' incorporates:
       *  Concatenate: '<S101>/Vector Concatenate'
       *  ForEachSliceSelector generated from: '<S257>/VelVeh'
       *  Trigonometry: '<S124>/sincos'
       */
      tmp_5 = _mm_loadu_pd(&rtb_VectorConcatenate_i[rtb_phidot_tmp]);
      tmp_6 = _mm_loadu_pd(&rtb_VectorConcatenate_i[rtb_phidot_tmp + 3]);
      tmp_3 = _mm_loadu_pd(&rtb_VectorConcatenate_i[rtb_phidot_tmp + 6]);
      _mm_storeu_pd(
          &rtb_sincos_o2_a[rtb_phidot_tmp],
          _mm_add_pd(_mm_mul_pd(_mm_set1_pd(Chassis_B.MatrixConcatenate[Itemp_tmp]), tmp_3),
                     _mm_add_pd(_mm_mul_pd(_mm_set1_pd(Chassis_B.MatrixConcatenate[iy]), tmp_6),
                                _mm_add_pd(_mm_mul_pd(_mm_set1_pd(Chassis_B.MatrixConcatenate[3 * ForEach_itr]), tmp_5),
                                           _mm_set1_pd(0.0)))));
    }

    /* Product: '<S257>/Divide1' incorporates:
     *  Concatenate: '<S101>/Vector Concatenate'
     *  ForEachSliceSelector generated from: '<S257>/VelVeh'
     *  Trigonometry: '<S124>/sincos'
     */
    for (rtb_phidot_tmp = 2; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
      rtb_sincos_o2_a[rtb_phidot_tmp] =
          (rtb_VectorConcatenate_i[rtb_phidot_tmp + 3] * Chassis_B.MatrixConcatenate[iy] +
           Chassis_B.MatrixConcatenate[3 * ForEach_itr] * rtb_VectorConcatenate_i[rtb_phidot_tmp]) +
          rtb_VectorConcatenate_i[rtb_phidot_tmp + 6] * Chassis_B.MatrixConcatenate[Itemp_tmp];
    }

    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Trigonometry: '<S259>/sincos' incorporates:
       *  Constant: '<S257>/Constant'
       *  SignalConversion generated from: '<S259>/sincos'
       */
      rtb_sincos_o1_d[0] = std::cos(Chassis_P.CoreSubsys_pna.Constant_Value[2]);
      rtb_Add1_b_idx_1 = std::sin(Chassis_P.CoreSubsys_pna.Constant_Value[2]);
      rtb_sincos_o1_d[1] = std::cos(Chassis_P.CoreSubsys_pna.Constant_Value[1]);
      Fzo_prime_idx_2 = std::sin(Chassis_P.CoreSubsys_pna.Constant_Value[1]);
      rtb_sincos_o1_d[2] = std::cos(Chassis_P.CoreSubsys_pna.Constant_Value[0]);
      rtb_ixk = std::sin(Chassis_P.CoreSubsys_pna.Constant_Value[0]);

      /* Fcn: '<S259>/Fcn11' incorporates:
       *  Concatenate: '<S261>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[0] = rtb_sincos_o1_d[0] * rtb_sincos_o1_d[1];

      /* Fcn: '<S259>/Fcn21' incorporates:
       *  Concatenate: '<S261>/Vector Concatenate'
       *  Fcn: '<S259>/Fcn22'
       */
      rtb_thetadot = Fzo_prime_idx_2 * rtb_ixk;
      Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[1] =
          rtb_thetadot * rtb_sincos_o1_d[0] - rtb_Add1_b_idx_1 * rtb_sincos_o1_d[2];

      /* Fcn: '<S259>/Fcn31' incorporates:
       *  Concatenate: '<S261>/Vector Concatenate'
       *  Fcn: '<S259>/Fcn32'
       */
      rtb_UnaryMinus_i = Fzo_prime_idx_2 * rtb_sincos_o1_d[2];
      Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[2] =
          rtb_UnaryMinus_i * rtb_sincos_o1_d[0] + rtb_Add1_b_idx_1 * rtb_ixk;

      /* Fcn: '<S259>/Fcn12' incorporates:
       *  Concatenate: '<S261>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[3] = rtb_Add1_b_idx_1 * rtb_sincos_o1_d[1];

      /* Fcn: '<S259>/Fcn22' incorporates:
       *  Concatenate: '<S261>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[4] =
          rtb_thetadot * rtb_Add1_b_idx_1 + rtb_sincos_o1_d[0] * rtb_sincos_o1_d[2];

      /* Fcn: '<S259>/Fcn32' incorporates:
       *  Concatenate: '<S261>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[5] =
          rtb_UnaryMinus_i * rtb_Add1_b_idx_1 - rtb_sincos_o1_d[0] * rtb_ixk;

      /* Fcn: '<S259>/Fcn13' incorporates:
       *  Concatenate: '<S261>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[6] = -Fzo_prime_idx_2;

      /* Fcn: '<S259>/Fcn23' incorporates:
       *  Concatenate: '<S261>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[7] = rtb_sincos_o1_d[1] * rtb_ixk;

      /* Fcn: '<S259>/Fcn33' incorporates:
       *  Concatenate: '<S261>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[8] = rtb_sincos_o1_d[1] * rtb_sincos_o1_d[2];
    }

    /* Product: '<S257>/Divide2' incorporates:
     *  Concatenate: '<S261>/Vector Concatenate'
     *  Trigonometry: '<S124>/sincos'
     */
    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
      rtb_Divide3[rtb_phidot_tmp] =
          (Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[rtb_phidot_tmp + 3] * rtb_sincos_o2_a[1] +
           Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[rtb_phidot_tmp] * rtb_sincos_o2_a[0]) +
          Chassis_B.CoreSubsys_pna[ForEach_itr].VectorConcatenate[rtb_phidot_tmp + 6] * rtb_sincos_o2_a[2];
    }

    /* End of Product: '<S257>/Divide2' */

    /* ForEachSliceAssignment generated from: '<S257>/ydotWheel' */
    rtb_ImpAsg_InsertedFor_ydotWheel_at_inport_0[ForEach_itr] = rtb_Divide3[1];

    /* ForEachSliceAssignment generated from: '<S257>/xdotWheel' */
    rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[ForEach_itr] = rtb_Divide3[0];
  }

  /* End of Outputs for SubSystem: '<S215>/Wheel to Body Transform' */
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Concatenate: '<S10>/Matrix Concatenate1' incorporates:
     *  Constant: '<S10>/SteerRates'
     */
    Chassis_B.MatrixConcatenate1[0] = Chassis_P.SteerRates_Value[0];
    Chassis_B.MatrixConcatenate1[1] = Chassis_P.SteerRates_Value[0];
    Chassis_B.MatrixConcatenate1[2] = Chassis_P.SteerRates_Value[0];
    Chassis_B.MatrixConcatenate1[3] = Chassis_P.SteerRates_Value[1];
    Chassis_B.MatrixConcatenate1[4] = Chassis_P.SteerRates_Value[1];
    Chassis_B.MatrixConcatenate1[5] = Chassis_P.SteerRates_Value[1];
    Chassis_B.MatrixConcatenate1[6] = Chassis_P.SteerRates_Value[2];
    Chassis_B.MatrixConcatenate1[7] = Chassis_P.SteerRates_Value[2];
    Chassis_B.MatrixConcatenate1[8] = Chassis_P.SteerRates_Value[2];
    Chassis_B.MatrixConcatenate1[9] = Chassis_P.SteerRates_Value[3];
    Chassis_B.MatrixConcatenate1[10] = Chassis_P.SteerRates_Value[3];
    Chassis_B.MatrixConcatenate1[11] = Chassis_P.SteerRates_Value[3];
  }

  /* Concatenate: '<S10>/Matrix Concatenate' incorporates:
   *  Constant: '<S10>/Constant'
   *  Reshape: '<S10>/Reshape4'
   */
  rtb_VectorConcatenate1_g[0] = Chassis_P.Constant_Value_o[0];
  rtb_VectorConcatenate1_g[1] = Chassis_P.Constant_Value_o[0];
  rtb_VectorConcatenate1_g[3] = Chassis_P.Constant_Value_o[1];
  rtb_VectorConcatenate1_g[4] = Chassis_P.Constant_Value_o[1];
  rtb_VectorConcatenate1_g[6] = Chassis_P.Constant_Value_o[2];
  rtb_VectorConcatenate1_g[7] = Chassis_P.Constant_Value_o[2];
  rtb_VectorConcatenate1_g[9] = Chassis_P.Constant_Value_o[3];
  rtb_VectorConcatenate1_g[10] = Chassis_P.Constant_Value_o[3];
  rtb_VectorConcatenate1_g[2] = rtb_pqr[2];
  rtb_VectorConcatenate1_g[5] = rtb_pqr[2];
  rtb_VectorConcatenate1_g[8] = rtb_pqr[2];
  rtb_VectorConcatenate1_g[11] = rtb_pqr[2];
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= 10; rtb_phidot_tmp += 2) {
    /* Sum: '<S10>/Add' incorporates:
     *  Concatenate: '<S10>/Matrix Concatenate1'
     */
    tmp_5 = _mm_loadu_pd(&Chassis_B.MatrixConcatenate1[rtb_phidot_tmp]);
    tmp_6 = _mm_loadu_pd(&rtb_VectorConcatenate1_g[rtb_phidot_tmp]);
    _mm_storeu_pd(&rtb_AngVel_g[rtb_phidot_tmp], _mm_add_pd(tmp_5, tmp_6));
  }

  /* UnaryMinus: '<S215>/Unary Minus' incorporates:
   *  Selector: '<S4>/Selector1'
   *  Sum: '<S10>/Add'
   */
  rtb_UnaryMinus[0] = -rtb_AngVel_g[2];
  rtb_UnaryMinus[1] = -rtb_AngVel_g[5];
  rtb_UnaryMinus[2] = -rtb_AngVel_g[8];
  rtb_UnaryMinus[3] = -rtb_AngVel_g[11];
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* MATLAB Function: '<S215>/Camber Angle' incorporates:
     *  Constant: '<S1>/Constant6'
     */
    Chassis_B.y[0] = Chassis_P.Constant6_Value[5] / Chassis_P.Constant6_Value[8];
    Chassis_B.y[0] = std::atan(Chassis_B.y[0]);
    Chassis_B.y[1] = Chassis_P.Constant6_Value[14] / Chassis_P.Constant6_Value[17];
    Chassis_B.y[1] = std::atan(Chassis_B.y[1]);
    Chassis_B.y[2] = Chassis_P.Constant6_Value[23] / Chassis_P.Constant6_Value[26];
    Chassis_B.y[2] = std::atan(Chassis_B.y[2]);
    Chassis_B.y[3] = Chassis_P.Constant6_Value[32] / Chassis_P.Constant6_Value[35];
    Chassis_B.y[3] = std::atan(Chassis_B.y[3]);
  }

  /* Concatenate: '<S216>/Vector Concatenate' incorporates:
   *  Inport: '<Root>/Env'
   *  Reshape: '<S216>/Reshape'
   */
  rtb_VectorConcatenate_d[1] = Chassis_U.Env.Gnd.mu.mu_FL;
  rtb_VectorConcatenate_d[28] = Chassis_U.Env.Gnd.mu.mu_FR;
  rtb_VectorConcatenate_d[55] = Chassis_U.Env.Gnd.mu.mu_RL;
  rtb_VectorConcatenate_d[82] = Chassis_U.Env.Gnd.mu.mu_RR;
  rtb_VectorConcatenate_d[2] = Chassis_U.Env.Gnd.mu.mu_FL;
  rtb_VectorConcatenate_d[29] = Chassis_U.Env.Gnd.mu.mu_FR;
  rtb_VectorConcatenate_d[56] = Chassis_U.Env.Gnd.mu.mu_RL;
  rtb_VectorConcatenate_d[83] = Chassis_U.Env.Gnd.mu.mu_RR;

  /* Sum: '<S215>/Add' incorporates:
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S4>/Selector3'
   */
  rtb_Add_j[0] = Chassis_X.Integrator1_CSTATE_p[0] + Chassis_B.y[0];

  /* Concatenate: '<S216>/Vector Concatenate' incorporates:
   *  Constant: '<S216>/0'
   *  Constant: '<S216>/ones2'
   */
  rtb_VectorConcatenate_d[0] = Chassis_P.ones2_Value[0];
  rtb_VectorConcatenate_d[3] = Chassis_P.u_Value_m[0];

  /* Sum: '<S215>/Add' incorporates:
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S4>/Selector3'
   */
  rtb_Add_j[1] = Chassis_B.y[1] + Chassis_X.Integrator1_CSTATE_p[3];

  /* Concatenate: '<S216>/Vector Concatenate' incorporates:
   *  Constant: '<S216>/0'
   *  Constant: '<S216>/ones2'
   */
  rtb_VectorConcatenate_d[27] = Chassis_P.ones2_Value[1];
  rtb_VectorConcatenate_d[30] = Chassis_P.u_Value_m[1];

  /* Sum: '<S215>/Add' incorporates:
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S4>/Selector3'
   */
  rtb_Add_j[2] = Chassis_B.y[2] + Chassis_X.Integrator1_CSTATE_p[6];

  /* Concatenate: '<S216>/Vector Concatenate' incorporates:
   *  Constant: '<S216>/0'
   *  Constant: '<S216>/ones2'
   */
  rtb_VectorConcatenate_d[54] = Chassis_P.ones2_Value[2];
  rtb_VectorConcatenate_d[57] = Chassis_P.u_Value_m[2];

  /* Sum: '<S215>/Add' incorporates:
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S4>/Selector3'
   */
  rtb_Add_j[3] = Chassis_B.y[3] + Chassis_X.Integrator1_CSTATE_p[9];

  /* Concatenate: '<S216>/Vector Concatenate' incorporates:
   *  Constant: '<S216>/0'
   *  Constant: '<S216>/1'
   *  Constant: '<S216>/2'
   *  Constant: '<S216>/ones'
   *  Constant: '<S216>/ones1'
   *  Constant: '<S216>/ones2'
   *  Constant: '<S216>/ones3'
   */
  rtb_VectorConcatenate_d[81] = Chassis_P.ones2_Value[3];
  rtb_VectorConcatenate_d[84] = Chassis_P.u_Value_m[3];
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 9; rtb_phidot_tmp++) {
    rtb_VectorConcatenate_d[rtb_phidot_tmp + 4] = Chassis_P.ones1_Value[rtb_phidot_tmp];
    rtb_VectorConcatenate_d[rtb_phidot_tmp + 31] = Chassis_P.ones1_Value[rtb_phidot_tmp + 9];
    rtb_VectorConcatenate_d[rtb_phidot_tmp + 58] = Chassis_P.ones1_Value[rtb_phidot_tmp + 18];
    rtb_VectorConcatenate_d[rtb_phidot_tmp + 85] = Chassis_P.ones1_Value[rtb_phidot_tmp + 27];
  }

  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 4; rtb_phidot_tmp++) {
    rtb_VectorConcatenate_d[27 * rtb_phidot_tmp + 13] = Chassis_P.u_Value_j[rtb_phidot_tmp];
    for (Itemp_tmp = 0; Itemp_tmp < 5; Itemp_tmp++) {
      rtb_VectorConcatenate_d[(Itemp_tmp + 27 * rtb_phidot_tmp) + 14] =
          Chassis_P.ones3_Value[5 * rtb_phidot_tmp + Itemp_tmp];
    }

    Itemp_tmp = rtb_phidot_tmp << 1;
    rtb_VectorConcatenate_d[27 * rtb_phidot_tmp + 19] = Chassis_P.u_Value_jv[Itemp_tmp];
    rtb_VectorConcatenate_d[27 * rtb_phidot_tmp + 20] = Chassis_P.u_Value_jv[Itemp_tmp + 1];
  }

  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 6; rtb_phidot_tmp++) {
    rtb_VectorConcatenate_d[rtb_phidot_tmp + 21] = Chassis_P.ones_Value[rtb_phidot_tmp];
    rtb_VectorConcatenate_d[rtb_phidot_tmp + 48] = Chassis_P.ones_Value[rtb_phidot_tmp + 6];
    rtb_VectorConcatenate_d[rtb_phidot_tmp + 75] = Chassis_P.ones_Value[rtb_phidot_tmp + 12];
    rtb_VectorConcatenate_d[rtb_phidot_tmp + 102] = Chassis_P.ones_Value[rtb_phidot_tmp + 18];
  }

  /* MATLAB Function: '<S229>/Magic Tire Const Input' incorporates:
   *  Concatenate: '<S216>/Vector Concatenate'
   *  Constant: '<S212>/Pressure'
   *  Selector: '<S4>/Selector1'
   *  Sum: '<S104>/Sum of Elements15'
   *  Sum: '<S104>/Sum of Elements16'
   *  Sum: '<S107>/Sum of Elements'
   *  Sum: '<S107>/Sum of Elements1'
   *  Sum: '<S107>/Sum of Elements2'
   *  Sum: '<S10>/Add'
   *  Sum: '<S112>/Sum of Elements'
   *  Sum: '<S112>/Sum of Elements1'
   *  Sum: '<S114>/Sum of Elements'
   *  Sum: '<S114>/Sum of Elements1'
   *  Sum: '<S114>/Sum of Elements2'
   *  Sum: '<S131>/Sum of Elements'
   *  Sum: '<S136>/Sum of Elements'
   *  Sum: '<S44>/Sum of Elements'
   *  Sum: '<S53>/Sum of Elements'
   *  Sum: '<S70>/Sum of Elements'
   *  Sum: '<S84>/Sum of Elements'
   *  UnaryMinus: '<S215>/Unary Minus'
   */
  rtb_phidot = Chassis_P.CombinedSlipWheel2DOF_Q_FZ1;
  // modified by dongyuanhu
  // if (Chassis_P.MagicTireConstInput_vdynMF[3] == 1.0) {
  if (std::abs(Chassis_P.MagicTireConstInput_vdynMF[3] - 1.0) < 0.5) {
    real_T Drphi_tmp;
    real_T Drphi_tmp_0;
    real_T Drphi_tmp_1;
    real_T Drphi_tmp_2;
    real_T Drphi_tmp_3;
    real_T Dx_tmp;
    real_T SHy_idx_0;
    real_T SHy_idx_1;
    real_T SHy_idx_2;
    real_T b_idx_0;
    real_T gamma_star_tmp;
    real_T lam_Cz_0;
    real_T lam_Cz_tmp;
    real_T lam_mux_tmp;
    real_T lam_mux_tmp_0;
    real_T lam_mux_tmp_1;
    real_T lam_muy_prime;
    real_T lam_muy_prime_idx_1;
    real_T lam_muy_prime_idx_2;
    real_T mu_y_idx_0;
    real_T mu_y_idx_1;
    real_T mu_y_idx_2;
    real_T mu_y_tmp;
    real_T rtb_a_o;
    real_T rtb_a_tmp;
    real_T rtb_xdot_tmp_0;
    real_T rtb_xdot_tmp_1;
    real_T rtb_xdot_tmp_2;
    int8_T s;
    Chassis_div0protect(rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0, Chassis_P.CombinedSlipWheel2DOF_VXLOW, Vc, rtb_a);
    rtb_thetadot = Chassis_P.Pressure_const;
    rtb_Add_n = rtb_Add_j[0];
    lam_mux[0] = rtb_VectorConcatenate_d[1];
    rtb_sig_x[0] = rtb_VectorConcatenate_d[2];
    if (rtb_Add_j[0] < Chassis_P.CombinedSlipWheel2DOF_CAMMIN) {
      rtb_Add_n = Chassis_P.CombinedSlipWheel2DOF_CAMMIN;
    }

    if (rtb_Add_n > Chassis_P.CombinedSlipWheel2DOF_CAMMAX) {
      rtb_Add_n = Chassis_P.CombinedSlipWheel2DOF_CAMMAX;
    }

    if (Chassis_P.Pressure_const < Chassis_P.CombinedSlipWheel2DOF_PRESMIN) {
      rtb_thetadot = Chassis_P.CombinedSlipWheel2DOF_PRESMIN;
    }

    if (rtb_thetadot > Chassis_P.CombinedSlipWheel2DOF_PRESMAX) {
      rtb_thetadot = Chassis_P.CombinedSlipWheel2DOF_PRESMAX;
    }

    rtb_sig_y[0] = (rtb_thetadot - Chassis_P.CombinedSlipWheel2DOF_NOMPRES) / Chassis_P.CombinedSlipWheel2DOF_NOMPRES;
    if (rtb_VectorConcatenate_d[1] <= 0.0) {
      lam_mux[0] = 2.2204460492503131E-16;
    }

    if (rtb_VectorConcatenate_d[2] <= 0.0) {
      rtb_sig_x[0] = 2.2204460492503131E-16;
    }

    rtb_Add_j[0] = rtb_Add_n;
    rtb_Mx[0] = rtb_thetadot;
    rtb_thetadot = Chassis_P.Pressure_const;
    rtb_Add_n = rtb_Add_j[1];
    lam_mux[1] = rtb_VectorConcatenate_d[28];
    rtb_sig_x[1] = rtb_VectorConcatenate_d[29];
    if (rtb_Add_j[1] < Chassis_P.CombinedSlipWheel2DOF_CAMMIN) {
      rtb_Add_n = Chassis_P.CombinedSlipWheel2DOF_CAMMIN;
    }

    if (rtb_Add_n > Chassis_P.CombinedSlipWheel2DOF_CAMMAX) {
      rtb_Add_n = Chassis_P.CombinedSlipWheel2DOF_CAMMAX;
    }

    if (Chassis_P.Pressure_const < Chassis_P.CombinedSlipWheel2DOF_PRESMIN) {
      rtb_thetadot = Chassis_P.CombinedSlipWheel2DOF_PRESMIN;
    }

    if (rtb_thetadot > Chassis_P.CombinedSlipWheel2DOF_PRESMAX) {
      rtb_thetadot = Chassis_P.CombinedSlipWheel2DOF_PRESMAX;
    }

    rtb_sig_y[1] = (rtb_thetadot - Chassis_P.CombinedSlipWheel2DOF_NOMPRES) / Chassis_P.CombinedSlipWheel2DOF_NOMPRES;
    if (rtb_VectorConcatenate_d[28] <= 0.0) {
      lam_mux[1] = 2.2204460492503131E-16;
    }

    if (rtb_VectorConcatenate_d[29] <= 0.0) {
      rtb_sig_x[1] = 2.2204460492503131E-16;
    }

    rtb_Add_j[1] = rtb_Add_n;
    rtb_Mx[1] = rtb_thetadot;
    rtb_thetadot = Chassis_P.Pressure_const;
    rtb_Add_n = rtb_Add_j[2];
    lam_mux[2] = rtb_VectorConcatenate_d[55];
    rtb_sig_x[2] = rtb_VectorConcatenate_d[56];
    if (rtb_Add_j[2] < Chassis_P.CombinedSlipWheel2DOF_CAMMIN) {
      rtb_Add_n = Chassis_P.CombinedSlipWheel2DOF_CAMMIN;
    }

    if (rtb_Add_n > Chassis_P.CombinedSlipWheel2DOF_CAMMAX) {
      rtb_Add_n = Chassis_P.CombinedSlipWheel2DOF_CAMMAX;
    }

    if (Chassis_P.Pressure_const < Chassis_P.CombinedSlipWheel2DOF_PRESMIN) {
      rtb_thetadot = Chassis_P.CombinedSlipWheel2DOF_PRESMIN;
    }

    if (rtb_thetadot > Chassis_P.CombinedSlipWheel2DOF_PRESMAX) {
      rtb_thetadot = Chassis_P.CombinedSlipWheel2DOF_PRESMAX;
    }

    rtb_sig_y[2] = (rtb_thetadot - Chassis_P.CombinedSlipWheel2DOF_NOMPRES) / Chassis_P.CombinedSlipWheel2DOF_NOMPRES;
    if (rtb_VectorConcatenate_d[55] <= 0.0) {
      lam_mux[2] = 2.2204460492503131E-16;
    }

    if (rtb_VectorConcatenate_d[56] <= 0.0) {
      rtb_sig_x[2] = 2.2204460492503131E-16;
    }

    rtb_Add_j[2] = rtb_Add_n;
    rtb_Mx[2] = rtb_thetadot;
    rtb_thetadot = Chassis_P.Pressure_const;
    rtb_Add_n = rtb_Add_j[3];
    lam_mux_0 = rtb_VectorConcatenate_d[82];
    rtb_sig_x_f = rtb_VectorConcatenate_d[83];
    if (rtb_Add_j[3] < Chassis_P.CombinedSlipWheel2DOF_CAMMIN) {
      rtb_Add_n = Chassis_P.CombinedSlipWheel2DOF_CAMMIN;
    }

    if (rtb_Add_n > Chassis_P.CombinedSlipWheel2DOF_CAMMAX) {
      rtb_Add_n = Chassis_P.CombinedSlipWheel2DOF_CAMMAX;
    }

    if (Chassis_P.Pressure_const < Chassis_P.CombinedSlipWheel2DOF_PRESMIN) {
      rtb_thetadot = Chassis_P.CombinedSlipWheel2DOF_PRESMIN;
    }

    if (rtb_thetadot > Chassis_P.CombinedSlipWheel2DOF_PRESMAX) {
      rtb_thetadot = Chassis_P.CombinedSlipWheel2DOF_PRESMAX;
    }

    rtb_sig_y[3] = (rtb_thetadot - Chassis_P.CombinedSlipWheel2DOF_NOMPRES) / Chassis_P.CombinedSlipWheel2DOF_NOMPRES;
    if (rtb_VectorConcatenate_d[82] <= 0.0) {
      lam_mux_0 = 2.2204460492503131E-16;
    }

    if (rtb_VectorConcatenate_d[83] <= 0.0) {
      rtb_sig_x_f = 2.2204460492503131E-16;
    }

    rtb_Add_j[3] = rtb_Add_n;
    rtb_Mx[3] = rtb_thetadot;
    if (Chassis_P.CombinedSlipWheel2DOF_Q_FZ1 == 0.0) {
      boolean_T tempInds;
      rtb_thetadot = Chassis_P.CombinedSlipWheel2DOF_VERTICAL_STIFFNESS *
                     Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
      tempInds = (rtb_thetadot * rtb_thetadot - 4.0 * Chassis_P.CombinedSlipWheel2DOF_Q_FZ2 < 0.0);
      b_idx_0 = Chassis_P.CombinedSlipWheel2DOF_Q_FZ1;
      iy = 0;
      if (tempInds) {
        iy = 1;
      }

      if (iy - 1 >= 0) {
        b_idx_0 = Chassis_P.CombinedSlipWheel2DOF_Q_FZ2 * 0.0;
      }

      iy = 0;
      ibmat = 0;
      if (!tempInds) {
        iy = 1;
      }

      rtb_phidot_tmp = 0;
      if (!tempInds) {
        ibmat = 1;
      }

      Itemp_tmp = 0;
      if (!tempInds) {
        rtb_phidot_tmp = 1;
        Itemp_tmp = 1;
      }

      if ((iy != ibmat) || ((iy == 1 ? ibmat : iy) != rtb_phidot_tmp)) {
        Chassis_binary_expand_op(&lam_muy_prime_idx_3, tmp_size, Chassis_P.CombinedSlipWheel2DOF_VERTICAL_STIFFNESS, iy,
                                 Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS, ibmat,
                                 Chassis_P.CombinedSlipWheel2DOF_FNOMIN, rtb_phidot_tmp);
      }

      Chassis_binary_expand_op(&rtb_Product_ju, tmp_size_0, Chassis_P.CombinedSlipWheel2DOF_VERTICAL_STIFFNESS, iy,
                               Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS, ibmat,
                               Chassis_P.CombinedSlipWheel2DOF_FNOMIN, rtb_phidot_tmp);
      if ((iy == ibmat) && ((iy == 1 ? ibmat : iy) == rtb_phidot_tmp) && (tmp_size_0[1] == Itemp_tmp)) {
        tb_size[0] = 1;
        tb_size[1] = iy;
        if (ibmat - 1 >= 0) {
          SHy_idx_3 = rtb_thetadot * rtb_thetadot - 4.0 * Chassis_P.CombinedSlipWheel2DOF_Q_FZ2;
        }
      } else {
        Chassis_binary_expand_op_b3i5z2tw(&SHy_idx_3, tb_size, Chassis_P.CombinedSlipWheel2DOF_VERTICAL_STIFFNESS, iy,
                                          Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS, ibmat,
                                          Chassis_P.CombinedSlipWheel2DOF_FNOMIN, rtb_phidot_tmp,
                                          Chassis_P.CombinedSlipWheel2DOF_Q_FZ2, Itemp_tmp);
      }

      Chassis_sqrt(&SHy_idx_3, tb_size);
      if (!tempInds) {
        b_idx_0 = SHy_idx_3;
      }

      rtb_phidot = b_idx_0;
    }

    Drphi_0 = Chassis_P.CombinedSlipWheel2DOF_RIM_RADIUS + Chassis_P.CombinedSlipWheel2DOF_BOTTOM_OFFST;
    iy = 0;
    rtb_thetadot = rtb_VectorConcatenate_d[0] * Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    lam_muy_prime_idx_3 =
        Chassis_P.CombinedSlipWheel2DOF_Q_FCX * rtb_Integrator[0] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    rtb_Product_ju = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_muy_prime_idx_3 =
        Chassis_P.CombinedSlipWheel2DOF_Q_FCY * rtb_Integrator_g[0] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    SHy_idx_3 = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_muy_prime_idx_3 = rtb_Saturation[0] / Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS;
    rtb_xdot_tmp = std::abs(Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[0]);
    rtb_xdot[0] = std::fmax(
        (((Chassis_P.CombinedSlipWheel2DOF_Q_V2 * rtb_xdot_tmp * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS /
               Chassis_P.CombinedSlipWheel2DOF_LONGVL +
           1.0) -
          rtb_Product_ju) -
         SHy_idx_3) *
            ((Chassis_P.CombinedSlipWheel2DOF_Q_FZ3 * rtb_Add_j[0] * rtb_Add_j[0] + rtb_phidot) * rtb_Saturation[0] /
                 Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS +
             lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * Chassis_P.CombinedSlipWheel2DOF_Q_FZ2) *
            (Chassis_P.CombinedSlipWheel2DOF_PFZ1 * rtb_sig_y[0] + 1.0) * rtb_thetadot,
        (Drphi_0 - (Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS + rtb_Saturation[0]) / std::cos(rtb_Add_j[0])) *
            Chassis_P.CombinedSlipWheel2DOF_BOTTOM_STIFF);
    if (rtb_Saturation[0] > 0.0) {
      iy = 1;
    }

    rtb_UnaryMinus_i = rtb_thetadot;
    rtb_thetadot = rtb_VectorConcatenate_d[27] * Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    lam_muy_prime_idx_3 =
        Chassis_P.CombinedSlipWheel2DOF_Q_FCX * rtb_Integrator[1] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    rtb_Product_ju = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_muy_prime_idx_3 =
        Chassis_P.CombinedSlipWheel2DOF_Q_FCY * rtb_Integrator_g[1] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    SHy_idx_3 = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_muy_prime_idx_3 = rtb_Saturation[1] / Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS;
    rtb_xdot_tmp_0 = std::abs(Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[1]);
    rtb_xdot[1] = std::fmax(
        (((Chassis_P.CombinedSlipWheel2DOF_Q_V2 * rtb_xdot_tmp_0 * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS /
               Chassis_P.CombinedSlipWheel2DOF_LONGVL +
           1.0) -
          rtb_Product_ju) -
         SHy_idx_3) *
            ((Chassis_P.CombinedSlipWheel2DOF_Q_FZ3 * rtb_Add_j[1] * rtb_Add_j[1] + rtb_phidot) * rtb_Saturation[1] /
                 Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS +
             lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * Chassis_P.CombinedSlipWheel2DOF_Q_FZ2) *
            (Chassis_P.CombinedSlipWheel2DOF_PFZ1 * rtb_sig_y[1] + 1.0) * rtb_thetadot,
        (Drphi_0 - (Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS + rtb_Saturation[1]) / std::cos(rtb_Add_j[1])) *
            Chassis_P.CombinedSlipWheel2DOF_BOTTOM_STIFF);
    if (rtb_Saturation[1] > 0.0) {
      iy++;
    }

    rtb_Add1_b_idx_1 = rtb_thetadot;
    rtb_thetadot = rtb_VectorConcatenate_d[54] * Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    lam_muy_prime_idx_3 =
        Chassis_P.CombinedSlipWheel2DOF_Q_FCX * rtb_Integrator[2] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    rtb_Product_ju = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_muy_prime_idx_3 =
        Chassis_P.CombinedSlipWheel2DOF_Q_FCY * rtb_Integrator_g[2] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    SHy_idx_3 = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_muy_prime_idx_3 = rtb_Saturation[2] / Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS;
    rtb_xdot_tmp_1 = std::abs(Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[2]);
    rtb_xdot[2] = std::fmax(
        (((Chassis_P.CombinedSlipWheel2DOF_Q_V2 * rtb_xdot_tmp_1 * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS /
               Chassis_P.CombinedSlipWheel2DOF_LONGVL +
           1.0) -
          rtb_Product_ju) -
         SHy_idx_3) *
            ((Chassis_P.CombinedSlipWheel2DOF_Q_FZ3 * rtb_Add_j[2] * rtb_Add_j[2] + rtb_phidot) * rtb_Saturation[2] /
                 Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS +
             lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * Chassis_P.CombinedSlipWheel2DOF_Q_FZ2) *
            (Chassis_P.CombinedSlipWheel2DOF_PFZ1 * rtb_sig_y[2] + 1.0) * rtb_thetadot,
        (Drphi_0 - (Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS + rtb_Saturation[2]) / std::cos(rtb_Add_j[2])) *
            Chassis_P.CombinedSlipWheel2DOF_BOTTOM_STIFF);
    if (rtb_Saturation[2] > 0.0) {
      iy++;
    }

    Fzo_prime_idx_2 = rtb_thetadot;
    rtb_thetadot = rtb_VectorConcatenate_d[81] * Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    lam_muy_prime_idx_3 =
        Chassis_P.CombinedSlipWheel2DOF_Q_FCX * rtb_Integrator[3] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    rtb_Product_ju = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_muy_prime_idx_3 =
        Chassis_P.CombinedSlipWheel2DOF_Q_FCY * rtb_Integrator_g[3] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    SHy_idx_3 = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_muy_prime_idx_3 = rtb_Saturation[3] / Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS;
    rtb_xdot_tmp_2 = std::abs(Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[3]);
    rtb_xdot[3] = std::fmax(
        (((Chassis_P.CombinedSlipWheel2DOF_Q_V2 * rtb_xdot_tmp_2 * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS /
               Chassis_P.CombinedSlipWheel2DOF_LONGVL +
           1.0) -
          rtb_Product_ju) -
         SHy_idx_3) *
            ((Chassis_P.CombinedSlipWheel2DOF_Q_FZ3 * rtb_Add_n * rtb_Add_n + rtb_phidot) * rtb_Saturation[3] /
                 Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS +
             lam_muy_prime_idx_3 * lam_muy_prime_idx_3 * Chassis_P.CombinedSlipWheel2DOF_Q_FZ2) *
            (Chassis_P.CombinedSlipWheel2DOF_PFZ1 * rtb_sig_y[3] + 1.0) * rtb_thetadot,
        (Drphi_0 - (Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS + rtb_Saturation[3]) / std::cos(rtb_Add_n)) *
            Chassis_P.CombinedSlipWheel2DOF_BOTTOM_STIFF);
    if (rtb_Saturation[3] > 0.0) {
      iy++;
    }

    n_size = iy;
    iy = 0;
    if (rtb_Saturation[0] > 0.0) {
      n_data[0] = 1;
      iy = 1;
    }

    if (rtb_Saturation[1] > 0.0) {
      n_data[iy] = 2;
      iy++;
    }

    if (rtb_Saturation[2] > 0.0) {
      n_data[iy] = 3;
      iy++;
    }

    if (rtb_Saturation[3] > 0.0) {
      n_data[iy] = 4;
    }

    iy = 0;
    if (rtb_Saturation[0] > 0.0) {
      iy = 1;
    }

    if (rtb_Saturation[1] > 0.0) {
      iy++;
    }

    if (rtb_Saturation[2] > 0.0) {
      iy++;
    }

    if (rtb_Saturation[3] > 0.0) {
      iy++;
    }

    Itemp_tmp = iy;
    iy = 0;
    if (rtb_Saturation[0] > 0.0) {
      tmpDrphiVar_data[0] =
          Chassis_P.CombinedSlipWheel2DOF_Q_FCY2 * rtb_Integrator_g[0] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
      iy = 1;
    }

    if (rtb_Saturation[1] > 0.0) {
      tmpDrphiVar_data[iy] =
          Chassis_P.CombinedSlipWheel2DOF_Q_FCY2 * rtb_Integrator_g[1] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
      iy++;
    }

    if (rtb_Saturation[2] > 0.0) {
      tmpDrphiVar_data[iy] =
          Chassis_P.CombinedSlipWheel2DOF_Q_FCY2 * rtb_Integrator_g[2] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
      iy++;
    }

    if (rtb_Saturation[3] > 0.0) {
      tmpDrphiVar_data[iy] =
          Chassis_P.CombinedSlipWheel2DOF_Q_FCY2 * rtb_Integrator_g[3] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    }

    iy = 0;
    if (rtb_Saturation[0] > 0.0) {
      iy = 1;
    }

    if (rtb_Saturation[1] > 0.0) {
      iy++;
    }

    if (rtb_Saturation[2] > 0.0) {
      iy++;
    }

    if (rtb_Saturation[3] > 0.0) {
      iy++;
    }

    o_size = iy;
    iy = 0;
    if (rtb_Saturation[0] > 0.0) {
      o_data[0] = 1;
      iy = 1;
    }

    if (rtb_Saturation[1] > 0.0) {
      o_data[iy] = 2;
      iy++;
    }

    if (rtb_Saturation[2] > 0.0) {
      o_data[iy] = 3;
      iy++;
    }

    if (rtb_Saturation[3] > 0.0) {
      o_data[iy] = 4;
    }

    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
      Kyalpha[rtb_phidot_tmp] = rtb_Saturation[n_data[rtb_phidot_tmp] - 1];
    }

    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
      Gykappa[rtb_phidot_tmp] = rtb_Saturation[n_data[rtb_phidot_tmp] - 1];
    }

    iy = n_size;
    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
      cosprimealpha[rtb_phidot_tmp] = rtb_Saturation[n_data[rtb_phidot_tmp] - 1];
    }

    Chassis_abs(cosprimealpha, &n_size, tmp_data, &tmp_size_1);
    Chassis_abs(Kyalpha, &n_size, tmp_data, &iy);
    Chassis_abs(Gykappa, &n_size, tmp_data, &rtb_phidot_tmp);
    if ((n_size == iy) && ((n_size == 1 ? rtb_phidot_tmp : n_size) == Itemp_tmp) &&
        (((n_size == 1 ? tmp_size_1 : n_size) == 1 ? Itemp_tmp
          : n_size == 1                            ? tmp_size_1
                                                   : n_size) == o_size)) {
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        Kyalpha[rtb_phidot_tmp] = rtb_Saturation[n_data[rtb_phidot_tmp] - 1];
      }

      Chassis_abs(Kyalpha, &n_size, tmp_data, &tmp_size_1);
      iy = n_size;
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        Vs[rtb_phidot_tmp] =
            (rtb_xdot[n_data[rtb_phidot_tmp] - 1] / tmp_data[rtb_phidot_tmp] + tmpDrphiVar_data[rtb_phidot_tmp]) *
            rtb_VectorConcatenate_d[(o_data[rtb_phidot_tmp] - 1) * 27 + 22];
      }
    } else {
      Chassis_binary_expand_op_b3i5z2t(Vs, &iy, rtb_xdot, n_data, &n_size, rtb_Saturation, tmpDrphiVar_data, &Itemp_tmp,
                                       rtb_VectorConcatenate_d, o_data, &o_size);
    }

    iy = 0;
    rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_VERTICAL_STIFFNESS * rtb_VectorConcatenate_d[22];
    rtb_xdot_o = rtb_xdot[0];
    if (rtb_Saturation[0] > 0.0) {
      rtb_Product_ju = Vs[0];
      iy = 1;
    }

    FzUnSat[0] = rtb_xdot[0];
    if (rtb_xdot[0] < Chassis_P.CombinedSlipWheel2DOF_FZMIN) {
      rtb_xdot_o = Chassis_P.CombinedSlipWheel2DOF_FZMIN;
    }

    if (rtb_xdot_o > Chassis_P.CombinedSlipWheel2DOF_FZMAX) {
      rtb_xdot_o = Chassis_P.CombinedSlipWheel2DOF_FZMAX;
    }

    rtb_phidot = (rtb_xdot_o - rtb_UnaryMinus_i) / rtb_UnaryMinus_i;
    rtb_My[0] = (Chassis_P.CombinedSlipWheel2DOF_PECP2 * rtb_phidot + 1.0) * Chassis_P.CombinedSlipWheel2DOF_PECP1;
    lam_muy_prime_idx_3 = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[0] *
                          Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS / Chassis_P.CombinedSlipWheel2DOF_LONGVL;
    Chassis_B.Re[0] = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_Cz_0 = rtb_xdot[0] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    Chassis_B.Re[0] =
        (Chassis_P.CombinedSlipWheel2DOF_Q_V1 * Chassis_B.Re[0] + Chassis_P.CombinedSlipWheel2DOF_Q_RE0) *
            Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS -
        (std::atan(Chassis_P.CombinedSlipWheel2DOF_BREFF * lam_Cz_0) * Chassis_P.CombinedSlipWheel2DOF_DREFF +
         Chassis_P.CombinedSlipWheel2DOF_FREFF * lam_Cz_0) *
            (Chassis_P.CombinedSlipWheel2DOF_FNOMIN / rtb_Product_ju);
    if (Chassis_B.Re[0] < 0.001) {
      Chassis_B.Re[0] = 0.001;
    }

    Drphi_0 = Chassis_B.Re[0] * Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[0] -
              rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[0];
    rtb_Product_ju = Drphi_0 / rtb_a[0];
    if (rtb_Product_ju < Chassis_P.CombinedSlipWheel2DOF_KPUMIN) {
      rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_KPUMIN;
    }

    if (rtb_Product_ju > Chassis_P.CombinedSlipWheel2DOF_KPUMAX) {
      rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_KPUMAX;
    }

    lam_Hx[0] = (Chassis_P.CombinedSlipWheel2DOF_PHX2 * rtb_phidot + Chassis_P.CombinedSlipWheel2DOF_PHX1) *
                    rtb_VectorConcatenate_d[10] +
                rtb_Product_ju;
    rtb_Alpha_b = rt_atan2d_snf(rtb_ImpAsg_InsertedFor_ydotWheel_at_inport_0[0], rtb_a[0]);
    if (rtb_Alpha_b < Chassis_P.CombinedSlipWheel2DOF_ALPMIN) {
      rtb_Alpha_b = Chassis_P.CombinedSlipWheel2DOF_ALPMIN;
    }

    if (rtb_Alpha_b > Chassis_P.CombinedSlipWheel2DOF_ALPMAX) {
      rtb_Alpha_b = Chassis_P.CombinedSlipWheel2DOF_ALPMAX;
    }

    SHy_idx_0 = std::sin(rtb_Add_j[0]);
    lam_muy_prime_idx_3 = -rtb_a[0] * rtb_Product_ju;
    rtb_a_o = -rtb_a[0] * std::tan(rtb_Alpha_b);
    rtb_psidot = rtb_a_o * rtb_a_o;
    isLowSpeed[0] = (std::sqrt(Drphi_0 * Drphi_0 + rtb_psidot) < Chassis_P.CombinedSlipWheel2DOF_VXLOW);
    lam_muy_prime_idx_3 = std::sqrt(lam_muy_prime_idx_3 * lam_muy_prime_idx_3 + rtb_psidot) *
                              rtb_VectorConcatenate_d[3] / Chassis_P.CombinedSlipWheel2DOF_LONGVL +
                          1.0;
    rtb_ixk = lam_mux[0] / lam_muy_prime_idx_3;
    rtb_Saturation[0] = rtb_phidot;
    rtb_xdot[0] = rtb_xdot_o;
    rtb_Kappa[0] = rtb_Product_ju;
    rtb_Alpha[0] = rtb_Alpha_b;
    gamma_star[0] = SHy_idx_0;
    Vc[0] = std::sqrt(
        rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[0] * rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[0] + rtb_psidot);
    lam_muV[0] = lam_muy_prime_idx_3;
    rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_VERTICAL_STIFFNESS * rtb_VectorConcatenate_d[49];
    rtb_xdot_o = rtb_xdot[1];
    if (rtb_Saturation[1] > 0.0) {
      rtb_Product_ju = Vs[iy];
      iy++;
    }

    FzUnSat[1] = rtb_xdot[1];
    if (rtb_xdot[1] < Chassis_P.CombinedSlipWheel2DOF_FZMIN) {
      rtb_xdot_o = Chassis_P.CombinedSlipWheel2DOF_FZMIN;
    }

    if (rtb_xdot_o > Chassis_P.CombinedSlipWheel2DOF_FZMAX) {
      rtb_xdot_o = Chassis_P.CombinedSlipWheel2DOF_FZMAX;
    }

    rtb_phidot = (rtb_xdot_o - rtb_Add1_b_idx_1) / rtb_Add1_b_idx_1;
    rtb_My[1] = (Chassis_P.CombinedSlipWheel2DOF_PECP2 * rtb_phidot + 1.0) * Chassis_P.CombinedSlipWheel2DOF_PECP1;
    lam_muy_prime_idx_3 = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[1] *
                          Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS / Chassis_P.CombinedSlipWheel2DOF_LONGVL;
    Chassis_B.Re[1] = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_Cz_0 = rtb_xdot[1] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    Chassis_B.Re[1] =
        (Chassis_P.CombinedSlipWheel2DOF_Q_V1 * Chassis_B.Re[1] + Chassis_P.CombinedSlipWheel2DOF_Q_RE0) *
            Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS -
        (std::atan(Chassis_P.CombinedSlipWheel2DOF_BREFF * lam_Cz_0) * Chassis_P.CombinedSlipWheel2DOF_DREFF +
         Chassis_P.CombinedSlipWheel2DOF_FREFF * lam_Cz_0) *
            (Chassis_P.CombinedSlipWheel2DOF_FNOMIN / rtb_Product_ju);
    if (Chassis_B.Re[1] < 0.001) {
      Chassis_B.Re[1] = 0.001;
    }

    Drphi_0 = Chassis_B.Re[1] * Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[1] -
              rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[1];
    rtb_Product_ju = Drphi_0 / rtb_a[1];
    if (rtb_Product_ju < Chassis_P.CombinedSlipWheel2DOF_KPUMIN) {
      rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_KPUMIN;
    }

    if (rtb_Product_ju > Chassis_P.CombinedSlipWheel2DOF_KPUMAX) {
      rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_KPUMAX;
    }

    lam_Hx[1] = (Chassis_P.CombinedSlipWheel2DOF_PHX2 * rtb_phidot + Chassis_P.CombinedSlipWheel2DOF_PHX1) *
                    rtb_VectorConcatenate_d[37] +
                rtb_Product_ju;
    rtb_Alpha_b = rt_atan2d_snf(rtb_ImpAsg_InsertedFor_ydotWheel_at_inport_0[1], rtb_a[1]);
    if (rtb_Alpha_b < Chassis_P.CombinedSlipWheel2DOF_ALPMIN) {
      rtb_Alpha_b = Chassis_P.CombinedSlipWheel2DOF_ALPMIN;
    }

    if (rtb_Alpha_b > Chassis_P.CombinedSlipWheel2DOF_ALPMAX) {
      rtb_Alpha_b = Chassis_P.CombinedSlipWheel2DOF_ALPMAX;
    }

    SHy_idx_1 = std::sin(rtb_Add_j[1]);
    lam_muy_prime_idx_3 = -rtb_a[1] * rtb_Product_ju;
    rtb_a_o = -rtb_a[1] * std::tan(rtb_Alpha_b);
    rtb_psidot = rtb_a_o * rtb_a_o;
    isLowSpeed[1] = (std::sqrt(Drphi_0 * Drphi_0 + rtb_psidot) < Chassis_P.CombinedSlipWheel2DOF_VXLOW);
    lam_muy_prime_idx_3 = std::sqrt(lam_muy_prime_idx_3 * lam_muy_prime_idx_3 + rtb_psidot) *
                              rtb_VectorConcatenate_d[30] / Chassis_P.CombinedSlipWheel2DOF_LONGVL +
                          1.0;
    lam_mux_tmp = lam_mux[1] / lam_muy_prime_idx_3;
    rtb_Saturation[1] = rtb_phidot;
    rtb_xdot[1] = rtb_xdot_o;
    rtb_Kappa[1] = rtb_Product_ju;
    rtb_Alpha[1] = rtb_Alpha_b;
    gamma_star[1] = SHy_idx_1;
    Vc[1] = std::sqrt(
        rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[1] * rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[1] + rtb_psidot);
    lam_muV[1] = lam_muy_prime_idx_3;
    rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_VERTICAL_STIFFNESS * rtb_VectorConcatenate_d[76];
    rtb_xdot_o = rtb_xdot[2];
    if (rtb_Saturation[2] > 0.0) {
      rtb_Product_ju = Vs[iy];
      iy++;
    }

    FzUnSat[2] = rtb_xdot[2];
    if (rtb_xdot[2] < Chassis_P.CombinedSlipWheel2DOF_FZMIN) {
      rtb_xdot_o = Chassis_P.CombinedSlipWheel2DOF_FZMIN;
    }

    if (rtb_xdot_o > Chassis_P.CombinedSlipWheel2DOF_FZMAX) {
      rtb_xdot_o = Chassis_P.CombinedSlipWheel2DOF_FZMAX;
    }

    rtb_phidot = (rtb_xdot_o - Fzo_prime_idx_2) / Fzo_prime_idx_2;
    rtb_My[2] = (Chassis_P.CombinedSlipWheel2DOF_PECP2 * rtb_phidot + 1.0) * Chassis_P.CombinedSlipWheel2DOF_PECP1;
    lam_muy_prime_idx_3 = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[2] *
                          Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS / Chassis_P.CombinedSlipWheel2DOF_LONGVL;
    Chassis_B.Re[2] = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_Cz_0 = rtb_xdot[2] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    Chassis_B.Re[2] =
        (Chassis_P.CombinedSlipWheel2DOF_Q_V1 * Chassis_B.Re[2] + Chassis_P.CombinedSlipWheel2DOF_Q_RE0) *
            Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS -
        (std::atan(Chassis_P.CombinedSlipWheel2DOF_BREFF * lam_Cz_0) * Chassis_P.CombinedSlipWheel2DOF_DREFF +
         Chassis_P.CombinedSlipWheel2DOF_FREFF * lam_Cz_0) *
            (Chassis_P.CombinedSlipWheel2DOF_FNOMIN / rtb_Product_ju);
    if (Chassis_B.Re[2] < 0.001) {
      Chassis_B.Re[2] = 0.001;
    }

    Drphi_0 = Chassis_B.Re[2] * Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[2] -
              rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[2];
    rtb_Product_ju = Drphi_0 / rtb_a[2];
    if (rtb_Product_ju < Chassis_P.CombinedSlipWheel2DOF_KPUMIN) {
      rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_KPUMIN;
    }

    if (rtb_Product_ju > Chassis_P.CombinedSlipWheel2DOF_KPUMAX) {
      rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_KPUMAX;
    }

    lam_Hx[2] = (Chassis_P.CombinedSlipWheel2DOF_PHX2 * rtb_phidot + Chassis_P.CombinedSlipWheel2DOF_PHX1) *
                    rtb_VectorConcatenate_d[64] +
                rtb_Product_ju;
    rtb_Alpha_b = rt_atan2d_snf(rtb_ImpAsg_InsertedFor_ydotWheel_at_inport_0[2], rtb_a[2]);
    if (rtb_Alpha_b < Chassis_P.CombinedSlipWheel2DOF_ALPMIN) {
      rtb_Alpha_b = Chassis_P.CombinedSlipWheel2DOF_ALPMIN;
    }

    if (rtb_Alpha_b > Chassis_P.CombinedSlipWheel2DOF_ALPMAX) {
      rtb_Alpha_b = Chassis_P.CombinedSlipWheel2DOF_ALPMAX;
    }

    SHy_idx_2 = std::sin(rtb_Add_j[2]);
    lam_muy_prime_idx_3 = -rtb_a[2] * rtb_Product_ju;
    rtb_a_o = -rtb_a[2] * std::tan(rtb_Alpha_b);
    rtb_psidot = rtb_a_o * rtb_a_o;
    isLowSpeed[2] = (std::sqrt(Drphi_0 * Drphi_0 + rtb_psidot) < Chassis_P.CombinedSlipWheel2DOF_VXLOW);
    lam_muy_prime_idx_3 = std::sqrt(lam_muy_prime_idx_3 * lam_muy_prime_idx_3 + rtb_psidot) *
                              rtb_VectorConcatenate_d[57] / Chassis_P.CombinedSlipWheel2DOF_LONGVL +
                          1.0;
    lam_mux_tmp_0 = lam_mux[2] / lam_muy_prime_idx_3;
    rtb_Saturation[2] = rtb_phidot;
    rtb_xdot[2] = rtb_xdot_o;
    rtb_Kappa[2] = rtb_Product_ju;
    rtb_Alpha[2] = rtb_Alpha_b;
    gamma_star[2] = SHy_idx_2;
    Vc[2] = std::sqrt(
        rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[2] * rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[2] + rtb_psidot);
    lam_muV[2] = lam_muy_prime_idx_3;
    rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_VERTICAL_STIFFNESS * rtb_VectorConcatenate_d[103];
    rtb_xdot_o = rtb_xdot[3];
    if (rtb_Saturation[3] > 0.0) {
      rtb_Product_ju = Vs[iy];
    }

    FzUnSat[3] = rtb_xdot[3];
    if (rtb_xdot[3] < Chassis_P.CombinedSlipWheel2DOF_FZMIN) {
      rtb_xdot_o = Chassis_P.CombinedSlipWheel2DOF_FZMIN;
    }

    if (rtb_xdot_o > Chassis_P.CombinedSlipWheel2DOF_FZMAX) {
      rtb_xdot_o = Chassis_P.CombinedSlipWheel2DOF_FZMAX;
    }

    rtb_phidot = (rtb_xdot_o - rtb_thetadot) / rtb_thetadot;
    rtb_My[3] = (Chassis_P.CombinedSlipWheel2DOF_PECP2 * rtb_phidot + 1.0) * Chassis_P.CombinedSlipWheel2DOF_PECP1;
    lam_muy_prime_idx_3 = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[3] *
                          Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS / Chassis_P.CombinedSlipWheel2DOF_LONGVL;
    Chassis_B.Re[3] = lam_muy_prime_idx_3 * lam_muy_prime_idx_3;
    lam_Cz_0 = rtb_xdot[3] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    Chassis_B.Re[3] =
        (Chassis_P.CombinedSlipWheel2DOF_Q_V1 * Chassis_B.Re[3] + Chassis_P.CombinedSlipWheel2DOF_Q_RE0) *
            Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS -
        (std::atan(Chassis_P.CombinedSlipWheel2DOF_BREFF * lam_Cz_0) * Chassis_P.CombinedSlipWheel2DOF_DREFF +
         Chassis_P.CombinedSlipWheel2DOF_FREFF * lam_Cz_0) *
            (Chassis_P.CombinedSlipWheel2DOF_FNOMIN / rtb_Product_ju);
    if (Chassis_B.Re[3] < 0.001) {
      Chassis_B.Re[3] = 0.001;
    }

    Drphi_0 = Chassis_B.Re[3] * Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[3] -
              rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[3];
    rtb_Product_ju = Drphi_0 / rtb_a[3];
    if (rtb_Product_ju < Chassis_P.CombinedSlipWheel2DOF_KPUMIN) {
      rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_KPUMIN;
    }

    if (rtb_Product_ju > Chassis_P.CombinedSlipWheel2DOF_KPUMAX) {
      rtb_Product_ju = Chassis_P.CombinedSlipWheel2DOF_KPUMAX;
    }

    lam_Hx[3] = (Chassis_P.CombinedSlipWheel2DOF_PHX2 * rtb_phidot + Chassis_P.CombinedSlipWheel2DOF_PHX1) *
                    rtb_VectorConcatenate_d[91] +
                rtb_Product_ju;
    rtb_Alpha_b = rt_atan2d_snf(rtb_ImpAsg_InsertedFor_ydotWheel_at_inport_0[3], rtb_a[3]);
    if (rtb_Alpha_b < Chassis_P.CombinedSlipWheel2DOF_ALPMIN) {
      rtb_Alpha_b = Chassis_P.CombinedSlipWheel2DOF_ALPMIN;
    }

    if (rtb_Alpha_b > Chassis_P.CombinedSlipWheel2DOF_ALPMAX) {
      rtb_Alpha_b = Chassis_P.CombinedSlipWheel2DOF_ALPMAX;
    }

    gamma_star_tmp = std::sin(rtb_Add_n);
    lam_muy_prime_idx_3 = -rtb_a[3] * rtb_Product_ju;
    rtb_a_tmp = std::tan(rtb_Alpha_b);
    rtb_a_o = -rtb_a[3] * rtb_a_tmp;
    rtb_psidot = rtb_a_o * rtb_a_o;
    isLowSpeed[3] = (std::sqrt(Drphi_0 * Drphi_0 + rtb_psidot) < Chassis_P.CombinedSlipWheel2DOF_VXLOW);
    lam_muy_prime_idx_3 = std::sqrt(lam_muy_prime_idx_3 * lam_muy_prime_idx_3 + rtb_psidot) *
                              rtb_VectorConcatenate_d[84] / Chassis_P.CombinedSlipWheel2DOF_LONGVL +
                          1.0;
    rtb_Saturation[3] = rtb_phidot;
    rtb_xdot[3] = rtb_xdot_o;
    rtb_Kappa[3] = rtb_Product_ju;
    rtb_Alpha[3] = rtb_Alpha_b;
    gamma_star[3] = gamma_star_tmp;
    Vc[3] = std::sqrt(
        rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[3] * rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[3] + rtb_psidot);
    lam_mux_tmp_1 = lam_mux_0 / lam_muy_prime_idx_3;
    Chassis_div0protect(rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0, Chassis_P.CombinedSlipWheel2DOF_VXLOW, phi_t,
                        lam_Cz);
    Chassis_div0protect(Vc, Chassis_P.CombinedSlipWheel2DOF_VXLOW, cosprimealpha, lam_Cz);
    lam_mux_0 = rtb_sig_y[0] * rtb_sig_y[0];
    rtb_psidot = rtb_sig_x[0] / lam_muV[0];
    lam_muy_prime_idx_0 = rtb_psidot * 10.0 / (9.0 * rtb_psidot + 1.0);
    mu_y_idx_0 = SHy_idx_0 * SHy_idx_0;
    rtb_kxj = ((Chassis_P.CombinedSlipWheel2DOF_PPY3 * rtb_sig_y[0] + 1.0) +
               lam_mux_0 * Chassis_P.CombinedSlipWheel2DOF_PPY4) *
              (Chassis_P.CombinedSlipWheel2DOF_PDY2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_PDY1) *
              (1.0 - mu_y_idx_0 * Chassis_P.CombinedSlipWheel2DOF_PDY3) * rtb_psidot;
    rtb_sig_x[0] = rtb_psidot;
    phi_t[0] = rtb_AngVel_g[2] / phi_t[0] * std::cos(rtb_Alpha[0]);
    rtb_Gain2_e = rtb_sig_y[1] * rtb_sig_y[1];
    rtb_psidot = rtb_sig_x[1] / lam_muV[1];
    lam_muy_prime_idx_1 = rtb_psidot * 10.0 / (9.0 * rtb_psidot + 1.0);
    rtb_a_o = SHy_idx_1 * SHy_idx_1;
    rtb_jxi = ((Chassis_P.CombinedSlipWheel2DOF_PPY3 * rtb_sig_y[1] + 1.0) +
               rtb_Gain2_e * Chassis_P.CombinedSlipWheel2DOF_PPY4) *
              (Chassis_P.CombinedSlipWheel2DOF_PDY2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_PDY1) *
              (1.0 - rtb_a_o * Chassis_P.CombinedSlipWheel2DOF_PDY3) * rtb_psidot;
    rtb_sig_x[1] = rtb_psidot;
    phi_t[1] = rtb_AngVel_g[5] / phi_t[1] * std::cos(rtb_Alpha[1]);
    rtb_Product1_h = rtb_sig_y[2] * rtb_sig_y[2];
    rtb_psidot = rtb_sig_x[2] / lam_muV[2];
    lam_muy_prime_idx_2 = rtb_psidot * 10.0 / (9.0 * rtb_psidot + 1.0);
    mu_y_tmp = SHy_idx_2 * SHy_idx_2;
    rtb_Product2_l = ((Chassis_P.CombinedSlipWheel2DOF_PPY3 * rtb_sig_y[2] + 1.0) +
                      rtb_Product1_h * Chassis_P.CombinedSlipWheel2DOF_PPY4) *
                     (Chassis_P.CombinedSlipWheel2DOF_PDY2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_PDY1) *
                     (1.0 - mu_y_tmp * Chassis_P.CombinedSlipWheel2DOF_PDY3) * rtb_psidot;
    rtb_sig_x[2] = rtb_psidot;
    phi_t[2] = rtb_AngVel_g[8] / phi_t[2] * std::cos(rtb_Alpha[2]);
    Dx_tmp = rtb_sig_y[3] * rtb_sig_y[3];
    rtb_psidot = rtb_sig_x_f / lam_muy_prime_idx_3;
    lam_muy_prime_idx_3 = rtb_psidot * 10.0 / (9.0 * rtb_psidot + 1.0);
    mu_y_idx_1 = gamma_star_tmp * gamma_star_tmp;
    rtb_sig_x_f =
        ((Chassis_P.CombinedSlipWheel2DOF_PPY3 * rtb_sig_y[3] + 1.0) + Dx_tmp * Chassis_P.CombinedSlipWheel2DOF_PPY4) *
        (Chassis_P.CombinedSlipWheel2DOF_PDY2 * rtb_phidot + Chassis_P.CombinedSlipWheel2DOF_PDY1) *
        (1.0 - mu_y_idx_1 * Chassis_P.CombinedSlipWheel2DOF_PDY3) * rtb_psidot;
    phi_t[3] = rtb_AngVel_g[11] / phi_t[3] * std::cos(rtb_Alpha_b);
    iy = 0;
    if (isLowSpeed[0]) {
      iy = 1;
    }

    if (isLowSpeed[1]) {
      iy++;
    }

    if (isLowSpeed[2]) {
      iy++;
    }

    if (isLowSpeed[3]) {
      iy++;
    }

    n_size = iy;
    iy = 0;
    if (isLowSpeed[0]) {
      p_data[0] = 1;
      iy = 1;
    }

    if (isLowSpeed[1]) {
      p_data[iy] = 2;
      iy++;
    }

    if (isLowSpeed[2]) {
      p_data[iy] = 3;
      iy++;
    }

    if (isLowSpeed[3]) {
      p_data[iy] = 4;
    }

    iy = 0;
    if (isLowSpeed[0]) {
      iy = 1;
    }

    if (isLowSpeed[1]) {
      iy++;
    }

    if (isLowSpeed[2]) {
      iy++;
    }

    if (isLowSpeed[3]) {
      iy++;
    }

    o_size = iy;
    iy = 0;
    if (isLowSpeed[0]) {
      q_data[0] = 1;
      iy = 1;
    }

    if (isLowSpeed[1]) {
      q_data[iy] = 2;
      iy++;
    }

    if (isLowSpeed[2]) {
      q_data[iy] = 3;
      iy++;
    }

    if (isLowSpeed[3]) {
      q_data[iy] = 4;
    }

    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
      Kyalpha[rtb_phidot_tmp] = rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[p_data[rtb_phidot_tmp] - 1];
    }

    Chassis_abs(Kyalpha, &n_size, tmp_data, &tmp_size_1);
    if (o_size == tmp_size_1) {
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        Kyalpha[rtb_phidot_tmp] = rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[p_data[rtb_phidot_tmp] - 1];
      }

      Chassis_abs(Kyalpha, &n_size, tmp_data, &tmp_size_1);
      Itemp_tmp = o_size;
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < o_size; rtb_phidot_tmp++) {
        tmpDrphiVar_data[rtb_phidot_tmp] =
            phi_t[q_data[rtb_phidot_tmp] - 1] * (tmp_data[rtb_phidot_tmp] / Chassis_P.CombinedSlipWheel2DOF_VXLOW);
      }
    } else {
      Chassis_binary_expand_op_b3i5z2(tmpDrphiVar_data, &Itemp_tmp, phi_t, q_data, &o_size,
                                      rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0, p_data, &n_size,
                                      Chassis_P.CombinedSlipWheel2DOF_VXLOW);
    }

    iy = 0;
    Chassis_div0protect_p(Vc, Kyalpha);
    if (isLowSpeed[0]) {
      phi_t[0] = tmpDrphiVar_data[0];
      iy = 1;
    }

    rtb_UnaryMinus[0] = (-rtb_AngVel_g[2] - (1.0 - rtb_My[0]) * rtb_xdot_tmp * SHy_idx_0) * -(1.0 / Kyalpha[0]);
    if (isLowSpeed[1]) {
      phi_t[1] = tmpDrphiVar_data[iy];
      iy++;
    }

    rtb_UnaryMinus[1] = (-rtb_AngVel_g[5] - (1.0 - rtb_My[1]) * rtb_xdot_tmp_0 * SHy_idx_1) * -(1.0 / Kyalpha[1]);
    if (isLowSpeed[2]) {
      phi_t[2] = tmpDrphiVar_data[iy];
      iy++;
    }

    rtb_UnaryMinus[2] = (-rtb_AngVel_g[8] - (1.0 - rtb_My[2]) * rtb_xdot_tmp_1 * SHy_idx_2) * -(1.0 / Kyalpha[2]);
    if (isLowSpeed[3]) {
      phi_t[3] = tmpDrphiVar_data[iy];
    }

    rtb_UnaryMinus[3] = (-rtb_AngVel_g[11] - (1.0 - rtb_My[3]) * rtb_xdot_tmp_2 * gamma_star_tmp) * -(1.0 / Kyalpha[3]);
    if (Chassis_P.CombinedSlipWheel2DOF_turnslip == 1.0) {
      isLowSpeed[0] = (std::abs(rtb_UnaryMinus[0]) > 0.01);
      isLowSpeed[1] = (std::abs(rtb_UnaryMinus[1]) > 0.01);
      isLowSpeed[2] = (std::abs(rtb_UnaryMinus[2]) > 0.01);
      isLowSpeed[3] = (std::abs(rtb_UnaryMinus[3]) > 0.01);
    } else {
      isLowSpeed[0] = false;
      isLowSpeed[1] = false;
      isLowSpeed[2] = false;
      isLowSpeed[3] = false;
    }

    rtb_a[0] = Chassis_P.CombinedSlipWheel2DOF_RHY2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_RHY1;
    lam_Cz_0 = Chassis_P.CombinedSlipWheel2DOF_REY2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_REY1;
    lam_muV[0] = lam_Cz_0;
    if (lam_Cz_0 > 1.0) {
      lam_muV[0] = 1.0;
    }

    SHy_idx_3 = (mu_y_idx_0 * Chassis_P.CombinedSlipWheel2DOF_RBY4 + Chassis_P.CombinedSlipWheel2DOF_RBY1) *
                std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RBY2 * rtb_Alpha[0])) * rtb_VectorConcatenate_d[19];
    if (SHy_idx_3 < 0.0) {
      SHy_idx_3 = 0.0;
    }

    rtb_b[0] = SHy_idx_3;
    rtb_a[1] = Chassis_P.CombinedSlipWheel2DOF_RHY2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_RHY1;
    lam_Cz_0 = Chassis_P.CombinedSlipWheel2DOF_REY2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_REY1;
    lam_muV[1] = lam_Cz_0;
    if (lam_Cz_0 > 1.0) {
      lam_muV[1] = 1.0;
    }

    SHy_idx_3 = (rtb_a_o * Chassis_P.CombinedSlipWheel2DOF_RBY4 + Chassis_P.CombinedSlipWheel2DOF_RBY1) *
                std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RBY2 * rtb_Alpha[1])) * rtb_VectorConcatenate_d[46];
    if (SHy_idx_3 < 0.0) {
      SHy_idx_3 = 0.0;
    }

    rtb_b[1] = SHy_idx_3;
    rtb_a[2] = Chassis_P.CombinedSlipWheel2DOF_RHY2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_RHY1;
    lam_Cz_0 = Chassis_P.CombinedSlipWheel2DOF_REY2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_REY1;
    lam_muV[2] = lam_Cz_0;
    if (lam_Cz_0 > 1.0) {
      lam_muV[2] = 1.0;
    }

    SHy_idx_3 = (mu_y_tmp * Chassis_P.CombinedSlipWheel2DOF_RBY4 + Chassis_P.CombinedSlipWheel2DOF_RBY1) *
                std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RBY2 * rtb_Alpha[2])) * rtb_VectorConcatenate_d[73];
    if (SHy_idx_3 < 0.0) {
      SHy_idx_3 = 0.0;
    }

    rtb_b[2] = SHy_idx_3;
    rtb_a[3] = Chassis_P.CombinedSlipWheel2DOF_RHY2 * rtb_phidot + Chassis_P.CombinedSlipWheel2DOF_RHY1;
    lam_Cz_0 = Chassis_P.CombinedSlipWheel2DOF_REY2 * rtb_phidot + Chassis_P.CombinedSlipWheel2DOF_REY1;
    lam_muV[3] = lam_Cz_0;
    if (lam_Cz_0 > 1.0) {
      lam_muV[3] = 1.0;
    }

    SHy_idx_3 = (mu_y_idx_1 * Chassis_P.CombinedSlipWheel2DOF_RBY4 + Chassis_P.CombinedSlipWheel2DOF_RBY1) *
                std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RBY2 * rtb_Alpha_b)) * rtb_VectorConcatenate_d[100];
    if (SHy_idx_3 < 0.0) {
      SHy_idx_3 = 0.0;
    }

    rtb_b[3] = SHy_idx_3;
    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 36; rtb_phidot_tmp++) {
      zeta[rtb_phidot_tmp] = 1.0;
    }

    iy = 0;
    if (isLowSpeed[0]) {
      iy = 1;
    }

    if (isLowSpeed[1]) {
      iy++;
    }

    if (isLowSpeed[2]) {
      iy++;
    }

    if (isLowSpeed[3]) {
      iy++;
    }

    n_size = iy;
    iy = 0;
    if (isLowSpeed[0]) {
      r_data[0] = 1;
      iy = 1;
    }

    if (isLowSpeed[1]) {
      r_data[iy] = 2;
      iy++;
    }

    if (isLowSpeed[2]) {
      r_data[iy] = 3;
      iy++;
    }

    if (isLowSpeed[3]) {
      r_data[iy] = 4;
    }

    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
      zeta[9 * (r_data[rtb_phidot_tmp] - 1)] = 0.0;
    }

    if (Chassis_any(isLowSpeed)) {
      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      b_x[0] = std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_PDXP3 * rtb_Kappa[0]));
      Vc[0] = std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_PDYP3 * std::tan(rtb_Alpha[0])));
      if (isLowSpeed[1]) {
        iy++;
      }

      b_x[1] = std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_PDXP3 * rtb_Kappa[1]));
      Vc[1] = std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_PDYP3 * std::tan(rtb_Alpha[1])));
      if (isLowSpeed[2]) {
        iy++;
      }

      b_x[2] = std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_PDXP3 * rtb_Kappa[2]));
      Vc[2] = std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_PDYP3 * std::tan(rtb_Alpha[2])));
      if (isLowSpeed[3]) {
        iy++;
      }

      b_x[3] = std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_PDXP3 * rtb_Product_ju));
      Vc[3] = std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_PDYP3 * rtb_a_tmp));
      n_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        s_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        s_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        s_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        s_data[iy] = 4;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      ibmat = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        t_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        t_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        t_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        t_data[iy] = 4;
      }

      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        s = s_data[rtb_phidot_tmp];
        tmpDrphiVar_data[rtb_phidot_tmp] = (rtb_Saturation[s - 1] * Chassis_P.CombinedSlipWheel2DOF_PDXP2 + 1.0) *
                                           Chassis_P.CombinedSlipWheel2DOF_PDXP1 * b_x[s - 1] *
                                           Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_UnaryMinus[s - 1];
      }

      Chassis_atan(tmpDrphiVar_data, &n_size);
      Chassis_cos(tmpDrphiVar_data, &n_size);
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < ibmat; rtb_phidot_tmp++) {
        zeta[9 * (t_data[rtb_phidot_tmp] - 1) + 1] = tmpDrphiVar_data[rtb_phidot_tmp];
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      o_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        u_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        u_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        u_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        u_data[iy] = 4;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      ibmat = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        v_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        v_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        v_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        v_data[iy] = 4;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      w_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        w_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        w_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        w_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        w_data[iy] = 4;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      n_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        x_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        x_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        x_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        x_data[iy] = 4;
      }

      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < ibmat; rtb_phidot_tmp++) {
        Kyalpha[rtb_phidot_tmp] = rtb_UnaryMinus[v_data[rtb_phidot_tmp] - 1];
      }

      Chassis_abs(Kyalpha, &ibmat, tmp_data, &tmp_size_1);
      Itemp_tmp = tmp_size_1;
      iy = (tmp_size_1 / 2) << 1;
      ibmat = iy - 2;
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= ibmat; rtb_phidot_tmp += 2) {
        tmp_5 = _mm_loadu_pd(&tmp_data[rtb_phidot_tmp]);
        _mm_storeu_pd(&tmpDrphiVar_data[rtb_phidot_tmp],
                      _mm_mul_pd(_mm_set1_pd(Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS), tmp_5));
      }

      for (rtb_phidot_tmp = iy; rtb_phidot_tmp < tmp_size_1; rtb_phidot_tmp++) {
        tmpDrphiVar_data[rtb_phidot_tmp] = Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * tmp_data[rtb_phidot_tmp];
      }

      Chassis_sqrt_c(tmpDrphiVar_data, &tmp_size_1);
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < o_size; rtb_phidot_tmp++) {
        Kyalpha[rtb_phidot_tmp] = rtb_UnaryMinus[u_data[rtb_phidot_tmp] - 1];
      }

      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < o_size; rtb_phidot_tmp++) {
        Gykappa[rtb_phidot_tmp] = rtb_UnaryMinus[u_data[rtb_phidot_tmp] - 1];
      }

      Chassis_abs(Gykappa, &o_size, tmp_data, &tmp_size_1);
      Chassis_abs(Kyalpha, &o_size, tmp_data, &iy);
      if ((iy == Itemp_tmp) && ((tmp_size_1 == 1 ? Itemp_tmp : tmp_size_1) == w_size)) {
        for (rtb_phidot_tmp = 0; rtb_phidot_tmp < o_size; rtb_phidot_tmp++) {
          Kyalpha[rtb_phidot_tmp] = rtb_UnaryMinus[u_data[rtb_phidot_tmp] - 1];
        }

        Chassis_abs(Kyalpha, &o_size, tmp_data, &tmp_size_1);
        iy = w_size;
        for (rtb_phidot_tmp = 0; rtb_phidot_tmp < w_size; rtb_phidot_tmp++) {
          s = w_data[rtb_phidot_tmp];
          Vs[rtb_phidot_tmp] = (rtb_Saturation[s - 1] * Chassis_P.CombinedSlipWheel2DOF_PDYP2 + 1.0) *
                               Chassis_P.CombinedSlipWheel2DOF_PDYP1 * Vc[s - 1] *
                               (Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * tmp_data[rtb_phidot_tmp] +
                                Chassis_P.CombinedSlipWheel2DOF_PDYP4 * tmpDrphiVar_data[rtb_phidot_tmp]);
        }
      } else {
        Chassis_binary_expand_op_b3i5z(Vs, &iy, Chassis_P.CombinedSlipWheel2DOF_PDYP1,
                                       Chassis_P.CombinedSlipWheel2DOF_PDYP2, rtb_Saturation, Vc, w_data, &w_size,
                                       Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS, rtb_UnaryMinus, u_data, &o_size,
                                       Chassis_P.CombinedSlipWheel2DOF_PDYP4, tmpDrphiVar_data, &Itemp_tmp);
      }

      Chassis_atan(Vs, &iy);
      Chassis_cos(Vs, &iy);
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        zeta[9 * (x_data[rtb_phidot_tmp] - 1) + 2] = Vs[rtb_phidot_tmp];
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      n_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        y_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        y_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        y_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        y_data[iy] = 4;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      ibmat = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        ab_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        ab_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        ab_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        ab_data[iy] = 4;
      }

      Drphi_0 = Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS *
                Chassis_P.CombinedSlipWheel2DOF_PKYP1;
      Itemp_tmp = n_size;
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        rtb_phidot = rtb_UnaryMinus[y_data[rtb_phidot_tmp] - 1];
        tmpDrphiVar_data[rtb_phidot_tmp] = rtb_phidot * rtb_phidot * Drphi_0;
      }

      Chassis_atan(tmpDrphiVar_data, &n_size);
      Chassis_cos(tmpDrphiVar_data, &n_size);
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < ibmat; rtb_phidot_tmp++) {
        zeta[9 * (ab_data[rtb_phidot_tmp] - 1) + 3] = tmpDrphiVar_data[rtb_phidot_tmp];
      }
    }

    Drphi_0 = 1.0 - Chassis_P.CombinedSlipWheel2DOF_PKY3 * 0.0;
    rtb_phidot = rtb_xdot[0] / rtb_UnaryMinus_i;
    rtb_a_tmp = (Chassis_P.CombinedSlipWheel2DOF_PPY1 * rtb_sig_y[0] + 1.0) *
                (Chassis_P.CombinedSlipWheel2DOF_PKY1 * rtb_UnaryMinus_i);
    lam_Cz_0 =
        std::sin(std::atan(rtb_phidot /
                           (mu_y_idx_0 * Chassis_P.CombinedSlipWheel2DOF_PKY5 + Chassis_P.CombinedSlipWheel2DOF_PKY2) /
                           (Chassis_P.CombinedSlipWheel2DOF_PPY2 * rtb_sig_y[0] + 1.0)) *
                 Chassis_P.CombinedSlipWheel2DOF_PKY4);
    rtb_xdot_tmp = (1.0 - Chassis_P.CombinedSlipWheel2DOF_PKY3 * std::abs(SHy_idx_0)) * rtb_a_tmp * lam_Cz_0 * zeta[3] *
                   rtb_VectorConcatenate_d[5];
    Vc[0] = rtb_a_tmp * Drphi_0 * lam_Cz_0 * zeta[3] * rtb_VectorConcatenate_d[5];
    Kyalpha[0] = rtb_xdot_tmp;
    Mzphiinf[0] = rtb_phidot;
    rtb_phidot = rtb_xdot[1] / rtb_Add1_b_idx_1;
    rtb_a_tmp = (Chassis_P.CombinedSlipWheel2DOF_PPY1 * rtb_sig_y[1] + 1.0) *
                (Chassis_P.CombinedSlipWheel2DOF_PKY1 * rtb_Add1_b_idx_1);
    lam_Cz_0 = std::sin(
        std::atan(rtb_phidot / (rtb_a_o * Chassis_P.CombinedSlipWheel2DOF_PKY5 + Chassis_P.CombinedSlipWheel2DOF_PKY2) /
                  (Chassis_P.CombinedSlipWheel2DOF_PPY2 * rtb_sig_y[1] + 1.0)) *
        Chassis_P.CombinedSlipWheel2DOF_PKY4);
    rtb_xdot_tmp_0 = (1.0 - Chassis_P.CombinedSlipWheel2DOF_PKY3 * std::abs(SHy_idx_1)) * rtb_a_tmp * lam_Cz_0 *
                     zeta[12] * rtb_VectorConcatenate_d[32];
    Vc[1] = rtb_a_tmp * Drphi_0 * lam_Cz_0 * zeta[12] * rtb_VectorConcatenate_d[32];
    Kyalpha[1] = rtb_xdot_tmp_0;
    Mzphiinf[1] = rtb_phidot;
    rtb_phidot = rtb_xdot[2] / Fzo_prime_idx_2;
    rtb_a_tmp = (Chassis_P.CombinedSlipWheel2DOF_PPY1 * rtb_sig_y[2] + 1.0) *
                (Chassis_P.CombinedSlipWheel2DOF_PKY1 * Fzo_prime_idx_2);
    lam_Cz_0 =
        std::sin(std::atan(rtb_phidot /
                           (mu_y_tmp * Chassis_P.CombinedSlipWheel2DOF_PKY5 + Chassis_P.CombinedSlipWheel2DOF_PKY2) /
                           (Chassis_P.CombinedSlipWheel2DOF_PPY2 * rtb_sig_y[2] + 1.0)) *
                 Chassis_P.CombinedSlipWheel2DOF_PKY4);
    rtb_xdot_tmp_1 = (1.0 - Chassis_P.CombinedSlipWheel2DOF_PKY3 * std::abs(SHy_idx_2)) * rtb_a_tmp * lam_Cz_0 *
                     zeta[21] * rtb_VectorConcatenate_d[59];
    Vc[2] = rtb_a_tmp * Drphi_0 * lam_Cz_0 * zeta[21] * rtb_VectorConcatenate_d[59];
    Kyalpha[2] = rtb_xdot_tmp_1;
    Mzphiinf[2] = rtb_phidot;
    rtb_phidot = rtb_xdot_o / rtb_thetadot;
    rtb_a_tmp = (Chassis_P.CombinedSlipWheel2DOF_PPY1 * rtb_sig_y[3] + 1.0) *
                (Chassis_P.CombinedSlipWheel2DOF_PKY1 * rtb_thetadot);
    lam_Cz_0 =
        std::sin(std::atan(rtb_phidot /
                           (mu_y_idx_1 * Chassis_P.CombinedSlipWheel2DOF_PKY5 + Chassis_P.CombinedSlipWheel2DOF_PKY2) /
                           (Chassis_P.CombinedSlipWheel2DOF_PPY2 * rtb_sig_y[3] + 1.0)) *
                 Chassis_P.CombinedSlipWheel2DOF_PKY4);
    rtb_xdot_tmp_2 = (1.0 - Chassis_P.CombinedSlipWheel2DOF_PKY3 * std::abs(gamma_star_tmp)) * rtb_a_tmp * lam_Cz_0 *
                     zeta[30] * rtb_VectorConcatenate_d[86];
    Vc[3] = rtb_a_tmp * Drphi_0 * lam_Cz_0 * zeta[30] * rtb_VectorConcatenate_d[86];
    Kyalpha[3] = rtb_xdot_tmp_2;
    Chassis_div0protect(Kyalpha, 0.0001, b_x, lam_Cz);
    Chassis_div0protect(Vc, 0.0001, Drphi, lam_Cz);
    b_idx_0 = Chassis_P.CombinedSlipWheel2DOF_PHYP1;
    if (Chassis_P.CombinedSlipWheel2DOF_PHYP1 < 0.0) {
      b_idx_0 = 0.0;
    }

    lam_Cz_tmp = std::tanh(rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[0]);
    lam_Cz[0] = (Chassis_P.CombinedSlipWheel2DOF_PHYP3 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_PHYP2) *
                lam_Cz_tmp;
    SHy_idx_3 = std::tanh(rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[1]);
    lam_Cz[1] =
        (Chassis_P.CombinedSlipWheel2DOF_PHYP3 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_PHYP2) * SHy_idx_3;
    lam_Cz_0 = std::tanh(rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[2]);
    lam_Cz[2] =
        (Chassis_P.CombinedSlipWheel2DOF_PHYP3 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_PHYP2) * lam_Cz_0;
    Drphi_0 = std::tanh(rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[3]);
    mu_y_idx_2 =
        (Chassis_P.CombinedSlipWheel2DOF_PHYP3 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_PHYP2) * Drphi_0;
    lam_Cz[3] = mu_y_idx_2;
    rtb_a_tmp = Chassis_P.CombinedSlipWheel2DOF_PHYP4;
    if (Chassis_P.CombinedSlipWheel2DOF_PHYP4 > 1.0) {
      rtb_a_tmp = 1.0;
    }

    Vs[0] = (Chassis_P.CombinedSlipWheel2DOF_PKY7 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_PKY6) *
            rtb_xdot[0] * (Chassis_P.CombinedSlipWheel2DOF_PPY5 * rtb_sig_y[0] + 1.0) * rtb_VectorConcatenate_d[14] /
            (1.0 - rtb_My[0]) / (b_idx_0 * lam_Cz[0] * Drphi[0]);
    Kyalpha[0] = Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_UnaryMinus[0];
    Vs[1] = (Chassis_P.CombinedSlipWheel2DOF_PKY7 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_PKY6) *
            rtb_xdot[1] * (Chassis_P.CombinedSlipWheel2DOF_PPY5 * rtb_sig_y[1] + 1.0) * rtb_VectorConcatenate_d[41] /
            (1.0 - rtb_My[1]) / (b_idx_0 * lam_Cz[1] * Drphi[1]);
    Kyalpha[1] = Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_UnaryMinus[1];
    Vs[2] = (Chassis_P.CombinedSlipWheel2DOF_PKY7 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_PKY6) *
            rtb_xdot[2] * (Chassis_P.CombinedSlipWheel2DOF_PPY5 * rtb_sig_y[2] + 1.0) * rtb_VectorConcatenate_d[68] /
            (1.0 - rtb_My[2]) / (b_idx_0 * lam_Cz[2] * Drphi[2]);
    Kyalpha[2] = Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_UnaryMinus[2];
    Vs[3] = (Chassis_P.CombinedSlipWheel2DOF_PKY7 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_PKY6) *
            rtb_xdot_o * (Chassis_P.CombinedSlipWheel2DOF_PPY5 * rtb_sig_y[3] + 1.0) * rtb_VectorConcatenate_d[95] /
            (1.0 - rtb_My[3]) / (mu_y_idx_2 * b_idx_0 * Drphi[3]);
    Kyalpha[3] = Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_UnaryMinus[3];
    Chassis_magicsin(lam_Cz, b_idx_0, Vs, rtb_a_tmp, Kyalpha, tmp_data);
    Vc[0] = tmp_data[0] * lam_Cz_tmp;
    SVygamma[0] = (Chassis_P.CombinedSlipWheel2DOF_PVY4 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_PVY3) *
                  rtb_xdot[0] * SHy_idx_0 * zeta[2] * rtb_VectorConcatenate_d[14] * lam_muy_prime_idx_0;
    SHy_idx_0 = 0.0;
    Gykappa[0] = rtb_Kappa[0] + rtb_a[0];
    Vc[1] = tmp_data[1] * SHy_idx_3;
    SVygamma[1] = (Chassis_P.CombinedSlipWheel2DOF_PVY4 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_PVY3) *
                  rtb_xdot[1] * SHy_idx_1 * zeta[11] * rtb_VectorConcatenate_d[41] * lam_muy_prime_idx_1;
    SHy_idx_1 = 0.0;
    Gykappa[1] = rtb_Kappa[1] + rtb_a[1];
    Vc[2] = tmp_data[2] * lam_Cz_0;
    SVygamma[2] = (Chassis_P.CombinedSlipWheel2DOF_PVY4 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_PVY3) *
                  rtb_xdot[2] * SHy_idx_2 * zeta[20] * rtb_VectorConcatenate_d[68] * lam_muy_prime_idx_2;
    SHy_idx_2 = 0.0;
    Gykappa[2] = rtb_Kappa[2] + rtb_a[2];
    Vc[3] = tmp_data[3] * Drphi_0;
    SVygamma[3] = (Chassis_P.CombinedSlipWheel2DOF_PVY4 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_PVY3) *
                  rtb_xdot_o * gamma_star_tmp * zeta[29] * rtb_VectorConcatenate_d[95] * lam_muy_prime_idx_3;
    SHy_idx_3 = 0.0;
    Gykappa[3] = rtb_Product_ju + rtb_a[3];
    Chassis_magiccos(Chassis_P.CombinedSlipWheel2DOF_RCY1, rtb_b, lam_muV, Gykappa, Kyalpha);
    Chassis_magiccos(Chassis_P.CombinedSlipWheel2DOF_RCY1, rtb_b, lam_muV, rtb_a, tmp_data);
    Drphi_0 = Kyalpha[0] / tmp_data[0];
    if (Drphi_0 < 0.0) {
      Drphi_0 = 0.0;
    }

    Gykappa[0] = Drphi_0;
    Drphi_0 = Kyalpha[1] / tmp_data[1];
    if (Drphi_0 < 0.0) {
      Drphi_0 = 0.0;
    }

    Gykappa[1] = Drphi_0;
    Drphi_0 = Kyalpha[2] / tmp_data[2];
    if (Drphi_0 < 0.0) {
      Drphi_0 = 0.0;
    }

    Gykappa[2] = Drphi_0;
    Drphi_0 = Kyalpha[3] / tmp_data[3];
    if (Drphi_0 < 0.0) {
      Drphi_0 = 0.0;
    }

    Gykappa[3] = Drphi_0;
    if (Chassis_any(isLowSpeed)) {
      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      rtb_phidot_tmp = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        bb_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        bb_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        bb_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        bb_data[iy] = 4;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      tmp_size_1 = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        cb_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        cb_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        cb_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        cb_data[iy] = 4;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      Itemp_tmp = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        db_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        db_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        db_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        db_data[iy] = 4;
      }

      if (tmp_size_1 == rtb_phidot_tmp) {
        for (rtb_phidot_tmp = 0; rtb_phidot_tmp < Itemp_tmp; rtb_phidot_tmp++) {
          s = bb_data[rtb_phidot_tmp];
          zeta[9 * (db_data[rtb_phidot_tmp] - 1) + 4] =
              (Vc[cb_data[rtb_phidot_tmp] - 1] + 1.0) - SVygamma[s - 1] / b_x[s - 1];
        }
      } else {
        Chassis_binary_expand_op_b3i5(zeta, db_data, &Itemp_tmp, Vc, cb_data, &tmp_size_1, SVygamma, bb_data,
                                      &rtb_phidot_tmp, b_x);
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      n_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        eb_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        eb_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        eb_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        eb_data[iy] = 4;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      ibmat = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        fb_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        fb_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        fb_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        fb_data[iy] = 4;
      }

      Drphi_0 = Chassis_P.CombinedSlipWheel2DOF_QDTP1 * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS;
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        tmpDrphiVar_data[rtb_phidot_tmp] = rtb_UnaryMinus[eb_data[rtb_phidot_tmp] - 1] * Drphi_0;
      }

      Chassis_atan(tmpDrphiVar_data, &n_size);
      Chassis_cos(tmpDrphiVar_data, &n_size);
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < ibmat; rtb_phidot_tmp++) {
        zeta[9 * (fb_data[rtb_phidot_tmp] - 1) + 5] = tmpDrphiVar_data[rtb_phidot_tmp];
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      n_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        gb_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        gb_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        gb_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        gb_data[iy] = 4;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      ibmat = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        hb_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        hb_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        hb_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        hb_data[iy] = 4;
      }

      Drphi_0 = Chassis_P.CombinedSlipWheel2DOF_QBRP1 * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS;
      Itemp_tmp = n_size;
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        tmpDrphiVar_data[rtb_phidot_tmp] = rtb_UnaryMinus[gb_data[rtb_phidot_tmp] - 1] * Drphi_0;
      }

      Chassis_atan(tmpDrphiVar_data, &n_size);
      Chassis_cos(tmpDrphiVar_data, &n_size);
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < ibmat; rtb_phidot_tmp++) {
        zeta[9 * (hb_data[rtb_phidot_tmp] - 1) + 6] = tmpDrphiVar_data[rtb_phidot_tmp];
      }

      Drphi_0 = Chassis_P.CombinedSlipWheel2DOF_QCRP2 * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS;
      gamma_star_tmp = Chassis_P.CombinedSlipWheel2DOF_QCRP1 * rtb_kxj *
                       Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_xdot[0] * std::sqrt(Mzphiinf[0]) *
                       rtb_VectorConcatenate_d[26];
      if (gamma_star_tmp < 0.0) {
        gamma_star_tmp = 1.0E-6;
      }

      Mzphiinf[0] = gamma_star_tmp;
      rtb_b[0] = std::atan(Drphi_0 * std::abs(phi_t[0]));
      gamma_star_tmp = Chassis_P.CombinedSlipWheel2DOF_QCRP1 * rtb_jxi *
                       Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_xdot[1] * std::sqrt(Mzphiinf[1]) *
                       rtb_VectorConcatenate_d[53];
      if (gamma_star_tmp < 0.0) {
        gamma_star_tmp = 1.0E-6;
      }

      Mzphiinf[1] = gamma_star_tmp;
      rtb_b[1] = std::atan(Drphi_0 * std::abs(phi_t[1]));
      gamma_star_tmp = Chassis_P.CombinedSlipWheel2DOF_QCRP1 * rtb_Product2_l *
                       Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_xdot[2] * std::sqrt(Mzphiinf[2]) *
                       rtb_VectorConcatenate_d[80];
      if (gamma_star_tmp < 0.0) {
        gamma_star_tmp = 1.0E-6;
      }

      Mzphiinf[2] = gamma_star_tmp;
      rtb_b[2] = std::atan(Drphi_0 * std::abs(phi_t[2]));
      gamma_star_tmp = Chassis_P.CombinedSlipWheel2DOF_QCRP1 * rtb_sig_x_f *
                       Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_xdot_o * std::sqrt(rtb_phidot) *
                       rtb_VectorConcatenate_d[107];
      if (gamma_star_tmp < 0.0) {
        gamma_star_tmp = 1.0E-6;
      }

      Mzphiinf[3] = gamma_star_tmp;
      rtb_b[3] = std::atan(Drphi_0 * std::abs(phi_t[3]));
      b_idx_0 = Chassis_P.CombinedSlipWheel2DOF_QDRP1;
      rtb_a_tmp = Chassis_P.CombinedSlipWheel2DOF_QDRP2;
      if (Chassis_P.CombinedSlipWheel2DOF_QDRP1 < 0.0) {
        b_idx_0 = 0.0;
      }

      if (Chassis_P.CombinedSlipWheel2DOF_QDRP2 < 0.0) {
        rtb_a_tmp = 0.0;
      }

      rtb_phidot = std::sin(1.5707963267948966 * b_idx_0);
      lam_Cz_0 = Mzphiinf[0] / rtb_phidot;
      if (std::isnan(1.0 - rtb_My[0])) {
        Drphi_0 = (rtNaN);
      } else if (1.0 - rtb_My[0] < 0.0) {
        Drphi_0 = -1.0;
      } else {
        Drphi_0 = (1.0 - rtb_My[0] > 0.0);
      }

      Vs[0] = ((Chassis_P.CombinedSlipWheel2DOF_QDZ11 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_QDZ10) *
                   std::abs(rtb_Add_j[0]) +
               (Chassis_P.CombinedSlipWheel2DOF_QDZ9 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_QDZ8)) *
              (rtb_xdot[0] * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS) * rtb_VectorConcatenate_d[15] /
              (b_idx_0 * lam_Cz_0 * (1.0 - rtb_My[0]) + 0.0001 * Drphi_0);
      Kyalpha[0] = -Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_UnaryMinus[0];
      lam_Cz[0] = lam_Cz_0;
      lam_Cz_0 = Mzphiinf[1] / rtb_phidot;
      if (std::isnan(1.0 - rtb_My[1])) {
        Drphi_0 = (rtNaN);
      } else if (1.0 - rtb_My[1] < 0.0) {
        Drphi_0 = -1.0;
      } else {
        Drphi_0 = (1.0 - rtb_My[1] > 0.0);
      }

      Vs[1] = ((Chassis_P.CombinedSlipWheel2DOF_QDZ11 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_QDZ10) *
                   std::abs(rtb_Add_j[1]) +
               (Chassis_P.CombinedSlipWheel2DOF_QDZ9 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_QDZ8)) *
              (rtb_xdot[1] * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS) * rtb_VectorConcatenate_d[42] /
              (b_idx_0 * lam_Cz_0 * (1.0 - rtb_My[1]) + 0.0001 * Drphi_0);
      Kyalpha[1] = -Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_UnaryMinus[1];
      lam_Cz[1] = lam_Cz_0;
      lam_Cz_0 = Mzphiinf[2] / rtb_phidot;
      if (std::isnan(1.0 - rtb_My[2])) {
        Drphi_0 = (rtNaN);
      } else if (1.0 - rtb_My[2] < 0.0) {
        Drphi_0 = -1.0;
      } else {
        Drphi_0 = (1.0 - rtb_My[2] > 0.0);
      }

      Vs[2] = ((Chassis_P.CombinedSlipWheel2DOF_QDZ11 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_QDZ10) *
                   std::abs(rtb_Add_j[2]) +
               (Chassis_P.CombinedSlipWheel2DOF_QDZ9 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_QDZ8)) *
              (rtb_xdot[2] * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS) * rtb_VectorConcatenate_d[69] /
              (b_idx_0 * lam_Cz_0 * (1.0 - rtb_My[2]) + 0.0001 * Drphi_0);
      Kyalpha[2] = -Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_UnaryMinus[2];
      lam_Cz[2] = lam_Cz_0;
      lam_Cz_0 = gamma_star_tmp / rtb_phidot;
      if (std::isnan(1.0 - rtb_My[3])) {
        Drphi_0 = (rtNaN);
      } else if (1.0 - rtb_My[3] < 0.0) {
        Drphi_0 = -1.0;
      } else {
        Drphi_0 = (1.0 - rtb_My[3] > 0.0);
      }

      Vs[3] = ((Chassis_P.CombinedSlipWheel2DOF_QDZ11 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_QDZ10) *
                   std::abs(rtb_Add_n) +
               (Chassis_P.CombinedSlipWheel2DOF_QDZ9 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_QDZ8)) *
              (rtb_xdot_o * Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS) * rtb_VectorConcatenate_d[96] /
              (b_idx_0 * lam_Cz_0 * (1.0 - rtb_My[3]) + 0.0001 * Drphi_0);
      Kyalpha[3] = -Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_UnaryMinus[3];
      lam_Cz[3] = lam_Cz_0;
      Chassis_magicsin(lam_Cz, b_idx_0, Vs, rtb_a_tmp, Kyalpha, Drphi);
      iy = 0;
      if (std::isnan(Drphi[0])) {
        lam_Cz_0 = (rtNaN);
      } else if (Drphi[0] < 0.0) {
        lam_Cz_0 = -1.0;
      } else {
        lam_Cz_0 = (Drphi[0] > 0.0);
      }

      if (lam_Cz_0 == 0.0) {
        lam_Cz_0 = 1.0;
      }

      if (isLowSpeed[0]) {
        iy = 1;
      }

      lam_Cz[0] = lam_Cz_0;
      if (std::isnan(Drphi[1])) {
        lam_Cz_0 = (rtNaN);
      } else if (Drphi[1] < 0.0) {
        lam_Cz_0 = -1.0;
      } else {
        lam_Cz_0 = (Drphi[1] > 0.0);
      }

      if (lam_Cz_0 == 0.0) {
        lam_Cz_0 = 1.0;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      lam_Cz[1] = lam_Cz_0;
      if (std::isnan(Drphi[2])) {
        lam_Cz_0 = (rtNaN);
      } else if (Drphi[2] < 0.0) {
        lam_Cz_0 = -1.0;
      } else {
        lam_Cz_0 = (Drphi[2] > 0.0);
      }

      if (lam_Cz_0 == 0.0) {
        lam_Cz_0 = 1.0;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      lam_Cz[2] = lam_Cz_0;
      if (std::isnan(Drphi[3])) {
        lam_Cz_0 = (rtNaN);
      } else if (Drphi[3] < 0.0) {
        lam_Cz_0 = -1.0;
      } else {
        lam_Cz_0 = (Drphi[3] > 0.0);
      }

      if (lam_Cz_0 == 0.0) {
        lam_Cz_0 = 1.0;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      lam_Cz[3] = lam_Cz_0;
      n_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        ib_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        ib_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        ib_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        ib_data[iy] = 4;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      o_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        kb_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        kb_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        kb_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        kb_data[iy] = 4;
      }

      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        Vs[rtb_phidot_tmp] = Drphi[ib_data[rtb_phidot_tmp] - 1];
      }

      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        Kyalpha[rtb_phidot_tmp] = Drphi[ib_data[rtb_phidot_tmp] - 1];
      }

      Chassis_abs(Kyalpha, &n_size, tmp_data, &tmp_size_1);
      Chassis_abs(Vs, &n_size, tmp_data, &iy);
      if ((iy == o_size) && ((tmp_size_1 == 1 ? o_size : tmp_size_1) == o_size)) {
        for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
          Vs[rtb_phidot_tmp] = Drphi[ib_data[rtb_phidot_tmp] - 1];
        }

        Chassis_abs(Vs, &n_size, tmp_data, &tmp_size_1);
        Itemp_tmp = o_size;
        for (rtb_phidot_tmp = 0; rtb_phidot_tmp < o_size; rtb_phidot_tmp++) {
          s = kb_data[rtb_phidot_tmp];
          tmpDrphiVar_data[rtb_phidot_tmp] = Mzphiinf[s - 1] * 2.0 / 3.1415926535897931 * rtb_b[s - 1] *
                                             Gykappa[s - 1] / (lam_Cz[s - 1] * 0.0001 - tmp_data[rtb_phidot_tmp]);
        }
      } else {
        Chassis_binary_expand_op_b3i(tmpDrphiVar_data, &Itemp_tmp, Mzphiinf, rtb_b, Gykappa, kb_data, &o_size, Drphi,
                                     ib_data, &n_size, lam_Cz);
      }

      for (ibmat = 0; ibmat < Itemp_tmp; ibmat++) {
        rtb_phidot = tmpDrphiVar_data[ibmat];
        if (rtb_phidot > 1.0) {
          rtb_phidot = 1.0;
        }

        tmpDrphiVar_data[ibmat] = rtb_phidot;
      }

      for (ibmat = 0; ibmat < Itemp_tmp; ibmat++) {
        rtb_phidot = tmpDrphiVar_data[ibmat];
        if (rtb_phidot < -1.0) {
          rtb_phidot = -1.0;
        }

        tmpDrphiVar_data[ibmat] = rtb_phidot;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      n_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        nb_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        nb_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        nb_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        nb_data[iy] = 4;
      }

      Chassis_acos(tmpDrphiVar_data, &Itemp_tmp);
      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        zeta[9 * (nb_data[rtb_phidot_tmp] - 1) + 7] = 0.63661977236758138 * tmpDrphiVar_data[rtb_phidot_tmp];
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      n_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        ob_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        ob_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        ob_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        ob_data[iy] = 4;
      }

      for (rtb_phidot_tmp = 0; rtb_phidot_tmp < n_size; rtb_phidot_tmp++) {
        s = ob_data[rtb_phidot_tmp];
        zeta[9 * (s - 1) + 8] = Drphi[s - 1] + 1.0;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      tmp_size_1 = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        lam_ykappa[0] = 0.0 * rtb_Saturation[0] * rtb_VectorConcatenate_d[11];
        iy = 1;
      }

      if (isLowSpeed[1]) {
        lam_ykappa[iy] = 0.0 * rtb_Saturation[1] * rtb_VectorConcatenate_d[38];
        iy++;
      }

      if (isLowSpeed[2]) {
        lam_ykappa[iy] = 0.0 * rtb_Saturation[2] * rtb_VectorConcatenate_d[65];
        iy++;
      }

      if (isLowSpeed[3]) {
        lam_ykappa[iy] = 0.0 * rtb_Saturation[3] * rtb_VectorConcatenate_d[92];
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      rtb_phidot_tmp = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        qb_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        qb_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        qb_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        qb_data[iy] = 4;
      }

      iy = 0;
      if (isLowSpeed[0]) {
        iy = 1;
      }

      if (isLowSpeed[1]) {
        iy++;
      }

      if (isLowSpeed[2]) {
        iy++;
      }

      if (isLowSpeed[3]) {
        iy++;
      }

      n_size = iy;
      iy = 0;
      if (isLowSpeed[0]) {
        rb_data[0] = 1;
        iy = 1;
      }

      if (isLowSpeed[1]) {
        rb_data[iy] = 2;
        iy++;
      }

      if (isLowSpeed[2]) {
        rb_data[iy] = 3;
        iy++;
      }

      if (isLowSpeed[3]) {
        rb_data[iy] = 4;
      }

      if ((tmp_size_1 == rtb_phidot_tmp) && ((tmp_size_1 == 1 ? rtb_phidot_tmp : tmp_size_1) == n_size)) {
        Itemp_tmp = tmp_size_1;
        for (rtb_phidot_tmp = 0; rtb_phidot_tmp < tmp_size_1; rtb_phidot_tmp++) {
          s = rb_data[rtb_phidot_tmp];
          tmpDrphiVar_data[rtb_phidot_tmp] =
              (Vc[qb_data[rtb_phidot_tmp] - 1] + lam_ykappa[rtb_phidot_tmp]) - SVygamma[s - 1] / b_x[s - 1];
        }
      } else {
        Chassis_binary_expand_op_b3(tmpDrphiVar_data, &Itemp_tmp, lam_ykappa, &tmp_size_1, Vc, qb_data, &rtb_phidot_tmp,
                                    SVygamma, rb_data, &n_size, b_x);
      }

      iy = 0;
      if (isLowSpeed[0]) {
        SHy_idx_0 = tmpDrphiVar_data[0];
        iy = 1;
      }

      if (isLowSpeed[1]) {
        SHy_idx_1 = tmpDrphiVar_data[iy];
        iy++;
      }

      if (isLowSpeed[2]) {
        SHy_idx_2 = tmpDrphiVar_data[iy];
        iy++;
      }

      if (isLowSpeed[3]) {
        SHy_idx_3 = tmpDrphiVar_data[iy];
      }
    }

    lam_Cz_tmp = Chassis_P.CombinedSlipWheel2DOF_PCX1 * rtb_VectorConcatenate_d[6];
    if (lam_Cz_tmp < 0.0) {
      lam_Cz_tmp = 0.0;
    }

    gamma_star_tmp = rtb_Add_j[0] * rtb_Add_j[0];
    rtb_phidot = ((Chassis_P.CombinedSlipWheel2DOF_PPX3 * rtb_sig_y[0] + 1.0) +
                  Chassis_P.CombinedSlipWheel2DOF_PPX4 * lam_mux_0) *
                 (Chassis_P.CombinedSlipWheel2DOF_PDX2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_PDX1) *
                 (1.0 - gamma_star_tmp * Chassis_P.CombinedSlipWheel2DOF_PDX3) * rtb_ixk * rtb_xdot[0] * zeta[1];
    if (rtb_phidot < 0.0) {
      rtb_phidot = 0.0;
    }

    Drphi_tmp_0 = rtb_Saturation[0] * rtb_Saturation[0];
    Drphi_0 = ((Chassis_P.CombinedSlipWheel2DOF_PEX2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_PEX1) +
               Drphi_tmp_0 * Chassis_P.CombinedSlipWheel2DOF_PEX3) *
              (1.0 - std::tanh(10.0 * lam_Hx[0]) * Chassis_P.CombinedSlipWheel2DOF_PEX4) * rtb_VectorConcatenate_d[8];
    if (Drphi_0 > 1.0) {
      Drphi_0 = 1.0;
    }

    Kyalpha[0] = lam_Cz_tmp * rtb_phidot;
    lam_Cz_0 = Chassis_P.CombinedSlipWheel2DOF_REX2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_REX1;
    lam_muV[0] = lam_Cz_0;
    if (lam_Cz_0 > 1.0) {
      lam_muV[0] = 1.0;
    }

    Drphi[0] = lam_Cz_tmp;
    Dx[0] = rtb_phidot;
    Vs[0] = Drphi_0;
    rtb_UnaryMinus[0] =
        (Chassis_P.CombinedSlipWheel2DOF_PKX2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_PKX1) *
        rtb_xdot[0] * std::exp(Chassis_P.CombinedSlipWheel2DOF_PKX3 * rtb_Saturation[0]) *
        ((Chassis_P.CombinedSlipWheel2DOF_PPX1 * rtb_sig_y[0] + 1.0) +
         lam_mux_0 * Chassis_P.CombinedSlipWheel2DOF_PPX2) *
        rtb_VectorConcatenate_d[4];
    lam_Cz_tmp = Chassis_P.CombinedSlipWheel2DOF_PCX1 * rtb_VectorConcatenate_d[33];
    if (lam_Cz_tmp < 0.0) {
      lam_Cz_tmp = 0.0;
    }

    lam_mux_0 = rtb_Add_j[1] * rtb_Add_j[1];
    rtb_phidot = ((Chassis_P.CombinedSlipWheel2DOF_PPX3 * rtb_sig_y[1] + 1.0) +
                  Chassis_P.CombinedSlipWheel2DOF_PPX4 * rtb_Gain2_e) *
                 (Chassis_P.CombinedSlipWheel2DOF_PDX2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_PDX1) *
                 (1.0 - lam_mux_0 * Chassis_P.CombinedSlipWheel2DOF_PDX3) * lam_mux_tmp * rtb_xdot[1] * zeta[10];
    if (rtb_phidot < 0.0) {
      rtb_phidot = 0.0;
    }

    Drphi_tmp_1 = rtb_Saturation[1] * rtb_Saturation[1];
    Drphi_0 = ((Chassis_P.CombinedSlipWheel2DOF_PEX2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_PEX1) +
               Drphi_tmp_1 * Chassis_P.CombinedSlipWheel2DOF_PEX3) *
              (1.0 - std::tanh(10.0 * lam_Hx[1]) * Chassis_P.CombinedSlipWheel2DOF_PEX4) * rtb_VectorConcatenate_d[35];
    if (Drphi_0 > 1.0) {
      Drphi_0 = 1.0;
    }

    Kyalpha[1] = lam_Cz_tmp * rtb_phidot;
    lam_Cz_0 = Chassis_P.CombinedSlipWheel2DOF_REX2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_REX1;
    lam_muV[1] = lam_Cz_0;
    if (lam_Cz_0 > 1.0) {
      lam_muV[1] = 1.0;
    }

    Drphi[1] = lam_Cz_tmp;
    Dx[1] = rtb_phidot;
    Vs[1] = Drphi_0;
    rtb_UnaryMinus[1] =
        (Chassis_P.CombinedSlipWheel2DOF_PKX2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_PKX1) *
        rtb_xdot[1] * std::exp(Chassis_P.CombinedSlipWheel2DOF_PKX3 * rtb_Saturation[1]) *
        ((Chassis_P.CombinedSlipWheel2DOF_PPX1 * rtb_sig_y[1] + 1.0) +
         rtb_Gain2_e * Chassis_P.CombinedSlipWheel2DOF_PPX2) *
        rtb_VectorConcatenate_d[31];
    lam_Cz_tmp = Chassis_P.CombinedSlipWheel2DOF_PCX1 * rtb_VectorConcatenate_d[60];
    if (lam_Cz_tmp < 0.0) {
      lam_Cz_tmp = 0.0;
    }

    rtb_Gain2_e = rtb_Add_j[2] * rtb_Add_j[2];
    rtb_phidot = ((Chassis_P.CombinedSlipWheel2DOF_PPX3 * rtb_sig_y[2] + 1.0) +
                  Chassis_P.CombinedSlipWheel2DOF_PPX4 * rtb_Product1_h) *
                 (Chassis_P.CombinedSlipWheel2DOF_PDX2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_PDX1) *
                 (1.0 - rtb_Gain2_e * Chassis_P.CombinedSlipWheel2DOF_PDX3) * lam_mux_tmp_0 * rtb_xdot[2] * zeta[19];
    if (rtb_phidot < 0.0) {
      rtb_phidot = 0.0;
    }

    Drphi_tmp_2 = rtb_Saturation[2] * rtb_Saturation[2];
    Drphi_0 = ((Chassis_P.CombinedSlipWheel2DOF_PEX2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_PEX1) +
               Drphi_tmp_2 * Chassis_P.CombinedSlipWheel2DOF_PEX3) *
              (1.0 - std::tanh(10.0 * lam_Hx[2]) * Chassis_P.CombinedSlipWheel2DOF_PEX4) * rtb_VectorConcatenate_d[62];
    if (Drphi_0 > 1.0) {
      Drphi_0 = 1.0;
    }

    Kyalpha[2] = lam_Cz_tmp * rtb_phidot;
    lam_Cz_0 = Chassis_P.CombinedSlipWheel2DOF_REX2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_REX1;
    lam_muV[2] = lam_Cz_0;
    if (lam_Cz_0 > 1.0) {
      lam_muV[2] = 1.0;
    }

    Drphi[2] = lam_Cz_tmp;
    Dx[2] = rtb_phidot;
    Vs[2] = Drphi_0;
    rtb_UnaryMinus[2] =
        (Chassis_P.CombinedSlipWheel2DOF_PKX2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_PKX1) *
        rtb_xdot[2] * std::exp(Chassis_P.CombinedSlipWheel2DOF_PKX3 * rtb_Saturation[2]) *
        ((Chassis_P.CombinedSlipWheel2DOF_PPX1 * rtb_sig_y[2] + 1.0) +
         rtb_Product1_h * Chassis_P.CombinedSlipWheel2DOF_PPX2) *
        rtb_VectorConcatenate_d[58];
    lam_Cz_tmp = Chassis_P.CombinedSlipWheel2DOF_PCX1 * rtb_VectorConcatenate_d[87];
    if (lam_Cz_tmp < 0.0) {
      lam_Cz_tmp = 0.0;
    }

    rtb_Product1_h = rtb_Add_n * rtb_Add_n;
    rtb_phidot =
        ((Chassis_P.CombinedSlipWheel2DOF_PPX3 * rtb_sig_y[3] + 1.0) + Chassis_P.CombinedSlipWheel2DOF_PPX4 * Dx_tmp) *
        (Chassis_P.CombinedSlipWheel2DOF_PDX2 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_PDX1) *
        (1.0 - rtb_Product1_h * Chassis_P.CombinedSlipWheel2DOF_PDX3) * lam_mux_tmp_1 * rtb_xdot_o * zeta[28];
    if (rtb_phidot < 0.0) {
      rtb_phidot = 0.0;
    }

    Drphi_tmp_3 = rtb_Saturation[3] * rtb_Saturation[3];
    Drphi_0 = ((Chassis_P.CombinedSlipWheel2DOF_PEX2 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_PEX1) +
               Drphi_tmp_3 * Chassis_P.CombinedSlipWheel2DOF_PEX3) *
              (1.0 - std::tanh(10.0 * lam_Hx[3]) * Chassis_P.CombinedSlipWheel2DOF_PEX4) * rtb_VectorConcatenate_d[89];
    if (Drphi_0 > 1.0) {
      Drphi_0 = 1.0;
    }

    Dx_tmp =
        (Chassis_P.CombinedSlipWheel2DOF_PKX2 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_PKX1) * rtb_xdot_o *
        std::exp(Chassis_P.CombinedSlipWheel2DOF_PKX3 * rtb_Saturation[3]) *
        ((Chassis_P.CombinedSlipWheel2DOF_PPX1 * rtb_sig_y[3] + 1.0) + Dx_tmp * Chassis_P.CombinedSlipWheel2DOF_PPX2) *
        rtb_VectorConcatenate_d[85];
    Kyalpha[3] = lam_Cz_tmp * rtb_phidot;
    lam_Cz_0 = Chassis_P.CombinedSlipWheel2DOF_REX2 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_REX1;
    lam_muV[3] = lam_Cz_0;
    if (lam_Cz_0 > 1.0) {
      lam_muV[3] = 1.0;
    }

    Drphi[3] = lam_Cz_tmp;
    Dx[3] = rtb_phidot;
    Vs[3] = Drphi_0;
    Chassis_div0protect(Kyalpha, 0.0001, Vc, lam_Cz);
    mu_y_idx_0 = (mu_y_idx_0 * Chassis_P.CombinedSlipWheel2DOF_RBX3 + Chassis_P.CombinedSlipWheel2DOF_RBX1) *
                 std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RBX2 * rtb_Kappa[0])) * rtb_VectorConcatenate_d[18];
    if (mu_y_idx_0 < 0.0) {
      mu_y_idx_0 = 0.0;
    }

    lam_ykappa[0] = mu_y_idx_0;
    mu_y_idx_0 = (rtb_a_o * Chassis_P.CombinedSlipWheel2DOF_RBX3 + Chassis_P.CombinedSlipWheel2DOF_RBX1) *
                 std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RBX2 * rtb_Kappa[1])) * rtb_VectorConcatenate_d[45];
    if (mu_y_idx_0 < 0.0) {
      mu_y_idx_0 = 0.0;
    }

    lam_ykappa[1] = mu_y_idx_0;
    mu_y_idx_0 = (mu_y_tmp * Chassis_P.CombinedSlipWheel2DOF_RBX3 + Chassis_P.CombinedSlipWheel2DOF_RBX1) *
                 std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RBX2 * rtb_Kappa[2])) * rtb_VectorConcatenate_d[72];
    if (mu_y_idx_0 < 0.0) {
      mu_y_idx_0 = 0.0;
    }

    lam_ykappa[2] = mu_y_idx_0;
    mu_y_idx_0 = (mu_y_idx_1 * Chassis_P.CombinedSlipWheel2DOF_RBX3 + Chassis_P.CombinedSlipWheel2DOF_RBX1) *
                 std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RBX2 * rtb_Product_ju)) *
                 rtb_VectorConcatenate_d[99];
    if (mu_y_idx_0 < 0.0) {
      mu_y_idx_0 = 0.0;
    }

    lam_ykappa[3] = mu_y_idx_0;
    Chassis_magiccos(Chassis_P.CombinedSlipWheel2DOF_RCX1, lam_ykappa, lam_muV, rtb_Alpha, Kyalpha);
    Chassis_magiccos_n(Chassis_P.CombinedSlipWheel2DOF_RCX1, lam_ykappa, lam_muV, tmp_data);
    lam_Cz_0 = Kyalpha[0] / tmp_data[0];
    if (lam_Cz_0 < 0.0) {
      lam_Cz_0 = 0.0;
    }

    rtb_a[0] = rtb_UnaryMinus[0] / Vc[0];
    lam_Cz[0] = lam_Cz_0;
    lam_Cz_0 = Kyalpha[1] / tmp_data[1];
    if (lam_Cz_0 < 0.0) {
      lam_Cz_0 = 0.0;
    }

    rtb_a[1] = rtb_UnaryMinus[1] / Vc[1];
    lam_Cz[1] = lam_Cz_0;
    lam_Cz_0 = Kyalpha[2] / tmp_data[2];
    if (lam_Cz_0 < 0.0) {
      lam_Cz_0 = 0.0;
    }

    rtb_a[2] = rtb_UnaryMinus[2] / Vc[2];
    lam_Cz[2] = lam_Cz_0;
    lam_Cz_0 = Kyalpha[3] / tmp_data[3];
    if (lam_Cz_0 < 0.0) {
      lam_Cz_0 = 0.0;
    }

    rtb_a[3] = Dx_tmp / Vc[3];
    Chassis_magicsin_g(Dx, Drphi, rtb_a, Vs, lam_Hx, Kyalpha);
    mu_y_tmp = Chassis_P.CombinedSlipWheel2DOF_PCY1 * rtb_VectorConcatenate_d[7];
    if (mu_y_tmp < 0.0) {
      mu_y_tmp = 0.0;
    }

    rtb_kxj *= rtb_xdot[0];
    rtb_a_o = rtb_kxj * zeta[2];
    lam_muV[0] = mu_y_tmp * rtb_a_o;
    lam_mux[0] = ((Chassis_P.CombinedSlipWheel2DOF_PVX2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_PVX1) *
                      rtb_xdot[0] * (rtb_ixk * 10.0 / (9.0 * rtb_ixk + 1.0)) * rtb_VectorConcatenate_d[12] * zeta[1] +
                  Kyalpha[0]) *
                 lam_Cz[0];
    phi_t[0] = mu_y_tmp;
    mu_y_idx_0 = rtb_kxj;
    rtb_a[0] = rtb_a_o;
    mu_y_tmp = Chassis_P.CombinedSlipWheel2DOF_PCY1 * rtb_VectorConcatenate_d[34];
    if (mu_y_tmp < 0.0) {
      mu_y_tmp = 0.0;
    }

    rtb_kxj = rtb_jxi * rtb_xdot[1];
    rtb_a_o = rtb_kxj * zeta[11];
    lam_muV[1] = mu_y_tmp * rtb_a_o;
    lam_mux[1] =
        ((Chassis_P.CombinedSlipWheel2DOF_PVX2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_PVX1) *
             rtb_xdot[1] * (lam_mux_tmp * 10.0 / (9.0 * lam_mux_tmp + 1.0)) * rtb_VectorConcatenate_d[39] * zeta[10] +
         Kyalpha[1]) *
        lam_Cz[1];
    phi_t[1] = mu_y_tmp;
    mu_y_idx_1 = rtb_kxj;
    rtb_a[1] = rtb_a_o;
    mu_y_tmp = Chassis_P.CombinedSlipWheel2DOF_PCY1 * rtb_VectorConcatenate_d[61];
    if (mu_y_tmp < 0.0) {
      mu_y_tmp = 0.0;
    }

    rtb_kxj = rtb_Product2_l * rtb_xdot[2];
    rtb_a_o = rtb_kxj * zeta[20];
    lam_muV[2] = mu_y_tmp * rtb_a_o;
    lam_mux[2] = ((Chassis_P.CombinedSlipWheel2DOF_PVX2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_PVX1) *
                      rtb_xdot[2] * (lam_mux_tmp_0 * 10.0 / (9.0 * lam_mux_tmp_0 + 1.0)) * rtb_VectorConcatenate_d[66] *
                      zeta[19] +
                  Kyalpha[2]) *
                 lam_Cz[2];
    phi_t[2] = mu_y_tmp;
    mu_y_idx_2 = rtb_kxj;
    rtb_a[2] = rtb_a_o;
    rtb_ixk = ((Chassis_P.CombinedSlipWheel2DOF_PVX2 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_PVX1) *
                   rtb_xdot_o * (lam_mux_tmp_1 * 10.0 / (9.0 * lam_mux_tmp_1 + 1.0)) * rtb_VectorConcatenate_d[93] *
                   zeta[28] +
               Kyalpha[3]) *
              lam_Cz_0;
    mu_y_tmp = Chassis_P.CombinedSlipWheel2DOF_PCY1 * rtb_VectorConcatenate_d[88];
    if (mu_y_tmp < 0.0) {
      mu_y_tmp = 0.0;
    }

    rtb_kxj = rtb_sig_x_f * rtb_xdot_o;
    rtb_a_o = rtb_kxj * zeta[29];
    lam_muV[3] = mu_y_tmp * rtb_a_o;
    lam_mux[3] = rtb_ixk;
    phi_t[3] = mu_y_tmp;
    rtb_a[3] = rtb_a_o;
    Chassis_div0protect(lam_muV, 0.0001, Mzphiinf, lam_Cz);
    rtb_phidot = rtb_xdot_tmp / Mzphiinf[0];
    isLowSpeed_0[0] = !isLowSpeed[0];
    Mzphiinf[0] = rtb_xdot_tmp / Mzphiinf[0];
    rtb_xdot_o = rtb_xdot_tmp_0 / Mzphiinf[1];
    isLowSpeed_0[1] = !isLowSpeed[1];
    Mzphiinf[1] = rtb_xdot_o;
    lam_mux_tmp_0 = rtb_xdot_tmp_1 / Mzphiinf[2];
    isLowSpeed_0[2] = !isLowSpeed[2];
    Mzphiinf[2] = lam_mux_tmp_0;
    lam_mux_tmp = rtb_xdot_tmp_2 / Mzphiinf[3];
    isLowSpeed_0[3] = !isLowSpeed[3];
    Mzphiinf[3] = lam_mux_tmp;
    if (Chassis_any(isLowSpeed_0)) {
      iy = 0;
      if (!isLowSpeed[0]) {
        iy = 1;
      }

      if (!isLowSpeed[1]) {
        iy++;
      }

      if (!isLowSpeed[2]) {
        iy++;
      }

      if (!isLowSpeed[3]) {
        iy++;
      }

      rtb_phidot_tmp = iy;
      iy = 0;
      if (!isLowSpeed[0]) {
        jb_data[0] = 1;
        iy = 1;
      }

      if (!isLowSpeed[1]) {
        jb_data[iy] = 2;
        iy++;
      }

      if (!isLowSpeed[2]) {
        jb_data[iy] = 3;
        iy++;
      }

      if (!isLowSpeed[3]) {
        jb_data[iy] = 4;
      }

      iy = 0;
      if (!isLowSpeed[0]) {
        iy = 1;
      }

      if (!isLowSpeed[1]) {
        iy++;
      }

      if (!isLowSpeed[2]) {
        iy++;
      }

      if (!isLowSpeed[3]) {
        iy++;
      }

      n_size = iy;
      iy = 0;
      if (!isLowSpeed[0]) {
        lb_data[0] = 1;
        iy = 1;
      }

      if (!isLowSpeed[1]) {
        lb_data[iy] = 2;
        iy++;
      }

      if (!isLowSpeed[2]) {
        lb_data[iy] = 3;
        iy++;
      }

      if (!isLowSpeed[3]) {
        lb_data[iy] = 4;
      }

      iy = 0;
      if (!isLowSpeed[0]) {
        iy = 1;
      }

      if (!isLowSpeed[1]) {
        iy++;
      }

      if (!isLowSpeed[2]) {
        iy++;
      }

      if (!isLowSpeed[3]) {
        iy++;
      }

      tmp_size_1 = iy;
      iy = 0;
      if (!isLowSpeed[0]) {
        k_data[0] = 0.0 * rtb_Saturation[0] * rtb_VectorConcatenate_d[11];
        iy = 1;
      }

      if (!isLowSpeed[1]) {
        k_data[iy] = 0.0 * rtb_Saturation[1] * rtb_VectorConcatenate_d[38];
        iy++;
      }

      if (!isLowSpeed[2]) {
        k_data[iy] = 0.0 * rtb_Saturation[2] * rtb_VectorConcatenate_d[65];
        iy++;
      }

      if (!isLowSpeed[3]) {
        k_data[iy] = 0.0 * rtb_Saturation[3] * rtb_VectorConcatenate_d[92];
      }

      iy = 0;
      if (!isLowSpeed[0]) {
        iy = 1;
      }

      if (!isLowSpeed[1]) {
        iy++;
      }

      if (!isLowSpeed[2]) {
        iy++;
      }

      if (!isLowSpeed[3]) {
        iy++;
      }

      o_size = iy;
      iy = 0;
      if (!isLowSpeed[0]) {
        mb_data[0] = 1;
        iy = 1;
      }

      if (!isLowSpeed[1]) {
        mb_data[iy] = 2;
        iy++;
      }

      if (!isLowSpeed[2]) {
        mb_data[iy] = 3;
        iy++;
      }

      if (!isLowSpeed[3]) {
        mb_data[iy] = 4;
      }

      iy = 0;
      if (!isLowSpeed[0]) {
        iy = 1;
      }

      if (!isLowSpeed[1]) {
        iy++;
      }

      if (!isLowSpeed[2]) {
        iy++;
      }

      if (!isLowSpeed[3]) {
        iy++;
      }

      w_size = iy;
      iy = 0;
      if (!isLowSpeed[0]) {
        pb_data[0] = 1;
        iy = 1;
      }

      if (!isLowSpeed[1]) {
        pb_data[iy] = 2;
        iy++;
      }

      if (!isLowSpeed[2]) {
        pb_data[iy] = 3;
        iy++;
      }

      if (!isLowSpeed[3]) {
        pb_data[iy] = 4;
      }

      if ((rtb_phidot_tmp == n_size) && ((rtb_phidot_tmp == 1 ? n_size : rtb_phidot_tmp) == n_size) &&
          (((rtb_phidot_tmp == 1 ? n_size : rtb_phidot_tmp) == 1 ? n_size
            : rtb_phidot_tmp == 1                                ? n_size
                                                                 : rtb_phidot_tmp) == o_size) &&
          ((((rtb_phidot_tmp == 1 ? n_size : rtb_phidot_tmp) == 1 ? n_size
             : rtb_phidot_tmp == 1                                ? n_size
                                                                  : rtb_phidot_tmp) == 1
                ? o_size
            : (rtb_phidot_tmp == 1 ? n_size : rtb_phidot_tmp) == 1 ? n_size
            : rtb_phidot_tmp == 1                                  ? n_size
                                                                   : rtb_phidot_tmp) == tmp_size_1) &&
          ((tmp_size_1 == 1 ? ((rtb_phidot_tmp == 1 ? n_size : rtb_phidot_tmp) == 1 ? n_size
                               : rtb_phidot_tmp == 1                                ? n_size
                                                                                    : rtb_phidot_tmp) == 1
                                  ? o_size
                              : (rtb_phidot_tmp == 1 ? n_size : rtb_phidot_tmp) == 1 ? n_size
                              : rtb_phidot_tmp == 1                                  ? n_size
                                                                                     : rtb_phidot_tmp
                            : tmp_size_1) == w_size)) {
        for (rtb_phidot_tmp = 0; rtb_phidot_tmp < tmp_size_1; rtb_phidot_tmp++) {
          int8_T jb;
          s = lb_data[rtb_phidot_tmp];
          jb = jb_data[rtb_phidot_tmp];
          tmpDrphiVar_data[rtb_phidot_tmp] =
              (((rtb_Saturation[jb - 1] * Chassis_P.CombinedSlipWheel2DOF_PKY7 + Chassis_P.CombinedSlipWheel2DOF_PKY6) *
                    rtb_xdot[jb - 1] * (rtb_sig_y[jb - 1] * Chassis_P.CombinedSlipWheel2DOF_PPY5 + 1.0) *
                    rtb_VectorConcatenate_d[(jb - 1) * 27 + 14] * gamma_star[jb - 1] -
                SVygamma[s - 1]) /
                   b_x[s - 1] * zeta[(mb_data[rtb_phidot_tmp] - 1) * 9] +
               k_data[rtb_phidot_tmp]) +
              zeta[(pb_data[rtb_phidot_tmp] - 1) * 9 + 4];
        }
      } else {
        Chassis_binary_expand_op_b(tmpDrphiVar_data, &Itemp_tmp, k_data, &tmp_size_1, rtb_xdot,
                                   Chassis_P.CombinedSlipWheel2DOF_PKY6, Chassis_P.CombinedSlipWheel2DOF_PKY7,
                                   rtb_Saturation, Chassis_P.CombinedSlipWheel2DOF_PPY5, rtb_sig_y,
                                   rtb_VectorConcatenate_d, jb_data, &rtb_phidot_tmp, gamma_star, SVygamma, lb_data,
                                   &n_size, b_x, zeta, mb_data, &o_size, pb_data, &w_size);
      }

      iy = 0;
      if (!isLowSpeed[0]) {
        SHy_idx_0 = tmpDrphiVar_data[0] - 1.0;
        iy = 1;
      }

      if (!isLowSpeed[1]) {
        SHy_idx_1 = tmpDrphiVar_data[iy] - 1.0;
        iy++;
      }

      if (!isLowSpeed[2]) {
        SHy_idx_2 = tmpDrphiVar_data[iy] - 1.0;
        iy++;
      }

      if (!isLowSpeed[3]) {
        SHy_idx_3 = tmpDrphiVar_data[iy] - 1.0;
      }
    }

    lam_Cz_0 = rtb_Alpha[0] + SHy_idx_0;
    if (std::isnan(lam_Cz_0)) {
      rtb_a_tmp = (rtNaN);
    } else if (lam_Cz_0 < 0.0) {
      rtb_a_tmp = -1.0;
    } else {
      rtb_a_tmp = (lam_Cz_0 > 0.0);
    }

    if (rtb_a_tmp == 0.0) {
      rtb_a_tmp = 1.0;
    }

    lam_Cz_tmp = gamma_star[0] * gamma_star[0];
    Drphi_0 = ((lam_Cz_tmp * Chassis_P.CombinedSlipWheel2DOF_PEY5 + 1.0) -
               Chassis_P.CombinedSlipWheel2DOF_PEY4 * gamma_star[0] * rtb_a_tmp) *
              (Chassis_P.CombinedSlipWheel2DOF_PEY2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_PEY1) *
              rtb_VectorConcatenate_d[9];
    if (Drphi_0 > 1.0) {
      Drphi_0 = 1.0;
    }

    rtb_jxi = (0.0 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_RVY3 * gamma_star[0]) * mu_y_idx_0 *
              std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RVY4 * rtb_Alpha[0])) * zeta[2] *
              std::sin(std::atan(Chassis_P.CombinedSlipWheel2DOF_RVY6 * rtb_Kappa[0]) *
                       Chassis_P.CombinedSlipWheel2DOF_RVY5) *
              rtb_VectorConcatenate_d[20];
    lam_muy_prime_idx_0 =
        0.0 * rtb_Saturation[0] * rtb_xdot[0] * rtb_VectorConcatenate_d[13] * lam_muy_prime_idx_0 * zeta[2] +
        SVygamma[0];
    lam_Cz[0] = lam_Cz_0;
    Drphi[0] = Drphi_0;
    lam_Cz_0 = rtb_Alpha[1] + SHy_idx_1;
    if (std::isnan(lam_Cz_0)) {
      rtb_a_tmp = (rtNaN);
    } else if (lam_Cz_0 < 0.0) {
      rtb_a_tmp = -1.0;
    } else {
      rtb_a_tmp = (lam_Cz_0 > 0.0);
    }

    if (rtb_a_tmp == 0.0) {
      rtb_a_tmp = 1.0;
    }

    b_idx_0 = gamma_star[1] * gamma_star[1];
    Drphi_0 = ((b_idx_0 * Chassis_P.CombinedSlipWheel2DOF_PEY5 + 1.0) -
               Chassis_P.CombinedSlipWheel2DOF_PEY4 * gamma_star[1] * rtb_a_tmp) *
              (Chassis_P.CombinedSlipWheel2DOF_PEY2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_PEY1) *
              rtb_VectorConcatenate_d[36];
    if (Drphi_0 > 1.0) {
      Drphi_0 = 1.0;
    }

    rtb_Product2_l = (0.0 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_RVY3 * gamma_star[1]) * mu_y_idx_1 *
                     std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RVY4 * rtb_Alpha[1])) * zeta[11] *
                     std::sin(std::atan(Chassis_P.CombinedSlipWheel2DOF_RVY6 * rtb_Kappa[1]) *
                              Chassis_P.CombinedSlipWheel2DOF_RVY5) *
                     rtb_VectorConcatenate_d[47];
    lam_muy_prime_idx_1 =
        0.0 * rtb_Saturation[1] * rtb_xdot[1] * rtb_VectorConcatenate_d[40] * lam_muy_prime_idx_1 * zeta[11] +
        SVygamma[1];
    lam_Cz[1] = lam_Cz_0;
    Drphi[1] = Drphi_0;
    lam_Cz_0 = rtb_Alpha[2] + SHy_idx_2;
    if (std::isnan(lam_Cz_0)) {
      rtb_a_tmp = (rtNaN);
    } else if (lam_Cz_0 < 0.0) {
      rtb_a_tmp = -1.0;
    } else {
      rtb_a_tmp = (lam_Cz_0 > 0.0);
    }

    if (rtb_a_tmp == 0.0) {
      rtb_a_tmp = 1.0;
    }

    Drphi_tmp = gamma_star[2] * gamma_star[2];
    Drphi_0 = ((Drphi_tmp * Chassis_P.CombinedSlipWheel2DOF_PEY5 + 1.0) -
               Chassis_P.CombinedSlipWheel2DOF_PEY4 * gamma_star[2] * rtb_a_tmp) *
              (Chassis_P.CombinedSlipWheel2DOF_PEY2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_PEY1) *
              rtb_VectorConcatenate_d[63];
    if (Drphi_0 > 1.0) {
      Drphi_0 = 1.0;
    }

    rtb_sig_x_f = (0.0 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_RVY3 * gamma_star[2]) * mu_y_idx_2 *
                  std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RVY4 * rtb_Alpha[2])) * zeta[20] *
                  std::sin(std::atan(Chassis_P.CombinedSlipWheel2DOF_RVY6 * rtb_Kappa[2]) *
                           Chassis_P.CombinedSlipWheel2DOF_RVY5) *
                  rtb_VectorConcatenate_d[74];
    lam_muy_prime_idx_2 =
        0.0 * rtb_Saturation[2] * rtb_xdot[2] * rtb_VectorConcatenate_d[67] * lam_muy_prime_idx_2 * zeta[20] +
        SVygamma[2];
    lam_Cz[2] = lam_Cz_0;
    Drphi[2] = Drphi_0;
    lam_muy_prime =
        0.0 * rtb_Saturation[3] * rtb_xdot[3] * rtb_VectorConcatenate_d[94] * lam_muy_prime_idx_3 * zeta[29] +
        SVygamma[3];
    lam_Cz_0 = rtb_Alpha_b + SHy_idx_3;
    if (std::isnan(lam_Cz_0)) {
      rtb_a_tmp = (rtNaN);
    } else if (lam_Cz_0 < 0.0) {
      rtb_a_tmp = -1.0;
    } else {
      rtb_a_tmp = (lam_Cz_0 > 0.0);
    }

    if (rtb_a_tmp == 0.0) {
      rtb_a_tmp = 1.0;
    }

    rtb_a_o = gamma_star[3] * gamma_star[3];
    Drphi_0 = ((rtb_a_o * Chassis_P.CombinedSlipWheel2DOF_PEY5 + 1.0) -
               Chassis_P.CombinedSlipWheel2DOF_PEY4 * gamma_star[3] * rtb_a_tmp) *
              (Chassis_P.CombinedSlipWheel2DOF_PEY2 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_PEY1) *
              rtb_VectorConcatenate_d[90];
    if (Drphi_0 > 1.0) {
      Drphi_0 = 1.0;
    }

    rtb_kxj = (0.0 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_RVY3 * gamma_star[3]) * rtb_kxj *
              std::cos(std::atan(Chassis_P.CombinedSlipWheel2DOF_RVY4 * rtb_Alpha_b)) * zeta[29] *
              std::sin(std::atan(Chassis_P.CombinedSlipWheel2DOF_RVY6 * rtb_Product_ju) *
                       Chassis_P.CombinedSlipWheel2DOF_RVY5) *
              rtb_VectorConcatenate_d[101];
    lam_Cz[3] = lam_Cz_0;
    Drphi[3] = Drphi_0;
    Chassis_magicsin_g(rtb_a, phi_t, Mzphiinf, Drphi, lam_Cz, Kyalpha);
    lam_mux_tmp_1 = (Kyalpha[0] + lam_muy_prime_idx_0) * Gykappa[0] + rtb_jxi;
    rtb_a[0] = rtb_VectorConcatenate_d[25];
    Gykappa[0] = lam_mux_tmp_1;
    mu_y_idx_1 = (Kyalpha[1] + lam_muy_prime_idx_1) * Gykappa[1] + rtb_Product2_l;
    rtb_a[1] = rtb_VectorConcatenate_d[52];
    Gykappa[1] = mu_y_idx_1;
    mu_y_idx_2 = (Kyalpha[2] + lam_muy_prime_idx_2) * Gykappa[2] + rtb_sig_x_f;
    rtb_a[2] = rtb_VectorConcatenate_d[79];
    Gykappa[2] = mu_y_idx_2;
    lam_Cz_0 = (Kyalpha[3] + lam_muy_prime) * Gykappa[3] + rtb_kxj;
    rtb_a[3] = rtb_VectorConcatenate_d[106];
    Gykappa[3] = lam_Cz_0;
    Chassis_rollingMoment(lam_mux, rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0, FzUnSat, rtb_Mx, rtb_Add_j,
                          Chassis_P.CombinedSlipWheel2DOF_LONGVL, Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS,
                          Chassis_P.CombinedSlipWheel2DOF_FNOMIN, Chassis_P.CombinedSlipWheel2DOF_NOMPRES,
                          Chassis_P.CombinedSlipWheel2DOF_QSY1, Chassis_P.CombinedSlipWheel2DOF_QSY2,
                          Chassis_P.CombinedSlipWheel2DOF_QSY3, Chassis_P.CombinedSlipWheel2DOF_QSY4,
                          Chassis_P.CombinedSlipWheel2DOF_QSY5, Chassis_P.CombinedSlipWheel2DOF_QSY6,
                          Chassis_P.CombinedSlipWheel2DOF_QSY7, Chassis_P.CombinedSlipWheel2DOF_QSY8, rtb_a, Kyalpha);
    lam_muy_prime_idx_3 = Chassis_P.CombinedSlipWheel2DOF_QSX6 * rtb_xdot[0] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    mu_y_idx_0 = Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_xdot[0];
    rtb_a_tmp = std::abs(rtb_Add_j[0]);
    rtb_Mx[0] =
        (((((0.0 * rtb_VectorConcatenate_d[24] - (Chassis_P.CombinedSlipWheel2DOF_PPMX1 * rtb_sig_y[0] + 1.0) *
                                                     (Chassis_P.CombinedSlipWheel2DOF_QSX2 * rtb_Add_j[0])) -
            Chassis_P.CombinedSlipWheel2DOF_QSX12 * rtb_Add_j[0] * rtb_a_tmp) +
           Chassis_P.CombinedSlipWheel2DOF_QSX3 * lam_mux_tmp_1 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) +
          std::sin(
              std::atan(Chassis_P.CombinedSlipWheel2DOF_QSX9 * lam_mux_tmp_1 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
                  Chassis_P.CombinedSlipWheel2DOF_QSX8 +
              Chassis_P.CombinedSlipWheel2DOF_QSX7 * rtb_Add_j[0]) *
              (std::cos(std::atan(lam_muy_prime_idx_3 * lam_muy_prime_idx_3) * Chassis_P.CombinedSlipWheel2DOF_QSX5) *
               Chassis_P.CombinedSlipWheel2DOF_QSX4)) +
         std::atan(Chassis_P.CombinedSlipWheel2DOF_QSX11 * rtb_xdot[0] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
             Chassis_P.CombinedSlipWheel2DOF_QSX10 * rtb_Add_j[0]) *
            (mu_y_idx_0 * rtb_VectorConcatenate_d[23]) +
        Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * lam_mux_tmp_1 * rtb_VectorConcatenate_d[23] *
            (Chassis_P.CombinedSlipWheel2DOF_QSX14 * rtb_a_tmp + Chassis_P.CombinedSlipWheel2DOF_QSX13);
    Vs[0] = ((Chassis_P.CombinedSlipWheel2DOF_QHZ4 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_QHZ3) *
                 gamma_star[0] +
             0.0 * rtb_Saturation[0]) +
            rtb_Alpha[0];
    Drphi_0 = 0.0 * rtb_Add_j[0] + 1.0;
    lam_muy_prime_idx_3 =
        ((Chassis_P.CombinedSlipWheel2DOF_QBZ2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_QBZ1) +
         Drphi_tmp_0 * Chassis_P.CombinedSlipWheel2DOF_QBZ3) *
        ((Chassis_P.CombinedSlipWheel2DOF_QBZ5 * rtb_a_tmp + Drphi_0) +
         lam_Cz_tmp * Chassis_P.CombinedSlipWheel2DOF_QBZ6) *
        rtb_VectorConcatenate_d[5] / rtb_sig_x[0];
    if (lam_muy_prime_idx_3 < 0.0) {
      lam_muy_prime_idx_3 = 0.0;
    }

    rtb_My[0] = std::tanh(10.0 * Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[0]) * Kyalpha[0];
    lam_ykappa[0] = mu_y_idx_0;
    cosprimealpha[0] = rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[0] / cosprimealpha[0];
    SHy_idx_0 = (lam_muy_prime_idx_0 / b_x[0] + SHy_idx_0) + rtb_Alpha[0];
    lam_muV[0] = lam_muy_prime_idx_3;
    Drphi[0] = Drphi_0;
    lam_muy_prime_idx_3 = Chassis_P.CombinedSlipWheel2DOF_QSX6 * rtb_xdot[1] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    mu_y_idx_0 = Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_xdot[1];
    rtb_a_tmp = std::abs(rtb_Add_j[1]);
    rtb_Mx[1] =
        (((((0.0 * rtb_VectorConcatenate_d[51] - (Chassis_P.CombinedSlipWheel2DOF_PPMX1 * rtb_sig_y[1] + 1.0) *
                                                     (Chassis_P.CombinedSlipWheel2DOF_QSX2 * rtb_Add_j[1])) -
            Chassis_P.CombinedSlipWheel2DOF_QSX12 * rtb_Add_j[1] * rtb_a_tmp) +
           Chassis_P.CombinedSlipWheel2DOF_QSX3 * mu_y_idx_1 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) +
          std::sin(
              std::atan(Chassis_P.CombinedSlipWheel2DOF_QSX9 * mu_y_idx_1 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
                  Chassis_P.CombinedSlipWheel2DOF_QSX8 +
              Chassis_P.CombinedSlipWheel2DOF_QSX7 * rtb_Add_j[1]) *
              (std::cos(std::atan(lam_muy_prime_idx_3 * lam_muy_prime_idx_3) * Chassis_P.CombinedSlipWheel2DOF_QSX5) *
               Chassis_P.CombinedSlipWheel2DOF_QSX4)) +
         std::atan(Chassis_P.CombinedSlipWheel2DOF_QSX11 * rtb_xdot[1] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
             Chassis_P.CombinedSlipWheel2DOF_QSX10 * rtb_Add_j[1]) *
            (mu_y_idx_0 * rtb_VectorConcatenate_d[50]) +
        Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * mu_y_idx_1 * rtb_VectorConcatenate_d[50] *
            (Chassis_P.CombinedSlipWheel2DOF_QSX14 * rtb_a_tmp + Chassis_P.CombinedSlipWheel2DOF_QSX13);
    Vs[1] = ((Chassis_P.CombinedSlipWheel2DOF_QHZ4 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_QHZ3) *
                 gamma_star[1] +
             0.0 * rtb_Saturation[1]) +
            rtb_Alpha[1];
    Drphi_0 = 0.0 * rtb_Add_j[1] + 1.0;
    lam_muy_prime_idx_3 =
        ((Chassis_P.CombinedSlipWheel2DOF_QBZ2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_QBZ1) +
         Drphi_tmp_1 * Chassis_P.CombinedSlipWheel2DOF_QBZ3) *
        ((Chassis_P.CombinedSlipWheel2DOF_QBZ5 * rtb_a_tmp + Drphi_0) +
         b_idx_0 * Chassis_P.CombinedSlipWheel2DOF_QBZ6) *
        rtb_VectorConcatenate_d[32] / rtb_sig_x[1];
    if (lam_muy_prime_idx_3 < 0.0) {
      lam_muy_prime_idx_3 = 0.0;
    }

    rtb_My[1] = std::tanh(10.0 * Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[1]) * Kyalpha[1];
    lam_ykappa[1] = mu_y_idx_0;
    cosprimealpha[1] = rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[1] / cosprimealpha[1];
    SHy_idx_1 = (lam_muy_prime_idx_1 / b_x[1] + SHy_idx_1) + rtb_Alpha[1];
    lam_muV[1] = lam_muy_prime_idx_3;
    Drphi[1] = Drphi_0;
    lam_muy_prime_idx_3 = Chassis_P.CombinedSlipWheel2DOF_QSX6 * rtb_xdot[2] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    mu_y_idx_0 = Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_xdot[2];
    rtb_a_tmp = std::abs(rtb_Add_j[2]);
    rtb_Mx[2] =
        (((((0.0 * rtb_VectorConcatenate_d[78] - (Chassis_P.CombinedSlipWheel2DOF_PPMX1 * rtb_sig_y[2] + 1.0) *
                                                     (Chassis_P.CombinedSlipWheel2DOF_QSX2 * rtb_Add_j[2])) -
            Chassis_P.CombinedSlipWheel2DOF_QSX12 * rtb_Add_j[2] * rtb_a_tmp) +
           Chassis_P.CombinedSlipWheel2DOF_QSX3 * mu_y_idx_2 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) +
          std::sin(
              std::atan(Chassis_P.CombinedSlipWheel2DOF_QSX9 * mu_y_idx_2 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
                  Chassis_P.CombinedSlipWheel2DOF_QSX8 +
              Chassis_P.CombinedSlipWheel2DOF_QSX7 * rtb_Add_j[2]) *
              (std::cos(std::atan(lam_muy_prime_idx_3 * lam_muy_prime_idx_3) * Chassis_P.CombinedSlipWheel2DOF_QSX5) *
               Chassis_P.CombinedSlipWheel2DOF_QSX4)) +
         std::atan(Chassis_P.CombinedSlipWheel2DOF_QSX11 * rtb_xdot[2] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
             Chassis_P.CombinedSlipWheel2DOF_QSX10 * rtb_Add_j[2]) *
            (mu_y_idx_0 * rtb_VectorConcatenate_d[77]) +
        Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * mu_y_idx_2 * rtb_VectorConcatenate_d[77] *
            (Chassis_P.CombinedSlipWheel2DOF_QSX14 * rtb_a_tmp + Chassis_P.CombinedSlipWheel2DOF_QSX13);
    Vs[2] = ((Chassis_P.CombinedSlipWheel2DOF_QHZ4 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_QHZ3) *
                 gamma_star[2] +
             0.0 * rtb_Saturation[2]) +
            rtb_Alpha[2];
    Drphi_0 = 0.0 * rtb_Add_j[2] + 1.0;
    lam_muy_prime_idx_3 =
        ((Chassis_P.CombinedSlipWheel2DOF_QBZ2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_QBZ1) +
         Drphi_tmp_2 * Chassis_P.CombinedSlipWheel2DOF_QBZ3) *
        ((Chassis_P.CombinedSlipWheel2DOF_QBZ5 * rtb_a_tmp + Drphi_0) +
         Drphi_tmp * Chassis_P.CombinedSlipWheel2DOF_QBZ6) *
        rtb_VectorConcatenate_d[59] / rtb_sig_x[2];
    if (lam_muy_prime_idx_3 < 0.0) {
      lam_muy_prime_idx_3 = 0.0;
    }

    rtb_My[2] = std::tanh(10.0 * Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[2]) * Kyalpha[2];
    lam_ykappa[2] = mu_y_idx_0;
    cosprimealpha[2] = rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[2] / cosprimealpha[2];
    SHy_idx_2 = (lam_muy_prime_idx_2 / b_x[2] + SHy_idx_2) + rtb_Alpha[2];
    lam_muV[2] = lam_muy_prime_idx_3;
    Drphi[2] = Drphi_0;
    lam_muy_prime_idx_3 = Chassis_P.CombinedSlipWheel2DOF_QSX6 * rtb_xdot[3] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN;
    mu_y_idx_0 = Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_xdot[3];
    rtb_a_tmp = std::abs(rtb_Add_n);
    rtb_Mx[3] =
        (((((0.0 * rtb_VectorConcatenate_d[105] - (Chassis_P.CombinedSlipWheel2DOF_PPMX1 * rtb_sig_y[3] + 1.0) *
                                                      (Chassis_P.CombinedSlipWheel2DOF_QSX2 * rtb_Add_n)) -
            Chassis_P.CombinedSlipWheel2DOF_QSX12 * rtb_Add_n * rtb_a_tmp) +
           Chassis_P.CombinedSlipWheel2DOF_QSX3 * lam_Cz_0 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) +
          std::sin(std::atan(Chassis_P.CombinedSlipWheel2DOF_QSX9 * lam_Cz_0 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
                       Chassis_P.CombinedSlipWheel2DOF_QSX8 +
                   Chassis_P.CombinedSlipWheel2DOF_QSX7 * rtb_Add_n) *
              (std::cos(std::atan(lam_muy_prime_idx_3 * lam_muy_prime_idx_3) * Chassis_P.CombinedSlipWheel2DOF_QSX5) *
               Chassis_P.CombinedSlipWheel2DOF_QSX4)) +
         std::atan(Chassis_P.CombinedSlipWheel2DOF_QSX11 * rtb_xdot[3] / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
             Chassis_P.CombinedSlipWheel2DOF_QSX10 * rtb_Add_n) *
            (mu_y_idx_0 * rtb_VectorConcatenate_d[104]) +
        Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * lam_Cz_0 * rtb_VectorConcatenate_d[104] *
            (Chassis_P.CombinedSlipWheel2DOF_QSX14 * rtb_a_tmp + Chassis_P.CombinedSlipWheel2DOF_QSX13);
    lam_muy_prime_idx_2 = rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[3] / cosprimealpha[3];
    Vs[3] = ((Chassis_P.CombinedSlipWheel2DOF_QHZ4 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_QHZ3) *
                 gamma_star[3] +
             0.0 * rtb_Saturation[3]) +
            rtb_Alpha_b;
    rtb_Alpha_b += lam_muy_prime / b_x[3] + SHy_idx_3;
    Drphi_0 = 0.0 * rtb_Add_n + 1.0;
    lam_muy_prime_idx_3 =
        ((Chassis_P.CombinedSlipWheel2DOF_QBZ2 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_QBZ1) +
         Drphi_tmp_3 * Chassis_P.CombinedSlipWheel2DOF_QBZ3) *
        ((Chassis_P.CombinedSlipWheel2DOF_QBZ5 * rtb_a_tmp + Drphi_0) +
         rtb_a_o * Chassis_P.CombinedSlipWheel2DOF_QBZ6) *
        rtb_VectorConcatenate_d[86] / rtb_psidot;
    if (lam_muy_prime_idx_3 < 0.0) {
      lam_muy_prime_idx_3 = 0.0;
    }

    rtb_My[3] = std::tanh(10.0 * Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[3]) * Kyalpha[3];
    lam_muV[3] = lam_muy_prime_idx_3;
    b_idx_0 = Chassis_P.CombinedSlipWheel2DOF_QCZ1;
    if (Chassis_P.CombinedSlipWheel2DOF_QCZ1 < 0.0) {
      b_idx_0 = 0.0;
    }

    rtb_a_o = (Chassis_P.CombinedSlipWheel2DOF_QEZ5 * gamma_star[0] * 2.0 / 3.1415926535897931 *
                   std::atan(lam_muV[0] * b_idx_0 * Vs[0]) +
               1.0) *
              ((Chassis_P.CombinedSlipWheel2DOF_QEZ2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_QEZ1) +
               Drphi_tmp_0 * Chassis_P.CombinedSlipWheel2DOF_QEZ3);
    if (rtb_a_o > 1.0) {
      rtb_a_o = 1.0;
    }

    rtb_a_tmp = std::tan(SHy_idx_0);
    SHy_idx_3 = rtb_UnaryMinus[0] / b_x[0];
    lam_Cz_tmp = SHy_idx_3 * SHy_idx_3 * (rtb_Kappa[0] * rtb_Kappa[0]);
    SHy_idx_3 = std::sqrt(rtb_a_tmp * rtb_a_tmp + lam_Cz_tmp);
    rtb_a_tmp = std::tan(Vs[0]);
    Kyalpha[0] = (Chassis_P.CombinedSlipWheel2DOF_QDZ2 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_QDZ1) *
                 (1.0 - Chassis_P.CombinedSlipWheel2DOF_PPZ1 * rtb_sig_y[0]) *
                 (gamma_star_tmp * Chassis_P.CombinedSlipWheel2DOF_QDZ4 + Drphi[0]) * rtb_xdot[0] *
                 (Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS / rtb_UnaryMinus_i) * rtb_VectorConcatenate_d[16] *
                 zeta[5];
    FzUnSat[0] = std::atan(std::sqrt(rtb_a_tmp * rtb_a_tmp + lam_Cz_tmp)) * std::tanh(1000.0 * Vs[0]);
    rtb_Add_j[0] =
        ((((Chassis_P.CombinedSlipWheel2DOF_QDZ9 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_QDZ8) *
               (Chassis_P.CombinedSlipWheel2DOF_PPZ2 * rtb_sig_y[0] + 1.0) +
           (Chassis_P.CombinedSlipWheel2DOF_QDZ11 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_QDZ10) *
               std::abs(gamma_star[0])) *
              gamma_star[0] * rtb_VectorConcatenate_d[15] * zeta[0] +
          0.0 * rtb_Saturation[0] * rtb_VectorConcatenate_d[17] * zeta[2]) *
             lam_ykappa[0] * rtb_sig_x[0] * std::tanh(10.0 * rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[0]) *
             cosprimealpha[0] +
         zeta[8]) -
        1.0;
    lam_Cz[0] = zeta[7];
    tmp_data[0] = (Chassis_P.CombinedSlipWheel2DOF_QBZ9 * rtb_VectorConcatenate_d[5] / rtb_sig_x[0] +
                   Chassis_P.CombinedSlipWheel2DOF_QBZ10 * rtb_phidot * phi_t[0]) *
                  zeta[6];
    rtb_b[0] = std::tanh(1000.0 * SHy_idx_0) * std::atan(SHy_idx_3);
    rtb_a[0] = rtb_a_o;
    rtb_a_o = (Chassis_P.CombinedSlipWheel2DOF_QEZ5 * gamma_star[1] * 2.0 / 3.1415926535897931 *
                   std::atan(b_idx_0 * lam_muV[1] * Vs[1]) +
               1.0) *
              ((Chassis_P.CombinedSlipWheel2DOF_QEZ2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_QEZ1) +
               Drphi_tmp_1 * Chassis_P.CombinedSlipWheel2DOF_QEZ3);
    if (rtb_a_o > 1.0) {
      rtb_a_o = 1.0;
    }

    rtb_a_tmp = std::tan(SHy_idx_1);
    SHy_idx_3 = rtb_UnaryMinus[1] / b_x[1];
    lam_Cz_tmp = SHy_idx_3 * SHy_idx_3 * (rtb_Kappa[1] * rtb_Kappa[1]);
    SHy_idx_3 = std::sqrt(rtb_a_tmp * rtb_a_tmp + lam_Cz_tmp);
    rtb_a_tmp = std::tan(Vs[1]);
    Kyalpha[1] = (Chassis_P.CombinedSlipWheel2DOF_QDZ2 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_QDZ1) *
                 (1.0 - Chassis_P.CombinedSlipWheel2DOF_PPZ1 * rtb_sig_y[1]) *
                 (lam_mux_0 * Chassis_P.CombinedSlipWheel2DOF_QDZ4 + Drphi[1]) * rtb_xdot[1] *
                 (Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS / rtb_Add1_b_idx_1) * rtb_VectorConcatenate_d[43] *
                 zeta[14];
    FzUnSat[1] = std::atan(std::sqrt(rtb_a_tmp * rtb_a_tmp + lam_Cz_tmp)) * std::tanh(1000.0 * Vs[1]);
    rtb_Add_j[1] =
        ((((Chassis_P.CombinedSlipWheel2DOF_QDZ9 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_QDZ8) *
               (Chassis_P.CombinedSlipWheel2DOF_PPZ2 * rtb_sig_y[1] + 1.0) +
           (Chassis_P.CombinedSlipWheel2DOF_QDZ11 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_QDZ10) *
               std::abs(gamma_star[1])) *
              gamma_star[1] * rtb_VectorConcatenate_d[42] * zeta[9] +
          0.0 * rtb_Saturation[1] * rtb_VectorConcatenate_d[44] * zeta[11]) *
             lam_ykappa[1] * rtb_sig_x[1] * std::tanh(10.0 * rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[1]) *
             cosprimealpha[1] +
         zeta[17]) -
        1.0;
    lam_Cz[1] = zeta[16];
    tmp_data[1] = (Chassis_P.CombinedSlipWheel2DOF_QBZ9 * rtb_VectorConcatenate_d[32] / rtb_sig_x[1] +
                   Chassis_P.CombinedSlipWheel2DOF_QBZ10 * rtb_xdot_o * phi_t[1]) *
                  zeta[15];
    rtb_b[1] = std::tanh(1000.0 * SHy_idx_1) * std::atan(SHy_idx_3);
    rtb_a[1] = rtb_a_o;
    rtb_a_o = (Chassis_P.CombinedSlipWheel2DOF_QEZ5 * gamma_star[2] * 2.0 / 3.1415926535897931 *
                   std::atan(b_idx_0 * lam_muV[2] * Vs[2]) +
               1.0) *
              ((Chassis_P.CombinedSlipWheel2DOF_QEZ2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_QEZ1) +
               Drphi_tmp_2 * Chassis_P.CombinedSlipWheel2DOF_QEZ3);
    if (rtb_a_o > 1.0) {
      rtb_a_o = 1.0;
    }

    rtb_a_tmp = std::tan(SHy_idx_2);
    SHy_idx_3 = rtb_UnaryMinus[2] / b_x[2];
    lam_Cz_tmp = SHy_idx_3 * SHy_idx_3 * (rtb_Kappa[2] * rtb_Kappa[2]);
    SHy_idx_3 = std::sqrt(rtb_a_tmp * rtb_a_tmp + lam_Cz_tmp);
    rtb_a_tmp = std::tan(Vs[2]);
    Kyalpha[2] = (Chassis_P.CombinedSlipWheel2DOF_QDZ2 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_QDZ1) *
                 (1.0 - Chassis_P.CombinedSlipWheel2DOF_PPZ1 * rtb_sig_y[2]) *
                 (rtb_Gain2_e * Chassis_P.CombinedSlipWheel2DOF_QDZ4 + Drphi[2]) * rtb_xdot[2] *
                 (Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS / Fzo_prime_idx_2) * rtb_VectorConcatenate_d[70] *
                 zeta[23];
    FzUnSat[2] = std::atan(std::sqrt(rtb_a_tmp * rtb_a_tmp + lam_Cz_tmp)) * std::tanh(1000.0 * Vs[2]);
    rtb_Add_j[2] =
        ((((Chassis_P.CombinedSlipWheel2DOF_QDZ9 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_QDZ8) *
               (Chassis_P.CombinedSlipWheel2DOF_PPZ2 * rtb_sig_y[2] + 1.0) +
           (Chassis_P.CombinedSlipWheel2DOF_QDZ11 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_QDZ10) *
               std::abs(gamma_star[2])) *
              gamma_star[2] * rtb_VectorConcatenate_d[69] * zeta[18] +
          0.0 * rtb_Saturation[2] * rtb_VectorConcatenate_d[71] * zeta[20]) *
             lam_ykappa[2] * rtb_sig_x[2] * std::tanh(10.0 * rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[2]) *
             cosprimealpha[2] +
         zeta[26]) -
        1.0;
    lam_Cz[2] = zeta[25];
    tmp_data[2] = (Chassis_P.CombinedSlipWheel2DOF_QBZ9 * rtb_VectorConcatenate_d[59] / rtb_sig_x[2] +
                   Chassis_P.CombinedSlipWheel2DOF_QBZ10 * lam_mux_tmp_0 * phi_t[2]) *
                  zeta[24];
    rtb_b[2] = std::tanh(1000.0 * SHy_idx_2) * std::atan(SHy_idx_3);
    rtb_a[2] = rtb_a_o;
    rtb_a_o = (Chassis_P.CombinedSlipWheel2DOF_QEZ5 * gamma_star[3] * 2.0 / 3.1415926535897931 *
                   std::atan(b_idx_0 * lam_muy_prime_idx_3 * Vs[3]) +
               1.0) *
              ((Chassis_P.CombinedSlipWheel2DOF_QEZ2 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_QEZ1) +
               Drphi_tmp_3 * Chassis_P.CombinedSlipWheel2DOF_QEZ3);
    if (rtb_a_o > 1.0) {
      rtb_a_o = 1.0;
    }

    rtb_a_tmp = std::tan(rtb_Alpha_b);
    SHy_idx_3 = Dx_tmp / b_x[3];
    lam_Cz_tmp = SHy_idx_3 * SHy_idx_3 * (rtb_Product_ju * rtb_Product_ju);
    SHy_idx_3 = std::sqrt(rtb_a_tmp * rtb_a_tmp + lam_Cz_tmp);
    rtb_a_tmp = std::tan(Vs[3]);
    Kyalpha[3] = (Chassis_P.CombinedSlipWheel2DOF_QDZ2 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_QDZ1) *
                 (1.0 - Chassis_P.CombinedSlipWheel2DOF_PPZ1 * rtb_sig_y[3]) *
                 (rtb_Product1_h * Chassis_P.CombinedSlipWheel2DOF_QDZ4 + Drphi_0) * rtb_xdot[3] *
                 (Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS / rtb_thetadot) * rtb_VectorConcatenate_d[97] *
                 zeta[32];
    FzUnSat[3] = std::atan(std::sqrt(rtb_a_tmp * rtb_a_tmp + lam_Cz_tmp)) * std::tanh(1000.0 * Vs[3]);
    rtb_Add_j[3] =
        ((((Chassis_P.CombinedSlipWheel2DOF_QDZ9 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_QDZ8) *
               (Chassis_P.CombinedSlipWheel2DOF_PPZ2 * rtb_sig_y[3] + 1.0) +
           (Chassis_P.CombinedSlipWheel2DOF_QDZ11 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_QDZ10) *
               std::abs(gamma_star[3])) *
              gamma_star[3] * rtb_VectorConcatenate_d[96] * zeta[27] +
          0.0 * rtb_Saturation[3] * rtb_VectorConcatenate_d[98] * zeta[29]) *
             mu_y_idx_0 * rtb_psidot * std::tanh(10.0 * rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[3]) *
             lam_muy_prime_idx_2 +
         zeta[35]) -
        1.0;
    lam_Cz[3] = zeta[34];
    tmp_data[3] = (Chassis_P.CombinedSlipWheel2DOF_QBZ9 * rtb_VectorConcatenate_d[86] / rtb_psidot +
                   Chassis_P.CombinedSlipWheel2DOF_QBZ10 * lam_mux_tmp * mu_y_tmp) *
                  zeta[33];
    rtb_b[3] = std::tanh(1000.0 * rtb_Alpha_b) * std::atan(SHy_idx_3);
    rtb_a[3] = rtb_a_o;
    Chassis_magiccos_ngy(Kyalpha, b_idx_0, lam_muV, rtb_a, FzUnSat, b_x);
    Chassis_magiccos_ng(rtb_Add_j, lam_Cz, tmp_data, rtb_b, Kyalpha);
    rtb_sig_x[0] = std::abs(rtb_UnaryMinus[0] / (((Chassis_P.CombinedSlipWheel2DOF_PCFX1 * rtb_Saturation[0] + 1.0) +
                                                  Drphi_tmp_0 * Chassis_P.CombinedSlipWheel2DOF_PCFX2) *
                                                 Chassis_P.CombinedSlipWheel2DOF_LONGITUDINAL_STIFFNESS *
                                                 (Chassis_P.CombinedSlipWheel2DOF_PCFX3 * rtb_sig_y[0] + 1.0)));
    cosprimealpha[0] =
        ((Chassis_P.CombinedSlipWheel2DOF_SSZ4 * rtb_Saturation[0] + Chassis_P.CombinedSlipWheel2DOF_SSZ3) *
             gamma_star[0] +
         Chassis_P.CombinedSlipWheel2DOF_SSZ2 * lam_mux_tmp_1 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
            Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_VectorConcatenate_d[21] * lam_mux[0] +
        (-(b_x[0] * cosprimealpha[0] * rtb_VectorConcatenate_d[0]) * (lam_mux_tmp_1 - rtb_jxi) + Kyalpha[0]);
    rtb_sig_y[0] = std::abs(rtb_xdot_tmp / (((Chassis_P.CombinedSlipWheel2DOF_PCFY1 * rtb_Saturation[0] + 1.0) +
                                             Drphi_tmp_0 * Chassis_P.CombinedSlipWheel2DOF_PCFY2) *
                                            Chassis_P.CombinedSlipWheel2DOF_LATERAL_STIFFNESS *
                                            (Chassis_P.CombinedSlipWheel2DOF_PCFY3 * rtb_sig_y[0] + 1.0)));
    rtb_sig_x[1] = std::abs(rtb_UnaryMinus[1] / (((Chassis_P.CombinedSlipWheel2DOF_PCFX1 * rtb_Saturation[1] + 1.0) +
                                                  Drphi_tmp_1 * Chassis_P.CombinedSlipWheel2DOF_PCFX2) *
                                                 Chassis_P.CombinedSlipWheel2DOF_LONGITUDINAL_STIFFNESS *
                                                 (Chassis_P.CombinedSlipWheel2DOF_PCFX3 * rtb_sig_y[1] + 1.0)));
    cosprimealpha[1] =
        ((Chassis_P.CombinedSlipWheel2DOF_SSZ4 * rtb_Saturation[1] + Chassis_P.CombinedSlipWheel2DOF_SSZ3) *
             gamma_star[1] +
         Chassis_P.CombinedSlipWheel2DOF_SSZ2 * mu_y_idx_1 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
            Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_VectorConcatenate_d[48] * lam_mux[1] +
        (-(b_x[1] * cosprimealpha[1] * rtb_VectorConcatenate_d[27]) * (mu_y_idx_1 - rtb_Product2_l) + Kyalpha[1]);
    rtb_sig_y[1] = std::abs(rtb_xdot_tmp_0 / (((Chassis_P.CombinedSlipWheel2DOF_PCFY1 * rtb_Saturation[1] + 1.0) +
                                               Drphi_tmp_1 * Chassis_P.CombinedSlipWheel2DOF_PCFY2) *
                                              Chassis_P.CombinedSlipWheel2DOF_LATERAL_STIFFNESS *
                                              (Chassis_P.CombinedSlipWheel2DOF_PCFY3 * rtb_sig_y[1] + 1.0)));
    rtb_sig_x[2] = std::abs(rtb_UnaryMinus[2] / (((Chassis_P.CombinedSlipWheel2DOF_PCFX1 * rtb_Saturation[2] + 1.0) +
                                                  Drphi_tmp_2 * Chassis_P.CombinedSlipWheel2DOF_PCFX2) *
                                                 Chassis_P.CombinedSlipWheel2DOF_LONGITUDINAL_STIFFNESS *
                                                 (Chassis_P.CombinedSlipWheel2DOF_PCFX3 * rtb_sig_y[2] + 1.0)));
    cosprimealpha[2] =
        ((Chassis_P.CombinedSlipWheel2DOF_SSZ4 * rtb_Saturation[2] + Chassis_P.CombinedSlipWheel2DOF_SSZ3) *
             gamma_star[2] +
         Chassis_P.CombinedSlipWheel2DOF_SSZ2 * mu_y_idx_2 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
            Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_VectorConcatenate_d[75] * lam_mux[2] +
        (-(b_x[2] * cosprimealpha[2] * rtb_VectorConcatenate_d[54]) * (mu_y_idx_2 - rtb_sig_x_f) + Kyalpha[2]);
    rtb_sig_y[2] = std::abs(rtb_xdot_tmp_1 / (((Chassis_P.CombinedSlipWheel2DOF_PCFY1 * rtb_Saturation[2] + 1.0) +
                                               Drphi_tmp_2 * Chassis_P.CombinedSlipWheel2DOF_PCFY2) *
                                              Chassis_P.CombinedSlipWheel2DOF_LATERAL_STIFFNESS *
                                              (Chassis_P.CombinedSlipWheel2DOF_PCFY3 * rtb_sig_y[2] + 1.0)));
    rtb_sig_x[3] = std::abs(Dx_tmp / (((Chassis_P.CombinedSlipWheel2DOF_PCFX1 * rtb_Saturation[3] + 1.0) +
                                       Drphi_tmp_3 * Chassis_P.CombinedSlipWheel2DOF_PCFX2) *
                                      Chassis_P.CombinedSlipWheel2DOF_LONGITUDINAL_STIFFNESS *
                                      (Chassis_P.CombinedSlipWheel2DOF_PCFX3 * rtb_sig_y[3] + 1.0)));
    cosprimealpha[3] =
        ((Chassis_P.CombinedSlipWheel2DOF_SSZ4 * rtb_Saturation[3] + Chassis_P.CombinedSlipWheel2DOF_SSZ3) *
             gamma_star[3] +
         Chassis_P.CombinedSlipWheel2DOF_SSZ2 * lam_Cz_0 / Chassis_P.CombinedSlipWheel2DOF_FNOMIN) *
            Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS * rtb_VectorConcatenate_d[102] * rtb_ixk +
        (-(b_x[3] * lam_muy_prime_idx_2 * rtb_VectorConcatenate_d[81]) * (lam_Cz_0 - rtb_kxj) + Kyalpha[3]);
    rtb_sig_y[3] = std::abs(rtb_xdot_tmp_2 / (((Chassis_P.CombinedSlipWheel2DOF_PCFY1 * rtb_Saturation[3] + 1.0) +
                                               Drphi_tmp_3 * Chassis_P.CombinedSlipWheel2DOF_PCFY2) *
                                              Chassis_P.CombinedSlipWheel2DOF_LATERAL_STIFFNESS *
                                              (Chassis_P.CombinedSlipWheel2DOF_PCFY3 * rtb_sig_y[3] + 1.0)));
  } else {
    Kyalpha[0] = Chassis_P.Pressure_const;
    Kyalpha[1] = Chassis_P.Pressure_const;
    Kyalpha[2] = Chassis_P.Pressure_const;
    Kyalpha[3] = Chassis_P.Pressure_const;
    Chassis_vdyncsmtire(Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0, rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0,
                        rtb_ImpAsg_InsertedFor_ydotWheel_at_inport_0, rtb_UnaryMinus, rtb_Add_j, Kyalpha,
                        rtb_VectorConcatenate_d, rtb_Saturation, Chassis_P.CombinedSlipWheel2DOF_turnslip,
                        Chassis_P.CombinedSlipWheel2DOF_PRESMAX, Chassis_P.CombinedSlipWheel2DOF_PRESMIN,
                        Chassis_P.CombinedSlipWheel2DOF_FZMAX, Chassis_P.CombinedSlipWheel2DOF_FZMIN,
                        Chassis_P.CombinedSlipWheel2DOF_VXLOW, Chassis_P.CombinedSlipWheel2DOF_KPUMAX,
                        Chassis_P.CombinedSlipWheel2DOF_KPUMIN, Chassis_P.CombinedSlipWheel2DOF_ALPMAX,
                        Chassis_P.CombinedSlipWheel2DOF_ALPMIN, Chassis_P.CombinedSlipWheel2DOF_CAMMIN,
                        Chassis_P.CombinedSlipWheel2DOF_CAMMAX, Chassis_P.CombinedSlipWheel2DOF_LONGVL,
                        Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS, Chassis_P.CombinedSlipWheel2DOF_RIM_RADIUS,
                        Chassis_P.CombinedSlipWheel2DOF_NOMPRES, Chassis_P.CombinedSlipWheel2DOF_FNOMIN,
                        Chassis_P.MagicTireConstInput_vdynMF[38], Chassis_P.MagicTireConstInput_vdynMF[43],
                        Chassis_P.MagicTireConstInput_vdynMF[42], Chassis_P.MagicTireConstInput_vdynMF[44],
                        Chassis_P.MagicTireConstInput_vdynMF[45], Chassis_P.MagicTireConstInput_vdynMF[46],
                        Chassis_P.MagicTireConstInput_vdynMF[47], Chassis_P.MagicTireConstInput_vdynMF[48],
                        Chassis_P.MagicTireConstInput_vdynMF[49], Chassis_P.MagicTireConstInput_vdynMF[51],
                        Chassis_P.MagicTireConstInput_vdynMF[52], Chassis_P.MagicTireConstInput_vdynMF[54],
                        Chassis_P.MagicTireConstInput_vdynMF[55], Chassis_P.MagicTireConstInput_vdynMF[62],
                        Chassis_P.MagicTireConstInput_vdynMF[63], Chassis_P.MagicTireConstInput_vdynMF[134],
                        Chassis_P.MagicTireConstInput_vdynMF[135], Chassis_P.MagicTireConstInput_vdynMF[136],
                        Chassis_P.MagicTireConstInput_vdynMF[137], Chassis_P.MagicTireConstInput_vdynMF[138],
                        Chassis_P.MagicTireConstInput_vdynMF[139], Chassis_P.MagicTireConstInput_vdynMF[140],
                        Chassis_P.MagicTireConstInput_vdynMF[141], Chassis_P.MagicTireConstInput_vdynMF[142],
                        Chassis_P.MagicTireConstInput_vdynMF[143], Chassis_P.MagicTireConstInput_vdynMF[144],
                        Chassis_P.MagicTireConstInput_vdynMF[145], Chassis_P.MagicTireConstInput_vdynMF[146],
                        Chassis_P.MagicTireConstInput_vdynMF[147], Chassis_P.MagicTireConstInput_vdynMF[148],
                        Chassis_P.MagicTireConstInput_vdynMF[149], Chassis_P.MagicTireConstInput_vdynMF[150],
                        Chassis_P.MagicTireConstInput_vdynMF[151], Chassis_P.MagicTireConstInput_vdynMF[152],
                        Chassis_P.MagicTireConstInput_vdynMF[153], Chassis_P.MagicTireConstInput_vdynMF[154],
                        Chassis_P.MagicTireConstInput_vdynMF[155], Chassis_P.MagicTireConstInput_vdynMF[156],
                        Chassis_P.MagicTireConstInput_vdynMF[157], Chassis_P.MagicTireConstInput_vdynMF[158],
                        Chassis_P.MagicTireConstInput_vdynMF[161], Chassis_P.MagicTireConstInput_vdynMF[162],
                        Chassis_P.MagicTireConstInput_vdynMF[163], Chassis_P.MagicTireConstInput_vdynMF[164],
                        Chassis_P.MagicTireConstInput_vdynMF[165], Chassis_P.MagicTireConstInput_vdynMF[166],
                        Chassis_P.MagicTireConstInput_vdynMF[167], Chassis_P.MagicTireConstInput_vdynMF[168],
                        Chassis_P.MagicTireConstInput_vdynMF[169], Chassis_P.MagicTireConstInput_vdynMF[170],
                        Chassis_P.MagicTireConstInput_vdynMF[174], Chassis_P.MagicTireConstInput_vdynMF[175],
                        Chassis_P.MagicTireConstInput_vdynMF[176], Chassis_P.MagicTireConstInput_vdynMF[177],
                        Chassis_P.MagicTireConstInput_vdynMF[178], Chassis_P.MagicTireConstInput_vdynMF[179],
                        Chassis_P.MagicTireConstInput_vdynMF[180], Chassis_P.MagicTireConstInput_vdynMF[182],
                        Chassis_P.MagicTireConstInput_vdynMF[183], Chassis_P.MagicTireConstInput_vdynMF[184],
                        Chassis_P.MagicTireConstInput_vdynMF[185], Chassis_P.MagicTireConstInput_vdynMF[186],
                        Chassis_P.MagicTireConstInput_vdynMF[187], Chassis_P.MagicTireConstInput_vdynMF[188],
                        Chassis_P.MagicTireConstInput_vdynMF[189], Chassis_P.MagicTireConstInput_vdynMF[190],
                        Chassis_P.MagicTireConstInput_vdynMF[195], Chassis_P.MagicTireConstInput_vdynMF[196],
                        Chassis_P.MagicTireConstInput_vdynMF[197], Chassis_P.MagicTireConstInput_vdynMF[198],
                        Chassis_P.MagicTireConstInput_vdynMF[199], Chassis_P.MagicTireConstInput_vdynMF[200],
                        Chassis_P.MagicTireConstInput_vdynMF[201], Chassis_P.MagicTireConstInput_vdynMF[202],
                        Chassis_P.MagicTireConstInput_vdynMF[203], Chassis_P.MagicTireConstInput_vdynMF[205],
                        Chassis_P.MagicTireConstInput_vdynMF[206], Chassis_P.MagicTireConstInput_vdynMF[207],
                        Chassis_P.MagicTireConstInput_vdynMF[208], Chassis_P.MagicTireConstInput_vdynMF[209],
                        Chassis_P.MagicTireConstInput_vdynMF[210], Chassis_P.MagicTireConstInput_vdynMF[213],
                        Chassis_P.MagicTireConstInput_vdynMF[214], Chassis_P.MagicTireConstInput_vdynMF[215],
                        Chassis_P.MagicTireConstInput_vdynMF[216], Chassis_P.MagicTireConstInput_vdynMF[217],
                        Chassis_P.MagicTireConstInput_vdynMF[218], Chassis_P.MagicTireConstInput_vdynMF[219],
                        Chassis_P.MagicTireConstInput_vdynMF[220], Chassis_P.MagicTireConstInput_vdynMF[221],
                        Chassis_P.MagicTireConstInput_vdynMF[222], Chassis_P.MagicTireConstInput_vdynMF[223],
                        Chassis_P.MagicTireConstInput_vdynMF[224], Chassis_P.MagicTireConstInput_vdynMF[225],
                        Chassis_P.MagicTireConstInput_vdynMF[226], Chassis_P.MagicTireConstInput_vdynMF[227],
                        Chassis_P.MagicTireConstInput_vdynMF[229], Chassis_P.MagicTireConstInput_vdynMF[230],
                        Chassis_P.MagicTireConstInput_vdynMF[231], Chassis_P.MagicTireConstInput_vdynMF[232],
                        Chassis_P.MagicTireConstInput_vdynMF[233], Chassis_P.MagicTireConstInput_vdynMF[234],
                        Chassis_P.MagicTireConstInput_vdynMF[235], Chassis_P.MagicTireConstInput_vdynMF[237],
                        Chassis_P.MagicTireConstInput_vdynMF[240], Chassis_P.MagicTireConstInput_vdynMF[241],
                        Chassis_P.MagicTireConstInput_vdynMF[242], Chassis_P.MagicTireConstInput_vdynMF[243],
                        Chassis_P.MagicTireConstInput_vdynMF[244], Chassis_P.MagicTireConstInput_vdynMF[245],
                        Chassis_P.MagicTireConstInput_vdynMF[246], Chassis_P.MagicTireConstInput_vdynMF[248],
                        Chassis_P.MagicTireConstInput_vdynMF[251], Chassis_P.MagicTireConstInput_vdynMF[252],
                        Chassis_P.MagicTireConstInput_vdynMF[253], Chassis_P.MagicTireConstInput_vdynMF[254],
                        Chassis_P.MagicTireConstInput_vdynMF[256], Chassis_P.MagicTireConstInput_vdynMF[257],
                        Chassis_P.MagicTireConstInput_vdynMF[258], Chassis_P.MagicTireConstInput_vdynMF[259],
                        Chassis_P.MagicTireConstInput_vdynMF[260], Chassis_P.MagicTireConstInput_vdynMF[261],
                        Chassis_P.MagicTireConstInput_vdynMF[262], Chassis_P.MagicTireConstInput_vdynMF[263],
                        Chassis_P.MagicTireConstInput_vdynMF[264], Chassis_P.MagicTireConstInput_vdynMF[265],
                        Chassis_P.MagicTireConstInput_vdynMF[266], Chassis_P.MagicTireConstInput_vdynMF[267],
                        Chassis_P.MagicTireConstInput_vdynMF[268], Chassis_P.MagicTireConstInput_vdynMF[269],
                        Chassis_P.MagicTireConstInput_vdynMF[270], Chassis_P.MagicTireConstInput_vdynMF[271],
                        Chassis_P.MagicTireConstInput_vdynMF[272], Chassis_P.MagicTireConstInput_vdynMF[273],
                        Chassis_P.MagicTireConstInput_vdynMF[274], Chassis_P.MagicTireConstInput_vdynMF[275],
                        Chassis_P.MagicTireConstInput_vdynMF[276], Chassis_P.MagicTireConstInput_vdynMF[277],
                        Chassis_P.MagicTireConstInput_vdynMF[278], Chassis_P.CombinedSlipWheel2DOF_WIDTH,
                        Chassis_P.MagicTireConstInput_vdynMF[86], Chassis_P.MagicTireConstInput_vdynMF[87],
                        Chassis_P.MagicTireConstInput_vdynMF[88], Chassis_P.MagicTireConstInput_vdynMF[89],
                        Chassis_P.MagicTireConstInput_vdynMF[171], Chassis_P.MagicTireConstInput_vdynMF[172],
                        Chassis_P.MagicTireConstInput_vdynMF[173], Chassis_P.MagicTireConstInput_vdynMF[50],
                        Chassis_P.MagicTireConstInput_vdynMF[64], Chassis_P.MagicTireConstInput_vdynMF[65],
                        Chassis_P.MagicTireConstInput_vdynMF[79], Chassis_P.MagicTireConstInput_vdynMF[80],
                        Chassis_P.MagicTireConstInput_vdynMF[81], Chassis_P.MagicTireConstInput_vdynMF[82],
                        Chassis_P.MagicTireConstInput_vdynMF[83], Chassis_P.MagicTireConstInput_vdynMF[84],
                        rtb_Integrator, rtb_Integrator_g, lam_mux, Gykappa, rtb_xdot, rtb_Mx, rtb_My, cosprimealpha,
                        Chassis_B.Re, rtb_Kappa, rtb_Alpha, rtb_sig_x, rtb_sig_y, rtb_a, rtb_b);
  }

  /* End of MATLAB Function: '<S229>/Magic Tire Const Input' */

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[0] = rtb_Mx[0];

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[4] = rtb_Integrator_c[0];

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[8] = cosprimealpha[0];

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[1] = rtb_Mx[1];

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[5] = rtb_Integrator_c[1];

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[9] = cosprimealpha[1];

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[2] = rtb_Mx[2];

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[6] = rtb_Integrator_c[2];

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[10] = cosprimealpha[2];

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[3] = rtb_Mx[3];

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[7] = rtb_Integrator_c[3];

  /* SignalConversion generated from: '<S213>/Vector Concatenate1' */
  rtb_VectorConcatenate1_g[11] = cosprimealpha[3];
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 4; rtb_phidot_tmp++) {
    /* Math: '<S213>/Math Function1' incorporates:
     *  Concatenate: '<S213>/Vector Concatenate1'
     */
    Chassis_B.MathFunction1[3 * rtb_phidot_tmp] = rtb_VectorConcatenate1_g[rtb_phidot_tmp];

    /* Math: '<S213>/Math Function1' incorporates:
     *  Concatenate: '<S218>/Vector Concatenate3'
     */
    Itemp_tmp = 3 * rtb_phidot_tmp + 1;

    /* Math: '<S213>/Math Function1' incorporates:
     *  Concatenate: '<S213>/Vector Concatenate1'
     */
    Chassis_B.MathFunction1[Itemp_tmp] = rtb_VectorConcatenate1_g[rtb_phidot_tmp + 4];

    /* Math: '<S213>/Math Function1' incorporates:
     *  Concatenate: '<S218>/Vector Concatenate3'
     *  Selector: '<S4>/Selector2'
     */
    iy = 3 * rtb_phidot_tmp + 2;

    /* Math: '<S213>/Math Function1' incorporates:
     *  Concatenate: '<S213>/Vector Concatenate1'
     */
    Chassis_B.MathFunction1[iy] = rtb_VectorConcatenate1_g[rtb_phidot_tmp + 8];

    /* Concatenate: '<S218>/Vector Concatenate3' incorporates:
     *  Constant: '<S218>/Constant'
     *  Integrator: '<S211>/Integrator1'
     *  Selector: '<S4>/Selector2'
     *  Selector: '<S4>/Selector3'
     */
    Chassis_B.VectorConcatenate3_b[3 * rtb_phidot_tmp] = Chassis_X.Integrator1_CSTATE_p[3 * rtb_phidot_tmp];
    Chassis_B.VectorConcatenate3_b[Itemp_tmp] = Chassis_P.Constant_Value_a[rtb_phidot_tmp];
    Chassis_B.VectorConcatenate3_b[iy] = Chassis_X.Integrator1_CSTATE_p[iy];
  }

  /* Outputs for Iterator SubSystem: '<S213>/Wheel to Body Transform' incorporates:
   *  ForEach: '<S220>/For Each'
   */
  for (ForEach_itr_k = 0; ForEach_itr_k < 4; ForEach_itr_k++) {
    /* Product: '<S221>/Element Product' incorporates:
     *  Constant: '<S220>/Constant1'
     *  ForEachSliceSelector generated from: '<S220>/<M>'
     *  ForEachSliceSelector generated from: '<S220>/<Re>'
     *  ForEachSliceSelector generated from: '<S220>/Forces'
     *  ForEachSliceSelector generated from: '<S220>/WheelAngles'
     *  Math: '<S213>/Math Function'
     *  SignalConversion generated from: '<S222>/sincos'
     *  Sum: '<S220>/Add'
     */
    tmp[0] = Chassis_P.CoreSubsys_pn.Constant1_Value[1];
    tmp[1] = Chassis_B.Re[ForEach_itr_k];
    tmp[2] = Chassis_P.CoreSubsys_pn.Constant1_Value[0];
    tmp[3] = Chassis_B.Re[ForEach_itr_k];
    rtb_phidot_tmp = 3 * ForEach_itr_k + 2;
    Drphi_0 = Chassis_B.MathFunction[rtb_phidot_tmp];
    tmp_0[0] = Drphi_0;
    tmp[4] = Chassis_P.CoreSubsys_pn.Constant1_Value[0];
    Fzo_prime_idx_2 = Chassis_B.MathFunction[3 * ForEach_itr_k];
    tmp_0[1] = Fzo_prime_idx_2;
    Itemp_tmp = 3 * ForEach_itr_k + 1;
    rtb_Add_n = Chassis_B.MathFunction[Itemp_tmp];
    tmp_0[3] = rtb_Add_n;
    tmp[5] = Chassis_P.CoreSubsys_pn.Constant1_Value[1];
    tmp_0[2] = rtb_Add_n;
    tmp_0[4] = Drphi_0;
    tmp_0[5] = Fzo_prime_idx_2;
    for (iy = 0; iy <= 4; iy += 2) {
      /* Product: '<S221>/Element Product' */
      tmp_5 = _mm_loadu_pd(&tmp[iy]);
      tmp_6 = _mm_loadu_pd(&tmp_0[iy]);
      _mm_storeu_pd(&rtb_ElementProduct[iy], _mm_mul_pd(tmp_5, tmp_6));
    }

    /* Sum: '<S220>/Add' incorporates:
     *  ForEachSliceSelector generated from: '<S220>/<M>'
     *  Sum: '<S221>/Sum'
     *  Trigonometry: '<S124>/sincos'
     */
    rtb_sincos_o2_a[0] = (rtb_ElementProduct[0] - rtb_ElementProduct[3]) + Chassis_B.MathFunction1[3 * ForEach_itr_k];
    rtb_sincos_o2_a[1] = (rtb_ElementProduct[1] - rtb_ElementProduct[4]) + Chassis_B.MathFunction1[Itemp_tmp];
    rtb_sincos_o2_a[2] = (rtb_ElementProduct[2] - rtb_ElementProduct[5]) + Chassis_B.MathFunction1[rtb_phidot_tmp];
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      for (iy = 0; iy < 3; iy++) {
        /* Math: '<S220>/Transpose1' incorporates:
         *  ForEachSliceSelector generated from: '<S220>/DCM'
         */
        ibmat = 9 * ForEach_itr_k + iy;

        /* Math: '<S220>/Transpose1' incorporates:
         *  Constant: '<S1>/Constant6'
         *  ForEachSliceSelector generated from: '<S220>/DCM'
         */
        Chassis_B.CoreSubsys_pn[ForEach_itr_k].Transpose1[3 * iy] = Chassis_P.Constant6_Value[ibmat];
        Chassis_B.CoreSubsys_pn[ForEach_itr_k].Transpose1[3 * iy + 1] = Chassis_P.Constant6_Value[ibmat + 3];
        Chassis_B.CoreSubsys_pn[ForEach_itr_k].Transpose1[3 * iy + 2] = Chassis_P.Constant6_Value[ibmat + 6];
      }
    }

    /* SignalConversion generated from: '<S222>/sincos' incorporates:
     *  Concatenate: '<S218>/Vector Concatenate3'
     *  ForEachSliceSelector generated from: '<S220>/WheelAngles'
     */
    rtb_Add_ms[0] = Chassis_B.VectorConcatenate3_b[rtb_phidot_tmp];
    rtb_Add_ms[1] = Chassis_B.VectorConcatenate3_b[Itemp_tmp];
    rtb_Add_ms[2] = Chassis_B.VectorConcatenate3_b[3 * ForEach_itr_k];

    /* Trigonometry: '<S222>/sincos' */
    rtb_Add1_b_idx_1 = std::cos(rtb_Add_ms[0]);
    Fzo_prime_idx_2 = std::sin(rtb_Add_ms[0]);
    rtb_ixk = std::cos(rtb_Add_ms[1]);
    rtb_Add_n = std::sin(rtb_Add_ms[1]);
    rtb_Product_ju = std::cos(rtb_Add_ms[2]);
    lam_mux_0 = std::sin(rtb_Add_ms[2]);

    /* Fcn: '<S222>/Fcn11' */
    rtb_VectorConcatenate_i[0] = rtb_Add1_b_idx_1 * rtb_ixk;

    /* Fcn: '<S222>/Fcn21' incorporates:
     *  Fcn: '<S222>/Fcn22'
     */
    rtb_thetadot = rtb_Add_n * lam_mux_0;
    rtb_VectorConcatenate_i[1] = rtb_thetadot * rtb_Add1_b_idx_1 - Fzo_prime_idx_2 * rtb_Product_ju;

    /* Fcn: '<S222>/Fcn31' incorporates:
     *  Fcn: '<S222>/Fcn32'
     */
    rtb_UnaryMinus_i = rtb_Add_n * rtb_Product_ju;
    rtb_VectorConcatenate_i[2] = rtb_UnaryMinus_i * rtb_Add1_b_idx_1 + Fzo_prime_idx_2 * lam_mux_0;

    /* Fcn: '<S222>/Fcn12' */
    rtb_VectorConcatenate_i[3] = Fzo_prime_idx_2 * rtb_ixk;

    /* Fcn: '<S222>/Fcn22' */
    rtb_VectorConcatenate_i[4] = rtb_thetadot * Fzo_prime_idx_2 + rtb_Add1_b_idx_1 * rtb_Product_ju;

    /* Fcn: '<S222>/Fcn32' */
    rtb_VectorConcatenate_i[5] = rtb_UnaryMinus_i * Fzo_prime_idx_2 - rtb_Add1_b_idx_1 * lam_mux_0;

    /* Fcn: '<S222>/Fcn13' */
    rtb_VectorConcatenate_i[6] = -rtb_Add_n;

    /* Fcn: '<S222>/Fcn23' */
    rtb_VectorConcatenate_i[7] = rtb_ixk * lam_mux_0;

    /* Fcn: '<S222>/Fcn33' */
    rtb_VectorConcatenate_i[8] = rtb_ixk * rtb_Product_ju;
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Trigonometry: '<S223>/sincos' incorporates:
       *  Constant: '<S220>/Constant'
       *  SignalConversion generated from: '<S223>/sincos'
       */
      rtb_Add1_b_idx_1 = std::cos(Chassis_P.CoreSubsys_pn.Constant_Value[2]);
      Fzo_prime_idx_2 = std::sin(Chassis_P.CoreSubsys_pn.Constant_Value[2]);
      rtb_ixk = std::cos(Chassis_P.CoreSubsys_pn.Constant_Value[1]);
      rtb_Add_n = std::sin(Chassis_P.CoreSubsys_pn.Constant_Value[1]);
      rtb_Product_ju = std::cos(Chassis_P.CoreSubsys_pn.Constant_Value[0]);
      lam_mux_0 = std::sin(Chassis_P.CoreSubsys_pn.Constant_Value[0]);

      /* Fcn: '<S223>/Fcn11' incorporates:
       *  Concatenate: '<S225>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[0] = rtb_Add1_b_idx_1 * rtb_ixk;

      /* Fcn: '<S223>/Fcn21' incorporates:
       *  Concatenate: '<S225>/Vector Concatenate'
       *  Fcn: '<S223>/Fcn22'
       */
      rtb_thetadot = rtb_Add_n * lam_mux_0;
      Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[1] =
          rtb_thetadot * rtb_Add1_b_idx_1 - Fzo_prime_idx_2 * rtb_Product_ju;

      /* Fcn: '<S223>/Fcn31' incorporates:
       *  Concatenate: '<S225>/Vector Concatenate'
       *  Fcn: '<S223>/Fcn32'
       */
      rtb_UnaryMinus_i = rtb_Add_n * rtb_Product_ju;
      Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[2] =
          rtb_UnaryMinus_i * rtb_Add1_b_idx_1 + Fzo_prime_idx_2 * lam_mux_0;

      /* Fcn: '<S223>/Fcn12' incorporates:
       *  Concatenate: '<S225>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[3] = Fzo_prime_idx_2 * rtb_ixk;

      /* Fcn: '<S223>/Fcn22' incorporates:
       *  Concatenate: '<S225>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[4] =
          rtb_thetadot * Fzo_prime_idx_2 + rtb_Add1_b_idx_1 * rtb_Product_ju;

      /* Fcn: '<S223>/Fcn32' incorporates:
       *  Concatenate: '<S225>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[5] =
          rtb_UnaryMinus_i * Fzo_prime_idx_2 - rtb_Add1_b_idx_1 * lam_mux_0;

      /* Fcn: '<S223>/Fcn13' incorporates:
       *  Concatenate: '<S225>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[6] = -rtb_Add_n;

      /* Fcn: '<S223>/Fcn23' incorporates:
       *  Concatenate: '<S225>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[7] = rtb_ixk * lam_mux_0;

      /* Fcn: '<S223>/Fcn33' incorporates:
       *  Concatenate: '<S225>/Vector Concatenate'
       */
      Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[8] = rtb_ixk * rtb_Product_ju;
    }

    /* Product: '<S220>/Divide1' incorporates:
     *  Concatenate: '<S101>/Vector Concatenate'
     *  Concatenate: '<S225>/Vector Concatenate'
     *  ForEachSliceSelector generated from: '<S220>/Forces'
     *  Math: '<S213>/Math Function'
     *  Math: '<S220>/Transpose'
     *  Math: '<S220>/Transpose1'
     *  Math: '<S97>/Transpose'
     *  Product: '<S220>/Divide2'
     *  Product: '<S220>/Divide3'
     *  Trigonometry: '<S124>/sincos'
     */
    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
      for (Itemp_tmp = 0; Itemp_tmp < 3; Itemp_tmp++) {
        iy = 3 * Itemp_tmp + rtb_phidot_tmp;
        rtb_VectorConcatenate_b_0[iy] = 0.0;
        rtb_VectorConcatenate_b_0[iy] += rtb_VectorConcatenate_i[3 * rtb_phidot_tmp] *
                                         Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[3 * Itemp_tmp];
        rtb_VectorConcatenate_b_0[iy] += rtb_VectorConcatenate_i[3 * rtb_phidot_tmp + 1] *
                                         Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[3 * Itemp_tmp + 1];
        rtb_VectorConcatenate_b_0[iy] += rtb_VectorConcatenate_i[3 * rtb_phidot_tmp + 2] *
                                         Chassis_B.CoreSubsys_pn[ForEach_itr_k].VectorConcatenate[3 * Itemp_tmp + 2];
      }
    }

    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
      /* Product: '<S220>/Divide2' */
      rtb_sincos_o1_d[rtb_phidot_tmp] = 0.0;

      /* Product: '<S220>/Divide3' */
      rtb_Divide3[rtb_phidot_tmp] = 0.0;
      for (Itemp_tmp = 0; Itemp_tmp < 3; Itemp_tmp++) {
        /* Product: '<S220>/Divide2' incorporates:
         *  Product: '<S220>/Divide3'
         */
        iy = 3 * Itemp_tmp + rtb_phidot_tmp;
        rtb_Transpose_n[iy] = 0.0;
        rtb_Transpose_n[iy] += rtb_VectorConcatenate_b_0[3 * Itemp_tmp] *
                               Chassis_B.CoreSubsys_pn[ForEach_itr_k].Transpose1[rtb_phidot_tmp];
        rtb_Transpose_n[iy] += rtb_VectorConcatenate_b_0[3 * Itemp_tmp + 1] *
                               Chassis_B.CoreSubsys_pn[ForEach_itr_k].Transpose1[rtb_phidot_tmp + 3];
        rtb_Transpose_n[iy] += rtb_VectorConcatenate_b_0[3 * Itemp_tmp + 2] *
                               Chassis_B.CoreSubsys_pn[ForEach_itr_k].Transpose1[rtb_phidot_tmp + 6];
        rtb_sincos_o1_d[rtb_phidot_tmp] += Chassis_B.MathFunction[3 * ForEach_itr_k + Itemp_tmp] * rtb_Transpose_n[iy];
        rtb_Divide3[rtb_phidot_tmp] += rtb_Transpose_n[iy] * rtb_sincos_o2_a[Itemp_tmp];
      }
    }

    /* End of Product: '<S220>/Divide1' */

    /* ForEachSliceAssignment generated from: '<S220>/Mz' */
    rtb_ImpAsg_InsertedFor_Mz_at_inport_0[ForEach_itr_k] = rtb_Divide3[2];

    /* ForEachSliceAssignment generated from: '<S220>/My' */
    rtb_ImpAsg_InsertedFor_My_at_inport_0[ForEach_itr_k] = rtb_Divide3[1];

    /* ForEachSliceAssignment generated from: '<S220>/Mx' */
    rtb_ImpAsg_InsertedFor_Mx_at_inport_0[ForEach_itr_k] = rtb_Divide3[0];

    /* ForEachSliceAssignment generated from: '<S220>/Fy' */
    rtb_ImpAsg_InsertedFor_Fy_at_inport_0[ForEach_itr_k] = rtb_sincos_o1_d[1];

    /* ForEachSliceAssignment generated from: '<S220>/Fx' */
    rtb_ImpAsg_InsertedFor_Fx_at_inport_0[ForEach_itr_k] = rtb_sincos_o1_d[0];
  }

  /* End of Outputs for SubSystem: '<S213>/Wheel to Body Transform' */

  /* Concatenate: '<S9>/Matrix Concatenate4' incorporates:
   *  ForEachSliceAssignment generated from: '<S220>/Fx'
   *  ForEachSliceAssignment generated from: '<S220>/Fy'
   */
  rtb_MatrixConcatenate4[0] = rtb_ImpAsg_InsertedFor_Fx_at_inport_0[0];
  rtb_MatrixConcatenate4[1] = rtb_ImpAsg_InsertedFor_Fy_at_inport_0[0];

  /* Concatenate: '<S9>/Matrix Concatenate2' incorporates:
   *  UnaryMinus: '<S217>/Unary Minus1'
   *  UnaryMinus: '<S217>/Unary Minus2'
   */
  rtb_MatrixConcatenate2[0] = rtb_z_idx_0;
  rtb_MatrixConcatenate2[1] = -rtb_Switch[0];

  /* Concatenate: '<S9>/Matrix Concatenate4' incorporates:
   *  ForEachSliceAssignment generated from: '<S220>/Fx'
   *  ForEachSliceAssignment generated from: '<S220>/Fy'
   */
  rtb_MatrixConcatenate4[2] = rtb_ImpAsg_InsertedFor_Fx_at_inport_0[1];
  rtb_MatrixConcatenate4[3] = rtb_ImpAsg_InsertedFor_Fy_at_inport_0[1];

  /* Concatenate: '<S9>/Matrix Concatenate2' incorporates:
   *  UnaryMinus: '<S217>/Unary Minus1'
   *  UnaryMinus: '<S217>/Unary Minus2'
   */
  rtb_MatrixConcatenate2[2] = rtb_z_idx_1;
  rtb_MatrixConcatenate2[3] = -rtb_Switch[1];

  /* Concatenate: '<S9>/Matrix Concatenate4' incorporates:
   *  ForEachSliceAssignment generated from: '<S220>/Fx'
   *  ForEachSliceAssignment generated from: '<S220>/Fy'
   */
  rtb_MatrixConcatenate4[4] = rtb_ImpAsg_InsertedFor_Fx_at_inport_0[2];
  rtb_MatrixConcatenate4[5] = rtb_ImpAsg_InsertedFor_Fy_at_inport_0[2];

  /* Concatenate: '<S9>/Matrix Concatenate2' incorporates:
   *  UnaryMinus: '<S217>/Unary Minus1'
   *  UnaryMinus: '<S217>/Unary Minus2'
   */
  rtb_MatrixConcatenate2[4] = rtb_z_idx_2;
  rtb_MatrixConcatenate2[5] = -rtb_Switch[2];

  /* Concatenate: '<S9>/Matrix Concatenate4' incorporates:
   *  ForEachSliceAssignment generated from: '<S220>/Fx'
   *  ForEachSliceAssignment generated from: '<S220>/Fy'
   */
  rtb_MatrixConcatenate4[6] = rtb_ImpAsg_InsertedFor_Fx_at_inport_0[3];
  rtb_MatrixConcatenate4[7] = rtb_ImpAsg_InsertedFor_Fy_at_inport_0[3];

  /* Concatenate: '<S9>/Matrix Concatenate2' incorporates:
   *  UnaryMinus: '<S217>/Unary Minus1'
   *  UnaryMinus: '<S217>/Unary Minus2'
   */
  rtb_MatrixConcatenate2[6] = -rtb_IntegratorSecondOrder_o1_k;
  rtb_MatrixConcatenate2[7] = -rtb_Switch[3];

  /* Concatenate: '<S9>/Matrix Concatenate3' incorporates:
   *  Constant: '<S12>/Inertial Frame CG to Axle Offset'
   *  Reshape: '<S12>/Reshape'
   *  Reshape: '<S12>/Reshape1'
   *  Selector: '<S9>/Selector'
   *  Selector: '<S9>/Selector1'
   *  Sum: '<S12>/Sum'
   */
  rtb_MatrixConcatenate3[0] = rtb_Add_ct[2] - Chassis_P.InertialFrameCGtoAxleOffset_Value[2];
  rtb_MatrixConcatenate3[1] = rtb_V_wb[2];
  rtb_MatrixConcatenate3[2] = rtb_Add_fb[2] - Chassis_P.InertialFrameCGtoAxleOffset_Value[5];
  rtb_MatrixConcatenate3[3] = rtb_Sum_hy[2];
  rtb_MatrixConcatenate3[4] = rtb_Add_d1[2] - Chassis_P.InertialFrameCGtoAxleOffset_Value[8];
  rtb_MatrixConcatenate3[5] = rtb_UnaryMinus_b[2];
  rtb_MatrixConcatenate3[6] = rtb_Add_cm[2] - Chassis_P.InertialFrameCGtoAxleOffset_Value[11];
  rtb_MatrixConcatenate3[7] = rtb_UnaryMinus1[2];

  /* Trigonometry: '<S9>/Small angle approximation for downstream multiplications' */
  rtb_Add1_b_idx_1 = std::tan(rtb_Subtract1_idx_0);

  /* Concatenate: '<S13>/Matrix Concatenate' incorporates:
   *  ForEachSliceAssignment generated from: '<S220>/Mx'
   *  ForEachSliceAssignment generated from: '<S220>/My'
   *  ForEachSliceAssignment generated from: '<S220>/Mz'
   */
  rtb_VectorConcatenate1_g[0] = rtb_ImpAsg_InsertedFor_Mx_at_inport_0[0];
  rtb_VectorConcatenate1_g[1] = rtb_ImpAsg_InsertedFor_My_at_inport_0[0];
  rtb_VectorConcatenate1_g[2] = rtb_ImpAsg_InsertedFor_Mz_at_inport_0[0];

  /* Gain: '<S6>/Gain' incorporates:
   *  Integrator: '<S8>/Integrator1'
   */
  rtb_UnaryMinus_i = Chassis_P.Gain_Gain * Chassis_X.Integrator1_CSTATE_o[0];

  /* Concatenate: '<S13>/Matrix Concatenate' incorporates:
   *  ForEachSliceAssignment generated from: '<S220>/Mx'
   *  ForEachSliceAssignment generated from: '<S220>/My'
   *  ForEachSliceAssignment generated from: '<S220>/Mz'
   */
  rtb_VectorConcatenate1_g[3] = rtb_ImpAsg_InsertedFor_Mx_at_inport_0[1];
  rtb_VectorConcatenate1_g[4] = rtb_ImpAsg_InsertedFor_My_at_inport_0[1];
  rtb_VectorConcatenate1_g[5] = rtb_ImpAsg_InsertedFor_Mz_at_inport_0[1];

  /* Gain: '<S6>/Gain' incorporates:
   *  Integrator: '<S8>/Integrator1'
   */
  Fzo_prime_idx_2 = Chassis_P.Gain_Gain * Chassis_X.Integrator1_CSTATE_o[1];

  /* Concatenate: '<S13>/Matrix Concatenate' incorporates:
   *  ForEachSliceAssignment generated from: '<S220>/Mx'
   *  ForEachSliceAssignment generated from: '<S220>/My'
   *  ForEachSliceAssignment generated from: '<S220>/Mz'
   */
  rtb_VectorConcatenate1_g[6] = rtb_ImpAsg_InsertedFor_Mx_at_inport_0[2];
  rtb_VectorConcatenate1_g[7] = rtb_ImpAsg_InsertedFor_My_at_inport_0[2];
  rtb_VectorConcatenate1_g[8] = rtb_ImpAsg_InsertedFor_Mz_at_inport_0[2];

  /* Gain: '<S6>/Gain' incorporates:
   *  Integrator: '<S8>/Integrator1'
   */
  rtb_thetadot = Chassis_P.Gain_Gain * Chassis_X.Integrator1_CSTATE_o[2];

  /* Concatenate: '<S13>/Matrix Concatenate' incorporates:
   *  ForEachSliceAssignment generated from: '<S220>/Mx'
   *  ForEachSliceAssignment generated from: '<S220>/My'
   *  ForEachSliceAssignment generated from: '<S220>/Mz'
   */
  rtb_VectorConcatenate1_g[9] = rtb_ImpAsg_InsertedFor_Mx_at_inport_0[3];
  rtb_VectorConcatenate1_g[10] = rtb_ImpAsg_InsertedFor_My_at_inport_0[3];
  rtb_VectorConcatenate1_g[11] = rtb_ImpAsg_InsertedFor_Mz_at_inport_0[3];

  /* Gain: '<S6>/Gain' incorporates:
   *  Integrator: '<S8>/Integrator1'
   */
  rtb_ixk = Chassis_P.Gain_Gain * Chassis_X.Integrator1_CSTATE_o[3];

  /* Signum: '<S6>/Sign' */
  if (std::isnan(rtb_UnaryMinus_i)) {
    Drphi_0 = (rtNaN);
  } else if (rtb_UnaryMinus_i < 0.0) {
    Drphi_0 = -1.0;
  } else {
    Drphi_0 = (rtb_UnaryMinus_i > 0.0);
  }

  if (std::isnan(Fzo_prime_idx_2)) {
    Fzo_prime_idx_2 = (rtNaN);
  } else if (Fzo_prime_idx_2 < 0.0) {
    Fzo_prime_idx_2 = -1.0;
  } else {
    Fzo_prime_idx_2 = (Fzo_prime_idx_2 > 0.0);
  }

  /* Sum: '<S14>/Add' incorporates:
   *  Abs: '<S14>/Abs'
   *  Abs: '<S14>/Abs1'
   *  Constant: '<S14>/Front Track'
   *  Integrator: '<S7>/Integrator1'
   *  Product: '<S14>/Product'
   *  Product: '<S14>/Product1'
   *  Signum: '<S6>/Sign'
   */
  rtb_VectorConcatenate_b[0] = (std::abs(Chassis_X.Integrator1_CSTATE_a[0]) * Drphi_0 + Chassis_P.FrontTrack_Value) +
                               std::abs(Chassis_X.Integrator1_CSTATE_a[1]) * Fzo_prime_idx_2;

  /* Signum: '<S6>/Sign' */
  if (std::isnan(rtb_thetadot)) {
    Drphi_0 = (rtNaN);
  } else if (rtb_thetadot < 0.0) {
    Drphi_0 = -1.0;
  } else {
    Drphi_0 = (rtb_thetadot > 0.0);
  }

  if (std::isnan(rtb_ixk)) {
    Fzo_prime_idx_2 = (rtNaN);
  } else if (rtb_ixk < 0.0) {
    Fzo_prime_idx_2 = -1.0;
  } else {
    Fzo_prime_idx_2 = (rtb_ixk > 0.0);
  }

  /* Sum: '<S15>/Add1' incorporates:
   *  Abs: '<S15>/Abs2'
   *  Abs: '<S15>/Abs3'
   *  Constant: '<S15>/Rear Track'
   *  Integrator: '<S7>/Integrator1'
   *  Product: '<S15>/Product2'
   *  Product: '<S15>/Product3'
   *  Signum: '<S6>/Sign'
   */
  rtb_VectorConcatenate_b[1] = (std::abs(Chassis_X.Integrator1_CSTATE_a[2]) * Drphi_0 + Chassis_P.RearTrack_Value) +
                               std::abs(Chassis_X.Integrator1_CSTATE_a[3]) * Fzo_prime_idx_2;

  /* Outputs for Iterator SubSystem: '<S9>/For each track and axle combination calculate suspension forces and moments'
   * incorporates: ForEach: '<S17>/For Each'
   */
  for (ForEach_itr_n = 0; ForEach_itr_n < 4; ForEach_itr_n++) {
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Sum: '<S19>/Sum2' incorporates:
       *  Constant: '<S19>/Vehicle Vehicle Wheel Offset3'
       *  Constant: '<S9>/Axle Number'
       *  Constant: '<S9>/Wheel Number'
       *  ForEachSliceSelector generated from: '<S17>/Axle Number'
       *  ForEachSliceSelector generated from: '<S17>/Wheel Number'
       *  Selector: '<S19>/Selector1'
       */
      /* Unit Conversion - from: deg to: rad
         Expression: output = (0.0174533*input) + (0) */
      Chassis_B.CoreSubsys[ForEach_itr_n].Sum2 =
          Chassis_P.CoreSubsys.VehicleVehicleWheelOffset3_Value
              [static_cast<int32_T>(Chassis_P.IndependentKandCSuspension_AxleNumVec[ForEach_itr_n]) - 1] +
          Chassis_P.IndependentKandCSuspension_WhlNumVec[ForEach_itr_n];

      /* Selector: '<S26>/Selector5' incorporates:
       *  Constant: '<S26>/Constant4'
       *  UnitConversion: '<S26>/Unit Conversion'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].Selector5 =
          Chassis_P
              .IndependentKandCSuspension_StatCamber[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) -
                                                     1] *
          0.017453292519943295;
    }

    /* Selector: '<S19>/Selector' incorporates:
     *  Selector: '<S19>/Selector2'
     *  Sum: '<S76>/Add'
     */
    rtb_phidot_tmp = (static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1) << 1;

    /* Sum: '<S76>/Add' incorporates:
     *  Concatenate: '<S9>/Matrix Concatenate2'
     *  Concatenate: '<S9>/Matrix Concatenate3'
     *  Selector: '<S19>/Selector'
     *  Selector: '<S19>/Selector2'
     */
    rtb_phidot = rtb_MatrixConcatenate3[rtb_phidot_tmp] - rtb_MatrixConcatenate2[rtb_phidot_tmp];

    /* Gain: '<S77>/Height Sign Convention' */
    rtb_thetadot = Chassis_P.CoreSubsys.HeightSignConvention_Gain * rtb_phidot;

    /* Gain: '<S24>/Sign Convention' */
    rtb_psidot = Chassis_P.CoreSubsys.SignConvention_Gain * rtb_thetadot;

    /* Gain: '<S26>/Gain5' */
    rtb_kxj = Chassis_P.CoreSubsys.Gain5_Gain * rtb_psidot;

    /* Lookup_n-D: '<S39>/X-Y FL' incorporates:
     *  Product: '<S130>/k x j'
     */
    rtb_VectorConcatenate_pe[0] =
        look1_pbinlcapw(rtb_kxj, Chassis_P.CoreSubsys.XYFL_bp01Data_g, Chassis_P.CoreSubsys.XYFL_tableData_o,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex, m_bumpcamber_vs_compress_maxIndex);

    /* Lookup_n-D: '<S39>/X-Y FR' incorporates:
     *  Product: '<S130>/k x j'
     */
    rtb_VectorConcatenate_pe[1] =
        look1_pbinlcapw(rtb_kxj, Chassis_P.CoreSubsys.XYFR_bp01Data_g, Chassis_P.CoreSubsys.XYFR_tableData_j,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_k, m_bumpcamber_vs_compress_maxIndex);

    /* Lookup_n-D: '<S39>/X-Y RL' incorporates:
     *  Product: '<S130>/k x j'
     */
    rtb_VectorConcatenate_pe[2] =
        look1_pbinlcapw(rtb_kxj, Chassis_P.CoreSubsys.XYRL_bp01Data_p, Chassis_P.CoreSubsys.XYRL_tableData_a,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_p, m_bumpcamber_vs_compress_maxIndex_rear);

    /* Lookup_n-D: '<S39>/X-Y RR' incorporates:
     *  Product: '<S130>/k x j'
     */
    rtb_VectorConcatenate_pe[3] =
        look1_pbinlcapw(rtb_kxj, Chassis_P.CoreSubsys.XYRR_bp01Data_i, Chassis_P.CoreSubsys.XYRR_tableData_l,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_a, m_bumpcamber_vs_compress_maxIndex_rear);

    /* UnitConversion: '<S39>/Unit Conversion' incorporates:
     *  Selector: '<S39>/Selector5'
     */
    /* Unit Conversion - from: deg to: rad
       Expression: output = (0.0174533*input) + (0) */
    rtb_xdot_tmp = rtb_VectorConcatenate_pe[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1] *
                   0.017453292519943295;
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Sum: '<S44>/Sum of Elements' incorporates:
       *  Constant: '<S43>/Constant3'
       *  Constant: '<S44>/Axle Numbers'
       *  Constant: '<S9>/Axle Number'
       *  DataTypeConversion: '<S44>/Data Type Conversion'
       *  ForEachSliceSelector generated from: '<S17>/Axle Number'
       *  Product: '<S44>/Product'
       *  RelationalOperator: '<S44>/Relational Operator'
       *  Selector: '<S39>/Selector5'
       *  Selector: '<S43>/Selector1'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].SumofElements =
          Chassis_P.CambVsSteerAng[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1] *
          static_cast<real_T>(Chassis_P.IndependentKandCSuspension_AxleNumVec[ForEach_itr_n] ==
                              Chassis_P.CoreSubsys.SelectSteerCamberSlope_AxleNums);

      /* Selector: '<S20>/Selector5' incorporates:
       *  Constant: '<S9>/Axle Number'
       *  ForEachSliceSelector generated from: '<S17>/Axle Number'
       *  Selector: '<S64>/Selector1'
       */
      rtb_phidot_tmp = static_cast<int32_T>(Chassis_P.IndependentKandCSuspension_AxleNumVec[ForEach_itr_n]) - 1;

      /* Product: '<S20>/Product' incorporates:
       *  Constant: '<S20>/Vehicle Vehicle Wheel Offset1'
       *  Selector: '<S20>/Selector5'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].Product =
          Chassis_P.CoreSubsys.VehicleVehicleWheelOffset1_Value[rtb_phidot_tmp] *
          Chassis_B.CoreSubsys[ForEach_itr_n].Sum2;

      /* Sum: '<S64>/Sum1' incorporates:
       *  Constant: '<S64>/Constant'
       *  Constant: '<S64>/Vehicle Vehicle Wheel Offset3'
       *  Constant: '<S9>/Wheel Number'
       *  ForEachSliceSelector generated from: '<S17>/Wheel Number'
       *  Gain: '<S64>/Gain'
       *  Selector: '<S64>/Selector1'
       *  Sum: '<S64>/Sum'
       */
      /* Unit Conversion - from: deg to: rad
         Expression: output = (0.0174533*input) + (0) */
      rtb_UnaryMinus_i = (Chassis_P.CoreSubsys.VehicleVehicleWheelOffset3_Value_k[rtb_phidot_tmp] +
                          Chassis_P.CoreSubsys.Constant_Value_d) *
                             Chassis_P.CoreSubsys.Gain_Gain -
                         Chassis_P.IndependentKandCSuspension_WhlNumVec[ForEach_itr_n];

      /* Signum: '<S64>/Sign' */
      if (std::isnan(rtb_UnaryMinus_i)) {
        Drphi_0 = (rtNaN);
      } else if (rtb_UnaryMinus_i < 0.0) {
        Drphi_0 = -1.0;
      } else {
        Drphi_0 = (rtb_UnaryMinus_i > 0.0);
      }

      /* Product: '<S64>/Product' incorporates:
       *  Constant: '<S31>/Constant1'
       *  Selector: '<S39>/Selector5'
       *  Selector: '<S64>/Selector5'
       *  Signum: '<S64>/Sign'
       *  UnitConversion: '<S31>/Unit Conversion'
       */
      rtb_Product_ju =
          Chassis_P
              .IndependentKandCSuspension_StatToe[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1] *
          0.017453292519943295 * Drphi_0;

      /* Sum: '<S70>/Sum of Elements' incorporates:
       *  Constant: '<S70>/Axle Numbers'
       *  Constant: '<S9>/Axle Number'
       *  DataTypeConversion: '<S70>/Data Type Conversion'
       *  ForEachSliceSelector generated from: '<S17>/Axle Number'
       *  Product: '<S70>/Product'
       *  RelationalOperator: '<S70>/Relational Operator'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].SumofElements_j =
          static_cast<real_T>(Chassis_P.IndependentKandCSuspension_AxleNumVec[ForEach_itr_n] ==
                              Chassis_P.CoreSubsys.SelectStaticToeSetting_AxleNums[0]) *
              rtb_Product_ju +
          static_cast<real_T>(Chassis_P.IndependentKandCSuspension_AxleNumVec[ForEach_itr_n] ==
                              Chassis_P.CoreSubsys.SelectStaticToeSetting_AxleNums[1]) *
              rtb_Product_ju;
    }

    /* Gain: '<S31>/Gain5' */
    rtb_ixk = Chassis_P.CoreSubsys.Gain5_Gain_l * rtb_psidot;

    /* Lookup_n-D: '<S72>/X-Y FL' incorporates:
     *  Product: '<S130>/i x k'
     */
    rtb_VectorConcatenate_pe[0] =
        look1_pbinlcapw(rtb_ixk, Chassis_P.CoreSubsys.XYFL_bp01Data_n, Chassis_P.CoreSubsys.XYFL_tableData_n,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_l, m_bumptoe_vs_compress_maxIndex);

    /* Lookup_n-D: '<S72>/X-Y FR' incorporates:
     *  Product: '<S130>/i x k'
     */
    rtb_VectorConcatenate_pe[1] =
        look1_pbinlcapw(rtb_ixk, Chassis_P.CoreSubsys.XYFR_bp01Data_e, Chassis_P.CoreSubsys.XYFR_tableData_h,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_aa, m_bumptoe_vs_compress_maxIndex);

    /* Lookup_n-D: '<S72>/X-Y RL' incorporates:
     *  Product: '<S130>/i x k'
     */
    rtb_VectorConcatenate_pe[2] =
        look1_pbinlcapw(rtb_ixk, Chassis_P.CoreSubsys.XYRL_bp01Data_j, Chassis_P.CoreSubsys.XYRL_tableData_k,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_o, m_bumptoe_vs_compress_maxIndex_rear);

    /* Lookup_n-D: '<S72>/X-Y RR' incorporates:
     *  Product: '<S130>/i x k'
     */
    rtb_VectorConcatenate_pe[3] =
        look1_pbinlcapw(rtb_ixk, Chassis_P.CoreSubsys.XYRR_bp01Data_p, Chassis_P.CoreSubsys.XYRR_tableData_p,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_g, m_bumptoe_vs_compress_maxIndex_rear);

    /* Unit Conversion - from: deg to: rad
       Expression: output = (0.0174533*input) + (0) */
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Selector: '<S71>/Selector10' incorporates:
       *  Constant: '<S71>/Constant12'
       *  Selector: '<S39>/Selector5'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].Selector10 =
          Chassis_P.AlgnTrqSteerCompl[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];

      /* Selector: '<S73>/Selector2' incorporates:
       *  Constant: '<S73>/Constant2'
       *  Selector: '<S39>/Selector5'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].Selector2 =
          Chassis_P.LatSteerCompl[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    }

    /* SignalConversion generated from: '<S20>/Selector3' incorporates:
     *  Constant: '<S20>/Constant'
     *  Inport: '<Root>/DriveLineInput'
     */
    /* Unit Conversion - from: N*m to: kN*m
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: deg to: rad
       Expression: output = (0.0174533*input) + (0) */
    /* Unit Conversion - from: N to: kN
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: deg to: rad
       Expression: output = (0.0174533*input) + (0) */
    /* Unit Conversion - from: N to: kN
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: deg to: rad
       Expression: output = (0.0174533*input) + (0) */
    rtb_Sum_hy[0] = Chassis_P.CoreSubsys.Constant_Value;
    rtb_Sum_hy[1] = Chassis_U.DriveLineInput.StrgAng[0];
    rtb_Sum_hy[2] = Chassis_U.DriveLineInput.StrgAng[1];

    /* Gain: '<S31>/Gain1' incorporates:
     *  ForEachSliceAssignment generated from: '<S220>/Fx'
     *  Gain: '<S26>/Gain1'
     *  Gain: '<S27>/Gain2'
     *  Gain: '<S29>/Gain'
     *  Gain: '<S30>/Gain'
     *  Product: '<S22>/Product1'
     *  Selector: '<S22>/Selector2'
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S41>/Selector5'
     *  Selector: '<S50>/Selector5'
     *  Selector: '<S59>/Selector1'
     *  Selector: '<S63>/Selector3'
     *  Selector: '<S74>/Selector3'
     *  Selector: '<S88>/Selector3'
     */
    Drphi_0 = rtb_ImpAsg_InsertedFor_Fx_at_inport_0[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    Fzo_prime_idx_2 = Drphi_0 * Chassis_P.CoreSubsys.Gain1_Gain;

    /* Switch: '<S74>/Switch' incorporates:
     *  Constant: '<S74>/Constant3'
     *  Constant: '<S74>/Constant6'
     *  Gain: '<S31>/Gain1'
     *  Gain: '<S74>/Gain2'
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S74>/Selector6'
     */
    if (Fzo_prime_idx_2 * Chassis_P.CoreSubsys.Gain2_Gain >= Chassis_P.CoreSubsys.Switch_Threshold) {
      rtb_Add_n =
          Chassis_P.CoreSubsys.Constant6_Value[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    } else {
      rtb_Add_n =
          Chassis_P.CoreSubsys.Constant3_Value[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    }

    /* Gain: '<S31>/Gain4' incorporates:
     *  Concatenate: '<S13>/Matrix Concatenate'
     *  Gain: '<S26>/Gain4'
     *  Selector: '<S38>/Selector'
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S71>/Selector'
     */
    rtb_UnaryMinus_i =
        rtb_VectorConcatenate1_g[(static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1) * 3 + 2];

    /* Gain: '<S31>/Gain3' incorporates:
     *  ForEachSliceAssignment generated from: '<S220>/Fy'
     *  Gain: '<S26>/Gain3'
     *  Gain: '<S29>/Gain3'
     *  Selector: '<S22>/Selector1'
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S40>/Selector3'
     *  Selector: '<S58>/Selector3'
     *  Selector: '<S73>/Selector1'
     *  Selector: '<S87>/Selector3'
     *  UnaryMinus: '<S22>/Unary Minus'
     */
    lam_mux_0 =
        rtb_ImpAsg_InsertedFor_Fy_at_inport_0[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];

    /* Sum: '<S24>/Sum3' incorporates:
     *  ForEachSliceAssignment generated from: '<S220>/Fy'
     *  Gain: '<S31>/Gain'
     *  Gain: '<S31>/Gain3'
     *  Gain: '<S31>/Gain4'
     *  Product: '<S71>/Product9'
     *  Product: '<S73>/Product1'
     *  Product: '<S74>/Product4'
     *  Selector: '<S20>/Selector3'
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S71>/Selector'
     *  Selector: '<S72>/Selector5'
     *  Selector: '<S73>/Selector1'
     *  Sum: '<S31>/Sum'
     *  Sum: '<S31>/Sum1'
     *  Switch: '<S74>/Switch'
     *  UnitConversion: '<S71>/Unit Conversion'
     *  UnitConversion: '<S71>/Unit Conversion1'
     *  UnitConversion: '<S72>/Unit Conversion'
     *  UnitConversion: '<S73>/Unit Conversion'
     *  UnitConversion: '<S73>/Unit Conversion1'
     *  UnitConversion: '<S74>/Unit Conversion'
     *  UnitConversion: '<S74>/Unit Conversion1'
     */
    rtb_Product_ju = ((((rtb_UnaryMinus_i * Chassis_P.CoreSubsys.Gain4_Gain * 0.001 *
                             Chassis_B.CoreSubsys[ForEach_itr_n].Selector10 * 0.017453292519943295 +
                         rtb_VectorConcatenate_pe[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1] *
                             0.017453292519943295) +
                        lam_mux_0 * Chassis_P.CoreSubsys.Gain3_Gain * 0.001 *
                            Chassis_B.CoreSubsys[ForEach_itr_n].Selector2 * 0.017453292519943295) +
                       Fzo_prime_idx_2 * 0.001 * rtb_Add_n * 0.017453292519943295) *
                          Chassis_P.CoreSubsys.Gain_Gain_c +
                      Chassis_B.CoreSubsys[ForEach_itr_n].SumofElements_j) +
                     rtb_Sum_hy[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Product)];
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Selector: '<S38>/Selector8' incorporates:
       *  Constant: '<S38>/Constant11'
       *  Selector: '<S39>/Selector5'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].Selector8 =
          Chassis_P.AlgnTrqCambCompl[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];

      /* Selector: '<S40>/Selector4' incorporates:
       *  Constant: '<S40>/Constant2'
       *  Selector: '<S39>/Selector5'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].Selector4 =
          Chassis_P.LatCambCompl[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    }

    /* Gain: '<S26>/Gain1' */
    /* Unit Conversion - from: N*m to: kN*m
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: deg to: rad
       Expression: output = (0.0174533*input) + (0) */
    /* Unit Conversion - from: N to: kN
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: deg to: rad
       Expression: output = (0.0174533*input) + (0) */
    /* Unit Conversion - from: N to: kN
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: deg to: rad
       Expression: output = (0.0174533*input) + (0) */
    Fzo_prime_idx_2 = Drphi_0 * Chassis_P.CoreSubsys.Gain1_Gain_b;

    /* Switch: '<S41>/Switch' incorporates:
     *  Constant: '<S41>/Constant1'
     *  Constant: '<S41>/Constant6'
     *  Gain: '<S26>/Gain1'
     *  Gain: '<S41>/Gain2'
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S41>/Selector6'
     */
    if (Fzo_prime_idx_2 * Chassis_P.CoreSubsys.Gain2_Gain_j >= Chassis_P.CoreSubsys.Switch_Threshold_j) {
      rtb_Add_n =
          Chassis_P.CoreSubsys.Constant6_Value_j[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    } else {
      rtb_Add_n =
          Chassis_P.CoreSubsys.Constant1_Value_j[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    }

    /* Sum: '<S26>/Sum2' incorporates:
     *  Gain: '<S26>/Gain'
     *  Gain: '<S26>/Gain3'
     *  Gain: '<S26>/Gain4'
     *  Product: '<S38>/Product8'
     *  Product: '<S40>/Product1'
     *  Product: '<S41>/Product4'
     *  Product: '<S43>/Product2'
     *  Switch: '<S41>/Switch'
     *  UnitConversion: '<S38>/Unit Conversion'
     *  UnitConversion: '<S38>/Unit Conversion1'
     *  UnitConversion: '<S39>/Unit Conversion'
     *  UnitConversion: '<S40>/Unit Conversion'
     *  UnitConversion: '<S40>/Unit Conversion1'
     *  UnitConversion: '<S41>/Unit Conversion'
     *  UnitConversion: '<S41>/Unit Conversion1'
     */
    rtb_kxj = ((rtb_UnaryMinus_i * Chassis_P.CoreSubsys.Gain4_Gain_j * 0.001 *
                    Chassis_B.CoreSubsys[ForEach_itr_n].Selector8 * 0.017453292519943295 +
                (Chassis_P.CoreSubsys.Gain_Gain_i * rtb_Product_ju * Chassis_B.CoreSubsys[ForEach_itr_n].SumofElements +
                 (Chassis_B.CoreSubsys[ForEach_itr_n].Selector5 + rtb_xdot_tmp))) +
               lam_mux_0 * Chassis_P.CoreSubsys.Gain3_Gain_i * 0.001 * Chassis_B.CoreSubsys[ForEach_itr_n].Selector4 *
                   0.017453292519943295) +
              Fzo_prime_idx_2 * 0.001 * rtb_Add_n * 0.017453292519943295;
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Selector: '<S27>/Selector5' incorporates:
       *  Constant: '<S27>/Constant7'
       *  Selector: '<S39>/Selector5'
       *  UnitConversion: '<S27>/Unit Conversion'
       */
      /* Unit Conversion - from: deg to: rad
         Expression: output = (0.0174533*input) + (0) */
      Chassis_B.CoreSubsys[ForEach_itr_n].Selector5_j =
          Chassis_P.StatCaster[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1] *
          0.017453292519943295;
    }

    /* Gain: '<S27>/Gain4' */
    rtb_Gain2_e = Chassis_P.CoreSubsys.Gain4_Gain_f * rtb_psidot;

    /* Lookup_n-D: '<S49>/X-Y FL' incorporates:
     *  Sum: '<S27>/Sum1'
     */
    rtb_VectorConcatenate_pe[0] =
        look1_pbinlcapw(rtb_Gain2_e, Chassis_P.CoreSubsys.XYFL_bp01Data_l, Chassis_P.CoreSubsys.XYFL_tableData_n0,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_n, m_bumpcaster_vs_compress_maxIndex);

    /* Lookup_n-D: '<S49>/X-Y FR' incorporates:
     *  Sum: '<S27>/Sum1'
     */
    rtb_VectorConcatenate_pe[1] =
        look1_pbinlcapw(rtb_Gain2_e, Chassis_P.CoreSubsys.XYFR_bp01Data_b, Chassis_P.CoreSubsys.XYFR_tableData_n,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_pu, m_bumpcaster_vs_compress_maxIndex);

    /* Lookup_n-D: '<S49>/X-Y RL' incorporates:
     *  Sum: '<S27>/Sum1'
     */
    rtb_VectorConcatenate_pe[2] =
        look1_pbinlcapw(rtb_Gain2_e, Chassis_P.CoreSubsys.XYRL_bp01Data_d, Chassis_P.CoreSubsys.XYRL_tableData_as,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_gf, m_bumpcaster_vs_compress_maxIndex_rear);

    /* Lookup_n-D: '<S49>/X-Y RR' incorporates:
     *  Sum: '<S27>/Sum1'
     */
    rtb_VectorConcatenate_pe[3] =
        look1_pbinlcapw(rtb_Gain2_e, Chassis_P.CoreSubsys.XYRR_bp01Data_d, Chassis_P.CoreSubsys.XYRR_tableData_pz,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_lb, m_bumpcaster_vs_compress_maxIndex_rear);

    /* Unit Conversion - from: deg to: rad
       Expression: output = (0.0174533*input) + (0) */
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Sum: '<S53>/Sum of Elements' incorporates:
       *  Constant: '<S52>/Constant6'
       *  Constant: '<S53>/Axle Numbers'
       *  Constant: '<S9>/Axle Number'
       *  DataTypeConversion: '<S53>/Data Type Conversion'
       *  ForEachSliceSelector generated from: '<S17>/Axle Number'
       *  Product: '<S53>/Product'
       *  RelationalOperator: '<S53>/Relational Operator'
       *  Selector: '<S39>/Selector5'
       *  Selector: '<S52>/Selector3'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].SumofElements_p =
          Chassis_P.CastVsSteerAng[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1] *
          static_cast<real_T>(Chassis_P.IndependentKandCSuspension_AxleNumVec[ForEach_itr_n] ==
                              Chassis_P.CoreSubsys.SelectSteerCasterSlope_AxleNums);
    }

    /* Gain: '<S27>/Gain2' */
    /* Unit Conversion - from: N to: kN
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: deg to: rad
       Expression: output = (0.0174533*input) + (0) */
    Fzo_prime_idx_2 = Drphi_0 * Chassis_P.CoreSubsys.Gain2_Gain_l;

    /* Switch: '<S50>/Switch' incorporates:
     *  Constant: '<S50>/Constant2'
     *  Constant: '<S50>/Constant4'
     *  Gain: '<S27>/Gain2'
     *  Gain: '<S50>/Gain2'
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S50>/Selector6'
     */
    if (Fzo_prime_idx_2 * Chassis_P.CoreSubsys.Gain2_Gain_o >= Chassis_P.CoreSubsys.Switch_Threshold_o) {
      rtb_Add_n =
          Chassis_P.CoreSubsys.Constant4_Value[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    } else {
      rtb_Add_n =
          Chassis_P.CoreSubsys.Constant2_Value[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    }

    /* Sum: '<S27>/Sum1' incorporates:
     *  Gain: '<S27>/Gain'
     *  Product: '<S50>/Product3'
     *  Product: '<S52>/Product4'
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S49>/Selector5'
     *  Switch: '<S50>/Switch'
     *  UnitConversion: '<S49>/Unit Conversion'
     *  UnitConversion: '<S50>/Unit Conversion'
     *  UnitConversion: '<S50>/Unit Conversion1'
     */
    rtb_Gain2_e =
        ((rtb_VectorConcatenate_pe[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1] *
              0.017453292519943295 +
          Chassis_B.CoreSubsys[ForEach_itr_n].Selector5_j) +
         Chassis_P.CoreSubsys.Gain_Gain_o * rtb_Product_ju * Chassis_B.CoreSubsys[ForEach_itr_n].SumofElements_p) +
        Fzo_prime_idx_2 * 0.001 * rtb_Add_n * 0.017453292519943295;
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Sum: '<S28>/Sum1' incorporates:
       *  Constant: '<S28>/Constant'
       *  Constant: '<S28>/Vehicle Vehicle Wheel Offset3'
       *  Constant: '<S9>/Axle Number'
       *  Constant: '<S9>/Wheel Number'
       *  ForEachSliceSelector generated from: '<S17>/Axle Number'
       *  ForEachSliceSelector generated from: '<S17>/Wheel Number'
       *  Gain: '<S28>/Gain'
       *  Selector: '<S28>/Selector1'
       *  Sum: '<S28>/Sum'
       */
      rtb_UnaryMinus_i =
          (Chassis_P.CoreSubsys.VehicleVehicleWheelOffset3_Value_l
               [static_cast<int32_T>(Chassis_P.IndependentKandCSuspension_AxleNumVec[ForEach_itr_n]) - 1] +
           Chassis_P.CoreSubsys.Constant_Value_dc) *
              Chassis_P.CoreSubsys.Gain_Gain_d -
          Chassis_P.IndependentKandCSuspension_WhlNumVec[ForEach_itr_n];

      /* Signum: '<S28>/Sign' */
      if (std::isnan(rtb_UnaryMinus_i)) {
        /* Signum: '<S28>/Sign' */
        Chassis_B.CoreSubsys[ForEach_itr_n].Sign = (rtNaN);
      } else if (rtb_UnaryMinus_i < 0.0) {
        /* Signum: '<S28>/Sign' */
        Chassis_B.CoreSubsys[ForEach_itr_n].Sign = -1.0;
      } else {
        /* Signum: '<S28>/Sign' */
        Chassis_B.CoreSubsys[ForEach_itr_n].Sign = (rtb_UnaryMinus_i > 0.0);
      }

      /* End of Signum: '<S28>/Sign' */

      /* Selector: '<S90>/Selector2' incorporates:
       *  Constant: '<S90>/Constant5'
       *  Selector: '<S39>/Selector5'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].Selector2_m =
          Chassis_P.NrmlWhlRates[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];

      /* Selector: '<S90>/Selector1' incorporates:
       *  Constant: '<S90>/Constant1'
       *  Selector: '<S39>/Selector5'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].Selector1 =
          Chassis_P.NrmlWhlFrcOff[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    }

    /* Sum: '<S90>/Sum' incorporates:
     *  Gain: '<S77>/Gain2'
     *  Product: '<S90>/Product3'
     *  UnitConversion: '<S90>/Unit Conversion'
     */
    /* Unit Conversion - from: m to: mm
       Expression: output = (1000*input) + (0) */
    rtb_UnaryMinus_i =
        Chassis_P.CoreSubsys.Gain2_Gain_k * rtb_phidot * 1000.0 * Chassis_B.CoreSubsys[ForEach_itr_n].Selector2_m +
        Chassis_B.CoreSubsys[ForEach_itr_n].Selector1;

    /* UnitConversion: '<S89>/Unit Conversion1' incorporates:
     *  Gain: '<S77>/Gain1'
     */
    /* Unit Conversion - from: m to: mm
       Expression: output = (1000*input) + (0) */
    rtb_ixk = Chassis_P.CoreSubsys.Gain1_Gain_h * rtb_phidot * 1000.0;

    /* Lookup_n-D: '<S89>/X-Y FL1' incorporates:
     *  Product: '<S130>/i x k'
     */
    rtb_VectorConcatenate_pe[0] =
        look1_pbinlcapw(rtb_ixk, Chassis_P.CoreSubsys.XYFL1_bp01Data_p, Chassis_P.CoreSubsys.XYFL1_tableData_p,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_gq, 2U);

    /* Lookup_n-D: '<S89>/X-Y FR1' incorporates:
     *  Product: '<S130>/i x k'
     */
    rtb_VectorConcatenate_pe[1] =
        look1_pbinlcapw(rtb_ixk, Chassis_P.CoreSubsys.XYFR1_bp01Data_p, Chassis_P.CoreSubsys.XYFR1_tableData_p,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_gqg, 2U);

    /* Lookup_n-D: '<S89>/X-Y RL1' incorporates:
     *  Product: '<S130>/i x k'
     */
    rtb_VectorConcatenate_pe[2] =
        look1_pbinlcapw(rtb_ixk, Chassis_P.CoreSubsys.XYRL1_bp01Data_p, Chassis_P.CoreSubsys.XYRL1_tableData_p,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_gqgq, 2U);

    /* Lookup_n-D: '<S89>/X-Y RR1' incorporates:
     *  Product: '<S130>/i x k'
     */
    rtb_VectorConcatenate_pe[3] =
        look1_pbinlcapw(rtb_ixk, Chassis_P.CoreSubsys.XYRR1_bp01Data_p, Chassis_P.CoreSubsys.XYRR1_tableData_p,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_gqgqr, 2U);

    /* Abs: '<S89>/Abs' incorporates:
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S89>/Selector1'
     */
    rtb_ixk = std::abs(rtb_VectorConcatenate_pe[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1]);

    /* Selector: '<S19>/Selector' incorporates:
     *  Selector: '<S19>/Selector2'
     *  Selector: '<S39>/Selector5'
     *  Sum: '<S76>/Add2'
     */
    rtb_phidot_tmp = ((static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1) << 1) + 1;

    /* Sum: '<S76>/Add2' incorporates:
     *  Concatenate: '<S9>/Matrix Concatenate2'
     *  Concatenate: '<S9>/Matrix Concatenate3'
     *  Selector: '<S19>/Selector'
     *  Selector: '<S19>/Selector2'
     */
    rtb_jxi = rtb_MatrixConcatenate3[rtb_phidot_tmp] - rtb_MatrixConcatenate2[rtb_phidot_tmp];

    /* Product: '<S89>/Product' incorporates:
     *  UnitConversion: '<S89>/Unit Conversion'
     */
    /* Unit Conversion - from: m/s to: mm/s
       Expression: output = (1000*input) + (0) */
    rtb_Product1_h = 1000.0 * rtb_jxi * rtb_ixk;

    /* Lookup_n-D: '<S89>/X-Y FL' incorporates:
     *  Product: '<S22>/Product1'
     */
    rtb_VectorConcatenate_pe[0] =
        look1_pbinlcapw(rtb_Product1_h, Chassis_P.CoreSubsys.XYFL_bp01Data_p, Chassis_P.CoreSubsys.XYFL_tableData_p,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_gqgqrq, m_shckdamping_vs_comprate_maxIndex);

    /* Lookup_n-D: '<S89>/X-Y FR' incorporates:
     *  Product: '<S22>/Product1'
     */
    rtb_VectorConcatenate_pe[1] =
        look1_pbinlcapw(rtb_Product1_h, Chassis_P.CoreSubsys.XYFR_bp01Data_p, Chassis_P.CoreSubsys.XYFR_tableData_p,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_gqgqrq5, m_shckdamping_vs_comprate_maxIndex);

    /* Lookup_n-D: '<S89>/X-Y RL' incorporates:
     *  Product: '<S22>/Product1'
     */
    rtb_VectorConcatenate_pe[2] = look1_pbinlcapw(
        rtb_Product1_h, Chassis_P.CoreSubsys.XYRL_bp01Data_p1, Chassis_P.CoreSubsys.XYRL_tableData_p,
        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_gqgqrq5i, m_shckdamping_vs_comprate_maxIndex_rear);

    /* Lookup_n-D: '<S89>/X-Y RR' incorporates:
     *  Product: '<S22>/Product1'
     */
    rtb_VectorConcatenate_pe[3] = look1_pbinlcapw(
        rtb_Product1_h, Chassis_P.CoreSubsys.XYRR_bp01Data_p1, Chassis_P.CoreSubsys.XYRR_tableData_p1,
        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_gqgqrq5iw, m_shckdamping_vs_comprate_maxIndex_rear);

    /* Product: '<S89>/Product2' incorporates:
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S89>/Selector5'
     */
    rtb_ixk *= rtb_VectorConcatenate_pe[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];

    /* Sum: '<S77>/Add1' */
    rtb_Product1_h = rtb_UnaryMinus_i + rtb_ixk;

    /* Product: '<S77>/Product1' */
    rtb_jxi *= rtb_Product1_h;

    /* Signum: '<S77>/Sign1' */
    if (std::isnan(rtb_phidot)) {
      Fzo_prime_idx_2 = (rtNaN);
    } else if (rtb_phidot < 0.0) {
      Fzo_prime_idx_2 = -1.0;
    } else {
      Fzo_prime_idx_2 = (rtb_phidot > 0.0);
    }

    /* Product: '<S77>/Product2' incorporates:
     *  Signum: '<S77>/Sign1'
     */
    rtb_Product2_l = Fzo_prime_idx_2 * rtb_phidot * rtb_UnaryMinus_i;

    /* UnitConversion: '<S62>/Unit Conversion1' incorporates:
     *  Gain: '<S30>/Gain4'
     */
    /* Unit Conversion - from: m to: mm
       Expression: output = (1000*input) + (0) */
    rtb_UnaryMinus_i = Chassis_P.CoreSubsys.Gain4_Gain_g * rtb_psidot * 1000.0;

    /* Lookup_n-D: '<S62>/X-Y FL' incorporates:
     *  UnaryMinus: '<S22>/Unary Minus'
     */
    rtb_VectorConcatenate_pe[0] =
        look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYFL_bp01Data_o, Chassis_P.CoreSubsys.XYFL_tableData_o5,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_pi, m_lngdisp_vs_compress_maxIndex);

    /* Lookup_n-D: '<S62>/X-Y FR' incorporates:
     *  UnaryMinus: '<S22>/Unary Minus'
     */
    rtb_VectorConcatenate_pe[1] =
        look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYFR_bp01Data_o, Chassis_P.CoreSubsys.XYFR_tableData_o,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_piw, m_lngdisp_vs_compress_maxIndex);

    /* Lookup_n-D: '<S62>/X-Y RL' incorporates:
     *  UnaryMinus: '<S22>/Unary Minus'
     */
    rtb_VectorConcatenate_pe[2] =
        look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYRL_bp01Data_o, Chassis_P.CoreSubsys.XYRL_tableData_o,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_piwu, m_lngdisp_vs_compress_maxIndex_rear);

    /* Lookup_n-D: '<S62>/X-Y RR' incorporates:
     *  UnaryMinus: '<S22>/Unary Minus'
     */
    rtb_VectorConcatenate_pe[3] =
        look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYRR_bp01Data_o, Chassis_P.CoreSubsys.XYRR_tableData_o,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_piwuj, m_lngdisp_vs_compress_maxIndex_rear);

    /* Gain: '<S30>/Gain' */
    /* Unit Conversion - from: mm to: m
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: N to: kN
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: mm to: m
       Expression: output = (0.001*input) + (0) */
    Fzo_prime_idx_2 = Drphi_0 * Chassis_P.CoreSubsys.Gain_Gain_k;

    /* Switch: '<S63>/Switch' incorporates:
     *  Constant: '<S63>/Constant1'
     *  Constant: '<S63>/Constant2'
     *  Gain: '<S30>/Gain'
     *  Gain: '<S63>/Gain2'
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S63>/Selector4'
     */
    if (Fzo_prime_idx_2 * Chassis_P.CoreSubsys.Gain2_Gain_m >= Chassis_P.CoreSubsys.Switch_Threshold_m) {
      rtb_Add_n =
          Chassis_P.CoreSubsys.Constant2_Value_m[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    } else {
      rtb_Add_n =
          Chassis_P.CoreSubsys.Constant1_Value_m[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    }

    /* UnitConversion: '<S62>/Unit Conversion2' incorporates:
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S62>/Selector5'
     */
    rtb_sig_x_f = rtb_VectorConcatenate_pe[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1] * 0.001;

    /* UnitConversion: '<S57>/Unit Conversion2' incorporates:
     *  Gain: '<S29>/Gain4'
     */
    /* Unit Conversion - from: m to: mm
       Expression: output = (1000*input) + (0) */
    rtb_psidot = Chassis_P.CoreSubsys.Gain4_Gain_e * rtb_psidot * 1000.0;

    /* Lookup_n-D: '<S57>/X-Y FL' incorporates:
     *  Fcn: '<S124>/psidot'
     */
    rtb_VectorConcatenate_pe[0] =
        look1_pbinlcapw(rtb_psidot, Chassis_P.CoreSubsys.XYFL_bp01Data_g3, Chassis_P.CoreSubsys.XYFL_tableData_g,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_p3, m_latdisp_vs_compress_maxIndex);

    /* Lookup_n-D: '<S57>/X-Y FR' incorporates:
     *  Fcn: '<S124>/psidot'
     */
    rtb_VectorConcatenate_pe[1] =
        look1_pbinlcapw(rtb_psidot, Chassis_P.CoreSubsys.XYFR_bp01Data_g3, Chassis_P.CoreSubsys.XYFR_tableData_g,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_p31, m_latdisp_vs_compress_maxIndex);

    /* Lookup_n-D: '<S57>/X-Y RL' incorporates:
     *  Fcn: '<S124>/psidot'
     */
    rtb_VectorConcatenate_pe[2] =
        look1_pbinlcapw(rtb_psidot, Chassis_P.CoreSubsys.XYRL_bp01Data_g, Chassis_P.CoreSubsys.XYRL_tableData_g,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_p31z, m_latdisp_vs_compress_maxIndex_rear);

    /* Lookup_n-D: '<S57>/X-Y RR' incorporates:
     *  Fcn: '<S124>/psidot'
     */
    rtb_VectorConcatenate_pe[3] =
        look1_pbinlcapw(rtb_psidot, Chassis_P.CoreSubsys.XYRR_bp01Data_g, Chassis_P.CoreSubsys.XYRR_tableData_g,
                        &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_p31zl, m_latdisp_vs_compress_maxIndex_rear);

    /* Unit Conversion - from: mm to: m
       Expression: output = (0.001*input) + (0) */
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Selector: '<S58>/Selector4' incorporates:
       *  Constant: '<S58>/Constant2'
       *  Selector: '<S39>/Selector5'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].Selector4_b =
          Chassis_P.LatWhlCtrComplLat[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    }

    /* Gain: '<S29>/Gain' */
    /* Unit Conversion - from: N to: kN
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: mm to: m
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: N to: kN
       Expression: output = (0.001*input) + (0) */
    /* Unit Conversion - from: mm to: m
       Expression: output = (0.001*input) + (0) */
    rtb_psidot = Drphi_0 * Chassis_P.CoreSubsys.Gain_Gain_a;

    /* Switch: '<S59>/Switch' incorporates:
     *  Constant: '<S59>/Constant1'
     *  Constant: '<S59>/Constant3'
     *  Gain: '<S29>/Gain'
     *  Gain: '<S59>/Gain2'
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S59>/Selector2'
     */
    if (rtb_psidot * Chassis_P.CoreSubsys.Gain2_Gain_l2 >= Chassis_P.CoreSubsys.Switch_Threshold_l) {
      rtb_Alpha_b =
          Chassis_P.CoreSubsys.Constant3_Value_l[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    } else {
      rtb_Alpha_b =
          Chassis_P.CoreSubsys.Constant1_Value_l[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    }

    /* UnitConversion: '<S57>/Unit Conversion1' incorporates:
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S57>/Selector5'
     */
    rtb_xdot_o = rtb_VectorConcatenate_pe[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1] * 0.001;

    /* UnitConversion: '<S87>/Unit Conversion1' incorporates:
     *  Gain: '<S77>/Gain3'
     */
    /* Unit Conversion - from: m to: mm
       Expression: output = (1000*input) + (0) */
    rtb_UnaryMinus_i = Chassis_P.CoreSubsys.Gain3_Gain_g * rtb_phidot * 1000.0;

    /* Lookup_n-D: '<S87>/CPz to tanTheta_CPSA_LF' incorporates:
     *  UnaryMinus: '<S22>/Unary Minus'
     */
    rtb_VectorConcatenate_pe[0] = look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.CPztotanTheta_CPSA_LF_bp01Data,
                                                  Chassis_P.CoreSubsys.CPztotanTheta_CPSA_LF_tableData,
                                                  &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_h, 14U);

    /* Lookup_n-D: '<S87>/CPz to tanTheta_CPSA_RF' incorporates:
     *  UnaryMinus: '<S22>/Unary Minus'
     */
    rtb_VectorConcatenate_pe[1] = look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.CPztotanTheta_CPSA_RF_bp01Data,
                                                  Chassis_P.CoreSubsys.CPztotanTheta_CPSA_RF_tableData,
                                                  &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_hc, 14U);

    /* Lookup_n-D: '<S87>/CPz to tanTheta_CPSA_LR' incorporates:
     *  UnaryMinus: '<S22>/Unary Minus'
     */
    rtb_VectorConcatenate_pe[2] = look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.CPztotanTheta_CPSA_LR_bp01Data,
                                                  Chassis_P.CoreSubsys.CPztotanTheta_CPSA_LR_tableData,
                                                  &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_hcw, 14U);

    /* Lookup_n-D: '<S87>/CPz to tanTheta_CPSA_RR' incorporates:
     *  UnaryMinus: '<S22>/Unary Minus'
     */
    rtb_VectorConcatenate_pe[3] = look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.CPztotanTheta_CPSA_RR_bp01Data,
                                                  Chassis_P.CoreSubsys.CPztotanTheta_CPSA_RR_tableData,
                                                  &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_hcwc, 14U);

    /* Selector: '<S87>/Selector5' incorporates:
     *  Selector: '<S39>/Selector5'
     */
    lam_muy_prime_idx_3 = rtb_VectorConcatenate_pe[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1];
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Selector: '<S83>/Selector' incorporates:
       *  Constant: '<S9>/Axle Number'
       *  ForEachSliceSelector generated from: '<S17>/Axle Number'
       *  Selector: '<S78>/Selector1'
       */
      rtb_phidot_tmp = static_cast<int32_T>(Chassis_P.IndependentKandCSuspension_AxleNumVec[ForEach_itr_n]) - 1;

      /* Switch: '<S83>/Switch' incorporates:
       *  Constant: '<S83>/Constant'
       *  Constant: '<S83>/Constant1'
       *  Constant: '<S86>/Constant3'
       *  Constant: '<S86>/Constant4'
       *  DataTypeConversion: '<S83>/Data Type Conversion'
       *  Selector: '<S83>/Selector'
       *  Sum: '<S86>/Sum'
       */
      if (Chassis_P.CoreSubsys.Constant_Value_i[rtb_phidot_tmp] != 0.0) {
        rtb_UnaryMinus_i = Chassis_P.RollStiffArb[0] - Chassis_P.RollStiffNoArb[0];
        lam_muy_prime_idx_0 = Chassis_P.RollStiffArb[1] - Chassis_P.RollStiffNoArb[1];
      } else {
        rtb_UnaryMinus_i = Chassis_P.CoreSubsys.Constant1_Value[0];
        lam_muy_prime_idx_0 = Chassis_P.CoreSubsys.Constant1_Value[1];
      }

      /* End of Switch: '<S83>/Switch' */

      /* ForEachSliceSelector generated from: '<S17>/Axle Number' incorporates:
       *  Constant: '<S9>/Axle Number'
       */
      /* Unit Conversion - from: N*m/deg to: N*m/rad
         Expression: output = (57.2958*input) + (0) */
      rtb_xdot_tmp = Chassis_P.IndependentKandCSuspension_AxleNumVec[ForEach_itr_n];

      /* Sum: '<S84>/Sum of Elements' incorporates:
       *  Constant: '<S84>/Axle Numbers'
       *  DataTypeConversion: '<S84>/Data Type Conversion'
       *  Product: '<S84>/Product'
       *  RelationalOperator: '<S84>/Relational Operator'
       *  UnitConversion: '<S83>/Unit Conversion'
       */
      Chassis_B.CoreSubsys[ForEach_itr_n].SumofElements_o =
          57.295779513082323 * rtb_UnaryMinus_i *
              static_cast<real_T>(rtb_xdot_tmp ==
                                  Chassis_P.CoreSubsys.ConstrainedspringdampercombinationforKC_AxleNums[0]) +
          57.295779513082323 * lam_muy_prime_idx_0 *
              static_cast<real_T>(rtb_xdot_tmp ==
                                  Chassis_P.CoreSubsys.ConstrainedspringdampercombinationforKC_AxleNums[1]);

      /* Sum: '<S78>/Sum1' incorporates:
       *  Constant: '<S78>/Constant'
       *  Constant: '<S78>/Vehicle Vehicle Wheel Offset3'
       *  Constant: '<S9>/Wheel Number'
       *  ForEachSliceSelector generated from: '<S17>/Wheel Number'
       *  Gain: '<S78>/Gain'
       *  Selector: '<S78>/Selector1'
       *  Sum: '<S78>/Sum'
       */
      rtb_UnaryMinus_i = (Chassis_P.CoreSubsys.VehicleVehicleWheelOffset3_Value_d[rtb_phidot_tmp] +
                          Chassis_P.CoreSubsys.Constant_Value_l) *
                             Chassis_P.CoreSubsys.Gain_Gain_m -
                         Chassis_P.IndependentKandCSuspension_WhlNumVec[ForEach_itr_n];

      /* Signum: '<S78>/Sign1' */
      if (std::isnan(rtb_UnaryMinus_i)) {
        /* Signum: '<S78>/Sign1' */
        Chassis_B.CoreSubsys[ForEach_itr_n].Sign1 = (rtNaN);
      } else if (rtb_UnaryMinus_i < 0.0) {
        /* Signum: '<S78>/Sign1' */
        Chassis_B.CoreSubsys[ForEach_itr_n].Sign1 = -1.0;
      } else {
        /* Signum: '<S78>/Sign1' */
        Chassis_B.CoreSubsys[ForEach_itr_n].Sign1 = (rtb_UnaryMinus_i > 0.0);
      }

      /* End of Signum: '<S78>/Sign1' */
    }

    /* UnitConversion: '<S88>/Unit Conversion1' incorporates:
     *  Gain: '<S77>/Gain5'
     */
    /* Unit Conversion - from: m to: mm
       Expression: output = (1000*input) + (0) */
    rtb_UnaryMinus_i = Chassis_P.CoreSubsys.Gain5_Gain_j * rtb_phidot * 1000.0;

    /* Switch: '<S88>/Switch1' */
    if (Drphi_0 >= Chassis_P.CoreSubsys.Switch1_Threshold) {
      /* Lookup_n-D: '<S88>/X-Y RR' incorporates:
       *  UnitConversion: '<S88>/Unit Conversion1'
       */
      rtb_VectorConcatenate_pe[3] =
          look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYRR_bp01Data, Chassis_P.CoreSubsys.XYRR_tableData,
                          &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_her230, 14U);

      /* Lookup_n-D: '<S88>/X-Y RL' incorporates:
       *  UnitConversion: '<S88>/Unit Conversion1'
       */
      rtb_VectorConcatenate_pe[2] =
          look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYRL_bp01Data, Chassis_P.CoreSubsys.XYRL_tableData,
                          &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_her230u, 14U);

      /* Lookup_n-D: '<S88>/X-Y FR' incorporates:
       *  UnitConversion: '<S88>/Unit Conversion1'
       */
      rtb_VectorConcatenate_pe[1] =
          look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYFR_bp01Data, Chassis_P.CoreSubsys.XYFR_tableData,
                          &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_her230um, 14U);

      /* Lookup_n-D: '<S88>/X-Y FL' incorporates:
       *  UnitConversion: '<S88>/Unit Conversion1'
       */
      rtb_VectorConcatenate_pe[0] =
          look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYFL_bp01Data, Chassis_P.CoreSubsys.XYFL_tableData,
                          &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_her230umm, 14U);
    } else {
      /* Lookup_n-D: '<S88>/X-Y RR1' incorporates:
       *  UnitConversion: '<S88>/Unit Conversion1'
       */
      rtb_VectorConcatenate_pe[3] =
          look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYRR1_bp01Data, Chassis_P.CoreSubsys.XYRR1_tableData,
                          &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_he, 14U);

      /* Lookup_n-D: '<S88>/X-Y RL1' incorporates:
       *  UnitConversion: '<S88>/Unit Conversion1'
       */
      rtb_VectorConcatenate_pe[2] =
          look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYRL1_bp01Data, Chassis_P.CoreSubsys.XYRL1_tableData,
                          &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_her, 14U);

      /* Lookup_n-D: '<S88>/X-Y FR1' incorporates:
       *  UnitConversion: '<S88>/Unit Conversion1'
       */
      rtb_VectorConcatenate_pe[1] =
          look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYFR1_bp01Data, Chassis_P.CoreSubsys.XYFR1_tableData,
                          &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_her2, 14U);

      /* Lookup_n-D: '<S88>/X-Y FL1' incorporates:
       *  UnitConversion: '<S88>/Unit Conversion1'
       */
      rtb_VectorConcatenate_pe[0] =
          look1_pbinlcapw(rtb_UnaryMinus_i, Chassis_P.CoreSubsys.XYFL1_bp01Data, Chassis_P.CoreSubsys.XYFL1_tableData,
                          &Chassis_DW.CoreSubsys[ForEach_itr_n].m_bpIndex_her23, 14U);
    }

    /* End of Switch: '<S88>/Switch1' */

    /* Selector: '<S78>/Selector2' incorporates:
     *  Constant: '<S9>/Axle Number'
     *  ForEachSliceSelector generated from: '<S17>/Axle Number'
     *  Selector: '<S25>/Selector1'
     */
    rtb_phidot_tmp = static_cast<int32_T>(Chassis_P.IndependentKandCSuspension_AxleNumVec[ForEach_itr_n]) - 1;

    /* Gain: '<S77>/Gain4' incorporates:
     *  Gain: '<S78>/Gain1'
     *  Gain: '<S88>/Gain'
     *  Product: '<S78>/Divide'
     *  Product: '<S78>/Product'
     *  Product: '<S78>/Product2'
     *  Product: '<S87>/Product1'
     *  Product: '<S88>/Product1'
     *  Selector: '<S39>/Selector5'
     *  Selector: '<S78>/Selector2'
     *  Selector: '<S87>/Selector5'
     *  Selector: '<S88>/Selector5'
     *  Sum: '<S77>/Add'
     */
    rtb_ixk = ((Chassis_B.CoreSubsys[ForEach_itr_n].SumofElements_o * rtb_Add1_b_idx_1 /
                    rtb_VectorConcatenate_b[rtb_phidot_tmp] * Chassis_B.CoreSubsys[ForEach_itr_n].Sign1 *
                    Chassis_P.CoreSubsys.Gain1_Gain_l +
                ((rtb_Product1_h + rtb_ixk) + lam_mux_0 * lam_muy_prime_idx_3)) +
               rtb_VectorConcatenate_pe[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1] * Drphi_0 *
                   Chassis_P.CoreSubsys.Gain_Gain_f) *
              Chassis_P.CoreSubsys.Gain4_Gain_gl;

    /* Sum: '<S22>/Sum' incorporates:
     *  Gain: '<S76>/Sign convention'
     *  Selector: '<S22>/Selector'
     *  Selector: '<S39>/Selector5'
     */
    rtb_Product1_h = Chassis_B.Re[static_cast<int32_T>(Chassis_B.CoreSubsys[ForEach_itr_n].Sum2) - 1] +
                     Chassis_P.CoreSubsys.Signconvention_Gain * rtb_phidot;
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Sum: '<S25>/Sum1' incorporates:
       *  Constant: '<S25>/Constant'
       *  Constant: '<S25>/Vehicle Vehicle Wheel Offset3'
       *  Constant: '<S9>/Wheel Number'
       *  ForEachSliceSelector generated from: '<S17>/Wheel Number'
       *  Gain: '<S25>/Gain'
       *  Selector: '<S25>/Selector1'
       *  Sum: '<S25>/Sum'
       */
      rtb_UnaryMinus_i = Chassis_P.IndependentKandCSuspension_WhlNumVec[ForEach_itr_n] -
                         (Chassis_P.CoreSubsys.VehicleVehicleWheelOffset3_Value_b[rtb_phidot_tmp] +
                          Chassis_P.CoreSubsys.Constant_Value_j) *
                             Chassis_P.CoreSubsys.Gain_Gain_e;

      /* Signum: '<S25>/Sign' */
      if (std::isnan(rtb_UnaryMinus_i)) {
        /* Signum: '<S25>/Sign' */
        Chassis_B.CoreSubsys[ForEach_itr_n].Sign_c = (rtNaN);
      } else if (rtb_UnaryMinus_i < 0.0) {
        /* Signum: '<S25>/Sign' */
        Chassis_B.CoreSubsys[ForEach_itr_n].Sign_c = -1.0;
      } else {
        /* Signum: '<S25>/Sign' */
        Chassis_B.CoreSubsys[ForEach_itr_n].Sign_c = (rtb_UnaryMinus_i > 0.0);
      }

      /* End of Signum: '<S25>/Sign' */
    }

    /* ForEachSliceAssignment generated from: '<S17>/WhlFz' */
    rtb_ImpAsg_InsertedFor_WhlFz_at_inport_0[ForEach_itr_n] = rtb_ixk;

    /* ForEachSliceAssignment generated from: '<S17>/WhlAng' incorporates:
     *  Product: '<S25>/Product'
     *  SignalConversion generated from: '<S17>/WhlAng'
     */
    rtb_ImpAsg_InsertedFor_WhlAng_at_inport_0[ForEach_itr_n * 3] = rtb_kxj * Chassis_B.CoreSubsys[ForEach_itr_n].Sign_c;
    rtb_ImpAsg_InsertedFor_WhlAng_at_inport_0[ForEach_itr_n * 3 + 1] = rtb_Gain2_e;
    rtb_ImpAsg_InsertedFor_WhlAng_at_inport_0[ForEach_itr_n * 3 + 2] = rtb_Product_ju;

    /* ForEachSliceAssignment generated from: '<S17>/VehM' incorporates:
     *  Constant: '<S22>/Constant'
     *  Product: '<S22>/Product'
     *  Product: '<S22>/Product1'
     *  UnaryMinus: '<S22>/Unary Minus'
     */
    rtb_phidot_tmp = ForEach_itr_n * 3;
    rtb_ImpAsg_InsertedFor_VehM_at_inport_0[rtb_phidot_tmp] = -(lam_mux_0 * rtb_Product1_h);
    rtb_ImpAsg_InsertedFor_VehM_at_inport_0[rtb_phidot_tmp + 1] = Drphi_0 * rtb_Product1_h;
    rtb_ImpAsg_InsertedFor_VehM_at_inport_0[rtb_phidot_tmp + 2] = Chassis_P.CoreSubsys.Constant_Value_h;

    /* ForEachSliceAssignment generated from: '<S17>/VehFz' incorporates:
     *  Gain: '<S76>/Vehicle Force Sign'
     */
    rtb_ImpAsg_InsertedFor_VehFz_at_inport_0[ForEach_itr_n] = Chassis_P.CoreSubsys.VehicleForceSign_Gain * rtb_ixk;

    /* ForEachSliceAssignment generated from: '<S17>/Info' incorporates:
     *  Gain: '<S29>/Gain1'
     *  Gain: '<S29>/Gain3'
     *  Gain: '<S30>/Gain1'
     *  Product: '<S28>/Product'
     *  Product: '<S58>/Product1'
     *  Product: '<S59>/Product2'
     *  Product: '<S63>/Product1'
     *  SignalConversion generated from: '<S17>/Info'
     *  Sum: '<S29>/Add'
     *  Sum: '<S30>/Add'
     *  Switch: '<S59>/Switch'
     *  Switch: '<S63>/Switch'
     *  UnitConversion: '<S57>/Unit Conversion1'
     *  UnitConversion: '<S58>/Unit Conversion'
     *  UnitConversion: '<S58>/Unit Conversion1'
     *  UnitConversion: '<S59>/Unit Conversion'
     *  UnitConversion: '<S59>/Unit Conversion1'
     *  UnitConversion: '<S62>/Unit Conversion2'
     *  UnitConversion: '<S63>/Unit Conversion'
     *  UnitConversion: '<S63>/Unit Conversion1'
     */
    rtb_phidot_tmp = ForEach_itr_n << 3;
    rtb_ImpAsg_InsertedFor_Info_at_inport_0[rtb_phidot_tmp] = rtb_kxj;
    rtb_ImpAsg_InsertedFor_Info_at_inport_0[rtb_phidot_tmp + 1] = rtb_Gain2_e;
    rtb_ImpAsg_InsertedFor_Info_at_inport_0[rtb_phidot_tmp + 2] =
        rtb_Product_ju * Chassis_B.CoreSubsys[ForEach_itr_n].Sign;
    rtb_ImpAsg_InsertedFor_Info_at_inport_0[rtb_phidot_tmp + 3] = rtb_thetadot;
    rtb_ImpAsg_InsertedFor_Info_at_inport_0[rtb_phidot_tmp + 4] = rtb_jxi;
    rtb_ImpAsg_InsertedFor_Info_at_inport_0[rtb_phidot_tmp + 5] = rtb_Product2_l;
    rtb_ImpAsg_InsertedFor_Info_at_inport_0[rtb_phidot_tmp + 6] =
        (Fzo_prime_idx_2 * 0.001 * rtb_Add_n * 0.001 + rtb_sig_x_f) * Chassis_P.CoreSubsys.Gain1_Gain_c;
    rtb_ImpAsg_InsertedFor_Info_at_inport_0[rtb_phidot_tmp + 7] =
        ((lam_mux_0 * Chassis_P.CoreSubsys.Gain3_Gain_a * 0.001 * Chassis_B.CoreSubsys[ForEach_itr_n].Selector4_b *
              0.001 +
          rtb_xdot_o) +
         rtb_psidot * 0.001 * rtb_Alpha_b * 0.001) *
        Chassis_P.CoreSubsys.Gain1_Gain_k;
  }

  /* End of Outputs for SubSystem: '<S9>/For each track and axle combination calculate suspension forces and moments' */

  /* Concatenate: '<S9>/Matrix Concatenate' incorporates:
   *  Concatenate: '<S9>/Matrix Concatenate4'
   *  ForEachSliceAssignment generated from: '<S17>/VehFz'
   */
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 4; rtb_phidot_tmp++) {
    Itemp_tmp = rtb_phidot_tmp << 1;
    rtb_VectorConcatenate1_g[3 * rtb_phidot_tmp] = rtb_MatrixConcatenate4[Itemp_tmp];
    rtb_VectorConcatenate1_g[3 * rtb_phidot_tmp + 1] = rtb_MatrixConcatenate4[Itemp_tmp + 1];
    rtb_VectorConcatenate1_g[3 * rtb_phidot_tmp + 2] = rtb_ImpAsg_InsertedFor_VehFz_at_inport_0[rtb_phidot_tmp];
  }

  /* End of Concatenate: '<S9>/Matrix Concatenate' */

  /* Outputs for Iterator SubSystem: '<S93>/Susp2Body' incorporates:
   *  ForEach: '<S97>/For Each'
   */
  for (ForEach_itr_l = 0; ForEach_itr_l < 4; ForEach_itr_l++) {
    /* Trigonometry: '<S98>/sincos' incorporates:
     *  UnaryMinus: '<S97>/Unary Minus'
     */
    rtb_Product_fe[1] = std::cos(-rtb_angles_idx_1);
    rtb_Add1_b_idx_1 = std::sin(-rtb_angles_idx_1);
    rtb_Product_fe[2] = std::cos(-rtb_angles_idx_0);
    Fzo_prime_idx_2 = std::sin(-rtb_angles_idx_0);

    /* Fcn: '<S98>/Fcn11' */
    rtb_VectorConcatenate_i[0] = rtb_Add_f_tmp * rtb_Product_fe[1];

    /* Fcn: '<S98>/Fcn21' incorporates:
     *  Fcn: '<S98>/Fcn22'
     */
    rtb_thetadot = rtb_Add1_b_idx_1 * Fzo_prime_idx_2;
    rtb_VectorConcatenate_i[1] = rtb_thetadot * rtb_Add_f_tmp - rtb_pqr_tmp_0 * rtb_Product_fe[2];

    /* Fcn: '<S98>/Fcn31' incorporates:
     *  Fcn: '<S98>/Fcn32'
     */
    rtb_UnaryMinus_i = rtb_Add1_b_idx_1 * rtb_Product_fe[2];
    rtb_VectorConcatenate_i[2] = rtb_UnaryMinus_i * rtb_Add_f_tmp + rtb_pqr_tmp_0 * Fzo_prime_idx_2;

    /* Fcn: '<S98>/Fcn12' */
    rtb_VectorConcatenate_i[3] = rtb_pqr_tmp_0 * rtb_Product_fe[1];

    /* Fcn: '<S98>/Fcn22' */
    rtb_VectorConcatenate_i[4] = rtb_thetadot * rtb_pqr_tmp_0 + rtb_Add_f_tmp * rtb_Product_fe[2];

    /* Fcn: '<S98>/Fcn32' */
    rtb_VectorConcatenate_i[5] = rtb_UnaryMinus_i * rtb_pqr_tmp_0 - rtb_Add_f_tmp * Fzo_prime_idx_2;

    /* Fcn: '<S98>/Fcn13' */
    rtb_VectorConcatenate_i[6] = -rtb_Add1_b_idx_1;

    /* Fcn: '<S98>/Fcn23' */
    rtb_VectorConcatenate_i[7] = rtb_Product_fe[1] * Fzo_prime_idx_2;

    /* Fcn: '<S98>/Fcn33' */
    rtb_VectorConcatenate_i[8] = rtb_Product_fe[1] * rtb_Product_fe[2];

    /* Math: '<S97>/Transpose' incorporates:
     *  Concatenate: '<S101>/Vector Concatenate'
     */
    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
      rtb_Transpose_n[3 * rtb_phidot_tmp] = rtb_VectorConcatenate_i[rtb_phidot_tmp];
      rtb_Transpose_n[3 * rtb_phidot_tmp + 1] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 3];
      rtb_Transpose_n[3 * rtb_phidot_tmp + 2] = rtb_VectorConcatenate_i[rtb_phidot_tmp + 6];
    }

    /* End of Math: '<S97>/Transpose' */
    for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= 0; rtb_phidot_tmp += 2) {
      /* Product: '<S100>/Product' incorporates:
       *  ForEachSliceAssignment generated from: '<S17>/VehM'
       *  ForEachSliceSelector generated from: '<S97>/Ms'
       *  Math: '<S97>/Transpose'
       */
      tmp_5 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp]);
      tmp_6 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp + 3]);
      tmp_3 = _mm_loadu_pd(&rtb_Transpose_n[rtb_phidot_tmp + 6]);
      _mm_storeu_pd(
          &rtb_Product_fe[rtb_phidot_tmp],
          _mm_add_pd(
              _mm_mul_pd(_mm_set1_pd(rtb_ImpAsg_InsertedFor_VehM_at_inport_0[3 * ForEach_itr_l + 2]), tmp_3),
              _mm_add_pd(
                  _mm_mul_pd(_mm_set1_pd(rtb_ImpAsg_InsertedFor_VehM_at_inport_0[3 * ForEach_itr_l + 1]), tmp_6),
                  _mm_add_pd(_mm_mul_pd(_mm_set1_pd(rtb_ImpAsg_InsertedFor_VehM_at_inport_0[3 * ForEach_itr_l]), tmp_5),
                             _mm_set1_pd(0.0)))));
    }

    /* Product: '<S100>/Product' incorporates:
     *  ForEachSliceAssignment generated from: '<S17>/VehM'
     *  ForEachSliceSelector generated from: '<S97>/Ms'
     *  Math: '<S97>/Transpose'
     */
    for (rtb_phidot_tmp = 2; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
      rtb_Product_fe[rtb_phidot_tmp] =
          (rtb_ImpAsg_InsertedFor_VehM_at_inport_0[3 * ForEach_itr_l + 1] * rtb_Transpose_n[rtb_phidot_tmp + 3] +
           rtb_ImpAsg_InsertedFor_VehM_at_inport_0[3 * ForEach_itr_l] * rtb_Transpose_n[rtb_phidot_tmp]) +
          rtb_ImpAsg_InsertedFor_VehM_at_inport_0[3 * ForEach_itr_l + 2] * rtb_Transpose_n[rtb_phidot_tmp + 6];
    }

    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
      /* ForEachSliceAssignment generated from: '<S97>/MsB' incorporates:
       *  Concatenate: '<S125>/Vector Concatenate'
       *  Product: '<S97>/Inertial to Body1'
       */
      iy = 3 * ForEach_itr_l + rtb_phidot_tmp;
      rtb_VectorConcatenate2[iy] = 0.0;
      rtb_VectorConcatenate2[iy] += rtb_VectorConcatenate[rtb_phidot_tmp] * rtb_Product_fe[0];
      rtb_VectorConcatenate2[iy] += rtb_VectorConcatenate[rtb_phidot_tmp + 3] * rtb_Product_fe[1];
      rtb_VectorConcatenate2[iy] += rtb_VectorConcatenate[rtb_phidot_tmp + 6] * rtb_Product_fe[2];

      /* Product: '<S99>/Product' incorporates:
       *  ForEachSliceSelector generated from: '<S97>/Fs'
       *  Math: '<S97>/Transpose'
       */
      rtb_Sum_hy[rtb_phidot_tmp] =
          (rtb_VectorConcatenate1_g[3 * ForEach_itr_l + 1] * rtb_Transpose_n[rtb_phidot_tmp + 3] +
           rtb_VectorConcatenate1_g[3 * ForEach_itr_l] * rtb_Transpose_n[rtb_phidot_tmp]) +
          rtb_VectorConcatenate1_g[3 * ForEach_itr_l + 2] * rtb_Transpose_n[rtb_phidot_tmp + 6];
    }

    /* ForEachSliceAssignment generated from: '<S97>/FsB' incorporates:
     *  Concatenate: '<S125>/Vector Concatenate'
     *  Product: '<S97>/Inertial to Body'
     *  Product: '<S99>/Product'
     */
    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
      iy = 3 * ForEach_itr_l + rtb_phidot_tmp;
      rtb_ImpAsg_InsertedFor_FsB_at_inport_0[iy] = 0.0;
      rtb_ImpAsg_InsertedFor_FsB_at_inport_0[iy] += rtb_VectorConcatenate[rtb_phidot_tmp] * rtb_Sum_hy[0];
      rtb_ImpAsg_InsertedFor_FsB_at_inport_0[iy] += rtb_VectorConcatenate[rtb_phidot_tmp + 3] * rtb_Sum_hy[1];
      rtb_ImpAsg_InsertedFor_FsB_at_inport_0[iy] += rtb_VectorConcatenate[rtb_phidot_tmp + 6] * rtb_Sum_hy[2];
    }

    /* End of ForEachSliceAssignment generated from: '<S97>/FsB' */
  }

  /* End of Outputs for SubSystem: '<S93>/Susp2Body' */

  /* Sum: '<S102>/Sum' incorporates:
   *  ForEachSliceAssignment generated from: '<S97>/FsB'
   *  Selector: '<S107>/Selector1'
   *  Sum: '<S107>/Sum of Elements'
   */
  rtb_Add1_b_idx_1 = (((rtb_ImpAsg_InsertedFor_FsB_at_inport_0[0] + rtb_ImpAsg_InsertedFor_FsB_at_inport_0[3]) +
                       rtb_ImpAsg_InsertedFor_FsB_at_inport_0[6]) +
                      rtb_ImpAsg_InsertedFor_FsB_at_inport_0[9]) +
                     ((rtb_UnaryMinus_h_idx_0 + rtb_Fg_B[0]) - rtb_Product[0]);

  /* Sum: '<S107>/Sum of Elements1' incorporates:
   *  ForEachSliceAssignment generated from: '<S97>/FsB'
   *  Selector: '<S107>/Selector2'
   *  Sum: '<S104>/Sum of Elements10'
   */
  rtb_angles_idx_0 = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[1] + rtb_ImpAsg_InsertedFor_FsB_at_inport_0[4];

  /* Sum: '<S102>/Sum' incorporates:
   *  ForEachSliceAssignment generated from: '<S97>/FsB'
   *  Selector: '<S107>/Selector2'
   *  Sum: '<S107>/Sum of Elements1'
   */
  Fzo_prime_idx_2 =
      ((rtb_angles_idx_0 + rtb_ImpAsg_InsertedFor_FsB_at_inport_0[7]) + rtb_ImpAsg_InsertedFor_FsB_at_inport_0[10]) +
      ((rtb_UnaryMinus_h_idx_1 + rtb_Fg_B[1]) - rtb_Product[1]);

  /* Sum: '<S107>/Sum of Elements2' incorporates:
   *  ForEachSliceAssignment generated from: '<S97>/FsB'
   *  Selector: '<S107>/Selector3'
   *  Sum: '<S104>/Sum of Elements2'
   */
  rtb_angles_idx_1 = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[2] + rtb_ImpAsg_InsertedFor_FsB_at_inport_0[5];

  /* Sum: '<S102>/Sum' incorporates:
   *  ForEachSliceAssignment generated from: '<S97>/FsB'
   *  Selector: '<S107>/Selector3'
   *  Sum: '<S107>/Sum of Elements2'
   */
  rtb_ixk =
      ((rtb_angles_idx_1 + rtb_ImpAsg_InsertedFor_FsB_at_inport_0[8]) + rtb_ImpAsg_InsertedFor_FsB_at_inport_0[11]) +
      ((rtb_UnaryMinus_h_idx_2 + rtb_Fg_B[2]) - rtb_Product[2]);
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Sum: '<S131>/Sum of Elements' */
    Chassis_B.SumofElements[0] = 0.0;
    Chassis_B.SumofElements[1] = 0.0;
    Chassis_B.SumofElements[2] = 0.0;

    /* Sum: '<S149>/Mx' incorporates:
     *  Product: '<S149>/Product4'
     *  Product: '<S149>/Product5'
     */
    Chassis_B.Mx = rtb_Product1_l * 0.0 - rtb_Product_c * 0.0;
  }

  /* Product: '<S133>/j x k' incorporates:
   *  Product: '<S93>/Product'
   */
  rtb_pqr_tmp_0 = Chassis_B.ubvbwb[1] * rtb_pqr[2];

  /* Sum: '<S109>/Sum' incorporates:
   *  Product: '<S109>/Product'
   *  Product: '<S133>/j x k'
   *  Product: '<S134>/k x j'
   *  Sum: '<S117>/Sum'
   *  Sum: '<S118>/Sum'
   */
  Chassis_B.Sum[0] = (rtb_Add1_b_idx_1 + Chassis_B.SumofElements[0]) / Chassis_B.Mbar +
                     (rtb_pqr_tmp_0 - rtb_pqr[1] * Chassis_B.ubvbwb[2]);

  /* Product: '<S134>/i x k' incorporates:
   *  Product: '<S93>/Product1'
   */
  rtb_Add_f_tmp = Chassis_B.ubvbwb[0] * rtb_pqr[2];

  /* Sum: '<S109>/Sum' incorporates:
   *  Product: '<S109>/Product'
   *  Product: '<S133>/i x j'
   *  Product: '<S133>/k x i'
   *  Product: '<S134>/i x k'
   *  Product: '<S134>/j x i'
   *  Sum: '<S117>/Sum'
   *  Sum: '<S118>/Sum'
   */
  Chassis_B.Sum[1] = (Fzo_prime_idx_2 + Chassis_B.SumofElements[1]) / Chassis_B.Mbar +
                     (rtb_pqr[0] * Chassis_B.ubvbwb[2] - rtb_Add_f_tmp);
  Chassis_B.Sum[2] = (rtb_ixk + Chassis_B.SumofElements[2]) / Chassis_B.Mbar +
                     (Chassis_B.ubvbwb[0] * rtb_pqr[1] - rtb_pqr[0] * Chassis_B.ubvbwb[1]);

  /* Sum: '<S104>/Sum of Elements6' incorporates:
   *  ForEachSliceAssignment generated from: '<S97>/FsB'
   *  Product: '<S104>/Product1'
   *  Product: '<S104>/Product2'
   *  Selector: '<S107>/Selector2'
   *  Selector: '<S107>/Selector3'
   *  Sum: '<S104>/Sum of Elements1'
   *  Sum: '<S104>/Sum of Elements15'
   */
  /* Unit Conversion - from: m/s^2 to: gn
     Expression: output = (0.101972*input) + (0) */
  rtb_Add_ms[0] = (Chassis_B.Mx - (((Chassis_B.Wbar[0] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[2] -
                                     Chassis_B.Wbar[1] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[5]) +
                                    Chassis_B.Wbar[2] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[8]) -
                                   Chassis_B.Wbar[3] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[11])) -
                  (((rtb_ImpAsg_InsertedFor_FsB_at_inport_0[1] * Chassis_B.Xbar[2] +
                     Chassis_B.Xbar[2] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[4]) +
                    Chassis_B.Xbar[2] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[7]) +
                   Chassis_B.Xbar[2] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[10]);
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* Sum: '<S149>/My' incorporates:
     *  Product: '<S149>/Product'
     *  Product: '<S149>/Product3'
     */
    Chassis_B.My = rtb_Product_c * 0.0 + 0.0 * rtb_Product2_c;

    /* Sum: '<S149>/Mz' incorporates:
     *  Product: '<S149>/Product1'
     *  Product: '<S149>/Product2'
     */
    Chassis_B.Mz = (0.0 - 0.0 * rtb_Product1_l) - 0.0 * rtb_Product2_c;
  }

  /* Sum: '<S104>/Sum of Elements9' incorporates:
   *  ForEachSliceAssignment generated from: '<S97>/FsB'
   *  Product: '<S104>/Product'
   *  Product: '<S104>/Product4'
   *  Product: '<S104>/Product5'
   *  Selector: '<S107>/Selector1'
   *  Selector: '<S107>/Selector3'
   *  Sum: '<S104>/Sum of Elements16'
   *  Sum: '<S104>/Sum of Elements3'
   *  Sum: '<S104>/Sum of Elements4'
   */
  rtb_Add_ms[1] =
      ((((rtb_ImpAsg_InsertedFor_FsB_at_inport_0[0] * Chassis_B.Xbar[2] +
          Chassis_B.Xbar[2] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[3]) +
         Chassis_B.Xbar[2] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[6]) +
        Chassis_B.Xbar[2] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[9]) +
       ((rtb_ImpAsg_InsertedFor_FsB_at_inport_0[8] + rtb_ImpAsg_InsertedFor_FsB_at_inport_0[11]) * Chassis_B.Xbar[1] -
        rtb_angles_idx_1 * Chassis_B.Xbar[0])) +
      Chassis_B.My;

  /* Sum: '<S104>/Sum of Elements5' incorporates:
   *  ForEachSliceAssignment generated from: '<S97>/FsB'
   *  Product: '<S104>/Product3'
   *  Product: '<S104>/Product6'
   *  Product: '<S104>/Product7'
   *  Selector: '<S107>/Selector1'
   *  Selector: '<S107>/Selector2'
   *  Sum: '<S104>/Sum of Elements11'
   *  Sum: '<S104>/Sum of Elements7'
   *  Sum: '<S104>/Sum of Elements8'
   */
  rtb_Add_ms[2] =
      ((((rtb_ImpAsg_InsertedFor_FsB_at_inport_0[0] * Chassis_B.Wbar[0] -
          Chassis_B.Wbar[1] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[3]) +
         Chassis_B.Wbar[2] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[6]) -
        Chassis_B.Wbar[3] * rtb_ImpAsg_InsertedFor_FsB_at_inport_0[9]) +
       (rtb_angles_idx_0 * Chassis_B.Xbar[0] -
        (rtb_ImpAsg_InsertedFor_FsB_at_inport_0[7] + rtb_ImpAsg_InsertedFor_FsB_at_inport_0[10]) * Chassis_B.Xbar[1])) +
      Chassis_B.Mz;

  /* Product: '<S136>/Product4' incorporates:
   *  Constant: '<S136>/Constant3'
   */
  rtb_UnaryMinus_i = Chassis_P.VEH.FrontAxlePositionfromCG + Chassis_P.VEH.RearAxlePositionfromCG;
  rtb_Product2_c = rtb_Abs1[3] * rtb_UnaryMinus_i;

  /* Sum: '<S102>/Add' incorporates:
   *  Constant: '<S92>/Constant1'
   *  ForEachSliceAssignment generated from: '<S97>/MsB'
   *  Selector: '<S114>/Selector1'
   *  Sum: '<S114>/Sum of Elements'
   */
  rtb_Product1_l = ((((rtb_VectorConcatenate2[0] + rtb_VectorConcatenate2[3]) + rtb_VectorConcatenate2[6]) +
                     rtb_VectorConcatenate2[9]) +
                    (rtb_Add_ms[0] + Chassis_P.Constant1_Value[0])) -
                   rtb_Product2_c;

  /* Product: '<S136>/Product4' */
  rtb_UnaryMinus_b[0] = rtb_Product2_c;

  /* Sum: '<S102>/Add' */
  rtb_Add_ms[0] = rtb_Product1_l;

  /* Product: '<S136>/Product4' incorporates:
   *  Constant: '<S136>/Constant3'
   */
  rtb_Product2_c = rtb_Abs1[4] * rtb_UnaryMinus_i;

  /* Sum: '<S102>/Add' incorporates:
   *  Constant: '<S92>/Constant1'
   *  ForEachSliceAssignment generated from: '<S97>/MsB'
   *  Selector: '<S114>/Selector2'
   *  Sum: '<S114>/Sum of Elements1'
   */
  rtb_Product_c = ((((rtb_VectorConcatenate2[1] + rtb_VectorConcatenate2[4]) + rtb_VectorConcatenate2[7]) +
                    rtb_VectorConcatenate2[10]) +
                   (rtb_Add_ms[1] + Chassis_P.Constant1_Value[1])) -
                  rtb_Product2_c;

  /* Product: '<S136>/Product4' */
  rtb_UnaryMinus_b[1] = rtb_Product2_c;

  /* Sum: '<S102>/Add' */
  rtb_Add_ms[1] = rtb_Product_c;

  /* Product: '<S136>/Product4' incorporates:
   *  Constant: '<S136>/Constant3'
   */
  rtb_Product2_c = rtb_Abs1[5] * rtb_UnaryMinus_i;

  /* Sum: '<S102>/Add' incorporates:
   *  Constant: '<S92>/Constant1'
   *  ForEachSliceAssignment generated from: '<S97>/MsB'
   *  Selector: '<S114>/Selector3'
   *  Sum: '<S114>/Sum of Elements2'
   */
  rtb_angles_idx_0 = ((((rtb_VectorConcatenate2[2] + rtb_VectorConcatenate2[5]) + rtb_VectorConcatenate2[8]) +
                       rtb_VectorConcatenate2[11]) +
                      (rtb_Add_ms[2] + Chassis_P.Constant1_Value[2])) -
                     rtb_Product2_c;
  rtb_Add_ms[2] = rtb_angles_idx_0;
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
      /* Concatenate: '<S117>/Matrix Concatenation' incorporates:
       *  Constant: '<S102>/Constant1'
       *  Selector: '<S116>/Selector1'
       */
      rtb_MatrixConcatenation[6 * rtb_phidot_tmp] = rtb_Ibar[3 * rtb_phidot_tmp];
      Itemp_tmp = 6 * rtb_phidot_tmp + 3;
      rtb_MatrixConcatenation[Itemp_tmp] = Chassis_P.Constant1_Value_l[3 * rtb_phidot_tmp];

      /* Selector: '<S116>/Selector1' incorporates:
       *  Concatenate: '<S117>/Matrix Concatenation'
       */
      Chassis_B.Selector1_b[3 * rtb_phidot_tmp] = rtb_MatrixConcatenation[Itemp_tmp];

      /* Selector: '<S116>/Selector' incorporates:
       *  Concatenate: '<S117>/Matrix Concatenation'
       */
      Chassis_B.Selector[3 * rtb_phidot_tmp] = rtb_MatrixConcatenation[6 * rtb_phidot_tmp];

      /* Concatenate: '<S117>/Matrix Concatenation' incorporates:
       *  Constant: '<S102>/Constant1'
       *  Selector: '<S116>/Selector'
       *  Selector: '<S116>/Selector1'
       */
      Itemp_tmp = 3 * rtb_phidot_tmp + 1;
      iy = 6 * rtb_phidot_tmp + 1;
      rtb_MatrixConcatenation[iy] = rtb_Ibar[Itemp_tmp];
      ibmat = 6 * rtb_phidot_tmp + 4;
      rtb_MatrixConcatenation[ibmat] = Chassis_P.Constant1_Value_l[Itemp_tmp];

      /* Selector: '<S116>/Selector1' incorporates:
       *  Concatenate: '<S117>/Matrix Concatenation'
       */
      Chassis_B.Selector1_b[Itemp_tmp] = rtb_MatrixConcatenation[ibmat];

      /* Selector: '<S116>/Selector' incorporates:
       *  Concatenate: '<S117>/Matrix Concatenation'
       */
      Chassis_B.Selector[Itemp_tmp] = rtb_MatrixConcatenation[iy];

      /* Concatenate: '<S117>/Matrix Concatenation' incorporates:
       *  Constant: '<S102>/Constant1'
       *  Selector: '<S116>/Selector'
       *  Selector: '<S116>/Selector1'
       */
      Itemp_tmp = 3 * rtb_phidot_tmp + 2;
      iy = 6 * rtb_phidot_tmp + 2;
      rtb_MatrixConcatenation[iy] = rtb_Ibar[Itemp_tmp];
      ibmat = 6 * rtb_phidot_tmp + 5;
      rtb_MatrixConcatenation[ibmat] = Chassis_P.Constant1_Value_l[Itemp_tmp];

      /* Selector: '<S116>/Selector1' incorporates:
       *  Concatenate: '<S117>/Matrix Concatenation'
       */
      Chassis_B.Selector1_b[Itemp_tmp] = rtb_MatrixConcatenation[ibmat];

      /* Selector: '<S116>/Selector' incorporates:
       *  Concatenate: '<S117>/Matrix Concatenation'
       */
      Chassis_B.Selector[Itemp_tmp] = rtb_MatrixConcatenation[iy];
    }
  }

  for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= 0; rtb_phidot_tmp += 2) {
    /* Product: '<S127>/Product' incorporates:
     *  Integrator: '<S109>/p,q,r '
     *  Selector: '<S116>/Selector'
     *  Sum: '<S126>/Sum'
     */
    tmp_5 = _mm_loadu_pd(&Chassis_B.Selector[rtb_phidot_tmp]);
    tmp_6 = _mm_loadu_pd(&Chassis_B.Selector[rtb_phidot_tmp + 3]);
    tmp_3 = _mm_loadu_pd(&Chassis_B.Selector[rtb_phidot_tmp + 6]);
    _mm_storeu_pd(&rtb_Sum_hy[rtb_phidot_tmp],
                  _mm_add_pd(_mm_mul_pd(tmp_3, _mm_set1_pd(rtb_pqr[2])),
                             _mm_add_pd(_mm_mul_pd(tmp_6, _mm_set1_pd(rtb_pqr[1])),
                                        _mm_add_pd(_mm_mul_pd(tmp_5, _mm_set1_pd(rtb_pqr[0])), _mm_set1_pd(0.0)))));
  }

  /* Product: '<S127>/Product' incorporates:
   *  Integrator: '<S109>/p,q,r '
   *  Selector: '<S116>/Selector'
   *  Sum: '<S126>/Sum'
   */
  for (rtb_phidot_tmp = 2; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    rtb_Sum_hy[rtb_phidot_tmp] =
        (Chassis_B.Selector[rtb_phidot_tmp + 3] * rtb_pqr[1] + Chassis_B.Selector[rtb_phidot_tmp] * rtb_pqr[0]) +
        Chassis_B.Selector[rtb_phidot_tmp + 6] * rtb_pqr[2];
  }

  if (rtmIsMajorTimeStep((&Chassis_M))) {
    for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
      /* Selector: '<S116>/Selector2' incorporates:
       *  Concatenate: '<S117>/Matrix Concatenation'
       */
      Chassis_B.Selector2[3 * rtb_phidot_tmp] = rtb_MatrixConcatenation[6 * rtb_phidot_tmp];
      Chassis_B.Selector2[3 * rtb_phidot_tmp + 1] = rtb_MatrixConcatenation[6 * rtb_phidot_tmp + 1];
      Chassis_B.Selector2[3 * rtb_phidot_tmp + 2] = rtb_MatrixConcatenation[6 * rtb_phidot_tmp + 2];
    }
  }

  /* Sum: '<S126>/Sum' incorporates:
   *  Product: '<S129>/i x j'
   *  Product: '<S129>/j x k'
   *  Product: '<S129>/k x i'
   *  Product: '<S130>/i x k'
   *  Product: '<S130>/j x i'
   *  Product: '<S130>/k x j'
   */
  rtb_Product_fe[0] = rtb_pqr[1] * rtb_Sum_hy[2];
  rtb_Product_fe[1] = rtb_Sum_hy[0] * rtb_pqr[2];
  rtb_Product_fe[2] = rtb_pqr[0] * rtb_Sum_hy[1];
  rtb_sincos_o2_a[0] = rtb_Sum_hy[1] * rtb_pqr[2];
  rtb_sincos_o2_a[1] = rtb_pqr[0] * rtb_Sum_hy[2];
  rtb_sincos_o2_a[2] = rtb_Sum_hy[0] * rtb_pqr[1];
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp <= 0; rtb_phidot_tmp += 2) {
    __m128d tmp_1;

    /* Product: '<S128>/Product' incorporates:
     *  Selector: '<S116>/Selector1'
     */
    tmp_5 = _mm_loadu_pd(&Chassis_B.Selector1_b[rtb_phidot_tmp]);
    tmp_6 = _mm_loadu_pd(&Chassis_B.Selector1_b[rtb_phidot_tmp + 3]);
    tmp_3 = _mm_loadu_pd(&Chassis_B.Selector1_b[rtb_phidot_tmp + 6]);

    /* Sum: '<S116>/Sum2' incorporates:
     *  Product: '<S128>/Product'
     */
    tmp_4 = _mm_loadu_pd(&rtb_Add_ms[rtb_phidot_tmp]);

    /* Sum: '<S126>/Sum' incorporates:
     *  Product: '<S128>/Product'
     */
    tmp_2 = _mm_loadu_pd(&rtb_Product_fe[rtb_phidot_tmp]);
    tmp_1 = _mm_loadu_pd(&rtb_sincos_o2_a[rtb_phidot_tmp]);

    /* Sum: '<S116>/Sum2' incorporates:
     *  Integrator: '<S109>/p,q,r '
     *  Product: '<S128>/Product'
     */
    _mm_storeu_pd(
        &rtb_Sum_hy[rtb_phidot_tmp],
        _mm_sub_pd(_mm_sub_pd(tmp_4, _mm_add_pd(_mm_mul_pd(tmp_3, _mm_set1_pd(rtb_pqr[2])),
                                                _mm_add_pd(_mm_mul_pd(tmp_6, _mm_set1_pd(rtb_pqr[1])),
                                                           _mm_add_pd(_mm_mul_pd(tmp_5, _mm_set1_pd(rtb_pqr[0])),
                                                                      _mm_set1_pd(0.0))))),
                   _mm_sub_pd(tmp_2, tmp_1)));
  }

  for (rtb_phidot_tmp = 2; rtb_phidot_tmp < 3; rtb_phidot_tmp++) {
    /* Sum: '<S116>/Sum2' incorporates:
     *  Integrator: '<S109>/p,q,r '
     *  Product: '<S128>/Product'
     *  Selector: '<S116>/Selector1'
     *  Sum: '<S126>/Sum'
     */
    rtb_Sum_hy[rtb_phidot_tmp] =
        (rtb_Add_ms[rtb_phidot_tmp] - ((Chassis_B.Selector1_b[rtb_phidot_tmp + 3] * rtb_pqr[1] +
                                        Chassis_B.Selector1_b[rtb_phidot_tmp] * rtb_pqr[0]) +
                                       Chassis_B.Selector1_b[rtb_phidot_tmp + 6] * rtb_pqr[2])) -
        (rtb_Product_fe[rtb_phidot_tmp] - rtb_sincos_o2_a[rtb_phidot_tmp]);
  }

  /* Product: '<S116>/Product2' incorporates:
   *  Selector: '<S116>/Selector2'
   */
  rt_mrdivide_U1d1x3_U2d3x3_Yd1x3_snf(rtb_Sum_hy, Chassis_B.Selector2, Chassis_B.Product2);
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    /* BusCreator generated from: '<Root>/VehBody' */
    Chassis_B.Hitch_n.Fx = 0.0;
    Chassis_B.Hitch_n.Fy = 0.0;
    Chassis_B.Hitch_n.Fz = 0.0;

    /* BusCreator generated from: '<Root>/VehBody' incorporates:
     *  Constant: '<S94>/Constant'
     */
    Chassis_B.Tires.FrntTires.Lft.Fx = Chassis_P.Constant_Value_g[0];
    Chassis_B.Tires.FrntTires.Lft.Fy = Chassis_P.Constant_Value_g[1];
    Chassis_B.Tires.FrntTires.Lft.Fz = Chassis_P.Constant_Value_g[2];
    Chassis_B.Tires.FrntTires.Rght.Fx = Chassis_P.Constant_Value_g[3];
    Chassis_B.Tires.FrntTires.Rght.Fy = Chassis_P.Constant_Value_g[4];
    Chassis_B.Tires.FrntTires.Rght.Fz = Chassis_P.Constant_Value_g[5];

    /* BusCreator generated from: '<Root>/VehBody' incorporates:
     *  Constant: '<S94>/Constant'
     */
    Chassis_B.Tires.RearTires.Lft.Fx = Chassis_P.Constant_Value_g[6];
    Chassis_B.Tires.RearTires.Lft.Fy = Chassis_P.Constant_Value_g[7];
    Chassis_B.Tires.RearTires.Lft.Fz = Chassis_P.Constant_Value_g[8];
    Chassis_B.Tires.RearTires.Rght.Fx = Chassis_P.Constant_Value_g[9];
    Chassis_B.Tires.RearTires.Rght.Fy = Chassis_P.Constant_Value_g[10];
    Chassis_B.Tires.RearTires.Rght.Fz = Chassis_P.Constant_Value_g[11];

    /* BusCreator generated from: '<Root>/VehBody' incorporates:
     *  Constant: '<S92>/Constant1'
     */
    Chassis_B.Ext.Mx = Chassis_P.Constant1_Value[0];
    Chassis_B.Ext.My = Chassis_P.Constant1_Value[1];
    Chassis_B.Ext.Mz = Chassis_P.Constant1_Value[2];

    /* BusCreator generated from: '<Root>/VehBody' */
    Chassis_B.Hitch.Mx = 0.0;
    Chassis_B.Hitch.My = 0.0;
    Chassis_B.Hitch.Mz = 0.0;
  }

  /* Product: '<S112>/Product8' */
  rtb_Abs1[0] = rtb_Add1_b_idx_1 * Chassis_B.ubvbwb[0];

  /* Product: '<S112>/Product1' incorporates:
   *  Sum: '<S102>/Add'
   */
  rtb_Abs1[3] = rtb_Product1_l * rtb_pqr[0];

  /* Product: '<S112>/Product8' */
  rtb_Abs1[1] = Fzo_prime_idx_2 * Chassis_B.ubvbwb[1];

  /* Product: '<S112>/Product1' incorporates:
   *  Sum: '<S102>/Add'
   */
  rtb_Abs1[4] = rtb_Product_c * rtb_pqr[1];

  /* Product: '<S112>/Product8' */
  rtb_Abs1[2] = rtb_ixk * Chassis_B.ubvbwb[2];

  /* Product: '<S112>/Product1' incorporates:
   *  Sum: '<S102>/Add'
   */
  rtb_Abs1[5] = rtb_angles_idx_0 * rtb_pqr[2];

  /* Sum: '<S112>/Sum of Elements' */
  rtb_phidot = -0.0;
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 6; rtb_phidot_tmp++) {
    /* Abs: '<S112>/Abs' */
    rtb_angles_idx_1 = std::abs(rtb_Abs1[rtb_phidot_tmp]);

    /* Sum: '<S112>/Sum of Elements' */
    rtb_phidot += rtb_angles_idx_1;

    /* Abs: '<S112>/Abs' */
    rtb_Abs1[rtb_phidot_tmp] = rtb_angles_idx_1;
  }

  /* Product: '<S112>/Product9' */
  rtb_Abs1[0] = Chassis_B.ubvbwb[0] * rtb_Product[0];

  /* Product: '<S112>/Product2' incorporates:
   *  UnaryMinus: '<S102>/Unary Minus'
   */
  rtb_Abs1[3] = rtb_pqr[0] * -rtb_UnaryMinus_b[0];

  /* Product: '<S112>/Product9' */
  rtb_Abs1[1] = Chassis_B.ubvbwb[1] * rtb_Product[1];

  /* Product: '<S112>/Product2' incorporates:
   *  UnaryMinus: '<S102>/Unary Minus'
   */
  rtb_Abs1[4] = rtb_pqr[1] * -rtb_UnaryMinus_b[1];

  /* Product: '<S112>/Product9' */
  rtb_Abs1[2] = Chassis_B.ubvbwb[2] * rtb_Product[2];

  /* Product: '<S112>/Product2' incorporates:
   *  UnaryMinus: '<S102>/Unary Minus'
   */
  rtb_Abs1[5] = rtb_pqr[2] * -rtb_Product2_c;

  /* Sum: '<S112>/Sum of Elements1' */
  rtb_thetadot = -0.0;
  for (rtb_phidot_tmp = 0; rtb_phidot_tmp < 6; rtb_phidot_tmp++) {
    /* Abs: '<S112>/Abs1' */
    rtb_angles_idx_1 = std::abs(rtb_Abs1[rtb_phidot_tmp]);

    /* Sum: '<S112>/Sum of Elements1' */
    rtb_thetadot += rtb_angles_idx_1;

    /* Abs: '<S112>/Abs1' */
    rtb_Abs1[rtb_phidot_tmp] = rtb_angles_idx_1;
  }

  /* Outport: '<Root>/VehBody' incorporates:
   *  BusCreator generated from: '<Root>/VehBody'
   */
  Chassis_Y.VehBody.BdyFrm.Pwr.PwrExt = rtb_phidot;

  /* Sum: '<S174>/Add1' */
  rtb_angles_idx_1 = rtb_Sum_eb[0] + Chassis_B.ubvbwb[0];
  rtb_UnaryMinus_i = rtb_Sum_eb[1] + Chassis_B.ubvbwb[1];

  /* Switch: '<S180>/Switch' incorporates:
   *  Abs: '<S180>/Abs'
   *  Constant: '<S181>/Constant'
   *  Constant: '<S182>/Constant'
   *  Fcn: '<S180>/Fcn'
   *  Logic: '<S180>/Logical Operator'
   *  RelationalOperator: '<S181>/Compare'
   *  RelationalOperator: '<S182>/Compare'
   */
  if ((rtb_angles_idx_1 >= -Chassis_P.VehicleBody6DOF_xdot_tol) &&
      (rtb_angles_idx_1 <= Chassis_P.VehicleBody6DOF_xdot_tol)) {
    rtb_phidot = 0.2 / (3.0 - rt_powd_snf(rtb_angles_idx_1 / 0.1, 2.0));
  } else {
    rtb_phidot = std::abs(rtb_angles_idx_1);
  }

  /* End of Switch: '<S180>/Switch' */

  /* Outport: '<Root>/VehBody' incorporates:
   *  BusCreator generated from: '<Root>/VehBody'
   *  Concatenate: '<S125>/Vector Concatenate'
   *  ForEachSliceAssignment generated from: '<S97>/FsB'
   *  Product: '<S175>/Divide'
   *  Selector: '<S107>/Selector4'
   *  Selector: '<S107>/Selector5'
   *  Selector: '<S107>/Selector6'
   *  Selector: '<S107>/Selector7'
   *  Sum: '<S102>/Add'
   *  Sum: '<S113>/Add'
   *  Sum: '<S158>/Add1'
   *  Sum: '<S166>/Add1'
   *  Sum: '<S174>/Add1'
   *  Sum: '<S186>/Add1'
   *  Sum: '<S194>/Add1'
   *  Sum: '<S202>/Add1'
   *  Trigonometry: '<S175>/Trigonometric Function'
   *  UnaryMinus: '<S102>/Unary Minus'
   *  UnaryMinus: '<S102>/Unary Minus1'
   *  UnitConversion: '<S113>/Unit Conversion3'
   */
  Chassis_Y.VehBody.InertFrm.Cg.Disp.X = rtb_pqr_a;
  Chassis_Y.VehBody.InertFrm.Cg.Disp.Y = rtb_VectorConcatenate_tmp;
  Chassis_Y.VehBody.InertFrm.Cg.Disp.Z = rtb_Subtract1_idx_1_tmp;
  Chassis_Y.VehBody.InertFrm.Cg.Vel.Xdot = rtb_Integrator1_idx_0_tmp;
  Chassis_Y.VehBody.InertFrm.Cg.Vel.Ydot = rtb_Cg_h_Vel_Ydot;
  Chassis_Y.VehBody.InertFrm.Cg.Vel.Zdot = rtb_Cg_h_Vel_Zdot;
  Chassis_Y.VehBody.InertFrm.Cg.Ang.phi = rtb_Subtract1_idx_0;
  Chassis_Y.VehBody.InertFrm.Cg.Ang.theta = rtb_Subtract1_idx_1;
  Chassis_Y.VehBody.InertFrm.Cg.Ang.psi = rtb_Subtract1_idx_2;
  Chassis_Y.VehBody.InertFrm.FrntAxl.Lft.Disp.X = rtb_Add_ct[0];
  Chassis_Y.VehBody.InertFrm.FrntAxl.Lft.Disp.Y = rtb_Add_ct[1];
  Chassis_Y.VehBody.InertFrm.FrntAxl.Lft.Disp.Z = rtb_Add_ct[2];
  Chassis_Y.VehBody.InertFrm.FrntAxl.Lft.Vel.Xdot = rtb_V_wb[0];
  Chassis_Y.VehBody.InertFrm.FrntAxl.Lft.Vel.Ydot = rtb_V_wb[1];
  Chassis_Y.VehBody.InertFrm.FrntAxl.Lft.Vel.Zdot = rtb_V_wb[2];
  Chassis_Y.VehBody.InertFrm.FrntAxl.Rght.Disp.X = rtb_Add_fb[0];
  Chassis_Y.VehBody.InertFrm.FrntAxl.Rght.Disp.Y = rtb_Add_fb[1];
  Chassis_Y.VehBody.InertFrm.FrntAxl.Rght.Disp.Z = rtb_Add_fb[2];
  Chassis_Y.VehBody.InertFrm.FrntAxl.Rght.Vel.Xdot = Itemp_tmp_0;
  Chassis_Y.VehBody.InertFrm.FrntAxl.Rght.Vel.Ydot = rtb_FrntAxl_Rght_Vel_Ydot;
  Chassis_Y.VehBody.InertFrm.FrntAxl.Rght.Vel.Zdot = rtb_FrntAxl_Rght_Vel_Zdot;
  Chassis_Y.VehBody.InertFrm.RearAxl.Lft.Disp.X = rtb_Add_d1[0];
  Chassis_Y.VehBody.InertFrm.RearAxl.Lft.Disp.Y = rtb_Add_d1[1];
  Chassis_Y.VehBody.InertFrm.RearAxl.Lft.Disp.Z = rtb_Add_d1[2];
  Chassis_Y.VehBody.InertFrm.RearAxl.Lft.Vel.Xdot = rtb_Lft_c_Vel_Xdot;
  Chassis_Y.VehBody.InertFrm.RearAxl.Lft.Vel.Ydot = rtb_Lft_c_Vel_Ydot;
  Chassis_Y.VehBody.InertFrm.RearAxl.Lft.Vel.Zdot = rtb_UnaryMinus_b[2];
  Chassis_Y.VehBody.InertFrm.RearAxl.Rght.Disp.X = rtb_Add_cm[0];
  Chassis_Y.VehBody.InertFrm.RearAxl.Rght.Disp.Y = rtb_Add_cm[1];
  Chassis_Y.VehBody.InertFrm.RearAxl.Rght.Disp.Z = rtb_Add_cm[2];
  Chassis_Y.VehBody.InertFrm.RearAxl.Rght.Vel.Xdot = rtb_UnaryMinus1[0];
  Chassis_Y.VehBody.InertFrm.RearAxl.Rght.Vel.Ydot = rtb_UnaryMinus1[1];
  Chassis_Y.VehBody.InertFrm.RearAxl.Rght.Vel.Zdot = rtb_UnaryMinus1[2];
  Chassis_Y.VehBody.InertFrm.Hitch.Disp.X = rtb_UnitConversion3_h;
  Chassis_Y.VehBody.InertFrm.Hitch.Disp.Y = rtb_Hitch_Disp_Y;
  Chassis_Y.VehBody.InertFrm.Hitch.Disp.Z = rtb_Hitch_Disp_Z;
  Chassis_Y.VehBody.InertFrm.Hitch.Vel.Xdot = rtb_Hitch_Vel_Xdot;
  Chassis_Y.VehBody.InertFrm.Hitch.Vel.Ydot = rtb_Hitch_Vel_Ydot;
  Chassis_Y.VehBody.InertFrm.Hitch.Vel.Zdot = rtb_Hitch_Vel_Zdot;
  Chassis_Y.VehBody.InertFrm.Geom.Disp.X = rtb_Add[0];
  Chassis_Y.VehBody.InertFrm.Geom.Disp.Y = rtb_Add[1];
  Chassis_Y.VehBody.InertFrm.Geom.Disp.Z = rtb_Add[2];
  Chassis_Y.VehBody.InertFrm.Geom.Vel.Xdot = rtb_Vel_Xdot;
  Chassis_Y.VehBody.InertFrm.Geom.Vel.Ydot = rtb_Vel_Ydot;
  Chassis_Y.VehBody.InertFrm.Geom.Vel.Zdot = rtb_thetadot_tmp;
  Chassis_Y.VehBody.BdyFrm.Cg.Vel.xdot = rtb_VectorConcatenate_b_tmp;
  Chassis_Y.VehBody.BdyFrm.Cg.Vel.ydot = rtb_UnaryMinus_d_tmp;
  Chassis_Y.VehBody.BdyFrm.Cg.Vel.zdot = rtb_VectorConcatenate_b_tmp_0;
  Chassis_Y.VehBody.BdyFrm.Cg.Ang.Beta = rtb_Beta;
  Chassis_Y.VehBody.BdyFrm.Cg.AngVel.p = rtb_VectorConcatenate_b_tmp_1;
  Chassis_Y.VehBody.BdyFrm.Cg.AngVel.q = rtb_VectorConcatenate_b_tmp_2;
  Chassis_Y.VehBody.BdyFrm.Cg.AngVel.r = rtb_VectorConcatenate_b_tmp_3;
  Chassis_Y.VehBody.BdyFrm.Cg.Acc.ax = (rtb_UnitConversion3[0] + Chassis_B.Sum[0]) * 0.10197162129779282;
  Chassis_Y.VehBody.BdyFrm.Cg.Acc.ay = (rtb_UnitConversion3[1] + Chassis_B.Sum[1]) * 0.10197162129779282;
  Chassis_Y.VehBody.BdyFrm.Cg.Acc.az = (rtb_UnitConversion3[2] + Chassis_B.Sum[2]) * 0.10197162129779282;
  Chassis_Y.VehBody.BdyFrm.Cg.Acc.xddot = Chassis_B.Sum[0];
  Chassis_Y.VehBody.BdyFrm.Cg.Acc.yddot = Chassis_B.Sum[1];
  Chassis_Y.VehBody.BdyFrm.Cg.Acc.zddot = Chassis_B.Sum[2];
  Chassis_Y.VehBody.BdyFrm.Cg.AngAcc.pdot = Chassis_B.Product2[0];
  Chassis_Y.VehBody.BdyFrm.Cg.AngAcc.qdot = Chassis_B.Product2[1];
  Chassis_Y.VehBody.BdyFrm.Cg.AngAcc.rdot = Chassis_B.Product2[2];
  std::memcpy(&Chassis_Y.VehBody.BdyFrm.Cg.DCM[0], &rtb_VectorConcatenate[0], 9U * sizeof(real_T));
  Chassis_Y.VehBody.BdyFrm.Forces.Body.Fx = rtb_Add1_b_idx_1;
  Chassis_Y.VehBody.BdyFrm.Forces.Body.Fy = Fzo_prime_idx_2;
  Chassis_Y.VehBody.BdyFrm.Forces.Body.Fz = rtb_ixk;
  Chassis_Y.VehBody.BdyFrm.Forces.Ext.Fx = rtb_UnaryMinus_h_idx_0;
  Chassis_Y.VehBody.BdyFrm.Forces.Ext.Fy = rtb_UnaryMinus_h_idx_1;
  Chassis_Y.VehBody.BdyFrm.Forces.Ext.Fz = rtb_UnaryMinus_h_idx_2;
  Chassis_Y.VehBody.BdyFrm.Forces.FrntAxl.Lft.Fx = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[0];
  Chassis_Y.VehBody.BdyFrm.Forces.FrntAxl.Lft.Fy = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[1];
  Chassis_Y.VehBody.BdyFrm.Forces.FrntAxl.Lft.Fz = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[2];
  Chassis_Y.VehBody.BdyFrm.Forces.FrntAxl.Rght.Fx = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[3];
  Chassis_Y.VehBody.BdyFrm.Forces.FrntAxl.Rght.Fy = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[4];
  Chassis_Y.VehBody.BdyFrm.Forces.FrntAxl.Rght.Fz = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[5];
  Chassis_Y.VehBody.BdyFrm.Forces.RearAxl.Lft.Fx = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[6];
  Chassis_Y.VehBody.BdyFrm.Forces.RearAxl.Lft.Fy = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[7];
  Chassis_Y.VehBody.BdyFrm.Forces.RearAxl.Lft.Fz = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[8];
  Chassis_Y.VehBody.BdyFrm.Forces.RearAxl.Rght.Fx = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[9];
  Chassis_Y.VehBody.BdyFrm.Forces.RearAxl.Rght.Fy = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[10];
  Chassis_Y.VehBody.BdyFrm.Forces.RearAxl.Rght.Fz = rtb_ImpAsg_InsertedFor_FsB_at_inport_0[11];
  Chassis_Y.VehBody.BdyFrm.Forces.Hitch = Chassis_B.Hitch_n;
  Chassis_Y.VehBody.BdyFrm.Forces.Tires = Chassis_B.Tires;
  Chassis_Y.VehBody.BdyFrm.Forces.Drag.Fx = -rtb_Product[0];
  Chassis_Y.VehBody.BdyFrm.Forces.Drag.Fy = -rtb_Product[1];
  Chassis_Y.VehBody.BdyFrm.Forces.Drag.Fz = -rtb_Product[2];
  Chassis_Y.VehBody.BdyFrm.Forces.Grvty.Fx = rtb_Fg_B[0];
  Chassis_Y.VehBody.BdyFrm.Forces.Grvty.Fy = rtb_Fg_B[1];
  Chassis_Y.VehBody.BdyFrm.Forces.Grvty.Fz = rtb_Fg_B[2];
  Chassis_Y.VehBody.BdyFrm.Moments.Body.Mx = rtb_Product1_l;
  Chassis_Y.VehBody.BdyFrm.Moments.Body.My = rtb_Product_c;
  Chassis_Y.VehBody.BdyFrm.Moments.Body.Mz = rtb_angles_idx_0;
  Chassis_Y.VehBody.BdyFrm.Moments.Drag.Mx = -rtb_UnaryMinus_b[0];
  Chassis_Y.VehBody.BdyFrm.Moments.Drag.My = -rtb_UnaryMinus_b[1];
  Chassis_Y.VehBody.BdyFrm.Moments.Drag.Mz = -rtb_Product2_c;
  Chassis_Y.VehBody.BdyFrm.Moments.Ext = Chassis_B.Ext;
  Chassis_Y.VehBody.BdyFrm.Moments.Hitch = Chassis_B.Hitch;
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Lft.Disp.x = Chassis_B.Selector1[0];
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Lft.Disp.y = Chassis_B.Selector1[1];
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Lft.Disp.z = Chassis_B.Selector1[2];
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Lft.Vel.ydot = rtb_Add1_b_idx_0;
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Lft.Vel.zdot = rtb_Add1_b_idx_2 + Chassis_B.ubvbwb[2];
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Rght.Disp.x = Chassis_B.Selector1_n[0];
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Rght.Disp.y = Chassis_B.Selector1_n[1];
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Rght.Disp.z = Chassis_B.Selector1_n[2];
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Rght.Vel.xdot = rtb_Add1_h[0];
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Rght.Vel.ydot = u0;
  Chassis_Y.VehBody.BdyFrm.FrntAxl.Rght.Vel.zdot = rtb_Add1_h[2] + Chassis_B.ubvbwb[2];
  Chassis_Y.VehBody.BdyFrm.RearAxl.Lft.Disp.x = Chassis_B.Selector1_o[0];
  Chassis_Y.VehBody.BdyFrm.RearAxl.Lft.Disp.y = Chassis_B.Selector1_o[1];
  Chassis_Y.VehBody.BdyFrm.RearAxl.Lft.Disp.z = Chassis_B.Selector1_o[2];
  Chassis_Y.VehBody.BdyFrm.RearAxl.Lft.Vel.xdot = rtb_Add1_j[0];
  Chassis_Y.VehBody.BdyFrm.RearAxl.Lft.Vel.ydot = u0_0;
  Chassis_Y.VehBody.BdyFrm.RearAxl.Lft.Vel.zdot = rtb_Add1_j[2] + Chassis_B.ubvbwb[2];
  Chassis_Y.VehBody.BdyFrm.RearAxl.Rght.Disp.x = Chassis_B.Selector1_l[0];
  Chassis_Y.VehBody.BdyFrm.RearAxl.Rght.Disp.y = Chassis_B.Selector1_l[1];
  Chassis_Y.VehBody.BdyFrm.RearAxl.Rght.Disp.z = Chassis_B.Selector1_l[2];
  Chassis_Y.VehBody.BdyFrm.RearAxl.Rght.Vel.xdot = rtb_Add1_p[0];
  Chassis_Y.VehBody.BdyFrm.RearAxl.Rght.Vel.ydot = rtb_xdot_p;
  Chassis_Y.VehBody.BdyFrm.RearAxl.Rght.Vel.zdot = rtb_Add1_p[2] + Chassis_B.ubvbwb[2];
  Chassis_Y.VehBody.BdyFrm.Hitch.Disp.x = Chassis_B.VectorConcatenate[0];
  Chassis_Y.VehBody.BdyFrm.Hitch.Disp.y = Chassis_B.VectorConcatenate[1];
  Chassis_Y.VehBody.BdyFrm.Hitch.Disp.z = Chassis_B.VectorConcatenate[2];
  Chassis_Y.VehBody.BdyFrm.Hitch.Vel.xdot = rtb_Add1_l[0] + Chassis_B.ubvbwb[0];
  Chassis_Y.VehBody.BdyFrm.Hitch.Vel.ydot = rtb_Add1_l[1] + Chassis_B.ubvbwb[1];
  Chassis_Y.VehBody.BdyFrm.Hitch.Vel.zdot = rtb_Add1_l[2] + Chassis_B.ubvbwb[2];
  Chassis_Y.VehBody.BdyFrm.Pwr.Drag = rtb_thetadot;
  Chassis_Y.VehBody.BdyFrm.Geom.Disp.x = Chassis_B.Subtract[0];
  Chassis_Y.VehBody.BdyFrm.Geom.Disp.y = Chassis_B.Subtract[1];
  Chassis_Y.VehBody.BdyFrm.Geom.Disp.z = Chassis_B.Subtract[2];
  Chassis_Y.VehBody.BdyFrm.Geom.Vel.xdot = rtb_angles_idx_1;
  Chassis_Y.VehBody.BdyFrm.Geom.Vel.ydot = rtb_UnaryMinus_i;
  Chassis_Y.VehBody.BdyFrm.Geom.Vel.zdot = rtb_Sum_eb[2] + Chassis_B.ubvbwb[2];
  Chassis_Y.VehBody.BdyFrm.Geom.Ang.Beta = std::atan(rtb_UnaryMinus_i / rtb_phidot);

  /* SignalConversion generated from: '<S219>/Vector Concatenate4' */
  rtb_ImpAsg_InsertedFor_Fx_at_inport_0[0] = Chassis_B.Selector1[2];

  /* SignalConversion generated from: '<S219>/Vector Concatenate4' */
  rtb_ImpAsg_InsertedFor_Fx_at_inport_0[1] = Chassis_B.Selector1_n[2];

  /* SignalConversion generated from: '<S219>/Vector Concatenate4' */
  rtb_ImpAsg_InsertedFor_Fx_at_inport_0[2] = Chassis_B.Selector1_o[2];

  /* SignalConversion generated from: '<S219>/Vector Concatenate4' */
  rtb_ImpAsg_InsertedFor_Fx_at_inport_0[3] = Chassis_B.Selector1_l[2];

  /* SignalConversion generated from: '<S219>/Vector Concatenate3' */
  rtb_ImpAsg_InsertedFor_Fy_at_inport_0[0] = rtb_Add_ct[2];

  /* SignalConversion generated from: '<S219>/Vector Concatenate3' */
  rtb_ImpAsg_InsertedFor_Fy_at_inport_0[1] = rtb_Add_fb[2];

  /* SignalConversion generated from: '<S219>/Vector Concatenate3' */
  rtb_ImpAsg_InsertedFor_Fy_at_inport_0[2] = rtb_Add_d1[2];

  /* SignalConversion generated from: '<S219>/Vector Concatenate3' */
  rtb_ImpAsg_InsertedFor_Fy_at_inport_0[3] = rtb_Add_cm[2];
  for (Itemp_tmp = 0; Itemp_tmp < 4; Itemp_tmp++) {
    /* Sum: '<S219>/Subtract' */
    rtb_pqr_a = rtb_IntegratorSecondOrder_o1[Itemp_tmp];

    /* Outport: '<Root>/WheelsOut' incorporates:
     *  BusCreator generated from: '<Root>/WheelsOut'
     *  Sum: '<S219>/Subtract'
     */
    Chassis_Y.WheelsOut.TireFrame.Omega[Itemp_tmp] = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[Itemp_tmp];
    Chassis_Y.WheelsOut.TireFrame.Fx[Itemp_tmp] = rtb_Integrator[Itemp_tmp];
    Chassis_Y.WheelsOut.TireFrame.Fy[Itemp_tmp] = rtb_Integrator_g[Itemp_tmp];
    Chassis_Y.WheelsOut.TireFrame.Fz[Itemp_tmp] = rtb_Saturation_h2[Itemp_tmp];
    Chassis_Y.WheelsOut.TireFrame.Mx[Itemp_tmp] = rtb_Mx[Itemp_tmp];
    Chassis_Y.WheelsOut.TireFrame.My[Itemp_tmp] = rtb_Integrator_c[Itemp_tmp];
    Chassis_Y.WheelsOut.TireFrame.Mz[Itemp_tmp] = cosprimealpha[Itemp_tmp];
    Chassis_Y.WheelsOut.TireFrame.Re[Itemp_tmp] = Chassis_B.Re[Itemp_tmp];
    Chassis_Y.WheelsOut.TireFrame.dz[Itemp_tmp] = (rtb_ImpAsg_InsertedFor_Fx_at_inport_0[Itemp_tmp] - rtb_pqr_a) -
                                                  rtb_ImpAsg_InsertedFor_Fy_at_inport_0[Itemp_tmp];
    Chassis_Y.WheelsOut.TireFrame.z[Itemp_tmp] = rtb_pqr_a;
    Chassis_Y.WheelsOut.TireFrame.zdot[Itemp_tmp] = rtb_Switch[Itemp_tmp];
    Chassis_Y.WheelsOut.TireFrame.Kappa[Itemp_tmp] = rtb_Kappa[Itemp_tmp];
    Chassis_Y.WheelsOut.TireFrame.Alpha[Itemp_tmp] = rtb_Alpha[Itemp_tmp];

    /* Selector: '<S4>/Selector2' incorporates:
     *  Concatenate: '<S9>/Matrix Concatenate1'
     */
    rtb_phidot_tmp = 3 * Itemp_tmp + 2;

    /* Outport: '<Root>/WheelsOut' incorporates:
     *  Integrator: '<S211>/Integrator1'
     *  Selector: '<S4>/Selector2'
     *  Selector: '<S4>/Selector3'
     */
    Chassis_Y.WheelsOut.Steering.WhlAng[Itemp_tmp] = Chassis_X.Integrator1_CSTATE_p[rtb_phidot_tmp];
    Chassis_Y.WheelsOut.Steering.WhlCmb[Itemp_tmp] = Chassis_X.Integrator1_CSTATE_p[3 * Itemp_tmp];

    /* Concatenate: '<S9>/Matrix Concatenate1' incorporates:
     *  Concatenate: '<S9>/Matrix Concatenate4'
     *  ForEachSliceAssignment generated from: '<S17>/WhlFz'
     */
    iy = Itemp_tmp << 1;
    rtb_ImpAsg_InsertedFor_FsB_at_inport_0[3 * Itemp_tmp] = rtb_MatrixConcatenate4[iy];
    rtb_ImpAsg_InsertedFor_FsB_at_inport_0[3 * Itemp_tmp + 1] = rtb_MatrixConcatenate4[iy + 1];
    rtb_ImpAsg_InsertedFor_FsB_at_inport_0[rtb_phidot_tmp] = rtb_ImpAsg_InsertedFor_WhlFz_at_inport_0[Itemp_tmp];
  }

  /* Outport: '<Root>/Susp' incorporates:
   *  BusCreator generated from: '<Root>/Susp'
   *  Integrator: '<S211>/Integrator1'
   *  Selector: '<S5>/Selector1'
   *  Selector: '<S5>/Selector2'
   *  UnaryMinus: '<S217>/Unary Minus1'
   */
  Chassis_Y.Susp.Whl.xdot[0] = rtb_VectorConcatenate8[0];
  Chassis_Y.Susp.Whl.ydot[0] = rtb_Add1_b_idx_0;
  Chassis_Y.Susp.Whl.xdot[1] = rtb_VectorConcatenate8[3];
  Chassis_Y.Susp.Whl.ydot[1] = u0;
  Chassis_Y.Susp.Whl.xdot[2] = rtb_VectorConcatenate8[6];
  Chassis_Y.Susp.Whl.ydot[2] = u0_0;
  Chassis_Y.Susp.Whl.xdot[3] = rtb_VectorConcatenate8[9];
  Chassis_Y.Susp.Whl.ydot[3] = rtb_xdot_p;
  Chassis_Y.Susp.Whl.z[0] = rtb_z_idx_0;
  Chassis_Y.Susp.Whl.z[1] = rtb_z_idx_1;
  Chassis_Y.Susp.Whl.z[2] = rtb_z_idx_2;
  Chassis_Y.Susp.Whl.z[3] = -rtb_IntegratorSecondOrder_o1_k;

  /* Product: '<S7>/Divide' incorporates:
   *  Constant: '<S7>/Constant'
   *  ForEachSliceAssignment generated from: '<S17>/Info'
   *  Integrator: '<S7>/Integrator1'
   *  Selector: '<S9>/dWhlY select'
   *  Sum: '<S7>/Sum'
   */
  Chassis_B.Divide[0] =
      (rtb_ImpAsg_InsertedFor_Info_at_inport_0[7] - Chassis_X.Integrator1_CSTATE_a[0]) * Chassis_P.ContLPF_wc;

  /* Product: '<S8>/Divide' incorporates:
   *  Constant: '<S8>/Constant'
   *  ForEachSliceAssignment generated from: '<S17>/Info'
   *  Integrator: '<S8>/Integrator1'
   *  Selector: '<S9>/Height select'
   *  Sum: '<S8>/Sum'
   */
  Chassis_B.Divide_h[0] =
      (rtb_ImpAsg_InsertedFor_Info_at_inport_0[3] - Chassis_X.Integrator1_CSTATE_o[0]) * Chassis_P.ContLPF2_wc;

  /* Product: '<S7>/Divide' incorporates:
   *  Constant: '<S7>/Constant'
   *  ForEachSliceAssignment generated from: '<S17>/Info'
   *  Integrator: '<S7>/Integrator1'
   *  Selector: '<S9>/dWhlY select'
   *  Sum: '<S7>/Sum'
   */
  Chassis_B.Divide[1] =
      (rtb_ImpAsg_InsertedFor_Info_at_inport_0[15] - Chassis_X.Integrator1_CSTATE_a[1]) * Chassis_P.ContLPF_wc;

  /* Product: '<S8>/Divide' incorporates:
   *  Constant: '<S8>/Constant'
   *  ForEachSliceAssignment generated from: '<S17>/Info'
   *  Integrator: '<S8>/Integrator1'
   *  Selector: '<S9>/Height select'
   *  Sum: '<S8>/Sum'
   */
  Chassis_B.Divide_h[1] =
      (rtb_ImpAsg_InsertedFor_Info_at_inport_0[11] - Chassis_X.Integrator1_CSTATE_o[1]) * Chassis_P.ContLPF2_wc;

  /* Product: '<S7>/Divide' incorporates:
   *  Constant: '<S7>/Constant'
   *  ForEachSliceAssignment generated from: '<S17>/Info'
   *  Integrator: '<S7>/Integrator1'
   *  Selector: '<S9>/dWhlY select'
   *  Sum: '<S7>/Sum'
   */
  Chassis_B.Divide[2] =
      (rtb_ImpAsg_InsertedFor_Info_at_inport_0[23] - Chassis_X.Integrator1_CSTATE_a[2]) * Chassis_P.ContLPF_wc;

  /* Product: '<S8>/Divide' incorporates:
   *  Constant: '<S8>/Constant'
   *  ForEachSliceAssignment generated from: '<S17>/Info'
   *  Integrator: '<S8>/Integrator1'
   *  Selector: '<S9>/Height select'
   *  Sum: '<S8>/Sum'
   */
  Chassis_B.Divide_h[2] =
      (rtb_ImpAsg_InsertedFor_Info_at_inport_0[19] - Chassis_X.Integrator1_CSTATE_o[2]) * Chassis_P.ContLPF2_wc;

  /* Product: '<S7>/Divide' incorporates:
   *  Constant: '<S7>/Constant'
   *  ForEachSliceAssignment generated from: '<S17>/Info'
   *  Integrator: '<S7>/Integrator1'
   *  Selector: '<S9>/dWhlY select'
   *  Sum: '<S7>/Sum'
   */
  Chassis_B.Divide[3] =
      (rtb_ImpAsg_InsertedFor_Info_at_inport_0[31] - Chassis_X.Integrator1_CSTATE_a[3]) * Chassis_P.ContLPF_wc;

  /* Product: '<S8>/Divide' incorporates:
   *  Constant: '<S8>/Constant'
   *  ForEachSliceAssignment generated from: '<S17>/Info'
   *  Integrator: '<S8>/Integrator1'
   *  Selector: '<S9>/Height select'
   *  Sum: '<S8>/Sum'
   */
  Chassis_B.Divide_h[3] =
      (rtb_ImpAsg_InsertedFor_Info_at_inport_0[27] - Chassis_X.Integrator1_CSTATE_o[3]) * Chassis_P.ContLPF2_wc;

  /* Product: '<S95>/Divide' incorporates:
   *  Constant: '<S95>/Constant'
   *  Sum: '<S93>/Add'
   *  Sum: '<S93>/Add1'
   *  Sum: '<S95>/Sum'
   */
  Chassis_B.Divide_e[0] = ((Chassis_B.Sum[0] - rtb_pqr_tmp_0) - rtb_Integrator1_idx_0) * Chassis_P.Bushings_wc;
  Chassis_B.Divide_e[1] = ((rtb_Add_f_tmp + Chassis_B.Sum[1]) - rtb_Integrator1_idx_1) * Chassis_P.Bushings_wc;

  /* Trigonometry: '<S124>/sincos' */
  Drphi_0 = std::sin(Chassis_B.phithetapsi[0]);
  rtb_xdot_o = std::cos(Chassis_B.phithetapsi[0]);
  rtb_Add_n = std::cos(Chassis_B.phithetapsi[1]);

  /* Fcn: '<S124>/phidot' incorporates:
   *  Fcn: '<S124>/psidot'
   *  Trigonometry: '<S124>/sincos'
   */
  // added by dongyuanhu
  real_T rtb_pqr_saturation[3];

  /*
  rtb_pqr_saturation[0]=std::abs(rtb_pqr[0]) >= 0.001 ? rtb_pqr[0] : 0.0;
  rtb_pqr_saturation[1]=std::abs(rtb_pqr[1]) >= 0.001 ? rtb_pqr[1] : 0.0;
  rtb_pqr_saturation[2]=std::abs(rtb_pqr[2]) >= 0.001 ? rtb_pqr[2] : 0.0;
  */
  rtb_pqr_saturation[0] = rtb_pqr[0];
  rtb_pqr_saturation[1] = rtb_pqr[1];
  rtb_pqr_saturation[2] = rtb_pqr[2];

  rtb_pqr_a = Drphi_0 * rtb_pqr_saturation[1] + rtb_xdot_o * rtb_pqr_saturation[2];

  /* SignalConversion generated from: '<S115>/phi theta psi' incorporates:
   *  Fcn: '<S124>/phidot'
   *  Fcn: '<S124>/psidot'
   *  Fcn: '<S124>/thetadot'
   *  Trigonometry: '<S124>/sincos'
   */
  Chassis_B.TmpSignalConversionAtphithetapsiInport1[0] =
      std::sin(Chassis_B.phithetapsi[1]) / rtb_Add_n * rtb_pqr_a + rtb_pqr_saturation[0];
  Chassis_B.TmpSignalConversionAtphithetapsiInport1[1] =
      rtb_xdot_o * rtb_pqr_saturation[1] - Drphi_0 * rtb_pqr_saturation[2];
  Chassis_B.TmpSignalConversionAtphithetapsiInport1[2] = rtb_pqr_a / rtb_Add_n;

  /* Product: '<S210>/Divide' incorporates:
   *  Concatenate: '<S9>/Matrix Concatenate1'
   *  Constant: '<S210>/Constant'
   *  Selector: '<S4>/Selector'
   *  Sum: '<S210>/Sum'
   */
  Chassis_B.Divide_c[0] =
      (rtb_ImpAsg_InsertedFor_FsB_at_inport_0[2] - rtb_Integrator1_h_idx_0) * Chassis_P.ContLPF_wc_m;
  Chassis_B.Divide_c[1] =
      (rtb_ImpAsg_InsertedFor_FsB_at_inport_0[5] - rtb_Integrator1_h_idx_1) * Chassis_P.ContLPF_wc_m;
  Chassis_B.Divide_c[2] =
      (rtb_ImpAsg_InsertedFor_FsB_at_inport_0[8] - rtb_Integrator1_h_idx_2) * Chassis_P.ContLPF_wc_m;
  Chassis_B.Divide_c[3] =
      (rtb_ImpAsg_InsertedFor_FsB_at_inport_0[11] - rtb_Integrator1_h_idx_3) * Chassis_P.ContLPF_wc_m;
  for (Itemp_tmp = 0; Itemp_tmp <= 10; Itemp_tmp += 2) {
    /* BusCreator generated from: '<Root>/Susp' incorporates:
     *  Concatenate: '<S9>/Matrix Concatenate'
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     */
    tmp_5 = _mm_loadu_pd(&rtb_VectorConcatenate1_g[Itemp_tmp]);

    /* Outport: '<Root>/Susp' incorporates:
     *  BusCreator generated from: '<Root>/Susp'
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     */
    _mm_storeu_pd(&Chassis_Y.Susp.Veh.F[Itemp_tmp], tmp_5);

    /* BusCreator generated from: '<Root>/Susp' incorporates:
     *  ForEachSliceAssignment generated from: '<S17>/VehM'
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     */
    tmp_5 = _mm_loadu_pd(&rtb_ImpAsg_InsertedFor_VehM_at_inport_0[Itemp_tmp]);

    /* Outport: '<Root>/Susp' incorporates:
     *  BusCreator generated from: '<Root>/Susp'
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     */
    _mm_storeu_pd(&Chassis_Y.Susp.Veh.M[Itemp_tmp], tmp_5);

    /* BusCreator generated from: '<Root>/Susp' incorporates:
     *  Concatenate: '<S9>/Matrix Concatenate1'
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     */
    tmp_5 = _mm_loadu_pd(&rtb_ImpAsg_InsertedFor_FsB_at_inport_0[Itemp_tmp]);

    /* Outport: '<Root>/Susp' incorporates:
     *  BusCreator generated from: '<Root>/Susp'
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     */
    _mm_storeu_pd(&Chassis_Y.Susp.Whl.F[Itemp_tmp], tmp_5);

    /* BusCreator generated from: '<Root>/Susp' incorporates:
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     */
    tmp_5 = _mm_loadu_pd(&rtb_ImpAsg_InsertedFor_WhlAng_at_inport_0[Itemp_tmp]);

    /* Outport: '<Root>/Susp' incorporates:
     *  BusCreator generated from: '<Root>/Susp'
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     */
    _mm_storeu_pd(&Chassis_Y.Susp.Whl.Ang[Itemp_tmp], tmp_5);

    /* BusCreator generated from: '<Root>/Susp' incorporates:
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     *  Sum: '<S10>/Add'
     */
    tmp_6 = _mm_loadu_pd(&rtb_AngVel_g[Itemp_tmp]);

    /* Outport: '<Root>/Susp' incorporates:
     *  BusCreator generated from: '<Root>/Susp'
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     */
    _mm_storeu_pd(&Chassis_Y.Susp.Whl.AngVel[Itemp_tmp], tmp_6);

    /* Integrator: '<S211>/Integrator1' incorporates:
     *  BusCreator generated from: '<Root>/Susp'
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     */
    tmp_6 = _mm_loadu_pd(&Chassis_X.Integrator1_CSTATE_p[Itemp_tmp]);

    /* Product: '<S211>/Divide' incorporates:
     *  BusCreator generated from: '<Root>/Susp'
     *  Constant: '<S211>/Constant'
     *  ForEachSliceAssignment generated from: '<S17>/WhlAng'
     */
    _mm_storeu_pd(&Chassis_B.Divide_m[Itemp_tmp],
                  _mm_mul_pd(_mm_sub_pd(tmp_5, tmp_6), _mm_set1_pd(Chassis_P.ContLPF1_wc)));
  }

  /* Sum: '<S230>/Sum2' incorporates:
   *  Constant: '<S230>/Fg'
   */
  rtb_UnaryMinus_i = Chassis_P.CombinedSlipWheel2DOF_GRAVITY * Chassis_P.CombinedSlipWheel2DOF_MASS;

  /* Gain: '<S230>/Gain1' */
  rtb_pqr_a = 1.0 / Chassis_P.CombinedSlipWheel2DOF_MASS;

  /* Product: '<S232>/Product2' incorporates:
   *  Product: '<S233>/Product2'
   */
  rtb_Subtract1_idx_0 = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[0] * Chassis_B.Re[0];

  /* Saturate: '<S232>/Saturation1' */
  if (rtb_sig_x[0] > Chassis_P.Saturation1_UpperSat) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat;
  } else if (rtb_sig_x[0] < Chassis_P.Saturation1_LowerSat) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat;
  } else {
    Drphi_0 = rtb_sig_x[0];
  }

  /* Abs: '<S232>/Abs' incorporates:
   *  Abs: '<S235>/Abs'
   *  Sum: '<S232>/Add1'
   */
  rtb_Subtract1_idx_1 = std::abs(rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[0] - rtb_Subtract1_idx_0);

  /* Product: '<S232>/Product3' incorporates:
   *  Abs: '<S232>/Abs'
   *  Saturate: '<S232>/Saturation1'
   */
  u0 = rtb_Subtract1_idx_1 / Drphi_0;

  /* Saturate: '<S232>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_d) {
    u0 = Chassis_P.Saturation_UpperSat_d;
  } else if (u0 < Chassis_P.Saturation_LowerSat_f) {
    u0 = Chassis_P.Saturation_LowerSat_f;
  }

  /* Product: '<S232>/Product1' incorporates:
   *  Saturate: '<S232>/Saturation'
   *  Sum: '<S232>/Add'
   */
  Chassis_B.Product1[0] = (lam_mux[0] - rtb_Integrator[0]) * u0;

  /* Saturate: '<S233>/Saturation1' */
  if (rtb_sig_y[0] > Chassis_P.Saturation1_UpperSat_k) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat_k;
  } else if (rtb_sig_y[0] < Chassis_P.Saturation1_LowerSat_f) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat_f;
  } else {
    Drphi_0 = rtb_sig_y[0];
  }

  /* Product: '<S233>/Product3' incorporates:
   *  Abs: '<S233>/Abs'
   *  Saturate: '<S233>/Saturation1'
   *  Sum: '<S233>/Add1'
   */
  u0 = std::abs(rtb_ImpAsg_InsertedFor_ydotWheel_at_inport_0[0] - rtb_Subtract1_idx_0) / Drphi_0;

  /* Saturate: '<S233>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_k) {
    u0 = Chassis_P.Saturation_UpperSat_k;
  } else if (u0 < Chassis_P.Saturation_LowerSat_p) {
    u0 = Chassis_P.Saturation_LowerSat_p;
  }

  /* Product: '<S233>/Product1' incorporates:
   *  Saturate: '<S233>/Saturation'
   *  Sum: '<S233>/Add'
   */
  Chassis_B.Product1_k[0] = (Gykappa[0] - rtb_Integrator_g[0]) * u0;

  /* Saturate: '<S235>/Saturation1' incorporates:
   *  Saturate: '<S232>/Saturation1'
   */
  if (rtb_sig_x[0] > Chassis_P.Saturation1_UpperSat_g) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat_g;
  } else if (rtb_sig_x[0] < Chassis_P.Saturation1_LowerSat_a) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat_a;
  } else {
    Drphi_0 = rtb_sig_x[0];
  }

  /* Product: '<S235>/Product3' incorporates:
   *  Saturate: '<S235>/Saturation1'
   */
  u0 = rtb_Subtract1_idx_1 / Drphi_0;

  /* Saturate: '<S235>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_o) {
    u0 = Chassis_P.Saturation_UpperSat_o;
  } else if (u0 < Chassis_P.Saturation_LowerSat_o) {
    u0 = Chassis_P.Saturation_LowerSat_o;
  }

  /* Product: '<S235>/Product1' incorporates:
   *  Saturate: '<S235>/Saturation'
   *  Sum: '<S235>/Add'
   */
  Chassis_B.Product1_o[0] = (rtb_My[0] - rtb_Integrator_c[0]) * u0;

  /* Switch: '<S230>/Switch' incorporates:
   *  Constant: '<S230>/Constant'
   */
  if (rtb_Sum6[0] > Chassis_P.Switch_Threshold) {
    Drphi_0 = rtb_Switch[0];
  } else {
    Drphi_0 = Chassis_P.Constant_Value_d;
  }

  /* Gain: '<S230>/Gain1' incorporates:
   *  Gain: '<S230>/Gain2'
   *  Sum: '<S230>/Sum2'
   *  Switch: '<S230>/Switch'
   */
  Chassis_B.Gain1[0] = (((rtb_xdot[0] + rtb_UnaryMinus_i) - rtb_Saturation_h2[0]) -
                        Chassis_P.CombinedSlipWheel2DOF_VERTICAL_DAMPING * Drphi_0) *
                       rtb_pqr_a;

  /* Product: '<S232>/Product2' incorporates:
   *  Product: '<S233>/Product2'
   */
  rtb_Subtract1_idx_0 = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[1] * Chassis_B.Re[1];

  /* Saturate: '<S232>/Saturation1' */
  if (rtb_sig_x[1] > Chassis_P.Saturation1_UpperSat) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat;
  } else if (rtb_sig_x[1] < Chassis_P.Saturation1_LowerSat) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat;
  } else {
    Drphi_0 = rtb_sig_x[1];
  }

  /* Abs: '<S232>/Abs' incorporates:
   *  Abs: '<S235>/Abs'
   *  Sum: '<S232>/Add1'
   */
  rtb_Subtract1_idx_1 = std::abs(rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[1] - rtb_Subtract1_idx_0);

  /* Product: '<S232>/Product3' incorporates:
   *  Abs: '<S232>/Abs'
   *  Saturate: '<S232>/Saturation1'
   */
  u0 = rtb_Subtract1_idx_1 / Drphi_0;

  /* Saturate: '<S232>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_d) {
    u0 = Chassis_P.Saturation_UpperSat_d;
  } else if (u0 < Chassis_P.Saturation_LowerSat_f) {
    u0 = Chassis_P.Saturation_LowerSat_f;
  }

  /* Product: '<S232>/Product1' incorporates:
   *  Saturate: '<S232>/Saturation'
   *  Sum: '<S232>/Add'
   */
  Chassis_B.Product1[1] = (lam_mux[1] - rtb_Integrator[1]) * u0;

  /* Saturate: '<S233>/Saturation1' */
  if (rtb_sig_y[1] > Chassis_P.Saturation1_UpperSat_k) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat_k;
  } else if (rtb_sig_y[1] < Chassis_P.Saturation1_LowerSat_f) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat_f;
  } else {
    Drphi_0 = rtb_sig_y[1];
  }

  /* Product: '<S233>/Product3' incorporates:
   *  Abs: '<S233>/Abs'
   *  Saturate: '<S233>/Saturation1'
   *  Sum: '<S233>/Add1'
   */
  u0 = std::abs(rtb_ImpAsg_InsertedFor_ydotWheel_at_inport_0[1] - rtb_Subtract1_idx_0) / Drphi_0;

  /* Saturate: '<S233>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_k) {
    u0 = Chassis_P.Saturation_UpperSat_k;
  } else if (u0 < Chassis_P.Saturation_LowerSat_p) {
    u0 = Chassis_P.Saturation_LowerSat_p;
  }

  /* Product: '<S233>/Product1' incorporates:
   *  Saturate: '<S233>/Saturation'
   *  Sum: '<S233>/Add'
   */
  Chassis_B.Product1_k[1] = (Gykappa[1] - rtb_Integrator_g[1]) * u0;

  /* Saturate: '<S235>/Saturation1' incorporates:
   *  Saturate: '<S232>/Saturation1'
   */
  if (rtb_sig_x[1] > Chassis_P.Saturation1_UpperSat_g) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat_g;
  } else if (rtb_sig_x[1] < Chassis_P.Saturation1_LowerSat_a) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat_a;
  } else {
    Drphi_0 = rtb_sig_x[1];
  }

  /* Product: '<S235>/Product3' incorporates:
   *  Saturate: '<S235>/Saturation1'
   */
  u0 = rtb_Subtract1_idx_1 / Drphi_0;

  /* Saturate: '<S235>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_o) {
    u0 = Chassis_P.Saturation_UpperSat_o;
  } else if (u0 < Chassis_P.Saturation_LowerSat_o) {
    u0 = Chassis_P.Saturation_LowerSat_o;
  }

  /* Product: '<S235>/Product1' incorporates:
   *  Saturate: '<S235>/Saturation'
   *  Sum: '<S235>/Add'
   */
  Chassis_B.Product1_o[1] = (rtb_My[1] - rtb_Integrator_c[1]) * u0;

  /* Switch: '<S230>/Switch' incorporates:
   *  Constant: '<S230>/Constant'
   */
  if (rtb_Sum6[1] > Chassis_P.Switch_Threshold) {
    Drphi_0 = rtb_Switch[1];
  } else {
    Drphi_0 = Chassis_P.Constant_Value_d;
  }

  /* Gain: '<S230>/Gain1' incorporates:
   *  Gain: '<S230>/Gain2'
   *  Sum: '<S230>/Sum2'
   *  Switch: '<S230>/Switch'
   */
  Chassis_B.Gain1[1] = (((rtb_xdot[1] + rtb_UnaryMinus_i) - rtb_Saturation_h2[1]) -
                        Chassis_P.CombinedSlipWheel2DOF_VERTICAL_DAMPING * Drphi_0) *
                       rtb_pqr_a;

  /* Product: '<S232>/Product2' incorporates:
   *  Product: '<S233>/Product2'
   */
  rtb_Subtract1_idx_0 = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[2] * Chassis_B.Re[2];

  /* Saturate: '<S232>/Saturation1' */
  if (rtb_sig_x[2] > Chassis_P.Saturation1_UpperSat) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat;
  } else if (rtb_sig_x[2] < Chassis_P.Saturation1_LowerSat) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat;
  } else {
    Drphi_0 = rtb_sig_x[2];
  }

  /* Abs: '<S232>/Abs' incorporates:
   *  Abs: '<S235>/Abs'
   *  Sum: '<S232>/Add1'
   */
  rtb_Subtract1_idx_1 = std::abs(rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[2] - rtb_Subtract1_idx_0);

  /* Product: '<S232>/Product3' incorporates:
   *  Abs: '<S232>/Abs'
   *  Saturate: '<S232>/Saturation1'
   */
  u0 = rtb_Subtract1_idx_1 / Drphi_0;

  /* Saturate: '<S232>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_d) {
    u0 = Chassis_P.Saturation_UpperSat_d;
  } else if (u0 < Chassis_P.Saturation_LowerSat_f) {
    u0 = Chassis_P.Saturation_LowerSat_f;
  }

  /* Product: '<S232>/Product1' incorporates:
   *  Saturate: '<S232>/Saturation'
   *  Sum: '<S232>/Add'
   */
  Chassis_B.Product1[2] = (lam_mux[2] - rtb_Integrator[2]) * u0;

  /* Saturate: '<S233>/Saturation1' */
  if (rtb_sig_y[2] > Chassis_P.Saturation1_UpperSat_k) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat_k;
  } else if (rtb_sig_y[2] < Chassis_P.Saturation1_LowerSat_f) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat_f;
  } else {
    Drphi_0 = rtb_sig_y[2];
  }

  /* Product: '<S233>/Product3' incorporates:
   *  Abs: '<S233>/Abs'
   *  Saturate: '<S233>/Saturation1'
   *  Sum: '<S233>/Add1'
   */
  u0 = std::abs(rtb_ImpAsg_InsertedFor_ydotWheel_at_inport_0[2] - rtb_Subtract1_idx_0) / Drphi_0;

  /* Saturate: '<S233>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_k) {
    u0 = Chassis_P.Saturation_UpperSat_k;
  } else if (u0 < Chassis_P.Saturation_LowerSat_p) {
    u0 = Chassis_P.Saturation_LowerSat_p;
  }

  /* Product: '<S233>/Product1' incorporates:
   *  Saturate: '<S233>/Saturation'
   *  Sum: '<S233>/Add'
   */
  Chassis_B.Product1_k[2] = (Gykappa[2] - rtb_Integrator_g[2]) * u0;

  /* Saturate: '<S235>/Saturation1' incorporates:
   *  Saturate: '<S232>/Saturation1'
   */
  if (rtb_sig_x[2] > Chassis_P.Saturation1_UpperSat_g) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat_g;
  } else if (rtb_sig_x[2] < Chassis_P.Saturation1_LowerSat_a) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat_a;
  } else {
    Drphi_0 = rtb_sig_x[2];
  }

  /* Product: '<S235>/Product3' incorporates:
   *  Saturate: '<S235>/Saturation1'
   */
  u0 = rtb_Subtract1_idx_1 / Drphi_0;

  /* Saturate: '<S235>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_o) {
    u0 = Chassis_P.Saturation_UpperSat_o;
  } else if (u0 < Chassis_P.Saturation_LowerSat_o) {
    u0 = Chassis_P.Saturation_LowerSat_o;
  }

  /* Product: '<S235>/Product1' incorporates:
   *  Saturate: '<S235>/Saturation'
   *  Sum: '<S235>/Add'
   */
  Chassis_B.Product1_o[2] = (rtb_My[2] - rtb_Integrator_c[2]) * u0;

  /* Switch: '<S230>/Switch' incorporates:
   *  Constant: '<S230>/Constant'
   */
  if (rtb_Sum6[2] > Chassis_P.Switch_Threshold) {
    Drphi_0 = rtb_Switch[2];
  } else {
    Drphi_0 = Chassis_P.Constant_Value_d;
  }

  /* Gain: '<S230>/Gain1' incorporates:
   *  Gain: '<S230>/Gain2'
   *  Sum: '<S230>/Sum2'
   *  Switch: '<S230>/Switch'
   */
  Chassis_B.Gain1[2] = (((rtb_xdot[2] + rtb_UnaryMinus_i) - rtb_Saturation_h2[2]) -
                        Chassis_P.CombinedSlipWheel2DOF_VERTICAL_DAMPING * Drphi_0) *
                       rtb_pqr_a;

  /* Product: '<S232>/Product2' incorporates:
   *  Product: '<S233>/Product2'
   */
  rtb_Subtract1_idx_0 = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[3] * Chassis_B.Re[3];

  /* Saturate: '<S232>/Saturation1' */
  if (rtb_sig_x[3] > Chassis_P.Saturation1_UpperSat) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat;
  } else if (rtb_sig_x[3] < Chassis_P.Saturation1_LowerSat) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat;
  } else {
    Drphi_0 = rtb_sig_x[3];
  }

  /* Abs: '<S232>/Abs' incorporates:
   *  Abs: '<S235>/Abs'
   *  Sum: '<S232>/Add1'
   */
  rtb_Subtract1_idx_1 = std::abs(rtb_ImpAsg_InsertedFor_xdotWheel_at_inport_0[3] - rtb_Subtract1_idx_0);

  /* Product: '<S232>/Product3' incorporates:
   *  Abs: '<S232>/Abs'
   *  Saturate: '<S232>/Saturation1'
   */
  u0 = rtb_Subtract1_idx_1 / Drphi_0;

  /* Saturate: '<S232>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_d) {
    u0 = Chassis_P.Saturation_UpperSat_d;
  } else if (u0 < Chassis_P.Saturation_LowerSat_f) {
    u0 = Chassis_P.Saturation_LowerSat_f;
  }

  /* Product: '<S232>/Product1' incorporates:
   *  Saturate: '<S232>/Saturation'
   *  Sum: '<S232>/Add'
   */
  Chassis_B.Product1[3] = (lam_mux[3] - rtb_Integrator[3]) * u0;

  /* Saturate: '<S233>/Saturation1' */
  if (rtb_sig_y[3] > Chassis_P.Saturation1_UpperSat_k) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat_k;
  } else if (rtb_sig_y[3] < Chassis_P.Saturation1_LowerSat_f) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat_f;
  } else {
    Drphi_0 = rtb_sig_y[3];
  }

  /* Product: '<S233>/Product3' incorporates:
   *  Abs: '<S233>/Abs'
   *  Saturate: '<S233>/Saturation1'
   *  Sum: '<S233>/Add1'
   */
  u0 = std::abs(rtb_ImpAsg_InsertedFor_ydotWheel_at_inport_0[3] - rtb_Subtract1_idx_0) / Drphi_0;

  /* Saturate: '<S233>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_k) {
    u0 = Chassis_P.Saturation_UpperSat_k;
  } else if (u0 < Chassis_P.Saturation_LowerSat_p) {
    u0 = Chassis_P.Saturation_LowerSat_p;
  }

  /* Product: '<S233>/Product1' incorporates:
   *  Saturate: '<S233>/Saturation'
   *  Sum: '<S233>/Add'
   */
  Chassis_B.Product1_k[3] = (Gykappa[3] - rtb_Integrator_g[3]) * u0;

  /* Saturate: '<S235>/Saturation1' incorporates:
   *  Saturate: '<S232>/Saturation1'
   */
  if (rtb_sig_x[3] > Chassis_P.Saturation1_UpperSat_g) {
    Drphi_0 = Chassis_P.Saturation1_UpperSat_g;
  } else if (rtb_sig_x[3] < Chassis_P.Saturation1_LowerSat_a) {
    Drphi_0 = Chassis_P.Saturation1_LowerSat_a;
  } else {
    Drphi_0 = rtb_sig_x[3];
  }

  /* Product: '<S235>/Product3' incorporates:
   *  Saturate: '<S235>/Saturation1'
   */
  u0 = rtb_Subtract1_idx_1 / Drphi_0;

  /* Saturate: '<S235>/Saturation' */
  if (u0 > Chassis_P.Saturation_UpperSat_o) {
    u0 = Chassis_P.Saturation_UpperSat_o;
  } else if (u0 < Chassis_P.Saturation_LowerSat_o) {
    u0 = Chassis_P.Saturation_LowerSat_o;
  }

  /* Product: '<S235>/Product1' incorporates:
   *  Saturate: '<S235>/Saturation'
   *  Sum: '<S235>/Add'
   */
  Chassis_B.Product1_o[3] = (rtb_My[3] - rtb_Integrator_c[3]) * u0;

  /* Switch: '<S230>/Switch' incorporates:
   *  Constant: '<S230>/Constant'
   */
  if (rtb_Sum6[3] > Chassis_P.Switch_Threshold) {
    Drphi_0 = rtb_Switch[3];
  } else {
    Drphi_0 = Chassis_P.Constant_Value_d;
  }

  /* Gain: '<S230>/Gain1' incorporates:
   *  Gain: '<S230>/Gain2'
   *  Sum: '<S230>/Sum2'
   *  Switch: '<S230>/Switch'
   */
  Chassis_B.Gain1[3] =
      (((rtb_xdot[3] + rtb_UnaryMinus_i) - rtb_pqr_tmp) - Chassis_P.CombinedSlipWheel2DOF_VERTICAL_DAMPING * Drphi_0) *
      rtb_pqr_a;
  if (rtmIsMajorTimeStep((&Chassis_M))) {
    if (rtmIsMajorTimeStep((&Chassis_M))) {
      /* Update for Memory: '<S141>/Memory1' */
      Chassis_DW.Memory1_PreviousInput[0] = Chassis_B.phithetapsi[0];
      Chassis_DW.Memory1_PreviousInput[1] = Chassis_B.phithetapsi[1];
      Chassis_DW.Memory1_PreviousInput[2] = Chassis_B.phithetapsi[2];

      /* Update for Memory: '<S239>/Memory' */
      Chassis_DW.Memory_PreviousInput[0] = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[0];
      Chassis_DW.Memory_PreviousInput[1] = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[1];
      Chassis_DW.Memory_PreviousInput[2] = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[2];
      Chassis_DW.Memory_PreviousInput[3] = Chassis_B.ImpAsg_InsertedFor_Omega_at_inport_0[3];
    }
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&Chassis_M))) {
    rt_ertODEUpdateContinuousStates(&(&Chassis_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&Chassis_M)->Timing.clockTick0)) {
      ++(&Chassis_M)->Timing.clockTickH0;
    }

    (&Chassis_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&Chassis_M)->solverInfo);

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
      (&Chassis_M)->Timing.clockTick1++;
      if (!(&Chassis_M)->Timing.clockTick1) {
        (&Chassis_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void Chassis::Chassis_derivatives() {
  /* local scratch DWork variables */
  int32_T ForEach_itr_o;
  XDot_Chassis_T *_rtXdot;
  _rtXdot = ((XDot_Chassis_T *)(&Chassis_M)->derivs);

  /* Derivatives for Integrator: '<S109>/xe,ye,ze' */
  _rtXdot->xeyeze_CSTATE[0] = Chassis_B.Product[0];

  /* Derivatives for Integrator: '<S115>/phi theta psi' */
  _rtXdot->phithetapsi_CSTATE[0] = Chassis_B.TmpSignalConversionAtphithetapsiInport1[0];

  /* Derivatives for Integrator: '<S109>/ub,vb,wb' */
  _rtXdot->ubvbwb_CSTATE[0] = Chassis_B.Sum[0];

  /* Derivatives for Integrator: '<S109>/p,q,r ' */
  _rtXdot->pqr_CSTATE[0] = Chassis_B.Product2[0];

  /* Derivatives for Integrator: '<S109>/xe,ye,ze' */
  _rtXdot->xeyeze_CSTATE[1] = Chassis_B.Product[1];

  /* Derivatives for Integrator: '<S115>/phi theta psi' */
  _rtXdot->phithetapsi_CSTATE[1] = Chassis_B.TmpSignalConversionAtphithetapsiInport1[1];

  /* Derivatives for Integrator: '<S109>/ub,vb,wb' */
  _rtXdot->ubvbwb_CSTATE[1] = Chassis_B.Sum[1];

  /* Derivatives for Integrator: '<S109>/p,q,r ' */
  _rtXdot->pqr_CSTATE[1] = Chassis_B.Product2[1];

  /* Derivatives for Integrator: '<S109>/xe,ye,ze' */
  _rtXdot->xeyeze_CSTATE[2] = Chassis_B.Product[2];

  /* Derivatives for Integrator: '<S115>/phi theta psi' */
  _rtXdot->phithetapsi_CSTATE[2] = Chassis_B.TmpSignalConversionAtphithetapsiInport1[2];

  /* Derivatives for Integrator: '<S109>/ub,vb,wb' */
  _rtXdot->ubvbwb_CSTATE[2] = Chassis_B.Sum[2];

  /* Derivatives for Integrator: '<S109>/p,q,r ' */
  _rtXdot->pqr_CSTATE[2] = Chassis_B.Product2[2];

  /* Derivatives for Integrator: '<S95>/Integrator1' */
  _rtXdot->Integrator1_CSTATE[0] = Chassis_B.Divide_e[0];
  _rtXdot->Integrator1_CSTATE[1] = Chassis_B.Divide_e[1];

  /* Derivatives for SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  if (Chassis_DW.IntegratorSecondOrder_MODE[0] == 0) {
    _rtXdot->IntegratorSecondOrder_CSTATE[0] = Chassis_X.IntegratorSecondOrder_CSTATE[4];
    _rtXdot->IntegratorSecondOrder_CSTATE[4] = Chassis_B.Gain1[0];
  }

  /* Derivatives for Integrator: '<S232>/Integrator' */
  _rtXdot->Integrator_CSTATE[0] = Chassis_B.Product1[0];

  /* Derivatives for Integrator: '<S233>/Integrator' */
  _rtXdot->Integrator_CSTATE_c[0] = Chassis_B.Product1_k[0];

  /* Derivatives for Integrator: '<S210>/Integrator1' */
  _rtXdot->Integrator1_CSTATE_h[0] = Chassis_B.Divide_c[0];

  /* Derivatives for Integrator: '<S235>/Integrator' */
  _rtXdot->Integrator_CSTATE_a[0] = Chassis_B.Product1_o[0];

  /* Derivatives for SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  if (Chassis_DW.IntegratorSecondOrder_MODE[1] == 0) {
    _rtXdot->IntegratorSecondOrder_CSTATE[1] = Chassis_X.IntegratorSecondOrder_CSTATE[5];
    _rtXdot->IntegratorSecondOrder_CSTATE[5] = Chassis_B.Gain1[1];
  }

  /* Derivatives for Integrator: '<S232>/Integrator' */
  _rtXdot->Integrator_CSTATE[1] = Chassis_B.Product1[1];

  /* Derivatives for Integrator: '<S233>/Integrator' */
  _rtXdot->Integrator_CSTATE_c[1] = Chassis_B.Product1_k[1];

  /* Derivatives for Integrator: '<S210>/Integrator1' */
  _rtXdot->Integrator1_CSTATE_h[1] = Chassis_B.Divide_c[1];

  /* Derivatives for Integrator: '<S235>/Integrator' */
  _rtXdot->Integrator_CSTATE_a[1] = Chassis_B.Product1_o[1];

  /* Derivatives for SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  if (Chassis_DW.IntegratorSecondOrder_MODE[2] == 0) {
    _rtXdot->IntegratorSecondOrder_CSTATE[2] = Chassis_X.IntegratorSecondOrder_CSTATE[6];
    _rtXdot->IntegratorSecondOrder_CSTATE[6] = Chassis_B.Gain1[2];
  }

  /* Derivatives for Integrator: '<S232>/Integrator' */
  _rtXdot->Integrator_CSTATE[2] = Chassis_B.Product1[2];

  /* Derivatives for Integrator: '<S233>/Integrator' */
  _rtXdot->Integrator_CSTATE_c[2] = Chassis_B.Product1_k[2];

  /* Derivatives for Integrator: '<S210>/Integrator1' */
  _rtXdot->Integrator1_CSTATE_h[2] = Chassis_B.Divide_c[2];

  /* Derivatives for Integrator: '<S235>/Integrator' */
  _rtXdot->Integrator_CSTATE_a[2] = Chassis_B.Product1_o[2];

  /* Derivatives for SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
  if (Chassis_DW.IntegratorSecondOrder_MODE[3] == 0) {
    _rtXdot->IntegratorSecondOrder_CSTATE[3] = Chassis_X.IntegratorSecondOrder_CSTATE[7];
    _rtXdot->IntegratorSecondOrder_CSTATE[7] = Chassis_B.Gain1[3];
  }

  /* Derivatives for Integrator: '<S232>/Integrator' */
  _rtXdot->Integrator_CSTATE[3] = Chassis_B.Product1[3];

  /* Derivatives for Integrator: '<S233>/Integrator' */
  _rtXdot->Integrator_CSTATE_c[3] = Chassis_B.Product1_k[3];

  /* Derivatives for Integrator: '<S210>/Integrator1' */
  _rtXdot->Integrator1_CSTATE_h[3] = Chassis_B.Divide_c[3];

  /* Derivatives for Integrator: '<S235>/Integrator' */
  _rtXdot->Integrator_CSTATE_a[3] = Chassis_B.Product1_o[3];

  /* Derivatives for Iterator SubSystem: '<S237>/Clutch Scalar Parameters' */
  for (ForEach_itr_o = 0; ForEach_itr_o < 4; ForEach_itr_o++) {
    /* Derivatives for Iterator SubSystem: '<S237>/Clutch Scalar Parameters' */
    /* Derivatives for Chart: '<S241>/Clutch' */
    Chassis_Clutch_Deriv(&Chassis_B.CoreSubsys_d0[ForEach_itr_o].sf_Clutch,
                         &Chassis_DW.CoreSubsys_d0[ForEach_itr_o].sf_Clutch,
                         &_rtXdot->CoreSubsys_d0[ForEach_itr_o].sf_Clutch);

    /* End of Derivatives for SubSystem: '<S237>/Clutch Scalar Parameters' */
  }

  /* End of Derivatives for SubSystem: '<S237>/Clutch Scalar Parameters' */
  /* Derivatives for Integrator: '<S211>/Integrator1' incorporates:
   *  Product: '<S211>/Divide'
   */
  std::memcpy(&_rtXdot->Integrator1_CSTATE_p[0], &Chassis_B.Divide_m[0], 12U * sizeof(real_T));

  /* Derivatives for Integrator: '<S7>/Integrator1' incorporates:
   *  Product: '<S7>/Divide'
   */
  _rtXdot->Integrator1_CSTATE_a[0] = Chassis_B.Divide[0];

  /* Derivatives for Integrator: '<S8>/Integrator1' incorporates:
   *  Product: '<S8>/Divide'
   */
  _rtXdot->Integrator1_CSTATE_o[0] = Chassis_B.Divide_h[0];

  /* Derivatives for Integrator: '<S7>/Integrator1' incorporates:
   *  Product: '<S7>/Divide'
   */
  _rtXdot->Integrator1_CSTATE_a[1] = Chassis_B.Divide[1];

  /* Derivatives for Integrator: '<S8>/Integrator1' incorporates:
   *  Product: '<S8>/Divide'
   */
  _rtXdot->Integrator1_CSTATE_o[1] = Chassis_B.Divide_h[1];

  /* Derivatives for Integrator: '<S7>/Integrator1' incorporates:
   *  Product: '<S7>/Divide'
   */
  _rtXdot->Integrator1_CSTATE_a[2] = Chassis_B.Divide[2];

  /* Derivatives for Integrator: '<S8>/Integrator1' incorporates:
   *  Product: '<S8>/Divide'
   */
  _rtXdot->Integrator1_CSTATE_o[2] = Chassis_B.Divide_h[2];

  /* Derivatives for Integrator: '<S7>/Integrator1' incorporates:
   *  Product: '<S7>/Divide'
   */
  _rtXdot->Integrator1_CSTATE_a[3] = Chassis_B.Divide[3];

  /* Derivatives for Integrator: '<S8>/Integrator1' incorporates:
   *  Product: '<S8>/Divide'
   */
  _rtXdot->Integrator1_CSTATE_o[3] = Chassis_B.Divide_h[3];

  /* Derivatives for Integrator: '<S113>/Integrator' */
  _rtXdot->Integrator_CSTATE_o[0] = Chassis_B.ubvbwb[0];
  _rtXdot->Integrator_CSTATE_o[1] = Chassis_B.ubvbwb[1];
  _rtXdot->Integrator_CSTATE_o[2] = Chassis_B.ubvbwb[2];
}

/* Model initialize function */
void Chassis::initialize() {
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* non-finite (run-time) assignments */
  Chassis_P.Saturation_UpperSat = rtInf;
  Chassis_P.Saturation_UpperSat_c = rtInf;
  Chassis_P.Saturation_UpperSat_g = rtInf;
  Chassis_P.DisallowNegativeBrakeTorque_UpperSat = rtInf;
  Chassis_P.Saturation1_UpperSat = rtInf;
  Chassis_P.Saturation_UpperSat_d = rtInf;
  Chassis_P.Saturation1_UpperSat_k = rtInf;
  Chassis_P.Saturation_UpperSat_k = rtInf;
  Chassis_P.Saturation1_UpperSat_g = rtInf;
  Chassis_P.Saturation_UpperSat_o = rtInf;

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&Chassis_M)->solverInfo, &(&Chassis_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&Chassis_M)->solverInfo, &rtmGetTPtr((&Chassis_M)));
    rtsiSetStepSizePtr(&(&Chassis_M)->solverInfo, &(&Chassis_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&Chassis_M)->solverInfo, &(&Chassis_M)->derivs);
    rtsiSetContStatesPtr(&(&Chassis_M)->solverInfo, (real_T **)&(&Chassis_M)->contStates);
    rtsiSetNumContStatesPtr(&(&Chassis_M)->solverInfo, &(&Chassis_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&Chassis_M)->solverInfo, &(&Chassis_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&Chassis_M)->solverInfo, &(&Chassis_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&Chassis_M)->solverInfo, &(&Chassis_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&Chassis_M)->solverInfo, (&rtmGetErrorStatus((&Chassis_M))));
    rtsiSetRTModelPtr(&(&Chassis_M)->solverInfo, (&Chassis_M));
  }

  rtsiSetSimTimeStep(&(&Chassis_M)->solverInfo, MAJOR_TIME_STEP);
  (&Chassis_M)->intgData.y = (&Chassis_M)->odeY;
  (&Chassis_M)->intgData.f[0] = (&Chassis_M)->odeF[0];
  (&Chassis_M)->intgData.f[1] = (&Chassis_M)->odeF[1];
  (&Chassis_M)->intgData.f[2] = (&Chassis_M)->odeF[2];
  (&Chassis_M)->intgData.f[3] = (&Chassis_M)->odeF[3];
  (&Chassis_M)->contStates = ((X_Chassis_T *)&Chassis_X);
  (&Chassis_M)->periodicContStateIndices = ((int_T *)Chassis_PeriodicIndX);
  (&Chassis_M)->periodicContStateRanges = ((real_T *)Chassis_PeriodicRngX);
  rtsiSetSolverData(&(&Chassis_M)->solverInfo, static_cast<void *>(&(&Chassis_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&Chassis_M)->solverInfo, false);
  rtsiSetSolverName(&(&Chassis_M)->solverInfo, "ode4");
  rtmSetTPtr((&Chassis_M), &(&Chassis_M)->Timing.tArray[0]);
  (&Chassis_M)->Timing.stepSize0 = 0.001;
  Chassis_PrevZCX.FunctionCallSubsystem_Trig_ZCE[0] = POS_ZCSIG;
  Chassis_PrevZCX.FunctionCallSubsystem_Trig_ZCE[1] = POS_ZCSIG;
  Chassis_PrevZCX.FunctionCallSubsystem_Trig_ZCE[2] = POS_ZCSIG;

  {
    /* local scratch DWork variables */
    int32_T ForEach_itr_o;
    real_T tmp;

    /* InitializeConditions for Integrator: '<S109>/xe,ye,ze' */
    Chassis_X.xeyeze_CSTATE[0] = Chassis_P.VehicleBody6DOF_Xe_o[0];

    /* InitializeConditions for Integrator: '<S115>/phi theta psi' */
    Chassis_X.phithetapsi_CSTATE[0] = Chassis_P.VehicleBody6DOF_eul_o[0];

    /* InitializeConditions for Integrator: '<S109>/ub,vb,wb' */
    Chassis_X.ubvbwb_CSTATE[0] = Chassis_P.VehicleBody6DOF_xbdot_o[0];

    /* InitializeConditions for Memory: '<S141>/Memory1' */
    Chassis_DW.Memory1_PreviousInput[0] = Chassis_P.Memory1_InitialCondition;

    /* InitializeConditions for Integrator: '<S109>/p,q,r ' */
    Chassis_X.pqr_CSTATE[0] = Chassis_P.VehicleBody6DOF_p_o[0];

    /* InitializeConditions for Integrator: '<S109>/xe,ye,ze' */
    Chassis_X.xeyeze_CSTATE[1] = Chassis_P.VehicleBody6DOF_Xe_o[1];

    /* InitializeConditions for Integrator: '<S115>/phi theta psi' */
    Chassis_X.phithetapsi_CSTATE[1] = Chassis_P.VehicleBody6DOF_eul_o[1];

    /* InitializeConditions for Integrator: '<S109>/ub,vb,wb' */
    Chassis_X.ubvbwb_CSTATE[1] = Chassis_P.VehicleBody6DOF_xbdot_o[1];

    /* InitializeConditions for Memory: '<S141>/Memory1' */
    Chassis_DW.Memory1_PreviousInput[1] = Chassis_P.Memory1_InitialCondition;

    /* InitializeConditions for Integrator: '<S109>/p,q,r ' */
    Chassis_X.pqr_CSTATE[1] = Chassis_P.VehicleBody6DOF_p_o[1];

    /* InitializeConditions for Integrator: '<S109>/xe,ye,ze' */
    Chassis_X.xeyeze_CSTATE[2] = Chassis_P.VehicleBody6DOF_Xe_o[2];

    /* InitializeConditions for Integrator: '<S115>/phi theta psi' */
    Chassis_X.phithetapsi_CSTATE[2] = Chassis_P.VehicleBody6DOF_eul_o[2];

    /* InitializeConditions for Integrator: '<S109>/ub,vb,wb' */
    Chassis_X.ubvbwb_CSTATE[2] = Chassis_P.VehicleBody6DOF_xbdot_o[2];

    /* InitializeConditions for Memory: '<S141>/Memory1' */
    Chassis_DW.Memory1_PreviousInput[2] = Chassis_P.Memory1_InitialCondition;

    /* InitializeConditions for Integrator: '<S109>/p,q,r ' */
    Chassis_X.pqr_CSTATE[2] = Chassis_P.VehicleBody6DOF_p_o[2];

    /* InitializeConditions for Integrator: '<S95>/Integrator1' */
    Chassis_X.Integrator1_CSTATE[0] = Chassis_P.Integrator1_IC;
    Chassis_X.Integrator1_CSTATE[1] = Chassis_P.Integrator1_IC;

    /* InitializeConditions for SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
    Chassis_X.IntegratorSecondOrder_CSTATE[0] = -(Chassis_P.VEH.InitialVertPosition);
    Chassis_X.IntegratorSecondOrder_CSTATE[1] = -(Chassis_P.VEH.InitialVertPosition);
    Chassis_X.IntegratorSecondOrder_CSTATE[2] = -(Chassis_P.VEH.InitialVertPosition);
    Chassis_X.IntegratorSecondOrder_CSTATE[3] = -(Chassis_P.VEH.InitialVertPosition);

    /* InitializeConditions for Memory: '<S239>/Memory' */
    tmp = Chassis_P.VEH.InitialLongVel / Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS;

    /* InitializeConditions for SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
    Chassis_X.IntegratorSecondOrder_CSTATE[4] = Chassis_P.CombinedSlipWheel2DOF_zdoto;

    /* InitializeConditions for Integrator: '<S232>/Integrator' */
    Chassis_X.Integrator_CSTATE[0] = Chassis_P.Integrator_IC;

    /* InitializeConditions for Integrator: '<S233>/Integrator' */
    Chassis_X.Integrator_CSTATE_c[0] = Chassis_P.Integrator_IC_h;

    /* InitializeConditions for Integrator: '<S210>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_h[0] = Chassis_P.Integrator1_IC_d;

    /* InitializeConditions for Integrator: '<S235>/Integrator' */
    Chassis_X.Integrator_CSTATE_a[0] = Chassis_P.Integrator_IC_a;

    /* InitializeConditions for Memory: '<S239>/Memory' */
    Chassis_DW.Memory_PreviousInput[0] = tmp;

    /* InitializeConditions for SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
    Chassis_X.IntegratorSecondOrder_CSTATE[5] = Chassis_P.CombinedSlipWheel2DOF_zdoto;

    /* InitializeConditions for Integrator: '<S232>/Integrator' */
    Chassis_X.Integrator_CSTATE[1] = Chassis_P.Integrator_IC;

    /* InitializeConditions for Integrator: '<S233>/Integrator' */
    Chassis_X.Integrator_CSTATE_c[1] = Chassis_P.Integrator_IC_h;

    /* InitializeConditions for Integrator: '<S210>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_h[1] = Chassis_P.Integrator1_IC_d;

    /* InitializeConditions for Integrator: '<S235>/Integrator' */
    Chassis_X.Integrator_CSTATE_a[1] = Chassis_P.Integrator_IC_a;

    /* InitializeConditions for Memory: '<S239>/Memory' */
    Chassis_DW.Memory_PreviousInput[1] = tmp;

    /* InitializeConditions for SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
    Chassis_X.IntegratorSecondOrder_CSTATE[6] = Chassis_P.CombinedSlipWheel2DOF_zdoto;

    /* InitializeConditions for Integrator: '<S232>/Integrator' */
    Chassis_X.Integrator_CSTATE[2] = Chassis_P.Integrator_IC;

    /* InitializeConditions for Integrator: '<S233>/Integrator' */
    Chassis_X.Integrator_CSTATE_c[2] = Chassis_P.Integrator_IC_h;

    /* InitializeConditions for Integrator: '<S210>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_h[2] = Chassis_P.Integrator1_IC_d;

    /* InitializeConditions for Integrator: '<S235>/Integrator' */
    Chassis_X.Integrator_CSTATE_a[2] = Chassis_P.Integrator_IC_a;

    /* InitializeConditions for Memory: '<S239>/Memory' */
    Chassis_DW.Memory_PreviousInput[2] = tmp;

    /* InitializeConditions for SecondOrderIntegrator: '<S230>/Integrator, Second-Order' */
    Chassis_X.IntegratorSecondOrder_CSTATE[7] = Chassis_P.CombinedSlipWheel2DOF_zdoto;

    /* InitializeConditions for Integrator: '<S232>/Integrator' */
    Chassis_X.Integrator_CSTATE[3] = Chassis_P.Integrator_IC;

    /* InitializeConditions for Integrator: '<S233>/Integrator' */
    Chassis_X.Integrator_CSTATE_c[3] = Chassis_P.Integrator_IC_h;

    /* InitializeConditions for Integrator: '<S210>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_h[3] = Chassis_P.Integrator1_IC_d;

    /* InitializeConditions for Integrator: '<S235>/Integrator' */
    Chassis_X.Integrator_CSTATE_a[3] = Chassis_P.Integrator_IC_a;

    /* InitializeConditions for Memory: '<S239>/Memory' */
    Chassis_DW.Memory_PreviousInput[3] = tmp;

    /* InitializeConditions for Integrator: '<S211>/Integrator1' */
    for (int32_T i{0}; i < 12; i++) {
      Chassis_X.Integrator1_CSTATE_p[i] = Chassis_P.Integrator1_IC_l;
    }

    /* End of InitializeConditions for Integrator: '<S211>/Integrator1' */

    /* InitializeConditions for Integrator: '<S7>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_a[0] = Chassis_P.Integrator1_IC_j;

    /* InitializeConditions for Integrator: '<S8>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_o[0] = Chassis_P.Integrator1_IC_g;

    /* InitializeConditions for Integrator: '<S7>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_a[1] = Chassis_P.Integrator1_IC_j;

    /* InitializeConditions for Integrator: '<S8>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_o[1] = Chassis_P.Integrator1_IC_g;

    /* InitializeConditions for Integrator: '<S7>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_a[2] = Chassis_P.Integrator1_IC_j;

    /* InitializeConditions for Integrator: '<S8>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_o[2] = Chassis_P.Integrator1_IC_g;

    /* InitializeConditions for Integrator: '<S7>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_a[3] = Chassis_P.Integrator1_IC_j;

    /* InitializeConditions for Integrator: '<S8>/Integrator1' */
    Chassis_X.Integrator1_CSTATE_o[3] = Chassis_P.Integrator1_IC_g;

    /* InitializeConditions for Integrator: '<S113>/Integrator' */
    Chassis_X.Integrator_CSTATE_o[0] = Chassis_P.Integrator_IC_e;

    /* SystemInitialize for Triggered SubSystem: '<S141>/Function-Call Subsystem' */
    /* InitializeConditions for UnitDelay: '<S143>/Unit Delay' */
    Chassis_DW.UnitDelay_DSTATE[0] = Chassis_P.UnitDelay_InitialCondition;

    /* SystemInitialize for Sum: '<S143>/Sum' incorporates:
     *  Outport: '<S143>/Out1'
     */
    Chassis_B.Sum_j[0] = Chassis_P.Out1_Y0;

    /* End of SystemInitialize for SubSystem: '<S141>/Function-Call Subsystem' */

    /* InitializeConditions for Integrator: '<S113>/Integrator' */
    Chassis_X.Integrator_CSTATE_o[1] = Chassis_P.Integrator_IC_e;

    /* SystemInitialize for Triggered SubSystem: '<S141>/Function-Call Subsystem' */
    /* InitializeConditions for UnitDelay: '<S143>/Unit Delay' */
    Chassis_DW.UnitDelay_DSTATE[1] = Chassis_P.UnitDelay_InitialCondition;

    /* SystemInitialize for Sum: '<S143>/Sum' incorporates:
     *  Outport: '<S143>/Out1'
     */
    Chassis_B.Sum_j[1] = Chassis_P.Out1_Y0;

    /* End of SystemInitialize for SubSystem: '<S141>/Function-Call Subsystem' */

    /* InitializeConditions for Integrator: '<S113>/Integrator' */
    Chassis_X.Integrator_CSTATE_o[2] = Chassis_P.Integrator_IC_e;

    /* SystemInitialize for Triggered SubSystem: '<S141>/Function-Call Subsystem' */
    /* InitializeConditions for UnitDelay: '<S143>/Unit Delay' */
    Chassis_DW.UnitDelay_DSTATE[2] = Chassis_P.UnitDelay_InitialCondition;

    /* SystemInitialize for Sum: '<S143>/Sum' incorporates:
     *  Outport: '<S143>/Out1'
     */
    Chassis_B.Sum_j[2] = Chassis_P.Out1_Y0;

    /* End of SystemInitialize for SubSystem: '<S141>/Function-Call Subsystem' */
    /* SystemInitialize for Iterator SubSystem: '<S237>/Clutch Scalar Parameters' */
    for (ForEach_itr_o = 0; ForEach_itr_o < 4; ForEach_itr_o++) {
      /* SystemInitialize for Iterator SubSystem: '<S237>/Clutch Scalar Parameters' */
      /* SystemInitialize for Chart: '<S241>/Clutch' */
      Chassis_Clutch_Init(Chassis_P.VEH.InitialLongVel / Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS,
                          &Chassis_B.CoreSubsys_d0[ForEach_itr_o].sf_Clutch,
                          &Chassis_DW.CoreSubsys_d0[ForEach_itr_o].sf_Clutch, &Chassis_P.CoreSubsys_d0.sf_Clutch,
                          &Chassis_X.CoreSubsys_d0[ForEach_itr_o].sf_Clutch);

      /* End of SystemInitialize for SubSystem: '<S237>/Clutch Scalar Parameters' */
    }

    /* End of SystemInitialize for SubSystem: '<S237>/Clutch Scalar Parameters' */

    /* InitializeConditions for root-level periodic continuous states */
    {
      int_T rootPeriodicContStateIndices[3]{3, 4, 5};

      real_T rootPeriodicContStateRanges[6]{-3.1415926535897931, 3.1415926535897931,  -3.1415926535897931,
                                            3.1415926535897931,  -3.1415926535897931, 3.1415926535897931};

      (void)std::memcpy((void *)Chassis_PeriodicIndX, rootPeriodicContStateIndices, 3 * sizeof(int_T));
      (void)std::memcpy((void *)Chassis_PeriodicRngX, rootPeriodicContStateRanges, 6 * sizeof(real_T));
    }
  }
}

/* Model terminate function */
void Chassis::terminate() { /* (no terminate code required) */
}

/* Constructor */
Chassis::Chassis() : Chassis_U(), Chassis_Y(), Chassis_B(), Chassis_DW(), Chassis_X(), Chassis_PrevZCX(), Chassis_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
Chassis::~Chassis() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_Chassis_T *Chassis::getRTM() { return (&Chassis_M); }
