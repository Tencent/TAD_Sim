/*
 * TX_DriveLine.h
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

#ifndef RTW_HEADER_TX_DriveLine_h_
#define RTW_HEADER_TX_DriveLine_h_
#include "TX_DriveLine_types.h"
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

/* Block signals for system '<S108>/Limited Slip Differential' */
struct B_LimitedSlipDifferential_TX_DriveLine_T {
  real_T xdot[2]; /* '<S108>/Limited Slip Differential' */
};

/* Block signals (default storage) */
struct B_TX_DriveLine_T {
  driveline_out Merge;                                                   /* '<S1>/Merge' */
  real_T Memory;                                                         /* '<S45>/Memory' */
  real_T domega_o;                                                       /* '<S44>/domega_o' */
  real_T Memory_n;                                                       /* '<S37>/Memory' */
  real_T domega_o_h;                                                     /* '<S36>/domega_o' */
  real_T Memory_p;                                                       /* '<S82>/Memory' */
  real_T domega_o_o;                                                     /* '<S81>/domega_o' */
  real_T Memory_nc;                                                      /* '<S90>/Memory' */
  real_T domega_o_f;                                                     /* '<S89>/domega_o' */
  real_T VectorConcatenate[2];                                           /* '<S10>/Vector Concatenate' */
  real_T VectorConcatenate_i[2];                                         /* '<S55>/Vector Concatenate' */
  real_T Product;                                                        /* '<S12>/Product' */
  real_T Subtract;                                                       /* '<S36>/Subtract' */
  real_T Product_i;                                                      /* '<S37>/Product' */
  real_T Subtract_o;                                                     /* '<S44>/Subtract' */
  real_T Product_g;                                                      /* '<S45>/Product' */
  real_T Product_o;                                                      /* '<S57>/Product' */
  real_T Subtract_h;                                                     /* '<S81>/Subtract' */
  real_T Product_h;                                                      /* '<S82>/Product' */
  real_T Subtract_e;                                                     /* '<S89>/Subtract' */
  real_T Product_j;                                                      /* '<S90>/Product' */
  real_T Memory_l;                                                       /* '<S333>/Memory' */
  real_T Constant1;                                                      /* '<S331>/Constant1' */
  real_T DeadZone;                                                       /* '<S331>/Dead Zone' */
  real_T Memory_e;                                                       /* '<S370>/Memory' */
  real_T domega_o_g;                                                     /* '<S369>/domega_o' */
  real_T IC;                                                             /* '<S338>/IC' */
  real_T Switch;                                                         /* '<S338>/Switch' */
  real_T IC_a;                                                           /* '<S336>/IC' */
  real_T Switch_j;                                                       /* '<S336>/Switch' */
  real_T IC_m;                                                           /* '<S334>/IC' */
  real_T Switch_d;                                                       /* '<S334>/Switch' */
  real_T Memory_nv;                                                      /* '<S143>/Memory' */
  real_T domega_o_a;                                                     /* '<S142>/domega_o' */
  real_T Memory_nn;                                                      /* '<S135>/Memory' */
  real_T domega_o_i;                                                     /* '<S134>/domega_o' */
  real_T Memory_f;                                                       /* '<S311>/Memory' */
  real_T domega_o_k;                                                     /* '<S310>/domega_o' */
  real_T Memory_g;                                                       /* '<S319>/Memory' */
  real_T domega_o_kv;                                                    /* '<S318>/domega_o' */
  real_T Memory_c;                                                       /* '<S244>/Memory' */
  real_T domega_o_a2;                                                    /* '<S243>/domega_o' */
  real_T Memory_pc;                                                      /* '<S252>/Memory' */
  real_T domega_o_hw;                                                    /* '<S251>/domega_o' */
  real_T Memory_ly;                                                      /* '<S228>/Memory' */
  real_T domega_o_b;                                                     /* '<S227>/domega_o' */
  real_T Memory_pz;                                                      /* '<S236>/Memory' */
  real_T domega_o_c;                                                     /* '<S235>/domega_o' */
  real_T VectorConcatenate_l[2];                                         /* '<S108>/Vector Concatenate' */
  real_T VectorConcatenate_c[2];                                         /* '<S284>/Vector Concatenate' */
  real_T VectorConcatenate_lo[2];                                        /* '<S158>/Vector Concatenate' */
  real_T Product_p;                                                      /* '<S110>/Product' */
  real_T Subtract_a;                                                     /* '<S134>/Subtract' */
  real_T Product_e;                                                      /* '<S135>/Product' */
  real_T Subtract_k;                                                     /* '<S142>/Subtract' */
  real_T Product_b;                                                      /* '<S143>/Product' */
  real_T Memory_j;                                                       /* '<S212>/Memory' */
  real_T domega_o_az;                                                    /* '<S211>/domega_o' */
  real_T VectorConcatenate_e[2];                                         /* '<S159>/Vector Concatenate' */
  real_T Product_c;                                                      /* '<S161>/Product' */
  real_T Memory_la;                                                      /* '<S220>/Memory' */
  real_T domega_o_n;                                                     /* '<S219>/domega_o' */
  real_T VectorConcatenate_f[2];                                         /* '<S185>/Vector Concatenate' */
  real_T Product_f;                                                      /* '<S187>/Product' */
  real_T Subtract_l;                                                     /* '<S211>/Subtract' */
  real_T Product_a;                                                      /* '<S212>/Product' */
  real_T Subtract_f;                                                     /* '<S219>/Subtract' */
  real_T Product_iv;                                                     /* '<S220>/Product' */
  real_T Subtract_n;                                                     /* '<S227>/Subtract' */
  real_T Product_e3;                                                     /* '<S228>/Product' */
  real_T Subtract_lk;                                                    /* '<S235>/Subtract' */
  real_T Product_pv;                                                     /* '<S236>/Product' */
  real_T Subtract_j;                                                     /* '<S243>/Subtract' */
  real_T Product_pr;                                                     /* '<S244>/Product' */
  real_T Subtract_d;                                                     /* '<S251>/Subtract' */
  real_T Product_l;                                                      /* '<S252>/Product' */
  real_T Product_h1;                                                     /* '<S286>/Product' */
  real_T Subtract_hz;                                                    /* '<S310>/Subtract' */
  real_T Product_lx;                                                     /* '<S311>/Product' */
  real_T Subtract_p;                                                     /* '<S318>/Subtract' */
  real_T Product_af;                                                     /* '<S319>/Product' */
  real_T Integrator1;                                                    /* '<S382>/Integrator1' */
  real_T Product_cs;                                                     /* '<S333>/Product' */
  real_T Subtract_dt;                                                    /* '<S369>/Subtract' */
  real_T Product_jf;                                                     /* '<S370>/Product' */
  real_T Divide;                                                         /* '<S382>/Divide' */
  real_T Product4;                                                       /* '<S337>/Product4' */
  real_T Product8;                                                       /* '<S337>/Product8' */
  real_T Product8_i;                                                     /* '<S335>/Product8' */
  real_T xdot[2];                                                        /* '<S158>/TransferCase' */
  int32_T DataTypeConversion;                                            /* '<S98>/Data Type Conversion' */
  int32_T Uk1;                                                           /* '<S326>/Delay Input1' */
  int8_T GearCmd;                                                        /* '<S377>/TCM Shift Controller' */
  int8_T y;                                                              /* '<S98>/hold' */
  boolean_T gearMode;                                                    /* '<S378>/Logical Operator' */
  boolean_T gearMode_c;                                                  /* '<S380>/Logical Operator' */
  boolean_T LogicalOperator1;                                            /* '<S379>/Logical Operator1' */
  boolean_T FixPtRelationalOperator;                                     /* '<S326>/FixPt Relational Operator' */
  B_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential_h; /* '<S55>/Limited Slip Differential' */
  B_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential_l; /* '<S10>/Limited Slip Differential' */
  B_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential_b; /* '<S284>/Limited Slip Differential' */
  B_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential_k; /* '<S185>/Limited Slip Differential' */
  B_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential_i; /* '<S159>/Limited Slip Differential' */
  B_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential;   /* '<S108>/Limited Slip Differential' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_TX_DriveLine_T {
  real_T Memory_PreviousInput;       /* '<S45>/Memory' */
  real_T Memory_PreviousInput_c;     /* '<S37>/Memory' */
  real_T Memory_PreviousInput_cw;    /* '<S82>/Memory' */
  real_T Memory_PreviousInput_ch;    /* '<S90>/Memory' */
  real_T Memory_PreviousInput_e;     /* '<S333>/Memory' */
  real_T Memory_PreviousInput_i;     /* '<S370>/Memory' */
  real_T Memory_PreviousInput_es;    /* '<S143>/Memory' */
  real_T Memory_PreviousInput_f;     /* '<S135>/Memory' */
  real_T Memory_PreviousInput_o;     /* '<S311>/Memory' */
  real_T Memory_PreviousInput_h;     /* '<S319>/Memory' */
  real_T Memory_PreviousInput_n;     /* '<S244>/Memory' */
  real_T Memory_PreviousInput_b;     /* '<S252>/Memory' */
  real_T Memory_PreviousInput_fa;    /* '<S228>/Memory' */
  real_T Memory_PreviousInput_l;     /* '<S236>/Memory' */
  real_T Memory_PreviousInput_bo;    /* '<S212>/Memory' */
  real_T Memory_PreviousInput_fn;    /* '<S220>/Memory' */
  int32_T DelayInput1_DSTATE;        /* '<S326>/Delay Input1' */
  int32_T sfEvent;                   /* '<S377>/TCM Shift Controller' */
  uint32_T temporalCounter_i1;       /* '<S377>/TCM Shift Controller' */
  uint32_T temporalCounter_i1_a;     /* '<S98>/hold' */
  int_T Integrator_IWORK;            /* '<S45>/Integrator' */
  int_T Integrator_IWORK_h;          /* '<S37>/Integrator' */
  int_T Integrator_IWORK_k;          /* '<S82>/Integrator' */
  int_T Integrator_IWORK_j;          /* '<S90>/Integrator' */
  int_T Integrator_IWORK_g;          /* '<S10>/Integrator' */
  int_T Integrator_IWORK_d;          /* '<S55>/Integrator' */
  int_T Integrator_IWORK_c;          /* '<S333>/Integrator' */
  int_T Integrator_IWORK_m;          /* '<S370>/Integrator' */
  int_T Integrator_IWORK_gr;         /* '<S143>/Integrator' */
  int_T Integrator_IWORK_e;          /* '<S135>/Integrator' */
  int_T Integrator_IWORK_i;          /* '<S311>/Integrator' */
  int_T Integrator_IWORK_gy;         /* '<S319>/Integrator' */
  int_T Integrator_IWORK_ij;         /* '<S244>/Integrator' */
  int_T Integrator_IWORK_hd;         /* '<S252>/Integrator' */
  int_T Integrator_IWORK_hs;         /* '<S228>/Integrator' */
  int_T Integrator_IWORK_n;          /* '<S236>/Integrator' */
  int_T Integrator_IWORK_p;          /* '<S108>/Integrator' */
  int_T Integrator_IWORK_a;          /* '<S284>/Integrator' */
  int_T Integrator_IWORK_ao;         /* '<S158>/Integrator' */
  int_T Integrator_IWORK_ct;         /* '<S212>/Integrator' */
  int_T Integrator_IWORK_b;          /* '<S159>/Integrator' */
  int_T Integrator_IWORK_o;          /* '<S220>/Integrator' */
  int_T Integrator_IWORK_f;          /* '<S185>/Integrator' */
  int_T xe_IWORK;                    /* '<S337>/xe' */
  int_T xv_IWORK;                    /* '<S337>/xv' */
  int_T x_IWORK;                     /* '<S335>/x' */
  int8_T SwitchCase_ActiveSubsystem; /* '<S1>/Switch Case' */
  int8_T If_ActiveSubsystem;         /* '<S331>/If' */
  uint8_T is_GearCmd;                /* '<S377>/TCM Shift Controller' */
  uint8_T is_SelectionState;         /* '<S377>/TCM Shift Controller' */
  uint8_T is_active_c3_TX_DriveLine; /* '<S377>/TCM Shift Controller' */
  uint8_T is_active_GearCmd;         /* '<S377>/TCM Shift Controller' */
  uint8_T is_active_SelectionState;  /* '<S377>/TCM Shift Controller' */
  uint8_T is_c1_TX_DriveLine;        /* '<S98>/hold' */
  uint8_T is_active_c1_TX_DriveLine; /* '<S98>/hold' */
  boolean_T IC_FirstOutputTime;      /* '<S338>/IC' */
  boolean_T IC_FirstOutputTime_e;    /* '<S336>/IC' */
  boolean_T IC_FirstOutputTime_f;    /* '<S334>/IC' */
};

/* Continuous states (default storage) */
struct X_TX_DriveLine_T {
  real_T Integrator_CSTATE;       /* '<S45>/Integrator' */
  real_T Integrator_CSTATE_g;     /* '<S44>/Integrator' */
  real_T Integrator_CSTATE_j;     /* '<S37>/Integrator' */
  real_T Integrator_CSTATE_l;     /* '<S36>/Integrator' */
  real_T Integrator_CSTATE_p;     /* '<S82>/Integrator' */
  real_T Integrator_CSTATE_e;     /* '<S81>/Integrator' */
  real_T Integrator_CSTATE_i;     /* '<S90>/Integrator' */
  real_T Integrator_CSTATE_im;    /* '<S89>/Integrator' */
  real_T Integrator_CSTATE_b[2];  /* '<S10>/Integrator' */
  real_T Integrator_CSTATE_n[2];  /* '<S55>/Integrator' */
  real_T Integrator_CSTATE_m;     /* '<S12>/Integrator' */
  real_T Integrator_CSTATE_ne;    /* '<S57>/Integrator' */
  real_T Integrator_CSTATE_f;     /* '<S333>/Integrator' */
  real_T Integrator_CSTATE_h;     /* '<S370>/Integrator' */
  real_T Integrator_CSTATE_c;     /* '<S369>/Integrator' */
  real_T Integrator_CSTATE_i5;    /* '<S143>/Integrator' */
  real_T Integrator_CSTATE_fk;    /* '<S142>/Integrator' */
  real_T Integrator_CSTATE_d;     /* '<S135>/Integrator' */
  real_T Integrator_CSTATE_nq;    /* '<S134>/Integrator' */
  real_T Integrator_CSTATE_ng;    /* '<S311>/Integrator' */
  real_T Integrator_CSTATE_d1;    /* '<S310>/Integrator' */
  real_T Integrator_CSTATE_hk;    /* '<S319>/Integrator' */
  real_T Integrator_CSTATE_lg;    /* '<S318>/Integrator' */
  real_T Integrator_CSTATE_lz;    /* '<S244>/Integrator' */
  real_T Integrator_CSTATE_iy;    /* '<S243>/Integrator' */
  real_T Integrator_CSTATE_bb;    /* '<S252>/Integrator' */
  real_T Integrator_CSTATE_b2;    /* '<S251>/Integrator' */
  real_T Integrator_CSTATE_c1;    /* '<S228>/Integrator' */
  real_T Integrator_CSTATE_hkp;   /* '<S227>/Integrator' */
  real_T Integrator_CSTATE_bx;    /* '<S236>/Integrator' */
  real_T Integrator_CSTATE_g1;    /* '<S235>/Integrator' */
  real_T Integrator_CSTATE_h2[2]; /* '<S108>/Integrator' */
  real_T Integrator_CSTATE_c0[2]; /* '<S284>/Integrator' */
  real_T Integrator_CSTATE_g3[2]; /* '<S158>/Integrator' */
  real_T Integrator_CSTATE_nv;    /* '<S110>/Integrator' */
  real_T Integrator_CSTATE_db;    /* '<S212>/Integrator' */
  real_T Integrator_CSTATE_dv;    /* '<S211>/Integrator' */
  real_T Integrator_CSTATE_eu[2]; /* '<S159>/Integrator' */
  real_T Integrator_CSTATE_ea;    /* '<S161>/Integrator' */
  real_T Integrator_CSTATE_ny;    /* '<S220>/Integrator' */
  real_T Integrator_CSTATE_hz;    /* '<S219>/Integrator' */
  real_T Integrator_CSTATE_j2[2]; /* '<S185>/Integrator' */
  real_T Integrator_CSTATE_a;     /* '<S187>/Integrator' */
  real_T Integrator_CSTATE_a4;    /* '<S286>/Integrator' */
  real_T Integrator1_CSTATE;      /* '<S382>/Integrator1' */
  real_T we;                      /* '<S337>/xe' */
  real_T wv;                      /* '<S337>/xv' */
  real_T w;                       /* '<S335>/x' */
};

