/*
 * HEVControllerPwr.cpp
 *
 * Code generation for model "HEVControllerPwr".
 *
 * Model version              : 1.1440
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Dec 27 10:10:55 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "HEVControllerPwr.h"
#include <cmath>
#include "HEVControllerPwr_private.h"
#include "HEVControllerPwr_types.h"
#include "rtwtypes.h"

extern "C" {

#include "rt_nonfinite.h"
}

/* Named constants for Chart: '<S2>/HEV_DriveMode' */
const uint8_T HEVControllerPwr_IN_EVDrive_SerializeIncluded{1U};

const uint8_T HEVControllerPwr_IN_EngineDrive{2U};

const uint8_T HEVControllerPwr_IN_HybridDrive_Parallel{3U};

const uint8_T HEVControllerPwr_IN_NO_ACTIVE_CHILD{0U};

/* Named constants for Chart: '<S11>/PwrCal' */
const uint8_T HEVControllerPwr_IN_EV{1U};

const uint8_T HEVControllerPwr_IN_Engine{2U};

const uint8_T HEVControllerPwr_IN_HEVDrive{1U};

const uint8_T HEVControllerPwr_IN_HEV_ReGenMode{2U};

const uint8_T HEVControllerPwr_IN_Hybrid{3U};

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void HEVControllerPwr::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  HEVControllerPwr_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  HEVControllerPwr_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  HEVControllerPwr_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  HEVControllerPwr_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/*
 * Output and update for action system:
 *    '<S23>/NoPwrMode'
 *    '<S24>/NoPwrMode'
 *    '<S25>/NoPwrMode'
 *    '<S26>/NoPwrMode'
 */
void HEVControllerPwr::HEVControllerPwr_NoPwrMode(HEVPwrDmnd_Output *rty_HEV_PwrOutput, real_T rtp_EngIdleSpd,
                                                  P_NoPwrMode_HEVControllerPwr_T *localP) {
  /* BusCreator: '<S28>/Bus Creator' incorporates:
   *  Constant: '<S28>/EngIdleSpd_RPM'
   *  Constant: '<S28>/Zero'
   */
  rty_HEV_PwrOutput->EV_PwrDmnd = localP->Zero_Value;
  rty_HEV_PwrOutput->Engine_TractionPwrDmnd = localP->Zero_Value;
  rty_HEV_PwrOutput->Engine_SerializePwrDmnd = localP->Zero_Value;
  rty_HEV_PwrOutput->Engine_ChargingPwrDmnd = localP->Zero_Value;
  rty_HEV_PwrOutput->P4_Motor_Enabled = localP->Zero_Value;
  rty_HEV_PwrOutput->P4_PwrDmnd = localP->Zero_Value;
  rty_HEV_PwrOutput->Engine_Request_Spd_RPM = rtp_EngIdleSpd;
}

/* System initialize for function-call system: '<S22>/EV_Pwr_Calculation' */
void HEVControllerPwr::HEVControllerPwr_EV_Pwr_Calculation_Init(B_EV_Pwr_Calculation_HEVControllerPwr_T *localB,
                                                                P_EV_Pwr_Calculation_HEVControllerPwr_T *localP) {
  /* SystemInitialize for Merge: '<S23>/Merge' */
  localB->Merge = localP->Merge_InitialOutput;
}

/* Output and update for function-call system: '<S22>/EV_Pwr_Calculation' */
void HEVControllerPwr::HEVControllerPwr_EV_Pwr_Calculation(
    const HEVMode_Output *rtu_HEV_Mode, real_T rtu_Pwr_Dmnd_w_, real_T rtp_HEV_4WD_FrontPwrRatio, real_T rtp_EnableP4,
    real_T rtp_EngIdleSpd, real_T rtp_MaxChrgPwr, real_T rtp_DefaultChrgPwr,
    const real_T rtp_Inverse_EngOptimal_Speed_RPM[11], const real_T rtp_Inverse_EngOptimal_Pwr_KW[11],
    B_EV_Pwr_Calculation_HEVControllerPwr_T *localB, P_EV_Pwr_Calculation_HEVControllerPwr_T *localP,
    P_HEVControllerPwr_T *HEVControllerPwr_P) {
  /* SwitchCase: '<S23>/Switch Case' */
  switch (static_cast<int32_T>(rtu_HEV_Mode->HEV_Mode)) {
    case 0: {
      real_T rtb_Front_PwrDmnd_p;
      real_T rtb_P4_PwrDmnd_k;

      /* Outputs for IfAction SubSystem: '<S23>/EVPwrMode' incorporates:
       *  ActionPort: '<S27>/Action Port'
       */
      /* Product: '<S30>/Front_PwrDmnd' incorporates:
       *  Constant: '<S30>/HEV_4WD_FrontPwrRatio'
       */
      rtb_Front_PwrDmnd_p = rtu_Pwr_Dmnd_w_ * rtp_HEV_4WD_FrontPwrRatio;

      /* Switch: '<S30>/Rear' incorporates:
       *  Constant: '<S27>/EnableP4'
       *  Constant: '<S30>/Zero'
       *  Sum: '<S30>/RearPwrDmnd'
       */
      if (rtp_EnableP4 > localP->Rear_Threshold) {
        rtb_P4_PwrDmnd_k = rtu_Pwr_Dmnd_w_ - rtb_Front_PwrDmnd_p;
      } else {
        rtb_P4_PwrDmnd_k = localP->Zero_Value;
      }

      /* End of Switch: '<S30>/Rear' */

      /* Switch: '<S30>/Front' incorporates:
       *  Constant: '<S27>/EnableP4'
       */
      if (!(rtp_EnableP4 > localP->Front_Threshold)) {
        rtb_Front_PwrDmnd_p = rtu_Pwr_Dmnd_w_;
      }

      /* Saturate: '<S30>/Saturation' incorporates:
       *  Switch: '<S30>/Front'
       */
      if (rtb_Front_PwrDmnd_p > localP->Saturation_UpperSat) {
        /* BusCreator: '<S27>/Bus Creator' */
        localB->Merge.EV_PwrDmnd = localP->Saturation_UpperSat;
      } else if (rtb_Front_PwrDmnd_p < localP->Saturation_LowerSat) {
        /* BusCreator: '<S27>/Bus Creator' */
        localB->Merge.EV_PwrDmnd = localP->Saturation_LowerSat;
      } else {
        /* BusCreator: '<S27>/Bus Creator' */
        localB->Merge.EV_PwrDmnd = rtb_Front_PwrDmnd_p;
      }

      /* End of Saturate: '<S30>/Saturation' */

      /* BusCreator: '<S27>/Bus Creator' incorporates:
       *  Constant: '<S27>/EnableP4'
       *  Constant: '<S27>/Zero'
       */
      localB->Merge.Engine_TractionPwrDmnd = localP->Zero_Value_i;
      localB->Merge.Engine_SerializePwrDmnd = localP->Zero_Value_i;
      localB->Merge.Engine_ChargingPwrDmnd = localP->Zero_Value_i;
      localB->Merge.P4_Motor_Enabled = rtp_EnableP4;

      /* Saturate: '<S30>/Saturation1' */
      if (rtb_P4_PwrDmnd_k > localP->Saturation1_UpperSat) {
        /* BusCreator: '<S27>/Bus Creator' */
        localB->Merge.P4_PwrDmnd = localP->Saturation1_UpperSat;
      } else if (rtb_P4_PwrDmnd_k < localP->Saturation1_LowerSat) {
        /* BusCreator: '<S27>/Bus Creator' */
        localB->Merge.P4_PwrDmnd = localP->Saturation1_LowerSat;
      } else {
        /* BusCreator: '<S27>/Bus Creator' */
        localB->Merge.P4_PwrDmnd = rtb_P4_PwrDmnd_k;
      }

      /* End of Saturate: '<S30>/Saturation1' */

      /* BusCreator: '<S27>/Bus Creator' incorporates:
       *  Constant: '<S27>/EngIdleSpd_RPM'
       */
      localB->Merge.Engine_Request_Spd_RPM = rtp_EngIdleSpd;

      /* End of Outputs for SubSystem: '<S23>/EVPwrMode' */
    } break;

    case 4: {
      real_T rtb_Front_PwrDmnd;
      real_T rtb_Front_PwrDmnd_p;
      real_T rtb_P4_PwrDmnd_k;

      /* Outputs for IfAction SubSystem: '<S23>/SerializePwrMode' incorporates:
       *  ActionPort: '<S29>/Action Port'
       */
      /* Saturate: '<S29>/SerializePwrSaturation' */
      rtb_P4_PwrDmnd_k = rtp_MaxChrgPwr - rtp_DefaultChrgPwr;
      if (!(rtu_Pwr_Dmnd_w_ > rtb_P4_PwrDmnd_k)) {
        if (rtu_Pwr_Dmnd_w_ < localP->SerializePwrSaturation_LowerSat) {
          rtb_P4_PwrDmnd_k = localP->SerializePwrSaturation_LowerSat;
        } else {
          rtb_P4_PwrDmnd_k = rtu_Pwr_Dmnd_w_;
        }
      }

      /* End of Saturate: '<S29>/SerializePwrSaturation' */

      /* Product: '<S31>/Front_PwrDmnd' incorporates:
       *  Constant: '<S31>/HEV_4WD_FrontPwrRatio'
       */
      rtb_Front_PwrDmnd = rtb_P4_PwrDmnd_k * rtp_HEV_4WD_FrontPwrRatio;

      /* Switch: '<S31>/Front' incorporates:
       *  Constant: '<S29>/EnableP4'
       */
      if (rtp_EnableP4 > localP->Front_Threshold_b) {
        rtb_Front_PwrDmnd_p = rtb_Front_PwrDmnd;
      } else {
        rtb_Front_PwrDmnd_p = rtb_P4_PwrDmnd_k;
      }

      /* Saturate: '<S31>/Saturation1' incorporates:
       *  Switch: '<S31>/Front'
       */
      if (rtb_Front_PwrDmnd_p > localP->Saturation1_UpperSat_o) {
        rtb_Front_PwrDmnd_p = localP->Saturation1_UpperSat_o;
      } else if (rtb_Front_PwrDmnd_p < localP->Saturation1_LowerSat_d) {
        rtb_Front_PwrDmnd_p = localP->Saturation1_LowerSat_d;
      }

      /* End of Saturate: '<S31>/Saturation1' */

      /* Switch: '<S31>/Rear' incorporates:
       *  Constant: '<S29>/EnableP4'
       *  Constant: '<S31>/Zero'
       *  Sum: '<S31>/RearPwrDmnd'
       */
      if (rtp_EnableP4 > localP->Rear_Threshold_m) {
        rtb_P4_PwrDmnd_k -= rtb_Front_PwrDmnd;
      } else {
        rtb_P4_PwrDmnd_k = localP->Zero_Value_l;
      }

      /* End of Switch: '<S31>/Rear' */

      /* Saturate: '<S31>/Saturation2' */
      if (rtb_P4_PwrDmnd_k > localP->Saturation2_UpperSat) {
        rtb_P4_PwrDmnd_k = localP->Saturation2_UpperSat;
      } else if (rtb_P4_PwrDmnd_k < localP->Saturation2_LowerSat) {
        rtb_P4_PwrDmnd_k = localP->Saturation2_LowerSat;
      }

      /* End of Saturate: '<S31>/Saturation2' */

      /* BusCreator: '<S29>/Bus Creator' incorporates:
       *  Constant: '<S29>/DefaultChrgPwr'
       *  Constant: '<S29>/EnableP4'
       *  Constant: '<S29>/Zero'
       */
      localB->Merge.EV_PwrDmnd = rtb_Front_PwrDmnd_p;
      localB->Merge.Engine_TractionPwrDmnd = localP->Zero_Value_ij;
      localB->Merge.Engine_SerializePwrDmnd = rtb_Front_PwrDmnd_p;
      localB->Merge.Engine_ChargingPwrDmnd = rtp_DefaultChrgPwr;
      localB->Merge.P4_Motor_Enabled = rtp_EnableP4;
      localB->Merge.P4_PwrDmnd = rtb_P4_PwrDmnd_k;

      /* Lookup_n-D: '<S29>/Eng_Optimal_Pwr' incorporates:
       *  Constant: '<S29>/DefaultChrgPwr'
       *  Gain: '<S29>/KW'
       *  Sum: '<S29>/Add'
       */
      rtb_Front_PwrDmnd_p =
          look1_binlxpw(((rtb_Front_PwrDmnd_p + rtb_P4_PwrDmnd_k) + rtp_DefaultChrgPwr) * localP->KW_Gain,
                        rtp_Inverse_EngOptimal_Pwr_KW, rtp_Inverse_EngOptimal_Speed_RPM, 10U);

      /* Saturate: '<S29>/Saturation' */
      if (rtb_Front_PwrDmnd_p > HEVControllerPwr_P->EngMaxSpd) {
        /* BusCreator: '<S29>/Bus Creator' */
        localB->Merge.Engine_Request_Spd_RPM = HEVControllerPwr_P->EngMaxSpd;
      } else if (rtb_Front_PwrDmnd_p < rtp_EngIdleSpd) {
        /* BusCreator: '<S29>/Bus Creator' */
        localB->Merge.Engine_Request_Spd_RPM = rtp_EngIdleSpd;
      } else {
        /* BusCreator: '<S29>/Bus Creator' */
        localB->Merge.Engine_Request_Spd_RPM = rtb_Front_PwrDmnd_p;
      }

      /* End of Saturate: '<S29>/Saturation' */
      /* End of Outputs for SubSystem: '<S23>/SerializePwrMode' */
    } break;

    default:
      /* Outputs for IfAction SubSystem: '<S23>/NoPwrMode' incorporates:
       *  ActionPort: '<S28>/Action Port'
       */
      HEVControllerPwr_NoPwrMode(&localB->Merge, rtp_EngIdleSpd, &localP->NoPwrMode);

      /* End of Outputs for SubSystem: '<S23>/NoPwrMode' */
      break;
  }

  /* End of SwitchCase: '<S23>/Switch Case' */
}

/* System initialize for function-call system: '<S22>/Engine_Pwr_Calculation' */
void HEVControllerPwr::HEVControllerPwr_Engine_Pwr_Calculation_Init(
    B_Engine_Pwr_Calculation_HEVControllerPwr_T *localB, P_Engine_Pwr_Calculation_HEVControllerPwr_T *localP) {
  /* SystemInitialize for Merge: '<S24>/Merge' */
  localB->Merge = localP->Merge_InitialOutput;
}

/* Output and update for function-call system: '<S22>/Engine_Pwr_Calculation' */
void HEVControllerPwr::HEVControllerPwr_Engine_Pwr_Calculation(
    const HEVMode_Output *rtu_HEV_Mode, real_T rtu_Pwr_Dmnd_w_, real_T rtu_Engine_Speed_rpm_, real_T rtp_DefaultChrgPwr,
    const real_T rtp_EngOptimal_Pwr_KW[11], const real_T rtp_EngOptimal_Speed_RPM[11], real_T rtp_MaxChrgPwr,
    real_T rtp_EngIdleSpd, B_Engine_Pwr_Calculation_HEVControllerPwr_T *localB,
    P_Engine_Pwr_Calculation_HEVControllerPwr_T *localP) {
  /* SwitchCase: '<S24>/Switch Case' */
  if (static_cast<int32_T>(rtu_HEV_Mode->HEV_Mode) == 1) {
    /* Outputs for IfAction SubSystem: '<S24>/EnginePwrMode' incorporates:
     *  ActionPort: '<S32>/Action Port'
     */
    /* Switch: '<S32>/ChargingPwr' */
    if (rtu_HEV_Mode->Engine_ChargingSwitch > localP->ChargingPwr_Threshold) {
      real_T rtb_Add_b;

      /* Sum: '<S32>/Add' incorporates:
       *  Gain: '<S32>/Gain'
       *  Lookup_n-D: '<S32>/EngineOptimalPwr'
       */
      rtb_Add_b = localP->Gain_Gain *
                      look1_binlxpw(rtu_Engine_Speed_rpm_, rtp_EngOptimal_Speed_RPM, rtp_EngOptimal_Pwr_KW, 10U) -
                  rtu_Pwr_Dmnd_w_;

      /* Switch: '<S34>/Switch2' incorporates:
       *  Constant: '<S32>/DefaultChargePwr'
       *  Constant: '<S32>/MaxChrgPwr'
       *  RelationalOperator: '<S34>/LowerRelop1'
       *  RelationalOperator: '<S34>/UpperRelop'
       *  Switch: '<S34>/Switch'
       */
      if (rtb_Add_b > rtp_MaxChrgPwr) {
        /* BusCreator: '<S32>/Bus Creator' */
        localB->Merge.Engine_ChargingPwrDmnd = rtp_MaxChrgPwr;
      } else if (rtb_Add_b < rtp_DefaultChrgPwr) {
        /* Switch: '<S34>/Switch' incorporates:
         *  BusCreator: '<S32>/Bus Creator'
         *  Constant: '<S32>/DefaultChargePwr'
         */
        localB->Merge.Engine_ChargingPwrDmnd = rtp_DefaultChrgPwr;
      } else {
        /* BusCreator: '<S32>/Bus Creator' incorporates:
         *  Switch: '<S34>/Switch'
         */
        localB->Merge.Engine_ChargingPwrDmnd = rtb_Add_b;
      }

      /* End of Switch: '<S34>/Switch2' */
    } else {
      /* BusCreator: '<S32>/Bus Creator' incorporates:
       *  Constant: '<S32>/Zero'
       */
      localB->Merge.Engine_ChargingPwrDmnd = localP->Zero_Value;
    }

    /* End of Switch: '<S32>/ChargingPwr' */

    /* BusCreator: '<S32>/Bus Creator' incorporates:
     *  Constant: '<S32>/EngIdleSpd_RPM'
     *  Constant: '<S32>/Zero'
     */
    localB->Merge.EV_PwrDmnd = localP->Zero_Value;
    localB->Merge.Engine_TractionPwrDmnd = rtu_Pwr_Dmnd_w_;
    localB->Merge.Engine_SerializePwrDmnd = localP->Zero_Value;
    localB->Merge.P4_Motor_Enabled = localP->Zero_Value;
    localB->Merge.P4_PwrDmnd = localP->Zero_Value;
    localB->Merge.Engine_Request_Spd_RPM = rtp_EngIdleSpd;

    /* End of Outputs for SubSystem: '<S24>/EnginePwrMode' */
  } else {
    /* Outputs for IfAction SubSystem: '<S24>/NoPwrMode' incorporates:
     *  ActionPort: '<S33>/Action Port'
     */
    HEVControllerPwr_NoPwrMode(&localB->Merge, rtp_EngIdleSpd, &localP->NoPwrMode);

    /* End of Outputs for SubSystem: '<S24>/NoPwrMode' */
  }

  /* End of SwitchCase: '<S24>/Switch Case' */
}

