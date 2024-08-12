#include "msgs.h"

#include "utils/leb128.h"


using namespace tx_sim::utils;


namespace tx_sim {
namespace impl {

// below three assign ops only apply for corresponding type lhs, otherwise it should be treated as a programing error
// and crash the program(acts like assertion failure) instead of silence/throws.

ModuleResponse& ModuleResponse::operator=(const ModuleResetResponse& other) {
  return dynamic_cast<ModuleResponse&>(*dynamic_cast<ModuleResetResponse*>(this) = other);
}

ModuleResponse& ModuleResponse::operator=(const ModuleStepResponse& other) {
  return dynamic_cast<ModuleResponse&>(*dynamic_cast<ModuleStepResponse*>(this) = other);
}

ModuleResponse& ModuleResponse::operator=(const ModuleStopResponse& other) {
  return dynamic_cast<ModuleResponse&>(*dynamic_cast<ModuleStopResponse*>(this) = other);
}

/******************************************************************************/
/****************** message encode/decode functions ***************************/
/******************************************************************************/

/***************************** init messages **********************************/

ModuleInitRequest::ModuleInitRequest() : ModuleRequest(tx_sim::impl::kModuleInit) {}

bool ModuleInitRequest::operator==(const ModuleInitRequest& rhs) const {
  return type == rhs.type && init_args == rhs.init_args;
}

void ModuleInitRequest::Encode(zmq::multipart_t& msg) const {
  msg.clear();
  uint8_t buf[10];
  AddMsgType(buf, type, msg);
  AddMsgSize(buf, init_args.size(), msg);
  for (const std::pair<std::string, std::string>& kv : init_args) {
    msg.addstr(kv.first);
    msg.addstr(kv.second);
  }
  std::string fixed_group = group;
  if (fixed_group.size() == 0) { fixed_group = std::string("none"); }
  msg.addstr(fixed_group);
}

void ModuleInitRequest::Decode(zmq::multipart_t& msg) {
  type = static_cast<ModuleRequestType>(PopMsgType(msg));
  size_t len = PopMsgSize(msg);
  init_args.clear();
  for (size_t i = 0; i < len; ++i) {
    std::string key(PopMsgStr(msg));  // ensure key is popped first.
    init_args[key] = PopMsgStr(msg);
  }
  group = PopMsgStr(msg);
  if (group == "none") group = "";
}

ModuleInitResponse::ModuleInitResponse() {}

bool ModuleInitResponse::operator==(const ModuleInitResponse& rhs) const {
  return type == rhs.type && err == rhs.err && sub_topics == rhs.sub_topics && pub_topics == rhs.pub_topics &&
         sub_shmems == rhs.sub_shmems && pub_shmems == rhs.pub_shmems;
}

void ModuleInitResponse::Encode(zmq::multipart_t& msg) const {
  msg.clear();
  uint8_t buf[10];
  AddMsgType(buf, type, msg);
  msg.addstr(err);
  AddMsgSize(buf, sub_topics.size(), msg);
  for (const std::string& t : sub_topics) msg.addstr(t);
  AddMsgSize(buf, pub_topics.size(), msg);
  for (const std::string& t : pub_topics) msg.addstr(t);
  AddMsgSize(buf, sub_shmems.size(), msg);
  for (const std::string& t : sub_shmems) msg.addstr(t);
  AddMsgSize(buf, pub_shmems.size(), msg);
  for (const auto& ts : pub_shmems) {
    msg.addstr(ts.first);
    AddMsgSize(buf, ts.second, msg);
  }
  AddMsgSize(buf, pub_topics_broadcast.size(), msg);
  for (const auto& ts : pub_topics_broadcast) { msg.addstr(ts); }
  AddMsgSize(buf, pub_shmems_broadcast.size(), msg);
  for (const auto& ts : pub_shmems_broadcast) { msg.addstr(ts); }
}

void ModuleInitResponse::Decode(zmq::multipart_t& msg) {
  type = static_cast<ModuleResponseType>(PopMsgType(msg));
  err = PopMsgStr(msg);
  sub_topics.clear();
  size_t len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) sub_topics.insert(PopMsgStr(msg));
  pub_topics.clear();
  len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) pub_topics.insert(PopMsgStr(msg));
  len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) sub_shmems.insert(PopMsgStr(msg));
  len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) {
    std::string t = PopMsgStr(msg);
    pub_shmems[t] = PopMsgSize(msg);
  }

  len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) {
    std::string t = PopMsgStr(msg);
    pub_topics_broadcast.insert(t);
  }
  len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) {
    std::string t = PopMsgStr(msg);
    pub_shmems_broadcast.insert(t);
  }
}


