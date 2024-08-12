// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <stdint.h>
#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "zmq_addon.hpp"

#include "utils/constant.h"

namespace tx_sim {
namespace coordinator {

//! @brief 结构体名：TopicPubSubInfo
//! @details 结构体功能：存储主题发布和订阅信息
struct TopicPubSubInfo {
  //! @brief 成员变量：sub_topics
  //! @details 成员变量功能：存储订阅的主题列表
  std::vector<std::string> sub_topics;

  //! @brief 成员变量：pub_topics
  //! @details 成员变量功能：存储发布的主题列表
  std::vector<std::string> pub_topics;

  //! @brief 成员变量：sub_shmems
  //! @details 成员变量功能：存储订阅的共享内存列表
  std::vector<std::string> sub_shmems;

  //! @brief 成员变量：pub_shmems
  //! @details 成员变量功能：存储发布的共享内存列表
  std::vector<std::pair<std::string, size_t>> pub_shmems;
  std::unordered_set<std::string> pub_topics_broadcast;
  std::unordered_set<std::string> pub_shmems_broadcast;

  //! @brief 成员函数：SetTopics
  //! @details 成员函数功能：设置主题发布和订阅信息
  //!
  //! @param[in] st 订阅的主题集合
  //! @param[in] pt 发布的主题集合
  //! @param[in] ss 订阅的共享内存集合
  //! @param[in] ps 发布的共享内存集合
  void SetTopics(const std::unordered_set<std::string>& st, const std::unordered_set<std::string>& pt,
                 const std::unordered_set<std::string>& ss, const std::unordered_map<std::string, size_t>& ps,
                 const std::unordered_set<std::string>& ptb, const std::unordered_set<std::string>& psb) {
    sub_topics.insert(sub_topics.end(), st.cbegin(), st.cend());
    pub_topics.insert(pub_topics.end(), pt.cbegin(), pt.cend());
    sub_shmems.insert(sub_shmems.end(), ss.cbegin(), ss.cend());
    pub_shmems.insert(pub_shmems.end(), ps.cbegin(), ps.cend());
    pub_topics_broadcast = ptb;
    pub_shmems_broadcast = psb;
  }

  //! @brief 成员函数：Clear
  //! @details 成员函数功能：清除主题发布和订阅信息
  void Clear() {
    sub_topics.clear();
    pub_topics.clear();
    sub_shmems.clear();
    pub_shmems.clear();
    pub_topics_broadcast.clear();
    pub_shmems_broadcast.clear();
  }

  //! @brief 成员函数：operator=
  //! @details 成员函数功能：赋值操作符
  //!
  //! @param[in] other 另一个TopicPubSubInfo对象
  //! @return 返回赋值后的TopicPubSubInfo对象
  TopicPubSubInfo& operator=(const TopicPubSubInfo& other) {
    if (this == &other) return *this;
    sub_topics = other.sub_topics;
    pub_topics = other.pub_topics;
    sub_shmems = other.sub_shmems;
    pub_shmems = other.pub_shmems;
    return *this;
  }

  //! @brief 成员函数：operator==
  //! @details 成员函数功能：比较两个TopicPubSubInfo对象是否相等
  //!
  //! @param[in] rhs 另一个TopicPubSubInfo对象
  //! @return 如果两个对象相等，返回true，否则返回false
  bool operator==(const TopicPubSubInfo& rhs) const;

  //! @brief 成员函数：operator!=
  //! @details 成员函数功能：比较两个TopicPubSubInfo对象是否不相等
  //!
  //! @param[in] rhs 另一个TopicPubSubInfo对象
  //! @return 如果两个对象不相等，返回true，否则返回false
  bool operator!=(const TopicPubSubInfo& rhs) const { return !(*this == rhs); }
};

//! @brief 结构体名：ModuleInitStatus
//! @details 结构体功能：存储模块初始化状态信息
struct ModuleInitStatus {
  //! @brief 成员变量：name
  //! @details 成员变量功能：存储模块名称
  std::string name;

  //! @brief 成员变量：state
  //! @details 成员变量功能：存储模块初始化状态
  tx_sim::impl::ModuleInitState state{tx_sim::impl::kModuleInitStart};

  //! @brief 成员变量：topic_info
  //! @details 成员变量功能：存储主题发布和订阅信息
  TopicPubSubInfo topic_info;

  //! @brief 成员变量：log_file
  //! @details 成员变量功能：存储日志文件路径
  std::string log_file;

  //! @brief 成员变量：binary_updated
  //! @details 成员变量功能：标记模块二进制文件是否已更新
  bool binary_updated{false};

  //! @brief 构造函数
  //! @details 构造函数功能：初始化模块初始化状态信息
  ModuleInitStatus() {}

