// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "replay_xosc.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "tx_xml_utils.h"
#include "xml/intros_ptree.hpp"
#include "xml/util_traits.hpp"
using namespace utils::intros_ptree;
using namespace boost::property_tree;

TX_NAMESPACE_OPEN(SceneLoader)
TX_NAMESPACE_OPEN(OSC)

BEGIN_INTROS_TYPE_USER_NAME(xmlFileHeader, "FileHeader")
ADD_INTROS_ITEM_USER_NAME(description, MAKE_USER_NAME("description", "", true))
ADD_INTROS_ITEM_USER_NAME(date, MAKE_USER_NAME("date", "", true))
ADD_INTROS_ITEM_USER_NAME(author, MAKE_USER_NAME("author", "", true))
ADD_INTROS_ITEM_USER_NAME(revMajor, MAKE_USER_NAME("revMajor", "", true))
ADD_INTROS_ITEM_USER_NAME(revMinor, MAKE_USER_NAME("revMinor", "", true))
END_INTROS_TYPE(xmlFileHeader)

BEGIN_INTROS_TYPE_USER_NAME(xmlLogicFile, "LogicFile")
ADD_INTROS_ITEM_USER_NAME(filepath, MAKE_USER_NAME("filepath", "", true))
END_INTROS_TYPE(xmlLogicFile)

BEGIN_INTROS_TYPE_USER_NAME(xmlRoadNetwork, "RoadNetwork")
ADD_INTROS_ITEM_USER_NAME(LogicFile, "LogicFile")
END_INTROS_TYPE(xmlRoadNetwork)

BEGIN_INTROS_TYPE_USER_NAME(xmlCenter, "Center")
ADD_INTROS_ITEM_USER_NAME(x, MAKE_USER_NAME("x", "", true))
ADD_INTROS_ITEM_USER_NAME(y, MAKE_USER_NAME("y", "", true))
ADD_INTROS_ITEM_USER_NAME(z, MAKE_USER_NAME("z", "", true))
END_INTROS_TYPE(xmlCenter)

BEGIN_INTROS_TYPE_USER_NAME(xmlDimensions, "Dimensions")
ADD_INTROS_ITEM_USER_NAME(width, MAKE_USER_NAME("width", "", true))
ADD_INTROS_ITEM_USER_NAME(length, MAKE_USER_NAME("length", "", true))
ADD_INTROS_ITEM_USER_NAME(height, MAKE_USER_NAME("height", "", true))
END_INTROS_TYPE(xmlDimensions)

BEGIN_INTROS_TYPE_USER_NAME(xmlBoundingBox, "BoundingBox")
ADD_INTROS_ITEM_USER_NAME(Center, MAKE_USER_NAME("Center", "", false))
ADD_INTROS_ITEM_USER_NAME(Dimensions, MAKE_USER_NAME("Dimensions", "", false))
END_INTROS_TYPE(xmlBoundingBox)

BEGIN_INTROS_TYPE_USER_NAME(xmlPerformance, "Performance")
ADD_INTROS_ITEM_USER_NAME(maxSpeed, MAKE_USER_NAME("maxSpeed", "", true))
ADD_INTROS_ITEM_USER_NAME(maxDeceleration, MAKE_USER_NAME("maxDeceleration", "", true))
ADD_INTROS_ITEM_USER_NAME(maxAcceleration, MAKE_USER_NAME("maxAcceleration", "", true))
END_INTROS_TYPE(xmlPerformance)

BEGIN_INTROS_TYPE_USER_NAME(xmlFrontAxle, "FrontAxle")
ADD_INTROS_ITEM_USER_NAME(maxSteering, MAKE_USER_NAME("maxSteering", "", true))
ADD_INTROS_ITEM_USER_NAME(wheelDiameter, MAKE_USER_NAME("wheelDiameter", "", true))
ADD_INTROS_ITEM_USER_NAME(trackWidth, MAKE_USER_NAME("trackWidth", "", true))
ADD_INTROS_ITEM_USER_NAME(positionX, MAKE_USER_NAME("positionX", "", true))
ADD_INTROS_ITEM_USER_NAME(positionZ, MAKE_USER_NAME("positionZ", "", true))
END_INTROS_TYPE(xmlFrontAxle)

BEGIN_INTROS_TYPE_USER_NAME(xmlRearAxle, "RearAxle")
ADD_INTROS_ITEM_USER_NAME(maxSteering, MAKE_USER_NAME("maxSteering", "", true))
ADD_INTROS_ITEM_USER_NAME(wheelDiameter, MAKE_USER_NAME("wheelDiameter", "", true))
ADD_INTROS_ITEM_USER_NAME(trackWidth, MAKE_USER_NAME("trackWidth", "", true))
ADD_INTROS_ITEM_USER_NAME(positionX, MAKE_USER_NAME("positionX", "", true))
ADD_INTROS_ITEM_USER_NAME(positionZ, MAKE_USER_NAME("positionZ", "", true))
END_INTROS_TYPE(xmlRearAxle)

