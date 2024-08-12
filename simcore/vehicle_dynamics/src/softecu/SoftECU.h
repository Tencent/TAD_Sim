/*
 * SoftECU.h
 *
 * Code generation for model "SoftECU".
 *
 * Model version              : 2.90
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul  5 17:08:36 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_SoftECU_h_
#define RTW_HEADER_SoftECU_h_
#include <cstring>
#include "SoftECU_types.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rtwtypes.h"

#include "inc/car_common.h"

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

/* Block signals for system '<S1>/ax_estimator' */
struct B_ax_estimator_SoftECU_T {
  real_T Integrator1; /* '<S195>/Integrator1' */
  real_T Divide;      /* '<S195>/Divide' */
  real_T Delay;       /* '<S196>/Delay' */
};

/* Block states (default storage) for system '<S1>/ax_estimator' */
struct DW_ax_estimator_SoftECU_T {
  real_T Delay_DSTATE[100];  /* '<S196>/Delay' */
  int8_T If_ActiveSubsystem; /* '<S6>/If' */
};

/* Continuous states for system '<S1>/ax_estimator' */
struct X_ax_estimator_SoftECU_T {
  real_T Integrator1_CSTATE; /* '<S195>/Integrator1' */
};

/* State derivatives for system '<S1>/ax_estimator' */
struct XDot_ax_estimator_SoftECU_T {
  real_T Integrator1_CSTATE; /* '<S195>/Integrator1' */
};

/* State Disabled for system '<S1>/ax_estimator' */
struct XDis_ax_estimator_SoftECU_T {
  boolean_T Integrator1_CSTATE; /* '<S195>/Integrator1' */
};

/* Block signals (default storage) */
struct B_SoftECU_T {
  real_T target_acc_m_s2;
  /* '<Root>/BusConversion_InsertedFor_ecu_in_at_outport_0' */
  real_T veh_speed_vx;
  /* '<Root>/BusConversion_InsertedFor_ecu_in_at_outport_0' */
  real_T brake_pressure_0_1; /* '<S1>/Switch1' */
  real_T slip_ratio[4];
  /* '<Root>/BusConversion_InsertedFor_ecu_in_at_outport_0' */
  real_T Filter;              /* '<S171>/Filter' */
  real_T IntegralGain;        /* '<S173>/Integral Gain' */
  real_T Integrator;          /* '<S176>/Integrator' */
  real_T FilterCoefficient;   /* '<S179>/Filter Coefficient' */
  real_T Filter_p;            /* '<S108>/Filter' */
  real_T IntegralGain_o;      /* '<S110>/Integral Gain' */
  real_T Integrator_m;        /* '<S113>/Integrator' */
  real_T FilterCoefficient_o; /* '<S116>/Filter Coefficient' */
  real_T Memory[4];           /* '<S2>/Memory' */
  real_T Abs;                 /* '<S2>/Abs' */
  real_T OnAtRef_OffAt05Ref;  /* '<S2>/OnAtRef_OffAt0.5Ref' */
  real_T ABS_flag[4];         /* '<S2>/ABS_flag' */
  real_T Product1[4];         /* '<S2>/Product1' */
  real_T Filter_po;           /* '<S38>/Filter' */
  real_T IntegralGain_n;      /* '<S40>/Integral Gain' */
  real_T Integrator_k;        /* '<S43>/Integrator' */
  real_T FilterCoefficient_i; /* '<S46>/Filter Coefficient' */
  real_T pt_trq_cmd_Nm;       /* '<S1>/Merge' */
  real_T request_torque;
  /* '<Root>/BusConversion_InsertedFor_ecu_in_at_outport_0' */
  real_T batt_soc_0_1;
  /* '<Root>/BusConversion_InsertedFor_ecu_in_at_outport_0' */
  real_T throttle_01; /* '<S1>/Merge1' */
  uint8_T brake_control_mode;
  /* '<Root>/BusConversion_InsertedFor_ecu_in_at_outport_0' */
  uint8_T gear_cmd__0N1D2R3P;
  /* '<Root>/BusConversion_InsertedFor_ecu_in_at_outport_0' */
  uint8_T acc_control_mode;
  /* '<Root>/BusConversion_InsertedFor_ecu_in_at_outport_0' */
  boolean_T LogicalOperator;             /* '<S7>/Logical Operator' */
  boolean_T switchOffMotor;              /* '<S141>/switchOffMotor' */
  boolean_T LogicalOperator1;            /* '<S74>/Logical Operator1' */
  boolean_T Merge1;                      /* '<S2>/Merge1' */
  boolean_T abs_on;                      /* '<S2>/if_on' */
  boolean_T LogicalOperator_i;           /* '<S8>/Logical Operator' */
  B_ax_estimator_SoftECU_T ax_estimator; /* '<S1>/ax_estimator' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_SoftECU_T {
  real_T Filter_DSTATE;                      /* '<S171>/Filter' */
  real_T Integrator_DSTATE;                  /* '<S176>/Integrator' */
  real_T Filter_DSTATE_i;                    /* '<S108>/Filter' */
  real_T Integrator_DSTATE_c;                /* '<S113>/Integrator' */
  real_T Filter_DSTATE_h;                    /* '<S38>/Filter' */
  real_T Integrator_DSTATE_f;                /* '<S43>/Integrator' */
  real_T Memory_PreviousInput[4];            /* '<S2>/Memory' */
  int8_T If_ActiveSubsystem;                 /* '<S1>/If' */
  int8_T If_ActiveSubsystem_c;               /* '<S5>/If' */
  int8_T If1_ActiveSubsystem;                /* '<S5>/If1' */
  int8_T Filter_PrevResetState;              /* '<S171>/Filter' */
  int8_T Integrator_PrevResetState;          /* '<S176>/Integrator' */
  int8_T If_ActiveSubsystem_g;               /* '<S4>/If' */
  int8_T Filter_PrevResetState_p;            /* '<S108>/Filter' */
  int8_T Integrator_PrevResetState_m;        /* '<S113>/Integrator' */
  int8_T If_ActiveSubsystem_l;               /* '<S2>/If' */
  int8_T Filter_PrevResetState_c;            /* '<S38>/Filter' */
  int8_T Integrator_PrevResetState_o;        /* '<S43>/Integrator' */
  boolean_T RelayCutOffAt6_EnableAt5_Mode;   /* '<S7>/RelayCutOffAt6_EnableAt5' */
  boolean_T RelayCutOffAt58_EnableAt55_Mode; /* '<S7>/RelayCutOffAt58_EnableAt55' */
  boolean_T OnAtRef_OffAt05Ref_Mode;         /* '<S2>/OnAtRef_OffAt0.5Ref' */
  DW_ax_estimator_SoftECU_T ax_estimator;    /* '<S1>/ax_estimator' */
};

