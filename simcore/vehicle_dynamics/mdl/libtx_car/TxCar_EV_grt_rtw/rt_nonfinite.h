/*
 * rt_nonfinite.h
 *
 * Code generation for model "TxCar_EV".
 *
 * Model version              : 2.1534
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul 19 14:25:50 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_rt_nonfinite_h_
#define RTW_HEADER_rt_nonfinite_h_
#include <stddef.h>
#include "rtwtypes.h"
#define NOT_USING_NONFINITE_LITERALS 1
#ifdef __cplusplus

extern "C" {

#endif

extern real_T rtInf;
extern real_T rtMinusInf;
extern real_T rtNaN;
extern real32_T rtInfF;
extern real32_T rtMinusInfF;
extern real32_T rtNaNF;
extern void rt_InitInfAndNaN(size_t realSize);
extern boolean_T rtIsInf(real_T value);
extern boolean_T rtIsInfF(real32_T value);
extern boolean_T rtIsNaN(real_T value);
extern boolean_T rtIsNaNF(real32_T value);
struct BigEndianIEEEDouble {
  struct {
    uint32_T wordH;
    uint32_T wordL;
  } words;
};

struct LittleEndianIEEEDouble {
  struct {
    uint32_T wordL;
    uint32_T wordH;
  } words;
};

struct IEEESingle {
  union {
    real32_T wordLreal;
    uint32_T wordLuint;
  } wordL;
};

#ifdef __cplusplus

} /* extern "C" */

#endif
#endif /* RTW_HEADER_rt_nonfinite_h_ */
