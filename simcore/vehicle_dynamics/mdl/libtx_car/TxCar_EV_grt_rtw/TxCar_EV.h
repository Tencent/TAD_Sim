/*
 * TxCar_EV.h
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

#ifndef RTW_HEADER_TxCar_EV_h_
#define RTW_HEADER_TxCar_EV_h_
#include "TxCar_EV_types.h"
#include "battery_sfunction_cgen_wrapper.h"
#include "brakehydraulic_sfunc_cgen_wrapper.h"
#include "cartype_sfunction_cgen_wrapper.h"
#include "chassis_sfunc_cgen_wrapper.h"
#include "driveline_ev_front_sfunction_cgen_wrapper.h"
#include "driveline_ev_rear_sfunction_cgen_wrapper.h"
#include "dynamic_steer_sfunc_cgen_wrapper.h"
#include "ecu_sfunction_cgen_wrapper.h"
#include "front_motor_sfunction_cgen_wrapper.h"
#include "rear_motor_sfunction_cgen_wrapper.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rtwtypes.h"

extern "C" {

#include "rtGetNaN.h"
}

extern "C" {

#include "rt_nonfinite.h"
}

extern "C" {

#include "rtGetInf.h"
}

#include <cstring>

/* Macros for accessing real-time model data structure */
#ifndef rtmGetContStateDisabled
#  define rtmGetContStateDisabled(rtm) ((rtm)->contStateDisabled)
#endif

#ifndef rtmSetContStateDisabled
#  define rtmSetContStateDisabled(rtm, val) ((rtm)->contStateDisabled = (val))
#endif

#ifndef rtmGetContStates
#  define rtmGetContStates(rtm) ((rtm)->contStates)
#endif

#ifndef rtmSetContStates
#  define rtmSetContStates(rtm, val) ((rtm)->contStates = (val))
#endif

#ifndef rtmGetContTimeOutputInconsistentWithStateAtMajorStepFlag
#  define rtmGetContTimeOutputInconsistentWithStateAtMajorStepFlag(rtm) ((rtm)->CTOutputIncnstWithState)
#endif

#ifndef rtmSetContTimeOutputInconsistentWithStateAtMajorStepFlag
#  define rtmSetContTimeOutputInconsistentWithStateAtMajorStepFlag(rtm, val) ((rtm)->CTOutputIncnstWithState = (val))
#endif

#ifndef rtmGetDerivCacheNeedsReset
#  define rtmGetDerivCacheNeedsReset(rtm) ((rtm)->derivCacheNeedsReset)
#endif

#ifndef rtmSetDerivCacheNeedsReset
#  define rtmSetDerivCacheNeedsReset(rtm, val) ((rtm)->derivCacheNeedsReset = (val))
#endif

#ifndef rtmGetIntgData
#  define rtmGetIntgData(rtm) ((rtm)->intgData)
#endif

#ifndef rtmSetIntgData
#  define rtmSetIntgData(rtm, val) ((rtm)->intgData = (val))
#endif

#ifndef rtmGetOdeF
#  define rtmGetOdeF(rtm) ((rtm)->odeF)
#endif

#ifndef rtmSetOdeF
#  define rtmSetOdeF(rtm, val) ((rtm)->odeF = (val))
#endif

#ifndef rtmGetOdeY
#  define rtmGetOdeY(rtm) ((rtm)->odeY)
#endif

#ifndef rtmSetOdeY
#  define rtmSetOdeY(rtm, val) ((rtm)->odeY = (val))
#endif

#ifndef rtmGetPeriodicContStateIndices
#  define rtmGetPeriodicContStateIndices(rtm) ((rtm)->periodicContStateIndices)
#endif

#ifndef rtmSetPeriodicContStateIndices
#  define rtmSetPeriodicContStateIndices(rtm, val) ((rtm)->periodicContStateIndices = (val))
#endif

#ifndef rtmGetPeriodicContStateRanges
#  define rtmGetPeriodicContStateRanges(rtm) ((rtm)->periodicContStateRanges)
#endif

