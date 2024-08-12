// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "shmem_pool.h"

#include <cassert>
#include <sstream>

#include "boost/algorithm/string/join.hpp"
#include "boost/interprocess/shared_memory_object.hpp"
#include "glog/logging.h"


using namespace boost::interprocess;
using namespace boost::algorithm;


namespace tx_sim {
namespace coordinator {
//! @brief 添加共享内存发布信息
//! @details 添加共享内存发布信息
//! @param[in] pubs 一个包含共享内存发布信息的vector
void ShmemPool::AddShmemPubInfo(const std::vector<std::pair<std::string, size_t>>& pubs) {
  for (const auto& pub : pubs) pub_info_[pub.first] = pub.second;
}

//! @brief 更新共享内存发布信息
//! @return 更新成功返回true，否则返回false
bool ShmemPool::UpdateShmemPubs() {
  std::ostringstream ss;
  for (const auto& pub : pub_info_) ss << "(topic: " << pub.first << ", size: " << pub.second << ") ";
  LOG(INFO) << "newly updated pub shmem info: [" << ss.str() << "]";
  LOG(INFO) << "currently shmem status: " << PrintShmemStatus();

  // clear unused shmems ...
  // by now all modules should be stopped and un-mapped the shmem.
  bool succeed = true;
  std::unordered_set<std::string> unused;
  for (auto& kv : shmem_pub_status_) {
    auto it = pub_info_.find(kv.first);
    if (it == pub_info_.end()  // the topic is not used anymore. destroy all shmems under it.
        ||
        it->second != shmem_pub_info_[kv.first]) {  // topic is still used but size is different. re-creates it later.
      for (const std::string& shmem : kv.second.all_pubs) {
        if (!shared_memory_object::remove(shmem.c_str())) {
          LOG(ERROR) << "could not remove shmem: " << shmem;
          succeed = false;  // continue to remove others.
        }
      }
      unused.insert(kv.first);
    } else {  // clear the pub status.
      kv.second.latest_pub.clear();
      kv.second.publishing.clear();
    }
  }
  for (const std::string& topic : unused) shmem_pub_status_.erase(topic);
  LOG(INFO) << "done removing un-used shmem topic: [" << join(unused, ",") << "]";
  if (!succeed) return false;

  shmem_sub_status_.clear();
  shmem_pub_info_.clear();
  for (const auto& kv : pub_info_) shmem_pub_info_[kv.first] = kv.second;
  // creating new pub shmem ...
  for (const auto& kv : pub_info_) {
    if (shmem_pub_status_.find(kv.first) == shmem_pub_status_.end() && CreateShmem(kv.first).empty()) return false;
  }

  return true;
}

//! @brief 获取订阅共享内存的名称
//! @param[in] topic 主题名称
//! @param[in] module_name 模块名称
//! @return 返回订阅共享内存的名称
std::string ShmemPool::GetSubShmemName(const std::string& topic, const std::string module_name) {
  auto it = shmem_pub_status_.find(topic);
  if (it != shmem_pub_status_.end() && !it->second.latest_pub.empty()) {  // has latest pub shmem.
    const std::string& sub_name = it->second.latest_pub;
    shmem_sub_status_[topic][sub_name].insert(module_name);  // record the sub status.
    module_sub_shmems_[module_name][topic] = sub_name;
    return sub_name;
  }
  return "";  // no shmem published yet.
}

//! @brief 完成订阅共享内存
//! @param[in] topic 主题名称
//! @param[in] module_name 模块名称
void ShmemPool::DoneWithSubShmem(const std::string& topic, const std::string module_name) {
  auto module_it = module_sub_shmems_.find(module_name);
  if (module_it != module_sub_shmems_.end() && !module_it->second.empty()) {
    auto topic_shmem_it = module_it->second.find(topic);  // which shmem of the topic used by the module currently.
    assert(topic_shmem_it != module_it->second.end());
    auto shmem_it = shmem_sub_status_.find(topic);  // the overall shmem using status of the topic.
    assert(shmem_it != shmem_sub_status_.end());
    auto shmem_status_it = shmem_it->second.find(topic_shmem_it->second);  // modules using the shmem currently.
    assert(shmem_status_it != shmem_it->second.end());
    size_t removed_num = shmem_status_it->second.erase(module_name);  // clear the sub status.
    assert(removed_num == 1);
    module_it->second.erase(topic_shmem_it);
  }
}

//! @brief 获取发布共享内存的名称
//! @param[in] topic 主题名称
//! @param[in] module_name 模块名称
//! @return 返回发布共享内存的名称
std::string ShmemPool::GetPubShmemName(const std::string& topic, const std::string module_name) {
  assert(shmem_pub_status_.find(topic) != shmem_pub_status_.end());
  ShmemPubStatus& pub_status = shmem_pub_status_[topic];
  const ShmemSubStatus& sub_status = shmem_sub_status_[topic];
  // try to find a shmem that neither is the last published one nor the one that are currently used by someone.
  for (const std::string& shmem : pub_status.all_pubs) {
    if (shmem == pub_status.latest_pub) continue;
    auto it = sub_status.find(shmem);
    if (it == sub_status.end() || it->second.size() == 0) {  // currently no modules are using the shmem.
      pub_status.publishing = shmem;
      return shmem;
    }
  }
  // could not find an idle shmem. creating a new one.
  std::string new_shmem_name(CreateShmem(topic));
  if (!new_shmem_name.empty()) pub_status.publishing = new_shmem_name;
  return new_shmem_name;  // TODO(nemo): if it is an empty name, it means that we cannot allocate a new shmem. let
                          // module to handle it?
}

//! @brief 完成发布共享内存
//! @param[in] topic 主题名称
//! @param[in] module_name 模块名称
void ShmemPool::DoneWithPubShmem(const std::string& topic, const std::string module_name) {
  assert(shmem_pub_status_.find(topic) != shmem_pub_status_.end());
  ShmemPubStatus& pub_status = shmem_pub_status_[topic];
  pub_status.latest_pub = pub_status.publishing;
  pub_status.publishing.clear();
}

//! @brief 打印共享内存状态
//! @return 返回共享内存状态信息
std::string ShmemPool::PrintShmemStatus() {
  std::ostringstream ss;
  ss << "shmem sub status => {";
  for (const auto& sub_st : shmem_sub_status_) {
    ss << sub_st.first << ": [";
    for (const auto& shmem_st : sub_st.second) {
      ss << shmem_st.first << ": (";
      for (const std::string& module : shmem_st.second) ss << module << ", ";
      ss << ") ";
    }
    ss << "] ";
  }
  ss << "} shmem pub status => {";
  for (const auto& pub_st : shmem_pub_status_) {
    ss << pub_st.first << ": [";
    const ShmemPubStatus& shmem_st = pub_st.second;
    ss << "latest pub: " << shmem_st.latest_pub << ", ";
    ss << "publishing: " << shmem_st.publishing << ", ";
    ss << "all pubs: (";
    for (const std::string& shmem : shmem_st.all_pubs) ss << shmem << ", ";
    ss << ")]";
  }
  ss << "}";
  return ss.str();
}

//! @brief 创建共享内存
//! @param[in] topic 主题名称
//! @return 返回共享内存名称
std::string ShmemPool::CreateShmem(const std::string& topic) {
  assert(shmem_pub_info_.find(topic) != shmem_pub_info_.end());
  ShmemPubStatus& s = shmem_pub_status_[topic];  // create if necessary.
  std::string name(topic + "_" + std::to_string(s.all_pubs.size()));
  try {
    shared_memory_object shm_obj(open_or_create, name.c_str(), read_write);
    shm_obj.truncate(shmem_pub_info_[topic]);
  } catch (const std::exception& e) {
    LOG(ERROR) << "could not create shmem: " << name << ", what: " << e.what();
    return "";
  }
  s.all_pubs.insert(name);
  LOG(INFO) << ">>> created a new shmem: " << name << " <<<";
  return name;
}

}  // namespace coordinator
}  // namespace tx_sim
