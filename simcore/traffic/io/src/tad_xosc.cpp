// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_xosc.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "tx_xml_utils.h"
#include "xml/intros_ptree.hpp"
#include "xml/util_traits.hpp"
using namespace utils::intros_ptree;
using namespace boost::property_tree;

TX_NAMESPACE_OPEN(SceneLoader)
TX_NAMESPACE_OPEN(OSC_TADSim)

BEGIN_INTROS_TYPE_USER_NAME(xmlEntityRef, "EntityRef")
ADD_INTROS_ITEM_USER_NAME(entityRef, MAKE_USER_NAME("entityRef", "", true))
END_INTROS_TYPE(xmlEntityRef)

/******************************************************************************************/
BEGIN_INTROS_TYPE_USER_NAME(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlEntityCondition::
                                xmlTimeToCollisionCondition::xmlTimeToCollisionConditionTarget,
                            "TimeToCollisionConditionTarget")
ADD_INTROS_ITEM_USER_NAME(EntityRef, MAKE_USER_NAME("EntityRef", "", false))
END_INTROS_TYPE(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlEntityCondition::xmlTimeToCollisionCondition::
                    xmlTimeToCollisionConditionTarget)

BEGIN_INTROS_TYPE_USER_NAME(
    xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlEntityCondition::xmlTimeToCollisionCondition,
    "TimeToCollisionCondition")
