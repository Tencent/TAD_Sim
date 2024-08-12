// Copyright 2024 Tencent Inc. All rights reserved.
/**
 * @file txSimLogger.h
 * @author DongYuanHu (dongyuanhu@tencent.com)
 * @brief protobuf logger module, no TXSIM_MODULE declaration
 * @version 0.1
 * @date 2023-07-05
 *
 *
 */

#pragma once

#include "txsim_log_common.h"
#include "txsim_log_queue.h"

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

/**
 * @brief Logger defination about how to record logs and where they are stored.
 */
namespace pblog {
class txSimLogger {
 public:
  txSimLogger();
  virtual ~txSimLogger();

  void Init(tx_sim::InitHelper &helper);
  void Reset(tx_sim::ResetHelper &helper);
  void Step(tx_sim::StepHelper &helper);
  void Stop(tx_sim::StopHelper &helper);

  // use this function to push subcribe data to _pbLog_queue_ptr, and _pbLog_queue_ptr will handle it automatically.
  bool ProducePBLogEvent(const std::string &topic, const std::string &payload_in, const int &t_ms);

 private:
  // check if the topic in the topic list
  bool IsInSubTopic(const std::string &topicIn);
  // clear _pbLog_queue_ptr
  void ClearPbLogQueuePtr();
  // post process after simulation finished
  void PostProcessFunc(const std::string script, const std::string logfile);
  // delete empty log files when there is not any data logged
  void DeleteEmptyLog(const bool data_logged);
  /**
   * @brief preprocess the subscribe topic data
   * @param topic_in the origin topic name
   * @param payload_in the payload of the origin topic
   * @param topic_payload_out the list of the output topic data. The key is the output topic name, the value is the
   * output payload.
   */
  void PreProcessTopicData(const std::string &topic_in, const std::string &payload_in,
                           std::vector<std::pair<std::string, std::string>> &topic_payload_out);

 private:
  const std::string name = "protobuf logger";
  std::string payload;

  std::string _log_file_path;
  std::vector<std::string> _topic_list;
  std::string _log_file_name;
  std::string _post_script;
  const std::string TOPIC_PBLOG_FILE_PATH = "PBLOG_FILE_PATH";

  bool _data_logged;
  int _sync_process;

  // the queue ptr of protobuf logger
  CPBLogQueue *_pbLog_queue_ptr;
};
}  // namespace pblog
