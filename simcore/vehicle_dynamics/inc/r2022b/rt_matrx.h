
#ifndef rt_matrx_h
#define rt_matrx_h

#if defined(_MSC_VER)
#  pragma once
#endif
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3))
#  pragma once
#endif

/*
 * Copyright 1994-2022 The MathWorks, Inc.
 */

/*==========*
 * Includes *
 *==========*/

#if defined(MATLAB_MEX_FILE)
#  error "rt_matrix cannot be used within a mex file. It is for codegen only."
#endif

#include <stdlib.h> /* needed for malloc, calloc, free, realloc */

#include "rtwtypes.h"

#include "rt_mxclassid.h" /* mxClassID */

/*==========*
 * Typedefs *
 *==========*/

#ifndef rt_typedefs_h
#  define rt_typedefs_h

#  if !defined(TYPEDEF_MX_ARRAY)
#    define TYPEDEF_MX_ARRAY
typedef real_T mxArray;
#  endif

typedef real_T mxChar;

#  if !defined(TMW_NAME_LENGTH_MAX)
#    define TMW_NAME_LENGTH_MAX 64
#  endif
#  define mxMAXNAM TMW_NAME_LENGTH_MAX /* maximum name length */

typedef enum { mxREAL, mxCOMPLEX } mxComplexity;

#endif /* rt_typedefs_h */

/*==================*
 * Extern variables *
 *==================*/

#ifdef __cplusplus
extern "C" {
#endif

extern real_T rtInf;
extern real_T rtMinusInf;
extern real_T rtNaN;

/*=======================================*
 * Defines for mx Routines and constants *
 *=======================================*/

#define mxCalloc(n, size) calloc(n, size)

#define mxCreateCharArray(ndim, dims) mxCreateNumericArray(ndim, dims, mxCHAR_CLASS);

#define mxDestroyArray(pa) \
  if (pa) free(pa)

/* NOTE: You cannot mxFree(mxGetPr(pa)) !!! */
#define mxFree(ptr) \
  if (ptr) free(ptr)

#define mxGetClassID(pa) mxDOUBLE_CLASS

/* NOTE: mxGetClassName(pa) returns "double" even on a character array */
#define mxGetClassName(pa) "double"

#define mxGetData(pa) ((void *)(&((pa)[2])))

#define mxGetElementSize(pa) (sizeof(real_T))

#define mxGetInf() rtInf

#define mxGetM(pa) ((size_t)((pa)[0]))
#define mxGetN(pa) ((size_t)((pa)[1]))

#define mxGetNaN() rtNaN

#define mxGetNumberOfDimensions(pa) (2)
#define mxGetNumberOfElements(pa) (mxGetM(pa) * mxGetN(pa))

/* NOTE: mxGetPr() of an empty matrix does NOT return NULL */
#define mxGetPr(pa) (&((pa)[2]))

/* NOTE: mxGetDoubles() of an empty matrix does NOT return NULL */
#define mxGetDoubles(pa) (&((pa)[2]))

#define mxGetScalar(pa) ((pa)[2])

#define mxIsComplex(pa) false

#define mxIsDouble(pa) true

#define mxIsEmpty(pa) (mxGetM(pa) == 0 || mxGetN(pa) == 0)

#define mxIsFinite(r) ((r) > rtMinusInf && (r) < rtInf)

#define mxIsInf(r) ((r) == rtInf || (r) == rtMinusInf)

#define mxIsInt16(pa) false

#define mxIsInt32(pa) false

#define mxIsInt8(pa) false

#define mxIsLogical(pa) false

#define mxIsNumeric(pa) true

#define mxIsSingle(pa) false

#define mxIsSparse(pa) false

#define mxIsStruct(pa) false

#define mxIsUint16(pa) false

#define mxIsUint32(pa) false

#define mxIsUint8(pa) false

#define mxMalloc(n) malloc(n)

#define mxRealloc(p, n) realloc(p, n)

/*==========================*
 * Visible/extern functions *
 *=========================*/

extern mxArray *rt_mxCreateCharMatrixFromStrings(int_T m, const char_T **str);

extern mxArray *rt_mxCreateString(const char *str);

extern mxArray *rt_mxCreateDoubleMatrix(int m, int n, mxComplexity flag);

extern mxArray *rt_mxCreateNumericArray(int_T ndims, const mwSize *dims, mxClassID classid, mxComplexity flag);

extern mxArray *rt_mxDuplicateArray(const mxArray *pa);

extern const mwSize *rt_mxGetDimensions(const mxArray *pa);

extern real_T rt_mxGetEps(void);

extern int_T rt_mxGetString(const mxArray *pa, char_T *buf, int_T buflen);

#ifdef __cplusplus
}
#endif

