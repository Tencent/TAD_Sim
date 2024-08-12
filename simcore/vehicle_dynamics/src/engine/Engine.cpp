/*
 * Engine.cpp
 *
 * Code generation for model "Engine".
 *
 * Model version              : 2.21
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Dec  6 13:01:57 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "Engine.h"
#include "Engine_private.h"
#include "rtwtypes.h"

extern "C" {

#include "rt_nonfinite.h"
}

real_T look2_binlcpw(real_T u0, real_T u1, const real_T bp0[], const real_T bp1[], const real_T table[],
                     const uint32_T maxIndex[], uint32_T stride) {
  real_T fractions[2];
  real_T frac;
  real_T yL_0d0;
  real_T yL_0d1;
  uint32_T bpIndices[2];
  uint32_T bpIdx;
  uint32_T iLeft;
  uint32_T iRght;

  /* Column-major Lookup 2-D
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
    iLeft = maxIndex[0U] - 1U;
    frac = 1.0;
  }

  fractions[0U] = frac;
  bpIndices[0U] = iLeft;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
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
    iLeft = maxIndex[1U] - 1U;
    frac = 1.0;
  }

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'portable wrapping'
   */
  bpIdx = iLeft * stride + bpIndices[0U];
  yL_0d0 = table[bpIdx];
  yL_0d0 += (table[bpIdx + 1U] - yL_0d0) * fractions[0U];
  bpIdx += stride;
  yL_0d1 = table[bpIdx];
  return (((table[bpIdx + 1U] - yL_0d1) * fractions[0U] + yL_0d1) - yL_0d0) * frac + yL_0d0;
}

real_T look2_binlxpw(real_T u0, real_T u1, const real_T bp0[], const real_T bp1[], const real_T table[],
                     const uint32_T maxIndex[], uint32_T stride) {
  real_T fractions[2];
  real_T frac;
  real_T yL_0d0;
  real_T yL_0d1;
  uint32_T bpIndices[2];
  uint32_T bpIdx;
  uint32_T iLeft;
  uint32_T iRght;

  /* Column-major Lookup 2-D
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
    iLeft = maxIndex[0U] - 1U;
    frac = (u0 - bp0[maxIndex[0U] - 1U]) / (bp0[maxIndex[0U]] - bp0[maxIndex[0U] - 1U]);
  }

  fractions[0U] = frac;
  bpIndices[0U] = iLeft;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Linear'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u1 <= bp1[0U]) {
    iLeft = 0U;
    frac = (u1 - bp1[0U]) / (bp1[1U] - bp1[0U]);
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
    iLeft = maxIndex[1U] - 1U;
    frac = (u1 - bp1[maxIndex[1U] - 1U]) / (bp1[maxIndex[1U]] - bp1[maxIndex[1U] - 1U]);
  }

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'portable wrapping'
   */
  bpIdx = iLeft * stride + bpIndices[0U];
  yL_0d0 = table[bpIdx];
  yL_0d0 += (table[bpIdx + 1U] - yL_0d0) * fractions[0U];
  bpIdx += stride;
  yL_0d1 = table[bpIdx];
  return (((table[bpIdx + 1U] - yL_0d1) * fractions[0U] + yL_0d1) - yL_0d0) * frac + yL_0d0;
}

/* State reduction function */
void local_stateReduction(real_T *x, int_T *p, int_T n, real_T *r);

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void Engine::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  int_T nXc{3};

  rtsiSetSimTimeStep(si, MINOR_TIME_STEP);

  /* Save the state values at time t in y, we'll use x as ynew. */
  (void)std::memcpy(y, x, static_cast<uint_T>(nXc) * sizeof(real_T));

  /* Assumes that rtsiSetT and ModelOutputs are up-to-date */
  /* f0 = f(t,y) */
  rtsiSetdX(si, f0);
  Engine_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  Engine_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  Engine_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  Engine_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  local_stateReduction(rtsiGetContStates(si), rtsiGetPeriodicContStateIndices(si), 1,
                       rtsiGetPeriodicContStateRanges(si));
  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Model step function */
