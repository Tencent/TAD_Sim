/*
 * Chassis_private.h
 *
 * Code generation for model "Chassis".
 *
 * Model version              : 2.33
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Thu Feb 16 17:07:18 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_Chassis_private_h_
#define RTW_HEADER_Chassis_private_h_
#include "Chassis.h"
#include "Chassis_types.h"
#include "multiword_types.h"
#include "rtwtypes.h"
#include "zero_crossing_types.h"

/* Private macros used by the generated code to access rtModel */
#ifndef rtmIsMajorTimeStep
#  define rtmIsMajorTimeStep(rtm) (((rtm)->Timing.simTimeStep) == MAJOR_TIME_STEP)
#endif

#ifndef rtmIsMinorTimeStep
#  define rtmIsMinorTimeStep(rtm) (((rtm)->Timing.simTimeStep) == MINOR_TIME_STEP)
#endif

#ifndef rtmSetTPtr
#  define rtmSetTPtr(rtm, val) ((rtm)->Timing.t = (val))
#endif

extern real_T rt_powd_snf(real_T u0, real_T u1);
extern real_T rt_atan2d_snf(real_T u0, real_T u1);
extern void rt_mrdivide_U1d1x3_U2d3x3_Yd1x3_snf(const real_T u0[3], const real_T u1[9], real_T y[3]);
extern real_T look1_binlcpw(real_T u0, const real_T bp0[], const real_T table[], uint32_T maxIndex);
extern real_T look1_binlxpw(real_T u0, const real_T bp0[], const real_T table[], uint32_T maxIndex);
extern real_T look1_pbinlcapw(real_T u0, const real_T bp0[], const real_T table[], uint32_T prevIndex[],
                              uint32_T maxIndex);
extern real_T look2_pbinlcpw(real_T u0, real_T u1, const real_T bp0[], const real_T bp1[], const real_T table[],
                             uint32_T prevIndex[], const uint32_T maxIndex[], uint32_T stride);

/* private model entry point functions */
extern void Chassis_derivatives();

#endif /* RTW_HEADER_Chassis_private_h_ */
