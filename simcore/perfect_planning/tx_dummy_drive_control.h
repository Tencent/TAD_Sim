// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "control_v2.pb.h"
#include "traffic.pb.h"
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_sim_point.h"
#include "tx_sim_time.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class DummyDriverControl {
 public:
  virtual ~DummyDriverControl() TX_DEFAULT;

#if __Control_V2__
  virtual void InitControlV2Pb() TX_NOEXCEPT;
  virtual sim_msg::Control_V2 control_v2_pb(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT {
    return m_pb_control_v2;
  }
  virtual void UpdateControlV2_GearMode(const Base::Enums::ControlPathGear _gear) TX_NOEXCEPT;
  virtual void UpdateControlPb(Base::TimeParamManager const& timeMgr, const Base::txFloat _acc,
                               const Base::txFloat _front_wheel_angle) TX_NOEXCEPT = 0;
  virtual sim_msg::Traffic DebugTraffic() const TX_NOEXCEPT { return m_debug_object_traffic; }
  virtual void ClearControlPb() TX_NOEXCEPT;
  virtual void AddDebugObject(Base::TimeParamManager const& timeMgr, const Base::txSysId id,
                              const hadmap::txPoint& gps) TX_NOEXCEPT;
#else  /*__Control_V2__*/
  virtual void InitControlPb() TX_NOEXCEPT;
  virtual void UpdateControlPb(Base::TimeParamManager const& timeMgr, const Base::txFloat _acc,
                               const Base::txFloat _front_wheel_angle) TX_NOEXCEPT;
  virtual sim_msg::Control control_pb(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT { return m_pb_control; }
#endif /*__Control_V2__*/

 protected:
#if __Control_V2__
  sim_msg::Control_V2 m_pb_control_v2;
  Base::txSize m_frame_id = 0;
  sim_msg::Traffic m_debug_object_traffic;
#else  /*__Control_V2__*/
  sim_msg::Control m_pb_control;
#endif /*__Control_V2__*/
};

TX_NAMESPACE_CLOSE(TrafficFlow)
