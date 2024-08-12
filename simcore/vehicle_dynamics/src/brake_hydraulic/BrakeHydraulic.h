/*
 * BrakeHydraulic.h
 *
 * Code generation for model "BrakeHydraulic".
 *
 * Model version              : 2.37
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Thu May 11 21:00:45 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_BrakeHydraulic_h_
#define RTW_HEADER_BrakeHydraulic_h_
#include <cstring>
#include "BrakeHydraulic_types.h"
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

/* Block signals (default storage) */
struct B_BrakeHydraulic_T {
  real_T Product1[4]; /* '<S1>/Product1' */
  real_T Divide;      /* '<S3>/Divide' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_BrakeHydraulic_T {
  real_T fluidTransportDelay_DSTATE[400]; /* '<S1>/fluidTransportDelay' */
  real_T Memory_PreviousInput[4];         /* '<Root>/Memory' */
};

/* Continuous states (default storage) */
struct X_BrakeHydraulic_T {
  real_T Integrator1_CSTATE; /* '<S3>/Integrator1' */
};

/* State derivatives (default storage) */
struct XDot_BrakeHydraulic_T {
  real_T Integrator1_CSTATE; /* '<S3>/Integrator1' */
};

/* State disabled  */
struct XDis_BrakeHydraulic_T {
  boolean_T Integrator1_CSTATE; /* '<S3>/Integrator1' */
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
struct ExtU_BrakeHydraulic_T {
  real_T BrkCmd01; /* '<Root>/BrkCmd[01]' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_BrakeHydraulic_T {
  real_T BrkPrsPa[4]; /* '<Root>/BrkPrs[Pa]' */
};

/* Parameters (default storage) */
struct P_BrakeHydraulic_T_ {
  real_T brake_const_time_fluid; /* Variable: brake_const_time_fluid
                                  * Referenced by: '<S3>/Constant'
                                  */
  real_T brake_fluid_transport_const_time;
  /* Variable: brake_fluid_transport_const_time
   * Referenced by: '<S1>/Constant3'
   */
  real_T max_main_cylinder_pressure;           /* Variable: max_main_cylinder_pressure
                                                * Referenced by:
                                                *   '<S1>/Constant1'
                                                *   '<S1>/Saturation'
                                                */
  real_T rear_brake_ratio;                     /* Variable: rear_brake_ratio
                                                * Referenced by: '<S1>/Rear Bias'
                                                */
  real_T Integrator1_IC;                       /* Expression: 0
                                                * Referenced by: '<S3>/Integrator1'
                                                */
  real_T Memory_InitialCondition;              /* Expression: 0
                                                * Referenced by: '<Root>/Memory'
                                                */
  real_T FrontBias_Value;                      /* Expression: 1
                                                * Referenced by: '<S1>/Front Bias'
                                                */
  real_T Gain_Gain;                            /* Expression: 1000
                                                * Referenced by: '<S1>/Gain'
                                                */
  real_T fluidTransportDelay_InitialCondition; /* Expression: 0.0
                                                * Referenced by: '<S1>/fluidTransportDelay'
                                                */
  real_T Saturation_LowerSat;                  /* Expression: 0
                                                * Referenced by: '<S1>/Saturation'
                                                */
};

/* Real-time Model Data Structure */
struct tag_RTM_BrakeHydraulic_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_BrakeHydraulic_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_BrakeHydraulic_T *contStateDisabled;
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

/* Class declaration for model BrakeHydraulic */
class MODULE_API BrakeHydraulic {
  /* public data and function members */
 public:
  /* Copy Constructor */
  BrakeHydraulic(BrakeHydraulic const &) = delete;

  /* Assignment Operator */
  BrakeHydraulic &operator=(BrakeHydraulic const &) & = delete;

  /* Move Constructor */
  BrakeHydraulic(BrakeHydraulic &&) = delete;

  /* Move Assignment Operator */
  BrakeHydraulic &operator=(BrakeHydraulic &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_BrakeHydraulic_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_BrakeHydraulic_T *pExtU_BrakeHydraulic_T) {
    BrakeHydraulic_U = *pExtU_BrakeHydraulic_T;
  }

  /* Root outports get method */
  const ExtY_BrakeHydraulic_T &getExternalOutputs() const { return BrakeHydraulic_Y; }

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  static void terminate();

  /* Constructor */
  BrakeHydraulic();

  /* Destructor */
  ~BrakeHydraulic();

 protected:
  /* Tunable parameters */
  static P_BrakeHydraulic_T BrakeHydraulic_P;

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_BrakeHydraulic_T BrakeHydraulic_U;

  /* External outputs */
  ExtY_BrakeHydraulic_T BrakeHydraulic_Y;

  /* Block signals */
  B_BrakeHydraulic_T BrakeHydraulic_B;

  /* Block states */
  DW_BrakeHydraulic_T BrakeHydraulic_DW;

  /* Tunable parameters */
  // static P_BrakeHydraulic_T BrakeHydraulic_P;

  /* Block continuous states */
  X_BrakeHydraulic_T BrakeHydraulic_X;

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void BrakeHydraulic_derivatives();

  /* Real-Time Model */
  RT_MODEL_BrakeHydraulic_T BrakeHydraulic_M;
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
 * '<Root>' : 'BrakeHydraulic'
 * '<S1>'   : 'BrakeHydraulic/BrakeHydraulics'
 * '<S2>'   : 'BrakeHydraulic/BrakeHydraulics/Hydraulic'
 * '<S3>'   : 'BrakeHydraulic/BrakeHydraulics/Hydraulic/BrakeAct'
 */
#endif /* RTW_HEADER_BrakeHydraulic_h_ */
