// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "par_converter.h"
#include "par_converter_helper.h"

#include <array>

namespace tx_car {
/* carsim 2016.1 converter */
class ParConverter_2019_1 : public ParConverter {
 public:
  ParConverter_2019_1();

 protected:
  virtual bool convertSprungMass(const tx_car::CarSimParData& carsimParData, tx_car::SprungMassDD& sprungMass) override;
  virtual bool convertBody(const tx_car::CarSimParData& carsimParData, tx_car::VehBodyDD& body) override;
  virtual bool convertEngine(const tx_car::CarSimParData& carsimParData, tx_car::Engine& engine) override;
  virtual bool convertDriveLine(const tx_car::CarSimParData& carsimParData, tx_car::DriveLine& driveline) override;
  virtual bool convertSteerMapped(const tx_car::CarSimParData& carsimParData,
                                  tx_car::SteerMapped& steerMapped) override;
  virtual bool convertDynamicSteer(const tx_car::CarSimParData& carsimParData,
                                   tx_car::DynamicSteer& dynamicSteer) override;
  virtual bool convertSuspension(const tx_car::CarSimParData& carsimParData, tx_car::SuspensionDD& susp) override;
  virtual bool convertWheelTire(const tx_car::CarSimParData& carsimParData, tx_car::WheelTireDD& wheelTire) override;
  virtual bool convertCarType(const tx_car::CarSimParData& carsimParData, tx_car::CarType& carType,
                              tx_car::Pow_ECU& ecu);
};
}  // namespace tx_car
