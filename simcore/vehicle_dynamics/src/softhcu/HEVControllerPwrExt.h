// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_SOFTHCU_HEVCONTROLLERPWREXT_H_
#define VEHICLEDYNAMICS_SOFTHCU_HEVCONTROLLERPWREXT_H_

#include "HEVControllerPwr.h"
#include "inc/car_common.h"

namespace tx_car {
namespace hcu {
class MODULE_API HEVControllerPwrExt : public HEVControllerPwr {
 private:
  /* data */
 public:
  HEVControllerPwrExt(/* args */);
  ~HEVControllerPwrExt();

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  void terminate();

 protected:
  /*!
   * @brief load parameter from txcar.json and init HEVControllerPwr model
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

}  // namespace hcu
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_SOFTHCU_HEVCONTROLLERPWREXT_H_