/* System initialize for function-call system: '<S22>/HEV_Pwr_Calculation' */
void HEVControllerPwr::HEVControllerPwr_HEV_Pwr_Calculation_Init(B_HEV_Pwr_Calculation_HEVControllerPwr_T *localB,
                                                                 P_HEV_Pwr_Calculation_HEVControllerPwr_T *localP) {
  /* SystemInitialize for Merge: '<S25>/Merge' */
  localB->Merge = localP->Merge_InitialOutput;
}

/* Output and update for function-call system: '<S22>/HEV_Pwr_Calculation' */
void HEVControllerPwr::HEVControllerPwr_HEV_Pwr_Calculation(
    const HEVMode_Output *rtu_HEV_Mode, real_T rtu_Pwr_Dmnd_w_, real_T rtu_Engine_Speed_rpm_,
    const real_T rtp_EngOptimal_Pwr_KW[11], const real_T rtp_EngOptimal_Speed_RPM[11], real_T rtp_HEV_4WD_FrontPwrRatio,
    real_T rtp_EnableP4, real_T rtp_EngIdleSpd, B_HEV_Pwr_Calculation_HEVControllerPwr_T *localB,
    P_HEV_Pwr_Calculation_HEVControllerPwr_T *localP) {
  /* SwitchCase: '<S25>/Switch Case' */
  if (static_cast<int32_T>(rtu_HEV_Mode->HEV_Mode) == 2) {
    real_T rtb_Front_PwrDmnd;
    real_T rtb_Gain_n;
    real_T rtb_P4_PwrDmnd_k;
    real_T rtb_Saturation_e;

    /* Outputs for IfAction SubSystem: '<S25>/HybridPwrMode' incorporates:
     *  ActionPort: '<S35>/Action Port'
     */
    /* Gain: '<S35>/Gain' incorporates:
     *  Lookup_n-D: '<S35>/EngineOptimalPwr'
     */
    rtb_Gain_n =
        localP->Gain_Gain * look1_binlxpw(rtu_Engine_Speed_rpm_, rtp_EngOptimal_Speed_RPM, rtp_EngOptimal_Pwr_KW, 10U);

    /* Switch: '<S37>/Switch2' incorporates:
     *  Constant: '<S35>/Constant1'
     *  RelationalOperator: '<S37>/LowerRelop1'
     *  RelationalOperator: '<S37>/UpperRelop'
     *  Switch: '<S37>/Switch'
     */
    if (rtb_Gain_n > rtu_Pwr_Dmnd_w_) {
      rtb_Gain_n = rtu_Pwr_Dmnd_w_;
    } else if (rtb_Gain_n < localP->Constant1_Value) {
      /* Switch: '<S37>/Switch' incorporates:
       *  Constant: '<S35>/Constant1'
       */
      rtb_Gain_n = localP->Constant1_Value;
    }

    /* End of Switch: '<S37>/Switch2' */

    /* Sum: '<S35>/Add' */
    rtb_Saturation_e = rtu_Pwr_Dmnd_w_ - rtb_Gain_n;

    /* Saturate: '<S35>/Saturation' */
    if (rtb_Saturation_e > localP->Saturation_UpperSat) {
      rtb_Saturation_e = localP->Saturation_UpperSat;
    } else if (rtb_Saturation_e < localP->Saturation_LowerSat) {
      rtb_Saturation_e = localP->Saturation_LowerSat;
    }

    /* End of Saturate: '<S35>/Saturation' */

    /* Product: '<S38>/Front_PwrDmnd' incorporates:
     *  Constant: '<S38>/HEV_4WD_FrontPwrRatio'
     */
    rtb_Front_PwrDmnd = rtb_Saturation_e * rtp_HEV_4WD_FrontPwrRatio;

    /* Switch: '<S38>/Rear' incorporates:
     *  Constant: '<S35>/EnableP4'
     *  Constant: '<S38>/Zero'
     *  Sum: '<S38>/RearPwrDmnd'
     */
    if (rtp_EnableP4 > localP->Rear_Threshold) {
      rtb_P4_PwrDmnd_k = rtb_Saturation_e - rtb_Front_PwrDmnd;
    } else {
      rtb_P4_PwrDmnd_k = localP->Zero_Value;
    }

    /* End of Switch: '<S38>/Rear' */

    /* Switch: '<S38>/Front' incorporates:
     *  Constant: '<S35>/EnableP4'
     */
    if (rtp_EnableP4 > localP->Front_Threshold) {
      rtb_Saturation_e = rtb_Front_PwrDmnd;
    }

    /* Saturate: '<S38>/Saturation' incorporates:
     *  Switch: '<S38>/Front'
     */
    if (rtb_Saturation_e > localP->Saturation_UpperSat_l) {
      /* BusCreator: '<S35>/Bus Creator' */
      localB->Merge.EV_PwrDmnd = localP->Saturation_UpperSat_l;
    } else if (rtb_Saturation_e < localP->Saturation_LowerSat_d) {
      /* BusCreator: '<S35>/Bus Creator' */
      localB->Merge.EV_PwrDmnd = localP->Saturation_LowerSat_d;
    } else {
      /* BusCreator: '<S35>/Bus Creator' */
      localB->Merge.EV_PwrDmnd = rtb_Saturation_e;
    }

    /* End of Saturate: '<S38>/Saturation' */

    /* BusCreator: '<S35>/Bus Creator' incorporates:
     *  Constant: '<S35>/EnableP4'
     *  Constant: '<S35>/Zero'
     */
    localB->Merge.Engine_TractionPwrDmnd = rtb_Gain_n;
    localB->Merge.Engine_SerializePwrDmnd = localP->Zero_Value_c;
    localB->Merge.Engine_ChargingPwrDmnd = localP->Zero_Value_c;
    localB->Merge.P4_Motor_Enabled = rtp_EnableP4;

    /* Saturate: '<S38>/Saturation1' */
    if (rtb_P4_PwrDmnd_k > localP->Saturation1_UpperSat) {
      /* BusCreator: '<S35>/Bus Creator' */
      localB->Merge.P4_PwrDmnd = localP->Saturation1_UpperSat;
    } else if (rtb_P4_PwrDmnd_k < localP->Saturation1_LowerSat) {
      /* BusCreator: '<S35>/Bus Creator' */
      localB->Merge.P4_PwrDmnd = localP->Saturation1_LowerSat;
    } else {
      /* BusCreator: '<S35>/Bus Creator' */
      localB->Merge.P4_PwrDmnd = rtb_P4_PwrDmnd_k;
    }

    /* End of Saturate: '<S38>/Saturation1' */

    /* BusCreator: '<S35>/Bus Creator' incorporates:
     *  Constant: '<S35>/EngIdleSpd_RPM'
     */
    localB->Merge.Engine_Request_Spd_RPM = rtp_EngIdleSpd;

    /* End of Outputs for SubSystem: '<S25>/HybridPwrMode' */
  } else {
    /* Outputs for IfAction SubSystem: '<S25>/NoPwrMode' incorporates:
     *  ActionPort: '<S36>/Action Port'
     */
    HEVControllerPwr_NoPwrMode(&localB->Merge, rtp_EngIdleSpd, &localP->NoPwrMode);

    /* End of Outputs for SubSystem: '<S25>/NoPwrMode' */
  }

  /* End of SwitchCase: '<S25>/Switch Case' */
}

/* System initialize for function-call system: '<S22>/ReGen_Pwr_Calculation' */
void HEVControllerPwr::HEVControllerPwr_ReGen_Pwr_Calculation_Init(B_ReGen_Pwr_Calculation_HEVControllerPwr_T *localB,
                                                                   P_ReGen_Pwr_Calculation_HEVControllerPwr_T *localP) {
  /* SystemInitialize for Merge: '<S26>/Merge' */
  localB->Merge = localP->Merge_InitialOutput;
}

/* Output and update for function-call system: '<S22>/ReGen_Pwr_Calculation' */
void HEVControllerPwr::HEVControllerPwr_ReGen_Pwr_Calculation(
    const HEVMode_Output *rtu_HEV_Mode, real_T rtu_Motor_Speed_rads_, real_T rtu_P4_Motor_Speed_rads_,
    const real_T rtp_FrontMotor_MaxTorque[13], const real_T rtp_FrontMotor_Speed_rads[13], real_T rtp_EnableP4,
    const real_T rtp_P4_Motor_MaxTorque[13], const real_T rtp_P4_Motor_Speed_rads[13], real_T rtp_EngIdleSpd,
    real_T rtp_MaxReGenPwr, B_ReGen_Pwr_Calculation_HEVControllerPwr_T *localB,
    P_ReGen_Pwr_Calculation_HEVControllerPwr_T *localP) {
  real_T rtb_Product_c;
  real_T rtb_Switch2_d;

  /* Product: '<S41>/Divide' incorporates:
   *  Constant: '<S41>/Constant'
   */
  rtb_Switch2_d = rtu_Motor_Speed_rads_ / localP->Constant_Value;

  /* Saturate: '<S41>/Saturation' */
  if (rtb_Switch2_d > localP->Saturation_UpperSat) {
    rtb_Switch2_d = localP->Saturation_UpperSat;
  } else if (rtb_Switch2_d < localP->Saturation_LowerSat) {
    rtb_Switch2_d = localP->Saturation_LowerSat;
  }

  /* Product: '<S41>/Product' incorporates:
   *  Constant: '<S26>/MaxReGenPwr'
   *  Saturate: '<S41>/Saturation'
   */
  rtb_Product_c = rtb_Switch2_d * rtp_MaxReGenPwr;

  /* SwitchCase: '<S26>/Switch Case' */
  if (static_cast<int32_T>(rtu_HEV_Mode->HEV_Mode) == 3) {
    real_T rtb_Switch2_p;
    real_T tmp;

    /* Outputs for IfAction SubSystem: '<S26>/HybridPwrMode' incorporates:
     *  ActionPort: '<S39>/Action Port'
     */
    /* Abs: '<S39>/Abs' */
    rtb_Switch2_d = std::abs(rtu_Motor_Speed_rads_);

    /* Abs: '<S39>/Abs1' */
    rtb_Switch2_p = std::abs(rtu_P4_Motor_Speed_rads_);

    /* Product: '<S39>/MotorMaxPwr' incorporates:
     *  Lookup_n-D: '<S39>/MotorMaxTrq'
     *  Switch: '<S43>/Switch2'
     */
    rtb_Switch2_d *= look1_binlxpw(rtb_Switch2_d, rtp_FrontMotor_Speed_rads, rtp_FrontMotor_MaxTorque, 12U);

    /* Switch: '<S42>/Switch' incorporates:
     *  Constant: '<S39>/EnableP4'
     *  Constant: '<S42>/All'
     *  Constant: '<S42>/Half'
     */
    if (rtp_EnableP4 > localP->Switch_Threshold) {
      tmp = localP->Half_Value;
    } else {
      tmp = localP->All_Value;
    }

    /* DeadZone: '<S39>/ReGenPwr_DeadZone' */
    if (rtb_Product_c > localP->ReGenPwr_DeadZone_End) {
      rtb_Product_c -= localP->ReGenPwr_DeadZone_End;
    } else if (rtb_Product_c >= localP->ReGenPwr_DeadZone_Start) {
      rtb_Product_c = 0.0;
    } else {
      rtb_Product_c -= localP->ReGenPwr_DeadZone_Start;
    }

    /* Product: '<S42>/Product1' incorporates:
     *  DeadZone: '<S39>/ReGenPwr_DeadZone'
     *  Switch: '<S42>/Switch'
     */
    rtb_Product_c *= tmp;

    /* Switch: '<S43>/Switch2' incorporates:
     *  RelationalOperator: '<S43>/LowerRelop1'
     */
    if (!(rtb_Product_c > rtb_Switch2_d)) {
      /* Switch: '<S43>/Switch' incorporates:
       *  Constant: '<S39>/Constant1'
       *  RelationalOperator: '<S43>/UpperRelop'
       */
      if (rtb_Product_c < localP->Constant1_Value) {
        rtb_Switch2_d = localP->Constant1_Value;
      } else {
        rtb_Switch2_d = rtb_Product_c;
      }

      /* End of Switch: '<S43>/Switch' */
    }

    /* End of Switch: '<S43>/Switch2' */

    /* Product: '<S39>/MotorMaxPwr_P4' incorporates:
     *  Lookup_n-D: '<S39>/MotorMaxTrq_P4'
     *  Switch: '<S44>/Switch2'
     */
    rtb_Switch2_p *= look1_binlxpw(rtb_Switch2_p, rtp_P4_Motor_Speed_rads, rtp_P4_Motor_MaxTorque, 12U);

    /* Switch: '<S44>/Switch2' incorporates:
     *  RelationalOperator: '<S44>/LowerRelop1'
     */
    if (!(rtb_Product_c > rtb_Switch2_p)) {
      /* Switch: '<S44>/Switch' incorporates:
       *  Constant: '<S39>/Constant1'
       *  RelationalOperator: '<S44>/UpperRelop'
       */
      if (rtb_Product_c < localP->Constant1_Value) {
        rtb_Switch2_p = localP->Constant1_Value;
      } else {
        rtb_Switch2_p = rtb_Product_c;
      }

      /* End of Switch: '<S44>/Switch' */
    }

    /* End of Switch: '<S44>/Switch2' */

    /* BusCreator: '<S39>/Bus Creator' incorporates:
     *  Constant: '<S39>/Constant1'
     *  Constant: '<S39>/EnableP4'
     *  Constant: '<S39>/EngIdleSpd_RPM'
     *  Gain: '<S39>/Gain'
     *  Gain: '<S39>/Gain1'
     *  Product: '<S39>/Product'
     */
    localB->Merge.EV_PwrDmnd = localP->Gain_Gain * rtb_Switch2_d;
    localB->Merge.Engine_TractionPwrDmnd = localP->Constant1_Value;
    localB->Merge.Engine_SerializePwrDmnd = localP->Constant1_Value;
    localB->Merge.Engine_ChargingPwrDmnd = localP->Constant1_Value;
    localB->Merge.P4_Motor_Enabled = rtp_EnableP4;
    localB->Merge.P4_PwrDmnd = localP->Gain1_Gain * rtb_Switch2_p * rtp_EnableP4;
    localB->Merge.Engine_Request_Spd_RPM = rtp_EngIdleSpd;

    /* End of Outputs for SubSystem: '<S26>/HybridPwrMode' */
  } else {
    /* Outputs for IfAction SubSystem: '<S26>/NoPwrMode' incorporates:
     *  ActionPort: '<S40>/Action Port'
     */
    HEVControllerPwr_NoPwrMode(&localB->Merge, rtp_EngIdleSpd, &localP->NoPwrMode);

    /* End of Outputs for SubSystem: '<S26>/NoPwrMode' */
  }

  /* End of SwitchCase: '<S26>/Switch Case' */
}

/* System initialize for atomic system: '<S4>/ABS_EBD' */
void HEVControllerPwr::HEVControllerPwr_ABS_EBD_Init() {
  /* Start for If: '<S49>/If' */
  HEVControllerPwr_DW.If_ActiveSubsystem_m = -1;

  /* InitializeConditions for Memory: '<S49>/Memory' */
  HEVControllerPwr_DW.Memory_PreviousInput[0] = HEVControllerPwr_P.Memory_InitialCondition;
  HEVControllerPwr_DW.Memory_PreviousInput[1] = HEVControllerPwr_P.Memory_InitialCondition;
  HEVControllerPwr_DW.Memory_PreviousInput[2] = HEVControllerPwr_P.Memory_InitialCondition;
  HEVControllerPwr_DW.Memory_PreviousInput[3] = HEVControllerPwr_P.Memory_InitialCondition;

  /* SystemInitialize for IfAction SubSystem: '<S49>/accControl' */
  /* InitializeConditions for DiscreteIntegrator: '<S85>/Filter' */
  HEVControllerPwr_DW.Filter_DSTATE_l = HEVControllerPwr_P.DiscretePIDController_InitialConditionForFilter;

  /* InitializeConditions for DiscreteIntegrator: '<S90>/Integrator' */
  HEVControllerPwr_DW.Integrator_DSTATE_c = HEVControllerPwr_P.DiscretePIDController_InitialConditionForIntegrator;

  /* End of SystemInitialize for SubSystem: '<S49>/accControl' */
}

