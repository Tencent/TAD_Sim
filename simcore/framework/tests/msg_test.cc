#include "gtest/gtest.h"

#include "coordinator/config.h"
#include "coordinator/scenario_parser.h"
#include "coordinator/status.h"
#include "utils.h"
#include "utils/msgs.h"


#define TXSIM_TEST_COMPARE_DECODED_ZMQ_MSG(type, expected) \
  do {                                                     \
    type decoded;                                          \
    zmq::multipart_t msg;                                  \
    expected.Encode(msg);                                  \
    decoded.Decode(msg);                                   \
    EXPECT_EQ(decoded, expected);                          \
    EXPECT_TRUE(msg.empty());                              \
  } while (false);


using namespace tx_sim::coordinator;
using namespace tx_sim::impl;


namespace tx_sim {
namespace test {

/**************************** module message encoding/decoding **************************/

TEST(ModuleMsgTest, EnDecodingInitRequest) {
  ModuleInitRequest expected;
  expected.init_args["foo"] = "bar";
  expected.init_args["foo_again"] = "baz";
  TXSIM_TEST_COMPARE_DECODED_ZMQ_MSG(ModuleInitRequest, expected)
}

TEST(ModuleMsgTest, EnDecodingInitResponse) {
  ModuleInitResponse expected;
  expected.type = kModuleReInit;
  expected.err = "blablabla";
  expected.sub_topics.insert({"t1", "t2"});
  expected.pub_topics.insert("t3");
  expected.sub_shmems.insert("shm003");
  expected.pub_shmems.emplace("shm_xyz", 2048);
  TXSIM_TEST_COMPARE_DECODED_ZMQ_MSG(ModuleInitResponse, expected)
}

TEST(ModuleMsgTest, EnDecodingResetRequest) {
  ModuleResetRequest expected;
  expected.scenario_path = "/path/to/some/scenario_file.sim";
  expected.map_path = "/path/to/corresponding/map_file.xodr";
  expected.map_local_origin[0] = 123.321;
  expected.map_local_origin[1] = 456.567;
  expected.map_local_origin[2] = 78.67;
  // expected.ego_path.emplace_back(12.3, 23.4, 34.5);
  // expected.ego_path.emplace_back(111, 222, 33.1);
  // expected.ego_path.emplace_back(432.23, 336.0, 29);
  // expected.ego_initial_velocity = 16.67;
  // expected.ego_initial_theta = 2.12;
  // expected.ego_speed_limit = 33.34;
  // expected.geo_fence.insert(expected.geo_fence.end(), {{12, 34}, {56, 78}});
  tx_sim::coordinator::AssembleInitialLocation(expected);
  TXSIM_TEST_COMPARE_DECODED_ZMQ_MSG(ModuleResetRequest, expected);
}

TEST(ModuleMsgTest, EnDecodingResetResponse) {
  ModuleResetResponse expected;
  expected.type = kModuleError;
  expected.err = "blablabla";
  TXSIM_TEST_COMPARE_DECODED_ZMQ_MSG(ModuleResetResponse, expected);
}

TEST(ModuleMsgTest, EnDecodingStepRequest) {
  ModuleStepRequest expected;
  expected.sim_time = 430;
  expected.messages["abc"].rebuild("xyz", 3);
  expected.messages["123"].rebuild("456", 3);
  expected.sub_topic_shmem_names["s1"] = "mem@234";
  expected.pub_topic_shmem_names["s2"] = "mem@345";
  TXSIM_TEST_COMPARE_DECODED_ZMQ_MSG(ModuleStepRequest, expected);
}

TEST(ModuleMsgTest, EnDecodingStepResponse) {
  ModuleStepResponse expected;
  expected.type = kModuleRequireStop;
  expected.err = "blablabla";
  expected.time_cost = 430;
  expected.messages["abc"].rebuild("xyz", 3);
  expected.messages["123"].rebuild("456", 3);
  TXSIM_TEST_COMPARE_DECODED_ZMQ_MSG(ModuleStepResponse, expected);
}

TEST(ModuleMsgTest, EnDecodingStopRequest) {
  ModuleStopRequest expected;
  TXSIM_TEST_COMPARE_DECODED_ZMQ_MSG(ModuleStopRequest, expected);
}

TEST(ModuleMsgTest, EnDecodingStopResponse) {
  ModuleStopResponse expected;
  expected.type = kModuleSystemError;
  expected.err = "blablabla";
  expected.feedbacks["v"] = "100";
  expected.feedbacks["xxx"] = "cat";
  TXSIM_TEST_COMPARE_DECODED_ZMQ_MSG(ModuleStopResponse, expected);
}

/**************************** config message encoding/decoding **************************/

TEST(ConfigMsgTest, EnDecodingCoordinatorConfig) {
  CoordinatorConfig expected, decoded;
  expected.control_rate = 90;
  expected.scenario_time_limit = 600;
  expected.coord_mode = kCoordinateAsync;
  expected.module_configs.emplace_back("ma", 20, 8);
  auto& m1 = expected.module_configs.back();
  m1.init_args["foo"] = "bar";
  m1.conn_args.cmd_timeout = 1000;
  m1.conn_args.step_timeout = 200;
  m1.auto_launch = true;
  m1.dep_paths.emplace_back("path1");
  m1.dep_paths.emplace_back("path2");
  m1.so_path = "/path/to/shared_lib.so";
  expected.module_configs.emplace_back("mb", 100, 40);
  auto& m2 = expected.module_configs.back();
  m2.init_args["k1"] = "v1";
  m2.init_args["k2"] = "v2";
  m2.bin_path = "/path/to/executable";
  m2.bin_args.insert(m2.bin_args.end(), {"--foo", "bar", "-a", "20", "xyz"});
  std::string jstr = expected.EncodeToStr();
  decoded.DecodeFromStr(jstr);
  EXPECT_EQ(decoded.control_rate, expected.control_rate);
  EXPECT_EQ(decoded.scenario_time_limit, expected.scenario_time_limit);
  EXPECT_EQ(decoded.coord_mode, expected.coord_mode);
  for (size_t i = 0; i < expected.module_configs.size(); ++i)
    EXPECT_EQ(decoded.module_configs[i], expected.module_configs[i]);
}

/**************************** status message encoding/decoding **************************/

TEST(StatusMsgTest, EnDecodingCommandStatus) {
  CommandStatus expected;
  expected.ec = kCmdScenarioStopped;
  expected.total_time_cost = 1540;
  expected.module_status.emplace_back("ma", kModuleError, "blabla");
  auto& ms = expected.module_status.back();
  ms.elapsed_time = 14;
  ms.feedback = "{\"foo\":\"bar\",\"hello\":\"world\"}";
  expected.module_status.emplace_back("mb", kModuleProcessExit, "killed");
  expected.init_status.emplace_back("ma", kModuleInitSucceed);
  auto& ti = expected.init_status.back().topic_info;
  ti.sub_topics.insert(ti.sub_topics.end(), {"t1", "t2", "t3"});
  ti.pub_topics.insert(ti.pub_topics.end(), {"t4", "t5"});
  ti.sub_shmems.insert(ti.sub_shmems.end(), {"s1", "s2"});
  ti.pub_shmems.emplace_back("s3", 1024);
  expected.init_status.emplace_back("mb", kModuleInitFailed);
  expected.step_message.timestamp = 640;
  auto& sms = expected.step_message.messages;
  zmq::message_t sm("asdfg", 5);
  sms.emplace_back(EgoTopic("ego1", "t1"), std::move(sm));
  sm.rebuild("bbc", 3);
  sms.emplace_back(EgoTopic("ego1", "t2"), std::move(sm));
  TXSIM_TEST_COMPARE_DECODED_ZMQ_MSG(CommandStatus, expected);
}

}  // namespace test
}  // namespace tx_sim