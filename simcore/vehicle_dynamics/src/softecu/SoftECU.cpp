/*
 * SoftECU.cpp
 *
 * Code generation for model "SoftECU".
 *
 * Model version              : 2.90
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul  5 17:08:36 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "SoftECU.h"
#include <cmath>
#include "SoftECU_private.h"
#include "SoftECU_types.h"
#include "rtwtypes.h"

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void SoftECU::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  SoftECU_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  SoftECU_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  SoftECU_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  SoftECU_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* System initialize for atomic system: '<S1>/ABS_EBD' */
void SoftECU::SoftECU_ABS_EBD_Init() {
  /* Start for If: '<S2>/If' */
  SoftECU_DW.If_ActiveSubsystem_l = -1;

  /* InitializeConditions for Memory: '<S2>/Memory' */
  SoftECU_DW.Memory_PreviousInput[0] = SoftECU_P.Memory_InitialCondition;
  SoftECU_DW.Memory_PreviousInput[1] = SoftECU_P.Memory_InitialCondition;
  SoftECU_DW.Memory_PreviousInput[2] = SoftECU_P.Memory_InitialCondition;
  SoftECU_DW.Memory_PreviousInput[3] = SoftECU_P.Memory_InitialCondition;

  /* SystemInitialize for IfAction SubSystem: '<S2>/accControl' */
  /* InitializeConditions for DiscreteIntegrator: '<S38>/Filter' */
  SoftECU_DW.Filter_DSTATE_h = SoftECU_P.DiscretePIDController_InitialConditionForFilter;

  /* InitializeConditions for DiscreteIntegrator: '<S43>/Integrator' */
  SoftECU_DW.Integrator_DSTATE_f = SoftECU_P.DiscretePIDController_InitialConditionForIntegrator;

  /* End of SystemInitialize for SubSystem: '<S2>/accControl' */
}

/* Outputs for atomic system: '<S1>/ABS_EBD' */
void SoftECU::SoftECU_ABS_EBD() {
  real_T rtb_Merge;
  int8_T rtAction;
  if (rtmIsMajorTimeStep((&SoftECU_M))) {
    /* Memory: '<S2>/Memory' */
    SoftECU_B.Memory[0] = SoftECU_DW.Memory_PreviousInput[0];
    SoftECU_B.Memory[1] = SoftECU_DW.Memory_PreviousInput[1];
    SoftECU_B.Memory[2] = SoftECU_DW.Memory_PreviousInput[2];
    SoftECU_B.Memory[3] = SoftECU_DW.Memory_PreviousInput[3];
  }

  /* If: '<S2>/If' */
  if (rtsiIsModeUpdateTimeStep(&(&SoftECU_M)->solverInfo)) {
    rtAction = static_cast<int8_T>(SoftECU_B.brake_control_mode != 1);
    SoftECU_DW.If_ActiveSubsystem_l = rtAction;
  } else {
    rtAction = SoftECU_DW.If_ActiveSubsystem_l;
  }

  switch (rtAction) {
    case 0:
      /* Outputs for IfAction SubSystem: '<S2>/pressureControl' incorporates:
       *  ActionPort: '<S9>/Action Port'
       */
      if (rtmIsMajorTimeStep((&SoftECU_M))) {
        /* Merge: '<S2>/Merge1' incorporates:
         *  Constant: '<S66>/Constant'
         *  Constant: '<S9>/abs_brake_pressure_01'
         *  RelationalOperator: '<S66>/Compare'
         */
        SoftECU_B.Merge1 = (SoftECU_P.abs_brake_pressure >= SoftECU_P.CompareToConstant_const);
      }

      /* Switch: '<S9>/Switch' incorporates:
       *  Constant: '<S9>/abs_brake_pressure_01'
       */
      if (SoftECU_B.Memory[0] > SoftECU_P.Switch_Threshold) {
        rtb_Merge = SoftECU_P.abs_brake_pressure;
      } else {
        rtb_Merge = SoftECU_B.batt_soc_0_1;
      }

      /* End of Switch: '<S9>/Switch' */
      /* End of Outputs for SubSystem: '<S2>/pressureControl' */
      break;

    case 1: {
      boolean_T rtb_LogicalOperator2;

      /* Outputs for IfAction SubSystem: '<S2>/accControl' incorporates:
       *  ActionPort: '<S8>/Action Port'
       */
      /* Product: '<S8>/Divide' incorporates:
       *  Constant: '<S8>/Constant'
       *  Sum: '<S8>/Add'
       */
      rtb_Merge = (SoftECU_B.target_acc_m_s2 - SoftECU_B.request_torque) / SoftECU_P.ecu_max_dec;

      /* Logic: '<S11>/Logical Operator2' incorporates:
       *  Abs: '<S11>/Abs'
       *  Constant: '<S60>/Constant'
       *  Constant: '<S61>/Constant'
       *  Constant: '<S62>/Constant'
       *  Constant: '<S63>/Constant'
       *  Constant: '<S64>/Constant'
       *  Constant: '<S65>/Constant'
       *  Logic: '<S11>/Logical Operator'
       *  Logic: '<S11>/Logical Operator1'
       *  Logic: '<S11>/Logical Operator3'
       *  RelationalOperator: '<S60>/Compare'
       *  RelationalOperator: '<S61>/Compare'
       *  RelationalOperator: '<S62>/Compare'
       *  RelationalOperator: '<S63>/Compare'
       *  RelationalOperator: '<S64>/Compare'
       *  RelationalOperator: '<S65>/Compare'
       */
      rtb_LogicalOperator2 = (((SoftECU_B.veh_speed_vx < -SoftECU_P.speedOffset) &&
                               (SoftECU_B.gear_cmd__0N1D2R3P == SoftECU_P.reverseGear_const) &&
                               (SoftECU_B.target_acc_m_s2 > SoftECU_P.brakeEnableThresh_Reverse_const)) ||
                              ((SoftECU_B.gear_cmd__0N1D2R3P != SoftECU_P.notReverse_const) &&
                               (SoftECU_B.target_acc_m_s2 < SoftECU_P.brakeEnableThresh_const) &&
                               (std::abs(SoftECU_B.veh_speed_vx) > SoftECU_P.speedOffset)));

      /* Logic: '<S8>/Logical Operator' */
      SoftECU_B.LogicalOperator_i = !rtb_LogicalOperator2;
      if (rtmIsMajorTimeStep((&SoftECU_M))) {
        /* DiscreteIntegrator: '<S38>/Filter' */
        if (SoftECU_B.LogicalOperator_i || (SoftECU_DW.Filter_PrevResetState_c != 0)) {
          SoftECU_DW.Filter_DSTATE_h = SoftECU_P.DiscretePIDController_InitialConditionForFilter;
        }

        /* DiscreteIntegrator: '<S38>/Filter' */
        SoftECU_B.Filter_po = SoftECU_DW.Filter_DSTATE_h;

        /* DiscreteIntegrator: '<S43>/Integrator' */
        if (SoftECU_B.LogicalOperator_i || (SoftECU_DW.Integrator_PrevResetState_o != 0)) {
          SoftECU_DW.Integrator_DSTATE_f = SoftECU_P.DiscretePIDController_InitialConditionForIntegrator;
        }

        /* DiscreteIntegrator: '<S43>/Integrator' */
        SoftECU_B.Integrator_k = SoftECU_DW.Integrator_DSTATE_f;
      }

      /* Gain: '<S40>/Integral Gain' */
      SoftECU_B.IntegralGain_n = SoftECU_P.brake_I * rtb_Merge;

      /* Gain: '<S46>/Filter Coefficient' incorporates:
       *  Gain: '<S37>/Derivative Gain'
       *  Sum: '<S38>/SumD'
       */
      SoftECU_B.FilterCoefficient_i =
          (SoftECU_P.brake_D * rtb_Merge - SoftECU_B.Filter_po) * SoftECU_P.DiscretePIDController_N;

      /* Merge: '<S2>/Merge1' incorporates:
       *  SignalConversion generated from: '<S8>/brakeEnable'
       */
      SoftECU_B.Merge1 = rtb_LogicalOperator2;

      /* Switch: '<S8>/abs' incorporates:
       *  Constant: '<S8>/abs_brake_pressure_01'
       */
      if (SoftECU_B.Memory[0] > SoftECU_P.abs_Threshold) {
        rtb_Merge = SoftECU_P.abs_brake_pressure;
      } else {
        /* Sum: '<S52>/Sum' incorporates:
         *  Gain: '<S48>/Proportional Gain'
         */
        rtb_Merge = (SoftECU_P.brake_P * rtb_Merge + SoftECU_B.Integrator_k) + SoftECU_B.FilterCoefficient_i;

        /* Saturate: '<S50>/Saturation' */
        if (rtb_Merge > SoftECU_P.DiscretePIDController_UpperSaturationLimit) {
          rtb_Merge = SoftECU_P.DiscretePIDController_UpperSaturationLimit;
        } else if (rtb_Merge < SoftECU_P.DiscretePIDController_LowerSaturationLimit) {
          rtb_Merge = SoftECU_P.DiscretePIDController_LowerSaturationLimit;
        }

        /* End of Saturate: '<S50>/Saturation' */
      }

      /* Product: '<S8>/Product' incorporates:
       *  Switch: '<S8>/abs'
       */
      rtb_Merge *= static_cast<real_T>(rtb_LogicalOperator2);

      /* End of Outputs for SubSystem: '<S2>/accControl' */
    } break;
  }

  /* End of If: '<S2>/If' */

  /* Saturate: '<S2>/Saturation' */
  if (rtb_Merge > SoftECU_P.Saturation_UpperSat) {
    /* Merge: '<S1>/Merge1' */
    SoftECU_B.throttle_01 = SoftECU_P.Saturation_UpperSat;
  } else if (rtb_Merge < SoftECU_P.Saturation_LowerSat) {
    /* Merge: '<S1>/Merge1' */
    SoftECU_B.throttle_01 = SoftECU_P.Saturation_LowerSat;
  } else {
    /* Merge: '<S1>/Merge1' */
    SoftECU_B.throttle_01 = rtb_Merge;
  }

  /* End of Saturate: '<S2>/Saturation' */
}

