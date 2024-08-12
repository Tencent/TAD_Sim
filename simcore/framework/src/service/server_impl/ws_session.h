// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <memory>

#include "boost/asio.hpp"
#include "boost/beast/core.hpp"
#include "boost/beast/http.hpp"
#include "boost/beast/websocket.hpp"
#include "boost/smart_ptr.hpp"

#include "coordinator/coordinator.h"
#include "service/local_status_writer.h"

namespace tx_sim {
namespace service {

// using boost::enable_shared_from_this instead of the std's since c++11 has no member of weak_from_this(until c++17).
class WsSession final : public boost::enable_shared_from_this<WsSession> {
 public:
  WsSession(boost::asio::ip::tcp::socket&& socket, std::shared_ptr<tx_sim::coordinator::Coordinator>& cp,
            size_t session_id, std::shared_ptr<SimLocalStatusWriter>& sim_writer, bool sub_raw_msg = false);
  ~WsSession();

  void Start(boost::beast::http::request<boost::beast::http::string_body> ws_req);
  void SendStatus(const std::shared_ptr<const std::string>& payload);
  void SendMsgs(const std::shared_ptr<const std::vector<uint8_t>>& payload);

 private:
  void OnAccept(boost::beast::error_code ec);
  void OnRead(boost::beast::error_code ec, size_t bytes_read);
  void OnStatusSend(const std::shared_ptr<const std::string>& payload);
  void OnMsgsSend(const std::shared_ptr<const std::vector<uint8_t>>& payload);
  void OnWrite(boost::beast::error_code ec, size_t bytes_written);

 private:
  const std::string session_id_;
  const bool push_raw_msg_;
  std::shared_ptr<SimLocalStatusWriter> writer_;
  std::shared_ptr<tx_sim::coordinator::Coordinator> codr_;
  boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;
  boost::beast::flat_buffer buf_;
  std::vector<std::shared_ptr<const std::string>> status_queue_;
  std::vector<std::shared_ptr<const std::vector<uint8_t>>> msg_queue_;
};

}  // namespace service
}  // namespace tx_sim
