#pragma once

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "utils/defs.h"
#include "zmq.hpp"

namespace tx_sim {
namespace utils {

// an interface encapsulating module process creation/termination for two reason:
// 1. for mock impl used in unit tests;
// 2. on Linux, currently, our implementation uses prctl(PR_SET_PDEATHSIG, SIGKILL) to force the childs to be killed on
//    tadsim exit. but the PR_SET_PDEATHSIG flag is actually means the signal is propagated on the exit of the thread,
//    not the process, which created the children. so we need ensure the child process creation is always forked in one
//    same thread, and using RAII of c++ class to simplify our implementation.
class ModuleProcessService {
 public:
  virtual txsim_pid_t CreateModule(const std::string& path, const std::vector<std::string>& args,
                                   const std::vector<tx_sim::impl::StringPair>& envs,
                                   const std::string& stdout_path) = 0;

  virtual void TerminateModule(txsim_pid_t pid) = 0;
};

class ModuleProcessServiceImpl final : public ModuleProcessService {
 public:
  ModuleProcessServiceImpl(std::shared_ptr<zmq::context_t>& ctx);
  ~ModuleProcessServiceImpl();

  txsim_pid_t CreateModule(const std::string& path, const std::vector<std::string>& args,
                           const std::vector<tx_sim::impl::StringPair>& envs, const std::string& stdout_path) override;

  void TerminateModule(txsim_pid_t pid) override;

 private:
  static const std::string kCalleeBindAddress;
  std::unique_ptr<zmq::socket_t> caller_, callee_;
  std::unique_ptr<std::thread> serv_;
};

}  // namespace utils
}  // namespace tx_sim
