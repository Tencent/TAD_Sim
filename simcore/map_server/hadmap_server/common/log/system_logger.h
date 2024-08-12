/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include "common/log/log.h"

template <LOGTYPE logType>
class LoggerTemplate {
 public:
  static void Debug(const char* strFormat, ...);
  static void Info(const char* strFormat, ...);
  static void Warn(const char* strFormat, ...);
  static void Error(const char* strFormat, ...);
  static void Fatal(const char* strFormat, ...);
};

template <LOGTYPE logType>
void LoggerTemplate<logType>::Debug(const char* strFormat, ...) {
  LOG_EXTRACT_PARAM;
  CLog::Debug(logType, buffer);
}

template <LOGTYPE logType>
void LoggerTemplate<logType>::Info(const char* strFormat, ...) {
  LOG_EXTRACT_PARAM;

  CLog::Info(logType, buffer);
}
template <LOGTYPE logType>
void LoggerTemplate<logType>::Warn(const char* strFormat, ...) {
  LOG_EXTRACT_PARAM;
  CLog::Warn(logType, buffer);
}

template <LOGTYPE logType>
void LoggerTemplate<logType>::Error(const char* strFormat, ...) {
  LOG_EXTRACT_PARAM;
  CLog::Error(logType, buffer);
}

template <LOGTYPE logType>
void LoggerTemplate<logType>::Fatal(const char* strFormat, ...) {
  LOG_EXTRACT_PARAM;
  CLog::Fatal(logType, buffer);
}

typedef LoggerTemplate<LOGXML> XMLLogger;
typedef LoggerTemplate<LOGSYSTEM> SystemLogger;

#ifndef KLOG_DEBUG
#  if defined(_WIN32)
#    define SYSTEM_LOGGER_DEBUG(fmt, ...) \
      SystemLogger::Debug("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)
#  else
#    define SYSTEM_LOGGER_DEBUG(fmt, args...) \
      SystemLogger::Debug("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, ##args)
#  endif
#else
#  define SYSTEM_LOGGER_DEBUG KLOG_DEBUG
#endif

#ifndef KLOG_TRACE
#  if defined(_WIN32)
#    define SYSTEM_LOGGER_INFO(fmt, ...) \
      SystemLogger::Info("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)
#  else
#    define SYSTEM_LOGGER_INFO(fmt, args...) \
      SystemLogger::Info("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, ##args)
#  endif
#else
#  define SYSTEM_LOGGER_INFO KLOG_TRACE
#endif

#ifndef KLOG_WARNING
#  if defined(_WIN32)
#    define SYSTEM_LOGGER_WARN(fmt, ...) \
      SystemLogger::Warn("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)
#  else
#    define SYSTEM_LOGGER_WARN(fmt, args...) \
      SystemLogger::Warn("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, ##args)
#  endif
#else
#  define SYSTEM_LOGGER_WARN KLOG_WARNING
#endif

#ifndef KLOG_FATAL
#  if defined(_WIN32)
#    define SYSTEM_LOGGER_ERROR(fmt, ...) \
      SystemLogger::Error("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)
#    define SYSTEM_LOGGER_FATAL(fmt, ...) \
      SystemLogger::Fatal("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, __VA_ARGS__)
#  else
#    define SYSTEM_LOGGER_ERROR(fmt, args...) \
      SystemLogger::Error("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, ##args)
#    define SYSTEM_LOGGER_FATAL(fmt, args...) \
      SystemLogger::Fatal("%s:%d %s] " fmt, CLog::GetFileName(__FILE__), __LINE__, __FUNCTION__, ##args)
#  endif
#else
#  define SYSTEM_LOGGER_ERROR KLOG_FATAL
#  define SYSTEM_LOGGER_FATAL KLOG_FATAL
#endif

# define BOOST_NEW_API 1

#ifndef BOOST_COPY_OPTION
#  if defined(BOOST_NEW_API)
#    define BOOST_COPY_OPTION boost::filesystem::copy_options::overwrite_existing
#  else
#    define BOOST_COPY_OPTION boost::filesystem::copy_option::overwrite_if_exists
#  endif
#endif