/* Update for atomic system: '<S1>/ABS_EBD' */
void SoftECU::SoftECU_ABS_EBD_Update() {
  /* Abs: '<S2>/Abs' */
  SoftECU_B.Abs = std::abs(SoftECU_B.slip_ratio[0]);

  /* Relay: '<S2>/OnAtRef_OffAt0.5Ref' */
  if (rtsiIsModeUpdateTimeStep(&(&SoftECU_M)->solverInfo)) {
    SoftECU_DW.OnAtRef_OffAt05Ref_Mode =
        ((SoftECU_B.Abs >= SoftECU_P.slip_ratio_ref) ||
         ((!(SoftECU_B.Abs <= SoftECU_P.slip_disable_ratio_ref)) && SoftECU_DW.OnAtRef_OffAt05Ref_Mode));
  }

  if (SoftECU_DW.OnAtRef_OffAt05Ref_Mode) {
    /* Relay: '<S2>/OnAtRef_OffAt0.5Ref' */
    SoftECU_B.OnAtRef_OffAt05Ref = SoftECU_P.OnAtRef_OffAt05Ref_YOn;
  } else {
    /* Relay: '<S2>/OnAtRef_OffAt0.5Ref' */
    SoftECU_B.OnAtRef_OffAt05Ref = SoftECU_P.OnAtRef_OffAt05Ref_YOff;
  }

  /* End of Relay: '<S2>/OnAtRef_OffAt0.5Ref' */
  if (rtmIsMajorTimeStep((&SoftECU_M))) {
    /* Logic: '<S2>/if_on' incorporates:
     *  Constant: '<S2>/abs_switch'
     */
    SoftECU_B.abs_on = ((SoftECU_B.OnAtRef_OffAt05Ref != 0.0) && (SoftECU_P.enable_abs != 0.0));

    /* Product: '<S2>/ABS_flag' incorporates:
     *  Constant: '<S2>/Constant1'
     */
    SoftECU_B.ABS_flag[0] = static_cast<real_T>(SoftECU_B.abs_on) * SoftECU_P.Constant1_Value[0];
    SoftECU_B.ABS_flag[1] = static_cast<real_T>(SoftECU_B.abs_on) * SoftECU_P.Constant1_Value[1];
    SoftECU_B.ABS_flag[2] = static_cast<real_T>(SoftECU_B.abs_on) * SoftECU_P.Constant1_Value[2];
    SoftECU_B.ABS_flag[3] = static_cast<real_T>(SoftECU_B.abs_on) * SoftECU_P.Constant1_Value[3];
  }

  /* Product: '<S2>/Product1' */
  SoftECU_B.Product1[0] = static_cast<real_T>(SoftECU_B.Merge1) * SoftECU_B.ABS_flag[0];
  SoftECU_B.Product1[1] = static_cast<real_T>(SoftECU_B.Merge1) * SoftECU_B.ABS_flag[1];
  SoftECU_B.Product1[2] = static_cast<real_T>(SoftECU_B.Merge1) * SoftECU_B.ABS_flag[2];
  SoftECU_B.Product1[3] = static_cast<real_T>(SoftECU_B.Merge1) * SoftECU_B.ABS_flag[3];
  if (rtmIsMajorTimeStep((&SoftECU_M))) {
    /* Update for Memory: '<S2>/Memory' */
    SoftECU_DW.Memory_PreviousInput[0] = SoftECU_B.Product1[0];
    SoftECU_DW.Memory_PreviousInput[1] = SoftECU_B.Product1[1];
    SoftECU_DW.Memory_PreviousInput[2] = SoftECU_B.Product1[2];
    SoftECU_DW.Memory_PreviousInput[3] = SoftECU_B.Product1[3];
  }

  /* Update for If: '<S2>/If' */
  if (SoftECU_DW.If_ActiveSubsystem_l == 1) {
    /* Update for IfAction SubSystem: '<S2>/accControl' incorporates:
     *  ActionPort: '<S8>/Action Port'
     */
    if (rtmIsMajorTimeStep((&SoftECU_M))) {
      /* Update for DiscreteIntegrator: '<S38>/Filter' */
      SoftECU_DW.Filter_DSTATE_h += SoftECU_P.Filter_gainval * SoftECU_B.FilterCoefficient_i;
      SoftECU_DW.Filter_PrevResetState_c = static_cast<int8_T>(SoftECU_B.LogicalOperator_i);

      /* Update for DiscreteIntegrator: '<S43>/Integrator' incorporates:
       *  DiscreteIntegrator: '<S38>/Filter'
       */
      SoftECU_DW.Integrator_DSTATE_f += SoftECU_P.Integrator_gainval * SoftECU_B.IntegralGain_n;
      SoftECU_DW.Integrator_PrevResetState_o = static_cast<int8_T>(SoftECU_B.LogicalOperator_i);
    }

    /* End of Update for SubSystem: '<S2>/accControl' */
  }

  /* End of Update for If: '<S2>/If' */
}

/* System initialize for action system: '<S1>/EMS' */
void SoftECU::SoftECU_EMS_Init() {
  /* Start for If: '<S4>/If' */
  SoftECU_DW.If_ActiveSubsystem_g = -1;

  /* SystemInitialize for IfAction SubSystem: '<S4>/useTargetAccMode' */
  /* InitializeConditions for DiscreteIntegrator: '<S108>/Filter' */
  SoftECU_DW.Filter_DSTATE_i = SoftECU_P.DiscretePIDController_InitialConditionForFilter_e;

  /* InitializeConditions for DiscreteIntegrator: '<S113>/Integrator' */
  SoftECU_DW.Integrator_DSTATE_c = SoftECU_P.DiscretePIDController_InitialConditionForIntegrator_k;

  /* End of SystemInitialize for SubSystem: '<S4>/useTargetAccMode' */
}