/* Outputs for atomic system: '<S4>/ABS_EBD' */
void HEVControllerPwr::HEVControllerPwr_ABS_EBD() {
  real_T rtb_Merge;
  int8_T rtAction;
  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* Memory: '<S49>/Memory' */
    HEVControllerPwr_B.Memory[0] = HEVControllerPwr_DW.Memory_PreviousInput[0];
    HEVControllerPwr_B.Memory[1] = HEVControllerPwr_DW.Memory_PreviousInput[1];
    HEVControllerPwr_B.Memory[2] = HEVControllerPwr_DW.Memory_PreviousInput[2];
    HEVControllerPwr_B.Memory[3] = HEVControllerPwr_DW.Memory_PreviousInput[3];
  }

  /* If: '<S49>/If' */
  if (rtsiIsModeUpdateTimeStep(&(&HEVControllerPwr_M)->solverInfo)) {
    rtAction = static_cast<int8_T>(HEVControllerPwr_B.brake_control_mode != 1);
    HEVControllerPwr_DW.If_ActiveSubsystem_m = rtAction;
  } else {
    rtAction = HEVControllerPwr_DW.If_ActiveSubsystem_m;
  }

  switch (rtAction) {
    case 0:
      /* Outputs for IfAction SubSystem: '<S49>/pressureControl' incorporates:
       *  ActionPort: '<S56>/Action Port'
       */
      if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
        /* Merge: '<S49>/Merge1' incorporates:
         *  Constant: '<S113>/Constant'
         *  Constant: '<S56>/abs_brake_pressure_01'
         *  RelationalOperator: '<S113>/Compare'
         */
        HEVControllerPwr_B.Merge1 =
            (HEVControllerPwr_P.abs_brake_pressure >= HEVControllerPwr_P.CompareToConstant_const);
      }

      /* Switch: '<S56>/Switch' incorporates:
       *  Constant: '<S56>/abs_brake_pressure_01'
       */
      if (HEVControllerPwr_B.Memory[0] > HEVControllerPwr_P.Switch_Threshold) {
        rtb_Merge = HEVControllerPwr_P.abs_brake_pressure;
      } else {
        rtb_Merge = HEVControllerPwr_B.Saturation;
      }

      /* End of Switch: '<S56>/Switch' */
      /* End of Outputs for SubSystem: '<S49>/pressureControl' */
      break;

    case 1: {
      boolean_T rtb_LogicalOperator2;

      /* Outputs for IfAction SubSystem: '<S49>/accControl' incorporates:
       *  ActionPort: '<S55>/Action Port'
       */
      /* Product: '<S55>/Divide' incorporates:
       *  Constant: '<S55>/Constant'
       *  Sum: '<S55>/Add'
       */
      rtb_Merge =
          (HEVControllerPwr_B.target_acc_m_s2 - HEVControllerPwr_B.brake_pressure_0_1) / HEVControllerPwr_P.ecu_max_dec;

      /* Logic: '<S58>/Logical Operator2' incorporates:
       *  Abs: '<S58>/Abs'
       *  Constant: '<S107>/Constant'
       *  Constant: '<S108>/Constant'
       *  Constant: '<S109>/Constant'
       *  Constant: '<S110>/Constant'
       *  Constant: '<S111>/Constant'
       *  Constant: '<S112>/Constant'
       *  Logic: '<S58>/Logical Operator'
       *  Logic: '<S58>/Logical Operator1'
       *  Logic: '<S58>/Logical Operator3'
       *  RelationalOperator: '<S107>/Compare'
       *  RelationalOperator: '<S108>/Compare'
       *  RelationalOperator: '<S109>/Compare'
       *  RelationalOperator: '<S110>/Compare'
       *  RelationalOperator: '<S111>/Compare'
       *  RelationalOperator: '<S112>/Compare'
       */
      rtb_LogicalOperator2 =
          (((HEVControllerPwr_B.veh_speed_vx < -HEVControllerPwr_P.speedOffset) &&
            (HEVControllerPwr_B.gear_cmd__0N1D2R3P == HEVControllerPwr_P.reverseGear_const) &&
            (HEVControllerPwr_B.target_acc_m_s2 > HEVControllerPwr_P.brakeEnableThresh_Reverse_const)) ||
           ((HEVControllerPwr_B.gear_cmd__0N1D2R3P != HEVControllerPwr_P.notReverse_const) &&
            (HEVControllerPwr_B.target_acc_m_s2 < HEVControllerPwr_P.brakeEnableThresh_const) &&
            (std::abs(HEVControllerPwr_B.veh_speed_vx) > HEVControllerPwr_P.speedOffset)));

      /* Logic: '<S55>/Logical Operator' */
      HEVControllerPwr_B.LogicalOperator_e = !rtb_LogicalOperator2;
      if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
        /* DiscreteIntegrator: '<S85>/Filter' */
        if (HEVControllerPwr_B.LogicalOperator_e || (HEVControllerPwr_DW.Filter_PrevResetState_h != 0)) {
          HEVControllerPwr_DW.Filter_DSTATE_l = HEVControllerPwr_P.DiscretePIDController_InitialConditionForFilter;
        }

        /* DiscreteIntegrator: '<S85>/Filter' */
        HEVControllerPwr_B.Filter_o = HEVControllerPwr_DW.Filter_DSTATE_l;

        /* DiscreteIntegrator: '<S90>/Integrator' */
        if (HEVControllerPwr_B.LogicalOperator_e || (HEVControllerPwr_DW.Integrator_PrevResetState_n != 0)) {
          HEVControllerPwr_DW.Integrator_DSTATE_c =
              HEVControllerPwr_P.DiscretePIDController_InitialConditionForIntegrator;
        }

        /* DiscreteIntegrator: '<S90>/Integrator' */
        HEVControllerPwr_B.Integrator_o = HEVControllerPwr_DW.Integrator_DSTATE_c;
      }

      /* Gain: '<S87>/Integral Gain' */
      HEVControllerPwr_B.IntegralGain = HEVControllerPwr_P.brake_I * rtb_Merge;

      /* Gain: '<S93>/Filter Coefficient' incorporates:
       *  Gain: '<S84>/Derivative Gain'
       *  Sum: '<S85>/SumD'
       */
      HEVControllerPwr_B.FilterCoefficient_g = (HEVControllerPwr_P.brake_D * rtb_Merge - HEVControllerPwr_B.Filter_o) *
                                               HEVControllerPwr_P.DiscretePIDController_N;

      /* Merge: '<S49>/Merge1' incorporates:
       *  SignalConversion generated from: '<S55>/brakeEnable'
       */
      HEVControllerPwr_B.Merge1 = rtb_LogicalOperator2;

      /* Switch: '<S55>/abs' incorporates:
       *  Constant: '<S55>/abs_brake_pressure_01'
       */
      if (HEVControllerPwr_B.Memory[0] > HEVControllerPwr_P.abs_Threshold) {
        rtb_Merge = HEVControllerPwr_P.abs_brake_pressure;
      } else {
        /* Sum: '<S99>/Sum' incorporates:
         *  Gain: '<S95>/Proportional Gain'
         */
        rtb_Merge = (HEVControllerPwr_P.brake_P * rtb_Merge + HEVControllerPwr_B.Integrator_o) +
                    HEVControllerPwr_B.FilterCoefficient_g;

        /* Saturate: '<S97>/Saturation' */
        if (rtb_Merge > HEVControllerPwr_P.DiscretePIDController_UpperSaturationLimit) {
          rtb_Merge = HEVControllerPwr_P.DiscretePIDController_UpperSaturationLimit;
        } else if (rtb_Merge < HEVControllerPwr_P.DiscretePIDController_LowerSaturationLimit) {
          rtb_Merge = HEVControllerPwr_P.DiscretePIDController_LowerSaturationLimit;
        }

        /* End of Saturate: '<S97>/Saturation' */
      }

      /* Product: '<S55>/Product' incorporates:
       *  Switch: '<S55>/abs'
       */
      rtb_Merge *= static_cast<real_T>(rtb_LogicalOperator2);

      /* End of Outputs for SubSystem: '<S49>/accControl' */
    } break;
  }

  /* End of If: '<S49>/If' */

  /* Saturate: '<S49>/Saturation' */
  if (rtb_Merge > HEVControllerPwr_P.Saturation_UpperSat) {
    /* Switch: '<S4>/Switch1' */
    HEVControllerPwr_B.brake_pressure_0_1 = HEVControllerPwr_P.Saturation_UpperSat;
  } else if (rtb_Merge < HEVControllerPwr_P.Saturation_LowerSat) {
    /* Switch: '<S4>/Switch1' */
    HEVControllerPwr_B.brake_pressure_0_1 = HEVControllerPwr_P.Saturation_LowerSat;
  } else {
    /* Switch: '<S4>/Switch1' */
    HEVControllerPwr_B.brake_pressure_0_1 = rtb_Merge;
  }

  /* End of Saturate: '<S49>/Saturation' */
}

/* Update for atomic system: '<S4>/ABS_EBD' */
void HEVControllerPwr::HEVControllerPwr_ABS_EBD_Update() {
  /* Abs: '<S49>/Abs' */
  HEVControllerPwr_B.Abs = std::abs(HEVControllerPwr_B.slip_ratio[0]);

  /* Relay: '<S49>/OnAtRef_OffAt0.5Ref' */
  if (rtsiIsModeUpdateTimeStep(&(&HEVControllerPwr_M)->solverInfo)) {
    HEVControllerPwr_DW.OnAtRef_OffAt05Ref_Mode =
        ((HEVControllerPwr_B.Abs >= HEVControllerPwr_P.slip_ratio_ref) ||
         ((!(HEVControllerPwr_B.Abs <= HEVControllerPwr_P.slip_disable_ratio_ref)) &&
          HEVControllerPwr_DW.OnAtRef_OffAt05Ref_Mode));
  }

  if (HEVControllerPwr_DW.OnAtRef_OffAt05Ref_Mode) {
    /* Relay: '<S49>/OnAtRef_OffAt0.5Ref' */
    HEVControllerPwr_B.OnAtRef_OffAt05Ref = HEVControllerPwr_P.OnAtRef_OffAt05Ref_YOn;
  } else {
    /* Relay: '<S49>/OnAtRef_OffAt0.5Ref' */
    HEVControllerPwr_B.OnAtRef_OffAt05Ref = HEVControllerPwr_P.OnAtRef_OffAt05Ref_YOff;
  }

  /* End of Relay: '<S49>/OnAtRef_OffAt0.5Ref' */
  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* Logic: '<S49>/if_on' incorporates:
     *  Constant: '<S49>/abs_switch'
     */
    HEVControllerPwr_B.abs_on =
        ((HEVControllerPwr_B.OnAtRef_OffAt05Ref != 0.0) && (HEVControllerPwr_P.enable_abs != 0.0));

    /* Product: '<S49>/ABS_flag' incorporates:
     *  Constant: '<S49>/Constant1'
     */
    HEVControllerPwr_B.ABS_flag[0] =
        static_cast<real_T>(HEVControllerPwr_B.abs_on) * HEVControllerPwr_P.Constant1_Value[0];
    HEVControllerPwr_B.ABS_flag[1] =
        static_cast<real_T>(HEVControllerPwr_B.abs_on) * HEVControllerPwr_P.Constant1_Value[1];
    HEVControllerPwr_B.ABS_flag[2] =
        static_cast<real_T>(HEVControllerPwr_B.abs_on) * HEVControllerPwr_P.Constant1_Value[2];
    HEVControllerPwr_B.ABS_flag[3] =
        static_cast<real_T>(HEVControllerPwr_B.abs_on) * HEVControllerPwr_P.Constant1_Value[3];
  }

  /* Product: '<S49>/Product1' */
  HEVControllerPwr_B.Product1[0] = static_cast<real_T>(HEVControllerPwr_B.Merge1) * HEVControllerPwr_B.ABS_flag[0];
  HEVControllerPwr_B.Product1[1] = static_cast<real_T>(HEVControllerPwr_B.Merge1) * HEVControllerPwr_B.ABS_flag[1];
  HEVControllerPwr_B.Product1[2] = static_cast<real_T>(HEVControllerPwr_B.Merge1) * HEVControllerPwr_B.ABS_flag[2];
  HEVControllerPwr_B.Product1[3] = static_cast<real_T>(HEVControllerPwr_B.Merge1) * HEVControllerPwr_B.ABS_flag[3];
  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* Update for Memory: '<S49>/Memory' */
    HEVControllerPwr_DW.Memory_PreviousInput[0] = HEVControllerPwr_B.Product1[0];
    HEVControllerPwr_DW.Memory_PreviousInput[1] = HEVControllerPwr_B.Product1[1];
    HEVControllerPwr_DW.Memory_PreviousInput[2] = HEVControllerPwr_B.Product1[2];
    HEVControllerPwr_DW.Memory_PreviousInput[3] = HEVControllerPwr_B.Product1[3];
  }

  /* Update for If: '<S49>/If' */
  if (HEVControllerPwr_DW.If_ActiveSubsystem_m == 1) {
    /* Update for IfAction SubSystem: '<S49>/accControl' incorporates:
     *  ActionPort: '<S55>/Action Port'
     */
    if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
      /* Update for DiscreteIntegrator: '<S85>/Filter' */
      HEVControllerPwr_DW.Filter_DSTATE_l += HEVControllerPwr_P.Filter_gainval * HEVControllerPwr_B.FilterCoefficient_g;
      HEVControllerPwr_DW.Filter_PrevResetState_h = static_cast<int8_T>(HEVControllerPwr_B.LogicalOperator_e);

      /* Update for DiscreteIntegrator: '<S90>/Integrator' incorporates:
       *  DiscreteIntegrator: '<S85>/Filter'
       */
      HEVControllerPwr_DW.Integrator_DSTATE_c +=
          HEVControllerPwr_P.Integrator_gainval * HEVControllerPwr_B.IntegralGain;
      HEVControllerPwr_DW.Integrator_PrevResetState_n = static_cast<int8_T>(HEVControllerPwr_B.LogicalOperator_e);
    }

    /* End of Update for SubSystem: '<S49>/accControl' */
  }

  /* End of Update for If: '<S49>/If' */
}

/* System initialize for atomic system: '<S4>/HCU' */
void HEVControllerPwr::HEVControllerPwr_HCU_Init() {
  /* Start for If: '<S51>/If' */
  HEVControllerPwr_DW.If_ActiveSubsystem = -1;

  /* SystemInitialize for IfAction SubSystem: '<S51>/useTargetAccMode' */
  /* InitializeConditions for DiscreteIntegrator: '<S166>/Integrator' */
  HEVControllerPwr_DW.Integrator_DSTATE = HEVControllerPwr_P.DiscretePIDController_InitialConditionForIntegrator_e;

  /* InitializeConditions for DiscreteIntegrator: '<S161>/Filter' */
  HEVControllerPwr_DW.Filter_DSTATE = HEVControllerPwr_P.DiscretePIDController_InitialConditionForFilter_p;

  /* End of SystemInitialize for SubSystem: '<S51>/useTargetAccMode' */
}

