// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <iomanip>
#include <sstream>
#include <vector>
#include "tx_header.h"
TX_NAMESPACE_OPEN(SceneLoader)
TX_NAMESPACE_OPEN(OSC)

using txString = Base::txString;

struct xmlFile {
  /*
  <File filepath=""/>
  */
  txString filepath;
  friend std::ostream& operator<<(std::ostream& os, const xmlFile& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<File " << TX_VARS_NAME(filepath, quoted(v.filepath)) << "/>";
    return os;
  }
};

struct xmlProperty {
  /*
  <Property name="control" value="external"/>
  */
  txString name;
  txString value;
  friend std::ostream& operator<<(std::ostream& os, const xmlProperty& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Property " << TX_VARS_NAME(name, quoted(v.name))
       << TX_VARS_NAME(value, quoted(v.value)) << "/>";
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
  xmlProperty Property;
  xmlFile File;
  friend std::ostream& operator<<(std::ostream& os, const xmlProperties& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Properties>" << std::endl
       << v.Property << std::endl
       << v.File << std::endl
       << "</Properties>";
    return os;
  }
};

struct xmlRearAxle {
  /*
  <RearAxle maxSteering="0" wheelDiameter="0.63994" trackWidth="1.608" positionX="0" positionZ="0.31997"/>
  */
  txString maxSteering;
  txString wheelDiameter;
  txString trackWidth;
  txString positionX;
  txString positionZ;
  friend std::ostream& operator<<(std::ostream& os, const xmlRearAxle& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<RearAxle " << TX_VARS_NAME(maxSteering, quoted(v.maxSteering))
       << TX_VARS_NAME(wheelDiameter, quoted(v.wheelDiameter)) << TX_VARS_NAME(trackWidth, quoted(v.trackWidth))
       << TX_VARS_NAME(positionX, quoted(v.positionX)) << TX_VARS_NAME(positionZ, quoted(v.positionZ)) << "/>";
    return os;
  }
};

struct xmlFrontAxle {
  /*
  <FrontAxle maxSteering="0.46" wheelDiameter="0.63994" trackWidth="1.608" positionX="2.536" positionZ="0.31997"/>
  */
  txString maxSteering;
  txString wheelDiameter;
  txString trackWidth;
  txString positionX;
  txString positionZ;
  friend std::ostream& operator<<(std::ostream& os, const xmlFrontAxle& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<FrontAxle " << TX_VARS_NAME(maxSteering, quoted(v.maxSteering))
       << TX_VARS_NAME(wheelDiameter, quoted(v.wheelDiameter)) << TX_VARS_NAME(trackWidth, quoted(v.trackWidth))
       << TX_VARS_NAME(positionX, quoted(v.positionX)) << TX_VARS_NAME(positionZ, quoted(v.positionZ)) << "/>";
    return os;
  }
};

struct xmlAxles {
  /*
  <Axles>
      <FrontAxle maxSteering="0.46" wheelDiameter="0.63994" trackWidth="1.608" positionX="2.536" positionZ="0.31997"/>
      <RearAxle maxSteering="0" wheelDiameter="0.63994" trackWidth="1.608" positionX="0" positionZ="0.31997"/>
  </Axles>
  */
  xmlFrontAxle FrontAxle;
  xmlRearAxle RearAxle;
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
  txString maxSpeed;
  txString maxDeceleration;
  txString maxAcceleration;
  friend std::ostream& operator<<(std::ostream& os, const xmlPerformance& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Performance " << TX_VARS_NAME(maxSpeed, quoted(v.maxSpeed))
       << TX_VARS_NAME(maxDeceleration, quoted(v.maxDeceleration))
       << TX_VARS_NAME(maxAcceleration, quoted(v.maxAcceleration)) << "/>";
    return os;
  }
};

struct xmlCenter {
  /*
  <Center x="121.17487722345179" y="31.270958542979315" z="0.0"/>
  */
  txString x;
  txString y;
  txString z;
  friend std::ostream& operator<<(std::ostream& os, const xmlCenter& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Center " << TX_VARS_NAME(x, quoted(v.x)) << TX_VARS_NAME(y, quoted(v.y))
       << TX_VARS_NAME(z, quoted(v.z)) << "/>";
    return os;
  }
};

struct xmlDimensions {
  /*
  <Dimensions width="1.0" length="2.0" height="1.0"/>
  */
  txString width;
  txString length;
  txString height;
  friend std::ostream& operator<<(std::ostream& os, const xmlDimensions& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Dimensions " << TX_VARS_NAME(width, quoted(v.width))
       << TX_VARS_NAME(length, quoted(v.length)) << TX_VARS_NAME(height, quoted(v.height)) << "/>";
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
     positionZ="0.31997"/> <RearAxle maxSteering="0" wheelDiameter="0.63994" trackWidth="1.608" positionX="0"
     positionZ="0.31997"/>
          </Axles>
          <Properties>
            <Property name="control" value="external"/>
            <File filepath=""/>
          </Properties>
        </Vehicle>
  */
  txString name;
  txString vehicleCategory;
  xmlBoundingBox BoundingBox;
  xmlPerformance Performance;
  xmlAxles Axles;
  xmlProperties Properties;
  friend std::ostream& operator<<(std::ostream& os, const xmlVehicle& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Vehicle " << TX_VARS_NAME(name, quoted(v.name))
       << TX_VARS_NAME(vehicleCategory, quoted(v.vehicleCategory)) << ">" << std::endl
       << v.BoundingBox << std::endl
       << v.Performance << std::endl
       << v.Axles << std::endl
       << v.Properties << std::endl
       << "</Vehicle>";
    return os;
  }
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
     positionZ="0.31997"/> <RearAxle maxSteering="0" wheelDiameter="0.63994" trackWidth="1.608" positionX="0"
     positionZ="0.31997"/>
          </Axles>
          <Properties>
            <Property name="control" value="external"/>
            <File filepath=""/>
          </Properties>
        </Vehicle>
      </ScenarioObject>
  */
  txString name;
  xmlVehicle Vehicle;
  friend std::ostream& operator<<(std::ostream& os, const xmlScenarioObject& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<ScenarioObject " << TX_VARS_NAME(name, quoted(v.name)) << ">" << std::endl
       << v.Vehicle << std::endl
       << "</ScenarioObject>";
    return os;
  }
};
using xmlEntities = std::vector<xmlScenarioObject>;

struct xmlLogicFile {
  /*
<LogicFile filepath="test/map/d2d_20190726.xodr"/>
  */
  txString filepath;
  friend std::ostream& operator<<(std::ostream& os, const xmlLogicFile& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<LogicFile " << TX_VARS_NAME(filepath, quoted(v.filepath)) << "/>";
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
  friend std::ostream& operator<<(std::ostream& os, const xmlRoadNetwork& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<RoadNetwork>" << std::endl << v.LogicFile << std::endl << "</RoadNetwork>";
    return os;
  }
};

struct xmlFileHeader {
  /*
<FileHeader
  description="simulation-test-1251316161/modules/144115205301725114/12334-5fd04d0ee0103c2314c7d695/sim_planning.tar.gz"
date="2020-12-10T02:28:13" author="" revMajor="1" revMinor="0"/>
  */
  txString description;
  txString date;
  txString author;
  txString revMajor;
  txString revMinor;
  friend std::ostream& operator<<(std::ostream& os, const xmlFileHeader& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<FileHeader " << TX_VARS_NAME(description, quoted(v.description))
       << TX_VARS_NAME(date, quoted(v.date)) << TX_VARS_NAME(author, quoted(v.author))
       << TX_VARS_NAME(revMajor, quoted(v.revMajor)) << TX_VARS_NAME(revMinor, quoted(v.revMinor)) << "/>";
    return os;
  }
};

struct xmlTrafficSignalStateAction {
  /*
<TrafficSignalStateAction name = "trafficLight_1011" state = "0" / >
  */
  txString name;
  txString state;
  friend std::ostream& operator<<(std::ostream& os, const xmlTrafficSignalStateAction& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<TrafficSignalStateAction " << TX_VARS_NAME(name, quoted(v.name))
       << TX_VARS_NAME(state, quoted(v.state)) << "/>";
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

struct xmlWorldPosition_1 {
  txString h;
  txString p;
  txString r;
  txString x;
  txString y;
  txString z;
  /*
  <WorldPosition h="-1.330236836202213" p="0.0" r="0.0" x="121.17487722345179" y="31.270958542979315" z="0.0"/>
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlWorldPosition_1& v) TX_NOEXCEPT {
    os << "<WorldPosition " << TX_VARS_NAME(h, quoted(v.h)) << TX_VARS_NAME(p, quoted(v.p))
       << TX_VARS_NAME(r, quoted(v.r)) << TX_VARS_NAME(x, quoted(v.x)) << TX_VARS_NAME(y, quoted(v.y))
       << TX_VARS_NAME(z, quoted(v.z)) << ">";
    return os;
  }
};

struct xmlPosition_1 {
  xmlWorldPosition_1 WorldPosition;
  friend std::ostream& operator<<(std::ostream& os, const xmlPosition_1& v) TX_NOEXCEPT {
    os << "<Position>" << std::endl << v.WorldPosition << std::endl << "</Position>";
    return os;
  }
};

struct xmlTeleportAction {
  xmlPosition_1 Position;
  friend std::ostream& operator<<(std::ostream& os, const xmlTeleportAction& v) TX_NOEXCEPT {
    os << "<TeleportAction>" << std::endl << v.Position << std::endl << "</TeleportAction>";
    return os;
  }
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
  txString entityRef;
  xmlPrivateAction_1 PrivateAction;

  friend std::ostream& operator<<(std::ostream& os, const xmlPrivate& v) TX_NOEXCEPT {
    os << "<Private " << TX_VARS_NAME(entityRef, quoted(v.entityRef)) << ">" << std::endl
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
  txString parameterType;
  txString name;
  txString value;
  friend std::ostream& operator<<(std::ostream& os, const xmlParameterDeclaration& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<ParameterDeclaration " << TX_VARS_NAME(parameterType, quoted(v.parameterType))
       << TX_VARS_NAME(name, quoted(v.name)) << TX_VARS_NAME(value, quoted(v.value)) << "/>";
    return os;
  }
};

struct xmlParameterDeclarations {
  xmlParameterDeclaration ParameterDeclaration;
  friend std::ostream& operator<<(std::ostream& os, const xmlParameterDeclarations& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<ParameterDeclarations>" << std::endl
       << v.ParameterDeclaration << std::endl
       << "</ParameterDeclarations>";
    return os;
  }
};

struct xmlEntityRef {
  /*
  <EntityRef entityRef="egocar"/>
  */
  txString entityRef;
  friend std::ostream& operator<<(std::ostream& os, const xmlEntityRef& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<EntityRef " << TX_VARS_NAME(entityRef, quoted(v.entityRef)) << "/>";
    return os;
  }
};

struct xmlActors {
  /*
  <Actors selectTriggeringEntities="false">
  */
  txString selectTriggeringEntities;
  xmlEntityRef EntityRef;

  /*
  <Actors selectTriggeringEntities="false">
          <EntityRef entityRef="egocar"/>
        </Actors>
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlActors& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Actors " << TX_VARS_NAME(selectTriggeringEntities, quoted(v.selectTriggeringEntities))
       << ">" << std::endl
       << v.EntityRef << std::endl
       << "</Actors>";
    return os;
  }
};

struct xmlWorldPosition {
  /*
  <WorldPosition h="-1.330236836202213" x="121.17487722345179" y="31.270958542979315" z="0.0"/>
  */
  txString h;
  txString x;
  txString y;
  txString z;
  friend std::ostream& operator<<(std::ostream& os, const xmlWorldPosition& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<WorldPosition " << TX_VARS_NAME(h, quoted(v.h)) << TX_VARS_NAME(x, quoted(v.x))
       << TX_VARS_NAME(y, quoted(v.y)) << TX_VARS_NAME(z, quoted(v.z)) << "/>";
    return os;
  }
};

struct xmlPosition {
  xmlWorldPosition WorldPosition;
  /*
  <Position>
  <WorldPosition h="-1.330236836202213" x="121.17487722345179" y="31.270958542979315" z="0.0"/>
  </Position>
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlPosition& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Position> " << std::endl << v.WorldPosition << std::endl << "</Position>";
    return os;
  }
};

struct xmlVertex {
  xmlPosition Position;
  txString time;
  /*
  <Vertex time="0.0">
      <Position>
      <WorldPosition h="-1.330236836202213" x="121.17487722345179" y="31.270958542979315" z="0.0"/>
      </Position>
  </Vertex>
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlVertex& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Vertex " << TX_VARS_NAME(time, quoted(v.time)) << ">" << std::endl
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
  txString name;
  txString closed;
  xmlShape Shape;
  friend std::ostream& operator<<(std::ostream& os, const xmlTrajectory& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Trajectory " << TX_VARS_NAME(name, quoted(v.name))
       << TX_VARS_NAME(closed, quoted(v.closed)) << ">" << std::endl
       << v.Shape << std::endl
       << "</Trajectory>";
    return os;
  }
};

struct xmlTrajectoryFollowingMode {
  txString followingMode;
  /*
  <TrajectoryFollowingMode followingMode="follow"/>
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlTrajectoryFollowingMode& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<TrajectoryFollowingMode " << TX_VARS_NAME(followingMode, quoted(v.followingMode))
       << "/>";
    return os;
  }
};

struct xmlFollowTrajectoryAction {
  xmlTrajectory Trajectory;
  xmlTrajectoryFollowingMode TrajectoryFollowingMode;
  friend std::ostream& operator<<(std::ostream& os, const xmlFollowTrajectoryAction& v) TX_NOEXCEPT {
    os << "<FollowTrajectoryAction>" << std::endl
       << v.Trajectory << std::endl
       << v.TrajectoryFollowingMode << std::endl
       << "</FollowTrajectoryAction>";
    return os;
  }
};

struct xmlRoutingAction {
  xmlFollowTrajectoryAction FollowTrajectoryAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlRoutingAction& v) TX_NOEXCEPT {
    os << "<RoutingAction>" << std::endl << v.FollowTrajectoryAction << std::endl << "</RoutingAction>";
    return os;
  }
};

struct xmlPrivateAction {
  xmlRoutingAction RoutingAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlPrivateAction& v) TX_NOEXCEPT {
    os << "<PrivateAction>" << std::endl << v.RoutingAction << std::endl << "</PrivateAction>";
    return os;
  }
};

struct xmlAction {
  txString name;
  /*
  <Action name="Action1">
  */
  xmlPrivateAction PrivateAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlAction& v) TX_NOEXCEPT {
    os << "<Action " << TX_VARS_NAME(name, quoted(v.name)) << ">" << std::endl
       << v.PrivateAction << std::endl
       << "</Action>";
    return os;
  }
};

struct xmlEvent {
  /*
  <Event name="Event1" priority="overwrite">
  */
  txString name;
  txString priority;
  xmlAction Action;
  friend std::ostream& operator<<(std::ostream& os, const xmlEvent& v) TX_NOEXCEPT {
    os << "<Event " << TX_VARS_NAME(name, quoted(v.name)) << TX_VARS_NAME(priority, quoted(v.priority)) << ">"
       << std::endl
       << v.Action << std::endl
       << "</Event>";
    return os;
  }
};

struct xmlManeuver {
  txString name;
  xmlEvent Event;
  friend std::ostream& operator<<(std::ostream& os, const xmlManeuver& v) TX_NOEXCEPT {
    os << "<Maneuver " << TX_VARS_NAME(name, quoted(v.name)) << ">" << std::endl
       << v.Event << std::endl
       << "</Maneuver>";
    return os;
  }
};

struct xmlManeuverGroup {
  /*
  <ManeuverGroup maximumExecutionCount="1" name="egocar">
  */
  txString maximumExecutionCount;
  txString name;
  xmlActors Actors;
  xmlManeuver Maneuver;
  friend std::ostream& operator<<(std::ostream& os, const xmlManeuverGroup& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<ManeuverGroup " << TX_VARS_NAME(maximumExecutionCount, quoted(v.maximumExecutionCount))
       << TX_VARS_NAME(name, quoted(v.name)) << ">" << std::endl
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
  txString name;
  xmlManeuverGroups ManeuverGroup;
  friend std::ostream& operator<<(std::ostream& os, const xmlAct& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Act " << TX_VARS_NAME(name, quoted(v.name)) << ">" << std::endl;
    for (const auto& refManeuverGroup : v.ManeuverGroup) {
      os << refManeuverGroup << std::endl;
    }
    os << "</Act>";
    return os;
  }
};

struct xmlStory {
  xmlParameterDeclarations ParameterDeclarations;
  xmlAct Act;
  /*
  <Story name="MyStory">
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlStory& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Story> " << v.ParameterDeclarations << std::endl << v.Act << std::endl << "</Story>";
    return os;
  }
};

struct xmlStoryboard {
  xmlInit Init;
  xmlStory Story;

  friend std::ostream& operator<<(std::ostream& os, const xmlStoryboard& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Storyboard> " << std::endl
       << v.Init << std::endl
       << v.Story << std::endl
       << "</Storyboard>";
    return os;
  }
};

struct xmlOpenSCENARIO {
  xmlFileHeader FileHeader;
  xmlRoadNetwork RoadNetwork;
  xmlEntities Entities;
  xmlStoryboard Storyboard;
  friend std::ostream& operator<<(std::ostream& os, const xmlOpenSCENARIO& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<OpenSCENARIO> " << std::endl
       << v.FileHeader << std::endl
       << v.RoadNetwork << std::endl
       << "<Entities>" << std::endl;
    for (const auto& refEntity : v.Entities) {
      os << refEntity << std::endl;
    }
    os << "</Entities>" << std::endl << v.Storyboard << std::endl << "</OpenSCENARIO>";
    return os;
  }
};

using OpenSCENARIO_ptr = std::shared_ptr<xmlOpenSCENARIO>;

void debug_tip();

extern OpenSCENARIO_ptr load_scene_replay_xosc(const Base::txString& _traffic_path) TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(OSC)
TX_NAMESPACE_CLOSE(SceneLoader)
