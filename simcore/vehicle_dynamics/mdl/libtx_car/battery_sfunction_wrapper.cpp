
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

#include "battery_sfunction_bus.h"

/* %%%-SFUNWIZ_wrapper_includes_Changes_BEGIN --- EDIT HERE TO _END */
#include <math.h>
#include <iostream>
#include "BatteryExt.h"
/* %%%-SFUNWIZ_wrapper_includes_Changes_END --- EDIT HERE TO _BEGIN */
#define u_width 1
#define y_width 1

/*
 * Create external references here.
 *
 */
/* %%%-SFUNWIZ_wrapper_externs_Changes_BEGIN --- EDIT HERE TO _END */
/* extern double func(double a); */

tx_car::power::BatteryExt g_battery;
/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Start function
 *
 */
void battery_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
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

  g_battery.initialize();
  /* %%%-SFUNWIZ_wrapper_Start_Changes_END --- EDIT HERE TO _BEGIN */
}
/*
 * Output function
 *
 */
void battery_sfunction_Outputs_wrapper(const batt_in *BattIn, batt_out *BattOut, const int8_T *lic_path,
                                       const int_T p_width0, const int8_T *par_path, const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
  tx_car::power::ExtU_Battery_T in;
  in.batt_input = *BattIn;

  g_battery.setExternalInputs(&in);
  // LOG_0<<"g_batteryIn.mot_in.trq_cmd_Nm "<<g_batteryIn.mot_in.trq_cmd_Nm<<"\n";
  g_battery.step();
  *BattOut = g_battery.getExternalOutputs().batt_output;
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
}

/*
 * Terminate function
 *
 */
void battery_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                         const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Terminate code goes here.
   */
  g_battery.terminate();
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_END --- EDIT HERE TO _BEGIN */
}
