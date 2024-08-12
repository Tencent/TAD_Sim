/*
 * Battery_types.h
 *
 * Code generation for model "Battery".
 *
 * Model version              : 1.33
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Tue Jun 14 11:38:22 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_Battery_types_h_
#define RTW_HEADER_Battery_types_h_
#include "rtwtypes.h"

namespace tx_car {
namespace power {

/* Model Code Variants */
#ifndef DEFINED_TYPEDEF_FOR_batt_in_
#  define DEFINED_TYPEDEF_FOR_batt_in_

struct batt_in {
  real_T batt_curr_A;
  real_T batt_temp_K;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_batt_out_
#  define DEFINED_TYPEDEF_FOR_batt_out_

struct batt_out {
  real_T batt_volt_V;
  real_T batt_soc_null;
  real_T batt_pwr_W;
  real_T batt_loss_pwr_W;
};

#endif

/* Parameters (default storage) */
typedef struct P_Battery_T_ P_Battery_T;

/* Forward declaration for rtModel */
typedef struct tag_RTM_Battery_T RT_MODEL_Battery_T;

}  // namespace power
}  // namespace tx_car

#endif /* RTW_HEADER_Battery_types_h_ */
