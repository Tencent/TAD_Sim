#include "utils.h"

#ifdef __linux__
#  include <limits.h>
#  include <unistd.h>
#elif defined _WIN32
#  include <Windows.h>
#  ifndef PATH_MAX
#    define PATH_MAX 4096
#  endif
#endif


namespace tx_sim {

namespace utils {

std::string GetSelfExecutablePath() {
#ifdef __linux__
  char buf[PATH_MAX];  // https://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html
  ssize_t nbytes = readlink("/proc/self/exe", buf, PATH_MAX);
  if (nbytes < 0 || nbytes >= PATH_MAX) return "";
  return buf;
#elif defined _WIN32
  char buf[PATH_MAX];
  DWORD nbytes = GetModuleFileName(NULL, buf, PATH_MAX);
  if (nbytes < 0 || nbytes >= PATH_MAX) return "";
  return buf;
#endif
}

}  // namespace utils

namespace coordinator {

void PrintTo(const TopicPubSubInfo& t, std::ostream* os) {
  *os << "{sub_topics: " << ::testing::PrintToString(t.sub_topics)
      << ", pub_topics: " << ::testing::PrintToString(t.pub_topics)
      << ", sub_shmems: " << ::testing::PrintToString(t.sub_shmems)
      << ", pub_shmems: " << ::testing::PrintToString(t.pub_shmems) << "}";
}

void PrintTo(const ModuleInitStatus& s, std::ostream* os) {
  *os << "{name: " << s.name << ", state: " << Enum2String(s.state) << ", topic_info: ";
  PrintTo(s.topic_info, os);
  *os << "}";
}

void PrintTo(const StepMessage& m, std::ostream* os) {
  *os << "{timestamp: " << m.timestamp << ", messages: " << ::testing::PrintToString(m.messages) << "}";
}

void PrintTo(const ModuleCmdStatus& s, std::ostream* os) {
  *os << "{name: " << s.name << ", elapsed_time: " << s.elapsed_time << ", ec: " << Enum2String(s.ec)
      << ", msg: " << s.msg << ", feedback:" << s.feedback << "}";
}

void PrintTo(const CommandStatus& s, std::ostream* os) {
  *os << "{ec: " << Enum2String(s.ec) << ", total_time_cost: " << s.total_time_cost
      << ", module_status: " << ::testing::PrintToString(s.module_status)
      << ", init_status: " << ::testing::PrintToString(s.init_status)
      << ", step_message: " << ::testing::PrintToString(s.step_message) << "}";
}

void PrintTo(const ModuleConfig& c, std::ostream* os) {
  *os << "{name: " << c.name << ", execute_period: " << c.execute_period << ", response_time: " << c.response_time
      << ", init_args: " << ::testing::PrintToString(c.init_args) << ", conn_args: (" << c.conn_args.cmd_timeout << ", "
      << c.conn_args.step_timeout << "), auto_launch: " << c.auto_launch
      << ", dep_paths: " << ::testing::PrintToString(c.dep_paths) << ", so_path: " << c.so_path
      << ", bin_path: " << c.bin_path << ", bin_args: " << ::testing::PrintToString(c.bin_args)
      << ", category: " << c.category << "}";
}

void PrintTo(const CoordinatorConfig& c, std::ostream* os) {
  *os << "{control_rate: " << c.control_rate << ", scenario_time_limit: " << c.scenario_time_limit
      << ", coord_mode: " << c.coord_mode << ", module_configs: " << ::testing::PrintToString(c.module_configs) << "}";
}

}  // namespace coordinator

namespace impl {

void PrintTo(const ModuleResetRequest& q, std::ostream* os) {
  *os << "ModuleResetRequest{scenario_path: " << q.scenario_path << ", map_path: " << q.map_path
      << ", map_local_origin: "
      << ::testing::PrintToString(q.map_local_origin)
      //<< ", ego_path: " << ::testing::PrintToString(q.ego_path) << ", ego_initial_velocity: " <<
      //q.ego_initial_velocity
      //<< ", ego_initial_theta: " << q.ego_initial_theta << ", ego_speed_limit: " << q.ego_speed_limit
      << ", geo_fence: " << ::testing::PrintToString(q.geo_fence) << ", initial_location: (size "
      //<< q.initial_location.size() << ")}"
      ;
}

void PrintTo(const ModuleResetResponse& p, std::ostream* os) {
  *os << "ModuleResetResponse{type: " << Enum2String(p.type) << ", err: " << p.err << "}";
}

void PrintTo(const ModuleStepRequest& q, std::ostream* os) {
  *os << "ModuleStepRequest{sim_time: " << q.sim_time << ", messages: " << ::testing::PrintToString(q.messages)
      << ", sub_topic_shmem_names: " << ::testing::PrintToString(q.sub_topic_shmem_names)
      << ", pub_topic_shmem_names: " << ::testing::PrintToString(q.pub_topic_shmem_names) << "}";
}

void PrintTo(const ModuleStepResponse& p, std::ostream* os) {
  *os << "ModuleStepResponse{type: " << Enum2String(p.type) << ", err: " << p.err << ", time_cost: " << p.time_cost
      << ", messages: " << ::testing::PrintToString(p.messages) << "}";
}

void PrintTo(const ModuleStopRequest& q, std::ostream* os) {
  *os << "ModuleStopRequest{}";
}

void PrintTo(const ModuleStopResponse& p, std::ostream* os) {
  *os << "ModuleStopResponse{type: " << Enum2String(p.type) << ", err: " << p.err
      << ", feedback: " << ::testing::PrintToString(p.feedbacks) << "}";
}

void PrintTo(const ModuleRequest& q, std::ostream* os) {
  switch (q.type) {
    case tx_sim::impl::kModuleReset: PrintTo(*dynamic_cast<const ModuleResetRequest*>(&q), os); return;
    case tx_sim::impl::kModuleStep: PrintTo(*dynamic_cast<const ModuleStepRequest*>(&q), os); return;
    case tx_sim::impl::kModuleStop: PrintTo(*dynamic_cast<const ModuleStopRequest*>(&q), os); return;
    default: *os << "ModuleRequest{type: " << q.type << "}";
  }
}

void PrintTo(const ModuleResponse& p, std::ostream* os) {
  *os << "ModuleResponse{type: " << Enum2String(p.type) << ", err: " << p.err << "}";
}

}  // namespace impl

}  // namespace tx_sim