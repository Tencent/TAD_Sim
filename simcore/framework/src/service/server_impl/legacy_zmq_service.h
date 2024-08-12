// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include "zmq_addon.hpp"

#include "coordinator/config_mgr.h"
#include "coordinator/coordinator.h"
#include "service_context.h"

class LegacyZmqService final {
 public:
  LegacyZmqService(std::shared_ptr<zmq::context_t>& zmq_ctx,
                   std::shared_ptr<tx_sim::service::LocalServiceContext>& context, int worker_thread_num = 3);
  ~LegacyZmqService();

  void Serve();

 private:
  void RunWorker();
  void DispatchPlayerCmd(tx_sim::impl::Command cmd, zmq::multipart_t& msg, zmq::message_t& id, zmq::socket_t& sock);
  void DispatchManagerCmd(tx_sim::impl::Command cmd, zmq::multipart_t& msg, zmq::message_t& id, zmq::socket_t& sock);

 private:
  static const std::string kServerBackendEndpoint;
  std::atomic_bool serving_{true};
  std::unique_ptr<zmq::socket_t> frontend_, backend_;
  std::queue<zmq::message_t> available_workers_;
  std::mutex manager_op_mtx_;
  std::unique_ptr<std::thread> serving_thread_;
  std::shared_ptr<zmq::context_t> zmq_ctx_;
  std::shared_ptr<tx_sim::service::LocalServiceContext> context_;
};
