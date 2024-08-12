#include <iomanip>
#include <iostream>
#include <vector>

#include "coordinator/local_client.h"
#include "utils/json_helper.h"
#include "utils/os.h"


using namespace tx_sim::cli;
using namespace tx_sim::coordinator;
using namespace tx_sim::impl;
using namespace tx_sim::utils;


void ComputeFeedbackStatics(const std::vector<std::string>& feedbacks) {
  uint32_t total_period = 0, total_exec_time = 0, total_min_period = 0, total_max_period = 0, total_min_exec_time = 0,
           total_max_exec_time = 0;
  std::unordered_map<std::string, std::string> fbs;
  size_t module_count = feedbacks.size();
  for (const std::string& fb : feedbacks) {
    Json2Map(fb, fbs);
    total_period += std::stoi(fbs.at("mean_step_period"));
    total_exec_time += std::stoi(fbs.at("mean_step_exec_time"));
    total_min_period += std::stoi(fbs.at("min_step_period"));
    total_max_period += std::stoi(fbs.at("max_step_period"));
    total_min_exec_time += std::stoi(fbs.at("min_step_exec_time"));
    total_max_exec_time += std::stoi(fbs.at("max_step_exec_time"));
  }
  std::cout << std::left << std::setw(20) << (total_period / module_count) << std::setw(16)
            << (total_exec_time / module_count) << std::setw(16) << (total_min_period / module_count) << std::setw(20)
            << (total_max_period / module_count) << std::setw(20) << (total_min_exec_time / module_count)
            << std::setw(16) << (total_max_exec_time / module_count);
}


int main(int argc, char* argv[]) {
  CmdErrorCode ec = kCmdAccepted;

  Client cli(GetLocalServiceBindAddress());
  try {
    cli.Setup("", [&ec](const CommandStatus& status) { ec = status.ec; });
    if (ec != kCmdSucceed) return 1;

    std::vector<std::string> feedbacks;
    cli.Run([&ec, &feedbacks](const CommandStatus& status) {
      ec = status.ec;
      if (ec == kCmdScenarioTimeout)
        for (const auto& m : status.module_status)
          if (!m.feedback.empty()) feedbacks.push_back(m.feedback);
    });
    if (ec != kCmdScenarioTimeout) return 3;
    ComputeFeedbackStatics(feedbacks);
  } catch (const std::exception& e) {
    std::cerr << "client error: " << e.what() << std::endl;
    return 4;
  }

  return 0;
}