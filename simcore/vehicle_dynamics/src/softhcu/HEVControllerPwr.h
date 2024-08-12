/*
 * HEVControllerPwr.h
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

#ifndef RTW_HEADER_HEVControllerPwr_h_
#define RTW_HEADER_HEVControllerPwr_h_
#include "HEVControllerPwr_types.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rtwtypes.h"

#include "inc/car_common.h"

extern "C" {

#include "rtGetInf.h"
}

#include <cstring>

extern "C" {

#include "rt_nonfinite.h"
}

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

/* Block signals for system '<S22>/EV_Pwr_Calculation' */
struct B_EV_Pwr_Calculation_HEVControllerPwr_T {
  HEVPwrDmnd_Output Merge; /* '<S23>/Merge' */
};

/* Block signals for system '<S22>/Engine_Pwr_Calculation' */
struct B_Engine_Pwr_Calculation_HEVControllerPwr_T {
  HEVPwrDmnd_Output Merge; /* '<S24>/Merge' */
};

/* Block signals for system '<S22>/HEV_Pwr_Calculation' */
struct B_HEV_Pwr_Calculation_HEVControllerPwr_T {
  HEVPwrDmnd_Output Merge; /* '<S25>/Merge' */
};

/* Block signals for system '<S22>/ReGen_Pwr_Calculation' */
struct B_ReGen_Pwr_Calculation_HEVControllerPwr_T {
  HEVPwrDmnd_Output Merge; /* '<S26>/Merge' */
};

/* Block signals for system '<S4>/ax_estimator' */
struct B_ax_estimator_HEVControllerPwr_T {
  real_T Integrator1; /* '<S186>/Integrator1' */
  real_T Divide;      /* '<S186>/Divide' */
  real_T Delay;       /* '<S187>/Delay' */
};

/* Block states (default storage) for system '<S4>/ax_estimator' */
struct DW_ax_estimator_HEVControllerPwr_T {
  real_T Delay_DSTATE[100];  /* '<S187>/Delay' */
  int8_T If_ActiveSubsystem; /* '<S53>/If' */
};

/* Continuous states for system '<S4>/ax_estimator' */
struct X_ax_estimator_HEVControllerPwr_T {
  real_T Integrator1_CSTATE; /* '<S186>/Integrator1' */
};

/* State derivatives for system '<S4>/ax_estimator' */
struct XDot_ax_estimator_HEVControllerPwr_T {
  real_T Integrator1_CSTATE; /* '<S186>/Integrator1' */
};

/* State Disabled for system '<S4>/ax_estimator' */
struct XDis_ax_estimator_HEVControllerPwr_T {
  boolean_T Integrator1_CSTATE; /* '<S186>/Integrator1' */
};

/* Block signals (default storage) */
struct B_HEVControllerPwr_T {
  HEVMode_Input BusConversion_InsertedFor_HEV_DriveMode_at_inport_0_BusCreator1;
  HEVPwrDmnd_Output HEV_PwrDmnd_Output; /* '<S11>/PwrCal' */
  HEVMode_Output ReGen_HEVMode;         /* '<S13>/Bus Creator' */
  HEVMode_Output ReGen_HEVMode_p;       /* '<S9>/Bus Creator' */
  HEVMode_Output MultiportSwitch;       /* '<S7>/Multiport Switch' */
  HEVMode_Output HEV_Mode;              /* '<S11>/PwrCal' */
  HEVMode_Output HEV_Mode_f;            /* '<S11>/PwrCal' */
  HEVMode_Output HEV_Mode_j;            /* '<S11>/PwrCal' */
  HEVMode_Output HEV_Mode_g;            /* '<S11>/PwrCal' */
  HEVMode_Output hev_mode_output;       /* '<S2>/HEV_DriveMode' */
  real_T target_acc_m_s2;
  /* '<Root>/BusConversion_InsertedFor_SoftECU_Input_at_outport_0' */
  real_T veh_speed_vx;
  /* '<Root>/BusConversion_InsertedFor_SoftECU_Input_at_outport_0' */
  real_T ReverseSpeedLimit_kph; /* '<S5>/ReverseSpeedLimit_kph' */
  real_T pwr_dmnd;              /* '<S5>/TotalPwdDmnd1' */
  real_T P4_PwrDmnd;            /* '<S10>/P4_PwrDmnd' */
  real_T slip_ratio[4];
  /* '<Root>/BusConversion_InsertedFor_SoftECU_Input_at_outport_0' */
  real_T Integrator;           /* '<S166>/Integrator' */
  real_T Filter;               /* '<S161>/Filter' */
  real_T FilterCoefficient;    /* '<S169>/Filter Coefficient' */
  real_T Switch;               /* '<S157>/Switch' */
  real_T Memory[4];            /* '<S49>/Memory' */
  real_T Abs;                  /* '<S49>/Abs' */
  real_T OnAtRef_OffAt05Ref;   /* '<S49>/OnAtRef_OffAt0.5Ref' */
  real_T ABS_flag[4];          /* '<S49>/ABS_flag' */
  real_T Product1[4];          /* '<S49>/Product1' */
  real_T Filter_o;             /* '<S85>/Filter' */
  real_T IntegralGain;         /* '<S87>/Integral Gain' */
  real_T Integrator_o;         /* '<S90>/Integrator' */
  real_T FilterCoefficient_g;  /* '<S93>/Filter Coefficient' */
  real_T Pwr_Dmnd_w_;          /* '<S11>/PwrCal' */
  real_T Pwr_Dmnd_w__l;        /* '<S11>/PwrCal' */
  real_T Engine_Speed_rpm_;    /* '<S11>/PwrCal' */
  real_T Pwr_Dmnd_w__e;        /* '<S11>/PwrCal' */
  real_T Engine_Speed_rpm__k;  /* '<S11>/PwrCal' */
  real_T Motor_Speed_rads_;    /* '<S11>/PwrCal' */
  real_T P4_Motor_Speed_rads_; /* '<S11>/PwrCal' */
  real_T brake_pressure_0_1;   /* '<S4>/Switch1' */
  real_T Saturation;           /* '<S51>/Saturation' */
  uint8_T brake_control_mode;
  /* '<Root>/BusConversion_InsertedFor_SoftECU_Input_at_outport_0' */
  uint8_T gear_cmd__0N1D2R3P;
  /* '<Root>/BusConversion_InsertedFor_SoftECU_Input_at_outport_0' */
  uint8_T acc_control_mode;
  /* '<Root>/BusConversion_InsertedFor_SoftECU_Input_at_outport_0' */
  boolean_T LogicalOperator;                                          /* '<S54>/Logical Operator' */
  boolean_T Compare;                                                  /* '<S122>/Compare' */
  boolean_T LogicalOperator1;                                         /* '<S121>/Logical Operator1' */
  boolean_T Merge1;                                                   /* '<S49>/Merge1' */
  boolean_T abs_on;                                                   /* '<S49>/if_on' */
  boolean_T LogicalOperator_e;                                        /* '<S55>/Logical Operator' */
  B_ax_estimator_HEVControllerPwr_T ax_estimator;                     /* '<S4>/ax_estimator' */
  B_ReGen_Pwr_Calculation_HEVControllerPwr_T ReGen_Pwr_Calculation;   /* '<S22>/ReGen_Pwr_Calculation' */
  B_HEV_Pwr_Calculation_HEVControllerPwr_T HEV_Pwr_Calculation;       /* '<S22>/HEV_Pwr_Calculation' */
  B_Engine_Pwr_Calculation_HEVControllerPwr_T Engine_Pwr_Calculation; /* '<S22>/Engine_Pwr_Calculation' */
  B_EV_Pwr_Calculation_HEVControllerPwr_T EV_Pwr_Calculation;         /* '<S22>/EV_Pwr_Calculation' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_HEVControllerPwr_T {
  real_T Integrator_DSTATE;                        /* '<S166>/Integrator' */
  real_T Filter_DSTATE;                            /* '<S161>/Filter' */
  real_T Filter_DSTATE_l;                          /* '<S85>/Filter' */
  real_T Integrator_DSTATE_c;                      /* '<S90>/Integrator' */
  real_T Memory_PreviousInput[4];                  /* '<S49>/Memory' */
  real_T t_local;                                  /* '<S2>/HEV_DriveMode' */
  int8_T If_ActiveSubsystem;                       /* '<S51>/If' */
  int8_T Integrator_PrevResetState;                /* '<S166>/Integrator' */
  int8_T Filter_PrevResetState;                    /* '<S161>/Filter' */
  int8_T If_ActiveSubsystem_m;                     /* '<S49>/If' */
  int8_T Filter_PrevResetState_h;                  /* '<S85>/Filter' */
  int8_T Integrator_PrevResetState_n;              /* '<S90>/Integrator' */
  uint8_T is_c4_HEVControllerPwr;                  /* '<S11>/PwrCal' */
  uint8_T is_HEVDrive;                             /* '<S11>/PwrCal' */
  uint8_T is_active_c4_HEVControllerPwr;           /* '<S11>/PwrCal' */
  uint8_T is_c2_HEVControllerPwr;                  /* '<S2>/HEV_DriveMode' */
  uint8_T is_active_c2_HEVControllerPwr;           /* '<S2>/HEV_DriveMode' */
  boolean_T RelayCutOffAt6_EnableAt5_Mode;         /* '<S54>/RelayCutOffAt6_EnableAt5' */
  boolean_T RelayCutOffAt58_EnableAt55_Mode;       /* '<S54>/RelayCutOffAt58_EnableAt55' */
  boolean_T ReverseSpeedLimit_kph_Mode;            /* '<S5>/ReverseSpeedLimit_kph' */
  boolean_T OnAtRef_OffAt05Ref_Mode;               /* '<S49>/OnAtRef_OffAt0.5Ref' */
  boolean_T switchToEngine;                        /* '<S2>/HEV_DriveMode' */
  boolean_T switchToEV;                            /* '<S2>/HEV_DriveMode' */
  boolean_T switchToHEV;                           /* '<S2>/HEV_DriveMode' */
  DW_ax_estimator_HEVControllerPwr_T ax_estimator; /* '<S4>/ax_estimator' */
};

