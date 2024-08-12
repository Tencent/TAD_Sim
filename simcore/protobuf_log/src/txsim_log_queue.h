// Copyright 2024 Tencent Inc. All rights reserved.
/**
 * @brief protobuf payload event
 *
 */
#ifndef SIMCORE_PROTOBUF_LOG_TXSIMLOGQUEUE_H_
#define SIMCORE_PROTOBUF_LOG_TXSIMLOGQUEUE_H_

#include "glog/logging.h"
#include "txsim_log_common.h"

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

// define Log‘s using
enum PBLogQueueDir { PBLogQueue_Write, PBLogQueue_Read };

/**
 * @brief Log queue for logger and log player
 */
class CPBLogQueue {
 public:
  explicit CPBLogQueue(const uint32_t &max_queue_size = 1024);
  void Clear(const PBLogQueueDir &dir = PBLogQueueDir::PBLogQueue_Write);
  void Initialize(const std::string &pblog_file, const PBLogQueueDir &dir = PBLogQueueDir::PBLogQueue_Write);

 public:
  // for protobuf logger
  void ProduceLogEvent(const pblog::PBLogEvent &event);

  // for protobuf player
  // thread function to read pb log events from file
  void ReadPbLogEventsFunc();

  // get one pb log event
  pblog::PBLogEvent GetOnePBLogEvent();
  // consume one pb log event
  void ConsumeOnePBLogEvent();

 private:
  // no copy construct
  CPBLogQueue(const CPBLogQueue &) = delete;
  CPBLogQueue &operator=(const CPBLogQueue &) = delete;

  // no lock
  // check if queue is full
  bool IsQueueFull(const std::queue<pblog::PBLogEvent> &pblog_queue, const uint32_t &max_size);
  // check if queue is empty
  bool IsQueueEmpty(const std::queue<pblog::PBLogEvent> &pblog_queue);
  // clear queue
  void ClearQueue(std::queue<pblog::PBLogEvent> &pblog_queue);

  // thread func to stream pb log into file
  void ConsumePBLogEventFunc();

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
};

#endif  // SIMCORE_PROTOBUF_LOG_TXSIMLOGQUEUE_H_
