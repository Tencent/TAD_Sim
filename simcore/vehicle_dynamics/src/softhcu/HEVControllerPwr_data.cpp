/*
 * HEVControllerPwr_data.cpp
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

#include "HEVControllerPwr.h"

/* Block parameters (default storage) */
P_HEVControllerPwr_T HEVControllerPwr::HEVControllerPwr_P{
    /* Variable: VEH
     * Referenced by:
     *   '<S183>/InertiaForce'
     *   '<S184>/AeroDragForce'
     *   '<S184>/RollingResistanceForce'
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

    /* Variable: DriveLinePowType
     * Referenced by: '<S50>/Constant2'
     */
    2.0,

    /* Variable: EnableP4
     * Referenced by:
     *   '<S22>/EV_Pwr_Calculation'
     *   '<S22>/HEV_Pwr_Calculation'
     *   '<S22>/ReGen_Pwr_Calculation'
     */
    0.0,

    /* Variable: EngIdleSpd
     * Referenced by:
     *   '<S22>/EV_Pwr_Calculation'
     *   '<S22>/Engine_Pwr_Calculation'
     *   '<S22>/HEV_Pwr_Calculation'
     *   '<S22>/ReGen_Pwr_Calculation'
     */
    750.0,

    /* Variable: EngMaxSpd
     * Referenced by: '<S29>/Saturation'
     */
    6500.0,

    /* Variable: HEV_Mode_Duration
     * Referenced by: '<S2>/HEV_Mode_Duration'
     */
    0.5,

    /* Variable: Inverse_EngOptimal_Pwr_KW
     * Referenced by: '<S22>/EV_Pwr_Calculation'
     */
    {0.0, 11.0, 22.0, 33.0, 44.0, 55.0, 66.0, 77.0, 88.0, 99.0, 110.0},

    /* Variable: Inverse_EngOptimal_Speed_RPM
     * Referenced by: '<S22>/EV_Pwr_Calculation'
     */
    {400.0, 1100.0, 1600.0, 2200.0, 2600.0, 3000.0, 3200.0, 3400.0, 3600.0, 3800.0, 4000.0},

    /* Variable: abs_brake_pressure
     * Referenced by:
     *   '<S55>/abs_brake_pressure_01'
     *   '<S56>/abs_brake_pressure_01'
     */
    0.2,

    /* Variable: brake_D
     * Referenced by: '<S84>/Derivative Gain'
     */
    0.0,

    /* Variable: brake_I
     * Referenced by: '<S87>/Integral Gain'
     */
    0.02,

    /* Variable: brake_P
     * Referenced by: '<S95>/Proportional Gain'
     */
    0.1,

    /* Variable: ecu_max_acc
     * Referenced by: '<S121>/Constant'
     */
    10.0,

    /* Variable: ecu_max_dec
     * Referenced by: '<S55>/Constant'
     */
    -10.0,

    /* Variable: enable_abs
     * Referenced by: '<S49>/abs_switch'
     */
    0.0,

    /* Variable: front_diff_ratio
     * Referenced by: '<S5>/front_diff_ratio'
     */
    3.1739,

    /* Variable: front_motor_ratio
     * Referenced by: '<S5>/front_motor_ratio'
     */
    3.7,

    /* Variable: hcu_acc_D
     * Referenced by: '<S160>/Derivative Gain'
     */
    0.0,

    /* Variable: hcu_acc_I
     * Referenced by: '<S163>/Integral Gain'
     */
    0.01,

    /* Variable: hcu_acc_P
     * Referenced by: '<S171>/Proportional Gain'
     */
    0.1,

    /* Variable: slip_disable_ratio_ref
     * Referenced by: '<S49>/OnAtRef_OffAt0.5Ref'
     */
    0.1,

    /* Variable: slip_ratio_ref
     * Referenced by: '<S49>/OnAtRef_OffAt0.5Ref'
     */
    0.25,

    /* Variable: softecu_ax_delay
     * Referenced by: '<S187>/AxDelayStep'
     */
    5.0,

    /* Variable: speedOffset
     * Referenced by:
     *   '<S116>/Constant'
     *   '<S124>/Constant'
     *   '<S125>/Constant'
     *   '<S111>/Constant'
     *   '<S112>/Constant'
     */
    0.01,

    /* Variable: stepTime
     * Referenced by:
     *   '<S2>/StepTime'
     *   '<S187>/Gain'
     */
    0.001,

    /* Variable: total_torque_data
     * Referenced by: '<S5>/Total Torque Request Table'
     */
    {0.0,    0.0,    -16.0,  -50.0,  -50.0,  -50.0,  -60.0,  -60.0,  -60.0,  -60.0,  -60.0,  -60.0,  -60.0,  -60.0,
     330.0,  300.0,  220.0,  73.0,   45.0,   21.0,   1.0,    -2.0,   -4.0,   9.0,    14.0,   19.0,   19.0,   20.0,
     600.0,  490.0,  420.0,  218.0,  144.0,  110.0,  78.0,   68.0,   52.0,   53.0,   55.0,   59.0,   59.0,   59.0,
     730.0,  570.0,  507.0,  355.0,  269.0,  219.0,  153.0,  136.0,  108.0,  98.0,   96.0,   100.0,  100.0,  99.0,
     900.0,  780.0,  714.0,  564.0,  410.0,  348.0,  253.0,  226.0,  165.0,  142.0,  137.0,  140.0,  141.0,  138.0,
     1138.0, 1087.0, 1018.0, 897.0,  749.0,  635.0,  523.0,  484.0,  370.0,  241.0,  168.0,  159.0,  176.0,  195.0,
     1368.0, 1355.0, 1349.0, 1219.0, 1034.0, 889.0,  752.0,  707.0,  575.0,  444.0,  314.0,  276.0,  277.0,  281.0,
     1698.0, 1693.0, 1691.0, 1541.0, 1368.0, 1193.0, 981.0,  928.0,  800.0,  647.0,  525.0,  443.0,  417.0,  396.0,
     2028.0, 2030.0, 2032.0, 1884.0, 1752.0, 1547.0, 1311.0, 1252.0, 1035.0, 849.0,  736.0,  703.0,  667.0,  651.0,
     2609.0, 2610.0, 2611.0, 2527.0, 2396.0, 2294.0, 2200.0, 2020.0, 1575.0, 1291.0, 1111.0, 1058.0, 1018.0, 984.0,
     3190.0, 3190.0, 3190.0, 3190.0, 3190.0, 3190.0, 3190.0, 2887.0, 2165.0, 1732.0, 1486.0, 1403.0, 1338.0, 1280.0},

    /* Variable: total_torque_speed_bpt
     * Referenced by: '<S5>/Total Torque Request Table'
     */
    {0.0, 7.0, 10.0, 20.0, 30.0, 40.0, 54.0, 60.0, 80.0, 100.0, 120.0, 135.0, 150.0, 165.0},

    /* Variable: total_torque_throttle_bpt
     * Referenced by: '<S5>/Total Torque Request Table'
     */
    {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0},

    /* Mask Parameter: SoftHCU_AirDensity
     * Referenced by: '<S184>/AeroDragForce'
     */
    1.204,

    /* Mask Parameter: HEVController_BrakeEnableThresh
     * Referenced by: '<S45>/Constant'
     */
    1.0E-5,

    /* Mask Parameter: HEVController_DefaultChrgPwr
     * Referenced by:
     *   '<S22>/EV_Pwr_Calculation'
     *   '<S22>/Engine_Pwr_Calculation'
     */
    3300.0,

    /* Mask Parameter: HEVController_EngOptimal_Pwr_KW
     * Referenced by:
     *   '<S22>/Engine_Pwr_Calculation'
     *   '<S22>/HEV_Pwr_Calculation'
     */
    {0.0, 11.0, 22.0, 33.000000000000007, 44.0, 55.0, 66.0, 77.0, 88.0, 99.0, 110.0},

    /* Mask Parameter: HEVController_EngOptimal_Speed_RPM
     * Referenced by:
     *   '<S22>/Engine_Pwr_Calculation'
     *   '<S22>/HEV_Pwr_Calculation'
     */
    {0.0, 400.0, 800.0, 1200.0000000000002, 1600.0, 2000.0, 2400.0, 2800.0, 3200.0, 3600.0, 4000.0},

    /* Mask Parameter: HEVController_EngineStepInSpeed_kph
     * Referenced by: '<S2>/EngineStepInSpeed_kph'
     */
    45.0,

    /* Mask Parameter: HEVController_FrontMotor_MaxTorque
     * Referenced by:
     *   '<S8>/Front_MotorMaxTrq'
     *   '<S22>/ReGen_Pwr_Calculation'
     */
    {380.64, 380.64, 380.64, 354.6446, 290.18, 253.8265, 240.3134, 203.8175, 176.5011, 155.4483, 138.7867, 0.0, 0.0},

    /* Mask Parameter: HEVController_FrontMotor_Speed_rads
     * Referenced by:
     *   '<S8>/Front_MotorMaxTrq'
     *   '<S22>/ReGen_Pwr_Calculation'
     */
    {0.0, 174.567828, 349.0309379, 523.5987667, 698.1665955, 819.6415093, 872.6297045, 1047.197533, 1221.765362,
     1396.228471, 1570.7963, 1649.336115, 2356.19445},

    /* Mask Parameter: HEVController_HEV_4WD_FrontPwrRatio
     * Referenced by:
     *   '<S22>/EV_Pwr_Calculation'
     *   '<S22>/HEV_Pwr_Calculation'
     */
    0.5,

    /* Mask Parameter: DiscretePIDController_InitialConditionForFilter
     * Referenced by: '<S85>/Filter'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_InitialConditionForFilter_p
     * Referenced by: '<S161>/Filter'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator
     * Referenced by: '<S90>/Integrator'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_InitialConditionForIntegrator_e
     * Referenced by: '<S166>/Integrator'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_LowerSaturationLimit
     * Referenced by: '<S97>/Saturation'
     */
    0.0,

    /* Mask Parameter: DiscretePIDController_LowerSaturationLimit_n
     * Referenced by:
     *   '<S173>/Saturation'
     *   '<S159>/DeadZone'
     */
    -0.3,

    /* Mask Parameter: HEVController_MaxChrgPwr
     * Referenced by:
     *   '<S22>/EV_Pwr_Calculation'
     *   '<S22>/Engine_Pwr_Calculation'
     */
    50000.0,

    /* Mask Parameter: SoftHCU_MaxPwrOfSystem
     * Referenced by: '<S121>/MaxPwrOfSystem'
     */
    165000.0,

    /* Mask Parameter: HEVController_MaxReGenPwr
     * Referenced by: '<S22>/ReGen_Pwr_Calculation'
     */
    30000.0,

    /* Mask Parameter: HEVController_MinSocDriveVehicle
     * Referenced by: '<S2>/MinSocDriveVehicle'
     */
    10.0,

    /* Mask Parameter: DivProtect_MinVal
     * Referenced by:
     *   '<S14>/Constant'
     *   '<S16>/Constant'
     */
    1.0,

    /* Mask Parameter: DivProtect_MinVal_p
     * Referenced by:
     *   '<S18>/Constant'
     *   '<S20>/Constant'
     */
    1.0,

    /* Mask Parameter: DiscretePIDController_N
     * Referenced by: '<S93>/Filter Coefficient'
     */
    100.0,

    /* Mask Parameter: DiscretePIDController_N_m
     * Referenced by: '<S169>/Filter Coefficient'
     */
    100.0,

    /* Mask Parameter: HEVController_P4_Motor_MaxTorque
     * Referenced by:
     *   '<S10>/P4_MotorMaxTrq'
     *   '<S22>/ReGen_Pwr_Calculation'
     */
    {380.64, 380.64, 380.64, 354.6446, 290.18, 253.8265, 240.3134, 203.8175, 176.5011, 155.4483, 138.7867, 0.0, 0.0},

    /* Mask Parameter: HEVController_P4_Motor_Speed_rads
     * Referenced by:
     *   '<S10>/P4_MotorMaxTrq'
     *   '<S22>/ReGen_Pwr_Calculation'
     */
    {0.0, 174.567828, 349.0309379, 523.5987667, 698.1665955, 819.6415093, 872.6297045, 1047.197533, 1221.765362,
     1396.228471, 1570.7963, 1649.336115, 2356.19445},

    /* Mask Parameter: HEVController_PwrDmnd_EV_Only
     * Referenced by: '<S2>/PwrDmnd_EV_Only'
     */
    40000.0,

    /* Mask Parameter: HEVController_PwrDmnd_Hybrid
     * Referenced by: '<S2>/PwrDmnd_Hybrid'
     */
    60000.0,

    /* Mask Parameter: SoftHCU_RollingCoeff
     * Referenced by: '<S184>/RollingCoeff'
     */
    0.02,

    /* Mask Parameter: HEVController_StartChargingSoc
     * Referenced by: '<S2>/StartChargingSoc'
     */
    25.0,

    /* Mask Parameter: HEVController_TargetSoc
     * Referenced by: '<S2>/TargetSoc'
     */
    70.0,

    /* Mask Parameter: HEVController_ThrottleDisableThresh
     * Referenced by: '<S46>/Constant'
     */
    1.0E-5,

    /* Mask Parameter: DiscretePIDController_UpperSaturationLimit
     * Referenced by: '<S97>/Saturation'
     */
    1.0,

    /* Mask Parameter: DiscretePIDController_UpperSaturationLimit_p
     * Referenced by:
     *   '<S173>/Saturation'
     *   '<S159>/DeadZone'
     */
    0.3,

    /* Mask Parameter: CompareToConstant_const
     * Referenced by: '<S113>/Constant'
     */
    1.0E-9,

    /* Mask Parameter: brakeEnableThresh_Reverse_const
     * Referenced by: '<S108>/Constant'
     */
    0.005,

    /* Mask Parameter: brakeEnableThresh_const
     * Referenced by: '<S107>/Constant'
     */
    -0.005,

    /* Mask Parameter: Accel_const
     * Referenced by: '<S129>/Constant'
     */
    0.01,

    /* Mask Parameter: CompareToConstant1_const
     * Referenced by: '<S130>/Constant'
     */
    0.0,

    /* Mask Parameter: CompareToConstant_const_m
     * Referenced by: '<S185>/Constant'
     */
    0.01,

    /* Mask Parameter: CompareToConstant1_const_l
     * Referenced by: '<S128>/Constant'
     */
    0.0,

    /* Mask Parameter: CompareToConstant_const_p
     * Referenced by: '<S122>/Constant'
     */
    0.1,

    /* Mask Parameter: CompareToConstant1_const_k
     * Referenced by: '<S123>/Constant'
     */
    1.0E-5,

    /* Mask Parameter: CompareToConstant_const_h
     * Referenced by: '<S118>/Constant'
     */
    0.0,

    /* Mask Parameter: VXLOW_ReGen_const
     * Referenced by: '<S48>/Constant'
     */
    10.0,

    /* Mask Parameter: Gear_DriveMode_const
     * Referenced by: '<S47>/Constant'
     */
    1.0,

    /* Mask Parameter: reverseGear_const
     * Referenced by: '<S110>/Constant'
     */
    2U,

    /* Mask Parameter: notReverse_const
     * Referenced by: '<S109>/Constant'
     */
    2U,

    /* Mask Parameter: reverseGear_const_n
     * Referenced by: '<S127>/Constant'
     */
    2U,

    /* Mask Parameter: forwardGear_const
     * Referenced by: '<S126>/Constant'
     */
    1U,

    /* Expression: -1
     * Referenced by: '<S8>/Gain'
     */
    -1.0,

    /* Expression: -1
     * Referenced by: '<S10>/Gain'
     */
    -1.0,

    /* Expression: 0
     * Referenced by: '<S56>/Switch'
     */
    0.0,

    /* Computed Parameter: Filter_gainval
     * Referenced by: '<S85>/Filter'
     */
    0.001,

    /* Computed Parameter: Integrator_gainval
     * Referenced by: '<S90>/Integrator'
     */
    1.0,

    /* Expression: 0.5
     * Referenced by: '<S55>/abs'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S49>/OnAtRef_OffAt0.5Ref'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S49>/OnAtRef_OffAt0.5Ref'
     */
    0.0,

    /* Expression: [1 1 1 1]
     * Referenced by: '<S49>/Constant1'
     */
    {1.0, 1.0, 1.0, 1.0},

    /* Expression: 0
     * Referenced by: '<S49>/Memory'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S49>/Saturation'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S49>/Saturation'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S121>/Saturation'
     */
    1.0,

    /* Expression: 0.0178
     * Referenced by: '<S121>/Saturation'
     */
    0.0178,

    /* Expression: 0
     * Referenced by: '<S157>/Constant1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S121>/Constant2'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S157>/Clamping_zero'
     */
    0.0,

    /* Expression: -0.01
     * Referenced by: '<S121>/TargetAccDeadZone'
     */
    -0.01,

    /* Expression: 0.01
     * Referenced by: '<S121>/TargetAccDeadZone'
     */
    0.01,

    /* Computed Parameter: Integrator_gainval_g
     * Referenced by: '<S166>/Integrator'
     */
    1.0,

    /* Computed Parameter: Filter_gainval_j
     * Referenced by: '<S161>/Filter'
     */
    0.001,

    /* Expression: 0
     * Referenced by: '<S183>/ZeroPwr'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S121>/ZeroPwr'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S120>/Saturation'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S120>/Saturation'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S117>/Zero'
     */
    0.0,

    /* Expression: -1
     * Referenced by: '<S119>/fliper'
     */
    -1.0,

    /* Expression: 0
     * Referenced by: '<S51>/no_throttle'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S51>/Saturation'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S51>/Saturation'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S17>/Constant'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S21>/Constant'
     */
    0.0,

    /* Expression: 0.6
     * Referenced by: '<S4>/autoHoldBrakePressure_01'
     */
    0.6,

    /* Expression: 0.5
     * Referenced by: '<S4>/Switch1'
     */
    0.5,

    /* Expression: -5
     * Referenced by: '<S54>/RelayCutOffAt6_EnableAt5'
     */
    -5.0,

    /* Expression: -6
     * Referenced by: '<S54>/RelayCutOffAt6_EnableAt5'
     */
    -6.0,

    /* Expression: 0
     * Referenced by: '<S54>/RelayCutOffAt6_EnableAt5'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S54>/RelayCutOffAt6_EnableAt5'
     */
    1.0,

    /* Expression: 58
     * Referenced by: '<S54>/RelayCutOffAt58_EnableAt55'
     */
    58.0,

    /* Expression: 55
     * Referenced by: '<S54>/RelayCutOffAt58_EnableAt55'
     */
    55.0,

    /* Expression: 1
     * Referenced by: '<S54>/RelayCutOffAt58_EnableAt55'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S54>/RelayCutOffAt58_EnableAt55'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S52>/Constant'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S13>/EV_Swtich'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S13>/Engine_Switch'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S13>/Engine_ChargingSwitch'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S13>/Engine_ClutchSwitch'
     */
    0.0,

    /* Expression: 3.6
     * Referenced by: '<S1>/kph'
     */
    3.6,

    /* Expression: 1
     * Referenced by: '<S1>/Saturation1'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S1>/Saturation1'
     */
    0.0,

    /* Expression: 100
     * Referenced by: '<S3>/Gain'
     */
    100.0,

    /* Expression: 0
     * Referenced by: '<S5>/Zero'
     */
    0.0,

    /* Expression: 3.6
     * Referenced by: '<S5>/Gain'
     */
    3.6,

    /* Expression: -5*3.6
     * Referenced by: '<S5>/ReverseSpeedLimit_kph'
     */
    -18.0,

    /* Expression: -6*3.6
     * Referenced by: '<S5>/ReverseSpeedLimit_kph'
     */
    -21.6,

    /* Expression: 1
     * Referenced by: '<S5>/ReverseSpeedLimit_kph'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S5>/ReverseSpeedLimit_kph'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S5>/Dead Zone'
     */
    0.0,

    /* Expression: ThrottleDisableThresh
     * Referenced by: '<S5>/Dead Zone'
     */
    1.0E-5,

    /* Expression: 1
     * Referenced by: '<S5>/ThrottleSaturation'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S5>/ThrottleSaturation'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S9>/EV_Swtich'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S9>/Engine_Switch'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S9>/Engine_ChargingSwitch'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S9>/Engine_ClutchSwitch'
     */
    1.0,

    /* Expression: 30/pi
     * Referenced by: '<S7>/rpm'
     */
    9.5492965855137211,

    /* Expression: 1
     * Referenced by: '<S14>/Constant1'
     */
    1.0,

    /* Expression: -1
     * Referenced by: '<S14>/Constant2'
     */
    -1.0,

    /* Expression: 1
     * Referenced by: '<S18>/Constant1'
     */
    1.0,

    /* Expression: -1
     * Referenced by: '<S18>/Constant2'
     */
    -1.0,

    /* Computed Parameter: HEV_Mode_Value
     * Referenced by: '<S13>/HEV_Mode'
     */
    3U,

    /* Computed Parameter: TotalTorqueRequestTable_maxIndex
     * Referenced by: '<S5>/Total Torque Request Table'
     */
    {13U, 10U},

    /* Computed Parameter: HEV_Mode_Value_d
     * Referenced by: '<S9>/HEV_Mode'
     */
    1U,

    /* Computed Parameter: Constant_Value_e
     * Referenced by: '<S157>/Constant'
     */
    1,

    /* Computed Parameter: Constant2_Value_p
     * Referenced by: '<S157>/Constant2'
     */
    -1,

    /* Computed Parameter: Constant3_Value
     * Referenced by: '<S157>/Constant3'
     */
    1,

    /* Computed Parameter: Constant4_Value
     * Referenced by: '<S157>/Constant4'
     */
    -1,

    /* Start of '<S4>/ax_estimator' */
    {/* Mask Parameter: SpeedThresh_const
      * Referenced by: '<S189>/Constant'
      */
     1.3888888888888888,

     /* Mask Parameter: AxAct_wc
      * Referenced by: '<S186>/Constant'
      */
     50.0,

     /* Expression: 0.0
      * Referenced by: '<S187>/Delay'
      */
     0.0,

     /* Expression: 0
      * Referenced by: '<S186>/Integrator1'
      */
     0.0},

    /* End of '<S4>/ax_estimator' */

    /* Start of '<S22>/ReGen_Pwr_Calculation' */
    {
        /* Computed Parameter: Merge_InitialOutput
         * Referenced by: '<S26>/Merge'
         */
        {
            0.0, /* EV_PwrDmnd */
            0.0, /* Engine_TractionPwrDmnd */
            0.0, /* Engine_SerializePwrDmnd */
            0.0, /* Engine_ChargingPwrDmnd */
            0.0, /* P4_Motor_Enabled */
            0.0, /* P4_PwrDmnd */
            0.0  /* Engine_Request_Spd_RPM */
        },

        /* Expression: 0.5
         * Referenced by: '<S42>/Half'
         */
        0.5,

        /* Expression: 1
         * Referenced by: '<S42>/All'
         */
        1.0,

        /* Expression: 0.5
         * Referenced by: '<S42>/Switch'
         */
        0.5,

        /* Expression: 0
         * Referenced by: '<S39>/ReGenPwr_DeadZone'
         */
        0.0,

        /* Expression: 1000
         * Referenced by: '<S39>/ReGenPwr_DeadZone'
         */
        1000.0,

        /* Expression: 0
         * Referenced by: '<S39>/Constant1'
         */
        0.0,

        /* Expression: -1
         * Referenced by: '<S39>/Gain'
         */
        -1.0,

        /* Expression: -1
         * Referenced by: '<S39>/Gain1'
         */
        -1.0,

        /* Expression: 800
         * Referenced by: '<S41>/Constant'
         */
        800.0,

        /* Expression: 1
         * Referenced by: '<S41>/Saturation'
         */
        1.0,

        /* Expression: 0
         * Referenced by: '<S41>/Saturation'
         */
        0.0,

        /* Start of '<S26>/NoPwrMode' */
        {/* Expression: 0
          * Referenced by: '<S40>/Zero'
          */
         0.0}
        /* End of '<S26>/NoPwrMode' */
    },

    /* End of '<S22>/ReGen_Pwr_Calculation' */

    /* Start of '<S22>/HEV_Pwr_Calculation' */
    {
        /* Computed Parameter: Merge_InitialOutput
         * Referenced by: '<S25>/Merge'
         */
        {
            0.0, /* EV_PwrDmnd */
            0.0, /* Engine_TractionPwrDmnd */
            0.0, /* Engine_SerializePwrDmnd */
            0.0, /* Engine_ChargingPwrDmnd */
            0.0, /* P4_Motor_Enabled */
            0.0, /* P4_PwrDmnd */
            0.0  /* Engine_Request_Spd_RPM */
        },

        /* Expression: 0
         * Referenced by: '<S35>/Constant1'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S38>/Zero'
         */
        0.0,

        /* Expression: 1000
         * Referenced by: '<S35>/Gain'
         */
        1000.0,

        /* Expression: inf
         * Referenced by: '<S35>/Saturation'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S35>/Saturation'
         */
        0.0,

        /* Expression: 0.5
         * Referenced by: '<S38>/Front'
         */
        0.5,

        /* Expression: inf
         * Referenced by: '<S38>/Saturation'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S38>/Saturation'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S35>/Zero'
         */
        0.0,

        /* Expression: 0.5
         * Referenced by: '<S38>/Rear'
         */
        0.5,

        /* Expression: inf
         * Referenced by: '<S38>/Saturation1'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S38>/Saturation1'
         */
        0.0,

        /* Start of '<S25>/NoPwrMode' */
        {/* Expression: 0
          * Referenced by: '<S36>/Zero'
          */
         0.0}
        /* End of '<S25>/NoPwrMode' */
    },

    /* End of '<S22>/HEV_Pwr_Calculation' */

    /* Start of '<S22>/Engine_Pwr_Calculation' */
    {
        /* Computed Parameter: Merge_InitialOutput
         * Referenced by: '<S24>/Merge'
         */
        {
            0.0, /* EV_PwrDmnd */
            0.0, /* Engine_TractionPwrDmnd */
            0.0, /* Engine_SerializePwrDmnd */
            0.0, /* Engine_ChargingPwrDmnd */
            0.0, /* P4_Motor_Enabled */
            0.0, /* P4_PwrDmnd */
            0.0  /* Engine_Request_Spd_RPM */
        },

        /* Expression: 1000
         * Referenced by: '<S32>/Gain'
         */
        1000.0,

        /* Expression: 0
         * Referenced by: '<S32>/Zero'
         */
        0.0,

        /* Expression: 0.5
         * Referenced by: '<S32>/ChargingPwr'
         */
        0.5,

        /* Start of '<S24>/NoPwrMode' */
        {/* Expression: 0
          * Referenced by: '<S33>/Zero'
          */
         0.0}
        /* End of '<S24>/NoPwrMode' */
    },

    /* End of '<S22>/Engine_Pwr_Calculation' */

    /* Start of '<S22>/EV_Pwr_Calculation' */
    {
        /* Computed Parameter: Merge_InitialOutput
         * Referenced by: '<S23>/Merge'
         */
        {
            0.0, /* EV_PwrDmnd */
            0.0, /* Engine_TractionPwrDmnd */
            0.0, /* Engine_SerializePwrDmnd */
            0.0, /* Engine_ChargingPwrDmnd */
            0.0, /* P4_Motor_Enabled */
            0.0, /* P4_PwrDmnd */
            0.0  /* Engine_Request_Spd_RPM */
        },

        /* Expression: 0
         * Referenced by: '<S30>/Zero'
         */
        0.0,

        /* Expression: 0.5
         * Referenced by: '<S30>/Front'
         */
        0.5,

        /* Expression: inf
         * Referenced by: '<S30>/Saturation'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S30>/Saturation'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S27>/Zero'
         */
        0.0,

        /* Expression: 0.5
         * Referenced by: '<S30>/Rear'
         */
        0.5,

        /* Expression: inf
         * Referenced by: '<S30>/Saturation1'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S30>/Saturation1'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S31>/Zero'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S29>/SerializePwrSaturation'
         */
        0.0,

        /* Expression: 0.5
         * Referenced by: '<S31>/Front'
         */
        0.5,

        /* Expression: inf
         * Referenced by: '<S31>/Saturation1'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S31>/Saturation1'
         */
        0.0,

        /* Expression: 0.5
         * Referenced by: '<S31>/Rear'
         */
        0.5,

        /* Expression: inf
         * Referenced by: '<S31>/Saturation2'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S31>/Saturation2'
         */
        0.0,

        /* Expression: 0
         * Referenced by: '<S29>/Zero'
         */
        0.0,

        /* Expression: 0.001
         * Referenced by: '<S29>/KW'
         */
        0.001,

        /* Start of '<S23>/NoPwrMode' */
        {/* Expression: 0
          * Referenced by: '<S28>/Zero'
          */
         0.0}
        /* End of '<S23>/NoPwrMode' */
    }
    /* End of '<S22>/EV_Pwr_Calculation' */
};
