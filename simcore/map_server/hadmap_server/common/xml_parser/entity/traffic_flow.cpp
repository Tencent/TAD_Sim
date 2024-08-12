/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/traffic_flow.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>

#include "common/engine/math/vec3.h"
#include "common/utils/misc.h"
#include "common/xml_parser/entity/behavior_tree.h"

#define FOREACH_XML(parent, name, iter) \
  for (TiXmlElement* iter = parent->FirstChildElement(name); iter; iter = iter->NextSiblingElement(name))

#define FOREACH_ATTR(node, iter) for (TiXmlAttribute* iter = node->FirstAttribute(); iter; iter = iter->Next())

#define STR(s) #s

#define FIND_OR_CREATE_NODE(parent, node, name)                     \
  TiXmlElement* node = parent->FirstChildElement(name);             \
  if (!node) {                                                      \
    node = parent->InsertEndChild(TiXmlElement(name))->ToElement(); \
  }

Json::Value ParseLonLatByLoc(TiXmlElement* xml, const Json::Value& locs, const std::string& key) {
  const char* loc = xml->Attribute(key.c_str());
  if (!loc || strlen(loc) == 0) {
    return Json::Value();
  }
  auto pos = locs.get(loc, Json::Value(Json::objectValue)).get("pos", "").asString();
  if (pos.empty()) {
    return Json::Value();
  }
  CPoint3D point(pos);
  Json::Value result;
  result["lon"] = point.X();
  result["lat"] = point.Y();
  return result;
}

std::map<std::string, CTrafficFlow::VehType> CTrafficFlow::veh_type_map_ = {
    {"101", CTrafficFlow::VehType("101", "Sedan", "4.5", "1.8", "1.5", "TrafficVehicle")},
    {"103", CTrafficFlow::VehType("103", "Truck", "4.5", "1.8", "1.5", "TrafficVehicle")},
    {"Sedan", CTrafficFlow::VehType("101", "Sedan", "4.5", "1.8", "1.5", "TrafficVehicle", true)},
    {"Truck", CTrafficFlow::VehType("103", "Truck", "4.5", "1.8", "1.5", "TrafficVehicle", true)},
};

const char* CTrafficFlow::kVehInputString = "vehInput";
const char* CTrafficFlow::kVehExitString = "vehExit";
const char* CTrafficFlow::kVehRouteString = "routeGroup";

void CTrafficFlow::ParseFromXml(TiXmlElement* xml_traffic_flow) {
  if (!xml_traffic_flow || !xml_traffic_flow->Parent()) {
    return;
  }
  json_meta_.clear();
  ParseVehicleComposition(xml_traffic_flow, json_meta_["vehicleComposition"]);
  ParseVehicleInputAndExit(xml_traffic_flow, json_meta_["vehiclePoint"]);
  ParseRouteGroup(xml_traffic_flow, json_meta_["decisionRoute"]);

  ParsePedestrianSim(xml_traffic_flow, json_meta_);
}

void CTrafficFlow::SerializeToXml(TiXmlElement* xml_traffic_flow) {
  if (!xml_traffic_flow || !xml_traffic_flow->Parent()) {
    return;
  }
  SerializeDefaultVehType(xml_traffic_flow);
  SerializeDefaultBehavior(xml_traffic_flow);
  SerializeVehicleComposition(json_meta_["vehicleComposition"], xml_traffic_flow);
  SerializeVehicleInputAndExit(json_meta_["vehiclePoint"], xml_traffic_flow);
  SerializeRouteGroup(json_meta_["decisionRoute"], xml_traffic_flow);

  SerializePedestrianSim(json_meta_, xml_traffic_flow);
}

