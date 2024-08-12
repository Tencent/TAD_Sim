/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "open_drive_tpl.h"
#include "open_scenario_tpl.h"
#include "sim_tpl.h"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"
#include "gtest/gtest.h"

TEST(XmlTplTest, ForSim) {
  std::istringstream iss(R"(
<?xml version="1.0" encoding="utf-8"?>
<simulation version="1.0" date_version="1685497100916" sim_id="15">
    <traffic>L4_Collision_SECURITY_20231124112315.simrec</traffic>
    <mapfile lon="106.48565684" lat="-0.024283329999999999" alt="-4.9999999999999998e-08" unrealLevelIndex="0">
        ../hadmap/re_project_mannul_0414.xodr
    </mapfile>
    <planner>
        <ego type="car" name="suv"/>
        <altitude start="3.6973506212234497e-07" end="-0"/>
        <route id="0" type="start_end" start="106.48394129730637,-0.02030009262801" 
        end="106.48450094382468,-0.01998795601796" mid=""/>
        <start_v>0.1</start_v>
        <theta>0.51619234506524969</theta>
        <Velocity_Max>20</Velocity_Max>
        <control_longitudinal>true</control_longitudinal>
        <control_lateral>true</control_lateral>
        <acceleration_max>12</acceleration_max>
        <deceleration_max>10</deceleration_max>
        <trajectory_enabled>false</trajectory_enabled>
        <scene_event version="1.1.0.0"/>
        <InputPath points=""/>
        <ControlPath points="" sampleInterval=""/>
    </planner>
</simulation>
  )");
  SIM::ENG::SimulationPtr sim = SIM::ENG::Load(iss);
  EXPECT_EQ(sim->mapfile.path, "../hadmap/re_project_mannul_0414.xodr");

  sim->mapfile.path = "../hadmap/re_project_mannul_0414.xodr";
  std::cout << SIM::ENG::Dump(sim) << std::endl;
  iss.str(SIM::ENG::Dump(sim));

  sim = SIM::ENG::Load(iss);
  EXPECT_EQ(sim->mapfile.path, "../hadmap/re_project_mannul_0414.xodr");
}

TEST(XmlTplTest, ForPtree) {
  boost::property_tree::ptree pt;
  pt.put("node1", "value1");
  pt.put("node2", "value2");

  pt.put("node2", "");
  pt.put_child("node2", boost::property_tree::ptree());

  std::stringstream ss;
  boost::property_tree::xml_parser::write_xml(ss, pt);
  EXPECT_EQ(ss.str(), "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<node1>value1</node1><node2/>");
}