#ifndef rtmSetPeriodicContStateRanges
#  define rtmSetPeriodicContStateRanges(rtm, val) ((rtm)->periodicContStateRanges = (val))
#endif

#ifndef rtmGetZCCacheNeedsReset
#  define rtmGetZCCacheNeedsReset(rtm) ((rtm)->zCCacheNeedsReset)
#endif

#ifndef rtmSetZCCacheNeedsReset
#  define rtmSetZCCacheNeedsReset(rtm, val) ((rtm)->zCCacheNeedsReset = (val))
#endif

#ifndef rtmGetdX
#  define rtmGetdX(rtm) ((rtm)->derivs)
#endif

#ifndef rtmSetdX
#  define rtmSetdX(rtm, val) ((rtm)->derivs = (val))
#endif

#ifndef rtmGetErrorStatus
#  define rtmGetErrorStatus(rtm) ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#  define rtmSetErrorStatus(rtm, val) ((rtm)->errorStatus = (val))
#endif

#ifndef rtmGetStopRequested
#  define rtmGetStopRequested(rtm) ((rtm)->Timing.stopRequestedFlag)
#endif

#ifndef rtmSetStopRequested
#  define rtmSetStopRequested(rtm, val) ((rtm)->Timing.stopRequestedFlag = (val))
#endif

#ifndef rtmGetStopRequestedPtr
#  define rtmGetStopRequestedPtr(rtm) (&((rtm)->Timing.stopRequestedFlag))
#endif

#ifndef rtmGetT
#  define rtmGetT(rtm) (rtmGetTPtr((rtm))[0])
#endif

#ifndef rtmGetTPtr
#  define rtmGetTPtr(rtm) ((rtm)->Timing.t)
#endif

/* Block signals (default storage) */
struct B_TxCar_EV_T {
  powecu_in BusCreator1;  /* '<S44>/Bus Creator1' */
  driveline_in driveline; /* '<S19>/Bus Creator' */
  Chassis_DriveLine_Input BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1;
  powecu_out SFunction; /* '<S43>/S-Function' */
  Env_Bus_Output BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1;
  driveline_out SFunction_m;     /* '<S15>/S-Function' */
  driveline_out SFunction_o;     /* '<S16>/S-Function' */
  driveline_out MultiportSwitch; /* '<S11>/Multiport Switch' */
  Ctrl_Input BusConversion_InsertedFor_Steer_Mapped_at_inport_0_BusCreator1;
  cartype_parsing_out SFunction_e; /* '<S1>/S-Function' */
  batt_out SFunction_k;            /* '<S31>/S-Function' */
  moter_in BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1;
  moter_in BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m;
  motor_out SFunction_h;  /* '<S32>/S-Function' */
  motor_out SFunction_ep; /* '<S33>/S-Function' */
  batt_in BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p;
  real_T ax;                 /* '<S44>/Gain' */
  real_T acc_cmd;            /* '<S19>/Gain5' */
  real_T brake_cmd_0_1;      /* '<S19>/Gain1' */
  real_T wheel_spd_rad_s[4]; /* '<S19>/Gain2' */
  real_T abs_flag;           /* '<S19>/Gain3' */
  real_T engine_trq_Nm;      /* '<S19>/Gain4' */
  real_T mot_front_trq_Nm;   /* '<S19>/Gain6' */
  real_T mot_rear_trq_Nm;    /* '<S19>/Gain7' */
  real_T Divide;             /* '<S17>/Divide' */
  boolean_T diff_prs_cmd[4]; /* '<S19>/Data Type Conversion' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_TxCar_EV_T {
  real_T Memory_1_PreviousInput[4];  /* '<S8>/Memory' */
  real_T Memory_2_PreviousInput[4];  /* '<S8>/Memory' */
  real_T Memory_3_PreviousInput[4];  /* '<S8>/Memory' */
  real_T Memory_5_PreviousInput;     /* '<S28>/Memory' */
  real_T Memory_1_PreviousInput_i;   /* '<S28>/Memory' */
  real_T Memory_2_PreviousInput_p;   /* '<S28>/Memory' */
  real_T Memory_3_PreviousInput_k;   /* '<S28>/Memory' */
  real_T Memory_4_PreviousInput;     /* '<S28>/Memory' */
  real_T Memory_6_PreviousInput;     /* '<S28>/Memory' */
  real_T Memory1_PreviousInput;      /* '<S29>/Memory1' */
  void* Chassis_env_inBUS;           /* '<S2>/Chassis' */
  void* Chassis_driveline_inBUS;     /* '<S2>/Chassis' */
  void* Chassis_body_outBUS;         /* '<S2>/Chassis' */
  void* Chassis_wheel_outBUS;        /* '<S2>/Chassis' */
  void* Chassis_susp_outBUS;         /* '<S2>/Chassis' */
  void* SFunction_ecu_inBUS;         /* '<S43>/S-Function' */
  void* SFunction_ecu_outBUS;        /* '<S43>/S-Function' */
  void* SFunction_DrivelineInBUS;    /* '<S15>/S-Function' */
  void* SFunction_DrivelineOutBUS;   /* '<S15>/S-Function' */
  void* SFunction_DrivelineInBUS_e;  /* '<S16>/S-Function' */
  void* SFunction_DrivelineOutBUS_c; /* '<S16>/S-Function' */
  void* SFunction_CarTypeOutBUS;     /* '<S1>/S-Function' */
  void* SFunction_BattInBUS;         /* '<S31>/S-Function' */
  void* SFunction_BattOutBUS;        /* '<S31>/S-Function' */
  void* SFunction_MotorInBUS;        /* '<S32>/S-Function' */
  void* SFunction_MotorOutBUS;       /* '<S32>/S-Function' */
  void* SFunction_MotorInBUS_p;      /* '<S33>/S-Function' */
  void* SFunction_MotorOutBUS_g;     /* '<S33>/S-Function' */
  void* Steer_Mapped_driver_inBUS;   /* '<S48>/Steer_Mapped' */
  void* Steer_Mapped_body_inBUS;     /* '<S48>/Steer_Mapped' */
  void* Steer_Mapped_susp_inBUS;     /* '<S48>/Steer_Mapped' */
  void* Steer_Mapped_wheel_inBUS;    /* '<S48>/Steer_Mapped' */
  void* Steer_Mapped_steer_outBUS;   /* '<S48>/Steer_Mapped' */
};

