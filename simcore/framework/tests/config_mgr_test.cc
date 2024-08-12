#include "coordinator/config_mgr.h"
#include <math.h>
#include "coordinator_test.h"
#include "gtest/gtest.h"
#include "utils.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <unordered_map>


using namespace tx_sim::coordinator;
using namespace tx_sim::impl;
namespace fs = boost::filesystem;


namespace tx_sim {
namespace test {
TEST(ConfigMgrTest, ParsingOpenScenario) {
  fs::path testResPath = GetTestResourceDirPath();
  fs::path databasePath = testResPath / "data/service.sqlite";
  fs::path configPath = testResPath / "sys/local_service.config";
  tx_sim::coordinator::ConfigManager configMgr(databasePath.lexically_normal().string());
  configMgr.InitConfigData(configPath.lexically_normal().string());
}


}  // namespace test
}  // namespace tx_sim