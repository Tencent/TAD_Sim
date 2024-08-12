// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "connection.h"

#include <cassert>

#include "boost/uuid/nil_generator.hpp"
#include "glog/logging.h"

#include "utils/constant.h"
#include "utils/id_impl.h"
#include "utils/leb128.h"
#include "utils/time.h"
#include "utils/zmq_routine.h"

using namespace tx_sim::utils;
using namespace tx_sim::impl;

namespace tx_sim {
namespace coordinator {

//! @brief 函数名：DecodeResponse
//! @details 函数功能：回复数据解码
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
static void DecodeResponse(ModuleResponse& resp, zmq::multipart_t& msg) {
  try {
    resp.Decode(msg);
  } catch (const std::exception& e) {
    LOG(ERROR) << "decoding response failed: " << e.what();
    resp.type = kModuleVersionIncompatible;
  }
}

//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
ZmqConnection::ZmqConnection(const std::shared_ptr<zmq::context_t>& ctx, const std::string& addr,
                             const ConnectionConfig& config)
    : zmq_ctx_(ctx), addr_(addr), config_(config) {
  sock_.reset(new zmq::socket_t(*zmq_ctx_, zmq::socket_type::req));
  sock_->set(zmq::sockopt::linger, 0);
  sock_->set(zmq::sockopt::routing_id, addr);
  sock_->connect(kInnerModuleForwardingEndpoint);
}

//! @brief 函数名：
//! @details 函数功能：发送方法
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
bool ZmqConnection::Send(ModuleRequest& request) {
  zmq::multipart_t msg;
  request.Encode(msg);
  // usually Reset takes much longer time.
  timeout_point_ = GetCpuTimeInMillis() + (request.type == kModuleStep ? config_.step_timeout : config_.cmd_timeout);
  if (DoZMQSendRoutine(*sock_, msg, "zmq connection sending error") == kZmqOpSuccess) {
    waiting_reply_ = true;
    return true;
  }
  return false;
}

//! @brief 函数名：
//! @details 函数功能：zmq接收方法
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
bool ZmqConnection::Receive(ModuleResponse& response, uint32_t waiting_time) {
  assert(timeout_point_ > 0);
  zmq::multipart_t msg;
  if (ReceiveMessageBlocking(msg, waiting_time)) {
    waiting_reply_ = false;
    DecodeResponse(response, msg);
  } else {
    if (config_.step_timeout == 0 || GetCpuTimeInMillis() < timeout_point_) return false;
    response.type = kModuleTimeout;
  }
  timeout_point_ = 0;
  return true;
}

//! @brief 函数名：
//! @details 函数功能：判断是否在接受
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
bool ZmqConnection::WaitingReply() {
  return waiting_reply_;
}

//! @brief 函数名：
//! @details 函数功能：收数据时等待
//!
//! @param[in] blocking_time: 等待时间
//! @param[out]
//!
//! @return
//!
//! @note
bool ZmqConnection::ReceiveMessageBlocking(zmq::multipart_t& msg, uint32_t blocking_time) {
  zmq::pollitem_t item{sock_->handle(), 0, ZMQ_POLLIN, 0};
  zmq::poll(&item, 1, blocking_time);
  if (item.revents & ZMQ_POLLIN) {  // response received.
    ZmqOpRet ret = DoZMQRecvRoutine(*sock_, msg, "zmq connection receiving error");
    assert(ret == kZmqOpSuccess);
    return true;
  }
  return false;
}

ZmqConnectionFactory::ZmqConnectionFactory(const std::shared_ptr<zmq::context_t>& ctx) : zmq_ctx_(ctx) {}

std::shared_ptr<ModuleConnection> ZmqConnectionFactory::CreateConnection(const std::string& endpoint,
                                                                         const ConnectionConfig& config) {
  return std::make_shared<ZmqConnection>(zmq_ctx_, endpoint, config);
}

}  // namespace coordinator
}  // namespace tx_sim
