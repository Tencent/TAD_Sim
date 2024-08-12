#ifndef _SOFTHCU_SFUNCTION_CGEN_WRAPPER_H_
#define _SOFTHCU_SFUNCTION_CGEN_WRAPPER_H_
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "softhcu_sfunction_bus.h"
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void softhcu_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0,
                                                       const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void softhcu_sfunction_Outputs_wrapper_cgen(const powecu_in *ecu_in, const real_T *frontMotSpd,
                                                         const real_T *P4MotSpd, const real_T *engSpd,
                                                         powecu_out *ecu_out, HEV_ControllerOutput *hev_out,
                                                         const int8_T *lic_path, const int_T p_width0,
                                                         const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void softhcu_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0,
                                                           const int8_T *par_path, const int_T p_width1);

#undef SFB_EXTERN_C
#endif
