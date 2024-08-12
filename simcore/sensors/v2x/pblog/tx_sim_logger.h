/**
 * @file txSimLogger.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once

#include "tx_sim_log_common.h"
#include "tx_sim_log_queue.h"

#ifdef _WIN32
#  define GLOG_NO_ABBREVIATED_SEVERITIES
#endif

#include <ctime>
#include "glog/logging.h"
#include "txsim_module.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace pblog {
class txSimLogger {
 public:
  txSimLogger();
  virtual ~txSimLogger();

  void Init(tx_sim::InitHelper &helper);
  void Reset(tx_sim::ResetHelper &helper);
  void Step(tx_sim::StepHelper &helper);
  void Stop(tx_sim::StopHelper &helper);

  bool ProducePBLogEvent(const std::string &topic, const std::string &payload_in, const int &t_ms);

 private:
  bool IsInSubTopic(const std::string &topicIn);
  void ClearPbLogQueuePtr();
  void PostProcessFunc(const std::string script, const std::string logfile);
  void DeleteEmptyLog(const bool data_logged);

 private:
  const std::string name = "protobuf logger";
  std::string payload;

  std::string _log_file_path;
  std::vector<std::string> _topic_list;
  std::string _log_file_name;
  std::string _post_script;

  bool _data_logged;
  int _sync_process;

  CPBLogQueue *_pbLog_queue_ptr;
  bool m_bJson = true;
};
}  // namespace pblog
