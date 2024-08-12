/*
 * HEVControllerPwr_types.h
 *
 * Code generation for model "HEVControllerPwr".
 *
 * Model version              : 1.1440
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Dec 27 10:10:55 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_HEVControllerPwr_types_h_
#define RTW_HEADER_HEVControllerPwr_types_h_
#include "rtwtypes.h"
#ifndef DEFINED_TYPEDEF_FOR_Ctrl_Input_
#  define DEFINED_TYPEDEF_FOR_Ctrl_Input_

struct Ctrl_Input {
  uint8_T acc_control_mode;
  uint8_T brake_control_mode;
  uint8_T steer_control_mode;
  real_T target_acc_m_s2;
  real_T request_torque;
  real_T brake_pressure_01;
  real_T steer_request_torque;
  real_T steer_cmd_rad;
  uint8_T gear_cmd__0N1D2R3P;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_powecu_in_
#  define DEFINED_TYPEDEF_FOR_powecu_in_

struct powecu_in {
  real_T acc_feedback_m_s2;
  real_T eng_spd_rad_s;
  real_T batt_soc_0_1;
  real_T diff_trq_req_Nm;
  real_T slip_ratio[4];
  real_T veh_speed_vx;
  Ctrl_Input driver_input;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_HEVMode_Input_
#  define DEFINED_TYPEDEF_FOR_HEVMode_Input_

struct HEVMode_Input {
  real_T SoC_MinVehicleDrive;
  real_T SoC_StartCharging;
  real_T SoC_EndCharging;
  real_T Speed_EngineStepIn;
  real_T PwrDmnd_EVOnly;
  real_T PwrDmnd_Hybrid;
  real_T SoC_Battery;
  real_T PwrDmnd;
  real_T Vehicle_Speed;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_HEVMode_Output_
#  define DEFINED_TYPEDEF_FOR_HEVMode_Output_

struct HEVMode_Output {
  real_T EV_Swtich;
  real_T Engine_Switch;
  real_T Engine_ChargingSwitch;
  real_T Engine_ClutchSwitch;
  uint32_T HEV_Mode;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_HEVPwrDmnd_Output_
#  define DEFINED_TYPEDEF_FOR_HEVPwrDmnd_Output_

struct HEVPwrDmnd_Output {
  real_T EV_PwrDmnd;
  real_T Engine_TractionPwrDmnd;
  real_T Engine_SerializePwrDmnd;
  real_T Engine_ChargingPwrDmnd;
  real_T P4_Motor_Enabled;
  real_T P4_PwrDmnd;
  real_T Engine_Request_Spd_RPM;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_HEV_ControllerInput_
#  define DEFINED_TYPEDEF_FOR_HEV_ControllerInput_

struct HEV_ControllerInput {
  real_T Throttle_01;
  real_T BrakePressure_01;
  real_T BattSoc_0_100;
  real_T BodyVx_ms;
  real_T MotSpeed_rads;
  real_T P4_MotSpeed_rads;
  real_T EngSpeed_rads;
  real_T gear_cmd_0N1D2R3P;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_powecu_out_
#  define DEFINED_TYPEDEF_FOR_powecu_out_

struct powecu_out {
  real_T pt_trq_cmd_Nm;
  real_T brake_pressure_0_1;
  real_T brake_pressure_Pa[4];
  real_T abs_flag[4];
  real_T throttle_01;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_HEV_ControllerOutput_
#  define DEFINED_TYPEDEF_FOR_HEV_ControllerOutput_

struct HEV_ControllerOutput {
  HEVMode_Output hev_mode_output;
  HEVPwrDmnd_Output HEV_PwrDmnd_Output;
  real_T MotRequestTorque_Nm;
  real_T P4_MotRequestTorque_Nm;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_struct_A6Yb21eKNPgaupJlKafAPB_
#  define DEFINED_TYPEDEF_FOR_struct_A6Yb21eKNPgaupJlKafAPB_

struct struct_A6Yb21eKNPgaupJlKafAPB {
  real_T FR;
  real_T FL;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_struct_NtCe3ohFvXuRZXQoDidoXH_
#  define DEFINED_TYPEDEF_FOR_struct_NtCe3ohFvXuRZXQoDidoXH_

struct struct_NtCe3ohFvXuRZXQoDidoXH {
  real_T RR;
  real_T RL;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_struct_WPo6c0btEjgdkiBqVilJ2B_
#  define DEFINED_TYPEDEF_FOR_struct_WPo6c0btEjgdkiBqVilJ2B_

struct struct_WPo6c0btEjgdkiBqVilJ2B {
  real_T Mass;
  struct_A6Yb21eKNPgaupJlKafAPB StaticNormalFrontLoad;
  struct_NtCe3ohFvXuRZXQoDidoXH StaticNormalRearLoad;
  real_T WheelBase;
  real_T FrontAxlePositionfromCG;
  real_T RearAxlePositionfromCG;
  real_T HeightCG;
  real_T FrontalArea;
  real_T DragCoefficient;
  real_T NumberOfWheelsPerAxle;
  real_T PitchMomentInertia;
  real_T RollMomentInertia;
  real_T YawMomentInertia;
  real_T SteeringRatio;
  real_T SprungMass;
  real_T InitialLongPosition;
  real_T InitialLatPosition;
  real_T InitialVertPosition;
  real_T InitialRollAngle;
  real_T InitialPitchAngle;
  real_T InitialYawAngle;
  real_T InitialLongVel;
  real_T InitialLatVel;
  real_T InitialVertVel;
  real_T InitialRollRate;
  real_T InitialPitchRate;
  real_T InitialYawRate;
  real_T UnsprungMassRearAxle;
  real_T UnsprungMassFrontAxle;
  real_T TrackWidthFront;
  real_T TrackWidthRear;
};

#endif

/* Parameters for system: '<S23>/NoPwrMode' */
typedef struct P_NoPwrMode_HEVControllerPwr_T_ P_NoPwrMode_HEVControllerPwr_T;

/* Parameters for system: '<S22>/EV_Pwr_Calculation' */
typedef struct P_EV_Pwr_Calculation_HEVControllerPwr_T_ P_EV_Pwr_Calculation_HEVControllerPwr_T;

/* Parameters for system: '<S22>/Engine_Pwr_Calculation' */
typedef struct P_Engine_Pwr_Calculation_HEVControllerPwr_T_ P_Engine_Pwr_Calculation_HEVControllerPwr_T;

/* Parameters for system: '<S22>/HEV_Pwr_Calculation' */
typedef struct P_HEV_Pwr_Calculation_HEVControllerPwr_T_ P_HEV_Pwr_Calculation_HEVControllerPwr_T;

/* Parameters for system: '<S22>/ReGen_Pwr_Calculation' */
typedef struct P_ReGen_Pwr_Calculation_HEVControllerPwr_T_ P_ReGen_Pwr_Calculation_HEVControllerPwr_T;

/* Parameters for system: '<S4>/ax_estimator' */
typedef struct P_ax_estimator_HEVControllerPwr_T_ P_ax_estimator_HEVControllerPwr_T;

/* Parameters (default storage) */
typedef struct P_HEVControllerPwr_T_ P_HEVControllerPwr_T;

/* Forward declaration for rtModel */
typedef struct tag_RTM_HEVControllerPwr_T RT_MODEL_HEVControllerPwr_T;

#endif /* RTW_HEADER_HEVControllerPwr_types_h_ */
