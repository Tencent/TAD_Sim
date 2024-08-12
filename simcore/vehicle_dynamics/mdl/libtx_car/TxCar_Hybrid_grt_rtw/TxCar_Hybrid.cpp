/*
 * TxCar_Hybrid.cpp
 *
 * Code generation for model "TxCar_Hybrid".
 *
 * Model version              : 2.2139
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Fri Jul 28 13:31:29 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TxCar_Hybrid.h"
#include <cmath>
#include "TxCar_Hybrid_private.h"
#include "rtwtypes.h"

extern "C" {

#include "rt_nonfinite.h"
}

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void TxCar_Hybrid::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  int_T nXc{5};

  rtsiSetSimTimeStep(si, MINOR_TIME_STEP);

  /* Save the state values at time t in y, we'll use x as ynew. */
  (void)std::memcpy(y, x, static_cast<uint_T>(nXc) * sizeof(real_T));

  /* Assumes that rtsiSetT and ModelOutputs are up-to-date */
  /* f0 = f(t,y) */
  rtsiSetdX(si, f0);
  TxCar_Hybrid_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  TxCar_Hybrid_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  TxCar_Hybrid_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  TxCar_Hybrid_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Model step function */
void TxCar_Hybrid::step() {
  /* local block i/o variables */
  real_T rtb_FilterCoefficient;
  real_T rtb_Merge1;
  real_T rtb_Product_a;
  real_T rtb_batt_soc_null;
  real_T rtb_eng_trq_Nm;
  real_T rtb_IntegralGain;
  real_T rtb_wheel_angle[4];
  real_T rtb_AxlTrq[4];
  boolean_T rtb_Reset_Throttle;
  real_T rtb_ABSFlag_idx_0;
  real_T rtb_ABSFlag_idx_1;
  real_T rtb_ABSFlag_idx_2;
  real_T rtb_ABSFlag_idx_3;
  real_T rtb_Add_p;
  real_T rtb_DiffPrsCmd_idx_0;
  real_T rtb_DiffPrsCmd_idx_1;
  real_T rtb_DiffPrsCmd_idx_2;
  real_T rtb_DiffPrsCmd_idx_3;
  real_T rtb_Product_k;
  real_T rtb_Saturation_idx_0;
  real_T rtb_Saturation_idx_1;
  real_T rtb_Saturation_idx_2;
  real_T rtb_Saturation_idx_3;
  real_T rtb_batt_volt_V;
  real_T rtb_brake_pressure_Pa_idx_0;
  real_T rtb_brake_pressure_Pa_idx_1;
  real_T rtb_brake_pressure_Pa_idx_2;
  real_T rtb_brake_pressure_Pa_idx_3;
  real_T rtb_current;
  real_T rtb_eng_trqNm;
  real_T rtb_mot_front_trqNm;
  real_T rtb_mot_rear_trqNm;
  real_T rtb_throttle_01;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* set solver stop time */
    if (!((&TxCar_Hybrid_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&TxCar_Hybrid_M)->solverInfo, (((&TxCar_Hybrid_M)->Timing.clockTickH0 + 1) *
                                                             (&TxCar_Hybrid_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(
          &(&TxCar_Hybrid_M)->solverInfo,
          (((&TxCar_Hybrid_M)->Timing.clockTick0 + 1) * (&TxCar_Hybrid_M)->Timing.stepSize0 +
           (&TxCar_Hybrid_M)->Timing.clockTickH0 * (&TxCar_Hybrid_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&TxCar_Hybrid_M))) {
    (&TxCar_Hybrid_M)->Timing.t[0] = rtsiGetT(&(&TxCar_Hybrid_M)->solverInfo);
  }

  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* Constant: '<S5>/Constant3' incorporates:
     *  BusCreator generated from: '<S2>/Chassis'
     */
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[0] = TxCar_Hybrid_P.Constant3_Value;

    /* Constant: '<S5>/Constant4' incorporates:
     *  BusCreator generated from: '<S2>/Chassis'
     */
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[1] = TxCar_Hybrid_P.Constant4_Value;

    /* Constant: '<S5>/Constant5' incorporates:
     *  BusCreator generated from: '<S2>/Chassis'
     */
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[2] = TxCar_Hybrid_P.Constant5_Value;

    /* Switch: '<S36>/Switch' incorporates:
     *  Constant: '<S37>/Constant'
     *  Inport generated from: '<Root>/Friction'
     *  RelationalOperator: '<S37>/Compare'
     */
    if (TxCar_Hybrid_U.Friction.mu_FL <= TxCar_Hybrid_P.CompareToConstant_const) {
      /* BusCreator generated from: '<S2>/Chassis' incorporates:
       *  Constant: '<S36>/const_mu'
       */
      TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FL =
          TxCar_Hybrid_P.const_mu_Value[0];
      TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FR =
          TxCar_Hybrid_P.const_mu_Value[1];
      TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RL =
          TxCar_Hybrid_P.const_mu_Value[2];
      TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RR =
          TxCar_Hybrid_P.const_mu_Value[3];
    } else {
      /* BusCreator generated from: '<S2>/Chassis' */
      TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FL =
          TxCar_Hybrid_U.Friction.mu_FL;
      TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FR =
          TxCar_Hybrid_U.Friction.mu_FR;
      TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RL =
          TxCar_Hybrid_U.Friction.mu_RL;
      TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RR =
          TxCar_Hybrid_U.Friction.mu_RR;
    }

    /* End of Switch: '<S36>/Switch' */

    /* BusCreator generated from: '<S2>/Chassis' incorporates:
     *  Inport generated from: '<Root>/Height'
     */
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.z = TxCar_Hybrid_U.Height;

    /* BusCreator generated from: '<S2>/Chassis' incorporates:
     *  Memory generated from: '<S9>/Memory'
     */
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[0] =
        TxCar_Hybrid_DW.Memory_1_PreviousInput[0];
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[0] =
        TxCar_Hybrid_DW.Memory_2_PreviousInput[0];
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[0] =
        TxCar_Hybrid_DW.Memory_3_PreviousInput[0];
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[1] =
        TxCar_Hybrid_DW.Memory_1_PreviousInput[1];
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[1] =
        TxCar_Hybrid_DW.Memory_2_PreviousInput[1];
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[1] =
        TxCar_Hybrid_DW.Memory_3_PreviousInput[1];
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[2] =
        TxCar_Hybrid_DW.Memory_1_PreviousInput[2];
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[2] =
        TxCar_Hybrid_DW.Memory_2_PreviousInput[2];
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[2] =
        TxCar_Hybrid_DW.Memory_3_PreviousInput[2];
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[3] =
        TxCar_Hybrid_DW.Memory_1_PreviousInput[3];
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[3] =
        TxCar_Hybrid_DW.Memory_2_PreviousInput[3];
    TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[3] =
        TxCar_Hybrid_DW.Memory_3_PreviousInput[3];

    /* S-Function (chassis_sfunc): '<S2>/Chassis' incorporates:
     *  Outport generated from: '<Root>/Body'
     *  Outport generated from: '<Root>/Susp'
     *  Outport generated from: '<Root>/WheelsOut'
     */
    chassis_sfunc_Outputs_wrapper_cgen(&TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1,
                                       &TxCar_Hybrid_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1,
                                       &TxCar_Hybrid_Y.Body, &TxCar_Hybrid_Y.WheelsOut, &TxCar_Hybrid_Y.Susp,
                                       TxCar_Hybrid_P.Chassis_P1, 21, TxCar_Hybrid_P.Chassis_P2, 38);

    /* Gain: '<S115>/Gain' incorporates:
     *  Outport generated from: '<Root>/Body'
     */
    TxCar_Hybrid_B.ax = TxCar_Hybrid_P.Gain_Gain * TxCar_Hybrid_Y.Body.BdyFrm.Cg.Acc.ax;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' incorporates:
     *  Memory generated from: '<S40>/Memory'
     */
    TxCar_Hybrid_Y.PowerBus_Out_battery_info_batt_soc_null = TxCar_Hybrid_DW.Memory_5_PreviousInput;
  }

  /* Integrator: '<S18>/Integrator1' */
  TxCar_Hybrid_B.Integrator1 = TxCar_Hybrid_X.Integrator1_CSTATE;

  /* BusCreator: '<S115>/Bus Creator1' incorporates:
   *  Constant: '<S119>/Constant1'
   *  Inport generated from: '<Root>/DriverIn'
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/PowerBus_Outport_2'
   *  Outport generated from: '<Root>/WheelsOut'
   */
  TxCar_Hybrid_B.BusCreator1.acc_feedback_m_s2 = TxCar_Hybrid_B.ax;
  TxCar_Hybrid_B.BusCreator1.eng_spd_rad_s = TxCar_Hybrid_B.Integrator1;
  TxCar_Hybrid_B.BusCreator1.batt_soc_0_1 = TxCar_Hybrid_Y.PowerBus_Out_battery_info_batt_soc_null;
  TxCar_Hybrid_B.BusCreator1.diff_trq_req_Nm = TxCar_Hybrid_P.Constant1_Value_k;
  TxCar_Hybrid_B.BusCreator1.slip_ratio[0] = TxCar_Hybrid_Y.WheelsOut.TireFrame.Kappa[0];
  TxCar_Hybrid_B.BusCreator1.slip_ratio[1] = TxCar_Hybrid_Y.WheelsOut.TireFrame.Kappa[1];
  TxCar_Hybrid_B.BusCreator1.slip_ratio[2] = TxCar_Hybrid_Y.WheelsOut.TireFrame.Kappa[2];
  TxCar_Hybrid_B.BusCreator1.slip_ratio[3] = TxCar_Hybrid_Y.WheelsOut.TireFrame.Kappa[3];
  TxCar_Hybrid_B.BusCreator1.veh_speed_vx = TxCar_Hybrid_Y.Body.BdyFrm.Cg.Vel.xdot;
  TxCar_Hybrid_B.BusCreator1.driver_input = TxCar_Hybrid_U.DriverIn;

  /* Integrator: '<S20>/Integrator1' */
  TxCar_Hybrid_B.Integrator1_j = TxCar_Hybrid_X.Integrator1_CSTATE_c;

  /* Integrator: '<S25>/Integrator1' */
  TxCar_Hybrid_B.Integrator1_e = TxCar_Hybrid_X.Integrator1_CSTATE_a;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* S-Function (softhcu_sfunction): '<S120>/S-Function' incorporates:
     *  Outport generated from: '<Root>/OutBus_Outport_8'
     */
    softhcu_sfunction_Outputs_wrapper_cgen(&TxCar_Hybrid_B.BusCreator1, &TxCar_Hybrid_B.Integrator1_j,
                                           &TxCar_Hybrid_B.Integrator1_e, &TxCar_Hybrid_B.Integrator1,
                                           &TxCar_Hybrid_B.SFunction_o1, &TxCar_Hybrid_Y.OutBus_HEV_Output,
                                           TxCar_Hybrid_P.SFunction_P1, 21, TxCar_Hybrid_P.SFunction_P2, 38);

    /* Gain: '<S28>/Gain5' */
    TxCar_Hybrid_B.acc_cmd = TxCar_Hybrid_P.Gain5_Gain * TxCar_Hybrid_B.SFunction_o1.throttle_01;

    /* Gain: '<S28>/Gain1' incorporates:
     *  Constant: '<S28>/Constant1'
     */
    TxCar_Hybrid_B.brake_cmd_0_1 = TxCar_Hybrid_P.Gain1_Gain * TxCar_Hybrid_P.Constant1_Value_f;

    /* Gain: '<S28>/Gain2' incorporates:
     *  Outport generated from: '<Root>/WheelsOut'
     */
    TxCar_Hybrid_B.wheel_spd_rad_s[0] = TxCar_Hybrid_P.Gain2_Gain * TxCar_Hybrid_Y.WheelsOut.TireFrame.Omega[0];
    TxCar_Hybrid_B.wheel_spd_rad_s[1] = TxCar_Hybrid_P.Gain2_Gain * TxCar_Hybrid_Y.WheelsOut.TireFrame.Omega[1];
    TxCar_Hybrid_B.wheel_spd_rad_s[2] = TxCar_Hybrid_P.Gain2_Gain * TxCar_Hybrid_Y.WheelsOut.TireFrame.Omega[2];
    TxCar_Hybrid_B.wheel_spd_rad_s[3] = TxCar_Hybrid_P.Gain2_Gain * TxCar_Hybrid_Y.WheelsOut.TireFrame.Omega[3];

    /* Gain: '<S28>/Gain3' incorporates:
     *  Constant: '<S28>/Constant2'
     */
    TxCar_Hybrid_B.abs_flag = TxCar_Hybrid_P.Gain3_Gain * TxCar_Hybrid_P.Constant2_Value_j;

    /* Memory generated from: '<S40>/Memory' */
    rtb_eng_trqNm = TxCar_Hybrid_DW.Memory_1_PreviousInput_i;

    /* Gain: '<S28>/Gain4' incorporates:
     *  Memory generated from: '<S40>/Memory'
     */
    TxCar_Hybrid_B.engine_trq_Nm = TxCar_Hybrid_P.Gain4_Gain * TxCar_Hybrid_DW.Memory_1_PreviousInput_i;
  }

  /* MultiPortSwitch: '<S32>/Multiport Switch' incorporates:
   *  Inport generated from: '<Root>/DriverIn'
   */
  switch (TxCar_Hybrid_U.DriverIn.gear_cmd__0N1D2R3P) {
    case 0:
      /* BusCreator: '<S28>/Bus Creator' incorporates:
       *  Constant: '<S32>/N'
       */
      TxCar_Hybrid_B.driveline.gear_cmd_driver = TxCar_Hybrid_P.N_Value;
      break;

    case 1:
      /* BusCreator: '<S28>/Bus Creator' incorporates:
       *  Constant: '<S32>/D'
       */
      TxCar_Hybrid_B.driveline.gear_cmd_driver = TxCar_Hybrid_P.D_Value;
      break;

    case 2:
      /* BusCreator: '<S28>/Bus Creator' incorporates:
       *  Constant: '<S32>/R'
       */
      TxCar_Hybrid_B.driveline.gear_cmd_driver = TxCar_Hybrid_P.R_Value;
      break;

    default:
      /* BusCreator: '<S28>/Bus Creator' incorporates:
       *  Constant: '<S32>/P'
       */
      TxCar_Hybrid_B.driveline.gear_cmd_driver = TxCar_Hybrid_P.P_Value;
      break;
  }

  /* End of MultiPortSwitch: '<S32>/Multiport Switch' */
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* SignalConversion generated from: '<S119>/Vector Concatenate2' incorporates:
     *  Constant: '<S119>/Constant'
     */
    rtb_DiffPrsCmd_idx_0 = TxCar_Hybrid_P.Constant_Value_e;

    /* SignalConversion generated from: '<S119>/Vector Concatenate2' incorporates:
     *  Constant: '<S119>/Constant'
     */
    rtb_DiffPrsCmd_idx_1 = TxCar_Hybrid_P.Constant_Value_e;

    /* SignalConversion generated from: '<S119>/Vector Concatenate2' incorporates:
     *  Constant: '<S119>/Constant'
     */
    rtb_DiffPrsCmd_idx_2 = TxCar_Hybrid_P.Constant_Value_e;

    /* SignalConversion generated from: '<S119>/Vector Concatenate2' incorporates:
     *  Constant: '<S119>/Constant'
     */
    rtb_DiffPrsCmd_idx_3 = TxCar_Hybrid_P.Constant_Value_e;

    /* DataTypeConversion: '<S28>/Data Type Conversion' incorporates:
     *  Constant: '<S119>/Constant'
     */
    TxCar_Hybrid_B.diff_prs_cmd[0] = (TxCar_Hybrid_P.Constant_Value_e != 0.0);
    TxCar_Hybrid_B.diff_prs_cmd[1] = (TxCar_Hybrid_P.Constant_Value_e != 0.0);
    TxCar_Hybrid_B.diff_prs_cmd[2] = (TxCar_Hybrid_P.Constant_Value_e != 0.0);
    TxCar_Hybrid_B.diff_prs_cmd[3] = (TxCar_Hybrid_P.Constant_Value_e != 0.0);

    /* Memory generated from: '<S40>/Memory' */
    rtb_mot_front_trqNm = TxCar_Hybrid_DW.Memory_2_PreviousInput_p;

    /* Gain: '<S28>/Gain6' incorporates:
     *  Memory generated from: '<S40>/Memory'
     */
    TxCar_Hybrid_B.mot_front_trq_Nm = TxCar_Hybrid_P.Gain6_Gain * TxCar_Hybrid_DW.Memory_2_PreviousInput_p;

    /* Memory generated from: '<S40>/Memory' */
    rtb_mot_rear_trqNm = TxCar_Hybrid_DW.Memory_3_PreviousInput_k;

    /* S-Function (hybridparser_sfunction): '<S6>/S-Function' incorporates:
     *  Constant: '<S6>/NoUse'
     */
    hybridparser_sfunction_Outputs_wrapper_cgen(
        &TxCar_Hybrid_P.NoUse_Value, &TxCar_Hybrid_B.EnableP2, &TxCar_Hybrid_B.EnableP3, &TxCar_Hybrid_B.EnableP4,
        &TxCar_Hybrid_B.EngToGenerator_GearRatio, &TxCar_Hybrid_B.EngClutch_TimeConst,
        &TxCar_Hybrid_B.FrontMotGearRatio, TxCar_Hybrid_P.SFunction_P1_b, 21, TxCar_Hybrid_P.SFunction_P2_m, 38);

    /* Gain: '<S28>/Gain7' incorporates:
     *  Memory generated from: '<S40>/Memory'
     *  Product: '<S28>/Product'
     */
    TxCar_Hybrid_B.mot_rear_trq_Nm =
        TxCar_Hybrid_DW.Memory_3_PreviousInput_k * TxCar_Hybrid_B.EnableP4 * TxCar_Hybrid_P.Gain7_Gain;
  }

  /* BusCreator: '<S28>/Bus Creator' */
  TxCar_Hybrid_B.driveline.acc_cmd = TxCar_Hybrid_B.acc_cmd;
  TxCar_Hybrid_B.driveline.brake_cmd_0_1 = TxCar_Hybrid_B.brake_cmd_0_1;
  TxCar_Hybrid_B.driveline.abs_flag = TxCar_Hybrid_B.abs_flag;
  TxCar_Hybrid_B.driveline.engine_trq_Nm = TxCar_Hybrid_B.engine_trq_Nm;
  TxCar_Hybrid_B.driveline.wheel_spd_rad_s[0] = TxCar_Hybrid_B.wheel_spd_rad_s[0];
  TxCar_Hybrid_B.driveline.diff_prs_cmd[0] = TxCar_Hybrid_B.diff_prs_cmd[0];
  TxCar_Hybrid_B.driveline.wheel_spd_rad_s[1] = TxCar_Hybrid_B.wheel_spd_rad_s[1];
  TxCar_Hybrid_B.driveline.diff_prs_cmd[1] = TxCar_Hybrid_B.diff_prs_cmd[1];
  TxCar_Hybrid_B.driveline.wheel_spd_rad_s[2] = TxCar_Hybrid_B.wheel_spd_rad_s[2];
  TxCar_Hybrid_B.driveline.diff_prs_cmd[2] = TxCar_Hybrid_B.diff_prs_cmd[2];
  TxCar_Hybrid_B.driveline.wheel_spd_rad_s[3] = TxCar_Hybrid_B.wheel_spd_rad_s[3];
  TxCar_Hybrid_B.driveline.diff_prs_cmd[3] = TxCar_Hybrid_B.diff_prs_cmd[3];
  TxCar_Hybrid_B.driveline.mot_front_trq_Nm = TxCar_Hybrid_B.mot_front_trq_Nm;
  TxCar_Hybrid_B.driveline.mot_rear_trq_Nm = TxCar_Hybrid_B.mot_rear_trq_Nm;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* S-Function (driveline_ev_rear_sfunction): '<S16>/S-Function' */
    driveline_ev_rear_sfunction_Outputs_wrapper_cgen(&TxCar_Hybrid_B.driveline, &TxCar_Hybrid_B.SFunction,
                                                     TxCar_Hybrid_P.SFunction_P1_l, 21, TxCar_Hybrid_P.SFunction_P2_m5,
                                                     38);

    /* Memory: '<S27>/Memory' */
    TxCar_Hybrid_B.Memory = TxCar_Hybrid_DW.Memory_PreviousInput;
  }

  /* MultiPortSwitch: '<S24>/P3_Trq_Selector' */
  if (static_cast<int32_T>(TxCar_Hybrid_B.EnableP3) == 0) {
    /* MultiPortSwitch: '<S24>/P3_Trq_Selector' */
    TxCar_Hybrid_B.P3_Trq_Selector = TxCar_Hybrid_B.Memory;
  } else {
    /* MultiPortSwitch: '<S24>/P3_Trq_Selector' incorporates:
     *  Product: '<S24>/Product'
     *  Sum: '<S24>/P3_Trq_Couple'
     */
    TxCar_Hybrid_B.P3_Trq_Selector =
        TxCar_Hybrid_B.driveline.mot_front_trq_Nm * TxCar_Hybrid_B.FrontMotGearRatio + TxCar_Hybrid_B.Memory;
  }

  /* End of MultiPortSwitch: '<S24>/P3_Trq_Selector' */
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* SignalConversion generated from: '<S17>/S-Function' */
    TxCar_Hybrid_B.TmpSignalConversionAtSFunctionInport3[0] = 0.0;
    TxCar_Hybrid_B.TmpSignalConversionAtSFunctionInport3[1] = 0.0;
    TxCar_Hybrid_B.TmpSignalConversionAtSFunctionInport3[2] = 0.0;
    TxCar_Hybrid_B.TmpSignalConversionAtSFunctionInport3[3] = 0.0;

    /* S-Function (driveline_ice_sfunction): '<S17>/S-Function' */
    driveline_ice_sfunction_Outputs_wrapper_cgen(
        &TxCar_Hybrid_B.P3_Trq_Selector, &TxCar_Hybrid_B.driveline.wheel_spd_rad_s[0],
        &TxCar_Hybrid_B.TmpSignalConversionAtSFunctionInport3[0], &TxCar_Hybrid_B.SFunction_o1_m[0],
        &TxCar_Hybrid_B.SFunction_o2_b, TxCar_Hybrid_P.SFunction_P1_lm, 21, TxCar_Hybrid_P.SFunction_P2_n, 38);
  }

  /* MultiPortSwitch: '<S23>/P2_TrqRouter' */
  if (static_cast<int32_T>(TxCar_Hybrid_B.EnableP2) == 0) {
    /* BusCreator generated from: '<S27>/S-Function' */
    TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.engine_trq_Nm =
        TxCar_Hybrid_B.driveline.engine_trq_Nm;
  } else {
    /* BusCreator generated from: '<S27>/S-Function' incorporates:
     *  Sum: '<S23>/P2_TrqCouple'
     */
    TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.engine_trq_Nm =
        TxCar_Hybrid_B.driveline.engine_trq_Nm + TxCar_Hybrid_B.driveline.mot_front_trq_Nm;
  }

  /* End of MultiPortSwitch: '<S23>/P2_TrqRouter' */

  /* BusCreator generated from: '<S27>/S-Function' */
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.acc_cmd = TxCar_Hybrid_B.driveline.acc_cmd;
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.brake_cmd_0_1 =
      TxCar_Hybrid_B.driveline.brake_cmd_0_1;
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.abs_flag =
      TxCar_Hybrid_B.driveline.abs_flag;
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.gear_cmd_driver =
      TxCar_Hybrid_B.driveline.gear_cmd_driver;
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.wheel_spd_rad_s[0] =
      TxCar_Hybrid_B.driveline.wheel_spd_rad_s[0];
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.diff_prs_cmd[0] =
      TxCar_Hybrid_B.driveline.diff_prs_cmd[0];
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.wheel_spd_rad_s[1] =
      TxCar_Hybrid_B.driveline.wheel_spd_rad_s[1];
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.diff_prs_cmd[1] =
      TxCar_Hybrid_B.driveline.diff_prs_cmd[1];
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.wheel_spd_rad_s[2] =
      TxCar_Hybrid_B.driveline.wheel_spd_rad_s[2];
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.diff_prs_cmd[2] =
      TxCar_Hybrid_B.driveline.diff_prs_cmd[2];
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.wheel_spd_rad_s[3] =
      TxCar_Hybrid_B.driveline.wheel_spd_rad_s[3];
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.diff_prs_cmd[3] =
      TxCar_Hybrid_B.driveline.diff_prs_cmd[3];
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.mot_front_trq_Nm =
      TxCar_Hybrid_B.driveline.mot_front_trq_Nm;
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.mot_rear_trq_Nm =
      TxCar_Hybrid_B.driveline.mot_rear_trq_Nm;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* S-Function (transmission_sfunction): '<S27>/S-Function' */
    transmission_sfunction_Outputs_wrapper_cgen(
        &TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1, &TxCar_Hybrid_B.SFunction_o1,
        &TxCar_Hybrid_B.SFunction_o2_b, &TxCar_Hybrid_B.SFunction_o1_e, &TxCar_Hybrid_B.SFunction_o2_h,
        &TxCar_Hybrid_B.SFunction_o3, TxCar_Hybrid_P.SFunction_P1_j, 21, TxCar_Hybrid_P.SFunction_P2_e, 38);

    /* Outputs for Enabled SubSystem: '<S19>/P2_HEV' incorporates:
     *  EnablePort: '<S29>/Enable'
     */
    if (TxCar_Hybrid_B.EnableP2 > 0.0) {
      /* Merge: '<S19>/Merge' incorporates:
       *  SignalConversion generated from: '<S29>/DriveShaftSpd[rad|s]'
       */
      TxCar_Hybrid_B.Merge = TxCar_Hybrid_B.SFunction_o2_h;
    }

    /* End of Outputs for SubSystem: '<S19>/P2_HEV' */

    /* Outputs for Enabled SubSystem: '<S19>/P3_HEV' incorporates:
     *  EnablePort: '<S30>/Enable'
     */
    if (TxCar_Hybrid_B.EnableP3 > 0.0) {
      /* Merge: '<S19>/Merge' incorporates:
       *  Product: '<S30>/Product'
       */
      TxCar_Hybrid_B.Merge = TxCar_Hybrid_B.SFunction_o2_b * TxCar_Hybrid_B.FrontMotGearRatio;
    }

    /* End of Outputs for SubSystem: '<S19>/P3_HEV' */

    /* BusCreator: '<S22>/Bus Creator2' incorporates:
     *  BusCreator: '<S21>/Bus Creator2'
     *  Product: '<S22>/Product'
     *  Product: '<S31>/Product1'
     *  Product: '<S31>/Product2'
     */
    TxCar_Hybrid_B.BusCreator2.gear_engaged = TxCar_Hybrid_B.SFunction_o1_e;
    TxCar_Hybrid_B.BusCreator2.eng_spd_rad_s = TxCar_Hybrid_B.SFunction_o2_h;
    TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[0] = TxCar_Hybrid_B.SFunction_o1_m[0];
    TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[1] = TxCar_Hybrid_B.SFunction_o1_m[1];
    TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[2] =
        TxCar_Hybrid_B.SFunction.wheel_drive_trq_Nm[2] * TxCar_Hybrid_B.EnableP4;
    TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[3] =
        TxCar_Hybrid_B.SFunction.wheel_drive_trq_Nm[3] * TxCar_Hybrid_B.EnableP4;
    TxCar_Hybrid_B.BusCreator2.trans_out_shaft_spd_rad_s = TxCar_Hybrid_B.SFunction_o2_b;
    TxCar_Hybrid_B.BusCreator2.front_mot_spd_rad_s = TxCar_Hybrid_B.Merge;
    TxCar_Hybrid_B.BusCreator2.rear_mot_spd_rad_s =
        TxCar_Hybrid_B.SFunction.rear_mot_spd_rad_s * TxCar_Hybrid_B.EnableP4;
  }

  /* Product: '<S18>/Divide' incorporates:
   *  Constant: '<S18>/Constant'
   *  Sum: '<S18>/Sum'
   */
  TxCar_Hybrid_B.Divide =
      (TxCar_Hybrid_B.BusCreator2.eng_spd_rad_s - TxCar_Hybrid_B.Integrator1) * TxCar_Hybrid_P.EngSpdAct_wc;

  /* Product: '<S20>/Divide' incorporates:
   *  Constant: '<S20>/Constant'
   *  Sum: '<S20>/Sum'
   */
  TxCar_Hybrid_B.Divide_h = (TxCar_Hybrid_B.BusCreator2.front_mot_spd_rad_s - TxCar_Hybrid_B.Integrator1_j) *
                            TxCar_Hybrid_P.FrontMotorSpd_Filter_wc;

  /* Product: '<S25>/Divide' incorporates:
   *  Constant: '<S25>/Constant'
   *  Sum: '<S25>/Sum'
   */
  TxCar_Hybrid_B.Divide_p = (TxCar_Hybrid_B.BusCreator2.rear_mot_spd_rad_s - TxCar_Hybrid_B.Integrator1_e) *
                            TxCar_Hybrid_P.RearMotorSpd_Filter_wc;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* Saturate: '<S12>/Saturation' */
    if (TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[0] > TxCar_Hybrid_P.Saturation_UpperSat) {
      rtb_Saturation_idx_0 = TxCar_Hybrid_P.Saturation_UpperSat;
    } else if (TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[0] < TxCar_Hybrid_P.Saturation_LowerSat) {
      rtb_Saturation_idx_0 = TxCar_Hybrid_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_0 = TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[0];
    }

    if (TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[1] > TxCar_Hybrid_P.Saturation_UpperSat) {
      rtb_Saturation_idx_1 = TxCar_Hybrid_P.Saturation_UpperSat;
    } else if (TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[1] < TxCar_Hybrid_P.Saturation_LowerSat) {
      rtb_Saturation_idx_1 = TxCar_Hybrid_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_1 = TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[1];
    }

    if (TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[2] > TxCar_Hybrid_P.Saturation_UpperSat) {
      rtb_Saturation_idx_2 = TxCar_Hybrid_P.Saturation_UpperSat;
    } else if (TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[2] < TxCar_Hybrid_P.Saturation_LowerSat) {
      rtb_Saturation_idx_2 = TxCar_Hybrid_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_2 = TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[2];
    }

    if (TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[3] > TxCar_Hybrid_P.Saturation_UpperSat) {
      rtb_Saturation_idx_3 = TxCar_Hybrid_P.Saturation_UpperSat;
    } else if (TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[3] < TxCar_Hybrid_P.Saturation_LowerSat) {
      rtb_Saturation_idx_3 = TxCar_Hybrid_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_3 = TxCar_Hybrid_B.BusCreator2.wheel_drive_trq_Nm[3];
    }

    /* End of Saturate: '<S12>/Saturation' */
  }

  /* Product: '<S26>/Divide' incorporates:
   *  Constant: '<S26>/Constant'
   *  Integrator: '<S26>/Integrator1'
   *  Sum: '<S26>/Sum'
   */
  TxCar_Hybrid_B.Divide_c =
      (TxCar_Hybrid_B.BusCreator2.trans_out_shaft_spd_rad_s - TxCar_Hybrid_X.Integrator1_CSTATE_e) *
      TxCar_Hybrid_P.TransOutSpd_Filter_wc;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    int8_T rtAction;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' incorporates:
     *  Gain: '<S114>/Gain2'
     */
    TxCar_Hybrid_Y.SoftEcu_Out_Brake_BrkPrsCmd01 =
        TxCar_Hybrid_P.Gain2_Gain_l * TxCar_Hybrid_B.SFunction_o1.brake_pressure_0_1;

    /* S-Function (brakehydraulic_sfunc): '<S15>/S-Function' incorporates:
     *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
     *  Outport generated from: '<Root>/SoftEcu_Outport_4'
     */
    brakehydraulic_sfunc_Outputs_wrapper_cgen(&TxCar_Hybrid_Y.SoftEcu_Out_Brake_BrkPrsCmd01,
                                              &TxCar_Hybrid_Y.DriveLineBus_Out_BrkPrs[0], TxCar_Hybrid_P.SFunction_P1_f,
                                              21, TxCar_Hybrid_P.SFunction_P2_i, 38);

    /* Memory generated from: '<S40>/Memory' */
    rtb_batt_volt_V = TxCar_Hybrid_DW.Memory_4_PreviousInput;

    /* Memory generated from: '<S40>/Memory' */
    rtb_current = TxCar_Hybrid_DW.Memory_6_PreviousInput;

    /* Memory: '<S44>/Memory' */
    TxCar_Hybrid_B.Memory_p = TxCar_Hybrid_DW.Memory_PreviousInput_p;

    /* SwitchCase: '<S44>/Switch Case' incorporates:
     *  Outport generated from: '<Root>/OutBus_Outport_8'
     */
    if (rtsiIsModeUpdateTimeStep(&(&TxCar_Hybrid_M)->solverInfo)) {
      if (static_cast<int32_T>(TxCar_Hybrid_Y.OutBus_HEV_Output.hev_mode_output.HEV_Mode) == 4) {
        rtAction = 0;
      } else {
        rtAction = 1;
      }

      TxCar_Hybrid_DW.SwitchCase_ActiveSubsystem = rtAction;
    } else {
      rtAction = TxCar_Hybrid_DW.SwitchCase_ActiveSubsystem;
    }

    switch (rtAction) {
      case 0:
        /* Outputs for IfAction SubSystem: '<S44>/SerializePwr' incorporates:
         *  ActionPort: '<S55>/Action Port'
         */
        /* Merge: '<S44>/Merge' incorporates:
         *  Product: '<S55>/Product'
         */
        TxCar_Hybrid_B.Merge_c = TxCar_Hybrid_B.Memory_p * TxCar_Hybrid_B.EngToGenerator_GearRatio;

        /* Merge: '<S44>/Merge1' incorporates:
         *  Constant: '<S55>/Zero_Torque'
         *  SignalConversion generated from: '<S55>/engine_traction_torque[Nm]'
         */
        rtb_Merge1 = TxCar_Hybrid_P.Zero_Torque_Value;

        /* End of Outputs for SubSystem: '<S44>/SerializePwr' */
        break;

      case 1:
        /* Outputs for IfAction SubSystem: '<S44>/ParallelPwr' incorporates:
         *  ActionPort: '<S54>/Action Port'
         */
        /* Sum: '<S54>/Add' incorporates:
         *  Outport generated from: '<Root>/OutBus_Outport_8'
         */
        rtb_Add_p = TxCar_Hybrid_Y.OutBus_HEV_Output.HEV_PwrDmnd_Output.Engine_TractionPwrDmnd +
                    TxCar_Hybrid_Y.OutBus_HEV_Output.HEV_PwrDmnd_Output.Engine_ChargingPwrDmnd;

        /* Switch: '<S56>/Switch' incorporates:
         *  Abs: '<S56>/Abs'
         *  Constant: '<S56>/Constant'
         *  Constant: '<S57>/Constant'
         *  Product: '<S56>/Product'
         *  RelationalOperator: '<S57>/Compare'
         *  Switch: '<S56>/sign'
         */
        if (std::abs(rtb_Add_p) < TxCar_Hybrid_P.DivProtect_MinVal) {
          /* Switch: '<S56>/sign' incorporates:
           *  Constant: '<S56>/Constant1'
           *  Constant: '<S56>/Constant2'
           *  Constant: '<S58>/Constant'
           *  RelationalOperator: '<S58>/Compare'
           */
          if (rtb_Add_p >= TxCar_Hybrid_P.Constant_Value) {
            rtb_Add_p = TxCar_Hybrid_P.Constant1_Value;
          } else {
            rtb_Add_p = TxCar_Hybrid_P.Constant2_Value;
          }

          rtb_Add_p *= TxCar_Hybrid_P.DivProtect_MinVal;
        }

        /* End of Switch: '<S56>/Switch' */

        /* Merge: '<S44>/Merge1' incorporates:
         *  Outport generated from: '<Root>/OutBus_Outport_8'
         *  Product: '<S54>/Divide'
         */
        rtb_Merge1 = TxCar_Hybrid_B.Memory_p *
                     TxCar_Hybrid_Y.OutBus_HEV_Output.HEV_PwrDmnd_Output.Engine_TractionPwrDmnd / rtb_Add_p;

        /* Merge: '<S44>/Merge' incorporates:
         *  Outport generated from: '<Root>/OutBus_Outport_8'
         *  Product: '<S54>/Divide1'
         */
        TxCar_Hybrid_B.Merge_c = TxCar_Hybrid_B.Memory_p *
                                 TxCar_Hybrid_Y.OutBus_HEV_Output.HEV_PwrDmnd_Output.Engine_ChargingPwrDmnd *
                                 TxCar_Hybrid_B.EngToGenerator_GearRatio / rtb_Add_p;

        /* End of Outputs for SubSystem: '<S44>/ParallelPwr' */
        break;
    }

    /* End of SwitchCase: '<S44>/Switch Case' */

    /* Gain: '<S41>/Gain3' incorporates:
     *  Memory: '<S41>/Memory1'
     */
    rtb_Add_p = TxCar_Hybrid_P.Gain3_Gain_p * TxCar_Hybrid_DW.Memory1_PreviousInput;

    /* Saturate: '<S41>/Saturation' */
    if (rtb_Add_p > TxCar_Hybrid_P.Saturation_UpperSat_o) {
      /* BusCreator generated from: '<S42>/S-Function' */
      TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_curr_A =
          TxCar_Hybrid_P.Saturation_UpperSat_o;
    } else if (rtb_Add_p < TxCar_Hybrid_P.Saturation_LowerSat_o) {
      /* BusCreator generated from: '<S42>/S-Function' */
      TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_curr_A =
          TxCar_Hybrid_P.Saturation_LowerSat_o;
    } else {
      /* BusCreator generated from: '<S42>/S-Function' */
      TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_curr_A = rtb_Add_p;
    }

    /* End of Saturate: '<S41>/Saturation' */

    /* BusCreator generated from: '<S42>/S-Function' incorporates:
     *  Constant: '<S41>/Constant'
     */
    TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_temp_K =
        TxCar_Hybrid_P.Constant_Value_n;

    /* S-Function (battery_sfunction): '<S42>/S-Function' */
    battery_sfunction_Outputs_wrapper_cgen(
        &TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p, &TxCar_Hybrid_B.SFunction_k,
        TxCar_Hybrid_P.SFunction_P1_m, 21, TxCar_Hybrid_P.SFunction_P2_g, 38);

    /* Switch: '<S59>/Switch' incorporates:
     *  Abs: '<S59>/Abs'
     *  Constant: '<S60>/Constant'
     *  RelationalOperator: '<S60>/Compare'
     */
    if (std::abs(TxCar_Hybrid_B.SFunction_k.batt_volt_V) < TxCar_Hybrid_P.DivProtect_MinVal_e) {
      /* Switch: '<S59>/sign' incorporates:
       *  Constant: '<S59>/Constant1'
       *  Constant: '<S59>/Constant2'
       *  Constant: '<S61>/Constant'
       *  RelationalOperator: '<S61>/Compare'
       */
      if (TxCar_Hybrid_B.SFunction_k.batt_volt_V >= TxCar_Hybrid_P.Constant_Value_p) {
        rtb_Add_p = TxCar_Hybrid_P.Constant1_Value_d;
      } else {
        rtb_Add_p = TxCar_Hybrid_P.Constant2_Value_o;
      }

      /* Switch: '<S59>/Switch' incorporates:
       *  Constant: '<S59>/Constant'
       *  Product: '<S59>/Product'
       *  Switch: '<S59>/sign'
       */
      TxCar_Hybrid_B.Switch = rtb_Add_p * TxCar_Hybrid_P.DivProtect_MinVal_e;
    } else {
      /* Switch: '<S59>/Switch' */
      TxCar_Hybrid_B.Switch = TxCar_Hybrid_B.SFunction_k.batt_volt_V;
    }

    /* End of Switch: '<S59>/Switch' */
  }

  /* BusCreator generated from: '<S45>/S-Function' incorporates:
   *  Gain: '<S50>/Gain2'
   *  Outport generated from: '<Root>/OutBus_Outport_8'
   */
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_e.trq_cmd_Nm =
      TxCar_Hybrid_Y.OutBus_HEV_Output.MotRequestTorque_Nm;
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_e.mot_spd_RPM =
      TxCar_Hybrid_P.Gain2_Gain_c * TxCar_Hybrid_B.Integrator1_j;
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_e.batt_volt_V =
      TxCar_Hybrid_B.SFunction_k.batt_volt_V;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* S-Function (front_motor_sfunction): '<S45>/S-Function' */
    front_motor_sfunction_Outputs_wrapper_cgen(
        &TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_e, &TxCar_Hybrid_B.SFunction_h,
        TxCar_Hybrid_P.SFunction_P1_h, 21, TxCar_Hybrid_P.SFunction_P2_b, 38);
  }

  /* BusCreator generated from: '<S47>/S-Function' incorporates:
   *  Gain: '<S52>/Gain4'
   *  Outport generated from: '<Root>/OutBus_Outport_8'
   *  Product: '<S52>/Product'
   */
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.trq_cmd_Nm =
      TxCar_Hybrid_Y.OutBus_HEV_Output.P4_MotRequestTorque_Nm;
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.mot_spd_RPM =
      TxCar_Hybrid_B.Integrator1_e * TxCar_Hybrid_B.EnableP4 * TxCar_Hybrid_P.Gain4_Gain_p;
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.batt_volt_V =
      TxCar_Hybrid_B.SFunction_k.batt_volt_V;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* S-Function (rear_motor_sfunction): '<S47>/S-Function' */
    rear_motor_sfunction_Outputs_wrapper_cgen(
        &TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m, &TxCar_Hybrid_B.SFunction_ep,
        TxCar_Hybrid_P.SFunction_P1_g, 21, TxCar_Hybrid_P.SFunction_P2_o, 38);

    /* Product: '<S41>/Product1' */
    TxCar_Hybrid_B.Product1 = TxCar_Hybrid_B.SFunction_ep.batt_curr_A * TxCar_Hybrid_B.EnableP4;
  }

  /* Product: '<S46>/Divide' incorporates:
   *  Gain: '<S44>/rads'
   *  Integrator: '<S62>/Integrator1'
   *  Product: '<S44>/Divide'
   *  Product: '<S46>/Product2'
   */
  rtb_Add_p = TxCar_Hybrid_P.rads_Gain * TxCar_Hybrid_X.Integrator1_CSTATE_ce /
              TxCar_Hybrid_B.EngToGenerator_GearRatio * TxCar_Hybrid_B.Merge_c / TxCar_Hybrid_B.Switch;

  /* Saturate: '<S46>/GeneratorPwrSaturation' */
  if (rtb_Add_p > TxCar_Hybrid_P.GeneratorPwrSaturation_UpperSat) {
    rtb_Add_p = TxCar_Hybrid_P.GeneratorPwrSaturation_UpperSat;
  } else if (rtb_Add_p < TxCar_Hybrid_P.GeneratorPwrSaturation_LowerSat) {
    rtb_Add_p = TxCar_Hybrid_P.GeneratorPwrSaturation_LowerSat;
  }

  /* Sum: '<S41>/Add' incorporates:
   *  Gain: '<S46>/Gain'
   *  Saturate: '<S46>/GeneratorPwrSaturation'
   */
  TxCar_Hybrid_B.Add =
      (TxCar_Hybrid_P.Gain_Gain_o * rtb_Add_p + TxCar_Hybrid_B.SFunction_h.batt_curr_A) + TxCar_Hybrid_B.Product1;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* Switch: '<S49>/Pwr_Dmnd' incorporates:
     *  Outport generated from: '<Root>/OutBus_Outport_8'
     *  Sum: '<S49>/ParallelPwr'
     *  Sum: '<S49>/SerializePwr'
     */
    if (TxCar_Hybrid_Y.OutBus_HEV_Output.hev_mode_output.Engine_ClutchSwitch > TxCar_Hybrid_P.Pwr_Dmnd_Threshold) {
      rtb_Add_p = TxCar_Hybrid_Y.OutBus_HEV_Output.HEV_PwrDmnd_Output.Engine_TractionPwrDmnd +
                  TxCar_Hybrid_Y.OutBus_HEV_Output.HEV_PwrDmnd_Output.Engine_ChargingPwrDmnd;
    } else {
      rtb_Add_p = TxCar_Hybrid_Y.OutBus_HEV_Output.HEV_PwrDmnd_Output.Engine_SerializePwrDmnd +
                  TxCar_Hybrid_Y.OutBus_HEV_Output.HEV_PwrDmnd_Output.Engine_ChargingPwrDmnd;
    }

    /* Product: '<S49>/Pwr_Cal' incorporates:
     *  Outport generated from: '<Root>/OutBus_Outport_8'
     *  Switch: '<S49>/Pwr_Dmnd'
     */
    TxCar_Hybrid_B.Pwr_Cal = TxCar_Hybrid_Y.OutBus_HEV_Output.hev_mode_output.Engine_Switch * rtb_Add_p;
  }

  /* Saturate: '<S49>/Saturation' */
  if (TxCar_Hybrid_X.Integrator1_CSTATE_ce > TxCar_Hybrid_P.Saturation_UpperSat_h) {
    rtb_Add_p = TxCar_Hybrid_P.Saturation_UpperSat_h;
  } else if (TxCar_Hybrid_X.Integrator1_CSTATE_ce < TxCar_Hybrid_P.Saturation_LowerSat_n) {
    rtb_Add_p = TxCar_Hybrid_P.Saturation_LowerSat_n;
  } else {
    rtb_Add_p = TxCar_Hybrid_X.Integrator1_CSTATE_ce;
  }

  /* Product: '<S49>/Normal_PwrDmnd' incorporates:
   *  Constant: '<S49>/Const_Pwr'
   *  Gain: '<S49>/rad|s'
   *  Product: '<S49>/EngAct_Pwr'
   *  Saturate: '<S49>/Saturation'
   *  Sum: '<S49>/Add'
   */
  TxCar_Hybrid_B.Normal_PwrDmnd =
      (TxCar_Hybrid_B.Pwr_Cal - TxCar_Hybrid_P.rads_Gain_k * rtb_Add_p * TxCar_Hybrid_B.Memory_p) /
      TxCar_Hybrid_P.Const_Pwr_Value;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* Logic: '<S49>/Reset_Throttle' incorporates:
     *  DataTypeConversion: '<S49>/Data Type Conversion'
     *  Outport generated from: '<Root>/OutBus_Outport_8'
     */
    rtb_Reset_Throttle = !(TxCar_Hybrid_Y.OutBus_HEV_Output.hev_mode_output.Engine_Switch != 0.0);

    /* DiscreteIntegrator: '<S95>/Integrator' */
    if (rtb_Reset_Throttle || (TxCar_Hybrid_DW.Integrator_PrevResetState != 0)) {
      TxCar_Hybrid_DW.Integrator_DSTATE = TxCar_Hybrid_P.Throttle_PID_InitialConditionForIntegrator;
    }

    /* DiscreteIntegrator: '<S90>/Filter' */
    if (rtb_Reset_Throttle || (TxCar_Hybrid_DW.Filter_PrevResetState != 0)) {
      TxCar_Hybrid_DW.Filter_DSTATE = TxCar_Hybrid_P.Throttle_PID_InitialConditionForFilter;
    }

    /* Gain: '<S98>/Filter Coefficient' incorporates:
     *  DiscreteIntegrator: '<S90>/Filter'
     *  Gain: '<S89>/Derivative Gain'
     *  Sum: '<S90>/SumD'
     */
    rtb_FilterCoefficient =
        (TxCar_Hybrid_P.Throttle_PID_D * TxCar_Hybrid_B.Normal_PwrDmnd - TxCar_Hybrid_DW.Filter_DSTATE) *
        TxCar_Hybrid_P.Throttle_PID_N;

    /* Sum: '<S104>/Sum' incorporates:
     *  DiscreteIntegrator: '<S95>/Integrator'
     *  Gain: '<S100>/Proportional Gain'
     */
    rtb_Add_p = (TxCar_Hybrid_P.Throttle_PID_P * TxCar_Hybrid_B.Normal_PwrDmnd + TxCar_Hybrid_DW.Integrator_DSTATE) +
                rtb_FilterCoefficient;

    /* Saturate: '<S102>/Saturation' */
    if (rtb_Add_p > TxCar_Hybrid_P.Throttle_PID_UpperSaturationLimit) {
      /* Saturate: '<S102>/Saturation' */
      TxCar_Hybrid_B.Saturation = TxCar_Hybrid_P.Throttle_PID_UpperSaturationLimit;
    } else if (rtb_Add_p < TxCar_Hybrid_P.Throttle_PID_LowerSaturationLimit) {
      /* Saturate: '<S102>/Saturation' */
      TxCar_Hybrid_B.Saturation = TxCar_Hybrid_P.Throttle_PID_LowerSaturationLimit;
    } else {
      /* Saturate: '<S102>/Saturation' */
      TxCar_Hybrid_B.Saturation = rtb_Add_p;
    }

    /* End of Saturate: '<S102>/Saturation' */
  }

  /* BusCreator generated from: '<S43>/S-Function' incorporates:
   *  Integrator: '<S62>/Integrator1'
   */
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_er.throttle_01 = TxCar_Hybrid_B.Saturation;
  TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_er.eng_spd_rpm =
      TxCar_Hybrid_X.Integrator1_CSTATE_ce;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* S-Function (engine_sfunction): '<S43>/S-Function' */
    engine_sfunction_Outputs_wrapper_cgen(
        &TxCar_Hybrid_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_er, &TxCar_Hybrid_B.SFunction_i,
        TxCar_Hybrid_P.SFunction_P1_c, 21, TxCar_Hybrid_P.SFunction_P2_d, 38);

    /* Product: '<S41>/Product' */
    rtb_Product_a = TxCar_Hybrid_B.SFunction_ep.mot_trq_Nm * TxCar_Hybrid_B.EnableP4;

    /* SignalConversion generated from: '<S41>/Bus Selector2' */
    rtb_batt_soc_null = TxCar_Hybrid_B.SFunction_k.batt_soc_null;

    /* SignalConversion generated from: '<S41>/Bus Selector' */
    rtb_eng_trq_Nm = TxCar_Hybrid_B.SFunction_i.eng_trq_Nm;
  }

  /* Switch: '<S48>/Switch' incorporates:
   *  Gain: '<S48>/RPM'
   *  Outport generated from: '<Root>/OutBus_Outport_8'
   */
  if (TxCar_Hybrid_Y.OutBus_HEV_Output.hev_mode_output.Engine_ClutchSwitch > TxCar_Hybrid_P.Switch_Threshold) {
    rtb_Add_p = TxCar_Hybrid_P.RPM_Gain * TxCar_Hybrid_B.Integrator1;
  } else {
    rtb_Add_p = TxCar_Hybrid_Y.OutBus_HEV_Output.HEV_PwrDmnd_Output.Engine_Request_Spd_RPM;
  }

  /* Product: '<S62>/Divide' incorporates:
   *  Constant: '<S62>/Constant'
   *  Integrator: '<S62>/Integrator1'
   *  Sum: '<S62>/Sum'
   *  Switch: '<S48>/Switch'
   */
  TxCar_Hybrid_B.Divide_f = (rtb_Add_p - TxCar_Hybrid_X.Integrator1_CSTATE_ce) * TxCar_Hybrid_P.LPF_wc;

  /* BusCreator generated from: '<S121>/Steer_Mapped' incorporates:
   *  Inport generated from: '<Root>/DriverIn'
   */
  TxCar_Hybrid_B.BusConversion_InsertedFor_Steer_Mapped_at_inport_0_BusCreator1 = TxCar_Hybrid_U.DriverIn;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* Gain: '<S92>/Integral Gain' */
    rtb_IntegralGain = TxCar_Hybrid_P.Throttle_PID_I * TxCar_Hybrid_B.Normal_PwrDmnd;

    /* S-Function (dynamic_steer_sfunc): '<S121>/Steer_Mapped' incorporates:
     *  Outport generated from: '<Root>/Body'
     *  Outport generated from: '<Root>/Bus Element Out2'
     *  Outport generated from: '<Root>/Susp'
     *  Outport generated from: '<Root>/WheelsOut'
     */
    dynamic_steer_sfunc_Outputs_wrapper_cgen(
        &TxCar_Hybrid_B.BusConversion_InsertedFor_Steer_Mapped_at_inport_0_BusCreator1, &TxCar_Hybrid_Y.Body,
        &TxCar_Hybrid_Y.Susp, &TxCar_Hybrid_Y.WheelsOut, &TxCar_Hybrid_Y.SteerBus_Out, TxCar_Hybrid_P.Steer_Mapped_P1,
        21, TxCar_Hybrid_P.Steer_Mapped_P2, 38);

    /* DeadZone: '<S112>/Dead Zone' incorporates:
     *  BusCreator: '<S115>/Bus Creator1'
     *  Outport generated from: '<Root>/Body'
     */
    if (TxCar_Hybrid_Y.Body.BdyFrm.Cg.Vel.xdot > TxCar_Hybrid_P.DeadZone_End) {
      rtb_Product_k = TxCar_Hybrid_Y.Body.BdyFrm.Cg.Vel.xdot - TxCar_Hybrid_P.DeadZone_End;
    } else if (TxCar_Hybrid_Y.Body.BdyFrm.Cg.Vel.xdot >= TxCar_Hybrid_P.DeadZone_Start) {
      rtb_Product_k = 0.0;
    } else {
      rtb_Product_k = TxCar_Hybrid_Y.Body.BdyFrm.Cg.Vel.xdot - TxCar_Hybrid_P.DeadZone_Start;
    }

    /* Signum: '<S112>/Sign' incorporates:
     *  DeadZone: '<S112>/Dead Zone'
     */
    if (std::isnan(rtb_Product_k)) {
      rtb_Add_p = (rtNaN);
    } else if (rtb_Product_k < 0.0) {
      rtb_Add_p = -1.0;
    } else {
      rtb_Add_p = (rtb_Product_k > 0.0);
    }

    /* Product: '<S112>/Product' incorporates:
     *  Constant: '<S112>/AxleTrqOffset'
     *  Signum: '<S112>/Sign'
     */
    rtb_Product_k = rtb_Add_p * TxCar_Hybrid_P.AxleTrqOffset_Value;

    /* Saturate: '<S9>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[0] > TxCar_Hybrid_P.Saturation_UpperSat_k) {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[0] = TxCar_Hybrid_P.Saturation_UpperSat_k;
    } else if (TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[0] < TxCar_Hybrid_P.Saturation_LowerSat_l) {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[0] = TxCar_Hybrid_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[0] = TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[0];
    }

    /* Sum: '<S112>/Add' */
    rtb_AxlTrq[0] = rtb_Product_k + rtb_Saturation_idx_0;

    /* Gain: '<S114>/Gain' */
    rtb_brake_pressure_Pa_idx_0 = TxCar_Hybrid_P.Gain_Gain_a * TxCar_Hybrid_B.SFunction_o1.brake_pressure_Pa[0];

    /* Gain: '<S114>/Gain1' */
    rtb_ABSFlag_idx_0 = TxCar_Hybrid_P.Gain1_Gain_o * TxCar_Hybrid_B.SFunction_o1.abs_flag[0];

    /* Saturate: '<S9>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[1] > TxCar_Hybrid_P.Saturation_UpperSat_k) {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[1] = TxCar_Hybrid_P.Saturation_UpperSat_k;
    } else if (TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[1] < TxCar_Hybrid_P.Saturation_LowerSat_l) {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[1] = TxCar_Hybrid_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[1] = TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[1];
    }

    /* Sum: '<S112>/Add' */
    rtb_AxlTrq[1] = rtb_Product_k + rtb_Saturation_idx_1;

    /* Gain: '<S114>/Gain' */
    rtb_brake_pressure_Pa_idx_1 = TxCar_Hybrid_P.Gain_Gain_a * TxCar_Hybrid_B.SFunction_o1.brake_pressure_Pa[1];

    /* Gain: '<S114>/Gain1' */
    rtb_ABSFlag_idx_1 = TxCar_Hybrid_P.Gain1_Gain_o * TxCar_Hybrid_B.SFunction_o1.abs_flag[1];

    /* Saturate: '<S9>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[2] > TxCar_Hybrid_P.Saturation_UpperSat_k) {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[2] = TxCar_Hybrid_P.Saturation_UpperSat_k;
    } else if (TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[2] < TxCar_Hybrid_P.Saturation_LowerSat_l) {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[2] = TxCar_Hybrid_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[2] = TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[2];
    }

    /* Sum: '<S112>/Add' */
    rtb_AxlTrq[2] = rtb_Product_k + rtb_Saturation_idx_2;

    /* Gain: '<S114>/Gain' */
    rtb_brake_pressure_Pa_idx_2 = TxCar_Hybrid_P.Gain_Gain_a * TxCar_Hybrid_B.SFunction_o1.brake_pressure_Pa[2];

    /* Gain: '<S114>/Gain1' */
    rtb_ABSFlag_idx_2 = TxCar_Hybrid_P.Gain1_Gain_o * TxCar_Hybrid_B.SFunction_o1.abs_flag[2];

    /* Saturate: '<S9>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[3] > TxCar_Hybrid_P.Saturation_UpperSat_k) {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[3] = TxCar_Hybrid_P.Saturation_UpperSat_k;
    } else if (TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[3] < TxCar_Hybrid_P.Saturation_LowerSat_l) {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[3] = TxCar_Hybrid_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S9>/Saturation' */
      rtb_wheel_angle[3] = TxCar_Hybrid_Y.SteerBus_Out.wheel_angle[3];
    }

    /* Sum: '<S112>/Add' */
    rtb_AxlTrq[3] = rtb_Product_k + rtb_Saturation_idx_3;

    /* Gain: '<S114>/Gain' */
    rtb_brake_pressure_Pa_idx_3 = TxCar_Hybrid_P.Gain_Gain_a * TxCar_Hybrid_B.SFunction_o1.brake_pressure_Pa[3];

    /* Gain: '<S114>/Gain1' */
    rtb_ABSFlag_idx_3 = TxCar_Hybrid_P.Gain1_Gain_o * TxCar_Hybrid_B.SFunction_o1.abs_flag[3];

    /* Gain: '<S117>/Gain' */
    rtb_Product_k = TxCar_Hybrid_P.Gain_Gain_m * TxCar_Hybrid_B.SFunction_o1.pt_trq_cmd_Nm;

    /* Gain: '<S117>/Gain1' */
    rtb_throttle_01 = TxCar_Hybrid_P.Gain1_Gain_f * TxCar_Hybrid_B.SFunction_o1.throttle_01;

    /* S-Function (cartype_sfunction): '<S1>/S-Function' incorporates:
     *  Constant: '<S1>/Zero'
     */
    cartype_sfunction_Outputs_wrapper_cgen(&TxCar_Hybrid_P.Zero_Value, &TxCar_Hybrid_B.SFunction_e,
                                           TxCar_Hybrid_P.SFunction_P1_lo, 21, TxCar_Hybrid_P.SFunction_P2_ok, 38);

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_Hybrid_Y.DriveLineBus_Out_AxlTrq[0] = rtb_Saturation_idx_0;
    TxCar_Hybrid_Y.DriveLineBus_Out_AxlTrq[1] = rtb_Saturation_idx_1;
    TxCar_Hybrid_Y.DriveLineBus_Out_AxlTrq[2] = rtb_Saturation_idx_2;
    TxCar_Hybrid_Y.DriveLineBus_Out_AxlTrq[3] = rtb_Saturation_idx_3;
  }

  /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
  TxCar_Hybrid_Y.DriveLineBus_Out_EngSpd = TxCar_Hybrid_B.Integrator1;

  /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
   *  Integrator: '<S26>/Integrator1'
   */
  TxCar_Hybrid_Y.DriveLineBus_Out_PropShftSpd = TxCar_Hybrid_X.Integrator1_CSTATE_e;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_Hybrid_Y.DriveLineBus_Out_Gear = TxCar_Hybrid_B.BusCreator2.gear_engaged;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_Hybrid_Y.PowerBus_Out_eng_trqNm = rtb_eng_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_Hybrid_Y.PowerBus_Out_mot_front_trqNm = rtb_mot_front_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_Hybrid_Y.PowerBus_Out_mot_rear_trqNm = rtb_mot_rear_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_Hybrid_Y.PowerBus_Out_battery_info_batt_volt_V = rtb_batt_volt_V;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_Hybrid_Y.PowerBus_Out_current = rtb_current;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_ECU_TrqCmd = rtb_Product_k;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_ECU_throttle_01 = rtb_throttle_01;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' incorporates:
     *  Constant: '<S114>/Constant'
     */
    TxCar_Hybrid_Y.SoftEcu_Out_Brake_TCSFlag = TxCar_Hybrid_P.Constant_Value_p5;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Brake_ABSFlag[0] = rtb_ABSFlag_idx_0;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Brake_brake_pressure_Pa[0] = rtb_brake_pressure_Pa_idx_0;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Diff_DiffPrsCmd[0] = rtb_DiffPrsCmd_idx_0;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Brake_ABSFlag[1] = rtb_ABSFlag_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Brake_brake_pressure_Pa[1] = rtb_brake_pressure_Pa_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Diff_DiffPrsCmd[1] = rtb_DiffPrsCmd_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Brake_ABSFlag[2] = rtb_ABSFlag_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Brake_brake_pressure_Pa[2] = rtb_brake_pressure_Pa_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Diff_DiffPrsCmd[2] = rtb_DiffPrsCmd_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Brake_ABSFlag[3] = rtb_ABSFlag_idx_3;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Brake_brake_pressure_Pa[3] = rtb_brake_pressure_Pa_idx_3;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Hybrid_Y.SoftEcu_Out_Diff_DiffPrsCmd[3] = rtb_DiffPrsCmd_idx_3;
  }

  /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
  TxCar_Hybrid_Y.DriveLineBus_Out_FrontMotSpd = TxCar_Hybrid_B.Integrator1_j;

  /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
  TxCar_Hybrid_Y.DriveLineBus_Out_RearMotSpd = TxCar_Hybrid_B.Integrator1_e;
  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
      /* Update for Memory generated from: '<S9>/Memory' */
      TxCar_Hybrid_DW.Memory_1_PreviousInput[0] = rtb_wheel_angle[0];

      /* Update for Memory generated from: '<S9>/Memory' */
      TxCar_Hybrid_DW.Memory_2_PreviousInput[0] = rtb_AxlTrq[0];

      /* Update for Memory generated from: '<S9>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_Hybrid_DW.Memory_3_PreviousInput[0] = TxCar_Hybrid_Y.DriveLineBus_Out_BrkPrs[0];

      /* Update for Memory generated from: '<S9>/Memory' */
      TxCar_Hybrid_DW.Memory_1_PreviousInput[1] = rtb_wheel_angle[1];

      /* Update for Memory generated from: '<S9>/Memory' */
      TxCar_Hybrid_DW.Memory_2_PreviousInput[1] = rtb_AxlTrq[1];

      /* Update for Memory generated from: '<S9>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_Hybrid_DW.Memory_3_PreviousInput[1] = TxCar_Hybrid_Y.DriveLineBus_Out_BrkPrs[1];

      /* Update for Memory generated from: '<S9>/Memory' */
      TxCar_Hybrid_DW.Memory_1_PreviousInput[2] = rtb_wheel_angle[2];

      /* Update for Memory generated from: '<S9>/Memory' */
      TxCar_Hybrid_DW.Memory_2_PreviousInput[2] = rtb_AxlTrq[2];

      /* Update for Memory generated from: '<S9>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_Hybrid_DW.Memory_3_PreviousInput[2] = TxCar_Hybrid_Y.DriveLineBus_Out_BrkPrs[2];

      /* Update for Memory generated from: '<S9>/Memory' */
      TxCar_Hybrid_DW.Memory_1_PreviousInput[3] = rtb_wheel_angle[3];

      /* Update for Memory generated from: '<S9>/Memory' */
      TxCar_Hybrid_DW.Memory_2_PreviousInput[3] = rtb_AxlTrq[3];

      /* Update for Memory generated from: '<S9>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_Hybrid_DW.Memory_3_PreviousInput[3] = TxCar_Hybrid_Y.DriveLineBus_Out_BrkPrs[3];

      /* Update for Memory generated from: '<S40>/Memory' */
      TxCar_Hybrid_DW.Memory_5_PreviousInput = rtb_batt_soc_null;

      /* Update for Memory generated from: '<S40>/Memory' */
      TxCar_Hybrid_DW.Memory_1_PreviousInput_i = rtb_Merge1;

      /* Update for Memory generated from: '<S40>/Memory' */
      TxCar_Hybrid_DW.Memory_2_PreviousInput_p = TxCar_Hybrid_B.SFunction_h.mot_trq_Nm;

      /* Update for Memory generated from: '<S40>/Memory' */
      TxCar_Hybrid_DW.Memory_3_PreviousInput_k = rtb_Product_a;

      /* Update for Memory: '<S27>/Memory' */
      TxCar_Hybrid_DW.Memory_PreviousInput = TxCar_Hybrid_B.SFunction_o3;

      /* Update for Memory generated from: '<S40>/Memory' */
      TxCar_Hybrid_DW.Memory_4_PreviousInput = TxCar_Hybrid_B.SFunction_k.batt_volt_V;

      /* Update for Memory generated from: '<S40>/Memory' */
      TxCar_Hybrid_DW.Memory_6_PreviousInput = TxCar_Hybrid_B.Add;

      /* Update for Memory: '<S44>/Memory' */
      TxCar_Hybrid_DW.Memory_PreviousInput_p = rtb_eng_trq_Nm;

      /* Update for Memory: '<S41>/Memory1' */
      TxCar_Hybrid_DW.Memory1_PreviousInput = TxCar_Hybrid_B.Add;

      /* Update for DiscreteIntegrator: '<S95>/Integrator' */
      TxCar_Hybrid_DW.Integrator_DSTATE += TxCar_Hybrid_P.Integrator_gainval * rtb_IntegralGain;
      TxCar_Hybrid_DW.Integrator_PrevResetState = static_cast<int8_T>(rtb_Reset_Throttle);

      /* Update for DiscreteIntegrator: '<S90>/Filter' incorporates:
       *  DiscreteIntegrator: '<S95>/Integrator'
       */
      TxCar_Hybrid_DW.Filter_DSTATE += TxCar_Hybrid_P.Filter_gainval * rtb_FilterCoefficient;
      TxCar_Hybrid_DW.Filter_PrevResetState = static_cast<int8_T>(rtb_Reset_Throttle);
    }
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&TxCar_Hybrid_M))) {
    rt_ertODEUpdateContinuousStates(&(&TxCar_Hybrid_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&TxCar_Hybrid_M)->Timing.clockTick0)) {
      ++(&TxCar_Hybrid_M)->Timing.clockTickH0;
    }

    (&TxCar_Hybrid_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&TxCar_Hybrid_M)->solverInfo);

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
      (&TxCar_Hybrid_M)->Timing.clockTick1++;
      if (!(&TxCar_Hybrid_M)->Timing.clockTick1) {
        (&TxCar_Hybrid_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void TxCar_Hybrid::TxCar_Hybrid_derivatives() {
  XDot_TxCar_Hybrid_T *_rtXdot;
  _rtXdot = ((XDot_TxCar_Hybrid_T *)(&TxCar_Hybrid_M)->derivs);

  /* Derivatives for Integrator: '<S18>/Integrator1' */
  _rtXdot->Integrator1_CSTATE = TxCar_Hybrid_B.Divide;

  /* Derivatives for Integrator: '<S20>/Integrator1' */
  _rtXdot->Integrator1_CSTATE_c = TxCar_Hybrid_B.Divide_h;

  /* Derivatives for Integrator: '<S25>/Integrator1' */
  _rtXdot->Integrator1_CSTATE_a = TxCar_Hybrid_B.Divide_p;

  /* Derivatives for Integrator: '<S26>/Integrator1' */
  _rtXdot->Integrator1_CSTATE_e = TxCar_Hybrid_B.Divide_c;

  /* Derivatives for Integrator: '<S62>/Integrator1' */
  _rtXdot->Integrator1_CSTATE_ce = TxCar_Hybrid_B.Divide_f;
}

/* Model initialize function */
void TxCar_Hybrid::initialize() {
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&TxCar_Hybrid_M)->solverInfo, &(&TxCar_Hybrid_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&TxCar_Hybrid_M)->solverInfo, &rtmGetTPtr((&TxCar_Hybrid_M)));
    rtsiSetStepSizePtr(&(&TxCar_Hybrid_M)->solverInfo, &(&TxCar_Hybrid_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&TxCar_Hybrid_M)->solverInfo, &(&TxCar_Hybrid_M)->derivs);
    rtsiSetContStatesPtr(&(&TxCar_Hybrid_M)->solverInfo, (real_T **)&(&TxCar_Hybrid_M)->contStates);
    rtsiSetNumContStatesPtr(&(&TxCar_Hybrid_M)->solverInfo, &(&TxCar_Hybrid_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&TxCar_Hybrid_M)->solverInfo, &(&TxCar_Hybrid_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&TxCar_Hybrid_M)->solverInfo, &(&TxCar_Hybrid_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&TxCar_Hybrid_M)->solverInfo, &(&TxCar_Hybrid_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&TxCar_Hybrid_M)->solverInfo, (&rtmGetErrorStatus((&TxCar_Hybrid_M))));
    rtsiSetRTModelPtr(&(&TxCar_Hybrid_M)->solverInfo, (&TxCar_Hybrid_M));
  }

  rtsiSetSimTimeStep(&(&TxCar_Hybrid_M)->solverInfo, MAJOR_TIME_STEP);
  (&TxCar_Hybrid_M)->intgData.y = (&TxCar_Hybrid_M)->odeY;
  (&TxCar_Hybrid_M)->intgData.f[0] = (&TxCar_Hybrid_M)->odeF[0];
  (&TxCar_Hybrid_M)->intgData.f[1] = (&TxCar_Hybrid_M)->odeF[1];
  (&TxCar_Hybrid_M)->intgData.f[2] = (&TxCar_Hybrid_M)->odeF[2];
  (&TxCar_Hybrid_M)->intgData.f[3] = (&TxCar_Hybrid_M)->odeF[3];
  (&TxCar_Hybrid_M)->contStates = ((X_TxCar_Hybrid_T *)&TxCar_Hybrid_X);
  rtsiSetSolverData(&(&TxCar_Hybrid_M)->solverInfo, static_cast<void *>(&(&TxCar_Hybrid_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&TxCar_Hybrid_M)->solverInfo, false);
  rtsiSetSolverName(&(&TxCar_Hybrid_M)->solverInfo, "ode4");
  rtmSetTPtr((&TxCar_Hybrid_M), &(&TxCar_Hybrid_M)->Timing.tArray[0]);
  (&TxCar_Hybrid_M)->Timing.stepSize0 = 0.001;

  /* Start for S-Function (chassis_sfunc): '<S2>/Chassis' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S2>/Chassis */
  chassis_sfunc_Start_wrapper_cgen(TxCar_Hybrid_P.Chassis_P1, 21, TxCar_Hybrid_P.Chassis_P2, 38);

  /* Start for S-Function (softhcu_sfunction): '<S120>/S-Function' incorporates:
   *  Outport generated from: '<Root>/OutBus_Outport_8'
   */

  /* S-Function Block: <S120>/S-Function */
  softhcu_sfunction_Start_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1, 21, TxCar_Hybrid_P.SFunction_P2, 38);

  /* Start for S-Function (hybridparser_sfunction): '<S6>/S-Function' incorporates:
   *  Constant: '<S6>/NoUse'
   */

  /* S-Function Block: <S6>/S-Function */
  hybridparser_sfunction_Start_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_b, 21, TxCar_Hybrid_P.SFunction_P2_m, 38);

  /* Start for S-Function (driveline_ev_rear_sfunction): '<S16>/S-Function' */

  /* S-Function Block: <S16>/S-Function */
  driveline_ev_rear_sfunction_Start_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_l, 21, TxCar_Hybrid_P.SFunction_P2_m5, 38);

  /* Start for S-Function (driveline_ice_sfunction): '<S17>/S-Function' */

  /* S-Function Block: <S17>/S-Function */
  driveline_ice_sfunction_Start_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_lm, 21, TxCar_Hybrid_P.SFunction_P2_n, 38);

  /* Start for S-Function (transmission_sfunction): '<S27>/S-Function' */

  /* S-Function Block: <S27>/S-Function */
  transmission_sfunction_Start_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_j, 21, TxCar_Hybrid_P.SFunction_P2_e, 38);

  /* Start for S-Function (brakehydraulic_sfunc): '<S15>/S-Function' incorporates:
   *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
   *  Outport generated from: '<Root>/SoftEcu_Outport_4'
   */

  /* S-Function Block: <S15>/S-Function */
  brakehydraulic_sfunc_Start_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_f, 21, TxCar_Hybrid_P.SFunction_P2_i, 38);

  /* Start for SwitchCase: '<S44>/Switch Case' */
  TxCar_Hybrid_DW.SwitchCase_ActiveSubsystem = -1;

  /* Start for S-Function (battery_sfunction): '<S42>/S-Function' */

  /* S-Function Block: <S42>/S-Function */
  battery_sfunction_Start_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_m, 21, TxCar_Hybrid_P.SFunction_P2_g, 38);

  /* Start for S-Function (front_motor_sfunction): '<S45>/S-Function' */

  /* S-Function Block: <S45>/S-Function */
  front_motor_sfunction_Start_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_h, 21, TxCar_Hybrid_P.SFunction_P2_b, 38);

  /* Start for S-Function (rear_motor_sfunction): '<S47>/S-Function' */

  /* S-Function Block: <S47>/S-Function */
  rear_motor_sfunction_Start_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_g, 21, TxCar_Hybrid_P.SFunction_P2_o, 38);

  /* Start for S-Function (engine_sfunction): '<S43>/S-Function' */

  /* S-Function Block: <S43>/S-Function */
  engine_sfunction_Start_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_c, 21, TxCar_Hybrid_P.SFunction_P2_d, 38);

  /* Start for S-Function (dynamic_steer_sfunc): '<S121>/Steer_Mapped' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Bus Element Out2'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S121>/Steer_Mapped */
  dynamic_steer_sfunc_Start_wrapper_cgen(TxCar_Hybrid_P.Steer_Mapped_P1, 21, TxCar_Hybrid_P.Steer_Mapped_P2, 38);

  /* Start for S-Function (cartype_sfunction): '<S1>/S-Function' incorporates:
   *  Constant: '<S1>/Zero'
   */

  /* S-Function Block: <S1>/S-Function */
  cartype_sfunction_Start_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_lo, 21, TxCar_Hybrid_P.SFunction_P2_ok, 38);

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_1_PreviousInput[0] = TxCar_Hybrid_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_2_PreviousInput[0] = TxCar_Hybrid_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_3_PreviousInput[0] = TxCar_Hybrid_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_1_PreviousInput[1] = TxCar_Hybrid_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_2_PreviousInput[1] = TxCar_Hybrid_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_3_PreviousInput[1] = TxCar_Hybrid_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_1_PreviousInput[2] = TxCar_Hybrid_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_2_PreviousInput[2] = TxCar_Hybrid_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_3_PreviousInput[2] = TxCar_Hybrid_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_1_PreviousInput[3] = TxCar_Hybrid_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_2_PreviousInput[3] = TxCar_Hybrid_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S9>/Memory' */
  TxCar_Hybrid_DW.Memory_3_PreviousInput[3] = TxCar_Hybrid_P.Memory_3_InitialCondition;

  /* InitializeConditions for Integrator: '<S18>/Integrator1' */
  TxCar_Hybrid_X.Integrator1_CSTATE = TxCar_Hybrid_P.Integrator1_IC;

  /* InitializeConditions for Memory generated from: '<S40>/Memory' */
  TxCar_Hybrid_DW.Memory_5_PreviousInput = TxCar_Hybrid_P.Memory_5_InitialCondition;

  /* InitializeConditions for Integrator: '<S20>/Integrator1' */
  TxCar_Hybrid_X.Integrator1_CSTATE_c = TxCar_Hybrid_P.Integrator1_IC_j;

  /* InitializeConditions for Integrator: '<S25>/Integrator1' */
  TxCar_Hybrid_X.Integrator1_CSTATE_a = TxCar_Hybrid_P.Integrator1_IC_o;

  /* InitializeConditions for Memory generated from: '<S40>/Memory' */
  TxCar_Hybrid_DW.Memory_1_PreviousInput_i = TxCar_Hybrid_P.Memory_1_InitialCondition_l;

  /* InitializeConditions for Memory generated from: '<S40>/Memory' */
  TxCar_Hybrid_DW.Memory_2_PreviousInput_p = TxCar_Hybrid_P.Memory_2_InitialCondition_l;

  /* InitializeConditions for Memory generated from: '<S40>/Memory' */
  TxCar_Hybrid_DW.Memory_3_PreviousInput_k = TxCar_Hybrid_P.Memory_3_InitialCondition_l;

  /* InitializeConditions for Memory: '<S27>/Memory' */
  TxCar_Hybrid_DW.Memory_PreviousInput = TxCar_Hybrid_P.Memory_InitialCondition;

  /* InitializeConditions for Integrator: '<S26>/Integrator1' */
  TxCar_Hybrid_X.Integrator1_CSTATE_e = TxCar_Hybrid_P.Integrator1_IC_f;

  /* InitializeConditions for Memory generated from: '<S40>/Memory' */
  TxCar_Hybrid_DW.Memory_4_PreviousInput = TxCar_Hybrid_P.Memory_4_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S40>/Memory' */
  TxCar_Hybrid_DW.Memory_6_PreviousInput = TxCar_Hybrid_P.Memory_6_InitialCondition;

  /* InitializeConditions for Memory: '<S44>/Memory' */
  TxCar_Hybrid_DW.Memory_PreviousInput_p = TxCar_Hybrid_P.Memory_InitialCondition_f;

  /* InitializeConditions for Integrator: '<S62>/Integrator1' */
  TxCar_Hybrid_X.Integrator1_CSTATE_ce = TxCar_Hybrid_P.Integrator1_IC_d;

  /* InitializeConditions for Memory: '<S41>/Memory1' */
  TxCar_Hybrid_DW.Memory1_PreviousInput = TxCar_Hybrid_P.Memory1_InitialCondition;

  /* InitializeConditions for DiscreteIntegrator: '<S95>/Integrator' */
  TxCar_Hybrid_DW.Integrator_DSTATE = TxCar_Hybrid_P.Throttle_PID_InitialConditionForIntegrator;

  /* InitializeConditions for DiscreteIntegrator: '<S90>/Filter' */
  TxCar_Hybrid_DW.Filter_DSTATE = TxCar_Hybrid_P.Throttle_PID_InitialConditionForFilter;

  /* SystemInitialize for Merge: '<S19>/Merge' */
  TxCar_Hybrid_B.Merge = TxCar_Hybrid_P.Merge_InitialOutput;
}