/* Outputs for action system: '<S1>/EMS' */
void SoftECU::SoftECU_EMS() {
  real_T rtb_Switch;
  real_T u1;
  int8_T rtAction;
  boolean_T rtb_LogicalOperator_b;

  /* Logic: '<S70>/Logical Operator' incorporates:
   *  Constant: '<S76>/Constant'
   *  Constant: '<S77>/Constant'
   *  RelationalOperator: '<S76>/Compare'
   *  RelationalOperator: '<S77>/Compare'
   */
  rtb_LogicalOperator_b = ((SoftECU_B.brake_pressure_0_1 >= SoftECU_P.CompareToConstant1_const_j) ||
                           (SoftECU_B.Memory[0] >= SoftECU_P.CompareToConstant_const_f));

  /* Switch: '<S72>/Switch1' incorporates:
   *  Constant: '<S72>/fliper'
   *  Constant: '<S78>/Constant'
   *  Constant: '<S79>/Constant'
   *  RelationalOperator: '<S78>/Compare'
   *  RelationalOperator: '<S79>/Compare'
   */
  if (SoftECU_B.gear_cmd__0N1D2R3P == SoftECU_P.reverseGear1_const) {
    rtb_Switch = SoftECU_P.fliper_Value;
  } else {
    rtb_Switch = (SoftECU_B.gear_cmd__0N1D2R3P == SoftECU_P.forwardGear_const);
  }

  /* End of Switch: '<S72>/Switch1' */

  /* If: '<S4>/If' */
  if (rtsiIsModeUpdateTimeStep(&(&SoftECU_M)->solverInfo)) {
    if (SoftECU_B.acc_control_mode == 0) {
      rtAction = 0;
    } else if (SoftECU_B.acc_control_mode == 1) {
      rtAction = 1;
    } else {
      rtAction = 2;
    }

    SoftECU_DW.If_ActiveSubsystem_g = rtAction;
  } else {
    rtAction = SoftECU_DW.If_ActiveSubsystem_g;
  }

  switch (rtAction) {
    case 0:
      /* Outputs for IfAction SubSystem: '<S4>/useTargetAccMode' incorporates:
       *  ActionPort: '<S74>/Action Port'
       */
      /* Product: '<S74>/Divide' incorporates:
       *  Constant: '<S74>/Constant'
       *  Product: '<S4>/Product1'
       *  Product: '<S4>/Product2'
       *  Sum: '<S74>/acc_err'
       */
      rtb_Switch = (SoftECU_B.target_acc_m_s2 * rtb_Switch - SoftECU_B.ax_estimator.Integrator1 * rtb_Switch) /
                   SoftECU_P.ecu_max_acc;

      /* Logic: '<S74>/Logical Operator1' incorporates:
       *  Clock: '<S74>/Clock'
       *  Constant: '<S80>/Constant'
       *  RelationalOperator: '<S80>/Compare'
       */
      SoftECU_B.LogicalOperator1 = (rtb_LogicalOperator_b || SoftECU_B.LogicalOperator ||
                                    ((&SoftECU_M)->Timing.t[0] <= SoftECU_P.CompareToConstant1_const));
      if (rtmIsMajorTimeStep((&SoftECU_M))) {
        /* DiscreteIntegrator: '<S108>/Filter' */
        if (SoftECU_B.LogicalOperator1 || (SoftECU_DW.Filter_PrevResetState_p != 0)) {
          SoftECU_DW.Filter_DSTATE_i = SoftECU_P.DiscretePIDController_InitialConditionForFilter_e;
        }

        /* DiscreteIntegrator: '<S108>/Filter' */
        SoftECU_B.Filter_p = SoftECU_DW.Filter_DSTATE_i;

        /* DiscreteIntegrator: '<S113>/Integrator' */
        if (SoftECU_B.LogicalOperator1 || (SoftECU_DW.Integrator_PrevResetState_m != 0)) {
          SoftECU_DW.Integrator_DSTATE_c = SoftECU_P.DiscretePIDController_InitialConditionForIntegrator_k;
        }

        /* DiscreteIntegrator: '<S113>/Integrator' */
        SoftECU_B.Integrator_m = SoftECU_DW.Integrator_DSTATE_c;
      }

      /* Gain: '<S110>/Integral Gain' */
      SoftECU_B.IntegralGain_o = SoftECU_P.eng_acc_I * rtb_Switch;

      /* Gain: '<S116>/Filter Coefficient' incorporates:
       *  Gain: '<S107>/Derivative Gain'
       *  Sum: '<S108>/SumD'
       */
      SoftECU_B.FilterCoefficient_o =
          (SoftECU_P.eng_acc_D * rtb_Switch - SoftECU_B.Filter_p) * SoftECU_P.DiscretePIDController_N_i;

      /* Switch: '<S74>/Switch' incorporates:
       *  Constant: '<S74>/Constant2'
       */
      if (SoftECU_B.LogicalOperator1) {
        rtb_Switch = SoftECU_P.Constant2_Value;
      } else {
        /* Sum: '<S122>/Sum' incorporates:
         *  Gain: '<S118>/Proportional Gain'
         */
        rtb_Switch = (SoftECU_P.eng_acc_P * rtb_Switch + SoftECU_B.Integrator_m) + SoftECU_B.FilterCoefficient_o;

        /* Saturate: '<S120>/Saturation' */
        if (rtb_Switch > SoftECU_P.DiscretePIDController_UpperSaturationLimit_m) {
          rtb_Switch = SoftECU_P.DiscretePIDController_UpperSaturationLimit_m;
        } else if (rtb_Switch < SoftECU_P.DiscretePIDController_LowerSaturationLimit_o) {
          rtb_Switch = SoftECU_P.DiscretePIDController_LowerSaturationLimit_o;
        }

        /* End of Saturate: '<S120>/Saturation' */
      }

      /* End of Switch: '<S74>/Switch' */
      /* End of Outputs for SubSystem: '<S4>/useTargetAccMode' */
      break;

    case 1:
      /* Outputs for IfAction SubSystem: '<S4>/useTorqueRequestMode' incorporates:
       *  ActionPort: '<S75>/Action Port'
       */
      /* Product: '<S75>/Divide' incorporates:
       *  Constant: '<S75>/Constant'
       */
      rtb_Switch = SoftECU_B.request_torque / SoftECU_P.EngMaxTrq;

      /* End of Outputs for SubSystem: '<S4>/useTorqueRequestMode' */
      break;

    case 2:
      /* Outputs for IfAction SubSystem: '<S4>/throttleControlMode' incorporates:
       *  ActionPort: '<S73>/Action Port'
       */
      /* Saturate: '<S73>/Saturation' */
      if (SoftECU_B.request_torque > SoftECU_P.Saturation_UpperSat_l) {
        rtb_Switch = SoftECU_P.Saturation_UpperSat_l;
      } else if (SoftECU_B.request_torque < SoftECU_P.Saturation_LowerSat_j) {
        rtb_Switch = SoftECU_P.Saturation_LowerSat_j;
      } else {
        rtb_Switch = SoftECU_B.request_torque;
      }

      /* End of Saturate: '<S73>/Saturation' */
      /* End of Outputs for SubSystem: '<S4>/throttleControlMode' */
      break;
  }

  /* End of If: '<S4>/If' */

  /* Switch: '<S4>/Switch' incorporates:
   *  Constant: '<S4>/no_throttle'
   *  Constant: '<S71>/Constant'
   *  Logic: '<S4>/Logical Operator'
   *  RelationalOperator: '<S71>/Compare'
   */
  if (rtb_LogicalOperator_b && (rtb_Switch > SoftECU_P.CompareToConstant_const_b)) {
    rtb_Switch = SoftECU_P.no_throttle_Value;
  }

  /* End of Switch: '<S4>/Switch' */

  /* Saturate: '<S4>/Saturation' */
  if (rtb_Switch > SoftECU_P.Saturation_UpperSat_lm) {
    SoftECU_B.throttle_01 = SoftECU_P.Saturation_UpperSat_lm;
  } else if (rtb_Switch < SoftECU_P.Saturation_LowerSat_h) {
    SoftECU_B.throttle_01 = SoftECU_P.Saturation_LowerSat_h;
  } else {
    SoftECU_B.throttle_01 = rtb_Switch;
  }

  /* End of Saturate: '<S4>/Saturation' */

  /* Gain: '<S4>/Gain' */
  rtb_Switch *= SoftECU_P.EngMaxTrq;

  /* Saturate: '<S4>/Saturation1' */
  u1 = -0.2 * SoftECU_P.EngMaxTrq;
  if (rtb_Switch > SoftECU_P.EngMaxTrq) {
    /* Merge: '<S1>/Merge' */
    SoftECU_B.pt_trq_cmd_Nm = SoftECU_P.EngMaxTrq;
  } else if (rtb_Switch < u1) {
    /* Merge: '<S1>/Merge' */
    SoftECU_B.pt_trq_cmd_Nm = u1;
  } else {
    /* Merge: '<S1>/Merge' */
    SoftECU_B.pt_trq_cmd_Nm = rtb_Switch;
  }

  /* End of Saturate: '<S4>/Saturation1' */
}