/***************************** reset messages *********************************/

ModuleEgoInfo::ModuleEgoInfo() {}

ModuleEgoInfo::ModuleEgoInfo(const ModuleEgoInfo& other) {
  ego_path = other.ego_path;
  ego_initial_velocity = other.ego_initial_velocity;
  ego_initial_theta = other.ego_initial_theta;
  ego_speed_limit = other.ego_speed_limit;
  ego_group_name = other.ego_group_name;
  initial_location.copy(const_cast<zmq::message_t&>(other.initial_location));
  for (auto& item : other.ego_vehicle_measurement) {
    zmq::message_t copied;
    copied.copy(const_cast<zmq::message_t&>(item.second));
    ego_vehicle_measurement.emplace_back();
    ego_vehicle_measurement.back().first = item.first;
    std::get<1>(ego_vehicle_measurement.back()) = std::move(copied);
  }
}

ModuleEgoInfo& ModuleEgoInfo::operator=(const ModuleEgoInfo& other) {
  ego_path = other.ego_path;
  ego_initial_velocity = other.ego_initial_velocity;
  ego_initial_theta = other.ego_initial_theta;
  ego_speed_limit = other.ego_speed_limit;
  ego_group_name = other.ego_group_name;
  initial_location.copy(const_cast<zmq::message_t&>(other.initial_location));
  for (auto& item : other.ego_vehicle_measurement) {
    zmq::message_t copied;
    copied.copy(const_cast<zmq::message_t&>(item.second));
    ego_vehicle_measurement.emplace_back();
    ego_vehicle_measurement.back().first = item.first;
    std::get<1>(ego_vehicle_measurement.back()) = std::move(copied);
  }
  return *this;
}

ModuleResetRequest::ModuleResetRequest() : ModuleRequest(tx_sim::impl::kModuleReset) {}

ModuleResetRequest& ModuleResetRequest::operator=(const ModuleResetRequest& other) {
  scenario_path = other.scenario_path;
  map_path = other.map_path;
  for (size_t i = 0; i < 3; ++i) map_local_origin[i] = other.map_local_origin[i];
  geo_fence = other.geo_fence;

  m_scene_pb.copy(*const_cast<zmq::message_t*>(&other.m_scene_pb));
  m_setting_pb.copy(*const_cast<zmq::message_t*>(&other.m_setting_pb));
  ego_infos.insert(ego_infos.end(), other.ego_infos.begin(), other.ego_infos.end());

  m_labels = other.m_labels;
  return *this;
}

bool ModuleResetRequest::operator==(const ModuleResetRequest& rhs) const {
  bool result = (type == rhs.type && scenario_path == rhs.scenario_path && map_path == rhs.map_path &&
                 0 == std::memcmp(map_local_origin, rhs.map_local_origin, sizeof map_local_origin) &&
                 geo_fence == rhs.geo_fence && m_scene_pb == rhs.m_scene_pb && m_setting_pb == rhs.m_setting_pb &&
                 m_labels == rhs.m_labels);
  if (ego_infos.size() == rhs.ego_infos.size()) {
    for (int i = 0; i < ego_infos.size(); ++i) {
      result = result && (ego_infos[i].ego_path == rhs.ego_infos[i].ego_path &&
                          ego_infos[i].ego_initial_velocity == rhs.ego_infos[i].ego_initial_velocity &&
                          ego_infos[i].ego_initial_theta == rhs.ego_infos[i].ego_initial_theta &&
                          ego_infos[i].ego_speed_limit == rhs.ego_infos[i].ego_speed_limit);
    }
  } else {
    result = false;
  }
  return result;
}

