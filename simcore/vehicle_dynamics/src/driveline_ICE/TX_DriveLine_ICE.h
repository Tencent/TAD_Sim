/*
 * TX_DriveLine_ICE.h
 *
 * Code generation for model "TX_DriveLine_ICE".
 *
 * Model version              : 1.200
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Tue Jul 18 21:36:16 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_TX_DriveLine_ICE_h_
#define RTW_HEADER_TX_DriveLine_ICE_h_
#include <cstring>
#include "TX_DriveLine_ICE_types.h"
#include "rt_zcfcn.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rtwtypes.h"
#include "zero_crossing_types.h"

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

/* Block signals for system '<S8>/Limited Slip Differential' */
struct B_LimitedSlipDifferential_TX_DriveLine_ICE_T {
  real_T xdot[2]; /* '<S8>/Limited Slip Differential' */
};

/* Block signals (default storage) */
struct B_TX_DriveLine_ICE_T {
  real_T Memory;                                                             /* '<S43>/Memory' */
  real_T domega_o;                                                           /* '<S42>/domega_o' */
  real_T Memory_e;                                                           /* '<S35>/Memory' */
  real_T domega_o_e;                                                         /* '<S34>/domega_o' */
  real_T Memory_f;                                                           /* '<S211>/Memory' */
  real_T domega_o_eq;                                                        /* '<S210>/domega_o' */
  real_T Memory_j;                                                           /* '<S219>/Memory' */
  real_T domega_o_b;                                                         /* '<S218>/domega_o' */
  real_T Memory_i;                                                           /* '<S144>/Memory' */
  real_T domega_o_br;                                                        /* '<S143>/domega_o' */
  real_T Memory_el;                                                          /* '<S152>/Memory' */
  real_T domega_o_a;                                                         /* '<S151>/domega_o' */
  real_T Memory_b;                                                           /* '<S128>/Memory' */
  real_T domega_o_p;                                                         /* '<S127>/domega_o' */
  real_T Memory_g;                                                           /* '<S136>/Memory' */
  real_T domega_o_g;                                                         /* '<S135>/domega_o' */
  real_T diffDir;                                                            /* '<S14>/Switch' */
  real_T VectorConcatenate[2];                                               /* '<S8>/Vector Concatenate' */
  real_T diffDir_p;                                                          /* '<S190>/Switch' */
  real_T VectorConcatenate_m[2];                                             /* '<S184>/Vector Concatenate' */
  real_T diffDir_k;                                                          /* '<S162>/Switch' */
  real_T VectorConcatenate_n[2];                                             /* '<S58>/Vector Concatenate' */
  real_T Add2;                                                               /* '<S162>/Add2' */
  real_T upi;                                                                /* '<S8>/2*pi' */
  real_T diffDir_d;                                                          /* '<S17>/Switch' */
  real_T Product;                                                            /* '<S10>/Product' */
  real_T Subtract;                                                           /* '<S34>/Subtract' */
  real_T Product_c;                                                          /* '<S35>/Product' */
  real_T Subtract_k;                                                         /* '<S42>/Subtract' */
  real_T Product_a;                                                          /* '<S43>/Product' */
  real_T upi_f;                                                              /* '<S59>/2*pi' */
  real_T Memory_i2;                                                          /* '<S112>/Memory' */
  real_T domega_o_b2;                                                        /* '<S111>/domega_o' */
  real_T diffDir_h;                                                          /* '<S65>/Switch' */
  real_T VectorConcatenate_l[2];                                             /* '<S59>/Vector Concatenate' */
  real_T diffDir_n;                                                          /* '<S68>/Switch' */
  real_T Product_h;                                                          /* '<S61>/Product' */
  real_T upi_e;                                                              /* '<S85>/2*pi' */
  real_T Memory_n;                                                           /* '<S120>/Memory' */
  real_T domega_o_d;                                                         /* '<S119>/domega_o' */
  real_T diffDir_dx;                                                         /* '<S91>/Switch' */
  real_T VectorConcatenate_o[2];                                             /* '<S85>/Vector Concatenate' */
  real_T diffDir_o;                                                          /* '<S94>/Switch' */
  real_T Product_d;                                                          /* '<S87>/Product' */
  real_T Subtract_i;                                                         /* '<S111>/Subtract' */
  real_T Product_e;                                                          /* '<S112>/Product' */
  real_T Subtract_kz;                                                        /* '<S119>/Subtract' */
  real_T Product_k;                                                          /* '<S120>/Product' */
  real_T Subtract_b;                                                         /* '<S127>/Subtract' */
  real_T Product_he;                                                         /* '<S128>/Product' */
  real_T Subtract_h;                                                         /* '<S135>/Subtract' */
  real_T Product_b;                                                          /* '<S136>/Product' */
  real_T Subtract_g;                                                         /* '<S143>/Subtract' */
  real_T Product_j;                                                          /* '<S144>/Product' */
  real_T Subtract_p;                                                         /* '<S151>/Subtract' */
  real_T Product_a3;                                                         /* '<S152>/Product' */
  real_T diffDir_ks;                                                         /* '<S165>/Switch1' */
  real_T upi_i;                                                              /* '<S184>/2*pi' */
  real_T diffDir_m;                                                          /* '<S193>/Switch' */
  real_T Product_cv;                                                         /* '<S186>/Product' */
  real_T Subtract_pe;                                                        /* '<S210>/Subtract' */
  real_T Product_l;                                                          /* '<S211>/Product' */
  real_T Subtract_n;                                                         /* '<S218>/Subtract' */
  real_T Product_ag;                                                         /* '<S219>/Product' */
  real_T xdot[2];                                                            /* '<S58>/TransferCase' */
  B_LimitedSlipDifferential_TX_DriveLine_ICE_T sf_LimitedSlipDifferential_c; /* '<S184>/Limited Slip Differential' */
  B_LimitedSlipDifferential_TX_DriveLine_ICE_T sf_LimitedSlipDifferential_d; /* '<S85>/Limited Slip Differential' */
  B_LimitedSlipDifferential_TX_DriveLine_ICE_T sf_LimitedSlipDifferential_i; /* '<S59>/Limited Slip Differential' */
  B_LimitedSlipDifferential_TX_DriveLine_ICE_T sf_LimitedSlipDifferential;   /* '<S8>/Limited Slip Differential' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_TX_DriveLine_ICE_T {
  real_T Memory_PreviousInput;    /* '<S43>/Memory' */
  real_T Memory_PreviousInput_b;  /* '<S35>/Memory' */
  real_T Memory_PreviousInput_o;  /* '<S211>/Memory' */
  real_T Memory_PreviousInput_h;  /* '<S219>/Memory' */
  real_T Memory_PreviousInput_i;  /* '<S144>/Memory' */
  real_T Memory_PreviousInput_e;  /* '<S152>/Memory' */
  real_T Memory_PreviousInput_d;  /* '<S128>/Memory' */
  real_T Memory_PreviousInput_g;  /* '<S136>/Memory' */
  real_T Memory_PreviousInput_h0; /* '<S112>/Memory' */
  real_T Memory_PreviousInput_hy; /* '<S120>/Memory' */
  int_T Integrator_IWORK;         /* '<S43>/Integrator' */
  int_T Integrator_IWORK_d;       /* '<S35>/Integrator' */
  int_T Integrator_IWORK_e;       /* '<S211>/Integrator' */
  int_T Integrator_IWORK_o;       /* '<S219>/Integrator' */
  int_T Integrator_IWORK_m;       /* '<S144>/Integrator' */
  int_T Integrator_IWORK_c;       /* '<S152>/Integrator' */
  int_T Integrator_IWORK_i;       /* '<S128>/Integrator' */
  int_T Integrator_IWORK_iy;      /* '<S136>/Integrator' */
  int_T Integrator_IWORK_ok;      /* '<S8>/Integrator' */
  int_T Integrator_IWORK_ec;      /* '<S184>/Integrator' */
  int_T Integrator_IWORK_ecw;     /* '<S58>/Integrator' */
  int_T Integrator_IWORK_mo;      /* '<S112>/Integrator' */
  int_T Integrator_IWORK_j;       /* '<S59>/Integrator' */
  int_T Integrator_IWORK_k;       /* '<S120>/Integrator' */
  int_T Integrator_IWORK_f;       /* '<S85>/Integrator' */
};