/* Continuous states (default storage) */
struct X_HEVControllerPwr_T {
  X_ax_estimator_HEVControllerPwr_T ax_estimator; /* '<S4>/ax_estimator' */
};

/* State derivatives (default storage) */
struct XDot_HEVControllerPwr_T {
  XDot_ax_estimator_HEVControllerPwr_T ax_estimator; /* '<S4>/ax_estimator' */
};

/* State disabled  */
struct XDis_HEVControllerPwr_T {
  XDis_ax_estimator_HEVControllerPwr_T ax_estimator; /* '<S4>/ax_estimator' */
};

#ifndef ODE4_INTG
#  define ODE4_INTG

/* ODE4 Integration Data */
struct ODE4_IntgData {
  real_T *y;    /* output */
  real_T *f[4]; /* derivatives */
};

#endif

/* External inputs (root inport signals with default storage) */
struct ExtU_HEVControllerPwr_T {
  powecu_in SoftECU_Input; /* '<Root>/SoftECU_Input' */
  real_T FrontMotSpdrads;  /* '<Root>/FrontMotSpd[rad|s]' */
  real_T P4MotSpdrads;     /* '<Root>/P4MotSpd[rad|s]' */
  real_T EngineSpdrads;    /* '<Root>/EngineSpd[rad|s]' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_HEVControllerPwr_T {
  powecu_out SoftECU_Output;       /* '<Root>/SoftECU_Output' */
  HEV_ControllerOutput HEV_Output; /* '<Root>/HEV_Output' */
};

/* Parameters for system: '<S23>/NoPwrMode' */
struct P_NoPwrMode_HEVControllerPwr_T_ {
  real_T Zero_Value; /* Expression: 0
                      * Referenced by: '<S28>/Zero'
                      */
};

/* Parameters for system: '<S22>/EV_Pwr_Calculation' */
struct P_EV_Pwr_Calculation_HEVControllerPwr_T_ {
  HEVPwrDmnd_Output Merge_InitialOutput;
  /* Computed Parameter: Merge_InitialOutput
   * Referenced by: '<S23>/Merge'
   */
  real_T Zero_Value;                        /* Expression: 0
                                             * Referenced by: '<S30>/Zero'
                                             */
  real_T Front_Threshold;                   /* Expression: 0.5
                                             * Referenced by: '<S30>/Front'
                                             */
  real_T Saturation_UpperSat;               /* Expression: inf
                                             * Referenced by: '<S30>/Saturation'
                                             */
  real_T Saturation_LowerSat;               /* Expression: 0
                                             * Referenced by: '<S30>/Saturation'
                                             */
  real_T Zero_Value_i;                      /* Expression: 0
                                             * Referenced by: '<S27>/Zero'
                                             */
  real_T Rear_Threshold;                    /* Expression: 0.5
                                             * Referenced by: '<S30>/Rear'
                                             */
  real_T Saturation1_UpperSat;              /* Expression: inf
                                             * Referenced by: '<S30>/Saturation1'
                                             */
  real_T Saturation1_LowerSat;              /* Expression: 0
                                             * Referenced by: '<S30>/Saturation1'
                                             */
  real_T Zero_Value_l;                      /* Expression: 0
                                             * Referenced by: '<S31>/Zero'
                                             */
  real_T SerializePwrSaturation_LowerSat;   /* Expression: 0
                                             * Referenced by: '<S29>/SerializePwrSaturation'
                                             */
  real_T Front_Threshold_b;                 /* Expression: 0.5
                                             * Referenced by: '<S31>/Front'
                                             */
  real_T Saturation1_UpperSat_o;            /* Expression: inf
                                             * Referenced by: '<S31>/Saturation1'
                                             */
  real_T Saturation1_LowerSat_d;            /* Expression: 0
                                             * Referenced by: '<S31>/Saturation1'
                                             */
  real_T Rear_Threshold_m;                  /* Expression: 0.5
                                             * Referenced by: '<S31>/Rear'
                                             */
  real_T Saturation2_UpperSat;              /* Expression: inf
                                             * Referenced by: '<S31>/Saturation2'
                                             */
  real_T Saturation2_LowerSat;              /* Expression: 0
                                             * Referenced by: '<S31>/Saturation2'
                                             */
  real_T Zero_Value_ij;                     /* Expression: 0
                                             * Referenced by: '<S29>/Zero'
                                             */
  real_T KW_Gain;                           /* Expression: 0.001
                                             * Referenced by: '<S29>/KW'
                                             */
  P_NoPwrMode_HEVControllerPwr_T NoPwrMode; /* '<S23>/NoPwrMode' */
};

/* Parameters for system: '<S22>/Engine_Pwr_Calculation' */
struct P_Engine_Pwr_Calculation_HEVControllerPwr_T_ {
  HEVPwrDmnd_Output Merge_InitialOutput;
  /* Computed Parameter: Merge_InitialOutput
   * Referenced by: '<S24>/Merge'
   */
  real_T Gain_Gain;                         /* Expression: 1000
                                             * Referenced by: '<S32>/Gain'
                                             */
  real_T Zero_Value;                        /* Expression: 0
                                             * Referenced by: '<S32>/Zero'
                                             */
  real_T ChargingPwr_Threshold;             /* Expression: 0.5
                                             * Referenced by: '<S32>/ChargingPwr'
                                             */
  P_NoPwrMode_HEVControllerPwr_T NoPwrMode; /* '<S24>/NoPwrMode' */
};

/* Parameters for system: '<S22>/HEV_Pwr_Calculation' */
struct P_HEV_Pwr_Calculation_HEVControllerPwr_T_ {
  HEVPwrDmnd_Output Merge_InitialOutput;
  /* Computed Parameter: Merge_InitialOutput
   * Referenced by: '<S25>/Merge'
   */
  real_T Constant1_Value;                   /* Expression: 0
                                             * Referenced by: '<S35>/Constant1'
                                             */
  real_T Zero_Value;                        /* Expression: 0
                                             * Referenced by: '<S38>/Zero'
                                             */
  real_T Gain_Gain;                         /* Expression: 1000
                                             * Referenced by: '<S35>/Gain'
                                             */
  real_T Saturation_UpperSat;               /* Expression: inf
                                             * Referenced by: '<S35>/Saturation'
                                             */
  real_T Saturation_LowerSat;               /* Expression: 0
                                             * Referenced by: '<S35>/Saturation'
                                             */
  real_T Front_Threshold;                   /* Expression: 0.5
                                             * Referenced by: '<S38>/Front'
                                             */
  real_T Saturation_UpperSat_l;             /* Expression: inf
                                             * Referenced by: '<S38>/Saturation'
                                             */
  real_T Saturation_LowerSat_d;             /* Expression: 0
                                             * Referenced by: '<S38>/Saturation'
                                             */
  real_T Zero_Value_c;                      /* Expression: 0
                                             * Referenced by: '<S35>/Zero'
                                             */
  real_T Rear_Threshold;                    /* Expression: 0.5
                                             * Referenced by: '<S38>/Rear'
                                             */
  real_T Saturation1_UpperSat;              /* Expression: inf
                                             * Referenced by: '<S38>/Saturation1'
                                             */
  real_T Saturation1_LowerSat;              /* Expression: 0
                                             * Referenced by: '<S38>/Saturation1'
                                             */
  P_NoPwrMode_HEVControllerPwr_T NoPwrMode; /* '<S25>/NoPwrMode' */
};

