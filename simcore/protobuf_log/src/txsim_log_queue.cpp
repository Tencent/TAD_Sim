// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "txsim_log_queue.h"

CPBLogQueue::CPBLogQueue(const uint32_t &max_queue_size) {
  // set max queue size
  _max_logger_queue_size = max_queue_size;
  // _max_consumer_queue_size=10*max_queue_size;
  _max_player_queue_size = max_queue_size;
}

void CPBLogQueue::ProduceLogEvent(const pblog::PBLogEvent &event) {
  _logger_queue_lock.lock();
  // sync producer and consumer when producer queue is full
  if (IsQueueFull(_logger_queue, _max_logger_queue_size)) {
    _logger_queue.pop();
    LOG(ERROR) << "logger | message queue is full, consumer is too slow, "
                  "remove the oldest message, message lost!\n";
  }
  _logger_queue.push(event);
  _logger_queue_lock.unlock();
}

bool CPBLogQueue::IsQueueFull(const std::queue<pblog::PBLogEvent> &pblog_queue, const uint32_t &max_size) {
  return pblog_queue.size() >= max_size ? true : false;
}

bool CPBLogQueue::IsQueueEmpty(const std::queue<pblog::PBLogEvent> &pblog_queue) {
  return pblog_queue.size() == 0 ? true : false;
}

void CPBLogQueue::ClearQueue(std::queue<pblog::PBLogEvent> &pblog_queue) {
  while (!pblog_queue.empty()) {
    pblog_queue.pop();
  }
}

void CPBLogQueue::Initialize(const std::string &pblog_file, const PBLogQueueDir &dir) {
  // clear queue
  ClearQueue(_logger_queue);
  ClearQueue(_player_queue);

  // if quit streaming thread
  _stop_thread = false;
  _wt_counter = 0;

  // streaming pblog to file
  if (dir == PBLogQueueDir::PBLogQueue_Write) {
    // create output stream
    if (pblog::CPBLogCommon::createOutStream(_ptr_outstream, pblog_file)) {
      LOG(INFO) << "logger | protobuf log file opened in w mode. " << pblog_file << "\n";

      // create a thread to stream pb log
      _thread_handle = std::thread(&CPBLogQueue::ConsumePBLogEventFunc, this);

      if (_thread_handle.joinable()) {
        LOG(INFO) << "logger | pb log streaming thread created.\n";
      } else {
        LOG(INFO) << "logger | fail to create pb log streaming thread.\n";
      }
    } else {
      LOG(ERROR) << "logger | protobuf log file fail to open in w mode. " << pblog_file << "\n";
      pblog::CPBLogCommon::CloseOutStream(_ptr_outstream);
    }
  } else if (dir == PBLogQueueDir::PBLogQueue_Read) {
    // read pblog from stream
    // create input stream
    if (pblog::CPBLogCommon::CreateInStream(_ptr_instream, pblog_file)) {
      LOG(INFO) << "player | protobuf log file opened in r mode. " << pblog_file << "\n";

      // create a thread to read pb log
      _thread_handle = std::thread(&CPBLogQueue::ReadPbLogEventsFunc, this);

      if (_thread_handle.joinable()) {
        LOG(INFO) << "player | pb log read thread created.\n";
      } else {
        LOG(INFO) << "player | fail to create pb log read thread.\n";
      }
    } else {
      LOG(ERROR) << "player | protobuf log file fail to open in r mode. " << pblog_file << "\n";
      pblog::CPBLogCommon::CloseInStream(_ptr_instream);
    }
  }
}

// thread func to stream pb log into file
void CPBLogQueue::ConsumePBLogEventFunc() {
  bool ifStopStreaming = false;

  // streaming
  while (!ifStopStreaming) {
    // streaming pb log
    _logger_queue_lock.lock();
    if (!IsQueueEmpty(_logger_queue)) {
      pblog::PBLogEvent event = _logger_queue.front();
      _logger_queue.pop();
      // release lock
      _logger_queue_lock.unlock();

      // stream pb log event
      if (pblog::CPBLogCommon::IsOutStreamValid(_ptr_outstream)) {
        pblog::CPBLogCommon::WriteEvent2Stream(event, _ptr_outstream);
        if (1024 <= _wt_counter++) {
          _ptr_outstream.flush();
          _wt_counter = 0;
        }
      } else {
        LOG(ERROR) << "logger | pb log stream is not valid.\n";
      }
    } else {
      // release lock, so that this thread can sleep without blocking producer
      _logger_queue_lock.unlock();

      // sleep 1 millisecond
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // check if stop streaming
    _stop_thread_lock.lock();
    ifStopStreaming = _stop_thread;
    _stop_thread_lock.unlock();
  }

  // streaming pb log event left in consumer queue
  _logger_queue_lock.lock();
  while (!IsQueueEmpty(_logger_queue)) {
    pblog::PBLogEvent event = _logger_queue.front();
    _logger_queue.pop();
    pblog::CPBLogCommon::WriteEvent2Stream(event, _ptr_outstream);
  }
  _logger_queue_lock.unlock();
}

void CPBLogQueue::Clear(const PBLogQueueDir &dir) {
  _stop_thread_lock.lock();
  _stop_thread = true;
  _stop_thread_lock.unlock();

  if (_thread_handle.joinable()) {
    _thread_handle.join();
    LOG(INFO) << "logger | pb log streaming thread quit.\n";
  }
  if (dir == PBLogQueueDir::PBLogQueue_Write)
    pblog::CPBLogCommon::CloseOutStream(_ptr_outstream);
  else if (dir == PBLogQueueDir::PBLogQueue_Read)
    pblog::CPBLogCommon::CloseInStream(_ptr_instream);

  LOG(INFO) << "queue | pb log queue object stoppped.\n";
}

// thread function to read pb log events from file
void CPBLogQueue::ReadPbLogEventsFunc() {
  bool ifStopThread = false;

  while (!ifStopThread) {
    _player_queue_lock.lock();
    if (IsQueueFull(_player_queue, _max_player_queue_size)) {
      _player_queue_lock.unlock();
      // LOG(INFO)<<"player | player queue is full, read thread sleep for 3
      // millisecond.\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(3));
    } else {
      try {
        pblog::PBLogEvent event;

        if (pblog::CPBLogCommon::ReadEventFromStream(event, _ptr_instream)) {
          _player_queue.push(event);
        } else {
          ifStopThread = true;
        }
      } catch (std::exception &e) {
        LOG(INFO) << "player | exception " << e.what() << "\n";
        ifStopThread = true;
      }

      _player_queue_lock.unlock();
    }

    // quit if reach end of file
    _stop_thread_lock.lock();
    ifStopThread = _stop_thread;
    _stop_thread_lock.unlock();
  }

  LOG(INFO) << "player | end of replay, read thread quit.\n";
}

// consume one pb log event
pblog::PBLogEvent CPBLogQueue::GetOnePBLogEvent() {
  pblog::PBLogEvent event = pblog::CPBLogCommon::GetInitLogEvent();

  // get the front event from queue
  _player_queue_lock.lock();
  if (!_player_queue.empty()) {
    event = _player_queue.front();
  }
  _player_queue_lock.unlock();

  return event;
}

void CPBLogQueue::ConsumeOnePBLogEvent() {
  // pop one event from queue
  _player_queue_lock.lock();
  if (!_player_queue.empty()) {
    _player_queue.pop();
  }
  _player_queue_lock.unlock();
}
