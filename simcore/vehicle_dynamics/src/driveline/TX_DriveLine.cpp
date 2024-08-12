/*
 * TX_DriveLine.cpp
 *
 * Code generation for model "TX_DriveLine".
 *
 * Model version              : 1.189
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Thu May 11 17:31:43 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TX_DriveLine.h"
#include <emmintrin.h>
#include <cmath>
#include "TX_DriveLine_private.h"
#include "rtwtypes.h"
#include "zero_crossing_types.h"

extern "C" {

#include "rt_nonfinite.h"
}

/* Named constants for Chart: '<S98>/hold' */
const uint8_T TX_DriveLine_IN_Hold{1U};

const uint8_T TX_DriveLine_IN_NO_ACTIVE_CHILD{0U};

const uint8_T TX_DriveLine_IN_Normal{2U};

/* Named constants for Chart: '<S377>/TCM Shift Controller' */
const uint8_T TX_DriveLine_IN_DownShifting{1U};

const uint8_T TX_DriveLine_IN_G1{1U};

const uint8_T TX_DriveLine_IN_G2{2U};

const uint8_T TX_DriveLine_IN_G3{3U};

const uint8_T TX_DriveLine_IN_G4{4U};

const uint8_T TX_DriveLine_IN_G5{5U};

const uint8_T TX_DriveLine_IN_G6{6U};

const uint8_T TX_DriveLine_IN_G7{7U};

const uint8_T TX_DriveLine_IN_G8{8U};

const uint8_T TX_DriveLine_IN_Neutral{9U};

const uint8_T TX_DriveLine_IN_R{10U};

const uint8_T TX_DriveLine_IN_SteadyState{2U};

const uint8_T TX_DriveLine_IN_UpShifting{3U};

const uint8_T TX_DriveLine_IN_preDownShifting{4U};

const uint8_T TX_DriveLine_IN_preUpShifting{5U};

const int32_T TX_DriveLine_event_Down{0};

const int32_T TX_DriveLine_event_Up{1};

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void TX_DriveLine::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  int_T nXc{55};

  rtsiSetSimTimeStep(si, MINOR_TIME_STEP);

  /* Save the state values at time t in y, we'll use x as ynew. */
  (void)std::memcpy(y, x, static_cast<uint_T>(nXc) * sizeof(real_T));

  /* Assumes that rtsiSetT and ModelOutputs are up-to-date */
  /* f0 = f(t,y) */
  rtsiSetdX(si, f0);
  TX_DriveLine_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  TX_DriveLine_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  TX_DriveLine_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  TX_DriveLine_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Function for MATLAB Function: '<S108>/Limited Slip Differential' */
void TX_DriveLine::TX_DriveLine_automldiffls(const real_T u[4], real_T bw1, real_T bd, real_T bw2, real_T Ndiff,
                                             real_T shaftSwitch, real_T Jd, real_T Jw1, real_T Jw2, const real_T x[2],
                                             real_T y[4], real_T xdot[2]) {
  real_T diffDir_0[8];
  real_T diffDir_1[8];
  real_T invJ_0[8];
  real_T invJ[4];
  real_T invJ_1[4];
  real_T invJ_2[2];
  real_T invJ_3[2];
  real_T NbdTerm;
  real_T NbdTerm_tmp;
  real_T term1;
  real_T term2;
  int32_T diffDir;
  NbdTerm_tmp = Ndiff * Ndiff;
  NbdTerm = NbdTerm_tmp * bd / 4.0;
  if (shaftSwitch != 0.0) {
    diffDir = -1;
  } else {
    diffDir = 1;
  }

  term1 = NbdTerm_tmp * Jd;
  term2 = (4.0 * Jw1 * Jw2 + term1 * Jw1) + Jw2 * term1;
  invJ[0] = (Jw2 * 4.0 + term1) / term2;
  NbdTerm_tmp = -term1 / term2;
  invJ[2] = NbdTerm_tmp;
  invJ[1] = NbdTerm_tmp;
  invJ[3] = (Jw1 * 4.0 + term1) / term2;
  NbdTerm_tmp = static_cast<real_T>(diffDir) / 2.0 * Ndiff;
  diffDir_0[0] = NbdTerm_tmp;
  diffDir_0[2] = -1.0;
  diffDir_0[4] = 0.0;
  diffDir_0[6] = -0.5;
  diffDir_0[1] = NbdTerm_tmp;
  diffDir_0[3] = 0.0;
  diffDir_0[5] = -1.0;
  diffDir_0[7] = 0.5;
  term1 = -(bw1 + NbdTerm);
  term2 = -(NbdTerm + bw2);
  diffDir_1[0] = NbdTerm_tmp;
  diffDir_1[4] = NbdTerm_tmp;
  for (diffDir = 0; diffDir < 2; diffDir++) {
    int32_T diffDir_tmp;
    for (diffDir_tmp = 0; diffDir_tmp < 4; diffDir_tmp++) {
      int32_T invJ_tmp;
      int32_T invJ_tmp_tmp;
      invJ_tmp_tmp = diffDir_tmp << 1;
      invJ_tmp = diffDir + invJ_tmp_tmp;
      invJ_0[invJ_tmp] = 0.0;
      invJ_0[invJ_tmp] += diffDir_0[invJ_tmp_tmp] * invJ[diffDir];
      invJ_0[invJ_tmp] += diffDir_0[invJ_tmp_tmp + 1] * invJ[diffDir + 2];
    }

    invJ_1[diffDir] = 0.0;
    invJ_1[diffDir] += invJ[diffDir] * term1;
    NbdTerm_tmp = invJ[diffDir + 2];
    invJ_1[diffDir] += NbdTerm_tmp * -NbdTerm;
    invJ_1[diffDir + 2] = 0.0;
    invJ_1[diffDir + 2] += invJ[diffDir] * -NbdTerm;
    invJ_1[diffDir + 2] += NbdTerm_tmp * term2;
    invJ_2[diffDir] = 0.0;
    invJ_2[diffDir] += invJ_0[diffDir] * u[0];
    invJ_2[diffDir] += invJ_0[diffDir + 2] * u[1];
    invJ_2[diffDir] += invJ_0[diffDir + 4] * u[2];
    invJ_2[diffDir] += invJ_0[diffDir + 6] * u[3];
    invJ_3[diffDir] = 0.0;
    invJ_3[diffDir] += invJ_1[diffDir] * x[0];
    invJ_3[diffDir] += invJ_1[diffDir + 2] * x[1];
    xdot[diffDir] = invJ_2[diffDir] + invJ_3[diffDir];
    diffDir_tmp = diffDir << 2;
    diffDir_1[diffDir_tmp + 1] = static_cast<real_T>(diffDir) - 1.0;
    diffDir_1[diffDir_tmp + 2] = 0.0 - static_cast<real_T>(diffDir);
    diffDir_1[diffDir_tmp + 3] = 0.5 - static_cast<real_T>(diffDir);
  }

  for (diffDir = 0; diffDir <= 2; diffDir += 2) {
    __m128d tmp;
    __m128d tmp_0;
    tmp = _mm_loadu_pd(&diffDir_1[diffDir]);
    tmp_0 = _mm_loadu_pd(&diffDir_1[diffDir + 4]);
    _mm_storeu_pd(&y[diffDir], _mm_add_pd(_mm_add_pd(_mm_mul_pd(tmp_0, _mm_set1_pd(x[1])),
                                                     _mm_add_pd(_mm_mul_pd(tmp, _mm_set1_pd(x[0])), _mm_set1_pd(0.0))),
                                          _mm_set1_pd(0.0 * u[3] + (0.0 * u[2] + (0.0 * u[1] + 0.0 * u[0])))));
  }
}

/*
 * Output and update for atomic system:
 *    '<S108>/Limited Slip Differential'
 *    '<S159>/Limited Slip Differential'
 *    '<S185>/Limited Slip Differential'
 *    '<S284>/Limited Slip Differential'
 *    '<S10>/Limited Slip Differential'
 *    '<S55>/Limited Slip Differential'
 */
void TX_DriveLine::TX_DriveLine_LimitedSlipDifferential(real_T rtu_u, real_T rtu_u_n, real_T rtu_u_o, real_T rtu_u_h,
                                                        real_T rtu_bw1, real_T rtu_bd, real_T rtu_bw2, real_T rtu_Ndiff,
                                                        real_T rtu_Jd, real_T rtu_Jw1, real_T rtu_Jw2,
                                                        const real_T rtu_x[2],
                                                        B_LimitedSlipDifferential_TX_DriveLine_T *localB,
                                                        P_LimitedSlipDifferential_TX_DriveLine_T *localP) {
  real_T a__2[4];
  real_T rtu_u_0[4];
  if (localP->LimitedSlipDifferential_shaftSwitchMask == 1.0) {
    /* SignalConversion generated from: '<S113>/ SFunction ' */
    rtu_u_0[0] = rtu_u;
    rtu_u_0[1] = rtu_u_n;
    rtu_u_0[2] = rtu_u_o;
    rtu_u_0[3] = rtu_u_h;
    TX_DriveLine_automldiffls(rtu_u_0, rtu_bw1, rtu_bd, rtu_bw2, rtu_Ndiff, 1.0, rtu_Jd, rtu_Jw1, rtu_Jw2, rtu_x, a__2,
                              localB->xdot);
  } else {
    /* SignalConversion generated from: '<S113>/ SFunction ' */
    rtu_u_0[0] = rtu_u;
    rtu_u_0[1] = rtu_u_n;
    rtu_u_0[2] = rtu_u_o;
    rtu_u_0[3] = rtu_u_h;
    TX_DriveLine_automldiffls(rtu_u_0, rtu_bw1, rtu_bd, rtu_bw2, rtu_Ndiff, 0.0, rtu_Jd, rtu_Jw1, rtu_Jw2, rtu_x, a__2,
                              localB->xdot);
  }
}

/* Function for Chart: '<S377>/TCM Shift Controller' */
void TX_DriveLine::TX_DriveLine_GearCmd(void) {
  switch (TX_DriveLine_DW.is_GearCmd) {
    case TX_DriveLine_IN_G1:
      if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Up) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G2;
        TX_DriveLine_B.GearCmd = 2;
      } else if (TX_DriveLine_B.LogicalOperator1) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_Neutral;
        TX_DriveLine_B.GearCmd = 0;
      }
      break;

    case TX_DriveLine_IN_G2:
      if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Up) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G3;
        TX_DriveLine_B.GearCmd = 3;
      } else if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Down) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G1;
        TX_DriveLine_B.GearCmd = 1;
      }
      break;

    case TX_DriveLine_IN_G3:
      if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Up) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G4;
        TX_DriveLine_B.GearCmd = 4;
      } else if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Down) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G2;
        TX_DriveLine_B.GearCmd = 2;
      }
      break;

    case TX_DriveLine_IN_G4:
      if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Up) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G5;
        TX_DriveLine_B.GearCmd = 5;
      } else if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Down) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G3;
        TX_DriveLine_B.GearCmd = 3;
      }
      break;

    case TX_DriveLine_IN_G5:
      if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Up) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G6;
        TX_DriveLine_B.GearCmd = 6;
      } else if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Down) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G4;
        TX_DriveLine_B.GearCmd = 4;
      }
      break;

    case TX_DriveLine_IN_G6:
      if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Up) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G7;
        TX_DriveLine_B.GearCmd = 7;
      } else if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Down) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G5;
        TX_DriveLine_B.GearCmd = 5;
      }
      break;

    case TX_DriveLine_IN_G7:
      if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Up) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G8;
        TX_DriveLine_B.GearCmd = 8;
      } else if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Down) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G6;
        TX_DriveLine_B.GearCmd = 6;
      }
      break;

    case TX_DriveLine_IN_G8:
      if (TX_DriveLine_DW.sfEvent == TX_DriveLine_event_Down) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G7;
        TX_DriveLine_B.GearCmd = 7;
      }
      break;

    case TX_DriveLine_IN_Neutral:
      if (TX_DriveLine_B.gearMode) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_G1;
        TX_DriveLine_B.GearCmd = 1;
      } else if (TX_DriveLine_B.gearMode_c) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_R;
        TX_DriveLine_B.GearCmd = -1;
      }
      break;

    case TX_DriveLine_IN_R:
      if (TX_DriveLine_B.LogicalOperator1) {
        TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_Neutral;
        TX_DriveLine_B.GearCmd = 0;
      }
      break;

    default:
      /* Unreachable state, for coverage only */
      TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_NO_ACTIVE_CHILD;
      break;
  }
}

