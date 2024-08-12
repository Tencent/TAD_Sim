#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "ecu_sfunction_cgen_wrapper.h"

extern void ecu_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                        const int_T p_width1);
extern void ecu_sfunction_Outputs_wrapper(const powecu_in *ecu_in, powecu_out *ecu_out, const int8_T *lic_path,
                                          const int_T p_width0, const int8_T *par_path, const int_T p_width1);
extern void ecu_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                            const int_T p_width1);
void ecu_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                      const int_T p_width1) {
  ecu_sfunction_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void ecu_sfunction_Outputs_wrapper_cgen(const powecu_in *ecu_in, powecu_out *ecu_out, const int8_T *lic_path,
                                        const int_T p_width0, const int8_T *par_path, const int_T p_width1) {
  ecu_sfunction_Outputs_wrapper((powecu_in *)ecu_in, (powecu_out *)ecu_out, lic_path, p_width0, par_path, p_width1);
}

void ecu_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                          const int_T p_width1) {
  ecu_sfunction_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
