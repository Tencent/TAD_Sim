// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_DRIVELINE_DRIVELINEEXT_H_
#define VEHICLEDYNAMICS_DRIVELINE_DRIVELINEEXT_H_
#include "TX_DriveLine.h"
#include "inc/car_common.h"

namespace tx_car {
namespace driveline {
class MODULE_API DriveLineExt : public ::TX_DriveLine {
 public:
  DriveLineExt(/* args */);
  virtual ~DriveLineExt();

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
}  // namespace driveline
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_DRIVELINE_DRIVELINEEXT_H_