/* Model step function */
void TX_DriveLine::step() {
  /* local block i/o variables */
  real_T rtb_Integrator[2];
  real_T rtb_Integrator_c[2];
  real_T rtb_Product4;
  real_T rtb_Product4_n;
  real_T rtb_Product4_p;
  real_T rtb_Integrator_c4;
  real_T rtb_Product4_h;
  real_T rtb_Product4_o;
  real_T rtb_Product4_l;
  real_T rtb_Integrator_o;
  real_T rtb_Integrator_cd[2];
  real_T rtb_Integrator_l[2];
  real_T rtb_Product4_hr;
  real_T rtb_Product4_ht;
  real_T rtb_Product4_i;
  real_T rtb_Integrator_b;
  real_T rtb_Integrator_f[2];
  real_T rtb_Product4_j;
  real_T rtb_Product4_g;
  real_T rtb_Product4_nd;
  real_T rtb_Integrator_ce;
  real_T rtb_Integrator_p[2];
  real_T rtb_Product4_pl;
  real_T rtb_Product4_a;
  real_T rtb_Product4_ha;
  real_T rtb_Integrator_m;
  real_T rtb_Product4_go;
  real_T rtb_Product4_po;
  real_T rtb_Product4_ni;
  real_T rtb_Integrator_m0;
  real_T rtb_VectorConcatenate_c[6];
  real_T tmp[6];
  real_T rtb_VectorConcatenate_c_0[4];
  real_T rtb_VectorConcatenate_h[4];
  real_T rtb_VectorConcatenate_c_1[2];
  real_T rtb_VectorConcatenate_c_2[2];
  real_T rtb_Abs_e;
  real_T rtb_Product2_d;
  int8_T rtAction;
  int8_T rtPrevAction;
  if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
    /* set solver stop time */
    if (!((&TX_DriveLine_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&TX_DriveLine_M)->solverInfo, (((&TX_DriveLine_M)->Timing.clockTickH0 + 1) *
                                                             (&TX_DriveLine_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(
          &(&TX_DriveLine_M)->solverInfo,
          (((&TX_DriveLine_M)->Timing.clockTick0 + 1) * (&TX_DriveLine_M)->Timing.stepSize0 +
           (&TX_DriveLine_M)->Timing.clockTickH0 * (&TX_DriveLine_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&TX_DriveLine_M))) {
    (&TX_DriveLine_M)->Timing.t[0] = rtsiGetT(&(&TX_DriveLine_M)->solverInfo);
  }

  /* SwitchCase: '<S1>/Switch Case' incorporates:
   *  Concatenate: '<S108>/Vector Concatenate'
   *  Concatenate: '<S10>/Vector Concatenate'
   *  Concatenate: '<S158>/Vector Concatenate'
   *  Concatenate: '<S159>/Vector Concatenate'
   *  Concatenate: '<S185>/Vector Concatenate'
   *  Concatenate: '<S284>/Vector Concatenate'
   *  Concatenate: '<S55>/Vector Concatenate'
   *  Constant: '<S108>/Constant'
   *  Constant: '<S108>/Constant1'
   *  Constant: '<S10>/Constant'
   *  Constant: '<S10>/Constant1'
   *  Constant: '<S134>/domega_o'
   *  Constant: '<S142>/domega_o'
   *  Constant: '<S158>/Constant'
   *  Constant: '<S158>/Constant1'
   *  Constant: '<S159>/Constant'
   *  Constant: '<S159>/Constant1'
   *  Constant: '<S185>/Constant'
   *  Constant: '<S185>/Constant1'
   *  Constant: '<S1>/PowerType[1-ICE;2-MOT]'
   *  Constant: '<S211>/domega_o'
   *  Constant: '<S219>/domega_o'
   *  Constant: '<S227>/domega_o'
   *  Constant: '<S235>/domega_o'
   *  Constant: '<S243>/domega_o'
   *  Constant: '<S251>/domega_o'
   *  Constant: '<S284>/Constant'
   *  Constant: '<S284>/Constant1'
   *  Constant: '<S310>/domega_o'
   *  Constant: '<S318>/domega_o'
   *  Constant: '<S331>/Constant1'
   *  Constant: '<S369>/domega_o'
   *  Constant: '<S36>/domega_o'
   *  Constant: '<S44>/domega_o'
   *  Constant: '<S55>/Constant'
   *  Constant: '<S55>/Constant1'
   *  Constant: '<S81>/domega_o'
   *  Constant: '<S89>/domega_o'
   */
  rtPrevAction = TX_DriveLine_DW.SwitchCase_ActiveSubsystem;
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
    switch (TX_DriveLine_P.driveline_pow_type) {
      case 1:
        rtAction = 0;
        break;

      case 2:
        rtAction = 1;
        break;

      default:
        rtAction = 2;
        break;
    }

    TX_DriveLine_DW.SwitchCase_ActiveSubsystem = rtAction;
  } else {
    rtAction = TX_DriveLine_DW.SwitchCase_ActiveSubsystem;
  }

  if ((rtPrevAction != rtAction) && (rtPrevAction == 0)) {
    /* Disable for If: '<S331>/If' */
    TX_DriveLine_DW.If_ActiveSubsystem = -1;
  }

  switch (rtAction) {
    case 0: {
      real_T rtb_Add1;
      real_T rtb_Gain_p;
      real_T rtb_Integrator_k3;
      real_T rtb_Product1_n;
      real_T rtb_Product6;
      real_T rtb_Product_gq;
      real_T rtb_Subtract1_b0;
      real_T rtb_Subtract1_bs;
      real_T rtb_Subtract1_dg;
      real_T rtb_Subtract1_hs;
      real_T rtb_Subtract1_iv;
      real_T rtb_Subtract1_k;
      real_T rtb_Subtract1_lh;
      real_T rtb_Sum_dv;
      real_T rtb_SumofElements;
      real_T rtb_SumofElements_c;
      real_T rtb_SumofElements_f;
      real_T rtb_SumofElements_l3;
      real_T rtb_Switch;
      real_T rtb_VectorConcatenate_c_tmp;
      int32_T i;
      ZCEventType zcEvent;

      /* Outputs for IfAction SubSystem: '<S1>/ICEDriveLine' incorporates:
       *  ActionPort: '<S3>/Action Port'
       */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S333>/Memory' */
        TX_DriveLine_B.Memory_l = TX_DriveLine_DW.Memory_PreviousInput_e;
        TX_DriveLine_B.Constant1 = TX_DriveLine_P.IdealFixedGearTransmission_G_o;
      }

      /* Integrator: '<S333>/Integrator' incorporates:
       *  Constant: '<S331>/Constant1'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_m, (TX_DriveLine_B.Memory_l));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_c != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_f = TX_DriveLine_B.Constant1;
        }
      }

      rtb_Integrator_k3 = TX_DriveLine_X.Integrator_CSTATE_f;

      /* Rounding: '<S331>/Round' incorporates:
       *  Integrator: '<S333>/Integrator'
       */
      rtb_SumofElements_f = std::round(TX_DriveLine_X.Integrator_CSTATE_f);

      /* DeadZone: '<S331>/Dead Zone' incorporates:
       *  Integrator: '<S333>/Integrator'
       */
      if (TX_DriveLine_X.Integrator_CSTATE_f > TX_DriveLine_P.DeadZone_End) {
        /* DeadZone: '<S331>/Dead Zone' */
        TX_DriveLine_B.DeadZone = TX_DriveLine_X.Integrator_CSTATE_f - TX_DriveLine_P.DeadZone_End;
      } else if (TX_DriveLine_X.Integrator_CSTATE_f >= TX_DriveLine_P.DeadZone_Start) {
        /* DeadZone: '<S331>/Dead Zone' */
        TX_DriveLine_B.DeadZone = 0.0;
      } else {
        /* DeadZone: '<S331>/Dead Zone' */
        TX_DriveLine_B.DeadZone = TX_DriveLine_X.Integrator_CSTATE_f - TX_DriveLine_P.DeadZone_Start;
      }

      /* End of DeadZone: '<S331>/Dead Zone' */

      /* Switch: '<S101>/Switch' incorporates:
       *  Abs: '<S101>/Abs'
       *  Constant: '<S101>/Constant'
       *  Constant: '<S391>/Constant'
       *  Constant: '<S392>/Constant'
       *  Gain: '<S101>/speed'
       *  Inport: '<Root>/DriveLineIn'
       *  Logic: '<S101>/lowSpeedAndNegativeTorque'
       *  RelationalOperator: '<S391>/Compare'
       *  RelationalOperator: '<S392>/Compare'
       *  Sum: '<S101>/Sum of Elements'
       */
      if ((TX_DriveLine_U.DriveLineIn.engine_trq_Nm < TX_DriveLine_P.Constant_Value_i) &&
          ((((std::abs(TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[0]) +
              std::abs(TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[1])) +
             std::abs(TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[2])) +
            std::abs(TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[3])) *
               (TX_DriveLine_P.StatLdWhlR[0] / 4.0) <
           TX_DriveLine_P.CompareToConstant_const)) {
        rtb_Switch = TX_DriveLine_P.Constant_Value_n;
      } else {
        rtb_Switch = TX_DriveLine_U.DriveLineIn.engine_trq_Nm;
      }

      /* End of Switch: '<S101>/Switch' */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S370>/Memory' */
        TX_DriveLine_B.Memory_e = TX_DriveLine_DW.Memory_PreviousInput_i;
        TX_DriveLine_B.domega_o_g = TX_DriveLine_P.PropShaft_domega_o;
      }

      /* Integrator: '<S370>/Integrator' incorporates:
       *  Constant: '<S369>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_e, (TX_DriveLine_B.Memory_e));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_m != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_h = TX_DriveLine_B.domega_o_g;
        }
      }

      rtb_SumofElements = TX_DriveLine_X.Integrator_CSTATE_h;

      /* Sum: '<S369>/Subtract1' incorporates:
       *  Gain: '<S369>/Gain1'
       *  Gain: '<S369>/Gain2'
       *  Integrator: '<S369>/Integrator'
       *  Integrator: '<S370>/Integrator'
       */
      rtb_Subtract1_b0 = TX_DriveLine_P.PropShaft_b * TX_DriveLine_X.Integrator_CSTATE_h +
                         TX_DriveLine_P.PropShaft_k * TX_DriveLine_X.Integrator_CSTATE_c;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* InitialCondition: '<S338>/IC' */
        if (TX_DriveLine_DW.IC_FirstOutputTime) {
          TX_DriveLine_DW.IC_FirstOutputTime = false;

          /* InitialCondition: '<S338>/IC' */
          TX_DriveLine_B.IC = TX_DriveLine_P.IC_Value;
        } else {
          /* InitialCondition: '<S338>/IC' incorporates:
           *  Constant: '<S338>/Constant1'
           */
          TX_DriveLine_B.IC = TX_DriveLine_P.Constant1_Value_m;
        }

        /* End of InitialCondition: '<S338>/IC' */

        /* InitialCondition: '<S336>/IC' */
        if (TX_DriveLine_DW.IC_FirstOutputTime_e) {
          TX_DriveLine_DW.IC_FirstOutputTime_e = false;

          /* InitialCondition: '<S336>/IC' */
          TX_DriveLine_B.IC_a = TX_DriveLine_P.IC_Value_j;
        } else {
          /* InitialCondition: '<S336>/IC' incorporates:
           *  Constant: '<S336>/Constant1'
           */
          TX_DriveLine_B.IC_a = TX_DriveLine_P.Constant1_Value_n;
        }

        /* End of InitialCondition: '<S336>/IC' */
      }

      /* Switch: '<S338>/Switch' */
      if (TX_DriveLine_B.IC > TX_DriveLine_P.Switch_Threshold_h) {
        /* Switch: '<S338>/Switch' */
        TX_DriveLine_B.Switch = TX_DriveLine_X.w;
      } else {
        /* Switch: '<S338>/Switch' incorporates:
         *  Constant: '<S338>/Constant'
         */
        TX_DriveLine_B.Switch = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0] *
                                (TX_DriveLine_P.ratio_diff_front + TX_DriveLine_P.ratio_diff_rear) / 2.0;
      }

      /* End of Switch: '<S338>/Switch' */

      /* Switch: '<S336>/Switch' */
      if (TX_DriveLine_B.IC_a > TX_DriveLine_P.Switch_Threshold_d) {
        /* Switch: '<S336>/Switch' */
        TX_DriveLine_B.Switch_j = TX_DriveLine_X.w;
      } else {
        /* Switch: '<S336>/Switch' incorporates:
         *  Constant: '<S336>/Constant'
         */
        TX_DriveLine_B.Switch_j = TX_DriveLine_P.EngIdleSpd / 60.0 * 2.0 * 3.1415926;
      }

      /* End of Switch: '<S336>/Switch' */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* InitialCondition: '<S334>/IC' */
        if (TX_DriveLine_DW.IC_FirstOutputTime_f) {
          TX_DriveLine_DW.IC_FirstOutputTime_f = false;

          /* InitialCondition: '<S334>/IC' */
          TX_DriveLine_B.IC_m = TX_DriveLine_P.IC_Value_o;
        } else {
          /* InitialCondition: '<S334>/IC' incorporates:
           *  Constant: '<S334>/Constant1'
           */
          TX_DriveLine_B.IC_m = TX_DriveLine_P.Constant1_Value_fx;
        }

        /* End of InitialCondition: '<S334>/IC' */
      }

      /* Switch: '<S334>/Switch' */
      if (TX_DriveLine_B.IC_m > TX_DriveLine_P.Switch_Threshold_a) {
        /* Switch: '<S334>/Switch' */
        TX_DriveLine_B.Switch_d = TX_DriveLine_X.we;
      } else {
        /* Switch: '<S334>/Switch' incorporates:
         *  Constant: '<S334>/Constant'
         */
        TX_DriveLine_B.Switch_d = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0] *
                                  (TX_DriveLine_P.ratio_diff_front + TX_DriveLine_P.ratio_diff_rear) / 2.0;
      }

      /* End of Switch: '<S334>/Switch' */

      /* If: '<S331>/If' */
      rtPrevAction = TX_DriveLine_DW.If_ActiveSubsystem;
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        rtAction = static_cast<int8_T>(!(TX_DriveLine_B.DeadZone != 0.0));
        TX_DriveLine_DW.If_ActiveSubsystem = rtAction;
      } else {
        rtAction = TX_DriveLine_DW.If_ActiveSubsystem;
      }

      switch (rtAction) {
        case 0:
          if (rtAction != rtPrevAction) {
            /* InitializeConditions for IfAction SubSystem: '<S331>/Locked' incorporates:
             *  ActionPort: '<S335>/Action'
             */
            /* InitializeConditions for If: '<S331>/If' incorporates:
             *  Integrator: '<S335>/x'
             */
            if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
              TX_DriveLine_X.w = 0.0;
            }

            TX_DriveLine_DW.x_IWORK = 1;

            /* End of InitializeConditions for SubSystem: '<S331>/Locked' */
          }

          /* Outputs for IfAction SubSystem: '<S331>/Locked' incorporates:
           *  ActionPort: '<S335>/Action'
           */
          /* Lookup_n-D: '<S349>/Gear2Ratios' incorporates:
           *  Integrator: '<S333>/Integrator'
           */
          rtb_Product2_d = look1_binlxpw(TX_DriveLine_X.Integrator_CSTATE_f, TX_DriveLine_P.Trans.GearNums,
                                         TX_DriveLine_P.Trans.GearRatios, 8U);

          /* Abs: '<S335>/Abs' */
          rtb_Abs_e = std::abs(rtb_Product2_d);

          /* Lookup_n-D: '<S358>/Eta 1D' incorporates:
           *  Integrator: '<S333>/Integrator'
           */
          rtb_Product1_n = look1_binlcpw(TX_DriveLine_X.Integrator_CSTATE_f, TX_DriveLine_P.Trans.GearNums,
                                         TX_DriveLine_P.Trans.Efficiencies, 8U);

          /* Integrator: '<S335>/x' */
          if (TX_DriveLine_DW.x_IWORK != 0) {
            TX_DriveLine_X.w = TX_DriveLine_B.Switch_d;
            rtsiSetContTimeOutputInconsistentWithStateAtMajorStep(&(&TX_DriveLine_M)->solverInfo, true);
          }

          /* Abs: '<S350>/Abs' incorporates:
           *  Abs: '<S352>/Abs'
           *  Integrator: '<S335>/x'
           */
          rtb_Product6 = std::abs(TX_DriveLine_X.w);

          /* Gain: '<S351>/Gain' incorporates:
           *  Abs: '<S350>/Abs'
           *  Constant: '<S351>/Constant1'
           *  Gain: '<S351>/Gain1'
           *  Gain: '<S351>/Gain2'
           *  Sum: '<S351>/Subtract1'
           *  Sum: '<S351>/Subtract2'
           *  Trigonometry: '<S351>/Trigonometric Function'
           */
          rtb_Sum_dv = (std::tanh((TX_DriveLine_P.Gain1_Gain * rtb_Product6 - TX_DriveLine_P.Constant1_Value) *
                                  TX_DriveLine_P.Gain2_Gain) +
                        TX_DriveLine_P.Constant1_Value) *
                       TX_DriveLine_P.Gain_Gain;

          /* Product: '<S335>/Product5' incorporates:
           *  UnaryMinus: '<S369>/Unary Minus'
           */
          rtb_Product_gq = -rtb_Subtract1_b0 / rtb_Product2_d;

          /* Gain: '<S353>/Gain' incorporates:
           *  Constant: '<S353>/Constant1'
           *  Gain: '<S353>/Gain1'
           *  Gain: '<S353>/Gain2'
           *  Sum: '<S353>/Subtract1'
           *  Sum: '<S353>/Subtract2'
           *  Trigonometry: '<S353>/Trigonometric Function'
           */
          rtb_Product6 = (std::tanh((TX_DriveLine_P.Gain1_Gain_d * rtb_Product6 - TX_DriveLine_P.Constant1_Value_e) *
                                    TX_DriveLine_P.Gain2_Gain_e) +
                          TX_DriveLine_P.Constant1_Value_e) *
                         TX_DriveLine_P.Gain_Gain_f;

          /* Switch: '<S346>/Switch' incorporates:
           *  Constant: '<S346>/Constant'
           *  Integrator: '<S335>/x'
           *  Product: '<S346>/Product1'
           */
          if (rtb_Switch * TX_DriveLine_X.w > TX_DriveLine_P.Switch_Threshold) {
            rtb_VectorConcatenate_c_tmp = rtb_Product1_n;
          } else {
            rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value;
          }

          /* Switch: '<S347>/Switch' incorporates:
           *  Constant: '<S347>/Constant'
           *  Integrator: '<S335>/x'
           *  Product: '<S347>/Product1'
           */
          if (!(rtb_Product_gq * TX_DriveLine_X.w > TX_DriveLine_P.Switch_Threshold_p)) {
            rtb_Product1_n = TX_DriveLine_P.Constant_Value_kg;
          }

          /* Product: '<S335>/Product8' incorporates:
           *  Constant: '<S350>/Constant'
           *  Constant: '<S351>/Constant1'
           *  Constant: '<S352>/Constant'
           *  Constant: '<S353>/Constant1'
           *  Integrator: '<S333>/Integrator'
           *  Integrator: '<S335>/x'
           *  Lookup_n-D: '<S349>/Gear2damping'
           *  Lookup_n-D: '<S349>/Gear2inertias'
           *  Product: '<S335>/Product1'
           *  Product: '<S335>/Product3'
           *  Product: '<S335>/Product6'
           *  Product: '<S346>/Product4'
           *  Product: '<S347>/Product4'
           *  Product: '<S350>/Product3'
           *  Product: '<S350>/Product5'
           *  Product: '<S352>/Product3'
           *  Product: '<S352>/Product5'
           *  Sum: '<S335>/Sum'
           *  Sum: '<S335>/Sum1'
           *  Sum: '<S350>/Subtract1'
           *  Sum: '<S351>/Subtract'
           *  Sum: '<S352>/Subtract1'
           *  Sum: '<S353>/Subtract'
           *  Switch: '<S346>/Switch'
           *  Switch: '<S347>/Switch'
           */
          TX_DriveLine_B.Product8_i =
              ((((TX_DriveLine_P.Constant1_Value - rtb_Sum_dv) * TX_DriveLine_P.Constant_Value_k +
                 rtb_VectorConcatenate_c_tmp * rtb_Sum_dv) *
                    rtb_Switch +
                ((TX_DriveLine_P.Constant1_Value_e - rtb_Product6) * TX_DriveLine_P.Constant_Value_o +
                 rtb_Product1_n * rtb_Product6) *
                    rtb_Product_gq) -
               1.0 / rtb_Abs_e / rtb_Abs_e *
                   look1_binlxpw(rtb_Integrator_k3, TX_DriveLine_P.Trans.GearNums, TX_DriveLine_P.Trans.DampingCoeffs,
                                 8U) *
                   TX_DriveLine_X.w) *
              (1.0 /
               (look1_binlxpw(rtb_Integrator_k3, TX_DriveLine_P.Trans.GearNums, TX_DriveLine_P.Trans.Inertias, 8U) /
                rtb_Abs_e / rtb_Abs_e));

          /* SignalConversion: '<S335>/Signal Conversion2' incorporates:
           *  Integrator: '<S335>/x'
           */
          rtb_Abs_e = TX_DriveLine_X.w;

          /* SignalConversion: '<S335>/Signal Conversion3' incorporates:
           *  Integrator: '<S335>/x'
           *  Product: '<S335>/Product2'
           */
          rtb_Product2_d = TX_DriveLine_X.w / rtb_Product2_d;

          /* End of Outputs for SubSystem: '<S331>/Locked' */
          break;

        case 1: {
          uint32_T Product4_tmp;
          if (rtAction != rtPrevAction) {
            /* InitializeConditions for IfAction SubSystem: '<S331>/Unlocked' incorporates:
             *  ActionPort: '<S337>/Action'
             */
            /* InitializeConditions for If: '<S331>/If' incorporates:
             *  Integrator: '<S337>/xe'
             *  Integrator: '<S337>/xv'
             */
            if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
              TX_DriveLine_X.we = 0.0;
              TX_DriveLine_X.wv = 0.0;
            }

            TX_DriveLine_DW.xe_IWORK = 1;
            TX_DriveLine_DW.xv_IWORK = 1;

            /* End of InitializeConditions for SubSystem: '<S331>/Unlocked' */
          }

          /* Outputs for IfAction SubSystem: '<S331>/Unlocked' incorporates:
           *  ActionPort: '<S337>/Action'
           */
          /* Lookup_n-D: '<S361>/Gear2Ratios' incorporates:
           *  Integrator: '<S333>/Integrator'
           */
          rtb_Product2_d = look1_binlxpw(TX_DriveLine_X.Integrator_CSTATE_f, TX_DriveLine_P.Trans.GearNums,
                                         TX_DriveLine_P.Trans.GearRatios, 8U);

          /* Abs: '<S337>/Abs' */
          rtb_Abs_e = std::abs(rtb_Product2_d);

          /* Integrator: '<S337>/xe' */
          if (TX_DriveLine_DW.xe_IWORK != 0) {
            TX_DriveLine_X.we = TX_DriveLine_B.Switch;
            rtsiSetContTimeOutputInconsistentWithStateAtMajorStep(&(&TX_DriveLine_M)->solverInfo, true);
          }

          /* Integrator: '<S337>/xv' */
          if (TX_DriveLine_DW.xv_IWORK != 0) {
            TX_DriveLine_X.wv = TX_DriveLine_B.Switch_j;
          }

          /* Product: '<S337>/Product5' incorporates:
           *  UnaryMinus: '<S369>/Unary Minus'
           */
          rtb_Product1_n = -rtb_Subtract1_b0 / rtb_Product2_d;

          /* Switch: '<S359>/Switch' incorporates:
           *  Constant: '<S359>/Constant'
           *  Integrator: '<S333>/Integrator'
           *  Integrator: '<S337>/xv'
           *  Lookup_n-D: '<S368>/Eta 1D'
           *  Product: '<S359>/Product1'
           */
          if (rtb_Product1_n * TX_DriveLine_X.wv > TX_DriveLine_P.Switch_Threshold_b) {
            rtb_Sum_dv =
                look1_binlcpw(rtb_Integrator_k3, TX_DriveLine_P.Trans.GearNums, TX_DriveLine_P.Trans.Efficiencies, 8U);
          } else {
            rtb_Sum_dv = TX_DriveLine_P.Constant_Value_a;
          }

          /* End of Switch: '<S359>/Switch' */

          /* Gain: '<S363>/Gain' incorporates:
           *  Abs: '<S362>/Abs'
           *  Constant: '<S363>/Constant1'
           *  Gain: '<S363>/Gain1'
           *  Gain: '<S363>/Gain2'
           *  Integrator: '<S337>/xv'
           *  Sum: '<S363>/Subtract1'
           *  Sum: '<S363>/Subtract2'
           *  Trigonometry: '<S363>/Trigonometric Function'
           */
          rtb_Product_gq = (std::tanh((TX_DriveLine_P.Gain1_Gain_f * std::abs(TX_DriveLine_X.wv) -
                                       TX_DriveLine_P.Constant1_Value_f) *
                                      TX_DriveLine_P.Gain2_Gain_l) +
                            TX_DriveLine_P.Constant1_Value_f) *
                           TX_DriveLine_P.Gain_Gain_j;

          /* Lookup_n-D: '<S337>/Gear2inertias' incorporates:
           *  Constant: '<S337>/Neutral'
           *  Lookup_n-D: '<S337>/Gear2damping'
           */
          Product4_tmp = plook_u32d_binckan(TX_DriveLine_P.Neutral_Value, TX_DriveLine_P.Trans.GearNums, 8U);

          /* Product: '<S337>/Product4' incorporates:
           *  Constant: '<S337>/First'
           *  Integrator: '<S337>/xe'
           *  Lookup_n-D: '<S337>/Gear2damping'
           *  Lookup_n-D: '<S337>/Gear2inertias'
           *  Lookup_n-D: '<S337>/Gear2inertias1'
           *  Product: '<S337>/Product7'
           *  Sum: '<S337>/Subtract'
           *  Sum: '<S337>/Sum1'
           */
          TX_DriveLine_B.Product4 =
              1.0 /
              (TX_DriveLine_P.Trans
                   .Inertias[plook_u32d_binckan(TX_DriveLine_P.First_Value, TX_DriveLine_P.Trans.GearNums, 8U)] -
               TX_DriveLine_P.Trans.Inertias[Product4_tmp]) *
              (rtb_Switch - TX_DriveLine_P.Trans.DampingCoeffs[Product4_tmp] * TX_DriveLine_X.we);

          /* Product: '<S337>/Product8' incorporates:
           *  Constant: '<S362>/Constant'
           *  Constant: '<S363>/Constant1'
           *  Integrator: '<S333>/Integrator'
           *  Integrator: '<S337>/xv'
           *  Lookup_n-D: '<S361>/Gear2damping'
           *  Lookup_n-D: '<S361>/Gear2inertias'
           *  Product: '<S337>/Product1'
           *  Product: '<S337>/Product3'
           *  Product: '<S337>/Product6'
           *  Product: '<S359>/Product4'
           *  Product: '<S362>/Product3'
           *  Product: '<S362>/Product5'
           *  Sum: '<S337>/Sum'
           *  Sum: '<S362>/Subtract1'
           *  Sum: '<S363>/Subtract'
           */
          TX_DriveLine_B.Product8 =
              (((TX_DriveLine_P.Constant1_Value_f - rtb_Product_gq) * TX_DriveLine_P.Constant_Value_h +
                rtb_Sum_dv * rtb_Product_gq) *
                   rtb_Product1_n -
               1.0 / rtb_Abs_e / rtb_Abs_e *
                   look1_binlxpw(rtb_Integrator_k3, TX_DriveLine_P.Trans.GearNums, TX_DriveLine_P.Trans.DampingCoeffs,
                                 8U) *
                   TX_DriveLine_X.wv) *
              (1.0 /
               (look1_binlxpw(rtb_Integrator_k3, TX_DriveLine_P.Trans.GearNums, TX_DriveLine_P.Trans.Inertias, 8U) /
                rtb_Abs_e / rtb_Abs_e));

          /* SignalConversion: '<S337>/Signal Conversion' incorporates:
           *  Integrator: '<S337>/xv'
           *  Product: '<S337>/Product2'
           */
          rtb_Product2_d = TX_DriveLine_X.wv / rtb_Product2_d;

          /* SignalConversion: '<S337>/Signal Conversion1' incorporates:
           *  Integrator: '<S337>/xe'
           */
          rtb_Abs_e = TX_DriveLine_X.we;

          /* End of Outputs for SubSystem: '<S331>/Unlocked' */
        } break;
      }

      /* End of If: '<S331>/If' */

      /* Saturate: '<S328>/Saturation' */
      if (rtb_Abs_e > TX_DriveLine_P.Saturation_UpperSat_m) {
        rtb_Abs_e = TX_DriveLine_P.Saturation_UpperSat_m;
      } else if (rtb_Abs_e < TX_DriveLine_P.Saturation_LowerSat_a) {
        rtb_Abs_e = TX_DriveLine_P.Saturation_LowerSat_a;
      }

      /* End of Saturate: '<S328>/Saturation' */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S143>/Memory' */
        TX_DriveLine_B.Memory_nv = TX_DriveLine_DW.Memory_PreviousInput_es;
        TX_DriveLine_B.domega_o_a = TX_DriveLine_P.TorsionalCompliance2_domega_o;
      }

      /* Integrator: '<S143>/Integrator' incorporates:
       *  Constant: '<S142>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_f, (TX_DriveLine_B.Memory_nv));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_gr != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_i5 = TX_DriveLine_B.domega_o_a;
        }
      }

      /* Sum: '<S142>/Subtract1' incorporates:
       *  Gain: '<S142>/Gain1'
       *  Gain: '<S142>/Gain2'
       *  Integrator: '<S142>/Integrator'
       *  Integrator: '<S143>/Integrator'
       */
      rtb_Subtract1_dg = TX_DriveLine_P.TorsionalCompliance2_b * TX_DriveLine_X.Integrator_CSTATE_i5 +
                         TX_DriveLine_P.TorsionalCompliance2_k * TX_DriveLine_X.Integrator_CSTATE_fk;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S135>/Memory' */
        TX_DriveLine_B.Memory_nn = TX_DriveLine_DW.Memory_PreviousInput_f;
        TX_DriveLine_B.domega_o_i = TX_DriveLine_P.TorsionalCompliance1_domega_o;
      }

      /* Integrator: '<S135>/Integrator' incorporates:
       *  Constant: '<S134>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_k, (TX_DriveLine_B.Memory_nn));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_e != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_d = TX_DriveLine_B.domega_o_i;
        }
      }

      /* Sum: '<S134>/Subtract1' incorporates:
       *  Gain: '<S134>/Gain1'
       *  Gain: '<S134>/Gain2'
       *  Integrator: '<S134>/Integrator'
       *  Integrator: '<S135>/Integrator'
       */
      rtb_Subtract1_k = TX_DriveLine_P.TorsionalCompliance1_b * TX_DriveLine_X.Integrator_CSTATE_d +
                        TX_DriveLine_P.TorsionalCompliance1_k * TX_DriveLine_X.Integrator_CSTATE_nq;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S311>/Memory' */
        TX_DriveLine_B.Memory_f = TX_DriveLine_DW.Memory_PreviousInput_o;
        TX_DriveLine_B.domega_o_k = TX_DriveLine_P.TorsionalCompliance_domega_o;
      }

      /* Integrator: '<S311>/Integrator' incorporates:
       *  Constant: '<S310>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_hb, (TX_DriveLine_B.Memory_f));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_i != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_ng = TX_DriveLine_B.domega_o_k;
        }
      }

      /* Sum: '<S310>/Subtract1' incorporates:
       *  Gain: '<S310>/Gain1'
       *  Gain: '<S310>/Gain2'
       *  Integrator: '<S310>/Integrator'
       *  Integrator: '<S311>/Integrator'
       */
      rtb_Product1_n = TX_DriveLine_P.TorsionalCompliance_b * TX_DriveLine_X.Integrator_CSTATE_ng +
                       TX_DriveLine_P.TorsionalCompliance_k * TX_DriveLine_X.Integrator_CSTATE_d1;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S319>/Memory' */
        TX_DriveLine_B.Memory_g = TX_DriveLine_DW.Memory_PreviousInput_h;
        TX_DriveLine_B.domega_o_kv = TX_DriveLine_P.TorsionalCompliance1_domega_o_c;
      }

      /* Integrator: '<S319>/Integrator' incorporates:
       *  Constant: '<S318>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_d, (TX_DriveLine_B.Memory_g));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_gy != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_hk = TX_DriveLine_B.domega_o_kv;
        }
      }

      /* Sum: '<S318>/Subtract1' incorporates:
       *  Gain: '<S318>/Gain1'
       *  Gain: '<S318>/Gain2'
       *  Integrator: '<S318>/Integrator'
       *  Integrator: '<S319>/Integrator'
       */
      rtb_Sum_dv = TX_DriveLine_P.TorsionalCompliance1_b_j * TX_DriveLine_X.Integrator_CSTATE_hk +
                   TX_DriveLine_P.TorsionalCompliance1_k_b * TX_DriveLine_X.Integrator_CSTATE_lg;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S244>/Memory' */
        TX_DriveLine_B.Memory_c = TX_DriveLine_DW.Memory_PreviousInput_n;
        TX_DriveLine_B.domega_o_a2 = TX_DriveLine_P.TorsionalCompliance4_domega_o;
      }

      /* Integrator: '<S244>/Integrator' incorporates:
       *  Constant: '<S243>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_p, (TX_DriveLine_B.Memory_c));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_ij != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_lz = TX_DriveLine_B.domega_o_a2;
        }
      }

      /* Sum: '<S243>/Subtract1' incorporates:
       *  Gain: '<S243>/Gain1'
       *  Gain: '<S243>/Gain2'
       *  Integrator: '<S243>/Integrator'
       *  Integrator: '<S244>/Integrator'
       */
      rtb_Subtract1_hs = TX_DriveLine_P.TorsionalCompliance4_b * TX_DriveLine_X.Integrator_CSTATE_lz +
                         TX_DriveLine_P.TorsionalCompliance4_k * TX_DriveLine_X.Integrator_CSTATE_iy;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S252>/Memory' */
        TX_DriveLine_B.Memory_pc = TX_DriveLine_DW.Memory_PreviousInput_b;
        TX_DriveLine_B.domega_o_hw = TX_DriveLine_P.TorsionalCompliance5_domega_o;
      }

      /* Integrator: '<S252>/Integrator' incorporates:
       *  Constant: '<S251>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_n, (TX_DriveLine_B.Memory_pc));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_hd != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_bb = TX_DriveLine_B.domega_o_hw;
        }
      }

      /* Sum: '<S251>/Subtract1' incorporates:
       *  Gain: '<S251>/Gain1'
       *  Gain: '<S251>/Gain2'
       *  Integrator: '<S251>/Integrator'
       *  Integrator: '<S252>/Integrator'
       */
      rtb_Subtract1_bs = TX_DriveLine_P.TorsionalCompliance5_b * TX_DriveLine_X.Integrator_CSTATE_bb +
                         TX_DriveLine_P.TorsionalCompliance5_k * TX_DriveLine_X.Integrator_CSTATE_b2;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S228>/Memory' */
        TX_DriveLine_B.Memory_ly = TX_DriveLine_DW.Memory_PreviousInput_fa;
        TX_DriveLine_B.domega_o_b = TX_DriveLine_P.TorsionalCompliance2_domega_o_f;
      }

      /* Integrator: '<S228>/Integrator' incorporates:
       *  Constant: '<S227>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_pu, (TX_DriveLine_B.Memory_ly));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_hs != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_c1 = TX_DriveLine_B.domega_o_b;
        }
      }

      /* Sum: '<S227>/Subtract1' incorporates:
       *  Gain: '<S227>/Gain1'
       *  Gain: '<S227>/Gain2'
       *  Integrator: '<S227>/Integrator'
       *  Integrator: '<S228>/Integrator'
       */
      rtb_Subtract1_iv = TX_DriveLine_P.TorsionalCompliance2_b_d * TX_DriveLine_X.Integrator_CSTATE_c1 +
                         TX_DriveLine_P.TorsionalCompliance2_k_l * TX_DriveLine_X.Integrator_CSTATE_hkp;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S236>/Memory' */
        TX_DriveLine_B.Memory_pz = TX_DriveLine_DW.Memory_PreviousInput_l;
        TX_DriveLine_B.domega_o_c = TX_DriveLine_P.TorsionalCompliance3_domega_o;
      }

      /* Integrator: '<S236>/Integrator' incorporates:
       *  Constant: '<S235>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_hbb, (TX_DriveLine_B.Memory_pz));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_n != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_bx = TX_DriveLine_B.domega_o_c;
        }
      }

      /* Sum: '<S235>/Subtract1' incorporates:
       *  Gain: '<S235>/Gain1'
       *  Gain: '<S235>/Gain2'
       *  Integrator: '<S235>/Integrator'
       *  Integrator: '<S236>/Integrator'
       */
      rtb_Subtract1_lh = TX_DriveLine_P.TorsionalCompliance3_b * TX_DriveLine_X.Integrator_CSTATE_bx +
                         TX_DriveLine_P.TorsionalCompliance3_k * TX_DriveLine_X.Integrator_CSTATE_g1;

      /* MultiPortSwitch generated from: '<S97>/Multiport Switch' incorporates:
       *  Constant: '<S1>/DriveType[1-FD;2-RD;3-4WD]'
       */
      switch (static_cast<int32_T>(TX_DriveLine_P.drive_type)) {
        case 1:
          /* SignalConversion generated from: '<S102>/Vector Concatenate' */
          rtb_VectorConcatenate_h[3] = 0.0;

          /* SignalConversion generated from: '<S102>/Vector Concatenate' */
          rtb_VectorConcatenate_h[2] = 0.0;

          /* SignalConversion generated from: '<S102>/Vector Concatenate' */
          rtb_VectorConcatenate_h[1] = rtb_Subtract1_k;

          /* SignalConversion generated from: '<S102>/Vector Concatenate' */
          rtb_VectorConcatenate_h[0] = rtb_Subtract1_dg;
          break;

        case 2:
          /* SignalConversion generated from: '<S104>/Vector Concatenate' */
          rtb_VectorConcatenate_h[3] = rtb_Sum_dv;

          /* SignalConversion generated from: '<S104>/Vector Concatenate' */
          rtb_VectorConcatenate_h[2] = rtb_Product1_n;

          /* SignalConversion generated from: '<S104>/Vector Concatenate' */
          rtb_VectorConcatenate_h[1] = 0.0;

          /* SignalConversion generated from: '<S104>/Vector Concatenate' */
          rtb_VectorConcatenate_h[0] = 0.0;
          break;

        default:
          /* SignalConversion generated from: '<S103>/Vector Concatenate' */
          rtb_VectorConcatenate_h[3] = rtb_Subtract1_lh;

          /* SignalConversion generated from: '<S103>/Vector Concatenate' */
          rtb_VectorConcatenate_h[2] = rtb_Subtract1_iv;

          /* SignalConversion generated from: '<S103>/Vector Concatenate' */
          rtb_VectorConcatenate_h[1] = rtb_Subtract1_bs;

          /* SignalConversion generated from: '<S103>/Vector Concatenate' */
          rtb_VectorConcatenate_h[0] = rtb_Subtract1_hs;
          break;
      }

      /* Switch: '<S114>/Switch' incorporates:
       *  Constant: '<S114>/Constant'
       *  Constant: '<S114>/Constant1'
       *  UnaryMinus: '<S114>/Unary Minus'
       */
      if (TX_DriveLine_P.Constant_Value_j > TX_DriveLine_P.Switch_Threshold_f) {
        rtb_Product_gq = TX_DriveLine_P.Constant1_Value_g;
      } else {
        rtb_Product_gq = -TX_DriveLine_P.Constant1_Value_g;
      }

      /* End of Switch: '<S114>/Switch' */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        TX_DriveLine_B.VectorConcatenate_l[0] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
        TX_DriveLine_B.VectorConcatenate_l[1] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
      }

      /* Integrator: '<S108>/Integrator' incorporates:
       *  Concatenate: '<S108>/Vector Concatenate'
       *  Constant: '<S108>/Constant'
       *  Constant: '<S108>/Constant1'
       */
      /* Limited  Integrator  */
      if (TX_DriveLine_DW.Integrator_IWORK_p != 0) {
        TX_DriveLine_X.Integrator_CSTATE_h2[0] = TX_DriveLine_B.VectorConcatenate_l[0];
        TX_DriveLine_X.Integrator_CSTATE_h2[1] = TX_DriveLine_B.VectorConcatenate_l[1];
      }

      /* Gain: '<S114>/Gain' */
      rtb_Gain_p = TX_DriveLine_P.ratio_diff_front / 2.0;

      /* Integrator: '<S108>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_h2[0] >= TX_DriveLine_P.Integrator_UpperSat) {
        TX_DriveLine_X.Integrator_CSTATE_h2[0] = TX_DriveLine_P.Integrator_UpperSat;
      } else if (TX_DriveLine_X.Integrator_CSTATE_h2[0] <= TX_DriveLine_P.Integrator_LowerSat) {
        TX_DriveLine_X.Integrator_CSTATE_h2[0] = TX_DriveLine_P.Integrator_LowerSat;
      }

      /* Integrator: '<S108>/Integrator' */
      rtb_Integrator_cd[0] = TX_DriveLine_X.Integrator_CSTATE_h2[0];

      /* Integrator: '<S108>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_h2[1] >= TX_DriveLine_P.Integrator_UpperSat) {
        TX_DriveLine_X.Integrator_CSTATE_h2[1] = TX_DriveLine_P.Integrator_UpperSat;
      } else if (TX_DriveLine_X.Integrator_CSTATE_h2[1] <= TX_DriveLine_P.Integrator_LowerSat) {
        TX_DriveLine_X.Integrator_CSTATE_h2[1] = TX_DriveLine_P.Integrator_LowerSat;
      }

      /* Integrator: '<S108>/Integrator' */
      rtb_Integrator_cd[1] = TX_DriveLine_X.Integrator_CSTATE_h2[1];

      /* Sum: '<S114>/Sum of Elements' incorporates:
       *  Gain: '<S114>/Gain'
       *  Product: '<S114>/Product'
       */
      rtb_SumofElements_c =
          rtb_Product_gq * rtb_Integrator_cd[0] * rtb_Gain_p + rtb_Product_gq * rtb_Integrator_cd[1] * rtb_Gain_p;

      /* Switch: '<S290>/Switch' incorporates:
       *  Constant: '<S290>/Constant'
       *  Constant: '<S290>/Constant1'
       *  UnaryMinus: '<S290>/Unary Minus'
       */
      if (TX_DriveLine_P.Constant_Value_jl > TX_DriveLine_P.Switch_Threshold_o) {
        rtb_Product_gq = TX_DriveLine_P.Constant1_Value_b;
      } else {
        rtb_Product_gq = -TX_DriveLine_P.Constant1_Value_b;
      }

      /* End of Switch: '<S290>/Switch' */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        TX_DriveLine_B.VectorConcatenate_c[0] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
        TX_DriveLine_B.VectorConcatenate_c[1] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
      }

      /* Integrator: '<S284>/Integrator' incorporates:
       *  Concatenate: '<S284>/Vector Concatenate'
       *  Constant: '<S284>/Constant'
       *  Constant: '<S284>/Constant1'
       */
      /* Limited  Integrator  */
      if (TX_DriveLine_DW.Integrator_IWORK_a != 0) {
        TX_DriveLine_X.Integrator_CSTATE_c0[0] = TX_DriveLine_B.VectorConcatenate_c[0];
        TX_DriveLine_X.Integrator_CSTATE_c0[1] = TX_DriveLine_B.VectorConcatenate_c[1];
      }

      /* Gain: '<S290>/Gain' */
      rtb_Gain_p = TX_DriveLine_P.ratio_diff_rear / 2.0;

      /* Integrator: '<S284>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_c0[0] >= TX_DriveLine_P.Integrator_UpperSat_f) {
        TX_DriveLine_X.Integrator_CSTATE_c0[0] = TX_DriveLine_P.Integrator_UpperSat_f;
      } else if (TX_DriveLine_X.Integrator_CSTATE_c0[0] <= TX_DriveLine_P.Integrator_LowerSat_p) {
        TX_DriveLine_X.Integrator_CSTATE_c0[0] = TX_DriveLine_P.Integrator_LowerSat_p;
      }

      /* Integrator: '<S284>/Integrator' */
      rtb_Integrator_l[0] = TX_DriveLine_X.Integrator_CSTATE_c0[0];

      /* Integrator: '<S284>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_c0[1] >= TX_DriveLine_P.Integrator_UpperSat_f) {
        TX_DriveLine_X.Integrator_CSTATE_c0[1] = TX_DriveLine_P.Integrator_UpperSat_f;
      } else if (TX_DriveLine_X.Integrator_CSTATE_c0[1] <= TX_DriveLine_P.Integrator_LowerSat_p) {
        TX_DriveLine_X.Integrator_CSTATE_c0[1] = TX_DriveLine_P.Integrator_LowerSat_p;
      }

      /* Integrator: '<S284>/Integrator' */
      rtb_Integrator_l[1] = TX_DriveLine_X.Integrator_CSTATE_c0[1];

      /* Sum: '<S290>/Sum of Elements' incorporates:
       *  Gain: '<S290>/Gain'
       *  Product: '<S290>/Product'
       */
      rtb_SumofElements_l3 =
          rtb_Product_gq * rtb_Integrator_l[0] * rtb_Gain_p + rtb_Product_gq * rtb_Integrator_l[1] * rtb_Gain_p;

      /* Switch: '<S262>/Switch' incorporates:
       *  Constant: '<S262>/Constant'
       *  Constant: '<S262>/Constant1'
       *  UnaryMinus: '<S262>/Unary Minus'
       */
      if (TX_DriveLine_P.TransferCase_shaftSwitchMask > TX_DriveLine_P.Switch_Threshold_c) {
        rtb_Product_gq = TX_DriveLine_P.Constant1_Value_c;
      } else {
        rtb_Product_gq = -TX_DriveLine_P.Constant1_Value_c;
      }

      /* End of Switch: '<S262>/Switch' */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        TX_DriveLine_B.VectorConcatenate_lo[0] =
            -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0] * TX_DriveLine_P.ratio_diff_front;
        TX_DriveLine_B.VectorConcatenate_lo[1] =
            -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0] * TX_DriveLine_P.ratio_diff_rear;
      }

      /* Integrator: '<S158>/Integrator' incorporates:
       *  Concatenate: '<S158>/Vector Concatenate'
       *  Constant: '<S158>/Constant'
       *  Constant: '<S158>/Constant1'
       */
      /* Limited  Integrator  */
      if (TX_DriveLine_DW.Integrator_IWORK_ao != 0) {
        TX_DriveLine_X.Integrator_CSTATE_g3[0] = TX_DriveLine_B.VectorConcatenate_lo[0];
        TX_DriveLine_X.Integrator_CSTATE_g3[1] = TX_DriveLine_B.VectorConcatenate_lo[1];
      }

      if (TX_DriveLine_X.Integrator_CSTATE_g3[0] >= TX_DriveLine_P.Integrator_UpperSat_e) {
        TX_DriveLine_X.Integrator_CSTATE_g3[0] = TX_DriveLine_P.Integrator_UpperSat_e;
      } else if (TX_DriveLine_X.Integrator_CSTATE_g3[0] <= TX_DriveLine_P.Integrator_LowerSat_i) {
        TX_DriveLine_X.Integrator_CSTATE_g3[0] = TX_DriveLine_P.Integrator_LowerSat_i;
      }

      if (TX_DriveLine_X.Integrator_CSTATE_g3[1] >= TX_DriveLine_P.Integrator_UpperSat_e) {
        TX_DriveLine_X.Integrator_CSTATE_g3[1] = TX_DriveLine_P.Integrator_UpperSat_e;
      } else if (TX_DriveLine_X.Integrator_CSTATE_g3[1] <= TX_DriveLine_P.Integrator_LowerSat_i) {
        TX_DriveLine_X.Integrator_CSTATE_g3[1] = TX_DriveLine_P.Integrator_LowerSat_i;
      }

      /* Sum: '<S262>/Add1' incorporates:
       *  Constant: '<S158>/TrqSplitRatioConstantConstant'
       *  Constant: '<S262>/Constant2'
       *  Gain: '<S262>/Gain'
       *  Integrator: '<S158>/Integrator'
       *  Product: '<S262>/Product'
       *  Product: '<S262>/Product1'
       *  Product: '<S262>/Product2'
       *  Sum: '<S262>/Add2'
       */
      rtb_Add1 = rtb_Product_gq * TX_DriveLine_X.Integrator_CSTATE_g3[0] * TX_DriveLine_P.TransferCase_Ndiff *
                     TX_DriveLine_P.TransferCase_TrqSplitRatio +
                 rtb_Product_gq * TX_DriveLine_X.Integrator_CSTATE_g3[1] * TX_DriveLine_P.TransferCase_Ndiff *
                     (TX_DriveLine_P.Constant2_Value - TX_DriveLine_P.TransferCase_TrqSplitRatio);

      /* MultiPortSwitch generated from: '<S97>/Multiport Switch' incorporates:
       *  Constant: '<S1>/DriveType[1-FD;2-RD;3-4WD]'
       */
      switch (static_cast<int32_T>(TX_DriveLine_P.drive_type)) {
        case 1:
          rtb_Switch = rtb_SumofElements_c;
          break;

        case 2:
          rtb_Switch = rtb_SumofElements_l3;
          break;

        default:
          rtb_Switch = rtb_Add1;
          break;
      }

      /* BusCreator: '<S3>/Bus Creator1' incorporates:
       *  Constant: '<S3>/Constant2'
       *  Constant: '<S3>/Constant3'
       */
      TX_DriveLine_B.Merge.gear_engaged = rtb_SumofElements_f;
      TX_DriveLine_B.Merge.eng_spd_rad_s = rtb_Abs_e;
      TX_DriveLine_B.Merge.wheel_drive_trq_Nm[0] = rtb_VectorConcatenate_h[0];
      TX_DriveLine_B.Merge.wheel_drive_trq_Nm[1] = rtb_VectorConcatenate_h[1];
      TX_DriveLine_B.Merge.wheel_drive_trq_Nm[2] = rtb_VectorConcatenate_h[2];
      TX_DriveLine_B.Merge.wheel_drive_trq_Nm[3] = rtb_VectorConcatenate_h[3];
      TX_DriveLine_B.Merge.trans_out_shaft_spd_rad_s = rtb_Switch;
      TX_DriveLine_B.Merge.front_mot_spd_rad_s = TX_DriveLine_P.Constant2_Value_b;
      TX_DriveLine_B.Merge.rear_mot_spd_rad_s = TX_DriveLine_P.Constant3_Value;

      /* Sum: '<S114>/Add' incorporates:
       *  Gain: '<S114>/Gain1'
       */
      rtb_Product_gq =
          TX_DriveLine_P.Gain1_Gain_fp * rtb_Integrator_cd[0] - TX_DriveLine_P.Gain1_Gain_fp * rtb_Integrator_cd[1];

      /* Gain: '<S132>/Gain' incorporates:
       *  Abs: '<S131>/Abs'
       *  Constant: '<S132>/Constant1'
       *  Gain: '<S132>/Gain1'
       *  Gain: '<S132>/Gain2'
       *  SignalConversion generated from: '<S114>/Vector Concatenate'
       *  Sum: '<S132>/Subtract1'
       *  Sum: '<S132>/Subtract2'
       *  Trigonometry: '<S132>/Trigonometric Function'
       */
      rtb_Product6 =
          (std::tanh((TX_DriveLine_P.Gain1_Gain_i * std::abs(rtb_SumofElements_c) - TX_DriveLine_P.Constant1_Value_l) *
                     TX_DriveLine_P.Gain2_Gain_el) +
           TX_DriveLine_P.Constant1_Value_l) *
          TX_DriveLine_P.Gain_Gain_jy;

      /* Switch: '<S125>/Switch' incorporates:
       *  Constant: '<S125>/Constant'
       *  Constant: '<S133>/Constant'
       *  Product: '<S125>/Product1'
       *  SignalConversion generated from: '<S114>/Vector Concatenate'
       */
      if (rtb_Subtract1_b0 * rtb_SumofElements_c > TX_DriveLine_P.Switch_Threshold_oj) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.ICEFrontLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_f;
      }

      /* Product: '<S125>/Product4' incorporates:
       *  Constant: '<S131>/Constant'
       *  Constant: '<S132>/Constant1'
       *  Product: '<S131>/Product3'
       *  Product: '<S131>/Product5'
       *  Sum: '<S131>/Subtract1'
       *  Sum: '<S132>/Subtract'
       *  Switch: '<S125>/Switch'
       */
      rtb_Product4_hr = ((TX_DriveLine_P.Constant1_Value_l - rtb_Product6) * TX_DriveLine_P.Constant_Value_l +
                         rtb_VectorConcatenate_c_tmp * rtb_Product6) *
                        rtb_Subtract1_b0;

      /* Gain: '<S128>/Gain' incorporates:
       *  Abs: '<S127>/Abs'
       *  Constant: '<S128>/Constant1'
       *  Gain: '<S128>/Gain1'
       *  Gain: '<S128>/Gain2'
       *  Sum: '<S128>/Subtract1'
       *  Sum: '<S128>/Subtract2'
       *  Trigonometry: '<S128>/Trigonometric Function'
       *  UnaryMinus: '<S114>/Unary Minus1'
       */
      rtb_SumofElements_f = (std::tanh((TX_DriveLine_P.Gain1_Gain_h * std::abs(-rtb_Integrator_cd[0]) -
                                        TX_DriveLine_P.Constant1_Value_o) *
                                       TX_DriveLine_P.Gain2_Gain_o) +
                             TX_DriveLine_P.Constant1_Value_o) *
                            TX_DriveLine_P.Gain_Gain_k;

      /* Switch: '<S123>/Switch' incorporates:
       *  Constant: '<S123>/Constant'
       *  Constant: '<S133>/Constant'
       *  Product: '<S123>/Product1'
       *  UnaryMinus: '<S114>/Unary Minus1'
       *  UnaryMinus: '<S142>/Unary Minus'
       */
      if (-rtb_Subtract1_dg * -rtb_Integrator_cd[0] > TX_DriveLine_P.Switch_Threshold_l) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.ICEFrontLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_aw;
      }

      /* Product: '<S123>/Product4' incorporates:
       *  Constant: '<S127>/Constant'
       *  Constant: '<S128>/Constant1'
       *  Product: '<S127>/Product3'
       *  Product: '<S127>/Product5'
       *  Sum: '<S127>/Subtract1'
       *  Sum: '<S128>/Subtract'
       *  Switch: '<S123>/Switch'
       *  UnaryMinus: '<S142>/Unary Minus'
       */
      rtb_Product4_ht = ((TX_DriveLine_P.Constant1_Value_o - rtb_SumofElements_f) * TX_DriveLine_P.Constant_Value_b +
                         rtb_VectorConcatenate_c_tmp * rtb_SumofElements_f) *
                        -rtb_Subtract1_dg;

      /* Gain: '<S130>/Gain' incorporates:
       *  Abs: '<S129>/Abs'
       *  Constant: '<S130>/Constant1'
       *  Gain: '<S130>/Gain1'
       *  Gain: '<S130>/Gain2'
       *  Sum: '<S130>/Subtract1'
       *  Sum: '<S130>/Subtract2'
       *  Trigonometry: '<S130>/Trigonometric Function'
       *  UnaryMinus: '<S114>/Unary Minus1'
       */
      rtb_SumofElements_f = (std::tanh((TX_DriveLine_P.Gain1_Gain_a * std::abs(-rtb_Integrator_cd[1]) -
                                        TX_DriveLine_P.Constant1_Value_mu) *
                                       TX_DriveLine_P.Gain2_Gain_n) +
                             TX_DriveLine_P.Constant1_Value_mu) *
                            TX_DriveLine_P.Gain_Gain_c;

      /* Switch: '<S124>/Switch' incorporates:
       *  Constant: '<S124>/Constant'
       *  Constant: '<S133>/Constant'
       *  Product: '<S124>/Product1'
       *  UnaryMinus: '<S114>/Unary Minus1'
       *  UnaryMinus: '<S134>/Unary Minus'
       */
      if (-rtb_Subtract1_k * -rtb_Integrator_cd[1] > TX_DriveLine_P.Switch_Threshold_pg) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.ICEFrontLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_m;
      }

      /* Product: '<S124>/Product4' incorporates:
       *  Constant: '<S129>/Constant'
       *  Constant: '<S130>/Constant1'
       *  Product: '<S129>/Product3'
       *  Product: '<S129>/Product5'
       *  Sum: '<S129>/Subtract1'
       *  Sum: '<S130>/Subtract'
       *  Switch: '<S124>/Switch'
       *  UnaryMinus: '<S134>/Unary Minus'
       */
      rtb_Product4_i = ((TX_DriveLine_P.Constant1_Value_mu - rtb_SumofElements_f) * TX_DriveLine_P.Constant_Value_lm +
                        rtb_VectorConcatenate_c_tmp * rtb_SumofElements_f) *
                       -rtb_Subtract1_k;

      /* Integrator: '<S110>/Integrator' */
      rtb_Integrator_b = TX_DriveLine_X.Integrator_CSTATE_nv;

      /* MATLAB Function: '<S108>/Limited Slip Differential' incorporates:
       *  Constant: '<S108>/Jd'
       *  Constant: '<S108>/Jw1'
       *  Constant: '<S108>/Jw3'
       *  Constant: '<S108>/Ndiff2'
       *  Constant: '<S108>/bd'
       *  Constant: '<S108>/bw1'
       *  Constant: '<S108>/bw2'
       */
      TX_DriveLine_LimitedSlipDifferential(
          rtb_Product4_hr, rtb_Product4_ht, rtb_Product4_i, rtb_Integrator_b,
          TX_DriveLine_P.ICEFrontLimitedSlipDifferential_bw1, TX_DriveLine_P.ICEFrontLimitedSlipDifferential_bd,
          TX_DriveLine_P.ICEFrontLimitedSlipDifferential_bw2, TX_DriveLine_P.ratio_diff_front,
          TX_DriveLine_P.ICEFrontLimitedSlipDifferential_Jd, TX_DriveLine_P.ICEFrontLimitedSlipDifferential_Jw1,
          TX_DriveLine_P.ICEFrontLimitedSlipDifferential_Jw2, rtb_Integrator_cd,
          &TX_DriveLine_B.sf_LimitedSlipDifferential, &TX_DriveLine_P.sf_LimitedSlipDifferential);

      /* Product: '<S110>/Product' incorporates:
       *  Abs: '<S122>/Abs'
       *  Constant: '<S108>/Constant3'
       *  Constant: '<S122>/Constant'
       *  Constant: '<S122>/Constant1'
       *  Constant: '<S122>/Constant2'
       *  Gain: '<S108>/2*pi'
       *  Gain: '<S122>/Gain'
       *  Lookup_n-D: '<S122>/mu Table'
       *  Product: '<S108>/Product1'
       *  Product: '<S122>/Product'
       *  Sum: '<S110>/Sum'
       *  Trigonometry: '<S122>/Trigonometric Function'
       */
      TX_DriveLine_B.Product_p =
          (TX_DriveLine_P.ICEFrontLimitedSlipDifferential_Fc * TX_DriveLine_P.ICEFrontLimitedSlipDifferential_Ndisks *
               TX_DriveLine_P.ICEFrontLimitedSlipDifferential_Reff *
               std::tanh(TX_DriveLine_P.Gain_Gain_kg * rtb_Product_gq) *
               look1_binlxpw(std::abs(rtb_Product_gq), TX_DriveLine_P.ICEFrontLimitedSlipDifferential_dw,
                             TX_DriveLine_P.ICEFrontLimitedSlipDifferential_muc, 7U) -
           rtb_Integrator_b) *
          (1.0 / TX_DriveLine_P.ICEFrontLimitedSlipDifferential_tauC * TX_DriveLine_P.upi_Gain);

      /* Sum: '<S134>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S114>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_a = -rtb_Integrator_cd[1] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[1];

      /* Switch: '<S135>/Switch' incorporates:
       *  Integrator: '<S135>/Integrator'
       */
      if (TX_DriveLine_B.Memory_nn != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_d;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_i;
      }

      /* Product: '<S135>/Product' incorporates:
       *  Constant: '<S134>/omega_c'
       *  Sum: '<S135>/Sum'
       *  Switch: '<S135>/Switch'
       */
      TX_DriveLine_B.Product_e =
          (TX_DriveLine_B.Subtract_a - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance1_omega_c;

      /* Sum: '<S142>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S114>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_k = -rtb_Integrator_cd[0] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[0];

      /* Switch: '<S143>/Switch' incorporates:
       *  Integrator: '<S143>/Integrator'
       */
      if (TX_DriveLine_B.Memory_nv != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_i5;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_a;
      }

      /* Product: '<S143>/Product' incorporates:
       *  Constant: '<S142>/omega_c'
       *  Sum: '<S143>/Sum'
       *  Switch: '<S143>/Switch'
       */
      TX_DriveLine_B.Product_b =
          (TX_DriveLine_B.Subtract_k - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance2_omega_c;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S212>/Memory' */
        TX_DriveLine_B.Memory_j = TX_DriveLine_DW.Memory_PreviousInput_bo;
        TX_DriveLine_B.domega_o_az = TX_DriveLine_P.TorsionalCompliance_domega_o_d;
      }

      /* Integrator: '<S212>/Integrator' incorporates:
       *  Constant: '<S211>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_nr, (TX_DriveLine_B.Memory_j));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_ct != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_db = TX_DriveLine_B.domega_o_az;
        }
      }

      /* Sum: '<S211>/Subtract1' incorporates:
       *  Gain: '<S211>/Gain1'
       *  Gain: '<S211>/Gain2'
       *  Integrator: '<S211>/Integrator'
       *  Integrator: '<S212>/Integrator'
       */
      rtb_Product6 = TX_DriveLine_P.TorsionalCompliance_b_k * TX_DriveLine_X.Integrator_CSTATE_db +
                     TX_DriveLine_P.TorsionalCompliance_k_h * TX_DriveLine_X.Integrator_CSTATE_dv;

      /* Switch: '<S165>/Switch' incorporates:
       *  Constant: '<S165>/Constant'
       *  Constant: '<S165>/Constant1'
       *  UnaryMinus: '<S165>/Unary Minus'
       */
      if (TX_DriveLine_P.Constant_Value_oe > TX_DriveLine_P.Switch_Threshold_ap) {
        rtb_SumofElements_c = TX_DriveLine_P.Constant1_Value_et;
      } else {
        rtb_SumofElements_c = -TX_DriveLine_P.Constant1_Value_et;
      }

      /* End of Switch: '<S165>/Switch' */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        TX_DriveLine_B.VectorConcatenate_e[0] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
        TX_DriveLine_B.VectorConcatenate_e[1] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
      }

      /* Integrator: '<S159>/Integrator' incorporates:
       *  Concatenate: '<S159>/Vector Concatenate'
       *  Constant: '<S159>/Constant'
       *  Constant: '<S159>/Constant1'
       */
      /* Limited  Integrator  */
      if (TX_DriveLine_DW.Integrator_IWORK_b != 0) {
        TX_DriveLine_X.Integrator_CSTATE_eu[0] = TX_DriveLine_B.VectorConcatenate_e[0];
        TX_DriveLine_X.Integrator_CSTATE_eu[1] = TX_DriveLine_B.VectorConcatenate_e[1];
      }

      /* Gain: '<S165>/Gain' */
      rtb_Gain_p = TX_DriveLine_P.ratio_diff_front / 2.0;

      /* Integrator: '<S159>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_eu[0] >= TX_DriveLine_P.Integrator_UpperSat_er) {
        TX_DriveLine_X.Integrator_CSTATE_eu[0] = TX_DriveLine_P.Integrator_UpperSat_er;
      } else if (TX_DriveLine_X.Integrator_CSTATE_eu[0] <= TX_DriveLine_P.Integrator_LowerSat_n) {
        TX_DriveLine_X.Integrator_CSTATE_eu[0] = TX_DriveLine_P.Integrator_LowerSat_n;
      }

      /* Integrator: '<S159>/Integrator' */
      rtb_Integrator_f[0] = TX_DriveLine_X.Integrator_CSTATE_eu[0];

      /* Integrator: '<S159>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_eu[1] >= TX_DriveLine_P.Integrator_UpperSat_er) {
        TX_DriveLine_X.Integrator_CSTATE_eu[1] = TX_DriveLine_P.Integrator_UpperSat_er;
      } else if (TX_DriveLine_X.Integrator_CSTATE_eu[1] <= TX_DriveLine_P.Integrator_LowerSat_n) {
        TX_DriveLine_X.Integrator_CSTATE_eu[1] = TX_DriveLine_P.Integrator_LowerSat_n;
      }

      /* Integrator: '<S159>/Integrator' */
      rtb_Integrator_f[1] = TX_DriveLine_X.Integrator_CSTATE_eu[1];

      /* Sum: '<S165>/Sum of Elements' incorporates:
       *  Gain: '<S165>/Gain'
       *  Product: '<S165>/Product'
       */
      rtb_SumofElements_c = rtb_SumofElements_c * rtb_Integrator_f[0] * rtb_Gain_p +
                            rtb_SumofElements_c * rtb_Integrator_f[1] * rtb_Gain_p;

      /* Sum: '<S165>/Add' incorporates:
       *  Gain: '<S165>/Gain1'
       */
      rtb_Gain_p =
          TX_DriveLine_P.Gain1_Gain_g * rtb_Integrator_f[0] - TX_DriveLine_P.Gain1_Gain_g * rtb_Integrator_f[1];

      /* Gain: '<S183>/Gain' incorporates:
       *  Abs: '<S182>/Abs'
       *  Constant: '<S183>/Constant1'
       *  Gain: '<S183>/Gain1'
       *  Gain: '<S183>/Gain2'
       *  SignalConversion generated from: '<S165>/Vector Concatenate'
       *  Sum: '<S183>/Subtract1'
       *  Sum: '<S183>/Subtract2'
       *  Trigonometry: '<S183>/Trigonometric Function'
       */
      rtb_SumofElements_f =
          (std::tanh((TX_DriveLine_P.Gain1_Gain_a0 * std::abs(rtb_SumofElements_c) - TX_DriveLine_P.Constant1_Value_i) *
                     TX_DriveLine_P.Gain2_Gain_ls) +
           TX_DriveLine_P.Constant1_Value_i) *
          TX_DriveLine_P.Gain_Gain_l;

      /* Switch: '<S176>/Switch' incorporates:
       *  Constant: '<S176>/Constant'
       *  Constant: '<S184>/Constant'
       *  Product: '<S176>/Product1'
       *  SignalConversion generated from: '<S165>/Vector Concatenate'
       *  UnaryMinus: '<S211>/Unary Minus'
       */
      if (-rtb_Product6 * rtb_SumofElements_c > TX_DriveLine_P.Switch_Threshold_f0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_hn;
      }

      /* Product: '<S176>/Product4' incorporates:
       *  Constant: '<S182>/Constant'
       *  Constant: '<S183>/Constant1'
       *  Product: '<S182>/Product3'
       *  Product: '<S182>/Product5'
       *  Sum: '<S182>/Subtract1'
       *  Sum: '<S183>/Subtract'
       *  Switch: '<S176>/Switch'
       *  UnaryMinus: '<S211>/Unary Minus'
       */
      rtb_Product4_j = ((TX_DriveLine_P.Constant1_Value_i - rtb_SumofElements_f) * TX_DriveLine_P.Constant_Value_kt +
                        rtb_VectorConcatenate_c_tmp * rtb_SumofElements_f) *
                       -rtb_Product6;

      /* Gain: '<S179>/Gain' incorporates:
       *  Abs: '<S178>/Abs'
       *  Constant: '<S179>/Constant1'
       *  Gain: '<S179>/Gain1'
       *  Gain: '<S179>/Gain2'
       *  Sum: '<S179>/Subtract1'
       *  Sum: '<S179>/Subtract2'
       *  Trigonometry: '<S179>/Trigonometric Function'
       *  UnaryMinus: '<S165>/Unary Minus1'
       */
      rtb_SumofElements_f =
          (std::tanh((TX_DriveLine_P.Gain1_Gain_b * std::abs(-rtb_Integrator_f[0]) - TX_DriveLine_P.Constant1_Value_h) *
                     TX_DriveLine_P.Gain2_Gain_c) +
           TX_DriveLine_P.Constant1_Value_h) *
          TX_DriveLine_P.Gain_Gain_cl;

      /* Switch: '<S174>/Switch' incorporates:
       *  Constant: '<S174>/Constant'
       *  Constant: '<S184>/Constant'
       *  Product: '<S174>/Product1'
       *  UnaryMinus: '<S165>/Unary Minus1'
       *  UnaryMinus: '<S243>/Unary Minus'
       */
      if (-rtb_Subtract1_hs * -rtb_Integrator_f[0] > TX_DriveLine_P.Switch_Threshold_oa) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_is;
      }

      /* Product: '<S174>/Product4' incorporates:
       *  Constant: '<S178>/Constant'
       *  Constant: '<S179>/Constant1'
       *  Product: '<S178>/Product3'
       *  Product: '<S178>/Product5'
       *  Sum: '<S178>/Subtract1'
       *  Sum: '<S179>/Subtract'
       *  Switch: '<S174>/Switch'
       *  UnaryMinus: '<S243>/Unary Minus'
       */
      rtb_Product4_g = ((TX_DriveLine_P.Constant1_Value_h - rtb_SumofElements_f) * TX_DriveLine_P.Constant_Value_ae +
                        rtb_VectorConcatenate_c_tmp * rtb_SumofElements_f) *
                       -rtb_Subtract1_hs;

      /* Gain: '<S181>/Gain' incorporates:
       *  Abs: '<S180>/Abs'
       *  Constant: '<S181>/Constant1'
       *  Gain: '<S181>/Gain1'
       *  Gain: '<S181>/Gain2'
       *  Sum: '<S181>/Subtract1'
       *  Sum: '<S181>/Subtract2'
       *  Trigonometry: '<S181>/Trigonometric Function'
       *  UnaryMinus: '<S165>/Unary Minus1'
       */
      rtb_SumofElements_f = (std::tanh((TX_DriveLine_P.Gain1_Gain_j * std::abs(-rtb_Integrator_f[1]) -
                                        TX_DriveLine_P.Constant1_Value_ln) *
                                       TX_DriveLine_P.Gain2_Gain_m) +
                             TX_DriveLine_P.Constant1_Value_ln) *
                            TX_DriveLine_P.Gain_Gain_p;

      /* Switch: '<S175>/Switch' incorporates:
       *  Constant: '<S175>/Constant'
       *  Constant: '<S184>/Constant'
       *  Product: '<S175>/Product1'
       *  UnaryMinus: '<S165>/Unary Minus1'
       *  UnaryMinus: '<S251>/Unary Minus'
       */
      if (-rtb_Subtract1_bs * -rtb_Integrator_f[1] > TX_DriveLine_P.Switch_Threshold_n) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_mr;
      }

      /* Product: '<S175>/Product4' incorporates:
       *  Constant: '<S180>/Constant'
       *  Constant: '<S181>/Constant1'
       *  Product: '<S180>/Product3'
       *  Product: '<S180>/Product5'
       *  Sum: '<S180>/Subtract1'
       *  Sum: '<S181>/Subtract'
       *  Switch: '<S175>/Switch'
       *  UnaryMinus: '<S251>/Unary Minus'
       */
      rtb_Product4_nd = ((TX_DriveLine_P.Constant1_Value_ln - rtb_SumofElements_f) * TX_DriveLine_P.Constant_Value_fi +
                         rtb_VectorConcatenate_c_tmp * rtb_SumofElements_f) *
                        -rtb_Subtract1_bs;

      /* Integrator: '<S161>/Integrator' */
      rtb_Integrator_ce = TX_DriveLine_X.Integrator_CSTATE_ea;

      /* MATLAB Function: '<S159>/Limited Slip Differential' incorporates:
       *  Constant: '<S159>/Jd'
       *  Constant: '<S159>/Jw1'
       *  Constant: '<S159>/Jw3'
       *  Constant: '<S159>/Ndiff2'
       *  Constant: '<S159>/bd'
       *  Constant: '<S159>/bw1'
       *  Constant: '<S159>/bw2'
       */
      TX_DriveLine_LimitedSlipDifferential(
          rtb_Product4_j, rtb_Product4_g, rtb_Product4_nd, rtb_Integrator_ce,
          TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_bw1, TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_bd,
          TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_bw2, TX_DriveLine_P.ratio_diff_front,
          TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_Jd, TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_Jw1,
          TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_Jw2, rtb_Integrator_f,
          &TX_DriveLine_B.sf_LimitedSlipDifferential_i, &TX_DriveLine_P.sf_LimitedSlipDifferential_i);

      /* Product: '<S161>/Product' incorporates:
       *  Abs: '<S173>/Abs'
       *  Constant: '<S159>/Constant3'
       *  Constant: '<S173>/Constant'
       *  Constant: '<S173>/Constant1'
       *  Constant: '<S173>/Constant2'
       *  Gain: '<S159>/2*pi'
       *  Gain: '<S173>/Gain'
       *  Lookup_n-D: '<S173>/mu Table'
       *  Product: '<S159>/Product1'
       *  Product: '<S173>/Product'
       *  Sum: '<S161>/Sum'
       *  Trigonometry: '<S173>/Trigonometric Function'
       */
      TX_DriveLine_B.Product_c =
          (TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_Fc *
               TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_Ndisks *
               TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_Reff *
               std::tanh(TX_DriveLine_P.Gain_Gain_d * rtb_Gain_p) *
               look1_binlcpw(std::abs(rtb_Gain_p), TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_dw,
                             TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_muc, 6U) -
           rtb_Integrator_ce) *
          (1.0 / TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_tauC * TX_DriveLine_P.upi_Gain_a);
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S220>/Memory' */
        TX_DriveLine_B.Memory_la = TX_DriveLine_DW.Memory_PreviousInput_fn;
        TX_DriveLine_B.domega_o_n = TX_DriveLine_P.TorsionalCompliance1_domega_o_h;
      }

      /* Integrator: '<S220>/Integrator' incorporates:
       *  Constant: '<S219>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_j, (TX_DriveLine_B.Memory_la));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_o != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_ny = TX_DriveLine_B.domega_o_n;
        }
      }

      /* Sum: '<S219>/Subtract1' incorporates:
       *  Gain: '<S219>/Gain1'
       *  Gain: '<S219>/Gain2'
       *  Integrator: '<S219>/Integrator'
       *  Integrator: '<S220>/Integrator'
       */
      rtb_Product_gq = TX_DriveLine_P.TorsionalCompliance1_b_f * TX_DriveLine_X.Integrator_CSTATE_ny +
                       TX_DriveLine_P.TorsionalCompliance1_k_a * TX_DriveLine_X.Integrator_CSTATE_hz;

      /* Switch: '<S191>/Switch' incorporates:
       *  Constant: '<S191>/Constant'
       *  Constant: '<S191>/Constant1'
       *  UnaryMinus: '<S191>/Unary Minus'
       */
      if (TX_DriveLine_P.Constant_Value_os > TX_DriveLine_P.Switch_Threshold_a5) {
        rtb_SumofElements_f = TX_DriveLine_P.Constant1_Value_p;
      } else {
        rtb_SumofElements_f = -TX_DriveLine_P.Constant1_Value_p;
      }

      /* End of Switch: '<S191>/Switch' */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        TX_DriveLine_B.VectorConcatenate_f[0] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
        TX_DriveLine_B.VectorConcatenate_f[1] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
      }

      /* Integrator: '<S185>/Integrator' incorporates:
       *  Concatenate: '<S185>/Vector Concatenate'
       *  Constant: '<S185>/Constant'
       *  Constant: '<S185>/Constant1'
       */
      /* Limited  Integrator  */
      if (TX_DriveLine_DW.Integrator_IWORK_f != 0) {
        TX_DriveLine_X.Integrator_CSTATE_j2[0] = TX_DriveLine_B.VectorConcatenate_f[0];
        TX_DriveLine_X.Integrator_CSTATE_j2[1] = TX_DriveLine_B.VectorConcatenate_f[1];
      }

      /* Gain: '<S191>/Gain' */
      rtb_Gain_p = TX_DriveLine_P.ratio_diff_rear / 2.0;

      /* Integrator: '<S185>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_j2[0] >= TX_DriveLine_P.Integrator_UpperSat_a) {
        TX_DriveLine_X.Integrator_CSTATE_j2[0] = TX_DriveLine_P.Integrator_UpperSat_a;
      } else if (TX_DriveLine_X.Integrator_CSTATE_j2[0] <= TX_DriveLine_P.Integrator_LowerSat_j) {
        TX_DriveLine_X.Integrator_CSTATE_j2[0] = TX_DriveLine_P.Integrator_LowerSat_j;
      }

      /* Integrator: '<S185>/Integrator' */
      rtb_Integrator_p[0] = TX_DriveLine_X.Integrator_CSTATE_j2[0];

      /* Integrator: '<S185>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_j2[1] >= TX_DriveLine_P.Integrator_UpperSat_a) {
        TX_DriveLine_X.Integrator_CSTATE_j2[1] = TX_DriveLine_P.Integrator_UpperSat_a;
      } else if (TX_DriveLine_X.Integrator_CSTATE_j2[1] <= TX_DriveLine_P.Integrator_LowerSat_j) {
        TX_DriveLine_X.Integrator_CSTATE_j2[1] = TX_DriveLine_P.Integrator_LowerSat_j;
      }

      /* Integrator: '<S185>/Integrator' */
      rtb_Integrator_p[1] = TX_DriveLine_X.Integrator_CSTATE_j2[1];

      /* Sum: '<S191>/Sum of Elements' incorporates:
       *  Gain: '<S191>/Gain'
       *  Product: '<S191>/Product'
       */
      rtb_SumofElements_f = rtb_SumofElements_f * rtb_Integrator_p[0] * rtb_Gain_p +
                            rtb_SumofElements_f * rtb_Integrator_p[1] * rtb_Gain_p;

      /* Sum: '<S191>/Add' incorporates:
       *  Gain: '<S191>/Gain1'
       */
      rtb_Subtract1_hs =
          TX_DriveLine_P.Gain1_Gain_o * rtb_Integrator_p[0] - TX_DriveLine_P.Gain1_Gain_o * rtb_Integrator_p[1];

      /* Gain: '<S209>/Gain' incorporates:
       *  Abs: '<S208>/Abs'
       *  Constant: '<S209>/Constant1'
       *  Gain: '<S209>/Gain1'
       *  Gain: '<S209>/Gain2'
       *  SignalConversion generated from: '<S191>/Vector Concatenate'
       *  Sum: '<S209>/Subtract1'
       *  Sum: '<S209>/Subtract2'
       *  Trigonometry: '<S209>/Trigonometric Function'
       */
      rtb_Gain_p =
          (std::tanh((TX_DriveLine_P.Gain1_Gain_n * std::abs(rtb_SumofElements_f) - TX_DriveLine_P.Constant1_Value_hd) *
                     TX_DriveLine_P.Gain2_Gain_mk) +
           TX_DriveLine_P.Constant1_Value_hd) *
          TX_DriveLine_P.Gain_Gain_f1;

      /* Switch: '<S202>/Switch' incorporates:
       *  Constant: '<S202>/Constant'
       *  Constant: '<S210>/Constant'
       *  Product: '<S202>/Product1'
       *  SignalConversion generated from: '<S191>/Vector Concatenate'
       *  UnaryMinus: '<S219>/Unary Minus'
       */
      if (-rtb_Product_gq * rtb_SumofElements_f > TX_DriveLine_P.Switch_Threshold_i) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_l2;
      }

      /* Product: '<S202>/Product4' incorporates:
       *  Constant: '<S208>/Constant'
       *  Constant: '<S209>/Constant1'
       *  Product: '<S208>/Product3'
       *  Product: '<S208>/Product5'
       *  Sum: '<S208>/Subtract1'
       *  Sum: '<S209>/Subtract'
       *  Switch: '<S202>/Switch'
       *  UnaryMinus: '<S219>/Unary Minus'
       */
      rtb_Product4_pl = ((TX_DriveLine_P.Constant1_Value_hd - rtb_Gain_p) * TX_DriveLine_P.Constant_Value_lo +
                         rtb_VectorConcatenate_c_tmp * rtb_Gain_p) *
                        -rtb_Product_gq;

      /* Gain: '<S205>/Gain' incorporates:
       *  Abs: '<S204>/Abs'
       *  Constant: '<S205>/Constant1'
       *  Gain: '<S205>/Gain1'
       *  Gain: '<S205>/Gain2'
       *  Sum: '<S205>/Subtract1'
       *  Sum: '<S205>/Subtract2'
       *  Trigonometry: '<S205>/Trigonometric Function'
       *  UnaryMinus: '<S191>/Unary Minus1'
       */
      rtb_Gain_p = (std::tanh((TX_DriveLine_P.Gain1_Gain_de * std::abs(-rtb_Integrator_p[0]) -
                               TX_DriveLine_P.Constant1_Value_e4) *
                              TX_DriveLine_P.Gain2_Gain_os) +
                    TX_DriveLine_P.Constant1_Value_e4) *
                   TX_DriveLine_P.Gain_Gain_g1;

      /* Switch: '<S200>/Switch' incorporates:
       *  Constant: '<S200>/Constant'
       *  Constant: '<S210>/Constant'
       *  Product: '<S200>/Product1'
       *  UnaryMinus: '<S191>/Unary Minus1'
       *  UnaryMinus: '<S227>/Unary Minus'
       */
      if (-rtb_Subtract1_iv * -rtb_Integrator_p[0] > TX_DriveLine_P.Switch_Threshold_dt) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_d;
      }

      /* Product: '<S200>/Product4' incorporates:
       *  Constant: '<S204>/Constant'
       *  Constant: '<S205>/Constant1'
       *  Product: '<S204>/Product3'
       *  Product: '<S204>/Product5'
       *  Sum: '<S204>/Subtract1'
       *  Sum: '<S205>/Subtract'
       *  Switch: '<S200>/Switch'
       *  UnaryMinus: '<S227>/Unary Minus'
       */
      rtb_Product4_a = ((TX_DriveLine_P.Constant1_Value_e4 - rtb_Gain_p) * TX_DriveLine_P.Constant_Value_ft +
                        rtb_VectorConcatenate_c_tmp * rtb_Gain_p) *
                       -rtb_Subtract1_iv;

      /* Gain: '<S207>/Gain' incorporates:
       *  Abs: '<S206>/Abs'
       *  Constant: '<S207>/Constant1'
       *  Gain: '<S207>/Gain1'
       *  Gain: '<S207>/Gain2'
       *  Sum: '<S207>/Subtract1'
       *  Sum: '<S207>/Subtract2'
       *  Trigonometry: '<S207>/Trigonometric Function'
       *  UnaryMinus: '<S191>/Unary Minus1'
       */
      rtb_Gain_p = (std::tanh((TX_DriveLine_P.Gain1_Gain_hc * std::abs(-rtb_Integrator_p[1]) -
                               TX_DriveLine_P.Constant1_Value_d) *
                              TX_DriveLine_P.Gain2_Gain_d) +
                    TX_DriveLine_P.Constant1_Value_d) *
                   TX_DriveLine_P.Gain_Gain_le;

      /* Switch: '<S201>/Switch' incorporates:
       *  Constant: '<S201>/Constant'
       *  Constant: '<S210>/Constant'
       *  Product: '<S201>/Product1'
       *  UnaryMinus: '<S191>/Unary Minus1'
       *  UnaryMinus: '<S235>/Unary Minus'
       */
      if (-rtb_Subtract1_lh * -rtb_Integrator_p[1] > TX_DriveLine_P.Switch_Threshold_e) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_g;
      }

      /* Product: '<S201>/Product4' incorporates:
       *  Constant: '<S206>/Constant'
       *  Constant: '<S207>/Constant1'
       *  Product: '<S206>/Product3'
       *  Product: '<S206>/Product5'
       *  Sum: '<S206>/Subtract1'
       *  Sum: '<S207>/Subtract'
       *  Switch: '<S201>/Switch'
       *  UnaryMinus: '<S235>/Unary Minus'
       */
      rtb_Product4_ha = ((TX_DriveLine_P.Constant1_Value_d - rtb_Gain_p) * TX_DriveLine_P.Constant_Value_jf +
                         rtb_VectorConcatenate_c_tmp * rtb_Gain_p) *
                        -rtb_Subtract1_lh;

      /* Integrator: '<S187>/Integrator' */
      rtb_Integrator_m = TX_DriveLine_X.Integrator_CSTATE_a;

      /* MATLAB Function: '<S185>/Limited Slip Differential' incorporates:
       *  Constant: '<S185>/Jd'
       *  Constant: '<S185>/Jw1'
       *  Constant: '<S185>/Jw3'
       *  Constant: '<S185>/Ndiff2'
       *  Constant: '<S185>/bd'
       *  Constant: '<S185>/bw1'
       *  Constant: '<S185>/bw2'
       */
      TX_DriveLine_LimitedSlipDifferential(
          rtb_Product4_pl, rtb_Product4_a, rtb_Product4_ha, rtb_Integrator_m,
          TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_bw1, TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_bd,
          TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_bw2, TX_DriveLine_P.ratio_diff_rear,
          TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_Jd, TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_Jw1,
          TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_Jw2, rtb_Integrator_p,
          &TX_DriveLine_B.sf_LimitedSlipDifferential_k, &TX_DriveLine_P.sf_LimitedSlipDifferential_k);

      /* Product: '<S187>/Product' incorporates:
       *  Abs: '<S199>/Abs'
       *  Constant: '<S185>/Constant3'
       *  Constant: '<S199>/Constant'
       *  Constant: '<S199>/Constant1'
       *  Constant: '<S199>/Constant2'
       *  Gain: '<S185>/2*pi'
       *  Gain: '<S199>/Gain'
       *  Lookup_n-D: '<S199>/mu Table'
       *  Product: '<S185>/Product1'
       *  Product: '<S199>/Product'
       *  Sum: '<S187>/Sum'
       *  Trigonometry: '<S199>/Trigonometric Function'
       */
      TX_DriveLine_B.Product_f =
          (TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_Fc *
               TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_Ndisks *
               TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_Reff *
               std::tanh(TX_DriveLine_P.Gain_Gain_b * rtb_Subtract1_hs) *
               look1_binlcpw(std::abs(rtb_Subtract1_hs), TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_dw,
                             TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_muc, 6U) -
           rtb_Integrator_m) *
          (1.0 / TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_tauC * TX_DriveLine_P.upi_Gain_p);

      /* Sum: '<S211>/Subtract' incorporates:
       *  Integrator: '<S158>/Integrator'
       *  UnaryMinus: '<S262>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_l = rtb_SumofElements_c - (-TX_DriveLine_X.Integrator_CSTATE_g3[0]);

      /* Switch: '<S212>/Switch' incorporates:
       *  Integrator: '<S212>/Integrator'
       */
      if (TX_DriveLine_B.Memory_j != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_db;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_az;
      }

      /* Product: '<S212>/Product' incorporates:
       *  Constant: '<S211>/omega_c'
       *  Sum: '<S212>/Sum'
       *  Switch: '<S212>/Switch'
       */
      TX_DriveLine_B.Product_a =
          (TX_DriveLine_B.Subtract_l - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance_omega_c;

      /* Sum: '<S219>/Subtract' incorporates:
       *  Integrator: '<S158>/Integrator'
       *  UnaryMinus: '<S262>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_f = rtb_SumofElements_f - (-TX_DriveLine_X.Integrator_CSTATE_g3[1]);

      /* Switch: '<S220>/Switch' incorporates:
       *  Integrator: '<S220>/Integrator'
       */
      if (TX_DriveLine_B.Memory_la != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_ny;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_n;
      }

      /* Product: '<S220>/Product' incorporates:
       *  Constant: '<S219>/omega_c'
       *  Sum: '<S220>/Sum'
       *  Switch: '<S220>/Switch'
       */
      TX_DriveLine_B.Product_iv =
          (TX_DriveLine_B.Subtract_f - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance1_omega_c_b;

      /* Sum: '<S227>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S191>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_n = -rtb_Integrator_p[0] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[2];

      /* Switch: '<S228>/Switch' incorporates:
       *  Integrator: '<S228>/Integrator'
       */
      if (TX_DriveLine_B.Memory_ly != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_c1;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_b;
      }

      /* Product: '<S228>/Product' incorporates:
       *  Constant: '<S227>/omega_c'
       *  Sum: '<S228>/Sum'
       *  Switch: '<S228>/Switch'
       */
      TX_DriveLine_B.Product_e3 =
          (TX_DriveLine_B.Subtract_n - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance2_omega_c_m;

      /* Sum: '<S235>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S191>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_lk = -rtb_Integrator_p[1] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[3];

      /* Switch: '<S236>/Switch' incorporates:
       *  Integrator: '<S236>/Integrator'
       */
      if (TX_DriveLine_B.Memory_pz != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_bx;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_c;
      }

      /* Product: '<S236>/Product' incorporates:
       *  Constant: '<S235>/omega_c'
       *  Sum: '<S236>/Sum'
       *  Switch: '<S236>/Switch'
       */
      TX_DriveLine_B.Product_pv =
          (TX_DriveLine_B.Subtract_lk - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance3_omega_c;

      /* Sum: '<S243>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S165>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_j = -rtb_Integrator_f[0] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[0];

      /* Switch: '<S244>/Switch' incorporates:
       *  Integrator: '<S244>/Integrator'
       */
      if (TX_DriveLine_B.Memory_c != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_lz;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_a2;
      }

      /* Product: '<S244>/Product' incorporates:
       *  Constant: '<S243>/omega_c'
       *  Sum: '<S244>/Sum'
       *  Switch: '<S244>/Switch'
       */
      TX_DriveLine_B.Product_pr =
          (TX_DriveLine_B.Subtract_j - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance4_omega_c;

      /* Sum: '<S251>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S165>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_d = -rtb_Integrator_f[1] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[1];

      /* Switch: '<S252>/Switch' incorporates:
       *  Integrator: '<S252>/Integrator'
       */
      if (TX_DriveLine_B.Memory_pc != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_bb;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_hw;
      }

      /* Product: '<S252>/Product' incorporates:
       *  Constant: '<S251>/omega_c'
       *  Sum: '<S252>/Sum'
       *  Switch: '<S252>/Switch'
       */
      TX_DriveLine_B.Product_l =
          (TX_DriveLine_B.Subtract_d - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance5_omega_c;

      /* Gain: '<S279>/Gain' incorporates:
       *  Abs: '<S278>/Abs'
       *  Constant: '<S279>/Constant1'
       *  Gain: '<S279>/Gain1'
       *  Gain: '<S279>/Gain2'
       *  SignalConversion generated from: '<S262>/Vector Concatenate'
       *  Sum: '<S279>/Subtract1'
       *  Sum: '<S279>/Subtract2'
       *  Trigonometry: '<S279>/Trigonometric Function'
       */
      rtb_SumofElements_f =
          (std::tanh((TX_DriveLine_P.Gain1_Gain_bu * std::abs(rtb_Add1) - TX_DriveLine_P.Constant1_Value_ff) *
                     TX_DriveLine_P.Gain2_Gain_f) +
           TX_DriveLine_P.Constant1_Value_ff) *
          TX_DriveLine_P.Gain_Gain_d2;

      /* Switch: '<S272>/Switch' incorporates:
       *  Constant: '<S272>/Constant'
       *  Constant: '<S280>/Constant'
       *  Product: '<S272>/Product1'
       *  SignalConversion generated from: '<S262>/Vector Concatenate'
       */
      if (rtb_Subtract1_b0 * rtb_Add1 > TX_DriveLine_P.Switch_Threshold_j) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.TransferCase_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_jo;
      }

      /* Sum: '<S278>/Subtract1' incorporates:
       *  Constant: '<S278>/Constant'
       *  Constant: '<S279>/Constant1'
       *  Product: '<S278>/Product3'
       *  Product: '<S278>/Product5'
       *  Sum: '<S279>/Subtract'
       *  Switch: '<S272>/Switch'
       */
      rtb_SumofElements_c =
          (TX_DriveLine_P.Constant1_Value_ff - rtb_SumofElements_f) * TX_DriveLine_P.Constant_Value_hk +
          rtb_VectorConcatenate_c_tmp * rtb_SumofElements_f;

      /* Gain: '<S275>/Gain' incorporates:
       *  Abs: '<S274>/Abs'
       *  Constant: '<S275>/Constant1'
       *  Gain: '<S275>/Gain1'
       *  Gain: '<S275>/Gain2'
       *  Integrator: '<S158>/Integrator'
       *  Sum: '<S275>/Subtract1'
       *  Sum: '<S275>/Subtract2'
       *  Trigonometry: '<S275>/Trigonometric Function'
       *  UnaryMinus: '<S262>/Unary Minus1'
       */
      rtb_SumofElements_f =
          (std::tanh((TX_DriveLine_P.Gain1_Gain_fc * std::abs(-TX_DriveLine_X.Integrator_CSTATE_g3[0]) -
                      TX_DriveLine_P.Constant1_Value_a) *
                     TX_DriveLine_P.Gain2_Gain_og) +
           TX_DriveLine_P.Constant1_Value_a) *
          TX_DriveLine_P.Gain_Gain_po;

      /* Switch: '<S270>/Switch' incorporates:
       *  Constant: '<S270>/Constant'
       *  Constant: '<S280>/Constant'
       *  Integrator: '<S158>/Integrator'
       *  Product: '<S270>/Product1'
       *  UnaryMinus: '<S262>/Unary Minus1'
       */
      if (rtb_Product6 * -TX_DriveLine_X.Integrator_CSTATE_g3[0] > TX_DriveLine_P.Switch_Threshold_fm) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.TransferCase_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_am;
      }

      /* Sum: '<S274>/Subtract1' incorporates:
       *  Constant: '<S274>/Constant'
       *  Constant: '<S275>/Constant1'
       *  Product: '<S274>/Product3'
       *  Product: '<S274>/Product5'
       *  Sum: '<S275>/Subtract'
       *  Switch: '<S270>/Switch'
       */
      rtb_Gain_p = (TX_DriveLine_P.Constant1_Value_a - rtb_SumofElements_f) * TX_DriveLine_P.Constant_Value_fo +
                   rtb_VectorConcatenate_c_tmp * rtb_SumofElements_f;

      /* Gain: '<S277>/Gain' incorporates:
       *  Abs: '<S276>/Abs'
       *  Constant: '<S277>/Constant1'
       *  Gain: '<S277>/Gain1'
       *  Gain: '<S277>/Gain2'
       *  Integrator: '<S158>/Integrator'
       *  Sum: '<S277>/Subtract1'
       *  Sum: '<S277>/Subtract2'
       *  Trigonometry: '<S277>/Trigonometric Function'
       *  UnaryMinus: '<S262>/Unary Minus1'
       */
      rtb_SumofElements_f =
          (std::tanh((TX_DriveLine_P.Gain1_Gain_cd * std::abs(-TX_DriveLine_X.Integrator_CSTATE_g3[1]) -
                      TX_DriveLine_P.Constant1_Value_gs) *
                     TX_DriveLine_P.Gain2_Gain_ek) +
           TX_DriveLine_P.Constant1_Value_gs) *
          TX_DriveLine_P.Gain_Gain_kj;

      /* SignalConversion generated from: '<S261>/ SFunction ' incorporates:
       *  MATLAB Function: '<S158>/TransferCase'
       *  Product: '<S270>/Product4'
       *  Product: '<S272>/Product4'
       */
      rtb_Subtract1_dg = rtb_SumofElements_c * rtb_Subtract1_b0;
      rtb_Product6 *= rtb_Gain_p;

      /* Switch: '<S271>/Switch' incorporates:
       *  Constant: '<S271>/Constant'
       *  Constant: '<S280>/Constant'
       *  Integrator: '<S158>/Integrator'
       *  Product: '<S271>/Product1'
       *  UnaryMinus: '<S262>/Unary Minus1'
       */
      if (rtb_Product_gq * -TX_DriveLine_X.Integrator_CSTATE_g3[1] > TX_DriveLine_P.Switch_Threshold_dtz) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.TransferCase_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_mj;
      }

      /* SignalConversion generated from: '<S261>/ SFunction ' incorporates:
       *  Constant: '<S276>/Constant'
       *  Constant: '<S277>/Constant1'
       *  MATLAB Function: '<S158>/TransferCase'
       *  Product: '<S271>/Product4'
       *  Product: '<S276>/Product3'
       *  Product: '<S276>/Product5'
       *  Sum: '<S276>/Subtract1'
       *  Sum: '<S277>/Subtract'
       *  Switch: '<S271>/Switch'
       */
      rtb_Product_gq *= (TX_DriveLine_P.Constant1_Value_gs - rtb_SumofElements_f) * TX_DriveLine_P.Constant_Value_dn +
                        rtb_VectorConcatenate_c_tmp * rtb_SumofElements_f;

      /* MATLAB Function: '<S158>/TransferCase' incorporates:
       *  Constant: '<S158>/Jd'
       *  Constant: '<S158>/Jw1'
       *  Constant: '<S158>/Jw3'
       *  Constant: '<S158>/Ndiff2'
       *  Constant: '<S158>/SpdLockConstantConstant'
       *  Constant: '<S158>/TrqSplitRatioConstantConstant'
       *  Constant: '<S158>/bd'
       *  Constant: '<S158>/bw1'
       *  Constant: '<S158>/bw2'
       *  Integrator: '<S158>/Integrator'
       */
      if (TX_DriveLine_P.TransferCase_SpdLock == 1.0) {
        if (TX_DriveLine_P.TransferCase_shaftSwitchMask == 1.0) {
          rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Ndiff;
          rtb_SumofElements_f = rtb_VectorConcatenate_c_tmp * TX_DriveLine_P.TransferCase_Jd;
          rtb_SumofElements_f = 1.0 / ((rtb_SumofElements_f * TX_DriveLine_P.TransferCase_TrqSplitRatio +
                                        (TX_DriveLine_P.TransferCase_Jw1 + TX_DriveLine_P.TransferCase_Jw2)) +
                                       (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio) * rtb_SumofElements_f);
          rtb_SumofElements_f = -((rtb_VectorConcatenate_c_tmp * TX_DriveLine_P.TransferCase_TrqSplitRatio *
                                       TX_DriveLine_P.TransferCase_bd +
                                   (TX_DriveLine_P.TransferCase_bw1 + TX_DriveLine_P.TransferCase_bw2)) +
                                  (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio) * rtb_VectorConcatenate_c_tmp *
                                      TX_DriveLine_P.TransferCase_bd) *
                                    rtb_SumofElements_f * TX_DriveLine_X.Integrator_CSTATE_g3[0] +
                                ((rtb_SumofElements_f * -TX_DriveLine_P.TransferCase_Ndiff * rtb_Subtract1_dg +
                                  -rtb_SumofElements_f * rtb_Product6) +
                                 -rtb_SumofElements_f * rtb_Product_gq);
          TX_DriveLine_B.xdot[0] = rtb_SumofElements_f;
          TX_DriveLine_B.xdot[1] = rtb_SumofElements_f;
        } else {
          rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Ndiff;
          rtb_SumofElements_f = rtb_VectorConcatenate_c_tmp * TX_DriveLine_P.TransferCase_Jd;
          rtb_SumofElements_f = 1.0 / ((rtb_SumofElements_f * TX_DriveLine_P.TransferCase_TrqSplitRatio +
                                        (TX_DriveLine_P.TransferCase_Jw1 + TX_DriveLine_P.TransferCase_Jw2)) +
                                       (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio) * rtb_SumofElements_f);
          rtb_SumofElements_f = -((rtb_VectorConcatenate_c_tmp * TX_DriveLine_P.TransferCase_TrqSplitRatio *
                                       TX_DriveLine_P.TransferCase_bd +
                                   (TX_DriveLine_P.TransferCase_bw1 + TX_DriveLine_P.TransferCase_bw2)) +
                                  (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio) * rtb_VectorConcatenate_c_tmp *
                                      TX_DriveLine_P.TransferCase_bd) *
                                    rtb_SumofElements_f * TX_DriveLine_X.Integrator_CSTATE_g3[0] +
                                ((rtb_SumofElements_f * TX_DriveLine_P.TransferCase_Ndiff * rtb_Subtract1_dg +
                                  -rtb_SumofElements_f * rtb_Product6) +
                                 -rtb_SumofElements_f * rtb_Product_gq);
          TX_DriveLine_B.xdot[0] = rtb_SumofElements_f;
          TX_DriveLine_B.xdot[1] = rtb_SumofElements_f;
        }
      } else if (TX_DriveLine_P.TransferCase_shaftSwitchMask == 1.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Ndiff;
        rtb_Subtract1_bs =
            (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio) * (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio);
        rtb_Subtract1_k = TX_DriveLine_P.TransferCase_TrqSplitRatio * TX_DriveLine_P.TransferCase_TrqSplitRatio;
        rtb_Subtract1_hs = (TX_DriveLine_P.TransferCase_Jw2 * TX_DriveLine_P.TransferCase_Jd *
                                rtb_VectorConcatenate_c_tmp * rtb_Subtract1_k +
                            TX_DriveLine_P.TransferCase_Jw1 * TX_DriveLine_P.TransferCase_Jd *
                                rtb_VectorConcatenate_c_tmp * rtb_Subtract1_bs) +
                           TX_DriveLine_P.TransferCase_Jw1 * TX_DriveLine_P.TransferCase_Jw2;
        rtb_Gain_p =
            TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Jd;
        rtb_VectorConcatenate_h[0] =
            (rtb_Gain_p * rtb_Subtract1_bs + TX_DriveLine_P.TransferCase_Jw2) / rtb_Subtract1_hs;
        rtb_SumofElements_f =
            -(TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Jd *
              TX_DriveLine_P.TransferCase_TrqSplitRatio * (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio)) /
            rtb_Subtract1_hs;
        rtb_VectorConcatenate_h[2] = rtb_SumofElements_f;
        rtb_VectorConcatenate_h[1] = rtb_SumofElements_f;
        rtb_VectorConcatenate_h[3] =
            (rtb_Gain_p * rtb_Subtract1_k + TX_DriveLine_P.TransferCase_Jw1) / rtb_Subtract1_hs;
        tmp[0] = -(TX_DriveLine_P.TransferCase_TrqSplitRatio * TX_DriveLine_P.TransferCase_Ndiff);
        tmp[2] = -1.0;
        tmp[4] = 0.0;
        tmp[1] = -((1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio) * TX_DriveLine_P.TransferCase_Ndiff);
        tmp[3] = 0.0;
        tmp[5] = -1.0;
        rtb_Gain_p = -(rtb_Subtract1_k * rtb_VectorConcatenate_c_tmp * TX_DriveLine_P.TransferCase_bd +
                       TX_DriveLine_P.TransferCase_bw1);
        rtb_Subtract1_k =
            -((1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio) * TX_DriveLine_P.TransferCase_TrqSplitRatio *
              rtb_VectorConcatenate_c_tmp * TX_DriveLine_P.TransferCase_bd);
        rtb_Subtract1_hs =
            -(rtb_VectorConcatenate_c_tmp * TX_DriveLine_P.TransferCase_bd * TX_DriveLine_P.TransferCase_TrqSplitRatio *
              (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio));
        rtb_VectorConcatenate_c_tmp =
            -(rtb_Subtract1_bs * rtb_VectorConcatenate_c_tmp * TX_DriveLine_P.TransferCase_bd +
              TX_DriveLine_P.TransferCase_bw2);
        for (i = 0; i < 2; i++) {
          for (int32_T i_0{0}; i_0 < 3; i_0++) {
            int32_T rtb_VectorConcatenate_c_tmp_0;
            int32_T rtb_VectorConcatenate_c_tmp_tmp;
            rtb_VectorConcatenate_c_tmp_tmp = i_0 << 1;
            rtb_VectorConcatenate_c_tmp_0 = i + rtb_VectorConcatenate_c_tmp_tmp;
            rtb_VectorConcatenate_c[rtb_VectorConcatenate_c_tmp_0] = 0.0;
            rtb_VectorConcatenate_c[rtb_VectorConcatenate_c_tmp_0] +=
                tmp[rtb_VectorConcatenate_c_tmp_tmp] * rtb_VectorConcatenate_h[i];
            rtb_VectorConcatenate_c[rtb_VectorConcatenate_c_tmp_0] +=
                tmp[rtb_VectorConcatenate_c_tmp_tmp + 1] * rtb_VectorConcatenate_h[i + 2];
          }

          rtb_VectorConcatenate_c_0[i] = 0.0;
          rtb_VectorConcatenate_c_0[i] += rtb_VectorConcatenate_h[i] * rtb_Gain_p;
          rtb_SumofElements_f = rtb_VectorConcatenate_h[i + 2];
          rtb_VectorConcatenate_c_0[i] += rtb_SumofElements_f * rtb_Subtract1_hs;
          rtb_VectorConcatenate_c_0[i + 2] = 0.0;
          rtb_VectorConcatenate_c_0[i + 2] += rtb_VectorConcatenate_h[i] * rtb_Subtract1_k;
          rtb_VectorConcatenate_c_0[i + 2] += rtb_SumofElements_f * rtb_VectorConcatenate_c_tmp;
          rtb_VectorConcatenate_c_1[i] = 0.0;
          rtb_VectorConcatenate_c_1[i] += rtb_VectorConcatenate_c[i] * rtb_Subtract1_dg;
          rtb_VectorConcatenate_c_1[i] += rtb_VectorConcatenate_c[i + 2] * rtb_Product6;
          rtb_VectorConcatenate_c_1[i] += rtb_VectorConcatenate_c[i + 4] * rtb_Product_gq;
          rtb_VectorConcatenate_c_2[i] = 0.0;
          rtb_VectorConcatenate_c_2[i] += rtb_VectorConcatenate_c_0[i] * TX_DriveLine_X.Integrator_CSTATE_g3[0];
          rtb_VectorConcatenate_c_2[i] += rtb_VectorConcatenate_c_0[i + 2] * TX_DriveLine_X.Integrator_CSTATE_g3[1];
          TX_DriveLine_B.xdot[i] = rtb_VectorConcatenate_c_1[i] + rtb_VectorConcatenate_c_2[i];
        }
      } else {
        rtb_SumofElements_f = TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Ndiff;
        rtb_VectorConcatenate_c_tmp =
            (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio) * (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio);
        rtb_Gain_p = TX_DriveLine_P.TransferCase_TrqSplitRatio * TX_DriveLine_P.TransferCase_TrqSplitRatio;
        rtb_Subtract1_k =
            (TX_DriveLine_P.TransferCase_Jw2 * TX_DriveLine_P.TransferCase_Jd * rtb_SumofElements_f * rtb_Gain_p +
             TX_DriveLine_P.TransferCase_Jw1 * TX_DriveLine_P.TransferCase_Jd * rtb_SumofElements_f *
                 rtb_VectorConcatenate_c_tmp) +
            TX_DriveLine_P.TransferCase_Jw1 * TX_DriveLine_P.TransferCase_Jw2;
        rtb_Subtract1_hs =
            TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Jd;
        rtb_VectorConcatenate_h[0] =
            (rtb_Subtract1_hs * rtb_VectorConcatenate_c_tmp + TX_DriveLine_P.TransferCase_Jw2) / rtb_Subtract1_k;
        rtb_Subtract1_bs =
            -(TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Ndiff * TX_DriveLine_P.TransferCase_Jd *
              TX_DriveLine_P.TransferCase_TrqSplitRatio * (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio)) /
            rtb_Subtract1_k;
        rtb_VectorConcatenate_h[2] = rtb_Subtract1_bs;
        rtb_VectorConcatenate_h[1] = rtb_Subtract1_bs;
        rtb_VectorConcatenate_h[3] =
            (rtb_Subtract1_hs * rtb_Gain_p + TX_DriveLine_P.TransferCase_Jw1) / rtb_Subtract1_k;
        tmp[0] = TX_DriveLine_P.TransferCase_TrqSplitRatio * TX_DriveLine_P.TransferCase_Ndiff;
        tmp[2] = -1.0;
        tmp[4] = 0.0;
        tmp[1] = (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio) * TX_DriveLine_P.TransferCase_Ndiff;
        tmp[3] = 0.0;
        tmp[5] = -1.0;
        rtb_Gain_p =
            -(rtb_Gain_p * rtb_SumofElements_f * TX_DriveLine_P.TransferCase_bd + TX_DriveLine_P.TransferCase_bw1);
        rtb_Subtract1_k =
            -((1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio) * TX_DriveLine_P.TransferCase_TrqSplitRatio *
              rtb_SumofElements_f * TX_DriveLine_P.TransferCase_bd);
        rtb_Subtract1_hs =
            -(rtb_SumofElements_f * TX_DriveLine_P.TransferCase_bd * TX_DriveLine_P.TransferCase_TrqSplitRatio *
              (1.0 - TX_DriveLine_P.TransferCase_TrqSplitRatio));
        rtb_VectorConcatenate_c_tmp =
            -(rtb_VectorConcatenate_c_tmp * rtb_SumofElements_f * TX_DriveLine_P.TransferCase_bd +
              TX_DriveLine_P.TransferCase_bw2);
        for (i = 0; i < 2; i++) {
          for (int32_T i_0{0}; i_0 < 3; i_0++) {
            int32_T rtb_VectorConcatenate_c_tmp_0;
            int32_T rtb_VectorConcatenate_c_tmp_tmp;
            rtb_VectorConcatenate_c_tmp_tmp = i_0 << 1;
            rtb_VectorConcatenate_c_tmp_0 = i + rtb_VectorConcatenate_c_tmp_tmp;
            rtb_VectorConcatenate_c[rtb_VectorConcatenate_c_tmp_0] = 0.0;
            rtb_VectorConcatenate_c[rtb_VectorConcatenate_c_tmp_0] +=
                tmp[rtb_VectorConcatenate_c_tmp_tmp] * rtb_VectorConcatenate_h[i];
            rtb_VectorConcatenate_c[rtb_VectorConcatenate_c_tmp_0] +=
                tmp[rtb_VectorConcatenate_c_tmp_tmp + 1] * rtb_VectorConcatenate_h[i + 2];
          }

          rtb_VectorConcatenate_c_0[i] = 0.0;
          rtb_VectorConcatenate_c_0[i] += rtb_VectorConcatenate_h[i] * rtb_Gain_p;
          rtb_SumofElements_f = rtb_VectorConcatenate_h[i + 2];
          rtb_VectorConcatenate_c_0[i] += rtb_SumofElements_f * rtb_Subtract1_hs;
          rtb_VectorConcatenate_c_0[i + 2] = 0.0;
          rtb_VectorConcatenate_c_0[i + 2] += rtb_VectorConcatenate_h[i] * rtb_Subtract1_k;
          rtb_VectorConcatenate_c_0[i + 2] += rtb_SumofElements_f * rtb_VectorConcatenate_c_tmp;
          rtb_VectorConcatenate_c_1[i] = 0.0;
          rtb_VectorConcatenate_c_1[i] += rtb_VectorConcatenate_c[i] * rtb_Subtract1_dg;
          rtb_VectorConcatenate_c_1[i] += rtb_VectorConcatenate_c[i + 2] * rtb_Product6;
          rtb_VectorConcatenate_c_1[i] += rtb_VectorConcatenate_c[i + 4] * rtb_Product_gq;
          rtb_VectorConcatenate_c_2[i] = 0.0;
          rtb_VectorConcatenate_c_2[i] += rtb_VectorConcatenate_c_0[i] * TX_DriveLine_X.Integrator_CSTATE_g3[0];
          rtb_VectorConcatenate_c_2[i] += rtb_VectorConcatenate_c_0[i + 2] * TX_DriveLine_X.Integrator_CSTATE_g3[1];
          TX_DriveLine_B.xdot[i] = rtb_VectorConcatenate_c_1[i] + rtb_VectorConcatenate_c_2[i];
        }
      }

      /* Sum: '<S290>/Add' incorporates:
       *  Gain: '<S290>/Gain1'
       */
      rtb_Gain_p =
          TX_DriveLine_P.Gain1_Gain_nn * rtb_Integrator_l[0] - TX_DriveLine_P.Gain1_Gain_nn * rtb_Integrator_l[1];

      /* Gain: '<S308>/Gain' incorporates:
       *  Abs: '<S307>/Abs'
       *  Constant: '<S308>/Constant1'
       *  Gain: '<S308>/Gain1'
       *  Gain: '<S308>/Gain2'
       *  SignalConversion generated from: '<S290>/Vector Concatenate'
       *  Sum: '<S308>/Subtract1'
       *  Sum: '<S308>/Subtract2'
       *  Trigonometry: '<S308>/Trigonometric Function'
       */
      rtb_Subtract1_hs = (std::tanh((TX_DriveLine_P.Gain1_Gain_hh * std::abs(rtb_SumofElements_l3) -
                                     TX_DriveLine_P.Constant1_Value_hv) *
                                    TX_DriveLine_P.Gain2_Gain_fz) +
                          TX_DriveLine_P.Constant1_Value_hv) *
                         TX_DriveLine_P.Gain_Gain_a;

      /* Switch: '<S301>/Switch' incorporates:
       *  Constant: '<S301>/Constant'
       *  Constant: '<S309>/Constant'
       *  Product: '<S301>/Product1'
       *  SignalConversion generated from: '<S290>/Vector Concatenate'
       */
      if (rtb_Subtract1_b0 * rtb_SumofElements_l3 > TX_DriveLine_P.Switch_Threshold_nk) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.ICERearLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_jd;
      }

      /* Product: '<S301>/Product4' incorporates:
       *  Constant: '<S307>/Constant'
       *  Constant: '<S308>/Constant1'
       *  Product: '<S307>/Product3'
       *  Product: '<S307>/Product5'
       *  Sum: '<S307>/Subtract1'
       *  Sum: '<S308>/Subtract'
       *  Switch: '<S301>/Switch'
       */
      rtb_Product4_go = ((TX_DriveLine_P.Constant1_Value_hv - rtb_Subtract1_hs) * TX_DriveLine_P.Constant_Value_gb +
                         rtb_VectorConcatenate_c_tmp * rtb_Subtract1_hs) *
                        rtb_Subtract1_b0;

      /* Gain: '<S304>/Gain' incorporates:
       *  Abs: '<S303>/Abs'
       *  Constant: '<S304>/Constant1'
       *  Gain: '<S304>/Gain1'
       *  Gain: '<S304>/Gain2'
       *  Sum: '<S304>/Subtract1'
       *  Sum: '<S304>/Subtract2'
       *  Trigonometry: '<S304>/Trigonometric Function'
       *  UnaryMinus: '<S290>/Unary Minus1'
       */
      rtb_SumofElements_f = (std::tanh((TX_DriveLine_P.Gain1_Gain_de0 * std::abs(-rtb_Integrator_l[0]) -
                                        TX_DriveLine_P.Constant1_Value_fb) *
                                       TX_DriveLine_P.Gain2_Gain_nt) +
                             TX_DriveLine_P.Constant1_Value_fb) *
                            TX_DriveLine_P.Gain_Gain_h;

      /* Switch: '<S299>/Switch' incorporates:
       *  Constant: '<S299>/Constant'
       *  Constant: '<S309>/Constant'
       *  Product: '<S299>/Product1'
       *  UnaryMinus: '<S290>/Unary Minus1'
       *  UnaryMinus: '<S310>/Unary Minus'
       */
      if (-rtb_Product1_n * -rtb_Integrator_l[0] > TX_DriveLine_P.Switch_Threshold_gb) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.ICERearLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_e;
      }

      /* Product: '<S299>/Product4' incorporates:
       *  Constant: '<S303>/Constant'
       *  Constant: '<S304>/Constant1'
       *  Product: '<S303>/Product3'
       *  Product: '<S303>/Product5'
       *  Sum: '<S303>/Subtract1'
       *  Sum: '<S304>/Subtract'
       *  Switch: '<S299>/Switch'
       *  UnaryMinus: '<S310>/Unary Minus'
       */
      rtb_Product4_po = ((TX_DriveLine_P.Constant1_Value_fb - rtb_SumofElements_f) * TX_DriveLine_P.Constant_Value_hz +
                         rtb_VectorConcatenate_c_tmp * rtb_SumofElements_f) *
                        -rtb_Product1_n;

      /* Gain: '<S306>/Gain' incorporates:
       *  Abs: '<S305>/Abs'
       *  Constant: '<S306>/Constant1'
       *  Gain: '<S306>/Gain1'
       *  Gain: '<S306>/Gain2'
       *  Sum: '<S306>/Subtract1'
       *  Sum: '<S306>/Subtract2'
       *  Trigonometry: '<S306>/Trigonometric Function'
       *  UnaryMinus: '<S290>/Unary Minus1'
       */
      rtb_SumofElements_f = (std::tanh((TX_DriveLine_P.Gain1_Gain_k * std::abs(-rtb_Integrator_l[1]) -
                                        TX_DriveLine_P.Constant1_Value_dz) *
                                       TX_DriveLine_P.Gain2_Gain_g) +
                             TX_DriveLine_P.Constant1_Value_dz) *
                            TX_DriveLine_P.Gain_Gain_o;

      /* Switch: '<S300>/Switch' incorporates:
       *  Constant: '<S300>/Constant'
       *  Constant: '<S309>/Constant'
       *  Product: '<S300>/Product1'
       *  UnaryMinus: '<S290>/Unary Minus1'
       *  UnaryMinus: '<S318>/Unary Minus'
       */
      if (-rtb_Sum_dv * -rtb_Integrator_l[1] > TX_DriveLine_P.Switch_Threshold_ch) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.ICERearLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_as;
      }

      /* Product: '<S300>/Product4' incorporates:
       *  Constant: '<S305>/Constant'
       *  Constant: '<S306>/Constant1'
       *  Product: '<S305>/Product3'
       *  Product: '<S305>/Product5'
       *  Sum: '<S305>/Subtract1'
       *  Sum: '<S306>/Subtract'
       *  Switch: '<S300>/Switch'
       *  UnaryMinus: '<S318>/Unary Minus'
       */
      rtb_Product4_ni = ((TX_DriveLine_P.Constant1_Value_dz - rtb_SumofElements_f) * TX_DriveLine_P.Constant_Value_i1s +
                         rtb_VectorConcatenate_c_tmp * rtb_SumofElements_f) *
                        -rtb_Sum_dv;

      /* Integrator: '<S286>/Integrator' */
      rtb_Integrator_m0 = TX_DriveLine_X.Integrator_CSTATE_a4;

      /* MATLAB Function: '<S284>/Limited Slip Differential' incorporates:
       *  Constant: '<S284>/Jd'
       *  Constant: '<S284>/Jw1'
       *  Constant: '<S284>/Jw3'
       *  Constant: '<S284>/Ndiff2'
       *  Constant: '<S284>/bd'
       *  Constant: '<S284>/bw1'
       *  Constant: '<S284>/bw2'
       */
      TX_DriveLine_LimitedSlipDifferential(
          rtb_Product4_go, rtb_Product4_po, rtb_Product4_ni, rtb_Integrator_m0,
          TX_DriveLine_P.ICERearLimitedSlipDifferential_bw1, TX_DriveLine_P.ICERearLimitedSlipDifferential_bd,
          TX_DriveLine_P.ICERearLimitedSlipDifferential_bw2, TX_DriveLine_P.ratio_diff_rear,
          TX_DriveLine_P.ICERearLimitedSlipDifferential_Jd, TX_DriveLine_P.ICERearLimitedSlipDifferential_Jw1,
          TX_DriveLine_P.ICERearLimitedSlipDifferential_Jw2, rtb_Integrator_l,
          &TX_DriveLine_B.sf_LimitedSlipDifferential_b, &TX_DriveLine_P.sf_LimitedSlipDifferential_b);

      /* Product: '<S286>/Product' incorporates:
       *  Abs: '<S298>/Abs'
       *  Constant: '<S284>/Constant3'
       *  Constant: '<S298>/Constant'
       *  Constant: '<S298>/Constant1'
       *  Constant: '<S298>/Constant2'
       *  Gain: '<S284>/2*pi'
       *  Gain: '<S298>/Gain'
       *  Lookup_n-D: '<S298>/mu Table'
       *  Product: '<S284>/Product1'
       *  Product: '<S298>/Product'
       *  Sum: '<S286>/Sum'
       *  Trigonometry: '<S298>/Trigonometric Function'
       */
      TX_DriveLine_B.Product_h1 =
          (TX_DriveLine_P.ICERearLimitedSlipDifferential_Fc * TX_DriveLine_P.ICERearLimitedSlipDifferential_Ndisks *
               TX_DriveLine_P.ICERearLimitedSlipDifferential_Reff *
               std::tanh(TX_DriveLine_P.Gain_Gain_d2j * rtb_Gain_p) *
               look1_binlxpw(std::abs(rtb_Gain_p), TX_DriveLine_P.ICERearLimitedSlipDifferential_dw,
                             TX_DriveLine_P.ICERearLimitedSlipDifferential_muc, 7U) -
           rtb_Integrator_m0) *
          (1.0 / TX_DriveLine_P.ICERearLimitedSlipDifferential_tauC * TX_DriveLine_P.upi_Gain_b);

      /* Sum: '<S310>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S290>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_hz = -rtb_Integrator_l[0] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[2];

      /* Switch: '<S311>/Switch' incorporates:
       *  Integrator: '<S311>/Integrator'
       */
      if (TX_DriveLine_B.Memory_f != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_ng;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_k;
      }

      /* Product: '<S311>/Product' incorporates:
       *  Constant: '<S310>/omega_c'
       *  Sum: '<S311>/Sum'
       *  Switch: '<S311>/Switch'
       */
      TX_DriveLine_B.Product_lx =
          (TX_DriveLine_B.Subtract_hz - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance_omega_c_b;

      /* Sum: '<S318>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S290>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_p = -rtb_Integrator_l[1] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[3];

      /* Switch: '<S319>/Switch' incorporates:
       *  Integrator: '<S319>/Integrator'
       */
      if (TX_DriveLine_B.Memory_g != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_hk;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_kv;
      }

      /* Product: '<S319>/Product' incorporates:
       *  Constant: '<S318>/omega_c'
       *  Sum: '<S319>/Sum'
       *  Switch: '<S319>/Switch'
       */
      TX_DriveLine_B.Product_af =
          (TX_DriveLine_B.Subtract_p - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance1_omega_c_k;

      /* Integrator: '<S382>/Integrator1' */
      TX_DriveLine_B.Integrator1 = TX_DriveLine_X.Integrator1_CSTATE;

      /* Logic: '<S378>/Logical Operator' incorporates:
       *  Constant: '<S383>/Constant'
       *  Constant: '<S384>/Constant'
       *  Inport: '<Root>/DriveLineIn'
       *  RelationalOperator: '<S383>/Compare'
       *  RelationalOperator: '<S384>/Compare'
       */
      TX_DriveLine_B.gearMode =
          ((TX_DriveLine_U.DriveLineIn.gear_cmd_driver > TX_DriveLine_P.CompareToConstant1_const) &&
           (TX_DriveLine_B.Integrator1 >= TX_DriveLine_P.CompareToConstant_const_n));

      /* Logic: '<S380>/Logical Operator' incorporates:
       *  Constant: '<S386>/Constant'
       *  Constant: '<S387>/Constant'
       *  Inport: '<Root>/DriveLineIn'
       *  RelationalOperator: '<S386>/Compare'
       *  RelationalOperator: '<S387>/Compare'
       */
      TX_DriveLine_B.gearMode_c =
          ((TX_DriveLine_U.DriveLineIn.gear_cmd_driver < TX_DriveLine_P.CompareToConstant1_const_h) &&
           (TX_DriveLine_B.Integrator1 <= TX_DriveLine_P.CompareToConstant_const_m));

      /* Gain: '<S100>/rpm' */
      rtb_Abs_e *= TX_DriveLine_P.rpm_Gain;

      /* Logic: '<S379>/Logical Operator1' incorporates:
       *  Constant: '<S379>/Constant'
       *  Constant: '<S379>/Constant1'
       *  Constant: '<S379>/Constant2'
       *  Constant: '<S385>/Constant'
       *  Inport: '<Root>/DriveLineIn'
       *  Inport: '<Root>/SoftECUOut'
       *  Logic: '<S379>/Logical Operator'
       *  RelationalOperator: '<S379>/Relational Operator'
       *  RelationalOperator: '<S379>/Relational Operator1'
       *  RelationalOperator: '<S379>/Relational Operator2'
       *  RelationalOperator: '<S385>/Compare'
       */
      TX_DriveLine_B.LogicalOperator1 =
          ((TX_DriveLine_U.DriveLineIn.gear_cmd_driver == TX_DriveLine_P.CompareToConstant_const_l) ||
           ((TX_DriveLine_U.SoftECUOut.throttle_01 <= TX_DriveLine_P.Constant1_Value_bi) &&
            (rtb_Abs_e <= TX_DriveLine_P.Constant_Value_kh)) ||
           (rtb_Abs_e <= TX_DriveLine_P.Constant2_Value_k));
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Chart: '<S377>/TCM Shift Controller' */
        if (TX_DriveLine_DW.temporalCounter_i1 < MAX_uint32_T) {
          TX_DriveLine_DW.temporalCounter_i1++;
        }

        TX_DriveLine_DW.sfEvent = -1;
        if (TX_DriveLine_DW.is_active_c3_TX_DriveLine == 0U) {
          TX_DriveLine_DW.is_active_c3_TX_DriveLine = 1U;
          TX_DriveLine_DW.is_active_GearCmd = 1U;
          TX_DriveLine_DW.is_GearCmd = TX_DriveLine_IN_Neutral;
          TX_DriveLine_B.GearCmd = 0;
          TX_DriveLine_DW.is_active_SelectionState = 1U;
          TX_DriveLine_DW.is_SelectionState = TX_DriveLine_IN_SteadyState;
        } else {
          if (TX_DriveLine_DW.is_active_GearCmd != 0U) {
            TX_DriveLine_GearCmd();
          }

          if (TX_DriveLine_DW.is_active_SelectionState != 0U) {
            /* Outputs for Function Call SubSystem: '<S381>/calc_up' */
            /* Lookup_n-D: '<S390>/Calculate Upshift Threshold' incorporates:
             *  Inport: '<Root>/SoftECUOut'
             */
            rtb_SumofElements_l3 = look2_binlcapw(
                TX_DriveLine_U.SoftECUOut.throttle_01, static_cast<real_T>(TX_DriveLine_B.GearCmd),
                TX_DriveLine_P.Pedal_Positions_UpShift, TX_DriveLine_P.Up_Gears, TX_DriveLine_P.Upshift_Speeds,
                TX_DriveLine_P.CalculateUpshiftThreshold_maxIndex, m_upshift_accload_points_num);

            /* End of Outputs for SubSystem: '<S381>/calc_up' */

            /* Outputs for Function Call SubSystem: '<S381>/calc_down' */
            /* Lookup_n-D: '<S389>/Calculate  Downshift Threshold' incorporates:
             *  Inport: '<Root>/SoftECUOut'
             */
            rtb_Subtract1_b0 = look2_binlcapw(
                TX_DriveLine_U.SoftECUOut.throttle_01, static_cast<real_T>(TX_DriveLine_B.GearCmd),
                TX_DriveLine_P.Pedal_Positions_DnShift, TX_DriveLine_P.Dn_Gears, TX_DriveLine_P.Downshift_Speeds,
                TX_DriveLine_P.CalculateDownshiftThreshold_maxIndex, m_downshift_accload_points_num);

            /* End of Outputs for SubSystem: '<S381>/calc_down' */
            switch (TX_DriveLine_DW.is_SelectionState) {
              case TX_DriveLine_IN_DownShifting:
                i = TX_DriveLine_DW.sfEvent;
                TX_DriveLine_DW.sfEvent = TX_DriveLine_event_Down;
                if (TX_DriveLine_DW.is_active_GearCmd != 0U) {
                  TX_DriveLine_GearCmd();
                }

                TX_DriveLine_DW.sfEvent = i;
                TX_DriveLine_DW.is_SelectionState = TX_DriveLine_IN_SteadyState;
                break;

              case TX_DriveLine_IN_SteadyState:
                if ((TX_DriveLine_B.Integrator1 < rtb_Subtract1_b0) && (TX_DriveLine_B.GearCmd > 1)) {
                  TX_DriveLine_DW.is_SelectionState = TX_DriveLine_IN_preDownShifting;
                  TX_DriveLine_DW.temporalCounter_i1 = 0U;
                } else if ((TX_DriveLine_B.Integrator1 > rtb_SumofElements_l3) && (TX_DriveLine_B.GearCmd <= 7)) {
                  TX_DriveLine_DW.is_SelectionState = TX_DriveLine_IN_preUpShifting;
                  TX_DriveLine_DW.temporalCounter_i1 = 0U;
                }
                break;

              case TX_DriveLine_IN_UpShifting:
                i = TX_DriveLine_DW.sfEvent;
                TX_DriveLine_DW.sfEvent = TX_DriveLine_event_Up;
                if (TX_DriveLine_DW.is_active_GearCmd != 0U) {
                  TX_DriveLine_GearCmd();
                }

                TX_DriveLine_DW.sfEvent = i;
                TX_DriveLine_DW.is_SelectionState = TX_DriveLine_IN_SteadyState;
                break;

              case TX_DriveLine_IN_preDownShifting:
                if ((TX_DriveLine_DW.temporalCounter_i1 >=
                     static_cast<uint32_T>(std::ceil(TX_DriveLine_P.pre_shift_wait_time * 1000.0))) &&
                    (TX_DriveLine_B.Integrator1 < rtb_Subtract1_b0 - 2.0)) {
                  TX_DriveLine_DW.is_SelectionState = TX_DriveLine_IN_DownShifting;
                  TX_DriveLine_B.GearCmd = static_cast<int8_T>(static_cast<real_T>(TX_DriveLine_B.GearCmd) - 0.5);
                } else if (TX_DriveLine_B.Integrator1 >= rtb_Subtract1_b0) {
                  TX_DriveLine_DW.is_SelectionState = TX_DriveLine_IN_SteadyState;
                }
                break;

              case TX_DriveLine_IN_preUpShifting:
                if ((TX_DriveLine_DW.temporalCounter_i1 >=
                     static_cast<uint32_T>(std::ceil(TX_DriveLine_P.pre_shift_wait_time * 1000.0))) &&
                    (TX_DriveLine_B.Integrator1 > rtb_SumofElements_l3)) {
                  TX_DriveLine_DW.is_SelectionState = TX_DriveLine_IN_UpShifting;
                  TX_DriveLine_B.GearCmd = static_cast<int8_T>(static_cast<real_T>(TX_DriveLine_B.GearCmd) + 0.5);
                } else if (TX_DriveLine_B.Integrator1 <= rtb_SumofElements_l3) {
                  TX_DriveLine_DW.is_SelectionState = TX_DriveLine_IN_SteadyState;
                }
                break;

              default:
                /* Unreachable state, for coverage only */
                TX_DriveLine_DW.is_SelectionState = TX_DriveLine_IN_NO_ACTIVE_CHILD;
                break;
            }
          }
        }

        /* End of Chart: '<S377>/TCM Shift Controller' */

        /* UnitDelay: '<S326>/Delay Input1' */
        TX_DriveLine_B.Uk1 = TX_DriveLine_DW.DelayInput1_DSTATE;
      }

      /* DataTypeConversion: '<S98>/Data Type Conversion' */
      TX_DriveLine_B.DataTypeConversion = TX_DriveLine_B.GearCmd;

      /* RelationalOperator: '<S326>/FixPt Relational Operator' */
      TX_DriveLine_B.FixPtRelationalOperator = (TX_DriveLine_B.DataTypeConversion != TX_DriveLine_B.Uk1);
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Chart: '<S98>/hold' incorporates:
         *  Constant: '<S98>/HoldSecond'
         */
        if (TX_DriveLine_DW.temporalCounter_i1_a < MAX_uint32_T) {
          TX_DriveLine_DW.temporalCounter_i1_a++;
        }

        if (TX_DriveLine_DW.is_active_c1_TX_DriveLine == 0U) {
          TX_DriveLine_DW.is_active_c1_TX_DriveLine = 1U;
          TX_DriveLine_DW.is_c1_TX_DriveLine = TX_DriveLine_IN_Normal;
          TX_DriveLine_B.y = TX_DriveLine_B.GearCmd;
        } else if (TX_DriveLine_DW.is_c1_TX_DriveLine == TX_DriveLine_IN_Hold) {
          if (TX_DriveLine_DW.temporalCounter_i1_a >=
              static_cast<uint32_T>(std::ceil(TX_DriveLine_P.HoldSecond_Value * 1000.0))) {
            TX_DriveLine_DW.is_c1_TX_DriveLine = TX_DriveLine_IN_Normal;
            TX_DriveLine_B.y = TX_DriveLine_B.GearCmd;
          }

          /* case IN_Normal: */
        } else if (TX_DriveLine_B.FixPtRelationalOperator) {
          TX_DriveLine_DW.is_c1_TX_DriveLine = TX_DriveLine_IN_Hold;
          TX_DriveLine_DW.temporalCounter_i1_a = 0U;
          TX_DriveLine_B.y = TX_DriveLine_B.GearCmd;
        }

        /* End of Chart: '<S98>/hold' */
      }

      /* Switch: '<S333>/Switch' */
      if (!(TX_DriveLine_B.Memory_l != 0.0)) {
        rtb_Integrator_k3 = TX_DriveLine_B.Constant1;
      }

      /* Product: '<S333>/Product' incorporates:
       *  Constant: '<S331>/Constant'
       *  Gain: '<S331>/2*pi'
       *  Product: '<S331>/Product'
       *  Sum: '<S333>/Sum'
       *  Switch: '<S333>/Switch'
       */
      TX_DriveLine_B.Product_cs = 1.0 / TX_DriveLine_P.Trans.ShiftTau * TX_DriveLine_P.upi_Gain_a4 *
                                  (static_cast<real_T>(TX_DriveLine_B.y) - rtb_Integrator_k3);

      /* Sum: '<S369>/Subtract' */
      TX_DriveLine_B.Subtract_dt = rtb_Product2_d - rtb_Switch;

      /* Switch: '<S370>/Switch' */
      if (!(TX_DriveLine_B.Memory_e != 0.0)) {
        rtb_SumofElements = TX_DriveLine_B.domega_o_g;
      }

      /* Product: '<S370>/Product' incorporates:
       *  Constant: '<S369>/omega_c'
       *  Sum: '<S370>/Sum'
       *  Switch: '<S370>/Switch'
       */
      TX_DriveLine_B.Product_jf = (TX_DriveLine_B.Subtract_dt - rtb_SumofElements) * TX_DriveLine_P.PropShaft_omega_c;

      /* Product: '<S382>/Divide' incorporates:
       *  Constant: '<S382>/Constant'
       *  Gain: '<S377>/Gain'
       *  Gain: '<S377>/kph'
       *  Inport: '<Root>/DriveLineIn'
       *  Sum: '<S377>/Add'
       *  Sum: '<S382>/Sum'
       */
      TX_DriveLine_B.Divide =
          ((((TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[0] + TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[1]) +
             TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[2]) +
            TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[3]) *
               (TX_DriveLine_P.StatLdWhlR[0] / 4.0) * TX_DriveLine_P.kph_Gain -
           TX_DriveLine_B.Integrator1) *
          TX_DriveLine_P.Valve_wc;

      /* End of Outputs for SubSystem: '<S1>/ICEDriveLine' */
    } break;

    case 1: {
      real_T rtb_Gain_p;
      real_T rtb_Integrator_k3;
      real_T rtb_Subtract1_b0;
      real_T rtb_SumofElements;
      real_T rtb_SumofElements_c;
      real_T rtb_SumofElements_f;
      real_T rtb_SumofElements_l3;
      real_T rtb_Switch;
      real_T rtb_VectorConcatenate_c_tmp;
      ZCEventType zcEvent;

      /* Outputs for IfAction SubSystem: '<S1>/EVDriveLine' incorporates:
       *  ActionPort: '<S2>/Action Port'
       */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S45>/Memory' */
        TX_DriveLine_B.Memory = TX_DriveLine_DW.Memory_PreviousInput;
        TX_DriveLine_B.domega_o = TX_DriveLine_P.TorsionalCompliance2_domega_o_k;
      }

      /* Integrator: '<S45>/Integrator' incorporates:
       *  Constant: '<S44>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent = rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE, (TX_DriveLine_B.Memory));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK != 0)) {
          TX_DriveLine_X.Integrator_CSTATE = TX_DriveLine_B.domega_o;
        }
      }

      /* Sum: '<S44>/Subtract1' incorporates:
       *  Gain: '<S44>/Gain1'
       *  Gain: '<S44>/Gain2'
       *  Integrator: '<S44>/Integrator'
       *  Integrator: '<S45>/Integrator'
       */
      rtb_Integrator_k3 = TX_DriveLine_P.TorsionalCompliance2_b_k * TX_DriveLine_X.Integrator_CSTATE +
                          TX_DriveLine_P.TorsionalCompliance2_k_g * TX_DriveLine_X.Integrator_CSTATE_g;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S37>/Memory' */
        TX_DriveLine_B.Memory_n = TX_DriveLine_DW.Memory_PreviousInput_c;
        TX_DriveLine_B.domega_o_h = TX_DriveLine_P.TorsionalCompliance1_domega_o_k;
      }

      /* Integrator: '<S37>/Integrator' incorporates:
       *  Constant: '<S36>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_b, (TX_DriveLine_B.Memory_n));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_h != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_j = TX_DriveLine_B.domega_o_h;
        }
      }

      /* Sum: '<S36>/Subtract1' incorporates:
       *  Gain: '<S36>/Gain1'
       *  Gain: '<S36>/Gain2'
       *  Integrator: '<S36>/Integrator'
       *  Integrator: '<S37>/Integrator'
       */
      rtb_SumofElements = TX_DriveLine_P.TorsionalCompliance1_b_d * TX_DriveLine_X.Integrator_CSTATE_j +
                          TX_DriveLine_P.TorsionalCompliance1_k_f * TX_DriveLine_X.Integrator_CSTATE_l;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S82>/Memory' */
        TX_DriveLine_B.Memory_p = TX_DriveLine_DW.Memory_PreviousInput_cw;
        TX_DriveLine_B.domega_o_o = TX_DriveLine_P.TorsionalCompliance_domega_o_i;
      }

      /* Integrator: '<S82>/Integrator' incorporates:
       *  Constant: '<S81>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_i, (TX_DriveLine_B.Memory_p));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_k != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_p = TX_DriveLine_B.domega_o_o;
        }
      }

      /* Sum: '<S81>/Subtract1' incorporates:
       *  Gain: '<S81>/Gain1'
       *  Gain: '<S81>/Gain2'
       *  Integrator: '<S81>/Integrator'
       *  Integrator: '<S82>/Integrator'
       */
      rtb_SumofElements_l3 = TX_DriveLine_P.TorsionalCompliance_b_m * TX_DriveLine_X.Integrator_CSTATE_p +
                             TX_DriveLine_P.TorsionalCompliance_k_g * TX_DriveLine_X.Integrator_CSTATE_e;
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        /* Memory: '<S90>/Memory' */
        TX_DriveLine_B.Memory_nc = TX_DriveLine_DW.Memory_PreviousInput_ch;
        TX_DriveLine_B.domega_o_f = TX_DriveLine_P.TorsionalCompliance1_domega_o_p;
      }

      /* Integrator: '<S90>/Integrator' incorporates:
       *  Constant: '<S89>/domega_o'
       */
      if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_M)->solverInfo)) {
        zcEvent =
            rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_h, (TX_DriveLine_B.Memory_nc));

        /* evaluate zero-crossings */
        if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_DW.Integrator_IWORK_j != 0)) {
          TX_DriveLine_X.Integrator_CSTATE_i = TX_DriveLine_B.domega_o_f;
        }
      }

      /* Sum: '<S89>/Subtract1' incorporates:
       *  Gain: '<S89>/Gain1'
       *  Gain: '<S89>/Gain2'
       *  Integrator: '<S89>/Integrator'
       *  Integrator: '<S90>/Integrator'
       */
      rtb_Subtract1_b0 = TX_DriveLine_P.TorsionalCompliance1_b_n * TX_DriveLine_X.Integrator_CSTATE_i +
                         TX_DriveLine_P.TorsionalCompliance1_k_p * TX_DriveLine_X.Integrator_CSTATE_im;

      /* MultiPortSwitch generated from: '<S4>/Multiport Switch' incorporates:
       *  Constant: '<S1>/DriveType[1-FD;2-RD;3-4WD]'
       */
      switch (static_cast<int32_T>(TX_DriveLine_P.drive_type)) {
        case 1:
          /* BusCreator: '<S2>/Bus Creator2' incorporates:
           *  SignalConversion generated from: '<S5>/Vector Concatenate'
           * */
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[0] = rtb_Integrator_k3;
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[1] = rtb_SumofElements;
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[2] = 0.0;
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[3] = 0.0;
          break;

        case 2:
          /* BusCreator: '<S2>/Bus Creator2' incorporates:
           *  SignalConversion generated from: '<S6>/Vector Concatenate'
           * */
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[0] = 0.0;
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[1] = 0.0;
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[2] = rtb_SumofElements_l3;
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[3] = rtb_Subtract1_b0;
          break;

        default:
          /* BusCreator: '<S2>/Bus Creator2' incorporates:
           *  SignalConversion generated from: '<S5>/Vector Concatenate'
           *  SignalConversion generated from: '<S6>/Vector Concatenate'
           * */
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[0] = rtb_Integrator_k3;
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[1] = rtb_SumofElements;
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[2] = rtb_SumofElements_l3;
          TX_DriveLine_B.Merge.wheel_drive_trq_Nm[3] = rtb_Subtract1_b0;
          break;
      }

      /* Switch: '<S16>/Switch' incorporates:
       *  Constant: '<S16>/Constant'
       *  Constant: '<S16>/Constant1'
       *  UnaryMinus: '<S16>/Unary Minus'
       */
      if (TX_DriveLine_P.Constant_Value_c > TX_DriveLine_P.Switch_Threshold_ce) {
        rtb_SumofElements_f = TX_DriveLine_P.Constant1_Value_nb;
      } else {
        rtb_SumofElements_f = -TX_DriveLine_P.Constant1_Value_nb;
      }

      /* End of Switch: '<S16>/Switch' */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        TX_DriveLine_B.VectorConcatenate[0] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
        TX_DriveLine_B.VectorConcatenate[1] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
      }

      /* Integrator: '<S10>/Integrator' incorporates:
       *  Concatenate: '<S10>/Vector Concatenate'
       *  Constant: '<S10>/Constant'
       *  Constant: '<S10>/Constant1'
       */
      /* Limited  Integrator  */
      if (TX_DriveLine_DW.Integrator_IWORK_g != 0) {
        TX_DriveLine_X.Integrator_CSTATE_b[0] = TX_DriveLine_B.VectorConcatenate[0];
        TX_DriveLine_X.Integrator_CSTATE_b[1] = TX_DriveLine_B.VectorConcatenate[1];
      }

      /* Gain: '<S16>/Gain' */
      rtb_Gain_p = TX_DriveLine_P.ratio_diff_front / 2.0;

      /* Integrator: '<S10>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_b[0] >= TX_DriveLine_P.Integrator_UpperSat_d) {
        TX_DriveLine_X.Integrator_CSTATE_b[0] = TX_DriveLine_P.Integrator_UpperSat_d;
      } else if (TX_DriveLine_X.Integrator_CSTATE_b[0] <= TX_DriveLine_P.Integrator_LowerSat_o) {
        TX_DriveLine_X.Integrator_CSTATE_b[0] = TX_DriveLine_P.Integrator_LowerSat_o;
      }

      /* Integrator: '<S10>/Integrator' */
      rtb_Integrator[0] = TX_DriveLine_X.Integrator_CSTATE_b[0];

      /* Integrator: '<S10>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_b[1] >= TX_DriveLine_P.Integrator_UpperSat_d) {
        TX_DriveLine_X.Integrator_CSTATE_b[1] = TX_DriveLine_P.Integrator_UpperSat_d;
      } else if (TX_DriveLine_X.Integrator_CSTATE_b[1] <= TX_DriveLine_P.Integrator_LowerSat_o) {
        TX_DriveLine_X.Integrator_CSTATE_b[1] = TX_DriveLine_P.Integrator_LowerSat_o;
      }

      /* Integrator: '<S10>/Integrator' */
      rtb_Integrator[1] = TX_DriveLine_X.Integrator_CSTATE_b[1];

      /* Sum: '<S16>/Sum of Elements' incorporates:
       *  Gain: '<S16>/Gain'
       *  Product: '<S16>/Product'
       */
      rtb_SumofElements_f =
          rtb_SumofElements_f * rtb_Integrator[0] * rtb_Gain_p + rtb_SumofElements_f * rtb_Integrator[1] * rtb_Gain_p;

      /* MultiPortSwitch generated from: '<S4>/Multiport Switch' incorporates:
       *  Constant: '<S1>/DriveType[1-FD;2-RD;3-4WD]'
       */
      switch (static_cast<int32_T>(TX_DriveLine_P.drive_type)) {
        case 1:
          /* BusCreator: '<S2>/Bus Creator2' incorporates:
           *  Gain: '<S5>/Gain1'
           */
          TX_DriveLine_B.Merge.front_mot_spd_rad_s = TX_DriveLine_P.front_motor_gear_ratio * rtb_SumofElements_f;
          break;

        case 2:
          /* BusCreator: '<S2>/Bus Creator2' incorporates:
           *  Constant: '<S6>/Constant'
           */
          TX_DriveLine_B.Merge.front_mot_spd_rad_s = TX_DriveLine_P.Constant_Value_jz;
          break;

        default:
          /* BusCreator: '<S2>/Bus Creator2' incorporates:
           *  Gain: '<S5>/Gain1'
           */
          TX_DriveLine_B.Merge.front_mot_spd_rad_s = TX_DriveLine_P.front_motor_gear_ratio * rtb_SumofElements_f;
          break;
      }

      /* Switch: '<S61>/Switch' incorporates:
       *  Constant: '<S61>/Constant'
       *  Constant: '<S61>/Constant1'
       *  UnaryMinus: '<S61>/Unary Minus'
       */
      if (TX_DriveLine_P.Constant_Value_ll > TX_DriveLine_P.Switch_Threshold_nf) {
        rtb_Product2_d = TX_DriveLine_P.Constant1_Value_e4b;
      } else {
        rtb_Product2_d = -TX_DriveLine_P.Constant1_Value_e4b;
      }

      /* End of Switch: '<S61>/Switch' */
      if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
        TX_DriveLine_B.VectorConcatenate_i[0] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
        TX_DriveLine_B.VectorConcatenate_i[1] = -TX_DriveLine_P.VEH.InitialLongVel / TX_DriveLine_P.StatLdWhlR[0];
      }

      /* Integrator: '<S55>/Integrator' incorporates:
       *  Concatenate: '<S55>/Vector Concatenate'
       *  Constant: '<S55>/Constant'
       *  Constant: '<S55>/Constant1'
       */
      /* Limited  Integrator  */
      if (TX_DriveLine_DW.Integrator_IWORK_d != 0) {
        TX_DriveLine_X.Integrator_CSTATE_n[0] = TX_DriveLine_B.VectorConcatenate_i[0];
        TX_DriveLine_X.Integrator_CSTATE_n[1] = TX_DriveLine_B.VectorConcatenate_i[1];
      }

      /* Gain: '<S61>/Gain' */
      rtb_Gain_p = TX_DriveLine_P.ratio_diff_rear / 2.0;

      /* Integrator: '<S55>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_n[0] >= TX_DriveLine_P.Integrator_UpperSat_do) {
        TX_DriveLine_X.Integrator_CSTATE_n[0] = TX_DriveLine_P.Integrator_UpperSat_do;
      } else if (TX_DriveLine_X.Integrator_CSTATE_n[0] <= TX_DriveLine_P.Integrator_LowerSat_a) {
        TX_DriveLine_X.Integrator_CSTATE_n[0] = TX_DriveLine_P.Integrator_LowerSat_a;
      }

      /* Integrator: '<S55>/Integrator' */
      rtb_Integrator_c[0] = TX_DriveLine_X.Integrator_CSTATE_n[0];

      /* Integrator: '<S55>/Integrator' */
      if (TX_DriveLine_X.Integrator_CSTATE_n[1] >= TX_DriveLine_P.Integrator_UpperSat_do) {
        TX_DriveLine_X.Integrator_CSTATE_n[1] = TX_DriveLine_P.Integrator_UpperSat_do;
      } else if (TX_DriveLine_X.Integrator_CSTATE_n[1] <= TX_DriveLine_P.Integrator_LowerSat_a) {
        TX_DriveLine_X.Integrator_CSTATE_n[1] = TX_DriveLine_P.Integrator_LowerSat_a;
      }

      /* Integrator: '<S55>/Integrator' */
      rtb_Integrator_c[1] = TX_DriveLine_X.Integrator_CSTATE_n[1];

      /* Sum: '<S61>/Sum of Elements' incorporates:
       *  Gain: '<S61>/Gain'
       *  Product: '<S61>/Product'
       */
      rtb_Product2_d =
          rtb_Product2_d * rtb_Integrator_c[0] * rtb_Gain_p + rtb_Product2_d * rtb_Integrator_c[1] * rtb_Gain_p;

      /* MultiPortSwitch generated from: '<S4>/Multiport Switch' incorporates:
       *  Constant: '<S1>/DriveType[1-FD;2-RD;3-4WD]'
       */
      switch (static_cast<int32_T>(TX_DriveLine_P.drive_type)) {
        case 1:
          /* BusCreator: '<S2>/Bus Creator2' incorporates:
           *  Constant: '<S5>/Constant'
           */
          TX_DriveLine_B.Merge.rear_mot_spd_rad_s = TX_DriveLine_P.Constant_Value_mw;
          break;

        case 2:
          /* BusCreator: '<S2>/Bus Creator2' incorporates:
           *  Gain: '<S6>/Gain1'
           */
          TX_DriveLine_B.Merge.rear_mot_spd_rad_s = TX_DriveLine_P.rear_motor_gear_ratio * rtb_Product2_d;
          break;

        default:
          /* BusCreator: '<S2>/Bus Creator2' incorporates:
           *  Gain: '<S6>/Gain1'
           */
          TX_DriveLine_B.Merge.rear_mot_spd_rad_s = TX_DriveLine_P.rear_motor_gear_ratio * rtb_Product2_d;
          break;
      }

      /* BusCreator: '<S2>/Bus Creator2' incorporates:
       *  Constant: '<S2>/Constant'
       *  Constant: '<S2>/Constant1'
       *  Constant: '<S2>/Constant2'
       */
      TX_DriveLine_B.Merge.gear_engaged = TX_DriveLine_P.Constant_Value_mx;
      TX_DriveLine_B.Merge.eng_spd_rad_s = TX_DriveLine_P.Constant1_Value_pn;
      TX_DriveLine_B.Merge.trans_out_shaft_spd_rad_s = TX_DriveLine_P.Constant2_Value_g;

      /* Gain: '<S5>/Gain' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       */
      rtb_Abs_e = TX_DriveLine_P.front_motor_gear_ratio * TX_DriveLine_U.DriveLineIn.mot_front_trq_Nm;

      /* Sum: '<S16>/Add' incorporates:
       *  Gain: '<S16>/Gain1'
       */
      rtb_Switch = TX_DriveLine_P.Gain1_Gain_p * rtb_Integrator[0] - TX_DriveLine_P.Gain1_Gain_p * rtb_Integrator[1];

      /* Gain: '<S34>/Gain' incorporates:
       *  Abs: '<S33>/Abs'
       *  Constant: '<S34>/Constant1'
       *  Gain: '<S34>/Gain1'
       *  Gain: '<S34>/Gain2'
       *  SignalConversion generated from: '<S16>/Vector Concatenate'
       *  Sum: '<S34>/Subtract1'
       *  Sum: '<S34>/Subtract2'
       *  Trigonometry: '<S34>/Trigonometric Function'
       */
      rtb_SumofElements_c = (std::tanh((TX_DriveLine_P.Gain1_Gain_f5 * std::abs(rtb_SumofElements_f) -
                                        TX_DriveLine_P.Constant1_Value_bl) *
                                       TX_DriveLine_P.Gain2_Gain_fo) +
                             TX_DriveLine_P.Constant1_Value_bl) *
                            TX_DriveLine_P.Gain_Gain_hb;

      /* Switch: '<S27>/Switch' incorporates:
       *  Constant: '<S27>/Constant'
       *  Constant: '<S35>/Constant'
       *  Product: '<S27>/Product1'
       *  SignalConversion generated from: '<S16>/Vector Concatenate'
       */
      if (rtb_Abs_e * rtb_SumofElements_f > TX_DriveLine_P.Switch_Threshold_m) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.EVFrontLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_ju;
      }

      /* Product: '<S27>/Product4' incorporates:
       *  Constant: '<S33>/Constant'
       *  Constant: '<S34>/Constant1'
       *  Product: '<S33>/Product3'
       *  Product: '<S33>/Product5'
       *  Sum: '<S33>/Subtract1'
       *  Sum: '<S34>/Subtract'
       *  Switch: '<S27>/Switch'
       */
      rtb_Product4 = ((TX_DriveLine_P.Constant1_Value_bl - rtb_SumofElements_c) * TX_DriveLine_P.Constant_Value_n3 +
                      rtb_VectorConcatenate_c_tmp * rtb_SumofElements_c) *
                     rtb_Abs_e;

      /* Gain: '<S30>/Gain' incorporates:
       *  Abs: '<S29>/Abs'
       *  Constant: '<S30>/Constant1'
       *  Gain: '<S30>/Gain1'
       *  Gain: '<S30>/Gain2'
       *  Sum: '<S30>/Subtract1'
       *  Sum: '<S30>/Subtract2'
       *  Trigonometry: '<S30>/Trigonometric Function'
       *  UnaryMinus: '<S16>/Unary Minus1'
       */
      rtb_Abs_e =
          (std::tanh((TX_DriveLine_P.Gain1_Gain_l * std::abs(-rtb_Integrator[0]) - TX_DriveLine_P.Constant1_Value_pb) *
                     TX_DriveLine_P.Gain2_Gain_mq) +
           TX_DriveLine_P.Constant1_Value_pb) *
          TX_DriveLine_P.Gain_Gain_dr;

      /* Switch: '<S25>/Switch' incorporates:
       *  Constant: '<S25>/Constant'
       *  Constant: '<S35>/Constant'
       *  Product: '<S25>/Product1'
       *  UnaryMinus: '<S16>/Unary Minus1'
       *  UnaryMinus: '<S44>/Unary Minus'
       */
      if (-rtb_Integrator_k3 * -rtb_Integrator[0] > TX_DriveLine_P.Switch_Threshold_cl) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.EVFrontLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_j1;
      }

      /* Product: '<S25>/Product4' incorporates:
       *  Constant: '<S29>/Constant'
       *  Constant: '<S30>/Constant1'
       *  Product: '<S29>/Product3'
       *  Product: '<S29>/Product5'
       *  Sum: '<S29>/Subtract1'
       *  Sum: '<S30>/Subtract'
       *  Switch: '<S25>/Switch'
       *  UnaryMinus: '<S44>/Unary Minus'
       */
      rtb_Product4_n = ((TX_DriveLine_P.Constant1_Value_pb - rtb_Abs_e) * TX_DriveLine_P.Constant_Value_er +
                        rtb_VectorConcatenate_c_tmp * rtb_Abs_e) *
                       -rtb_Integrator_k3;

      /* Gain: '<S32>/Gain' incorporates:
       *  Abs: '<S31>/Abs'
       *  Constant: '<S32>/Constant1'
       *  Gain: '<S32>/Gain1'
       *  Gain: '<S32>/Gain2'
       *  Sum: '<S32>/Subtract1'
       *  Sum: '<S32>/Subtract2'
       *  Trigonometry: '<S32>/Trigonometric Function'
       *  UnaryMinus: '<S16>/Unary Minus1'
       */
      rtb_Abs_e =
          (std::tanh((TX_DriveLine_P.Gain1_Gain_cf * std::abs(-rtb_Integrator[1]) - TX_DriveLine_P.Constant1_Value_oj) *
                     TX_DriveLine_P.Gain2_Gain_dk) +
           TX_DriveLine_P.Constant1_Value_oj) *
          TX_DriveLine_P.Gain_Gain_ag;

      /* Switch: '<S26>/Switch' incorporates:
       *  Constant: '<S26>/Constant'
       *  Constant: '<S35>/Constant'
       *  Product: '<S26>/Product1'
       *  UnaryMinus: '<S16>/Unary Minus1'
       *  UnaryMinus: '<S36>/Unary Minus'
       */
      if (-rtb_SumofElements * -rtb_Integrator[1] > TX_DriveLine_P.Switch_Threshold_n1) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.EVFrontLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_db;
      }

      /* Product: '<S26>/Product4' incorporates:
       *  Constant: '<S31>/Constant'
       *  Constant: '<S32>/Constant1'
       *  Product: '<S31>/Product3'
       *  Product: '<S31>/Product5'
       *  Sum: '<S31>/Subtract1'
       *  Sum: '<S32>/Subtract'
       *  Switch: '<S26>/Switch'
       *  UnaryMinus: '<S36>/Unary Minus'
       */
      rtb_Product4_p = ((TX_DriveLine_P.Constant1_Value_oj - rtb_Abs_e) * TX_DriveLine_P.Constant_Value_ni +
                        rtb_VectorConcatenate_c_tmp * rtb_Abs_e) *
                       -rtb_SumofElements;

      /* Integrator: '<S12>/Integrator' */
      rtb_Integrator_c4 = TX_DriveLine_X.Integrator_CSTATE_m;

      /* MATLAB Function: '<S10>/Limited Slip Differential' incorporates:
       *  Constant: '<S10>/Jd'
       *  Constant: '<S10>/Jw1'
       *  Constant: '<S10>/Jw3'
       *  Constant: '<S10>/Ndiff2'
       *  Constant: '<S10>/bd'
       *  Constant: '<S10>/bw1'
       *  Constant: '<S10>/bw2'
       */
      TX_DriveLine_LimitedSlipDifferential(
          rtb_Product4, rtb_Product4_n, rtb_Product4_p, rtb_Integrator_c4,
          TX_DriveLine_P.EVFrontLimitedSlipDifferential_bw1, TX_DriveLine_P.EVFrontLimitedSlipDifferential_bd,
          TX_DriveLine_P.EVFrontLimitedSlipDifferential_bw2, TX_DriveLine_P.ratio_diff_front,
          TX_DriveLine_P.EVFrontLimitedSlipDifferential_Jd, TX_DriveLine_P.EVFrontLimitedSlipDifferential_Jw1,
          TX_DriveLine_P.EVFrontLimitedSlipDifferential_Jw2, rtb_Integrator,
          &TX_DriveLine_B.sf_LimitedSlipDifferential_l, &TX_DriveLine_P.sf_LimitedSlipDifferential_l);

      /* Product: '<S12>/Product' incorporates:
       *  Abs: '<S24>/Abs'
       *  Constant: '<S10>/Constant3'
       *  Constant: '<S24>/Constant'
       *  Constant: '<S24>/Constant1'
       *  Constant: '<S24>/Constant2'
       *  Gain: '<S10>/2*pi'
       *  Gain: '<S24>/Gain'
       *  Lookup_n-D: '<S24>/mu Table'
       *  Product: '<S10>/Product1'
       *  Product: '<S24>/Product'
       *  Sum: '<S12>/Sum'
       *  Trigonometry: '<S24>/Trigonometric Function'
       */
      TX_DriveLine_B.Product =
          (TX_DriveLine_P.EVFrontLimitedSlipDifferential_Fc * TX_DriveLine_P.EVFrontLimitedSlipDifferential_Ndisks *
               TX_DriveLine_P.EVFrontLimitedSlipDifferential_Reff *
               std::tanh(TX_DriveLine_P.Gain_Gain_o2 * rtb_Switch) *
               look1_binlxpw(std::abs(rtb_Switch), TX_DriveLine_P.EVFrontLimitedSlipDifferential_dw,
                             TX_DriveLine_P.EVFrontLimitedSlipDifferential_muc, 7U) -
           rtb_Integrator_c4) *
          (1.0 / TX_DriveLine_P.EVFrontLimitedSlipDifferential_tauC * TX_DriveLine_P.upi_Gain_m);

      /* Sum: '<S36>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S16>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract = -rtb_Integrator[1] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[1];

      /* Switch: '<S37>/Switch' incorporates:
       *  Integrator: '<S37>/Integrator'
       */
      if (TX_DriveLine_B.Memory_n != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_j;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_h;
      }

      /* Product: '<S37>/Product' incorporates:
       *  Constant: '<S36>/omega_c'
       *  Sum: '<S37>/Sum'
       *  Switch: '<S37>/Switch'
       */
      TX_DriveLine_B.Product_i =
          (TX_DriveLine_B.Subtract - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance1_omega_c_g;

      /* Sum: '<S44>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S16>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_o = -rtb_Integrator[0] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[0];

      /* Switch: '<S45>/Switch' incorporates:
       *  Integrator: '<S45>/Integrator'
       */
      if (TX_DriveLine_B.Memory != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o;
      }

      /* Product: '<S45>/Product' incorporates:
       *  Constant: '<S44>/omega_c'
       *  Sum: '<S45>/Sum'
       *  Switch: '<S45>/Switch'
       */
      TX_DriveLine_B.Product_g =
          (TX_DriveLine_B.Subtract_o - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance2_omega_c_p;

      /* Gain: '<S6>/Gain' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       */
      rtb_SumofElements_c = TX_DriveLine_P.rear_motor_gear_ratio * TX_DriveLine_U.DriveLineIn.mot_rear_trq_Nm;

      /* Sum: '<S61>/Add' incorporates:
       *  Gain: '<S61>/Gain1'
       */
      rtb_Abs_e =
          TX_DriveLine_P.Gain1_Gain_jj * rtb_Integrator_c[0] - TX_DriveLine_P.Gain1_Gain_jj * rtb_Integrator_c[1];

      /* Gain: '<S79>/Gain' incorporates:
       *  Abs: '<S78>/Abs'
       *  Constant: '<S79>/Constant1'
       *  Gain: '<S79>/Gain1'
       *  Gain: '<S79>/Gain2'
       *  SignalConversion generated from: '<S61>/Vector Concatenate'
       *  Sum: '<S79>/Subtract1'
       *  Sum: '<S79>/Subtract2'
       *  Trigonometry: '<S79>/Trigonometric Function'
       */
      rtb_Switch =
          (std::tanh((TX_DriveLine_P.Gain1_Gain_ds * std::abs(rtb_Product2_d) - TX_DriveLine_P.Constant1_Value_k) *
                     TX_DriveLine_P.Gain2_Gain_ny) +
           TX_DriveLine_P.Constant1_Value_k) *
          TX_DriveLine_P.Gain_Gain_fq;

      /* Switch: '<S72>/Switch' incorporates:
       *  Constant: '<S72>/Constant'
       *  Constant: '<S80>/Constant'
       *  Product: '<S72>/Product1'
       *  SignalConversion generated from: '<S61>/Vector Concatenate'
       */
      if (rtb_SumofElements_c * rtb_Product2_d > TX_DriveLine_P.Switch_Threshold_mh) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.EVRearLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_na;
      }

      /* Product: '<S72>/Product4' incorporates:
       *  Constant: '<S78>/Constant'
       *  Constant: '<S79>/Constant1'
       *  Product: '<S78>/Product3'
       *  Product: '<S78>/Product5'
       *  Sum: '<S78>/Subtract1'
       *  Sum: '<S79>/Subtract'
       *  Switch: '<S72>/Switch'
       */
      rtb_Product4_h = ((TX_DriveLine_P.Constant1_Value_k - rtb_Switch) * TX_DriveLine_P.Constant_Value_l1 +
                        rtb_VectorConcatenate_c_tmp * rtb_Switch) *
                       rtb_SumofElements_c;

      /* Gain: '<S75>/Gain' incorporates:
       *  Abs: '<S74>/Abs'
       *  Constant: '<S75>/Constant1'
       *  Gain: '<S75>/Gain1'
       *  Gain: '<S75>/Gain2'
       *  Sum: '<S75>/Subtract1'
       *  Sum: '<S75>/Subtract2'
       *  Trigonometry: '<S75>/Trigonometric Function'
       *  UnaryMinus: '<S61>/Unary Minus1'
       */
      rtb_Product2_d = (std::tanh((TX_DriveLine_P.Gain1_Gain_ph * std::abs(-rtb_Integrator_c[0]) -
                                   TX_DriveLine_P.Constant1_Value_nv) *
                                  TX_DriveLine_P.Gain2_Gain_en) +
                        TX_DriveLine_P.Constant1_Value_nv) *
                       TX_DriveLine_P.Gain_Gain_oe;

      /* Switch: '<S70>/Switch' incorporates:
       *  Constant: '<S70>/Constant'
       *  Constant: '<S80>/Constant'
       *  Product: '<S70>/Product1'
       *  UnaryMinus: '<S61>/Unary Minus1'
       *  UnaryMinus: '<S81>/Unary Minus'
       */
      if (-rtb_SumofElements_l3 * -rtb_Integrator_c[0] > TX_DriveLine_P.Switch_Threshold_ly) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.EVRearLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_h2;
      }

      /* Product: '<S70>/Product4' incorporates:
       *  Constant: '<S74>/Constant'
       *  Constant: '<S75>/Constant1'
       *  Product: '<S74>/Product3'
       *  Product: '<S74>/Product5'
       *  Sum: '<S74>/Subtract1'
       *  Sum: '<S75>/Subtract'
       *  Switch: '<S70>/Switch'
       *  UnaryMinus: '<S81>/Unary Minus'
       */
      rtb_Product4_o = ((TX_DriveLine_P.Constant1_Value_nv - rtb_Product2_d) * TX_DriveLine_P.Constant_Value_nk +
                        rtb_VectorConcatenate_c_tmp * rtb_Product2_d) *
                       -rtb_SumofElements_l3;

      /* Gain: '<S77>/Gain' incorporates:
       *  Abs: '<S76>/Abs'
       *  Constant: '<S77>/Constant1'
       *  Gain: '<S77>/Gain1'
       *  Gain: '<S77>/Gain2'
       *  Sum: '<S77>/Subtract1'
       *  Sum: '<S77>/Subtract2'
       *  Trigonometry: '<S77>/Trigonometric Function'
       *  UnaryMinus: '<S61>/Unary Minus1'
       */
      rtb_Product2_d = (std::tanh((TX_DriveLine_P.Gain1_Gain_f0 * std::abs(-rtb_Integrator_c[1]) -
                                   TX_DriveLine_P.Constant1_Value_kc) *
                                  TX_DriveLine_P.Gain2_Gain_ce) +
                        TX_DriveLine_P.Constant1_Value_kc) *
                       TX_DriveLine_P.Gain_Gain_h2;

      /* Switch: '<S71>/Switch' incorporates:
       *  Constant: '<S71>/Constant'
       *  Constant: '<S80>/Constant'
       *  Product: '<S71>/Product1'
       *  UnaryMinus: '<S61>/Unary Minus1'
       *  UnaryMinus: '<S89>/Unary Minus'
       */
      if (-rtb_Subtract1_b0 * -rtb_Integrator_c[1] > TX_DriveLine_P.Switch_Threshold_gq) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.EVRearLimitedSlipDifferential_eta;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_P.Constant_Value_fu;
      }

      /* Product: '<S71>/Product4' incorporates:
       *  Constant: '<S76>/Constant'
       *  Constant: '<S77>/Constant1'
       *  Product: '<S76>/Product3'
       *  Product: '<S76>/Product5'
       *  Sum: '<S76>/Subtract1'
       *  Sum: '<S77>/Subtract'
       *  Switch: '<S71>/Switch'
       *  UnaryMinus: '<S89>/Unary Minus'
       */
      rtb_Product4_l = ((TX_DriveLine_P.Constant1_Value_kc - rtb_Product2_d) * TX_DriveLine_P.Constant_Value_om +
                        rtb_VectorConcatenate_c_tmp * rtb_Product2_d) *
                       -rtb_Subtract1_b0;

      /* Integrator: '<S57>/Integrator' */
      rtb_Integrator_o = TX_DriveLine_X.Integrator_CSTATE_ne;

      /* MATLAB Function: '<S55>/Limited Slip Differential' incorporates:
       *  Constant: '<S55>/Jd'
       *  Constant: '<S55>/Jw1'
       *  Constant: '<S55>/Jw3'
       *  Constant: '<S55>/Ndiff2'
       *  Constant: '<S55>/bd'
       *  Constant: '<S55>/bw1'
       *  Constant: '<S55>/bw2'
       */
      TX_DriveLine_LimitedSlipDifferential(
          rtb_Product4_h, rtb_Product4_o, rtb_Product4_l, rtb_Integrator_o,
          TX_DriveLine_P.EVRearLimitedSlipDifferential_bw1, TX_DriveLine_P.EVRearLimitedSlipDifferential_bd,
          TX_DriveLine_P.EVRearLimitedSlipDifferential_bw2, TX_DriveLine_P.ratio_diff_rear,
          TX_DriveLine_P.EVRearLimitedSlipDifferential_Jd, TX_DriveLine_P.EVRearLimitedSlipDifferential_Jw1,
          TX_DriveLine_P.EVRearLimitedSlipDifferential_Jw2, rtb_Integrator_c,
          &TX_DriveLine_B.sf_LimitedSlipDifferential_h, &TX_DriveLine_P.sf_LimitedSlipDifferential_h);

      /* Product: '<S57>/Product' incorporates:
       *  Abs: '<S69>/Abs'
       *  Constant: '<S55>/Constant3'
       *  Constant: '<S69>/Constant'
       *  Constant: '<S69>/Constant1'
       *  Constant: '<S69>/Constant2'
       *  Gain: '<S55>/2*pi'
       *  Gain: '<S69>/Gain'
       *  Lookup_n-D: '<S69>/mu Table'
       *  Product: '<S55>/Product1'
       *  Product: '<S69>/Product'
       *  Sum: '<S57>/Sum'
       *  Trigonometry: '<S69>/Trigonometric Function'
       */
      TX_DriveLine_B.Product_o =
          (TX_DriveLine_P.EVRearLimitedSlipDifferential_Fc * TX_DriveLine_P.EVRearLimitedSlipDifferential_Ndisks *
               TX_DriveLine_P.EVRearLimitedSlipDifferential_Reff * std::tanh(TX_DriveLine_P.Gain_Gain_pd * rtb_Abs_e) *
               look1_binlxpw(std::abs(rtb_Abs_e), TX_DriveLine_P.EVRearLimitedSlipDifferential_dw,
                             TX_DriveLine_P.EVRearLimitedSlipDifferential_muc, 7U) -
           rtb_Integrator_o) *
          (1.0 / TX_DriveLine_P.EVRearLimitedSlipDifferential_tauC * TX_DriveLine_P.upi_Gain_o);

      /* Sum: '<S81>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S61>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_h = -rtb_Integrator_c[0] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[2];

      /* Switch: '<S82>/Switch' incorporates:
       *  Integrator: '<S82>/Integrator'
       */
      if (TX_DriveLine_B.Memory_p != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_p;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_o;
      }

      /* Product: '<S82>/Product' incorporates:
       *  Constant: '<S81>/omega_c'
       *  Sum: '<S82>/Sum'
       *  Switch: '<S82>/Switch'
       */
      TX_DriveLine_B.Product_h =
          (TX_DriveLine_B.Subtract_h - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance_omega_c_p;

      /* Sum: '<S89>/Subtract' incorporates:
       *  Inport: '<Root>/DriveLineIn'
       *  UnaryMinus: '<S61>/Unary Minus1'
       */
      TX_DriveLine_B.Subtract_e = -rtb_Integrator_c[1] - TX_DriveLine_U.DriveLineIn.wheel_spd_rad_s[3];

      /* Switch: '<S90>/Switch' incorporates:
       *  Integrator: '<S90>/Integrator'
       */
      if (TX_DriveLine_B.Memory_nc != 0.0) {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_X.Integrator_CSTATE_i;
      } else {
        rtb_VectorConcatenate_c_tmp = TX_DriveLine_B.domega_o_f;
      }

      /* Product: '<S90>/Product' incorporates:
       *  Constant: '<S89>/omega_c'
       *  Sum: '<S90>/Sum'
       *  Switch: '<S90>/Switch'
       */
      TX_DriveLine_B.Product_j =
          (TX_DriveLine_B.Subtract_e - rtb_VectorConcatenate_c_tmp) * TX_DriveLine_P.TorsionalCompliance1_omega_c_h;

      /* End of Outputs for SubSystem: '<S1>/EVDriveLine' */
    } break;

    case 2:
      break;
  }

  /* End of SwitchCase: '<S1>/Switch Case' */

  /* Outport: '<Root>/DrivelineOut' */
  TX_DriveLine_Y.DrivelineOut = TX_DriveLine_B.Merge;
  if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
    /* Update for SwitchCase: '<S1>/Switch Case' */
    switch (TX_DriveLine_DW.SwitchCase_ActiveSubsystem) {
      case 0:
        /* Update for IfAction SubSystem: '<S1>/ICEDriveLine' incorporates:
         *  ActionPort: '<S3>/Action Port'
         */
        if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
          /* Update for Memory: '<S333>/Memory' incorporates:
           *  Constant: '<S333>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_e = TX_DriveLine_P.Reset_Value_mz;

          /* Update for Memory: '<S370>/Memory' incorporates:
           *  Constant: '<S370>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_i = TX_DriveLine_P.Reset_Value_g;
        }

        /* Update for Integrator: '<S333>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_c = 0;

        /* Update for Integrator: '<S370>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_m = 0;

        /* Update for If: '<S331>/If' */
        switch (TX_DriveLine_DW.If_ActiveSubsystem) {
          case 0:
            /* Update for IfAction SubSystem: '<S331>/Locked' incorporates:
             *  ActionPort: '<S335>/Action'
             */
            /* Update for Integrator: '<S335>/x' */
            TX_DriveLine_DW.x_IWORK = 0;

            /* End of Update for SubSystem: '<S331>/Locked' */
            break;

          case 1:
            /* Update for IfAction SubSystem: '<S331>/Unlocked' incorporates:
             *  ActionPort: '<S337>/Action'
             */
            /* Update for Integrator: '<S337>/xe' */
            TX_DriveLine_DW.xe_IWORK = 0;

            /* Update for Integrator: '<S337>/xv' */
            TX_DriveLine_DW.xv_IWORK = 0;

            /* End of Update for SubSystem: '<S331>/Unlocked' */
            break;
        }

        /* End of Update for If: '<S331>/If' */
        if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
          /* Update for Memory: '<S143>/Memory' incorporates:
           *  Constant: '<S143>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_es = TX_DriveLine_P.Reset_Value_f;

          /* Update for Memory: '<S135>/Memory' incorporates:
           *  Constant: '<S135>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_f = TX_DriveLine_P.Reset_Value;
        }

        /* Update for Integrator: '<S143>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_gr = 0;

        /* Update for Integrator: '<S135>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_e = 0;
        if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
          /* Update for Memory: '<S311>/Memory' incorporates:
           *  Constant: '<S311>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_o = TX_DriveLine_P.Reset_Value_m;

          /* Update for Memory: '<S319>/Memory' incorporates:
           *  Constant: '<S319>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_h = TX_DriveLine_P.Reset_Value_fz;
        }

        /* Update for Integrator: '<S311>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_i = 0;

        /* Update for Integrator: '<S319>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_gy = 0;
        if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
          /* Update for Memory: '<S244>/Memory' incorporates:
           *  Constant: '<S244>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_n = TX_DriveLine_P.Reset_Value_i;

          /* Update for Memory: '<S252>/Memory' incorporates:
           *  Constant: '<S252>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_b = TX_DriveLine_P.Reset_Value_f3;
        }

        /* Update for Integrator: '<S244>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_ij = 0;

        /* Update for Integrator: '<S252>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_hd = 0;
        if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
          /* Update for Memory: '<S228>/Memory' incorporates:
           *  Constant: '<S228>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_fa = TX_DriveLine_P.Reset_Value_n;

          /* Update for Memory: '<S236>/Memory' incorporates:
           *  Constant: '<S236>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_l = TX_DriveLine_P.Reset_Value_c;
        }

        /* Update for Integrator: '<S228>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_hs = 0;

        /* Update for Integrator: '<S236>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_n = 0;

        /* Update for Integrator: '<S108>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_p = 0;

        /* Update for Integrator: '<S284>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_a = 0;

        /* Update for Integrator: '<S158>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_ao = 0;
        if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
          /* Update for Memory: '<S212>/Memory' incorporates:
           *  Constant: '<S212>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_bo = TX_DriveLine_P.Reset_Value_d;

          /* Update for Memory: '<S220>/Memory' incorporates:
           *  Constant: '<S220>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_fn = TX_DriveLine_P.Reset_Value_fu;
        }

        /* Update for Integrator: '<S212>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_ct = 0;

        /* Update for Integrator: '<S159>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_b = 0;

        /* Update for Integrator: '<S220>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_o = 0;

        /* Update for Integrator: '<S185>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_f = 0;
        if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
          /* Update for UnitDelay: '<S326>/Delay Input1' */
          TX_DriveLine_DW.DelayInput1_DSTATE = TX_DriveLine_B.DataTypeConversion;
        }

        /* End of Update for SubSystem: '<S1>/ICEDriveLine' */
        break;

      case 1:
        /* Update for IfAction SubSystem: '<S1>/EVDriveLine' incorporates:
         *  ActionPort: '<S2>/Action Port'
         */
        if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
          /* Update for Memory: '<S45>/Memory' incorporates:
           *  Constant: '<S45>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput = TX_DriveLine_P.Reset_Value_g5;

          /* Update for Memory: '<S37>/Memory' incorporates:
           *  Constant: '<S37>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_c = TX_DriveLine_P.Reset_Value_j;
        }

        /* Update for Integrator: '<S45>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK = 0;

        /* Update for Integrator: '<S37>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_h = 0;
        if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
          /* Update for Memory: '<S82>/Memory' incorporates:
           *  Constant: '<S82>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_cw = TX_DriveLine_P.Reset_Value_o;

          /* Update for Memory: '<S90>/Memory' incorporates:
           *  Constant: '<S90>/Reset'
           */
          TX_DriveLine_DW.Memory_PreviousInput_ch = TX_DriveLine_P.Reset_Value_h;
        }

        /* Update for Integrator: '<S82>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_k = 0;

        /* Update for Integrator: '<S90>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_j = 0;

        /* Update for Integrator: '<S10>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_g = 0;

        /* Update for Integrator: '<S55>/Integrator' */
        TX_DriveLine_DW.Integrator_IWORK_d = 0;

        /* End of Update for SubSystem: '<S1>/EVDriveLine' */
        break;

      case 2:
        break;
    }

    /* End of Update for SwitchCase: '<S1>/Switch Case' */

    /* ContTimeOutputInconsistentWithStateAtMajorOutputFlag is set, need to run a minor output */
    if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
      if (rtsiGetContTimeOutputInconsistentWithStateAtMajorStep(&(&TX_DriveLine_M)->solverInfo)) {
        rtsiSetSimTimeStep(&(&TX_DriveLine_M)->solverInfo, MINOR_TIME_STEP);
        rtsiSetContTimeOutputInconsistentWithStateAtMajorStep(&(&TX_DriveLine_M)->solverInfo, false);
        TX_DriveLine::step();
        rtsiSetSimTimeStep(&(&TX_DriveLine_M)->solverInfo, MAJOR_TIME_STEP);
      }
    }
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&TX_DriveLine_M))) {
    rt_ertODEUpdateContinuousStates(&(&TX_DriveLine_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&TX_DriveLine_M)->Timing.clockTick0)) {
      ++(&TX_DriveLine_M)->Timing.clockTickH0;
    }

    (&TX_DriveLine_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&TX_DriveLine_M)->solverInfo);

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
      (&TX_DriveLine_M)->Timing.clockTick1++;
      if (!(&TX_DriveLine_M)->Timing.clockTick1) {
        (&TX_DriveLine_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void TX_DriveLine::TX_DriveLine_derivatives() {
  XDot_TX_DriveLine_T *_rtXdot;
  _rtXdot = ((XDot_TX_DriveLine_T *)(&TX_DriveLine_M)->derivs);

  /* Derivatives for SwitchCase: '<S1>/Switch Case' */
  {
    real_T *dx;
    int_T i;
    dx = &(((XDot_TX_DriveLine_T *)(&TX_DriveLine_M)->derivs)->Integrator_CSTATE_f);
    for (i = 0; i < 41; i++) {
      dx[i] = 0.0;
    }
  }

  {
    real_T *dx;
    int_T i;
    dx = &(((XDot_TX_DriveLine_T *)(&TX_DriveLine_M)->derivs)->Integrator_CSTATE);
    for (i = 0; i < 14; i++) {
      dx[i] = 0.0;
    }
  }

  switch (TX_DriveLine_DW.SwitchCase_ActiveSubsystem) {
    case 0: {
      boolean_T lsat;
      boolean_T usat;

      /* Derivatives for IfAction SubSystem: '<S1>/ICEDriveLine' incorporates:
       *  ActionPort: '<S3>/Action Port'
       */
      /* Derivatives for Integrator: '<S333>/Integrator' */
      _rtXdot->Integrator_CSTATE_f = TX_DriveLine_B.Product_cs;

      /* Derivatives for Integrator: '<S370>/Integrator' */
      _rtXdot->Integrator_CSTATE_h = TX_DriveLine_B.Product_jf;

      /* Derivatives for Integrator: '<S369>/Integrator' */
      _rtXdot->Integrator_CSTATE_c = TX_DriveLine_B.Subtract_dt;

      /* Derivatives for If: '<S331>/If' */
      ((XDot_TX_DriveLine_T *)(&TX_DriveLine_M)->derivs)->w = 0.0;

      {
        real_T *dx;
        int_T i;
        dx = &(((XDot_TX_DriveLine_T *)(&TX_DriveLine_M)->derivs)->we);
        for (i = 0; i < 2; i++) {
          dx[i] = 0.0;
        }
      }

      switch (TX_DriveLine_DW.If_ActiveSubsystem) {
        case 0:
          /* Derivatives for IfAction SubSystem: '<S331>/Locked' incorporates:
           *  ActionPort: '<S335>/Action'
           */
          /* Derivatives for Integrator: '<S335>/x' */
          _rtXdot->w = TX_DriveLine_B.Product8_i;

          /* End of Derivatives for SubSystem: '<S331>/Locked' */
          break;

        case 1:
          /* Derivatives for IfAction SubSystem: '<S331>/Unlocked' incorporates:
           *  ActionPort: '<S337>/Action'
           */
          /* Derivatives for Integrator: '<S337>/xe' */
          _rtXdot->we = TX_DriveLine_B.Product4;

          /* Derivatives for Integrator: '<S337>/xv' */
          _rtXdot->wv = TX_DriveLine_B.Product8;

          /* End of Derivatives for SubSystem: '<S331>/Unlocked' */
          break;
      }

      /* End of Derivatives for If: '<S331>/If' */

      /* Derivatives for Integrator: '<S143>/Integrator' */
      _rtXdot->Integrator_CSTATE_i5 = TX_DriveLine_B.Product_b;

      /* Derivatives for Integrator: '<S142>/Integrator' */
      _rtXdot->Integrator_CSTATE_fk = TX_DriveLine_B.Subtract_k;

      /* Derivatives for Integrator: '<S135>/Integrator' */
      _rtXdot->Integrator_CSTATE_d = TX_DriveLine_B.Product_e;

      /* Derivatives for Integrator: '<S134>/Integrator' */
      _rtXdot->Integrator_CSTATE_nq = TX_DriveLine_B.Subtract_a;

      /* Derivatives for Integrator: '<S311>/Integrator' */
      _rtXdot->Integrator_CSTATE_ng = TX_DriveLine_B.Product_lx;

      /* Derivatives for Integrator: '<S310>/Integrator' */
      _rtXdot->Integrator_CSTATE_d1 = TX_DriveLine_B.Subtract_hz;

      /* Derivatives for Integrator: '<S319>/Integrator' */
      _rtXdot->Integrator_CSTATE_hk = TX_DriveLine_B.Product_af;

      /* Derivatives for Integrator: '<S318>/Integrator' */
      _rtXdot->Integrator_CSTATE_lg = TX_DriveLine_B.Subtract_p;

      /* Derivatives for Integrator: '<S244>/Integrator' */
      _rtXdot->Integrator_CSTATE_lz = TX_DriveLine_B.Product_pr;

      /* Derivatives for Integrator: '<S243>/Integrator' */
      _rtXdot->Integrator_CSTATE_iy = TX_DriveLine_B.Subtract_j;

      /* Derivatives for Integrator: '<S252>/Integrator' */
      _rtXdot->Integrator_CSTATE_bb = TX_DriveLine_B.Product_l;

      /* Derivatives for Integrator: '<S251>/Integrator' */
      _rtXdot->Integrator_CSTATE_b2 = TX_DriveLine_B.Subtract_d;

      /* Derivatives for Integrator: '<S228>/Integrator' */
      _rtXdot->Integrator_CSTATE_c1 = TX_DriveLine_B.Product_e3;

      /* Derivatives for Integrator: '<S227>/Integrator' */
      _rtXdot->Integrator_CSTATE_hkp = TX_DriveLine_B.Subtract_n;

      /* Derivatives for Integrator: '<S236>/Integrator' */
      _rtXdot->Integrator_CSTATE_bx = TX_DriveLine_B.Product_pv;

      /* Derivatives for Integrator: '<S235>/Integrator' */
      _rtXdot->Integrator_CSTATE_g1 = TX_DriveLine_B.Subtract_lk;

      /* Derivatives for Integrator: '<S108>/Integrator' */
      lsat = (TX_DriveLine_X.Integrator_CSTATE_h2[0] <= TX_DriveLine_P.Integrator_LowerSat);
      usat = (TX_DriveLine_X.Integrator_CSTATE_h2[0] >= TX_DriveLine_P.Integrator_UpperSat);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential.xdot[0] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential.xdot[0] < 0.0))) {
        _rtXdot->Integrator_CSTATE_h2[0] = TX_DriveLine_B.sf_LimitedSlipDifferential.xdot[0];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_h2[0] = 0.0;
      }

      lsat = (TX_DriveLine_X.Integrator_CSTATE_h2[1] <= TX_DriveLine_P.Integrator_LowerSat);
      usat = (TX_DriveLine_X.Integrator_CSTATE_h2[1] >= TX_DriveLine_P.Integrator_UpperSat);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential.xdot[1] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential.xdot[1] < 0.0))) {
        _rtXdot->Integrator_CSTATE_h2[1] = TX_DriveLine_B.sf_LimitedSlipDifferential.xdot[1];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_h2[1] = 0.0;
      }

      /* End of Derivatives for Integrator: '<S108>/Integrator' */

      /* Derivatives for Integrator: '<S284>/Integrator' */
      lsat = (TX_DriveLine_X.Integrator_CSTATE_c0[0] <= TX_DriveLine_P.Integrator_LowerSat_p);
      usat = (TX_DriveLine_X.Integrator_CSTATE_c0[0] >= TX_DriveLine_P.Integrator_UpperSat_f);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential_b.xdot[0] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential_b.xdot[0] < 0.0))) {
        _rtXdot->Integrator_CSTATE_c0[0] = TX_DriveLine_B.sf_LimitedSlipDifferential_b.xdot[0];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_c0[0] = 0.0;
      }

      lsat = (TX_DriveLine_X.Integrator_CSTATE_c0[1] <= TX_DriveLine_P.Integrator_LowerSat_p);
      usat = (TX_DriveLine_X.Integrator_CSTATE_c0[1] >= TX_DriveLine_P.Integrator_UpperSat_f);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential_b.xdot[1] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential_b.xdot[1] < 0.0))) {
        _rtXdot->Integrator_CSTATE_c0[1] = TX_DriveLine_B.sf_LimitedSlipDifferential_b.xdot[1];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_c0[1] = 0.0;
      }

      /* End of Derivatives for Integrator: '<S284>/Integrator' */

      /* Derivatives for Integrator: '<S158>/Integrator' */
      lsat = (TX_DriveLine_X.Integrator_CSTATE_g3[0] <= TX_DriveLine_P.Integrator_LowerSat_i);
      usat = (TX_DriveLine_X.Integrator_CSTATE_g3[0] >= TX_DriveLine_P.Integrator_UpperSat_e);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.xdot[0] > 0.0)) ||
          (usat && (TX_DriveLine_B.xdot[0] < 0.0))) {
        _rtXdot->Integrator_CSTATE_g3[0] = TX_DriveLine_B.xdot[0];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_g3[0] = 0.0;
      }

      lsat = (TX_DriveLine_X.Integrator_CSTATE_g3[1] <= TX_DriveLine_P.Integrator_LowerSat_i);
      usat = (TX_DriveLine_X.Integrator_CSTATE_g3[1] >= TX_DriveLine_P.Integrator_UpperSat_e);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.xdot[1] > 0.0)) ||
          (usat && (TX_DriveLine_B.xdot[1] < 0.0))) {
        _rtXdot->Integrator_CSTATE_g3[1] = TX_DriveLine_B.xdot[1];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_g3[1] = 0.0;
      }

      /* End of Derivatives for Integrator: '<S158>/Integrator' */

      /* Derivatives for Integrator: '<S110>/Integrator' */
      _rtXdot->Integrator_CSTATE_nv = TX_DriveLine_B.Product_p;

      /* Derivatives for Integrator: '<S212>/Integrator' */
      _rtXdot->Integrator_CSTATE_db = TX_DriveLine_B.Product_a;

      /* Derivatives for Integrator: '<S211>/Integrator' */
      _rtXdot->Integrator_CSTATE_dv = TX_DriveLine_B.Subtract_l;

      /* Derivatives for Integrator: '<S159>/Integrator' */
      lsat = (TX_DriveLine_X.Integrator_CSTATE_eu[0] <= TX_DriveLine_P.Integrator_LowerSat_n);
      usat = (TX_DriveLine_X.Integrator_CSTATE_eu[0] >= TX_DriveLine_P.Integrator_UpperSat_er);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential_i.xdot[0] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential_i.xdot[0] < 0.0))) {
        _rtXdot->Integrator_CSTATE_eu[0] = TX_DriveLine_B.sf_LimitedSlipDifferential_i.xdot[0];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_eu[0] = 0.0;
      }

      lsat = (TX_DriveLine_X.Integrator_CSTATE_eu[1] <= TX_DriveLine_P.Integrator_LowerSat_n);
      usat = (TX_DriveLine_X.Integrator_CSTATE_eu[1] >= TX_DriveLine_P.Integrator_UpperSat_er);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential_i.xdot[1] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential_i.xdot[1] < 0.0))) {
        _rtXdot->Integrator_CSTATE_eu[1] = TX_DriveLine_B.sf_LimitedSlipDifferential_i.xdot[1];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_eu[1] = 0.0;
      }

      /* End of Derivatives for Integrator: '<S159>/Integrator' */

      /* Derivatives for Integrator: '<S161>/Integrator' */
      _rtXdot->Integrator_CSTATE_ea = TX_DriveLine_B.Product_c;

      /* Derivatives for Integrator: '<S220>/Integrator' */
      _rtXdot->Integrator_CSTATE_ny = TX_DriveLine_B.Product_iv;

      /* Derivatives for Integrator: '<S219>/Integrator' */
      _rtXdot->Integrator_CSTATE_hz = TX_DriveLine_B.Subtract_f;

      /* Derivatives for Integrator: '<S185>/Integrator' */
      lsat = (TX_DriveLine_X.Integrator_CSTATE_j2[0] <= TX_DriveLine_P.Integrator_LowerSat_j);
      usat = (TX_DriveLine_X.Integrator_CSTATE_j2[0] >= TX_DriveLine_P.Integrator_UpperSat_a);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential_k.xdot[0] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential_k.xdot[0] < 0.0))) {
        _rtXdot->Integrator_CSTATE_j2[0] = TX_DriveLine_B.sf_LimitedSlipDifferential_k.xdot[0];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_j2[0] = 0.0;
      }

      lsat = (TX_DriveLine_X.Integrator_CSTATE_j2[1] <= TX_DriveLine_P.Integrator_LowerSat_j);
      usat = (TX_DriveLine_X.Integrator_CSTATE_j2[1] >= TX_DriveLine_P.Integrator_UpperSat_a);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential_k.xdot[1] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential_k.xdot[1] < 0.0))) {
        _rtXdot->Integrator_CSTATE_j2[1] = TX_DriveLine_B.sf_LimitedSlipDifferential_k.xdot[1];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_j2[1] = 0.0;
      }

      /* End of Derivatives for Integrator: '<S185>/Integrator' */

      /* Derivatives for Integrator: '<S187>/Integrator' */
      _rtXdot->Integrator_CSTATE_a = TX_DriveLine_B.Product_f;

      /* Derivatives for Integrator: '<S286>/Integrator' */
      _rtXdot->Integrator_CSTATE_a4 = TX_DriveLine_B.Product_h1;

      /* Derivatives for Integrator: '<S382>/Integrator1' */
      _rtXdot->Integrator1_CSTATE = TX_DriveLine_B.Divide;

      /* End of Derivatives for SubSystem: '<S1>/ICEDriveLine' */
    } break;

    case 1: {
      boolean_T lsat;
      boolean_T usat;

      /* Derivatives for IfAction SubSystem: '<S1>/EVDriveLine' incorporates:
       *  ActionPort: '<S2>/Action Port'
       */
      /* Derivatives for Integrator: '<S45>/Integrator' */
      _rtXdot->Integrator_CSTATE = TX_DriveLine_B.Product_g;

      /* Derivatives for Integrator: '<S44>/Integrator' */
      _rtXdot->Integrator_CSTATE_g = TX_DriveLine_B.Subtract_o;

      /* Derivatives for Integrator: '<S37>/Integrator' */
      _rtXdot->Integrator_CSTATE_j = TX_DriveLine_B.Product_i;

      /* Derivatives for Integrator: '<S36>/Integrator' */
      _rtXdot->Integrator_CSTATE_l = TX_DriveLine_B.Subtract;

      /* Derivatives for Integrator: '<S82>/Integrator' */
      _rtXdot->Integrator_CSTATE_p = TX_DriveLine_B.Product_h;

      /* Derivatives for Integrator: '<S81>/Integrator' */
      _rtXdot->Integrator_CSTATE_e = TX_DriveLine_B.Subtract_h;

      /* Derivatives for Integrator: '<S90>/Integrator' */
      _rtXdot->Integrator_CSTATE_i = TX_DriveLine_B.Product_j;

      /* Derivatives for Integrator: '<S89>/Integrator' */
      _rtXdot->Integrator_CSTATE_im = TX_DriveLine_B.Subtract_e;

      /* Derivatives for Integrator: '<S10>/Integrator' */
      lsat = (TX_DriveLine_X.Integrator_CSTATE_b[0] <= TX_DriveLine_P.Integrator_LowerSat_o);
      usat = (TX_DriveLine_X.Integrator_CSTATE_b[0] >= TX_DriveLine_P.Integrator_UpperSat_d);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential_l.xdot[0] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential_l.xdot[0] < 0.0))) {
        _rtXdot->Integrator_CSTATE_b[0] = TX_DriveLine_B.sf_LimitedSlipDifferential_l.xdot[0];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_b[0] = 0.0;
      }

      lsat = (TX_DriveLine_X.Integrator_CSTATE_b[1] <= TX_DriveLine_P.Integrator_LowerSat_o);
      usat = (TX_DriveLine_X.Integrator_CSTATE_b[1] >= TX_DriveLine_P.Integrator_UpperSat_d);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential_l.xdot[1] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential_l.xdot[1] < 0.0))) {
        _rtXdot->Integrator_CSTATE_b[1] = TX_DriveLine_B.sf_LimitedSlipDifferential_l.xdot[1];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_b[1] = 0.0;
      }

      /* End of Derivatives for Integrator: '<S10>/Integrator' */

      /* Derivatives for Integrator: '<S55>/Integrator' */
      lsat = (TX_DriveLine_X.Integrator_CSTATE_n[0] <= TX_DriveLine_P.Integrator_LowerSat_a);
      usat = (TX_DriveLine_X.Integrator_CSTATE_n[0] >= TX_DriveLine_P.Integrator_UpperSat_do);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential_h.xdot[0] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential_h.xdot[0] < 0.0))) {
        _rtXdot->Integrator_CSTATE_n[0] = TX_DriveLine_B.sf_LimitedSlipDifferential_h.xdot[0];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_n[0] = 0.0;
      }

      lsat = (TX_DriveLine_X.Integrator_CSTATE_n[1] <= TX_DriveLine_P.Integrator_LowerSat_a);
      usat = (TX_DriveLine_X.Integrator_CSTATE_n[1] >= TX_DriveLine_P.Integrator_UpperSat_do);
      if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_B.sf_LimitedSlipDifferential_h.xdot[1] > 0.0)) ||
          (usat && (TX_DriveLine_B.sf_LimitedSlipDifferential_h.xdot[1] < 0.0))) {
        _rtXdot->Integrator_CSTATE_n[1] = TX_DriveLine_B.sf_LimitedSlipDifferential_h.xdot[1];
      } else {
        /* in saturation */
        _rtXdot->Integrator_CSTATE_n[1] = 0.0;
      }

      /* End of Derivatives for Integrator: '<S55>/Integrator' */

      /* Derivatives for Integrator: '<S12>/Integrator' */
      _rtXdot->Integrator_CSTATE_m = TX_DriveLine_B.Product;

      /* Derivatives for Integrator: '<S57>/Integrator' */
      _rtXdot->Integrator_CSTATE_ne = TX_DriveLine_B.Product_o;

      /* End of Derivatives for SubSystem: '<S1>/EVDriveLine' */
    } break;

    case 2:
      break;
  }

  /* End of Derivatives for SwitchCase: '<S1>/Switch Case' */
}

