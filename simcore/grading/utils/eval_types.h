// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "eigen3/Eigen/Dense"
#include "eval_log.h"

#define INVALID_MAP_ID 0xFFFFFFFFFFFFFFFF
#define LANE_INVALID_ID 0x0

namespace eval {
// shared ptr
template <class T>
using CSharedPtr = std::shared_ptr<T>;

// point coordinate type
enum CoordType { Coord_ENU, Coord_WGS84 };

// shape type
enum ShapeType { Shape_Base, Shape_RECT, Shape_Line };

// actor type
enum ActorType {
  Actor_Base,
  Actor_Static,
  Actor_Dynamic,
  Actor_Vehicle,
  Actor_Ego_Front,
  Actor_Ego_Trailer,
  Actor_SignalLight,
  Actor_Environment,
  Actor_Dynamic_FLU,
  Actor_Vehicle_FLU
};

enum DynamicActorType { Pedestrian = 0, Animal = 1, Bike = 2, Motor = 3, PortCrane = 4, DynamicErrortype = 10000 };

// traffic light state
enum SignalLightColor { GREEN = 0, YELLOW = 1, RED = 2 };

// traffic light
enum SignalLightPhase { U = 0, L = 1, T = 2, R = 3 };

// valid stat
enum ActorState { Actor_Invalid = 0, Actor_Valid = 1 };

// state
enum OnOff { OFF = 0, ON = 1 };

// actor wheels with stopline (local side & opposite side)
enum StateWhithStopLine {
  EgoFarFromStopLine = 0,
  EgoFrontOfLocalSideStopLine = 1,
  EgoOnLocalSideStopLine = 2,
  EgoInJunction = 3,
  EgoOnOppositeSideStopLine = 4,
  EgoBehindOppositeSideStopLine = 5
};

// eval parameters that do not change once initialized
struct EvalParams {
  std::string m_ego_groupname;            // ego group name, such as "Ego_001"
  std::string m_cloud_dir;                // cloud tadsim work dir
  std::vector<std::string> m_sub_topics;  // sub topics
  std::string m_log_folder;               // report dir, it is weird for some legacy reason (.pblog actually)
  bool m_sync_save_report;                // if sync save report
  bool m_is_simcity;                      // if is simcity mode(disable data log and report save in simcity mode)
  std::string external_lib_file;          // external eval algorithm lib file
};

// vehicle feedback info
struct VehicleBodyControl {
 public:
  eval::OnOff m_turn_left_lamp;
  eval::OnOff m_turn_right_lamp;
  eval::OnOff m_harzard;
  eval::OnOff m_position_lamp;
  eval::OnOff m_low_beam;
  eval::OnOff m_high_beam;
  eval::OnOff m_horn;

  VehicleBodyControl() { memset(this, 0, sizeof(VehicleBodyControl)); }
};

// vehicle state info
struct VehicleState {
  // gear mode
  enum GearMode { NO_CONTROL = 0, PARK = 1, REVERSE = 2, NEUTRAL = 3, DRIVE = 4 };

  struct Wheel4 {
    double frontLeft;
    double frontRight;
    double rearLeft;
    double rearRight;
  };

  struct Door4 {
    int32_t frontLeft;
    int32_t frontRight;
    int32_t rearLeft;
    int32_t rearRight;
  };

  struct BMSState {
    float bat_temperature;    // battery temperature, unit deg
    float bat_soc;            // battery soc, [0,1]
    float bat_pwr;            // battery power, unit kwh
    float bat_voltage;        // battery voltage, unit Voltage
    int32_t bat_charge_time;  // battery charging time, uint s
    // repeated uint32 chaging_time_seq = 6;  // battery charging time sequence
    bool bat_charging;  // battery is charging
  };

  struct PowerTrainState {
    GearMode gead_mode;         // gear lever, PRND, 1,2,3,4
    float total_mileage;        // vehicle total mileage, unit km
    double engine_speed;        // engine speed, unit rpm
    double accpedal_position;   // throttle position, unit %
    int32_t gear_engaged;       // actual gear of gear box, -1~6
    float trip_A;               // driving distance in this drive cycle trip A, unit km
    BMSState bms_state;         // battery management system state
    float speed_kph;            // vehicle speed, unit km/h
    float thermal_energy;       // thermal management energy power ratio, kwh
    double actualDriveTroque;   // actual total drive torque, unit Nm
    double motorRequestTorque;  // motor request torque
    double motorActualTorque;   // motor actual torque
    double energySourceLevel;   // energy source level, eg. gasline or SoC, unit %
    double frontMotorSpeed;     // front motor speed, unit rad/s
    double rearMotorSpeed;      // rear motor speed, unit rad/s
    double omegaGearSpeed;      // transmission out or motor out rotation speed, unit rad/s
  };

