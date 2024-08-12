
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
#include "hybrid_parser/HybridParserExt.h"
/* %%%-SFUNWIZ_wrapper_includes_Changes_END --- EDIT HERE TO _BEGIN */
#define u_width 1
#define y_width 1
#define y_1_width 1
#define y_2_width 1
#define y_3_width 1
#define y_4_width 1
#define y_5_width 1

/*
 * Create external references here.
 *
 */
/* %%%-SFUNWIZ_wrapper_externs_Changes_BEGIN --- EDIT HERE TO _END */
/* extern double func(double a); */

std::shared_ptr<tx_car::hybrid_parser::HybridParserExt> g_hybrid_parser;
/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Start function
 *
 */
void hybridparser_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
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

  g_hybrid_parser = std::make_shared<tx_car::hybrid_parser::HybridParserExt>();

  g_hybrid_parser->initialize();
  /* %%%-SFUNWIZ_wrapper_Start_Changes_END --- EDIT HERE TO _BEGIN */
}
/*
 * Output function
 *
 */
void hybridparser_sfunction_Outputs_wrapper(const real_T *ununsed, real_T *EnableP2, real_T *EnableP3, real_T *EnableP4,
                                            real_T *EngToGenerator_GearRatio, real_T *EngClutch_TimeConst,
                                            real_T *FrontMotGearRatio, const int8_T *lic_path, const int_T p_width0,
                                            const int8_T *par_path, const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
  ExtU_HybridParser_T in;
  in.NoUse = *ununsed;

  g_hybrid_parser->setExternalInputs(&in);
  g_hybrid_parser->step();

  const auto &ecu_out = g_hybrid_parser->getExternalOutputs();
  *EnableP2 = ecu_out.EnableP2;
  *EnableP3 = ecu_out.EnableP3;
  *EnableP4 = ecu_out.EnableP4;
  *EngToGenerator_GearRatio = ecu_out.EngToGenerator_GearRatio;
  *EngClutch_TimeConst = ecu_out.EngClutch_TimeConsts;
  *FrontMotGearRatio = ecu_out.FrontMotGearRatio;
  /* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
}

/*
 * Terminate function
 *
 */
void hybridparser_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                              const int_T p_width1) {
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_BEGIN --- EDIT HERE TO _END */
  /*
   * Custom Terminate code goes here.
   */
  g_hybrid_parser->terminate();
  g_hybrid_parser.reset();
  /* %%%-SFUNWIZ_wrapper_Terminate_Changes_END --- EDIT HERE TO _BEGIN */
}
