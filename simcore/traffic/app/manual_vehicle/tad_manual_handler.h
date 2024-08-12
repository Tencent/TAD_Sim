// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "location.pb.h"
#include "traffic.pb.h"
#include "tx_component.h"
#include "tx_header.h"
#include "tx_serialization.h"
#include "tx_sim_point.h"
#include "tx_sim_time.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class IManualHandler {
 public:
  enum class ManualHandlerType : Base::txInt { mVehicle, mPedestrian, mUndef };
  virtual ~IManualHandler() TX_DEFAULT;
  virtual Base::txBool IsManualMode() const TX_NOEXCEPT { return sim_msg::ControlState::Manual == m_ManualMode; }
  virtual void ClearManualMode() TX_NOEXCEPT { m_ManualMode = sim_msg::ControlState::Autonomous; }
  virtual ManualHandlerType ManualElementType() const TX_NOEXCEPT = 0;
  virtual void UpdateManualLocation(Base::TimeParamManager const& timeMgr,
                                    const sim_msg::Location& _location) TX_NOEXCEPT = 0;
  virtual Base::txString ManualDesc() const TX_NOEXCEPT = 0;

 protected:
  sim_msg::ControlState m_ManualMode = sim_msg::ControlState::Autonomous;
  Base::txFloat m_rpy_yaw;
  Coord::txENU m_last_Pos;
  Base::txVec3 m_velocity3D;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    archive(m_ManualMode, m_rpy_yaw, m_last_Pos, m_velocity3D);
  }
};

using IManualHandlerPtr = std::shared_ptr<IManualHandler>;
TX_NAMESPACE_CLOSE(TrafficFlow)
