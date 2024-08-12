/*
 * EMotor_private.h
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

#ifndef RTW_HEADER_EMotor_private_h_
#define RTW_HEADER_EMotor_private_h_
#include "EMotor.h"
#include "builtin_typeid_types.h"
#include "multiword_types.h"
#include "rtwtypes.h"

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

extern real_T look1_binlcapw(real_T u0, const real_T bp0[], const real_T table[], uint32_T maxIndex);
extern real_T look1_binlcpw(real_T u0, const real_T bp0[], const real_T table[], uint32_T maxIndex);
extern real_T look2_binlcapw(real_T u0, real_T u1, const real_T bp0[], const real_T bp1[], const real_T table[],
                             const uint32_T maxIndex[], uint32_T stride);

/* private model entry point functions */
extern void EMotor_derivatives();

#endif /* RTW_HEADER_EMotor_private_h_ */
