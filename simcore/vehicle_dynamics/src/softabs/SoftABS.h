/*
 * SoftABS.h
 *
 * Code generation for model "SoftABS".
 *
 * Model version              : 2.108
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Fri Jul 21 17:32:21 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_SoftABS_h_
#define RTW_HEADER_SoftABS_h_
#include "SoftABS_types.h"
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

#ifndef rtmGetT
#  define rtmGetT(rtm) (rtmGetTPtr((rtm))[0])
#endif

#ifndef rtmGetTPtr
#  define rtmGetTPtr(rtm) ((rtm)->Timing.t)
#endif

/* Block signals (default storage) */
struct B_SoftABS_T {
  real_T Memory[4];         /* '<S2>/Memory' */
  real_T brake_pressure_01; /* '<S2>/Saturation' */
  real_T Abs;               /* '<S2>/Abs' */
  real_T OnAtEnableRef_OffAtDisableRef;
  /* '<S2>/OnAtEnableRef_OffAtDisableRef' */
  real_T ABS_flag[4];        /* '<S2>/ABS_flag' */
  real_T Product1[4];        /* '<S2>/Product1' */
  real_T Filter;             /* '<S40>/Filter' */
  real_T IntegralGain;       /* '<S42>/Integral Gain' */
  real_T Integrator;         /* '<S45>/Integrator' */
  real_T FilterCoefficient;  /* '<S48>/Filter Coefficient' */
  boolean_T Merge1;          /* '<S2>/Merge1' */
  boolean_T abs_on;          /* '<S2>/if_on' */
  boolean_T LogicalOperator; /* '<S4>/Logical Operator' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_SoftABS_T {
  real_T Filter_DSTATE;             /* '<S40>/Filter' */
  real_T Integrator_DSTATE;         /* '<S45>/Integrator' */
  real_T Memory_PreviousInput[4];   /* '<S2>/Memory' */
  int8_T If_ActiveSubsystem;        /* '<S2>/If' */
  int8_T Filter_PrevResetState;     /* '<S40>/Filter' */
  int8_T Integrator_PrevResetState; /* '<S45>/Integrator' */
  boolean_T OnAtEnableRef_OffAtDisableRef_Mode;
  /* '<S2>/OnAtEnableRef_OffAtDisableRef' */
};

/* External inputs (root inport signals with default storage) */
struct ExtU_SoftABS_T {
  powecu_in ecu_in; /* '<Root>/ecu_in' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_SoftABS_T {
  powecu_out ecu_out; /* '<Root>/ecu_out' */
};

