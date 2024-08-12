// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_DRIVELINE_EV_FRONT_TX_DRIVELINE_EV_FRONT_EXT_H_
#define VEHICLEDYNAMICS_DRIVELINE_EV_FRONT_TX_DRIVELINE_EV_FRONT_EXT_H_
#include "TX_DriveLine_EV_Front.h"
#include "inc/car_common.h"

namespace tx_car {
namespace driveline_ev_front {
class MODULE_API TX_DriveLine_EV_Front_Ext : public ::TX_DriveLine_EV_Front {
 public:
  TX_DriveLine_EV_Front_Ext(/* args */);
  virtual ~TX_DriveLine_EV_Front_Ext();

  bool parsingParameterFromJson(char* errorLog = nullptr);
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
  bool loadParam(const std::string& par_path);

  /* init model by loaded json file */
  bool initParam();
};
}  // namespace driveline_ev_front
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_DRIVELINE_EV_FRONT_TX_DRIVELINE_EV_FRONT_EXT_H_
