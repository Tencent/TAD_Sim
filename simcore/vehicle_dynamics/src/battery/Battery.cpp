/*
 * Battery.cpp
 *
 * Code generation for model "Battery".
 *
 * Model version              : 1.36
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Thu Jun 16 17:51:30 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "Battery.h"
#include "Battery_capi.h"
#include "Battery_private.h"
#include "rtwtypes.h"

#include "car.pb.h"
#include "inc/proto_helper.h"

namespace tx_car {
namespace power {

real_T look1_binlcapw(real_T u0, const real_T bp0[], const real_T table[], uint32_T maxIndex) {
  real_T frac;
  real_T y;
  uint32_T iLeft;

  /* Column-major Lookup 1-D
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
    iLeft = maxIndex;
    frac = 0.0;
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Overflow mode: 'portable wrapping'
   */
  if (iLeft == maxIndex) {
    y = table[iLeft];
  } else {
    real_T yL_0d0;
    yL_0d0 = table[iLeft];
    y = (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
  }

  return y;
}

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

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void Battery::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  int_T nXc{1};

  rtsiSetSimTimeStep(si, MINOR_TIME_STEP);

  /* Save the state values at time t in y, we'll use x as ynew. */
  (void)std::memcpy(y, x, static_cast<uint_T>(nXc) * sizeof(real_T));

  /* Assumes that rtsiSetT and ModelOutputs are up-to-date */
  /* f0 = f(t,y) */
  rtsiSetdX(si, f0);
  Battery_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  Battery_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  Battery_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  Battery_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Model step function */
