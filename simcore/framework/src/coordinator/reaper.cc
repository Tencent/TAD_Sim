// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "reaper.h"

#include <thread>

#include "utils/os.h"
#include "utils/proc.h"
#include "utils/time.h"

namespace tx_sim {
namespace coordinator {
//! @brief 构造函数：ModuleReaper
//! @details 构造函数功能：初始化一个ModuleReaper对象
ModuleReaper::ModuleReaper() {
  std::thread t(&ModuleReaper::Reap, this);
  t.detach();
}

//! @brief 函数名：GetModuleExitStatus
//! @details 函数功能：获取指定进程ID的模块退出状态
//!
//! @param[in] pid 一个包含进程ID的txsim_pid_t对象
//! @param[in] stat_desc 一个包含模块退出状态描述的字符串
//! @param[in] forced 一个布尔值，表示是否强制获取模块退出状态，默认为false
//!
//! @return 返回一个布尔值，表示模块退出状态是否正常
bool ModuleReaper::GetModuleExitStatus(txsim_pid_t pid, std::string& stat_desc, bool forced) {
  if (pid == txsim_invalid_pid_t) return false;
  bool exited = false, locked = true;
  if (forced) BusyLock();
  else
    locked = TryBusyLock();
  if (locked) {
    auto it = exit_info_.find(pid);
    if (it != exit_info_.end()) {
      stat_desc = it->second;
      exit_info_.erase(it);
      exited = true;
    }
    BusyUnLock();
  }
  return exited;
}

//! @brief 函数名：AddQueryHandle
//! @details 函数功能：添加查询句柄
//!
//! @param[in] hdl 一个包含查询句柄的txsim_pid_t对象
void ModuleReaper::AddQueryHandle(txsim_pid_t hdl) {
#ifdef _WIN32
  BusyLock();
  query_handles_.insert(hdl);
  BusyUnLock();
#endif  // _WIN32
}

//! @brief 函数名：Reap
//! @details 函数功能：回收模块的退出状态
void ModuleReaper::Reap() {
  tx_sim::utils::SetThreadName("txsim-reaper");
  tx_sim::utils::ChildProcessStatus stat;
  while (true) {
#ifdef _WIN32
    UpdateQueryIds(stat.queries);
#endif  // _WIN32
    tx_sim::utils::PollChildrenExitStatus(stat);
    if (stat.pid != txsim_invalid_pid_t) {
      BusyLock();
      exit_info_[stat.pid] = stat.status_desc;
#ifdef _WIN32
      query_handles_.erase(stat.pid);
#endif  // _WIN32
      BusyUnLock();
    } else {  // only sleeps when no evnet occurred.
      tx_sim::utils::SleepForMillis(kReapingInterval);
    }
  }
}

//! @brief 函数名：TryBusyLock
//! @details 函数功能：尝试获取忙锁
//!
//! @return 返回一个布尔值，表示忙锁是否成功获取
bool ModuleReaper::TryBusyLock() {
  bool expected = false;
  return busy_.compare_exchange_strong(expected, true, std::memory_order_acq_rel);
}

//! @brief 函数名：BusyLock
//! @details 函数功能：获取忙锁
void ModuleReaper::BusyLock() {
  do {  // busy waiting since read op is quick.
    bool expected = false;
    if (busy_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) break;
    tx_sim::utils::SleepForMillis(10);
  } while (true);
}

//! @brief 函数名：BusyUnLock
//! @details 函数功能：释放忙锁
void ModuleReaper::BusyUnLock() {
  busy_.store(false, std::memory_order_release);
}

//! @brief 函数名：UpdateQueryIds
//! @details 函数功能：更新查询ID列表
//!
//! @param[in] ids 一个包含查询ID列表的向量
#ifdef _WIN32
void ModuleReaper::UpdateQueryIds(std::vector<txsim_pid_t>& ids) {
  ids.clear();
  BusyLock();
  for (const auto& hdl : query_handles_) ids.push_back(hdl);
  BusyUnLock();
}
#endif  // _WIN32

}  // namespace coordinator
}  // namespace tx_sim
