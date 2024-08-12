/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <map>
#include <string>
#include "acceleration_param.h"
#include "common/xml_parser/entity/merge_param.h"
#include "common/xml_parser/entity/velocity_param.h"
struct sTagEgoParam {
  sTagEgoParam() { Reset(); }

  void Reset();

  double m_ego_velocity_start;
  double m_ego_velocity_end;
  double m_ego_velocity_sep;
};

struct sTagBatchSceneParam {
  sTagBatchSceneParam() { Reset(); }

  void Reset();

  int m_export_flag;  // 0:不导入,1:导入
  int m_type;
  std::string m_strPrefix;
  int m_vehicle_id;
  double m_vehicle_velocity_start;
  double m_vehicle_velocity_end;
  double m_vehicle_velocity_sep;
  double m_vehicle_pos_start;
  double m_vehicle_pos_end;
  double m_vehicle_pos_sep;
  double m_vehicle_offset_start;
  double m_vehicle_offset_end;
  double m_vehicle_offset_sep;
  double m_ego_velocity_start;
  double m_ego_velocity_end;
  double m_ego_velocity_sep;

  CAccelerationParam m_acceleration_param;
  CMergeParam m_merge_param;
  CVelocityParam m_velocity_param;

  std::map<std::string, sTagEgoParam> m_ego = {};

  std::string m_strInfo;
};

struct sTagVehicleParam {
  sTagVehicleParam() { Reset(); }

  void Reset();

  int m_vehicle_id;
  double m_vehicle_velocity_start;
  double m_vehicle_velocity_end;
  double m_vehicle_velocity_sep;
  double m_vehicle_pos_start;
  double m_vehicle_pos_end;
  double m_vehicle_pos_sep;
  double m_vehicle_offset_start;
  double m_vehicle_offset_end;
  double m_vehicle_offset_sep;

  CAccelerationParam m_acceleration_param;
  CMergeParam m_merge_param;
  CVelocityParam m_velocity_param;
};

struct sTagBatchSceneParamV2 {
  sTagBatchSceneParamV2() { Reset(); }

  void Reset();

  int m_export_flag;  // 0:不导入,1:导入
  int m_type;
  int m_file_type;
  std::string m_strPrefix;

  // ego
  std::map<std::string, sTagEgoParam> m_ego;

  // vehicles
  std::vector<sTagVehicleParam> m_vehicles;

  std::string m_strInfo;
};
