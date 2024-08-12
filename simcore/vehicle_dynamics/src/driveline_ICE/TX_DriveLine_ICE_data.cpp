/*
 * TX_DriveLine_ICE_data.cpp
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

#include "TX_DriveLine_ICE.h"

/* Block parameters (default storage) */
P_TX_DriveLine_ICE_T TX_DriveLine_ICE::TX_DriveLine_ICE_P{
    /* Variable: VEH
     * Referenced by:
     *   '<S58>/Constant'
     *   '<S58>/Constant1'
     *   '<S8>/Constant'
     *   '<S8>/Constant1'
     *   '<S59>/Constant'
     *   '<S59>/Constant1'
     *   '<S85>/Constant'
     *   '<S85>/Constant1'
     *   '<S184>/Constant'
     *   '<S184>/Constant1'
     */
    {1830.0,

     {481.50000000000006, 481.50000000000006},

     {420.0, 420.0},
     3.05,
     1.4,
     1.6,
     0.166,
     2.11,
     0.33,
     2.0,
     2788.0,
     928.0,
     3234.0,
     18.0,
     1650.0,
     0.0,
     0.0,
     0.0,
     0.0,
     0.0,
     0.0,
     22.222222222222221,
     0.0,
     0.0,
     0.0,
     0.0,
     0.0,
     90.0,
     90.0,
     1.6,
     1.6},

    /* Variable: StatLdWhlR
     * Referenced by:
     *   '<S58>/Constant'
     *   '<S58>/Constant1'
     *   '<S8>/Constant'
     *   '<S8>/Constant1'
     *   '<S59>/Constant'
     *   '<S59>/Constant1'
     *   '<S85>/Constant'
     *   '<S85>/Constant1'
     *   '<S184>/Constant'
     *   '<S184>/Constant1'
     */
    {0.364, 0.364, 0.364, 0.364},

    /* Variable: drive_type
     * Referenced by: '<Root>/DriveType[1-FD;2-RD;3-4WD]'
     */
    3.0,

    /* Variable: ratio_diff_front
     * Referenced by:
     *   '<S58>/Constant'
     *   '<S8>/Ndiff2'
     *   '<S59>/Ndiff2'
     *   '<S14>/Gain'
     *   '<S65>/Gain'
     *   '<S17>/Gain'
     *   '<S68>/Gain'
     */
    2.65,

    /* Variable: ratio_diff_rear
     * Referenced by:
     *   '<S58>/Constant1'
     *   '<S85>/Ndiff2'
     *   '<S184>/Ndiff2'
     *   '<S91>/Gain'
     *   '<S190>/Gain'
     *   '<S94>/Gain'
     *   '<S193>/Gain'
     */
    2.65,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_Fc
     * Referenced by: '<S22>/Constant2'
     */
    1000.0,

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Fc
     * Referenced by: '<S73>/Constant2'
     */
    500.0,

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Fc
     * Referenced by: '<S99>/Constant2'
     */
    500.0,

    /* Mask Parameter: ICERearLimitedSlipDifferential_Fc
     * Referenced by: '<S198>/Constant2'
     */
    1000.0,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_Jd
     * Referenced by: '<S8>/Jd'
     */
    0.08,

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Jd
     * Referenced by: '<S59>/Jd'
     */
    0.1,

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Jd
     * Referenced by: '<S85>/Jd'
     */
    0.1,

    /* Mask Parameter: TransferCase_Jd
     * Referenced by: '<S58>/Jd'
     */
    0.1,

    /* Mask Parameter: ICERearLimitedSlipDifferential_Jd
     * Referenced by: '<S184>/Jd'
     */
    0.08,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_Jw1
     * Referenced by: '<S8>/Jw1'
     */
    0.05,

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Jw1
     * Referenced by: '<S59>/Jw1'
     */
    0.1,

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Jw1
     * Referenced by: '<S85>/Jw1'
     */
    0.1,

    /* Mask Parameter: TransferCase_Jw1
     * Referenced by: '<S58>/Jw1'
     */
    0.1,

    /* Mask Parameter: ICERearLimitedSlipDifferential_Jw1
     * Referenced by: '<S184>/Jw1'
     */
    0.01,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_Jw2
     * Referenced by: '<S8>/Jw3'
     */
    0.05,

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Jw2
     * Referenced by: '<S59>/Jw3'
     */
    0.1,

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Jw2
     * Referenced by: '<S85>/Jw3'
     */
    0.1,

    /* Mask Parameter: TransferCase_Jw2
     * Referenced by: '<S58>/Jw3'
     */
    0.1,

    /* Mask Parameter: ICERearLimitedSlipDifferential_Jw2
     * Referenced by: '<S184>/Jw3'
     */
    0.01,

    /* Mask Parameter: TransferCase_Ndiff
     * Referenced by:
     *   '<S58>/Ndiff2'
     *   '<S162>/Gain'
     *   '<S165>/Gain1'
     */
    1.0,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_Ndisks
     * Referenced by: '<S22>/Constant1'
     */
    8.0,

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Ndisks
     * Referenced by: '<S73>/Constant1'
     */
    4.0,

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Ndisks
     * Referenced by: '<S99>/Constant1'
     */
    4.0,

    /* Mask Parameter: ICERearLimitedSlipDifferential_Ndisks
     * Referenced by: '<S198>/Constant1'
     */
    4.0,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_Reff
     * Referenced by: '<S22>/Constant'
     */
    0.08,

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_Reff
     * Referenced by: '<S73>/Constant'
     */
    0.2,

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_Reff
     * Referenced by: '<S99>/Constant'
     */
    0.2,

    /* Mask Parameter: ICERearLimitedSlipDifferential_Reff
     * Referenced by: '<S198>/Constant'
     */
    0.08,

    /* Mask Parameter: TransferCase_SpdLock
     * Referenced by: '<S58>/SpdLockConstantConstant'
     */
    0.0,

    /* Mask Parameter: TransferCase_TrqSplitRatio
     * Referenced by: '<S58>/TrqSplitRatioConstantConstant'
     */
    0.5,

    /* Mask Parameter: TorsionalCompliance2_b
     * Referenced by: '<S42>/Gain2'
     */
    57.29,

    /* Mask Parameter: TorsionalCompliance1_b
     * Referenced by: '<S34>/Gain2'
     */
    57.29,

    /* Mask Parameter: TorsionalCompliance_b
     * Referenced by: '<S210>/Gain2'
     */
    57.29,

    /* Mask Parameter: TorsionalCompliance1_b_b
     * Referenced by: '<S218>/Gain2'
     */
    57.29,

    /* Mask Parameter: TorsionalCompliance4_b
     * Referenced by: '<S143>/Gain2'
     */
    57.297,

    /* Mask Parameter: TorsionalCompliance5_b
     * Referenced by: '<S151>/Gain2'
     */
    57.297,

    /* Mask Parameter: TorsionalCompliance2_b_f
     * Referenced by: '<S127>/Gain2'
     */
    57.297,

    /* Mask Parameter: TorsionalCompliance3_b
     * Referenced by: '<S135>/Gain2'
     */
    57.297,

    /* Mask Parameter: TorsionalCompliance_b_o
     * Referenced by: '<S111>/Gain2'
     */
    45.8376,

    /* Mask Parameter: TorsionalCompliance1_b_j
     * Referenced by: '<S119>/Gain2'
     */
    45.8376,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_bd
     * Referenced by: '<S8>/bd'
     */
    0.001,

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_bd
     * Referenced by: '<S59>/bd'
     */
    0.001,

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_bd
     * Referenced by: '<S85>/bd'
     */
    0.001,

    /* Mask Parameter: TransferCase_bd
     * Referenced by: '<S58>/bd'
     */
    0.001,

    /* Mask Parameter: ICERearLimitedSlipDifferential_bd
     * Referenced by: '<S184>/bd'
     */
    0.001,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_bw1
     * Referenced by: '<S8>/bw1'
     */
    0.001,

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_bw1
     * Referenced by: '<S59>/bw1'
     */
    0.001,

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_bw1
     * Referenced by: '<S85>/bw1'
     */
    0.001,

    /* Mask Parameter: TransferCase_bw1
     * Referenced by: '<S58>/bw1'
     */
    0.001,

    /* Mask Parameter: ICERearLimitedSlipDifferential_bw1
     * Referenced by: '<S184>/bw1'
     */
    0.001,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_bw2
     * Referenced by: '<S8>/bw2'
     */
    0.001,

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_bw2
     * Referenced by: '<S59>/bw2'
     */
    0.001,

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_bw2
     * Referenced by: '<S85>/bw2'
     */
    0.001,

    /* Mask Parameter: TransferCase_bw2
     * Referenced by: '<S58>/bw2'
     */
    0.001,

    /* Mask Parameter: ICERearLimitedSlipDifferential_bw2
     * Referenced by: '<S184>/bw2'
     */
    0.001,

    /* Mask Parameter: TorsionalCompliance2_domega_o
     * Referenced by: '<S42>/domega_o'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance1_domega_o
     * Referenced by: '<S34>/domega_o'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance_domega_o
     * Referenced by: '<S210>/domega_o'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance1_domega_o_f
     * Referenced by: '<S218>/domega_o'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance4_domega_o
     * Referenced by: '<S143>/domega_o'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance5_domega_o
     * Referenced by: '<S151>/domega_o'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance2_domega_o_j
     * Referenced by: '<S127>/domega_o'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance3_domega_o
     * Referenced by: '<S135>/domega_o'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance_domega_o_m
     * Referenced by: '<S111>/domega_o'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance1_domega_o_p
     * Referenced by: '<S119>/domega_o'
     */
    0.0,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_dw
     * Referenced by: '<S22>/mu Table'
     */
    {0.0, 10.0, 20.0, 40.0, 60.0, 80.0, 100.0, 1000.0},

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_dw
     * Referenced by: '<S73>/mu Table'
     */
    {0.0, 10.0, 20.0, 40.0, 60.0, 80.0, 100.0},

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_dw
     * Referenced by: '<S99>/mu Table'
     */
    {0.0, 10.0, 20.0, 40.0, 60.0, 80.0, 100.0},

    /* Mask Parameter: ICERearLimitedSlipDifferential_dw
     * Referenced by: '<S198>/mu Table'
     */
    {0.0, 10.0, 20.0, 40.0, 60.0, 80.0, 100.0, 1000.0},

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_eta
     * Referenced by: '<S33>/Constant'
     */
    1.0,

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_eta
     * Referenced by: '<S84>/Constant'
     */
    1.0,

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_eta
     * Referenced by: '<S110>/Constant'
     */
    1.0,

    /* Mask Parameter: TransferCase_eta
     * Referenced by: '<S180>/Constant'
     */
    1.0,

    /* Mask Parameter: ICERearLimitedSlipDifferential_eta
     * Referenced by: '<S209>/Constant'
     */
    1.0,

    /* Mask Parameter: TorsionalCompliance2_k
     * Referenced by: '<S42>/Gain1'
     */
    5729.0,

    /* Mask Parameter: TorsionalCompliance1_k
     * Referenced by: '<S34>/Gain1'
     */
    5729.0,

    /* Mask Parameter: TorsionalCompliance_k
     * Referenced by: '<S210>/Gain1'
     */
    5729.0,

    /* Mask Parameter: TorsionalCompliance1_k_b
     * Referenced by: '<S218>/Gain1'
     */
    5729.0,

    /* Mask Parameter: TorsionalCompliance4_k
     * Referenced by: '<S143>/Gain1'
     */
    5729.7,

    /* Mask Parameter: TorsionalCompliance5_k
     * Referenced by: '<S151>/Gain1'
     */
    5729.7,

    /* Mask Parameter: TorsionalCompliance2_k_i
     * Referenced by: '<S127>/Gain1'
     */
    5729.7,

    /* Mask Parameter: TorsionalCompliance3_k
     * Referenced by: '<S135>/Gain1'
     */
    5729.7,

    /* Mask Parameter: TorsionalCompliance_k_h
     * Referenced by: '<S111>/Gain1'
     */
    4583.76,

    /* Mask Parameter: TorsionalCompliance1_k_p
     * Referenced by: '<S119>/Gain1'
     */
    4583.76,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_muc
     * Referenced by: '<S22>/mu Table'
     */
    {0.16, 0.13, 0.115, 0.11, 0.105, 0.1025, 0.10125, 0.10125},

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_muc
     * Referenced by: '<S73>/mu Table'
     */
    {0.16, 0.13, 0.115, 0.11, 0.105, 0.1025, 0.10125},

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_muc
     * Referenced by: '<S99>/mu Table'
     */
    {0.16, 0.13, 0.115, 0.11, 0.105, 0.1025, 0.10125},

    /* Mask Parameter: ICERearLimitedSlipDifferential_muc
     * Referenced by: '<S198>/mu Table'
     */
    {0.16, 0.13, 0.115, 0.11, 0.105, 0.1025, 0.10125, 0.10125},

    /* Mask Parameter: TorsionalCompliance1_omega_c
     * Referenced by: '<S34>/omega_c'
     */
    150.0,

    /* Mask Parameter: TorsionalCompliance2_omega_c
     * Referenced by: '<S42>/omega_c'
     */
    150.0,

    /* Mask Parameter: TorsionalCompliance_omega_c
     * Referenced by: '<S111>/omega_c'
     */
    150.0,

    /* Mask Parameter: TorsionalCompliance1_omega_c_g
     * Referenced by: '<S119>/omega_c'
     */
    150.0,

    /* Mask Parameter: TorsionalCompliance2_omega_c_i
     * Referenced by: '<S127>/omega_c'
     */
    150.0,

    /* Mask Parameter: TorsionalCompliance3_omega_c
     * Referenced by: '<S135>/omega_c'
     */
    150.0,

    /* Mask Parameter: TorsionalCompliance4_omega_c
     * Referenced by: '<S143>/omega_c'
     */
    150.0,

    /* Mask Parameter: TorsionalCompliance5_omega_c
     * Referenced by: '<S151>/omega_c'
     */
    150.0,

    /* Mask Parameter: TorsionalCompliance_omega_c_k
     * Referenced by: '<S210>/omega_c'
     */
    150.0,

    /* Mask Parameter: TorsionalCompliance1_omega_c_n
     * Referenced by: '<S218>/omega_c'
     */
    150.0,

    /* Mask Parameter: TransferCase_shaftSwitchMask
     * Referenced by:
     *   '<S58>/TransferCase'
     *   '<S162>/Constant'
     *   '<S165>/Constant'
     */
    1.0,

    /* Mask Parameter: ICEFrontLimitedSlipDifferential_tauC
     * Referenced by: '<S8>/Constant3'
     */
    0.04,

    /* Mask Parameter: uWD_ICEFrontLimitedSlipDifferential_tauC
     * Referenced by: '<S59>/Constant3'
     */
    0.01,

    /* Mask Parameter: uWD_ICERearLimitedSlipDifferential_tauC
     * Referenced by: '<S85>/Constant3'
     */
    0.01,

    /* Mask Parameter: ICERearLimitedSlipDifferential_tauC
     * Referenced by: '<S184>/Constant3'
     */
    0.04,

    /* Mask Parameter: TorsionalCompliance2_theta_o
     * Referenced by: '<S42>/Integrator'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance1_theta_o
     * Referenced by: '<S34>/Integrator'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance_theta_o
     * Referenced by: '<S210>/Integrator'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance1_theta_o_e
     * Referenced by: '<S218>/Integrator'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance4_theta_o
     * Referenced by: '<S143>/Integrator'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance5_theta_o
     * Referenced by: '<S151>/Integrator'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance2_theta_o_j
     * Referenced by: '<S127>/Integrator'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance3_theta_o
     * Referenced by: '<S135>/Integrator'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance_theta_o_k
     * Referenced by: '<S111>/Integrator'
     */
    0.0,

    /* Mask Parameter: TorsionalCompliance1_theta_o_d
     * Referenced by: '<S119>/Integrator'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S43>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S35>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S211>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S219>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S144>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S152>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S128>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S136>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S14>/Constant1'
     */
    1.0,

    /* Expression: shaftSwitchMask
     * Referenced by: '<S14>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S14>/Switch'
     */
    1.0,

    /* Expression: maxAbsSpd
     * Referenced by: '<S8>/Integrator'
     */
    5000.0,

    /* Expression: -maxAbsSpd
     * Referenced by: '<S8>/Integrator'
     */
    -5000.0,

    /* Expression: 1
     * Referenced by: '<S190>/Constant1'
     */
    1.0,

    /* Expression: shaftSwitchMask
     * Referenced by: '<S190>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S190>/Switch'
     */
    1.0,

    /* Expression: maxAbsSpd
     * Referenced by: '<S184>/Integrator'
     */
    5000.0,

    /* Expression: -maxAbsSpd
     * Referenced by: '<S184>/Integrator'
     */
    -5000.0,

    /* Expression: 1
     * Referenced by: '<S162>/Constant1'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S162>/Switch'
     */
    1.0,

    /* Expression: maxAbsSpd
     * Referenced by: '<S58>/Integrator'
     */
    5000.0,

    /* Expression: -maxAbsSpd
     * Referenced by: '<S58>/Integrator'
     */
    -5000.0,

    /* Expression: 1
     * Referenced by: '<S162>/Constant2'
     */
    1.0,

    /* Expression: 2*pi
     * Referenced by: '<S8>/2*pi'
     */
    6.2831853071795862,

    /* Expression: 1/2
     * Referenced by: '<S14>/Gain1'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S25>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S25>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S32>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S32>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S32>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S32>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S31>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S23>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S23>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S28>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S28>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S28>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S28>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S27>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S24>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S24>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S30>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S30>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S30>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S30>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S29>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S10>/Integrator'
     */
    0.0,

    /* Expression: shaftSwitchMask
     * Referenced by: '<S17>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S17>/Constant2'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S17>/Switch'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S22>/Gain'
     */
    4.0,

    /* Expression: 1
     * Referenced by: '<S35>/Reset'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S43>/Reset'
     */
    1.0,

    /* Expression: 2*pi
     * Referenced by: '<S59>/2*pi'
     */
    6.2831853071795862,

    /* Expression: 0
     * Referenced by: '<S112>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S65>/Constant1'
     */
    1.0,

    /* Expression: shaftSwitchMask
     * Referenced by: '<S65>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S65>/Switch'
     */
    1.0,

    /* Expression: maxAbsSpd
     * Referenced by: '<S59>/Integrator'
     */
    5000.0,

    /* Expression: -maxAbsSpd
     * Referenced by: '<S59>/Integrator'
     */
    -5000.0,

    /* Expression: 1/2
     * Referenced by: '<S65>/Gain1'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S76>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S76>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S83>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S83>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S83>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S83>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S82>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S74>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S74>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S79>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S79>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S79>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S79>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S78>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S75>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S75>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S81>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S81>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S81>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S81>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S80>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S61>/Integrator'
     */
    0.0,

    /* Expression: shaftSwitchMask
     * Referenced by: '<S68>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S68>/Constant2'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S68>/Switch'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S73>/Gain'
     */
    4.0,

    /* Expression: 2*pi
     * Referenced by: '<S85>/2*pi'
     */
    6.2831853071795862,

    /* Expression: 0
     * Referenced by: '<S120>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S91>/Constant1'
     */
    1.0,

    /* Expression: shaftSwitchMask
     * Referenced by: '<S91>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S91>/Switch'
     */
    1.0,

    /* Expression: maxAbsSpd
     * Referenced by: '<S85>/Integrator'
     */
    5000.0,

    /* Expression: -maxAbsSpd
     * Referenced by: '<S85>/Integrator'
     */
    -5000.0,

    /* Expression: 1/2
     * Referenced by: '<S91>/Gain1'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S102>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S102>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S109>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S109>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S109>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S109>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S108>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S100>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S100>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S105>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S105>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S105>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S105>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S104>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S101>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S101>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S107>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S107>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S107>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S107>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S106>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S87>/Integrator'
     */
    0.0,

    /* Expression: shaftSwitchMask
     * Referenced by: '<S94>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S94>/Constant2'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S94>/Switch'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S99>/Gain'
     */
    4.0,

    /* Expression: 1
     * Referenced by: '<S112>/Reset'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S120>/Reset'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S128>/Reset'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S136>/Reset'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S144>/Reset'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S152>/Reset'
     */
    1.0,

    /* Expression: 1/2
     * Referenced by: '<S162>/Gain1'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S172>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S172>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S179>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S179>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S179>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S179>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S178>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S170>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S170>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S175>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S175>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S175>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S175>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S174>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S171>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S171>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S177>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S177>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S177>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S177>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S176>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S165>/Constant6'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S165>/Switch1'
     */
    1.0,

    /* Expression: 2*pi
     * Referenced by: '<S184>/2*pi'
     */
    6.2831853071795862,

    /* Expression: 1/2
     * Referenced by: '<S190>/Gain1'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S201>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S201>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S208>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S208>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S208>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S208>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S207>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S199>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S199>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S204>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S204>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S204>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S204>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S203>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S200>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S200>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S206>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S206>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S206>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S206>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S205>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S186>/Integrator'
     */
    0.0,

    /* Expression: shaftSwitchMask
     * Referenced by: '<S193>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S193>/Constant2'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S193>/Switch'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S198>/Gain'
     */
    4.0,

    /* Expression: 1
     * Referenced by: '<S211>/Reset'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S219>/Reset'
     */
    1.0,

    /* Start of '<S184>/Limited Slip Differential' */
    {/* Expression: shaftSwitchMask
      * Referenced by: '<S184>/Limited Slip Differential'
      */
     1.0},

    /* End of '<S184>/Limited Slip Differential' */

    /* Start of '<S85>/Limited Slip Differential' */
    {/* Expression: shaftSwitchMask
      * Referenced by: '<S85>/Limited Slip Differential'
      */
     1.0},

    /* End of '<S85>/Limited Slip Differential' */

    /* Start of '<S59>/Limited Slip Differential' */
    {/* Expression: shaftSwitchMask
      * Referenced by: '<S59>/Limited Slip Differential'
      */
     1.0},

    /* End of '<S59>/Limited Slip Differential' */

    /* Start of '<S8>/Limited Slip Differential' */
    {/* Expression: shaftSwitchMask
      * Referenced by: '<S8>/Limited Slip Differential'
      */
     1.0}
    /* End of '<S8>/Limited Slip Differential' */
};