/* State derivatives (default storage) */
struct XDot_TX_DriveLine_T {
  real_T Integrator_CSTATE;       /* '<S45>/Integrator' */
  real_T Integrator_CSTATE_g;     /* '<S44>/Integrator' */
  real_T Integrator_CSTATE_j;     /* '<S37>/Integrator' */
  real_T Integrator_CSTATE_l;     /* '<S36>/Integrator' */
  real_T Integrator_CSTATE_p;     /* '<S82>/Integrator' */
  real_T Integrator_CSTATE_e;     /* '<S81>/Integrator' */
  real_T Integrator_CSTATE_i;     /* '<S90>/Integrator' */
  real_T Integrator_CSTATE_im;    /* '<S89>/Integrator' */
  real_T Integrator_CSTATE_b[2];  /* '<S10>/Integrator' */
  real_T Integrator_CSTATE_n[2];  /* '<S55>/Integrator' */
  real_T Integrator_CSTATE_m;     /* '<S12>/Integrator' */
  real_T Integrator_CSTATE_ne;    /* '<S57>/Integrator' */
  real_T Integrator_CSTATE_f;     /* '<S333>/Integrator' */
  real_T Integrator_CSTATE_h;     /* '<S370>/Integrator' */
  real_T Integrator_CSTATE_c;     /* '<S369>/Integrator' */
  real_T Integrator_CSTATE_i5;    /* '<S143>/Integrator' */
  real_T Integrator_CSTATE_fk;    /* '<S142>/Integrator' */
  real_T Integrator_CSTATE_d;     /* '<S135>/Integrator' */
  real_T Integrator_CSTATE_nq;    /* '<S134>/Integrator' */
  real_T Integrator_CSTATE_ng;    /* '<S311>/Integrator' */
  real_T Integrator_CSTATE_d1;    /* '<S310>/Integrator' */
  real_T Integrator_CSTATE_hk;    /* '<S319>/Integrator' */
  real_T Integrator_CSTATE_lg;    /* '<S318>/Integrator' */
  real_T Integrator_CSTATE_lz;    /* '<S244>/Integrator' */
  real_T Integrator_CSTATE_iy;    /* '<S243>/Integrator' */
  real_T Integrator_CSTATE_bb;    /* '<S252>/Integrator' */
  real_T Integrator_CSTATE_b2;    /* '<S251>/Integrator' */
  real_T Integrator_CSTATE_c1;    /* '<S228>/Integrator' */
  real_T Integrator_CSTATE_hkp;   /* '<S227>/Integrator' */
  real_T Integrator_CSTATE_bx;    /* '<S236>/Integrator' */
  real_T Integrator_CSTATE_g1;    /* '<S235>/Integrator' */
  real_T Integrator_CSTATE_h2[2]; /* '<S108>/Integrator' */
  real_T Integrator_CSTATE_c0[2]; /* '<S284>/Integrator' */
  real_T Integrator_CSTATE_g3[2]; /* '<S158>/Integrator' */
  real_T Integrator_CSTATE_nv;    /* '<S110>/Integrator' */
  real_T Integrator_CSTATE_db;    /* '<S212>/Integrator' */
  real_T Integrator_CSTATE_dv;    /* '<S211>/Integrator' */
  real_T Integrator_CSTATE_eu[2]; /* '<S159>/Integrator' */
  real_T Integrator_CSTATE_ea;    /* '<S161>/Integrator' */
  real_T Integrator_CSTATE_ny;    /* '<S220>/Integrator' */
  real_T Integrator_CSTATE_hz;    /* '<S219>/Integrator' */
  real_T Integrator_CSTATE_j2[2]; /* '<S185>/Integrator' */
  real_T Integrator_CSTATE_a;     /* '<S187>/Integrator' */
  real_T Integrator_CSTATE_a4;    /* '<S286>/Integrator' */
  real_T Integrator1_CSTATE;      /* '<S382>/Integrator1' */
  real_T we;                      /* '<S337>/xe' */
  real_T wv;                      /* '<S337>/xv' */
  real_T w;                       /* '<S335>/x' */
};

/* State disabled  */
struct XDis_TX_DriveLine_T {
  boolean_T Integrator_CSTATE;       /* '<S45>/Integrator' */
  boolean_T Integrator_CSTATE_g;     /* '<S44>/Integrator' */
  boolean_T Integrator_CSTATE_j;     /* '<S37>/Integrator' */
  boolean_T Integrator_CSTATE_l;     /* '<S36>/Integrator' */
  boolean_T Integrator_CSTATE_p;     /* '<S82>/Integrator' */
  boolean_T Integrator_CSTATE_e;     /* '<S81>/Integrator' */
  boolean_T Integrator_CSTATE_i;     /* '<S90>/Integrator' */
  boolean_T Integrator_CSTATE_im;    /* '<S89>/Integrator' */
  boolean_T Integrator_CSTATE_b[2];  /* '<S10>/Integrator' */
  boolean_T Integrator_CSTATE_n[2];  /* '<S55>/Integrator' */
  boolean_T Integrator_CSTATE_m;     /* '<S12>/Integrator' */
  boolean_T Integrator_CSTATE_ne;    /* '<S57>/Integrator' */
  boolean_T Integrator_CSTATE_f;     /* '<S333>/Integrator' */
  boolean_T Integrator_CSTATE_h;     /* '<S370>/Integrator' */
  boolean_T Integrator_CSTATE_c;     /* '<S369>/Integrator' */
  boolean_T Integrator_CSTATE_i5;    /* '<S143>/Integrator' */
  boolean_T Integrator_CSTATE_fk;    /* '<S142>/Integrator' */
  boolean_T Integrator_CSTATE_d;     /* '<S135>/Integrator' */
  boolean_T Integrator_CSTATE_nq;    /* '<S134>/Integrator' */
  boolean_T Integrator_CSTATE_ng;    /* '<S311>/Integrator' */
  boolean_T Integrator_CSTATE_d1;    /* '<S310>/Integrator' */
  boolean_T Integrator_CSTATE_hk;    /* '<S319>/Integrator' */
  boolean_T Integrator_CSTATE_lg;    /* '<S318>/Integrator' */
  boolean_T Integrator_CSTATE_lz;    /* '<S244>/Integrator' */
  boolean_T Integrator_CSTATE_iy;    /* '<S243>/Integrator' */
  boolean_T Integrator_CSTATE_bb;    /* '<S252>/Integrator' */
  boolean_T Integrator_CSTATE_b2;    /* '<S251>/Integrator' */
  boolean_T Integrator_CSTATE_c1;    /* '<S228>/Integrator' */
  boolean_T Integrator_CSTATE_hkp;   /* '<S227>/Integrator' */
  boolean_T Integrator_CSTATE_bx;    /* '<S236>/Integrator' */
  boolean_T Integrator_CSTATE_g1;    /* '<S235>/Integrator' */
  boolean_T Integrator_CSTATE_h2[2]; /* '<S108>/Integrator' */
  boolean_T Integrator_CSTATE_c0[2]; /* '<S284>/Integrator' */
  boolean_T Integrator_CSTATE_g3[2]; /* '<S158>/Integrator' */
  boolean_T Integrator_CSTATE_nv;    /* '<S110>/Integrator' */
  boolean_T Integrator_CSTATE_db;    /* '<S212>/Integrator' */
  boolean_T Integrator_CSTATE_dv;    /* '<S211>/Integrator' */
  boolean_T Integrator_CSTATE_eu[2]; /* '<S159>/Integrator' */
  boolean_T Integrator_CSTATE_ea;    /* '<S161>/Integrator' */
  boolean_T Integrator_CSTATE_ny;    /* '<S220>/Integrator' */
  boolean_T Integrator_CSTATE_hz;    /* '<S219>/Integrator' */
  boolean_T Integrator_CSTATE_j2[2]; /* '<S185>/Integrator' */
  boolean_T Integrator_CSTATE_a;     /* '<S187>/Integrator' */
  boolean_T Integrator_CSTATE_a4;    /* '<S286>/Integrator' */
  boolean_T Integrator1_CSTATE;      /* '<S382>/Integrator1' */
  boolean_T we;                      /* '<S337>/xe' */
  boolean_T wv;                      /* '<S337>/xv' */
  boolean_T w;                       /* '<S335>/x' */
};

