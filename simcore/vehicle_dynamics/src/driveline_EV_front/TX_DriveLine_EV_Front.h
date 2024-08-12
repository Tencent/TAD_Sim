/*
 * TX_DriveLine_EV_Front.h
 *
 * Code generation for model "TX_DriveLine_EV_Front".
 *
 * Model version              : 1.211
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul 19 10:42:00 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_TX_DriveLine_EV_Front_h_
#define RTW_HEADER_TX_DriveLine_EV_Front_h_
#include <cstring>
#include "TX_DriveLine_EV_Front_types.h"
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

/* Block signals (default storage) */
struct B_TX_DriveLine_EV_Front_T {
  real_T Memory;               /* '<S42>/Memory' */
  real_T domega_o;             /* '<S41>/domega_o' */
  real_T Memory_j;             /* '<S34>/Memory' */
  real_T domega_o_a;           /* '<S33>/domega_o' */
  real_T diffDir;              /* '<S13>/Switch' */
  real_T VectorConcatenate[2]; /* '<S7>/Vector Concatenate' */
  real_T upi;                  /* '<S7>/2*pi' */
  real_T diffDir_j;            /* '<S16>/Switch' */
  real_T Product;              /* '<S9>/Product' */
  real_T Subtract;             /* '<S33>/Subtract' */
  real_T Product_n;            /* '<S34>/Product' */
  real_T Subtract_c;           /* '<S41>/Subtract' */
  real_T Product_c;            /* '<S42>/Product' */
  real_T xdot[2];              /* '<S7>/Limited Slip Differential' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_TX_DriveLine_EV_Front_T {
  real_T Memory_PreviousInput;   /* '<S42>/Memory' */
  real_T Memory_PreviousInput_m; /* '<S34>/Memory' */
  int_T Integrator_IWORK;        /* '<S42>/Integrator' */
  int_T Integrator_IWORK_o;      /* '<S34>/Integrator' */
  int_T Integrator_IWORK_c;      /* '<S7>/Integrator' */
};

/* Continuous states (default storage) */
struct X_TX_DriveLine_EV_Front_T {
  real_T Integrator_CSTATE;      /* '<S42>/Integrator' */
  real_T Integrator_CSTATE_g;    /* '<S41>/Integrator' */
  real_T Integrator_CSTATE_f;    /* '<S34>/Integrator' */
  real_T Integrator_CSTATE_m;    /* '<S33>/Integrator' */
  real_T Integrator_CSTATE_j[2]; /* '<S7>/Integrator' */
  real_T Integrator_CSTATE_i;    /* '<S9>/Integrator' */
};

/* State derivatives (default storage) */
struct XDot_TX_DriveLine_EV_Front_T {
  real_T Integrator_CSTATE;      /* '<S42>/Integrator' */
  real_T Integrator_CSTATE_g;    /* '<S41>/Integrator' */
  real_T Integrator_CSTATE_f;    /* '<S34>/Integrator' */
  real_T Integrator_CSTATE_m;    /* '<S33>/Integrator' */
  real_T Integrator_CSTATE_j[2]; /* '<S7>/Integrator' */
  real_T Integrator_CSTATE_i;    /* '<S9>/Integrator' */
};

/* State disabled  */
struct XDis_TX_DriveLine_EV_Front_T {
  boolean_T Integrator_CSTATE;      /* '<S42>/Integrator' */
  boolean_T Integrator_CSTATE_g;    /* '<S41>/Integrator' */
  boolean_T Integrator_CSTATE_f;    /* '<S34>/Integrator' */
  boolean_T Integrator_CSTATE_m;    /* '<S33>/Integrator' */
  boolean_T Integrator_CSTATE_j[2]; /* '<S7>/Integrator' */
  boolean_T Integrator_CSTATE_i;    /* '<S9>/Integrator' */
};