/* Continuous states (default storage) */
struct X_SoftECU_T {
  X_ax_estimator_SoftECU_T ax_estimator; /* '<S1>/ax_estimator' */
};

/* State derivatives (default storage) */
struct XDot_SoftECU_T {
  XDot_ax_estimator_SoftECU_T ax_estimator; /* '<S1>/ax_estimator' */
};

/* State disabled  */
struct XDis_SoftECU_T {
  XDis_ax_estimator_SoftECU_T ax_estimator; /* '<S1>/ax_estimator' */
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
struct ExtU_SoftECU_T {
  powecu_in ecu_in; /* '<Root>/ecu_in' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_SoftECU_T {
  powecu_out ecu_out; /* '<Root>/ecu_out' */
};

/* Parameters for system: '<S1>/ax_estimator' */
struct P_ax_estimator_SoftECU_T_ {
  real_T SpeedThresh_const;      /* Mask Parameter: SpeedThresh_const
                                  * Referenced by: '<S198>/Constant'
                                  */
  real_T AxAct_wc;               /* Mask Parameter: AxAct_wc
                                  * Referenced by: '<S195>/Constant'
                                  */
  real_T Delay_InitialCondition; /* Expression: 0.0
                                  * Referenced by: '<S196>/Delay'
                                  */
  real_T Integrator1_IC;         /* Expression: 0
                                  * Referenced by: '<S195>/Integrator1'
                                  */
};

/* Parameters (default storage) */
struct P_SoftECU_T_ {
  real_T DriveLinePowType;            /* Variable: DriveLinePowType
                                       * Referenced by:
                                       *   '<S1>/Constant1'
                                       *   '<S3>/Constant2'
                                       */
  real_T EngMaxTrq;                   /* Variable: EngMaxTrq
                                       * Referenced by:
                                       *   '<S4>/Gain'
                                       *   '<S4>/Saturation1'
                                       *   '<S75>/Constant'
                                       */
  real_T MotorMaxTrq;                 /* Variable: MotorMaxTrq
                                       * Referenced by:
                                       *   '<S5>/Constant'
                                       *   '<S5>/Saturation1'
                                       *   '<S139>/Constant1'
                                       *   '<S141>/Constant1'
                                       */
  real_T MotorPowerRegenerationRatio; /* Variable: MotorPowerRegenerationRatio
                                       * Referenced by: '<S139>/Constant4'
                                       */
  real_T abs_brake_pressure;          /* Variable: abs_brake_pressure
                                       * Referenced by:
                                       *   '<S8>/abs_brake_pressure_01'
                                       *   '<S9>/abs_brake_pressure_01'
                                       */
  real_T brake_D;                     /* Variable: brake_D
                                       * Referenced by: '<S37>/Derivative Gain'
                                       */
  real_T brake_I;                     /* Variable: brake_I
                                       * Referenced by: '<S40>/Integral Gain'
                                       */
  real_T brake_P;                     /* Variable: brake_P
                                       * Referenced by: '<S48>/Proportional Gain'
                                       */
  real_T ecu_max_acc;                 /* Variable: ecu_max_acc
                                       * Referenced by:
                                       *   '<S74>/Constant'
                                       *   '<S141>/Constant'
                                       */
  real_T ecu_max_dec;                 /* Variable: ecu_max_dec
                                       * Referenced by: '<S8>/Constant'
                                       */
  real_T enable_abs;                  /* Variable: enable_abs
                                       * Referenced by: '<S2>/abs_switch'
                                       */
  real_T eng_acc_D;                   /* Variable: eng_acc_D
                                       * Referenced by: '<S107>/Derivative Gain'
                                       */
  real_T eng_acc_I;                   /* Variable: eng_acc_I
                                       * Referenced by: '<S110>/Integral Gain'
                                       */
  real_T eng_acc_P;                   /* Variable: eng_acc_P
                                       * Referenced by: '<S118>/Proportional Gain'
                                       */
  real_T slip_disable_ratio_ref;      /* Variable: slip_disable_ratio_ref
                                       * Referenced by: '<S2>/OnAtRef_OffAt0.5Ref'
                                       */
  real_T slip_ratio_ref;              /* Variable: slip_ratio_ref
                                       * Referenced by: '<S2>/OnAtRef_OffAt0.5Ref'
                                       */
  real_T softecu_ax_delay;            /* Variable: softecu_ax_delay
                                       * Referenced by: '<S196>/AxDelayStep'
                                       */
  real_T speedOffset;                 /* Variable: speedOffset
                                       * Referenced by:
                                       *   '<S69>/Constant'
                                       *   '<S64>/Constant'
                                       *   '<S65>/Constant'
                                       */
  real_T stepTime;                    /* Variable: stepTime
                                       * Referenced by: '<S196>/Gain'
                                       */
  real_T vcu_acc_D;                   /* Variable: vcu_acc_D
                                       * Referenced by: '<S170>/Derivative Gain'
                                       */
  real_T vcu_acc_I;                   /* Variable: vcu_acc_I
                                       * Referenced by: '<S173>/Integral Gain'
                                       */
  real_T vcu_acc_P;                   /* Variable: vcu_acc_P
                                       * Referenced by: '<S181>/Proportional Gain'
                                       */
  real_T DiscretePIDController_InitialConditionForFilter;
  /* Mask Parameter: DiscretePIDController_InitialConditionForFilter
   * Referenced by: '<S38>/Filter'
   */
  real_T DiscretePIDController_InitialConditionForFilter_e;
  /* Mask Parameter: DiscretePIDController_InitialConditionForFilter_e
   * Referenced by: '<S108>/Filter'
   */
  real_T DiscretePIDController_InitialConditionForFilter_b;
  /* Mask Parameter: DiscretePIDController_InitialConditionForFilter_b
   * Referenced by: '<S171>/Filter'
   */
  real_T DiscretePIDController_InitialConditionForIntegrator;
  /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator
   * Referenced by: '<S43>/Integrator'
   */
  real_T DiscretePIDController_InitialConditionForIntegrator_k;
  /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator_k
   * Referenced by: '<S113>/Integrator'
   */
  real_T DiscretePIDController_InitialConditionForIntegrator_h;
  /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator_h
   * Referenced by: '<S176>/Integrator'
   */
  real_T DiscretePIDController_LowerSaturationLimit;
  /* Mask Parameter: DiscretePIDController_LowerSaturationLimit
   * Referenced by: '<S50>/Saturation'
   */
  real_T DiscretePIDController_LowerSaturationLimit_o;
  /* Mask Parameter: DiscretePIDController_LowerSaturationLimit_o
   * Referenced by: '<S120>/Saturation'
   */
  real_T DiscretePIDController_LowerSaturationLimit_k;
  /* Mask Parameter: DiscretePIDController_LowerSaturationLimit_k
   * Referenced by: '<S183>/Saturation'
   */
  real_T DiscretePIDController_N;   /* Mask Parameter: DiscretePIDController_N
                                     * Referenced by: '<S46>/Filter Coefficient'
                                     */
  real_T DiscretePIDController_N_i; /* Mask Parameter: DiscretePIDController_N_i
                                     * Referenced by: '<S116>/Filter Coefficient'
                                     */
  real_T DiscretePIDController_N_k; /* Mask Parameter: DiscretePIDController_N_k
                                     * Referenced by: '<S179>/Filter Coefficient'
                                     */
  real_T DiscretePIDController_UpperSaturationLimit;
  /* Mask Parameter: DiscretePIDController_UpperSaturationLimit
   * Referenced by: '<S50>/Saturation'
   */
  real_T DiscretePIDController_UpperSaturationLimit_m;
  /* Mask Parameter: DiscretePIDController_UpperSaturationLimit_m
   * Referenced by: '<S120>/Saturation'
   */
  real_T DiscretePIDController_UpperSaturationLimit_f;
  /* Mask Parameter: DiscretePIDController_UpperSaturationLimit_f
   * Referenced by: '<S183>/Saturation'
   */
  real_T CompareToConstant_const; /* Mask Parameter: CompareToConstant_const
                                   * Referenced by: '<S66>/Constant'
                                   */
  real_T brakeEnableThresh_Reverse_const;
  /* Mask Parameter: brakeEnableThresh_Reverse_const
   * Referenced by: '<S61>/Constant'
   */
  real_T brakeEnableThresh_const;   /* Mask Parameter: brakeEnableThresh_const
                                     * Referenced by: '<S60>/Constant'
                                     */
  real_T CompareToConstant1_const;  /* Mask Parameter: CompareToConstant1_const
                                     * Referenced by: '<S80>/Constant'
                                     */
  real_T CompareToConstant_const_f; /* Mask Parameter: CompareToConstant_const_f
                                     * Referenced by: '<S76>/Constant'
                                     */
  real_T CompareToConstant1_const_j;
  /* Mask Parameter: CompareToConstant1_const_j
   * Referenced by: '<S77>/Constant'
   */
  real_T CompareToConstant_const_b; /* Mask Parameter: CompareToConstant_const_b
                                     * Referenced by: '<S71>/Constant'
                                     */
  real_T CompareToConstant_const_c; /* Mask Parameter: CompareToConstant_const_c
                                     * Referenced by: '<S142>/Constant'
                                     */
  real_T speedLow_xms_const;        /* Mask Parameter: speedLow_xms_const
                                     * Referenced by: '<S144>/Constant'
                                     */
  real_T CompareToConstant_const_e; /* Mask Parameter: CompareToConstant_const_e
                                     * Referenced by: '<S138>/Constant'
                                     */
  real_T CompareToConstant_const_k; /* Mask Parameter: CompareToConstant_const_k
                                     * Referenced by: '<S135>/Constant'
                                     */
  real_T CompareToConstant1_const_c;
  /* Mask Parameter: CompareToConstant1_const_c
   * Referenced by: '<S136>/Constant'
   */
  uint8_T reverseGear_const;  /* Mask Parameter: reverseGear_const
                               * Referenced by: '<S63>/Constant'
                               */
  uint8_T notReverse_const;   /* Mask Parameter: notReverse_const
                               * Referenced by: '<S62>/Constant'
                               */
  uint8_T reverseGear1_const; /* Mask Parameter: reverseGear1_const
                               * Referenced by: '<S79>/Constant'
                               */
  uint8_T forwardGear_const;  /* Mask Parameter: forwardGear_const
                               * Referenced by: '<S78>/Constant'
                               */
  uint8_T CompareToConstant1_const_p;
  /* Mask Parameter: CompareToConstant1_const_p
   * Referenced by: '<S193>/Constant'
   */
  uint8_T CompareToConstant2_const;         /* Mask Parameter: CompareToConstant2_const
                                             * Referenced by: '<S194>/Constant'
                                             */
  uint8_T CompareToConstant_const_l;        /* Mask Parameter: CompareToConstant_const_l
                                             * Referenced by: '<S137>/Constant'
                                             */
  real_T Switch_Threshold;                  /* Expression: 0
                                             * Referenced by: '<S9>/Switch'
                                             */
  real_T Filter_gainval;                    /* Computed Parameter: Filter_gainval
                                             * Referenced by: '<S38>/Filter'
                                             */
  real_T Integrator_gainval;                /* Computed Parameter: Integrator_gainval
                                             * Referenced by: '<S43>/Integrator'
                                             */
  real_T abs_Threshold;                     /* Expression: 0.5
                                             * Referenced by: '<S8>/abs'
                                             */
  real_T OnAtRef_OffAt05Ref_YOn;            /* Expression: 1
                                             * Referenced by: '<S2>/OnAtRef_OffAt0.5Ref'
                                             */
  real_T OnAtRef_OffAt05Ref_YOff;           /* Expression: 0
                                             * Referenced by: '<S2>/OnAtRef_OffAt0.5Ref'
                                             */
  real_T Constant1_Value[4];                /* Expression: [1 1 1 1]
                                             * Referenced by: '<S2>/Constant1'
                                             */
  real_T Memory_InitialCondition;           /* Expression: 0
                                             * Referenced by: '<S2>/Memory'
                                             */
  real_T Saturation_UpperSat;               /* Expression: 1
                                             * Referenced by: '<S2>/Saturation'
                                             */
  real_T Saturation_LowerSat;               /* Expression: 0
                                             * Referenced by: '<S2>/Saturation'
                                             */
  real_T Constant2_Value;                   /* Expression: 0
                                             * Referenced by: '<S74>/Constant2'
                                             */
  real_T Filter_gainval_p;                  /* Computed Parameter: Filter_gainval_p
                                             * Referenced by: '<S108>/Filter'
                                             */
  real_T Integrator_gainval_k;              /* Computed Parameter: Integrator_gainval_k
                                             * Referenced by: '<S113>/Integrator'
                                             */
  real_T Saturation_UpperSat_l;             /* Expression: 1
                                             * Referenced by: '<S73>/Saturation'
                                             */
  real_T Saturation_LowerSat_j;             /* Expression: 0
                                             * Referenced by: '<S73>/Saturation'
                                             */
  real_T fliper_Value;                      /* Expression: -1
                                             * Referenced by: '<S72>/fliper'
                                             */
  real_T no_throttle_Value;                 /* Expression: 0
                                             * Referenced by: '<S4>/no_throttle'
                                             */
  real_T Saturation_UpperSat_lm;            /* Expression: 1
                                             * Referenced by: '<S4>/Saturation'
                                             */
  real_T Saturation_LowerSat_h;             /* Expression: 0
                                             * Referenced by: '<S4>/Saturation'
                                             */
  real_T Constant2_Value_l;                 /* Expression: 0.05
                                             * Referenced by: '<S141>/Constant2'
                                             */
  real_T Filter_gainval_k;                  /* Computed Parameter: Filter_gainval_k
                                             * Referenced by: '<S171>/Filter'
                                             */
  real_T Integrator_gainval_i;              /* Computed Parameter: Integrator_gainval_i
                                             * Referenced by: '<S176>/Integrator'
                                             */
  real_T cutoffMotorOutput_Value;           /* Expression: 0
                                             * Referenced by: '<S141>/cutoffMotorOutput'
                                             */
  real_T Gain_Gain;                         /* Expression: -1
                                             * Referenced by: '<S132>/Gain'
                                             */
  real_T Constant_Value;                    /* Expression: 0
                                             * Referenced by: '<S132>/Constant'
                                             */
  real_T Constant2_Value_b;                 /* Expression: 0.05
                                             * Referenced by: '<S132>/Constant2'
                                             */
  real_T Constant3_Value;                   /* Expression: 0.9
                                             * Referenced by: '<S132>/Constant3'
                                             */
  real_T averageTorqueCoeff_Value;          /* Expression: 0.4
                                             * Referenced by: '<S139>/averageTorqueCoeff'
                                             */
  real_T Saturation_UpperSat_e;             /* Expression: 1
                                             * Referenced by: '<S5>/Saturation'
                                             */
  real_T Saturation_LowerSat_o;             /* Expression: 0
                                             * Referenced by: '<S5>/Saturation'
                                             */
  real_T autoHoldBrakePressure_01_Value;    /* Expression: 0.6
                                             * Referenced by: '<S1>/autoHoldBrakePressure_01'
                                             */
  real_T Switch1_Threshold;                 /* Expression: 0.5
                                             * Referenced by: '<S1>/Switch1'
                                             */
  real_T RelayCutOffAt6_EnableAt5_OnVal;    /* Expression: -5
                                             * Referenced by: '<S7>/RelayCutOffAt6_EnableAt5'
                                             */
  real_T RelayCutOffAt6_EnableAt5_OffVal;   /* Expression: -6
                                             * Referenced by: '<S7>/RelayCutOffAt6_EnableAt5'
                                             */
  real_T RelayCutOffAt6_EnableAt5_YOn;      /* Expression: 0
                                             * Referenced by: '<S7>/RelayCutOffAt6_EnableAt5'
                                             */
  real_T RelayCutOffAt6_EnableAt5_YOff;     /* Expression: 1
                                             * Referenced by: '<S7>/RelayCutOffAt6_EnableAt5'
                                             */
  real_T RelayCutOffAt58_EnableAt55_OnVal;  /* Expression: 58
                                             * Referenced by: '<S7>/RelayCutOffAt58_EnableAt55'
                                             */
  real_T RelayCutOffAt58_EnableAt55_OffVal; /* Expression: 55
                                             * Referenced by: '<S7>/RelayCutOffAt58_EnableAt55'
                                             */
  real_T RelayCutOffAt58_EnableAt55_YOn;    /* Expression: 1
                                             * Referenced by: '<S7>/RelayCutOffAt58_EnableAt55'
                                             */
  real_T RelayCutOffAt58_EnableAt55_YOff;   /* Expression: 0
                                             * Referenced by: '<S7>/RelayCutOffAt58_EnableAt55'
                                             */
  real_T Constant_Value_o;                  /* Expression: 0
                                             * Referenced by: '<S1>/Constant'
                                             */
  P_ax_estimator_SoftECU_T ax_estimator;    /* '<S1>/ax_estimator' */
};

/* Real-time Model Data Structure */
struct tag_RTM_SoftECU_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_SoftECU_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_SoftECU_T *contStateDisabled;
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

/* Class declaration for model SoftECU */
class MODULE_API SoftECU {
  /* public data and function members */
 public:
  /* Copy Constructor */
  SoftECU(SoftECU const &) = delete;

