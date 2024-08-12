

#ifndef OSC_STRUCTURE_H
#define OSC_STRUCTURE_H

#include <string>
#include <memory>
#include <vector>

namespace osc {

	namespace xsd {
	template<typename T>
	struct Attribute : public std::shared_ptr<T> {
		Attribute() = default;
		virtual ~Attribute() = default;
		Attribute<T> &operator=(const T &v) {
			this->reset(new T(v));
			return *this;
		}

		Attribute<T> &create() {
			this->reset(new T);
			return *this;
		}
	};

	typedef Attribute<double> d_double;
	typedef Attribute<int> d_int;
	typedef Attribute<unsigned int> d_uint;
	typedef Attribute<std::string> d_string;
	typedef Attribute<float> d_float;
	template<typename T>
	using Vector = std::vector<T>;
	} // namespace xsd

	typedef xsd::d_string parameter;
	typedef xsd::d_string Boolean;
	typedef xsd::d_string DateTime;
	typedef xsd::d_string Double;
	typedef xsd::d_string Int;
	typedef xsd::d_string String;
	typedef xsd::d_string UnsignedInt;
	typedef xsd::d_string UnsignedShort;
	typedef xsd::d_string CloudState;
	typedef xsd::d_string ConditionEdge;
	typedef xsd::d_string DynamicsDimension;
	typedef xsd::d_string DynamicsShape;
	typedef xsd::d_string FollowingMode;
	typedef xsd::d_string MiscObjectCategory;
	typedef xsd::d_string ObjectType;
	typedef xsd::d_string ParameterType;
	typedef xsd::d_string PedestrianCategory;
	typedef xsd::d_string PrecipitationType;
	typedef xsd::d_string Priority;
	typedef xsd::d_string ReferenceContext;
	typedef xsd::d_string RelativeDistanceType;
	typedef xsd::d_string RouteStrategy;
	typedef xsd::d_string Rule;
	typedef xsd::d_string SpeedTargetValueType;
	typedef xsd::d_string StoryboardElementState;
	typedef xsd::d_string StoryboardElementType;
	typedef xsd::d_string TriggeringEntitiesRule;
	typedef xsd::d_string VehicleCategory;
	struct AbsoluteSpeed;
	struct AbsoluteTargetLane;
	struct AbsoluteTargetLaneOffset;
	struct AbsoluteTargetSpeed;
	struct AccelerationCondition;
	struct AcquirePositionAction;
	struct Act;
	struct Action;
	struct ActivateControllerAction;
	struct Actors;
	struct AddEntityAction;
	struct AssignControllerAction;
	struct AssignRouteAction;
	struct Axle;
	struct Axles;
	struct BoundingBox;
	struct ByEntityCondition;
	struct ByObjectType;
	struct ByType;
	struct ByValueCondition;
	struct Catalog;
	struct CatalogLocations;
	struct CatalogReference;
	struct Center;
	struct CentralSwarmObject;
	struct Clothoid;
	struct CollisionCondition;
	struct Condition;
	struct ConditionGroup;
	struct Controller;
	struct ControllerAction;
	struct ControllerCatalogLocation;
	struct ControllerDistribution;
	struct ControllerDistributionEntry;
	struct ControlPoint;
	struct CustomCommandAction;
	struct DeleteEntityAction;
	struct Dimensions;
	struct Directory;
	struct DistanceCondition;
	struct DynamicConstraints;
	struct EndOfRoadCondition;
	struct Entities;
	struct EntityAction;
	struct EntityCondition;
	struct EntityRef;
	struct EntitySelection;
	struct Environment;
	struct EnvironmentAction;
	struct EnvironmentCatalogLocation;
	struct Event;
	struct File;
	struct FileHeader;
	struct FinalSpeed;
	struct Fog;
	struct FollowTrajectoryAction;
	struct GlobalAction;
	struct InfrastructureAction;
	struct Init;
	struct InitActions;
	struct InRoutePosition;
	struct Knot;
	struct LaneChangeAction;
	struct LaneChangeTarget;
	struct LaneOffsetAction;
	struct LaneOffsetActionDynamics;
	struct LaneOffsetTarget;
	struct LanePosition;
	struct LateralAction;
	struct LateralDistanceAction;
	struct LongitudinalAction;
	struct LongitudinalDistanceAction;
	struct Maneuver;
	struct ManeuverCatalogLocation;
	struct ManeuverGroup;
	struct MiscObject;
	struct MiscObjectCatalogLocation;
	struct ModifyRule;
	struct None;
	struct Nurbs;
	struct ObjectController;
	struct OffroadCondition;
	struct OpenScenario;
	struct Orientation;
	struct OverrideBrakeAction;
	struct OverrideClutchAction;
	struct OverrideControllerValueAction;
	struct OverrideGearAction;
	struct OverrideParkingBrakeAction;
	struct OverrideSteeringWheelAction;
	struct OverrideThrottleAction;
	struct ParameterAction;
	struct ParameterAddValueRule;
	struct ParameterAssignment;
	struct ParameterAssignments;
	struct ParameterCondition;
	struct ParameterDeclaration;
	struct ParameterDeclarations;
	struct ParameterModifyAction;
	struct ParameterMultiplyByValueRule;
	struct ParameterSetAction;
	struct Pedestrian;
	struct PedestrianCatalogLocation;
	struct Performance;
	struct Phase;
	struct Polyline;
	struct Position;
	struct PositionInLaneCoordinates;
	struct PositionInRoadCoordinates;
	struct PositionOfCurrentEntity;
	struct Precipitation;
	struct Private;
	struct PrivateAction;
	struct Properties;
	struct Property;
	struct ReachPositionCondition;
	struct RelativeDistanceCondition;
	struct RelativeLanePosition;
	struct RelativeObjectPosition;
	struct RelativeRoadPosition;
	struct RelativeSpeedCondition;
	struct RelativeSpeedToMaster;
	struct RelativeTargetLane;
	struct RelativeTargetLaneOffset;
	struct RelativeTargetSpeed;
	struct RelativeWorldPosition;
	struct RoadCondition;
	struct RoadNetwork;
	struct RoadPosition;
	struct Route;
	struct RouteCatalogLocation;
	struct RoutePosition;
	struct RouteRef;
	struct RoutingAction;
	struct ScenarioObject;
	struct SelectedEntities;
	struct Shape;
	struct SimulationTimeCondition;
	struct SpeedAction;
	struct SpeedActionTarget;
	struct SpeedCondition;
	struct StandStillCondition;
	struct Story;
	struct Storyboard;
	struct StoryboardElementStateCondition;
	struct Sun;
	struct SynchronizeAction;
	struct TeleportAction;
	struct TimeHeadwayCondition;
	struct TimeOfDay;
	struct TimeOfDayCondition;
	struct TimeReference;
	struct TimeToCollisionCondition;
	struct TimeToCollisionConditionTarget;
	struct Timing;
	struct TrafficAction;
	struct TrafficDefinition;
	struct TrafficSignalAction;
	struct TrafficSignalCondition;
	struct TrafficSignalController;
	struct TrafficSignals;
	struct TrafficSignalControllerAction;
	struct TrafficSignalControllerCondition;
	struct TrafficSignalState;
	struct TrafficSignalStateAction;
	struct TrafficSinkAction;
	struct TrafficSourceAction;
	struct TrafficSwarmAction;
	struct Trajectory;
	struct TrajectoryCatalogLocation;
	struct TrajectoryFollowingMode;
	struct TransitionDynamics;
	struct TraveledDistanceCondition;
	struct Trigger;
	struct TriggeringEntities;
	struct UserDefinedAction;
	struct UserDefinedValueCondition;
	struct Vehicle;
	struct VehicleCatalogLocation;
	struct VehicleCategoryDistribution;
	struct VehicleCategoryDistributionEntry;
	struct Vertex;
	struct VisibilityAction;
	struct Waypoint;
	struct Weather;
	struct WorldPosition;


