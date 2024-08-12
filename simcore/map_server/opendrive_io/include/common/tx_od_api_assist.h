// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_od_error_code.h"
#include "tx_type_defs.h"

TX_NAMESPACE_OPEN(Utils)
/*!
 * Concatenation of status information, [C Interface]
 *
 * @param[txErrorCode _c Status code] key The key of the initialization parameter.
 * @return The corresponding string value for the key. If the frontend does not add the corresponding initialization
 * parameter key, an empty string is returned.
 */
tx_od_error_info make_error_code(const txErrorCode _c, const Base::txString& strInfo) TX_NOEXCEPT;
tx_od_error_info make_error_code(const txErrorCode _c, const Base::txString& strInfo,
                                 tx_od_error_info& ret_err_code) TX_NOEXCEPT;

inline tx_od_error_info make_error_code(const txErrorCode _c, const Base::txStringStream& streamInfo,
                                        tx_od_error_info& ret_err_code) TX_NOEXCEPT {
  return make_error_code(_c, streamInfo.str(), ret_err_code);
}
TX_NAMESPACE_CLOSE(Utils)
