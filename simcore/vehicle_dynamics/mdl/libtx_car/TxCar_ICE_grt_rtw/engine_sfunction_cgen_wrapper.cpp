#include <string.h>
#ifdef MATLAB_MEX_FILE
#  include "tmwtypes.h"
#else
#  include "rtwtypes.h"
#endif

#include "engine_sfunction_cgen_wrapper.h"

extern void engine_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                           const int_T p_width1);
extern void engine_sfunction_Outputs_wrapper(const ENGINE_INPUTBUS *EngineIn, ENGINE_OUTPUTBUS *EngineOut,
                                             const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                             const int_T p_width1);
extern void engine_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                               const int_T p_width1);
void engine_sfunction_Start_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                         const int_T p_width1) {
  engine_sfunction_Start_wrapper(lic_path, p_width0, par_path, p_width1);
}

void engine_sfunction_Outputs_wrapper_cgen(const ENGINE_INPUTBUS *EngineIn, ENGINE_OUTPUTBUS *EngineOut,
                                           const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                           const int_T p_width1) {
  engine_sfunction_Outputs_wrapper(
      const_cast<ENGINE_INPUTBUS *>(reinterpret_cast<const ENGINE_INPUTBUS *>(EngineIn)),
      const_cast<ENGINE_OUTPUTBUS *>(reinterpret_cast<const ENGINE_OUTPUTBUS *>(EngineOut)), lic_path, p_width0,
      par_path, p_width1);
}

void engine_sfunction_Terminate_wrapper_cgen(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                             const int_T p_width1) {
  engine_sfunction_Terminate_wrapper(lic_path, p_width0, par_path, p_width1);
}