/* Continuous states (default storage) */
struct X_TX_DriveLine_ICE_T {
  real_T Integrator_CSTATE;       /* '<S43>/Integrator' */
  real_T Integrator_CSTATE_j;     /* '<S42>/Integrator' */
  real_T Integrator_CSTATE_c;     /* '<S35>/Integrator' */
  real_T Integrator_CSTATE_n;     /* '<S34>/Integrator' */
  real_T Integrator_CSTATE_d;     /* '<S211>/Integrator' */
  real_T Integrator_CSTATE_h;     /* '<S210>/Integrator' */
  real_T Integrator_CSTATE_k;     /* '<S219>/Integrator' */
  real_T Integrator_CSTATE_b;     /* '<S218>/Integrator' */
  real_T Integrator_CSTATE_c4;    /* '<S144>/Integrator' */
  real_T Integrator_CSTATE_c3;    /* '<S143>/Integrator' */
  real_T Integrator_CSTATE_nl;    /* '<S152>/Integrator' */
  real_T Integrator_CSTATE_g;     /* '<S151>/Integrator' */
  real_T Integrator_CSTATE_e;     /* '<S128>/Integrator' */
  real_T Integrator_CSTATE_ch;    /* '<S127>/Integrator' */
  real_T Integrator_CSTATE_jc;    /* '<S136>/Integrator' */
  real_T Integrator_CSTATE_l;     /* '<S135>/Integrator' */
  real_T Integrator_CSTATE_hh[2]; /* '<S8>/Integrator' */
  real_T Integrator_CSTATE_a[2];  /* '<S184>/Integrator' */
  real_T Integrator_CSTATE_kh[2]; /* '<S58>/Integrator' */
  real_T Integrator_CSTATE_ca;    /* '<S10>/Integrator' */
  real_T Integrator_CSTATE_p;     /* '<S112>/Integrator' */
  real_T Integrator_CSTATE_px;    /* '<S111>/Integrator' */
  real_T Integrator_CSTATE_i[2];  /* '<S59>/Integrator' */
  real_T Integrator_CSTATE_f;     /* '<S61>/Integrator' */
  real_T Integrator_CSTATE_d0;    /* '<S120>/Integrator' */
  real_T Integrator_CSTATE_dx;    /* '<S119>/Integrator' */
  real_T Integrator_CSTATE_f3[2]; /* '<S85>/Integrator' */
  real_T Integrator_CSTATE_n3;    /* '<S87>/Integrator' */
  real_T Integrator_CSTATE_ba;    /* '<S186>/Integrator' */
};

/* State derivatives (default storage) */
struct XDot_TX_DriveLine_ICE_T {
  real_T Integrator_CSTATE;       /* '<S43>/Integrator' */
  real_T Integrator_CSTATE_j;     /* '<S42>/Integrator' */
  real_T Integrator_CSTATE_c;     /* '<S35>/Integrator' */
  real_T Integrator_CSTATE_n;     /* '<S34>/Integrator' */
  real_T Integrator_CSTATE_d;     /* '<S211>/Integrator' */
  real_T Integrator_CSTATE_h;     /* '<S210>/Integrator' */
  real_T Integrator_CSTATE_k;     /* '<S219>/Integrator' */
  real_T Integrator_CSTATE_b;     /* '<S218>/Integrator' */
  real_T Integrator_CSTATE_c4;    /* '<S144>/Integrator' */
  real_T Integrator_CSTATE_c3;    /* '<S143>/Integrator' */
  real_T Integrator_CSTATE_nl;    /* '<S152>/Integrator' */
  real_T Integrator_CSTATE_g;     /* '<S151>/Integrator' */
  real_T Integrator_CSTATE_e;     /* '<S128>/Integrator' */
  real_T Integrator_CSTATE_ch;    /* '<S127>/Integrator' */
  real_T Integrator_CSTATE_jc;    /* '<S136>/Integrator' */
  real_T Integrator_CSTATE_l;     /* '<S135>/Integrator' */
  real_T Integrator_CSTATE_hh[2]; /* '<S8>/Integrator' */
  real_T Integrator_CSTATE_a[2];  /* '<S184>/Integrator' */
  real_T Integrator_CSTATE_kh[2]; /* '<S58>/Integrator' */
  real_T Integrator_CSTATE_ca;    /* '<S10>/Integrator' */
  real_T Integrator_CSTATE_p;     /* '<S112>/Integrator' */
  real_T Integrator_CSTATE_px;    /* '<S111>/Integrator' */
  real_T Integrator_CSTATE_i[2];  /* '<S59>/Integrator' */
  real_T Integrator_CSTATE_f;     /* '<S61>/Integrator' */
  real_T Integrator_CSTATE_d0;    /* '<S120>/Integrator' */
  real_T Integrator_CSTATE_dx;    /* '<S119>/Integrator' */
  real_T Integrator_CSTATE_f3[2]; /* '<S85>/Integrator' */
  real_T Integrator_CSTATE_n3;    /* '<S87>/Integrator' */
  real_T Integrator_CSTATE_ba;    /* '<S186>/Integrator' */
};

/* State disabled  */
struct XDis_TX_DriveLine_ICE_T {
  boolean_T Integrator_CSTATE;       /* '<S43>/Integrator' */
  boolean_T Integrator_CSTATE_j;     /* '<S42>/Integrator' */
  boolean_T Integrator_CSTATE_c;     /* '<S35>/Integrator' */
  boolean_T Integrator_CSTATE_n;     /* '<S34>/Integrator' */
  boolean_T Integrator_CSTATE_d;     /* '<S211>/Integrator' */
  boolean_T Integrator_CSTATE_h;     /* '<S210>/Integrator' */
  boolean_T Integrator_CSTATE_k;     /* '<S219>/Integrator' */
  boolean_T Integrator_CSTATE_b;     /* '<S218>/Integrator' */
  boolean_T Integrator_CSTATE_c4;    /* '<S144>/Integrator' */
  boolean_T Integrator_CSTATE_c3;    /* '<S143>/Integrator' */
  boolean_T Integrator_CSTATE_nl;    /* '<S152>/Integrator' */
  boolean_T Integrator_CSTATE_g;     /* '<S151>/Integrator' */
  boolean_T Integrator_CSTATE_e;     /* '<S128>/Integrator' */
  boolean_T Integrator_CSTATE_ch;    /* '<S127>/Integrator' */
  boolean_T Integrator_CSTATE_jc;    /* '<S136>/Integrator' */
  boolean_T Integrator_CSTATE_l;     /* '<S135>/Integrator' */
  boolean_T Integrator_CSTATE_hh[2]; /* '<S8>/Integrator' */
  boolean_T Integrator_CSTATE_a[2];  /* '<S184>/Integrator' */
  boolean_T Integrator_CSTATE_kh[2]; /* '<S58>/Integrator' */
  boolean_T Integrator_CSTATE_ca;    /* '<S10>/Integrator' */
  boolean_T Integrator_CSTATE_p;     /* '<S112>/Integrator' */
  boolean_T Integrator_CSTATE_px;    /* '<S111>/Integrator' */
  boolean_T Integrator_CSTATE_i[2];  /* '<S59>/Integrator' */
  boolean_T Integrator_CSTATE_f;     /* '<S61>/Integrator' */
  boolean_T Integrator_CSTATE_d0;    /* '<S120>/Integrator' */
  boolean_T Integrator_CSTATE_dx;    /* '<S119>/Integrator' */
  boolean_T Integrator_CSTATE_f3[2]; /* '<S85>/Integrator' */
  boolean_T Integrator_CSTATE_n3;    /* '<S87>/Integrator' */
  boolean_T Integrator_CSTATE_ba;    /* '<S186>/Integrator' */
};