/* Model initialize function */
void TX_DriveLine::initialize() {
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* non-finite (run-time) assignments */
  TX_DriveLine_P.Saturation1_LowerSat = rtMinusInf;
  TX_DriveLine_P.Saturation2_UpperSat = rtInf;
  TX_DriveLine_P.Saturation1_LowerSat_k = rtMinusInf;
  TX_DriveLine_P.Saturation2_UpperSat_p = rtInf;

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&TX_DriveLine_M)->solverInfo, &(&TX_DriveLine_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&TX_DriveLine_M)->solverInfo, &rtmGetTPtr((&TX_DriveLine_M)));
    rtsiSetStepSizePtr(&(&TX_DriveLine_M)->solverInfo, &(&TX_DriveLine_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&TX_DriveLine_M)->solverInfo, &(&TX_DriveLine_M)->derivs);
    rtsiSetContStatesPtr(&(&TX_DriveLine_M)->solverInfo, (real_T **)&(&TX_DriveLine_M)->contStates);
    rtsiSetNumContStatesPtr(&(&TX_DriveLine_M)->solverInfo, &(&TX_DriveLine_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&TX_DriveLine_M)->solverInfo, &(&TX_DriveLine_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&TX_DriveLine_M)->solverInfo, &(&TX_DriveLine_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&TX_DriveLine_M)->solverInfo, &(&TX_DriveLine_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&TX_DriveLine_M)->solverInfo, (&rtmGetErrorStatus((&TX_DriveLine_M))));
    rtsiSetRTModelPtr(&(&TX_DriveLine_M)->solverInfo, (&TX_DriveLine_M));
  }

  rtsiSetSimTimeStep(&(&TX_DriveLine_M)->solverInfo, MAJOR_TIME_STEP);
  (&TX_DriveLine_M)->intgData.y = (&TX_DriveLine_M)->odeY;
  (&TX_DriveLine_M)->intgData.f[0] = (&TX_DriveLine_M)->odeF[0];
  (&TX_DriveLine_M)->intgData.f[1] = (&TX_DriveLine_M)->odeF[1];
  (&TX_DriveLine_M)->intgData.f[2] = (&TX_DriveLine_M)->odeF[2];
  (&TX_DriveLine_M)->intgData.f[3] = (&TX_DriveLine_M)->odeF[3];
  (&TX_DriveLine_M)->contStates = ((X_TX_DriveLine_T *)&TX_DriveLine_X);
  rtsiSetSolverData(&(&TX_DriveLine_M)->solverInfo, static_cast<void *>(&(&TX_DriveLine_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&TX_DriveLine_M)->solverInfo, false);
  rtsiSetSolverName(&(&TX_DriveLine_M)->solverInfo, "ode4");
  rtmSetTPtr((&TX_DriveLine_M), &(&TX_DriveLine_M)->Timing.tArray[0]);
  (&TX_DriveLine_M)->Timing.stepSize0 = 0.001;
  rtmSetFirstInitCond((&TX_DriveLine_M), 1);

  /* Start for SwitchCase: '<S1>/Switch Case' */
  TX_DriveLine_DW.SwitchCase_ActiveSubsystem = -1;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_m = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_e = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_f = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_k = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_hb = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_d = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_p = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_n = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_pu = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_hbb = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_nr = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_j = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_b = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_i = UNINITIALIZED_ZCSIG;
  TX_DriveLine_PrevZCX.Integrator_Reset_ZCE_h = UNINITIALIZED_ZCSIG;

  /* SystemInitialize for IfAction SubSystem: '<S1>/ICEDriveLine' */
  /* Start for Constant: '<S331>/Constant1' */
  TX_DriveLine_B.Constant1 = TX_DriveLine_P.IdealFixedGearTransmission_G_o;

  /* Start for Constant: '<S369>/domega_o' */
  TX_DriveLine_B.domega_o_g = TX_DriveLine_P.PropShaft_domega_o;

  /* Start for InitialCondition: '<S338>/IC' */
  TX_DriveLine_DW.IC_FirstOutputTime = true;

  /* Start for InitialCondition: '<S336>/IC' */
  TX_DriveLine_DW.IC_FirstOutputTime_e = true;

  /* Start for InitialCondition: '<S334>/IC' */
  TX_DriveLine_DW.IC_FirstOutputTime_f = true;

  /* Start for If: '<S331>/If' */
  TX_DriveLine_DW.If_ActiveSubsystem = -1;

  /* Start for Constant: '<S142>/domega_o' */
  TX_DriveLine_B.domega_o_a = TX_DriveLine_P.TorsionalCompliance2_domega_o;

  /* Start for Constant: '<S134>/domega_o' */
  TX_DriveLine_B.domega_o_i = TX_DriveLine_P.TorsionalCompliance1_domega_o;

  /* Start for Constant: '<S310>/domega_o' */
  TX_DriveLine_B.domega_o_k = TX_DriveLine_P.TorsionalCompliance_domega_o;

  /* Start for Constant: '<S318>/domega_o' */
  TX_DriveLine_B.domega_o_kv = TX_DriveLine_P.TorsionalCompliance1_domega_o_c;

  /* Start for Constant: '<S243>/domega_o' */
  TX_DriveLine_B.domega_o_a2 = TX_DriveLine_P.TorsionalCompliance4_domega_o;

  /* Start for Constant: '<S251>/domega_o' */
  TX_DriveLine_B.domega_o_hw = TX_DriveLine_P.TorsionalCompliance5_domega_o;

  /* Start for Constant: '<S227>/domega_o' */
  TX_DriveLine_B.domega_o_b = TX_DriveLine_P.TorsionalCompliance2_domega_o_f;

  /* Start for Constant: '<S235>/domega_o' */
  TX_DriveLine_B.domega_o_c = TX_DriveLine_P.TorsionalCompliance3_domega_o;

  /* Start for Constant: '<S211>/domega_o' */
  TX_DriveLine_B.domega_o_az = TX_DriveLine_P.TorsionalCompliance_domega_o_d;

  /* Start for Constant: '<S219>/domega_o' */
  TX_DriveLine_B.domega_o_n = TX_DriveLine_P.TorsionalCompliance1_domega_o_h;

  /* InitializeConditions for Memory: '<S333>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_e = TX_DriveLine_P.Memory_InitialCondition;

  /* InitializeConditions for Integrator: '<S333>/Integrator' incorporates:
   *  Integrator: '<S370>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE_f = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_h = 0.0;
  }

  TX_DriveLine_DW.Integrator_IWORK_c = 1;

  /* End of InitializeConditions for Integrator: '<S333>/Integrator' */

  /* InitializeConditions for Memory: '<S370>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_i = TX_DriveLine_P.Memory_InitialCondition_j;

  /* InitializeConditions for Integrator: '<S370>/Integrator' */
  TX_DriveLine_DW.Integrator_IWORK_m = 1;

  /* InitializeConditions for Integrator: '<S369>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_c = TX_DriveLine_P.PropShaft_theta_o;

  /* InitializeConditions for Memory: '<S143>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_es = TX_DriveLine_P.Memory_InitialCondition_k;

  /* InitializeConditions for Integrator: '<S143>/Integrator' incorporates:
   *  Integrator: '<S135>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE_i5 = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_d = 0.0;
  }

  TX_DriveLine_DW.Integrator_IWORK_gr = 1;

  /* End of InitializeConditions for Integrator: '<S143>/Integrator' */

  /* InitializeConditions for Integrator: '<S142>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_fk = TX_DriveLine_P.TorsionalCompliance2_theta_o;

  /* InitializeConditions for Memory: '<S135>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_f = TX_DriveLine_P.Memory_InitialCondition_n;

  /* InitializeConditions for Integrator: '<S135>/Integrator' */
  TX_DriveLine_DW.Integrator_IWORK_e = 1;

  /* InitializeConditions for Integrator: '<S134>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_nq = TX_DriveLine_P.TorsionalCompliance1_theta_o;

  /* InitializeConditions for Memory: '<S311>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_o = TX_DriveLine_P.Memory_InitialCondition_nh;

  /* InitializeConditions for Integrator: '<S311>/Integrator' incorporates:
   *  Integrator: '<S319>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE_ng = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_hk = 0.0;
  }

  TX_DriveLine_DW.Integrator_IWORK_i = 1;

  /* End of InitializeConditions for Integrator: '<S311>/Integrator' */

  /* InitializeConditions for Integrator: '<S310>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_d1 = TX_DriveLine_P.TorsionalCompliance_theta_o;

  /* InitializeConditions for Memory: '<S319>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_h = TX_DriveLine_P.Memory_InitialCondition_f;

  /* InitializeConditions for Integrator: '<S319>/Integrator' */
  TX_DriveLine_DW.Integrator_IWORK_gy = 1;

  /* InitializeConditions for Integrator: '<S318>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_lg = TX_DriveLine_P.TorsionalCompliance1_theta_o_c;

  /* InitializeConditions for Memory: '<S244>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_n = TX_DriveLine_P.Memory_InitialCondition_c;

  /* InitializeConditions for Integrator: '<S244>/Integrator' incorporates:
   *  Integrator: '<S252>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE_lz = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_bb = 0.0;
  }

  TX_DriveLine_DW.Integrator_IWORK_ij = 1;

  /* End of InitializeConditions for Integrator: '<S244>/Integrator' */

  /* InitializeConditions for Integrator: '<S243>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_iy = TX_DriveLine_P.TorsionalCompliance4_theta_o;

  /* InitializeConditions for Memory: '<S252>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_b = TX_DriveLine_P.Memory_InitialCondition_o;

  /* InitializeConditions for Integrator: '<S252>/Integrator' */
  TX_DriveLine_DW.Integrator_IWORK_hd = 1;

  /* InitializeConditions for Integrator: '<S251>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_b2 = TX_DriveLine_P.TorsionalCompliance5_theta_o;

  /* InitializeConditions for Memory: '<S228>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_fa = TX_DriveLine_P.Memory_InitialCondition_ja;

  /* InitializeConditions for Integrator: '<S228>/Integrator' incorporates:
   *  Integrator: '<S236>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE_c1 = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_bx = 0.0;
  }

  TX_DriveLine_DW.Integrator_IWORK_hs = 1;

  /* End of InitializeConditions for Integrator: '<S228>/Integrator' */

  /* InitializeConditions for Integrator: '<S227>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_hkp = TX_DriveLine_P.TorsionalCompliance2_theta_o_l;

  /* InitializeConditions for Memory: '<S236>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_l = TX_DriveLine_P.Memory_InitialCondition_i;

  /* InitializeConditions for Integrator: '<S236>/Integrator' */
  TX_DriveLine_DW.Integrator_IWORK_n = 1;

  /* InitializeConditions for Integrator: '<S235>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_g1 = TX_DriveLine_P.TorsionalCompliance3_theta_o;

  /* InitializeConditions for Integrator: '<S108>/Integrator' incorporates:
   *  Integrator: '<S284>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE_h2[0] = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_h2[1] = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_c0[0] = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_c0[1] = 0.0;
  }

  TX_DriveLine_DW.Integrator_IWORK_p = 1;

  /* End of InitializeConditions for Integrator: '<S108>/Integrator' */

  /* InitializeConditions for Integrator: '<S284>/Integrator' */
  TX_DriveLine_DW.Integrator_IWORK_a = 1;

  /* InitializeConditions for Integrator: '<S158>/Integrator' incorporates:
   *  Integrator: '<S212>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE_g3[0] = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_g3[1] = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_db = 0.0;
  }

  TX_DriveLine_DW.Integrator_IWORK_ao = 1;

  /* End of InitializeConditions for Integrator: '<S158>/Integrator' */

  /* InitializeConditions for Integrator: '<S110>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_nv = TX_DriveLine_P.Integrator_IC;

  /* InitializeConditions for Memory: '<S212>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_bo = TX_DriveLine_P.Memory_InitialCondition_g;

  /* InitializeConditions for Integrator: '<S212>/Integrator' */
  TX_DriveLine_DW.Integrator_IWORK_ct = 1;

  /* InitializeConditions for Integrator: '<S211>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_dv = TX_DriveLine_P.TorsionalCompliance_theta_o_l;

  /* InitializeConditions for Integrator: '<S159>/Integrator' incorporates:
   *  Integrator: '<S220>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE_eu[0] = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_eu[1] = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_ny = 0.0;
  }

  TX_DriveLine_DW.Integrator_IWORK_b = 1;

  /* End of InitializeConditions for Integrator: '<S159>/Integrator' */

  /* InitializeConditions for Integrator: '<S161>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_ea = TX_DriveLine_P.Integrator_IC_l;

  /* InitializeConditions for Memory: '<S220>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_fn = TX_DriveLine_P.Memory_InitialCondition_nv;

  /* InitializeConditions for Integrator: '<S220>/Integrator' */
  TX_DriveLine_DW.Integrator_IWORK_o = 1;

  /* InitializeConditions for Integrator: '<S219>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_hz = TX_DriveLine_P.TorsionalCompliance1_theta_o_f;

  /* SystemInitialize for IfAction SubSystem: '<S331>/Locked' */
  /* InitializeConditions for Integrator: '<S185>/Integrator' incorporates:
   *  Integrator: '<S335>/x'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE_j2[0] = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_j2[1] = 0.0;
    TX_DriveLine_X.w = 0.0;
  }

  /* End of SystemInitialize for SubSystem: '<S331>/Locked' */
  TX_DriveLine_DW.Integrator_IWORK_f = 1;

  /* End of InitializeConditions for Integrator: '<S185>/Integrator' */

  /* InitializeConditions for Integrator: '<S187>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_a = TX_DriveLine_P.Integrator_IC_d;

  /* InitializeConditions for Integrator: '<S286>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_a4 = TX_DriveLine_P.Integrator_IC_f;

  /* InitializeConditions for Integrator: '<S382>/Integrator1' */
  TX_DriveLine_X.Integrator1_CSTATE = TX_DriveLine_P.Integrator1_IC;

  /* InitializeConditions for UnitDelay: '<S326>/Delay Input1' */
  TX_DriveLine_DW.DelayInput1_DSTATE = TX_DriveLine_P.GearCmdChange_vinit;

  /* SystemInitialize for IfAction SubSystem: '<S331>/Locked' */
  /* InitializeConditions for Integrator: '<S335>/x' */
  TX_DriveLine_DW.x_IWORK = 1;

  /* End of SystemInitialize for SubSystem: '<S331>/Locked' */

  /* SystemInitialize for IfAction SubSystem: '<S331>/Unlocked' */
  /* InitializeConditions for Integrator: '<S337>/xe' incorporates:
   *  Integrator: '<S337>/xv'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.we = 0.0;
    TX_DriveLine_X.wv = 0.0;
  }

  TX_DriveLine_DW.xe_IWORK = 1;

  /* End of InitializeConditions for Integrator: '<S337>/xe' */

  /* InitializeConditions for Integrator: '<S337>/xv' */
  TX_DriveLine_DW.xv_IWORK = 1;

  /* End of SystemInitialize for SubSystem: '<S331>/Unlocked' */

  /* SystemInitialize for Chart: '<S377>/TCM Shift Controller' */
  TX_DriveLine_DW.sfEvent = -1;

  /* End of SystemInitialize for SubSystem: '<S1>/ICEDriveLine' */

  /* SystemInitialize for IfAction SubSystem: '<S1>/EVDriveLine' */
  /* Start for Constant: '<S44>/domega_o' */
  TX_DriveLine_B.domega_o = TX_DriveLine_P.TorsionalCompliance2_domega_o_k;

  /* Start for Constant: '<S36>/domega_o' */
  TX_DriveLine_B.domega_o_h = TX_DriveLine_P.TorsionalCompliance1_domega_o_k;

  /* Start for Constant: '<S81>/domega_o' */
  TX_DriveLine_B.domega_o_o = TX_DriveLine_P.TorsionalCompliance_domega_o_i;

  /* Start for Constant: '<S89>/domega_o' */
  TX_DriveLine_B.domega_o_f = TX_DriveLine_P.TorsionalCompliance1_domega_o_p;

  /* InitializeConditions for Memory: '<S45>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput = TX_DriveLine_P.Memory_InitialCondition_d;

  /* InitializeConditions for Integrator: '<S45>/Integrator' incorporates:
   *  Integrator: '<S37>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_j = 0.0;
  }

  TX_DriveLine_DW.Integrator_IWORK = 1;

  /* End of InitializeConditions for Integrator: '<S45>/Integrator' */

  /* InitializeConditions for Integrator: '<S44>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_g = TX_DriveLine_P.TorsionalCompliance2_theta_o_m;

  /* InitializeConditions for Memory: '<S37>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_c = TX_DriveLine_P.Memory_InitialCondition_m;

  /* InitializeConditions for Integrator: '<S37>/Integrator' */
  TX_DriveLine_DW.Integrator_IWORK_h = 1;

  /* InitializeConditions for Integrator: '<S36>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_l = TX_DriveLine_P.TorsionalCompliance1_theta_o_n;

  /* InitializeConditions for Memory: '<S82>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_cw = TX_DriveLine_P.Memory_InitialCondition_o3;

  /* InitializeConditions for Integrator: '<S82>/Integrator' incorporates:
   *  Integrator: '<S90>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE_p = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_i = 0.0;
  }

  TX_DriveLine_DW.Integrator_IWORK_k = 1;

  /* End of InitializeConditions for Integrator: '<S82>/Integrator' */

  /* InitializeConditions for Integrator: '<S81>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_e = TX_DriveLine_P.TorsionalCompliance_theta_o_h;

  /* InitializeConditions for Memory: '<S90>/Memory' */
  TX_DriveLine_DW.Memory_PreviousInput_ch = TX_DriveLine_P.Memory_InitialCondition_is;

  /* InitializeConditions for Integrator: '<S90>/Integrator' */
  TX_DriveLine_DW.Integrator_IWORK_j = 1;

  /* InitializeConditions for Integrator: '<S89>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_im = TX_DriveLine_P.TorsionalCompliance1_theta_o_l;

  /* InitializeConditions for Integrator: '<S10>/Integrator' incorporates:
   *  Integrator: '<S55>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    TX_DriveLine_X.Integrator_CSTATE_b[0] = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_b[1] = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_n[0] = 0.0;
    TX_DriveLine_X.Integrator_CSTATE_n[1] = 0.0;
  }

  TX_DriveLine_DW.Integrator_IWORK_g = 1;

  /* End of InitializeConditions for Integrator: '<S10>/Integrator' */

  /* InitializeConditions for Integrator: '<S55>/Integrator' */
  TX_DriveLine_DW.Integrator_IWORK_d = 1;

  /* InitializeConditions for Integrator: '<S12>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_m = TX_DriveLine_P.Integrator_IC_lt;

  /* InitializeConditions for Integrator: '<S57>/Integrator' */
  TX_DriveLine_X.Integrator_CSTATE_ne = TX_DriveLine_P.Integrator_IC_k;

  /* End of SystemInitialize for SubSystem: '<S1>/EVDriveLine' */

  /* SystemInitialize for Merge: '<S1>/Merge' */
  TX_DriveLine_B.Merge = TX_DriveLine_P.Merge_InitialOutput;

  /* set "at time zero" to false */
  if (rtmIsFirstInitCond((&TX_DriveLine_M))) {
    rtmSetFirstInitCond((&TX_DriveLine_M), 0);
  }
}

/* Model terminate function */
void TX_DriveLine::terminate() { /* (no terminate code required) */
}

/* Constructor */
TX_DriveLine::TX_DriveLine()
    : TX_DriveLine_U(),
      TX_DriveLine_Y(),
      TX_DriveLine_B(),
      TX_DriveLine_DW(),
      TX_DriveLine_X(),
      TX_DriveLine_PrevZCX(),
      TX_DriveLine_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
TX_DriveLine::~TX_DriveLine() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_TX_DriveLine_T *TX_DriveLine::getRTM() { return (&TX_DriveLine_M); }
