#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "dynamic_steer_sfunc_cgen_wrapper.h"

extern void dynamic_steer_sfunc_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                              const int_T p_width1);
extern void dynamic_steer_sfunc_Outputs_wrapper(const Ctrl_Input *driver_in, const Vehicle_Body_Output *body_in,
                                                const Susp_Output *susp_in, const Wheels_Output *wheel_in,
                                                Steer_Output *steer_out, const int8_T *lic_path, const int_T p_width0,
                                                const int8_T *par_path, const int_T p_width1);
extern void dynamic_steer_sfunc_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                  const int_T p_width1);
void dynamic_steer_sfunc_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                            const int_T p_width1) {
  dynamic_steer_sfunc_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void dynamic_steer_sfunc_Outputs_wrapper_cgen(const Ctrl_Input *driver_in, const Vehicle_Body_Output *body_in,
                                              const Susp_Output *susp_in, const Wheels_Output *wheel_in,
                                              Steer_Output *steer_out, const int8_T *lic_path, const int_T p_width0,
                                              const int8_T *par_path, const int_T p_width1) {
  dynamic_steer_sfunc_Outputs_wrapper((Ctrl_Input *)driver_in, (Vehicle_Body_Output *)body_in, (Susp_Output *)susp_in,
                                      (Wheels_Output *)wheel_in, (Steer_Output *)steer_out, lic_path, p_width0,
                                      par_path, p_width1);
}

void dynamic_steer_sfunc_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                const int_T p_width1) {
  dynamic_steer_sfunc_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
