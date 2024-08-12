#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "driveline_ev_rear_sfunction_cgen_wrapper.h"

extern void driveline_ev_rear_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                      const int8_T *par_path, const int_T p_width1);
extern void driveline_ev_rear_sfunction_Outputs_wrapper(const driveline_in *DrivelineIn, driveline_out *DrivelineOut,
                                                        const int8_T *lic_path, const int_T p_width0,
                                                        const int8_T *par_path, const int_T p_width1);
extern void driveline_ev_rear_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                          const int8_T *par_path, const int_T p_width1);
void driveline_ev_rear_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0,
                                                    const int8_T *par_path, const int_T p_width1) {
  driveline_ev_rear_sfunction_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void driveline_ev_rear_sfunction_Outputs_wrapper_cgen(const driveline_in *DrivelineIn, driveline_out *DrivelineOut,
                                                      const int8_T *lic_path, const int_T p_width0,
                                                      const int8_T *par_path, const int_T p_width1) {
  driveline_ev_rear_sfunction_Outputs_wrapper((driveline_in *)DrivelineIn, (driveline_out *)DrivelineOut, lic_path,
                                              p_width0, par_path, p_width1);
}

void driveline_ev_rear_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0,
                                                        const int8_T *par_path, const int_T p_width1) {
  driveline_ev_rear_sfunction_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
