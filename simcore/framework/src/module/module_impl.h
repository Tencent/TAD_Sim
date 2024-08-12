// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <atomic>
#include <functional>
#include <thread>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "messenger_impl.h"
#include "utils/constant.h"
#include "utils/msgs.h"

namespace tx_sim {
namespace impl {

typedef void (*ModuleCallbackPtr)(void*, void*, void**);

class ModuleServiceImpl final {
 public:
  ModuleServiceImpl(void* init_cb, void* reset_cb, void* step_cb, void* stop_cb, uint32_t api_version,
                    const std::string& so_dir = "");
  ~ModuleServiceImpl();

  void Serve(void* module, const std::string& mid, const std::string& service_addr = "");
  void Shutdown();

 private:
  typedef std::function<void()> CmdOp;

  void RunServe();
  bool CheckCallbackCommand(ModuleRequestType req_typ, const ModuleData& data, zmq::multipart_t& msg);
  void HandleCommand(zmq::multipart_t& msg);
  void DoCommandRoutine(ModuleData& data, ModuleCallbackPtr cb);
  bool InitiateService();
  bool RunModuleRegisterTask();
  void Heartbeat();
  void SendModuleMessage(tx_sim::impl::ModuleMessageType t, zmq::multipart_t& msg);

  bool initiated_{false};
  uint8_t msg_buf_[10];
  uint32_t heartbeat_interval_ = 0;  // default 0 means no heartbeating. unit: second
  std::unordered_map<std::string, std::string> init_args_;
  ModuleInitResponse init_result_;
  std::string module_id_, service_addr_, module_addr_, uuid_, so_dir_;
  std::unique_ptr<zmq::socket_t> socket_ = nullptr, inner_cmd_socket_ = nullptr, inner_cmd_handle_socket_ = nullptr;
  std::unique_ptr<zmq::context_t> zmq_ctx_ = nullptr;
  void* module_ = nullptr;
  std::atomic_bool running_{true};
  std::unique_ptr<std::thread> run_serve_task_ = nullptr;
  ModuleCallbackPtr init_cb_, reset_cb_, step_cb_, stop_cb_;
};

}  // namespace impl
}  // namespace tx_sim
