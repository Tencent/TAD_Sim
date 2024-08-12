/*
 * SoftABS_data.cpp
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

#include "SoftABS.h"

/* Block parameters (default storage) */
P_SoftABS_T SoftABS::SoftABS_P{/* Variable: DriveLinePowType
                                * Referenced by: '<S3>/Constant2'
                                */
                               2.0,

                               /* Variable: abs_brake_pressure
                                * Referenced by:
                                *   '<S4>/abs_brake_pressure_01'
                                *   '<S5>/abs_brake_pressure_01'
                                */
                               0.2,

                               /* Variable: brake_D
                                * Referenced by: '<S39>/Derivative Gain'
                                */
                               0.0,

                               /* Variable: brake_I
                                * Referenced by: '<S42>/Integral Gain'
                                */
                               0.02,

                               /* Variable: brake_P
                                * Referenced by: '<S50>/Proportional Gain'
                                */
                               0.1,

                               /* Variable: ecu_max_dec
                                * Referenced by: '<S4>/Constant'
                                */
                               -10.0,

                               /* Variable: enable_abs
                                * Referenced by: '<S2>/abs_switch'
                                */
                               0.0,

                               /* Variable: slip_disable_ratio_ref
                                * Referenced by: '<S2>/OnAtEnableRef_OffAtDisableRef'
                                */
                               0.1,

                               /* Variable: slip_ratio_ref
                                * Referenced by: '<S2>/OnAtEnableRef_OffAtDisableRef'
                                */
                               0.25,

                               /* Variable: speedOffset
                                * Referenced by:
                                *   '<S65>/Constant'
                                *   '<S12>/Constant'
                                *   '<S13>/Constant'
                                */
                               0.01,

                               /* Mask Parameter: DiscretePIDController_InitialConditionForFilter
                                * Referenced by: '<S40>/Filter'
                                */
                               0.0,

                               /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator
                                * Referenced by: '<S45>/Integrator'
                                */
                               0.0,

                               /* Mask Parameter: DiscretePIDController_LowerSaturationLimit
                                * Referenced by: '<S52>/Saturation'
                                */
                               0.0,

                               /* Mask Parameter: DiscretePIDController_N
                                * Referenced by: '<S48>/Filter Coefficient'
                                */
                               100.0,

                               /* Mask Parameter: DiscretePIDController_UpperSaturationLimit
                                * Referenced by: '<S52>/Saturation'
                                */
                               1.0,

                               /* Mask Parameter: CompareToConstant_const
                                * Referenced by: '<S62>/Constant'
                                */
                               1.0E-9,

                               /* Mask Parameter: brakeEnableThresh_Reverse_const
                                * Referenced by: '<S9>/Constant'
                                */
                               0.005,

                               /* Mask Parameter: brakeEnableThresh_const
                                * Referenced by: '<S8>/Constant'
                                */
                               -0.005,

                               /* Mask Parameter: reverseGear_const
                                * Referenced by: '<S11>/Constant'
                                */
                               2U,

                               /* Mask Parameter: notReverse_const
                                * Referenced by: '<S10>/Constant'
                                */
                               2U,

                               /* Expression: 0
                                * Referenced by: '<S5>/Switch'
                                */
                               0.0,

                               /* Computed Parameter: Filter_gainval
                                * Referenced by: '<S40>/Filter'
                                */
                               0.001,

                               /* Computed Parameter: Integrator_gainval
                                * Referenced by: '<S45>/Integrator'
                                */
                               1.0,

                               /* Expression: 0.5
                                * Referenced by: '<S4>/abs'
                                */
                               0.5,

                               /* Expression: 1
                                * Referenced by: '<S2>/OnAtEnableRef_OffAtDisableRef'
                                */
                               1.0,

                               /* Expression: 0
                                * Referenced by: '<S2>/OnAtEnableRef_OffAtDisableRef'
                                */
                               0.0,

                               /* Expression: [1 1 1 1]
                                * Referenced by: '<S2>/Constant1'
                                */
                               {1.0, 1.0, 1.0, 1.0},

                               /* Expression: 0
                                * Referenced by: '<S2>/Memory'
                                */
                               0.0,

                               /* Expression: 1
                                * Referenced by: '<S2>/Saturation'
                                */
                               1.0,

                               /* Expression: 0
                                * Referenced by: '<S2>/Saturation'
                                */
                               0.0,

                               /* Expression: 0.6
                                * Referenced by: '<S1>/autoHoldBrakePressure_01'
                                */
                               0.6,

                               /* Expression: 0.5
                                * Referenced by: '<S1>/Switch1'
                                */
                               0.5,

                               /* Expression: 0
                                * Referenced by: '<S1>/ZeroBrakePressure'
                                */
                               0.0};