/* Parameters for system: '<S22>/ReGen_Pwr_Calculation' */
struct P_ReGen_Pwr_Calculation_HEVControllerPwr_T_ {
  HEVPwrDmnd_Output Merge_InitialOutput;
  /* Computed Parameter: Merge_InitialOutput
   * Referenced by: '<S26>/Merge'
   */
  real_T Half_Value;                        /* Expression: 0.5
                                             * Referenced by: '<S42>/Half'
                                             */
  real_T All_Value;                         /* Expression: 1
                                             * Referenced by: '<S42>/All'
                                             */
  real_T Switch_Threshold;                  /* Expression: 0.5
                                             * Referenced by: '<S42>/Switch'
                                             */
  real_T ReGenPwr_DeadZone_Start;           /* Expression: 0
                                             * Referenced by: '<S39>/ReGenPwr_DeadZone'
                                             */
  real_T ReGenPwr_DeadZone_End;             /* Expression: 1000
                                             * Referenced by: '<S39>/ReGenPwr_DeadZone'
                                             */
  real_T Constant1_Value;                   /* Expression: 0
                                             * Referenced by: '<S39>/Constant1'
                                             */
  real_T Gain_Gain;                         /* Expression: -1
                                             * Referenced by: '<S39>/Gain'
                                             */
  real_T Gain1_Gain;                        /* Expression: -1
                                             * Referenced by: '<S39>/Gain1'
                                             */
  real_T Constant_Value;                    /* Expression: 800
                                             * Referenced by: '<S41>/Constant'
                                             */
  real_T Saturation_UpperSat;               /* Expression: 1
                                             * Referenced by: '<S41>/Saturation'
                                             */
  real_T Saturation_LowerSat;               /* Expression: 0
                                             * Referenced by: '<S41>/Saturation'
                                             */
  P_NoPwrMode_HEVControllerPwr_T NoPwrMode; /* '<S26>/NoPwrMode' */
};

/* Parameters for system: '<S4>/ax_estimator' */
struct P_ax_estimator_HEVControllerPwr_T_ {
  real_T SpeedThresh_const;      /* Mask Parameter: SpeedThresh_const
                                  * Referenced by: '<S189>/Constant'
                                  */
  real_T AxAct_wc;               /* Mask Parameter: AxAct_wc
                                  * Referenced by: '<S186>/Constant'
                                  */
  real_T Delay_InitialCondition; /* Expression: 0.0
                                  * Referenced by: '<S187>/Delay'
                                  */
  real_T Integrator1_IC;         /* Expression: 0
                                  * Referenced by: '<S186>/Integrator1'
                                  */
};

