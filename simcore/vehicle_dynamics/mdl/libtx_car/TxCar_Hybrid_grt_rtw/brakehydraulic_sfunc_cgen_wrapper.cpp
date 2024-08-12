#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "brakehydraulic_sfunc_cgen_wrapper.h"

extern void brakehydraulic_sfunc_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                               const int_T p_width1);
extern void brakehydraulic_sfunc_Outputs_wrapper(const real_T *BrkPressCmd01, real_T *BrkPressPa,
                                                 const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                 const int_T p_width1);
extern void brakehydraulic_sfunc_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                   const int_T p_width1);
void brakehydraulic_sfunc_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                             const int_T p_width1) {
  brakehydraulic_sfunc_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void brakehydraulic_sfunc_Outputs_wrapper_cgen(const real_T *BrkPressCmd01, real_T *BrkPressPa, const int8_T *lic_path,
                                               const int_T p_width0, const int8_T *par_path, const int_T p_width1) {
  brakehydraulic_sfunc_Outputs_wrapper(BrkPressCmd01, BrkPressPa, lic_path, p_width0, par_path, p_width1);
}

void brakehydraulic_sfunc_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                 const int_T p_width1) {
  brakehydraulic_sfunc_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
