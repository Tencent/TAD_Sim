// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <stdint.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "boost/interprocess/mapped_region.hpp"
#include "boost/interprocess/shared_memory_object.hpp"

#include "utils/leb128.h"
#include "utils/msgs.h"

extern "C" {

void txsim_set_module_error(void* data, const char* err);

/********************************* Init API ***********************************/
const char* txsim_init_parameter(void* data, const char* key);
void txsim_subscribe(void* data, const char* topic);
void txsim_publish(void* data, const char* topic);
void txsim_subscribe_shmem(void* data, const char* topic);
void txsim_publish_shmem(void* data, const char* topic, size_t max_size);

/******************************** Reset API ***********************************/
const char* txsim_map_file_path(void* data);
void txsim_map_local_origin(void* data, double* lon, double* lat, double* alt);
void txsim_ego_destination(void* data, double* lon, double* lat, double* alt);
double txsim_ego_speed_limit(void* data);
uint32_t txsim_location_raw_message(void* data, const char** raw);
uint32_t txsim_geo_fence(void* data, double* point_array);
const char* txsim_scenario_file_path(void* data);
uint32_t txsim_ego_path(void* data, double* point_array);
uint32_t txsim_vehicle_measurements(void* data, int64_t* idx, const char** raw);
uint32_t txsim_scene_pb(void* data, const char** row);
uint32_t txsim_setting_pb(void* data, const char** row);
const char* txsim_group_name(void* data);

/********************************* Step API ***********************************/
uint32_t txsim_subscribed_raw_message(void* data, const char* topic, const char** msg);
void txsim_publish_raw_message(void* data, const char* topic, const char* payload, uint32_t len);
void txsim_stop_scenario(void* data, const char* reason);
double txsim_timestamp(void* data);
uint32_t txsim_subscribed_shmem_data(void* data, const char* topic, const char** buf);
uint32_t txsim_published_shmem_buffer(void* data, const char* topic, char** buf);
double txsim_curtime_timestamp(void* data);

/********************************* Stop API ***********************************/
void txsim_set_feedback(void* data, const char* key, const char* value);
const char* txsim_module_status(void* data);

}  // extern "C"

// clang-format off

#define TXSIM_INIT_CALLBACKS_DEF(var) void* var[6] {   \
  (void*)&txsim_set_module_error,                       \
  (void*)&txsim_init_parameter,                         \
  (void*)&txsim_subscribe,                              \
  (void*)&txsim_publish,                                \
  (void*)&txsim_subscribe_shmem,                        \
  (void*)&txsim_publish_shmem,                          \
}

#define TXSIM_RESET_CALLBACKS_DEF(var) void* var[13] {  \
  (void*)&txsim_set_module_error,                       \
  (void*)&txsim_map_file_path,                          \
  (void*)&txsim_map_local_origin,                       \
  (void*)&txsim_ego_destination,                        \
  (void*)&txsim_ego_speed_limit,                        \
  (void*)&txsim_location_raw_message,                   \
  (void*)&txsim_geo_fence,                              \
  (void*)&txsim_scenario_file_path,                     \
  (void*)&txsim_ego_path,                               \
  (void*)&txsim_vehicle_measurements,                   \
  (void*)&txsim_scene_pb,                               \
  (void*)&txsim_setting_pb,                             \
  (void*)&txsim_group_name,                             \
}

#define TXSIM_STEP_CALLBACKS_DEF(var) void* var[8] {    \
  (void*)&txsim_set_module_error,                       \
  (void*)&txsim_subscribed_raw_message,                 \
  (void*)&txsim_publish_raw_message,                    \
  (void*)&txsim_stop_scenario,                          \
  (void*)&txsim_timestamp,                              \
  (void*)&txsim_subscribed_shmem_data,                  \
  (void*)&txsim_published_shmem_buffer,                 \
  (void*)&txsim_curtime_timestamp                       \
}

#define TXSIM_STOP_CALLBACKS_DEF(var) void* var[3] {    \
  (void*)&txsim_set_module_error,                       \
  (void*)&txsim_set_feedback,                           \
  (void*)&txsim_module_status,                          \
}

// clang-format on

#define TXSIM_MESSENGER_IMPL_API_VERSION 6130803

namespace tx_sim {
namespace impl {

typedef std::unique_ptr<boost::interprocess::mapped_region> ShmemPtr;

class ModuleData {
 public:
  void Decode(zmq::multipart_t& msg);
  void Encode(zmq::multipart_t& msg) const;
  void FreeResources();
  void** cbs();
  void set_publishes(const std::unordered_set<std::string>& topics) { publishes_ = topics; }
  bool Publishes(const std::string& topic) { return publishes_.find(topic) != publishes_.end(); }
  ShmemPtr& RetriveShmemPtr(const std::string& shmem_name, boost::interprocess::mode_t mode);
  void set_err(const char* err) {
    resp_->err = err;
    resp_->type = tx_sim::impl::kModuleError;
  }
  ModuleRequestType req_type() { return req_type_; }
  template <class T>
  std::shared_ptr<T> req() {
    return std::dynamic_pointer_cast<T>(req_);
  }
  template <class T>
  const std::shared_ptr<T> req() const {
    return std::dynamic_pointer_cast<T>(req_);
  }
  template <class T>
  std::shared_ptr<T> resp() {
    return std::dynamic_pointer_cast<T>(resp_);
  }

 private:
  std::shared_ptr<tx_sim::impl::ModuleRequest> req_;
  std::shared_ptr<tx_sim::impl::ModuleResponse> resp_;
  ModuleRequestType req_type_;
  TXSIM_INIT_CALLBACKS_DEF(init_cbs_);
  TXSIM_RESET_CALLBACKS_DEF(reset_cbs_);
  TXSIM_STEP_CALLBACKS_DEF(step_cbs_);
  TXSIM_STOP_CALLBACKS_DEF(stop_cbs_);
  std::unordered_map<std::string, ShmemPtr> opened_shmems_;
  std::unordered_set<std::string> publishes_;
};

}  // namespace impl
}  // namespace tx_sim