/* Zero-crossing (trigger) state */
struct PrevZCX_TX_DriveLine_T {
  ZCSigState Integrator_Reset_ZCE;     /* '<S45>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_b;   /* '<S37>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_i;   /* '<S82>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_h;   /* '<S90>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_m;   /* '<S333>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_e;   /* '<S370>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_f;   /* '<S143>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_k;   /* '<S135>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_hb;  /* '<S311>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_d;   /* '<S319>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_p;   /* '<S244>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_n;   /* '<S252>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_pu;  /* '<S228>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_hbb; /* '<S236>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_nr;  /* '<S212>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_j;   /* '<S220>/Integrator' */
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
struct ExtU_TX_DriveLine_T {
  driveline_in DriveLineIn; /* '<Root>/DriveLineIn' */
  powecu_out SoftECUOut;    /* '<Root>/SoftECUOut' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_TX_DriveLine_T {
  driveline_out DrivelineOut; /* '<Root>/DrivelineOut' */
};

/* Parameters for system: '<S108>/Limited Slip Differential' */
struct P_LimitedSlipDifferential_TX_DriveLine_T_ {
  real_T LimitedSlipDifferential_shaftSwitchMask; /* Expression: shaftSwitchMask
                                                   * Referenced by: '<S108>/Limited Slip Differential'
                                                   */
};

/* Parameters (default storage) */
struct P_TX_DriveLine_T_ {
  struct_TdKnmvEBt9bbFNHrBt8OwG Trans;                      /* Variable: Trans
                                                             * Referenced by:
                                                             *   '<S331>/Constant'
                                                             *   '<S337>/Gear2damping'
                                                             *   '<S337>/Gear2inertias'
                                                             *   '<S337>/Gear2inertias1'
                                                             *   '<S349>/Gear2Ratios'
                                                             *   '<S349>/Gear2damping'
                                                             *   '<S349>/Gear2inertias'
                                                             *   '<S361>/Gear2Ratios'
                                                             *   '<S361>/Gear2damping'
                                                             *   '<S361>/Gear2inertias'
                                                             *   '<S358>/Eta 1D'
                                                             *   '<S368>/Eta 1D'
                                                             */
  struct_hFtzkwIyDan9Uw0DEd8s7D VEH;                        /* Variable: VEH
                                                             * Referenced by:
                                                             *   '<S158>/Constant'
                                                             *   '<S158>/Constant1'
                                                             *   '<S10>/Constant'
                                                             *   '<S10>/Constant1'
                                                             *   '<S55>/Constant'
                                                             *   '<S55>/Constant1'
                                                             *   '<S108>/Constant'
                                                             *   '<S108>/Constant1'
                                                             *   '<S159>/Constant'
                                                             *   '<S159>/Constant1'
                                                             *   '<S185>/Constant'
                                                             *   '<S185>/Constant1'
                                                             *   '<S284>/Constant'
                                                             *   '<S284>/Constant1'
                                                             *   '<S334>/Constant'
                                                             *   '<S338>/Constant'
                                                             */
  real_T Dn_Gears[tx_car::kMap1dSize];                /* Variable: Dn_Gears
                                                             * Referenced by:
                                                             *   '<S388>/Calculate  Downshift Threshold'
                                                             *   '<S389>/Calculate  Downshift Threshold'
                                                             */
  real_T Downshift_Speeds[tx_car::kMap2dSize];        /* Variable: Downshift_Speeds
                                                             * Referenced by:
                                                             *   '<S388>/Calculate  Downshift Threshold'
                                                             *   '<S389>/Calculate  Downshift Threshold'
                                                             */
  real_T EngIdleSpd;                                        /* Variable: EngIdleSpd
                                                             * Referenced by: '<S336>/Constant'
                                                             */
  real_T Pedal_Positions_DnShift[tx_car::kMap1dSize]; /* Variable: Pedal_Positions_DnShift
                                                             * Referenced by:
                                                             *   '<S388>/Calculate  Downshift Threshold'
                                                             *   '<S389>/Calculate  Downshift Threshold'
                                                             */
  real_T Pedal_Positions_UpShift[tx_car::kMap1dSize]; /* Variable: Pedal_Positions_UpShift
                                                             * Referenced by:
                                                             *   '<S388>/Calculate Upshift Threshold'
                                                             *   '<S390>/Calculate Upshift Threshold'
                                                             */
  real_T StatLdWhlR[4];                                     /* Variable: StatLdWhlR
                                                             * Referenced by:
                                                             *   '<S101>/speed'
                                                             *   '<S377>/Gain'
                                                             *   '<S158>/Constant'
                                                             *   '<S158>/Constant1'
                                                             *   '<S10>/Constant'
                                                             *   '<S10>/Constant1'
                                                             *   '<S55>/Constant'
                                                             *   '<S55>/Constant1'
                                                             *   '<S108>/Constant'
                                                             *   '<S108>/Constant1'
                                                             *   '<S159>/Constant'
                                                             *   '<S159>/Constant1'
                                                             *   '<S185>/Constant'
                                                             *   '<S185>/Constant1'
                                                             *   '<S284>/Constant'
                                                             *   '<S284>/Constant1'
                                                             *   '<S334>/Constant'
                                                             *   '<S338>/Constant'
                                                             */
  real_T Up_Gears[tx_car::kMap1dSize];                /* Variable: Up_Gears
                                                             * Referenced by:
                                                             *   '<S388>/Calculate Upshift Threshold'
                                                             *   '<S390>/Calculate Upshift Threshold'
                                                             */
  real_T Upshift_Speeds[tx_car::kMap2dSize];          /* Variable: Upshift_Speeds
                                                             * Referenced by:
                                                             *   '<S388>/Calculate Upshift Threshold'
                                                             *   '<S390>/Calculate Upshift Threshold'
                                                             */
  real_T drive_type;                                        /* Variable: drive_type
                                                             * Referenced by: '<S1>/DriveType[1-FD;2-RD;3-4WD]'
                                                             */
  real_T front_motor_gear_ratio;                            /* Variable: front_motor_gear_ratio
                                                             * Referenced by:
                                                             *   '<S5>/Gain'
                                                             *   '<S5>/Gain1'
                                                             */
  real_T pre_shift_wait_time;                               /* Variable: pre_shift_wait_time
                                                             * Referenced by: '<S377>/TCM Shift Controller'
                                                             */
  real_T ratio_diff_front;                                  /* Variable: ratio_diff_front
                                                             * Referenced by:
                                                             *   '<S158>/Constant'
                                                             *   '<S10>/Ndiff2'
                                                             *   '<S108>/Ndiff2'
                                                             *   '<S159>/Ndiff2'
                                                             *   '<S16>/Gain'
                                                             *   '<S114>/Gain'
                                                             *   '<S165>/Gain'
                                                             *   '<S334>/Constant'
                                                             *   '<S338>/Constant'
                                                             *   '<S19>/Gain'
                                                             *   '<S117>/Gain'
                                                             *   '<S168>/Gain'
                                                             */
  real_T ratio_diff_rear;                                   /* Variable: ratio_diff_rear
                                                             * Referenced by:
                                                             *   '<S158>/Constant1'
                                                             *   '<S55>/Ndiff2'
                                                             *   '<S185>/Ndiff2'
                                                             *   '<S284>/Ndiff2'
                                                             *   '<S61>/Gain'
                                                             *   '<S191>/Gain'
                                                             *   '<S290>/Gain'
                                                             *   '<S334>/Constant'
                                                             *   '<S338>/Constant'
                                                             *   '<S64>/Gain'
                                                             *   '<S194>/Gain'
                                                             *   '<S293>/Gain'
                                                             */
  real_T rear_motor_gear_ratio;                             /* Variable: rear_motor_gear_ratio
                                                             * Referenced by:
                                                             *   '<S6>/Gain'
                                                             *   '<S6>/Gain1'
                                                             */
  uint8_T driveline_pow_type;                               /* Variable: driveline_pow_type
                                                             * Referenced by: '<S1>/PowerType[1-ICE;2-MOT]'
                                                             */
  real_T ICEFrontLimitedSlipDifferential_Fc;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Fc
   * Referenced by: '<S122>/Constant2'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Fc;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Fc
   * Referenced by: '<S173>/Constant2'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Fc;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Fc
   * Referenced by: '<S199>/Constant2'
   */
  real_T ICERearLimitedSlipDifferential_Fc;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Fc
   * Referenced by: '<S298>/Constant2'
   */
  real_T EVFrontLimitedSlipDifferential_Fc;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Fc
   * Referenced by: '<S24>/Constant2'
   */
  real_T EVRearLimitedSlipDifferential_Fc;
  /* Mask Parameter: EVRearLimitedSlipDifferential_Fc
   * Referenced by: '<S69>/Constant2'
   */
  real_T IdealFixedGearTransmission_G_o;
  /* Mask Parameter: IdealFixedGearTransmission_G_o
   * Referenced by: '<S331>/Constant1'
   */
  real_T ICEFrontLimitedSlipDifferential_Jd;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Jd
   * Referenced by: '<S108>/Jd'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Jd;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Jd
   * Referenced by: '<S159>/Jd'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Jd;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Jd
   * Referenced by: '<S185>/Jd'
   */
  real_T TransferCase_Jd; /* Mask Parameter: TransferCase_Jd
                           * Referenced by: '<S158>/Jd'
                           */
  real_T ICERearLimitedSlipDifferential_Jd;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Jd
   * Referenced by: '<S284>/Jd'
   */
  real_T EVFrontLimitedSlipDifferential_Jd;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Jd
   * Referenced by: '<S10>/Jd'
   */
  real_T EVRearLimitedSlipDifferential_Jd;
  /* Mask Parameter: EVRearLimitedSlipDifferential_Jd
   * Referenced by: '<S55>/Jd'
   */
  real_T ICEFrontLimitedSlipDifferential_Jw1;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Jw1
   * Referenced by: '<S108>/Jw1'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Jw1;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Jw1
   * Referenced by: '<S159>/Jw1'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Jw1;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Jw1
   * Referenced by: '<S185>/Jw1'
   */
  real_T TransferCase_Jw1; /* Mask Parameter: TransferCase_Jw1
                            * Referenced by: '<S158>/Jw1'
                            */
  real_T ICERearLimitedSlipDifferential_Jw1;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Jw1
   * Referenced by: '<S284>/Jw1'
   */
  real_T EVFrontLimitedSlipDifferential_Jw1;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Jw1
   * Referenced by: '<S10>/Jw1'
   */
  real_T EVRearLimitedSlipDifferential_Jw1;
  /* Mask Parameter: EVRearLimitedSlipDifferential_Jw1
   * Referenced by: '<S55>/Jw1'
   */
  real_T ICEFrontLimitedSlipDifferential_Jw2;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Jw2
   * Referenced by: '<S108>/Jw3'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Jw2;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Jw2
   * Referenced by: '<S159>/Jw3'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Jw2;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Jw2
   * Referenced by: '<S185>/Jw3'
   */
  real_T TransferCase_Jw2; /* Mask Parameter: TransferCase_Jw2
                            * Referenced by: '<S158>/Jw3'
                            */
  real_T ICERearLimitedSlipDifferential_Jw2;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Jw2
   * Referenced by: '<S284>/Jw3'
   */
  real_T EVFrontLimitedSlipDifferential_Jw2;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Jw2
   * Referenced by: '<S10>/Jw3'
   */
  real_T EVRearLimitedSlipDifferential_Jw2;
  /* Mask Parameter: EVRearLimitedSlipDifferential_Jw2
   * Referenced by: '<S55>/Jw3'
   */
  real_T TransferCase_Ndiff; /* Mask Parameter: TransferCase_Ndiff
                              * Referenced by:
                              *   '<S158>/Ndiff2'
                              *   '<S262>/Gain'
                              *   '<S265>/Gain1'
                              */
  real_T ICEFrontLimitedSlipDifferential_Ndisks;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Ndisks
   * Referenced by: '<S122>/Constant1'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Ndisks;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Ndisks
   * Referenced by: '<S173>/Constant1'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Ndisks;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Ndisks
   * Referenced by: '<S199>/Constant1'
   */
  real_T ICERearLimitedSlipDifferential_Ndisks;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Ndisks
   * Referenced by: '<S298>/Constant1'
   */
  real_T EVFrontLimitedSlipDifferential_Ndisks;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Ndisks
   * Referenced by: '<S24>/Constant1'
   */
  real_T EVRearLimitedSlipDifferential_Ndisks;
  /* Mask Parameter: EVRearLimitedSlipDifferential_Ndisks
   * Referenced by: '<S69>/Constant1'
   */
  real_T ICEFrontLimitedSlipDifferential_Reff;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_Reff
   * Referenced by: '<S122>/Constant'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_Reff;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Reff
   * Referenced by: '<S173>/Constant'
   */
  real_T uWD_ICERearLimitedSlipDifferential_Reff;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Reff
   * Referenced by: '<S199>/Constant'
   */
  real_T ICERearLimitedSlipDifferential_Reff;
  /* Mask Parameter: ICERearLimitedSlipDifferential_Reff
   * Referenced by: '<S298>/Constant'
   */
  real_T EVFrontLimitedSlipDifferential_Reff;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Reff
   * Referenced by: '<S24>/Constant'
   */
  real_T EVRearLimitedSlipDifferential_Reff;
  /* Mask Parameter: EVRearLimitedSlipDifferential_Reff
   * Referenced by: '<S69>/Constant'
   */
  real_T TransferCase_SpdLock; /* Mask Parameter: TransferCase_SpdLock
                                * Referenced by: '<S158>/SpdLockConstantConstant'
                                */
  real_T TransferCase_TrqSplitRatio;
  /* Mask Parameter: TransferCase_TrqSplitRatio
   * Referenced by: '<S158>/TrqSplitRatioConstantConstant'
   */
  real_T PropShaft_b;              /* Mask Parameter: PropShaft_b
                                    * Referenced by: '<S369>/Gain2'
                                    */
  real_T TorsionalCompliance2_b;   /* Mask Parameter: TorsionalCompliance2_b
                                    * Referenced by: '<S142>/Gain2'
                                    */
  real_T TorsionalCompliance1_b;   /* Mask Parameter: TorsionalCompliance1_b
                                    * Referenced by: '<S134>/Gain2'
                                    */
  real_T TorsionalCompliance_b;    /* Mask Parameter: TorsionalCompliance_b
                                    * Referenced by: '<S310>/Gain2'
                                    */
  real_T TorsionalCompliance1_b_j; /* Mask Parameter: TorsionalCompliance1_b_j
                                    * Referenced by: '<S318>/Gain2'
                                    */
  real_T TorsionalCompliance4_b;   /* Mask Parameter: TorsionalCompliance4_b
                                    * Referenced by: '<S243>/Gain2'
                                    */
  real_T TorsionalCompliance5_b;   /* Mask Parameter: TorsionalCompliance5_b
                                    * Referenced by: '<S251>/Gain2'
                                    */
  real_T TorsionalCompliance2_b_d; /* Mask Parameter: TorsionalCompliance2_b_d
                                    * Referenced by: '<S227>/Gain2'
                                    */
  real_T TorsionalCompliance3_b;   /* Mask Parameter: TorsionalCompliance3_b
                                    * Referenced by: '<S235>/Gain2'
                                    */
  real_T TorsionalCompliance_b_k;  /* Mask Parameter: TorsionalCompliance_b_k
                                    * Referenced by: '<S211>/Gain2'
                                    */
  real_T TorsionalCompliance1_b_f; /* Mask Parameter: TorsionalCompliance1_b_f
                                    * Referenced by: '<S219>/Gain2'
                                    */
  real_T TorsionalCompliance2_b_k; /* Mask Parameter: TorsionalCompliance2_b_k
                                    * Referenced by: '<S44>/Gain2'
                                    */
  real_T TorsionalCompliance1_b_d; /* Mask Parameter: TorsionalCompliance1_b_d
                                    * Referenced by: '<S36>/Gain2'
                                    */
  real_T TorsionalCompliance_b_m;  /* Mask Parameter: TorsionalCompliance_b_m
                                    * Referenced by: '<S81>/Gain2'
                                    */
  real_T TorsionalCompliance1_b_n; /* Mask Parameter: TorsionalCompliance1_b_n
                                    * Referenced by: '<S89>/Gain2'
                                    */
  real_T ICEFrontLimitedSlipDifferential_bd;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_bd
   * Referenced by: '<S108>/bd'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_bd;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_bd
   * Referenced by: '<S159>/bd'
   */
  real_T uWD_ICERearLimitedSlipDifferential_bd;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_bd
   * Referenced by: '<S185>/bd'
   */
  real_T TransferCase_bd; /* Mask Parameter: TransferCase_bd
                           * Referenced by: '<S158>/bd'
                           */
  real_T ICERearLimitedSlipDifferential_bd;
  /* Mask Parameter: ICERearLimitedSlipDifferential_bd
   * Referenced by: '<S284>/bd'
   */
  real_T EVFrontLimitedSlipDifferential_bd;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_bd
   * Referenced by: '<S10>/bd'
   */
  real_T EVRearLimitedSlipDifferential_bd;
  /* Mask Parameter: EVRearLimitedSlipDifferential_bd
   * Referenced by: '<S55>/bd'
   */
  real_T ICEFrontLimitedSlipDifferential_bw1;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_bw1
   * Referenced by: '<S108>/bw1'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_bw1;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_bw1
   * Referenced by: '<S159>/bw1'
   */
  real_T uWD_ICERearLimitedSlipDifferential_bw1;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_bw1
   * Referenced by: '<S185>/bw1'
   */
  real_T TransferCase_bw1; /* Mask Parameter: TransferCase_bw1
                            * Referenced by: '<S158>/bw1'
                            */
  real_T ICERearLimitedSlipDifferential_bw1;
  /* Mask Parameter: ICERearLimitedSlipDifferential_bw1
   * Referenced by: '<S284>/bw1'
   */
  real_T EVFrontLimitedSlipDifferential_bw1;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_bw1
   * Referenced by: '<S10>/bw1'
   */
  real_T EVRearLimitedSlipDifferential_bw1;
  /* Mask Parameter: EVRearLimitedSlipDifferential_bw1
   * Referenced by: '<S55>/bw1'
   */
  real_T ICEFrontLimitedSlipDifferential_bw2;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_bw2
   * Referenced by: '<S108>/bw2'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_bw2;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_bw2
   * Referenced by: '<S159>/bw2'
   */
  real_T uWD_ICERearLimitedSlipDifferential_bw2;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_bw2
   * Referenced by: '<S185>/bw2'
   */
  real_T TransferCase_bw2; /* Mask Parameter: TransferCase_bw2
                            * Referenced by: '<S158>/bw2'
                            */
  real_T ICERearLimitedSlipDifferential_bw2;
  /* Mask Parameter: ICERearLimitedSlipDifferential_bw2
   * Referenced by: '<S284>/bw2'
   */
  real_T EVFrontLimitedSlipDifferential_bw2;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_bw2
   * Referenced by: '<S10>/bw2'
   */
  real_T EVRearLimitedSlipDifferential_bw2;
  /* Mask Parameter: EVRearLimitedSlipDifferential_bw2
   * Referenced by: '<S55>/bw2'
   */
  real_T CompareToConstant_const;   /* Mask Parameter: CompareToConstant_const
                                     * Referenced by: '<S391>/Constant'
                                     */
  real_T CompareToConstant1_const;  /* Mask Parameter: CompareToConstant1_const
                                     * Referenced by: '<S384>/Constant'
                                     */
  real_T CompareToConstant_const_n; /* Mask Parameter: CompareToConstant_const_n
                                     * Referenced by: '<S383>/Constant'
                                     */
  real_T CompareToConstant1_const_h;
  /* Mask Parameter: CompareToConstant1_const_h
   * Referenced by: '<S387>/Constant'
   */
  real_T CompareToConstant_const_m; /* Mask Parameter: CompareToConstant_const_m
                                     * Referenced by: '<S386>/Constant'
                                     */
  real_T CompareToConstant_const_l; /* Mask Parameter: CompareToConstant_const_l
                                     * Referenced by: '<S385>/Constant'
                                     */
  real_T PropShaft_domega_o;        /* Mask Parameter: PropShaft_domega_o
                                     * Referenced by: '<S369>/domega_o'
                                     */
  real_T TorsionalCompliance2_domega_o;
  /* Mask Parameter: TorsionalCompliance2_domega_o
   * Referenced by: '<S142>/domega_o'
   */
  real_T TorsionalCompliance1_domega_o;
  /* Mask Parameter: TorsionalCompliance1_domega_o
   * Referenced by: '<S134>/domega_o'
   */
  real_T TorsionalCompliance_domega_o;
  /* Mask Parameter: TorsionalCompliance_domega_o
   * Referenced by: '<S310>/domega_o'
   */
  real_T TorsionalCompliance1_domega_o_c;
  /* Mask Parameter: TorsionalCompliance1_domega_o_c
   * Referenced by: '<S318>/domega_o'
   */
  real_T TorsionalCompliance4_domega_o;
  /* Mask Parameter: TorsionalCompliance4_domega_o
   * Referenced by: '<S243>/domega_o'
   */
  real_T TorsionalCompliance5_domega_o;
  /* Mask Parameter: TorsionalCompliance5_domega_o
   * Referenced by: '<S251>/domega_o'
   */
  real_T TorsionalCompliance2_domega_o_f;
  /* Mask Parameter: TorsionalCompliance2_domega_o_f
   * Referenced by: '<S227>/domega_o'
   */
  real_T TorsionalCompliance3_domega_o;
  /* Mask Parameter: TorsionalCompliance3_domega_o
   * Referenced by: '<S235>/domega_o'
   */
  real_T TorsionalCompliance_domega_o_d;
  /* Mask Parameter: TorsionalCompliance_domega_o_d
   * Referenced by: '<S211>/domega_o'
   */
  real_T TorsionalCompliance1_domega_o_h;
  /* Mask Parameter: TorsionalCompliance1_domega_o_h
   * Referenced by: '<S219>/domega_o'
   */
  real_T TorsionalCompliance2_domega_o_k;
  /* Mask Parameter: TorsionalCompliance2_domega_o_k
   * Referenced by: '<S44>/domega_o'
   */
  real_T TorsionalCompliance1_domega_o_k;
  /* Mask Parameter: TorsionalCompliance1_domega_o_k
   * Referenced by: '<S36>/domega_o'
   */
  real_T TorsionalCompliance_domega_o_i;
  /* Mask Parameter: TorsionalCompliance_domega_o_i
   * Referenced by: '<S81>/domega_o'
   */
  real_T TorsionalCompliance1_domega_o_p;
  /* Mask Parameter: TorsionalCompliance1_domega_o_p
   * Referenced by: '<S89>/domega_o'
   */
  real_T ICEFrontLimitedSlipDifferential_dw[8];
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_dw
   * Referenced by: '<S122>/mu Table'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_dw[7];
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_dw
   * Referenced by: '<S173>/mu Table'
   */
  real_T uWD_ICERearLimitedSlipDifferential_dw[7];
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_dw
   * Referenced by: '<S199>/mu Table'
   */
  real_T ICERearLimitedSlipDifferential_dw[8];
  /* Mask Parameter: ICERearLimitedSlipDifferential_dw
   * Referenced by: '<S298>/mu Table'
   */
  real_T EVFrontLimitedSlipDifferential_dw[8];
  /* Mask Parameter: EVFrontLimitedSlipDifferential_dw
   * Referenced by: '<S24>/mu Table'
   */
  real_T EVRearLimitedSlipDifferential_dw[8];
  /* Mask Parameter: EVRearLimitedSlipDifferential_dw
   * Referenced by: '<S69>/mu Table'
   */
  real_T ICEFrontLimitedSlipDifferential_eta;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_eta
   * Referenced by: '<S133>/Constant'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_eta;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_eta
   * Referenced by: '<S184>/Constant'
   */
  real_T uWD_ICERearLimitedSlipDifferential_eta;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_eta
   * Referenced by: '<S210>/Constant'
   */
  real_T TransferCase_eta; /* Mask Parameter: TransferCase_eta
                            * Referenced by: '<S280>/Constant'
                            */
  real_T ICERearLimitedSlipDifferential_eta;
  /* Mask Parameter: ICERearLimitedSlipDifferential_eta
   * Referenced by: '<S309>/Constant'
   */
  real_T EVFrontLimitedSlipDifferential_eta;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_eta
   * Referenced by: '<S35>/Constant'
   */
  real_T EVRearLimitedSlipDifferential_eta;
  /* Mask Parameter: EVRearLimitedSlipDifferential_eta
   * Referenced by: '<S80>/Constant'
   */
  real_T PropShaft_k;              /* Mask Parameter: PropShaft_k
                                    * Referenced by: '<S369>/Gain1'
                                    */
  real_T TorsionalCompliance2_k;   /* Mask Parameter: TorsionalCompliance2_k
                                    * Referenced by: '<S142>/Gain1'
                                    */
  real_T TorsionalCompliance1_k;   /* Mask Parameter: TorsionalCompliance1_k
                                    * Referenced by: '<S134>/Gain1'
                                    */
  real_T TorsionalCompliance_k;    /* Mask Parameter: TorsionalCompliance_k
                                    * Referenced by: '<S310>/Gain1'
                                    */
  real_T TorsionalCompliance1_k_b; /* Mask Parameter: TorsionalCompliance1_k_b
                                    * Referenced by: '<S318>/Gain1'
                                    */
  real_T TorsionalCompliance4_k;   /* Mask Parameter: TorsionalCompliance4_k
                                    * Referenced by: '<S243>/Gain1'
                                    */
  real_T TorsionalCompliance5_k;   /* Mask Parameter: TorsionalCompliance5_k
                                    * Referenced by: '<S251>/Gain1'
                                    */
  real_T TorsionalCompliance2_k_l; /* Mask Parameter: TorsionalCompliance2_k_l
                                    * Referenced by: '<S227>/Gain1'
                                    */
  real_T TorsionalCompliance3_k;   /* Mask Parameter: TorsionalCompliance3_k
                                    * Referenced by: '<S235>/Gain1'
                                    */
  real_T TorsionalCompliance_k_h;  /* Mask Parameter: TorsionalCompliance_k_h
                                    * Referenced by: '<S211>/Gain1'
                                    */
  real_T TorsionalCompliance1_k_a; /* Mask Parameter: TorsionalCompliance1_k_a
                                    * Referenced by: '<S219>/Gain1'
                                    */
  real_T TorsionalCompliance2_k_g; /* Mask Parameter: TorsionalCompliance2_k_g
                                    * Referenced by: '<S44>/Gain1'
                                    */
  real_T TorsionalCompliance1_k_f; /* Mask Parameter: TorsionalCompliance1_k_f
                                    * Referenced by: '<S36>/Gain1'
                                    */
  real_T TorsionalCompliance_k_g;  /* Mask Parameter: TorsionalCompliance_k_g
                                    * Referenced by: '<S81>/Gain1'
                                    */
  real_T TorsionalCompliance1_k_p; /* Mask Parameter: TorsionalCompliance1_k_p
                                    * Referenced by: '<S89>/Gain1'
                                    */
  real_T ICEFrontLimitedSlipDifferential_muc[8];
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_muc
   * Referenced by: '<S122>/mu Table'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_muc[7];
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_muc
   * Referenced by: '<S173>/mu Table'
   */
  real_T uWD_ICERearLimitedSlipDifferential_muc[7];
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_muc
   * Referenced by: '<S199>/mu Table'
   */
  real_T ICERearLimitedSlipDifferential_muc[8];
  /* Mask Parameter: ICERearLimitedSlipDifferential_muc
   * Referenced by: '<S298>/mu Table'
   */
  real_T EVFrontLimitedSlipDifferential_muc[8];
  /* Mask Parameter: EVFrontLimitedSlipDifferential_muc
   * Referenced by: '<S24>/mu Table'
   */
  real_T EVRearLimitedSlipDifferential_muc[8];
  /* Mask Parameter: EVRearLimitedSlipDifferential_muc
   * Referenced by: '<S69>/mu Table'
   */
  real_T TorsionalCompliance1_omega_c;
  /* Mask Parameter: TorsionalCompliance1_omega_c
   * Referenced by: '<S134>/omega_c'
   */
  real_T TorsionalCompliance2_omega_c;
  /* Mask Parameter: TorsionalCompliance2_omega_c
   * Referenced by: '<S142>/omega_c'
   */
  real_T TorsionalCompliance_omega_c;
  /* Mask Parameter: TorsionalCompliance_omega_c
   * Referenced by: '<S211>/omega_c'
   */
  real_T TorsionalCompliance1_omega_c_b;
  /* Mask Parameter: TorsionalCompliance1_omega_c_b
   * Referenced by: '<S219>/omega_c'
   */
  real_T TorsionalCompliance2_omega_c_m;
  /* Mask Parameter: TorsionalCompliance2_omega_c_m
   * Referenced by: '<S227>/omega_c'
   */
  real_T TorsionalCompliance3_omega_c;
  /* Mask Parameter: TorsionalCompliance3_omega_c
   * Referenced by: '<S235>/omega_c'
   */
  real_T TorsionalCompliance4_omega_c;
  /* Mask Parameter: TorsionalCompliance4_omega_c
   * Referenced by: '<S243>/omega_c'
   */
  real_T TorsionalCompliance5_omega_c;
  /* Mask Parameter: TorsionalCompliance5_omega_c
   * Referenced by: '<S251>/omega_c'
   */
  real_T TorsionalCompliance_omega_c_b;
  /* Mask Parameter: TorsionalCompliance_omega_c_b
   * Referenced by: '<S310>/omega_c'
   */
  real_T TorsionalCompliance1_omega_c_k;
  /* Mask Parameter: TorsionalCompliance1_omega_c_k
   * Referenced by: '<S318>/omega_c'
   */
  real_T PropShaft_omega_c; /* Mask Parameter: PropShaft_omega_c
                             * Referenced by: '<S369>/omega_c'
                             */
  real_T TorsionalCompliance1_omega_c_g;
  /* Mask Parameter: TorsionalCompliance1_omega_c_g
   * Referenced by: '<S36>/omega_c'
   */
  real_T TorsionalCompliance2_omega_c_p;
  /* Mask Parameter: TorsionalCompliance2_omega_c_p
   * Referenced by: '<S44>/omega_c'
   */
  real_T TorsionalCompliance_omega_c_p;
  /* Mask Parameter: TorsionalCompliance_omega_c_p
   * Referenced by: '<S81>/omega_c'
   */
  real_T TorsionalCompliance1_omega_c_h;
  /* Mask Parameter: TorsionalCompliance1_omega_c_h
   * Referenced by: '<S89>/omega_c'
   */
  real_T TransferCase_shaftSwitchMask;
  /* Mask Parameter: TransferCase_shaftSwitchMask
   * Referenced by:
   *   '<S158>/TransferCase'
   *   '<S262>/Constant'
   *   '<S265>/Constant'
   */
  real_T ICEFrontLimitedSlipDifferential_tauC;
  /* Mask Parameter: ICEFrontLimitedSlipDifferential_tauC
   * Referenced by: '<S108>/Constant3'
   */
  real_T uWD_ICEFrontLimitedSlipDifferential_tauC;
  /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_tauC
   * Referenced by: '<S159>/Constant3'
   */
  real_T uWD_ICERearLimitedSlipDifferential_tauC;
  /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_tauC
   * Referenced by: '<S185>/Constant3'
   */
  real_T ICERearLimitedSlipDifferential_tauC;
  /* Mask Parameter: ICERearLimitedSlipDifferential_tauC
   * Referenced by: '<S284>/Constant3'
   */
  real_T EVFrontLimitedSlipDifferential_tauC;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_tauC
   * Referenced by: '<S10>/Constant3'
   */
  real_T EVRearLimitedSlipDifferential_tauC;
  /* Mask Parameter: EVRearLimitedSlipDifferential_tauC
   * Referenced by: '<S55>/Constant3'
   */
  real_T PropShaft_theta_o; /* Mask Parameter: PropShaft_theta_o
                             * Referenced by: '<S369>/Integrator'
                             */
  real_T TorsionalCompliance2_theta_o;
  /* Mask Parameter: TorsionalCompliance2_theta_o
   * Referenced by: '<S142>/Integrator'
   */
  real_T TorsionalCompliance1_theta_o;
  /* Mask Parameter: TorsionalCompliance1_theta_o
   * Referenced by: '<S134>/Integrator'
   */
  real_T TorsionalCompliance_theta_o;
  /* Mask Parameter: TorsionalCompliance_theta_o
   * Referenced by: '<S310>/Integrator'
   */
  real_T TorsionalCompliance1_theta_o_c;
  /* Mask Parameter: TorsionalCompliance1_theta_o_c
   * Referenced by: '<S318>/Integrator'
   */
  real_T TorsionalCompliance4_theta_o;
  /* Mask Parameter: TorsionalCompliance4_theta_o
   * Referenced by: '<S243>/Integrator'
   */
  real_T TorsionalCompliance5_theta_o;
  /* Mask Parameter: TorsionalCompliance5_theta_o
   * Referenced by: '<S251>/Integrator'
   */
  real_T TorsionalCompliance2_theta_o_l;
  /* Mask Parameter: TorsionalCompliance2_theta_o_l
   * Referenced by: '<S227>/Integrator'
   */
  real_T TorsionalCompliance3_theta_o;
  /* Mask Parameter: TorsionalCompliance3_theta_o
   * Referenced by: '<S235>/Integrator'
   */
  real_T TorsionalCompliance_theta_o_l;
  /* Mask Parameter: TorsionalCompliance_theta_o_l
   * Referenced by: '<S211>/Integrator'
   */
  real_T TorsionalCompliance1_theta_o_f;
  /* Mask Parameter: TorsionalCompliance1_theta_o_f
   * Referenced by: '<S219>/Integrator'
   */
  real_T TorsionalCompliance2_theta_o_m;
  /* Mask Parameter: TorsionalCompliance2_theta_o_m
   * Referenced by: '<S44>/Integrator'
   */
  real_T TorsionalCompliance1_theta_o_n;
  /* Mask Parameter: TorsionalCompliance1_theta_o_n
   * Referenced by: '<S36>/Integrator'
   */
  real_T TorsionalCompliance_theta_o_h;
  /* Mask Parameter: TorsionalCompliance_theta_o_h
   * Referenced by: '<S81>/Integrator'
   */
  real_T TorsionalCompliance1_theta_o_l;
  /* Mask Parameter: TorsionalCompliance1_theta_o_l
   * Referenced by: '<S89>/Integrator'
   */
  real_T div0protectabspoly_thresh; /* Mask Parameter: div0protectabspoly_thresh
                                     * Referenced by:
                                     *   '<S355>/Constant'
                                     *   '<S356>/Constant'
                                     */
  real_T div0protectabspoly_thresh_k;
  /* Mask Parameter: div0protectabspoly_thresh_k
   * Referenced by:
   *   '<S365>/Constant'
   *   '<S366>/Constant'
   */
  real_T Valve_wc;                   /* Mask Parameter: Valve_wc
                                      * Referenced by: '<S382>/Constant'
                                      */
  int32_T GearCmdChange_vinit;       /* Mask Parameter: GearCmdChange_vinit
                                      * Referenced by: '<S326>/Delay Input1'
                                      */
  driveline_out Merge_InitialOutput; /* Computed Parameter: Merge_InitialOutput
                                      * Referenced by: '<S1>/Merge'
                                      */
  real_T Constant_Value;             /* Expression: 1
                                      * Referenced by: '<S346>/Constant'
                                      */
  real_T Switch_Threshold;           /* Expression: 0
                                      * Referenced by: '<S346>/Switch'
                                      */
  real_T Gain1_Gain;                 /* Expression: 20
                                      * Referenced by: '<S351>/Gain1'
                                      */
  real_T Constant1_Value;            /* Expression: 1
                                      * Referenced by: '<S351>/Constant1'
                                      */
  real_T Gain2_Gain;                 /* Expression: 4
                                      * Referenced by: '<S351>/Gain2'
                                      */
  real_T Gain_Gain;                  /* Expression: .5
                                      * Referenced by: '<S351>/Gain'
                                      */
  real_T Constant_Value_k;           /* Expression: 1
                                      * Referenced by: '<S350>/Constant'
                                      */
  real_T Constant_Value_kg;          /* Expression: 1
                                      * Referenced by: '<S347>/Constant'
                                      */
  real_T Switch_Threshold_p;         /* Expression: 0
                                      * Referenced by: '<S347>/Switch'
                                      */
  real_T Gain1_Gain_d;               /* Expression: 20
                                      * Referenced by: '<S353>/Gain1'
                                      */
  real_T Constant1_Value_e;          /* Expression: 1
                                      * Referenced by: '<S353>/Constant1'
                                      */
  real_T Gain2_Gain_e;               /* Expression: 4
                                      * Referenced by: '<S353>/Gain2'
                                      */
  real_T Gain_Gain_f;                /* Expression: .5
                                      * Referenced by: '<S353>/Gain'
                                      */
  real_T Constant_Value_o;           /* Expression: 1
                                      * Referenced by: '<S352>/Constant'
                                      */
  real_T Gain_Gain_fx;               /* Expression: -1
                                      * Referenced by: '<S348>/Gain'
                                      */
  real_T Saturation1_UpperSat;       /* Expression: 0
                                      * Referenced by: '<S348>/Saturation1'
                                      */
  real_T Saturation1_LowerSat;       /* Expression: -inf
                                      * Referenced by: '<S348>/Saturation1'
                                      */
  real_T Saturation2_UpperSat;       /* Expression: inf
                                      * Referenced by: '<S348>/Saturation2'
                                      */
  real_T Saturation2_LowerSat;       /* Expression: 0
                                      * Referenced by: '<S348>/Saturation2'
                                      */
  real_T Saturation_UpperSat;        /* Expression: 1
                                      * Referenced by: '<S348>/Saturation'
                                      */
  real_T Saturation_LowerSat;        /* Expression: 0
                                      * Referenced by: '<S348>/Saturation'
                                      */
  real_T Neutral_Value;              /* Expression: 0
                                      * Referenced by: '<S337>/Neutral'
                                      */
  real_T Constant_Value_a;           /* Expression: 1
                                      * Referenced by: '<S359>/Constant'
                                      */
  real_T Switch_Threshold_b;         /* Expression: 0
                                      * Referenced by: '<S359>/Switch'
                                      */
  real_T Gain1_Gain_f;               /* Expression: 20
                                      * Referenced by: '<S363>/Gain1'
                                      */
  real_T Constant1_Value_f;          /* Expression: 1
                                      * Referenced by: '<S363>/Constant1'
                                      */
  real_T Gain2_Gain_l;               /* Expression: 4
                                      * Referenced by: '<S363>/Gain2'
                                      */
  real_T Gain_Gain_j;                /* Expression: .5
                                      * Referenced by: '<S363>/Gain'
                                      */
  real_T Constant_Value_h;           /* Expression: 1
                                      * Referenced by: '<S362>/Constant'
                                      */
  real_T First_Value;                /* Expression: 1
                                      * Referenced by: '<S337>/First'
                                      */
  real_T Gain_Gain_g;                /* Expression: -1
                                      * Referenced by: '<S360>/Gain'
                                      */
  real_T Saturation1_UpperSat_k;     /* Expression: 0
                                      * Referenced by: '<S360>/Saturation1'
                                      */
  real_T Saturation1_LowerSat_k;     /* Expression: -inf
                                      * Referenced by: '<S360>/Saturation1'
                                      */
  real_T Saturation2_UpperSat_p;     /* Expression: inf
                                      * Referenced by: '<S360>/Saturation2'
                                      */
  real_T Saturation2_LowerSat_j;     /* Expression: 0
                                      * Referenced by: '<S360>/Saturation2'
                                      */
  real_T Saturation_UpperSat_o;      /* Expression: 1
                                      * Referenced by: '<S360>/Saturation'
                                      */
  real_T Saturation_LowerSat_n;      /* Expression: 0
                                      * Referenced by: '<S360>/Saturation'
                                      */
  real_T up_th_Y0;                   /* Computed Parameter: up_th_Y0
                                      * Referenced by: '<S390>/up_th'
                                      */
  real_T down_th_Y0;                 /* Computed Parameter: down_th_Y0
                                      * Referenced by: '<S389>/down_th'
                                      */
  real_T SpdThr_Y0;                  /* Computed Parameter: SpdThr_Y0
                                      * Referenced by: '<S388>/SpdThr'
                                      */
  real_T Constant_Value_i;           /* Expression: 0
                                      * Referenced by: '<S392>/Constant'
                                      */
  real_T Memory_InitialCondition;    /* Expression: 0
                                      * Referenced by: '<S333>/Memory'
                                      */
  real_T DeadZone_Start;             /* Expression: -0.5
                                      * Referenced by: '<S331>/Dead Zone'
                                      */
  real_T DeadZone_End;               /* Expression: 0.5
                                      * Referenced by: '<S331>/Dead Zone'
                                      */
  real_T Constant_Value_n;           /* Expression: 0
                                      * Referenced by: '<S101>/Constant'
                                      */
  real_T Memory_InitialCondition_j;  /* Expression: 0
                                      * Referenced by: '<S370>/Memory'
                                      */
  real_T Constant1_Value_m;          /* Expression: 1
                                      * Referenced by: '<S338>/Constant1'
                                      */
  real_T IC_Value;                   /* Expression: 0
                                      * Referenced by: '<S338>/IC'
                                      */
  real_T Switch_Threshold_h;         /* Expression: 0
                                      * Referenced by: '<S338>/Switch'
                                      */
  real_T Constant1_Value_n;          /* Expression: 1
                                      * Referenced by: '<S336>/Constant1'
                                      */
  real_T IC_Value_j;                 /* Expression: 0
                                      * Referenced by: '<S336>/IC'
                                      */
  real_T Switch_Threshold_d;         /* Expression: 0
                                      * Referenced by: '<S336>/Switch'
                                      */
  real_T Constant1_Value_fx;         /* Expression: 1
                                      * Referenced by: '<S334>/Constant1'
                                      */
  real_T IC_Value_o;                 /* Expression: 0
                                      * Referenced by: '<S334>/IC'
                                      */
  real_T Switch_Threshold_a;         /* Expression: 0
                                      * Referenced by: '<S334>/Switch'
                                      */
  real_T Saturation_UpperSat_m;      /* Expression: 9000.0/60.0*2*3.1415926
                                      * Referenced by: '<S328>/Saturation'
                                      */
  real_T Saturation_LowerSat_a;      /* Expression: 750*2*3.1415926/60
                                      * Referenced by: '<S328>/Saturation'
                                      */
  real_T Memory_InitialCondition_k;  /* Expression: 0
                                      * Referenced by: '<S143>/Memory'
                                      */
  real_T Memory_InitialCondition_n;  /* Expression: 0
                                      * Referenced by: '<S135>/Memory'
                                      */
  real_T Memory_InitialCondition_nh; /* Expression: 0
                                      * Referenced by: '<S311>/Memory'
                                      */
  real_T Memory_InitialCondition_f;  /* Expression: 0
                                      * Referenced by: '<S319>/Memory'
                                      */
  real_T Memory_InitialCondition_c;  /* Expression: 0
                                      * Referenced by: '<S244>/Memory'
                                      */
  real_T Memory_InitialCondition_o;  /* Expression: 0
                                      * Referenced by: '<S252>/Memory'
                                      */
  real_T Memory_InitialCondition_ja; /* Expression: 0
                                      * Referenced by: '<S228>/Memory'
                                      */
  real_T Memory_InitialCondition_i;  /* Expression: 0
                                      * Referenced by: '<S236>/Memory'
                                      */
  real_T Constant1_Value_g;          /* Expression: 1
                                      * Referenced by: '<S114>/Constant1'
                                      */
  real_T Constant_Value_j;           /* Expression: shaftSwitchMask
                                      * Referenced by: '<S114>/Constant'
                                      */
  real_T Switch_Threshold_f;         /* Expression: 1
                                      * Referenced by: '<S114>/Switch'
                                      */
  real_T Integrator_UpperSat;        /* Expression: maxAbsSpd
                                      * Referenced by: '<S108>/Integrator'
                                      */
  real_T Integrator_LowerSat;        /* Expression: -maxAbsSpd
                                      * Referenced by: '<S108>/Integrator'
                                      */
  real_T Constant1_Value_b;          /* Expression: 1
                                      * Referenced by: '<S290>/Constant1'
                                      */
  real_T Constant_Value_jl;          /* Expression: shaftSwitchMask
                                      * Referenced by: '<S290>/Constant'
                                      */
  real_T Switch_Threshold_o;         /* Expression: 1
                                      * Referenced by: '<S290>/Switch'
                                      */
  real_T Integrator_UpperSat_f;      /* Expression: maxAbsSpd
                                      * Referenced by: '<S284>/Integrator'
                                      */
  real_T Integrator_LowerSat_p;      /* Expression: -maxAbsSpd
                                      * Referenced by: '<S284>/Integrator'
                                      */
  real_T Constant1_Value_c;          /* Expression: 1
                                      * Referenced by: '<S262>/Constant1'
                                      */
  real_T Switch_Threshold_c;         /* Expression: 1
                                      * Referenced by: '<S262>/Switch'
                                      */
  real_T Integrator_UpperSat_e;      /* Expression: maxAbsSpd
                                      * Referenced by: '<S158>/Integrator'
                                      */
  real_T Integrator_LowerSat_i;      /* Expression: -maxAbsSpd
                                      * Referenced by: '<S158>/Integrator'
                                      */
  real_T Constant2_Value;            /* Expression: 1
                                      * Referenced by: '<S262>/Constant2'
                                      */
  real_T Constant2_Value_b;          /* Expression: 0
                                      * Referenced by: '<S3>/Constant2'
                                      */
  real_T Constant3_Value;            /* Expression: 0
                                      * Referenced by: '<S3>/Constant3'
                                      */
  real_T upi_Gain;                   /* Expression: 2*pi
                                      * Referenced by: '<S108>/2*pi'
                                      */
  real_T Gain1_Gain_fp;              /* Expression: 1/2
                                      * Referenced by: '<S114>/Gain1'
                                      */
  real_T Constant_Value_f;           /* Expression: 1
                                      * Referenced by: '<S125>/Constant'
                                      */
  real_T Switch_Threshold_oj;        /* Expression: 0
                                      * Referenced by: '<S125>/Switch'
                                      */
  real_T Gain1_Gain_i;               /* Expression: 20
                                      * Referenced by: '<S132>/Gain1'
                                      */
  real_T Constant1_Value_l;          /* Expression: 1
                                      * Referenced by: '<S132>/Constant1'
                                      */
  real_T Gain2_Gain_el;              /* Expression: 4
                                      * Referenced by: '<S132>/Gain2'
                                      */
  real_T Gain_Gain_jy;               /* Expression: .5
                                      * Referenced by: '<S132>/Gain'
                                      */
  real_T Constant_Value_l;           /* Expression: 1
                                      * Referenced by: '<S131>/Constant'
                                      */
  real_T Constant_Value_aw;          /* Expression: 1
                                      * Referenced by: '<S123>/Constant'
                                      */
  real_T Switch_Threshold_l;         /* Expression: 0
                                      * Referenced by: '<S123>/Switch'
                                      */
  real_T Gain1_Gain_h;               /* Expression: 20
                                      * Referenced by: '<S128>/Gain1'
                                      */
  real_T Constant1_Value_o;          /* Expression: 1
                                      * Referenced by: '<S128>/Constant1'
                                      */
  real_T Gain2_Gain_o;               /* Expression: 4
                                      * Referenced by: '<S128>/Gain2'
                                      */
  real_T Gain_Gain_k;                /* Expression: .5
                                      * Referenced by: '<S128>/Gain'
                                      */
  real_T Constant_Value_b;           /* Expression: 1
                                      * Referenced by: '<S127>/Constant'
                                      */
  real_T Constant_Value_m;           /* Expression: 1
                                      * Referenced by: '<S124>/Constant'
                                      */
  real_T Switch_Threshold_pg;        /* Expression: 0
                                      * Referenced by: '<S124>/Switch'
                                      */
  real_T Gain1_Gain_a;               /* Expression: 20
                                      * Referenced by: '<S130>/Gain1'
                                      */
  real_T Constant1_Value_mu;         /* Expression: 1
                                      * Referenced by: '<S130>/Constant1'
                                      */
  real_T Gain2_Gain_n;               /* Expression: 4
                                      * Referenced by: '<S130>/Gain2'
                                      */
  real_T Gain_Gain_c;                /* Expression: .5
                                      * Referenced by: '<S130>/Gain'
                                      */
  real_T Constant_Value_lm;          /* Expression: 1
                                      * Referenced by: '<S129>/Constant'
                                      */
  real_T Integrator_IC;              /* Expression: 0
                                      * Referenced by: '<S110>/Integrator'
                                      */
  real_T Constant_Value_i1;          /* Expression: shaftSwitchMask
                                      * Referenced by: '<S117>/Constant'
                                      */
  real_T Constant2_Value_c;          /* Expression: 1
                                      * Referenced by: '<S117>/Constant2'
                                      */
  real_T Switch_Threshold_g;         /* Expression: 1
                                      * Referenced by: '<S117>/Switch'
                                      */
  real_T Gain_Gain_kg;               /* Expression: 4
                                      * Referenced by: '<S122>/Gain'
                                      */
  real_T Reset_Value;                /* Expression: 1
                                      * Referenced by: '<S135>/Reset'
                                      */
  real_T Reset_Value_f;              /* Expression: 1
                                      * Referenced by: '<S143>/Reset'
                                      */
  real_T upi_Gain_a;                 /* Expression: 2*pi
                                      * Referenced by: '<S159>/2*pi'
                                      */
  real_T Memory_InitialCondition_g;  /* Expression: 0
                                      * Referenced by: '<S212>/Memory'
                                      */
  real_T Constant1_Value_et;         /* Expression: 1
                                      * Referenced by: '<S165>/Constant1'
                                      */
  real_T Constant_Value_oe;          /* Expression: shaftSwitchMask
                                      * Referenced by: '<S165>/Constant'
                                      */
  real_T Switch_Threshold_ap;        /* Expression: 1
                                      * Referenced by: '<S165>/Switch'
                                      */
  real_T Integrator_UpperSat_er;     /* Expression: maxAbsSpd
                                      * Referenced by: '<S159>/Integrator'
                                      */
  real_T Integrator_LowerSat_n;      /* Expression: -maxAbsSpd
                                      * Referenced by: '<S159>/Integrator'
                                      */
  real_T Gain1_Gain_g;               /* Expression: 1/2
                                      * Referenced by: '<S165>/Gain1'
                                      */
  real_T Constant_Value_hn;          /* Expression: 1
                                      * Referenced by: '<S176>/Constant'
                                      */
  real_T Switch_Threshold_f0;        /* Expression: 0
                                      * Referenced by: '<S176>/Switch'
                                      */
  real_T Gain1_Gain_a0;              /* Expression: 20
                                      * Referenced by: '<S183>/Gain1'
                                      */
  real_T Constant1_Value_i;          /* Expression: 1
                                      * Referenced by: '<S183>/Constant1'
                                      */
  real_T Gain2_Gain_ls;              /* Expression: 4
                                      * Referenced by: '<S183>/Gain2'
                                      */
  real_T Gain_Gain_l;                /* Expression: .5
                                      * Referenced by: '<S183>/Gain'
                                      */
  real_T Constant_Value_kt;          /* Expression: 1
                                      * Referenced by: '<S182>/Constant'
                                      */
  real_T Constant_Value_is;          /* Expression: 1
                                      * Referenced by: '<S174>/Constant'
                                      */
  real_T Switch_Threshold_oa;        /* Expression: 0
                                      * Referenced by: '<S174>/Switch'
                                      */
  real_T Gain1_Gain_b;               /* Expression: 20
                                      * Referenced by: '<S179>/Gain1'
                                      */
  real_T Constant1_Value_h;          /* Expression: 1
                                      * Referenced by: '<S179>/Constant1'
                                      */
  real_T Gain2_Gain_c;               /* Expression: 4
                                      * Referenced by: '<S179>/Gain2'
                                      */
  real_T Gain_Gain_cl;               /* Expression: .5
                                      * Referenced by: '<S179>/Gain'
                                      */
  real_T Constant_Value_ae;          /* Expression: 1
                                      * Referenced by: '<S178>/Constant'
                                      */
  real_T Constant_Value_mr;          /* Expression: 1
                                      * Referenced by: '<S175>/Constant'
                                      */
  real_T Switch_Threshold_n;         /* Expression: 0
                                      * Referenced by: '<S175>/Switch'
                                      */
  real_T Gain1_Gain_j;               /* Expression: 20
                                      * Referenced by: '<S181>/Gain1'
                                      */
  real_T Constant1_Value_ln;         /* Expression: 1
                                      * Referenced by: '<S181>/Constant1'
                                      */
  real_T Gain2_Gain_m;               /* Expression: 4
                                      * Referenced by: '<S181>/Gain2'
                                      */
  real_T Gain_Gain_p;                /* Expression: .5
                                      * Referenced by: '<S181>/Gain'
                                      */
  real_T Constant_Value_fi;          /* Expression: 1
                                      * Referenced by: '<S180>/Constant'
                                      */
  real_T Integrator_IC_l;            /* Expression: 0
                                      * Referenced by: '<S161>/Integrator'
                                      */
  real_T Constant_Value_by;          /* Expression: shaftSwitchMask
                                      * Referenced by: '<S168>/Constant'
                                      */
  real_T Constant2_Value_a;          /* Expression: 1
                                      * Referenced by: '<S168>/Constant2'
                                      */
  real_T Switch_Threshold_cr;        /* Expression: 1
                                      * Referenced by: '<S168>/Switch'
                                      */
  real_T Gain_Gain_d;                /* Expression: 4
                                      * Referenced by: '<S173>/Gain'
                                      */
  real_T upi_Gain_p;                 /* Expression: 2*pi
                                      * Referenced by: '<S185>/2*pi'
                                      */
  real_T Memory_InitialCondition_nv; /* Expression: 0
                                      * Referenced by: '<S220>/Memory'
                                      */
  real_T Constant1_Value_p;          /* Expression: 1
                                      * Referenced by: '<S191>/Constant1'
                                      */
  real_T Constant_Value_os;          /* Expression: shaftSwitchMask
                                      * Referenced by: '<S191>/Constant'
                                      */
  real_T Switch_Threshold_a5;        /* Expression: 1
                                      * Referenced by: '<S191>/Switch'
                                      */
  real_T Integrator_UpperSat_a;      /* Expression: maxAbsSpd
                                      * Referenced by: '<S185>/Integrator'
                                      */
  real_T Integrator_LowerSat_j;      /* Expression: -maxAbsSpd
                                      * Referenced by: '<S185>/Integrator'
                                      */
  real_T Gain1_Gain_o;               /* Expression: 1/2
                                      * Referenced by: '<S191>/Gain1'
                                      */
  real_T Constant_Value_l2;          /* Expression: 1
                                      * Referenced by: '<S202>/Constant'
                                      */
  real_T Switch_Threshold_i;         /* Expression: 0
                                      * Referenced by: '<S202>/Switch'
                                      */
  real_T Gain1_Gain_n;               /* Expression: 20
                                      * Referenced by: '<S209>/Gain1'
                                      */
  real_T Constant1_Value_hd;         /* Expression: 1
                                      * Referenced by: '<S209>/Constant1'
                                      */
  real_T Gain2_Gain_mk;              /* Expression: 4
                                      * Referenced by: '<S209>/Gain2'
                                      */
  real_T Gain_Gain_f1;               /* Expression: .5
                                      * Referenced by: '<S209>/Gain'
                                      */
  real_T Constant_Value_lo;          /* Expression: 1
                                      * Referenced by: '<S208>/Constant'
                                      */
  real_T Constant_Value_d;           /* Expression: 1
                                      * Referenced by: '<S200>/Constant'
                                      */
  real_T Switch_Threshold_dt;        /* Expression: 0
                                      * Referenced by: '<S200>/Switch'
                                      */
  real_T Gain1_Gain_de;              /* Expression: 20
                                      * Referenced by: '<S205>/Gain1'
                                      */
  real_T Constant1_Value_e4;         /* Expression: 1
                                      * Referenced by: '<S205>/Constant1'
                                      */
  real_T Gain2_Gain_os;              /* Expression: 4
                                      * Referenced by: '<S205>/Gain2'
                                      */
  real_T Gain_Gain_g1;               /* Expression: .5
                                      * Referenced by: '<S205>/Gain'
                                      */
  real_T Constant_Value_ft;          /* Expression: 1
                                      * Referenced by: '<S204>/Constant'
                                      */
  real_T Constant_Value_g;           /* Expression: 1
                                      * Referenced by: '<S201>/Constant'
                                      */
  real_T Switch_Threshold_e;         /* Expression: 0
                                      * Referenced by: '<S201>/Switch'
                                      */
  real_T Gain1_Gain_hc;              /* Expression: 20
                                      * Referenced by: '<S207>/Gain1'
                                      */
  real_T Constant1_Value_d;          /* Expression: 1
                                      * Referenced by: '<S207>/Constant1'
                                      */
  real_T Gain2_Gain_d;               /* Expression: 4
                                      * Referenced by: '<S207>/Gain2'
                                      */
  real_T Gain_Gain_le;               /* Expression: .5
                                      * Referenced by: '<S207>/Gain'
                                      */
  real_T Constant_Value_jf;          /* Expression: 1
                                      * Referenced by: '<S206>/Constant'
                                      */
  real_T Integrator_IC_d;            /* Expression: 0
                                      * Referenced by: '<S187>/Integrator'
                                      */
  real_T Constant_Value_fy;          /* Expression: shaftSwitchMask
                                      * Referenced by: '<S194>/Constant'
                                      */
  real_T Constant2_Value_at;         /* Expression: 1
                                      * Referenced by: '<S194>/Constant2'
                                      */
  real_T Switch_Threshold_ih;        /* Expression: 1
                                      * Referenced by: '<S194>/Switch'
                                      */
  real_T Gain_Gain_b;                /* Expression: 4
                                      * Referenced by: '<S199>/Gain'
                                      */
  real_T Reset_Value_d;              /* Expression: 1
                                      * Referenced by: '<S212>/Reset'
                                      */
  real_T Reset_Value_fu;             /* Expression: 1
                                      * Referenced by: '<S220>/Reset'
                                      */
  real_T Reset_Value_n;              /* Expression: 1
                                      * Referenced by: '<S228>/Reset'
                                      */
  real_T Reset_Value_c;              /* Expression: 1
                                      * Referenced by: '<S236>/Reset'
                                      */
  real_T Reset_Value_i;              /* Expression: 1
                                      * Referenced by: '<S244>/Reset'
                                      */
  real_T Reset_Value_f3;             /* Expression: 1
                                      * Referenced by: '<S252>/Reset'
                                      */
  real_T Gain1_Gain_c;               /* Expression: 1/2
                                      * Referenced by: '<S262>/Gain1'
                                      */
  real_T Constant_Value_jo;          /* Expression: 1
                                      * Referenced by: '<S272>/Constant'
                                      */
  real_T Switch_Threshold_j;         /* Expression: 0
                                      * Referenced by: '<S272>/Switch'
                                      */
  real_T Gain1_Gain_bu;              /* Expression: 20
                                      * Referenced by: '<S279>/Gain1'
                                      */
  real_T Constant1_Value_ff;         /* Expression: 1
                                      * Referenced by: '<S279>/Constant1'
                                      */
  real_T Gain2_Gain_f;               /* Expression: 4
                                      * Referenced by: '<S279>/Gain2'
                                      */
  real_T Gain_Gain_d2;               /* Expression: .5
                                      * Referenced by: '<S279>/Gain'
                                      */
  real_T Constant_Value_hk;          /* Expression: 1
                                      * Referenced by: '<S278>/Constant'
                                      */
  real_T Constant_Value_am;          /* Expression: 1
                                      * Referenced by: '<S270>/Constant'
                                      */
  real_T Switch_Threshold_fm;        /* Expression: 0
                                      * Referenced by: '<S270>/Switch'
                                      */
  real_T Gain1_Gain_fc;              /* Expression: 20
                                      * Referenced by: '<S275>/Gain1'
                                      */
  real_T Constant1_Value_a;          /* Expression: 1
                                      * Referenced by: '<S275>/Constant1'
                                      */
  real_T Gain2_Gain_og;              /* Expression: 4
                                      * Referenced by: '<S275>/Gain2'
                                      */
  real_T Gain_Gain_po;               /* Expression: .5
                                      * Referenced by: '<S275>/Gain'
                                      */
  real_T Constant_Value_fo;          /* Expression: 1
                                      * Referenced by: '<S274>/Constant'
                                      */
  real_T Constant_Value_mj;          /* Expression: 1
                                      * Referenced by: '<S271>/Constant'
                                      */
  real_T Switch_Threshold_dtz;       /* Expression: 0
                                      * Referenced by: '<S271>/Switch'
                                      */
  real_T Gain1_Gain_cd;              /* Expression: 20
                                      * Referenced by: '<S277>/Gain1'
                                      */
  real_T Constant1_Value_gs;         /* Expression: 1
                                      * Referenced by: '<S277>/Constant1'
                                      */
  real_T Gain2_Gain_ek;              /* Expression: 4
                                      * Referenced by: '<S277>/Gain2'
                                      */
  real_T Gain_Gain_kj;               /* Expression: .5
                                      * Referenced by: '<S277>/Gain'
                                      */
  real_T Constant_Value_dn;          /* Expression: 1
                                      * Referenced by: '<S276>/Constant'
                                      */
  real_T Constant6_Value;            /* Expression: 1
                                      * Referenced by: '<S265>/Constant6'
                                      */
  real_T Switch1_Threshold;          /* Expression: 1
                                      * Referenced by: '<S265>/Switch1'
                                      */
  real_T upi_Gain_b;                 /* Expression: 2*pi
                                      * Referenced by: '<S284>/2*pi'
                                      */
  real_T Gain1_Gain_nn;              /* Expression: 1/2
                                      * Referenced by: '<S290>/Gain1'
                                      */
  real_T Constant_Value_jd;          /* Expression: 1
                                      * Referenced by: '<S301>/Constant'
                                      */
  real_T Switch_Threshold_nk;        /* Expression: 0
                                      * Referenced by: '<S301>/Switch'
                                      */
  real_T Gain1_Gain_hh;              /* Expression: 20
                                      * Referenced by: '<S308>/Gain1'
                                      */
  real_T Constant1_Value_hv;         /* Expression: 1
                                      * Referenced by: '<S308>/Constant1'
                                      */
  real_T Gain2_Gain_fz;              /* Expression: 4
                                      * Referenced by: '<S308>/Gain2'
                                      */
  real_T Gain_Gain_a;                /* Expression: .5
                                      * Referenced by: '<S308>/Gain'
                                      */
  real_T Constant_Value_gb;          /* Expression: 1
                                      * Referenced by: '<S307>/Constant'
                                      */
  real_T Constant_Value_e;           /* Expression: 1
                                      * Referenced by: '<S299>/Constant'
                                      */
  real_T Switch_Threshold_gb;        /* Expression: 0
                                      * Referenced by: '<S299>/Switch'
                                      */
  real_T Gain1_Gain_de0;             /* Expression: 20
                                      * Referenced by: '<S304>/Gain1'
                                      */
  real_T Constant1_Value_fb;         /* Expression: 1
                                      * Referenced by: '<S304>/Constant1'
                                      */
  real_T Gain2_Gain_nt;              /* Expression: 4
                                      * Referenced by: '<S304>/Gain2'
                                      */
  real_T Gain_Gain_h;                /* Expression: .5
                                      * Referenced by: '<S304>/Gain'
                                      */
  real_T Constant_Value_hz;          /* Expression: 1
                                      * Referenced by: '<S303>/Constant'
                                      */
  real_T Constant_Value_as;          /* Expression: 1
                                      * Referenced by: '<S300>/Constant'
                                      */
  real_T Switch_Threshold_ch;        /* Expression: 0
                                      * Referenced by: '<S300>/Switch'
                                      */
  real_T Gain1_Gain_k;               /* Expression: 20
                                      * Referenced by: '<S306>/Gain1'
                                      */
  real_T Constant1_Value_dz;         /* Expression: 1
                                      * Referenced by: '<S306>/Constant1'
                                      */
  real_T Gain2_Gain_g;               /* Expression: 4
                                      * Referenced by: '<S306>/Gain2'
                                      */
  real_T Gain_Gain_o;                /* Expression: .5
                                      * Referenced by: '<S306>/Gain'
                                      */
  real_T Constant_Value_i1s;         /* Expression: 1
                                      * Referenced by: '<S305>/Constant'
                                      */
  real_T Integrator_IC_f;            /* Expression: 0
                                      * Referenced by: '<S286>/Integrator'
                                      */
  real_T Constant_Value_ed;          /* Expression: shaftSwitchMask
                                      * Referenced by: '<S293>/Constant'
                                      */
  real_T Constant2_Value_j;          /* Expression: 1
                                      * Referenced by: '<S293>/Constant2'
                                      */
  real_T Switch_Threshold_od;        /* Expression: 1
                                      * Referenced by: '<S293>/Switch'
                                      */
  real_T Gain_Gain_d2j;              /* Expression: 4
                                      * Referenced by: '<S298>/Gain'
                                      */
  real_T Reset_Value_m;              /* Expression: 1
                                      * Referenced by: '<S311>/Reset'
                                      */
  real_T Reset_Value_fz;             /* Expression: 1
                                      * Referenced by: '<S319>/Reset'
                                      */
  real_T Integrator1_IC;             /* Expression: 0
                                      * Referenced by: '<S382>/Integrator1'
                                      */
  real_T Constant1_Value_bi;         /* Expression: 1e-2
                                      * Referenced by: '<S379>/Constant1'
                                      */
  real_T rpm_Gain;                   /* Expression: 1.0/2.0/pi*60
                                      * Referenced by: '<S100>/rpm'
                                      */
  real_T Constant_Value_kh;          /* Expression: 600
                                      * Referenced by: '<S379>/Constant'
                                      */
  real_T Constant2_Value_k;          /* Expression: 600
                                      * Referenced by: '<S379>/Constant2'
                                      */
  real_T HoldSecond_Value;           /* Expression: 0.5
                                      * Referenced by: '<S98>/HoldSecond'
                                      */
  real_T upi_Gain_a4;                /* Expression: 2*pi
                                      * Referenced by: '<S331>/2*pi'
                                      */
  real_T Reset_Value_mz;             /* Expression: 1
                                      * Referenced by: '<S333>/Reset'
                                      */
  real_T Merge2_1_InitialOutput;     /* Computed Parameter: Merge2_1_InitialOutput
                                      * Referenced by: '<S331>/Merge2'
                                      */
  real_T Merge2_2_InitialOutput;     /* Computed Parameter: Merge2_2_InitialOutput
                                      * Referenced by: '<S331>/Merge2'
                                      */
  real_T Merge2_3_InitialOutput;     /* Computed Parameter: Merge2_3_InitialOutput
                                      * Referenced by: '<S331>/Merge2'
                                      */
  real_T Reset_Value_g;              /* Expression: 1
                                      * Referenced by: '<S370>/Reset'
                                      */
  real_T kph_Gain;                   /* Expression: 3.6
                                      * Referenced by: '<S377>/kph'
                                      */
  real_T Constant_Value_mx;          /* Expression: 0
                                      * Referenced by: '<S2>/Constant'
                                      */
  real_T Constant1_Value_pn;         /* Expression: 0
                                      * Referenced by: '<S2>/Constant1'
                                      */
  real_T Memory_InitialCondition_d;  /* Expression: 0
                                      * Referenced by: '<S45>/Memory'
                                      */
  real_T Memory_InitialCondition_m;  /* Expression: 0
                                      * Referenced by: '<S37>/Memory'
                                      */
  real_T Memory_InitialCondition_o3; /* Expression: 0
                                      * Referenced by: '<S82>/Memory'
                                      */
  real_T Memory_InitialCondition_is; /* Expression: 0
                                      * Referenced by: '<S90>/Memory'
                                      */
  real_T Constant2_Value_g;          /* Expression: 0
                                      * Referenced by: '<S2>/Constant2'
                                      */
  real_T Constant1_Value_nb;         /* Expression: 1
                                      * Referenced by: '<S16>/Constant1'
                                      */
  real_T Constant_Value_c;           /* Expression: shaftSwitchMask
                                      * Referenced by: '<S16>/Constant'
                                      */
  real_T Switch_Threshold_ce;        /* Expression: 1
                                      * Referenced by: '<S16>/Switch'
                                      */
  real_T Integrator_UpperSat_d;      /* Expression: maxAbsSpd
                                      * Referenced by: '<S10>/Integrator'
                                      */
  real_T Integrator_LowerSat_o;      /* Expression: -maxAbsSpd
                                      * Referenced by: '<S10>/Integrator'
                                      */
  real_T Constant_Value_jz;          /* Expression: 0
                                      * Referenced by: '<S6>/Constant'
                                      */
  real_T Constant_Value_mw;          /* Expression: 0
                                      * Referenced by: '<S5>/Constant'
                                      */
  real_T Constant1_Value_e4b;        /* Expression: 1
                                      * Referenced by: '<S61>/Constant1'
                                      */
  real_T Constant_Value_ll;          /* Expression: shaftSwitchMask
                                      * Referenced by: '<S61>/Constant'
                                      */
  real_T Switch_Threshold_nf;        /* Expression: 1
                                      * Referenced by: '<S61>/Switch'
                                      */
  real_T Integrator_UpperSat_do;     /* Expression: maxAbsSpd
                                      * Referenced by: '<S55>/Integrator'
                                      */
  real_T Integrator_LowerSat_a;      /* Expression: -maxAbsSpd
                                      * Referenced by: '<S55>/Integrator'
                                      */
  real_T upi_Gain_m;                 /* Expression: 2*pi
                                      * Referenced by: '<S10>/2*pi'
                                      */
  real_T Gain1_Gain_p;               /* Expression: 1/2
                                      * Referenced by: '<S16>/Gain1'
                                      */
  real_T Constant_Value_ju;          /* Expression: 1
                                      * Referenced by: '<S27>/Constant'
                                      */
  real_T Switch_Threshold_m;         /* Expression: 0
                                      * Referenced by: '<S27>/Switch'
                                      */
  real_T Gain1_Gain_f5;              /* Expression: 20
                                      * Referenced by: '<S34>/Gain1'
                                      */
  real_T Constant1_Value_bl;         /* Expression: 1
                                      * Referenced by: '<S34>/Constant1'
                                      */
  real_T Gain2_Gain_fo;              /* Expression: 4
                                      * Referenced by: '<S34>/Gain2'
                                      */
  real_T Gain_Gain_hb;               /* Expression: .5
                                      * Referenced by: '<S34>/Gain'
                                      */
  real_T Constant_Value_n3;          /* Expression: 1
                                      * Referenced by: '<S33>/Constant'
                                      */
  real_T Constant_Value_j1;          /* Expression: 1
                                      * Referenced by: '<S25>/Constant'
                                      */
  real_T Switch_Threshold_cl;        /* Expression: 0
                                      * Referenced by: '<S25>/Switch'
                                      */
  real_T Gain1_Gain_l;               /* Expression: 20
                                      * Referenced by: '<S30>/Gain1'
                                      */
  real_T Constant1_Value_pb;         /* Expression: 1
                                      * Referenced by: '<S30>/Constant1'
                                      */
  real_T Gain2_Gain_mq;              /* Expression: 4
                                      * Referenced by: '<S30>/Gain2'
                                      */
  real_T Gain_Gain_dr;               /* Expression: .5
                                      * Referenced by: '<S30>/Gain'
                                      */
  real_T Constant_Value_er;          /* Expression: 1
                                      * Referenced by: '<S29>/Constant'
                                      */
  real_T Constant_Value_db;          /* Expression: 1
                                      * Referenced by: '<S26>/Constant'
                                      */
  real_T Switch_Threshold_n1;        /* Expression: 0
                                      * Referenced by: '<S26>/Switch'
                                      */
  real_T Gain1_Gain_cf;              /* Expression: 20
                                      * Referenced by: '<S32>/Gain1'
                                      */
  real_T Constant1_Value_oj;         /* Expression: 1
                                      * Referenced by: '<S32>/Constant1'
                                      */
  real_T Gain2_Gain_dk;              /* Expression: 4
                                      * Referenced by: '<S32>/Gain2'
                                      */
  real_T Gain_Gain_ag;               /* Expression: .5
                                      * Referenced by: '<S32>/Gain'
                                      */
  real_T Constant_Value_ni;          /* Expression: 1
                                      * Referenced by: '<S31>/Constant'
                                      */
  real_T Integrator_IC_lt;           /* Expression: 0
                                      * Referenced by: '<S12>/Integrator'
                                      */
  real_T Constant_Value_e3;          /* Expression: shaftSwitchMask
                                      * Referenced by: '<S19>/Constant'
                                      */
  real_T Constant2_Value_o;          /* Expression: 1
                                      * Referenced by: '<S19>/Constant2'
                                      */
  real_T Switch_Threshold_p0;        /* Expression: 1
                                      * Referenced by: '<S19>/Switch'
                                      */
  real_T Gain_Gain_o2;               /* Expression: 4
                                      * Referenced by: '<S24>/Gain'
                                      */
  real_T Reset_Value_j;              /* Expression: 1
                                      * Referenced by: '<S37>/Reset'
                                      */
  real_T Reset_Value_g5;             /* Expression: 1
                                      * Referenced by: '<S45>/Reset'
                                      */
  real_T upi_Gain_o;                 /* Expression: 2*pi
                                      * Referenced by: '<S55>/2*pi'
                                      */
  real_T Gain1_Gain_jj;              /* Expression: 1/2
                                      * Referenced by: '<S61>/Gain1'
                                      */
  real_T Constant_Value_na;          /* Expression: 1
                                      * Referenced by: '<S72>/Constant'
                                      */
  real_T Switch_Threshold_mh;        /* Expression: 0
                                      * Referenced by: '<S72>/Switch'
                                      */
  real_T Gain1_Gain_ds;              /* Expression: 20
                                      * Referenced by: '<S79>/Gain1'
                                      */
  real_T Constant1_Value_k;          /* Expression: 1
                                      * Referenced by: '<S79>/Constant1'
                                      */
  real_T Gain2_Gain_ny;              /* Expression: 4
                                      * Referenced by: '<S79>/Gain2'
                                      */
  real_T Gain_Gain_fq;               /* Expression: .5
                                      * Referenced by: '<S79>/Gain'
                                      */
  real_T Constant_Value_l1;          /* Expression: 1
                                      * Referenced by: '<S78>/Constant'
                                      */
  real_T Constant_Value_h2;          /* Expression: 1
                                      * Referenced by: '<S70>/Constant'
                                      */
  real_T Switch_Threshold_ly;        /* Expression: 0
                                      * Referenced by: '<S70>/Switch'
                                      */
  real_T Gain1_Gain_ph;              /* Expression: 20
                                      * Referenced by: '<S75>/Gain1'
                                      */
  real_T Constant1_Value_nv;         /* Expression: 1
                                      * Referenced by: '<S75>/Constant1'
                                      */
  real_T Gain2_Gain_en;              /* Expression: 4
                                      * Referenced by: '<S75>/Gain2'
                                      */
  real_T Gain_Gain_oe;               /* Expression: .5
                                      * Referenced by: '<S75>/Gain'
                                      */
  real_T Constant_Value_nk;          /* Expression: 1
                                      * Referenced by: '<S74>/Constant'
                                      */
  real_T Constant_Value_fu;          /* Expression: 1
                                      * Referenced by: '<S71>/Constant'
                                      */
  real_T Switch_Threshold_gq;        /* Expression: 0
                                      * Referenced by: '<S71>/Switch'
                                      */
  real_T Gain1_Gain_f0;              /* Expression: 20
                                      * Referenced by: '<S77>/Gain1'
                                      */
  real_T Constant1_Value_kc;         /* Expression: 1
                                      * Referenced by: '<S77>/Constant1'
                                      */
  real_T Gain2_Gain_ce;              /* Expression: 4
                                      * Referenced by: '<S77>/Gain2'
                                      */
  real_T Gain_Gain_h2;               /* Expression: .5
                                      * Referenced by: '<S77>/Gain'
                                      */
  real_T Constant_Value_om;          /* Expression: 1
                                      * Referenced by: '<S76>/Constant'
                                      */
  real_T Integrator_IC_k;            /* Expression: 0
                                      * Referenced by: '<S57>/Integrator'
                                      */
  real_T Constant_Value_cw;          /* Expression: shaftSwitchMask
                                      * Referenced by: '<S64>/Constant'
                                      */
  real_T Constant2_Value_p;          /* Expression: 1
                                      * Referenced by: '<S64>/Constant2'
                                      */
  real_T Switch_Threshold_oq;        /* Expression: 1
                                      * Referenced by: '<S64>/Switch'
                                      */
  real_T Gain_Gain_pd;               /* Expression: 4
                                      * Referenced by: '<S69>/Gain'
                                      */
  real_T Reset_Value_o;              /* Expression: 1
                                      * Referenced by: '<S82>/Reset'
                                      */
  real_T Reset_Value_h;              /* Expression: 1
                                      * Referenced by: '<S90>/Reset'
                                      */
  uint32_T CalculateUpshiftThreshold_maxIndex[2];
  /* Computed Parameter: CalculateUpshiftThreshold_maxIndex
   * Referenced by: '<S390>/Calculate Upshift Threshold'
   */
  uint32_T CalculateDownshiftThreshold_maxIndex[2];
  /* Computed Parameter: CalculateDownshiftThreshold_maxIndex
   * Referenced by: '<S389>/Calculate  Downshift Threshold'
   */
  uint32_T CalculateUpshiftThreshold_maxIndex_e[2];
  /* Computed Parameter: CalculateUpshiftThreshold_maxIndex_e
   * Referenced by: '<S388>/Calculate Upshift Threshold'
   */
  uint32_T CalculateDownshiftThreshold_maxIndex_m[2];
  /* Computed Parameter: CalculateDownshiftThreshold_maxIndex_m
   * Referenced by: '<S388>/Calculate  Downshift Threshold'
   */
  P_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential_h; /* '<S55>/Limited Slip Differential' */
  P_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential_l; /* '<S10>/Limited Slip Differential' */
  P_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential_b; /* '<S284>/Limited Slip Differential' */
  P_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential_k; /* '<S185>/Limited Slip Differential' */
  P_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential_i; /* '<S159>/Limited Slip Differential' */
  P_LimitedSlipDifferential_TX_DriveLine_T sf_LimitedSlipDifferential;   /* '<S108>/Limited Slip Differential' */
};

/* Real-time Model Data Structure */
struct tag_RTM_TX_DriveLine_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_TX_DriveLine_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_TX_DriveLine_T *contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[55];
  real_T odeF[4][55];
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

/* Class declaration for model TX_DriveLine */
class MODULE_API TX_DriveLine {
  /* public data and function members */
 public:
  /* Copy Constructor */
  TX_DriveLine(TX_DriveLine const &) = delete;

