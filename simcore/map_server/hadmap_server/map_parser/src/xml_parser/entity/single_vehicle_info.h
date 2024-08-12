/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <list>
#include <string>
#include "acceleration.h"
#include "common/xml_parser/entity/merge.h"
#include "common/xml_parser/entity/vehicle.h"
#include "common/xml_parser/entity/velocity.h"

class CSingleVehicleInfo {
 public:
 public:
  CVehicle m_vehicle;
  CAcceleration m_accs;
  CMerge m_merge;
  CVelocity m_velocity;
};
