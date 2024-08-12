// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_TRANSMISSION_TX_TRANSMISSIONEXT_H_
#define VEHICLEDYNAMICS_TRANSMISSION_TX_TRANSMISSIONEXT_H_
#include "TX_Transmission.h"
#include "inc/car_common.h"

namespace tx_car {
namespace transmission {
// max gear number, including 1 reverse gear and 1 neutral gear. @dhu
const int constMaxGearNum = 10;

class MODULE_API TX_TransmissionExt : public ::TX_Transmission {
 public:
  TX_TransmissionExt(/* args */);
  virtual ~TX_TransmissionExt();

  bool parsingParameterFromJson(char *errorLog = nullptr);
  // parsing parameter from json file

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  void terminate();

 protected:
  bool initMDL();

  /* load parameter from json file */
  bool loadParam(const std::string &par_path);

  /* init model by loaded json file */
  bool initParam();
};
}  // namespace transmission
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_TRANSMISSION_TX_TRANSMISSIONEXT_H_