  /* Assignment Operator */
  SoftECU &operator=(SoftECU const &) & = delete;

  /* Move Constructor */
  SoftECU(SoftECU &&) = delete;

  /* Move Assignment Operator */
  SoftECU &operator=(SoftECU &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_SoftECU_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_SoftECU_T *pExtU_SoftECU_T) { SoftECU_U = *pExtU_SoftECU_T; }

  /* Root outports get method */
  const ExtY_SoftECU_T &getExternalOutputs() const { return SoftECU_Y; }

  /* model start function */
  void start();

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  static void terminate();

  /* Constructor */
  SoftECU();

  /* Destructor */
  ~SoftECU();

 protected:
  /* Tunable parameters */
  static P_SoftECU_T SoftECU_P;

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_SoftECU_T SoftECU_U;

  /* External outputs */
  ExtY_SoftECU_T SoftECU_Y;

  /* Block signals */
  B_SoftECU_T SoftECU_B;

  /* Block states */
  DW_SoftECU_T SoftECU_DW;

  /* Tunable parameters */
  // static P_SoftECU_T SoftECU_P;

  /* Block continuous states */
  X_SoftECU_T SoftECU_X;

  /* private member function(s) for subsystem '<S1>/ABS_EBD'*/
  void SoftECU_ABS_EBD_Init();
  void SoftECU_ABS_EBD_Update();
  void SoftECU_ABS_EBD();

