/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "common/xml_parser/xosc_replay/tpl_macros.h"
#include "common/xml_parser/xosc_replay/type_adapter.h"

TX_NAMESPACE_OPEN(SIM)
TX_NAMESPACE_OPEN(OSC)

struct xmlFile {
  /*
  <File filepath=""/>
  */
  std::string filepath;
  friend std::ostream& operator<<(std::ostream& os, const xmlFile& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<File " << TX_VARS_NAME(filepath, std::quoted(v.filepath)) << "/>";
    return os;
  }
};

struct xmlProperty {
  /*
  <Property name="control" value="external"/>
  */
  std::string name;
  std::string value;
  friend std::ostream& operator<<(std::ostream& os, const xmlProperty& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Property " << TX_VARS_NAME(name, std::quoted(v.name))
       << TX_VARS_NAME(value, std::quoted(v.value)) << "/>";
    return os;
  }
};

struct xmlProperties {
  /*
  <Properties>
    <Property name="control" value="external"/>
    <File filepath=""/>
  </Properties>
  */
  std::vector<xmlProperty> Property;
  xmlFile File;
  friend std::ostream& operator<<(std::ostream& os, const xmlProperties& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Properties>" << std::endl;
    for (auto& p : v.Property) {
      os << p << std::endl;
    }
    os << v.File << std::endl << "</Properties>";
    return os;
  }
};

struct xmlAxle {
  /*
  <RearAxle maxSteering="0" wheelDiameter="0.63994" trackWidth="1.608" positionX="0"
  positionZ="0.31997"/>
  <FrontAxle maxSteering="0.46" wheelDiameter="0.63994" trackWidth="1.608" positionX="2.536"
  positionZ="0.31997"/>
  */
  double maxSteering = 0.0;
  double wheelDiameter = 0.0;
  double trackWidth = 0.0;
  double positionX = 0.0;
  double positionZ = 0.0;
  friend std::ostream& operator<<(std::ostream& os, const xmlAxle& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Axle " << TX_VARS_NAME(maxSteering, v.maxSteering)
       << TX_VARS_NAME(wheelDiameter, v.wheelDiameter) << TX_VARS_NAME(trackWidth, v.trackWidth)
       << TX_VARS_NAME(positionX, v.positionX) << TX_VARS_NAME(positionZ, v.positionZ) << "/>";
    return os;
  }
};

struct xmlAxles {
  /*
  <Axles>
    <FrontAxle maxSteering="0.46" wheelDiameter="0.63994" trackWidth="1.608" positionX="2.536"
  positionZ="0.31997"/> <RearAxle maxSteering="0" wheelDiameter="0.63994" trackWidth="1.608"
  positionX="0" positionZ="0.31997"/>
  </Axles>
  */
  xmlAxle FrontAxle;
  xmlAxle RearAxle;
  friend std::ostream& operator<<(std::ostream& os, const xmlAxles& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Axles> " << std::endl
       << v.FrontAxle << std::endl
       << v.RearAxle << std::endl
       << "</Axles>";
    return os;
  }
};

struct xmlPerformance {
  /*
  <Performance maxSpeed="69.444" maxDeceleration="10" maxAcceleration="100"/>
  */
  double maxSpeed = 20.0;
  double maxDeceleration = 10.0;
  double maxAcceleration = 12.0;
  friend std::ostream& operator<<(std::ostream& os, const xmlPerformance& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Performance " << TX_VARS_NAME(maxSpeed, v.maxSpeed)
       << TX_VARS_NAME(maxDeceleration, v.maxDeceleration) << TX_VARS_NAME(maxAcceleration, v.maxAcceleration) << "/>";
    return os;
  }
};

struct xmlCenter {
  /*
  <Center x="121.17487722345179" y="31.270958542979315" z="0.0"/>
  */
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  friend std::ostream& operator<<(std::ostream& os, const xmlCenter& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Center " << TX_VARS_NAME(x, v.x) << TX_VARS_NAME(y, v.y) << TX_VARS_NAME(z, v.z)
       << "/>";
    return os;
  }
};

struct xmlDimensions {
  /*
  <Dimensions width="1.0" length="2.0" height="1.0"/>
  */
  double width = 0.0;
  double length = 0.0;
  double height = 0.0;
  friend std::ostream& operator<<(std::ostream& os, const xmlDimensions& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Dimensions " << TX_VARS_NAME(width, v.width) << TX_VARS_NAME(length, v.length)
       << TX_VARS_NAME(height, v.height) << "/>";
    return os;
  }
};

struct xmlBoundingBox {
  /*
  <BoundingBox>
    <Center x="121.17487722345179" y="31.270958542979315" z="0.0"/>
    <Dimensions width="1.0" length="2.0" height="1.0"/>
  </BoundingBox>
  */
  xmlCenter Center;
  xmlDimensions Dimensions;
  friend std::ostream& operator<<(std::ostream& os, const xmlBoundingBox& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<BoundingBox>" << std::endl
       << v.Center << std::endl
       << v.Dimensions << std::endl
       << "</BoundingBox>";
    return os;
  }
};

