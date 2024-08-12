/*
 * TX_DriveLine_EV_Rear_data.cpp
 *
 * Code generation for model "TX_DriveLine_EV_Rear".
 *
 * Model version              : 1.205
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul 19 10:41:20 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TX_DriveLine_EV_Rear.h"

/* Block parameters (default storage) */
P_TX_DriveLine_EV_Rear_T TX_DriveLine_EV_Rear::TX_DriveLine_EV_Rear_P{
    /* Variable: VEH
     * Referenced by:
     *   '<S7>/Constant'
     *   '<S7>/Constant1'
     */
    {1830.0,

     {481.50000000000006, 481.50000000000006},

     {420.0, 420.0},
     3.05,
     1.4,
     1.6,
     0.166,
     2.11,
     0.33,
     2.0,
     2788.0,
     928.0,
     3234.0,
     18.0,
     1650.0,
     0.0,
     0.0,
     0.0,
     0.0,
     0.0,
     0.0,
     22.222222222222221,
     0.0,
     0.0,
     0.0,
     0.0,
     0.0,
     90.0,
     90.0,
     1.6,
     1.6},

    /* Variable: StatLdWhlR
     * Referenced by:
     *   '<S7>/Constant'
     *   '<S7>/Constant1'
     */
    {0.364, 0.364, 0.364, 0.364},

    /* Variable: ratio_diff_rear
     * Referenced by:
     *   '<S7>/Ndiff2'
     *   '<S13>/Gain'
     *   '<S16>/Gain'
     */
    2.65,

    /* Variable: rear_motor_gear_ratio
     * Referenced by:
     *   '<S3>/Gain'
     *   '<S3>/Gain1'
     */
    3.7,

    /* Mask Parameter: EVRearLimitedSlipDifferential_Fc
     * Referenced by: '<S21>/Constant2'
     */
    1000.0,

    /* Mask Parameter: EVRearLimitedSlipDifferential_Jd
     * Referenced by: '<S7>/Jd'
     */
    0.08,

    /* Mask Parameter: EVRearLimitedSlipDifferential_Jw1
     * Referenced by: '<S7>/Jw1'
     */
    0.05,

    /* Mask Parameter: EVRearLimitedSlipDifferential_Jw2
     * Referenced by: '<S7>/Jw3'
     */
    0.05,

    /* Mask Parameter: EVRearLimitedSlipDifferential_Ndisks
     * Referenced by: '<S21>/Constant1'
     */
    8.0,

    /* Mask Parameter: EVRearLimitedSlipDifferential_Reff
     * Referenced by: '<S21>/Constant'
     */
    0.08,

    /* Mask Parameter: TorsionalCompliance_b
     * Referenced by: '<S33>/Gain2'
     */
    50.0,

    /* Mask Parameter: TorsionalCompliance1_b
     * Referenced by: '<S41>/Gain2'
     */
    50.0,

    /* Mask Parameter: EVRearLimitedSlipDifferential_bd
     * Referenced by: '<S7>/bd'
     */
    0.001,

    /* Mask Parameter: EVRearLimitedSlipDifferential_bw1
     * Referenced by: '<S7>/bw1'
     */
    0.001,

    /* Mask Parameter: EVRearLimitedSlipDifferential_bw2
     * Referenced by: '<S7>/bw2'
     */
    0.001,

    /* Mask Parameter: TorsionalCompliance_domega_o
     * Referenced by: '<S33>/domega_o'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance1_domega_o
     * Referenced by: '<S41>/domega_o'
     */
    0.0,

    /* Mask Parameter: EVRearLimitedSlipDifferential_dw
     * Referenced by: '<S21>/mu Table'
     */
    {0.0, 10.0, 20.0, 40.0, 60.0, 80.0, 100.0, 1000.0},

    /* Mask Parameter: EVRearLimitedSlipDifferential_eta
     * Referenced by: '<S32>/Constant'
     */
    1.0,

    /* Mask Parameter: TorsionalCompliance_k
     * Referenced by: '<S33>/Gain1'
     */
    5000.0,

    /* Mask Parameter: TorsionalCompliance1_k
     * Referenced by: '<S41>/Gain1'
     */
    5000.0,

    /* Mask Parameter: EVRearLimitedSlipDifferential_muc
     * Referenced by: '<S21>/mu Table'
     */
    {0.16, 0.13, 0.115, 0.11, 0.105, 0.1025, 0.10125, 0.10125},

    /* Mask Parameter: TorsionalCompliance_omega_c
     * Referenced by: '<S33>/omega_c'
     */
    150.0,

    /* Mask Parameter: TorsionalCompliance1_omega_c
     * Referenced by: '<S41>/omega_c'
     */
    150.0,

    /* Mask Parameter: EVRearLimitedSlipDifferential_tauC
     * Referenced by: '<S7>/Constant3'
     */
    0.04,

    /* Mask Parameter: TorsionalCompliance_theta_o
     * Referenced by: '<S33>/Integrator'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance1_theta_o
     * Referenced by: '<S41>/Integrator'
     */
    0.0,

    /* Expression: shaftSwitchMask
     * Referenced by: '<S7>/Limited Slip Differential'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S1>/Constant'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S1>/Constant1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S34>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S42>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S1>/Constant2'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S3>/Constant'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S13>/Constant1'
     */
    1.0,

    /* Expression: shaftSwitchMask
     * Referenced by: '<S13>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S13>/Switch'
     */
    1.0,

    /* Expression: maxAbsSpd
     * Referenced by: '<S7>/Integrator'
     */
    5000.0,

    /* Expression: -maxAbsSpd
     * Referenced by: '<S7>/Integrator'
     */
    -5000.0,

    /* Expression: 2*pi
     * Referenced by: '<S7>/2*pi'
     */
    6.2831853071795862,

    /* Expression: 1/2
     * Referenced by: '<S13>/Gain1'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S24>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S24>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S31>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S31>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S31>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S31>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S30>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S22>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S22>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S27>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S27>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S27>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S27>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S26>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S23>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S23>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S29>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S29>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S29>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S29>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S28>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S9>/Integrator'
     */
    0.0,

    /* Expression: shaftSwitchMask
     * Referenced by: '<S16>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S16>/Constant2'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S16>/Switch'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S21>/Gain'
     */
    4.0,

    /* Expression: 1
     * Referenced by: '<S34>/Reset'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S42>/Reset'
     */
    1.0};