	struct AbsoluteSpeed {
		Double _value;
	};

	struct AbsoluteTargetLane {
		String _value;
	};

	struct AbsoluteTargetLaneOffset {
		Double _value;
	};

	struct AbsoluteTargetSpeed {
		Double _value;
	};

	struct AccelerationCondition {
		Rule _rule;
		Double _value;
	};

	struct AcquirePositionAction {
		xsd::Attribute<Position> sub_Position;
	};

	struct Act {
		xsd::Vector<ManeuverGroup> sub_ManeuverGroup{};
		xsd::Attribute<Trigger> sub_StartTrigger;
		xsd::Attribute<Trigger> sub_StopTrigger;
		String _name;
	};

	struct Action {
		xsd::Attribute<GlobalAction> sub_GlobalAction;
		xsd::Attribute<PrivateAction> sub_PrivateAction;
		xsd::Attribute<UserDefinedAction> sub_UserDefinedAction;
		String _name;
	};

	struct ActivateControllerAction {
		Boolean _lateral;
		Boolean _longitudinal;
	};

	struct Actors {
		xsd::Vector<EntityRef> sub_EntityRef{};
		Boolean _selectTriggeringEntities;
	};

	struct AddEntityAction {
		xsd::Attribute<Position> sub_Position;
	};

	struct AssignControllerAction {
		xsd::Attribute<CatalogReference> sub_CatalogReference;
		xsd::Attribute<Controller> sub_Controller;
	};

	struct AssignRouteAction {
		xsd::Attribute<CatalogReference> sub_CatalogReference;
		xsd::Attribute<Route> sub_Route;
	};

	struct Axle {
		Double _maxSteering;
		Double _positionX;
		Double _positionZ;
		Double _trackWidth;
		Double _wheelDiameter;
	};

	struct Axles {
		xsd::Vector<Axle> sub_AdditionalAxle{};
		xsd::Attribute<Axle> sub_FrontAxle;
		xsd::Attribute<Axle> sub_RearAxle;
	};

	struct BoundingBox {
		xsd::Attribute<Center> sub_Center;
		xsd::Attribute<Dimensions> sub_Dimensions;
	};

