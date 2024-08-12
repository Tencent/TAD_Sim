/*
 * builtin_typeid_types.h
 *
 * Prerelease License - for engineering feedback and testing purposes
 * only. Not for sale.
 *
 * Code generation for model "parsePBCHPayload".
 *
 * Model version              : 3.11
 * Simulink Coder version : 9.6 (R2022a) 11-Jun-2021
 * C source code generated on : Thu Jul  8 14:58:08 2021
 *
 * Target selection: systemverilog_dpi_grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: ASIC/FPGA->ASIC/FPGA
 * Emulation hardware selection:
 *    Differs from embedded hardware (Generic->Custom)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef BUILTIN_TYPEID_TYPES_H
#define BUILTIN_TYPEID_TYPES_H
#include "rtwtypes.h"
#ifndef BUILTIN_TYPEID_TYPES
#  define BUILTIN_TYPEID_TYPES

/* Enumeration of built-in data types */
typedef enum {
  SS_DOUBLE = 0, /* real_T    */
  SS_SINGLE = 1, /* real32_T  */
  SS_INT8 = 2,   /* int8_T    */
  SS_UINT8 = 3,  /* uint8_T   */
  SS_INT16 = 4,  /* int16_T   */
  SS_UINT16 = 5, /* uint16_T  */
  SS_INT32 = 6,  /* int32_T   */
  SS_UINT32 = 7, /* uint32_T  */
  SS_BOOLEAN = 8 /* boolean_T */
} BuiltInDTypeId;

#  define SS_NUM_BUILT_IN_DTYPE ((int_T)SS_BOOLEAN + 1)

/* Enumeration for MAT-file logging code */
typedef int_T DTypeId;

#endif /* BUILTIN_TYPEID_TYPES */
#endif /* BUILTIN_TYPEID_TYPES_H */
