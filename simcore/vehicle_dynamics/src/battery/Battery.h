/*
 * Battery.h
 *
 * Code generation for model "Battery".
 *
 * Model version              : 1.36
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Thu Jun 16 17:51:30 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_Battery_h_
#define RTW_HEADER_Battery_h_
#include <cstring>
#include "Battery_types.h"
#include "rtw_continuous.h"
#include "rtw_modelmap.h"
#include "rtw_solver.h"
#include "rtwtypes.h"

#include <iostream>
#include "../../inc/car_common.h"

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

#ifndef rtmGetDataMapInfo
#  define rtmGetDataMapInfo(rtm) ((rtm)->DataMapInfo)
#endif

#ifndef rtmSetDataMapInfo
#  define rtmSetDataMapInfo(rtm, val) ((rtm)->DataMapInfo = (val))
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

namespace tx_car {
namespace power {

/* Block signals (default storage) */
struct B_Battery_T {
  real_T Switch; /* '<S6>/Switch' */
  real_T Gain1;  /* '<S6>/Gain1' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_Battery_T {
  int_T IntegratorLimited_IWORK; /* '<S6>/Integrator Limited' */
};

/* Continuous states (default storage) */
struct X_Battery_T {
  real_T IntegratorLimited_CSTATE; /* '<S6>/Integrator Limited' */
};

/* State derivatives (default storage) */
struct XDot_Battery_T {
  real_T IntegratorLimited_CSTATE; /* '<S6>/Integrator Limited' */
};

/* State disabled  */
struct XDis_Battery_T {
  boolean_T IntegratorLimited_CSTATE; /* '<S6>/Integrator Limited' */
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
struct ExtU_Battery_T {
  batt_in batt_input; /* '<Root>/batt_input' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_Battery_T {
  batt_out batt_output; /* '<Root>/batt_output' */
};

/* Parameters (default storage) */
struct P_Battery_T_ {
  real_T BattChargeMax;                        /* Variable: BattChargeMax
                                                * Referenced by:
                                                *   '<S2>/Constant1'
                                                *   '<S6>/Constant1'
                                                *   '<S6>/Integrator Limited'
                                                *   '<S6>/Switch'
                                                *   '<S7>/Constant1'
                                                */
  real_T BattInitSoc;                          /* Variable: BattInitSoc
                                                * Referenced by: '<S2>/Constant1'
                                                */
  real_T BattTempBp[tx_car::kMap1dSize]; /* Variable: BattTempBp
                                                * Referenced by: '<S8>/R'
                                                */
  real_T CapLUTBp[tx_car::kMap1dSize];   /* Variable: CapLUTBp
                                                * Referenced by: '<S8>/Em'
                                                */
  real_T CapSOCBp[tx_car::kMap1dSize];   /* Variable: CapSOCBp
                                                * Referenced by: '<S8>/R'
                                                */
  real_T Em[tx_car::kMap1dSize];         /* Variable: Em
                                                * Referenced by: '<S8>/Em'
                                                */
  real_T Np;                                   /* Variable: Np
                                                * Referenced by:
                                                *   '<S6>/Gain1'
                                                *   '<S8>/Gain2'
                                                *   '<S8>/Gain4'
                                                */
  real_T Ns;                                   /* Variable: Ns
                                                * Referenced by:
                                                *   '<S8>/Gain1'
                                                *   '<S8>/Gain3'
                                                */
  real_T RInt[tx_car::kMap2dSize];       /* Variable: RInt
                                                * Referenced by: '<S8>/R'
                                                */
  real_T IntegratorLimited_LowerSat;           /* Expression: 0
                                                * Referenced by: '<S6>/Integrator Limited'
                                                */
  real_T Gain2_Gain;                           /* Expression: 1
                                                * Referenced by: '<Root>/Gain2'
                                                */
  real_T Gain_Gain;                            /* Expression: 1
                                                * Referenced by: '<Root>/Gain'
                                                */
  real_T Gain1_Gain;                           /* Expression: -1
                                                * Referenced by: '<S3>/Gain1'
                                                */
  real_T Gain1_Gain_c;                         /* Expression: -1
                                                * Referenced by: '<Root>/Gain1'
                                                */
  uint32_T R_maxIndex[2];                      /* Computed Parameter: R_maxIndex
                                                * Referenced by: '<S8>/R'
                                                */
};

/* Real-time Model Data Structure */
struct tag_RTM_Battery_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_Battery_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  boolean_T *contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[1];
  real_T odeF[4][1];
  ODE4_IntgData intgData;

