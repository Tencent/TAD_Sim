
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

#include "transmission_sfunction_bus.h"

/* %%%-SFUNWIZ_wrapper_includes_Changes_BEGIN --- EDIT HERE TO _END */
#include <math.h>
#include <memory>
#include "TX_TransmissionExt.h"
/* %%%-SFUNWIZ_wrapper_includes_Changes_END --- EDIT HERE TO _BEGIN */
#define u_width 1
#define u_1_width 1
#define u_2_width 1
#define y_width 1
#define y_1_width 1
#define y_2_width 1

/*
 * Create external references here.
 *
 */
/* %%%-SFUNWIZ_wrapper_externs_Changes_BEGIN --- EDIT HERE TO _END */
/* extern double func(double a); */
std::shared_ptr<tx_car::transmission::TX_TransmissionExt> g_transmission;
/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Start function
 *
 */
void transmission_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                          const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Start_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Start code goes here.
   */
  /*create transmission*/
  g_transmission = std::make_shared<tx_car::transmission::TX_TransmissionExt>();

  char lic_buf[256], par_buf[256];
  memcpy(lic_buf, lic_path, p_width0);
  memcpy(par_buf, par_path, p_width1);
  lic_buf[p_width0] = '\0';
  par_buf[p_width1] = '\0';
  tx_car::car_config::setParPath(par_buf);

  g_transmission->initialize();
  /* %%%-SFUNWIZ_wrapper_Start_Changes_END --- EDIT HERE TO _BEGIN */
}
/*
 * Output function
 *
 */
void transmission_sfunction_Outputs_wrapper(const driveline_in *DrivelineIn, const powecu_out *softecu_output,
                                            const real_T *PropShftSpd, real_T *TransGear, real_T *EngSpeed,
                                            real_T *DrvShfTrq, const int8_T *lic_path, const int_T p_width0,
                                            const int8_T *par_path, const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
  ExtU_TX_Transmission_T in;
  in.DriveLineIn = *DrivelineIn;
  in.SoftECUOut = *softecu_output;
  in.PropShftSpd = *PropShftSpd;

  g_transmission->setExternalInputs(&in);
  // LOG_0<<"g_transmissionIn.mot_in.trq_cmd_Nm "<<g_transmissionIn.mot_in.trq_cmd_Nm<<"\n";
  g_transmission->step();
  auto transOut = g_transmission->getExternalOutputs();
  *TransGear = transOut.TransGear;
  *EngSpeed = transOut.EngSpeedrads;
  *DrvShfTrq = transOut.DrvShfTrqNm;
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
}

/*
 * Terminate function
 *
 */
void transmission_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                              const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Terminate code goes here.
   */
  g_transmission->terminate();
  g_transmission.reset();
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_END --- EDIT HERE TO _BEGIN */
}
