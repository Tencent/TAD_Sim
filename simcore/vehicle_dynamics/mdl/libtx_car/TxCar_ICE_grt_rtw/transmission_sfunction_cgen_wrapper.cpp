#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "transmission_sfunction_cgen_wrapper.h"

extern void transmission_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                 const int_T p_width1);
extern void transmission_sfunction_Outputs_wrapper(const driveline_in *DrivelineIn, const powecu_out *softecu_output,
                                                   const real_T *PropShftSpd, real_T *TransGear, real_T *EngSpeed,
                                                   real_T *DrvShfTrq, const int8_T *lic_path, const int_T p_width0,
                                                   const int8_T *par_path, const int_T p_width1);
extern void transmission_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                     const int8_T *par_path, const int_T p_width1);
void transmission_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                               const int_T p_width1) {
  transmission_sfunction_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void transmission_sfunction_Outputs_wrapper_cgen(const driveline_in *DrivelineIn, const powecu_out *softecu_output,
                                                 const real_T *PropShftSpd, real_T *TransGear, real_T *EngSpeed,
                                                 real_T *DrvShfTrq, const int8_T *lic_path, const int_T p_width0,
                                                 const int8_T *par_path, const int_T p_width1) {
  transmission_sfunction_Outputs_wrapper((driveline_in *)DrivelineIn, (powecu_out *)softecu_output, PropShftSpd,
                                         TransGear, EngSpeed, DrvShfTrq, lic_path, p_width0, par_path, p_width1);
}

void transmission_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                   const int_T p_width1) {
  transmission_sfunction_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