void ModuleResetRequest::Encode(zmq::multipart_t& msg) const {
  msg.clear();
  uint8_t buf[10];
  AddMsgType(buf, type, msg);
  msg.addstr(scenario_path);
  msg.addstr(map_path);
  AddMsgDouble(buf, map_local_origin[0], msg);
  AddMsgDouble(buf, map_local_origin[1], msg);
  AddMsgDouble(buf, map_local_origin[2], msg);

  AddMsgSize(buf, ego_infos.size(), msg);
  for (const auto& egoinfo : ego_infos) {
    AddMsgSize(buf, egoinfo.ego_path.size(), msg);
    for (const auto& point : egoinfo.ego_path) {
      AddMsgDouble(buf, std::get<0>(point), msg);
      AddMsgDouble(buf, std::get<1>(point), msg);
      AddMsgDouble(buf, std::get<2>(point), msg);
    }
    AddMsgDouble(buf, egoinfo.ego_initial_velocity, msg);
    AddMsgDouble(buf, egoinfo.ego_initial_theta, msg);
    AddMsgDouble(buf, egoinfo.ego_speed_limit, msg);
    msg.addstr(egoinfo.ego_group_name);
    zmq::message_t copied;
    copied.copy(const_cast<zmq::message_t&>(egoinfo.initial_location));
    msg.add(std::move(copied));
  }

  AddMsgSize(buf, geo_fence.size(), msg);
  for (const auto& point : geo_fence) {
    AddMsgDouble(buf, point.first, msg);
    AddMsgDouble(buf, point.second, msg);
  }
  zmq::message_t copied;
  for (const auto& egoinfo : ego_infos) {
    AddMsgSize(buf, egoinfo.ego_vehicle_measurement.size(), msg);
    for (const auto& m : egoinfo.ego_vehicle_measurement) {
      AddMsgInt64(buf, m.first, msg);
      copied.copy(const_cast<zmq::message_t&>(m.second));
      msg.add(std::move(copied));
    }
  }

  copied.copy(const_cast<zmq::message_t&>(m_scene_pb));
  msg.add(std::move(copied));

  copied.copy(const_cast<zmq::message_t&>(m_setting_pb));
  msg.add(std::move(copied));

  msg.addstr(group);
}


void ModuleResetRequest::Decode(zmq::multipart_t& msg) {
  type = static_cast<ModuleRequestType>(PopMsgType(msg));
  scenario_path = PopMsgStr(msg);
  map_path = PopMsgStr(msg);
  map_local_origin[0] = PopMsgDouble(msg);
  map_local_origin[1] = PopMsgDouble(msg);
  map_local_origin[2] = PopMsgDouble(msg);

  size_t ego_size = PopMsgSize(msg);
  for (size_t e = 0; e < ego_size; ++e) {
    ego_infos.emplace_back();
    ego_infos[e].ego_path.clear();
    size_t len1 = PopMsgSize(msg);
    for (size_t i = 0; i < len1; ++i) {
      double x = PopMsgDouble(msg);
      double y = PopMsgDouble(msg);
      double z = PopMsgDouble(msg);
      ego_infos[e].ego_path.emplace_back(x, y, z);
    }
    ego_infos[e].ego_initial_velocity = PopMsgDouble(msg);
    ego_infos[e].ego_initial_theta = PopMsgDouble(msg);
    ego_infos[e].ego_speed_limit = PopMsgDouble(msg);

    ego_infos[e].ego_group_name = PopMsgStr(msg);
    ego_infos[e].initial_location = PopMsg(msg);
  }

  geo_fence.clear();
  size_t len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) {
    double x = PopMsgDouble(msg);
    double y = PopMsgDouble(msg);
    geo_fence.emplace_back(x, y);
  }

  for (size_t e = 0; e < ego_size; ++e) {
    size_t len1 = PopMsgSize(msg);
    for (size_t i = 0; i < len1; ++i) {
      ego_infos[e].ego_vehicle_measurement.emplace_back();
      ego_infos[e].ego_vehicle_measurement.back().first = PopMsgInt64(msg);
      std::get<1>(ego_infos[e].ego_vehicle_measurement.back()) = std::move(PopMsg(msg));
    }
  }
  m_scene_pb = PopMsg(msg);
  m_setting_pb = PopMsg(msg);
  group = PopMsgStr(msg);
}