/* Zero-crossing (trigger) state */
struct PrevZCX_TX_DriveLine_EV_Front_T {
  ZCSigState Integrator_Reset_ZCE;   /* '<S42>/Integrator' */
  ZCSigState Integrator_Reset_ZCE_k; /* '<S34>/Integrator' */
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
struct ExtU_TX_DriveLine_EV_Front_T {
  driveline_in DriveLineIn; /* '<Root>/DriveLineIn' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_TX_DriveLine_EV_Front_T {
  driveline_out DrivelineOut; /* '<Root>/DrivelineOut' */
};

/* Parameters (default storage) */
struct P_TX_DriveLine_EV_Front_T_ {
  struct_WPo6c0btEjgdkiBqVilJ2B VEH; /* Variable: VEH
                                      * Referenced by:
                                      *   '<S7>/Constant'
                                      *   '<S7>/Constant1'
                                      */
  real_T StatLdWhlR[4];              /* Variable: StatLdWhlR
                                      * Referenced by:
                                      *   '<S7>/Constant'
                                      *   '<S7>/Constant1'
                                      */
  real_T front_motor_gear_ratio;     /* Variable: front_motor_gear_ratio
                                      * Referenced by:
                                      *   '<S3>/Gain'
                                      *   '<S3>/Gain1'
                                      */
  real_T ratio_diff_front;           /* Variable: ratio_diff_front
                                      * Referenced by:
                                      *   '<S7>/Ndiff2'
                                      *   '<S13>/Gain'
                                      *   '<S16>/Gain'
                                      */
  real_T EVFrontLimitedSlipDifferential_Fc;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Fc
   * Referenced by: '<S21>/Constant2'
   */
  real_T EVFrontLimitedSlipDifferential_Jd;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Jd
   * Referenced by: '<S7>/Jd'
   */
  real_T EVFrontLimitedSlipDifferential_Jw1;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Jw1
   * Referenced by: '<S7>/Jw1'
   */
  real_T EVFrontLimitedSlipDifferential_Jw2;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Jw2
   * Referenced by: '<S7>/Jw3'
   */
  real_T EVFrontLimitedSlipDifferential_Ndisks;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Ndisks
   * Referenced by: '<S21>/Constant1'
   */
  real_T EVFrontLimitedSlipDifferential_Reff;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_Reff
   * Referenced by: '<S21>/Constant'
   */
  real_T TorsionalCompliance2_b; /* Mask Parameter: TorsionalCompliance2_b
                                  * Referenced by: '<S41>/Gain2'
                                  */
  real_T TorsionalCompliance1_b; /* Mask Parameter: TorsionalCompliance1_b
                                  * Referenced by: '<S33>/Gain2'
                                  */
  real_T EVFrontLimitedSlipDifferential_bd;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_bd
   * Referenced by: '<S7>/bd'
   */
  real_T EVFrontLimitedSlipDifferential_bw1;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_bw1
   * Referenced by: '<S7>/bw1'
   */
  real_T EVFrontLimitedSlipDifferential_bw2;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_bw2
   * Referenced by: '<S7>/bw2'
   */
  real_T TorsionalCompliance2_domega_o;
  /* Mask Parameter: TorsionalCompliance2_domega_o
   * Referenced by: '<S41>/domega_o'
   */
  real_T TorsionalCompliance1_domega_o;
  /* Mask Parameter: TorsionalCompliance1_domega_o
   * Referenced by: '<S33>/domega_o'
   */
  real_T EVFrontLimitedSlipDifferential_dw[8];
  /* Mask Parameter: EVFrontLimitedSlipDifferential_dw
   * Referenced by: '<S21>/mu Table'
   */
  real_T EVFrontLimitedSlipDifferential_eta;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_eta
   * Referenced by: '<S32>/Constant'
   */
  real_T TorsionalCompliance2_k; /* Mask Parameter: TorsionalCompliance2_k
                                  * Referenced by: '<S41>/Gain1'
                                  */
  real_T TorsionalCompliance1_k; /* Mask Parameter: TorsionalCompliance1_k
                                  * Referenced by: '<S33>/Gain1'
                                  */
  real_T EVFrontLimitedSlipDifferential_muc[8];
  /* Mask Parameter: EVFrontLimitedSlipDifferential_muc
   * Referenced by: '<S21>/mu Table'
   */
  real_T TorsionalCompliance1_omega_c;
  /* Mask Parameter: TorsionalCompliance1_omega_c
   * Referenced by: '<S33>/omega_c'
   */
  real_T TorsionalCompliance2_omega_c;
  /* Mask Parameter: TorsionalCompliance2_omega_c
   * Referenced by: '<S41>/omega_c'
   */
  real_T EVFrontLimitedSlipDifferential_tauC;
  /* Mask Parameter: EVFrontLimitedSlipDifferential_tauC
   * Referenced by: '<S7>/Constant3'
   */
  real_T TorsionalCompliance2_theta_o;
  /* Mask Parameter: TorsionalCompliance2_theta_o
   * Referenced by: '<S41>/Integrator'
   */
  real_T TorsionalCompliance1_theta_o;
  /* Mask Parameter: TorsionalCompliance1_theta_o
   * Referenced by: '<S33>/Integrator'
   */
  real_T LimitedSlipDifferential_shaftSwitchMask; /* Expression: shaftSwitchMask
                                                   * Referenced by: '<S7>/Limited Slip Differential'
                                                   */
  real_T Constant_Value;                          /* Expression: 0
                                                   * Referenced by: '<S1>/Constant'
                                                   */
  real_T Constant1_Value;                         /* Expression: 0
                                                   * Referenced by: '<S1>/Constant1'
                                                   */
  real_T Memory_InitialCondition;                 /* Expression: 0
                                                   * Referenced by: '<S42>/Memory'
                                                   */
  real_T Memory_InitialCondition_f;               /* Expression: 0
                                                   * Referenced by: '<S34>/Memory'
                                                   */
  real_T Constant2_Value;                         /* Expression: 0
                                                   * Referenced by: '<S1>/Constant2'
                                                   */
  real_T Constant1_Value_h;                       /* Expression: 1
                                                   * Referenced by: '<S13>/Constant1'
                                                   */
  real_T Constant_Value_n;                        /* Expression: shaftSwitchMask
                                                   * Referenced by: '<S13>/Constant'
                                                   */
  real_T Switch_Threshold;                        /* Expression: 1
                                                   * Referenced by: '<S13>/Switch'
                                                   */
  real_T Integrator_UpperSat;                     /* Expression: maxAbsSpd
                                                   * Referenced by: '<S7>/Integrator'
                                                   */
  real_T Integrator_LowerSat;                     /* Expression: -maxAbsSpd
                                                   * Referenced by: '<S7>/Integrator'
                                                   */
  real_T Constant_Value_b;                        /* Expression: 0
                                                   * Referenced by: '<S3>/Constant'
                                                   */
  real_T upi_Gain;                                /* Expression: 2*pi
                                                   * Referenced by: '<S7>/2*pi'
                                                   */
  real_T Gain1_Gain;                              /* Expression: 1/2
                                                   * Referenced by: '<S13>/Gain1'
                                                   */
  real_T Constant_Value_g;                        /* Expression: 1
                                                   * Referenced by: '<S24>/Constant'
                                                   */
  real_T Switch_Threshold_f;                      /* Expression: 0
                                                   * Referenced by: '<S24>/Switch'
                                                   */
  real_T Gain1_Gain_c;                            /* Expression: 20
                                                   * Referenced by: '<S31>/Gain1'
                                                   */
  real_T Constant1_Value_hc;                      /* Expression: 1
                                                   * Referenced by: '<S31>/Constant1'
                                                   */
  real_T Gain2_Gain;                              /* Expression: 4
                                                   * Referenced by: '<S31>/Gain2'
                                                   */
  real_T Gain_Gain;                               /* Expression: .5
                                                   * Referenced by: '<S31>/Gain'
                                                   */
  real_T Constant_Value_h;                        /* Expression: 1
                                                   * Referenced by: '<S30>/Constant'
                                                   */
  real_T Constant_Value_hv;                       /* Expression: 1
                                                   * Referenced by: '<S22>/Constant'
                                                   */
  real_T Switch_Threshold_e;                      /* Expression: 0
                                                   * Referenced by: '<S22>/Switch'
                                                   */
  real_T Gain1_Gain_d;                            /* Expression: 20
                                                   * Referenced by: '<S27>/Gain1'
                                                   */
  real_T Constant1_Value_o;                       /* Expression: 1
                                                   * Referenced by: '<S27>/Constant1'
                                                   */
  real_T Gain2_Gain_d;                            /* Expression: 4
                                                   * Referenced by: '<S27>/Gain2'
                                                   */
  real_T Gain_Gain_n;                             /* Expression: .5
                                                   * Referenced by: '<S27>/Gain'
                                                   */
  real_T Constant_Value_i;                        /* Expression: 1
                                                   * Referenced by: '<S26>/Constant'
                                                   */
  real_T Constant_Value_h2;                       /* Expression: 1
                                                   * Referenced by: '<S23>/Constant'
                                                   */
  real_T Switch_Threshold_l;                      /* Expression: 0
                                                   * Referenced by: '<S23>/Switch'
                                                   */
  real_T Gain1_Gain_l;                            /* Expression: 20
                                                   * Referenced by: '<S29>/Gain1'
                                                   */
  real_T Constant1_Value_p;                       /* Expression: 1
                                                   * Referenced by: '<S29>/Constant1'
                                                   */
  real_T Gain2_Gain_ds;                           /* Expression: 4
                                                   * Referenced by: '<S29>/Gain2'
                                                   */
  real_T Gain_Gain_k;                             /* Expression: .5
                                                   * Referenced by: '<S29>/Gain'
                                                   */
  real_T Constant_Value_iv;                       /* Expression: 1
                                                   * Referenced by: '<S28>/Constant'
                                                   */
  real_T Integrator_IC;                           /* Expression: 0
                                                   * Referenced by: '<S9>/Integrator'
                                                   */
  real_T Constant_Value_hw;                       /* Expression: shaftSwitchMask
                                                   * Referenced by: '<S16>/Constant'
                                                   */
  real_T Constant2_Value_e;                       /* Expression: 1
                                                   * Referenced by: '<S16>/Constant2'
                                                   */
  real_T Switch_Threshold_p;                      /* Expression: 1
                                                   * Referenced by: '<S16>/Switch'
                                                   */
  real_T Gain_Gain_p;                             /* Expression: 4
                                                   * Referenced by: '<S21>/Gain'
                                                   */
  real_T Reset_Value;                             /* Expression: 1
                                                   * Referenced by: '<S34>/Reset'
                                                   */
  real_T Reset_Value_k;                           /* Expression: 1
                                                   * Referenced by: '<S42>/Reset'
                                                   */
};

/* Real-time Model Data Structure */
struct tag_RTM_TX_DriveLine_EV_Front_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_TX_DriveLine_EV_Front_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_TX_DriveLine_EV_Front_T *contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[7];
  real_T odeF[4][7];
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

/* Class declaration for model TX_DriveLine_EV_Front */
class MODULE_API TX_DriveLine_EV_Front {
  /* public data and function members */
 public:
  /* Copy Constructor */
  TX_DriveLine_EV_Front(TX_DriveLine_EV_Front const &) = delete;