	struct ByEntityCondition {
		xsd::Attribute<EntityCondition> sub_EntityCondition;
		xsd::Attribute<TriggeringEntities> sub_TriggeringEntities;
	};

	struct ByObjectType {
		ObjectType _type;
	};

	struct ByType {
		ObjectType _objectType;
	};

	struct ByValueCondition {
		xsd::Attribute<ParameterCondition> sub_ParameterCondition;
		xsd::Attribute<SimulationTimeCondition> sub_SimulationTimeCondition;
		xsd::Attribute<StoryboardElementStateCondition> sub_StoryboardElementStateCondition;
		xsd::Attribute<TimeOfDayCondition> sub_TimeOfDayCondition;
		xsd::Attribute<TrafficSignalCondition> sub_TrafficSignalCondition;
		xsd::Attribute<TrafficSignalControllerCondition> sub_TrafficSignalControllerCondition;
		xsd::Attribute<UserDefinedValueCondition> sub_UserDefinedValueCondition;
	};

	struct Catalog {
		xsd::Vector<Controller> sub_Controller{};
		xsd::Vector<Environment> sub_Environment{};
		xsd::Vector<Maneuver> sub_Maneuver{};
		xsd::Vector<MiscObject> sub_MiscObject{};
		xsd::Vector<Pedestrian> sub_Pedestrian{};
		xsd::Vector<Route> sub_Route{};
		xsd::Vector<Trajectory> sub_Trajectory{};
		xsd::Vector<Vehicle> sub_Vehicle{};
		String _name;
	};

	struct CatalogLocations {
		xsd::Attribute<ControllerCatalogLocation> sub_ControllerCatalog;
		xsd::Attribute<EnvironmentCatalogLocation> sub_EnvironmentCatalog;
		xsd::Attribute<ManeuverCatalogLocation> sub_ManeuverCatalog;
		xsd::Attribute<MiscObjectCatalogLocation> sub_MiscObjectCatalog;
		xsd::Attribute<PedestrianCatalogLocation> sub_PedestrianCatalog;
		xsd::Attribute<RouteCatalogLocation> sub_RouteCatalog;
		xsd::Attribute<TrajectoryCatalogLocation> sub_TrajectoryCatalog;
		xsd::Attribute<VehicleCatalogLocation> sub_VehicleCatalog;
	};

	struct CatalogReference {
		xsd::Attribute<ParameterAssignments> sub_ParameterAssignments;
		String _catalogName;
		String _entryName;
	};

	struct Center {
		Double _x;
		Double _y;
		Double _z;
	};

	struct CentralSwarmObject {
		String _entityRef;
	};

	struct Clothoid {
		xsd::Attribute<Position> sub_Position;
		Double _curvature;
		Double _curvatureDot;
		Double _length;
		Double _startTime;
		Double _stopTime;
	};

	struct CollisionCondition {
		xsd::Attribute<ByObjectType> sub_ByType;
		xsd::Attribute<EntityRef> sub_EntityRef;
	};

	struct Condition {
		xsd::Attribute<ByEntityCondition> sub_ByEntityCondition;
		xsd::Attribute<ByValueCondition> sub_ByValueCondition;
		ConditionEdge _conditionEdge;
		Double _delay;
		String _name;
	};

	struct ConditionGroup {
		xsd::Vector<Condition> sub_Condition{};
	};

	struct Controller {
		xsd::Attribute<ParameterDeclarations> sub_ParameterDeclarations;
		xsd::Attribute<Properties> sub_Properties;
		String _name;
	};

	struct ControllerAction {
		xsd::Attribute<AssignControllerAction> sub_AssignControllerAction;
		xsd::Attribute<OverrideControllerValueAction> sub_OverrideControllerValueAction;
	};

	struct ControllerCatalogLocation {
		xsd::Attribute<Directory> sub_Directory;
	};

	struct ControllerDistribution {
		xsd::Vector<ControllerDistributionEntry> sub_ControllerDistributionEntry{};
	};

	struct ControllerDistributionEntry {
		xsd::Attribute<CatalogReference> sub_CatalogReference;
		xsd::Attribute<Controller> sub_Controller;
		Double _weight;
	};

	struct ControlPoint {
		xsd::Attribute<Position> sub_Position;
		Double _time;
		Double _weight;
	};

	struct CustomCommandAction {
          String _type;
          String _text;
	};

	struct DeleteEntityAction {
	};

	struct Dimensions {
		Double _height;
		Double _length;
		Double _width;
	};

	struct Directory {
		String _path;
	};

	struct DistanceCondition {
		xsd::Attribute<Position> sub_Position;
		Boolean _alongRoute;
		Boolean _freespace;
		Rule _rule;
		Double _value;
	};

	struct DynamicConstraints {
		Double _maxAcceleration;
		Double _maxDeceleration;
		Double _maxSpeed;
	};

	struct EndOfRoadCondition {
		Double _duration;
	};

	struct Entities {
		xsd::Vector<EntitySelection> sub_EntitySelection{};
		xsd::Vector<ScenarioObject> sub_ScenarioObject{};
	};