struct xmlVehicle {
  /*
  <Vehicle name="car_test" vehicleCategory="car">
    <BoundingBox>
      <Center x="121.17487722345179" y="31.270958542979315" z="0.0"/>
      <Dimensions width="1.0" length="2.0" height="1.0"/>
    </BoundingBox>
    <Performance maxSpeed="69.444" maxDeceleration="10" maxAcceleration="100"/>
    <Axles>
      <FrontAxle maxSteering="0.46" wheelDiameter="0.63994" trackWidth="1.608" positionX="2.536"
  positionZ="0.31997"/> <RearAxle maxSteering="0" wheelDiameter="0.63994" trackWidth="1.608"
  positionX="0" positionZ="0.31997"/>
    </Axles>
    <Properties>
      <Property name="control" value="external"/>
      <File filepath=""/>
    </Properties>
  </Vehicle>
  */
  std::string name;
  std::string vehicleCategory;
  xmlBoundingBox BoundingBox;
  xmlPerformance Performance;
  xmlAxles Axles;
  xmlProperties Properties;
  friend std::ostream& operator<<(std::ostream& os, const xmlVehicle& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Vehicle " << TX_VARS_NAME(name, std::quoted(v.name))
       << TX_VARS_NAME(vehicleCategory, std::quoted(v.vehicleCategory)) << ">" << std::endl
       << v.BoundingBox << std::endl
       << v.Performance << std::endl
       << v.Axles << std::endl
       << v.Properties << std::endl
       << "</Vehicle>";
    return os;
  }
  bool empty() const { return name.empty() || vehicleCategory.empty(); }
};

struct xmlPedestrian {
  /*
    <Pedestrian mass="65" model="model" name="human" pedestrianCategory="pedestrian">
      <BoundingBox>
        <Center x="0" y="0" z="0"/>
        <Dimensions width="0.55" length="0.55" height="1.76"/>
      </BoundingBox>
      <Properties>
        <Property name="control" value="internal"/>
      </Properties>
    </Pedestrian>
  */
  std::string name;
  std::string pedestrianCategory;
  xmlBoundingBox BoundingBox;
  xmlProperties Properties;
  friend std::ostream& operator<<(std::ostream& os, const xmlPedestrian& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Pedestrian" << TX_VARS_NAME(name, std::quoted(v.name))
       << TX_VARS_NAME(pedestrianCategory, std::quoted(v.pedestrianCategory)) << ">" << std::endl
       << v.BoundingBox << std::endl
       << v.Properties << std::endl
       << "</Pedestrian>";
    return os;
  }
  bool empty() const { return name.empty() || pedestrianCategory.empty(); }
};

struct xmlMiscObject {
  double mass = 0.0;
  std::string miscObjectCategory;
  std::string name;
  xmlBoundingBox BoundingBox;
  xmlProperties Properties;
  friend std::ostream& operator<<(std::ostream& os, const xmlMiscObject& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<MiscObject" << TX_VARS_NAME(mass, v.mass)
       << TX_VARS_NAME(miscObjectCategory, std::quoted(v.miscObjectCategory)) << TX_VARS_NAME(name, std::quoted(v.name))
       << ">" << std::endl
       << v.BoundingBox << std::endl
       << v.Properties << std::endl
       << "</MiscObject>";
    return os;
  }
  bool empty() const { return name.empty() || miscObjectCategory.empty(); }
};

