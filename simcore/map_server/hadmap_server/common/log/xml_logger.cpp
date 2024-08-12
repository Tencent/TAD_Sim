/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/log/xml_logger.h"

void CXmlLogger::Info(const char* strFormat, ...) {
  LOG_EXTRACT_PARAM;

  CLog::Info(LOGXML, buffer);
}

void CXmlLogger::Debug(const char* strFormat, ...) {
  LOG_EXTRACT_PARAM;

  CLog::Debug(LOGXML, buffer);
}

void CXmlLogger::Warn(const char* strFormat, ...) {
  LOG_EXTRACT_PARAM;

  CLog::Warn(LOGXML, buffer);
}

void CXmlLogger::Error(const char* strFormat, ...) {
  LOG_EXTRACT_PARAM;

  CLog::Error(LOGXML, buffer);
}

void CXmlLogger::Fatal(const char* strFormat, ...) {
  LOG_EXTRACT_PARAM;

  CLog::Fatal(LOGXML, buffer);
}
