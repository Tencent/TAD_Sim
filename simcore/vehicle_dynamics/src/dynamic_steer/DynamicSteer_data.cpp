/*
 * DynamicSteer_data.cpp
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

#include "DynamicSteer.h"

/* Block parameters (default storage) */
P_DynamicSteer_T DynamicSteer::DynamicSteer_P{
    /* Mask Parameter: DynamicSteering_D
     * Referenced by:
     *   '<S24>/Constant2'
     *   '<S24>/Constant3'
     *   '<S24>/Constant9'
     *   '<S25>/Constant2'
     *   '<S25>/Constant3'
     *   '<S25>/Constant9'
     *   '<S26>/Constant2'
     *   '<S26>/Constant3'
     */
    0.15,

    /* Mask Parameter: DynamicSteering_Eta
     * Referenced by: '<S12>/Gain1'
     */
    1.0,

    /* Mask Parameter: DynamicSteering_FricTrq
     * Referenced by:
     *   '<S52>/Gain'
     *   '<S52>/Gain1'
     */
    0.1,

    /* Mask Parameter: DynamicSteering_J1
     * Referenced by: '<S51>/Constant'
     */
    0.1,

    /* Mask Parameter: DynamicSteering_J2
     * Referenced by: '<S53>/Constant3'
     */
    0.01,

    /* Mask Parameter: DynamicSteering_PnnRadiusTbl
     * Referenced by:
     *   '<S21>/1-D Lookup Table'
     *   '<S21>/1-D Lookup Table1'
     */
    {0.0055, 0.0055, 0.0056, 0.0057, 0.0057, 0.0057, 0.0058, 0.0057, 0.0056, 0.0055, 0.0055},

    /* Mask Parameter: DynamicSteering_PwrLmt
     * Referenced by: '<S12>/Saturation2'
     */
    1000.0,

    /* Mask Parameter: MappedSteer_RackDispBpts
     * Referenced by:
     *   '<S78>/leff_whl_ang'
     *   '<S78>/rack_disp'
     *   '<S78>/right_whl_ang'
     */
    {-86.36,  -83.038, -79.963, -77.107, -74.448, -71.967, -69.645, -67.469, -65.424, -63.5,   -61.686,
     -59.972, -58.351, -56.816, -55.359, -53.975, -52.659, -51.405, -50.209, -49.068, -47.978, -46.935,
     -45.936, -44.979, -44.061, -43.18,  0.0,     43.18,   44.061,  44.979,  45.936,  46.935,  47.978,
     49.068,  50.209,  51.405,  52.659,  53.975,  55.359,  56.816,  58.351,  59.972,  61.686,  63.5,
     65.424,  67.469,  69.645,  71.967,  74.448,  77.107,  79.963,  83.038,  86.36},

    /* Mask Parameter: DynamicSteering_RckCsLngth
     * Referenced by:
     *   '<S24>/Constant1'
     *   '<S25>/Constant1'
     *   '<S26>/Constant1'
     */
    0.48,

    /* Mask Parameter: MappedSteer_SpdFctTbl
     * Referenced by: '<S78>/SpeedCoeff'
     */
    {1.0, 1.0},

    /* Mask Parameter: MappedSteer_SteerAngBps
     * Referenced by: '<S78>/rack_disp'
     */
    {-15.995768061602829, -15.139160464724013, -14.389541550992451,
     -13.724222040132211, -13.127493968875347, -12.587838164158704,
     -12.096353446797101, -11.646058499782562, -11.23136826950871,
     -10.848093965770754, -10.492221331289111, -10.160783306335388,
     -9.8511618970315933, -9.561088175350136,  -9.288642279113823,
     -9.0320788790706548, -8.7901762447442415, -8.5615381127329844,
     -8.3449427525604882, -8.1396920325259536, -7.9445642221529891,
     -7.7590357226659918, -7.5822338694389657, -7.4136350636963142,
     -7.2525411737372369, -7.0986031337113369, 0.0,
     7.0986031337113369,  7.2525411737372369,  7.4136350636963142,
     7.5822338694389657,  7.7590357226659918,  7.9445642221529891,
     8.1396920325259536,  8.3449427525604882,  8.5615381127329844,
     8.7901762447442415,  9.0320788790706548,  9.288642279113823,
     9.561088175350136,   9.8511618970315933,  10.160783306335388,
     10.492221331289111,  10.848093965770754,  11.23136826950871,
     11.646058499782562,  12.096353446797101,  12.587838164158704,
     13.127493968875347,  13.724222040132211,  14.389541550992451,
     15.139160464724013,  15.995768061602829},

    /* Mask Parameter: DynamicSteering_StrgAngBpts
     * Referenced by:
     *   '<S21>/1-D Lookup Table'
     *   '<S21>/1-D Lookup Table1'
     */
    {-6.2832, -5.0265, -3.7699, -2.5133, -1.2566, 0.0, 1.2566, 2.5133, 3.7699, 5.0265, 6.2832},

    /* Mask Parameter: DynamicSteering_StrgArmLngth
     * Referenced by:
     *   '<S24>/Constant12'
     *   '<S24>/Constant4'
     *   '<S24>/Constant7'
     *   '<S24>/Gain'
     *   '<S24>/Gain2'
     *   '<S25>/Constant12'
     *   '<S25>/Constant4'
     *   '<S25>/Constant7'
     *   '<S25>/Gain'
     *   '<S25>/Gain2'
     *   '<S26>/Constant4'
     *   '<S26>/Constant7'
     */
    0.117,

    /* Mask Parameter: DynamicSteering_StrgRng
     * Referenced by:
     *   '<S51>/Integrator, Second-Order'
     *   '<S53>/Integrator, Second-Order1'
     */
    15.707963267948966,

    /* Mask Parameter: DynamicSteering_TieRodLngth
     * Referenced by:
     *   '<S24>/Constant10'
     *   '<S24>/Constant5'
     *   '<S25>/Constant10'
     *   '<S25>/Constant5'
     *   '<S26>/Constant5'
     */
    0.435,

    /* Mask Parameter: DynamicSteering_TrckWdth
     * Referenced by:
     *   '<S24>/Constant'
     *   '<S25>/Constant'
     *   '<S26>/Constant'
     */
    1.37,

    /* Mask Parameter: DynamicSteering_TrqBpts
     * Referenced by: '<S12>/TrqAssistTbl'
     */
    {-100.0, 0.0, 100.0},

    /* Mask Parameter: DynamicSteering_TrqLmt
     * Referenced by: '<S12>/Saturation1'
     */
    100.0,

    /* Mask Parameter: DynamicSteering_TrqTbl
     * Referenced by: '<S12>/TrqAssistTbl'
     */
    {0.0, 0.0, 0.0, -100.0, 0.0, 100.0},

    /* Mask Parameter: MappedSteer_VehSpdBpts
     * Referenced by: '<S78>/SpeedCoeff'
     */
    {-1.0, 1.0},

    /* Mask Parameter: DynamicSteering_VehSpdBpts
     * Referenced by: '<S12>/TrqAssistTbl'
     */
    {0.0, 20.0},

    /* Mask Parameter: MappedSteer_WhlLftTbl
     * Referenced by: '<S78>/leff_whl_ang'
     */
    {-0.77920326382515159, -0.7409345058357546,  -0.70747650218712121,
     -0.6777716536893118,  -0.6510887717564301,  -0.62689814459717252,
     -0.60480198654226847, -0.584492948950166,   -0.56572799768958493,
     -0.54831123037302976, -0.53208216345218118, -0.51690750176874112,
     -0.50267520583123992, -0.48929011854352988, -0.47667068066966511,
     -0.46474641825791085, -0.45345599555143046, -0.44274568241466822,
     -0.43256813556993323, -0.42288141667516238, -0.41364819575564932,
     -0.40483509757823993, -0.39641216042473093, -0.38835238684412088,
     -0.38063136561429373, -0.373226952870365,   0.0,
     0.45794923261335435,  0.46886959036600762,  0.48032972906348487,
     0.49237099939549028,  0.50503911170966642,  0.51838473134340246,
     0.53246417378847477,  0.54734022237780211,  0.56308309432518611,
     0.57977158880289315,  0.59749445964311176,  0.61635206659395714,
     0.63645837756118928,  0.65794341695608793,  0.68095628633678873,
     0.7056689323996056,   0.732280905968302,    0.76102546157076056,
     0.79217750689012745,  0.82606418556873218,  0.863079314746192,
     0.90370369615405022,  0.94853478566586147,  0.99833211723630522,
     1.0540910760488047,   1.1171721225188114},

    /* Mask Parameter: MappedSteer_WhlRghtTbl
     * Referenced by: '<S78>/right_whl_ang'
     */
    {-1.1171721225188114,  -1.0540910760488047,  -0.99833211723630522,
     -0.94853478566586147, -0.90370369615405022, -0.863079314746192,
     -0.82606418556873218, -0.79217750689012745, -0.76102546157076056,
     -0.732280905968302,   -0.7056689323996056,  -0.68095628633678873,
     -0.65794341695608793, -0.63645837756118928, -0.61635206659395714,
     -0.59749445964311176, -0.57977158880289315, -0.56308309432518611,
     -0.54734022237780211, -0.53246417378847477, -0.51838473134340246,
     -0.50503911170966642, -0.49237099939549028, -0.48032972906348487,
     -0.46886959036600762, -0.45794923261335435, 0.0,
     0.373226952870365,    0.38063136561429373,  0.38835238684412088,
     0.39641216042473093,  0.40483509757823993,  0.41364819575564932,
     0.42288141667516238,  0.43256813556993323,  0.44274568241466822,
     0.45345599555143046,  0.46474641825791085,  0.47667068066966511,
     0.48929011854352988,  0.50267520583123992,  0.51690750176874112,
     0.53208216345218118,  0.54831123037302976,  0.56572799768958493,
     0.584492948950166,    0.60480198654226847,  0.62689814459717252,
     0.6510887717564301,   0.6777716536893118,   0.70747650218712121,
     0.7409345058357546,   0.77920326382515159},

    /* Mask Parameter: DynamicSteering_b1
     * Referenced by: '<S54>/b'
     */
    0.001,

    /* Mask Parameter: DynamicSteering_b2
     * Referenced by: '<S15>/Gain2'
     */
    1.0,

    /* Mask Parameter: DynamicSteering_b3
     * Referenced by: '<S15>/Gain1'
     */
    0.001,

    /* Mask Parameter: DynamicSteering_beta_l
     * Referenced by: '<S54>/Constant'
     */
    0.1,

    /* Mask Parameter: DynamicSteering_beta_u
     * Referenced by: '<S54>/Constant1'
     */
    0.1,

    /* Mask Parameter: CompareToConstant_const
     * Referenced by: '<S66>/Constant'
     */
    0.01,

    /* Mask Parameter: DynamicSteering_k1
     * Referenced by: '<S54>/k'
     */
    60.0,

    /* Mask Parameter: DynamicSteering_omega_c
     * Referenced by: '<S17>/Constant'
     */
    200.0,

    /* Mask Parameter: DynamicSteering_omega_o
     * Referenced by:
     *   '<S51>/Constant2'
     *   '<S53>/Constant5'
     */
    0.0,

    /* Mask Parameter: DynamicSteering_theta_o
     * Referenced by:
     *   '<S51>/Constant1'
     *   '<S53>/Constant4'
     */
    0.0,

    /* Mask Parameter: div0protectpoly_thresh
     * Referenced by:
     *   '<S19>/Constant'
     *   '<S20>/Constant'
     */
    0.01,

    /* Mask Parameter: div0protectpoly_thresh_j
     * Referenced by:
     *   '<S28>/Constant'
     *   '<S29>/Constant'
     */
    0.01,

    /* Mask Parameter: div0protectpoly_thresh_k
     * Referenced by:
     *   '<S31>/Constant'
     *   '<S32>/Constant'
     */
    0.01,

    /* Mask Parameter: div0protectabspoly_thresh
     * Referenced by:
     *   '<S59>/Constant'
     *   '<S60>/Constant'
     */
    0.001,

    /* Mask Parameter: div0protectpoly_thresh_i
     * Referenced by:
     *   '<S61>/Constant'
     *   '<S62>/Constant'
     */
    1.0E-6,

    /* Mask Parameter: div0protectabspoly_thresh_o
     * Referenced by:
     *   '<S64>/Constant'
     *   '<S65>/Constant'
     */
    1.0E-6,

    /* Mask Parameter: div0protectabspoly1_thresh
     * Referenced by:
     *   '<S68>/Constant'
     *   '<S69>/Constant'
     */
    1.0E-6,

    /* Computed Parameter: TransferFcn_A
     * Referenced by: '<S76>/Transfer Fcn'
     */
    {-4.0005828, -161.91290025},

    /* Computed Parameter: TransferFcn_C
     * Referenced by: '<S76>/Transfer Fcn'
     */
    {0.0, 161.91290025},

    /* Expression: 1
     * Referenced by: '<S4>/secondOrderSwitch'
     */
    1.0,

    /* Expression: 0.5
     * Referenced by: '<S4>/Switch'
     */
    0.5,

    /* Expression: 0
     * Referenced by: '<S79>/Constant1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S4>/Constant'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S18>/Switch1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S27>/Switch1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S30>/Switch1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S58>/Switch1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S2>/Constant1'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S17>/Integrator1'
     */
    0.0,

    /* Expression: pi/2
     * Referenced by: '<S26>/Constant8'
     */
    1.5707963267948966,

    /* Expression: 2
     * Referenced by: '<S26>/Constant6'
     */
    2.0,

    /* Expression: pi/2
     * Referenced by: '<S25>/Constant8'
     */
    1.5707963267948966,

    /* Expression: -0.001
     * Referenced by: '<S13>/Dead Zone'
     */
    -0.001,

    /* Expression: 0.001
     * Referenced by: '<S13>/Dead Zone'
     */
    0.001,

    /* Expression: 2
     * Referenced by: '<S25>/Constant6'
     */
    2.0,

    /* Expression: 1
     * Referenced by: '<S10>/index'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S10>/Switch'
     */
    0.0,

    /* Expression: pi/2
     * Referenced by: '<S24>/Constant8'
     */
    1.5707963267948966,

    /* Expression: -1
     * Referenced by: '<S21>/Gain'
     */
    -1.0,

    /* Expression: 2
     * Referenced by: '<S24>/Constant6'
     */
    2.0,

    /* Expression: 0
     * Referenced by: '<S10>/Switch1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S18>/Constant'
     */
    1.0,

    /* Expression: pi/2
     * Referenced by: '<S24>/Constant11'
     */
    1.5707963267948966,

    /* Expression: 1
     * Referenced by: '<S27>/Constant'
     */
    1.0,

    /* Expression: 1
     * Referenced by: '<S16>/index'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S16>/Switch1'
     */
    0.0,

    /* Expression: pi/2
     * Referenced by: '<S25>/Constant11'
     */
    1.5707963267948966,

    /* Expression: 1
     * Referenced by: '<S30>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S16>/Switch'
     */
    0.0,

    /* Expression: -1
     * Referenced by: '<S21>/Gain1'
     */
    -1.0,

    /* Expression: -0.001
     * Referenced by: '<S13>/Dead Zone2'
     */
    -0.001,

    /* Expression: 0.001
     * Referenced by: '<S13>/Dead Zone2'
     */
    0.001,

    /* Expression: 0
     * Referenced by: '<S56>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S54>/domega_o1'
     */
    0.0,

    /* Expression: 1
     * Referenced by: '<S58>/Constant'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S54>/Switch'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S55>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S54>/domega_o'
     */
    0.0,

    /* Expression: 10
     * Referenced by: '<S54>/omega_c'
     */
    10.0,

    /* Expression: 1
     * Referenced by: '<S55>/Reset'
     */
    1.0,

    /* Expression: 10
     * Referenced by: '<S54>/omega_c1'
     */
    10.0,

    /* Expression: 1
     * Referenced by: '<S56>/Reset'
     */
    1.0,

    /* Expression: 0
     * Referenced by: '<S1>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S1>/Memory'
     */
    0.0,

    /* Expression: 0
     * Referenced by: '<S1>/Memory'
     */
    0.0,

    /* Computed Parameter: TrqAssistTbl_maxIndex
     * Referenced by: '<S12>/TrqAssistTbl'
     */
    {2U, 1U}};