struct xmlCatalogReference {
  std::string catalogName;
  std::string entryName;
  friend std::ostream& operator<<(std::ostream& os, const xmlCatalogReference& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<CatalogReference " << TX_VARS_NAME(catalogName, std::quoted(v.catalogName))
       << TX_VARS_NAME(entryName, std::quoted(v.entryName)) << ">" << std::endl
       << "</CatalogReference";
    return os;
  }
  bool empty() const { return catalogName.empty() || entryName.empty(); }
  void clear() { catalogName = "", entryName = ""; }
};
struct xmlScenarioObject {
  /*
  <ScenarioObject name="egocar">
    <Vehicle name="car_test" vehicleCategory="car">
      <BoundingBox>
        <Center x="121.17487722345179" y="31.270958542979315" z="0.0"/>
        <Dimensions width="1.0" length="2.0" height="1.0"/>
      </BoundingBox>
      <Performance maxSpeed="69.444" maxDeceleration="10" maxAcceleration="100"/>
      <Axles>
        <FrontAxle maxSteering="0.46" wheelDiameter="0.63994" trackWidth="1.608" positionX="2.536"
  positionZ="0.31997"/> <RearAxle maxSteering="0" wheelDiameter="0.63994" trackWidth="1.608"
  positionX="0" positionZ="0.31997"/>
      </Axles>
      <Properties>
        <Property name="control" value="external"/>
        <File filepath=""/>
      </Properties>
    </Vehicle>
  </ScenarioObject>
  */
  std::string name;
  xmlVehicle Vehicle;
  xmlPedestrian Pedestrian;
  xmlMiscObject MiscObject;
  xmlCatalogReference CatalogReference;
  friend std::ostream& operator<<(std::ostream& os, const xmlScenarioObject& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<ScenarioObject " << TX_VARS_NAME(name, std::quoted(v.name)) << ">" << std::endl;
    if (!v.Vehicle.name.empty()) {
      os << v.Vehicle << std::endl;
    }
    if (!v.Pedestrian.name.empty()) {
      os << v.Pedestrian << std::endl;
    }
    if (!v.MiscObject.name.empty()) {
      os << v.MiscObject << std::endl;
    }
    if (!v.CatalogReference.entryName.empty()) {
      os << v.CatalogReference << std::endl;
    }
    os << "</ScenarioObject>";
    return os;
  }
  int64_t id = 0;
  int32_t age = 0;  // ms
  std::string category;
  osc_type_t type = {-1, -1};
  void Decode() {
    auto calc_model_id = [](const auto& object) -> int {
      for (auto& p : object.Properties.Property) {
        if (p.name == "model_id") {
          return atoi(p.value.c_str());
        }
      }
      return -1;
    };
    // calc type and id by category
    if (!Vehicle.vehicleCategory.empty()) {
      category = Vehicle.vehicleCategory + "|" + Vehicle.name;
      type = {ObjectType::kVehicle, calc_model_id(Vehicle)};
    } else if (!Pedestrian.pedestrianCategory.empty()) {
      category = Pedestrian.pedestrianCategory + "|" + Pedestrian.name;
      type = {ObjectType::kDynamicObstacle, calc_model_id(Pedestrian)};
    } else if (!MiscObject.miscObjectCategory.empty()) {
      category = MiscObject.miscObjectCategory + "|" + MiscObject.name;
      type = {ObjectType::kStaticObstacle, calc_model_id(MiscObject)};
    }
    auto iter = kCategory2Type.find(category);
    if (iter == kCategory2Type.end()) {
      iter = kCategory2Type.find(category.substr(0, category.find("|")));
    }
    if ((type.first == -1 || type.second == -1) && iter != kCategory2Type.end()) {
      type = iter->second;
    }
    id = Name2Id(type.first, name);
  }
  void Encode() {
    // calc category and name by type
    // auto iter = kType2Category.find(type);
    // if (iter != kType2Category.end()) {
    //   category = iter->second;
    // }
    // auto pos = category.find("|");
    // if (type.first == ObjectType::kVehicle) {
    //   Vehicle.vehicleCategory = category.substr(0, pos);
    //   Vehicle.name = category.substr(pos + 1);
    // } else if (type.first == ObjectType::kDynamicObstacle) {
    //   Pedestrian.pedestrianCategory = category.substr(0, pos);
    //   Pedestrian.name = category.substr(pos + 1);
    // } else if (type.first == ObjectType::kStaticObstacle) {
    //   MiscObject.miscObjectCategory = category.substr(0, pos);
    //   MiscObject.name = category.substr(pos + 1);
    // }
    name = Id2Name(type.first, id);
  }
  xmlDimensions& Dimensions() {
    if (type.first == ObjectType::kDynamicObstacle) {
      return Pedestrian.BoundingBox.Dimensions;
    } else if (type.first == ObjectType::kStaticObstacle) {
      return MiscObject.BoundingBox.Dimensions;
    } else {
      return Vehicle.BoundingBox.Dimensions;
    }
  }
};
using xmlEntities = std::vector<xmlScenarioObject>;

struct xmlLogicFile {
  /*
  <LogicFile filepath="test/map/d2d_20190726.xodr"/>
  */
  std::string filepath;
  friend std::ostream& operator<<(std::ostream& os, const xmlLogicFile& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<LogicFile " << TX_VARS_NAME(filepath, std::quoted(v.filepath)) << "/>";
    return os;
  }
};

/*
<TrafficSignals>
  <TrafficSignalController delay="0" name="23187226" reference="-1">
    <Phase duration="16.042614936828613" name="stop">
      <TrafficSignalState state="true;false;false" trafficSignalId="23187226"/>
    </Phase>
    <Phase duration="8.616866827011108" name="go">
      <TrafficSignalState state="false;false;true" trafficSignalId="23187226"/>
    </Phase>
  </TrafficSignalController>
  <TrafficSignalController delay="0" name="23187225" reference="-1">
    <Phase duration="16.042614936828613" name="stop">
      <TrafficSignalState state="true;false;false" trafficSignalId="23187225"/>
    </Phase>
    <Phase duration="8.616866827011108" name="go">
      <TrafficSignalState state="false;false;true" trafficSignalId="23187225"/>
    </Phase>
  </TrafficSignalController>
</TrafficSignals>
*/
struct xmlTrafficSignalState {
  std::string trafficSignalId;
  std::string state;
  double x = 0.0;
  double y = 0.0;
  int color = 0;
  friend std::ostream& operator<<(std::ostream& os, const xmlTrafficSignalState& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<TrafficSignalState " << TX_VARS_NAME(state, std::quoted(v.state))
       << TX_VARS_NAME(trafficSignalId, std::quoted(v.trafficSignalId)) << "/>";
    return os;
  }
};

struct xmlPhase {
  std::string name;
  double duration;
  double time = 0.0;
  std::vector<xmlTrafficSignalState> TrafficSignalState;
  friend std::ostream& operator<<(std::ostream& os, const xmlPhase& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Phase " << TX_VARS_NAME(name, std::quoted(v.name))
       << TX_VARS_NAME(duration, v.duration) << ">" << std::endl;
    for (auto&& s : v.TrafficSignalState) {
      os << s << std::endl;
    }
    os << "</Phase>";
    return os;
  }
};

struct xmlTrafficSignalController {
  std::string name;
  double delay;
  std::string reference;
  std::vector<xmlPhase> Phase;
  friend std::ostream& operator<<(std::ostream& os, const xmlTrafficSignalController& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<TrafficSignalController " << TX_VARS_NAME(name, std::quoted(v.name))
       << TX_VARS_NAME(delay, v.delay) << TX_VARS_NAME(reference, std::quoted(v.reference)) << ">" << std::endl;
    for (auto&& p : v.Phase) {
      os << p << std::endl;
    }
    os << "</TrafficSignalController>";
    return os;
  }
};

