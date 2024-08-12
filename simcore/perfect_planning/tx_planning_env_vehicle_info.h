// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <tbb/tbb.h>
#include "tx_traffic_element_base.h"
// #include "tbb/concurrent_hash_map.h"
// #include "tbb/concurrent_unordered_set.h"
// #include "tbb/blocked_range.h"
// #include "tbb/parallel_for.h"
// #include "tx_hash_utils.h"
#include <map>
#include "traffic.pb.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class txEgoEnvVehicleInfo_Factory;

class txEgoEnvVehicleInfo : public Base::ISimulationConsistency {
 protected:
  txEgoEnvVehicleInfo() TX_DEFAULT;

 public:
  friend class txEgoEnvVehicleInfo_Factory;
  virtual ~txEgoEnvVehicleInfo() TX_DEFAULT;
  virtual Base::txSysId ConsistencyId() const TX_NOEXCEPT TX_OVERRIDE { return mConsistencyId; }
  virtual Coord::txENU StableGeomCenter() const TX_NOEXCEPT TX_OVERRIDE { return mStablePosition; }

  virtual Coord::txWGS84 StablePositionOnLane() const TX_NOEXCEPT TX_OVERRIDE { return mStablePositionOnLane; }
  virtual Base::Info_Lane_t StableLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return mStableLaneInfo; }
  virtual Base::txVec3 StableLaneDir() const TX_NOEXCEPT TX_OVERRIDE { return mStableLaneDir; }
  virtual Unit::txDegree StableHeading() const TX_NOEXCEPT TX_OVERRIDE { return mStableHeading; }
  virtual Base::txFloat StableVelocity() const TX_NOEXCEPT TX_OVERRIDE { return mStableVelocity; }
  virtual Base::txFloat StableAcc() const TX_NOEXCEPT TX_OVERRIDE { return mStableAcc; }
  virtual Base::IIdentity::ElementType ConsistencyElementType() const TX_NOEXCEPT TX_OVERRIDE {
    return +Base::IIdentity::ElementType::TAD_Vehicle;
  }
  virtual DrivingStatus StableDriving_Status() const TX_NOEXCEPT TX_OVERRIDE { return +DrivingStatus::normal; }
  virtual void SaveStableState() TX_NOEXCEPT TX_OVERRIDE {}
  virtual const Geometry::SpatialQuery::HashedLaneInfo& StableHashedLaneInfo() const TX_NOEXCEPT TX_OVERRIDE {
    return mStableHashedLaneInfo;
  }
  virtual Base::txBool Initialize(const sim_msg::Car& refCar) TX_NOEXCEPT;
  virtual Base::txFloat StableS() const TX_NOEXCEPT { return mST.x(); }
  virtual Base::txFloat StableT() const TX_NOEXCEPT { return mST.y(); }
  virtual Base::txFloat StableInvertS() const TX_NOEXCEPT { return mGeomLength - StableS(); }
  virtual Base::txFloat StableLength() const TX_NOEXCEPT { return mStableEgoGeomInfo.x(); }
  virtual Base::txFloat StableWidth() const TX_NOEXCEPT { return mStableEgoGeomInfo.y(); }
  virtual Base::txFloat StableHeight() const TX_NOEXCEPT { return mStableEgoGeomInfo.z(); }
  virtual Base::txFloat StableInvertDistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE { return StableS(); }
  virtual Base::txFloat StableDistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE { return StableInvertS(); }

 private:
  virtual Coord::txENU StableRearAxleCenter() const TX_NOEXCEPT TX_OVERRIDE { return StableGeomCenter(); }

 protected:
  Base::txSysId mConsistencyId;
  Coord::txENU mStablePosition;
  Coord::txWGS84 mStablePositionOnLane;
  Base::Info_Lane_t mStableLaneInfo;
  Base::txVec3 mStableLaneDir;
  Unit::txDegree mStableHeading;
  Base::txFloat mStableVelocity;
  Base::txFloat mStableAcc;
  Geometry::SpatialQuery::HashedLaneInfo mStableHashedLaneInfo;
  Base::txVec2 mST;
  Base::txFloat mGeomLength;
  Base::txVec3 mStableEgoGeomInfo;
};

using txEgoEnvVehicleInfoPtr = std::shared_ptr<txEgoEnvVehicleInfo>;
using WEAK_txEgoEnvVehicleInfo = std::weak_ptr<txEgoEnvVehicleInfo>;