/* Update for action system: '<S1>/EMS' */
void SoftECU::SoftECU_EMS_Update() {
  /* Update for If: '<S4>/If' */
  if (SoftECU_DW.If_ActiveSubsystem_g == 0) {
    /* Update for IfAction SubSystem: '<S4>/useTargetAccMode' incorporates:
     *  ActionPort: '<S74>/Action Port'
     */
    if (rtmIsMajorTimeStep((&SoftECU_M))) {
      /* Update for DiscreteIntegrator: '<S108>/Filter' */
      SoftECU_DW.Filter_DSTATE_i += SoftECU_P.Filter_gainval_p * SoftECU_B.FilterCoefficient_o;
      SoftECU_DW.Filter_PrevResetState_p = static_cast<int8_T>(SoftECU_B.LogicalOperator1);

      /* Update for DiscreteIntegrator: '<S113>/Integrator' incorporates:
       *  DiscreteIntegrator: '<S108>/Filter'
       */
      SoftECU_DW.Integrator_DSTATE_c += SoftECU_P.Integrator_gainval_k * SoftECU_B.IntegralGain_o;
      SoftECU_DW.Integrator_PrevResetState_m = static_cast<int8_T>(SoftECU_B.LogicalOperator1);
    }

    /* End of Update for SubSystem: '<S4>/useTargetAccMode' */
  }

  /* End of Update for If: '<S4>/If' */
}

/* System initialize for action system: '<S1>/VCU' */
void SoftECU::SoftECU_VCU_Init() {
  /* Start for If: '<S5>/If' */
  SoftECU_DW.If_ActiveSubsystem_c = -1;

  /* Start for If: '<S5>/If1' */
  SoftECU_DW.If1_ActiveSubsystem = -1;

  /* SystemInitialize for IfAction SubSystem: '<S5>/useAccMode' */
  /* InitializeConditions for DiscreteIntegrator: '<S171>/Filter' */
  SoftECU_DW.Filter_DSTATE = SoftECU_P.DiscretePIDController_InitialConditionForFilter_b;

  /* InitializeConditions for DiscreteIntegrator: '<S176>/Integrator' */
  SoftECU_DW.Integrator_DSTATE = SoftECU_P.DiscretePIDController_InitialConditionForIntegrator_h;

  /* End of SystemInitialize for SubSystem: '<S5>/useAccMode' */
}

