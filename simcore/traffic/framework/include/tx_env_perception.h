// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/hashed_lane_info_orthogonal_list.h"
#include "HdMap/tx_hashed_lane_info.h"
#include "HdMap/tx_hashed_road.h"
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_serialization.h"
#include "tx_signal_element.h"
#include "tx_sim_time.h"
#include "tx_vehicle_element.h"
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/
TX_NAMESPACE_OPEN(Base)

#if __TX_Mark__("nearest object")
struct Info_NearestCrossRoadVehicle {
  static constexpr txInt N_Dir = Base::Enums::szVehicle2Vehicle_RelativeDirection;
  Info_NearestCrossRoadVehicle() { clear(); }

  void clear() TX_NOEXCEPT {
    aroudDistance.fill(FLAGS_SUDOKU_GRID_MAX_DISTANCE);
    aroudElemPtr.fill(nullptr);
  }
  std::array<Base::IVehicleElementPtr, N_Dir> aroudElemPtr;
  std::array<Base::txFloat, N_Dir> aroudDistance;
};

#endif /*__TX_Mark__("nearest object")*/

class IEnvPerception {
 public:
  using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
  using HashedLaneInfoPtr = Geometry::SpatialQuery::HashedLaneInfoPtr;
  using HashedLaneInfoOrthogonalList = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList;
  using HashedLaneInfoOrthogonalListPtr = Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr;
  using Weak_HashedLaneInfoOrthogonalListPtr = Geometry::SpatialQuery::Weak_HashedLaneInfoOrthogonalListPtr;
  using OrthogonalListSearchCmd = HashedLaneInfoOrthogonalList::OrthogonalListSearchCmd;

 public:
  virtual ~IEnvPerception() TX_DEFAULT;
  virtual void ExecuteEnvPerception(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;

  /**
   * @brief 获取样本数据的采样方式
   *
   * @return 返回样本数据的采样方式
   */
  virtual Base::txBool ForceSampling() const TX_NOEXCEPT { return false; }

 protected:
  /**
   * @brief 更新hash道路缓存
   *
   * @param newHashedLaneInfo 新的hash道路信息
   */
  virtual void UpdateHashedRoadInfoCache(const HashedLaneInfo& newHashedLaneInfo) TX_NOEXCEPT {
    if (CallFail(newHashedLaneInfo == std::get<0>(m_HashedRoadInfoCache)) || CallSucc(ForceSampling())) {
      std::get<1>(m_HashedRoadInfoCache) = HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(newHashedLaneInfo);
      std::get<0>(m_HashedRoadInfoCache) = newHashedLaneInfo;
    }
  }

 protected:
  std::tuple<HashedLaneInfo,
             Weak_HashedLaneInfoOrthogonalListPtr /*HashedLaneInfoOrthogonalListPtr*/ TX_MARK("change lane")>
      m_HashedRoadInfoCache;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("IEnvPerception");
  }
};
using IEnvPerceptionPtr = std::shared_ptr<IEnvPerception>;

class IEnvPerception_TAD_AI : public IEnvPerception {
 public:
  using txSurroundVehiclefo = Base::txSurroundVehiclefo;
  IEnvPerception_TAD_AI() { /*ConfigureFocusTypes();*/ }
  virtual ~IEnvPerception_TAD_AI() TX_DEFAULT;

  virtual void ConfigureFocusTypes() TX_NOEXCEPT = 0;
  virtual void ConfigurePerceptionPeriod(const Base::txInt _period) TX_NOEXCEPT { m_perception_period = _period; }
  virtual Base::txInt GetPerceptionPeriod() const TX_NOEXCEPT { return m_perception_period; }
  virtual void SearchSurroundingVehicle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;
  virtual Info_NearestObject SearchSurroundingVehicle_RTree(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;
  virtual void SearchSurroundingPedestrian(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;
  virtual void SearchSurroundingObstacle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;
  virtual void SearchNextCrossVehicle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;
#if USE_SUDOKU_GRID
  virtual void Search_Follow_Front(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;
  virtual void Clear_Follow_Info() TX_NOEXCEPT { Set_Follow_Info(nullptr, 9999.0); }
  void Set_Follow_Info(Base::SimulationConsistencyPtr ptr, Base::txFloat _dist) TX_NOEXCEPT {
    std::get<_ElementIdx_>(m_follow_weak_info) = ptr;
    std::get<_DistIdx_>(m_follow_weak_info) = _dist;
  }
  virtual txSurroundVehiclefo& DrivingFollow_Front() TX_NOEXCEPT { return m_follow_weak_info; }
  virtual Base::SimulationConsistencyPtr FollowFrontPtr() const TX_NOEXCEPT {
    return Weak2SharedPtr(std::get<_ElementIdx_>(m_follow_weak_info));
  }
  virtual Base::txFloat& FollowFrontDistance() TX_NOEXCEPT { return std::get<_DistIdx_>(m_follow_weak_info); }

  virtual void Get_Region_Clockwise_Close_By_Enum(const Base::Enums::NearestElementDirection eValue,
                                                  std::vector<Base::txVec2>& retRegion) const TX_NOEXCEPT = 0;
  virtual void Compute_Region_Clockwise_Close(const Coord::txENU& vehPos, const Base::txVec3& laneDir,
                                              const Unit::txDegree& headingDegree) TX_NOEXCEPT = 0;
  virtual std::array<Base::txVec3, Info_NearestObject::N_Dir> ComputeTargetPos(
      const Base::txVec3& vPos, const Base::txVec3& vehicleLaneDir) const TX_NOEXCEPT {
    std::array<Base::txVec3, Info_NearestObject::N_Dir> retArray;
    const std::array<Base::txVec3, Base::Enums::szVehicleLaneChangeType> vLeftRightDir = {
        Utils::VetRotVecByDegree(vehicleLaneDir, Unit::txDegree::MakeDegree(LeftOnENU), Utils::Axis_Enu_Up()),
        Utils::VetRotVecByDegree(vehicleLaneDir, Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up())};

    const std::array<Base::txVec3, Base::Enums::szVehicleLaneChangeType> vTargetTempPos = {
        vPos + vLeftRightDir[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eLeft))] * 3.5,
        vPos + vLeftRightDir[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eRight))] * 3.5};

