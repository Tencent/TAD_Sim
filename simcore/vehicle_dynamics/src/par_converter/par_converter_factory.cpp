// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "par_converter_factory.h"
#include "par_converter.h"
#include "par_converter_2016_1.h"
#include "par_converter_2019_1.h"

namespace tx_car {
ParConverterPtr ParConverterFactory::makeConverter(const std::string& carsimVersion) {
  ParConverterPtr converterPtr;

  if (carsimVersion == Constants::version::carsim_2016_1) {
    converterPtr = std::make_shared<ParConverter_2016_1>();
  } else if (carsimVersion == Constants::version::carsim_2019_1) {
    converterPtr = std::make_shared<ParConverter_2019_1>();
  }

  return converterPtr;
}
}  // namespace tx_car
