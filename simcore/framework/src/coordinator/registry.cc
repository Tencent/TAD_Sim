// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "registry.h"

#include "glog/logging.h"

#include "utils/id_impl.h"
#include "utils/leb128.h"
#include "utils/os.h"
#include "utils/time.h"

using namespace tx_sim::utils;
using namespace tx_sim::impl;

namespace tx_sim {
namespace coordinator {

static bool CheckModuleVersion(uint32_t ver, const std::string& name, zmq::multipart_t& msg) {
  uint16_t major, minor, patch;
  const std::string module_ver_str = GetSMCPVersion(ver, major, minor, patch);
  LOG(INFO) << "module " << name << " SMCP version: " << module_ver_str;
  if ((kModuleConnVersion / 1000000) != major) {
    const std::string codr_ver_str = GetSMCPVersionStr(kModuleConnVersion);
    LOG(ERROR) << "module " << name << " SMCP version(" << module_ver_str << ") incompatible with tadsim("
               << codr_ver_str << ")";
    msg.clear();
    uint8_t buf[10];
    AddMsgType(buf, kModuleConnVersionIncompatible, msg);
    msg.addstr(codr_ver_str);
    return false;
  }
  return true;
}

//! @brief 构造函数：ModuleRegistryImpl
//! @details 构造函数功能：初始化一个ModuleRegistryImpl对象
//!
//! @param[in] ctx 一个包含zmq::context_t对象的std::shared_ptr指针
//! @param[in] instance_id 一个包含实例ID的uint16_t对象，默认为0
ModuleRegistryImpl::ModuleRegistryImpl(std::shared_ptr<zmq::context_t>& ctx, uint16_t instance_id)
    : instance_id_(instance_id) {
  InitService(ctx);
  serving_thread_.reset(new std::thread(&ModuleRegistryImpl::Serve, this, ctx));
}

//! @brief 析构函数：ModuleRegistryImpl
//! @details 析构函数功能：销毁一个ModuleRegistryImpl对象
ModuleRegistryImpl::~ModuleRegistryImpl() {
  running_.store(false, std::memory_order_release);
  if (serving_thread_ && serving_thread_->joinable()) serving_thread_->join();
  LOG(INFO) << "ModuleRegistryImpl destructed.";
}

//! @brief 函数名：SetupActiveModuleContext
//! @details 函数功能：设置活动模块上下文
//!
//! @param[in] module_setup 一个包含模块注册设置的ModuleRegisterSetup对象向量
void ModuleRegistryImpl::SetupActiveModuleContext(const std::vector<ModuleRegisterSetup>& module_setup) {
  for (auto& t : async_removing_threads_)
    if (t.joinable()) t.join();
  async_removing_threads_.clear();

  // all the old module which should be replaced by the new one of the setup has been cleared so far.
  // but before the mtx_ has been acquired, there's still chances that manually launched modules come in and registered
  // successfully. see below comments.
  std::lock_guard<std::mutex> lk(mtx_);
  PurgeRegistry();
  module_exit_desc_.clear();

  for (const ModuleRegisterSetup& s : module_setup) {
    // if there's no registered module named name_uuid.first currently, just adding it.
    // if there is one already, it's the cases that the above comment describes. we considering 2 cases here:
    //   1. if the currently setup uuid is empty, it means that this is a remote(manually-launching) module setup. we
    //      should using it directly since the module may have registered before this point. it is normal;
    //   2. if the setup uuid is non empty, it means that this is an auto-launching module. the currently registered
    //      module must be an unexpected one(manually launched somewhere). we just simply replace it with a new one
    //      with the new uuid. when the currently one's heartbeat arrived, it will be invalidated since that its uuid
    //      is not equals to the newly setup one's anymore.
    std::string name = std::get<0>(s), uuid = std::get<1>(s);
    uint32_t step_timeout = std::get<2>(s);
    if (modules_.find(name) == modules_.end()) {  // simply adding the new one.
      auto&& rc = modules_[name];
      rc.uuid = uuid;
    } else {
      // there is one registered already.
      if (uuid.empty()) {
        // cuurently active setup is a remote-launch module, using the current one directly.
        assert(!modules_.find(name)->second.uuid.empty());
      } else {
        // cuurently active setup is an auto-launch module, simply replacing it by the new one.
        assert(!modules_.find(name)->second.uuid.empty());
        modules_.erase(name);
        auto&& rc = modules_[name];
        rc.uuid = uuid;
      }
    }
  }
}

//! @brief 函数名：GetModuleRegisterStatus
//! @details 函数功能：获取指定名称的模块注册状态
//!
//! @param[in] name 一个包含模块名称的字符串
//! @param[in] process_exit_desc 一个包含模块进程退出描述的字符串
//! @param[in] addr 一个包含模块地址的字符串指针，默认为nullptr
//!
//! @return 返回一个布尔值，表示模块注册状态是否正常
bool ModuleRegistryImpl::GetModuleRegisterStatus(const std::string& name, std::string& process_exit_desc,
                                                 std::string* addr) {
  std::unique_lock<std::mutex> lk(mtx_, std::defer_lock);
  if (!addr) {
    if (!lk.try_lock()) return true;
  } else {
    lk.lock();
  }
  PurgeRegistry();
  const auto it = modules_.find(name);
  if (it == modules_.cend()) return false;  // the context only removed when manually launched module expired.
  if (addr) *addr = it->second.addr;
  // exit info only available for the auto-launched(sub-process) modules.
  auto desc_it = module_exit_desc_.find(name);
  if (desc_it != module_exit_desc_.end()) {
    process_exit_desc = desc_it->second;
    module_exit_desc_.erase(desc_it);
  }
  return !it->second.addr.empty();
}

//! @brief 函数名：RemoveRegisteredModule
//! @details 函数功能：移除指定名称的模块注册信息
//!
//! @param[in] name 一个包含模块名称的字符串
void ModuleRegistryImpl::RemoveRegisteredModule(const std::string& module_name) {
  std::lock_guard<std::mutex> lk(mtx_);
  auto it = modules_.find(module_name);
  if (it == modules_.end()) return;
  if (it->second.heartbeat_interval_ms == 0) {
    modules_.erase(it);  // just remove the module register context since there's no hearbeating.
    return;
  }
  // for those remote(non-auto-launch) module, we should tell it exit via heartbeat and waiting it to exit async.
  async_removing_threads_.emplace_back(
      [this](const std::string& name) {
        SetThreadName("txsim-registry-ejector");
        std::unique_lock<std::mutex> lk(mtx_);
        auto it = modules_.find(name);
        if (it == modules_.end()) return;
        auto&& rc = it->second;
        rc.disconnecting = true;
        LOG(INFO) << "waiting module " << name << "(" << rc.uuid << ") to disconnect ...";
        cv_.wait_for(lk, std::chrono::milliseconds(kDefaultHeartbeatTimeoutMillis), [&rc] { return rc.disconnected; });
        // do not using the above iterator since another removing thread may invalidated it.
        it = modules_.find(name);
        if (it == modules_.end()) return;  // may be expired.
        LOG(INFO) << "module " << name << "(" << it->second.uuid << ")"
                  << (it->second.disconnected ? " disconnected gracefully."
                                              : " disconnecting has not confirmed. removed forcibly.");
        modules_.erase(it);
      },
      module_name);
}

//! @brief 函数名：AddChildProcessHandle
//! @details 函数功能：添加子进程句柄
//!
//! @param[in] name 一个包含模块名称的字符串
//! @param[in] hdl 一个包含子进程句柄的txsim_pid_t对象
void ModuleRegistryImpl::AddChildProcessHandle(const std::string& name, txsim_pid_t hdl) {
  std::lock_guard<std::mutex> lk(mtx_);
  auto it = modules_.find(name);
  if (it != modules_.end()) it->second.child_pid = hdl;
  reaper_.AddQueryHandle(hdl);
}

//! @brief 函数名：GetChildProcessHandle
//! @details 函数功能：获取指定名称的子进程句柄
//!
//! @param[in] name 一个包含模块名称的字符串
//!
//! @return 返回一个包含子进程句柄的txsim_pid_t对象
txsim_pid_t ModuleRegistryImpl::GetChildProcessHandle(const std::string& name) {
  std::lock_guard<std::mutex> lk(mtx_);
  PurgeRegistry(true);
  auto it = modules_.find(name);
  return it == modules_.end() ? txsim_invalid_pid_t : it->second.child_pid;
}

//! @brief 函数名：GetLocalRegistryAddr
//! @details 函数功能：获取本地注册地址
//!
//! @return 返回一个包含本地注册地址的字符串
std::string ModuleRegistryImpl::GetLocalRegistryAddr() {
  std::string port_str = service_addr_.substr(service_addr_.rfind(':') + 1);
  return std::string("127.0.0.1:") + port_str;
}

RegisterKey ModuleRegistryImpl::GetRelRegisterKey(RegisterKey key, RegisterContext item) {
  RegisterKey new_key = key;
  for (const auto moduleItem : modules_) {
    if (moduleItem.second.uuid == item.uuid) {
      new_key = moduleItem.first;
      // LOG(INFO) << "registering module nw key" << new_key << "<" << item.uuid << "> ...";
      break;
    }
  }
  return new_key;
}

//! @brief 函数名：Serve
//! @details 函数功能：启动服务
//!
//! @param[in] ctx 一个包含zmq::context_t对象的std::shared_ptr指针
void ModuleRegistryImpl::Serve(std::shared_ptr<zmq::context_t> ctx) {
  static const std::string err_sock_recv("ModuleRegistry: socket recv error"),
      err_sock_send("ModuleRegistry: socket send error"),
      err_forward_send("ModuleRegistry: inner module forwarding socket send error"),
      err_forward_recv("ModuleRegistry: inner module forwarding socket recv error");
  SetThreadName("txsim-registry");

  zmq::multipart_t msg;
  zmq::pollitem_t items[] = {{sock_->handle(), 0, ZMQ_POLLIN, 0}, {inner_forwarding_sock_->handle(), 0, ZMQ_POLLIN, 0}};

  // since the routing id of the connection in the corresponding module context of coordinator is the same as the one of
  // sim module, we could pass it through between the sock_ and the inner_forward_sock_ directly.
  while (running_.load(std::memory_order_acquire)) {
    zmq::poll(items, 2, 1000);

    if (items[0].revents & ZMQ_POLLIN) {  // message of sim module comes in.
      ZmqOpRet ret = DoZMQRecvRoutine(*sock_, msg, err_sock_recv);
      if (ret == ZmqOpRet::kZmqOpError) break;
      assert(ret != ZmqOpRet::kZmqOpAgain);
      zmq::message_t routing_id = msg.pop();
      RegisterKey Origin_key;
      RegisterContext item;
      ModuleMessageType msg_type = UnloadModuleMessage(msg, Origin_key, item);
      RegisterKey key = GetRelRegisterKey(Origin_key, item);
      switch (msg_type) {
        case kModuleRegister: {
          item.addr = PopMsgStr(msg);  // just for back-compatibility. identical to the uuid(routing id).
          uint32_t smcp_ver = PopMsgUint(msg);
          if (CheckModuleVersion(smcp_ver, key, msg)) RegisterModule(key, item, msg);
          break;
        }
        case kModuleUnRegister: UnRegisterModule(key, item.uuid, msg); break;
        case kModuleResponse: {
          msg.push(zmq::message_t());  // prepend the empty delimiter for req socket.
          zmq::message_t id_copy;
          id_copy.copy(routing_id);
          msg.push(std::move(id_copy));
          if (DoZMQSendRoutine(*inner_forwarding_sock_, msg, err_forward_send) != ZmqOpRet::kZmqOpSuccess) break;
        }  // if everything's fine, we need to continue to refresh the module registration.
        case kModuleHeartbeat: RefreshModuleRegisterKey(key, item.uuid, msg); break;
        default: LOG(ERROR) << "Invalid module message type received: " << Enum2String(msg_type);
      }
      if (!msg.empty()) {
        msg.push(std::move(routing_id));
        if (DoZMQSendRoutine(*sock_, msg, err_sock_send) != ZmqOpRet::kZmqOpSuccess) break;
      }
    }

    if (items[1].revents & ZMQ_POLLIN) {  // requests from coordinator to sim module.
      ZmqOpRet ret = DoZMQRecvRoutine(*inner_forwarding_sock_, msg, err_forward_recv);
      if (ret == ZmqOpRet::kZmqOpError) break;
      assert(ret != ZmqOpRet::kZmqOpAgain);
      zmq::message_t routing_id = msg.pop();
      msg.pop();  // since here is a req socket to a dealer, we need pop the front empty delimiter msg out.
      AddMsgType(msg_buf_, kModuleRequest, msg, true);
      msg.push(std::move(routing_id));
      if (DoZMQSendRoutine(*sock_, msg, err_sock_send) != ZmqOpRet::kZmqOpSuccess) break;
    }
  }

  sock_->close();
  inner_forwarding_sock_->close();
  LOG(INFO) << "module registry serving finished.";
}

//! @brief 函数名：UnloadModuleMessage
//! @details 函数功能：卸载模块消息
//!
//! @param[in] msg 一个包含zmq::multipart_t对象的引用
//! @param[in] key 一个包含RegisterKey对象的引用
//! @param[in] item 一个包含RegisterContext对象的引用
//!
//! @return 返回一个tx_sim::impl::ModuleMessageType对象
ModuleMessageType ModuleRegistryImpl::UnloadModuleMessage(zmq::multipart_t& msg, RegisterKey& key,
                                                          RegisterContext& item) {
  ModuleMessageType msg_type;
  try {
    msg_type = static_cast<ModuleMessageType>(PopMsgType(msg));
    key.assign(PopMsgStr(msg));
    item.uuid = PopMsgStr(msg);
    return msg_type;
  } catch (const std::exception& e) {
    LOG(ERROR) << "registry unloading " << Enum2String(msg_type) << " message error: " << e.what();
    return kModuleMessageErrorType;
  }
}

//! @brief 函数名：RegisterModule
//! @details 函数功能：注册模块
//!
//! @param[in] key 一个包含RegisterKey对象的常量引用
//! @param[in] item 一个包含RegisterContext对象的常量引用
//! @param[in] ret 一个包含zmq::multipart_t对象的引用
void ModuleRegistryImpl::RegisterModule(const RegisterKey& key, const RegisterContext& item, zmq::multipart_t& ret) {
  LOG(INFO) << "registering module " << key << "<" << item.uuid << "> ...";
  std::lock_guard<std::mutex> lk(mtx_);
  PurgeRegistry();
  auto&& r = modules_[key];
  if (r.addr.empty()) {    // newly registered module.
    if (r.uuid.empty()) {  // remote(non-auto-launch) module.
      r.uuid = item.uuid;
      r.heartbeat_interval_ms = kDefaultHeartbeatIntervalMillis;
    } else if (r.uuid != item.uuid) {
      LOG(WARNING) << "unexpected module " << key << "(" << item.uuid << ") registering. ignored.";
      AddMsgType(msg_buf_, kModuleRegisterFailed, ret);
      return;
    }
  } else {
    if (r.heartbeat_interval_ms > 0) {  // a same named remote module has already registered.
      if (r.uuid == item.uuid) {
        LOG(ERROR) << "module " << key << "(" << item.uuid << ") re-registered(un-expected register request received)."
                   << " disconnected.";
        modules_.erase(key);  // disconnect the module since there should be some error.
        // TODO(nemo): ifffff there's a malicious module keep sending register request, there could be some problem.
      } else {
        LOG(WARNING) << "module " << key << "(" << item.uuid
                     << ") register failed since there is a same named remote module has" << " registered(" << r.uuid
                     << ") already. ignored.";
      }
      AddMsgType(msg_buf_, kModuleRegisterFailed, ret);
      return;
    } else {  // there's already an auto-launched module registered.
      LOG(WARNING) << "module " << key << "(" << item.uuid
                   << ") register failed since there is a same named auto-launched module has" << " registered("
                   << r.uuid << ") already. ignored.";
      AddMsgType(msg_buf_, kModuleRegisterFailed, ret);
      return;
    }
  }
  AddMsgType(msg_buf_, kModuleRegisterSucceed, ret);
  AddMsgUInt(msg_buf_, r.heartbeat_interval_ms, ret);
  r.addr = item.addr;
  if (r.heartbeat_interval_ms > 0) r.expired_time = std::numeric_limits<uint64_t>::max();
  LOG(INFO) << "module " << key << "<" << item.uuid << "> registered.";
}

//! @brief 函数名：UnRegisterModule
//! @details 函数功能：注销模块
//!
//! @param[in] key 一个包含RegisterKey对象的常量引用
//! @param[in] uuid 一个包含UUID的字符串
//! @param[in] ret 一个包含zmq::multipart_t对象的引用
void ModuleRegistryImpl::UnRegisterModule(const RegisterKey& key, const std::string& uuid, zmq::multipart_t& ret) {
  LOG(INFO) << "un-registering module " << key << "(" << uuid << ") ...";
  std::lock_guard<std::mutex> lk(mtx_);
  auto it = modules_.find(key);
  if (it != modules_.end()) {
    if (it->second.uuid == uuid) {
      AddMsgType(msg_buf_, kModuleRegisterSucceed, ret);
      if (it->second.disconnecting) {  // the removing thread is waiting to disconnect. let it erase the context.
        it->second.disconnected = true;
        cv_.notify_all();
      } else {
        modules_.erase(it);
      }
      LOG(INFO) << "module " << key << "(" << uuid << ") un-registered.";
    } else {
      LOG(WARNING) << "module " << key << "(" << uuid << ") un-register request is ignored since currently registered"
                   << " context(" << it->second.uuid << ") not match.";
      AddMsgType(msg_buf_, kModuleRegisterFailed, ret);
    }
  } else {
    LOG(WARNING) << "module " << key << " un-register request is ignored since the corresponding context not found.";
    AddMsgType(msg_buf_, kModuleRegisterFailed, ret);
  }
}

//! @brief 函数名：RefreshModuleRegisterKey
//! @details 函数功能：刷新模块注册密钥
//!
//! @param[in] key 一个包含RegisterKey对象的常量引用
//! @param[in] uuid 一个包含UUID的字符串
//! @param[in] ret 一个包含zmq::multipart_t对象的引用
void ModuleRegistryImpl::RefreshModuleRegisterKey(const RegisterKey& key, const std::string& uuid,
                                                  zmq::multipart_t& ret) {
  std::lock_guard<std::mutex> lk(mtx_);
  auto it = modules_.find(key);
  if (it == modules_.end()) {
    LOG(WARNING) << "unexpected heartbeat from module " << key << "(" << uuid
                 << ") since the corresponding context not found.";
    AddMsgType(msg_buf_, kModuleRegisterFailed, ret);
    return;
  }
  auto&& r = it->second;
  if (r.uuid != uuid) {
    LOG(WARNING) << "unexpected heartbeat from module " << key << "(" << uuid << ") since currently registered context"
                 << "(" << r.uuid << ") is different.";
    AddMsgType(msg_buf_, kModuleRegisterFailed, ret);
    return;
  }
  if (r.heartbeat_interval_ms == 0) return;
  if (r.disconnecting) {  // tell module sending un-register request to disconnect itself.
    LOG(INFO) << "asking module " << key << " (" << r.uuid << ") to disconnect ...";
    AddMsgType(msg_buf_, kModuleHeartbeatDisconnecting, ret);
    return;
  }
  // refresh its expired time.
  r.expired_time = std::numeric_limits<uint64_t>::max();
}

//! @brief 函数名：PurgeRegistry
//! @details 函数功能：清除注册表
//!
//! @param[in] force_check_child 一个布尔值，表示是否强制检查子进程，默认为false
// NOT thread safe!!!
void ModuleRegistryImpl::PurgeRegistry(bool force_check_child) {
  int64_t current_time = GetCpuTimeInMillis();
  if (current_time - last_purge_time_ < 500) return;
  last_purge_time_ = current_time;
  std::string exit_desc_str;
  for (auto it = modules_.begin(); it != modules_.end();) {
    if (it->second.heartbeat_interval_ms == 0) {  // auto-launched module.
      if (reaper_.GetModuleExitStatus(it->second.child_pid, exit_desc_str, force_check_child)) {
        LOG(INFO) << "module " << it->first << "(" << it->second.uuid << ") process exited: " << exit_desc_str;
        module_exit_desc_[it->first] = exit_desc_str;
        // the context of the auto-launched module should not be removed since its uuid assigned by the engine should be
        // remained and the next auto re-launched one can registered correctly.
        it->second.addr.clear();
        it->second.child_pid = txsim_invalid_pid_t;
      }
    } else {  // manually-launched module.
      if (it->second.heartbeat_interval_ms > 0 && current_time > it->second.expired_time) {
        LOG(WARNING) << "expired module " << it->first << "(" << it->second.uuid << ") has been purged.";
        it = modules_.erase(it);
        continue;
      }
    }
    ++it;
  }
}

//! @brief 函数名：InitService
//! @details 函数功能：初始化服务
//!
//! @param[in] ctx 一个包含zmq::context_t对象的std::shared_ptr指针的引用
void ModuleRegistryImpl::InitService(std::shared_ptr<zmq::context_t>& ctx) {
  sock_.reset(new zmq::socket_t(*ctx, zmq::socket_type::router));
  sock_->set(zmq::sockopt::linger, 0);
  service_addr_ = GetModuleRegistryBindAddress(instance_id_);
  LOG(INFO) << "binding module registry socket at " << service_addr_ << " ..." << std::endl;
  if (DoZMQBindRoutine(*sock_, service_addr_, "ModuleRegistry: register socket bind failed") != ZmqOpRet::kZmqOpSuccess)
    throw std::runtime_error("Module registry service initiation failed.");

  inner_forwarding_sock_.reset(new zmq::socket_t(*ctx, zmq::socket_type::router));
  inner_forwarding_sock_->set(zmq::sockopt::linger, 0);
  if (DoZMQBindRoutine(*inner_forwarding_sock_, kInnerModuleForwardingEndpoint,
                       "ModuleRegistry: inner module forwarding socket bind failed") != ZmqOpRet::kZmqOpSuccess)
    throw std::runtime_error("Module registry service initiation failed.");
}

}  // namespace coordinator
}  // namespace tx_sim
