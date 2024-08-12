#pragma once

#include <stdint.h>
#include <memory>
#include <stdexcept>

#include "txsim_module.h"


/// @cond
extern "C" {

typedef struct txsim_service_error* txsim_service_error_t;
TXSIM_API const char* txsim_service_error_message(txsim_service_error_t err);
TXSIM_API void txsim_service_error_destruct(txsim_service_error_t err);

typedef struct txsim_service* txsim_service_t;
TXSIM_API txsim_service_t txsim_service_construct(void* init_cb, void* reset_cb, void* step_cb, void* stop_cb,
                                                  uint32_t api_version, txsim_service_error_t* out_error);
TXSIM_API void txsim_service_destruct(txsim_service_t s);
TXSIM_API void txsim_service_serve(txsim_service_t s, const char* name, void* module, const char* address);
TXSIM_API void txsim_service_shutdown(txsim_service_t s);
TXSIM_API void txsim_service_wait(txsim_service_t s, txsim_service_error_t* out_error);

}  // extern "C"
/// @endcond


namespace tx_sim {

/// @cond
class ModuleServiceError {
 public:
  ~ModuleServiceError() noexcept(false) {
    if (err_) {
      std::string err_msg(txsim_service_error_message(err_));
      txsim_service_error_destruct(err_);
      //throw std::runtime_error(err_msg);
    }
  }

  operator txsim_service_error_t* () { return &err_; }

 private:
  txsim_service_error_t err_ = nullptr;
};
/// @endcond


/*! \~Chinese
 * 如果用户算法需要编译为一个可执行文件，则需要包含此头文件并使用该类初始化当前仿真模块
 * 的服务线程，以此来将当前实现的 tx_sim::SimModule 接入到仿真系统。如果算法编译为动态库
 * 时则不需要使用此类。
 */
class SimModuleService final {
 public:
  /*! \~Chinese
   * 调用此方法来启动当前仿真模块的服务。只有在调用此方法后该仿真模块才可被仿真系统所识别。
   * 该方法会立即返回。
   *
   * @param[in] name 该仿真模块的名字，不能为空，且必须和前端模块配置中的名字保持
   * 一致，否则无法被系统识别。
   * @param[in] module 继承并实现了仿真接口 tx_sim::SimModule 的子类。
   * @param[in] address 可选，如果传入非空字符串则指定该模块向指定地址进行注册，否则使用
   * 默认的本地仿真服务端口，通常用于启动在远端机器（和仿真系统不在同一台机器）上的模块。
   * 传入的字符串应符合"ip:port"的格式。
   */
  void Serve(const std::string& name, std::shared_ptr<SimModule> module, const std::string& address = "") {
    if (name.empty()) {
      //throw std::invalid_argument("the name provided in SimModuleService::Serve must NOT be empty.");
    }
    txsim_service_serve(s_, name.c_str(), (void*)module.get(), address.c_str());
  }

  /*! \~Chinese
   * 阻塞当前线程，等待当前的仿真模块服务停止。只有当仿真服务发生错误或者收到中断
   * 信号，或者其他线程调用了 #Shutdown 后才会停止服务。
   */
  void Wait() {
    txsim_service_wait(s_, ModuleServiceError{});
  }

  /*! \~Chinese
   * 通知当前的仿真模块服务停止。该方法只是发送停止信号后即立即返回，当前的仿真模块服务
   * 具体何时停止需要等待 #Wait 方法返回。
   */
  void Shutdown() {
    txsim_service_shutdown(s_);
  }

  /// \~Chinese 默认构造器。
  SimModuleService() {
    s_ = txsim_service_construct((void*)&txsim_init, (void*)&txsim_reset, (void*)&txsim_step, (void*)&txsim_stop,
      txsim_messenger_api_version(), ModuleServiceError{});
  }

  /// \~Chinese 默认析构器。
  ~SimModuleService() {
    if (s_) {
      Shutdown();
      Wait();
      txsim_service_destruct(s_);
    }
  }

 private:
  txsim_service_t s_ = nullptr;
};

} // namespace tx_sim
