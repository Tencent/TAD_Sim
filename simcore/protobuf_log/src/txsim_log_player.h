// Copyright 2024 Tencent Inc. All rights reserved.
/**
 * @brief
 *
 */

#pragma once

#include "txsim_log_common.h"
#include "txsim_module.h"

#ifdef _WIN32
#  define GLOG_NO_ABBREVIATED_SEVERITIES
#endif

#include "glog/logging.h"

#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

#include "txsim_log_queue.h"

/**
 * @brief Log player, inherits from tx_sim::SimModule
 */
class txSimLogplayer final : public tx_sim::SimModule {
 public:
  txSimLogplayer();
  virtual ~txSimLogplayer();

  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 public:
  // read pb log event that should be published in this step
  // void readPBLogEventsOneStep(const int64_t &simTimestamp);

  // check if the topic in the topic list
  bool IsPlaybackChannel(const std::string &topic);

  // clear _pbLog_queue_ptr
  void ClearPbLogQueuePtr();

  // Output topic list according to original topic name. For some ego topic name, need to publish extra topic
  void PreProcessTopic(const std::string &original_topic, std::vector<std::string> &all_topics);

  // Read Ego Group Info in log file
  void ReadEgoGroupInfo();

  // Read Topic List in log file
  void ReadTopicList();

 private:
  std::string name;
  std::string _payload;

  std::string _log_file_path;

  bool _first_frame = true;
  int64_t _first_log_t = 0;  // first pblog event timestamp, microsecond
  int64_t _first_sim_t = 0;  // first simulation timestamp, microsecond
  std::vector<std::string> _playback_channel_list;
  std::vector<std::string> _ego_group_list;
  std::vector<std::string> _channel_list_in_file;

  // pb log queue
  CPBLogQueue *_pblog_queue_ptr;
};
