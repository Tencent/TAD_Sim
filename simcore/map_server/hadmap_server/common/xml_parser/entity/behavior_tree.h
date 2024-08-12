/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <json/json.h>
#include <tinyxml.h>

#include "common/engine/math/vec3.h"

/*

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

*/
class BehaviorTree {
 public:
  TiXmlDocument Save(const Json::Value& root);

  TiXmlDocument Save(const std::string& json_str);

  Json::Value Parse(const TiXmlDocument& root);

  Json::Value Parse(const std::string& file_path);

 private:
  TiXmlElement CreateInitActionNode(int id, const CPoint3D& point);

  TiXmlElement CreateStepActionNode(int id);

  TiXmlElement CreateDecoratorLoopUntilNode(int id, const TiXmlElement& step);

  template <typename T>
  static T GetXmlAttribute(const TiXmlElement* xml, const char* key, T default_value = {}) {
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

 private:
  static constexpr const char* kInitActionNodeTpl = R"tpl(
    <node class="Action" id="0">
      <property Method="Self.bt::Controller::setTargetPosition(lon,lat,alt)" />
      <property ResultOption="BT_SUCCESS" />
    </node>
  )tpl";

  static constexpr const char* kStepActionNodeTpl = R"tpl(
    <node class="Action" id="0">
      <property Method="Self.bt::Controller::step()" />
      <property ResultOption="BT_INVALID" />
    </node>
  )tpl";

  static constexpr const char* kDecoratorLoopUntilNodeTpl = R"tpl(
    <node class="DecoratorLoopUntil" id="0">
      <property Count="const int -1" />
      <property DecorateWhenChildEnds="true" />
      <property Until="true" />
    </node>
  )tpl";

  static constexpr const char* kBTreeTpl = R"tpl(
    <?xml version="1.0" encoding="utf-8"?>
    <behavior name="test" agenttype="bt::Controller" version="5">
      <node class="Sequence" id="0">
      </node>
    </behavior>
  )tpl";
};
