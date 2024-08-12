
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

#include "softabs_sfunction_bus.h"

/* %%%-SFUNWIZ_wrapper_includes_Changes_BEGIN --- EDIT HERE TO _END */
#include <math.h>
#include <memory>
#include "softabs/SoftABSExt.h"
/* %%%-SFUNWIZ_wrapper_includes_Changes_END --- EDIT HERE TO _BEGIN */
#define u_width 1
#define y_width 1

/*
 * Create external references here.
 *
 */
/* %%%-SFUNWIZ_wrapper_externs_Changes_BEGIN --- EDIT HERE TO _END */
/* extern double func(double a); */

std::shared_ptr<tx_car::abs::SoftABSExt> g_softabs;
/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Start function
 *
 */
void softabs_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                     const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Start_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Start code goes here.
   */
  /*?????????????????????????*/

  char lic_buf[256], par_buf[256];
  memcpy(lic_buf, lic_path, p_width0);
  memcpy(par_buf, par_path, p_width1);
  lic_buf[p_width0] = '\0';
  par_buf[p_width1] = '\0';
  tx_car::car_config::setParPath(par_buf);

  g_softabs = std::make_shared<tx_car::abs::SoftABSExt>();

  g_softabs->initialize();
  /* %%%-SFUNWIZ_wrapper_Start_Changes_END --- EDIT HERE TO _BEGIN */
}
/*
 * Output function
 *
 */
void softabs_sfunction_Outputs_wrapper(const powecu_in *ecu_in, powecu_out *ecu_out, const int8_T *lic_path,
                                       const int_T p_width0, const int8_T *par_path, const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
  ExtU_SoftABS_T in;
  in.ecu_in = *ecu_in;

  g_softabs->setExternalInputs(&in);
  g_softabs->step();

  *ecu_out = g_softabs->getExternalOutputs().ecu_out;
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
}

/*
 * Terminate function
 *
 */
void softabs_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                         const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Terminate code goes here.
   */
  g_softabs->terminate();
  g_softabs.reset();
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_END --- EDIT HERE TO _BEGIN */
}
