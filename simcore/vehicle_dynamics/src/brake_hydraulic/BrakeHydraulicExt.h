// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_BRAKE_HYDRAULIC_BRAKEHYDRAULICEXT_H_
#define VEHICLEDYNAMICS_BRAKE_HYDRAULIC_BRAKEHYDRAULICEXT_H_

#include "BrakeHydraulic.h"
#include "inc/car_common.h"

namespace tx_car {
namespace brake_hydraulic {
class MODULE_API BrakeHydraulicExt : public BrakeHydraulic {
 private:
  /* data */
 public:
  BrakeHydraulicExt(/* args */);
  ~BrakeHydraulicExt();

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  void terminate();

 protected:
  /*!
   * @brief load parameter from txcar.json and init SoftECU model
   * @return true/false
   */
  bool initMDL();

  bool parsingParameterFromJson(char *errorLog = nullptr);
  // parsing parameter from json file

  /* load parameter from json file */
  bool loadParam(const std::string &par_path);

  /* init model by loaded json file */
  bool initParam();
};

}  // namespace brake_hydraulic
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_BRAKE_HYDRAULIC_BRAKEHYDRAULICEXT_H_