BEGIN_INTROS_TYPE_USER_NAME(xmlAxles, "Axles")
ADD_INTROS_ITEM_USER_NAME(FrontAxle, MAKE_USER_NAME("FrontAxle", "", false))
ADD_INTROS_ITEM_USER_NAME(RearAxle, MAKE_USER_NAME("RearAxle", "", false))
END_INTROS_TYPE(xmlAxles)

BEGIN_INTROS_TYPE_USER_NAME(xmlProperty, "Property")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
END_INTROS_TYPE(xmlProperty)

BEGIN_INTROS_TYPE_USER_NAME(xmlFile, "File")
ADD_INTROS_ITEM_USER_NAME(filepath, MAKE_USER_NAME("filepath", "", true))
END_INTROS_TYPE(xmlFile)

BEGIN_INTROS_TYPE_USER_NAME(xmlProperties, "Properties")
ADD_INTROS_ITEM_USER_NAME(Property, MAKE_USER_NAME("Property", "", false))
ADD_INTROS_ITEM_USER_NAME(File, MAKE_USER_NAME("File", "", false))
END_INTROS_TYPE(xmlProperties)

BEGIN_INTROS_TYPE_USER_NAME(xmlVehicle, "Vehicle")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(vehicleCategory, MAKE_USER_NAME("vehicleCategory", "", true))
ADD_INTROS_ITEM_USER_NAME(BoundingBox, MAKE_USER_NAME("BoundingBox", "", false))
ADD_INTROS_ITEM_USER_NAME(Performance, MAKE_USER_NAME("Performance", "", false))
ADD_INTROS_ITEM_USER_NAME(Axles, MAKE_USER_NAME("Axles", "", false))
ADD_INTROS_ITEM_USER_NAME(Properties, MAKE_USER_NAME("Properties", "", false))
END_INTROS_TYPE(xmlVehicle)

BEGIN_INTROS_TYPE_USER_NAME(xmlScenarioObject, "ScenarioObject")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(Vehicle, MAKE_USER_NAME("Vehicle", "", false))
END_INTROS_TYPE(xmlScenarioObject)

BEGIN_INTROS_TYPE_USER_NAME(xmlTrafficSignalStateAction, "TrafficSignalStateAction")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(state, MAKE_USER_NAME("state", "", true))
END_INTROS_TYPE(xmlTrafficSignalStateAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlTrafficSignalAction, "TrafficSignalAction")
ADD_INTROS_ITEM_USER_NAME(TrafficSignalStateAction, MAKE_USER_NAME("TrafficSignalStateAction", "", false))
END_INTROS_TYPE(xmlTrafficSignalAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlInfrastructureAction, "InfrastructureAction")
ADD_INTROS_ITEM_USER_NAME(TrafficSignalAction, MAKE_USER_NAME("TrafficSignalAction", "", false))
END_INTROS_TYPE(xmlInfrastructureAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlGlobalAction, "GlobalAction")
ADD_INTROS_ITEM_USER_NAME(InfrastructureAction, MAKE_USER_NAME("InfrastructureAction", "", false))
END_INTROS_TYPE(xmlGlobalAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivate, "Private")
ADD_INTROS_ITEM_USER_NAME(entityRef, MAKE_USER_NAME("entityRef", "", true))
ADD_INTROS_ITEM_USER_NAME(PrivateAction, MAKE_USER_NAME("PrivateAction", "", false))
END_INTROS_TYPE(xmlPrivate)

BEGIN_INTROS_TYPE_USER_NAME(xmlWorldPosition_1, "WorldPosition")
ADD_INTROS_ITEM_USER_NAME(h, MAKE_USER_NAME("h", "", true))
ADD_INTROS_ITEM_USER_NAME(p, MAKE_USER_NAME("p", "", true))
ADD_INTROS_ITEM_USER_NAME(r, MAKE_USER_NAME("r", "", true))
ADD_INTROS_ITEM_USER_NAME(x, MAKE_USER_NAME("x", "", true))
ADD_INTROS_ITEM_USER_NAME(y, MAKE_USER_NAME("y", "", true))
ADD_INTROS_ITEM_USER_NAME(z, MAKE_USER_NAME("z", "", true))
END_INTROS_TYPE(xmlWorldPosition_1)

BEGIN_INTROS_TYPE_USER_NAME(xmlPosition_1, "Position")
ADD_INTROS_ITEM_USER_NAME(WorldPosition, MAKE_USER_NAME("WorldPosition", "", false))
END_INTROS_TYPE(xmlPosition_1)

