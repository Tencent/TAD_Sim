#pragma once

#include "gmock/gmock.h"

#include "cli/sim_client.h"
#include "coordinator/config.h"
#include "coordinator/connection.h"
#include "coordinator/local_client.h"
#include "coordinator/module_mgr.h"
#include "coordinator/scenario_parser.h"

using namespace tx_sim::coordinator;
using namespace tx_sim::impl;
using namespace tx_sim::cli;

namespace tx_sim {
namespace test {

class MockScenarioParser : public IScenarioParser {
 public:
  MOCK_METHOD1(CheckIsOpenScenario, std::string(const std::string&));
  MOCK_METHOD2(ParseScenario, bool(const std::string&, ModuleResetRequest&));
  MOCK_METHOD2(ParseLogsimEvents, void(const std::string&, LogsimEventInfo&));
  MOCK_METHOD2(ParseLog2WorldConfig, void(const std::string&, Log2WorldPlayConfig&));
  MOCK_METHOD2(CacheLog2WorldConfig, void(const std::string&, Log2WorldPlayConfig&));
  MOCK_METHOD1(GetEgoType, std::string(const std::string&));
  MOCK_METHOD1(ExistSimRec, bool(const std::string&));
};

class MockConfigLoader : public ConfigLoader {
 public:
  MOCK_METHOD1(LoadCoordinatorConfig, void(CoordinatorConfig&));
  MOCK_METHOD1(LoadModuleConfig, bool(ModuleConfig&));
  MOCK_METHOD2(LoadModuleConfigByName, bool(std::string, std::vector<ModuleConfig>&));
};

class MockConnection : public ModuleConnection {
 public:
  MOCK_METHOD1(Send, bool(ModuleRequest&));
  MOCK_METHOD2(Receive, bool(ModuleResponse&, uint32_t));
  MOCK_METHOD0(WaitingReply, bool());
};

class MockModuleMgr : public ModuleManager {
 public:
  MOCK_METHOD1(UpdateConfigs, void(const CoordinatorConfig&));
  MOCK_METHOD2(MonitorInitWorkflow, size_t(std::vector<ModuleInitStatus>&, std::vector<ModuleCmdStatus>&));
  MOCK_METHOD1(GetConnection, std::shared_ptr<ModuleConnection>(const std::string&));
  MOCK_METHOD2(CheckModuleStatus, bool(const std::string&, std::string&));
  MOCK_METHOD1(RemoveRegistryModule, void(const std::string));
};

class MockClient : public ISimClient {
 public:
  MOCK_METHOD1(Setup, void(const tx_sim::coordinator::CommandStatusCb&));
  MOCK_METHOD1(UnSetup, void(const tx_sim::coordinator::CommandStatusCb&));
  MOCK_METHOD1(Step, void(const tx_sim::coordinator::CommandStatusCb&));
  MOCK_METHOD1(Stop, void(const tx_sim::coordinator::CommandStatusCb&));
  MOCK_METHOD1(Run, void(const tx_sim::coordinator::CommandStatusCb&));
  MOCK_METHOD1(Pause, void(const tx_sim::coordinator::CommandStatusCb&));

  MOCK_METHOD1(LaunchModule, tx_sim::impl::CmdErrorCode(const std::string&));
  MOCK_METHOD1(TerminateModule, tx_sim::impl::CmdErrorCode(const std::string&));

  MOCK_METHOD1(AddModuleConfig, tx_sim::impl::CmdErrorCode(const std::string&));
  MOCK_METHOD1(RemoveModuleConfig, tx_sim::impl::CmdErrorCode(const std::string&));
  MOCK_METHOD1(UpdateModuleConfig, tx_sim::impl::CmdErrorCode(const std::string&));
  MOCK_METHOD2(GetModuleConfig, tx_sim::impl::CmdErrorCode(const std::string&, std::string&));
  MOCK_METHOD1(GetAllModuleConfigs, tx_sim::impl::CmdErrorCode(std::vector<std::string>&));
  MOCK_METHOD1(SetSysConfigs, tx_sim::impl::CmdErrorCode(const std::string& config_json_str));
  MOCK_METHOD1(GetSysConfigs, tx_sim::impl::CmdErrorCode(std::string& config_json_str));
  MOCK_METHOD0(RestoreDefaultConfigs, tx_sim::impl::CmdErrorCode());
};

class MockClientFactory final : public ISimClientFactory {
 public:
  MockClientFactory(std::shared_ptr<ISimClient> client) : client_(client) {}
  std::shared_ptr<ISimClient> CreateSimClient(const std::string& endpoint) override { return client_; }

 private:
  std::shared_ptr<ISimClient> client_;
};

}  // namespace test
}  // namespace tx_sim
