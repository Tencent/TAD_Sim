/*
 * Chassis.h
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

#ifndef RTW_HEADER_Chassis_h_
#define RTW_HEADER_Chassis_h_
#include <cmath>
#include <cstring>
#include "Chassis_types.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rtwtypes.h"

#include "inc/car_common.h"

extern "C" {

#include "rt_nonfinite.h"
}

#include <cmath>
#include <cstring>
#include "zero_crossing_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetContStateDisabled
#  define rtmGetContStateDisabled(rtm) ((rtm)->contStateDisabled)
#endif

#ifndef rtmSetContStateDisabled
#  define rtmSetContStateDisabled(rtm, val) ((rtm)->contStateDisabled = (val))
#endif

#ifndef rtmGetContStates
#  define rtmGetContStates(rtm) ((rtm)->contStates)
#endif

#ifndef rtmSetContStates
#  define rtmSetContStates(rtm, val) ((rtm)->contStates = (val))
#endif

#ifndef rtmGetContTimeOutputInconsistentWithStateAtMajorStepFlag
#  define rtmGetContTimeOutputInconsistentWithStateAtMajorStepFlag(rtm) ((rtm)->CTOutputIncnstWithState)
#endif

#ifndef rtmSetContTimeOutputInconsistentWithStateAtMajorStepFlag
#  define rtmSetContTimeOutputInconsistentWithStateAtMajorStepFlag(rtm, val) ((rtm)->CTOutputIncnstWithState = (val))
#endif

#ifndef rtmGetDerivCacheNeedsReset
#  define rtmGetDerivCacheNeedsReset(rtm) ((rtm)->derivCacheNeedsReset)
#endif

#ifndef rtmSetDerivCacheNeedsReset
#  define rtmSetDerivCacheNeedsReset(rtm, val) ((rtm)->derivCacheNeedsReset = (val))
#endif

#ifndef rtmGetIntgData
#  define rtmGetIntgData(rtm) ((rtm)->intgData)
#endif

#ifndef rtmSetIntgData
#  define rtmSetIntgData(rtm, val) ((rtm)->intgData = (val))
#endif

#ifndef rtmGetOdeF
#  define rtmGetOdeF(rtm) ((rtm)->odeF)
#endif

#ifndef rtmSetOdeF
#  define rtmSetOdeF(rtm, val) ((rtm)->odeF = (val))
#endif

#ifndef rtmGetOdeY
#  define rtmGetOdeY(rtm) ((rtm)->odeY)
#endif

#ifndef rtmSetOdeY
#  define rtmSetOdeY(rtm, val) ((rtm)->odeY = (val))
#endif

#ifndef rtmGetPeriodicContStateIndices
#  define rtmGetPeriodicContStateIndices(rtm) ((rtm)->periodicContStateIndices)
#endif

#ifndef rtmSetPeriodicContStateIndices
#  define rtmSetPeriodicContStateIndices(rtm, val) ((rtm)->periodicContStateIndices = (val))
#endif

#ifndef rtmGetPeriodicContStateRanges
#  define rtmGetPeriodicContStateRanges(rtm) ((rtm)->periodicContStateRanges)
#endif

#ifndef rtmSetPeriodicContStateRanges
#  define rtmSetPeriodicContStateRanges(rtm, val) ((rtm)->periodicContStateRanges = (val))
#endif

#ifndef rtmGetZCCacheNeedsReset
#  define rtmGetZCCacheNeedsReset(rtm) ((rtm)->zCCacheNeedsReset)
#endif

#ifndef rtmSetZCCacheNeedsReset
#  define rtmSetZCCacheNeedsReset(rtm, val) ((rtm)->zCCacheNeedsReset = (val))
#endif

#ifndef rtmGetdX
#  define rtmGetdX(rtm) ((rtm)->derivs)
#endif

#ifndef rtmSetdX
#  define rtmSetdX(rtm, val) ((rtm)->derivs = (val))
#endif

#ifndef rtmGetErrorStatus
#  define rtmGetErrorStatus(rtm) ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#  define rtmSetErrorStatus(rtm, val) ((rtm)->errorStatus = (val))
#endif

#ifndef rtmGetStopRequested
#  define rtmGetStopRequested(rtm) ((rtm)->Timing.stopRequestedFlag)
#endif

#ifndef rtmSetStopRequested
#  define rtmSetStopRequested(rtm, val) ((rtm)->Timing.stopRequestedFlag = (val))
#endif

#ifndef rtmGetStopRequestedPtr
#  define rtmGetStopRequestedPtr(rtm) (&((rtm)->Timing.stopRequestedFlag))
#endif

#ifndef rtmGetT
#  define rtmGetT(rtm) (rtmGetTPtr((rtm))[0])
#endif

#ifndef rtmGetTPtr
#  define rtmGetTPtr(rtm) ((rtm)->Timing.t)
#endif

/* Block signals for system '<S9>/For each track and axle combination calculate suspension forces and moments' */
struct B_CoreSubsys_Chassis_T {
  real_T Sum2;            /* '<S19>/Sum2' */
  real_T Selector5;       /* '<S26>/Selector5' */
  real_T SumofElements;   /* '<S44>/Sum of Elements' */
  real_T Product;         /* '<S20>/Product' */
  real_T SumofElements_j; /* '<S70>/Sum of Elements' */
  real_T Selector10;      /* '<S71>/Selector10' */
  real_T Selector2;       /* '<S73>/Selector2' */
  real_T Selector8;       /* '<S38>/Selector8' */
  real_T Selector4;       /* '<S40>/Selector4' */
  real_T Selector5_j;     /* '<S27>/Selector5' */
  real_T SumofElements_p; /* '<S53>/Sum of Elements' */
  real_T Sign;            /* '<S28>/Sign' */
  real_T Selector2_m;     /* '<S90>/Selector2' */
  real_T Selector1;       /* '<S90>/Selector1' */
  real_T Selector4_b;     /* '<S58>/Selector4' */
  real_T SumofElements_o; /* '<S84>/Sum of Elements' */
  real_T Sign1;           /* '<S78>/Sign1' */
  real_T Sign_c;          /* '<S25>/Sign' */
};

/* Block states (default storage) for system '<S9>/For each track and axle combination calculate suspension forces and
 * moments' */
struct DW_CoreSubsys_Chassis_T {
  uint32_T m_bpIndex;           /* '<S39>/X-Y FL' */
  uint32_T m_bpIndex_k;         /* '<S39>/X-Y FR' */
  uint32_T m_bpIndex_p;         /* '<S39>/X-Y RL' */
  uint32_T m_bpIndex_a;         /* '<S39>/X-Y RR' */
  uint32_T m_bpIndex_l;         /* '<S72>/X-Y FL' */
  uint32_T m_bpIndex_aa;        /* '<S72>/X-Y FR' */
  uint32_T m_bpIndex_o;         /* '<S72>/X-Y RL' */
  uint32_T m_bpIndex_g;         /* '<S72>/X-Y RR' */
  uint32_T m_bpIndex_n;         /* '<S49>/X-Y FL' */
  uint32_T m_bpIndex_pu;        /* '<S49>/X-Y FR' */
  uint32_T m_bpIndex_gf;        /* '<S49>/X-Y RL' */
  uint32_T m_bpIndex_lb;        /* '<S49>/X-Y RR' */
  uint32_T m_bpIndex_gq;        /* '<S89>/X-Y FL1' */
  uint32_T m_bpIndex_gqg;       /* '<S89>/X-Y FR1' */
  uint32_T m_bpIndex_gqgq;      /* '<S89>/X-Y RL1' */
  uint32_T m_bpIndex_gqgqr;     /* '<S89>/X-Y RR1' */
  uint32_T m_bpIndex_gqgqrq;    /* '<S89>/X-Y FL' */
  uint32_T m_bpIndex_gqgqrq5;   /* '<S89>/X-Y FR' */
  uint32_T m_bpIndex_gqgqrq5i;  /* '<S89>/X-Y RL' */
  uint32_T m_bpIndex_gqgqrq5iw; /* '<S89>/X-Y RR' */
  uint32_T m_bpIndex_pi;        /* '<S62>/X-Y FL' */
  uint32_T m_bpIndex_piw;       /* '<S62>/X-Y FR' */
  uint32_T m_bpIndex_piwu;      /* '<S62>/X-Y RL' */
  uint32_T m_bpIndex_piwuj;     /* '<S62>/X-Y RR' */
  uint32_T m_bpIndex_p3;        /* '<S57>/X-Y FL' */
  uint32_T m_bpIndex_p31;       /* '<S57>/X-Y FR' */
  uint32_T m_bpIndex_p31z;      /* '<S57>/X-Y RL' */
  uint32_T m_bpIndex_p31zl;     /* '<S57>/X-Y RR' */
  uint32_T m_bpIndex_h;         /* '<S87>/CPz to tanTheta_CPSA_LF' */
  uint32_T m_bpIndex_hc;        /* '<S87>/CPz to tanTheta_CPSA_RF' */
  uint32_T m_bpIndex_hcw;       /* '<S87>/CPz to tanTheta_CPSA_LR' */
  uint32_T m_bpIndex_hcwc;      /* '<S87>/CPz to tanTheta_CPSA_RR' */
  uint32_T m_bpIndex_he;        /* '<S88>/X-Y RR1' */
  uint32_T m_bpIndex_her;       /* '<S88>/X-Y RL1' */
  uint32_T m_bpIndex_her2;      /* '<S88>/X-Y FR1' */
  uint32_T m_bpIndex_her23;     /* '<S88>/X-Y FL1' */
  uint32_T m_bpIndex_her230;    /* '<S88>/X-Y RR' */
  uint32_T m_bpIndex_her230u;   /* '<S88>/X-Y RL' */
  uint32_T m_bpIndex_her230um;  /* '<S88>/X-Y FR' */
  uint32_T m_bpIndex_her230umm; /* '<S88>/X-Y FL' */
};

/* Block signals for system '<S213>/Wheel to Body Transform' */
struct B_CoreSubsys_Chassis_ca_T {
  real_T Transpose1[9];        /* '<S220>/Transpose1' */
  real_T VectorConcatenate[9]; /* '<S225>/Vector Concatenate' */
};

/* Block signals for system '<S241>/Clutch' */
struct B_Clutch_Chassis_T {
  real_T Tout;                  /* '<S241>/Clutch' */
  real_T Tfmaxs;                /* '<S241>/Clutch' */
  real_T Tout_p;                /* '<S241>/Clutch' */
  real_T Tfmaxs_h;              /* '<S241>/Clutch' */
  real_T Omega;                 /* '<S241>/Clutch' */
  real_T Omegadot;              /* '<S241>/Clutch' */
  real_T Myb;                   /* '<S241>/Clutch' */
  real_T OutputInertia;         /* '<S244>/Output Inertia' */
  boolean_T CombinatorialLogic; /* '<S250>/Combinatorial  Logic' */
  boolean_T RelationalOperator; /* '<S254>/Relational Operator' */
};

/* Block states (default storage) for system '<S241>/Clutch' */
struct DW_Clutch_Chassis_T {
  real_T lastMajorTime;                          /* '<S241>/Clutch' */
  boolean_T UnitDelay_DSTATE;                    /* '<S250>/Unit Delay' */
  int8_T TmpIfAtSlippingInport3_ActiveSubsystem; /* synthesized block */
  int8_T TmpIfAtLockedInport2_ActiveSubsystem;   /* synthesized block */
  uint8_T is_c8_autolibshared;                   /* '<S241>/Clutch' */
  uint8_T is_active_c8_autolibshared;            /* '<S241>/Clutch' */
  boolean_T Slipping_entered;                    /* '<S241>/Clutch' */
  boolean_T Locked_entered;                      /* '<S241>/Clutch' */
};

/* Continuous states for system '<S241>/Clutch' */
struct X_Clutch_Chassis_T {
  real_T omegaWheel; /* '<S244>/omega wheel' */
};

/* State derivatives for system '<S241>/Clutch' */
struct XDot_Clutch_Chassis_T {
  real_T omegaWheel; /* '<S244>/omega wheel' */
};

/* State Disabled for system '<S241>/Clutch' */
struct XDis_Clutch_Chassis_T {
  boolean_T omegaWheel; /* '<S244>/omega wheel' */
};

/* Block signals for system '<S237>/Clutch Scalar Parameters' */
struct B_CoreSubsys_Chassis_f_T {
  B_Clutch_Chassis_T sf_Clutch; /* '<S241>/Clutch' */
};

/* Block states (default storage) for system '<S237>/Clutch Scalar Parameters' */
struct DW_CoreSubsys_Chassis_d_T {
  DW_Clutch_Chassis_T sf_Clutch; /* '<S241>/Clutch' */
};

/* Continuous states for system '<S237>/Clutch Scalar Parameters' */
struct X_CoreSubsys_Chassis_d_T {
  X_Clutch_Chassis_T sf_Clutch; /* '<S241>/Clutch' */
};

/* State derivatives for system '<S237>/Clutch Scalar Parameters' */
struct XDot_CoreSubsys_Chassis_j_T {
  XDot_Clutch_Chassis_T sf_Clutch; /* '<S241>/Clutch' */
};

/* State Disabled for system '<S237>/Clutch Scalar Parameters' */
struct XDis_CoreSubsys_Chassis_e_T {
  XDis_Clutch_Chassis_T sf_Clutch; /* '<S241>/Clutch' */
};

/* Block signals for system '<S215>/Wheel to Body Transform' */
struct B_CoreSubsys_Chassis_cau_T {
  real_T VectorConcatenate[9]; /* '<S261>/Vector Concatenate' */
};

