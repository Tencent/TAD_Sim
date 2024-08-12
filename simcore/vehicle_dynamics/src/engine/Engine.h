/*
 * Engine.h
 *
 * Code generation for model "Engine".
 *
 * Model version              : 2.21
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Dec  6 13:01:57 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_Engine_h_
#define RTW_HEADER_Engine_h_
#include <cstring>
#include "Engine_types.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rtwtypes.h"

#include "inc/car_common.h"

extern "C" {

#include "rtGetInf.h"
}

#include <cmath>

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

/* Block signals (default storage) */
struct B_Engine_T {
  real_T FuelFlwTable; /* '<S8>/FuelFlw Table' */
  real_T RPMtodegs;    /* '<S5>/RPM to deg//s' */
  real_T Divide;       /* '<S13>/Divide' */
};

/* Continuous states (default storage) */
struct X_Engine_T {
  real_T Integrator1_CSTATE;  /* '<S13>/Integrator1' */
  real_T Integrator_CSTATE;   /* '<S1>/Integrator' */
  real_T Integrator_CSTATE_h; /* '<S5>/Integrator' */
};

/* Periodic continuous state vector (global) */
typedef int_T PeriodicIndX_Engine_T[1];
typedef real_T PeriodicRngX_Engine_T[2];

/* State derivatives (default storage) */
struct XDot_Engine_T {
  real_T Integrator1_CSTATE;  /* '<S13>/Integrator1' */
  real_T Integrator_CSTATE;   /* '<S1>/Integrator' */
  real_T Integrator_CSTATE_h; /* '<S5>/Integrator' */
};

/* State disabled  */
struct XDis_Engine_T {
  boolean_T Integrator1_CSTATE;  /* '<S13>/Integrator1' */
  boolean_T Integrator_CSTATE;   /* '<S1>/Integrator' */
  boolean_T Integrator_CSTATE_h; /* '<S5>/Integrator' */
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
struct ExtU_Engine_T {
  ENGINE_INPUTBUS EngIn; /* '<Root>/EngIn' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_Engine_T {
  ENGINE_OUTPUTBUS EngOut; /* '<Root>/EngOut' */
};

/* Parameters (default storage) */
struct P_Engine_T_ {
  real_T ConstTimeEng;                                    /* Variable: ConstTimeEng
                                                           * Referenced by: '<S13>/Constant'
                                                           */
  real_T f_fuel[tx_car::kMap2dSize];                /* Variable: f_fuel
                                                           * Referenced by: '<S8>/FuelFlw Table'
                                                           */
  real_T f_tbrake[tx_car::kMap2dSize];              /* Variable: f_tbrake
                                                           * Referenced by: '<S8>/2-D Lookup
                                                           * Table'
                                                           */
  real_T f_tbrake_n_bpt[tx_car::kMap1dSize];        /* Variable: f_tbrake_n_bpt
                                                           * Referenced by:
                                                           *   '<S8>/2-D Lookup Table'
                                                           *   '<S8>/FuelFlw Table'
                                                           */
  real_T f_tbrake_t_bpt[tx_car::kMap1dSize];        /* Variable: f_tbrake_t_bpt
                                                           * Referenced by: '<S8>/2-D
                                                           * Lookup Table'
                                                           */
  real_T fuelDensity;                                     /* Variable: fuelDensity
                                                           * Referenced by: '<S1>/Constant'
                                                           */
  real_T Integrator1_IC;                                  /* Expression: 0
                                                           * Referenced by: '<S13>/Integrator1'
                                                           */
  real_T Constant_Value;                                  /* Expression: 0
                                                           * Referenced by: '<S3>/Constant'
                                                           */
  real_T Gain_Gain;                                       /* Expression: 1
                                                           * Referenced by: '<S2>/Gain'
                                                           */
  real_T rpmtorads_Gain;                                  /* Expression: -pi/30
                                                           * Referenced by: '<S7>/rpm to rad//s'
                                                           */
  real_T FuelFlwTable_bp01Data[tx_car::kMap1dSize]; /* Expression: f_tbrake_t_bpt*EngMaxTrq
                                                           * Referenced by: '<S8>/FuelFlw Table'
                                                           */
  real_T Gain_Gain_e;                                     /* Expression: 1000
                                                           * Referenced by: '<S1>/Gain'
                                                           */
  real_T Integrator_IC;                                   /* Expression: 0
                                                           * Referenced by: '<S1>/Integrator'
                                                           */
  real_T Integrator_UpperSat;                             /* Expression: inf
                                                           * Referenced by: '<S1>/Integrator'
                                                           */
  real_T Integrator_LowerSat;                             /* Expression: 0
                                                           * Referenced by: '<S1>/Integrator'
                                                           */
  real_T Integrator_IC_o;                                 /* Expression: 0
                                                           * Referenced by: '<S5>/Integrator'
                                                           */
  real_T Integrator_WrappedStateUpperValue;               /* Expression: 360*Cps
                                                           * Referenced by: '<S5>/Integrator'
                                                           */
  real_T Integrator_WrappedStateLowerValue;               /* Expression: 0
                                                           * Referenced by: '<S5>/Integrator'
                                                           */
  real_T RPMtodegs_Gain;                                  /* Expression: 180/30
                                                           * Referenced by: '<S5>/RPM to deg//s'
                                                           */
  uint32_T FuelFlwTable_maxIndex[2];                      /* Computed Parameter: FuelFlwTable_maxIndex
                                                           * Referenced by: '<S8>/FuelFlw Table'
                                                           */
  uint32_T uDLookupTable_maxIndex[2];
  /* Computed Parameter: uDLookupTable_maxIndex
   * Referenced by: '<S8>/2-D Lookup Table'
   */
};

/* Real-time Model Data Structure */
struct tag_RTM_Engine_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_Engine_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_Engine_T *contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[3];
  real_T odeF[4][3];
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

/* Class declaration for model Engine */
class MODULE_API Engine {
  /* public data and function members */
 public:
  /* Copy Constructor */
  Engine(Engine const &) = delete;

