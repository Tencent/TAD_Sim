/*
 * TxCar.cpp
 *
 * Code generation for model "TxCar".
 *
 * Model version              : 2.1095
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Thu May 11 21:28:18 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TxCar.h"
#include "TxCar_private.h"
#include "TxCar_types.h"
#include "rtwtypes.h"

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void TxCar::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  TxCar_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  TxCar_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  TxCar_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  TxCar_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Model step function */
void TxCar::step() {
  /* local block i/o variables */
  Steer_Output rtb_Memory;
  real_T rtb_Saturation[4];
  real_T rtb_Add;
  real_T rtb_mot_trq_Nm;
  real_T rtb_batt_soc_null;
  real_T rtb_TrqCmd;
  real_T rtb_ABSFlag_idx_1;
  real_T rtb_ABSFlag_idx_2;
  real_T rtb_ABSFlag_idx_3;
  real_T rtb_DiffPrsCmd_idx_0;
  real_T rtb_DiffPrsCmd_idx_1;
  real_T rtb_DiffPrsCmd_idx_2;
  real_T rtb_DiffPrsCmd_idx_3;
  real_T rtb_MultiportSwitch1;
  real_T rtb_Product;
  real_T rtb_batt_volt_V;
  real_T rtb_brake_pressure_Pa_idx_1;
  real_T rtb_brake_pressure_Pa_idx_2;
  real_T rtb_brake_pressure_Pa_idx_3;
  real_T rtb_current;
  real_T rtb_eng_trqNm;
  real_T rtb_mot_front_trqNm;
  real_T rtb_mot_rear_trqNm;
  real_T rtb_speed;
  real_T rtb_throttle_01;
  if (rtmIsMajorTimeStep((&TxCar_M))) {
    /* set solver stop time */
    if (!((&TxCar_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&TxCar_M)->solverInfo,
                            (((&TxCar_M)->Timing.clockTickH0 + 1) * (&TxCar_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(&(&TxCar_M)->solverInfo,
                            (((&TxCar_M)->Timing.clockTick0 + 1) * (&TxCar_M)->Timing.stepSize0 +
                             (&TxCar_M)->Timing.clockTickH0 * (&TxCar_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&TxCar_M))) {
    (&TxCar_M)->Timing.t[0] = rtsiGetT(&(&TxCar_M)->solverInfo);
  }

  if (rtmIsMajorTimeStep((&TxCar_M))) {
    /* Constant: '<S4>/Constant3' incorporates:
     *  BusCreator generated from: '<S1>/Chassis'
     */
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[0] = TxCar_P.Constant3_Value_l;

    /* Constant: '<S4>/Constant4' incorporates:
     *  BusCreator generated from: '<S1>/Chassis'
     */
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[1] = TxCar_P.Constant4_Value_c;

    /* Constant: '<S4>/Constant5' incorporates:
     *  BusCreator generated from: '<S1>/Chassis'
     */
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[2] = TxCar_P.Constant5_Value_m;

    /* Switch: '<S21>/Switch' incorporates:
     *  Constant: '<S22>/Constant'
     *  Inport generated from: '<Root>/Friction'
     *  RelationalOperator: '<S22>/Compare'
     */
    if (TxCar_U.Friction.mu_FL <= TxCar_P.CompareToConstant_const) {
      /* BusCreator generated from: '<S1>/Chassis' incorporates:
       *  Constant: '<S21>/const_mu'
       */
      TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FL = TxCar_P.const_mu_Value[0];
      TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FR = TxCar_P.const_mu_Value[1];
      TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RL = TxCar_P.const_mu_Value[2];
      TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RR = TxCar_P.const_mu_Value[3];
    } else {
      /* BusCreator generated from: '<S1>/Chassis' */
      TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FL = TxCar_U.Friction.mu_FL;
      TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FR = TxCar_U.Friction.mu_FR;
      TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RL = TxCar_U.Friction.mu_RL;
      TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RR = TxCar_U.Friction.mu_RR;
    }

    /* End of Switch: '<S21>/Switch' */

    /* BusCreator generated from: '<S1>/Chassis' incorporates:
     *  Inport generated from: '<Root>/Height'
     */
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.z = TxCar_U.Height;

    /* BusCreator generated from: '<S1>/Chassis' incorporates:
     *  Memory generated from: '<S7>/Memory'
     */
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[0] = TxCar_DW.Memory_1_PreviousInput[0];
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[0] = TxCar_DW.Memory_2_PreviousInput[0];
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[0] = TxCar_DW.Memory_3_PreviousInput[0];
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[1] = TxCar_DW.Memory_1_PreviousInput[1];
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[1] = TxCar_DW.Memory_2_PreviousInput[1];
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[1] = TxCar_DW.Memory_3_PreviousInput[1];
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[2] = TxCar_DW.Memory_1_PreviousInput[2];
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[2] = TxCar_DW.Memory_2_PreviousInput[2];
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[2] = TxCar_DW.Memory_3_PreviousInput[2];
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[3] = TxCar_DW.Memory_1_PreviousInput[3];
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[3] = TxCar_DW.Memory_2_PreviousInput[3];
    TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[3] = TxCar_DW.Memory_3_PreviousInput[3];

    /* S-Function (chassis_sfunc): '<S1>/Chassis' incorporates:
     *  Outport generated from: '<Root>/Body'
     *  Outport generated from: '<Root>/Susp'
     *  Outport generated from: '<Root>/WheelsOut'
     */
    chassis_sfunc_Outputs_wrapper_cgen(&TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1,
                                       &TxCar_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1,
                                       &TxCar_Y.Body, &TxCar_Y.WheelsOut, &TxCar_Y.Susp, TxCar_P.Chassis_P1, 21,
                                       TxCar_P.Chassis_P2, 22);

    /* Gain: '<S42>/Gain' incorporates:
     *  Outport generated from: '<Root>/Body'
     */
    TxCar_B.ax = TxCar_P.Gain_Gain * TxCar_Y.Body.BdyFrm.Cg.Acc.xddot;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' incorporates:
     *  Memory generated from: '<S25>/Memory'
     */
    TxCar_Y.PowerBus_Out_battery_info_batt_soc_null = TxCar_DW.Memory_5_PreviousInput;
  }

  /* BusCreator: '<S42>/Bus Creator1' incorporates:
   *  Constant: '<S45>/Constant1'
   *  Inport generated from: '<Root>/DriverIn'
   *  Integrator: '<S15>/Integrator1'
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/PowerBus_Outport_2'
   *  Outport generated from: '<Root>/WheelsOut'
   */
  TxCar_B.BusCreator1.acc_feedback_m_s2 = TxCar_B.ax;
  TxCar_B.BusCreator1.eng_spd_rad_s = TxCar_X.Integrator1_CSTATE;
  TxCar_B.BusCreator1.batt_soc_0_1 = TxCar_Y.PowerBus_Out_battery_info_batt_soc_null;
  TxCar_B.BusCreator1.diff_trq_req_Nm = TxCar_P.Constant1_Value_k;
  TxCar_B.BusCreator1.slip_ratio[0] = TxCar_Y.WheelsOut.TireFrame.Kappa[0];
  TxCar_B.BusCreator1.slip_ratio[1] = TxCar_Y.WheelsOut.TireFrame.Kappa[1];
  TxCar_B.BusCreator1.slip_ratio[2] = TxCar_Y.WheelsOut.TireFrame.Kappa[2];
  TxCar_B.BusCreator1.slip_ratio[3] = TxCar_Y.WheelsOut.TireFrame.Kappa[3];
  TxCar_B.BusCreator1.veh_speed_vx = TxCar_Y.Body.BdyFrm.Cg.Vel.xdot;
  TxCar_B.BusCreator1.driver_input = TxCar_U.DriverIn;
  if (rtmIsMajorTimeStep((&TxCar_M))) {
    /* S-Function (ecu_sfunction): '<S41>/S-Function' */
    ecu_sfunction_Outputs_wrapper_cgen(&TxCar_B.BusCreator1, &TxCar_B.SFunction, TxCar_P.SFunction_P1, 21,
                                       TxCar_P.SFunction_P2, 22);

    /* Gain: '<S16>/Gain5' */
    TxCar_B.acc_cmd = TxCar_P.Gain5_Gain * TxCar_B.SFunction.throttle_01;

    /* Gain: '<S16>/Gain1' incorporates:
     *  Constant: '<S16>/Constant1'
     */
    TxCar_B.brake_cmd_0_1 = TxCar_P.Gain1_Gain * TxCar_P.Constant1_Value_f;

    /* Gain: '<S16>/Gain2' incorporates:
     *  Outport generated from: '<Root>/WheelsOut'
     */
    TxCar_B.wheel_spd_rad_s[0] = TxCar_P.Gain2_Gain * TxCar_Y.WheelsOut.TireFrame.Omega[0];
    TxCar_B.wheel_spd_rad_s[1] = TxCar_P.Gain2_Gain * TxCar_Y.WheelsOut.TireFrame.Omega[1];
    TxCar_B.wheel_spd_rad_s[2] = TxCar_P.Gain2_Gain * TxCar_Y.WheelsOut.TireFrame.Omega[2];
    TxCar_B.wheel_spd_rad_s[3] = TxCar_P.Gain2_Gain * TxCar_Y.WheelsOut.TireFrame.Omega[3];

    /* Gain: '<S16>/Gain3' incorporates:
     *  Constant: '<S16>/Constant2'
     */
    TxCar_B.abs_flag = TxCar_P.Gain3_Gain * TxCar_P.Constant2_Value;

    /* Memory generated from: '<S25>/Memory' */
    rtb_eng_trqNm = TxCar_DW.Memory_1_PreviousInput_i;

    /* Gain: '<S16>/Gain4' incorporates:
     *  Memory generated from: '<S25>/Memory'
     */
    TxCar_B.engine_trq_Nm = TxCar_P.Gain4_Gain * TxCar_DW.Memory_1_PreviousInput_i;

    /* SignalConversion generated from: '<S45>/Vector Concatenate2' incorporates:
     *  Constant: '<S45>/Constant'
     */
    rtb_DiffPrsCmd_idx_0 = TxCar_P.Constant_Value;

    /* SignalConversion generated from: '<S45>/Vector Concatenate2' incorporates:
     *  Constant: '<S45>/Constant'
     */
    rtb_DiffPrsCmd_idx_1 = TxCar_P.Constant_Value;

    /* SignalConversion generated from: '<S45>/Vector Concatenate2' incorporates:
     *  Constant: '<S45>/Constant'
     */
    rtb_DiffPrsCmd_idx_2 = TxCar_P.Constant_Value;

    /* SignalConversion generated from: '<S45>/Vector Concatenate2' incorporates:
     *  Constant: '<S45>/Constant'
     */
    rtb_DiffPrsCmd_idx_3 = TxCar_P.Constant_Value;

    /* DataTypeConversion: '<S16>/Data Type Conversion' incorporates:
     *  Constant: '<S45>/Constant'
     */
    TxCar_B.diff_prs_cmd[0] = (TxCar_P.Constant_Value != 0.0);
    TxCar_B.diff_prs_cmd[1] = (TxCar_P.Constant_Value != 0.0);
    TxCar_B.diff_prs_cmd[2] = (TxCar_P.Constant_Value != 0.0);
    TxCar_B.diff_prs_cmd[3] = (TxCar_P.Constant_Value != 0.0);

    /* Memory generated from: '<S25>/Memory' */
    rtb_mot_front_trqNm = TxCar_DW.Memory_2_PreviousInput_p;

    /* Gain: '<S16>/Gain6' incorporates:
     *  Memory generated from: '<S25>/Memory'
     */
    TxCar_B.mot_front_trq_Nm = TxCar_P.Gain6_Gain * TxCar_DW.Memory_2_PreviousInput_p;

    /* Memory generated from: '<S25>/Memory' */
    rtb_mot_rear_trqNm = TxCar_DW.Memory_3_PreviousInput_k;

    /* Gain: '<S16>/Gain7' incorporates:
     *  Memory generated from: '<S25>/Memory'
     */
    TxCar_B.mot_rear_trq_Nm = TxCar_P.Gain7_Gain * TxCar_DW.Memory_3_PreviousInput_k;
  }

  /* MultiPortSwitch: '<S17>/Multiport Switch' incorporates:
   *  Inport generated from: '<Root>/DriverIn'
   */
  switch (TxCar_U.DriverIn.gear_cmd__0N1D2R3P) {
    case 0:
      /* BusCreator: '<S16>/Bus Creator' incorporates:
       *  Constant: '<S17>/N'
       */
      TxCar_B.driveline.gear_cmd_driver = TxCar_P.N_Value;
      break;

    case 1:
      /* BusCreator: '<S16>/Bus Creator' incorporates:
       *  Constant: '<S17>/D'
       */
      TxCar_B.driveline.gear_cmd_driver = TxCar_P.D_Value;
      break;

    case 2:
      /* BusCreator: '<S16>/Bus Creator' incorporates:
       *  Constant: '<S17>/R'
       */
      TxCar_B.driveline.gear_cmd_driver = TxCar_P.R_Value;
      break;

    default:
      /* BusCreator: '<S16>/Bus Creator' incorporates:
       *  Constant: '<S17>/P'
       */
      TxCar_B.driveline.gear_cmd_driver = TxCar_P.P_Value;
      break;
  }

  /* End of MultiPortSwitch: '<S17>/Multiport Switch' */

  /* BusCreator: '<S16>/Bus Creator' */
  TxCar_B.driveline.acc_cmd = TxCar_B.acc_cmd;
  TxCar_B.driveline.brake_cmd_0_1 = TxCar_B.brake_cmd_0_1;
  TxCar_B.driveline.abs_flag = TxCar_B.abs_flag;
  TxCar_B.driveline.engine_trq_Nm = TxCar_B.engine_trq_Nm;
  TxCar_B.driveline.wheel_spd_rad_s[0] = TxCar_B.wheel_spd_rad_s[0];
  TxCar_B.driveline.diff_prs_cmd[0] = TxCar_B.diff_prs_cmd[0];
  TxCar_B.driveline.wheel_spd_rad_s[1] = TxCar_B.wheel_spd_rad_s[1];
  TxCar_B.driveline.diff_prs_cmd[1] = TxCar_B.diff_prs_cmd[1];
  TxCar_B.driveline.wheel_spd_rad_s[2] = TxCar_B.wheel_spd_rad_s[2];
  TxCar_B.driveline.diff_prs_cmd[2] = TxCar_B.diff_prs_cmd[2];
  TxCar_B.driveline.wheel_spd_rad_s[3] = TxCar_B.wheel_spd_rad_s[3];
  TxCar_B.driveline.diff_prs_cmd[3] = TxCar_B.diff_prs_cmd[3];
  TxCar_B.driveline.mot_front_trq_Nm = TxCar_B.mot_front_trq_Nm;
  TxCar_B.driveline.mot_rear_trq_Nm = TxCar_B.mot_rear_trq_Nm;
  if (rtmIsMajorTimeStep((&TxCar_M))) {
    /* S-Function (driveline_sfunction): '<S14>/S-Function' */
    driveline_sfunction_Outputs_wrapper_cgen(&TxCar_B.driveline, &TxCar_B.SFunction, &TxCar_B.SFunction_a,
                                             TxCar_P.SFunction_P1_m, 21, TxCar_P.SFunction_P2_k, 22);
  }

  /* Product: '<S15>/Divide' incorporates:
   *  Constant: '<S15>/Constant'
   *  Integrator: '<S15>/Integrator1'
   *  Sum: '<S15>/Sum'
   */
  TxCar_B.Divide = (TxCar_B.SFunction_a.eng_spd_rad_s - TxCar_X.Integrator1_CSTATE) * TxCar_P.EngSpdAct_wc;
  if (rtmIsMajorTimeStep((&TxCar_M))) {
    /* Saturate: '<S10>/Saturation' */
    if (TxCar_B.SFunction_a.wheel_drive_trq_Nm[0] > TxCar_P.Saturation_UpperSat) {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[0] = TxCar_P.Saturation_UpperSat;
    } else if (TxCar_B.SFunction_a.wheel_drive_trq_Nm[0] < TxCar_P.Saturation_LowerSat) {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[0] = TxCar_P.Saturation_LowerSat;
    } else {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[0] = TxCar_B.SFunction_a.wheel_drive_trq_Nm[0];
    }

    if (TxCar_B.SFunction_a.wheel_drive_trq_Nm[1] > TxCar_P.Saturation_UpperSat) {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[1] = TxCar_P.Saturation_UpperSat;
    } else if (TxCar_B.SFunction_a.wheel_drive_trq_Nm[1] < TxCar_P.Saturation_LowerSat) {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[1] = TxCar_P.Saturation_LowerSat;
    } else {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[1] = TxCar_B.SFunction_a.wheel_drive_trq_Nm[1];
    }

    if (TxCar_B.SFunction_a.wheel_drive_trq_Nm[2] > TxCar_P.Saturation_UpperSat) {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[2] = TxCar_P.Saturation_UpperSat;
    } else if (TxCar_B.SFunction_a.wheel_drive_trq_Nm[2] < TxCar_P.Saturation_LowerSat) {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[2] = TxCar_P.Saturation_LowerSat;
    } else {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[2] = TxCar_B.SFunction_a.wheel_drive_trq_Nm[2];
    }

    if (TxCar_B.SFunction_a.wheel_drive_trq_Nm[3] > TxCar_P.Saturation_UpperSat) {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[3] = TxCar_P.Saturation_UpperSat;
    } else if (TxCar_B.SFunction_a.wheel_drive_trq_Nm[3] < TxCar_P.Saturation_LowerSat) {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[3] = TxCar_P.Saturation_LowerSat;
    } else {
      /* Saturate: '<S10>/Saturation' */
      rtb_Saturation[3] = TxCar_B.SFunction_a.wheel_drive_trq_Nm[3];
    }

    /* End of Saturate: '<S10>/Saturation' */

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' incorporates:
     *  Gain: '<S40>/Gain2'
     */
    TxCar_Y.SoftEcu_Out_Brake_BrkPrsCmd01 = TxCar_P.Gain2_Gain_l * TxCar_B.SFunction.brake_pressure_0_1;

    /* S-Function (brakehydraulic_sfunc): '<S13>/S-Function' incorporates:
     *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
     *  Outport generated from: '<Root>/SoftEcu_Outport_4'
     */
    brakehydraulic_sfunc_Outputs_wrapper_cgen(&TxCar_Y.SoftEcu_Out_Brake_BrkPrsCmd01,
                                              &TxCar_Y.DriveLineBus_Out_BrkPrs[0], TxCar_P.SFunction_P1_f, 21,
                                              TxCar_P.SFunction_P2_i, 22);

    /* Memory generated from: '<S25>/Memory' */
    rtb_batt_volt_V = TxCar_DW.Memory_4_PreviousInput;

    /* Memory generated from: '<S25>/Memory' */
    rtb_current = TxCar_DW.Memory_6_PreviousInput;

    /* S-Function (cartype_sfunction): '<S28>/S-Function' incorporates:
     *  Constant: '<S28>/Zero'
     */
    cartype_sfunction_Outputs_wrapper_cgen(&TxCar_P.Zero_Value, &TxCar_B.SFunction_e, TxCar_P.SFunction_P1_l, 21,
                                           TxCar_P.SFunction_P2_o, 22);

    /* Gain: '<S43>/Gain1' */
    rtb_throttle_01 = TxCar_P.Gain1_Gain_f * TxCar_B.SFunction.throttle_01;

    /* MultiPortSwitch: '<S33>/Multiport Switch2' */
    if (static_cast<int32_T>(TxCar_B.SFunction_e.proplutiontype) == 1) {
      /* MultiPortSwitch: '<S33>/Multiport Switch2' */
      TxCar_B.throttle_01 = rtb_throttle_01;
    } else {
      /* MultiPortSwitch: '<S33>/Multiport Switch2' incorporates:
       *  Constant: '<S33>/none'
       */
      TxCar_B.throttle_01 = TxCar_P.none_Value;
    }

    /* End of MultiPortSwitch: '<S33>/Multiport Switch2' */
  }

  /* BusCreator generated from: '<S29>/S-Function' */
  TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_f.throttle_01 = TxCar_B.throttle_01;

  /* MultiPortSwitch: '<S32>/Multiport Switch2' incorporates:
   *  Constant: '<S32>/none'
   *  Integrator: '<S15>/Integrator1'
   */
  if (static_cast<int32_T>(TxCar_B.SFunction_e.proplutiontype) == 1) {
    rtb_speed = TxCar_X.Integrator1_CSTATE;
  } else {
    rtb_speed = TxCar_P.none_Value_d;
  }

  /* BusCreator generated from: '<S29>/S-Function' incorporates:
   *  Gain: '<S32>/Gain'
   *  MultiPortSwitch: '<S32>/Multiport Switch2'
   */
  TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_f.eng_spd_rpm = TxCar_P.Gain_Gain_d * rtb_speed;
  if (rtmIsMajorTimeStep((&TxCar_M))) {
    /* S-Function (engine_sfunction): '<S29>/S-Function' */
    engine_sfunction_Outputs_wrapper_cgen(&TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_f,
                                          &TxCar_B.SFunction_f, TxCar_P.SFunction_P1_k, 21, TxCar_P.SFunction_P2_l, 22);

    /* Memory: '<S26>/Memory2' */
    rtb_speed = TxCar_DW.Memory2_PreviousInput;

    /* MultiPortSwitch: '<S36>/Multiport Switch1' incorporates:
     *  Constant: '<S36>/Constant1'
     */
    if (static_cast<int32_T>(TxCar_B.SFunction_e.proplutiontype) == 1) {
      rtb_MultiportSwitch1 = TxCar_P.Constant1_Value;
    } else {
      rtb_MultiportSwitch1 = rtb_speed;
    }

    /* End of MultiPortSwitch: '<S36>/Multiport Switch1' */

    /* Product: '<S34>/Product' */
    rtb_Product = rtb_MultiportSwitch1 * TxCar_B.SFunction_e.parsingbck_01;

    /* MultiPortSwitch: '<S26>/Multiport Switch3' */
    switch (static_cast<int32_T>(TxCar_B.SFunction_e.drivetype)) {
      case 1:
        /* BusCreator generated from: '<S30>/S-Function' */
        TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.trq_cmd_Nm = rtb_MultiportSwitch1;
        break;

      case 2:
        /* BusCreator generated from: '<S30>/S-Function' incorporates:
         *  Constant: '<S26>/Constant3'
         */
        TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.trq_cmd_Nm = TxCar_P.Constant3_Value;
        break;

      default:
        /* BusCreator generated from: '<S30>/S-Function' */
        TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.trq_cmd_Nm = rtb_Product;
        break;
    }

    /* End of MultiPortSwitch: '<S26>/Multiport Switch3' */

    /* Gain: '<S26>/Gain3' incorporates:
     *  Memory: '<S26>/Memory1'
     */
    rtb_brake_pressure_Pa_idx_1 = TxCar_P.Gain3_Gain_p * TxCar_DW.Memory1_PreviousInput;

    /* Saturate: '<S26>/Saturation' */
    if (rtb_brake_pressure_Pa_idx_1 > TxCar_P.Saturation_UpperSat_o) {
      /* BusCreator generated from: '<S27>/S-Function' */
      TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_curr_A = TxCar_P.Saturation_UpperSat_o;
    } else if (rtb_brake_pressure_Pa_idx_1 < TxCar_P.Saturation_LowerSat_o) {
      /* BusCreator generated from: '<S27>/S-Function' */
      TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_curr_A = TxCar_P.Saturation_LowerSat_o;
    } else {
      /* BusCreator generated from: '<S27>/S-Function' */
      TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_curr_A = rtb_brake_pressure_Pa_idx_1;
    }

    /* End of Saturate: '<S26>/Saturation' */

    /* BusCreator generated from: '<S27>/S-Function' incorporates:
     *  Constant: '<S26>/Constant'
     */
    TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_temp_K = TxCar_P.Constant_Value_n;

    /* S-Function (battery_sfunction): '<S27>/S-Function' */
    battery_sfunction_Outputs_wrapper_cgen(&TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p,
                                           &TxCar_B.SFunction_k, TxCar_P.SFunction_P1_mq, 21, TxCar_P.SFunction_P2_g,
                                           22);

    /* BusCreator generated from: '<S30>/S-Function' incorporates:
     *  Constant: '<S35>/4WD'
     *  Constant: '<S35>/FD'
     *  Gain: '<S35>/Gain2'
     *  Logic: '<S35>/Logical Operator'
     *  Product: '<S35>/Product1'
     *  RelationalOperator: '<S35>/Relational Operator'
     *  RelationalOperator: '<S35>/Relational Operator1'
     */
    TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.mot_spd_RPM =
        static_cast<real_T>((TxCar_B.SFunction_e.drivetype == TxCar_P.FD_Value) ||
                            (TxCar_B.SFunction_e.drivetype == TxCar_P.uWD_Value)) *
        (TxCar_P.Gain2_Gain_c * TxCar_B.SFunction_a.front_mot_spd_rad_s);
    TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.batt_volt_V = TxCar_B.SFunction_k.batt_volt_V;

    /* S-Function (front_motor_sfunction): '<S30>/S-Function' */
    front_motor_sfunction_Outputs_wrapper_cgen(&TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1,
                                               &TxCar_B.SFunction_h, TxCar_P.SFunction_P1_h, 21, TxCar_P.SFunction_P2_b,
                                               22);

    /* MultiPortSwitch: '<S26>/Multiport Switch4' */
    switch (static_cast<int32_T>(TxCar_B.SFunction_e.drivetype)) {
      case 1:
        /* BusCreator generated from: '<S31>/S-Function' incorporates:
         *  Constant: '<S26>/Constant4'
         */
        TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.trq_cmd_Nm = TxCar_P.Constant4_Value;
        break;

      case 2:
        /* MultiPortSwitch: '<S38>/Multiport Switch5' */
        if (static_cast<int32_T>(TxCar_B.SFunction_e.proplutiontype) == 1) {
          /* BusCreator generated from: '<S31>/S-Function' incorporates:
           *  Constant: '<S38>/Constant5'
           */
          TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.trq_cmd_Nm = TxCar_P.Constant5_Value;
        } else {
          /* BusCreator generated from: '<S31>/S-Function' */
          TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.trq_cmd_Nm = rtb_speed;
        }

        /* End of MultiPortSwitch: '<S38>/Multiport Switch5' */
        break;

      default:
        /* BusCreator generated from: '<S31>/S-Function' incorporates:
         *  Sum: '<S34>/Add1'
         */
        TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.trq_cmd_Nm =
            rtb_MultiportSwitch1 - rtb_Product;
        break;
    }

    /* End of MultiPortSwitch: '<S26>/Multiport Switch4' */

    /* BusCreator generated from: '<S31>/S-Function' incorporates:
     *  Constant: '<S37>/4WD'
     *  Constant: '<S37>/RD'
     *  Gain: '<S37>/Gain4'
     *  Logic: '<S37>/Logical Operator'
     *  Memory: '<S26>/Memory'
     *  Product: '<S37>/Product2'
     *  RelationalOperator: '<S37>/Relational Operator'
     *  RelationalOperator: '<S37>/Relational Operator2'
     */
    TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.mot_spd_RPM =
        static_cast<real_T>((TxCar_B.SFunction_e.drivetype == TxCar_P.RD_Value) ||
                            (TxCar_B.SFunction_e.drivetype == TxCar_P.uWD_Value_c)) *
        TxCar_DW.Memory_PreviousInput_a * TxCar_P.Gain4_Gain_p;
    TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.batt_volt_V = TxCar_B.SFunction_k.batt_volt_V;

    /* S-Function (rear_motor_sfunction): '<S31>/S-Function' */
    rear_motor_sfunction_Outputs_wrapper_cgen(&TxCar_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m,
                                              &TxCar_B.SFunction_ep, TxCar_P.SFunction_P1_g, 21,
                                              TxCar_P.SFunction_P2_ox, 22);

    /* Sum: '<S26>/Add' incorporates:
     *  SignalConversion generated from: '<S26>/Bus Selector4'
     */
    rtb_Add = TxCar_B.SFunction_h.batt_curr_A + TxCar_B.SFunction_ep.batt_curr_A;

    /* SignalConversion generated from: '<S26>/Bus Selector1' */
    rtb_mot_trq_Nm = TxCar_B.SFunction_h.mot_trq_Nm;

    /* SignalConversion generated from: '<S26>/Bus Selector2' */
    rtb_batt_soc_null = TxCar_B.SFunction_k.batt_soc_null;

    /* Gain: '<S40>/Gain' */
    rtb_MultiportSwitch1 = TxCar_P.Gain_Gain_a * TxCar_B.SFunction.brake_pressure_Pa[0];

    /* Gain: '<S40>/Gain1' */
    rtb_Product = TxCar_P.Gain1_Gain_o * TxCar_B.SFunction.abs_flag[0];

    /* Gain: '<S40>/Gain' */
    rtb_brake_pressure_Pa_idx_1 = TxCar_P.Gain_Gain_a * TxCar_B.SFunction.brake_pressure_Pa[1];

    /* Gain: '<S40>/Gain1' */
    rtb_ABSFlag_idx_1 = TxCar_P.Gain1_Gain_o * TxCar_B.SFunction.abs_flag[1];

    /* Gain: '<S40>/Gain' */
    rtb_brake_pressure_Pa_idx_2 = TxCar_P.Gain_Gain_a * TxCar_B.SFunction.brake_pressure_Pa[2];

    /* Gain: '<S40>/Gain1' */
    rtb_ABSFlag_idx_2 = TxCar_P.Gain1_Gain_o * TxCar_B.SFunction.abs_flag[2];

    /* Gain: '<S40>/Gain' */
    rtb_brake_pressure_Pa_idx_3 = TxCar_P.Gain_Gain_a * TxCar_B.SFunction.brake_pressure_Pa[3];

    /* Gain: '<S40>/Gain1' */
    rtb_ABSFlag_idx_3 = TxCar_P.Gain1_Gain_o * TxCar_B.SFunction.abs_flag[3];

    /* Gain: '<S43>/Gain' */
    rtb_TrqCmd = TxCar_P.Gain_Gain_m * TxCar_B.SFunction.pt_trq_cmd_Nm;

    /* Memory: '<S9>/Memory' */
    rtb_Memory = TxCar_DW.Memory_PreviousInput;
  }

  /* BusCreator generated from: '<S46>/Steer_Mapped' incorporates:
   *  Inport generated from: '<Root>/DriverIn'
   */
  TxCar_B.BusConversion_InsertedFor_Steer_Mapped_at_inport_0_BusCreator1 = TxCar_U.DriverIn;
  if (rtmIsMajorTimeStep((&TxCar_M))) {
    /* S-Function (dynamic_steer_sfunc): '<S46>/Steer_Mapped' incorporates:
     *  Outport generated from: '<Root>/Body'
     *  Outport generated from: '<Root>/Susp'
     *  Outport generated from: '<Root>/WheelsOut'
     */
    dynamic_steer_sfunc_Outputs_wrapper_cgen(&TxCar_B.BusConversion_InsertedFor_Steer_Mapped_at_inport_0_BusCreator1,
                                             &TxCar_Y.Body, &TxCar_Y.Susp, &TxCar_Y.WheelsOut, &TxCar_B.Steer_Mapped,
                                             TxCar_P.Steer_Mapped_P1, 21, TxCar_P.Steer_Mapped_P2, 22);

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_Y.DriveLineBus_Out_AxlTrq[0] = rtb_Saturation[0];
    TxCar_Y.DriveLineBus_Out_AxlTrq[1] = rtb_Saturation[1];
    TxCar_Y.DriveLineBus_Out_AxlTrq[2] = rtb_Saturation[2];
    TxCar_Y.DriveLineBus_Out_AxlTrq[3] = rtb_Saturation[3];
  }

  /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
   *  Integrator: '<S15>/Integrator1'
   */
  TxCar_Y.DriveLineBus_Out_EngSpd = TxCar_X.Integrator1_CSTATE;
  if (rtmIsMajorTimeStep((&TxCar_M))) {
    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_Y.DriveLineBus_Out_PropShftSpd = TxCar_B.SFunction_a.trans_out_shaft_spd_rad_s;

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_Y.DriveLineBus_Out_Gear = TxCar_B.SFunction_a.gear_engaged;

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_Y.DriveLineBus_Out_FrontMotSpd = TxCar_B.SFunction_a.front_mot_spd_rad_s;

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_Y.DriveLineBus_Out_RearMotSpd = TxCar_B.SFunction_a.rear_mot_spd_rad_s;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_Y.PowerBus_Out_eng_trqNm = rtb_eng_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_Y.PowerBus_Out_mot_front_trqNm = rtb_mot_front_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_Y.PowerBus_Out_mot_rear_trqNm = rtb_mot_rear_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_Y.PowerBus_Out_battery_info_batt_volt_V = rtb_batt_volt_V;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_Y.PowerBus_Out_current = rtb_current;

    /* Outport generated from: '<Root>/Bus Element Out2' */
    TxCar_Y.SteerBus_Out = rtb_Memory;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_ECU_TrqCmd = rtb_TrqCmd;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_ECU_throttle_01 = rtb_throttle_01;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' incorporates:
     *  Constant: '<S40>/Constant'
     */
    TxCar_Y.SoftEcu_Out_Brake_TCSFlag = TxCar_P.Constant_Value_p;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Brake_ABSFlag[0] = rtb_Product;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Brake_brake_pressure_Pa[0] = rtb_MultiportSwitch1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Diff_DiffPrsCmd[0] = rtb_DiffPrsCmd_idx_0;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Brake_ABSFlag[1] = rtb_ABSFlag_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Brake_brake_pressure_Pa[1] = rtb_brake_pressure_Pa_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Diff_DiffPrsCmd[1] = rtb_DiffPrsCmd_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Brake_ABSFlag[2] = rtb_ABSFlag_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Brake_brake_pressure_Pa[2] = rtb_brake_pressure_Pa_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Diff_DiffPrsCmd[2] = rtb_DiffPrsCmd_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Brake_ABSFlag[3] = rtb_ABSFlag_idx_3;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Brake_brake_pressure_Pa[3] = rtb_brake_pressure_Pa_idx_3;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_Y.SoftEcu_Out_Diff_DiffPrsCmd[3] = rtb_DiffPrsCmd_idx_3;
  }

  if (rtmIsMajorTimeStep((&TxCar_M))) {
    if (rtmIsMajorTimeStep((&TxCar_M))) {
      /* Update for Memory generated from: '<S7>/Memory' */
      TxCar_DW.Memory_1_PreviousInput[0] = rtb_Memory.wheel_angle[0];

      /* Update for Memory generated from: '<S7>/Memory' */
      TxCar_DW.Memory_2_PreviousInput[0] = rtb_Saturation[0];

      /* Update for Memory generated from: '<S7>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_DW.Memory_3_PreviousInput[0] = TxCar_Y.DriveLineBus_Out_BrkPrs[0];

      /* Update for Memory generated from: '<S7>/Memory' */
      TxCar_DW.Memory_1_PreviousInput[1] = rtb_Memory.wheel_angle[1];

      /* Update for Memory generated from: '<S7>/Memory' */
      TxCar_DW.Memory_2_PreviousInput[1] = rtb_Saturation[1];

      /* Update for Memory generated from: '<S7>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_DW.Memory_3_PreviousInput[1] = TxCar_Y.DriveLineBus_Out_BrkPrs[1];

      /* Update for Memory generated from: '<S7>/Memory' */
      TxCar_DW.Memory_1_PreviousInput[2] = rtb_Memory.wheel_angle[2];

      /* Update for Memory generated from: '<S7>/Memory' */
      TxCar_DW.Memory_2_PreviousInput[2] = rtb_Saturation[2];

      /* Update for Memory generated from: '<S7>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_DW.Memory_3_PreviousInput[2] = TxCar_Y.DriveLineBus_Out_BrkPrs[2];

      /* Update for Memory generated from: '<S7>/Memory' */
      TxCar_DW.Memory_1_PreviousInput[3] = rtb_Memory.wheel_angle[3];

      /* Update for Memory generated from: '<S7>/Memory' */
      TxCar_DW.Memory_2_PreviousInput[3] = rtb_Saturation[3];

      /* Update for Memory generated from: '<S7>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_DW.Memory_3_PreviousInput[3] = TxCar_Y.DriveLineBus_Out_BrkPrs[3];

      /* Update for Memory generated from: '<S25>/Memory' */
      TxCar_DW.Memory_5_PreviousInput = rtb_batt_soc_null;

      /* Update for Memory generated from: '<S25>/Memory' */
      TxCar_DW.Memory_1_PreviousInput_i = TxCar_B.SFunction_f.eng_trq_Nm;

      /* Update for Memory generated from: '<S25>/Memory' */
      TxCar_DW.Memory_2_PreviousInput_p = rtb_mot_trq_Nm;

      /* Update for Memory generated from: '<S25>/Memory' */
      TxCar_DW.Memory_3_PreviousInput_k = TxCar_B.SFunction_ep.mot_trq_Nm;

      /* Update for Memory generated from: '<S25>/Memory' */
      TxCar_DW.Memory_4_PreviousInput = TxCar_B.SFunction_k.batt_volt_V;

      /* Update for Memory generated from: '<S25>/Memory' */
      TxCar_DW.Memory_6_PreviousInput = rtb_Add;

      /* Update for Memory: '<S26>/Memory2' */
      TxCar_DW.Memory2_PreviousInput = rtb_TrqCmd;

      /* Update for Memory: '<S26>/Memory1' */
      TxCar_DW.Memory1_PreviousInput = rtb_Add;

      /* Update for Memory: '<S26>/Memory' */
      TxCar_DW.Memory_PreviousInput_a = TxCar_B.SFunction_a.rear_mot_spd_rad_s;

      /* Update for Memory: '<S9>/Memory' */
      TxCar_DW.Memory_PreviousInput = TxCar_B.Steer_Mapped;
    }
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&TxCar_M))) {
    rt_ertODEUpdateContinuousStates(&(&TxCar_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&TxCar_M)->Timing.clockTick0)) {
      ++(&TxCar_M)->Timing.clockTickH0;
    }

    (&TxCar_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&TxCar_M)->solverInfo);

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
      (&TxCar_M)->Timing.clockTick1++;
      if (!(&TxCar_M)->Timing.clockTick1) {
        (&TxCar_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void TxCar::TxCar_derivatives() {
  XDot_TxCar_T *_rtXdot;
  _rtXdot = ((XDot_TxCar_T *)(&TxCar_M)->derivs);

  /* Derivatives for Integrator: '<S15>/Integrator1' */
  _rtXdot->Integrator1_CSTATE = TxCar_B.Divide;
}

/* Model initialize function */
void TxCar::initialize() {
  /* Registration code */
  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&TxCar_M)->solverInfo, &(&TxCar_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&TxCar_M)->solverInfo, &rtmGetTPtr((&TxCar_M)));
    rtsiSetStepSizePtr(&(&TxCar_M)->solverInfo, &(&TxCar_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&TxCar_M)->solverInfo, &(&TxCar_M)->derivs);
    rtsiSetContStatesPtr(&(&TxCar_M)->solverInfo, (real_T **)&(&TxCar_M)->contStates);
    rtsiSetNumContStatesPtr(&(&TxCar_M)->solverInfo, &(&TxCar_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&TxCar_M)->solverInfo, &(&TxCar_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&TxCar_M)->solverInfo, &(&TxCar_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&TxCar_M)->solverInfo, &(&TxCar_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&TxCar_M)->solverInfo, (&rtmGetErrorStatus((&TxCar_M))));
    rtsiSetRTModelPtr(&(&TxCar_M)->solverInfo, (&TxCar_M));
  }

  rtsiSetSimTimeStep(&(&TxCar_M)->solverInfo, MAJOR_TIME_STEP);
  (&TxCar_M)->intgData.y = (&TxCar_M)->odeY;
  (&TxCar_M)->intgData.f[0] = (&TxCar_M)->odeF[0];
  (&TxCar_M)->intgData.f[1] = (&TxCar_M)->odeF[1];
  (&TxCar_M)->intgData.f[2] = (&TxCar_M)->odeF[2];
  (&TxCar_M)->intgData.f[3] = (&TxCar_M)->odeF[3];
  (&TxCar_M)->contStates = ((X_TxCar_T *)&TxCar_X);
  rtsiSetSolverData(&(&TxCar_M)->solverInfo, static_cast<void *>(&(&TxCar_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&TxCar_M)->solverInfo, false);
  rtsiSetSolverName(&(&TxCar_M)->solverInfo, "ode4");
  rtmSetTPtr((&TxCar_M), &(&TxCar_M)->Timing.tArray[0]);
  (&TxCar_M)->Timing.stepSize0 = 0.001;

  /* Start for S-Function (chassis_sfunc): '<S1>/Chassis' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S1>/Chassis */
  chassis_sfunc_Start_wrapper_cgen(TxCar_P.Chassis_P1, 21, TxCar_P.Chassis_P2, 22);

  /* Start for S-Function (ecu_sfunction): '<S41>/S-Function' */

  /* S-Function Block: <S41>/S-Function */
  ecu_sfunction_Start_wrapper_cgen(TxCar_P.SFunction_P1, 21, TxCar_P.SFunction_P2, 22);

  /* Start for S-Function (driveline_sfunction): '<S14>/S-Function' */

  /* S-Function Block: <S14>/S-Function */
  driveline_sfunction_Start_wrapper_cgen(TxCar_P.SFunction_P1_m, 21, TxCar_P.SFunction_P2_k, 22);

  /* Start for S-Function (brakehydraulic_sfunc): '<S13>/S-Function' incorporates:
   *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
   *  Outport generated from: '<Root>/SoftEcu_Outport_4'
   */

  /* S-Function Block: <S13>/S-Function */
  brakehydraulic_sfunc_Start_wrapper_cgen(TxCar_P.SFunction_P1_f, 21, TxCar_P.SFunction_P2_i, 22);

  /* Start for S-Function (cartype_sfunction): '<S28>/S-Function' incorporates:
   *  Constant: '<S28>/Zero'
   */

  /* S-Function Block: <S28>/S-Function */
  cartype_sfunction_Start_wrapper_cgen(TxCar_P.SFunction_P1_l, 21, TxCar_P.SFunction_P2_o, 22);

  /* Start for S-Function (engine_sfunction): '<S29>/S-Function' */

  /* S-Function Block: <S29>/S-Function */
  engine_sfunction_Start_wrapper_cgen(TxCar_P.SFunction_P1_k, 21, TxCar_P.SFunction_P2_l, 22);

  /* Start for S-Function (battery_sfunction): '<S27>/S-Function' */

  /* S-Function Block: <S27>/S-Function */
  battery_sfunction_Start_wrapper_cgen(TxCar_P.SFunction_P1_mq, 21, TxCar_P.SFunction_P2_g, 22);

  /* Start for S-Function (front_motor_sfunction): '<S30>/S-Function' */

  /* S-Function Block: <S30>/S-Function */
  front_motor_sfunction_Start_wrapper_cgen(TxCar_P.SFunction_P1_h, 21, TxCar_P.SFunction_P2_b, 22);

  /* Start for S-Function (rear_motor_sfunction): '<S31>/S-Function' */

  /* S-Function Block: <S31>/S-Function */
  rear_motor_sfunction_Start_wrapper_cgen(TxCar_P.SFunction_P1_g, 21, TxCar_P.SFunction_P2_ox, 22);

  /* Start for S-Function (dynamic_steer_sfunc): '<S46>/Steer_Mapped' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S46>/Steer_Mapped */
  dynamic_steer_sfunc_Start_wrapper_cgen(TxCar_P.Steer_Mapped_P1, 21, TxCar_P.Steer_Mapped_P2, 22);

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_1_PreviousInput[0] = TxCar_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_2_PreviousInput[0] = TxCar_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_3_PreviousInput[0] = TxCar_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_1_PreviousInput[1] = TxCar_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_2_PreviousInput[1] = TxCar_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_3_PreviousInput[1] = TxCar_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_1_PreviousInput[2] = TxCar_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_2_PreviousInput[2] = TxCar_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_3_PreviousInput[2] = TxCar_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_1_PreviousInput[3] = TxCar_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_2_PreviousInput[3] = TxCar_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S7>/Memory' */
  TxCar_DW.Memory_3_PreviousInput[3] = TxCar_P.Memory_3_InitialCondition;

  /* InitializeConditions for Integrator: '<S15>/Integrator1' */
  TxCar_X.Integrator1_CSTATE = TxCar_P.Integrator1_IC;

  /* InitializeConditions for Memory generated from: '<S25>/Memory' */
  TxCar_DW.Memory_5_PreviousInput = TxCar_P.Memory_5_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S25>/Memory' */
  TxCar_DW.Memory_1_PreviousInput_i = TxCar_P.Memory_1_InitialCondition_l;

  /* InitializeConditions for Memory generated from: '<S25>/Memory' */
  TxCar_DW.Memory_2_PreviousInput_p = TxCar_P.Memory_2_InitialCondition_l;

  /* InitializeConditions for Memory generated from: '<S25>/Memory' */
  TxCar_DW.Memory_3_PreviousInput_k = TxCar_P.Memory_3_InitialCondition_l;

  /* InitializeConditions for Memory generated from: '<S25>/Memory' */
  TxCar_DW.Memory_4_PreviousInput = TxCar_P.Memory_4_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S25>/Memory' */
  TxCar_DW.Memory_6_PreviousInput = TxCar_P.Memory_6_InitialCondition;

  /* InitializeConditions for Memory: '<S26>/Memory2' */
  TxCar_DW.Memory2_PreviousInput = TxCar_P.Memory2_InitialCondition;

  /* InitializeConditions for Memory: '<S26>/Memory1' */
  TxCar_DW.Memory1_PreviousInput = TxCar_P.Memory1_InitialCondition;

  /* InitializeConditions for Memory: '<S26>/Memory' */
  TxCar_DW.Memory_PreviousInput_a = TxCar_P.Memory_InitialCondition_e;

  /* InitializeConditions for Memory: '<S9>/Memory' */
  TxCar_DW.Memory_PreviousInput = TxCar_P.Memory_InitialCondition;
}

/* Model terminate function */
void TxCar::terminate() {
  /* Terminate for S-Function (chassis_sfunc): '<S1>/Chassis' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S1>/Chassis */
  chassis_sfunc_Terminate_wrapper_cgen(TxCar_P.Chassis_P1, 21, TxCar_P.Chassis_P2, 22);

  /* Terminate for S-Function (ecu_sfunction): '<S41>/S-Function' */

  /* S-Function Block: <S41>/S-Function */
  ecu_sfunction_Terminate_wrapper_cgen(TxCar_P.SFunction_P1, 21, TxCar_P.SFunction_P2, 22);

  /* Terminate for S-Function (driveline_sfunction): '<S14>/S-Function' */

  /* S-Function Block: <S14>/S-Function */
  driveline_sfunction_Terminate_wrapper_cgen(TxCar_P.SFunction_P1_m, 21, TxCar_P.SFunction_P2_k, 22);

  /* Terminate for S-Function (brakehydraulic_sfunc): '<S13>/S-Function' incorporates:
   *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
   *  Outport generated from: '<Root>/SoftEcu_Outport_4'
   */

  /* S-Function Block: <S13>/S-Function */
  brakehydraulic_sfunc_Terminate_wrapper_cgen(TxCar_P.SFunction_P1_f, 21, TxCar_P.SFunction_P2_i, 22);

  /* Terminate for S-Function (cartype_sfunction): '<S28>/S-Function' incorporates:
   *  Constant: '<S28>/Zero'
   */

  /* S-Function Block: <S28>/S-Function */
  cartype_sfunction_Terminate_wrapper_cgen(TxCar_P.SFunction_P1_l, 21, TxCar_P.SFunction_P2_o, 22);

  /* Terminate for S-Function (engine_sfunction): '<S29>/S-Function' */

  /* S-Function Block: <S29>/S-Function */
  engine_sfunction_Terminate_wrapper_cgen(TxCar_P.SFunction_P1_k, 21, TxCar_P.SFunction_P2_l, 22);

  /* Terminate for S-Function (battery_sfunction): '<S27>/S-Function' */

  /* S-Function Block: <S27>/S-Function */
  battery_sfunction_Terminate_wrapper_cgen(TxCar_P.SFunction_P1_mq, 21, TxCar_P.SFunction_P2_g, 22);

  /* Terminate for S-Function (front_motor_sfunction): '<S30>/S-Function' */

  /* S-Function Block: <S30>/S-Function */
  front_motor_sfunction_Terminate_wrapper_cgen(TxCar_P.SFunction_P1_h, 21, TxCar_P.SFunction_P2_b, 22);

  /* Terminate for S-Function (rear_motor_sfunction): '<S31>/S-Function' */

  /* S-Function Block: <S31>/S-Function */
  rear_motor_sfunction_Terminate_wrapper_cgen(TxCar_P.SFunction_P1_g, 21, TxCar_P.SFunction_P2_ox, 22);

  /* Terminate for S-Function (dynamic_steer_sfunc): '<S46>/Steer_Mapped' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S46>/Steer_Mapped */
  dynamic_steer_sfunc_Terminate_wrapper_cgen(TxCar_P.Steer_Mapped_P1, 21, TxCar_P.Steer_Mapped_P2, 22);
}

/* Constructor */
TxCar::TxCar() : TxCar_U(), TxCar_Y(), TxCar_B(), TxCar_DW(), TxCar_X(), TxCar_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
TxCar::~TxCar() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_TxCar_T *TxCar::getRTM() { return (&TxCar_M); }
