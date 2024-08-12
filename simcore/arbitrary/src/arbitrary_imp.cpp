#include "arbitrary_imp.h"

#include "control.pb.h"
#include "control_v2.pb.h"

#include "glog/logging.h"

namespace arbitrary {
CArbitraryImp::CArbitraryImp() { m_arbitrary_ctrl_payload.clear(); }
CArbitraryImp::~CArbitraryImp() {}
std::string CArbitraryImp::GetArbitrary_ControlPayload() const { return m_arbitrary_ctrl_payload; }
bool CArbitraryImp::DoArbitrary_Control(const std::string &ctrl_alg_payload, const std::string &ctrl_driver_payload) {
  // do clear
  m_arbitrary_ctrl_payload.clear();

  // condition if control_v2 payload is empty, either from driver or algorithm
  if (ctrl_alg_payload.size() == 0 || ctrl_driver_payload.size() == 0) {
    /**
     * @brief only driver payload is not empty, arbitrary payload comes from
     * driver
     * @param ctrl_alg_payload
     * @param ctrl_driver_payload
     * @return
     */
    if (ctrl_alg_payload.size() == 0 && ctrl_driver_payload.size() > 0) {
      LOG(WARNING) << "algorithm payload is empty. use driver payload.\n";
      m_arbitrary_ctrl_payload = ctrl_driver_payload;
      return true;
    }
    /**
     * @brief only algorithm payload is not empty, arbitrary payload comes from
     * algorithm
     * @param ctrl_alg_payload
     * @param ctrl_driver_payload
     * @return
     */
    else if (ctrl_driver_payload.size() == 0 && ctrl_alg_payload.size() > 0) {
      LOG(WARNING) << "driver payload is empty. use algorithm payload.\n";
      m_arbitrary_ctrl_payload = ctrl_alg_payload;
      return true;
    }
    /**
     * @brief both are empty, clear arbitrary payload
     * @param ctrl_alg_payload
     * @param ctrl_driver_payload
     * @return
     */
    else {
      LOG(WARNING) << "both driver and algorithm payload are empty, clear "
                      "output payload.\n";
      m_arbitrary_ctrl_payload.clear();
      return true;
    }
  } else {
    // condition both control_v2 payload are not empty
    sim_msg::Control_V2 ctrl_alg, ctrl_driver, ctrlOut;

    if (ctrl_alg.ParseFromString(ctrl_alg_payload) && ctrl_driver.ParseFromString(ctrl_driver_payload)) {
      const auto &algCtrlType = ctrl_alg.control_type();
      const auto &accAlgCtrlType = algCtrlType.acc_control_type();
      const auto &brakeAlgCtrlType = algCtrlType.brake_control_type();
      const auto &steerAlgCtrlType = algCtrlType.steer_control_type();

      // default ctrl output comes from driver
      ctrlOut.CopyFrom(ctrl_driver);

      // cmd comes from algorithm other than control_cmd
      ctrlOut.mutable_header()->CopyFrom(ctrl_alg.header());
      ctrlOut.mutable_body_cmd()->CopyFrom(ctrl_alg.body_cmd());
      ctrlOut.mutable_chassis_cmd()->CopyFrom(ctrl_alg.chassis_cmd());
      ctrlOut.mutable_control_flags()->CopyFrom(ctrl_alg.control_flags());
      ctrlOut.mutable_powertrain_cmd()->CopyFrom(ctrl_alg.powertrain_cmd());

      // when algorithm take over accel control
      if (accAlgCtrlType != sim_msg::Control_V2_CONTROL_TYPE_ACCEL_CONTROL_TYPE_ACCEL_NO_CONTROL) {
        ctrlOut.mutable_control_type()->set_acc_control_type(accAlgCtrlType);

        auto ctrlCmd = ctrlOut.mutable_control_cmd();

        // set gear cmd
        ctrlOut.set_gear_cmd(ctrl_alg.gear_cmd());

        switch (accAlgCtrlType) {
          case sim_msg::Control_V2_CONTROL_TYPE_ACCEL_CONTROL_TYPE_TARGET_ACC_CONTROL: {
            ctrlCmd->set_request_acc(ctrl_alg.control_cmd().request_acc());
            break;
          }
          case sim_msg::Control_V2_CONTROL_TYPE_ACCEL_CONTROL_TYPE_REQUEST_TORQUE_CONTROL: {
            ctrlCmd->set_request_torque(ctrl_alg.control_cmd().request_torque());
            break;
          }
          case sim_msg::Control_V2_CONTROL_TYPE_ACCEL_CONTROL_TYPE_REQUEST_DISTANCE_AND_VELOCITY: {
            ctrlCmd->set_request_distance_2_stop(ctrl_alg.control_cmd().request_distance_2_stop());
            ctrlCmd->set_max_velocity(ctrl_alg.control_cmd().max_velocity());
            break;
          }
          case sim_msg::Control_V2_CONTROL_TYPE_ACCEL_CONTROL_TYPE_REQUEST_THROTTLE: {
            ctrlCmd->set_request_throttle(ctrl_alg.control_cmd().request_throttle());
            break;
          }
          default: {
            LOG(ERROR) << "unknown accel control type from algorithm --> " << static_cast<int>(accAlgCtrlType) << "\n";
            break;
          }
        }
        VLOG(2) << "accel: algorithm take over with "
                << sim_msg::Control_V2_CONTROL_TYPE_ACCEL_CONTROL_TYPE_Name(accAlgCtrlType) << "\n";
      }

      // when algorithm take over brake control
      if (brakeAlgCtrlType != sim_msg::Control_V2_CONTROL_TYPE_BRAKE_CONTROL_TYPE_BRAKE_NO_CONTROL) {
        ctrlOut.mutable_control_type()->set_brake_control_type(brakeAlgCtrlType);

        auto ctrlCmd = ctrlOut.mutable_control_cmd();

        // set gear cmd
        ctrlOut.set_gear_cmd(ctrl_alg.gear_cmd());

        switch (brakeAlgCtrlType) {
          case sim_msg::Control_V2_CONTROL_TYPE_BRAKE_CONTROL_TYPE_BRAKE_TARGET_ACC_CONTROL: {
            ctrlCmd->set_request_acc(ctrl_alg.control_cmd().request_acc());
            break;
          }
          case sim_msg::Control_V2_CONTROL_TYPE_BRAKE_CONTROL_TYPE_REQUEST_BRAKE_PRESSURE_CONTROL: {
            ctrlCmd->set_request_brake_pressure(ctrl_alg.control_cmd().request_brake_pressure());
            break;
          }
          default: {
            LOG(ERROR) << "unknown brake control type from algorithm --> " << static_cast<int>(brakeAlgCtrlType)
                       << "\n";
            break;
          }
        }
        VLOG(2) << "brake: algorithm take over with "
                << sim_msg::Control_V2_CONTROL_TYPE_BRAKE_CONTROL_TYPE_Name(brakeAlgCtrlType) << "\n";
      }

      // when algorithm take over steer control
      if (steerAlgCtrlType != sim_msg::Control_V2_CONTROL_TYPE_STEER_CONTROL_TYPE_STEER_NO_CONTROL) {
        ctrlOut.mutable_control_type()->set_steer_control_type(steerAlgCtrlType);

        auto ctrlCmd = ctrlOut.mutable_control_cmd();

        switch (steerAlgCtrlType) {
          case sim_msg::Control_V2_CONTROL_TYPE_STEER_CONTROL_TYPE_TARGET_STEER_ANGLE: {
            ctrlCmd->set_request_steer_wheel_angle(ctrl_alg.control_cmd().request_steer_wheel_angle());
            break;
          }
          case sim_msg::Control_V2_CONTROL_TYPE_STEER_CONTROL_TYPE_REQUEST_STEER_TORQUE_CONTROL: {
            ctrlCmd->set_request_steer_torque(ctrl_alg.control_cmd().request_steer_torque());
            break;
          }
          case sim_msg::Control_V2_CONTROL_TYPE_STEER_CONTROL_TYPE_REQUEST_FRONT_WHEEL_ANGLE: {
            ctrlCmd->set_request_front_wheel_angle(ctrl_alg.control_cmd().request_front_wheel_angle());
            break;
          }
          default: {
            LOG(ERROR) << "unknown steer control type from algorithm --> " << static_cast<int>(steerAlgCtrlType)
                       << "\n";
            break;
          }
        }
        VLOG(2) << "steer: algorithm take over with "
                << sim_msg::Control_V2_CONTROL_TYPE_STEER_CONTROL_TYPE_Name(steerAlgCtrlType) << "\n";
      }

      // set output payload
      ctrlOut.SerializeToString(&m_arbitrary_ctrl_payload);
    } else {
      m_arbitrary_ctrl_payload.clear();
      LOG(ERROR) << "fail to parse control_v2.proto from driver or algorithm, "
                    "clear output payload.\n";
      return false;
    }

    /*
    sim_msg::Control ctrl_alg, ctrl_driver;

    if (ctrl_alg.ParseFromString(ctrl_alg_payload) &&
    ctrl_driver.ParseFromString(ctrl_driver_payload)) { if
    (ctrl_alg.control_mode() == sim_msg::Control_CONTROL_MODE_CM_AUTO_DRIVE) {
                    m_arbitrary_ctrl_payload = ctrl_alg_payload;
                    VLOG(2) << "control from algorithm is auto drive mode, use
    algorithm control command.\n";
            }
            else if (ctrl_alg.control_mode() ==
    sim_msg::Control_CONTROL_MODE_CM_ONLY_SPEED) { sim_msg::Control ctrl;
                    ctrl.CopyFrom(ctrl_alg);
                    ctrl.mutable_acc_cmd()->set_front_wheel_angle(ctrl_driver.acc_cmd().front_wheel_angle());
                    ctrl.SerializeToString(&m_arbitrary_ctrl_payload);
                    VLOG(2) << "control from algorithm is speed auto drive mode,
    use steer control command from driver.\n";
            }
            else if (ctrl_alg.control_mode() ==
    sim_msg::Control_CONTROL_MODE_CM_ONLY_STEER) { sim_msg::Control ctrl;
                    ctrl.CopyFrom(ctrl_alg);
                    ctrl.mutable_acc_cmd()->set_acc(ctrl_driver.acc_cmd().acc());
                    ctrl.SerializeToString(&m_arbitrary_ctrl_payload);
                    VLOG(2) << "control from algorithm is steer auto drive mode,
    use acc control command from driver.\n";
            }
            else {
                    VLOG(2) << "control from algorithm is not in auto mode, use
    control command from driver.\n"; m_arbitrary_ctrl_payload =
    ctrl_driver_payload;
            }
    }
    else {
            m_arbitrary_ctrl_payload.clear();
            LOG(ERROR) << "fail to parse control of driver or algorithm.\n";
            return false;
    }
    */
  }

  return true;
}
}  // namespace arbitrary
