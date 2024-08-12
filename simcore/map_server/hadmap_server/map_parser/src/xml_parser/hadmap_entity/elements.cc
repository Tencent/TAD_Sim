/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "elements.h"

CMapElement::CMapElement() {
  m_type = HET_Invalid;
  m_name = "";
  m_id = 0;
  m_description = "";
  m_width = 0;
  m_height = 0;
  m_length = 0;
  m_maxSpeed = 0;
  m_position[0] = 0.0;
  m_position[1] = 0.0;
  m_position[2] = 0.0;
  m_rotation[0] = 0.0;
  m_rotation[1] = 0.0;
  m_rotation[2] = 0.0;
  for (size_t i = 0; i < 15; i++) {
    /* code */
    m_matrix[i] = 0;
  }

  m_geometry = "";
}