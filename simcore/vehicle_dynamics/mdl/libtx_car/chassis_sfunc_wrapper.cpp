
/*
 * Include Files
 *
 */
#if defined(MATLAB_MEX_FILE)
#  include "simstruc_types.h"
#  include "tmwtypes.h"
#else
#  define SIMPLIFIED_RTWTYPES_COMPATIBILITY
#  include "rtwtypes.h"
#  undef SIMPLIFIED_RTWTYPES_COMPATIBILITY
#endif

#include "chassis_sfunc_bus.h"

/* %%%-SFUNWIZ_wrapper_includes_Changes_BEGIN --- EDIT HERE TO _END */
#include <math.h>
#include <memory>
#include "chassis/ChassisExt.h"
#include "inc/car_common.h"
/* %%%-SFUNWIZ_wrapper_includes_Changes_END --- EDIT HERE TO _BEGIN */
#define u_width 1
#define u_1_width 1
#define y_width 1
#define y_1_width 1
#define y_2_width 1

/*
 * Create external references here.
 *
 */
/* %%%-SFUNWIZ_wrapper_externs_Changes_BEGIN --- EDIT HERE TO _END */
/* extern double func(double a); */
std::shared_ptr<tx_car::chassis::ChassisExt> chs_ptr;
/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Start function
 *
 */
void chassis_sfunc_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                 const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Start_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Start code goes here.
   */
  chs_ptr = std::make_shared<tx_car::chassis::ChassisExt>();

  char lic_buf[256], par_buf[256];
  memcpy(lic_buf, lic_path, p_width0);
  memcpy(par_buf, par_path, p_width1);
  lic_buf[p_width0] = '\0';
  par_buf[p_width1] = '\0';
  tx_car::car_config::setParPath(par_buf);

  chs_ptr->initialize();
  /* %%%-SFUNWIZ_wrapper_Start_Changes_END --- EDIT HERE TO _BEGIN */
}
/*
 * Output function
 *
 */
void chassis_sfunc_Outputs_wrapper(const Env_Bus_Output *env_in, const Chassis_DriveLine_Input *driveline_in,
                                   Vehicle_Body_Output *body_out, Wheels_Output *wheel_out, Susp_Output *susp_out,
                                   const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                   const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
  /* This sample sets the output equal to the input
        y0[0] = u0[0];
   For complex signals use: y0[0].re = u0[0].re;
        y0[0].im = u0[0].im;
        y1[0].re = u1[0].re;
        y1[0].im = u1[0].im;
   */
  if (chs_ptr) {
    ExtU_Chassis_T u0;
    u0.DriveLineInput = *driveline_in;
    u0.Env = *env_in;
    chs_ptr->setExternalInputs(&u0);
    chs_ptr->step();
    const ExtY_Chassis_T &y0 = chs_ptr->getExternalOutputs();
    *body_out = y0.VehBody;
    *wheel_out = y0.WheelsOut;
    *susp_out = y0.Susp;
  }
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
}

/*
 * Terminate function
 *
 */
void chassis_sfunc_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                     const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Terminate code goes here.
   */
  chs_ptr->terminate();
  chs_ptr.reset();
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_END --- EDIT HERE TO _BEGIN */
}
