// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <cstdint>
#include <exception>
#include <map>
#include "inc/car_common.h"

#ifdef _WIN32
#  pragma warning(disable : 4251)
#endif  // _WIN32

#ifndef TNOParserMarco
#  define TNOParserMarco(KeyVal, TNODb, func) tx_car::TNOParser(KeyVal.first, KeyVal.second, TNODb, func)
#endif  // !TNOParserMarco

namespace tx_car {
const char *TNODirect(const char *ptr);
const char m_TNO_Version[] = "62";
const std::vector<std::string> m_TNO_Version_List{"62", "61", "PAC2002"};

typedef std::map<std::string, std::string> TNOPairs;
typedef std::map<std::string, TNOPairs> TNODb;

struct TNOTire_Model {
 public:
  const std::string id = "[MODEL]";
  std::pair<const std::string, std::string> FITTYP = {"FITTYP", "62"};  // Magic Formula Version number
  std::pair<const std::string, std::string> TYRESIDE = {"TYRESIDE", "Left"};
  std::pair<const std::string, double> LONGVL = {"LONGVL", 16.7};                  // nominal speed
  std::pair<const std::string, double> VXLOW = {"VXLOW", 1.0};                     // Lower boundary of slip calculation
  std::pair<const std::string, double> ROAD_INCREMENT = {"ROAD_INCREMENT", 0.01};  // Increment in road sampling
  std::pair<const std::string, int> ROAD_DIRECTION = {"ROAD_DIRECTION", 1};        // Direction of travelled distance

  bool parseFrom(const TNODb &models);
};

struct TNOTire_Dimenson {
  const std::string id = "[DIMENSION]";
  std::pair<const std::string, double> UNLOADED_RADIUS = {"UNLOADED_RADIUS", 0.3135};  // Free tyre radius
  std::pair<const std::string, double> WIDTH = {"WIDTH", 0.205};              // Nominal section width of the tyre
  std::pair<const std::string, double> RIM_RADIUS = {"RIM_RADIUS", 0.1905};   // Nominal rim radius
  std::pair<const std::string, double> RIM_WIDTH = {"RIM_WIDTH", 0.152};      // Rim width
  std::pair<const std::string, double> ASPECT_RATIO = {"ASPECT_RATIO", 0.6};  // Nominal aspect ratio

  bool parseFrom(const TNODb &models);
};

struct TNOTire_OperatingCondition {
  const std::string id = "[OPERATING_CONDITIONS]";
  std::pair<const std::string, double> INFLPRES = {"INFLPRES", 220000};  // Tyre inflation pressure
  std::pair<const std::string, double> NOMPRES = {"NOMPRES", 220000};    // Nominal tyre inflation pressure

  bool parseFrom(const TNODb &models);
};

struct TNOTire_Inertia {
  const std::string id = "[INERTIA]";
  std::pair<const std::string, double> MASS = {"MASS", 9.3};              // Tyre Mass
  std::pair<const std::string, double> IXX = {"IXX", 0.391};              // Tyre diametral moment of inertia
  std::pair<const std::string, double> IYY = {"IYY", 0.736};              // Tyre polar moment of inertia
  std::pair<const std::string, double> BELT_MASS = {"BELT_MASS", 7.698};  // Belt mass
  std::pair<const std::string, double> BELT_IXX = {"BELT_IXX", 0.3519};   // Belt diametral moment of inertia
  std::pair<const std::string, double> BELT_IYY = {"BELT_IYY", 0.4889};   // Belt polar moment of inertia
  std::pair<const std::string, double> GRAVITY = {"GRAVITY", -9.81};      // Gravity acting on belt in Z direction