  /* private member function(s) for subsystem '<S1>/EMS'*/
  void SoftECU_EMS_Init();
  void SoftECU_EMS_Update();
  void SoftECU_EMS();

  /* private member function(s) for subsystem '<S1>/VCU'*/
  void SoftECU_VCU_Init();
  void SoftECU_VCU_Update();
  void SoftECU_VCU();

  /* private member function(s) for subsystem '<S1>/ax_estimator'*/
  static void SoftECU_ax_estimator_Init(DW_ax_estimator_SoftECU_T *localDW, P_ax_estimator_SoftECU_T *localP,
                                        X_ax_estimator_SoftECU_T *localX);
  static void SoftECU_ax_estimator_Deriv(B_ax_estimator_SoftECU_T *localB, XDot_ax_estimator_SoftECU_T *localXdot);
  void SoftECU_ax_estimator_Update(real_T rtu_vx, DW_ax_estimator_SoftECU_T *localDW);
  void SoftECU_ax_estimator(real_T rtu_acc_feedback_m_s2, real_T rtu_vx, B_ax_estimator_SoftECU_T *localB,
                            DW_ax_estimator_SoftECU_T *localDW, P_ax_estimator_SoftECU_T *localP,
                            P_SoftECU_T *SoftECU_P, X_ax_estimator_SoftECU_T *localX);

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void SoftECU_derivatives();