/* Zero-crossing (trigger) state */
struct PrevZCX_TX_DriveLine_ICE_T {
  ZCSigState Integrator_Reset_ZCE;    /* '<S43>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_h;  /* '<S35>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_p;  /* '<S211>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_d;  /* '<S219>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_e;  /* '<S144>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_de; /* '<S152>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_c;  /* '<S128>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_o;  /* '<S136>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_i;  /* '<S112>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_j;  /* '<S120>/Integrator' */
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
struct ExtU_TX_DriveLine_ICE_T {
  real_T DrvShfTrqNm;      /* '<Root>/DrvShfTrq[Nm]' */
  real_T OmegaAxlerads[4]; /* '<Root>/OmegaAxle[rad|s]' */
  real_T DiffPrsCmd[4];    /* '<Root>/DiffPrsCmd' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_TX_DriveLine_ICE_T {
  real_T AxlTrqNm[4];    /* '<Root>/AxlTrq[Nm]' */
  real_T PropShaftSpdNm; /* '<Root>/PropShaftSpd[Nm]' */
};

/* Parameters for system: '<S8>/Limited Slip Differential' */
struct P_LimitedSlipDifferential_TX_DriveLine_ICE_T_ {
  real_T LimitedSlipDifferential_shaftSwitchMask; /* Expression: shaftSwitchMask
                                                   * Referenced by: '<S8>/Limited Slip Differential'
                                                   */
};

/* Parameters (default storage) */
struct P_TX_DriveLine_ICE_T_ {
  struct_WPo6c0btEjgdkiBqVilJ2B VEH; /* Variable: VEH
                                      * Referenced by:
                                      *   '<S58>/Constant'
                                      *   '<S58>/Constant1'
                                      *   '<S8>/Constant'
                                      *   '<S8>/Constant1'
                                      *   '<S59>/Constant'
                                      *   '<S59>/Constant1'
                                      *   '<S85>/Constant'
                                      *   '<S85>/Constant1'
                                      *   '<S184>/Constant'
                                      *   '<S184>/Constant1'
                                      */
  real_T StatLdWhlR[4];              /* Variable: StatLdWhlR
                                      * Referenced by:
                                      *   '<S58>/Constant'
                                      *   '<S58>/Constant1'
                                      *   '<S8>/Constant'
                                      *   '<S8>/Constant1'
                                      *   '<S59>/Constant'
                                      *   '<S59>/Constant1'
                                      *   '<S85>/Constant'
                                      *   '<S85>/Constant1'
                                      *   '<S184>/Constant'
                                      *   '<S184>/Constant1'
                                      */
  real_T drive_type;                 /* Variable: drive_type
                                      * Referenced by: '<Root>/DriveType[1-FD;2-RD;3-4WD]'
                                      */
  real_T ratio_diff_front;           /* Variable: ratio_diff_front
                                      * Referenced by:
                                      *   '<S58>/Constant'
                                      *   '<S8>/Ndiff2'
                                      *   '<S59>/Ndiff2'
                                      *   '<S14>/Gain'
                                      *   '<S65>/Gain'
                                      *   '<S17>/Gain'
                                      *   '<S68>/Gain'
                                      */
  real_T ratio_diff_rear;            /* Variable: ratio_diff_rear
                                      * Referenced by:
                                      *   '<S58>/Constant1'
                                      *   '<S85>/Ndiff2'
                                      *   '<S184>/Ndiff2'
                                      *   '<S91>/Gain'
                                      *   '<S190>/Gain'
                                      *   '<S94>/Gain'
                                      *   '<S193>/Gain'
                                      */
  real_T ICEFrontLimitedSlipDifferential_Fc;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Fc
   * Referenced by: '<S22>/Constant2'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Fc;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Fc
   * Referenced by: '<S73>/Constant2'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Fc;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Fc
   * Referenced by: '<S99>/Constant2'
   */
  real_T ICERearLimitedSlipDifferential_Fc;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Fc
   * Referenced by: '<S198>/Constant2'
   */
  real_T ICEFrontLimitedSlipDifferential_Jd;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Jd
   * Referenced by: '<S8>/Jd'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Jd;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Jd
   * Referenced by: '<S59>/Jd'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Jd;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Jd
   * Referenced by: '<S85>/Jd'
   */
  real_T TransferCase_Jd; /* Mask Parameter: TransferCase_Jd
                           * Referenced by: '<S58>/Jd'
                           */
  real_T ICERearLimitedSlipDifferential_Jd;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Jd
   * Referenced by: '<S184>/Jd'
   */
  real_T ICEFrontLimitedSlipDifferential_Jw1;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Jw1
   * Referenced by: '<S8>/Jw1'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Jw1;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Jw1
   * Referenced by: '<S59>/Jw1'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Jw1;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Jw1
   * Referenced by: '<S85>/Jw1'
   */
  real_T TransferCase_Jw1; /* Mask Parameter: TransferCase_Jw1
                            * Referenced by: '<S58>/Jw1'
                            */
  real_T ICERearLimitedSlipDifferential_Jw1;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Jw1
   * Referenced by: '<S184>/Jw1'
   */
  real_T ICEFrontLimitedSlipDifferential_Jw2;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Jw2
   * Referenced by: '<S8>/Jw3'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Jw2;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Jw2
   * Referenced by: '<S59>/Jw3'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Jw2;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Jw2
   * Referenced by: '<S85>/Jw3'
   */
  real_T TransferCase_Jw2; /* Mask Parameter: TransferCase_Jw2
                            * Referenced by: '<S58>/Jw3'
                            */
  real_T ICERearLimitedSlipDifferential_Jw2;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Jw2
   * Referenced by: '<S184>/Jw3'
   */
  real_T TransferCase_Ndiff; /* Mask Parameter: TransferCase_Ndiff
                              * Referenced by:
                              *   '<S58>/Ndiff2'
                              *   '<S162>/Gain'
                              *   '<S165>/Gain1'
                              */
  real_T ICEFrontLimitedSlipDifferential_Ndisks;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Ndisks
   * Referenced by: '<S22>/Constant1'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Ndisks;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Ndisks
   * Referenced by: '<S73>/Constant1'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Ndisks;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Ndisks
   * Referenced by: '<S99>/Constant1'
   */
  real_T ICERearLimitedSlipDifferential_Ndisks;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Ndisks
   * Referenced by: '<S198>/Constant1'
   */
  real_T ICEFrontLimitedSlipDifferential_Reff;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Reff
   * Referenced by: '<S22>/Constant'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Reff;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Reff
   * Referenced by: '<S73>/Constant'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Reff;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Reff
   * Referenced by: '<S99>/Constant'
   */
  real_T ICERearLimitedSlipDifferential_Reff;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Reff
   * Referenced by: '<S198>/Constant'
   */
  real_T TransferCase_SpdLock; /* Mask Parameter: TransferCase_SpdLock
                                * Referenced by: '<S58>/SpdLockConstantConstant'
                                */
  real_T TransferCase_TrqSplitRatio;
  /* Mask Parameter: TransferCase_TrqSplitRatio
   * Referenced by: '<S58>/TrqSplitRatioConstantConstant'
   */
  real_T TorsionalCompliance2_b;   /* Mask Parameter: TorsionalCompliance2_b
                                    * Referenced by: '<S42>/Gain2'
                                    */
  real_T TorsionalCompliance1_b;   /* Mask Parameter: TorsionalCompliance1_b
                                    * Referenced by: '<S34>/Gain2'
                                    */
  real_T TorsionalCompliance_b;    /* Mask Parameter: TorsionalCompliance_b
                                    * Referenced by: '<S210>/Gain2'
                                    */
  real_T TorsionalCompliance1_b_b; /* Mask Parameter: TorsionalCompliance1_b_b
                                    * Referenced by: '<S218>/Gain2'
                                    */
  real_T TorsionalCompliance4_b;   /* Mask Parameter: TorsionalCompliance4_b
                                    * Referenced by: '<S143>/Gain2'
                                    */
  real_T TorsionalCompliance5_b;   /* Mask Parameter: TorsionalCompliance5_b
                                    * Referenced by: '<S151>/Gain2'
                                    */
  real_T TorsionalCompliance2_b_f; /* Mask Parameter: TorsionalCompliance2_b_f
                                    * Referenced by: '<S127>/Gain2'
                                    */
  real_T TorsionalCompliance3_b;   /* Mask Parameter: TorsionalCompliance3_b
                                    * Referenced by: '<S135>/Gain2'
                                    */
  real_T TorsionalCompliance_b_o;  /* Mask Parameter: TorsionalCompliance_b_o
                                    * Referenced by: '<S111>/Gain2'
                                    */
  real_T TorsionalCompliance1_b_j; /* Mask Parameter: TorsionalCompliance1_b_j
                                    * Referenced by: '<S119>/Gain2'
                                    */
  real_T ICEFrontLimitedSlipDifferential_bd;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_bd
   * Referenced by: '<S8>/bd'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_bd;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_bd
   * Referenced by: '<S59>/bd'
   */
  real_T uWD_ICERearLimitedSlipDifferential_bd;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_bd
   * Referenced by: '<S85>/bd'
   */
  real_T TransferCase_bd; /* Mask Parameter: TransferCase_bd
                           * Referenced by: '<S58>/bd'
                           */
  real_T ICERearLimitedSlipDifferential_bd;
  /* Mask Parameter: ICERearLimitedSlipDifferential_bd
   * Referenced by: '<S184>/bd'
   */
  real_T ICEFrontLimitedSlipDifferential_bw1;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_bw1
   * Referenced by: '<S8>/bw1'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_bw1;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_bw1
   * Referenced by: '<S59>/bw1'
   */
  real_T uWD_ICERearLimitedSlipDifferential_bw1;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_bw1
   * Referenced by: '<S85>/bw1'
   */
  real_T TransferCase_bw1; /* Mask Parameter: TransferCase_bw1
                            * Referenced by: '<S58>/bw1'
                            */
  real_T ICERearLimitedSlipDifferential_bw1;
  /* Mask Parameter: ICERearLimitedSlipDifferential_bw1
   * Referenced by: '<S184>/bw1'
   */
  real_T ICEFrontLimitedSlipDifferential_bw2;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_bw2
   * Referenced by: '<S8>/bw2'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_bw2;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_bw2
   * Referenced by: '<S59>/bw2'
   */
  real_T uWD_ICERearLimitedSlipDifferential_bw2;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_bw2
   * Referenced by: '<S85>/bw2'
   */
  real_T TransferCase_bw2; /* Mask Parameter: TransferCase_bw2
                            * Referenced by: '<S58>/bw2'
                            */
  real_T ICERearLimitedSlipDifferential_bw2;
  /* Mask Parameter: ICERearLimitedSlipDifferential_bw2
   * Referenced by: '<S184>/bw2'
   */
  real_T TorsionalCompliance2_domega_o;
  /* Mask Parameter: TorsionalCompliance2_domega_o
   * Referenced by: '<S42>/domega_o'
   */
  real_T TorsionalCompliance1_domega_o;
  /* Mask Parameter: TorsionalCompliance1_domega_o
   * Referenced by: '<S34>/domega_o'
   */
  real_T TorsionalCompliance_domega_o;
  /* Mask Parameter: TorsionalCompliance_domega_o
   * Referenced by: '<S210>/domega_o'
   */
  real_T TorsionalCompliance1_domega_o_f;
  /* Mask Parameter: TorsionalCompliance1_domega_o_f
   * Referenced by: '<S218>/domega_o'
   */
  real_T TorsionalCompliance4_domega_o;
  /* Mask Parameter: TorsionalCompliance4_domega_o
   * Referenced by: '<S143>/domega_o'
   */
  real_T TorsionalCompliance5_domega_o;
  /* Mask Parameter: TorsionalCompliance5_domega_o
   * Referenced by: '<S151>/domega_o'
   */
  real_T TorsionalCompliance2_domega_o_j;
  /* Mask Parameter: TorsionalCompliance2_domega_o_j
   * Referenced by: '<S127>/domega_o'
   */
  real_T TorsionalCompliance3_domega_o;
  /* Mask Parameter: TorsionalCompliance3_domega_o
   * Referenced by: '<S135>/domega_o'
   */
  real_T TorsionalCompliance_domega_o_m;
  /* Mask Parameter: TorsionalCompliance_domega_o_m
   * Referenced by: '<S111>/domega_o'
   */
  real_T TorsionalCompliance1_domega_o_p;
  /* Mask Parameter: TorsionalCompliance1_domega_o_p
   * Referenced by: '<S119>/domega_o'
   */
  real_T ICEFrontLimitedSlipDifferential_dw[8];
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_dw
   * Referenced by: '<S22>/mu Table'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_dw[7];
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_dw
   * Referenced by: '<S73>/mu Table'
   */
  real_T uWD_ICERearLimitedSlipDifferential_dw[7];
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_dw
   * Referenced by: '<S99>/mu Table'
   */
  real_T ICERearLimitedSlipDifferential_dw[8];
  /* Mask Parameter: ICERearLimitedSlipDifferential_dw
   * Referenced by: '<S198>/mu Table'
   */
  real_T ICEFrontLimitedSlipDifferential_eta;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_eta
   * Referenced by: '<S33>/Constant'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_eta;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_eta
   * Referenced by: '<S84>/Constant'
   */
  real_T uWD_ICERearLimitedSlipDifferential_eta;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_eta
   * Referenced by: '<S110>/Constant'
   */
  real_T TransferCase_eta; /* Mask Parameter: TransferCase_eta
                            * Referenced by: '<S180>/Constant'
                            */
  real_T ICERearLimitedSlipDifferential_eta;
  /* Mask Parameter: ICERearLimitedSlipDifferential_eta
   * Referenced by: '<S209>/Constant'
   */
  real_T TorsionalCompliance2_k;   /* Mask Parameter: TorsionalCompliance2_k
                                    * Referenced by: '<S42>/Gain1'
                                    */
  real_T TorsionalCompliance1_k;   /* Mask Parameter: TorsionalCompliance1_k
                                    * Referenced by: '<S34>/Gain1'
                                    */
  real_T TorsionalCompliance_k;    /* Mask Parameter: TorsionalCompliance_k
                                    * Referenced by: '<S210>/Gain1'
                                    */
  real_T TorsionalCompliance1_k_b; /* Mask Parameter: TorsionalCompliance1_k_b
                                    * Referenced by: '<S218>/Gain1'
                                    */
  real_T TorsionalCompliance4_k;   /* Mask Parameter: TorsionalCompliance4_k
                                    * Referenced by: '<S143>/Gain1'
                                    */
  real_T TorsionalCompliance5_k;   /* Mask Parameter: TorsionalCompliance5_k
                                    * Referenced by: '<S151>/Gain1'
                                    */
  real_T TorsionalCompliance2_k_i; /* Mask Parameter: TorsionalCompliance2_k_i
                                    * Referenced by: '<S127>/Gain1'
                                    */
  real_T TorsionalCompliance3_k;   /* Mask Parameter: TorsionalCompliance3_k
                                    * Referenced by: '<S135>/Gain1'
                                    */
  real_T TorsionalCompliance_k_h;  /* Mask Parameter: TorsionalCompliance_k_h
                                    * Referenced by: '<S111>/Gain1'
                                    */
  real_T TorsionalCompliance1_k_p; /* Mask Parameter: TorsionalCompliance1_k_p
                                    * Referenced by: '<S119>/Gain1'
                                    */
  real_T ICEFrontLimitedSlipDifferential_muc[8];
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_muc
   * Referenced by: '<S22>/mu Table'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_muc[7];
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_muc
   * Referenced by: '<S73>/mu Table'
   */
  real_T uWD_ICERearLimitedSlipDifferential_muc[7];
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_muc
   * Referenced by: '<S99>/mu Table'
   */
  real_T ICERearLimitedSlipDifferential_muc[8];
  /* Mask Parameter: ICERearLimitedSlipDifferential_muc
   * Referenced by: '<S198>/mu Table'
   */
  real_T TorsionalCompliance1_omega_c;
  /* Mask Parameter: TorsionalCompliance1_omega_c
   * Referenced by: '<S34>/omega_c'
   */
  real_T TorsionalCompliance2_omega_c;
  /* Mask Parameter: TorsionalCompliance2_omega_c
   * Referenced by: '<S42>/omega_c'
   */
  real_T TorsionalCompliance_omega_c;
  /* Mask Parameter: TorsionalCompliance_omega_c
   * Referenced by: '<S111>/omega_c'
   */
  real_T TorsionalCompliance1_omega_c_g;
  /* Mask Parameter: TorsionalCompliance1_omega_c_g
   * Referenced by: '<S119>/omega_c'
   */
  real_T TorsionalCompliance2_omega_c_i;
  /* Mask Parameter: TorsionalCompliance2_omega_c_i
   * Referenced by: '<S127>/omega_c'
   */
  real_T TorsionalCompliance3_omega_c;
  /* Mask Parameter: TorsionalCompliance3_omega_c
   * Referenced by: '<S135>/omega_c'
   */
  real_T TorsionalCompliance4_omega_c;
  /* Mask Parameter: TorsionalCompliance4_omega_c
   * Referenced by: '<S143>/omega_c'
   */
  real_T TorsionalCompliance5_omega_c;
  /* Mask Parameter: TorsionalCompliance5_omega_c
   * Referenced by: '<S151>/omega_c'
   */
  real_T TorsionalCompliance_omega_c_k;
  /* Mask Parameter: TorsionalCompliance_omega_c_k
   * Referenced by: '<S210>/omega_c'
   */
  real_T TorsionalCompliance1_omega_c_n;
  /* Mask Parameter: TorsionalCompliance1_omega_c_n
   * Referenced by: '<S218>/omega_c'
   */
  real_T TransferCase_shaftSwitchMask;
  /* Mask Parameter: TransferCase_shaftSwitchMask
   * Referenced by:
   *   '<S58>/TransferCase'
   *   '<S162>/Constant'
   *   '<S165>/Constant'
   */
  real_T ICEFrontLimitedSlipDifferential_tauC;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_tauC
   * Referenced by: '<S8>/Constant3'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_tauC;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_tauC
   * Referenced by: '<S59>/Constant3'
   */
  real_T uWD_ICERearLimitedSlipDifferential_tauC;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_tauC
   * Referenced by: '<S85>/Constant3'
   */
  real_T ICERearLimitedSlipDifferential_tauC;
  /* Mask Parameter: ICERearLimitedSlipDifferential_tauC
   * Referenced by: '<S184>/Constant3'
   */
  real_T TorsionalCompliance2_theta_o;
  /* Mask Parameter: TorsionalCompliance2_theta_o
   * Referenced by: '<S42>/Integrator'
   */
  real_T TorsionalCompliance1_theta_o;
  /* Mask Parameter: TorsionalCompliance1_theta_o
   * Referenced by: '<S34>/Integrator'
   */
  real_T TorsionalCompliance_theta_o;
  /* Mask Parameter: TorsionalCompliance_theta_o
   * Referenced by: '<S210>/Integrator'
   */
  real_T TorsionalCompliance1_theta_o_e;
  /* Mask Parameter: TorsionalCompliance1_theta_o_e
   * Referenced by: '<S218>/Integrator'
   */
  real_T TorsionalCompliance4_theta_o;
  /* Mask Parameter: TorsionalCompliance4_theta_o
   * Referenced by: '<S143>/Integrator'
   */
  real_T TorsionalCompliance5_theta_o;
  /* Mask Parameter: TorsionalCompliance5_theta_o
   * Referenced by: '<S151>/Integrator'
   */
  real_T TorsionalCompliance2_theta_o_j;
  /* Mask Parameter: TorsionalCompliance2_theta_o_j
   * Referenced by: '<S127>/Integrator'
   */
  real_T TorsionalCompliance3_theta_o;
  /* Mask Parameter: TorsionalCompliance3_theta_o
   * Referenced by: '<S135>/Integrator'
   */
  real_T TorsionalCompliance_theta_o_k;
  /* Mask Parameter: TorsionalCompliance_theta_o_k
   * Referenced by: '<S111>/Integrator'
   */
  real_T TorsionalCompliance1_theta_o_d;
  /* Mask Parameter: TorsionalCompliance1_theta_o_d
   * Referenced by: '<S119>/Integrator'
   */
  real_T Memory_InitialCondition;                                            /* Expression: 0
                                                                              * Referenced by: '<S43>/Memory'
                                                                              */
  real_T Memory_InitialCondition_n;                                          /* Expression: 0
                                                                              * Referenced by: '<S35>/Memory'
                                                                              */
  real_T Memory_InitialCondition_b;                                          /* Expression: 0
                                                                              * Referenced by: '<S211>/Memory'
                                                                              */
  real_T Memory_InitialCondition_k;                                          /* Expression: 0
                                                                              * Referenced by: '<S219>/Memory'
                                                                              */
  real_T Memory_InitialCondition_bc;                                         /* Expression: 0
                                                                              * Referenced by: '<S144>/Memory'
                                                                              */
  real_T Memory_InitialCondition_c;                                          /* Expression: 0
                                                                              * Referenced by: '<S152>/Memory'
                                                                              */
  real_T Memory_InitialCondition_h;                                          /* Expression: 0
                                                                              * Referenced by: '<S128>/Memory'
                                                                              */
  real_T Memory_InitialCondition_p;                                          /* Expression: 0
                                                                              * Referenced by: '<S136>/Memory'
                                                                              */
  real_T Constant1_Value;                                                    /* Expression: 1
                                                                              * Referenced by: '<S14>/Constant1'
                                                                              */
  real_T Constant_Value;                                                     /* Expression: shaftSwitchMask
                                                                              * Referenced by: '<S14>/Constant'
                                                                              */
  real_T Switch_Threshold;                                                   /* Expression: 1
                                                                              * Referenced by: '<S14>/Switch'
                                                                              */
  real_T Integrator_UpperSat;                                                /* Expression: maxAbsSpd
                                                                              * Referenced by: '<S8>/Integrator'
                                                                              */
  real_T Integrator_LowerSat;                                                /* Expression: -maxAbsSpd
                                                                              * Referenced by: '<S8>/Integrator'
                                                                              */
  real_T Constant1_Value_h;                                                  /* Expression: 1
                                                                              * Referenced by: '<S190>/Constant1'
                                                                              */
  real_T Constant_Value_e;                                                   /* Expression: shaftSwitchMask
                                                                              * Referenced by: '<S190>/Constant'
                                                                              */
  real_T Switch_Threshold_f;                                                 /* Expression: 1
                                                                              * Referenced by: '<S190>/Switch'
                                                                              */
  real_T Integrator_UpperSat_d;                                              /* Expression: maxAbsSpd
                                                                              * Referenced by: '<S184>/Integrator'
                                                                              */
  real_T Integrator_LowerSat_g;                                              /* Expression: -maxAbsSpd
                                                                              * Referenced by: '<S184>/Integrator'
                                                                              */
  real_T Constant1_Value_m;                                                  /* Expression: 1
                                                                              * Referenced by: '<S162>/Constant1'
                                                                              */
  real_T Switch_Threshold_l;                                                 /* Expression: 1
                                                                              * Referenced by: '<S162>/Switch'
                                                                              */
  real_T Integrator_UpperSat_a;                                              /* Expression: maxAbsSpd
                                                                              * Referenced by: '<S58>/Integrator'
                                                                              */
  real_T Integrator_LowerSat_e;                                              /* Expression: -maxAbsSpd
                                                                              * Referenced by: '<S58>/Integrator'
                                                                              */
  real_T Constant2_Value;                                                    /* Expression: 1
                                                                              * Referenced by: '<S162>/Constant2'
                                                                              */
  real_T upi_Gain;                                                           /* Expression: 2*pi
                                                                              * Referenced by: '<S8>/2*pi'
                                                                              */
  real_T Gain1_Gain;                                                         /* Expression: 1/2
                                                                              * Referenced by: '<S14>/Gain1'
                                                                              */
  real_T Constant_Value_d;                                                   /* Expression: 1
                                                                              * Referenced by: '<S25>/Constant'
                                                                              */
  real_T Switch_Threshold_d;                                                 /* Expression: 0
                                                                              * Referenced by: '<S25>/Switch'
                                                                              */
  real_T Gain1_Gain_e;                                                       /* Expression: 20
                                                                              * Referenced by: '<S32>/Gain1'
                                                                              */
  real_T Constant1_Value_b;                                                  /* Expression: 1
                                                                              * Referenced by: '<S32>/Constant1'
                                                                              */
  real_T Gain2_Gain;                                                         /* Expression: 4
                                                                              * Referenced by: '<S32>/Gain2'
                                                                              */
  real_T Gain_Gain;                                                          /* Expression: .5
                                                                              * Referenced by: '<S32>/Gain'
                                                                              */
  real_T Constant_Value_p;                                                   /* Expression: 1
                                                                              * Referenced by: '<S31>/Constant'
                                                                              */
  real_T Constant_Value_eq;                                                  /* Expression: 1
                                                                              * Referenced by: '<S23>/Constant'
                                                                              */
  real_T Switch_Threshold_b;                                                 /* Expression: 0
                                                                              * Referenced by: '<S23>/Switch'
                                                                              */
  real_T Gain1_Gain_h;                                                       /* Expression: 20
                                                                              * Referenced by: '<S28>/Gain1'
                                                                              */
  real_T Constant1_Value_a;                                                  /* Expression: 1
                                                                              * Referenced by: '<S28>/Constant1'
                                                                              */
  real_T Gain2_Gain_h;                                                       /* Expression: 4
                                                                              * Referenced by: '<S28>/Gain2'
                                                                              */
  real_T Gain_Gain_i;                                                        /* Expression: .5
                                                                              * Referenced by: '<S28>/Gain'
                                                                              */
  real_T Constant_Value_h;                                                   /* Expression: 1
                                                                              * Referenced by: '<S27>/Constant'
                                                                              */
  real_T Constant_Value_ps;                                                  /* Expression: 1
                                                                              * Referenced by: '<S24>/Constant'
                                                                              */
  real_T Switch_Threshold_h;                                                 /* Expression: 0
                                                                              * Referenced by: '<S24>/Switch'
                                                                              */
  real_T Gain1_Gain_m;                                                       /* Expression: 20
                                                                              * Referenced by: '<S30>/Gain1'
                                                                              */
  real_T Constant1_Value_p;                                                  /* Expression: 1
                                                                              * Referenced by: '<S30>/Constant1'
                                                                              */
  real_T Gain2_Gain_f;                                                       /* Expression: 4
                                                                              * Referenced by: '<S30>/Gain2'
                                                                              */
  real_T Gain_Gain_b;                                                        /* Expression: .5
                                                                              * Referenced by: '<S30>/Gain'
                                                                              */
  real_T Constant_Value_j;                                                   /* Expression: 1
                                                                              * Referenced by: '<S29>/Constant'
                                                                              */
  real_T Integrator_IC;                                                      /* Expression: 0
                                                                              * Referenced by: '<S10>/Integrator'
                                                                              */
  real_T Constant_Value_b;                                                   /* Expression: shaftSwitchMask
                                                                              * Referenced by: '<S17>/Constant'
                                                                              */
  real_T Constant2_Value_m;                                                  /* Expression: 1
                                                                              * Referenced by: '<S17>/Constant2'
                                                                              */
  real_T Switch_Threshold_p;                                                 /* Expression: 1
                                                                              * Referenced by: '<S17>/Switch'
                                                                              */
  real_T Gain_Gain_g;                                                        /* Expression: 4
                                                                              * Referenced by: '<S22>/Gain'
                                                                              */
  real_T Reset_Value;                                                        /* Expression: 1
                                                                              * Referenced by: '<S35>/Reset'
                                                                              */
  real_T Reset_Value_g;                                                      /* Expression: 1
                                                                              * Referenced by: '<S43>/Reset'
                                                                              */
  real_T upi_Gain_o;                                                         /* Expression: 2*pi
                                                                              * Referenced by: '<S59>/2*pi'
                                                                              */
  real_T Memory_InitialCondition_i;                                          /* Expression: 0
                                                                              * Referenced by: '<S112>/Memory'
                                                                              */
  real_T Constant1_Value_ms;                                                 /* Expression: 1
                                                                              * Referenced by: '<S65>/Constant1'
                                                                              */
  real_T Constant_Value_o;                                                   /* Expression: shaftSwitchMask
                                                                              * Referenced by: '<S65>/Constant'
                                                                              */
  real_T Switch_Threshold_a;                                                 /* Expression: 1
                                                                              * Referenced by: '<S65>/Switch'
                                                                              */
  real_T Integrator_UpperSat_j;                                              /* Expression: maxAbsSpd
                                                                              * Referenced by: '<S59>/Integrator'
                                                                              */
  real_T Integrator_LowerSat_a;                                              /* Expression: -maxAbsSpd
                                                                              * Referenced by: '<S59>/Integrator'
                                                                              */
  real_T Gain1_Gain_c;                                                       /* Expression: 1/2
                                                                              * Referenced by: '<S65>/Gain1'
                                                                              */
  real_T Constant_Value_l;                                                   /* Expression: 1
                                                                              * Referenced by: '<S76>/Constant'
                                                                              */
  real_T Switch_Threshold_fj;                                                /* Expression: 0
                                                                              * Referenced by: '<S76>/Switch'
                                                                              */
  real_T Gain1_Gain_k;                                                       /* Expression: 20
                                                                              * Referenced by: '<S83>/Gain1'
                                                                              */
  real_T Constant1_Value_be;                                                 /* Expression: 1
                                                                              * Referenced by: '<S83>/Constant1'
                                                                              */
  real_T Gain2_Gain_d;                                                       /* Expression: 4
                                                                              * Referenced by: '<S83>/Gain2'
                                                                              */
  real_T Gain_Gain_c;                                                        /* Expression: .5
                                                                              * Referenced by: '<S83>/Gain'
                                                                              */
  real_T Constant_Value_g;                                                   /* Expression: 1
                                                                              * Referenced by: '<S82>/Constant'
                                                                              */
  real_T Constant_Value_l2;                                                  /* Expression: 1
                                                                              * Referenced by: '<S74>/Constant'
                                                                              */
  real_T Switch_Threshold_fh;                                                /* Expression: 0
                                                                              * Referenced by: '<S74>/Switch'
                                                                              */
  real_T Gain1_Gain_g;                                                       /* Expression: 20
                                                                              * Referenced by: '<S79>/Gain1'
                                                                              */
  real_T Constant1_Value_j;                                                  /* Expression: 1
                                                                              * Referenced by: '<S79>/Constant1'
                                                                              */
  real_T Gain2_Gain_hy;                                                      /* Expression: 4
                                                                              * Referenced by: '<S79>/Gain2'
                                                                              */
  real_T Gain_Gain_j;                                                        /* Expression: .5
                                                                              * Referenced by: '<S79>/Gain'
                                                                              */
  real_T Constant_Value_hc;                                                  /* Expression: 1
                                                                              * Referenced by: '<S78>/Constant'
                                                                              */
  real_T Constant_Value_dv;                                                  /* Expression: 1
                                                                              * Referenced by: '<S75>/Constant'
                                                                              */
  real_T Switch_Threshold_au;                                                /* Expression: 0
                                                                              * Referenced by: '<S75>/Switch'
                                                                              */
  real_T Gain1_Gain_hg;                                                      /* Expression: 20
                                                                              * Referenced by: '<S81>/Gain1'
                                                                              */
  real_T Constant1_Value_o;                                                  /* Expression: 1
                                                                              * Referenced by: '<S81>/Constant1'
                                                                              */
  real_T Gain2_Gain_n;                                                       /* Expression: 4
                                                                              * Referenced by: '<S81>/Gain2'
                                                                              */
  real_T Gain_Gain_h;                                                        /* Expression: .5
                                                                              * Referenced by: '<S81>/Gain'
                                                                              */
  real_T Constant_Value_a;                                                   /* Expression: 1
                                                                              * Referenced by: '<S80>/Constant'
                                                                              */
  real_T Integrator_IC_p;                                                    /* Expression: 0
                                                                              * Referenced by: '<S61>/Integrator'
                                                                              */
  real_T Constant_Value_i;                                                   /* Expression: shaftSwitchMask
                                                                              * Referenced by: '<S68>/Constant'
                                                                              */
  real_T Constant2_Value_b;                                                  /* Expression: 1
                                                                              * Referenced by: '<S68>/Constant2'
                                                                              */
  real_T Switch_Threshold_e;                                                 /* Expression: 1
                                                                              * Referenced by: '<S68>/Switch'
                                                                              */
  real_T Gain_Gain_n;                                                        /* Expression: 4
                                                                              * Referenced by: '<S73>/Gain'
                                                                              */
  real_T upi_Gain_i;                                                         /* Expression: 2*pi
                                                                              * Referenced by: '<S85>/2*pi'
                                                                              */
  real_T Memory_InitialCondition_nn;                                         /* Expression: 0
                                                                              * Referenced by: '<S120>/Memory'
                                                                              */
  real_T Constant1_Value_b1;                                                 /* Expression: 1
                                                                              * Referenced by: '<S91>/Constant1'
                                                                              */
  real_T Constant_Value_psd;                                                 /* Expression: shaftSwitchMask
                                                                              * Referenced by: '<S91>/Constant'
                                                                              */
  real_T Switch_Threshold_db;                                                /* Expression: 1
                                                                              * Referenced by: '<S91>/Switch'
                                                                              */
  real_T Integrator_UpperSat_b;                                              /* Expression: maxAbsSpd
                                                                              * Referenced by: '<S85>/Integrator'
                                                                              */
  real_T Integrator_LowerSat_l;                                              /* Expression: -maxAbsSpd
                                                                              * Referenced by: '<S85>/Integrator'
                                                                              */
  real_T Gain1_Gain_k2;                                                      /* Expression: 1/2
                                                                              * Referenced by: '<S91>/Gain1'
                                                                              */
  real_T Constant_Value_o0;                                                  /* Expression: 1
                                                                              * Referenced by: '<S102>/Constant'
                                                                              */
  real_T Switch_Threshold_dw;                                                /* Expression: 0
                                                                              * Referenced by: '<S102>/Switch'
                                                                              */
  real_T Gain1_Gain_o;                                                       /* Expression: 20
                                                                              * Referenced by: '<S109>/Gain1'
                                                                              */
  real_T Constant1_Value_l;                                                  /* Expression: 1
                                                                              * Referenced by: '<S109>/Constant1'
                                                                              */
  real_T Gain2_Gain_l;                                                       /* Expression: 4
                                                                              * Referenced by: '<S109>/Gain2'
                                                                              */
  real_T Gain_Gain_ge;                                                       /* Expression: .5
                                                                              * Referenced by: '<S109>/Gain'
                                                                              */
  real_T Constant_Value_jf;                                                  /* Expression: 1
                                                                              * Referenced by: '<S108>/Constant'
                                                                              */
  real_T Constant_Value_pz;                                                  /* Expression: 1
                                                                              * Referenced by: '<S100>/Constant'
                                                                              */
  real_T Switch_Threshold_d2;                                                /* Expression: 0
                                                                              * Referenced by: '<S100>/Switch'
                                                                              */
  real_T Gain1_Gain_l;                                                       /* Expression: 20
                                                                              * Referenced by: '<S105>/Gain1'
                                                                              */
  real_T Constant1_Value_n;                                                  /* Expression: 1
                                                                              * Referenced by: '<S105>/Constant1'
                                                                              */
  real_T Gain2_Gain_g;                                                       /* Expression: 4
                                                                              * Referenced by: '<S105>/Gain2'
                                                                              */
  real_T Gain_Gain_p;                                                        /* Expression: .5
                                                                              * Referenced by: '<S105>/Gain'
                                                                              */
  real_T Constant_Value_f;                                                   /* Expression: 1
                                                                              * Referenced by: '<S104>/Constant'
                                                                              */
  real_T Constant_Value_bt;                                                  /* Expression: 1
                                                                              * Referenced by: '<S101>/Constant'
                                                                              */
  real_T Switch_Threshold_ez;                                                /* Expression: 0
                                                                              * Referenced by: '<S101>/Switch'
                                                                              */
  real_T Gain1_Gain_hj;                                                      /* Expression: 20
                                                                              * Referenced by: '<S107>/Gain1'
                                                                              */
  real_T Constant1_Value_o3;                                                 /* Expression: 1
                                                                              * Referenced by: '<S107>/Constant1'
                                                                              */
  real_T Gain2_Gain_m;                                                       /* Expression: 4
                                                                              * Referenced by: '<S107>/Gain2'
                                                                              */
  real_T Gain_Gain_e;                                                        /* Expression: .5
                                                                              * Referenced by: '<S107>/Gain'
                                                                              */
  real_T Constant_Value_pp;                                                  /* Expression: 1
                                                                              * Referenced by: '<S106>/Constant'
                                                                              */
  real_T Integrator_IC_pr;                                                   /* Expression: 0
                                                                              * Referenced by: '<S87>/Integrator'
                                                                              */
  real_T Constant_Value_op;                                                  /* Expression: shaftSwitchMask
                                                                              * Referenced by: '<S94>/Constant'
                                                                              */
  real_T Constant2_Value_h;                                                  /* Expression: 1
                                                                              * Referenced by: '<S94>/Constant2'
                                                                              */
  real_T Switch_Threshold_f5;                                                /* Expression: 1
                                                                              * Referenced by: '<S94>/Switch'
                                                                              */
  real_T Gain_Gain_k;                                                        /* Expression: 4
                                                                              * Referenced by: '<S99>/Gain'
                                                                              */
  real_T Reset_Value_n;                                                      /* Expression: 1
                                                                              * Referenced by: '<S112>/Reset'
                                                                              */
  real_T Reset_Value_j;                                                      /* Expression: 1
                                                                              * Referenced by: '<S120>/Reset'
                                                                              */
  real_T Reset_Value_l;                                                      /* Expression: 1
                                                                              * Referenced by: '<S128>/Reset'
                                                                              */
  real_T Reset_Value_jc;                                                     /* Expression: 1
                                                                              * Referenced by: '<S136>/Reset'
                                                                              */
  real_T Reset_Value_c;                                                      /* Expression: 1
                                                                              * Referenced by: '<S144>/Reset'
                                                                              */
  real_T Reset_Value_jd;                                                     /* Expression: 1
                                                                              * Referenced by: '<S152>/Reset'
                                                                              */
  real_T Gain1_Gain_g5;                                                      /* Expression: 1/2
                                                                              * Referenced by: '<S162>/Gain1'
                                                                              */
  real_T Constant_Value_dr;                                                  /* Expression: 1
                                                                              * Referenced by: '<S172>/Constant'
                                                                              */
  real_T Switch_Threshold_j;                                                 /* Expression: 0
                                                                              * Referenced by: '<S172>/Switch'
                                                                              */
  real_T Gain1_Gain_gv;                                                      /* Expression: 20
                                                                              * Referenced by: '<S179>/Gain1'
                                                                              */
  real_T Constant1_Value_d;                                                  /* Expression: 1
                                                                              * Referenced by: '<S179>/Constant1'
                                                                              */
  real_T Gain2_Gain_h0;                                                      /* Expression: 4
                                                                              * Referenced by: '<S179>/Gain2'
                                                                              */
  real_T Gain_Gain_bj;                                                       /* Expression: .5
                                                                              * Referenced by: '<S179>/Gain'
                                                                              */
  real_T Constant_Value_gv;                                                  /* Expression: 1
                                                                              * Referenced by: '<S178>/Constant'
                                                                              */
  real_T Constant_Value_m;                                                   /* Expression: 1
                                                                              * Referenced by: '<S170>/Constant'
                                                                              */
  real_T Switch_Threshold_bl;                                                /* Expression: 0
                                                                              * Referenced by: '<S170>/Switch'
                                                                              */
  real_T Gain1_Gain_b;                                                       /* Expression: 20
                                                                              * Referenced by: '<S175>/Gain1'
                                                                              */
  real_T Constant1_Value_f;                                                  /* Expression: 1
                                                                              * Referenced by: '<S175>/Constant1'
                                                                              */
  real_T Gain2_Gain_c;                                                       /* Expression: 4
                                                                              * Referenced by: '<S175>/Gain2'
                                                                              */
  real_T Gain_Gain_nw;                                                       /* Expression: .5
                                                                              * Referenced by: '<S175>/Gain'
                                                                              */
  real_T Constant_Value_fb;                                                  /* Expression: 1
                                                                              * Referenced by: '<S174>/Constant'
                                                                              */
  real_T Constant_Value_jc;                                                  /* Expression: 1
                                                                              * Referenced by: '<S171>/Constant'
                                                                              */
  real_T Switch_Threshold_pr;                                                /* Expression: 0
                                                                              * Referenced by: '<S171>/Switch'
                                                                              */
  real_T Gain1_Gain_kz;                                                      /* Expression: 20
                                                                              * Referenced by: '<S177>/Gain1'
                                                                              */
  real_T Constant1_Value_bz;                                                 /* Expression: 1
                                                                              * Referenced by: '<S177>/Constant1'
                                                                              */
  real_T Gain2_Gain_fr;                                                      /* Expression: 4
                                                                              * Referenced by: '<S177>/Gain2'
                                                                              */
  real_T Gain_Gain_jk;                                                       /* Expression: .5
                                                                              * Referenced by: '<S177>/Gain'
                                                                              */
  real_T Constant_Value_pa;                                                  /* Expression: 1
                                                                              * Referenced by: '<S176>/Constant'
                                                                              */
  real_T Constant6_Value;                                                    /* Expression: 1
                                                                              * Referenced by: '<S165>/Constant6'
                                                                              */
  real_T Switch1_Threshold;                                                  /* Expression: 1
                                                                              * Referenced by: '<S165>/Switch1'
                                                                              */
  real_T upi_Gain_h;                                                         /* Expression: 2*pi
                                                                              * Referenced by: '<S184>/2*pi'
                                                                              */
  real_T Gain1_Gain_cp;                                                      /* Expression: 1/2
                                                                              * Referenced by: '<S190>/Gain1'
                                                                              */
  real_T Constant_Value_b3;                                                  /* Expression: 1
                                                                              * Referenced by: '<S201>/Constant'
                                                                              */
  real_T Switch_Threshold_af;                                                /* Expression: 0
                                                                              * Referenced by: '<S201>/Switch'
                                                                              */
  real_T Gain1_Gain_mc;                                                      /* Expression: 20
                                                                              * Referenced by: '<S208>/Gain1'
                                                                              */
  real_T Constant1_Value_p5;                                                 /* Expression: 1
                                                                              * Referenced by: '<S208>/Constant1'
                                                                              */
  real_T Gain2_Gain_gc;                                                      /* Expression: 4
                                                                              * Referenced by: '<S208>/Gain2'
                                                                              */
  real_T Gain_Gain_ka;                                                       /* Expression: .5
                                                                              * Referenced by: '<S208>/Gain'
                                                                              */
  real_T Constant_Value_ea;                                                  /* Expression: 1
                                                                              * Referenced by: '<S207>/Constant'
                                                                              */
  real_T Constant_Value_f1;                                                  /* Expression: 1
                                                                              * Referenced by: '<S199>/Constant'
                                                                              */
  real_T Switch_Threshold_g;                                                 /* Expression: 0
                                                                              * Referenced by: '<S199>/Switch'
                                                                              */
  real_T Gain1_Gain_f;                                                       /* Expression: 20
                                                                              * Referenced by: '<S204>/Gain1'
                                                                              */
  real_T Constant1_Value_pt;                                                 /* Expression: 1
                                                                              * Referenced by: '<S204>/Constant1'
                                                                              */
  real_T Gain2_Gain_f1;                                                      /* Expression: 4
                                                                              * Referenced by: '<S204>/Gain2'
                                                                              */
  real_T Gain_Gain_jx;                                                       /* Expression: .5
                                                                              * Referenced by: '<S204>/Gain'
                                                                              */
  real_T Constant_Value_lw;                                                  /* Expression: 1
                                                                              * Referenced by: '<S203>/Constant'
                                                                              */
  real_T Constant_Value_c;                                                   /* Expression: 1
                                                                              * Referenced by: '<S200>/Constant'
                                                                              */
  real_T Switch_Threshold_lz;                                                /* Expression: 0
                                                                              * Referenced by: '<S200>/Switch'
                                                                              */
  real_T Gain1_Gain_ol;                                                      /* Expression: 20
                                                                              * Referenced by: '<S206>/Gain1'
                                                                              */
  real_T Constant1_Value_hv;                                                 /* Expression: 1
                                                                              * Referenced by: '<S206>/Constant1'
                                                                              */
  real_T Gain2_Gain_fs;                                                      /* Expression: 4
                                                                              * Referenced by: '<S206>/Gain2'
                                                                              */
  real_T Gain_Gain_pj;                                                       /* Expression: .5
                                                                              * Referenced by: '<S206>/Gain'
                                                                              */
  real_T Constant_Value_eg;                                                  /* Expression: 1
                                                                              * Referenced by: '<S205>/Constant'
                                                                              */
  real_T Integrator_IC_c;                                                    /* Expression: 0
                                                                              * Referenced by: '<S186>/Integrator'
                                                                              */
  real_T Constant_Value_n;                                                   /* Expression: shaftSwitchMask
                                                                              * Referenced by: '<S193>/Constant'
                                                                              */
  real_T Constant2_Value_j;                                                  /* Expression: 1
                                                                              * Referenced by: '<S193>/Constant2'
                                                                              */
  real_T Switch_Threshold_di;                                                /* Expression: 1
                                                                              * Referenced by: '<S193>/Switch'
                                                                              */
  real_T Gain_Gain_m;                                                        /* Expression: 4
                                                                              * Referenced by: '<S198>/Gain'
                                                                              */
  real_T Reset_Value_ck;                                                     /* Expression: 1
                                                                              * Referenced by: '<S211>/Reset'
                                                                              */
  real_T Reset_Value_j5;                                                     /* Expression: 1
                                                                              * Referenced by: '<S219>/Reset'
                                                                              */
  P_LimitedSlipDifferential_TX_DriveLine_ICE_T sf_LimitedSlipDifferential_c; /* '<S184>/Limited Slip Differential' */
  P_LimitedSlipDifferential_TX_DriveLine_ICE_T sf_LimitedSlipDifferential_d; /* '<S85>/Limited Slip Differential' */
  P_LimitedSlipDifferential_TX_DriveLine_ICE_T sf_LimitedSlipDifferential_i; /* '<S59>/Limited Slip Differential' */
  P_LimitedSlipDifferential_TX_DriveLine_ICE_T sf_LimitedSlipDifferential;   /* '<S8>/Limited Slip Differential' */
};

/* Real-time Model Data Structure */
struct tag_RTM_TX_DriveLine_ICE_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_TX_DriveLine_ICE_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_TX_DriveLine_ICE_T *contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[34];
  real_T odeF[4][34];
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

/* Class declaration for model TX_DriveLine_ICE */
class MODULE_API TX_DriveLine_ICE {
  /* public data and function members */
 public:
  /* Copy Constructor */
  TX_DriveLine_ICE(TX_DriveLine_ICE const &) = delete;