void CTrafficFlow::ParseVehicleComposition(TiXmlElement* xml_traffic_flow, Json::Value& json_comps) {
  if (!xml_traffic_flow) {
    return;
  }

  TiXmlElement* xml_veh_comps = xml_traffic_flow->FirstChildElement("vehicleComposition");
  if (!xml_veh_comps) {
    return;
  }

  FOREACH_XML(xml_veh_comps, "vehComp", xml_veh_comp) {
    Json::Value one(Json::objectValue);
    one["id"] = GetXmlAttribute(xml_veh_comp, "id", 0);
    one["name"] = GetXmlAttribute(xml_veh_comp, "name", std::string{});

    // set generationType and aggress
    ParseVehComp(xml_veh_comp, one);

    json_comps.append(one);
  }
}

void CTrafficFlow::ParseVehComp(TiXmlElement* xml_veh_comp, Json::Value& json_comp) {
  if (!xml_veh_comp) {
    return;
  }

  auto get_value = [&](const std::string& key, size_t index) {
    return GetXmlAttribute(xml_veh_comp, (key + std::to_string(index)).c_str(), std::string{});
  };

  auto& aggress = json_comp["aggress"];
  auto& generation_type = json_comp["generationType"];
  for (size_t i = 0; i < kVehTypeCount; i++) {
    std::string type = get_value("type", i + 1);
    if (type.empty()) {
      continue;
    }
    auto iter = veh_type_map_.find(type);
    if (iter == veh_type_map_.end()) {
      continue;
    }
    generation_type[iter->second.type] = atof(get_value("percentage", i + 1).c_str());
    aggress[iter->second.type] = atof(get_value("aggress", i + 1).c_str());
  }
}

void CTrafficFlow::ParseVehicleInputAndExit(TiXmlElement* xml_traffic_flow, Json::Value& json_veh_points) {
  if (!xml_traffic_flow) {
    return;
  }

  Json::Value json_locations;
  auto parent = xml_traffic_flow->Parent();
  if (parent) {
    auto xml_locations = parent->FirstChildElement("locations");
    ParseLocation(xml_locations, json_locations);
  }

  ParseVehicleInputAndExitImpl(xml_traffic_flow->FirstChildElement("vehicleInput"), kVehInputString, json_locations,
                               json_veh_points["generationPoint"]);

  ParseVehicleInputAndExitImpl(xml_traffic_flow->FirstChildElement("vehicleExit"), kVehExitString, json_locations,
                               json_veh_points["disappearPoint"]);
}

void CTrafficFlow::ParseVehicleInputAndExitImpl(TiXmlElement* xml_veh_points, const std::string& child_name,
                                                const Json::Value& json_locations, Json::Value& json_veh_points) {
  if (json_veh_points.isArray()) {
    json_veh_points = Json::Value(Json::arrayValue);
  }

  if (!xml_veh_points) {
    return;
  }
  FOREACH_XML(xml_veh_points, child_name.c_str(), xml_veh_point) {
    Json::Value one(Json::objectValue);
    if (child_name == kVehInputString) {
      one["generationCarsNum"] = GetXmlAttribute(xml_veh_point, "number", 0);
      one["generationInterval"] = GetXmlAttribute(xml_veh_point, "timeHeadway", 0);
      one["initialSpeed"] = GetXmlAttribute(xml_veh_point, "start_v", 0.0);
      one["lifePeriod"] = GetXmlAttribute(xml_veh_point, "duration", 0);
      one["maxSpeed"] = GetXmlAttribute(xml_veh_point, "max_v", 0.0);
      one["vehicleCompositionId"] = GetXmlAttribute(xml_veh_point, "composition", 0);
    }
    // common
    one["id"] = GetXmlAttribute(xml_veh_point, "id", 0);
    // x, y, z
    CPoint3D xyz(GetXmlAttribute(xml_veh_point, "xyz", std::string{}));
    one["x"] = xyz.X();
    one["y"] = xyz.Y();
    one["z"] = xyz.Z();
    // lon, lat
    auto loc = GetXmlAttribute(xml_veh_point, "location", std::string{});
    auto pos = json_locations.get(loc, Json::Value(Json::objectValue)).get("pos", "").asString();
    CPoint3D point(pos);
    one["lon"] = point.X();
    one["lat"] = point.Y();

    json_veh_points.append(one);
  }
}

