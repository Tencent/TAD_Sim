// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_logger.h"
#include "tx_flags.h"
#include "tx_path_utils.h"
#include "tx_type_def.h"
TX_NAMESPACE_OPEN(Base)

void Logger::Initialize(int argc, char* argv[]) {
  /*1. create log directory if necessary.*/
  if (FLAGS_tx_log_dir.size() > 0) {
    FLAGS_log_dir = FLAGS_tx_log_dir;
    Utils::CreateDirectory(FLAGS_tx_log_dir);
    LOG(INFO) << FLAGS_tx_log_dir;
  }

  // FLAGS_logtostderr = 1;
  FLAGS_alsologtostderr = FLAGS_LogLevel_Console;

  FLAGS_logbufsecs = 0;

  FLAGS_max_log_size = 100;

  FLAGS_stop_logging_if_full_disk = true;

  google::SetLogFilenameExtension("stlog");

  /*FLAGS_logtostderr = true;*/

  FLAGS_colorlogtostderr = true;

  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "Log init success.";
}

void Logger::Release() { google::ShutdownGoogleLogging(); }

TX_NAMESPACE_CLOSE(Base)