  bool parseFrom(const TNODb &models);
};

struct TNOTire_Vertical {
  const std::string id = "[VERTICAL]";
  std::pair<const std::string, double> FNOMIN = {"FNOMIN", 4000};                              // Nominal wheel load
  std::pair<const std::string, double> VERTICAL_STIFFNESS = {"VERTICAL_STIFFNESS", 209651.5};  // Tyre vertical
                                                                                               // stiffness
  std::pair<const std::string, double> VERTICAL_DAMPING = {"VERTICAL_DAMPING", 500};           // Tyre vertical damping
  std::pair<const std::string, double> MC_CONTOUR_A = {"MC_CONTOUR_A", 0.0};  // Motorcycle contour ellips A
  std::pair<const std::string, double> MC_CONTOUR_B = {"MC_CONTOUR_B", 0.0};  // Motorcycle contour ellips B
  std::pair<const std::string, double> BREFF = {"BREFF", 8.39};     // Low load stiffness effective rolling radius
  std::pair<const std::string, double> DREFF = {"DREFF", 0.2587};   // Peak value of effective rolling radius
  std::pair<const std::string, double> FREFF = {"FREFF", 0.07355};  // High load stiffness effective rolling radius
  std::pair<const std::string, double> Q_RE0 = {"Q_RE0", 0.9974};  // Ratio of free tyre radius with nominal tyre radius
  std::pair<const std::string, double> Q_V1 = {"Q_V1", 7.724E-4};  // Tyre radius increase with speed
  std::pair<const std::string, double> Q_V2 = {"Q_V2", 0.04663};   // Vertical stiffness increase with speed
  std::pair<const std::string, double> Q_FZ2 = {"Q_FZ2", 15.416};  // Quadratic term in load vs. deflection
  std::pair<const std::string, double> Q_FCX = {"Q_FCX", 0.13682};  // Longitudinal force influence on vertical
                                                                    // stiffness
  std::pair<const std::string, double> Q_FCY = {"Q_FCY", 0.1078};   // Lateral force influence on vertical stiffness
  std::pair<const std::string, double> Q_CAM = {"Q_CAM", 0};        // Stiffness reduction due to camber
  std::pair<const std::string, double> PFZ1 = {"PFZ1", 0.7097};     // Pressure effect on vertical stiffness
  std::pair<const std::string, double> Q_FCY2 = {
      "Q_FCY2", -0.4751};  // Explicit load dependency for including the lateral force influence on vertical stiffness
  std::pair<const std::string, double> Q_CAM1 = {
      "Q_CAM1", 85.19};  // Linear load dependent camber angle influence on vertical stiffness
  std::pair<const std::string, double> Q_CAM2 = {
      "Q_CAM2", 257.4};  // Quadratic load dependent camber angle influence on vertical stiffness
  std::pair<const std::string, double> Q_CAM3 = {
      "Q_CAM3", 0.5119};  // Linear load and camber angle dependent reduction on vertical stiffness
  std::pair<const std::string, double> Q_FYS1 = {
      "Q_FYS1", -20496.4};  // ombined camber angle and side slip angle effect on vertical stiffness(constant)
  std::pair<const std::string, double> Q_FYS2 = {
      "Q_FYS2", -60000};  // Combined camber angle and side slip angle linear effect on vertical stiffness
  std::pair<const std::string, double> Q_FYS3 = {
      "Q_FYS3", 88211.7};  // Combined camber angle and side slip angle quadratic effect on vertical stiffness
  std::pair<const std::string, double> BOTTOM_OFFST = {"BOTTOM_OFFST",
                                                       0.01};  // Distance to rim when bottoming starts to occur
  std::pair<const std::string, double> BOTTOM_STIFF = {"BOTTOM_STIFF", 2000000};  // Vertical stiffness of bottomed tyre

  bool parseFrom(const TNODb &models);
};

struct TNOTire_Structural {
  const std::string id = "[STRUCTURAL]";
  std::pair<const std::string, double> LONGITUDINAL_STIFFNESS = {"LONGITUDINAL_STIFFNESS",
                                                                 381913.4};  // Tyre overall longitudinal stiffness
  std::pair<const std::string, double> LATERAL_STIFFNESS = {"LATERAL_STIFFNESS",
                                                            157632.5};           // Tyre overall lateral stiffness
  std::pair<const std::string, double> YAW_STIFFNESS = {"YAW_STIFFNESS", 5000};  // Tyre overall yaw stiffness
  std::pair<const std::string, double> FREQ_LONG = {"FREQ_LONG",
                                                    76.63};             // Undamped frequency fore/aft and vertical mode
  std::pair<const std::string, double> FREQ_LAT = {"FREQ_LAT", 41.47};  // Undamped frequency lateral mode
  std::pair<const std::string, double> FREQ_YAW = {"FREQ_YAW", 53.46};  // Undamped frequency yaw and camber mode
  std::pair<const std::string, double> FREQ_WINDUP = {"FREQ_WINDUP", 67.73};  // Undamped frequency wind-up mode
  std::pair<const std::string, double> DAMP_LONG = {"DAMP_LONG",
                                                    0.04881};  // Dimensionless damping fore/aft and vertical mode
  std::pair<const std::string, double> DAMP_LAT = {"DAMP_LAT", 0.0050};  // Dimensionless damping lateral mode
  std::pair<const std::string, double> DAMP_YAW = {"DAMP_YAW", 0.1};     // Dimensionless damping yaw and camber mode
  std::pair<const std::string, double> DAMP_WINDUP = {"DAMP_WINDUP", 0.05157};  // Dimensionless damping wind-up mode
  std::pair<const std::string, double> DAMP_RESIDUAL = {"DAMP_RESIDUAL",
                                                        0.0020};  // Residual damping (proportional to stiffness)
  std::pair<const std::string, double> DAMP_VLOW = {
      "DAMP_VLOW", 0.0010};  // Additional low speed damping (proportional to stiffness)
  std::pair<const std::string, double> Q_BVX = {"Q_BVX",
                                                0};  // Load and speed influence on in-plane translation stiffness
  std::pair<const std::string, double> Q_BVT = {"Q_BVT", 0};  // Load and speed influence on in-plane rotation stiffness
  std::pair<const std::string, double> PCFX1 = {
      "PCFX1", 0.18677};  // Tyre overall longitudinal stiffness vertical deflection dependency linear term
  std::pair<const std::string, double> PCFX2 = {
      "PCFX2", 0};  // Tyre overall longitudinal stiffness vertical deflection dependency quadratic term
  std::pair<const std::string, double> PCFX3 = {"PCFX3", 0};  // Tyre overall longitudinal stiffness pressure dependency
  std::pair<const std::string, double> PCFY1 = {
      "PCFY1", 0.16365};  // Tyre overall lateral stiffness vertical deflection dependency linear term
  std::pair<const std::string, double> PCFY2 = {
      "PCFY2", 0};  // Tyre overall lateral stiffness vertical deflection dependency quadratic term
  std::pair<const std::string, double> PCFY3 = {"PCFY3", 0.24993};  // Tyre overall lateral stiffness pressure
                                                                    // dependency
  std::pair<const std::string, double> PCMZ1 = {"PCMZ1", 0};        // Tyre overall yaw stiffness pressure dependency

