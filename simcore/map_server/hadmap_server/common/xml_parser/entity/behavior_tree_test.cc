/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/behavior_tree.h"

#include "gtest/gtest.h"

TEST(BehaviorTreeTest, Save) {
  Json::Value root;
  Json::Reader{}.parse(R"(
    {"id":1,"points":[{"lon":111,"lat":0.01}]}
  )",
                       root);
  TiXmlDocument doc = BehaviorTree{}.Save(root);
  TiXmlPrinter printer;
  doc.Accept(&printer);
  std::cout << printer.CStr() << std::endl;
}

TEST(BehaviorTreeTest, Parse) {
  TiXmlDocument doc;
  doc.Parse(R"raw(
<?xml version="1.0" encoding="utf-8"?>
<!--EXPORTED BY TOOL, DON'T MODIFY IT!-->
<!--Source File: test.xml-->
<behavior name="test" agenttype="bt::Controller" version="5">
  <node class="Sequence" id="0">
    <node class="Action" id="10">
      <property Method="Self.bt::Controller::setTargetPosition(-4547,2340,25)" />
      <property ResultOption="BT_SUCCESS" />
    </node>
    <node class="DecoratorLoopUntil" id="2">
      <property Count="const int -1" />
      <property DecorateWhenChildEnds="true" />
      <property Until="true" />
      <node class="Action" id="3">
        <property Method="Self.bt::Controller::step()" />
        <property ResultOption="BT_INVALID" />
      </node>
    </node>
    <node class="Action" id="1">
      <property Method="Self.bt::Controller::setTargetPosition(2698,1856,25)" />
      <property ResultOption="BT_SUCCESS" />
    </node>
    <node class="DecoratorLoopUntil" id="4">
      <property Count="const int -1" />
      <property DecorateWhenChildEnds="true" />
      <property Until="true" />
      <node class="Action" id="8">
        <property Method="Self.bt::Controller::step()" />
        <property ResultOption="BT_INVALID" />
      </node>
    </node>
    <node class="Action" id="6">
      <property Method="Self.bt::Controller::setTargetPosition(-4430,2620,25)" />
      <property ResultOption="BT_SUCCESS" />
    </node>
    <node class="DecoratorLoopUntil" id="11">
      <property Count="const int -1" />
      <property DecorateWhenChildEnds="true" />
      <property Until="true" />
      <node class="Action" id="12">
        <property Method="Self.bt::Controller::step()" />
        <property ResultOption="BT_INVALID" />
      </node>
    </node>
  </node>
</behavior>
  )raw");
  Json::Value root = BehaviorTree{}.Parse(doc);
  std::cout << root.toStyledString() << std::endl;
}