  //! @brief 构造函数
  //! @details 构造函数功能：初始化模块初始化状态信息
  //!
  //! @param[in] n 模块名称
  //! @param[in] s 模块初始化状态
  ModuleInitStatus(const std::string& n, tx_sim::impl::ModuleInitState s) : name(n), state(s) {}

  //! @brief 成员函数：operator==
  //! @details 成员函数功能：比较两个ModuleInitStatus对象是否相等
  //!
  //! @param[in] rhs 另一个ModuleInitStatus对象
  //! @return 如果两个对象相等，返回true，否则返回false
  bool operator==(const ModuleInitStatus& rhs) const;

  //! @brief 成员函数：operator!=
  //! @details 成员函数功能：比较两个ModuleInitStatus对象是否不相等
  //!
  //! @param[in] rhs 另一个ModuleInitStatus对象
  //! @return 如果两个对象不相等，返回true，否则返回false
  bool operator!=(const ModuleInitStatus& rhs) const { return !(*this == rhs); }
};

struct EgoScheme {
  std::string sEgoGroup;
  std::string sScheme;
  EgoScheme() {
    sEgoGroup = "";
    sScheme = "";
  }

  EgoScheme(std::string inputGroup, std::string inputScheme) {
    sEgoGroup = inputGroup;
    sScheme = inputScheme;
  }

  bool operator==(const EgoScheme& rhs) const { return sEgoGroup == rhs.sEgoGroup && sScheme == rhs.sScheme; }
};

struct EgoSchemeHash {
  std::size_t operator()(const EgoScheme& k) const {
    return std::hash<std::string>()(k.sEgoGroup) ^ (std::hash<std::string>()(k.sScheme) << 1);
  }
};

struct EgoSchemeEqual {
  bool operator()(const EgoScheme& lhs, const EgoScheme& rhs) const {
    return lhs.sEgoGroup == rhs.sEgoGroup && lhs.sScheme == rhs.sScheme;
  }
};

struct EgoTopic {
  std::string sEgoGroup;
  std::string sTopic;
  EgoTopic() {
    sEgoGroup = "";
    sTopic = "";
  }

  EgoTopic(std::string inputTopic) {
    sEgoGroup = "";
    sTopic = inputTopic;
  }
  EgoTopic(std::string inputGroup, std::string inputTopic) {
    sEgoGroup = inputGroup;
    sTopic = inputTopic;
  }

  bool operator==(const EgoTopic& rhs) const { return sEgoGroup == rhs.sEgoGroup && sTopic == rhs.sTopic; }
};

struct EgoTopicHash {
  std::size_t operator()(const EgoTopic& k) const {
    return std::hash<std::string>()(k.sEgoGroup) ^ (std::hash<std::string>()(k.sTopic) << 1);
  }
};

struct EgoTopicEqual {
  bool operator()(const EgoTopic& lhs, const EgoTopic& rhs) const {
    return lhs.sEgoGroup == rhs.sEgoGroup && lhs.sTopic == rhs.sTopic;
  }
};

typedef std::vector<std::pair<EgoTopic, zmq::message_t>> TopicMessageList;

struct StepMessage {
  //! @brief 成员变量：timestamp
  //! @details 成员变量功能：存储时间戳（以毫秒为单位）
  uint64_t timestamp{0};

  //! @brief 成员变量：messages
  //! @details 成员变量功能：存储主题消息对列表
  TopicMessageList messages;

  //! @brief 成员函数：operator==
  //! @details 成员函数功能：比较两个StepMessage对象是否相等
  //!
  //! @param[in] rhs 另一个StepMessage对象
  //! @return 如果两个对象相等，返回true，否则返回false
  bool operator==(const StepMessage& rhs) const;

  //! @brief 成员函数：operator!=
  //! @details 成员函数功能：比较两个StepMessage对象是否不相等
  //!
  //! @param[in] rhs 另一个StepMessage对象
  //! @return 如果两个对象不相等，返回true，否则返回false
  bool operator!=(const StepMessage& rhs) const { return !(*this == rhs); }
};

//! @brief 结构体名：ModuleCmdStatus
//! @details 结构体功能：存储模块命令状态信息
struct ModuleCmdStatus {
  //! @brief 成员变量：name
  //! @details 成员变量功能：存储模块名称
  std::string name;

  //! @brief 成员变量：elapsed_time
  //! @details 成员变量功能：存储模块运行时间（以毫秒为单位）
  uint32_t elapsed_time{0};

  //! @brief 成员变量：memory
  //! @details 成员变量功能：存储模块内存使用量（以KB为单位）
  uint32_t memory{0};

  //! @brief 成员变量：cpu_time_us
  //! @details 成员变量功能：存储模块用户空间CPU时间（以微秒为单位）
  uint32_t cpu_time_us{0};