/* Parameters (default storage) */
struct P_HEVControllerPwr_T_ {
  struct_WPo6c0btEjgdkiBqVilJ2B VEH;                             /* Variable: VEH
                                                                  * Referenced by:
                                                                  *   '<S183>/InertiaForce'
                                                                  *   '<S184>/AeroDragForce'
                                                                  *   '<S184>/RollingResistanceForce'
                                                                  */
  real_T DriveLinePowType;                                       /* Variable: DriveLinePowType
                                                                  * Referenced by: '<S50>/Constant2'
                                                                  */
  real_T EnableP4;                                               /* Variable: EnableP4
                                                                  * Referenced by:
                                                                  *   '<S22>/EV_Pwr_Calculation'
                                                                  *   '<S22>/HEV_Pwr_Calculation'
                                                                  *   '<S22>/ReGen_Pwr_Calculation'
                                                                  */
  real_T EngIdleSpd;                                             /* Variable: EngIdleSpd
                                                                  * Referenced by:
                                                                  *   '<S22>/EV_Pwr_Calculation'
                                                                  *   '<S22>/Engine_Pwr_Calculation'
                                                                  *   '<S22>/HEV_Pwr_Calculation'
                                                                  *   '<S22>/ReGen_Pwr_Calculation'
                                                                  */
  real_T EngMaxSpd;                                              /* Variable: EngMaxSpd
                                                                  * Referenced by: '<S29>/Saturation'
                                                                  */
  real_T HEV_Mode_Duration;                                      /* Variable: HEV_Mode_Duration
                                                                  * Referenced by: '<S2>/HEV_Mode_Duration'
                                                                  */
  real_T Inverse_EngOptimal_Pwr_KW[tx_car::kMap1dSize];    /* Variable: Inverse_EngOptimal_Pwr_KW
                                                                  * Referenced by:
                                                                  * '<S20>/EV_Pwr_Calculation'
                                                                  */
  real_T Inverse_EngOptimal_Speed_RPM[tx_car::kMap1dSize]; /* Variable: Inverse_EngOptimal_Speed_RPM
                                                                  * Referenced by: '<S20>/EV_Pwr_Calculation'
                                                                  */
  real_T abs_brake_pressure;                                     /* Variable: abs_brake_pressure
                                                                  * Referenced by:
                                                                  *   '<S55>/abs_brake_pressure_01'
                                                                  *   '<S56>/abs_brake_pressure_01'
                                                                  */
  real_T brake_D;                                                /* Variable: brake_D
                                                                  * Referenced by: '<S84>/Derivative Gain'
                                                                  */
  real_T brake_I;                                                /* Variable: brake_I
                                                                  * Referenced by: '<S87>/Integral Gain'
                                                                  */
  real_T brake_P;                                                /* Variable: brake_P
                                                                  * Referenced by: '<S95>/Proportional Gain'
                                                                  */
  real_T ecu_max_acc;                                            /* Variable: ecu_max_acc
                                                                  * Referenced by: '<S121>/Constant'
                                                                  */
  real_T ecu_max_dec;                                            /* Variable: ecu_max_dec
                                                                  * Referenced by: '<S55>/Constant'
                                                                  */
  real_T enable_abs;                                             /* Variable: enable_abs
                                                                  * Referenced by: '<S49>/abs_switch'
                                                                  */
  real_T front_diff_ratio;                                       /* Variable: front_diff_ratio
                                                                  * Referenced by: '<S5>/front_diff_ratio'
                                                                  */
  real_T front_motor_ratio;                                      /* Variable: front_motor_ratio
                                                                  * Referenced by: '<S5>/front_motor_ratio'
                                                                  */
  real_T hcu_acc_D;                                              /* Variable: hcu_acc_D
                                                                  * Referenced by: '<S160>/Derivative Gain'
                                                                  */
  real_T hcu_acc_I;                                              /* Variable: hcu_acc_I
                                                                  * Referenced by: '<S163>/Integral Gain'
                                                                  */
  real_T hcu_acc_P;                                              /* Variable: hcu_acc_P
                                                                  * Referenced by: '<S171>/Proportional Gain'
                                                                  */
  real_T slip_disable_ratio_ref;                                 /* Variable: slip_disable_ratio_ref
                                                                  * Referenced by: '<S49>/OnAtRef_OffAt0.5Ref'
                                                                  */
  real_T slip_ratio_ref;                                         /* Variable: slip_ratio_ref
                                                                  * Referenced by: '<S49>/OnAtRef_OffAt0.5Ref'
                                                                  */
  real_T softecu_ax_delay;                                       /* Variable: softecu_ax_delay
                                                                  * Referenced by: '<S187>/AxDelayStep'
                                                                  */
  real_T speedOffset;                                            /* Variable: speedOffset
                                                                  * Referenced by:
                                                                  *   '<S116>/Constant'
                                                                  *   '<S124>/Constant'
                                                                  *   '<S125>/Constant'
                                                                  *   '<S111>/Constant'
                                                                  *   '<S112>/Constant'
                                                                  */
  real_T stepTime;                                               /* Variable: stepTime
                                                                  * Referenced by:
                                                                  *   '<S2>/StepTime'
                                                                  *   '<S187>/Gain'
                                                                  */
  real_T total_torque_data[154];                                 /* Variable: total_torque_data
                                                                  * Referenced by: '<S5>/Total Torque Request Table'
                                                                  */
  real_T total_torque_speed_bpt[14];                             /* Variable: total_torque_speed_bpt
                                                                  * Referenced by: '<S5>/Total Torque Request Table'
                                                                  */
  real_T total_torque_throttle_bpt[11];                          /* Variable: total_torque_throttle_bpt
                                                                  * Referenced by: '<S5>/Total Torque Request Table'
                                                                  */
  real_T SoftHCU_AirDensity;                                     /* Mask Parameter: SoftHCU_AirDensity
                                                                  * Referenced by: '<S184>/AeroDragForce'
                                                                  */
  real_T HEVController_BrakeEnableThresh;
  /* Mask Parameter: HEVController_BrakeEnableThresh
   * Referenced by: '<S45>/Constant'
   */
  real_T HEVController_DefaultChrgPwr;
  /* Mask Parameter: HEVController_DefaultChrgPwr
   * Referenced by:
   *   '<S22>/EV_Pwr_Calculation'
   *   '<S22>/Engine_Pwr_Calculation'
   */
  real_T HEVController_EngOptimal_Pwr_KW[tx_car::kMap1dSize];
  /* Mask Parameter: HEVController_EngOptimal_Pwr_KW
   * Referenced by:
   *   '<S22>/Engine_Pwr_Calculation'
   *   '<S22>/HEV_Pwr_Calculation'
   */
  real_T HEVController_EngOptimal_Speed_RPM[tx_car::kMap1dSize];
  /* Mask Parameter: HEVController_EngOptimal_Speed_RPM
   * Referenced by:
   *   '<S22>/Engine_Pwr_Calculation'
   *   '<S22>/HEV_Pwr_Calculation'
   */
  real_T HEVController_EngineStepInSpeed_kph;
  /* Mask Parameter: HEVController_EngineStepInSpeed_kph
   * Referenced by: '<S2>/EngineStepInSpeed_kph'
   */
  real_T HEVController_FrontMotor_MaxTorque[tx_car::kMap1dSize];
  /* Mask Parameter: HEVController_FrontMotor_MaxTorque
   * Referenced by:
   *   '<S8>/Front_MotorMaxTrq'
   *   '<S22>/ReGen_Pwr_Calculation'
   */
  real_T HEVController_FrontMotor_Speed_rads[tx_car::kMap1dSize];
  /* Mask Parameter: HEVController_FrontMotor_Speed_rads
   * Referenced by:
   *   '<S8>/Front_MotorMaxTrq'
   *   '<S22>/ReGen_Pwr_Calculation'
   */
  real_T HEVController_HEV_4WD_FrontPwrRatio;
  /* Mask Parameter: HEVController_HEV_4WD_FrontPwrRatio
   * Referenced by:
   *   '<S22>/EV_Pwr_Calculation'
   *   '<S22>/HEV_Pwr_Calculation'
   */
  real_T DiscretePIDController_InitialConditionForFilter;
  /* Mask Parameter: DiscretePIDController_InitialConditionForFilter
   * Referenced by: '<S85>/Filter'
   */
  real_T DiscretePIDController_InitialConditionForFilter_p;
  /* Mask Parameter: DiscretePIDController_InitialConditionForFilter_p
   * Referenced by: '<S161>/Filter'
   */
  real_T DiscretePIDController_InitialConditionForIntegrator;
  /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator
   * Referenced by: '<S90>/Integrator'
   */
  real_T DiscretePIDController_InitialConditionForIntegrator_e;
  /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator_e
   * Referenced by: '<S166>/Integrator'
   */
  real_T DiscretePIDController_LowerSaturationLimit;
  /* Mask Parameter: DiscretePIDController_LowerSaturationLimit
   * Referenced by: '<S97>/Saturation'
   */
  real_T DiscretePIDController_LowerSaturationLimit_n;
  /* Mask Parameter: DiscretePIDController_LowerSaturationLimit_n
   * Referenced by:
   *   '<S173>/Saturation'
   *   '<S159>/DeadZone'
   */
  real_T HEVController_MaxChrgPwr;  /* Mask Parameter: HEVController_MaxChrgPwr
                                     * Referenced by:
                                     *   '<S22>/EV_Pwr_Calculation'
                                     *   '<S22>/Engine_Pwr_Calculation'
                                     */
  real_T SoftHCU_MaxPwrOfSystem;    /* Mask Parameter: SoftHCU_MaxPwrOfSystem
                                     * Referenced by: '<S121>/MaxPwrOfSystem'
                                     */
  real_T HEVController_MaxReGenPwr; /* Mask Parameter: HEVController_MaxReGenPwr
                                     * Referenced by: '<S22>/ReGen_Pwr_Calculation'
                                     */
  real_T HEVController_MinSocDriveVehicle;
  /* Mask Parameter: HEVController_MinSocDriveVehicle
   * Referenced by: '<S2>/MinSocDriveVehicle'
   */
  real_T DivProtect_MinVal;         /* Mask Parameter: DivProtect_MinVal
                                     * Referenced by:
                                     *   '<S14>/Constant'
                                     *   '<S16>/Constant'
                                     */
  real_T DivProtect_MinVal_p;       /* Mask Parameter: DivProtect_MinVal_p
                                     * Referenced by:
                                     *   '<S18>/Constant'
                                     *   '<S20>/Constant'
                                     */
  real_T DiscretePIDController_N;   /* Mask Parameter: DiscretePIDController_N
                                     * Referenced by: '<S93>/Filter Coefficient'
                                     */
  real_T DiscretePIDController_N_m; /* Mask Parameter: DiscretePIDController_N_m
                                     * Referenced by: '<S169>/Filter Coefficient'
                                     */
  real_T HEVController_P4_Motor_MaxTorque[tx_car::kMap1dSize];
  /* Mask Parameter: HEVController_P4_Motor_MaxTorque
   * Referenced by:
   *   '<S10>/P4_MotorMaxTrq'
   *   '<S22>/ReGen_Pwr_Calculation'
   */
  real_T HEVController_P4_Motor_Speed_rads[tx_car::kMap1dSize];
  /* Mask Parameter: HEVController_P4_Motor_Speed_rads
   * Referenced by:
   *   '<S10>/P4_MotorMaxTrq'
   *   '<S22>/ReGen_Pwr_Calculation'
   */
  real_T HEVController_PwrDmnd_EV_Only;
  /* Mask Parameter: HEVController_PwrDmnd_EV_Only
   * Referenced by: '<S2>/PwrDmnd_EV_Only'
   */
  real_T HEVController_PwrDmnd_Hybrid;
  /* Mask Parameter: HEVController_PwrDmnd_Hybrid
   * Referenced by: '<S2>/PwrDmnd_Hybrid'
   */
  real_T SoftHCU_RollingCoeff; /* Mask Parameter: SoftHCU_RollingCoeff
                                * Referenced by: '<S184>/RollingCoeff'
                                */
  real_T HEVController_StartChargingSoc;
  /* Mask Parameter: HEVController_StartChargingSoc
   * Referenced by: '<S2>/StartChargingSoc'
   */
  real_T HEVController_TargetSoc; /* Mask Parameter: HEVController_TargetSoc
                                   * Referenced by: '<S2>/TargetSoc'
                                   */
  real_T HEVController_ThrottleDisableThresh;
  /* Mask Parameter: HEVController_ThrottleDisableThresh
   * Referenced by: '<S46>/Constant'
   */
  real_T DiscretePIDController_UpperSaturationLimit;
  /* Mask Parameter: DiscretePIDController_UpperSaturationLimit
   * Referenced by: '<S97>/Saturation'
   */
  real_T DiscretePIDController_UpperSaturationLimit_p;
  /* Mask Parameter: DiscretePIDController_UpperSaturationLimit_p
   * Referenced by:
   *   '<S173>/Saturation'
   *   '<S159>/DeadZone'
   */
  real_T CompareToConstant_const; /* Mask Parameter: CompareToConstant_const
                                   * Referenced by: '<S113>/Constant'
                                   */
  real_T brakeEnableThresh_Reverse_const;
  /* Mask Parameter: brakeEnableThresh_Reverse_const
   * Referenced by: '<S108>/Constant'
   */
  real_T brakeEnableThresh_const;   /* Mask Parameter: brakeEnableThresh_const
                                     * Referenced by: '<S107>/Constant'
                                     */
  real_T Accel_const;               /* Mask Parameter: Accel_const
                                     * Referenced by: '<S129>/Constant'
                                     */
  real_T CompareToConstant1_const;  /* Mask Parameter: CompareToConstant1_const
                                     * Referenced by: '<S130>/Constant'
                                     */
  real_T CompareToConstant_const_m; /* Mask Parameter: CompareToConstant_const_m
                                     * Referenced by: '<S185>/Constant'
                                     */
  real_T CompareToConstant1_const_l;
  /* Mask Parameter: CompareToConstant1_const_l
   * Referenced by: '<S128>/Constant'
   */
  real_T CompareToConstant_const_p; /* Mask Parameter: CompareToConstant_const_p
                                     * Referenced by: '<S122>/Constant'
                                     */
  real_T CompareToConstant1_const_k;
  /* Mask Parameter: CompareToConstant1_const_k
   * Referenced by: '<S123>/Constant'
   */
  real_T CompareToConstant_const_h;         /* Mask Parameter: CompareToConstant_const_h
                                             * Referenced by: '<S118>/Constant'
                                             */
  real_T VXLOW_ReGen_const;                 /* Mask Parameter: VXLOW_ReGen_const
                                             * Referenced by: '<S48>/Constant'
                                             */
  real_T Gear_DriveMode_const;              /* Mask Parameter: Gear_DriveMode_const
                                             * Referenced by: '<S47>/Constant'
                                             */
  uint8_T reverseGear_const;                /* Mask Parameter: reverseGear_const
                                             * Referenced by: '<S110>/Constant'
                                             */
  uint8_T notReverse_const;                 /* Mask Parameter: notReverse_const
                                             * Referenced by: '<S109>/Constant'
                                             */
  uint8_T reverseGear_const_n;              /* Mask Parameter: reverseGear_const_n
                                             * Referenced by: '<S127>/Constant'
                                             */
  uint8_T forwardGear_const;                /* Mask Parameter: forwardGear_const
                                             * Referenced by: '<S126>/Constant'
                                             */
  real_T Gain_Gain;                         /* Expression: -1
                                             * Referenced by: '<S8>/Gain'
                                             */
  real_T Gain_Gain_f;                       /* Expression: -1
                                             * Referenced by: '<S10>/Gain'
                                             */
  real_T Switch_Threshold;                  /* Expression: 0
                                             * Referenced by: '<S56>/Switch'
                                             */
  real_T Filter_gainval;                    /* Computed Parameter: Filter_gainval
                                             * Referenced by: '<S85>/Filter'
                                             */
  real_T Integrator_gainval;                /* Computed Parameter: Integrator_gainval
                                             * Referenced by: '<S90>/Integrator'
                                             */
  real_T abs_Threshold;                     /* Expression: 0.5
                                             * Referenced by: '<S55>/abs'
                                             */
  real_T OnAtRef_OffAt05Ref_YOn;            /* Expression: 1
                                             * Referenced by: '<S49>/OnAtRef_OffAt0.5Ref'
                                             */
  real_T OnAtRef_OffAt05Ref_YOff;           /* Expression: 0
                                             * Referenced by: '<S49>/OnAtRef_OffAt0.5Ref'
                                             */
  real_T Constant1_Value[4];                /* Expression: [1 1 1 1]
                                             * Referenced by: '<S49>/Constant1'
                                             */
  real_T Memory_InitialCondition;           /* Expression: 0
                                             * Referenced by: '<S49>/Memory'
                                             */
  real_T Saturation_UpperSat;               /* Expression: 1
                                             * Referenced by: '<S49>/Saturation'
                                             */
  real_T Saturation_LowerSat;               /* Expression: 0
                                             * Referenced by: '<S49>/Saturation'
                                             */
  real_T Saturation_UpperSat_a;             /* Expression: 1
                                             * Referenced by: '<S121>/Saturation'
                                             */
  real_T Saturation_LowerSat_n;             /* Expression: 0.0178
                                             * Referenced by: '<S121>/Saturation'
                                             */
  real_T Constant1_Value_h;                 /* Expression: 0
                                             * Referenced by: '<S157>/Constant1'
                                             */
  real_T Constant2_Value;                   /* Expression: 0
                                             * Referenced by: '<S121>/Constant2'
                                             */
  real_T Clamping_zero_Value;               /* Expression: 0
                                             * Referenced by: '<S157>/Clamping_zero'
                                             */
  real_T TargetAccDeadZone_Start;           /* Expression: -0.01
                                             * Referenced by: '<S121>/TargetAccDeadZone'
                                             */
  real_T TargetAccDeadZone_End;             /* Expression: 0.01
                                             * Referenced by: '<S121>/TargetAccDeadZone'
                                             */
  real_T Integrator_gainval_g;              /* Computed Parameter: Integrator_gainval_g
                                             * Referenced by: '<S166>/Integrator'
                                             */
  real_T Filter_gainval_j;                  /* Computed Parameter: Filter_gainval_j
                                             * Referenced by: '<S161>/Filter'
                                             */
  real_T ZeroPwr_Value;                     /* Expression: 0
                                             * Referenced by: '<S183>/ZeroPwr'
                                             */
  real_T ZeroPwr_Value_a;                   /* Expression: 0
                                             * Referenced by: '<S121>/ZeroPwr'
                                             */
  real_T Saturation_UpperSat_g;             /* Expression: 1
                                             * Referenced by: '<S120>/Saturation'
                                             */
  real_T Saturation_LowerSat_c;             /* Expression: 0
                                             * Referenced by: '<S120>/Saturation'
                                             */
  real_T Zero_Value;                        /* Expression: 0
                                             * Referenced by: '<S117>/Zero'
                                             */
  real_T fliper_Value;                      /* Expression: -1
                                             * Referenced by: '<S119>/fliper'
                                             */
  real_T no_throttle_Value;                 /* Expression: 0
                                             * Referenced by: '<S51>/no_throttle'
                                             */
  real_T Saturation_UpperSat_o;             /* Expression: 1
                                             * Referenced by: '<S51>/Saturation'
                                             */
  real_T Saturation_LowerSat_p;             /* Expression: 0
                                             * Referenced by: '<S51>/Saturation'
                                             */
  real_T Constant_Value;                    /* Expression: 0
                                             * Referenced by: '<S17>/Constant'
                                             */
  real_T Constant_Value_h;                  /* Expression: 0
                                             * Referenced by: '<S21>/Constant'
                                             */
  real_T autoHoldBrakePressure_01_Value;    /* Expression: 0.6
                                             * Referenced by: '<S4>/autoHoldBrakePressure_01'
                                             */
  real_T Switch1_Threshold;                 /* Expression: 0.5
                                             * Referenced by: '<S4>/Switch1'
                                             */
  real_T RelayCutOffAt6_EnableAt5_OnVal;    /* Expression: -5
                                             * Referenced by: '<S54>/RelayCutOffAt6_EnableAt5'
                                             */
  real_T RelayCutOffAt6_EnableAt5_OffVal;   /* Expression: -6
                                             * Referenced by: '<S54>/RelayCutOffAt6_EnableAt5'
                                             */
  real_T RelayCutOffAt6_EnableAt5_YOn;      /* Expression: 0
                                             * Referenced by: '<S54>/RelayCutOffAt6_EnableAt5'
                                             */
  real_T RelayCutOffAt6_EnableAt5_YOff;     /* Expression: 1
                                             * Referenced by: '<S54>/RelayCutOffAt6_EnableAt5'
                                             */
  real_T RelayCutOffAt58_EnableAt55_OnVal;  /* Expression: 58
                                             * Referenced by: '<S54>/RelayCutOffAt58_EnableAt55'
                                             */
  real_T RelayCutOffAt58_EnableAt55_OffVal; /* Expression: 55
                                             * Referenced by: '<S54>/RelayCutOffAt58_EnableAt55'
                                             */
  real_T RelayCutOffAt58_EnableAt55_YOn;    /* Expression: 1
                                             * Referenced by: '<S54>/RelayCutOffAt58_EnableAt55'
                                             */
  real_T RelayCutOffAt58_EnableAt55_YOff;   /* Expression: 0
                                             * Referenced by: '<S54>/RelayCutOffAt58_EnableAt55'
                                             */
  real_T Constant_Value_d;                  /* Expression: 0
                                             * Referenced by: '<S52>/Constant'
                                             */
  real_T EV_Swtich_Value;                   /* Expression: 1
                                             * Referenced by: '<S13>/EV_Swtich'
                                             */
  real_T Engine_Switch_Value;               /* Expression: 0
                                             * Referenced by: '<S13>/Engine_Switch'
                                             */
  real_T Engine_ChargingSwitch_Value;       /* Expression: 0
                                             * Referenced by: '<S13>/Engine_ChargingSwitch'
                                             */
  real_T Engine_ClutchSwitch_Value;         /* Expression: 0
                                             * Referenced by: '<S13>/Engine_ClutchSwitch'
                                             */
  real_T kph_Gain;                          /* Expression: 3.6
                                             * Referenced by: '<S1>/kph'
                                             */
  real_T Saturation1_UpperSat;              /* Expression: 1
                                             * Referenced by: '<S1>/Saturation1'
                                             */
  real_T Saturation1_LowerSat;              /* Expression: 0
                                             * Referenced by: '<S1>/Saturation1'
                                             */
  real_T Gain_Gain_j;                       /* Expression: 100
                                             * Referenced by: '<S3>/Gain'
                                             */
  real_T Zero_Value_h;                      /* Expression: 0
                                             * Referenced by: '<S5>/Zero'
                                             */
  real_T Gain_Gain_b;                       /* Expression: 3.6
                                             * Referenced by: '<S5>/Gain'
                                             */
  real_T ReverseSpeedLimit_kph_OnVal;       /* Expression: -5*3.6
                                             * Referenced by: '<S5>/ReverseSpeedLimit_kph'
                                             */
  real_T ReverseSpeedLimit_kph_OffVal;      /* Expression: -6*3.6
                                             * Referenced by: '<S5>/ReverseSpeedLimit_kph'
                                             */
  real_T ReverseSpeedLimit_kph_YOn;         /* Expression: 1
                                             * Referenced by: '<S5>/ReverseSpeedLimit_kph'
                                             */
  real_T ReverseSpeedLimit_kph_YOff;        /* Expression: 0
                                             * Referenced by: '<S5>/ReverseSpeedLimit_kph'
                                             */
  real_T DeadZone_Start;                    /* Expression: 0
                                             * Referenced by: '<S5>/Dead Zone'
                                             */
  real_T DeadZone_End;                      /* Expression: ThrottleDisableThresh
                                             * Referenced by: '<S5>/Dead Zone'
                                             */
  real_T ThrottleSaturation_UpperSat;       /* Expression: 1
                                             * Referenced by: '<S5>/ThrottleSaturation'
                                             */
  real_T ThrottleSaturation_LowerSat;       /* Expression: 0
                                             * Referenced by: '<S5>/ThrottleSaturation'
                                             */
  real_T EV_Swtich_Value_m;                 /* Expression: 0
                                             * Referenced by: '<S9>/EV_Swtich'
                                             */
  real_T Engine_Switch_Value_b;             /* Expression: 1
                                             * Referenced by: '<S9>/Engine_Switch'
                                             */
  real_T Engine_ChargingSwitch_Value_i;     /* Expression: 0
                                             * Referenced by: '<S9>/Engine_ChargingSwitch'
                                             */
  real_T Engine_ClutchSwitch_Value_o;       /* Expression: 1
                                             * Referenced by: '<S9>/Engine_ClutchSwitch'
                                             */
  real_T rpm_Gain;                          /* Expression: 30/pi
                                             * Referenced by: '<S7>/rpm'
                                             */
  real_T Constant1_Value_i;                 /* Expression: 1
                                             * Referenced by: '<S14>/Constant1'
                                             */
  real_T Constant2_Value_i;                 /* Expression: -1
                                             * Referenced by: '<S14>/Constant2'
                                             */
  real_T Constant1_Value_c;                 /* Expression: 1
                                             * Referenced by: '<S18>/Constant1'
                                             */
  real_T Constant2_Value_g;                 /* Expression: -1
                                             * Referenced by: '<S18>/Constant2'
                                             */
  uint32_T HEV_Mode_Value;                  /* Computed Parameter: HEV_Mode_Value
                                             * Referenced by: '<S13>/HEV_Mode'
                                             */
  uint32_T TotalTorqueRequestTable_maxIndex[2];
  /* Computed Parameter: TotalTorqueRequestTable_maxIndex
   * Referenced by: '<S5>/Total Torque Request Table'
   */
  uint32_T HEV_Mode_Value_d;                                          /* Computed Parameter: HEV_Mode_Value_d
                                                                       * Referenced by: '<S9>/HEV_Mode'
                                                                       */
  int8_T Constant_Value_e;                                            /* Computed Parameter: Constant_Value_e
                                                                       * Referenced by: '<S157>/Constant'
                                                                       */
  int8_T Constant2_Value_p;                                           /* Computed Parameter: Constant2_Value_p
                                                                       * Referenced by: '<S157>/Constant2'
                                                                       */
  int8_T Constant3_Value;                                             /* Computed Parameter: Constant3_Value
                                                                       * Referenced by: '<S157>/Constant3'
                                                                       */
  int8_T Constant4_Value;                                             /* Computed Parameter: Constant4_Value
                                                                       * Referenced by: '<S157>/Constant4'
                                                                       */
  P_ax_estimator_HEVControllerPwr_T ax_estimator;                     /* '<S4>/ax_estimator' */
  P_ReGen_Pwr_Calculation_HEVControllerPwr_T ReGen_Pwr_Calculation;   /* '<S22>/ReGen_Pwr_Calculation' */
  P_HEV_Pwr_Calculation_HEVControllerPwr_T HEV_Pwr_Calculation;       /* '<S22>/HEV_Pwr_Calculation' */
  P_Engine_Pwr_Calculation_HEVControllerPwr_T Engine_Pwr_Calculation; /* '<S22>/Engine_Pwr_Calculation' */
  P_EV_Pwr_Calculation_HEVControllerPwr_T EV_Pwr_Calculation;         /* '<S22>/EV_Pwr_Calculation' */
};

