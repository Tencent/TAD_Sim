// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "status.h"

#include "boost/algorithm/string/join.hpp"
#include "boost/range/adaptor/transformed.hpp"

#include "utils/json_helper.h"
#include "utils/leb128.h"


using namespace tx_sim::impl;
using namespace tx_sim::utils;


namespace tx_sim {
namespace coordinator {

//! @brief 成员函数：operator==
//! @details 成员函数功能：比较两个TopicPubSubInfo对象是否相等
//!
//! @param[in] rhs 另一个TopicPubSubInfo对象
//! @return 如果两个对象相等，返回true，否则返回false
bool TopicPubSubInfo::operator==(const TopicPubSubInfo& rhs) const {
  return CompareNamedStatusList(sub_topics, rhs.sub_topics) && CompareNamedStatusList(pub_topics, rhs.pub_topics) &&
         CompareNamedStatusList(sub_shmems, rhs.sub_shmems) && CompareNamedStatusList(pub_shmems, rhs.pub_shmems);
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
static void EncodeModuleStatus(const ModuleCmdStatus& st, zmq::multipart_t& msg, uint8_t* buf) {
  msg.addstr(st.name);
  AddMsgUInt(buf, st.elapsed_time, msg);
  AddMsgUInt(buf, st.memory, msg);
  AddMsgUInt(buf, st.cpu_time_us, msg);
  AddMsgUInt(buf, st.cpu_time_sy, msg);
  AddMsgUInt(buf, st.fps, msg);
  AddMsgType(buf, st.ec, msg);
  msg.addstr(st.msg);
  msg.addstr(st.feedback);
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
static void DecodeModuleStatus(ModuleCmdStatus& st, zmq::multipart_t& msg) {
  st.name = PopMsgStr(msg);
  st.elapsed_time = static_cast<uint32_t>(PopMsgUint(msg));
  st.memory = static_cast<uint32_t>(PopMsgUint(msg));
  st.cpu_time_us = static_cast<uint32_t>(PopMsgUint(msg));
  st.cpu_time_sy = static_cast<uint32_t>(PopMsgUint(msg));
  st.fps = static_cast<uint16_t>(PopMsgUint(msg));
  st.ec = static_cast<ModuleResponseType>(PopMsgType(msg));
  st.msg = PopMsgStr(msg);
  st.feedback = PopMsgStr(msg);
}

//! @brief 成员函数：operator==
//! @details 成员函数功能：比较两个ModuleCmdStatus对象是否相等
//!
//! @param[in] rhs 另一个ModuleCmdStatus对象
//! @return 如果两个对象相等，返回true，否则返回false
bool ModuleCmdStatus::operator==(const ModuleCmdStatus& rhs) const {
  if (name == rhs.name && ec == rhs.ec && elapsed_time == rhs.elapsed_time && msg == rhs.msg) {
    // some json implementation may appends '\n', ' ', etc. so simply size() == size() not works well.
    if (feedback.empty() && rhs.feedback.empty()) return true;
    if (feedback.empty() != rhs.feedback.empty()) return false;
    std::unordered_map<std::string, std::string> fd1, fd2;
    Json2Map(feedback, fd1);
    Json2Map(rhs.feedback, fd2);
    return fd1 == fd2;
  }
  return false;
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
static void EncodeInitStatus(const ModuleInitStatus& st, zmq::multipart_t& msg, uint8_t* buf) {
  msg.addstr(st.name);
  AddMsgType(buf, st.state, msg);
  // === encoding TopicPubSubInfo ... ===
  AddMsgSize(buf, st.topic_info.sub_topics.size(), msg);
  for (const std::string& t : st.topic_info.sub_topics) msg.addstr(t);
  AddMsgSize(buf, st.topic_info.pub_topics.size(), msg);
  for (const std::string& t : st.topic_info.pub_topics) msg.addstr(t);
  AddMsgSize(buf, st.topic_info.sub_shmems.size(), msg);
  for (const std::string& t : st.topic_info.sub_shmems) msg.addstr(t);
  AddMsgSize(buf, st.topic_info.pub_shmems.size(), msg);
  for (const auto& t : st.topic_info.pub_shmems) {
    std::string topic_name(t.first);
    msg.addstr(topic_name);
    AddMsgSize(buf, t.second, msg);
  }
  // === encoding TopicPubSubInfo finished. ===
  msg.addstr(st.log_file);
  AddMsgType(buf, static_cast<int16_t>(st.binary_updated), msg);
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
static void DecodeInitStatus(ModuleInitStatus& st, zmq::multipart_t& msg) {
  st.name = PopMsgStr(msg);
  st.state = static_cast<ModuleInitState>(PopMsgType(msg));
  // === decoding TopicPubSubInfo ... ===
  size_t count = PopMsgSize(msg);
  for (size_t i = 0; i < count; ++i) st.topic_info.sub_topics.emplace_back(PopMsgStr(msg));
  count = PopMsgSize(msg);
  for (size_t i = 0; i < count; ++i) st.topic_info.pub_topics.emplace_back(PopMsgStr(msg));
  count = PopMsgSize(msg);
  for (size_t i = 0; i < count; ++i) st.topic_info.sub_shmems.emplace_back(PopMsgStr(msg));
  count = PopMsgSize(msg);
  for (size_t i = 0; i < count; ++i) {
    std::string topic_name(PopMsgStr(msg));
    st.topic_info.pub_shmems.emplace_back(topic_name, PopMsgSize(msg));
  }
  // === decoding TopicPubSubInfo finished. ===
  st.log_file = PopMsgStr(msg);
  st.binary_updated = static_cast<bool>(PopMsgType(msg));
}

//! @brief 成员函数：operator==
//! @details 成员函数功能：比较两个ModuleInitStatus对象是否相等
//!
//! @param[in] rhs 另一个ModuleInitStatus对象
//! @return 如果两个对象相等，返回true，否则返回false
bool ModuleInitStatus::operator==(const ModuleInitStatus& rhs) const {
  return name == rhs.name && state == rhs.state && topic_info == topic_info && log_file == rhs.log_file;
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
static void EncodeStepMessage(const StepMessage& st, zmq::multipart_t& msg, uint8_t* buf) {
  AddMsgUInt(buf, st.timestamp, msg);
  AddMsgSize(buf, st.messages.size(), msg);
  for (const std::pair<EgoTopic, zmq::message_t>& item : st.messages) {
    msg.addstr(item.first.sEgoGroup);
    msg.addstr(item.first.sTopic);
    zmq::message_t copied;
    copied.copy(const_cast<zmq::message_t&>(item.second));
    msg.add(std::move(copied));
  }
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
static void DecodeStepMessage(StepMessage& st, zmq::multipart_t& msg) {
  st.timestamp = PopMsgUint(msg);
  size_t count = PopMsgSize(msg);
  st.messages.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    st.messages.emplace_back();
    st.messages.back().first.sEgoGroup = PopMsgStr(msg);
    st.messages.back().first.sTopic = PopMsgStr(msg);
    zmq::message_t t(PopMsg(msg));
    st.messages.back().second.move(t);
  }
}

//! @brief 成员函数：operator==
//! @details 成员函数功能：比较两个StepMessage对象是否相等
//!
//! @param[in] rhs 另一个StepMessage对象
//! @return 如果两个对象相等，返回true，否则返回false
bool StepMessage::operator==(const StepMessage& rhs) const {
  return timestamp == rhs.timestamp && CompareNamedStatusList(messages, rhs.messages);
}

//! @brief 成员函数：operator==
//! @details 成员函数功能：比较两个CommandStatus对象是否相等
//!
//! @param[in] rhs 另一个CommandStatus对象
//! @return 如果两个对象相等，返回true，否则返回false
void CommandStatus::Encode(zmq::multipart_t& msg) const {
  uint8_t buf[10];
  AddMsgType(buf, ec, msg);
  AddMsgUInt(buf, total_time_cost, msg);
  AddMsgSize(buf, module_status.size(), msg);
  for (const ModuleCmdStatus& status : module_status) EncodeModuleStatus(status, msg, buf);
  AddMsgSize(buf, init_status.size(), msg);
  for (const ModuleInitStatus& status : init_status) EncodeInitStatus(status, msg, buf);
  EncodeStepMessage(step_message, msg, buf);
}

//! @brief 成员函数：Decode
//! @details 成员函数功能：从多部分消息中解码命令状态信息
//!
//! @param[in,out] msg 多部分消息
void CommandStatus::Decode(zmq::multipart_t& msg) {
  Clear();
  ec = static_cast<CmdErrorCode>(PopMsgType(msg));
  total_time_cost = static_cast<uint32_t>(PopMsgUint(msg));
  size_t count = PopMsgSize(msg);
  module_status.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    module_status.emplace_back();
    DecodeModuleStatus(module_status.back(), msg);
  }
  count = PopMsgSize(msg);
  init_status.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    init_status.emplace_back();
    DecodeInitStatus(init_status.back(), msg);
  }
  DecodeStepMessage(step_message, msg);
}

//! @brief 成员函数：EncodeRawMsgs
//! @details 成员函数功能：将原始消息编码为字节数组
//!
//! @param[in,out] payload 字节数组
void CommandStatus::EncodeRawMsgs(std::vector<uint8_t>& payload) const {
  size_t total_bytes = 8 + 4;
  for (const auto& kv : step_message.messages)
    total_bytes += (4 + kv.first.sEgoGroup.size() + 4 + kv.first.sTopic.size() + 4 + kv.second.size());
  payload.resize(total_bytes);
  uint32_t msg_cnt = step_message.messages.size(), idx = 0;
  std::memcpy(payload.data() + idx, reinterpret_cast<const uint8_t*>(&step_message.timestamp), 8);
  idx += 8;
  std::memcpy(payload.data() + idx, reinterpret_cast<uint8_t*>(&msg_cnt), 4);
  idx += 4;
  for (const auto& kv : step_message.messages) {
    size_t len = kv.first.sEgoGroup.size();
    std::memcpy(payload.data() + idx, reinterpret_cast<uint8_t*>(&len), 4);
    idx += 4;
    std::memcpy(payload.data() + idx, reinterpret_cast<const uint8_t*>(kv.first.sEgoGroup.c_str()), len);
    idx += len;
    len = kv.first.sTopic.size();
    std::memcpy(payload.data() + idx, reinterpret_cast<uint8_t*>(&len), 4);
    idx += 4;
    std::memcpy(payload.data() + idx, reinterpret_cast<const uint8_t*>(kv.first.sTopic.c_str()), len);
    idx += len;
    len = kv.second.size();
    std::memcpy(payload.data() + idx, reinterpret_cast<uint8_t*>(&len), 4);
    idx += 4;
    std::memcpy(payload.data() + idx, reinterpret_cast<const uint8_t*>(kv.second.data()), len);
    idx += len;
  }
}

//! @brief 成员函数：operator=
//! @details 成员函数功能：赋值操作
//!
//! @param[in] st 另一个CommandStatus对象
//! @return 当前对象的引用
CommandStatus& CommandStatus::operator=(const CommandStatus& st) {
  if (this == &st) return *this;
  ec = st.ec;
  total_time_cost = st.total_time_cost;
  total_cpu_time_us = st.total_cpu_time_us;
  total_cpu_time_sy = st.total_cpu_time_sy;
  module_status = st.module_status;
  init_status = st.init_status;
  step_message.timestamp = st.step_message.timestamp;
  step_message.messages.clear();
  for (const std::pair<EgoTopic, zmq::message_t>& kv : st.step_message.messages) {
    zmq::message_t copied;
    copied.copy(const_cast<zmq::message_t&>(kv.second));
    step_message.messages.emplace_back(kv.first, std::move(copied));
  }
  return *this;
}

//! @brief 成员函数：operator==
//! @details 成员函数功能：比较两个CommandStatus对象是否相等
//!
//! @param[in] rhs 另一个CommandStatus对象
//! @return 如果两个对象相等，返回true，否则返回false
bool CommandStatus::operator==(const CommandStatus& rhs) const {
  return ec == rhs.ec && step_message == rhs.step_message && CompareNamedStatusList(module_status, rhs.module_status) &&
         CompareNamedStatusList(init_status, rhs.init_status);
}

//! @brief 成员函数：Clear
//! @details 成员函数功能：清除命令状态信息
void CommandStatus::Clear() {
  ec = kCmdSucceed;
  total_time_cost = 0;
  module_status.clear();
  init_status.clear();
  step_message.timestamp = 0;
  step_message.messages.clear();
}

//! @brief 成员函数：ApplyErrCode
//! @details 成员函数功能：应用目标错误代码（仅在目标错误代码大于当前错误代码时生效）
//!
//! @param[in] target 目标错误代码
void CommandStatus::ApplyErrCode(CmdErrorCode target) {
  if (ec == CmdErrorCode::kCmdScenarioStopped) { return; }
  if (target > ec) ec = target;
}

//! @brief 成员函数：GetTimeCostStr
//! @details 成员函数功能：获取命令运行时间字符串
//!
//! @return 命令运行时间字符串
std::string CommandStatus::GetTimeCostStr() const {
  char str_buf[256];
  return boost::algorithm::join(
      module_status | boost::adaptors::transformed([&str_buf](const ModuleCmdStatus& m) {
        double us = (double)m.cpu_time_us / 1000, sy = (double)m.cpu_time_sy / 1000;  // in millis
        int len = std::snprintf(str_buf, 256, "%s:%d/%.3f/%.3f", m.name.c_str(), m.elapsed_time, us, sy);
        assert(len >= 0);
        if (len >= 256) {
          std::vector<char> buf(len + 1);
          len = std::snprintf(&buf[0], buf.size(), "%s:%d/%.3f/%.3f", m.name.c_str(), m.elapsed_time, us, sy);
          assert(len >= 0);
          return std::string(buf.begin(), buf.end());
        }
        return std::string(str_buf);
      }),
      ", ");
}


}  // namespace coordinator
}  // namespace tx_sim