	struct EntityAction {
		xsd::Attribute<AddEntityAction> sub_AddEntityAction;
		xsd::Attribute<DeleteEntityAction> sub_DeleteEntityAction;
		String _entityRef;
	};

	struct EntityCondition {
		xsd::Attribute<AccelerationCondition> sub_AccelerationCondition;
		xsd::Attribute<CollisionCondition> sub_CollisionCondition;
		xsd::Attribute<DistanceCondition> sub_DistanceCondition;
		xsd::Attribute<EndOfRoadCondition> sub_EndOfRoadCondition;
		xsd::Attribute<OffroadCondition> sub_OffroadCondition;
		xsd::Attribute<ReachPositionCondition> sub_ReachPositionCondition;
		xsd::Attribute<RelativeDistanceCondition> sub_RelativeDistanceCondition;
		xsd::Attribute<RelativeSpeedCondition> sub_RelativeSpeedCondition;
		xsd::Attribute<SpeedCondition> sub_SpeedCondition;
		xsd::Attribute<StandStillCondition> sub_StandStillCondition;
		xsd::Attribute<TimeHeadwayCondition> sub_TimeHeadwayCondition;
		xsd::Attribute<TimeToCollisionCondition> sub_TimeToCollisionCondition;
		xsd::Attribute<TraveledDistanceCondition> sub_TraveledDistanceCondition;
	};

	struct EntityRef {
		String _entityRef;
	};

	struct EntitySelection {
		xsd::Attribute<SelectedEntities> sub_Members;
		String _name;
	};

	struct Environment {
		xsd::Attribute<ParameterDeclarations> sub_ParameterDeclarations;
		xsd::Attribute<RoadCondition> sub_RoadCondition;
		xsd::Attribute<TimeOfDay> sub_TimeOfDay;
		xsd::Attribute<Weather> sub_Weather;
		String _name;
	};

	struct EnvironmentAction {
		xsd::Attribute<CatalogReference> sub_CatalogReference;
		xsd::Attribute<Environment> sub_Environment;
	};

	struct EnvironmentCatalogLocation {
		xsd::Attribute<Directory> sub_Directory;
	};

	struct Event {
		xsd::Vector<Action> sub_Action{};
		xsd::Attribute<Trigger> sub_StartTrigger;
		UnsignedInt _maximumExecutionCount;
		String _name;
		Priority _priority;
	};

	struct File {
		String _filepath;
	};

	struct FileHeader {
		String _author;
		DateTime _date;
		String _description;
		UnsignedShort _revMajor;
		UnsignedShort _revMinor;
	};

	struct FinalSpeed {
		xsd::Attribute<AbsoluteSpeed> sub_AbsoluteSpeed;
		xsd::Attribute<RelativeSpeedToMaster> sub_RelativeSpeedToMaster;
	};

	struct Fog {
		xsd::Attribute<BoundingBox> sub_BoundingBox;
		Double _visualRange;
	};

	struct FollowTrajectoryAction {
		xsd::Attribute<CatalogReference> sub_CatalogReference;
		xsd::Attribute<TimeReference> sub_TimeReference;
		xsd::Attribute<Trajectory> sub_Trajectory;
		xsd::Attribute<TrajectoryFollowingMode> sub_TrajectoryFollowingMode;
	};

	struct GlobalAction {
		xsd::Attribute<EntityAction> sub_EntityAction;
		xsd::Attribute<EnvironmentAction> sub_EnvironmentAction;
		xsd::Attribute<InfrastructureAction> sub_InfrastructureAction;
		xsd::Attribute<ParameterAction> sub_ParameterAction;
		xsd::Attribute<TrafficAction> sub_TrafficAction;
	};

	struct InfrastructureAction {
		xsd::Attribute<TrafficSignalAction> sub_TrafficSignalAction;
	};

	struct Init {
		xsd::Attribute<InitActions> sub_Actions;
	};

	struct InitActions {
		xsd::Vector<GlobalAction> sub_GlobalAction{};
		xsd::Vector<Private> sub_Private{};
		xsd::Vector<UserDefinedAction> sub_UserDefinedAction{};
	};

	struct InRoutePosition {
		xsd::Attribute<PositionOfCurrentEntity> sub_FromCurrentEntity;
		xsd::Attribute<PositionInLaneCoordinates> sub_FromLaneCoordinates;
		xsd::Attribute<PositionInRoadCoordinates> sub_FromRoadCoordinates;
	};

	struct Knot {
		Double _value;
	};

	struct LaneChangeAction {
		xsd::Attribute<TransitionDynamics> sub_LaneChangeActionDynamics;
		xsd::Attribute<LaneChangeTarget> sub_LaneChangeTarget;
		Double _targetLaneOffset;
	};

	struct LaneChangeTarget {
		xsd::Attribute<AbsoluteTargetLane> sub_AbsoluteTargetLane;
		xsd::Attribute<RelativeTargetLane> sub_RelativeTargetLane;
	};

	struct LaneOffsetAction {
		xsd::Attribute<LaneOffsetActionDynamics> sub_LaneOffsetActionDynamics;
		xsd::Attribute<LaneOffsetTarget> sub_LaneOffsetTarget;
		Boolean _continuous;
	};

