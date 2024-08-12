/*
 * BrakeHydraulic_private.h
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

#ifndef RTW_HEADER_BrakeHydraulic_private_h_
#define RTW_HEADER_BrakeHydraulic_private_h_
#include "BrakeHydraulic_types.h"
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

/* private model entry point functions */
extern void BrakeHydraulic_derivatives();

#endif /* RTW_HEADER_BrakeHydraulic_private_h_ */