struct xmlTrafficSignals {
  std::vector<xmlTrafficSignalController> TrafficSignalController;
  friend std::ostream& operator<<(std::ostream& os, const xmlTrafficSignals& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<TrafficSignals>" << std::endl;
    for (auto&& c : v.TrafficSignalController) {
      os << c << std::endl;
    }
    os << "</TrafficSignals>";
    return os;
  }
};

struct xmlRoadNetwork {
  /*
  <RoadNetwork>
    <LogicFile filepath="test/map/d2d_20190726.xodr"/>
  </RoadNetwork>
  */
  xmlLogicFile LogicFile;
  xmlTrafficSignals TrafficSignals;
  friend std::ostream& operator<<(std::ostream& os, const xmlRoadNetwork& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<RoadNetwork>" << std::endl
       << v.LogicFile << std::endl
       << v.TrafficSignals << std::endl;
    os << "</RoadNetwork>";
    return os;
  }
};

struct xmlFileHeader {
  /*
  <FileHeader
  description="simulation-test-1251316161/modules/144115205301725114/12334-5fd04d0ee0103c2314c7d695/sim_planning.tar.gz"
  date="2020-12-10T02:28:13" author="" revMajor="1" revMinor="0"/>
  */
  std::string description;
  std::string date;
  std::string author;
  std::string revMajor;
  std::string revMinor;
  friend std::ostream& operator<<(std::ostream& os, const xmlFileHeader& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<FileHeader " << TX_VARS_NAME(description, std::quoted(v.description))
       << TX_VARS_NAME(date, std::quoted(v.date)) << TX_VARS_NAME(author, std::quoted(v.author))
       << TX_VARS_NAME(revMajor, std::quoted(v.revMajor)) << TX_VARS_NAME(revMinor, std::quoted(v.revMinor)) << "/>";
    return os;
  }
};

struct xmlTrafficSignalStateAction {
  /*
  <TrafficSignalStateAction name = "trafficLight_1011" state = "0" />
  */
  std::string name;
  std::string state;
  friend std::ostream& operator<<(std::ostream& os, const xmlTrafficSignalStateAction& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<TrafficSignalStateAction " << TX_VARS_NAME(name, std::quoted(v.name))
       << TX_VARS_NAME(state, std::quoted(v.state)) << "/>";
    return os;
  }
};

struct xmlTrafficSignalAction {
  xmlTrafficSignalStateAction TrafficSignalStateAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlTrafficSignalAction& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<TrafficSignalAction>" << std::endl
       << v.TrafficSignalStateAction << std::endl
       << "</TrafficSignalAction>";
    return os;
  }
};

struct xmlInfrastructureAction {
  xmlTrafficSignalAction TrafficSignalAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlInfrastructureAction& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<InfrastructureAction>" << std::endl
       << v.TrafficSignalAction << std::endl
       << "</InfrastructureAction>";
    return os;
  }
};

struct xmlGlobalAction {
  xmlInfrastructureAction InfrastructureAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlGlobalAction& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<GlobalAction>" << std::endl
       << v.InfrastructureAction << std::endl
       << "</GlobalAction>";
    return os;
  }
};

struct xmlWorldPosition {
  double h = 0.0;
  double p = 0.0;
  double r = 0.0;
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  double v = 0.0;
  double acc = 0.0;
  /*
  <WorldPosition h="-1.330236836202213" p="0.0" r="0.0" x="121.17487722345179"
  y="31.270958542979315" z="0.0"/>
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlWorldPosition& v) TX_NOEXCEPT {
    os << "<WorldPosition " << TX_VARS_NAME(h, v.h) << TX_VARS_NAME(p, v.p) << TX_VARS_NAME(r, v.r)
       << TX_VARS_NAME(x, v.x) << TX_VARS_NAME(y, v.y) << TX_VARS_NAME(z, v.z) << ">";
    return os;
  }
  bool empty() const {
    return std::abs(x - 0.0) < std::numeric_limits<double>::epsilon() &&
           std::abs(y - 0.0) < std::numeric_limits<double>::epsilon();
  }
};

struct xmlPosition {
  xmlWorldPosition WorldPosition;
  friend std::ostream& operator<<(std::ostream& os, const xmlPosition& v) TX_NOEXCEPT {
    os << "<Position>" << std::endl << v.WorldPosition << std::endl << "</Position>";
    return os;
  }
  bool empty() const { return WorldPosition.empty(); }
};

struct xmlTeleportAction {
  xmlPosition Position;
  friend std::ostream& operator<<(std::ostream& os, const xmlTeleportAction& v) TX_NOEXCEPT {
    os << "<TeleportAction>" << std::endl << v.Position << std::endl << "</TeleportAction>";
    return os;
  }
  bool empty() const { return Position.empty(); }
};

struct xmlPrivateAction_1 {
  xmlTeleportAction TeleportAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlPrivateAction_1& v) TX_NOEXCEPT {
    os << "<PrivateAction>" << std::endl << v.TeleportAction << std::endl << "</PrivateAction>";
    return os;
  }
};

struct xmlPrivate {
  /*
  <Private entityRef="egocar">
  */
  std::string entityRef;
  xmlPrivateAction_1 PrivateAction;

  friend std::ostream& operator<<(std::ostream& os, const xmlPrivate& v) TX_NOEXCEPT {
    os << "<Private " << TX_VARS_NAME(entityRef, std::quoted(v.entityRef)) << ">" << std::endl
       << v.PrivateAction << std::endl
       << "</Private>";
    return os;
  }
};