/* Real-time Model Data Structure */
struct tag_RTM_HEVControllerPwr_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_HEVControllerPwr_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_HEVControllerPwr_T *contStateDisabled;
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
    time_T *t;
    time_T tArray[2];
  } Timing;
};

/* Class declaration for model HEVControllerPwr */
class MODULE_API HEVControllerPwr {
  /* public data and function members */
 public:
  /* Copy Constructor */
  HEVControllerPwr(HEVControllerPwr const &) = delete;

  /* Assignment Operator */
  HEVControllerPwr &operator=(HEVControllerPwr const &) & = delete;

  /* Move Constructor */
  HEVControllerPwr(HEVControllerPwr &&) = delete;

  /* Move Assignment Operator */
  HEVControllerPwr &operator=(HEVControllerPwr &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_HEVControllerPwr_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_HEVControllerPwr_T *pExtU_HEVControllerPwr_T) {
    HEVControllerPwr_U = *pExtU_HEVControllerPwr_T;
  }

  /* Root outports get method */
  const ExtY_HEVControllerPwr_T &getExternalOutputs() const { return HEVControllerPwr_Y; }

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  static void terminate();

  /* Constructor */
  HEVControllerPwr();

  /* Destructor */
  ~HEVControllerPwr();

 protected:
  /* Tunable parameters */
  static P_HEVControllerPwr_T HEVControllerPwr_P;