void CTrafficFlow::ParseRouteGroup(TiXmlElement* xml_traffic_flow, Json::Value& json_route_groups) {
  if (!xml_traffic_flow) {
    return;
  }

  Json::Value json_locations;
  if (xml_traffic_flow->Parent()) {
    auto xml_locations = xml_traffic_flow->Parent()->FirstChildElement("locations");
    ParseLocation(xml_locations, json_locations);
  }

  TiXmlElement* xml_route_groups = xml_traffic_flow->FirstChildElement("routeGroups");
  if (!xml_route_groups) {
    return;
  }

  FOREACH_XML(xml_route_groups, "routeGroup", xml_route_group) {
    Json::Value json_route_group;
    // one start
    Json::Value start = ParseLonLatByLoc(xml_route_group, json_locations, "start");
    if (!start.empty()) {
      json_route_group["startRoute"].append(start);
    }
    // multi end
    for (int i = 1; true; i++) {
      std::string endkey = std::string("end") + std::to_string(i);
      if (!xml_route_group->Attribute(endkey.c_str())) {
        break;
      }
      std::string percentagekey = std::string("percentage") + std::to_string(i);
      Json::Value dest = ParseLonLatByLoc(xml_route_group, json_locations, endkey);
      if (!dest.empty()) {
        dest["trafficFlow"] = GetXmlAttribute(xml_route_group, percentagekey.c_str(), 0);
        json_route_group["destRoute"].append(dest);
      }
    }
    json_route_group["id"] = GetXmlAttribute(xml_route_group, "id", 0);
    if (!json_route_group.empty()) {
      json_route_groups.append(json_route_group);
    }
  }
}

void CTrafficFlow::ParseLocation(TiXmlElement* xml_locations, Json::Value& json_locations) {
  if (!xml_locations) {
    return;
  }

  FOREACH_XML(xml_locations, "location", xml_location) {
    Json::Value one(Json::objectValue);
    one["id"] = GetXmlAttribute(xml_location, "id", std::string{});
    one["pos"] = GetXmlAttribute(xml_location, "pos", std::string{});
    one["info"] = GetXmlAttribute(xml_location, "info", std::string{});
    json_locations[one["id"].asString()] = one;
  }
}

