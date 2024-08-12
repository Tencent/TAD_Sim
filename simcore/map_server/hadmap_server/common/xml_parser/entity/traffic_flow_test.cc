/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/traffic_flow.h"

#include <gtest/gtest.h>

class TrafficFlowTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {}
};

TEST_F(TrafficFlowTest, ParseVehicleComposition) {
  TiXmlDocument xml;
  xml.Parse(R"(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <traffic>
      <trafficFlow>
        <vehicleComposition>
          <vehComp id="1001" type1="101" percentage1="80" behavior1="1" aggress1="0.5" 
          type2="102" percentage2="15" behavior2="1" aggress2="0.6" type3="103" percentage3="5" 
          behavior3="1" aggress3="0.1" info="Platoon1" />
        </vehicleComposition>
      </trafficFlow>
    </traffic>
  )");
  auto traffic = xml.FirstChildElement("traffic");
  ASSERT_TRUE(traffic != nullptr);
  auto xml_traffic_flow = traffic->FirstChildElement("trafficFlow");
  ASSERT_TRUE(xml_traffic_flow != nullptr);
  Json::Value data(Json::arrayValue);
  CTrafficFlow{}.ParseVehicleComposition(xml_traffic_flow, data);
  std::cout << data.toStyledString() << std::endl;
}

TEST_F(TrafficFlowTest, ParseVehicleInputAndExit) {
  TiXmlDocument xml;
  /*
  xml.Parse("(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <traffic>
      <locations>
        <location id="100001" pos="114.4217763,23.0994605" info="Start10001"/>
        <location id="100002" pos="114.4237081,23.1014154" info="Start10002"/>
        <location id="100003" pos="114.4183473,23.1005033" info="Start10003"/>
      </locations>
      <trafficFlow>
        <vehicleInput>
        <vehInput id="10001" location="100001" composition="1001" start_v="14.667"
        max_v="16.667" distribution="Fixed" timeHeadway="1" duration="100000" cover="100" info="Platoon1"/>
        <vehInput id="10002" location="100002" composition="1001" start_v="14.667"
        max_v="16.667" distribution="Fixed" timeHeadway="2" duration="100000" cover="100" info="Platoon1"/>
        </vehicleInput>
        <vehicleExit>
          <vehExit id="20001" location="200001" cover="0" info="Exit1"/>
          <vehExit id="20002" location="200002" cover="0" info="Exit2"/>
          <vehExit id="20003" location="200003" cover="0" info="Exit3"/>
        </vehicleExit>
      </trafficFlow>
    </traffic>
  )");
  */
  auto traffic = xml.FirstChildElement("traffic");
  ASSERT_TRUE(traffic != nullptr);
  auto xml_traffic_flow = traffic->FirstChildElement("trafficFlow");
  ASSERT_TRUE(xml_traffic_flow != nullptr);
  Json::Value data(Json::objectValue);
  CTrafficFlow{}.ParseVehicleInputAndExit(xml_traffic_flow, data);
  std::cout << data.toStyledString() << std::endl;
}

TEST_F(TrafficFlowTest, ParseRouteGroup) {
  TiXmlDocument xml;
  xml.Parse(R"(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <traffic>
      <locations>
        <location id="800010" pos="114.4222270,23.0988283" info="Route8001"/>
        <location id="800011" pos="114.4206245,23.1005751" info="Route8001"/>
        <location id="800012" pos="114.4211009,23.1008316" info="Route8001"/>
        <location id="800013" pos="114.4215938,23.1003306" info="Route8001"/>
        <location id="800020" pos="114.4235238,23.1012865" info="Route8002"/>
        <location id="800021" pos="114.4212187,23.0999158" info="Route8002"/>
        <location id="800022" pos="114.4206245,23.1005751" info="Route8002"/>
        <location id="800023" pos="114.4213650,23.1007345" info="Route8002"/>
      </locations>
      <trafficFlow>
        <routeGroups>	
          <routeGroup id="8001" start="800010" mid1="0" end1="800011" percentage1="10" mid2="0" end2="800012" 
          percentage2="80" mid3="0" end3="800013" percentage3="10" />
          <routeGroup id="8002" start="800020" mid1="0" end1="800021" percentage1="20" mid2="0" end2="800022" 
          percentage2="30" mid3="0" end3="800023" percentage3="50" />
        </routeGroups>
      </trafficFlow>
    </traffic>
  )");
  auto traffic = xml.FirstChildElement("traffic");
  ASSERT_TRUE(traffic != nullptr);
  auto xml_traffic_flow = traffic->FirstChildElement("trafficFlow");
  ASSERT_TRUE(xml_traffic_flow != nullptr);
  Json::Value data(Json::arrayValue);
  CTrafficFlow{}.ParseRouteGroup(xml_traffic_flow, data);
  std::cout << data.toStyledString() << std::endl;
}

