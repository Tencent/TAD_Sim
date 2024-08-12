/*
 * multiword_types.h
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

#ifndef MULTIWORD_TYPES_H
#define MULTIWORD_TYPES_H
#include "rtwtypes.h"

/*
 * MultiWord supporting definitions
 */
typedef long long longlong_T;

/*
 * MultiWord types
 */
typedef struct {
  uint64_T chunks[2];
} int128m_T;

typedef struct {
  int128m_T re;
  int128m_T im;
} cint128m_T;

typedef struct {
  uint64_T chunks[2];
} uint128m_T;

typedef struct {
  uint128m_T re;
  uint128m_T im;
} cuint128m_T;

typedef struct {
  uint64_T chunks[3];
} int192m_T;

typedef struct {
  int192m_T re;
  int192m_T im;
} cint192m_T;

typedef struct {
  uint64_T chunks[3];
} uint192m_T;

typedef struct {
  uint192m_T re;
  uint192m_T im;
} cuint192m_T;

typedef struct {
  uint64_T chunks[4];
} int256m_T;

typedef struct {
  int256m_T re;
  int256m_T im;
} cint256m_T;

typedef struct {
  uint64_T chunks[4];
} uint256m_T;

typedef struct {
  uint256m_T re;
  uint256m_T im;
} cuint256m_T;

typedef struct {
  uint64_T chunks[5];
} int320m_T;

typedef struct {
  int320m_T re;
  int320m_T im;
} cint320m_T;

typedef struct {
  uint64_T chunks[5];
} uint320m_T;

typedef struct {
  uint320m_T re;
  uint320m_T im;
} cuint320m_T;

typedef struct {
  uint64_T chunks[6];
} int384m_T;

typedef struct {
  int384m_T re;
  int384m_T im;
} cint384m_T;

typedef struct {
  uint64_T chunks[6];
} uint384m_T;

typedef struct {
  uint384m_T re;
  uint384m_T im;
} cuint384m_T;

typedef struct {
  uint64_T chunks[7];
} int448m_T;

typedef struct {
  int448m_T re;
  int448m_T im;
} cint448m_T;

typedef struct {
  uint64_T chunks[7];
} uint448m_T;

typedef struct {
  uint448m_T re;
  uint448m_T im;
} cuint448m_T;

typedef struct {
  uint64_T chunks[8];
} int512m_T;

typedef struct {
  int512m_T re;
  int512m_T im;
} cint512m_T;

typedef struct {
  uint64_T chunks[8];
} uint512m_T;

typedef struct {
  uint512m_T re;
  uint512m_T im;
} cuint512m_T;

typedef struct {
  uint64_T chunks[9];
} int576m_T;

typedef struct {
  int576m_T re;
  int576m_T im;
} cint576m_T;

typedef struct {
  uint64_T chunks[9];
} uint576m_T;

typedef struct {
  uint576m_T re;
  uint576m_T im;
} cuint576m_T;

typedef struct {
  uint64_T chunks[10];
} int640m_T;

typedef struct {
  int640m_T re;
  int640m_T im;
} cint640m_T;

typedef struct {
  uint64_T chunks[10];
} uint640m_T;

typedef struct {
  uint640m_T re;
  uint640m_T im;
} cuint640m_T;

typedef struct {
  uint64_T chunks[11];
} int704m_T;

typedef struct {
  int704m_T re;
  int704m_T im;
} cint704m_T;

typedef struct {
  uint64_T chunks[11];
} uint704m_T;

typedef struct {
  uint704m_T re;
  uint704m_T im;
} cuint704m_T;

typedef struct {
  uint64_T chunks[12];
} int768m_T;

typedef struct {
  int768m_T re;
  int768m_T im;
} cint768m_T;

typedef struct {
  uint64_T chunks[12];
} uint768m_T;

typedef struct {
  uint768m_T re;
  uint768m_T im;
} cuint768m_T;

typedef struct {
  uint64_T chunks[13];
} int832m_T;

typedef struct {
  int832m_T re;
  int832m_T im;
} cint832m_T;

typedef struct {
  uint64_T chunks[13];
} uint832m_T;

typedef struct {
  uint832m_T re;
  uint832m_T im;
} cuint832m_T;

typedef struct {
  uint64_T chunks[14];
} int896m_T;

typedef struct {
  int896m_T re;
  int896m_T im;
} cint896m_T;

typedef struct {
  uint64_T chunks[14];
} uint896m_T;

typedef struct {
  uint896m_T re;
  uint896m_T im;
} cuint896m_T;

typedef struct {
  uint64_T chunks[15];
} int960m_T;

typedef struct {
  int960m_T re;
  int960m_T im;
} cint960m_T;

typedef struct {
  uint64_T chunks[15];
} uint960m_T;

typedef struct {
  uint960m_T re;
  uint960m_T im;
} cuint960m_T;

typedef struct {
  uint64_T chunks[16];
} int1024m_T;

typedef struct {
  int1024m_T re;
  int1024m_T im;
} cint1024m_T;

typedef struct {
  uint64_T chunks[16];
} uint1024m_T;

typedef struct {
  uint1024m_T re;
  uint1024m_T im;
} cuint1024m_T;

typedef struct {
  uint64_T chunks[17];
} int1088m_T;

typedef struct {
  int1088m_T re;
  int1088m_T im;
} cint1088m_T;

