/**
 * @file ObuSensor.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "obu_sensor.h"
#include <float.h>
#include <algorithm>
#include <iostream>

/**
 * @brief Construct a new Obu Sensor:: Obu Sensor object
 *
 */
ObuSensor::ObuSensor() { T = Eigen::Vector3d::Zero(); }

/**
 * @brief Destroy the Obu Sensor:: Obu Sensor object
 *
 */
ObuSensor::~ObuSensor() {}

/**
 * @brief set position
 *
 * @param p
 */
void ObuSensor::setPosition(const Eigen::Vector3d &p) { T = p; }

/**
 * @brief init
 *
 * @return true
 * @return false
 */
bool ObuSensor::Init() { return true; }
