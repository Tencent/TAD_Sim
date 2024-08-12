// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;    // from <boost/beast.hpp>
namespace http = beast::http;      // from <boost/beast/http.hpp>
namespace net = boost::asio;       // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>
namespace tx_sim {
namespace service {
class CHttpSessionClient : public std::enable_shared_from_this<CHttpSessionClient> {
 public:
  // Objects are constructed with a strand to
  // ensure that handlers do not execute concurrently.
  explicit CHttpSessionClient(net::io_context& ioc)
      : resolver_(net::make_strand(ioc)), stream_(net::make_strand(ioc)) {}
  void run(char const* host, char const* port, char const* target, int version, const std::string& body);
  void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
  void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
  void on_write(beast::error_code ec, std::size_t bytes_transferred);
  void on_read(beast::error_code ec, std::size_t bytes_transferred);

 private:
  tcp::resolver resolver_;
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;  // (Must persist between reads)
  http::request<http::string_body> req_;
  http::response<http::string_body> res_;
};
}  // namespace service
}  // namespace tx_sim
