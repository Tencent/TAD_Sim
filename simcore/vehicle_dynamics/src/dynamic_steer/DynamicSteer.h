/*
 * DynamicSteer.h
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

#ifndef RTW_HEADER_DynamicSteer_h_
#define RTW_HEADER_DynamicSteer_h_
#include "DynamicSteer_types.h"
#include "rt_zcfcn.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rtwtypes.h"

#include "inc/car_common.h"

extern "C" {

#include "rt_nonfinite.h"
}

#include <cstring>
#include "zero_crossing_types.h"

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
struct B_DynamicSteer_T {
  real_T steer_cmd_rad;
  /* '<Root>/BusConversion_InsertedFor_driverIn_at_outport_0' */
  real_T WhlAng[4];          /* '<S1>/Merge' */
  real_T steerAngleFeedback; /* '<S1>/Merge2' */
  real_T steerTrqIn;         /* '<S1>/Merge3' */
  real_T Divide;             /* '<S17>/Divide' */
  real_T Memory;             /* '<S56>/Memory' */
  real_T domega_o1;          /* '<S54>/domega_o1' */
  real_T Memory_f;           /* '<S55>/Memory' */
  real_T domega_o;           /* '<S54>/domega_o' */
  real_T Product;            /* '<S55>/Product' */
  real_T Product_j;          /* '<S56>/Product' */
  real_T Divide_i;           /* '<S51>/Divide' */
  real_T Divide1;            /* '<S53>/Divide1' */
  uint8_T steer_control_mode;
  /* '<Root>/BusConversion_InsertedFor_driverIn_at_outport_0' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_DynamicSteer_T {
  real_T Memory_1_PreviousInput[4];        /* '<S1>/Memory' */
  real_T Memory_2_PreviousInput;           /* '<S1>/Memory' */
  real_T Memory_3_PreviousInput;           /* '<S1>/Memory' */
  real_T Memory_PreviousInput;             /* '<S56>/Memory' */
  real_T Memory_PreviousInput_c;           /* '<S55>/Memory' */
  int_T Integrator_IWORK;                  /* '<S56>/Integrator' */
  int_T Integrator_IWORK_o;                /* '<S55>/Integrator' */
  int_T IntegratorSecondOrder1_MODE;       /* '<S53>/Integrator, Second-Order1' */
  int_T IntegratorSecondOrder_MODE;        /* '<S51>/Integrator, Second-Order' */
  int8_T If_ActiveSubsystem;               /* '<S1>/If' */
  boolean_T IntegratorSecondOrder1_DWORK1; /* '<S53>/Integrator, Second-Order1' */
  boolean_T IntegratorSecondOrder_DWORK1;  /* '<S51>/Integrator, Second-Order' */
};

/* Continuous states (default storage) */
struct X_DynamicSteer_T {
  real_T Integrator1_CSTATE;               /* '<S17>/Integrator1' */
  real_T IntegratorSecondOrder1_CSTATE[2]; /* '<S53>/Integrator, Second-Order1' */
  real_T IntegratorSecondOrder_CSTATE[2];  /* '<S51>/Integrator, Second-Order' */
  real_T Integrator_CSTATE;                /* '<S56>/Integrator' */
  real_T Integrator_CSTATE_o;              /* '<S55>/Integrator' */
  real_T TransferFcn_CSTATE[2];            /* '<S76>/Transfer Fcn' */
};

/* State derivatives (default storage) */
struct XDot_DynamicSteer_T {
  real_T Integrator1_CSTATE;               /* '<S17>/Integrator1' */
  real_T IntegratorSecondOrder1_CSTATE[2]; /* '<S53>/Integrator, Second-Order1' */
  real_T IntegratorSecondOrder_CSTATE[2];  /* '<S51>/Integrator, Second-Order' */
  real_T Integrator_CSTATE;                /* '<S56>/Integrator' */
  real_T Integrator_CSTATE_o;              /* '<S55>/Integrator' */
  real_T TransferFcn_CSTATE[2];            /* '<S76>/Transfer Fcn' */
};