#define mxCreateCharMatrixFromStrings(m, str) rt_mxCreateCharMatrixFromStrings(m, str)

#define mxCreateString(str) rt_mxCreateString(str)

#define mxCreateDoubleMatrix(m, n, flag) rt_mxCreateDoubleMatrix(m, n, flag)

#define mxCreateNumericArray(ndims, dims, classid, flag) rt_mxCreateNumericArray(ndims, dims, classid, flag)

#define mxDuplicateArray(pa) rt_mxDuplicateArray(pa)

#define mxGetDimensions(pa) rt_mxGetDimensions(pa)

#define mxGetEps() rt_mxGetEps()

#define mxGetString(pa, buf, buflen) rt_mxGetString(pa, buf, buflen)

/*=========================*
 * Unsupported mx Routines *
 *=========================*/

#define mxCalcSingleSubscript(pa, nsubs, subs) mxCalcSingleSubscript_is_not_supported_in_Simulink_Coder

#define mxCreateCellArray(ndim, dims) mxCreateCellArray_is_not_supported_in_Simulink_Coder

#define mxCreateCellMatrix(m, n) mxCreateCellMatrix_is_not_supported_in_Simulink_Coder

#define mxCreateSparse(pm, pn, pnzmax, pcmplx_flg) mxCreateSparse_is_not_supported_in_Simulink_Coder

#define mxCreateStructArray(ndim, dims, nfields, fieldnames) mxCreateStructArray_is_not_supported_in_Simulink_Coder

#define mxCreateStructMatrix(m, n, nfields, fieldnames) mxCreateStructMatrix_is_not_supported_in_Simulink_Coder

#define mxGetCell(pa, i) mxGetCell_is_not_supported_in_Simulink_Coder

#define mxGetField(pa, i, fieldname) mxGetField_is_not_supported_in_Simulink_Coder

#define mxGetFieldByNumber(s, i, fieldnum) mxGetFieldByNumber_is_not_supported_in_Simulink_Coder

#define mxGetFieldNameByNumber(pa, n) mxGetFieldNameByNumber_is_not_supported_in_Simulink_Coder

#define mxGetFieldNumber(pa, fieldname) mxGetFieldNumber_is_not_supported_in_Simulink_Coder

#define mxGetImagData(pa) mxGetImagData_is_not_supported_in_Simulink_Coder

#define mxGetIr(ppa) mxGetIr_is_not_supported_in_Simulink_Coder

#define mxGetJc(ppa) mxGetJc_is_not_supported_in_Simulink_Coder

#define mxGetNumberOfFields(pa) mxGetNumberOfFields_is_not_supported_in_Simulink_Coder

#define mxGetNzmax(pa) mxGetNzmax_is_not_supported_in_Simulink_Coder

#define mxGetPi(pa) mxGetPi_is_not_supported_in_Simulink_Coder

#define mxIsFromGlobalWS(pa) mxIsFromGlobalWS_is_not_supported_in_Simulink_Coder

#define mxIsNaN(r) mxIsNaN_is_not_supported_in_Simulink_Coder

#define mxIsChar(pa) mxIsChar_is_not_supported_in_Simulink_Coder

#define mxIsClass(pa, class) mxIsClass_is_not_supported_in_Simulink_Coder

#define mxIsCell(pa) mxIsCell_is_not_supported_in_Simulink_Coder

#define mxSetCell(pa, i, value) mxSetCell_is_not_supported_in_Simulink_Coder

#define mxSetClassName(pa, classname) mxSetClassName_is_not_supported_in_Simulink_Coder

