/*
 * TxCar_Hybrid_data.cpp
 *
 * Code generation for model "TxCar_Hybrid".
 *
 * Model version              : 2.2139
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Fri Jul 28 13:31:29 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TxCar_Hybrid.h"

/* Block parameters (default storage) */
P_TxCar_Hybrid_T TxCar_Hybrid::TxCar_Hybrid_P{
    /* Mask Parameter: Throttle_PID_D
     * Referenced by: '<S89>/Derivative Gain'
     */
    0.0,

    /* Mask Parameter: Throttle_PID_I
     * Referenced by: '<S92>/Integral Gain'
     */
    0.01,

    /* Mask Parameter: Throttle_PID_InitialConditionForFilter
     * Referenced by: '<S90>/Filter'
     */
    0.0,

    /* Mask Parameter: Throttle_PID_InitialConditionForIntegrator
     * Referenced by: '<S95>/Integrator'
     */
    0.0,

    /* Mask Parameter: Throttle_PID_LowerSaturationLimit
     * Referenced by: '<S102>/Saturation'
     */
    0.0,

    /* Mask Parameter: DivProtect_MinVal
     * Referenced by:
     *   '<S56>/Constant'
     *   '<S57>/Constant'
     */
    1.0,

    /* Mask Parameter: DivProtect_MinVal_e
     * Referenced by:
     *   '<S59>/Constant'
     *   '<S60>/Constant'
     */
    1.0,

    /* Mask Parameter: Throttle_PID_N
     * Referenced by: '<S98>/Filter Coefficient'
     */
    100.0,

    /* Mask Parameter: Throttle_PID_P
     * Referenced by: '<S100>/Proportional Gain'
     */
    0.1,

    /* Mask Parameter: Throttle_PID_UpperSaturationLimit
     * Referenced by: '<S102>/Saturation'
     */
    1.0,

    /* Mask Parameter: CompareToConstant_const
     * Referenced by: '<S37>/Constant'
     */
    0.1,

    /* Mask Parameter: EngSpdAct_wc
     * Referenced by: '<S18>/Constant'
     */
    20.0,

    /* Mask Parameter: FrontMotorSpd_Filter_wc
     * Referenced by: '<S20>/Constant'
     */
    20.0,

    /* Mask Parameter: RearMotorSpd_Filter_wc
     * Referenced by: '<S25>/Constant'
     */
    20.0,

    /* Mask Parameter: TransOutSpd_Filter_wc
     * Referenced by: '<S26>/Constant'
     */
    20.0,

    /* Mask Parameter: LPF_wc
     * Referenced by: '<S62>/Constant'
     */
    50.0,

    /* Expression: 0.9*ones(1,4)
     * Referenced by: '<S36>/const_mu'
     */
    {0.9, 0.9, 0.9, 0.9},

    /* Expression: 0
     * Referenced by: '<S55>/Zero_Torque'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S56>/Constant1'
     */
    1.0,

    /* Expression: -1
     * Referenced by: '<S56>/Constant2'
     */
    -1.0,

    /* Expression: 0
     * Referenced by: '<S58>/Constant'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S59>/Constant1'
     */
    1.0,

    /* Expression: -1
     * Referenced by: '<S59>/Constant2'
     */
    -1.0,

    /* Expression: 30/pi
     * Referenced by: '<S48>/RPM'
     */
    9.5492965855137211,

    /* Expression: 0
     * Referenced by: '<S61>/Constant'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S5>/Constant3'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S5>/Constant4'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S5>/Constant5'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S9>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S9>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S9>/Memory'
     */
    0.0,

    /* Expression: 9.81
     * Referenced by: '<S115>/Gain'
     */
    9.81,

    /* Expression: 0
     * Referenced by: '<S18>/Integrator1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S40>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S119>/Constant1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S20>/Integrator1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S25>/Integrator1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S28>/Gain5'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S28>/Constant1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S28>/Gain1'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S28>/Gain2'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S28>/Constant2'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S28>/Gain3'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S40>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S28>/Gain4'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S32>/N'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S32>/D'
     */
    1.0,

    /* Expression: -1
     * Referenced by: '<S32>/R'
     */
    -1.0,

    /* Expression: 0
     * Referenced by: '<S32>/P'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S119>/Constant'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S40>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S28>/Gain6'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S40>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S6>/NoUse'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S28>/Gain7'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S27>/Memory'
     */
    0.0,

    /* Computed Parameter: Merge_InitialOutput
     * Referenced by: '<S19>/Merge'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S21>/Constant3'
     */
    0.0,

    /* Expression: 5000
     * Referenced by: '<S12>/Saturation'
     */
    5000.0,

    /* Expression: -5000
     * Referenced by: '<S12>/Saturation'
     */
    -5000.0,

    /* Expression: 0
     * Referenced by: '<S26>/Integrator1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S114>/Gain2'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S40>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S40>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S44>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S62>/Integrator1'
     */
    0.0,

    /* Expression: pi/30
     * Referenced by: '<S44>/rads'
     */
    0.10471975511965977,

    /* Expression: 0
     * Referenced by: '<S41>/Memory1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S41>/Gain3'
     */
    1.0,

    /* Expression: 1000
     * Referenced by: '<S41>/Saturation'
     */
    1000.0,

    /* Expression: -1000
     * Referenced by: '<S41>/Saturation'
     */
    -1000.0,

    /* Expression: 30+273.15
     * Referenced by: '<S41>/Constant'
     */
    303.15,

    /* Expression: 300000
     * Referenced by: '<S46>/GeneratorPwrSaturation'
     */
    300000.0,

    /* Expression: 0
     * Referenced by: '<S46>/GeneratorPwrSaturation'
     */
    0.0,

    /* Expression: -1
     * Referenced by: '<S46>/Gain'
     */
    -1.0,

    /* Expression: 60/2/3.1415
     * Referenced by: '<S50>/Gain2'
     */
    9.5495782269616427,

    /* Expression: 60/2/3.1415
     * Referenced by: '<S52>/Gain4'
     */
    9.5495782269616427,

    /* Expression: 0.5
     * Referenced by: '<S49>/Pwr_Dmnd'
     */
    0.5,

    /* Expression: 7000
     * Referenced by: '<S49>/Saturation'
     */
    7000.0,

    /* Expression: 750
     * Referenced by: '<S49>/Saturation'
     */
    750.0,

    /* Expression: pi/30.0
     * Referenced by: '<S49>/rad|s'
     */
    0.10471975511965977,

    /* Expression: 40000
     * Referenced by: '<S49>/Const_Pwr'
     */
    40000.0,

    /* Computed Parameter: Integrator_gainval
     * Referenced by: '<S95>/Integrator'
     */
    1.0,

    /* Computed Parameter: Filter_gainval
     * Referenced by: '<S90>/Filter'
     */
    0.001,

    /* Expression: 0.5
     * Referenced by: '<S48>/Switch'
     */
    0.5,

    /* Expression: 45/180.0*pi
     * Referenced by: '<S9>/Saturation'
     */
    0.78539816339744828,

    /* Expression: -45/180.0*pi
     * Referenced by: '<S9>/Saturation'
     */
    -0.78539816339744828,

    /* Expression: -0.1
     * Referenced by: '<S112>/Dead Zone'
     */
    -0.1,

    /* Expression: 0.1
     * Referenced by: '<S112>/Dead Zone'
     */
    0.1,

    /* Expression: 30
     * Referenced by: '<S112>/AxleTrqOffset'
     */
    30.0,

    /* Expression: 0
     * Referenced by: '<S114>/Constant'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S114>/Gain'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S114>/Gain1'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S117>/Gain'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S117>/Gain1'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S1>/Zero'
     */
    0.0,

    /* Expression: int8(lic_path)
     * Referenced by: '<S2>/Chassis'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S2>/Chassis'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S120>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S120>/S-Function'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S6>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S6>/S-Function'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S16>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S16>/S-Function'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S17>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S17>/S-Function'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S27>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S27>/S-Function'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S15>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S15>/S-Function'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S42>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S42>/S-Function'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S45>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S45>/S-Function'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S47>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S47>/S-Function'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S43>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S43>/S-Function'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S121>/Steer_Mapped'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S121>/Steer_Mapped'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S1>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S1>/S-Function'
     */
    {46,  46,  47,  46,  46, 47,  112, 97, 114, 97,  109, 47,  116, 120, 99, 97,  114, 95,  116,
     101, 109, 112, 108, 97, 116, 101, 95, 104, 121, 98,  114, 105, 100, 46, 106, 115, 111, 110}};