struct xmlActions {
  std::vector<xmlGlobalAction> GlobalAction;
  std::vector<xmlPrivate> Private;
  friend std::ostream& operator<<(std::ostream& os, const xmlActions& v) TX_NOEXCEPT {
    os << "<Actions>" << std::endl;
    for (const auto& refGlobalAction : v.GlobalAction) {
      os << refGlobalAction << std::endl;
    }
    for (const auto& refPrivate : v.Private) {
      os << refPrivate << std::endl;
    }
    os << "</Actions>";
    return os;
  }
};

struct xmlInit {
  // std::vector<xmlGlobalAction>  GlobalActions;
  xmlActions Actions;
  friend std::ostream& operator<<(std::ostream& os, const xmlInit& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Init>" << std::endl << v.Actions << std::endl << "</Init>";
    return os;
  }
};

struct xmlParameterDeclaration {
  /*
<ParameterDeclaration parameterType="string" name="$owner" value="A2"/>
  */
  std::string parameterType;
  std::string name;
  std::string value;
  xmlParameterDeclaration() {}
  xmlParameterDeclaration(const std::string& t, const std::string& n, const std::string& v)
      : parameterType(t), name(n), value(v) {}
  friend std::ostream& operator<<(std::ostream& os, const xmlParameterDeclaration& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<ParameterDeclaration " << TX_VARS_NAME(parameterType, std::quoted(v.parameterType))
       << TX_VARS_NAME(name, std::quoted(v.name)) << TX_VARS_NAME(value, std::quoted(v.value)) << "/>";
    return os;
  }
};

struct xmlParameterDeclarations {
  std::vector<xmlParameterDeclaration> ParameterDeclaration;
  friend std::ostream& operator<<(std::ostream& os, const xmlParameterDeclarations& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<ParameterDeclarations>" << std::endl;
    for (auto& p : v.ParameterDeclaration) {
      os << p << std::endl;
    }
    os << "</ParameterDeclarations>";
    return os;
  }
};

struct xmlEntityRef {
  /*
  <EntityRef entityRef="egocar"/>
  */
  std::string entityRef;
  friend std::ostream& operator<<(std::ostream& os, const xmlEntityRef& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<EntityRef " << TX_VARS_NAME(entityRef, std::quoted(v.entityRef)) << "/>";
    return os;
  }
};

struct xmlActors {
  /*
  <Actors selectTriggeringEntities="false">
  */
  bool selectTriggeringEntities = false;
  xmlEntityRef EntityRef;

  /*
  <Actors selectTriggeringEntities="false">
          <EntityRef entityRef="egocar"/>
        </Actors>
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlActors& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Actors " << TX_VARS_NAME(selectTriggeringEntities, v.selectTriggeringEntities) << ">"
       << std::endl
       << v.EntityRef << std::endl
       << "</Actors>";
    return os;
  }
};

struct xmlVertex {
  xmlPosition Position;
  std::string time;
  /*
  <Vertex time="0.0">
      <Position>
      <WorldPosition h="-1.330236836202213" x="121.17487722345179" y="31.270958542979315" z="0.0"/>
      </Position>
  </Vertex>
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlVertex& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Vertex " << TX_VARS_NAME(time, std::quoted(v.time)) << ">" << std::endl
       << v.Position << std::endl
       << "</Vertex>";
    return os;
  }
};

struct xmlPolyline {
  /*
  <Vertex time="0.0">
  */
  std::vector<xmlVertex> Vertexs;
  friend std::ostream& operator<<(std::ostream& os, const xmlPolyline& v) TX_NOEXCEPT {
    os << "<Polyline>" << std::endl;
    for (const auto& refVertex : v.Vertexs) {
      os << refVertex << std::endl;
    }
    os << "</Polyline>";
    return os;
  }
};

struct xmlShape {
  xmlPolyline Polyline;
  friend std::ostream& operator<<(std::ostream& os, const xmlShape& v) TX_NOEXCEPT {
    os << "<Shape>" << std::endl << v.Polyline << std::endl << "</Shape>";
    return os;
  }
};

struct xmlTrajectory {
  /*
  <Trajectory name="Trajectory1" closed="false">
  */
  std::string name;
  bool closed = false;
  xmlShape Shape;
  friend std::ostream& operator<<(std::ostream& os, const xmlTrajectory& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Trajectory " << TX_VARS_NAME(name, std::quoted(v.name))
       << TX_VARS_NAME(closed, v.closed) << ">" << std::endl
       << v.Shape << std::endl
       << "</Trajectory>";
    return os;
  }
};

struct xmlNone {
  /*
  <None/>
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlNone& v) TX_NOEXCEPT {
    os << "<None/>";
    return os;
  }
};

struct xmlTimeReference {
  /*
  <TimeReference>
      <None/>
  </TimeReference>
  */
  xmlNone None;
  friend std::ostream& operator<<(std::ostream& os, const xmlTimeReference& v) TX_NOEXCEPT {
    os << "<TimeReference>" << v.None << std::endl << "</TimeReference>";
    return os;
  }
};