class txEgoEnvObstacleInfo : public txEgoEnvVehicleInfo {
 public:
  virtual Base::txBool Initialize(const sim_msg::StaticObstacle& refObs) TX_NOEXCEPT;
};
using txEgoEnvObstacleInfoPtr = std::shared_ptr<txEgoEnvObstacleInfo>;
using WEAK_txEgoEnvObstacleInfo = std::weak_ptr<txEgoEnvObstacleInfo>;

using txEgoSurroundVehiclefo = std::tuple<Base::txFloat, Base::SimulationConsistencyPtr>;

class txEgoEnvVehicleInfo_Factory {
 public:
  using txSysId = Base::txSysId;
  static txEgoEnvVehicleInfoPtr GetEnvVehicleInfoPtr(const sim_msg::Car& refCar) TX_NOEXCEPT;
  static txEgoEnvObstacleInfoPtr GetEnvObstacleInfoPtr(const sim_msg::StaticObstacle& refObs) TX_NOEXCEPT;
  static void Clear() TX_NOEXCEPT {
    s_CarId2SurroundVehicleInfoPtr.clear();
    s_CarId2SurroundObstacleInfoPtr.clear();
  }

 protected:
  /*using carId2SurroundVehicleInfoPtr = tbb::concurrent_hash_map<txSysId, txEgoEnvVehicleInfoPtr,
   * Utils::txSysIdHashCompare>;*/
  static std::map<txSysId, txEgoEnvVehicleInfoPtr> s_CarId2SurroundVehicleInfoPtr;
  static std::map<txSysId, txEgoEnvObstacleInfoPtr> s_CarId2SurroundObstacleInfoPtr;
  TX_MARK("pedestrian id conflict with vehicle id");
  static tbb::mutex tbbMutex;
};

class txEgoSignalInfo {
 public:
  using SIGN_LIGHT_COLOR_TYPE = Base::Enums::SIGN_LIGHT_COLOR_TYPE;
  virtual SIGN_LIGHT_COLOR_TYPE GetCurrentSignLightColorType() const TX_NOEXCEPT { return m_color; }
  virtual Base::txSysId Id() const TX_NOEXCEPT { return mSignalId; }
  virtual Base::txBool Initialize(const Base::txSysId _signal_id, const SIGN_LIGHT_COLOR_TYPE _color) TX_NOEXCEPT {
    mSignalId = _signal_id;
    m_color = _color;
    return true;
  }
  virtual Base::txString Str() const TX_NOEXCEPT;

 protected:
  SIGN_LIGHT_COLOR_TYPE m_color;
  Base::txSysId mSignalId;
};
using txEgoSignalInfoPtr = std::shared_ptr<txEgoSignalInfo>;

class txEgoDynamicObsInfo {
 public:
  txEgoDynamicObsInfo() TX_DEFAULT;
  explicit txEgoDynamicObsInfo(const sim_msg::DynamicObstacle& _d) {
    mDynamicObs.CopyFrom(_d);
    mIsValid = true;
  }
  txEgoDynamicObsInfo& operator=(const txEgoDynamicObsInfo& other) TX_NOEXCEPT {
    mIsValid = other.mIsValid;
    mDynamicObs.CopyFrom(other.mDynamicObs);
    return (*this);
  }
  Base::txString Str() const TX_NOEXCEPT;
  Base::txSysId DynamicObsId() const TX_NOEXCEPT { return mDynamicObs.id(); }
  Coord::txWGS84 Position() const TX_NOEXCEPT {
    return Coord::txWGS84(hadmap::txPoint(mDynamicObs.x(), mDynamicObs.y(), mDynamicObs.z()));
  }
  Base::txFloat Length() const TX_NOEXCEPT { return mDynamicObs.length(); }
  Base::txFloat Width() const TX_NOEXCEPT { return mDynamicObs.width(); }
  Base::txFloat Height() const TX_NOEXCEPT { return mDynamicObs.height(); }
  Base::txBool IsValid() const TX_NOEXCEPT { return mIsValid; }

 protected:
  sim_msg::DynamicObstacle mDynamicObs;
  Base::txBool mIsValid = false;
};

using txEgoSurroundPedestrianInfo = std::tuple<Base::txFloat, txEgoDynamicObsInfo>;

TX_NAMESPACE_CLOSE(TrafficFlow)