void CTrafficFlow::ParsePedestrianSim(TiXmlElement* xml_traffic_flow, Json::Value& json_pedestrian_sim) {
  if (!xml_traffic_flow) {
    return;
  }

  auto to_json_object = [](TiXmlElement* node) {
    Json::Value o(Json::objectValue);
    FOREACH_ATTR(node, iter) { o[iter->Name()] = iter->Value(); }
    return o;
  };

  // pedBehaviors
  auto& json_ped_behaviors = json_pedestrian_sim["pedBehaviors"];
  TiXmlElement* xml_ped_behaviors = xml_traffic_flow->FirstChildElement("pedBehaviors");
  if (xml_ped_behaviors) {
    FOREACH_XML(xml_ped_behaviors, "pedBehavior", xml_ped_behavior) {
      Json::Value one = to_json_object(xml_ped_behavior);
      one["id"] = GetXmlAttribute(xml_ped_behavior, "id", 0);
      json_ped_behaviors.append(one);
    }
  }

  // pedestrianType
  auto& json_ped_types = json_pedestrian_sim["pedestrianType"];
  TiXmlElement* xml_ped_types = xml_traffic_flow->FirstChildElement("pedestrianType");
  if (xml_ped_types) {
    FOREACH_XML(xml_ped_types, "pedType", xml_ped_type) {
      Json::Value one(Json::objectValue);
      one["id"] = GetXmlAttribute(xml_ped_type, "id", 0);
      one["basicModel"] = GetXmlAttribute(xml_ped_type, "basicModel", std::string{});
      one["pedBehavior"] = GetXmlAttribute(xml_ped_type, "pedBehavior", 0);
      json_ped_types.append(one);
    }
  }

  // pedComposition
  auto& json_ped_comps = json_pedestrian_sim["pedComposition"];
  TiXmlElement* xml_ped_comps = xml_traffic_flow->FirstChildElement("pedComposition");
  if (xml_ped_comps) {
    FOREACH_XML(xml_ped_comps, "pedComp", xml_ped_comp) {
      Json::Value one(Json::objectValue);
      one["id"] = GetXmlAttribute(xml_ped_comp, "id", 0);
      one["types"] = Json::Value(Json::arrayValue);
      for (int i = 1; i < 64; i++) {
        std::string idx = std::to_string(i);
        std::string key_type = "type" + idx;
        if (xml_ped_comp->Attribute(key_type) == nullptr) {
          break;
        }
        std::string key_percentage = "percentage" + idx;
        std::string key_btree = "bTree" + idx;
        Json::Value inner_one(Json::objectValue);
        inner_one["id"] = GetXmlAttribute(xml_ped_comp, key_type.c_str(), 0);
        inner_one["percentage"] = GetXmlAttribute(xml_ped_comp, key_percentage.c_str(), std::string{});
        inner_one["bTree"] = GetXmlAttribute(xml_ped_comp, key_btree.c_str(), 0);
        one["types"].append(inner_one);
      }
      json_ped_comps.append(one);
    }
  }

  Json::Value json_locations(Json::objectValue);
  auto parent = xml_traffic_flow->Parent();
  if (parent) {
    auto xml_locations = parent->FirstChildElement("locations");
    ParseLocation(xml_locations, json_locations);
  }

  // pedestrianInput
  auto& json_ped_inputs = json_pedestrian_sim["pedestrianInput"];
  TiXmlElement* xml_ped_inputs = xml_traffic_flow->FirstChildElement("pedestrianInput");
  if (xml_ped_inputs) {
    FOREACH_XML(xml_ped_inputs, "pedInput", xml_ped_input) {
      Json::Value one = to_json_object(xml_ped_input);
      one["id"] = GetXmlAttribute(xml_ped_input, "id", 0);
      one["pedComp"] = GetXmlAttribute(xml_ped_input, "pedComp", 0);
      one.removeMember("location");
      // lon, lat
      auto loc = GetXmlAttribute(xml_ped_input, "location", std::string{});
      auto pos = json_locations.get(loc, Json::Value(Json::objectValue)).get("pos", "").asString();
      CPoint3D point(pos);
      one["location"]["lon"] = point.X();
      one["location"]["lat"] = point.Y();
      json_ped_inputs.append(one);
    }
  }

  // pedRoute
  auto& json_ped_routes = json_pedestrian_sim["pedRoute"];
  TiXmlElement* xml_ped_btrees = xml_traffic_flow->FirstChildElement("pedBehaviorTrees");
  if (xml_ped_btrees) {
    FOREACH_XML(xml_ped_btrees, "pedBehaviorTree", xml_ped_btree) {
      boost::filesystem::path file(file_path_);
      file = file.parent_path() / xml_ped_btree->GetText();
      json_ped_routes.append(BehaviorTree{}.Parse(file.string()));
    }
  }

  // pedSimulatoinOfFront
  TiXmlElement* xml_ped_front = xml_traffic_flow->FirstChildElement("pedSimulatoinOfFront");
  if (xml_ped_front) {
    json_pedestrian_sim["pedSimulatoinOfFront"] = StringToJson(xml_ped_front->GetText());
  }
}

void CTrafficFlow::SerializeVehicleComposition(const Json::Value& json_comps, TiXmlElement* xml_traffic_flow) {
  if (!json_comps.isArray()) {
    return;
  }

  TiXmlElement* xml_veh_comps = xml_traffic_flow->FirstChildElement("vehicleComposition");
  if (!xml_veh_comps) {
    xml_veh_comps = xml_traffic_flow->InsertEndChild(TiXmlElement("vehicleComposition"))->ToElement();
  }

  for (auto&& json_comp : json_comps) {
    TiXmlElement* one = new TiXmlElement("vehComp");
    SetXmlAttribute(one, "id", json_comp.get("id", 0).asInt());
    SetXmlAttribute(one, "name", json_comp.get("name", "").asCString());

    SerializeVehComp(json_comp, one);

    xml_veh_comps->LinkEndChild(one);
  }
}

