// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "vehicle_element_inject_mode.h"
#include <iostream>
#include "mapengine/hadmap_engine.h"

double InjectImpl::DrivingFollow(hadmap::txMapHandle* hdmap_handler, const ParamKVMap& in_param_kvmap) noexcept {
  return 1.0;
}

double InjectImpl::DrivingChangeLane(hadmap::txMapHandle* hdmap_handler, const ParamKVMap& in_param_kvmap) noexcept {
  return 0.0;
}
