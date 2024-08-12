/*
 * SoftECU_data.cpp
 *
 * Code generation for model "SoftECU".
 *
 * Model version              : 2.90
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul  5 17:08:36 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "SoftECU.h"

/* Block parameters (default storage) */
P_SoftECU_T SoftECU::SoftECU_P{
    /* Variable: DriveLinePowType
     * Referenced by:
     *   '<S1>/Constant1'
     *   '<S3>/Constant2'
     */
    2.0,

    /* Variable: EngMaxTrq
     * Referenced by:
     *   '<S4>/Gain'
     *   '<S4>/Saturation1'
     *   '<S75>/Constant'
     */
    514.89,

    /* Variable: MotorMaxTrq
     * Referenced by:
     *   '<S5>/Constant'
     *   '<S5>/Saturation1'
     *   '<S139>/Constant1'
     *   '<S141>/Constant1'
     */
    760.0,

    /* Variable: MotorPowerRegenerationRatio
     * Referenced by: '<S139>/Constant4'
     */
    0.3,

    /* Variable: abs_brake_pressure
     * Referenced by:
     *   '<S8>/abs_brake_pressure_01'
     *   '<S9>/abs_brake_pressure_01'
     */
    0.2,

    /* Variable: brake_D
     * Referenced by: '<S37>/Derivative Gain'
     */
    0.0,

    /* Variable: brake_I
     * Referenced by: '<S40>/Integral Gain'
     */
    0.02,

    /* Variable: brake_P
     * Referenced by: '<S48>/Proportional Gain'
     */
    0.1,

    /* Variable: ecu_max_acc
     * Referenced by:
     *   '<S74>/Constant'
     *   '<S141>/Constant'
     */
    10.0,

    /* Variable: ecu_max_dec
     * Referenced by: '<S8>/Constant'
     */
    -10.0,

    /* Variable: enable_abs
     * Referenced by: '<S2>/abs_switch'
     */
    0.0,

    /* Variable: eng_acc_D
     * Referenced by: '<S107>/Derivative Gain'
     */
    0.0,

    /* Variable: eng_acc_I
     * Referenced by: '<S110>/Integral Gain'
     */
    0.035,

    /* Variable: eng_acc_P
     * Referenced by: '<S118>/Proportional Gain'
     */
    0.37,

    /* Variable: slip_disable_ratio_ref
     * Referenced by: '<S2>/OnAtRef_OffAt0.5Ref'
     */
    0.1,

    /* Variable: slip_ratio_ref
     * Referenced by: '<S2>/OnAtRef_OffAt0.5Ref'
     */
    0.25,

    /* Variable: softecu_ax_delay
     * Referenced by: '<S196>/AxDelayStep'
     */
    5.0,

    /* Variable: speedOffset
     * Referenced by:
     *   '<S69>/Constant'
     *   '<S64>/Constant'
     *   '<S65>/Constant'
     */
    0.01,

    /* Variable: stepTime
     * Referenced by: '<S196>/Gain'
     */
    0.001,

    /* Variable: vcu_acc_D
     * Referenced by: '<S170>/Derivative Gain'
     */
    0.0,

    /* Variable: vcu_acc_I
     * Referenced by: '<S173>/Integral Gain'
     */
    0.02,

    /* Variable: vcu_acc_P
     * Referenced by: '<S181>/Proportional Gain'
     */
    0.4,

    /* Mask Parameter: DiscretePIDController_InitialConditionForFilter
     * Referenced by: '<S38>/Filter'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_InitialConditionForFilter_e
     * Referenced by: '<S108>/Filter'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_InitialConditionForFilter_b
     * Referenced by: '<S171>/Filter'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator
     * Referenced by: '<S43>/Integrator'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator_k
     * Referenced by: '<S113>/Integrator'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator_h
     * Referenced by: '<S176>/Integrator'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_LowerSaturationLimit
     * Referenced by: '<S50>/Saturation'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_LowerSaturationLimit_o
     * Referenced by: '<S120>/Saturation'
     */
    -1.0,

    /* Mask Parameter: DiscretePIDController_LowerSaturationLimit_k
     * Referenced by: '<S183>/Saturation'
     */
    -1.0,

    /* Mask Parameter: DiscretePIDController_N
     * Referenced by: '<S46>/Filter Coefficient'
     */
    100.0,

    /* Mask Parameter: DiscretePIDController_N_i
     * Referenced by: '<S116>/Filter Coefficient'
     */
    100.0,

    /* Mask Parameter: DiscretePIDController_N_k
     * Referenced by: '<S179>/Filter Coefficient'
     */
    100.0,

    /* Mask Parameter: DiscretePIDController_UpperSaturationLimit
     * Referenced by: '<S50>/Saturation'
     */
    1.0,

    /* Mask Parameter: DiscretePIDController_UpperSaturationLimit_m
     * Referenced by: '<S120>/Saturation'
     */
    1.0,

    /* Mask Parameter: DiscretePIDController_UpperSaturationLimit_f
     * Referenced by: '<S183>/Saturation'
     */
    1.0,

    /* Mask Parameter: CompareToConstant_const
     * Referenced by: '<S66>/Constant'
     */
    1.0E-9,

    /* Mask Parameter: brakeEnableThresh_Reverse_const
     * Referenced by: '<S61>/Constant'
     */
    0.005,

    /* Mask Parameter: brakeEnableThresh_const
     * Referenced by: '<S60>/Constant'
     */
    -0.005,

    /* Mask Parameter: CompareToConstant1_const
     * Referenced by: '<S80>/Constant'
     */
    0.1,

    /* Mask Parameter: CompareToConstant_const_f
     * Referenced by: '<S76>/Constant'
     */
    0.1,

    /* Mask Parameter: CompareToConstant1_const_j
     * Referenced by: '<S77>/Constant'
     */
    1.0E-5,

    /* Mask Parameter: CompareToConstant_const_b
     * Referenced by: '<S71>/Constant'
     */
    0.0,

    /* Mask Parameter: CompareToConstant_const_c
     * Referenced by: '<S142>/Constant'
     */
    0.1,

    /* Mask Parameter: speedLow_xms_const
     * Referenced by: '<S144>/Constant'
     */
    1.3888888888888888,

    /* Mask Parameter: CompareToConstant_const_e
     * Referenced by: '<S138>/Constant'
     */
    1.3888888888888888,

    /* Mask Parameter: CompareToConstant_const_k
     * Referenced by: '<S135>/Constant'
     */
    0.1,

    /* Mask Parameter: CompareToConstant1_const_c
     * Referenced by: '<S136>/Constant'
     */
    1.0E-5,

    /* Mask Parameter: reverseGear_const
     * Referenced by: '<S63>/Constant'
     */
    2U,

    /* Mask Parameter: notReverse_const
     * Referenced by: '<S62>/Constant'
     */
    2U,

    /* Mask Parameter: reverseGear1_const
     * Referenced by: '<S79>/Constant'
     */
    2U,

    /* Mask Parameter: forwardGear_const
     * Referenced by: '<S78>/Constant'
     */
    1U,

    /* Mask Parameter: CompareToConstant1_const_p
     * Referenced by: '<S193>/Constant'
     */
    1U,

    /* Mask Parameter: CompareToConstant2_const
     * Referenced by: '<S194>/Constant'
     */
    2U,

    /* Mask Parameter: CompareToConstant_const_l
     * Referenced by: '<S137>/Constant'
     */
    1U,

    /* Expression: 0
     * Referenced by: '<S9>/Switch'
     */
    0.0,

    /* Computed Parameter: Filter_gainval
     * Referenced by: '<S38>/Filter'
     */
    0.001,

    /* Computed Parameter: Integrator_gainval
     * Referenced by: '<S43>/Integrator'
     */
    1.0,

    /* Expression: 0.5
     * Referenced by: '<S8>/abs'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S2>/OnAtRef_OffAt0.5Ref'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S2>/OnAtRef_OffAt0.5Ref'
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

    /* Expression: 0
     * Referenced by: '<S74>/Constant2'
     */
    0.0,

    /* Computed Parameter: Filter_gainval_p
     * Referenced by: '<S108>/Filter'
     */
    0.001,

    /* Computed Parameter: Integrator_gainval_k
     * Referenced by: '<S113>/Integrator'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S73>/Saturation'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S73>/Saturation'
     */
    0.0,

    /* Expression: -1
     * Referenced by: '<S72>/fliper'
     */
    -1.0,

    /* Expression: 0
     * Referenced by: '<S4>/no_throttle'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S4>/Saturation'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S4>/Saturation'
     */
    0.0,

    /* Expression: 0.05
     * Referenced by: '<S141>/Constant2'
     */
    0.05,

    /* Computed Parameter: Filter_gainval_k
     * Referenced by: '<S171>/Filter'
     */
    0.001,

    /* Computed Parameter: Integrator_gainval_i
     * Referenced by: '<S176>/Integrator'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S141>/cutoffMotorOutput'
     */
    0.0,

    /* Expression: -1
     * Referenced by: '<S132>/Gain'
     */
    -1.0,

    /* Expression: 0
     * Referenced by: '<S132>/Constant'
     */
    0.0,

    /* Expression: 0.05
     * Referenced by: '<S132>/Constant2'
     */
    0.05,

    /* Expression: 0.9
     * Referenced by: '<S132>/Constant3'
     */
    0.9,

    /* Expression: 0.4
     * Referenced by: '<S139>/averageTorqueCoeff'
     */
    0.4,

    /* Expression: 1
     * Referenced by: '<S5>/Saturation'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S5>/Saturation'
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

    /* Expression: -5
     * Referenced by: '<S7>/RelayCutOffAt6_EnableAt5'
     */
    -5.0,

    /* Expression: -6
     * Referenced by: '<S7>/RelayCutOffAt6_EnableAt5'
     */
    -6.0,

    /* Expression: 0
     * Referenced by: '<S7>/RelayCutOffAt6_EnableAt5'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S7>/RelayCutOffAt6_EnableAt5'
     */
    1.0,

    /* Expression: 58
     * Referenced by: '<S7>/RelayCutOffAt58_EnableAt55'
     */
    58.0,

    /* Expression: 55
     * Referenced by: '<S7>/RelayCutOffAt58_EnableAt55'
     */
    55.0,

    /* Expression: 1
     * Referenced by: '<S7>/RelayCutOffAt58_EnableAt55'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S7>/RelayCutOffAt58_EnableAt55'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S1>/Constant'
     */
    0.0,

    /* Start of '<S1>/ax_estimator' */
    {/* Mask Parameter: SpeedThresh_const
      * Referenced by: '<S198>/Constant'
      */
     1.4,

     /* Mask Parameter: AxAct_wc
      * Referenced by: '<S195>/Constant'
      */
     50.0,

     /* Expression: 0.0
      * Referenced by: '<S196>/Delay'
      */
     0.0,

     /* Expression: 0
      * Referenced by: '<S195>/Integrator1'
      */
     0.0}
    /* End of '<S1>/ax_estimator' */
};