  /* Assignment Operator */
  TX_DriveLine_ICE &operator=(TX_DriveLine_ICE const &) & = delete;

  /* Move Constructor */
  TX_DriveLine_ICE(TX_DriveLine_ICE &&) = delete;

  /* Move Assignment Operator */
  TX_DriveLine_ICE &operator=(TX_DriveLine_ICE &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_TX_DriveLine_ICE_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_TX_DriveLine_ICE_T *pExtU_TX_DriveLine_ICE_T) {
    TX_DriveLine_ICE_U = *pExtU_TX_DriveLine_ICE_T;
  }

  /* Root outports get method */
  const ExtY_TX_DriveLine_ICE_T &getExternalOutputs() const { return TX_DriveLine_ICE_Y; }

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
  TX_DriveLine_ICE();

  /* Destructor */
  ~TX_DriveLine_ICE();

 protected:
  /* Tunable parameters */
  static P_TX_DriveLine_ICE_T TX_DriveLine_ICE_P;

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_TX_DriveLine_ICE_T TX_DriveLine_ICE_U;

  /* External outputs */
  ExtY_TX_DriveLine_ICE_T TX_DriveLine_ICE_Y;

  /* Block signals */
  B_TX_DriveLine_ICE_T TX_DriveLine_ICE_B;

