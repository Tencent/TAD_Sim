/*
 * TX_Transmission.h
 *
 * Code generation for model "TX_Transmission".
 *
 * Model version              : 1.210
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Dec  6 16:43:43 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_TX_Transmission_h_
#define RTW_HEADER_TX_Transmission_h_
#include "TX_Transmission_types.h"
#include "rt_zcfcn.h"
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
struct B_TX_Transmission_T {
  real_T Memory;              /* '<S15>/Memory' */
  real_T Constant1;           /* '<S13>/Constant1' */
  real_T Round;               /* '<S13>/Round' */
  real_T Memory_d;            /* '<S12>/Memory' */
  real_T Constant;            /* '<S8>/Constant' */
  real_T Memory_c;            /* '<S52>/Memory' */
  real_T domega_o;            /* '<S51>/domega_o' */
  real_T Integrator1;         /* '<S64>/Integrator1' */
  real_T DeadZone;            /* '<S13>/Dead Zone' */
  real_T IC;                  /* '<S20>/IC' */
  real_T Switch;              /* '<S20>/Switch' */
  real_T IC_i;                /* '<S18>/IC' */
  real_T Switch_h;            /* '<S18>/Switch' */
  real_T IC_e;                /* '<S16>/IC' */
  real_T Switch_j;            /* '<S16>/Switch' */
  real_T Divide;              /* '<S9>/Divide' */
  real_T upi;                 /* '<S13>/2*pi' */
  real_T Product;             /* '<S15>/Product' */
  real_T Subtract;            /* '<S51>/Subtract' */
  real_T Product_i;           /* '<S52>/Product' */
  real_T Divide_f;            /* '<S64>/Divide' */
  real_T Product4;            /* '<S19>/Product4' */
  real_T Product8;            /* '<S19>/Product8' */
  real_T Product8_m;          /* '<S17>/Product8' */
  int8_T Saturation;          /* '<S13>/Saturation' */
  int8_T GearCmd;             /* '<S59>/TCM Shift Controller' */
  int8_T y;                   /* '<S2>/hold' */
  boolean_T gearMode;         /* '<S60>/Logical Operator' */
  boolean_T gearMode_i;       /* '<S62>/Logical Operator' */
  boolean_T LogicalOperator1; /* '<S61>/Logical Operator1' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_TX_Transmission_T {
  real_T Memory_PreviousInput;          /* '<S15>/Memory' */
  real_T Memory_PreviousInput_h;        /* '<S12>/Memory' */
  real_T Memory_PreviousInput_o;        /* '<S52>/Memory' */
  int32_T DelayInput1_DSTATE;           /* '<S6>/Delay Input1' */
  int32_T sfEvent;                      /* '<S59>/TCM Shift Controller' */
  uint32_T temporalCounter_i1;          /* '<S59>/TCM Shift Controller' */
  uint32_T temporalCounter_i1_e;        /* '<S2>/hold' */
  int_T Integrator_IWORK;               /* '<S15>/Integrator' */
  int_T Integrator_IWORK_o;             /* '<S9>/Integrator' */
  int_T Integrator_IWORK_i;             /* '<S52>/Integrator' */
  int_T xe_IWORK;                       /* '<S19>/xe' */
  int_T xv_IWORK;                       /* '<S19>/xv' */
  int_T x_IWORK;                        /* '<S17>/x' */
  int8_T If_ActiveSubsystem;            /* '<S13>/If' */
  uint8_T is_GearCmd;                   /* '<S59>/TCM Shift Controller' */
  uint8_T is_SelectionState;            /* '<S59>/TCM Shift Controller' */
  uint8_T is_active_c4_TX_Transmission; /* '<S59>/TCM Shift Controller' */
  uint8_T is_active_GearCmd;            /* '<S59>/TCM Shift Controller' */
  uint8_T is_active_SelectionState;     /* '<S59>/TCM Shift Controller' */
  uint8_T is_c3_TX_Transmission;        /* '<S2>/hold' */
  uint8_T is_active_c3_TX_Transmission; /* '<S2>/hold' */
  boolean_T IC_FirstOutputTime;         /* '<S20>/IC' */
  boolean_T IC_FirstOutputTime_p;       /* '<S18>/IC' */
  boolean_T IC_FirstOutputTime_o;       /* '<S16>/IC' */
};