    return std::array<Base::txVec3, Info_NearestObject::N_Dir>{
        vPos,
        vPos,
        vPos,
        vTargetTempPos[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eLeft))],
        vTargetTempPos[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eLeft))],
        vPos,
        vTargetTempPos[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eRight))],
        vTargetTempPos[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eRight))]};
  }
#endif /*USE_SUDOKU_GRID*/

#if __TX_Mark__("nearest object wrap")
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> FrontVeh_CurrentLane() const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RearVeh_CurrentLane() const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> FrontVeh_TargetLane() const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> FrontVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RearVeh_TargetLane() const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RearVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> Any_SideVeh_TargetLane() const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SideVeh_TargetLane() const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SideVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> LeftVeh_TargetLane() const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RightVeh_TargetLane() const TX_NOEXCEPT = 0;
#endif /*__TX_Mark__("nearest object wrap")*/

#if __TX_Mark__("nearest cross vehicle wrap")
  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> OpposeSideVeh_NextCross() const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> SameSideVeh_NextCross() const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> LeftSideVeh_NextCross() const TX_NOEXCEPT = 0;
  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> RightSideVeh_NextCross() const TX_NOEXCEPT = 0;
#endif /*__TX_Mark__("nearest cross vehicle wrap")*/

  Base::txBool CheckSamplingPeriod() TX_NOEXCEPT {
    m_sampling_counting =
        ((m_sampling_counting + 1) >= FLAGS_AiVehEnvPercptionPeriod) ? (0) : ((m_sampling_counting + 1));
    return ((GetPerceptionPeriod() == m_sampling_counting) ? (true) : (false));
  }

  virtual Base::txBool ValidSampling(const Base::TimeParamManager&) const TX_NOEXCEPT { return true; }
  virtual Base::txBool ForceSampling() const TX_NOEXCEPT TX_OVERRIDE { return m_force_sampling; }
  virtual void ClearForceSampling() TX_NOEXCEPT { m_force_sampling = false; }

 protected:
  txSurroundVehiclefo m_follow_weak_info;
  Base::txInt m_perception_period = 0;
  Base::txInt m_sampling_counting = 0;
  Base::txBool m_last_is_on_lanelink = true;
  Base::txBool m_last_lanekeep = true;
  Base::txBool m_force_sampling = false;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("IEnvPerception_TAD_AI");
    archive(_MAKE_NVP_("perception_period", m_perception_period));
    archive(_MAKE_NVP_("sampling_counting", m_sampling_counting));
    archive(_MAKE_NVP_("last_is_on_lanelink", m_last_is_on_lanelink));
    archive(_MAKE_NVP_("last_lanekeep", m_last_lanekeep));
  }
};

#if USE_HashedRoadNetwork
class IEnvPerception_TAD_AI_Hashed : public IEnvPerception_TAD_AI {
 public:
  using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
  using HashedLaneInfoPtr = Geometry::SpatialQuery::HashedLaneInfoPtr;
  using HashedLaneInfoOrthogonalList = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList;
  using HashedLaneInfoOrthogonalListPtr = Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr;
  using Weak_HashedLaneInfoOrthogonalListPtr = Geometry::SpatialQuery::Weak_HashedLaneInfoOrthogonalListPtr;
  using OrthogonalListSearchCmd = HashedLaneInfoOrthogonalList::OrthogonalListSearchCmd;

 public:
  IEnvPerception_TAD_AI_Hashed() TX_DEFAULT;
  virtual ~IEnvPerception_TAD_AI_Hashed() TX_DEFAULT;

 protected:
  virtual Info_NearestObject SearchSurroundingVehicle_HLane(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT = 0;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("IEnvPerception_TAD_AI_Hashed");
    archive(cereal::base_class<Base::IEnvPerception_TAD_AI>(this));
  }
};

#endif /*USE_HashedRoadNetwork*/

TX_NAMESPACE_CLOSE(Base)
#undef SerializeInfo

CEREAL_REGISTER_TYPE(Base::IEnvPerception_TAD_AI);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IEnvPerception, Base::IEnvPerception_TAD_AI);

CEREAL_REGISTER_TYPE(Base::IEnvPerception_TAD_AI_Hashed);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IEnvPerception_TAD_AI, Base::IEnvPerception_TAD_AI_Hashed);