/* Block signals (default storage) */
struct B_Chassis_T {
  BdyFrm_Tires Tires;
  BdyFrm_Moment Ext;
  BdyFrm_Moment Hitch;
  BdyFrm_Force Hitch_n;
  real_T phithetapsi[3];                             /* '<S115>/phi theta psi' */
  real_T ubvbwb[3];                                  /* '<S109>/ub,vb,wb' */
  real_T Product[3];                                 /* '<S122>/Product' */
  real_T Memory1[3];                                 /* '<S141>/Memory1' */
  real_T Selector1[3];                               /* '<S152>/Selector1' */
  real_T Selector1_n[3];                             /* '<S153>/Selector1' */
  real_T Selector1_o[3];                             /* '<S156>/Selector1' */
  real_T Selector1_l[3];                             /* '<S157>/Selector1' */
  real_T VectorConcatenate[3];                       /* '<S155>/Vector Concatenate' */
  real_T Subtract[3];                                /* '<S154>/Subtract' */
  real_T VectorConcatenate_p[3];                     /* '<S93>/Vector Concatenate' */
  real_T Fg_I[3];                                    /* '<S111>/Product' */
  real_T VectorConcatenate_k[6];                     /* '<S136>/Vector Concatenate' */
  real_T MathFunction[12];                           /* '<S213>/Math Function' */
  real_T Add1[4];                                    /* '<S231>/Add1' */
  real_T UnitConversion1[4];                         /* '<S240>/Unit Conversion1' */
  real_T Ratioofstatictokinetic[4];                  /* '<S238>/Ratio of static to kinetic' */
  real_T Ratioofstatictokinetic1[4];                 /* '<S238>/Ratio of static to kinetic1' */
  real_T MatrixConcatenate[12];                      /* '<S4>/Matrix Concatenate' */
  real_T VectorConcatenate3[12];                     /* '<S256>/Vector Concatenate3' */
  real_T MatrixConcatenate1[12];                     /* '<S10>/Matrix Concatenate1' */
  real_T MathFunction1[12];                          /* '<S213>/Math Function1' */
  real_T VectorConcatenate3_b[12];                   /* '<S218>/Vector Concatenate3' */
  real_T SumofElements[3];                           /* '<S131>/Sum of Elements' */
  real_T Sum[3];                                     /* '<S109>/Sum' */
  real_T Mx;                                         /* '<S149>/Mx' */
  real_T My;                                         /* '<S149>/My' */
  real_T Mz;                                         /* '<S149>/Mz' */
  real_T Selector1_b[9];                             /* '<S116>/Selector1' */
  real_T Selector[9];                                /* '<S116>/Selector' */
  real_T Selector2[9];                               /* '<S116>/Selector2' */
  real_T Product2[3];                                /* '<S116>/Product2' */
  real_T Divide[4];                                  /* '<S7>/Divide' */
  real_T Divide_h[4];                                /* '<S8>/Divide' */
  real_T Divide_e[2];                                /* '<S95>/Divide' */
  real_T TmpSignalConversionAtphithetapsiInport1[3]; /* '<S115>/phidot thetadot psidot' */
  real_T Divide_c[4];                                /* '<S210>/Divide' */
  real_T Divide_m[12];                               /* '<S211>/Divide' */
  real_T Product1[4];                                /* '<S232>/Product1' */
  real_T Product1_k[4];                              /* '<S233>/Product1' */
  real_T Product1_o[4];                              /* '<S235>/Product1' */
  real_T Gain1[4];                                   /* '<S230>/Gain1' */
  real_T y[4];                                       /* '<S215>/Camber Angle' */
  real_T ImpAsg_InsertedFor_Omega_at_inport_0[4];    /* '<S241>/Clutch' */
  real_T Re[4];                                      /* '<S229>/Magic Tire Const Input' */
  real_T Mbar;                                       /* '<S94>/vehdyncginert' */
  real_T Rbar[3];                                    /* '<S94>/vehdyncginert' */
  real_T Xbar[3];                                    /* '<S94>/vehdyncginert' */
  real_T Wbar[4];                                    /* '<S94>/vehdyncginert' */
  real_T HPbar[12];                                  /* '<S94>/vehdyncginert' */
  real_T Sum_j[3];                                   /* '<S143>/Sum' */
  boolean_T Compare[3];                              /* '<S142>/Compare' */
  B_CoreSubsys_Chassis_cau_T CoreSubsys_pna[4];      /* '<S215>/Wheel to Body Transform' */
  B_CoreSubsys_Chassis_f_T CoreSubsys_d0[4];         /* '<S237>/Clutch Scalar Parameters' */
  B_CoreSubsys_Chassis_ca_T CoreSubsys_pn[4];        /* '<S213>/Wheel to Body Transform' */
  B_CoreSubsys_Chassis_T CoreSubsys[4];
  /* '<S9>/For each track and axle combination calculate suspension forces and moments' */
};

/* Block states (default storage) for system '<Root>' */
struct DW_Chassis_T {
  real_T UnitDelay_DSTATE[3];                 /* '<S143>/Unit Delay' */
  real_T Memory1_PreviousInput[3];            /* '<S141>/Memory1' */
  real_T Memory_PreviousInput[4];             /* '<S239>/Memory' */
  real_T Product2_DWORK4[9];                  /* '<S116>/Product2' */
  uint32_T m_bpIndex[8];                      /* '<S240>/2-D Lookup Table' */
  int_T IntegratorSecondOrder_MODE[4];        /* '<S230>/Integrator, Second-Order' */
  boolean_T IntegratorSecondOrder_DWORK1;     /* '<S230>/Integrator, Second-Order' */
  DW_CoreSubsys_Chassis_d_T CoreSubsys_d0[4]; /* '<S237>/Clutch Scalar Parameters' */
  DW_CoreSubsys_Chassis_T CoreSubsys[4];
  /* '<S9>/For each track and axle combination calculate suspension forces and moments' */
};

/* Continuous states (default storage) */
struct X_Chassis_T {
  real_T xeyeze_CSTATE[3];                   /* '<S109>/xe,ye,ze' */
  real_T phithetapsi_CSTATE[3];              /* '<S115>/phi theta psi' */
  real_T ubvbwb_CSTATE[3];                   /* '<S109>/ub,vb,wb' */
  real_T pqr_CSTATE[3];                      /* '<S109>/p,q,r ' */
  real_T Integrator1_CSTATE[2];              /* '<S95>/Integrator1' */
  real_T IntegratorSecondOrder_CSTATE[8];    /* '<S230>/Integrator, Second-Order' */
  real_T Integrator_CSTATE[4];               /* '<S232>/Integrator' */
  real_T Integrator_CSTATE_c[4];             /* '<S233>/Integrator' */
  real_T Integrator1_CSTATE_h[4];            /* '<S210>/Integrator1' */
  real_T Integrator_CSTATE_a[4];             /* '<S235>/Integrator' */
  real_T Integrator1_CSTATE_p[12];           /* '<S211>/Integrator1' */
  real_T Integrator1_CSTATE_a[4];            /* '<S7>/Integrator1' */
  real_T Integrator1_CSTATE_o[4];            /* '<S8>/Integrator1' */
  real_T Integrator_CSTATE_o[3];             /* '<S113>/Integrator' */
  X_CoreSubsys_Chassis_d_T CoreSubsys_d0[4]; /* '<S241>/CoreSubsys' */
};

/* Periodic continuous state vector (global) */
typedef int_T PeriodicIndX_Chassis_T[3];
typedef real_T PeriodicRngX_Chassis_T[6];

/* State derivatives (default storage) */
struct XDot_Chassis_T {
  real_T xeyeze_CSTATE[3];                      /* '<S109>/xe,ye,ze' */
  real_T phithetapsi_CSTATE[3];                 /* '<S115>/phi theta psi' */
  real_T ubvbwb_CSTATE[3];                      /* '<S109>/ub,vb,wb' */
  real_T pqr_CSTATE[3];                         /* '<S109>/p,q,r ' */
  real_T Integrator1_CSTATE[2];                 /* '<S95>/Integrator1' */
  real_T IntegratorSecondOrder_CSTATE[8];       /* '<S230>/Integrator, Second-Order' */
  real_T Integrator_CSTATE[4];                  /* '<S232>/Integrator' */
  real_T Integrator_CSTATE_c[4];                /* '<S233>/Integrator' */
  real_T Integrator1_CSTATE_h[4];               /* '<S210>/Integrator1' */
  real_T Integrator_CSTATE_a[4];                /* '<S235>/Integrator' */
  real_T Integrator1_CSTATE_p[12];              /* '<S211>/Integrator1' */
  real_T Integrator1_CSTATE_a[4];               /* '<S7>/Integrator1' */
  real_T Integrator1_CSTATE_o[4];               /* '<S8>/Integrator1' */
  real_T Integrator_CSTATE_o[3];                /* '<S113>/Integrator' */
  XDot_CoreSubsys_Chassis_j_T CoreSubsys_d0[4]; /* '<S241>/CoreSubsys' */
};

/* State disabled  */
struct XDis_Chassis_T {
  boolean_T xeyeze_CSTATE[3];                   /* '<S109>/xe,ye,ze' */
  boolean_T phithetapsi_CSTATE[3];              /* '<S115>/phi theta psi' */
  boolean_T ubvbwb_CSTATE[3];                   /* '<S109>/ub,vb,wb' */
  boolean_T pqr_CSTATE[3];                      /* '<S109>/p,q,r ' */
  boolean_T Integrator1_CSTATE[2];              /* '<S95>/Integrator1' */
  boolean_T IntegratorSecondOrder_CSTATE[8];    /* '<S230>/Integrator, Second-Order' */
  boolean_T Integrator_CSTATE[4];               /* '<S232>/Integrator' */
  boolean_T Integrator_CSTATE_c[4];             /* '<S233>/Integrator' */
  boolean_T Integrator1_CSTATE_h[4];            /* '<S210>/Integrator1' */
  boolean_T Integrator_CSTATE_a[4];             /* '<S235>/Integrator' */
  boolean_T Integrator1_CSTATE_p[12];           /* '<S211>/Integrator1' */
  boolean_T Integrator1_CSTATE_a[4];            /* '<S7>/Integrator1' */
  boolean_T Integrator1_CSTATE_o[4];            /* '<S8>/Integrator1' */
  boolean_T Integrator_CSTATE_o[3];             /* '<S113>/Integrator' */
  XDis_CoreSubsys_Chassis_e_T CoreSubsys_d0[4]; /* '<S241>/CoreSubsys' */
};

/* Zero-crossing (trigger) state */
struct PrevZCX_Chassis_T {
  ZCSigState FunctionCallSubsystem_Trig_ZCE[3]; /* '<S141>/Function-Call Subsystem' */
};

#ifndef ODE4_INTG
#  define ODE4_INTG

/* ODE4 Integration Data */
struct ODE4_IntgData {
  real_T *y;    /* output */
  real_T *f[4]; /* derivatives */
};

#endif

/* External inputs (root inport signals with default storage) */
struct ExtU_Chassis_T {
  Chassis_DriveLine_Input DriveLineInput; /* '<Root>/DriveLineInput' */
  Env_Bus_Output Env;                     /* '<Root>/Env' */
};

/* External outputs (root outports fed by signals with default storage) */
struct ExtY_Chassis_T {
  Vehicle_Body_Output VehBody; /* '<Root>/VehBody' */
  Wheels_Output WheelsOut;     /* '<Root>/WheelsOut' */
  Susp_Output Susp;            /* '<Root>/Susp' */
};

