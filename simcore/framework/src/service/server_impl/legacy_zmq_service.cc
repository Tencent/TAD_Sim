// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "legacy_zmq_service.h"

#include "utils/defs.h"
#include "utils/leb128.h"
#include "utils/os.h"
#include "utils/zmq_routine.h"


using namespace tx_sim::coordinator;
using namespace tx_sim::impl;
using namespace tx_sim::utils;


static void AddMsgID(zmq::multipart_t& msg, zmq::message_t& id) {
  zmq::message_t c_id;
  c_id.copy(id);
  msg.push(std::move(c_id));
}


const std::string LegacyZmqService::kServerBackendEndpoint = "inproc://txsim-service-backend";


LegacyZmqService::LegacyZmqService(std::shared_ptr<zmq::context_t>& zmq_ctx,
                                   std::shared_ptr<tx_sim::service::LocalServiceContext>& context,
                                   int worker_thread_num)
    : zmq_ctx_(zmq_ctx), context_(context) {
  frontend_.reset(new zmq::socket_t(*zmq_ctx_, ZMQ_ROUTER));
  backend_.reset(new zmq::socket_t(*zmq_ctx_, ZMQ_ROUTER));
  backend_->bind(kServerBackendEndpoint);

  for (int c = 0; c < worker_thread_num; ++c) {
    std::thread t(&LegacyZmqService::RunWorker, this);
    t.detach();
  }
  // receiving worker's ids.
  zmq::multipart_t msg;
  for (int c = 0; c < worker_thread_num; ++c) {
    msg.recv(*backend_);
    available_workers_.push(msg.pop());
  }

  LOG(INFO) << "legacy zmq service started with " << worker_thread_num << " worker thread.";
}


LegacyZmqService::~LegacyZmqService() {
  if (serving_thread_ && serving_thread_->joinable()) {
    serving_.store(false, std::memory_order_release);
    serving_thread_->join();
  }
}


void LegacyZmqService::Serve() {
  const std::string endpoint = GetLocalServiceBindAddress();
  if (DoZMQBindRoutine(*frontend_, endpoint, "legacy zmq service starting error") != kZmqOpSuccess) return;
  LOG(INFO) << "legacy zmq service listening on " << endpoint << " ...";
  serving_thread_.reset(new std::thread([this] {
    zmq::multipart_t msg;
    zmq::socket_t &bs = *backend_, &fs = *frontend_;
    zmq::pollitem_t items[] = {{backend_->handle(), 0, ZMQ_POLLIN, 0}, {frontend_->handle(), 0, ZMQ_POLLIN, 0}};
    try {
      while (serving_.load(std::memory_order_acquire)) {
        zmq::poll(items, 2, 1000);
        if (items[0].revents & ZMQ_POLLIN) {  // backend worker reply.
          msg.recv(bs);
          zmq::message_t wid = msg.pop();
          if (GetMsgType(msg[1]) != kCmdAccepted) available_workers_.push(std::move(wid));
          msg.send(fs);
        }
        if (items[1].revents & ZMQ_POLLIN) {  // incoming client request.
          msg.recv(fs);
          if (available_workers_.empty()) {  // reject the request since no available worker.
            zmq::message_t client_id(msg.pop());
            CommandStatus s(kCmdServerBusy);
            msg.clear();
            s.Encode(msg);
            msg.push(std::move(client_id));
            msg.send(fs);
          } else {  // dispatch the request to a vailable worker.
            zmq::message_t wid = std::move(available_workers_.front());
            available_workers_.pop();
            msg.push(std::move(wid));
            msg.send(bs);
          }
        }
      }
    } catch (...) { /* zmq::proxy always throw on EINTER/ETERM since zmq_proxy always return -1 on interrupt. */
    }
    LOG(INFO) << "legacy zmq service serving finished.";
  }));
  return;
}


