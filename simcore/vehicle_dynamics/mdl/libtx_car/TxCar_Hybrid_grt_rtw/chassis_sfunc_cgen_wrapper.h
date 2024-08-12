#ifndef _CHASSIS_SFUNC_CGEN_WRAPPER_H_
#define _CHASSIS_SFUNC_CGEN_WRAPPER_H_
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "chassis_sfunc_bus.h"
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void chassis_sfunc_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                   const int_T p_width1);
SFB_EXTERN_C void chassis_sfunc_Outputs_wrapper_cgen(const Env_Bus_Output *env_in,
                                                     const Chassis_DriveLine_Input *driveline_in,
                                                     Vehicle_Body_Output *body_out, Wheels_Output *wheel_out,
                                                     Susp_Output *susp_out, const int8_T *lic_path,
                                                     const int_T p_width0, const int8_T *par_path,
                                                     const int_T p_width1);
SFB_EXTERN_C void chassis_sfunc_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0,
                                                       const int8_T *par_path, const int_T p_width1);

#undef SFB_EXTERN_C
#endif
