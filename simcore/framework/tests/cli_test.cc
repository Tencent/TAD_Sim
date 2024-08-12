#include "gtest/gtest.h"

#include "cli/cli.hpp"
#include "cli/root_cmd.h"
#include "mocks.h"
#include "utils.h"
#include "utils/json_helper.h"

using ::testing::_;
using ::testing::ByRef;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Mock;
using ::testing::Return;
using namespace tx_sim::cli;

namespace tx_sim {
namespace test {

class CliRequestTest : public ::testing::Test {
 protected:
  const std::string config_json_str =
      "{\"name\":\"Traffic\",\n\"stepTime\":20,\n\"endpoint\":\"tcp://"
      "127.0.0.1:8888\",\n\"initArgs\":{},\n\"cmdTimeout\": 3000,\n\"stepTimeout\": 500,\n\"autoLaunch\": "
      "true,\n\"depPaths\": [\"/opt/TADSim/resources/app/buildin/simdeps\"],\n\"soPath\" : \"\",\n\"binPath\" : "
      "\"/opt/TADSim/resources/app/buildin/txSimTraffic\",\n\"binArgs\" : [\"tcp://127.0.0.1:8888\"]\n}";
  const std::string sys_config_json_str =
      "{\"playControlRate\": 200,\n\"scenarioTimeLimit\":60,\n\"coordinationMode\":2\n}";
  std::shared_ptr<MockClient> mock_client_;
  std::unique_ptr<RootCmd> root_cmd_;
  void SetUp() override {
    mock_client_ = std::make_shared<MockClient>();
    root_cmd_ = std::unique_ptr<RootCmd>(new RootCmd(nullptr, std::make_shared<MockClientFactory>(mock_client_)));
  }
};

/**
 * test cmd: txsim-cli conf show --mod
 * test target:
 * this would cause RootCmd return ArgumentMismatch error
 */
TEST_F(CliRequestTest, CLIArgsMismatchErr) {
  const char* argv[] = {"txsim-cli", "conf", "show", "--mod"};
  root_cmd_->ExecuteCmd(4, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCLIArgumentMismatch);
}

/**
 * test cmd: txsim-cli conf show -m sys
 * test target:
 * the GetSysConfigs method in class Client should be called with certain error code
 */
TEST_F(CliRequestTest, ConfShowSysRequest) {
  const char* argv[] = {"txsim-cli", "conf", "show", "-m", "sys"};
  EXPECT_CALL(*mock_client_, GetSysConfigs)
      .WillOnce(Return(tx_sim::impl::kCmdSucceed))
      .WillOnce(Return(tx_sim::impl::kCmdSystemError));
  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSucceed);

  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSystemError);
}

/**
 * test cmd: txsim-cli conf show --mod all
 * test target:
 * the GetAllModuleConfigs method in class Client should be called.
 */
TEST_F(CliRequestTest, ConfShowAllRequest) {
  const char* argv[] = {"txsim-cli", "conf", "show", "--mod", "all"};
  EXPECT_CALL(*mock_client_, GetAllModuleConfigs)
      .WillOnce(Return(tx_sim::impl::kCmdSucceed))
      .WillOnce(Return(tx_sim::impl::kCmdRejected));
  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSucceed);

  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdRejected);
}

/**
 * test cmd: txsim-cli conf show -m Traffic
 * test target:
 * the GetModuleConfigs method in class Client should be called with corresponding argument, which is Traffic here.
 */
TEST_F(CliRequestTest, ConfShowModuleRequest) {
  const char* argv[] = {"txsim-cli", "conf", "show", "-m", "Traffic"};
  EXPECT_CALL(*mock_client_, GetModuleConfig(Eq("Traffic"), _))
      .WillOnce(Return(tx_sim::impl::kCmdSucceed))
      .WillOnce(Return(tx_sim::impl::kCmdFailed));

  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSucceed);

  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdFailed);
}

/**
 * test cmd: txsim-cli conf restore
 * test target:
 * the RestoreDefaultConfigs method in class Client should be called.
 */
TEST_F(CliRequestTest, ConfRestoreRequest) {
  const char* argv[] = {"txsim-cli", "conf", "restore"};
  EXPECT_CALL(*mock_client_, RestoreDefaultConfigs)
      .WillOnce(Return(tx_sim::impl::kCmdSucceed))
      .WillOnce(Return(tx_sim::impl::kCmdSystemError));
  root_cmd_->ExecuteCmd(3, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSucceed);

  root_cmd_->ExecuteCmd(3, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSystemError);
}

/**
 * test cmd: txsim-cli conf rm --mod Planning
 * test target:
 * the RemoveModuleConfig method in class Client should be called with corresponding arguments, which is planning here.
 */
TEST_F(CliRequestTest, ConfRmRequest) {
  const char* argv[] = {"txsim-cli", "conf", "rm", "--mod", "Planning"};
  EXPECT_CALL(*mock_client_, RemoveModuleConfig(Eq("Planning")))
      .WillOnce(Return(tx_sim::impl::kCmdSucceed))
      .WillOnce(Return(tx_sim::impl::kCmdScenarioTimeout));
  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSucceed);

  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdScenarioTimeout);
}


/**
 * test cmd: txsim-cli conf upd -f test-upd.json
 * test target:
 * the UpdateModuleConfig method in class Client should be called with correct arguments, here the file content should
 * be the same with "config_json_str" after formatting with Json::Value
 */
