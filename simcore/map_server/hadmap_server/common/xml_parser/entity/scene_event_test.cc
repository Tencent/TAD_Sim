/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/scene_event.h"
#include "common/utils/misc.h"

#include "gtest/gtest.h"
#include "json/json.h"
#include "tinyxml.h"

class SceneEventTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {}
};

template <typename T>
T GetXmlAttribute(TiXmlElement* xml, const char* key, T default_value = {}) {
  if (!xml) {
    return default_value;
  }
  const char* p = xml->Attribute(key);
  if (p) {
    std::istringstream iss(p);
    iss >> default_value;
  }
  return default_value;
}

#define SCENE_EVENT_EQ_XML(scene_event, xml)                                                     \
  TiXmlDocument doc;                                                                             \
  doc.Parse(xml);                                                                                \
  scene_event.m_id;                                                                              \
  auto event = doc.FirstChildElement("event");                                                   \
  ASSERT_TRUE(event != nullptr);                                                                 \
  EXPECT_EQ(GetXmlAttribute(event, "id", std::string{}), scene_event.m_id);                      \
  EXPECT_EQ(GetXmlAttribute(event, "name", std::string{}), scene_event.strName);                 \
  EXPECT_EQ(GetXmlAttribute(event, "endCondition", std::string{}), scene_event.strEndCondition); \
  EXPECT_EQ(GetXmlAttribute(event, "action", std::string{}), scene_event.strAction);

TEST_F(SceneEventTest, time_trigger) {
  {
    // v1.1
    Json::Value root;
    Json::Reader{}.parse(R"()", root);
    SceneEvent scene_event;
    scene_event.fromJson2Xml(root);
    SCENE_EVENT_EQ_XML(scene_event, R"(
      <event id="0" name="new_trigger_1" type="time_trigger" 
      condition="trigger_time:1" endCondition="" action="[]" info="" />
    )");

    scene_event.SegmentString();
    EXPECT_EQ(scene_event.saveJson(), root);
  }
  {
    Json::Value root;
    Json::Reader{}.parse(R"()", root);
    SceneEvent scene_event;
    scene_event.fromJson2Xml(root);
    SCENE_EVENT_EQ_XML(scene_event, R"()");
  }
  {
    // v1.0
    Json::Value root;
    Json::Reader{}.parse(R"()", root);
    SceneEvent scene_event;
    scene_event.fromJson2Xml(root, "1.0.0.0");
    SCENE_EVENT_EQ_XML(scene_event, R"()");

    scene_event.SegmentString1_0();
    EXPECT_EQ(scene_event.saveJson("1.0.0.0"), root);
  }
  {
    // v1.0
    Json::Value root;
    Json::Reader{}.parse(R"()", root);
    SceneEvent scene_event;
    scene_event.fromJson2Xml(root, "1.0.0.0");
    SCENE_EVENT_EQ_XML(scene_event, R"()");
    scene_event.SegmentString1_0();
    EXPECT_EQ(scene_event.ActionVer.size(), 3);
    EXPECT_EQ(scene_event.saveJson("1.0.0.0"), root);
  }
}

TEST_F(SceneEventTest, ttc_trigger) {
  Json::Value root;
  Json::Reader{}.parse(R"()", root);
  SceneEvent scene_event;
  scene_event.fromJson2Xml(root);

  SCENE_EVENT_EQ_XML(scene_event, R"(
    <event id="1" name="new_trigger_2" type="ttc_trigger" condition="ttc_threshold:3,equation_op:eq,
    target_element:ego,distance_type:euclideandistance,condition_boundary:rising,trigger_count:1" 
    endCondition="" action="[]" info="" />
  )");
}

TEST_F(SceneEventTest, EncodeActionToJson) {
  std::string s = R"()";
  s = R"()";
  std::cout << JsonToString(SceneEvent::EncodeActionToJson(s)) << std::endl;

  s = R"([{actionid:0,type:assign,value:L4_Planning,subtype:none,offset:0.0,multi:{resume_sw:false,cancel_sw:false,
  speed_inc_sw:false,speed_dec_sw:false,set_timegap:null,set_speed:20}}])";
  EXPECT_EQ(JsonToString(SceneEvent::EncodeActionToJson(s)), JsonToString(SceneEvent{}.MakeJson(s)));
  EXPECT_EQ(JsonToString(SceneEvent::EncodeActionToJson("")), "[]");
  EXPECT_EQ(JsonToString(SceneEvent::EncodeActionToJson("[]")), "[]");
  EXPECT_EQ(JsonToString(SceneEvent::EncodeActionToJson("[{}]")), "[]");
}