TEST_F(TrafficFlowTest, ParsePedestrianSim) {
  TiXmlDocument xml;
  xml.Parse(R"(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <traffic>
      <locations>
        <location id="100001" pos="114.3794315,22.7247789" info="Start10001"/>
        <location id="100002" pos="114.3815197,22.7211420" info="Start10002"/>
        <location id="100003" pos="114.3826679,22.7179155" info="Start10003"/>
      </locations>
      <trafficFlow>
        <pedestrianType>
          <pedType id="101" type="AggressMan" basicModel="man" pedBehavior="1" />
          <!--行人类型，所使用的基础模型（五种之一），以及对应的微观行为类型,由pedBehavior中定义-->
          <pedType id="102" type="ConserveWoman" basicModel="woman" pedBehavior="2" />
          <pedType id="103" type="NormalMan" basicModel="man" pedBehavior="2" />
          <pedType id="104" type="NormalOldman" basicModel="oldman" pedBehavior="3" />
        </pedestrianType>
        <pedComposition>
          <pedComp id="1001" type1="101" percentage1="80" bTree1="Test1" type2="102"
           percentage2="15" bTree2="Test2" type3="103" percentage3="5" bTree3="Test1" info="PedGroup1" />
          <!--定义行人组成，定义行人组成中不同类型（由上面pedType中定义）的组成比例，
          所使用的行为树（由外部文件定义，行为树也描述了行人的路径）。由于行人路径由行为树定义，
          因此对每一个行人产生点都要定义相应的行人组成。 -->
          <pedComp id="1002" type1="102" percentage1="40" bTree1="Test4" type2="103" 
          percentage2="50" bTree2="Test4" type3="104" percentage3="10" bTree3="Test5" info="PedGroup2" />
        </pedComposition>
        <pedBehaviors model="DetourCrowd">
          <!--行人的微观模型控制类型，可以选择社会力SF或者RVO或者DetourCrowd 目前只支持DetourCrowd-->
          <pedBehavior id="1" type="DetourCrowd" maxAngleVel="180" maxNeighbors="40" 
          neighborDist="10" radius="0.3" height="1.8" prefSpeed="1.6" maxSpeed="2" 
          maxAccel="3"  isObstacleAvoidance="1" obstacleAvoidanceType="3" isSeparate="1" 
          separationWeight="0.8" isOptimizeTopo="1" optTimeThr="0.02" />
          <pedBehavior id="2" type="DetourCrowd" maxAngleVel="180" maxNeighbors="40" 
          neighborDist="10" radius="0.3" height="1.8" prefSpeed="2.2" maxSpeed="3" 
          maxAccel="5"  isObstacleAvoidance="1" obstacleAvoidanceType="3" isSeparate="1" 
          separationWeight="0.8" isOptimizeTopo="1" optTimeThr="0.02" />
        </pedBehaviors>
        <pedestrianInput><!--由于行人路径由行为树定义，因此对每一个行人产生点都要定义相应的行人组成。 -->
          <!--行人组成产生点的位置（由location中的对应位置所定义），组成（由上面pedComposition定义），
          以及行人产生的模式（如固定时间间隔）和时间间隔,以及可以产生的最大行人数量-->
          <pedInput id="10001" location="100001" pedComp="1001" distribution="Fixed" timeHeadway="1" 
          duration="10000" max="100" info="PedGroup1"/>
          <pedInput id="10003" location="100003" pedComp="1002" distribution="Fixed" timeHeadway="1" 
          duration="10000" max="100" info="PedGroup2"/>
        </pedestrianInput>
      </trafficFlow>
    </traffic>
  )");
  auto traffic = xml.FirstChildElement("traffic");
  ASSERT_TRUE(traffic != nullptr);
  auto xml_traffic_flow = traffic->FirstChildElement("trafficFlow");
  ASSERT_TRUE(xml_traffic_flow != nullptr);
  Json::Value data(Json::objectValue);
  CTrafficFlow{}.ParsePedestrianSim(xml_traffic_flow, data);
  std::cout << data.toStyledString() << std::endl;
}

