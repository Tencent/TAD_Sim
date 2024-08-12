/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>

#include <stdarg.h>

#ifdef USE_FOUNDATION_LOG
#  include "Log.h"
#endif

enum LOGTYPE {
  LOGXML = 0,
  LOGSYSTEM,
};

#define LOG_EXTRACT_PARAM                            \
  va_list va;                                        \
  const int MAX_BUFFER_SIZE = 10240;                 \
  char buffer[MAX_BUFFER_SIZE] = {0};                \
  va_start(va, strFormat);                           \
  vsnprintf(buffer, MAX_BUFFER_SIZE, strFormat, va); \
  va_end(va);

class CLog {
 public:
  static void Initialize(const char* strParentPath, bool use_directly = false);
  static void Finialize();
  static void Debug(LOGTYPE type, const char* strContent);
  static void Info(LOGTYPE type, const char* strContent);
  static void Warn(LOGTYPE type, const char* strContent);
  static void Error(LOGTYPE type, const char* strContent);
  static void Fatal(LOGTYPE type, const char* strContent);

  static const char* ConvertParam(const char* strFormat, ...);

  static std::string GetTime();
  static const char* GetFileName(const char* full_name);
};