/* Outputs for action system: '<S1>/VCU' */
void SoftECU::SoftECU_VCU() {
  real_T rtb_Saturation1;
  real_T rtb_Sum;
  int8_T rtAction;

  /* If: '<S5>/If' */
  if (rtsiIsModeUpdateTimeStep(&(&SoftECU_M)->solverInfo)) {
    rtAction = static_cast<int8_T>(SoftECU_B.acc_control_mode != 0);
    SoftECU_DW.If_ActiveSubsystem_c = rtAction;
  } else {
    rtAction = SoftECU_DW.If_ActiveSubsystem_c;
  }

  switch (rtAction) {
    case 0:
      /* Outputs for IfAction SubSystem: '<S5>/useAccMode' incorporates:
       *  ActionPort: '<S133>/Action Port'
       */
      /* Product: '<S141>/Divide' incorporates:
       *  Constant: '<S141>/Constant'
       *  Sum: '<S141>/acc_err'
       */
      rtb_Sum = (SoftECU_B.target_acc_m_s2 - SoftECU_B.ax_estimator.Integrator1) / SoftECU_P.ecu_max_acc;

      /* Logic: '<S141>/switchOffMotor' incorporates:
       *  Abs: '<S141>/Abs'
       *  Clock: '<S141>/Clock'
       *  Constant: '<S135>/Constant'
       *  Constant: '<S136>/Constant'
       *  Constant: '<S142>/Constant'
       *  Constant: '<S144>/Constant'
       *  Logic: '<S130>/Logical Operator'
       *  Logic: '<S141>/Logical Operator1'
       *  RelationalOperator: '<S135>/Compare'
       *  RelationalOperator: '<S136>/Compare'
       *  RelationalOperator: '<S142>/Compare'
       *  RelationalOperator: '<S144>/Compare'
       */
      SoftECU_B.switchOffMotor = (SoftECU_B.LogicalOperator ||
                                  ((std::abs(SoftECU_B.veh_speed_vx) <= SoftECU_P.speedLow_xms_const) &&
                                   ((SoftECU_B.brake_pressure_0_1 >= SoftECU_P.CompareToConstant1_const_c) ||
                                    (SoftECU_B.Memory[0] >= SoftECU_P.CompareToConstant_const_k))) ||
                                  ((&SoftECU_M)->Timing.t[0] <= SoftECU_P.CompareToConstant_const_c));
      if (rtmIsMajorTimeStep((&SoftECU_M))) {
        /* DiscreteIntegrator: '<S171>/Filter' */
        if (SoftECU_B.switchOffMotor || (SoftECU_DW.Filter_PrevResetState != 0)) {
          SoftECU_DW.Filter_DSTATE = SoftECU_P.DiscretePIDController_InitialConditionForFilter_b;
        }

        /* DiscreteIntegrator: '<S171>/Filter' */
        SoftECU_B.Filter = SoftECU_DW.Filter_DSTATE;

        /* DiscreteIntegrator: '<S176>/Integrator' */
        if (SoftECU_B.switchOffMotor || (SoftECU_DW.Integrator_PrevResetState != 0)) {
          SoftECU_DW.Integrator_DSTATE = SoftECU_P.DiscretePIDController_InitialConditionForIntegrator_h;
        }

        /* DiscreteIntegrator: '<S176>/Integrator' */
        SoftECU_B.Integrator = SoftECU_DW.Integrator_DSTATE;
      }

      /* Gain: '<S173>/Integral Gain' */
      SoftECU_B.IntegralGain = SoftECU_P.vcu_acc_I * rtb_Sum;

      /* Gain: '<S179>/Filter Coefficient' incorporates:
       *  Gain: '<S170>/Derivative Gain'
       *  Sum: '<S171>/SumD'
       */
      SoftECU_B.FilterCoefficient =
          (SoftECU_P.vcu_acc_D * rtb_Sum - SoftECU_B.Filter) * SoftECU_P.DiscretePIDController_N_k;

      /* Switch: '<S141>/Switch' incorporates:
       *  Constant: '<S141>/Constant1'
       *  Constant: '<S141>/Constant2'
       *  Constant: '<S141>/cutoffMotorOutput'
       *  Product: '<S141>/Product'
       *  RelationalOperator: '<S141>/battery_check'
       *  Saturate: '<S183>/Saturation'
       */
      if (SoftECU_B.switchOffMotor) {
        rtb_Sum = SoftECU_P.cutoffMotorOutput_Value;
      } else {
        /* Sum: '<S185>/Sum' incorporates:
         *  Gain: '<S181>/Proportional Gain'
         */
        rtb_Sum = (SoftECU_P.vcu_acc_P * rtb_Sum + SoftECU_B.Integrator) + SoftECU_B.FilterCoefficient;

        /* Saturate: '<S183>/Saturation' */
        if (rtb_Sum > SoftECU_P.DiscretePIDController_UpperSaturationLimit_f) {
          rtb_Sum = SoftECU_P.DiscretePIDController_UpperSaturationLimit_f;
        } else if (rtb_Sum < SoftECU_P.DiscretePIDController_LowerSaturationLimit_k) {
          rtb_Sum = SoftECU_P.DiscretePIDController_LowerSaturationLimit_k;
        }

        rtb_Sum = rtb_Sum * SoftECU_P.MotorMaxTrq *
                  static_cast<real_T>(SoftECU_B.batt_soc_0_1 >= SoftECU_P.Constant2_Value_l);
      }

      /* End of Switch: '<S141>/Switch' */
      /* End of Outputs for SubSystem: '<S5>/useAccMode' */
      break;

    case 1:
      /* Outputs for IfAction SubSystem: '<S5>/useTorqueMode' incorporates:
       *  ActionPort: '<S134>/Action Port'
       */
      /* Product: '<S134>/Product' incorporates:
       *  Constant: '<S193>/Constant'
       *  Constant: '<S194>/Constant'
       *  Logic: '<S134>/Logical Operator'
       *  Logic: '<S134>/Logical Operator1'
       *  RelationalOperator: '<S193>/Compare'
       *  RelationalOperator: '<S194>/Compare'
       */
      rtb_Sum = static_cast<real_T>((SoftECU_B.gear_cmd__0N1D2R3P == SoftECU_P.CompareToConstant1_const_p) ||
                                    (SoftECU_B.gear_cmd__0N1D2R3P == SoftECU_P.CompareToConstant2_const)) *
                SoftECU_B.request_torque * static_cast<real_T>(!SoftECU_B.LogicalOperator);

      /* End of Outputs for SubSystem: '<S5>/useTorqueMode' */
      break;
  }

  /* End of If: '<S5>/If' */

  /* If: '<S5>/If1' */
  if (rtsiIsModeUpdateTimeStep(&(&SoftECU_M)->solverInfo)) {
    rtAction = static_cast<int8_T>(!(rtb_Sum >= 0.0));
    SoftECU_DW.If1_ActiveSubsystem = rtAction;
  } else {
    rtAction = SoftECU_DW.If1_ActiveSubsystem;
  }

  switch (rtAction) {
    case 0:
      /* Outputs for IfAction SubSystem: '<S5>/ForwardAndPositiveTorque' incorporates:
       *  ActionPort: '<S131>/Action Port'
       */
      /* Product: '<S131>/Product' incorporates:
       *  Constant: '<S137>/Constant'
       *  RelationalOperator: '<S137>/Compare'
       */
      rtb_Saturation1 =
          static_cast<real_T>(SoftECU_B.gear_cmd__0N1D2R3P == SoftECU_P.CompareToConstant_const_l) * rtb_Sum;

      /* End of Outputs for SubSystem: '<S5>/ForwardAndPositiveTorque' */
      break;

    case 1:
      /* Outputs for IfAction SubSystem: '<S5>/ReverseOrNegativeTorque(PowerRecycle)' incorporates:
       *  ActionPort: '<S132>/Action Port'
       */
      /* MultiPortSwitch: '<S132>/Multiport Switch' incorporates:
       *  Constant: '<S132>/Constant'
       *  Constant: '<S132>/Constant2'
       *  Constant: '<S132>/Constant3'
       *  Constant: '<S138>/Constant'
       *  Product: '<S132>/Product'
       *  Product: '<S132>/Product4'
       *  RelationalOperator: '<S132>/Relational Operator1'
       *  RelationalOperator: '<S132>/Relational Operator2'
       *  RelationalOperator: '<S138>/Compare'
       */
      switch (SoftECU_B.gear_cmd__0N1D2R3P) {
        case 0:
          rtb_Saturation1 = SoftECU_P.Constant_Value;
          break;

        case 1:
          /* Product: '<S139>/Product1' incorporates:
           *  Constant: '<S139>/Constant1'
           *  Constant: '<S139>/Constant4'
           *  Constant: '<S139>/averageTorqueCoeff'
           */
          rtb_Saturation1 =
              SoftECU_P.averageTorqueCoeff_Value * SoftECU_P.MotorMaxTrq * SoftECU_P.MotorPowerRegenerationRatio;

          /* Switch: '<S140>/Switch2' incorporates:
           *  RelationalOperator: '<S140>/LowerRelop1'
           */
          if (!(rtb_Sum > rtb_Saturation1)) {
            /* Gain: '<S132>/Gain' */
            rtb_Saturation1 *= SoftECU_P.Gain_Gain;

            /* Switch: '<S140>/Switch' incorporates:
             *  RelationalOperator: '<S140>/UpperRelop'
             */
            if (!(rtb_Sum < rtb_Saturation1)) {
              rtb_Saturation1 = rtb_Sum;
            }

            /* End of Switch: '<S140>/Switch' */
          }

          /* End of Switch: '<S140>/Switch2' */
          rtb_Saturation1 = static_cast<real_T>(SoftECU_B.veh_speed_vx > SoftECU_P.CompareToConstant_const_e) *
                            rtb_Saturation1 * static_cast<real_T>(SoftECU_B.batt_soc_0_1 <= SoftECU_P.Constant3_Value);
          break;

        case 2:
          rtb_Saturation1 = static_cast<real_T>(SoftECU_B.batt_soc_0_1 >= SoftECU_P.Constant2_Value_b) * rtb_Sum;
          break;

        default:
          rtb_Saturation1 = SoftECU_P.Constant_Value;
          break;
      }

      /* End of MultiPortSwitch: '<S132>/Multiport Switch' */
      /* End of Outputs for SubSystem: '<S5>/ReverseOrNegativeTorque(PowerRecycle)' */
      break;
  }

  /* End of If: '<S5>/If1' */

  /* Saturate: '<S5>/Saturation1' */
  if (rtb_Saturation1 > SoftECU_P.MotorMaxTrq) {
    rtb_Saturation1 = SoftECU_P.MotorMaxTrq;
  } else if (rtb_Saturation1 < -SoftECU_P.MotorMaxTrq) {
    rtb_Saturation1 = -SoftECU_P.MotorMaxTrq;
  }

  /* End of Saturate: '<S5>/Saturation1' */

  /* SignalConversion generated from: '<S5>/trq_out[Nm]' */
  SoftECU_B.pt_trq_cmd_Nm = rtb_Saturation1;

  /* Product: '<S5>/Divide' incorporates:
   *  Constant: '<S5>/Constant'
   */
  rtb_Sum = rtb_Saturation1 / SoftECU_P.MotorMaxTrq;

  /* Saturate: '<S5>/Saturation' */
  if (rtb_Sum > SoftECU_P.Saturation_UpperSat_e) {
    SoftECU_B.throttle_01 = SoftECU_P.Saturation_UpperSat_e;
  } else if (rtb_Sum < SoftECU_P.Saturation_LowerSat_o) {
    SoftECU_B.throttle_01 = SoftECU_P.Saturation_LowerSat_o;
  } else {
    SoftECU_B.throttle_01 = rtb_Sum;
  }

  /* End of Saturate: '<S5>/Saturation' */
}

