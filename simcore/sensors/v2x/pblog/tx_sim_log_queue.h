/**
 * @file txSimLogQueue.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once
#include "glog/logging.h"
#include "tx_sim_log_common.h"

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#undef max
#undef min

enum PBLogQueueDir { PBLogQueue_Write, PBLogQueue_Read };

class CPBLogQueue {
 private:
  uint32_t _max_logger_queue_size;              // max queue size for producer
  std::queue<pblog::PBLogEvent> _logger_queue;  // queue for producer
  std::mutex _logger_queue_lock;                // lock for producer queue
  pblog::PBLogOutStream _ptr_outstream;         // output stream
  uint32_t _wt_counter;

  uint32_t _max_player_queue_size;              // max player queu size
  std::queue<pblog::PBLogEvent> _player_queue;  // queue for log player
  std::mutex _player_queue_lock;                // lock for consumer queue
  pblog::PBLogInStream _ptr_instream;           // input stream

  bool _stop_thread;  // if stop streaming pb log event
  std::mutex _stop_thread_lock;
  std::thread _thread_handle;  // thread streaming pb log event
  bool m_bjson = true;

 private:
  // no copy construct
  CPBLogQueue(const CPBLogQueue &) = delete;
  CPBLogQueue &operator=(const CPBLogQueue &) = delete;

  // no lock
  bool IsQueueFull(const std::queue<pblog::PBLogEvent> &pblog_queue, const uint32_t &max_size);
  bool IsQueueEmpty(const std::queue<pblog::PBLogEvent> &pblog_queue);
  void ClearQueue(std::queue<pblog::PBLogEvent> &pblog_queue);

  // thread func to stream pb log into file
  void ConsumePBLogEventFunc();
  // common use
 public:
  explicit CPBLogQueue(const uint32_t &max_queue_size = 1024);
  void Clear(const PBLogQueueDir &dir = PBLogQueueDir::PBLogQueue_Write);
  void Initialize(const std::string &pblog_file, const PBLogQueueDir &dir = PBLogQueueDir::PBLogQueue_Write,
                  bool bjson = true);

 public:
  // for protobuf logger
  void ProduceLogEvent(const pblog::PBLogEvent &event);

  // for protobuf player
  // thread function to read pb log events from file
  void ReadPbLogEventsFunc();

  // consume one pb log event
  pblog::PBLogEvent GetOnePBLogEvent();
  void ConsumeOnePBLogEvent();
};