/* Parameters for system: '<S9>/For each track and axle combination calculate suspension forces and moments' */
struct P_CoreSubsys_Chassis_T_ {
  real_T SelectSteerCamberSlope_AxleNums;
  /* Mask Parameter: SelectSteerCamberSlope_AxleNums
   * Referenced by: '<S44>/Axle Numbers'
   */
  real_T SelectStaticToeSetting_AxleNums[2];
  /* Mask Parameter: SelectStaticToeSetting_AxleNums
   * Referenced by: '<S70>/Axle Numbers'
   */
  real_T SelectSteerCasterSlope_AxleNums;
  /* Mask Parameter: SelectSteerCasterSlope_AxleNums
   * Referenced by: '<S53>/Axle Numbers'
   */
  real_T ConstrainedspringdampercombinationforKC_AxleNums[2];
  /* Mask Parameter: ConstrainedspringdampercombinationforKC_AxleNums
   * Referenced by: '<S84>/Axle Numbers'
   */
  real_T XYRR_tableData[15];                          /* Expression: tanTheta_SVSA_WC_RR
                                                       * Referenced by: '<S88>/X-Y RR'
                                                       */
  real_T XYRR_bp01Data[15];                           /* Expression: LatWhlCtrDisp.RR(:, 1)
                                                       * Referenced by: '<S88>/X-Y RR'
                                                       */
  real_T XYRL_tableData[15];                          /* Expression: tanTheta_SVSA_WC_RL
                                                       * Referenced by: '<S88>/X-Y RL'
                                                       */
  real_T XYRL_bp01Data[15];                           /* Expression: LatWhlCtrDisp.RL(:, 1)
                                                       * Referenced by: '<S88>/X-Y RL'
                                                       */
  real_T XYFR_tableData[15];                          /* Expression: tanTheta_SVSA_WC_FR
                                                       * Referenced by: '<S88>/X-Y FR'
                                                       */
  real_T XYFR_bp01Data[15];                           /* Expression: LatWhlCtrDisp.FR(:, 1)
                                                       * Referenced by: '<S88>/X-Y FR'
                                                       */
  real_T XYFL_tableData[15];                          /* Expression: tanTheta_SVSA_WC_FL
                                                       * Referenced by: '<S88>/X-Y FL'
                                                       */
  real_T XYFL_bp01Data[15];                           /* Expression: LatWhlCtrDisp.FL(:, 1)
                                                       * Referenced by: '<S88>/X-Y FL'
                                                       */
  real_T XYRR1_tableData[15];                         /* Expression: tanTheta_SVSA_CP_RR
                                                       * Referenced by: '<S88>/X-Y RR1'
                                                       */
  real_T XYRR1_bp01Data[15];                          /* Expression: LatWhlCtrDisp.RR(:, 1)
                                                       * Referenced by: '<S88>/X-Y RR1'
                                                       */
  real_T XYRL1_tableData[15];                         /* Expression: tanTheta_SVSA_CP_RL
                                                       * Referenced by: '<S88>/X-Y RL1'
                                                       */
  real_T XYRL1_bp01Data[15];                          /* Expression: LatWhlCtrDisp.RL(:, 1)
                                                       * Referenced by: '<S88>/X-Y RL1'
                                                       */
  real_T XYFR1_tableData[15];                         /* Expression: tanTheta_SVSA_CP_FR
                                                       * Referenced by: '<S88>/X-Y FR1'
                                                       */
  real_T XYFR1_bp01Data[15];                          /* Expression: LatWhlCtrDisp.FR(:, 1)
                                                       * Referenced by: '<S88>/X-Y FR1'
                                                       */
  real_T XYFL1_tableData[15];                         /* Expression: tanTheta_SVSA_CP_FL
                                                       * Referenced by: '<S88>/X-Y FL1'
                                                       */
  real_T XYFL1_bp01Data[15];                          /* Expression: LatWhlCtrDisp.FL(:, 1)
                                                       * Referenced by: '<S88>/X-Y FL1'
                                                       */
  real_T Constant1_Value[2];                          /* Expression: [0 0]
                                                       * Referenced by: '<S83>/Constant1'
                                                       */
  real_T VehicleVehicleWheelOffset3_Value[3];         /* Expression: [0 cumsum(NumWhlsByAxl)]
                                                       * Referenced by: '<S19>/Vehicle Vehicle Wheel Offset3'
                                                       */
  real_T HeightSignConvention_Gain;                   /* Expression: -1
                                                       * Referenced by: '<S77>/Height Sign Convention'
                                                       */
  real_T SignConvention_Gain;                         /* Expression: -1
                                                       * Referenced by: '<S24>/Sign Convention'
                                                       */
  real_T Gain5_Gain;                                  /* Expression: VerticalSign
                                                       * Referenced by: '<S26>/Gain5'
                                                       */
  real_T XYFL_tableData_o[tx_car::kMap1dSize];  /* Expression: BumpCamber.FL(:, 2)
                                                       * Referenced by: '<S39>/X-Y FL'
                                                       */
  real_T XYFL_bp01Data_g[tx_car::kMap1dSize];   /* Expression: BumpCamber.FL(:, 1)
                                                       * Referenced by: '<S39>/X-Y FL'
                                                       */
  real_T XYFR_tableData_j[tx_car::kMap1dSize];  /* Expression: BumpCamber.FR(:, 2)
                                                       * Referenced by: '<S39>/X-Y FR'
                                                       */
  real_T XYFR_bp01Data_g[tx_car::kMap1dSize];   /* Expression: BumpCamber.FR(:, 1)
                                                       * Referenced by: '<S39>/X-Y FR'
                                                       */
  real_T XYRL_tableData_a[tx_car::kMap1dSize];  /* Expression: BumpCamber.RL(:, 2)
                                                       * Referenced by: '<S39>/X-Y RL'
                                                       */
  real_T XYRL_bp01Data_p[tx_car::kMap1dSize];   /* Expression: BumpCamber.RL(:, 1)
                                                       * Referenced by: '<S39>/X-Y RL'
                                                       */
  real_T XYRR_tableData_l[tx_car::kMap1dSize];  /* Expression: BumpCamber.RR(:, 2)
                                                       * Referenced by: '<S39>/X-Y RR'
                                                       */
  real_T XYRR_bp01Data_i[tx_car::kMap1dSize];   /* Expression: BumpCamber.RR(:, 1)
                                                       * Referenced by: '<S39>/X-Y RR'
                                                       */
  real_T Constant_Value;                              /* Expression: 0
                                                       * Referenced by: '<S20>/Constant'
                                                       */
  real_T VehicleVehicleWheelOffset1_Value[2];         /* Expression: StrgEnByAxl
                                                       * Referenced by: '<S20>/Vehicle Vehicle Wheel Offset1'
                                                       */
  real_T VehicleVehicleWheelOffset3_Value_k[2];       /* Expression: NumWhlsByAxl
                                                       * Referenced by: '<S64>/Vehicle Vehicle Wheel Offset3'
                                                       */
  real_T Constant_Value_d;                            /* Expression: 1
                                                       * Referenced by: '<S64>/Constant'
                                                       */
  real_T Gain_Gain;                                   /* Expression: 1/2
                                                       * Referenced by: '<S64>/Gain'
                                                       */
  real_T Gain5_Gain_l;                                /* Expression: VerticalSign
                                                       * Referenced by: '<S31>/Gain5'
                                                       */
  real_T XYFL_tableData_n[tx_car::kMap1dSize];  /* Expression: BumpSteer.FL(:, 2)
                                                       * Referenced by: '<S72>/X-Y FL'
                                                       */
  real_T XYFL_bp01Data_n[tx_car::kMap1dSize];   /* Expression: BumpSteer.FL(:, 1)
                                                       * Referenced by: '<S72>/X-Y FL'
                                                       */
  real_T XYFR_tableData_h[tx_car::kMap1dSize];  /* Expression: BumpSteer.FR(:, 2)
                                                       * Referenced by: '<S72>/X-Y FR'
                                                       */
  real_T XYFR_bp01Data_e[tx_car::kMap1dSize];   /* Expression: BumpSteer.FR(:, 1)
                                                       * Referenced by: '<S72>/X-Y FR'
                                                       */
  real_T XYRL_tableData_k[tx_car::kMap1dSize];  /* Expression: BumpSteer.RL(:, 2)
                                                       * Referenced by: '<S72>/X-Y RL'
                                                       */
  real_T XYRL_bp01Data_j[tx_car::kMap1dSize];   /* Expression: BumpSteer.RL(:, 1)
                                                       * Referenced by: '<S72>/X-Y RL'
                                                       */
  real_T XYRR_tableData_p[tx_car::kMap1dSize];  /* Expression: BumpSteer.RR(:, 2)
                                                       * Referenced by: '<S72>/X-Y RR'
                                                       */
  real_T XYRR_bp01Data_p[tx_car::kMap1dSize];   /* Expression: BumpSteer.RR(:, 1)
                                                       * Referenced by: '<S72>/X-Y RR'
                                                       */
  real_T Gain4_Gain;                                  /* Expression: WhlMzSign
                                                       * Referenced by: '<S31>/Gain4'
                                                       */
  real_T Gain3_Gain;                                  /* Expression: LateralSign
                                                       * Referenced by: '<S31>/Gain3'
                                                       */
  real_T Constant6_Value[4];                          /* Expression: LngSteerCompl(2, :)
                                                       * Referenced by: '<S74>/Constant6'
                                                       */
  real_T Gain1_Gain;                                  /* Expression: LongitudinalSign
                                                       * Referenced by: '<S31>/Gain1'
                                                       */
  real_T Gain2_Gain;                                  /* Expression: LongitudinalSign
                                                       * Referenced by: '<S74>/Gain2'
                                                       */
  real_T Constant3_Value[4];                          /* Expression: LngSteerCompl(1, :)
                                                       * Referenced by: '<S74>/Constant3'
                                                       */
  real_T Switch_Threshold;                            /* Expression: 0
                                                       * Referenced by: '<S74>/Switch'
                                                       */
  real_T Gain_Gain_c;                                 /* Expression: SteerSign
                                                       * Referenced by: '<S31>/Gain'
                                                       */
  real_T Gain_Gain_i;                                 /* Expression: SteerSign
                                                       * Referenced by: '<S26>/Gain'
                                                       */
  real_T Gain4_Gain_j;                                /* Expression: WhlMzSign
                                                       * Referenced by: '<S26>/Gain4'
                                                       */
  real_T Gain3_Gain_i;                                /* Expression: LateralSign
                                                       * Referenced by: '<S26>/Gain3'
                                                       */
  real_T Constant6_Value_j[4];                        /* Expression: LngCambCompl(2, :)
                                                       * Referenced by: '<S41>/Constant6'
                                                       */
  real_T Gain1_Gain_b;                                /* Expression: LongitudinalSign
                                                       * Referenced by: '<S26>/Gain1'
                                                       */
  real_T Gain2_Gain_j;                                /* Expression: LongitudinalSign
                                                       * Referenced by: '<S41>/Gain2'
                                                       */
  real_T Constant1_Value_j[4];                        /* Expression: LngCambCompl(1, :)
                                                       * Referenced by: '<S41>/Constant1'
                                                       */
  real_T Switch_Threshold_j;                          /* Expression: 0
                                                       * Referenced by: '<S41>/Switch'
                                                       */
  real_T Gain4_Gain_f;                                /* Expression: VerticalSign
                                                       * Referenced by: '<S27>/Gain4'
                                                       */
  real_T XYFL_tableData_n0[tx_car::kMap1dSize]; /* Expression: BumpCaster.FL(:, 2)
                                                       * Referenced by: '<S49>/X-Y FL'
                                                       */
  real_T XYFL_bp01Data_l[tx_car::kMap1dSize];   /* Expression: BumpCaster.FL(:, 1)
                                                       * Referenced by: '<S49>/X-Y FL'
                                                       */
  real_T XYFR_tableData_n[tx_car::kMap1dSize];  /* Expression: BumpCaster.FR(:, 2)
                                                       * Referenced by: '<S49>/X-Y FR'
                                                       */
  real_T XYFR_bp01Data_b[tx_car::kMap1dSize];   /* Expression: BumpCaster.FR(:, 1)
                                                       * Referenced by: '<S49>/X-Y FR'
                                                       */
  real_T XYRL_tableData_as[tx_car::kMap1dSize]; /* Expression: BumpCaster.RL(:, 2)
                                                       * Referenced by: '<S49>/X-Y RL'
                                                       */
  real_T XYRL_bp01Data_d[tx_car::kMap1dSize];   /* Expression: BumpCaster.RL(:, 1)
                                                       * Referenced by: '<S49>/X-Y RL'
                                                       */
  real_T XYRR_tableData_pz[tx_car::kMap1dSize]; /* Expression: BumpCaster.RR(:, 2)
                                                       * Referenced by: '<S49>/X-Y RR'
                                                       */
  real_T XYRR_bp01Data_d[tx_car::kMap1dSize];   /* Expression: BumpCaster.RR(:, 1)
                                                       * Referenced by: '<S49>/X-Y RR'
                                                       */
  real_T Gain_Gain_o;                                 /* Expression: SteerSign
                                                       * Referenced by: '<S27>/Gain'
                                                       */
  real_T Constant4_Value[4];                          /* Expression: LngCastCompl(2, :)
                                                       * Referenced by: '<S50>/Constant4'
                                                       */
  real_T Gain2_Gain_l;                                /* Expression: LongitudinalSign
                                                       * Referenced by: '<S27>/Gain2'
                                                       */
  real_T Gain2_Gain_o;                                /* Expression: LongitudinalSign
                                                       * Referenced by: '<S50>/Gain2'
                                                       */
  real_T Constant2_Value[4];                          /* Expression: LngCastCompl(1, :)
                                                       * Referenced by: '<S50>/Constant2'
                                                       */
  real_T Switch_Threshold_o;                          /* Expression: 0
                                                       * Referenced by: '<S50>/Switch'
                                                       */
  real_T VehicleVehicleWheelOffset3_Value_l[2];       /* Expression: NumWhlsByAxl
                                                       * Referenced by: '<S28>/Vehicle Vehicle Wheel Offset3'
                                                       */
  real_T Constant_Value_dc;                           /* Expression: 1
                                                       * Referenced by: '<S28>/Constant'
                                                       */
  real_T Gain_Gain_d;                                 /* Expression: 1/2
                                                       * Referenced by: '<S28>/Gain'
                                                       */
  real_T Gain2_Gain_k;                                /* Expression: VerticalSign
                                                       * Referenced by: '<S77>/Gain2'
                                                       */
  real_T Gain1_Gain_h;                                /* Expression: VerticalSign
                                                       * Referenced by: '<S77>/Gain1'
                                                       */
  real_T XYFL1_tableData_p[3];                        /* Expression: MotRatios.FL(:, 2)
                                                       * Referenced by: '<S89>/X-Y FL1'
                                                       */
  real_T XYFL1_bp01Data_p[3];                         /* Expression: MotRatios.FL(:, 1)
                                                       * Referenced by: '<S89>/X-Y FL1'
                                                       */
  real_T XYFR1_tableData_p[3];                        /* Expression: MotRatios.FR(:, 2)
                                                       * Referenced by: '<S89>/X-Y FR1'
                                                       */
  real_T XYFR1_bp01Data_p[3];                         /* Expression: MotRatios.FR(:, 1)
                                                       * Referenced by: '<S89>/X-Y FR1'
                                                       */
  real_T XYRL1_tableData_p[3];                        /* Expression: MotRatios.RL(:, 2)
                                                       * Referenced by: '<S89>/X-Y RL1'
                                                       */
  real_T XYRL1_bp01Data_p[3];                         /* Expression: MotRatios.RL(:, 1)
                                                       * Referenced by: '<S89>/X-Y RL1'
                                                       */
  real_T XYRR1_tableData_p[3];                        /* Expression: MotRatios.RR(:, 2)
                                                       * Referenced by: '<S89>/X-Y RR1'
                                                       */
  real_T XYRR1_bp01Data_p[3];                         /* Expression: MotRatios.RR(:, 1)
                                                       * Referenced by: '<S89>/X-Y RR1'
                                                       */
  real_T XYFL_tableData_p[tx_car::kMap1dSize];  /* Expression: ShckFrcVsCompRate.FL(:, 2)
                                                       * Referenced by: '<S89>/X-Y FL'
                                                       */
  real_T XYFL_bp01Data_p[tx_car::kMap1dSize];   /* Expression: ShckFrcVsCompRate.FL(:, 1)
                                                       * Referenced by: '<S89>/X-Y FL'
                                                       */
  real_T XYFR_tableData_p[tx_car::kMap1dSize];  /* Expression: ShckFrcVsCompRate.FR(:, 2)
                                                       * Referenced by: '<S89>/X-Y FR'
                                                       */
  real_T XYFR_bp01Data_p[tx_car::kMap1dSize];   /* Expression: ShckFrcVsCompRate.FR(:, 1)
                                                       * Referenced by: '<S89>/X-Y FR'
                                                       */
  real_T XYRL_tableData_p[tx_car::kMap1dSize];  /* Expression: ShckFrcVsCompRate.RL(:, 2)
                                                       * Referenced by: '<S89>/X-Y RL'
                                                       */
  real_T XYRL_bp01Data_p1[tx_car::kMap1dSize];  /* Expression: ShckFrcVsCompRate.RL(:, 1)
                                                       * Referenced by: '<S89>/X-Y RL'
                                                       */
  real_T XYRR_tableData_p1[tx_car::kMap1dSize]; /* Expression: ShckFrcVsCompRate.RR(:, 2)
                                                       * Referenced by: '<S89>/X-Y RR'
                                                       */
  real_T XYRR_bp01Data_p1[tx_car::kMap1dSize];  /* Expression: ShckFrcVsCompRate.RR(:, 1)
                                                       * Referenced by: '<S89>/X-Y RR'
                                                       */
  real_T Gain4_Gain_g;                                /* Expression: VerticalSign
                                                       * Referenced by: '<S30>/Gain4'
                                                       */
  real_T XYFL_tableData_o5[tx_car::kMap1dSize]; /* Expression: LngWhlCtrDisp.FL(:, 2)
                                                       * Referenced by: '<S62>/X-Y FL'
                                                       */
  real_T XYFL_bp01Data_o[tx_car::kMap1dSize];   /* Expression: LngWhlCtrDisp.FL(:, 1)
                                                       * Referenced by: '<S62>/X-Y FL'
                                                       */
  real_T XYFR_tableData_o[tx_car::kMap1dSize];  /* Expression: LngWhlCtrDisp.FR(:, 2)
                                                       * Referenced by: '<S62>/X-Y FR'
                                                       */
  real_T XYFR_bp01Data_o[tx_car::kMap1dSize];   /* Expression: LngWhlCtrDisp.FR(:, 1)
                                                       * Referenced by: '<S62>/X-Y FR'
                                                       */
  real_T XYRL_tableData_o[tx_car::kMap1dSize];  /* Expression: LngWhlCtrDisp.RL(:, 2)
                                                       * Referenced by: '<S62>/X-Y RL'
                                                       */
  real_T XYRL_bp01Data_o[tx_car::kMap1dSize];   /* Expression: LngWhlCtrDisp.RL(:, 1)
                                                       * Referenced by: '<S62>/X-Y RL'
                                                       */
  real_T XYRR_tableData_o[tx_car::kMap1dSize];  /* Expression: LngWhlCtrDisp.RR(:, 2)
                                                       * Referenced by: '<S62>/X-Y RR'
                                                       */
  real_T XYRR_bp01Data_o[tx_car::kMap1dSize];   /* Expression: LngWhlCtrDisp.RR(:, 1)
                                                       * Referenced by: '<S62>/X-Y RR'
                                                       */
  real_T Constant2_Value_m[4];                        /* Expression: LngWhlCtrCompl(2, :)
                                                       * Referenced by: '<S63>/Constant2'
                                                       */
  real_T Gain_Gain_k;                                 /* Expression: LongitudinalSign
                                                       * Referenced by: '<S30>/Gain'
                                                       */
  real_T Gain2_Gain_m;                                /* Expression: LongitudinalSign
                                                       * Referenced by: '<S63>/Gain2'
                                                       */
  real_T Constant1_Value_m[4];                        /* Expression: LngWhlCtrCompl(1, :)
                                                       * Referenced by: '<S63>/Constant1'
                                                       */
  real_T Switch_Threshold_m;                          /* Expression: 0
                                                       * Referenced by: '<S63>/Switch'
                                                       */
  real_T Gain1_Gain_c;                                /* Expression: LongitudinalSign
                                                       * Referenced by: '<S30>/Gain1'
                                                       */
  real_T Gain4_Gain_e;                                /* Expression: VerticalSign
                                                       * Referenced by: '<S29>/Gain4'
                                                       */
  real_T XYFL_tableData_g[tx_car::kMap1dSize];  /* Expression: LatWhlCtrDisp.FL(:, 2)
                                                       * Referenced by: '<S57>/X-Y FL'
                                                       */
  real_T XYFL_bp01Data_g3[tx_car::kMap1dSize];  /* Expression: LatWhlCtrDisp.FL(:, 1)
                                                       * Referenced by: '<S57>/X-Y FL'
                                                       */
  real_T XYFR_tableData_g[tx_car::kMap1dSize];  /* Expression: LatWhlCtrDisp.FR(:, 2)
                                                       * Referenced by: '<S57>/X-Y FR'
                                                       */
  real_T XYFR_bp01Data_g3[tx_car::kMap1dSize];  /* Expression: LatWhlCtrDisp.FR(:, 1)
                                                       * Referenced by: '<S57>/X-Y FR'
                                                       */
  real_T XYRL_tableData_g[tx_car::kMap1dSize];  /* Expression: LatWhlCtrDisp.RL(:, 2)
                                                       * Referenced by: '<S57>/X-Y RL'
                                                       */
  real_T XYRL_bp01Data_g[tx_car::kMap1dSize];   /* Expression: LatWhlCtrDisp.RL(:, 1)
                                                       * Referenced by: '<S57>/X-Y RL'
                                                       */
  real_T XYRR_tableData_g[tx_car::kMap1dSize];  /* Expression: LatWhlCtrDisp.RR(:, 2)
                                                       * Referenced by: '<S57>/X-Y RR'
                                                       */
  real_T XYRR_bp01Data_g[tx_car::kMap1dSize];   /* Expression: LatWhlCtrDisp.RR(:, 1)
                                                       * Referenced by: '<S57>/X-Y RR'
                                                       */
  real_T Gain3_Gain_a;                                /* Expression: LateralSign
                                                       * Referenced by: '<S29>/Gain3'
                                                       */
  real_T Constant3_Value_l[4];                        /* Expression: LatWhlCtrComplLngBrk(2, :)
                                                       * Referenced by: '<S59>/Constant3'
                                                       */
  real_T Gain_Gain_a;                                 /* Expression: LongitudinalSign
                                                       * Referenced by: '<S29>/Gain'
                                                       */
  real_T Gain2_Gain_l2;                               /* Expression: LongitudinalSign
                                                       * Referenced by: '<S59>/Gain2'
                                                       */
  real_T Constant1_Value_l[4];                        /* Expression: LatWhlCtrComplLngBrk(1, :)
                                                       * Referenced by: '<S59>/Constant1'
                                                       */
  real_T Switch_Threshold_l;                          /* Expression: 0
                                                       * Referenced by: '<S59>/Switch'
                                                       */
  real_T Gain1_Gain_k;                                /* Expression: LateralSign
                                                       * Referenced by: '<S29>/Gain1'
                                                       */
  real_T Gain3_Gain_g;                                /* Expression: VerticalSign
                                                       * Referenced by: '<S77>/Gain3'
                                                       */
  real_T CPztotanTheta_CPSA_LF_tableData[15];         /* Expression: tanTheta_FL
                                                       * Referenced by: '<S87>/CPz to tanTheta_CPSA_LF'
                                                       */
  real_T CPztotanTheta_CPSA_LF_bp01Data[15];          /* Expression: LatWhlCtrDisp.FL(:, 1)
                                                       * Referenced by: '<S87>/CPz to tanTheta_CPSA_LF'
                                                       */
  real_T CPztotanTheta_CPSA_RF_tableData[15];         /* Expression: tanTheta_FR
                                                       * Referenced by: '<S87>/CPz to tanTheta_CPSA_RF'
                                                       */
  real_T CPztotanTheta_CPSA_RF_bp01Data[15];          /* Expression: LatWhlCtrDisp.FR(:, 1)
                                                       * Referenced by: '<S87>/CPz to tanTheta_CPSA_RF'
                                                       */
  real_T CPztotanTheta_CPSA_LR_tableData[15];         /* Expression: tanTheta_RL
                                                       * Referenced by: '<S87>/CPz to tanTheta_CPSA_LR'
                                                       */
  real_T CPztotanTheta_CPSA_LR_bp01Data[15];          /* Expression: LatWhlCtrDisp.RL(:, 1)
                                                       * Referenced by: '<S87>/CPz to tanTheta_CPSA_LR'
                                                       */
  real_T CPztotanTheta_CPSA_RR_tableData[15];         /* Expression: tanTheta_RR
                                                       * Referenced by: '<S87>/CPz to tanTheta_CPSA_RR'
                                                       */
  real_T CPztotanTheta_CPSA_RR_bp01Data[15];          /* Expression: LatWhlCtrDisp.RR(:, 1)
                                                       * Referenced by: '<S87>/CPz to tanTheta_CPSA_RR'
                                                       */
  real_T Constant_Value_i[2];                         /* Expression: AntiSwayEnByAxl
                                                       * Referenced by: '<S83>/Constant'
                                                       */
  real_T VehicleVehicleWheelOffset3_Value_d[2];       /* Expression: NumWhlsByAxl
                                                       * Referenced by: '<S78>/Vehicle Vehicle Wheel Offset3'
                                                       */
  real_T Constant_Value_l;                            /* Expression: 1
                                                       * Referenced by: '<S78>/Constant'
                                                       */
  real_T Gain_Gain_m;                                 /* Expression: 1/2
                                                       * Referenced by: '<S78>/Gain'
                                                       */
  real_T Gain1_Gain_l;                                /* Expression: -1
                                                       * Referenced by: '<S78>/Gain1'
                                                       */
  real_T Gain5_Gain_j;                                /* Expression: VerticalSign
                                                       * Referenced by: '<S77>/Gain5'
                                                       */
  real_T Switch1_Threshold;                           /* Expression: 0
                                                       * Referenced by: '<S88>/Switch1'
                                                       */
  real_T Gain_Gain_f;                                 /* Expression: -1
                                                       * Referenced by: '<S88>/Gain'
                                                       */
  real_T Gain4_Gain_gl;                               /* Expression: VerticalSign
                                                       * Referenced by: '<S77>/Gain4'
                                                       */
  real_T VehicleForceSign_Gain;                       /* Expression: -1
                                                       * Referenced by: '<S76>/Vehicle Force Sign'
                                                       */
  real_T Signconvention_Gain;                         /* Expression: -1
                                                       * Referenced by: '<S76>/Sign convention'
                                                       */
  real_T Constant_Value_h;                            /* Expression: 0
                                                       * Referenced by: '<S22>/Constant'
                                                       */
  real_T VehicleVehicleWheelOffset3_Value_b[2];       /* Expression: NumWhlsByAxl
                                                       * Referenced by: '<S25>/Vehicle Vehicle Wheel Offset3'
                                                       */
  real_T Constant_Value_j;                            /* Expression: 1
                                                       * Referenced by: '<S25>/Constant'
                                                       */
  real_T Gain_Gain_e;                                 /* Expression: 1/2
                                                       * Referenced by: '<S25>/Gain'
                                                       */
};

