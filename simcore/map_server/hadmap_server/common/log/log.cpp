/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/log/log.h"

#include <iostream>

#include <string.h>
#include <time.h>
#include <map>

#include "boost/filesystem.hpp"
#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/loggingmacros.h>

std::map<LOGTYPE, log4cplus::Logger> logger_table = {{LOGXML, {}}, {LOGSYSTEM, {}}};

void CLog::Initialize(const char* strParentPath, bool use_directly) {
  boost::filesystem::path parentDir = strParentPath;
  if (parentDir.empty()) {
    return;
  }
  boost::filesystem::path logDir = use_directly ? parentDir : parentDir.append("/scene_log");
  if (!boost::filesystem::exists(logDir) || !boost::filesystem::is_directory(logDir)) {
    boost::filesystem::create_directories(logDir);
  }

  std::string time_string = GetTime();

  for (auto kv : std::map<LOGTYPE, std::string>{{LOGSYSTEM, "_system.log"}, {LOGXML, "_xml.log"}}) {
    auto log_path = logDir / (time_string + kv.second);
    auto log_name = log_path.lexically_normal().make_preferred().string();

    log4cplus::Logger logger = log4cplus::Logger::getInstance(kv.second);
    log4cplus::SharedAppenderPtr fileAppender(new log4cplus::FileAppender(log_name));
#ifdef _WIN32
    std::unique_ptr<log4cplus::Layout> layout(
        new log4cplus::PatternLayout(LOG4CPLUS_TEXT("%d{%Y-%m-%d %H:%M:%S} [%-5p] - %m%n")));
#else
    std::auto_ptr<log4cplus::Layout> layout(
        new log4cplus::PatternLayout(LOG4CPLUS_TEXT("%d{MAP_SERVER} %-5p [%c] - %m%n")));
#endif
    fileAppender->setLayout(std::move(layout));
    logger.addAppender(fileAppender);
    // logger.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
    logger_table[kv.first] = logger;
  }
}

void CLog::Finialize() {
  for (auto iter = logger_table.begin(); iter != logger_table.end(); iter++) {
    iter->second.shutdown();
  }
}

const char* CLog::ConvertParam(const char* strFormat, ...) {
  va_list va;
  const int MAX_BUFFER_SIZE = 1024;
  static char buffer[MAX_BUFFER_SIZE] = {0};
  memset(buffer, 0, MAX_BUFFER_SIZE);

  va_start(va, strFormat);
  vsnprintf(buffer, MAX_BUFFER_SIZE, strFormat, va);
  va_end(va);

  return buffer;
}

#define DEFINE_CLOG_MEMBER_FUNCTION(name, callee)         \
  void CLog::name(LOGTYPE type, const char* strContent) { \
    if (logger_table.count(type) <= 0) {                  \
      std::cout << strContent << std::endl;               \
      return;                                             \
    }                                                     \
    callee(logger_table[type], strContent);               \
  }

DEFINE_CLOG_MEMBER_FUNCTION(Info, LOG4CPLUS_INFO);
DEFINE_CLOG_MEMBER_FUNCTION(Debug, LOG4CPLUS_DEBUG);
DEFINE_CLOG_MEMBER_FUNCTION(Warn, LOG4CPLUS_WARN);
DEFINE_CLOG_MEMBER_FUNCTION(Error, LOG4CPLUS_ERROR);
DEFINE_CLOG_MEMBER_FUNCTION(Fatal, LOG4CPLUS_FATAL);

std::string CLog::GetTime() {
  std::time_t time = std::time(nullptr);
  char mbstr[64] = {0};
  std::strftime(mbstr, sizeof(mbstr), "%Y_%m_%d_%H_%M_%S", std::localtime(&time));
  return mbstr;
}

const char* CLog::GetFileName(const char* full_name) {
  if (full_name == nullptr) return "";
  std::size_t len = std::strlen(full_name);

  for (std::size_t i = len - 1; i >= 0; i--) {
    if (full_name[i] == '/' || full_name[i] == '\\') {
      return (full_name + i + 1);
    }
  }

  return full_name;
}