BEGIN_INTROS_TYPE_USER_NAME(xmlTeleportAction, "TeleportAction")
ADD_INTROS_ITEM_USER_NAME(Position, MAKE_USER_NAME("Position", "", false))
END_INTROS_TYPE(xmlTeleportAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction_1, "PrivateAction")
ADD_INTROS_ITEM_USER_NAME(TeleportAction, MAKE_USER_NAME("TeleportAction", "", false))
END_INTROS_TYPE(xmlPrivateAction_1)

BEGIN_INTROS_TYPE_USER_NAME(xmlActions, "Actions")
ADD_INTROS_ITEM_USER_NAME(GlobalAction, MAKE_USER_NAME("GlobalAction", "", false))
ADD_INTROS_ITEM_USER_NAME(Private, MAKE_USER_NAME("Private", "", false))
END_INTROS_TYPE(xmlActions)

BEGIN_INTROS_TYPE_USER_NAME(xmlInit, "Init")
ADD_INTROS_ITEM_USER_NAME(Actions, MAKE_USER_NAME("Actions", "", false))
END_INTROS_TYPE(xmlInit)

BEGIN_INTROS_TYPE_USER_NAME(xmlParameterDeclaration, "ParameterDeclaration")
ADD_INTROS_ITEM_USER_NAME(parameterType, MAKE_USER_NAME("parameterType", "", true))
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
END_INTROS_TYPE(xmlParameterDeclaration)

BEGIN_INTROS_TYPE_USER_NAME(xmlParameterDeclarations, "ParameterDeclarations")
ADD_INTROS_ITEM_USER_NAME(ParameterDeclaration, MAKE_USER_NAME("ParameterDeclaration", "", false))
END_INTROS_TYPE(xmlParameterDeclarations)

BEGIN_INTROS_TYPE_USER_NAME(xmlEntityRef, "EntityRef")
ADD_INTROS_ITEM_USER_NAME(entityRef, MAKE_USER_NAME("entityRef", "", true))
END_INTROS_TYPE(xmlEntityRef)

BEGIN_INTROS_TYPE_USER_NAME(xmlActors, "Actors")
ADD_INTROS_ITEM_USER_NAME(selectTriggeringEntities, MAKE_USER_NAME("selectTriggeringEntities", "", true))
ADD_INTROS_ITEM_USER_NAME(EntityRef, MAKE_USER_NAME("EntityRef", "", false))
END_INTROS_TYPE(xmlActors)

BEGIN_INTROS_TYPE_USER_NAME(xmlWorldPosition, "WorldPosition")
ADD_INTROS_ITEM_USER_NAME(h, MAKE_USER_NAME("h", "", true))
ADD_INTROS_ITEM_USER_NAME(x, MAKE_USER_NAME("x", "", true))
ADD_INTROS_ITEM_USER_NAME(y, MAKE_USER_NAME("y", "", true))
ADD_INTROS_ITEM_USER_NAME(z, MAKE_USER_NAME("z", "", true))
END_INTROS_TYPE(xmlWorldPosition)

BEGIN_INTROS_TYPE_USER_NAME(xmlPosition, "Position")
ADD_INTROS_ITEM_USER_NAME(WorldPosition, MAKE_USER_NAME("WorldPosition", "", false))
END_INTROS_TYPE(xmlPosition)

BEGIN_INTROS_TYPE_USER_NAME(xmlVertex, "Vertex")
ADD_INTROS_ITEM_USER_NAME(time, MAKE_USER_NAME("time", "", true))
ADD_INTROS_ITEM_USER_NAME(Position, MAKE_USER_NAME("Position", "", false))
END_INTROS_TYPE(xmlVertex)

BEGIN_INTROS_TYPE_USER_NAME(xmlPolyline, "Polyline")
ADD_INTROS_ITEM_USER_NAME(Vertexs, MAKE_USER_NAME("Vertex", "", false))
END_INTROS_TYPE(xmlPolyline)

BEGIN_INTROS_TYPE_USER_NAME(xmlShape, "Shape")
ADD_INTROS_ITEM_USER_NAME(Polyline, MAKE_USER_NAME("Polyline", "", false))
END_INTROS_TYPE(xmlShape)

BEGIN_INTROS_TYPE_USER_NAME(xmlTrajectory, "Trajectory")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(closed, MAKE_USER_NAME("closed", "", true))
ADD_INTROS_ITEM_USER_NAME(Shape, MAKE_USER_NAME("Shape", "", false))
END_INTROS_TYPE(xmlTrajectory)

BEGIN_INTROS_TYPE_USER_NAME(xmlTrajectoryFollowingMode, "TrajectoryFollowingMode")
ADD_INTROS_ITEM_USER_NAME(followingMode, MAKE_USER_NAME("followingMode", "", true))
END_INTROS_TYPE(xmlTrajectoryFollowingMode)