  /* Assignment Operator */
  TX_DriveLine_EV_Front &operator=(TX_DriveLine_EV_Front const &) & = delete;

  /* Move Constructor */
  TX_DriveLine_EV_Front(TX_DriveLine_EV_Front &&) = delete;

  /* Move Assignment Operator */
  TX_DriveLine_EV_Front &operator=(TX_DriveLine_EV_Front &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_TX_DriveLine_EV_Front_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_TX_DriveLine_EV_Front_T *pExtU_TX_DriveLine_EV_Front_T) {
    TX_DriveLine_EV_Front_U = *pExtU_TX_DriveLine_EV_Front_T;
  }

  /* Root outports get method */
  const ExtY_TX_DriveLine_EV_Front_T &getExternalOutputs() const { return TX_DriveLine_EV_Front_Y; }

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
  TX_DriveLine_EV_Front();

  /* Destructor */
  ~TX_DriveLine_EV_Front();

 protected:
  /* Tunable parameters */
  static P_TX_DriveLine_EV_Front_T TX_DriveLine_EV_Front_P;

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_TX_DriveLine_EV_Front_T TX_DriveLine_EV_Front_U;

  /* External outputs */
  ExtY_TX_DriveLine_EV_Front_T TX_DriveLine_EV_Front_Y;

