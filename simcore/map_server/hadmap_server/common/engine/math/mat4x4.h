/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <glm/glm.hpp>
class CMat4x4 {
 public:
  CMat4x4();
  CMat4x4(const CMat4x4& mat4);
  explicit CMat4x4(const glm::mat4& mat4);
  void Reset();
  void Identity();
  CMat4x4& operator=(const CMat4x4& other);

  glm::mat4 glmMat4() const;
  CMat4x4& operator=(const glm::mat4& glmMat4);
  friend CMat4x4 operator*(const CMat4x4& left, const CMat4x4& right);

 protected:
  float m_data[4][4];
};

CMat4x4 operator*(const CMat4x4& left, const CMat4x4& right);
