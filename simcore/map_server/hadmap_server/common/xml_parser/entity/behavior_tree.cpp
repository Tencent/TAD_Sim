/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/behavior_tree.h"

#include <regex>

#include "common/utils/misc.h"

TiXmlDocument BehaviorTree::Save(const Json::Value& root) {
  // {"id":1,"points":[{"lon":0,"lat":0}]}
  if (!root.isObject()) {
    return {};
  }
  /*
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
  */
  TiXmlDocument xml;
  xml.Parse(kBTreeTpl);
  xml.FirstChildElement("behavior")->SetAttribute("name", root.get("id", 0).asString());

  int id = 0;

  for (auto& point : root.get("points", Json::Value(Json::arrayValue))) {
    if (!point.isObject()) {
      continue;
    }
    if (!point["lon"].isDouble() || !point["lat"].isDouble()) {
      continue;
    }
    CPoint3D xyz(point.get("lon", 0.0).asDouble(), point.get("lat", 0.0).asDouble(), point.get("alt", 0.0).asDouble());
    auto action = CreateInitActionNode(++id, xyz);
    auto loop = CreateDecoratorLoopUntilNode(++id, CreateStepActionNode(++id));
    auto behavior = xml.FirstChildElement("behavior");
    behavior->FirstChildElement("node")->InsertEndChild(action);
    behavior->FirstChildElement("node")->InsertEndChild(loop);
  }

  return xml;
}

TiXmlDocument BehaviorTree::Save(const std::string& json_str) {
  Json::Value input;
  input << json_str;
  return Save(input);
}

Json::Value BehaviorTree::Parse(const TiXmlDocument& xml) {
  Json::Value root(Json::objectValue);

  const auto* behavior = xml.FirstChildElement("behavior");
  if (!behavior) {
    return root;
  }

  const auto* sequence = behavior->FirstChildElement("node");
  if (!sequence) {
    return root;
  }

  root["id"] = GetXmlAttribute(behavior, "name", 0);
  root["points"] = Json::Value(Json::arrayValue);

  for (const auto* node = sequence->FirstChildElement("node"); node; node = node->NextSiblingElement("node")) {
    // init
    std::string method = GetXmlAttribute(node->FirstChildElement("property"), "Method", std::string{});
    std::smatch m;
    if (std::regex_search(method, m, std::regex(R"(\((.*?)\))")) && m.size() > 1) {
      CPoint3D point(m[1].str());
      Json::Value one;
      one["lon"] = point.X();
      one["lat"] = point.Y();
      root["points"].append(one);
    }
    // step
    node = node->NextSiblingElement("node");
    if (!node) {
      break;
    }
  }

  return root;
}

Json::Value BehaviorTree::Parse(const std::string& file_path) {
  TiXmlDocument doc;
  doc.LoadFile(file_path.c_str());
  return Parse(doc);
}

TiXmlElement BehaviorTree::CreateInitActionNode(int id, const CPoint3D& point) {
  TiXmlElement node("node");
  node.Parse(kInitActionNodeTpl, 0, TIXML_ENCODING_UNKNOWN);
  node.SetAttribute("id", id);

  std::string method = "Self.bt::Controller::setTargetPosition(" + point.ToString() + ")";
  node.FirstChildElement("property")->SetAttribute("Method", method);
  return node;
}

TiXmlElement BehaviorTree::CreateStepActionNode(int id) {
  TiXmlElement node("node");
  node.Parse(kStepActionNodeTpl, 0, TIXML_ENCODING_UNKNOWN);
  node.SetAttribute("id", id);
  return node;
}

TiXmlElement BehaviorTree::CreateDecoratorLoopUntilNode(int id, const TiXmlElement& step) {
  TiXmlElement node("node");
  node.Parse(kDecoratorLoopUntilNodeTpl, 0, TIXML_ENCODING_UNKNOWN);
  node.SetAttribute("id", id);
  node.InsertEndChild(step);
  return node;
}
