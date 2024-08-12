// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>

#include "utils/eval_types.h"

namespace eval {
// topic
namespace topic {
const char LOCATION[] = "LOCATION";
const char LOCATION_TRAILER[] = "LOCATION_TRAILER";
const char LOCATION_UNION[] = "EgoUnion/LOCATION";
const char TRAFFIC[] = "TRAFFIC";
const char VEHICLE_STATE[] = "VEHICLE_STATE";
const char CONTROL[] = "CONTROL";
const char CONTROL_V2[] = "CONTROL_V2";
const char GRADING[] = "GRADING";
const char GRADING_STATISTICS[] = "GRADING_STATISTICS";
const char PREDICTIONS[] = "PREDICTION";
const char TRAJECTORY[] = "TRAJECTORY";
const char ENVIRONMENTAL[] = "ENVIRONMENTAL";
const char MODULE_CONFIG[] = ".play_config";
const char PARKING_STATE[] = "PARKING_STATE";
const char PARKING_SPACE[] = "TXSIM_PARKING_SPACE";
const char L4_ALGORITHM_SIGNAL[] = "L4_ALGORITHM_SIGNAL";
}  // namespace topic

// max point size in Shape_Line
const uint32_t const_max_point_size = 20;
const uint32_t const_max_actor_size = 500;
const uint32_t const_corner_size = 4;
const double const_max_dist = 200.0;  // unit m
const double const_max_thw = 10.0;    // unit s
const double const_max_ttc = 10.0;
const size_t const_MSPS = 300;  // map_sample_points_size, about 30 m
const char const_Ego_Front[] = "Ego.Front";
const char const_Ego_Trailer[] = "Ego.Trailer";
const double const_Zero_Threshold = 1e-12;
const double const_deg_2_rad = 1.0 / 180.0 * 3.1415926;
const double const_limit_eps = 0.0001;  // unit m/s

// ego sizes
namespace ego_size {
const RectSize ego(4.68, 2.18, 1.76);
const RectSize trailer(12.31528, 2.3, 3.0);
}  // namespace ego_size

// simcity KPI
namespace SimCity {
const std::vector<std::string> constSimCityKPIs{
    "Collision",       "MaxAcceleration_V", "MaxSpeed_V",       "MinAveragerTimeHeadway",
    "RunTrafficLight", "CrossSolidLine",    "MaxDeceleration_V"};

}
}  // namespace eval