  /* Block states */
  DW_TX_DriveLine_ICE_T TX_DriveLine_ICE_DW;

  /* Tunable parameters */
  // static P_TX_DriveLine_ICE_T TX_DriveLine_ICE_P;

  /* Block continuous states */
  X_TX_DriveLine_ICE_T TX_DriveLine_ICE_X;

  /* Triggered events */
  PrevZCX_TX_DriveLine_ICE_T TX_DriveLine_ICE_PrevZCX;

  /* private member function(s) for subsystem '<S8>/Limited Slip Differential'*/
  void TX_DriveLine_ICE_LimitedSlipDifferential(real_T rtu_u, real_T rtu_u_j, real_T rtu_u_m, real_T rtu_u_n,
                                                real_T rtu_bw1, real_T rtu_bd, real_T rtu_bw2, real_T rtu_Ndiff,
                                                real_T rtu_Jd, real_T rtu_Jw1, real_T rtu_Jw2, const real_T rtu_x[2],
                                                B_LimitedSlipDifferential_TX_DriveLine_ICE_T *localB,
                                                P_LimitedSlipDifferential_TX_DriveLine_ICE_T *localP);
  void TX_DriveLine_ICE_automldiffls(const real_T u[4], real_T bw1, real_T bd, real_T bw2, real_T Ndiff,
                                     real_T shaftSwitch, real_T Jd, real_T Jw1, real_T Jw2, const real_T x[2],
                                     real_T y[4], real_T xdot[2]);

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void TX_DriveLine_ICE_derivatives();

