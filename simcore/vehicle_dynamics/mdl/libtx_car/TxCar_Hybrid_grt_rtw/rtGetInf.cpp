/*
 * rtGetInf.cpp
 *
 * Code generation for model "TxCar_Hybrid".
 *
 * Model version              : 2.2139
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Fri Jul 28 13:31:29 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "rtwtypes.h"

extern "C" {

#include "rtGetInf.h"
}

#include <stddef.h>

extern "C" {

#include "rt_nonfinite.h"
}

#define NumBitsPerChar 8U

extern "C" {
/*
 * Initialize rtInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
real_T rtGetInf(void) {
  size_t bitsPerReal{sizeof(real_T) * (NumBitsPerChar)};

  real_T inf{0.0};

  if (bitsPerReal == 32U) {
    inf = rtGetInfF();
  } else {
    uint16_T one{1U};

    enum { LittleEndian, BigEndian } machByteOrder{(*((uint8_T *)&one) == 1U) ? LittleEndian : BigEndian};
    switch (machByteOrder) {
      case LittleEndian: {
        union {
          LittleEndianIEEEDouble bitVal;
          real_T fltVal;
        } tmpVal;

        tmpVal.bitVal.words.wordH = 0x7FF00000U;
        tmpVal.bitVal.words.wordL = 0x00000000U;
        inf = tmpVal.fltVal;
        break;
      }

      case BigEndian: {
        union {
          BigEndianIEEEDouble bitVal;
          real_T fltVal;
        } tmpVal;

        tmpVal.bitVal.words.wordH = 0x7FF00000U;
        tmpVal.bitVal.words.wordL = 0x00000000U;
        inf = tmpVal.fltVal;
        break;
      }
    }
  }

  return inf;
}

/*
 * Initialize rtInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
real32_T rtGetInfF(void) {
  IEEESingle infF;
  infF.wordL.wordLuint = 0x7F800000U;
  return infF.wordL.wordLreal;
}

/*
 * Initialize rtMinusInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
real_T rtGetMinusInf(void) {
  size_t bitsPerReal{sizeof(real_T) * (NumBitsPerChar)};

  real_T minf{0.0};

  if (bitsPerReal == 32U) {
    minf = rtGetMinusInfF();
  } else {
    uint16_T one{1U};

    enum { LittleEndian, BigEndian } machByteOrder{(*((uint8_T *)&one) == 1U) ? LittleEndian : BigEndian};
    switch (machByteOrder) {
      case LittleEndian: {
        union {
          LittleEndianIEEEDouble bitVal;
          real_T fltVal;
        } tmpVal;

        tmpVal.bitVal.words.wordH = 0xFFF00000U;
        tmpVal.bitVal.words.wordL = 0x00000000U;
        minf = tmpVal.fltVal;
        break;
      }

      case BigEndian: {
        union {
          BigEndianIEEEDouble bitVal;
          real_T fltVal;
        } tmpVal;

        tmpVal.bitVal.words.wordH = 0xFFF00000U;
        tmpVal.bitVal.words.wordL = 0x00000000U;
        minf = tmpVal.fltVal;
        break;
      }
    }
  }

  return minf;
}

/*
 * Initialize rtMinusInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
real32_T rtGetMinusInfF(void) {
  IEEESingle minfF;
  minfF.wordL.wordLuint = 0xFF800000U;
  return minfF.wordL.wordLreal;
}
}
