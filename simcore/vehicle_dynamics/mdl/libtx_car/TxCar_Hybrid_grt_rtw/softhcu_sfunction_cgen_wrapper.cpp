#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "softhcu_sfunction_cgen_wrapper.h"

extern void softhcu_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                            const int_T p_width1);
extern void softhcu_sfunction_Outputs_wrapper(const powecu_in *ecu_in, const real_T *frontMotSpd,
                                              const real_T *P4MotSpd, const real_T *engSpd, powecu_out *ecu_out,
                                              HEV_ControllerOutput *hev_out, const int8_T *lic_path,
                                              const int_T p_width0, const int8_T *par_path, const int_T p_width1);
extern void softhcu_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                const int_T p_width1);
void softhcu_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                          const int_T p_width1) {
  softhcu_sfunction_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void softhcu_sfunction_Outputs_wrapper_cgen(const powecu_in *ecu_in, const real_T *frontMotSpd, const real_T *P4MotSpd,
                                            const real_T *engSpd, powecu_out *ecu_out, HEV_ControllerOutput *hev_out,
                                            const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                            const int_T p_width1) {
  softhcu_sfunction_Outputs_wrapper((powecu_in *)ecu_in, frontMotSpd, P4MotSpd, engSpd, (powecu_out *)ecu_out,
                                    (HEV_ControllerOutput *)hev_out, lic_path, p_width0, par_path, p_width1);
}

void softhcu_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                              const int_T p_width1) {
  softhcu_sfunction_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
