/*
 * DynamicSteer.cpp
 *
 * Code generation for model "DynamicSteer".
 *
 * Model version              : 2.38
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Sat Jun 24 17:12:04 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "DynamicSteer.h"
#include <cmath>
#include "DynamicSteer_private.h"
#include "rtwtypes.h"
#include "zero_crossing_types.h"

extern "C" {

#include "rt_nonfinite.h"
}

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void DynamicSteer::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  int_T nXc{9};

  rtsiSetSimTimeStep(si, MINOR_TIME_STEP);

  /* Save the state values at time t in y, we'll use x as ynew. */
  (void)std::memcpy(y, x, static_cast<uint_T>(nXc) * sizeof(real_T));

  /* Assumes that rtsiSetT and ModelOutputs are up-to-date */
  /* f0 = f(t,y) */
  rtsiSetdX(si, f0);
  DynamicSteer_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  DynamicSteer_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  DynamicSteer_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  DynamicSteer_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

real_T rt_hypotd_snf(real_T u0, real_T u1) {
  real_T a;
  real_T b;
  real_T y;
  a = std::abs(u0);
  b = std::abs(u1);
  if (a < b) {
    a /= b;
    y = std::sqrt(a * a + 1.0) * b;
  } else if (a > b) {
    b /= a;
    y = std::sqrt(b * b + 1.0) * a;
  } else if (std::isnan(b)) {
    y = (rtNaN);
  } else {
    y = a * 1.4142135623730951;
  }

  return y;
}

real_T rt_powd_snf(real_T u0, real_T u1);

