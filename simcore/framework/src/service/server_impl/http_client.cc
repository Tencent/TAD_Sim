// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "http_client.h"
#include "glog/logging.h"

namespace tx_sim {
namespace service {

void fail(beast::error_code ec, char const* what) {
  LOG(ERROR) << what << ": " << ec.message();
}

// Start the asynchronous operation
void CHttpSessionClient::run(char const* host, char const* port, char const* target, int version,
                             const std::string& body) {
  // Set up an HTTP GET request message
  req_.method(http::verb::post);
  req_.target(target);
  req_.set(http::field::host, host);
  req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req_.set(http::field::content_type, "application/json");

  req_.content_length(body.size());
  req_.body() = body;
  // Look up the domain name
  LOG(INFO) << "http Type " << http::verb::post;
  LOG(INFO) << "http target " << target;
  LOG(INFO) << "http host " << host;
  LOG(INFO) << "http port " << port;
  LOG(INFO) << "http body size " << body.size();
  resolver_.async_resolve(host, port, beast::bind_front_handler(&CHttpSessionClient::on_resolve, shared_from_this()));
}

void CHttpSessionClient::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
  if (ec) return fail(ec, "resolve");
  LOG(INFO) << "on_resolve";

  // Set a timeout on the operation
  stream_.expires_after(std::chrono::seconds(30));

  // Make the connection on the IP address we get from a lookup
  stream_.async_connect(results, beast::bind_front_handler(&CHttpSessionClient::on_connect, shared_from_this()));
}

void CHttpSessionClient::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
  if (ec) return fail(ec, "connect");
  LOG(INFO) << "connect";

  // Set a timeout on the operation
  stream_.expires_after(std::chrono::seconds(30));

  // Send the HTTP request to the remote host
  http::async_write(stream_, req_, beast::bind_front_handler(&CHttpSessionClient::on_write, shared_from_this()));
}

void CHttpSessionClient::on_write(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec) return fail(ec, "write");
  LOG(INFO) << "on_write";
  LOG(INFO) << res_ << std::endl;

  // Receive the HTTP response
  http::async_read(stream_, buffer_, res_, beast::bind_front_handler(&CHttpSessionClient::on_read, shared_from_this()));
}

void CHttpSessionClient::on_read(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec) return fail(ec, "read");
  LOG(INFO) << "on_read";
  // Write the message to standard out
  LOG(INFO) << res_ << std::endl;

  // Gracefully close the socket
  stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

  // not_connected happens sometimes so don't bother reporting it.
  if (ec && ec != beast::errc::not_connected) return fail(ec, "shutdown");

  // If we get here then the connection is closed gracefully
}


}  // namespace service
}  // namespace tx_sim
