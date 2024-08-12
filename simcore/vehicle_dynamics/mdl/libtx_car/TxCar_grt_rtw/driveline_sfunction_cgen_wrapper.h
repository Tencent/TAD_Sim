#ifndef _DRIVELINE_SFUNCTION_CGEN_WRAPPER_H_
#define _DRIVELINE_SFUNCTION_CGEN_WRAPPER_H_
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "driveline_sfunction_bus.h"
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void driveline_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0,
                                                         const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void driveline_sfunction_Outputs_wrapper_cgen(const driveline_in *DrivelineIn,
                                                           const powecu_out *softecu_output,
                                                           driveline_out *DrivelineOut, const int8_T *lic_path,
                                                           const int_T p_width0, const int8_T *par_path,
                                                           const int_T p_width1);
SFB_EXTERN_C void driveline_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0,
                                                             const int8_T *par_path, const int_T p_width1);

#undef SFB_EXTERN_C
#endif