TEST_F(CliRequestTest, ConfUpdRequestWithFile) {
  const char* argv[] = {"txsim-cli", "conf", "upd", "-f", "test-upd.json"};
  // create a temporary json file and delete after using it
  std::ofstream ofs("test-upd.json", std::ios::out);
  ofs << config_json_str;
  ofs.close();
  Json::Value expected_json;
  tx_sim::utils::ReadJsonDocFromString(expected_json, config_json_str);
  // The parameters much equals to the one converting directly from std::string
  EXPECT_CALL(*mock_client_, UpdateModuleConfig(JsonStrEqualsTo(ByRef(expected_json))))
      .WillOnce(Return(tx_sim::impl::kCmdSucceed))
      .WillOnce(Return(tx_sim::impl::kCmdServerBusy));
  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSucceed);

  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdServerBusy);
  std::remove("test-upd.json");
}

/**
 * test cmd: txsim-cli conf upd --config "config_string_str" (defined in class CliRequestTest)
 * test target:
 * the UpdateModuleConfig method in class Client should be called with correct arguments, which is the content of
 * config_string_str here
 */
TEST_F(CliRequestTest, ConfUpdRequestWithStr) {
  const char* argv[] = {"txsim-cli", "conf", "upd", "--config", config_json_str.c_str()};
  EXPECT_CALL(*mock_client_, UpdateModuleConfig(Eq(config_json_str)))
      .WillOnce(Return(tx_sim::impl::kCmdSucceed))
      .WillOnce(Return(tx_sim::impl::kCmdScenarioStopped));
  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSucceed);

  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdScenarioStopped);
}

/**
 * test cmd: txsim-cli conf add -f test-upd.json
 * test target:
 * the AddModuleConfig method in class Client should be called with correct arguments, here the file content should
 * be the same with "config_json_str" after formatting with Json::Value, the same as testng UpdateModuleConfig
 */
TEST_F(CliRequestTest, ConfAddRequestWithFile) {
  const char* argv[] = {"txsim-cli", "conf", "add", "-f", "test-add.json"};
  std::ofstream ofs("test-add.json", std::ios::out);
  ofs << config_json_str;
  ofs.close();
  Json::Value expected_json;
  tx_sim::utils::ReadJsonDocFromString(expected_json, config_json_str);
  EXPECT_CALL(*mock_client_, AddModuleConfig(JsonStrEqualsTo(ByRef(expected_json))))
      .WillOnce(Return(tx_sim::impl::kCmdSucceed))
      .WillOnce(Return(tx_sim::impl::kCmdSystemError));
  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSucceed);

  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSystemError);
  std::remove("test-add.json");
}

/**
 * test cmd: txsim-cli conf add -c "config_string_str" (defined in class CliRequestTest)
 * test target:
 * the AddModuleConfig method in class Client should be called with correct arguments, which is the content of
 * config_string_str here
 */
TEST_F(CliRequestTest, ConfAddRequestWithStr) {
  const char* argv[] = {"txsim-cli", "conf", "add", "--config", config_json_str.c_str()};
  EXPECT_CALL(*mock_client_, AddModuleConfig(Eq(config_json_str)))
      .WillOnce(Return(tx_sim::impl::kCmdSucceed))
      .WillOnce(Return(tx_sim::impl::kCmdSystemError));
  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSucceed);

  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSystemError);
}

/**
 * test cmd: txsim-cli conf set-sys -f test-set-sys.json
 * test target:
 * the SetSysConfigs method in class Client should be called with correct arguments, here the file content should
 * be the same with "config_json_str" after formatting with Json::Value, the same as testng UpdateModuleConfig
 */
TEST_F(CliRequestTest, ConfSetSysRequestWithFile) {
  const char* argv[] = {"txsim-cli", "conf", "set-sys", "-f", "test-set-sys.json"};
  std::ofstream ofs("test-set-sys.json", std::ios::out);
  ofs << sys_config_json_str;
  ofs.close();
  Json::Value expected_json;
  tx_sim::utils::ReadJsonDocFromString(expected_json, sys_config_json_str);
  EXPECT_CALL(*mock_client_, SetSysConfigs(JsonStrEqualsTo(ByRef(expected_json))))
      .WillOnce(Return(tx_sim::impl::kCmdSucceed))
      .WillOnce(Return(tx_sim::impl::kCmdSystemError));
  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSucceed);

  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSystemError);
  std::remove("test-set-sys.json");
}

/**
 * test cmd: txsim-cli conf set-sys -c "sys_config_string_str" (defined in class CliRequestTest)
 * test target:
 * the SetSysConfigs method in class Client should be called with correct arguments, which is the content of
 * sys_config_string_str here
 */
TEST_F(CliRequestTest, ConfSetSysRequestWithStr) {
  const char* argv[] = {"txsim-cli", "conf", "set-sys", "--config", sys_config_json_str.c_str()};
  EXPECT_CALL(*mock_client_, SetSysConfigs(Eq(sys_config_json_str)))
      .WillOnce(Return(tx_sim::impl::kCmdSucceed))
      .WillOnce(Return(tx_sim::impl::kCmdSystemError));
  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSucceed);

  root_cmd_->ExecuteCmd(5, argv);
  EXPECT_EQ(root_cmd_->error_code(), tx_sim::cli::kCmdSystemError);
}

}  // namespace test
}  // namespace tx_sim
