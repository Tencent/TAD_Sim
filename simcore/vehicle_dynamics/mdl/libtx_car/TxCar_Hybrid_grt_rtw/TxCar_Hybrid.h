/*
 * TxCar_Hybrid.h
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

#ifndef RTW_HEADER_TxCar_Hybrid_h_
#define RTW_HEADER_TxCar_Hybrid_h_
#include "TxCar_Hybrid_types.h"
#include "battery_sfunction_cgen_wrapper.h"
#include "brakehydraulic_sfunc_cgen_wrapper.h"
#include "cartype_sfunction_cgen_wrapper.h"
#include "chassis_sfunc_cgen_wrapper.h"
#include "driveline_ev_rear_sfunction_cgen_wrapper.h"
#include "driveline_ice_sfunction_cgen_wrapper.h"
#include "dynamic_steer_sfunc_cgen_wrapper.h"
#include "engine_sfunction_cgen_wrapper.h"
#include "front_motor_sfunction_cgen_wrapper.h"
#include "hybridparser_sfunction_cgen_wrapper.h"
#include "rear_motor_sfunction_cgen_wrapper.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rtwtypes.h"
#include "softhcu_sfunction_cgen_wrapper.h"
#include "transmission_sfunction_cgen_wrapper.h"

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
struct B_TxCar_Hybrid_T {
  powecu_in BusCreator1;  /* '<S115>/Bus Creator1' */
  driveline_in driveline; /* '<S28>/Bus Creator' */
  driveline_in BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1;
  Chassis_DriveLine_Input BusConversion_InsertedFor_Chassis_at_inport_1_BusCreator1;
  powecu_out SFunction_o1; /* '<S120>/S-Function' */
  Env_Bus_Output BusConversion_InsertedFor_Chassis_at_inport_0_BusCreator1;
  driveline_out SFunction;   /* '<S16>/S-Function' */
  driveline_out BusCreator2; /* '<S22>/Bus Creator2' */
  Ctrl_Input BusConversion_InsertedFor_Steer_Mapped_at_inport_0_BusCreator1;
  cartype_parsing_out SFunction_e; /* '<S1>/S-Function' */
  batt_out SFunction_k;            /* '<S42>/S-Function' */
  ENGINE_OUTPUTBUS SFunction_i;    /* '<S43>/S-Function' */
  moter_in BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_e;
  moter_in BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_m;
  motor_out SFunction_h;  /* '<S45>/S-Function' */
  motor_out SFunction_ep; /* '<S47>/S-Function' */
  batt_in BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_p;
  ENGINE_INPUTBUS BusConversion_InsertedFor_SFunction_at_inport_0_BusCreator1_er;
  real_T ax;                       /* '<S115>/Gain' */
  real_T Integrator1;              /* '<S18>/Integrator1' */
  real_T Integrator1_j;            /* '<S20>/Integrator1' */
  real_T Integrator1_e;            /* '<S25>/Integrator1' */
  real_T acc_cmd;                  /* '<S28>/Gain5' */
  real_T brake_cmd_0_1;            /* '<S28>/Gain1' */
  real_T wheel_spd_rad_s[4];       /* '<S28>/Gain2' */
  real_T abs_flag;                 /* '<S28>/Gain3' */
  real_T engine_trq_Nm;            /* '<S28>/Gain4' */
  real_T mot_front_trq_Nm;         /* '<S28>/Gain6' */
  real_T EnableP2;                 /* '<S6>/S-Function' */
  real_T EnableP3;                 /* '<S6>/S-Function' */
  real_T EnableP4;                 /* '<S6>/S-Function' */
  real_T EngToGenerator_GearRatio; /* '<S6>/S-Function' */
  real_T EngClutch_TimeConst;      /* '<S6>/S-Function' */
  real_T FrontMotGearRatio;        /* '<S6>/S-Function' */
  real_T mot_rear_trq_Nm;          /* '<S28>/Gain7' */
  real_T Memory;                   /* '<S27>/Memory' */
  real_T P3_Trq_Selector;          /* '<S24>/P3_Trq_Selector' */
  real_T TmpSignalConversionAtSFunctionInport3[4];
  real_T SFunction_o1_m[4];  /* '<S17>/S-Function' */
  real_T SFunction_o2_b;     /* '<S17>/S-Function' */
  real_T SFunction_o1_e;     /* '<S27>/S-Function' */
  real_T SFunction_o2_h;     /* '<S27>/S-Function' */
  real_T SFunction_o3;       /* '<S27>/S-Function' */
  real_T Merge;              /* '<S19>/Merge' */
  real_T Divide;             /* '<S18>/Divide' */
  real_T Divide_h;           /* '<S20>/Divide' */
  real_T Divide_p;           /* '<S25>/Divide' */
  real_T Divide_c;           /* '<S26>/Divide' */
  real_T Memory_p;           /* '<S44>/Memory' */
  real_T Merge_c;            /* '<S44>/Merge' */
  real_T Switch;             /* '<S59>/Switch' */
  real_T Product1;           /* '<S41>/Product1' */
  real_T Add;                /* '<S41>/Add' */
  real_T Pwr_Cal;            /* '<S49>/Pwr_Cal' */
  real_T Normal_PwrDmnd;     /* '<S49>/Normal_PwrDmnd' */
  real_T Saturation;         /* '<S102>/Saturation' */
  real_T Divide_f;           /* '<S62>/Divide' */
  boolean_T diff_prs_cmd[4]; /* '<S28>/Data Type Conversion' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_TxCar_Hybrid_T {
  real_T Integrator_DSTATE;          /* '<S95>/Integrator' */
  real_T Filter_DSTATE;              /* '<S90>/Filter' */
  real_T Memory_1_PreviousInput[4];  /* '<S9>/Memory' */
  real_T Memory_2_PreviousInput[4];  /* '<S9>/Memory' */
  real_T Memory_3_PreviousInput[4];  /* '<S9>/Memory' */
  real_T Memory_5_PreviousInput;     /* '<S40>/Memory' */
  real_T Memory_1_PreviousInput_i;   /* '<S40>/Memory' */
  real_T Memory_2_PreviousInput_p;   /* '<S40>/Memory' */
  real_T Memory_3_PreviousInput_k;   /* '<S40>/Memory' */
  real_T Memory_PreviousInput;       /* '<S27>/Memory' */
  real_T Memory_4_PreviousInput;     /* '<S40>/Memory' */
  real_T Memory_6_PreviousInput;     /* '<S40>/Memory' */
  real_T Memory_PreviousInput_p;     /* '<S44>/Memory' */
  real_T Memory1_PreviousInput;      /* '<S41>/Memory1' */
  void* Chassis_env_inBUS;           /* '<S2>/Chassis' */
  void* Chassis_driveline_inBUS;     /* '<S2>/Chassis' */
  void* Chassis_body_outBUS;         /* '<S2>/Chassis' */
  void* Chassis_wheel_outBUS;        /* '<S2>/Chassis' */
  void* Chassis_susp_outBUS;         /* '<S2>/Chassis' */
  void* SFunction_ecu_inBUS;         /* '<S120>/S-Function' */
  void* SFunction_ecu_outBUS;        /* '<S120>/S-Function' */
  void* SFunction_hev_outBUS;        /* '<S120>/S-Function' */
  void* SFunction_DrivelineInBUS;    /* '<S16>/S-Function' */
  void* SFunction_DrivelineOutBUS;   /* '<S16>/S-Function' */
  void* SFunction_DrivelineInBUS_j;  /* '<S27>/S-Function' */
  void* SFunction_softecu_outputBUS; /* '<S27>/S-Function' */
  void* SFunction_BattInBUS;         /* '<S42>/S-Function' */
  void* SFunction_BattOutBUS;        /* '<S42>/S-Function' */
  void* SFunction_MotorInBUS;        /* '<S45>/S-Function' */
  void* SFunction_MotorOutBUS;       /* '<S45>/S-Function' */
  void* SFunction_MotorInBUS_p;      /* '<S47>/S-Function' */
  void* SFunction_MotorOutBUS_g;     /* '<S47>/S-Function' */
  void* SFunction_EngineInBUS;       /* '<S43>/S-Function' */
  void* SFunction_EngineOutBUS;      /* '<S43>/S-Function' */
  void* Steer_Mapped_driver_inBUS;   /* '<S121>/Steer_Mapped' */
  void* Steer_Mapped_body_inBUS;     /* '<S121>/Steer_Mapped' */
  void* Steer_Mapped_susp_inBUS;     /* '<S121>/Steer_Mapped' */
  void* Steer_Mapped_wheel_inBUS;    /* '<S121>/Steer_Mapped' */
  void* Steer_Mapped_steer_outBUS;   /* '<S121>/Steer_Mapped' */
  void* SFunction_CarTypeOutBUS;     /* '<S1>/S-Function' */
  int8_T SwitchCase_ActiveSubsystem; /* '<S44>/Switch Case' */
  int8_T Integrator_PrevResetState;  /* '<S95>/Integrator' */
  int8_T Filter_PrevResetState;      /* '<S90>/Filter' */
};

