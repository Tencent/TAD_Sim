/*
 * Engine_types.h
 *
 * Code generation for model "Engine".
 *
 * Model version              : 2.21
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Dec  6 13:01:57 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_Engine_types_h_
#define RTW_HEADER_Engine_types_h_
#include "rtwtypes.h"
#ifndef DEFINED_TYPEDEF_FOR_ENGINE_INPUTBUS_
#  define DEFINED_TYPEDEF_FOR_ENGINE_INPUTBUS_

struct ENGINE_INPUTBUS {
  real_T throttle_01;
  real_T eng_spd_rpm;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_ENGINE_OUTPUTBUS_
#  define DEFINED_TYPEDEF_FOR_ENGINE_OUTPUTBUS_

struct ENGINE_OUTPUTBUS {
  real_T eng_trq_Nm;
  real_T pow_out_W;
  real_T fuel_rate_g_s;
  real_T fuel_cosume_L;
};

#endif

/* Parameters (default storage) */
typedef struct P_Engine_T_ P_Engine_T;

/* Forward declaration for rtModel */
typedef struct tag_RTM_Engine_T RT_MODEL_Engine_T;

#endif /* RTW_HEADER_Engine_types_h_ */