  /* Real-Time Model */
  RT_MODEL_TX_DriveLine_ICE_T TX_DriveLine_ICE_M;
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
 * '<Root>' : 'TX_DriveLine_ICE'
 * '<S1>'   : 'TX_DriveLine_ICE/ICE_DriveLine'
 * '<S2>'   : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive'
 * '<S3>'   : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive'
 * '<S4>'   : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive'
 * '<S5>'   : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential'
 * '<S6>'   : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance1'
 * '<S7>'   : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance2'
 * '<S8>'   : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential'
 * '<S9>'   : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation'
 * '<S10>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Clutch Response'
 * '<S11>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Coupling Torque'
 * '<S12>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency'
 * '<S13>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Limited Slip Differential'
 * '<S14>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/y'
 * '<S15>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power'
 * '<S16>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Damping Power'
 * '<S17>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Kinetic Power'
 * '<S18>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator'
 * '<S19>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S20>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S21>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S22>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Coupling Torque/Ideal Wet Clutch Coupling'
 * '<S23>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency'
 * '<S24>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency'
 * '<S25>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency'
 * '<S26>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Eta'
 * '<S27>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency/Low speed blend'
 * '<S28>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency/Low speed blend/blend'
 * '<S29>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency/Low speed blend'
 * '<S30>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency/Low speed blend/blend'
 * '<S31>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency/Low speed blend'
 * '<S32>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency/Low speed blend/blend'
 * '<S33>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Eta/Constant Eta'
 * '<S34>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance Linear'
 * '<S35>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance Linear/Cont
 * LPF IC Dyn'
 * '<S36>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance Linear/Power'
 * '<S37>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S38>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S39>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S40>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S41>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S42>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance Linear'
 * '<S43>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance Linear/Cont
 * LPF IC Dyn'
 * '<S44>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance Linear/Power'
 * '<S45>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S46>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S47>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S48>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S49>'  : 'TX_DriveLine_ICE/ICE_DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S50>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential'
 * '<S51>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential'
 * '<S52>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance'
 * '<S53>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance1'
 * '<S54>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance2'
 * '<S55>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance3'
 * '<S56>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance4'
 * '<S57>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance5'
 * '<S58>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case'
 * '<S59>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential'
 * '<S60>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation'
 * '<S61>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Clutch Response'
 * '<S62>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Coupling Torque'
 * '<S63>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency'
 * '<S64>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Limited Slip Differential'
 * '<S65>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/y'
 * '<S66>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power'
 * '<S67>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Damping Power'
 * '<S68>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Kinetic Power'
 * '<S69>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator'
 * '<S70>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S71>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S72>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S73>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Coupling Torque/Ideal Wet Clutch Coupling'
 * '<S74>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency'
 * '<S75>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency'
 * '<S76>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency'
 * '<S77>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Eta'
 * '<S78>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency/Low speed blend'
 * '<S79>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency/Low speed blend/blend'
 * '<S80>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency/Low speed blend'
 * '<S81>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency/Low speed blend/blend'
 * '<S82>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency/Low speed blend'
 * '<S83>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency/Low speed blend/blend'
 * '<S84>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Eta/Constant Eta'
 * '<S85>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential'
 * '<S86>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation'
 * '<S87>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Clutch Response'
 * '<S88>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Coupling Torque'
 * '<S89>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency'
 * '<S90>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Limited Slip Differential'
 * '<S91>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/y'
 * '<S92>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power'
 * '<S93>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Damping Power'
 * '<S94>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Kinetic Power'
 * '<S95>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator'
 * '<S96>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S97>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S98>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S99>'  : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Coupling Torque/Ideal Wet Clutch Coupling'
 * '<S100>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency'
 * '<S101>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency'
 * '<S102>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency'
 * '<S103>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Eta'
 * '<S104>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency/Low speed blend'
 * '<S105>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency/Low speed blend/blend'
 * '<S106>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency/Low speed blend'
 * '<S107>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency/Low speed blend/blend'
 * '<S108>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency/Low speed blend'
 * '<S109>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency/Low speed blend/blend'
 * '<S110>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Eta/Constant Eta'
 * '<S111>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional Compliance Linear'
 * '<S112>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional Compliance Linear/Cont
 * LPF IC Dyn'
 * '<S113>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power'
 * '<S114>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S115>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S116>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S117>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S118>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S119>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional Compliance Linear'
 * '<S120>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional Compliance Linear/Cont
 * LPF IC Dyn'
 * '<S121>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power'
 * '<S122>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S123>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S124>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S125>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S126>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S127>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional Compliance Linear'
 * '<S128>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional Compliance Linear/Cont
 * LPF IC Dyn'
 * '<S129>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power'
 * '<S130>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S131>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S132>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S133>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S134>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S135>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional Compliance Linear'
 * '<S136>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional Compliance Linear/Cont
 * LPF IC Dyn'
 * '<S137>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional Compliance
 * Linear/Power'
 * '<S138>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S139>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S140>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S141>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S142>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S143>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional Compliance Linear'
 * '<S144>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional Compliance Linear/Cont
 * LPF IC Dyn'
 * '<S145>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional Compliance
 * Linear/Power'
 * '<S146>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S147>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S148>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S149>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S150>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S151>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional Compliance Linear'
 * '<S152>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional Compliance Linear/Cont
 * LPF IC Dyn'
 * '<S153>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional Compliance
 * Linear/Power'
 * '<S154>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S155>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S156>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S157>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S158>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S159>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation'
 * '<S160>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency'
 * '<S161>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/TransferCase'
 * '<S162>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/y'
 * '<S163>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power'
 * '<S164>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power/Damping Power'
 * '<S165>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power/Kinetic Power'
 * '<S166>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power/Power Accounting Bus
 * Creator'
 * '<S167>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrNotTrnsfrd Input'
 * '<S168>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrStored Input'
 * '<S169>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrTrnsfrd Input'
 * '<S170>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 1 Efficiency'
 * '<S171>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 2 Efficiency'
 * '<S172>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Drive Efficiency'
 * '<S173>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Eta'
 * '<S174>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 1 Efficiency/Low speed
 * blend'
 * '<S175>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 1 Efficiency/Low speed
 * blend/blend'
 * '<S176>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 2 Efficiency/Low speed
 * blend'
 * '<S177>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 2 Efficiency/Low speed
 * blend/blend'
 * '<S178>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Drive Efficiency/Low speed
 * blend'
 * '<S179>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Drive Efficiency/Low speed
 * blend/blend'
 * '<S180>' : 'TX_DriveLine_ICE/ICE_DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Eta/Constant Eta'
 * '<S181>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential'
 * '<S182>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance'
 * '<S183>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance1'
 * '<S184>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip Differential'
 * '<S185>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation'
 * '<S186>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Clutch Response'
 * '<S187>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Coupling Torque'
 * '<S188>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency'
 * '<S189>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Limited Slip Differential'
 * '<S190>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/y'
 * '<S191>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power'
 * '<S192>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Damping Power'
 * '<S193>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Kinetic Power'
 * '<S194>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator'
 * '<S195>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S196>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S197>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S198>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Coupling Torque/Ideal Wet Clutch Coupling'
 * '<S199>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency'
 * '<S200>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency'
 * '<S201>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency'
 * '<S202>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Eta'
 * '<S203>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency/Low speed blend'
 * '<S204>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency/Low speed blend/blend'
 * '<S205>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency/Low speed blend'
 * '<S206>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency/Low speed blend/blend'
 * '<S207>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency/Low speed blend'
 * '<S208>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency/Low speed blend/blend'
 * '<S209>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Eta/Constant Eta'
 * '<S210>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance Linear'
 * '<S211>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance Linear/Cont LPF
 * IC Dyn'
 * '<S212>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance Linear/Power'
 * '<S213>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S214>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S215>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S216>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S217>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S218>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance Linear'
 * '<S219>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance Linear/Cont
 * LPF IC Dyn'
 * '<S220>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance Linear/Power'
 * '<S221>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S222>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S223>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S224>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S225>' : 'TX_DriveLine_ICE/ICE_DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 */
#endif /* RTW_HEADER_TX_DriveLine_ICE_h_ */
