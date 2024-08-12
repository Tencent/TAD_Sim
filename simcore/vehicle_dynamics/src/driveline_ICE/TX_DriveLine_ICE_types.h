/*
 * TX_DriveLine_ICE_types.h
 *
 * Code generation for model "TX_DriveLine_ICE".
 *
 * Model version              : 1.200
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Tue Jul 18 21:36:16 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_TX_DriveLine_ICE_types_h_
#define RTW_HEADER_TX_DriveLine_ICE_types_h_
#include "rtwtypes.h"
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

/* Parameters for system: '<S8>/Limited Slip Differential' */
typedef struct P_LimitedSlipDifferential_TX_DriveLine_ICE_T_ P_LimitedSlipDifferential_TX_DriveLine_ICE_T;

/* Parameters (default storage) */
typedef struct P_TX_DriveLine_ICE_T_ P_TX_DriveLine_ICE_T;

/* Forward declaration for rtModel */
typedef struct tag_RTM_TX_DriveLine_ICE_T RT_MODEL_TX_DriveLine_ICE_T;

#endif /* RTW_HEADER_TX_DriveLine_ICE_types_h_ */