/* State disabled  */
struct XDis_DynamicSteer_T {
  boolean_T Integrator1_CSTATE;               /* '<S17>/Integrator1' */
  boolean_T IntegratorSecondOrder1_CSTATE[2]; /* '<S53>/Integrator, Second-Order1' */
  boolean_T IntegratorSecondOrder_CSTATE[2];  /* '<S51>/Integrator, Second-Order' */
  boolean_T Integrator_CSTATE;                /* '<S56>/Integrator' */
  boolean_T Integrator_CSTATE_o;              /* '<S55>/Integrator' */
  boolean_T TransferFcn_CSTATE[2];            /* '<S76>/Transfer Fcn' */
};

/* Zero-crossing (trigger) state */
struct PrevZCX_DynamicSteer_T {
  ZCSigState Integrator_Reset_ZCE;   /* '<S56>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_g; /* '<S55>/Integrator' */
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
struct ExtU_DynamicSteer_T {
  Ctrl_Input driverIn;         /* '<Root>/driverIn' */
  Vehicle_Body_Output bodyOut; /* '<Root>/bodyOut' */
  Susp_Output suspOut;         /* '<Root>/suspOut' */
  Wheels_Output wheelOut;      /* '<Root>/wheelOut' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_DynamicSteer_T {
  Steer_Output steerOut; /* '<Root>/steerOut' */
};

/* Parameters (default storage) */
struct P_DynamicSteer_T_ {
  real_T DynamicSteering_D;       /* Mask Parameter: DynamicSteering_D
                                   * Referenced by:
                                   *   '<S24>/Constant2'
                                   *   '<S24>/Constant3'
                                   *   '<S24>/Constant9'
                                   *   '<S25>/Constant2'
                                   *   '<S25>/Constant3'
                                   *   '<S25>/Constant9'
                                   *   '<S26>/Constant2'
                                   *   '<S26>/Constant3'
                                   */
  real_T DynamicSteering_Eta;     /* Mask Parameter: DynamicSteering_Eta
                                   * Referenced by: '<S12>/Gain1'
                                   */
  real_T DynamicSteering_FricTrq; /* Mask Parameter: DynamicSteering_FricTrq
                                   * Referenced by:
                                   *   '<S52>/Gain'
                                   *   '<S52>/Gain1'
                                   */
  real_T DynamicSteering_J1;      /* Mask Parameter: DynamicSteering_J1
                                   * Referenced by: '<S51>/Constant'
                                   */
  real_T DynamicSteering_J2;      /* Mask Parameter: DynamicSteering_J2
                                   * Referenced by: '<S53>/Constant3'
                                   */
  real_T DynamicSteering_PnnRadiusTbl[tx_car::kMap1dSize];
  /* Mask Parameter: DynamicSteering_PnnRadiusTbl
   * Referenced by:
   *   '<S21>/1-D Lookup Table'
   *   '<S21>/1-D Lookup Table1'
   */
  real_T DynamicSteering_PwrLmt; /* Mask Parameter: DynamicSteering_PwrLmt
                                  * Referenced by: '<S12>/Saturation2'
                                  */
  real_T MappedSteer_RackDispBpts[tx_car::kMap1dSize];
  /* Mask Parameter: MappedSteer_RackDispBpts
   * Referenced by:
   *   '<S78>/leff_whl_ang'
   *   '<S78>/rack_disp'
   *   '<S78>/right_whl_ang'
   */
  real_T DynamicSteering_RckCsLngth;
  /* Mask Parameter: DynamicSteering_RckCsLngth
   * Referenced by:
   *   '<S24>/Constant1'
   *   '<S25>/Constant1'
   *   '<S26>/Constant1'
   */
  real_T MappedSteer_SpdFctTbl[tx_car::kMap1dSize];   /* Mask Parameter: MappedSteer_SpdFctTbl
                                                             * Referenced by: '<S78>/SpeedCoeff'
                                                             */
  real_T MappedSteer_SteerAngBps[tx_car::kMap1dSize]; /* Mask Parameter: MappedSteer_SteerAngBps
                                                             * Referenced by: '<S78>/rack_disp'
                                                             */
  real_T DynamicSteering_StrgAngBpts[tx_car::kMap1dSize];
  /* Mask Parameter: DynamicSteering_StrgAngBpts
   * Referenced by:
   *   '<S21>/1-D Lookup Table'
   *   '<S21>/1-D Lookup Table1'
   */
  real_T DynamicSteering_StrgArmLngth;
  /* Mask Parameter: DynamicSteering_StrgArmLngth
   * Referenced by:
   *   '<S24>/Constant12'
   *   '<S24>/Constant4'
   *   '<S24>/Constant7'
   *   '<S24>/Gain'
   *   '<S24>/Gain2'
   *   '<S25>/Constant12'
   *   '<S25>/Constant4'
   *   '<S25>/Constant7'
   *   '<S25>/Gain'
   *   '<S25>/Gain2'
   *   '<S26>/Constant4'
   *   '<S26>/Constant7'
   */
  real_T DynamicSteering_StrgRng; /* Mask Parameter: DynamicSteering_StrgRng
                                   * Referenced by:
                                   *   '<S51>/Integrator, Second-Order'
                                   *   '<S53>/Integrator, Second-Order1'
                                   */
  real_T DynamicSteering_TieRodLngth;
  /* Mask Parameter: DynamicSteering_TieRodLngth
   * Referenced by:
   *   '<S24>/Constant10'
   *   '<S24>/Constant5'
   *   '<S25>/Constant10'
   *   '<S25>/Constant5'
   *   '<S26>/Constant5'
   */
  real_T DynamicSteering_TrckWdth;                          /* Mask Parameter: DynamicSteering_TrckWdth
                                                             * Referenced by:
                                                             *   '<S24>/Constant'
                                                             *   '<S25>/Constant'
                                                             *   '<S26>/Constant'
                                                             */
  real_T DynamicSteering_TrqBpts[tx_car::kMap1dSize]; /* Mask Parameter: DynamicSteering_TrqBpts
                                                             * Referenced by: '<S12>/TrqAssistTbl'
                                                             */
  real_T DynamicSteering_TrqLmt;                            /* Mask Parameter: DynamicSteering_TrqLmt
                                                             * Referenced by: '<S12>/Saturation1'
                                                             */
  real_T DynamicSteering_TrqTbl[tx_car::kMap2dSize];  /* Mask Parameter: DynamicSteering_TrqTbl
                                                             * Referenced by: '<S12>/TrqAssistTbl'
                                                             */
  real_T MappedSteer_VehSpdBpts[tx_car::kMap1dSize];  /* Mask Parameter: MappedSteer_VehSpdBpts
                                                             * Referenced by: '<S78>/SpeedCoeff'
                                                             */
  real_T DynamicSteering_VehSpdBpts[tx_car::kMap1dSize];
  /* Mask Parameter: DynamicSteering_VehSpdBpts
   * Referenced by: '<S12>/TrqAssistTbl'
   */
  real_T MappedSteer_WhlLftTbl[tx_car::kMap1dSize];  /* Mask Parameter: MappedSteer_WhlLftTbl
                                                            * Referenced by: '<S78>/leff_whl_ang'
                                                            */
  real_T MappedSteer_WhlRghtTbl[tx_car::kMap1dSize]; /* Mask Parameter: MappedSteer_WhlRghtTbl
                                                            * Referenced by: '<S78>/right_whl_ang'
                                                            */
  real_T DynamicSteering_b1;                               /* Mask Parameter: DynamicSteering_b1
                                                            * Referenced by: '<S54>/b'
                                                            */
  real_T DynamicSteering_b2;                               /* Mask Parameter: DynamicSteering_b2
                                                            * Referenced by: '<S15>/Gain2'
                                                            */
  real_T DynamicSteering_b3;                               /* Mask Parameter: DynamicSteering_b3
                                                            * Referenced by: '<S15>/Gain1'
                                                            */
  real_T DynamicSteering_beta_l;                           /* Mask Parameter: DynamicSteering_beta_l
                                                            * Referenced by: '<S54>/Constant'
                                                            */
  real_T DynamicSteering_beta_u;                           /* Mask Parameter: DynamicSteering_beta_u
                                                            * Referenced by: '<S54>/Constant1'
                                                            */
  real_T CompareToConstant_const;                          /* Mask Parameter: CompareToConstant_const
                                                            * Referenced by: '<S66>/Constant'
                                                            */
  real_T DynamicSteering_k1;                               /* Mask Parameter: DynamicSteering_k1
                                                            * Referenced by: '<S54>/k'
                                                            */
  real_T DynamicSteering_omega_c;                          /* Mask Parameter: DynamicSteering_omega_c
                                                            * Referenced by: '<S17>/Constant'
                                                            */
  real_T DynamicSteering_omega_o;                          /* Mask Parameter: DynamicSteering_omega_o
                                                            * Referenced by:
                                                            *   '<S51>/Constant2'
                                                            *   '<S53>/Constant5'
                                                            */
  real_T DynamicSteering_theta_o;                          /* Mask Parameter: DynamicSteering_theta_o
                                                            * Referenced by:
                                                            *   '<S51>/Constant1'
                                                            *   '<S53>/Constant4'
                                                            */
  real_T div0protectpoly_thresh;                           /* Mask Parameter: div0protectpoly_thresh
                                                            * Referenced by:
                                                            *   '<S19>/Constant'
                                                            *   '<S20>/Constant'
                                                            */
  real_T div0protectpoly_thresh_j;                         /* Mask Parameter: div0protectpoly_thresh_j
                                                            * Referenced by:
                                                            *   '<S28>/Constant'
                                                            *   '<S29>/Constant'
                                                            */
  real_T div0protectpoly_thresh_k;                         /* Mask Parameter: div0protectpoly_thresh_k
                                                            * Referenced by:
                                                            *   '<S31>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S33>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S33>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S33>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S33>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S33>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S33>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S33>/Constant'
                                                            *   '<S32>/Constant'
                                                            *   '<S33>/Constant'
                                                            */
  real_T div0protectabspoly_thresh;                        /* Mask Parameter: div0protectabspoly_thresh
                                                            * Referenced by:
                                                            *   '<S59>/Constant'
                                                            *   '<S60>/Constant'
                                                            */
  real_T div0protectpoly_thresh_i;                         /* Mask Parameter: div0protectpoly_thresh_i
                                                            * Referenced by:
                                                            *   '<S61>/Constant'
                                                            *   '<S62>/Constant'
                                                            */
  real_T div0protectabspoly_thresh_o;
  /* Mask Parameter: div0protectabspoly_thresh_o
   * Referenced by:
   *   '<S64>/Constant'
   *   '<S65>/Constant'
   */
  real_T div0protectabspoly1_thresh;
  /* Mask Parameter: div0protectabspoly1_thresh
   * Referenced by:
   *   '<S68>/Constant'
   *   '<S69>/Constant'
   */
  real_T TransferFcn_A[2];           /* Computed Parameter: TransferFcn_A
                                      * Referenced by: '<S76>/Transfer Fcn'
                                      */
  real_T TransferFcn_C[2];           /* Computed Parameter: TransferFcn_C
                                      * Referenced by: '<S76>/Transfer Fcn'
                                      */
  real_T secondOrderSwitch_Value;    /* Expression: 1
                                      * Referenced by: '<S4>/secondOrderSwitch'
                                      */
  real_T Switch_Threshold;           /* Expression: 0.5
                                      * Referenced by: '<S4>/Switch'
                                      */
  real_T Constant1_Value;            /* Expression: 0
                                      * Referenced by: '<S79>/Constant1'
                                      */
  real_T Constant_Value;             /* Expression: 0
                                      * Referenced by: '<S4>/Constant'
                                      */
  real_T Switch1_Threshold;          /* Expression: 0
                                      * Referenced by: '<S18>/Switch1'
                                      */
  real_T Switch1_Threshold_k;        /* Expression: 0
                                      * Referenced by: '<S27>/Switch1'
                                      */
  real_T Switch1_Threshold_p;        /* Expression: 0
                                      * Referenced by: '<S30>/Switch1'
                                      */
  real_T Switch1_Threshold_f;        /* Expression: 0
                                      * Referenced by: '<S58>/Switch1'
                                      */
  real_T Constant1_Value_i;          /* Expression: 0
                                      * Referenced by: '<S2>/Constant1'
                                      */
  real_T Integrator1_IC;             /* Expression: 0
                                      * Referenced by: '<S17>/Integrator1'
                                      */
  real_T Constant8_Value;            /* Expression: pi/2
                                      * Referenced by: '<S26>/Constant8'
                                      */
  real_T Constant6_Value;            /* Expression: 2
                                      * Referenced by: '<S26>/Constant6'
                                      */
  real_T Constant8_Value_n;          /* Expression: pi/2
                                      * Referenced by: '<S25>/Constant8'
                                      */
  real_T DeadZone_Start;             /* Expression: -0.001
                                      * Referenced by: '<S13>/Dead Zone'
                                      */
  real_T DeadZone_End;               /* Expression: 0.001
                                      * Referenced by: '<S13>/Dead Zone'
                                      */
  real_T Constant6_Value_d;          /* Expression: 2
                                      * Referenced by: '<S25>/Constant6'
                                      */
  real_T index_Value;                /* Expression: 1
                                      * Referenced by: '<S10>/index'
                                      */
  real_T Switch_Threshold_k;         /* Expression: 0
                                      * Referenced by: '<S10>/Switch'
                                      */
  real_T Constant8_Value_c;          /* Expression: pi/2
                                      * Referenced by: '<S24>/Constant8'
                                      */
  real_T Gain_Gain;                  /* Expression: -1
                                      * Referenced by: '<S21>/Gain'
                                      */
  real_T Constant6_Value_o;          /* Expression: 2
                                      * Referenced by: '<S24>/Constant6'
                                      */
  real_T Switch1_Threshold_g;        /* Expression: 0
                                      * Referenced by: '<S10>/Switch1'
                                      */
  real_T Constant_Value_e;           /* Expression: 1
                                      * Referenced by: '<S18>/Constant'
                                      */
  real_T Constant11_Value;           /* Expression: pi/2
                                      * Referenced by: '<S24>/Constant11'
                                      */
  real_T Constant_Value_l;           /* Expression: 1
                                      * Referenced by: '<S27>/Constant'
                                      */
  real_T index_Value_c;              /* Expression: 1
                                      * Referenced by: '<S16>/index'
                                      */
  real_T Switch1_Threshold_c;        /* Expression: 0
                                      * Referenced by: '<S16>/Switch1'
                                      */
  real_T Constant11_Value_a;         /* Expression: pi/2
                                      * Referenced by: '<S25>/Constant11'
                                      */
  real_T Constant_Value_j;           /* Expression: 1
                                      * Referenced by: '<S30>/Constant'
                                      */
  real_T Switch_Threshold_e;         /* Expression: 0
                                      * Referenced by: '<S16>/Switch'
                                      */
  real_T Gain1_Gain;                 /* Expression: -1
                                      * Referenced by: '<S21>/Gain1'
                                      */
  real_T DeadZone2_Start;            /* Expression: -0.001
                                      * Referenced by: '<S13>/Dead Zone2'
                                      */
  real_T DeadZone2_End;              /* Expression: 0.001
                                      * Referenced by: '<S13>/Dead Zone2'
                                      */
  real_T Memory_InitialCondition;    /* Expression: 0
                                      * Referenced by: '<S56>/Memory'
                                      */
  real_T domega_o1_Value;            /* Expression: 0
                                      * Referenced by: '<S54>/domega_o1'
                                      */
  real_T Constant_Value_o;           /* Expression: 1
                                      * Referenced by: '<S58>/Constant'
                                      */
  real_T Switch_Threshold_a;         /* Expression: 0
                                      * Referenced by: '<S54>/Switch'
                                      */
  real_T Memory_InitialCondition_g;  /* Expression: 0
                                      * Referenced by: '<S55>/Memory'
                                      */
  real_T domega_o_Value;             /* Expression: 0
                                      * Referenced by: '<S54>/domega_o'
                                      */
  real_T omega_c_Value;              /* Expression: 10
                                      * Referenced by: '<S54>/omega_c'
                                      */
  real_T Reset_Value;                /* Expression: 1
                                      * Referenced by: '<S55>/Reset'
                                      */
  real_T omega_c1_Value;             /* Expression: 10
                                      * Referenced by: '<S54>/omega_c1'
                                      */
  real_T Reset_Value_b;              /* Expression: 1
                                      * Referenced by: '<S56>/Reset'
                                      */
  real_T Memory_1_InitialCondition;  /* Expression: 0
                                      * Referenced by: '<S1>/Memory'
                                      */
  real_T Memory_2_InitialCondition;  /* Expression: 0
                                      * Referenced by: '<S1>/Memory'
                                      */
  real_T Memory_3_InitialCondition;  /* Expression: 0
                                      * Referenced by: '<S1>/Memory'
                                      */
  uint32_T TrqAssistTbl_maxIndex[2]; /* Computed Parameter: TrqAssistTbl_maxIndex
                                      * Referenced by: '<S12>/TrqAssistTbl'
                                      */
};

/* Real-time Model Data Structure */
struct tag_RTM_DynamicSteer_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_DynamicSteer_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_DynamicSteer_T *contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[9];
  real_T odeF[4][9];
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
    boolean_T firstInitCondFlag;
    SimTimeStep simTimeStep;
    boolean_T stopRequestedFlag;
    time_T *t;
    time_T tArray[2];
  } Timing;
};

