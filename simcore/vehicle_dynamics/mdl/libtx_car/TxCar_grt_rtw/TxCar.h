/*
 * TxCar.h
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

#ifndef RTW_HEADER_TxCar_h_
#define RTW_HEADER_TxCar_h_
#include <cstring>
#include "TxCar_types.h"
#include "battery_sfunction_cgen_wrapper.h"
#include "brakehydraulic_sfunc_cgen_wrapper.h"
#include "cartype_sfunction_cgen_wrapper.h"
#include "chassis_sfunc_cgen_wrapper.h"
#include "driveline_sfunction_cgen_wrapper.h"
#include "dynamic_steer_sfunc_cgen_wrapper.h"
#include "ecu_sfunction_cgen_wrapper.h"
#include "engine_sfunction_cgen_wrapper.h"
#include "front_motor_sfunction_cgen_wrapper.h"
#include "rear_motor_sfunction_cgen_wrapper.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rtwtypes.h"

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
struct B_TxCar_T {
  powecu_in BusCreator1;  /* '<S42>/Bus Creator1' */
  driveline_in driveline; /* '<S16>/Bus Creator' */
  Chassis_DriveLine_Input BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1;
  powecu_out SFunction; /* '<S41>/S-Function' */
  Env_Bus_Output BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1;
  driveline_out SFunction_a; /* '<S14>/S-Function' */
  Ctrl_Input BusConversion_InsertedFor_Steer_Mapped_at_inport_0_BusCreator1;
  Steer_Output Steer_Mapped;       /* '<S46>/Steer_Mapped' */
  cartype_parsing_out SFunction_e; /* '<S28>/S-Function' */
  batt_out SFunction_k;            /* '<S27>/S-Function' */
  ENGINE_OUTPUTBUS SFunction_f;    /* '<S29>/S-Function' */
  moter_in BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1;
  moter_in BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m;
  motor_out SFunction_h;  /* '<S30>/S-Function' */
  motor_out SFunction_ep; /* '<S31>/S-Function' */
  batt_in BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p;
  ENGINE_INPUTBUS BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_f;
  real_T ax;                 /* '<S42>/Gain' */
  real_T acc_cmd;            /* '<S16>/Gain5' */
  real_T brake_cmd_0_1;      /* '<S16>/Gain1' */
  real_T wheel_spd_rad_s[4]; /* '<S16>/Gain2' */
  real_T abs_flag;           /* '<S16>/Gain3' */
  real_T engine_trq_Nm;      /* '<S16>/Gain4' */
  real_T mot_front_trq_Nm;   /* '<S16>/Gain6' */
  real_T mot_rear_trq_Nm;    /* '<S16>/Gain7' */
  real_T Divide;             /* '<S15>/Divide' */
  real_T throttle_01;        /* '<S33>/Multiport Switch2' */
  boolean_T diff_prs_cmd[4]; /* '<S16>/Data Type Conversion' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_TxCar_T {
  Steer_Output Memory_PreviousInput; /* '<S9>/Memory' */
  real_T Memory_1_PreviousInput[4];  /* '<S7>/Memory' */
  real_T Memory_2_PreviousInput[4];  /* '<S7>/Memory' */
  real_T Memory_3_PreviousInput[4];  /* '<S7>/Memory' */
  real_T Memory_5_PreviousInput;     /* '<S25>/Memory' */
  real_T Memory_1_PreviousInput_i;   /* '<S25>/Memory' */
  real_T Memory_2_PreviousInput_p;   /* '<S25>/Memory' */
  real_T Memory_3_PreviousInput_k;   /* '<S25>/Memory' */
  real_T Memory_4_PreviousInput;     /* '<S25>/Memory' */
  real_T Memory_6_PreviousInput;     /* '<S25>/Memory' */
  real_T Memory2_PreviousInput;      /* '<S26>/Memory2' */
  real_T Memory1_PreviousInput;      /* '<S26>/Memory1' */
  real_T Memory_PreviousInput_a;     /* '<S26>/Memory' */
  void* Chassis_env_inBUS;           /* '<S1>/Chassis' */
  void* Chassis_driveline_inBUS;     /* '<S1>/Chassis' */
  void* Chassis_body_outBUS;         /* '<S1>/Chassis' */
  void* Chassis_wheel_outBUS;        /* '<S1>/Chassis' */
  void* Chassis_susp_outBUS;         /* '<S1>/Chassis' */
  void* SFunction_ecu_inBUS;         /* '<S41>/S-Function' */
  void* SFunction_ecu_outBUS;        /* '<S41>/S-Function' */
  void* SFunction_DrivelineInBUS;    /* '<S14>/S-Function' */
  void* SFunction_softecu_outputBUS; /* '<S14>/S-Function' */
  void* SFunction_DrivelineOutBUS;   /* '<S14>/S-Function' */
  void* SFunction_CarTypeOutBUS;     /* '<S28>/S-Function' */
  void* SFunction_EngineInBUS;       /* '<S29>/S-Function' */
  void* SFunction_EngineOutBUS;      /* '<S29>/S-Function' */
  void* SFunction_BattInBUS;         /* '<S27>/S-Function' */
  void* SFunction_BattOutBUS;        /* '<S27>/S-Function' */
  void* SFunction_MotorInBUS;        /* '<S30>/S-Function' */
  void* SFunction_MotorOutBUS;       /* '<S30>/S-Function' */
  void* SFunction_MotorInBUS_p;      /* '<S31>/S-Function' */
  void* SFunction_MotorOutBUS_g;     /* '<S31>/S-Function' */
  void* Steer_Mapped_driver_inBUS;   /* '<S46>/Steer_Mapped' */
  void* Steer_Mapped_body_inBUS;     /* '<S46>/Steer_Mapped' */
  void* Steer_Mapped_susp_inBUS;     /* '<S46>/Steer_Mapped' */
  void* Steer_Mapped_wheel_inBUS;    /* '<S46>/Steer_Mapped' */
  void* Steer_Mapped_steer_outBUS;   /* '<S46>/Steer_Mapped' */
};

