// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_od_api_assist.h"
#include <cstring>

TX_NAMESPACE_OPEN(Utils)

#define txMin(a, b) (((a) < (b)) ? (a) : (b))

tx_od_error_info make_error_code(const txErrorCode _c, const Base::txString& strInfo,
                                 tx_od_error_info& ret) TX_NOEXCEPT {
  ret.err_code = _c;
  memset(&ret.err_desc[0], '\0', sizeof(ret.err_desc));
  memcpy(&ret.err_desc[0], strInfo.c_str(), txMin(tx_od_error_info::ERR_DESC_SIZE - 1, (strInfo.size() + 1)));
  return ret;
}

tx_od_error_info make_error_code(const txErrorCode _c, const Base::txString& strInfo) TX_NOEXCEPT {
  tx_od_error_info ret;
  ret.err_code = _c;
  memset(&ret.err_desc[0], '\0', sizeof(ret.err_desc));
  memcpy(&ret.err_desc[0], strInfo.c_str(), txMin(tx_od_error_info::ERR_DESC_SIZE - 1, (strInfo.size() + 1)));
  return std::move(ret);
}

TX_NAMESPACE_CLOSE(Utils)
