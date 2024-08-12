
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

#include "driveline_ev_rear_sfunction_bus.h"

/* %%%-SFUNWIZ_wrapper_includes_Changes_BEGIN --- EDIT HERE TO _END */
#include <math.h>
#include <memory>
#include "TX_DriveLine_EV_Rear_Ext.h"
/* %%%-SFUNWIZ_wrapper_includes_Changes_END --- EDIT HERE TO _BEGIN */
#define u_width 1
#define y_width 1

/*
 * Create external references here.
 *
 */
/* %%%-SFUNWIZ_wrapper_externs_Changes_BEGIN --- EDIT HERE TO _END */
/* extern double func(double a); */
std::shared_ptr<tx_car::driveline_ev_rear::TX_DriveLine_EV_Rear_Ext> g_driveline_ev_rear;
/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Start function
 *
 */
void driveline_ev_rear_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                               const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Start_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Start code goes here.
   */
  /*?????????????????????????*/
  g_driveline_ev_rear = std::make_shared<tx_car::driveline_ev_rear::TX_DriveLine_EV_Rear_Ext>();

  char lic_buf[256], par_buf[256];
  memcpy(lic_buf, lic_path, p_width0);
  memcpy(par_buf, par_path, p_width1);
  lic_buf[p_width0] = '\0';
  par_buf[p_width1] = '\0';
  tx_car::car_config::setParPath(par_buf);

  g_driveline_ev_rear->initialize();
  /* %%%-SFUNWIZ_wrapper_Start_Changes_END --- EDIT HERE TO _BEGIN */
}
/*
 * Output function
 *
 */
void driveline_ev_rear_sfunction_Outputs_wrapper(const driveline_in *DrivelineIn, driveline_out *DrivelineOut,
                                                 const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                 const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
  ExtU_TX_DriveLine_EV_Rear_T in;
  in.DriveLineIn = *DrivelineIn;

  g_driveline_ev_rear->setExternalInputs(&in);
  // LOG_0<<"g_driveline_ev_rearIn.mot_in.trq_cmd_Nm "<<g_driveline_ev_rearIn.mot_in.trq_cmd_Nm<<"\n";
  g_driveline_ev_rear->step();
  *DrivelineOut = g_driveline_ev_rear->getExternalOutputs().DrivelineOut;
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
}

/*
 * Terminate function
 *
 */
void driveline_ev_rear_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                   const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Terminate code goes here.
   */
  g_driveline_ev_rear->terminate();
  g_driveline_ev_rear.reset();
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_END --- EDIT HERE TO _BEGIN */
}
