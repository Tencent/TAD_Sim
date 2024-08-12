/*
 * TX_Transmission_data.cpp
 *
 * Code generation for model "TX_Transmission".
 *
 * Model version              : 1.210
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Dec  6 16:43:43 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TX_Transmission.h"

/* Block parameters (default storage) */
P_TX_Transmission_T TX_Transmission::TX_Transmission_P{
    /* Variable: Trans
     * Referenced by:
     *   '<S13>/Constant'
     *   '<S19>/Gear2damping'
     *   '<S19>/Gear2inertias'
     *   '<S19>/Gear2inertias1'
     *   '<S31>/Gear2Ratios'
     *   '<S31>/Gear2damping'
     *   '<S31>/Gear2inertias'
     *   '<S43>/Gear2Ratios'
     *   '<S43>/Gear2damping'
     *   '<S43>/Gear2inertias'
     *   '<S40>/Eta 1D'
     *   '<S50>/Eta 1D'
     */
    {0.01,
     0.001,
     2.77,

     {-3.42, 4.38, 4.38, 2.86, 1.92, 1.37, 1.0, 0.82, 0.7},

     {-1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0},

     {0.034, 0.034, 0.037, 0.034, 0.042, 0.04, 0.04, 0.04, 0.04},

     {0.003, 0.001, 0.003, 0.0025, 0.002, 0.001, 0.001, 0.001, 0.001},

     {0.9, 0.92, 0.92, 0.92, 0.95, 0.95, 0.98, 0.99, 0.99},
     0.35},

    /* Variable: VEH
     * Referenced by:
     *   '<S8>/Constant'
     *   '<S16>/Constant'
     *   '<S20>/Constant'
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
     11.38,
     0.0,
     0.0,
     0.0,
     0.0,
     0.0,
     90.0,
     90.0,
     1.6,
     1.6},

    /* Variable: Dn_Gears
     * Referenced by:
     *   '<S70>/Calculate  Downshift Threshold'
     *   '<S71>/Calculate  Downshift Threshold'
     */
    {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},

    /* Variable: Downshift_Speeds
     * Referenced by:
     *   '<S70>/Calculate  Downshift Threshold'
     *   '<S71>/Calculate  Downshift Threshold'
     */
    {12.43125,
     16.81875,
     21.20625,
     25.59375,
     29.98125,
     34.36875,
     38.75625,
     43.143750000000004,
     47.53125,
     51.91875,
     56.30625,
     60.69374999999998,
     62.15625,
     66.543749999999989,
     70.931249999999977,
     75.318750000000009,
     76.78125,
     81.16875,
     85.55625,
     89.943750000000009,
     89.578125,
     94.696875,
     99.815625,
     104.934375,
     102.375,
     108.22500000000001,
     114.075,
     119.925},

    /* Variable: EngIdleSpd
     * Referenced by:
     *   '<S8>/Saturation'
     *   '<S18>/Constant'
     */
    750.0,

    /* Variable: Pedal_Positions_DnShift
     * Referenced by:
     *   '<S70>/Calculate  Downshift Threshold'
     *   '<S71>/Calculate  Downshift Threshold'
     */
    {0.1, 0.4, 0.5, 0.9},

    /* Variable: Pedal_Positions_UpShift
     * Referenced by:
     *   '<S70>/Calculate Upshift Threshold'
     *   '<S72>/Calculate Upshift Threshold'
     */
    {0.1, 0.4, 0.5, 0.9},

    /* Variable: StatLdWhlR
     * Referenced by:
     *   '<S5>/speed'
     *   '<S8>/Constant'
     *   '<S59>/Gain'
     *   '<S16>/Constant'
     *   '<S20>/Constant'
     */
    {0.364, 0.364, 0.364, 0.364},

    /* Variable: Up_Gears
     * Referenced by:
     *   '<S70>/Calculate Upshift Threshold'
     *   '<S72>/Calculate Upshift Threshold'
     */
    {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0},

    /* Variable: Upshift_Speeds
     * Referenced by:
     *   '<S70>/Calculate Upshift Threshold'
     *   '<S72>/Calculate Upshift Threshold'
     */
    {25.875,
     32.625,
     39.375,
     66.0,
     52.875,
     59.625,
     66.375,
     105.0,
     79.875,
     86.625,
     93.374999999999972,
     154.5,
     102.37499999999997,
     109.12499999999997,
     115.875,
     201.0,
     124.875,
     131.625,
     138.375,
     277.5,
     145.6875,
     153.5625,
     161.4375,
     323.75,
     166.5,
     175.5,
     184.5,
     370.0},

    /* Variable: init_gear_num
     * Referenced by:
     *   '<S59>/init_gear_num'
     *   '<S13>/Constant1'
     */
    0.0,

    /* Variable: init_gear_ratio
     * Referenced by:
     *   '<S8>/Constant'
     *   '<S16>/Constant'
     *   '<S20>/Constant'
     */
    3.029,

    /* Variable: max_gear_num
     * Referenced by: '<S59>/max_gear_num'
     */
    8.0,
    /* Variable: pre_shift_wait_time
     * Referenced by: '<S59>/TCM Shift Controller'
     */
    0.5,

    /* Variable: ratio_diff_front
     * Referenced by:
     *   '<S8>/Constant'
     *   '<S16>/Constant'
     *   '<S20>/Constant'
     */
    4.1,

    /* Variable: ratio_diff_rear
     * Referenced by:
     *   '<S8>/Constant'
     *   '<S16>/Constant'
     *   '<S20>/Constant'
     */
    4.1,

    /* Mask Parameter: PropShaft_b
     * Referenced by: '<S51>/Gain2'
     */
    57.297,

    /* Mask Parameter: CompareToConstant1_const
     * Referenced by: '<S66>/Constant'
     */
    0.5,

    /* Mask Parameter: CompareToConstant_const
     * Referenced by: '<S65>/Constant'
     */
    -0.01,

    /* Mask Parameter: CompareToConstant1_const_m
     * Referenced by: '<S69>/Constant'
     */
    -0.5,

    /* Mask Parameter: CompareToConstant_const_h
     * Referenced by: '<S68>/Constant'
     */
    0.01,

    /* Mask Parameter: CompareToConstant_const_p
     * Referenced by: '<S67>/Constant'
     */
    0.0,

    /* Mask Parameter: CompareToConstant_const_i
     * Referenced by: '<S73>/Constant'
     */
    1.0,

    /* Mask Parameter: PropShaft_domega_o
     * Referenced by: '<S51>/domega_o'
     */
    0.0,

    /* Mask Parameter: PropShaft_k
     * Referenced by: '<S51>/Gain1'
     */
    5729.7,

    /* Mask Parameter: PropShaft_omega_c
     * Referenced by: '<S51>/omega_c'
     */
    200.0,

    /* Mask Parameter: PropShaft_theta_o
     * Referenced by: '<S51>/Integrator'
     */
    0.0,

    /* Mask Parameter: div0protectabspoly_thresh
     * Referenced by:
     *   '<S37>/Constant'
     *   '<S38>/Constant'
     */
    1.0E-6,

    /* Mask Parameter: div0protectabspoly_thresh_h
     * Referenced by:
     *   '<S47>/Constant'
     *   '<S48>/Constant'
     */
    1.0E-6,

    /* Mask Parameter: ContLPFIC_wc
     * Referenced by: '<S9>/Constant'
     */
    20.0,

    /* Mask Parameter: Valve_wc
     * Referenced by: '<S64>/Constant'
     */
    100.0,

    /* Mask Parameter: GearCmdChange_vinit
     * Referenced by: '<S6>/Delay Input1'
     */
    0,

    /* Expression: 1
     * Referenced by: '<S28>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S28>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S33>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S33>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S33>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S33>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S32>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S29>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S29>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S35>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S35>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S35>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S35>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S34>/Constant'
     */
    1.0,

    /* Expression: -1
     * Referenced by: '<S30>/Gain'
     */
    -1.0,

    /* Expression: 0
     * Referenced by: '<S30>/Saturation1'
     */
    0.0,

    /* Expression: -inf
     * Referenced by: '<S30>/Saturation1'
     */
    0.0,

    /* Expression: inf
     * Referenced by: '<S30>/Saturation2'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S30>/Saturation2'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S30>/Saturation'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S30>/Saturation'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S19>/Neutral'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S41>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S41>/Switch'
     */
    0.0,

    /* Expression: 20
     * Referenced by: '<S45>/Gain1'
     */
    20.0,

    /* Expression: 1
     * Referenced by: '<S45>/Constant1'
     */
    1.0,

    /* Expression: 4
     * Referenced by: '<S45>/Gain2'
     */
    4.0,

    /* Expression: .5
     * Referenced by: '<S45>/Gain'
     */
    0.5,

    /* Expression: 1
     * Referenced by: '<S44>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S19>/First'
     */
    1.0,

    /* Expression: -1
     * Referenced by: '<S42>/Gain'
     */
    -1.0,

    /* Expression: 0
     * Referenced by: '<S42>/Saturation1'
     */
    0.0,

    /* Expression: -inf
     * Referenced by: '<S42>/Saturation1'
     */
    0.0,

    /* Expression: inf
     * Referenced by: '<S42>/Saturation2'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S42>/Saturation2'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S42>/Saturation'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S42>/Saturation'
     */
    0.0,

    /* Computed Parameter: up_th_Y0
     * Referenced by: '<S72>/up_th'
     */
    0.0,

    /* Computed Parameter: down_th_Y0
     * Referenced by: '<S71>/down_th'
     */
    0.0,

    /* Computed Parameter: SpdThr_Y0
     * Referenced by: '<S70>/SpdThr'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S74>/Constant'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S15>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S12>/Memory'
     */
    0.0,

    /* Expression: 9000.0/60.0*2*3.1415926
     * Referenced by: '<S8>/Saturation'
     */
    942.47778,

    /* Expression: 0
     * Referenced by: '<S52>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S64>/Integrator1'
     */
    0.0,

    /* Expression: 1e-2
     * Referenced by: '<S61>/Constant1'
     */
    0.01,

    /* Expression: 1.0/2.0/pi*60
     * Referenced by: '<S4>/rpm'
     */
    9.5492965855137211,

    /* Expression: 600
     * Referenced by: '<S61>/Constant'
     */
    600.0,

    /* Expression: 600
     * Referenced by: '<S61>/Constant2'
     */
    600.0,

    /* Expression: 0.5
     * Referenced by: '<S2>/HoldSecond'
     */
    0.5,

    /* Expression: -0.5
     * Referenced by: '<S13>/Dead Zone'
     */
    -0.5,

    /* Expression: 0.5
     * Referenced by: '<S13>/Dead Zone'
     */
    0.5,

    /* Expression: 0
     * Referenced by: '<S5>/Constant'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S20>/Constant1'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S20>/IC'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S20>/Switch'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S18>/Constant1'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S18>/IC'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S18>/Switch'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S16>/Constant1'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S16>/IC'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S16>/Switch'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S12>/Reset'
     */
    1.0,

    /* Expression: 2*pi
     * Referenced by: '<S13>/2*pi'
     */
    6.2831853071795862,

    /* Expression: 1
     * Referenced by: '<S15>/Reset'
     */
    1.0,

    /* Computed Parameter: Merge2_1_InitialOutput
     * Referenced by: '<S13>/Merge2'
     */
    0.0,

    /* Computed Parameter: Merge2_2_InitialOutput
     * Referenced by: '<S13>/Merge2'
     */
    0.0,

    /* Computed Parameter: Merge2_3_InitialOutput
     * Referenced by: '<S13>/Merge2'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S52>/Reset'
     */
    1.0,

    /* Expression: 3.6
     * Referenced by: '<S59>/kph'
     */
    3.6,

    /* Computed Parameter: CalculateUpshiftThreshold_maxIndex
     * Referenced by: '<S72>/Calculate Upshift Threshold'
     */
    {3U, 6U},

    /* Computed Parameter: CalculateDownshiftThreshold_maxIndex
     * Referenced by: '<S71>/Calculate  Downshift Threshold'
     */
    {3U, 6U},

    /* Computed Parameter: CalculateUpshiftThreshold_maxIndex_p
     * Referenced by: '<S70>/Calculate Upshift Threshold'
     */
    {3U, 6U},

    /* Computed Parameter: CalculateDownshiftThreshold_maxIndex_b
     * Referenced by: '<S70>/Calculate  Downshift Threshold'
     */
    {3U, 6U},

    /* Computed Parameter: Saturation_UpperSat_k
     * Referenced by: '<S13>/Saturation'
     */
    7,

    /* Computed Parameter: Saturation_LowerSat_e
     * Referenced by: '<S13>/Saturation'
     */
    -1};
