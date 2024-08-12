// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <sstream>
#include "hadmap.h"
#include "tx_algebra_defs.h"
#include "tx_enum_def.h"
#include "tx_marco.h"
#include "tx_type_def.h"
#ifdef ON_CLOUD
#  include <cereal/archives/binary.hpp>
#  include <cereal/archives/json.hpp>
#endif
#include <cereal/types/array.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/vector.hpp>
#include <boost/optional.hpp>
#include <cereal/types/base_class.hpp>

#define _MAKE_NVP_ cereal::make_nvp

namespace cereal {
template <class Archive>
void serialize(Archive& archive, hadmap::txPoint& m) {
  archive(_MAKE_NVP_("Lon", __Lon__(m)), _MAKE_NVP_("Lat", __Lat__(m)), _MAKE_NVP_("Alt", __Alt__(m)));
}

template <class Archive>
void serialize(Archive& archive, Base::txVec3& m) {
  archive(_MAKE_NVP_("x", (m.x())), _MAKE_NVP_("y", (m.y())), _MAKE_NVP_("z", (m.z())));
}

template <class Archive>
void serialize(Archive& archive, Base::txVec2& m) {
  archive(_MAKE_NVP_("x", (m.x())), _MAKE_NVP_("y", (m.y())));
}

template <class Archive>
void serialize(Archive& archive, Base::txVec2I& m) {
  archive(_MAKE_NVP_("x", (m.x())), _MAKE_NVP_("y", (m.y())));
}

template <class Archive>
void serialize(Archive& archive, Base::txMat2& m) {
  archive(_MAKE_NVP_("m00", (m.coeffRef(0, 0))), _MAKE_NVP_("m01", (m.coeffRef(0, 1))),
          _MAKE_NVP_("m10", (m.coeffRef(1, 0))), _MAKE_NVP_("m11", (m.coeffRef(1, 1))));
}

template <class Archive>
void serialize(Archive& archive, Base::txMat3& m) {
  archive(_MAKE_NVP_("m00", (m.coeffRef(0, 0))), _MAKE_NVP_("m01", (m.coeffRef(0, 1))),
          _MAKE_NVP_("m02", (m.coeffRef(0, 2))),

          _MAKE_NVP_("m10", (m.coeffRef(1, 0))), _MAKE_NVP_("m11", (m.coeffRef(1, 1))),
          _MAKE_NVP_("m12", (m.coeffRef(1, 2))),

          _MAKE_NVP_("m20", (m.coeffRef(2, 0))), _MAKE_NVP_("m21", (m.coeffRef(2, 1))),
          _MAKE_NVP_("m22", (m.coeffRef(2, 2))));
}

template <class Archive>
void serialize(Archive& archive, Base::txLaneUId& m) {
  archive(_MAKE_NVP_("roadId", (m.roadId)), _MAKE_NVP_("sectionId", (m.sectionId)), _MAKE_NVP_("laneId", (m.laneId)));
}
#if __TX_Mark__("boost::optional")
// ! Saving for boost::optional
template <class Archive, class Optioned>
inline void save(Archive& ar, ::boost::optional<Optioned> const& optional) {
  bool initFlag = static_cast<bool>(optional);
  if (initFlag) {
    ar(_MAKE_NVP_("initialized", true));
    ar(_MAKE_NVP_("value", optional.get()));
  } else {
    ar(_MAKE_NVP_("initialized", false));
  }
}

// ! Loading for boost::optional
template <class Archive, class Optioned>
inline void load(Archive& ar, ::boost::optional<Optioned>& optional) {
  bool initFlag;
  ar(_MAKE_NVP_("initialized", initFlag));
  if (initFlag) {
    Optioned val;
    ar(_MAKE_NVP_("value", val));
    optional = val;
  } else {
    optional = ::boost::none;  // this is all we need to do to reset the internal flag and value
  }
}
#endif /*__TX_Mark__("boost::optional")*/

#if !__TX_Mark__("Base::Enums::VEHICLE_TYPE")
template <class Archive>
void save(Archive& ar, Base::Enums::VEHICLE_TYPE const& vehicle_type) {
  ar(_MAKE_NVP_("vehicle_type_integral", vehicle_type._to_integral()));
}

// ! Loading for boost::optional
template <class Archive>
void load(Archive& ar, Base::Enums::VEHICLE_TYPE& vehicle_type) {
  Base::Enums::VEHICLE_TYPE::_integral integral;
  ar(_MAKE_NVP_("vehicle_type_integral", integral));
  vehicle_type._from_integral(integral);
}
#endif /*__TX_Mark__("Base::Enums::VEHICLE_TYPE")*/

#if !__TX_Mark__("Base::ITrafficElement::ElementType")
template <class Archive>
void save(Archive& ar, Base::Enums::ElementType const& element_type) {
  ar(_MAKE_NVP_("element_type_integral", element_type._to_integral()));
}

// ! Loading for boost::optional
template <class Archive>
void load(Archive& ar, Base::Enums::ElementType& element_type) {
  Base::Enums::ElementType::_integral integral;
  ar(_MAKE_NVP_("element_type_integral", integral));
  element_type._from_integral(integral);
}
#endif /*__TX_Mark__("Base::ITrafficElement::ElementType")*/

#if !__TX_Mark__("Base::ITrafficElement::VehicleMoveLaneState")
template <class Archive>
void save(Archive& ar, Base::Enums::VehicleMoveLaneState const& vehicleMoveLaneState) {
  ar(_MAKE_NVP_("vehicleMoveLaneState_integral", vehicleMoveLaneState._to_integral()));
}

// ! Loading for boost::optional
template <class Archive>
void load(Archive& ar, Base::Enums::VehicleMoveLaneState& vehicleMoveLaneState) {
  Base::Enums::VehicleMoveLaneState::_integral integral;
  ar(_MAKE_NVP_("vehicleMoveLaneState_integral", integral));
  vehicleMoveLaneState._from_integral(integral);
}
#endif /*__TX_Mark__("Base::ITrafficElement::VehicleMoveLaneState")*/

#if !__TX_Mark__("Base::Enums::TAD_VehicleState_AI")
template <class Archive>
void save(Archive& ar, Base::Enums::TAD_VehicleState_AI const& FSM_AI) {
  ar(_MAKE_NVP_("FSM_AI_integral", FSM_AI._to_integral()));
}

// ! Loading for boost::optional
template <class Archive>
void load(Archive& ar, Base::Enums::TAD_VehicleState_AI& FSM_AI) {
  Base::Enums::TAD_VehicleState_AI::_integral integral;
  ar(_MAKE_NVP_("FSM_AI_integral", integral));
  FSM_AI._from_integral(integral);
}
#endif /*__TX_Mark__("Base::Enums::TAD_VehicleState_AI")*/

}  // namespace cereal
