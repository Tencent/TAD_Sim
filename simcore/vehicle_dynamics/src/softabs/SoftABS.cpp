/*
 * SoftABS.cpp
 *
 * Code generation for model "SoftABS".
 *
 * Model version              : 2.108
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Fri Jul 21 17:32:21 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "SoftABS.h"
#include <cmath>
#include "SoftABS_private.h"
#include "rtwtypes.h"

/* System initialize for atomic system: '<S1>/ABS_EBD' */
void SoftABS::SoftABS_ABS_EBD_Init() {
  /* Start for If: '<S2>/If' */
  SoftABS_DW.If_ActiveSubsystem = -1;

  /* InitializeConditions for Memory: '<S2>/Memory' */
  SoftABS_DW.Memory_PreviousInput[0] = SoftABS_P.Memory_InitialCondition;
  SoftABS_DW.Memory_PreviousInput[1] = SoftABS_P.Memory_InitialCondition;
  SoftABS_DW.Memory_PreviousInput[2] = SoftABS_P.Memory_InitialCondition;
  SoftABS_DW.Memory_PreviousInput[3] = SoftABS_P.Memory_InitialCondition;

  /* SystemInitialize for IfAction SubSystem: '<S2>/accControl' */
  /* InitializeConditions for DiscreteIntegrator: '<S40>/Filter' */
  SoftABS_DW.Filter_DSTATE = SoftABS_P.DiscretePIDController_InitialConditionForFilter;

  /* InitializeConditions for DiscreteIntegrator: '<S45>/Integrator' */
  SoftABS_DW.Integrator_DSTATE = SoftABS_P.DiscretePIDController_InitialConditionForIntegrator;

  /* End of SystemInitialize for SubSystem: '<S2>/accControl' */
}