  static uint32_t m_EngOptimal_Pwr_MaxIndex;      // = 10;
  static uint32_t m_Inv_EngOptimal_Pwr_MaxIndex;  // = 10;
  static uint32_t m_FrontMot_TrqTable_MaxIndex;   // = 12;
  static uint32_t m_P4Mot_TrqTable_MaxIndex;      // = 12;

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_HEVControllerPwr_T HEVControllerPwr_U;

  /* External outputs */
  ExtY_HEVControllerPwr_T HEVControllerPwr_Y;

  /* Block signals */
  B_HEVControllerPwr_T HEVControllerPwr_B;

  /* Block states */
  DW_HEVControllerPwr_T HEVControllerPwr_DW;

  /* Tunable parameters */
  // static P_HEVControllerPwr_T HEVControllerPwr_P;

  /* Block continuous states */
  X_HEVControllerPwr_T HEVControllerPwr_X;

  /* private member function(s) for subsystem '<S23>/NoPwrMode'*/
  static void HEVControllerPwr_NoPwrMode(HEVPwrDmnd_Output *rty_HEV_PwrOutput, real_T rtp_EngIdleSpd,
                                         P_NoPwrMode_HEVControllerPwr_T *localP);

  /* private member function(s) for subsystem '<S22>/EV_Pwr_Calculation'*/
  static void HEVControllerPwr_EV_Pwr_Calculation_Init(B_EV_Pwr_Calculation_HEVControllerPwr_T *localB,
                                                       P_EV_Pwr_Calculation_HEVControllerPwr_T *localP);
  static void HEVControllerPwr_EV_Pwr_Calculation(
      const HEVMode_Output *rtu_HEV_Mode, real_T rtu_Pwr_Dmnd_w_, real_T rtp_HEV_4WD_FrontPwrRatio, real_T rtp_EnableP4,
      real_T rtp_EngIdleSpd, real_T rtp_MaxChrgPwr, real_T rtp_DefaultChrgPwr,
      const real_T rtp_Inverse_EngOptimal_Speed_RPM[11], const real_T rtp_Inverse_EngOptimal_Pwr_KW[11],
      B_EV_Pwr_Calculation_HEVControllerPwr_T *localB, P_EV_Pwr_Calculation_HEVControllerPwr_T *localP,
      P_HEVControllerPwr_T *HEVControllerPwr_P);

  /* private member function(s) for subsystem '<S22>/Engine_Pwr_Calculation'*/
  static void HEVControllerPwr_Engine_Pwr_Calculation_Init(B_Engine_Pwr_Calculation_HEVControllerPwr_T *localB,
                                                           P_Engine_Pwr_Calculation_HEVControllerPwr_T *localP);
  static void HEVControllerPwr_Engine_Pwr_Calculation(const HEVMode_Output *rtu_HEV_Mode, real_T rtu_Pwr_Dmnd_w_,
                                                      real_T rtu_Engine_Speed_rpm_, real_T rtp_DefaultChrgPwr,
                                                      const real_T rtp_EngOptimal_Pwr_KW[11],
                                                      const real_T rtp_EngOptimal_Speed_RPM[11], real_T rtp_MaxChrgPwr,
                                                      real_T rtp_EngIdleSpd,
                                                      B_Engine_Pwr_Calculation_HEVControllerPwr_T *localB,
                                                      P_Engine_Pwr_Calculation_HEVControllerPwr_T *localP);

