/*
 * TxCar_EV.cpp
 *
 * Code generation for model "TxCar_EV".
 *
 * Model version              : 2.1534
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul 19 14:25:50 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TxCar_EV.h"
#include <cmath>
#include "TxCar_EV_private.h"
#include "rtwtypes.h"

extern "C" {

#include "rt_nonfinite.h"
}

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void TxCar_EV::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  TxCar_EV_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  TxCar_EV_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  TxCar_EV_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  TxCar_EV_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Model step function */
void TxCar_EV::step() {
  /* local block i/o variables */
  real_T rtb_Add;
  real_T rtb_mot_trq_Nm;
  real_T rtb_batt_soc_null;
  real_T rtb_wheel_angle[4];
  real_T rtb_AxlTrq[4];
  real_T rtb_ABSFlag_idx_0;
  real_T rtb_ABSFlag_idx_1;
  real_T rtb_ABSFlag_idx_2;
  real_T rtb_ABSFlag_idx_3;
  real_T rtb_DiffPrsCmd_idx_0;
  real_T rtb_DiffPrsCmd_idx_1;
  real_T rtb_DiffPrsCmd_idx_2;
  real_T rtb_DiffPrsCmd_idx_3;
  real_T rtb_MultiportSwitch1;
  real_T rtb_Saturation_idx_0;
  real_T rtb_Saturation_idx_1;
  real_T rtb_Saturation_idx_2;
  real_T rtb_Saturation_idx_3;
  real_T rtb_TrqCmd;
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
  if (rtmIsMajorTimeStep((&TxCar_EV_M))) {
    /* set solver stop time */
    if (!((&TxCar_EV_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&TxCar_EV_M)->solverInfo,
                            (((&TxCar_EV_M)->Timing.clockTickH0 + 1) * (&TxCar_EV_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(&(&TxCar_EV_M)->solverInfo,
                            (((&TxCar_EV_M)->Timing.clockTick0 + 1) * (&TxCar_EV_M)->Timing.stepSize0 +
                             (&TxCar_EV_M)->Timing.clockTickH0 * (&TxCar_EV_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&TxCar_EV_M))) {
    (&TxCar_EV_M)->Timing.t[0] = rtsiGetT(&(&TxCar_EV_M)->solverInfo);
  }

  if (rtmIsMajorTimeStep((&TxCar_EV_M))) {
    /* Constant: '<S5>/Constant3' incorporates:
     *  BusCreator generated from: '<S2>/Chassis'
     */
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[0] = TxCar_EV_P.Constant3_Value;

    /* Constant: '<S5>/Constant4' incorporates:
     *  BusCreator generated from: '<S2>/Chassis'
     */
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[1] = TxCar_EV_P.Constant4_Value;

    /* Constant: '<S5>/Constant5' incorporates:
     *  BusCreator generated from: '<S2>/Chassis'
     */
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[2] = TxCar_EV_P.Constant5_Value_m;

    /* Switch: '<S24>/Switch' incorporates:
     *  Constant: '<S25>/Constant'
     *  Inport generated from: '<Root>/Friction'
     *  RelationalOperator: '<S25>/Compare'
     */
    if (TxCar_EV_U.Friction.mu_FL <= TxCar_EV_P.CompareToConstant_const) {
      /* BusCreator generated from: '<S2>/Chassis' incorporates:
       *  Constant: '<S24>/const_mu'
       */
      TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FL = TxCar_EV_P.const_mu_Value[0];
      TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FR = TxCar_EV_P.const_mu_Value[1];
      TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RL = TxCar_EV_P.const_mu_Value[2];
      TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RR = TxCar_EV_P.const_mu_Value[3];
    } else {
      /* BusCreator generated from: '<S2>/Chassis' */
      TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FL = TxCar_EV_U.Friction.mu_FL;
      TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FR = TxCar_EV_U.Friction.mu_FR;
      TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RL = TxCar_EV_U.Friction.mu_RL;
      TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RR = TxCar_EV_U.Friction.mu_RR;
    }

    /* End of Switch: '<S24>/Switch' */

    /* BusCreator generated from: '<S2>/Chassis' incorporates:
     *  Inport generated from: '<Root>/Height'
     */
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.z = TxCar_EV_U.Height;

    /* BusCreator generated from: '<S2>/Chassis' incorporates:
     *  Memory generated from: '<S8>/Memory'
     */
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[0] =
        TxCar_EV_DW.Memory_1_PreviousInput[0];
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[0] =
        TxCar_EV_DW.Memory_2_PreviousInput[0];
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[0] =
        TxCar_EV_DW.Memory_3_PreviousInput[0];
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[1] =
        TxCar_EV_DW.Memory_1_PreviousInput[1];
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[1] =
        TxCar_EV_DW.Memory_2_PreviousInput[1];
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[1] =
        TxCar_EV_DW.Memory_3_PreviousInput[1];
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[2] =
        TxCar_EV_DW.Memory_1_PreviousInput[2];
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[2] =
        TxCar_EV_DW.Memory_2_PreviousInput[2];
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[2] =
        TxCar_EV_DW.Memory_3_PreviousInput[2];
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[3] =
        TxCar_EV_DW.Memory_1_PreviousInput[3];
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[3] =
        TxCar_EV_DW.Memory_2_PreviousInput[3];
    TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[3] =
        TxCar_EV_DW.Memory_3_PreviousInput[3];

    /* S-Function (chassis_sfunc): '<S2>/Chassis' incorporates:
     *  Outport generated from: '<Root>/Body'
     *  Outport generated from: '<Root>/Susp'
     *  Outport generated from: '<Root>/WheelsOut'
     */
    chassis_sfunc_Outputs_wrapper_cgen(&TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1,
                                       &TxCar_EV_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1,
                                       &TxCar_EV_Y.Body, &TxCar_EV_Y.WheelsOut, &TxCar_EV_Y.Susp, TxCar_EV_P.Chassis_P1,
                                       21, TxCar_EV_P.Chassis_P2, 22);

    /* Gain: '<S44>/Gain' incorporates:
     *  Outport generated from: '<Root>/Body'
     */
    TxCar_EV_B.ax = TxCar_EV_P.Gain_Gain * TxCar_EV_Y.Body.BdyFrm.Cg.Acc.ax;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' incorporates:
     *  Memory generated from: '<S28>/Memory'
     */
    TxCar_EV_Y.PowerBus_Out_battery_info_batt_soc_null = TxCar_EV_DW.Memory_5_PreviousInput;
  }

  /* BusCreator: '<S44>/Bus Creator1' incorporates:
   *  Constant: '<S47>/Constant1'
   *  Inport generated from: '<Root>/DriverIn'
   *  Integrator: '<S17>/Integrator1'
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/PowerBus_Outport_2'
   *  Outport generated from: '<Root>/WheelsOut'
   */
  TxCar_EV_B.BusCreator1.acc_feedback_m_s2 = TxCar_EV_B.ax;
  TxCar_EV_B.BusCreator1.eng_spd_rad_s = TxCar_EV_X.Integrator1_CSTATE;
  TxCar_EV_B.BusCreator1.batt_soc_0_1 = TxCar_EV_Y.PowerBus_Out_battery_info_batt_soc_null;
  TxCar_EV_B.BusCreator1.diff_trq_req_Nm = TxCar_EV_P.Constant1_Value_k;
  TxCar_EV_B.BusCreator1.slip_ratio[0] = TxCar_EV_Y.WheelsOut.TireFrame.Kappa[0];
  TxCar_EV_B.BusCreator1.slip_ratio[1] = TxCar_EV_Y.WheelsOut.TireFrame.Kappa[1];
  TxCar_EV_B.BusCreator1.slip_ratio[2] = TxCar_EV_Y.WheelsOut.TireFrame.Kappa[2];
  TxCar_EV_B.BusCreator1.slip_ratio[3] = TxCar_EV_Y.WheelsOut.TireFrame.Kappa[3];
  TxCar_EV_B.BusCreator1.veh_speed_vx = TxCar_EV_Y.Body.BdyFrm.Cg.Vel.xdot;
  TxCar_EV_B.BusCreator1.driver_input = TxCar_EV_U.DriverIn;
  if (rtmIsMajorTimeStep((&TxCar_EV_M))) {
    /* S-Function (ecu_sfunction): '<S43>/S-Function' */
    ecu_sfunction_Outputs_wrapper_cgen(&TxCar_EV_B.BusCreator1, &TxCar_EV_B.SFunction, TxCar_EV_P.SFunction_P1, 21,
                                       TxCar_EV_P.SFunction_P2, 22);

    /* Gain: '<S19>/Gain5' */
    TxCar_EV_B.acc_cmd = TxCar_EV_P.Gain5_Gain * TxCar_EV_B.SFunction.throttle_01;

    /* Gain: '<S19>/Gain1' incorporates:
     *  Constant: '<S19>/Constant1'
     */
    TxCar_EV_B.brake_cmd_0_1 = TxCar_EV_P.Gain1_Gain * TxCar_EV_P.Constant1_Value_f;

    /* Gain: '<S19>/Gain2' incorporates:
     *  Outport generated from: '<Root>/WheelsOut'
     */
    TxCar_EV_B.wheel_spd_rad_s[0] = TxCar_EV_P.Gain2_Gain * TxCar_EV_Y.WheelsOut.TireFrame.Omega[0];
    TxCar_EV_B.wheel_spd_rad_s[1] = TxCar_EV_P.Gain2_Gain * TxCar_EV_Y.WheelsOut.TireFrame.Omega[1];
    TxCar_EV_B.wheel_spd_rad_s[2] = TxCar_EV_P.Gain2_Gain * TxCar_EV_Y.WheelsOut.TireFrame.Omega[2];
    TxCar_EV_B.wheel_spd_rad_s[3] = TxCar_EV_P.Gain2_Gain * TxCar_EV_Y.WheelsOut.TireFrame.Omega[3];

    /* Gain: '<S19>/Gain3' incorporates:
     *  Constant: '<S19>/Constant2'
     */
    TxCar_EV_B.abs_flag = TxCar_EV_P.Gain3_Gain * TxCar_EV_P.Constant2_Value;

    /* Memory generated from: '<S28>/Memory' */
    rtb_eng_trqNm = TxCar_EV_DW.Memory_1_PreviousInput_i;

    /* Gain: '<S19>/Gain4' incorporates:
     *  Memory generated from: '<S28>/Memory'
     */
    TxCar_EV_B.engine_trq_Nm = TxCar_EV_P.Gain4_Gain * TxCar_EV_DW.Memory_1_PreviousInput_i;

    /* SignalConversion generated from: '<S47>/Vector Concatenate2' incorporates:
     *  Constant: '<S47>/Constant'
     */
    rtb_DiffPrsCmd_idx_0 = TxCar_EV_P.Constant_Value;

    /* SignalConversion generated from: '<S47>/Vector Concatenate2' incorporates:
     *  Constant: '<S47>/Constant'
     */
    rtb_DiffPrsCmd_idx_1 = TxCar_EV_P.Constant_Value;

    /* SignalConversion generated from: '<S47>/Vector Concatenate2' incorporates:
     *  Constant: '<S47>/Constant'
     */
    rtb_DiffPrsCmd_idx_2 = TxCar_EV_P.Constant_Value;

    /* SignalConversion generated from: '<S47>/Vector Concatenate2' incorporates:
     *  Constant: '<S47>/Constant'
     */
    rtb_DiffPrsCmd_idx_3 = TxCar_EV_P.Constant_Value;

    /* DataTypeConversion: '<S19>/Data Type Conversion' incorporates:
     *  Constant: '<S47>/Constant'
     */
    TxCar_EV_B.diff_prs_cmd[0] = (TxCar_EV_P.Constant_Value != 0.0);
    TxCar_EV_B.diff_prs_cmd[1] = (TxCar_EV_P.Constant_Value != 0.0);
    TxCar_EV_B.diff_prs_cmd[2] = (TxCar_EV_P.Constant_Value != 0.0);
    TxCar_EV_B.diff_prs_cmd[3] = (TxCar_EV_P.Constant_Value != 0.0);

    /* Memory generated from: '<S28>/Memory' */
    rtb_mot_front_trqNm = TxCar_EV_DW.Memory_2_PreviousInput_p;

    /* Gain: '<S19>/Gain6' incorporates:
     *  Memory generated from: '<S28>/Memory'
     */
    TxCar_EV_B.mot_front_trq_Nm = TxCar_EV_P.Gain6_Gain * TxCar_EV_DW.Memory_2_PreviousInput_p;

    /* Memory generated from: '<S28>/Memory' */
    rtb_mot_rear_trqNm = TxCar_EV_DW.Memory_3_PreviousInput_k;

    /* Gain: '<S19>/Gain7' incorporates:
     *  Memory generated from: '<S28>/Memory'
     */
    TxCar_EV_B.mot_rear_trq_Nm = TxCar_EV_P.Gain7_Gain * TxCar_EV_DW.Memory_3_PreviousInput_k;
  }

  /* MultiPortSwitch: '<S20>/Multiport Switch' incorporates:
   *  Inport generated from: '<Root>/DriverIn'
   */
  switch (TxCar_EV_U.DriverIn.gear_cmd__0N1D2R3P) {
    case 0:
      /* BusCreator: '<S19>/Bus Creator' incorporates:
       *  Constant: '<S20>/N'
       */
      TxCar_EV_B.driveline.gear_cmd_driver = TxCar_EV_P.N_Value;
      break;

    case 1:
      /* BusCreator: '<S19>/Bus Creator' incorporates:
       *  Constant: '<S20>/D'
       */
      TxCar_EV_B.driveline.gear_cmd_driver = TxCar_EV_P.D_Value;
      break;

    case 2:
      /* BusCreator: '<S19>/Bus Creator' incorporates:
       *  Constant: '<S20>/R'
       */
      TxCar_EV_B.driveline.gear_cmd_driver = TxCar_EV_P.R_Value;
      break;

    default:
      /* BusCreator: '<S19>/Bus Creator' incorporates:
       *  Constant: '<S20>/P'
       */
      TxCar_EV_B.driveline.gear_cmd_driver = TxCar_EV_P.P_Value;
      break;
  }

  /* End of MultiPortSwitch: '<S20>/Multiport Switch' */

  /* BusCreator: '<S19>/Bus Creator' */
  TxCar_EV_B.driveline.acc_cmd = TxCar_EV_B.acc_cmd;
  TxCar_EV_B.driveline.brake_cmd_0_1 = TxCar_EV_B.brake_cmd_0_1;
  TxCar_EV_B.driveline.abs_flag = TxCar_EV_B.abs_flag;
  TxCar_EV_B.driveline.engine_trq_Nm = TxCar_EV_B.engine_trq_Nm;
  TxCar_EV_B.driveline.wheel_spd_rad_s[0] = TxCar_EV_B.wheel_spd_rad_s[0];
  TxCar_EV_B.driveline.diff_prs_cmd[0] = TxCar_EV_B.diff_prs_cmd[0];
  TxCar_EV_B.driveline.wheel_spd_rad_s[1] = TxCar_EV_B.wheel_spd_rad_s[1];
  TxCar_EV_B.driveline.diff_prs_cmd[1] = TxCar_EV_B.diff_prs_cmd[1];
  TxCar_EV_B.driveline.wheel_spd_rad_s[2] = TxCar_EV_B.wheel_spd_rad_s[2];
  TxCar_EV_B.driveline.diff_prs_cmd[2] = TxCar_EV_B.diff_prs_cmd[2];
  TxCar_EV_B.driveline.wheel_spd_rad_s[3] = TxCar_EV_B.wheel_spd_rad_s[3];
  TxCar_EV_B.driveline.diff_prs_cmd[3] = TxCar_EV_B.diff_prs_cmd[3];
  TxCar_EV_B.driveline.mot_front_trq_Nm = TxCar_EV_B.mot_front_trq_Nm;
  TxCar_EV_B.driveline.mot_rear_trq_Nm = TxCar_EV_B.mot_rear_trq_Nm;
  if (rtmIsMajorTimeStep((&TxCar_EV_M))) {
    /* S-Function (driveline_ev_front_sfunction): '<S15>/S-Function' */
    driveline_ev_front_sfunction_Outputs_wrapper_cgen(&TxCar_EV_B.driveline, &TxCar_EV_B.SFunction_m,
                                                      TxCar_EV_P.SFunction_P1_c, 21, TxCar_EV_P.SFunction_P2_b, 22);

    /* S-Function (driveline_ev_rear_sfunction): '<S16>/S-Function' */
    driveline_ev_rear_sfunction_Outputs_wrapper_cgen(&TxCar_EV_B.driveline, &TxCar_EV_B.SFunction_o,
                                                     TxCar_EV_P.SFunction_P1_l, 21, TxCar_EV_P.SFunction_P2_m, 22);

    /* S-Function (cartype_sfunction): '<S1>/S-Function' incorporates:
     *  Constant: '<S1>/Zero'
     */
    cartype_sfunction_Outputs_wrapper_cgen(&TxCar_EV_P.Zero_Value, &TxCar_EV_B.SFunction_e, TxCar_EV_P.SFunction_P1_lo,
                                           21, TxCar_EV_P.SFunction_P2_o, 22);

    /* MultiPortSwitch: '<S11>/Multiport Switch' */
    switch (static_cast<int32_T>(TxCar_EV_B.SFunction_e.drivetype)) {
      case 1:
        /* MultiPortSwitch: '<S11>/Multiport Switch' */
        TxCar_EV_B.MultiportSwitch = TxCar_EV_B.SFunction_m;
        break;

      case 2:
        /* MultiPortSwitch: '<S11>/Multiport Switch' */
        TxCar_EV_B.MultiportSwitch = TxCar_EV_B.SFunction_o;
        break;

      default:
        /* MultiPortSwitch: '<S11>/Multiport Switch' incorporates:
         *  BusCreator: '<S18>/Bus Creator2'
         *  Constant: '<S18>/Constant'
         *  Constant: '<S18>/Constant1'
         *  Constant: '<S18>/Constant2'
         *  SignalConversion generated from: '<S18>/Bus Creator2'
         */
        TxCar_EV_B.MultiportSwitch.gear_engaged = TxCar_EV_P.Constant_Value_n;
        TxCar_EV_B.MultiportSwitch.eng_spd_rad_s = TxCar_EV_P.Constant1_Value_o;
        TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[0] = TxCar_EV_B.SFunction_m.wheel_drive_trq_Nm[0];
        TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[2] = TxCar_EV_B.SFunction_o.wheel_drive_trq_Nm[2];
        TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[1] = TxCar_EV_B.SFunction_m.wheel_drive_trq_Nm[1];
        TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[3] = TxCar_EV_B.SFunction_o.wheel_drive_trq_Nm[3];
        TxCar_EV_B.MultiportSwitch.trans_out_shaft_spd_rad_s = TxCar_EV_P.Constant2_Value_d;
        TxCar_EV_B.MultiportSwitch.front_mot_spd_rad_s = TxCar_EV_B.SFunction_m.front_mot_spd_rad_s;
        TxCar_EV_B.MultiportSwitch.rear_mot_spd_rad_s = TxCar_EV_B.SFunction_o.rear_mot_spd_rad_s;
        break;
    }

    /* End of MultiPortSwitch: '<S11>/Multiport Switch' */
  }

  /* Product: '<S17>/Divide' incorporates:
   *  Constant: '<S17>/Constant'
   *  Integrator: '<S17>/Integrator1'
   *  Sum: '<S17>/Sum'
   */
  TxCar_EV_B.Divide =
      (TxCar_EV_B.MultiportSwitch.eng_spd_rad_s - TxCar_EV_X.Integrator1_CSTATE) * TxCar_EV_P.EngSpdAct_wc;
  if (rtmIsMajorTimeStep((&TxCar_EV_M))) {
    real_T rtb_Product;
    real_T u0;

    /* Saturate: '<S11>/Saturation' */
    if (TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[0] > TxCar_EV_P.Saturation_UpperSat) {
      rtb_Saturation_idx_0 = TxCar_EV_P.Saturation_UpperSat;
    } else if (TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[0] < TxCar_EV_P.Saturation_LowerSat) {
      rtb_Saturation_idx_0 = TxCar_EV_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_0 = TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[0];
    }

    if (TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[1] > TxCar_EV_P.Saturation_UpperSat) {
      rtb_Saturation_idx_1 = TxCar_EV_P.Saturation_UpperSat;
    } else if (TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[1] < TxCar_EV_P.Saturation_LowerSat) {
      rtb_Saturation_idx_1 = TxCar_EV_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_1 = TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[1];
    }

    if (TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[2] > TxCar_EV_P.Saturation_UpperSat) {
      rtb_Saturation_idx_2 = TxCar_EV_P.Saturation_UpperSat;
    } else if (TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[2] < TxCar_EV_P.Saturation_LowerSat) {
      rtb_Saturation_idx_2 = TxCar_EV_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_2 = TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[2];
    }

    if (TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[3] > TxCar_EV_P.Saturation_UpperSat) {
      rtb_Saturation_idx_3 = TxCar_EV_P.Saturation_UpperSat;
    } else if (TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[3] < TxCar_EV_P.Saturation_LowerSat) {
      rtb_Saturation_idx_3 = TxCar_EV_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_3 = TxCar_EV_B.MultiportSwitch.wheel_drive_trq_Nm[3];
    }

    /* End of Saturate: '<S11>/Saturation' */

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' incorporates:
     *  Gain: '<S42>/Gain2'
     */
    TxCar_EV_Y.SoftEcu_Out_Brake_BrkPrsCmd01 = TxCar_EV_P.Gain2_Gain_l * TxCar_EV_B.SFunction.brake_pressure_0_1;

    /* S-Function (brakehydraulic_sfunc): '<S14>/S-Function' incorporates:
     *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
     *  Outport generated from: '<Root>/SoftEcu_Outport_4'
     */
    brakehydraulic_sfunc_Outputs_wrapper_cgen(&TxCar_EV_Y.SoftEcu_Out_Brake_BrkPrsCmd01,
                                              &TxCar_EV_Y.DriveLineBus_Out_BrkPrs[0], TxCar_EV_P.SFunction_P1_f, 21,
                                              TxCar_EV_P.SFunction_P2_i, 22);

    /* Memory generated from: '<S28>/Memory' */
    rtb_batt_volt_V = TxCar_EV_DW.Memory_4_PreviousInput;

    /* Memory generated from: '<S28>/Memory' */
    rtb_current = TxCar_EV_DW.Memory_6_PreviousInput;

    /* Gain: '<S45>/Gain' */
    rtb_TrqCmd = TxCar_EV_P.Gain_Gain_m * TxCar_EV_B.SFunction.pt_trq_cmd_Nm;

    /* MultiPortSwitch: '<S37>/Multiport Switch1' incorporates:
     *  Constant: '<S37>/Constant1'
     */
    if (static_cast<int32_T>(TxCar_EV_B.SFunction_e.proplutiontype) == 1) {
      rtb_MultiportSwitch1 = TxCar_EV_P.Constant1_Value;
    } else {
      rtb_MultiportSwitch1 = rtb_TrqCmd;
    }

    /* End of MultiPortSwitch: '<S37>/Multiport Switch1' */

    /* Product: '<S30>/Product' */
    rtb_Product = rtb_MultiportSwitch1 * TxCar_EV_B.SFunction_e.parsingbck_01;

    /* MultiPortSwitch: '<S29>/Multiport Switch3' */
    switch (static_cast<int32_T>(TxCar_EV_B.SFunction_e.drivetype)) {
      case 1:
        /* BusCreator generated from: '<S32>/S-Function' */
        TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.trq_cmd_Nm = rtb_MultiportSwitch1;
        break;

      case 2:
        /* BusCreator generated from: '<S32>/S-Function' incorporates:
         *  Constant: '<S29>/Zero_If_RWD'
         */
        TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.trq_cmd_Nm =
            TxCar_EV_P.Zero_If_RWD_Value;
        break;

      default:
        /* BusCreator generated from: '<S32>/S-Function' */
        TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.trq_cmd_Nm = rtb_Product;
        break;
    }

    /* End of MultiPortSwitch: '<S29>/Multiport Switch3' */

    /* Gain: '<S29>/Gain3' incorporates:
     *  Memory: '<S29>/Memory1'
     */
    u0 = TxCar_EV_P.Gain3_Gain_p * TxCar_EV_DW.Memory1_PreviousInput;

    /* Saturate: '<S29>/Saturation' */
    if (u0 > TxCar_EV_P.Saturation_UpperSat_o) {
      /* BusCreator generated from: '<S31>/S-Function' */
      TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_curr_A =
          TxCar_EV_P.Saturation_UpperSat_o;
    } else if (u0 < TxCar_EV_P.Saturation_LowerSat_o) {
      /* BusCreator generated from: '<S31>/S-Function' */
      TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_curr_A =
          TxCar_EV_P.Saturation_LowerSat_o;
    } else {
      /* BusCreator generated from: '<S31>/S-Function' */
      TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_curr_A = u0;
    }

    /* End of Saturate: '<S29>/Saturation' */

    /* BusCreator generated from: '<S31>/S-Function' incorporates:
     *  Constant: '<S29>/Constant'
     */
    TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p.batt_temp_K = TxCar_EV_P.Constant_Value_nf;

    /* S-Function (battery_sfunction): '<S31>/S-Function' */
    battery_sfunction_Outputs_wrapper_cgen(&TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p,
                                           &TxCar_EV_B.SFunction_k, TxCar_EV_P.SFunction_P1_m, 21,
                                           TxCar_EV_P.SFunction_P2_g, 22);

    /* BusCreator generated from: '<S32>/S-Function' incorporates:
     *  Constant: '<S36>/4WD'
     *  Constant: '<S36>/FD'
     *  Gain: '<S36>/Gain2'
     *  Logic: '<S36>/Logical Operator'
     *  Product: '<S36>/Product1'
     *  RelationalOperator: '<S36>/Relational Operator'
     *  RelationalOperator: '<S36>/Relational Operator1'
     */
    TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.mot_spd_RPM =
        static_cast<real_T>((TxCar_EV_B.SFunction_e.drivetype == TxCar_EV_P.FD_Value) ||
                            (TxCar_EV_B.SFunction_e.drivetype == TxCar_EV_P.uWD_Value)) *
        (TxCar_EV_P.Gain2_Gain_c * TxCar_EV_B.MultiportSwitch.front_mot_spd_rad_s);
    TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.batt_volt_V =
        TxCar_EV_B.SFunction_k.batt_volt_V;

    /* S-Function (front_motor_sfunction): '<S32>/S-Function' */
    front_motor_sfunction_Outputs_wrapper_cgen(&TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1,
                                               &TxCar_EV_B.SFunction_h, TxCar_EV_P.SFunction_P1_h, 21,
                                               TxCar_EV_P.SFunction_P2_by, 22);

    /* MultiPortSwitch: '<S29>/Multiport Switch4' */
    switch (static_cast<int32_T>(TxCar_EV_B.SFunction_e.drivetype)) {
      case 1:
        /* BusCreator generated from: '<S33>/S-Function' incorporates:
         *  Constant: '<S29>/Zero_FWD'
         */
        TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.trq_cmd_Nm = TxCar_EV_P.Zero_FWD_Value;
        break;

      case 2:
        /* MultiPortSwitch: '<S39>/Multiport Switch5' */
        if (static_cast<int32_T>(TxCar_EV_B.SFunction_e.proplutiontype) == 1) {
          /* BusCreator generated from: '<S33>/S-Function' incorporates:
           *  Constant: '<S39>/Constant5'
           */
          TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.trq_cmd_Nm =
              TxCar_EV_P.Constant5_Value;
        } else {
          /* BusCreator generated from: '<S33>/S-Function' */
          TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.trq_cmd_Nm = rtb_TrqCmd;
        }

        /* End of MultiPortSwitch: '<S39>/Multiport Switch5' */
        break;

      default:
        /* BusCreator generated from: '<S33>/S-Function' incorporates:
         *  Sum: '<S30>/Add1'
         */
        TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.trq_cmd_Nm =
            rtb_MultiportSwitch1 - rtb_Product;
        break;
    }

    /* End of MultiPortSwitch: '<S29>/Multiport Switch4' */

    /* BusCreator generated from: '<S33>/S-Function' incorporates:
     *  Constant: '<S38>/4WD'
     *  Constant: '<S38>/RD'
     *  Gain: '<S38>/Gain4'
     *  Logic: '<S38>/Logical Operator'
     *  Product: '<S38>/Product2'
     *  RelationalOperator: '<S38>/Relational Operator'
     *  RelationalOperator: '<S38>/Relational Operator2'
     */
    TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.mot_spd_RPM =
        static_cast<real_T>((TxCar_EV_B.SFunction_e.drivetype == TxCar_EV_P.RD_Value) ||
                            (TxCar_EV_B.SFunction_e.drivetype == TxCar_EV_P.uWD_Value_c)) *
        TxCar_EV_B.MultiportSwitch.rear_mot_spd_rad_s * TxCar_EV_P.Gain4_Gain_p;
    TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m.batt_volt_V =
        TxCar_EV_B.SFunction_k.batt_volt_V;

    /* S-Function (rear_motor_sfunction): '<S33>/S-Function' */
    rear_motor_sfunction_Outputs_wrapper_cgen(&TxCar_EV_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m,
                                              &TxCar_EV_B.SFunction_ep, TxCar_EV_P.SFunction_P1_g, 21,
                                              TxCar_EV_P.SFunction_P2_ox, 22);

    /* Sum: '<S29>/Add' */
    rtb_Add = TxCar_EV_B.SFunction_h.batt_curr_A + TxCar_EV_B.SFunction_ep.batt_curr_A;

    /* SignalConversion generated from: '<S29>/Bus Selector1' */
    rtb_mot_trq_Nm = TxCar_EV_B.SFunction_h.mot_trq_Nm;

    /* SignalConversion generated from: '<S29>/Bus Selector2' */
    rtb_batt_soc_null = TxCar_EV_B.SFunction_k.batt_soc_null;
  }

  /* BusCreator generated from: '<S48>/Steer_Mapped' incorporates:
   *  Inport generated from: '<Root>/DriverIn'
   */
  TxCar_EV_B.BusConversion_InsertedFor_Steer_Mapped_at_inport_0_BusCreator1 = TxCar_EV_U.DriverIn;
  if (rtmIsMajorTimeStep((&TxCar_EV_M))) {
    /* S-Function (dynamic_steer_sfunc): '<S48>/Steer_Mapped' incorporates:
     *  Outport generated from: '<Root>/Body'
     *  Outport generated from: '<Root>/Bus Element Out2'
     *  Outport generated from: '<Root>/Susp'
     *  Outport generated from: '<Root>/WheelsOut'
     */
    dynamic_steer_sfunc_Outputs_wrapper_cgen(&TxCar_EV_B.BusConversion_InsertedFor_Steer_Mapped_at_inport_0_BusCreator1,
                                             &TxCar_EV_Y.Body, &TxCar_EV_Y.Susp, &TxCar_EV_Y.WheelsOut,
                                             &TxCar_EV_Y.SteerBus_Out, TxCar_EV_P.Steer_Mapped_P1, 21,
                                             TxCar_EV_P.Steer_Mapped_P2, 22);

    /* DeadZone: '<S40>/Dead Zone' incorporates:
     *  BusCreator: '<S44>/Bus Creator1'
     *  Outport generated from: '<Root>/Body'
     */
    if (TxCar_EV_Y.Body.BdyFrm.Cg.Vel.xdot > TxCar_EV_P.DeadZone_End) {
      rtb_MultiportSwitch1 = TxCar_EV_Y.Body.BdyFrm.Cg.Vel.xdot - TxCar_EV_P.DeadZone_End;
    } else if (TxCar_EV_Y.Body.BdyFrm.Cg.Vel.xdot >= TxCar_EV_P.DeadZone_Start) {
      rtb_MultiportSwitch1 = 0.0;
    } else {
      rtb_MultiportSwitch1 = TxCar_EV_Y.Body.BdyFrm.Cg.Vel.xdot - TxCar_EV_P.DeadZone_Start;
    }

    /* Signum: '<S40>/Sign' incorporates:
     *  DeadZone: '<S40>/Dead Zone'
     */
    if (std::isnan(rtb_MultiportSwitch1)) {
      rtb_MultiportSwitch1 = (rtNaN);
    } else if (rtb_MultiportSwitch1 < 0.0) {
      rtb_MultiportSwitch1 = -1.0;
    } else {
      rtb_MultiportSwitch1 = (rtb_MultiportSwitch1 > 0.0);
    }

    /* Product: '<S40>/Product' incorporates:
     *  Constant: '<S40>/AxleTrqOffset'
     *  Signum: '<S40>/Sign'
     */
    rtb_MultiportSwitch1 *= TxCar_EV_P.AxleTrqOffset_Value;

    /* Gain: '<S45>/Gain1' */
    rtb_throttle_01 = TxCar_EV_P.Gain1_Gain_f * TxCar_EV_B.SFunction.throttle_01;

    /* Saturate: '<S8>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_EV_Y.SteerBus_Out.wheel_angle[0] > TxCar_EV_P.Saturation_UpperSat_k) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[0] = TxCar_EV_P.Saturation_UpperSat_k;
    } else if (TxCar_EV_Y.SteerBus_Out.wheel_angle[0] < TxCar_EV_P.Saturation_LowerSat_l) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[0] = TxCar_EV_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[0] = TxCar_EV_Y.SteerBus_Out.wheel_angle[0];
    }

    /* Sum: '<S40>/Add' */
    rtb_AxlTrq[0] = rtb_MultiportSwitch1 + rtb_Saturation_idx_0;

    /* Gain: '<S42>/Gain' */
    rtb_brake_pressure_Pa_idx_0 = TxCar_EV_P.Gain_Gain_a * TxCar_EV_B.SFunction.brake_pressure_Pa[0];

    /* Gain: '<S42>/Gain1' */
    rtb_ABSFlag_idx_0 = TxCar_EV_P.Gain1_Gain_o * TxCar_EV_B.SFunction.abs_flag[0];

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
     *  Sum: '<S40>/Add'
     */
    TxCar_EV_Y.DriveLineBus_Out_AxlTrq[0] = rtb_Saturation_idx_0;

    /* Saturate: '<S8>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_EV_Y.SteerBus_Out.wheel_angle[1] > TxCar_EV_P.Saturation_UpperSat_k) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[1] = TxCar_EV_P.Saturation_UpperSat_k;
    } else if (TxCar_EV_Y.SteerBus_Out.wheel_angle[1] < TxCar_EV_P.Saturation_LowerSat_l) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[1] = TxCar_EV_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[1] = TxCar_EV_Y.SteerBus_Out.wheel_angle[1];
    }

    /* Sum: '<S40>/Add' */
    rtb_AxlTrq[1] = rtb_MultiportSwitch1 + rtb_Saturation_idx_1;

    /* Gain: '<S42>/Gain' */
    rtb_brake_pressure_Pa_idx_1 = TxCar_EV_P.Gain_Gain_a * TxCar_EV_B.SFunction.brake_pressure_Pa[1];

    /* Gain: '<S42>/Gain1' */
    rtb_ABSFlag_idx_1 = TxCar_EV_P.Gain1_Gain_o * TxCar_EV_B.SFunction.abs_flag[1];

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
     *  Sum: '<S40>/Add'
     */
    TxCar_EV_Y.DriveLineBus_Out_AxlTrq[1] = rtb_Saturation_idx_1;

    /* Saturate: '<S8>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_EV_Y.SteerBus_Out.wheel_angle[2] > TxCar_EV_P.Saturation_UpperSat_k) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[2] = TxCar_EV_P.Saturation_UpperSat_k;
    } else if (TxCar_EV_Y.SteerBus_Out.wheel_angle[2] < TxCar_EV_P.Saturation_LowerSat_l) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[2] = TxCar_EV_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[2] = TxCar_EV_Y.SteerBus_Out.wheel_angle[2];
    }

    /* Sum: '<S40>/Add' */
    rtb_AxlTrq[2] = rtb_MultiportSwitch1 + rtb_Saturation_idx_2;

    /* Gain: '<S42>/Gain' */
    rtb_brake_pressure_Pa_idx_2 = TxCar_EV_P.Gain_Gain_a * TxCar_EV_B.SFunction.brake_pressure_Pa[2];

    /* Gain: '<S42>/Gain1' */
    rtb_ABSFlag_idx_2 = TxCar_EV_P.Gain1_Gain_o * TxCar_EV_B.SFunction.abs_flag[2];

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
     *  Sum: '<S40>/Add'
     */
    TxCar_EV_Y.DriveLineBus_Out_AxlTrq[2] = rtb_Saturation_idx_2;

    /* Saturate: '<S8>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_EV_Y.SteerBus_Out.wheel_angle[3] > TxCar_EV_P.Saturation_UpperSat_k) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[3] = TxCar_EV_P.Saturation_UpperSat_k;
    } else if (TxCar_EV_Y.SteerBus_Out.wheel_angle[3] < TxCar_EV_P.Saturation_LowerSat_l) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[3] = TxCar_EV_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[3] = TxCar_EV_Y.SteerBus_Out.wheel_angle[3];
    }

    /* Sum: '<S40>/Add' */
    rtb_AxlTrq[3] = rtb_MultiportSwitch1 + rtb_Saturation_idx_3;

    /* Gain: '<S42>/Gain' */
    rtb_brake_pressure_Pa_idx_3 = TxCar_EV_P.Gain_Gain_a * TxCar_EV_B.SFunction.brake_pressure_Pa[3];

    /* Gain: '<S42>/Gain1' */
    rtb_ABSFlag_idx_3 = TxCar_EV_P.Gain1_Gain_o * TxCar_EV_B.SFunction.abs_flag[3];

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
     *  Sum: '<S40>/Add'
     */
    TxCar_EV_Y.DriveLineBus_Out_AxlTrq[3] = rtb_Saturation_idx_3;
  }

  /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
   *  Integrator: '<S17>/Integrator1'
   */
  TxCar_EV_Y.DriveLineBus_Out_EngSpd = TxCar_EV_X.Integrator1_CSTATE;
  if (rtmIsMajorTimeStep((&TxCar_EV_M))) {
    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_EV_Y.DriveLineBus_Out_PropShftSpd = TxCar_EV_B.MultiportSwitch.trans_out_shaft_spd_rad_s;

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_EV_Y.DriveLineBus_Out_Gear = TxCar_EV_B.MultiportSwitch.gear_engaged;

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_EV_Y.DriveLineBus_Out_FrontMotSpd = TxCar_EV_B.MultiportSwitch.front_mot_spd_rad_s;

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_EV_Y.DriveLineBus_Out_RearMotSpd = TxCar_EV_B.MultiportSwitch.rear_mot_spd_rad_s;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_EV_Y.PowerBus_Out_eng_trqNm = rtb_eng_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_EV_Y.PowerBus_Out_mot_front_trqNm = rtb_mot_front_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_EV_Y.PowerBus_Out_mot_rear_trqNm = rtb_mot_rear_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_EV_Y.PowerBus_Out_battery_info_batt_volt_V = rtb_batt_volt_V;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_EV_Y.PowerBus_Out_current = rtb_current;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_ECU_TrqCmd = rtb_TrqCmd;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_ECU_throttle_01 = rtb_throttle_01;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' incorporates:
     *  Constant: '<S42>/Constant'
     */
    TxCar_EV_Y.SoftEcu_Out_Brake_TCSFlag = TxCar_EV_P.Constant_Value_p;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Brake_ABSFlag[0] = rtb_ABSFlag_idx_0;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Brake_brake_pressure_Pa[0] = rtb_brake_pressure_Pa_idx_0;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Diff_DiffPrsCmd[0] = rtb_DiffPrsCmd_idx_0;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Brake_ABSFlag[1] = rtb_ABSFlag_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Brake_brake_pressure_Pa[1] = rtb_brake_pressure_Pa_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Diff_DiffPrsCmd[1] = rtb_DiffPrsCmd_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Brake_ABSFlag[2] = rtb_ABSFlag_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Brake_brake_pressure_Pa[2] = rtb_brake_pressure_Pa_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Diff_DiffPrsCmd[2] = rtb_DiffPrsCmd_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Brake_ABSFlag[3] = rtb_ABSFlag_idx_3;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Brake_brake_pressure_Pa[3] = rtb_brake_pressure_Pa_idx_3;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_EV_Y.SoftEcu_Out_Diff_DiffPrsCmd[3] = rtb_DiffPrsCmd_idx_3;
  }

  if (rtmIsMajorTimeStep((&TxCar_EV_M))) {
    if (rtmIsMajorTimeStep((&TxCar_EV_M))) {
      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_EV_DW.Memory_1_PreviousInput[0] = rtb_wheel_angle[0];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_EV_DW.Memory_2_PreviousInput[0] = rtb_AxlTrq[0];

      /* Update for Memory generated from: '<S8>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_EV_DW.Memory_3_PreviousInput[0] = TxCar_EV_Y.DriveLineBus_Out_BrkPrs[0];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_EV_DW.Memory_1_PreviousInput[1] = rtb_wheel_angle[1];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_EV_DW.Memory_2_PreviousInput[1] = rtb_AxlTrq[1];

      /* Update for Memory generated from: '<S8>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_EV_DW.Memory_3_PreviousInput[1] = TxCar_EV_Y.DriveLineBus_Out_BrkPrs[1];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_EV_DW.Memory_1_PreviousInput[2] = rtb_wheel_angle[2];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_EV_DW.Memory_2_PreviousInput[2] = rtb_AxlTrq[2];

      /* Update for Memory generated from: '<S8>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_EV_DW.Memory_3_PreviousInput[2] = TxCar_EV_Y.DriveLineBus_Out_BrkPrs[2];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_EV_DW.Memory_1_PreviousInput[3] = rtb_wheel_angle[3];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_EV_DW.Memory_2_PreviousInput[3] = rtb_AxlTrq[3];

      /* Update for Memory generated from: '<S8>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_EV_DW.Memory_3_PreviousInput[3] = TxCar_EV_Y.DriveLineBus_Out_BrkPrs[3];

      /* Update for Memory generated from: '<S28>/Memory' */
      TxCar_EV_DW.Memory_5_PreviousInput = rtb_batt_soc_null;

      /* Update for Memory generated from: '<S28>/Memory' */
      TxCar_EV_DW.Memory_1_PreviousInput_i = 0.0;

      /* Update for Memory generated from: '<S28>/Memory' */
      TxCar_EV_DW.Memory_2_PreviousInput_p = rtb_mot_trq_Nm;

      /* Update for Memory generated from: '<S28>/Memory' */
      TxCar_EV_DW.Memory_3_PreviousInput_k = TxCar_EV_B.SFunction_ep.mot_trq_Nm;

      /* Update for Memory generated from: '<S28>/Memory' */
      TxCar_EV_DW.Memory_4_PreviousInput = TxCar_EV_B.SFunction_k.batt_volt_V;

      /* Update for Memory generated from: '<S28>/Memory' */
      TxCar_EV_DW.Memory_6_PreviousInput = rtb_Add;

      /* Update for Memory: '<S29>/Memory1' */
      TxCar_EV_DW.Memory1_PreviousInput = rtb_Add;
    }
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&TxCar_EV_M))) {
    rt_ertODEUpdateContinuousStates(&(&TxCar_EV_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&TxCar_EV_M)->Timing.clockTick0)) {
      ++(&TxCar_EV_M)->Timing.clockTickH0;
    }

    (&TxCar_EV_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&TxCar_EV_M)->solverInfo);

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
      (&TxCar_EV_M)->Timing.clockTick1++;
      if (!(&TxCar_EV_M)->Timing.clockTick1) {
        (&TxCar_EV_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void TxCar_EV::TxCar_EV_derivatives() {
  XDot_TxCar_EV_T *_rtXdot;
  _rtXdot = ((XDot_TxCar_EV_T *)(&TxCar_EV_M)->derivs);

  /* Derivatives for Integrator: '<S17>/Integrator1' */
  _rtXdot->Integrator1_CSTATE = TxCar_EV_B.Divide;
}

/* Model initialize function */
void TxCar_EV::initialize() {
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&TxCar_EV_M)->solverInfo, &(&TxCar_EV_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&TxCar_EV_M)->solverInfo, &rtmGetTPtr((&TxCar_EV_M)));
    rtsiSetStepSizePtr(&(&TxCar_EV_M)->solverInfo, &(&TxCar_EV_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&TxCar_EV_M)->solverInfo, &(&TxCar_EV_M)->derivs);
    rtsiSetContStatesPtr(&(&TxCar_EV_M)->solverInfo, (real_T **)&(&TxCar_EV_M)->contStates);
    rtsiSetNumContStatesPtr(&(&TxCar_EV_M)->solverInfo, &(&TxCar_EV_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&TxCar_EV_M)->solverInfo, &(&TxCar_EV_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&TxCar_EV_M)->solverInfo, &(&TxCar_EV_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&TxCar_EV_M)->solverInfo, &(&TxCar_EV_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&TxCar_EV_M)->solverInfo, (&rtmGetErrorStatus((&TxCar_EV_M))));
    rtsiSetRTModelPtr(&(&TxCar_EV_M)->solverInfo, (&TxCar_EV_M));
  }

  rtsiSetSimTimeStep(&(&TxCar_EV_M)->solverInfo, MAJOR_TIME_STEP);
  (&TxCar_EV_M)->intgData.y = (&TxCar_EV_M)->odeY;
  (&TxCar_EV_M)->intgData.f[0] = (&TxCar_EV_M)->odeF[0];
  (&TxCar_EV_M)->intgData.f[1] = (&TxCar_EV_M)->odeF[1];
  (&TxCar_EV_M)->intgData.f[2] = (&TxCar_EV_M)->odeF[2];
  (&TxCar_EV_M)->intgData.f[3] = (&TxCar_EV_M)->odeF[3];
  (&TxCar_EV_M)->contStates = ((X_TxCar_EV_T *)&TxCar_EV_X);
  rtsiSetSolverData(&(&TxCar_EV_M)->solverInfo, static_cast<void *>(&(&TxCar_EV_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&TxCar_EV_M)->solverInfo, false);
  rtsiSetSolverName(&(&TxCar_EV_M)->solverInfo, "ode4");
  rtmSetTPtr((&TxCar_EV_M), &(&TxCar_EV_M)->Timing.tArray[0]);
  (&TxCar_EV_M)->Timing.stepSize0 = 0.001;

  /* Start for S-Function (chassis_sfunc): '<S2>/Chassis' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S2>/Chassis */
  chassis_sfunc_Start_wrapper_cgen(TxCar_EV_P.Chassis_P1, 21, TxCar_EV_P.Chassis_P2, 22);

  /* Start for S-Function (ecu_sfunction): '<S43>/S-Function' */

  /* S-Function Block: <S43>/S-Function */
  ecu_sfunction_Start_wrapper_cgen(TxCar_EV_P.SFunction_P1, 21, TxCar_EV_P.SFunction_P2, 22);

  /* Start for S-Function (driveline_ev_front_sfunction): '<S15>/S-Function' */

  /* S-Function Block: <S15>/S-Function */
  driveline_ev_front_sfunction_Start_wrapper_cgen(TxCar_EV_P.SFunction_P1_c, 21, TxCar_EV_P.SFunction_P2_b, 22);

  /* Start for S-Function (driveline_ev_rear_sfunction): '<S16>/S-Function' */

  /* S-Function Block: <S16>/S-Function */
  driveline_ev_rear_sfunction_Start_wrapper_cgen(TxCar_EV_P.SFunction_P1_l, 21, TxCar_EV_P.SFunction_P2_m, 22);

  /* Start for S-Function (cartype_sfunction): '<S1>/S-Function' incorporates:
   *  Constant: '<S1>/Zero'
   */

  /* S-Function Block: <S1>/S-Function */
  cartype_sfunction_Start_wrapper_cgen(TxCar_EV_P.SFunction_P1_lo, 21, TxCar_EV_P.SFunction_P2_o, 22);

  /* Start for S-Function (brakehydraulic_sfunc): '<S14>/S-Function' incorporates:
   *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
   *  Outport generated from: '<Root>/SoftEcu_Outport_4'
   */

  /* S-Function Block: <S14>/S-Function */
  brakehydraulic_sfunc_Start_wrapper_cgen(TxCar_EV_P.SFunction_P1_f, 21, TxCar_EV_P.SFunction_P2_i, 22);

  /* Start for S-Function (battery_sfunction): '<S31>/S-Function' */

  /* S-Function Block: <S31>/S-Function */
  battery_sfunction_Start_wrapper_cgen(TxCar_EV_P.SFunction_P1_m, 21, TxCar_EV_P.SFunction_P2_g, 22);

  /* Start for S-Function (front_motor_sfunction): '<S32>/S-Function' */

  /* S-Function Block: <S32>/S-Function */
  front_motor_sfunction_Start_wrapper_cgen(TxCar_EV_P.SFunction_P1_h, 21, TxCar_EV_P.SFunction_P2_by, 22);

  /* Start for S-Function (rear_motor_sfunction): '<S33>/S-Function' */

  /* S-Function Block: <S33>/S-Function */
  rear_motor_sfunction_Start_wrapper_cgen(TxCar_EV_P.SFunction_P1_g, 21, TxCar_EV_P.SFunction_P2_ox, 22);

  /* Start for S-Function (dynamic_steer_sfunc): '<S48>/Steer_Mapped' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Bus Element Out2'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S48>/Steer_Mapped */
  dynamic_steer_sfunc_Start_wrapper_cgen(TxCar_EV_P.Steer_Mapped_P1, 21, TxCar_EV_P.Steer_Mapped_P2, 22);

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_1_PreviousInput[0] = TxCar_EV_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_2_PreviousInput[0] = TxCar_EV_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_3_PreviousInput[0] = TxCar_EV_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_1_PreviousInput[1] = TxCar_EV_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_2_PreviousInput[1] = TxCar_EV_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_3_PreviousInput[1] = TxCar_EV_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_1_PreviousInput[2] = TxCar_EV_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_2_PreviousInput[2] = TxCar_EV_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_3_PreviousInput[2] = TxCar_EV_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_1_PreviousInput[3] = TxCar_EV_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_2_PreviousInput[3] = TxCar_EV_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_EV_DW.Memory_3_PreviousInput[3] = TxCar_EV_P.Memory_3_InitialCondition;

  /* InitializeConditions for Integrator: '<S17>/Integrator1' */
  TxCar_EV_X.Integrator1_CSTATE = TxCar_EV_P.Integrator1_IC;

  /* InitializeConditions for Memory generated from: '<S28>/Memory' */
  TxCar_EV_DW.Memory_5_PreviousInput = TxCar_EV_P.Memory_5_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S28>/Memory' */
  TxCar_EV_DW.Memory_1_PreviousInput_i = TxCar_EV_P.Memory_1_InitialCondition_l;

  /* InitializeConditions for Memory generated from: '<S28>/Memory' */
  TxCar_EV_DW.Memory_2_PreviousInput_p = TxCar_EV_P.Memory_2_InitialCondition_l;

  /* InitializeConditions for Memory generated from: '<S28>/Memory' */
  TxCar_EV_DW.Memory_3_PreviousInput_k = TxCar_EV_P.Memory_3_InitialCondition_l;

  /* InitializeConditions for Memory generated from: '<S28>/Memory' */
  TxCar_EV_DW.Memory_4_PreviousInput = TxCar_EV_P.Memory_4_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S28>/Memory' */
  TxCar_EV_DW.Memory_6_PreviousInput = TxCar_EV_P.Memory_6_InitialCondition;

  /* InitializeConditions for Memory: '<S29>/Memory1' */
  TxCar_EV_DW.Memory1_PreviousInput = TxCar_EV_P.Memory1_InitialCondition;
}

/* Model terminate function */
void TxCar_EV::terminate() {
  /* Terminate for S-Function (chassis_sfunc): '<S2>/Chassis' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S2>/Chassis */
  chassis_sfunc_Terminate_wrapper_cgen(TxCar_EV_P.Chassis_P1, 21, TxCar_EV_P.Chassis_P2, 22);

  /* Terminate for S-Function (ecu_sfunction): '<S43>/S-Function' */

  /* S-Function Block: <S43>/S-Function */
  ecu_sfunction_Terminate_wrapper_cgen(TxCar_EV_P.SFunction_P1, 21, TxCar_EV_P.SFunction_P2, 22);

  /* Terminate for S-Function (driveline_ev_front_sfunction): '<S15>/S-Function' */

  /* S-Function Block: <S15>/S-Function */
  driveline_ev_front_sfunction_Terminate_wrapper_cgen(TxCar_EV_P.SFunction_P1_c, 21, TxCar_EV_P.SFunction_P2_b, 22);

  /* Terminate for S-Function (driveline_ev_rear_sfunction): '<S16>/S-Function' */

  /* S-Function Block: <S16>/S-Function */
  driveline_ev_rear_sfunction_Terminate_wrapper_cgen(TxCar_EV_P.SFunction_P1_l, 21, TxCar_EV_P.SFunction_P2_m, 22);

  /* Terminate for S-Function (cartype_sfunction): '<S1>/S-Function' incorporates:
   *  Constant: '<S1>/Zero'
   */

  /* S-Function Block: <S1>/S-Function */
  cartype_sfunction_Terminate_wrapper_cgen(TxCar_EV_P.SFunction_P1_lo, 21, TxCar_EV_P.SFunction_P2_o, 22);

  /* Terminate for S-Function (brakehydraulic_sfunc): '<S14>/S-Function' incorporates:
   *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
   *  Outport generated from: '<Root>/SoftEcu_Outport_4'
   */

  /* S-Function Block: <S14>/S-Function */
  brakehydraulic_sfunc_Terminate_wrapper_cgen(TxCar_EV_P.SFunction_P1_f, 21, TxCar_EV_P.SFunction_P2_i, 22);

  /* Terminate for S-Function (battery_sfunction): '<S31>/S-Function' */

  /* S-Function Block: <S31>/S-Function */
  battery_sfunction_Terminate_wrapper_cgen(TxCar_EV_P.SFunction_P1_m, 21, TxCar_EV_P.SFunction_P2_g, 22);

  /* Terminate for S-Function (front_motor_sfunction): '<S32>/S-Function' */

  /* S-Function Block: <S32>/S-Function */
  front_motor_sfunction_Terminate_wrapper_cgen(TxCar_EV_P.SFunction_P1_h, 21, TxCar_EV_P.SFunction_P2_by, 22);

  /* Terminate for S-Function (rear_motor_sfunction): '<S33>/S-Function' */

  /* S-Function Block: <S33>/S-Function */
  rear_motor_sfunction_Terminate_wrapper_cgen(TxCar_EV_P.SFunction_P1_g, 21, TxCar_EV_P.SFunction_P2_ox, 22);

  /* Terminate for S-Function (dynamic_steer_sfunc): '<S48>/Steer_Mapped' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Bus Element Out2'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S48>/Steer_Mapped */
  dynamic_steer_sfunc_Terminate_wrapper_cgen(TxCar_EV_P.Steer_Mapped_P1, 21, TxCar_EV_P.Steer_Mapped_P2, 22);
}

/* Constructor */
TxCar_EV::TxCar_EV() : TxCar_EV_U(), TxCar_EV_Y(), TxCar_EV_B(), TxCar_EV_DW(), TxCar_EV_X(), TxCar_EV_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
TxCar_EV::~TxCar_EV() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_TxCar_EV_T *TxCar_EV::getRTM() { return (&TxCar_EV_M); }