/* Continuous states (default storage) */
struct X_TxCar_Hybrid_T {
  real_T Integrator1_CSTATE;    /* '<S18>/Integrator1' */
  real_T Integrator1_CSTATE_c;  /* '<S20>/Integrator1' */
  real_T Integrator1_CSTATE_a;  /* '<S25>/Integrator1' */
  real_T Integrator1_CSTATE_e;  /* '<S26>/Integrator1' */
  real_T Integrator1_CSTATE_ce; /* '<S62>/Integrator1' */
};

/* State derivatives (default storage) */
struct XDot_TxCar_Hybrid_T {
  real_T Integrator1_CSTATE;    /* '<S18>/Integrator1' */
  real_T Integrator1_CSTATE_c;  /* '<S20>/Integrator1' */
  real_T Integrator1_CSTATE_a;  /* '<S25>/Integrator1' */
  real_T Integrator1_CSTATE_e;  /* '<S26>/Integrator1' */
  real_T Integrator1_CSTATE_ce; /* '<S62>/Integrator1' */
};

/* State disabled  */
struct XDis_TxCar_Hybrid_T {
  boolean_T Integrator1_CSTATE;    /* '<S18>/Integrator1' */
  boolean_T Integrator1_CSTATE_c;  /* '<S20>/Integrator1' */
  boolean_T Integrator1_CSTATE_a;  /* '<S25>/Integrator1' */
  boolean_T Integrator1_CSTATE_e;  /* '<S26>/Integrator1' */
  boolean_T Integrator1_CSTATE_ce; /* '<S62>/Integrator1' */
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
struct ExtU_TxCar_Hybrid_T {
  Ctrl_Input DriverIn;    /* '<Root>/DriverIn' */
  Env_Bus_Mu Friction;    /* '<Root>/Friction' */
  Env_Bus_GroundZ Height; /* '<Root>/Height' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_TxCar_Hybrid_T {
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
  Vehicle_Body_Output Body;               /* '<Root>/Body' */
  Susp_Output Susp;                       /* '<Root>/Susp' */
  Wheels_Output WheelsOut;                /* '<Root>/WheelsOut' */
  HEV_ControllerOutput OutBus_HEV_Output; /* '<Root>/OutBus_HEV_Output' */
};

/* Parameters (default storage) */
struct P_TxCar_Hybrid_T_ {
  real_T Throttle_PID_D; /* Mask Parameter: Throttle_PID_D
                          * Referenced by: '<S89>/Derivative Gain'
                          */
  real_T Throttle_PID_I; /* Mask Parameter: Throttle_PID_I
                          * Referenced by: '<S92>/Integral Gain'
                          */
  real_T Throttle_PID_InitialConditionForFilter;
  /* Mask Parameter: Throttle_PID_InitialConditionForFilter
   * Referenced by: '<S90>/Filter'
   */
  real_T Throttle_PID_InitialConditionForIntegrator;
  /* Mask Parameter: Throttle_PID_InitialConditionForIntegrator
   * Referenced by: '<S95>/Integrator'
   */
  real_T Throttle_PID_LowerSaturationLimit;
  /* Mask Parameter: Throttle_PID_LowerSaturationLimit
   * Referenced by: '<S102>/Saturation'
   */
  real_T DivProtect_MinVal;   /* Mask Parameter: DivProtect_MinVal
                               * Referenced by:
                               *   '<S56>/Constant'
                               *   '<S57>/Constant'
                               */
  real_T DivProtect_MinVal_e; /* Mask Parameter: DivProtect_MinVal_e
                               * Referenced by:
                               *   '<S59>/Constant'
                               *   '<S60>/Constant'
                               */
  real_T Throttle_PID_N;      /* Mask Parameter: Throttle_PID_N
                               * Referenced by: '<S98>/Filter Coefficient'
                               */
  real_T Throttle_PID_P;      /* Mask Parameter: Throttle_PID_P
                               * Referenced by: '<S100>/Proportional Gain'
                               */
  real_T Throttle_PID_UpperSaturationLimit;
  /* Mask Parameter: Throttle_PID_UpperSaturationLimit
   * Referenced by: '<S102>/Saturation'
   */
  real_T CompareToConstant_const;         /* Mask Parameter: CompareToConstant_const
                                           * Referenced by: '<S37>/Constant'
                                           */
  real_T EngSpdAct_wc;                    /* Mask Parameter: EngSpdAct_wc
                                           * Referenced by: '<S18>/Constant'
                                           */
  real_T FrontMotorSpd_Filter_wc;         /* Mask Parameter: FrontMotorSpd_Filter_wc
                                           * Referenced by: '<S20>/Constant'
                                           */
  real_T RearMotorSpd_Filter_wc;          /* Mask Parameter: RearMotorSpd_Filter_wc
                                           * Referenced by: '<S25>/Constant'
                                           */
  real_T TransOutSpd_Filter_wc;           /* Mask Parameter: TransOutSpd_Filter_wc
                                           * Referenced by: '<S26>/Constant'
                                           */
  real_T LPF_wc;                          /* Mask Parameter: LPF_wc
                                           * Referenced by: '<S62>/Constant'
                                           */
  real_T const_mu_Value[4];               /* Expression: 0.9*ones(1,4)
                                           * Referenced by: '<S36>/const_mu'
                                           */
  real_T Zero_Torque_Value;               /* Expression: 0
                                           * Referenced by: '<S55>/Zero_Torque'
                                           */
  real_T Constant1_Value;                 /* Expression: 1
                                           * Referenced by: '<S56>/Constant1'
                                           */
  real_T Constant2_Value;                 /* Expression: -1
                                           * Referenced by: '<S56>/Constant2'
                                           */
  real_T Constant_Value;                  /* Expression: 0
                                           * Referenced by: '<S58>/Constant'
                                           */
  real_T Constant1_Value_d;               /* Expression: 1
                                           * Referenced by: '<S59>/Constant1'
                                           */
  real_T Constant2_Value_o;               /* Expression: -1
                                           * Referenced by: '<S59>/Constant2'
                                           */
  real_T RPM_Gain;                        /* Expression: 30/pi
                                           * Referenced by: '<S48>/RPM'
                                           */
  real_T Constant_Value_p;                /* Expression: 0
                                           * Referenced by: '<S61>/Constant'
                                           */
  real_T Constant3_Value;                 /* Expression: 0
                                           * Referenced by: '<S5>/Constant3'
                                           */
  real_T Constant4_Value;                 /* Expression: 0
                                           * Referenced by: '<S5>/Constant4'
                                           */
  real_T Constant5_Value;                 /* Expression: 0
                                           * Referenced by: '<S5>/Constant5'
                                           */
  real_T Memory_1_InitialCondition;       /* Expression: 0
                                           * Referenced by: '<S9>/Memory'
                                           */
  real_T Memory_2_InitialCondition;       /* Expression: 0
                                           * Referenced by: '<S9>/Memory'
                                           */
  real_T Memory_3_InitialCondition;       /* Expression: 0
                                           * Referenced by: '<S9>/Memory'
                                           */
  real_T Gain_Gain;                       /* Expression: 9.81
                                           * Referenced by: '<S115>/Gain'
                                           */
  real_T Integrator1_IC;                  /* Expression: 0
                                           * Referenced by: '<S18>/Integrator1'
                                           */
  real_T Memory_5_InitialCondition;       /* Expression: 0
                                           * Referenced by: '<S40>/Memory'
                                           */
  real_T Constant1_Value_k;               /* Expression: 0
                                           * Referenced by: '<S119>/Constant1'
                                           */
  real_T Integrator1_IC_j;                /* Expression: 0
                                           * Referenced by: '<S20>/Integrator1'
                                           */
  real_T Integrator1_IC_o;                /* Expression: 0
                                           * Referenced by: '<S25>/Integrator1'
                                           */
  real_T Gain5_Gain;                      /* Expression: 1
                                           * Referenced by: '<S28>/Gain5'
                                           */
  real_T Constant1_Value_f;               /* Expression: 0
                                           * Referenced by: '<S28>/Constant1'
                                           */
  real_T Gain1_Gain;                      /* Expression: 1
                                           * Referenced by: '<S28>/Gain1'
                                           */
  real_T Gain2_Gain;                      /* Expression: 1
                                           * Referenced by: '<S28>/Gain2'
                                           */
  real_T Constant2_Value_j;               /* Expression: 0
                                           * Referenced by: '<S28>/Constant2'
                                           */
  real_T Gain3_Gain;                      /* Expression: 1
                                           * Referenced by: '<S28>/Gain3'
                                           */
  real_T Memory_1_InitialCondition_l;     /* Expression: 0
                                           * Referenced by: '<S40>/Memory'
                                           */
  real_T Gain4_Gain;                      /* Expression: 1
                                           * Referenced by: '<S28>/Gain4'
                                           */
  real_T N_Value;                         /* Expression: 0
                                           * Referenced by: '<S32>/N'
                                           */
  real_T D_Value;                         /* Expression: 1
                                           * Referenced by: '<S32>/D'
                                           */
  real_T R_Value;                         /* Expression: -1
                                           * Referenced by: '<S32>/R'
                                           */
  real_T P_Value;                         /* Expression: 0
                                           * Referenced by: '<S32>/P'
                                           */
  real_T Constant_Value_e;                /* Expression: 0
                                           * Referenced by: '<S119>/Constant'
                                           */
  real_T Memory_2_InitialCondition_l;     /* Expression: 0
                                           * Referenced by: '<S40>/Memory'
                                           */
  real_T Gain6_Gain;                      /* Expression: 1
                                           * Referenced by: '<S28>/Gain6'
                                           */
  real_T Memory_3_InitialCondition_l;     /* Expression: 0
                                           * Referenced by: '<S40>/Memory'
                                           */
  real_T NoUse_Value;                     /* Expression: 0
                                           * Referenced by: '<S6>/NoUse'
                                           */
  real_T Gain7_Gain;                      /* Expression: 1
                                           * Referenced by: '<S28>/Gain7'
                                           */
  real_T Memory_InitialCondition;         /* Expression: 0
                                           * Referenced by: '<S27>/Memory'
                                           */
  real_T Merge_InitialOutput;             /* Computed Parameter: Merge_InitialOutput
                                           * Referenced by: '<S19>/Merge'
                                           */
  real_T Constant3_Value_c;               /* Expression: 0
                                           * Referenced by: '<S21>/Constant3'
                                           */
  real_T Saturation_UpperSat;             /* Expression: 5000
                                           * Referenced by: '<S12>/Saturation'
                                           */
  real_T Saturation_LowerSat;             /* Expression: -5000
                                           * Referenced by: '<S12>/Saturation'
                                           */
  real_T Integrator1_IC_f;                /* Expression: 0
                                           * Referenced by: '<S26>/Integrator1'
                                           */
  real_T Gain2_Gain_l;                    /* Expression: 1
                                           * Referenced by: '<S114>/Gain2'
                                           */
  real_T Memory_4_InitialCondition;       /* Expression: 0
                                           * Referenced by: '<S40>/Memory'
                                           */
  real_T Memory_6_InitialCondition;       /* Expression: 0
                                           * Referenced by: '<S40>/Memory'
                                           */
  real_T Memory_InitialCondition_f;       /* Expression: 0
                                           * Referenced by: '<S44>/Memory'
                                           */
  real_T Integrator1_IC_d;                /* Expression: 0
                                           * Referenced by: '<S62>/Integrator1'
                                           */
  real_T rads_Gain;                       /* Expression: pi/30
                                           * Referenced by: '<S44>/rads'
                                           */
  real_T Memory1_InitialCondition;        /* Expression: 0
                                           * Referenced by: '<S41>/Memory1'
                                           */
  real_T Gain3_Gain_p;                    /* Expression: 1
                                           * Referenced by: '<S41>/Gain3'
                                           */
  real_T Saturation_UpperSat_o;           /* Expression: 1000
                                           * Referenced by: '<S41>/Saturation'
                                           */
  real_T Saturation_LowerSat_o;           /* Expression: -1000
                                           * Referenced by: '<S41>/Saturation'
                                           */
  real_T Constant_Value_n;                /* Expression: 30+273.15
                                           * Referenced by: '<S41>/Constant'
                                           */
  real_T GeneratorPwrSaturation_UpperSat; /* Expression: 300000
                                           * Referenced by: '<S46>/GeneratorPwrSaturation'
                                           */
  real_T GeneratorPwrSaturation_LowerSat; /* Expression: 0
                                           * Referenced by: '<S46>/GeneratorPwrSaturation'
                                           */
  real_T Gain_Gain_o;                     /* Expression: -1
                                           * Referenced by: '<S46>/Gain'
                                           */
  real_T Gain2_Gain_c;                    /* Expression: 60/2/3.1415
                                           * Referenced by: '<S50>/Gain2'
                                           */
  real_T Gain4_Gain_p;                    /* Expression: 60/2/3.1415
                                           * Referenced by: '<S52>/Gain4'
                                           */
  real_T Pwr_Dmnd_Threshold;              /* Expression: 0.5
                                           * Referenced by: '<S49>/Pwr_Dmnd'
                                           */
  real_T Saturation_UpperSat_h;           /* Expression: 7000
                                           * Referenced by: '<S49>/Saturation'
                                           */
  real_T Saturation_LowerSat_n;           /* Expression: 750
                                           * Referenced by: '<S49>/Saturation'
                                           */
  real_T rads_Gain_k;                     /* Expression: pi/30.0
                                           * Referenced by: '<S49>/rad|s'
                                           */
  real_T Const_Pwr_Value;                 /* Expression: 40000
                                           * Referenced by: '<S49>/Const_Pwr'
                                           */
  real_T Integrator_gainval;              /* Computed Parameter: Integrator_gainval
                                           * Referenced by: '<S95>/Integrator'
                                           */
  real_T Filter_gainval;                  /* Computed Parameter: Filter_gainval
                                           * Referenced by: '<S90>/Filter'
                                           */
  real_T Switch_Threshold;                /* Expression: 0.5
                                           * Referenced by: '<S48>/Switch'
                                           */
  real_T Saturation_UpperSat_k;           /* Expression: 45/180.0*pi
                                           * Referenced by: '<S9>/Saturation'
                                           */
  real_T Saturation_LowerSat_l;           /* Expression: -45/180.0*pi
                                           * Referenced by: '<S9>/Saturation'
                                           */
  real_T DeadZone_Start;                  /* Expression: -0.1
                                           * Referenced by: '<S112>/Dead Zone'
                                           */
  real_T DeadZone_End;                    /* Expression: 0.1
                                           * Referenced by: '<S112>/Dead Zone'
                                           */
  real_T AxleTrqOffset_Value;             /* Expression: 30
                                           * Referenced by: '<S112>/AxleTrqOffset'
                                           */
  real_T Constant_Value_p5;               /* Expression: 0
                                           * Referenced by: '<S114>/Constant'
                                           */
  real_T Gain_Gain_a;                     /* Expression: 1
                                           * Referenced by: '<S114>/Gain'
                                           */
  real_T Gain1_Gain_o;                    /* Expression: 1
                                           * Referenced by: '<S114>/Gain1'
                                           */
  real_T Gain_Gain_m;                     /* Expression: 1
                                           * Referenced by: '<S117>/Gain'
                                           */
  real_T Gain1_Gain_f;                    /* Expression: 1
                                           * Referenced by: '<S117>/Gain1'
                                           */
  real_T Zero_Value;                      /* Expression: 0
                                           * Referenced by: '<S1>/Zero'
                                           */
  int8_T Chassis_P1[21];                  /* Expression: int8(lic_path)
                                           * Referenced by: '<S2>/Chassis'
                                           */
  int8_T Chassis_P2[38];                  /* Expression: int8(par_path)
                                           * Referenced by: '<S2>/Chassis'
                                           */
  int8_T SFunction_P1[21];                /* Expression: int8(lic_path)
                                           * Referenced by: '<S120>/S-Function'
                                           */
  int8_T SFunction_P2[38];                /* Expression: int8(par_path)
                                           * Referenced by: '<S120>/S-Function'
                                           */
  int8_T SFunction_P1_b[21];              /* Expression: int8(lic_path)
                                           * Referenced by: '<S6>/S-Function'
                                           */
  int8_T SFunction_P2_m[38];              /* Expression: int8(par_path)
                                           * Referenced by: '<S6>/S-Function'
                                           */
  int8_T SFunction_P1_l[21];              /* Expression: int8(lic_path)
                                           * Referenced by: '<S16>/S-Function'
                                           */
  int8_T SFunction_P2_m5[38];             /* Expression: int8(par_path)
                                           * Referenced by: '<S16>/S-Function'
                                           */
  int8_T SFunction_P1_lm[21];             /* Expression: int8(lic_path)
                                           * Referenced by: '<S17>/S-Function'
                                           */
  int8_T SFunction_P2_n[38];              /* Expression: int8(par_path)
                                           * Referenced by: '<S17>/S-Function'
                                           */
  int8_T SFunction_P1_j[21];              /* Expression: int8(lic_path)
                                           * Referenced by: '<S27>/S-Function'
                                           */
  int8_T SFunction_P2_e[38];              /* Expression: int8(par_path)
                                           * Referenced by: '<S27>/S-Function'
                                           */
  int8_T SFunction_P1_f[21];              /* Expression: int8(lic_path)
                                           * Referenced by: '<S15>/S-Function'
                                           */
  int8_T SFunction_P2_i[38];              /* Expression: int8(par_path)
                                           * Referenced by: '<S15>/S-Function'
                                           */
  int8_T SFunction_P1_m[21];              /* Expression: int8(lic_path)
                                           * Referenced by: '<S42>/S-Function'
                                           */
  int8_T SFunction_P2_g[38];              /* Expression: int8(par_path)
                                           * Referenced by: '<S42>/S-Function'
                                           */
  int8_T SFunction_P1_h[21];              /* Expression: int8(lic_path)
                                           * Referenced by: '<S45>/S-Function'
                                           */
  int8_T SFunction_P2_b[38];              /* Expression: int8(par_path)
                                           * Referenced by: '<S45>/S-Function'
                                           */
  int8_T SFunction_P1_g[21];              /* Expression: int8(lic_path)
                                           * Referenced by: '<S47>/S-Function'
                                           */
  int8_T SFunction_P2_o[38];              /* Expression: int8(par_path)
                                           * Referenced by: '<S47>/S-Function'
                                           */
  int8_T SFunction_P1_c[21];              /* Expression: int8(lic_path)
                                           * Referenced by: '<S43>/S-Function'
                                           */
  int8_T SFunction_P2_d[38];              /* Expression: int8(par_path)
                                           * Referenced by: '<S43>/S-Function'
                                           */
  int8_T Steer_Mapped_P1[21];             /* Expression: int8(lic_path)
                                           * Referenced by: '<S121>/Steer_Mapped'
                                           */
  int8_T Steer_Mapped_P2[38];             /* Expression: int8(par_path)
                                           * Referenced by: '<S121>/Steer_Mapped'
                                           */
  int8_T SFunction_P1_lo[21];             /* Expression: int8(lic_path)
                                           * Referenced by: '<S1>/S-Function'
                                           */
  int8_T SFunction_P2_ok[38];             /* Expression: int8(par_path)
                                           * Referenced by: '<S1>/S-Function'
                                           */
};

/* Real-time Model Data Structure */
struct tag_RTM_TxCar_Hybrid_T {
  const char_T* errorStatus;
  RTWSolverInfo solverInfo;
  X_TxCar_Hybrid_T* contStates;
  int_T* periodicContStateIndices;
  real_T* periodicContStateRanges;
  real_T* derivs;
  XDis_TxCar_Hybrid_T* contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[5];
  real_T odeF[4][5];
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

/* Class declaration for model TxCar_Hybrid */
class TxCar_Hybrid final {
  /* public data and function members */
 public:
  /* Copy Constructor */
  TxCar_Hybrid(TxCar_Hybrid const&) = delete;