TEST_F(TrafficFlowTest, ParseFromXml) {
  TiXmlDocument xml;
  xml.Parse(R"(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <traffic>
        <trafficFlow>
            <vehicleType>
                <vehType id="101" type="Sedan" length="4.5" width="1.8" height="1.5" behavior="TrafficVehicle" />
                <vehType id="103" type="Truck" length="4.5" width="1.8" height="1.5" behavior="TrafficVehicle" />
            </vehicleType>
            <behaviors>
                <behavior id="2" type="Arterial" AX="2" BX_Add="2" BX_Mult="3" />
            </behaviors>
            <vehicleComposition>
                <vehComp id="1001" name="" type1="101" percentage1="80.000000" behavior1="2" aggress1="0.100000" 
                type2="103" percentage2="20.000000" behavior2="2" aggress2="0.100000" />
            </vehicleComposition>
            <vehicleInput>
                <vehInput id="10001" location="100001" composition="1001" start_v="14.667" max_v="16.667" 
                timeHeadway="1" duration="100000" number="0" tid="1" 
                xyz="0.000000000000,0.000000000000,0.000000000000" />
                <vehInput id="10002" location="100002" composition="1001" start_v="14.667" max_v="16.667" 
                timeHeadway="2" duration="100000" number="0" tid="1" 
                xyz="0.000000000000,0.000000000000,0.000000000000" />
            </vehicleInput>
            <vehicleExit>
                <vehExit id="20001" location="100003" tid="1" xyz="0.000000000000,0.000000000000,0.000000000000" />
                <vehExit id="20002" location="100004" tid="1" xyz="0.000000000000,0.000000000000,0.000000000000" />
                <vehExit id="20003" location="100005" tid="1" xyz="0.000000000000,0.000000000000,0.000000000000" />
            </vehicleExit>
            <routeGroups>	
              <routeGroup id="8001" start="800010" mid1="0" end1="800011" percentage1="10" mid2="0" end2="800012" 
              percentage2="80" mid3="0" end3="800013" percentage3="10" />
              <routeGroup id="8002" start="800020" mid1="0" end1="800021" percentage1="20" mid2="0" end2="800022" 
              percentage2="30" mid3="0" end3="800023" percentage3="50" />
            </routeGroups>
            <pedBehaviors>
              <pedBehavior height="1.8" id="1" isObstacleAvoidance="1" isOptimizeTopo="1" isSeparate="1" maxAccel="3" 
              maxAngleVel="180" maxNeighbors="40" maxSpeed="2" neighborDist="10" obstacleAvoidanceType="3" 
              optTimeThr="0.02" prefSpeed="1.6" radius="0.3" separationWeight="0.8" type="DetourCrowd" />
              <pedBehavior height="1.8" id="2" isObstacleAvoidance="1" isOptimizeTopo="1" isSeparate="1" maxAccel="5" 
              maxAngleVel="180" maxNeighbors="40" maxSpeed="3" neighborDist="10" obstacleAvoidanceType="3" 
              optTimeThr="0.02" prefSpeed="2.2" radius="0.3" separationWeight="0.8" type="DetourCrowd" />
              <pedBehavior height="1.8" id="3" isObstacleAvoidance="1" isOptimizeTopo="0" isSeparate="1" maxAccel="3" 
              maxAngleVel="180" maxNeighbors="40" maxSpeed="2" neighborDist="20" obstacleAvoidanceType="3" 
              optTimeThr="0.02" prefSpeed="1.6" radius="0.3" separationWeight="0.8" type="DetourCrowd" />
              <pedBehavior height="1.8" id="4" isObstacleAvoidance="1" isOptimizeTopo="1" isSeparate="0" maxAccel="5" 
              maxAngleVel="180" maxNeighbors="30" maxSpeed="3" neighborDist="20" obstacleAvoidanceType="3" 
              optTimeThr="0.02" prefSpeed="2.2" radius="0.4" separationWeight="0.8" type="DetourCrowd" />
              <pedBehavior height="1.8" id="5" maxAccel="3" maxAngleVel="180" maxNeighbors="40" maxSpeed="2.1" 
              neighborDist="20" prefSpeed="1.6" radius="0.3" tau="4" tauObst="1.50" type="RVO" />
              <pedBehavior height="1.8" id="6" maxAccel="3" maxAngleVel="180" maxNeighbors="40" maxSpeed="2.1" 
              neighborDist="20" prefSpeed="1.6" radius="0.3" tau="4" tauObst="1.50" type="RVO" />
              <pedBehavior bodyForce="1200" forceDistance="0.015" friction="2400" height="1.8" id="7" mass="80" 
              maxAccel="3" maxAngleVel="180" maxNeighbors="40" maxSpeed="2.1" neighborDist="20" 
              prefSpeed="1.6" radius="0.3" reactionTime="0.5" type="SF" />
            </pedBehaviors>
            <pedestrianType>
              <pedType basicModel="man" id="101" pedBehavior="1" />
              <pedType basicModel="woman" id="102" pedBehavior="2" />
              <pedType basicModel="man" id="103" pedBehavior="2" />
              <pedType basicModel="oldman" id="104" pedBehavior="3" />
            </pedestrianType>
            <pedComposition>
              <pedComp bTree1="0" bTree2="0" bTree3="0" id="1001" percentage1="80" percentage2="15" 
              percentage3="5" type1="101" type2="102" type3="103" />
              <pedComp bTree1="0" bTree2="0" bTree3="0" id="1002" percentage1="40" percentage2="50" 
              percentage3="10" type1="102" type2="103" type3="104" />
            </pedComposition>
            <pedestrianInput>
              <pedInput distribution="Fixed" duration="10000" id="10001" info="PedGroup1" 
              location="1" max="100" pedComp="1001" timeHeadway="1" />
              <pedInput distribution="Fixed" duration="10000" id="10003" info="PedGroup2" 
              location="2" max="100" pedComp="1002" timeHeadway="1" />
            </pedestrianInput>
            <pedSimulatoinOfFront>
              <![CDATA[]]>
            </pedSimulatoinOfFront>
            <pedBehaviorTrees>
              <pedBehaviorTree>_btree_0.xml</pedBehaviorTree>
            </pedBehaviorTrees>
        </trafficFlow>
        <locations>
            <location id="100001" pos="114.421776,23.099460" info="vehInput" />
            <location id="100002" pos="114.423708,23.101415" info="vehInput" />
            <location id="100003" pos="114.422002,23.098856" info="vehExit" />
            <location id="100004" pos="114.423337,23.101046" info="vehExit" />
            <location id="100005" pos="114.418393,23.100627" info="vehExit" />
            <location id="800010" pos="114.4222270,23.0988283" info="Route8001"/>
            <location id="800011" pos="114.4206245,23.1005751" info="Route8001"/>
            <location id="800012" pos="114.4211009,23.1008316" info="Route8001"/>
            <location id="800013" pos="114.4215938,23.1003306" info="Route8001"/>
            <location id="800020" pos="114.4235238,23.1012865" info="Route8002"/>
            <location id="800021" pos="114.4212187,23.0999158" info="Route8002"/>
            <location id="800022" pos="114.4206245,23.1005751" info="Route8002"/>
            <location id="800023" pos="114.4213650,23.1007345" info="Route8002"/>
            <location id="1" pos="114.379431,22.724779" info="pedInput" />
            <location id="2" pos="114.382668,22.717916" info="pedInput" />
        </locations>
    </traffic>
  )");
  auto traffic = xml.FirstChildElement("traffic");
  ASSERT_TRUE(traffic != nullptr);
  auto xml_traffic_flow = traffic->FirstChildElement("trafficFlow");
  ASSERT_TRUE(xml_traffic_flow != nullptr);
  CTrafficFlow traffic_flow;
  traffic_flow.ParseFromXml(xml_traffic_flow);
  std::cout << traffic_flow.JsonMeta().toStyledString() << std::endl;
}