/* Update for action system: '<S1>/VCU' */
void SoftECU::SoftECU_VCU_Update() {
  /* Update for If: '<S5>/If' */
  if (SoftECU_DW.If_ActiveSubsystem_c == 0) {
    /* Update for IfAction SubSystem: '<S5>/useAccMode' incorporates:
     *  ActionPort: '<S133>/Action Port'
     */
    if (rtmIsMajorTimeStep((&SoftECU_M))) {
      /* Update for DiscreteIntegrator: '<S171>/Filter' */
      SoftECU_DW.Filter_DSTATE += SoftECU_P.Filter_gainval_k * SoftECU_B.FilterCoefficient;
      SoftECU_DW.Filter_PrevResetState = static_cast<int8_T>(SoftECU_B.switchOffMotor);

      /* Update for DiscreteIntegrator: '<S176>/Integrator' incorporates:
       *  DiscreteIntegrator: '<S171>/Filter'
       */
      SoftECU_DW.Integrator_DSTATE += SoftECU_P.Integrator_gainval_i * SoftECU_B.IntegralGain;
      SoftECU_DW.Integrator_PrevResetState = static_cast<int8_T>(SoftECU_B.switchOffMotor);
    }

    /* End of Update for SubSystem: '<S5>/useAccMode' */
  }

  /* End of Update for If: '<S5>/If' */
}

/* System initialize for atomic system: '<S1>/ax_estimator' */
void SoftECU::SoftECU_ax_estimator_Init(DW_ax_estimator_SoftECU_T *localDW, P_ax_estimator_SoftECU_T *localP,
                                        X_ax_estimator_SoftECU_T *localX) {
  /* Start for If: '<S6>/If' */
  localDW->If_ActiveSubsystem = -1;

  /* InitializeConditions for Integrator: '<S195>/Integrator1' */
  localX->Integrator1_CSTATE = localP->Integrator1_IC;

  /* SystemInitialize for IfAction SubSystem: '<S6>/AxCal' */
  /* InitializeConditions for Delay: '<S196>/Delay' */
  for (int32_T i{0}; i < 100; i++) {
    localDW->Delay_DSTATE[i] = localP->Delay_InitialCondition;
  }

  /* End of InitializeConditions for Delay: '<S196>/Delay' */
  /* End of SystemInitialize for SubSystem: '<S6>/AxCal' */
}

/* Outputs for atomic system: '<S1>/ax_estimator' */
void SoftECU::SoftECU_ax_estimator(real_T rtu_acc_feedback_m_s2, real_T rtu_vx, B_ax_estimator_SoftECU_T *localB,
                                   DW_ax_estimator_SoftECU_T *localDW, P_ax_estimator_SoftECU_T *localP,
                                   P_SoftECU_T *SoftECU_P, X_ax_estimator_SoftECU_T *localX) {
  real_T rtb_Merge_i;
  int8_T rtAction;

  /* Abs: '<S6>/Abs1' */
  rtb_Merge_i = std::abs(rtu_vx);

  /* If: '<S6>/If' incorporates:
   *  Constant: '<S198>/Constant'
   *  RelationalOperator: '<S198>/Compare'
   */
  if (rtsiIsModeUpdateTimeStep(&(&SoftECU_M)->solverInfo)) {
    rtAction = static_cast<int8_T>(!(rtb_Merge_i >= localP->SpeedThresh_const));
    localDW->If_ActiveSubsystem = rtAction;
  } else {
    rtAction = localDW->If_ActiveSubsystem;
  }

  switch (rtAction) {
    case 0:
      /* Outputs for IfAction SubSystem: '<S6>/If Action Subsystem' incorporates:
       *  ActionPort: '<S197>/Action Port'
       */
      /* SignalConversion generated from: '<S197>/<acc_feedback_m_s2>' */
      rtb_Merge_i = rtu_acc_feedback_m_s2;

      /* End of Outputs for SubSystem: '<S6>/If Action Subsystem' */
      break;

    case 1: {
      /* Outputs for IfAction SubSystem: '<S6>/AxCal' incorporates:
       *  ActionPort: '<S196>/Action Port'
       */
      if (rtmIsMajorTimeStep((&SoftECU_M))) {
        /* Delay: '<S196>/Delay' incorporates:
         *  Constant: '<S196>/AxDelayStep'
         */
        if ((SoftECU_P->softecu_ax_delay < 1.0) || std::isnan(SoftECU_P->softecu_ax_delay)) {
          /* Delay: '<S196>/Delay' */
          localB->Delay = rtu_vx;
        } else {
          uint32_T tmp;
          if (SoftECU_P->softecu_ax_delay > 100.0) {
            tmp = 100U;
          } else {
            rtb_Merge_i = std::trunc(SoftECU_P->softecu_ax_delay);
            if (std::isnan(rtb_Merge_i) || std::isinf(rtb_Merge_i)) {
              rtb_Merge_i = 0.0;
            } else {
              rtb_Merge_i = std::fmod(rtb_Merge_i, 4.294967296E+9);
            }

            tmp = rtb_Merge_i < 0.0 ? static_cast<uint32_T>(-static_cast<int32_T>(static_cast<uint32_T>(-rtb_Merge_i)))
                                    : static_cast<uint32_T>(rtb_Merge_i);
          }

          /* Delay: '<S196>/Delay' */
          localB->Delay = localDW->Delay_DSTATE[100U - tmp];
        }

        /* End of Delay: '<S196>/Delay' */
      }

      /* Product: '<S196>/Divide' incorporates:
       *  Constant: '<S196>/AxDelayStep'
       *  Gain: '<S196>/Gain'
       *  Sum: '<S196>/Subtract'
       */
      rtb_Merge_i = (rtu_vx - localB->Delay) / (SoftECU_P->stepTime * SoftECU_P->softecu_ax_delay);

      /* End of Outputs for SubSystem: '<S6>/AxCal' */
    } break;
  }

  /* End of If: '<S6>/If' */

  /* Integrator: '<S195>/Integrator1' */
  localB->Integrator1 = localX->Integrator1_CSTATE;

  /* Product: '<S195>/Divide' incorporates:
   *  Constant: '<S195>/Constant'
   *  Sum: '<S195>/Sum'
   */
  localB->Divide = (rtb_Merge_i - localB->Integrator1) * localP->AxAct_wc;
}

/* Update for atomic system: '<S1>/ax_estimator' */
void SoftECU::SoftECU_ax_estimator_Update(real_T rtu_vx, DW_ax_estimator_SoftECU_T *localDW) {
  /* Update for If: '<S6>/If' */
  if (localDW->If_ActiveSubsystem == 1) {
    /* Update for IfAction SubSystem: '<S6>/AxCal' incorporates:
     *  ActionPort: '<S196>/Action Port'
     */
    if (rtmIsMajorTimeStep((&SoftECU_M))) {
      /* Update for Delay: '<S196>/Delay' */
      for (int_T idxDelay{0}; idxDelay < 99; idxDelay++) {
        localDW->Delay_DSTATE[idxDelay] = localDW->Delay_DSTATE[idxDelay + 1];
      }

      localDW->Delay_DSTATE[99] = rtu_vx;

      /* End of Update for Delay: '<S196>/Delay' */
    }

    /* End of Update for SubSystem: '<S6>/AxCal' */
  }

  /* End of Update for If: '<S6>/If' */
}

/* Derivatives for atomic system: '<S1>/ax_estimator' */
void SoftECU::SoftECU_ax_estimator_Deriv(B_ax_estimator_SoftECU_T *localB, XDot_ax_estimator_SoftECU_T *localXdot) {
  /* Derivatives for Integrator: '<S195>/Integrator1' */
  localXdot->Integrator1_CSTATE = localB->Divide;
}

