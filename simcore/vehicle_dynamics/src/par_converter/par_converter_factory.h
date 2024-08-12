// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "par_converter_core.h"

namespace tx_car {
class ParConverterFactory {
 public:
  static ParConverterPtr makeConverter(const std::string& carsimVersion);
};
}  // namespace tx_car