  //! @brief 成员变量：cpu_time_sy
  //! @details 成员变量功能：存储模块系统空间CPU时间（以微秒为单位）
  uint32_t cpu_time_sy{0};

  //! @brief 成员变量：fps
  //! @details 成员变量功能：存储模块帧率
  uint16_t fps{0};

  //! @brief 成员变量：ec
  //! @details 成员变量功能：存储模块响应类型
  tx_sim::impl::ModuleResponseType ec{tx_sim::impl::kModuleOK};

  //! @brief 成员变量：msg
  //! @details 成员变量功能：存储模块错误信息
  std::string msg;

  //! @brief 成员变量：feedback
  //! @details 成员变量功能：存储模块反馈信息（JSON字符串）
  std::string feedback;

  //! @brief 构造函数
  //! @details 构造函数功能：初始化模块命令状态信息
  ModuleCmdStatus() {}

  //! @brief 构造函数
  //! @details 构造函数功能：初始化模块命令状态信息
  //!
  //! @param[in] n 模块名称
  //! @param[in] t 模块响应类型
  //! @param[in] err 模块错误信息
  ModuleCmdStatus(const std::string& n, tx_sim::impl::ModuleResponseType t, const std::string& err)
      : name(n), ec(t), msg(err) {}

  //! @brief 成员函数：operator==
  //! @details 成员函数功能：比较两个ModuleCmdStatus对象是否相等
  //!
  //! @param[in] rhs 另一个ModuleCmdStatus对象
  //! @return 如果两个对象相等，返回true，否则返回false
  bool operator==(const ModuleCmdStatus& rhs) const;

  //! @brief 成员函数：operator!=
  //! @details 成员函数功能：比较两个ModuleCmdStatus对象是否不相等
  //!
  //! @param[in] rhs 另一个ModuleCmdStatus对象
  //! @return 如果两个对象不相等，返回true，否则返回false
  bool operator!=(const ModuleCmdStatus& rhs) const { return !(*this == rhs); }
};

//! @brief 结构体名：CommandStatus
//! @details 结构体功能：存储命令状态信息
struct CommandStatus {
  //! @brief 成员变量：ec
  //! @details 成员变量功能：存储命令错误代码
  tx_sim::impl::CmdErrorCode ec{tx_sim::impl::kCmdSucceed};

  //! @brief 成员变量：total_time_cost
  //! @details 成员变量功能：存储命令总运行时间（以毫秒为单位）
  uint32_t total_time_cost{0};

  //! @brief 成员变量：total_cpu_time_us
  //! @details 成员变量功能：存储命令用户空间CPU时间（以毫秒为单位）
  double total_cpu_time_us{0};

  //! @brief 成员变量：total_cpu_time_sy
  //! @details 成员变量功能：存储命令系统空间CPU时间（以毫秒为单位）
  double total_cpu_time_sy{0};

  //! @brief 成员变量：module_status
  //! @details 成员变量功能：存储模块命令状态信息列表
  std::vector<ModuleCmdStatus> module_status;

  //! @brief 成员变量：init_status
  //! @details 成员变量功能：存储模块初始化状态信息列表
  std::vector<ModuleInitStatus> init_status;

  //! @brief 成员变量：step_message
  //! @details 成员变量功能：存储时间戳和主题消息对列表
  StepMessage step_message;

  //! @brief 构造函数
  //! @details 构造函数功能：初始化命令状态信息
  CommandStatus() {}

  //! @brief 构造函数
  //! @details 构造函数功能：初始化命令状态信息
  //!
  //! @param[in] ret 命令错误代码
  CommandStatus(tx_sim::impl::CmdErrorCode ret) : ec(ret) {}

  //! @brief 构造函数
  //! @details 构造函数功能：初始化命令状态信息
  //!
  //! @param[in] st 另一个CommandStatus对象
  CommandStatus(const CommandStatus& st) { *this = st; }

  //! @brief 成员函数：operator=
  //! @details 成员函数功能：赋值操作
  //!
  //! @param[in] st 另一个CommandStatus对象
  //! @return 当前对象的引用
  CommandStatus& operator=(const CommandStatus& st);

  //! @brief 成员函数：operator==
  //! @details 成员函数功能：比较两个CommandStatus对象是否相等
  //!
  //! @param[in] rhs 另一个CommandStatus对象
  //! @return 如果两个对象相等，返回true，否则返回false
  bool operator==(const CommandStatus& rhs) const;

  //! @brief 成员函数：operator!=
  //! @details 成员函数功能：比较两个CommandStatus对象是否不相等
  //!
  //! @param[in] rhs 另一个CommandStatus对象
  //! @return 如果两个对象不相等，返回true，否则返回false
  bool operator!=(const CommandStatus& rhs) const { return !(*this == rhs); }