/* Continuous states (default storage) */
struct X_TX_Transmission_T {
  real_T Integrator_CSTATE;    /* '<S15>/Integrator' */
  real_T Integrator_CSTATE_h;  /* '<S9>/Integrator' */
  real_T Integrator_CSTATE_c;  /* '<S52>/Integrator' */
  real_T Integrator_CSTATE_hz; /* '<S51>/Integrator' */
  real_T Integrator1_CSTATE;   /* '<S64>/Integrator1' */
  real_T we;                   /* '<S19>/xe' */
  real_T wv;                   /* '<S19>/xv' */
  real_T w;                    /* '<S17>/x' */
};

/* State derivatives (default storage) */
struct XDot_TX_Transmission_T {
  real_T Integrator_CSTATE;    /* '<S15>/Integrator' */
  real_T Integrator_CSTATE_h;  /* '<S9>/Integrator' */
  real_T Integrator_CSTATE_c;  /* '<S52>/Integrator' */
  real_T Integrator_CSTATE_hz; /* '<S51>/Integrator' */
  real_T Integrator1_CSTATE;   /* '<S64>/Integrator1' */
  real_T we;                   /* '<S19>/xe' */
  real_T wv;                   /* '<S19>/xv' */
  real_T w;                    /* '<S17>/x' */
};

/* State disabled  */
struct XDis_TX_Transmission_T {
  boolean_T Integrator_CSTATE;    /* '<S15>/Integrator' */
  boolean_T Integrator_CSTATE_h;  /* '<S9>/Integrator' */
  boolean_T Integrator_CSTATE_c;  /* '<S52>/Integrator' */
  boolean_T Integrator_CSTATE_hz; /* '<S51>/Integrator' */
  boolean_T Integrator1_CSTATE;   /* '<S64>/Integrator1' */
  boolean_T we;                   /* '<S19>/xe' */
  boolean_T wv;                   /* '<S19>/xv' */
  boolean_T w;                    /* '<S17>/x' */
};

