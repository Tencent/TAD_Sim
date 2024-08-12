// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <gflags/gflags.h>
namespace trafficga {
DECLARE_string(flagsfile);
DECLARE_int64(stage);
DECLARE_string(scene);
DECLARE_string(data);
DECLARE_int64(start_idx);
DECLARE_int64(end_idx);
DECLARE_bool(islonlat);
DECLARE_int32(interval_num);
DECLARE_int64(DNA);
DECLARE_double(CROSS_RATE);
DECLARE_double(MUTATION_RATE);
DECLARE_int32(N_GENERATIONS);
DECLARE_double(e);
DECLARE_double(OBS_DISTANCE);
DECLARE_int32(MAX_STEP);
DECLARE_double(TIME_STEP);
};  // namespace trafficga