/* Parameters for system: '<S213>/Wheel to Body Transform' */
struct P_CoreSubsys_Chassis_gm_T_ {
  real_T Constant1_Value[2]; /* Expression: zeros(2, 1)
                              * Referenced by: '<S220>/Constant1'
                              */
  real_T Constant_Value[3];  /* Expression: [pi; 0; 0]
                              * Referenced by: '<S220>/Constant'
                              */
};

/* Parameters for system: '<S241>/Clutch' */
struct P_Clutch_Chassis_T_ {
  real_T Constant_Value;   /* Expression: 0
                            * Referenced by: '<S245>/Constant'
                            */
  real_T Constant_Value_l; /* Expression: 0
                            * Referenced by: '<S243>/Constant'
                            */
  real_T Constant1_Value;  /* Expression: 0
                            * Referenced by: '<S243>/Constant1'
                            */
  real_T Constant2_Value;  /* Expression: 0
                            * Referenced by: '<S243>/Constant2'
                            */
  real_T u_Gain;           /* Expression: -4
                            * Referenced by: '<S244>/-4'
                            */
  boolean_T yn_Y0;         /* Computed Parameter: yn_Y0
                            * Referenced by: '<S246>/yn'
                            */
  boolean_T yn_Y0_d;       /* Computed Parameter: yn_Y0_d
                            * Referenced by: '<S245>/yn'
                            */
  boolean_T UnitDelay_InitialCondition;
  /* Computed Parameter: UnitDelay_InitialCondition
   * Referenced by: '<S250>/Unit Delay'
   */
  boolean_T CombinatorialLogic_table[8];
  /* Computed Parameter: CombinatorialLogic_table
   * Referenced by: '<S250>/Combinatorial  Logic'
   */
};

/* Parameters for system: '<S237>/Clutch Scalar Parameters' */
struct P_CoreSubsys_Chassis_l_T_ {
  real_T Clutch_OmegaTol;       /* Mask Parameter: Clutch_OmegaTol
                                 * Referenced by: '<S241>/Clutch'
                                 */
  P_Clutch_Chassis_T sf_Clutch; /* '<S241>/Clutch' */
};

/* Parameters for system: '<S215>/Wheel to Body Transform' */
struct P_CoreSubsys_Chassis_gmy_T_ {
  real_T Constant_Value[3]; /* Expression: [pi; 0; 0]
                             * Referenced by: '<S257>/Constant'
                             */
};

