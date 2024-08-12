/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/xosc_replay/open_scenario_tpl.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "intros_ptree.hpp"
#include "util_traits.hpp"

using namespace utils::intros_ptree;
using namespace boost::property_tree;

TX_NAMESPACE_OPEN(SIM)
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

BEGIN_INTROS_TYPE_USER_NAME(xmlTrafficSignalState, "TrafficSignalState")
ADD_INTROS_ITEM_USER_NAME(state, MAKE_USER_NAME("state", "", true))
ADD_INTROS_ITEM_USER_NAME(trafficSignalId, MAKE_USER_NAME("trafficSignalId", "", true))
END_INTROS_TYPE(xmlTrafficSignalState)

BEGIN_INTROS_TYPE_USER_NAME(xmlPhase, "Phase")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(duration, MAKE_USER_NAME("duration", "", true))
ADD_INTROS_ITEM_USER_NAME(TrafficSignalState, MAKE_USER_NAME("TrafficSignalState", "", false))
END_INTROS_TYPE(xmlPhase)

BEGIN_INTROS_TYPE_USER_NAME(xmlTrafficSignalController, "TrafficSignalController")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(delay, MAKE_USER_NAME("delay", "", true))
ADD_INTROS_ITEM_USER_NAME(reference, MAKE_USER_NAME("reference", "", true))
ADD_INTROS_ITEM_USER_NAME(Phase, MAKE_USER_NAME("Phase", "", false))
END_INTROS_TYPE(xmlTrafficSignalController)

BEGIN_INTROS_TYPE_USER_NAME(xmlTrafficSignals, "TrafficSignals")
ADD_INTROS_ITEM_USER_NAME(TrafficSignalController, MAKE_USER_NAME("TrafficSignalController", "", false))
END_INTROS_TYPE(xmlTrafficSignals)

BEGIN_INTROS_TYPE_USER_NAME(xmlRoadNetwork, "RoadNetwork")
ADD_INTROS_ITEM_USER_NAME(LogicFile, "LogicFile")
ADD_INTROS_ITEM_USER_NAME(TrafficSignals, "TrafficSignals")
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

BEGIN_INTROS_TYPE(xmlAxle)
ADD_INTROS_ITEM_USER_NAME(maxSteering, MAKE_USER_NAME("maxSteering", "", true))
ADD_INTROS_ITEM_USER_NAME(wheelDiameter, MAKE_USER_NAME("wheelDiameter", "", true))
ADD_INTROS_ITEM_USER_NAME(trackWidth, MAKE_USER_NAME("trackWidth", "", true))
ADD_INTROS_ITEM_USER_NAME(positionX, MAKE_USER_NAME("positionX", "", true))
ADD_INTROS_ITEM_USER_NAME(positionZ, MAKE_USER_NAME("positionZ", "", true))
END_INTROS_TYPE(xmlAxle)

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

BEGIN_INTROS_TYPE_USER_NAME(xmlPedestrian, "Pedestrian")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(pedestrianCategory, MAKE_USER_NAME("pedestrianCategory", "", true))
ADD_INTROS_ITEM_USER_NAME(BoundingBox, MAKE_USER_NAME("BoundingBox", "", false))
ADD_INTROS_ITEM_USER_NAME(Properties, MAKE_USER_NAME("Properties", "", false))
END_INTROS_TYPE(xmlPedestrian)

BEGIN_INTROS_TYPE_USER_NAME(xmlMiscObject, "MiscObject")
ADD_INTROS_ITEM_USER_NAME(mass, MAKE_USER_NAME("mass", "", true))
ADD_INTROS_ITEM_USER_NAME(miscObjectCategory, MAKE_USER_NAME("miscObjectCategory", "", true))
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(BoundingBox, MAKE_USER_NAME("BoundingBox", "", false))
ADD_INTROS_ITEM_USER_NAME(Properties, MAKE_USER_NAME("Properties", "", false))
END_INTROS_TYPE(xmlMiscObject)

BEGIN_INTROS_TYPE_USER_NAME(xmlCatalogReference, "CatalogReference")
ADD_INTROS_ITEM_USER_NAME(catalogName, MAKE_USER_NAME("catalogName", "", true))
ADD_INTROS_ITEM_USER_NAME(entryName, MAKE_USER_NAME("entryName", "", true))
END_INTROS_TYPE(xmlCatalogReference)