	struct LaneOffsetActionDynamics {
		DynamicsShape _dynamicsShape;
		Double _maxLateralAcc;
	};

	struct LaneOffsetTarget {
		xsd::Attribute<AbsoluteTargetLaneOffset> sub_AbsoluteTargetLaneOffset;
		xsd::Attribute<RelativeTargetLaneOffset> sub_RelativeTargetLaneOffset;
	};

	struct LanePosition {
		xsd::Attribute<Orientation> sub_Orientation;
		String _laneId;
		Double _offset;
		String _roadId;
		Double _s;
	};

	struct LateralAction {
		xsd::Attribute<LaneChangeAction> sub_LaneChangeAction;
		xsd::Attribute<LaneOffsetAction> sub_LaneOffsetAction;
		xsd::Attribute<LateralDistanceAction> sub_LateralDistanceAction;
	};

	struct LateralDistanceAction {
		xsd::Attribute<DynamicConstraints> sub_DynamicConstraints;
		Boolean _continuous;
		Double _distance;
		String _entityRef;
		Boolean _freespace;
	};

	struct LongitudinalAction {
		xsd::Attribute<LongitudinalDistanceAction> sub_LongitudinalDistanceAction;
		xsd::Attribute<SpeedAction> sub_SpeedAction;
	};

	struct LongitudinalDistanceAction {
		xsd::Attribute<DynamicConstraints> sub_DynamicConstraints;
		Boolean _continuous;
		Double _distance;
		String _entityRef;
		Boolean _freespace;
		Double _timeGap;
	};

	struct Maneuver {
		xsd::Vector<Event> sub_Event{};
		xsd::Attribute<ParameterDeclarations> sub_ParameterDeclarations;
		String _name;
	};

	struct ManeuverCatalogLocation {
		xsd::Attribute<Directory> sub_Directory;
	};

	struct ManeuverGroup {
		xsd::Attribute<Actors> sub_Actors;
		xsd::Vector<CatalogReference> sub_CatalogReference{};
		xsd::Vector<Maneuver> sub_Maneuver{};
		UnsignedInt _maximumExecutionCount;
		String _name;
	};

	struct MiscObject {
		xsd::Attribute<BoundingBox> sub_BoundingBox;
		xsd::Attribute<ParameterDeclarations> sub_ParameterDeclarations;
		xsd::Attribute<Properties> sub_Properties;
		Double _mass;
		MiscObjectCategory _miscObjectCategory;
		String _name;
	};

	struct MiscObjectCatalogLocation {
		xsd::Attribute<Directory> sub_Directory;
	};

	struct ModifyRule {
		xsd::Attribute<ParameterAddValueRule> sub_AddValue;
		xsd::Attribute<ParameterMultiplyByValueRule> sub_MultiplyByValue;
	};

	struct None {
	};

	struct Nurbs {
		xsd::Vector<ControlPoint> sub_ControlPoint{};
		xsd::Vector<Knot> sub_Knot{};
		UnsignedInt _order;
	};

	struct ObjectController {
		xsd::Attribute<CatalogReference> sub_CatalogReference;
		xsd::Attribute<Controller> sub_Controller;
	};

	struct OffroadCondition {
		Double _duration;
	};

	struct OpenScenario {
		xsd::Attribute<Catalog> sub_Catalog;
		xsd::Attribute<CatalogLocations> sub_CatalogLocations;
		xsd::Attribute<Entities> sub_Entities;
		xsd::Attribute<FileHeader> sub_FileHeader;
		xsd::Attribute<ParameterDeclarations> sub_ParameterDeclarations;
		xsd::Attribute<RoadNetwork> sub_RoadNetwork;
		xsd::Attribute<Storyboard> sub_Storyboard;
	};

	struct Orientation {
		Double _h;
		Double _p;
		Double _r;
		ReferenceContext _type;
	};

	struct OverrideBrakeAction {
		Boolean _active;
		Double _value;
	};

	struct OverrideClutchAction {
		Boolean _active;
		Double _value;
	};

	struct OverrideControllerValueAction {
		xsd::Attribute<OverrideBrakeAction> sub_Brake;
		xsd::Attribute<OverrideClutchAction> sub_Clutch;
		xsd::Attribute<OverrideGearAction> sub_Gear;
		xsd::Attribute<OverrideParkingBrakeAction> sub_ParkingBrake;
		xsd::Attribute<OverrideSteeringWheelAction> sub_SteeringWheel;
		xsd::Attribute<OverrideThrottleAction> sub_Throttle;
	};

	struct OverrideGearAction {
		Boolean _active;
		Double _number;
	};

	struct OverrideParkingBrakeAction {
		Boolean _active;
		Double _value;
	};

	struct OverrideSteeringWheelAction {
		Boolean _active;
		Double _value;
	};

	struct OverrideThrottleAction {
		Boolean _active;
		Double _value;
	};

	struct ParameterAction {
		xsd::Attribute<ParameterModifyAction> sub_ModifyAction;
		xsd::Attribute<ParameterSetAction> sub_SetAction;
		String _parameterRef;
	};

