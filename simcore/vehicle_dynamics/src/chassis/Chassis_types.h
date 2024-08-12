/*
 * Chassis_types.h
 *
 * Code generation for model "Chassis".
 *
 * Model version              : 2.33
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Thu Feb 16 17:07:18 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_Chassis_types_h_
#define RTW_HEADER_Chassis_types_h_
#include "rtwtypes.h"
#ifndef DEFINED_TYPEDEF_FOR_Chassis_DriveLine_Input_
#  define DEFINED_TYPEDEF_FOR_Chassis_DriveLine_Input_

struct Chassis_DriveLine_Input {
  real_T StrgAng[4];
  real_T AxlTrq[4];
  real_T BrkPrs[4];
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Env_Bus_GroundZ_
#  define DEFINED_TYPEDEF_FOR_Env_Bus_GroundZ_

struct Env_Bus_GroundZ {
  real_T G_FL_z;
  real_T G_FR_z;
  real_T G_RL_z;
  real_T G_RR_z;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Env_Bus_Mu_
#  define DEFINED_TYPEDEF_FOR_Env_Bus_Mu_

struct Env_Bus_Mu {
  real_T mu_FL;
  real_T mu_FR;
  real_T mu_RL;
  real_T mu_RR;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Env_Ground_
#  define DEFINED_TYPEDEF_FOR_Env_Ground_

struct Env_Ground {
  Env_Bus_GroundZ z;
  Env_Bus_Mu mu;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_Env_Bus_Output_
#  define DEFINED_TYPEDEF_FOR_Env_Bus_Output_

struct Env_Bus_Output {
  real_T Wind[3];
  Env_Ground Gnd;
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

#ifndef DEFINED_TYPEDEF_FOR_struct_hFtzkwIyDan9Uw0DEd8s7D_
#  define DEFINED_TYPEDEF_FOR_struct_hFtzkwIyDan9Uw0DEd8s7D_

struct struct_hFtzkwIyDan9Uw0DEd8s7D {
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
  real_T TrackWidth;
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
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_struct_c0tAwyh9OLYdudVeq0gUOE_
#  define DEFINED_TYPEDEF_FOR_struct_c0tAwyh9OLYdudVeq0gUOE_

struct struct_c0tAwyh9OLYdudVeq0gUOE {
  real_T FL[24];
  real_T FR[24];
  real_T RL[24];
  real_T RR[24];
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_struct_dDdmcOYPVmGdAXFrMV3khH_
#  define DEFINED_TYPEDEF_FOR_struct_dDdmcOYPVmGdAXFrMV3khH_

struct struct_dDdmcOYPVmGdAXFrMV3khH {
  real_T FL[6];
  real_T FR[6];
  real_T RL[6];
  real_T RR[6];
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_struct_BQkQZp112X5omA3WFa2DuB_
#  define DEFINED_TYPEDEF_FOR_struct_BQkQZp112X5omA3WFa2DuB_

struct struct_BQkQZp112X5omA3WFa2DuB {
  real_T FL[30];
  real_T FR[30];
  real_T RL[30];
  real_T RR[30];
};

#endif

/* Parameters for system: '<S9>/For each track and axle combination calculate suspension forces and moments' */
typedef struct P_CoreSubsys_Chassis_T_ P_CoreSubsys_Chassis_T;

/* Parameters for system: '<S213>/Wheel to Body Transform' */
typedef struct P_CoreSubsys_Chassis_gm_T_ P_CoreSubsys_Chassis_gm_T;

/* Parameters for system: '<S241>/Clutch' */
typedef struct P_Clutch_Chassis_T_ P_Clutch_Chassis_T;

/* Parameters for system: '<S237>/Clutch Scalar Parameters' */
typedef struct P_CoreSubsys_Chassis_l_T_ P_CoreSubsys_Chassis_l_T;

/* Parameters for system: '<S215>/Wheel to Body Transform' */
typedef struct P_CoreSubsys_Chassis_gmy_T_ P_CoreSubsys_Chassis_gmy_T;

/* Parameters (default storage) */
typedef struct P_Chassis_T_ P_Chassis_T;

/* Forward declaration for rtModel */
typedef struct tag_RTM_Chassis_T RT_MODEL_Chassis_T;

#endif /* RTW_HEADER_Chassis_types_h_ */
