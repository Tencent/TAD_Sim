/*
 * BrakeHydraulic_data.cpp
 *
 * Code generation for model "BrakeHydraulic".
 *
 * Model version              : 2.37
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Thu May 11 21:00:45 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "BrakeHydraulic.h"

/* Block parameters (default storage) */
P_BrakeHydraulic_T BrakeHydraulic::BrakeHydraulic_P{/* Variable: brake_const_time_fluid
                                                     * Referenced by: '<S3>/Constant'
                                                     */
                                                    0.06,

                                                    /* Variable: brake_fluid_transport_const_time
                                                     * Referenced by: '<S1>/Constant3'
                                                     */
                                                    0.01,

                                                    /* Variable: max_main_cylinder_pressure
                                                     * Referenced by:
                                                     *   '<S1>/Constant1'
                                                     *   '<S1>/Saturation'
                                                     */
                                                    1.5E+7,

                                                    /* Variable: rear_brake_ratio
                                                     * Referenced by: '<S1>/Rear Bias'
                                                     */
                                                    0.43,

                                                    /* Expression: 0
                                                     * Referenced by: '<S3>/Integrator1'
                                                     */
                                                    0.0,

                                                    /* Expression: 0
                                                     * Referenced by: '<Root>/Memory'
                                                     */
                                                    0.0,

                                                    /* Expression: 1
                                                     * Referenced by: '<S1>/Front Bias'
                                                     */
                                                    1.0,

                                                    /* Expression: 1000
                                                     * Referenced by: '<S1>/Gain'
                                                     */
                                                    1000.0,

                                                    /* Expression: 0.0
                                                     * Referenced by: '<S1>/fluidTransportDelay'
                                                     */
                                                    0.0,

                                                    /* Expression: 0
                                                     * Referenced by: '<S1>/Saturation'
                                                     */
                                                    0.0};
