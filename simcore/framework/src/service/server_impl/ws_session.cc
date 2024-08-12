// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "ws_session.h"

#include "glog/logging.h"


namespace basio = boost::asio;
namespace beast = boost::beast;
namespace bhttp = beast::http;
namespace bws = beast::websocket;
using btcp = boost::asio::ip::tcp;
using namespace tx_sim::coordinator;
using namespace tx_sim::impl;
using namespace tx_sim::utils;


namespace tx_sim {
namespace service {

WsSession::WsSession(boost::asio::ip::tcp::socket&& socket, std::shared_ptr<tx_sim::coordinator::Coordinator>& cp,
                     size_t session_id, std::shared_ptr<tx_sim::service::SimLocalStatusWriter>& sim_writer,
                     bool sub_raw_msg)
    : ws_(std::move(socket)),
      codr_(cp),
      session_id_(std::string("[ws-") + std::to_string(session_id) + "]"),
      writer_(sim_writer),
      push_raw_msg_(sub_raw_msg) {
  VLOG(3) << session_id_ << " new websocket session created.";
}


WsSession::~WsSession() {
  if (writer_) writer_->Leave(this);
  VLOG(3) << session_id_ << " websocket session destroyed.";
}


void WsSession::Start(boost::beast::http::request<boost::beast::http::string_body> ws_req) {
  ws_.set_option(bws::stream_base::timeout::suggested(beast::role_type::server));
  ws_.set_option(
      bws::stream_base::decorator([](bws::response_type& resp) { resp.set(bhttp::field::server, "txSim-data"); }));
  ws_.async_accept(ws_req, beast::bind_front_handler(&WsSession::OnAccept, shared_from_this()));
}


void WsSession::SendStatus(const std::shared_ptr<const std::string>& payload) {
  basio::post(ws_.get_executor(), beast::bind_front_handler(&WsSession::OnStatusSend, shared_from_this(), payload));
}


void WsSession::SendMsgs(const std::shared_ptr<const std::vector<uint8_t>>& payload) {
  basio::post(ws_.get_executor(), beast::bind_front_handler(&WsSession::OnMsgsSend, shared_from_this(), payload));
}


void WsSession::OnAccept(boost::beast::error_code ec) {
  if (ec) {
    LOG(ERROR) << session_id_ << " accept error: " << ec.message();
    return;
  }

  if (push_raw_msg_) ws_.binary(true);
  if (writer_) writer_->Join(this, push_raw_msg_);

  // expecting the EOF from the client.
  ws_.async_read(buf_, beast::bind_front_handler(&WsSession::OnRead, shared_from_this()));
}


void WsSession::OnRead(boost::beast::error_code ec, size_t bytes_read) {
  if (ec) {
    if (ec == bws::error::closed)
      VLOG(3) << session_id_ << " the websocket stream was gracefully closed at both endpoints.";
    else
      LOG(ERROR) << session_id_ << " read error[" << ec.value() << "]: " << ec.message();
    if (writer_) writer_->Leave(this);
    return;
  }

  // TODO(nemo): un-expected this?
  VLOG(2) << session_id_ << " read msg with " << bytes_read << " bytes.";
  buf_.consume(buf_.size());
  ws_.async_read(buf_, beast::bind_front_handler(&WsSession::OnRead, shared_from_this()));
}


void WsSession::OnStatusSend(const std::shared_ptr<const std::string>& payload) {
  status_queue_.push_back(payload);
  if (status_queue_.size() > 1) return;
  ws_.async_write(basio::buffer(*status_queue_.front()),
                  beast::bind_front_handler(&WsSession::OnWrite, shared_from_this()));
}


void WsSession::OnMsgsSend(const std::shared_ptr<const std::vector<uint8_t>>& payload) {
  msg_queue_.push_back(payload);
  if (msg_queue_.size() > 1) return;
  ws_.async_write(basio::buffer(*msg_queue_.front()),
                  beast::bind_front_handler(&WsSession::OnWrite, shared_from_this()));
}


void WsSession::OnWrite(boost::beast::error_code ec, size_t bytes_written) {
  if (ec) {
    LOG(ERROR) << session_id_ << " write error: " << ec.message();
    return;
  }

  if (push_raw_msg_) {
    msg_queue_.erase(msg_queue_.begin());
    if (!msg_queue_.empty())
      ws_.async_write(basio::buffer(*msg_queue_.front()),
                      beast::bind_front_handler(&WsSession::OnWrite, shared_from_this()));
  } else {
    status_queue_.erase(status_queue_.begin());
    if (!status_queue_.empty())
      ws_.async_write(basio::buffer(*status_queue_.front()),
                      beast::bind_front_handler(&WsSession::OnWrite, shared_from_this()));
  }
}

}  // namespace service
}  // namespace tx_sim