TEST_F(TrafficFlowTest, SerializeVehicleComposition) {
  TiXmlDocument xml;
  xml.Parse(R"(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <traffic>
      <trafficFlow>
      </trafficFlow>
    </traffic>
  )");

  Json::Value json;
  Json::Reader{}.parse(R"(
    {"vehicleComposition":[{"aggressiveness":0.1,"generationType":{"Sedan":80,"Truck":20},"id":1001,"name":""}]}
  )",
                       json);

  auto xml_traffic_flow = xml.FirstChildElement("traffic")->FirstChildElement("trafficFlow");

  CTrafficFlow traffic_flow;
  traffic_flow.SerializeVehicleComposition(json["vehicleComposition"], xml_traffic_flow);

  TiXmlPrinter printer;
  xml.Accept(&printer);
  std::cout << printer.CStr() << std::endl;
}

TEST_F(TrafficFlowTest, SerializeVehicleInputAndExit) {
  TiXmlDocument xml;
  xml.Parse(R"(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <traffic>
      <trafficFlow>
      </trafficFlow>
    </traffic>
  )");

  Json::Value json;
  Json::Reader{}.parse(R"(
    {
      "vehiclePoint":{
        "disappearPoint":[
          {"id":20001,"lat":23.0988557,"lon":114.4220023,"trafficFlowId":1,"x":0,"y":0,"z":0},
          {"id":20002,"lat":23.1010456,"lon":114.4233371,"trafficFlowId":1,"x":0,"y":0,"z":0},
          {"id":20003,"lat":23.1006265,"lon":114.4183928,"trafficFlowId":1,"x":0,"y":0,"z":0}
        ],"generationPoint":[
          {"generationCarsNum":0,"generationInterval":1,"id":10001,"initialSpeed":14.667,"lat":23.0994605,
          "lifePeriod":100000,"lon":114.4217763,"maxSpeed":16.667,"trafficFlowId":1,
          "vehicleCompositionId":1001,"x":0,"y":0,"z":0},
          {"generationCarsNum":0,"generationInterval":2,"id":10002,"initialSpeed":14.667,"lat":23.1014154,
          "lifePeriod":100000,"lon":114.4237081,"maxSpeed":16.667,"trafficFlowId":1,
          "vehicleCompositionId":1001,"x":0,"y":0,"z":0}
        ]
      }
    }
  )",
                       json);

  auto xml_traffic_flow = xml.FirstChildElement("traffic")->FirstChildElement("trafficFlow");

  CTrafficFlow traffic_flow;
  traffic_flow.SerializeVehicleInputAndExit(json["vehiclePoint"], xml_traffic_flow);

  TiXmlPrinter printer;
  xml.Accept(&printer);
  std::cout << printer.CStr() << std::endl;
}

