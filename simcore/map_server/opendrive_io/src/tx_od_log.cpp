// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_od_log.h"
#include "boost/filesystem.hpp"
#include "tx_od_flags.h"
TX_NAMESPACE_OPEN(Base)

// Logger::Initialize function to initialize the logger with the specified parent path
void Logger::Initialize(const char* strParentPath) TX_NOEXCEPT {
  // Set the FLAGS_alsologtostderr flag to the value of FLAGS_LogLevel_Console
  FLAGS_alsologtostderr = FLAGS_LogLevel_Console;

  // Set the FLAGS_logbufsecs flag to 0
  FLAGS_logbufsecs = 0;

  // Set the FLAGS_max_log_size flag to 100
  FLAGS_max_log_size = 100;

  // Set the FLAGS_stop_logging_if_full_disk flag to true
  FLAGS_stop_logging_if_full_disk = true;

  // Set the log filename extension to "stlog"
  google::SetLogFilenameExtension("stlog");

  // Set the FLAGS_colorlogtostderr flag to true
  FLAGS_colorlogtostderr = true;

  // Create a logDir path
  boost::filesystem::path parentDir = strParentPath;
  boost::filesystem::path logDir = parentDir.append("/hadmap_log");

  // Check if the logDir does not exist or is not a directory, and create the necessary directories if needed
  if (!boost::filesystem::exists(logDir) || !boost::filesystem::is_directory(logDir)) {
    boost::filesystem::create_directories(logDir);
  }

  // Initialize Google logging with the mapedit
  google::InitGoogleLogging(AppName);

  // Set the FLAGS_log_dir flag to the logDir path
  FLAGS_log_dir = logDir.string().c_str();

  // Log an INFO message indicating that the log initialization was successful
  LOG(INFO) << "Log init success.";
}

void Logger::Release() TX_NOEXCEPT { google::ShutdownGoogleLogging(); }

void Logger::Debug(LOGTYPE type, const char* strContent) { LOG(INFO) << strContent; }

void Logger::Info(LOGTYPE type, const char* strContent) { LOG(INFO) << strContent; }

void Logger::Warn(LOGTYPE type, const char* strContent) { LOG(WARNING) << strContent; }

void Logger::Error(LOGTYPE type, const char* strContent) { LOG(ERROR) << strContent; }

void Logger::Fatal(LOGTYPE type, const char* strContent) { LOG(FATAL) << strContent; }

TX_NAMESPACE_CLOSE(Base)
