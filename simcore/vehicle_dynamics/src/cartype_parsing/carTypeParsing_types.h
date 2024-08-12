/*
 * carTypeParsing_types.h
 *
 * Code generation for model "carTypeParsing".
 *
 * Model version              : 1.11
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Fri Jul  8 17:01:31 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_carTypeParsing_types_h_
#define RTW_HEADER_carTypeParsing_types_h_
#include "rtwtypes.h"

/* Model Code Variants */
#ifndef DEFINED_TYPEDEF_FOR_cartype_parsing_out_
#  define DEFINED_TYPEDEF_FOR_cartype_parsing_out_

struct cartype_parsing_out {
  real_T drivetype;
  real_T proplutiontype;
  real_T parsingbck_01;
  real_T parsingbck_02;
  real_T parsingbck_03;
};

#endif

/* Parameters (default storage) */
typedef struct P_carTypeParsing_T_ P_carTypeParsing_T;

/* Forward declaration for rtModel */
typedef struct tag_RTM_carTypeParsing_T RT_MODEL_carTypeParsing_T;

#endif /* RTW_HEADER_carTypeParsing_types_h_ */