  /* private member function(s) for subsystem '<S22>/HEV_Pwr_Calculation'*/
  static void HEVControllerPwr_HEV_Pwr_Calculation_Init(B_HEV_Pwr_Calculation_HEVControllerPwr_T *localB,
                                                        P_HEV_Pwr_Calculation_HEVControllerPwr_T *localP);
  static void HEVControllerPwr_HEV_Pwr_Calculation(const HEVMode_Output *rtu_HEV_Mode, real_T rtu_Pwr_Dmnd_w_,
                                                   real_T rtu_Engine_Speed_rpm_, const real_T rtp_EngOptimal_Pwr_KW[11],
                                                   const real_T rtp_EngOptimal_Speed_RPM[11],
                                                   real_T rtp_HEV_4WD_FrontPwrRatio, real_T rtp_EnableP4,
                                                   real_T rtp_EngIdleSpd,
                                                   B_HEV_Pwr_Calculation_HEVControllerPwr_T *localB,
                                                   P_HEV_Pwr_Calculation_HEVControllerPwr_T *localP);

  /* private member function(s) for subsystem '<S22>/ReGen_Pwr_Calculation'*/
  static void HEVControllerPwr_ReGen_Pwr_Calculation_Init(B_ReGen_Pwr_Calculation_HEVControllerPwr_T *localB,
                                                          P_ReGen_Pwr_Calculation_HEVControllerPwr_T *localP);
  static void HEVControllerPwr_ReGen_Pwr_Calculation(
      const HEVMode_Output *rtu_HEV_Mode, real_T rtu_Motor_Speed_rads_, real_T rtu_P4_Motor_Speed_rads_,
      const real_T rtp_FrontMotor_MaxTorque[13], const real_T rtp_FrontMotor_Speed_rads[13], real_T rtp_EnableP4,
      const real_T rtp_P4_Motor_MaxTorque[13], const real_T rtp_P4_Motor_Speed_rads[13], real_T rtp_EngIdleSpd,
      real_T rtp_MaxReGenPwr, B_ReGen_Pwr_Calculation_HEVControllerPwr_T *localB,
      P_ReGen_Pwr_Calculation_HEVControllerPwr_T *localP);

  /* private member function(s) for subsystem '<S4>/ABS_EBD'*/
  void HEVControllerPwr_ABS_EBD_Init();
  void HEVControllerPwr_ABS_EBD_Update();
  void HEVControllerPwr_ABS_EBD();

  /* private member function(s) for subsystem '<S4>/HCU'*/
  void HEVControllerPwr_HCU_Init();
  void HEVControllerPwr_HCU_Update();
  void HEVControllerPwr_HCU();

  /* private member function(s) for subsystem '<S4>/ax_estimator'*/
  static void HEVControllerPwr_ax_estimator_Init(DW_ax_estimator_HEVControllerPwr_T *localDW,
                                                 P_ax_estimator_HEVControllerPwr_T *localP,
                                                 X_ax_estimator_HEVControllerPwr_T *localX);
  static void HEVControllerPwr_ax_estimator_Deriv(B_ax_estimator_HEVControllerPwr_T *localB,
                                                  XDot_ax_estimator_HEVControllerPwr_T *localXdot);
  void HEVControllerPwr_ax_estimator_Update(real_T rtu_vx, DW_ax_estimator_HEVControllerPwr_T *localDW);
  void HEVControllerPwr_ax_estimator(real_T rtu_acc_feedback_m_s2, real_T rtu_vx,
                                     B_ax_estimator_HEVControllerPwr_T *localB,
                                     DW_ax_estimator_HEVControllerPwr_T *localDW,
                                     P_ax_estimator_HEVControllerPwr_T *localP,
                                     P_HEVControllerPwr_T *HEVControllerPwr_P,
                                     X_ax_estimator_HEVControllerPwr_T *localX);

  /* private member function(s) for subsystem '<Root>'*/
  real_T HEVControllerPwr_from_hev_to_engine(real_T hevmode_in_SoC_MinVehicleDrive,
                                             real_T hevmode_in_Speed_EngineStepIn, real_T hevmode_in_PwrDmnd_EVOnly,
                                             real_T hevmode_in_PwrDmnd_Hybrid, real_T hevmode_in_SoC_Battery,
                                             real_T hevmode_in_PwrDmnd, real_T hevmode_in_Vehicle_Speed);
  real_T HEVControllerPwr_from_hev_to_ev(real_T hevmode_in_SoC_MinVehicleDrive, real_T hevmode_in_Speed_EngineStepIn,
                                         real_T hevmode_in_PwrDmnd_EVOnly, real_T hevmode_in_SoC_Battery,
                                         real_T hevmode_in_PwrDmnd, real_T hevmode_in_Vehicle_Speed);
  void HEVControllerPwr_charging_decision(real_T hevmode_in_SoC_StartCharging, real_T hevmode_in_SoC_Battery,
                                          HEVMode_Output *hevmode_out);
  void HEVControllerPwr_serialize_drive_mode(real_T hevmode_in_SoC_StartCharging, real_T hevmode_in_SoC_Battery,
                                             HEVMode_Output *hevmode_out);
  real_T HEVControllerPwr_from_ev_to_engine(real_T hevmode_in_SoC_MinVehicleDrive, real_T hevmode_in_Speed_EngineStepIn,
                                            real_T hevmode_in_PwrDmnd_EVOnly, real_T hevmode_in_PwrDmnd_Hybrid,
                                            real_T hevmode_in_SoC_Battery, real_T hevmode_in_PwrDmnd,
                                            real_T hevmode_in_Vehicle_Speed);
  real_T HEVControllerPwr_from_engine_to_hev(real_T hevmode_in_SoC_MinVehicleDrive, real_T hevmode_in_PwrDmnd_Hybrid,
                                             real_T hevmode_in_SoC_Battery, real_T hevmode_in_PwrDmnd);
  real_T HEVControllerPwr_from_engine_to_ev(real_T hevmode_in_SoC_MinVehicleDrive, real_T hevmode_in_Speed_EngineStepIn,
                                            real_T hevmode_in_PwrDmnd_EVOnly, real_T hevmode_in_SoC_Battery,
                                            real_T hevmode_in_PwrDmnd, real_T hevmode_in_Vehicle_Speed);

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void HEVControllerPwr_derivatives();

