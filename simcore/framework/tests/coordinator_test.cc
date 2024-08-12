#include "gtest/gtest.h"

#include "coordinator/coordinator.h"
#include "coordinator/scenario_parser.h"
#include "coordinator_test.h"
#include "mocks.h"
#include "txsim_messenger.h"
#include "utils.h"


using ::testing::ByRef;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::Throw;


namespace tx_sim {
namespace test {

// TODO(nemo): the resource path should be passed through cmdline args or env vars.
fs::path GetTestResourceDirPath() {
  fs::path resource_path(tx_sim::utils::GetSelfExecutablePath());
  do {
    resource_path = resource_path.parent_path();
  } while (resource_path.filename() != "framework_v2" && !resource_path.empty());
  resource_path /= "tests/resources";
  return resource_path;
}

class CodrTest : public testing::Test {
 protected:
  void SetExpectedUpdatingModuleConfigs(const CoordinatorConfig& loaded, CoordinatorConfig& cfgs) {
    for (const auto& c : loaded.module_configs) {
      cfgs.module_configs.emplace_back();
      cfgs.module_configs.back() = c;
      cfgs.module_configs.back().init_args[tx_sim::constant::kInitKeyModuleExecutePeriod] =
          std::to_string(c.execute_period);
    }
  }

  void SetCmdCallbackExpectaions(CommandInfo& cmd_info, CmdCbStatusExpectation& expected) {
    cmd_info.status_cb = [&expected](const CommandStatus& status) {
      ASSERT_LT(expected.status_idx, expected.cmd_status_list.size());
      EXPECT_EQ(status, expected.cmd_status_list[expected.status_idx++]);
    };
  }

  void SetInitWorkflowExpectations(CommandInfo& cmd_info, CmdCbStatusExpectation& expected_cb_status) {
    size_t expected_finish_count = 0;
    {
      InSequence monitor_seq;
      // accepted -> init workflow... -> cmd result
      for (size_t i = 1; i < expected_cb_status.cmd_status_list.size() - 1; ++i) {
        const CommandStatus& cmd_st = expected_cb_status.cmd_status_list[i];
        for (const ModuleInitStatus& s : cmd_st.init_status)
          if (s.state == kModuleInitSucceed || s.state == kModuleInitFailed) ++expected_finish_count;
        EXPECT_CALL(*mock_mgr_, MonitorInitWorkflow)
            .WillOnce(DoAll(SetArgReferee<0>(cmd_st.init_status), SetArgReferee<1>(cmd_st.module_status),
                            Return(expected_finish_count)));
      }
    }

    SetCmdCallbackExpectaions(cmd_info, expected_cb_status);
  }

  void Trans2Setup(const CoordinatorConfig& expected_config, std::vector<CommandStatus>* expected_status = nullptr) {
    // set expected init status.
    std::vector<ModuleInitStatus> expected_init_status;
    for (const auto& c : expected_config.module_configs) {
      expected_init_status.emplace_back(c.name, kModuleInitSucceed);
      auto it = pubsubs_.find(c.name);
      if (it != pubsubs_.end()) expected_init_status.back().topic_info = it->second;
    }

    ON_CALL(*mock_cfg_ldr_, LoadCoordinatorConfig).WillByDefault(SetArgReferee<0>(expected_config));
    ON_CALL(*mock_mgr_, MonitorInitWorkflow)
        .WillByDefault(DoAll(SetArgReferee<0>(expected_init_status), Return(expected_config.module_configs.size())));
    for (const auto& c : expected_config.module_configs)
      ON_CALL(*mock_mgr_, GetConnection(c.name)).WillByDefault(Return(mock_conns_[c.name]));

    if (expected_status) {                          // set callback status of setup phrase if needed.
      expected_status->emplace_back(kCmdAccepted);  // first -> accepted confirmation.
      expected_status->emplace_back(kCmdAccepted);  // second -> all module succeed.
      for (const auto& c : expected_config.module_configs)
        expected_status->back().init_status.emplace_back(c.name, kModuleInitSucceed);
    }
  }