/* Outputs for atomic system: '<S4>/HCU' */
void HEVControllerPwr::HEVControllerPwr_HCU() {
  real_T rtb_Product2;
  real_T rtb_Switch_e;
  int8_T rtAction;
  boolean_T rtb_LogicalOperator_k;
  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* RelationalOperator: '<S122>/Compare' incorporates:
     *  Constant: '<S122>/Constant'
     */
    HEVControllerPwr_B.Compare = (HEVControllerPwr_B.Memory[0] >= HEVControllerPwr_P.CompareToConstant_const_p);
  }

  /* MultiPortSwitch: '<S117>/Multiport Switch' incorporates:
   *  Constant: '<S117>/Zero'
   *  Constant: '<S124>/Constant'
   *  Constant: '<S125>/Constant'
   *  DataTypeConversion: '<S117>/Data Type Conversion'
   *  RelationalOperator: '<S124>/Compare'
   *  RelationalOperator: '<S125>/Compare'
   */
  switch (HEVControllerPwr_B.gear_cmd__0N1D2R3P) {
    case 0:
      rtb_Switch_e = HEVControllerPwr_P.Zero_Value;
      break;

    case 1:
      rtb_Switch_e = (HEVControllerPwr_B.veh_speed_vx < -HEVControllerPwr_P.speedOffset);
      break;

    case 2:
      rtb_Switch_e = (HEVControllerPwr_B.veh_speed_vx > HEVControllerPwr_P.speedOffset);
      break;

    default:
      rtb_Switch_e = HEVControllerPwr_P.Zero_Value;
      break;
  }

  /* End of MultiPortSwitch: '<S117>/Multiport Switch' */

  /* Logic: '<S117>/Logical Operator' incorporates:
   *  Constant: '<S123>/Constant'
   *  RelationalOperator: '<S123>/Compare'
   */
  rtb_LogicalOperator_k = ((HEVControllerPwr_B.brake_pressure_0_1 >= HEVControllerPwr_P.CompareToConstant1_const_k) ||
                           HEVControllerPwr_B.Compare || (rtb_Switch_e != 0.0));

  /* Switch: '<S119>/Switch1' incorporates:
   *  Constant: '<S119>/fliper'
   *  Constant: '<S126>/Constant'
   *  Constant: '<S127>/Constant'
   *  RelationalOperator: '<S126>/Compare'
   *  RelationalOperator: '<S127>/Compare'
   */
  if (HEVControllerPwr_B.gear_cmd__0N1D2R3P == HEVControllerPwr_P.reverseGear_const_n) {
    rtb_Switch_e = HEVControllerPwr_P.fliper_Value;
  } else {
    rtb_Switch_e = (HEVControllerPwr_B.gear_cmd__0N1D2R3P == HEVControllerPwr_P.forwardGear_const);
  }

  /* End of Switch: '<S119>/Switch1' */

  /* Product: '<S51>/Product2' */
  rtb_Product2 = HEVControllerPwr_B.target_acc_m_s2 * rtb_Switch_e;

  /* If: '<S51>/If' */
  if (rtsiIsModeUpdateTimeStep(&(&HEVControllerPwr_M)->solverInfo)) {
    rtAction = static_cast<int8_T>(HEVControllerPwr_B.acc_control_mode != 0);
    HEVControllerPwr_DW.If_ActiveSubsystem = rtAction;
  } else {
    rtAction = HEVControllerPwr_DW.If_ActiveSubsystem;
  }

  switch (rtAction) {
    case 0: {
      real_T rtb_DeadZone;
      real_T rtb_IntegralGain;
      int8_T tmp;

      /* Outputs for IfAction SubSystem: '<S51>/useTargetAccMode' incorporates:
       *  ActionPort: '<S121>/Action Port'
       */
      /* Sum: '<S121>/acc_err' incorporates:
       *  Product: '<S51>/Product1'
       */
      rtb_IntegralGain = rtb_Product2 - HEVControllerPwr_B.ax_estimator.Integrator1 * rtb_Switch_e;

      /* DeadZone: '<S121>/TargetAccDeadZone' */
      if (rtb_IntegralGain > HEVControllerPwr_P.TargetAccDeadZone_End) {
        rtb_IntegralGain -= HEVControllerPwr_P.TargetAccDeadZone_End;
      } else if (rtb_IntegralGain >= HEVControllerPwr_P.TargetAccDeadZone_Start) {
        rtb_IntegralGain = 0.0;
      } else {
        rtb_IntegralGain -= HEVControllerPwr_P.TargetAccDeadZone_Start;
      }

      /* Product: '<S121>/Divide' incorporates:
       *  Constant: '<S121>/Constant'
       *  DeadZone: '<S121>/TargetAccDeadZone'
       */
      rtb_IntegralGain /= HEVControllerPwr_P.ecu_max_acc;

      /* Logic: '<S121>/Logical Operator1' incorporates:
       *  Clock: '<S121>/Clock'
       *  Constant: '<S130>/Constant'
       *  RelationalOperator: '<S130>/Compare'
       */
      HEVControllerPwr_B.LogicalOperator1 =
          (rtb_LogicalOperator_k || HEVControllerPwr_B.LogicalOperator ||
           ((&HEVControllerPwr_M)->Timing.t[0] < HEVControllerPwr_P.CompareToConstant1_const));
      if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
        /* DiscreteIntegrator: '<S166>/Integrator' */
        if (HEVControllerPwr_B.LogicalOperator1 || (HEVControllerPwr_DW.Integrator_PrevResetState != 0)) {
          HEVControllerPwr_DW.Integrator_DSTATE =
              HEVControllerPwr_P.DiscretePIDController_InitialConditionForIntegrator_e;
        }

        /* DiscreteIntegrator: '<S166>/Integrator' */
        HEVControllerPwr_B.Integrator = HEVControllerPwr_DW.Integrator_DSTATE;

        /* DiscreteIntegrator: '<S161>/Filter' */
        if (HEVControllerPwr_B.LogicalOperator1 || (HEVControllerPwr_DW.Filter_PrevResetState != 0)) {
          HEVControllerPwr_DW.Filter_DSTATE = HEVControllerPwr_P.DiscretePIDController_InitialConditionForFilter_p;
        }

        /* DiscreteIntegrator: '<S161>/Filter' */
        HEVControllerPwr_B.Filter = HEVControllerPwr_DW.Filter_DSTATE;
      }

      /* Gain: '<S169>/Filter Coefficient' incorporates:
       *  Gain: '<S160>/Derivative Gain'
       *  Sum: '<S161>/SumD'
       */
      HEVControllerPwr_B.FilterCoefficient =
          (HEVControllerPwr_P.hcu_acc_D * rtb_IntegralGain - HEVControllerPwr_B.Filter) *
          HEVControllerPwr_P.DiscretePIDController_N_m;

      /* Sum: '<S175>/Sum' incorporates:
       *  Gain: '<S171>/Proportional Gain'
       */
      rtb_Switch_e = (HEVControllerPwr_P.hcu_acc_P * rtb_IntegralGain + HEVControllerPwr_B.Integrator) +
                     HEVControllerPwr_B.FilterCoefficient;

      /* DeadZone: '<S159>/DeadZone' */
      if (rtb_Switch_e > HEVControllerPwr_P.DiscretePIDController_UpperSaturationLimit_p) {
        rtb_DeadZone = rtb_Switch_e - HEVControllerPwr_P.DiscretePIDController_UpperSaturationLimit_p;
      } else if (rtb_Switch_e >= HEVControllerPwr_P.DiscretePIDController_LowerSaturationLimit_n) {
        rtb_DeadZone = 0.0;
      } else {
        rtb_DeadZone = rtb_Switch_e - HEVControllerPwr_P.DiscretePIDController_LowerSaturationLimit_n;
      }

      /* End of DeadZone: '<S159>/DeadZone' */

      /* Gain: '<S163>/Integral Gain' */
      rtb_IntegralGain *= HEVControllerPwr_P.hcu_acc_I;

      /* Switch: '<S157>/Switch1' incorporates:
       *  Constant: '<S157>/Clamping_zero'
       *  Constant: '<S157>/Constant'
       *  Constant: '<S157>/Constant2'
       *  RelationalOperator: '<S157>/fix for DT propagation issue'
       */
      if (rtb_DeadZone > HEVControllerPwr_P.Clamping_zero_Value) {
        rtAction = HEVControllerPwr_P.Constant_Value_e;
      } else {
        rtAction = HEVControllerPwr_P.Constant2_Value_p;
      }

      /* Switch: '<S157>/Switch2' incorporates:
       *  Constant: '<S157>/Clamping_zero'
       *  Constant: '<S157>/Constant3'
       *  Constant: '<S157>/Constant4'
       *  RelationalOperator: '<S157>/fix for DT propagation issue1'
       */
      if (rtb_IntegralGain > HEVControllerPwr_P.Clamping_zero_Value) {
        tmp = HEVControllerPwr_P.Constant3_Value;
      } else {
        tmp = HEVControllerPwr_P.Constant4_Value;
      }

      /* Switch: '<S157>/Switch' incorporates:
       *  Constant: '<S157>/Clamping_zero'
       *  Logic: '<S157>/AND3'
       *  RelationalOperator: '<S157>/Equal1'
       *  RelationalOperator: '<S157>/Relational Operator'
       *  Switch: '<S157>/Switch1'
       *  Switch: '<S157>/Switch2'
       */
      if ((HEVControllerPwr_P.Clamping_zero_Value != rtb_DeadZone) && (rtAction == tmp)) {
        /* Switch: '<S157>/Switch' incorporates:
         *  Constant: '<S157>/Constant1'
         */
        HEVControllerPwr_B.Switch = HEVControllerPwr_P.Constant1_Value_h;
      } else {
        /* Switch: '<S157>/Switch' */
        HEVControllerPwr_B.Switch = rtb_IntegralGain;
      }

      /* End of Switch: '<S157>/Switch' */

      /* Switch: '<S121>/Switch' incorporates:
       *  Constant: '<S121>/Constant2'
       *  Constant: '<S129>/Constant'
       *  RelationalOperator: '<S129>/Compare'
       *  Saturate: '<S121>/Saturation'
       *  Saturate: '<S173>/Saturation'
       *  Sum: '<S121>/Throttle'
       *  Switch: '<S121>/Switch1'
       */
      if (HEVControllerPwr_B.LogicalOperator1) {
        rtb_Switch_e = HEVControllerPwr_P.Constant2_Value;
      } else {
        if (rtb_Product2 > HEVControllerPwr_P.Accel_const) {
          /* Switch: '<S183>/Switch' incorporates:
           *  Abs: '<S183>/Abs'
           *  Constant: '<S183>/ZeroPwr'
           *  Constant: '<S185>/Constant'
           *  Gain: '<S183>/InertiaForce'
           *  Product: '<S183>/AccelPwr'
           *  RelationalOperator: '<S185>/Compare'
           *  Switch: '<S121>/Switch1'
           */
          if (rtb_Product2 > HEVControllerPwr_P.CompareToConstant_const_m) {
            rtb_IntegralGain = HEVControllerPwr_P.VEH.Mass * rtb_Product2 * std::abs(HEVControllerPwr_B.veh_speed_vx);
          } else {
            rtb_IntegralGain = HEVControllerPwr_P.ZeroPwr_Value;
          }

          /* Switch: '<S121>/Switch1' incorporates:
           *  Abs: '<S184>/Abs'
           *  Constant: '<S184>/RollingCoeff'
           *  Gain: '<S184>/AeroDragForce'
           *  Gain: '<S184>/RollingResistanceForce'
           *  Math: '<S184>/Square'
           *  Product: '<S184>/ResistancePwr'
           *  Sum: '<S132>/DrivingResistancePwrCal'
           *  Sum: '<S184>/ResistanceForce'
           *  Switch: '<S183>/Switch'
           */
          rtb_Product2 = (0.5 * HEVControllerPwr_P.VEH.DragCoefficient * HEVControllerPwr_P.SoftHCU_AirDensity *
                              HEVControllerPwr_P.VEH.FrontalArea *
                              (HEVControllerPwr_B.veh_speed_vx * HEVControllerPwr_B.veh_speed_vx) +
                          HEVControllerPwr_P.VEH.Mass * 9.806 * HEVControllerPwr_P.SoftHCU_RollingCoeff) *
                             std::abs(HEVControllerPwr_B.veh_speed_vx) +
                         rtb_IntegralGain;
        } else {
          /* Switch: '<S121>/Switch1' incorporates:
           *  Constant: '<S121>/ZeroPwr'
           */
          rtb_Product2 = HEVControllerPwr_P.ZeroPwr_Value_a;
        }

        /* Product: '<S121>/Divide1' incorporates:
         *  Constant: '<S121>/MaxPwrOfSystem'
         */
        rtb_Product2 /= HEVControllerPwr_P.SoftHCU_MaxPwrOfSystem;

        /* Saturate: '<S173>/Saturation' */
        if (rtb_Switch_e > HEVControllerPwr_P.DiscretePIDController_UpperSaturationLimit_p) {
          rtb_Switch_e = HEVControllerPwr_P.DiscretePIDController_UpperSaturationLimit_p;
        } else if (rtb_Switch_e < HEVControllerPwr_P.DiscretePIDController_LowerSaturationLimit_n) {
          rtb_Switch_e = HEVControllerPwr_P.DiscretePIDController_LowerSaturationLimit_n;
        }

        /* Saturate: '<S121>/Saturation' */
        if (rtb_Product2 > HEVControllerPwr_P.Saturation_UpperSat_a) {
          rtb_Product2 = HEVControllerPwr_P.Saturation_UpperSat_a;
        } else if (rtb_Product2 < HEVControllerPwr_P.Saturation_LowerSat_n) {
          rtb_Product2 = HEVControllerPwr_P.Saturation_LowerSat_n;
        }

        rtb_Switch_e += rtb_Product2;
      }

      /* End of Switch: '<S121>/Switch' */
      /* End of Outputs for SubSystem: '<S51>/useTargetAccMode' */
    } break;

    case 1:
      /* Outputs for IfAction SubSystem: '<S51>/throttleControlMode' incorporates:
       *  ActionPort: '<S120>/Action Port'
       */
      /* Product: '<S120>/Product' incorporates:
       *  Clock: '<S120>/Clock'
       *  Constant: '<S128>/Constant'
       *  RelationalOperator: '<S128>/Compare'
       */
      rtb_Switch_e =
          static_cast<real_T>((&HEVControllerPwr_M)->Timing.t[0] >= HEVControllerPwr_P.CompareToConstant1_const_l) *
          HEVControllerPwr_B.Saturation;

      /* Saturate: '<S120>/Saturation' */
      if (rtb_Switch_e > HEVControllerPwr_P.Saturation_UpperSat_g) {
        rtb_Switch_e = HEVControllerPwr_P.Saturation_UpperSat_g;
      } else if (rtb_Switch_e < HEVControllerPwr_P.Saturation_LowerSat_c) {
        rtb_Switch_e = HEVControllerPwr_P.Saturation_LowerSat_c;
      }

      /* End of Saturate: '<S120>/Saturation' */
      /* End of Outputs for SubSystem: '<S51>/throttleControlMode' */
      break;
  }

  /* End of If: '<S51>/If' */

  /* Switch: '<S51>/Switch' incorporates:
   *  Constant: '<S118>/Constant'
   *  Constant: '<S51>/no_throttle'
   *  Logic: '<S51>/Logical Operator'
   *  RelationalOperator: '<S118>/Compare'
   */
  if (rtb_LogicalOperator_k && (rtb_Switch_e > HEVControllerPwr_P.CompareToConstant_const_h)) {
    rtb_Switch_e = HEVControllerPwr_P.no_throttle_Value;
  }

  /* Saturate: '<S51>/Saturation' incorporates:
   *  Switch: '<S51>/Switch'
   */
  if (rtb_Switch_e > HEVControllerPwr_P.Saturation_UpperSat_o) {
    HEVControllerPwr_B.Saturation = HEVControllerPwr_P.Saturation_UpperSat_o;
  } else if (rtb_Switch_e < HEVControllerPwr_P.Saturation_LowerSat_p) {
    HEVControllerPwr_B.Saturation = HEVControllerPwr_P.Saturation_LowerSat_p;
  } else {
    HEVControllerPwr_B.Saturation = rtb_Switch_e;
  }

  /* End of Saturate: '<S51>/Saturation' */
}

/* Update for atomic system: '<S4>/HCU' */
void HEVControllerPwr::HEVControllerPwr_HCU_Update() {
  /* Update for If: '<S51>/If' */
  if (HEVControllerPwr_DW.If_ActiveSubsystem == 0) {
    /* Update for IfAction SubSystem: '<S51>/useTargetAccMode' incorporates:
     *  ActionPort: '<S121>/Action Port'
     */
    if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
      /* Update for DiscreteIntegrator: '<S166>/Integrator' */
      HEVControllerPwr_DW.Integrator_DSTATE += HEVControllerPwr_P.Integrator_gainval_g * HEVControllerPwr_B.Switch;
      HEVControllerPwr_DW.Integrator_PrevResetState = static_cast<int8_T>(HEVControllerPwr_B.LogicalOperator1);

      /* Update for DiscreteIntegrator: '<S161>/Filter' incorporates:
       *  DiscreteIntegrator: '<S166>/Integrator'
       */
      HEVControllerPwr_DW.Filter_DSTATE += HEVControllerPwr_P.Filter_gainval_j * HEVControllerPwr_B.FilterCoefficient;
      HEVControllerPwr_DW.Filter_PrevResetState = static_cast<int8_T>(HEVControllerPwr_B.LogicalOperator1);
    }

    /* End of Update for SubSystem: '<S51>/useTargetAccMode' */
  }

  /* End of Update for If: '<S51>/If' */
}

/* System initialize for atomic system: '<S4>/ax_estimator' */
void HEVControllerPwr::HEVControllerPwr_ax_estimator_Init(DW_ax_estimator_HEVControllerPwr_T *localDW,
                                                          P_ax_estimator_HEVControllerPwr_T *localP,
                                                          X_ax_estimator_HEVControllerPwr_T *localX) {
  /* Start for If: '<S53>/If' */
  localDW->If_ActiveSubsystem = -1;

  /* InitializeConditions for Integrator: '<S186>/Integrator1' */
  localX->Integrator1_CSTATE = localP->Integrator1_IC;

  /* SystemInitialize for IfAction SubSystem: '<S53>/AxCal' */
  /* InitializeConditions for Delay: '<S187>/Delay' */
  for (int32_T i{0}; i < 100; i++) {
    localDW->Delay_DSTATE[i] = localP->Delay_InitialCondition;
  }

  /* End of InitializeConditions for Delay: '<S187>/Delay' */
  /* End of SystemInitialize for SubSystem: '<S53>/AxCal' */
}

/* Outputs for atomic system: '<S4>/ax_estimator' */
void HEVControllerPwr::HEVControllerPwr_ax_estimator(real_T rtu_acc_feedback_m_s2, real_T rtu_vx,
                                                     B_ax_estimator_HEVControllerPwr_T *localB,
                                                     DW_ax_estimator_HEVControllerPwr_T *localDW,
                                                     P_ax_estimator_HEVControllerPwr_T *localP,
                                                     P_HEVControllerPwr_T *HEVControllerPwr_P,
                                                     X_ax_estimator_HEVControllerPwr_T *localX) {
  real_T rtb_Merge_h;
  int8_T rtAction;

  /* Abs: '<S53>/Abs1' */
  rtb_Merge_h = std::abs(rtu_vx);

  /* If: '<S53>/If' incorporates:
   *  Constant: '<S189>/Constant'
   *  RelationalOperator: '<S189>/Compare'
   */
  if (rtsiIsModeUpdateTimeStep(&(&HEVControllerPwr_M)->solverInfo)) {
    rtAction = static_cast<int8_T>(!(rtb_Merge_h >= localP->SpeedThresh_const));
    localDW->If_ActiveSubsystem = rtAction;
  } else {
    rtAction = localDW->If_ActiveSubsystem;
  }

  switch (rtAction) {
    case 0:
      /* Outputs for IfAction SubSystem: '<S53>/If Action Subsystem' incorporates:
       *  ActionPort: '<S188>/Action Port'
       */
      /* SignalConversion generated from: '<S188>/<acc_feedback_m_s2>' */
      rtb_Merge_h = rtu_acc_feedback_m_s2;

      /* End of Outputs for SubSystem: '<S53>/If Action Subsystem' */
      break;

    case 1: {
      /* Outputs for IfAction SubSystem: '<S53>/AxCal' incorporates:
       *  ActionPort: '<S187>/Action Port'
       */
      if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
        /* Delay: '<S187>/Delay' incorporates:
         *  Constant: '<S187>/AxDelayStep'
         */
        if ((HEVControllerPwr_P->softecu_ax_delay < 1.0) || std::isnan(HEVControllerPwr_P->softecu_ax_delay)) {
          /* Delay: '<S187>/Delay' */
          localB->Delay = rtu_vx;
        } else {
          uint32_T tmp;
          if (HEVControllerPwr_P->softecu_ax_delay > 100.0) {
            tmp = 100U;
          } else {
            rtb_Merge_h = std::trunc(HEVControllerPwr_P->softecu_ax_delay);
            if (std::isnan(rtb_Merge_h) || std::isinf(rtb_Merge_h)) {
              rtb_Merge_h = 0.0;
            } else {
              rtb_Merge_h = std::fmod(rtb_Merge_h, 4.294967296E+9);
            }

            tmp = rtb_Merge_h < 0.0 ? static_cast<uint32_T>(-static_cast<int32_T>(static_cast<uint32_T>(-rtb_Merge_h)))
                                    : static_cast<uint32_T>(rtb_Merge_h);
          }

          /* Delay: '<S187>/Delay' */
          localB->Delay = localDW->Delay_DSTATE[100U - tmp];
        }

        /* End of Delay: '<S187>/Delay' */
      }

      /* Product: '<S187>/Divide' incorporates:
       *  Constant: '<S187>/AxDelayStep'
       *  Gain: '<S187>/Gain'
       *  Sum: '<S187>/Subtract'
       */
      rtb_Merge_h = (rtu_vx - localB->Delay) / (HEVControllerPwr_P->stepTime * HEVControllerPwr_P->softecu_ax_delay);

      /* End of Outputs for SubSystem: '<S53>/AxCal' */
    } break;
  }

  /* End of If: '<S53>/If' */

  /* Integrator: '<S186>/Integrator1' */
  localB->Integrator1 = localX->Integrator1_CSTATE;

  /* Product: '<S186>/Divide' incorporates:
   *  Constant: '<S186>/Constant'
   *  Sum: '<S186>/Sum'
   */
  localB->Divide = (rtb_Merge_h - localB->Integrator1) * localP->AxAct_wc;
}

/* Update for atomic system: '<S4>/ax_estimator' */
void HEVControllerPwr::HEVControllerPwr_ax_estimator_Update(real_T rtu_vx,
                                                            DW_ax_estimator_HEVControllerPwr_T *localDW) {
  /* Update for If: '<S53>/If' */
  if (localDW->If_ActiveSubsystem == 1) {
    /* Update for IfAction SubSystem: '<S53>/AxCal' incorporates:
     *  ActionPort: '<S187>/Action Port'
     */
    if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
      /* Update for Delay: '<S187>/Delay' */
      for (int_T idxDelay{0}; idxDelay < 99; idxDelay++) {
        localDW->Delay_DSTATE[idxDelay] = localDW->Delay_DSTATE[idxDelay + 1];
      }

      localDW->Delay_DSTATE[99] = rtu_vx;

      /* End of Update for Delay: '<S187>/Delay' */
    }

    /* End of Update for SubSystem: '<S53>/AxCal' */
  }

  /* End of Update for If: '<S53>/If' */
}

/* Derivatives for atomic system: '<S4>/ax_estimator' */
void HEVControllerPwr::HEVControllerPwr_ax_estimator_Deriv(B_ax_estimator_HEVControllerPwr_T *localB,
                                                           XDot_ax_estimator_HEVControllerPwr_T *localXdot) {
  /* Derivatives for Integrator: '<S186>/Integrator1' */
  localXdot->Integrator1_CSTATE = localB->Divide;
}

