/*
 * TxCar_EV_data.cpp
 *
 * Code generation for model "TxCar_EV".
 *
 * Model version              : 2.1534
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul 19 14:25:50 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TxCar_EV.h"

/* Block parameters (default storage) */
P_TxCar_EV_T TxCar_EV::TxCar_EV_P{
    /* Mask Parameter: CompareToConstant_const
     * Referenced by: '<S25>/Constant'
     */
    0.1,

    /* Mask Parameter: EngSpdAct_wc
     * Referenced by: '<S17>/Constant'
     */
    20.0,

    /* Expression: 0.9*ones(1,4)
     * Referenced by: '<S24>/const_mu'
     */
    {0.9, 0.9, 0.9, 0.9},

    /* Expression: 0
     * Referenced by: '<S29>/Zero_If_RWD'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S29>/Zero_FWD'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S39>/Constant5'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S37>/Constant1'
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
     * Referenced by: '<S44>/Gain'
     */
    9.81,

    /* Expression: 0
     * Referenced by: '<S17>/Integrator1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S28>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S47>/Constant1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S19>/Gain5'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S19>/Constant1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S19>/Gain1'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S19>/Gain2'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S19>/Constant2'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S19>/Gain3'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S28>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S19>/Gain4'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S20>/N'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S20>/D'
     */
    1.0,

    /* Expression: -1
     * Referenced by: '<S20>/R'
     */
    -1.0,

    /* Expression: 0
     * Referenced by: '<S20>/P'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S47>/Constant'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S28>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S19>/Gain6'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S28>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S19>/Gain7'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S1>/Zero'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S18>/Constant'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S18>/Constant1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S18>/Constant2'
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
     * Referenced by: '<S42>/Gain2'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S28>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S28>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S45>/Gain'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S36>/FD'
     */
    1.0,

    /* Expression: 3
     * Referenced by: '<S36>/4WD'
     */
    3.0,

    /* Expression: 60/2/3.1415
     * Referenced by: '<S36>/Gain2'
     */
    9.5495782269616427,

    /* Expression: 0
     * Referenced by: '<S29>/Memory1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S29>/Gain3'
     */
    1.0,

    /* Expression: 1000
     * Referenced by: '<S29>/Saturation'
     */
    1000.0,

    /* Expression: -1000
     * Referenced by: '<S29>/Saturation'
     */
    -1000.0,

    /* Expression: 30+273.15
     * Referenced by: '<S29>/Constant'
     */
    303.15,

    /* Expression: 2
     * Referenced by: '<S38>/RD'
     */
    2.0,

    /* Expression: 3
     * Referenced by: '<S38>/4WD'
     */
    3.0,

    /* Expression: 60/2/3.1415
     * Referenced by: '<S38>/Gain4'
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
     * Referenced by: '<S40>/Dead Zone'
     */
    -0.1,

    /* Expression: 0.1
     * Referenced by: '<S40>/Dead Zone'
     */
    0.1,

    /* Expression: 30
     * Referenced by: '<S40>/AxleTrqOffset'
     */
    30.0,

    /* Expression: 0
     * Referenced by: '<S42>/Constant'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S42>/Gain'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S42>/Gain1'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S45>/Gain1'
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
     * Referenced by: '<S43>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S43>/S-Function'
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
     * Referenced by: '<S16>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S16>/S-Function'
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
     * Referenced by: '<S14>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S14>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S31>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S31>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S32>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S32>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S33>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S33>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S48>/Steer_Mapped'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S48>/Steer_Mapped'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110}};