	struct ParameterAddValueRule {
		Double _value;
	};

	struct ParameterAssignment {
		String _parameterRef;
		String _value;
	};

	struct ParameterAssignments {
		xsd::Vector<ParameterAssignment> sub_ParameterAssignment{};
	};

	struct ParameterCondition {
		String _parameterRef;
		Rule _rule;
		String _value;
	};

	struct ParameterDeclaration {
		String _name;
		ParameterType _parameterType;
		String _value;
	};

	struct ParameterDeclarations {
		xsd::Vector<ParameterDeclaration> sub_ParameterDeclaration{};
	};

	struct ParameterModifyAction {
		xsd::Attribute<ModifyRule> sub_Rule;
	};

	struct ParameterMultiplyByValueRule {
		Double _value;
	};

	struct ParameterSetAction {
		String _value;
	};

	struct Pedestrian {
		xsd::Attribute<BoundingBox> sub_BoundingBox;
		xsd::Attribute<ParameterDeclarations> sub_ParameterDeclarations;
		xsd::Attribute<Properties> sub_Properties;
		Double _mass;
		String _model;
		String _name;
		PedestrianCategory _pedestrianCategory;
	};

	struct PedestrianCatalogLocation {
		xsd::Attribute<Directory> sub_Directory;
	};

	struct Performance {
		Double _maxAcceleration;
		Double _maxDeceleration;
		Double _maxSpeed;
	};

	struct Phase {
		xsd::Vector<TrafficSignalState> sub_TrafficSignalState{};
		Double _duration;
		String _name;
	};

	struct Polyline {
		xsd::Vector<Vertex> sub_Vertex{};
	};

	struct Position {
		xsd::Attribute<LanePosition> sub_LanePosition;
		xsd::Attribute<RelativeLanePosition> sub_RelativeLanePosition;
		xsd::Attribute<RelativeObjectPosition> sub_RelativeObjectPosition;
		xsd::Attribute<RelativeRoadPosition> sub_RelativeRoadPosition;
		xsd::Attribute<RelativeWorldPosition> sub_RelativeWorldPosition;
		xsd::Attribute<RoadPosition> sub_RoadPosition;
		xsd::Attribute<RoutePosition> sub_RoutePosition;
		xsd::Attribute<WorldPosition> sub_WorldPosition;
	};

	struct PositionInLaneCoordinates {
		String _laneId;
		Double _laneOffset;
		Double _pathS;
	};

	struct PositionInRoadCoordinates {
		Double _pathS;
		Double _t;
	};

	struct PositionOfCurrentEntity {
		String _entityRef;
	};

	struct Precipitation {
		Double _intensity;
		PrecipitationType _precipitationType;
	};

	struct Private {
		xsd::Vector<PrivateAction> sub_PrivateAction{};
		String _entityRef;
	};

	struct PrivateAction {
		xsd::Attribute<ActivateControllerAction> sub_ActivateControllerAction;
		xsd::Attribute<ControllerAction> sub_ControllerAction;
		xsd::Attribute<LateralAction> sub_LateralAction;
		xsd::Attribute<LongitudinalAction> sub_LongitudinalAction;
		xsd::Attribute<RoutingAction> sub_RoutingAction;
		xsd::Attribute<SynchronizeAction> sub_SynchronizeAction;
		xsd::Attribute<TeleportAction> sub_TeleportAction;
		xsd::Attribute<VisibilityAction> sub_VisibilityAction;
	};

	struct Properties {
		xsd::Vector<File> sub_File{};
		xsd::Vector<Property> sub_Property{};
	};

	struct Property {
		String _name;
		String _value;
	};

	struct ReachPositionCondition {
		xsd::Attribute<Position> sub_Position;
		Double _tolerance;
	};

	struct RelativeDistanceCondition {
		String _entityRef;
		Boolean _freespace;
		RelativeDistanceType _relativeDistanceType;
		Rule _rule;
		Double _value;
	};

	struct RelativeLanePosition {
		xsd::Attribute<Orientation> sub_Orientation;
		Int _dLane;
		Double _ds;
		String _entityRef;
		Double _offset;
	};

	struct RelativeObjectPosition {
		xsd::Attribute<Orientation> sub_Orientation;
		Double _dx;
		Double _dy;
		Double _dz;
		String _entityRef;
	};

	struct RelativeRoadPosition {
		xsd::Attribute<Orientation> sub_Orientation;
		Double _ds;
		Double _dt;
		String _entityRef;
	};

	struct RelativeSpeedCondition {
		String _entityRef;
		Rule _rule;
		Double _value;
	};

	struct RelativeSpeedToMaster {
		SpeedTargetValueType _speedTargetValueType;
		Double _value;
	};

	struct RelativeTargetLane {
		String _entityRef;
		Int _value;
	};

	struct RelativeTargetLaneOffset {
		String _entityRef;
		Double _value;
	};

	struct RelativeTargetSpeed {
		Boolean _continuous;
		String _entityRef;
		SpeedTargetValueType _speedTargetValueType;
		Double _value;
	};