/* Function for Chart: '<S2>/HEV_DriveMode' */
real_T HEVControllerPwr::HEVControllerPwr_from_hev_to_engine(real_T hevmode_in_SoC_MinVehicleDrive,
                                                             real_T hevmode_in_Speed_EngineStepIn,
                                                             real_T hevmode_in_PwrDmnd_EVOnly,
                                                             real_T hevmode_in_PwrDmnd_Hybrid,
                                                             real_T hevmode_in_SoC_Battery, real_T hevmode_in_PwrDmnd,
                                                             real_T hevmode_in_Vehicle_Speed) {
  real_T if_switch;
  if_switch = 0.0;
  if (hevmode_in_SoC_Battery < hevmode_in_SoC_MinVehicleDrive) {
    if_switch = 1.0;
  } else {
    if ((hevmode_in_PwrDmnd < hevmode_in_PwrDmnd_EVOnly) &&
        (std::abs(hevmode_in_Vehicle_Speed) >= hevmode_in_Speed_EngineStepIn)) {
      if_switch = 1.0;
    }

    if ((hevmode_in_PwrDmnd >= hevmode_in_PwrDmnd_EVOnly) && (hevmode_in_PwrDmnd < hevmode_in_PwrDmnd_Hybrid)) {
      if_switch = 1.0;
    }
  }

  return if_switch;
}

/* Function for Chart: '<S2>/HEV_DriveMode' */
real_T HEVControllerPwr::HEVControllerPwr_from_hev_to_ev(real_T hevmode_in_SoC_MinVehicleDrive,
                                                         real_T hevmode_in_Speed_EngineStepIn,
                                                         real_T hevmode_in_PwrDmnd_EVOnly,
                                                         real_T hevmode_in_SoC_Battery, real_T hevmode_in_PwrDmnd,
                                                         real_T hevmode_in_Vehicle_Speed) {
  real_T if_switch;
  if_switch = 0.0;
  if ((!(hevmode_in_SoC_Battery < hevmode_in_SoC_MinVehicleDrive)) &&
      (hevmode_in_PwrDmnd < hevmode_in_PwrDmnd_EVOnly) &&
      (std::abs(hevmode_in_Vehicle_Speed) < hevmode_in_Speed_EngineStepIn)) {
    if_switch = 1.0;
  }

  return if_switch;
}

/* Function for Chart: '<S2>/HEV_DriveMode' */
void HEVControllerPwr::HEVControllerPwr_charging_decision(real_T hevmode_in_SoC_StartCharging,
                                                          real_T hevmode_in_SoC_Battery, HEVMode_Output *hevmode_out) {
  if (hevmode_in_SoC_Battery < hevmode_in_SoC_StartCharging) {
    hevmode_out->Engine_ChargingSwitch = 1.0;
  } else if (hevmode_in_SoC_Battery >= hevmode_in_SoC_StartCharging * 1.1) {
    hevmode_out->Engine_ChargingSwitch = 0.0;
  }
}

/* Function for Chart: '<S2>/HEV_DriveMode' */
void HEVControllerPwr::HEVControllerPwr_serialize_drive_mode(real_T hevmode_in_SoC_StartCharging,
                                                             real_T hevmode_in_SoC_Battery,
                                                             HEVMode_Output *hevmode_out) {
  if (hevmode_in_SoC_Battery < hevmode_in_SoC_StartCharging) {
    hevmode_out->Engine_ChargingSwitch = 1.0;
    hevmode_out->Engine_ClutchSwitch = 0.0;
    hevmode_out->Engine_Switch = 1.0;
    hevmode_out->HEV_Mode = 4U;
  } else if (hevmode_in_SoC_Battery >= hevmode_in_SoC_StartCharging * 1.1) {
    hevmode_out->Engine_ChargingSwitch = 0.0;
    hevmode_out->Engine_ClutchSwitch = 0.0;
    hevmode_out->Engine_Switch = 0.0;
    hevmode_out->HEV_Mode = 0U;
  }
}

/* Function for Chart: '<S2>/HEV_DriveMode' */
real_T HEVControllerPwr::HEVControllerPwr_from_ev_to_engine(real_T hevmode_in_SoC_MinVehicleDrive,
                                                            real_T hevmode_in_Speed_EngineStepIn,
                                                            real_T hevmode_in_PwrDmnd_EVOnly,
                                                            real_T hevmode_in_PwrDmnd_Hybrid,
                                                            real_T hevmode_in_SoC_Battery, real_T hevmode_in_PwrDmnd,
                                                            real_T hevmode_in_Vehicle_Speed) {
  real_T if_switch;
  if_switch = 0.0;
  if (hevmode_in_SoC_Battery < hevmode_in_SoC_MinVehicleDrive) {
    if_switch = 1.0;
  } else if ((hevmode_in_PwrDmnd >= hevmode_in_PwrDmnd_EVOnly * 1.1) &&
             (hevmode_in_PwrDmnd < hevmode_in_PwrDmnd_Hybrid) &&
             (std::abs(hevmode_in_Vehicle_Speed) >= hevmode_in_Speed_EngineStepIn * 1.1)) {
    if_switch = 1.0;
  }

  return if_switch;
}

/* Function for Chart: '<S2>/HEV_DriveMode' */
real_T HEVControllerPwr::HEVControllerPwr_from_engine_to_hev(real_T hevmode_in_SoC_MinVehicleDrive,
                                                             real_T hevmode_in_PwrDmnd_Hybrid,
                                                             real_T hevmode_in_SoC_Battery, real_T hevmode_in_PwrDmnd) {
  real_T if_switch;
  if_switch = 0.0;
  if ((!(hevmode_in_SoC_Battery < hevmode_in_SoC_MinVehicleDrive * 1.2)) &&
      (hevmode_in_PwrDmnd >= hevmode_in_PwrDmnd_Hybrid)) {
    if_switch = 1.0;
  }

  return if_switch;
}

/* Function for Chart: '<S2>/HEV_DriveMode' */
real_T HEVControllerPwr::HEVControllerPwr_from_engine_to_ev(real_T hevmode_in_SoC_MinVehicleDrive,
                                                            real_T hevmode_in_Speed_EngineStepIn,
                                                            real_T hevmode_in_PwrDmnd_EVOnly,
                                                            real_T hevmode_in_SoC_Battery, real_T hevmode_in_PwrDmnd,
                                                            real_T hevmode_in_Vehicle_Speed) {
  real_T if_switch;
  if_switch = 0.0;
  if ((!(hevmode_in_SoC_Battery < hevmode_in_SoC_MinVehicleDrive * 1.2)) &&
      (hevmode_in_PwrDmnd < hevmode_in_PwrDmnd_EVOnly * 0.9) &&
      (std::abs(hevmode_in_Vehicle_Speed) < hevmode_in_Speed_EngineStepIn * 0.9)) {
    if_switch = 1.0;
  }

  return if_switch;
}

