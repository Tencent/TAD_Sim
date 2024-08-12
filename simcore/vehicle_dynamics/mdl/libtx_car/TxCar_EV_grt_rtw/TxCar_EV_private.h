/*
 * TxCar_EV_private.h
 *
 * Code generation for model "TxCar_EV".
 *
 * Model version              : 2.1534
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul 19 14:25:50 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_TxCar_EV_private_h_
#define RTW_HEADER_TxCar_EV_private_h_
#include "TxCar_EV_types.h"
#include "multiword_types.h"
#include "rtwtypes.h"

/* Private macros used by the generated code to access rtModel */
#ifndef rtmIsMajorTimeStep
#  define rtmIsMajorTimeStep(rtm) (((rtm)->Timing.simTimeStep) == MAJOR_TIME_STEP)
#endif

#ifndef rtmIsMinorTimeStep
#  define rtmIsMinorTimeStep(rtm) (((rtm)->Timing.simTimeStep) == MINOR_TIME_STEP)
#endif

#ifndef rtmSetTPtr
#  define rtmSetTPtr(rtm, val) ((rtm)->Timing.t = (val))
#endif

#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void chassis_sfunc_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                              const int_T p_width1);
SFB_EXTERN_C void chassis_sfunc_Outputs_wrapper(const Env_Bus_Output *env_in,
                                                const Chassis_DriveLine_Input *driveline_in,
                                                Vehicle_Body_Output *body_out, Wheels_Output *wheel_out,
                                                Susp_Output *susp_out, const int8_T *lic_path, const int_T p_width0,
                                                const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void chassis_sfunc_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                  const int_T p_width1);

#undef SFB_EXTERN_C
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void ecu_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                              const int_T p_width1);
SFB_EXTERN_C void ecu_sfunction_Outputs_wrapper(const powecu_in *ecu_in, powecu_out *ecu_out, const int8_T *lic_path,
                                                const int_T p_width0, const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void ecu_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                  const int_T p_width1);

#undef SFB_EXTERN_C
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void driveline_ev_front_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                             const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void driveline_ev_front_sfunction_Outputs_wrapper(const driveline_in *DrivelineIn,
                                                               driveline_out *DrivelineOut, const int8_T *lic_path,
                                                               const int_T p_width0, const int8_T *par_path,
                                                               const int_T p_width1);
SFB_EXTERN_C void driveline_ev_front_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                                 const int8_T *par_path, const int_T p_width1);

#undef SFB_EXTERN_C
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void driveline_ev_rear_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                            const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void driveline_ev_rear_sfunction_Outputs_wrapper(const driveline_in *DrivelineIn,
                                                              driveline_out *DrivelineOut, const int8_T *lic_path,
                                                              const int_T p_width0, const int8_T *par_path,
                                                              const int_T p_width1);
SFB_EXTERN_C void driveline_ev_rear_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                                const int8_T *par_path, const int_T p_width1);

#undef SFB_EXTERN_C
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void cartype_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                  const int_T p_width1);
SFB_EXTERN_C void cartype_sfunction_Outputs_wrapper(const real_T *UnUsedSig, cartype_parsing_out *CarTypeOut,
                                                    const int8_T *lic_path, const int_T p_width0,
                                                    const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void cartype_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                      const int8_T *par_path, const int_T p_width1);

#undef SFB_EXTERN_C
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void brakehydraulic_sfunc_Start_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                     const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void brakehydraulic_sfunc_Outputs_wrapper(const real_T *BrkPressCmd01, real_T *BrkPressPa,
                                                       const int8_T *lic_path, const int_T p_width0,
                                                       const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void brakehydraulic_sfunc_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                         const int8_T *par_path, const int_T p_width1);

#undef SFB_EXTERN_C
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void battery_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0, const int8_T *par_path,
                                                  const int_T p_width1);
SFB_EXTERN_C void battery_sfunction_Outputs_wrapper(const batt_in *BattIn, batt_out *BattOut, const int8_T *lic_path,
                                                    const int_T p_width0, const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void battery_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                      const int8_T *par_path, const int_T p_width1);

#undef SFB_EXTERN_C
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void front_motor_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                      const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void front_motor_sfunction_Outputs_wrapper(const moter_in *MotorIn, motor_out *MotorOut,
                                                        const int8_T *lic_path, const int_T p_width0,
                                                        const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void front_motor_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                          const int8_T *par_path, const int_T p_width1);

#undef SFB_EXTERN_C
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void rear_motor_sfunction_Start_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                     const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void rear_motor_sfunction_Outputs_wrapper(const moter_in *MotorIn, motor_out *MotorOut,
                                                       const int8_T *lic_path, const int_T p_width0,
                                                       const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void rear_motor_sfunction_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                         const int8_T *par_path, const int_T p_width1);

#undef SFB_EXTERN_C
#ifdef __cplusplus
#  define SFB_EXTERN_C extern "C"
#else
#  define SFB_EXTERN_C extern
#endif

SFB_EXTERN_C void dynamic_steer_sfunc_Start_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                    const int8_T *par_path, const int_T p_width1);
SFB_EXTERN_C void dynamic_steer_sfunc_Outputs_wrapper(const Ctrl_Input *driver_in, const Vehicle_Body_Output *body_in,
                                                      const Susp_Output *susp_in, const Wheels_Output *wheel_in,
                                                      Steer_Output *steer_out, const int8_T *lic_path,
                                                      const int_T p_width0, const int8_T *par_path,
                                                      const int_T p_width1);
SFB_EXTERN_C void dynamic_steer_sfunc_Terminate_wrapper(const int8_T *lic_path, const int_T p_width0,
                                                        const int8_T *par_path, const int_T p_width1);

#undef SFB_EXTERN_C

/* private model entry point functions */
extern void TxCar_EV_derivatives();

#endif /* RTW_HEADER_TxCar_EV_private_h_ */