BEGIN_INTROS_TYPE_USER_NAME(xmlFollowTrajectoryAction, "FollowTrajectoryAction")
ADD_INTROS_ITEM_USER_NAME(Trajectory, MAKE_USER_NAME("Trajectory", "", false))
ADD_INTROS_ITEM_USER_NAME(TrajectoryFollowingMode, MAKE_USER_NAME("TrajectoryFollowingMode", "", false))
END_INTROS_TYPE(xmlFollowTrajectoryAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlRoutingAction, "RoutingAction")
ADD_INTROS_ITEM_USER_NAME(FollowTrajectoryAction, MAKE_USER_NAME("FollowTrajectoryAction", "", false))
END_INTROS_TYPE(xmlRoutingAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction, "PrivateAction")
ADD_INTROS_ITEM_USER_NAME(RoutingAction, MAKE_USER_NAME("RoutingAction", "", false))
END_INTROS_TYPE(xmlPrivateAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlAction, "Action")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(PrivateAction, MAKE_USER_NAME("PrivateAction", "", false))
END_INTROS_TYPE(xmlAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlEvent, "Actors")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(priority, MAKE_USER_NAME("priority", "", true))
ADD_INTROS_ITEM_USER_NAME(Action, MAKE_USER_NAME("Action", "", false))
END_INTROS_TYPE(xmlEvent)

BEGIN_INTROS_TYPE_USER_NAME(xmlManeuver, "Maneuver")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(Event, MAKE_USER_NAME("Event", "", false))
END_INTROS_TYPE(xmlManeuver)

BEGIN_INTROS_TYPE_USER_NAME(xmlManeuverGroup, "ManeuverGroup")
ADD_INTROS_ITEM_USER_NAME(maximumExecutionCount, MAKE_USER_NAME("maximumExecutionCount", "", true))
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(Actors, MAKE_USER_NAME("Actors", "", false))
ADD_INTROS_ITEM_USER_NAME(Maneuver, MAKE_USER_NAME("Maneuver", "", false))
END_INTROS_TYPE(xmlManeuverGroup)

BEGIN_INTROS_TYPE_USER_NAME(xmlAct, "Act")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(ManeuverGroup, MAKE_USER_NAME("ManeuverGroup", "", false))
END_INTROS_TYPE(xmlAct)

BEGIN_INTROS_TYPE_USER_NAME(xmlStory, "Story")
ADD_INTROS_ITEM_USER_NAME(ParameterDeclarations, MAKE_USER_NAME("ParameterDeclarations", "", false))
ADD_INTROS_ITEM_USER_NAME(Act, MAKE_USER_NAME("Act", "", false))
END_INTROS_TYPE(xmlStory)

BEGIN_INTROS_TYPE_USER_NAME(xmlStoryboard, "Storyboard")
ADD_INTROS_ITEM_USER_NAME(Init, MAKE_USER_NAME("Init", "", false))
ADD_INTROS_ITEM_USER_NAME(Story, MAKE_USER_NAME("Story", "", false))
END_INTROS_TYPE(xmlStoryboard)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO, "OpenSCENARIO")
ADD_INTROS_ITEM_USER_NAME(FileHeader, MAKE_USER_NAME("FileHeader", "", false))
ADD_INTROS_ITEM_USER_NAME(RoadNetwork, MAKE_USER_NAME("RoadNetwork", "", false))
ADD_INTROS_ITEM_USER_NAME(Entities, MAKE_USER_NAME("ScenarioObject", "Entities", false))
ADD_INTROS_ITEM_USER_NAME(Storyboard, MAKE_USER_NAME("Storyboard", "", false))
END_INTROS_TYPE(xmlOpenSCENARIO)

OpenSCENARIO_ptr load_scene_replay_xosc(const Base::txString& _traffic_path) TX_NOEXCEPT {
  std::ifstream infile(_traffic_path);
  if (infile) {
    std::stringstream ss;
    ss << infile.rdbuf();
    infile.close();
    ptree tree;
    Utils::FileToStreamWithStandaloneYes(ss);
    read_xml(ss, tree);
    // xml_writer_settings<Base::txString> settings(' ', 4); // this is needed for xml printing to have proper
    // whitespace write_xml(std::cout, tree, settings);
    return std::make_shared<xmlOpenSCENARIO>(make_intros_object<xmlOpenSCENARIO>(tree));
  } else {
    LOG(WARNING) << "Load Xml File Failure. " << _traffic_path;
    return nullptr;
  }
}

TX_NAMESPACE_CLOSE(OSC)
TX_NAMESPACE_CLOSE(SceneLoader)
