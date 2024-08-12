/*
 * EMotor_data.cpp
 *
 * Code generation for model "EMotor".
 *
 * Model version              : 1.31
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Thu Sep 15 20:22:54 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "EMotor.h"

/* Block parameters (default storage) */
P_EMotor_T EMotor::EMotor_P{
    /* Variable: mot_tc_s
     * Referenced by: '<S6>/Gain1'
     */
    0.01,

    /* Expression: 1/60*2*3.1415926
     * Referenced by: '<Root>/GainPRM'
     */
    0.10471975333333333,

    /* Expression: T_t_extended
     * Referenced by: '<S6>/Max Torque Limit'
     */
    {0.0, 0.07, 0.08, 0.09, 0.08, 0.07, 0.0},

    /* Expression: w_t_extended
     * Referenced by: '<S6>/Max Torque Limit'
     */
    {-800.0, -750.0, -375.0, 0.0, 375.0, 750.0, 800.0},

    /* Expression: 0
     * Referenced by: '<S6>/Integrator'
     */
    0.0,

    /* Expression: [-1 1]
     * Referenced by: '<S6>/Interpolated zero-crossing'
     */
    {-1.0, 1.0},

    /* Expression: [-1 1]
     * Referenced by: '<S6>/Interpolated zero-crossing'
     */
    {-1.0, 1.0},

    /* Expression: x_losses_mat
     * Referenced by: '<S5>/2-D Lookup Table'
     */
    {3.1,  2.03, 1.68, 2.03, 3.1,  2.21, 1.14, 0.78, 1.14, 2.21, 1.67, 0.69, 0.24, 0.69, 1.67, 1.49, 0.42, 0.06,
     0.42, 1.49, 1.67, 0.69, 0.24, 0.69, 1.67, 2.21, 1.14, 0.78, 1.14, 2.21, 3.1,  2.03, 1.68, 2.03, 3.1},

    /* Expression: x_w_eff_vec
     * Referenced by: '<S5>/2-D Lookup Table'
     */
    {-8000.0, -4000.0, 0.0, 4000.0, 8000.0},

    /* Expression: x_T_eff_vec
     * Referenced by: '<S5>/2-D Lookup Table'
     */
    {-0.09, -0.06, -0.03, 0.0, 0.03, 0.06, 0.09},

    /* Expression: Inf
     * Referenced by: '<S3>/Saturation'
     */
    0.0,

    /* Expression: 0.0001
     * Referenced by: '<S3>/Saturation'
     */
    0.0001,

    /* Computed Parameter: uDLookupTable_maxIndex
     * Referenced by: '<S5>/2-D Lookup Table'
     */
    {4U, 6U}};