ModuleResetResponse::ModuleResetResponse() {}

ModuleResetResponse& ModuleResetResponse::operator=(const ModuleResetResponse& other) {
  if (this == &other) return *this;
  type = other.type;
  err = other.err;
  return *this;
}

bool ModuleResetResponse::operator==(const ModuleResetResponse& rhs) const {
  return type == rhs.type && err == rhs.err;
}

void ModuleResetResponse::Encode(zmq::multipart_t& msg) const {
  msg.clear();
  uint8_t buf[10];
  AddMsgType(buf, type, msg);
  msg.addstr(err);
}

void ModuleResetResponse::Decode(zmq::multipart_t& msg) {
  type = static_cast<ModuleResponseType>(PopMsgType(msg));
  err = PopMsgStr(msg);
}


/***************************** step messages **********************************/

ModuleStepRequest::ModuleStepRequest() : ModuleRequest(tx_sim::impl::kModuleStep) {}

bool ModuleStepRequest::operator==(const ModuleStepRequest& rhs) const {
  return type == rhs.type && sim_time == rhs.sim_time && messages == rhs.messages &&
         sub_topic_shmem_names == rhs.sub_topic_shmem_names && pub_topic_shmem_names == rhs.pub_topic_shmem_names;
}

void ModuleStepRequest::Encode(zmq::multipart_t& msg) const {
  msg.clear();
  uint8_t buf[10];
  AddMsgType(buf, type, msg);
  AddMsgUInt(buf, sim_time, msg);
  AddMsgDouble(buf, cur_time, msg);
  AddMsgSize(buf, messages.size(), msg);
  for (const auto& kv : messages) {
    msg.addstr(kv.first);
    zmq::message_t copied;
    copied.copy(const_cast<zmq::message_t&>(kv.second));
    msg.add(std::move(copied));
  }
  AddMsgSize(buf, sub_topic_shmem_names.size(), msg);
  for (const auto& kv : sub_topic_shmem_names) {
    msg.addstr(kv.first);
    msg.addstr(kv.second);
  }
  AddMsgSize(buf, pub_topic_shmem_names.size(), msg);
  for (const auto& kv : pub_topic_shmem_names) {
    msg.addstr(kv.first);
    msg.addstr(kv.second);
  }
  std::string fixed_group = group;
  if (fixed_group.size() == 0) { fixed_group = std::string("none"); }
  msg.addstr(fixed_group);
}

void ModuleStepRequest::Decode(zmq::multipart_t& msg) {
  type = static_cast<ModuleRequestType>(PopMsgType(msg));
  sim_time = PopMsgUint(msg);
  cur_time = PopMsgDouble(msg);
  messages.clear();
  size_t len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) {
    std::string topic = PopMsgStr(msg);
    messages[topic] = PopMsg(msg);
  }
  len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) {
    std::string topic = PopMsgStr(msg);
    sub_topic_shmem_names[topic] = PopMsgStr(msg);
  }
  len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) {
    std::string topic = PopMsgStr(msg);
    pub_topic_shmem_names[topic] = PopMsgStr(msg);
  }
  group = PopMsgStr(msg);
  if (group == "none") group = "";
}

ModuleStepResponse::ModuleStepResponse() {}

ModuleStepResponse& ModuleStepResponse::operator=(const ModuleStepResponse& other) {
  if (this == &other) return *this;
  type = other.type;
  err = other.err;
  time_cost = other.time_cost;
  cpu_time_us = other.cpu_time_us;
  cpu_time_sy = other.cpu_time_sy;
  messages.clear();
  for (const auto& item : other.messages) messages[item.first].copy(const_cast<zmq::message_t&>(item.second));
  return *this;
}

