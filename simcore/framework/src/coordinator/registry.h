// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <atomic>
#include <condition_variable>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "boost/uuid/nil_generator.hpp"
#include "boost/uuid/uuid.hpp"

#include "config.h"
#include "reaper.h"
#include "utils/constant.h"
#include "utils/defs.h"
#include "utils/id_impl.h"
#include "utils/zmq_routine.h"

namespace zmq {
class context_t;
class socket_t;
}  // namespace zmq

namespace tx_sim {
namespace coordinator {

typedef std::string RegisterKey;
typedef std::tuple<std::string, std::string, uint32_t> ModuleRegisterSetup;

//! @brief 类：ModuleRegistry
//! @details 类功能：声明一个ModuleRegistry类，用于管理模块注册信息
class ModuleRegistry {
 public:
  //! @brief 函数名：SetupActiveModuleContext
  //! @details 函数功能：设置活动模块上下文
  //!
  //! @param[in] modules 一个包含模块注册设置的ModuleRegisterSetup对象向量
  virtual void SetupActiveModuleContext(const std::vector<ModuleRegisterSetup>& modules) = 0;

  //! @brief 函数名：GetModuleRegisterStatus
  //! @details 函数功能：获取指定名称的模块注册状态
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //! @param[in] process_exit_desc 一个包含模块进程退出描述的字符串
  //! @param[in] addr 一个包含模块地址的字符串指针，默认为nullptr
  //!
  //! @return 返回一个布尔值，表示模块注册状态是否正常
  virtual bool GetModuleRegisterStatus(const std::string& name, std::string& process_exit_desc,
                                       std::string* addr = nullptr) = 0;

  //! @brief 函数名：RemoveRegisteredModule
  //! @details 函数功能：移除指定名称的模块注册信息
  //!
  //! @param[in] name 一个包含模块名称的字符串
  virtual void RemoveRegisteredModule(const std::string& name) = 0;

  //! @brief 函数名：AddChildProcessHandle
  //! @details 函数功能：添加子进程句柄
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //! @param[in] hdl 一个包含子进程句柄的txsim_pid_t对象
  virtual void AddChildProcessHandle(const std::string& name, txsim_pid_t hdl) = 0;

  //! @brief 函数名：GetChildProcessHandle
  //! @details 函数功能：获取指定名称的子进程句柄
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //!
  //! @return 返回一个包含子进程句柄的txsim_pid_t对象
  virtual txsim_pid_t GetChildProcessHandle(const std::string& name) = 0;

  //! @brief 函数名：GetLocalRegistryAddr
  //! @details 函数功能：获取本地注册地址
  //!
  //! @return 返回一个包含本地注册地址的字符串
  virtual std::string GetLocalRegistryAddr() = 0;
};

//! @brief 类：ModuleRegistryImpl
//! @details 类功能：声明一个ModuleRegistryImpl类，用于实现ModuleRegistry接口
class ModuleRegistryImpl final : public ModuleRegistry {
 public:
  //! @brief 构造函数：ModuleRegistryImpl
  //! @details 构造函数功能：初始化一个ModuleRegistryImpl对象
  //!
  //! @param[in] ctx 一个包含zmq::context_t对象的std::shared_ptr指针
  //! @param[in] instance_id 一个包含实例ID的uint16_t对象，默认为0
  ModuleRegistryImpl(std::shared_ptr<zmq::context_t>& ctx, uint16_t instance_id = 0);

  //! @brief 析构函数：ModuleRegistryImpl
  //! @details 析构函数功能：销毁一个ModuleRegistryImpl对象
  ~ModuleRegistryImpl();

  //! @brief 宏：TXSIM_DISABLE_COPY_MOVE
  //! @details 宏功能：禁用ModuleRegistryImpl类的拷贝和移动构造函数
  TXSIM_DISABLE_COPY_MOVE(ModuleRegistryImpl)

  //! @brief 函数名：SetupActiveModuleContext
  //! @details 函数功能：设置活动模块上下文
  //!
  //! @param[in] module_setup 一个包含模块注册设置的ModuleRegisterSetup对象向量
  void SetupActiveModuleContext(const std::vector<ModuleRegisterSetup>& module_setup) override;

  //! @brief 函数名：GetModuleRegisterStatus
  //! @details 函数功能：获取指定名称的模块注册状态
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //! @param[in] process_exit_desc 一个包含模块进程退出描述的字符串
  //! @param[in] addr 一个包含模块地址的字符串指针，默认为nullptr
  //!
  //! @return 返回一个布尔值，表示模块注册状态是否正常
  bool GetModuleRegisterStatus(const std::string& name, std::string& process_exit_desc, std::string* addr) override;

  //! @brief 函数名：RemoveRegisteredModule
  //! @details 函数功能：移除指定名称的模块注册信息
  //!
  //! @param[in] name 一个包含模块名称的字符串
  void RemoveRegisteredModule(const std::string& name) override;

  //! @brief 函数名：AddChildProcessHandle
  //! @details 函数功能：添加子进程句柄
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //! @param[in] hdl 一个包含子进程句柄的txsim_pid_t对象
  void AddChildProcessHandle(const std::string& name, txsim_pid_t hdl) override;

  //! @brief 函数名：GetChildProcessHandle
  //! @details 函数功能：获取指定名称的子进程句柄
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //!
  //! @return 返回一个包含子进程句柄的txsim_pid_t对象
  txsim_pid_t GetChildProcessHandle(const std::string& name) override;

