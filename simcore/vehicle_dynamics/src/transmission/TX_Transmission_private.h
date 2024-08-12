/*
 * TX_Transmission_private.h
 *
 * Code generation for model "TX_Transmission".
 *
 * Model version              : 1.210
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Dec  6 16:43:43 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_TX_Transmission_private_h_
#define RTW_HEADER_TX_Transmission_private_h_
#include "TX_Transmission.h"
#include "TX_Transmission_types.h"
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
extern real_T look2_binlcapw(real_T u0, real_T u1, const real_T bp0[], const real_T bp1[], const real_T table[],
                             const uint32_T maxIndex[], uint32_T stride);
extern uint32_T plook_u32d_binckan(real_T u, const real_T bp[], uint32_T maxIndex);
extern uint32_T binsearch_u32d(real_T u, const real_T bp[], uint32_T startIndex, uint32_T maxIndex);

/* private model entry point functions */
extern void TX_Transmission_derivatives();

#endif /* RTW_HEADER_TX_Transmission_private_h_ */
