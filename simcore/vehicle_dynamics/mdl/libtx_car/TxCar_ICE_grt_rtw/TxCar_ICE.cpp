/*
 * TxCar_ICE.cpp
 *
 * Code generation for model "TxCar_ICE".
 *
 * Model version              : 2.1530
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul 19 14:26:25 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TxCar_ICE.h"
#include <cmath>
#include "TxCar_ICE_private.h"
#include "rtwtypes.h"

extern "C" {

#include "rt_nonfinite.h"
}

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void TxCar_ICE::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  TxCar_ICE_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  TxCar_ICE_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  TxCar_ICE_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  TxCar_ICE_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Model step function */
void TxCar_ICE::step() {
  /* local block i/o variables */
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
  real_T rtb_speed;
  real_T rtb_throttle_01;
  if (rtmIsMajorTimeStep((&TxCar_ICE_M))) {
    /* set solver stop time */
    if (!((&TxCar_ICE_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&TxCar_ICE_M)->solverInfo, (((&TxCar_ICE_M)->Timing.clockTickH0 + 1) *
                                                          (&TxCar_ICE_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(&(&TxCar_ICE_M)->solverInfo,
                            (((&TxCar_ICE_M)->Timing.clockTick0 + 1) * (&TxCar_ICE_M)->Timing.stepSize0 +
                             (&TxCar_ICE_M)->Timing.clockTickH0 * (&TxCar_ICE_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&TxCar_ICE_M))) {
    (&TxCar_ICE_M)->Timing.t[0] = rtsiGetT(&(&TxCar_ICE_M)->solverInfo);
  }

  if (rtmIsMajorTimeStep((&TxCar_ICE_M))) {
    /* Constant: '<S5>/Constant3' incorporates:
     *  BusCreator generated from: '<S2>/Chassis'
     */
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[0] = TxCar_ICE_P.Constant3_Value;

    /* Constant: '<S5>/Constant4' incorporates:
     *  BusCreator generated from: '<S2>/Chassis'
     */
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[1] = TxCar_ICE_P.Constant4_Value;

    /* Constant: '<S5>/Constant5' incorporates:
     *  BusCreator generated from: '<S2>/Chassis'
     */
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Wind[2] = TxCar_ICE_P.Constant5_Value;

    /* Switch: '<S23>/Switch' incorporates:
     *  Constant: '<S24>/Constant'
     *  Inport generated from: '<Root>/Friction'
     *  RelationalOperator: '<S24>/Compare'
     */
    if (TxCar_ICE_U.Friction.mu_FL <= TxCar_ICE_P.CompareToConstant_const) {
      /* BusCreator generated from: '<S2>/Chassis' incorporates:
       *  Constant: '<S23>/const_mu'
       */
      TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FL =
          TxCar_ICE_P.const_mu_Value[0];
      TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FR =
          TxCar_ICE_P.const_mu_Value[1];
      TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RL =
          TxCar_ICE_P.const_mu_Value[2];
      TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RR =
          TxCar_ICE_P.const_mu_Value[3];
    } else {
      /* BusCreator generated from: '<S2>/Chassis' */
      TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FL = TxCar_ICE_U.Friction.mu_FL;
      TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_FR = TxCar_ICE_U.Friction.mu_FR;
      TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RL = TxCar_ICE_U.Friction.mu_RL;
      TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.mu.mu_RR = TxCar_ICE_U.Friction.mu_RR;
    }

    /* End of Switch: '<S23>/Switch' */

    /* BusCreator generated from: '<S2>/Chassis' incorporates:
     *  Inport generated from: '<Root>/Height'
     */
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1.Gnd.z = TxCar_ICE_U.Height;

    /* BusCreator generated from: '<S2>/Chassis' incorporates:
     *  Memory generated from: '<S8>/Memory'
     */
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[0] =
        TxCar_ICE_DW.Memory_1_PreviousInput[0];
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[0] =
        TxCar_ICE_DW.Memory_2_PreviousInput[0];
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[0] =
        TxCar_ICE_DW.Memory_3_PreviousInput[0];
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[1] =
        TxCar_ICE_DW.Memory_1_PreviousInput[1];
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[1] =
        TxCar_ICE_DW.Memory_2_PreviousInput[1];
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[1] =
        TxCar_ICE_DW.Memory_3_PreviousInput[1];
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[2] =
        TxCar_ICE_DW.Memory_1_PreviousInput[2];
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[2] =
        TxCar_ICE_DW.Memory_2_PreviousInput[2];
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[2] =
        TxCar_ICE_DW.Memory_3_PreviousInput[2];
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.StrgAng[3] =
        TxCar_ICE_DW.Memory_1_PreviousInput[3];
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.AxlTrq[3] =
        TxCar_ICE_DW.Memory_2_PreviousInput[3];
    TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1.BrkPrs[3] =
        TxCar_ICE_DW.Memory_3_PreviousInput[3];

    /* S-Function (chassis_sfunc): '<S2>/Chassis' incorporates:
     *  Outport generated from: '<Root>/Body'
     *  Outport generated from: '<Root>/Susp'
     *  Outport generated from: '<Root>/WheelsOut'
     */
    chassis_sfunc_Outputs_wrapper_cgen(&TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1,
                                       &TxCar_ICE_B.BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1,
                                       &TxCar_ICE_Y.Body, &TxCar_ICE_Y.WheelsOut, &TxCar_ICE_Y.Susp,
                                       TxCar_ICE_P.Chassis_P1, 21, TxCar_ICE_P.Chassis_P2, 22);

    /* Gain: '<S36>/Gain' incorporates:
     *  Outport generated from: '<Root>/Body'
     */
    TxCar_ICE_B.ax = TxCar_ICE_P.Gain_Gain * TxCar_ICE_Y.Body.BdyFrm.Cg.Acc.ax;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' incorporates:
     *  Memory generated from: '<S27>/Memory'
     */
    TxCar_ICE_Y.PowerBus_Out_battery_info_batt_soc_null = TxCar_ICE_DW.Memory_4_PreviousInput;
  }

  /* BusCreator: '<S36>/Bus Creator1' incorporates:
   *  Constant: '<S39>/Constant1'
   *  Inport generated from: '<Root>/DriverIn'
   *  Integrator: '<S16>/Integrator1'
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/PowerBus_Outport_2'
   *  Outport generated from: '<Root>/WheelsOut'
   */
  TxCar_ICE_B.BusCreator1.acc_feedback_m_s2 = TxCar_ICE_B.ax;
  TxCar_ICE_B.BusCreator1.eng_spd_rad_s = TxCar_ICE_X.Integrator1_CSTATE;
  TxCar_ICE_B.BusCreator1.batt_soc_0_1 = TxCar_ICE_Y.PowerBus_Out_battery_info_batt_soc_null;
  TxCar_ICE_B.BusCreator1.diff_trq_req_Nm = TxCar_ICE_P.Constant1_Value;
  TxCar_ICE_B.BusCreator1.slip_ratio[0] = TxCar_ICE_Y.WheelsOut.TireFrame.Kappa[0];
  TxCar_ICE_B.BusCreator1.slip_ratio[1] = TxCar_ICE_Y.WheelsOut.TireFrame.Kappa[1];
  TxCar_ICE_B.BusCreator1.slip_ratio[2] = TxCar_ICE_Y.WheelsOut.TireFrame.Kappa[2];
  TxCar_ICE_B.BusCreator1.slip_ratio[3] = TxCar_ICE_Y.WheelsOut.TireFrame.Kappa[3];
  TxCar_ICE_B.BusCreator1.veh_speed_vx = TxCar_ICE_Y.Body.BdyFrm.Cg.Vel.xdot;
  TxCar_ICE_B.BusCreator1.driver_input = TxCar_ICE_U.DriverIn;
  if (rtmIsMajorTimeStep((&TxCar_ICE_M))) {
    /* S-Function (ecu_sfunction): '<S35>/S-Function' */
    ecu_sfunction_Outputs_wrapper_cgen(&TxCar_ICE_B.BusCreator1, &TxCar_ICE_B.SFunction, TxCar_ICE_P.SFunction_P1, 21,
                                       TxCar_ICE_P.SFunction_P2, 22);

    /* Gain: '<S18>/Gain5' */
    TxCar_ICE_B.acc_cmd = TxCar_ICE_P.Gain5_Gain * TxCar_ICE_B.SFunction.throttle_01;

    /* Gain: '<S18>/Gain1' incorporates:
     *  Constant: '<S18>/Constant1'
     */
    TxCar_ICE_B.brake_cmd_0_1 = TxCar_ICE_P.Gain1_Gain * TxCar_ICE_P.Constant1_Value_f;

    /* Gain: '<S18>/Gain2' incorporates:
     *  Outport generated from: '<Root>/WheelsOut'
     */
    TxCar_ICE_B.wheel_spd_rad_s[0] = TxCar_ICE_P.Gain2_Gain * TxCar_ICE_Y.WheelsOut.TireFrame.Omega[0];
    TxCar_ICE_B.wheel_spd_rad_s[1] = TxCar_ICE_P.Gain2_Gain * TxCar_ICE_Y.WheelsOut.TireFrame.Omega[1];
    TxCar_ICE_B.wheel_spd_rad_s[2] = TxCar_ICE_P.Gain2_Gain * TxCar_ICE_Y.WheelsOut.TireFrame.Omega[2];
    TxCar_ICE_B.wheel_spd_rad_s[3] = TxCar_ICE_P.Gain2_Gain * TxCar_ICE_Y.WheelsOut.TireFrame.Omega[3];

    /* Gain: '<S18>/Gain3' incorporates:
     *  Constant: '<S18>/Constant2'
     */
    TxCar_ICE_B.abs_flag = TxCar_ICE_P.Gain3_Gain * TxCar_ICE_P.Constant2_Value;

    /* Memory generated from: '<S27>/Memory' */
    rtb_eng_trqNm = TxCar_ICE_DW.Memory_1_PreviousInput_i;

    /* Gain: '<S18>/Gain4' incorporates:
     *  Memory generated from: '<S27>/Memory'
     */
    TxCar_ICE_B.engine_trq_Nm = TxCar_ICE_P.Gain4_Gain * TxCar_ICE_DW.Memory_1_PreviousInput_i;

    /* SignalConversion generated from: '<S39>/Vector Concatenate2' incorporates:
     *  Constant: '<S39>/Constant'
     */
    rtb_DiffPrsCmd_idx_0 = TxCar_ICE_P.Constant_Value;

    /* SignalConversion generated from: '<S39>/Vector Concatenate2' incorporates:
     *  Constant: '<S39>/Constant'
     */
    rtb_DiffPrsCmd_idx_1 = TxCar_ICE_P.Constant_Value;

    /* SignalConversion generated from: '<S39>/Vector Concatenate2' incorporates:
     *  Constant: '<S39>/Constant'
     */
    rtb_DiffPrsCmd_idx_2 = TxCar_ICE_P.Constant_Value;

    /* SignalConversion generated from: '<S39>/Vector Concatenate2' incorporates:
     *  Constant: '<S39>/Constant'
     */
    rtb_DiffPrsCmd_idx_3 = TxCar_ICE_P.Constant_Value;

    /* DataTypeConversion: '<S18>/Data Type Conversion' incorporates:
     *  Constant: '<S39>/Constant'
     */
    TxCar_ICE_B.diff_prs_cmd[0] = (TxCar_ICE_P.Constant_Value != 0.0);
    TxCar_ICE_B.diff_prs_cmd[1] = (TxCar_ICE_P.Constant_Value != 0.0);
    TxCar_ICE_B.diff_prs_cmd[2] = (TxCar_ICE_P.Constant_Value != 0.0);
    TxCar_ICE_B.diff_prs_cmd[3] = (TxCar_ICE_P.Constant_Value != 0.0);

    /* Memory generated from: '<S27>/Memory' */
    rtb_mot_front_trqNm = TxCar_ICE_DW.Memory_2_PreviousInput_p;

    /* Gain: '<S18>/Gain6' incorporates:
     *  Memory generated from: '<S27>/Memory'
     */
    TxCar_ICE_B.mot_front_trq_Nm = TxCar_ICE_P.Gain6_Gain * TxCar_ICE_DW.Memory_2_PreviousInput_p;

    /* Memory generated from: '<S27>/Memory' */
    rtb_mot_rear_trqNm = TxCar_ICE_DW.Memory_3_PreviousInput_k;

    /* Gain: '<S18>/Gain7' incorporates:
     *  Memory generated from: '<S27>/Memory'
     */
    TxCar_ICE_B.mot_rear_trq_Nm = TxCar_ICE_P.Gain7_Gain * TxCar_ICE_DW.Memory_3_PreviousInput_k;
  }

  /* MultiPortSwitch: '<S19>/Multiport Switch' incorporates:
   *  Inport generated from: '<Root>/DriverIn'
   */
  switch (TxCar_ICE_U.DriverIn.gear_cmd__0N1D2R3P) {
    case 0:
      /* BusCreator: '<S18>/Bus Creator' incorporates:
       *  Constant: '<S19>/N'
       */
      TxCar_ICE_B.driveline.gear_cmd_driver = TxCar_ICE_P.N_Value;
      break;

    case 1:
      /* BusCreator: '<S18>/Bus Creator' incorporates:
       *  Constant: '<S19>/D'
       */
      TxCar_ICE_B.driveline.gear_cmd_driver = TxCar_ICE_P.D_Value;
      break;

    case 2:
      /* BusCreator: '<S18>/Bus Creator' incorporates:
       *  Constant: '<S19>/R'
       */
      TxCar_ICE_B.driveline.gear_cmd_driver = TxCar_ICE_P.R_Value;
      break;

    default:
      /* BusCreator: '<S18>/Bus Creator' incorporates:
       *  Constant: '<S19>/P'
       */
      TxCar_ICE_B.driveline.gear_cmd_driver = TxCar_ICE_P.P_Value;
      break;
  }

  /* End of MultiPortSwitch: '<S19>/Multiport Switch' */

  /* BusCreator: '<S18>/Bus Creator' */
  TxCar_ICE_B.driveline.acc_cmd = TxCar_ICE_B.acc_cmd;
  TxCar_ICE_B.driveline.brake_cmd_0_1 = TxCar_ICE_B.brake_cmd_0_1;
  TxCar_ICE_B.driveline.abs_flag = TxCar_ICE_B.abs_flag;
  TxCar_ICE_B.driveline.engine_trq_Nm = TxCar_ICE_B.engine_trq_Nm;
  TxCar_ICE_B.driveline.wheel_spd_rad_s[0] = TxCar_ICE_B.wheel_spd_rad_s[0];
  TxCar_ICE_B.driveline.diff_prs_cmd[0] = TxCar_ICE_B.diff_prs_cmd[0];
  TxCar_ICE_B.driveline.wheel_spd_rad_s[1] = TxCar_ICE_B.wheel_spd_rad_s[1];
  TxCar_ICE_B.driveline.diff_prs_cmd[1] = TxCar_ICE_B.diff_prs_cmd[1];
  TxCar_ICE_B.driveline.wheel_spd_rad_s[2] = TxCar_ICE_B.wheel_spd_rad_s[2];
  TxCar_ICE_B.driveline.diff_prs_cmd[2] = TxCar_ICE_B.diff_prs_cmd[2];
  TxCar_ICE_B.driveline.wheel_spd_rad_s[3] = TxCar_ICE_B.wheel_spd_rad_s[3];
  TxCar_ICE_B.driveline.diff_prs_cmd[3] = TxCar_ICE_B.diff_prs_cmd[3];
  TxCar_ICE_B.driveline.mot_front_trq_Nm = TxCar_ICE_B.mot_front_trq_Nm;
  TxCar_ICE_B.driveline.mot_rear_trq_Nm = TxCar_ICE_B.mot_rear_trq_Nm;
  if (rtmIsMajorTimeStep((&TxCar_ICE_M))) {
    /* Memory: '<S17>/Memory' */
    TxCar_ICE_B.Memory = TxCar_ICE_DW.Memory_PreviousInput;

    /* SignalConversion generated from: '<S15>/S-Function' */
    TxCar_ICE_B.TmpSignalConversionAtSFunctionInport3[0] = 0.0;
    TxCar_ICE_B.TmpSignalConversionAtSFunctionInport3[1] = 0.0;
    TxCar_ICE_B.TmpSignalConversionAtSFunctionInport3[2] = 0.0;
    TxCar_ICE_B.TmpSignalConversionAtSFunctionInport3[3] = 0.0;

    /* S-Function (driveline_ice_sfunction): '<S15>/S-Function' incorporates:
     *  Outport generated from: '<Root>/WheelsOut'
     */
    driveline_ice_sfunction_Outputs_wrapper_cgen(&TxCar_ICE_B.Memory, &TxCar_ICE_Y.WheelsOut.TireFrame.Omega[0],
                                                 &TxCar_ICE_B.TmpSignalConversionAtSFunctionInport3[0],
                                                 &TxCar_ICE_B.SFunction_o1[0], &TxCar_ICE_B.SFunction_o2,
                                                 TxCar_ICE_P.SFunction_P1_k, 21, TxCar_ICE_P.SFunction_P2_o, 22);

    /* S-Function (transmission_sfunction): '<S17>/S-Function' */
    transmission_sfunction_Outputs_wrapper_cgen(&TxCar_ICE_B.driveline, &TxCar_ICE_B.SFunction,
                                                &TxCar_ICE_B.SFunction_o2, &TxCar_ICE_B.SFunction_o1_i,
                                                &TxCar_ICE_B.SFunction_o2_j, &TxCar_ICE_B.SFunction_o3,
                                                TxCar_ICE_P.SFunction_P1_p, 21, TxCar_ICE_P.SFunction_P2_e, 22);

    /* BusCreator: '<S11>/Bus Creator1' incorporates:
     *  Constant: '<S11>/Constant2'
     *  Constant: '<S11>/Constant3'
     */
    TxCar_ICE_B.BusCreator1_p.gear_engaged = TxCar_ICE_B.SFunction_o1_i;
    TxCar_ICE_B.BusCreator1_p.eng_spd_rad_s = TxCar_ICE_B.SFunction_o2_j;
    TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[0] = TxCar_ICE_B.SFunction_o1[0];
    TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[1] = TxCar_ICE_B.SFunction_o1[1];
    TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[2] = TxCar_ICE_B.SFunction_o1[2];
    TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[3] = TxCar_ICE_B.SFunction_o1[3];
    TxCar_ICE_B.BusCreator1_p.trans_out_shaft_spd_rad_s = TxCar_ICE_B.SFunction_o2;
    TxCar_ICE_B.BusCreator1_p.front_mot_spd_rad_s = TxCar_ICE_P.Constant2_Value_k;
    TxCar_ICE_B.BusCreator1_p.rear_mot_spd_rad_s = TxCar_ICE_P.Constant3_Value_o;
  }

  /* Product: '<S16>/Divide' incorporates:
   *  Constant: '<S16>/Constant'
   *  Integrator: '<S16>/Integrator1'
   *  Sum: '<S16>/Sum'
   */
  TxCar_ICE_B.Divide =
      (TxCar_ICE_B.BusCreator1_p.eng_spd_rad_s - TxCar_ICE_X.Integrator1_CSTATE) * TxCar_ICE_P.EngSpdAct_wc;
  if (rtmIsMajorTimeStep((&TxCar_ICE_M))) {
    /* Saturate: '<S11>/Saturation' */
    if (TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[0] > TxCar_ICE_P.Saturation_UpperSat) {
      rtb_Saturation_idx_0 = TxCar_ICE_P.Saturation_UpperSat;
    } else if (TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[0] < TxCar_ICE_P.Saturation_LowerSat) {
      rtb_Saturation_idx_0 = TxCar_ICE_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_0 = TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[0];
    }

    if (TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[1] > TxCar_ICE_P.Saturation_UpperSat) {
      rtb_Saturation_idx_1 = TxCar_ICE_P.Saturation_UpperSat;
    } else if (TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[1] < TxCar_ICE_P.Saturation_LowerSat) {
      rtb_Saturation_idx_1 = TxCar_ICE_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_1 = TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[1];
    }

    if (TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[2] > TxCar_ICE_P.Saturation_UpperSat) {
      rtb_Saturation_idx_2 = TxCar_ICE_P.Saturation_UpperSat;
    } else if (TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[2] < TxCar_ICE_P.Saturation_LowerSat) {
      rtb_Saturation_idx_2 = TxCar_ICE_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_2 = TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[2];
    }

    if (TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[3] > TxCar_ICE_P.Saturation_UpperSat) {
      rtb_Saturation_idx_3 = TxCar_ICE_P.Saturation_UpperSat;
    } else if (TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[3] < TxCar_ICE_P.Saturation_LowerSat) {
      rtb_Saturation_idx_3 = TxCar_ICE_P.Saturation_LowerSat;
    } else {
      rtb_Saturation_idx_3 = TxCar_ICE_B.BusCreator1_p.wheel_drive_trq_Nm[3];
    }

    /* End of Saturate: '<S11>/Saturation' */

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' incorporates:
     *  Gain: '<S34>/Gain2'
     */
    TxCar_ICE_Y.SoftEcu_Out_Brake_BrkPrsCmd01 = TxCar_ICE_P.Gain2_Gain_l * TxCar_ICE_B.SFunction.brake_pressure_0_1;

    /* S-Function (brakehydraulic_sfunc): '<S14>/S-Function' incorporates:
     *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
     *  Outport generated from: '<Root>/SoftEcu_Outport_4'
     */
    brakehydraulic_sfunc_Outputs_wrapper_cgen(&TxCar_ICE_Y.SoftEcu_Out_Brake_BrkPrsCmd01,
                                              &TxCar_ICE_Y.DriveLineBus_Out_BrkPrs[0], TxCar_ICE_P.SFunction_P1_f, 21,
                                              TxCar_ICE_P.SFunction_P2_i, 22);

    /* Memory generated from: '<S27>/Memory' */
    rtb_batt_volt_V = TxCar_ICE_DW.Memory_5_PreviousInput;

    /* Memory generated from: '<S27>/Memory' */
    rtb_current = TxCar_ICE_DW.Memory_6_PreviousInput;

    /* S-Function (cartype_sfunction): '<S1>/S-Function' incorporates:
     *  Constant: '<S1>/Zero'
     */
    cartype_sfunction_Outputs_wrapper_cgen(&TxCar_ICE_P.Zero_Value, &TxCar_ICE_B.SFunction_e,
                                           TxCar_ICE_P.SFunction_P1_l, 21, TxCar_ICE_P.SFunction_P2_ok, 22);

    /* Gain: '<S37>/Gain1' */
    rtb_throttle_01 = TxCar_ICE_P.Gain1_Gain_f * TxCar_ICE_B.SFunction.throttle_01;

    /* MultiPortSwitch: '<S31>/Multiport Switch2' */
    if (static_cast<int32_T>(TxCar_ICE_B.SFunction_e.proplutiontype) == 1) {
      /* MultiPortSwitch: '<S31>/Multiport Switch2' */
      TxCar_ICE_B.throttle_01 = rtb_throttle_01;
    } else {
      /* MultiPortSwitch: '<S31>/Multiport Switch2' incorporates:
       *  Constant: '<S31>/none'
       */
      TxCar_ICE_B.throttle_01 = TxCar_ICE_P.none_Value;
    }

    /* End of MultiPortSwitch: '<S31>/Multiport Switch2' */
  }

  /* BusCreator generated from: '<S29>/S-Function' */
  TxCar_ICE_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.throttle_01 = TxCar_ICE_B.throttle_01;

  /* MultiPortSwitch: '<S30>/Multiport Switch2' incorporates:
   *  Constant: '<S30>/none'
   *  Integrator: '<S16>/Integrator1'
   */
  if (static_cast<int32_T>(TxCar_ICE_B.SFunction_e.proplutiontype) == 1) {
    rtb_speed = TxCar_ICE_X.Integrator1_CSTATE;
  } else {
    rtb_speed = TxCar_ICE_P.none_Value_d;
  }

  /* BusCreator generated from: '<S29>/S-Function' incorporates:
   *  Gain: '<S30>/Gain'
   *  MultiPortSwitch: '<S30>/Multiport Switch2'
   */
  TxCar_ICE_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1.eng_spd_rpm =
      TxCar_ICE_P.Gain_Gain_d * rtb_speed;
  if (rtmIsMajorTimeStep((&TxCar_ICE_M))) {
    /* S-Function (engine_sfunction): '<S29>/S-Function' */
    engine_sfunction_Outputs_wrapper_cgen(&TxCar_ICE_B.BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1,
                                          &TxCar_ICE_B.SFunction_f, TxCar_ICE_P.SFunction_P1_ka, 21,
                                          TxCar_ICE_P.SFunction_P2_l, 22);
  }

  /* BusCreator generated from: '<S40>/Steer_Mapped' incorporates:
   *  Inport generated from: '<Root>/DriverIn'
   */
  TxCar_ICE_B.BusConversion_InsertedFor_Steer_Mapped_at_inport_0_BusCreator1 = TxCar_ICE_U.DriverIn;
  if (rtmIsMajorTimeStep((&TxCar_ICE_M))) {
    /* S-Function (dynamic_steer_sfunc): '<S40>/Steer_Mapped' incorporates:
     *  Outport generated from: '<Root>/Body'
     *  Outport generated from: '<Root>/Bus Element Out2'
     *  Outport generated from: '<Root>/Susp'
     *  Outport generated from: '<Root>/WheelsOut'
     */
    dynamic_steer_sfunc_Outputs_wrapper_cgen(
        &TxCar_ICE_B.BusConversion_InsertedFor_Steer_Mapped_at_inport_0_BusCreator1, &TxCar_ICE_Y.Body,
        &TxCar_ICE_Y.Susp, &TxCar_ICE_Y.WheelsOut, &TxCar_ICE_Y.SteerBus_Out, TxCar_ICE_P.Steer_Mapped_P1, 21,
        TxCar_ICE_P.Steer_Mapped_P2, 22);

    /* DeadZone: '<S32>/Dead Zone' incorporates:
     *  BusCreator: '<S36>/Bus Creator1'
     *  Outport generated from: '<Root>/Body'
     */
    if (TxCar_ICE_Y.Body.BdyFrm.Cg.Vel.xdot > TxCar_ICE_P.DeadZone_End) {
      rtb_TrqCmd = TxCar_ICE_Y.Body.BdyFrm.Cg.Vel.xdot - TxCar_ICE_P.DeadZone_End;
    } else if (TxCar_ICE_Y.Body.BdyFrm.Cg.Vel.xdot >= TxCar_ICE_P.DeadZone_Start) {
      rtb_TrqCmd = 0.0;
    } else {
      rtb_TrqCmd = TxCar_ICE_Y.Body.BdyFrm.Cg.Vel.xdot - TxCar_ICE_P.DeadZone_Start;
    }

    /* Signum: '<S32>/Sign' incorporates:
     *  DeadZone: '<S32>/Dead Zone'
     */
    if (std::isnan(rtb_TrqCmd)) {
      rtb_speed = (rtNaN);
    } else if (rtb_TrqCmd < 0.0) {
      rtb_speed = -1.0;
    } else {
      rtb_speed = (rtb_TrqCmd > 0.0);
    }

    /* Product: '<S32>/Product' incorporates:
     *  Constant: '<S32>/AxleTrqOffset'
     *  Signum: '<S32>/Sign'
     */
    rtb_speed *= TxCar_ICE_P.AxleTrqOffset_Value;

    /* Gain: '<S37>/Gain' */
    rtb_TrqCmd = TxCar_ICE_P.Gain_Gain_m * TxCar_ICE_B.SFunction.pt_trq_cmd_Nm;

    /* Saturate: '<S8>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_ICE_Y.SteerBus_Out.wheel_angle[0] > TxCar_ICE_P.Saturation_UpperSat_k) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[0] = TxCar_ICE_P.Saturation_UpperSat_k;
    } else if (TxCar_ICE_Y.SteerBus_Out.wheel_angle[0] < TxCar_ICE_P.Saturation_LowerSat_l) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[0] = TxCar_ICE_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[0] = TxCar_ICE_Y.SteerBus_Out.wheel_angle[0];
    }

    /* Sum: '<S32>/Add' */
    rtb_AxlTrq[0] = rtb_speed + rtb_Saturation_idx_0;

    /* Gain: '<S34>/Gain' */
    rtb_brake_pressure_Pa_idx_0 = TxCar_ICE_P.Gain_Gain_a * TxCar_ICE_B.SFunction.brake_pressure_Pa[0];

    /* Gain: '<S34>/Gain1' */
    rtb_ABSFlag_idx_0 = TxCar_ICE_P.Gain1_Gain_o * TxCar_ICE_B.SFunction.abs_flag[0];

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
     *  Sum: '<S32>/Add'
     */
    TxCar_ICE_Y.DriveLineBus_Out_AxlTrq[0] = rtb_Saturation_idx_0;

    /* Saturate: '<S8>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_ICE_Y.SteerBus_Out.wheel_angle[1] > TxCar_ICE_P.Saturation_UpperSat_k) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[1] = TxCar_ICE_P.Saturation_UpperSat_k;
    } else if (TxCar_ICE_Y.SteerBus_Out.wheel_angle[1] < TxCar_ICE_P.Saturation_LowerSat_l) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[1] = TxCar_ICE_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[1] = TxCar_ICE_Y.SteerBus_Out.wheel_angle[1];
    }

    /* Sum: '<S32>/Add' */
    rtb_AxlTrq[1] = rtb_speed + rtb_Saturation_idx_1;

    /* Gain: '<S34>/Gain' */
    rtb_brake_pressure_Pa_idx_1 = TxCar_ICE_P.Gain_Gain_a * TxCar_ICE_B.SFunction.brake_pressure_Pa[1];

    /* Gain: '<S34>/Gain1' */
    rtb_ABSFlag_idx_1 = TxCar_ICE_P.Gain1_Gain_o * TxCar_ICE_B.SFunction.abs_flag[1];

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
     *  Sum: '<S32>/Add'
     */
    TxCar_ICE_Y.DriveLineBus_Out_AxlTrq[1] = rtb_Saturation_idx_1;

    /* Saturate: '<S8>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_ICE_Y.SteerBus_Out.wheel_angle[2] > TxCar_ICE_P.Saturation_UpperSat_k) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[2] = TxCar_ICE_P.Saturation_UpperSat_k;
    } else if (TxCar_ICE_Y.SteerBus_Out.wheel_angle[2] < TxCar_ICE_P.Saturation_LowerSat_l) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[2] = TxCar_ICE_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[2] = TxCar_ICE_Y.SteerBus_Out.wheel_angle[2];
    }

    /* Sum: '<S32>/Add' */
    rtb_AxlTrq[2] = rtb_speed + rtb_Saturation_idx_2;

    /* Gain: '<S34>/Gain' */
    rtb_brake_pressure_Pa_idx_2 = TxCar_ICE_P.Gain_Gain_a * TxCar_ICE_B.SFunction.brake_pressure_Pa[2];

    /* Gain: '<S34>/Gain1' */
    rtb_ABSFlag_idx_2 = TxCar_ICE_P.Gain1_Gain_o * TxCar_ICE_B.SFunction.abs_flag[2];

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
     *  Sum: '<S32>/Add'
     */
    TxCar_ICE_Y.DriveLineBus_Out_AxlTrq[2] = rtb_Saturation_idx_2;

    /* Saturate: '<S8>/Saturation' incorporates:
     *  Outport generated from: '<Root>/Bus Element Out2'
     */
    if (TxCar_ICE_Y.SteerBus_Out.wheel_angle[3] > TxCar_ICE_P.Saturation_UpperSat_k) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[3] = TxCar_ICE_P.Saturation_UpperSat_k;
    } else if (TxCar_ICE_Y.SteerBus_Out.wheel_angle[3] < TxCar_ICE_P.Saturation_LowerSat_l) {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[3] = TxCar_ICE_P.Saturation_LowerSat_l;
    } else {
      /* Saturate: '<S8>/Saturation' */
      rtb_wheel_angle[3] = TxCar_ICE_Y.SteerBus_Out.wheel_angle[3];
    }

    /* Sum: '<S32>/Add' */
    rtb_AxlTrq[3] = rtb_speed + rtb_Saturation_idx_3;

    /* Gain: '<S34>/Gain' */
    rtb_brake_pressure_Pa_idx_3 = TxCar_ICE_P.Gain_Gain_a * TxCar_ICE_B.SFunction.brake_pressure_Pa[3];

    /* Gain: '<S34>/Gain1' */
    rtb_ABSFlag_idx_3 = TxCar_ICE_P.Gain1_Gain_o * TxCar_ICE_B.SFunction.abs_flag[3];

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
     *  Sum: '<S32>/Add'
     */
    TxCar_ICE_Y.DriveLineBus_Out_AxlTrq[3] = rtb_Saturation_idx_3;
  }

  /* Outport generated from: '<Root>/DriveLineBus_Outport_1' incorporates:
   *  Integrator: '<S16>/Integrator1'
   */
  TxCar_ICE_Y.DriveLineBus_Out_EngSpd = TxCar_ICE_X.Integrator1_CSTATE;
  if (rtmIsMajorTimeStep((&TxCar_ICE_M))) {
    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_ICE_Y.DriveLineBus_Out_PropShftSpd = TxCar_ICE_B.BusCreator1_p.trans_out_shaft_spd_rad_s;

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_ICE_Y.DriveLineBus_Out_Gear = TxCar_ICE_B.BusCreator1_p.gear_engaged;

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_ICE_Y.DriveLineBus_Out_FrontMotSpd = TxCar_ICE_B.BusCreator1_p.front_mot_spd_rad_s;

    /* Outport generated from: '<Root>/DriveLineBus_Outport_1' */
    TxCar_ICE_Y.DriveLineBus_Out_RearMotSpd = TxCar_ICE_B.BusCreator1_p.rear_mot_spd_rad_s;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_ICE_Y.PowerBus_Out_eng_trqNm = rtb_eng_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_ICE_Y.PowerBus_Out_mot_front_trqNm = rtb_mot_front_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_ICE_Y.PowerBus_Out_mot_rear_trqNm = rtb_mot_rear_trqNm;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_ICE_Y.PowerBus_Out_battery_info_batt_volt_V = rtb_batt_volt_V;

    /* Outport generated from: '<Root>/PowerBus_Outport_2' */
    TxCar_ICE_Y.PowerBus_Out_current = rtb_current;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_ECU_TrqCmd = rtb_TrqCmd;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_ECU_throttle_01 = rtb_throttle_01;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' incorporates:
     *  Constant: '<S34>/Constant'
     */
    TxCar_ICE_Y.SoftEcu_Out_Brake_TCSFlag = TxCar_ICE_P.Constant_Value_p;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Brake_ABSFlag[0] = rtb_ABSFlag_idx_0;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Brake_brake_pressure_Pa[0] = rtb_brake_pressure_Pa_idx_0;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Diff_DiffPrsCmd[0] = rtb_DiffPrsCmd_idx_0;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Brake_ABSFlag[1] = rtb_ABSFlag_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Brake_brake_pressure_Pa[1] = rtb_brake_pressure_Pa_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Diff_DiffPrsCmd[1] = rtb_DiffPrsCmd_idx_1;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Brake_ABSFlag[2] = rtb_ABSFlag_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Brake_brake_pressure_Pa[2] = rtb_brake_pressure_Pa_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Diff_DiffPrsCmd[2] = rtb_DiffPrsCmd_idx_2;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Brake_ABSFlag[3] = rtb_ABSFlag_idx_3;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Brake_brake_pressure_Pa[3] = rtb_brake_pressure_Pa_idx_3;

    /* Outport generated from: '<Root>/SoftEcu_Outport_4' */
    TxCar_ICE_Y.SoftEcu_Out_Diff_DiffPrsCmd[3] = rtb_DiffPrsCmd_idx_3;
  }

  if (rtmIsMajorTimeStep((&TxCar_ICE_M))) {
    if (rtmIsMajorTimeStep((&TxCar_ICE_M))) {
      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_ICE_DW.Memory_1_PreviousInput[0] = rtb_wheel_angle[0];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_ICE_DW.Memory_2_PreviousInput[0] = rtb_AxlTrq[0];

      /* Update for Memory generated from: '<S8>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_ICE_DW.Memory_3_PreviousInput[0] = TxCar_ICE_Y.DriveLineBus_Out_BrkPrs[0];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_ICE_DW.Memory_1_PreviousInput[1] = rtb_wheel_angle[1];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_ICE_DW.Memory_2_PreviousInput[1] = rtb_AxlTrq[1];

      /* Update for Memory generated from: '<S8>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_ICE_DW.Memory_3_PreviousInput[1] = TxCar_ICE_Y.DriveLineBus_Out_BrkPrs[1];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_ICE_DW.Memory_1_PreviousInput[2] = rtb_wheel_angle[2];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_ICE_DW.Memory_2_PreviousInput[2] = rtb_AxlTrq[2];

      /* Update for Memory generated from: '<S8>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_ICE_DW.Memory_3_PreviousInput[2] = TxCar_ICE_Y.DriveLineBus_Out_BrkPrs[2];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_ICE_DW.Memory_1_PreviousInput[3] = rtb_wheel_angle[3];

      /* Update for Memory generated from: '<S8>/Memory' */
      TxCar_ICE_DW.Memory_2_PreviousInput[3] = rtb_AxlTrq[3];

      /* Update for Memory generated from: '<S8>/Memory' incorporates:
       *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
       */
      TxCar_ICE_DW.Memory_3_PreviousInput[3] = TxCar_ICE_Y.DriveLineBus_Out_BrkPrs[3];

      /* Update for Memory generated from: '<S27>/Memory' */
      TxCar_ICE_DW.Memory_4_PreviousInput = 0.0;

      /* Update for Memory generated from: '<S27>/Memory' */
      TxCar_ICE_DW.Memory_1_PreviousInput_i = TxCar_ICE_B.SFunction_f.eng_trq_Nm;

      /* Update for Memory generated from: '<S27>/Memory' */
      TxCar_ICE_DW.Memory_2_PreviousInput_p = 0.0;

      /* Update for Memory generated from: '<S27>/Memory' */
      TxCar_ICE_DW.Memory_3_PreviousInput_k = 0.0;

      /* Update for Memory: '<S17>/Memory' */
      TxCar_ICE_DW.Memory_PreviousInput = TxCar_ICE_B.SFunction_o3;

      /* Update for Memory generated from: '<S27>/Memory' */
      TxCar_ICE_DW.Memory_5_PreviousInput = 0.0;

      /* Update for Memory generated from: '<S27>/Memory' */
      TxCar_ICE_DW.Memory_6_PreviousInput = 0.0;
    }
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&TxCar_ICE_M))) {
    rt_ertODEUpdateContinuousStates(&(&TxCar_ICE_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&TxCar_ICE_M)->Timing.clockTick0)) {
      ++(&TxCar_ICE_M)->Timing.clockTickH0;
    }

    (&TxCar_ICE_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&TxCar_ICE_M)->solverInfo);

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
      (&TxCar_ICE_M)->Timing.clockTick1++;
      if (!(&TxCar_ICE_M)->Timing.clockTick1) {
        (&TxCar_ICE_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void TxCar_ICE::TxCar_ICE_derivatives() {
  XDot_TxCar_ICE_T *_rtXdot;
  _rtXdot = reinterpret_cast<XDot_TxCar_ICE_T *>(&(&TxCar_ICE_M)->derivs);

  /* Derivatives for Integrator: '<S16>/Integrator1' */
  _rtXdot->Integrator1_CSTATE = TxCar_ICE_B.Divide;
}

/* Model initialize function */
void TxCar_ICE::initialize() {
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&TxCar_ICE_M)->solverInfo, &(&TxCar_ICE_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&TxCar_ICE_M)->solverInfo, &rtmGetTPtr((&TxCar_ICE_M)));
    rtsiSetStepSizePtr(&(&TxCar_ICE_M)->solverInfo, &(&TxCar_ICE_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&TxCar_ICE_M)->solverInfo, &(&TxCar_ICE_M)->derivs);
    rtsiSetContStatesPtr(&(&TxCar_ICE_M)->solverInfo, reinterpret_cast<real_T **>(&(TxCar_ICE_M.contStates)));
    rtsiSetNumContStatesPtr(&(&TxCar_ICE_M)->solverInfo, &(&TxCar_ICE_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&TxCar_ICE_M)->solverInfo, &(&TxCar_ICE_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&TxCar_ICE_M)->solverInfo, &(&TxCar_ICE_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&TxCar_ICE_M)->solverInfo, &(&TxCar_ICE_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&TxCar_ICE_M)->solverInfo, (&rtmGetErrorStatus((&TxCar_ICE_M))));
    rtsiSetRTModelPtr(&(&TxCar_ICE_M)->solverInfo, (&TxCar_ICE_M));
  }

  rtsiSetSimTimeStep(&(&TxCar_ICE_M)->solverInfo, MAJOR_TIME_STEP);
  (&TxCar_ICE_M)->intgData.y = (&TxCar_ICE_M)->odeY;
  (&TxCar_ICE_M)->intgData.f[0] = (&TxCar_ICE_M)->odeF[0];
  (&TxCar_ICE_M)->intgData.f[1] = (&TxCar_ICE_M)->odeF[1];
  (&TxCar_ICE_M)->intgData.f[2] = (&TxCar_ICE_M)->odeF[2];
  (&TxCar_ICE_M)->intgData.f[3] = (&TxCar_ICE_M)->odeF[3];
  (&TxCar_ICE_M)->contStates = reinterpret_cast<X_TxCar_ICE_T *>(&TxCar_ICE_X);
  rtsiSetSolverData(&(&TxCar_ICE_M)->solverInfo, static_cast<void *>(&(&TxCar_ICE_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&TxCar_ICE_M)->solverInfo, false);
  rtsiSetSolverName(&(&TxCar_ICE_M)->solverInfo, "ode4");
  rtmSetTPtr((&TxCar_ICE_M), &(&TxCar_ICE_M)->Timing.tArray[0]);
  (&TxCar_ICE_M)->Timing.stepSize0 = 0.001;

  /* Start for S-Function (chassis_sfunc): '<S2>/Chassis' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S2>/Chassis */
  chassis_sfunc_Start_wrapper_cgen(TxCar_ICE_P.Chassis_P1, 21, TxCar_ICE_P.Chassis_P2, 22);

  /* Start for S-Function (ecu_sfunction): '<S35>/S-Function' */

  /* S-Function Block: <S35>/S-Function */
  ecu_sfunction_Start_wrapper_cgen(TxCar_ICE_P.SFunction_P1, 21, TxCar_ICE_P.SFunction_P2, 22);

  /* Start for S-Function (driveline_ice_sfunction): '<S15>/S-Function' incorporates:
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S15>/S-Function */
  driveline_ice_sfunction_Start_wrapper_cgen(TxCar_ICE_P.SFunction_P1_k, 21, TxCar_ICE_P.SFunction_P2_o, 22);

  /* Start for S-Function (transmission_sfunction): '<S17>/S-Function' */

  /* S-Function Block: <S17>/S-Function */
  transmission_sfunction_Start_wrapper_cgen(TxCar_ICE_P.SFunction_P1_p, 21, TxCar_ICE_P.SFunction_P2_e, 22);

  /* Start for S-Function (brakehydraulic_sfunc): '<S14>/S-Function' incorporates:
   *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
   *  Outport generated from: '<Root>/SoftEcu_Outport_4'
   */

  /* S-Function Block: <S14>/S-Function */
  brakehydraulic_sfunc_Start_wrapper_cgen(TxCar_ICE_P.SFunction_P1_f, 21, TxCar_ICE_P.SFunction_P2_i, 22);

  /* Start for S-Function (cartype_sfunction): '<S1>/S-Function' incorporates:
   *  Constant: '<S1>/Zero'
   */

  /* S-Function Block: <S1>/S-Function */
  cartype_sfunction_Start_wrapper_cgen(TxCar_ICE_P.SFunction_P1_l, 21, TxCar_ICE_P.SFunction_P2_ok, 22);

  /* Start for S-Function (engine_sfunction): '<S29>/S-Function' */

  /* S-Function Block: <S29>/S-Function */
  engine_sfunction_Start_wrapper_cgen(TxCar_ICE_P.SFunction_P1_ka, 21, TxCar_ICE_P.SFunction_P2_l, 22);

  /* Start for S-Function (dynamic_steer_sfunc): '<S40>/Steer_Mapped' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Bus Element Out2'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S40>/Steer_Mapped */
  dynamic_steer_sfunc_Start_wrapper_cgen(TxCar_ICE_P.Steer_Mapped_P1, 21, TxCar_ICE_P.Steer_Mapped_P2, 22);

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_1_PreviousInput[0] = TxCar_ICE_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_2_PreviousInput[0] = TxCar_ICE_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_3_PreviousInput[0] = TxCar_ICE_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_1_PreviousInput[1] = TxCar_ICE_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_2_PreviousInput[1] = TxCar_ICE_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_3_PreviousInput[1] = TxCar_ICE_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_1_PreviousInput[2] = TxCar_ICE_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_2_PreviousInput[2] = TxCar_ICE_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_3_PreviousInput[2] = TxCar_ICE_P.Memory_3_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_1_PreviousInput[3] = TxCar_ICE_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_2_PreviousInput[3] = TxCar_ICE_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S8>/Memory' */
  TxCar_ICE_DW.Memory_3_PreviousInput[3] = TxCar_ICE_P.Memory_3_InitialCondition;

  /* InitializeConditions for Integrator: '<S16>/Integrator1' */
  TxCar_ICE_X.Integrator1_CSTATE = TxCar_ICE_P.Integrator1_IC;

  /* InitializeConditions for Memory generated from: '<S27>/Memory' */
  TxCar_ICE_DW.Memory_4_PreviousInput = TxCar_ICE_P.Memory_4_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S27>/Memory' */
  TxCar_ICE_DW.Memory_1_PreviousInput_i = TxCar_ICE_P.Memory_1_InitialCondition_l;

  /* InitializeConditions for Memory generated from: '<S27>/Memory' */
  TxCar_ICE_DW.Memory_2_PreviousInput_p = TxCar_ICE_P.Memory_2_InitialCondition_l;

  /* InitializeConditions for Memory generated from: '<S27>/Memory' */
  TxCar_ICE_DW.Memory_3_PreviousInput_k = TxCar_ICE_P.Memory_3_InitialCondition_l;

  /* InitializeConditions for Memory: '<S17>/Memory' */
  TxCar_ICE_DW.Memory_PreviousInput = TxCar_ICE_P.Memory_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S27>/Memory' */
  TxCar_ICE_DW.Memory_5_PreviousInput = TxCar_ICE_P.Memory_5_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S27>/Memory' */
  TxCar_ICE_DW.Memory_6_PreviousInput = TxCar_ICE_P.Memory_6_InitialCondition;
}

/* Model terminate function */
void TxCar_ICE::terminate() {
  /* Terminate for S-Function (chassis_sfunc): '<S2>/Chassis' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S2>/Chassis */
  chassis_sfunc_Terminate_wrapper_cgen(TxCar_ICE_P.Chassis_P1, 21, TxCar_ICE_P.Chassis_P2, 22);

  /* Terminate for S-Function (ecu_sfunction): '<S35>/S-Function' */

  /* S-Function Block: <S35>/S-Function */
  ecu_sfunction_Terminate_wrapper_cgen(TxCar_ICE_P.SFunction_P1, 21, TxCar_ICE_P.SFunction_P2, 22);

  /* Terminate for S-Function (driveline_ice_sfunction): '<S15>/S-Function' incorporates:
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S15>/S-Function */
  driveline_ice_sfunction_Terminate_wrapper_cgen(TxCar_ICE_P.SFunction_P1_k, 21, TxCar_ICE_P.SFunction_P2_o, 22);

  /* Terminate for S-Function (transmission_sfunction): '<S17>/S-Function' */

  /* S-Function Block: <S17>/S-Function */
  transmission_sfunction_Terminate_wrapper_cgen(TxCar_ICE_P.SFunction_P1_p, 21, TxCar_ICE_P.SFunction_P2_e, 22);

  /* Terminate for S-Function (brakehydraulic_sfunc): '<S14>/S-Function' incorporates:
   *  Outport generated from: '<Root>/DriveLineBus_Outport_1'
   *  Outport generated from: '<Root>/SoftEcu_Outport_4'
   */

  /* S-Function Block: <S14>/S-Function */
  brakehydraulic_sfunc_Terminate_wrapper_cgen(TxCar_ICE_P.SFunction_P1_f, 21, TxCar_ICE_P.SFunction_P2_i, 22);

  /* Terminate for S-Function (cartype_sfunction): '<S1>/S-Function' incorporates:
   *  Constant: '<S1>/Zero'
   */

  /* S-Function Block: <S1>/S-Function */
  cartype_sfunction_Terminate_wrapper_cgen(TxCar_ICE_P.SFunction_P1_l, 21, TxCar_ICE_P.SFunction_P2_ok, 22);

  /* Terminate for S-Function (engine_sfunction): '<S29>/S-Function' */

  /* S-Function Block: <S29>/S-Function */
  engine_sfunction_Terminate_wrapper_cgen(TxCar_ICE_P.SFunction_P1_ka, 21, TxCar_ICE_P.SFunction_P2_l, 22);

  /* Terminate for S-Function (dynamic_steer_sfunc): '<S40>/Steer_Mapped' incorporates:
   *  Outport generated from: '<Root>/Body'
   *  Outport generated from: '<Root>/Bus Element Out2'
   *  Outport generated from: '<Root>/Susp'
   *  Outport generated from: '<Root>/WheelsOut'
   */

  /* S-Function Block: <S40>/Steer_Mapped */
  dynamic_steer_sfunc_Terminate_wrapper_cgen(TxCar_ICE_P.Steer_Mapped_P1, 21, TxCar_ICE_P.Steer_Mapped_P2, 22);
}

/* Constructor */
TxCar_ICE::TxCar_ICE() : TxCar_ICE_U(), TxCar_ICE_Y(), TxCar_ICE_B(), TxCar_ICE_DW(), TxCar_ICE_X(), TxCar_ICE_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
TxCar_ICE::~TxCar_ICE() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_TxCar_ICE_T *TxCar_ICE::getRTM() { return (&TxCar_ICE_M); }
