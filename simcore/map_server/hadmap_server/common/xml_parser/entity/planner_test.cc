/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/planner.h"
#include "common/utils/misc.h"
#include "common/xml_parser/entity/traffic.h"

#include <tinyxml.h>
#include "gtest/gtest.h"

TEST(CPlannerTest, AdjustTypeAndName) {
  CPlanner planner;
  planner.AdjustTypeAndName("", nullptr, nullptr);
  EXPECT_TRUE(planner.m_strType == "car" && planner.m_strName == "suv");

  planner.AdjustTypeAndName("", "xxxx", nullptr);
  EXPECT_TRUE(planner.m_strType == "car" && planner.m_strName == "suv");

  planner.AdjustTypeAndName("", "AIV_Empty_001", "");
  EXPECT_TRUE(planner.m_strType == "car" && planner.m_strName == "AIV_Empty_001");

  planner.AdjustTypeAndName("", "suv", "");
  EXPECT_TRUE(planner.m_strType == "car" && planner.m_strName == "suv");

  planner.AdjustTypeAndName("", "truck", "");
  EXPECT_TRUE(planner.m_strType == "combination" && planner.m_strName == "truck");

  planner.AdjustTypeAndName("/data/hdserver/cloud/resources/scenario/Catalogs/Vehicles/default.xosc", "truck",
                            "truck_truck1");
  EXPECT_EQ(planner.m_strType, "combination");
  EXPECT_EQ(planner.m_strName, "truck");

  planner.AdjustTypeAndName("/data/hdserver/cloud/resources/scenario/Catalogs/Vehicles/default.xosc", "car",
                            "AIV_FullLoad_001");
  EXPECT_TRUE(planner.m_strType == "car" && planner.m_strName == "AIV_FullLoad_001");
}

