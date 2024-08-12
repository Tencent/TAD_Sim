// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tx_sim {
namespace coordinator {

class ShmemPool final {
 public:
  //! @brief 添加共享内存发布信息
  //! @details 添加共享内存发布信息
  //! @param[in] pubs 一个包含共享内存发布信息的vector
  void AddShmemPubInfo(const std::vector<std::pair<std::string, size_t>>& pubs);

  //! @brief 更新共享内存发布信息
  //! @return 更新成功返回true，否则返回false
  bool UpdateShmemPubs();

  //! @brief 获取订阅共享内存的名称
  //! @param[in] topic 主题名称
  //! @param[in] module_name 模块名称
  //! @return 返回订阅共享内存的名称
  std::string GetSubShmemName(const std::string& topic, const std::string module_name);

  //! @brief 完成订阅共享内存
  //! @param[in] topic 主题名称
  //! @param[in] module_name 模块名称
  void DoneWithSubShmem(const std::string& topic, const std::string module_name);

  //! @brief 获取发布共享内存的名称
  //! @param[in] topic 主题名称
  //! @param[in] module_name 模块名称
  //! @return 返回发布共享内存的名称
  std::string GetPubShmemName(const std::string& topic, const std::string module_name);

  //! @brief 完成发布共享内存
  //! @param[in] topic 主题名称
  //! @param[in] module_name 模块名称
  void DoneWithPubShmem(const std::string& topic, const std::string module_name);

  //! @brief 打印共享内存状态
  //! @return 返回共享内存状态信息
  std::string PrintShmemStatus();

 private:
  //! @brief 创建共享内存
  //! @param[in] topic 主题名称
  //! @return 返回共享内存名称
  std::string CreateShmem(const std::string& topic);

  //! @brief 成员变量：pub_info_
  //! @details 成员变量功能：存储共享内存发布信息
  std::map<std::string, size_t> pub_info_;

  // key -> shmem name, value -> module names set.
  typedef std::unordered_map<std::string, std::unordered_set<std::string>> ShmemSubStatus;
  // key -> topic name, value -> shmem name.
  typedef std::unordered_map<std::string, std::string> TopicShmemMapping;
  //! @brief 成员变量：shmem_sub_status_
  //! @details 成员变量功能：存储共享内存订阅状态信息
  //! @note key -> topic name
  std::unordered_map<std::string, ShmemSubStatus> shmem_sub_status_;

  //! @brief 成员变量：module_sub_shmems_
  //! @details 成员变量功能：存储模块订阅的共享内存信息
  //! @note  key -> module name
  std::unordered_map<std::string, TopicShmemMapping> module_sub_shmems_;
  struct ShmemPubStatus {
    std::string latest_pub;
    std::string publishing;
    std::unordered_set<std::string> all_pubs;
  };
  //! @brief 成员变量：shmem_pub_status_
  //! @details 成员变量功能：存储共享内存发布状态信息
  std::unordered_map<std::string, ShmemPubStatus> shmem_pub_status_;

  //! @brief 成员变量：shmem_pub_info_
  //! @details 成员变量功能：存储共享内存发布信息
  std::unordered_map<std::string, size_t> shmem_pub_info_;
};

}  // namespace coordinator
}  // namespace tx_sim