  bool parseFrom(const TNODb &models);
};

struct TNOTire_ContactPatch {
  const std::string id = "[CONTACT_PATCH]";
  std::pair<const std::string, double> Q_RA1 = {"Q_RA1", 0.6715};   // Square root term in contact length equation
  std::pair<const std::string, double> Q_RA2 = {"Q_RA2", 0.7308};   // Linear term in contact length equation
  std::pair<const std::string, double> Q_RB1 = {"Q_RB1", 1.0588};   // Root term in contact width equation
  std::pair<const std::string, double> Q_RB2 = {"Q_RB2", -1.1863};  // Linear term in contact width equation
  std::pair<const std::string, double> ELLIPS_SHIFT = {"ELLIPS_SHIFT",
                                                       0.8328};  // Scaling of distance between front and rear ellipsoid
  std::pair<const std::string, double> ELLIPS_LENGTH = {"ELLIPS_LENGTH", 1.4634};     // Semi major axis of ellipsoid
  std::pair<const std::string, double> ELLIPS_HEIGHT = {"ELLIPS_HEIGHT", 0.9551};     // Semi minor axis of ellipsoid
  std::pair<const std::string, double> ELLIPS_ORDER = {"ELLIPS_ORDER", 1.5257};       // Order of ellipsoid
  std::pair<const std::string, double> ELLIPS_MAX_STEP = {"ELLIPS_MAX_STEP", 0.025};  // Maximum height of road step
  std::pair<const std::string, double> ELLIPS_NWIDTH = {"ELLIPS_NWIDTH", 10};         // Number of parallel ellipsoids
  std::pair<const std::string, double> ELLIPS_NLENGTH = {"ELLIPS_NLENGTH",
                                                         10};   // Number of ellipsoids at sides of contact patch
  std::pair<const std::string, double> ENV_C1 = {"ENV_C1", 0};  // Effective height attenuation
  std::pair<const std::string, double> ENV_C2 = {"ENV_C2", 0};  // Effective plane angle attenuation

  bool parseFrom(const TNODb &models);
};

struct TNOTire_InflationPressureRng {
  const std::string id = "[INFLATION_PRESSURE_RANGE]";
  std::pair<const std::string, double> PRESMIN = {"PRESMIN", 10000};    // Minimum valid tyre inflation pressure
  std::pair<const std::string, double> PRESMAX = {"PRESMAX", 1000000};  // Minimum valid tyre inflation pressure

  bool parseFrom(const TNODb &models);
};

struct TNOTire_VerticalForceRng {
  const std::string id = "[VERTICAL_FORCE_RANGE]";
  std::pair<const std::string, double> FZMIN = {"FZMIN", 100};    // Minimum allowed wheel load
  std::pair<const std::string, double> FZMAX = {"FZMAX", 10000};  // Maximum allowed wheel load

  bool parseFrom(const TNODb &models);
};

struct TNOTire_LongSlipRng {
  const std::string id = "[LONG_SLIP_RANGE]";
  std::pair<const std::string, double> KPUMIN = {"KPUMIN", -1.5};  // Minimum valid wheel slip
  std::pair<const std::string, double> KPUMAX = {"KPUMAX", 1.5};   // Maximum valid wheel slip

  bool parseFrom(const TNODb &models);
};

struct TNOTire_SlipAngRng {
  const std::string id = "[SLIP_ANGLE_RANGE]";
  std::pair<const std::string, double> ALPMIN = {"ALPMIN", -1.5};  // Minimum valid slip angle
  std::pair<const std::string, double> ALPMAX = {"ALPMAX", 1.5};   // Maximum valid slip angle

  bool parseFrom(const TNODb &models);
};

struct TNOTire_InclinationAngRng {
  const std::string id = "[INCLINATION_ANGLE_RANGE]";
  std::pair<const std::string, double> CAMMIN = {"CAMMIN", -0.175};  // Minimum valid camber angle
  std::pair<const std::string, double> CAMMAX = {"CAMMAX", 0.175};   // Maximum valid camber angle