/* Parameters (default storage) */
struct P_Chassis_T_ {
  struct_hFtzkwIyDan9Uw0DEd8s7D VEH; /* Variable: VEH
                                      * Referenced by:
                                      *  '<S93>/Estimated Unsprung Mass Orientation'
                                      *  '<S94>/vehdyncginert'
                                      *  '<S149>/Constant1'
                                      *  '<S230>/Integrator, Second-Order'
                                      *  '<S136>/Constant3'
                                      *  '<S136>/.5.*A.*Pabs.//R.//T'
                                      *  '<S239>/Memory'
                                      *  '<S241>/Clutch'
                                      */
  real_T AlgnTrqCambCompl[4];        /* Variable: AlgnTrqCambCompl
                                      * Referenced by: '<S38>/Constant11'
                                      */
  real_T AlgnTrqSteerCompl[4];       /* Variable: AlgnTrqSteerCompl
                                      * Referenced by: '<S71>/Constant12'
                                      */
  real_T CambVsSteerAng[4];          /* Variable: CambVsSteerAng
                                      * Referenced by: '<S43>/Constant3'
                                      */
  real_T CastVsSteerAng[4];          /* Variable: CastVsSteerAng
                                      * Referenced by: '<S52>/Constant6'
                                      */
  real_T LatCambCompl[4];            /* Variable: LatCambCompl
                                      * Referenced by: '<S40>/Constant2'
                                      */
  real_T LatSteerCompl[4];           /* Variable: LatSteerCompl
                                      * Referenced by: '<S73>/Constant2'
                                      */
  real_T LatWhlCtrComplLat[4];       /* Variable: LatWhlCtrComplLat
                                      * Referenced by: '<S58>/Constant2'
                                      */
  real_T NrmlWhlFrcOff[4];           /* Variable: NrmlWhlFrcOff
                                      * Referenced by: '<S90>/Constant1'
                                      */
  real_T NrmlWhlRates[4];            /* Variable: NrmlWhlRates
                                      * Referenced by: '<S90>/Constant5'
                                      */
  real_T RollStiffArb[2];            /* Variable: RollStiffArb
                                      * Referenced by: '<S86>/Constant3'
                                      */
  real_T RollStiffNoArb[2];          /* Variable: RollStiffNoArb
                                      * Referenced by: '<S86>/Constant4'
                                      */
  real_T StatCaster[4];              /* Variable: StatCaster
                                      * Referenced by: '<S27>/Constant7'
                                      */
  real_T CombinedSlipWheel2DOF_ALPMAX;
  /* Mask Parameter: CombinedSlipWheel2DOF_ALPMAX
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_ALPMIN;
  /* Mask Parameter: CombinedSlipWheel2DOF_ALPMIN
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T IndependentKandCSuspension_AxleNumVec[4];
  /* Mask Parameter: IndependentKandCSuspension_AxleNumVec
   * Referenced by: '<S9>/Axle Number'
   */
  real_T CombinedSlipWheel2DOF_BOTTOM_OFFST;
  /* Mask Parameter: CombinedSlipWheel2DOF_BOTTOM_OFFST
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_BOTTOM_STIFF;
  /* Mask Parameter: CombinedSlipWheel2DOF_BOTTOM_STIFF
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_BREFF;
  /* Mask Parameter: CombinedSlipWheel2DOF_BREFF
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_CAMMAX;
  /* Mask Parameter: CombinedSlipWheel2DOF_CAMMAX
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_CAMMIN;
  /* Mask Parameter: CombinedSlipWheel2DOF_CAMMIN
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T VehicleBody6DOF_Cd;                            /* Mask Parameter: VehicleBody6DOF_Cd
                                                         * Referenced by: '<S136>/Constant'
                                                         */
  real_T VehicleBody6DOF_Cl;                            /* Mask Parameter: VehicleBody6DOF_Cl
                                                         * Referenced by: '<S136>/Constant1'
                                                         */
  real_T VehicleBody6DOF_Cpm;                           /* Mask Parameter: VehicleBody6DOF_Cpm
                                                         * Referenced by: '<S136>/Constant2'
                                                         */
  real_T VehicleBody6DOF_Cs[tx_car::kMap1dSize];  /* Mask Parameter: VehicleBody6DOF_Cs
                                                         * Referenced by: '<S136>/Cs'
                                                         */
  real_T VehicleBody6DOF_Cym[tx_car::kMap1dSize]; /* Mask Parameter: VehicleBody6DOF_Cym
                                                         * Referenced by: '<S136>/Cym'
                                                         */
  real_T CombinedSlipWheel2DOF_DREFF;
  /* Mask Parameter: CombinedSlipWheel2DOF_DREFF
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_FNOMIN;
  /* Mask Parameter: CombinedSlipWheel2DOF_FNOMIN
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_FREFF;
  /* Mask Parameter: CombinedSlipWheel2DOF_FREFF
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_FZMAX;
  /* Mask Parameter: CombinedSlipWheel2DOF_FZMAX
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_FZMIN;
  /* Mask Parameter: CombinedSlipWheel2DOF_FZMIN
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_GRAVITY;
  /* Mask Parameter: CombinedSlipWheel2DOF_GRAVITY
   * Referenced by: '<S230>/Fg'
   */
  real_T CombinedSlipWheel2DOF_IYY; /* Mask Parameter: CombinedSlipWheel2DOF_IYY
                                     * Referenced by: '<S241>/Clutch'
                                     */
  real_T VehicleBody6DOF_Iveh[9];   /* Mask Parameter: VehicleBody6DOF_Iveh
                                     * Referenced by: '<S94>/vehdyncginert'
                                     */
  real_T CombinedSlipWheel2DOF_KPUMAX;
  /* Mask Parameter: CombinedSlipWheel2DOF_KPUMAX
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_KPUMIN;
  /* Mask Parameter: CombinedSlipWheel2DOF_KPUMIN
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_LATERAL_STIFFNESS;
  /* Mask Parameter: CombinedSlipWheel2DOF_LATERAL_STIFFNESS
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_LONGITUDINAL_STIFFNESS;
  /* Mask Parameter: CombinedSlipWheel2DOF_LONGITUDINAL_STIFFNESS
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_LONGVL;
  /* Mask Parameter: CombinedSlipWheel2DOF_LONGVL
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_MASS;
  /* Mask Parameter: CombinedSlipWheel2DOF_MASS
   * Referenced by:
   *   '<S230>/Fg'
   *   '<S230>/Gain1'
   */
  real_T CombinedSlipWheel2DOF_NOMPRES;
  /* Mask Parameter: CombinedSlipWheel2DOF_NOMPRES
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PCFX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PCFX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PCFX2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PCFX2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PCFX3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PCFX3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PCFY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PCFY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PCFY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PCFY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PCFY3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PCFY3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PCX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PCX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PCY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PCY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDX2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDX2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDX3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDX3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDXP1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDXP1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDXP2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDXP2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDXP3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDXP3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDY3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDY3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDYP1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDYP1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDYP2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDYP2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDYP3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDYP3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PDYP4;
  /* Mask Parameter: CombinedSlipWheel2DOF_PDYP4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PECP1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PECP1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PECP2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PECP2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PEX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PEX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PEX2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PEX2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PEX3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PEX3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PEX4;
  /* Mask Parameter: CombinedSlipWheel2DOF_PEX4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PEY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PEY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PEY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PEY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PEY3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PEY3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PEY4;
  /* Mask Parameter: CombinedSlipWheel2DOF_PEY4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PEY5;
  /* Mask Parameter: CombinedSlipWheel2DOF_PEY5
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PFZ1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PFZ1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PHX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PHX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PHX2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PHX2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PHY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PHY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PHY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PHY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PHYP1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PHYP1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PHYP2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PHYP2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PHYP3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PHYP3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PHYP4;
  /* Mask Parameter: CombinedSlipWheel2DOF_PHYP4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PKX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PKX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PKX2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PKX2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PKX3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PKX3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PKY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PKY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PKY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PKY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PKY3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PKY3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PKY4;
  /* Mask Parameter: CombinedSlipWheel2DOF_PKY4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PKY5;
  /* Mask Parameter: CombinedSlipWheel2DOF_PKY5
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PKY6;
  /* Mask Parameter: CombinedSlipWheel2DOF_PKY6
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PKY7;
  /* Mask Parameter: CombinedSlipWheel2DOF_PKY7
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PKYP1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PKYP1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPMX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPMX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPX2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPX2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPX3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPX3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPX4;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPX4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPY3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPY3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPY4;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPY4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPY5;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPY5
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPZ1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPZ1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PPZ2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PPZ2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PRESMAX;
  /* Mask Parameter: CombinedSlipWheel2DOF_PRESMAX
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PRESMIN;
  /* Mask Parameter: CombinedSlipWheel2DOF_PRESMIN
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PVX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PVX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PVX2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PVX2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PVY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_PVY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PVY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_PVY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PVY3;
  /* Mask Parameter: CombinedSlipWheel2DOF_PVY3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_PVY4;
  /* Mask Parameter: CombinedSlipWheel2DOF_PVY4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T VehicleBody6DOF_Pabs; /* Mask Parameter: VehicleBody6DOF_Pabs
                                * Referenced by: '<S136>/.5.*A.*Pabs.//R.//T'
                                */
  real_T CombinedSlipWheel2DOF_QBRP1;
  /* Mask Parameter: CombinedSlipWheel2DOF_QBRP1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QBZ1;
  /* Mask Parameter: CombinedSlipWheel2DOF_QBZ1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QBZ10;
  /* Mask Parameter: CombinedSlipWheel2DOF_QBZ10
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QBZ2;
  /* Mask Parameter: CombinedSlipWheel2DOF_QBZ2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QBZ3;
  /* Mask Parameter: CombinedSlipWheel2DOF_QBZ3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QBZ4;
  /* Mask Parameter: CombinedSlipWheel2DOF_QBZ4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QBZ5;
  /* Mask Parameter: CombinedSlipWheel2DOF_QBZ5
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QBZ6;
  /* Mask Parameter: CombinedSlipWheel2DOF_QBZ6
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QBZ9;
  /* Mask Parameter: CombinedSlipWheel2DOF_QBZ9
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QCRP1;
  /* Mask Parameter: CombinedSlipWheel2DOF_QCRP1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QCRP2;
  /* Mask Parameter: CombinedSlipWheel2DOF_QCRP2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QCZ1;
  /* Mask Parameter: CombinedSlipWheel2DOF_QCZ1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDRP1;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDRP1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDRP2;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDRP2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDTP1;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDTP1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDZ1;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDZ1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDZ10;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDZ10
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDZ11;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDZ11
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDZ2;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDZ2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDZ3;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDZ3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDZ4;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDZ4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDZ6;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDZ6
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDZ7;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDZ7
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDZ8;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDZ8
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QDZ9;
  /* Mask Parameter: CombinedSlipWheel2DOF_QDZ9
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QEZ1;
  /* Mask Parameter: CombinedSlipWheel2DOF_QEZ1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QEZ2;
  /* Mask Parameter: CombinedSlipWheel2DOF_QEZ2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QEZ3;
  /* Mask Parameter: CombinedSlipWheel2DOF_QEZ3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QEZ4;
  /* Mask Parameter: CombinedSlipWheel2DOF_QEZ4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QEZ5;
  /* Mask Parameter: CombinedSlipWheel2DOF_QEZ5
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QHZ1;
  /* Mask Parameter: CombinedSlipWheel2DOF_QHZ1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QHZ2;
  /* Mask Parameter: CombinedSlipWheel2DOF_QHZ2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QHZ3;
  /* Mask Parameter: CombinedSlipWheel2DOF_QHZ3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QHZ4;
  /* Mask Parameter: CombinedSlipWheel2DOF_QHZ4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX10;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX10
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX11;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX11
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX12;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX12
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX13;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX13
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX14;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX14
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX2;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX3;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX4;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX5;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX5
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX6;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX6
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX7;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX7
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX8;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX8
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSX9;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSX9
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSY3;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSY3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSY4;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSY4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSY5;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSY5
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSY6;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSY6
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSY7;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSY7
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_QSY8;
  /* Mask Parameter: CombinedSlipWheel2DOF_QSY8
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_FCX;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_FCX
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_FCY;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_FCY
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_FCY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_FCY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_FZ1;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_FZ1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_FZ2;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_FZ2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_FZ3;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_FZ3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_RA1;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_RA1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_RA2;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_RA2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_RB1;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_RB1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_RB2;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_RB2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_RE0;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_RE0
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_V1;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_V1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_Q_V2;
  /* Mask Parameter: CombinedSlipWheel2DOF_Q_V2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T DragForce_R; /* Mask Parameter: DragForce_R
                       * Referenced by: '<S136>/.5.*A.*Pabs.  // R.  // T'
                       */
  real_T CombinedSlipWheel2DOF_RBX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_RBX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RBX2;
  /* Mask Parameter: CombinedSlipWheel2DOF_RBX2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RBX3;
  /* Mask Parameter: CombinedSlipWheel2DOF_RBX3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RBY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_RBY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RBY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_RBY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RBY3;
  /* Mask Parameter: CombinedSlipWheel2DOF_RBY3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RBY4;
  /* Mask Parameter: CombinedSlipWheel2DOF_RBY4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RCX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_RCX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RCY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_RCY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_REX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_REX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_REX2;
  /* Mask Parameter: CombinedSlipWheel2DOF_REX2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_REY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_REY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_REY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_REY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RHX1;
  /* Mask Parameter: CombinedSlipWheel2DOF_RHX1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RHY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_RHY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RHY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_RHY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RIM_RADIUS;
  /* Mask Parameter: CombinedSlipWheel2DOF_RIM_RADIUS
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RVY1;
  /* Mask Parameter: CombinedSlipWheel2DOF_RVY1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RVY2;
  /* Mask Parameter: CombinedSlipWheel2DOF_RVY2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RVY3;
  /* Mask Parameter: CombinedSlipWheel2DOF_RVY3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RVY4;
  /* Mask Parameter: CombinedSlipWheel2DOF_RVY4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RVY5;
  /* Mask Parameter: CombinedSlipWheel2DOF_RVY5
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_RVY6;
  /* Mask Parameter: CombinedSlipWheel2DOF_RVY6
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_SSZ1;
  /* Mask Parameter: CombinedSlipWheel2DOF_SSZ1
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_SSZ2;
  /* Mask Parameter: CombinedSlipWheel2DOF_SSZ2
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_SSZ3;
  /* Mask Parameter: CombinedSlipWheel2DOF_SSZ3
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_SSZ4;
  /* Mask Parameter: CombinedSlipWheel2DOF_SSZ4
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T IndependentKandCSuspension_StatCamber[4];
  /* Mask Parameter: IndependentKandCSuspension_StatCamber
   * Referenced by: '<S26>/Constant4'
   */
  real_T IndependentKandCSuspension_StatToe[4];
  /* Mask Parameter: IndependentKandCSuspension_StatToe
   * Referenced by: '<S31>/Constant1'
   */
  real_T VehicleBody6DOF_Tair; /* Mask Parameter: VehicleBody6DOF_Tair
                                * Referenced by: '<S94>/AirTempConstant'
                                */
  real_T CombinedSlipWheel2DOF_UNLOADED_RADIUS;
  /* Mask Parameter: CombinedSlipWheel2DOF_UNLOADED_RADIUS
   * Referenced by:
   *   '<S229>/Magic Tire Const Input'
   *   '<S231>/Constant9'
   */
  real_T CombinedSlipWheel2DOF_VERTICAL_DAMPING;
  /* Mask Parameter: CombinedSlipWheel2DOF_VERTICAL_DAMPING
   * Referenced by: '<S230>/Gain2'
   */
  real_T CombinedSlipWheel2DOF_VERTICAL_STIFFNESS;
  /* Mask Parameter: CombinedSlipWheel2DOF_VERTICAL_STIFFNESS
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_VXLOW;
  /* Mask Parameter: CombinedSlipWheel2DOF_VXLOW
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T CombinedSlipWheel2DOF_WIDTH;
  /* Mask Parameter: CombinedSlipWheel2DOF_WIDTH
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T IndependentKandCSuspension_WhlNumVec[4];
  /* Mask Parameter: IndependentKandCSuspension_WhlNumVec
   * Referenced by: '<S9>/Wheel Number'
   */
  real_T VehicleBody6DOF_Xe_o[3];                          /* Mask Parameter: VehicleBody6DOF_Xe_o
                                                            * Referenced by: '<S109>/xe,ye,ze'
                                                            */
  real_T VehicleBody6DOF_beta_w[tx_car::kMap1dSize]; /* Mask Parameter: VehicleBody6DOF_beta_w
                                                            * Referenced by:
                                                            *   '<S136>/Cs'
                                                            *   '<S136>/Cym'
                                                            */
  real_T CombinedSlipWheel2DOF_br;                         /* Mask Parameter: CombinedSlipWheel2DOF_br
                                                            * Referenced by: '<S241>/Clutch'
                                                            */
  real_T CombinedSlipWheel2DOF_brake_n_bpt[tx_car::kMap1dSize];
  /* Mask Parameter: CombinedSlipWheel2DOF_brake_n_bpt
   * Referenced by: '<S240>/2-D Lookup Table'
   */
  real_T CombinedSlipWheel2DOF_brake_p_bpt[tx_car::kMap1dSize];
  /* Mask Parameter: CombinedSlipWheel2DOF_brake_p_bpt
   * Referenced by: '<S240>/2-D Lookup Table'
   */
  real_T CompareToConstant_const;  /* Mask Parameter: CompareToConstant_const
                                    * Referenced by: '<S142>/Constant'
                                    */
  real_T Pressure_const;           /* Mask Parameter: Pressure_const
                                    * Referenced by: '<S212>/Pressure'
                                    */
  real_T VehicleBody6DOF_d;        /* Mask Parameter: VehicleBody6DOF_d
                                    * Referenced by:
                                    *   '<S94>/vehdyncginert'
                                    *   '<S149>/Constant'
                                    */
  real_T VehicleBody6DOF_eul_o[3]; /* Mask Parameter: VehicleBody6DOF_eul_o
                                    * Referenced by: '<S115>/phi theta psi'
                                    */
  real_T CombinedSlipWheel2DOF_f_brake_t[tx_car::kMap2dSize];
  /* Mask Parameter: CombinedSlipWheel2DOF_f_brake_t
   * Referenced by: '<S240>/2-D Lookup Table'
   */
  real_T VehicleBody6DOF_g;       /* Mask Parameter: VehicleBody6DOF_g
                                   * Referenced by: '<S111>/g'
                                   */
  real_T VehicleBody6DOF_latOff;  /* Mask Parameter: VehicleBody6DOF_latOff
                                   * Referenced by: '<S154>/latOff'
                                   */
  real_T VehicleBody6DOF_longOff; /* Mask Parameter: VehicleBody6DOF_longOff
                                   * Referenced by: '<S154>/longOff'
                                   */
  real_T CombinedSlipWheel2DOF_mu_kinetic;
  /* Mask Parameter: CombinedSlipWheel2DOF_mu_kinetic
   * Referenced by:
   *   '<S238>/Ratio of static to kinetic'
   *   '<S238>/Ratio of static to kinetic1'
   */
  real_T CombinedSlipWheel2DOF_mu_static;
  /* Mask Parameter: CombinedSlipWheel2DOF_mu_static
   * Referenced by: '<S238>/Ratio of static to kinetic'
   */
  real_T VehicleBody6DOF_p_o[3]; /* Mask Parameter: VehicleBody6DOF_p_o
                                  * Referenced by: '<S109>/p,q,r '
                                  */
  real_T CombinedSlipWheel2DOF_turnslip;
  /* Mask Parameter: CombinedSlipWheel2DOF_turnslip
   * Referenced by: '<S229>/Magic Tire Const Input'
   */
  real_T VehicleBody6DOF_vertOff;    /* Mask Parameter: VehicleBody6DOF_vertOff
                                      * Referenced by: '<S154>/vertOff '
                                      */
  real_T ContLPF_wc;                 /* Mask Parameter: ContLPF_wc
                                      * Referenced by: '<S7>/Constant'
                                      */
  real_T ContLPF2_wc;                /* Mask Parameter: ContLPF2_wc
                                      * Referenced by: '<S8>/Constant'
                                      */
  real_T Bushings_wc;                /* Mask Parameter: Bushings_wc
                                      * Referenced by: '<S95>/Constant'
                                      */
  real_T ContLPF_wc_m;               /* Mask Parameter: ContLPF_wc_m
                                      * Referenced by: '<S210>/Constant'
                                      */
  real_T ContLPF1_wc;                /* Mask Parameter: ContLPF1_wc
                                      * Referenced by: '<S211>/Constant'
                                      */
  real_T VehicleBody6DOF_xbdot_o[3]; /* Mask Parameter: VehicleBody6DOF_xbdot_o
                                      * Referenced by: '<S109>/ub,vb,wb'
                                      */
  real_T VehicleBody6DOF_xdot_tol;   /* Mask Parameter: VehicleBody6DOF_xdot_tol
                                      * Referenced by:
                                      *   '<S145>/Constant'
                                      *   '<S146>/Constant'
                                      *   '<S181>/Constant'
                                      *   '<S182>/Constant'
                                      */
  real_T VehicleBody6DOF_z1I[9];     /* Mask Parameter: VehicleBody6DOF_z1I
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z1R[3];     /* Mask Parameter: VehicleBody6DOF_z1R
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z1m;        /* Mask Parameter: VehicleBody6DOF_z1m
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z2I[9];     /* Mask Parameter: VehicleBody6DOF_z2I
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z2R[3];     /* Mask Parameter: VehicleBody6DOF_z2R
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z2m;        /* Mask Parameter: VehicleBody6DOF_z2m
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z3I[9];     /* Mask Parameter: VehicleBody6DOF_z3I
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z3R[3];     /* Mask Parameter: VehicleBody6DOF_z3R
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z3m;        /* Mask Parameter: VehicleBody6DOF_z3m
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z4I[9];     /* Mask Parameter: VehicleBody6DOF_z4I
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z4R[3];     /* Mask Parameter: VehicleBody6DOF_z4R
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z4m;        /* Mask Parameter: VehicleBody6DOF_z4m
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z5I[9];     /* Mask Parameter: VehicleBody6DOF_z5I
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z5R[3];     /* Mask Parameter: VehicleBody6DOF_z5R
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z5m;        /* Mask Parameter: VehicleBody6DOF_z5m
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z6I[9];     /* Mask Parameter: VehicleBody6DOF_z6I
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z6R[3];     /* Mask Parameter: VehicleBody6DOF_z6R
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z6m;        /* Mask Parameter: VehicleBody6DOF_z6m
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z7I[9];     /* Mask Parameter: VehicleBody6DOF_z7I
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z7R[3];     /* Mask Parameter: VehicleBody6DOF_z7R
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T VehicleBody6DOF_z7m;        /* Mask Parameter: VehicleBody6DOF_z7m
                                      * Referenced by: '<S94>/vehdyncginert'
                                      */
  real_T CombinedSlipWheel2DOF_zdoto;
  /* Mask Parameter: CombinedSlipWheel2DOF_zdoto
   * Referenced by: '<S230>/Integrator, Second-Order'
   */
  real_T Out1_Y0;                            /* Computed Parameter: Out1_Y0
                                              * Referenced by: '<S143>/Out1'
                                              */
  real_T Constant_Value;                     /* Expression: 2*pi
                                              * Referenced by: '<S143>/Constant'
                                              */
  real_T UnitDelay_InitialCondition;         /* Expression: 0
                                              * Referenced by: '<S143>/Unit Delay'
                                              */
  real_T MagicTireConstInput_vdynMF[279];    /* Expression: vdynMF
                                              * Referenced by: '<S229>/Magic Tire Const Input'
                                              */
  real_T phithetapsi_WrappedStateUpperValue; /* Expression: pi
                                              * Referenced by: '<S115>/phi theta psi'
                                              */
  real_T phithetapsi_WrappedStateLowerValue; /* Expression: -pi
                                              * Referenced by: '<S115>/phi theta psi'
                                              */
  real_T Memory1_InitialCondition;           /* Expression: 0
                                              * Referenced by: '<S141>/Memory1'
                                              */
  real_T Integrator1_IC;                     /* Expression: 0
                                              * Referenced by: '<S95>/Integrator1'
                                              */
  real_T UnsprungMasses_Gain;
  /* Expression: sum([VEH.UnsprungMassFrontAxle/2 VEH.UnsprungMassFrontAxle/2 VEH.UnsprungMassRearAxle/2
   * VEH.UnsprungMassRearAxle/2]) Referenced by: '<S93>/Unsprung Masses'
   */
  real_T Vertical_Value;                       /* Expression: 0
                                                * Referenced by: '<S93>/Vertical'
                                                */
  real_T u_Value;                              /* Expression: 0
                                                * Referenced by: '<S111>/0'
                                                */
  real_T u_Gain[3];                            /* Expression: [4.*ones(2, 1); 0]
                                                * Referenced by: '<S136>/4'
                                                */
  real_T Constant4_Value[3];                   /* Expression: [0; 0; 1]
                                                * Referenced by: '<S136>/Constant4'
                                                */
  real_T Crm_tableData[2];                     /* Expression: [0 0]
                                                * Referenced by: '<S136>/Crm'
                                                */
  real_T Crm_bp01Data[2];                      /* Expression: [-1 1]
                                                * Referenced by: '<S136>/Crm'
                                                */
  real_T Integrator_IC;                        /* Expression: 0
                                                * Referenced by: '<S232>/Integrator'
                                                */
  real_T Integrator_IC_h;                      /* Expression: 0
                                                * Referenced by: '<S233>/Integrator'
                                                */
  real_T Integrator1_IC_d;                     /* Expression: 0
                                                * Referenced by: '<S210>/Integrator1'
                                                */
  real_T Saturation_UpperSat;                  /* Expression: inf
                                                * Referenced by: '<S4>/Saturation'
                                                */
  real_T Saturation_LowerSat;                  /* Expression: -10*9.81*2000
                                                * Referenced by: '<S4>/Saturation'
                                                */
  real_T Gain4_Gain[4];                        /* Expression: [1; -1; 1; -1] * 0 + 1
                                                * Referenced by: '<S215>/Gain4'
                                                */
  real_T Integrator_IC_a;                      /* Expression: 0
                                                * Referenced by: '<S235>/Integrator'
                                                */
  real_T Saturation_UpperSat_c;                /* Expression: inf
                                                * Referenced by: '<S230>/Saturation'
                                                */
  real_T Saturation_LowerSat_j;                /* Expression: 0
                                                * Referenced by: '<S230>/Saturation'
                                                */
  real_T Saturation_UpperSat_g;                /* Expression: inf
                                                * Referenced by: '<S231>/Saturation'
                                                */
  real_T Saturation_LowerSat_g;                /* Expression: 0
                                                * Referenced by: '<S231>/Saturation'
                                                */
  real_T DisallowNegativeBrakeTorque_UpperSat; /* Expression: inf
                                                * Referenced by: '<S240>/Disallow Negative Brake Torque'
                                                */
  real_T DisallowNegativeBrakeTorque_LowerSat; /* Expression: eps
                                                * Referenced by: '<S240>/Disallow Negative Brake Torque'
                                                */
  real_T Integrator1_IC_l;                     /* Expression: 0
                                                * Referenced by: '<S211>/Integrator1'
                                                */
  real_T Constant3_Value[4];                   /* Expression: ones(1, 4).*0
                                                * Referenced by: '<S256>/Constant3'
                                                */
  real_T SteerRates_Value[4];                  /* Expression: zeros(1, 4)
                                                * Referenced by: '<S10>/SteerRates'
                                                */
  real_T Constant_Value_o[4];                  /* Expression: zeros(1, 4)
                                                * Referenced by: '<S10>/Constant'
                                                */
  real_T Constant6_Value[36];                  /* Expression: repmat(eye(3), 1, 1, 4)
                                                * Referenced by: '<S1>/Constant6'
                                                */
  real_T ones2_Value[4];                       /* Expression: ones(1, numWheels)
                                                * Referenced by: '<S216>/ones2'
                                                */
  real_T u_Value_m[4];                         /* Expression: [zeros(1, numWheels)]
                                                * Referenced by: '<S216>/0'
                                                */
  real_T ones1_Value[36];                      /* Expression: [ones(9, numWheels)]
                                                * Referenced by: '<S216>/ones1'
                                                */
  real_T u_Value_j[4];                         /* Expression: [zeros(1, numWheels)]+1
                                                * Referenced by: '<S216>/2'
                                                */
  real_T ones3_Value[20];                      /* Expression: [ones(5, numWheels)]
                                                * Referenced by: '<S216>/ones3'
                                                */
  real_T u_Value_jv[8];                        /* Expression: [zeros(2, numWheels)]+1
                                                * Referenced by: '<S216>/1'
                                                */
  real_T ones_Value[24];                       /* Expression: [ones(6, numWheels)]
                                                * Referenced by: '<S216>/ones'
                                                */
  real_T Constant_Value_a[4];                  /* Expression: zeros(4, 1)
                                                * Referenced by: '<S218>/Constant'
                                                */
  real_T InertialFrameCGtoAxleOffset_Value[12];
  /* Expression: [zeros(2,4);VEH.HeightCG*ones(1, 4)]
   * Referenced by: '<S12>/Inertial Frame CG to Axle Offset'
   */
  real_T FrontTrack_Value;       /* Expression: w(1)
                                  * Referenced by: '<S14>/Front Track'
                                  */
  real_T Integrator1_IC_j;       /* Expression: 0
                                  * Referenced by: '<S7>/Integrator1'
                                  */
  real_T Integrator1_IC_g;       /* Expression: 0
                                  * Referenced by: '<S8>/Integrator1'
                                  */
  real_T Gain_Gain;              /* Expression: -1
                                  * Referenced by: '<S6>/Gain'
                                  */
  real_T RearTrack_Value;        /* Expression: w(2)
                                  * Referenced by: '<S15>/Rear Track'
                                  */
  real_T Constant1_Value[3];     /* Expression: [0; 0; 0]
                                  * Referenced by: '<S92>/Constant1'
                                  */
  real_T Constant1_Value_l[9];   /* Expression: zeros(3, 3)
                                  * Referenced by: '<S102>/Constant1'
                                  */
  real_T Constant_Value_g[12];   /* Expression: zeros(12, 1)
                                  * Referenced by: '<S94>/Constant'
                                  */
  real_T Integrator_IC_e;        /* Expression: 0
                                  * Referenced by: '<S113>/Integrator'
                                  */
  real_T Saturation1_UpperSat;   /* Expression: inf
                                  * Referenced by: '<S232>/Saturation1'
                                  */
  real_T Saturation1_LowerSat;   /* Expression: 1e-2
                                  * Referenced by: '<S232>/Saturation1'
                                  */
  real_T Saturation_UpperSat_d;  /* Expression: inf
                                  * Referenced by: '<S232>/Saturation'
                                  */
  real_T Saturation_LowerSat_f;  /* Expression: FxRelFreqLwrLim
                                  * Referenced by: '<S232>/Saturation'
                                  */
  real_T Saturation1_UpperSat_k; /* Expression: inf
                                  * Referenced by: '<S233>/Saturation1'
                                  */
  real_T Saturation1_LowerSat_f; /* Expression: 1e-2
                                  * Referenced by: '<S233>/Saturation1'
                                  */
  real_T Saturation_UpperSat_k;  /* Expression: inf
                                  * Referenced by: '<S233>/Saturation'
                                  */
  real_T Saturation_LowerSat_p;  /* Expression: FyRelFreqLwrLim
                                  * Referenced by: '<S233>/Saturation'
                                  */
  real_T Saturation1_UpperSat_g; /* Expression: inf
                                  * Referenced by: '<S235>/Saturation1'
                                  */
  real_T Saturation1_LowerSat_a; /* Expression: 1e-2
                                  * Referenced by: '<S235>/Saturation1'
                                  */
  real_T Saturation_UpperSat_o;  /* Expression: inf
                                  * Referenced by: '<S235>/Saturation'
                                  */
  real_T Saturation_LowerSat_o;  /* Expression: MyRelFreqLwrLim
                                  * Referenced by: '<S235>/Saturation'
                                  */
  real_T Constant_Value_d;       /* Expression: 0
                                  * Referenced by: '<S230>/Constant'
                                  */
  real_T Switch_Threshold;       /* Expression: 0
                                  * Referenced by: '<S230>/Switch'
                                  */
  uint32_T uDLookupTable_maxIndex[2];
  /* Computed Parameter: uDLookupTable_maxIndex
   * Referenced by: '<S240>/2-D Lookup Table'
   */
  P_CoreSubsys_Chassis_gmy_T CoreSubsys_pna; /* '<S215>/Wheel to Body Transform' */
  P_CoreSubsys_Chassis_l_T CoreSubsys_d0;    /* '<S237>/Clutch Scalar Parameters' */
  P_CoreSubsys_Chassis_gm_T CoreSubsys_pn;   /* '<S213>/Wheel to Body Transform' */
  P_CoreSubsys_Chassis_T CoreSubsys;
  /* '<S9>/For each track and axle combination calculate suspension forces and moments' */
};

