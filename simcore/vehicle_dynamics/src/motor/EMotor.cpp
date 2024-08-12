/*
 * EMotor.cpp
 *
 * Code generation for model "EMotor".
 *
 * Model version              : 1.31
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Thu Sep 15 20:22:54 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "EMotor.h"
#include "EMotor_capi.h"
#include "EMotor_private.h"
#include "rtwtypes.h"

extern "C" {

#include "rt_nonfinite.h"
}
real_T look1_binlcapw(real_T u0, const real_T bp0[], const real_T table[], uint32_T maxIndex) {
  real_T frac;
  real_T y;
  uint32_T iLeft;

  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = 0.0;
  } else if (u0 < bp0[maxIndex]) {
    uint32_T bpIdx;
    uint32_T iRght;

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

  if (iLeft == maxIndex) {
    y = table[iLeft];
  } else {
    real_T yL_0d0;
    yL_0d0 = table[iLeft];
    y = (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
  }

  return y;
}

/*
real_T look1_binlcpw(real_T u0, const real_T bp0[], const real_T table[],
                   uint32_T maxIndex)
{
real_T frac;
real_T yL_0d0;
uint32_T iLeft;

if (u0 <= bp0[0U]) {
  iLeft = 0U;
  frac = 0.0;
} else if (u0 < bp0[maxIndex]) {
  uint32_T bpIdx;
  uint32_T iRght;

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

yL_0d0 = table[iLeft];
return (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
}

*/