/* Class declaration for model DynamicSteer */
class MODULE_API DynamicSteer {
  /* public data and function members */
 public:
  /* Copy Constructor */
  DynamicSteer(DynamicSteer const &) = delete;

  /* Assignment Operator */
  DynamicSteer &operator=(DynamicSteer const &) & = delete;

  /* Move Constructor */
  DynamicSteer(DynamicSteer &&) = delete;

  /* Move Assignment Operator */
  DynamicSteer &operator=(DynamicSteer &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_DynamicSteer_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_DynamicSteer_T *pExtU_DynamicSteer_T) { DynamicSteer_U = *pExtU_DynamicSteer_T; }

  /* Root outports get method */
  const ExtY_DynamicSteer_T &getExternalOutputs() const { return DynamicSteer_Y; }

  void ModelPrevZCStateInit();

  /* model start function */
  void start();

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  static void terminate();

  /* Constructor */
  DynamicSteer();

  /* Destructor */
  ~DynamicSteer();

 protected:
  /* Tunable parameters */
  static P_DynamicSteer_T DynamicSteer_P;

  // mapped steer parameter
  uint32_t m_steer_bps_maxIndex = 52;
  uint32_t m_speed_coef_maxIndex = 1;

  // dynamic steer parameter
  uint32_t m_pinion_radius_maxIndex = 10;
  uint32_t m_power_assist_trqIn_maxIndex = 2;
  uint32_t m_power_assist_speed_maxIndex = 1;
  uint32_t m_power_assist_assisTrq_maxIndex = 5;

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_DynamicSteer_T DynamicSteer_U;

