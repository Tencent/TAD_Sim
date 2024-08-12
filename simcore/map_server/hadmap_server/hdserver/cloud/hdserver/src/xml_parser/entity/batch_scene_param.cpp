/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "batch_scene_param.h"
#include "common/engine/constant.h"

void sTagBatchSceneParam::Reset() {
  m_export_flag = NOT_EXPORT_TO_TADSIM;
  m_type = SCENE_BATCH_GEN_TYPE_SINGLE;
  m_strPrefix = "";
  m_vehicle_id = 0;
  m_vehicle_velocity_start = 0;
  m_vehicle_velocity_end = 0;
  m_vehicle_velocity_sep = 0;
  m_vehicle_pos_start = 0;
  m_vehicle_pos_end = 0;
  m_vehicle_pos_sep = 0;
  m_vehicle_offset_start = 0;
  m_vehicle_offset_end = 0;
  m_vehicle_offset_sep = 0;
  m_ego_velocity_start = 0;
  m_ego_velocity_end = 0;
  m_ego_velocity_sep = 0;

  m_strInfo = "";
}