  struct BodyState {
    int32_t SAS_Status;        // airbag status
    bool LeftTurnSigLampSts;   // left turn lamp status
    bool RightTurnSigLampSts;  // right turn lamp status
    bool BrakePedSts;          // brake pedal status
    Door4 DoorStatus;          // door status
    double ACAmbtTemp;         // indoor temperature, deg
    float cabin_temperature;   // cabin temperature, unit deg
    double sinSlope;           // sin value of current slope, vehicle head above tail means positive sin value
  };

  struct ChassisState {
    Wheel4 tire_pressure;
    double SteeringWheelAngle;          // steering angle, unit deg
    double SteeringWheelAngleSign;      // steering angle sign
    double ESP_MasterCylindBrakePress;  // brake cylinder pressure, unit MPa
    Wheel4 wheel_speed;                 // wheel speed, m/s
    double VehDynYawRate;               // yaw rate，unit rad/s
    double BrakePedalPos;               // brake pedal postion, %
    bool ESPFailSts;                    // EPS failure status
    bool EspFuncOffSts;                 // ESP on/off status
    double SteeringWheelSpd;            // steering wheel angle speed, unit rad/s
    double SteeringWheelTorque;         // steering wheel torque, unit Nm
    double vehCurvature;                // vehicle curvature, unit 1/m
    Wheel4 wheelRotSpd;                 // wheel rotation speed, unit rad/s
  };

 public:
  double time_stamp;  // unit s
  ChassisState chassis_state;
  PowerTrainState powertrain_state;
  BodyState body_state;
  VehicleState() { memset(this, 0, sizeof(VehicleState)); }
};

// vehicle state info
// struct VehicleControl_V2 {
// enum GEAR_MODE{
//    NO_CONTROL = 0;
//    PARK = 1;
//    REVERSE = 2;
//    NEUTRAL = 3;
//    DRIVE = 4;
//  };
//
//  /* control type, longitudinal and lateral decoupled */
//  struct CONTROL_TYPE{
//    enum ACCEL_CONTROL_TYPE {
//      TARGET_ACC_CONTROL = 0;             // target acc control
//      REQUEST_TORQUE_CONTROL = 1;         // request torque control
//      REQUEST_DISTANCE_AND_VELOCITY = 2;  // request distance to stop and max allowed velocity
//      REQUEST_THROTTLE = 3;               // request throttle control
//      ACCEL_NO_CONTROL = 7;               // no control
//    };
//
//    enum BRAKE_CONTROL_TYPE {
//      BRAKE_TARGET_ACC_CONTROL = 0;       // target acc control
//      REQUEST_BRAKE_PRESSURE_CONTROL = 1; // request brake pressure control
//      BRAKE_NO_CONTROL = 7;               // no control
//    };
//
//    enum STEER_CONTROL_TYPE {
//      TARGET_STEER_ANGLE = 0;             // target steering wheel angle control
//      REQUEST_STEER_TORQUE_CONTROL = 1;   // request steer torque control
//      REQUEST_FRONT_WHEEL_ANGLE = 2;      // request front wheel angle
//      STEER_NO_CONTROL = 7;               // no control
//    };
//
//    ACCEL_CONTROL_TYPE acc_control_type ;    // acc control type
//    BRAKE_CONTROL_TYPE brake_control_type ;  // brake control type
//    STEER_CONTROL_TYPE steer_control_type ;  // steer control type
//  };
//
//  /*
//  * control cmd, vehicle dynamics module should use
//  * various combinations of elements in this message
//  * according to message CONTROL_TYPE
//  */
//  struct ControlCmd{
//    double request_acc;                 // longitudinal target acc, used for acc or brake, unit m/s2
//    double request_torque;              // longitudinal request torque, used for acc, unit Nm
//    double request_brake_pressure;      // longitudinal request main cylinder brake pressure, used for brake, unit MPa
//    double request_steer_wheel_angle;   // lateral request steer wheel angle, unit deg
//    double request_steer_torque;        // lateral request steer torque, unit Nm
//    double request_distance_2_stop;     // longitudinal request distance to stop, used for
//    REQUEST_DISTANCE_AND_VELOCITY, unit m double max_velocity;                // longitudinal request max velocity,
//    used for REQUEST_DISTANCE_AND_VELOCITY, unit km/h double request_throttle;            // longitudinal request
//    throttle, [0,1] double request_front_wheel_angle;   // lateral request front wheel angle, unit rad
//  };
//
//  /* on/off state */
//  enum OnOff{
//    OFF = 0;
//    ON = 1;
//  };
//
//  /* body control command */
//  struct BodyCommand{
//    OnOff horn;               //horn
//    OnOff left_turn;          //turn left
//    OnOff right_turn;         //turn right
//    OnOff front_fog_lamp;     //front fog lamp
//    OnOff rear_fog_lamp;      //rear fog lamp
//    OnOff position_lamp;      //position lamp
//    OnOff low_beam;           //low beam
//    OnOff high_beam;         //high beam
//    OnOff hazard_light;       //hazard light
//  };
//
//  /* chassis command */
//  struct ChassisCommand{
//
//  };
//
//  /* powertrain command */
//  struct PowerTrainCommand{
//
//  };
//
//  /* control flags, adapted for custom defined flags */
//  struct ControlFlags{
//    uint32 flag_1;    // flag 1, custom is response for the meaning of this flags
//    uint32 flag_2;    // flag 2, custom is response for the meaning of this flags
//    uint32 flag_3;    // flag 3, custom is response for the meaning of this flags
//    uint32 flag_4;    // flag 4, custom is response for the meaning of this flags
//    uint32 flag_5;    // flag 5, custom is response for the meaning of this flags
//    uint32 flag_6;    // flag 6, custom is response for the meaning of this flags
//    uint32 flag_7;    // flag 7, custom is response for the meaning of this flags
//    uint32 flag_8;    // flag 8, custom is response for the meaning of this flags
//  };
//
//  struct Header
//  {
//      double time_stamp;
//      uint64 frame_id;
//      double sequence;
//  };
//
// public:
//  Header header=1;
//
//  CONTROL_TYPE control_type;        // control type, acc/torque, steer/torque
//
//  GEAR_MODE gear_cmd;               // PRND
//  ControlCmd control_cmd;           // control cmd
//
//  BodyCommand body_cmd;             // body control cmd
//  ChassisCommand chassis_cmd;       // chassis control cmd
//  PowerTrainCommand powertrain_cmd; // powertrain control cmd
//
//  ControlFlags control_flags;       // control flags, reserved for custom use
//
//  VehicleControl_V2() { memset(this, 0, sizeof(VehicleControl_V2)); }
//};

// polygon
typedef std::vector<Eigen::Vector3d> RectCorners;

namespace ego_size {
struct RectSize {
 public:
  double length;
  double width;
  double height;