/* Parameters (default storage) */
struct P_SoftABS_T_ {
  real_T DriveLinePowType;       /* Variable: DriveLinePowType
                                  * Referenced by: '<S3>/Constant2'
                                  */
  real_T abs_brake_pressure;     /* Variable: abs_brake_pressure
                                  * Referenced by:
                                  *   '<S4>/abs_brake_pressure_01'
                                  *   '<S5>/abs_brake_pressure_01'
                                  */
  real_T brake_D;                /* Variable: brake_D
                                  * Referenced by: '<S39>/Derivative Gain'
                                  */
  real_T brake_I;                /* Variable: brake_I
                                  * Referenced by: '<S42>/Integral Gain'
                                  */
  real_T brake_P;                /* Variable: brake_P
                                  * Referenced by: '<S50>/Proportional Gain'
                                  */
  real_T ecu_max_dec;            /* Variable: ecu_max_dec
                                  * Referenced by: '<S4>/Constant'
                                  */
  real_T enable_abs;             /* Variable: enable_abs
                                  * Referenced by: '<S2>/abs_switch'
                                  */
  real_T slip_disable_ratio_ref; /* Variable: slip_disable_ratio_ref
                                  * Referenced by: '<S2>/OnAtEnableRef_OffAtDisableRef'
                                  */
  real_T slip_ratio_ref;         /* Variable: slip_ratio_ref
                                  * Referenced by: '<S2>/OnAtEnableRef_OffAtDisableRef'
                                  */
  real_T speedOffset;            /* Variable: speedOffset
                                  * Referenced by:
                                  *   '<S65>/Constant'
                                  *   '<S12>/Constant'
                                  *   '<S13>/Constant'
                                  */
  real_T DiscretePIDController_InitialConditionForFilter;
  /* Mask Parameter: DiscretePIDController_InitialConditionForFilter
   * Referenced by: '<S40>/Filter'
   */
  real_T DiscretePIDController_InitialConditionForIntegrator;
  /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator
   * Referenced by: '<S45>/Integrator'
   */
  real_T DiscretePIDController_LowerSaturationLimit;
  /* Mask Parameter: DiscretePIDController_LowerSaturationLimit
   * Referenced by: '<S52>/Saturation'
   */
  real_T DiscretePIDController_N; /* Mask Parameter: DiscretePIDController_N
                                   * Referenced by: '<S48>/Filter Coefficient'
                                   */
  real_T DiscretePIDController_UpperSaturationLimit;
  /* Mask Parameter: DiscretePIDController_UpperSaturationLimit
   * Referenced by: '<S52>/Saturation'
   */
  real_T CompareToConstant_const; /* Mask Parameter: CompareToConstant_const
                                   * Referenced by: '<S62>/Constant'
                                   */
  real_T brakeEnableThresh_Reverse_const;
  /* Mask Parameter: brakeEnableThresh_Reverse_const
   * Referenced by: '<S9>/Constant'
   */
  real_T brakeEnableThresh_const;            /* Mask Parameter: brakeEnableThresh_const
                                              * Referenced by: '<S8>/Constant'
                                              */
  uint8_T reverseGear_const;                 /* Mask Parameter: reverseGear_const
                                              * Referenced by: '<S11>/Constant'
                                              */
  uint8_T notReverse_const;                  /* Mask Parameter: notReverse_const
                                              * Referenced by: '<S10>/Constant'
                                              */
  real_T Switch_Threshold;                   /* Expression: 0
                                              * Referenced by: '<S5>/Switch'
                                              */
  real_T Filter_gainval;                     /* Computed Parameter: Filter_gainval
                                              * Referenced by: '<S40>/Filter'
                                              */
  real_T Integrator_gainval;                 /* Computed Parameter: Integrator_gainval
                                              * Referenced by: '<S45>/Integrator'
                                              */
  real_T abs_Threshold;                      /* Expression: 0.5
                                              * Referenced by: '<S4>/abs'
                                              */
  real_T OnAtEnableRef_OffAtDisableRef_YOn;  /* Expression: 1
                                              * Referenced by: '<S2>/OnAtEnableRef_OffAtDisableRef'
                                              */
  real_T OnAtEnableRef_OffAtDisableRef_YOff; /* Expression: 0
                                              * Referenced by: '<S2>/OnAtEnableRef_OffAtDisableRef'
                                              */
  real_T Constant1_Value[4];                 /* Expression: [1 1 1 1]
                                              * Referenced by: '<S2>/Constant1'
                                              */
  real_T Memory_InitialCondition;            /* Expression: 0
                                              * Referenced by: '<S2>/Memory'
                                              */
  real_T Saturation_UpperSat;                /* Expression: 1
                                              * Referenced by: '<S2>/Saturation'
                                              */
  real_T Saturation_LowerSat;                /* Expression: 0
                                              * Referenced by: '<S2>/Saturation'
                                              */
  real_T autoHoldBrakePressure_01_Value;     /* Expression: 0.6
                                              * Referenced by: '<S1>/autoHoldBrakePressure_01'
                                              */
  real_T Switch1_Threshold;                  /* Expression: 0.5
                                              * Referenced by: '<S1>/Switch1'
                                              */
  real_T ZeroBrakePressure_Value;            /* Expression: 0
                                              * Referenced by: '<S1>/ZeroBrakePressure'
                                              */
};

/* Real-time Model Data Structure */
struct tag_RTM_SoftABS_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;

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
    time_T *t;
    time_T tArray[2];
  } Timing;
};

/* Class declaration for model SoftABS */
class MODULE_API SoftABS {
  /* public data and function members */
 public:
  /* Copy Constructor */
  SoftABS(SoftABS const &) = delete;

  /* Assignment Operator */
  SoftABS &operator=(SoftABS const &) & = delete;

  /* Move Constructor */
  SoftABS(SoftABS &&) = delete;

  /* Move Assignment Operator */
  SoftABS &operator=(SoftABS &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_SoftABS_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_SoftABS_T *pExtU_SoftABS_T) { SoftABS_U = *pExtU_SoftABS_T; }

  /* Root outports get method */
  const ExtY_SoftABS_T &getExternalOutputs() const { return SoftABS_Y; }

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  static void terminate();

  /* Constructor */
  SoftABS();

  /* Destructor */
  ~SoftABS();

 protected:
  /* Tunable parameters */
  static P_SoftABS_T SoftABS_P;

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_SoftABS_T SoftABS_U;

  /* External outputs */
  ExtY_SoftABS_T SoftABS_Y;

