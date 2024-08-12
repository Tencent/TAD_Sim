// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/hashed_lane_info_orthogonal_list.h"
#include "HdMap/tx_hashed_lane_info.h"
#include "tad_component.h"
#include "tad_planning.h"
#include "tad_vehicle_scan_region.h"
#include "tad_behavior_time_manager.h"
#include "tad_fsm_ai.h"
#include "tx_centripetal_cat_mull.h"
#include "tx_driving_change_lane.h"
#include "tx_driving_follower.h"
#include "tx_dummy_drive_control.h"
#include "tx_env_perception.h"
#include "tx_header.h"
#include "tx_planning_flags.h"
#include "tx_planning_nearest_object.h"
#include "tx_planning_scene_event_module.h"
#include "tx_scene_loader.h"
#include "tx_traffic_junction_rules.h"
#include "tx_traffic_rules.h"
#include "tx_vehicle_element.h"
#include "union.pb.h"
#include "vehicle_geometry.pb.h"
TX_NAMESPACE_OPEN(TrafficFlow)
class SimPlanningVehicleElement : public Base::IVehicleElement,
                                  public SM::txAITrafficState,
                                  public Base::IEnvPerception_TAD_AI_Hashed,
                                  public Base::IDrivingChangeLane,
                                  public Base::ITrafficRules,
                                  public Base::IDrivingFollower,
                                  public Base::ITrafficJunctionRules,
                                  public TAD_PlanningModule,
#if __Control_V2__
                                  public DummyDriverControl,
