/*
 * Copyright 1994-2009 The MathWorks, Inc.
 *
 * File: rt_sim.h
 *
 * Abstract:
 *	Function prototypes for routines which perform one time step of
 *	a real-time single tasking or multitasking system.
 */

#ifndef __RT_SIM__
#define __RT_SIM__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Functions that do not require the SimStruct as an arg
 */
#define rttiSetTaskTime(tPtr, sti, time) (tPtr)[sti] = (time)

#define rttiSetSampleHitInTask(perTaskSampHits, numSampTimes, task_j, task_i, hit) \
  (perTaskSampHits)[(task_j) + ((task_i) * (numSampTimes))] = (hit)

extern const char *rt_SimInitTimingEngine(int_T rtmNumSampTimes, real_T rtmStepSize, real_T *rtmSampleTimePtr,
                                          real_T *rtmOffsetTimePtr, int_T *rtmSampleHitPtr,
                                          int_T *rtmSampleTimeTaskIDPtr, real_T rtmTStart,
                                          SimTimeStep *rtmSimTimeStepPtr, void **rtmTimingDataPtr);
#ifdef RT_MALLOC
extern void rt_SimDestroyTimingEngine(void *rtmTimingData);
#endif

#if !defined(MULTITASKING)
extern void rt_SimUpdateDiscreteTaskSampleHits(int_T rtmNumSampTimes, void *rtmTimingData, int_T *rtmSampleHitPtr,
                                               real_T *rtmTPtr);
#  ifndef RT_MALLOC
extern time_T rt_SimGetNextSampleHit(void);
#  else
extern time_T rt_SimGetNextSampleHit(void *rtmTimingData, int_T rtmNumSampTimes);
#  endif
#else
#  ifndef USE_RTMODEL
extern time_T rt_SimUpdateDiscreteEvents(int_T rtmNumSampTimes, void *rtmTimingData, int_T *rtmSampleHitPtr,
                                         int_T *rtmPerTaskSampleHits);
#  endif
extern void rt_SimUpdateDiscreteTaskTime(real_T *rtmTPtr, void *rtmTimingData, int tid);
#endif

/*
 * Functions maintained for backwards compatibility
 */
#ifndef USE_RTMODEL
extern const char *rt_InitTimingEngine(SimStruct *S);
#  ifdef RT_MALLOC
extern void rt_DestroyTimingEngine(SimStruct *S);
#  endif

#  if !defined(MULTITASKING)
extern void rt_UpdateDiscreteTaskSampleHits(SimStruct *S);
#    ifndef RT_MALLOC
extern time_T rt_GetNextSampleHit(void);
#    else
extern time_T rt_GetNextSampleHit(SimStruct *S);
#    endif

#  else
extern time_T rt_UpdateDiscreteEvents(SimStruct *S);
extern void rt_UpdateDiscreteTaskTime(SimStruct *S, int tid);
#  endif
#endif /* !(USE_RTMODEL) */

#ifdef __cplusplus
}
#endif

#endif /* __RT_SIM__ */