/* Continuous states (default storage) */
struct X_TxCar_EV_T {
  real_T Integrator1_CSTATE; /* '<S17>/Integrator1' */
};

/* State derivatives (default storage) */
struct XDot_TxCar_EV_T {
  real_T Integrator1_CSTATE; /* '<S17>/Integrator1' */
};

/* State disabled  */
struct XDis_TxCar_EV_T {
  boolean_T Integrator1_CSTATE; /* '<S17>/Integrator1' */
};

#ifndef ODE4_INTG
#  define ODE4_INTG

/* ODE4 Integration Data */
struct ODE4_IntgData {
  real_T* y;    /* output */
  real_T* f[4]; /* derivatives */
};

#endif

/* External inputs (root inport signals with default storage) */
struct ExtU_TxCar_EV_T {
  Ctrl_Input DriverIn;    /* '<Root>/DriverIn' */
  Env_Bus_Mu Friction;    /* '<Root>/Friction' */
  Env_Bus_GroundZ Height; /* '<Root>/Height' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_TxCar_EV_T {
  real_T DriveLineBus_Out_AxlTrq[4]; /* '<Root>/DriveLineBus_Out_AxlTrq' */
  real_T DriveLineBus_Out_BrkPrs[4]; /* '<Root>/DriveLineBus_Out_BrkPrs' */
  real_T DriveLineBus_Out_EngSpd;    /* '<Root>/DriveLineBus_Out_EngSpd' */
  real_T DriveLineBus_Out_PropShftSpd;
  /* '<Root>/DriveLineBus_Out_PropShftSpd' */
  real_T DriveLineBus_Out_Gear; /* '<Root>/DriveLineBus_Out_Gear' */
  real_T DriveLineBus_Out_FrontMotSpd;
  /* '<Root>/DriveLineBus_Out_FrontMotSpd' */
  real_T DriveLineBus_Out_RearMotSpd; /* '<Root>/DriveLineBus_Out_RearMotSpd' */
  real_T PowerBus_Out_eng_trqNm;      /* '<Root>/PowerBus_Out_eng_trq[Nm]' */
  real_T PowerBus_Out_mot_front_trqNm;
  /* '<Root>/PowerBus_Out_mot_front_trq[Nm]' */
  real_T PowerBus_Out_mot_rear_trqNm;
  /* '<Root>/PowerBus_Out_mot_rear_trq[Nm]' */
  real_T PowerBus_Out_battery_info_batt_volt_V;
  /* '<Root>/PowerBus_Out_battery_info_batt_volt_V' */
  real_T PowerBus_Out_battery_info_batt_soc_null;
  /* '<Root>/PowerBus_Out_battery_info_batt_soc_null' */
  real_T PowerBus_Out_current;         /* '<Root>/PowerBus_Out_current' */
  Steer_Output SteerBus_Out;           /* '<Root>/SteerBus_Out' */
  real_T SoftEcu_Out_ECU_TrqCmd;       /* '<Root>/SoftEcu_Out_ECU_TrqCmd' */
  real_T SoftEcu_Out_ECU_throttle_01;  /* '<Root>/SoftEcu_Out_ECU_throttle_01' */
  real_T SoftEcu_Out_Brake_ABSFlag[4]; /* '<Root>/SoftEcu_Out_Brake_ABSFlag' */
  real_T SoftEcu_Out_Brake_BrkPrsCmd01;
  /* '<Root>/SoftEcu_Out_Brake_BrkPrsCmd[0-1]' */
  real_T SoftEcu_Out_Brake_brake_pressure_Pa[4];
  /* '<Root>/SoftEcu_Out_Brake_brake_pressure_Pa' */
  real_T SoftEcu_Out_Brake_TCSFlag; /* '<Root>/SoftEcu_Out_Brake_TCSFlag' */
  real_T SoftEcu_Out_Diff_DiffPrsCmd[4];
  /* '<Root>/SoftEcu_Out_Diff_DiffPrsCmd' */
  Vehicle_Body_Output Body; /* '<Root>/Body' */
  Susp_Output Susp;         /* '<Root>/Susp' */
  Wheels_Output WheelsOut;  /* '<Root>/WheelsOut' */
};

