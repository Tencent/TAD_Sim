#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "driveline_ice_sfunction_cgen_wrapper.h"

extern void driveline_ice_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                  const int_T p_width1);
extern void driveline_ice_sfunction_Outputs_wrapper(const real_T *DrvShfTrq, const real_T *OmegaAxle,
                                                    const real_T *DiffPrsCmd, real_T *AxlTrq, real_T *PropShaftSpd,
                                                    const int8_T *lic_path, const int_T p_width0,
                                                    const int8_T *par_path, const int_T p_width1);
extern void driveline_ice_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                      const int8_T *par_path, const int_T p_width1);
void driveline_ice_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                const int_T p_width1) {
  driveline_ice_sfunction_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void driveline_ice_sfunction_Outputs_wrapper_cgen(const real_T *DrvShfTrq, const real_T *OmegaAxle,
                                                  const real_T *DiffPrsCmd, real_T *AxlTrq, real_T *PropShaftSpd,
                                                  const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                  const int_T p_width1) {
  driveline_ice_sfunction_Outputs_wrapper(DrvShfTrq, OmegaAxle, DiffPrsCmd, AxlTrq, PropShaftSpd, lic_path, p_width0,
                                          par_path, p_width1);
}

void driveline_ice_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0,
                                                    const int8_T *par_path, const int_T p_width1) {
  driveline_ice_sfunction_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
