// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <memory>

#include "boost/asio.hpp"
#include "boost/beast/core.hpp"
#include "boost/beast/http.hpp"

#include "service/local_status_writer.h"
#include "service_context.h"

namespace tx_sim {
namespace service {

class HttpSession final : public std::enable_shared_from_this<HttpSession> {
 public:
  HttpSession(size_t session_id, boost::asio::ip::tcp::socket&& socket, const std::shared_ptr<LocalServiceContext>& ctx,
              std::shared_ptr<tx_sim::service::SimLocalStatusWriter>& writer);

  ~HttpSession();

  void Start();

 private:
  using request_body_t = boost::beast::http::string_body;
  using response_body_t = boost::beast::http::string_body;

  void DoRead();
  void OnRead(boost::beast::error_code ec, std::size_t bytes_read);

  void HandleConfigRequest(const boost::beast::http::request<request_body_t>&& req, const std::string& target);
  void HandlePlayRequest(const boost::beast::http::request<request_body_t>&& req, const std::string& target);
  void HandleDataRequest(const boost::beast::http::request<request_body_t>&& req, const std::string& target);

  void SendBadReqResponse(const std::string& target, boost::beast::http::verb method, const std::string& err);
  void SendOKResponse(const std::string& body, bool keep_alive);
  void SendResponse(boost::beast::http::status status, const std::string& content_type, const std::string& body,
                    bool keep_alive);

  void DoWrite();
  void OnWrite(bool close, boost::beast::error_code ec, std::size_t bytes_written);

  void Close();

 private:
  static const size_t kMaxHttpRequestBodySizeInBytes, kGenericRequestProcessTimeoutInSeconds;
  const size_t sid_;
  const std::string session_id_;
  boost::beast::tcp_stream stream_;
  boost::beast::flat_buffer buffer_;
  boost::optional<boost::beast::http::request_parser<request_body_t>> parser_;
  boost::optional<boost::beast::http::response<response_body_t>> response_;
  std::shared_ptr<LocalServiceContext> context_;
  std::shared_ptr<SimLocalStatusWriter> writer_;
};

}  // namespace service
}  // namespace tx_sim