void CTrafficFlow::SerializeVehComp(const Json::Value& json_comp, TiXmlElement* xml_veh_comp) {
  int index = 0;
  Json::Value aggress = json_comp.get("aggress", Json::Value(Json::objectValue));

  auto set_value = [&](const std::string& key, size_t index, const std::string& val) {
    xml_veh_comp->SetAttribute((key + std::to_string(index)).c_str(), val.c_str());
  };

  Json::Value generation_type = json_comp.get("generationType", Json::Value(Json::objectValue));
  for (auto iter = generation_type.begin(); iter != generation_type.end(); iter++) {
    std::string type = iter.key().asString();
    if (veh_type_map_.count(type) <= 0) {
      continue;
    }
    if (!iter->isDouble()) {
      continue;
    }
    ++index;
    set_value("type", index, veh_type_map_.at(type).id);
    set_value("percentage", index, std::to_string(iter->asFloat()));
    set_value("behavior", index, std::to_string(kDefaultBehavior));
    set_value("aggress", index, std::to_string(aggress.get(type, 0.0).asDouble()));
  }
}

void CTrafficFlow::SerializeVehicleInputAndExit(const Json::Value& json_veh_points, TiXmlElement* xml_traffic_flow) {
  if (!json_veh_points.isObject()) {
    return;
  }

  if (!xml_traffic_flow->Parent()->FirstChildElement("locations")) {
    xml_traffic_flow->Parent()->InsertEndChild(TiXmlElement("locations"));
  }

  if (!xml_traffic_flow->FirstChildElement("vehicleInput")) {
    xml_traffic_flow->InsertEndChild(TiXmlElement("vehicleInput"));
  }

  if (!xml_traffic_flow->FirstChildElement("vehicleExit")) {
    xml_traffic_flow->InsertEndChild(TiXmlElement("vehicleExit"));
  }

  SerializeVehicleInputAndExitImpl(json_veh_points["generationPoint"], kVehInputString,
                                   xml_traffic_flow->FirstChildElement("vehicleInput"));

  SerializeVehicleInputAndExitImpl(json_veh_points["disappearPoint"], kVehExitString,
                                   xml_traffic_flow->FirstChildElement("vehicleExit"));
}

void CTrafficFlow::SerializeVehicleInputAndExitImpl(const Json::Value& json_veh_points,
                                                    const std::string& veh_point_type, TiXmlElement* xml_veh_points) {
  if (!xml_veh_points) {
    return;
  }

  // 上层已保证，非空
  auto xml_locations = xml_veh_points->Parent()->Parent()->FirstChildElement("locations");

  for (auto&& json_veh_point : json_veh_points) {
    int location = SerializeLocation(json_veh_point, veh_point_type, xml_locations);

    TiXmlElement* one = new TiXmlElement(veh_point_type.c_str());
    SetXmlAttribute(one, "id", json_veh_point.get("id", 0).asInt());
    SetXmlAttribute(one, "location", location);
    if (veh_point_type == kVehInputString) {
      SetXmlAttribute(one, "composition", json_veh_point.get("vehicleCompositionId", 0).asInt());
      SetXmlAttribute(one, "start_v", json_veh_point.get("initialSpeed", 0.0).asDouble());
      SetXmlAttribute(one, "max_v", json_veh_point.get("maxSpeed", 0.0).asDouble());
      SetXmlAttribute(one, "timeHeadway", json_veh_point.get("generationInterval", 0).asInt());
      SetXmlAttribute(one, "duration", json_veh_point.get("lifePeriod", 0).asInt());
      SetXmlAttribute(one, "number", json_veh_point.get("generationCarsNum", 0).asInt());
      SetXmlAttribute(one, "cover", 100);
    }
    CPoint3D xyz(json_veh_point["x"].asDouble(), json_veh_point["y"].asDouble(), json_veh_point["z"].asDouble());
    SetXmlAttribute(one, "xyz", xyz.ToString().c_str());

    xml_veh_points->LinkEndChild(one);
  }
}

