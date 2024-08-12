

#include "openscenario.hpp"

namespace osc {

bool __parse__AbsoluteSpeed(const tinyxml2::XMLElement *elem, AbsoluteSpeed &obj) {

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__AbsoluteTargetLane(const tinyxml2::XMLElement *elem, AbsoluteTargetLane &obj) {

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__AbsoluteTargetLaneOffset(const tinyxml2::XMLElement *elem, AbsoluteTargetLaneOffset &obj) {

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__AbsoluteTargetSpeed(const tinyxml2::XMLElement *elem, AbsoluteTargetSpeed &obj) {

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__AccelerationCondition(const tinyxml2::XMLElement *elem, AccelerationCondition &obj) {

	if(elem->Attribute("rule") != nullptr)
		obj._rule = std::string(elem->Attribute("rule"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__AcquirePositionAction(const tinyxml2::XMLElement *elem, AcquirePositionAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));



	return true;

}

bool __parse__Act(const tinyxml2::XMLElement *elem, Act &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ManeuverGroup");
	while(e != nullptr) {

		obj.sub_ManeuverGroup.emplace_back();
		__parse__ManeuverGroup(e, obj.sub_ManeuverGroup.back());

		e = e->NextSiblingElement("ManeuverGroup");
	}

	e = elem->FirstChildElement("StartTrigger");
	if(e != nullptr)
		__parse__Trigger(e, *(obj.sub_StartTrigger.create()));

	e = elem->FirstChildElement("StopTrigger");
	if(e != nullptr)
		__parse__Trigger(e, *(obj.sub_StopTrigger.create()));



	return true;

}

bool __parse__Action(const tinyxml2::XMLElement *elem, Action &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("GlobalAction");
	if(e != nullptr)
		__parse__GlobalAction(e, *(obj.sub_GlobalAction.create()));

	e = elem->FirstChildElement("PrivateAction");
	if(e != nullptr)
		__parse__PrivateAction(e, *(obj.sub_PrivateAction.create()));

	e = elem->FirstChildElement("UserDefinedAction");
	if(e != nullptr)
		__parse__UserDefinedAction(e, *(obj.sub_UserDefinedAction.create()));



	return true;

}

bool __parse__ActivateControllerAction(const tinyxml2::XMLElement *elem, ActivateControllerAction &obj) {

	if(elem->Attribute("lateral") != nullptr)
		obj._lateral = std::string(elem->Attribute("lateral"));

	if(elem->Attribute("longitudinal") != nullptr)
		obj._longitudinal = std::string(elem->Attribute("longitudinal"));



	return true;

}

bool __parse__Actors(const tinyxml2::XMLElement *elem, Actors &obj) {

	if(elem->Attribute("selectTriggeringEntities") != nullptr)
		obj._selectTriggeringEntities = std::string(elem->Attribute("selectTriggeringEntities"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("EntityRef");
	while(e != nullptr) {

		obj.sub_EntityRef.emplace_back();
		__parse__EntityRef(e, obj.sub_EntityRef.back());

		e = e->NextSiblingElement("EntityRef");
	}



	return true;

}

bool __parse__AddEntityAction(const tinyxml2::XMLElement *elem, AddEntityAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));



	return true;

}

bool __parse__AssignControllerAction(const tinyxml2::XMLElement *elem, AssignControllerAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("CatalogReference");
	if(e != nullptr)
		__parse__CatalogReference(e, *(obj.sub_CatalogReference.create()));

	e = elem->FirstChildElement("Controller");
	if(e != nullptr)
		__parse__Controller(e, *(obj.sub_Controller.create()));



	return true;

}

bool __parse__AssignRouteAction(const tinyxml2::XMLElement *elem, AssignRouteAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("CatalogReference");
	if(e != nullptr)
		__parse__CatalogReference(e, *(obj.sub_CatalogReference.create()));

	e = elem->FirstChildElement("Route");
	if(e != nullptr)
		__parse__Route(e, *(obj.sub_Route.create()));



	return true;

}

bool __parse__Axle(const tinyxml2::XMLElement *elem, Axle &obj) {

	if(elem->Attribute("maxSteering") != nullptr)
		obj._maxSteering = std::string(elem->Attribute("maxSteering"));

	if(elem->Attribute("positionX") != nullptr)
		obj._positionX = std::string(elem->Attribute("positionX"));

	if(elem->Attribute("positionZ") != nullptr)
		obj._positionZ = std::string(elem->Attribute("positionZ"));

	if(elem->Attribute("trackWidth") != nullptr)
		obj._trackWidth = std::string(elem->Attribute("trackWidth"));

	if(elem->Attribute("wheelDiameter") != nullptr)
		obj._wheelDiameter = std::string(elem->Attribute("wheelDiameter"));



	return true;

}

bool __parse__Axles(const tinyxml2::XMLElement *elem, Axles &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("AdditionalAxle");
	while(e != nullptr) {

		obj.sub_AdditionalAxle.emplace_back();
		__parse__Axle(e, obj.sub_AdditionalAxle.back());

		e = e->NextSiblingElement("AdditionalAxle");
	}

	e = elem->FirstChildElement("FrontAxle");
	if(e != nullptr)
		__parse__Axle(e, *(obj.sub_FrontAxle.create()));

	e = elem->FirstChildElement("RearAxle");
	if(e != nullptr)
		__parse__Axle(e, *(obj.sub_RearAxle.create()));



	return true;

}

bool __parse__BoundingBox(const tinyxml2::XMLElement *elem, BoundingBox &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Center");
	if(e != nullptr)
		__parse__Center(e, *(obj.sub_Center.create()));

	e = elem->FirstChildElement("Dimensions");
	if(e != nullptr)
		__parse__Dimensions(e, *(obj.sub_Dimensions.create()));



	return true;

}

bool __parse__ByEntityCondition(const tinyxml2::XMLElement *elem, ByEntityCondition &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("EntityCondition");
	if(e != nullptr)
		__parse__EntityCondition(e, *(obj.sub_EntityCondition.create()));

	e = elem->FirstChildElement("TriggeringEntities");
	if(e != nullptr)
		__parse__TriggeringEntities(e, *(obj.sub_TriggeringEntities.create()));



	return true;

}

bool __parse__ByObjectType(const tinyxml2::XMLElement *elem, ByObjectType &obj) {

	if(elem->Attribute("type") != nullptr)
		obj._type = std::string(elem->Attribute("type"));



	return true;

}

bool __parse__ByType(const tinyxml2::XMLElement *elem, ByType &obj) {

	if(elem->Attribute("objectType") != nullptr)
		obj._objectType = std::string(elem->Attribute("objectType"));



	return true;

}

bool __parse__ByValueCondition(const tinyxml2::XMLElement *elem, ByValueCondition &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ParameterCondition");
	if(e != nullptr)
		__parse__ParameterCondition(e, *(obj.sub_ParameterCondition.create()));

	e = elem->FirstChildElement("SimulationTimeCondition");
	if(e != nullptr)
		__parse__SimulationTimeCondition(e, *(obj.sub_SimulationTimeCondition.create()));

	e = elem->FirstChildElement("StoryboardElementStateCondition");
	if(e != nullptr)
		__parse__StoryboardElementStateCondition(e, *(obj.sub_StoryboardElementStateCondition.create()));

	e = elem->FirstChildElement("TimeOfDayCondition");
	if(e != nullptr)
		__parse__TimeOfDayCondition(e, *(obj.sub_TimeOfDayCondition.create()));

	e = elem->FirstChildElement("TrafficSignalCondition");
	if(e != nullptr)
		__parse__TrafficSignalCondition(e, *(obj.sub_TrafficSignalCondition.create()));

	e = elem->FirstChildElement("TrafficSignalControllerCondition");
	if(e != nullptr)
		__parse__TrafficSignalControllerCondition(e, *(obj.sub_TrafficSignalControllerCondition.create()));

	e = elem->FirstChildElement("UserDefinedValueCondition");
	if(e != nullptr)
		__parse__UserDefinedValueCondition(e, *(obj.sub_UserDefinedValueCondition.create()));



	return true;

}

bool __parse__Catalog(const tinyxml2::XMLElement *elem, Catalog &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Controller");
	while(e != nullptr) {

		obj.sub_Controller.emplace_back();
		__parse__Controller(e, obj.sub_Controller.back());

		e = e->NextSiblingElement("Controller");
	}

	e = elem->FirstChildElement("Environment");
	while(e != nullptr) {

		obj.sub_Environment.emplace_back();
		__parse__Environment(e, obj.sub_Environment.back());

		e = e->NextSiblingElement("Environment");
	}

	e = elem->FirstChildElement("Maneuver");
	while(e != nullptr) {

		obj.sub_Maneuver.emplace_back();
		__parse__Maneuver(e, obj.sub_Maneuver.back());

		e = e->NextSiblingElement("Maneuver");
	}

	e = elem->FirstChildElement("MiscObject");
	while(e != nullptr) {

		obj.sub_MiscObject.emplace_back();
		__parse__MiscObject(e, obj.sub_MiscObject.back());

		e = e->NextSiblingElement("MiscObject");
	}

	e = elem->FirstChildElement("Pedestrian");
	while(e != nullptr) {

		obj.sub_Pedestrian.emplace_back();
		__parse__Pedestrian(e, obj.sub_Pedestrian.back());

		e = e->NextSiblingElement("Pedestrian");
	}

	e = elem->FirstChildElement("Route");
	while(e != nullptr) {

		obj.sub_Route.emplace_back();
		__parse__Route(e, obj.sub_Route.back());

		e = e->NextSiblingElement("Route");
	}

	e = elem->FirstChildElement("Trajectory");
	while(e != nullptr) {

		obj.sub_Trajectory.emplace_back();
		__parse__Trajectory(e, obj.sub_Trajectory.back());

		e = e->NextSiblingElement("Trajectory");
	}

	e = elem->FirstChildElement("Vehicle");
	while(e != nullptr) {

		obj.sub_Vehicle.emplace_back();
		__parse__Vehicle(e, obj.sub_Vehicle.back());

		e = e->NextSiblingElement("Vehicle");
	}



	return true;

}

bool __parse__CatalogLocations(const tinyxml2::XMLElement *elem, CatalogLocations &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ControllerCatalog");
	if(e != nullptr)
		__parse__ControllerCatalogLocation(e, *(obj.sub_ControllerCatalog.create()));

	e = elem->FirstChildElement("EnvironmentCatalog");
	if(e != nullptr)
		__parse__EnvironmentCatalogLocation(e, *(obj.sub_EnvironmentCatalog.create()));

	e = elem->FirstChildElement("ManeuverCatalog");
	if(e != nullptr)
		__parse__ManeuverCatalogLocation(e, *(obj.sub_ManeuverCatalog.create()));

	e = elem->FirstChildElement("MiscObjectCatalog");
	if(e != nullptr)
		__parse__MiscObjectCatalogLocation(e, *(obj.sub_MiscObjectCatalog.create()));

	e = elem->FirstChildElement("PedestrianCatalog");
	if(e != nullptr)
		__parse__PedestrianCatalogLocation(e, *(obj.sub_PedestrianCatalog.create()));

	e = elem->FirstChildElement("RouteCatalog");
	if(e != nullptr)
		__parse__RouteCatalogLocation(e, *(obj.sub_RouteCatalog.create()));

	e = elem->FirstChildElement("TrajectoryCatalog");
	if(e != nullptr)
		__parse__TrajectoryCatalogLocation(e, *(obj.sub_TrajectoryCatalog.create()));

	e = elem->FirstChildElement("VehicleCatalog");
	if(e != nullptr)
		__parse__VehicleCatalogLocation(e, *(obj.sub_VehicleCatalog.create()));



	return true;

}

bool __parse__CatalogReference(const tinyxml2::XMLElement *elem, CatalogReference &obj) {

	if(elem->Attribute("catalogName") != nullptr)
		obj._catalogName = std::string(elem->Attribute("catalogName"));

	if(elem->Attribute("entryName") != nullptr)
		obj._entryName = std::string(elem->Attribute("entryName"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ParameterAssignments");
	if(e != nullptr)
		__parse__ParameterAssignments(e, *(obj.sub_ParameterAssignments.create()));



	return true;

}

bool __parse__Center(const tinyxml2::XMLElement *elem, Center &obj) {

	if(elem->Attribute("x") != nullptr)
		obj._x = std::string(elem->Attribute("x"));

	if(elem->Attribute("y") != nullptr)
		obj._y = std::string(elem->Attribute("y"));

	if(elem->Attribute("z") != nullptr)
		obj._z = std::string(elem->Attribute("z"));



	return true;

}

bool __parse__CentralSwarmObject(const tinyxml2::XMLElement *elem, CentralSwarmObject &obj) {

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));



	return true;

}

bool __parse__Clothoid(const tinyxml2::XMLElement *elem, Clothoid &obj) {

	if(elem->Attribute("curvature") != nullptr)
		obj._curvature = std::string(elem->Attribute("curvature"));

	if(elem->Attribute("curvatureDot") != nullptr)
		obj._curvatureDot = std::string(elem->Attribute("curvatureDot"));

	if(elem->Attribute("length") != nullptr)
		obj._length = std::string(elem->Attribute("length"));

	if(elem->Attribute("startTime") != nullptr)
		obj._startTime = std::string(elem->Attribute("startTime"));

	if(elem->Attribute("stopTime") != nullptr)
		obj._stopTime = std::string(elem->Attribute("stopTime"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));



	return true;

}

bool __parse__CollisionCondition(const tinyxml2::XMLElement *elem, CollisionCondition &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ByType");
	if(e != nullptr)
		__parse__ByObjectType(e, *(obj.sub_ByType.create()));

	e = elem->FirstChildElement("EntityRef");
	if(e != nullptr)
		__parse__EntityRef(e, *(obj.sub_EntityRef.create()));



	return true;

}

bool __parse__Condition(const tinyxml2::XMLElement *elem, Condition &obj) {

	if(elem->Attribute("conditionEdge") != nullptr)
		obj._conditionEdge = std::string(elem->Attribute("conditionEdge"));

	if(elem->Attribute("delay") != nullptr)
		obj._delay = std::string(elem->Attribute("delay"));

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ByEntityCondition");
	if(e != nullptr)
		__parse__ByEntityCondition(e, *(obj.sub_ByEntityCondition.create()));

	e = elem->FirstChildElement("ByValueCondition");
	if(e != nullptr)
		__parse__ByValueCondition(e, *(obj.sub_ByValueCondition.create()));



	return true;

}

bool __parse__ConditionGroup(const tinyxml2::XMLElement *elem, ConditionGroup &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Condition");
	while(e != nullptr) {

		obj.sub_Condition.emplace_back();
		__parse__Condition(e, obj.sub_Condition.back());

		e = e->NextSiblingElement("Condition");
	}



	return true;

}

bool __parse__Controller(const tinyxml2::XMLElement *elem, Controller &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ParameterDeclarations");
	if(e != nullptr)
		__parse__ParameterDeclarations(e, *(obj.sub_ParameterDeclarations.create()));

	e = elem->FirstChildElement("Properties");
	if(e != nullptr)
		__parse__Properties(e, *(obj.sub_Properties.create()));



	return true;

}

bool __parse__ControllerAction(const tinyxml2::XMLElement *elem, ControllerAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("AssignControllerAction");
	if(e != nullptr)
		__parse__AssignControllerAction(e, *(obj.sub_AssignControllerAction.create()));

	e = elem->FirstChildElement("OverrideControllerValueAction");
	if(e != nullptr)
		__parse__OverrideControllerValueAction(e, *(obj.sub_OverrideControllerValueAction.create()));



	return true;

}

bool __parse__ControllerCatalogLocation(const tinyxml2::XMLElement *elem, ControllerCatalogLocation &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Directory");
	if(e != nullptr)
		__parse__Directory(e, *(obj.sub_Directory.create()));



	return true;

}

bool __parse__ControllerDistribution(const tinyxml2::XMLElement *elem, ControllerDistribution &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ControllerDistributionEntry");
	while(e != nullptr) {

		obj.sub_ControllerDistributionEntry.emplace_back();
		__parse__ControllerDistributionEntry(e, obj.sub_ControllerDistributionEntry.back());

		e = e->NextSiblingElement("ControllerDistributionEntry");
	}



	return true;

}

bool __parse__ControllerDistributionEntry(const tinyxml2::XMLElement *elem, ControllerDistributionEntry &obj) {

	if(elem->Attribute("weight") != nullptr)
		obj._weight = std::string(elem->Attribute("weight"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("CatalogReference");
	if(e != nullptr)
		__parse__CatalogReference(e, *(obj.sub_CatalogReference.create()));

	e = elem->FirstChildElement("Controller");
	if(e != nullptr)
		__parse__Controller(e, *(obj.sub_Controller.create()));



	return true;

}

bool __parse__ControlPoint(const tinyxml2::XMLElement *elem, ControlPoint &obj) {

	if(elem->Attribute("time") != nullptr)
		obj._time = std::string(elem->Attribute("time"));

	if(elem->Attribute("weight") != nullptr)
		obj._weight = std::string(elem->Attribute("weight"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));



	return true;

}

bool __parse__CustomCommandAction(const tinyxml2::XMLElement *elem, CustomCommandAction &obj) {



	return true;

}

bool __parse__DeleteEntityAction(const tinyxml2::XMLElement *elem, DeleteEntityAction &obj) {



	return true;

}

bool __parse__Dimensions(const tinyxml2::XMLElement *elem, Dimensions &obj) {

	if(elem->Attribute("height") != nullptr)
		obj._height = std::string(elem->Attribute("height"));

	if(elem->Attribute("length") != nullptr)
		obj._length = std::string(elem->Attribute("length"));

	if(elem->Attribute("width") != nullptr)
		obj._width = std::string(elem->Attribute("width"));



	return true;

}

bool __parse__Directory(const tinyxml2::XMLElement *elem, Directory &obj) {

	if(elem->Attribute("path") != nullptr)
		obj._path = std::string(elem->Attribute("path"));



	return true;

}

bool __parse__DistanceCondition(const tinyxml2::XMLElement *elem, DistanceCondition &obj) {

	if(elem->Attribute("alongRoute") != nullptr)
		obj._alongRoute = std::string(elem->Attribute("alongRoute"));

	if(elem->Attribute("freespace") != nullptr)
		obj._freespace = std::string(elem->Attribute("freespace"));

	if(elem->Attribute("rule") != nullptr)
		obj._rule = std::string(elem->Attribute("rule"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));



	return true;

}

bool __parse__DynamicConstraints(const tinyxml2::XMLElement *elem, DynamicConstraints &obj) {

	if(elem->Attribute("maxAcceleration") != nullptr)
		obj._maxAcceleration = std::string(elem->Attribute("maxAcceleration"));

	if(elem->Attribute("maxDeceleration") != nullptr)
		obj._maxDeceleration = std::string(elem->Attribute("maxDeceleration"));

	if(elem->Attribute("maxSpeed") != nullptr)
		obj._maxSpeed = std::string(elem->Attribute("maxSpeed"));



	return true;

}

bool __parse__EndOfRoadCondition(const tinyxml2::XMLElement *elem, EndOfRoadCondition &obj) {

	if(elem->Attribute("duration") != nullptr)
		obj._duration = std::string(elem->Attribute("duration"));



	return true;

}

bool __parse__Entities(const tinyxml2::XMLElement *elem, Entities &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("EntitySelection");
	while(e != nullptr) {

		obj.sub_EntitySelection.emplace_back();
		__parse__EntitySelection(e, obj.sub_EntitySelection.back());

		e = e->NextSiblingElement("EntitySelection");
	}

	e = elem->FirstChildElement("ScenarioObject");
	while(e != nullptr) {

		obj.sub_ScenarioObject.emplace_back();
		__parse__ScenarioObject(e, obj.sub_ScenarioObject.back());

		e = e->NextSiblingElement("ScenarioObject");
	}



	return true;

}

bool __parse__EntityAction(const tinyxml2::XMLElement *elem, EntityAction &obj) {

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("AddEntityAction");
	if(e != nullptr)
		__parse__AddEntityAction(e, *(obj.sub_AddEntityAction.create()));

	e = elem->FirstChildElement("DeleteEntityAction");
	if(e != nullptr)
		__parse__DeleteEntityAction(e, *(obj.sub_DeleteEntityAction.create()));



	return true;

}

bool __parse__EntityCondition(const tinyxml2::XMLElement *elem, EntityCondition &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("AccelerationCondition");
	if(e != nullptr)
		__parse__AccelerationCondition(e, *(obj.sub_AccelerationCondition.create()));

	e = elem->FirstChildElement("CollisionCondition");
	if(e != nullptr)
		__parse__CollisionCondition(e, *(obj.sub_CollisionCondition.create()));

	e = elem->FirstChildElement("DistanceCondition");
	if(e != nullptr)
		__parse__DistanceCondition(e, *(obj.sub_DistanceCondition.create()));

	e = elem->FirstChildElement("EndOfRoadCondition");
	if(e != nullptr)
		__parse__EndOfRoadCondition(e, *(obj.sub_EndOfRoadCondition.create()));

	e = elem->FirstChildElement("OffroadCondition");
	if(e != nullptr)
		__parse__OffroadCondition(e, *(obj.sub_OffroadCondition.create()));

	e = elem->FirstChildElement("ReachPositionCondition");
	if(e != nullptr)
		__parse__ReachPositionCondition(e, *(obj.sub_ReachPositionCondition.create()));

	e = elem->FirstChildElement("RelativeDistanceCondition");
	if(e != nullptr)
		__parse__RelativeDistanceCondition(e, *(obj.sub_RelativeDistanceCondition.create()));

	e = elem->FirstChildElement("RelativeSpeedCondition");
	if(e != nullptr)
		__parse__RelativeSpeedCondition(e, *(obj.sub_RelativeSpeedCondition.create()));

	e = elem->FirstChildElement("SpeedCondition");
	if(e != nullptr)
		__parse__SpeedCondition(e, *(obj.sub_SpeedCondition.create()));

	e = elem->FirstChildElement("StandStillCondition");
	if(e != nullptr)
		__parse__StandStillCondition(e, *(obj.sub_StandStillCondition.create()));

	e = elem->FirstChildElement("TimeHeadwayCondition");
	if(e != nullptr)
		__parse__TimeHeadwayCondition(e, *(obj.sub_TimeHeadwayCondition.create()));

	e = elem->FirstChildElement("TimeToCollisionCondition");
	if(e != nullptr)
		__parse__TimeToCollisionCondition(e, *(obj.sub_TimeToCollisionCondition.create()));

	e = elem->FirstChildElement("TraveledDistanceCondition");
	if(e != nullptr)
		__parse__TraveledDistanceCondition(e, *(obj.sub_TraveledDistanceCondition.create()));



	return true;

}

bool __parse__EntityRef(const tinyxml2::XMLElement *elem, EntityRef &obj) {

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));



	return true;

}

bool __parse__EntitySelection(const tinyxml2::XMLElement *elem, EntitySelection &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Members");
	if(e != nullptr)
		__parse__SelectedEntities(e, *(obj.sub_Members.create()));



	return true;

}

bool __parse__Environment(const tinyxml2::XMLElement *elem, Environment &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ParameterDeclarations");
	if(e != nullptr)
		__parse__ParameterDeclarations(e, *(obj.sub_ParameterDeclarations.create()));

	e = elem->FirstChildElement("RoadCondition");
	if(e != nullptr)
		__parse__RoadCondition(e, *(obj.sub_RoadCondition.create()));

	e = elem->FirstChildElement("TimeOfDay");
	if(e != nullptr)
		__parse__TimeOfDay(e, *(obj.sub_TimeOfDay.create()));

	e = elem->FirstChildElement("Weather");
	if(e != nullptr)
		__parse__Weather(e, *(obj.sub_Weather.create()));



	return true;

}

bool __parse__EnvironmentAction(const tinyxml2::XMLElement *elem, EnvironmentAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("CatalogReference");
	if(e != nullptr)
		__parse__CatalogReference(e, *(obj.sub_CatalogReference.create()));

	e = elem->FirstChildElement("Environment");
	if(e != nullptr)
		__parse__Environment(e, *(obj.sub_Environment.create()));



	return true;

}

bool __parse__EnvironmentCatalogLocation(const tinyxml2::XMLElement *elem, EnvironmentCatalogLocation &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Directory");
	if(e != nullptr)
		__parse__Directory(e, *(obj.sub_Directory.create()));



	return true;

}

bool __parse__Event(const tinyxml2::XMLElement *elem, Event &obj) {

	if(elem->Attribute("maximumExecutionCount") != nullptr)
		obj._maximumExecutionCount = std::string(elem->Attribute("maximumExecutionCount"));

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	if(elem->Attribute("priority") != nullptr)
		obj._priority = std::string(elem->Attribute("priority"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Action");
	while(e != nullptr) {

		obj.sub_Action.emplace_back();
		__parse__Action(e, obj.sub_Action.back());

		e = e->NextSiblingElement("Action");
	}

	e = elem->FirstChildElement("StartTrigger");
	if(e != nullptr)
		__parse__Trigger(e, *(obj.sub_StartTrigger.create()));



	return true;

}

bool __parse__File(const tinyxml2::XMLElement *elem, File &obj) {

	if(elem->Attribute("filepath") != nullptr)
		obj._filepath = std::string(elem->Attribute("filepath"));



	return true;

}

bool __parse__FileHeader(const tinyxml2::XMLElement *elem, FileHeader &obj) {

	if(elem->Attribute("author") != nullptr)
		obj._author = std::string(elem->Attribute("author"));

	if(elem->Attribute("date") != nullptr)
		obj._date = std::string(elem->Attribute("date"));

	if(elem->Attribute("description") != nullptr)
		obj._description = std::string(elem->Attribute("description"));

	if(elem->Attribute("revMajor") != nullptr)
		obj._revMajor = std::string(elem->Attribute("revMajor"));

	if(elem->Attribute("revMinor") != nullptr)
		obj._revMinor = std::string(elem->Attribute("revMinor"));



	return true;

}

bool __parse__FinalSpeed(const tinyxml2::XMLElement *elem, FinalSpeed &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("AbsoluteSpeed");
	if(e != nullptr)
		__parse__AbsoluteSpeed(e, *(obj.sub_AbsoluteSpeed.create()));

	e = elem->FirstChildElement("RelativeSpeedToMaster");
	if(e != nullptr)
		__parse__RelativeSpeedToMaster(e, *(obj.sub_RelativeSpeedToMaster.create()));



	return true;

}

bool __parse__Fog(const tinyxml2::XMLElement *elem, Fog &obj) {

	if(elem->Attribute("visualRange") != nullptr)
		obj._visualRange = std::string(elem->Attribute("visualRange"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("BoundingBox");
	if(e != nullptr)
		__parse__BoundingBox(e, *(obj.sub_BoundingBox.create()));



	return true;

}

bool __parse__FollowTrajectoryAction(const tinyxml2::XMLElement *elem, FollowTrajectoryAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("CatalogReference");
	if(e != nullptr)
		__parse__CatalogReference(e, *(obj.sub_CatalogReference.create()));

	e = elem->FirstChildElement("TimeReference");
	if(e != nullptr)
		__parse__TimeReference(e, *(obj.sub_TimeReference.create()));

	e = elem->FirstChildElement("Trajectory");
	if(e != nullptr)
		__parse__Trajectory(e, *(obj.sub_Trajectory.create()));

	e = elem->FirstChildElement("TrajectoryFollowingMode");
	if(e != nullptr)
		__parse__TrajectoryFollowingMode(e, *(obj.sub_TrajectoryFollowingMode.create()));



	return true;

}

bool __parse__GlobalAction(const tinyxml2::XMLElement *elem, GlobalAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("EntityAction");
	if(e != nullptr)
		__parse__EntityAction(e, *(obj.sub_EntityAction.create()));

	e = elem->FirstChildElement("EnvironmentAction");
	if(e != nullptr)
		__parse__EnvironmentAction(e, *(obj.sub_EnvironmentAction.create()));

	e = elem->FirstChildElement("InfrastructureAction");
	if(e != nullptr)
		__parse__InfrastructureAction(e, *(obj.sub_InfrastructureAction.create()));

	e = elem->FirstChildElement("ParameterAction");
	if(e != nullptr)
		__parse__ParameterAction(e, *(obj.sub_ParameterAction.create()));

	e = elem->FirstChildElement("TrafficAction");
	if(e != nullptr)
		__parse__TrafficAction(e, *(obj.sub_TrafficAction.create()));



	return true;

}

bool __parse__InfrastructureAction(const tinyxml2::XMLElement *elem, InfrastructureAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("TrafficSignalAction");
	if(e != nullptr)
		__parse__TrafficSignalAction(e, *(obj.sub_TrafficSignalAction.create()));



	return true;

}

bool __parse__Init(const tinyxml2::XMLElement *elem, Init &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Actions");
	if(e != nullptr)
		__parse__InitActions(e, *(obj.sub_Actions.create()));



	return true;

}

bool __parse__InitActions(const tinyxml2::XMLElement *elem, InitActions &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("GlobalAction");
	while(e != nullptr) {

		obj.sub_GlobalAction.emplace_back();
		__parse__GlobalAction(e, obj.sub_GlobalAction.back());

		e = e->NextSiblingElement("GlobalAction");
	}

	e = elem->FirstChildElement("Private");
	while(e != nullptr) {

		obj.sub_Private.emplace_back();
		__parse__Private(e, obj.sub_Private.back());

		e = e->NextSiblingElement("Private");
	}

	e = elem->FirstChildElement("UserDefinedAction");
	while(e != nullptr) {

		obj.sub_UserDefinedAction.emplace_back();
		__parse__UserDefinedAction(e, obj.sub_UserDefinedAction.back());

		e = e->NextSiblingElement("UserDefinedAction");
	}



	return true;

}

bool __parse__InRoutePosition(const tinyxml2::XMLElement *elem, InRoutePosition &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("FromCurrentEntity");
	if(e != nullptr)
		__parse__PositionOfCurrentEntity(e, *(obj.sub_FromCurrentEntity.create()));

	e = elem->FirstChildElement("FromLaneCoordinates");
	if(e != nullptr)
		__parse__PositionInLaneCoordinates(e, *(obj.sub_FromLaneCoordinates.create()));

	e = elem->FirstChildElement("FromRoadCoordinates");
	if(e != nullptr)
		__parse__PositionInRoadCoordinates(e, *(obj.sub_FromRoadCoordinates.create()));



	return true;

}

bool __parse__Knot(const tinyxml2::XMLElement *elem, Knot &obj) {

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__LaneChangeAction(const tinyxml2::XMLElement *elem, LaneChangeAction &obj) {

	if(elem->Attribute("targetLaneOffset") != nullptr)
		obj._targetLaneOffset = std::string(elem->Attribute("targetLaneOffset"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("LaneChangeActionDynamics");
	if(e != nullptr)
		__parse__TransitionDynamics(e, *(obj.sub_LaneChangeActionDynamics.create()));

	e = elem->FirstChildElement("LaneChangeTarget");
	if(e != nullptr)
		__parse__LaneChangeTarget(e, *(obj.sub_LaneChangeTarget.create()));



	return true;

}

bool __parse__LaneChangeTarget(const tinyxml2::XMLElement *elem, LaneChangeTarget &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("AbsoluteTargetLane");
	if(e != nullptr)
		__parse__AbsoluteTargetLane(e, *(obj.sub_AbsoluteTargetLane.create()));

	e = elem->FirstChildElement("RelativeTargetLane");
	if(e != nullptr)
		__parse__RelativeTargetLane(e, *(obj.sub_RelativeTargetLane.create()));



	return true;

}

bool __parse__LaneOffsetAction(const tinyxml2::XMLElement *elem, LaneOffsetAction &obj) {

	if(elem->Attribute("continuous") != nullptr)
		obj._continuous = std::string(elem->Attribute("continuous"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("LaneOffsetActionDynamics");
	if(e != nullptr)
		__parse__LaneOffsetActionDynamics(e, *(obj.sub_LaneOffsetActionDynamics.create()));

	e = elem->FirstChildElement("LaneOffsetTarget");
	if(e != nullptr)
		__parse__LaneOffsetTarget(e, *(obj.sub_LaneOffsetTarget.create()));



	return true;

}

bool __parse__LaneOffsetActionDynamics(const tinyxml2::XMLElement *elem, LaneOffsetActionDynamics &obj) {

	if(elem->Attribute("dynamicsShape") != nullptr)
		obj._dynamicsShape = std::string(elem->Attribute("dynamicsShape"));

	if(elem->Attribute("maxLateralAcc") != nullptr)
		obj._maxLateralAcc = std::string(elem->Attribute("maxLateralAcc"));



	return true;

}

bool __parse__LaneOffsetTarget(const tinyxml2::XMLElement *elem, LaneOffsetTarget &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("AbsoluteTargetLaneOffset");
	if(e != nullptr)
		__parse__AbsoluteTargetLaneOffset(e, *(obj.sub_AbsoluteTargetLaneOffset.create()));

	e = elem->FirstChildElement("RelativeTargetLaneOffset");
	if(e != nullptr)
		__parse__RelativeTargetLaneOffset(e, *(obj.sub_RelativeTargetLaneOffset.create()));



	return true;

}

bool __parse__LanePosition(const tinyxml2::XMLElement *elem, LanePosition &obj) {

	if(elem->Attribute("laneId") != nullptr)
		obj._laneId = std::string(elem->Attribute("laneId"));

	if(elem->Attribute("offset") != nullptr)
		obj._offset = std::string(elem->Attribute("offset"));

	if(elem->Attribute("roadId") != nullptr)
		obj._roadId = std::string(elem->Attribute("roadId"));

	if(elem->Attribute("s") != nullptr)
		obj._s = std::string(elem->Attribute("s"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Orientation");
	if(e != nullptr)
		__parse__Orientation(e, *(obj.sub_Orientation.create()));



	return true;

}

bool __parse__LateralAction(const tinyxml2::XMLElement *elem, LateralAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("LaneChangeAction");
	if(e != nullptr)
		__parse__LaneChangeAction(e, *(obj.sub_LaneChangeAction.create()));

	e = elem->FirstChildElement("LaneOffsetAction");
	if(e != nullptr)
		__parse__LaneOffsetAction(e, *(obj.sub_LaneOffsetAction.create()));

	e = elem->FirstChildElement("LateralDistanceAction");
	if(e != nullptr)
		__parse__LateralDistanceAction(e, *(obj.sub_LateralDistanceAction.create()));



	return true;

}

bool __parse__LateralDistanceAction(const tinyxml2::XMLElement *elem, LateralDistanceAction &obj) {

	if(elem->Attribute("continuous") != nullptr)
		obj._continuous = std::string(elem->Attribute("continuous"));

	if(elem->Attribute("distance") != nullptr)
		obj._distance = std::string(elem->Attribute("distance"));

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	if(elem->Attribute("freespace") != nullptr)
		obj._freespace = std::string(elem->Attribute("freespace"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("DynamicConstraints");
	if(e != nullptr)
		__parse__DynamicConstraints(e, *(obj.sub_DynamicConstraints.create()));



	return true;

}

bool __parse__LongitudinalAction(const tinyxml2::XMLElement *elem, LongitudinalAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("LongitudinalDistanceAction");
	if(e != nullptr)
		__parse__LongitudinalDistanceAction(e, *(obj.sub_LongitudinalDistanceAction.create()));

	e = elem->FirstChildElement("SpeedAction");
	if(e != nullptr)
		__parse__SpeedAction(e, *(obj.sub_SpeedAction.create()));



	return true;

}

bool __parse__LongitudinalDistanceAction(const tinyxml2::XMLElement *elem, LongitudinalDistanceAction &obj) {

	if(elem->Attribute("continuous") != nullptr)
		obj._continuous = std::string(elem->Attribute("continuous"));

	if(elem->Attribute("distance") != nullptr)
		obj._distance = std::string(elem->Attribute("distance"));

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	if(elem->Attribute("freespace") != nullptr)
		obj._freespace = std::string(elem->Attribute("freespace"));

	if(elem->Attribute("timeGap") != nullptr)
		obj._timeGap = std::string(elem->Attribute("timeGap"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("DynamicConstraints");
	if(e != nullptr)
		__parse__DynamicConstraints(e, *(obj.sub_DynamicConstraints.create()));



	return true;

}

bool __parse__Maneuver(const tinyxml2::XMLElement *elem, Maneuver &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Event");
	while(e != nullptr) {

		obj.sub_Event.emplace_back();
		__parse__Event(e, obj.sub_Event.back());

		e = e->NextSiblingElement("Event");
	}

	e = elem->FirstChildElement("ParameterDeclarations");
	if(e != nullptr)
		__parse__ParameterDeclarations(e, *(obj.sub_ParameterDeclarations.create()));



	return true;

}

bool __parse__ManeuverCatalogLocation(const tinyxml2::XMLElement *elem, ManeuverCatalogLocation &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Directory");
	if(e != nullptr)
		__parse__Directory(e, *(obj.sub_Directory.create()));



	return true;

}

bool __parse__ManeuverGroup(const tinyxml2::XMLElement *elem, ManeuverGroup &obj) {

	if(elem->Attribute("maximumExecutionCount") != nullptr)
		obj._maximumExecutionCount = std::string(elem->Attribute("maximumExecutionCount"));

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Actors");
	if(e != nullptr)
		__parse__Actors(e, *(obj.sub_Actors.create()));

	e = elem->FirstChildElement("CatalogReference");
	while(e != nullptr) {

		obj.sub_CatalogReference.emplace_back();
		__parse__CatalogReference(e, obj.sub_CatalogReference.back());

		e = e->NextSiblingElement("CatalogReference");
	}

	e = elem->FirstChildElement("Maneuver");
	while(e != nullptr) {

		obj.sub_Maneuver.emplace_back();
		__parse__Maneuver(e, obj.sub_Maneuver.back());

		e = e->NextSiblingElement("Maneuver");
	}



	return true;

}

bool __parse__MiscObject(const tinyxml2::XMLElement *elem, MiscObject &obj) {

	if(elem->Attribute("mass") != nullptr)
		obj._mass = std::string(elem->Attribute("mass"));

	if(elem->Attribute("miscObjectCategory") != nullptr)
		obj._miscObjectCategory = std::string(elem->Attribute("miscObjectCategory"));

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("BoundingBox");
	if(e != nullptr)
		__parse__BoundingBox(e, *(obj.sub_BoundingBox.create()));

	e = elem->FirstChildElement("ParameterDeclarations");
	if(e != nullptr)
		__parse__ParameterDeclarations(e, *(obj.sub_ParameterDeclarations.create()));

	e = elem->FirstChildElement("Properties");
	if(e != nullptr)
		__parse__Properties(e, *(obj.sub_Properties.create()));



	return true;

}

bool __parse__MiscObjectCatalogLocation(const tinyxml2::XMLElement *elem, MiscObjectCatalogLocation &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Directory");
	if(e != nullptr)
		__parse__Directory(e, *(obj.sub_Directory.create()));



	return true;

}

bool __parse__ModifyRule(const tinyxml2::XMLElement *elem, ModifyRule &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("AddValue");
	if(e != nullptr)
		__parse__ParameterAddValueRule(e, *(obj.sub_AddValue.create()));

	e = elem->FirstChildElement("MultiplyByValue");
	if(e != nullptr)
		__parse__ParameterMultiplyByValueRule(e, *(obj.sub_MultiplyByValue.create()));



	return true;

}

bool __parse__None(const tinyxml2::XMLElement *elem, None &obj) {



	return true;

}

bool __parse__Nurbs(const tinyxml2::XMLElement *elem, Nurbs &obj) {

	if(elem->Attribute("order") != nullptr)
		obj._order = std::string(elem->Attribute("order"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ControlPoint");
	while(e != nullptr) {

		obj.sub_ControlPoint.emplace_back();
		__parse__ControlPoint(e, obj.sub_ControlPoint.back());

		e = e->NextSiblingElement("ControlPoint");
	}

	e = elem->FirstChildElement("Knot");
	while(e != nullptr) {

		obj.sub_Knot.emplace_back();
		__parse__Knot(e, obj.sub_Knot.back());

		e = e->NextSiblingElement("Knot");
	}



	return true;

}

bool __parse__ObjectController(const tinyxml2::XMLElement *elem, ObjectController &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("CatalogReference");
	if(e != nullptr)
		__parse__CatalogReference(e, *(obj.sub_CatalogReference.create()));

	e = elem->FirstChildElement("Controller");
	if(e != nullptr)
		__parse__Controller(e, *(obj.sub_Controller.create()));



	return true;

}

bool __parse__OffroadCondition(const tinyxml2::XMLElement *elem, OffroadCondition &obj) {

	if(elem->Attribute("duration") != nullptr)
		obj._duration = std::string(elem->Attribute("duration"));



	return true;

}

bool __parse__OpenScenario(const tinyxml2::XMLElement *elem, OpenScenario &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Catalog");
	if(e != nullptr)
		__parse__Catalog(e, *(obj.sub_Catalog.create()));

	e = elem->FirstChildElement("CatalogLocations");
	if(e != nullptr)
		__parse__CatalogLocations(e, *(obj.sub_CatalogLocations.create()));

	e = elem->FirstChildElement("Entities");
	if(e != nullptr)
		__parse__Entities(e, *(obj.sub_Entities.create()));

	e = elem->FirstChildElement("FileHeader");
	if(e != nullptr)
		__parse__FileHeader(e, *(obj.sub_FileHeader.create()));

	e = elem->FirstChildElement("ParameterDeclarations");
	if(e != nullptr)
		__parse__ParameterDeclarations(e, *(obj.sub_ParameterDeclarations.create()));

	e = elem->FirstChildElement("RoadNetwork");
	if(e != nullptr)
		__parse__RoadNetwork(e, *(obj.sub_RoadNetwork.create()));

	e = elem->FirstChildElement("Storyboard");
	if(e != nullptr)
		__parse__Storyboard(e, *(obj.sub_Storyboard.create()));



	return true;

}

bool __parse__Orientation(const tinyxml2::XMLElement *elem, Orientation &obj) {

	if(elem->Attribute("h") != nullptr)
		obj._h = std::string(elem->Attribute("h"));

	if(elem->Attribute("p") != nullptr)
		obj._p = std::string(elem->Attribute("p"));

	if(elem->Attribute("r") != nullptr)
		obj._r = std::string(elem->Attribute("r"));

	if(elem->Attribute("type") != nullptr)
		obj._type = std::string(elem->Attribute("type"));



	return true;

}

bool __parse__OverrideBrakeAction(const tinyxml2::XMLElement *elem, OverrideBrakeAction &obj) {

	if(elem->Attribute("active") != nullptr)
		obj._active = std::string(elem->Attribute("active"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__OverrideClutchAction(const tinyxml2::XMLElement *elem, OverrideClutchAction &obj) {

	if(elem->Attribute("active") != nullptr)
		obj._active = std::string(elem->Attribute("active"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__OverrideControllerValueAction(const tinyxml2::XMLElement *elem, OverrideControllerValueAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Brake");
	if(e != nullptr)
		__parse__OverrideBrakeAction(e, *(obj.sub_Brake.create()));

	e = elem->FirstChildElement("Clutch");
	if(e != nullptr)
		__parse__OverrideClutchAction(e, *(obj.sub_Clutch.create()));

	e = elem->FirstChildElement("Gear");
	if(e != nullptr)
		__parse__OverrideGearAction(e, *(obj.sub_Gear.create()));

	e = elem->FirstChildElement("ParkingBrake");
	if(e != nullptr)
		__parse__OverrideParkingBrakeAction(e, *(obj.sub_ParkingBrake.create()));

	e = elem->FirstChildElement("SteeringWheel");
	if(e != nullptr)
		__parse__OverrideSteeringWheelAction(e, *(obj.sub_SteeringWheel.create()));

	e = elem->FirstChildElement("Throttle");
	if(e != nullptr)
		__parse__OverrideThrottleAction(e, *(obj.sub_Throttle.create()));



	return true;

}

bool __parse__OverrideGearAction(const tinyxml2::XMLElement *elem, OverrideGearAction &obj) {

	if(elem->Attribute("active") != nullptr)
		obj._active = std::string(elem->Attribute("active"));

	if(elem->Attribute("number") != nullptr)
		obj._number = std::string(elem->Attribute("number"));



	return true;

}

bool __parse__OverrideParkingBrakeAction(const tinyxml2::XMLElement *elem, OverrideParkingBrakeAction &obj) {

	if(elem->Attribute("active") != nullptr)
		obj._active = std::string(elem->Attribute("active"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__OverrideSteeringWheelAction(const tinyxml2::XMLElement *elem, OverrideSteeringWheelAction &obj) {

	if(elem->Attribute("active") != nullptr)
		obj._active = std::string(elem->Attribute("active"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__OverrideThrottleAction(const tinyxml2::XMLElement *elem, OverrideThrottleAction &obj) {

	if(elem->Attribute("active") != nullptr)
		obj._active = std::string(elem->Attribute("active"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__ParameterAction(const tinyxml2::XMLElement *elem, ParameterAction &obj) {

	if(elem->Attribute("parameterRef") != nullptr)
		obj._parameterRef = std::string(elem->Attribute("parameterRef"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ModifyAction");
	if(e != nullptr)
		__parse__ParameterModifyAction(e, *(obj.sub_ModifyAction.create()));

	e = elem->FirstChildElement("SetAction");
	if(e != nullptr)
		__parse__ParameterSetAction(e, *(obj.sub_SetAction.create()));



	return true;

}

bool __parse__ParameterAddValueRule(const tinyxml2::XMLElement *elem, ParameterAddValueRule &obj) {

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__ParameterAssignment(const tinyxml2::XMLElement *elem, ParameterAssignment &obj) {

	if(elem->Attribute("parameterRef") != nullptr)
		obj._parameterRef = std::string(elem->Attribute("parameterRef"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__ParameterAssignments(const tinyxml2::XMLElement *elem, ParameterAssignments &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ParameterAssignment");
	while(e != nullptr) {

		obj.sub_ParameterAssignment.emplace_back();
		__parse__ParameterAssignment(e, obj.sub_ParameterAssignment.back());

		e = e->NextSiblingElement("ParameterAssignment");
	}



	return true;

}

bool __parse__ParameterCondition(const tinyxml2::XMLElement *elem, ParameterCondition &obj) {

	if(elem->Attribute("parameterRef") != nullptr)
		obj._parameterRef = std::string(elem->Attribute("parameterRef"));

	if(elem->Attribute("rule") != nullptr)
		obj._rule = std::string(elem->Attribute("rule"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__ParameterDeclaration(const tinyxml2::XMLElement *elem, ParameterDeclaration &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	if(elem->Attribute("parameterType") != nullptr)
		obj._parameterType = std::string(elem->Attribute("parameterType"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__ParameterDeclarations(const tinyxml2::XMLElement *elem, ParameterDeclarations &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ParameterDeclaration");
	while(e != nullptr) {

		obj.sub_ParameterDeclaration.emplace_back();
		__parse__ParameterDeclaration(e, obj.sub_ParameterDeclaration.back());

		e = e->NextSiblingElement("ParameterDeclaration");
	}



	return true;

}

bool __parse__ParameterModifyAction(const tinyxml2::XMLElement *elem, ParameterModifyAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Rule");
	if(e != nullptr)
		__parse__ModifyRule(e, *(obj.sub_Rule.create()));



	return true;

}

bool __parse__ParameterMultiplyByValueRule(const tinyxml2::XMLElement *elem, ParameterMultiplyByValueRule &obj) {

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__ParameterSetAction(const tinyxml2::XMLElement *elem, ParameterSetAction &obj) {

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__Pedestrian(const tinyxml2::XMLElement *elem, Pedestrian &obj) {

	if(elem->Attribute("mass") != nullptr)
		obj._mass = std::string(elem->Attribute("mass"));

	if(elem->Attribute("model") != nullptr)
		obj._model = std::string(elem->Attribute("model"));

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	if(elem->Attribute("pedestrianCategory") != nullptr)
		obj._pedestrianCategory = std::string(elem->Attribute("pedestrianCategory"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("BoundingBox");
	if(e != nullptr)
		__parse__BoundingBox(e, *(obj.sub_BoundingBox.create()));

	e = elem->FirstChildElement("ParameterDeclarations");
	if(e != nullptr)
		__parse__ParameterDeclarations(e, *(obj.sub_ParameterDeclarations.create()));

	e = elem->FirstChildElement("Properties");
	if(e != nullptr)
		__parse__Properties(e, *(obj.sub_Properties.create()));



	return true;

}

bool __parse__PedestrianCatalogLocation(const tinyxml2::XMLElement *elem, PedestrianCatalogLocation &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Directory");
	if(e != nullptr)
		__parse__Directory(e, *(obj.sub_Directory.create()));



	return true;

}

bool __parse__Performance(const tinyxml2::XMLElement *elem, Performance &obj) {

	if(elem->Attribute("maxAcceleration") != nullptr)
		obj._maxAcceleration = std::string(elem->Attribute("maxAcceleration"));

	if(elem->Attribute("maxDeceleration") != nullptr)
		obj._maxDeceleration = std::string(elem->Attribute("maxDeceleration"));

	if(elem->Attribute("maxSpeed") != nullptr)
		obj._maxSpeed = std::string(elem->Attribute("maxSpeed"));



	return true;

}

bool __parse__Phase(const tinyxml2::XMLElement *elem, Phase &obj) {

	if(elem->Attribute("duration") != nullptr)
		obj._duration = std::string(elem->Attribute("duration"));

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("TrafficSignalState");
	while(e != nullptr) {

		obj.sub_TrafficSignalState.emplace_back();
		__parse__TrafficSignalState(e, obj.sub_TrafficSignalState.back());

		e = e->NextSiblingElement("TrafficSignalState");
	}



	return true;

}

bool __parse__Polyline(const tinyxml2::XMLElement *elem, Polyline &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Vertex");
	while(e != nullptr) {

		obj.sub_Vertex.emplace_back();
		__parse__Vertex(e, obj.sub_Vertex.back());

		e = e->NextSiblingElement("Vertex");
	}



	return true;

}

bool __parse__Position(const tinyxml2::XMLElement *elem, Position &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("LanePosition");
	if(e != nullptr)
		__parse__LanePosition(e, *(obj.sub_LanePosition.create()));

	e = elem->FirstChildElement("RelativeLanePosition");
	if(e != nullptr)
		__parse__RelativeLanePosition(e, *(obj.sub_RelativeLanePosition.create()));

	e = elem->FirstChildElement("RelativeObjectPosition");
	if(e != nullptr)
		__parse__RelativeObjectPosition(e, *(obj.sub_RelativeObjectPosition.create()));

	e = elem->FirstChildElement("RelativeRoadPosition");
	if(e != nullptr)
		__parse__RelativeRoadPosition(e, *(obj.sub_RelativeRoadPosition.create()));

	e = elem->FirstChildElement("RelativeWorldPosition");
	if(e != nullptr)
		__parse__RelativeWorldPosition(e, *(obj.sub_RelativeWorldPosition.create()));

	e = elem->FirstChildElement("RoadPosition");
	if(e != nullptr)
		__parse__RoadPosition(e, *(obj.sub_RoadPosition.create()));

	e = elem->FirstChildElement("RoutePosition");
	if(e != nullptr)
		__parse__RoutePosition(e, *(obj.sub_RoutePosition.create()));

	e = elem->FirstChildElement("WorldPosition");
	if(e != nullptr)
		__parse__WorldPosition(e, *(obj.sub_WorldPosition.create()));



	return true;

}

bool __parse__PositionInLaneCoordinates(const tinyxml2::XMLElement *elem, PositionInLaneCoordinates &obj) {

	if(elem->Attribute("laneId") != nullptr)
		obj._laneId = std::string(elem->Attribute("laneId"));

	if(elem->Attribute("laneOffset") != nullptr)
		obj._laneOffset = std::string(elem->Attribute("laneOffset"));

	if(elem->Attribute("pathS") != nullptr)
		obj._pathS = std::string(elem->Attribute("pathS"));



	return true;

}

bool __parse__PositionInRoadCoordinates(const tinyxml2::XMLElement *elem, PositionInRoadCoordinates &obj) {

	if(elem->Attribute("pathS") != nullptr)
		obj._pathS = std::string(elem->Attribute("pathS"));

	if(elem->Attribute("t") != nullptr)
		obj._t = std::string(elem->Attribute("t"));



	return true;

}

bool __parse__PositionOfCurrentEntity(const tinyxml2::XMLElement *elem, PositionOfCurrentEntity &obj) {

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));



	return true;

}

bool __parse__Precipitation(const tinyxml2::XMLElement *elem, Precipitation &obj) {

	if(elem->Attribute("intensity") != nullptr)
		obj._intensity = std::string(elem->Attribute("intensity"));

	if(elem->Attribute("precipitationType") != nullptr)
		obj._precipitationType = std::string(elem->Attribute("precipitationType"));



	return true;

}

bool __parse__Private(const tinyxml2::XMLElement *elem, Private &obj) {

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("PrivateAction");
	while(e != nullptr) {

		obj.sub_PrivateAction.emplace_back();
		__parse__PrivateAction(e, obj.sub_PrivateAction.back());

		e = e->NextSiblingElement("PrivateAction");
	}



	return true;

}

bool __parse__PrivateAction(const tinyxml2::XMLElement *elem, PrivateAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ActivateControllerAction");
	if(e != nullptr)
		__parse__ActivateControllerAction(e, *(obj.sub_ActivateControllerAction.create()));

	e = elem->FirstChildElement("ControllerAction");
	if(e != nullptr)
		__parse__ControllerAction(e, *(obj.sub_ControllerAction.create()));

	e = elem->FirstChildElement("LateralAction");
	if(e != nullptr)
		__parse__LateralAction(e, *(obj.sub_LateralAction.create()));

	e = elem->FirstChildElement("LongitudinalAction");
	if(e != nullptr)
		__parse__LongitudinalAction(e, *(obj.sub_LongitudinalAction.create()));

	e = elem->FirstChildElement("RoutingAction");
	if(e != nullptr)
		__parse__RoutingAction(e, *(obj.sub_RoutingAction.create()));

	e = elem->FirstChildElement("SynchronizeAction");
	if(e != nullptr)
		__parse__SynchronizeAction(e, *(obj.sub_SynchronizeAction.create()));

	e = elem->FirstChildElement("TeleportAction");
	if(e != nullptr)
		__parse__TeleportAction(e, *(obj.sub_TeleportAction.create()));

	e = elem->FirstChildElement("VisibilityAction");
	if(e != nullptr)
		__parse__VisibilityAction(e, *(obj.sub_VisibilityAction.create()));



	return true;

}

bool __parse__Properties(const tinyxml2::XMLElement *elem, Properties &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("File");
	while(e != nullptr) {

		obj.sub_File.emplace_back();
		__parse__File(e, obj.sub_File.back());

		e = e->NextSiblingElement("File");
	}

	e = elem->FirstChildElement("Property");
	while(e != nullptr) {

		obj.sub_Property.emplace_back();
		__parse__Property(e, obj.sub_Property.back());

		e = e->NextSiblingElement("Property");
	}



	return true;

}

bool __parse__Property(const tinyxml2::XMLElement *elem, Property &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__ReachPositionCondition(const tinyxml2::XMLElement *elem, ReachPositionCondition &obj) {

	if(elem->Attribute("tolerance") != nullptr)
		obj._tolerance = std::string(elem->Attribute("tolerance"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));



	return true;

}

bool __parse__RelativeDistanceCondition(const tinyxml2::XMLElement *elem, RelativeDistanceCondition &obj) {

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	if(elem->Attribute("freespace") != nullptr)
		obj._freespace = std::string(elem->Attribute("freespace"));

	if(elem->Attribute("relativeDistanceType") != nullptr)
		obj._relativeDistanceType = std::string(elem->Attribute("relativeDistanceType"));

	if(elem->Attribute("rule") != nullptr)
		obj._rule = std::string(elem->Attribute("rule"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__RelativeLanePosition(const tinyxml2::XMLElement *elem, RelativeLanePosition &obj) {

	if(elem->Attribute("dLane") != nullptr)
		obj._dLane = std::string(elem->Attribute("dLane"));

	if(elem->Attribute("ds") != nullptr)
		obj._ds = std::string(elem->Attribute("ds"));

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	if(elem->Attribute("offset") != nullptr)
		obj._offset = std::string(elem->Attribute("offset"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Orientation");
	if(e != nullptr)
		__parse__Orientation(e, *(obj.sub_Orientation.create()));



	return true;

}

bool __parse__RelativeObjectPosition(const tinyxml2::XMLElement *elem, RelativeObjectPosition &obj) {

	if(elem->Attribute("dx") != nullptr)
		obj._dx = std::string(elem->Attribute("dx"));

	if(elem->Attribute("dy") != nullptr)
		obj._dy = std::string(elem->Attribute("dy"));

	if(elem->Attribute("dz") != nullptr)
		obj._dz = std::string(elem->Attribute("dz"));

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Orientation");
	if(e != nullptr)
		__parse__Orientation(e, *(obj.sub_Orientation.create()));



	return true;

}

bool __parse__RelativeRoadPosition(const tinyxml2::XMLElement *elem, RelativeRoadPosition &obj) {

	if(elem->Attribute("ds") != nullptr)
		obj._ds = std::string(elem->Attribute("ds"));

	if(elem->Attribute("dt") != nullptr)
		obj._dt = std::string(elem->Attribute("dt"));

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Orientation");
	if(e != nullptr)
		__parse__Orientation(e, *(obj.sub_Orientation.create()));



	return true;

}

bool __parse__RelativeSpeedCondition(const tinyxml2::XMLElement *elem, RelativeSpeedCondition &obj) {

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	if(elem->Attribute("rule") != nullptr)
		obj._rule = std::string(elem->Attribute("rule"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__RelativeSpeedToMaster(const tinyxml2::XMLElement *elem, RelativeSpeedToMaster &obj) {

	if(elem->Attribute("speedTargetValueType") != nullptr)
		obj._speedTargetValueType = std::string(elem->Attribute("speedTargetValueType"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__RelativeTargetLane(const tinyxml2::XMLElement *elem, RelativeTargetLane &obj) {

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__RelativeTargetLaneOffset(const tinyxml2::XMLElement *elem, RelativeTargetLaneOffset &obj) {

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__RelativeTargetSpeed(const tinyxml2::XMLElement *elem, RelativeTargetSpeed &obj) {

	if(elem->Attribute("continuous") != nullptr)
		obj._continuous = std::string(elem->Attribute("continuous"));

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	if(elem->Attribute("speedTargetValueType") != nullptr)
		obj._speedTargetValueType = std::string(elem->Attribute("speedTargetValueType"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__RelativeWorldPosition(const tinyxml2::XMLElement *elem, RelativeWorldPosition &obj) {

	if(elem->Attribute("dx") != nullptr)
		obj._dx = std::string(elem->Attribute("dx"));

	if(elem->Attribute("dy") != nullptr)
		obj._dy = std::string(elem->Attribute("dy"));

	if(elem->Attribute("dz") != nullptr)
		obj._dz = std::string(elem->Attribute("dz"));

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Orientation");
	if(e != nullptr)
		__parse__Orientation(e, *(obj.sub_Orientation.create()));



	return true;

}

bool __parse__RoadCondition(const tinyxml2::XMLElement *elem, RoadCondition &obj) {

	if(elem->Attribute("frictionScaleFactor") != nullptr)
		obj._frictionScaleFactor = std::string(elem->Attribute("frictionScaleFactor"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Properties");
	if(e != nullptr)
		__parse__Properties(e, *(obj.sub_Properties.create()));



	return true;

}

bool __parse__RoadNetwork(const tinyxml2::XMLElement *elem, RoadNetwork &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("LogicFile");
	if(e != nullptr)
		__parse__File(e, *(obj.sub_LogicFile.create()));

	e = elem->FirstChildElement("SceneGraphFile");
	if(e != nullptr)
		__parse__File(e, *(obj.sub_SceneGraphFile.create()));

	e = elem->FirstChildElement("TrafficSignals");
	if(e != nullptr)
		__parse__TrafficSignals(e, *(obj.sub_TrafficSignals.create()));



	return true;

}

bool __parse__RoadPosition(const tinyxml2::XMLElement *elem, RoadPosition &obj) {

	if(elem->Attribute("roadId") != nullptr)
		obj._roadId = std::string(elem->Attribute("roadId"));

	if(elem->Attribute("s") != nullptr)
		obj._s = std::string(elem->Attribute("s"));

	if(elem->Attribute("t") != nullptr)
		obj._t = std::string(elem->Attribute("t"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Orientation");
	if(e != nullptr)
		__parse__Orientation(e, *(obj.sub_Orientation.create()));



	return true;

}

bool __parse__Route(const tinyxml2::XMLElement *elem, Route &obj) {

	if(elem->Attribute("closed") != nullptr)
		obj._closed = std::string(elem->Attribute("closed"));

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ParameterDeclarations");
	if(e != nullptr)
		__parse__ParameterDeclarations(e, *(obj.sub_ParameterDeclarations.create()));

	e = elem->FirstChildElement("Waypoint");
	while(e != nullptr) {

		obj.sub_Waypoint.emplace_back();
		__parse__Waypoint(e, obj.sub_Waypoint.back());

		e = e->NextSiblingElement("Waypoint");
	}



	return true;

}

bool __parse__RouteCatalogLocation(const tinyxml2::XMLElement *elem, RouteCatalogLocation &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Directory");
	if(e != nullptr)
		__parse__Directory(e, *(obj.sub_Directory.create()));



	return true;

}

bool __parse__RoutePosition(const tinyxml2::XMLElement *elem, RoutePosition &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("InRoutePosition");
	if(e != nullptr)
		__parse__InRoutePosition(e, *(obj.sub_InRoutePosition.create()));

	e = elem->FirstChildElement("Orientation");
	if(e != nullptr)
		__parse__Orientation(e, *(obj.sub_Orientation.create()));

	e = elem->FirstChildElement("RouteRef");
	if(e != nullptr)
		__parse__RouteRef(e, *(obj.sub_RouteRef.create()));



	return true;

}

bool __parse__RouteRef(const tinyxml2::XMLElement *elem, RouteRef &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("CatalogReference");
	if(e != nullptr)
		__parse__CatalogReference(e, *(obj.sub_CatalogReference.create()));

	e = elem->FirstChildElement("Route");
	if(e != nullptr)
		__parse__Route(e, *(obj.sub_Route.create()));



	return true;

}

bool __parse__RoutingAction(const tinyxml2::XMLElement *elem, RoutingAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("AcquirePositionAction");
	if(e != nullptr)
		__parse__AcquirePositionAction(e, *(obj.sub_AcquirePositionAction.create()));

	e = elem->FirstChildElement("AssignRouteAction");
	if(e != nullptr)
		__parse__AssignRouteAction(e, *(obj.sub_AssignRouteAction.create()));

	e = elem->FirstChildElement("FollowTrajectoryAction");
	if(e != nullptr)
		__parse__FollowTrajectoryAction(e, *(obj.sub_FollowTrajectoryAction.create()));



	return true;

}

bool __parse__ScenarioObject(const tinyxml2::XMLElement *elem, ScenarioObject &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("CatalogReference");
	if(e != nullptr)
		__parse__CatalogReference(e, *(obj.sub_CatalogReference.create()));

	e = elem->FirstChildElement("MiscObject");
	if(e != nullptr)
		__parse__MiscObject(e, *(obj.sub_MiscObject.create()));

	e = elem->FirstChildElement("ObjectController");
	if(e != nullptr)
		__parse__ObjectController(e, *(obj.sub_ObjectController.create()));

	e = elem->FirstChildElement("Pedestrian");
	if(e != nullptr)
		__parse__Pedestrian(e, *(obj.sub_Pedestrian.create()));

	e = elem->FirstChildElement("Vehicle");
	if(e != nullptr)
		__parse__Vehicle(e, *(obj.sub_Vehicle.create()));



	return true;

}

bool __parse__SelectedEntities(const tinyxml2::XMLElement *elem, SelectedEntities &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ByType");
	if(e != nullptr)
		__parse__ByType(e, *(obj.sub_ByType.create()));

	e = elem->FirstChildElement("EntityRef");
	if(e != nullptr)
		__parse__EntityRef(e, *(obj.sub_EntityRef.create()));



	return true;

}

bool __parse__Shape(const tinyxml2::XMLElement *elem, Shape &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Clothoid");
	if(e != nullptr)
		__parse__Clothoid(e, *(obj.sub_Clothoid.create()));

	e = elem->FirstChildElement("Nurbs");
	if(e != nullptr)
		__parse__Nurbs(e, *(obj.sub_Nurbs.create()));

	e = elem->FirstChildElement("Polyline");
	if(e != nullptr)
		__parse__Polyline(e, *(obj.sub_Polyline.create()));



	return true;

}

bool __parse__SimulationTimeCondition(const tinyxml2::XMLElement *elem, SimulationTimeCondition &obj) {

	if(elem->Attribute("rule") != nullptr)
		obj._rule = std::string(elem->Attribute("rule"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__SpeedAction(const tinyxml2::XMLElement *elem, SpeedAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("SpeedActionDynamics");
	if(e != nullptr)
		__parse__TransitionDynamics(e, *(obj.sub_SpeedActionDynamics.create()));

	e = elem->FirstChildElement("SpeedActionTarget");
	if(e != nullptr)
		__parse__SpeedActionTarget(e, *(obj.sub_SpeedActionTarget.create()));



	return true;

}

bool __parse__SpeedActionTarget(const tinyxml2::XMLElement *elem, SpeedActionTarget &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("AbsoluteTargetSpeed");
	if(e != nullptr)
		__parse__AbsoluteTargetSpeed(e, *(obj.sub_AbsoluteTargetSpeed.create()));

	e = elem->FirstChildElement("RelativeTargetSpeed");
	if(e != nullptr)
		__parse__RelativeTargetSpeed(e, *(obj.sub_RelativeTargetSpeed.create()));



	return true;

}

bool __parse__SpeedCondition(const tinyxml2::XMLElement *elem, SpeedCondition &obj) {

	if(elem->Attribute("rule") != nullptr)
		obj._rule = std::string(elem->Attribute("rule"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__StandStillCondition(const tinyxml2::XMLElement *elem, StandStillCondition &obj) {

	if(elem->Attribute("duration") != nullptr)
		obj._duration = std::string(elem->Attribute("duration"));



	return true;

}

bool __parse__Story(const tinyxml2::XMLElement *elem, Story &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Act");
	while(e != nullptr) {

		obj.sub_Act.emplace_back();
		__parse__Act(e, obj.sub_Act.back());

		e = e->NextSiblingElement("Act");
	}

	e = elem->FirstChildElement("ParameterDeclarations");
	if(e != nullptr)
		__parse__ParameterDeclarations(e, *(obj.sub_ParameterDeclarations.create()));



	return true;

}

bool __parse__Storyboard(const tinyxml2::XMLElement *elem, Storyboard &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Init");
	if(e != nullptr)
		__parse__Init(e, *(obj.sub_Init.create()));

	e = elem->FirstChildElement("StopTrigger");
	if(e != nullptr)
		__parse__Trigger(e, *(obj.sub_StopTrigger.create()));

	e = elem->FirstChildElement("Story");
	while(e != nullptr) {

		obj.sub_Story.emplace_back();
		__parse__Story(e, obj.sub_Story.back());

		e = e->NextSiblingElement("Story");
	}



	return true;

}

bool __parse__StoryboardElementStateCondition(const tinyxml2::XMLElement *elem, StoryboardElementStateCondition &obj) {

	if(elem->Attribute("state") != nullptr)
		obj._state = std::string(elem->Attribute("state"));

	if(elem->Attribute("storyboardElementRef") != nullptr)
		obj._storyboardElementRef = std::string(elem->Attribute("storyboardElementRef"));

	if(elem->Attribute("storyboardElementType") != nullptr)
		obj._storyboardElementType = std::string(elem->Attribute("storyboardElementType"));



	return true;

}

bool __parse__Sun(const tinyxml2::XMLElement *elem, Sun &obj) {

	if(elem->Attribute("azimuth") != nullptr)
		obj._azimuth = std::string(elem->Attribute("azimuth"));

	if(elem->Attribute("elevation") != nullptr)
		obj._elevation = std::string(elem->Attribute("elevation"));

	if(elem->Attribute("intensity") != nullptr)
		obj._intensity = std::string(elem->Attribute("intensity"));



	return true;

}

bool __parse__SynchronizeAction(const tinyxml2::XMLElement *elem, SynchronizeAction &obj) {

	if(elem->Attribute("masterEntityRef") != nullptr)
		obj._masterEntityRef = std::string(elem->Attribute("masterEntityRef"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("FinalSpeed");
	if(e != nullptr)
		__parse__FinalSpeed(e, *(obj.sub_FinalSpeed.create()));

	e = elem->FirstChildElement("TargetPosition");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_TargetPosition.create()));

	e = elem->FirstChildElement("TargetPositionMaster");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_TargetPositionMaster.create()));



	return true;

}

bool __parse__TeleportAction(const tinyxml2::XMLElement *elem, TeleportAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));



	return true;

}

bool __parse__TimeHeadwayCondition(const tinyxml2::XMLElement *elem, TimeHeadwayCondition &obj) {

	if(elem->Attribute("alongRoute") != nullptr)
		obj._alongRoute = std::string(elem->Attribute("alongRoute"));

	if(elem->Attribute("entityRef") != nullptr)
		obj._entityRef = std::string(elem->Attribute("entityRef"));

	if(elem->Attribute("freespace") != nullptr)
		obj._freespace = std::string(elem->Attribute("freespace"));

	if(elem->Attribute("rule") != nullptr)
		obj._rule = std::string(elem->Attribute("rule"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__TimeOfDay(const tinyxml2::XMLElement *elem, TimeOfDay &obj) {

	if(elem->Attribute("animation") != nullptr)
		obj._animation = std::string(elem->Attribute("animation"));

	if(elem->Attribute("dateTime") != nullptr)
		obj._dateTime = std::string(elem->Attribute("dateTime"));



	return true;

}

bool __parse__TimeOfDayCondition(const tinyxml2::XMLElement *elem, TimeOfDayCondition &obj) {

	if(elem->Attribute("dateTime") != nullptr)
		obj._dateTime = std::string(elem->Attribute("dateTime"));

	if(elem->Attribute("rule") != nullptr)
		obj._rule = std::string(elem->Attribute("rule"));



	return true;

}

bool __parse__TimeReference(const tinyxml2::XMLElement *elem, TimeReference &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("None");
	if(e != nullptr)
		__parse__None(e, *(obj.sub_None.create()));

	e = elem->FirstChildElement("Timing");
	if(e != nullptr)
		__parse__Timing(e, *(obj.sub_Timing.create()));



	return true;

}

bool __parse__TimeToCollisionCondition(const tinyxml2::XMLElement *elem, TimeToCollisionCondition &obj) {

	if(elem->Attribute("alongRoute") != nullptr)
		obj._alongRoute = std::string(elem->Attribute("alongRoute"));

	if(elem->Attribute("freespace") != nullptr)
		obj._freespace = std::string(elem->Attribute("freespace"));

	if(elem->Attribute("rule") != nullptr)
		obj._rule = std::string(elem->Attribute("rule"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("TimeToCollisionConditionTarget");
	if(e != nullptr)
		__parse__TimeToCollisionConditionTarget(e, *(obj.sub_TimeToCollisionConditionTarget.create()));



	return true;

}

bool __parse__TimeToCollisionConditionTarget(const tinyxml2::XMLElement *elem, TimeToCollisionConditionTarget &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("EntityRef");
	if(e != nullptr)
		__parse__EntityRef(e, *(obj.sub_EntityRef.create()));

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));



	return true;

}

bool __parse__Timing(const tinyxml2::XMLElement *elem, Timing &obj) {

	if(elem->Attribute("domainAbsoluteRelative") != nullptr)
		obj._domainAbsoluteRelative = std::string(elem->Attribute("domainAbsoluteRelative"));

	if(elem->Attribute("offset") != nullptr)
		obj._offset = std::string(elem->Attribute("offset"));

	if(elem->Attribute("scale") != nullptr)
		obj._scale = std::string(elem->Attribute("scale"));



	return true;

}

bool __parse__TrafficAction(const tinyxml2::XMLElement *elem, TrafficAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("TrafficSinkAction");
	if(e != nullptr)
		__parse__TrafficSinkAction(e, *(obj.sub_TrafficSinkAction.create()));

	e = elem->FirstChildElement("TrafficSourceAction");
	if(e != nullptr)
		__parse__TrafficSourceAction(e, *(obj.sub_TrafficSourceAction.create()));

	e = elem->FirstChildElement("TrafficSwarmAction");
	if(e != nullptr)
		__parse__TrafficSwarmAction(e, *(obj.sub_TrafficSwarmAction.create()));



	return true;

}

bool __parse__TrafficDefinition(const tinyxml2::XMLElement *elem, TrafficDefinition &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ControllerDistribution");
	if(e != nullptr)
		__parse__ControllerDistribution(e, *(obj.sub_ControllerDistribution.create()));

	e = elem->FirstChildElement("VehicleCategoryDistribution");
	if(e != nullptr)
		__parse__VehicleCategoryDistribution(e, *(obj.sub_VehicleCategoryDistribution.create()));



	return true;

}

bool __parse__TrafficSignalAction(const tinyxml2::XMLElement *elem, TrafficSignalAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("TrafficSignalControllerAction");
	if(e != nullptr)
		__parse__TrafficSignalControllerAction(e, *(obj.sub_TrafficSignalControllerAction.create()));

	e = elem->FirstChildElement("TrafficSignalStateAction");
	if(e != nullptr)
		__parse__TrafficSignalStateAction(e, *(obj.sub_TrafficSignalStateAction.create()));



	return true;

}

bool __parse__TrafficSignalCondition(const tinyxml2::XMLElement *elem, TrafficSignalCondition &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	if(elem->Attribute("state") != nullptr)
		obj._state = std::string(elem->Attribute("state"));



	return true;

}

bool __parse__TrafficSignalController(const tinyxml2::XMLElement *elem, TrafficSignalController &obj) {

	if(elem->Attribute("delay") != nullptr)
		obj._delay = std::string(elem->Attribute("delay"));

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	if(elem->Attribute("reference") != nullptr)
		obj._reference = std::string(elem->Attribute("reference"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Phase");
	while(e != nullptr) {

		obj.sub_Phase.emplace_back();
		__parse__Phase(e, obj.sub_Phase.back());

		e = e->NextSiblingElement("Phase");
	}



	return true;

}

bool __parse__TrafficSignals(const tinyxml2::XMLElement *elem, TrafficSignals &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("TrafficSignalController");
	while(e != nullptr) {

		obj.sub_TrafficSignalController.emplace_back();
		__parse__TrafficSignalController(e, obj.sub_TrafficSignalController.back());

		e = e->NextSiblingElement("TrafficSignalController");
	}



	return true;

}

bool __parse__TrafficSignalControllerAction(const tinyxml2::XMLElement *elem, TrafficSignalControllerAction &obj) {

	if(elem->Attribute("phase") != nullptr)
		obj._phase = std::string(elem->Attribute("phase"));

	if(elem->Attribute("trafficSignalControllerRef") != nullptr)
		obj._trafficSignalControllerRef = std::string(elem->Attribute("trafficSignalControllerRef"));



	return true;

}

bool __parse__TrafficSignalControllerCondition(const tinyxml2::XMLElement *elem, TrafficSignalControllerCondition &obj) {

	if(elem->Attribute("phase") != nullptr)
		obj._phase = std::string(elem->Attribute("phase"));

	if(elem->Attribute("trafficSignalControllerRef") != nullptr)
		obj._trafficSignalControllerRef = std::string(elem->Attribute("trafficSignalControllerRef"));



	return true;

}

bool __parse__TrafficSignalState(const tinyxml2::XMLElement *elem, TrafficSignalState &obj) {

	if(elem->Attribute("state") != nullptr)
		obj._state = std::string(elem->Attribute("state"));

	if(elem->Attribute("trafficSignalId") != nullptr)
		obj._trafficSignalId = std::string(elem->Attribute("trafficSignalId"));



	return true;

}

bool __parse__TrafficSignalStateAction(const tinyxml2::XMLElement *elem, TrafficSignalStateAction &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	if(elem->Attribute("state") != nullptr)
		obj._state = std::string(elem->Attribute("state"));



	return true;

}

bool __parse__TrafficSinkAction(const tinyxml2::XMLElement *elem, TrafficSinkAction &obj) {

	if(elem->Attribute("radius") != nullptr)
		obj._radius = std::string(elem->Attribute("radius"));

	if(elem->Attribute("rate") != nullptr)
		obj._rate = std::string(elem->Attribute("rate"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));

	e = elem->FirstChildElement("TrafficDefinition");
	if(e != nullptr)
		__parse__TrafficDefinition(e, *(obj.sub_TrafficDefinition.create()));



	return true;

}

bool __parse__TrafficSourceAction(const tinyxml2::XMLElement *elem, TrafficSourceAction &obj) {

	if(elem->Attribute("radius") != nullptr)
		obj._radius = std::string(elem->Attribute("radius"));

	if(elem->Attribute("rate") != nullptr)
		obj._rate = std::string(elem->Attribute("rate"));

	if(elem->Attribute("velocity") != nullptr)
		obj._velocity = std::string(elem->Attribute("velocity"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));

	e = elem->FirstChildElement("TrafficDefinition");
	if(e != nullptr)
		__parse__TrafficDefinition(e, *(obj.sub_TrafficDefinition.create()));



	return true;

}

bool __parse__TrafficSwarmAction(const tinyxml2::XMLElement *elem, TrafficSwarmAction &obj) {

	if(elem->Attribute("innerRadius") != nullptr)
		obj._innerRadius = std::string(elem->Attribute("innerRadius"));

	if(elem->Attribute("numberOfVehicles") != nullptr)
		obj._numberOfVehicles = std::string(elem->Attribute("numberOfVehicles"));

	if(elem->Attribute("offset") != nullptr)
		obj._offset = std::string(elem->Attribute("offset"));

	if(elem->Attribute("semiMajorAxis") != nullptr)
		obj._semiMajorAxis = std::string(elem->Attribute("semiMajorAxis"));

	if(elem->Attribute("semiMinorAxis") != nullptr)
		obj._semiMinorAxis = std::string(elem->Attribute("semiMinorAxis"));

	if(elem->Attribute("velocity") != nullptr)
		obj._velocity = std::string(elem->Attribute("velocity"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("CentralObject");
	if(e != nullptr)
		__parse__CentralSwarmObject(e, *(obj.sub_CentralObject.create()));

	e = elem->FirstChildElement("TrafficDefinition");
	if(e != nullptr)
		__parse__TrafficDefinition(e, *(obj.sub_TrafficDefinition.create()));



	return true;

}

bool __parse__Trajectory(const tinyxml2::XMLElement *elem, Trajectory &obj) {

	if(elem->Attribute("closed") != nullptr)
		obj._closed = std::string(elem->Attribute("closed"));

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ParameterDeclarations");
	if(e != nullptr)
		__parse__ParameterDeclarations(e, *(obj.sub_ParameterDeclarations.create()));

	e = elem->FirstChildElement("Shape");
	if(e != nullptr)
		__parse__Shape(e, *(obj.sub_Shape.create()));



	return true;

}

bool __parse__TrajectoryCatalogLocation(const tinyxml2::XMLElement *elem, TrajectoryCatalogLocation &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Directory");
	if(e != nullptr)
		__parse__Directory(e, *(obj.sub_Directory.create()));



	return true;

}

bool __parse__TrajectoryFollowingMode(const tinyxml2::XMLElement *elem, TrajectoryFollowingMode &obj) {

	if(elem->Attribute("followingMode") != nullptr)
		obj._followingMode = std::string(elem->Attribute("followingMode"));



	return true;

}

bool __parse__TransitionDynamics(const tinyxml2::XMLElement *elem, TransitionDynamics &obj) {

	if(elem->Attribute("dynamicsDimension") != nullptr)
		obj._dynamicsDimension = std::string(elem->Attribute("dynamicsDimension"));

	if(elem->Attribute("dynamicsShape") != nullptr)
		obj._dynamicsShape = std::string(elem->Attribute("dynamicsShape"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__TraveledDistanceCondition(const tinyxml2::XMLElement *elem, TraveledDistanceCondition &obj) {

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__Trigger(const tinyxml2::XMLElement *elem, Trigger &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("ConditionGroup");
	while(e != nullptr) {

		obj.sub_ConditionGroup.emplace_back();
		__parse__ConditionGroup(e, obj.sub_ConditionGroup.back());

		e = e->NextSiblingElement("ConditionGroup");
	}



	return true;

}

bool __parse__TriggeringEntities(const tinyxml2::XMLElement *elem, TriggeringEntities &obj) {

	if(elem->Attribute("triggeringEntitiesRule") != nullptr)
		obj._triggeringEntitiesRule = std::string(elem->Attribute("triggeringEntitiesRule"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("EntityRef");
	while(e != nullptr) {

		obj.sub_EntityRef.emplace_back();
		__parse__EntityRef(e, obj.sub_EntityRef.back());

		e = e->NextSiblingElement("EntityRef");
	}



	return true;

}

bool __parse__UserDefinedAction(const tinyxml2::XMLElement *elem, UserDefinedAction &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("CustomCommandAction");
	if(e != nullptr)
		__parse__CustomCommandAction(e, *(obj.sub_CustomCommandAction.create()));



	return true;

}

bool __parse__UserDefinedValueCondition(const tinyxml2::XMLElement *elem, UserDefinedValueCondition &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	if(elem->Attribute("rule") != nullptr)
		obj._rule = std::string(elem->Attribute("rule"));

	if(elem->Attribute("value") != nullptr)
		obj._value = std::string(elem->Attribute("value"));



	return true;

}

bool __parse__Vehicle(const tinyxml2::XMLElement *elem, Vehicle &obj) {

	if(elem->Attribute("name") != nullptr)
		obj._name = std::string(elem->Attribute("name"));

	if(elem->Attribute("vehicleCategory") != nullptr)
		obj._vehicleCategory = std::string(elem->Attribute("vehicleCategory"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Axles");
	if(e != nullptr)
		__parse__Axles(e, *(obj.sub_Axles.create()));

	e = elem->FirstChildElement("BoundingBox");
	if(e != nullptr)
		__parse__BoundingBox(e, *(obj.sub_BoundingBox.create()));

	e = elem->FirstChildElement("ParameterDeclarations");
	if(e != nullptr)
		__parse__ParameterDeclarations(e, *(obj.sub_ParameterDeclarations.create()));

	e = elem->FirstChildElement("Performance");
	if(e != nullptr)
		__parse__Performance(e, *(obj.sub_Performance.create()));

	e = elem->FirstChildElement("Properties");
	if(e != nullptr)
		__parse__Properties(e, *(obj.sub_Properties.create()));



	return true;

}

bool __parse__VehicleCatalogLocation(const tinyxml2::XMLElement *elem, VehicleCatalogLocation &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Directory");
	if(e != nullptr)
		__parse__Directory(e, *(obj.sub_Directory.create()));



	return true;

}

bool __parse__VehicleCategoryDistribution(const tinyxml2::XMLElement *elem, VehicleCategoryDistribution &obj) {

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("VehicleCategoryDistributionEntry");
	while(e != nullptr) {

		obj.sub_VehicleCategoryDistributionEntry.emplace_back();
		__parse__VehicleCategoryDistributionEntry(e, obj.sub_VehicleCategoryDistributionEntry.back());

		e = e->NextSiblingElement("VehicleCategoryDistributionEntry");
	}



	return true;

}

bool __parse__VehicleCategoryDistributionEntry(const tinyxml2::XMLElement *elem, VehicleCategoryDistributionEntry &obj) {

	if(elem->Attribute("category") != nullptr)
		obj._category = std::string(elem->Attribute("category"));

	if(elem->Attribute("weight") != nullptr)
		obj._weight = std::string(elem->Attribute("weight"));



	return true;

}

bool __parse__Vertex(const tinyxml2::XMLElement *elem, Vertex &obj) {

	if(elem->Attribute("time") != nullptr)
		obj._time = std::string(elem->Attribute("time"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));



	return true;

}

bool __parse__VisibilityAction(const tinyxml2::XMLElement *elem, VisibilityAction &obj) {

	if(elem->Attribute("graphics") != nullptr)
		obj._graphics = std::string(elem->Attribute("graphics"));

	if(elem->Attribute("sensors") != nullptr)
		obj._sensors = std::string(elem->Attribute("sensors"));

	if(elem->Attribute("traffic") != nullptr)
		obj._traffic = std::string(elem->Attribute("traffic"));



	return true;

}

bool __parse__Waypoint(const tinyxml2::XMLElement *elem, Waypoint &obj) {

	if(elem->Attribute("routeStrategy") != nullptr)
		obj._routeStrategy = std::string(elem->Attribute("routeStrategy"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Position");
	if(e != nullptr)
		__parse__Position(e, *(obj.sub_Position.create()));



	return true;

}

bool __parse__Weather(const tinyxml2::XMLElement *elem, Weather &obj) {

	if(elem->Attribute("cloudState") != nullptr)
		obj._cloudState = std::string(elem->Attribute("cloudState"));

	const tinyxml2::XMLElement *e;

	e = elem->FirstChildElement("Fog");
	if(e != nullptr)
		__parse__Fog(e, *(obj.sub_Fog.create()));

	e = elem->FirstChildElement("Precipitation");
	if(e != nullptr)
		__parse__Precipitation(e, *(obj.sub_Precipitation.create()));

	e = elem->FirstChildElement("Sun");
	if(e != nullptr)
		__parse__Sun(e, *(obj.sub_Sun.create()));



	return true;

}

bool __parse__WorldPosition(const tinyxml2::XMLElement *elem, WorldPosition &obj) {

	if(elem->Attribute("h") != nullptr)
		obj._h = std::string(elem->Attribute("h"));

	if(elem->Attribute("p") != nullptr)
		obj._p = std::string(elem->Attribute("p"));

	if(elem->Attribute("r") != nullptr)
		obj._r = std::string(elem->Attribute("r"));

	if(elem->Attribute("x") != nullptr)
		obj._x = std::string(elem->Attribute("x"));

	if(elem->Attribute("y") != nullptr)
		obj._y = std::string(elem->Attribute("y"));

	if(elem->Attribute("z") != nullptr)
		obj._z = std::string(elem->Attribute("z"));



	return true;

}

bool __save__parameter(const std::string &name, const parameter &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__Boolean(const std::string &name, const Boolean &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__DateTime(const std::string &name, const DateTime &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__Double(const std::string &name, const Double &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__Int(const std::string &name, const Int &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__String(const std::string &name, const String &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__UnsignedInt(const std::string &name, const UnsignedInt &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__UnsignedShort(const std::string &name, const UnsignedShort &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__CloudState(const std::string &name, const CloudState &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__ConditionEdge(const std::string &name, const ConditionEdge &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__DynamicsDimension(const std::string &name, const DynamicsDimension &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__DynamicsShape(const std::string &name, const DynamicsShape &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__FollowingMode(const std::string &name, const FollowingMode &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__MiscObjectCategory(const std::string &name, const MiscObjectCategory &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__ObjectType(const std::string &name, const ObjectType &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__ParameterType(const std::string &name, const ParameterType &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__PedestrianCategory(const std::string &name, const PedestrianCategory &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__PrecipitationType(const std::string &name, const PrecipitationType &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__Priority(const std::string &name, const Priority &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__ReferenceContext(const std::string &name, const ReferenceContext &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__RelativeDistanceType(const std::string &name, const RelativeDistanceType &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__RouteStrategy(const std::string &name, const RouteStrategy &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__Rule(const std::string &name, const Rule &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__SpeedTargetValueType(const std::string &name, const SpeedTargetValueType &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__StoryboardElementState(const std::string &name, const StoryboardElementState &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__StoryboardElementType(const std::string &name, const StoryboardElementType &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__TriggeringEntitiesRule(const std::string &name, const TriggeringEntitiesRule &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__VehicleCategory(const std::string &name, const VehicleCategory &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.get() && obj->length() > 0)
		elem->SetAttribute(name.c_str(), obj->c_str());

	return true;

}

bool __save__AbsoluteSpeed(const AbsoluteSpeed &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__AbsoluteTargetLane(const AbsoluteTargetLane &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__AbsoluteTargetLaneOffset(const AbsoluteTargetLaneOffset &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__AbsoluteTargetSpeed(const AbsoluteTargetSpeed &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__AccelerationCondition(const AccelerationCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._rule.get() && obj._rule->length() > 0)
		elem->SetAttribute("rule", obj._rule->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__AcquirePositionAction(const AcquirePositionAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Act(const Act &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	for(auto &ob : obj.sub_ManeuverGroup){
		tinyxml2::XMLElement* e = doc.NewElement("ManeuverGroup");
		__save__ManeuverGroup(ob, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_StartTrigger.get()){
		tinyxml2::XMLElement* e = doc.NewElement("StartTrigger");
		__save__Trigger(*obj.sub_StartTrigger, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_StopTrigger.get()){
		tinyxml2::XMLElement* e = doc.NewElement("StopTrigger");
		__save__Trigger(*obj.sub_StopTrigger, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Action(const Action &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj.sub_GlobalAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("GlobalAction");
		__save__GlobalAction(*obj.sub_GlobalAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_PrivateAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("PrivateAction");
		__save__PrivateAction(*obj.sub_PrivateAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_UserDefinedAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("UserDefinedAction");
		__save__UserDefinedAction(*obj.sub_UserDefinedAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ActivateControllerAction(const ActivateControllerAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._lateral.get() && obj._lateral->length() > 0)
		elem->SetAttribute("lateral", obj._lateral->c_str());

	if(obj._longitudinal.get() && obj._longitudinal->length() > 0)
		elem->SetAttribute("longitudinal", obj._longitudinal->c_str());



	return true;

}

bool __save__Actors(const Actors &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._selectTriggeringEntities.get() && obj._selectTriggeringEntities->length() > 0)
		elem->SetAttribute("selectTriggeringEntities", obj._selectTriggeringEntities->c_str());

	for(auto &ob : obj.sub_EntityRef){
		tinyxml2::XMLElement* e = doc.NewElement("EntityRef");
		__save__EntityRef(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__AddEntityAction(const AddEntityAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__AssignControllerAction(const AssignControllerAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_CatalogReference.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CatalogReference");
		__save__CatalogReference(*obj.sub_CatalogReference, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Controller.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Controller");
		__save__Controller(*obj.sub_Controller, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__AssignRouteAction(const AssignRouteAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_CatalogReference.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CatalogReference");
		__save__CatalogReference(*obj.sub_CatalogReference, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Route.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Route");
		__save__Route(*obj.sub_Route, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Axle(const Axle &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._maxSteering.get() && obj._maxSteering->length() > 0)
		elem->SetAttribute("maxSteering", obj._maxSteering->c_str());

	if(obj._positionX.get() && obj._positionX->length() > 0)
		elem->SetAttribute("positionX", obj._positionX->c_str());

	if(obj._positionZ.get() && obj._positionZ->length() > 0)
		elem->SetAttribute("positionZ", obj._positionZ->c_str());

	if(obj._trackWidth.get() && obj._trackWidth->length() > 0)
		elem->SetAttribute("trackWidth", obj._trackWidth->c_str());

	if(obj._wheelDiameter.get() && obj._wheelDiameter->length() > 0)
		elem->SetAttribute("wheelDiameter", obj._wheelDiameter->c_str());



	return true;

}

bool __save__Axles(const Axles &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_AdditionalAxle){
		tinyxml2::XMLElement* e = doc.NewElement("AdditionalAxle");
		__save__Axle(ob, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_FrontAxle.get()){
		tinyxml2::XMLElement* e = doc.NewElement("FrontAxle");
		__save__Axle(*obj.sub_FrontAxle, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RearAxle.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RearAxle");
		__save__Axle(*obj.sub_RearAxle, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__BoundingBox(const BoundingBox &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Center.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Center");
		__save__Center(*obj.sub_Center, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Dimensions.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Dimensions");
		__save__Dimensions(*obj.sub_Dimensions, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ByEntityCondition(const ByEntityCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_EntityCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("EntityCondition");
		__save__EntityCondition(*obj.sub_EntityCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TriggeringEntities.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TriggeringEntities");
		__save__TriggeringEntities(*obj.sub_TriggeringEntities, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ByObjectType(const ByObjectType &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._type.get() && obj._type->length() > 0)
		elem->SetAttribute("type", obj._type->c_str());



	return true;

}

bool __save__ByType(const ByType &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._objectType.get() && obj._objectType->length() > 0)
		elem->SetAttribute("objectType", obj._objectType->c_str());



	return true;

}

bool __save__ByValueCondition(const ByValueCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_ParameterCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterCondition");
		__save__ParameterCondition(*obj.sub_ParameterCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_SimulationTimeCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("SimulationTimeCondition");
		__save__SimulationTimeCondition(*obj.sub_SimulationTimeCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_StoryboardElementStateCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("StoryboardElementStateCondition");
		__save__StoryboardElementStateCondition(*obj.sub_StoryboardElementStateCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TimeOfDayCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TimeOfDayCondition");
		__save__TimeOfDayCondition(*obj.sub_TimeOfDayCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrafficSignalCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficSignalCondition");
		__save__TrafficSignalCondition(*obj.sub_TrafficSignalCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrafficSignalControllerCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficSignalControllerCondition");
		__save__TrafficSignalControllerCondition(*obj.sub_TrafficSignalControllerCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_UserDefinedValueCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("UserDefinedValueCondition");
		__save__UserDefinedValueCondition(*obj.sub_UserDefinedValueCondition, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Catalog(const Catalog &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	for(auto &ob : obj.sub_Controller){
		tinyxml2::XMLElement* e = doc.NewElement("Controller");
		__save__Controller(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Environment){
		tinyxml2::XMLElement* e = doc.NewElement("Environment");
		__save__Environment(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Maneuver){
		tinyxml2::XMLElement* e = doc.NewElement("Maneuver");
		__save__Maneuver(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_MiscObject){
		tinyxml2::XMLElement* e = doc.NewElement("MiscObject");
		__save__MiscObject(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Pedestrian){
		tinyxml2::XMLElement* e = doc.NewElement("Pedestrian");
		__save__Pedestrian(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Route){
		tinyxml2::XMLElement* e = doc.NewElement("Route");
		__save__Route(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Trajectory){
		tinyxml2::XMLElement* e = doc.NewElement("Trajectory");
		__save__Trajectory(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Vehicle){
		tinyxml2::XMLElement* e = doc.NewElement("Vehicle");
		__save__Vehicle(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__CatalogLocations(const CatalogLocations &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_ControllerCatalog.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ControllerCatalog");
		__save__ControllerCatalogLocation(*obj.sub_ControllerCatalog, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_EnvironmentCatalog.get()){
		tinyxml2::XMLElement* e = doc.NewElement("EnvironmentCatalog");
		__save__EnvironmentCatalogLocation(*obj.sub_EnvironmentCatalog, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ManeuverCatalog.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ManeuverCatalog");
		__save__ManeuverCatalogLocation(*obj.sub_ManeuverCatalog, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_MiscObjectCatalog.get()){
		tinyxml2::XMLElement* e = doc.NewElement("MiscObjectCatalog");
		__save__MiscObjectCatalogLocation(*obj.sub_MiscObjectCatalog, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_PedestrianCatalog.get()){
		tinyxml2::XMLElement* e = doc.NewElement("PedestrianCatalog");
		__save__PedestrianCatalogLocation(*obj.sub_PedestrianCatalog, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RouteCatalog.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RouteCatalog");
		__save__RouteCatalogLocation(*obj.sub_RouteCatalog, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrajectoryCatalog.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrajectoryCatalog");
		__save__TrajectoryCatalogLocation(*obj.sub_TrajectoryCatalog, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_VehicleCatalog.get()){
		tinyxml2::XMLElement* e = doc.NewElement("VehicleCatalog");
		__save__VehicleCatalogLocation(*obj.sub_VehicleCatalog, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__CatalogReference(const CatalogReference &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._catalogName.get() && obj._catalogName->length() > 0)
		elem->SetAttribute("catalogName", obj._catalogName->c_str());

	if(obj._entryName.get() && obj._entryName->length() > 0)
		elem->SetAttribute("entryName", obj._entryName->c_str());

	if(obj.sub_ParameterAssignments.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterAssignments");
		__save__ParameterAssignments(*obj.sub_ParameterAssignments, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Center(const Center &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._x.get() && obj._x->length() > 0)
		elem->SetAttribute("x", obj._x->c_str());

	if(obj._y.get() && obj._y->length() > 0)
		elem->SetAttribute("y", obj._y->c_str());

	if(obj._z.get() && obj._z->length() > 0)
		elem->SetAttribute("z", obj._z->c_str());



	return true;

}

bool __save__CentralSwarmObject(const CentralSwarmObject &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());



	return true;

}

bool __save__Clothoid(const Clothoid &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._curvature.get() && obj._curvature->length() > 0)
		elem->SetAttribute("curvature", obj._curvature->c_str());

	if(obj._curvatureDot.get() && obj._curvatureDot->length() > 0)
		elem->SetAttribute("curvatureDot", obj._curvatureDot->c_str());

	if(obj._length.get() && obj._length->length() > 0)
		elem->SetAttribute("length", obj._length->c_str());

	if(obj._startTime.get() && obj._startTime->length() > 0)
		elem->SetAttribute("startTime", obj._startTime->c_str());

	if(obj._stopTime.get() && obj._stopTime->length() > 0)
		elem->SetAttribute("stopTime", obj._stopTime->c_str());

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__CollisionCondition(const CollisionCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_ByType.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ByType");
		__save__ByObjectType(*obj.sub_ByType, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_EntityRef.get()){
		tinyxml2::XMLElement* e = doc.NewElement("EntityRef");
		__save__EntityRef(*obj.sub_EntityRef, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Condition(const Condition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._conditionEdge.get() && obj._conditionEdge->length() > 0)
		elem->SetAttribute("conditionEdge", obj._conditionEdge->c_str());

	if(obj._delay.get() && obj._delay->length() > 0)
		elem->SetAttribute("delay", obj._delay->c_str());

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj.sub_ByEntityCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ByEntityCondition");
		__save__ByEntityCondition(*obj.sub_ByEntityCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ByValueCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ByValueCondition");
		__save__ByValueCondition(*obj.sub_ByValueCondition, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ConditionGroup(const ConditionGroup &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_Condition){
		tinyxml2::XMLElement* e = doc.NewElement("Condition");
		__save__Condition(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Controller(const Controller &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj.sub_ParameterDeclarations.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterDeclarations");
		__save__ParameterDeclarations(*obj.sub_ParameterDeclarations, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Properties.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Properties");
		__save__Properties(*obj.sub_Properties, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ControllerAction(const ControllerAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_AssignControllerAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("AssignControllerAction");
		__save__AssignControllerAction(*obj.sub_AssignControllerAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_OverrideControllerValueAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("OverrideControllerValueAction");
		__save__OverrideControllerValueAction(*obj.sub_OverrideControllerValueAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ControllerCatalogLocation(const ControllerCatalogLocation &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Directory.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Directory");
		__save__Directory(*obj.sub_Directory, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ControllerDistribution(const ControllerDistribution &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_ControllerDistributionEntry){
		tinyxml2::XMLElement* e = doc.NewElement("ControllerDistributionEntry");
		__save__ControllerDistributionEntry(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ControllerDistributionEntry(const ControllerDistributionEntry &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._weight.get() && obj._weight->length() > 0)
		elem->SetAttribute("weight", obj._weight->c_str());

	if(obj.sub_CatalogReference.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CatalogReference");
		__save__CatalogReference(*obj.sub_CatalogReference, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Controller.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Controller");
		__save__Controller(*obj.sub_Controller, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ControlPoint(const ControlPoint &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._time.get() && obj._time->length() > 0)
		elem->SetAttribute("time", obj._time->c_str());

	if(obj._weight.get() && obj._weight->length() > 0)
		elem->SetAttribute("weight", obj._weight->c_str());

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__CustomCommandAction(const CustomCommandAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {



	return true;

}

bool __save__DeleteEntityAction(const DeleteEntityAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {



	return true;

}

bool __save__Dimensions(const Dimensions &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._height.get() && obj._height->length() > 0)
		elem->SetAttribute("height", obj._height->c_str());

	if(obj._length.get() && obj._length->length() > 0)
		elem->SetAttribute("length", obj._length->c_str());

	if(obj._width.get() && obj._width->length() > 0)
		elem->SetAttribute("width", obj._width->c_str());



	return true;

}

bool __save__Directory(const Directory &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._path.get() && obj._path->length() > 0)
		elem->SetAttribute("path", obj._path->c_str());



	return true;

}

bool __save__DistanceCondition(const DistanceCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._alongRoute.get() && obj._alongRoute->length() > 0)
		elem->SetAttribute("alongRoute", obj._alongRoute->c_str());

	if(obj._freespace.get() && obj._freespace->length() > 0)
		elem->SetAttribute("freespace", obj._freespace->c_str());

	if(obj._rule.get() && obj._rule->length() > 0)
		elem->SetAttribute("rule", obj._rule->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__DynamicConstraints(const DynamicConstraints &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._maxAcceleration.get() && obj._maxAcceleration->length() > 0)
		elem->SetAttribute("maxAcceleration", obj._maxAcceleration->c_str());

	if(obj._maxDeceleration.get() && obj._maxDeceleration->length() > 0)
		elem->SetAttribute("maxDeceleration", obj._maxDeceleration->c_str());

	if(obj._maxSpeed.get() && obj._maxSpeed->length() > 0)
		elem->SetAttribute("maxSpeed", obj._maxSpeed->c_str());



	return true;

}

bool __save__EndOfRoadCondition(const EndOfRoadCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._duration.get() && obj._duration->length() > 0)
		elem->SetAttribute("duration", obj._duration->c_str());



	return true;

}

bool __save__Entities(const Entities &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_EntitySelection){
		tinyxml2::XMLElement* e = doc.NewElement("EntitySelection");
		__save__EntitySelection(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_ScenarioObject){
		tinyxml2::XMLElement* e = doc.NewElement("ScenarioObject");
		__save__ScenarioObject(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__EntityAction(const EntityAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj.sub_AddEntityAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("AddEntityAction");
		__save__AddEntityAction(*obj.sub_AddEntityAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_DeleteEntityAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("DeleteEntityAction");
		__save__DeleteEntityAction(*obj.sub_DeleteEntityAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__EntityCondition(const EntityCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_AccelerationCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("AccelerationCondition");
		__save__AccelerationCondition(*obj.sub_AccelerationCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_CollisionCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CollisionCondition");
		__save__CollisionCondition(*obj.sub_CollisionCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_DistanceCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("DistanceCondition");
		__save__DistanceCondition(*obj.sub_DistanceCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_EndOfRoadCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("EndOfRoadCondition");
		__save__EndOfRoadCondition(*obj.sub_EndOfRoadCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_OffroadCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("OffroadCondition");
		__save__OffroadCondition(*obj.sub_OffroadCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ReachPositionCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ReachPositionCondition");
		__save__ReachPositionCondition(*obj.sub_ReachPositionCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RelativeDistanceCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RelativeDistanceCondition");
		__save__RelativeDistanceCondition(*obj.sub_RelativeDistanceCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RelativeSpeedCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RelativeSpeedCondition");
		__save__RelativeSpeedCondition(*obj.sub_RelativeSpeedCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_SpeedCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("SpeedCondition");
		__save__SpeedCondition(*obj.sub_SpeedCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_StandStillCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("StandStillCondition");
		__save__StandStillCondition(*obj.sub_StandStillCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TimeHeadwayCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TimeHeadwayCondition");
		__save__TimeHeadwayCondition(*obj.sub_TimeHeadwayCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TimeToCollisionCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TimeToCollisionCondition");
		__save__TimeToCollisionCondition(*obj.sub_TimeToCollisionCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TraveledDistanceCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TraveledDistanceCondition");
		__save__TraveledDistanceCondition(*obj.sub_TraveledDistanceCondition, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__EntityRef(const EntityRef &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());



	return true;

}

bool __save__EntitySelection(const EntitySelection &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj.sub_Members.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Members");
		__save__SelectedEntities(*obj.sub_Members, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Environment(const Environment &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj.sub_ParameterDeclarations.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterDeclarations");
		__save__ParameterDeclarations(*obj.sub_ParameterDeclarations, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RoadCondition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RoadCondition");
		__save__RoadCondition(*obj.sub_RoadCondition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TimeOfDay.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TimeOfDay");
		__save__TimeOfDay(*obj.sub_TimeOfDay, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Weather.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Weather");
		__save__Weather(*obj.sub_Weather, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__EnvironmentAction(const EnvironmentAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_CatalogReference.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CatalogReference");
		__save__CatalogReference(*obj.sub_CatalogReference, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Environment.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Environment");
		__save__Environment(*obj.sub_Environment, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__EnvironmentCatalogLocation(const EnvironmentCatalogLocation &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Directory.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Directory");
		__save__Directory(*obj.sub_Directory, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Event(const Event &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._maximumExecutionCount.get() && obj._maximumExecutionCount->length() > 0)
		elem->SetAttribute("maximumExecutionCount", obj._maximumExecutionCount->c_str());

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj._priority.get() && obj._priority->length() > 0)
		elem->SetAttribute("priority", obj._priority->c_str());

	for(auto &ob : obj.sub_Action){
		tinyxml2::XMLElement* e = doc.NewElement("Action");
		__save__Action(ob, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_StartTrigger.get()){
		tinyxml2::XMLElement* e = doc.NewElement("StartTrigger");
		__save__Trigger(*obj.sub_StartTrigger, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__File(const File &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._filepath.get() && obj._filepath->length() > 0)
		elem->SetAttribute("filepath", obj._filepath->c_str());



	return true;

}

bool __save__FileHeader(const FileHeader &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._author.get() && obj._author->length() > 0)
		elem->SetAttribute("author", obj._author->c_str());
	else
		elem->SetAttribute("author", "");

	if(obj._date.get() && obj._date->length() > 0)
		elem->SetAttribute("date", obj._date->c_str());
    else
        elem->SetAttribute("date", "");

	if(obj._description.get() && obj._description->length() > 0)
		elem->SetAttribute("description", obj._description->c_str());
    else
        elem->SetAttribute("description", "");
	if(obj._revMajor.get() && obj._revMajor->length() > 0)
		elem->SetAttribute("revMajor", obj._revMajor->c_str());
    else
        elem->SetAttribute("revMajor", "");
	if(obj._revMinor.get() && obj._revMinor->length() > 0)
		elem->SetAttribute("revMinor", obj._revMinor->c_str());
    else
        elem->SetAttribute("revMinor", "");



	return true;

}

bool __save__FinalSpeed(const FinalSpeed &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_AbsoluteSpeed.get()){
		tinyxml2::XMLElement* e = doc.NewElement("AbsoluteSpeed");
		__save__AbsoluteSpeed(*obj.sub_AbsoluteSpeed, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RelativeSpeedToMaster.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RelativeSpeedToMaster");
		__save__RelativeSpeedToMaster(*obj.sub_RelativeSpeedToMaster, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Fog(const Fog &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._visualRange.get() && obj._visualRange->length() > 0)
		elem->SetAttribute("visualRange", obj._visualRange->c_str());

	if(obj.sub_BoundingBox.get()){
		tinyxml2::XMLElement* e = doc.NewElement("BoundingBox");
		__save__BoundingBox(*obj.sub_BoundingBox, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__FollowTrajectoryAction(const FollowTrajectoryAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_CatalogReference.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CatalogReference");
		__save__CatalogReference(*obj.sub_CatalogReference, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TimeReference.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TimeReference");
		__save__TimeReference(*obj.sub_TimeReference, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Trajectory.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Trajectory");
		__save__Trajectory(*obj.sub_Trajectory, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrajectoryFollowingMode.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrajectoryFollowingMode");
		__save__TrajectoryFollowingMode(*obj.sub_TrajectoryFollowingMode, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__GlobalAction(const GlobalAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_EntityAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("EntityAction");
		__save__EntityAction(*obj.sub_EntityAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_EnvironmentAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("EnvironmentAction");
		__save__EnvironmentAction(*obj.sub_EnvironmentAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_InfrastructureAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("InfrastructureAction");
		__save__InfrastructureAction(*obj.sub_InfrastructureAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ParameterAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterAction");
		__save__ParameterAction(*obj.sub_ParameterAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrafficAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficAction");
		__save__TrafficAction(*obj.sub_TrafficAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__InfrastructureAction(const InfrastructureAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_TrafficSignalAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficSignalAction");
		__save__TrafficSignalAction(*obj.sub_TrafficSignalAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Init(const Init &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Actions.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Actions");
		__save__InitActions(*obj.sub_Actions, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__InitActions(const InitActions &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_GlobalAction){
		tinyxml2::XMLElement* e = doc.NewElement("GlobalAction");
		__save__GlobalAction(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Private){
		tinyxml2::XMLElement* e = doc.NewElement("Private");
		__save__Private(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_UserDefinedAction){
		tinyxml2::XMLElement* e = doc.NewElement("UserDefinedAction");
		__save__UserDefinedAction(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__InRoutePosition(const InRoutePosition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_FromCurrentEntity.get()){
		tinyxml2::XMLElement* e = doc.NewElement("FromCurrentEntity");
		__save__PositionOfCurrentEntity(*obj.sub_FromCurrentEntity, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_FromLaneCoordinates.get()){
		tinyxml2::XMLElement* e = doc.NewElement("FromLaneCoordinates");
		__save__PositionInLaneCoordinates(*obj.sub_FromLaneCoordinates, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_FromRoadCoordinates.get()){
		tinyxml2::XMLElement* e = doc.NewElement("FromRoadCoordinates");
		__save__PositionInRoadCoordinates(*obj.sub_FromRoadCoordinates, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Knot(const Knot &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__LaneChangeAction(const LaneChangeAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._targetLaneOffset.get() && obj._targetLaneOffset->length() > 0)
		elem->SetAttribute("targetLaneOffset", obj._targetLaneOffset->c_str());

	if(obj.sub_LaneChangeActionDynamics.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LaneChangeActionDynamics");
		__save__TransitionDynamics(*obj.sub_LaneChangeActionDynamics, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_LaneChangeTarget.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LaneChangeTarget");
		__save__LaneChangeTarget(*obj.sub_LaneChangeTarget, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__LaneChangeTarget(const LaneChangeTarget &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_AbsoluteTargetLane.get()){
		tinyxml2::XMLElement* e = doc.NewElement("AbsoluteTargetLane");
		__save__AbsoluteTargetLane(*obj.sub_AbsoluteTargetLane, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RelativeTargetLane.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RelativeTargetLane");
		__save__RelativeTargetLane(*obj.sub_RelativeTargetLane, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__LaneOffsetAction(const LaneOffsetAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._continuous.get() && obj._continuous->length() > 0)
		elem->SetAttribute("continuous", obj._continuous->c_str());

	if(obj.sub_LaneOffsetActionDynamics.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LaneOffsetActionDynamics");
		__save__LaneOffsetActionDynamics(*obj.sub_LaneOffsetActionDynamics, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_LaneOffsetTarget.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LaneOffsetTarget");
		__save__LaneOffsetTarget(*obj.sub_LaneOffsetTarget, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__LaneOffsetActionDynamics(const LaneOffsetActionDynamics &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._dynamicsShape.get() && obj._dynamicsShape->length() > 0)
		elem->SetAttribute("dynamicsShape", obj._dynamicsShape->c_str());

	if(obj._maxLateralAcc.get() && obj._maxLateralAcc->length() > 0)
		elem->SetAttribute("maxLateralAcc", obj._maxLateralAcc->c_str());



	return true;

}

bool __save__LaneOffsetTarget(const LaneOffsetTarget &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_AbsoluteTargetLaneOffset.get()){
		tinyxml2::XMLElement* e = doc.NewElement("AbsoluteTargetLaneOffset");
		__save__AbsoluteTargetLaneOffset(*obj.sub_AbsoluteTargetLaneOffset, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RelativeTargetLaneOffset.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RelativeTargetLaneOffset");
		__save__RelativeTargetLaneOffset(*obj.sub_RelativeTargetLaneOffset, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__LanePosition(const LanePosition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._laneId.get() && obj._laneId->length() > 0)
		elem->SetAttribute("laneId", obj._laneId->c_str());

	if(obj._offset.get() && obj._offset->length() > 0)
		elem->SetAttribute("offset", obj._offset->c_str());

	if(obj._roadId.get() && obj._roadId->length() > 0)
		elem->SetAttribute("roadId", obj._roadId->c_str());

	if(obj._s.get() && obj._s->length() > 0)
		elem->SetAttribute("s", obj._s->c_str());

	if(obj.sub_Orientation.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Orientation");
		__save__Orientation(*obj.sub_Orientation, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__LateralAction(const LateralAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_LaneChangeAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LaneChangeAction");
		__save__LaneChangeAction(*obj.sub_LaneChangeAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_LaneOffsetAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LaneOffsetAction");
		__save__LaneOffsetAction(*obj.sub_LaneOffsetAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_LateralDistanceAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LateralDistanceAction");
		__save__LateralDistanceAction(*obj.sub_LateralDistanceAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__LateralDistanceAction(const LateralDistanceAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._continuous.get() && obj._continuous->length() > 0)
		elem->SetAttribute("continuous", obj._continuous->c_str());

	if(obj._distance.get() && obj._distance->length() > 0)
		elem->SetAttribute("distance", obj._distance->c_str());

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj._freespace.get() && obj._freespace->length() > 0)
		elem->SetAttribute("freespace", obj._freespace->c_str());

	if(obj.sub_DynamicConstraints.get()){
		tinyxml2::XMLElement* e = doc.NewElement("DynamicConstraints");
		__save__DynamicConstraints(*obj.sub_DynamicConstraints, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__LongitudinalAction(const LongitudinalAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_LongitudinalDistanceAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LongitudinalDistanceAction");
		__save__LongitudinalDistanceAction(*obj.sub_LongitudinalDistanceAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_SpeedAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("SpeedAction");
		__save__SpeedAction(*obj.sub_SpeedAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__LongitudinalDistanceAction(const LongitudinalDistanceAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._continuous.get() && obj._continuous->length() > 0)
		elem->SetAttribute("continuous", obj._continuous->c_str());

	if(obj._distance.get() && obj._distance->length() > 0)
		elem->SetAttribute("distance", obj._distance->c_str());

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj._freespace.get() && obj._freespace->length() > 0)
		elem->SetAttribute("freespace", obj._freespace->c_str());

	if(obj._timeGap.get() && obj._timeGap->length() > 0)
		elem->SetAttribute("timeGap", obj._timeGap->c_str());

	if(obj.sub_DynamicConstraints.get()){
		tinyxml2::XMLElement* e = doc.NewElement("DynamicConstraints");
		__save__DynamicConstraints(*obj.sub_DynamicConstraints, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Maneuver(const Maneuver &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	for(auto &ob : obj.sub_Event){
		tinyxml2::XMLElement* e = doc.NewElement("Event");
		__save__Event(ob, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ParameterDeclarations.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterDeclarations");
		__save__ParameterDeclarations(*obj.sub_ParameterDeclarations, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ManeuverCatalogLocation(const ManeuverCatalogLocation &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Directory.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Directory");
		__save__Directory(*obj.sub_Directory, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ManeuverGroup(const ManeuverGroup &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._maximumExecutionCount.get() && obj._maximumExecutionCount->length() > 0)
		elem->SetAttribute("maximumExecutionCount", obj._maximumExecutionCount->c_str());

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj.sub_Actors.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Actors");
		__save__Actors(*obj.sub_Actors, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_CatalogReference){
		tinyxml2::XMLElement* e = doc.NewElement("CatalogReference");
		__save__CatalogReference(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Maneuver){
		tinyxml2::XMLElement* e = doc.NewElement("Maneuver");
		__save__Maneuver(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__MiscObject(const MiscObject &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._mass.get() && obj._mass->length() > 0)
		elem->SetAttribute("mass", obj._mass->c_str());

	if(obj._miscObjectCategory.get() && obj._miscObjectCategory->length() > 0)
		elem->SetAttribute("miscObjectCategory", obj._miscObjectCategory->c_str());

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj.sub_BoundingBox.get()){
		tinyxml2::XMLElement* e = doc.NewElement("BoundingBox");
		__save__BoundingBox(*obj.sub_BoundingBox, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ParameterDeclarations.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterDeclarations");
		__save__ParameterDeclarations(*obj.sub_ParameterDeclarations, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Properties.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Properties");
		__save__Properties(*obj.sub_Properties, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__MiscObjectCatalogLocation(const MiscObjectCatalogLocation &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Directory.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Directory");
		__save__Directory(*obj.sub_Directory, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ModifyRule(const ModifyRule &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_AddValue.get()){
		tinyxml2::XMLElement* e = doc.NewElement("AddValue");
		__save__ParameterAddValueRule(*obj.sub_AddValue, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_MultiplyByValue.get()){
		tinyxml2::XMLElement* e = doc.NewElement("MultiplyByValue");
		__save__ParameterMultiplyByValueRule(*obj.sub_MultiplyByValue, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__None(const None &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {



	return true;

}

bool __save__Nurbs(const Nurbs &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._order.get() && obj._order->length() > 0)
		elem->SetAttribute("order", obj._order->c_str());

	for(auto &ob : obj.sub_ControlPoint){
		tinyxml2::XMLElement* e = doc.NewElement("ControlPoint");
		__save__ControlPoint(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Knot){
		tinyxml2::XMLElement* e = doc.NewElement("Knot");
		__save__Knot(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ObjectController(const ObjectController &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_CatalogReference.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CatalogReference");
		__save__CatalogReference(*obj.sub_CatalogReference, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Controller.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Controller");
		__save__Controller(*obj.sub_Controller, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__OffroadCondition(const OffroadCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._duration.get() && obj._duration->length() > 0)
		elem->SetAttribute("duration", obj._duration->c_str());



	return true;

}

bool __save__OpenScenario(const OpenScenario &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {
	if (obj.sub_FileHeader.get()) {
	  tinyxml2::XMLElement *e = doc.NewElement("FileHeader");
	  __save__FileHeader(*obj.sub_FileHeader, e, doc);
	  elem->InsertEndChild(e);
	}

	if (obj.sub_ParameterDeclarations.get()) {
       tinyxml2::XMLElement *e = doc.NewElement("ParameterDeclarations");
       __save__ParameterDeclarations(*obj.sub_ParameterDeclarations, e, doc);
       elem->InsertEndChild(e);
    }

	if(obj.sub_Catalog.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Catalog");
		__save__Catalog(*obj.sub_Catalog, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_CatalogLocations.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CatalogLocations");
		__save__CatalogLocations(*obj.sub_CatalogLocations, e, doc);
		elem->InsertEndChild(e);
	}

    if (obj.sub_RoadNetwork.get()) {
      tinyxml2::XMLElement *e = doc.NewElement("RoadNetwork");
      __save__RoadNetwork(*obj.sub_RoadNetwork, e, doc);
      elem->InsertEndChild(e);
    }

	if(obj.sub_Entities.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Entities");
		__save__Entities(*obj.sub_Entities, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Storyboard.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Storyboard");
		__save__Storyboard(*obj.sub_Storyboard, e, doc);
		elem->InsertEndChild(e);
	}

	return true;
}

bool __save__Orientation(const Orientation &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._h.get() && obj._h->length() > 0)
		elem->SetAttribute("h", obj._h->c_str());

	if(obj._p.get() && obj._p->length() > 0)
		elem->SetAttribute("p", obj._p->c_str());

	if(obj._r.get() && obj._r->length() > 0)
		elem->SetAttribute("r", obj._r->c_str());

	if(obj._type.get() && obj._type->length() > 0)
		elem->SetAttribute("type", obj._type->c_str());



	return true;

}

bool __save__OverrideBrakeAction(const OverrideBrakeAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._active.get() && obj._active->length() > 0)
		elem->SetAttribute("active", obj._active->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__OverrideClutchAction(const OverrideClutchAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._active.get() && obj._active->length() > 0)
		elem->SetAttribute("active", obj._active->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__OverrideControllerValueAction(const OverrideControllerValueAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Brake.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Brake");
		__save__OverrideBrakeAction(*obj.sub_Brake, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Clutch.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Clutch");
		__save__OverrideClutchAction(*obj.sub_Clutch, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Gear.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Gear");
		__save__OverrideGearAction(*obj.sub_Gear, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ParkingBrake.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParkingBrake");
		__save__OverrideParkingBrakeAction(*obj.sub_ParkingBrake, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_SteeringWheel.get()){
		tinyxml2::XMLElement* e = doc.NewElement("SteeringWheel");
		__save__OverrideSteeringWheelAction(*obj.sub_SteeringWheel, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Throttle.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Throttle");
		__save__OverrideThrottleAction(*obj.sub_Throttle, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__OverrideGearAction(const OverrideGearAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._active.get() && obj._active->length() > 0)
		elem->SetAttribute("active", obj._active->c_str());

	if(obj._number.get() && obj._number->length() > 0)
		elem->SetAttribute("number", obj._number->c_str());



	return true;

}

bool __save__OverrideParkingBrakeAction(const OverrideParkingBrakeAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._active.get() && obj._active->length() > 0)
		elem->SetAttribute("active", obj._active->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__OverrideSteeringWheelAction(const OverrideSteeringWheelAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._active.get() && obj._active->length() > 0)
		elem->SetAttribute("active", obj._active->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__OverrideThrottleAction(const OverrideThrottleAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._active.get() && obj._active->length() > 0)
		elem->SetAttribute("active", obj._active->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__ParameterAction(const ParameterAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._parameterRef.get() && obj._parameterRef->length() > 0)
		elem->SetAttribute("parameterRef", obj._parameterRef->c_str());

	if(obj.sub_ModifyAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ModifyAction");
		__save__ParameterModifyAction(*obj.sub_ModifyAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_SetAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("SetAction");
		__save__ParameterSetAction(*obj.sub_SetAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ParameterAddValueRule(const ParameterAddValueRule &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__ParameterAssignment(const ParameterAssignment &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._parameterRef.get() && obj._parameterRef->length() > 0)
		elem->SetAttribute("parameterRef", obj._parameterRef->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__ParameterAssignments(const ParameterAssignments &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_ParameterAssignment){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterAssignment");
		__save__ParameterAssignment(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ParameterCondition(const ParameterCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._parameterRef.get() && obj._parameterRef->length() > 0)
		elem->SetAttribute("parameterRef", obj._parameterRef->c_str());

	if(obj._rule.get() && obj._rule->length() > 0)
		elem->SetAttribute("rule", obj._rule->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__ParameterDeclaration(const ParameterDeclaration &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj._parameterType.get() && obj._parameterType->length() > 0)
		elem->SetAttribute("parameterType", obj._parameterType->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());
    else
        elem->SetAttribute("value", "");


	return true;

}

bool __save__ParameterDeclarations(const ParameterDeclarations &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_ParameterDeclaration){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterDeclaration");
		__save__ParameterDeclaration(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ParameterModifyAction(const ParameterModifyAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Rule.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Rule");
		__save__ModifyRule(*obj.sub_Rule, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ParameterMultiplyByValueRule(const ParameterMultiplyByValueRule &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__ParameterSetAction(const ParameterSetAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__Pedestrian(const Pedestrian &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._mass.get() && obj._mass->length() > 0)
		elem->SetAttribute("mass", obj._mass->c_str());

	if(obj._model.get() && obj._model->length() > 0)
		elem->SetAttribute("model", obj._model->c_str());

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj._pedestrianCategory.get() && obj._pedestrianCategory->length() > 0)
		elem->SetAttribute("pedestrianCategory", obj._pedestrianCategory->c_str());

	if(obj.sub_BoundingBox.get()){
		tinyxml2::XMLElement* e = doc.NewElement("BoundingBox");
		__save__BoundingBox(*obj.sub_BoundingBox, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ParameterDeclarations.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterDeclarations");
		__save__ParameterDeclarations(*obj.sub_ParameterDeclarations, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Properties.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Properties");
		__save__Properties(*obj.sub_Properties, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__PedestrianCatalogLocation(const PedestrianCatalogLocation &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Directory.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Directory");
		__save__Directory(*obj.sub_Directory, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Performance(const Performance &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._maxAcceleration.get() && obj._maxAcceleration->length() > 0)
		elem->SetAttribute("maxAcceleration", obj._maxAcceleration->c_str());

	if(obj._maxDeceleration.get() && obj._maxDeceleration->length() > 0)
		elem->SetAttribute("maxDeceleration", obj._maxDeceleration->c_str());

	if(obj._maxSpeed.get() && obj._maxSpeed->length() > 0)
		elem->SetAttribute("maxSpeed", obj._maxSpeed->c_str());



	return true;

}

bool __save__Phase(const Phase &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._duration.get() && obj._duration->length() > 0)
		elem->SetAttribute("duration", obj._duration->c_str());

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	for(auto &ob : obj.sub_TrafficSignalState){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficSignalState");
		__save__TrafficSignalState(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Polyline(const Polyline &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_Vertex){
		tinyxml2::XMLElement* e = doc.NewElement("Vertex");
		__save__Vertex(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Position(const Position &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_LanePosition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LanePosition");
		__save__LanePosition(*obj.sub_LanePosition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RelativeLanePosition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RelativeLanePosition");
		__save__RelativeLanePosition(*obj.sub_RelativeLanePosition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RelativeObjectPosition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RelativeObjectPosition");
		__save__RelativeObjectPosition(*obj.sub_RelativeObjectPosition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RelativeRoadPosition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RelativeRoadPosition");
		__save__RelativeRoadPosition(*obj.sub_RelativeRoadPosition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RelativeWorldPosition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RelativeWorldPosition");
		__save__RelativeWorldPosition(*obj.sub_RelativeWorldPosition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RoadPosition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RoadPosition");
		__save__RoadPosition(*obj.sub_RoadPosition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RoutePosition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RoutePosition");
		__save__RoutePosition(*obj.sub_RoutePosition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_WorldPosition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("WorldPosition");
		__save__WorldPosition(*obj.sub_WorldPosition, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__PositionInLaneCoordinates(const PositionInLaneCoordinates &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._laneId.get() && obj._laneId->length() > 0)
		elem->SetAttribute("laneId", obj._laneId->c_str());

	if(obj._laneOffset.get() && obj._laneOffset->length() > 0)
		elem->SetAttribute("laneOffset", obj._laneOffset->c_str());

	if(obj._pathS.get() && obj._pathS->length() > 0)
		elem->SetAttribute("pathS", obj._pathS->c_str());



	return true;

}

bool __save__PositionInRoadCoordinates(const PositionInRoadCoordinates &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._pathS.get() && obj._pathS->length() > 0)
		elem->SetAttribute("pathS", obj._pathS->c_str());

	if(obj._t.get() && obj._t->length() > 0)
		elem->SetAttribute("t", obj._t->c_str());



	return true;

}

bool __save__PositionOfCurrentEntity(const PositionOfCurrentEntity &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());



	return true;

}

bool __save__Precipitation(const Precipitation &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._intensity.get() && obj._intensity->length() > 0)
		elem->SetAttribute("intensity", obj._intensity->c_str());

	if(obj._precipitationType.get() && obj._precipitationType->length() > 0)
		elem->SetAttribute("precipitationType", obj._precipitationType->c_str());



	return true;

}

bool __save__Private(const Private &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	for(auto &ob : obj.sub_PrivateAction){
		tinyxml2::XMLElement* e = doc.NewElement("PrivateAction");
		__save__PrivateAction(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__PrivateAction(const PrivateAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_ActivateControllerAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ActivateControllerAction");
		__save__ActivateControllerAction(*obj.sub_ActivateControllerAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ControllerAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ControllerAction");
		__save__ControllerAction(*obj.sub_ControllerAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_LateralAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LateralAction");
		__save__LateralAction(*obj.sub_LateralAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_LongitudinalAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LongitudinalAction");
		__save__LongitudinalAction(*obj.sub_LongitudinalAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RoutingAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RoutingAction");
		__save__RoutingAction(*obj.sub_RoutingAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_SynchronizeAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("SynchronizeAction");
		__save__SynchronizeAction(*obj.sub_SynchronizeAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TeleportAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TeleportAction");
		__save__TeleportAction(*obj.sub_TeleportAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_VisibilityAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("VisibilityAction");
		__save__VisibilityAction(*obj.sub_VisibilityAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Properties(const Properties &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_File){
		tinyxml2::XMLElement* e = doc.NewElement("File");
		__save__File(ob, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Property){
		tinyxml2::XMLElement* e = doc.NewElement("Property");
		__save__Property(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Property(const Property &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());
    else
        elem->SetAttribute("value", "");

	return true;

}

bool __save__ReachPositionCondition(const ReachPositionCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._tolerance.get() && obj._tolerance->length() > 0)
		elem->SetAttribute("tolerance", obj._tolerance->c_str());

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__RelativeDistanceCondition(const RelativeDistanceCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj._freespace.get() && obj._freespace->length() > 0)
		elem->SetAttribute("freespace", obj._freespace->c_str());

	if(obj._relativeDistanceType.get() && obj._relativeDistanceType->length() > 0)
		elem->SetAttribute("relativeDistanceType", obj._relativeDistanceType->c_str());

	if(obj._rule.get() && obj._rule->length() > 0)
		elem->SetAttribute("rule", obj._rule->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__RelativeLanePosition(const RelativeLanePosition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._dLane.get() && obj._dLane->length() > 0)
		elem->SetAttribute("dLane", obj._dLane->c_str());

	if(obj._ds.get() && obj._ds->length() > 0)
		elem->SetAttribute("ds", obj._ds->c_str());

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj._offset.get() && obj._offset->length() > 0)
		elem->SetAttribute("offset", obj._offset->c_str());

	if(obj.sub_Orientation.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Orientation");
		__save__Orientation(*obj.sub_Orientation, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__RelativeObjectPosition(const RelativeObjectPosition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._dx.get() && obj._dx->length() > 0)
		elem->SetAttribute("dx", obj._dx->c_str());

	if(obj._dy.get() && obj._dy->length() > 0)
		elem->SetAttribute("dy", obj._dy->c_str());

	if(obj._dz.get() && obj._dz->length() > 0)
		elem->SetAttribute("dz", obj._dz->c_str());

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj.sub_Orientation.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Orientation");
		__save__Orientation(*obj.sub_Orientation, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__RelativeRoadPosition(const RelativeRoadPosition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._ds.get() && obj._ds->length() > 0)
		elem->SetAttribute("ds", obj._ds->c_str());

	if(obj._dt.get() && obj._dt->length() > 0)
		elem->SetAttribute("dt", obj._dt->c_str());

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj.sub_Orientation.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Orientation");
		__save__Orientation(*obj.sub_Orientation, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__RelativeSpeedCondition(const RelativeSpeedCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj._rule.get() && obj._rule->length() > 0)
		elem->SetAttribute("rule", obj._rule->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__RelativeSpeedToMaster(const RelativeSpeedToMaster &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._speedTargetValueType.get() && obj._speedTargetValueType->length() > 0)
		elem->SetAttribute("speedTargetValueType", obj._speedTargetValueType->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__RelativeTargetLane(const RelativeTargetLane &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__RelativeTargetLaneOffset(const RelativeTargetLaneOffset &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__RelativeTargetSpeed(const RelativeTargetSpeed &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._continuous.get() && obj._continuous->length() > 0)
		elem->SetAttribute("continuous", obj._continuous->c_str());

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj._speedTargetValueType.get() && obj._speedTargetValueType->length() > 0)
		elem->SetAttribute("speedTargetValueType", obj._speedTargetValueType->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__RelativeWorldPosition(const RelativeWorldPosition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._dx.get() && obj._dx->length() > 0)
		elem->SetAttribute("dx", obj._dx->c_str());

	if(obj._dy.get() && obj._dy->length() > 0)
		elem->SetAttribute("dy", obj._dy->c_str());

	if(obj._dz.get() && obj._dz->length() > 0)
		elem->SetAttribute("dz", obj._dz->c_str());

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj.sub_Orientation.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Orientation");
		__save__Orientation(*obj.sub_Orientation, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__RoadCondition(const RoadCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._frictionScaleFactor.get() && obj._frictionScaleFactor->length() > 0)
		elem->SetAttribute("frictionScaleFactor", obj._frictionScaleFactor->c_str());

	if(obj.sub_Properties.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Properties");
		__save__Properties(*obj.sub_Properties, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__RoadNetwork(const RoadNetwork &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_LogicFile.get()){
		tinyxml2::XMLElement* e = doc.NewElement("LogicFile");
		__save__File(*obj.sub_LogicFile, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_SceneGraphFile.get()){
		tinyxml2::XMLElement* e = doc.NewElement("SceneGraphFile");
		__save__File(*obj.sub_SceneGraphFile, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrafficSignals.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficSignals");
		__save__TrafficSignals(*obj.sub_TrafficSignals, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__RoadPosition(const RoadPosition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._roadId.get() && obj._roadId->length() > 0)
		elem->SetAttribute("roadId", obj._roadId->c_str());

	if(obj._s.get() && obj._s->length() > 0)
		elem->SetAttribute("s", obj._s->c_str());

	if(obj._t.get() && obj._t->length() > 0)
		elem->SetAttribute("t", obj._t->c_str());

	if(obj.sub_Orientation.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Orientation");
		__save__Orientation(*obj.sub_Orientation, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Route(const Route &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._closed.get() && obj._closed->length() > 0)
		elem->SetAttribute("closed", obj._closed->c_str());

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj.sub_ParameterDeclarations.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterDeclarations");
		__save__ParameterDeclarations(*obj.sub_ParameterDeclarations, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Waypoint){
		tinyxml2::XMLElement* e = doc.NewElement("Waypoint");
		__save__Waypoint(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__RouteCatalogLocation(const RouteCatalogLocation &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Directory.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Directory");
		__save__Directory(*obj.sub_Directory, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__RoutePosition(const RoutePosition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_InRoutePosition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("InRoutePosition");
		__save__InRoutePosition(*obj.sub_InRoutePosition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Orientation.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Orientation");
		__save__Orientation(*obj.sub_Orientation, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RouteRef.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RouteRef");
		__save__RouteRef(*obj.sub_RouteRef, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__RouteRef(const RouteRef &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_CatalogReference.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CatalogReference");
		__save__CatalogReference(*obj.sub_CatalogReference, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Route.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Route");
		__save__Route(*obj.sub_Route, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__RoutingAction(const RoutingAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_AcquirePositionAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("AcquirePositionAction");
		__save__AcquirePositionAction(*obj.sub_AcquirePositionAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_AssignRouteAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("AssignRouteAction");
		__save__AssignRouteAction(*obj.sub_AssignRouteAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_FollowTrajectoryAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("FollowTrajectoryAction");
		__save__FollowTrajectoryAction(*obj.sub_FollowTrajectoryAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__ScenarioObject(const ScenarioObject &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj.sub_CatalogReference.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CatalogReference");
		__save__CatalogReference(*obj.sub_CatalogReference, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_MiscObject.get()){
		tinyxml2::XMLElement* e = doc.NewElement("MiscObject");
		__save__MiscObject(*obj.sub_MiscObject, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ObjectController.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ObjectController");
		__save__ObjectController(*obj.sub_ObjectController, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Pedestrian.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Pedestrian");
		__save__Pedestrian(*obj.sub_Pedestrian, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Vehicle.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Vehicle");
		__save__Vehicle(*obj.sub_Vehicle, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__SelectedEntities(const SelectedEntities &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_ByType.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ByType");
		__save__ByType(*obj.sub_ByType, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_EntityRef.get()){
		tinyxml2::XMLElement* e = doc.NewElement("EntityRef");
		__save__EntityRef(*obj.sub_EntityRef, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Shape(const Shape &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Clothoid.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Clothoid");
		__save__Clothoid(*obj.sub_Clothoid, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Nurbs.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Nurbs");
		__save__Nurbs(*obj.sub_Nurbs, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Polyline.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Polyline");
		__save__Polyline(*obj.sub_Polyline, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__SimulationTimeCondition(const SimulationTimeCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._rule.get() && obj._rule->length() > 0)
		elem->SetAttribute("rule", obj._rule->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__SpeedAction(const SpeedAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_SpeedActionDynamics.get()){
		tinyxml2::XMLElement* e = doc.NewElement("SpeedActionDynamics");
		__save__TransitionDynamics(*obj.sub_SpeedActionDynamics, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_SpeedActionTarget.get()){
		tinyxml2::XMLElement* e = doc.NewElement("SpeedActionTarget");
		__save__SpeedActionTarget(*obj.sub_SpeedActionTarget, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__SpeedActionTarget(const SpeedActionTarget &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_AbsoluteTargetSpeed.get()){
		tinyxml2::XMLElement* e = doc.NewElement("AbsoluteTargetSpeed");
		__save__AbsoluteTargetSpeed(*obj.sub_AbsoluteTargetSpeed, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_RelativeTargetSpeed.get()){
		tinyxml2::XMLElement* e = doc.NewElement("RelativeTargetSpeed");
		__save__RelativeTargetSpeed(*obj.sub_RelativeTargetSpeed, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__SpeedCondition(const SpeedCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._rule.get() && obj._rule->length() > 0)
		elem->SetAttribute("rule", obj._rule->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__StandStillCondition(const StandStillCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._duration.get() && obj._duration->length() > 0)
		elem->SetAttribute("duration", obj._duration->c_str());



	return true;

}

bool __save__Story(const Story &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	for(auto &ob : obj.sub_Act){
		tinyxml2::XMLElement* e = doc.NewElement("Act");
		__save__Act(ob, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ParameterDeclarations.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterDeclarations");
		__save__ParameterDeclarations(*obj.sub_ParameterDeclarations, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Storyboard(const Storyboard &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Init.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Init");
		__save__Init(*obj.sub_Init, e, doc);
		elem->InsertEndChild(e);
	}

	for(auto &ob : obj.sub_Story){
		tinyxml2::XMLElement* e = doc.NewElement("Story");
		__save__Story(ob, e, doc);
		elem->InsertEndChild(e);
	}
	
	if (obj.sub_StopTrigger.get()) {
      tinyxml2::XMLElement *e = doc.NewElement("StopTrigger");
      __save__Trigger(*obj.sub_StopTrigger, e, doc);
      elem->InsertEndChild(e);
    }

	return true;

}

bool __save__StoryboardElementStateCondition(const StoryboardElementStateCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._state.get() && obj._state->length() > 0)
		elem->SetAttribute("state", obj._state->c_str());

	if(obj._storyboardElementRef.get() && obj._storyboardElementRef->length() > 0)
		elem->SetAttribute("storyboardElementRef", obj._storyboardElementRef->c_str());

	if(obj._storyboardElementType.get() && obj._storyboardElementType->length() > 0)
		elem->SetAttribute("storyboardElementType", obj._storyboardElementType->c_str());



	return true;

}

bool __save__Sun(const Sun &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._azimuth.get() && obj._azimuth->length() > 0)
		elem->SetAttribute("azimuth", obj._azimuth->c_str());

	if(obj._elevation.get() && obj._elevation->length() > 0)
		elem->SetAttribute("elevation", obj._elevation->c_str());

	if(obj._intensity.get() && obj._intensity->length() > 0)
		elem->SetAttribute("intensity", obj._intensity->c_str());



	return true;

}

bool __save__SynchronizeAction(const SynchronizeAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._masterEntityRef.get() && obj._masterEntityRef->length() > 0)
		elem->SetAttribute("masterEntityRef", obj._masterEntityRef->c_str());

	if(obj.sub_FinalSpeed.get()){
		tinyxml2::XMLElement* e = doc.NewElement("FinalSpeed");
		__save__FinalSpeed(*obj.sub_FinalSpeed, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TargetPosition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TargetPosition");
		__save__Position(*obj.sub_TargetPosition, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TargetPositionMaster.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TargetPositionMaster");
		__save__Position(*obj.sub_TargetPositionMaster, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TeleportAction(const TeleportAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TimeHeadwayCondition(const TimeHeadwayCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._alongRoute.get() && obj._alongRoute->length() > 0)
		elem->SetAttribute("alongRoute", obj._alongRoute->c_str());

	if(obj._entityRef.get() && obj._entityRef->length() > 0)
		elem->SetAttribute("entityRef", obj._entityRef->c_str());

	if(obj._freespace.get() && obj._freespace->length() > 0)
		elem->SetAttribute("freespace", obj._freespace->c_str());

	if(obj._rule.get() && obj._rule->length() > 0)
		elem->SetAttribute("rule", obj._rule->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__TimeOfDay(const TimeOfDay &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._animation.get() && obj._animation->length() > 0)
		elem->SetAttribute("animation", obj._animation->c_str());

	if(obj._dateTime.get() && obj._dateTime->length() > 0)
		elem->SetAttribute("dateTime", obj._dateTime->c_str());



	return true;

}

bool __save__TimeOfDayCondition(const TimeOfDayCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._dateTime.get() && obj._dateTime->length() > 0)
		elem->SetAttribute("dateTime", obj._dateTime->c_str());

	if(obj._rule.get() && obj._rule->length() > 0)
		elem->SetAttribute("rule", obj._rule->c_str());



	return true;

}

bool __save__TimeReference(const TimeReference &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_None.get()){
		tinyxml2::XMLElement* e = doc.NewElement("None");
		__save__None(*obj.sub_None, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Timing.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Timing");
		__save__Timing(*obj.sub_Timing, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TimeToCollisionCondition(const TimeToCollisionCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._alongRoute.get() && obj._alongRoute->length() > 0)
		elem->SetAttribute("alongRoute", obj._alongRoute->c_str());

	if(obj._freespace.get() && obj._freespace->length() > 0)
		elem->SetAttribute("freespace", obj._freespace->c_str());

	if(obj._rule.get() && obj._rule->length() > 0)
		elem->SetAttribute("rule", obj._rule->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());

	if(obj.sub_TimeToCollisionConditionTarget.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TimeToCollisionConditionTarget");
		__save__TimeToCollisionConditionTarget(*obj.sub_TimeToCollisionConditionTarget, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TimeToCollisionConditionTarget(const TimeToCollisionConditionTarget &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_EntityRef.get()){
		tinyxml2::XMLElement* e = doc.NewElement("EntityRef");
		__save__EntityRef(*obj.sub_EntityRef, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Timing(const Timing &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._domainAbsoluteRelative.get() && obj._domainAbsoluteRelative->length() > 0)
		elem->SetAttribute("domainAbsoluteRelative", obj._domainAbsoluteRelative->c_str());

	if(obj._offset.get() && obj._offset->length() > 0)
		elem->SetAttribute("offset", obj._offset->c_str());

	if(obj._scale.get() && obj._scale->length() > 0)
		elem->SetAttribute("scale", obj._scale->c_str());



	return true;

}

bool __save__TrafficAction(const TrafficAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_TrafficSinkAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficSinkAction");
		__save__TrafficSinkAction(*obj.sub_TrafficSinkAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrafficSourceAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficSourceAction");
		__save__TrafficSourceAction(*obj.sub_TrafficSourceAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrafficSwarmAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficSwarmAction");
		__save__TrafficSwarmAction(*obj.sub_TrafficSwarmAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TrafficDefinition(const TrafficDefinition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj.sub_ControllerDistribution.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ControllerDistribution");
		__save__ControllerDistribution(*obj.sub_ControllerDistribution, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_VehicleCategoryDistribution.get()){
		tinyxml2::XMLElement* e = doc.NewElement("VehicleCategoryDistribution");
		__save__VehicleCategoryDistribution(*obj.sub_VehicleCategoryDistribution, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TrafficSignalAction(const TrafficSignalAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_TrafficSignalControllerAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficSignalControllerAction");
		__save__TrafficSignalControllerAction(*obj.sub_TrafficSignalControllerAction, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrafficSignalStateAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficSignalStateAction");
		__save__TrafficSignalStateAction(*obj.sub_TrafficSignalStateAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TrafficSignalCondition(const TrafficSignalCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj._state.get() && obj._state->length() > 0)
		elem->SetAttribute("state", obj._state->c_str());



	return true;

}

bool __save__TrafficSignalController(const TrafficSignalController &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._delay.get() && obj._delay->length() > 0)
		elem->SetAttribute("delay", obj._delay->c_str());

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj._reference.get() && obj._reference->length() > 0)
		elem->SetAttribute("reference", obj._reference->c_str());

	for(auto &ob : obj.sub_Phase){
		tinyxml2::XMLElement* e = doc.NewElement("Phase");
		__save__Phase(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TrafficSignals(const TrafficSignals &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_TrafficSignalController){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficSignalController");
		__save__TrafficSignalController(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TrafficSignalControllerAction(const TrafficSignalControllerAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._phase.get() && obj._phase->length() > 0)
		elem->SetAttribute("phase", obj._phase->c_str());

	if(obj._trafficSignalControllerRef.get() && obj._trafficSignalControllerRef->length() > 0)
		elem->SetAttribute("trafficSignalControllerRef", obj._trafficSignalControllerRef->c_str());



	return true;

}

bool __save__TrafficSignalControllerCondition(const TrafficSignalControllerCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._phase.get() && obj._phase->length() > 0)
		elem->SetAttribute("phase", obj._phase->c_str());

	if(obj._trafficSignalControllerRef.get() && obj._trafficSignalControllerRef->length() > 0)
		elem->SetAttribute("trafficSignalControllerRef", obj._trafficSignalControllerRef->c_str());



	return true;

}

bool __save__TrafficSignalState(const TrafficSignalState &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._state.get() && obj._state->length() > 0)
		elem->SetAttribute("state", obj._state->c_str());

	if(obj._trafficSignalId.get() && obj._trafficSignalId->length() > 0)
		elem->SetAttribute("trafficSignalId", obj._trafficSignalId->c_str());
	else
        elem->SetAttribute("trafficSignalId", "");



	return true;

}

bool __save__TrafficSignalStateAction(const TrafficSignalStateAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj._state.get() && obj._state->length() > 0)
		elem->SetAttribute("state", obj._state->c_str());



	return true;

}

bool __save__TrafficSinkAction(const TrafficSinkAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._radius.get() && obj._radius->length() > 0)
		elem->SetAttribute("radius", obj._radius->c_str());

	if(obj._rate.get() && obj._rate->length() > 0)
		elem->SetAttribute("rate", obj._rate->c_str());

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrafficDefinition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficDefinition");
		__save__TrafficDefinition(*obj.sub_TrafficDefinition, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TrafficSourceAction(const TrafficSourceAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._radius.get() && obj._radius->length() > 0)
		elem->SetAttribute("radius", obj._radius->c_str());

	if(obj._rate.get() && obj._rate->length() > 0)
		elem->SetAttribute("rate", obj._rate->c_str());

	if(obj._velocity.get() && obj._velocity->length() > 0)
		elem->SetAttribute("velocity", obj._velocity->c_str());

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrafficDefinition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficDefinition");
		__save__TrafficDefinition(*obj.sub_TrafficDefinition, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TrafficSwarmAction(const TrafficSwarmAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._innerRadius.get() && obj._innerRadius->length() > 0)
		elem->SetAttribute("innerRadius", obj._innerRadius->c_str());

	if(obj._numberOfVehicles.get() && obj._numberOfVehicles->length() > 0)
		elem->SetAttribute("numberOfVehicles", obj._numberOfVehicles->c_str());

	if(obj._offset.get() && obj._offset->length() > 0)
		elem->SetAttribute("offset", obj._offset->c_str());

	if(obj._semiMajorAxis.get() && obj._semiMajorAxis->length() > 0)
		elem->SetAttribute("semiMajorAxis", obj._semiMajorAxis->c_str());

	if(obj._semiMinorAxis.get() && obj._semiMinorAxis->length() > 0)
		elem->SetAttribute("semiMinorAxis", obj._semiMinorAxis->c_str());

	if(obj._velocity.get() && obj._velocity->length() > 0)
		elem->SetAttribute("velocity", obj._velocity->c_str());

	if(obj.sub_CentralObject.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CentralObject");
		__save__CentralSwarmObject(*obj.sub_CentralObject, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_TrafficDefinition.get()){
		tinyxml2::XMLElement* e = doc.NewElement("TrafficDefinition");
		__save__TrafficDefinition(*obj.sub_TrafficDefinition, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Trajectory(const Trajectory &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._closed.get() && obj._closed->length() > 0)
		elem->SetAttribute("closed", obj._closed->c_str());

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj.sub_ParameterDeclarations.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterDeclarations");
		__save__ParameterDeclarations(*obj.sub_ParameterDeclarations, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Shape.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Shape");
		__save__Shape(*obj.sub_Shape, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TrajectoryCatalogLocation(const TrajectoryCatalogLocation &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Directory.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Directory");
		__save__Directory(*obj.sub_Directory, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TrajectoryFollowingMode(const TrajectoryFollowingMode &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._followingMode.get() && obj._followingMode->length() > 0)
		elem->SetAttribute("followingMode", obj._followingMode->c_str());



	return true;

}

bool __save__TransitionDynamics(const TransitionDynamics &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._dynamicsDimension.get() && obj._dynamicsDimension->length() > 0)
		elem->SetAttribute("dynamicsDimension", obj._dynamicsDimension->c_str());

	if(obj._dynamicsShape.get() && obj._dynamicsShape->length() > 0)
		elem->SetAttribute("dynamicsShape", obj._dynamicsShape->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__TraveledDistanceCondition(const TraveledDistanceCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__Trigger(const Trigger &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_ConditionGroup){
		tinyxml2::XMLElement* e = doc.NewElement("ConditionGroup");
		__save__ConditionGroup(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__TriggeringEntities(const TriggeringEntities &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._triggeringEntitiesRule.get() && obj._triggeringEntitiesRule->length() > 0)
		elem->SetAttribute("triggeringEntitiesRule", obj._triggeringEntitiesRule->c_str());

	for(auto &ob : obj.sub_EntityRef){
		tinyxml2::XMLElement* e = doc.NewElement("EntityRef");
		__save__EntityRef(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__UserDefinedAction(const UserDefinedAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_CustomCommandAction.get()){
		tinyxml2::XMLElement* e = doc.NewElement("CustomCommandAction");
		__save__CustomCommandAction(*obj.sub_CustomCommandAction, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__UserDefinedValueCondition(const UserDefinedValueCondition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj._rule.get() && obj._rule->length() > 0)
		elem->SetAttribute("rule", obj._rule->c_str());

	if(obj._value.get() && obj._value->length() > 0)
		elem->SetAttribute("value", obj._value->c_str());



	return true;

}

bool __save__Vehicle(const Vehicle &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._name.get() && obj._name->length() > 0)
		elem->SetAttribute("name", obj._name->c_str());

	if(obj._vehicleCategory.get() && obj._vehicleCategory->length() > 0)
		elem->SetAttribute("vehicleCategory", obj._vehicleCategory->c_str());

	if(obj.sub_Axles.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Axles");
		__save__Axles(*obj.sub_Axles, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_BoundingBox.get()){
		tinyxml2::XMLElement* e = doc.NewElement("BoundingBox");
		__save__BoundingBox(*obj.sub_BoundingBox, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_ParameterDeclarations.get()){
		tinyxml2::XMLElement* e = doc.NewElement("ParameterDeclarations");
		__save__ParameterDeclarations(*obj.sub_ParameterDeclarations, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Performance.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Performance");
		__save__Performance(*obj.sub_Performance, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Properties.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Properties");
		__save__Properties(*obj.sub_Properties, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__VehicleCatalogLocation(const VehicleCatalogLocation &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj.sub_Directory.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Directory");
		__save__Directory(*obj.sub_Directory, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__VehicleCategoryDistribution(const VehicleCategoryDistribution &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	for(auto &ob : obj.sub_VehicleCategoryDistributionEntry){
		tinyxml2::XMLElement* e = doc.NewElement("VehicleCategoryDistributionEntry");
		__save__VehicleCategoryDistributionEntry(ob, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__VehicleCategoryDistributionEntry(const VehicleCategoryDistributionEntry &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._category.get() && obj._category->length() > 0)
		elem->SetAttribute("category", obj._category->c_str());

	if(obj._weight.get() && obj._weight->length() > 0)
		elem->SetAttribute("weight", obj._weight->c_str());



	return true;

}

bool __save__Vertex(const Vertex &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._time.get() && obj._time->length() > 0)
		elem->SetAttribute("time", obj._time->c_str());

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__VisibilityAction(const VisibilityAction &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._graphics.get() && obj._graphics->length() > 0)
		elem->SetAttribute("graphics", obj._graphics->c_str());

	if(obj._sensors.get() && obj._sensors->length() > 0)
		elem->SetAttribute("sensors", obj._sensors->c_str());

	if(obj._traffic.get() && obj._traffic->length() > 0)
		elem->SetAttribute("traffic", obj._traffic->c_str());



	return true;

}

bool __save__Waypoint(const Waypoint &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._routeStrategy.get() && obj._routeStrategy->length() > 0)
		elem->SetAttribute("routeStrategy", obj._routeStrategy->c_str());

	if(obj.sub_Position.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Position");
		__save__Position(*obj.sub_Position, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__Weather(const Weather &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._cloudState.get() && obj._cloudState->length() > 0)
		elem->SetAttribute("cloudState", obj._cloudState->c_str());

	if(obj.sub_Fog.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Fog");
		__save__Fog(*obj.sub_Fog, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Precipitation.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Precipitation");
		__save__Precipitation(*obj.sub_Precipitation, e, doc);
		elem->InsertEndChild(e);
	}

	if(obj.sub_Sun.get()){
		tinyxml2::XMLElement* e = doc.NewElement("Sun");
		__save__Sun(*obj.sub_Sun, e, doc);
		elem->InsertEndChild(e);
	}



	return true;

}

bool __save__WorldPosition(const WorldPosition &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {

	if(obj._h.get() && obj._h->length() > 0)
		elem->SetAttribute("h", obj._h->c_str());

	if(obj._p.get() && obj._p->length() > 0)
		elem->SetAttribute("p", obj._p->c_str());

	if(obj._r.get() && obj._r->length() > 0)
		elem->SetAttribute("r", obj._r->c_str());

	if(obj._x.get() && obj._x->length() > 0)
		elem->SetAttribute("x", obj._x->c_str());

	if(obj._y.get() && obj._y->length() > 0)
		elem->SetAttribute("y", obj._y->c_str());

	if(obj._z.get() && obj._z->length() > 0)
		elem->SetAttribute("z", obj._z->c_str());



	return true;

}


} // namespace osc