  /*
   * DataMapInfo:
   * The following substructure contains information regarding
   * structures generated in the model's C API.
   */
  struct {
    rtwCAPI_ModelMappingInfo mmi;
    void *dataAddress[15];
    int32_T *vardimsAddress[15];
    RTWLoggingFcnPtr loggingPtrs[15];
  } DataMapInfo;

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

/* Function to get C API Model Mapping Static Info */
extern const rtwCAPI_ModelMappingStaticInfo *Battery_GetCAPIStaticMap(void);

/* Class declaration for model Battery */
class MODULE_API Battery {
  /* public data and function members */
 public:
  /* Copy Constructor */
  Battery(Battery const &) = delete;

  /* Assignment Operator */
  Battery &operator=(Battery const &) & = delete;

  /* Move Constructor */
  Battery(Battery &&) = delete;

  /* Move Assignment Operator */
  Battery &operator=(Battery &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_Battery_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_Battery_T *pExtU_Battery_T) { Battery_U = *pExtU_Battery_T; }

  /* Root outports get method */
  const ExtY_Battery_T &getExternalOutputs() const { return Battery_Y; }

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  static void terminate();

  /* Constructor */
  Battery();

  /* Destructor */
  ~Battery();

  static P_Battery_T &getParamterStruct();  // get the reference of battery parameter struct

 protected:
  /* Tunable parameters */
  static P_Battery_T Battery_P;

  int m_batt_ocv_map_num = 101;

  int m_resis_map_temp_num = 7;

  /* private data and function members */
 protected:
  /* External inputs */
  ExtU_Battery_T Battery_U;

  /* External outputs */
  ExtY_Battery_T Battery_Y;

  /* Block signals */
  B_Battery_T Battery_B;

  /* Block states */
  DW_Battery_T Battery_DW;

  /* Tunable parameters */
  // static P_Battery_T Battery_P;

  /* Block continuous states */
  X_Battery_T Battery_X;

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void Battery_derivatives();

  /* Real-Time Model */
  RT_MODEL_Battery_T Battery_M;
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
 * '<Root>' : 'Battery'
 * '<S1>'   : 'Battery/Datasheet Battery'
 * '<S2>'   : 'Battery/Datasheet Battery/Datasheet Battery Internal'
 * '<S3>'   : 'Battery/Datasheet Battery/Info Bus'
 * '<S4>'   : 'Battery/Datasheet Battery/Output Passthrough'
 * '<S5>'   : 'Battery/Datasheet Battery/Datasheet Battery Internal/Datasheet Battery'
 * '<S6>'   : 'Battery/Datasheet Battery/Datasheet Battery Internal/Datasheet Battery/Charge Model'
 * '<S7>'   : 'Battery/Datasheet Battery/Datasheet Battery Internal/Datasheet Battery/State of Charge Capacity'
 * '<S8>'   : 'Battery/Datasheet Battery/Datasheet Battery Internal/Datasheet Battery/Voltage and Power Calculation'
 * '<S9>'   : 'Battery/Datasheet Battery/Info Bus/Power Accounting Bus Creator'
 * '<S10>'  : 'Battery/Datasheet Battery/Info Bus/Power Accounting Bus Creator/PwrNotTrnsfrd Input'
 * '<S11>'  : 'Battery/Datasheet Battery/Info Bus/Power Accounting Bus Creator/PwrStored Input'
 * '<S12>'  : 'Battery/Datasheet Battery/Info Bus/Power Accounting Bus Creator/PwrTrnsfrd Input'
 */

}  // namespace power
}  // namespace tx_car
#endif /* RTW_HEADER_Battery_h_ */