  bool parseFrom(const TNODb &models);
};

struct TNOTire_ScalingCoef {
  const std::string id = "[SCALING_COEFFICIENTS]";
  std::pair<const std::string, double> LFZO = {"LFZO", 1};    // Scale factor of nominal(rated) load
  std::pair<const std::string, double> LCX = {"LCX", 1};      // Scale factor of Fx shape factor
  std::pair<const std::string, double> LMUX = {"LMUX", 1};    // Scale factor of Fx peak friction coefficient
  std::pair<const std::string, double> LEX = {"LEX", 1};      // Scale factor of Fx curvature factor
  std::pair<const std::string, double> LKX = {"LKX", 1};      // Scale factor of Fx slip stiffness
  std::pair<const std::string, double> LHX = {"LHX", 1};      // Scale factor of Fx horizontal shift
  std::pair<const std::string, double> LVX = {"LVX", 1};      // Scale factor of Fx vertical shift
  std::pair<const std::string, double> LCY = {"LCY", 1};      // Scale factor of Fy shape factor
  std::pair<const std::string, double> LMUY = {"LMUY", 1};    // Scale factor of Fy peak friction coefficient
  std::pair<const std::string, double> LEY = {"LEY", 1};      // Scale factor of Fy curvature factor
  std::pair<const std::string, double> LKY = {"LKY", 1};      // Scale factor of Fy cornering stiffness
  std::pair<const std::string, double> LKYC = {"LKYC", 1};    // Scale factor of Fy camber stiffness
  std::pair<const std::string, double> LKZC = {"LKZC", 1};    // Scale factor of Mz camber stiffness
  std::pair<const std::string, double> LHY = {"LHY", 1};      // Scale factor of Fy horizontal shift
  std::pair<const std::string, double> LVY = {"LVY", 1};      // Scale factor of Fy vertical shift
  std::pair<const std::string, double> LTR = {"LTR", 1};      // Scale factor of Peak of pneumatic trail
  std::pair<const std::string, double> LRES = {"LRES", 1};    // Scale factor for offset of Mz residual torque
  std::pair<const std::string, double> LXAL = {"LXAL", 1};    // Scale factor of alpha influence on Fx
  std::pair<const std::string, double> LYKA = {"LYKA", 1};    // Scale factor of kappa influence on Fy
  std::pair<const std::string, double> LVYKA = {"LVYKA", 1};  // Scale factor of kappa induced Fy
  std::pair<const std::string, double> LS = {"LS", 1};        // Scale factor of Moment arm of Fx
  std::pair<const std::string, double> LMX = {"LMX", 1};      // Scale factor of Mx overturning moment
  std::pair<const std::string, double> LVMX = {"LVMX", 1};    // Scale factor of Mx vertical shift
  std::pair<const std::string, double> LMY = {"LMY", 1};      // Scale factor of rolling resistance torque
  std::pair<const std::string, double> LMP = {"LMP", 1};      // Scale factor of Mz parking torque

  bool parseFrom(const TNODb &models);
};

struct TNOTire_LongCoef {
  const std::string id = "[LONGITUDINAL_COEFFICIENTS]";
  std::pair<const std::string, double> PCX1 = {"PCX1", 1.579};      // Shape factor Cfx for longitudinal force
  std::pair<const std::string, double> PDX1 = {"PDX1", 1.0422};     // Longitudinal friction Mux at Fznom
  std::pair<const std::string, double> PDX2 = {"PDX2", -0.0827};    // Variation of friction Mux with load
  std::pair<const std::string, double> PDX3 = {"PDX3", 0};          // Variation of friction Mux with camber
  std::pair<const std::string, double> PEX1 = {"PEX1", 0.11185};    // Longitudinal curvature Efx at Fznom
  std::pair<const std::string, double> PEX2 = {"PEX2", 0.3127};     // Variation of curvature Efx with load
  std::pair<const std::string, double> PEX3 = {"PEX3", 0};          // Variation of curvature Efx with load squared
  std::pair<const std::string, double> PEX4 = {"PEX4", 0.001603};   // Factor in curvature Efx while driving
  std::pair<const std::string, double> PKX1 = {"PKX1", 21.69};      // Longitudinal slip stiffness Kfx/Fz at Fznom
  std::pair<const std::string, double> PKX2 = {"PKX2", 13.774};     // Variation of slip stiffness Kfx/Fz with load
  std::pair<const std::string, double> PKX3 = {"PKX3", -0.4119};    // Exponent in slip stiffness Kfx/Fz with load
  std::pair<const std::string, double> PHX1 = {"PHX1", 2.1585E-4};  // Horizontal shift Shx at Fznom
  std::pair<const std::string, double> PHX2 = {"PHX2", 0.0011538};  // Variation of shift Shx with load
  std::pair<const std::string, double> PVX1 = {"PVX1", 1.5973E-5};  // Vertical shift Svx/Fz at Fznom
  std::pair<const std::string, double> PVX2 = {"PVX2", 1.043E-4};   // Variation of shift Svx/Fz with load
  std::pair<const std::string, double> PPX1 = {
      "PPX1", -0.3489};  // linear influence of inflation pressure on longitudinal slip stiffness
  std::pair<const std::string, double> PPX2 = {
      "PPX2", 0.382};  // quadratic influence of inflation pressure on longitudinal slip stiffness
  std::pair<const std::string, double> PPX3 = {
      "PPX3", -0.09634};  // linear influence of inflation pressure on peak longitudinal friction
  std::pair<const std::string, double> PPX4 = {
      "PPX4", 0.06447};  // quadratic influence of inflation pressure on peak longitudinal friction
  std::pair<const std::string, double> RBX1 = {"RBX1", 12.531};    // Slope factor for combined slip Fx reduction
  std::pair<const std::string, double> RBX2 = {"RBX2", 9.532};     // Variation of slope Fx reduction with kappa
  std::pair<const std::string, double> RBX3 = {"RBX3", 0};         // Influence of camber on stiffness for Fx combined
  std::pair<const std::string, double> RCX1 = {"RCX1", 1.019};     // Shape factor for combined slip Fx reduction
  std::pair<const std::string, double> REX1 = {"REX1", -0.4487};   // Curvature factor of combined Fx
  std::pair<const std::string, double> REX2 = {"REX2", -0.4726};   // Curvature factor of combined Fx with load
  std::pair<const std::string, double> RHX1 = {"RHX1", 2.374E-4};  // Shift factor for combined slip Fx reduction

