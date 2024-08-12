/*
 * TX_DriveLine_ICE_private.h
 *
 * Code generation for model "TX_DriveLine_ICE".
 *
 * Model version              : 1.200
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Tue Jul 18 21:36:16 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_TX_DriveLine_ICE_private_h_
#define RTW_HEADER_TX_DriveLine_ICE_private_h_
#include "TX_DriveLine_ICE.h"
#include "TX_DriveLine_ICE_types.h"
#include "multiword_types.h"
#include "rtwtypes.h"
#include "zero_crossing_types.h"

/* Private macros used by the generated code to access rtModel */
#ifndef rtmSetFirstInitCond
#  define rtmSetFirstInitCond(rtm, val) ((rtm)->Timing.firstInitCondFlag = (val))
#endif

#ifndef rtmIsFirstInitCond
#  define rtmIsFirstInitCond(rtm) ((rtm)->Timing.firstInitCondFlag)
#endif

#ifndef rtmIsMajorTimeStep
#  define rtmIsMajorTimeStep(rtm) (((rtm)->Timing.simTimeStep) == MAJOR_TIME_STEP)
#endif

#ifndef rtmIsMinorTimeStep
#  define rtmIsMinorTimeStep(rtm) (((rtm)->Timing.simTimeStep) == MINOR_TIME_STEP)
#endif

#ifndef rtmSetTPtr
#  define rtmSetTPtr(rtm, val) ((rtm)->Timing.t = (val))
#endif

extern real_T look1_binlxpw(real_T u0, const real_T bp0[], const real_T table[], uint32_T maxIndex);
extern real_T look1_binlcpw(real_T u0, const real_T bp0[], const real_T table[], uint32_T maxIndex);

/* private model entry point functions */
extern void TX_DriveLine_ICE_derivatives();

#endif /* RTW_HEADER_TX_DriveLine_ICE_private_h_ */