  /* Assignment Operator */
  Engine &operator=(Engine const &) & = delete;

  /* Move Constructor */
  Engine(Engine &&) = delete;

  /* Move Assignment Operator */
  Engine &operator=(Engine &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_Engine_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_Engine_T *pExtU_Engine_T) { Engine_U = *pExtU_Engine_T; }

  /* Root outports get method */
  const ExtY_Engine_T &getExternalOutputs() const { return Engine_Y; }

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  static void terminate();

  /* Constructor */
  Engine();

  /* Destructor */
  ~Engine();

 protected:
  /* Tunable parameters */
  static P_Engine_T Engine_P;

  int m_eng_map_spd_num = 22;  // map speed

  int m_eng_trq_req_num = 10;  // map trq

  int m_throttle_num = 10;  // map throttle

  double m_step_size = 0.001;

  double m_max_spd = 6500.0;  // max engine speed

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_Engine_T Engine_U;

  /* External outputs */
  ExtY_Engine_T Engine_Y;

  /* Block signals */
  B_Engine_T Engine_B;

  /* Tunable parameters */
  // static P_Engine_T Engine_P;

  /* Block continuous states */
  X_Engine_T Engine_X;
  PeriodicIndX_Engine_T Engine_PeriodicIndX; /* Block periodic continuous states */
  PeriodicRngX_Engine_T Engine_PeriodicRngX;

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void Engine_derivatives();

  /* Real-Time Model */
  RT_MODEL_Engine_T Engine_M;
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
 * '<Root>' : 'Engine'
 * '<S1>'   : 'Engine/Engine'
 * '<S2>'   : 'Engine/Engine/Engine'
 * '<S3>'   : 'Engine/Engine/Engine/Accessory Load Model'
 * '<S4>'   : 'Engine/Engine/Engine/MappedEngine'
 * '<S5>'   : 'Engine/Engine/Engine/MappedEngine/Engine Crank Angle Calculation'
 * '<S6>'   : 'Engine/Engine/Engine/MappedEngine/Fuel Volume Flow'
 * '<S7>'   : 'Engine/Engine/Engine/MappedEngine/Mapped Engine Power Info'
 * '<S8>'   : 'Engine/Engine/Engine/MappedEngine/MappedEngine'
 * '<S9>'   : 'Engine/Engine/Engine/MappedEngine/Mapped Engine Power Info/Power Accounting Bus Creator'
 * '<S10>'  : 'Engine/Engine/Engine/MappedEngine/Mapped Engine Power Info/Power Accounting Bus Creator/No PwrStored
 * Input'
 * '<S11>'  : 'Engine/Engine/Engine/MappedEngine/Mapped Engine Power Info/Power Accounting Bus Creator/PwrNotTrnsfrd
 * Input'
 * '<S12>'  : 'Engine/Engine/Engine/MappedEngine/Mapped Engine Power Info/Power Accounting Bus Creator/PwrTrnsfrd Input'
 * '<S13>'  : 'Engine/Engine/Engine/MappedEngine/MappedEngine/Valve'
 */
#endif /* RTW_HEADER_Engine_h_ */