/* Zero-crossing (trigger) state */
struct PrevZCX_TX_Transmission_T {
  ZCSigState Integrator_Reset_ZCE;   /* '<S15>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_e; /* '<S9>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_c; /* '<S52>/Integrator' */
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
struct ExtU_TX_Transmission_T {
  driveline_in DriveLineIn; /* '<Root>/DriveLineIn' */
  powecu_out SoftECUOut;    /* '<Root>/SoftECUOut' */
  real_T PropShftSpd;       /* '<Root>/PropShftSpd' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_TX_Transmission_T {
  real_T TransGear;    /* '<Root>/TransGear[]' */
  real_T EngSpeedrads; /* '<Root>/EngSpeed[rad|s]' */
  real_T DrvShfTrqNm;  /* '<Root>/DrvShfTrq[Nm]' */
};

/* Parameters (default storage) */
struct P_TX_Transmission_T_ {
  struct_TdKnmvEBt9bbFNHrBt8OwG Trans;                      /* Variable: Trans
                                                             * Referenced by:
                                                             *   '<S13>/Constant'
                                                             *   '<S19>/Gear2damping'
                                                             *   '<S19>/Gear2inertias'
                                                             *   '<S19>/Gear2inertias1'
                                                             *   '<S31>/Gear2Ratios'
                                                             *   '<S31>/Gear2damping'
                                                             *   '<S31>/Gear2inertias'
                                                             *   '<S43>/Gear2Ratios'
                                                             *   '<S43>/Gear2damping'
                                                             *   '<S43>/Gear2inertias'
                                                             *   '<S40>/Eta 1D'
                                                             *   '<S50>/Eta 1D'
                                                             */
  struct_WPo6c0btEjgdkiBqVilJ2B VEH;                        /* Variable: VEH
                                                             * Referenced by:
                                                             *   '<S8>/Constant'
                                                             *   '<S16>/Constant'
                                                             *   '<S20>/Constant'
                                                             */
  real_T Dn_Gears[tx_car::kMap1dSize];                /* Variable: Dn_Gears
                                                             * Referenced by:
                                                             *   '<S70>/Calculate  Downshift Threshold'
                                                             *   '<S71>/Calculate  Downshift Threshold'
                                                             */
  real_T Downshift_Speeds[tx_car::kMap2dSize];        /* Variable: Downshift_Speeds
                                                             * Referenced by:
                                                             *   '<S70>/Calculate  Downshift Threshold'
                                                             *   '<S71>/Calculate  Downshift Threshold'
                                                             */
  real_T EngIdleSpd;                                        /* Variable: EngIdleSpd
                                                             * Referenced by:
                                                             *   '<S8>/Saturation'
                                                             *   '<S18>/Constant'
                                                             */
  real_T Pedal_Positions_DnShift[tx_car::kMap1dSize]; /* Variable: Pedal_Positions_DnShift
                                                             * Referenced by:
                                                             *   '<S70>/Calculate  Downshift Threshold'
                                                             *   '<S71>/Calculate  Downshift Threshold'
                                                             */
  real_T Pedal_Positions_UpShift[tx_car::kMap1dSize]; /* Variable: Pedal_Positions_UpShift
                                                             * Referenced by:
                                                             *   '<S70>/Calculate Upshift Threshold'
                                                             *   '<S72>/Calculate Upshift Threshold'
                                                             */
  real_T StatLdWhlR[4];                                     /* Variable: StatLdWhlR
                                                             * Referenced by:
                                                             *   '<S5>/speed'
                                                             *   '<S8>/Constant'
                                                             *   '<S59>/Gain'
                                                             *   '<S16>/Constant'
                                                             *   '<S20>/Constant'
                                                             */
  real_T Up_Gears[tx_car::kMap1dSize];                /* Variable: Up_Gears
                                                             * Referenced by:
                                                             *   '<S70>/Calculate Upshift Threshold'
                                                             *   '<S72>/Calculate Upshift Threshold'
                                                             */
  real_T Upshift_Speeds[tx_car::kMap2dSize];          /* Variable: Upshift_Speeds
                                                             * Referenced by:
                                                             *   '<S70>/Calculate Upshift Threshold'
                                                             *   '<S72>/Calculate Upshift Threshold'
                                                             */
  real_T init_gear_num;                                     /* Variable: init_gear_num
                                                             * Referenced by:
                                                             *   '<S59>/init_gear_num'
                                                             *   '<S13>/Constant1'
                                                             */
  real_T max_gear_num;                                      /* Variable: max_gear_num
                                                             * Referenced by: '<S59>/max_gear_num'
                                                             */
  real_T init_gear_ratio;                                   /* Variable: init_gear_ratio
                                                             * Referenced by:
                                                             *   '<S8>/Constant'
                                                             *   '<S16>/Constant'
                                                             *   '<S20>/Constant'
                                                             */
  real_T pre_shift_wait_time;                               /* Variable: pre_shift_wait_time
                                                             * Referenced by: '<S59>/TCM Shift Controller'
                                                             */
  real_T ratio_diff_front;                                  /* Variable: ratio_diff_front
                                                             * Referenced by:
                                                             *   '<S8>/Constant'
                                                             *   '<S16>/Constant'
                                                             *   '<S20>/Constant'
                                                             */
  real_T ratio_diff_rear;                                   /* Variable: ratio_diff_rear
                                                             * Referenced by:
                                                             *   '<S8>/Constant'
                                                             *   '<S16>/Constant'
                                                             *   '<S20>/Constant'
                                                             */
  real_T PropShaft_b;                                       /* Mask Parameter: PropShaft_b
                                                             * Referenced by: '<S51>/Gain2'
                                                             */
  real_T CompareToConstant1_const;                          /* Mask Parameter: CompareToConstant1_const
                                                             * Referenced by: '<S66>/Constant'
                                                             */
  real_T CompareToConstant_const;                           /* Mask Parameter: CompareToConstant_const
                                                             * Referenced by: '<S65>/Constant'
                                                             */
  real_T CompareToConstant1_const_m;
  /* Mask Parameter: CompareToConstant1_const_m
   * Referenced by: '<S69>/Constant'
   */
  real_T CompareToConstant_const_h; /* Mask Parameter: CompareToConstant_const_h
                                     * Referenced by: '<S68>/Constant'
                                     */
  real_T CompareToConstant_const_p; /* Mask Parameter: CompareToConstant_const_p
                                     * Referenced by: '<S67>/Constant'
                                     */
  real_T CompareToConstant_const_i; /* Mask Parameter: CompareToConstant_const_i
                                     * Referenced by: '<S73>/Constant'
                                     */
  real_T PropShaft_domega_o;        /* Mask Parameter: PropShaft_domega_o
                                     * Referenced by: '<S51>/domega_o'
                                     */
  real_T PropShaft_k;               /* Mask Parameter: PropShaft_k
                                     * Referenced by: '<S51>/Gain1'
                                     */
  real_T PropShaft_omega_c;         /* Mask Parameter: PropShaft_omega_c
                                     * Referenced by: '<S51>/omega_c'
                                     */
  real_T PropShaft_theta_o;         /* Mask Parameter: PropShaft_theta_o
                                     * Referenced by: '<S51>/Integrator'
                                     */
  real_T div0protectabspoly_thresh; /* Mask Parameter: div0protectabspoly_thresh
                                     * Referenced by:
                                     *   '<S37>/Constant'
                                     *   '<S38>/Constant'
                                     */
  real_T div0protectabspoly_thresh_h;
  /* Mask Parameter: div0protectabspoly_thresh_h
   * Referenced by:
   *   '<S47>/Constant'
   *   '<S48>/Constant'
   */
  real_T ContLPFIC_wc;              /* Mask Parameter: ContLPFIC_wc
                                     * Referenced by: '<S9>/Constant'
                                     */
  real_T Valve_wc;                  /* Mask Parameter: Valve_wc
                                     * Referenced by: '<S64>/Constant'
                                     */
  int32_T GearCmdChange_vinit;      /* Mask Parameter: GearCmdChange_vinit
                                     * Referenced by: '<S6>/Delay Input1'
                                     */
  real_T Constant_Value;            /* Expression: 1
                                     * Referenced by: '<S28>/Constant'
                                     */
  real_T Switch_Threshold;          /* Expression: 0
                                     * Referenced by: '<S28>/Switch'
                                     */
  real_T Gain1_Gain;                /* Expression: 20
                                     * Referenced by: '<S33>/Gain1'
                                     */
  real_T Constant1_Value;           /* Expression: 1
                                     * Referenced by: '<S33>/Constant1'
                                     */
  real_T Gain2_Gain;                /* Expression: 4
                                     * Referenced by: '<S33>/Gain2'
                                     */
  real_T Gain_Gain;                 /* Expression: .5
                                     * Referenced by: '<S33>/Gain'
                                     */
  real_T Constant_Value_k;          /* Expression: 1
                                     * Referenced by: '<S32>/Constant'
                                     */
  real_T Constant_Value_p;          /* Expression: 1
                                     * Referenced by: '<S29>/Constant'
                                     */
  real_T Switch_Threshold_b;        /* Expression: 0
                                     * Referenced by: '<S29>/Switch'
                                     */
  real_T Gain1_Gain_o;              /* Expression: 20
                                     * Referenced by: '<S35>/Gain1'
                                     */
  real_T Constant1_Value_h;         /* Expression: 1
                                     * Referenced by: '<S35>/Constant1'
                                     */
  real_T Gain2_Gain_j;              /* Expression: 4
                                     * Referenced by: '<S35>/Gain2'
                                     */
  real_T Gain_Gain_i;               /* Expression: .5
                                     * Referenced by: '<S35>/Gain'
                                     */
  real_T Constant_Value_m;          /* Expression: 1
                                     * Referenced by: '<S34>/Constant'
                                     */
  real_T Gain_Gain_o;               /* Expression: -1
                                     * Referenced by: '<S30>/Gain'
                                     */
  real_T Saturation1_UpperSat;      /* Expression: 0
                                     * Referenced by: '<S30>/Saturation1'
                                     */
  real_T Saturation1_LowerSat;      /* Expression: -inf
                                     * Referenced by: '<S30>/Saturation1'
                                     */
  real_T Saturation2_UpperSat;      /* Expression: inf
                                     * Referenced by: '<S30>/Saturation2'
                                     */
  real_T Saturation2_LowerSat;      /* Expression: 0
                                     * Referenced by: '<S30>/Saturation2'
                                     */
  real_T Saturation_UpperSat;       /* Expression: 1
                                     * Referenced by: '<S30>/Saturation'
                                     */
  real_T Saturation_LowerSat;       /* Expression: 0
                                     * Referenced by: '<S30>/Saturation'
                                     */
  real_T Neutral_Value;             /* Expression: 0
                                     * Referenced by: '<S19>/Neutral'
                                     */
  real_T Constant_Value_i;          /* Expression: 1
                                     * Referenced by: '<S41>/Constant'
                                     */
  real_T Switch_Threshold_l;        /* Expression: 0
                                     * Referenced by: '<S41>/Switch'
                                     */
  real_T Gain1_Gain_m;              /* Expression: 20
                                     * Referenced by: '<S45>/Gain1'
                                     */
  real_T Constant1_Value_l;         /* Expression: 1
                                     * Referenced by: '<S45>/Constant1'
                                     */
  real_T Gain2_Gain_o;              /* Expression: 4
                                     * Referenced by: '<S45>/Gain2'
                                     */
  real_T Gain_Gain_oj;              /* Expression: .5
                                     * Referenced by: '<S45>/Gain'
                                     */
  real_T Constant_Value_f;          /* Expression: 1
                                     * Referenced by: '<S44>/Constant'
                                     */
  real_T First_Value;               /* Expression: 1
                                     * Referenced by: '<S19>/First'
                                     */
  real_T Gain_Gain_e;               /* Expression: -1
                                     * Referenced by: '<S42>/Gain'
                                     */
  real_T Saturation1_UpperSat_d;    /* Expression: 0
                                     * Referenced by: '<S42>/Saturation1'
                                     */
  real_T Saturation1_LowerSat_j;    /* Expression: -inf
                                     * Referenced by: '<S42>/Saturation1'
                                     */
  real_T Saturation2_UpperSat_l;    /* Expression: inf
                                     * Referenced by: '<S42>/Saturation2'
                                     */
  real_T Saturation2_LowerSat_e;    /* Expression: 0
                                     * Referenced by: '<S42>/Saturation2'
                                     */
  real_T Saturation_UpperSat_p;     /* Expression: 1
                                     * Referenced by: '<S42>/Saturation'
                                     */
  real_T Saturation_LowerSat_g;     /* Expression: 0
                                     * Referenced by: '<S42>/Saturation'
                                     */
  real_T up_th_Y0;                  /* Computed Parameter: up_th_Y0
                                     * Referenced by: '<S72>/up_th'
                                     */
  real_T down_th_Y0;                /* Computed Parameter: down_th_Y0
                                     * Referenced by: '<S71>/down_th'
                                     */
  real_T SpdThr_Y0;                 /* Computed Parameter: SpdThr_Y0
                                     * Referenced by: '<S70>/SpdThr'
                                     */
  real_T Constant_Value_pi;         /* Expression: 0
                                     * Referenced by: '<S74>/Constant'
                                     */
  real_T Memory_InitialCondition;   /* Expression: 0
                                     * Referenced by: '<S15>/Memory'
                                     */
  real_T Memory_InitialCondition_d; /* Expression: 0
                                     * Referenced by: '<S12>/Memory'
                                     */
  real_T Saturation_UpperSat_g;     /* Expression: 9000.0/60.0*2*3.1415926
                                     * Referenced by: '<S8>/Saturation'
                                     */
  real_T Memory_InitialCondition_o; /* Expression: 0
                                     * Referenced by: '<S52>/Memory'
                                     */
  real_T Integrator1_IC;            /* Expression: 0
                                     * Referenced by: '<S64>/Integrator1'
                                     */
  real_T Constant1_Value_hx;        /* Expression: 1e-2
                                     * Referenced by: '<S61>/Constant1'
                                     */
  real_T rpm_Gain;                  /* Expression: 1.0/2.0/pi*60
                                     * Referenced by: '<S4>/rpm'
                                     */
  real_T Constant_Value_e;          /* Expression: 600
                                     * Referenced by: '<S61>/Constant'
                                     */
  real_T Constant2_Value;           /* Expression: 600
                                     * Referenced by: '<S61>/Constant2'
                                     */
  real_T HoldSecond_Value;          /* Expression: 0.5
                                     * Referenced by: '<S2>/HoldSecond'
                                     */
  real_T DeadZone_Start;            /* Expression: -0.5
                                     * Referenced by: '<S13>/Dead Zone'
                                     */
  real_T DeadZone_End;              /* Expression: 0.5
                                     * Referenced by: '<S13>/Dead Zone'
                                     */
  real_T Constant_Value_b;          /* Expression: 0
                                     * Referenced by: '<S5>/Constant'
                                     */
  real_T Constant1_Value_c;         /* Expression: 1
                                     * Referenced by: '<S20>/Constant1'
                                     */
  real_T IC_Value;                  /* Expression: 0
                                     * Referenced by: '<S20>/IC'
                                     */
  real_T Switch_Threshold_i;        /* Expression: 0
                                     * Referenced by: '<S20>/Switch'
                                     */
  real_T Constant1_Value_o;         /* Expression: 1
                                     * Referenced by: '<S18>/Constant1'
                                     */
  real_T IC_Value_d;                /* Expression: 0
                                     * Referenced by: '<S18>/IC'
                                     */
  real_T Switch_Threshold_d;        /* Expression: 0
                                     * Referenced by: '<S18>/Switch'
                                     */
  real_T Constant1_Value_e;         /* Expression: 1
                                     * Referenced by: '<S16>/Constant1'
                                     */
  real_T IC_Value_n;                /* Expression: 0
                                     * Referenced by: '<S16>/IC'
                                     */
  real_T Switch_Threshold_o;        /* Expression: 0
                                     * Referenced by: '<S16>/Switch'
                                     */
  real_T Reset_Value;               /* Expression: 1
                                     * Referenced by: '<S12>/Reset'
                                     */
  real_T upi_Gain;                  /* Expression: 2*pi
                                     * Referenced by: '<S13>/2*pi'
                                     */
  real_T Reset_Value_j;             /* Expression: 1
                                     * Referenced by: '<S15>/Reset'
                                     */
  real_T Merge2_1_InitialOutput;    /* Computed Parameter: Merge2_1_InitialOutput
                                     * Referenced by: '<S13>/Merge2'
                                     */
  real_T Merge2_2_InitialOutput;    /* Computed Parameter: Merge2_2_InitialOutput
                                     * Referenced by: '<S13>/Merge2'
                                     */
  real_T Merge2_3_InitialOutput;    /* Computed Parameter: Merge2_3_InitialOutput
                                     * Referenced by: '<S13>/Merge2'
                                     */
  real_T Reset_Value_l;             /* Expression: 1
                                     * Referenced by: '<S52>/Reset'
                                     */
  real_T kph_Gain;                  /* Expression: 3.6
                                     * Referenced by: '<S59>/kph'
                                     */
  uint32_T CalculateUpshiftThreshold_maxIndex[2];
  /* Computed Parameter: CalculateUpshiftThreshold_maxIndex
   * Referenced by: '<S72>/Calculate Upshift Threshold'
   */
  uint32_T CalculateDownshiftThreshold_maxIndex[2];
  /* Computed Parameter: CalculateDownshiftThreshold_maxIndex
   * Referenced by: '<S71>/Calculate  Downshift Threshold'
   */
  uint32_T CalculateUpshiftThreshold_maxIndex_p[2];
  /* Computed Parameter: CalculateUpshiftThreshold_maxIndex_p
   * Referenced by: '<S70>/Calculate Upshift Threshold'
   */
  uint32_T CalculateDownshiftThreshold_maxIndex_b[2];
  /* Computed Parameter: CalculateDownshiftThreshold_maxIndex_b
   * Referenced by: '<S70>/Calculate  Downshift Threshold'
   */
  int8_T Saturation_UpperSat_k; /* Computed Parameter: Saturation_UpperSat_k
                                 * Referenced by: '<S13>/Saturation'
                                 */
  int8_T Saturation_LowerSat_e; /* Computed Parameter: Saturation_LowerSat_e
                                 * Referenced by: '<S13>/Saturation'
                                 */
};

/* Real-time Model Data Structure */
struct tag_RTM_TX_Transmission_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_TX_Transmission_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_TX_Transmission_T *contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[8];
  real_T odeF[4][8];
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

/* Class declaration for model TX_Transmission */
class MODULE_API TX_Transmission {
  /* public data and function members */
 public:
  /* Copy Constructor */
  TX_Transmission(TX_Transmission const &) = delete;

