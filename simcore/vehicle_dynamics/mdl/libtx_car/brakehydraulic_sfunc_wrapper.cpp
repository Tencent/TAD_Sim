
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

/* %%%-SFUNWIZ_wrapper_includes_Changes_BEGIN --- EDIT HERE TO _END */
#include <math.h>
#include <memory>
#include "brake_hydraulic/BrakeHydraulicExt.h"
/* %%%-SFUNWIZ_wrapper_includes_Changes_END --- EDIT HERE TO _BEGIN */
#define u_width 1
#define y_width 4

/*
 * Create external references here.
 *
 */
/* %%%-SFUNWIZ_wrapper_externs_Changes_BEGIN --- EDIT HERE TO _END */
/* extern double func(double a); */
std::shared_ptr<tx_car::brake_hydraulic::BrakeHydraulicExt> brk_hyd_ptr;
ExtU_BrakeHydraulic_T g_brake_hdyd_u0;
/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Start function
 *
 */
void brakehydraulic_sfunc_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
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

  brk_hyd_ptr = std::make_shared<tx_car::brake_hydraulic::BrakeHydraulicExt>();
  brk_hyd_ptr->initialize();
  /* %%%-SFUNWIZ_wrapper_Start_Changes_END --- EDIT HERE TO _BEGIN */
}
/*
 * Output function
 *
 */
void brakehydraulic_sfunc_Outputs_wrapper(const real_T *BrkPressCmd01, real_T *BrkPressPa, const int8_T *lic_path,
                                          const int_T p_width0, const int8_T *par_path, const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
  /* This sample sets the output equal to the input
        y0[0] = g_brake_hdyd_u0[0];
   For complex signals use: y0[0].re = g_brake_hdyd_u0[0].re;
        y0[0].im = g_brake_hdyd_u0[0].im;
        y1[0].re = u1[0].re;
        y1[0].im = u1[0].im;
   */
  if (brk_hyd_ptr) {
    g_brake_hdyd_u0.BrkCmd01 = *BrkPressCmd01;
    brk_hyd_ptr->setExternalInputs(&g_brake_hdyd_u0);
    brk_hyd_ptr->step();
    const ExtY_BrakeHydraulic_T &y0 = brk_hyd_ptr->getExternalOutputs();
    std::memcpy(BrkPressPa, y0.BrkPrsPa, sizeof(real_T) * 4);
  }
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
}

/*
 * Terminate function
 *
 */
void brakehydraulic_sfunc_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                            const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Terminate code goes here.
   */
  brk_hyd_ptr->terminate();
  brk_hyd_ptr.reset();
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_END --- EDIT HERE TO _BEGIN */
}