/* Real-time Model Data Structure */
struct tag_RTM_Chassis_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_Chassis_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_Chassis_T *contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[65];
  real_T odeF[4][65];
  ODE4_IntgData intgData;

  /*
   * Sizes:
   * The following substructure contains sizes information
   * for many of the model attributes such as inputs, outputs,
   * dwork, sample times, etc.
   */
  struct {
    int_T numContStates;
    int_T numPeriodicContStates;
    int_T numSampTimes;
  } Sizes;

  /*
   * Timing:
   * The following substructure contains information regarding
   * the timing information for the model.
   */
  struct {
    uint32_T clockTick0;
    uint32_T clockTickH0;
    time_T stepSize0;
    uint32_T clockTick1;
    uint32_T clockTickH1;
    SimTimeStep simTimeStep;
    boolean_T stopRequestedFlag;
    time_T *t;
    time_T tArray[2];
  } Timing;
};

/* Class declaration for model Chassis */
class MODULE_API Chassis {
  /* public data and function members */
 public:
  /* Copy Constructor */
  Chassis(Chassis const &) = delete;

  /* Assignment Operator */
  Chassis &operator=(Chassis const &) & = delete;

  /* Move Constructor */
  Chassis(Chassis &&) = delete;

  /* Move Assignment Operator */
  Chassis &operator=(Chassis &&) = delete;

  /* Real-Time Model get method */
  RT_MODEL_Chassis_T *getRTM();

  /* Root inports set method */
  void setExternalInputs(const ExtU_Chassis_T *pExtU_Chassis_T) { Chassis_U = *pExtU_Chassis_T; }

  /* Root outports get method */
  const ExtY_Chassis_T &getExternalOutputs() const { return Chassis_Y; }

  void ModelPrevZCStateInit();

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  static void terminate();

  /* Constructor */
  Chassis();

  /* Destructor */
  ~Chassis();

 protected:
  /* Tunable parameters */
  static P_Chassis_T Chassis_P;

  // body
  /* beta_w vs cs map size */
  uint32_T m_beta_w_cs_maxIndex = 30;

  /* beta_w vs cym map size */
  uint32_T m_beta_w_cym_maxIndex = 30;

  // suspension
  uint32_t m_shckdamping_vs_comprate_maxIndex = 11;
  uint32_t m_bumptoe_vs_compress_maxIndex = 14;
  uint32_t m_bumpcamber_vs_compress_maxIndex = 14;
  uint32_t m_bumpcaster_vs_compress_maxIndex = 14;
  uint32_t m_latdisp_vs_compress_maxIndex = 14;
  uint32_t m_lngdisp_vs_compress_maxIndex = 14;

  uint32_t m_shckdamping_vs_comprate_maxIndex_rear = 11;
  uint32_t m_bumptoe_vs_compress_maxIndex_rear = 14;
  uint32_t m_bumpcamber_vs_compress_maxIndex_rear = 14;
  uint32_t m_bumpcaster_vs_compress_maxIndex_rear = 14;
  uint32_t m_latdisp_vs_compress_maxIndex_rear = 14;
  uint32_t m_lngdisp_vs_compress_maxIndex_rear = 14;

  // mapped brake
  uint32_t m_mapped_brake_pressure_maxIndex = 18;
  uint32_t m_mapped_brake_wheelspeed_maxIndex = 19;
  uint32_t m_mapped_brake_torque_maxIndex = 379;

  /* private data and function members */
 private:
  /* External inputs */
  ExtU_Chassis_T Chassis_U;

  /* External outputs */
  ExtY_Chassis_T Chassis_Y;

  /* Block signals */
  B_Chassis_T Chassis_B;

  /* Block states */
  DW_Chassis_T Chassis_DW;

  /* Tunable parameters */
  // static P_Chassis_T Chassis_P;

  /* Block continuous states */
  X_Chassis_T Chassis_X;

  /* Triggered events */
  PrevZCX_Chassis_T Chassis_PrevZCX;
  PeriodicIndX_Chassis_T Chassis_PeriodicIndX; /* Block periodic continuous states */
  PeriodicRngX_Chassis_T Chassis_PeriodicRngX;

  /* private member function(s) for subsystem '<S241>/Clutch'*/
  static void Chassis_Clutch_Init(real_T rtp_omegao, B_Clutch_Chassis_T *localB, DW_Clutch_Chassis_T *localDW,
                                  P_Clutch_Chassis_T *localP, X_Clutch_Chassis_T *localX);
  void Chassis_Clutch_Deriv(B_Clutch_Chassis_T *localB, DW_Clutch_Chassis_T *localDW, XDot_Clutch_Chassis_T *localXdot);
  void Chassis_Clutch(real_T rtu_Tout, real_T rtu_Tfmaxs, real_T rtu_Tfmaxk, real_T rtp_omegao, real_T rtp_br,
                      real_T rtp_Iyy, real_T rtp_OmegaTol, B_Clutch_Chassis_T *localB, DW_Clutch_Chassis_T *localDW,
                      P_Clutch_Chassis_T *localP, X_Clutch_Chassis_T *localX);
  boolean_T Chassis_detectSlip(real_T Tout, real_T Tfmaxs, B_Clutch_Chassis_T *localB);
  boolean_T Chassis_detectLockup(real_T Tout, real_T Tfmaxs, real_T rtp_br, B_Clutch_Chassis_T *localB,
                                 DW_Clutch_Chassis_T *localDW, P_Clutch_Chassis_T *localP);