  /* Assignment Operator */
  TX_DriveLine &operator=(TX_DriveLine const &) & = delete;

  /* Move Constructor */
  TX_DriveLine(TX_DriveLine &&) = delete;

  /* Move Assignment Operator */
  TX_DriveLine &operator=(TX_DriveLine &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_TX_DriveLine_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_TX_DriveLine_T *pExtU_TX_DriveLine_T) { TX_DriveLine_U = *pExtU_TX_DriveLine_T; }

  /* Root outports get method */
  const ExtY_TX_DriveLine_T &getExternalOutputs() const { return TX_DriveLine_Y; }

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
  TX_DriveLine();

  /* Destructor */
  ~TX_DriveLine();

 protected:
  int m_AT_gear_num = 10;  // -1 0 1~8

  int m_AT_shift_gear_num = 7;  //  1->2, 2->3 ...

  int m_upshift_accload_points_num = 4;

  int m_downshift_accload_points_num = 4;

  static P_TX_DriveLine_T TX_DriveLine_P;

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_TX_DriveLine_T TX_DriveLine_U;

  /* External outputs */
  ExtY_TX_DriveLine_T TX_DriveLine_Y;

  /* Block signals */
  B_TX_DriveLine_T TX_DriveLine_B;

  /* Block states */
  DW_TX_DriveLine_T TX_DriveLine_DW;

