// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_HYBRID_PARSER_HYBRIDPARSEREXT_H_
#define VEHICLEDYNAMICS_HYBRID_PARSER_HYBRIDPARSEREXT_H_

#include "HybridParser.h"
#include "inc/car_common.h"

namespace tx_car {
namespace hybrid_parser {
class MODULE_API HybridParserExt : public HybridParser {
 private:
  /* data */
 public:
  HybridParserExt(/* args */);
  ~HybridParserExt();

  /* Initial conditions function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  void terminate();

 protected:
  bool initMDL();

  bool parsingParameterFromJson(char *errorLog = nullptr);
  // parsing parameter from json file

  /* load parameter from json file */
  bool loadParam(const std::string &par_path);

  /* init model by loaded json file */
  bool initParam();
};

}  // namespace hybrid_parser
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_HYBRID_PARSER_HYBRIDPARSEREXT_H_
