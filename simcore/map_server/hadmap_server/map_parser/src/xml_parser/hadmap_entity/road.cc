/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "road.h"

CRoad::CRoad() {
  m_widthOfLane = 3.5f;
  m_numberOfLanes = 4;
  m_roadThickness = 0.1f;
  m_roadFriction = 0.8f;
  m_roadLaneMark = 0;
}