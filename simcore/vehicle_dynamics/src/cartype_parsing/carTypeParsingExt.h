// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_CARTYPE_PARSING_CARTYPEPARSINGEXT_H_
#define VEHICLEDYNAMICS_CARTYPE_PARSING_CARTYPEPARSINGEXT_H_

#include "carTypeParsing.h"
#include "inc/car_common.h"

namespace tx_car {
namespace cartype {

class MODULE_API CarTypeParsingExt : public carTypeParsing {
 private:
  /* data */
 public:
  CarTypeParsingExt(/* args */);
  ~CarTypeParsingExt();

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  void terminate();

 protected:
  bool initMDL();

  bool parsingParameterFromJson(char* errorLog = nullptr);
  // parsing parameter from json file

  /* load parameter from json file */
  bool loadParam(const std::string& par_path);

  /* init model by loaded json file */
  bool initParam();
};

}  // namespace cartype
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_CARTYPE_PARSING_CARTYPEPARSINGEXT_H_