  /* Block signals */
  B_TX_DriveLine_EV_Front_T TX_DriveLine_EV_Front_B;

  /* Block states */
  DW_TX_DriveLine_EV_Front_T TX_DriveLine_EV_Front_DW;

  /* Tunable parameters */
  // static P_TX_DriveLine_EV_Front_T TX_DriveLine_EV_Front_P;

  /* Block continuous states */
  X_TX_DriveLine_EV_Front_T TX_DriveLine_EV_Front_X;

  /* Triggered events */
  PrevZCX_TX_DriveLine_EV_Front_T TX_DriveLine_EV_Front_PrevZCX;

  /* private member function(s) for subsystem '<Root>'*/
  void TX_DriveLine_EV_Front_automldiffls(const real_T u[4], real_T bw1, real_T bd, real_T bw2, real_T Ndiff,
                                          real_T shaftSwitch, real_T Jd, real_T Jw1, real_T Jw2, const real_T x[2],
                                          real_T y[4], real_T xdot[2]);

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void TX_DriveLine_EV_Front_derivatives();

  /* Real-Time Model */
  RT_MODEL_TX_DriveLine_EV_Front_T TX_DriveLine_EV_Front_M;
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
 * '<Root>' : 'TX_DriveLine_EV_Front'
 * '<S1>'   : 'TX_DriveLine_EV_Front/EV_DriveLine_Front'
 * '<S2>'   : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine'
 * '<S3>'   : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive'
 * '<S4>'   : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/EVFrontLimitedSlipDifferential'
 * '<S5>'   : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance1'
 * '<S6>'   : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance2'
 * '<S7>'   : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential'
 * '<S8>'   : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation'
 * '<S9>'   : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Clutch Response'
 * '<S10>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Coupling Torque'
 * '<S11>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency'
 * '<S12>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Limited Slip Differential'
 * '<S13>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/y'
 * '<S14>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power'
 * '<S15>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Damping Power'
 * '<S16>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Kinetic Power'
 * '<S17>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus Creator'
 * '<S18>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrNotTrnsfrd Input'
 * '<S19>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrStored Input'
 * '<S20>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Bus Creation/Power/Power Accounting Bus
 * Creator/PwrTrnsfrd Input'
 * '<S21>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Coupling Torque/Ideal Wet Clutch Coupling'
 * '<S22>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 1 Efficiency'
 * '<S23>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 2 Efficiency'
 * '<S24>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Drive Efficiency'
 * '<S25>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Eta'
 * '<S26>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 1 Efficiency/Low speed blend'
 * '<S27>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 1 Efficiency/Low speed blend/blend'
 * '<S28>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 2 Efficiency/Low speed blend'
 * '<S29>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Axle 2 Efficiency/Low speed blend/blend'
 * '<S30>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Drive Efficiency/Low speed blend'
 * '<S31>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Drive Efficiency/Low speed blend/blend'
 * '<S32>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel
 * Drive/EVFrontLimitedSlipDifferential/Limited Slip Differential/Efficiency/Eta/Constant Eta'
 * '<S33>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear'
 * '<S34>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Cont LPF IC Dyn'
 * '<S35>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power'
 * '<S36>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Damping Power'
 * '<S37>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator'
 * '<S38>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S39>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S40>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance1/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S41>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear'
 * '<S42>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Cont LPF IC Dyn'
 * '<S43>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power'
 * '<S44>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Damping Power'
 * '<S45>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator'
 * '<S46>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S47>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrStored Input'
 * '<S48>'  : 'TX_DriveLine_EV_Front/EV_DriveLine_Front/DriveLine/Front Wheel Drive/Torsional Compliance2/Torsional
 * Compliance Linear/Power/Power Accounting Bus Creator/PwrTrnsfrd Input'
 */
#endif /* RTW_HEADER_TX_DriveLine_EV_Front_h_ */
