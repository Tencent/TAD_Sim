// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_traffic_element_base.h"
TX_NAMESPACE_OPEN(Base)

using txSurroundVehiclefo = std::tuple<Base::txFloat, Base::WEAK_SimulationConsistencyPtr>;

#define _DistIdx_ (0)
#define _ElementIdx_ (1)
struct Info_NearestObject {
  static constexpr txInt N_Dir = Base::Enums::szNearestElementDirection;
#if 0
  BETTER_ENUM(NearestElementDirection, txInt, eFront = 0, eBack = 1, eLeft = 2, eLeftFront = 3, eLeftBack = 4,
              eRight = 5, eRightFront = 6, eRightBack = 7)
#endif
  static constexpr txInt nFront = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eFront));
  static constexpr txInt nBack = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eBack));
  static constexpr txInt nLeft = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeft));
  static constexpr txInt nLeftFront = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeftFront));
  static constexpr txInt nLeftBack = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eLeftBack));
  static constexpr txInt nRight = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRight));
  static constexpr txInt nRightFront = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRightFront));
  static constexpr txInt nRightBack = __enum2idx__(_plus_(Base::Enums::NearestElementDirection::eRightBack));

  static std::tuple<Base::txFloat, Base::SimulationConsistencyPtr> default_value() TX_NOEXCEPT {
    return std::make_tuple(FLAGS_SUDOKU_GRID_MAX_DISTANCE, nullptr);
  }

  Info_NearestObject() { clear(); }

  txFloat dist(const txInt j_dir) const TX_NOEXCEPT { return std::get<_DistIdx_>(around_dist_veh_ptr_array[j_dir]); }

  txFloat& dist(const txInt j_dir) TX_NOEXCEPT { return std::get<_DistIdx_>(around_dist_veh_ptr_array[j_dir]); }

  Base::SimulationConsistencyPtr elemPtr(const txInt j_dir) const TX_NOEXCEPT {
    return Weak2SharedPtr(std::get<_ElementIdx_>(around_dist_veh_ptr_array[j_dir]));
  }

  void setElemPtr(const txInt j_dir, Base::SimulationConsistencyPtr ptr) TX_NOEXCEPT {
    std::get<_ElementIdx_>(around_dist_veh_ptr_array[j_dir]) = ptr;
  }

  void clear() TX_NOEXCEPT {
    txSurroundVehiclefo tmp;
    std::get<0>(tmp) = FLAGS_SUDOKU_GRID_MAX_DISTANCE;
    around_dist_veh_ptr_array.fill(tmp);
  }

  txBool Check() const TX_NOEXCEPT {
    for (txInt i = 0; i < N_Dir; ++i) {
      if (CallFail(Math::isEqual(dist(i), FLAGS_SUDOKU_GRID_MAX_DISTANCE))) {
        TX_MARK("has obj");
        if (Null_Pointer(elemPtr(i))) {
          return false;
        }
      } else {
        TX_MARK("null obj");
        if (NonNull_Pointer(elemPtr(i))) {
          return false;
        }
      }
    }
    return true;
  }

  txBool CheckEqual(const Info_NearestObject& o) const TX_NOEXCEPT {
    if (CallFail(Check())) {
      return false;
    }
    if (CallFail(o.Check())) {
      return false;
    }
    for (txInt i = 0; i < N_Dir; ++i) {
      if (CallSucc(Math::isEqual(dist(i), o.dist(i)))) {
        if (CallFail(Math::isEqual(dist(i), FLAGS_SUDOKU_GRID_MAX_DISTANCE))) {
          if ((elemPtr(i)->ConsistencyId()) != (o.elemPtr(i)->ConsistencyId())) {
            return false;
          }
        }
      } else {
        return false;
      }
    }
    return true;
  }

  txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << TX_VARS(SamplingTime());
    for (txInt j_dir = 0; j_dir < N_Dir; ++j_dir) {
      oss << "{" << __idx2enum__(Base::Enums::NearestElementDirection, j_dir) << " : ";
      if (NonNull_Pointer(elemPtr(j_dir))) {
        oss << TX_VARS_NAME(otherVehId, (elemPtr(j_dir)->ConsistencyId())) << ", "
            << TX_VARS_NAME("otherVehId_dist", (dist(j_dir))) << "},";
      } else {
        oss << TX_VARS_NAME(otherVehId, "none") << ", " << TX_VARS_NAME("otherVehId_dist", (999)) << "},";
      }
    }
    return oss.str();
  }

  Info_NearestObject& operator=(const Info_NearestObject& other) TX_NOEXCEPT {
    sampling_time = other.sampling_time;
    for (size_t idx = 0; idx < N_Dir; ++idx) {
      around_dist_veh_ptr_array[idx] = other.around_dist_veh_ptr_array[idx];
    }
    return (*this);
  }

  txFloat SamplingTime() const TX_NOEXCEPT { return sampling_time; }

  txFloat& SamplingTime() TX_NOEXCEPT { return sampling_time; }

  txBool IsValid() const TX_NOEXCEPT { return is_valid; }

  void setValid(const txBool _f) TX_NOEXCEPT { is_valid = _f; }

  txSurroundVehiclefo info_by_dir(const Base::Enums::NearestElementDirection& nDir) const {
    return around_dist_veh_ptr_array[__enum2int__(Base::Enums::NearestElementDirection, nDir)];
  }

  txSurroundVehiclefo info_by_dirIdx(const txInt j_dir) const { return around_dist_veh_ptr_array[j_dir]; }

 protected:
  std::array<txSurroundVehiclefo, N_Dir> around_dist_veh_ptr_array;
  Base::txFloat sampling_time = -1.0;
  Base::txBool is_valid = false;
};

struct Info_NearestDynamic {
  static constexpr txInt N_Dir = Base::Enums::szPedestrianTargetDirectionType;
  Info_NearestDynamic() { clear(); }

  txFloat dist(const txInt j_dir) const TX_NOEXCEPT { return aroudDistance[j_dir]; }

  txFloat& dist(const txInt j_dir) TX_NOEXCEPT { return aroudDistance[j_dir]; }

  Base::ITrafficElementPtr elemPtr(const txInt j_dir) const TX_NOEXCEPT { return aroudElemPtr[j_dir]; }

  void setElemPtr(const txInt j_dir, Base::ITrafficElementPtr ptr) TX_NOEXCEPT { aroudElemPtr[j_dir] = ptr; }

  void clear() TX_NOEXCEPT {
    aroudDistance.fill(FLAGS_SUDOKU_GRID_MAX_DISTANCE);
    aroudElemPtr.fill(nullptr);
  }

  txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    for (txInt j_dir = 0; j_dir < N_Dir; ++j_dir) {
      oss << "{" << __idx2enum__(Base::Enums::PedestrianTargetDirectionType, j_dir) << " : ";
      if (NonNull_Pointer(elemPtr(j_dir))) {
        oss << TX_VARS_NAME(otherVehId, (elemPtr(j_dir)->ConsistencyId())) << ", "
            << TX_VARS_NAME("otherVehId_dist", (dist(j_dir))) << "},";
      } else {
        oss << TX_VARS_NAME(otherVehId, "none") << ", " << TX_VARS_NAME("otherVehId_dist", (999)) << "},";
      }
    }
    return oss.str();
  }

  std::array<Base::ITrafficElementPtr, N_Dir> aroudElemPtr;
  std::array<Base::txFloat, N_Dir> aroudDistance;
};

TX_NAMESPACE_CLOSE(Base)
