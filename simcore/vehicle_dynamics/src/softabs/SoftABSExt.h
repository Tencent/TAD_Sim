// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_SOFTABS_SOFTABSEXT_H_
#define VEHICLEDYNAMICS_SOFTABS_SOFTABSEXT_H_

#include "SoftABS.h"
#include "inc/car_common.h"

namespace tx_car {
namespace abs {
class MODULE_API SoftABSExt : public SoftABS {
 private:
  /* data */
 public:
  SoftABSExt(/* args */);
  ~SoftABSExt();

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  void terminate();

 protected:
  /*!
   * @brief load parameter from txcar.json and init SoftABS model
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

}  // namespace abs
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_SOFTABS_SOFTABSEXT_H_
