/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "elements.h"

class CRoad : public CMapElement {
 public:
  CRoad();

  float m_widthOfLane;
  int m_numberOfLanes;
  float m_roadThickness;
  float m_roadFriction;
  int m_roadLaneMark;
};
