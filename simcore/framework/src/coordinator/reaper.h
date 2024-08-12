// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <stdint.h>
#include <atomic>
#include <string>
#include <unordered_map>
#ifdef _WIN32
#  include <unordered_set>
#endif  // _WIN32

#include "utils/defs.h"

namespace tx_sim {
namespace coordinator {

//! @brief 类：ModuleReaper
//! @details 类功能：声明一个ModuleReaper类，用于管理模块的退出状态
class ModuleReaper final {
 public:
  //! @brief 构造函数：ModuleReaper
  //! @details 构造函数功能：初始化一个ModuleReaper对象
  ModuleReaper();

  //! @brief 析构函数：ModuleReaper
  //! @details 析构函数功能：销毁一个ModuleReaper对象
  ~ModuleReaper() {}

  //! @brief 函数名：GetModuleExitStatus
  //! @details 函数功能：获取指定进程ID的模块退出状态
  //!
  //! @param[in] pid 一个包含进程ID的txsim_pid_t对象
  //! @param[in] stat_desc 一个包含模块退出状态描述的字符串
  //! @param[in] forced 一个布尔值，表示是否强制获取模块退出状态，默认为false
  //!
  //! @return 返回一个布尔值，表示模块退出状态是否正常
  bool GetModuleExitStatus(txsim_pid_t pid, std::string& stat_desc, bool forced = false);

  //! @brief 函数名：AddQueryHandle
  //! @details 函数功能：添加查询句柄
  //!
  //! @param[in] hdl 一个包含查询句柄的txsim_pid_t对象
  void AddQueryHandle(txsim_pid_t hdl);

 private:
  //! @brief 函数名：Reap
  //! @details 函数功能：回收模块的退出状态
  void Reap();

  //! @brief 函数名：TryBusyLock
  //! @details 函数功能：尝试获取忙锁
  //!
  //! @return 返回一个布尔值，表示忙锁是否成功获取
  bool TryBusyLock();

  //! @brief 函数名：BusyLock
  //! @details 函数功能：获取忙锁
  void BusyLock();

  //! @brief 函数名：BusyUnLock
  //! @details 函数功能：释放忙锁
  void BusyUnLock();

#ifdef _WIN32
  //! @brief 函数名：UpdateQueryIds
  //! @details 函数功能：更新查询ID列表
  //!
  //! @param[in] ids 一个包含查询ID列表的向量
  void UpdateQueryIds(std::vector<txsim_pid_t>& ids);
#endif  // _WIN32

  //! @brief 成员变量：kReapingInterval
  //! @details 成员变量功能：存储回收模块退出状态的时间间隔，单位为毫秒
  static const uint32_t kReapingInterval = 3000;  // ms.

  //! @brief 成员变量：busy_
  //! @details 成员变量功能：存储忙锁的状态
  std::atomic<bool> busy_{false};

  //! @brief 成员变量：exit_info_
  //! @details 成员变量功能：存储模块退出状态信息
  std::unordered_map<txsim_pid_t, std::string> exit_info_;

#ifdef _WIN32
  //! @brief 成员变量：query_handles_
  //! @details 成员变量功能：存储查询句柄列表
  std::unordered_set<txsim_pid_t> query_handles_;
#endif  // _WIN32
};

}  // namespace coordinator
}  // namespace tx_sim