  /* Tunable parameters */
  // static P_TX_DriveLine_T TX_DriveLine_P;

  /* Block continuous states */
  X_TX_DriveLine_T TX_DriveLine_X;

  /* Triggered events */
  PrevZCX_TX_DriveLine_T TX_DriveLine_PrevZCX;

  /* private member function(s) for subsystem '<S108>/Limited Slip Differential'*/
  void TX_DriveLine_LimitedSlipDifferential(real_T rtu_u, real_T rtu_u_n, real_T rtu_u_o, real_T rtu_u_h,
                                            real_T rtu_bw1, real_T rtu_bd, real_T rtu_bw2, real_T rtu_Ndiff,
                                            real_T rtu_Jd, real_T rtu_Jw1, real_T rtu_Jw2, const real_T rtu_x[2],
                                            B_LimitedSlipDifferential_TX_DriveLine_T *localB,
                                            P_LimitedSlipDifferential_TX_DriveLine_T *localP);
  void TX_DriveLine_automldiffls(const real_T u[4], real_T bw1, real_T bd, real_T bw2, real_T Ndiff, real_T shaftSwitch,
                                 real_T Jd, real_T Jw1, real_T Jw2, const real_T x[2], real_T y[4], real_T xdot[2]);

  /* private member function(s) for subsystem '<Root>'*/
  void TX_DriveLine_GearCmd(void);

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void TX_DriveLine_derivatives();

