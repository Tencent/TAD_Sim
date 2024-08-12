// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "defines.h"

#include <cstdlib>
#include <limits>

namespace trafficga {
DEFINE_string(flagsfile, "", "flags file, !empty, args from the file");

DEFINE_int64(stage, 1, "0:process json data  1:train ga");
DEFINE_string(scene, "scenefile.sim", "tadsim scene file");
DEFINE_string(data, "data", "data dir(in and out)");
DEFINE_int64(start_idx, 0, "process json data begin idx(idx - 1), example: data_1.json");
DEFINE_int64(end_idx, 1, "process json data end idx(idx - 1), example: data_2.json");
DEFINE_bool(islonlat, true, "data is lon lat");
DEFINE_int32(interval_num, 10, "interval num");
DEFINE_int64(DNA, 10, "DNA size");
DEFINE_double(CROSS_RATE, 0.8, "cross rate");
DEFINE_double(MUTATION_RATE, 0.003, "mutation rate");
DEFINE_int32(N_GENERATIONS, 200, "N_GENERATIONS size");
DEFINE_double(e, 0.003, "coverge condition");
DEFINE_double(OBS_DISTANCE, 1, "obs distance");
DEFINE_int32(MAX_STEP, 20, "max simulate step");
DEFINE_double(TIME_STEP, 20.0, "each step time");
};  // namespace trafficga