  /* Real-Time Model */
  RT_MODEL_SoftECU_T SoftECU_M;
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
 * '<Root>' : 'SoftECU'
 * '<S1>'   : 'SoftECU/SoftECU'
 * '<S2>'   : 'SoftECU/SoftECU/ABS_EBD'
 * '<S3>'   : 'SoftECU/SoftECU/AutoHold'
 * '<S4>'   : 'SoftECU/SoftECU/EMS'
 * '<S5>'   : 'SoftECU/SoftECU/VCU'
 * '<S6>'   : 'SoftECU/SoftECU/ax_estimator'
 * '<S7>'   : 'SoftECU/SoftECU/speedCutOff'
 * '<S8>'   : 'SoftECU/SoftECU/ABS_EBD/accControl'
 * '<S9>'   : 'SoftECU/SoftECU/ABS_EBD/pressureControl'
 * '<S10>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller'
 * '<S11>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Subsystem'
 * '<S12>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Anti-windup'
 * '<S13>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/D Gain'
 * '<S14>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Filter'
 * '<S15>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Filter ICs'
 * '<S16>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/I Gain'
 * '<S17>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain'
 * '<S18>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain Fdbk'
 * '<S19>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Integrator'
 * '<S20>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Integrator ICs'
 * '<S21>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/N Copy'
 * '<S22>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/N Gain'
 * '<S23>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/P Copy'
 * '<S24>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Parallel P Gain'
 * '<S25>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Reset Signal'
 * '<S26>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Saturation'
 * '<S27>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Saturation Fdbk'
 * '<S28>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Sum'
 * '<S29>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Sum Fdbk'
 * '<S30>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode'
 * '<S31>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode Sum'
 * '<S32>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Integral'
 * '<S33>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Ngain'
 * '<S34>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/postSat Signal'
 * '<S35>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/preSat Signal'
 * '<S36>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Anti-windup/Passthrough'
 * '<S37>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/D Gain/Internal Parameters'
 * '<S38>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Filter/Disc. Forward Euler Filter'
 * '<S39>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Filter ICs/Internal IC - Filter'
 * '<S40>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/I Gain/Internal Parameters'
 * '<S41>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain/Passthrough'
 * '<S42>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S43>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Integrator/Discrete'
 * '<S44>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Integrator ICs/Internal IC'
 * '<S45>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/N Copy/Disabled'
 * '<S46>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/N Gain/Internal Parameters'
 * '<S47>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/P Copy/Disabled'
 * '<S48>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Parallel P Gain/Internal Parameters'
 * '<S49>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Reset Signal/External Reset'
 * '<S50>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Saturation/Enabled'
 * '<S51>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Saturation Fdbk/Disabled'
 * '<S52>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Sum/Sum_PID'
 * '<S53>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Sum Fdbk/Disabled'
 * '<S54>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode/Disabled'
 * '<S55>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode Sum/Passthrough'
 * '<S56>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Integral/Passthrough'
 * '<S57>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Ngain/Passthrough'
 * '<S58>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/postSat Signal/Forward_Path'
 * '<S59>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Discrete PID Controller/preSat Signal/Forward_Path'
 * '<S60>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Subsystem/brakeEnableThresh'
 * '<S61>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Subsystem/brakeEnableThresh_Reverse'
 * '<S62>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Subsystem/notReverse'
 * '<S63>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Subsystem/reverseGear'
 * '<S64>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Subsystem/vxCheck'
 * '<S65>'  : 'SoftECU/SoftECU/ABS_EBD/accControl/Subsystem/vxCheckReverse'
 * '<S66>'  : 'SoftECU/SoftECU/ABS_EBD/pressureControl/Compare To Constant'
 * '<S67>'  : 'SoftECU/SoftECU/AutoHold/AutoHold'
 * '<S68>'  : 'SoftECU/SoftECU/AutoHold/EgoSpeedLow'
 * '<S69>'  : 'SoftECU/SoftECU/AutoHold/EgoSpeedLow/AutoHoldSpeedThresh'
 * '<S70>'  : 'SoftECU/SoftECU/EMS/CheckIfReset'
 * '<S71>'  : 'SoftECU/SoftECU/EMS/Compare To Constant'
 * '<S72>'  : 'SoftECU/SoftECU/EMS/accSignAdapter'
 * '<S73>'  : 'SoftECU/SoftECU/EMS/throttleControlMode'
 * '<S74>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode'
 * '<S75>'  : 'SoftECU/SoftECU/EMS/useTorqueRequestMode'
 * '<S76>'  : 'SoftECU/SoftECU/EMS/CheckIfReset/Compare To Constant'
 * '<S77>'  : 'SoftECU/SoftECU/EMS/CheckIfReset/Compare To Constant1'
 * '<S78>'  : 'SoftECU/SoftECU/EMS/accSignAdapter/forwardGear'
 * '<S79>'  : 'SoftECU/SoftECU/EMS/accSignAdapter/reverseGear1'
 * '<S80>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Compare To Constant1'
 * '<S81>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller'
 * '<S82>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Anti-windup'
 * '<S83>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/D Gain'
 * '<S84>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Filter'
 * '<S85>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Filter ICs'
 * '<S86>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/I Gain'
 * '<S87>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Ideal P Gain'
 * '<S88>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Ideal P Gain Fdbk'
 * '<S89>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Integrator'
 * '<S90>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Integrator ICs'
 * '<S91>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/N Copy'
 * '<S92>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/N Gain'
 * '<S93>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/P Copy'
 * '<S94>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Parallel P Gain'
 * '<S95>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Reset Signal'
 * '<S96>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Saturation'
 * '<S97>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Saturation Fdbk'
 * '<S98>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Sum'
 * '<S99>'  : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Sum Fdbk'
 * '<S100>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Tracking Mode'
 * '<S101>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Tracking Mode Sum'
 * '<S102>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Tsamp - Integral'
 * '<S103>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Tsamp - Ngain'
 * '<S104>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/postSat Signal'
 * '<S105>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/preSat Signal'
 * '<S106>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Anti-windup/Passthrough'
 * '<S107>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/D Gain/Internal Parameters'
 * '<S108>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Filter/Disc. Forward Euler Filter'
 * '<S109>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Filter ICs/Internal IC - Filter'
 * '<S110>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/I Gain/Internal Parameters'
 * '<S111>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Ideal P Gain/Passthrough'
 * '<S112>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S113>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Integrator/Discrete'
 * '<S114>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Integrator ICs/Internal IC'
 * '<S115>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/N Copy/Disabled'
 * '<S116>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/N Gain/Internal Parameters'
 * '<S117>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/P Copy/Disabled'
 * '<S118>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Parallel P Gain/Internal Parameters'
 * '<S119>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Reset Signal/External Reset'
 * '<S120>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Saturation/Enabled'
 * '<S121>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Saturation Fdbk/Disabled'
 * '<S122>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Sum/Sum_PID'
 * '<S123>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Sum Fdbk/Disabled'
 * '<S124>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Tracking Mode/Disabled'
 * '<S125>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Tracking Mode Sum/Passthrough'
 * '<S126>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Tsamp - Integral/Passthrough'
 * '<S127>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/Tsamp - Ngain/Passthrough'
 * '<S128>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/postSat Signal/Forward_Path'
 * '<S129>' : 'SoftECU/SoftECU/EMS/useTargetAccMode/Discrete PID Controller/preSat Signal/Forward_Path'
 * '<S130>' : 'SoftECU/SoftECU/VCU/CheckIfReset'
 * '<S131>' : 'SoftECU/SoftECU/VCU/ForwardAndPositiveTorque'
 * '<S132>' : 'SoftECU/SoftECU/VCU/ReverseOrNegativeTorque(PowerRecycle)'
 * '<S133>' : 'SoftECU/SoftECU/VCU/useAccMode'
 * '<S134>' : 'SoftECU/SoftECU/VCU/useTorqueMode'
 * '<S135>' : 'SoftECU/SoftECU/VCU/CheckIfReset/Compare To Constant'
 * '<S136>' : 'SoftECU/SoftECU/VCU/CheckIfReset/Compare To Constant1'
 * '<S137>' : 'SoftECU/SoftECU/VCU/ForwardAndPositiveTorque/Compare To Constant'
 * '<S138>' : 'SoftECU/SoftECU/VCU/ReverseOrNegativeTorque(PowerRecycle)/Compare To Constant'
 * '<S139>' : 'SoftECU/SoftECU/VCU/ReverseOrNegativeTorque(PowerRecycle)/recycleTorqueLimit'
 * '<S140>' : 'SoftECU/SoftECU/VCU/ReverseOrNegativeTorque(PowerRecycle)/recycleTorqueSaturation'
 * '<S141>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode'
 * '<S142>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Compare To Constant'
 * '<S143>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller'
 * '<S144>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/speedLow_x[m|s]'
 * '<S145>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Anti-windup'
 * '<S146>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/D Gain'
 * '<S147>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Filter'
 * '<S148>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Filter ICs'
 * '<S149>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/I Gain'
 * '<S150>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Ideal P Gain'
 * '<S151>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Ideal P Gain Fdbk'
 * '<S152>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Integrator'
 * '<S153>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Integrator ICs'
 * '<S154>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/N Copy'
 * '<S155>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/N Gain'
 * '<S156>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/P Copy'
 * '<S157>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Parallel P Gain'
 * '<S158>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Reset Signal'
 * '<S159>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Saturation'
 * '<S160>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Saturation Fdbk'
 * '<S161>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Sum'
 * '<S162>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Sum Fdbk'
 * '<S163>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Tracking Mode'
 * '<S164>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Tracking Mode Sum'
 * '<S165>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Tsamp - Integral'
 * '<S166>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Tsamp - Ngain'
 * '<S167>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/postSat Signal'
 * '<S168>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/preSat Signal'
 * '<S169>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Anti-windup/Passthrough'
 * '<S170>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/D Gain/Internal Parameters'
 * '<S171>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Filter/Disc. Forward Euler Filter'
 * '<S172>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Filter ICs/Internal IC - Filter'
 * '<S173>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/I Gain/Internal Parameters'
 * '<S174>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Ideal P Gain/Passthrough'
 * '<S175>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S176>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Integrator/Discrete'
 * '<S177>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Integrator ICs/Internal IC'
 * '<S178>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/N Copy/Disabled'
 * '<S179>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/N Gain/Internal Parameters'
 * '<S180>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/P Copy/Disabled'
 * '<S181>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Parallel P Gain/Internal Parameters'
 * '<S182>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Reset Signal/External Reset'
 * '<S183>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Saturation/Enabled'
 * '<S184>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Saturation Fdbk/Disabled'
 * '<S185>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Sum/Sum_PID'
 * '<S186>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Sum Fdbk/Disabled'
 * '<S187>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Tracking Mode/Disabled'
 * '<S188>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Tracking Mode Sum/Passthrough'
 * '<S189>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Tsamp - Integral/Passthrough'
 * '<S190>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/Tsamp - Ngain/Passthrough'
 * '<S191>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/postSat Signal/Forward_Path'
 * '<S192>' : 'SoftECU/SoftECU/VCU/useAccMode/accMode/Discrete PID Controller/preSat Signal/Forward_Path'
 * '<S193>' : 'SoftECU/SoftECU/VCU/useTorqueMode/Compare To Constant1'
 * '<S194>' : 'SoftECU/SoftECU/VCU/useTorqueMode/Compare To Constant2'
 * '<S195>' : 'SoftECU/SoftECU/ax_estimator/AxAct'
 * '<S196>' : 'SoftECU/SoftECU/ax_estimator/AxCal'
 * '<S197>' : 'SoftECU/SoftECU/ax_estimator/If Action Subsystem'
 * '<S198>' : 'SoftECU/SoftECU/ax_estimator/SpeedThresh'
 */
#endif /* RTW_HEADER_SoftECU_h_ */
