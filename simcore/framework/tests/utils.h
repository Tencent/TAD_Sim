#pragma once

#include "gmock/gmock.h"

#include "coordinator/config.h"
#include "coordinator/status.h"
#include "utils/json_helper.h"
#include "utils/msgs.h"

namespace tx_sim {

namespace utils {

std::string GetSelfExecutablePath();

}  // namespace utils

namespace test {

// #define EXPECT_FLOAT_EQ(a, b, precision) EXPECT_LT(std::abs(a - b), precision)

MATCHER_P(JsonStrEqualsTo, expected, "") {
  Json::Value doc;
  tx_sim::utils::ReadJsonDocFromString(doc, arg);
  return doc == expected;
}

MATCHER_P(ResetParamsEqualsTo, expected, "") {
  return arg.type == tx_sim::impl::kModuleReset && static_cast<tx_sim::impl::ModuleResetRequest&>(arg) == expected;
}

MATCHER(IsStepRequest, "") {
  return arg.type == tx_sim::impl::kModuleStep;
}

MATCHER_P(StepParamsEqualsTo, expected, "") {
  return arg.type == tx_sim::impl::kModuleStep && static_cast<tx_sim::impl::ModuleStepRequest&>(arg) == expected;
}

MATCHER(IsStopRequest, "") {
  return arg.type == tx_sim::impl::kModuleStop;
}

MATCHER_P(StopParamsEqualsTo, expected, "") {
  return arg.type == tx_sim::impl::kModuleStop && static_cast<tx_sim::impl::ModuleStopRequest&>(arg) == expected;
}

MATCHER(IsResetRequest, "") {
  return arg.type == tx_sim::impl::kModuleReset;
}

struct CmdCbStatusExpectation {
  size_t status_idx;
  const std::vector<tx_sim::coordinator::CommandStatus>& cmd_status_list;
  CmdCbStatusExpectation(const std::vector<tx_sim::coordinator::CommandStatus>& cs)
      : cmd_status_list(cs), status_idx(0) {}
};

}  // namespace test

// gtest requires all PrintTo funcs belongs to the same namespace that the printed class in.

namespace coordinator {

void PrintTo(const TopicPubSubInfo& t, std::ostream* os);
void PrintTo(const ModuleInitStatus& s, std::ostream* os);
void PrintTo(const StepMessage& m, std::ostream* os);
void PrintTo(const ModuleCmdStatus& s, std::ostream* os);
void PrintTo(const CommandStatus& s, std::ostream* os);

void PrintTo(const ModuleConfig& c, std::ostream* os);
void PrintTo(const CoordinatorConfig& c, std::ostream* os);

}  // namespace coordinator

namespace impl {

void PrintTo(const ModuleResetRequest& q, std::ostream* os);
void PrintTo(const ModuleResetResponse& p, std::ostream* os);

void PrintTo(const ModuleStepRequest& q, std::ostream* os);
void PrintTo(const ModuleStepResponse& p, std::ostream* os);

void PrintTo(const ModuleStopRequest& q, std::ostream* os);
void PrintTo(const ModuleStopResponse& p, std::ostream* os);

void PrintTo(const ModuleRequest& q, std::ostream* os);
void PrintTo(const ModuleResponse& p, std::ostream* os);

}  // namespace impl

}  // namespace tx_sim
