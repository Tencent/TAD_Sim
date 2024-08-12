#pragma once

#ifdef TXSIM_NO_GLOG
#  include <iostream>
#else
#  include "glog/logging.h"
#endif  // TXSIM_NO_GLOG
#include "zmq_addon.hpp"

namespace tx_sim {
namespace utils {

enum ZmqOpRet {
  kZmqOpSuccess,
  kZmqOpAgain,
  kZmqOpError,
};

inline ZmqOpRet DoZMQRoutine(const std::function<bool()>& op, const std::string& err_msg) {
  try {
    return op() ? kZmqOpSuccess : kZmqOpAgain;
  } catch (const zmq::error_t& e) {
    if (e.num() != EINTR && e.num() != ETERM && !err_msg.empty()) {
#ifdef TXSIM_NO_GLOG
      std::cerr << err_msg << ": [" << e.num() << "] " << e.what() << std::endl;
#else
      LOG(ERROR) << err_msg << ": [" << e.num() << "] " << e.what();
#endif  // TXSIM_NO_GLOG
    }
    return kZmqOpError;
  }
}

inline ZmqOpRet DoZMQBindRoutine(zmq::socket_t& sock, const std::string& addr, const std::string& err_msg) {
  return DoZMQRoutine(
      [&sock, &addr]() -> bool {
        sock.bind(addr);
        return true;
      },
      err_msg);
}

inline ZmqOpRet DoZMQSendRoutine(zmq::socket_t& sock, zmq::multipart_t& msg, const std::string& err_msg) {
  return DoZMQRoutine([&sock, &msg]() -> bool { return msg.send(sock); }, err_msg);
}

inline ZmqOpRet DoZMQRecvRoutine(zmq::socket_t& sock, zmq::multipart_t& msg, const std::string& err_msg) {
  return DoZMQRoutine([&sock, &msg]() -> bool { return msg.recv(sock); }, err_msg);
}

}  // namespace utils
}  // namespace tx_sim