int CTrafficFlow::SerializeLocation(const Json::Value& point, const std::string& point_type,
                                    TiXmlElement* xml_locations) {
  int max_id = GetMaxLocationId(xml_locations);
  // calc pos
  std::string pos;
  pos += std::to_string(point.get("lon", 0.0).asDouble());
  pos += ",";
  pos += std::to_string(point.get("lat", 0.0).asDouble());

  TiXmlElement* one = new TiXmlElement("location");
  SetXmlAttribute(one, "id", ++max_id);
  SetXmlAttribute(one, "pos", pos);
  SetXmlAttribute(one, "info", point_type);
  xml_locations->LinkEndChild(one);
  return max_id;
}

void CTrafficFlow::SerializeRouteGroup(const Json::Value& json_route_groups, TiXmlElement* xml_traffic_flow) {
  TiXmlElement* xml_locations = xml_traffic_flow->Parent()->FirstChildElement("locations");
  if (!xml_locations) {
    xml_locations = xml_traffic_flow->Parent()->InsertEndChild(TiXmlElement("locations"))->ToElement();
  }

  if (!json_route_groups.isArray()) {
    return;
  }

  TiXmlElement* xml_route_groups = xml_traffic_flow->FirstChildElement("routeGroups");
  if (!xml_route_groups) {
    xml_route_groups = xml_traffic_flow->InsertEndChild(TiXmlElement("routeGroups"))->ToElement();
  }
  for (auto&& json_route_group : json_route_groups) {
    TiXmlElement* one = new TiXmlElement("routeGroup");
    SetXmlAttribute(one, "id", json_route_group.get("id", 0).asInt());
    auto start = json_route_group.get("startRoute", Json::Value(Json::arrayValue));
    if (start.isArray() && start.size() == 1) {
      int location = SerializeLocation(start[0], kVehRouteString, xml_locations);
      SetXmlAttribute(one, "start", location);
    }
    auto dests = json_route_group.get("destRoute", Json::Value(Json::arrayValue));
    for (auto i = 0; i < dests.size(); i++) {
      int percentage = dests[i].get("trafficFlow", 0).asInt();
      int location = SerializeLocation(dests[i], kVehRouteString, xml_locations);
      SetXmlAttribute(one, (std::string("mid") + std::to_string(i + 1)).c_str(), 0);
      SetXmlAttribute(one, (std::string("end") + std::to_string(i + 1)).c_str(), location);
      SetXmlAttribute(one, (std::string("percentage") + std::to_string(i + 1)).c_str(), percentage);
    }
    xml_route_groups->LinkEndChild(one);
  }
}

void CTrafficFlow::SerializeDefaultVehType(TiXmlElement* xml_traffic_flow) {
  if (!xml_traffic_flow) {
    return;
  }
  TiXmlElement* xml_veh_types = xml_traffic_flow->FirstChildElement("vehicleType");
  if (!xml_veh_types) {
    xml_veh_types = xml_traffic_flow->InsertEndChild(TiXmlElement("vehicleType"))->ToElement();
  }

  for (auto iter = veh_type_map_.begin(); iter != veh_type_map_.end(); iter++) {
    if (iter->second.ignore) {
      continue;
    }
    TiXmlElement* one = new TiXmlElement("vehType");
    SetXmlAttribute(one, "id", iter->second.id);
    SetXmlAttribute(one, "type", iter->second.type);
    SetXmlAttribute(one, "length", iter->second.length);
    SetXmlAttribute(one, "width", iter->second.width);
    SetXmlAttribute(one, "height", iter->second.height);
    SetXmlAttribute(one, "behavior", iter->second.behavior);
    xml_veh_types->LinkEndChild(one);
  }
}

