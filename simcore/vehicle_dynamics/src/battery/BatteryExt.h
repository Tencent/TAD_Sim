// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_BATTERY_BATTERYEXT_H_
#define VEHICLEDYNAMICS_BATTERY_BATTERYEXT_H_
#include "Battery.h"
#include "inc/car_common.h"

namespace tx_car {
namespace power {
class MODULE_API BatteryExt : public Battery {
 public:
  BatteryExt(/* args */);
  ~BatteryExt();
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

  void setBattOcvSocNum(int num);  // set the battery ocv table SOC point number

  void setBattRisisTableTempNum(int num);  // set the battery resis table temp point number

 private:
  bool parsingParameterFromJson(char* errorLog = nullptr);
  // parsing parameter from json file
};

}  // namespace power
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_BATTERY_BATTERYEXT_H_
