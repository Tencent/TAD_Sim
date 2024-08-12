// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/tx_lane_geom_info.h"
#include "tx_centripetal_cat_mull.h"
#include "tx_planning_flags.h"
#include "tx_planning_vehicle_element.h"
#include "tx_trajectory_follower.h"
// #define SM_ASSERT(_Condition_, _Msg_) do { if (CallFail(_Condition_)) { LOG(WARNING) << _Msg_; system("pause"); } }
// while (false)
#define SM_ASSERT(_Condition_, _Msg_)
TX_NAMESPACE_OPEN(TrafficFlow)

class SimPlanningVehicleElement_TrajectoryFollow : public SimPlanningVehicleElement {
 public:
  using ParentClass = SimPlanningVehicleElement;
  using SegmentKinectParam = Base::SegmentKinectParam;
  using SegmentKinectParamList = Base::SegmentKinectParamList;
  using controlPoint = HdMap::txLaneInfoInterface::controlPoint;
  using controlPointVec = HdMap::txLaneInfoInterface::controlPointVec;
  using control_path_node_vec = CentripetalCatMull::control_path_node_vec;
  using ControlPathGear = Base::SegmentKinectParam::ControlPathGear;

 public:
  SimPlanningVehicleElement_TrajectoryFollow() TX_DEFAULT;
  virtual ~SimPlanningVehicleElement_TrajectoryFollow() TX_DEFAULT;
  virtual txBool InitializeEgo(sim_msg::Location& refEgoData, const control_path_node_vec& ref_control_path,
                               ISceneLoader::IViewerPtr _viewPtr, ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool InitializeTrajMgr(const control_path_node_vec& ref_control_path, const txFloat startTime) TX_NOEXCEPT;
  virtual txBool Pre_Update(const TimeParamManager&,
                            std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool FillingElement(Base::TimeParamManager const&, sim_msg::Location&) TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool FillingTrajectory(Base::TimeParamManager const&, sim_msg::Trajectory&) TX_NOEXCEPT TX_OVERRIDE;
  virtual void OnStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool CheckDeadLine() TX_NOEXCEPT TX_OVERRIDE;
  virtual txFloat DeadLineRadius() const TX_NOEXCEPT { return FLAGS_ego_DeadLineRadius; }
  virtual txBool Update_Kinetics(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;
  virtual txFloat Compute_Displacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  virtual std::tuple<txFloat, txFloat> ComputeTrajKinetics(const txFloat init_v, const txFloat final_v,
                                                           const txFloat s) const TX_NOEXCEPT;
  virtual Base::txFloat DistanceToEnd() const TX_NOEXCEPT {
    return (NonNull_Pointer(m_trajMgr)) ? (m_trajMgr->GetLength() - m_real_displacement) : (0.0);
  }
  virtual Base::txBool IsInTurnRight_InLane() const TX_NOEXCEPT { return false; }

 protected:
  HdMap::txLaneInfoInterfacePtr m_trajMgr = nullptr;
  SegmentKinectParamList m_KinectParamList;
  txFloat m_real_displacement = 0.0;
  Base::SegmentKinectParam m_cur_kiniect_param;
};
using SimPlanningVehicleElement_TrajectoryFollowPtr = std::shared_ptr<SimPlanningVehicleElement_TrajectoryFollow>;

class SimPlanningVehicleElement_BareMode : public SimPlanningVehicleElement {
 public:
  using ParentClass = SimPlanningVehicleElement;

 public:
  SimPlanningVehicleElement_BareMode() TX_DEFAULT;
  virtual ~SimPlanningVehicleElement_BareMode() TX_DEFAULT;

  virtual txFloat DrivingFollow(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE {
    return mKinetics.m_acceleration;
  }
  virtual txBool CheckPedestrianWalking(const txFloat relative_time,
                                        txFloat& refLateralVelocity) TX_NOEXCEPT TX_OVERRIDE {
    return false;
  }
  virtual txFloat DrivingChangeLane(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return 0.0; }
  virtual txEgoSignalInfoPtr env_signal_ptr() const TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
};
using SimPlanningVehicleElement_BareModePtr = std::shared_ptr<SimPlanningVehicleElement_BareMode>;

TX_NAMESPACE_CLOSE(TrafficFlow)