/* Outputs for atomic system: '<S1>/ABS_EBD' */
void SoftABS::SoftABS_ABS_EBD() {
  real_T rtb_Divide;

  /* Memory: '<S2>/Memory' */
  SoftABS_B.Memory[0] = SoftABS_DW.Memory_PreviousInput[0];
  SoftABS_B.Memory[1] = SoftABS_DW.Memory_PreviousInput[1];
  SoftABS_B.Memory[2] = SoftABS_DW.Memory_PreviousInput[2];
  SoftABS_B.Memory[3] = SoftABS_DW.Memory_PreviousInput[3];

  /* If: '<S2>/If' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  if (SoftABS_U.ecu_in.driver_input.brake_control_mode == 1) {
    SoftABS_DW.If_ActiveSubsystem = 0;

    /* Outputs for IfAction SubSystem: '<S2>/pressureControl' incorporates:
     *  ActionPort: '<S5>/Action Port'
     */
    /* Merge: '<S2>/Merge1' incorporates:
     *  Constant: '<S5>/abs_brake_pressure_01'
     *  Constant: '<S62>/Constant'
     *  RelationalOperator: '<S62>/Compare'
     */
    SoftABS_B.Merge1 = (SoftABS_P.abs_brake_pressure >= SoftABS_P.CompareToConstant_const);

    /* Switch: '<S5>/Switch' incorporates:
     *  Constant: '<S5>/abs_brake_pressure_01'
     */
    if (SoftABS_B.Memory[0] > SoftABS_P.Switch_Threshold) {
      rtb_Divide = SoftABS_P.abs_brake_pressure;
    } else {
      rtb_Divide = SoftABS_U.ecu_in.driver_input.brake_pressure_01;
    }

    /* End of Switch: '<S5>/Switch' */
    /* End of Outputs for SubSystem: '<S2>/pressureControl' */
  } else {
    boolean_T rtb_LogicalOperator2;
    SoftABS_DW.If_ActiveSubsystem = 1;

    /* Outputs for IfAction SubSystem: '<S2>/accControl' incorporates:
     *  ActionPort: '<S4>/Action Port'
     */
    /* Logic: '<S6>/Logical Operator2' incorporates:
     *  Abs: '<S6>/Abs'
     *  Constant: '<S10>/Constant'
     *  Constant: '<S11>/Constant'
     *  Constant: '<S12>/Constant'
     *  Constant: '<S13>/Constant'
     *  Constant: '<S8>/Constant'
     *  Constant: '<S9>/Constant'
     *  Logic: '<S6>/Logical Operator'
     *  Logic: '<S6>/Logical Operator1'
     *  Logic: '<S6>/Logical Operator3'
     *  RelationalOperator: '<S10>/Compare'
     *  RelationalOperator: '<S11>/Compare'
     *  RelationalOperator: '<S12>/Compare'
     *  RelationalOperator: '<S13>/Compare'
     *  RelationalOperator: '<S8>/Compare'
     *  RelationalOperator: '<S9>/Compare'
     */
    rtb_LogicalOperator2 =
        (((SoftABS_U.ecu_in.veh_speed_vx < -SoftABS_P.speedOffset) &&
          (SoftABS_U.ecu_in.driver_input.gear_cmd__0N1D2R3P == SoftABS_P.reverseGear_const) &&
          (SoftABS_U.ecu_in.driver_input.target_acc_m_s2 > SoftABS_P.brakeEnableThresh_Reverse_const)) ||
         ((SoftABS_U.ecu_in.driver_input.gear_cmd__0N1D2R3P != SoftABS_P.notReverse_const) &&
          (SoftABS_U.ecu_in.driver_input.target_acc_m_s2 < SoftABS_P.brakeEnableThresh_const) &&
          (std::abs(SoftABS_U.ecu_in.veh_speed_vx) > SoftABS_P.speedOffset)));

    /* Product: '<S4>/Divide' incorporates:
     *  Constant: '<S4>/Constant'
     *  Sum: '<S4>/Add'
     */
    rtb_Divide =
        (SoftABS_U.ecu_in.driver_input.target_acc_m_s2 - SoftABS_U.ecu_in.acc_feedback_m_s2) / SoftABS_P.ecu_max_dec;

    /* Logic: '<S4>/Logical Operator' */
    SoftABS_B.LogicalOperator = !rtb_LogicalOperator2;

    /* DiscreteIntegrator: '<S40>/Filter' */
    if (SoftABS_B.LogicalOperator || (SoftABS_DW.Filter_PrevResetState != 0)) {
      SoftABS_DW.Filter_DSTATE = SoftABS_P.DiscretePIDController_InitialConditionForFilter;
    }

    /* DiscreteIntegrator: '<S40>/Filter' */
    SoftABS_B.Filter = SoftABS_DW.Filter_DSTATE;

    /* Gain: '<S42>/Integral Gain' */
    SoftABS_B.IntegralGain = SoftABS_P.brake_I * rtb_Divide;

    /* DiscreteIntegrator: '<S45>/Integrator' */
    if (SoftABS_B.LogicalOperator || (SoftABS_DW.Integrator_PrevResetState != 0)) {
      SoftABS_DW.Integrator_DSTATE = SoftABS_P.DiscretePIDController_InitialConditionForIntegrator;
    }

    /* DiscreteIntegrator: '<S45>/Integrator' */
    SoftABS_B.Integrator = SoftABS_DW.Integrator_DSTATE;

    /* Gain: '<S48>/Filter Coefficient' incorporates:
     *  Gain: '<S39>/Derivative Gain'
     *  Sum: '<S40>/SumD'
     */
    SoftABS_B.FilterCoefficient =
        (SoftABS_P.brake_D * rtb_Divide - SoftABS_B.Filter) * SoftABS_P.DiscretePIDController_N;

    /* Merge: '<S2>/Merge1' incorporates:
     *  SignalConversion generated from: '<S4>/brakeEnable'
     */
    SoftABS_B.Merge1 = rtb_LogicalOperator2;

    /* Switch: '<S4>/abs' incorporates:
     *  Constant: '<S4>/abs_brake_pressure_01'
     */
    if (SoftABS_B.Memory[0] > SoftABS_P.abs_Threshold) {
      rtb_Divide = SoftABS_P.abs_brake_pressure;
    } else {
      /* Sum: '<S54>/Sum' incorporates:
       *  Gain: '<S50>/Proportional Gain'
       */
      rtb_Divide = (SoftABS_P.brake_P * rtb_Divide + SoftABS_B.Integrator) + SoftABS_B.FilterCoefficient;

      /* Saturate: '<S52>/Saturation' */
      if (rtb_Divide > SoftABS_P.DiscretePIDController_UpperSaturationLimit) {
        rtb_Divide = SoftABS_P.DiscretePIDController_UpperSaturationLimit;
      } else if (rtb_Divide < SoftABS_P.DiscretePIDController_LowerSaturationLimit) {
        rtb_Divide = SoftABS_P.DiscretePIDController_LowerSaturationLimit;
      }

      /* End of Saturate: '<S52>/Saturation' */
    }

    /* Product: '<S4>/Product' incorporates:
     *  Switch: '<S4>/abs'
     */
    rtb_Divide *= static_cast<real_T>(rtb_LogicalOperator2);

    /* End of Outputs for SubSystem: '<S2>/accControl' */
  }

  /* End of If: '<S2>/If' */

  /* Saturate: '<S2>/Saturation' */
  if (rtb_Divide > SoftABS_P.Saturation_UpperSat) {
    /* Saturate: '<S2>/Saturation' */
    SoftABS_B.brake_pressure_01 = SoftABS_P.Saturation_UpperSat;
  } else if (rtb_Divide < SoftABS_P.Saturation_LowerSat) {
    /* Saturate: '<S2>/Saturation' */
    SoftABS_B.brake_pressure_01 = SoftABS_P.Saturation_LowerSat;
  } else {
    /* Saturate: '<S2>/Saturation' */
    SoftABS_B.brake_pressure_01 = rtb_Divide;
  }

  /* End of Saturate: '<S2>/Saturation' */
}