  /* Assignment Operator */
  TX_Transmission &operator=(TX_Transmission const &) & = delete;

  /* Move Constructor */
  TX_Transmission(TX_Transmission &&) = delete;

  /* Move Assignment Operator */
  TX_Transmission &operator=(TX_Transmission &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_TX_Transmission_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_TX_Transmission_T *pExtU_TX_Transmission_T) {
    TX_Transmission_U = *pExtU_TX_Transmission_T;
  }

  /* Root outports get method */
  const ExtY_TX_Transmission_T &getExternalOutputs() const { return TX_Transmission_Y; }

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
  TX_Transmission();

  /* Destructor */
  ~TX_Transmission();

 protected:
  int m_AT_gear_num = 10;  // -1 0 1~8

  int m_AT_shift_gear_num = 7;  //  1->2, 2->3 ...

  int m_upshift_accload_points_num = 4;

  int m_downshift_accload_points_num = 4;

  /* Tunable parameters */
  static P_TX_Transmission_T TX_Transmission_P;

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_TX_Transmission_T TX_Transmission_U;

  /* External outputs */
  ExtY_TX_Transmission_T TX_Transmission_Y;

  /* Block signals */
  B_TX_Transmission_T TX_Transmission_B;

  /* Block states */
  DW_TX_Transmission_T TX_Transmission_DW;

