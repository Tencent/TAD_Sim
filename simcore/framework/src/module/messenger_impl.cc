// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "messenger_impl.h"


static const std::string kEmptyString;


extern "C" {

void txsim_set_module_error(void* data, const char* err) {
  static_cast<tx_sim::impl::ModuleData*>(data)->set_err(err);
}

/******************************************************************************/
/************************* Init API Implementation ****************************/
/******************************************************************************/

const char* txsim_init_parameter(void* data, const char* key) {
  const std::unordered_map<std::string, std::string>& args =
      static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleInitRequest>()->init_args;
  auto it = args.find(key);
  return it == args.cend() ? kEmptyString.c_str() : it->second.c_str();
}

void txsim_subscribe(void* data, const char* topic) {
  static_cast<tx_sim::impl::ModuleData*>(data)->resp<tx_sim::impl::ModuleInitResponse>()->sub_topics.insert(topic);
}

void txsim_publish(void* data, const char* topic) {
  auto& module_data = *static_cast<tx_sim::impl::ModuleData*>(data);
  module_data.resp<tx_sim::impl::ModuleInitResponse>()->pub_topics.insert(topic);
}

void txsim_subscribe_shmem(void* data, const char* topic) {
  static_cast<tx_sim::impl::ModuleData*>(data)->resp<tx_sim::impl::ModuleInitResponse>()->sub_shmems.insert(topic);
}

void txsim_publish_shmem(void* data, const char* topic, size_t max_size) {
  static_cast<tx_sim::impl::ModuleData*>(data)->resp<tx_sim::impl::ModuleInitResponse>()->pub_shmems[topic] = max_size;
}

/*
void txsim_subscribe_group(void* data, const char* topic) {
    auto& module_data = *static_cast<tx_sim::impl::ModuleData*>(data);
  const std::string group = module_data.req<tx_sim::impl::ModuleInitRequest>()->group;
  std::string grouped_topic = group.length() > 0 ? group + "/" + topic : topic;
  module_data.resp<tx_sim::impl::ModuleInitResponse>()->sub_topics.insert(grouped_topic);
}

void txsim_publish_group(void* data, const char* topic, bool broadcast) {
  std::string topic_str(topic);
  auto& module_data = *static_cast<tx_sim::impl::ModuleData*>(data);
  const std::string group = module_data.req<tx_sim::impl::ModuleInitRequest>()->group;
  if (group.size()>0) {
    std::string grouped_topic = group + "/" + topic_str;
    module_data.resp<tx_sim::impl::ModuleInitResponse>()->pub_topics.insert(grouped_topic);
    if (broadcast)
        module_data.resp<tx_sim::impl::ModuleInitResponse>()->pub_topics_broadcast.insert(grouped_topic);
  } else {
    module_data.resp<tx_sim::impl::ModuleInitResponse>()->pub_topics.insert(topic_str);
  }
}

void txsim_subscribe_shmem_group(void* data, const char* topic) {
  auto& module_data = *static_cast<tx_sim::impl::ModuleData*>(data);
  const std::string group = module_data.req<tx_sim::impl::ModuleInitRequest>()->group;
  std::string grouped_topic = group.length() > 0 ? group + "/" + topic : topic;
  module_data.resp<tx_sim::impl::ModuleInitResponse>()->sub_shmems.insert(grouped_topic);
}

void txsim_publish_shmem_group(void* data, const char* topic, size_t max_size, bool broadcast) {
    std::string topic_str(topic);
    auto& module_data = *static_cast<tx_sim::impl::ModuleData*>(data);
  const std::string group = module_data.req<tx_sim::impl::ModuleInitRequest>()->group;
  if (group.size() > 0) {
    std::string grouped_topic = group + "/" + topic_str;
    module_data.resp<tx_sim::impl::ModuleInitResponse>()->pub_shmems[grouped_topic] = max_size;
    if (broadcast)
        module_data.resp<tx_sim::impl::ModuleInitResponse>()->pub_shmems_broadcast.insert(grouped_topic);
  } else {
    module_data.resp<tx_sim::impl::ModuleInitResponse>()->pub_shmems[topic_str] = max_size;
  }
}
*/

/******************************************************************************/
/************************* Reset API Implementation ***************************/
/******************************************************************************/

const char* txsim_map_file_path(void* data) {
  return static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->map_path.c_str();
}

void txsim_map_local_origin(void* data, double* lon, double* lat, double* alt) {
  const double* orig =
      static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->map_local_origin;
  *lon = orig[0];
  *lat = orig[1];
  *alt = orig[2];
}

void txsim_ego_destination(void* data, double* lon, double* lat, double* alt) {
  std::string sGroup = static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->group;
  for (const auto& egoItem :
       static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->ego_infos) {
    if (egoItem.ego_group_name == sGroup) {
      const std::vector<std::tuple<double, double, double>>& path = egoItem.ego_path;
      *lon = std::get<0>(path.back());
      *lat = std::get<1>(path.back());
      *alt = std::get<2>(path.back());
      return;
    }
  }
}

double txsim_ego_speed_limit(void* data) {
  std::string sGroup = static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->group;
  for (const auto& egoItem :
       static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->ego_infos) {
    if (egoItem.ego_group_name == sGroup) { return egoItem.ego_speed_limit; }
  }
  return 0.0;
}

uint32_t txsim_location_raw_message(void* data, const char** raw) {
  std::string sGroup = static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->group;
  for (const auto& egoItem :
       static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->ego_infos) {
    if (egoItem.ego_group_name == sGroup) {
      const zmq::message_t& m = egoItem.initial_location;
      *raw = static_cast<const char*>(m.data());
      return m.size();
    }
  }
  return 0;
}

uint32_t txsim_geo_fence(void* data, double* point_array) {
  std::vector<std::pair<double, double>>& gs =
      static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->geo_fence;
  if (point_array != nullptr) {
    size_t idx = 0;
    for (const std::pair<double, double>& g : gs) {
      point_array[idx++] = g.first;
      point_array[idx++] = g.second;
    }
  }
  return gs.size();
}

const char* txsim_scenario_file_path(void* data) {
  return static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->scenario_path.c_str();
}

uint32_t txsim_ego_path(void* data, double* point_array) {
  std::string sGroup = static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->group;
  for (const auto& egoItem :
       static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->ego_infos) {
    if (egoItem.ego_group_name == sGroup) {
      const std::vector<std::tuple<double, double, double>>& path = egoItem.ego_path;
      if (point_array != nullptr) {
        size_t idx = 0;
        for (const std::tuple<double, double, double>& p : path) {
          point_array[idx++] = std::get<0>(p);
          point_array[idx++] = std::get<1>(p);
          point_array[idx++] = std::get<2>(p);
        }
      }
      return path.size();
    }
  }
  return 0;
}

uint32_t txsim_vehicle_measurements(void* data, int64_t* idx, const char** raw) {
  int nEgoSize = 0;
  std::string sGroup = static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->group;
  printf("sGroup: %s\n", sGroup.c_str());
  auto& egosInfo = static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->ego_infos;
  for (int i = 0; i < egosInfo.size(); i++) {
    const auto& egoItem = egosInfo[i];
    if (sGroup.empty()) {
      const std::vector<std::pair<int64_t, zmq::message_t>>& measurements = egoItem.ego_vehicle_measurement;
      if (raw == nullptr) {
        nEgoSize += measurements.size();
        continue;
      }
      if (i != *idx) continue;
      const auto& item = measurements.front();
      *idx = item.first;
      *raw = static_cast<const char*>(std::get<1>(item).data());
      printf("idx: %d\n", item.first);
      printf("sGroup: %d\n", std::get<1>(item).size());
      return std::get<1>(item).size();
    } else if (egoItem.ego_group_name == sGroup) {
      const std::vector<std::pair<int64_t, zmq::message_t>>& measurements = egoItem.ego_vehicle_measurement;
      if (raw == nullptr) return measurements.size();
      if (*idx < 0 || *idx >= measurements.size()) return 0;
      const auto& item = measurements[*idx];
      *idx = item.first;
      *raw = static_cast<const char*>(std::get<1>(item).data());
      return std::get<1>(item).size();
    }
  }
  return nEgoSize;
}

uint32_t txsim_scene_pb(void* data, const char** raw) {
  const zmq::message_t& m =
      static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->m_scene_pb;
  *raw = static_cast<const char*>(m.data());
  return m.size();
}

uint32_t txsim_setting_pb(void* data, const char** raw) {
  const zmq::message_t& m =
      static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->m_setting_pb;
  *raw = static_cast<const char*>(m.data());
  return m.size();
}

const char* txsim_group_name(void* data) {
  return static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleResetRequest>()->group.c_str();
}


/******************************************************************************/
/************************* Step API Implementation ****************************/
/******************************************************************************/

uint32_t txsim_subscribed_raw_message(void* data, const char* topic, const char** msg) {
  auto& module_data = *static_cast<tx_sim::impl::ModuleData*>(data);
  const std::string group = module_data.req<tx_sim::impl::ModuleStepRequest>()->group;
  std::string grouped_topic = group.length() > 0 ? group + "/" + topic : topic;

  const std::unordered_map<std::string, zmq::message_t>& m =
      module_data.req<tx_sim::impl::ModuleStepRequest>()->messages;
  auto it = m.find(topic);
  auto groupit = m.find(grouped_topic);
  if (it != m.cend()) {
    *msg = static_cast<const char*>(it->second.data());
    return it->second.size();
  } else if (groupit != m.cend()) {
    *msg = static_cast<const char*>(groupit->second.data());
    return groupit->second.size();
  } else {
    *msg = kEmptyString.c_str();
    return 0;
  }
}

void txsim_publish_raw_message(void* data, const char* topic, const char* payload, uint32_t len) {
  auto& module_data = *static_cast<tx_sim::impl::ModuleData*>(data);
  const std::string group = module_data.req<tx_sim::impl::ModuleStepRequest>()->group;
  std::string grouped_topic = group.length() > 0 ? group + "/" + topic : topic;
  if (module_data.Publishes(topic)) {
    std::unordered_map<std::string, zmq::message_t>& m = module_data.resp<tx_sim::impl::ModuleStepResponse>()->messages;
    m[topic].rebuild(payload, len);
  }
  if (module_data.Publishes(grouped_topic)) {
    std::unordered_map<std::string, zmq::message_t>& m = module_data.resp<tx_sim::impl::ModuleStepResponse>()->messages;
    m[grouped_topic].rebuild(payload, len);
    std::string msg_payload;
    const char* msg = static_cast<const char*>(m[grouped_topic].data());
    uint32_t len = m[grouped_topic].size();
    msg_payload.assign(msg, len);
  }
}

void txsim_stop_scenario(void* data, const char* reason) {
  std::shared_ptr<tx_sim::impl::ModuleStepResponse> r =
      static_cast<tx_sim::impl::ModuleData*>(data)->resp<tx_sim::impl::ModuleStepResponse>();
  r->type = tx_sim::impl::kModuleRequireStop;
  r->err.assign(reason);
}

double txsim_timestamp(void* data) {
  return static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleStepRequest>()->sim_time;
}

double txsim_curtime_timestamp(void* data) {
  return static_cast<tx_sim::impl::ModuleData*>(data)->req<tx_sim::impl::ModuleStepRequest>()->cur_time;
}

uint32_t txsim_subscribed_shmem_data(void* data, const char* topic, const char** buf) {
  auto& module_data = *static_cast<tx_sim::impl::ModuleData*>(data);
  const std::string group = module_data.req<tx_sim::impl::ModuleStepRequest>()->group;
  std::string grouped_topic = group.length() > 0 ? group + "/" + topic : topic;

  if (module_data.req<tx_sim::impl::ModuleStepRequest>()->PublishesShmem(topic)) {
    tx_sim::impl::ShmemPtr& shmem_ptr =
        module_data.RetriveShmemPtr(module_data.req<tx_sim::impl::ModuleStepRequest>()->sub_topic_shmem_names[topic],
                                    boost::interprocess::read_only);
    *buf = shmem_ptr ? static_cast<const char*>(shmem_ptr->get_address()) : nullptr;
    return shmem_ptr ? shmem_ptr->get_size() : 0;
  }
  if (module_data.req<tx_sim::impl::ModuleStepRequest>()->PublishesShmem(grouped_topic)) {
    tx_sim::impl::ShmemPtr& shmem_ptr = module_data.RetriveShmemPtr(
        module_data.req<tx_sim::impl::ModuleStepRequest>()->sub_topic_shmem_names[grouped_topic],
        boost::interprocess::read_only);
    *buf = shmem_ptr ? static_cast<const char*>(shmem_ptr->get_address()) : nullptr;
    return shmem_ptr ? shmem_ptr->get_size() : 0;
  }
  return 0;
}

uint32_t txsim_published_shmem_buffer(void* data, const char* topic, char** buf) {
  auto& module_data = *static_cast<tx_sim::impl::ModuleData*>(data);
  const std::string group = module_data.req<tx_sim::impl::ModuleStepRequest>()->group;
  std::string grouped_topic = group.length() > 0 ? group + "/" + topic : topic;
  if (module_data.req<tx_sim::impl::ModuleStepRequest>()->PublishesShmem(topic)) {
    tx_sim::impl::ShmemPtr& shmem_ptr =
        module_data.RetriveShmemPtr(module_data.req<tx_sim::impl::ModuleStepRequest>()->pub_topic_shmem_names[topic],
                                    boost::interprocess::read_write);
    *buf = shmem_ptr ? static_cast<char*>(shmem_ptr->get_address()) : nullptr;
    return shmem_ptr ? shmem_ptr->get_size() : 0;
  }
  if (module_data.req<tx_sim::impl::ModuleStepRequest>()->PublishesShmem(grouped_topic)) {
    tx_sim::impl::ShmemPtr& shmem_ptr = module_data.RetriveShmemPtr(
        module_data.req<tx_sim::impl::ModuleStepRequest>()->pub_topic_shmem_names[grouped_topic],
        boost::interprocess::read_write);
    *buf = shmem_ptr ? static_cast<char*>(shmem_ptr->get_address()) : nullptr;
    return shmem_ptr ? shmem_ptr->get_size() : 0;
  }
  return 0;
}

/******************************************************************************/
/************************* Stop API Implementation ****************************/
/******************************************************************************/
void txsim_set_feedback(void* data, const char* key, const char* value) {
  static_cast<tx_sim::impl::ModuleData*>(data)->resp<tx_sim::impl::ModuleStopResponse>()->feedbacks[key].assign(value);
}

const char* txsim_module_status(void* data) {
  return static_cast<tx_sim::impl::ModuleData*>(data)
      ->req<tx_sim::impl::ModuleStopRequest>()
      ->m_sGradingPerfStats.c_str();
}


}  // extern "C"