BEGIN_INTROS_TYPE_USER_NAME(xmlScenarioObject, "ScenarioObject")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(Vehicle, MAKE_USER_NAME("Vehicle", "", false))
ADD_INTROS_ITEM_USER_NAME(Pedestrian, MAKE_USER_NAME("Pedestrian", "", false))
ADD_INTROS_ITEM_USER_NAME(MiscObject, MAKE_USER_NAME("MiscObject", "", false))
ADD_INTROS_ITEM_USER_NAME(CatalogReference, MAKE_USER_NAME("CatalogReference", "", false))
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

BEGIN_INTROS_TYPE_USER_NAME(xmlWorldPosition, "WorldPosition")
ADD_INTROS_ITEM_USER_NAME(h, MAKE_USER_NAME("h", "", true))
ADD_INTROS_ITEM_USER_NAME(p, MAKE_USER_NAME("p", "", true))
ADD_INTROS_ITEM_USER_NAME(r, MAKE_USER_NAME("r", "", true))
ADD_INTROS_ITEM_USER_NAME(x, MAKE_USER_NAME("x", "", true))
ADD_INTROS_ITEM_USER_NAME(y, MAKE_USER_NAME("y", "", true))
ADD_INTROS_ITEM_USER_NAME(z, MAKE_USER_NAME("z", "", true))
END_INTROS_TYPE(xmlWorldPosition)

BEGIN_INTROS_TYPE_USER_NAME(xmlPosition, "Position")
ADD_INTROS_ITEM_USER_NAME(WorldPosition, MAKE_USER_NAME("WorldPosition", "", false))
END_INTROS_TYPE(xmlPosition)

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

BEGIN_INTROS_TYPE_USER_NAME(xmlNone, "None")
END_INTROS_TYPE(xmlNone)

BEGIN_INTROS_TYPE_USER_NAME(xmlTimeReference, "TimeReference")
ADD_INTROS_ITEM_USER_NAME(None, MAKE_USER_NAME("None", "", false))
END_INTROS_TYPE(xmlTimeReference)

BEGIN_INTROS_TYPE_USER_NAME(xmlTrajectoryFollowingMode, "TrajectoryFollowingMode")
ADD_INTROS_ITEM_USER_NAME(followingMode, MAKE_USER_NAME("followingMode", "", true))
END_INTROS_TYPE(xmlTrajectoryFollowingMode)

BEGIN_INTROS_TYPE_USER_NAME(xmlFollowTrajectoryAction, "FollowTrajectoryAction")
ADD_INTROS_ITEM_USER_NAME(Trajectory, MAKE_USER_NAME("Trajectory", "", false))
ADD_INTROS_ITEM_USER_NAME(TimeReference, MAKE_USER_NAME("TimeReference", "", false))
ADD_INTROS_ITEM_USER_NAME(TrajectoryFollowingMode, MAKE_USER_NAME("TrajectoryFollowingMode", "", false))
END_INTROS_TYPE(xmlFollowTrajectoryAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlAcquirePositionAction, "AcquirePositionAction")
ADD_INTROS_ITEM_USER_NAME(Position, MAKE_USER_NAME("Position", "", false))
END_INTROS_TYPE(xmlAcquirePositionAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlRoutingAction, "RoutingAction")
ADD_INTROS_ITEM_USER_NAME(AcquirePositionAction, MAKE_USER_NAME("AcquirePositionAction", "", false))
ADD_INTROS_ITEM_USER_NAME(FollowTrajectoryAction, MAKE_USER_NAME("FollowTrajectoryAction", "", false))
END_INTROS_TYPE(xmlRoutingAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlAbsoluteTargetSpeed, "AbsoluteTargetSpeed")
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
END_INTROS_TYPE(xmlAbsoluteTargetSpeed)

