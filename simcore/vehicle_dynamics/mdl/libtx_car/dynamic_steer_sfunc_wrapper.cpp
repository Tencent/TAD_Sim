
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

#include "dynamic_steer_sfunc_bus.h"

/* %%%-SFUNWIZ_wrapper_includes_Changes_BEGIN --- EDIT HERE TO _END */
#include <math.h>
#include <memory>
#include "dynamic_steer/DynamicSteerExt.h"
/* %%%-SFUNWIZ_wrapper_includes_Changes_END --- EDIT HERE TO _BEGIN */
#define u_width 1
#define u_1_width 1
#define u_2_width 1
#define u_3_width 1
#define y_width 1

/*
 * Create external references here.
 *
 */
/* %%%-SFUNWIZ_wrapper_externs_Changes_BEGIN --- EDIT HERE TO _END */
/* extern double func(double a); */
std::shared_ptr<tx_car::steer::DynamicSteerExt> steer_ptr;
ExtU_DynamicSteer_T g_steer_u;
/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Start function
 *
 */
void dynamic_steer_sfunc_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                       const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Start_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Start code goes here.
   */
  char lic_buf[256], par_buf[256];
  memcpy(lic_buf, lic_path, p_width0);
  memcpy(par_buf, par_path, p_width1);
  lic_buf[p_width0] = '\0';
  par_buf[p_width1] = '\0';
  tx_car::car_config::setParPath(par_buf);

  steer_ptr = std::make_shared<tx_car::steer::DynamicSteerExt>();
  if (steer_ptr) steer_ptr->initialize();
  /* %%%-SFUNWIZ_wrapper_Start_Changes_END --- EDIT HERE TO _BEGIN */
}
/*
 * Output function
 *
 */
void dynamic_steer_sfunc_Outputs_wrapper(const Ctrl_Input *driver_in, const Vehicle_Body_Output *body_in,
                                         const Susp_Output *susp_in, const Wheels_Output *wheel_in,
                                         Steer_Output *steer_out, const int8_T *lic_path, const int_T p_width0,
                                         const int8_T *par_path, const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
  /* This sample sets the output equal to the input
        y0[0] = u0[0];
   For complex signals use: y0[0].re = u0[0].re;
        y0[0].im = u0[0].im;
        y1[0].re = u1[0].re;
        y1[0].im = u1[0].im;
   */
  if (steer_ptr) {
    g_steer_u = {*driver_in, *body_in, *susp_in, *wheel_in};
    steer_ptr->setExternalInputs(&g_steer_u);
    steer_ptr->step();
    const ExtY_DynamicSteer_T &ext_y = steer_ptr->getExternalOutputs();
    *steer_out = ext_y.steerOut;
  } else {
    std::cerr << "steer model is nullptr.\n";
  }
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
}

/*
 * Terminate function
 *
 */
void dynamic_steer_sfunc_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                           const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Terminate code goes here.
   */
  if (steer_ptr) steer_ptr->terminate();
  steer_ptr.reset();
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_END --- EDIT HERE TO _BEGIN */
}
