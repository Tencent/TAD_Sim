/*
 * DynamicSteer_types.h
 *
 * Code generation for model "DynamicSteer".
 *
 * Model version              : 2.38
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Sat Jun 24 17:12:04 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_DynamicSteer_types_h_
#define RTW_HEADER_DynamicSteer_types_h_
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

#ifndef DEFINED_TYPEDEF_FOR_InertFrm_Disp_
#  define DEFINED_TYPEDEF_FOR_InertFrm_Disp_

struct InertFrm_Disp {
  real_T X;
  real_T Y;
  real_T Z;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_InertFrm_Vel_
#  define DEFINED_TYPEDEF_FOR_InertFrm_Vel_

struct InertFrm_Vel {
  real_T Xdot;
  real_T Ydot;
  real_T Zdot;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_InertFrm_Angle_
#  define DEFINED_TYPEDEF_FOR_InertFrm_Angle_

struct InertFrm_Angle {
  real_T phi;
  real_T theta;
  real_T psi;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_InertFrm_DispVelAng_
#  define DEFINED_TYPEDEF_FOR_InertFrm_DispVelAng_

struct InertFrm_DispVelAng {
  InertFrm_Disp Disp;
  InertFrm_Vel Vel;
  InertFrm_Angle Ang;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_InertFrm_DispVel_
#  define DEFINED_TYPEDEF_FOR_InertFrm_DispVel_

struct InertFrm_DispVel {
  InertFrm_Disp Disp;
  InertFrm_Vel Vel;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_InertFrm_Axle_
#  define DEFINED_TYPEDEF_FOR_InertFrm_Axle_

struct InertFrm_Axle {
  InertFrm_DispVel Lft;
  InertFrm_DispVel Rght;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Vehicle_InertFrm_Output_
#  define DEFINED_TYPEDEF_FOR_Vehicle_InertFrm_Output_

struct Vehicle_InertFrm_Output {
  InertFrm_DispVelAng Cg;
  InertFrm_Axle FrntAxl;
  InertFrm_Axle RearAxl;
  InertFrm_DispVel Hitch;
  InertFrm_DispVel Geom;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_Vel_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_Vel_

struct BdyFrm_Vel {
  real_T xdot;
  real_T ydot;
  real_T zdot;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_Angle_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_Angle_

struct BdyFrm_Angle {
  real_T Beta;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_AngVel_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_AngVel_

struct BdyFrm_AngVel {
  real_T p;
  real_T q;
  real_T r;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_Acc_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_Acc_

struct BdyFrm_Acc {
  real_T ax;
  real_T ay;
  real_T az;
  real_T xddot;
  real_T yddot;
  real_T zddot;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_AngAcc_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_AngAcc_

struct BdyFrm_AngAcc {
  real_T pdot;
  real_T qdot;
  real_T rdot;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_Cg_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_Cg_

struct BdyFrm_Cg {
  BdyFrm_Vel Vel;
  BdyFrm_Angle Ang;
  BdyFrm_AngVel AngVel;
  BdyFrm_Acc Acc;
  BdyFrm_AngAcc AngAcc;
  real_T DCM[9];
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_Force_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_Force_

struct BdyFrm_Force {
  real_T Fx;
  real_T Fy;
  real_T Fz;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_LftRght_Force_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_LftRght_Force_

struct BdyFrm_LftRght_Force {
  BdyFrm_Force Lft;
  BdyFrm_Force Rght;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_Tires_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_Tires_

struct BdyFrm_Tires {
  BdyFrm_LftRght_Force FrntTires;
  BdyFrm_LftRght_Force RearTires;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_Forces_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_Forces_

struct BdyFrm_Forces {
  BdyFrm_Force Body;
  BdyFrm_Force Ext;
  BdyFrm_LftRght_Force FrntAxl;
  BdyFrm_LftRght_Force RearAxl;
  BdyFrm_Force Hitch;
  BdyFrm_Tires Tires;
  BdyFrm_Force Drag;
  BdyFrm_Force Grvty;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_Moment_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_Moment_

struct BdyFrm_Moment {
  real_T Mx;
  real_T My;
  real_T Mz;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_Moments_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_Moments_

struct BdyFrm_Moments {
  BdyFrm_Moment Body;
  BdyFrm_Moment Drag;
  BdyFrm_Moment Ext;
  BdyFrm_Moment Hitch;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_Disp_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_Disp_

struct BdyFrm_Disp {
  real_T x;
  real_T y;
  real_T z;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_DispVel_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_DispVel_

struct BdyFrm_DispVel {
  BdyFrm_Disp Disp;
  BdyFrm_Vel Vel;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_LftRght_DispVel_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_LftRght_DispVel_

struct BdyFrm_LftRght_DispVel {
  BdyFrm_DispVel Lft;
  BdyFrm_DispVel Rght;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_Pwr_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_Pwr_

struct BdyFrm_Pwr {
  real_T PwrExt;
  real_T Drag;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_BdyFrm_DispVelAng_
#  define DEFINED_TYPEDEF_FOR_BdyFrm_DispVelAng_

struct BdyFrm_DispVelAng {
  BdyFrm_Disp Disp;
  BdyFrm_Vel Vel;
  BdyFrm_Angle Ang;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Vehicle_BdyFrm_Output_
#  define DEFINED_TYPEDEF_FOR_Vehicle_BdyFrm_Output_

struct Vehicle_BdyFrm_Output {
  BdyFrm_Cg Cg;
  BdyFrm_Forces Forces;
  BdyFrm_Moments Moments;
  BdyFrm_LftRght_DispVel FrntAxl;
  BdyFrm_LftRght_DispVel RearAxl;
  BdyFrm_DispVel Hitch;
  BdyFrm_Pwr Pwr;
  BdyFrm_DispVelAng Geom;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Vehicle_Body_Output_
#  define DEFINED_TYPEDEF_FOR_Vehicle_Body_Output_

struct Vehicle_Body_Output {
  Vehicle_InertFrm_Output InertFrm;
  Vehicle_BdyFrm_Output BdyFrm;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Susp_Veh_
#  define DEFINED_TYPEDEF_FOR_Susp_Veh_

struct Susp_Veh {
  real_T F[12];
  real_T M[12];
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Susp_Whl_
#  define DEFINED_TYPEDEF_FOR_Susp_Whl_

struct Susp_Whl {
  real_T F[12];
  real_T xdot[4];
  real_T ydot[4];
  real_T Ang[12];
  real_T z[4];
  real_T AngVel[12];
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Susp_Output_
#  define DEFINED_TYPEDEF_FOR_Susp_Output_

struct Susp_Output {
  Susp_Veh Veh;
  Susp_Whl Whl;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Wheels_TireFrame_Output_
#  define DEFINED_TYPEDEF_FOR_Wheels_TireFrame_Output_

struct Wheels_TireFrame_Output {
  real_T Omega[4];
  real_T Fx[4];
  real_T Fy[4];
  real_T Fz[4];
  real_T Mx[4];
  real_T My[4];
  real_T Mz[4];
  real_T Re[4];
  real_T dz[4];
  real_T z[4];
  real_T zdot[4];
  real_T Kappa[4];
  real_T Alpha[4];
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Wheels_Steer_Output_
#  define DEFINED_TYPEDEF_FOR_Wheels_Steer_Output_

struct Wheels_Steer_Output {
  real_T WhlAng[4];
  real_T WhlCmb[4];
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Wheels_Output_
#  define DEFINED_TYPEDEF_FOR_Wheels_Output_

struct Wheels_Output {
  Wheels_TireFrame_Output TireFrame;
  Wheels_Steer_Output Steering;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Steer_Output_
#  define DEFINED_TYPEDEF_FOR_Steer_Output_

struct Steer_Output {
  real_T wheel_angle[4];
  real_T steerWheelFeedback;
  real_T steerTorqueInput;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Steer_Input_
#  define DEFINED_TYPEDEF_FOR_Steer_Input_

struct Steer_Input {
  real_T steer_angle;
  real_T veh_speed;
};

#endif

/* Parameters (default storage) */
typedef struct P_DynamicSteer_T_ P_DynamicSteer_T;

/* Forward declaration for rtModel */
typedef struct tag_RTM_DynamicSteer_T RT_MODEL_DynamicSteer_T;

#endif /* RTW_HEADER_DynamicSteer_types_h_ */
