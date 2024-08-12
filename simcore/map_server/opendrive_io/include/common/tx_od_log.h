// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <stdarg.h>
#include "tx_od_marco.h"

#define LoaderInfo LOG_IF(INFO, FLAGS_LogLevel_XmlLoader)
#define LogInfo LOG(INFO)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(Base)

enum LOGTYPE {
  LOGXML = 0,
  LOGSYSTEM,
};

class Logger {
 public:
  static void Initialize(const char* strParentPath) TX_NOEXCEPT;
  static void Release() TX_NOEXCEPT;
  static void Debug(LOGTYPE type, const char* strContent);
  static void Info(LOGTYPE type, const char* strContent);
  static void Warn(LOGTYPE type, const char* strContent);
  static void Error(LOGTYPE type, const char* strContent);
  static void Fatal(LOGTYPE type, const char* strContent);
};

TX_NAMESPACE_CLOSE(Base)
