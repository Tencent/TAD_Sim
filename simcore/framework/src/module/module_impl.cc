// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "module_impl.h"

#include <iostream>

#include "utils/id_impl.h"
#include "utils/leb128.h"
#include "utils/os.h"
#include "utils/time.h"
#define TXSIM_NO_GLOG
#include "utils/zmq_routine.h"
#include "visibility.h"


#define TXSIM_INIT_KEY_SO_DIR "_cwd"

using namespace tx_sim::utils;


inline std::string GetPortOfTcpEndpoint(const std::string& endpoint) {
  return endpoint.substr(endpoint.find_last_of(':') + 1);
}


namespace tx_sim {
namespace impl {

ModuleServiceImpl::ModuleServiceImpl(void* init_cb, void* reset_cb, void* step_cb, void* stop_cb, uint32_t api_version,
                                     const std::string& so_dir) {
  if (TXSIM_MESSENGER_IMPL_API_VERSION < api_version) {
    std::string sError;
    sError += "tadsim version:";
    sError += std::to_string(TXSIM_MESSENGER_IMPL_API_VERSION);
    sError += ".\r\n";
    sError += "api version:";
    sError += std::to_string(api_version);
    sError += ".\r\n";
    sError += "Your code was compiled by a newer version of txSim headers. Please Update your TADSim software.";
    throw std::runtime_error(sError);
  }

  so_dir_ = so_dir;
  init_cb_ = reinterpret_cast<ModuleCallbackPtr>(init_cb);
  reset_cb_ = reinterpret_cast<ModuleCallbackPtr>(reset_cb);
  step_cb_ = reinterpret_cast<ModuleCallbackPtr>(step_cb);
  stop_cb_ = reinterpret_cast<ModuleCallbackPtr>(stop_cb);
  zmq_ctx_.reset(new zmq::context_t(1));

  uuid_ = GetEnvVar(kChildModuleEnvUuid);
  if (uuid_.empty()) uuid_ = GenerateRandomUUID();
}


ModuleServiceImpl::~ModuleServiceImpl() {
  if (run_serve_task_ && run_serve_task_->joinable()) run_serve_task_->join();
  if (socket_) socket_->close();
  if (inner_cmd_socket_) inner_cmd_socket_->close();
  if (zmq_ctx_) zmq_ctx_->close();
}


void ModuleServiceImpl::Serve(void* module, const std::string& mid, const std::string& service_addr) {
  module_ = module;
  module_id_ = mid;
  service_addr_ =
      "tcp://" + (service_addr.empty() ? std::string("127.0.0.1:") + kDefaultModuleRegistryPort : service_addr);

  try {
    while (1) {
      if (InitiateService()) break;
    }
    std::cout << "Module " << module_id_ << " connected to TAD Sim @ " << service_addr_ << std::endl;
  } catch (const zmq::error_t& e) {
    std::stringstream ss;
    ss << "Module Service " << module_id_ << " initiation failed: [" << e.num() << "] " << e.what();
    throw std::runtime_error(ss.str());
  }

  run_serve_task_.reset(new std::thread(&ModuleServiceImpl::RunServe, this));
  zmq::multipart_t msg;
  while (running_.load(std::memory_order_acquire)) {
    auto ret = DoZMQRecvRoutine(*inner_cmd_handle_socket_, msg, "inner cmd handling socket recv error");
    if (ret == kZmqOpAgain) continue;
    if (ret == kZmqOpError) {
      Shutdown();
      continue;
    }
    HandleCommand(msg);
    if (DoZMQSendRoutine(*inner_cmd_handle_socket_, msg, "inner cmd handling socket send error") == kZmqOpError)
      Shutdown();
  }

  std::cout << "Module " << module_id_ << " service exit." << std::endl;
}


void ModuleServiceImpl::Shutdown() {
  running_.store(false, std::memory_order_release);
}


void ModuleServiceImpl::RunServe() {
  zmq::multipart_t msg;
  zmq::pollitem_t poll_items[] = {{socket_->handle(), 0, ZMQ_POLLIN, 0},
                                  {inner_cmd_socket_->handle(), 0, ZMQ_POLLIN, 0}};
  long poll_timeout = heartbeat_interval_ > 0 ? heartbeat_interval_ : -1;
  inner_cmd_socket_->connect(kInnerModuleImplCmdForwardingEndpoint);

  while (running_.load(std::memory_order_acquire)) {
    zmq::poll(poll_items, 2, poll_timeout);

    if (poll_items[0].revents & ZMQ_POLLIN) {  // incoming msg from coordinator.
      if (DoZMQRecvRoutine(*socket_, msg, "module service recv error") == kZmqOpError) break;
      ModuleMessageType t = static_cast<ModuleMessageType>(PopMsgType(msg));
      switch (t) {
        case kModuleRegisterFailed: {
          std::cerr << "Module " << module_id_ << " serivce received close request. shutting down ..." << std::endl;
          Shutdown();
          break;
        }
        case kModuleHeartbeatDisconnecting: {
          SendModuleMessage(kModuleUnRegister, msg);
          if (DoZMQRecvRoutine(*socket_, msg, "module service unregister recv error") == kZmqOpSuccess) {
            bool unreg_ack_ok = (static_cast<ModuleMessageType>(PopMsgType(msg)) == kModuleRegisterSucceed);
            std::cout << "SimModule " << module_id_
                      << (unreg_ack_ok ? " un-registered itself from TAD Sim service."
                                       : " failed un-registering itself from TAD Sim service.");
          }
          Shutdown();
          break;
        }
        case kModuleRequest: {
          if (DoZMQSendRoutine(*inner_cmd_socket_, msg, "inner cmd forwarding socket send error") == kZmqOpError)
            Shutdown();
        }
      }
    }

    if (poll_items[1].revents & ZMQ_POLLIN) {
      if (DoZMQRecvRoutine(*inner_cmd_socket_, msg, "inner cmd forwarding socket recv error") == kZmqOpSuccess)
        SendModuleMessage(kModuleResponse, msg);
      else
        Shutdown();
    } else if (heartbeat_interval_ > 0) {  // sending heartbeat when idle.
      SendModuleMessage(kModuleHeartbeat, msg);
    }
  }

  std::cout << "Module " << module_id_ << " run serving finished." << std::endl;
}


bool ModuleServiceImpl::CheckCallbackCommand(ModuleRequestType req_typ, const ModuleData& data, zmq::multipart_t& msg) {
  if (req_typ == kModuleInit && initiated_) {  // "One Init" rule on module side.
    if (init_args_ != data.req<ModuleInitRequest>()->init_args) {
      ModuleInitResponse r;
      r.type = kModuleReInit;
      r.Encode(msg);
      return false;
    }
    init_result_.Encode(msg);
    return false;
  } else if (req_typ != kModuleInit && !initiated_) {  // ignoring other request if not initiated yet.
    data.Encode(msg);                                  // default kModuleOK.
    return false;
  }
  return true;
}


void ModuleServiceImpl::HandleCommand(zmq::multipart_t& msg) {
  ModuleData data;
  try {
    data.Decode(msg);
  } catch (const std::exception& e) {
    std::cerr << "module data decoding error: " << e.what() << std::endl;
    msg.clear();
    return;  // send back the empty message and cause coordinator to parse failed and get kModuleVersionIncompatible.
  }

  ModuleRequestType req_type = data.req_type();
  if (!CheckCallbackCommand(req_type, data, msg)) return;
  switch (req_type) {  // handling module callback requests ...
    case kModuleInit: {
      init_args_ = data.req<ModuleInitRequest>()->init_args;
      data.req<ModuleInitRequest>()->init_args[TXSIM_INIT_KEY_SO_DIR] = so_dir_;
      init_cb_(module_, static_cast<void*>(&data), data.cbs());
      initiated_ = true;
      init_result_ = *data.resp<ModuleInitResponse>();  // copy init result.
      break;
    }
    case kModuleReset: {
      reset_cb_(module_, static_cast<void*>(&data), data.cbs());
      break;
    }
    case kModuleStep: {
      data.set_publishes(init_result_.pub_topics);
      int64_t start_time = GetCpuTimeInMillis();
      uint64_t us1 = 0, sy1 = 0, us2 = 0, sy2 = 0;
      std::tie(us1, sy1) = GetProcessCpuTime();
      step_cb_(module_, static_cast<void*>(&data), data.cbs());
      std::tie(us2, sy2) = GetProcessCpuTime();
      data.resp<ModuleStepResponse>()->cpu_time_us = us2 - us1;
      data.resp<ModuleStepResponse>()->cpu_time_sy = sy2 - sy1;
      data.resp<ModuleStepResponse>()->time_cost = GetCpuTimeInMillis() - start_time;
      break;
    }
    case kModuleStop: {
      stop_cb_(module_, static_cast<void*>(&data), data.cbs());
      data.FreeResources();
      break;
    }
    default: break;  // already checked in ModuleData::Decode method.
  }

  data.Encode(msg);
}


bool ModuleServiceImpl::InitiateService() {
  socket_.reset(new zmq::socket_t(*zmq_ctx_, zmq::socket_type::dealer));
  socket_->set(zmq::sockopt::linger, 0);       // no wait at close time.
  socket_->set(zmq::sockopt::rcvtimeo, 3000);  // 3 seconds timeout to receive register ack.
  socket_->set(zmq::sockopt::routing_id, uuid_);
  if (!RunModuleRegisterTask()) return false;
  inner_cmd_handle_socket_.reset(new zmq::socket_t(*zmq_ctx_, zmq::socket_type::rep));
  inner_cmd_handle_socket_->set(zmq::sockopt::linger, 0);
  inner_cmd_handle_socket_->set(zmq::sockopt::rcvtimeo, 1000);
  inner_cmd_handle_socket_->bind(kInnerModuleImplCmdForwardingEndpoint);
  inner_cmd_socket_.reset(new zmq::socket_t(*zmq_ctx_, zmq::socket_type::req));
  inner_cmd_socket_->set(zmq::sockopt::linger, 0);
  return true;
}


bool ModuleServiceImpl::RunModuleRegisterTask() {
  zmq::multipart_t msg;
  bool reg_ack_ok;
  try {
    socket_->connect(service_addr_);
    msg.addstr(uuid_);  // just for back-compatibility. identical to the uuid(routing id).
    AddMsgUInt(msg_buf_, kModuleConnVersion, msg);
    SendModuleMessage(kModuleRegister, msg);
    reg_ack_ok = msg.recv(*socket_);
  } catch (const zmq::error_t& e) {
    socket_->close();
    std::ostringstream ss;
    ss << "Module Service register socket error: [" << e.num() << "] " << e.what();
    throw std::runtime_error(ss.str());
  }

  if (reg_ack_ok) {
    assert(msg.size() == 2);
    ModuleMessageType t = static_cast<ModuleMessageType>(PopMsgType(msg));
    if (t == kModuleConnVersionIncompatible) {
      const std::string codr_ver = PopMsgStr(msg), module_ver = GetSMCPVersionStr(kModuleConnVersion);
      std::cerr << "** SimModule " << module_id_ << " SMCP version " << module_ver
                << " incompatible with tadsim: " << codr_ver << "\n"
                << "** please update the software properly." << std::endl;
      // std::exit(3);
      return false;
    }
    reg_ack_ok = (t == kModuleRegisterSucceed);
    if (reg_ack_ok) {
      heartbeat_interval_ = PopMsgUint(msg);
      if (heartbeat_interval_ > 0) std::cout << "SimModule " << module_id_ << " heartbeating enabled." << std::endl;
    } else {
      std::cerr << "** SimModule " << module_id_ << " could not register itself to TAD Sim.\n"
                << "** please check the TAD Sim log.";
      // std::exit(2);
      return false;
    }
  } else {
    std::cerr << "** SimModule " << module_id_ << " could not register itself to TAD Sim.\n"
              << "** Make sure the TAD Sim service is running locally.\n"
              << "** Or provide the ip:port address if you are trying connecting to the remote TAD Sim service."
              << std::endl;
    // std::exit(1);
    return reg_ack_ok;
  }
  return reg_ack_ok;
}


// |----------------------|
// | msg_type             |
// |----------------------|
// | name                 |
// |----------------------|
// | uuid                 |
// |----------------------|
// | payload:             |
// | (register) |---------|
// |            | addr    |
// |            |---------|
// |            | version |
// |            |---------|
// | (response) |---------|
// |            | data    |
// |            |---------|
// |----------------------|
void ModuleServiceImpl::SendModuleMessage(tx_sim::impl::ModuleMessageType t, zmq::multipart_t& msg) {
  zmq::multipart_t header;
  AddMsgType(msg_buf_, t, header);
  header.addstr(module_id_);
  header.addstr(uuid_);
  msg.prepend(std::move(header));
  DoZMQSendRoutine(*socket_, msg, Enum2String(t) + ": module service sending error");
}

}  // namespace impl
}  // namespace tx_sim