  //! @brief 成员函数：Clear
  //! @details 成员函数功能：清除命令状态信息
  void Clear();

  //! @brief 成员函数：ApplyErrCode
  //! @details 成员函数功能：应用目标错误代码（仅在目标错误代码大于当前错误代码时生效）
  //!
  //! @param[in] target 目标错误代码
  void ApplyErrCode(tx_sim::impl::CmdErrorCode target);

  //! @brief 成员函数：GetTimeCostStr
  //! @details 成员函数功能：获取命令运行时间字符串
  //!
  //! @return 命令运行时间字符串
  std::string GetTimeCostStr() const;

  //! @brief 成员函数：Encode
  //! @details 成员函数功能：将命令状态信息编码为多部分消息
  //!
  //! @param[in,out] msg 多部分消息
  void Encode(zmq::multipart_t& msg) const;

  //! @brief 成员函数：Decode
  //! @details 成员函数功能：从多部分消息中解码命令状态信息
  //!
  //! @param[in,out] msg 多部分消息
  void Decode(zmq::multipart_t& msg);

  //! @brief 成员函数：EncodeRawMsgs
  //! @details 成员函数功能：将原始消息编码为字节数组
  //!
  //! @param[in,out] payload 字节数组
  void EncodeRawMsgs(std::vector<uint8_t>& payload) const;
};

//! @brief 类型别名：CommandStatusCb
//! @details 类型别名功能：定义一个回调函数类型，用于处理命令状态信息
typedef std::function<void(const CommandStatus& status)> CommandStatusCb;

//! @brief 类型别名：TopicWriterCb
//! @details 类型别名功能：定义一个回调函数类型，用于处理主题写入器信息
typedef std::function<void(const std::unordered_map<std::string, std::set<std::string>>& su)> TopicWriterCb;

//! @brief 函数名：GetStatusName
//! @details 函数功能：获取模块命令状态信息的名称
//!
//! @param[in] st 一个包含模块命令状态信息的ModuleCmdStatus对象
//! @return 模块命令状态信息的名称
inline std::string GetStatusName(const ModuleCmdStatus& st) { return st.name; }

//! @brief 函数名：GetStatusName
//! @details 函数功能：获取模块初始化状态信息的名称
//!
//! @param[in] st 一个包含模块初始化状态信息的ModuleInitStatus对象
//! @return 模块初始化状态信息的名称
inline std::string GetStatusName(const ModuleInitStatus& st) { return st.name; }
//! @brief 函数名：GetStatusName
//! @details 函数功能：获取字符串键值对的名称
//!
//! @param[in] p 一个包含字符串键值对的std::pair对象
//! @return 字符串键值对的名称
inline std::string GetStatusName(const std::pair<EgoTopic, zmq::message_t>& p) {
  return p.first.sEgoGroup + p.first.sTopic;
}

//! @brief 函数名：GetStatusName
//! @details 函数功能：获取字符串键值对的名称
//!
//! @param[in] p 一个包含字符串键值对的std::pair对象
//! @return 字符串键值对的名称
inline std::string GetStatusName(const std::pair<EgoTopic, size_t>& p) { return p.first.sEgoGroup + p.first.sTopic; }

//! @brief 函数名：GetStatusName
//! @details 函数功能：获取字符串的名称
//!
//! @param[in] s 一个字符串
//! @return 字符串的名称
inline std::string GetStatusName(const std::string& s) { return s; }

//! @brief 函数名：CompareNamedStatusList
//! @details 函数功能：比较两个命名状态列表是否相等
//!
//! @tparam T 模板类型，可以是ModuleCmdStatus、ModuleInitStatus等
//! @param[in] lhs 第一个命名状态列表
//! @param[in] rhs 第二个命名状态列表
//! @return 如果两个列表相等，返回true，否则返回false
template <class T>
bool CompareNamedStatusList(const std::vector<T>& lhs, const std::vector<T>& rhs) {
  // 如果两个列表的大小不相等，直接返回false
  if (lhs.size() != rhs.size()) return false;
  // 使用哈希表存储第二个列表中的命名状态
  std::unordered_map<EgoTopic, const T*, EgoTopicHash, EgoTopicEqual> compared;
  for (const auto& rst : rhs) compared[GetStatusName(rst)] = &rst;

  // 遍历第一个列表，检查每个命名状态是否在第二个列表中
  for (const auto& lst : lhs) {
    auto it = compared.find(GetStatusName(lst));
    // 如果找不到对应的命名状态或者两个命名状态不相等，返回false
    if (it == compared.end() || lst != *it->second) return false;
  }

  // 如果所有命名状态都相等，返回true
  return true;
}

}  // namespace coordinator
}  // namespace tx_sim
