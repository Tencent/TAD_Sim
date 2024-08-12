#include <math.h>

#include "gtest/gtest.h"


#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <unordered_map>
#include "boost/foreach.hpp"


namespace fs = boost::filesystem;


void GetRouteFromSceneXML(std::string cloudcity_simfile, int nEgoId) {
  using boost::property_tree::ptree;
  ptree pt;
  read_xml(cloudcity_simfile, pt);
  std::unordered_map<std::string, std::string> locMap;
  BOOST_FOREACH (const ptree::value_type& v, pt.get_child("traffic.locations")) {
    if (v.first == "<xmlcomment>") continue;
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

      std::cout << beginPosX << std::endl;
      std::cout << beginPosX << std::endl;
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
      std::cout << endPosX << std::endl;
      std::cout << endPosY << std::endl;
    }
  }
}


namespace tx_sim {
namespace test {

TEST(CloudCity, ParsingSimFile) {
  GetRouteFromSceneXML("E:\\data\\WXWork\\1688855893371377\\Cache\\File\\2024-03\\test0315.xml", 2);
}


}  // namespace test
}  // namespace tx_sim