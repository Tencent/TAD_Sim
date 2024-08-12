
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
#include "TX_DriveLine_ICE_Ext.h"
/* %%%-SFUNWIZ_wrapper_includes_Changes_END --- EDIT HERE TO _BEGIN */
#define u_width 1
#define u_1_width 4
#define u_2_width 4
#define y_width 4
#define y_1_width 1

/*
 * Create external references here.
 *
 */
/* %%%-SFUNWIZ_wrapper_externs_Changes_BEGIN --- EDIT HERE TO _END */
/* extern double func(double a); */
std::shared_ptr<tx_car::driveline_ice::TX_DriveLine_ICE_Ext> g_driveline_ICE;
/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Start function
 *
 */
void driveline_ice_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                           const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Start_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Start code goes here.
   */
  /*create transmission*/
  g_driveline_ICE = std::make_shared<tx_car::driveline_ice::TX_DriveLine_ICE_Ext>();

  char lic_buf[256], par_buf[256];
  memcpy(lic_buf, lic_path, p_width0);
  memcpy(par_buf, par_path, p_width1);
  lic_buf[p_width0] = '\0';
  par_buf[p_width1] = '\0';
  tx_car::car_config::setParPath(par_buf);

  g_driveline_ICE->initialize();
  /* %%%-SFUNWIZ_wrapper_Start_Changes_END --- EDIT HERE TO _BEGIN */
}
/*
 * Output function
 *
 */
void driveline_ice_sfunction_Outputs_wrapper(const real_T *DrvShfTrq, const real_T *OmegaAxle, const real_T *DiffPrsCmd,
                                             real_T *AxlTrq, real_T *PropShaftSpd, const int8_T *lic_path,
                                             const int_T p_width0, const int8_T *par_path, const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
  ExtU_TX_DriveLine_ICE_T in;
  in.DrvShfTrqNm = *DrvShfTrq;
  memcpy(in.OmegaAxlerads, OmegaAxle, sizeof(real_T) * 4);
  memcpy(in.DiffPrsCmd, DiffPrsCmd, sizeof(real_T) * 4);

  g_driveline_ICE->setExternalInputs(&in);
  g_driveline_ICE->step();
  auto out = g_driveline_ICE->getExternalOutputs();
  memcpy(AxlTrq, out.AxlTrqNm, sizeof(real_T) * 4);
  *PropShaftSpd = out.PropShaftSpdNm;
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
}

/*
 * Terminate function
 *
 */
void driveline_ice_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                               const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Terminate code goes here.
   */
  g_driveline_ICE->terminate();
  g_driveline_ICE.reset();
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_END --- EDIT HERE TO _BEGIN */
}
