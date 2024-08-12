#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "hybridparser_sfunction_cgen_wrapper.h"

extern void hybridparser_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                 const int_T p_width1);
extern void hybridparser_sfunction_Outputs_wrapper(const real_T *ununsed, real_T *EnableP2, real_T *EnableP3,
                                                   real_T *EnableP4, real_T *EngToGenerator_GearRatio,
                                                   real_T *EngClutch_TimeConst, real_T *FrontMotGearRatio,
                                                   const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                   const int_T p_width1);
extern void hybridparser_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                     const int8_T *par_path, const int_T p_width1);
void hybridparser_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                               const int_T p_width1) {
  hybridparser_sfunction_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void hybridparser_sfunction_Outputs_wrapper_cgen(const real_T *ununsed, real_T *EnableP2, real_T *EnableP3,
                                                 real_T *EnableP4, real_T *EngToGenerator_GearRatio,
                                                 real_T *EngClutch_TimeConst, real_T *FrontMotGearRatio,
                                                 const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                 const int_T p_width1) {
  hybridparser_sfunction_Outputs_wrapper(ununsed, EnableP2, EnableP3, EnableP4, EngToGenerator_GearRatio,
                                         EngClutch_TimeConst, FrontMotGearRatio, lic_path, p_width0, par_path,
                                         p_width1);
}

void hybridparser_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                   const int_T p_width1) {
  hybridparser_sfunction_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