  /* Assignment Operator */
  TxCar_Hybrid& operator=(TxCar_Hybrid const&) & = delete;

  /* Move Constructor */
  TxCar_Hybrid(TxCar_Hybrid&&) = delete;

  /* Move Assignment Operator */
  TxCar_Hybrid& operator=(TxCar_Hybrid&&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_TxCar_Hybrid_T* getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_TxCar_Hybrid_T* pExtU_TxCar_Hybrid_T) { TxCar_Hybrid_U = *pExtU_TxCar_Hybrid_T; }

  /* Root outports get method */
  const ExtY_TxCar_Hybrid_T& getExternalOutputs() const { return TxCar_Hybrid_Y; }

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  void terminate();

  /* Constructor */
  TxCar_Hybrid();

  /* Destructor */
  ~TxCar_Hybrid();

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_TxCar_Hybrid_T TxCar_Hybrid_U;

  /* External outputs */
  ExtY_TxCar_Hybrid_T TxCar_Hybrid_Y;

  /* Block signals */
  B_TxCar_Hybrid_T TxCar_Hybrid_B;

  /* Block states */
  DW_TxCar_Hybrid_T TxCar_Hybrid_DW;

  /* Tunable parameters */
  static P_TxCar_Hybrid_T TxCar_Hybrid_P;