/* Update for atomic system: '<S1>/ABS_EBD' */
void SoftABS::SoftABS_ABS_EBD_Update() {
  /* Abs: '<S2>/Abs' incorporates:
   *  Inport: '<Root>/ecu_in'
   */
  SoftABS_B.Abs = std::abs(SoftABS_U.ecu_in.slip_ratio[0]);

  /* Relay: '<S2>/OnAtEnableRef_OffAtDisableRef' */
  if (rtsiIsModeUpdateTimeStep(&(&SoftABS_M)->solverInfo)) {
    SoftABS_DW.OnAtEnableRef_OffAtDisableRef_Mode =
        ((SoftABS_B.Abs >= SoftABS_P.slip_ratio_ref) ||
         ((!(SoftABS_B.Abs <= SoftABS_P.slip_disable_ratio_ref)) && SoftABS_DW.OnAtEnableRef_OffAtDisableRef_Mode));
  }

  if (SoftABS_DW.OnAtEnableRef_OffAtDisableRef_Mode) {
    /* Relay: '<S2>/OnAtEnableRef_OffAtDisableRef' */
    SoftABS_B.OnAtEnableRef_OffAtDisableRef = SoftABS_P.OnAtEnableRef_OffAtDisableRef_YOn;
  } else {
    /* Relay: '<S2>/OnAtEnableRef_OffAtDisableRef' */
    SoftABS_B.OnAtEnableRef_OffAtDisableRef = SoftABS_P.OnAtEnableRef_OffAtDisableRef_YOff;
  }

  /* End of Relay: '<S2>/OnAtEnableRef_OffAtDisableRef' */

  /* Logic: '<S2>/if_on' incorporates:
   *  Constant: '<S2>/abs_switch'
   */
  SoftABS_B.abs_on = ((SoftABS_B.OnAtEnableRef_OffAtDisableRef != 0.0) && (SoftABS_P.enable_abs != 0.0));

  /* Product: '<S2>/ABS_flag' incorporates:
   *  Constant: '<S2>/Constant1'
   */
  SoftABS_B.ABS_flag[0] = static_cast<real_T>(SoftABS_B.abs_on) * SoftABS_P.Constant1_Value[0];

  /* Product: '<S2>/Product1' */
  SoftABS_B.Product1[0] = static_cast<real_T>(SoftABS_B.Merge1) * SoftABS_B.ABS_flag[0];

  /* Update for Memory: '<S2>/Memory' */
  SoftABS_DW.Memory_PreviousInput[0] = SoftABS_B.Product1[0];

  /* Product: '<S2>/ABS_flag' incorporates:
   *  Constant: '<S2>/Constant1'
   */
  SoftABS_B.ABS_flag[1] = static_cast<real_T>(SoftABS_B.abs_on) * SoftABS_P.Constant1_Value[1];

  /* Product: '<S2>/Product1' */
  SoftABS_B.Product1[1] = static_cast<real_T>(SoftABS_B.Merge1) * SoftABS_B.ABS_flag[1];

  /* Update for Memory: '<S2>/Memory' */
  SoftABS_DW.Memory_PreviousInput[1] = SoftABS_B.Product1[1];

  /* Product: '<S2>/ABS_flag' incorporates:
   *  Constant: '<S2>/Constant1'
   */
  SoftABS_B.ABS_flag[2] = static_cast<real_T>(SoftABS_B.abs_on) * SoftABS_P.Constant1_Value[2];

  /* Product: '<S2>/Product1' */
  SoftABS_B.Product1[2] = static_cast<real_T>(SoftABS_B.Merge1) * SoftABS_B.ABS_flag[2];

  /* Update for Memory: '<S2>/Memory' */
  SoftABS_DW.Memory_PreviousInput[2] = SoftABS_B.Product1[2];

  /* Product: '<S2>/ABS_flag' incorporates:
   *  Constant: '<S2>/Constant1'
   */
  SoftABS_B.ABS_flag[3] = static_cast<real_T>(SoftABS_B.abs_on) * SoftABS_P.Constant1_Value[3];

  /* Product: '<S2>/Product1' */
  SoftABS_B.Product1[3] = static_cast<real_T>(SoftABS_B.Merge1) * SoftABS_B.ABS_flag[3];

  /* Update for Memory: '<S2>/Memory' */
  SoftABS_DW.Memory_PreviousInput[3] = SoftABS_B.Product1[3];

  /* Update for If: '<S2>/If' */
  if (SoftABS_DW.If_ActiveSubsystem == 1) {
    /* Update for IfAction SubSystem: '<S2>/accControl' incorporates:
     *  ActionPort: '<S4>/Action Port'
     */
    /* Update for DiscreteIntegrator: '<S40>/Filter' */
    SoftABS_DW.Filter_DSTATE += SoftABS_P.Filter_gainval * SoftABS_B.FilterCoefficient;
    SoftABS_DW.Filter_PrevResetState = static_cast<int8_T>(SoftABS_B.LogicalOperator);

    /* Update for DiscreteIntegrator: '<S45>/Integrator' incorporates:
     *  DiscreteIntegrator: '<S40>/Filter'
     */
    SoftABS_DW.Integrator_DSTATE += SoftABS_P.Integrator_gainval * SoftABS_B.IntegralGain;
    SoftABS_DW.Integrator_PrevResetState = static_cast<int8_T>(SoftABS_B.LogicalOperator);

    /* End of Update for SubSystem: '<S2>/accControl' */
  }

  /* End of Update for If: '<S2>/If' */
}

