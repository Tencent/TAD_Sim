/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "error_code.h"
#include <boost/algorithm/string.hpp>
#include "common/log/system_logger.h"

std::string HadmapServerErrorMessage(int nErrorCode) {
  std::string strMsg = "OK";
  if (nErrorCode == HSEC_OK) {
  } else if (nErrorCode == HSEC_Error) {
    strMsg = "error ";
  } else {
    SYSTEM_LOGGER_ERROR("unknown error code!");
    strMsg = "unknown error, contact administrator!";
  }
  return strMsg;
}