/* Parameters (default storage) */
struct P_TxCar_EV_T_ {
  real_T CompareToConstant_const;     /* Mask Parameter: CompareToConstant_const
                                       * Referenced by: '<S25>/Constant'
                                       */
  real_T EngSpdAct_wc;                /* Mask Parameter: EngSpdAct_wc
                                       * Referenced by: '<S17>/Constant'
                                       */
  real_T const_mu_Value[4];           /* Expression: 0.9*ones(1,4)
                                       * Referenced by: '<S24>/const_mu'
                                       */
  real_T Zero_If_RWD_Value;           /* Expression: 0
                                       * Referenced by: '<S29>/Zero_If_RWD'
                                       */
  real_T Zero_FWD_Value;              /* Expression: 0
                                       * Referenced by: '<S29>/Zero_FWD'
                                       */
  real_T Constant5_Value;             /* Expression: 0
                                       * Referenced by: '<S39>/Constant5'
                                       */
  real_T Constant1_Value;             /* Expression: 0
                                       * Referenced by: '<S37>/Constant1'
                                       */
  real_T Constant3_Value;             /* Expression: 0
                                       * Referenced by: '<S5>/Constant3'
                                       */
  real_T Constant4_Value;             /* Expression: 0
                                       * Referenced by: '<S5>/Constant4'
                                       */
  real_T Constant5_Value_m;           /* Expression: 0
                                       * Referenced by: '<S5>/Constant5'
                                       */
  real_T Memory_1_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S8>/Memory'
                                       */
  real_T Memory_2_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S8>/Memory'
                                       */
  real_T Memory_3_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S8>/Memory'
                                       */
  real_T Gain_Gain;                   /* Expression: 9.81
                                       * Referenced by: '<S44>/Gain'
                                       */
  real_T Integrator1_IC;              /* Expression: 0
                                       * Referenced by: '<S17>/Integrator1'
                                       */
  real_T Memory_5_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S28>/Memory'
                                       */
  real_T Constant1_Value_k;           /* Expression: 0
                                       * Referenced by: '<S47>/Constant1'
                                       */
  real_T Gain5_Gain;                  /* Expression: 1
                                       * Referenced by: '<S19>/Gain5'
                                       */
  real_T Constant1_Value_f;           /* Expression: 0
                                       * Referenced by: '<S19>/Constant1'
                                       */
  real_T Gain1_Gain;                  /* Expression: 1
                                       * Referenced by: '<S19>/Gain1'
                                       */
  real_T Gain2_Gain;                  /* Expression: 1
                                       * Referenced by: '<S19>/Gain2'
                                       */
  real_T Constant2_Value;             /* Expression: 0
                                       * Referenced by: '<S19>/Constant2'
                                       */
  real_T Gain3_Gain;                  /* Expression: 1
                                       * Referenced by: '<S19>/Gain3'
                                       */
  real_T Memory_1_InitialCondition_l; /* Expression: 0
                                       * Referenced by: '<S28>/Memory'
                                       */
  real_T Gain4_Gain;                  /* Expression: 1
                                       * Referenced by: '<S19>/Gain4'
                                       */
  real_T N_Value;                     /* Expression: 0
                                       * Referenced by: '<S20>/N'
                                       */
  real_T D_Value;                     /* Expression: 1
                                       * Referenced by: '<S20>/D'
                                       */
  real_T R_Value;                     /* Expression: -1
                                       * Referenced by: '<S20>/R'
                                       */
  real_T P_Value;                     /* Expression: 0
                                       * Referenced by: '<S20>/P'
                                       */
  real_T Constant_Value;              /* Expression: 0
                                       * Referenced by: '<S47>/Constant'
                                       */
  real_T Memory_2_InitialCondition_l; /* Expression: 0
                                       * Referenced by: '<S28>/Memory'
                                       */
  real_T Gain6_Gain;                  /* Expression: 1
                                       * Referenced by: '<S19>/Gain6'
                                       */
  real_T Memory_3_InitialCondition_l; /* Expression: 0
                                       * Referenced by: '<S28>/Memory'
                                       */
  real_T Gain7_Gain;                  /* Expression: 1
                                       * Referenced by: '<S19>/Gain7'
                                       */
  real_T Zero_Value;                  /* Expression: 0
                                       * Referenced by: '<S1>/Zero'
                                       */
  real_T Constant_Value_n;            /* Expression: 0
                                       * Referenced by: '<S18>/Constant'
                                       */
  real_T Constant1_Value_o;           /* Expression: 0
                                       * Referenced by: '<S18>/Constant1'
                                       */
  real_T Constant2_Value_d;           /* Expression: 0
                                       * Referenced by: '<S18>/Constant2'
                                       */
  real_T Saturation_UpperSat;         /* Expression: 5000
                                       * Referenced by: '<S11>/Saturation'
                                       */
  real_T Saturation_LowerSat;         /* Expression: -5000
                                       * Referenced by: '<S11>/Saturation'
                                       */
  real_T Gain2_Gain_l;                /* Expression: 1
                                       * Referenced by: '<S42>/Gain2'
                                       */
  real_T Memory_4_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S28>/Memory'
                                       */
  real_T Memory_6_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S28>/Memory'
                                       */
  real_T Gain_Gain_m;                 /* Expression: 1
                                       * Referenced by: '<S45>/Gain'
                                       */
  real_T FD_Value;                    /* Expression: 1
                                       * Referenced by: '<S36>/FD'
                                       */
  real_T uWD_Value;                   /* Expression: 3
                                       * Referenced by: '<S36>/4WD'
                                       */
  real_T Gain2_Gain_c;                /* Expression: 60/2/3.1415
                                       * Referenced by: '<S36>/Gain2'
                                       */
  real_T Memory1_InitialCondition;    /* Expression: 0
                                       * Referenced by: '<S29>/Memory1'
                                       */
  real_T Gain3_Gain_p;                /* Expression: 1
                                       * Referenced by: '<S29>/Gain3'
                                       */
  real_T Saturation_UpperSat_o;       /* Expression: 1000
                                       * Referenced by: '<S29>/Saturation'
                                       */
  real_T Saturation_LowerSat_o;       /* Expression: -1000
                                       * Referenced by: '<S29>/Saturation'
                                       */
  real_T Constant_Value_nf;           /* Expression: 30+273.15
                                       * Referenced by: '<S29>/Constant'
                                       */
  real_T RD_Value;                    /* Expression: 2
                                       * Referenced by: '<S38>/RD'
                                       */
  real_T uWD_Value_c;                 /* Expression: 3
                                       * Referenced by: '<S38>/4WD'
                                       */
  real_T Gain4_Gain_p;                /* Expression: 60/2/3.1415
                                       * Referenced by: '<S38>/Gain4'
                                       */
  real_T Saturation_UpperSat_k;       /* Expression: 45/180.0*pi
                                       * Referenced by: '<S8>/Saturation'
                                       */
  real_T Saturation_LowerSat_l;       /* Expression: -45/180.0*pi
                                       * Referenced by: '<S8>/Saturation'
                                       */
  real_T DeadZone_Start;              /* Expression: -0.1
                                       * Referenced by: '<S40>/Dead Zone'
                                       */
  real_T DeadZone_End;                /* Expression: 0.1
                                       * Referenced by: '<S40>/Dead Zone'
                                       */
  real_T AxleTrqOffset_Value;         /* Expression: 30
                                       * Referenced by: '<S40>/AxleTrqOffset'
                                       */
  real_T Constant_Value_p;            /* Expression: 0
                                       * Referenced by: '<S42>/Constant'
                                       */
  real_T Gain_Gain_a;                 /* Expression: 1
                                       * Referenced by: '<S42>/Gain'
                                       */
  real_T Gain1_Gain_o;                /* Expression: 1
                                       * Referenced by: '<S42>/Gain1'
                                       */
  real_T Gain1_Gain_f;                /* Expression: 1
                                       * Referenced by: '<S45>/Gain1'
                                       */
  int8_T Chassis_P1[21];              /* Expression: int8(lic_path)
                                       * Referenced by: '<S2>/Chassis'
                                       */
  int8_T Chassis_P2[22];              /* Expression: int8(par_path)
                                       * Referenced by: '<S2>/Chassis'
                                       */
  int8_T SFunction_P1[21];            /* Expression: int8(lic_path)
                                       * Referenced by: '<S43>/S-Function'
                                       */
  int8_T SFunction_P2[22];            /* Expression: int8(par_path)
                                       * Referenced by: '<S43>/S-Function'
                                       */
  int8_T SFunction_P1_c[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S15>/S-Function'
                                       */
  int8_T SFunction_P2_b[22];          /* Expression: int8(par_path)
                                       * Referenced by: '<S15>/S-Function'
                                       */
  int8_T SFunction_P1_l[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S16>/S-Function'
                                       */
  int8_T SFunction_P2_m[22];          /* Expression: int8(par_path)
                                       * Referenced by: '<S16>/S-Function'
                                       */
  int8_T SFunction_P1_lo[21];         /* Expression: int8(lic_path)
                                       * Referenced by: '<S1>/S-Function'
                                       */
  int8_T SFunction_P2_o[22];          /* Expression: int8(par_path)
                                       * Referenced by: '<S1>/S-Function'
                                       */
  int8_T SFunction_P1_f[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S14>/S-Function'
                                       */
  int8_T SFunction_P2_i[22];          /* Expression: int8(par_path)
                                       * Referenced by: '<S14>/S-Function'
                                       */
  int8_T SFunction_P1_m[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S31>/S-Function'
                                       */
  int8_T SFunction_P2_g[22];          /* Expression: int8(par_path)
                                       * Referenced by: '<S31>/S-Function'
                                       */
  int8_T SFunction_P1_h[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S32>/S-Function'
                                       */
  int8_T SFunction_P2_by[22];         /* Expression: int8(par_path)
                                       * Referenced by: '<S32>/S-Function'
                                       */
  int8_T SFunction_P1_g[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S33>/S-Function'
                                       */
  int8_T SFunction_P2_ox[22];         /* Expression: int8(par_path)
                                       * Referenced by: '<S33>/S-Function'
                                       */
  int8_T Steer_Mapped_P1[21];         /* Expression: int8(lic_path)
                                       * Referenced by: '<S48>/Steer_Mapped'
                                       */
  int8_T Steer_Mapped_P2[22];         /* Expression: int8(par_path)
                                       * Referenced by: '<S48>/Steer_Mapped'
                                       */
};

/* Real-time Model Data Structure */
struct tag_RTM_TxCar_EV_T {
  const char_T* errorStatus;
  RTWSolverInfo solverInfo;
  X_TxCar_EV_T* contStates;
  int_T* periodicContStateIndices;
  real_T* periodicContStateRanges;
  real_T* derivs;
  XDis_TxCar_EV_T* contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[1];
  real_T odeF[4][1];
  ODE4_IntgData intgData;

  /*
   * Sizes:
   * The following substructure contains sizes information
   * for many of the model attributes such as inputs, outputs,
   * dwork, sample times, etc.
   */
  struct {
    int_T numContStates;
    int_T numPeriodicContStates;
    int_T numSampTimes;
  } Sizes;

  /*
   * Timing:
   * The following substructure contains information regarding
   * the timing information for the model.
   */
  struct {
    uint32_T clockTick0;
    uint32_T clockTickH0;
    time_T stepSize0;
    uint32_T clockTick1;
    uint32_T clockTickH1;
    SimTimeStep simTimeStep;
    boolean_T stopRequestedFlag;
    time_T* t;
    time_T tArray[2];
  } Timing;
};

/* Class declaration for model TxCar_EV */
class TxCar_EV final {
  /* public data and function members */
 public:
  /* Copy Constructor */
  TxCar_EV(TxCar_EV const&) = delete;

  /* Assignment Operator */
  TxCar_EV& operator=(TxCar_EV const&) & = delete;

  /* Move Constructor */
  TxCar_EV(TxCar_EV&&) = delete;

  /* Move Assignment Operator */
  TxCar_EV& operator=(TxCar_EV&&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_TxCar_EV_T* getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_TxCar_EV_T* pExtU_TxCar_EV_T) { TxCar_EV_U = *pExtU_TxCar_EV_T; }

  /* Root outports get method */
  const ExtY_TxCar_EV_T& getExternalOutputs() const { return TxCar_EV_Y; }

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  void terminate();

  /* Constructor */
  TxCar_EV();

  /* Destructor */
  ~TxCar_EV();

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_TxCar_EV_T TxCar_EV_U;

  /* External outputs */
  ExtY_TxCar_EV_T TxCar_EV_Y;

  /* Block signals */
  B_TxCar_EV_T TxCar_EV_B;

  /* Block states */
  DW_TxCar_EV_T TxCar_EV_DW;

  /* Tunable parameters */
  static P_TxCar_EV_T TxCar_EV_P;

  /* Block continuous states */
  X_TxCar_EV_T TxCar_EV_X;

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo* si);

  /* Derivatives member function */
  void TxCar_EV_derivatives();

  /* Real-Time Model */
  RT_MODEL_TxCar_EV_T TxCar_EV_M;
};

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'TxCar_EV'
 * '<S1>'   : 'TxCar_EV/CarTypeParsing'
 * '<S2>'   : 'TxCar_EV/Chassis'
 * '<S3>'   : 'TxCar_EV/DriveLine'
 * '<S4>'   : 'TxCar_EV/DriverSelector'
 * '<S5>'   : 'TxCar_EV/Environment'
 * '<S6>'   : 'TxCar_EV/Info'
 * '<S7>'   : 'TxCar_EV/Propulsion'
 * '<S8>'   : 'TxCar_EV/SignalAdapter'
 * '<S9>'   : 'TxCar_EV/SoftECU'
 * '<S10>'  : 'TxCar_EV/steer'
 * '<S11>'  : 'TxCar_EV/DriveLine/DriveLine'
 * '<S12>'  : 'TxCar_EV/DriveLine/Driveline Input'
 * '<S13>'  : 'TxCar_EV/DriveLine/Driveline Output'
 * '<S14>'  : 'TxCar_EV/DriveLine/DriveLine/BrakeHydraulic'
 * '<S15>'  : 'TxCar_EV/DriveLine/DriveLine/DriveLine_EV_Front'
 * '<S16>'  : 'TxCar_EV/DriveLine/DriveLine/DriveLine_EV_Rear'
 * '<S17>'  : 'TxCar_EV/DriveLine/DriveLine/EngSpdAct'
 * '<S18>'  : 'TxCar_EV/DriveLine/DriveLine/SignalRouter'
 * '<S19>'  : 'TxCar_EV/DriveLine/DriveLine/driveline_in_router'
 * '<S20>'  : 'TxCar_EV/DriveLine/Driveline Input/Subsystem'
 * '<S21>'  : 'TxCar_EV/DriverSelector/CtrlRouter'
 * '<S22>'  : 'TxCar_EV/Environment/Ground Feedback'
 * '<S23>'  : 'TxCar_EV/Environment/Ground Feedback/Constant'
 * '<S24>'  : 'TxCar_EV/Environment/Ground Feedback/Constant/FrictionChecker'
 * '<S25>'  : 'TxCar_EV/Environment/Ground Feedback/Constant/FrictionChecker/Compare To Constant'
 * '<S26>'  : 'TxCar_EV/Environment/Ground Feedback/Constant/FrictionChecker/Subsystem'
 * '<S27>'  : 'TxCar_EV/Propulsion/Input'
 * '<S28>'  : 'TxCar_EV/Propulsion/Out'
 * '<S29>'  : 'TxCar_EV/Propulsion/Power'
 * '<S30>'  : 'TxCar_EV/Propulsion/Power/4WD'
 * '<S31>'  : 'TxCar_EV/Propulsion/Power/Battery'
 * '<S32>'  : 'TxCar_EV/Propulsion/Power/Front Motor'
 * '<S33>'  : 'TxCar_EV/Propulsion/Power/Rear Motor'
 * '<S34>'  : 'TxCar_EV/Propulsion/Power/ice_speed_selector'
 * '<S35>'  : 'TxCar_EV/Propulsion/Power/ice_throttle_selector'
 * '<S36>'  : 'TxCar_EV/Propulsion/Power/motor_front_speed'
 * '<S37>'  : 'TxCar_EV/Propulsion/Power/motor_front_trq_selector'
 * '<S38>'  : 'TxCar_EV/Propulsion/Power/motor_rear_speed'
 * '<S39>'  : 'TxCar_EV/Propulsion/Power/motor_rear_trq_selector'
 * '<S40>'  : 'TxCar_EV/SignalAdapter/TrqRouter'
 * '<S41>'  : 'TxCar_EV/SoftECU/ActiveDiffCtrl'
 * '<S42>'  : 'TxCar_EV/SoftECU/BrakeSigRouter'
 * '<S43>'  : 'TxCar_EV/SoftECU/ECU'
 * '<S44>'  : 'TxCar_EV/SoftECU/ECUSigRouter'
 * '<S45>'  : 'TxCar_EV/SoftECU/TrqRouter'
 * '<S46>'  : 'TxCar_EV/SoftECU/ActiveDiffCtrl/Active Differential Control'
 * '<S47>'  : 'TxCar_EV/SoftECU/ActiveDiffCtrl/Active Differential Control/No Control'
 * '<S48>'  : 'TxCar_EV/steer/SteerSystem'
 */
#endif /* RTW_HEADER_TxCar_EV_h_ */
