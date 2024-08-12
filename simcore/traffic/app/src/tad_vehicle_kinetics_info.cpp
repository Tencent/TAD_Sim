// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_vehicle_kinetics_info.h"
#include "tx_math.h"
TX_NAMESPACE_OPEN(TrafficFlow)

#if USE_VehicleKinectInfo

Base::txBool IsPtInKineticsSegment(const Base::txVec2& pt, const Base::txVec2& segHead,
                                   const Base::txVec2& segTrail) TX_NOEXCEPT {
  // 如果给定的点与线段的两个端点不共线，则返回true，表示点在线段上
  if (!Math::IsSynchronicity(pt - segHead, pt - segTrail)) {
    return true;
  } else {
    return false;
  }
}

// 重载输出运算符，用于输出IKineticsHandler::RelationShip枚举类型的值
std::ostream& operator<<(std::ostream& os, const IKineticsHandler::RelationShip v) TX_NOEXCEPT {
  switch (v) {
    case IKineticsHandler::RelationShip::syntropy_egohead_front_of_selfhead_base_on_self: {
      os << "syntropy_egohead_front_of_selfhead_base_on_self";
      break;
    }
    case IKineticsHandler::RelationShip::syntropy_egohead_behind_of_selfhead_base_on_self: {
      os << "syntropy_egohead_behind_of_selfhead_base_on_self";
      break;
    }
    case IKineticsHandler::RelationShip::reverse_egohead_front_of_selfhead_base_on_self: {
      os << "reverse_egohead_front_of_selfhead_base_on_self";
      break;
    }
    case IKineticsHandler::RelationShip::reverse_egohead_behind_of_selfhead_base_on_self: {
      os << "reverse_egohead_behind_of_selfhead_base_on_self";
      break;
    }
  }
  return os;
}

#endif /*USE_VehicleKinectInfo*/

TX_NAMESPACE_CLOSE(TrafficFlow)
