// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_MOTOR_FRONTEMOTOREXT_H_
#define VEHICLEDYNAMICS_MOTOR_FRONTEMOTOREXT_H_
#include "EMotor.h"
#include "inc/car_common.h"

namespace tx_car {
namespace power {
class MODULE_API FrontEMotorExt : public EMotor {
 public:
  FrontEMotorExt(/* args */);
  ~FrontEMotorExt();

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

  void setMotTrqMapNum(int num);

  void setMotLossMapSpdNum(int num);
};
}  // namespace power
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_MOTOR_FRONTEMOTOREXT_H_
