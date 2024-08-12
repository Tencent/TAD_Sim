#include <math.h>

#include "gtest/gtest.h"

#include "coordinator/enviroment_parser.h"
#include "coordinator/scenario_parser.h"
#include "coordinator_test.h"
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

/*
typedef decltype(ModuleResetRequest::ego_path) EgoPathList;


static void CompareEgoPath(const EgoPathList& actual_path, const EgoPathList& expected_path, bool compare_z) {
for (size_t i = 0; i < expected_path.size(); ++i) {
const auto& actual = actual_path[i];
const auto& expected = expected_path[i];
EXPECT_FLOAT_EQ(std::get<0>(actual), std::get<0>(expected));
EXPECT_FLOAT_EQ(std::get<1>(actual), std::get<1>(expected));
if (compare_z) EXPECT_FLOAT_EQ(std::get<2>(actual), std::get<2>(expected));
}
}
*/

void TestScenaroParsing(const std::string& scenario_name) {
  ModuleResetRequest ret;
  auto parser = ScenarioXmlParser();
  EXPECT_TRUE(parser.ParseScenario(scenario_name, ret));
  // EXPECT_FLOAT_EQ(ret.ego_initial_theta, 1.278641);
  // EXPECT_EQ(ret.ego_initial_velocity, 4.17);
  // EXPECT_EQ(ret.ego_speed_limit, 20);
  // EXPECT_EQ(ret.map_path, xodr_path.normalize().make_preferred().string());
  // EXPECT_EQ(ret.ego_path.size(), 3);
  // EgoPathList expected_ego_path{std::make_tuple(112.204624, 32.133083, 9.812),    // start point
  //                               std::make_tuple(112.205064, 32.134253, 72.609),   // 2nd way point
  //                               std::make_tuple(112.206406, 32.137916, 72.554)};  // end point
  // CompareEgoPath(ret.ego_path, expected_ego_path, boost::algorithm::ends_with(scenario_name, ".sim"));
}
struct Key {
  Key() { std::cout << "111" << std::endl; }
  std::string first;
  std::string second;
};

struct KeyHash {
  std::size_t operator()(const Key& k) const {
    return std::hash<std::string>()(k.first) ^ (std::hash<std::string>()(k.second) << 1);
  }
};

struct KeyEqual {
  bool operator()(const Key& lhs, const Key& rhs) const { return lhs.first == rhs.first && lhs.second == rhs.second; }
};


TEST(ScenarioParserTest, ParsingSimScenario) {
  // TestScenaroParsing("test.sim");
  Key key;
  std::unordered_map<Key, std::string, KeyHash, KeyEqual> map;
  TestScenaroParsing(
      R"(E:\workspace\SimCore-window-devgroup\framework_v2\tests\resources\data\scenario\scene\t-label2.sim)");
  return;
  fs::path fSimPath = GetTestResourceDirPath() / "scene";
  fs::directory_iterator endIter;
  for (fs::directory_iterator iter(fSimPath); iter != endIter; iter++) {
    if (!boost::filesystem::is_directory(*iter)) {
      std::string extension = iter->path().extension().string();
      std::string sFullPath = iter->path().string();
      if (extension == ".sim") { TestScenaroParsing(sFullPath); }
    }
  }
}

TEST(ScenarioParserTest, ParsingEnviroment) {
  // TestScenaroParsing("test.sim");
  CEnviromentParser envParser("D:\\workspace\\SimCore\\framework_v2\\tests\\resources\\scene\\environment.xml");
  sim_msg::Scene scene;
  envParser.BuildEnviroment(scene.mutable_environment());
  std::cout << scene.DebugString() << std::endl;
}

void GetRouteFromSceneXML(int nEgoId) {
  std::cout << "eEgoID------------------" << nEgoId << std::endl;
  using boost::property_tree::ptree;
  ptree pt;
  read_xml("C:\\Users\\fangccheng\\Downloads\\scenes (10)\\TEST_0414_0606_signlights.xml", pt);
  std::unordered_map<std::string, std::string> locMap;
  BOOST_FOREACH (const ptree::value_type& v, pt.get_child("traffic.locations")) {
    // if (v.first == "<xmlcomment>") continue;
    std::string id = v.second.get<std::string>("<xmlattr>.id");
    std::string sPos = v.second.get<std::string>("<xmlattr>.pos");
    locMap[id] = sPos;
  }


  BOOST_FOREACH (const ptree::value_type& v, pt.get_child("traffic.ego.egoInput")) {
    if (v.first == "<xmlcomment>") continue;

    auto nEgoInputSize = pt.get_child("traffic.ego.egoInput").size();
    int id = v.second.get<int>("<xmlattr>.id");

    if (nEgoId > nEgoInputSize) {
      while ((nEgoId = nEgoId % nEgoInputSize) > nEgoInputSize) {}
    }

    if (nEgoId == 0) nEgoId = nEgoInputSize;

    if (nEgoId <= nEgoInputSize) {
      if (id != nEgoId) continue;
    }

    // if (nEgoId > nEgoInputSize && (nEgoId % nEgoInputSize != id) && (nEgoId % nEgoInputSize != 0)) continue;
    std::cout << "egoID:" << nEgoId << std::endl;
    std::cout << "egoinputID:" << id << std::endl;

    std::string sRoute = v.second.get<std::string>("<xmlattr>.route");
    std::vector<std::string> vecRoute;
    boost::split(vecRoute, sRoute, boost::is_any_of(","));

    BOOST_FOREACH (ptree::value_type& v, pt.get_child("traffic.ego.egoStart")) {
      if (v.first == "<xmlcomment>") continue;

      std::string id = v.second.get<std::string>("<xmlattr>.id");
      if (id != vecRoute[0]) continue;
      double startV = v.second.get<double>("<xmlattr>.start_v");
      double maxV = v.second.get<double>("<xmlattr>.max_v");
      double theta = v.second.get<double>("<xmlattr>.theta");


      std::string sStartlocationId = v.second.get<std::string>("<xmlattr>.location");
      std::vector<std::string> StartVecPos;
      std::string sStartLoc = locMap[sStartlocationId];
      boost::split(StartVecPos, sStartLoc, boost::is_any_of(","));
      double beginPosX = std::stod(StartVecPos[0]);
      double beginPosY = std::stod(StartVecPos[1]);
    }

    BOOST_FOREACH (ptree::value_type& v, pt.get_child("traffic.ego.egoEnd")) {
      if (v.first == "<xmlcomment>") continue;

      std::string id = v.second.get<std::string>("<xmlattr>.id");
      if (id != vecRoute[1]) continue;

      std::string sEndlocationId = v.second.get<std::string>("<xmlattr>.location");
      std::vector<std::string> EndVecPos;
      std::string sEndLoc = locMap[sEndlocationId];
      boost::split(EndVecPos, sEndLoc, boost::is_any_of(","));
      double endPosX = std::stod(EndVecPos[0]);
      double endPosY = std::stod(EndVecPos[1]);
    }
  }
}

TEST(ScenarioParserTest, ParsingOpenScenario) {
  try {
    for (int i = 1; i < 10; i++) { GetRouteFromSceneXML(i); }
  } catch (const std::exception& e) { std::cout << e.what() << std::endl; }

  return;
  TestScenaroParsing("test.xosc");
  TestScenaroParsing("test2.xosc");
}

}  // namespace test
}  // namespace tx_sim