/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
enum HadmapElementType {
  HET_Invalid = -1,
  HET_Straight,
  HET_Cross,
  HET_Curve,
};

class CMapElement {
 public:
  CMapElement();

  int m_type;
  std::string m_name;
  int m_id;
  std::string m_description;
  float m_width;
  float m_length;
  float m_height;
  float m_maxSpeed;
  double m_position[3];
  double m_rotation[3];
  double m_matrix[16];
  std::string m_geometry;
};