void Battery::step() {
  real_T rtb_Gain1_d;
  real_T rtb_Gain2;
  real_T rtb_Product;
  real_T rtb_Product_h;
  if (rtmIsMajorTimeStep((&Battery_M))) {
    /* set solver stop time */
    if (!((&Battery_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&Battery_M)->solverInfo,
                            (((&Battery_M)->Timing.clockTickH0 + 1) * (&Battery_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(&(&Battery_M)->solverInfo,
                            (((&Battery_M)->Timing.clockTick0 + 1) * (&Battery_M)->Timing.stepSize0 +
                             (&Battery_M)->Timing.clockTickH0 * (&Battery_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&Battery_M))) {
    (&Battery_M)->Timing.t[0] = rtsiGetT(&(&Battery_M)->solverInfo);
  }

  if (rtmIsMajorTimeStep((&Battery_M))) {
    /* Switch: '<S6>/Switch' incorporates:
     *  Constant: '<S2>/Constant1'
     */
    rtb_Product_h = Battery_P.BattInitSoc / 100.0 * Battery_P.BattChargeMax;
    if (rtb_Product_h > Battery_P.BattChargeMax) {
      /* Switch: '<S6>/Switch' incorporates:
       *  Constant: '<S6>/Constant1'
       */
      Battery_B.Switch = Battery_P.BattChargeMax;
    } else {
      /* Switch: '<S6>/Switch' */
      Battery_B.Switch = rtb_Product_h;
    }

    /* End of Switch: '<S6>/Switch' */
  }

  /* Integrator: '<S6>/Integrator Limited' */
  /* Limited  Integrator  */
  if (Battery_DW.IntegratorLimited_IWORK != 0) {
    Battery_X.IntegratorLimited_CSTATE = Battery_B.Switch;
  }

  if (Battery_X.IntegratorLimited_CSTATE >= Battery_P.BattChargeMax) {
    Battery_X.IntegratorLimited_CSTATE = Battery_P.BattChargeMax;
  } else if (Battery_X.IntegratorLimited_CSTATE <= Battery_P.IntegratorLimited_LowerSat) {
    Battery_X.IntegratorLimited_CSTATE = Battery_P.IntegratorLimited_LowerSat;
  }

  /* Product: '<S7>/Divide' incorporates:
   *  Constant: '<S7>/Constant1'
   *  Integrator: '<S6>/Integrator Limited'
   */
  rtb_Product_h = Battery_X.IntegratorLimited_CSTATE / Battery_P.BattChargeMax;

  /* Gain: '<S8>/Gain2' incorporates:
   *  Inport: '<Root>/batt_input'
   */
  rtb_Gain2 = 1.0 / Battery_P.Np * Battery_U.batt_input.batt_curr_A;

  /* Product: '<S8>/Product' incorporates:
   *  Inport: '<Root>/batt_input'
   *  Lookup_n-D: '<S8>/R'
   *  Product: '<S3>/Product'
   */
  rtb_Product = look2_binlcapw(Battery_U.batt_input.batt_temp_K, rtb_Product_h, Battery_P.BattTempBp,
                               Battery_P.CapSOCBp, Battery_P.RInt, Battery_P.R_maxIndex, m_resis_map_temp_num) *
                rtb_Gain2;

  /* Gain: '<S8>/Gain1' incorporates:
   *  Lookup_n-D: '<S8>/Em'
   *  Product: '<S3>/Product'
   *  Sum: '<S8>/Subtract'
   */
  rtb_Gain1_d =
      (look1_binlcapw(rtb_Product_h, Battery_P.CapLUTBp, Battery_P.Em, m_batt_ocv_map_num - 1) - rtb_Product) *
      Battery_P.Ns;

  /* Outport: '<Root>/batt_output' incorporates:
   *  BusCreator generated from: '<Root>/batt_output'
   *  Gain: '<Root>/Gain'
   *  Gain: '<Root>/Gain1'
   *  Gain: '<Root>/Gain2'
   *  Gain: '<S3>/Gain1'
   *  Gain: '<S8>/Gain3'
   *  Gain: '<S8>/Gain4'
   *  Inport: '<Root>/batt_input'
   *  Product: '<S3>/Product'
   *  Product: '<S8>/Product1'
   */
  Battery_Y.batt_output.batt_volt_V = rtb_Gain1_d;
  Battery_Y.batt_output.batt_soc_null = Battery_P.Gain2_Gain * rtb_Product_h;
  Battery_Y.batt_output.batt_pwr_W = Battery_U.batt_input.batt_curr_A * rtb_Gain1_d * Battery_P.Gain_Gain;
  Battery_Y.batt_output.batt_loss_pwr_W =
      rtb_Product * rtb_Gain2 * Battery_P.Ns * Battery_P.Np * Battery_P.Gain1_Gain * Battery_P.Gain1_Gain_c;

  /* Gain: '<S6>/Gain1' incorporates:
   *  Inport: '<Root>/batt_input'
   */
  Battery_B.Gain1 = -1.0 / (Battery_P.Np * 3600.0) * Battery_U.batt_input.batt_curr_A;
  if (rtmIsMajorTimeStep((&Battery_M))) {
    /* Update for Integrator: '<S6>/Integrator Limited' */
    Battery_DW.IntegratorLimited_IWORK = 0;
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&Battery_M))) {
    rt_ertODEUpdateContinuousStates(&(&Battery_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&Battery_M)->Timing.clockTick0)) {
      ++(&Battery_M)->Timing.clockTickH0;
    }

    (&Battery_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&Battery_M)->solverInfo);

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
      (&Battery_M)->Timing.clockTick1++;
      if (!(&Battery_M)->Timing.clockTick1) {
        (&Battery_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void Battery::Battery_derivatives() {
  XDot_Battery_T *_rtXdot;
  boolean_T lsat;
  boolean_T usat;
  _rtXdot = ((XDot_Battery_T *)(&Battery_M)->derivs);

  /* Derivatives for Integrator: '<S6>/Integrator Limited' */
  lsat = (Battery_X.IntegratorLimited_CSTATE <= Battery_P.IntegratorLimited_LowerSat);
  usat = (Battery_X.IntegratorLimited_CSTATE >= Battery_P.BattChargeMax);
  if (((!lsat) && (!usat)) || (lsat && (Battery_B.Gain1 > 0.0)) || (usat && (Battery_B.Gain1 < 0.0))) {
    _rtXdot->IntegratorLimited_CSTATE = Battery_B.Gain1;
  } else {
    /* in saturation */
    _rtXdot->IntegratorLimited_CSTATE = 0.0;
  }

  /* End of Derivatives for Integrator: '<S6>/Integrator Limited' */
}

/* Model initialize function */
void Battery::initialize() {
  /* Registration code */
  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&Battery_M)->solverInfo, &(&Battery_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&Battery_M)->solverInfo, &rtmGetTPtr((&Battery_M)));
    rtsiSetStepSizePtr(&(&Battery_M)->solverInfo, &(&Battery_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&Battery_M)->solverInfo, &(&Battery_M)->derivs);
    rtsiSetContStatesPtr(&(&Battery_M)->solverInfo, (real_T **)&(&Battery_M)->contStates);
    rtsiSetNumContStatesPtr(&(&Battery_M)->solverInfo, &(&Battery_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&Battery_M)->solverInfo, &(&Battery_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&Battery_M)->solverInfo, &(&Battery_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&Battery_M)->solverInfo, &(&Battery_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&Battery_M)->solverInfo, (&rtmGetErrorStatus((&Battery_M))));
    rtsiSetRTModelPtr(&(&Battery_M)->solverInfo, (&Battery_M));
  }

  rtsiSetSimTimeStep(&(&Battery_M)->solverInfo, MAJOR_TIME_STEP);
  (&Battery_M)->intgData.y = (&Battery_M)->odeY;
  (&Battery_M)->intgData.f[0] = (&Battery_M)->odeF[0];
  (&Battery_M)->intgData.f[1] = (&Battery_M)->odeF[1];
  (&Battery_M)->intgData.f[2] = (&Battery_M)->odeF[2];
  (&Battery_M)->intgData.f[3] = (&Battery_M)->odeF[3];
  (&Battery_M)->contStates = ((X_Battery_T *)&Battery_X);
  rtsiSetSolverData(&(&Battery_M)->solverInfo, static_cast<void *>(&(&Battery_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&Battery_M)->solverInfo, false);
  rtsiSetSolverName(&(&Battery_M)->solverInfo, "ode4");
  rtmSetTPtr((&Battery_M), &(&Battery_M)->Timing.tArray[0]);
  (&Battery_M)->Timing.stepSize0 = 0.001;
  rtmSetFirstInitCond((&Battery_M), 1);

  /* Initialize DataMapInfo substructure containing ModelMap for C API */
  Battery_InitializeDataMapInfo((&Battery_M), &Battery_P);

  /* InitializeConditions for Integrator: '<S6>/Integrator Limited' */
  if (rtmIsFirstInitCond((&Battery_M))) {
    Battery_X.IntegratorLimited_CSTATE = 0.0;
  }

  Battery_DW.IntegratorLimited_IWORK = 1;

  /* End of InitializeConditions for Integrator: '<S6>/Integrator Limited' */

  /* set "at time zero" to false */
  if (rtmIsFirstInitCond((&Battery_M))) {
    rtmSetFirstInitCond((&Battery_M), 0);
  }
}

/* Model terminate function */
void Battery::terminate() { /* (no terminate code required) */
}

/* Constructor */
Battery::Battery() : Battery_U(), Battery_Y(), Battery_B(), Battery_DW(), Battery_X(), Battery_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
Battery::~Battery() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_Battery_T *Battery::getRTM() { return (&Battery_M); }

P_Battery_T &Battery::getParamterStruct() {  // get the reference of battery parameter struct
  return Battery_P;
}

}  // namespace power
}  // namespace tx_car