ADD_INTROS_ITEM_USER_NAME(alongRoute, MAKE_USER_NAME("alongRoute", "", true))
ADD_INTROS_ITEM_USER_NAME(freespace, MAKE_USER_NAME("freespace", "", true))
ADD_INTROS_ITEM_USER_NAME(rule, MAKE_USER_NAME("rule", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
ADD_INTROS_ITEM_USER_NAME(TimeToCollisionConditionTarget, MAKE_USER_NAME("TimeToCollisionConditionTarget", "", false))
END_INTROS_TYPE(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlEntityCondition::xmlTimeToCollisionCondition)

BEGIN_INTROS_TYPE_USER_NAME(
    xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlEntityCondition::xmlRelativeDistanceCondition,
    "RelativeDistanceCondition")
ADD_INTROS_ITEM_USER_NAME(entityRef, MAKE_USER_NAME("entityRef", "", true))
ADD_INTROS_ITEM_USER_NAME(freespace, MAKE_USER_NAME("freespace", "", true))
ADD_INTROS_ITEM_USER_NAME(relativeDistanceType, MAKE_USER_NAME("relativeDistanceType", "", true))
ADD_INTROS_ITEM_USER_NAME(rule, MAKE_USER_NAME("rule", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
END_INTROS_TYPE(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlEntityCondition::xmlRelativeDistanceCondition)

BEGIN_INTROS_TYPE_USER_NAME(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlEntityCondition, "EntityCondition")
ADD_INTROS_ITEM_USER_NAME(TimeToCollisionCondition, MAKE_USER_NAME("TimeToCollisionCondition", "", false))
ADD_INTROS_ITEM_USER_NAME(RelativeDistanceCondition, MAKE_USER_NAME("RelativeDistanceCondition", "", false))
END_INTROS_TYPE(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlEntityCondition)

BEGIN_INTROS_TYPE_USER_NAME(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlTriggeringEntities,
                            "TriggeringEntities")
ADD_INTROS_ITEM_USER_NAME(triggeringEntitiesRule, MAKE_USER_NAME("triggeringEntitiesRule", "", true))
ADD_INTROS_ITEM_USER_NAME(EntityRef, MAKE_USER_NAME("EntityRef", "", false))
END_INTROS_TYPE(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlTriggeringEntities)

BEGIN_INTROS_TYPE_USER_NAME(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlByEntityCondition, "ByEntityCondition")
ADD_INTROS_ITEM_USER_NAME(TriggeringEntities, MAKE_USER_NAME("TriggeringEntities", "", false))
ADD_INTROS_ITEM_USER_NAME(EntityCondition, MAKE_USER_NAME("EntityCondition", "", false))
END_INTROS_TYPE(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlByEntityCondition)

BEGIN_INTROS_TYPE_USER_NAME(
    xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlByValueCondition::xmlSimulationTimeCondition,
    "SimulationTimeCondition")
ADD_INTROS_ITEM_USER_NAME(rule, MAKE_USER_NAME("rule", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
END_INTROS_TYPE(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlByValueCondition::xmlSimulationTimeCondition)

BEGIN_INTROS_TYPE_USER_NAME(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlByValueCondition, "ByValueCondition")
ADD_INTROS_ITEM_USER_NAME(SimulationTimeCondition, MAKE_USER_NAME("SimulationTimeCondition", "", false))
END_INTROS_TYPE(xmlStartTrigger::xmlConditionGroup::xmlCondition::xmlByValueCondition)

BEGIN_INTROS_TYPE_USER_NAME(xmlStartTrigger::xmlConditionGroup::xmlCondition, "Condition")
ADD_INTROS_ITEM_USER_NAME(conditionEdge, MAKE_USER_NAME("conditionEdge", "", true))
ADD_INTROS_ITEM_USER_NAME(delay, MAKE_USER_NAME("delay", "", true))
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(ByValueCondition, MAKE_USER_NAME("ByValueCondition", "", false))
ADD_INTROS_ITEM_USER_NAME(ByEntityCondition, MAKE_USER_NAME("ByEntityCondition", "", false))
END_INTROS_TYPE(xmlStartTrigger::xmlConditionGroup::xmlCondition)

BEGIN_INTROS_TYPE_USER_NAME(xmlStartTrigger::xmlConditionGroup, "ConditionGroup")
ADD_INTROS_ITEM_USER_NAME(Condition, MAKE_USER_NAME("Condition", "", false))
END_INTROS_TYPE(xmlStartTrigger::xmlConditionGroup)

BEGIN_INTROS_TYPE_USER_NAME(xmlStartTrigger, "StartTrigger")
ADD_INTROS_ITEM_USER_NAME(ConditionGroup, MAKE_USER_NAME("ConditionGroup", "", false))
END_INTROS_TYPE(xmlStartTrigger)

BEGIN_INTROS_TYPE_USER_NAME(xmlPosition::xmlRelativeObjectPosition::xmlOrientation, "Orientation")
ADD_INTROS_ITEM_USER_NAME(h, MAKE_USER_NAME("h", "", true))
ADD_INTROS_ITEM_USER_NAME(type, MAKE_USER_NAME("type", "", true))
END_INTROS_TYPE(xmlPosition::xmlRelativeObjectPosition::xmlOrientation)

BEGIN_INTROS_TYPE_USER_NAME(xmlPosition::xmlRelativeObjectPosition, "RelativeObjectPosition")
ADD_INTROS_ITEM_USER_NAME(dx, MAKE_USER_NAME("dx", "", true))
ADD_INTROS_ITEM_USER_NAME(dy, MAKE_USER_NAME("dy", "", true))
ADD_INTROS_ITEM_USER_NAME(entityRef, MAKE_USER_NAME("entityRef", "", true))
ADD_INTROS_ITEM_USER_NAME(Orientation, MAKE_USER_NAME("Orientation", "", false))
END_INTROS_TYPE(xmlPosition::xmlRelativeObjectPosition)

BEGIN_INTROS_TYPE_USER_NAME(xmlPosition::xmlWorldPosition, "WorldPosition")
ADD_INTROS_ITEM_USER_NAME(x, MAKE_USER_NAME("x", "", true))
ADD_INTROS_ITEM_USER_NAME(y, MAKE_USER_NAME("y", "", true))
ADD_INTROS_ITEM_USER_NAME(z, MAKE_USER_NAME("z", "", true))
ADD_INTROS_ITEM_USER_NAME(h, MAKE_USER_NAME("h", "", true))
END_INTROS_TYPE(xmlPosition::xmlWorldPosition)

BEGIN_INTROS_TYPE_USER_NAME(xmlPosition, "Position")
ADD_INTROS_ITEM_USER_NAME(WorldPosition, MAKE_USER_NAME("WorldPosition", "", false))
ADD_INTROS_ITEM_USER_NAME(RelativeObjectPosition, MAKE_USER_NAME("RelativeObjectPosition", "", false))
END_INTROS_TYPE(xmlPosition)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlRoutingAction::xmlAssignRouteAction::xmlRoute::xmlWaypoint, "Waypoint")
ADD_INTROS_ITEM_USER_NAME(routeStrategy, MAKE_USER_NAME("routeStrategy", "", true))
ADD_INTROS_ITEM_USER_NAME(Position, MAKE_USER_NAME("Position", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlRoutingAction::xmlAssignRouteAction::xmlRoute::xmlWaypoint)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlRoutingAction::xmlAssignRouteAction::xmlRoute, "Route")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(closed, MAKE_USER_NAME("closed", "", true))
ADD_INTROS_ITEM_USER_NAME(Waypoint, MAKE_USER_NAME("Waypoint", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlRoutingAction::xmlAssignRouteAction::xmlRoute)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlRoutingAction::xmlAssignRouteAction, "AssignRouteAction")
ADD_INTROS_ITEM_USER_NAME(Route, MAKE_USER_NAME("Route", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlRoutingAction::xmlAssignRouteAction)

BEGIN_INTROS_TYPE_USER_NAME(
    xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction::xmlTrajectory::xmlShape::xmlPolyline::xmlVertex,
    "Vertex")
ADD_INTROS_ITEM_USER_NAME(time, MAKE_USER_NAME("time", "", true))
ADD_INTROS_ITEM_USER_NAME(Position, MAKE_USER_NAME("Position", "", false))
END_INTROS_TYPE(
    xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction::xmlTrajectory::xmlShape::xmlPolyline::xmlVertex)

BEGIN_INTROS_TYPE_USER_NAME(
    xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction::xmlTrajectory::xmlShape::xmlPolyline, "Polyline")
ADD_INTROS_ITEM_USER_NAME(Vertexs, MAKE_USER_NAME("Vertex", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction::xmlTrajectory::xmlShape::xmlPolyline)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction::xmlTrajectory::xmlShape,
                            "Shape")
ADD_INTROS_ITEM_USER_NAME(Polyline, MAKE_USER_NAME("Polyline", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction::xmlTrajectory::xmlShape)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction::xmlTrajectory, "Trajectory")
ADD_INTROS_ITEM_USER_NAME(closed, MAKE_USER_NAME("closed", "", true))
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(Shape, MAKE_USER_NAME("Shape", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction::xmlTrajectory)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction::xmlTrajectoryFollowingMode,
                            "TrajectoryFollowingMode")
ADD_INTROS_ITEM_USER_NAME(followingMode, MAKE_USER_NAME("followingMode", "", true))
END_INTROS_TYPE(xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction::xmlTrajectoryFollowingMode)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction, "FollowTrajectoryAction")
ADD_INTROS_ITEM_USER_NAME(Trajectory, MAKE_USER_NAME("Trajectory", "", false))
ADD_INTROS_ITEM_USER_NAME(TrajectoryFollowingMode, MAKE_USER_NAME("TrajectoryFollowingMode", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlRoutingAction::xmlFollowTrajectoryAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlRoutingAction, "RoutingAction")
ADD_INTROS_ITEM_USER_NAME(AssignRouteAction, MAKE_USER_NAME("AssignRouteAction", "", false))
ADD_INTROS_ITEM_USER_NAME(FollowTrajectoryAction, MAKE_USER_NAME("FollowTrajectoryAction", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlRoutingAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlLongitudinalAction::xmlSpeedAction::xmlSpeedActionDynamics,
                            "SpeedActionDynamics")
ADD_INTROS_ITEM_USER_NAME(dynamicsShape, MAKE_USER_NAME("dynamicsShape", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
ADD_INTROS_ITEM_USER_NAME(dynamicsDimension, MAKE_USER_NAME("dynamicsDimension", "", true))
END_INTROS_TYPE(xmlPrivateAction::xmlLongitudinalAction::xmlSpeedAction::xmlSpeedActionDynamics)

BEGIN_INTROS_TYPE_USER_NAME(
    xmlPrivateAction::xmlLongitudinalAction::xmlSpeedAction::xmlSpeedActionTarget::xmlAbsoluteTargetSpeed,
    "AbsoluteTargetSpeed")
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
END_INTROS_TYPE(xmlPrivateAction::xmlLongitudinalAction::xmlSpeedAction::xmlSpeedActionTarget::xmlAbsoluteTargetSpeed)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlLongitudinalAction::xmlSpeedAction::xmlSpeedActionTarget,
                            "SpeedActionTarget")
ADD_INTROS_ITEM_USER_NAME(AbsoluteTargetSpeed, MAKE_USER_NAME("AbsoluteTargetSpeed", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlLongitudinalAction::xmlSpeedAction::xmlSpeedActionTarget)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlLongitudinalAction::xmlSpeedAction, "SpeedAction")
ADD_INTROS_ITEM_USER_NAME(SpeedActionDynamics, MAKE_USER_NAME("SpeedActionDynamics", "", false))
ADD_INTROS_ITEM_USER_NAME(SpeedActionTarget, MAKE_USER_NAME("SpeedActionTarget", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlLongitudinalAction::xmlSpeedAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlLongitudinalAction, "LongitudinalAction")
ADD_INTROS_ITEM_USER_NAME(SpeedAction, MAKE_USER_NAME("SpeedAction", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlLongitudinalAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlTeleportAction::xmlPosition::xmlWorldPosition, "WorldPosition")
ADD_INTROS_ITEM_USER_NAME(x, MAKE_USER_NAME("x", "", true))
ADD_INTROS_ITEM_USER_NAME(y, MAKE_USER_NAME("y", "", true))
ADD_INTROS_ITEM_USER_NAME(z, MAKE_USER_NAME("z", "", true))
ADD_INTROS_ITEM_USER_NAME(h, MAKE_USER_NAME("h", "", true))
END_INTROS_TYPE(xmlPrivateAction::xmlTeleportAction::xmlPosition::xmlWorldPosition)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlTeleportAction::xmlPosition, "Position")
ADD_INTROS_ITEM_USER_NAME(WorldPosition, MAKE_USER_NAME("WorldPosition", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlTeleportAction::xmlPosition)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlTeleportAction, "TeleportAction")
ADD_INTROS_ITEM_USER_NAME(Position, MAKE_USER_NAME("Position", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlTeleportAction)

BEGIN_INTROS_TYPE_USER_NAME(
    xmlPrivateAction::xmlLateralAction::xmlLaneChangeAction::xmlLaneChangeTarget::xmlRelativeTargetLane,
    "RelativeTargetLane")
ADD_INTROS_ITEM_USER_NAME(entityRef, MAKE_USER_NAME("entityRef", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
END_INTROS_TYPE(xmlPrivateAction::xmlLateralAction::xmlLaneChangeAction::xmlLaneChangeTarget::xmlRelativeTargetLane)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlLateralAction::xmlLaneChangeAction::xmlLaneChangeTarget,
                            "LaneChangeTarget")
ADD_INTROS_ITEM_USER_NAME(RelativeTargetLane, MAKE_USER_NAME("RelativeTargetLane", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlLateralAction::xmlLaneChangeAction::xmlLaneChangeTarget)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlLateralAction::xmlLaneChangeAction::xmlLaneChangeActionDynamics,
                            "LaneChangeActionDynamics")
ADD_INTROS_ITEM_USER_NAME(dynamicsDimension, MAKE_USER_NAME("dynamicsDimension", "", true))
ADD_INTROS_ITEM_USER_NAME(dynamicsShape, MAKE_USER_NAME("dynamicsShape", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
END_INTROS_TYPE(xmlPrivateAction::xmlLateralAction::xmlLaneChangeAction::xmlLaneChangeActionDynamics)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlLateralAction::xmlLaneChangeAction, "LaneChangeAction")
ADD_INTROS_ITEM_USER_NAME(targetLaneOffset, MAKE_USER_NAME("targetLaneOffset", "", true))
ADD_INTROS_ITEM_USER_NAME(LaneChangeActionDynamics, MAKE_USER_NAME("LaneChangeActionDynamics", "", false))
ADD_INTROS_ITEM_USER_NAME(LaneChangeTarget, MAKE_USER_NAME("LaneChangeTarget", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlLateralAction::xmlLaneChangeAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction::xmlLateralAction, "LateralAction")
ADD_INTROS_ITEM_USER_NAME(LaneChangeAction, MAKE_USER_NAME("LaneChangeAction", "", false))
END_INTROS_TYPE(xmlPrivateAction::xmlLateralAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlPrivateAction, "PrivateAction")
ADD_INTROS_ITEM_USER_NAME(RoutingAction, MAKE_USER_NAME("RoutingAction", "", false))
ADD_INTROS_ITEM_USER_NAME(LongitudinalAction, MAKE_USER_NAME("LongitudinalAction", "", false))
ADD_INTROS_ITEM_USER_NAME(TeleportAction, MAKE_USER_NAME("TeleportAction", "", false))
ADD_INTROS_ITEM_USER_NAME(LateralAction, MAKE_USER_NAME("LateralAction", "", false))
END_INTROS_TYPE(xmlPrivateAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlManeuver::xmlEvent::xmlAction, "Action")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(PrivateAction, MAKE_USER_NAME("PrivateAction", "", false))
END_INTROS_TYPE(xmlManeuver::xmlEvent::xmlAction)

BEGIN_INTROS_TYPE_USER_NAME(xmlManeuver::xmlEvent, "Event")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(priority, MAKE_USER_NAME("priority", "", true))
ADD_INTROS_ITEM_USER_NAME(Action, MAKE_USER_NAME("Action", "", false))
ADD_INTROS_ITEM_USER_NAME(StartTrigger, MAKE_USER_NAME("StartTrigger", "", false))
END_INTROS_TYPE(xmlManeuver::xmlEvent)

BEGIN_INTROS_TYPE_USER_NAME(xmlStory::xmlAct::xmlManeuverGroup::xmlActors, "Actors")
ADD_INTROS_ITEM_USER_NAME(selectTriggeringEntities, MAKE_USER_NAME("selectTriggeringEntities", "", true))
ADD_INTROS_ITEM_USER_NAME(EntityRef, MAKE_USER_NAME("EntityRef", "", false))
END_INTROS_TYPE(xmlStory::xmlAct::xmlManeuverGroup::xmlActors)

BEGIN_INTROS_TYPE_USER_NAME(xmlManeuver, "Maneuver")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(Event, MAKE_USER_NAME("Event", "", false))
END_INTROS_TYPE(xmlManeuver)

BEGIN_INTROS_TYPE_USER_NAME(xmlStory::xmlAct::xmlManeuverGroup, "ManeuverGroup")
ADD_INTROS_ITEM_USER_NAME(maximumExecutionCount, MAKE_USER_NAME("maximumExecutionCount", "", true))
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(Actors, MAKE_USER_NAME("Actors", "", false))
ADD_INTROS_ITEM_USER_NAME(Maneuver, MAKE_USER_NAME("Maneuver", "", false))
END_INTROS_TYPE(xmlStory::xmlAct::xmlManeuverGroup)

BEGIN_INTROS_TYPE_USER_NAME(xmlStory::xmlAct, "Act")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(ManeuverGroup, MAKE_USER_NAME("ManeuverGroup", "", false))
ADD_INTROS_ITEM_USER_NAME(StartTrigger, MAKE_USER_NAME("StartTrigger", "", false))
END_INTROS_TYPE(xmlStory::xmlAct)

BEGIN_INTROS_TYPE_USER_NAME(xmlStory, "Story")
ADD_INTROS_ITEM_USER_NAME(Act, MAKE_USER_NAME("Act", "", false))
END_INTROS_TYPE(xmlStory)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO::xmlStoryboard::xmlInit::xmlActions::xmlPrivate, "Private")
ADD_INTROS_ITEM_USER_NAME(entityRef, MAKE_USER_NAME("entityRef", "", true))
ADD_INTROS_ITEM_USER_NAME(PrivateAction, MAKE_USER_NAME("PrivateAction", "", false))
END_INTROS_TYPE(xmlOpenSCENARIO::xmlStoryboard::xmlInit::xmlActions::xmlPrivate)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO::xmlStoryboard::xmlInit::xmlActions, "Actions")
ADD_INTROS_ITEM_USER_NAME(Private, MAKE_USER_NAME("Private", "", false))
END_INTROS_TYPE(xmlOpenSCENARIO::xmlStoryboard::xmlInit::xmlActions)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO::xmlStoryboard::xmlInit, "Init")
ADD_INTROS_ITEM_USER_NAME(Actions, MAKE_USER_NAME("Actions", "", false))
END_INTROS_TYPE(xmlOpenSCENARIO::xmlStoryboard::xmlInit)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO::xmlStoryboard, "Storyboard")
ADD_INTROS_ITEM_USER_NAME(Init, MAKE_USER_NAME("Init", "", false))
ADD_INTROS_ITEM_USER_NAME(Story, MAKE_USER_NAME("Story", "", false))
END_INTROS_TYPE(xmlOpenSCENARIO::xmlStoryboard)

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

BEGIN_INTROS_TYPE_USER_NAME(xmlVehicle, "Vehicle")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(vehicleCategory, MAKE_USER_NAME("vehicleCategory", "", true))
ADD_INTROS_ITEM_USER_NAME(BoundingBox, MAKE_USER_NAME("BoundingBox", "", false))
ADD_INTROS_ITEM_USER_NAME(Performance, MAKE_USER_NAME("Performance", "", false))
ADD_INTROS_ITEM_USER_NAME(Axles, MAKE_USER_NAME("Axles", "", false))
ADD_INTROS_ITEM_USER_NAME(Properties, MAKE_USER_NAME("Property", "Properties", false))
END_INTROS_TYPE(xmlVehicle)

BEGIN_INTROS_TYPE_USER_NAME(xmlMiscObject, "MiscObject")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(miscObjectCategory, MAKE_USER_NAME("miscObjectCategory", "", true))
ADD_INTROS_ITEM_USER_NAME(mass, MAKE_USER_NAME("mass", "", true))
ADD_INTROS_ITEM_USER_NAME(BoundingBox, MAKE_USER_NAME("BoundingBox", "", false))
END_INTROS_TYPE(xmlMiscObject)

BEGIN_INTROS_TYPE_USER_NAME(xmlPedestrian, "Pedestrian")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(pedestrianCategory, MAKE_USER_NAME("pedestrianCategory", "", true))
ADD_INTROS_ITEM_USER_NAME(mass, MAKE_USER_NAME("mass", "", true))
ADD_INTROS_ITEM_USER_NAME(model, MAKE_USER_NAME("model", "", true))
ADD_INTROS_ITEM_USER_NAME(BoundingBox, MAKE_USER_NAME("BoundingBox", "", false))
ADD_INTROS_ITEM_USER_NAME(Properties, MAKE_USER_NAME("Property", "Properties", false))
END_INTROS_TYPE(xmlPedestrian)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO::xmlScenarioObject, "ScenarioObject")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(Vehicle, MAKE_USER_NAME("Vehicle", "", false))
ADD_INTROS_ITEM_USER_NAME(Obstacle, MAKE_USER_NAME("MiscObject", "", false))
ADD_INTROS_ITEM_USER_NAME(Pedestrian, MAKE_USER_NAME("Pedestrian", "", false))
END_INTROS_TYPE(xmlOpenSCENARIO::xmlScenarioObject)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO::xmlRoadNetwork::xmlLogicFile, "LogicFile")
ADD_INTROS_ITEM_USER_NAME(filepath, MAKE_USER_NAME("filepath", "", true))
END_INTROS_TYPE(xmlOpenSCENARIO::xmlRoadNetwork::xmlLogicFile)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO::xmlRoadNetwork, "RoadNetwork")
ADD_INTROS_ITEM_USER_NAME(LogicFile, "LogicFile")
END_INTROS_TYPE(xmlOpenSCENARIO::xmlRoadNetwork)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO::xmlParameterDeclarations::xmlParameterDeclaration, "ParameterDeclaration")
ADD_INTROS_ITEM_USER_NAME(parameterType, MAKE_USER_NAME("parameterType", "", true))
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
END_INTROS_TYPE(xmlOpenSCENARIO::xmlParameterDeclarations::xmlParameterDeclaration)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO::xmlParameterDeclarations, "ParameterDeclarations")
ADD_INTROS_ITEM_USER_NAME(ParameterDeclaration, MAKE_USER_NAME("ParameterDeclaration", "", false))
END_INTROS_TYPE(xmlOpenSCENARIO::xmlParameterDeclarations)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO::xmlFileHeader, "FileHeader")
ADD_INTROS_ITEM_USER_NAME(description, MAKE_USER_NAME("description", "", true))
ADD_INTROS_ITEM_USER_NAME(date, MAKE_USER_NAME("date", "", true))
ADD_INTROS_ITEM_USER_NAME(author, MAKE_USER_NAME("author", "", true))
ADD_INTROS_ITEM_USER_NAME(revMajor, MAKE_USER_NAME("revMajor", "", true))
ADD_INTROS_ITEM_USER_NAME(revMinor, MAKE_USER_NAME("revMinor", "", true))
END_INTROS_TYPE(xmlOpenSCENARIO::xmlFileHeader)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenSCENARIO, "OpenSCENARIO")
ADD_INTROS_ITEM_USER_NAME(FileHeader, MAKE_USER_NAME("FileHeader", "", false))
ADD_INTROS_ITEM_USER_NAME(ParameterDeclarations, MAKE_USER_NAME("ParameterDeclarations", "", false))
ADD_INTROS_ITEM_USER_NAME(RoadNetwork, MAKE_USER_NAME("RoadNetwork", "", false))
ADD_INTROS_ITEM_USER_NAME(Entities, MAKE_USER_NAME("ScenarioObject", "Entities", false))
ADD_INTROS_ITEM_USER_NAME(Storyboard, MAKE_USER_NAME("Storyboard", "", false))
END_INTROS_TYPE(xmlOpenSCENARIO)

OpenSCENARIO_ptr load_scene_tadsim_xosc(const Base::txString& _traffic_path) TX_NOEXCEPT {
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
    LOG(WARNING) << "Load Xml File Failure. " << _traffic_path << std::endl;
    return nullptr;
  }
}

TX_NAMESPACE_CLOSE(OSC_TADSim)
TX_NAMESPACE_CLOSE(SceneLoader)