  /* Block signals */
  B_SoftABS_T SoftABS_B;

  /* Block states */
  DW_SoftABS_T SoftABS_DW;

  /* Tunable parameters */
  // static P_SoftABS_T SoftABS_P;

  /* private member function(s) for subsystem '<S1>/ABS_EBD'*/
  void SoftABS_ABS_EBD_Init();
  void SoftABS_ABS_EBD_Update();
  void SoftABS_ABS_EBD();

  /* Real-Time Model */
  RT_MODEL_SoftABS_T SoftABS_M;
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
 * '<Root>' : 'SoftABS'
 * '<S1>'   : 'SoftABS/SoftABS'
 * '<S2>'   : 'SoftABS/SoftABS/ABS_EBD'
 * '<S3>'   : 'SoftABS/SoftABS/AutoHold'
 * '<S4>'   : 'SoftABS/SoftABS/ABS_EBD/accControl'
 * '<S5>'   : 'SoftABS/SoftABS/ABS_EBD/pressureControl'
 * '<S6>'   : 'SoftABS/SoftABS/ABS_EBD/accControl/BrakeSwitch'
 * '<S7>'   : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller'
 * '<S8>'   : 'SoftABS/SoftABS/ABS_EBD/accControl/BrakeSwitch/brakeEnableThresh'
 * '<S9>'   : 'SoftABS/SoftABS/ABS_EBD/accControl/BrakeSwitch/brakeEnableThresh_Reverse'
 * '<S10>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/BrakeSwitch/notReverse'
 * '<S11>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/BrakeSwitch/reverseGear'
 * '<S12>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/BrakeSwitch/vxCheck'
 * '<S13>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/BrakeSwitch/vxCheckReverse'
 * '<S14>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Anti-windup'
 * '<S15>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/D Gain'
 * '<S16>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Filter'
 * '<S17>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Filter ICs'
 * '<S18>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/I Gain'
 * '<S19>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain'
 * '<S20>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain Fdbk'
 * '<S21>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Integrator'
 * '<S22>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Integrator ICs'
 * '<S23>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/N Copy'
 * '<S24>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/N Gain'
 * '<S25>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/P Copy'
 * '<S26>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Parallel P Gain'
 * '<S27>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Reset Signal'
 * '<S28>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Saturation'
 * '<S29>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Saturation Fdbk'
 * '<S30>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Sum'
 * '<S31>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Sum Fdbk'
 * '<S32>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode'
 * '<S33>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode Sum'
 * '<S34>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Integral'
 * '<S35>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Ngain'
 * '<S36>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/postSat Signal'
 * '<S37>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/preSat Signal'
 * '<S38>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Anti-windup/Passthrough'
 * '<S39>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/D Gain/Internal Parameters'
 * '<S40>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Filter/Disc. Forward Euler Filter'
 * '<S41>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Filter ICs/Internal IC - Filter'
 * '<S42>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/I Gain/Internal Parameters'
 * '<S43>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain/Passthrough'
 * '<S44>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S45>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Integrator/Discrete'
 * '<S46>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Integrator ICs/Internal IC'
 * '<S47>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/N Copy/Disabled'
 * '<S48>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/N Gain/Internal Parameters'
 * '<S49>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/P Copy/Disabled'
 * '<S50>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Parallel P Gain/Internal Parameters'
 * '<S51>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Reset Signal/External Reset'
 * '<S52>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Saturation/Enabled'
 * '<S53>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Saturation Fdbk/Disabled'
 * '<S54>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Sum/Sum_PID'
 * '<S55>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Sum Fdbk/Disabled'
 * '<S56>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode/Disabled'
 * '<S57>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Tracking Mode Sum/Passthrough'
 * '<S58>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Integral/Passthrough'
 * '<S59>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/Tsamp - Ngain/Passthrough'
 * '<S60>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/postSat Signal/Forward_Path'
 * '<S61>'  : 'SoftABS/SoftABS/ABS_EBD/accControl/Discrete PID Controller/preSat Signal/Forward_Path'
 * '<S62>'  : 'SoftABS/SoftABS/ABS_EBD/pressureControl/Compare To Constant'
 * '<S63>'  : 'SoftABS/SoftABS/AutoHold/AutoHold'
 * '<S64>'  : 'SoftABS/SoftABS/AutoHold/EgoSpeedLow'
 * '<S65>'  : 'SoftABS/SoftABS/AutoHold/EgoSpeedLow/AutoHoldSpeedThresh'
 */
#endif /* RTW_HEADER_SoftABS_h_ */
