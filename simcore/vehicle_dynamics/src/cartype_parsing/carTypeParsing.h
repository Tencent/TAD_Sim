/*
 * carTypeParsing.h
 *
 * Code generation for model "carTypeParsing".
 *
 * Model version              : 1.11
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Fri Jul  8 17:01:31 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_carTypeParsing_h_
#define RTW_HEADER_carTypeParsing_h_
#include "carTypeParsing_types.h"
#include "rtw_continuous.h"
#include "rtw_modelmap.h"
#include "rtw_solver.h"
#include "rtwtypes.h"

#include "inc/car_common.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetDataMapInfo
#  define rtmGetDataMapInfo(rtm) ((rtm)->DataMapInfo)
#endif

#ifndef rtmSetDataMapInfo
#  define rtmSetDataMapInfo(rtm, val) ((rtm)->DataMapInfo = (val))
#endif

#ifndef rtmGetErrorStatus
#  define rtmGetErrorStatus(rtm) ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#  define rtmSetErrorStatus(rtm, val) ((rtm)->errorStatus = (val))
#endif

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_carTypeParsing_T {
  cartype_parsing_out CarType; /* '<Root>/CarType' */
};

/* Parameters (default storage) */
struct P_carTypeParsing_T_ {
  real_T drivetype;           /* Variable: drivetype
                               * Referenced by: '<Root>/Constant'
                               */
  real_T proplutiontype;      /* Variable: proplutiontype
                               * Referenced by: '<Root>/Constant1'
                               */
  real_T parsingbck_01_Value; /* Expression: 1
                               * Referenced by: '<Root>/parsingbck_01'
                               */
  real_T parsingbck_02_Value; /* Expression: 1
                               * Referenced by: '<Root>/parsingbck_02'
                               */
  real_T parsingbck_03_Value; /* Expression: 1
                               * Referenced by: '<Root>/parsingbck_03'
                               */
};

/* Real-time Model Data Structure */
struct tag_RTM_carTypeParsing_T {
  const char_T* errorStatus;

  /*
   * DataMapInfo:
   * The following substructure contains information regarding
   * structures generated in the model's C API.
   */
  struct {
    rtwCAPI_ModelMappingInfo mmi;
    void* dataAddress[5];
    int32_T* vardimsAddress[5];
    RTWLoggingFcnPtr loggingPtrs[5];
  } DataMapInfo;
};

/* Function to get C API Model Mapping Static Info */
extern const rtwCAPI_ModelMappingStaticInfo* carTypeParsing_GetCAPIStaticMap(void);

/* Class declaration for model carTypeParsing */
class MODULE_API carTypeParsing {
  /* public data and function members */
 public:
  /* Copy Constructor */
  carTypeParsing(carTypeParsing const&) = delete;

  /* Assignment Operator */
  carTypeParsing& operator=(carTypeParsing const&) & = delete;

  /* Move Constructor */
  carTypeParsing(carTypeParsing&&) = delete;

  /* Move Assignment Operator */
  carTypeParsing& operator=(carTypeParsing&&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_carTypeParsing_T* getRTM();

  /* Root outports get method */
  const ExtY_carTypeParsing_T& getExternalOutputs() const { return carTypeParsing_Y; }

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  static void terminate();

  /* Constructor */
  carTypeParsing();

  /* Destructor */
  ~carTypeParsing();

  /* private data and function members */
 protected:
  /* External outputs */
  ExtY_carTypeParsing_T carTypeParsing_Y;

  /* Tunable parameters */
  static P_carTypeParsing_T carTypeParsing_P;

  /* Real-Time Model */
  RT_MODEL_carTypeParsing_T carTypeParsing_M;
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
 * '<Root>' : 'carTypeParsing'
 */
#endif /* RTW_HEADER_carTypeParsing_h_ */
