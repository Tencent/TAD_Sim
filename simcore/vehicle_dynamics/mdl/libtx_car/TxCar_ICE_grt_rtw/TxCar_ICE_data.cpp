/*
 * TxCar_ICE_data.cpp
 *
 * Code generation for model "TxCar_ICE".
 *
 * Model version              : 2.1530
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul 19 14:26:25 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TxCar_ICE.h"

/* Block parameters (default storage) */
P_TxCar_ICE_T TxCar_ICE::TxCar_ICE_P{
    /* Mask Parameter: CompareToConstant_const
     * Referenced by: '<S24>/Constant'
     */
    0.1,

    /* Mask Parameter: EngSpdAct_wc
     * Referenced by: '<S16>/Constant'
     */
    20.0,

    /* Expression: 0.9*ones(1, 4)
     * Referenced by: '<S23>/const_mu'
     */
    {0.9, 0.9, 0.9, 0.9},

    /* Expression: 0
     * Referenced by: '<S31>/none'
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
     * Referenced by: '<S8>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S8>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S8>/Memory'
     */
    0.0,

    /* Expression: 9.81
     * Referenced by: '<S36>/Gain'
     */
    9.81,

    /* Expression: 0
     * Referenced by: '<S16>/Integrator1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S27>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S39>/Constant1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S18>/Gain5'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S18>/Constant1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S18>/Gain1'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S18>/Gain2'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S18>/Constant2'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S18>/Gain3'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S27>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S18>/Gain4'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S19>/N'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S19>/D'
     */
    1.0,

    /* Expression: -1
     * Referenced by: '<S19>/R'
     */
    -1.0,

    /* Expression: 0
     * Referenced by: '<S19>/P'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S39>/Constant'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S27>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S18>/Gain6'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S27>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S18>/Gain7'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S17>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S11>/Constant2'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S11>/Constant3'
     */
    0.0,

    /* Expression: 5000
     * Referenced by: '<S11>/Saturation'
     */
    5000.0,

    /* Expression: -5000
     * Referenced by: '<S11>/Saturation'
     */
    -5000.0,

    /* Expression: 1
     * Referenced by: '<S34>/Gain2'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S27>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S27>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S1>/Zero'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S37>/Gain1'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S30>/none'
     */
    0.0,

    /* Expression: 60/2/3.1415
     * Referenced by: '<S30>/Gain'
     */
    9.5495782269616427,

    /* Expression: 45/180.0*pi
     * Referenced by: '<S8>/Saturation'
     */
    0.78539816339744828,

    /* Expression: -45/180.0*pi
     * Referenced by: '<S8>/Saturation'
     */
    -0.78539816339744828,

    /* Expression: -0.1
     * Referenced by: '<S32>/Dead Zone'
     */
    -0.1,

    /* Expression: 0.1
     * Referenced by: '<S32>/Dead Zone'
     */
    0.1,

    /* Expression: 30
     * Referenced by: '<S32>/AxleTrqOffset'
     */
    30.0,

    /* Expression: 0
     * Referenced by: '<S34>/Constant'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S34>/Gain'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S34>/Gain1'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S37>/Gain'
     */
    1.0,

    /* Expression: int8(lic_path)
     * Referenced by: '<S2>/Chassis'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S2>/Chassis'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S35>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S35>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S15>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S15>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S17>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S17>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S14>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S14>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S1>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S1>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S29>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S29>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S40>/Steer_Mapped'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S40>/Steer_Mapped'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110}};