  explicit RectSize(double l = 0.0, double w = 0.0, double h = 0.0) : length(l), width(w), height(h) {}
};
}  // namespace ego_size

/**
 * @brief Lane ID defined in eval. Include tx_road_id, tx_section_id, tx_lane_id, tx_lanelink_id, etc.
 */
struct LaneID {
 public:
  uint64_t tx_road_id;
  uint64_t tx_section_id;
  int64_t tx_lane_id;
  uint64_t tx_lanelink_id;

  LaneID(uint64_t road_id, uint64_t section_id, int64_t lane_id, uint64_t lanelink_id)
      : tx_road_id(road_id), tx_section_id(section_id), tx_lane_id(lane_id), tx_lanelink_id(lanelink_id) {}
  LaneID() : LaneID(INVALID_MAP_ID, INVALID_MAP_ID, LANE_INVALID_ID, INVALID_MAP_ID) {}
  inline void From(uint64_t road_id, uint64_t section_id, int64_t lane_id, uint64_t lanelink_id = INVALID_MAP_ID) {
    tx_road_id = road_id;
    tx_section_id = section_id;
    tx_lane_id = lane_id;
    tx_lanelink_id = lanelink_id;
  }
  inline bool IsLaneValid() const {
    return tx_road_id != INVALID_MAP_ID && tx_section_id != INVALID_MAP_ID && tx_lane_id != LANE_INVALID_ID;
  }
  inline bool IsLaneLinkValid() { return !(tx_lanelink_id == INVALID_MAP_ID); }
  inline bool Equal(const LaneID &lane_id) const {
    return tx_road_id == lane_id.tx_road_id && tx_section_id == lane_id.tx_section_id &&
           tx_lane_id == lane_id.tx_lane_id;
  }
  inline bool LaneLinkEqual(const LaneID &lane_id) { return tx_lanelink_id == lane_id.tx_lanelink_id; }
  inline void Clear() {
    tx_road_id = INVALID_MAP_ID;
    tx_section_id = INVALID_MAP_ID;
    tx_lane_id = LANE_INVALID_ID;
    tx_lanelink_id = INVALID_MAP_ID;
  }

 public:
  void DebugShow(const std::string &pre_fix) const {
    VLOG_2 << pre_fix << "tx_road_id:" << tx_road_id << ", tx_section_id:" << tx_section_id
           << ", tx_lane_id:" << tx_lane_id << "\n";
  }
};
}  // namespace eval