struct xmlTrajectoryFollowingMode {
  std::string followingMode = "follow";
  /*
  <TrajectoryFollowingMode followingMode="follow"/>
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlTrajectoryFollowingMode& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<TrajectoryFollowingMode " << TX_VARS_NAME(followingMode, std::quoted(v.followingMode))
       << "/>";
    return os;
  }
};

struct xmlFollowTrajectoryAction {
  xmlTrajectory Trajectory;
  xmlTimeReference TimeReference;
  xmlTrajectoryFollowingMode TrajectoryFollowingMode;
  friend std::ostream& operator<<(std::ostream& os, const xmlFollowTrajectoryAction& v) TX_NOEXCEPT {
    os << "<FollowTrajectoryAction>" << std::endl
       << v.Trajectory << std::endl
       << v.TimeReference << std::endl
       << v.TrajectoryFollowingMode << std::endl
       << "</FollowTrajectoryAction>";
    return os;
  }
  bool empty() const { return Trajectory.Shape.Polyline.Vertexs.empty(); }
};

struct xmlAcquirePositionAction {
  xmlPosition Position;
  friend std::ostream& operator<<(std::ostream& os, const xmlAcquirePositionAction& v) TX_NOEXCEPT {
    os << "<AcquirePositionAction>" << std::endl << v.Position << std::endl << "</AcquirePositionAction>";
    return os;
  }
  bool empty() const { return Position.empty(); }
};

struct xmlRoutingAction {
  xmlAcquirePositionAction AcquirePositionAction;
  xmlFollowTrajectoryAction FollowTrajectoryAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlRoutingAction& v) TX_NOEXCEPT {
    os << "<RoutingAction>" << std::endl
       << v.AcquirePositionAction << std::endl
       << v.FollowTrajectoryAction << std::endl
       << "</RoutingAction>";
    return os;
  }
  bool empty() const { return AcquirePositionAction.empty() && FollowTrajectoryAction.empty(); }
};

struct xmlAbsoluteTargetSpeed {
  double value = 0.0;
  friend std::ostream& operator<<(std::ostream& os, const xmlAbsoluteTargetSpeed& v) TX_NOEXCEPT {
    os << "<AbsoluteTargetSpeed " << TX_VARS_NAME(value, v.value) << "/>";
    return os;
  }
  bool empty() const { return std::abs(value - 0.0) < std::numeric_limits<double>::epsilon(); }
};

struct xmlSpeedActionDynamics {
  std::string dynamicsDimension = "time";
  std::string dynamicsShape = "step";
  double value = 0.0;
  friend std::ostream& operator<<(std::ostream& os, const xmlSpeedActionDynamics& v) TX_NOEXCEPT {
    os << "<SpeedActionDynamics " << TX_VARS_NAME(dynamicsDimension, std::quoted(v.dynamicsDimension))
       << TX_VARS_NAME(dynamicsShape, std::quoted(v.dynamicsShape)) << TX_VARS_NAME(value, v.value) << "/>";
    return os;
  }
  bool empty() const { return dynamicsShape.empty() || dynamicsDimension.empty(); }
};

struct xmlSpeedActionTarget {
  xmlAbsoluteTargetSpeed AbsoluteTargetSpeed;
  friend std::ostream& operator<<(std::ostream& os, const xmlSpeedActionTarget& v) TX_NOEXCEPT {
    os << "<SpeedActionTarget>" << std::endl << v.AbsoluteTargetSpeed << std::endl << "</SpeedActionTarget>";
    return os;
  }
  bool empty() const { return AbsoluteTargetSpeed.empty(); }
};

struct xmlSpeedAction {
  xmlSpeedActionDynamics SpeedActionDynamics;
  xmlSpeedActionTarget SpeedActionTarget;
  friend std::ostream& operator<<(std::ostream& os, const xmlSpeedAction& v) TX_NOEXCEPT {
    os << "<SpeedAction>" << std::endl
       << v.SpeedActionDynamics << std::endl
       << v.SpeedActionTarget << std::endl
       << "</SpeedAction>";
    return os;
  }
  bool empty() const { return SpeedActionDynamics.empty() || SpeedActionTarget.empty(); }
};

struct xmlLongitudinalAction {
  xmlSpeedAction SpeedAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlLongitudinalAction& v) TX_NOEXCEPT {
    os << "<LongitudinalAction>" << std::endl << v.SpeedAction << std::endl << "</LongitudinalAction>";
    return os;
  }
  bool empty() const { return SpeedAction.empty(); }
};

struct xmlPrivateAction {
  xmlRoutingAction RoutingAction;
  xmlTeleportAction TeleportAction;
  xmlLongitudinalAction LongitudinalAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlPrivateAction& v) TX_NOEXCEPT {
    os << "<PrivateAction>" << std::endl << v.RoutingAction << std::endl << "</PrivateAction>";
    return os;
  }
  bool empty() const { return RoutingAction.empty() && TeleportAction.empty() && LongitudinalAction.empty(); }
};

struct xmlAction {
  std::string name;
  /*
  <Action name="Action1">
  */
  xmlAction() {}
  explicit xmlAction(const std::string& n) : name(n) {}
  xmlPrivateAction PrivateAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlAction& v) TX_NOEXCEPT {
    os << "<Action " << TX_VARS_NAME(name, std::quoted(v.name)) << ">" << std::endl
       << v.PrivateAction << std::endl
       << "</Action>";
    return os;
  }
};

struct xmlSimulationTimeCondition {
  int sign = 1;
  double value = 0.0;
  std::string rule;
  friend std::ostream& operator<<(std::ostream& os, const xmlSimulationTimeCondition& v) TX_NOEXCEPT {
    os << "<SimulationTimeCondition " << TX_VARS_NAME(rule, std::quoted(v.rule)) << TX_VARS_NAME(value, v.value)
       << "/>";
    return os;
  }
};