  void Trans2Reset(const CoordinatorConfig& expected_config) {
    Trans2Setup(expected_config);

    ModuleResetResponse expected_resp;
    expected_resp.type = kModuleOK;

    for (const auto& conn : mock_conns_) {
      ON_CALL(*conn.second, Send).WillByDefault(Return(true));
      ON_CALL(*conn.second, Receive).WillByDefault(DoAll(SetArgReferee<0>(expected_resp), Return(true)));
    }

    CommandInfo cmd_info(kCmdSetup);
    ASSERT_EQ(tested_codr_.Execute(cmd_info), kCmdSucceed);

    for (const auto& conn : mock_conns_) ASSERT_TRUE(Mock::VerifyAndClear(conn.second.get()));
  }

  void FallThroughReset(const std::vector<ModuleConfig>& modules, CommandStatus* status = nullptr) {
    for (const auto& m : modules) mock_conns_[m.name] = std::make_shared<NiceMock<MockConnection>>();
    for (const auto& c : modules) ON_CALL(*mock_mgr_, GetConnection(c.name)).WillByDefault(Return(mock_conns_[c.name]));
    ModuleResetResponse expected_resp;
    expected_resp.type = kModuleOK;
    for (const auto& conn : mock_conns_) {
      ON_CALL(*conn.second, Send).WillByDefault(Return(true));
      ON_CALL(*conn.second, Receive).WillByDefault(DoAll(SetArgReferee<0>(expected_resp), Return(true)));
    }
    if (status)
      for (const auto& m : modules) status->module_status.emplace_back(m.name, expected_resp.type, expected_resp.err);
  }

