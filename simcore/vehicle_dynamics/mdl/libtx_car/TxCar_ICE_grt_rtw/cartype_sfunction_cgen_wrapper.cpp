#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "cartype_sfunction_cgen_wrapper.h"

extern void cartype_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                            const int_T p_width1);
extern void cartype_sfunction_Outputs_wrapper(const real_T *UnUsedSig, cartype_parsing_out *CarTypeOut,
                                              const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                              const int_T p_width1);
extern void cartype_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                const int_T p_width1);
void cartype_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                          const int_T p_width1) {
  cartype_sfunction_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void cartype_sfunction_Outputs_wrapper_cgen(const real_T *UnUsedSig, cartype_parsing_out *CarTypeOut,
                                            const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                            const int_T p_width1) {
  cartype_sfunction_Outputs_wrapper(UnUsedSig, reinterpret_cast<cartype_parsing_out *>(CarTypeOut), lic_path, p_width0,
                                    par_path, p_width1);
}

void cartype_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                              const int_T p_width1) {
  cartype_sfunction_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