/* Model step function */
void SoftECU::step() {
  int32_T rtb_autoHoldSwitch;
  int8_T rtAction;
  boolean_T accMode;
  boolean_T gearDrive;
  boolean_T gearReverse;
  boolean_T rtb_Compare_ho;
  boolean_T torqueMode;
  if (rtmIsMajorTimeStep((&SoftECU_M))) {
    /* set solver stop time */
    if (!((&SoftECU_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&SoftECU_M)->solverInfo,
                            (((&SoftECU_M)->Timing.clockTickH0 + 1) * (&SoftECU_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(&(&SoftECU_M)->solverInfo,
                            (((&SoftECU_M)->Timing.clockTick0 + 1) * (&SoftECU_M)->Timing.stepSize0 +
                             (&SoftECU_M)->Timing.clockTickH0 * (&SoftECU_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&SoftECU_M))) {
    (&SoftECU_M)->Timing.t[0] = rtsiGetT(&(&SoftECU_M)->solverInfo);
  }

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.target_acc_m_s2 = SoftECU_U.ecu_in.driver_input.target_acc_m_s2;

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.request_torque = SoftECU_U.ecu_in.acc_feedback_m_s2;

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.veh_speed_vx = SoftECU_U.ecu_in.veh_speed_vx;

  /* Outputs for Atomic SubSystem: '<S1>/ax_estimator' */
  SoftECU_ax_estimator(SoftECU_B.request_torque, SoftECU_B.veh_speed_vx, &SoftECU_B.ax_estimator,
                       &SoftECU_DW.ax_estimator, &SoftECU_P.ax_estimator, &SoftECU_P, &SoftECU_X.ax_estimator);

  /* End of Outputs for SubSystem: '<S1>/ax_estimator' */

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.brake_control_mode = SoftECU_U.ecu_in.driver_input.brake_control_mode;

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.batt_soc_0_1 = SoftECU_U.ecu_in.driver_input.brake_pressure_01;

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.gear_cmd__0N1D2R3P = SoftECU_U.ecu_in.driver_input.gear_cmd__0N1D2R3P;

  /* Outputs for Atomic SubSystem: '<S1>/ABS_EBD' */
  SoftECU_ABS_EBD();

  /* End of Outputs for SubSystem: '<S1>/ABS_EBD' */

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.batt_soc_0_1 = SoftECU_U.ecu_in.batt_soc_0_1;

  /* RelationalOperator: '<S69>/Compare' incorporates:
   *  Abs: '<S68>/Abs'
   *  Constant: '<S69>/Constant'
   */
  rtb_Compare_ho = (std::abs(SoftECU_B.veh_speed_vx) < SoftECU_P.speedOffset);

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.acc_control_mode = SoftECU_U.ecu_in.driver_input.acc_control_mode;

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.request_torque = SoftECU_U.ecu_in.driver_input.request_torque;

  /* MATLAB Function: '<S3>/AutoHold' incorporates:
   *  Constant: '<S3>/Constant2'
   */
  rtb_autoHoldSwitch = 0;
  accMode = (SoftECU_B.acc_control_mode == 0);
  torqueMode = (SoftECU_B.acc_control_mode == 1);
  gearDrive = (SoftECU_B.gear_cmd__0N1D2R3P == 1);
  gearReverse = (SoftECU_B.gear_cmd__0N1D2R3P == 2);
  if (rtb_Compare_ho && ((SoftECU_B.gear_cmd__0N1D2R3P == 0) || (SoftECU_B.gear_cmd__0N1D2R3P == 3))) {
    rtb_autoHoldSwitch = 1;
  }

  if (rtb_Compare_ho && accMode && (SoftECU_B.target_acc_m_s2 < 0.001) && gearDrive) {
    rtb_autoHoldSwitch = 1;
  }

  if (rtb_Compare_ho && torqueMode && (SoftECU_B.request_torque < 1.0) && gearDrive) {
    rtb_autoHoldSwitch = 1;
  }

  if (rtb_Compare_ho && accMode && (SoftECU_B.target_acc_m_s2 > -0.001) && gearReverse) {
    rtb_autoHoldSwitch = 1;
  }

  if (rtb_Compare_ho && torqueMode && gearReverse && (SoftECU_P.DriveLinePowType == 1.0) &&
      (SoftECU_B.request_torque < 1.0)) {
    rtb_autoHoldSwitch = 1;
  }

  if (rtb_Compare_ho && torqueMode && gearReverse && (SoftECU_P.DriveLinePowType == 2.0) &&
      (SoftECU_B.request_torque > -1.0)) {
    rtb_autoHoldSwitch = 1;
  }

  /* End of MATLAB Function: '<S3>/AutoHold' */

  /* Switch: '<S1>/Switch1' incorporates:
   *  Constant: '<S1>/autoHoldBrakePressure_01'
   */
  if (rtb_autoHoldSwitch > SoftECU_P.Switch1_Threshold) {
    SoftECU_B.brake_pressure_0_1 = SoftECU_P.autoHoldBrakePressure_01_Value;
  } else {
    SoftECU_B.brake_pressure_0_1 = SoftECU_B.throttle_01;
  }

  /* End of Switch: '<S1>/Switch1' */

  /* If: '<S1>/If' incorporates:
   *  Constant: '<S1>/Constant1'
   *  Relay: '<S7>/RelayCutOffAt58_EnableAt55'
   *  Relay: '<S7>/RelayCutOffAt6_EnableAt5'
   */
  if (rtsiIsModeUpdateTimeStep(&(&SoftECU_M)->solverInfo)) {
    SoftECU_DW.RelayCutOffAt6_EnableAt5_Mode =
        ((SoftECU_B.veh_speed_vx >= SoftECU_P.RelayCutOffAt6_EnableAt5_OnVal) ||
         ((!(SoftECU_B.veh_speed_vx <= SoftECU_P.RelayCutOffAt6_EnableAt5_OffVal)) &&
          SoftECU_DW.RelayCutOffAt6_EnableAt5_Mode));
    SoftECU_DW.RelayCutOffAt58_EnableAt55_Mode =
        ((SoftECU_B.veh_speed_vx >= SoftECU_P.RelayCutOffAt58_EnableAt55_OnVal) ||
         ((!(SoftECU_B.veh_speed_vx <= SoftECU_P.RelayCutOffAt58_EnableAt55_OffVal)) &&
          SoftECU_DW.RelayCutOffAt58_EnableAt55_Mode));
    rtAction = static_cast<int8_T>(!(SoftECU_P.DriveLinePowType == 1.0));
    SoftECU_DW.If_ActiveSubsystem = rtAction;
  } else {
    rtAction = SoftECU_DW.If_ActiveSubsystem;
  }

  if (rtmIsMajorTimeStep((&SoftECU_M))) {
    real_T tmp;
    real_T tmp_0;

    /* Relay: '<S7>/RelayCutOffAt6_EnableAt5' */
    if (SoftECU_DW.RelayCutOffAt6_EnableAt5_Mode) {
      tmp = SoftECU_P.RelayCutOffAt6_EnableAt5_YOn;
    } else {
      tmp = SoftECU_P.RelayCutOffAt6_EnableAt5_YOff;
    }

    /* Relay: '<S7>/RelayCutOffAt58_EnableAt55' */
    if (SoftECU_DW.RelayCutOffAt58_EnableAt55_Mode) {
      tmp_0 = SoftECU_P.RelayCutOffAt58_EnableAt55_YOn;
    } else {
      tmp_0 = SoftECU_P.RelayCutOffAt58_EnableAt55_YOff;
    }

    /* Logic: '<S7>/Logical Operator' incorporates:
     *  Relay: '<S7>/RelayCutOffAt58_EnableAt55'
     *  Relay: '<S7>/RelayCutOffAt6_EnableAt5'
     */
    SoftECU_B.LogicalOperator = ((tmp != 0.0) || (tmp_0 != 0.0));
  }

  /* If: '<S1>/If' */
  switch (rtAction) {
    case 0:
      /* Outputs for IfAction SubSystem: '<S1>/EMS' incorporates:
       *  ActionPort: '<S4>/Action Port'
       */
      SoftECU_EMS();

      /* End of Outputs for SubSystem: '<S1>/EMS' */
      break;

    case 1:
      /* Outputs for IfAction SubSystem: '<S1>/VCU' incorporates:
       *  ActionPort: '<S5>/Action Port'
       */
      SoftECU_VCU();

      /* End of Outputs for SubSystem: '<S1>/VCU' */
      break;
  }

  /* Outport: '<Root>/ecu_out' incorporates:
   *  BusCreator: '<S1>/Bus Creator'
   *  Constant: '<S1>/Constant'
   */
  SoftECU_Y.ecu_out.pt_trq_cmd_Nm = SoftECU_B.pt_trq_cmd_Nm;
  SoftECU_Y.ecu_out.brake_pressure_0_1 = SoftECU_B.brake_pressure_0_1;
  SoftECU_Y.ecu_out.brake_pressure_Pa[0] = SoftECU_P.Constant_Value_o;
  SoftECU_Y.ecu_out.brake_pressure_Pa[1] = SoftECU_P.Constant_Value_o;
  SoftECU_Y.ecu_out.brake_pressure_Pa[2] = SoftECU_P.Constant_Value_o;
  SoftECU_Y.ecu_out.brake_pressure_Pa[3] = SoftECU_P.Constant_Value_o;
  SoftECU_Y.ecu_out.throttle_01 = SoftECU_B.throttle_01;
  SoftECU_Y.ecu_out.abs_flag[0] = SoftECU_B.Memory[0];

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.slip_ratio[0] = SoftECU_U.ecu_in.slip_ratio[0];

  /* Outport: '<Root>/ecu_out' incorporates:
   *  BusCreator: '<S1>/Bus Creator'
   */
  SoftECU_Y.ecu_out.abs_flag[1] = SoftECU_B.Memory[1];

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.slip_ratio[1] = SoftECU_U.ecu_in.slip_ratio[1];

  /* Outport: '<Root>/ecu_out' incorporates:
   *  BusCreator: '<S1>/Bus Creator'
   */
  SoftECU_Y.ecu_out.abs_flag[2] = SoftECU_B.Memory[2];

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.slip_ratio[2] = SoftECU_U.ecu_in.slip_ratio[2];

  /* Outport: '<Root>/ecu_out' incorporates:
   *  BusCreator: '<S1>/Bus Creator'
   */
  SoftECU_Y.ecu_out.abs_flag[3] = SoftECU_B.Memory[3];

  /* SignalConversion generated from: '<Root>/ecu_in' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftECU_B.slip_ratio[3] = SoftECU_U.ecu_in.slip_ratio[3];
  if (rtmIsMajorTimeStep((&SoftECU_M))) {
    /* Update for Atomic SubSystem: '<S1>/ax_estimator' */
    SoftECU_ax_estimator_Update(SoftECU_B.veh_speed_vx, &SoftECU_DW.ax_estimator);

    /* End of Update for SubSystem: '<S1>/ax_estimator' */

    /* Update for Atomic SubSystem: '<S1>/ABS_EBD' */
    SoftECU_ABS_EBD_Update();

    /* End of Update for SubSystem: '<S1>/ABS_EBD' */

    /* Update for If: '<S1>/If' */
    switch (SoftECU_DW.If_ActiveSubsystem) {
      case 0:
        /* Update for IfAction SubSystem: '<S1>/EMS' incorporates:
         *  ActionPort: '<S4>/Action Port'
         */
        SoftECU_EMS_Update();

        /* End of Update for SubSystem: '<S1>/EMS' */
        break;

      case 1:
        /* Update for IfAction SubSystem: '<S1>/VCU' incorporates:
         *  ActionPort: '<S5>/Action Port'
         */
        SoftECU_VCU_Update();

        /* End of Update for SubSystem: '<S1>/VCU' */
        break;
    }

    /* End of Update for If: '<S1>/If' */
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&SoftECU_M))) {
    rt_ertODEUpdateContinuousStates(&(&SoftECU_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&SoftECU_M)->Timing.clockTick0)) {
      ++(&SoftECU_M)->Timing.clockTickH0;
    }

    (&SoftECU_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&SoftECU_M)->solverInfo);

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
      (&SoftECU_M)->Timing.clockTick1++;
      if (!(&SoftECU_M)->Timing.clockTick1) {
        (&SoftECU_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void SoftECU::SoftECU_derivatives() {
  XDot_SoftECU_T *_rtXdot;
  _rtXdot = ((XDot_SoftECU_T *)(&SoftECU_M)->derivs);

  /* Derivatives for Atomic SubSystem: '<S1>/ax_estimator' */
  SoftECU_ax_estimator_Deriv(&SoftECU_B.ax_estimator, &_rtXdot->ax_estimator);

  /* End of Derivatives for SubSystem: '<S1>/ax_estimator' */
}

/* Model initialize function */
void SoftECU::initialize() {
  /* Registration code */
  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&SoftECU_M)->solverInfo, &(&SoftECU_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&SoftECU_M)->solverInfo, &rtmGetTPtr((&SoftECU_M)));
    rtsiSetStepSizePtr(&(&SoftECU_M)->solverInfo, &(&SoftECU_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&SoftECU_M)->solverInfo, &(&SoftECU_M)->derivs);
    rtsiSetContStatesPtr(&(&SoftECU_M)->solverInfo, (real_T **)&(&SoftECU_M)->contStates);
    rtsiSetNumContStatesPtr(&(&SoftECU_M)->solverInfo, &(&SoftECU_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&SoftECU_M)->solverInfo, &(&SoftECU_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&SoftECU_M)->solverInfo, &(&SoftECU_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&SoftECU_M)->solverInfo, &(&SoftECU_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&SoftECU_M)->solverInfo, (&rtmGetErrorStatus((&SoftECU_M))));
    rtsiSetRTModelPtr(&(&SoftECU_M)->solverInfo, (&SoftECU_M));
  }

  rtsiSetSimTimeStep(&(&SoftECU_M)->solverInfo, MAJOR_TIME_STEP);
  (&SoftECU_M)->intgData.y = (&SoftECU_M)->odeY;
  (&SoftECU_M)->intgData.f[0] = (&SoftECU_M)->odeF[0];
  (&SoftECU_M)->intgData.f[1] = (&SoftECU_M)->odeF[1];
  (&SoftECU_M)->intgData.f[2] = (&SoftECU_M)->odeF[2];
  (&SoftECU_M)->intgData.f[3] = (&SoftECU_M)->odeF[3];
  (&SoftECU_M)->contStates = ((X_SoftECU_T *)&SoftECU_X);
  rtsiSetSolverData(&(&SoftECU_M)->solverInfo, static_cast<void *>(&(&SoftECU_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&SoftECU_M)->solverInfo, false);
  rtsiSetSolverName(&(&SoftECU_M)->solverInfo, "ode4");
  rtmSetTPtr((&SoftECU_M), &(&SoftECU_M)->Timing.tArray[0]);
  (&SoftECU_M)->Timing.stepSize0 = 0.001;

  /* Start for If: '<S1>/If' */
  SoftECU_DW.If_ActiveSubsystem = -1;

  /* SystemInitialize for Atomic SubSystem: '<S1>/ax_estimator' */
  SoftECU_ax_estimator_Init(&SoftECU_DW.ax_estimator, &SoftECU_P.ax_estimator, &SoftECU_X.ax_estimator);

  /* End of SystemInitialize for SubSystem: '<S1>/ax_estimator' */

  /* SystemInitialize for Atomic SubSystem: '<S1>/ABS_EBD' */
  SoftECU_ABS_EBD_Init();

  /* End of SystemInitialize for SubSystem: '<S1>/ABS_EBD' */

  /* SystemInitialize for IfAction SubSystem: '<S1>/EMS' */
  SoftECU_EMS_Init();

  /* End of SystemInitialize for SubSystem: '<S1>/EMS' */

  /* SystemInitialize for IfAction SubSystem: '<S1>/VCU' */
  SoftECU_VCU_Init();

  /* End of SystemInitialize for SubSystem: '<S1>/VCU' */
}

/* Model terminate function */
void SoftECU::terminate() { /* (no terminate code required) */
}

/* Constructor */
SoftECU::SoftECU() : SoftECU_U(), SoftECU_Y(), SoftECU_B(), SoftECU_DW(), SoftECU_X(), SoftECU_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
SoftECU::~SoftECU() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_SoftECU_T *SoftECU::getRTM() { return (&SoftECU_M); }
