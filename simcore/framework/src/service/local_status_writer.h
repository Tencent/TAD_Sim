// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "utils/ds/spsc_queue/readerwritercircularbuffer.h"

#include "coordinator/status_writer.h"

namespace tx_sim {
namespace service {

class WsSession;

/**
 * @brief a local status writer used by http server in local service.
 */
class SimLocalStatusWriter final : public tx_sim::coordinator::SimStatusWriter {
 public:
  /**
   * @brief Constructor.
   */
  SimLocalStatusWriter();
  /**
   * @brief destructor.
   */
  ~SimLocalStatusWriter();

  /**
   * @brief enqueues the status message into the queue.
   * @param[in] st The status message to be enqueued.
   */
  void Write(const tx_sim::coordinator::CommandStatus& st) override;

  /**
   * @brief joins the status writing session to the map.
   * @param[in] s The WsSession pointer that instance that the local status writing session.
   * @param[in] push_raw_msg If true, push the raw status message.
   */
  void Join(WsSession* s, bool push_raw_msg);
  /**
   * @brief erases the specified session.
   * @param[in] s The WsSession pointer that will leave session map.
   */
  void Leave(WsSession* s);

 private:
  /**
   * @brief the thread that consume the status messages.
   */
  void Consume();

 private:
  // an atomic boolean to indicate if the consumer thread is running
  std::atomic_bool running_{true};
  // pointer to the consumer thread
  std::unique_ptr<std::thread> consumer_;
  // blocking reader-writer circular buffer
  moodycamel::BlockingReaderWriterCircularBuffer<tx_sim::coordinator::CommandStatus> queue_{10};
  // a map to store the websocket sessions and their corresponding flags
  std::unordered_map<WsSession*, bool> sessions_;
  // mutex for synchronizing access to the session map
  std::mutex mtx_;
};

}  // namespace service
}  // namespace tx_sim
