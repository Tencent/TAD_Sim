// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <memory>

#include "boost/asio.hpp"
#include "boost/beast/core.hpp"

namespace tx_sim {
namespace service {

class Listener final : public std::enable_shared_from_this<Listener> {
 public:
  Listener(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint ep);
  ~Listener();

  void Start(const std::function<void(boost::asio::ip::tcp::socket&&)>& on_accept_callback);

 private:
  void DoAccept();
  void OnAccept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);

 private:
  boost::asio::io_context& ioc_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::function<void(boost::asio::ip::tcp::socket&&)> cb_;
};

}  // namespace service
}  // namespace tx_sim
