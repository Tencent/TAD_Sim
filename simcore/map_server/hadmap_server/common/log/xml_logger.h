/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include "common/log/log.h"

class CXmlLogger {
 public:
  static void Info(const char* strFormat, ...);
  static void Debug(const char* strFormat, ...);
  static void Warn(const char* strFormat, ...);
  static void Error(const char* strFormat, ...);
  static void Fatal(const char* strFormat, ...);
};

#ifndef KLOG_DEBUG
#  if defined(_WIN32)
#    define XML_LOGGER_DEBUG(fmt, ...) \
      CXmlLogger::Debug("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)
#  else
#    define XML_LOGGER_DEBUG(fmt, args...) \
      CXmlLogger::Debug("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, ##args)
#  endif
#else
#  define XML_LOGGER_DEBUG KLOG_DEBUG
#endif

#ifndef KLOG_TRACE
#  if defined(_WIN32)
#    define XML_LOGGER_INFO(fmt, ...) \
      CXmlLogger::Info("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)
#  else
#    define XML_LOGGER_INFO(fmt, args...) \
      CXmlLogger::Info("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, ##args)
#  endif
#else
#  define XML_LOGGER_INFO KLOG_TRACE
#endif

#ifndef KLOG_WARNING
#  if defined(_WIN32)
#    define XML_LOGGER_WARN(fmt, ...) \
      CXmlLogger::Warn("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)
#  else
#    define XML_LOGGER_WARN(fmt, args...) \
      CXmlLogger::Warn("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, ##args)
#  endif
#else
#  define XML_LOGGER_WARN KLOG_WARNING
#endif

#ifndef KLOG_FATAL
#  if defined(_WIN32)
#    define XML_LOGGER_ERROR(fmt, ...) \
      CXmlLogger::Error("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)
#    define XML_LOGGER_FATAL(fmt, ...) \
      CXmlLogger::Fatal("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)
#  else
#    define XML_LOGGER_ERROR(fmt, args...) \
      CXmlLogger::Error("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, ##args)
#    define XML_LOGGER_FATAL(fmt, args...) \
      CXmlLogger::Fatal("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, ##args)
#  endif
#else
#  define XML_LOGGER_ERROR KLOG_FATAL
#  define XML_LOGGER_FATAL KLOG_FATAL
#endif
