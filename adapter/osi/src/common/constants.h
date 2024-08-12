// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>

namespace adapter {
// topic
namespace topic {
const char LOCATION[] = "LOCATION";
const char LOCATION_TRAILER[] = "LOCATION_TRAILER";
const char TRAFFIC[] = "TRAFFIC";
const char VEHICLE_STATE[] = "VEHICLE_STATE";
const char CONTROL[] = "CONTROL";
const char GRADING[] = "GRADING";
const char GRADING_STATISTICS[] = "GRADING_STATISTICS";
const char PREDICTIONS[] = "PREDICTION";
const char TRAJECTORY[] = "TRAJECTORY";
const char ENVIRONMENTAL[] = "ENVIRONMENTAL";
const char PARKING_STATE[] = "PARKING_STATE";
const char PARKING_SPACE[] = "TXSIM_PARKING_SPACE";
const char DETECTED_LINES[] = "TXSIM_DETECTED_LINES";
// osi
const char OSI_SENSORDATA[] = "OSI_SENSORDATA";
const char OSI_ENVIRONMENT[] = "OSI_ENVIRONMENT";
const char OSI_GROUNDTRUTH[] = "OSI_GROUNDTRUTH";
const char OSI_HOSTVEHICLEDATA[] = "OSI_HOSTVEHICLEDATA";
}  // namespace topic
}  // namespace adapter