/* Model step function */
void HEVControllerPwr::step() {
  HEVMode_Output rtb_HEVMode_Router;
  real_T rtb_Gain_p;
  real_T rtb_autoHoldSwitch;
  real_T u0;
  boolean_T accMode;
  boolean_T gearDrive;
  boolean_T gearReverse;
  boolean_T rtb_Compare_li;
  boolean_T torqueMode;
  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* set solver stop time */
    if (!((&HEVControllerPwr_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(
          &(&HEVControllerPwr_M)->solverInfo,
          (((&HEVControllerPwr_M)->Timing.clockTickH0 + 1) * (&HEVControllerPwr_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(
          &(&HEVControllerPwr_M)->solverInfo,
          (((&HEVControllerPwr_M)->Timing.clockTick0 + 1) * (&HEVControllerPwr_M)->Timing.stepSize0 +
           (&HEVControllerPwr_M)->Timing.clockTickH0 * (&HEVControllerPwr_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&HEVControllerPwr_M))) {
    (&HEVControllerPwr_M)->Timing.t[0] = rtsiGetT(&(&HEVControllerPwr_M)->solverInfo);
  }

  /* SignalConversion generated from: '<Root>/SoftECU_Input' incorporates:
   *  Inport: '<Root>/SoftECU_Input'
   */
  HEVControllerPwr_B.target_acc_m_s2 = HEVControllerPwr_U.SoftECU_Input.driver_input.target_acc_m_s2;

  /* SignalConversion generated from: '<Root>/SoftECU_Input' incorporates:
   *  Inport: '<Root>/SoftECU_Input'
   */
  HEVControllerPwr_B.brake_pressure_0_1 = HEVControllerPwr_U.SoftECU_Input.acc_feedback_m_s2;

  /* SignalConversion generated from: '<Root>/SoftECU_Input' incorporates:
   *  Inport: '<Root>/SoftECU_Input'
   */
  HEVControllerPwr_B.veh_speed_vx = HEVControllerPwr_U.SoftECU_Input.veh_speed_vx;

  /* Outputs for Atomic SubSystem: '<S4>/ax_estimator' */
  HEVControllerPwr_ax_estimator(HEVControllerPwr_B.brake_pressure_0_1, HEVControllerPwr_B.veh_speed_vx,
                                &HEVControllerPwr_B.ax_estimator, &HEVControllerPwr_DW.ax_estimator,
                                &HEVControllerPwr_P.ax_estimator, &HEVControllerPwr_P,
                                &HEVControllerPwr_X.ax_estimator);

  /* End of Outputs for SubSystem: '<S4>/ax_estimator' */

  /* SignalConversion generated from: '<Root>/SoftECU_Input' incorporates:
   *  Inport: '<Root>/SoftECU_Input'
   */
  HEVControllerPwr_B.brake_control_mode = HEVControllerPwr_U.SoftECU_Input.driver_input.brake_control_mode;

  /* SignalConversion generated from: '<Root>/SoftECU_Input' incorporates:
   *  Inport: '<Root>/SoftECU_Input'
   */
  HEVControllerPwr_B.Saturation = HEVControllerPwr_U.SoftECU_Input.driver_input.brake_pressure_01;

  /* SignalConversion generated from: '<Root>/SoftECU_Input' incorporates:
   *  Inport: '<Root>/SoftECU_Input'
   */
  HEVControllerPwr_B.gear_cmd__0N1D2R3P = HEVControllerPwr_U.SoftECU_Input.driver_input.gear_cmd__0N1D2R3P;

  /* Outputs for Atomic SubSystem: '<S4>/ABS_EBD' */
  HEVControllerPwr_ABS_EBD();

  /* End of Outputs for SubSystem: '<S4>/ABS_EBD' */

  /* RelationalOperator: '<S116>/Compare' incorporates:
   *  Abs: '<S115>/Abs'
   *  Constant: '<S116>/Constant'
   */
  rtb_Compare_li = (std::abs(HEVControllerPwr_B.veh_speed_vx) < HEVControllerPwr_P.speedOffset);

  /* SignalConversion generated from: '<Root>/SoftECU_Input' incorporates:
   *  Inport: '<Root>/SoftECU_Input'
   */
  HEVControllerPwr_B.acc_control_mode = HEVControllerPwr_U.SoftECU_Input.driver_input.acc_control_mode;

  /* SignalConversion generated from: '<Root>/SoftECU_Input' incorporates:
   *  Inport: '<Root>/SoftECU_Input'
   */
  HEVControllerPwr_B.Saturation = HEVControllerPwr_U.SoftECU_Input.driver_input.request_torque;

  /* MATLAB Function: '<S50>/AutoHold' incorporates:
   *  Constant: '<S50>/Constant2'
   */
  rtb_autoHoldSwitch = 0.0;
  accMode = (HEVControllerPwr_B.acc_control_mode == 0);
  torqueMode = (HEVControllerPwr_B.acc_control_mode == 1);
  gearDrive = (HEVControllerPwr_B.gear_cmd__0N1D2R3P == 1);
  gearReverse = (HEVControllerPwr_B.gear_cmd__0N1D2R3P == 2);
  if (rtb_Compare_li &&
      ((HEVControllerPwr_B.gear_cmd__0N1D2R3P == 0) || (HEVControllerPwr_B.gear_cmd__0N1D2R3P == 3))) {
    rtb_autoHoldSwitch = 1.0;
  }

  if (rtb_Compare_li && accMode && (HEVControllerPwr_B.target_acc_m_s2 < 0.001) && gearDrive) {
    rtb_autoHoldSwitch = 1.0;
  }

  if (rtb_Compare_li && torqueMode && (HEVControllerPwr_B.Saturation < 1.0) && gearDrive) {
    rtb_autoHoldSwitch = 1.0;
  }

  if (rtb_Compare_li && accMode && (HEVControllerPwr_B.target_acc_m_s2 > -0.001) && gearReverse) {
    rtb_autoHoldSwitch = 1.0;
  }

  if (rtb_Compare_li && torqueMode && gearReverse && (HEVControllerPwr_P.DriveLinePowType == 1.0) &&
      (HEVControllerPwr_B.Saturation < 1.0)) {
    rtb_autoHoldSwitch = 1.0;
  }

  if (rtb_Compare_li && torqueMode && gearReverse && (HEVControllerPwr_P.DriveLinePowType == 2.0) &&
      (HEVControllerPwr_B.Saturation > -1.0)) {
    rtb_autoHoldSwitch = 1.0;
  }

  /* End of MATLAB Function: '<S50>/AutoHold' */

  /* Switch: '<S4>/Switch1' incorporates:
   *  Constant: '<S4>/autoHoldBrakePressure_01'
   */
  if (rtb_autoHoldSwitch > HEVControllerPwr_P.Switch1_Threshold) {
    HEVControllerPwr_B.brake_pressure_0_1 = HEVControllerPwr_P.autoHoldBrakePressure_01_Value;
  }

  /* End of Switch: '<S4>/Switch1' */

  /* Relay: '<S54>/RelayCutOffAt6_EnableAt5' incorporates:
   *  Relay: '<S54>/RelayCutOffAt58_EnableAt55'
   */
  if (rtsiIsModeUpdateTimeStep(&(&HEVControllerPwr_M)->solverInfo)) {
    HEVControllerPwr_DW.RelayCutOffAt6_EnableAt5_Mode =
        ((HEVControllerPwr_B.veh_speed_vx >= HEVControllerPwr_P.RelayCutOffAt6_EnableAt5_OnVal) ||
         ((!(HEVControllerPwr_B.veh_speed_vx <= HEVControllerPwr_P.RelayCutOffAt6_EnableAt5_OffVal)) &&
          HEVControllerPwr_DW.RelayCutOffAt6_EnableAt5_Mode));
    HEVControllerPwr_DW.RelayCutOffAt58_EnableAt55_Mode =
        ((HEVControllerPwr_B.veh_speed_vx >= HEVControllerPwr_P.RelayCutOffAt58_EnableAt55_OnVal) ||
         ((!(HEVControllerPwr_B.veh_speed_vx <= HEVControllerPwr_P.RelayCutOffAt58_EnableAt55_OffVal)) &&
          HEVControllerPwr_DW.RelayCutOffAt58_EnableAt55_Mode));
  }

  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* Relay: '<S54>/RelayCutOffAt6_EnableAt5' */
    if (HEVControllerPwr_DW.RelayCutOffAt6_EnableAt5_Mode) {
      rtb_Gain_p = HEVControllerPwr_P.RelayCutOffAt6_EnableAt5_YOn;
    } else {
      rtb_Gain_p = HEVControllerPwr_P.RelayCutOffAt6_EnableAt5_YOff;
    }

    /* Relay: '<S54>/RelayCutOffAt58_EnableAt55' */
    if (HEVControllerPwr_DW.RelayCutOffAt58_EnableAt55_Mode) {
      rtb_autoHoldSwitch = HEVControllerPwr_P.RelayCutOffAt58_EnableAt55_YOn;
    } else {
      rtb_autoHoldSwitch = HEVControllerPwr_P.RelayCutOffAt58_EnableAt55_YOff;
    }

    /* Logic: '<S54>/Logical Operator' incorporates:
     *  Relay: '<S54>/RelayCutOffAt58_EnableAt55'
     *  Relay: '<S54>/RelayCutOffAt6_EnableAt5'
     */
    HEVControllerPwr_B.LogicalOperator = ((rtb_Gain_p != 0.0) || (rtb_autoHoldSwitch != 0.0));
  }

  /* Outputs for Atomic SubSystem: '<S4>/HCU' */
  HEVControllerPwr_HCU();

  /* End of Outputs for SubSystem: '<S4>/HCU' */

  /* Outport: '<Root>/SoftECU_Output' incorporates:
   *  BusCreator: '<S4>/Bus Creator'
   *  Constant: '<S52>/Constant'
   */
  HEVControllerPwr_Y.SoftECU_Output.pt_trq_cmd_Nm = 0.0;
  HEVControllerPwr_Y.SoftECU_Output.brake_pressure_0_1 = HEVControllerPwr_B.brake_pressure_0_1;
  HEVControllerPwr_Y.SoftECU_Output.brake_pressure_Pa[0] = HEVControllerPwr_P.Constant_Value_d;
  HEVControllerPwr_Y.SoftECU_Output.brake_pressure_Pa[1] = HEVControllerPwr_P.Constant_Value_d;
  HEVControllerPwr_Y.SoftECU_Output.brake_pressure_Pa[2] = HEVControllerPwr_P.Constant_Value_d;
  HEVControllerPwr_Y.SoftECU_Output.brake_pressure_Pa[3] = HEVControllerPwr_P.Constant_Value_d;
  HEVControllerPwr_Y.SoftECU_Output.abs_flag[0] = HEVControllerPwr_B.Memory[0];
  HEVControllerPwr_Y.SoftECU_Output.abs_flag[1] = HEVControllerPwr_B.Memory[1];
  HEVControllerPwr_Y.SoftECU_Output.abs_flag[2] = HEVControllerPwr_B.Memory[2];
  HEVControllerPwr_Y.SoftECU_Output.abs_flag[3] = HEVControllerPwr_B.Memory[3];
  HEVControllerPwr_Y.SoftECU_Output.throttle_01 = HEVControllerPwr_B.Saturation;
  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* BusCreator: '<S13>/Bus Creator' incorporates:
     *  Constant: '<S13>/EV_Swtich'
     *  Constant: '<S13>/Engine_ChargingSwitch'
     *  Constant: '<S13>/Engine_ClutchSwitch'
     *  Constant: '<S13>/Engine_Switch'
     *  Constant: '<S13>/HEV_Mode'
     */
    HEVControllerPwr_B.ReGen_HEVMode.EV_Swtich = HEVControllerPwr_P.EV_Swtich_Value;
    HEVControllerPwr_B.ReGen_HEVMode.Engine_Switch = HEVControllerPwr_P.Engine_Switch_Value;
    HEVControllerPwr_B.ReGen_HEVMode.Engine_ChargingSwitch = HEVControllerPwr_P.Engine_ChargingSwitch_Value;
    HEVControllerPwr_B.ReGen_HEVMode.Engine_ClutchSwitch = HEVControllerPwr_P.Engine_ClutchSwitch_Value;
    HEVControllerPwr_B.ReGen_HEVMode.HEV_Mode = HEVControllerPwr_P.HEV_Mode_Value;
  }

  /* Gain: '<S1>/kph' */
  rtb_autoHoldSwitch = HEVControllerPwr_P.kph_Gain * HEVControllerPwr_B.veh_speed_vx;

  /* Gain: '<S3>/Gain' incorporates:
   *  Inport: '<Root>/SoftECU_Input'
   *  SignalConversion generated from: '<Root>/SoftECU_Input'
   */
  HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery =
      HEVControllerPwr_P.Gain_Gain_j * HEVControllerPwr_U.SoftECU_Input.batt_soc_0_1;

  /* Gain: '<S5>/Gain' */
  rtb_Gain_p = HEVControllerPwr_P.Gain_Gain_b * HEVControllerPwr_B.veh_speed_vx;

  /* Relay: '<S5>/ReverseSpeedLimit_kph' */
  if (rtsiIsModeUpdateTimeStep(&(&HEVControllerPwr_M)->solverInfo)) {
    HEVControllerPwr_DW.ReverseSpeedLimit_kph_Mode =
        ((rtb_Gain_p >= HEVControllerPwr_P.ReverseSpeedLimit_kph_OnVal) ||
         ((!(rtb_Gain_p <= HEVControllerPwr_P.ReverseSpeedLimit_kph_OffVal)) &&
          HEVControllerPwr_DW.ReverseSpeedLimit_kph_Mode));
  }

  if (HEVControllerPwr_DW.ReverseSpeedLimit_kph_Mode) {
    /* Relay: '<S5>/ReverseSpeedLimit_kph' */
    HEVControllerPwr_B.ReverseSpeedLimit_kph = HEVControllerPwr_P.ReverseSpeedLimit_kph_YOn;
  } else {
    /* Relay: '<S5>/ReverseSpeedLimit_kph' */
    HEVControllerPwr_B.ReverseSpeedLimit_kph = HEVControllerPwr_P.ReverseSpeedLimit_kph_YOff;
  }

  /* End of Relay: '<S5>/ReverseSpeedLimit_kph' */

  /* MultiPortSwitch: '<S5>/GearSwitch' incorporates:
   *  Constant: '<S5>/Zero'
   *  DataTypeConversion: '<S3>/Data Type Conversion'
   */
  switch (HEVControllerPwr_B.gear_cmd__0N1D2R3P) {
    case 0:
      rtb_Gain_p = HEVControllerPwr_P.Zero_Value_h;
      break;

    case 1:
      break;

    case 2:
      rtb_Gain_p = HEVControllerPwr_B.ReverseSpeedLimit_kph;
      break;

    default:
      rtb_Gain_p = HEVControllerPwr_P.Zero_Value_h;
      break;
  }

  /* End of MultiPortSwitch: '<S5>/GearSwitch' */

  /* DeadZone: '<S5>/Dead Zone' incorporates:
   *  BusCreator: '<S4>/Bus Creator'
   */
  if (HEVControllerPwr_B.Saturation > HEVControllerPwr_P.DeadZone_End) {
    u0 = HEVControllerPwr_B.Saturation - HEVControllerPwr_P.DeadZone_End;
  } else if (HEVControllerPwr_B.Saturation >= HEVControllerPwr_P.DeadZone_Start) {
    u0 = 0.0;
  } else {
    u0 = HEVControllerPwr_B.Saturation - HEVControllerPwr_P.DeadZone_Start;
  }

  /* Saturate: '<S5>/ThrottleSaturation' incorporates:
   *  DeadZone: '<S5>/Dead Zone'
   */
  if (u0 > HEVControllerPwr_P.ThrottleSaturation_UpperSat) {
    u0 = HEVControllerPwr_P.ThrottleSaturation_UpperSat;
  } else if (u0 < HEVControllerPwr_P.ThrottleSaturation_LowerSat) {
    u0 = HEVControllerPwr_P.ThrottleSaturation_LowerSat;
  }

  /* Product: '<S5>/TotalPwdDmnd1' incorporates:
   *  Constant: '<S5>/front_diff_ratio'
   *  Constant: '<S5>/front_motor_ratio'
   *  Inport: '<Root>/FrontMotSpd[rad|s]'
   *  Lookup_n-D: '<S5>/Total Torque Request Table'
   *  Saturate: '<S5>/ThrottleSaturation'
   *  Switch: '<S15>/Switch2'
   */
  HEVControllerPwr_B.pwr_dmnd =
      look2_binlxpw(rtb_Gain_p, u0, HEVControllerPwr_P.total_torque_speed_bpt,
                    HEVControllerPwr_P.total_torque_throttle_bpt, HEVControllerPwr_P.total_torque_data,
                    HEVControllerPwr_P.TotalTorqueRequestTable_maxIndex, 14U) *
      HEVControllerPwr_U.FrontMotSpdrads / HEVControllerPwr_P.front_diff_ratio / HEVControllerPwr_P.front_motor_ratio;

  /* BusCreator generated from: '<S2>/HEV_DriveMode' incorporates:
   *  Constant: '<S2>/EngineStepInSpeed_kph'
   *  Constant: '<S2>/MinSocDriveVehicle'
   *  Constant: '<S2>/PwrDmnd_EV_Only'
   *  Constant: '<S2>/PwrDmnd_Hybrid'
   *  Constant: '<S2>/StartChargingSoc'
   *  Constant: '<S2>/TargetSoc'
   */
  HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_MinVehicleDrive =
      HEVControllerPwr_P.HEVController_MinSocDriveVehicle;
  HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_StartCharging =
      HEVControllerPwr_P.HEVController_StartChargingSoc;
  HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_EndCharging =
      HEVControllerPwr_P.HEVController_TargetSoc;
  HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.Speed_EngineStepIn =
      HEVControllerPwr_P.HEVController_EngineStepInSpeed_kph;
  HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd_EVOnly =
      HEVControllerPwr_P.HEVController_PwrDmnd_EV_Only;
  HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd_Hybrid =
      HEVControllerPwr_P.HEVController_PwrDmnd_Hybrid;
  HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd =
      HEVControllerPwr_B.pwr_dmnd;
  HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.Vehicle_Speed = rtb_autoHoldSwitch;
  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* Chart: '<S2>/HEV_DriveMode' incorporates:
     *  Constant: '<S2>/HEV_Mode_Duration'
     *  Constant: '<S2>/StepTime'
     */
    if (HEVControllerPwr_DW.is_active_c2_HEVControllerPwr == 0U) {
      HEVControllerPwr_DW.is_active_c2_HEVControllerPwr = 1U;
      HEVControllerPwr_DW.is_c2_HEVControllerPwr = HEVControllerPwr_IN_EVDrive_SerializeIncluded;
      HEVControllerPwr_DW.t_local = 0.0;
      HEVControllerPwr_DW.switchToEngine = false;
      HEVControllerPwr_DW.switchToHEV = false;
      HEVControllerPwr_B.hev_mode_output.EV_Swtich = 1.0;
      HEVControllerPwr_serialize_drive_mode(
          HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_StartCharging,
          HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
          &HEVControllerPwr_B.hev_mode_output);
    } else {
      switch (HEVControllerPwr_DW.is_c2_HEVControllerPwr) {
        case HEVControllerPwr_IN_EVDrive_SerializeIncluded:
          if (HEVControllerPwr_DW.switchToEngine) {
            HEVControllerPwr_DW.is_c2_HEVControllerPwr = HEVControllerPwr_IN_EngineDrive;
            HEVControllerPwr_DW.t_local = 0.0;
            HEVControllerPwr_DW.switchToEV = false;
            HEVControllerPwr_DW.switchToHEV = false;
            HEVControllerPwr_B.hev_mode_output.EV_Swtich = 0.0;
            HEVControllerPwr_B.hev_mode_output.Engine_Switch = 1.0;
            HEVControllerPwr_B.hev_mode_output.Engine_ClutchSwitch = 1.0;
            HEVControllerPwr_B.hev_mode_output.HEV_Mode = 1U;
            HEVControllerPwr_charging_decision(
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_StartCharging,
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                &HEVControllerPwr_B.hev_mode_output);
          } else if (HEVControllerPwr_DW.switchToHEV) {
            HEVControllerPwr_DW.is_c2_HEVControllerPwr = HEVControllerPwr_IN_HybridDrive_Parallel;
            HEVControllerPwr_DW.t_local = 0.0;
            HEVControllerPwr_DW.switchToEV = false;
            HEVControllerPwr_B.hev_mode_output.EV_Swtich = 1.0;
            HEVControllerPwr_B.hev_mode_output.Engine_Switch = 1.0;
            HEVControllerPwr_B.hev_mode_output.Engine_ChargingSwitch = 0.0;
            HEVControllerPwr_B.hev_mode_output.Engine_ClutchSwitch = 1.0;
            HEVControllerPwr_B.hev_mode_output.HEV_Mode = 2U;
          } else {
            HEVControllerPwr_serialize_drive_mode(
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_StartCharging,
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                &HEVControllerPwr_B.hev_mode_output);
            HEVControllerPwr_DW.t_local += HEVControllerPwr_P.stepTime;
            rtb_Compare_li = !(HEVControllerPwr_DW.t_local < HEVControllerPwr_P.HEV_Mode_Duration);
            HEVControllerPwr_DW.switchToEngine =
                ((HEVControllerPwr_from_ev_to_engine(
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .SoC_MinVehicleDrive,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .Speed_EngineStepIn,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd_EVOnly,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd_Hybrid,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .Vehicle_Speed) != 0.0) &&
                 rtb_Compare_li);
            HEVControllerPwr_DW.switchToHEV =
                ((HEVControllerPwr_from_engine_to_hev(
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .SoC_MinVehicleDrive,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd_Hybrid,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd) !=
                  0.0) &&
                 rtb_Compare_li);
          }
          break;

        case HEVControllerPwr_IN_EngineDrive:
          if (HEVControllerPwr_DW.switchToEV) {
            HEVControllerPwr_DW.is_c2_HEVControllerPwr = HEVControllerPwr_IN_EVDrive_SerializeIncluded;
            HEVControllerPwr_DW.t_local = 0.0;
            HEVControllerPwr_DW.switchToEngine = false;
            HEVControllerPwr_DW.switchToHEV = false;
            HEVControllerPwr_B.hev_mode_output.EV_Swtich = 1.0;
            HEVControllerPwr_serialize_drive_mode(
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_StartCharging,
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                &HEVControllerPwr_B.hev_mode_output);
          } else if (HEVControllerPwr_DW.switchToHEV) {
            HEVControllerPwr_DW.is_c2_HEVControllerPwr = HEVControllerPwr_IN_HybridDrive_Parallel;
            HEVControllerPwr_DW.t_local = 0.0;
            HEVControllerPwr_DW.switchToEngine = false;
            HEVControllerPwr_B.hev_mode_output.EV_Swtich = 1.0;
            HEVControllerPwr_B.hev_mode_output.Engine_Switch = 1.0;
            HEVControllerPwr_B.hev_mode_output.Engine_ChargingSwitch = 0.0;
            HEVControllerPwr_B.hev_mode_output.Engine_ClutchSwitch = 1.0;
            HEVControllerPwr_B.hev_mode_output.HEV_Mode = 2U;
          } else {
            HEVControllerPwr_charging_decision(
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_StartCharging,
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                &HEVControllerPwr_B.hev_mode_output);
            HEVControllerPwr_DW.t_local += HEVControllerPwr_P.stepTime;
            rtb_Compare_li = !(HEVControllerPwr_DW.t_local < HEVControllerPwr_P.HEV_Mode_Duration);
            HEVControllerPwr_DW.switchToEV =
                ((HEVControllerPwr_from_engine_to_ev(
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .SoC_MinVehicleDrive,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .Speed_EngineStepIn,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd_EVOnly,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .Vehicle_Speed) != 0.0) &&
                 rtb_Compare_li);
            HEVControllerPwr_DW.switchToHEV =
                ((HEVControllerPwr_from_engine_to_hev(
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .SoC_MinVehicleDrive,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd_Hybrid,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd) !=
                  0.0) &&
                 rtb_Compare_li);
          }
          break;

        default:
          /* case IN_HybridDrive_Parallel: */
          if (HEVControllerPwr_DW.switchToEV) {
            HEVControllerPwr_DW.is_c2_HEVControllerPwr = HEVControllerPwr_IN_EVDrive_SerializeIncluded;
            HEVControllerPwr_DW.t_local = 0.0;
            HEVControllerPwr_DW.switchToEngine = false;
            HEVControllerPwr_DW.switchToHEV = false;
            HEVControllerPwr_B.hev_mode_output.EV_Swtich = 1.0;
            HEVControllerPwr_serialize_drive_mode(
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_StartCharging,
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                &HEVControllerPwr_B.hev_mode_output);
          } else if (HEVControllerPwr_DW.switchToEngine) {
            HEVControllerPwr_DW.is_c2_HEVControllerPwr = HEVControllerPwr_IN_EngineDrive;
            HEVControllerPwr_DW.t_local = 0.0;
            HEVControllerPwr_DW.switchToHEV = false;
            HEVControllerPwr_B.hev_mode_output.EV_Swtich = 0.0;
            HEVControllerPwr_B.hev_mode_output.Engine_Switch = 1.0;
            HEVControllerPwr_B.hev_mode_output.Engine_ClutchSwitch = 1.0;
            HEVControllerPwr_B.hev_mode_output.HEV_Mode = 1U;
            HEVControllerPwr_charging_decision(
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_StartCharging,
                HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                &HEVControllerPwr_B.hev_mode_output);
          } else {
            HEVControllerPwr_DW.t_local += HEVControllerPwr_P.stepTime;
            rtb_Compare_li = !(HEVControllerPwr_DW.t_local < HEVControllerPwr_P.HEV_Mode_Duration);
            HEVControllerPwr_DW.switchToEngine =
                ((HEVControllerPwr_from_hev_to_engine(
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .SoC_MinVehicleDrive,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .Speed_EngineStepIn,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd_EVOnly,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd_Hybrid,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .Vehicle_Speed) != 0.0) &&
                 rtb_Compare_li);
            HEVControllerPwr_DW.switchToEV =
                ((HEVControllerPwr_from_hev_to_ev(
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .SoC_MinVehicleDrive,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .Speed_EngineStepIn,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd_EVOnly,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.SoC_Battery,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1.PwrDmnd,
                      HEVControllerPwr_B.BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1
                          .Vehicle_Speed) != 0.0) &&
                 rtb_Compare_li);
          }
          break;
      }
    }

    /* End of Chart: '<S2>/HEV_DriveMode' */
  }

  /* Saturate: '<S1>/Saturation1' incorporates:
   *  BusCreator: '<S4>/Bus Creator'
   */
  if (HEVControllerPwr_B.brake_pressure_0_1 > HEVControllerPwr_P.Saturation1_UpperSat) {
    rtb_Gain_p = HEVControllerPwr_P.Saturation1_UpperSat;
  } else if (HEVControllerPwr_B.brake_pressure_0_1 < HEVControllerPwr_P.Saturation1_LowerSat) {
    rtb_Gain_p = HEVControllerPwr_P.Saturation1_LowerSat;
  } else {
    rtb_Gain_p = HEVControllerPwr_B.brake_pressure_0_1;
  }

  /* Switch: '<S7>/HEVMode_Router' incorporates:
   *  BusCreator: '<S4>/Bus Creator'
   *  Constant: '<S45>/Constant'
   *  Constant: '<S46>/Constant'
   *  Constant: '<S47>/Constant'
   *  Constant: '<S48>/Constant'
   *  DataTypeConversion: '<S3>/Data Type Conversion'
   *  Logic: '<S12>/Logical Operator'
   *  Logic: '<S12>/ReGen_Enable'
   *  RelationalOperator: '<S45>/Compare'
   *  RelationalOperator: '<S46>/Compare'
   *  RelationalOperator: '<S47>/Compare'
   *  RelationalOperator: '<S48>/Compare'
   *  Saturate: '<S1>/Saturation1'
   */
  if ((rtb_autoHoldSwitch > HEVControllerPwr_P.VXLOW_ReGen_const) &&
      ((HEVControllerPwr_B.Saturation < HEVControllerPwr_P.HEVController_ThrottleDisableThresh) ||
       (rtb_Gain_p > HEVControllerPwr_P.HEVController_BrakeEnableThresh)) &&
      (HEVControllerPwr_B.gear_cmd__0N1D2R3P == HEVControllerPwr_P.Gear_DriveMode_const)) {
    rtb_HEVMode_Router = HEVControllerPwr_B.ReGen_HEVMode;
  } else {
    rtb_HEVMode_Router = HEVControllerPwr_B.hev_mode_output;
  }

  /* End of Switch: '<S7>/HEVMode_Router' */
  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* BusCreator: '<S9>/Bus Creator' incorporates:
     *  Constant: '<S9>/EV_Swtich'
     *  Constant: '<S9>/Engine_ChargingSwitch'
     *  Constant: '<S9>/Engine_ClutchSwitch'
     *  Constant: '<S9>/Engine_Switch'
     *  Constant: '<S9>/HEV_Mode'
     */
    HEVControllerPwr_B.ReGen_HEVMode_p.EV_Swtich = HEVControllerPwr_P.EV_Swtich_Value_m;
    HEVControllerPwr_B.ReGen_HEVMode_p.Engine_Switch = HEVControllerPwr_P.Engine_Switch_Value_b;
    HEVControllerPwr_B.ReGen_HEVMode_p.Engine_ChargingSwitch = HEVControllerPwr_P.Engine_ChargingSwitch_Value_i;
    HEVControllerPwr_B.ReGen_HEVMode_p.Engine_ClutchSwitch = HEVControllerPwr_P.Engine_ClutchSwitch_Value_o;
    HEVControllerPwr_B.ReGen_HEVMode_p.HEV_Mode = HEVControllerPwr_P.HEV_Mode_Value_d;
  }

  /* MultiPortSwitch: '<S7>/Multiport Switch' incorporates:
   *  DataTypeConversion: '<S3>/Data Type Conversion'
   */
  switch (HEVControllerPwr_B.gear_cmd__0N1D2R3P) {
    case 0:
      /* MultiPortSwitch: '<S7>/Multiport Switch' */
      HEVControllerPwr_B.MultiportSwitch = rtb_HEVMode_Router;
      break;

    case 1:
      /* MultiPortSwitch: '<S7>/Multiport Switch' */
      HEVControllerPwr_B.MultiportSwitch = rtb_HEVMode_Router;
      break;

    case 2:
      /* MultiPortSwitch: '<S7>/Multiport Switch' */
      HEVControllerPwr_B.MultiportSwitch = HEVControllerPwr_B.ReGen_HEVMode_p;
      break;

    default:
      /* MultiPortSwitch: '<S7>/Multiport Switch' */
      HEVControllerPwr_B.MultiportSwitch = rtb_HEVMode_Router;
      break;
  }

  /* End of MultiPortSwitch: '<S7>/Multiport Switch' */
  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* Gain: '<S7>/rpm' incorporates:
     *  Inport: '<Root>/EngineSpd[rad|s]'
     */
    rtb_autoHoldSwitch = HEVControllerPwr_P.rpm_Gain * HEVControllerPwr_U.EngineSpdrads;

    /* Chart: '<S11>/PwrCal' incorporates:
     *  Inport: '<Root>/FrontMotSpd[rad|s]'
     *  Inport: '<Root>/P4MotSpd[rad|s]'
     */
    if (HEVControllerPwr_DW.is_active_c4_HEVControllerPwr == 0U) {
      HEVControllerPwr_DW.is_active_c4_HEVControllerPwr = 1U;
      HEVControllerPwr_DW.is_c4_HEVControllerPwr = HEVControllerPwr_IN_HEVDrive;
      HEVControllerPwr_DW.is_HEVDrive = HEVControllerPwr_IN_EV;
      HEVControllerPwr_B.HEV_Mode = HEVControllerPwr_B.MultiportSwitch;
      HEVControllerPwr_B.Pwr_Dmnd_w_ = HEVControllerPwr_B.pwr_dmnd;

      /* Outputs for Function Call SubSystem: '<S22>/EV_Pwr_Calculation' */
      HEVControllerPwr_EV_Pwr_Calculation(
          &HEVControllerPwr_B.HEV_Mode, HEVControllerPwr_B.Pwr_Dmnd_w_,
          HEVControllerPwr_P.HEVController_HEV_4WD_FrontPwrRatio, HEVControllerPwr_P.EnableP4,
          HEVControllerPwr_P.EngIdleSpd, HEVControllerPwr_P.HEVController_MaxChrgPwr,
          HEVControllerPwr_P.HEVController_DefaultChrgPwr, HEVControllerPwr_P.Inverse_EngOptimal_Speed_RPM,
          HEVControllerPwr_P.Inverse_EngOptimal_Pwr_KW, &HEVControllerPwr_B.EV_Pwr_Calculation,
          &HEVControllerPwr_P.EV_Pwr_Calculation, &HEVControllerPwr_P);

      /* End of Outputs for SubSystem: '<S22>/EV_Pwr_Calculation' */
      HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.EV_Pwr_Calculation.Merge;
    } else if (HEVControllerPwr_DW.is_c4_HEVControllerPwr == HEVControllerPwr_IN_HEVDrive) {
      if (HEVControllerPwr_B.MultiportSwitch.HEV_Mode == 3U) {
        HEVControllerPwr_DW.is_HEVDrive = HEVControllerPwr_IN_NO_ACTIVE_CHILD;
        HEVControllerPwr_DW.is_c4_HEVControllerPwr = HEVControllerPwr_IN_HEV_ReGenMode;
        HEVControllerPwr_B.HEV_Mode_g = HEVControllerPwr_B.MultiportSwitch;
        HEVControllerPwr_B.Motor_Speed_rads_ = HEVControllerPwr_U.FrontMotSpdrads;
        HEVControllerPwr_B.P4_Motor_Speed_rads_ = HEVControllerPwr_U.P4MotSpdrads;

        /* Outputs for Function Call SubSystem: '<S22>/ReGen_Pwr_Calculation' */
        HEVControllerPwr_ReGen_Pwr_Calculation(
            &HEVControllerPwr_B.HEV_Mode_g, HEVControllerPwr_B.Motor_Speed_rads_,
            HEVControllerPwr_B.P4_Motor_Speed_rads_, HEVControllerPwr_P.HEVController_FrontMotor_MaxTorque,
            HEVControllerPwr_P.HEVController_FrontMotor_Speed_rads, HEVControllerPwr_P.EnableP4,
            HEVControllerPwr_P.HEVController_P4_Motor_MaxTorque, HEVControllerPwr_P.HEVController_P4_Motor_Speed_rads,
            HEVControllerPwr_P.EngIdleSpd, HEVControllerPwr_P.HEVController_MaxReGenPwr,
            &HEVControllerPwr_B.ReGen_Pwr_Calculation, &HEVControllerPwr_P.ReGen_Pwr_Calculation);

        /* End of Outputs for SubSystem: '<S22>/ReGen_Pwr_Calculation' */
        HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.ReGen_Pwr_Calculation.Merge;
      } else {
        switch (HEVControllerPwr_DW.is_HEVDrive) {
          case HEVControllerPwr_IN_EV:
            if (HEVControllerPwr_B.MultiportSwitch.HEV_Mode == 1U) {
              HEVControllerPwr_DW.is_HEVDrive = HEVControllerPwr_IN_Engine;
              HEVControllerPwr_B.HEV_Mode_f = HEVControllerPwr_B.MultiportSwitch;
              HEVControllerPwr_B.Pwr_Dmnd_w__l = HEVControllerPwr_B.pwr_dmnd;
              HEVControllerPwr_B.Engine_Speed_rpm_ = rtb_autoHoldSwitch;

              /* Outputs for Function Call SubSystem: '<S22>/Engine_Pwr_Calculation' */
              HEVControllerPwr_Engine_Pwr_Calculation(
                  &HEVControllerPwr_B.HEV_Mode_f, HEVControllerPwr_B.Pwr_Dmnd_w__l,
                  HEVControllerPwr_B.Engine_Speed_rpm_, HEVControllerPwr_P.HEVController_DefaultChrgPwr,
                  HEVControllerPwr_P.HEVController_EngOptimal_Pwr_KW,
                  HEVControllerPwr_P.HEVController_EngOptimal_Speed_RPM, HEVControllerPwr_P.HEVController_MaxChrgPwr,
                  HEVControllerPwr_P.EngIdleSpd, &HEVControllerPwr_B.Engine_Pwr_Calculation,
                  &HEVControllerPwr_P.Engine_Pwr_Calculation);

              /* End of Outputs for SubSystem: '<S22>/Engine_Pwr_Calculation' */
              HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.Engine_Pwr_Calculation.Merge;
            } else if (HEVControllerPwr_B.MultiportSwitch.HEV_Mode == 2U) {
              HEVControllerPwr_DW.is_HEVDrive = HEVControllerPwr_IN_Hybrid;
              HEVControllerPwr_B.HEV_Mode_j = HEVControllerPwr_B.MultiportSwitch;
              HEVControllerPwr_B.Pwr_Dmnd_w__e = HEVControllerPwr_B.pwr_dmnd;
              HEVControllerPwr_B.Engine_Speed_rpm__k = rtb_autoHoldSwitch;

              /* Outputs for Function Call SubSystem: '<S22>/HEV_Pwr_Calculation' */
              HEVControllerPwr_HEV_Pwr_Calculation(
                  &HEVControllerPwr_B.HEV_Mode_j, HEVControllerPwr_B.Pwr_Dmnd_w__e,
                  HEVControllerPwr_B.Engine_Speed_rpm__k, HEVControllerPwr_P.HEVController_EngOptimal_Pwr_KW,
                  HEVControllerPwr_P.HEVController_EngOptimal_Speed_RPM,
                  HEVControllerPwr_P.HEVController_HEV_4WD_FrontPwrRatio, HEVControllerPwr_P.EnableP4,
                  HEVControllerPwr_P.EngIdleSpd, &HEVControllerPwr_B.HEV_Pwr_Calculation,
                  &HEVControllerPwr_P.HEV_Pwr_Calculation);

              /* End of Outputs for SubSystem: '<S22>/HEV_Pwr_Calculation' */
              HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.HEV_Pwr_Calculation.Merge;
            } else {
              HEVControllerPwr_B.HEV_Mode = HEVControllerPwr_B.MultiportSwitch;
              HEVControllerPwr_B.Pwr_Dmnd_w_ = HEVControllerPwr_B.pwr_dmnd;

              /* Outputs for Function Call SubSystem: '<S22>/EV_Pwr_Calculation' */
              HEVControllerPwr_EV_Pwr_Calculation(
                  &HEVControllerPwr_B.HEV_Mode, HEVControllerPwr_B.Pwr_Dmnd_w_,
                  HEVControllerPwr_P.HEVController_HEV_4WD_FrontPwrRatio, HEVControllerPwr_P.EnableP4,
                  HEVControllerPwr_P.EngIdleSpd, HEVControllerPwr_P.HEVController_MaxChrgPwr,
                  HEVControllerPwr_P.HEVController_DefaultChrgPwr, HEVControllerPwr_P.Inverse_EngOptimal_Speed_RPM,
                  HEVControllerPwr_P.Inverse_EngOptimal_Pwr_KW, &HEVControllerPwr_B.EV_Pwr_Calculation,
                  &HEVControllerPwr_P.EV_Pwr_Calculation, &HEVControllerPwr_P);

              /* End of Outputs for SubSystem: '<S22>/EV_Pwr_Calculation' */
              HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.EV_Pwr_Calculation.Merge;
            }
            break;

          case HEVControllerPwr_IN_Engine:
            if (HEVControllerPwr_B.MultiportSwitch.HEV_Mode == 2U) {
              HEVControllerPwr_DW.is_HEVDrive = HEVControllerPwr_IN_Hybrid;
              HEVControllerPwr_B.HEV_Mode_j = HEVControllerPwr_B.MultiportSwitch;
              HEVControllerPwr_B.Pwr_Dmnd_w__e = HEVControllerPwr_B.pwr_dmnd;
              HEVControllerPwr_B.Engine_Speed_rpm__k = rtb_autoHoldSwitch;

              /* Outputs for Function Call SubSystem: '<S22>/HEV_Pwr_Calculation' */
              HEVControllerPwr_HEV_Pwr_Calculation(
                  &HEVControllerPwr_B.HEV_Mode_j, HEVControllerPwr_B.Pwr_Dmnd_w__e,
                  HEVControllerPwr_B.Engine_Speed_rpm__k, HEVControllerPwr_P.HEVController_EngOptimal_Pwr_KW,
                  HEVControllerPwr_P.HEVController_EngOptimal_Speed_RPM,
                  HEVControllerPwr_P.HEVController_HEV_4WD_FrontPwrRatio, HEVControllerPwr_P.EnableP4,
                  HEVControllerPwr_P.EngIdleSpd, &HEVControllerPwr_B.HEV_Pwr_Calculation,
                  &HEVControllerPwr_P.HEV_Pwr_Calculation);

              /* End of Outputs for SubSystem: '<S22>/HEV_Pwr_Calculation' */
              HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.HEV_Pwr_Calculation.Merge;
            } else if ((HEVControllerPwr_B.MultiportSwitch.HEV_Mode == 0U) ||
                       (HEVControllerPwr_B.MultiportSwitch.HEV_Mode == 4U)) {
              HEVControllerPwr_DW.is_HEVDrive = HEVControllerPwr_IN_EV;
              HEVControllerPwr_B.HEV_Mode = HEVControllerPwr_B.MultiportSwitch;
              HEVControllerPwr_B.Pwr_Dmnd_w_ = HEVControllerPwr_B.pwr_dmnd;

              /* Outputs for Function Call SubSystem: '<S22>/EV_Pwr_Calculation' */
              HEVControllerPwr_EV_Pwr_Calculation(
                  &HEVControllerPwr_B.HEV_Mode, HEVControllerPwr_B.Pwr_Dmnd_w_,
                  HEVControllerPwr_P.HEVController_HEV_4WD_FrontPwrRatio, HEVControllerPwr_P.EnableP4,
                  HEVControllerPwr_P.EngIdleSpd, HEVControllerPwr_P.HEVController_MaxChrgPwr,
                  HEVControllerPwr_P.HEVController_DefaultChrgPwr, HEVControllerPwr_P.Inverse_EngOptimal_Speed_RPM,
                  HEVControllerPwr_P.Inverse_EngOptimal_Pwr_KW, &HEVControllerPwr_B.EV_Pwr_Calculation,
                  &HEVControllerPwr_P.EV_Pwr_Calculation, &HEVControllerPwr_P);

              /* End of Outputs for SubSystem: '<S22>/EV_Pwr_Calculation' */
              HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.EV_Pwr_Calculation.Merge;
            } else {
              HEVControllerPwr_B.HEV_Mode_f = HEVControllerPwr_B.MultiportSwitch;
              HEVControllerPwr_B.Pwr_Dmnd_w__l = HEVControllerPwr_B.pwr_dmnd;
              HEVControllerPwr_B.Engine_Speed_rpm_ = rtb_autoHoldSwitch;

              /* Outputs for Function Call SubSystem: '<S22>/Engine_Pwr_Calculation' */
              HEVControllerPwr_Engine_Pwr_Calculation(
                  &HEVControllerPwr_B.HEV_Mode_f, HEVControllerPwr_B.Pwr_Dmnd_w__l,
                  HEVControllerPwr_B.Engine_Speed_rpm_, HEVControllerPwr_P.HEVController_DefaultChrgPwr,
                  HEVControllerPwr_P.HEVController_EngOptimal_Pwr_KW,
                  HEVControllerPwr_P.HEVController_EngOptimal_Speed_RPM, HEVControllerPwr_P.HEVController_MaxChrgPwr,
                  HEVControllerPwr_P.EngIdleSpd, &HEVControllerPwr_B.Engine_Pwr_Calculation,
                  &HEVControllerPwr_P.Engine_Pwr_Calculation);

              /* End of Outputs for SubSystem: '<S22>/Engine_Pwr_Calculation' */
              HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.Engine_Pwr_Calculation.Merge;
            }
            break;

          default:
            /* case IN_Hybrid: */
            if (HEVControllerPwr_B.MultiportSwitch.HEV_Mode == 1U) {
              HEVControllerPwr_DW.is_HEVDrive = HEVControllerPwr_IN_Engine;
              HEVControllerPwr_B.HEV_Mode_f = HEVControllerPwr_B.MultiportSwitch;
              HEVControllerPwr_B.Pwr_Dmnd_w__l = HEVControllerPwr_B.pwr_dmnd;
              HEVControllerPwr_B.Engine_Speed_rpm_ = rtb_autoHoldSwitch;

              /* Outputs for Function Call SubSystem: '<S22>/Engine_Pwr_Calculation' */
              HEVControllerPwr_Engine_Pwr_Calculation(
                  &HEVControllerPwr_B.HEV_Mode_f, HEVControllerPwr_B.Pwr_Dmnd_w__l,
                  HEVControllerPwr_B.Engine_Speed_rpm_, HEVControllerPwr_P.HEVController_DefaultChrgPwr,
                  HEVControllerPwr_P.HEVController_EngOptimal_Pwr_KW,
                  HEVControllerPwr_P.HEVController_EngOptimal_Speed_RPM, HEVControllerPwr_P.HEVController_MaxChrgPwr,
                  HEVControllerPwr_P.EngIdleSpd, &HEVControllerPwr_B.Engine_Pwr_Calculation,
                  &HEVControllerPwr_P.Engine_Pwr_Calculation);

              /* End of Outputs for SubSystem: '<S22>/Engine_Pwr_Calculation' */
              HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.Engine_Pwr_Calculation.Merge;
            } else if ((HEVControllerPwr_B.MultiportSwitch.HEV_Mode == 0U) ||
                       (HEVControllerPwr_B.MultiportSwitch.HEV_Mode == 4U)) {
              HEVControllerPwr_DW.is_HEVDrive = HEVControllerPwr_IN_EV;
              HEVControllerPwr_B.HEV_Mode = HEVControllerPwr_B.MultiportSwitch;
              HEVControllerPwr_B.Pwr_Dmnd_w_ = HEVControllerPwr_B.pwr_dmnd;

              /* Outputs for Function Call SubSystem: '<S22>/EV_Pwr_Calculation' */
              HEVControllerPwr_EV_Pwr_Calculation(
                  &HEVControllerPwr_B.HEV_Mode, HEVControllerPwr_B.Pwr_Dmnd_w_,
                  HEVControllerPwr_P.HEVController_HEV_4WD_FrontPwrRatio, HEVControllerPwr_P.EnableP4,
                  HEVControllerPwr_P.EngIdleSpd, HEVControllerPwr_P.HEVController_MaxChrgPwr,
                  HEVControllerPwr_P.HEVController_DefaultChrgPwr, HEVControllerPwr_P.Inverse_EngOptimal_Speed_RPM,
                  HEVControllerPwr_P.Inverse_EngOptimal_Pwr_KW, &HEVControllerPwr_B.EV_Pwr_Calculation,
                  &HEVControllerPwr_P.EV_Pwr_Calculation, &HEVControllerPwr_P);

              /* End of Outputs for SubSystem: '<S22>/EV_Pwr_Calculation' */
              HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.EV_Pwr_Calculation.Merge;
            } else {
              HEVControllerPwr_B.HEV_Mode_j = HEVControllerPwr_B.MultiportSwitch;
              HEVControllerPwr_B.Pwr_Dmnd_w__e = HEVControllerPwr_B.pwr_dmnd;
              HEVControllerPwr_B.Engine_Speed_rpm__k = rtb_autoHoldSwitch;

              /* Outputs for Function Call SubSystem: '<S22>/HEV_Pwr_Calculation' */
              HEVControllerPwr_HEV_Pwr_Calculation(
                  &HEVControllerPwr_B.HEV_Mode_j, HEVControllerPwr_B.Pwr_Dmnd_w__e,
                  HEVControllerPwr_B.Engine_Speed_rpm__k, HEVControllerPwr_P.HEVController_EngOptimal_Pwr_KW,
                  HEVControllerPwr_P.HEVController_EngOptimal_Speed_RPM,
                  HEVControllerPwr_P.HEVController_HEV_4WD_FrontPwrRatio, HEVControllerPwr_P.EnableP4,
                  HEVControllerPwr_P.EngIdleSpd, &HEVControllerPwr_B.HEV_Pwr_Calculation,
                  &HEVControllerPwr_P.HEV_Pwr_Calculation);

              /* End of Outputs for SubSystem: '<S22>/HEV_Pwr_Calculation' */
              HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.HEV_Pwr_Calculation.Merge;
            }
            break;
        }
      }

      /* case IN_HEV_ReGenMode: */
    } else if (HEVControllerPwr_B.MultiportSwitch.HEV_Mode != 3U) {
      HEVControllerPwr_DW.is_c4_HEVControllerPwr = HEVControllerPwr_IN_HEVDrive;
      HEVControllerPwr_DW.is_HEVDrive = HEVControllerPwr_IN_EV;
      HEVControllerPwr_B.HEV_Mode = HEVControllerPwr_B.MultiportSwitch;
      HEVControllerPwr_B.Pwr_Dmnd_w_ = HEVControllerPwr_B.pwr_dmnd;

      /* Outputs for Function Call SubSystem: '<S22>/EV_Pwr_Calculation' */
      HEVControllerPwr_EV_Pwr_Calculation(
          &HEVControllerPwr_B.HEV_Mode, HEVControllerPwr_B.Pwr_Dmnd_w_,
          HEVControllerPwr_P.HEVController_HEV_4WD_FrontPwrRatio, HEVControllerPwr_P.EnableP4,
          HEVControllerPwr_P.EngIdleSpd, HEVControllerPwr_P.HEVController_MaxChrgPwr,
          HEVControllerPwr_P.HEVController_DefaultChrgPwr, HEVControllerPwr_P.Inverse_EngOptimal_Speed_RPM,
          HEVControllerPwr_P.Inverse_EngOptimal_Pwr_KW, &HEVControllerPwr_B.EV_Pwr_Calculation,
          &HEVControllerPwr_P.EV_Pwr_Calculation, &HEVControllerPwr_P);

      /* End of Outputs for SubSystem: '<S22>/EV_Pwr_Calculation' */
      HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.EV_Pwr_Calculation.Merge;
    } else {
      HEVControllerPwr_B.HEV_Mode_g = HEVControllerPwr_B.MultiportSwitch;
      HEVControllerPwr_B.Motor_Speed_rads_ = HEVControllerPwr_U.FrontMotSpdrads;
      HEVControllerPwr_B.P4_Motor_Speed_rads_ = HEVControllerPwr_U.P4MotSpdrads;

      /* Outputs for Function Call SubSystem: '<S22>/ReGen_Pwr_Calculation' */
      HEVControllerPwr_ReGen_Pwr_Calculation(
          &HEVControllerPwr_B.HEV_Mode_g, HEVControllerPwr_B.Motor_Speed_rads_, HEVControllerPwr_B.P4_Motor_Speed_rads_,
          HEVControllerPwr_P.HEVController_FrontMotor_MaxTorque, HEVControllerPwr_P.HEVController_FrontMotor_Speed_rads,
          HEVControllerPwr_P.EnableP4, HEVControllerPwr_P.HEVController_P4_Motor_MaxTorque,
          HEVControllerPwr_P.HEVController_P4_Motor_Speed_rads, HEVControllerPwr_P.EngIdleSpd,
          HEVControllerPwr_P.HEVController_MaxReGenPwr, &HEVControllerPwr_B.ReGen_Pwr_Calculation,
          &HEVControllerPwr_P.ReGen_Pwr_Calculation);

      /* End of Outputs for SubSystem: '<S22>/ReGen_Pwr_Calculation' */
      HEVControllerPwr_B.HEV_PwrDmnd_Output = HEVControllerPwr_B.ReGen_Pwr_Calculation.Merge;
    }

    /* End of Chart: '<S11>/PwrCal' */
  }

  /* Abs: '<S8>/Abs1' incorporates:
   *  Abs: '<S14>/Abs'
   *  Inport: '<Root>/FrontMotSpd[rad|s]'
   */
  rtb_Gain_p = std::abs(HEVControllerPwr_U.FrontMotSpdrads);

  /* Lookup_n-D: '<S8>/Front_MotorMaxTrq' incorporates:
   *  Abs: '<S8>/Abs1'
   */
  rtb_autoHoldSwitch = look1_binlcapw(rtb_Gain_p, HEVControllerPwr_P.HEVController_FrontMotor_Speed_rads,
                                      HEVControllerPwr_P.HEVController_FrontMotor_MaxTorque, 12U);

  /* Switch: '<S14>/Switch' incorporates:
   *  Constant: '<S14>/Constant'
   *  Constant: '<S16>/Constant'
   *  Inport: '<Root>/FrontMotSpd[rad|s]'
   *  Product: '<S14>/Product'
   *  RelationalOperator: '<S16>/Compare'
   *  Switch: '<S14>/sign'
   */
  if (rtb_Gain_p < HEVControllerPwr_P.DivProtect_MinVal) {
    /* Switch: '<S14>/sign' incorporates:
     *  Constant: '<S14>/Constant1'
     *  Constant: '<S14>/Constant2'
     *  Constant: '<S17>/Constant'
     *  Inport: '<Root>/FrontMotSpd[rad|s]'
     *  RelationalOperator: '<S17>/Compare'
     */
    if (HEVControllerPwr_U.FrontMotSpdrads >= HEVControllerPwr_P.Constant_Value) {
      rtb_Gain_p = HEVControllerPwr_P.Constant1_Value_i;
    } else {
      rtb_Gain_p = HEVControllerPwr_P.Constant2_Value_i;
    }

    rtb_Gain_p *= HEVControllerPwr_P.DivProtect_MinVal;
  } else {
    rtb_Gain_p = HEVControllerPwr_U.FrontMotSpdrads;
  }

  /* End of Switch: '<S14>/Switch' */

  /* Product: '<S8>/CalMotorTorque' */
  rtb_Gain_p = HEVControllerPwr_B.HEV_PwrDmnd_Output.EV_PwrDmnd / rtb_Gain_p;

  /* Switch: '<S15>/Switch2' incorporates:
   *  RelationalOperator: '<S15>/LowerRelop1'
   */
  if (rtb_Gain_p > rtb_autoHoldSwitch) {
    /* Outport: '<Root>/HEV_Output' */
    HEVControllerPwr_Y.HEV_Output.MotRequestTorque_Nm = rtb_autoHoldSwitch;
  } else {
    /* Gain: '<S8>/Gain' */
    rtb_autoHoldSwitch *= HEVControllerPwr_P.Gain_Gain;

    /* Switch: '<S15>/Switch' incorporates:
     *  RelationalOperator: '<S15>/UpperRelop'
     */
    if (rtb_Gain_p < rtb_autoHoldSwitch) {
      /* Outport: '<Root>/HEV_Output' */
      HEVControllerPwr_Y.HEV_Output.MotRequestTorque_Nm = rtb_autoHoldSwitch;
    } else {
      /* Outport: '<Root>/HEV_Output' */
      HEVControllerPwr_Y.HEV_Output.MotRequestTorque_Nm = rtb_Gain_p;
    }

    /* End of Switch: '<S15>/Switch' */
  }

  /* End of Switch: '<S15>/Switch2' */

  /* Abs: '<S10>/Abs1' incorporates:
   *  Abs: '<S18>/Abs'
   *  Inport: '<Root>/P4MotSpd[rad|s]'
   */
  rtb_Gain_p = std::abs(HEVControllerPwr_U.P4MotSpdrads);

  /* Lookup_n-D: '<S10>/P4_MotorMaxTrq' incorporates:
   *  Abs: '<S10>/Abs1'
   */
  rtb_autoHoldSwitch = look1_binlxpw(rtb_Gain_p, HEVControllerPwr_P.HEVController_P4_Motor_Speed_rads,
                                     HEVControllerPwr_P.HEVController_P4_Motor_MaxTorque, 12U);
  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* Product: '<S10>/P4_PwrDmnd' */
    HEVControllerPwr_B.P4_PwrDmnd =
        HEVControllerPwr_B.HEV_PwrDmnd_Output.P4_Motor_Enabled * HEVControllerPwr_B.HEV_PwrDmnd_Output.P4_PwrDmnd;
  }

  /* Switch: '<S18>/Switch' incorporates:
   *  Constant: '<S18>/Constant'
   *  Constant: '<S20>/Constant'
   *  Inport: '<Root>/P4MotSpd[rad|s]'
   *  Product: '<S18>/Product'
   *  RelationalOperator: '<S20>/Compare'
   *  Switch: '<S18>/sign'
   */
  if (rtb_Gain_p < HEVControllerPwr_P.DivProtect_MinVal_p) {
    /* Switch: '<S18>/sign' incorporates:
     *  Constant: '<S18>/Constant1'
     *  Constant: '<S18>/Constant2'
     *  Constant: '<S21>/Constant'
     *  Inport: '<Root>/P4MotSpd[rad|s]'
     *  RelationalOperator: '<S21>/Compare'
     */
    if (HEVControllerPwr_U.P4MotSpdrads >= HEVControllerPwr_P.Constant_Value_h) {
      rtb_Gain_p = HEVControllerPwr_P.Constant1_Value_c;
    } else {
      rtb_Gain_p = HEVControllerPwr_P.Constant2_Value_g;
    }

    rtb_Gain_p *= HEVControllerPwr_P.DivProtect_MinVal_p;
  } else {
    rtb_Gain_p = HEVControllerPwr_U.P4MotSpdrads;
  }

  /* End of Switch: '<S18>/Switch' */

  /* Product: '<S10>/CalMotorTorque' */
  rtb_Gain_p = HEVControllerPwr_B.P4_PwrDmnd / rtb_Gain_p;

  /* Switch: '<S19>/Switch2' incorporates:
   *  RelationalOperator: '<S19>/LowerRelop1'
   */
  if (rtb_Gain_p > rtb_autoHoldSwitch) {
    /* Outport: '<Root>/HEV_Output' */
    HEVControllerPwr_Y.HEV_Output.P4_MotRequestTorque_Nm = rtb_autoHoldSwitch;
  } else {
    /* Gain: '<S10>/Gain' */
    rtb_autoHoldSwitch *= HEVControllerPwr_P.Gain_Gain_f;

    /* Switch: '<S19>/Switch' incorporates:
     *  RelationalOperator: '<S19>/UpperRelop'
     */
    if (rtb_Gain_p < rtb_autoHoldSwitch) {
      /* Outport: '<Root>/HEV_Output' */
      HEVControllerPwr_Y.HEV_Output.P4_MotRequestTorque_Nm = rtb_autoHoldSwitch;
    } else {
      /* Outport: '<Root>/HEV_Output' */
      HEVControllerPwr_Y.HEV_Output.P4_MotRequestTorque_Nm = rtb_Gain_p;
    }

    /* End of Switch: '<S19>/Switch' */
  }

  /* End of Switch: '<S19>/Switch2' */

  /* Outport: '<Root>/HEV_Output' incorporates:
   *  BusCreator generated from: '<Root>/HEV_Output'
   */
  HEVControllerPwr_Y.HEV_Output.hev_mode_output = HEVControllerPwr_B.MultiportSwitch;
  HEVControllerPwr_Y.HEV_Output.HEV_PwrDmnd_Output = HEVControllerPwr_B.HEV_PwrDmnd_Output;

  /* SignalConversion generated from: '<Root>/SoftECU_Input' incorporates:
   *  Inport: '<Root>/SoftECU_Input'
   */
  HEVControllerPwr_B.slip_ratio[0] = HEVControllerPwr_U.SoftECU_Input.slip_ratio[0];
  HEVControllerPwr_B.slip_ratio[1] = HEVControllerPwr_U.SoftECU_Input.slip_ratio[1];
  HEVControllerPwr_B.slip_ratio[2] = HEVControllerPwr_U.SoftECU_Input.slip_ratio[2];
  HEVControllerPwr_B.slip_ratio[3] = HEVControllerPwr_U.SoftECU_Input.slip_ratio[3];
  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    /* Update for Atomic SubSystem: '<S4>/ax_estimator' */
    HEVControllerPwr_ax_estimator_Update(HEVControllerPwr_B.veh_speed_vx, &HEVControllerPwr_DW.ax_estimator);

    /* End of Update for SubSystem: '<S4>/ax_estimator' */

    /* Update for Atomic SubSystem: '<S4>/ABS_EBD' */
    HEVControllerPwr_ABS_EBD_Update();

    /* End of Update for SubSystem: '<S4>/ABS_EBD' */

    /* Update for Atomic SubSystem: '<S4>/HCU' */
    HEVControllerPwr_HCU_Update();

    /* End of Update for SubSystem: '<S4>/HCU' */
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&HEVControllerPwr_M))) {
    rt_ertODEUpdateContinuousStates(&(&HEVControllerPwr_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&HEVControllerPwr_M)->Timing.clockTick0)) {
      ++(&HEVControllerPwr_M)->Timing.clockTickH0;
    }

    (&HEVControllerPwr_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&HEVControllerPwr_M)->solverInfo);

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
      (&HEVControllerPwr_M)->Timing.clockTick1++;
      if (!(&HEVControllerPwr_M)->Timing.clockTick1) {
        (&HEVControllerPwr_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void HEVControllerPwr::HEVControllerPwr_derivatives() {
  XDot_HEVControllerPwr_T *_rtXdot;
  _rtXdot = ((XDot_HEVControllerPwr_T *)(&HEVControllerPwr_M)->derivs);

  /* Derivatives for Atomic SubSystem: '<S4>/ax_estimator' */
  HEVControllerPwr_ax_estimator_Deriv(&HEVControllerPwr_B.ax_estimator, &_rtXdot->ax_estimator);

  /* End of Derivatives for SubSystem: '<S4>/ax_estimator' */
}

/* Model initialize function */
void HEVControllerPwr::initialize() {
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* non-finite (run-time) assignments */
  HEVControllerPwr_P.HEV_Pwr_Calculation.Saturation_UpperSat = rtInf;
  HEVControllerPwr_P.HEV_Pwr_Calculation.Saturation_UpperSat_l = rtInf;
  HEVControllerPwr_P.HEV_Pwr_Calculation.Saturation1_UpperSat = rtInf;
  HEVControllerPwr_P.EV_Pwr_Calculation.Saturation_UpperSat = rtInf;
  HEVControllerPwr_P.EV_Pwr_Calculation.Saturation1_UpperSat = rtInf;
  HEVControllerPwr_P.EV_Pwr_Calculation.Saturation1_UpperSat_o = rtInf;
  HEVControllerPwr_P.EV_Pwr_Calculation.Saturation2_UpperSat = rtInf;

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&HEVControllerPwr_M)->solverInfo, &(&HEVControllerPwr_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&HEVControllerPwr_M)->solverInfo, &rtmGetTPtr((&HEVControllerPwr_M)));
    rtsiSetStepSizePtr(&(&HEVControllerPwr_M)->solverInfo, &(&HEVControllerPwr_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&HEVControllerPwr_M)->solverInfo, &(&HEVControllerPwr_M)->derivs);
    rtsiSetContStatesPtr(&(&HEVControllerPwr_M)->solverInfo, (real_T **)&(&HEVControllerPwr_M)->contStates);
    rtsiSetNumContStatesPtr(&(&HEVControllerPwr_M)->solverInfo, &(&HEVControllerPwr_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&HEVControllerPwr_M)->solverInfo,
                                    &(&HEVControllerPwr_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&HEVControllerPwr_M)->solverInfo,
                                       &(&HEVControllerPwr_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&HEVControllerPwr_M)->solverInfo,
                                      &(&HEVControllerPwr_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&HEVControllerPwr_M)->solverInfo, (&rtmGetErrorStatus((&HEVControllerPwr_M))));
    rtsiSetRTModelPtr(&(&HEVControllerPwr_M)->solverInfo, (&HEVControllerPwr_M));
  }

  rtsiSetSimTimeStep(&(&HEVControllerPwr_M)->solverInfo, MAJOR_TIME_STEP);
  (&HEVControllerPwr_M)->intgData.y = (&HEVControllerPwr_M)->odeY;
  (&HEVControllerPwr_M)->intgData.f[0] = (&HEVControllerPwr_M)->odeF[0];
  (&HEVControllerPwr_M)->intgData.f[1] = (&HEVControllerPwr_M)->odeF[1];
  (&HEVControllerPwr_M)->intgData.f[2] = (&HEVControllerPwr_M)->odeF[2];
  (&HEVControllerPwr_M)->intgData.f[3] = (&HEVControllerPwr_M)->odeF[3];
  (&HEVControllerPwr_M)->contStates = ((X_HEVControllerPwr_T *)&HEVControllerPwr_X);
  rtsiSetSolverData(&(&HEVControllerPwr_M)->solverInfo, static_cast<void *>(&(&HEVControllerPwr_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&HEVControllerPwr_M)->solverInfo, false);
  rtsiSetSolverName(&(&HEVControllerPwr_M)->solverInfo, "ode4");
  rtmSetTPtr((&HEVControllerPwr_M), &(&HEVControllerPwr_M)->Timing.tArray[0]);
  (&HEVControllerPwr_M)->Timing.stepSize0 = 0.001;

  /* SystemInitialize for Atomic SubSystem: '<S4>/ax_estimator' */
  HEVControllerPwr_ax_estimator_Init(&HEVControllerPwr_DW.ax_estimator, &HEVControllerPwr_P.ax_estimator,
                                     &HEVControllerPwr_X.ax_estimator);

  /* End of SystemInitialize for SubSystem: '<S4>/ax_estimator' */

  /* SystemInitialize for Atomic SubSystem: '<S4>/ABS_EBD' */
  HEVControllerPwr_ABS_EBD_Init();

  /* End of SystemInitialize for SubSystem: '<S4>/ABS_EBD' */

  /* SystemInitialize for Atomic SubSystem: '<S4>/HCU' */
  HEVControllerPwr_HCU_Init();

  /* End of SystemInitialize for SubSystem: '<S4>/HCU' */

  /* SystemInitialize for Chart: '<S11>/PwrCal' incorporates:
   *  SubSystem: '<S22>/EV_Pwr_Calculation'
   */
  HEVControllerPwr_EV_Pwr_Calculation_Init(&HEVControllerPwr_B.EV_Pwr_Calculation,
                                           &HEVControllerPwr_P.EV_Pwr_Calculation);

  /* SystemInitialize for Chart: '<S11>/PwrCal' incorporates:
   *  SubSystem: '<S22>/Engine_Pwr_Calculation'
   */
  HEVControllerPwr_Engine_Pwr_Calculation_Init(&HEVControllerPwr_B.Engine_Pwr_Calculation,
                                               &HEVControllerPwr_P.Engine_Pwr_Calculation);

  /* SystemInitialize for Chart: '<S11>/PwrCal' incorporates:
   *  SubSystem: '<S22>/HEV_Pwr_Calculation'
   */
  HEVControllerPwr_HEV_Pwr_Calculation_Init(&HEVControllerPwr_B.HEV_Pwr_Calculation,
                                            &HEVControllerPwr_P.HEV_Pwr_Calculation);

  /* SystemInitialize for Chart: '<S11>/PwrCal' incorporates:
   *  SubSystem: '<S22>/ReGen_Pwr_Calculation'
   */
  HEVControllerPwr_ReGen_Pwr_Calculation_Init(&HEVControllerPwr_B.ReGen_Pwr_Calculation,
                                              &HEVControllerPwr_P.ReGen_Pwr_Calculation);
}

/* Model terminate function */
void HEVControllerPwr::terminate() { /* (no terminate code required) */
}

/* Constructor */
HEVControllerPwr::HEVControllerPwr()
    : HEVControllerPwr_U(),
      HEVControllerPwr_Y(),
      HEVControllerPwr_B(),
      HEVControllerPwr_DW(),
      HEVControllerPwr_X(),
      HEVControllerPwr_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
HEVControllerPwr::~HEVControllerPwr() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_HEVControllerPwr_T *HEVControllerPwr::getRTM() { return (&HEVControllerPwr_M); }
