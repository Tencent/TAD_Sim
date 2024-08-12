#pragma once

#include <unordered_map>
#include <unordered_set>

#include "zmq_addon.hpp"

#include "utils/constant.h"

namespace tx_sim {
namespace impl {

struct ModuleMessage {
  virtual void Encode(zmq::multipart_t& msg) const = 0;
  virtual void Decode(zmq::multipart_t& msg) = 0;  // TODO(nemo): a misleading name. more like a unloading op.
};

struct ModuleRequest : public ModuleMessage {
  tx_sim::impl::ModuleRequestType type;
  std::string group;
  ModuleRequest(tx_sim::impl::ModuleRequestType t) : type(t) {}
};

struct ModuleResetResponse;
struct ModuleStepResponse;
struct ModuleStopResponse;

struct ModuleResponse : public ModuleMessage {
  tx_sim::impl::ModuleResponseType type{kModuleOK};
  std::string err;
  ModuleResponse& operator=(const ModuleResetResponse& other);
  ModuleResponse& operator=(const ModuleStepResponse& other);
  ModuleResponse& operator=(const ModuleStopResponse& other);
};

/***************************** init messages **********************************/

struct ModuleInitRequest : public ModuleRequest {
  std::unordered_map<std::string, std::string> init_args;
  ModuleInitRequest();
  bool operator==(const ModuleInitRequest& rhs) const;
  bool operator!=(const ModuleInitRequest& rhs) const { return !(*this == rhs); }
  void Encode(zmq::multipart_t& msg) const override;
  void Decode(zmq::multipart_t& msg) override;
};

struct ModuleInitResponse : public ModuleResponse {
  std::unordered_set<std::string> sub_topics;
  std::unordered_set<std::string> pub_topics;
  std::unordered_set<std::string> sub_shmems;
  std::unordered_map<std::string, size_t> pub_shmems;
  std::unordered_set<std::string> pub_topics_broadcast;
  std::unordered_set<std::string> pub_shmems_broadcast;
  ModuleInitResponse();
  bool operator==(const ModuleInitResponse& rhs) const;
  bool operator!=(const ModuleInitResponse& rhs) const { return !(*this == rhs); }
  void Encode(zmq::multipart_t& msg) const override;
  void Decode(zmq::multipart_t& msg) override;
};

/***************************** reset messages *********************************/
struct SceneActionAssign {
  bool bResumeSwValue = false;
  bool bCancelSwValue = false;
  bool bSpeedIncSwValue = false;
  bool bSpeedDecSwValue = false;
  double dSetTimegapValue = 0.0;
  double dSetSpeedValue = 0.0;
};

struct ModuleEgoInfo {
  ModuleEgoInfo();
  ModuleEgoInfo(const ModuleEgoInfo& other);
  double ego_initial_velocity = 0;
  double ego_initial_theta = 0;
  double ego_speed_limit = 0;
  std::string ego_group_name;
  zmq::message_t initial_location;  // for deprecated ego_start_location API compitability.
  std::vector<std::tuple<double, double, double>> ego_path;
  std::vector<std::pair<int64_t, zmq::message_t>> ego_vehicle_measurement;
  ModuleEgoInfo& operator=(const ModuleEgoInfo& other);
};

struct ModuleResetRequest : public ModuleRequest {
  std::string scenario_path;
  std::string map_path;
  double map_local_origin[3];
  std::vector<ModuleEgoInfo> ego_infos;
  std::vector<std::pair<double, double>> geo_fence;
  zmq::message_t m_scene_pb;
  zmq::message_t m_setting_pb;

  std::string m_labels;
  ModuleResetRequest();
  ModuleResetRequest& operator=(const ModuleResetRequest& other);
  bool operator==(const ModuleResetRequest& rhs) const;
  bool operator!=(const ModuleResetRequest& rhs) const { return !(*this == rhs); }
  void Encode(zmq::multipart_t& msg) const override;
  void Decode(zmq::multipart_t& msg) override;
};

struct ModuleResetResponse : public ModuleResponse {
  ModuleResetResponse();
  ModuleResetResponse(const ModuleResetResponse& other) { *this = other; }
  ModuleResetResponse& operator=(const ModuleResetResponse& other);
  bool operator==(const ModuleResetResponse& rhs) const;
  bool operator!=(const ModuleResetResponse& rhs) const { return !(*this == rhs); }
  void Encode(zmq::multipart_t& msg) const override;
  void Decode(zmq::multipart_t& msg) override;
};

/***************************** step messages **********************************/

struct ModuleStepRequest : public ModuleRequest {
  uint64_t sim_time = 0;
  double cur_time = 0;
  std::unordered_map<std::string, zmq::message_t> messages;
  std::unordered_map<std::string, std::string> sub_topic_shmem_names;
  std::unordered_map<std::string, std::string> pub_topic_shmem_names;
  ModuleStepRequest();
  bool PublishesShmem(const std::string& topic) {
    return pub_topic_shmem_names.find(topic) != pub_topic_shmem_names.end();
  }

  bool operator==(const ModuleStepRequest& rhs) const;
  bool operator!=(const ModuleStepRequest& rhs) const { return !(*this == rhs); }
  void Encode(zmq::multipart_t& msg) const override;
  void Decode(zmq::multipart_t& msg) override;
};

struct ModuleStepResponse : public ModuleResponse {
  uint32_t time_cost = 0;
  uint64_t cpu_time_us = 0, cpu_time_sy = 0;
  std::unordered_map<std::string, zmq::message_t> messages;
  ModuleStepResponse();
  ModuleStepResponse(const ModuleStepResponse& other) { *this = other; }
  ModuleStepResponse& operator=(const ModuleStepResponse& other);
  bool operator==(const ModuleStepResponse& rhs) const;
  bool operator!=(const ModuleStepResponse& rhs) const { return !(*this == rhs); }
  void Encode(zmq::multipart_t& msg) const override;
  void Decode(zmq::multipart_t& msg) override;
};

/***************************** stop messages **********************************/
struct GradingPerfStats {
  uint64_t step_count = 0;      // step count
  uint64_t execute_period = 0;  // step period
  uint64_t ntime_out = 0;
  double total_step_real_time = 0;  // total step real time
  float per_step_time = 0.0;        // per step time
};

struct ModuleStopRequest : public ModuleRequest {
  ModuleStopRequest();
  std::string m_sGradingPerfStats;
  bool operator==(const ModuleStopRequest& rhs) const;
  bool operator!=(const ModuleStopRequest& rhs) const { return !(*this == rhs); }
  void Encode(zmq::multipart_t& msg) const override;
  void Decode(zmq::multipart_t& msg) override;
};

struct ModuleStopResponse : public ModuleResponse {
  std::unordered_map<std::string, std::string> feedbacks;
  ModuleStopResponse();
  ModuleStopResponse(const ModuleStopResponse& other) { *this = other; }
  ModuleStopResponse& operator=(const ModuleStopResponse& other);
  bool operator==(const ModuleStopResponse& rhs) const;
  bool operator!=(const ModuleStopResponse& rhs) const { return !(*this == rhs); }
  void Encode(zmq::multipart_t& msg) const override;
  void Decode(zmq::multipart_t& msg) override;
};

}  // namespace impl
}  // namespace tx_sim
