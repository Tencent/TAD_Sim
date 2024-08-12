/*
 * EMotor.h
 *
 * Code generation for model "EMotor".
 *
 * Model version              : 1.31
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Thu Sep 15 20:22:54 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_EMotor_h_
#define RTW_HEADER_EMotor_h_
#include "EMotor_types.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rtwtypes.h"

#include "inc/car_common.h"

extern "C" {

#include "rtGetInf.h"
}
#include <cstring>
#include "rtw_modelmap.h"

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

/* Block signals (default storage) */
struct B_EMotor_T {
  real_T Sum; /* '<S6>/Sum' */
};

/* Continuous states (default storage) */
struct X_EMotor_T {
  real_T Integrator_CSTATE; /* '<S6>/Integrator' */
};

/* State derivatives (default storage) */
struct XDot_EMotor_T {
  real_T Integrator_CSTATE; /* '<S6>/Integrator' */
};

/* State disabled  */
struct XDis_EMotor_T {
  boolean_T Integrator_CSTATE; /* '<S6>/Integrator' */
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
struct ExtU_EMotor_T {
  moter_in mot_in; /* '<Root>/mot_in' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_EMotor_T {
  motor_out mot_out; /* '<Root>/mot_out' */
};

/* Parameters (default storage) */
struct P_EMotor_T_ {
  real_T mot_tc_s;                                           /* Variable: mot_tc_s
                                                              * Referenced by: '<S6>/Gain1'
                                                              */
  real_T GainPRM_Gain;                                       /* Expression: 1/60*2*3.1415926
                                                              * Referenced by: '<Root>/GainPRM'
                                                              */
  real_T MaxTorqueLimit_tableData[tx_car::kMap1dSize]; /* Expression: T_t_extended
                                                              * Referenced by: '<S6>/Max Torque Limit'
                                                              */
  real_T MaxTorqueLimit_bp01Data[tx_car::kMap1dSize];  /* Expression: w_t_extended
                                                              * Referenced by: '<S6>/Max Torque Limit'
                                                              */
  real_T Integrator_IC;                                      /* Expression: 0
                                                              * Referenced by: '<S6>/Integrator'
                                                              */
  real_T Interpolatedzerocrossing_tableData[2];              /* Expression: [-1 1]
                                                              * Referenced by: '<S6>/Interpolated zero-crossing'
                                                              */
  real_T Interpolatedzerocrossing_bp01Data[2];               /* Expression: [-1 1]
                                                              * Referenced by: '<S6>/Interpolated zero-crossing'
                                                              */
  real_T uDLookupTable_tableData[tx_car::kMap2dSize];  /* Expression: x_losses_mat
                                                              * Referenced by: '<S5>/2-D Lookup Table'
                                                              */
  real_T uDLookupTable_bp01Data[tx_car::kMap1dSize];   /* Expression: x_w_eff_vec
                                                              * Referenced by: '<S5>/2-D Lookup Table'
                                                              */
  real_T uDLookupTable_bp02Data[tx_car::kMap1dSize];   /* Expression: x_T_eff_vec
                                                              * Referenced by: '<S5>/2-D Lookup Table'
                                                              */
  real_T Saturation_UpperSat;                                /* Expression: Inf
                                                              * Referenced by: '<S3>/Saturation'
                                                              */
  real_T Saturation_LowerSat;                                /* Expression: 0.0001
                                                              * Referenced by: '<S3>/Saturation'
                                                              */
  uint32_T uDLookupTable_maxIndex[2];
  /* Computed Parameter: uDLookupTable_maxIndex
   * Referenced by: '<S5>/2-D Lookup Table'
   */
};

/* Real-time Model Data Structure */
struct tag_RTM_EMotor_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_EMotor_T *contStates;
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
    void *dataAddress[13];
    int32_T *vardimsAddress[13];
    RTWLoggingFcnPtr loggingPtrs[13];
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
    SimTimeStep simTimeStep;
    boolean_T stopRequestedFlag;
    time_T *t;
    time_T tArray[2];
  } Timing;
};

/* Function to get C API Model Mapping Static Info */
extern const rtwCAPI_ModelMappingStaticInfo *EMotor_GetCAPIStaticMap(void);

/* Class declaration for model EMotor */
class MODULE_API EMotor {
  /* public data and function members */
 public:
  /* Copy Constructor */
  EMotor(EMotor const &) = delete;

  /* Assignment Operator */
  EMotor &operator=(EMotor const &) & = delete;

  /* Move Constructor */
  EMotor(EMotor &&) = delete;

  /* Move Assignment Operator */
  EMotor &operator=(EMotor &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_EMotor_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_EMotor_T *pExtU_EMotor_T) { EMotor_U = *pExtU_EMotor_T; }

  /* Root outports get method */
  const ExtY_EMotor_T &getExternalOutputs() const { return EMotor_Y; }

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  static void terminate();

  /* Constructor */
  EMotor();

  /* Destructor */
  ~EMotor();

  /* private data and function members */
 protected:
  /* External inputs */
  ExtU_EMotor_T EMotor_U;

  /* External outputs */
  ExtY_EMotor_T EMotor_Y;

  /* Block signals */
  B_EMotor_T EMotor_B;

  /* Tunable parameters */
  static P_EMotor_T EMotor_P;

  /* Block continuous states */
  X_EMotor_T EMotor_X;

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void EMotor_derivatives();

  /* Real-Time Model */
  RT_MODEL_EMotor_T EMotor_M;

  int m_mot_trq_map_num = 7;  // 电机外特性最大值

  int m_mot_loss_map_spd_num = 5;  // 效率点MAP表转速点数

  double m_step_size = 0.001;
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
 * '<Root>' : 'EMotor'
 * '<S1>'   : 'EMotor/Mapped Motor'
 * '<S2>'   : 'EMotor/Mapped Motor/Mapped Motor Core Speed 4'
 * '<S3>'   : 'EMotor/Mapped Motor/Mapped Motor Core Speed 4/Electrical Current'
 * '<S4>'   : 'EMotor/Mapped Motor/Mapped Motor Core Speed 4/Motor Units'
 * '<S5>'   : 'EMotor/Mapped Motor/Mapped Motor Core Speed 4/Tabular Power Loss Data'
 * '<S6>'   : 'EMotor/Mapped Motor/Mapped Motor Core Speed 4/Tabulated Torque-speed Envelope'
 * '<S7>'   : 'EMotor/Mapped Motor/Mapped Motor Core Speed 4/Motor Units/Power Accounting Bus Creator'
 * '<S8>'   : 'EMotor/Mapped Motor/Mapped Motor Core Speed 4/Motor Units/Power Accounting Bus Creator/PwrNotTrnsfrd
 * Input'
 * '<S9>'   : 'EMotor/Mapped Motor/Mapped Motor Core Speed 4/Motor Units/Power Accounting Bus Creator/PwrStored Input'
 * '<S10>'  : 'EMotor/Mapped Motor/Mapped Motor Core Speed 4/Motor Units/Power Accounting Bus Creator/PwrTrnsfrd Input'
 */
#endif /* RTW_HEADER_EMotor_h_ */