struct xmlByValueCondition {
  xmlSimulationTimeCondition SimulationTimeCondition;
  friend std::ostream& operator<<(std::ostream& os, const xmlByValueCondition& v) TX_NOEXCEPT {
    os << "<ByValueCondition>" << std::endl << v.SimulationTimeCondition << std::endl << "</ByValueCondition>";
    return os;
  }
};

struct xmlCondition {
  double delay = 0.0;
  std::string name;
  std::string conditionEdge = "none";
  xmlByValueCondition ByValueCondition;
  friend std::ostream& operator<<(std::ostream& os, const xmlCondition& v) TX_NOEXCEPT {
    os << "<Condition " << TX_VARS_NAME(name, std::quoted(v.name)) << TX_VARS_NAME(delay, v.delay) << std::endl
       << v.ByValueCondition << std::endl
       << "</Condition>";
    return os;
  }
};

struct xmlConditionGroup {
  std::vector<xmlCondition> Condition;
  friend std::ostream& operator<<(std::ostream& os, const xmlConditionGroup& v) TX_NOEXCEPT {
    os << "<ConditionGroup>" << std::endl;
    for (auto& c : v.Condition) {
      os << c << std::endl;
    }
    os << "</ConditionGroup>";
    return os;
  }
};

struct xmlStartTrigger {
  /*
  <StartTrigger>
    <ConditionGroup>
      <Condition conditionEdge="none" delay="0" name="仿真时间">
        <ByValueCondition>
          <SimulationTimeCondition rule="greaterThan" value="30.39871573448181"/>
        </ByValueCondition>
      </Condition>
    </ConditionGroup>
  </StartTrigger>
  */
  std::vector<xmlConditionGroup> ConditionGroup;
  friend std::ostream& operator<<(std::ostream& os, const xmlStartTrigger& v) TX_NOEXCEPT {
    os << "<StartTrigger>" << std::endl;
    for (auto& cg : v.ConditionGroup) {
      os << cg << std::endl;
    }
    os << "</StartTrigger>";
    return os;
  }
};

struct xmlEvent {
  /*
  <Event name="Event1" priority="overwrite">
  */
  uint32_t maximumExecutionCount = 1;
  std::string name;
  std::string priority = "overwrite";
  std::vector<xmlAction> Action;
  xmlStartTrigger StartTrigger;
  friend std::ostream& operator<<(std::ostream& os, const xmlEvent& v) TX_NOEXCEPT {
    os << "<Event " << TX_VARS_NAME(name, std::quoted(v.name)) << TX_VARS_NAME(priority, std::quoted(v.priority)) << ">"
       << std::endl;
    for (auto& a : v.Action) {
      os << a << std::endl;
    }
    os << v.StartTrigger << std::endl << "</Event>";
    return os;
  }
};

struct xmlManeuver {
  std::string name;
  std::vector<xmlEvent> Event;
  friend std::ostream& operator<<(std::ostream& os, const xmlManeuver& v) TX_NOEXCEPT {
    os << "<Maneuver " << TX_VARS_NAME(name, std::quoted(v.name)) << ">" << std::endl;
    for (auto& e : v.Event) {
      os << e << std::endl;
    }
    os << "</Maneuver>";
    return os;
  }
};

struct xmlManeuverGroup {
  /*
  <ManeuverGroup maximumExecutionCount="1" name="egocar">
  */
  uint32_t maximumExecutionCount = 1;
  std::string name;
  xmlActors Actors;
  xmlManeuver Maneuver;
  friend std::ostream& operator<<(std::ostream& os, const xmlManeuverGroup& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<ManeuverGroup " << TX_VARS_NAME(maximumExecutionCount, v.maximumExecutionCount)
       << TX_VARS_NAME(name, std::quoted(v.name)) << ">" << std::endl
       << v.Actors << std::endl
       << v.Maneuver << std::endl
       << "</ManeuverGroup>";
    return os;
  }
};

using xmlManeuverGroups = std::vector<xmlManeuverGroup>;

struct xmlAct {
  /*
  <Act name="Act1">
  */
  std::string name;
  xmlManeuverGroups ManeuverGroup;
  xmlStartTrigger StartTrigger;
  friend std::ostream& operator<<(std::ostream& os, const xmlAct& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Act " << TX_VARS_NAME(name, std::quoted(v.name)) << ">" << std::endl;
    for (const auto& refManeuverGroup : v.ManeuverGroup) {
      os << refManeuverGroup << std::endl;
    }
    os << v.StartTrigger << std::endl;
    os << "</Act>";
    return os;
  }
};

using xmlActs = std::vector<xmlAct>;