  //! @brief 函数名：GetLocalRegistryAddr
  //! @details 函数功能：获取本地注册地址
  //!
  //! @return 返回一个包含本地注册地址的字符串
  std::string GetLocalRegistryAddr() override;

 private:
  //! @brief 结构体：RegisterContext
  //! @details 结构体功能：存储模块注册上下文信息
  struct RegisterContext {
    std::string uuid;
    std::string addr;  // empty addr means the corresponding module has not registered into the system yet.
    uint64_t expired_time{std::numeric_limits<uint64_t>::max()};
    uint32_t heartbeat_interval_ms{0};  // the default 0 means no heartbeat.
    bool disconnecting{false}, disconnected{false};
    txsim_pid_t child_pid{txsim_invalid_pid_t};
  };

  //! @brief 函数名：Serve
  //! @details 函数功能：启动服务
  //!
  //! @param[in] ctx 一个包含zmq::context_t对象的std::shared_ptr指针
  void Serve(std::shared_ptr<zmq::context_t> ctx);

  //! @brief 函数名：UnloadModuleMessage
  //! @details 函数功能：卸载模块消息
  //!
  //! @param[in] msg 一个包含zmq::multipart_t对象的引用
  //! @param[in] key 一个包含RegisterKey对象的引用
  //! @param[in] item 一个包含RegisterContext对象的引用
  //!
  //! @return 返回一个tx_sim::impl::ModuleMessageType对象
  tx_sim::impl::ModuleMessageType UnloadModuleMessage(zmq::multipart_t& msg, RegisterKey& key, RegisterContext& item);

  //! @brief 函数名：RegisterModule
  //! @details 函数功能：注册模块
  //!
  //! @param[in] key 一个包含RegisterKey对象的常量引用
  //! @param[in] item 一个包含RegisterContext对象的常量引用
  //! @param[in] ret 一个包含zmq::multipart_t对象的引用
  void RegisterModule(const RegisterKey& key, const RegisterContext& item, zmq::multipart_t& ret);

  //! @brief 函数名：UnRegisterModule
  //! @details 函数功能：注销模块
  //!
  //! @param[in] key 一个包含RegisterKey对象的常量引用
  //! @param[in] uuid 一个包含UUID的字符串
  //! @param[in] ret 一个包含zmq::multipart_t对象的引用
  void UnRegisterModule(const RegisterKey& key, const std::string& uuid, zmq::multipart_t& ret);

  //! @brief 函数名：RefreshModuleRegisterKey
  //! @details 函数功能：刷新模块注册密钥
  //!
  //! @param[in] key 一个包含RegisterKey对象的常量引用
  //! @param[in] uuid 一个包含UUID的字符串
  //! @param[in] ret 一个包含zmq::multipart_t对象的引用
  void RefreshModuleRegisterKey(const RegisterKey& key, const std::string& uuid, zmq::multipart_t& ret);

  //! @brief 函数名：PurgeRegistry
  //! @details 函数功能：清除注册表
  //!
  //! @param[in] force_check_child 一个布尔值，表示是否强制检查子进程，默认为false
  void PurgeRegistry(bool force_check_child = false);

  //! @brief 函数名：InitService
  //! @details 函数功能：初始化服务
  //!
  //! @param[in] ctx 一个包含zmq::context_t对象的std::shared_ptr指针的引用
  void InitService(std::shared_ptr<zmq::context_t>& ctx);

 private:
  RegisterKey GetRelRegisterKey(RegisterKey key, RegisterContext item);
  //! @brief 成员变量：instance_id_
  //! @details 成员变量功能：存储实例ID
  const uint16_t instance_id_;

  //! @brief 成员变量：msg_buf_
  //! @details 成员变量功能：存储消息缓冲区
  uint8_t msg_buf_[10];

  //! @brief 成员变量：last_purge_time_
  //! @details 成员变量功能：存储上次清除注册表的时间
  int64_t last_purge_time_ = 0;

  //! @brief 成员变量：modules_
  //! @details 成员变量功能：存储模块注册上下文信息
  std::map<RegisterKey, RegisterContext> modules_;

  //! @brief 成员变量：sock_
  //! @details 成员变量功能：存储zmq::socket_t对象的unique_ptr
  std::unique_ptr<zmq::socket_t> sock_, inner_forwarding_sock_;

  //! @brief 成员变量：serving_thread_
  //! @details 成员变量功能：存储服务线程的unique_ptr
  std::unique_ptr<std::thread> serving_thread_;

  //! @brief 成员变量：async_removing_threads_
  //! @details 成员变量功能：存储异步移除线程的vector
  std::vector<std::thread> async_removing_threads_;

  //! @brief 成员变量：running_
  //! @details 成员变量功能：存储运行状态的原子布尔值
  std::atomic_bool running_{true};

  //! @brief 成员变量：mtx_
  //! @details 成员变量功能：存储互斥锁
  std::mutex mtx_;

  //! @brief 成员变量：cv_
  //! @details 成员变量功能：存储条件变量
  std::condition_variable cv_;

  //! @brief 成员变量：service_addr_
  //! @details 成员变量功能：存储服务地址
  std::string service_addr_;

  //! @brief 成员变量：reaper_
  //! @details 成员变量功能：存储ModuleReaper对象
  ModuleReaper reaper_;

  //! @brief 成员变量：module_exit_desc_
  //! @details 成员变量功能：存储模块退出描述的map
  std::map<std::string, std::string> module_exit_desc_;
};

}  // namespace coordinator
}  // namespace tx_sim