  /* Tunable parameters */
  // static P_TX_Transmission_T TX_Transmission_P;

  /* Block continuous states */
  X_TX_Transmission_T TX_Transmission_X;

  /* Triggered events */
  PrevZCX_TX_Transmission_T TX_Transmission_PrevZCX;

  /* private member function(s) for subsystem '<Root>'*/
  void TX_Transmission_GearCmd(void);

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void TX_Transmission_derivatives();

  /* Real-Time Model */
  RT_MODEL_TX_Transmission_T TX_Transmission_M;
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
 * '<Root>' : 'TX_Transmission'
 * '<S1>'   : 'TX_Transmission/Transmission'
 * '<S2>'   : 'TX_Transmission/Transmission/HoldGearCmd'
 * '<S3>'   : 'TX_Transmission/Transmission/Transmission3'
 * '<S4>'   : 'TX_Transmission/Transmission/TransmissionController'
 * '<S5>'   : 'TX_Transmission/Transmission/clutch'
 * '<S6>'   : 'TX_Transmission/Transmission/HoldGearCmd/GearCmdChange'
 * '<S7>'   : 'TX_Transmission/Transmission/HoldGearCmd/hold'
 * '<S8>'   : 'TX_Transmission/Transmission/Transmission3/Ideal'
 * '<S9>'   : 'TX_Transmission/Transmission/Transmission3/Ideal/Cont LPF IC'
 * '<S10>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission'
 * '<S11>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Prop Shaft'
 * '<S12>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Cont LPF IC/Init'
 * '<S13>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission'
 * '<S14>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation'
 * '<S15>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Cont LPF IC Dyn'
 * '<S16>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/IC tunable'
 * '<S17>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked'
 * '<S18>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Neutral IC'
 * '<S19>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked'
 * '<S20>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/input IC'
 * '<S21>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/Power Accounting Bus Creator'
 * '<S22>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/div0protect - abs poly1'
 * '<S23>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S24>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/Power Accounting Bus Creator/PwrStored Input'
 * '<S25>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S26>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/div0protect - abs poly1/Compare To Constant'
 * '<S27>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/div0protect - abs poly1/Compare To Constant1'
 * '<S28>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency'
 * '<S29>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency '
 * '<S30>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Mechanical Efficiency'
 * '<S31>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/gear2props'
 * '<S32>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency/Low speed blend'
 * '<S33>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency/Low speed blend/blend'
 * '<S34>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency /Low speed blend'
 * '<S35>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency /Low speed blend/blend'
 * '<S36>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Mechanical Efficiency/div0protect - abs poly'
 * '<S37>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Mechanical Efficiency/div0protect - abs poly/Compare To Constant'
 * '<S38>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Mechanical Efficiency/div0protect - abs poly/Compare To Constant1'
 * '<S39>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/gear2props/Eta Lookup'
 * '<S40>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/gear2props/Eta Lookup/Eta 1D'
 * '<S41>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Apply Efficiency'
 * '<S42>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Mechanical Efficiency'
 * '<S43>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/gear2props'
 * '<S44>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Apply Efficiency/Low speed blend'
 * '<S45>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Apply Efficiency/Low speed blend/blend'
 * '<S46>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Mechanical Efficiency/div0protect - abs poly'
 * '<S47>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Mechanical Efficiency/div0protect - abs poly/Compare To Constant'
 * '<S48>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Mechanical Efficiency/div0protect - abs poly/Compare To Constant1'
 * '<S49>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/gear2props/Eta Lookup'
 * '<S50>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/gear2props/Eta Lookup/Eta 1D'
 * '<S51>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Prop Shaft/Torsional Compliance Linear'
 * '<S52>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Prop Shaft/Torsional Compliance Linear/Cont LPF IC Dyn'
 * '<S53>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Prop Shaft/Torsional Compliance Linear/Power'
 * '<S54>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Prop Shaft/Torsional Compliance Linear/Power/Damping
 * Power'
 * '<S55>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Prop Shaft/Torsional Compliance Linear/Power/Power
 * Accounting Bus Creator'
 * '<S56>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Prop Shaft/Torsional Compliance Linear/Power/Power
 * Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S57>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Prop Shaft/Torsional Compliance Linear/Power/Power
 * Accounting Bus Creator/PwrStored Input'
 * '<S58>'  : 'TX_Transmission/Transmission/Transmission3/Ideal/Prop Shaft/Torsional Compliance Linear/Power/Power
 * Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S59>'  : 'TX_Transmission/Transmission/TransmissionController/TCM'
 * '<S60>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/DriveSelect'
 * '<S61>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/Neutral Select'
 * '<S62>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/ReverseSelect'
 * '<S63>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/TCM Shift Controller'
 * '<S64>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/Valve'
 * '<S65>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/DriveSelect/Compare To Constant'
 * '<S66>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/DriveSelect/Compare To Constant1'
 * '<S67>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/Neutral Select/Compare To Constant'
 * '<S68>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/ReverseSelect/Compare To Constant'
 * '<S69>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/ReverseSelect/Compare To Constant1'
 * '<S70>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/TCM Shift Controller/calc'
 * '<S71>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/TCM Shift Controller/calc_down'
 * '<S72>'  : 'TX_Transmission/Transmission/TransmissionController/TCM/TCM Shift Controller/calc_up'
 * '<S73>'  : 'TX_Transmission/Transmission/clutch/Compare To Constant'
 * '<S74>'  : 'TX_Transmission/Transmission/clutch/Compare To Zero'
 */
#endif /* RTW_HEADER_TX_Transmission_h_ */
