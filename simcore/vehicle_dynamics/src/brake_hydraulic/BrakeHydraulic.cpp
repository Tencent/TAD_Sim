/*
 * BrakeHydraulic.cpp
 *
 * Code generation for model "BrakeHydraulic".
 *
 * Model version              : 2.37
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Thu May 11 21:00:45 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "BrakeHydraulic.h"
#include <cmath>
#include "BrakeHydraulic_private.h"
#include "rtwtypes.h"

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void BrakeHydraulic::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  BrakeHydraulic_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  BrakeHydraulic_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  BrakeHydraulic_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  BrakeHydraulic_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Model step function */
void BrakeHydraulic::step() {
  /* local block i/o variables */
  real_T rtb_Saturation[4];
  real_T rtb_fluidTransportDelay_idx_0;
  uint32_T rtb_Gain;
  if (rtmIsMajorTimeStep((&BrakeHydraulic_M))) {
    /* set solver stop time */
    if (!((&BrakeHydraulic_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&BrakeHydraulic_M)->solverInfo, (((&BrakeHydraulic_M)->Timing.clockTickH0 + 1) *
                                                               (&BrakeHydraulic_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(
          &(&BrakeHydraulic_M)->solverInfo,
          (((&BrakeHydraulic_M)->Timing.clockTick0 + 1) * (&BrakeHydraulic_M)->Timing.stepSize0 +
           (&BrakeHydraulic_M)->Timing.clockTickH0 * (&BrakeHydraulic_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&BrakeHydraulic_M))) {
    (&BrakeHydraulic_M)->Timing.t[0] = rtsiGetT(&(&BrakeHydraulic_M)->solverInfo);
  }

  if (rtmIsMajorTimeStep((&BrakeHydraulic_M))) {
    /* Outport: '<Root>/BrkPrs[Pa]' incorporates:
     *  Memory: '<Root>/Memory'
     */
    BrakeHydraulic_Y.BrkPrsPa[0] = BrakeHydraulic_DW.Memory_PreviousInput[0];
    BrakeHydraulic_Y.BrkPrsPa[1] = BrakeHydraulic_DW.Memory_PreviousInput[1];
    BrakeHydraulic_Y.BrkPrsPa[2] = BrakeHydraulic_DW.Memory_PreviousInput[2];
    BrakeHydraulic_Y.BrkPrsPa[3] = BrakeHydraulic_DW.Memory_PreviousInput[3];

    /* Gain: '<S1>/Gain' incorporates:
     *  Constant: '<S1>/Constant3'
     */
    rtb_fluidTransportDelay_idx_0 =
        std::floor(BrakeHydraulic_P.Gain_Gain * BrakeHydraulic_P.brake_fluid_transport_const_time);
    if (std::isnan(rtb_fluidTransportDelay_idx_0) || std::isinf(rtb_fluidTransportDelay_idx_0)) {
      rtb_fluidTransportDelay_idx_0 = 0.0;
    } else {
      rtb_fluidTransportDelay_idx_0 = std::fmod(rtb_fluidTransportDelay_idx_0, 4.294967296E+9);
    }

    rtb_Gain = rtb_fluidTransportDelay_idx_0 < 0.0
                   ? static_cast<uint32_T>(-static_cast<int32_T>(static_cast<uint32_T>(-rtb_fluidTransportDelay_idx_0)))
                   : static_cast<uint32_T>(rtb_fluidTransportDelay_idx_0);

    /* End of Gain: '<S1>/Gain' */
  }

  /* Outputs for Atomic SubSystem: '<S1>/Hydraulic' */
  /* Product: '<S3>/Divide' incorporates:
   *  Constant: '<S3>/Constant'
   *  Inport: '<Root>/BrkCmd[01]'
   *  Integrator: '<S3>/Integrator1'
   *  Sum: '<S3>/Sum'
   */
  BrakeHydraulic_B.Divide = (BrakeHydraulic_U.BrkCmd01 - BrakeHydraulic_X.Integrator1_CSTATE) *
                            (1.0 / BrakeHydraulic_P.brake_const_time_fluid);

  /* Product: '<S1>/Product1' incorporates:
   *  Constant: '<S1>/Constant1'
   *  Constant: '<S1>/Front Bias'
   *  Integrator: '<S3>/Integrator1'
   *  Product: '<S1>/Product'
   */
  rtb_fluidTransportDelay_idx_0 = BrakeHydraulic_P.FrontBias_Value * BrakeHydraulic_X.Integrator1_CSTATE *
                                  BrakeHydraulic_P.max_main_cylinder_pressure;

  /* End of Outputs for SubSystem: '<S1>/Hydraulic' */

  /* Product: '<S1>/Product1' */
  BrakeHydraulic_B.Product1[0] = rtb_fluidTransportDelay_idx_0;
  BrakeHydraulic_B.Product1[1] = rtb_fluidTransportDelay_idx_0;

  /* Outputs for Atomic SubSystem: '<S1>/Hydraulic' */
  /* Product: '<S1>/Product1' incorporates:
   *  Constant: '<S1>/Constant1'
   *  Constant: '<S1>/Rear Bias'
   *  Integrator: '<S3>/Integrator1'
   *  Product: '<S1>/Product'
   */
  rtb_fluidTransportDelay_idx_0 = BrakeHydraulic_P.rear_brake_ratio * BrakeHydraulic_X.Integrator1_CSTATE *
                                  BrakeHydraulic_P.max_main_cylinder_pressure;

  /* End of Outputs for SubSystem: '<S1>/Hydraulic' */

  /* Product: '<S1>/Product1' */
  BrakeHydraulic_B.Product1[2] = rtb_fluidTransportDelay_idx_0;
  BrakeHydraulic_B.Product1[3] = rtb_fluidTransportDelay_idx_0;
  if (rtmIsMajorTimeStep((&BrakeHydraulic_M))) {
    real_T rtb_fluidTransportDelay_idx_1;
    real_T rtb_fluidTransportDelay_idx_2;
    real_T rtb_fluidTransportDelay_idx_3;

    /* Delay: '<S1>/fluidTransportDelay' */
    if (rtb_Gain <= 0U) {
      rtb_fluidTransportDelay_idx_0 = BrakeHydraulic_B.Product1[0];
      rtb_fluidTransportDelay_idx_1 = BrakeHydraulic_B.Product1[1];
      rtb_fluidTransportDelay_idx_2 = BrakeHydraulic_B.Product1[2];
      rtb_fluidTransportDelay_idx_3 = BrakeHydraulic_B.Product1[3];
    } else {
      int_T rtb_Gain_0;
      if (rtb_Gain > 100U) {
        rtb_Gain_0 = 100;
      } else {
        rtb_Gain_0 = static_cast<int32_T>(rtb_Gain);
      }

      rtb_Gain_0 = static_cast<int32_T>(100U - static_cast<uint32_T>(rtb_Gain_0)) << 2;
      rtb_fluidTransportDelay_idx_0 = BrakeHydraulic_DW.fluidTransportDelay_DSTATE[rtb_Gain_0];
      rtb_fluidTransportDelay_idx_1 = BrakeHydraulic_DW.fluidTransportDelay_DSTATE[rtb_Gain_0 + 1];
      rtb_fluidTransportDelay_idx_2 = BrakeHydraulic_DW.fluidTransportDelay_DSTATE[rtb_Gain_0 + 2];
      rtb_fluidTransportDelay_idx_3 = BrakeHydraulic_DW.fluidTransportDelay_DSTATE[rtb_Gain_0 + 3];
    }

    /* End of Delay: '<S1>/fluidTransportDelay' */

    /* Saturate: '<S1>/Saturation' */
    if (rtb_fluidTransportDelay_idx_0 > BrakeHydraulic_P.max_main_cylinder_pressure) {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[0] = BrakeHydraulic_P.max_main_cylinder_pressure;
    } else if (rtb_fluidTransportDelay_idx_0 < BrakeHydraulic_P.Saturation_LowerSat) {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[0] = BrakeHydraulic_P.Saturation_LowerSat;
    } else {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[0] = rtb_fluidTransportDelay_idx_0;
    }

    if (rtb_fluidTransportDelay_idx_1 > BrakeHydraulic_P.max_main_cylinder_pressure) {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[1] = BrakeHydraulic_P.max_main_cylinder_pressure;
    } else if (rtb_fluidTransportDelay_idx_1 < BrakeHydraulic_P.Saturation_LowerSat) {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[1] = BrakeHydraulic_P.Saturation_LowerSat;
    } else {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[1] = rtb_fluidTransportDelay_idx_1;
    }

    if (rtb_fluidTransportDelay_idx_2 > BrakeHydraulic_P.max_main_cylinder_pressure) {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[2] = BrakeHydraulic_P.max_main_cylinder_pressure;
    } else if (rtb_fluidTransportDelay_idx_2 < BrakeHydraulic_P.Saturation_LowerSat) {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[2] = BrakeHydraulic_P.Saturation_LowerSat;
    } else {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[2] = rtb_fluidTransportDelay_idx_2;
    }

    if (rtb_fluidTransportDelay_idx_3 > BrakeHydraulic_P.max_main_cylinder_pressure) {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[3] = BrakeHydraulic_P.max_main_cylinder_pressure;
    } else if (rtb_fluidTransportDelay_idx_3 < BrakeHydraulic_P.Saturation_LowerSat) {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[3] = BrakeHydraulic_P.Saturation_LowerSat;
    } else {
      /* Saturate: '<S1>/Saturation' */
      rtb_Saturation[3] = rtb_fluidTransportDelay_idx_3;
    }

    /* End of Saturate: '<S1>/Saturation' */
  }

  if (rtmIsMajorTimeStep((&BrakeHydraulic_M))) {
    if (rtmIsMajorTimeStep((&BrakeHydraulic_M))) {
      /* Update for Memory: '<Root>/Memory' */
      BrakeHydraulic_DW.Memory_PreviousInput[0] = rtb_Saturation[0];
      BrakeHydraulic_DW.Memory_PreviousInput[1] = rtb_Saturation[1];
      BrakeHydraulic_DW.Memory_PreviousInput[2] = rtb_Saturation[2];
      BrakeHydraulic_DW.Memory_PreviousInput[3] = rtb_Saturation[3];

      /* Update for Delay: '<S1>/fluidTransportDelay' */
      for (int_T idxDelay{0}; idxDelay < 99; idxDelay++) {
        int32_T fluidTransportDelay_DSTATE_tmp;
        int32_T fluidTransportDelay_DSTATE_tmp_0;
        fluidTransportDelay_DSTATE_tmp = (idxDelay + 1) << 2;
        fluidTransportDelay_DSTATE_tmp_0 = idxDelay << 2;
        BrakeHydraulic_DW.fluidTransportDelay_DSTATE[fluidTransportDelay_DSTATE_tmp_0] =
            BrakeHydraulic_DW.fluidTransportDelay_DSTATE[fluidTransportDelay_DSTATE_tmp];
        BrakeHydraulic_DW.fluidTransportDelay_DSTATE[fluidTransportDelay_DSTATE_tmp_0 + 1] =
            BrakeHydraulic_DW.fluidTransportDelay_DSTATE[fluidTransportDelay_DSTATE_tmp + 1];
        BrakeHydraulic_DW.fluidTransportDelay_DSTATE[fluidTransportDelay_DSTATE_tmp_0 + 2] =
            BrakeHydraulic_DW.fluidTransportDelay_DSTATE[fluidTransportDelay_DSTATE_tmp + 2];
        BrakeHydraulic_DW.fluidTransportDelay_DSTATE[fluidTransportDelay_DSTATE_tmp_0 + 3] =
            BrakeHydraulic_DW.fluidTransportDelay_DSTATE[fluidTransportDelay_DSTATE_tmp + 3];
      }

      BrakeHydraulic_DW.fluidTransportDelay_DSTATE[396] = BrakeHydraulic_B.Product1[0];
      BrakeHydraulic_DW.fluidTransportDelay_DSTATE[397] = BrakeHydraulic_B.Product1[1];
      BrakeHydraulic_DW.fluidTransportDelay_DSTATE[398] = BrakeHydraulic_B.Product1[2];
      BrakeHydraulic_DW.fluidTransportDelay_DSTATE[399] = BrakeHydraulic_B.Product1[3];

      /* End of Update for Delay: '<S1>/fluidTransportDelay' */
    }
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&BrakeHydraulic_M))) {
    rt_ertODEUpdateContinuousStates(&(&BrakeHydraulic_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&BrakeHydraulic_M)->Timing.clockTick0)) {
      ++(&BrakeHydraulic_M)->Timing.clockTickH0;
    }

    (&BrakeHydraulic_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&BrakeHydraulic_M)->solverInfo);

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
      (&BrakeHydraulic_M)->Timing.clockTick1++;
      if (!(&BrakeHydraulic_M)->Timing.clockTick1) {
        (&BrakeHydraulic_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void BrakeHydraulic::BrakeHydraulic_derivatives() {
  XDot_BrakeHydraulic_T *_rtXdot;
  _rtXdot = ((XDot_BrakeHydraulic_T *)(&BrakeHydraulic_M)->derivs);

  /* Derivatives for Atomic SubSystem: '<S1>/Hydraulic' */
  /* Derivatives for Integrator: '<S3>/Integrator1' */
  _rtXdot->Integrator1_CSTATE = BrakeHydraulic_B.Divide;

  /* End of Derivatives for SubSystem: '<S1>/Hydraulic' */
}

/* Model initialize function */
void BrakeHydraulic::initialize() {
  /* Registration code */
  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&BrakeHydraulic_M)->solverInfo, &(&BrakeHydraulic_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&BrakeHydraulic_M)->solverInfo, &rtmGetTPtr((&BrakeHydraulic_M)));
    rtsiSetStepSizePtr(&(&BrakeHydraulic_M)->solverInfo, &(&BrakeHydraulic_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&BrakeHydraulic_M)->solverInfo, &(&BrakeHydraulic_M)->derivs);
    rtsiSetContStatesPtr(&(&BrakeHydraulic_M)->solverInfo, (real_T **)&(&BrakeHydraulic_M)->contStates);
    rtsiSetNumContStatesPtr(&(&BrakeHydraulic_M)->solverInfo, &(&BrakeHydraulic_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&BrakeHydraulic_M)->solverInfo,
                                    &(&BrakeHydraulic_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&BrakeHydraulic_M)->solverInfo,
                                       &(&BrakeHydraulic_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&BrakeHydraulic_M)->solverInfo, &(&BrakeHydraulic_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&BrakeHydraulic_M)->solverInfo, (&rtmGetErrorStatus((&BrakeHydraulic_M))));
    rtsiSetRTModelPtr(&(&BrakeHydraulic_M)->solverInfo, (&BrakeHydraulic_M));
  }

  rtsiSetSimTimeStep(&(&BrakeHydraulic_M)->solverInfo, MAJOR_TIME_STEP);
  (&BrakeHydraulic_M)->intgData.y = (&BrakeHydraulic_M)->odeY;
  (&BrakeHydraulic_M)->intgData.f[0] = (&BrakeHydraulic_M)->odeF[0];
  (&BrakeHydraulic_M)->intgData.f[1] = (&BrakeHydraulic_M)->odeF[1];
  (&BrakeHydraulic_M)->intgData.f[2] = (&BrakeHydraulic_M)->odeF[2];
  (&BrakeHydraulic_M)->intgData.f[3] = (&BrakeHydraulic_M)->odeF[3];
  (&BrakeHydraulic_M)->contStates = ((X_BrakeHydraulic_T *)&BrakeHydraulic_X);
  rtsiSetSolverData(&(&BrakeHydraulic_M)->solverInfo, static_cast<void *>(&(&BrakeHydraulic_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&BrakeHydraulic_M)->solverInfo, false);
  rtsiSetSolverName(&(&BrakeHydraulic_M)->solverInfo, "ode4");
  rtmSetTPtr((&BrakeHydraulic_M), &(&BrakeHydraulic_M)->Timing.tArray[0]);
  (&BrakeHydraulic_M)->Timing.stepSize0 = 0.001;

  /* InitializeConditions for Memory: '<Root>/Memory' */
  BrakeHydraulic_DW.Memory_PreviousInput[0] = BrakeHydraulic_P.Memory_InitialCondition;
  BrakeHydraulic_DW.Memory_PreviousInput[1] = BrakeHydraulic_P.Memory_InitialCondition;
  BrakeHydraulic_DW.Memory_PreviousInput[2] = BrakeHydraulic_P.Memory_InitialCondition;
  BrakeHydraulic_DW.Memory_PreviousInput[3] = BrakeHydraulic_P.Memory_InitialCondition;

  /* InitializeConditions for Delay: '<S1>/fluidTransportDelay' */
  for (int32_T i{0}; i < 400; i++) {
    BrakeHydraulic_DW.fluidTransportDelay_DSTATE[i] = BrakeHydraulic_P.fluidTransportDelay_InitialCondition;
  }

  /* End of InitializeConditions for Delay: '<S1>/fluidTransportDelay' */

  /* SystemInitialize for Atomic SubSystem: '<S1>/Hydraulic' */
  /* InitializeConditions for Integrator: '<S3>/Integrator1' */
  BrakeHydraulic_X.Integrator1_CSTATE = BrakeHydraulic_P.Integrator1_IC;

  /* End of SystemInitialize for SubSystem: '<S1>/Hydraulic' */
}

/* Model terminate function */
void BrakeHydraulic::terminate() { /* (no terminate code required) */
}

/* Constructor */
BrakeHydraulic::BrakeHydraulic()
    : BrakeHydraulic_U(),
      BrakeHydraulic_Y(),
      BrakeHydraulic_B(),
      BrakeHydraulic_DW(),
      BrakeHydraulic_X(),
      BrakeHydraulic_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
BrakeHydraulic::~BrakeHydraulic() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_BrakeHydraulic_T *BrakeHydraulic::getRTM() { return (&BrakeHydraulic_M); }
