/**
 * @file UserTopic.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <string>

namespace tx_sim {

namespace user_topic {

const char kOsiSensorDatas[] = "TXSIM_SENSOR_DATA";
const char kFreeSpace[] = "TXSIM__FREESPACE";
const char kCameraObject[] = "TXSIM_CAMERA_OBJECT";
const char kLidarObject[] = "TXSIM_LIDAR_OBJECT";
const char kUltrasonicObject[] = "TXSIM_ULTRASONIC_OBJECT";
const char kLaneTruth[] = "TXSIM_DETECTED_LINES";
const char kParkingSpace[] = "TXSIM_PARKING_SPACE";
const char kFreeSpaceGrid[] = "TXSIM_FREESPACE_GRID";
const char kTrafficTilter[] = "TXSIM_TRAFFIC_FILTER";
const char kSensorObject[] = "TXSIM_SENSOR_OBJECT";

}  // namespace user_topic

}  // namespace tx_sim