	struct RelativeWorldPosition {
		xsd::Attribute<Orientation> sub_Orientation;
		Double _dx;
		Double _dy;
		Double _dz;
		String _entityRef;
	};

	struct RoadCondition {
		xsd::Attribute<Properties> sub_Properties;
		Double _frictionScaleFactor;
	};

	struct RoadNetwork {
		xsd::Attribute<File> sub_LogicFile;
		xsd::Attribute<File> sub_SceneGraphFile;
		xsd::Attribute<TrafficSignals> sub_TrafficSignals;
	};

	struct RoadPosition {
		xsd::Attribute<Orientation> sub_Orientation;
		String _roadId;
		Double _s;
		Double _t;
	};

	struct Route {
		xsd::Attribute<ParameterDeclarations> sub_ParameterDeclarations;
		xsd::Vector<Waypoint> sub_Waypoint{};
		Boolean _closed;
		String _name;
	};

	struct RouteCatalogLocation {
		xsd::Attribute<Directory> sub_Directory;
	};

	struct RoutePosition {
		xsd::Attribute<InRoutePosition> sub_InRoutePosition;
		xsd::Attribute<Orientation> sub_Orientation;
		xsd::Attribute<RouteRef> sub_RouteRef;
	};

	struct RouteRef {
		xsd::Attribute<CatalogReference> sub_CatalogReference;
		xsd::Attribute<Route> sub_Route;
	};

	struct RoutingAction {
		xsd::Attribute<AcquirePositionAction> sub_AcquirePositionAction;
		xsd::Attribute<AssignRouteAction> sub_AssignRouteAction;
		xsd::Attribute<FollowTrajectoryAction> sub_FollowTrajectoryAction;
	};

	struct ScenarioObject {
		xsd::Attribute<CatalogReference> sub_CatalogReference;
		xsd::Attribute<MiscObject> sub_MiscObject;
		xsd::Attribute<ObjectController> sub_ObjectController;
		xsd::Attribute<Pedestrian> sub_Pedestrian;
		xsd::Attribute<Vehicle> sub_Vehicle;
		String _name;
	};

	struct SelectedEntities {
		xsd::Attribute<ByType> sub_ByType;
		xsd::Attribute<EntityRef> sub_EntityRef;
	};

	struct Shape {
		xsd::Attribute<Clothoid> sub_Clothoid;
		xsd::Attribute<Nurbs> sub_Nurbs;
		xsd::Attribute<Polyline> sub_Polyline;
	};

	struct SimulationTimeCondition {
		Rule _rule;
		Double _value;
	};

	struct SpeedAction {
		xsd::Attribute<TransitionDynamics> sub_SpeedActionDynamics;
		xsd::Attribute<SpeedActionTarget> sub_SpeedActionTarget;
	};

	struct SpeedActionTarget {
		xsd::Attribute<AbsoluteTargetSpeed> sub_AbsoluteTargetSpeed;
		xsd::Attribute<RelativeTargetSpeed> sub_RelativeTargetSpeed;
	};

	struct SpeedCondition {
		Rule _rule;
		Double _value;
	};

	struct StandStillCondition {
		Double _duration;
	};

	struct Story {
		xsd::Vector<Act> sub_Act{};
		xsd::Attribute<ParameterDeclarations> sub_ParameterDeclarations;
		String _name;
	};

	struct Storyboard {
		xsd::Attribute<Init> sub_Init;
		xsd::Attribute<Trigger> sub_StopTrigger;
		xsd::Vector<Story> sub_Story{};
	};

	struct StoryboardElementStateCondition {
		StoryboardElementState _state;
		String _storyboardElementRef;
		StoryboardElementType _storyboardElementType;
	};

	struct Sun {
		Double _azimuth;
		Double _elevation;
		Double _intensity;
	};

	struct SynchronizeAction {
		xsd::Attribute<FinalSpeed> sub_FinalSpeed;
		xsd::Attribute<Position> sub_TargetPosition;
		xsd::Attribute<Position> sub_TargetPositionMaster;
		String _masterEntityRef;
	};

	struct TeleportAction {
		xsd::Attribute<Position> sub_Position;
	};

	struct TimeHeadwayCondition {
		Boolean _alongRoute;
		String _entityRef;
		Boolean _freespace;
		Rule _rule;
		Double _value;
	};

	struct TimeOfDay {
		Boolean _animation;
		DateTime _dateTime;
	};

	struct TimeOfDayCondition {
		DateTime _dateTime;
		Rule _rule;
	};

	struct TimeReference {
		xsd::Attribute<None> sub_None;
		xsd::Attribute<Timing> sub_Timing;
	};

	struct TimeToCollisionCondition {
		xsd::Attribute<TimeToCollisionConditionTarget> sub_TimeToCollisionConditionTarget;
		Boolean _alongRoute;
		Boolean _freespace;
		Rule _rule;
		Double _value;
	};

	struct TimeToCollisionConditionTarget {
		xsd::Attribute<EntityRef> sub_EntityRef;
		xsd::Attribute<Position> sub_Position;
	};

