// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <map>
#include <memory>
#include <vector>

#include "boost/uuid/uuid.hpp"
#include "zmq_addon.hpp"

#include "config.h"
#include "utils/constant.h"
#include "utils/defs.h"
#include "utils/msgs.h"

namespace tx_sim {
namespace coordinator {

enum ModuleConnnectionStatus : int16_t {
  kUnSet = 0,
  kSet = 1,
  kConnected = 2,
  kTimeout = 3,
  kClosed = 4,
  kError = 5,
};

/**
 *  async, not thread-safe connection class. return false means that the waiting/retry is still needed,
 *  otherwise it indicates there is a result(either success or failure) to be taken.
 */
class ModuleConnection {
 public:
  //!
  //! @brief 函数名：Send
  //! @details 函数功能：异步发送请求
  //!
  //! @param[in] request 要发送的请求
  //! @return 如果请求成功放入底层的待发送缓冲区，则返回 true，否则返回 false
  //! @note async op. return true immediately when request is put in the underlying outsanding buffer queue
  //! or false on queuing failed.
  virtual bool Send(tx_sim::impl::ModuleRequest& request) = 0;

  //!
  //! @brief 函数名：Receive
  //! @details 函数功能：异步接收响应
  //!
  //! @param[out] response 接收到的响应
  //! @param[in] waiting_time 等待时间（默认为 0，表示无限等待）
  //! @return 如果收到响应或超时/错误，则返回 true，否则返回 false
  //!
  //! @note async op. return true on response received or timeout/error, false otherwise.
  virtual bool Receive(tx_sim::impl::ModuleResponse& response, uint32_t waiting_time = 0) = 0;

  //!
  //! @brief 函数名：WaitingReply
  //! @details 函数功能：检查是否还有未收到的响应
  //!
  //! @return 如果没有收到响应，则返回 true，否则返回 false
  //!
  //! @note returns true if no response received yet for the last sent request.
  virtual bool WaitingReply() = 0;
};

class ModuleConnectionFactory {
 public:
  //!
  //! @brief 函数名：CreateConnection
  //! @details 函数功能：创建 ModuleConnection 对象
  //!
  //! @param[in] endpoint 连接端点
  //! @param[in] config 连接配置
  //! @return 返回创建的 ModuleConnection 对象
  //!
  //! @note returns nullptr if endpoint is empty.
  virtual std::shared_ptr<ModuleConnection> CreateConnection(const std::string& endpoint,
                                                             const ConnectionConfig& config) = 0;
};

class ZmqConnection final : public ModuleConnection {
 public:
  //!
  //! @brief 函数名：ZmqConnection
  //! @details 函数功能：构造函数，用于创建 ZmqConnection 对象
  //!
  //! @param[in] ctx 共享的 zmq::context_t 对象
  //! @param[in] addr 连接地址
  //! @param[in] config 连接配置
  //!
  ZmqConnection(const std::shared_ptr<zmq::context_t>& ctx, const std::string& addr, const ConnectionConfig& config);

  //!
  //! @brief 函数名：~ZmqConnection
  //! @details 函数功能：析构函数，用于销毁 ZmqConnection 对象
  //!
  ~ZmqConnection() {}

  //!
  //! @brief 函数名：TXSIM_DISABLE_COPY_MOVE
  //! @details 函数功能：禁用拷贝和移动构造函数
  //!
  TXSIM_DISABLE_COPY_MOVE(ZmqConnection)

  //!
  //! @brief 函数名：Send
  //! @details 函数功能：发送 ModuleRequest 对象
  //!
  //! @param[in] request 要发送的 ModuleRequest 对象
  //! @return 发送成功返回 true，否则返回 false
  //!
  bool Send(tx_sim::impl::ModuleRequest& request) override;

  //!
  //! @brief 函数名：Receive
  //! @details 函数功能：接收 ModuleResponse 对象
  //!
  //! @param[out] response 存储接收到的 ModuleResponse 对象
  //! @param[in] waiting_time 等待时间（单位：毫秒）
  //! @return 接收成功返回 true，否则返回 false
  //!
  bool Receive(tx_sim::impl::ModuleResponse& response, uint32_t waiting_time) override;

  //!
  //! @brief 函数名：WaitingReply
  //! @details 函数功能：检查是否正在等待回复
  //!
  //! @return 如果正在等待回复返回 true，否则返回 false
  //!
  bool WaitingReply() override;

 private:
  //!
  //! @brief 函数名：ReceiveMessageBlocking
  //! @details 函数功能：阻塞接收消息
  //!
  //! @param[out] msg 存储接收到的消息
  //! @param[in] blocking_time 阻塞时间（单位：毫秒）
  //! @return 接收成功返回 true，否则返回 false
  //!
  bool ReceiveMessageBlocking(zmq::multipart_t& msg, uint32_t blocking_time);

  int64_t timeout_point_ = 0;
  bool waiting_reply_{false};
  zmq::multipart_t received_msg_;
  const ConnectionConfig config_;
  const std::string addr_;
  std::shared_ptr<zmq::context_t> zmq_ctx_;
  std::unique_ptr<zmq::socket_t> sock_;
};

class ZmqConnectionFactory final : public ModuleConnectionFactory {
 public:
  //!
  //! @brief 函数名：ZmqConnectionFactory
  //! @details 函数功能：构造函数，用于创建 ZmqConnectionFactory 对象
  //!
  //! @param[in] ctx 共享的 zmq::context_t 对象
  //!
  ZmqConnectionFactory(const std::shared_ptr<zmq::context_t>& ctx);

  //!
  //! @brief 函数名：CreateConnection
  //! @details 函数功能：创建 ModuleConnection 对象
  //!
  //! @param[in] endpoint 连接端点
  //! @param[in] config 连接配置
  //! @return 返回创建的 ModuleConnection 对象
  //!
  std::shared_ptr<ModuleConnection> CreateConnection(const std::string& endpoint,
                                                     const ConnectionConfig& config) override;

 private:
  std::shared_ptr<zmq::context_t> zmq_ctx_;
};

}  // namespace coordinator
}  // namespace tx_sim