extern "C" {

struct txsim_impl_error {
  std::string msg;
};

TXSIM_API const char* txsim_impl_error_message(void* err) {
  return reinterpret_cast<txsim_impl_error*>(err)->msg.c_str();
}

TXSIM_API void txsim_impl_error_destruct(void* err) {
  delete reinterpret_cast<txsim_impl_error*>(err);
}

TXSIM_API void* txsim_new_module_impl(void* init_cb, void* reset_cb, void* step_cb, void* stop_cb, uint32_t api_version,
                                      void** err) {
  try {
    return new tx_sim::impl::ModuleServiceImpl(init_cb, reset_cb, step_cb, stop_cb, api_version);
  } catch (const std::exception& e) { *err = new txsim_impl_error{e.what()}; } catch (...) {
    *err = new txsim_impl_error{"Unknown module-impl internal error"};
  }
  return nullptr;
}

TXSIM_API void txsim_serve(void* impl, void* module, const char* name, const char* addr, void** err) {
  try {
    reinterpret_cast<tx_sim::impl::ModuleServiceImpl*>(impl)->Serve(module, name, addr);
  } catch (const std::exception& e) { *err = new txsim_impl_error{e.what()}; } catch (...) {
    *err = new txsim_impl_error{"Unknown module-impl internal error"};
  }
}

TXSIM_API void txsim_shutdown(void* impl) {
  reinterpret_cast<tx_sim::impl::ModuleServiceImpl*>(impl)->Shutdown();
}

TXSIM_API void txsim_delete_module_impl(void* impl) {
  delete reinterpret_cast<tx_sim::impl::ModuleServiceImpl*>(impl);
}

}  // extern "C"