	struct Timing {
		ReferenceContext _domainAbsoluteRelative;
		Double _offset;
		Double _scale;
	};

	struct TrafficAction {
		xsd::Attribute<TrafficSinkAction> sub_TrafficSinkAction;
		xsd::Attribute<TrafficSourceAction> sub_TrafficSourceAction;
		xsd::Attribute<TrafficSwarmAction> sub_TrafficSwarmAction;
	};

	struct TrafficDefinition {
		xsd::Attribute<ControllerDistribution> sub_ControllerDistribution;
		xsd::Attribute<VehicleCategoryDistribution> sub_VehicleCategoryDistribution;
		String _name;
	};

	struct TrafficSignalAction {
		xsd::Attribute<TrafficSignalControllerAction> sub_TrafficSignalControllerAction;
		xsd::Attribute<TrafficSignalStateAction> sub_TrafficSignalStateAction;
	};

	struct TrafficSignalCondition {
		String _name;
		String _state;
	};

	struct TrafficSignalController {
		xsd::Vector<Phase> sub_Phase{};
		Double _delay;
		String _name;
		String _reference;
	};

	struct TrafficSignals {
		xsd::Vector<TrafficSignalController> sub_TrafficSignalController{};
	};

	struct TrafficSignalControllerAction {
		String _phase;
		String _trafficSignalControllerRef;
	};

	struct TrafficSignalControllerCondition {
		String _phase;
		String _trafficSignalControllerRef;
	};

	struct TrafficSignalState {
		String _state;
		String _trafficSignalId;
	};

	struct TrafficSignalStateAction {
		String _name;
		String _state;
	};

	struct TrafficSinkAction {
		xsd::Attribute<Position> sub_Position;
		xsd::Attribute<TrafficDefinition> sub_TrafficDefinition;
		Double _radius;
		Double _rate;
	};

	struct TrafficSourceAction {
		xsd::Attribute<Position> sub_Position;
		xsd::Attribute<TrafficDefinition> sub_TrafficDefinition;
		Double _radius;
		Double _rate;
		Double _velocity;
	};

	struct TrafficSwarmAction {
		xsd::Attribute<CentralSwarmObject> sub_CentralObject;
		xsd::Attribute<TrafficDefinition> sub_TrafficDefinition;
		Double _innerRadius;
		UnsignedInt _numberOfVehicles;
		Double _offset;
		Double _semiMajorAxis;
		Double _semiMinorAxis;
		Double _velocity;
	};

	struct Trajectory {
		xsd::Attribute<ParameterDeclarations> sub_ParameterDeclarations;
		xsd::Attribute<Shape> sub_Shape;
		Boolean _closed;
		String _name;
	};

	struct TrajectoryCatalogLocation {
		xsd::Attribute<Directory> sub_Directory;
	};

	struct TrajectoryFollowingMode {
		FollowingMode _followingMode;
	};

	struct TransitionDynamics {
		DynamicsDimension _dynamicsDimension;
		DynamicsShape _dynamicsShape;
		Double _value;
	};

	struct TraveledDistanceCondition {
		Double _value;
	};

	struct Trigger {
		xsd::Vector<ConditionGroup> sub_ConditionGroup{};
	};

	struct TriggeringEntities {
		xsd::Vector<EntityRef> sub_EntityRef{};
		TriggeringEntitiesRule _triggeringEntitiesRule;
	};

	struct UserDefinedAction {
		xsd::Attribute<CustomCommandAction> sub_CustomCommandAction;
	};

	struct UserDefinedValueCondition {
		String _name;
		Rule _rule;
		String _value;
	};

	struct Vehicle {
		xsd::Attribute<Axles> sub_Axles;
		xsd::Attribute<BoundingBox> sub_BoundingBox;
		xsd::Attribute<ParameterDeclarations> sub_ParameterDeclarations;
		xsd::Attribute<Performance> sub_Performance;
		xsd::Attribute<Properties> sub_Properties;
		String _name;
		VehicleCategory _vehicleCategory;
	};

	struct VehicleCatalogLocation {
		xsd::Attribute<Directory> sub_Directory;
	};

	struct VehicleCategoryDistribution {
		xsd::Vector<VehicleCategoryDistributionEntry> sub_VehicleCategoryDistributionEntry{};
	};

	struct VehicleCategoryDistributionEntry {
		VehicleCategory _category;
		Double _weight;
	};

	struct Vertex {
		xsd::Attribute<Position> sub_Position;
		Double _time;
	};

	struct VisibilityAction {
		Boolean _graphics;
		Boolean _sensors;
		Boolean _traffic;
	};

	struct Waypoint {
		xsd::Attribute<Position> sub_Position;
		RouteStrategy _routeStrategy;
	};

	struct Weather {
		xsd::Attribute<Fog> sub_Fog;
		xsd::Attribute<Precipitation> sub_Precipitation;
		xsd::Attribute<Sun> sub_Sun;
		CloudState _cloudState;
	};

	struct WorldPosition {
		Double _h;
		Double _p;
		Double _r;
		Double _x;
		Double _y;
		Double _z;
	};


} // namespace osc

#endif // OSC_STRUCTURE_H