struct xmlStory {
  std::string name;
  xmlParameterDeclarations ParameterDeclarations;
  xmlActs Act;
  /*
  <Story name="MyStory">
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlStory& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Story " << TX_VARS_NAME(name, std::quoted(v.name)) << ">" << std::endl
       << v.ParameterDeclarations << std::endl;
    for (const auto& refAct : v.Act) {
      os << refAct << std::endl;
    }
    os << "</Story>";
    return os;
  }
};

struct xmlStopTrigger {
  /*
  <StopTrigger/>
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlStopTrigger& v) TX_NOEXCEPT {
    os << "<StopTrigger/>";
    return os;
  }
};

struct xmlStoryboard {
  xmlInit Init;
  xmlStory Story;
  xmlStopTrigger StopTrigger;

  friend std::ostream& operator<<(std::ostream& os, const xmlStoryboard& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Storyboard> " << std::endl
       << v.Init << std::endl
       << v.Story << std::endl
       << v.StopTrigger << std::endl
       << "</Storyboard>";
    return os;
  }
};

struct xmlDirectory {
  std::string path;
  friend std::ostream& operator<<(std::ostream& os, const xmlDirectory& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Directory " << TX_VARS_NAME(path, std::quoted(v.path)) << "/>";
    return os;
  }
  bool empty() const { return path.empty(); }
};

struct xmlVehicleCatalog {
  xmlDirectory Directory;
  xmlVehicleCatalog() { Directory.path = "Catalogs/Vehicles"; }
  friend std::ostream& operator<<(std::ostream& os, const xmlVehicleCatalog& v) TX_NOEXCEPT {
    os << "<VehicleCatalog>" << std::endl << v.Directory << std::endl << "</VehicleCatalog>";
    return os;
  }
};

struct xmlPedestrianCatalog {
  xmlDirectory Directory;
  xmlPedestrianCatalog() { Directory.path = "Catalogs/Pedestrians"; }
  friend std::ostream& operator<<(std::ostream& os, const xmlPedestrianCatalog& v) TX_NOEXCEPT {
    os << "<PedestrianCatalog>" << std::endl << v.Directory << std::endl << "</PedestrianCatalog>";
    return os;
  }
};

struct xmlMiscObjectCatalog {
  xmlDirectory Directory;
  xmlMiscObjectCatalog() { Directory.path = "Catalogs/MiscObjects"; }
  friend std::ostream& operator<<(std::ostream& os, const xmlMiscObjectCatalog& v) TX_NOEXCEPT {
    os << "<MiscObjectCatalog>" << std::endl << v.Directory << std::endl << "</MiscObjectCatalog>";
    return os;
  }
};

struct xmlCatalogLocations {
  xmlVehicleCatalog VehicleCatalog;
  xmlPedestrianCatalog PedestrianCatalog;
  xmlMiscObjectCatalog MiscObjectCatalog;
  friend std::ostream& operator<<(std::ostream& os, const xmlCatalogLocations& v) TX_NOEXCEPT {
    os << "<CatalogLocations>" << std::endl;
    if (!v.VehicleCatalog.Directory.empty()) {
      os << v.VehicleCatalog << std::endl;
    }
    if (!v.PedestrianCatalog.Directory.empty()) {
      os << v.PedestrianCatalog << std::endl;
    }
    if (!v.MiscObjectCatalog.Directory.empty()) {
      os << v.MiscObjectCatalog << std::endl;
    }
    os << "</CatalogLocations>" << std::endl;
    return os;
  }
};

struct xmlOpenSCENARIO {
  xmlFileHeader FileHeader;
  std::vector<xmlParameterDeclarations> ParameterDeclarations;
  xmlCatalogLocations CatalogLocations;
  xmlRoadNetwork RoadNetwork;
  xmlEntities Entities;
  xmlStoryboard Storyboard;
  friend std::ostream& operator<<(std::ostream& os, const xmlOpenSCENARIO& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<OpenSCENARIO> " << std::endl << v.FileHeader << std::endl;
    for (auto& ps : v.ParameterDeclarations) {
      os << ps << std::endl;
    }
    os << v.CatalogLocations << std::endl << v.RoadNetwork << std::endl << "<Entities>" << std::endl;
    for (const auto& refEntity : v.Entities) {
      os << refEntity << std::endl;
    }
    os << "</Entities>" << std::endl << v.Storyboard << std::endl << "</OpenSCENARIO>";
    return os;
  }
};

struct xmlCatalog {
  std::string name;
  std::vector<xmlMiscObject> MiscObject;
  std::vector<xmlVehicle> Vehicle;
  std::vector<xmlPedestrian> Pedestrian;
  friend std::ostream& operator<<(std::ostream& os, const xmlCatalog& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Catalog " << TX_VARS_NAME(name, std::quoted(v.name)) << ">" << std::endl;
    for (auto&& misc_object : v.MiscObject) {
      os << misc_object << std::endl;
    }
    for (auto&& vehicle : v.Vehicle) {
      os << vehicle << std::endl;
    }
    for (auto&& pedestrian : v.Pedestrian) {
      os << pedestrian << std::endl;
    }
    os << "</Catalog>" << std::endl;
    return os;
  }
};

struct xmlOpenCatalog {
  xmlFileHeader FileHeader;
  xmlCatalog Catalog;
  friend std::ostream& operator<<(std::ostream& os, const xmlOpenCatalog& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<OpenSCENARIO> " << std::endl
       << v.FileHeader << std::endl
       << v.Catalog << std::endl
       << "</OpenSCENARIO>";
    return os;
  }
};

using OpenScenarioPtr = std::shared_ptr<xmlOpenSCENARIO>;

using OpenCatalogPtr = std::shared_ptr<xmlOpenCatalog>;

// xosc file to openscenario
extern OpenScenarioPtr Load(std::istream& stream);

extern OpenScenarioPtr Load(const std::string& file);

extern OpenCatalogPtr LoadCatalog(std::istream& stream);

extern OpenCatalogPtr LoadCatalog(const std::string& file);

// dump openscenario to xosc
extern std::string Dump(const OpenScenarioPtr& xosc) TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(OSC)
TX_NAMESPACE_CLOSE(SIM)