BEGIN_INTROS_TYPE_USER_NAME(xmlSpeedActionDynamics, "SpeedActionDynamics")
ADD_INTROS_ITEM_USER_NAME(dynamicsShape, MAKE_USER_NAME("dynamicsShape", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
ADD_INTROS_ITEM_USER_NAME(dynamicsDimension, MAKE_USER_NAME("dynamicsDimension", "", true))
END_INTROS_TYPE(xmlSpeedActionDynamics)

BEGIN_INTROS_TYPE_USER_NAME(xmlSpeedActionTarget, "SpeedActionTarget")
ADD_INTROS_ITEM_USER_NAME(AbsoluteTargetSpeed, MAKE_USER_NAME("AbsoluteTargetSpeed", "", false))
END_INTROS_TYPE(xmlSpeedActionTarget)

BEGIN_INTROS_TYPE_USER_NAME(xmlSpeedAction, "SpeedAction")
ADD_INTROS_ITEM_USER_NAME(SpeedActionDynamics, MAKE_USER_NAME("SpeedActionDynamics", "", false))
ADD_INTROS_ITEM_USER_NAME(SpeedActionTarget, MAKE_USER_NAME("SpeedActionTarget", "", false))
END_INTROS_TYPE(xmlSpeedAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlLongitudinalAction, "LongitudinalAction")
ADD_INTROS_ITEM_USER_NAME(SpeedAction, MAKE_USER_NAME("SpeedAction", "", false))
END_INTROS_TYPE(xmlLongitudinalAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction, "PrivateAction")
ADD_INTROS_ITEM_USER_NAME(RoutingAction, MAKE_USER_NAME("RoutingAction", "", false))
ADD_INTROS_ITEM_USER_NAME(TeleportAction, MAKE_USER_NAME("TeleportAction", "", false))
ADD_INTROS_ITEM_USER_NAME(LongitudinalAction, MAKE_USER_NAME("LongitudinalAction", "", false))
END_INTROS_TYPE(xmlPrivateAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlAction, "Action")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(PrivateAction, MAKE_USER_NAME("PrivateAction", "", false))
END_INTROS_TYPE(xmlAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlSimulationTimeCondition, "SimulationTimeCondition")
ADD_INTROS_ITEM_USER_NAME(rule, MAKE_USER_NAME("rule", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
END_INTROS_TYPE(xmlSimulationTimeCondition)

BEGIN_INTROS_TYPE_USER_NAME(xmlByValueCondition, "ByValueCondition")
ADD_INTROS_ITEM_USER_NAME(SimulationTimeCondition, MAKE_USER_NAME("SimulationTimeCondition", "", false))
END_INTROS_TYPE(xmlByValueCondition)

BEGIN_INTROS_TYPE_USER_NAME(xmlCondition, "Condition")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(delay, MAKE_USER_NAME("delay", "", true))
ADD_INTROS_ITEM_USER_NAME(conditionEdge, MAKE_USER_NAME("conditionEdge", "", true))
ADD_INTROS_ITEM_USER_NAME(ByValueCondition, MAKE_USER_NAME("ByValueCondition", "", false))
END_INTROS_TYPE(xmlCondition)

BEGIN_INTROS_TYPE_USER_NAME(xmlConditionGroup, "ConditionGroup")
ADD_INTROS_ITEM_USER_NAME(Condition, MAKE_USER_NAME("Condition", "", false))
END_INTROS_TYPE(xmlConditionGroup)

BEGIN_INTROS_TYPE_USER_NAME(xmlStartTrigger, "StartTrigger")
ADD_INTROS_ITEM_USER_NAME(ConditionGroup, MAKE_USER_NAME("ConditionGroup", "", false))
END_INTROS_TYPE(xmlStartTrigger)

BEGIN_INTROS_TYPE_USER_NAME(xmlEvent, "Event")
ADD_INTROS_ITEM_USER_NAME(maximumExecutionCount, MAKE_USER_NAME("maximumExecutionCount", "", true))
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(priority, MAKE_USER_NAME("priority", "", true))
ADD_INTROS_ITEM_USER_NAME(Action, MAKE_USER_NAME("Action", "", false))
ADD_INTROS_ITEM_USER_NAME(StartTrigger, MAKE_USER_NAME("StartTrigger", "", false))
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
ADD_INTROS_ITEM_USER_NAME(StartTrigger, MAKE_USER_NAME("StartTrigger", "", false))
END_INTROS_TYPE(xmlAct)

BEGIN_INTROS_TYPE_USER_NAME(xmlStory, "Story")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(ParameterDeclarations, MAKE_USER_NAME("ParameterDeclarations", "", false))
ADD_INTROS_ITEM_USER_NAME(Act, MAKE_USER_NAME("Act", "", false))
END_INTROS_TYPE(xmlStory)

BEGIN_INTROS_TYPE_USER_NAME(xmlStopTrigger, "StopTrigger")
END_INTROS_TYPE(xmlStopTrigger)

BEGIN_INTROS_TYPE_USER_NAME(xmlStoryboard, "Storyboard")
ADD_INTROS_ITEM_USER_NAME(Init, MAKE_USER_NAME("Init", "", false))
ADD_INTROS_ITEM_USER_NAME(Story, MAKE_USER_NAME("Story", "", false))
ADD_INTROS_ITEM_USER_NAME(StopTrigger, MAKE_USER_NAME("StopTrigger", "", false))
END_INTROS_TYPE(xmlStoryboard)

BEGIN_INTROS_TYPE_USER_NAME(xmlDirectory, "Directory")
ADD_INTROS_ITEM_USER_NAME(path, MAKE_USER_NAME("path", "", true))
END_INTROS_TYPE(xmlDirectory)

BEGIN_INTROS_TYPE_USER_NAME(xmlVehicleCatalog, "VehicleCatalog")
ADD_INTROS_ITEM_USER_NAME(Directory, MAKE_USER_NAME("Directory", "", false))
END_INTROS_TYPE(xmlVehicleCatalog)

BEGIN_INTROS_TYPE_USER_NAME(xmlPedestrianCatalog, "PedestrianCatalog")
ADD_INTROS_ITEM_USER_NAME(Directory, MAKE_USER_NAME("Directory", "", false))
END_INTROS_TYPE(xmlPedestrianCatalog)

BEGIN_INTROS_TYPE_USER_NAME(xmlMiscObjectCatalog, "MiscObjectCatalog")
ADD_INTROS_ITEM_USER_NAME(Directory, MAKE_USER_NAME("Directory", "", false))
END_INTROS_TYPE(xmlMiscObjectCatalog)

BEGIN_INTROS_TYPE_USER_NAME(xmlCatalogLocations, "CatalogLocations")
ADD_INTROS_ITEM_USER_NAME(VehicleCatalog, MAKE_USER_NAME("VehicleCatalog", "", false))
ADD_INTROS_ITEM_USER_NAME(PedestrianCatalog, MAKE_USER_NAME("PedestrianCatalog", "", false))
ADD_INTROS_ITEM_USER_NAME(MiscObjectCatalog, MAKE_USER_NAME("MiscObjectCatalog", "", false))
END_INTROS_TYPE(xmlCatalogLocations)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO, "OpenSCENARIO")
ADD_INTROS_ITEM_USER_NAME(FileHeader, MAKE_USER_NAME("FileHeader", "", false))
ADD_INTROS_ITEM_USER_NAME(ParameterDeclarations, MAKE_USER_NAME("ParameterDeclarations", "", false))
ADD_INTROS_ITEM_USER_NAME(CatalogLocations, MAKE_USER_NAME("CatalogLocations", "", false))
ADD_INTROS_ITEM_USER_NAME(RoadNetwork, MAKE_USER_NAME("RoadNetwork", "", false))
ADD_INTROS_ITEM_USER_NAME(Entities, MAKE_USER_NAME("ScenarioObject", "Entities", false))
ADD_INTROS_ITEM_USER_NAME(Storyboard, MAKE_USER_NAME("Storyboard", "", false))
END_INTROS_TYPE(xmlOpenSCENARIO)

BEGIN_INTROS_TYPE_USER_NAME(xmlCatalog, "Catalog")
ADD_INTROS_ITEM_USER_NAME(MiscObject, MAKE_USER_NAME("MiscObject", "", false))
ADD_INTROS_ITEM_USER_NAME(Vehicle, MAKE_USER_NAME("Vehicle", "", false))
ADD_INTROS_ITEM_USER_NAME(Pedestrian, MAKE_USER_NAME("Pedestrian", "", false))
END_INTROS_TYPE(xmlCatalog)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenCatalog, "OpenSCENARIO")
ADD_INTROS_ITEM_USER_NAME(FileHeader, MAKE_USER_NAME("FileHeader", "", false))
ADD_INTROS_ITEM_USER_NAME(Catalog, MAKE_USER_NAME("Catalog", "", false))
END_INTROS_TYPE(xmlOpenCatalog)

OpenScenarioPtr Load(std::istream& stream) {
  if (!stream) {
    return nullptr;
  }
  ptree tree;
  read_xml(stream, tree);
  return std::make_shared<xmlOpenSCENARIO>(make_intros_object<xmlOpenSCENARIO>(tree));
}

OpenScenarioPtr Load(const std::string& file) {
  std::ifstream ifs(file);
  auto ptr = Load(ifs);
  ifs.close();
  return ptr;
}

OpenCatalogPtr LoadCatalog(std::istream& stream) {
  if (!stream) {
    return nullptr;
  }
  ptree tree;
  read_xml(stream, tree);
  return std::make_shared<xmlOpenCatalog>(make_intros_object<xmlOpenCatalog>(tree));
}

OpenCatalogPtr LoadCatalog(const std::string& file) {
  std::ifstream ifs(file);
  auto ptr = LoadCatalog(ifs);
  ifs.close();
  return ptr;
}

std::string Dump(const OpenScenarioPtr& xosc) TX_NOEXCEPT {
  if (!xosc) {
    return "";
  }

  std::ostringstream oss;
  ptree tree = make_ptree<xmlOpenSCENARIO>(*xosc);
  write_xml(oss, tree, xml_writer_settings<std::string>(' ', 2));
  return oss.str();
}

TX_NAMESPACE_CLOSE(OSC)
TX_NAMESPACE_CLOSE(SIM)