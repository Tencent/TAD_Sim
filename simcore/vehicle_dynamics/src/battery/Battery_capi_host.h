#ifndef RTW_HEADER_Battery_cap_host_h__
#  define RTW_HEADER_Battery_cap_host_h__
#  ifdef HOST_CAPI_BUILD
#    include "rtw_capi.h"
#    include "rtw_modelmap.h"

struct Battery_host_DataMapInfo_T {
  rtwCAPI_ModelMappingInfo mmi;
};

#    ifdef __cplusplus

extern "C" {

#    endif

namespace tx_car {
namespace power {

void Battery_host_InitializeDataMapInfo(Battery_host_DataMapInfo_T *dataMap, const char *path);

}
}  // namespace tx_car

#    ifdef __cplusplus
}
#    endif
#  endif /* HOST_CAPI_BUILD */
#endif   /* RTW_HEADER_Battery_cap_host_h__ */

/* EOF: Battery_capi_host.h */
