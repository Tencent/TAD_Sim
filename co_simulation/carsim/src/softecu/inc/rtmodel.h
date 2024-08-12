/*
 *  rtmodel.h:
 *
 * Code generation for model "softecu".
 *
 * Model version              : 1.169
 * Simulink Coder version : 9.1 (R2019a) 23-Nov-2018
 * C++ source code generated on : Thu May 21 14:36:09 2020
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Linux 64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_rtmodel_h_
#define RTW_HEADER_rtmodel_h_

/*
 *  Includes the appropriate headers when we are using rtModel
 */
#include "softecu.h"
#define GRTINTERFACE 0

/*
 * ROOT_IO_FORMAT: 0 (Individual arguments)
 * ROOT_IO_FORMAT: 1 (Structure reference)
 * ROOT_IO_FORMAT: 2 (Part of model data structure)
 */
#define ROOT_IO_FORMAT 2

/* Macros generated for backwards compatibility  */
#ifndef rtmGetStopRequested
#  define rtmGetStopRequested(rtm) ((void*)0)
#endif

#define MODEL_CLASSNAME softecuModelClass
#define MODEL_STEPNAME step
#endif /* RTW_HEADER_rtmodel_h_ */
