#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "utils/defs.h"

namespace tx_sim {
namespace utils {

txsim_pid_t CreateModuleProcess(const std::string& path, const std::vector<std::string>& args,
                                const std::vector<tx_sim::impl::StringPair>& envs, const std::string& stdout_path);

void TerminateModuleProcess(txsim_pid_t pid);

struct ChildProcessStatus {
  txsim_pid_t pid{txsim_invalid_pid_t};  // the pid of the child process whose status is returned, <= 0 for no status
                                         // of any children process avalible.
  std::string status_desc;               // a string description for exit code, un-caught signal or exception value.
  std::vector<txsim_pid_t> queries;      // specifies which processes to be queried, Windows only.
};

void PollChildrenExitStatus(ChildProcessStatus& status);

std::string GetProcessExitStatusDescription(int stat);

}  // namespace utils
}  // namespace tx_sim