  /* private member function(s) for subsystem '<Root>'*/
  void Chassis_div0protect(const real_T u[4], real_T tol, real_T y[4], real_T yabs[4]);
  void Chassis_binary_expand_op(real_T in1_data[], int32_T in1_size[2], real_T in2, int32_T in3, real_T in4,
                                int32_T in5, real_T in6, int32_T in7);
  void Chassis_binary_expand_op_b3i5z2tw(real_T in1_data[], int32_T in1_size[2], real_T in2, int32_T in3, real_T in4,
                                         int32_T in5, real_T in6, int32_T in7, real_T in8, int32_T in9);
  void Chassis_sqrt(real_T x_data[], const int32_T x_size[2]);
  void Chassis_abs(const real_T x_data[], const int32_T *x_size, real_T y_data[], int32_T *y_size);
  void Chassis_binary_expand_op_b3i5z2t(real_T in1_data[], int32_T *in1_size, const real_T in2[4],
                                        const int8_T in3_data[], const int32_T *in3_size, const real_T in4[4],
                                        const real_T in5_data[], const int32_T *in5_size, const real_T in6[108],
                                        const int8_T in7_data[], const int32_T *in7_size);
  void Chassis_binary_expand_op_b3i5z2(real_T in1_data[], int32_T *in1_size, const real_T in2[4],
                                       const int8_T in3_data[], const int32_T *in3_size, const real_T in4[4],
                                       const int8_T in5_data[], const int32_T *in5_size, real_T in6);
  void Chassis_div0protect_p(const real_T u[4], real_T y[4]);
  boolean_T Chassis_any(const boolean_T x[4]);
  void Chassis_atan(real_T x_data[], const int32_T *x_size);
  void Chassis_cos(real_T x_data[], const int32_T *x_size);
  void Chassis_sqrt_c(real_T x_data[], const int32_T *x_size);
  void Chassis_binary_expand_op_b3i5z(real_T in1_data[], int32_T *in1_size, real_T in2, real_T in3, const real_T in4[4],
                                      const real_T in5[4], const int8_T in6_data[], const int32_T *in6_size, real_T in7,
                                      const real_T in8[4], const int8_T in9_data[], const int32_T *in9_size,
                                      real_T in10, const real_T in11_data[], const int32_T *in11_size);
  void Chassis_magicsin(const real_T D[4], real_T C, const real_T B[4], real_T E, const real_T u[4], real_T y[4]);
  void Chassis_magiccos(real_T C, const real_T B[4], const real_T E[4], const real_T u[4], real_T y[4]);
  void Chassis_binary_expand_op_b3i5(real_T in1[36], const int8_T in2_data[], const int32_T *in2_size,
                                     const real_T in3[4], const int8_T in4_data[], const int32_T *in4_size,
                                     const real_T in5[4], const int8_T in6_data[], const int32_T *in6_size,
                                     const real_T in7[4]);
  void Chassis_binary_expand_op_b3i(real_T in1_data[], int32_T *in1_size, const real_T in2[4], const real_T in3[4],
                                    const real_T in4[4], const int8_T in5_data[], const int32_T *in5_size,
                                    const real_T in6[4], const int8_T in7_data[], const int32_T *in7_size,
                                    const real_T in8[4]);
  void Chassis_acos(real_T x_data[], const int32_T *x_size);
  void Chassis_binary_expand_op_b3(real_T in1_data[], int32_T *in1_size, const real_T in2_data[],
                                   const int32_T *in2_size, const real_T in3[4], const int8_T in4_data[],
                                   const int32_T *in4_size, const real_T in5[4], const int8_T in6_data[],
                                   const int32_T *in6_size, const real_T in7[4]);
  void Chassis_magiccos_n(real_T C, const real_T B[4], const real_T E[4], real_T y[4]);
  void Chassis_magicsin_g(const real_T D[4], const real_T C[4], const real_T B[4], const real_T E[4], const real_T u[4],
                          real_T y[4]);
  void Chassis_binary_expand_op_b(real_T in1_data[], int32_T *in1_size, const real_T in2_data[],
                                  const int32_T *in2_size, const real_T in3[4], real_T in4, real_T in5,
                                  const real_T in6[4], real_T in7, const real_T in8[4], const real_T in9[108],
                                  const int8_T in10_data[], const int32_T *in10_size, const real_T in11[4],
                                  const real_T in12[4], const int8_T in13_data[], const int32_T *in13_size,
                                  const real_T in14[4], const real_T in15[36], const int8_T in16_data[],
                                  const int32_T *in16_size, const int8_T in17_data[], const int32_T *in17_size);
  void Chassis_rollingMoment(const real_T Fx[4], const real_T Vcx[4], const real_T Fz[4], const real_T press[4],
                             const real_T b_gamma[4], real_T Vo, real_T Ro, real_T Fzo, real_T pio, real_T b_QSY1,
                             real_T b_QSY2, real_T b_QSY3, real_T b_QSY4, real_T b_QSY5, real_T b_QSY6, real_T b_QSY7,
                             real_T b_QSY8, const real_T lam_My[4], real_T My[4]);
  void Chassis_magiccos_ngy(const real_T D[4], real_T C, const real_T B[4], const real_T E[4], const real_T u[4],
                            real_T y[4]);
  void Chassis_magiccos_ng(const real_T D[4], const real_T C[4], const real_T B[4], const real_T u[4], real_T y[4]);
  void Chassis_vdyncsmtire(
      const real_T Omega[4], const real_T Vx[4], const real_T Vy[4], const real_T psidot[4], real_T b_gamma[4],
      real_T press[4], const real_T scaleFactors[108], const real_T rhoz[4], real_T b_turnslip, real_T b_PRESMAX,
      real_T b_PRESMIN, real_T b_FZMAX, real_T b_FZMIN, real_T b_VXLOW, real_T b_KPUMAX, real_T b_KPUMIN,
      real_T b_ALPMAX, real_T b_ALPMIN, real_T b_CAMMIN, real_T b_CAMMAX, real_T b_LONGVL, real_T b_UNLOADED_RADIUS,
      real_T b_RIM_RADIUS, real_T b_NOMPRES, real_T b_FNOMIN, real_T b_VERTICAL_STIFFNESS, real_T b_DREFF,
      real_T b_BREFF, real_T b_FREFF, real_T b_Q_RE0, real_T b_Q_V1, real_T b_Q_V2, real_T b_Q_FZ1, real_T b_Q_FZ2,
      real_T b_Q_FCX, real_T b_Q_FCY, real_T b_PFZ1, real_T b_Q_FCY2, real_T b_BOTTOM_OFFST, real_T b_BOTTOM_STIFF,
      real_T b_PCX1, real_T b_PDX1, real_T b_PDX2, real_T b_PDX3, real_T b_PEX1, real_T b_PEX2, real_T b_PEX3,
      real_T b_PEX4, real_T b_PKX1, real_T b_PKX2, real_T b_PKX3, real_T b_PHX1, real_T b_PHX2, real_T b_PVX1,
      real_T b_PVX2, real_T b_PPX1, real_T b_PPX2, real_T b_PPX3, real_T b_PPX4, real_T b_RBX1, real_T b_RBX2,
      real_T b_RBX3, real_T b_RCX1, real_T b_REX1, real_T b_REX2, real_T b_QSX2, real_T b_QSX3, real_T b_QSX4,
      real_T b_QSX5, real_T b_QSX6, real_T b_QSX7, real_T b_QSX8, real_T b_QSX9, real_T b_QSX10, real_T b_QSX11,
      real_T b_PPMX1, real_T b_PCY1, real_T b_PDY1, real_T b_PDY2, real_T b_PDY3, real_T b_PEY1, real_T b_PEY2,
      real_T b_PEY4, real_T b_PEY5, real_T b_PKY1, real_T b_PKY2, real_T b_PKY3, real_T b_PKY4, real_T b_PKY5,
      real_T b_PKY6, real_T b_PKY7, real_T b_PVY3, real_T b_PVY4, real_T b_PPY1, real_T b_PPY2, real_T b_PPY3,
      real_T b_PPY4, real_T b_PPY5, real_T b_RBY1, real_T b_RBY2, real_T b_RBY4, real_T b_RCY1, real_T b_REY1,
      real_T b_REY2, real_T b_RHY1, real_T b_RHY2, real_T b_RVY3, real_T b_RVY4, real_T b_RVY5, real_T b_RVY6,
      real_T b_QSY1, real_T b_QSY2, real_T b_QSY3, real_T b_QSY4, real_T b_QSY5, real_T b_QSY6, real_T b_QSY7,
      real_T b_QSY8, real_T b_QBZ1, real_T b_QBZ2, real_T b_QBZ3, real_T b_QBZ5, real_T b_QBZ6, real_T b_QBZ9,
      real_T b_QBZ10, real_T b_QCZ1, real_T b_QDZ1, real_T b_QDZ2, real_T b_QDZ4, real_T b_QDZ8, real_T b_QDZ9,
      real_T b_QDZ10, real_T b_QDZ11, real_T b_QEZ1, real_T b_QEZ2, real_T b_QEZ3, real_T b_QEZ5, real_T b_QHZ3,
      real_T b_QHZ4, real_T b_PPZ1, real_T b_PPZ2, real_T b_SSZ2, real_T b_SSZ3, real_T b_SSZ4, real_T b_PDXP1,
      real_T b_PDXP2, real_T b_PDXP3, real_T b_PKYP1, real_T b_PDYP1, real_T b_PDYP2, real_T b_PDYP3, real_T b_PDYP4,
      real_T b_PHYP1, real_T b_PHYP2, real_T b_PHYP3, real_T b_PHYP4, real_T b_PECP1, real_T b_PECP2, real_T b_QDTP1,
      real_T b_QCRP1, real_T b_QCRP2, real_T b_QBRP1, real_T b_QDRP1, real_T b_QDRP2, real_T b_WIDTH, real_T b_Q_RA1,
      real_T b_Q_RA2, real_T b_Q_RB1, real_T b_Q_RB2, real_T b_QSX12, real_T b_QSX13, real_T b_QSX14, real_T b_Q_FZ3,
      real_T b_LONGITUDINAL_STIFFNESS, real_T b_LATERAL_STIFFNESS, real_T b_PCFX1, real_T b_PCFX2, real_T b_PCFX3,
      real_T b_PCFY1, real_T b_PCFY2, real_T b_PCFY3, const real_T Fx_ext[4], const real_T Fy_ext[4], real_T Fx[4],
      real_T Fy[4], real_T Fz[4], real_T Mx[4], real_T My[4], real_T Mz[4], real_T Re[4], real_T kappa[4],
      real_T alpha[4], real_T sig_x[4], real_T sig_y[4], real_T patch_a[4], real_T patch_b[4]);

  /* Continuous states update member function*/
  void rt_ertODEUpdateContinuousStates(RTWSolverInfo *si);

  /* Derivatives member function */
  void Chassis_derivatives();

