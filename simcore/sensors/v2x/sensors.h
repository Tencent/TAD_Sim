/**
 * @file sensors.h
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once
#include <stdint.h>
#include <map>
#include <memory>
#include <string>
#include "scene.pb.h"
#include "sensor/obu_sensor.h"
#include "sensor/rsu_sensor.h"
#include "sensor_objects_def.h"
#include "txsim_messenger.h"

extern std::int64_t ego_id;
extern std::map<std::int64_t, std::shared_ptr<RsuSensor>> rsu_sensors;
extern std::map<std::int64_t, std::shared_ptr<ObuSensor>> traffic_obu_sensors;
extern std::map<std::int64_t, std::shared_ptr<ObuSensor>> ego_obu_sensors;  // only one for now
extern std::vector<ObuMountConfig> obu_mount_config_list;

bool LoadSensor(const sim_msg::Scene&, const tx_sim::Vector3d&, const std::string& groupname);