/* Model step function */
void SoftABS::step() {
  int32_T rtb_autoHoldSwitch;
  boolean_T accMode;
  boolean_T gearDrive;
  boolean_T gearReverse;
  boolean_T rtb_Compare_d;
  boolean_T torqueMode;

  /* RelationalOperator: '<S65>/Compare' incorporates:
   *  Abs: '<S64>/Abs'
   *  Constant: '<S65>/Constant'
   *  Inport: '<Root>/ecu_in'
   */
  rtb_Compare_d = (std::abs(SoftABS_U.ecu_in.veh_speed_vx) < SoftABS_P.speedOffset);

  /* MATLAB Function: '<S3>/AutoHold' incorporates:
   *  Constant: '<S3>/Constant2'
   *  Inport: '<Root>/ecu_in'
   */
  rtb_autoHoldSwitch = 0;
  accMode = (SoftABS_U.ecu_in.driver_input.acc_control_mode == 0);
  torqueMode = (SoftABS_U.ecu_in.driver_input.acc_control_mode == 1);
  gearDrive = (SoftABS_U.ecu_in.driver_input.gear_cmd__0N1D2R3P == 1);
  gearReverse = (SoftABS_U.ecu_in.driver_input.gear_cmd__0N1D2R3P == 2);
  if (rtb_Compare_d && ((SoftABS_U.ecu_in.driver_input.gear_cmd__0N1D2R3P == 0) ||
                        (SoftABS_U.ecu_in.driver_input.gear_cmd__0N1D2R3P == 3))) {
    rtb_autoHoldSwitch = 1;
  }

  if (rtb_Compare_d && accMode && (SoftABS_U.ecu_in.driver_input.target_acc_m_s2 < 0.001) && gearDrive) {
    rtb_autoHoldSwitch = 1;
  }

  if (rtb_Compare_d && torqueMode && (SoftABS_U.ecu_in.driver_input.request_torque < 1.0) && gearDrive) {
    rtb_autoHoldSwitch = 1;
  }

  if (rtb_Compare_d && accMode && (SoftABS_U.ecu_in.driver_input.target_acc_m_s2 > -0.001) && gearReverse) {
    rtb_autoHoldSwitch = 1;
  }

  if (rtb_Compare_d && torqueMode && gearReverse && (SoftABS_P.DriveLinePowType == 1.0) &&
      (SoftABS_U.ecu_in.driver_input.request_torque < 1.0)) {
    rtb_autoHoldSwitch = 1;
  }

  if (rtb_Compare_d && torqueMode && gearReverse && (SoftABS_P.DriveLinePowType == 2.0) &&
      (SoftABS_U.ecu_in.driver_input.request_torque > -1.0)) {
    rtb_autoHoldSwitch = 1;
  }

  /* End of MATLAB Function: '<S3>/AutoHold' */

  /* Outputs for Atomic SubSystem: '<S1>/ABS_EBD' */
  SoftABS_ABS_EBD();

  /* End of Outputs for SubSystem: '<S1>/ABS_EBD' */

  /* Outport: '<Root>/ecu_out' incorporates:
   *  BusCreator: '<S1>/Bus Creator'
   */
  SoftABS_Y.ecu_out.pt_trq_cmd_Nm = 0.0;

  /* Switch: '<S1>/Switch1' */
  if (rtb_autoHoldSwitch > SoftABS_P.Switch1_Threshold) {
    /* Outport: '<Root>/ecu_out' incorporates:
     *  Constant: '<S1>/autoHoldBrakePressure_01'
     */
    SoftABS_Y.ecu_out.brake_pressure_0_1 = SoftABS_P.autoHoldBrakePressure_01_Value;
  } else {
    /* Outport: '<Root>/ecu_out' */
    SoftABS_Y.ecu_out.brake_pressure_0_1 = SoftABS_B.brake_pressure_01;
  }

  /* End of Switch: '<S1>/Switch1' */

  /* Outport: '<Root>/ecu_out' incorporates:
   *  BusCreator: '<S1>/Bus Creator'
   *  Constant: '<S1>/ZeroBrakePressure'
   */
  SoftABS_Y.ecu_out.brake_pressure_Pa[0] = SoftABS_P.ZeroBrakePressure_Value;
  SoftABS_Y.ecu_out.brake_pressure_Pa[1] = SoftABS_P.ZeroBrakePressure_Value;
  SoftABS_Y.ecu_out.brake_pressure_Pa[2] = SoftABS_P.ZeroBrakePressure_Value;
  SoftABS_Y.ecu_out.brake_pressure_Pa[3] = SoftABS_P.ZeroBrakePressure_Value;
  SoftABS_Y.ecu_out.abs_flag[0] = SoftABS_B.Memory[0];
  SoftABS_Y.ecu_out.abs_flag[1] = SoftABS_B.Memory[1];
  SoftABS_Y.ecu_out.abs_flag[2] = SoftABS_B.Memory[2];
  SoftABS_Y.ecu_out.abs_flag[3] = SoftABS_B.Memory[3];
  SoftABS_Y.ecu_out.throttle_01 = 0.0;

  /* Update for Atomic SubSystem: '<S1>/ABS_EBD' */
  SoftABS_ABS_EBD_Update();

  /* End of Update for SubSystem: '<S1>/ABS_EBD' */

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The absolute time is the multiplication of "clockTick0"
   * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
   * overflow during the application lifespan selected.
   * Timer of this task consists of two 32 bit unsigned integers.
   * The two integers represent the low bits Timing.clockTick0 and the high bits
   * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
   */
  if (!(++(&SoftABS_M)->Timing.clockTick0)) {
    ++(&SoftABS_M)->Timing.clockTickH0;
  }

  (&SoftABS_M)->Timing.t[0] = (&SoftABS_M)->Timing.clockTick0 * (&SoftABS_M)->Timing.stepSize0 +
                              (&SoftABS_M)->Timing.clockTickH0 * (&SoftABS_M)->Timing.stepSize0 * 4294967296.0;

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
    (&SoftABS_M)->Timing.clockTick1++;
    if (!(&SoftABS_M)->Timing.clockTick1) {
      (&SoftABS_M)->Timing.clockTickH1++;
    }
  }
}

