/*
 * TxCar_data.cpp
 *
 * Code generation for model "TxCar".
 *
 * Model version              : 2.1095
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Thu May 11 21:28:18 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TxCar.h"

/* Block parameters (default storage) */
P_TxCar_T TxCar::TxCar_P{
    /* Mask Parameter: CompareToConstant_const
     * Referenced by: '<S22>/Constant'
     */
    0.1,

    /* Mask Parameter: EngSpdAct_wc
     * Referenced by: '<S15>/Constant'
     */
    20.0,

    /* Computed Parameter: Memory_InitialCondition
     * Referenced by: '<S9>/Memory'
     */
    {
        {0.0, 0.0, 0.0, 0.0}, /* wheel_angle */
        0.0,                  /* steerWheelFeedback */
        0.0                   /* steerTorqueInput */
    },

    /* Expression: 0.9*ones(1,4)
     * Referenced by: '<S21>/const_mu'
     */
    {0.9, 0.9, 0.9, 0.9},

    /* Expression: 0
     * Referenced by: '<S26>/Constant3'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S26>/Constant4'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S38>/Constant5'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S33>/none'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S36>/Constant1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S4>/Constant3'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S4>/Constant4'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S4>/Constant5'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S7>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S7>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S7>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S42>/Gain'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S15>/Integrator1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S25>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S45>/Constant1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S16>/Gain5'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S16>/Constant1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S16>/Gain1'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S16>/Gain2'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S16>/Constant2'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S16>/Gain3'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S25>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S16>/Gain4'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S17>/N'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S17>/D'
     */
    1.0,

    /* Expression: -1
     * Referenced by: '<S17>/R'
     */
    -1.0,

    /* Expression: 0
     * Referenced by: '<S17>/P'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S45>/Constant'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S25>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S16>/Gain6'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S25>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S16>/Gain7'
     */
    1.0,

    /* Expression: 5000
     * Referenced by: '<S10>/Saturation'
     */
    5000.0,

    /* Expression: -5000
     * Referenced by: '<S10>/Saturation'
     */
    -5000.0,

    /* Expression: 1
     * Referenced by: '<S40>/Gain2'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S25>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S25>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S28>/Zero'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S43>/Gain1'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S32>/none'
     */
    0.0,

    /* Expression: 60/2/3.1415
     * Referenced by: '<S32>/Gain'
     */
    9.5495782269616427,

    /* Expression: 0
     * Referenced by: '<S26>/Memory2'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S35>/FD'
     */
    1.0,

    /* Expression: 3
     * Referenced by: '<S35>/4WD'
     */
    3.0,

    /* Expression: 60/2/3.1415
     * Referenced by: '<S35>/Gain2'
     */
    9.5495782269616427,

    /* Expression: 0
     * Referenced by: '<S26>/Memory1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S26>/Gain3'
     */
    1.0,

    /* Expression: 1000
     * Referenced by: '<S26>/Saturation'
     */
    1000.0,

    /* Expression: -1000
     * Referenced by: '<S26>/Saturation'
     */
    -1000.0,

    /* Expression: 30+273.15
     * Referenced by: '<S26>/Constant'
     */
    303.15,

    /* Expression: 0
     * Referenced by: '<S26>/Memory'
     */
    0.0,

    /* Expression: 2
     * Referenced by: '<S37>/RD'
     */
    2.0,

    /* Expression: 3
     * Referenced by: '<S37>/4WD'
     */
    3.0,

    /* Expression: 60/2/3.1415
     * Referenced by: '<S37>/Gain4'
     */
    9.5495782269616427,

    /* Expression: 0
     * Referenced by: '<S40>/Constant'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S40>/Gain'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S40>/Gain1'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S43>/Gain'
     */
    1.0,

    /* Expression: int8(lic_path)
     * Referenced by: '<S1>/Chassis'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S1>/Chassis'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S41>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S41>/S-Function'
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
     * Referenced by: '<S13>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S13>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S28>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S28>/S-Function'
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
     * Referenced by: '<S27>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S27>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110},

    /* Expression: int8(lic_path)
     * Referenced by: '<S30>/S-Function'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S30>/S-Function'
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
     * Referenced by: '<S46>/Steer_Mapped'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 108, 105, 99},

    /* Expression: int8(par_path)
     * Referenced by: '<S46>/Steer_Mapped'
     */
    {46, 46, 47, 46, 46, 47, 112, 97, 114, 97, 109, 47, 116, 120, 99, 97, 114, 46, 106, 115, 111, 110}};