  bool parseFrom(const TNODb &models);
};

struct TNOTire_OverTurnCoef {
  const std::string id = "[OVERTURNING_COEFFICIENTS]";
  std::pair<const std::string, double> QSX1 = {"QSX1", -0.007773};  // Vertical shift of overturning moment
  std::pair<const std::string, double> QSX2 = {"QSX2", 0.9557};     // Camber induced overturning couple
  std::pair<const std::string, double> QSX3 = {"QSX3", 0.01587};    // Fy induced overturning couple
  std::pair<const std::string, double> QSX4 = {"QSX4", 0.1244};     // Mixed load lateral force and camber on Mx
  std::pair<const std::string, double> QSX5 = {"QSX5", 1.016};      // Load effect on Mx with lateral force and camber
  std::pair<const std::string, double> QSX6 = {"QSX6", 20};         // B - factor of load with Mx
  std::pair<const std::string, double> QSX7 = {"QSX7", 0.3059};     // Camber with load on Mx
  std::pair<const std::string, double> QSX8 = {"QSX8", -0.04331};   // Lateral force with load on Mx
  std::pair<const std::string, double> QSX9 = {"QSX9", 0.6269};     // B - factor of lateral force with load on Mx
  std::pair<const std::string, double> QSX10 = {"QSX10", 0.14217};  // Vertical force with camber on Mx
  std::pair<const std::string, double> QSX11 = {"QSX11", 0.3477};   // B - factor of vertical force with camber on Mx
  std::pair<const std::string, double> QSX12 = {"QSX12", 0};        // Camber squared induced overturning moment
  std::pair<const std::string, double> QSX13 = {"QSX13", 0};        // Lateral force induced overturning moment
  std::pair<const std::string, double> QSX14 = {"QSX14", 0};  // Lateral force induced overturning moment with camber
  std::pair<const std::string, double> PPMX1 = {"PPMX1", 0};  // Influence of inflation pressure on overturning moment