/* Model terminate function */
void TxCar_Hybrid::terminate() {
  /* Terminate for S-Function (chassis_sfunc): '<S2>/Chassis' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S2>/Chassis */
  chassis_sfunc_Terminate_wrapper_cgen(TxCar_Hybrid_P.Chassis_P1, 21, TxCar_Hybrid_P.Chassis_P2, 38);

  /* Terminate for S-Function (softhcu_sfunction): '<S120>/S-Function' incorporates:
   *  Outport generated from: '<Root>/OutBus_Outport_8'
   */

  /* S-Function Block: <S120>/S-Function */
  softhcu_sfunction_Terminate_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1, 21, TxCar_Hybrid_P.SFunction_P2, 38);

  /* Terminate for S-Function (hybridparser_sfunction): '<S6>/S-Function' incorporates:
   *  Constant: '<S6>/NoUse'
   */

  /* S-Function Block: <S6>/S-Function */
  hybridparser_sfunction_Terminate_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_b, 21, TxCar_Hybrid_P.SFunction_P2_m, 38);

  /* Terminate for S-Function (driveline_ev_rear_sfunction): '<S16>/S-Function' */

  /* S-Function Block: <S16>/S-Function */
  driveline_ev_rear_sfunction_Terminate_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_l, 21, TxCar_Hybrid_P.SFunction_P2_m5,
                                                     38);

  /* Terminate for S-Function (driveline_ice_sfunction): '<S17>/S-Function' */

  /* S-Function Block: <S17>/S-Function */
  driveline_ice_sfunction_Terminate_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_lm, 21, TxCar_Hybrid_P.SFunction_P2_n, 38);

  /* Terminate for S-Function (transmission_sfunction): '<S27>/S-Function' */

  /* S-Function Block: <S27>/S-Function */
  transmission_sfunction_Terminate_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_j, 21, TxCar_Hybrid_P.SFunction_P2_e, 38);

  /* Terminate for S-Function (brakehydraulic_sfunc): '<S15>/S-Function' incorporates:
   *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
   *  Outport generated from: '<Root>/SoftEcu_Outport_4'
   */

  /* S-Function Block: <S15>/S-Function */
  brakehydraulic_sfunc_Terminate_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_f, 21, TxCar_Hybrid_P.SFunction_P2_i, 38);

  /* Terminate for S-Function (battery_sfunction): '<S42>/S-Function' */

  /* S-Function Block: <S42>/S-Function */
  battery_sfunction_Terminate_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_m, 21, TxCar_Hybrid_P.SFunction_P2_g, 38);

  /* Terminate for S-Function (front_motor_sfunction): '<S45>/S-Function' */

  /* S-Function Block: <S45>/S-Function */
  front_motor_sfunction_Terminate_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_h, 21, TxCar_Hybrid_P.SFunction_P2_b, 38);

  /* Terminate for S-Function (rear_motor_sfunction): '<S47>/S-Function' */

  /* S-Function Block: <S47>/S-Function */
  rear_motor_sfunction_Terminate_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_g, 21, TxCar_Hybrid_P.SFunction_P2_o, 38);

  /* Terminate for S-Function (engine_sfunction): '<S43>/S-Function' */

  /* S-Function Block: <S43>/S-Function */
  engine_sfunction_Terminate_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_c, 21, TxCar_Hybrid_P.SFunction_P2_d, 38);

  /* Terminate for S-Function (dynamic_steer_sfunc): '<S121>/Steer_Mapped' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Bus Element Out2'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S121>/Steer_Mapped */
  dynamic_steer_sfunc_Terminate_wrapper_cgen(TxCar_Hybrid_P.Steer_Mapped_P1, 21, TxCar_Hybrid_P.Steer_Mapped_P2, 38);

  /* Terminate for S-Function (cartype_sfunction): '<S1>/S-Function' incorporates:
   *  Constant: '<S1>/Zero'
   */

  /* S-Function Block: <S1>/S-Function */
  cartype_sfunction_Terminate_wrapper_cgen(TxCar_Hybrid_P.SFunction_P1_lo, 21, TxCar_Hybrid_P.SFunction_P2_ok, 38);
}

/* Constructor */
TxCar_Hybrid::TxCar_Hybrid()
    : TxCar_Hybrid_U(), TxCar_Hybrid_Y(), TxCar_Hybrid_B(), TxCar_Hybrid_DW(), TxCar_Hybrid_X(), TxCar_Hybrid_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
TxCar_Hybrid::~TxCar_Hybrid() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_TxCar_Hybrid_T *TxCar_Hybrid::getRTM() { return (&TxCar_Hybrid_M); }