  /* Real-Time Model */
  RT_MODEL_TX_DriveLine_T TX_DriveLine_M;
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
 * '<Root>' : 'TX_DriveLine'
 * '<S1>'   : 'TX_DriveLine/DriveLine'
 * '<S2>'   : 'TX_DriveLine/DriveLine/EVDriveLine'
 * '<S3>'   : 'TX_DriveLine/DriveLine/ICEDriveLine'
 * '<S4>'   : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine'
 * '<S5>'   : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive'
 * '<S6>'   : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive'
 * '<S7>'   : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential'
 * '<S8>'   : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1'
 * '<S9>'   : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2'
 * '<S10>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential'
 * '<S11>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation'
 * '<S12>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Clutch Response'
 * '<S13>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Coupling Torque'
 * '<S14>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency'
 * '<S15>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Limited Slip Differential'
 * '<S16>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/y'
 * '<S17>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power'
 * '<S18>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Damping Power'
 * '<S19>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Kinetic Power'
 * '<S20>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator'
 * '<S21>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S22>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S23>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S24>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Coupling Torque/Ideal Wet Clutch Coupling'
 * '<S25>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 1 Efficiency'
 * '<S26>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 2 Efficiency'
 * '<S27>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Drive Efficiency'
 * '<S28>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Eta'
 * '<S29>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 1 Efficiency/Low speed blend'
 * '<S30>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 1 Efficiency/Low speed blend/blend'
 * '<S31>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 2 Efficiency/Low speed blend'
 * '<S32>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 2 Efficiency/Low speed blend/blend'
 * '<S33>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Drive Efficiency/Low speed blend'
 * '<S34>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Drive Efficiency/Low speed blend/blend'
 * '<S35>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Eta/Constant Eta'
 * '<S36>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear'
 * '<S37>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Cont LPF IC Dyn'
 * '<S38>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power'
 * '<S39>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S40>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S41>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S42>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S43>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S44>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear'
 * '<S45>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Cont LPF IC Dyn'
 * '<S46>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power'
 * '<S47>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S48>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S49>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S50>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S51>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S52>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential'
 * '<S53>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance'
 * '<S54>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1'
 * '<S55>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential'
 * '<S56>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation'
 * '<S57>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Clutch Response'
 * '<S58>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Coupling Torque'
 * '<S59>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency'
 * '<S60>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Limited Slip Differential'
 * '<S61>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/y'
 * '<S62>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power'
 * '<S63>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Damping Power'
 * '<S64>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Kinetic Power'
 * '<S65>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator'
 * '<S66>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S67>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S68>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S69>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Coupling Torque/Ideal Wet Clutch Coupling'
 * '<S70>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency'
 * '<S71>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency'
 * '<S72>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency'
 * '<S73>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Eta'
 * '<S74>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency/Low speed blend'
 * '<S75>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 1 Efficiency/Low speed blend/blend'
 * '<S76>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency/Low speed blend'
 * '<S77>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Axle 2 Efficiency/Low speed blend/blend'
 * '<S78>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency/Low speed blend'
 * '<S79>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Drive Efficiency/Low speed blend/blend'
 * '<S80>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/EVRearLimitedSlipDifferential/Limited Slip
 * Differential/Efficiency/Eta/Constant Eta'
 * '<S81>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear'
 * '<S82>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Cont LPF IC Dyn'
 * '<S83>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power'
 * '<S84>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S85>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S86>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S87>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S88>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S89>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear'
 * '<S90>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Cont LPF IC Dyn'
 * '<S91>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power'
 * '<S92>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S93>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S94>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S95>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S96>'  : 'TX_DriveLine/DriveLine/EVDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S97>'  : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine'
 * '<S98>'  : 'TX_DriveLine/DriveLine/ICEDriveLine/HoldGearCmd'
 * '<S99>'  : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission'
 * '<S100>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController'
 * '<S101>' : 'TX_DriveLine/DriveLine/ICEDriveLine/clutch'
 * '<S102>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive'
 * '<S103>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive'
 * '<S104>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive'
 * '<S105>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential'
 * '<S106>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1'
 * '<S107>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2'
 * '<S108>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential'
 * '<S109>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation'
 * '<S110>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Clutch Response'
 * '<S111>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Coupling Torque'
 * '<S112>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency'
 * '<S113>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Limited Slip Differential'
 * '<S114>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/y'
 * '<S115>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power'
 * '<S116>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Damping Power'
 * '<S117>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Kinetic Power'
 * '<S118>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator'
 * '<S119>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S120>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S121>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S122>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Coupling Torque/Ideal Wet Clutch Coupling'
 * '<S123>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 1 Efficiency'
 * '<S124>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 2 Efficiency'
 * '<S125>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Drive Efficiency'
 * '<S126>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Eta'
 * '<S127>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 1 Efficiency/Low speed blend'
 * '<S128>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 1 Efficiency/Low speed blend/blend'
 * '<S129>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 2 Efficiency/Low speed blend'
 * '<S130>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 2 Efficiency/Low speed blend/blend'
 * '<S131>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Drive Efficiency/Low speed blend'
 * '<S132>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Drive Efficiency/Low speed blend/blend'
 * '<S133>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/ICEFrontLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Eta/Constant Eta'
 * '<S134>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear'
 * '<S135>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Cont LPF IC Dyn'
 * '<S136>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power'
 * '<S137>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Damping Power'
 * '<S138>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator'
 * '<S139>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S140>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S141>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S142>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear'
 * '<S143>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Cont LPF IC Dyn'
 * '<S144>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power'
 * '<S145>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Damping Power'
 * '<S146>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator'
 * '<S147>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S148>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S149>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S150>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/4WD_ICEFrontLimitedSlipDifferential'
 * '<S151>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/4WD_ICERearLimitedSlipDifferential'
 * '<S152>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance'
 * '<S153>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance1'
 * '<S154>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance2'
 * '<S155>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance3'
 * '<S156>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance4'
 * '<S157>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance5'
 * '<S158>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case'
 * '<S159>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential'
 * '<S160>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation'
 * '<S161>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Clutch Response'
 * '<S162>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Coupling Torque'
 * '<S163>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency'
 * '<S164>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Limited Slip Differential'
 * '<S165>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/y'
 * '<S166>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power'
 * '<S167>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Damping Power'
 * '<S168>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Kinetic Power'
 * '<S169>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus Creator'
 * '<S170>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrNotTrnsfrd Input'
 * '<S171>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrStored Input'
 * '<S172>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrTrnsfrd Input'
 * '<S173>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Coupling Torque/Ideal Wet Clutch Coupling'
 * '<S174>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 1 Efficiency'
 * '<S175>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 2 Efficiency'
 * '<S176>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Drive Efficiency'
 * '<S177>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Eta'
 * '<S178>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 1 Efficiency/Low speed blend'
 * '<S179>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 1 Efficiency/Low speed
 * blend/blend'
 * '<S180>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 2 Efficiency/Low speed blend'
 * '<S181>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 2 Efficiency/Low speed
 * blend/blend'
 * '<S182>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Drive Efficiency/Low speed blend'
 * '<S183>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Drive Efficiency/Low speed
 * blend/blend'
 * '<S184>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICEFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Eta/Constant Eta'
 * '<S185>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential'
 * '<S186>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Bus Creation'
 * '<S187>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Clutch Response'
 * '<S188>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Coupling Torque'
 * '<S189>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency'
 * '<S190>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Limited Slip Differential'
 * '<S191>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/y'
 * '<S192>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power'
 * '<S193>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Damping Power'
 * '<S194>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Kinetic Power'
 * '<S195>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus Creator'
 * '<S196>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrNotTrnsfrd Input'
 * '<S197>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrStored Input'
 * '<S198>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrTrnsfrd Input'
 * '<S199>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Coupling Torque/Ideal Wet Clutch Coupling'
 * '<S200>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 1 Efficiency'
 * '<S201>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 2 Efficiency'
 * '<S202>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency/Drive Efficiency'
 * '<S203>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency/Eta'
 * '<S204>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 1 Efficiency/Low speed blend'
 * '<S205>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 1 Efficiency/Low speed
 * blend/blend'
 * '<S206>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 2 Efficiency/Low speed blend'
 * '<S207>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 2 Efficiency/Low speed
 * blend/blend'
 * '<S208>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency/Drive Efficiency/Low speed blend'
 * '<S209>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency/Drive Efficiency/Low speed blend/blend'
 * '<S210>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel
 * Drive/4WD_ICERearLimitedSlipDifferential/Limited Slip Differential/Efficiency/Eta/Constant Eta'
 * '<S211>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional
 * Compliance Linear'
 * '<S212>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional
 * Compliance Linear/Cont LPF IC Dyn'
 * '<S213>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional
 * Compliance Linear/Power'
 * '<S214>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional
 * Compliance Linear/Power/Damping Power'
 * '<S215>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator'
 * '<S216>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S217>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S218>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S219>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear'
 * '<S220>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Cont LPF IC Dyn'
 * '<S221>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power'
 * '<S222>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Damping Power'
 * '<S223>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator'
 * '<S224>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S225>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S226>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S227>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear'
 * '<S228>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Cont LPF IC Dyn'
 * '<S229>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power'
 * '<S230>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Damping Power'
 * '<S231>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator'
 * '<S232>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S233>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S234>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S235>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional
 * Compliance Linear'
 * '<S236>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional
 * Compliance Linear/Cont LPF IC Dyn'
 * '<S237>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional
 * Compliance Linear/Power'
 * '<S238>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional
 * Compliance Linear/Power/Damping Power'
 * '<S239>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator'
 * '<S240>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S241>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S242>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance3/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S243>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional
 * Compliance Linear'
 * '<S244>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional
 * Compliance Linear/Cont LPF IC Dyn'
 * '<S245>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional
 * Compliance Linear/Power'
 * '<S246>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional
 * Compliance Linear/Power/Damping Power'
 * '<S247>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator'
 * '<S248>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S249>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S250>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance4/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S251>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional
 * Compliance Linear'
 * '<S252>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional
 * Compliance Linear/Cont LPF IC Dyn'
 * '<S253>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional
 * Compliance Linear/Power'
 * '<S254>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional
 * Compliance Linear/Power/Damping Power'
 * '<S255>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator'
 * '<S256>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S257>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S258>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Torsional Compliance5/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S259>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation'
 * '<S260>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency'
 * '<S261>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/TransferCase'
 * '<S262>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/y'
 * '<S263>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power'
 * '<S264>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Bus
 * Creation/Power/Damping Power'
 * '<S265>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Bus
 * Creation/Power/Kinetic Power'
 * '<S266>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power/Power
 * Accounting Bus Creator'
 * '<S267>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power/Power
 * Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S268>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power/Power
 * Accounting Bus Creator/PwrStored Input'
 * '<S269>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Bus Creation/Power/Power
 * Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S270>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 1
 * Efficiency'
 * '<S271>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 2
 * Efficiency'
 * '<S272>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Drive
 * Efficiency'
 * '<S273>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Eta'
 * '<S274>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 1
 * Efficiency/Low speed blend'
 * '<S275>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 1
 * Efficiency/Low speed blend/blend'
 * '<S276>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 2
 * Efficiency/Low speed blend'
 * '<S277>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Axle 2
 * Efficiency/Low speed blend/blend'
 * '<S278>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Drive
 * Efficiency/Low speed blend'
 * '<S279>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Drive
 * Efficiency/Low speed blend/blend'
 * '<S280>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/ICE All Wheel Drive/Transfer Case/Efficiency/Eta/Constant
 * Eta'
 * '<S281>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential'
 * '<S282>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance'
 * '<S283>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1'
 * '<S284>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential'
 * '<S285>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation'
 * '<S286>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Clutch Response'
 * '<S287>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Coupling Torque'
 * '<S288>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency'
 * '<S289>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Limited Slip Differential'
 * '<S290>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/y'
 * '<S291>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power'
 * '<S292>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Damping Power'
 * '<S293>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Kinetic Power'
 * '<S294>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator'
 * '<S295>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S296>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S297>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Bus Creation/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S298>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Coupling Torque/Ideal Wet Clutch Coupling'
 * '<S299>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 1 Efficiency'
 * '<S300>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 2 Efficiency'
 * '<S301>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Drive Efficiency'
 * '<S302>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Eta'
 * '<S303>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 1 Efficiency/Low speed blend'
 * '<S304>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 1 Efficiency/Low speed blend/blend'
 * '<S305>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 2 Efficiency/Low speed blend'
 * '<S306>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Axle 2 Efficiency/Low speed blend/blend'
 * '<S307>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Drive Efficiency/Low speed blend'
 * '<S308>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Drive Efficiency/Low speed blend/blend'
 * '<S309>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/ICERearLimitedSlipDifferential/Limited
 * Slip Differential/Efficiency/Eta/Constant Eta'
 * '<S310>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear'
 * '<S311>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Cont LPF IC Dyn'
 * '<S312>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power'
 * '<S313>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S314>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S315>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S316>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S317>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S318>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear'
 * '<S319>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Cont LPF IC Dyn'
 * '<S320>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power'
 * '<S321>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S322>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator'
 * '<S323>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S324>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S325>' : 'TX_DriveLine/DriveLine/ICEDriveLine/DriveLine/Rear Wheel Drive/Torsional Compliance1/Torsional Compliance
 * Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S326>' : 'TX_DriveLine/DriveLine/ICEDriveLine/HoldGearCmd/GearCmdChange'
 * '<S327>' : 'TX_DriveLine/DriveLine/ICEDriveLine/HoldGearCmd/hold'
 * '<S328>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal'
 * '<S329>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission'
 * '<S330>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Prop Shaft'
 * '<S331>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission'
 * '<S332>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation'
 * '<S333>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Cont LPF IC Dyn'
 * '<S334>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/IC tunable'
 * '<S335>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked'
 * '<S336>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Neutral IC'
 * '<S337>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked'
 * '<S338>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/input IC'
 * '<S339>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/Power Accounting Bus Creator'
 * '<S340>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/div0protect - abs poly1'
 * '<S341>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S342>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/Power Accounting Bus Creator/PwrStored Input'
 * '<S343>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S344>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/div0protect - abs poly1/Compare To Constant'
 * '<S345>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Bus Creation/div0protect - abs poly1/Compare To Constant1'
 * '<S346>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency'
 * '<S347>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency '
 * '<S348>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Mechanical Efficiency'
 * '<S349>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/gear2props'
 * '<S350>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency/Low speed blend'
 * '<S351>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency/Low speed blend/blend'
 * '<S352>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency /Low speed blend'
 * '<S353>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Apply Efficiency /Low speed blend/blend'
 * '<S354>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Mechanical Efficiency/div0protect - abs poly'
 * '<S355>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Mechanical Efficiency/div0protect - abs poly/Compare To Constant'
 * '<S356>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/Mechanical Efficiency/div0protect - abs poly/Compare To Constant1'
 * '<S357>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/gear2props/Eta Lookup'
 * '<S358>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Locked/gear2props/Eta Lookup/Eta 1D'
 * '<S359>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Apply Efficiency'
 * '<S360>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Mechanical Efficiency'
 * '<S361>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/gear2props'
 * '<S362>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Apply Efficiency/Low speed blend'
 * '<S363>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Apply Efficiency/Low speed blend/blend'
 * '<S364>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Mechanical Efficiency/div0protect - abs poly'
 * '<S365>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Mechanical Efficiency/div0protect - abs poly/Compare To Constant'
 * '<S366>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/Mechanical Efficiency/div0protect - abs poly/Compare To Constant1'
 * '<S367>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/gear2props/Eta Lookup'
 * '<S368>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Ideal Fixed Gear Transmission/Ideal Fixed Gear
 * Transmission/Unlocked/gear2props/Eta Lookup/Eta 1D'
 * '<S369>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Prop Shaft/Torsional Compliance Linear'
 * '<S370>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Prop Shaft/Torsional Compliance Linear/Cont LPF IC
 * Dyn'
 * '<S371>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Prop Shaft/Torsional Compliance Linear/Power'
 * '<S372>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Prop Shaft/Torsional Compliance
 * Linear/Power/Damping Power'
 * '<S373>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Prop Shaft/Torsional Compliance Linear/Power/Power
 * Accounting Bus Creator'
 * '<S374>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Prop Shaft/Torsional Compliance Linear/Power/Power
 * Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S375>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Prop Shaft/Torsional Compliance Linear/Power/Power
 * Accounting Bus Creator/PwrStored Input'
 * '<S376>' : 'TX_DriveLine/DriveLine/ICEDriveLine/Transmission/Ideal/Prop Shaft/Torsional Compliance Linear/Power/Power
 * Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S377>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM'
 * '<S378>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/DriveSelect'
 * '<S379>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/Neutral Select'
 * '<S380>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/ReverseSelect'
 * '<S381>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/TCM Shift Controller'
 * '<S382>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/Valve'
 * '<S383>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/DriveSelect/Compare To Constant'
 * '<S384>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/DriveSelect/Compare To Constant1'
 * '<S385>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/Neutral Select/Compare To Constant'
 * '<S386>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/ReverseSelect/Compare To Constant'
 * '<S387>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/ReverseSelect/Compare To Constant1'
 * '<S388>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/TCM Shift Controller/calc'
 * '<S389>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/TCM Shift Controller/calc_down'
 * '<S390>' : 'TX_DriveLine/DriveLine/ICEDriveLine/TransmissionController/TCM/TCM Shift Controller/calc_up'
 * '<S391>' : 'TX_DriveLine/DriveLine/ICEDriveLine/clutch/Compare To Constant'
 * '<S392>' : 'TX_DriveLine/DriveLine/ICEDriveLine/clutch/Compare To Zero'
 */
#endif /* RTW_HEADER_TX_DriveLine_h_ */