void Engine::step() {
  real_T rtb_act_trq_Nm;
  if (rtmIsMajorTimeStep((&Engine_M))) {
    /* set solver stop time */
    if (!((&Engine_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&Engine_M)->solverInfo,
                            (((&Engine_M)->Timing.clockTickH0 + 1) * (&Engine_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(&(&Engine_M)->solverInfo,
                            (((&Engine_M)->Timing.clockTick0 + 1) * (&Engine_M)->Timing.stepSize0 +
                             (&Engine_M)->Timing.clockTickH0 * (&Engine_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&Engine_M))) {
    (&Engine_M)->Timing.t[0] = rtsiGetT(&(&Engine_M)->solverInfo);
  }

  /* Gain: '<S2>/Gain' incorporates:
   *  Inport: '<Root>/EngIn'
   */
  rtb_act_trq_Nm = Engine_P.Gain_Gain * Engine_U.EngIn.eng_spd_rpm;

  /* Lookup_n-D: '<S8>/FuelFlw Table' incorporates:
   *  Integrator: '<S13>/Integrator1'
   *  Lookup_n-D: '<S8>/2-D Lookup Table'
   */
  Engine_B.FuelFlwTable =
      look2_binlcapw(Engine_X.Integrator1_CSTATE, rtb_act_trq_Nm, Engine_P.FuelFlwTable_bp01Data,
                     Engine_P.f_tbrake_n_bpt, Engine_P.f_fuel, Engine_P.FuelFlwTable_maxIndex, m_eng_trq_req_num);

  /* Integrator: '<S1>/Integrator' */
  /* Limited  Integrator  */
  if (Engine_X.Integrator_CSTATE >= Engine_P.Integrator_UpperSat) {
    Engine_X.Integrator_CSTATE = Engine_P.Integrator_UpperSat;
  } else if (Engine_X.Integrator_CSTATE <= Engine_P.Integrator_LowerSat) {
    Engine_X.Integrator_CSTATE = Engine_P.Integrator_LowerSat;
  }

  /* Outport: '<Root>/EngOut' incorporates:
   *  Constant: '<S1>/Constant'
   *  Constant: '<S3>/Constant'
   *  Gain: '<S1>/Gain'
   *  Gain: '<S7>/rpm to rad//s'
   *  Integrator: '<S13>/Integrator1'
   *  Integrator: '<S1>/Integrator'
   *  Product: '<S1>/Divide'
   *  Product: '<S7>/Product'
   *  Sum: '<S3>/Net Torque'
   */
  Engine_Y.EngOut.eng_trq_Nm = Engine_X.Integrator1_CSTATE - Engine_P.Constant_Value;
  Engine_Y.EngOut.pow_out_W = Engine_P.rpmtorads_Gain * rtb_act_trq_Nm * Engine_X.Integrator1_CSTATE;
  Engine_Y.EngOut.fuel_rate_g_s = Engine_P.Gain_Gain_e * Engine_B.FuelFlwTable;
  Engine_Y.EngOut.fuel_cosume_L = Engine_X.Integrator_CSTATE / Engine_P.fuelDensity;

  /* Gain: '<S5>/RPM to deg//s' */
  Engine_B.RPMtodegs = Engine_P.RPMtodegs_Gain * rtb_act_trq_Nm;

  /* Product: '<S13>/Divide' incorporates:
   *  Constant: '<S13>/Constant'
   *  Inport: '<Root>/EngIn'
   *  Integrator: '<S13>/Integrator1'
   *  Lookup_n-D: '<S8>/2-D Lookup Table'
   *  Sum: '<S13>/Sum'
   */
  Engine_B.Divide =
      (look2_binlcapw(Engine_U.EngIn.throttle_01, rtb_act_trq_Nm, Engine_P.f_tbrake_t_bpt, Engine_P.f_tbrake_n_bpt,
                      Engine_P.f_tbrake, Engine_P.uDLookupTable_maxIndex, m_throttle_num) -
       Engine_X.Integrator1_CSTATE) *
      (1.0 / Engine_P.ConstTimeEng);
  if (rtmIsMajorTimeStep((&Engine_M))) {
    rt_ertODEUpdateContinuousStates(&(&Engine_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&Engine_M)->Timing.clockTick0)) {
      ++(&Engine_M)->Timing.clockTickH0;
    }

    (&Engine_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&Engine_M)->solverInfo);

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
      (&Engine_M)->Timing.clockTick1++;
      if (!(&Engine_M)->Timing.clockTick1) {
        (&Engine_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void Engine::Engine_derivatives() {
  XDot_Engine_T *_rtXdot;
  boolean_T lsat;
  boolean_T usat;
  _rtXdot = ((XDot_Engine_T *)(&Engine_M)->derivs);

  /* Derivatives for Integrator: '<S13>/Integrator1' */
  _rtXdot->Integrator1_CSTATE = Engine_B.Divide;

  /* Derivatives for Integrator: '<S1>/Integrator' */
  lsat = (Engine_X.Integrator_CSTATE <= Engine_P.Integrator_LowerSat);
  usat = (Engine_X.Integrator_CSTATE >= Engine_P.Integrator_UpperSat);
  if (((!lsat) && (!usat)) || (lsat && (Engine_B.FuelFlwTable > 0.0)) || (usat && (Engine_B.FuelFlwTable < 0.0))) {
    _rtXdot->Integrator_CSTATE = Engine_B.FuelFlwTable;
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE = 0.0;
  }

  /* End of Derivatives for Integrator: '<S1>/Integrator' */

  /* Derivatives for Integrator: '<S5>/Integrator' */
  _rtXdot->Integrator_CSTATE_h = Engine_B.RPMtodegs;
}

/* Model initialize function */
void Engine::initialize() {
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* non-finite (run-time) assignments */
  Engine_P.Integrator_UpperSat = rtInf;

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&Engine_M)->solverInfo, &(&Engine_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&Engine_M)->solverInfo, &rtmGetTPtr((&Engine_M)));
    rtsiSetStepSizePtr(&(&Engine_M)->solverInfo, &(&Engine_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&Engine_M)->solverInfo, &(&Engine_M)->derivs);
    rtsiSetContStatesPtr(&(&Engine_M)->solverInfo, (real_T **)&(&Engine_M)->contStates);
    rtsiSetNumContStatesPtr(&(&Engine_M)->solverInfo, &(&Engine_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&Engine_M)->solverInfo, &(&Engine_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&Engine_M)->solverInfo, &(&Engine_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&Engine_M)->solverInfo, &(&Engine_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&Engine_M)->solverInfo, (&rtmGetErrorStatus((&Engine_M))));
    rtsiSetRTModelPtr(&(&Engine_M)->solverInfo, (&Engine_M));
  }

  rtsiSetSimTimeStep(&(&Engine_M)->solverInfo, MAJOR_TIME_STEP);
  (&Engine_M)->intgData.y = (&Engine_M)->odeY;
  (&Engine_M)->intgData.f[0] = (&Engine_M)->odeF[0];
  (&Engine_M)->intgData.f[1] = (&Engine_M)->odeF[1];
  (&Engine_M)->intgData.f[2] = (&Engine_M)->odeF[2];
  (&Engine_M)->intgData.f[3] = (&Engine_M)->odeF[3];
  (&Engine_M)->contStates = ((X_Engine_T *)&Engine_X);
  (&Engine_M)->periodicContStateIndices = ((int_T *)Engine_PeriodicIndX);
  (&Engine_M)->periodicContStateRanges = ((real_T *)Engine_PeriodicRngX);
  rtsiSetSolverData(&(&Engine_M)->solverInfo, static_cast<void *>(&(&Engine_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&Engine_M)->solverInfo, false);
  rtsiSetSolverName(&(&Engine_M)->solverInfo, "ode4");
  rtmSetTPtr((&Engine_M), &(&Engine_M)->Timing.tArray[0]);
  (&Engine_M)->Timing.stepSize0 = 0.001;

  /* InitializeConditions for Integrator: '<S13>/Integrator1' */
  Engine_X.Integrator1_CSTATE = Engine_P.Integrator1_IC;

  /* InitializeConditions for Integrator: '<S1>/Integrator' */
  Engine_X.Integrator_CSTATE = Engine_P.Integrator_IC;

  /* InitializeConditions for Integrator: '<S5>/Integrator' */
  Engine_X.Integrator_CSTATE_h = Engine_P.Integrator_IC_o;

  /* InitializeConditions for root-level periodic continuous states */
  {
    int_T rootPeriodicContStateIndices[1]{2};

    real_T rootPeriodicContStateRanges[2]{0.0, 720.0};

    (void)std::memcpy((void *)Engine_PeriodicIndX, rootPeriodicContStateIndices, 1 * sizeof(int_T));
    (void)std::memcpy((void *)Engine_PeriodicRngX, rootPeriodicContStateRanges, 2 * sizeof(real_T));
  }
}

/* Model terminate function */
void Engine::terminate() { /* (no terminate code required) */
}

/* Constructor */
Engine::Engine() : Engine_U(), Engine_Y(), Engine_B(), Engine_X(), Engine_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
Engine::~Engine() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_Engine_T *Engine::getRTM() { return (&Engine_M); }