TEST(CPlannerTest, ParseSceneEvents) {
  TiXmlDocument xml;
  /*
  xml.Parse(R"(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <scene_event version="1.2.0.0">
      <event id="0" name="事件" type1="time_trigger" condition1="trigger_time:0.0,equation_op:gt" conditionNumber="1"
  endCondition="type:none,value:0.0" action='[{actionid:0,type:status,value:{&quot;obstacle&quot;:
  [{&quot;velocity&quot;: {&quot;y&quot;: -0.7623236179351807, &quot;x&quot;: 7.885900497436523, &quot;z&quot;: 0.0},
  &quot;acceleration&quot;: -1.0383177995681763, &quot;yaw&quot;: -0.113944411277771, &quot;yawRate&quot;: 0.0,
  &quot;age_frames&quot;: 42, &quot;movementClassification&quot;: 1, &quot;objectClassification&quot;: 4,
  &quot;varObjectSize&quot;: {&quot;y&quot;: 0.01845436543226242, &quot;x&quot;: 0.01269783079624176, &quot;z&quot;:
  0.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true,
  &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: 0.0, &quot;existenceProbability&quot;: 1.0, &quot;age&quot;:
  4, &quot;laneid&quot;: -1, &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;:
  0.09451983869075775, &quot;varPosition&quot;: {&quot;y&quot;: 0.16953231394290924, &quot;x&quot;: 0.520626962184906,
  &quot;z&quot;: 0.0}, &quot;angle&quot;: -0.113944411277771, &quot;varAngle&quot;: -0.113944411277771,
  &quot;objectSize&quot;: {&quot;x&quot;: 4.66974306113797, &quot;y&quot;: 2.001832484828182,
  &quot;z&quot;: 1.4940096139907837}, &quot;trackId&quot;: 14086, &quot;position&quot;: {&quot;x&quot;:
  754189.7874244766, &quot;y&quot;: 2549705.8749846453, &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;:
  2549705.863548142, &quot;x&quot;: 754192.1521622427, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549707.672607987,
  &quot;x&quot;: 754191.3238703157, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549705.8864211487, &quot;x&quot;:
  754187.4226867105, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549704.0773613034, &quot;x&quot;: 754188.2509786375,
  &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: -0.2520548105239868, &quot;x&quot;: 9.786653518676758,
  &quot;z&quot;: 0.0}, &quot;acceleration&quot;: -0.21786990761756897, &quot;yaw&quot;: -0.014117976650595665,
  &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 23, &quot;movementClassification&quot;: 1,
  &quot;objectClassification&quot;: 5, &quot;varObjectSize&quot;: {&quot;y&quot;: 0.012851402163505554, &quot;x&quot;:
  0.014190650545060635, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true,
  &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: 0.0,
  &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 2, &quot;laneid&quot;: -1, &quot;tail_lamp&quot;: 0,
  &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.037304189056158066, &quot;varPosition&quot;: {&quot;y&quot;:
  0.05033114179968834, &quot;x&quot;: 0.6511571407318115, &quot;z&quot;: 0.0}, &quot;angle&quot;: -0.014117976650595665,
  &quot;varAngle&quot;: -0.014117976650595665, &quot;objectSize&quot;: {&quot;x&quot;: 6.359659195356948,
  &quot;y&quot;: 2.3712878224323504, &quot;z&quot;: 2.6716625690460205}, &quot;trackId&quot;: 14089,
  &quot;position&quot;: {&quot;x&quot;: 754173.7913081107, &quot;y&quot;: 2549702.5615488826, &quot;z&quot;: 0},
  &quot;polygon&quot;: [{&quot;y&quot;: 2549703.1139222463, &quot;x&quot;: 754176.9484759554, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549705.042419922, &quot;x&quot;: 754175.8206193285, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549702.009175519, &quot;x&quot;: 754170.634140266, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549700.080677843,
  &quot;x&quot;: 754171.7619968929, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.014732686802744865,
  &quot;x&quot;: 0.3149268627166748, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.4090806841850281, &quot;yaw&quot;:
  -0.029081206768751144, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 20, &quot;movementClassification&quot;: 1,
  &quot;objectClassification&quot;: 5, &quot;varObjectSize&quot;: {&quot;y&quot;: 0.051883764564991, &quot;x&quot;:
  0.12044931203126907, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true,
  &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: 0.0,
  &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 2, &quot;laneid&quot;: -1, &quot;tail_lamp&quot;: 0,
  &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.010319292545318604, &quot;varPosition&quot;: {&quot;y&quot;:
  0.0433049276471138, &quot;x&quot;: 0.6740301847457886, &quot;z&quot;: 0.0}, &quot;angle&quot;: -0.029081206768751144,
  &quot;varAngle&quot;: -0.029081206768751144, &quot;objectSize&quot;: {&quot;x&quot;: 4.595096111417609,
  &quot;y&quot;: 2.2460758684420306, &quot;z&quot;: 1.6772761344909668}, &quot;trackId&quot;: 14090,
  &quot;position&quot;: {&quot;x&quot;: 754221.6012945686, &quot;y&quot;: 2549729.7803284056, &quot;z&quot;: 0},
  &quot;polygon&quot;: [{&quot;y&quot;: 2549730.4955254737, &quot;x&quot;: 754225.6655380403, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549732.892245761, &quot;x&quot;: 754224.3115624915, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549729.0651313374, &quot;x&quot;: 754217.5370510969, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549726.66841105,
  &quot;x&quot;: 754218.8910266458, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.34089910984039307,
  &quot;x&quot;: 5.42317008972168, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0, &quot;yaw&quot;:
  -0.009178061969578266, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 4, &quot;movementClassification&quot;: 1,
  &quot;objectClassification&quot;: 4, &quot;varObjectSize&quot;: {&quot;y&quot;: 0.02472420036792755, &quot;x&quot;:
  0.03376471996307373, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true,
  &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: 0.0,
  &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 0, &quot;laneid&quot;: -1, &quot;tail_lamp&quot;: 0,
  &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.02810698002576828, &quot;varPosition&quot;: {&quot;y&quot;:
  0.031532883644104004, &quot;x&quot;: 0.48427486419677734, &quot;z&quot;: 0.0}, &quot;angle&quot;:
  -0.009178061969578266, &quot;varAngle&quot;: -0.009178061969578266, &quot;objectSize&quot;:
  {&quot;x&quot;: 6.288227081560047, &quot;y&quot;: 2.239650249075907, &quot;z&quot;: 2.7363595962524414},
  &quot;trackId&quot;: 14093, &quot;position&quot;: {&quot;x&quot;: 754154.6088726907, &quot;y&quot;: 2549686.430769314,
  &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549686.7454056437, &quot;x&quot;: 754157.0116873143,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549688.3840163588, &quot;x&quot;: 754156.0432157872, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549686.1161329844, &quot;x&quot;: 754152.2060580672, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549684.4775222694, &quot;x&quot;: 754153.1745295943, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;:
  -0.16366241872310638, &quot;x&quot;: 6.810338020324707, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0,
  &quot;yaw&quot;: 0.013913864269852638, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 9,
  &quot;movementClassification&quot;: 1, &quot;objectClassification&quot;: 5, &quot;varObjectSize&quot;: {&quot;y&quot;:
  0.044669877737760544, &quot;x&quot;: 0.16341343522071838, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: 0.0, &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 0, &quot;laneid&quot;: -1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.020601090043783188,
  &quot;varPosition&quot;: {&quot;y&quot;: 0.033476945012807846, &quot;x&quot;: 0.49058958888053894, &quot;z&quot;:
  0.0}, &quot;angle&quot;: 0.013913864269852638, &quot;varAngle&quot;: 0.013913864269852638, &quot;objectSize&quot;:
  {&quot;x&quot;: 12.331075668604706, &quot;y&quot;: 3.0109045509418633, &quot;z&quot;: 3.966094732284546},
  &quot;trackId&quot;: 14095, &quot;position&quot;: {&quot;x&quot;: 754166.8494713402, &quot;y&quot;: 2549693.839757979,
  &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549694.9891371056, &quot;x&quot;: 754171.2977129337,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549697.3290162934, &quot;x&quot;: 754169.8382841588, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549692.690378852, &quot;x&quot;: 754162.4012297466, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549690.3504996644, &quot;x&quot;: 754163.8606585215, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;:
  -0.05346085503697395, &quot;x&quot;: 0.12799233198165894, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0,
  &quot;yaw&quot;: -0.003872224362567067, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 6,
  &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 5, &quot;varObjectSize&quot;: {&quot;y&quot;:
  0.050765931606292725, &quot;x&quot;: 0.021037861704826355, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: 0.0, &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 0, &quot;laneid&quot;: -1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.010788239538669586,
  &quot;varPosition&quot;: {&quot;y&quot;: 0.023796051740646362, &quot;x&quot;: 0.07372283935546875, &quot;z&quot;:
  0.0}, &quot;angle&quot;: -0.003872224362567067, &quot;varAngle&quot;: -0.003872224362567067, &quot;objectSize&quot;:
  {&quot;x&quot;: 7.929567337392568, &quot;y&quot;: 2.8872056005442865, &quot;z&quot;: 3.2544190883636475},
  &quot;trackId&quot;: 14097, &quot;position&quot;: {&quot;x&quot;: 754218.1272111441, &quot;y&quot;: 2549731.646201225,
  &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549732.3126155566, &quot;x&quot;: 754221.8812913201,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549734.6408213526, &quot;x&quot;: 754220.4871849404, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549730.9797868934, &quot;x&quot;: 754214.3731309681, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549728.6515810974, &quot;x&quot;: 754215.7672373478, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;:
  0.0, &quot;x&quot;: 0.0, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0, &quot;yaw&quot;: 2.9268321990966797,
  &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 0, &quot;movementClassification&quot;: 0,
  &quot;objectClassification&quot;: 2, &quot;varObjectSize&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0,
  &quot;z&quot;: -1.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true,
  &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: -1.0,
  &quot;existenceProbability&quot;: 0.7200000286102295, &quot;age&quot;: 0, &quot;laneid&quot;: 1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0, &quot;varPosition&quot;:
  {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;angle&quot;: 2.9268321990966797,
  &quot;varAngle&quot;: 2.9268321990966797, &quot;objectSize&quot;: {&quot;x&quot;: 0.6145424247240959, &quot;y&quot;:
  0.5394527312987736, &quot;z&quot;: 1.608574628829956}, &quot;trackId&quot;: 2010003, &quot;position&quot;:
  {&quot;x&quot;: 754226.9349686778, &quot;y&quot;: 2549716.5940228417, &quot;z&quot;: 0}, &quot;polygon&quot;:
  [{&quot;y&quot;: 2549716.7504287576, &quot;x&quot;: 754226.5572053925, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549716.2397136213, &quot;x&quot;: 754226.7309272934, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549716.4376169257,
  &quot;x&quot;: 754227.3127319631, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549716.948332062, &quot;x&quot;:
  754227.1390100622, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.0, &quot;x&quot;: 0.0,
  &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0, &quot;yaw&quot;: 2.9275732040405273, &quot;yawRate&quot;: 0.0,
  &quot;age_frames&quot;: 0, &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 2,
  &quot;varObjectSize&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: -1.0, &quot;existenceProbability&quot;: 0.6299999952316284, &quot;age&quot;: 0,
  &quot;laneid&quot;: 1, &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0,
  &quot;varPosition&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0},
  &quot;angle&quot;: 2.9275732040405273, &quot;varAngle&quot;: 2.9275732040405273, &quot;objectSize&quot;:
  {&quot;x&quot;: 0.6005771760431835, &quot;y&quot;: 0.5982791778507319, &quot;z&quot;: 1.602581262588501},
  &quot;trackId&quot;: 2010004, &quot;position&quot;: {&quot;x&quot;: 754226.5198968921, &quot;y&quot;:
  2549716.470921613, &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549716.6214184347, &quot;x&quot;:
  754226.1429820128, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549716.121411978, &quot;x&quot;: 754226.313603085,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549716.320424791, &quot;x&quot;: 754226.8968117714, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549716.820431248, &quot;x&quot;: 754226.7261906993, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;:
  {&quot;y&quot;: 5.338902473449707, &quot;x&quot;: 3.683285713195801, &quot;z&quot;: 0.0}, &quot;acceleration&quot;:
  0.7093154191970825, &quot;yaw&quot;: 0.840665340423584, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 18,
  &quot;movementClassification&quot;: 1, &quot;objectClassification&quot;: 4, &quot;varObjectSize&quot;: {&quot;y&quot;:
  0.00919474195688963, &quot;x&quot;: 0.04151472449302673, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: 0.0, &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 1, &quot;laneid&quot;: -1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 3.403864860534668,
  &quot;varPosition&quot;: {&quot;y&quot;: 3.8528733253479004, &quot;x&quot;: 0.03472320735454559, &quot;z&quot;: 0.0},
  &quot;angle&quot;: 0.840665340423584, &quot;varAngle&quot;: 0.840665340423584, &quot;objectSize&quot;:
  {&quot;x&quot;: 4.1523418425925644, &quot;y&quot;: 1.8412642479791048, &quot;z&quot;: 1.399999976158142},
  &quot;trackId&quot;: 14101, &quot;position&quot;: {&quot;x&quot;: 754297.5904572142, &quot;y&quot;: 2549770.979496178,
  &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549772.738323426, &quot;x&quot;: 754299.1926434237,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549773.3510955237, &quot;x&quot;: 754297.4007456402, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549769.2206689306, &quot;x&quot;: 754295.9882710046, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549768.6078968327, &quot;x&quot;: 754297.7801687882, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;:
  0.0, &quot;x&quot;: 0.0, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0, &quot;yaw&quot;: 3.0487093925476074,
  &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 0, &quot;movementClassification&quot;: 0,
  &quot;objectClassification&quot;: 2, &quot;varObjectSize&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0,
  &quot;z&quot;: -1.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true,
  &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: -1.0,
  &quot;existenceProbability&quot;: 0.5699999928474426, &quot;age&quot;: 0, &quot;laneid&quot;: 1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0, &quot;varPosition&quot;:
  {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;angle&quot;: 3.0487093925476074,
  &quot;varAngle&quot;: 3.0487093925476074, &quot;objectSize&quot;: {&quot;x&quot;: 0.5367302900000699, &quot;y&quot;:
  0.5154706840056408, &quot;z&quot;: 1.579459309577942}, &quot;trackId&quot;: 2010006, &quot;position&quot;:
  {&quot;x&quot;: 754223.6164249945, &quot;y&quot;: 2549714.471798973, &quot;z&quot;: 0}, &quot;polygon&quot;:
  [{&quot;y&quot;: 2549714.573671684, &quot;x&quot;: 754223.2760217611, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549714.1417135713, &quot;x&quot;: 754223.4849105913, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549714.369926262,
  &quot;x&quot;: 754223.956828228, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549714.801884375, &quot;x&quot;:
  754223.7479393978, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.0, &quot;x&quot;: 0.0,
  &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0, &quot;yaw&quot;: 3.0580410957336426, &quot;yawRate&quot;: 0.0,
  &quot;age_frames&quot;: 0, &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 2,
  &quot;varObjectSize&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: -1.0, &quot;existenceProbability&quot;: 0.49000000953674316, &quot;age&quot;: 0,
  &quot;laneid&quot;: 1, &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0,
  &quot;varPosition&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0},
  &quot;angle&quot;: 3.0580410957336426, &quot;varAngle&quot;: 3.0580410957336426, &quot;objectSize&quot;:
  {&quot;x&quot;: 0.6563702226625959, &quot;y&quot;: 0.5437205433503375, &quot;z&quot;: 1.7610063552856445},
  &quot;trackId&quot;: 2010007, &quot;position&quot;: {&quot;x&quot;: 754223.800135951, &quot;y&quot;:
  2549714.5430224235, &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549714.6311239023, &quot;x&quot;:
  754223.4224545127, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549714.189242998, &quot;x&quot;: 754223.641248388,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549714.4549209448, &quot;x&quot;: 754224.1778173894, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549714.896801849, &quot;x&quot;: 754223.9590235141, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;:
  {&quot;y&quot;: 0.0, &quot;x&quot;: 0.0, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0,
  &quot;yaw&quot;: 2.8691964149475098, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 0,
  &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 2, &quot;varObjectSize&quot;: {&quot;y&quot;:
  -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;:
  true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: -1.0,
  &quot;existenceProbability&quot;: 0.46000000834465027, &quot;age&quot;: 0, &quot;laneid&quot;: 1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0, &quot;varPosition&quot;:
  {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;angle&quot;: 2.8691964149475098,
  &quot;varAngle&quot;: 2.8691964149475098, &quot;objectSize&quot;: {&quot;x&quot;: 0.6340991855383697, &quot;y&quot;:
  0.5395118590712354, &quot;z&quot;: 1.7164891958236694}, &quot;trackId&quot;: 2010008, &quot;position&quot;:
  {&quot;x&quot;: 754224.4711294451, &quot;y&quot;: 2549707.275401612, &quot;z&quot;: 0}, &quot;polygon&quot;:
  [{&quot;y&quot;: 2549707.497630735, &quot;x&quot;: 754224.1297768528, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549706.9091430698, &quot;x&quot;: 754224.2929103229, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549707.0531724887,
  &quot;x&quot;: 754224.8124820375, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549707.641660154, &quot;x&quot;:
  754224.6493485674, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.0, &quot;x&quot;: 0.0,
  &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0, &quot;yaw&quot;: 1.8253346681594849, &quot;yawRate&quot;: 0.0,
  &quot;age_frames&quot;: 0, &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 2,
  &quot;varObjectSize&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: -1.0, &quot;existenceProbability&quot;: 0.4399999976158142, &quot;age&quot;: 0,
  &quot;laneid&quot;: 1, &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0,
  &quot;varPosition&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0},
  &quot;angle&quot;: 1.8253346681594849, &quot;varAngle&quot;: 1.8253346681594849, &quot;objectSize&quot;:
  {&quot;x&quot;: 0.6268565058234417, &quot;y&quot;: 0.5361811517525933, &quot;z&quot;: 1.7359797954559326},
  &quot;trackId&quot;: 2010009, &quot;position&quot;: {&quot;x&quot;: 754224.4825925499, &quot;y&quot;:
  2549707.2855720804, &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549707.696072428, &quot;x&quot;:
  754224.524817523, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549707.233136757, &quot;x&quot;: 754224.0732711352,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549706.875071733, &quot;x&quot;: 754224.4403675768, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549707.338007404, &quot;x&quot;: 754224.8919139645, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;:
  {&quot;y&quot;: 0.0, &quot;x&quot;: 0.0, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0, &quot;yaw&quot;:
  -2.994373083114624, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 0, &quot;movementClassification&quot;: 0,
  &quot;objectClassification&quot;: 2, &quot;varObjectSize&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0,
  &quot;z&quot;: -1.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true,
  &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: -1.0,
  &quot;existenceProbability&quot;: 0.36000001430511475, &quot;age&quot;: 0, &quot;laneid&quot;: 1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0, &quot;varPosition&quot;:
  {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;angle&quot;: -2.994373083114624,
  &quot;varAngle&quot;: -2.994373083114624, &quot;objectSize&quot;: {&quot;x&quot;: 0.6374803188120286, &quot;y&quot;:
  0.5315571429818663, &quot;z&quot;: 1.753071665763855}, &quot;trackId&quot;: 2010010, &quot;position&quot;:
  {&quot;x&quot;: 754224.3434999502, &quot;y&quot;: 2549708.9733050517, &quot;z&quot;: 0}, &quot;polygon&quot;:
  [{&quot;y&quot;: 2549709.023441377, &quot;x&quot;: 754223.8993044759, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549708.5276599117, &quot;x&quot;: 754224.3085188176, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549708.9231687263,
  &quot;x&quot;: 754224.7876954246, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549709.4189501917, &quot;x&quot;:
  754224.3784810829, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.0, &quot;x&quot;: 0.0,
  &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0, &quot;yaw&quot;: -3.0553882122039795, &quot;yawRate&quot;: 0.0,
  &quot;age_frames&quot;: 0, &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 2,
  &quot;varObjectSize&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: -1.0, &quot;existenceProbability&quot;: 0.4000000059604645, &quot;age&quot;: 0,
  &quot;laneid&quot;: 1, &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0,
  &quot;varPosition&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;angle&quot;:
  -3.0553882122039795, &quot;varAngle&quot;: -3.0553882122039795, &quot;objectSize&quot;: {&quot;x&quot;:
  0.6219154600452959, &quot;y&quot;: 0.5186902885061804, &quot;z&quot;: 1.7251052856445312}, &quot;trackId&quot;:
  2010011, &quot;position&quot;: {&quot;x&quot;: 754224.2806777786, &quot;y&quot;: 2549709.0288660913, &quot;z&quot;:
  0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549709.105524029, &quot;x&quot;: 754223.83270966, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549708.57897965, &quot;x&quot;: 754224.2162257433, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549708.9522081534, &quot;x&quot;: 754224.7286458972, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549709.4787525325,
  &quot;x&quot;: 754224.345129814, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.009207496419548988,
  &quot;x&quot;: 0.012490829452872276, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: -0.03263755142688751,
  &quot;yaw&quot;: -0.0005622330354526639, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 1088,
  &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 5, &quot;varObjectSize&quot;: {&quot;y&quot;:
  0.042907409369945526, &quot;x&quot;: 0.07464752346277237, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: 0.0, &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 108, &quot;laneid&quot;: -1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 1.5695040225982666,
  &quot;varPosition&quot;: {&quot;y&quot;: 0.01179547980427742, &quot;x&quot;: 0.03777739405632019, &quot;z&quot;: 0.0},
  &quot;angle&quot;: -0.0005622330354526639, &quot;varAngle&quot;: -0.0005622330354526639, &quot;objectSize&quot;:
  {&quot;x&quot;: 12.448831558237393, &quot;y&quot;: 2.8262107371682337, &quot;z&quot;: 3.6693732738494873},
  &quot;trackId&quot;: 13724, &quot;position&quot;: {&quot;x&quot;: 754214.4239350865, &quot;y&quot;:
  2549720.7268626676, &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549722.7219056776, &quot;x&quot;:
  754220.3396164953, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549725.0288834004, &quot;x&quot;: 754218.9481036022,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549718.7318196576, &quot;x&quot;: 754208.5082536776, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549716.424841935, &quot;x&quot;: 754209.8997665708, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;:
  {&quot;y&quot;: 0.0, &quot;x&quot;: 0.0, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0,
  &quot;yaw&quot;: 1.287652611732483, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 0,
  &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 2, &quot;varObjectSize&quot;: {&quot;y&quot;:
  -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;:
  true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: -1.0,
  &quot;existenceProbability&quot;: 0.3700000047683716, &quot;age&quot;: 0, &quot;laneid&quot;: 1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0, &quot;varPosition&quot;:
  {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;angle&quot;: 1.287652611732483,
  &quot;varAngle&quot;: 1.287652611732483, &quot;objectSize&quot;: {&quot;x&quot;: 0.6403223879820509, &quot;y&quot;:
  0.5607008936384892, &quot;z&quot;: 1.668745517730713}, &quot;trackId&quot;: 2010013, &quot;position&quot;:
  {&quot;x&quot;: 754224.9032401645, &quot;y&quot;: 2549707.262589849, &quot;z&quot;: 0}, &quot;polygon&quot;:
  [{&quot;y&quot;: 2549707.6219587005, &quot;x&quot;: 754225.1812235658, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549707.5222460986, &quot;x&quot;: 754224.5304139653, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549706.9032209977,
  &quot;x&quot;: 754224.6252567632, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549707.0029335995, &quot;x&quot;:
  754225.2760663637, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.0, &quot;x&quot;: 0.0,
  &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0, &quot;yaw&quot;: 1.2611397504806519, &quot;yawRate&quot;: 0.0,
  &quot;age_frames&quot;: 0, &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 2,
  &quot;varObjectSize&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: -1.0, &quot;existenceProbability&quot;: 0.38999998569488525, &quot;age&quot;: 0,
  &quot;laneid&quot;: 1, &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0,
  &quot;varPosition&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0},
  &quot;angle&quot;: 1.2611397504806519, &quot;varAngle&quot;: 1.2611397504806519, &quot;objectSize&quot;:
  {&quot;x&quot;: 0.6500940919453109, &quot;y&quot;: 0.5364157554722581, &quot;z&quot;: 1.7123664617538452},
  &quot;trackId&quot;: 2010014, &quot;position&quot;: {&quot;x&quot;: 754224.319234121, &quot;y&quot;:
  2549709.0380235147, &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549709.385897297, &quot;x&quot;:
  754224.6520670559, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549709.3322752886, &quot;x&quot;: 754223.9381700524,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549708.6901497324, &quot;x&quot;: 754223.9864011861, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549708.7437717407, &quot;x&quot;: 754224.7002981897, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;:
  {&quot;y&quot;: 0.0, &quot;x&quot;: 0.0, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0,
  &quot;yaw&quot;: 1.6628836393356323, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 0,
  &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 2, &quot;varObjectSize&quot;: {&quot;y&quot;:
  -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;:
  true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: -1.0,
  &quot;existenceProbability&quot;: 0.3799999952316284, &quot;age&quot;: 0, &quot;laneid&quot;: 1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0, &quot;varPosition&quot;:
  {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;angle&quot;: 1.6628836393356323,
  &quot;varAngle&quot;: 1.6628836393356323, &quot;objectSize&quot;: {&quot;x&quot;: 0.6530755160339093, &quot;y&quot;:
  0.5621422529151443, &quot;z&quot;: 1.6826704740524292}, &quot;trackId&quot;: 2010015, &quot;position&quot;:
  {&quot;x&quot;: 754248.3947768403, &quot;y&quot;: 2549681.668382987, &quot;z&quot;: 0}, &quot;polygon&quot;:
  [{&quot;y&quot;: 2549681.94376278, &quot;x&quot;: 754248.6156234887, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549681.8131020768, &quot;x&quot;: 754248.0728084295, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549681.3930031937,
  &quot;x&quot;: 754248.173930192, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549681.523663897, &quot;x&quot;:
  754248.7167452512, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.37790948152542114,
  &quot;x&quot;: 2.121377468109131, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.5477482676506042, &quot;yaw&quot;:
  0.23547972738742828, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 16, &quot;movementClassification&quot;: 1,
  &quot;objectClassification&quot;: 5, &quot;varObjectSize&quot;: {&quot;y&quot;: 0.04687780141830444, &quot;x&quot;:
  0.06283949315547943, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true,
  &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: 0.0,
  &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 1, &quot;laneid&quot;: -1, &quot;tail_lamp&quot;: 0,
  &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.2988971173763275, &quot;varPosition&quot;: {&quot;y&quot;:
  0.6166673898696899, &quot;x&quot;: 0.04972994327545166, &quot;z&quot;: 0.0}, &quot;angle&quot;: 0.23547972738742828,
  &quot;varAngle&quot;: 0.23547972738742828, &quot;objectSize&quot;: {&quot;x&quot;: 9.729720115651618,
  &quot;y&quot;: 3.0137252807821104, &quot;z&quot;: 3.6066694259643555}, &quot;trackId&quot;: 14113,
  &quot;position&quot;: {&quot;x&quot;: 754182.1914189172, &quot;y&quot;: 2549702.3095987164, &quot;z&quot;: 0},
  &quot;polygon&quot;: [{&quot;y&quot;: 2549702.828399246, &quot;x&quot;: 754186.2550235203, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549705.25623506, &quot;x&quot;: 754185.0373582385, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549701.790798187, &quot;x&quot;: 754178.127814314, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549699.362962373,
  &quot;x&quot;: 754179.3454795958, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.48609495162963867,
  &quot;x&quot;: 2.733088493347168, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.7413628101348877, &quot;yaw&quot;:
  0.2100357711315155, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 18, &quot;movementClassification&quot;: 1,
  &quot;objectClassification&quot;: 5, &quot;varObjectSize&quot;: {&quot;y&quot;: 0.017966870218515396, &quot;x&quot;:
  0.04761146754026413, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true,
  &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: 0.0,
  &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 1, &quot;laneid&quot;: -1, &quot;tail_lamp&quot;: 0,
  &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.14024043083190918, &quot;varPosition&quot;: {&quot;y&quot;:
  0.20391178131103516, &quot;x&quot;: 0.03690333664417267, &quot;z&quot;: 0.0}, &quot;angle&quot;: 0.2100357711315155,
  &quot;varAngle&quot;: 0.2100357711315155, &quot;objectSize&quot;: {&quot;x&quot;: 5.793015003460974,
  &quot;y&quot;: 2.3758008482422763, &quot;z&quot;: 2.5793163776397705}, &quot;trackId&quot;: 14107,
  &quot;position&quot;: {&quot;x&quot;: 754221.2152687326, &quot;y&quot;: 2549729.6761513706, &quot;z&quot;: 0},
  &quot;polygon&quot;: [{&quot;y&quot;: 2549730.212639273, &quot;x&quot;: 754224.8753671086, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549732.3955079783, &quot;x&quot;: 754223.7231042598, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549729.139663468, &quot;x&quot;: 754217.5551703566, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549726.956794763,
  &quot;x&quot;: 754218.7074332054, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: -0.007400955073535442,
  &quot;x&quot;: 0.67745441198349, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0, &quot;yaw&quot;:
  0.2590515613555908, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 4, &quot;movementClassification&quot;: 1,
  &quot;objectClassification&quot;: 4, &quot;varObjectSize&quot;: {&quot;y&quot;: 0.002723380923271179, &quot;x&quot;:
  0.004534244537353516, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true,
  &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: 0.0,
  &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 0, &quot;laneid&quot;: -1, &quot;tail_lamp&quot;: 0,
  &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.0616636797785759, &quot;varPosition&quot;: {&quot;y&quot;:
  0.20233643054962158, &quot;x&quot;: 0.0150299072265625, &quot;z&quot;: 0.0}, &quot;angle&quot;: 0.2590515613555908,
  &quot;varAngle&quot;: 0.2590515613555908, &quot;objectSize&quot;: {&quot;x&quot;: 4.431808948481226,
  &quot;y&quot;: 1.9646065233939307, &quot;z&quot;: 1.5273629426956177}, &quot;trackId&quot;: 14116,
  &quot;position&quot;: {&quot;x&quot;: 754198.8165687606, &quot;y&quot;: 2549707.4997024126, &quot;z&quot;: 0},
  &quot;polygon&quot;: [{&quot;y&quot;: 2549707.695187655, &quot;x&quot;: 754201.1091824138, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549709.2903616843, &quot;x&quot;: 754200.2615017059, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549707.3042171705, &quot;x&quot;: 754196.5239551073, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549705.709043141,
  &quot;x&quot;: 754197.3716358152, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.0, &quot;x&quot;:
  0.0, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0, &quot;yaw&quot;: -2.938439130783081, &quot;yawRate&quot;:
  0.0, &quot;age_frames&quot;: 0, &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 2,
  &quot;varObjectSize&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: -1.0, &quot;existenceProbability&quot;: 0.3700000047683716, &quot;age&quot;: 0,
  &quot;laneid&quot;: 1, &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0,
  &quot;varPosition&quot;: {&quot;y&quot;: -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;angle&quot;:
  -2.938439130783081, &quot;varAngle&quot;: -2.938439130783081, &quot;objectSize&quot;: {&quot;x&quot;:
  0.6922118071243766, &quot;y&quot;: 0.6342379448736222, &quot;z&quot;: 1.6828798055648804}, &quot;trackId&quot;:
  2010012, &quot;position&quot;: {&quot;x&quot;: 754224.2733112082, &quot;y&quot;: 2549708.9493141356, &quot;z&quot;:
  0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549708.9764080807, &quot;x&quot;: 754223.8048760276, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549708.480198245, &quot;x&quot;: 754224.2635192783, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549708.9222201905, &quot;x&quot;: 754224.7417463888, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549709.418430026,
  &quot;x&quot;: 754224.2831031382, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 0.0038597052916884422,
  &quot;x&quot;: 0.005148463882505894, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: -0.016623618081212044,
  &quot;yaw&quot;: 3.1409800052642822, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 704,
  &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 4, &quot;varObjectSize&quot;:
  {&quot;y&quot;: 2.384185791015625e-07, &quot;x&quot;: 6.6879771806830544e-40, &quot;z&quot;: 0.0},
  &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true,
  &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: 0.0, &quot;existenceProbability&quot;: 1.0, &quot;age&quot;:
  70, &quot;laneid&quot;: -1, &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0,
  &quot;varYaw&quot;: 1.5765979290008545, &quot;varPosition&quot;: {&quot;y&quot;: 0.011577920988202095, &quot;x&quot;:
  0.002057259436696768, &quot;z&quot;: 0.0}, &quot;angle&quot;: 3.1409800052642822,
  &quot;varAngle&quot;: 3.1409800052642822, &quot;objectSize&quot;: {&quot;x&quot;: 4.364827156133898,
  &quot;y&quot;: 2.190804243151825, &quot;z&quot;: 1.4681886434555054}, &quot;trackId&quot;: 13868,
  &quot;position&quot;: {&quot;x&quot;: 754226.7302439698, &quot;y&quot;: 2549723.857026175, &quot;z&quot;: 0},
  &quot;polygon&quot;: [{&quot;y&quot;: 2549723.667928572, &quot;x&quot;: 754224.295685092, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549721.7919058017, &quot;x&quot;: 754225.4271274451, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549724.0461237784, &quot;x&quot;: 754229.1648028477, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549725.9221465485,
  &quot;x&quot;: 754228.0333604945, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: 9.045133590698242,
  &quot;x&quot;: 0.7502111196517944, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.301500141620636,
  &quot;yaw&quot;: 1.5616627931594849, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 13,
  &quot;movementClassification&quot;: 1, &quot;objectClassification&quot;: 5, &quot;varObjectSize&quot;: {&quot;y&quot;:
  0.05941148102283478, &quot;x&quot;: 0.3979243338108063, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: 0.0, &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 1, &quot;laneid&quot;: -1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.24049167335033417,
  &quot;varPosition&quot;: {&quot;y&quot;: 0.41161274909973145, &quot;x&quot;: 0.3517218232154846, &quot;z&quot;: 0.0},
  &quot;angle&quot;: 1.5616627931594849, &quot;varAngle&quot;: 1.5616627931594849, &quot;objectSize&quot;:
  {&quot;x&quot;: 13.295665741333842, &quot;y&quot;: 3.303450822699182, &quot;z&quot;: 4.305897235870361},
  &quot;trackId&quot;: 14143, &quot;position&quot;: {&quot;x&quot;: 754202.2296292426, &quot;y&quot;:
  2549717.9540030966, &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549719.7672140026, &quot;x&quot;:
  754208.5267396846, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549722.478971598, &quot;x&quot;: 754206.9694598056,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549716.1407921906, &quot;x&quot;: 754195.9325188005, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549713.429034595, &quot;x&quot;: 754197.4897986795, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;:
  {&quot;y&quot;: 4.104961395263672, &quot;x&quot;: -0.2745184898376465, &quot;z&quot;: 0.0}, &quot;acceleration&quot;:
  0.0, &quot;yaw&quot;: 1.676039457321167, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 4,
  &quot;movementClassification&quot;: 1, &quot;objectClassification&quot;: 5, &quot;varObjectSize&quot;: {&quot;y&quot;:
  0.01605132222175598, &quot;x&quot;: 0.16217023134231567, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: 0.0, &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 0, &quot;laneid&quot;: -1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.4590504765510559,
  &quot;varPosition&quot;: {&quot;y&quot;: 1.2241711616516113, &quot;x&quot;: 0.19917869567871094, &quot;z&quot;: 0.0},
  &quot;angle&quot;: 1.676039457321167, &quot;varAngle&quot;: 1.676039457321167, &quot;objectSize&quot;:
  {&quot;x&quot;: 9.913563728592793, &quot;y&quot;: 3.0011284348234812, &quot;z&quot;: 3.8089098930358887},
  &quot;trackId&quot;: 14148, &quot;position&quot;: {&quot;x&quot;: 754219.5083149959, &quot;y&quot;:
  2549736.6685100505, &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549737.3137043673, &quot;x&quot;:
  754223.1391329103, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549739.674784264, &quot;x&quot;: 754221.6440654,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549736.0233157338, &quot;x&quot;: 754215.8774970815, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549733.662235837, &quot;x&quot;: 754217.3725645918, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;:
  {&quot;y&quot;: 3.607618808746338, &quot;x&quot;: 0.4305284023284912, &quot;z&quot;: 0.0}, &quot;acceleration&quot;:
  0.0, &quot;yaw&quot;: 1.6483476161956787, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 4,
  &quot;movementClassification&quot;: 1, &quot;objectClassification&quot;: 5, &quot;varObjectSize&quot;: {&quot;y&quot;:
  0.04496470093727112, &quot;x&quot;: 0.22028011083602905, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true,
  &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0,
  &quot;varYawRate&quot;: 0.0, &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 0, &quot;laneid&quot;: -1,
  &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.4172811508178711,
  &quot;varPosition&quot;: {&quot;y&quot;: 1.175732970237732, &quot;x&quot;: 0.16846585273742676, &quot;z&quot;: 0.0},
  &quot;angle&quot;: 1.6483476161956787, &quot;varAngle&quot;: 1.6483476161956787, &quot;objectSize&quot;:
  {&quot;x&quot;: 7.947175502998712, &quot;y&quot;: 2.786547183778197, &quot;z&quot;: 3.2045676708221436},
  &quot;trackId&quot;: 14149, &quot;position&quot;: {&quot;x&quot;: 754222.2366898394, &quot;y&quot;:
  2549734.1042263964, &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549734.5866885255, &quot;x&quot;:
  754225.730565534, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549736.8896645806, &quot;x&quot;: 754224.40031439,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549733.6217642673, &quot;x&quot;: 754218.7428141448, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549731.3187882123, &quot;x&quot;: 754220.0730652888, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;:
  {&quot;y&quot;: 0.0, &quot;x&quot;: 0.0, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: 0.0,
  &quot;yaw&quot;: 3.0352253913879395, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 0,
  &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 2, &quot;varObjectSize&quot;: {&quot;y&quot;:
  -1.0, &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;:
  true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: -1.0,
  &quot;existenceProbability&quot;: 0.550000011920929, &quot;age&quot;: 0, &quot;laneid&quot;: 1, &quot;tail_lamp&quot;:
  0, &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: -1.0, &quot;varPosition&quot;: {&quot;y&quot;: -1.0,
  &quot;x&quot;: -1.0, &quot;z&quot;: -1.0}, &quot;angle&quot;: 3.0352253913879395,
  &quot;varAngle&quot;: 3.0352253913879395, &quot;objectSize&quot;: {&quot;x&quot;: 0.6110020879973485, &quot;y&quot;:
  0.6081420778758665, &quot;z&quot;: 1.6135650873184204}, &quot;trackId&quot;: 2010005, &quot;position&quot;:
  {&quot;x&quot;: 754224.5453584462, &quot;y&quot;: 2549715.296254535, &quot;z&quot;: 0}, &quot;polygon&quot;:
  [{&quot;y&quot;: 2549715.4030143903, &quot;x&quot;: 754224.1889463505, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549714.954496646, &quot;x&quot;: 754224.3982911366, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549715.18949468,
  &quot;x&quot;: 754224.9017705419, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549715.6380124246, &quot;x&quot;:
  754224.6924257558, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;: {&quot;y&quot;: -0.009591497480869293, &quot;x&quot;:
  0.030430231243371964, &quot;z&quot;: 0.0}, &quot;acceleration&quot;: -0.21045908331871033, &quot;yaw&quot;:
  -0.06019482761621475, &quot;yawRate&quot;: 0.0, &quot;age_frames&quot;: 81, &quot;movementClassification&quot;: 0,
  &quot;objectClassification&quot;: 4, &quot;varObjectSize&quot;: {&quot;y&quot;: 0.0009991326369345188,
  &quot;x&quot;: 1.090315890905913e-05, &quot;z&quot;: 0.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true,
  &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true, &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: 0.0,
  &quot;existenceProbability&quot;: 1.0, &quot;age&quot;: 8, &quot;laneid&quot;: -1, &quot;tail_lamp&quot;: 0,
  &quot;varVelocity&quot;: 0.0, &quot;varYaw&quot;: 0.005607601720839739, &quot;varPosition&quot;: {&quot;y&quot;:
  0.013559013605117798, &quot;x&quot;: 0.0020334417931735516, &quot;z&quot;: 0.0}, &quot;angle&quot;:
  -0.06019482761621475, &quot;varAngle&quot;: -0.06019482761621475, &quot;objectSize&quot;:
  {&quot;x&quot;: 4.960976123734827, &quot;y&quot;: 2.0849556925009516, &quot;z&quot;: 1.4460833072662354},
  &quot;trackId&quot;: 14076, &quot;position&quot;: {&quot;x&quot;: 754213.7558404255, &quot;y&quot;:
  2549716.2726067686, &quot;z&quot;: 0}, &quot;polygon&quot;: [{&quot;y&quot;: 2549716.501733898, &quot;x&quot;:
  754216.4367137615, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549718.3480673674, &quot;x&quot;: 754215.4681617705,
  &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549716.043479639, &quot;x&quot;: 754211.0749670895, &quot;z&quot;: 0.0},
  {&quot;y&quot;: 2549714.19714617, &quot;x&quot;: 754212.0435190805, &quot;z&quot;: 0.0}]}, {&quot;velocity&quot;:
  {&quot;y&quot;: 0.005626769736409187, &quot;x&quot;: -0.00301847280934453, &quot;z&quot;: 0.0},
  &quot;acceleration&quot;: 0.02084617130458355, &quot;yaw&quot;: 3.1264445781707764, &quot;yawRate&quot;: 0.0,
  &quot;age_frames&quot;: 248, &quot;movementClassification&quot;: 0, &quot;objectClassification&quot;: 5,
  &quot;varObjectSize&quot;: {&quot;y&quot;: 0.006669965572655201, &quot;x&quot;: 0.010758617892861366, &quot;z&quot;:
  0.0}, &quot;isValid&quot;: true, &quot;atten&quot;: true, &quot;dev&quot;: true, &quot;isBoundingbox&quot;: true,
  &quot;varAcceleration&quot;: 0.0, &quot;varYawRate&quot;: 0.0, &quot;existenceProbability&quot;: 1.0, &quot;age&quot;:
  24, &quot;laneid&quot;: -1, &quot;tail_lamp&quot;: 0, &quot;varVelocity&quot;: 0.0,
  &quot;varYaw&quot;: 1.5737093687057495, &quot;varPosition&quot;: {&quot;y&quot;: 0.002947082044556737, &quot;x&quot;:
  0.006204649806022644, &quot;z&quot;: 0.0}, &quot;angle&quot;: 3.1264445781707764,
  &quot;varAngle&quot;: 3.1264445781707764, &quot;objectSize&quot;: {&quot;x&quot;: 5.409594059413843,
  &quot;y&quot;: 2.169380426411017, &quot;z&quot;: 2.614927291870117}, &quot;trackId&quot;: 13973, &quot;position&quot;:
  {&quot;x&quot;: 754223.8330936071, &quot;y&quot;: 2549727.1190065173, &quot;z&quot;: 0}, &quot;polygon&quot;:
  [{&quot;y&quot;: 2549726.6670849845, &quot;x&quot;: 754220.9104154799, &quot;z&quot;: 0.0}, {&quot;y&quot;:
  2549724.797028644, &quot;x&quot;: 754222.0015097947, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549727.57092805,
  &quot;x&quot;: 754226.7557717343, &quot;z&quot;: 0.0}, {&quot;y&quot;: 2549729.4409843907, &quot;x&quot;:
  754225.6646774195, &quot;z&quot;: 0.0}]}]},subtype:userdefine,offset:0.0,multi:{}}]' info="" />
    </scene_event>
  )");
  */
  CPlanner planner;
  ASSERT_TRUE(xml.FirstChildElement("scene_event") != nullptr);
  CTraffic{}.ParseSceneEvents(xml.FirstChildElement("scene_event"), planner.m_scenceEvents);
  ASSERT_TRUE(planner.m_scenceEvents.size() == 1);
  auto& event = planner.m_scenceEvents.begin()->second;
  Json::Value root = event.saveJson();

  std::cout << JsonToString(root) << std::endl;
}