TEST_F(TrafficFlowTest, SerializeRouteGroup) {
  TiXmlDocument xml;
  xml.Parse(R"(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <traffic>
      <trafficFlow>
      </trafficFlow>
    </traffic>
  )");

  Json::Value json;
  Json::Reader{}.parse(R"(
    {
      "decisionRoute":[
        {
          "destRoute":[{"lat":23.1005751,"lon":114.4206245,"trafficFlow":10},
          {"lat":23.1008316,"lon":114.4211009,"trafficFlow":80},
          {"lat":23.1003306,"lon":114.4215938,"trafficFlow":10}],
          "startRoute":[{"lat":23.0988283,"lon":114.422227}]
        },{
          "destRoute":[{"lat":23.0999158,"lon":114.4212187,"trafficFlow":20},
          {"lat":23.1005751,"lon":114.4206245,"trafficFlow":30},
          {"lat":23.1007345,"lon":114.421365,"trafficFlow":50}],
          "startRoute":[{"lat":23.1012865,"lon":114.4235238}]
        }
      ]
    }
  )",
                       json);

  auto xml_traffic_flow = xml.FirstChildElement("traffic")->FirstChildElement("trafficFlow");

  CTrafficFlow traffic_flow;
  traffic_flow.SerializeRouteGroup(json["decisionRoute"], xml_traffic_flow);

  TiXmlPrinter printer;
  xml.Accept(&printer);
  std::cout << printer.CStr() << std::endl;
}

TEST_F(TrafficFlowTest, SerializePedestrianSim) {
  TiXmlDocument xml;
  xml.Parse(R"(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <traffic>
      <trafficFlow>
      </trafficFlow>
    </traffic>
  )");

  Json::Value json;
  Json::Reader{}.parse(R"()", json);

  auto xml_traffic_flow = xml.FirstChildElement("traffic")->FirstChildElement("trafficFlow");

  CTrafficFlow traffic_flow;
  traffic_flow.SerializePedestrianSim(json, xml_traffic_flow);

  TiXmlPrinter printer;
  xml.Accept(&printer);
  std::cout << printer.CStr() << std::endl;
}

TEST_F(TrafficFlowTest, SerializeToXml) {
  TiXmlDocument xml;
  xml.Parse(R"(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <traffic>
      <trafficFlow>
      </trafficFlow>
    </traffic>
  )");

  Json::Value json;
  Json::Reader{}.parse(R"()", json);

  auto xml_traffic_flow = xml.FirstChildElement("traffic")->FirstChildElement("trafficFlow");

  CTrafficFlow traffic_flow;
  traffic_flow.JsonMeta() = json;
  traffic_flow.SerializeToXml(xml_traffic_flow);

  TiXmlPrinter printer;
  xml.Accept(&printer);
  std::cout << printer.CStr() << std::endl;
}