  /* External outputs */
  ExtY_DynamicSteer_T DynamicSteer_Y;

  /* Block signals */
  B_DynamicSteer_T DynamicSteer_B;

  /* Block states */
  DW_DynamicSteer_T DynamicSteer_DW;

  /* Tunable parameters */
  // static P_DynamicSteer_T DynamicSteer_P;

  /* Block continuous states */
  X_DynamicSteer_T DynamicSteer_X;

  /* Triggered events */
  PrevZCX_DynamicSteer_T DynamicSteer_PrevZCX;

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void DynamicSteer_derivatives();

  /* Real-Time Model */
  RT_MODEL_DynamicSteer_T DynamicSteer_M;
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
 * '<Root>' : 'DynamicSteer'
 * '<S1>'   : 'DynamicSteer/Steer'
 * '<S2>'   : 'DynamicSteer/Steer/DynamicSteer'
 * '<S3>'   : 'DynamicSteer/Steer/In'
 * '<S4>'   : 'DynamicSteer/Steer/MappedSteer'
 * '<S5>'   : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering'
 * '<S6>'   : 'DynamicSteer/Steer/DynamicSteer/Subsystem'
 * '<S7>'   : 'DynamicSteer/Steer/DynamicSteer/TireFeedback'
 * '<S8>'   : 'DynamicSteer/Steer/DynamicSteer/WheelAngle'
 * '<S9>'   : 'DynamicSteer/Steer/DynamicSteer/caster'
 * '<S10>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/AngSwitch'
 * '<S11>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/CalPwrLoss'
 * '<S12>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/PowerAssist'
 * '<S13>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio'
 * '<S14>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/SpdSwitch'
 * '<S15>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia'
 * '<S16>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqSwitch'
 * '<S17>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/PowerAssist/Cont LPF1'
 * '<S18>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/PowerAssist/div0protect - poly'
 * '<S19>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/PowerAssist/div0protect - poly/Compare To Constant'
 * '<S20>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/PowerAssist/div0protect - poly/Compare To Constant1'
 * '<S21>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion'
 * '<S22>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1'
 * '<S23>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/div0protect - poly'
 * '<S24>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm'
 * '<S25>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm1'
 * '<S26>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm2'
 * '<S27>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm/div0protect - poly'
 * '<S28>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm/div0protect - poly/Compare To Constant'
 * '<S29>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm/div0protect - poly/Compare To Constant1'
 * '<S30>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm1/div0protect - poly'
 * '<S31>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm1/div0protect - poly/Compare To Constant'
 * '<S32>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm1/div0protect - poly/Compare To Constant1'
 * '<S33>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm2/div0protect - poly'
 * '<S34>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm2/div0protect - poly/Compare To Constant'
 * '<S35>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion/Rack and pinion
 * algorithm2/div0protect - poly/Compare To Constant1'
 * '<S36>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm'
 * '<S37>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm1'
 * '<S38>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm2'
 * '<S39>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm/div0protect - poly'
 * '<S40>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm/div0protect - poly/Compare To Constant'
 * '<S41>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm/div0protect - poly/Compare To Constant1'
 * '<S42>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm1/div0protect - poly'
 * '<S43>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm1/div0protect - poly/Compare To Constant'
 * '<S44>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm1/div0protect - poly/Compare To Constant1'
 * '<S45>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm2/div0protect - poly'
 * '<S46>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm2/div0protect - poly/Compare To Constant'
 * '<S47>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/Rack and Pinion1/Rack and pinion
 * algorithm2/div0protect - poly/Compare To Constant1'
 * '<S48>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/div0protect - poly/Compare To
 * Constant'
 * '<S49>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/RackandPinVariableRatio/div0protect - poly/Compare To
 * Constant1'
 * '<S50>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Backlash1'
 * '<S51>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Column'
 * '<S52>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Friction'
 * '<S53>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Shaft'
 * '<S54>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Backlash1/Subsystem'
 * '<S55>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Backlash1/Subsystem/Cont LPF IC Dyn'
 * '<S56>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Backlash1/Subsystem/Cont LPF IC Dyn1'
 * '<S57>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Backlash1/Subsystem/div0protect - abs poly'
 * '<S58>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Backlash1/Subsystem/div0protect - poly'
 * '<S59>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Backlash1/Subsystem/div0protect - abs
 * poly/Compare To Constant'
 * '<S60>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Backlash1/Subsystem/div0protect - abs
 * poly/Compare To Constant1'
 * '<S61>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Backlash1/Subsystem/div0protect -
 * poly/Compare To Constant'
 * '<S62>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Backlash1/Subsystem/div0protect -
 * poly/Compare To Constant1'
 * '<S63>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Column/div0protect - abs poly'
 * '<S64>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Column/div0protect - abs poly/Compare To
 * Constant'
 * '<S65>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Column/div0protect - abs poly/Compare To
 * Constant1'
 * '<S66>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Friction/Compare To Constant'
 * '<S67>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Shaft/div0protect - abs poly1'
 * '<S68>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Shaft/div0protect - abs poly1/Compare To
 * Constant'
 * '<S69>'  : 'DynamicSteer/Steer/DynamicSteer/Dynamic Steering/TrqInertia/Shaft/div0protect - abs poly1/Compare To
 * Constant1'
 * '<S70>'  : 'DynamicSteer/Steer/DynamicSteer/TireFeedback/selector'
 * '<S71>'  : 'DynamicSteer/Steer/DynamicSteer/TireFeedback/selector1'
 * '<S72>'  : 'DynamicSteer/Steer/DynamicSteer/TireFeedback/selector2'
 * '<S73>'  : 'DynamicSteer/Steer/DynamicSteer/TireFeedback/selector3'
 * '<S74>'  : 'DynamicSteer/Steer/DynamicSteer/TireFeedback/selector4'
 * '<S75>'  : 'DynamicSteer/Steer/DynamicSteer/TireFeedback/selector5'
 * '<S76>'  : 'DynamicSteer/Steer/MappedSteer/2ndOrderSystem'
 * '<S77>'  : 'DynamicSteer/Steer/MappedSteer/MappedSteer'
 * '<S78>'  : 'DynamicSteer/Steer/MappedSteer/MappedSteer/MappedSteer'
 * '<S79>'  : 'DynamicSteer/Steer/MappedSteer/MappedSteer/route'
 */
#endif /* RTW_HEADER_DynamicSteer_h_ */
