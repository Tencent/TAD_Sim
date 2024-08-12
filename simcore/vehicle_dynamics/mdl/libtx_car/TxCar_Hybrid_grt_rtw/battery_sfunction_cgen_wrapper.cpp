#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "battery_sfunction_cgen_wrapper.h"

extern void battery_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                            const int_T p_width1);
extern void battery_sfunction_Outputs_wrapper(const batt_in *BattIn, batt_out *BattOut, const int8_T *lic_path,
                                              const int_T p_width0, const int8_T *par_path, const int_T p_width1);
extern void battery_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                const int_T p_width1);
void battery_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                          const int_T p_width1) {
  battery_sfunction_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void battery_sfunction_Outputs_wrapper_cgen(const batt_in *BattIn, batt_out *BattOut, const int8_T *lic_path,
                                            const int_T p_width0, const int8_T *par_path, const int_T p_width1) {
  battery_sfunction_Outputs_wrapper((batt_in *)BattIn, (batt_out *)BattOut, lic_path, p_width0, par_path, p_width1);
}

void battery_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                              const int_T p_width1) {
  battery_sfunction_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
