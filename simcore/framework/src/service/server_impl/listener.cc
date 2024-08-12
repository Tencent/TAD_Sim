// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "listener.h"

#include <chrono>
#include <ctime>
#include <sstream>

#include "boost/beast/http.hpp"

#include "http_session.h"


namespace beast = boost::beast;
namespace basio = boost::asio;
namespace bhttp = beast::http;
using bst_tcp = boost::asio::ip::tcp;


#ifdef TXSIM_ENCRYPTION_TIMESTAMP
void TestDateTime(basio::io_context& ioc) {
  time_t t = 0;
  bool got_time_from_net = false;
  try {
    bst_tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    const auto host = "www.qq.com";
    const auto result = resolver.resolve(host, "80");
    stream.connect(result);
    bhttp::request<bhttp::string_body> req{bhttp::verb::head, "/", 11};
    req.set(bhttp::field::host, host);
    req.set(bhttp::field::user_agent, "txSim");
    bhttp::write(stream, req);
    beast::flat_buffer buffer;
    bhttp::response_parser<bhttp::empty_body> p;
    p.skip(true);
    bhttp::read(stream, buffer, p);
    const auto& res = p.get();
    for (auto& h : res.base()) {
      if (h.name() != beast::http::field::date) continue;
      const auto str_t = h.value().substr(5, 20).to_string();
#  ifdef _WIN32
      std::tm tm;
      std::stringstream ss(str_t);
      ss >> std::get_time(&tm, "%d %b %Y %H:%M:%S");
      if (ss.fail()) {
        VLOG(3) << "unknown time string: " << str_t;
        break;
      }
      t = std::chrono::duration_cast<std::chrono::seconds>(
              std::chrono::system_clock::from_time_t(std::mktime(&tm)).time_since_epoch())
              .count();
#  elif defined __linux__
      struct tm tmp;
      memset(&tmp, 0, sizeof(struct tm));
      strptime(str_t.c_str(), "%d %b %Y %H:%M:%S", &tmp);
      t = mktime(&tmp);
#  endif  // _WIN32
      got_time_from_net = true;
      break;
    }
    beast::error_code ec;
    stream.socket().shutdown(bst_tcp::socket::shutdown_both, ec);
    if (ec && ec != beast::errc::not_connected) throw beast::system_error{ec};
  } catch (const std::exception& e) { VLOG(3) << "test error: " << e.what(); }
  if (got_time_from_net) {
    if (t > 1648194796 && t < TXSIM_ENCRYPTION_TIMESTAMP) return;
    if (t >= TXSIM_ENCRYPTION_TIMESTAMP) throw std::runtime_error("=====> Software Expired <=====");
    LOG(WARNING) << "invalid t -> " << t;
  }

  auto tt =
      std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  if (tt >= TXSIM_ENCRYPTION_TIMESTAMP) throw std::runtime_error("=====> Software Expired <=====");
}
#endif  // TXSIM_ENCRYPTION_TIMESTAMP


namespace tx_sim {
namespace service {

Listener::Listener(basio::io_context& ioc, bst_tcp::endpoint ep) : ioc_(ioc), acceptor_(basio::make_strand(ioc)) {
  acceptor_.open(ep.protocol());
  acceptor_.set_option(basio::socket_base::reuse_address(true));
  acceptor_.bind(ep);
  acceptor_.listen(basio::socket_base::max_listen_connections);
}


Listener::~Listener() {}


void Listener::Start(const std::function<void(boost::asio::ip::tcp::socket&&)>& on_accept_callback) {
#ifdef TXSIM_ENCRYPTION_TIMESTAMP
  TestDateTime(ioc_);
#endif  // TXSIM_ENCRYPTION_TIMESTAMP
  cb_ = on_accept_callback;
  basio::dispatch(acceptor_.get_executor(), beast::bind_front_handler(&Listener::DoAccept, shared_from_this()));
}


void Listener::DoAccept() {
  acceptor_.async_accept(basio::make_strand(ioc_), beast::bind_front_handler(&Listener::OnAccept, shared_from_this()));
}


void Listener::OnAccept(beast::error_code ec, bst_tcp::socket socket) {
  if (ec) {
    LOG(ERROR) << "error accepting incoming connection: " << ec.message();
  } else {
    VLOG(3) << "accepted new incoming connection.";
    cb_(std::move(socket));
  }

  DoAccept();
}

}  // namespace service
}  // namespace tx_sim