namespace tx_sim {
namespace impl {

void ModuleData::Decode(zmq::multipart_t& msg) {
  if (msg.size() == 0) throw std::runtime_error("empty msg.");
  req_type_ = static_cast<ModuleRequestType>(
      tx_sim::utils::DecodeSLEB128(static_cast<const uint8_t*>(msg[0].data()), msg[0].size()));
  switch (req_type_) {
    case kModuleInit: {
      req_.reset(new ModuleInitRequest);
      resp_.reset(new ModuleInitResponse);
      break;
    }
    case kModuleReset: {
      req_.reset(new ModuleResetRequest);
      resp_.reset(new ModuleResetResponse);
      break;
    }
    case kModuleStep: {
      req_.reset(new ModuleStepRequest);
      resp_.reset(new ModuleStepResponse);
      break;
    }
    case kModuleStop: {
      req_.reset(new ModuleStopRequest);
      resp_.reset(new ModuleStopResponse);
      break;
    }
    default: {
      throw std::runtime_error("unknown request type " + req_type_);
    }
  }
  req_->Decode(msg);
}


void ModuleData::Encode(zmq::multipart_t& msg) const {
  resp_->Encode(msg);
}


void ModuleData::FreeResources() {
  // unmapping mapped shared memories.
  for (auto& shmem : opened_shmems_) shmem.second = nullptr;
}


void** ModuleData::cbs() {
  switch (req_type_) {
    case kModuleInit: return init_cbs_;
    case kModuleReset: return reset_cbs_;
    case kModuleStep: return step_cbs_;
    case kModuleStop: return stop_cbs_;
    default: return nullptr;  // already checked in Decode().
  }
}


ShmemPtr& ModuleData::RetriveShmemPtr(const std::string& shmem_name, boost::interprocess::mode_t mode) {
  if (shmem_name.empty()) return opened_shmems_[shmem_name];  // return nullptr.
  ShmemPtr& p = opened_shmems_[shmem_name];
  if (p == nullptr) {  // open and mapping the named shared memory segment.
    try {
      boost::interprocess::shared_memory_object shm_obj(boost::interprocess::open_only, shmem_name.c_str(), mode);
      p.reset(new boost::interprocess::mapped_region(shm_obj, mode));
    } catch (const std::exception& e) {}
  }
  return p;
}

}  // namespace impl
}  // namespace tx_sim