  /* Real-Time Model */
  RT_MODEL_Chassis_T Chassis_M;
};

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'Chassis'
 * '<S1>'   : 'Chassis/Chassis'
 * '<S2>'   : 'Chassis/Chassis/Suspension'
 * '<S3>'   : 'Chassis/Chassis/Vehicle'
 * '<S4>'   : 'Chassis/Chassis/Wheels and Tires'
 * '<S5>'   : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension'
 * '<S6>'   : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Calculate dynamic track width
 * from lateral  wheel center displacement change'
 * '<S7>'   : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Cont LPF'
 * '<S8>'   : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Cont LPF2'
 * '<S9>'   : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension'
 * '<S10>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Steer Rate Adapter'
 * '<S11>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Steering Adapter'
 * '<S12>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Vehicle Adapter'
 * '<S13>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Wheel  Adapter'
 * '<S14>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Calculate dynamic track width
 * from lateral  wheel center displacement change/Front Track Calculation'
 * '<S15>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Calculate dynamic track width
 * from lateral  wheel center displacement change/Rear Track Calculation'
 * '<S16>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/Anti-Sway Force'
 * '<S17>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments'
 * '<S18>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/Anti-Sway Force/No Op Anti-Sway Force'
 * '<S19>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension'
 * '<S20>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Steering Delta
 * Select'
 * '<S21>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations'
 * '<S22>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Vehicle Moments
 * From X and Y Forces'
 * '<S23>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic'
 * '<S24>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled'
 * '<S25>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Adjust Camber Sign For Track'
 * '<S26>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects'
 * '<S27>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Caster Kinematic and Compliance Effects'
 * '<S28>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Convert Steer Angle to Toe Angle'
 * '<S29>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Lateral Wheel Center Displacement Effects'
 * '<S30>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Longitudinal Wheel Center Displacement Effects'
 * '<S31>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects'
 * '<S32>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Aligning Torque
 * Camber Compliance'
 * '<S33>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Bump Camber'
 * '<S34>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Lateral Camber
 * Compliance'
 * '<S35>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Longitudinal Camber
 * Compliance'
 * '<S36>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Roll Camber'
 * '<S37>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Steer Camber'
 * '<S38>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Aligning Torque
 * Camber Compliance/Aligning Torque Camber Compliance_Gradient'
 * '<S39>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Bump Camber/Bump
 * Camber_XYData'
 * '<S40>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Lateral Camber
 * Compliance/Lateral Camber Compliance_Gradient'
 * '<S41>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Longitudinal Camber
 * Compliance/Longitudinal Camber Compliance_Gradient'
 * '<S42>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Roll Camber/Inactive'
 * '<S43>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Steer Camber/Steer
 * Camber_Gradient'
 * '<S44>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Camber Kinematic and Compliance Effects/Steer Camber/Steer
 * Camber_Gradient/Select Steer Camber Slope'
 * '<S45>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Caster Kinematic and Compliance Effects/Bump Caster'
 * '<S46>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Caster Kinematic and Compliance Effects/Longitudinal Caster
 * Compliance'
 * '<S47>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Caster Kinematic and Compliance Effects/Roll Caster'
 * '<S48>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Caster Kinematic and Compliance Effects/Steer Caster'
 * '<S49>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Caster Kinematic and Compliance Effects/Bump Caster/Bump
 * Caster_XYData'
 * '<S50>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Caster Kinematic and Compliance Effects/Longitudinal Caster
 * Compliance/Longitudinal Caster Compliance_Gradient'
 * '<S51>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Caster Kinematic and Compliance Effects/Roll Caster/Inactive'
 * '<S52>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Caster Kinematic and Compliance Effects/Steer Caster/Steer
 * Caster_Gradient'
 * '<S53>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Caster Kinematic and Compliance Effects/Steer Caster/Steer
 * Caster_Gradient/Select Steer Caster Slope'
 * '<S54>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Lateral Wheel Center Displacement Effects/Bump Lateral Wheel
 * Displacement'
 * '<S55>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Lateral Wheel Center Displacement Effects/Lateral Wheel
 * Compliance'
 * '<S56>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Lateral Wheel Center Displacement Effects/Lateral Wheel
 * Compliance from Fx'
 * '<S57>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Lateral Wheel Center Displacement Effects/Bump Lateral Wheel
 * Displacement/Bump Lateral Wheel Displacement_XYData'
 * '<S58>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Lateral Wheel Center Displacement Effects/Lateral Wheel
 * Compliance/Lateral Wheel Compliance_Gradient'
 * '<S59>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Lateral Wheel Center Displacement Effects/Lateral Wheel
 * Compliance from Fx/Lateral Wheel Compliance from Fx_Gradient'
 * '<S60>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Longitudinal Wheel Center Displacement Effects/Bump
 * Longitudinal Wheel Displacement'
 * '<S61>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Longitudinal Wheel Center Displacement Effects/Longitudinal
 * Wheel Compliance'
 * '<S62>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Longitudinal Wheel Center Displacement Effects/Bump
 * Longitudinal Wheel Displacement/Bump Longitudinal Wheel Displacement_XYData'
 * '<S63>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Longitudinal Wheel Center Displacement Effects/Longitudinal
 * Wheel Compliance/Longitudinal Wheel Compliance_Gradient'
 * '<S64>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Adjust Toe Sign For
 * Track'
 * '<S65>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Aligning Torque Steer
 * Compliance'
 * '<S66>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Bump Steer'
 * '<S67>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Lateral Steer
 * Compliance'
 * '<S68>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Longitudinal Steer
 * Compliance'
 * '<S69>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Roll Steer'
 * '<S70>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Select Static Toe
 * Setting'
 * '<S71>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Aligning Torque Steer
 * Compliance/Aligning Torque Steer Compliance_Gradient'
 * '<S72>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Bump Steer/Bump
 * Steer_XYData'
 * '<S73>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Lateral Steer
 * Compliance/Lateral Steer Compliance_Gradient'
 * '<S74>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Longitudinal Steer
 * Compliance/Longitudinal Steer Compliance_Gradient'
 * '<S75>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Suspension Angle
 * Calculations/Kinematics and Compliance Steering Enabled/Steer Kinematic and Compliance Effects/Roll Steer/Inactive'
 * '<S76>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension'
 * '<S77>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C'
 * '<S78>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Anti-roll Bar
 * Force'
 * '<S79>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Lateral Vertical
 * Load Transfer Effects'
 * '<S80>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Longitudinal Side
 * View Swing Arm Anti-Effects'
 * '<S81>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Shock Force'
 * '<S82>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Wheel Rate'
 * '<S83>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Anti-roll Bar
 * Force/Select ARB Front and Rear Stiffnesses'
 * '<S84>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Anti-roll Bar
 * Force/Select By Axle'
 * '<S85>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Anti-roll Bar
 * Force/Select ARB Front and Rear Stiffnesses/Arb Roll Stiffness'
 * '<S86>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Anti-roll Bar
 * Force/Select ARB Front and Rear Stiffnesses/Arb Roll Stiffness/Independent Front And Rear'
 * '<S87>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Lateral Vertical
 * Load Transfer Effects/Using CPy vs CPz curve'
 * '<S88>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Longitudinal Side
 * View Swing Arm Anti-Effects/Anti-Effect_XYData'
 * '<S89>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Shock Force/Table
 * Based Shock Force_Individual'
 * '<S90>'  : 'Chassis/Chassis/Suspension/Kinematics and Compliance Independent Suspension/Independent K and C
 * Suspension/For each track and axle combination calculate suspension forces and moments/Suspension/Z axis suspension
 * characteristic/Kinematics and Compliance Suspension/Constrained spring damper combination for K&C/Wheel Rate/Wheel
 * Rate_Gradient'
 * '<S91>'  : 'Chassis/Chassis/Vehicle/Variant Subsystem'
 * '<S92>'  : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF'
 * '<S93>'  : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Suspension to Chasiss Transforms'
 * '<S94>'  : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF'
 * '<S95>'  : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Suspension to Chasiss Transforms/Bushings'
 * '<S96>'  : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Suspension to Chasiss Transforms/Estimated Unsprung Mass
 * Orientation'
 * '<S97>'  : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Suspension to Chasiss Transforms/Susp2Body'
 * '<S98>'  : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Suspension to Chasiss Transforms/Susp2Body/Rotation Angles
 * to Direction Cosine Matrix'
 * '<S99>'  : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Suspension to Chasiss Transforms/Susp2Body/transform to
 * Inertial axes '
 * '<S100>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Suspension to Chasiss Transforms/Susp2Body/transform to
 * Inertial axes 1'
 * '<S101>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Suspension to Chasiss Transforms/Susp2Body/Rotation Angles
 * to Direction Cosine Matrix/Create 3x3 Matrix'
 * '<S102>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body'
 * '<S103>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Forces'
 * '<S104>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Moment Calc'
 * '<S105>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Moments'
 * '<S106>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing'
 * '<S107>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Susp2Chassis'
 * '<S108>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/vehdyncginert'
 * '<S109>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)'
 * '<S110>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/Aero Drag'
 * '<S111>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/Gravity'
 * '<S112>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/Power
 * Calculations'
 * '<S113>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection'
 * '<S114>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/Wheel to CG'
 * '<S115>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Calculate DCM & Euler Angles'
 * '<S116>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Calculate omega_dot'
 * '<S117>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Determine Force,  Mass & Inertia'
 * '<S118>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Vbxw'
 * '<S119>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Velocity Conversion'
 * '<S120>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Velocity Conversion1'
 * '<S121>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Velocity Conversion2'
 * '<S122>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/transform to Inertial axes '
 * '<S123>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Calculate DCM & Euler Angles/Rotation Angles to Direction Cosine Matrix'
 * '<S124>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Calculate DCM & Euler Angles/phidot thetadot psidot'
 * '<S125>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Calculate DCM & Euler Angles/Rotation Angles to Direction Cosine Matrix/Create 3x3 Matrix'
 * '<S126>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Calculate omega_dot/3x3 Cross Product'
 * '<S127>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Calculate omega_dot/I x w'
 * '<S128>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Calculate omega_dot/I x w1'
 * '<S129>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Calculate omega_dot/3x3 Cross Product/Subsystem'
 * '<S130>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Calculate omega_dot/3x3 Cross Product/Subsystem1'
 * '<S131>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Determine Force,  Mass & Inertia/Mass input//output  momentum'
 * '<S132>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Determine Force,  Mass & Inertia/Mass input//output  momentum/For Each Subsystem'
 * '<S133>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Vbxw/Subsystem'
 * '<S134>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/6DOF (Euler
 * Angles)/Vbxw/Subsystem1'
 * '<S135>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/Aero Drag/Two
 * Axle Vehicle'
 * '<S136>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle Body/Aero Drag/Two
 * Axle Vehicle/Drag Force'
 * '<S137>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/Angle Wrap'
 * '<S138>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/Body Slip'
 * '<S139>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/phidot thetadot psidot'
 * '<S140>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/wxR'
 * '<S141>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/Angle Wrap/Unwrap'
 * '<S142>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/Angle Wrap/Unwrap/Compare To Constant'
 * '<S143>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/Angle Wrap/Unwrap/Function-Call Subsystem'
 * '<S144>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/Body Slip/div0protect - abs poly'
 * '<S145>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/Body Slip/div0protect - abs poly/Compare To Constant'
 * '<S146>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/Body Slip/div0protect - abs poly/Compare To Constant1'
 * '<S147>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/wxR/Subsystem'
 * '<S148>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/6 DOF Generic Vehicle
 * Body/SignalCollection/wxR/Subsystem1'
 * '<S149>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Moment Calc/Hitch moments from forces'
 * '<S150>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Moment Calc/Hitch moments from
 * forces/hitch geometry parameters'
 * '<S151>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Moment Calc/Hitch moments from
 * forces/hitch geometry parameters/hitch inactive'
 * '<S152>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Left'
 * '<S153>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Right'
 * '<S154>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric'
 * '<S155>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Hitch'
 * '<S156>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Left'
 * '<S157>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Right'
 * '<S158>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Left/Hard Point Coordinate Transform External Displacement'
 * '<S159>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Left/Hard Point Coordinate Transform External Displacement/Rotation Angles to Direction Cosine
 * Matrix'
 * '<S160>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Left/Hard Point Coordinate Transform External Displacement/transform to Inertial axes'
 * '<S161>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Left/Hard Point Coordinate Transform External Displacement/transform to Inertial axes1'
 * '<S162>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Left/Hard Point Coordinate Transform External Displacement/wxR'
 * '<S163>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Left/Hard Point Coordinate Transform External Displacement/Rotation Angles to Direction Cosine
 * Matrix/Create 3x3 Matrix'
 * '<S164>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Left/Hard Point Coordinate Transform External Displacement/wxR/Subsystem'
 * '<S165>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Left/Hard Point Coordinate Transform External Displacement/wxR/Subsystem1'
 * '<S166>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Right/Hard Point Coordinate Transform External Displacement'
 * '<S167>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Right/Hard Point Coordinate Transform External Displacement/Rotation Angles to Direction Cosine
 * Matrix'
 * '<S168>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Right/Hard Point Coordinate Transform External Displacement/transform to Inertial axes'
 * '<S169>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Right/Hard Point Coordinate Transform External Displacement/transform to Inertial axes1'
 * '<S170>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Right/Hard Point Coordinate Transform External Displacement/wxR'
 * '<S171>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Right/Hard Point Coordinate Transform External Displacement/Rotation Angles to Direction Cosine
 * Matrix/Create 3x3 Matrix'
 * '<S172>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Right/Hard Point Coordinate Transform External Displacement/wxR/Subsystem'
 * '<S173>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Front Right/Hard Point Coordinate Transform External Displacement/wxR/Subsystem1'
 * '<S174>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta'
 * '<S175>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta/Body Slip'
 * '<S176>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta/Rotation Angles to Direction Cosine
 * Matrix'
 * '<S177>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta/transform to Inertial axes'
 * '<S178>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta/transform to Inertial axes1'
 * '<S179>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta/wxR'
 * '<S180>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta/Body Slip/div0protect - abs poly'
 * '<S181>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta/Body Slip/div0protect - abs
 * poly/Compare To Constant'
 * '<S182>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta/Body Slip/div0protect - abs
 * poly/Compare To Constant1'
 * '<S183>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta/Rotation Angles to Direction Cosine
 * Matrix/Create 3x3 Matrix'
 * '<S184>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta/wxR/Subsystem'
 * '<S185>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Geometric/Hard Point Coordinate Transform External Displacement Beta/wxR/Subsystem1'
 * '<S186>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Hitch/Hard Point Coordinate Transform External Displacement'
 * '<S187>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Hitch/Hard Point Coordinate Transform External Displacement/Rotation Angles to Direction Cosine Matrix'
 * '<S188>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Hitch/Hard Point Coordinate Transform External Displacement/transform to Inertial axes'
 * '<S189>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Hitch/Hard Point Coordinate Transform External Displacement/transform to Inertial axes1'
 * '<S190>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Hitch/Hard Point Coordinate Transform External Displacement/wxR'
 * '<S191>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Hitch/Hard Point Coordinate Transform External Displacement/Rotation Angles to Direction Cosine
 * Matrix/Create 3x3 Matrix'
 * '<S192>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Hitch/Hard Point Coordinate Transform External Displacement/wxR/Subsystem'
 * '<S193>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Hitch/Hard Point Coordinate Transform External Displacement/wxR/Subsystem1'
 * '<S194>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Left/Hard Point Coordinate Transform External Displacement'
 * '<S195>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Left/Hard Point Coordinate Transform External Displacement/Rotation Angles to Direction Cosine Matrix'
 * '<S196>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Left/Hard Point Coordinate Transform External Displacement/transform to Inertial axes'
 * '<S197>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Left/Hard Point Coordinate Transform External Displacement/transform to Inertial axes1'
 * '<S198>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Left/Hard Point Coordinate Transform External Displacement/wxR'
 * '<S199>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Left/Hard Point Coordinate Transform External Displacement/Rotation Angles to Direction Cosine
 * Matrix/Create 3x3 Matrix'
 * '<S200>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Left/Hard Point Coordinate Transform External Displacement/wxR/Subsystem'
 * '<S201>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Left/Hard Point Coordinate Transform External Displacement/wxR/Subsystem1'
 * '<S202>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Right/Hard Point Coordinate Transform External Displacement'
 * '<S203>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Right/Hard Point Coordinate Transform External Displacement/Rotation Angles to Direction Cosine
 * Matrix'
 * '<S204>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Right/Hard Point Coordinate Transform External Displacement/transform to Inertial axes'
 * '<S205>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Right/Hard Point Coordinate Transform External Displacement/transform to Inertial axes1'
 * '<S206>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Right/Hard Point Coordinate Transform External Displacement/wxR'
 * '<S207>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Right/Hard Point Coordinate Transform External Displacement/Rotation Angles to Direction Cosine
 * Matrix/Create 3x3 Matrix'
 * '<S208>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Right/Hard Point Coordinate Transform External Displacement/wxR/Subsystem'
 * '<S209>' : 'Chassis/Chassis/Vehicle/Variant Subsystem/6DOF/Vehicle Body 6DOF/Signal Routing/Hard Point Coordinate
 * Transform Rear Right/Hard Point Coordinate Transform External Displacement/wxR/Subsystem1'
 * '<S210>' : 'Chassis/Chassis/Wheels and Tires/Cont LPF'
 * '<S211>' : 'Chassis/Chassis/Wheels and Tires/Cont LPF1'
 * '<S212>' : 'Chassis/Chassis/Wheels and Tires/VDBS'
 * '<S213>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tire2Veh'
 * '<S214>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires'
 * '<S215>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Veh2Tire'
 * '<S216>' : 'Chassis/Chassis/Wheels and Tires/VDBS/scale factors with friction'
 * '<S217>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tire2Veh/Suspension Routing'
 * '<S218>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tire2Veh/Wheel Angles'
 * '<S219>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tire2Veh/Wheel Routing'
 * '<S220>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tire2Veh/Wheel to Body Transform'
 * '<S221>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tire2Veh/Wheel to Body Transform/Cross Product'
 * '<S222>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tire2Veh/Wheel to Body Transform/Rotation Angles to Direction
 * Cosine Matrix'
 * '<S223>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tire2Veh/Wheel to Body Transform/Rotation Angles to Direction
 * Cosine Matrix1'
 * '<S224>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tire2Veh/Wheel to Body Transform/Rotation Angles to Direction
 * Cosine Matrix/Create 3x3 Matrix'
 * '<S225>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tire2Veh/Wheel to Body Transform/Rotation Angles to Direction
 * Cosine Matrix1/Create 3x3 Matrix'
 * '<S226>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector'
 * '<S227>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF'
 * '<S228>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Bus Routing'
 * '<S229>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Magic Tire Const
 * Input'
 * '<S230>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Vertical Wheel and
 * Unsprung Mass Response'
 * '<S231>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel Module'
 * '<S232>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Magic Tire Const
 * Input/Fx Relaxation'
 * '<S233>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Magic Tire Const
 * Input/Fy Relaxation'
 * '<S234>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Magic Tire Const
 * Input/Magic Tire Const Input'
 * '<S235>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Magic Tire Const
 * Input/My Relaxation'
 * '<S236>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel Module/Brakes'
 * '<S237>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel Module/Clutch'
 * '<S238>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Friction Model'
 * '<S239>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Brakes/Mapped Brake'
 * '<S240>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Brakes/Mapped Brake/Mapped Brake'
 * '<S241>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters'
 * '<S242>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch'
 * '<S243>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/Locked'
 * '<S244>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/Slipping'
 * '<S245>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/detectLockup'
 * '<S246>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/detectSlip'
 * '<S247>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/detectLockup/Friction Mode Logic'
 * '<S248>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/detectLockup/Friction Mode Logic/Break Apart Detection'
 * '<S249>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/detectLockup/Friction Mode Logic/Lockup Detection'
 * '<S250>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/detectLockup/Friction Mode Logic/Lockup FSM'
 * '<S251>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/detectLockup/Friction Mode Logic/Requisite Friction'
 * '<S252>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/detectLockup/Friction Mode Logic/Lockup Detection/Friction Calc'
 * '<S253>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/detectLockup/Friction Mode Logic/Lockup Detection/Required Friction for
 * Lockup'
 * '<S254>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Tires/MF Tires Vector/Combined Slip Wheel 2DOF/Wheel
 * Module/Clutch/Clutch Scalar Parameters/Clutch/detectSlip/Break Apart Detection'
 * '<S255>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Veh2Tire/Camber Angle'
 * '<S256>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Veh2Tire/Wheel Angles'
 * '<S257>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Veh2Tire/Wheel to Body Transform'
 * '<S258>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Veh2Tire/Wheel to Body Transform/Rotation Angles to Direction
 * Cosine Matrix'
 * '<S259>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Veh2Tire/Wheel to Body Transform/Rotation Angles to Direction
 * Cosine Matrix1'
 * '<S260>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Veh2Tire/Wheel to Body Transform/Rotation Angles to Direction
 * Cosine Matrix/Create 3x3 Matrix'
 * '<S261>' : 'Chassis/Chassis/Wheels and Tires/VDBS/Veh2Tire/Wheel to Body Transform/Rotation Angles to Direction
 * Cosine Matrix1/Create 3x3 Matrix'
 */
#endif /* RTW_HEADER_Chassis_h_ */