void LegacyZmqService::RunWorker() {
  BlockSignals();  // block interrupt signals so that only main thread(LegacyZmqService::Serve) catches.
  SetThreadName("txsim-zmq-worker");

  zmq::socket_t sock(*zmq_ctx_, ZMQ_DEALER);
  sock.connect(kServerBackendEndpoint);

  zmq::multipart_t msg;
  msg.addstr("ready");
  msg.send(sock);  // sending first ready msg to backend router.

  while (serving_.load(std::memory_order_acquire)) {
    if (DoZMQRecvRoutine(sock, msg, "") == kZmqOpError) return;  // only ETERM when interrupt.

    zmq::message_t client_id(msg.pop());
    Command cmd = static_cast<Command>(PopMsgType(msg));
    if (cmd < kCmdMaxPlayerType) {  // player command type which returns command status stream.
      DispatchPlayerCmd(cmd, msg, client_id, sock);
    } else {  // manager command type which returns only a error code to client.
      DispatchManagerCmd(cmd, msg, client_id, sock);
    }
  }

  LOG(INFO) << "legacy zmq service worker returned.";
}


void LegacyZmqService::DispatchPlayerCmd(Command cmd, zmq::multipart_t& msg, zmq::message_t& id, zmq::socket_t& sock) {
  CommandInfo cmd_info(cmd);
  // assembling cmd_info ...
  cmd_info.status_cb = [&id, &sock](const CommandStatus& status) {
    zmq::multipart_t m;
    status.Encode(m);
    AddMsgID(m, id);
    DoZMQSendRoutine(sock, m, "legacy service worker socket send error");
  };
  if (cmd == kCmdSetup) {
    std::string scenario_path = PopMsgStr(msg);
    CommandStatus failed_st(kCmdFailed);
    try {
      scenario_path = context_->core().snpr->CheckIsOpenScenario(scenario_path);
      cmd_info.reset_params.scenario_path = scenario_path;
    } catch (const std::exception& e) {
      LOG(ERROR) << "excuting cmd " << Enum2String(cmd) << " error: " << e.what();
      cmd_info.status_cb(failed_st);
      return;
    }
    if (!scenario_path.empty()) {  // log2world supported by .sim only currently.
      try {
        cmd_info.l2w_config = context_->GetScenarioLog2WorldConfig(scenario_path);
        context_->core().snpr->ParseLogsimEvents(scenario_path, cmd_info.logsim_events);
      } catch (const std::exception& e) {
        LOG(ERROR) << "legacy_zmq_service executing " << Enum2String(cmd)
                   << " parsing log2world config error: " << e.what();
        cmd_info.status_cb(failed_st);
        return;
      }
    }
  }
  if (cmd == kCmdRun || cmd == kCmdStep) {
    cmd_info.l2w_config = context_->GetScenarioLog2WorldConfig(context_->core().codr->current_scenario());
  }
  if (cmd == kCmdHighlightGroup) {
    std::string highlight = PopMsgStr(msg);
    cmd_info.highlight_group = highlight;
  }
  // executing command with cmd_info ...
  context_->core().codr->Execute(cmd_info);
}


void LegacyZmqService::DispatchManagerCmd(Command cmd, zmq::multipart_t& msg, zmq::message_t& id, zmq::socket_t& sock) {
  uint8_t buf[10];
  CmdErrorCode ec = kCmdSucceed;
  zmq::multipart_t ret_msg;
  try {
    std::lock_guard<std::mutex> lk(manager_op_mtx_);
    ec = context_->core().cfgr->Execute(cmd, msg, ret_msg);
  } catch (const std::exception& e) {
    LOG(ERROR) << "local service handle manager command(" << cmd << ") error: " << e.what();
    ec = kCmdSystemError;
  }
  AddMsgType(buf, ec, ret_msg, true);
  AddMsgID(ret_msg, id);
  DoZMQSendRoutine(sock, ret_msg, "legacy zmq service worker socket send error");
}