/* Model initialize function */
void SoftABS::initialize() {
  /* Registration code */
  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&SoftABS_M)->solverInfo, &(&SoftABS_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&SoftABS_M)->solverInfo, &rtmGetTPtr((&SoftABS_M)));
    rtsiSetStepSizePtr(&(&SoftABS_M)->solverInfo, &(&SoftABS_M)->Timing.stepSize0);
    rtsiSetErrorStatusPtr(&(&SoftABS_M)->solverInfo, (&rtmGetErrorStatus((&SoftABS_M))));
    rtsiSetRTModelPtr(&(&SoftABS_M)->solverInfo, (&SoftABS_M));
  }

  rtsiSetSimTimeStep(&(&SoftABS_M)->solverInfo, MAJOR_TIME_STEP);
  rtsiSetSolverName(&(&SoftABS_M)->solverInfo, "FixedStepDiscrete");
  rtmSetTPtr((&SoftABS_M), &(&SoftABS_M)->Timing.tArray[0]);
  (&SoftABS_M)->Timing.stepSize0 = 0.001;

  /* SystemInitialize for Atomic SubSystem: '<S1>/ABS_EBD' */
  SoftABS_ABS_EBD_Init();

  /* End of SystemInitialize for SubSystem: '<S1>/ABS_EBD' */
}

/* Model terminate function */
void SoftABS::terminate() { /* (no terminate code required) */
}

/* Constructor */
SoftABS::SoftABS() : SoftABS_U(), SoftABS_Y(), SoftABS_B(), SoftABS_DW(), SoftABS_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
SoftABS::~SoftABS() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_SoftABS_T* SoftABS::getRTM() { return (&SoftABS_M); }
