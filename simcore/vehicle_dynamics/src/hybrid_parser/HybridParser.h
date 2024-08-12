/*
 * HybridParser.h
 *
 * Code generation for model "HybridParser".
 *
 * Model version              : 2.4
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Fri Jul 28 13:05:33 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_HybridParser_h_
#define RTW_HEADER_HybridParser_h_
#include "HybridParser_types.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rtwtypes.h"

#include "inc/car_common.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#  define rtmGetErrorStatus(rtm) ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#  define rtmSetErrorStatus(rtm, val) ((rtm)->errorStatus = (val))
#endif

/* External inputs (root inport signals with default storage) */
struct ExtU_HybridParser_T {
  real_T NoUse; /* '<Root>/NoUse' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_HybridParser_T {
  real_T EnableP2;                 /* '<Root>/EnableP2' */
  real_T EnableP3;                 /* '<Root>/EnableP3' */
  real_T EnableP4;                 /* '<Root>/EnableP4' */
  real_T EngToGenerator_GearRatio; /* '<Root>/EngToGenerator_GearRatio' */
  real_T EngClutch_TimeConsts;     /* '<Root>/EngClutch_TimeConst[s]' */
  real_T FrontMotGearRatio;        /* '<Root>/FrontMotGearRatio' */
};

/* Parameters (default storage) */
struct P_HybridParser_T_ {
  real_T EnableP2;                /* Variable: EnableP2
                                   * Referenced by: '<S1>/P2'
                                   */
  real_T EnableP3;                /* Variable: EnableP3
                                   * Referenced by: '<S1>/P3'
                                   */
  real_T EnableP4;                /* Variable: EnableP4
                                   * Referenced by: '<S1>/P4'
                                   */
  real_T Engine_Clutch_TimeConst; /* Variable: Engine_Clutch_TimeConst
                                   * Referenced by: '<S1>/Engine_Clutch_TimeConst'
                                   */
  real_T ReGen_Motor_GearRatio;   /* Variable: ReGen_Motor_GearRatio
                                   * Referenced by: '<S1>/ReGen_Motor_GearRatio[]'
                                   */
  real_T front_motor_gear_ratio;  /* Variable: front_motor_gear_ratio
                                   * Referenced by: '<S1>/front_motor_gear_ratio'
                                   */
};

/* Real-time Model Data Structure */
struct tag_RTM_HybridParser_T {
  const char_T *errorStatus;
};

/* Class declaration for model HybridParser */
class MODULE_API HybridParser {
  /* public data and function members */
 public:
  /* Copy Constructor */
  HybridParser(HybridParser const &) = delete;

  /* Assignment Operator */
  HybridParser &operator=(HybridParser const &) & = delete;

  /* Move Constructor */
  HybridParser(HybridParser &&) = delete;

  /* Move Assignment Operator */
  HybridParser &operator=(HybridParser &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_HybridParser_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_HybridParser_T *pExtU_HybridParser_T) { HybridParser_U = *pExtU_HybridParser_T; }

  /* Root outports get method */
  const ExtY_HybridParser_T &getExternalOutputs() const { return HybridParser_Y; }

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  static void terminate();

  /* Constructor */
  HybridParser();

  /* Destructor */
  ~HybridParser();

 protected:
  /* Tunable parameters */
  static P_HybridParser_T HybridParser_P;

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_HybridParser_T HybridParser_U;

  /* External outputs */
  ExtY_HybridParser_T HybridParser_Y;

  /* Tunable parameters */
  // static P_HybridParser_T HybridParser_P;

  /* Real-Time Model */
  RT_MODEL_HybridParser_T HybridParser_M;
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
 * '<Root>' : 'HybridParser'
 * '<S1>'   : 'HybridParser/HybridParser'
 */
#endif /* RTW_HEADER_HybridParser_h_ */