void CTrafficFlow::SerializeDefaultBehavior(TiXmlElement* xml_traffic_flow) {
  if (!xml_traffic_flow) {
    return;
  }

  TiXmlDocument xml;
  xml.Parse(R"(
    <behaviors>
      <behavior id="2" type="Arterial" AX="2" BX_Add="2" BX_Mult="3"/>
    </behaviors>
  )");
  TiXmlElement* xml_behaviors = xml_traffic_flow->FirstChildElement("behaviors");
  if (!xml_behaviors || !xml_behaviors->FirstChildElement("behavior")) {
    xml_traffic_flow->InsertEndChild(*(xml.FirstChild()));
  }
}

void CTrafficFlow::SerializePedestrianSim(const Json::Value& json_pedestrian_sim, TiXmlElement* xml_traffic_flow) {
  if (!xml_traffic_flow) {
    return;
  }

  Json::Value root = json_pedestrian_sim;
  for (auto& comp : root["pedComposition"]) {
    for (Json::ArrayIndex i = 0; i < comp["types"].size(); i++) {
      auto& json = comp["types"][i];
      std::string suffix = std::to_string(i + 1);
      comp[std::string("type") + suffix] = json["id"];
      comp[std::string("percentage") + suffix] = json["percentage"];
      comp[std::string("bTree") + suffix] = json["bTree"];
    }
    comp.removeMember("types");
  }

  FIND_OR_CREATE_NODE(xml_traffic_flow->Parent(), xml_locations, STR(locations));
  for (auto& input : root["pedestrianInput"]) {
    int location = SerializeLocation(input["location"], "pedInput", xml_locations);
    input.removeMember("location");
    input["location"] = location;
  }

  auto to_xml_element = [&](const char* ele_key, const char* child_key) {
    FIND_OR_CREATE_NODE(xml_traffic_flow, node, ele_key);
    if (!root.isObject()) {
      return;
    }
    for (auto& record : root.get(ele_key, Json::Value(Json::arrayValue))) {
      TiXmlElement* one = new TiXmlElement(child_key);
      for (auto iter = record.begin(); iter != record.end(); iter++) {
        std::string key = iter.key().asString();
        SetXmlAttribute(one, key.c_str(), StringifyJsonValue(record[key]));
      }
      node->LinkEndChild(one);
    }
  };

  to_xml_element("pedBehaviors", "pedBehavior");
  to_xml_element("pedestrianType", "pedType");
  to_xml_element("pedComposition", "pedComp");
  to_xml_element("pedestrianInput", "pedInput");

  FIND_OR_CREATE_NODE(xml_traffic_flow, xml_ped_front, STR(pedSimulatoinOfFront));
  TiXmlText text(JsonToString(root["pedSimulatoinOfFront"]));
  text.SetCDATA(true);
  xml_ped_front->InsertEndChild(text);

  // /file/path/*_traffic.xml -> /file/path/*_bree_{id}.xml
  FIND_OR_CREATE_NODE(xml_traffic_flow, xml_ped_btrees, STR(pedBehaviorTrees));
  std::string prefix = file_path_.substr(0, file_path_.find_last_of("_"));
  for (auto& route : root["pedRoute"]) {
    std::string file = prefix + "_btree_" + route.get("id", 0).asString() + ".xml";
    BehaviorTree{}.Save(route).SaveFile(file.c_str());

    TiXmlElement btree("pedBehaviorTree");
    btree.InsertEndChild(TiXmlText(boost::filesystem::path(file).filename().string().c_str()));
    xml_ped_btrees->InsertEndChild(btree);
  }
}

int CTrafficFlow::GetMaxLocationId(TiXmlElement* xml_locations) {
  int result = 0;
  FOREACH_XML(xml_locations, "location", xml_location) {
    int id = GetXmlAttribute(xml_location, "id", 0u);
    result = std::max(result, id);
  }
  return result;
}