#endif /*__Control_V2__*/
                                  public txPlanningSceneEventModule {
 public:
  using ParentClass = Base::IVehicleElement;
  using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
  using SimulationConsistencyPtr = Base::SimulationConsistencyPtr;
  using AdjoinContainerType = HashedLaneInfoOrthogonalList::AdjoinContainerType;
  using control_path_node_vec = CentripetalCatMull::control_path_node_vec;
  SimPlanningVehicleElement() TX_DEFAULT;
  virtual ~SimPlanningVehicleElement() TX_DEFAULT;

 public:
  virtual txBool InitializeEgo(sim_msg::Location& refEgoData, const control_path_node_vec& ref_control_path,
                               ISceneLoader::IViewerPtr _viewPtr, ISceneLoaderPtr) TX_NOEXCEPT;
  virtual txBool Initialize(ISceneLoader::IViewerPtr, ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE { return false; }
  virtual txBool Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool Release() TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool FillingElement(Base::TimeParamManager const&, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE {
    return true;
  }
  virtual sim_msg::Car* FillingElement(txFloat const timeStamp, sim_msg::Car* pSimVehicle) TX_NOEXCEPT TX_OVERRIDE {
    return nullptr;
  }
  virtual txBool FillingElement(Base::TimeParamManager const&, sim_msg::Location&) TX_NOEXCEPT;
  virtual txBool FillingTrajectory(Base::TimeParamManager const&, sim_msg::Trajectory&) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool CheckReachable(const Base::Info_Lane_t& _laneInfo) const TX_NOEXCEPT TX_OVERRIDE {
    return mRouteAI.CheckReachable(_laneInfo);
  }
  virtual txBool Pre_Update(const TimeParamManager&,
                            std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT TX_OVERRIDE {
    return true;
  }
  virtual txBool Post_Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool InjectEnv(Base::TimeParamManager const& timeMgr, const txString& strPB) TX_NOEXCEPT;
  virtual txBool InjectLoc(Base::TimeParamManager const& timeMgr, const txString& strPB) TX_NOEXCEPT;
  virtual txBool InjectUnionLoc(Base::TimeParamManager const& timeMgr, const txString groupName,
                                const txString& strPB) TX_NOEXCEPT;
  virtual void ResetLaneKeep() TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool EventChangeLane(Base::TimeParamManager const& timeMgrtxInt, txInt dir) TX_NOEXCEPT;

 public:
  virtual VEHICLE_BEHAVIOR VehicleBehavior() const TX_NOEXCEPT TX_OVERRIDE { return VEHICLE_BEHAVIOR::ePlanning; }
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return ElementType::TAD_Ego; }
  virtual txBool CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool CheckEnd(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return IsAlive(); }
  virtual txBool CheckDeadLine() TX_NOEXCEPT TX_OVERRIDE;

  virtual Base::txBool IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT TX_OVERRIDE {
    return (+VEHICLE_BEHAVIOR::ePlanning == behavior);
  }

 public:
  virtual VEHICLE_TYPE VehicleType() const TX_NOEXCEPT TX_OVERRIDE { return VEHICLE_TYPE::SUV; }
  virtual txFloat Compute_LateralDisplacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual VehicleMoveLaneState GetSwitchLaneState() const TX_NOEXCEPT TX_OVERRIDE;
  virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE { return "SimPlanningVehicleElement"; }

 protected:
  virtual txBool Initialize_Kinetics(const txFloat start_v, const txFloat max_v, const txFloat max_acce,
                                     const txFloat max_dece) TX_NOEXCEPT;
  virtual txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE;

  virtual txBool LaneChangePostOp(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;
  virtual txBool UpdateSwitchLaneRunning(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;
  virtual txFloat Get_MoveTo_Sideway_Angle(const txFloat exper_time) TX_NOEXCEPT;
  virtual txFloat MoveBack_Sideway_Angle(const txFloat time) TX_NOEXCEPT;
  virtual txBool MoveStraightOnS(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;
  void CheckWayPoint() TX_NOEXCEPT;
  virtual txBool IsUnderLaneKeep() const TX_NOEXCEPT;
  Base::txFloat Random(const Base::txString& strMark) TX_NOEXCEPT;
  Base::txFloat GetRandomValue_NegOne2PosOne(Base::txString strMark) TX_NOEXCEPT;
#if __TX_Mark__("switch lane")
  virtual txFloat Driving_SwitchLaneOrg(const Base::TimeParamManager& timeMgr,
                                        Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base,
                                        Base::SimulationConsistencyPtr RearVeh_CurrentLane_Base,
                                        Base::SimulationConsistencyPtr LeftVeh_TargetLane_Base,
                                        Base::SimulationConsistencyPtr RightVeh_TargetLane_Base) TX_NOEXCEPT;
  virtual txBool SwitchLane(const PedestrianSearchType state,
                            Base::txFloat laneChangeTime = FLAGS_Average_LaneChanging_Duration_Short) TX_NOEXCEPT;
  virtual LaneChangeIntention LaneChanging(Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base,
                                           Base::SimulationConsistencyPtr RearVeh_CurrentLane_Base,
                                           Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base,
                                           Base::SimulationConsistencyPtr RearVeh_TargetLane_Base,
                                           const txFloat FrontGap_Current, const txFloat RearGap_Current,
                                           txFloat FrontGap_TargetLane, const txFloat RearGap_TargetLane,
                                           const txFloat SideGap_TargetLane,
                                           const PedestrianSearchType targetType) TX_NOEXCEPT;
  virtual txFloat ComputeMinLaneChangeTime(const Base::txFloat laneChangeTime) TX_NOEXCEPT;
  virtual txBool CheckCanSwitchRightLaneState(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool CheckCanSwitchLeftLaneState(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual txFloat FrontPedWalkingInSight(const PedestrianSearchType targetType,
                                         const Base::txFloat distance) TX_NOEXCEPT {
    return 0.0;
  }
  virtual txFloat MoveBack_Sideway(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual txFloat Get_MoveBack_Sideway(const txFloat exper_time) TX_NOEXCEPT;
  virtual txFloat MoveTo_Sideway(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual txFloat Get_MoveTo_Sideway(const txFloat exper_time) TX_NOEXCEPT;
  virtual txFloat DrivingChangeLane(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
#endif /*__TX_Mark__("switch lane")*/

#if __TX_Mark__("driving follow")
  using DrivingFollowType = Base::IDrivingFollower::DrivingFollowType;
  virtual txFloat DrivingFollow(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual txFloat DrivingFollowNormal(const Base::TimeParamManager& timeMgr,
                                      Base::SimulationConsistencyPtr frontElementPtr, const txFloat frontDist,
                                      const Base::txBool bHurryToExitState = false) TX_NOEXCEPT;

#endif /*__TX_Mark__("driving follow")*/

#if __TX_Mark__("ITrafficJunctionRules")
  virtual void UpdateNextCrossDirection() TX_NOEXCEPT TX_OVERRIDE {}
  virtual txFloat CheckSignLight(const txSysId singalId, const txFloat distance,
                                 const SIGN_LIGHT_COLOR_TYPE lightType) TX_NOEXCEPT TX_OVERRIDE;
  virtual txFloat TurnLeft_vs_GoStraight(const Base::IVehicleElementPtr pOpposingVeh, const txFloat distance,
                                         const txFloat acceleration) const TX_NOEXCEPT TX_OVERRIDE;
  virtual txFloat TurnRight_vs_TurnLeft_GoStraight(const Base::IVehicleElementPtr pOpposingVeh,
                                                   const Base::IVehicleElementPtr pLeftApproachingVeh,
                                                   const txFloat distance2signal,
                                                   const txFloat acceleration) const TX_NOEXCEPT TX_OVERRIDE;
#endif /*__TX_Mark__("ITrafficJunctionRules")*/

#if __TX_Mark__("pedestrian")
  virtual txBool CheckPedestrianWalking(const txFloat relative_time, txFloat& refLateralVelocity) TX_NOEXCEPT;
#endif /*__TX_Mark__("pedestrian")*/
#if USE_SUDOKU_GRID
 protected:
  virtual void Get_Region_Clockwise_Close_By_Enum(const Base::Enums::NearestElementDirection eValue,
                                                  std::vector<Base::txVec2>& retRegion) const TX_NOEXCEPT TX_OVERRIDE;
  virtual void Compute_Region_Clockwise_Close(const Coord::txENU& vehPos, const Base::txVec3& laneDir,
                                              const Unit::txDegree& headingDegree) TX_NOEXCEPT TX_OVERRIDE;
#endif /*USE_SUDOKU_GRID*/
 public:
  TX_MARK("IEnvPerception")
  using Info_NearestObject = Base::Info_NearestObject;
  using Info_NearestDynamic = Base::Info_NearestDynamic;
  using Info_NearestCrossRoadVehicle = Base::Info_NearestCrossRoadVehicle;
  virtual void ConfigureFocusTypes() TX_NOEXCEPT TX_OVERRIDE;
  virtual void ExecuteEnvPerception(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual void SearchSurroundingVehicle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual Info_NearestObject SearchSurroundingVehicle_RTree(const Base::TimeParamManager& timeMgr)
      TX_NOEXCEPT TX_OVERRIDE;
  virtual Info_NearestObject SearchSurroundingVehicle_HLane(const Base::TimeParamManager& timeMgr)
      TX_NOEXCEPT TX_OVERRIDE;
  virtual void Search_Follow_Front(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual void SearchSurroundingPedestrian(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual void SearchSurroundingObstacle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual void SearchSurroundingSignal(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual void CheckRouteEnd(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual void SearchNextCrossVehicle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> FrontVeh_CurrentLane() const TX_NOEXCEPT
      TX_OVERRIDE;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RearVeh_CurrentLane() const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> FrontVeh_TargetLane() const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> FrontVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RearVeh_TargetLane() const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RearVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> Any_SideVeh_TargetLane() const TX_NOEXCEPT
      TX_OVERRIDE;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SideVeh_TargetLane() const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SideVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> LeftVeh_TargetLane() const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RightVeh_TargetLane() const TX_NOEXCEPT TX_OVERRIDE;

  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> OpposeSideVeh_NextCross() const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> SameSideVeh_NextCross() const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> LeftSideVeh_NextCross() const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> RightSideVeh_NextCross() const TX_NOEXCEPT TX_OVERRIDE;

  virtual Base::txBool ValidSampling(const Base::TimeParamManager&) const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  virtual void RegisterInfoOnInit(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE;
  virtual void UpdateHashedLaneInfo(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE;
  virtual void UnRegisterInfoOnKill() TX_NOEXCEPT TX_OVERRIDE {}

 protected:
  virtual txFloat env_dist(const txInt j_dir) const TX_NOEXCEPT { return m_EgoNearestObject.dist(j_dir); }
  virtual SimulationConsistencyPtr env_elemPtr(const txInt j_dir) const TX_NOEXCEPT {
    return m_EgoNearestObject.elemPtr(j_dir);
  }
  virtual txEgoSignalInfoPtr env_signal_ptr() const TX_NOEXCEPT { return m_signal_ptr; }
  virtual txFloat env_signal_dist() const TX_NOEXCEPT { return mDistanceToTargetSignLight; }
  virtual void clear_env_signal() TX_NOEXCEPT {
    m_signal_ptr = nullptr;
    mDistanceToTargetSignLight = FLT_MAX;
  }
  virtual void Clear_Follow_Info() TX_NOEXCEPT TX_OVERRIDE {
    Base::IEnvPerception_TAD_AI_Hashed::Clear_Follow_Info();
    m_follow_front_hashnode.clear();
  }

 public:
  virtual hadmap::txLaneLinkPtr RandomNextLink(const hadmap::txLaneLinks& nextLaneLinks) TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool UpdateLocation(const Base::TimeParamManager&) TX_NOEXCEPT;

 public:
  using EgoSubType = Base::Enums::EgoSubType;
  void SetSubType(EgoSubType _subType) TX_NOEXCEPT { m_SubType = _subType; }
  EgoSubType GetSubType() const TX_NOEXCEPT { return m_SubType; }

  Base::txBool IsLeader() const TX_NOEXCEPT { return (_plus_(EgoSubType::eLeader) == GetSubType()); }
  Base::txBool IsFollower() const TX_NOEXCEPT { return !(IsLeader()); }

 protected:
  virtual txBool SyncPosition(const txFloat _passTime) TX_NOEXCEPT TX_OVERRIDE;
  virtual Coord::txENU StableRawVehicleCoord() const TX_NOEXCEPT;

 protected:
  virtual Base::txFloat local_cartesian_point_x() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }
  virtual Base::txFloat local_cartesian_point_y() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }
  virtual Base::txFloat local_cartesian_point_z() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

 protected:
  virtual txBool Ignore(const sim_msg::Car& refCar) const TX_NOEXCEPT { return false; }
  virtual txBool Ignore(const sim_msg::DynamicObstacle& refPed) const TX_NOEXCEPT;
  virtual txBool Ignore(const sim_msg::StaticObstacle& refObs) const TX_NOEXCEPT;

 protected:
  Base::txFloat RefuseLaneChangeVelocity(/* m/s */) const TX_NOEXCEPT { return FLAGS_Ego_RefuseLaneChangeVelocity; }
  Base::txFloat LaneKeepTime(/*second*/) const TX_NOEXCEPT { return mLaneKeepTime; }
  Base::txFloat RawLaneKeepTime() const TX_NOEXCEPT { return FLAGS_ego_lane_keep; }
  inline Base::txFloat SignalReactionGap() const TX_NOEXCEPT;

 public:
  virtual const Unit::txDegree& GetAngle() const TX_NOEXCEPT { return mLocation.fAngle(); }
  struct SceneEventTriggerInfo {
    using PolygonArray = Geometry::OBB2D::PolygonArray;
    Base::txFloat GetRawVelocity() const TX_NOEXCEPT { return mRawVelocity; }
    Base::txFloat GetLateralVelocity() const TX_NOEXCEPT { return mLateralVelocity; }
    const PolygonArray& GetPolygon() const TX_NOEXCEPT { return mPolygonArray; }
    Coord::txENU StableGeomCenter() const TX_NOEXCEPT { return mStableGeomCenter; }
    Base::txSysId Id() const TX_NOEXCEPT { return mId; }
    Base::txFloat GetLength() const TX_NOEXCEPT { return mLength; }
    Base::txFloat GetWidth() const TX_NOEXCEPT { return mWidth; }
    Base::txFloat GetHeight() const TX_NOEXCEPT { return mHeight; }

    Coord::txENU mStableGeomCenter;
    Base::txFloat mRawVelocity = 0.0;
    Base::txFloat mLateralVelocity = 0.0;
    PolygonArray mPolygonArray;
    Base::txSysId mId;
    Base::txFloat mLength = 0.0, mWidth = 0.0, mHeight = 0.0;
  };
  using SceneEventTriggerInfoPtr = std::shared_ptr<SceneEventTriggerInfo>;
  using DistanceProjectionType = Base::Enums::DistanceProjectionType;
  virtual SceneEventTriggerInfoPtr QuerySceneEventTriggerInfoFromEnvData(const ElementType _type,
                                                                         const Base::txSysId _elemId) const TX_NOEXCEPT;
  virtual Base::txFloat Compute_EGO_Distance(SceneEventTriggerInfoPtr target_ptr,
                                             const DistanceProjectionType projType) TX_NOEXCEPT;
  virtual Base::txFloat Compute_TTC_Distance(SceneEventTriggerInfoPtr target_ptr,
                                             const DistanceProjectionType projType) TX_NOEXCEPT;
  virtual FrenetProjInfo ComputeRoadProjectDistance(const Coord::txENU& target_element_geom_center) const TX_NOEXCEPT
      TX_OVERRIDE;

 protected:
  virtual SceneEventTriggerInfoPtr QuerySceneEventTriggerInfoFromEnvData_Car(const Base::txSysId _elemId) const
      TX_NOEXCEPT;
  virtual SceneEventTriggerInfoPtr QuerySceneEventTriggerInfoFromEnvData_Ped(const Base::txSysId _elemId) const
      TX_NOEXCEPT;
  virtual SceneEventTriggerInfoPtr QuerySceneEventTriggerInfoFromEnvData_Obs(const Base::txSysId _elemId) const
      TX_NOEXCEPT;

 public:
  virtual Base::txBool RelocateTracker(hadmap::txLanePtr pLane, const txFloat& _timestamp) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool RelocateTracker(hadmap::txLaneLinkPtr pLaneLink,
                                       const txFloat& _timestamp) TX_NOEXCEPT TX_OVERRIDE {
    return ParentClass::RelocateTracker(pLaneLink, _timestamp);
  }

 protected:
  virtual void UpdateLaneKeep(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual Base::txFloat SearchRoadEnd(const Base::txFloat search_distance) TX_NOEXCEPT;
  virtual txFloat route_end_dist() const TX_NOEXCEPT { return mDistanceToRouteEnd; }
  virtual void clear_route_end() TX_NOEXCEPT { mDistanceToRouteEnd = FLT_MAX; }
  virtual txFloat CheckRouteEnd(const txFloat distance) TX_NOEXCEPT;
  virtual void DecelerateLaneChange(const Base::txFloat velocity_factor, const Base::txFloat time_factor) TX_NOEXCEPT;
  virtual void DecelerateLaneChange(const Base::txFloat velocity_factor) TX_NOEXCEPT;
  txFloat DecelIntenseTimeFactor() const TX_NOEXCEPT { return FLAGS_decelerate_lanechange_intense_lanekeepfactor; }
  txFloat DecelIntenseSpeedFactor() const TX_NOEXCEPT { return FLAGS_decelerate_lanechange_intense_speedfactor; }
  txFloat DecelActionTimeFactor() const TX_NOEXCEPT { return FLAGS_decelerate_lanechange_lanekeepfactor; }
  txFloat DecelActionSpeedFactor() const TX_NOEXCEPT { return FLAGS_decelerate_lanechange_speedfactor; }
#if __Control_V2__
 public:
  virtual void UpdateControlPb(Base::TimeParamManager const& timeMgr, const Base::txFloat _acc,
                               const Base::txFloat _front_wheel_angle) TX_NOEXCEPT TX_OVERRIDE;
#endif /*__Control_V2__*/

 protected:
  void SetPlanningVelocity(const txFloat v, const txBool _forced = false) TX_NOEXCEPT;
  double AdjustThetaFromLastRpy(const Base::txFloat current_hdg, const Base::txFloat target_hdg,
                                const Base::txFloat max_hdg_change);

 protected:
  sim_msg::Location m_ego_init_location;
  Scene::VehicleScanRegion2D mVehicleScanRegion2D;
  Ego_NearestObject m_EgoNearestObject;
  Ego_NearestDynamicObs m_NearestDynamic;
  TAD_BehaviorTimeManager m_LaneAbortingTimeMgr;
  TAD_BehaviorTimeManager m_LaneChangeTimeMgr;
  Info_NearestCrossRoadVehicle m_NearestCrossRoadVehicle;
  std::set<Base::ITrafficElement::ElementType> m_vehicle_type_filter_set;
  sim_msg::Traffic m_env_traffic;
  sim_msg::Location m_dynamic_location;
  sim_msg::Union m_union_location;
  txEgoSignalInfoPtr m_signal_ptr = nullptr;
  AdjoinContainerType m_follow_front_hashnode;
  std::vector<txEgoEnvVehicleInfoPtr> m_surround_element_circle;

  EgoSubType m_SubType = _plus_(EgoSubType::eLeader);

  Base::txFloat mDistanceToRouteEnd = FLT_MAX;
  Base::txFloat mLaneKeepTime = 5.0;
  Base::txInt mEventChangeLane = 0;
  Base::txFloat mLastLaneChange = 9999.0;
  Base::txFloat mBeforeChangeLane = 0.0;
  std::string m_groupName;
};

using SimPlanningVehicleElementPtr = std::shared_ptr<SimPlanningVehicleElement>;
TX_NAMESPACE_CLOSE(TrafficFlow)