  bool parseFrom(const TNODb &models);
};

struct TNOTire_LatCoef {
  const std::string id = "[LATERAL_COEFFICIENTS]";
  std::pair<const std::string, double> PCY1 = {"PCY1", 1.3332};      // Shape factor Cfy for lateral forces
  std::pair<const std::string, double> PDY1 = {"PDY1", 0.8784};      // Lateral friction Muy
  std::pair<const std::string, double> PDY2 = {"PDY2", -0.06445};    // Variation of friction Muy with load
  std::pair<const std::string, double> PDY3 = {"PDY3", 0};           // Variation of friction Muy with squared camber
  std::pair<const std::string, double> PEY1 = {"PEY1", -0.8227};     // Lateral curvature Efy at Fznom
  std::pair<const std::string, double> PEY2 = {"PEY2", -0.6062};     // Variation of curvature Efy with load
  std::pair<const std::string, double> PEY3 = {"PEY3", 0.09825};     // Zero order camber dependency of curvature Efy
  std::pair<const std::string, double> PEY4 = {"PEY4", -6.546};      // Variation of curvature Efy with camber
  std::pair<const std::string, double> PEY5 = {"PEY5", 0};           // Variation of curvature Efy with camber squared
  std::pair<const std::string, double> PKY1 = {"PKY1", -15.314};     // Maximum value of stiffness Kfy / Fznom
  std::pair<const std::string, double> PKY2 = {"PKY2", 1.7044};      // Load at which Kfy reaches maximum value
  std::pair<const std::string, double> PKY3 = {"PKY3", 0.36986};     // Variation of Kfy / Fznom with camber
  std::pair<const std::string, double> PKY4 = {"PKY4", 1.9896};      // Curvature of stiffness Kfy
  std::pair<const std::string, double> PKY5 = {"PKY5", 0};           // Peak stiffness variation with camber squared
  std::pair<const std::string, double> PKY6 = {"PKY6", -0.9004};     // Fy camber stiffness factor
  std::pair<const std::string, double> PKY7 = {"PKY7", -0.2328};     // Vertical load dependency of camber stiffness
  std::pair<const std::string, double> PHY1 = {"PHY1", -0.0018043};  // Horizontal shift Shy at Fznom
  std::pair<const std::string, double> PHY2 = {"PHY2", 0.003518};    // Variation of shift Shy with load
  std::pair<const std::string, double> PVY1 = {"PVY1", -0.006642};   // Vertical shift in Svy / Fz at Fznom
  std::pair<const std::string, double> PVY2 = {"PVY2", 0.03597};     // Variation of shift Svy / Fz with load
  std::pair<const std::string, double> PVY3 = {"PVY3", -0.1621};     // Variation of shift Svy / Fz with camber
  std::pair<const std::string, double> PVY4 = {"PVY4", -0.4908};     // Variation of shift Svy / Fz with camber and load
  std::pair<const std::string, double> PPY1 = {"PPY1",
                                               -0.6249};  // influence of inflation pressure on cornering stiffness
  std::pair<const std::string, double> PPY2 = {
      "PPY2", -0.06535};  // influence of inflation pressure on dependency of nominal tyre load on cornering stiffness
  std::pair<const std::string, double> PPY3 = {
      "PPY3", -0.1668};  // linear influence of inflation pressure on lateral peak friction
  std::pair<const std::string, double> PPY4 = {
      "PPY4", 0.2834};  // quadratic influence of inflation pressure on lateral peak friction
  std::pair<const std::string, double> PPY5 = {"PPY5", 0};       // Influence of inflation pressure on camber stiffness
  std::pair<const std::string, double> RBY1 = {"RBY1", 10.615};  // Slope factor for combined Fy reduction
  std::pair<const std::string, double> RBY2 = {"RBY2", 7.705};   // Variation of slope Fy reduction with alpha
  std::pair<const std::string, double> RBY3 = {"RBY3", 3.433E-6};  // Shift term for alpha in slope Fy reduction
  std::pair<const std::string, double> RBY4 = {"RBY4", 0};         // Influence of camber on stiffness of Fy combined
  std::pair<const std::string, double> RCY1 = {"RCY1", 1.0585};    // Shape factor for combined Fy reduction
  std::pair<const std::string, double> REY1 = {"REY1", 0.3132};    // Curvature factor of combined Fy
  std::pair<const std::string, double> REY2 = {"REY2", 7.63E-6};   // Curvature factor of combined Fy with load
  std::pair<const std::string, double> RHY1 = {"RHY1", 0.011646};  // Shift factor for combined Fy reduction
  std::pair<const std::string, double> RHY2 = {"RHY2", 7.63E-6};   // Shift factor for combined Fy reduction with load
  std::pair<const std::string, double> RVY1 = {"RVY1", 0.05654};   // Kappa induced side force Svyk / Muy * Fz at Fznom
  std::pair<const std::string, double> RVY2 = {"RVY2", 7.63E-6};   // Variation of Svyk / Muy * Fz with load
  std::pair<const std::string, double> RVY3 = {"RVY3", 0};         // Variation of Svyk / Muy * Fz with camber
  std::pair<const std::string, double> RVY4 = {"RVY4", 99.9};      // Variation of Svyk / Muy * Fz with alpha
  std::pair<const std::string, double> RVY5 = {"RVY5", 2};         // Variation of Svyk / Muy * Fz with kappa
  std::pair<const std::string, double> RVY6 = {"RVY6", 15.814};    // Variation of Svyk / Muy * Fz with atan(kappa)

  bool parseFrom(const TNODb &models);
};

struct TNOTire_RollingCoef {
  const std::string id = "[ROLLING_COEFFICIENTS]";
  std::pair<const std::string, double> QSY1 = {"QSY1", 0.00702};   // Rolling resistance torque coefficient
  std::pair<const std::string, double> QSY2 = {"QSY2", 0};         // Rolling resistance torque depending on Fx
  std::pair<const std::string, double> QSY3 = {"QSY3", 0.001515};  // Rolling resistance torque depending on speed
  std::pair<const std::string, double> QSY4 = {"QSY4", 8.514E-5};  // Rolling resistance torque depending on speed ^ 4
  std::pair<const std::string, double> QSY5 = {"QSY5", 0};  // Rolling resistance torque depending on camber squared
  std::pair<const std::string, double> QSY6 = {"QSY6",
                                               0};  // Rolling resistance torque depending on load and camber squared
  std::pair<const std::string, double> QSY7 = {"QSY7", 0.9008};  // Rolling resistance torque coefficient load
                                                                 // dependency
  std::pair<const std::string, double> QSY8 = {"QSY8",
                                               -0.4089};  // Rolling resistance torque coefficient pressure dependency

