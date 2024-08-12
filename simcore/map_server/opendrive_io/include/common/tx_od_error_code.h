// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

enum txErrorCode {
  EC_SUCCESS = 0,
  EC_INVALID_INPUT = -1,
  EC_FILE_NOT_FOUND = -2,
  EC_UNKNOW_SESSION = -3,              // session is error
  EC_FILE_NOT_WRITE = -4,              // file is not allow write
  EC_INPUT_STRING_ERROR = -5,          // invaild input
  EC_REOPEN_MAP = -6,                  // invaild input
  EC_SESSION_NULL = -7,                // session is null
  EC_UNSUPPORT_MAP_TYPE_VERSION = -8,  // map type version is not support
  EC_FILE_LOAD_FAIL = -9,              // file load fail
  EC_INVALID_DATA = -10,               // invalid data
  EC_UNKNOW_DATA = -11,                // unknow data
  EC_FILE_CREATE_FAIL = -12,           // file create fail
  EC_DATA_IS_EMPTY = -13,              // data is empty
  EC_DATA_IS_ERROR = -14,              // data is error
  EC_CLOSE_MAP_ERROR = -15,            // close map error
  EC_UNKNOW_ERROR
};

struct tx_od_error_info {
  enum { ERR_DESC_SIZE = 256 };
  txErrorCode err_code;
  char err_desc[ERR_DESC_SIZE];
};