  std::shared_ptr<MockConfigLoader> mock_cfg_ldr_{new NiceMock<MockConfigLoader>()};
  std::shared_ptr<MockModuleMgr> mock_mgr_{new NiceMock<MockModuleMgr>()};
  std::shared_ptr<MockScenarioParser> mock_scenario_parser_{new NiceMock<MockScenarioParser>()};
  std::map<std::string, std::shared_ptr<MockConnection>> mock_conns_;
  std::map<std::string, TopicPubSubInfo> pubsubs_;
  Coordinator tested_codr_{"", mock_cfg_ldr_, mock_mgr_, mock_scenario_parser_};
};


TEST_F(CodrTest, InitialStatusCheck) {
  // 1. simulation time should be 0 on start-up.
  EXPECT_EQ(tested_codr_.current_simulation_time(), 0);

  // 2. following commands should NOT be allowed to execute on start-up:
  //    reset, step, run, pause, stop.
  std::vector<Command> cmds_expected_to_be_rejected{kCmdStep, kCmdRun, kCmdPause, kCmdStop};
  for (auto c : cmds_expected_to_be_rejected) {
    CommandInfo cmd_info(c);
    EXPECT_EQ(tested_codr_.Execute(cmd_info), kCmdRejected);
  }
}


TEST_F(CodrTest, SetupModules) {
  CmdErrorCode expected_result = kCmdSucceed;
  // 2-module configuration.
  CoordinatorConfig expected_config, expected_module_configs;
  expected_config.module_configs.emplace_back("test_a", 20);
  expected_config.module_configs.back().init_args["key_foo"] = "value_foo";
  expected_config.module_configs.emplace_back("test_b", 100);
  // expected updating module configs(with default init args added).
  SetExpectedUpdatingModuleConfigs(expected_config, expected_module_configs);
  // cmd accepted -> all module init succeed -> cmd succeed.
  std::vector<CommandStatus> expected_cb_status;
  expected_cb_status.emplace_back(kCmdAccepted);  // command accepted confirmation msg.
  for (const ModuleConfig& m : expected_config.module_configs) {
    expected_cb_status.emplace_back(kCmdAccepted);
    expected_cb_status.back().init_status.emplace_back(m.name, kModuleInitSucceed);
  }
  expected_cb_status.emplace_back(expected_result);
  FallThroughReset(expected_config.module_configs, &expected_cb_status.back());
  CmdCbStatusExpectation cb_expectation(expected_cb_status);

  EXPECT_CALL(*mock_cfg_ldr_, LoadCoordinatorConfig).WillOnce(SetArgReferee<0>(expected_config));
  EXPECT_CALL(*mock_mgr_, UpdateConfigs(expected_module_configs));
  for (const auto& c : expected_config.module_configs) EXPECT_CALL(*mock_mgr_, GetConnection(c.name));

  CommandInfo cmd_info(kCmdSetup);
  SetInitWorkflowExpectations(cmd_info, cb_expectation);

  EXPECT_EQ(tested_codr_.Execute(cmd_info), expected_result);
}


TEST_F(CodrTest, SetupWithExtraInitArgs) {
  const std::string no_override_init_key("foo2"), extra_init_key("foo3");
  CoordinatorConfig cfg, expected_module_configs;
  cfg.module_configs.emplace_back("test", 10);
  auto& m = cfg.module_configs.back();
  m.init_args["foo"] = "bar";
  m.init_args[no_override_init_key] = "bar2";
  ON_CALL(*mock_cfg_ldr_, LoadCoordinatorConfig).WillByDefault(SetArgReferee<0>(cfg));
  std::vector<ModuleInitStatus> expected_init_status;
  expected_init_status.emplace_back(m.name, kModuleInitSucceed);
  ON_CALL(*mock_mgr_, MonitorInitWorkflow).WillByDefault(DoAll(SetArgReferee<0>(expected_init_status), Return(1)));

  SetExpectedUpdatingModuleConfigs(cfg, expected_module_configs);
  expected_module_configs.module_configs.back().init_args[extra_init_key] = "baz3";
  EXPECT_CALL(*mock_mgr_, UpdateConfigs(expected_module_configs));

  CommandInfo cmd_info(kCmdSetup);
  cmd_info.extra_init_params.emplace_back(no_override_init_key, "bar2_overrode");
  cmd_info.extra_init_params.emplace_back(extra_init_key, "baz3");
  FallThroughReset(cfg.module_configs);
  EXPECT_EQ(tested_codr_.Execute(cmd_info), kCmdSucceed);
}


TEST_F(CodrTest, SetupWithConfigLoadingError) {
  CmdErrorCode expected_result = kCmdSystemError;
  std::vector<CommandStatus> expected_cb_status;
  expected_cb_status.emplace_back(kCmdAccepted);  // command accepted confirmation msg.
  expected_cb_status.emplace_back(expected_result);
  CmdCbStatusExpectation cb_expectation(expected_cb_status);

  std::runtime_error expected_err("some error");
  EXPECT_CALL(*mock_cfg_ldr_, LoadCoordinatorConfig).WillOnce(Throw(expected_err));

  CommandInfo cmd_info(kCmdSetup);
  SetCmdCallbackExpectaions(cmd_info, cb_expectation);

  EXPECT_EQ(tested_codr_.Execute(cmd_info), expected_result);
}


TEST_F(CodrTest, SetupFailingModule) {
  CmdErrorCode expected_result = kCmdFailed;
  std::string module_name("test");
  CoordinatorConfig expected_config;
  expected_config.module_configs.emplace_back(module_name, 10);
  ON_CALL(*mock_cfg_ldr_, LoadCoordinatorConfig).WillByDefault(SetArgReferee<0>(expected_config));

  // set expected callback cmd status(and corresponding init status).
  std::vector<CommandStatus> expected_cb_status;
  expected_cb_status.emplace_back(kCmdAccepted);  // command accepted confirmation msg.
  expected_cb_status.emplace_back(kCmdAccepted);
  expected_cb_status.back().init_status.emplace_back(module_name, kModuleInitFailed);
  expected_cb_status.back().module_status.emplace_back(module_name, kModuleError, "an error msg");
  expected_cb_status.emplace_back(expected_result);
  CmdCbStatusExpectation cb_expectation(expected_cb_status);

  CommandInfo cmd_info(kCmdSetup);
  SetInitWorkflowExpectations(cmd_info, cb_expectation);

  EXPECT_EQ(tested_codr_.Execute(cmd_info), expected_result);
}


TEST_F(CodrTest, SetupWithMaxMonitorCount) {
  CmdErrorCode expected_result = kCmdFailed;
  CoordinatorConfig cfg;
  cfg.module_configs.emplace_back("test", 10);
  ON_CALL(*mock_cfg_ldr_, LoadCoordinatorConfig).WillByDefault(SetArgReferee<0>(cfg));

  // set expected callback cmd status(and corresponding init status).
  std::vector<CommandStatus> expected_cb_status;
  expected_cb_status.emplace_back(kCmdAccepted);  // command accepted confirmation msg.
  expected_cb_status.emplace_back(kCmdAccepted);
  expected_cb_status.emplace_back(kCmdAccepted);
  expected_cb_status.emplace_back(kCmdAccepted);
  expected_cb_status.emplace_back(expected_result);
  CmdCbStatusExpectation cb_expectation(expected_cb_status);

  CommandInfo cmd_info(kCmdSetup);
  cmd_info.max_setup_monitoring_count = 3;
  SetInitWorkflowExpectations(cmd_info, cb_expectation);

  EXPECT_EQ(tested_codr_.Execute(cmd_info), expected_result);
}


TEST_F(CodrTest, ResetModules) {
  CmdErrorCode expected_cmd_result = kCmdSucceed;
  CoordinatorConfig cfg;
  cfg.module_configs.emplace_back("test_a", 10);
  cfg.module_configs.emplace_back("test_b", 10);
  for (const auto& m : cfg.module_configs) mock_conns_[m.name] = std::make_shared<MockConnection>();
  std::vector<CommandStatus> expected_cb_status;
  Trans2Setup(cfg, &expected_cb_status);

  CommandInfo cmd_info(kCmdSetup);
  // cmd_info.reset_params.ego_path.emplace_back(1, 2, 3);
  // cmd_info.reset_params.ego_path.emplace_back(4, 5, 6);
  // cmd_info.reset_params.ego_initial_velocity = 10;
  // cmd_info.reset_params.ego_initial_theta = 1.5;
  // cmd_info.reset_params.ego_speed_limit = 30;
  cmd_info.reset_params.geo_fence.emplace_back(11.1, 22.2);
  cmd_info.reset_params.geo_fence.emplace_back(33.3, 44.4);
  cmd_info.reset_params.map_path = "/some/path.map";
  cmd_info.reset_params.map_local_origin[0] = 1.2;
  cmd_info.reset_params.map_local_origin[1] = 2.3;
  cmd_info.reset_params.map_local_origin[2] = 3.4;

  ModuleResetRequest expected_req;
  expected_req = cmd_info.reset_params;
  AssembleInitialLocation(expected_req);
  ModuleResetResponse expected_resp;
  expected_resp.type = kModuleOK;

  expected_cb_status.emplace_back(expected_cmd_result);
  for (const auto& m : cfg.module_configs)
    expected_cb_status.back().module_status.emplace_back(m.name, expected_resp.type, expected_resp.err);
  CmdCbStatusExpectation cb_expectation(expected_cb_status);
  SetCmdCallbackExpectaions(cmd_info, cb_expectation);

  for (const auto& m : cfg.module_configs) {
    EXPECT_CALL(*mock_conns_[m.name], Send(ResetParamsEqualsTo(ByRef(expected_req)))).WillOnce(Return(true));
    EXPECT_CALL(*mock_conns_[m.name], Receive).WillOnce(DoAll(SetArgReferee<0>(expected_resp), Return(true)));
  }

  EXPECT_EQ(tested_codr_.Execute(cmd_info), kCmdSucceed);
}


/**
 * test target:
 * when the coordinator reset a module and the module throws some error, the coordinator should then automatically
 * stop the module and report the module name and error msg in command status callback.
 */
TEST_F(CodrTest, ResetFailingModule) {
  CmdErrorCode expected_cmd_result = kCmdFailed;
  CoordinatorConfig cfg;
  std::string name("test");
  cfg.module_configs.emplace_back(name, 10);
  mock_conns_[name] = std::make_shared<MockConnection>();
  std::vector<CommandStatus> expected_cb_status;
  Trans2Setup(cfg, &expected_cb_status);

  CommandInfo cmd_info(kCmdSetup);
  ModuleResetRequest expected_reset_req;
  expected_reset_req = cmd_info.reset_params;
  AssembleInitialLocation(expected_reset_req);
  ModuleResetResponse expected_reset_resp;
  expected_reset_resp.type = kModuleError;
  expected_reset_resp.err = "module reset test error.";
  ModuleStopRequest expected_stop_req;
  ModuleStopResponse default_stop_resp;

  expected_cb_status.emplace_back(expected_cmd_result);
  expected_cb_status.back().module_status.emplace_back(name, expected_reset_resp.type, expected_reset_resp.err);
  CmdCbStatusExpectation cb_expectation(expected_cb_status);
  SetCmdCallbackExpectaions(cmd_info, cb_expectation);

  InSequence module_call_seq;
  EXPECT_CALL(*mock_conns_[name], Send(ResetParamsEqualsTo(ByRef(expected_reset_req)))).WillOnce(Return(true));
  EXPECT_CALL(*mock_conns_[name], Receive).WillOnce(DoAll(SetArgReferee<0>(expected_reset_resp), Return(true)));
  EXPECT_CALL(*mock_conns_[name], Send(StopParamsEqualsTo(ByRef(expected_stop_req)))).WillOnce(Return(true));
  EXPECT_CALL(*mock_conns_[name], Receive).WillOnce(DoAll(SetArgReferee<0>(default_stop_resp), Return(true)));

  EXPECT_EQ(tested_codr_.Execute(cmd_info), expected_cmd_result);
}


/**
 * test target:
 * calling two modules' step and both returns successfully. subscribed topics should direct to the corresponding modules
 * and the published topics should be updated into system. command returns succeed.
 */
TEST_F(CodrTest, StepModules) {
  CmdErrorCode expected_cmd_result = kCmdSucceed;
  CoordinatorConfig cfg;
  cfg.module_configs.emplace_back("a", 10);
  cfg.module_configs.emplace_back("b", 10);
  for (const auto& m : cfg.module_configs) mock_conns_[m.name] = std::make_shared<NiceMock<MockConnection>>();
  std::vector<std::string>& ma_subs = pubsubs_["a"].sub_topics;
  std::vector<std::string>& ma_pubs = pubsubs_["a"].pub_topics;
  std::vector<std::string>& mb_subs = pubsubs_["b"].sub_topics;
  std::vector<std::string>& mb_pubs = pubsubs_["b"].pub_topics;
  // module a sub <- [t0, t1, t2], pub -> [t3].
  ma_subs.emplace_back("t0"), ma_subs.emplace_back("t1"), ma_subs.emplace_back("t2");
  ma_pubs.emplace_back("t3");
  // module b sub <- [t0, t3], pub -> [t1, t4].
  mb_subs.emplace_back("t0"), mb_subs.emplace_back("t3");
  mb_pubs.emplace_back("t1"), mb_pubs.emplace_back("t4");
  Trans2Reset(cfg);

  std::vector<CommandStatus> expected_cb_status[2];
  ModuleStepRequest ma_req[2], mb_req[2];
  ModuleStepResponse resp;
  resp.type = kModuleOK, resp.time_cost = 6;
  zmq::message_t t0_msg("outter space", 13);
  MockConnection& ma_conn = *mock_conns_["a"];
  MockConnection& mb_conn = *mock_conns_["b"];
  for (auto& kv : mock_conns_) ON_CALL(*kv.second, Receive).WillByDefault(Return(true));

  auto SetStepExpectations = [&](size_t s, uint64_t t, const std::vector<std::string>& msgs) {
    // module a expected request/response.
    ma_req[s].sim_time = t;
    ma_req[s].messages["t0"].copy(t0_msg);
    if (!msgs[0].empty()) ma_req[s].messages["t1"].rebuild(msgs[0].c_str(), msgs[0].size());
    EXPECT_CALL(ma_conn, Send(StepParamsEqualsTo(ByRef(ma_req[s])))).WillOnce(Return(true));
    resp.messages.clear();
    resp.messages["t3"].rebuild(msgs[1].c_str(), msgs[1].size());
    EXPECT_CALL(ma_conn, Receive).WillOnce(DoAll(SetArgReferee<0>(resp), Return(true)));
    // module b expected request/response.
    mb_req[s].sim_time = t;
    mb_req[s].messages["t0"].copy(t0_msg);
    mb_req[s].messages["t3"].rebuild(msgs[1].c_str(), msgs[1].size());
    EXPECT_CALL(mb_conn, Send(StepParamsEqualsTo(ByRef(mb_req[s])))).WillOnce(Return(true));
    resp.messages.clear();
    resp.messages["t1"].rebuild(msgs[2].c_str(), msgs[2].size());
    resp.messages["t4"].rebuild(msgs[3].c_str(), msgs[3].size());
    EXPECT_CALL(mb_conn, Receive).WillOnce(DoAll(SetArgReferee<0>(resp), Return(true)));
    // command status.
    expected_cb_status[s].emplace_back(kCmdAccepted);  // first confirm msg.
    expected_cb_status[s].emplace_back(kCmdSucceed);
    auto& ms = expected_cb_status[s].back().module_status;
    ms.insert(ms.end(), {{"a", kModuleOK, ""}, {"b", kModuleOK, ""}});
    ms[0].elapsed_time = ms[1].elapsed_time = resp.time_cost;
    expected_cb_status[s].back().step_message.timestamp = t;
    zmq::message_t msg_t1(msgs[2].c_str(), msgs[2].size()), msg_t3(msgs[1].c_str(), msgs[1].size()),
        msg_t4(msgs[3].c_str(), msgs[3].size());
    zmq::message_t t0_msg_copy;
    t0_msg_copy.copy(t0_msg);
    expected_cb_status[s].back().step_message.messages.emplace_back(EgoTopic("ego1", "t0"), std::move(t0_msg_copy));
    expected_cb_status[s].back().step_message.messages.emplace_back(EgoTopic("ego1", "t1"), std::move(msg_t1));
    expected_cb_status[s].back().step_message.messages.emplace_back(EgoTopic("ego1", "t3"), std::move(msg_t3));
    expected_cb_status[s].back().step_message.messages.emplace_back(EgoTopic("ego1", "t4"), std::move(msg_t4));
  };

  InSequence module_call_seq;
  SetStepExpectations(0, 0, {"", "ma@t3@s0", "mb@t1@s0", "mb@t4@s0"});           // step 0
  SetStepExpectations(1, 10, {"mb@t1@s0", "ma@t3@s1", "mb@t1@s1", "mb@t4@s1"});  // step 1

  for (size_t s = 0; s < 2; ++s) {
    CommandInfo cmd_info(kCmdStep);
    // cmd_info.input_msgs["t0"].copy(t0_msg);
    CmdCbStatusExpectation cb_expectation(expected_cb_status[s]);
    SetCmdCallbackExpectaions(cmd_info, cb_expectation);
    EXPECT_EQ(tested_codr_.Execute(cmd_info), expected_cmd_result);
  }
}


/**
 * test target:
 * stop current module(s) and returned succeed with module feedbacks recorded.
 */
TEST_F(CodrTest, StopModuleWithFeedback) {
  CmdErrorCode expected_cmd_result = kCmdScenarioStopped;
  CoordinatorConfig cfg;
  cfg.module_configs.emplace_back("a", 10);
  mock_conns_["a"] = std::make_shared<NiceMock<MockConnection>>();
  Trans2Reset(cfg);

  auto& conn = *mock_conns_["a"];
  ModuleStopResponse stop_resp;
  stop_resp.type = kModuleOK;
  stop_resp.feedbacks["foo"] = "bar";
  stop_resp.feedbacks["yez"] = "zen";

  CommandInfo cmd_info(kCmdStop);
  std::vector<CommandStatus> expected_cb_status;
  expected_cb_status.emplace_back(kCmdAccepted);
  expected_cb_status.emplace_back(expected_cmd_result);
  expected_cb_status.back().module_status.emplace_back("a", stop_resp.type, stop_resp.err);
  expected_cb_status.back().module_status.back().feedback = "{\"foo\":\"bar\",\"yez\":\"zen\"}";
  CmdCbStatusExpectation cb_expectation(expected_cb_status);
  SetCmdCallbackExpectaions(cmd_info, cb_expectation);

  EXPECT_CALL(conn, Send(IsStopRequest())).WillOnce(Return(true));
  EXPECT_CALL(conn, Receive).WillOnce(DoAll(SetArgReferee<0>(stop_resp), Return(true)));

  EXPECT_EQ(tested_codr_.Execute(cmd_info), expected_cmd_result);
}


/**
 * test target:
 * If last scenario is not stopped manually, the next setup command should execute stop first automatically.
 * When auto-stop modules on setup command, the callback should be called with multiple kCmdAccepted code and ends with
 * a single non-accepted code after the scenario reset. The auto stop command should returns kCmdAccepted code when
 * succeed, not kCmdSucceed, to allow following callbacks to continue send back to the front-end correctly.
 */
TEST_F(CodrTest, AutoStopModulesOnSetup) {
  CmdErrorCode expected_cmd_result = kCmdSucceed;  // in this case we assume it is succeed.
  size_t expected_min_setup_cb_count = 3;          // auto stop -> init workflow -> setup succeed.

  CoordinatorConfig cfg;
  cfg.module_configs.emplace_back("a", 10);
  mock_conns_["a"] = std::make_shared<NiceMock<MockConnection>>();
  Trans2Reset(cfg);

  auto& conn = *mock_conns_["a"];
  ModuleStopResponse expected_stop_resp;
  expected_stop_resp.type = kModuleOK;
  ModuleResetResponse expected_reset_resp;
  expected_reset_resp.type = kModuleOK;
  {
    InSequence module_call_seq;
    EXPECT_CALL(conn, Send(IsStopRequest())).WillOnce(Return(true));  // the auto stop request.
    EXPECT_CALL(conn, Receive).WillOnce(DoAll(SetArgReferee<0>(expected_stop_resp), Return(true)));
    EXPECT_CALL(conn, Send(IsResetRequest())).WillOnce(Return(true));
    EXPECT_CALL(conn, Receive).WillOnce(DoAll(SetArgReferee<0>(expected_reset_resp), Return(true)));
  }

  CommandInfo cmd_info(kCmdSetup);
  size_t setup_cb_count = 0;
  bool non_accepted_code_returned = false;
  cmd_info.status_cb = [&setup_cb_count, &non_accepted_code_returned](const CommandStatus& status) {
    ++setup_cb_count;
    if (non_accepted_code_returned)
      ASSERT_FALSE(non_accepted_code_returned);  // should not returns non-accepted code anymore.
    else if (status.ec != kCmdAccepted)
      non_accepted_code_returned = true;
  };

  EXPECT_EQ(tested_codr_.Execute(cmd_info), expected_cmd_result);
  EXPECT_GE(setup_cb_count, expected_min_setup_cb_count);
}


/**
 * test target:
 * running one module while the module request a stop, running command should finished and auto-stop and returned
 * kCmdScenarioStopped.
 */
TEST_F(CodrTest, RunModuleThenScenarioStopped) {
  CmdErrorCode expected_cmd_result = kCmdScenarioStopped;
  CoordinatorConfig cfg;
  cfg.module_configs.emplace_back("a", 10);
  mock_conns_["a"] = std::make_shared<NiceMock<MockConnection>>();
  Trans2Reset(cfg);

  auto& conn = *mock_conns_["a"];
  ModuleStepResponse step_resp;
  ModuleStopResponse stop_resp;
  {
    InSequence module_call_seq;
    // two steps(succeed -> require-stop) ...
    step_resp.type = kModuleOK;
    EXPECT_CALL(conn, Send(IsStepRequest())).WillOnce(Return(true));
    EXPECT_CALL(conn, Receive).WillOnce(DoAll(SetArgReferee<0>(step_resp), Return(true)));
    step_resp.type = kModuleRequireStop;
    EXPECT_CALL(conn, Send(IsStepRequest())).WillOnce(Return(true));
    EXPECT_CALL(conn, Receive).WillOnce(DoAll(SetArgReferee<0>(step_resp), Return(true)));
    // auto stop ...
    stop_resp.type = kModuleOK;
    EXPECT_CALL(conn, Send(IsStopRequest())).WillOnce(Return(true));
    EXPECT_CALL(conn, Receive).WillOnce(DoAll(SetArgReferee<0>(stop_resp), Return(true)));
  }

  CommandInfo cmd_info(kCmdRun);
  EXPECT_EQ(tested_codr_.Execute(cmd_info), expected_cmd_result);
}

}  // namespace test
}  // namespace tx_sim
