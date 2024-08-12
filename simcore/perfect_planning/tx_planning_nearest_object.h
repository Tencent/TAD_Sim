// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_planning_env_vehicle_info.h"
TX_NAMESPACE_OPEN(TrafficFlow)

#define _DistIdx_ (0)
#define _ElementIdx_ (1)

class Ego_NearestObject {
 public:
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txFloat = Base::txFloat;
  using txString = Base::txString;
  using txEgoEnvVehicleInfoPtr = TrafficFlow::txEgoEnvVehicleInfoPtr;
  using txEgoSurroundVehiclefo = TrafficFlow::txEgoSurroundVehiclefo;
  using SimulationConsistencyPtr = Base::SimulationConsistencyPtr;

 public:
  static constexpr txInt N_Dir = Base::Enums::szNearestElementDirection;
  static constexpr txInt nFront = (+Base::Enums::NearestElementDirection::eFront)._to_index();
  static constexpr txInt nBack = (+Base::Enums::NearestElementDirection::eBack)._to_index();
  static constexpr txInt nLeft = (+Base::Enums::NearestElementDirection::eLeft)._to_index();
  static constexpr txInt nLeftFront = (+Base::Enums::NearestElementDirection::eLeftFront)._to_index();
  static constexpr txInt nLeftBack = (+Base::Enums::NearestElementDirection::eLeftBack)._to_index();
  static constexpr txInt nRight = (+Base::Enums::NearestElementDirection::eRight)._to_index();
  static constexpr txInt nRightFront = (+Base::Enums::NearestElementDirection::eRightFront)._to_index();
  static constexpr txInt nRightBack = (+Base::Enums::NearestElementDirection::eRightBack)._to_index();

  static txEgoSurroundVehiclefo default_value() TX_NOEXCEPT {
    return std::make_tuple(FLAGS_SUDOKU_GRID_MAX_DISTANCE, nullptr);
  }

  Ego_NearestObject() { clear(); }

  void clear() TX_NOEXCEPT { around_dist_veh_ptr_array.fill(default_value()); }

  txFloat dist(const txInt j_dir) const TX_NOEXCEPT { return std::get<_DistIdx_>(around_dist_veh_ptr_array[j_dir]); }

  txFloat& dist(const txInt j_dir) TX_NOEXCEPT { return std::get<_DistIdx_>(around_dist_veh_ptr_array[j_dir]); }

  SimulationConsistencyPtr elemPtr(const txInt j_dir) const TX_NOEXCEPT {
    return std::get<_ElementIdx_>(around_dist_veh_ptr_array[j_dir]);
  }

  void setElemPtr(const txInt j_dir, SimulationConsistencyPtr ptr) TX_NOEXCEPT {
    std::get<_ElementIdx_>(around_dist_veh_ptr_array[j_dir]) = ptr;
  }

  txFloat SamplingTime() const TX_NOEXCEPT { return sampling_time; }
  txFloat& SamplingTime() TX_NOEXCEPT { return sampling_time; }

  txBool IsValid() const TX_NOEXCEPT { return is_valid; }
  void setValid(const txBool _f) TX_NOEXCEPT { is_valid = _f; }

  txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << TX_VARS(SamplingTime());
    for (txInt j_dir = 0; j_dir < N_Dir; ++j_dir) {
      oss << "{" << Base::Enums::NearestElementDirection::_from_index(j_dir)._to_string() << " : ";
      if (NonNull_Pointer(elemPtr(j_dir))) {
        oss << TX_VARS_NAME(otherVehId, (elemPtr(j_dir)->ConsistencyId())) << ", "
            << TX_VARS_NAME("otherVehId_dist", (dist(j_dir))) << "},";
      } else {
        oss << TX_VARS_NAME(otherVehId, "none") << ", " << TX_VARS_NAME("otherVehId_dist", (999)) << "},";
      }
    }
    return oss.str();
  }

  Ego_NearestObject& operator=(const Ego_NearestObject& other) TX_NOEXCEPT {
    sampling_time = other.sampling_time;
    for (size_t idx = 0; idx < N_Dir; ++idx) {
      around_dist_veh_ptr_array[idx] = other.around_dist_veh_ptr_array[idx];
    }
    return (*this);
  }

 protected:
  std::array<txEgoSurroundVehiclefo, N_Dir> around_dist_veh_ptr_array;
  txFloat sampling_time = -1.0;
  txBool is_valid = false;
};

class Ego_NearestDynamicObs {
 public:
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txFloat = Base::txFloat;
  using txString = Base::txString;
  static constexpr txInt N_Dir = Base::Enums::szPedestrianTargetDirectionType;
  Ego_NearestDynamicObs() { clear(); }

  txFloat dist(const txInt j_dir) const TX_NOEXCEPT {
    return std::get<_DistIdx_>(aroud_dynamicObs_dist_pb_array[j_dir]);
  }

  txFloat& dist(const txInt j_dir) TX_NOEXCEPT { return std::get<_DistIdx_>(aroud_dynamicObs_dist_pb_array[j_dir]); }

  txEgoDynamicObsInfo elemPtr(const txInt j_dir) const TX_NOEXCEPT {
    return std::get<_ElementIdx_>(aroud_dynamicObs_dist_pb_array[j_dir]);
  }

  txEgoDynamicObsInfo& elemPtr(const txInt j_dir) TX_NOEXCEPT {
    return std::get<_ElementIdx_>(aroud_dynamicObs_dist_pb_array[j_dir]);
  }

  void clear() TX_NOEXCEPT {
    aroud_dynamicObs_dist_pb_array.fill(std::make_tuple(FLAGS_SUDOKU_GRID_MAX_DISTANCE, txEgoDynamicObsInfo()));
  }

 protected:
  std::array<txEgoSurroundPedestrianInfo, N_Dir> aroud_dynamicObs_dist_pb_array;
};
TX_NAMESPACE_CLOSE(TrafficFlow)
