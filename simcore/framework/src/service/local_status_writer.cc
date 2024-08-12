// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "local_status_writer.h"

#include <chrono>
#include <utility>

#include "glog/logging.h"

#include "server_impl/ws_session.h"
#include "utils/json_helper.h"

namespace tx_sim {

namespace coordinator {  // TODO(nemo): temporarily implementing encoding here(instead of in the CommandStatus class)
                         //             since we could not brought jsoncpp dependencies into the txsim-client project.

/**
 * @brief Encodes the status of a module into a JSON value.
 * @param[in] mcs The ModuleCmdStatus object containing the status information.
 * @param[out] msg The Json::Value object to which the status information is encoded.
 */
static void EncodeModuleStatus(const ModuleCmdStatus& mcs, Json::Value& msg) {
  msg["name"] = mcs.name;
  msg["elapsedTime"] = mcs.elapsed_time;
  msg["memoryUsed"] = mcs.memory;
  msg["cpuTimeUser"] = mcs.cpu_time_us;
  msg["cpuTimeSys"] = mcs.cpu_time_sy;
  msg["fps"] = mcs.fps;
  msg["ec"] = mcs.ec;
  msg["msg"] = mcs.msg;
  msg["feedback"] = mcs.feedback;
}

/**
 * @brief Encodes the status of a init module into a JSON value.
 * @param[in] mis The ModuleInitStatus object containing the init status information.
 * @param[out] msg The Json::Value object to which the status information is encoded.
 */
static void EncodeInitStatus(const ModuleInitStatus& mis, Json::Value& msg) {
  msg["name"] = mis.name;
  msg["state"] = mis.state;
  auto& ti = msg["topicInfo"];
  ti = Json::objectValue;
  if (!mis.topic_info.sub_topics.empty()) {
    auto& ti_sub = ti["subs"];
    for (const auto& t : mis.topic_info.sub_topics) ti_sub.append(Json::Value(t));
  }
  if (!mis.topic_info.pub_topics.empty()) {
    auto& ti_pub = ti["pubs"];
    for (const auto& t : mis.topic_info.pub_topics) ti_pub.append(Json::Value(t));
  }
  if (!mis.topic_info.sub_shmems.empty()) {
    auto& ti_sub = ti["shmemSubs"];
    for (const auto& t : mis.topic_info.sub_shmems) ti_sub.append(Json::Value(t));
  }
  if (!mis.topic_info.pub_shmems.empty()) {
    auto& ti_pub = ti["shmemPubs"];
    for (const auto& t : mis.topic_info.pub_shmems) {
      auto& p = ti_pub.append(Json::Value());
      p["name"] = t.first;
      p["size"] = t.second;
    }
  }
  msg["logPath"] = mis.log_file;
  msg["binaryUpdated"] = mis.binary_updated;
}

/**
 * @brief Encodes the step message into a JSON value.
 * @param[in] sm The StepMessage object containing the step status information.
 * @param[out] msg The Json::Value object to which the status information is encoded.
 */
static void EncodeStepMessage(const StepMessage& sm, Json::Value& msg) {
  msg["timestamp"] = sm.timestamp;
  Json::Value& m_list = msg["msgInfo"];
  m_list = Json::arrayValue;
  for (const auto& m : sm.messages) {
    Json::Value& i = m_list.append(Json::Value());
    // i["egogroup"] = m.first.sEgoGroup;
    i["topic"] = m.first.sTopic;
    i["size"] = m.second.size();
  }
}

/**
 * @brief Encodes the cmd status into a JSON value.
 * @param[in] st The CommandStatus object containing the command status information.
 * @param[out] json_str The std::string to which the status information is encoded.
 */
static void EncodeCmdStatus2JsonStr(const CommandStatus& st, std::string& json_str) {
  Json::Value msg;
  msg["ec"] = st.ec;
  msg["totalTimeCost"] = st.total_time_cost;
  msg["totalCpuTimeUser"] = st.total_cpu_time_us;
  msg["totalCpuTimeSys"] = st.total_cpu_time_sy;
  if (!st.module_status.empty()) {
    Json::Value& m_node = msg["moduleStatus"];
    m_node = Json::arrayValue;
    for (const auto& m : st.module_status) EncodeModuleStatus(m, m_node.append(Json::Value()));
  }
  if (!st.init_status.empty()) {
    Json::Value& i_node = msg["initStatus"];
    i_node = Json::arrayValue;
    for (const auto& i : st.init_status) EncodeInitStatus(i, i_node.append(Json::Value()));
  }
  EncodeStepMessage(st.step_message, msg["stepMessage"]);
  tx_sim::utils::WriteJsonDocToString(msg, json_str);
}

}  // namespace coordinator

namespace service {

SimLocalStatusWriter::SimLocalStatusWriter() {
  // create a new thread to consume the status messages
  consumer_.reset(new std::thread(&SimLocalStatusWriter::Consume, this));
}

SimLocalStatusWriter::~SimLocalStatusWriter() {
  // set the running flag to false
  running_.store(false, std::memory_order_release);
  // stop the consumer thread
  if (consumer_ && consumer_->joinable()) consumer_->join();
}

void SimLocalStatusWriter::Write(const tx_sim::coordinator::CommandStatus& st) {
  if (!queue_.try_enqueue(st)) {
    VLOG_EVERY_N(0, 10) << "[WARNING] status msg producing too fast. status-monitoring websocket may lose msgs.";
  }
}

void SimLocalStatusWriter::Join(WsSession* s, bool push_raw_msg) {
  std::lock_guard<std::mutex> lk(mtx_);
  sessions_.insert({s, push_raw_msg});
}

void SimLocalStatusWriter::Leave(WsSession* s) {
  std::lock_guard<std::mutex> lk(mtx_);
  sessions_.erase(s);
}

void SimLocalStatusWriter::Consume() {
  LOG(INFO) << "local status writer start consuming sim status msgs ...";

  tx_sim::coordinator::CommandStatus cmd_st;
  // while the status writer is running
  while (running_.load(std::memory_order_acquire)) {
    // wait for a message to be enqueued
    if (!queue_.wait_dequeue_timed(cmd_st, std::chrono::seconds(1))) continue;

    std::vector<std::pair<boost::weak_ptr<WsSession>, bool>> ss;
    {
      std::lock_guard<std::mutex> lk(mtx_);
      // copy the sessions to a vector
      ss.reserve(sessions_.size());
      // WsSession pointers may be expired, so we use weak_ptr to avoid memory leak
      for (auto p : sessions_) ss.emplace_back(p.first->weak_from_this(), p.second);
    }

    std::shared_ptr<std::string> status;
    std::shared_ptr<std::vector<uint8_t>> msgs;
    for (const auto& wp : ss)
      // check if the session is still alive
      if (auto sp = wp.first.lock()) {
        if (wp.second) {  // raw msgs
          if (!msgs) {
            msgs.reset(new std::vector<uint8_t>);
            cmd_st.EncodeRawMsgs(*msgs);
          }
          sp->SendMsgs(msgs);
        } else {  // status
          if (!status) {
            status.reset(new std::string);
            tx_sim::coordinator::EncodeCmdStatus2JsonStr(cmd_st, *status);
          }
          sp->SendStatus(status);
        }
      }
  }

  LOG(INFO) << "local status writer consuming exit.";
}

}  // namespace service
}  // namespace tx_sim
