// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <cmath>
#include "par_converter_helper.h"

#define MM_2_M (0.001)                // millimeter to meter
#define MPa_2_Bar (10)                // MPa to Bar
#define Bar_2_MPa (0.1)               // MPa to Bar
#define N_2_KN (1000)                 // N to KN
#define Nm_2_KNm (1000)               // N*m to KN*m
#define RPM_2_RADS (2 * M_PI / 60.0)  // rpm to rad/s
#define MS_2_KPH (3.60)               // m/s to km/h

#ifndef DEG_2_RAD
#  define DEG_2_RAD (M_PI / 180.0)  // deg to rad
#endif                              // !DEG_2_RAD

#ifndef RAD_2_DEG
#  define RAD_2_DEG (180.0 / M_PI)  // rad to deg
#endif                              // !RAD_2_DEG

#ifndef M_2_MM
#  define M_2_MM (1000.0)  // rad to deg
#endif                     // !RAD_2_DEG

#define CONVERSION_LOG_FAIL(info) LOG_ERROR << info << ", quit conversion, no output file will be generated.\n"
#define CONVERSION_LOG_SKIPPED(info) LOG_WARNING << info << ", data will be dumped into output file.\n"

namespace tx_car {
class ParConverter {
 protected:
  virtual bool convertSprungMass(const tx_car::CarSimParData& carsimParData, tx_car::SprungMassDD& sprungMass) = 0;
  virtual bool convertBody(const tx_car::CarSimParData& carsimParData, tx_car::VehBodyDD& body) = 0;
  virtual bool convertEngine(const tx_car::CarSimParData& carsimParData, tx_car::Engine& engine) = 0;
  virtual bool convertDriveLine(const tx_car::CarSimParData& carsimParData, tx_car::DriveLine& driveline) = 0;
  virtual bool convertSteerMapped(const tx_car::CarSimParData& carsimParData, tx_car::SteerMapped& steerMapped) = 0;
  virtual bool convertDynamicSteer(const tx_car::CarSimParData& carsimParData, tx_car::DynamicSteer& dynamicSteer) = 0;
  virtual bool convertSuspension(const tx_car::CarSimParData& carsimParData, tx_car::SuspensionDD& susp) = 0;
  virtual bool convertWheelTire(const tx_car::CarSimParData& carsimParData, tx_car::WheelTireDD& wheelTire) = 0;
  virtual bool convertCarType(const tx_car::CarSimParData& carsimParData, tx_car::CarType& carType,
                              tx_car::Pow_ECU& ecu) = 0;

 public:
  // template used to convert carsim data to tx_car.json
  bool convertToTxCarJson(const tx_car::CarSimParData& carsimParData, tx_car::car& txcar);

  // find Data by keyword
  bool findParDataByKeyword(const tx_car::CarSimParData& parDataSet, const std::string& keyword,
                            const tx_car::CarSimParData_DataType& dataType, tx_car::CarSimParData_ParData& parData,
                            const tx_car::CarSimParData_Axle& axle = tx_car::CarSimParData_Axle_No_Axle,
                            const tx_car::CarSimParData_Side& side = tx_car::CarSimParData_Side_No_Side);
  bool findAllParDataByKeyword(const tx_car::CarSimParData& parDataSet, const std::string& keyword,
                               const tx_car::CarSimParData_DataType& dataType,
                               std::vector<tx_car::CarSimParData_ParData>& parDatas,
                               const tx_car::CarSimParData_Axle& axle = tx_car::CarSimParData_Axle_No_Axle,
                               const tx_car::CarSimParData_Side& side = tx_car::CarSimParData_Side_No_Side);
  void copyMap1DData(tx_car::Real1DMap& out, const tx_car::Real1DMap& in,
                     tx_car::DataSource dataSource = tx_car::DataSource::UserInput, double x0Scale = 1.0,
                     double y0Scale = 1.0);
  void copyMap2DData(tx_car::Real2DMap& out, const tx_car::Real2DMap& in,
                     tx_car::DataSource dataSource = tx_car::DataSource::UserInput);

 public:
  // file io
  void parsedParToProto(const ParsedData& parData, tx_car::CarSimParData& carsimParData);
  void parsedJsonToFile(tx_car::CarSimParData& carsimParData, const std::string& parsedJsonPath);
  void parsedJsonToFile(tx_car::car& txcar, const std::string& parsedJsonPath);

 protected:
  tx_car::car mCar;
};
}  // namespace tx_car