  /* Real-Time Model */
  RT_MODEL_HEVControllerPwr_T HEVControllerPwr_M;
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
 * '<Root>' : 'HEVControllerPwr'
 * '<S1>'   : 'HEVControllerPwr/SoftHCU'
 * '<S2>'   : 'HEVControllerPwr/SoftHCU/HEVController'
 * '<S3>'   : 'HEVControllerPwr/SoftHCU/SignalRouter'
 * '<S4>'   : 'HEVControllerPwr/SoftHCU/SoftHCU'
 * '<S5>'   : 'HEVControllerPwr/SoftHCU/Throttle2PwrDmnd'
 * '<S6>'   : 'HEVControllerPwr/SoftHCU/HEVController/HEV_DriveMode'
 * '<S7>'   : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution'
 * '<S8>'   : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/EV_Trq_Calculation'
 * '<S9>'   : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/Engine_HEVMode_For_Reverse'
 * '<S10>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/P4_EV_Trq_Calculation'
 * '<S11>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation'
 * '<S12>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/ReGenSwitch'
 * '<S13>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/ReGen_HEVMode'
 * '<S14>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/EV_Trq_Calculation/DivProtect'
 * '<S15>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/EV_Trq_Calculation/Saturation Dynamic'
 * '<S16>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/EV_Trq_Calculation/DivProtect/Compare To
 * Constant'
 * '<S17>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/EV_Trq_Calculation/DivProtect/Compare To Zero'
 * '<S18>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/P4_EV_Trq_Calculation/DivProtect'
 * '<S19>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/P4_EV_Trq_Calculation/Saturation Dynamic'
 * '<S20>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/P4_EV_Trq_Calculation/DivProtect/Compare To
 * Constant'
 * '<S21>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/P4_EV_Trq_Calculation/DivProtect/Compare To
 * Zero'
 * '<S22>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal'
 * '<S23>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/EV_Pwr_Calculation'
 * '<S24>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/Engine_Pwr_Calculation'
 * '<S25>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/HEV_Pwr_Calculation'
 * '<S26>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/ReGen_Pwr_Calculation'
 * '<S27>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/EV_Pwr_Calculation/EVPwrMode'
 * '<S28>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/EV_Pwr_Calculation/NoPwrMode'
 * '<S29>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/EV_Pwr_Calculation/SerializePwrMode'
 * '<S30>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/EV_Pwr_Calculation/EVPwrMode/WithP4'
 * '<S31>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/EV_Pwr_Calculation/SerializePwrMode/WithP4'
 * '<S32>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/Engine_Pwr_Calculation/EnginePwrMode'
 * '<S33>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/Engine_Pwr_Calculation/NoPwrMode'
 * '<S34>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/Engine_Pwr_Calculation/EnginePwrMode/Saturation
 * Dynamic'
 * '<S35>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/HEV_Pwr_Calculation/HybridPwrMode'
 * '<S36>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/HEV_Pwr_Calculation/NoPwrMode'
 * '<S37>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/HEV_Pwr_Calculation/HybridPwrMode/OptimalEnginePwrCal'
 * '<S38>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/HEV_Pwr_Calculation/HybridPwrMode/WithP4'
 * '<S39>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/ReGen_Pwr_Calculation/HybridPwrMode'
 * '<S40>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/ReGen_Pwr_Calculation/NoPwrMode'
 * '<S41>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/ReGen_Pwr_Calculation/ReGenPwr'
 * '<S42>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/ReGen_Pwr_Calculation/HybridPwrMode/ReGenPwr_Distribution'
 * '<S43>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/ReGen_Pwr_Calculation/HybridPwrMode/Saturation
 * Dynamic'
 * '<S44>'  :
 * 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/PwrCalculation/PwrCal/ReGen_Pwr_Calculation/HybridPwrMode/Saturation
 * Dynamic1'
 * '<S45>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/ReGenSwitch/Compare To Constant'
 * '<S46>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/ReGenSwitch/Compare To Constant1'
 * '<S47>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/ReGenSwitch/Gear_DriveMode'
 * '<S48>'  : 'HEVControllerPwr/SoftHCU/HEVController/HEV_PwrDistribution/ReGenSwitch/VXLOW_ReGen'
 * '<S49>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD'
 * '<S50>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/AutoHold'
 * '<S51>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU'
 * '<S52>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ZeroPressure'
 * '<S53>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ax_estimator'
 * '<S54>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/speedCutOff'
 * '<S55>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl'
 * '<S56>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/pressureControl'
 * '<S57>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller'
 * '<S58>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Subsystem'
 * '<S59>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Anti-windup'
 * '<S60>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/D Gain'
 * '<S61>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Filter'
 * '<S62>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Filter ICs'
 * '<S63>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/I Gain'
 * '<S64>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain'
 * '<S65>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain Fdbk'
 * '<S66>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Integrator'
 * '<S67>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Integrator ICs'
 * '<S68>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/N Copy'
 * '<S69>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/N Gain'
 * '<S70>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/P Copy'
 * '<S71>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Parallel P Gain'
 * '<S72>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Reset Signal'
 * '<S73>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Saturation'
 * '<S74>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Saturation Fdbk'
 * '<S75>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Sum'
 * '<S76>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Sum Fdbk'
 * '<S77>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode'
 * '<S78>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode Sum'
 * '<S79>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Integral'
 * '<S80>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Ngain'
 * '<S81>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/postSat Signal'
 * '<S82>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/preSat Signal'
 * '<S83>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Anti-windup/Passthrough'
 * '<S84>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/D Gain/Internal Parameters'
 * '<S85>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Filter/Disc. Forward Euler
 * Filter'
 * '<S86>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Filter ICs/Internal IC -
 * Filter'
 * '<S87>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/I Gain/Internal Parameters'
 * '<S88>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain/Passthrough'
 * '<S89>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S90>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Integrator/Discrete'
 * '<S91>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Integrator ICs/Internal IC'
 * '<S92>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/N Copy/Disabled'
 * '<S93>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/N Gain/Internal Parameters'
 * '<S94>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/P Copy/Disabled'
 * '<S95>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Parallel P Gain/Internal
 * Parameters'
 * '<S96>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Reset Signal/External Reset'
 * '<S97>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Saturation/Enabled'
 * '<S98>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Saturation Fdbk/Disabled'
 * '<S99>'  : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Sum/Sum_PID'
 * '<S100>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Sum Fdbk/Disabled'
 * '<S101>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode/Disabled'
 * '<S102>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode
 * Sum/Passthrough'
 * '<S103>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Integral/Passthrough'
 * '<S104>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Ngain/Passthrough'
 * '<S105>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/postSat Signal/Forward_Path'
 * '<S106>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Discrete PID Controller/preSat Signal/Forward_Path'
 * '<S107>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Subsystem/brakeEnableThresh'
 * '<S108>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Subsystem/brakeEnableThresh_Reverse'
 * '<S109>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Subsystem/notReverse'
 * '<S110>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Subsystem/reverseGear'
 * '<S111>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Subsystem/vxCheck'
 * '<S112>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/accControl/Subsystem/vxCheckReverse'
 * '<S113>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ABS_EBD/pressureControl/Compare To Constant'
 * '<S114>' : 'HEVControllerPwr/SoftHCU/SoftHCU/AutoHold/AutoHold'
 * '<S115>' : 'HEVControllerPwr/SoftHCU/SoftHCU/AutoHold/EgoSpeedLow'
 * '<S116>' : 'HEVControllerPwr/SoftHCU/SoftHCU/AutoHold/EgoSpeedLow/AutoHoldSpeedThresh'
 * '<S117>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/CheckIfReset'
 * '<S118>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/Compare To Constant'
 * '<S119>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/accSignAdapter'
 * '<S120>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/throttleControlMode'
 * '<S121>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode'
 * '<S122>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/CheckIfReset/Compare To Constant'
 * '<S123>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/CheckIfReset/Compare To Constant1'
 * '<S124>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/CheckIfReset/Forward'
 * '<S125>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/CheckIfReset/Reverse'
 * '<S126>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/accSignAdapter/forwardGear'
 * '<S127>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/accSignAdapter/reverseGear'
 * '<S128>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/throttleControlMode/Compare To Constant1'
 * '<S129>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Accel'
 * '<S130>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Compare To Constant1'
 * '<S131>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller'
 * '<S132>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/DrivePwrCalculation'
 * '<S133>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Anti-windup'
 * '<S134>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/D Gain'
 * '<S135>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Filter'
 * '<S136>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Filter ICs'
 * '<S137>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/I Gain'
 * '<S138>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Ideal P Gain'
 * '<S139>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Ideal P Gain Fdbk'
 * '<S140>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Integrator'
 * '<S141>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Integrator ICs'
 * '<S142>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/N Copy'
 * '<S143>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/N Gain'
 * '<S144>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/P Copy'
 * '<S145>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Parallel P Gain'
 * '<S146>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Reset Signal'
 * '<S147>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Saturation'
 * '<S148>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Saturation Fdbk'
 * '<S149>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Sum'
 * '<S150>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Sum Fdbk'
 * '<S151>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Tracking Mode'
 * '<S152>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Tracking Mode Sum'
 * '<S153>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Tsamp - Integral'
 * '<S154>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Tsamp - Ngain'
 * '<S155>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/postSat Signal'
 * '<S156>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/preSat Signal'
 * '<S157>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Anti-windup/Disc. Clamping
 * Parallel'
 * '<S158>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Anti-windup/Disc. Clamping
 * Parallel/Dead Zone'
 * '<S159>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Anti-windup/Disc. Clamping
 * Parallel/Dead Zone/Enabled'
 * '<S160>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/D Gain/Internal Parameters'
 * '<S161>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Filter/Disc. Forward Euler
 * Filter'
 * '<S162>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Filter ICs/Internal IC -
 * Filter'
 * '<S163>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/I Gain/Internal Parameters'
 * '<S164>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Ideal P Gain/Passthrough'
 * '<S165>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S166>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Integrator/Discrete'
 * '<S167>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Integrator ICs/Internal IC'
 * '<S168>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/N Copy/Disabled'
 * '<S169>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/N Gain/Internal Parameters'
 * '<S170>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/P Copy/Disabled'
 * '<S171>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Parallel P Gain/Internal
 * Parameters'
 * '<S172>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Reset Signal/External
 * Reset'
 * '<S173>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Saturation/Enabled'
 * '<S174>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Saturation Fdbk/Disabled'
 * '<S175>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Sum/Sum_PID'
 * '<S176>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Sum Fdbk/Disabled'
 * '<S177>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Tracking Mode/Disabled'
 * '<S178>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Tracking Mode
 * Sum/Passthrough'
 * '<S179>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Tsamp -
 * Integral/Passthrough'
 * '<S180>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/Tsamp - Ngain/Passthrough'
 * '<S181>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/postSat
 * Signal/Forward_Path'
 * '<S182>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/Discrete PID Controller/preSat Signal/Forward_Path'
 * '<S183>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/DrivePwrCalculation/AccelPwr'
 * '<S184>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/DrivePwrCalculation/ResistancePwr'
 * '<S185>' : 'HEVControllerPwr/SoftHCU/SoftHCU/HCU/useTargetAccMode/DrivePwrCalculation/AccelPwr/Compare To Constant'
 * '<S186>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ax_estimator/AxAct'
 * '<S187>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ax_estimator/AxCal'
 * '<S188>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ax_estimator/If Action Subsystem'
 * '<S189>' : 'HEVControllerPwr/SoftHCU/SoftHCU/ax_estimator/SpeedThresh'
 */
#endif /* RTW_HEADER_HEVControllerPwr_h_ */