typedef struct {
  uint64_T chunks[17];
} uint1088m_T;

typedef struct {
  uint1088m_T re;
  uint1088m_T im;
} cuint1088m_T;

typedef struct {
  uint64_T chunks[18];
} int1152m_T;

typedef struct {
  int1152m_T re;
  int1152m_T im;
} cint1152m_T;

typedef struct {
  uint64_T chunks[18];
} uint1152m_T;

typedef struct {
  uint1152m_T re;
  uint1152m_T im;
} cuint1152m_T;

typedef struct {
  uint64_T chunks[19];
} int1216m_T;

typedef struct {
  int1216m_T re;
  int1216m_T im;
} cint1216m_T;

typedef struct {
  uint64_T chunks[19];
} uint1216m_T;

typedef struct {
  uint1216m_T re;
  uint1216m_T im;
} cuint1216m_T;

typedef struct {
  uint64_T chunks[20];
} int1280m_T;

typedef struct {
  int1280m_T re;
  int1280m_T im;
} cint1280m_T;

typedef struct {
  uint64_T chunks[20];
} uint1280m_T;

typedef struct {
  uint1280m_T re;
  uint1280m_T im;
} cuint1280m_T;

typedef struct {
  uint64_T chunks[21];
} int1344m_T;

typedef struct {
  int1344m_T re;
  int1344m_T im;
} cint1344m_T;

typedef struct {
  uint64_T chunks[21];
} uint1344m_T;

typedef struct {
  uint1344m_T re;
  uint1344m_T im;
} cuint1344m_T;

typedef struct {
  uint64_T chunks[22];
} int1408m_T;

typedef struct {
  int1408m_T re;
  int1408m_T im;
} cint1408m_T;

typedef struct {
  uint64_T chunks[22];
} uint1408m_T;

typedef struct {
  uint1408m_T re;
  uint1408m_T im;
} cuint1408m_T;

typedef struct {
  uint64_T chunks[23];
} int1472m_T;

typedef struct {
  int1472m_T re;
  int1472m_T im;
} cint1472m_T;

typedef struct {
  uint64_T chunks[23];
} uint1472m_T;

typedef struct {
  uint1472m_T re;
  uint1472m_T im;
} cuint1472m_T;

typedef struct {
  uint64_T chunks[24];
} int1536m_T;

typedef struct {
  int1536m_T re;
  int1536m_T im;
} cint1536m_T;

typedef struct {
  uint64_T chunks[24];
} uint1536m_T;

typedef struct {
  uint1536m_T re;
  uint1536m_T im;
} cuint1536m_T;

typedef struct {
  uint64_T chunks[25];
} int1600m_T;

typedef struct {
  int1600m_T re;
  int1600m_T im;
} cint1600m_T;

typedef struct {
  uint64_T chunks[25];
} uint1600m_T;

typedef struct {
  uint1600m_T re;
  uint1600m_T im;
} cuint1600m_T;

typedef struct {
  uint64_T chunks[26];
} int1664m_T;

typedef struct {
  int1664m_T re;
  int1664m_T im;
} cint1664m_T;

typedef struct {
  uint64_T chunks[26];
} uint1664m_T;

typedef struct {
  uint1664m_T re;
  uint1664m_T im;
} cuint1664m_T;

typedef struct {
  uint64_T chunks[27];
} int1728m_T;

typedef struct {
  int1728m_T re;
  int1728m_T im;
} cint1728m_T;

typedef struct {
  uint64_T chunks[27];
} uint1728m_T;

typedef struct {
  uint1728m_T re;
  uint1728m_T im;
} cuint1728m_T;

typedef struct {
  uint64_T chunks[28];
} int1792m_T;

typedef struct {
  int1792m_T re;
  int1792m_T im;
} cint1792m_T;

typedef struct {
  uint64_T chunks[28];
} uint1792m_T;

typedef struct {
  uint1792m_T re;
  uint1792m_T im;
} cuint1792m_T;

typedef struct {
  uint64_T chunks[29];
} int1856m_T;

typedef struct {
  int1856m_T re;
  int1856m_T im;
} cint1856m_T;

typedef struct {
  uint64_T chunks[29];
} uint1856m_T;

typedef struct {
  uint1856m_T re;
  uint1856m_T im;
} cuint1856m_T;

typedef struct {
  uint64_T chunks[30];
} int1920m_T;

typedef struct {
  int1920m_T re;
  int1920m_T im;
} cint1920m_T;

typedef struct {
  uint64_T chunks[30];
} uint1920m_T;

typedef struct {
  uint1920m_T re;
  uint1920m_T im;
} cuint1920m_T;

typedef struct {
  uint64_T chunks[31];
} int1984m_T;

typedef struct {
  int1984m_T re;
  int1984m_T im;
} cint1984m_T;

typedef struct {
  uint64_T chunks[31];
} uint1984m_T;

typedef struct {
  uint1984m_T re;
  uint1984m_T im;
} cuint1984m_T;

typedef struct {
  uint64_T chunks[32];
} int2048m_T;

typedef struct {
  int2048m_T re;
  int2048m_T im;
} cint2048m_T;

typedef struct {
  uint64_T chunks[32];
} uint2048m_T;

typedef struct {
  uint2048m_T re;
  uint2048m_T im;
} cuint2048m_T;

#endif /* MULTIWORD_TYPES_H */
