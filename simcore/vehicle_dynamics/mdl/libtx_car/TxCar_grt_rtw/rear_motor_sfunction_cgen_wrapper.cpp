#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "rear_motor_sfunction_cgen_wrapper.h"

extern void rear_motor_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                               const int_T p_width1);
extern void rear_motor_sfunction_Outputs_wrapper(const moter_in *MotorIn, motor_out *MotorOut, const int8_T *lic_path,
                                                 const int_T p_width0, const int8_T *par_path, const int_T p_width1);
extern void rear_motor_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                   const int_T p_width1);
void rear_motor_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                             const int_T p_width1) {
  rear_motor_sfunction_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void rear_motor_sfunction_Outputs_wrapper_cgen(const moter_in *MotorIn, motor_out *MotorOut, const int8_T *lic_path,
                                               const int_T p_width0, const int8_T *par_path, const int_T p_width1) {
  rear_motor_sfunction_Outputs_wrapper((moter_in *)MotorIn, (motor_out *)MotorOut, lic_path, p_width0, par_path,
                                       p_width1);
}

void rear_motor_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                 const int_T p_width1) {
  rear_motor_sfunction_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