  bool parseFrom(const TNODb &models);
};

struct TNOTire_AlignCoef {
  const std::string id = "[ALIGNING_COEFFICIENTS]";
  std::pair<const std::string, double> QBZ1 = {"QBZ1", 11.692};    // Trail slope factor for trail Bpt at Fznom
  std::pair<const std::string, double> QBZ2 = {"QBZ2", -1.3225};   // Variation of slope Bpt with load
  std::pair<const std::string, double> QBZ3 = {"QBZ3", 0};         // Variation of slope Bpt with load squared
  std::pair<const std::string, double> QBZ4 = {"QBZ4", 0.17595};   // Variation of slope Bpt with camber
  std::pair<const std::string, double> QBZ5 = {"QBZ5", -0.16048};  // Variation of slope Bpt with absolute camber
  std::pair<const std::string, double> QBZ9 = {"QBZ9", 29.736};  // Factor for scaling factors of slope factor Br of Mzr
  std::pair<const std::string, double> QBZ10 = {"QBZ10",
                                                0};  // Factor for dimensionless cornering stiffness of Br of Mzr
  std::pair<const std::string, double> QCZ1 = {"QCZ1", 1.2793};      // Shape factor Cpt for pneumatic trail
  std::pair<const std::string, double> QDZ1 = {"QDZ1", 0.08993};     // Peak trail Dpt = Dpt * (Fz / Fznom * R0)
  std::pair<const std::string, double> QDZ2 = {"QDZ2", -0.005979};   // Variation of peak Dpt with load
  std::pair<const std::string, double> QDZ3 = {"QDZ3", 0.3787};      // Variation of peak Dpt with camber
  std::pair<const std::string, double> QDZ4 = {"QDZ4", 0};           // Variation of peak Dpt with camber squared
  std::pair<const std::string, double> QDZ6 = {"QDZ6", 0.0016198};   // Peak residual torque Dmr = Dmr / (Fz*R0)
  std::pair<const std::string, double> QDZ7 = {"QDZ7", -0.0021103};  // Variation of peak factor Dmr with load
  std::pair<const std::string, double> QDZ8 = {"QDZ8", -0.1404};     // Variation of peak factor Dmr with camber
  std::pair<const std::string, double> QDZ9 = {"QDZ9", 0.008132};   // Variation of peak factor Dmr with camber and load
  std::pair<const std::string, double> QDZ10 = {"QDZ10", 0};        // Variation of peak factor Dmr with camber squared
  std::pair<const std::string, double> QDZ11 = {"QDZ11", 0};        // Variation of Dmr with camber squared and load
  std::pair<const std::string, double> QEZ1 = {"QEZ1", -2.17};      // Trail curvature Ept at Fznom
  std::pair<const std::string, double> QEZ2 = {"QEZ2", 1.0036};     // Variation of curvature Ept with load
  std::pair<const std::string, double> QEZ3 = {"QEZ3", 0};          // Variation of curvature Ept with load squared
  std::pair<const std::string, double> QEZ4 = {"QEZ4", 0.2795};     // Variation of curvature Ept with sign of Alpha - t
  std::pair<const std::string, double> QEZ5 = {"QEZ5", -0.5441};    // Variation of Ept with camber and sign Alpha - t
  std::pair<const std::string, double> QHZ1 = {"QHZ1", 0.0015093};  // Trail horizontal shift Sht at Fznom
  std::pair<const std::string, double> QHZ2 = {"QHZ2", 0.0020433};  // Variation of shift Sht with load
  std::pair<const std::string, double> QHZ3 = {"QHZ3", 0.23785};    // Variation of shift Sht with camber
  std::pair<const std::string, double> QHZ4 = {"QHZ4", -0.2069};    // Variation of shift Sht with camber and load
  std::pair<const std::string, double> PPZ1 = {"PPZ1",
                                               -0.4402};  // effect of inflation pressure on length of pneumatic trail
  std::pair<const std::string, double> PPZ2 = {"PPZ2",
                                               0};  // Influence of inflation pressure on residual aligning torque
  std::pair<const std::string, double> SSZ1 = {"SSZ1", 0.00918};  // Nominal value of s / R0: effect of Fx on Mz
  std::pair<const std::string, double> SSZ2 = {"SSZ2", 0.03853};  // Variation of distance s / R0 with Fy / Fznom
  std::pair<const std::string, double> SSZ3 = {"SSZ3", 0};        // Variation of distance s / R0 with camber
  std::pair<const std::string, double> SSZ4 = {"SSZ4", 0};        // Variation of distance s / R0 with load and camber