/*
real_T look2_binlcapw(real_T u0, real_T u1, const real_T bp0[], const real_T
                      bp1[], const real_T table[], const uint32_T maxIndex[],
                      uint32_T stride)
{
  real_T fractions[2];
  real_T frac;
  real_T y;
  real_T yL_0d0;
  uint32_T bpIndices[2];
  uint32_T bpIdx;
  uint32_T iLeft;
  uint32_T iRght;

  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = 0.0;
  } else if (u0 < bp0[maxIndex[0U]]) {
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

  if (u1 <= bp1[0U]) {
    iLeft = 0U;
    frac = 0.0;
  } else if (u1 < bp1[maxIndex[1U]]) {
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
*/

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void EMotor::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  EMotor_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  EMotor_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  EMotor_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  EMotor_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Model step function */
void EMotor::step() {
  real_T rtb_Divide2;
  real_T rtb_Gain1;
  real_T rtb_uDLookupTable;
  real_T tmp;
  if (rtmIsMajorTimeStep((&EMotor_M))) {
    /* set solver stop time */
    if (!((&EMotor_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&EMotor_M)->solverInfo,
                            (((&EMotor_M)->Timing.clockTickH0 + 1) * (&EMotor_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(&(&EMotor_M)->solverInfo,
                            (((&EMotor_M)->Timing.clockTick0 + 1) * (&EMotor_M)->Timing.stepSize0 +
                             (&EMotor_M)->Timing.clockTickH0 * (&EMotor_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&EMotor_M))) {
    (&EMotor_M)->Timing.t[0] = rtsiGetT(&(&EMotor_M)->solverInfo);
  }

  /* Gain: '<Root>/GainPRM' incorporates:
   *  Inport: '<Root>/mot_in'
   */
  rtb_uDLookupTable = EMotor_P.GainPRM_Gain * EMotor_U.mot_in.mot_spd_RPM;

  /* Lookup_n-D: '<S6>/Max Torque Limit' incorporates:
   *  Lookup_n-D: '<S5>/2-D Lookup Table'
   */
  rtb_Divide2 = look1_binlcapw(rtb_uDLookupTable, EMotor_P.MaxTorqueLimit_bp01Data, EMotor_P.MaxTorqueLimit_tableData,
                               m_mot_trq_map_num - 1);

  /* Gain: '<S6>/Gain1' incorporates:
   *  Integrator: '<S6>/Integrator'
   */
  rtb_Gain1 = 1.0 / EMotor_P.mot_tc_s * EMotor_X.Integrator_CSTATE;

  /* Product: '<S6>/Divide2' incorporates:
   *  Lookup_n-D: '<S6>/Interpolated zero-crossing'
   *  Product: '<S6>/Product'
   */
  rtb_Divide2 *= look1_binlcpw(1.0 / rtb_Divide2 * rtb_Gain1, EMotor_P.Interpolatedzerocrossing_bp01Data,
                               EMotor_P.Interpolatedzerocrossing_tableData, 1U);

  /* Saturate: '<S3>/Saturation' incorporates:
   *  Inport: '<Root>/mot_in'
   */
  if (EMotor_U.mot_in.batt_volt_V > EMotor_P.Saturation_UpperSat) {
    tmp = EMotor_P.Saturation_UpperSat;
  } else if (EMotor_U.mot_in.batt_volt_V < EMotor_P.Saturation_LowerSat) {
    tmp = EMotor_P.Saturation_LowerSat;
  } else {
    tmp = EMotor_U.mot_in.batt_volt_V;
  }

  /* End of Saturate: '<S3>/Saturation' */

  /* Outport: '<Root>/mot_out' incorporates:
   *  BusCreator generated from: '<Root>/mot_out'
   *  Lookup_n-D: '<S5>/2-D Lookup Table'
   *  Product: '<S3>/Divide'
   *  Product: '<S5>/Product'
   *  Product: '<S6>/Divide2'
   *  Sum: '<S3>/Add'
   */
  // add by dongyuanhu
  real32_T pwr = rtb_uDLookupTable * rtb_Divide2;
  int signOfPwr = pwr < 0.0 ? -1 : 1;
  real32_T pwrLoss =
      look2_binlcapw(rtb_uDLookupTable, rtb_Divide2, EMotor_P.uDLookupTable_bp01Data, EMotor_P.uDLookupTable_bp02Data,
                     EMotor_P.uDLookupTable_tableData, EMotor_P.uDLookupTable_maxIndex, m_mot_loss_map_spd_num) *
      signOfPwr;
  EMotor_Y.mot_out.batt_curr_A = (pwr + pwrLoss) / tmp;

  /*
  EMotor_Y.mot_out.batt_curr_A = (rtb_uDLookupTable * rtb_Divide2 +
    look2_binlcapw(rtb_uDLookupTable, rtb_Divide2,
                   EMotor_P.uDLookupTable_bp01Data,
                   EMotor_P.uDLookupTable_bp02Data,
                   EMotor_P.uDLookupTable_tableData,
                   EMotor_P.uDLookupTable_maxIndex, m_mot_loss_map_spd_num)) / tmp;
  */
  EMotor_Y.mot_out.mot_trq_Nm = rtb_Divide2;

  /* Sum: '<S6>/Sum' incorporates:
   *  Inport: '<Root>/mot_in'
   */
  EMotor_B.Sum = EMotor_U.mot_in.trq_cmd_Nm - rtb_Gain1;
  if (rtmIsMajorTimeStep((&EMotor_M))) {
    rt_ertODEUpdateContinuousStates(&(&EMotor_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&EMotor_M)->Timing.clockTick0)) {
      ++(&EMotor_M)->Timing.clockTickH0;
    }

    (&EMotor_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&EMotor_M)->solverInfo);

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
      (&EMotor_M)->Timing.clockTick1++;
      if (!(&EMotor_M)->Timing.clockTick1) {
        (&EMotor_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void EMotor::EMotor_derivatives() {
  XDot_EMotor_T *_rtXdot;
  _rtXdot = ((XDot_EMotor_T *)(&EMotor_M)->derivs);

  /* Derivatives for Integrator: '<S6>/Integrator' */
  _rtXdot->Integrator_CSTATE = EMotor_B.Sum;
}

/* Model initialize function */
void EMotor::initialize() {
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* non-finite (run-time) assignments */
  EMotor_P.Saturation_UpperSat = rtInf;

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&EMotor_M)->solverInfo, &(&EMotor_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&EMotor_M)->solverInfo, &rtmGetTPtr((&EMotor_M)));
    rtsiSetStepSizePtr(&(&EMotor_M)->solverInfo, &(&EMotor_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&EMotor_M)->solverInfo, &(&EMotor_M)->derivs);
    rtsiSetContStatesPtr(&(&EMotor_M)->solverInfo, (real_T **)&(&EMotor_M)->contStates);
    rtsiSetNumContStatesPtr(&(&EMotor_M)->solverInfo, &(&EMotor_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&EMotor_M)->solverInfo, &(&EMotor_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&EMotor_M)->solverInfo, &(&EMotor_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&EMotor_M)->solverInfo, &(&EMotor_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&EMotor_M)->solverInfo, (&rtmGetErrorStatus((&EMotor_M))));
    rtsiSetRTModelPtr(&(&EMotor_M)->solverInfo, (&EMotor_M));
  }

  rtsiSetSimTimeStep(&(&EMotor_M)->solverInfo, MAJOR_TIME_STEP);
  (&EMotor_M)->intgData.y = (&EMotor_M)->odeY;
  (&EMotor_M)->intgData.f[0] = (&EMotor_M)->odeF[0];
  (&EMotor_M)->intgData.f[1] = (&EMotor_M)->odeF[1];
  (&EMotor_M)->intgData.f[2] = (&EMotor_M)->odeF[2];
  (&EMotor_M)->intgData.f[3] = (&EMotor_M)->odeF[3];
  (&EMotor_M)->contStates = ((X_EMotor_T *)&EMotor_X);
  rtsiSetSolverData(&(&EMotor_M)->solverInfo, static_cast<void *>(&(&EMotor_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&EMotor_M)->solverInfo, false);
  rtsiSetSolverName(&(&EMotor_M)->solverInfo, "ode4");
  rtmSetTPtr((&EMotor_M), &(&EMotor_M)->Timing.tArray[0]);
  (&EMotor_M)->Timing.stepSize0 = 0.001;

  /* Initialize DataMapInfo substructure containing ModelMap for C API */
  EMotor_InitializeDataMapInfo((&EMotor_M), &EMotor_P);

  /* InitializeConditions for Integrator: '<S6>/Integrator' */
  EMotor_X.Integrator_CSTATE = EMotor_P.Integrator_IC;
}

/* Model terminate function */
void EMotor::terminate() { /* (no terminate code required) */
}

/* Constructor */
EMotor::EMotor() : EMotor_U(), EMotor_Y(), EMotor_B(), EMotor_X(), EMotor_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
EMotor::~EMotor() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_EMotor_T *EMotor::getRTM() { return (&EMotor_M); }