/* Model step function */
void DynamicSteer::step() {
  real_T rtb_Hypot;
  real_T rtb_steer_request_torque;
  int8_T rtAction;
  if (rtmIsMajorTimeStep((&DynamicSteer_M))) {
    /* set solver stop time */
    if (!((&DynamicSteer_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&DynamicSteer_M)->solverInfo, (((&DynamicSteer_M)->Timing.clockTickH0 + 1) *
                                                             (&DynamicSteer_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(
          &(&DynamicSteer_M)->solverInfo,
          (((&DynamicSteer_M)->Timing.clockTick0 + 1) * (&DynamicSteer_M)->Timing.stepSize0 +
           (&DynamicSteer_M)->Timing.clockTickH0 * (&DynamicSteer_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&DynamicSteer_M))) {
    (&DynamicSteer_M)->Timing.t[0] = rtsiGetT(&(&DynamicSteer_M)->solverInfo);
  }

  if (rtmIsMajorTimeStep((&DynamicSteer_M))) {
    /* Memory generated from: '<S1>/Memory' */
    DynamicSteer_Y.steerOut.wheel_angle[0] = DynamicSteer_DW.Memory_1_PreviousInput[0];
    DynamicSteer_Y.steerOut.wheel_angle[1] = DynamicSteer_DW.Memory_1_PreviousInput[1];
    DynamicSteer_Y.steerOut.wheel_angle[2] = DynamicSteer_DW.Memory_1_PreviousInput[2];
    DynamicSteer_Y.steerOut.wheel_angle[3] = DynamicSteer_DW.Memory_1_PreviousInput[3];

    /* Memory generated from: '<S1>/Memory' */
    DynamicSteer_Y.steerOut.steerWheelFeedback = DynamicSteer_DW.Memory_2_PreviousInput;

    /* Memory generated from: '<S1>/Memory' */
    DynamicSteer_Y.steerOut.steerTorqueInput = DynamicSteer_DW.Memory_3_PreviousInput;
  }

  /* SignalConversion generated from: '<Root>/driverIn' incorporates:
   *  Inport: '<Root>/driverIn'
   */
  DynamicSteer_B.steer_control_mode = DynamicSteer_U.driverIn.steer_control_mode;

  /* SignalConversion generated from: '<Root>/driverIn' incorporates:
   *  Inport: '<Root>/driverIn'
   */
  rtb_steer_request_torque = DynamicSteer_U.driverIn.steer_request_torque;

  /* SignalConversion generated from: '<Root>/driverIn' incorporates:
   *  Inport: '<Root>/driverIn'
   */
  DynamicSteer_B.steer_cmd_rad = DynamicSteer_U.driverIn.steer_cmd_rad;

  /* Math: '<S3>/Hypot' incorporates:
   *  Inport: '<Root>/bodyOut'
   */
  rtb_Hypot = rt_hypotd_snf(DynamicSteer_U.bodyOut.InertFrm.Cg.Vel.Xdot, DynamicSteer_U.bodyOut.InertFrm.Cg.Vel.Ydot);

  /* If: '<S1>/If' incorporates:
   *  Constant: '<S54>/domega_o'
   *  Constant: '<S54>/domega_o1'
   */
  if (rtsiIsModeUpdateTimeStep(&(&DynamicSteer_M)->solverInfo)) {
    rtAction = static_cast<int8_T>(DynamicSteer_B.steer_control_mode != 0);
    DynamicSteer_DW.If_ActiveSubsystem = rtAction;
  } else {
    rtAction = DynamicSteer_DW.If_ActiveSubsystem;
  }

  switch (rtAction) {
    case 0: {
      real_T rtb_resp;

      /* Outputs for IfAction SubSystem: '<S1>/MappedSteer' incorporates:
       *  ActionPort: '<S4>/Action Port'
       */
      /* TransferFcn: '<S76>/Transfer Fcn' */
      rtb_resp = DynamicSteer_P.TransferFcn_C[0] * DynamicSteer_X.TransferFcn_CSTATE[0] +
                 DynamicSteer_P.TransferFcn_C[1] * DynamicSteer_X.TransferFcn_CSTATE[1];

      /* Switch: '<S4>/Switch' incorporates:
       *  Constant: '<S4>/secondOrderSwitch'
       */
      if (!(DynamicSteer_P.secondOrderSwitch_Value > DynamicSteer_P.Switch_Threshold)) {
        rtb_resp = DynamicSteer_B.steer_cmd_rad;
      }

      /* End of Switch: '<S4>/Switch' */

      /* Lookup_n-D: '<S78>/rack_disp' incorporates:
       *  Lookup_n-D: '<S78>/SpeedCoeff'
       *  Math: '<S3>/Hypot'
       *  Product: '<S78>/ang_alterred'
       */
      rtb_steer_request_torque = look1_binlxpw(
          rtb_resp * look1_binlxpw(rtb_Hypot, DynamicSteer_P.MappedSteer_VehSpdBpts,
                                   DynamicSteer_P.MappedSteer_SpdFctTbl, m_speed_coef_maxIndex),
          DynamicSteer_P.MappedSteer_SteerAngBps, DynamicSteer_P.MappedSteer_RackDispBpts, m_steer_bps_maxIndex);

      /* Merge: '<S1>/Merge' incorporates:
       *  Constant: '<S79>/Constant1'
       *  Lookup_n-D: '<S78>/leff_whl_ang'
       *  Lookup_n-D: '<S78>/right_whl_ang'
       *  SignalConversion generated from: '<S4>/WhlAng'
       */
      DynamicSteer_B.WhlAng[0] = look1_binlxpw(rtb_steer_request_torque, DynamicSteer_P.MappedSteer_RackDispBpts,
                                               DynamicSteer_P.MappedSteer_WhlLftTbl, m_steer_bps_maxIndex);
      DynamicSteer_B.WhlAng[1] = look1_binlxpw(rtb_steer_request_torque, DynamicSteer_P.MappedSteer_RackDispBpts,
                                               DynamicSteer_P.MappedSteer_WhlRghtTbl, m_steer_bps_maxIndex);
      DynamicSteer_B.WhlAng[2] = DynamicSteer_P.Constant1_Value;
      DynamicSteer_B.WhlAng[3] = DynamicSteer_P.Constant1_Value;

      /* Merge: '<S1>/Merge2' incorporates:
       *  SignalConversion generated from: '<S4>/steerAngleFeedback'
       */
      DynamicSteer_B.steerAngleFeedback = rtb_resp;
      if (rtmIsMajorTimeStep((&DynamicSteer_M))) {
        /* Merge: '<S1>/Merge3' incorporates:
         *  Constant: '<S4>/Constant'
         *  SignalConversion generated from: '<S4>/steerTrqIn'
         */
        DynamicSteer_B.steerTrqIn = DynamicSteer_P.Constant_Value;
      }

      /* End of Outputs for SubSystem: '<S1>/MappedSteer' */
    } break;

    case 1: {
      real_T rtb_Add1_d;
      real_T rtb_Add5_a;
      real_T rtb_Divide;
      real_T rtb_Divide_a;
      real_T rtb_IntegratorSecondOrder_o1;
      real_T rtb_Spd;
      real_T rtb_Switch1_j;
      real_T rtb_Switch1_lv;
      real_T rtb_Switch_h;
      real_T rtb_Switch_j;
      real_T rtb_delta;
      real_T rtb_resp;
      ZCEventType zcEvent;

      /* Outputs for IfAction SubSystem: '<S1>/DynamicSteer' incorporates:
       *  ActionPort: '<S2>/Action Port'
       */
      /* Integrator: '<S17>/Integrator1' */
      rtb_Switch_j = DynamicSteer_X.Integrator1_CSTATE;

      /* Sum: '<S5>/Add' incorporates:
       *  Inport: '<Root>/driverIn'
       *  Integrator: '<S17>/Integrator1'
       *  SignalConversion generated from: '<Root>/driverIn'
       */
      rtb_resp = DynamicSteer_X.Integrator1_CSTATE + DynamicSteer_U.driverIn.steer_request_torque;

      /* Sum: '<S26>/Add' incorporates:
       *  Constant: '<S26>/Constant'
       *  Constant: '<S26>/Constant1'
       */
      rtb_Switch1_j = DynamicSteer_P.DynamicSteering_TrckWdth / 2.0 - DynamicSteer_P.DynamicSteering_RckCsLngth / 2.0;

      /* Product: '<S26>/Divide' incorporates:
       *  Constant: '<S26>/Constant3'
       */
      rtb_Divide = DynamicSteer_P.DynamicSteering_D / rtb_Switch1_j;

      /* Sum: '<S26>/Add1' incorporates:
       *  Constant: '<S26>/Constant2'
       *  Math: '<S26>/Math Function'
       *  Math: '<S26>/Math Function1'
       */
      rtb_Switch1_j =
          rtb_Switch1_j * rtb_Switch1_j + DynamicSteer_P.DynamicSteering_D * DynamicSteer_P.DynamicSteering_D;

      /* Product: '<S26>/Divide1' incorporates:
       *  Constant: '<S26>/Constant4'
       *  Constant: '<S26>/Constant5'
       *  Constant: '<S26>/Constant6'
       *  Constant: '<S26>/Constant7'
       *  Math: '<S26>/Math Function2'
       *  Math: '<S26>/Math Function3'
       *  Sqrt: '<S26>/Sqrt'
       *  Sum: '<S26>/Add2'
       */
      rtb_Switch_h =
          ((DynamicSteer_P.DynamicSteering_StrgArmLngth * DynamicSteer_P.DynamicSteering_StrgArmLngth + rtb_Switch1_j) -
           DynamicSteer_P.DynamicSteering_TieRodLngth * DynamicSteer_P.DynamicSteering_TieRodLngth) /
          DynamicSteer_P.Constant6_Value / DynamicSteer_P.DynamicSteering_StrgArmLngth / std::sqrt(rtb_Switch1_j);

      /* Trigonometry: '<S26>/Trigonometric Function1' */
      if (rtb_Switch_h > 1.0) {
        rtb_Switch_h = 1.0;
      } else if (rtb_Switch_h < -1.0) {
        rtb_Switch_h = -1.0;
      }

      /* Sum: '<S26>/Add3' incorporates:
       *  Constant: '<S26>/Constant8'
       *  Trigonometry: '<S26>/Trigonometric Function'
       *  Trigonometry: '<S26>/Trigonometric Function1'
       */
      rtb_Switch1_j = (DynamicSteer_P.Constant8_Value - std::atan(rtb_Divide)) - std::acos(rtb_Switch_h);

      /* SecondOrderIntegrator: '<S53>/Integrator, Second-Order1' incorporates:
       *  Constant: '<S53>/Constant4'
       *  Constant: '<S53>/Constant5'
       */
      if (DynamicSteer_DW.IntegratorSecondOrder1_DWORK1) {
        DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] = DynamicSteer_P.DynamicSteering_theta_o;
        DynamicSteer_X.IntegratorSecondOrder1_CSTATE[1] = DynamicSteer_P.DynamicSteering_omega_o;
      }

      if (rtsiIsModeUpdateTimeStep(&(&DynamicSteer_M)->solverInfo)) {
        switch (DynamicSteer_DW.IntegratorSecondOrder1_MODE) {
          case 0:
            if (DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] <= -DynamicSteer_P.DynamicSteering_StrgRng) {
              DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] = -DynamicSteer_P.DynamicSteering_StrgRng;
              if (DynamicSteer_X.IntegratorSecondOrder1_CSTATE[1] > 0.0) {
                DynamicSteer_DW.IntegratorSecondOrder1_MODE = 0;
              } else {
                DynamicSteer_X.IntegratorSecondOrder1_CSTATE[1] = 0.0;
                DynamicSteer_DW.IntegratorSecondOrder1_MODE = 1;
              }

              rtsiSetBlockStateForSolverChangedAtMajorStep(&(&DynamicSteer_M)->solverInfo, true);
            }

            if (DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] >= DynamicSteer_P.DynamicSteering_StrgRng) {
              DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] = DynamicSteer_P.DynamicSteering_StrgRng;
              if (DynamicSteer_X.IntegratorSecondOrder1_CSTATE[1] < 0.0) {
                DynamicSteer_DW.IntegratorSecondOrder1_MODE = 0;
              } else {
                DynamicSteer_X.IntegratorSecondOrder1_CSTATE[1] = 0.0;
                DynamicSteer_DW.IntegratorSecondOrder1_MODE = 2;
              }

              rtsiSetBlockStateForSolverChangedAtMajorStep(&(&DynamicSteer_M)->solverInfo, true);
            }
            break;

          case 1:
            if (DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] > -DynamicSteer_P.DynamicSteering_StrgRng) {
              if (DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] >= DynamicSteer_P.DynamicSteering_StrgRng) {
                DynamicSteer_DW.IntegratorSecondOrder1_MODE = 2;
                DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] = DynamicSteer_P.DynamicSteering_StrgRng;
              } else {
                DynamicSteer_DW.IntegratorSecondOrder1_MODE = 0;
              }
            } else {
              DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] = -DynamicSteer_P.DynamicSteering_StrgRng;
              DynamicSteer_X.IntegratorSecondOrder1_CSTATE[1] = 0.0;
            }
            break;

          case 2:
            if (DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] < DynamicSteer_P.DynamicSteering_StrgRng) {
              if (DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] <= -DynamicSteer_P.DynamicSteering_StrgRng) {
                DynamicSteer_DW.IntegratorSecondOrder1_MODE = 1;
                DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] = -DynamicSteer_P.DynamicSteering_StrgRng;
              } else {
                DynamicSteer_DW.IntegratorSecondOrder1_MODE = 0;
              }
            } else {
              DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] = DynamicSteer_P.DynamicSteering_StrgRng;
              DynamicSteer_X.IntegratorSecondOrder1_CSTATE[1] = 0.0;
            }
            break;
        }
      }

      if (DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] < -DynamicSteer_P.DynamicSteering_StrgRng) {
        rtb_delta = -DynamicSteer_P.DynamicSteering_StrgRng;
      } else if (DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0] > DynamicSteer_P.DynamicSteering_StrgRng) {
        rtb_delta = DynamicSteer_P.DynamicSteering_StrgRng;
      } else {
        rtb_delta = DynamicSteer_X.IntegratorSecondOrder1_CSTATE[0];
      }

      rtb_Spd = DynamicSteer_X.IntegratorSecondOrder1_CSTATE[1];

      /* End of SecondOrderIntegrator: '<S53>/Integrator, Second-Order1' */

      /* DeadZone: '<S13>/Dead Zone' */
      if (rtb_delta > DynamicSteer_P.DeadZone_End) {
        rtb_Add1_d = rtb_delta - DynamicSteer_P.DeadZone_End;
      } else if (rtb_delta >= DynamicSteer_P.DeadZone_Start) {
        rtb_Add1_d = 0.0;
      } else {
        rtb_Add1_d = rtb_delta - DynamicSteer_P.DeadZone_Start;
      }

      /* End of DeadZone: '<S13>/Dead Zone' */

      /* Lookup_n-D: '<S21>/1-D Lookup Table' incorporates:
       *  Sum: '<S54>/Add1'
       */
      rtb_Add5_a = look1_binlxpw(rtb_Add1_d, DynamicSteer_P.DynamicSteering_StrgAngBpts,
                                 DynamicSteer_P.DynamicSteering_PnnRadiusTbl, m_pinion_radius_maxIndex);

      /* Sum: '<S25>/Add' incorporates:
       *  Constant: '<S25>/Constant'
       *  Constant: '<S25>/Constant1'
       *  Lookup_n-D: '<S21>/1-D Lookup Table'
       *  Product: '<S21>/Product1'
       */
      rtb_Switch1_lv = (DynamicSteer_P.DynamicSteering_TrckWdth / 2.0 - rtb_Add5_a * rtb_Add1_d) -
                       DynamicSteer_P.DynamicSteering_RckCsLngth / 2.0;

      /* Product: '<S25>/Divide' incorporates:
       *  Constant: '<S25>/Constant3'
       */
      rtb_Divide = DynamicSteer_P.DynamicSteering_D / rtb_Switch1_lv;

      /* Sum: '<S25>/Add1' incorporates:
       *  Constant: '<S25>/Constant2'
       *  Math: '<S25>/Math Function'
       *  Math: '<S25>/Math Function1'
       */
      rtb_Switch1_lv =
          rtb_Switch1_lv * rtb_Switch1_lv + DynamicSteer_P.DynamicSteering_D * DynamicSteer_P.DynamicSteering_D;

      /* Product: '<S25>/Divide1' incorporates:
       *  Constant: '<S25>/Constant4'
       *  Constant: '<S25>/Constant5'
       *  Constant: '<S25>/Constant6'
       *  Constant: '<S25>/Constant7'
       *  Math: '<S25>/Math Function2'
       *  Math: '<S25>/Math Function3'
       *  Sqrt: '<S25>/Sqrt'
       *  Sum: '<S25>/Add2'
       */
      rtb_Switch_h = ((DynamicSteer_P.DynamicSteering_StrgArmLngth * DynamicSteer_P.DynamicSteering_StrgArmLngth +
                       rtb_Switch1_lv) -
                      DynamicSteer_P.DynamicSteering_TieRodLngth * DynamicSteer_P.DynamicSteering_TieRodLngth) /
                     DynamicSteer_P.Constant6_Value_d / DynamicSteer_P.DynamicSteering_StrgArmLngth /
                     std::sqrt(rtb_Switch1_lv);

      /* Trigonometry: '<S25>/Trigonometric Function1' */
      if (rtb_Switch_h > 1.0) {
        rtb_Switch_h = 1.0;
      } else if (rtb_Switch_h < -1.0) {
        rtb_Switch_h = -1.0;
      }

      /* Sum: '<S25>/Add3' incorporates:
       *  Constant: '<S25>/Constant8'
       *  Trigonometry: '<S25>/Trigonometric Function'
       *  Trigonometry: '<S25>/Trigonometric Function1'
       */
      rtb_Switch1_lv = (DynamicSteer_P.Constant8_Value_n - std::atan(rtb_Divide)) - std::acos(rtb_Switch_h);

      /* Switch: '<S10>/Switch' incorporates:
       *  Constant: '<S10>/index'
       *  Sum: '<S21>/Add1'
       *  UnaryMinus: '<S10>/Unary Minus1'
       */
      if (DynamicSteer_P.index_Value > DynamicSteer_P.Switch_Threshold_k) {
        rtb_Divide = rtb_Switch1_j - rtb_Switch1_lv;
      } else {
        rtb_Divide = -(rtb_Switch1_j - rtb_Switch1_lv);
      }

      /* End of Switch: '<S10>/Switch' */

      /* Sum: '<S24>/Add' incorporates:
       *  Constant: '<S24>/Constant'
       *  Constant: '<S24>/Constant1'
       *  Gain: '<S21>/Gain'
       *  Lookup_n-D: '<S21>/1-D Lookup Table'
       *  Product: '<S21>/Product'
       */
      rtb_Add5_a =
          (DynamicSteer_P.DynamicSteering_TrckWdth / 2.0 - DynamicSteer_P.Gain_Gain * rtb_Add5_a * rtb_Add1_d) -
          DynamicSteer_P.DynamicSteering_RckCsLngth / 2.0;

      /* Product: '<S24>/Divide' incorporates:
       *  Constant: '<S24>/Constant3'
       */
      rtb_Divide_a = DynamicSteer_P.DynamicSteering_D / rtb_Add5_a;

      /* Sum: '<S24>/Add1' incorporates:
       *  Constant: '<S24>/Constant2'
       *  Math: '<S24>/Math Function'
       *  Math: '<S24>/Math Function1'
       */
      rtb_Add5_a = rtb_Add5_a * rtb_Add5_a + DynamicSteer_P.DynamicSteering_D * DynamicSteer_P.DynamicSteering_D;

      /* Product: '<S24>/Divide1' incorporates:
       *  Constant: '<S24>/Constant4'
       *  Constant: '<S24>/Constant5'
       *  Constant: '<S24>/Constant6'
       *  Constant: '<S24>/Constant7'
       *  Math: '<S24>/Math Function2'
       *  Math: '<S24>/Math Function3'
       *  Sqrt: '<S24>/Sqrt'
       *  Sum: '<S24>/Add2'
       */
      rtb_Switch_h =
          ((DynamicSteer_P.DynamicSteering_StrgArmLngth * DynamicSteer_P.DynamicSteering_StrgArmLngth + rtb_Add5_a) -
           DynamicSteer_P.DynamicSteering_TieRodLngth * DynamicSteer_P.DynamicSteering_TieRodLngth) /
          DynamicSteer_P.Constant6_Value_o / DynamicSteer_P.DynamicSteering_StrgArmLngth / std::sqrt(rtb_Add5_a);

      /* Trigonometry: '<S24>/Trigonometric Function1' */
      if (rtb_Switch_h > 1.0) {
        rtb_Switch_h = 1.0;
      } else if (rtb_Switch_h < -1.0) {
        rtb_Switch_h = -1.0;
      }

      /* Sum: '<S24>/Add3' incorporates:
       *  Constant: '<S24>/Constant8'
       *  Trigonometry: '<S24>/Trigonometric Function'
       *  Trigonometry: '<S24>/Trigonometric Function1'
       */
      rtb_Add5_a = (DynamicSteer_P.Constant8_Value_c - std::atan(rtb_Divide_a)) - std::acos(rtb_Switch_h);

      /* Switch: '<S10>/Switch1' incorporates:
       *  Constant: '<S10>/index'
       *  Sum: '<S21>/Add'
       *  UnaryMinus: '<S10>/Unary Minus'
       */
      if (DynamicSteer_P.index_Value > DynamicSteer_P.Switch1_Threshold_g) {
        rtb_Switch1_j = rtb_Add5_a - rtb_Switch1_j;
      } else {
        rtb_Switch1_j = -(rtb_Add5_a - rtb_Switch1_j);
      }

      /* End of Switch: '<S10>/Switch1' */

      /* Lookup_n-D: '<S12>/TrqAssistTbl' incorporates:
       *  Math: '<S3>/Hypot'
       *  SignalConversion generated from: '<S2>/<steer_request_torque>'
       */
      rtb_Switch_h = look2_binlcpw(rtb_steer_request_torque, rtb_Hypot, DynamicSteer_P.DynamicSteering_TrqBpts,
                                   DynamicSteer_P.DynamicSteering_VehSpdBpts, DynamicSteer_P.DynamicSteering_TrqTbl,
                                   DynamicSteer_P.TrqAssistTbl_maxIndex, m_power_assist_trqIn_maxIndex + 1);

      /* Saturate: '<S12>/Saturation1' */
      if (rtb_Switch_h > DynamicSteer_P.DynamicSteering_TrqLmt) {
        rtb_Switch_h = DynamicSteer_P.DynamicSteering_TrqLmt;
      } else if (rtb_Switch_h < -DynamicSteer_P.DynamicSteering_TrqLmt) {
        rtb_Switch_h = -DynamicSteer_P.DynamicSteering_TrqLmt;
      }

      /* End of Saturate: '<S12>/Saturation1' */

      /* SecondOrderIntegrator: '<S51>/Integrator, Second-Order' incorporates:
       *  Constant: '<S51>/Constant1'
       *  Constant: '<S51>/Constant2'
       */
      if (DynamicSteer_DW.IntegratorSecondOrder_DWORK1) {
        DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] = DynamicSteer_P.DynamicSteering_theta_o;
        DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] = DynamicSteer_P.DynamicSteering_omega_o;
      }

      if (rtsiIsModeUpdateTimeStep(&(&DynamicSteer_M)->solverInfo)) {
        switch (DynamicSteer_DW.IntegratorSecondOrder_MODE) {
          case 0:
            if (DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] <= -DynamicSteer_P.DynamicSteering_StrgRng) {
              DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] = -DynamicSteer_P.DynamicSteering_StrgRng;
              if (DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] > 0.0) {
                DynamicSteer_DW.IntegratorSecondOrder_MODE = 0;
              } else {
                DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] = 0.0;
                DynamicSteer_DW.IntegratorSecondOrder_MODE = 1;
              }

              rtsiSetBlockStateForSolverChangedAtMajorStep(&(&DynamicSteer_M)->solverInfo, true);
            }

            if (DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] >= DynamicSteer_P.DynamicSteering_StrgRng) {
              DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] = DynamicSteer_P.DynamicSteering_StrgRng;
              if (DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] < 0.0) {
                DynamicSteer_DW.IntegratorSecondOrder_MODE = 0;
              } else {
                DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] = 0.0;
                DynamicSteer_DW.IntegratorSecondOrder_MODE = 2;
              }

              rtsiSetBlockStateForSolverChangedAtMajorStep(&(&DynamicSteer_M)->solverInfo, true);
            }
            break;

          case 1:
            if (DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] > -DynamicSteer_P.DynamicSteering_StrgRng) {
              if (DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] >= DynamicSteer_P.DynamicSteering_StrgRng) {
                DynamicSteer_DW.IntegratorSecondOrder_MODE = 2;
                DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] = DynamicSteer_P.DynamicSteering_StrgRng;
              } else {
                DynamicSteer_DW.IntegratorSecondOrder_MODE = 0;
              }
            } else {
              DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] = -DynamicSteer_P.DynamicSteering_StrgRng;
              DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] = 0.0;
            }
            break;

          case 2:
            if (DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] < DynamicSteer_P.DynamicSteering_StrgRng) {
              if (DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] <= -DynamicSteer_P.DynamicSteering_StrgRng) {
                DynamicSteer_DW.IntegratorSecondOrder_MODE = 1;
                DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] = -DynamicSteer_P.DynamicSteering_StrgRng;
              } else {
                DynamicSteer_DW.IntegratorSecondOrder_MODE = 0;
              }
            } else {
              DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] = DynamicSteer_P.DynamicSteering_StrgRng;
              DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] = 0.0;
            }
            break;
        }
      }

      if (DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] < -DynamicSteer_P.DynamicSteering_StrgRng) {
        rtb_IntegratorSecondOrder_o1 = -DynamicSteer_P.DynamicSteering_StrgRng;
      } else if (DynamicSteer_X.IntegratorSecondOrder_CSTATE[0] > DynamicSteer_P.DynamicSteering_StrgRng) {
        rtb_IntegratorSecondOrder_o1 = DynamicSteer_P.DynamicSteering_StrgRng;
      } else {
        rtb_IntegratorSecondOrder_o1 = DynamicSteer_X.IntegratorSecondOrder_CSTATE[0];
      }

      /* Switch: '<S18>/Switch' incorporates:
       *  Constant: '<S19>/Constant'
       *  Constant: '<S20>/Constant'
       *  Fcn: '<S18>/Fcn'
       *  Logic: '<S18>/Logical Operator'
       *  Product: '<S18>/Product'
       *  RelationalOperator: '<S19>/Compare'
       *  RelationalOperator: '<S20>/Compare'
       *  SecondOrderIntegrator: '<S51>/Integrator, Second-Order'
       *  Switch: '<S18>/Switch1'
       */
      if ((DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] >= -DynamicSteer_P.div0protectpoly_thresh) &&
          (DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] <= DynamicSteer_P.div0protectpoly_thresh)) {
        /* Switch: '<S18>/Switch1' incorporates:
         *  Constant: '<S18>/Constant'
         *  UnaryMinus: '<S18>/Unary Minus'
         */
        if (DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] >= DynamicSteer_P.Switch1_Threshold) {
          rtb_Divide_a = DynamicSteer_P.Constant_Value_e;
        } else {
          rtb_Divide_a = -DynamicSteer_P.Constant_Value_e;
        }

        rtb_Hypot =
            0.02 / (3.0 - rt_powd_snf(DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] / 0.01, 2.0)) * rtb_Divide_a;
      } else {
        rtb_Hypot = DynamicSteer_X.IntegratorSecondOrder_CSTATE[1];
      }

      /* End of Switch: '<S18>/Switch' */

      /* Product: '<S12>/Product' */
      rtb_Switch_h *= rtb_Hypot;

      /* Saturate: '<S12>/Saturation2' */
      if (rtb_Switch_h > DynamicSteer_P.DynamicSteering_PwrLmt) {
        rtb_Switch_h = DynamicSteer_P.DynamicSteering_PwrLmt;
      } else if (rtb_Switch_h < -DynamicSteer_P.DynamicSteering_PwrLmt) {
        rtb_Switch_h = -DynamicSteer_P.DynamicSteering_PwrLmt;
      }

      /* Product: '<S17>/Divide' incorporates:
       *  Constant: '<S17>/Constant'
       *  Gain: '<S12>/Gain1'
       *  Product: '<S12>/Product1'
       *  Saturate: '<S12>/Saturation2'
       *  Sum: '<S17>/Sum'
       */
      DynamicSteer_B.Divide = (rtb_Switch_h / rtb_Hypot * DynamicSteer_P.DynamicSteering_Eta - rtb_Switch_j) *
                              DynamicSteer_P.DynamicSteering_omega_c;

      /* Trigonometry: '<S24>/Trigonometric Function2' incorporates:
       *  Trigonometry: '<S24>/Trigonometric Function7'
       */
      rtb_Divide_a = std::cos(rtb_Add5_a);

      /* Product: '<S24>/Divide2' incorporates:
       *  Constant: '<S24>/Constant10'
       *  Constant: '<S24>/Constant9'
       *  Gain: '<S24>/Gain'
       *  Sum: '<S24>/Add4'
       *  Trigonometry: '<S24>/Trigonometric Function2'
       */
      rtb_Switch_h = (DynamicSteer_P.DynamicSteering_D - DynamicSteer_P.DynamicSteering_StrgArmLngth * rtb_Divide_a) /
                     DynamicSteer_P.DynamicSteering_TieRodLngth;

      /* Trigonometry: '<S24>/Trigonometric Function3' */
      if (rtb_Switch_h > 1.0) {
        rtb_Switch_h = 1.0;
      } else if (rtb_Switch_h < -1.0) {
        rtb_Switch_h = -1.0;
      }

      /* Sum: '<S24>/Add7' incorporates:
       *  Constant: '<S24>/Constant11'
       *  Sum: '<S24>/Add5'
       *  Trigonometry: '<S24>/Trigonometric Function3'
       */
      rtb_Hypot = ((DynamicSteer_P.Constant11_Value - std::asin(rtb_Switch_h)) - rtb_Add5_a) + rtb_Add5_a;

      /* Trigonometry: '<S24>/Trigonometric Function5' */
      rtb_Switch_h = std::sin(rtb_Hypot);

      /* Sum: '<S24>/Add6' incorporates:
       *  Constant: '<S24>/Constant12'
       *  Gain: '<S24>/Gain2'
       *  Product: '<S24>/Divide3'
       *  Product: '<S24>/Divide6'
       *  Trigonometry: '<S24>/Trigonometric Function4'
       *  Trigonometry: '<S24>/Trigonometric Function6'
       */
      rtb_Hypot = std::cos(rtb_Hypot) * DynamicSteer_P.DynamicSteering_StrgArmLngth * std::sin(rtb_Add5_a) -
                  DynamicSteer_P.DynamicSteering_StrgArmLngth * rtb_Switch_h * rtb_Divide_a;

      /* Switch: '<S27>/Switch' incorporates:
       *  Constant: '<S28>/Constant'
       *  Constant: '<S29>/Constant'
       *  Fcn: '<S27>/Fcn'
       *  Logic: '<S27>/Logical Operator'
       *  Product: '<S27>/Product'
       *  RelationalOperator: '<S28>/Compare'
       *  RelationalOperator: '<S29>/Compare'
       *  Switch: '<S27>/Switch1'
       */
      if ((rtb_Hypot >= -DynamicSteer_P.div0protectpoly_thresh_j) &&
          (rtb_Hypot <= DynamicSteer_P.div0protectpoly_thresh_j)) {
        /* Switch: '<S27>/Switch1' incorporates:
         *  Constant: '<S27>/Constant'
         *  UnaryMinus: '<S27>/Unary Minus'
         */
        if (rtb_Hypot >= DynamicSteer_P.Switch1_Threshold_k) {
          rtb_Divide_a = DynamicSteer_P.Constant_Value_l;
        } else {
          rtb_Divide_a = -DynamicSteer_P.Constant_Value_l;
        }

        rtb_Hypot = 0.02 / (3.0 - rt_powd_snf(rtb_Hypot / 0.01, 2.0)) * rtb_Divide_a;
      }

      /* End of Switch: '<S27>/Switch' */

      /* Switch: '<S16>/Switch1' incorporates:
       *  Constant: '<S16>/index'
       *  Inport: '<Root>/wheelOut'
       *  UnaryMinus: '<S16>/Unary Minus1'
       */
      if (DynamicSteer_P.index_Value_c > DynamicSteer_P.Switch1_Threshold_c) {
        rtb_Divide_a = DynamicSteer_U.wheelOut.TireFrame.Mz[1];
      } else {
        rtb_Divide_a = -DynamicSteer_U.wheelOut.TireFrame.Mz[1];
      }

      /* Product: '<S24>/Divide7' incorporates:
       *  Product: '<S24>/Divide5'
       *  Switch: '<S16>/Switch1'
       */
      rtb_Switch_j = rtb_Switch_h / rtb_Hypot * rtb_Divide_a;

      /* Trigonometry: '<S25>/Trigonometric Function2' incorporates:
       *  Trigonometry: '<S25>/Trigonometric Function7'
       */
      rtb_Divide_a = std::cos(rtb_Switch1_lv);

      /* Product: '<S25>/Divide2' incorporates:
       *  Constant: '<S25>/Constant10'
       *  Constant: '<S25>/Constant9'
       *  Gain: '<S25>/Gain'
       *  Sum: '<S25>/Add4'
       *  Trigonometry: '<S25>/Trigonometric Function2'
       */
      rtb_Switch_h = (DynamicSteer_P.DynamicSteering_D - DynamicSteer_P.DynamicSteering_StrgArmLngth * rtb_Divide_a) /
                     DynamicSteer_P.DynamicSteering_TieRodLngth;

      /* Trigonometry: '<S25>/Trigonometric Function3' */
      if (rtb_Switch_h > 1.0) {
        rtb_Switch_h = 1.0;
      } else if (rtb_Switch_h < -1.0) {
        rtb_Switch_h = -1.0;
      }

      /* Sum: '<S25>/Add7' incorporates:
       *  Constant: '<S25>/Constant11'
       *  Sum: '<S25>/Add5'
       *  Trigonometry: '<S25>/Trigonometric Function3'
       */
      rtb_Hypot = ((DynamicSteer_P.Constant11_Value_a - std::asin(rtb_Switch_h)) - rtb_Switch1_lv) + rtb_Switch1_lv;

      /* Trigonometry: '<S25>/Trigonometric Function5' */
      rtb_Switch_h = std::sin(rtb_Hypot);

      /* Sum: '<S25>/Add6' incorporates:
       *  Constant: '<S25>/Constant12'
       *  Gain: '<S25>/Gain2'
       *  Product: '<S25>/Divide3'
       *  Product: '<S25>/Divide6'
       *  Trigonometry: '<S25>/Trigonometric Function4'
       *  Trigonometry: '<S25>/Trigonometric Function6'
       */
      rtb_Hypot = std::cos(rtb_Hypot) * DynamicSteer_P.DynamicSteering_StrgArmLngth * std::sin(rtb_Switch1_lv) -
                  DynamicSteer_P.DynamicSteering_StrgArmLngth * rtb_Switch_h * rtb_Divide_a;

      /* Switch: '<S30>/Switch' incorporates:
       *  Constant: '<S31>/Constant'
       *  Constant: '<S32>/Constant'
       *  Fcn: '<S30>/Fcn'
       *  Logic: '<S30>/Logical Operator'
       *  Product: '<S30>/Product'
       *  RelationalOperator: '<S31>/Compare'
       *  RelationalOperator: '<S32>/Compare'
       *  Switch: '<S30>/Switch1'
       */
      if ((rtb_Hypot >= -DynamicSteer_P.div0protectpoly_thresh_k) &&
          (rtb_Hypot <= DynamicSteer_P.div0protectpoly_thresh_k)) {
        /* Switch: '<S30>/Switch1' incorporates:
         *  Constant: '<S30>/Constant'
         *  UnaryMinus: '<S30>/Unary Minus'
         */
        if (rtb_Hypot >= DynamicSteer_P.Switch1_Threshold_p) {
          rtb_Divide_a = DynamicSteer_P.Constant_Value_j;
        } else {
          rtb_Divide_a = -DynamicSteer_P.Constant_Value_j;
        }

        rtb_Hypot = 0.02 / (3.0 - rt_powd_snf(rtb_Hypot / 0.01, 2.0)) * rtb_Divide_a;
      }

      /* End of Switch: '<S30>/Switch' */

      /* Switch: '<S16>/Switch' incorporates:
       *  Constant: '<S16>/index'
       *  Inport: '<Root>/wheelOut'
       *  UnaryMinus: '<S16>/Unary Minus'
       */
      if (DynamicSteer_P.index_Value_c > DynamicSteer_P.Switch_Threshold_e) {
        rtb_Divide_a = DynamicSteer_U.wheelOut.TireFrame.Mz[0];
      } else {
        rtb_Divide_a = -DynamicSteer_U.wheelOut.TireFrame.Mz[0];
      }

      /* Product: '<S21>/Product4' incorporates:
       *  Gain: '<S21>/Gain1'
       *  Lookup_n-D: '<S21>/1-D Lookup Table1'
       *  Product: '<S25>/Divide5'
       *  Product: '<S25>/Divide7'
       *  Sum: '<S21>/Add2'
       *  Sum: '<S54>/Add1'
       *  Switch: '<S16>/Switch'
       */
      rtb_Hypot = (rtb_Switch_h / rtb_Hypot * rtb_Divide_a + rtb_Switch_j) *
                  (DynamicSteer_P.Gain1_Gain * look1_binlxpw(rtb_Add1_d, DynamicSteer_P.DynamicSteering_StrgAngBpts,
                                                             DynamicSteer_P.DynamicSteering_PnnRadiusTbl,
                                                             m_pinion_radius_maxIndex));

      /* DeadZone: '<S13>/Dead Zone2' */
      if (rtb_Hypot > DynamicSteer_P.DeadZone2_End) {
        rtb_Hypot -= DynamicSteer_P.DeadZone2_End;
      } else if (rtb_Hypot >= DynamicSteer_P.DeadZone2_Start) {
        rtb_Hypot = 0.0;
      } else {
        rtb_Hypot -= DynamicSteer_P.DeadZone2_Start;
      }

      /* End of DeadZone: '<S13>/Dead Zone2' */

      /* Sum: '<S50>/Add' */
      rtb_delta = rtb_IntegratorSecondOrder_o1 - rtb_delta;
      if (rtmIsMajorTimeStep((&DynamicSteer_M))) {
        /* Memory: '<S56>/Memory' */
        DynamicSteer_B.Memory = DynamicSteer_DW.Memory_PreviousInput;
        DynamicSteer_B.domega_o1 = DynamicSteer_P.domega_o1_Value;
      }

      /* Integrator: '<S56>/Integrator' incorporates:
       *  Constant: '<S54>/domega_o1'
       */
      if (rtsiIsModeUpdateTimeStep(&(&DynamicSteer_M)->solverInfo)) {
        zcEvent = rt_ZCFcn(RISING_ZERO_CROSSING, &DynamicSteer_PrevZCX.Integrator_Reset_ZCE, (DynamicSteer_B.Memory));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (DynamicSteer_DW.Integrator_IWORK != 0)) {
          DynamicSteer_X.Integrator_CSTATE = DynamicSteer_B.domega_o1;
        }
      }

      /* Abs: '<S54>/Abs' incorporates:
       *  Integrator: '<S56>/Integrator'
       *  Sum: '<S54>/Add3'
       */
      rtb_Add1_d = std::abs(rtb_delta - DynamicSteer_X.Integrator_CSTATE);

      /* Switch: '<S57>/Switch' incorporates:
       *  Abs: '<S57>/Abs'
       *  Constant: '<S59>/Constant'
       *  Constant: '<S60>/Constant'
       *  Fcn: '<S57>/Fcn'
       *  Logic: '<S57>/Logical Operator'
       *  RelationalOperator: '<S59>/Compare'
       *  RelationalOperator: '<S60>/Compare'
       */
      if ((rtb_Add1_d >= -DynamicSteer_P.div0protectabspoly_thresh) &&
          (rtb_Add1_d <= DynamicSteer_P.div0protectabspoly_thresh)) {
        rtb_Switch1_lv = 0.002 / (3.0 - rt_powd_snf(rtb_Add1_d / 0.001, 2.0));
      } else {
        rtb_Switch1_lv = rtb_Add1_d;
      }

      /* End of Switch: '<S57>/Switch' */

      /* UnaryMinus: '<S54>/Unary Minus' */
      rtb_Add1_d = -rtb_Switch1_lv;

      /* Switch: '<S54>/Switch' incorporates:
       *  Constant: '<S54>/Constant'
       *  Constant: '<S54>/Constant1'
       */
      if (rtb_delta > DynamicSteer_P.Switch_Threshold_a) {
        rtb_Switch1_lv = DynamicSteer_P.DynamicSteering_beta_u;
      } else {
        rtb_Switch1_lv = DynamicSteer_P.DynamicSteering_beta_l;
      }

      /* End of Switch: '<S54>/Switch' */

      /* Switch: '<S58>/Switch' incorporates:
       *  Constant: '<S61>/Constant'
       *  Constant: '<S62>/Constant'
       *  Fcn: '<S58>/Fcn'
       *  Logic: '<S58>/Logical Operator'
       *  Product: '<S58>/Product'
       *  RelationalOperator: '<S61>/Compare'
       *  RelationalOperator: '<S62>/Compare'
       *  Switch: '<S58>/Switch1'
       */
      if ((rtb_Switch1_lv >= -DynamicSteer_P.div0protectpoly_thresh_i) &&
          (rtb_Switch1_lv <= DynamicSteer_P.div0protectpoly_thresh_i)) {
        /* Switch: '<S58>/Switch1' incorporates:
         *  Constant: '<S58>/Constant'
         *  UnaryMinus: '<S58>/Unary Minus'
         */
        if (rtb_Switch1_lv >= DynamicSteer_P.Switch1_Threshold_f) {
          rtb_Divide_a = DynamicSteer_P.Constant_Value_o;
        } else {
          rtb_Divide_a = -DynamicSteer_P.Constant_Value_o;
        }

        rtb_Switch1_lv = 2.0E-6 / (3.0 - rt_powd_snf(rtb_Switch1_lv / 1.0e-6, 2.0)) * rtb_Divide_a;
      }

      /* End of Switch: '<S58>/Switch' */
      if (rtmIsMajorTimeStep((&DynamicSteer_M))) {
        /* Memory: '<S55>/Memory' */
        DynamicSteer_B.Memory_f = DynamicSteer_DW.Memory_PreviousInput_c;
        DynamicSteer_B.domega_o = DynamicSteer_P.domega_o_Value;
      }

      /* Integrator: '<S55>/Integrator' incorporates:
       *  Constant: '<S54>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&DynamicSteer_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &DynamicSteer_PrevZCX.Integrator_Reset_ZCE_g, (DynamicSteer_B.Memory_f));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (DynamicSteer_DW.Integrator_IWORK_o != 0)) {
          DynamicSteer_X.Integrator_CSTATE_o = DynamicSteer_B.domega_o;
        }
      }

      /* Sum: '<S54>/Add5' incorporates:
       *  Gain: '<S54>/b'
       *  Gain: '<S54>/k'
       *  SecondOrderIntegrator: '<S51>/Integrator, Second-Order'
       *  Sum: '<S50>/Add2'
       */
      rtb_Add5_a = (DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] - rtb_Spd) * DynamicSteer_P.DynamicSteering_b1 +
                   DynamicSteer_P.DynamicSteering_k1 * rtb_delta;

      /* Sum: '<S54>/Add1' incorporates:
       *  Integrator: '<S55>/Integrator'
       *  Math: '<S54>/Math Function'
       *  Product: '<S54>/Divide'
       *  Product: '<S54>/Divide1'
       *  Sum: '<S54>/Add2'
       *
       * About '<S54>/Math Function':
       *  Operator: exp
       */
      rtb_Add1_d =
          std::exp(rtb_Add1_d / rtb_Switch1_lv) * (DynamicSteer_X.Integrator_CSTATE_o - rtb_Add5_a) + rtb_Add5_a;

      /* Switch: '<S52>/Switch' incorporates:
       *  Abs: '<S52>/Abs'
       *  Constant: '<S66>/Constant'
       *  Gain: '<S52>/Gain'
       *  Gain: '<S52>/Gain1'
       *  Product: '<S52>/Product'
       *  RelationalOperator: '<S66>/Compare'
       *  Signum: '<S52>/Sign'
       *  UnaryMinus: '<S52>/Unary Minus'
       */
      if (std::abs(rtb_Spd) >= DynamicSteer_P.CompareToConstant_const) {
        /* Signum: '<S52>/Sign' */
        if (std::isnan(rtb_Spd)) {
          rtb_Divide_a = (rtNaN);
        } else if (rtb_Spd < 0.0) {
          rtb_Divide_a = -1.0;
        } else {
          rtb_Divide_a = (rtb_Spd > 0.0);
        }

        rtb_Switch_j = -rtb_Divide_a * DynamicSteer_P.DynamicSteering_FricTrq;
      } else {
        rtb_Switch_j = -DynamicSteer_P.DynamicSteering_FricTrq / 0.01 * rtb_Spd;
      }

      /* End of Switch: '<S52>/Switch' */

      /* Sum: '<S15>/Add4' incorporates:
       *  Gain: '<S15>/Gain1'
       */
      rtb_Spd = ((rtb_Add1_d + rtb_Hypot) + rtb_Switch_j) + -DynamicSteer_P.DynamicSteering_b3 * rtb_Spd;

      /* Switch: '<S55>/Switch' incorporates:
       *  Integrator: '<S55>/Integrator'
       */
      if (DynamicSteer_B.Memory_f != 0.0) {
        rtb_Divide_a = DynamicSteer_X.Integrator_CSTATE_o;
      } else {
        rtb_Divide_a = DynamicSteer_B.domega_o;
      }

      /* Product: '<S55>/Product' incorporates:
       *  Constant: '<S54>/omega_c'
       *  Sum: '<S55>/Sum'
       *  Switch: '<S55>/Switch'
       */
      DynamicSteer_B.Product = (rtb_Add5_a - rtb_Divide_a) * DynamicSteer_P.omega_c_Value;

      /* Switch: '<S56>/Switch' incorporates:
       *  Integrator: '<S56>/Integrator'
       */
      if (DynamicSteer_B.Memory != 0.0) {
        rtb_Divide_a = DynamicSteer_X.Integrator_CSTATE;
      } else {
        rtb_Divide_a = DynamicSteer_B.domega_o1;
      }

      /* Product: '<S56>/Product' incorporates:
       *  Constant: '<S54>/omega_c1'
       *  Sum: '<S56>/Sum'
       *  Switch: '<S56>/Switch'
       */
      DynamicSteer_B.Product_j = (rtb_delta - rtb_Divide_a) * DynamicSteer_P.omega_c1_Value;

      /* Switch: '<S63>/Switch' incorporates:
       *  Abs: '<S63>/Abs'
       *  Constant: '<S51>/Constant'
       *  Constant: '<S64>/Constant'
       *  Constant: '<S65>/Constant'
       *  Fcn: '<S63>/Fcn'
       *  Logic: '<S63>/Logical Operator'
       *  RelationalOperator: '<S64>/Compare'
       *  RelationalOperator: '<S65>/Compare'
       */
      if ((DynamicSteer_P.DynamicSteering_J1 >= -DynamicSteer_P.div0protectabspoly_thresh_o) &&
          (DynamicSteer_P.DynamicSteering_J1 <= DynamicSteer_P.div0protectabspoly_thresh_o)) {
        rtb_Hypot = 2.0E-6 / (3.0 - rt_powd_snf(DynamicSteer_P.DynamicSteering_J1 / 1.0e-6, 2.0));
      } else {
        rtb_Hypot = std::abs(DynamicSteer_P.DynamicSteering_J1);
      }

      /* End of Switch: '<S63>/Switch' */

      /* Product: '<S51>/Divide' incorporates:
       *  Gain: '<S15>/Gain2'
       *  SecondOrderIntegrator: '<S51>/Integrator, Second-Order'
       *  Sum: '<S15>/Add'
       */
      DynamicSteer_B.Divide_i =
          ((-DynamicSteer_P.DynamicSteering_b2 * DynamicSteer_X.IntegratorSecondOrder_CSTATE[1] + rtb_resp) -
           rtb_Add1_d) /
          rtb_Hypot;

      /* Switch: '<S67>/Switch' incorporates:
       *  Abs: '<S67>/Abs'
       *  Constant: '<S53>/Constant3'
       *  Constant: '<S68>/Constant'
       *  Constant: '<S69>/Constant'
       *  Fcn: '<S67>/Fcn'
       *  Logic: '<S67>/Logical Operator'
       *  RelationalOperator: '<S68>/Compare'
       *  RelationalOperator: '<S69>/Compare'
       */
      if ((DynamicSteer_P.DynamicSteering_J2 >= -DynamicSteer_P.div0protectabspoly1_thresh) &&
          (DynamicSteer_P.DynamicSteering_J2 <= DynamicSteer_P.div0protectabspoly1_thresh)) {
        rtb_Hypot = 2.0E-6 / (3.0 - rt_powd_snf(DynamicSteer_P.DynamicSteering_J2 / 1.0e-6, 2.0));
      } else {
        rtb_Hypot = std::abs(DynamicSteer_P.DynamicSteering_J2);
      }

      /* End of Switch: '<S67>/Switch' */

      /* Product: '<S53>/Divide1' */
      DynamicSteer_B.Divide1 = rtb_Spd / rtb_Hypot;

      /* Merge: '<S1>/Merge' incorporates:
       *  Constant: '<S2>/Constant1'
       *  SignalConversion generated from: '<S2>/WhlAng'
       */
      DynamicSteer_B.WhlAng[0] = rtb_Divide;
      DynamicSteer_B.WhlAng[1] = rtb_Switch1_j;
      DynamicSteer_B.WhlAng[2] = DynamicSteer_P.Constant1_Value_i;
      DynamicSteer_B.WhlAng[3] = DynamicSteer_P.Constant1_Value_i;

      /* Merge: '<S1>/Merge2' incorporates:
       *  SignalConversion generated from: '<S2>/steerAngleFeedback[rad]'
       */
      DynamicSteer_B.steerAngleFeedback = rtb_IntegratorSecondOrder_o1;

      /* Merge: '<S1>/Merge3' incorporates:
       *  SignalConversion generated from: '<S2>/<steer_request_torque>'
       *  SignalConversion generated from: '<S2>/steerTrqIn'
       */
      DynamicSteer_B.steerTrqIn = rtb_steer_request_torque;

      /* End of Outputs for SubSystem: '<S1>/DynamicSteer' */
    } break;
  }

  /* End of If: '<S1>/If' */
  if (rtmIsMajorTimeStep((&DynamicSteer_M))) {
    if (rtmIsMajorTimeStep((&DynamicSteer_M))) {
      /* Update for Memory generated from: '<S1>/Memory' */
      DynamicSteer_DW.Memory_1_PreviousInput[0] = DynamicSteer_B.WhlAng[0];
      DynamicSteer_DW.Memory_1_PreviousInput[1] = DynamicSteer_B.WhlAng[1];
      DynamicSteer_DW.Memory_1_PreviousInput[2] = DynamicSteer_B.WhlAng[2];
      DynamicSteer_DW.Memory_1_PreviousInput[3] = DynamicSteer_B.WhlAng[3];

      /* Update for Memory generated from: '<S1>/Memory' */
      DynamicSteer_DW.Memory_2_PreviousInput = DynamicSteer_B.steerAngleFeedback;

      /* Update for Memory generated from: '<S1>/Memory' */
      DynamicSteer_DW.Memory_3_PreviousInput = DynamicSteer_B.steerTrqIn;
    }

    /* Update for If: '<S1>/If' */
    if (DynamicSteer_DW.If_ActiveSubsystem == 1) {
      int32_T uMode;

      /* Update for IfAction SubSystem: '<S1>/DynamicSteer' incorporates:
       *  ActionPort: '<S2>/Action Port'
       */
      /* Update for SecondOrderIntegrator: '<S53>/Integrator, Second-Order1' */
      DynamicSteer_DW.IntegratorSecondOrder1_DWORK1 = false;
      uMode = DynamicSteer_DW.IntegratorSecondOrder1_MODE;
      if (((DynamicSteer_DW.IntegratorSecondOrder1_MODE == 1) && (DynamicSteer_B.Divide1 > 0.0)) ||
          ((DynamicSteer_DW.IntegratorSecondOrder1_MODE == 2) && (DynamicSteer_B.Divide1 < 0.0))) {
        uMode = 0;
      }

      if (DynamicSteer_DW.IntegratorSecondOrder1_MODE != uMode) {
        DynamicSteer_DW.IntegratorSecondOrder1_MODE = uMode;
        rtsiSetBlockStateForSolverChangedAtMajorStep(&(&DynamicSteer_M)->solverInfo, true);
      }

      /* End of Update for SecondOrderIntegrator: '<S53>/Integrator, Second-Order1' */

      /* Update for SecondOrderIntegrator: '<S51>/Integrator, Second-Order' */
      DynamicSteer_DW.IntegratorSecondOrder_DWORK1 = false;
      uMode = DynamicSteer_DW.IntegratorSecondOrder_MODE;
      if (((DynamicSteer_DW.IntegratorSecondOrder_MODE == 1) && (DynamicSteer_B.Divide_i > 0.0)) ||
          ((DynamicSteer_DW.IntegratorSecondOrder_MODE == 2) && (DynamicSteer_B.Divide_i < 0.0))) {
        uMode = 0;
      }

      if (DynamicSteer_DW.IntegratorSecondOrder_MODE != uMode) {
        DynamicSteer_DW.IntegratorSecondOrder_MODE = uMode;
        rtsiSetBlockStateForSolverChangedAtMajorStep(&(&DynamicSteer_M)->solverInfo, true);
      }

      /* End of Update for SecondOrderIntegrator: '<S51>/Integrator, Second-Order' */
      if (rtmIsMajorTimeStep((&DynamicSteer_M))) {
        /* Update for Memory: '<S56>/Memory' incorporates:
         *  Constant: '<S56>/Reset'
         */
        DynamicSteer_DW.Memory_PreviousInput = DynamicSteer_P.Reset_Value_b;

        /* Update for Memory: '<S55>/Memory' incorporates:
         *  Constant: '<S55>/Reset'
         */
        DynamicSteer_DW.Memory_PreviousInput_c = DynamicSteer_P.Reset_Value;
      }

      /* Update for Integrator: '<S56>/Integrator' */
      DynamicSteer_DW.Integrator_IWORK = 0;

      /* Update for Integrator: '<S55>/Integrator' */
      DynamicSteer_DW.Integrator_IWORK_o = 0;

      /* End of Update for SubSystem: '<S1>/DynamicSteer' */
    }

    /* End of Update for If: '<S1>/If' */
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&DynamicSteer_M))) {
    rt_ertODEUpdateContinuousStates(&(&DynamicSteer_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&DynamicSteer_M)->Timing.clockTick0)) {
      ++(&DynamicSteer_M)->Timing.clockTickH0;
    }

    (&DynamicSteer_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&DynamicSteer_M)->solverInfo);

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
      (&DynamicSteer_M)->Timing.clockTick1++;
      if (!(&DynamicSteer_M)->Timing.clockTick1) {
        (&DynamicSteer_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void DynamicSteer::DynamicSteer_derivatives() {
  XDot_DynamicSteer_T *_rtXdot;
  _rtXdot = ((XDot_DynamicSteer_T *)(&DynamicSteer_M)->derivs);

  /* Derivatives for If: '<S1>/If' */
  {
    real_T *dx;
    int_T i;
    dx = &(((XDot_DynamicSteer_T *)(&DynamicSteer_M)->derivs)->TransferFcn_CSTATE[0]);
    for (i = 0; i < 2; i++) {
      dx[i] = 0.0;
    }
  }

  {
    real_T *dx;
    int_T i;
    dx = &(((XDot_DynamicSteer_T *)(&DynamicSteer_M)->derivs)->Integrator1_CSTATE);
    for (i = 0; i < 7; i++) {
      dx[i] = 0.0;
    }
  }

  switch (DynamicSteer_DW.If_ActiveSubsystem) {
    case 0:
      /* Derivatives for IfAction SubSystem: '<S1>/MappedSteer' incorporates:
       *  ActionPort: '<S4>/Action Port'
       */
      /* Derivatives for TransferFcn: '<S76>/Transfer Fcn' */
      _rtXdot->TransferFcn_CSTATE[0] = 0.0;
      _rtXdot->TransferFcn_CSTATE[0] += DynamicSteer_P.TransferFcn_A[0] * DynamicSteer_X.TransferFcn_CSTATE[0];
      _rtXdot->TransferFcn_CSTATE[1] = 0.0;
      _rtXdot->TransferFcn_CSTATE[0] += DynamicSteer_P.TransferFcn_A[1] * DynamicSteer_X.TransferFcn_CSTATE[1];
      _rtXdot->TransferFcn_CSTATE[1] += DynamicSteer_X.TransferFcn_CSTATE[0];
      _rtXdot->TransferFcn_CSTATE[0] += DynamicSteer_B.steer_cmd_rad;

      /* End of Derivatives for SubSystem: '<S1>/MappedSteer' */
      break;

    case 1:
      /* Derivatives for IfAction SubSystem: '<S1>/DynamicSteer' incorporates:
       *  ActionPort: '<S2>/Action Port'
       */
      /* Derivatives for Integrator: '<S17>/Integrator1' */
      _rtXdot->Integrator1_CSTATE = DynamicSteer_B.Divide;

      /* Derivatives for SecondOrderIntegrator: '<S53>/Integrator, Second-Order1' */
      switch (DynamicSteer_DW.IntegratorSecondOrder1_MODE) {
        case 0:
          _rtXdot->IntegratorSecondOrder1_CSTATE[0] = DynamicSteer_X.IntegratorSecondOrder1_CSTATE[1];
          _rtXdot->IntegratorSecondOrder1_CSTATE[1] = DynamicSteer_B.Divide1;
          break;

        case 1:
          _rtXdot->IntegratorSecondOrder1_CSTATE[0] = 0.0;
          _rtXdot->IntegratorSecondOrder1_CSTATE[1] = 0.0;
          break;

        case 2:
          _rtXdot->IntegratorSecondOrder1_CSTATE[0] = 0.0;
          _rtXdot->IntegratorSecondOrder1_CSTATE[1] = 0.0;
          break;
      }

      /* End of Derivatives for SecondOrderIntegrator: '<S53>/Integrator, Second-Order1' */

      /* Derivatives for SecondOrderIntegrator: '<S51>/Integrator, Second-Order' */
      switch (DynamicSteer_DW.IntegratorSecondOrder_MODE) {
        case 0:
          _rtXdot->IntegratorSecondOrder_CSTATE[0] = DynamicSteer_X.IntegratorSecondOrder_CSTATE[1];
          _rtXdot->IntegratorSecondOrder_CSTATE[1] = DynamicSteer_B.Divide_i;
          break;

        case 1:
          _rtXdot->IntegratorSecondOrder_CSTATE[0] = 0.0;
          _rtXdot->IntegratorSecondOrder_CSTATE[1] = 0.0;
          break;

        case 2:
          _rtXdot->IntegratorSecondOrder_CSTATE[0] = 0.0;
          _rtXdot->IntegratorSecondOrder_CSTATE[1] = 0.0;
          break;
      }

      /* End of Derivatives for SecondOrderIntegrator: '<S51>/Integrator, Second-Order' */

      /* Derivatives for Integrator: '<S56>/Integrator' */
      _rtXdot->Integrator_CSTATE = DynamicSteer_B.Product_j;

      /* Derivatives for Integrator: '<S55>/Integrator' */
      _rtXdot->Integrator_CSTATE_o = DynamicSteer_B.Product;

      /* End of Derivatives for SubSystem: '<S1>/DynamicSteer' */
      break;
  }

  /* End of Derivatives for If: '<S1>/If' */
}

/* Model initialize function */
void DynamicSteer::initialize() {
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&DynamicSteer_M)->solverInfo, &(&DynamicSteer_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&DynamicSteer_M)->solverInfo, &rtmGetTPtr((&DynamicSteer_M)));
    rtsiSetStepSizePtr(&(&DynamicSteer_M)->solverInfo, &(&DynamicSteer_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&DynamicSteer_M)->solverInfo, &(&DynamicSteer_M)->derivs);
    rtsiSetContStatesPtr(&(&DynamicSteer_M)->solverInfo, (real_T **)&(&DynamicSteer_M)->contStates);
    rtsiSetNumContStatesPtr(&(&DynamicSteer_M)->solverInfo, &(&DynamicSteer_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&DynamicSteer_M)->solverInfo, &(&DynamicSteer_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&DynamicSteer_M)->solverInfo, &(&DynamicSteer_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&DynamicSteer_M)->solverInfo, &(&DynamicSteer_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&DynamicSteer_M)->solverInfo, (&rtmGetErrorStatus((&DynamicSteer_M))));
    rtsiSetRTModelPtr(&(&DynamicSteer_M)->solverInfo, (&DynamicSteer_M));
  }

  rtsiSetSimTimeStep(&(&DynamicSteer_M)->solverInfo, MAJOR_TIME_STEP);
  (&DynamicSteer_M)->intgData.y = (&DynamicSteer_M)->odeY;
  (&DynamicSteer_M)->intgData.f[0] = (&DynamicSteer_M)->odeF[0];
  (&DynamicSteer_M)->intgData.f[1] = (&DynamicSteer_M)->odeF[1];
  (&DynamicSteer_M)->intgData.f[2] = (&DynamicSteer_M)->odeF[2];
  (&DynamicSteer_M)->intgData.f[3] = (&DynamicSteer_M)->odeF[3];
  (&DynamicSteer_M)->contStates = ((X_DynamicSteer_T *)&DynamicSteer_X);
  rtsiSetSolverData(&(&DynamicSteer_M)->solverInfo, static_cast<void *>(&(&DynamicSteer_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&DynamicSteer_M)->solverInfo, false);
  rtsiSetSolverName(&(&DynamicSteer_M)->solverInfo, "ode4");
  rtmSetTPtr((&DynamicSteer_M), &(&DynamicSteer_M)->Timing.tArray[0]);
  (&DynamicSteer_M)->Timing.stepSize0 = 0.001;
  rtmSetFirstInitCond((&DynamicSteer_M), 1);

  /* Start for If: '<S1>/If' */
  DynamicSteer_DW.If_ActiveSubsystem = -1;
  DynamicSteer_PrevZCX.Integrator_Reset_ZCE = UNINITIALIZED_ZCSIG;
  DynamicSteer_PrevZCX.Integrator_Reset_ZCE_g = UNINITIALIZED_ZCSIG;

  /* InitializeConditions for Memory generated from: '<S1>/Memory' */
  DynamicSteer_DW.Memory_1_PreviousInput[0] = DynamicSteer_P.Memory_1_InitialCondition;
  DynamicSteer_DW.Memory_1_PreviousInput[1] = DynamicSteer_P.Memory_1_InitialCondition;
  DynamicSteer_DW.Memory_1_PreviousInput[2] = DynamicSteer_P.Memory_1_InitialCondition;
  DynamicSteer_DW.Memory_1_PreviousInput[3] = DynamicSteer_P.Memory_1_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S1>/Memory' */
  DynamicSteer_DW.Memory_2_PreviousInput = DynamicSteer_P.Memory_2_InitialCondition;

  /* InitializeConditions for Memory generated from: '<S1>/Memory' */
  DynamicSteer_DW.Memory_3_PreviousInput = DynamicSteer_P.Memory_3_InitialCondition;

  /* SystemInitialize for IfAction SubSystem: '<S1>/MappedSteer' */
  /* InitializeConditions for TransferFcn: '<S76>/Transfer Fcn' */
  DynamicSteer_X.TransferFcn_CSTATE[0] = 0.0;
  DynamicSteer_X.TransferFcn_CSTATE[1] = 0.0;

  /* End of SystemInitialize for SubSystem: '<S1>/MappedSteer' */

  /* SystemInitialize for IfAction SubSystem: '<S1>/DynamicSteer' */
  /* Start for Constant: '<S54>/domega_o1' */
  DynamicSteer_B.domega_o1 = DynamicSteer_P.domega_o1_Value;

  /* Start for Constant: '<S54>/domega_o' */
  DynamicSteer_B.domega_o = DynamicSteer_P.domega_o_Value;

  /* InitializeConditions for Integrator: '<S17>/Integrator1' */
  DynamicSteer_X.Integrator1_CSTATE = DynamicSteer_P.Integrator1_IC;

  /* InitializeConditions for SecondOrderIntegrator: '<S53>/Integrator, Second-Order1' */
  DynamicSteer_DW.IntegratorSecondOrder1_DWORK1 = true;

  /* InitializeConditions for SecondOrderIntegrator: '<S51>/Integrator, Second-Order' */
  DynamicSteer_DW.IntegratorSecondOrder_DWORK1 = true;

  /* InitializeConditions for Memory: '<S56>/Memory' */
  DynamicSteer_DW.Memory_PreviousInput = DynamicSteer_P.Memory_InitialCondition;

  /* InitializeConditions for Integrator: '<S56>/Integrator' incorporates:
   *  Integrator: '<S55>/Integrator'
   */
  if (rtmIsFirstInitCond((&DynamicSteer_M))) {
    DynamicSteer_X.Integrator_CSTATE = 0.0;
    DynamicSteer_X.Integrator_CSTATE_o = 0.0;
  }

  DynamicSteer_DW.Integrator_IWORK = 1;

  /* End of InitializeConditions for Integrator: '<S56>/Integrator' */

  /* InitializeConditions for Memory: '<S55>/Memory' */
  DynamicSteer_DW.Memory_PreviousInput_c = DynamicSteer_P.Memory_InitialCondition_g;

  /* InitializeConditions for Integrator: '<S55>/Integrator' */
  DynamicSteer_DW.Integrator_IWORK_o = 1;

  /* End of SystemInitialize for SubSystem: '<S1>/DynamicSteer' */

  /* set "at time zero" to false */
  if (rtmIsFirstInitCond((&DynamicSteer_M))) {
    rtmSetFirstInitCond((&DynamicSteer_M), 0);
  }
}

/* Model terminate function */
void DynamicSteer::terminate() { /* (no terminate code required) */
}

/* Constructor */
DynamicSteer::DynamicSteer()
    : DynamicSteer_U(),
      DynamicSteer_Y(),
      DynamicSteer_B(),
      DynamicSteer_DW(),
      DynamicSteer_X(),
      DynamicSteer_PrevZCX(),
      DynamicSteer_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
DynamicSteer::~DynamicSteer() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_DynamicSteer_T *DynamicSteer::getRTM() { return (&DynamicSteer_M); }