  /* Block continuous states */
  X_TxCar_Hybrid_T TxCar_Hybrid_X;

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo* si);

  /* Derivatives member function */
  void TxCar_Hybrid_derivatives();

  /* Real-Time Model */
  RT_MODEL_TxCar_Hybrid_T TxCar_Hybrid_M;
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
 * '<Root>' : 'TxCar_Hybrid'
 * '<S1>'   : 'TxCar_Hybrid/CarTypeParsing'
 * '<S2>'   : 'TxCar_Hybrid/Chassis'
 * '<S3>'   : 'TxCar_Hybrid/DriveLine'
 * '<S4>'   : 'TxCar_Hybrid/DriverSelector'
 * '<S5>'   : 'TxCar_Hybrid/Environment'
 * '<S6>'   : 'TxCar_Hybrid/HybridParser'
 * '<S7>'   : 'TxCar_Hybrid/Info'
 * '<S8>'   : 'TxCar_Hybrid/Propulsion'
 * '<S9>'   : 'TxCar_Hybrid/SignalAdapter'
 * '<S10>'  : 'TxCar_Hybrid/SoftHCU'
 * '<S11>'  : 'TxCar_Hybrid/steer'
 * '<S12>'  : 'TxCar_Hybrid/DriveLine/DriveLine'
 * '<S13>'  : 'TxCar_Hybrid/DriveLine/Driveline Input'
 * '<S14>'  : 'TxCar_Hybrid/DriveLine/Driveline Output'
 * '<S15>'  : 'TxCar_Hybrid/DriveLine/DriveLine/BrakeHydraulic'
 * '<S16>'  : 'TxCar_Hybrid/DriveLine/DriveLine/DriveLine_EV_Rear'
 * '<S17>'  : 'TxCar_Hybrid/DriveLine/DriveLine/DriveLine_ICE_FWD'
 * '<S18>'  : 'TxCar_Hybrid/DriveLine/DriveLine/EngSpdAct'
 * '<S19>'  : 'TxCar_Hybrid/DriveLine/DriveLine/FrontDriveMotorSpd'
 * '<S20>'  : 'TxCar_Hybrid/DriveLine/DriveLine/FrontMotorSpd_Filter'
 * '<S21>'  : 'TxCar_Hybrid/DriveLine/DriveLine/HEV_Front_SignalCollector'
 * '<S22>'  : 'TxCar_Hybrid/DriveLine/DriveLine/HEV_SignalRouter'
 * '<S23>'  : 'TxCar_Hybrid/DriveLine/DriveLine/P2_Hybrid'
 * '<S24>'  : 'TxCar_Hybrid/DriveLine/DriveLine/P3_Hybrid'
 * '<S25>'  : 'TxCar_Hybrid/DriveLine/DriveLine/RearMotorSpd_Filter'
 * '<S26>'  : 'TxCar_Hybrid/DriveLine/DriveLine/TransOutSpd_Filter'
 * '<S27>'  : 'TxCar_Hybrid/DriveLine/DriveLine/Transmission'
 * '<S28>'  : 'TxCar_Hybrid/DriveLine/DriveLine/driveline_in_router'
 * '<S29>'  : 'TxCar_Hybrid/DriveLine/DriveLine/FrontDriveMotorSpd/P2_HEV'
 * '<S30>'  : 'TxCar_Hybrid/DriveLine/DriveLine/FrontDriveMotorSpd/P3_HEV'
 * '<S31>'  : 'TxCar_Hybrid/DriveLine/DriveLine/HEV_SignalRouter/TrqRouter'
 * '<S32>'  : 'TxCar_Hybrid/DriveLine/Driveline Input/Subsystem'
 * '<S33>'  : 'TxCar_Hybrid/DriverSelector/CtrlRouter'
 * '<S34>'  : 'TxCar_Hybrid/Environment/Ground Feedback'
 * '<S35>'  : 'TxCar_Hybrid/Environment/Ground Feedback/Constant'
 * '<S36>'  : 'TxCar_Hybrid/Environment/Ground Feedback/Constant/FrictionChecker'
 * '<S37>'  : 'TxCar_Hybrid/Environment/Ground Feedback/Constant/FrictionChecker/Compare To Constant'
 * '<S38>'  : 'TxCar_Hybrid/Environment/Ground Feedback/Constant/FrictionChecker/Subsystem'
 * '<S39>'  : 'TxCar_Hybrid/Propulsion/Input'
 * '<S40>'  : 'TxCar_Hybrid/Propulsion/Out'
 * '<S41>'  : 'TxCar_Hybrid/Propulsion/Power'
 * '<S42>'  : 'TxCar_Hybrid/Propulsion/Power/Battery'
 * '<S43>'  : 'TxCar_Hybrid/Propulsion/Power/Engine'
 * '<S44>'  : 'TxCar_Hybrid/Propulsion/Power/EngineTrq_Router'
 * '<S45>'  : 'TxCar_Hybrid/Propulsion/Power/Front Motor'
 * '<S46>'  : 'TxCar_Hybrid/Propulsion/Power/Generator'
 * '<S47>'  : 'TxCar_Hybrid/Propulsion/Power/Rear Motor'
 * '<S48>'  : 'TxCar_Hybrid/Propulsion/Power/ice_speed_selector'
 * '<S49>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector'
 * '<S50>'  : 'TxCar_Hybrid/Propulsion/Power/motor_front_speed'
 * '<S51>'  : 'TxCar_Hybrid/Propulsion/Power/motor_front_trq_selector'
 * '<S52>'  : 'TxCar_Hybrid/Propulsion/Power/motor_rear_speed'
 * '<S53>'  : 'TxCar_Hybrid/Propulsion/Power/motor_rear_trq_selector'
 * '<S54>'  : 'TxCar_Hybrid/Propulsion/Power/EngineTrq_Router/ParallelPwr'
 * '<S55>'  : 'TxCar_Hybrid/Propulsion/Power/EngineTrq_Router/SerializePwr'
 * '<S56>'  : 'TxCar_Hybrid/Propulsion/Power/EngineTrq_Router/ParallelPwr/DivProtect'
 * '<S57>'  : 'TxCar_Hybrid/Propulsion/Power/EngineTrq_Router/ParallelPwr/DivProtect/Compare To Constant'
 * '<S58>'  : 'TxCar_Hybrid/Propulsion/Power/EngineTrq_Router/ParallelPwr/DivProtect/Compare To Zero'
 * '<S59>'  : 'TxCar_Hybrid/Propulsion/Power/Generator/DivProtect'
 * '<S60>'  : 'TxCar_Hybrid/Propulsion/Power/Generator/DivProtect/Compare To Constant'
 * '<S61>'  : 'TxCar_Hybrid/Propulsion/Power/Generator/DivProtect/Compare To Zero'
 * '<S62>'  : 'TxCar_Hybrid/Propulsion/Power/ice_speed_selector/LPF'
 * '<S63>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID'
 * '<S64>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Anti-windup'
 * '<S65>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/D Gain'
 * '<S66>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Filter'
 * '<S67>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Filter ICs'
 * '<S68>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/I Gain'
 * '<S69>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Ideal P Gain'
 * '<S70>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Ideal P Gain Fdbk'
 * '<S71>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Integrator'
 * '<S72>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Integrator ICs'
 * '<S73>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/N Copy'
 * '<S74>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/N Gain'
 * '<S75>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/P Copy'
 * '<S76>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Parallel P Gain'
 * '<S77>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Reset Signal'
 * '<S78>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Saturation'
 * '<S79>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Saturation Fdbk'
 * '<S80>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Sum'
 * '<S81>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Sum Fdbk'
 * '<S82>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Tracking Mode'
 * '<S83>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Tracking Mode Sum'
 * '<S84>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Tsamp - Integral'
 * '<S85>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Tsamp - Ngain'
 * '<S86>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/postSat Signal'
 * '<S87>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/preSat Signal'
 * '<S88>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Anti-windup/Passthrough'
 * '<S89>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/D Gain/Internal Parameters'
 * '<S90>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Filter/Disc. Forward Euler Filter'
 * '<S91>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Filter ICs/Internal IC - Filter'
 * '<S92>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/I Gain/Internal Parameters'
 * '<S93>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Ideal P Gain/Passthrough'
 * '<S94>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Ideal P Gain Fdbk/Disabled'
 * '<S95>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Integrator/Discrete'
 * '<S96>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Integrator ICs/Internal IC'
 * '<S97>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/N Copy/Disabled'
 * '<S98>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/N Gain/Internal Parameters'
 * '<S99>'  : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/P Copy/Disabled'
 * '<S100>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Parallel P Gain/Internal Parameters'
 * '<S101>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Reset Signal/External Reset'
 * '<S102>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Saturation/Enabled'
 * '<S103>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Saturation Fdbk/Disabled'
 * '<S104>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Sum/Sum_PID'
 * '<S105>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Sum Fdbk/Disabled'
 * '<S106>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Tracking Mode/Disabled'
 * '<S107>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Tracking Mode Sum/Passthrough'
 * '<S108>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Tsamp - Integral/Passthrough'
 * '<S109>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/Tsamp - Ngain/Passthrough'
 * '<S110>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/postSat Signal/Forward_Path'
 * '<S111>' : 'TxCar_Hybrid/Propulsion/Power/ice_throttle_selector/Throttle_PID/preSat Signal/Forward_Path'
 * '<S112>' : 'TxCar_Hybrid/SignalAdapter/TrqRouter'
 * '<S113>' : 'TxCar_Hybrid/SoftHCU/ActiveDiffCtrl'
 * '<S114>' : 'TxCar_Hybrid/SoftHCU/BrakeSigRouter'
 * '<S115>' : 'TxCar_Hybrid/SoftHCU/ECUSigRouter'
 * '<S116>' : 'TxCar_Hybrid/SoftHCU/HevController_Pwr'
 * '<S117>' : 'TxCar_Hybrid/SoftHCU/TrqRouter'
 * '<S118>' : 'TxCar_Hybrid/SoftHCU/ActiveDiffCtrl/Active Differential Control'
 * '<S119>' : 'TxCar_Hybrid/SoftHCU/ActiveDiffCtrl/Active Differential Control/No Control'
 * '<S120>' : 'TxCar_Hybrid/SoftHCU/HevController_Pwr/SoftHCU'
 * '<S121>' : 'TxCar_Hybrid/steer/SteerSystem'
 */
#endif /* RTW_HEADER_TxCar_Hybrid_h_ */
