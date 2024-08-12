// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_SOFTECU_SOFTECUEXT_H_
#define VEHICLEDYNAMICS_SOFTECU_SOFTECUEXT_H_

#include "SoftECU.h"
#include "inc/car_common.h"

namespace tx_car {
namespace ecu {
class MODULE_API SoftECUExt : public SoftECU {
 private:
  /* data */
 public:
  SoftECUExt(/* args */);
  ~SoftECUExt();

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

}  // namespace ecu
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_SOFTECU_SOFTECUEXT_H_