  bool parseFrom(const TNODb &models);
};

struct TNOTire_TurnSlipCoef {
  const std::string id = "[TURNSLIP_COEFFICIENTS]";
  std::pair<const std::string, double> PDXP1 = {"PDXP1", 0.4};  // Peak Fx reduction due to spin parameter
  std::pair<const std::string, double> PDXP2 = {"PDXP2",
                                                0};  // Peak Fx reduction due to spin with varying load parameter
  std::pair<const std::string, double> PDXP3 = {"PDXP3", 0};    // Peak Fx reduction due to spin with kappa parameter
  std::pair<const std::string, double> PKYP1 = {"PKYP1", 1};    // Cornering stiffness reduction due to spin
  std::pair<const std::string, double> PDYP1 = {"PDYP1", 0.4};  // Peak Fy reduction due to spin parameter
  std::pair<const std::string, double> PDYP2 = {"PDYP2",
                                                0};  // Peak Fy reduction due to spin with varying load parameter
  std::pair<const std::string, double> PDYP3 = {"PDYP3", 0};  // Peak Fy reduction due to spin with alpha parameter
  std::pair<const std::string, double> PDYP4 = {"PDYP4", 0};  // Peak Fy reduction due to square root of spin parameter
  std::pair<const std::string, double> PHYP1 = {"PHYP1", 1};  // Fy - alpha curve lateral shift limitation
  std::pair<const std::string, double> PHYP2 = {"PHYP2", 0.15};  // Fy - alpha curve maximum lateral shift parameter
  std::pair<const std::string, double> PHYP3 = {
      "PHYP3", 0};  // Fy - alpha curve maximum lateral shift varying with load parameter
  std::pair<const std::string, double> PHYP4 = {"PHYP4", -4};  // Fy - alpha curve maximum lateral shift parameter
  std::pair<const std::string, double> PECP1 = {
      "PECP1", 0.5};  // Camber w.r.t.spin reduction factor parameter in camber stiffness
  std::pair<const std::string, double> PECP2 = {
      "PECP2", 0};  // Camber w.r.t.spin reduction factor varying with load parameter in camber stiffness
  std::pair<const std::string, double> QDTP1 = {"QDTP1",
                                                10};  // Pneumatic trail reduction factor due to turn slip parameter
  std::pair<const std::string, double> QCRP1 = {
      "QCRP1", 0.2};  // Turning moment at constant turning and zero forward speed parameter
  std::pair<const std::string, double> QCRP2 = {
      "QCRP2", 0.1};  // Turn slip moment(at alpha = 90deg) parameter for increase with spin
  std::pair<const std::string, double> QBRP1 = {
      "QBRP1", 0.1};  // Residual(spin) torque reduction factor parameter due to side slip
  std::pair<const std::string, double> QDRP1 = {"QDRP1", 1};  // Turn slip moment peak magnitude parameter

  bool parseFrom(const TNODb &models);
};

struct TNOTire {
  TNOTire_Model MODEL;
  TNOTire_Dimenson DIMENSION;
  TNOTire_OperatingCondition OPERATING_CONDITIONS;
  TNOTire_Inertia INERTIA;
  TNOTire_Vertical VERTICAL;
  TNOTire_Structural STRUCTURAL;
  TNOTire_ContactPatch CONTACT_PATCH;
  TNOTire_InflationPressureRng INFLATION_PRESSURE_RANGE;
  TNOTire_VerticalForceRng VERTICAL_FORCE_RANGE;
  TNOTire_LongSlipRng LONG_SLIP_RANGE;
  TNOTire_SlipAngRng SLIP_ANGLE_RANGE;
  TNOTire_InclinationAngRng INCLINATION_ANGLE_RANGE;
  TNOTire_ScalingCoef SCALING_COEFFICIENTS;
  TNOTire_LongCoef LONGITUDINAL_COEFFICIENTS;
  TNOTire_OverTurnCoef OVERTURNING_COEFFICIENTS;
  TNOTire_LatCoef LATERAL_COEFFICIENTS;
  TNOTire_RollingCoef ROLLING_COEFFICIENTS;
  TNOTire_AlignCoef ALIGNING_COEFFICIENTS;
  TNOTire_TurnSlipCoef TURNSLIP_COEFFICIENTS;
};

class MODULE_API TNOTireParser final {
 public:
  TNOTireParser();
  ~TNOTireParser();

  /* set TNO tir path */
  void setTirPath(const std::string &tir_path);

  /* get TNO tir path */
  const std::string &getTirPath();

  /* parse TNO tir */
  bool parseTNOTir();

  /* get TNO tire parameter */
  const TNOTire &getTNOTir();

 protected:
  /* pre-process of one line */
  void preProcessLine(std::string &line);

 private:
  std::string m_tir_path;
  TNOTire m_tir;
  const std::string comment_charcter = "$!";
  const char model_character = '[';
  const std::string m_stop_characters = " \'";
  TNODb m_TNODb;
};
}  // namespace tx_car