bool ModuleStepResponse::operator==(const ModuleStepResponse& rhs) const {
  return type == rhs.type && err == rhs.err && time_cost == rhs.time_cost && cpu_time_us == rhs.cpu_time_us &&
         cpu_time_sy == rhs.cpu_time_sy && messages == rhs.messages;
}

void ModuleStepResponse::Encode(zmq::multipart_t& msg) const {
  msg.clear();
  uint8_t buf[10];
  AddMsgType(buf, type, msg);
  msg.addstr(err);
  AddMsgUInt(buf, time_cost, msg);
  AddMsgUInt(buf, cpu_time_us, msg);
  AddMsgUInt(buf, cpu_time_sy, msg);
  AddMsgSize(buf, messages.size(), msg);
  for (const auto& kv : messages) {
    msg.addstr(kv.first);
    zmq::message_t copied;
    copied.copy(const_cast<zmq::message_t&>(kv.second));
    std::string msg_payload;
    const char* msg_buf = static_cast<const char*>(copied.data());
    uint32_t len = copied.size();
    msg_payload.assign(msg_buf, len);
    msg.add(std::move(copied));
  }
}

void ModuleStepResponse::Decode(zmq::multipart_t& msg) {
  type = static_cast<ModuleResponseType>(PopMsgType(msg));
  err = PopMsgStr(msg);
  time_cost = PopMsgUint(msg);
  cpu_time_us = PopMsgUint(msg);
  cpu_time_sy = PopMsgUint(msg);
  messages.clear();
  size_t len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) {
    std::string topic = PopMsgStr(msg);
    messages[topic] = PopMsg(msg);
    std::string msg_payload;
    const char* msg = static_cast<const char*>(messages[topic].data());
    uint32_t len1 = messages[topic].size();
    msg_payload.assign(msg, len1);
  }
}


/***************************** stop messages **********************************/

ModuleStopRequest::ModuleStopRequest() : ModuleRequest(tx_sim::impl::kModuleStop) {}

bool ModuleStopRequest::operator==(const ModuleStopRequest& rhs) const {
  return type == rhs.type;
}

void ModuleStopRequest::Encode(zmq::multipart_t& msg) const {
  msg.clear();
  uint8_t buf[10];
  AddMsgType(buf, type, msg);
  msg.addstr(m_sGradingPerfStats);
}

void ModuleStopRequest::Decode(zmq::multipart_t& msg) {
  type = static_cast<ModuleRequestType>(PopMsgType(msg));
  m_sGradingPerfStats = PopMsgStr(msg);
}

ModuleStopResponse::ModuleStopResponse() {}

ModuleStopResponse& ModuleStopResponse::operator=(const ModuleStopResponse& other) {
  if (this == &other) return *this;
  type = other.type;
  err = other.err;
  feedbacks = other.feedbacks;
  return *this;
}

bool ModuleStopResponse::operator==(const ModuleStopResponse& rhs) const {
  return type == rhs.type && err == rhs.err && feedbacks == rhs.feedbacks;
}

void ModuleStopResponse::Encode(zmq::multipart_t& msg) const {
  msg.clear();
  uint8_t buf[10];
  AddMsgType(buf, type, msg);
  msg.addstr(err);
  AddMsgSize(buf, feedbacks.size(), msg);
  for (const auto& fb : feedbacks) {
    msg.addstr(fb.first);
    msg.addstr(fb.second);
  }
}

void ModuleStopResponse::Decode(zmq::multipart_t& msg) {
  type = static_cast<ModuleResponseType>(PopMsgType(msg));
  err = PopMsgStr(msg);
  feedbacks.clear();
  size_t len = PopMsgSize(msg);
  for (size_t i = 0; i < len; ++i) {
    std::string topic = PopMsgStr(msg);
    feedbacks[topic] = PopMsgStr(msg);
  }
}

}  // namespace impl
}  // namespace tx_sim