/* Continuous states (default storage) */
struct X_TxCar_T {
  real_T Integrator1_CSTATE; /* '<S15>/Integrator1' */
};

/* State derivatives (default storage) */
struct XDot_TxCar_T {
  real_T Integrator1_CSTATE; /* '<S15>/Integrator1' */
};

/* State disabled  */
struct XDis_TxCar_T {
  boolean_T Integrator1_CSTATE; /* '<S15>/Integrator1' */
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
struct ExtU_TxCar_T {
  Ctrl_Input DriverIn;    /* '<Root>/DriverIn' */
  Env_Bus_Mu Friction;    /* '<Root>/Friction' */
  Env_Bus_GroundZ Height; /* '<Root>/Height' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_TxCar_T {
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
struct P_TxCar_T_ {
  real_T CompareToConstant_const; /* Mask Parameter: CompareToConstant_const
                                   * Referenced by: '<S22>/Constant'
                                   */
  real_T EngSpdAct_wc;            /* Mask Parameter: EngSpdAct_wc
                                   * Referenced by: '<S15>/Constant'
                                   */
  Steer_Output Memory_InitialCondition;
  /* Computed Parameter: Memory_InitialCondition
   * Referenced by: '<S9>/Memory'
   */
  real_T const_mu_Value[4];           /* Expression: 0.9*ones(1,4)
                                       * Referenced by: '<S21>/const_mu'
                                       */
  real_T Constant3_Value;             /* Expression: 0
                                       * Referenced by: '<S26>/Constant3'
                                       */
  real_T Constant4_Value;             /* Expression: 0
                                       * Referenced by: '<S26>/Constant4'
                                       */
  real_T Constant5_Value;             /* Expression: 0
                                       * Referenced by: '<S38>/Constant5'
                                       */
  real_T none_Value;                  /* Expression: 0
                                       * Referenced by: '<S33>/none'
                                       */
  real_T Constant1_Value;             /* Expression: 0
                                       * Referenced by: '<S36>/Constant1'
                                       */
  real_T Constant3_Value_l;           /* Expression: 0
                                       * Referenced by: '<S4>/Constant3'
                                       */
  real_T Constant4_Value_c;           /* Expression: 0
                                       * Referenced by: '<S4>/Constant4'
                                       */
  real_T Constant5_Value_m;           /* Expression: 0
                                       * Referenced by: '<S4>/Constant5'
                                       */
  real_T Memory_1_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S7>/Memory'
                                       */
  real_T Memory_2_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S7>/Memory'
                                       */
  real_T Memory_3_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S7>/Memory'
                                       */
  real_T Gain_Gain;                   /* Expression: 1
                                       * Referenced by: '<S42>/Gain'
                                       */
  real_T Integrator1_IC;              /* Expression: 0
                                       * Referenced by: '<S15>/Integrator1'
                                       */
  real_T Memory_5_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S25>/Memory'
                                       */
  real_T Constant1_Value_k;           /* Expression: 0
                                       * Referenced by: '<S45>/Constant1'
                                       */
  real_T Gain5_Gain;                  /* Expression: 1
                                       * Referenced by: '<S16>/Gain5'
                                       */
  real_T Constant1_Value_f;           /* Expression: 0
                                       * Referenced by: '<S16>/Constant1'
                                       */
  real_T Gain1_Gain;                  /* Expression: 1
                                       * Referenced by: '<S16>/Gain1'
                                       */
  real_T Gain2_Gain;                  /* Expression: 1
                                       * Referenced by: '<S16>/Gain2'
                                       */
  real_T Constant2_Value;             /* Expression: 0
                                       * Referenced by: '<S16>/Constant2'
                                       */
  real_T Gain3_Gain;                  /* Expression: 1
                                       * Referenced by: '<S16>/Gain3'
                                       */
  real_T Memory_1_InitialCondition_l; /* Expression: 0
                                       * Referenced by: '<S25>/Memory'
                                       */
  real_T Gain4_Gain;                  /* Expression: 1
                                       * Referenced by: '<S16>/Gain4'
                                       */
  real_T N_Value;                     /* Expression: 0
                                       * Referenced by: '<S17>/N'
                                       */
  real_T D_Value;                     /* Expression: 1
                                       * Referenced by: '<S17>/D'
                                       */
  real_T R_Value;                     /* Expression: -1
                                       * Referenced by: '<S17>/R'
                                       */
  real_T P_Value;                     /* Expression: 0
                                       * Referenced by: '<S17>/P'
                                       */
  real_T Constant_Value;              /* Expression: 0
                                       * Referenced by: '<S45>/Constant'
                                       */
  real_T Memory_2_InitialCondition_l; /* Expression: 0
                                       * Referenced by: '<S25>/Memory'
                                       */
  real_T Gain6_Gain;                  /* Expression: 1
                                       * Referenced by: '<S16>/Gain6'
                                       */
  real_T Memory_3_InitialCondition_l; /* Expression: 0
                                       * Referenced by: '<S25>/Memory'
                                       */
  real_T Gain7_Gain;                  /* Expression: 1
                                       * Referenced by: '<S16>/Gain7'
                                       */
  real_T Saturation_UpperSat;         /* Expression: 5000
                                       * Referenced by: '<S10>/Saturation'
                                       */
  real_T Saturation_LowerSat;         /* Expression: -5000
                                       * Referenced by: '<S10>/Saturation'
                                       */
  real_T Gain2_Gain_l;                /* Expression: 1
                                       * Referenced by: '<S40>/Gain2'
                                       */
  real_T Memory_4_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S25>/Memory'
                                       */
  real_T Memory_6_InitialCondition;   /* Expression: 0
                                       * Referenced by: '<S25>/Memory'
                                       */
  real_T Zero_Value;                  /* Expression: 0
                                       * Referenced by: '<S28>/Zero'
                                       */
  real_T Gain1_Gain_f;                /* Expression: 1
                                       * Referenced by: '<S43>/Gain1'
                                       */
  real_T none_Value_d;                /* Expression: 0
                                       * Referenced by: '<S32>/none'
                                       */
  real_T Gain_Gain_d;                 /* Expression: 60/2/3.1415
                                       * Referenced by: '<S32>/Gain'
                                       */
  real_T Memory2_InitialCondition;    /* Expression: 0
                                       * Referenced by: '<S26>/Memory2'
                                       */
  real_T FD_Value;                    /* Expression: 1
                                       * Referenced by: '<S35>/FD'
                                       */
  real_T uWD_Value;                   /* Expression: 3
                                       * Referenced by: '<S35>/4WD'
                                       */
  real_T Gain2_Gain_c;                /* Expression: 60/2/3.1415
                                       * Referenced by: '<S35>/Gain2'
                                       */
  real_T Memory1_InitialCondition;    /* Expression: 0
                                       * Referenced by: '<S26>/Memory1'
                                       */
  real_T Gain3_Gain_p;                /* Expression: 1
                                       * Referenced by: '<S26>/Gain3'
                                       */
  real_T Saturation_UpperSat_o;       /* Expression: 1000
                                       * Referenced by: '<S26>/Saturation'
                                       */
  real_T Saturation_LowerSat_o;       /* Expression: -1000
                                       * Referenced by: '<S26>/Saturation'
                                       */
  real_T Constant_Value_n;            /* Expression: 30+273.15
                                       * Referenced by: '<S26>/Constant'
                                       */
  real_T Memory_InitialCondition_e;   /* Expression: 0
                                       * Referenced by: '<S26>/Memory'
                                       */
  real_T RD_Value;                    /* Expression: 2
                                       * Referenced by: '<S37>/RD'
                                       */
  real_T uWD_Value_c;                 /* Expression: 3
                                       * Referenced by: '<S37>/4WD'
                                       */
  real_T Gain4_Gain_p;                /* Expression: 60/2/3.1415
                                       * Referenced by: '<S37>/Gain4'
                                       */
  real_T Constant_Value_p;            /* Expression: 0
                                       * Referenced by: '<S40>/Constant'
                                       */
  real_T Gain_Gain_a;                 /* Expression: 1
                                       * Referenced by: '<S40>/Gain'
                                       */
  real_T Gain1_Gain_o;                /* Expression: 1
                                       * Referenced by: '<S40>/Gain1'
                                       */
  real_T Gain_Gain_m;                 /* Expression: 1
                                       * Referenced by: '<S43>/Gain'
                                       */
  int8_T Chassis_P1[21];              /* Expression: int8(lic_path)
                                       * Referenced by: '<S1>/Chassis'
                                       */
  int8_T Chassis_P2[22];              /* Expression: int8(par_path)
                                       * Referenced by: '<S1>/Chassis'
                                       */
  int8_T SFunction_P1[21];            /* Expression: int8(lic_path)
                                       * Referenced by: '<S41>/S-Function'
                                       */
  int8_T SFunction_P2[22];            /* Expression: int8(par_path)
                                       * Referenced by: '<S41>/S-Function'
                                       */
  int8_T SFunction_P1_m[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S14>/S-Function'
                                       */
  int8_T SFunction_P2_k[22];          /* Expression: int8(par_path)
                                       * Referenced by: '<S14>/S-Function'
                                       */
  int8_T SFunction_P1_f[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S13>/S-Function'
                                       */
  int8_T SFunction_P2_i[22];          /* Expression: int8(par_path)
                                       * Referenced by: '<S13>/S-Function'
                                       */
  int8_T SFunction_P1_l[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S28>/S-Function'
                                       */
  int8_T SFunction_P2_o[22];          /* Expression: int8(par_path)
                                       * Referenced by: '<S28>/S-Function'
                                       */
  int8_T SFunction_P1_k[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S29>/S-Function'
                                       */
  int8_T SFunction_P2_l[22];          /* Expression: int8(par_path)
                                       * Referenced by: '<S29>/S-Function'
                                       */
  int8_T SFunction_P1_mq[21];         /* Expression: int8(lic_path)
                                       * Referenced by: '<S27>/S-Function'
                                       */
  int8_T SFunction_P2_g[22];          /* Expression: int8(par_path)
                                       * Referenced by: '<S27>/S-Function'
                                       */
  int8_T SFunction_P1_h[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S30>/S-Function'
                                       */
  int8_T SFunction_P2_b[22];          /* Expression: int8(par_path)
                                       * Referenced by: '<S30>/S-Function'
                                       */
  int8_T SFunction_P1_g[21];          /* Expression: int8(lic_path)
                                       * Referenced by: '<S31>/S-Function'
                                       */
  int8_T SFunction_P2_ox[22];         /* Expression: int8(par_path)
                                       * Referenced by: '<S31>/S-Function'
                                       */
  int8_T Steer_Mapped_P1[21];         /* Expression: int8(lic_path)
                                       * Referenced by: '<S46>/Steer_Mapped'
                                       */
  int8_T Steer_Mapped_P2[22];         /* Expression: int8(par_path)
                                       * Referenced by: '<S46>/Steer_Mapped'
                                       */
};

/* Real-time Model Data Structure */
struct tag_RTM_TxCar_T {
  const char_T* errorStatus;
  RTWSolverInfo solverInfo;
  X_TxCar_T* contStates;
  int_T* periodicContStateIndices;
  real_T* periodicContStateRanges;
  real_T* derivs;
  XDis_TxCar_T* contStateDisabled;
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

/* Class declaration for model TxCar */
class TxCar final {
  /* public data and function members */
 public:
  /* Copy Constructor */
  TxCar(TxCar const&) = delete;

  /* Assignment Operator */
  TxCar& operator=(TxCar const&) & = delete;

  /* Move Constructor */
  TxCar(TxCar&&) = delete;

  /* Move Assignment Operator */
  TxCar& operator=(TxCar&&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_TxCar_T* getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_TxCar_T* pExtU_TxCar_T) { TxCar_U = *pExtU_TxCar_T; }

  /* Root outports get method */
  const ExtY_TxCar_T& getExternalOutputs() const { return TxCar_Y; }

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  void terminate();

  /* Constructor */
  TxCar();

  /* Destructor */
  ~TxCar();

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_TxCar_T TxCar_U;

  /* External outputs */
  ExtY_TxCar_T TxCar_Y;

  /* Block signals */
  B_TxCar_T TxCar_B;

  /* Block states */
  DW_TxCar_T TxCar_DW;

  /* Tunable parameters */
  static P_TxCar_T TxCar_P;

  /* Block continuous states */
  X_TxCar_T TxCar_X;

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo* si);

  /* Derivatives member function */
  void TxCar_derivatives();

  /* Real-Time Model */
  RT_MODEL_TxCar_T TxCar_M;
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
 * '<Root>' : 'TxCar'
 * '<S1>'   : 'TxCar/Chassis'
 * '<S2>'   : 'TxCar/DriveLine'
 * '<S3>'   : 'TxCar/DriverSelector'
 * '<S4>'   : 'TxCar/Environment'
 * '<S5>'   : 'TxCar/Info'
 * '<S6>'   : 'TxCar/Propulsion'
 * '<S7>'   : 'TxCar/SignalAdapter'
 * '<S8>'   : 'TxCar/SoftECU'
 * '<S9>'   : 'TxCar/steer'
 * '<S10>'  : 'TxCar/DriveLine/DriveLine'
 * '<S11>'  : 'TxCar/DriveLine/Driveline Input'
 * '<S12>'  : 'TxCar/DriveLine/Driveline Output'
 * '<S13>'  : 'TxCar/DriveLine/DriveLine/BrakeHydraulic'
 * '<S14>'  : 'TxCar/DriveLine/DriveLine/DriveLine'
 * '<S15>'  : 'TxCar/DriveLine/DriveLine/EngSpdAct'
 * '<S16>'  : 'TxCar/DriveLine/DriveLine/driveline_in_router'
 * '<S17>'  : 'TxCar/DriveLine/Driveline Input/Subsystem'
 * '<S18>'  : 'TxCar/DriverSelector/CtrlRouter'
 * '<S19>'  : 'TxCar/Environment/Ground Feedback'
 * '<S20>'  : 'TxCar/Environment/Ground Feedback/Constant'
 * '<S21>'  : 'TxCar/Environment/Ground Feedback/Constant/FrictionChecker'
 * '<S22>'  : 'TxCar/Environment/Ground Feedback/Constant/FrictionChecker/Compare To Constant'
 * '<S23>'  : 'TxCar/Environment/Ground Feedback/Constant/FrictionChecker/Subsystem'
 * '<S24>'  : 'TxCar/Propulsion/Input'
 * '<S25>'  : 'TxCar/Propulsion/Out'
 * '<S26>'  : 'TxCar/Propulsion/Power'
 * '<S27>'  : 'TxCar/Propulsion/Power/Battery'
 * '<S28>'  : 'TxCar/Propulsion/Power/CarTypeParsing'
 * '<S29>'  : 'TxCar/Propulsion/Power/Engine'
 * '<S30>'  : 'TxCar/Propulsion/Power/Front Motor'
 * '<S31>'  : 'TxCar/Propulsion/Power/Rear Motor'
 * '<S32>'  : 'TxCar/Propulsion/Power/ice_speed_selector'
 * '<S33>'  : 'TxCar/Propulsion/Power/ice_throttle_selector'
 * '<S34>'  : 'TxCar/Propulsion/Power/mot_trq_spliter'
 * '<S35>'  : 'TxCar/Propulsion/Power/motor_front_speed'
 * '<S36>'  : 'TxCar/Propulsion/Power/motor_front_trq_selector'
 * '<S37>'  : 'TxCar/Propulsion/Power/motor_rear_speed'
 * '<S38>'  : 'TxCar/Propulsion/Power/motor_rear_trq_selector'
 * '<S39>'  : 'TxCar/SoftECU/ActiveDiffCtrl'
 * '<S40>'  : 'TxCar/SoftECU/BrakeSigRouter'
 * '<S41>'  : 'TxCar/SoftECU/ECU'
 * '<S42>'  : 'TxCar/SoftECU/ECUSigRouter'
 * '<S43>'  : 'TxCar/SoftECU/TrqRouter'
 * '<S44>'  : 'TxCar/SoftECU/ActiveDiffCtrl/Active Differential Control'
 * '<S45>'  : 'TxCar/SoftECU/ActiveDiffCtrl/Active Differential Control/No Control'
 * '<S46>'  : 'TxCar/steer/SteerSystem'
 */
#endif /* RTW_HEADER_TxCar_h_ */