#define mxSetData(pa, pr) mxSetData_is_not_supported_in_Simulink_Coder

#define mxSetDimensions(pa, size, ndims) mxSetDimensions_is_not_supported_in_Simulink_Coder

#define mxSetField(pa, i, fieldname, value) mxSetField_is_not_supported_in_Simulink_Coder

#define mxSetFieldByNumber(pa, index, fieldnum, value) mxSetFieldByNumber_is_not_supported_in_Simulink_Coder

#define mxSetFromGlobalWS(pa, global) mxSetFromGlobalWS_is_not_supported_in_Simulink_Coder

#define mxSetImagData(pa, pv) mxSetImagData_is_not_supported_in_Simulink_Coder

#define mxSetIr(ppa, ir) mxSetIr_is_not_supported_in_Simulink_Coder

#define mxSetJc(ppa, jc) mxSetJc_is_not_supported_in_Simulink_Coder

#define mxSetM(pa, m) mxSetM_is_not_supported_in_Simulink_Coder

#define mxSetN(pa, m) mxSetN_is_not_supported_in_Simulink_Coder

#define mxSetPr(pa, pr) mxSetPr_is_not_supported_in_Simulink_Coder

#define mxSetNzmax(pa, nzmax) mxSetNzmax_is_not_supported_in_Simulink_Coder

#define mxSetPi(pa, pv) mxSetPi_is_not_supported_in_Simulink_Coder

/*==========================*
 * Unsupported mex routines *
 *==========================*/

#define mexPrintAssertion(test, fname, linenum, message) mexPrintAssertion_is_not_supported_by_Simulink_Coder

#define mexEvalString(str) mexEvalString_is_not_supported_by_Simulink_Coder

#define mexErrMsgTxt(str) mexErrMsgTxt_is_not_supported_by_Simulink_Coder

#define mexWarnMsgTxt(warning_msg) mexWarnMsgTxt_is_not_supported_by_Simulink_Coder

#define mexPrintf mexPrintf_is_not_supported_by_Simulink_Coder

#define mexMakeArrayPersistent(pa) mexMakeArrayPersistent_is_not_supported_by_Simulink_Coder

#define mexMakeMemoryPersistent(ptr) mexMakeMemoryPersistent_is_not_supported_by_Simulink_Coder

#define mexLock() mexLock_is_not_supported_by_Simulink_Coder

#define mexUnlock() mexUnlock_is_not_supported_by_Simulink_Coder

#define mexFunctionName() mexFunctionName_is_not_supported_by_Simulink_Coder

#define mexIsLocked() mexIsLocked_is_not_supported_by_Simulink_Coder

#define mexGetFunctionHandle() mexGetFunctionHandle_is_not_supported_by_Simulink_Coder

#define mexCallMATLABFunction() mexCallMATLABFunction_is_not_supported_by_Simulink_Coder

#define mexRegisterFunction() mexRegisterFunction_is_not_supported_by_Simulink_Coder

#define mexSet(handle, property, value) mexSet_is_not_supported_by_Simulink_Coder

#define mexGet(handle, property) mexGet_is_not_supported_by_Simulink_Coder

#define mexCallMATLAB(nlhs, plhs, nrhs, prhs, fcn) mexCallMATLAB_is_not_supported_by_Simulink_Coder

#define mexSetTrapFlag(flag) mexSetTrapFlag_is_not_supported_by_Simulink_Coder

#define mexUnlink(a) mexUnlink_is_not_supported_by_Simulink_Coderw

#define mexSubsAssign(plhs, sub, nsubs, prhs) mexSubsAssign_is_not_supported_by_Simulink_Coder

#define mexSubsReference(prhs, subs, nsubs) mexSubsReference_is_not_supported_by_Simulink_Coder

#define mexPrintAssertion(test, fname, linenum, message) mexPrintAssertion_is_not_supported_by_Simulink_Coder

#define mexAddFlops(count) mexAddFlops_is_not_supported_by_Simulink_Coder

#define mexIsGlobal(pa) mexIsGlobal_is_not_supported_by_Simulink_Coder

#define mexAtExit(fcn) mexAtExit_is_not_supported_by_Simulink_Coder

#endif /* rt_matrx_h */
