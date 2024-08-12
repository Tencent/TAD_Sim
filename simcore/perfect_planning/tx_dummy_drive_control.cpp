// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_dummy_drive_control.h"

TX_NAMESPACE_OPEN(TrafficFlow)

#if __Control_V2__

void DummyDriverControl::ClearControlPb() TX_NOEXCEPT {
  // InitControlV2Pb();
  m_debug_object_traffic.Clear();
}

void DummyDriverControl::InitControlV2Pb() TX_NOEXCEPT {
  m_pb_control_v2.Clear();
  auto control_type_ptr = m_pb_control_v2.mutable_control_type();
  if (NonNull_Pointer(control_type_ptr)) {
    control_type_ptr->set_acc_control_type(sim_msg::Control_V2::CONTROL_TYPE::ACCEL_CONTROL_TYPE::
                                               Control_V2_CONTROL_TYPE_ACCEL_CONTROL_TYPE_TARGET_ACC_CONTROL);
    control_type_ptr->set_brake_control_type(sim_msg::Control_V2::CONTROL_TYPE::BRAKE_CONTROL_TYPE::
                                                 Control_V2_CONTROL_TYPE_BRAKE_CONTROL_TYPE_BRAKE_TARGET_ACC_CONTROL);
    /*control_type_ptr->set_steer_control_type(sim_msg::Control_V2::CONTROL_TYPE::
    STEER_CONTROL_TYPE::Control_V2_CONTROL_TYPE_STEER_CONTROL_TYPE_REQUEST_FRONT_WHEEL_ANGLE);*/
    control_type_ptr->set_steer_control_type(sim_msg::Control_V2::CONTROL_TYPE::STEER_CONTROL_TYPE::
                                                 Control_V2_CONTROL_TYPE_STEER_CONTROL_TYPE_TARGET_STEER_ANGLE);
  }
  m_pb_control_v2.set_gear_cmd(sim_msg::Control_V2::GEAR_MODE::Control_V2_GEAR_MODE_DRIVE);
}

void DummyDriverControl::UpdateControlV2_GearMode(const Base::Enums::ControlPathGear _gear) TX_NOEXCEPT {
  if (_plus_(Base::Enums::ControlPathGear::reverse) == _gear) {
    m_pb_control_v2.set_gear_cmd(sim_msg::Control_V2::GEAR_MODE::Control_V2_GEAR_MODE_REVERSE);
  }

  if (_plus_(Base::Enums::ControlPathGear::drive) == _gear) {
    m_pb_control_v2.set_gear_cmd(sim_msg::Control_V2::GEAR_MODE::Control_V2_GEAR_MODE_DRIVE);
  }

  m_pb_control_v2.set_gear_cmd(sim_msg::Control_V2::GEAR_MODE::Control_V2_GEAR_MODE_NO_CONTROL);
}

void DummyDriverControl::AddDebugObject(Base::TimeParamManager const& timeMgr, const Base::txSysId id,
                                        const hadmap::txPoint& gps) TX_NOEXCEPT {
  auto tmpObstacleData = m_debug_object_traffic.add_staticobstacles();
  if (NonNull_Pointer(tmpObstacleData)) {
    tmpObstacleData->set_t(timeMgr.TimeStamp());
    tmpObstacleData->set_id(id);
    tmpObstacleData->set_type(
        __enum2int__(Base::Enums::STATIC_ELEMENT_TYPE, _plus_(Base::Enums::STATIC_ELEMENT_TYPE::Box)));

    tmpObstacleData->set_x(__Lon__(gps));
    tmpObstacleData->set_y(__Lat__(gps));
    tmpObstacleData->set_z(0.0 /*__Alt__(geom_center_gps)*/);
    tmpObstacleData->set_length(1.0);
    tmpObstacleData->set_width(1.0);
    tmpObstacleData->set_height(1.0);
    tmpObstacleData->set_heading(0.0);
  }
}

#else  /*__Control_V2__*/
void DummyDriverControl::InitControlPb() TX_NOEXCEPT {
  m_pb_control.set_gear_cmd(sim_msg::Control_GEAR_MODE_DRIVE);
  m_pb_control.set_control_mode(sim_msg::Control_CONTROL_MODE_CM_AUTO_DRIVE);
  m_pb_control.set_contrl_type(sim_msg::Control_CONTROL_TYPE_ACC_CONTROL);
}

void DummyDriverControl::UpdateControlPb(Base::TimeParamManager const& timeMgr, const Base::txFloat _acc,
                                         const Base::txFloat _front_wheel_angle) TX_NOEXCEPT {
  m_pb_control.mutable_acc_cmd()->set_acc(_acc);
  m_pb_control.mutable_acc_cmd()->set_front_wheel_angle(_front_wheel_angle);
}
#endif /*__Control_V2__*/

TX_NAMESPACE_CLOSE(TrafficFlow)
