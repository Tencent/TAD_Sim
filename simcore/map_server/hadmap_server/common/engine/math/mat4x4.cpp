/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/engine/math/mat4x4.h"

CMat4x4::CMat4x4() { Reset(); }

CMat4x4::CMat4x4(const CMat4x4& mat4) { (*this) = mat4; }

CMat4x4::CMat4x4(const glm::mat4& mat4) { (*this) = mat4; }

void CMat4x4::Reset() {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      m_data[i][j] = 0;
    }
  }
}

void CMat4x4::Identity() {
  Reset();
  for (int i = 0; i < 4; ++i) {
    m_data[i][i] = 1.0;
  }
}

CMat4x4& CMat4x4::operator=(const CMat4x4& other) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      m_data[i][j] = other.m_data[i][j];
    }
  }

  return (*this);
}

glm::mat4 CMat4x4::glmMat4() const {
  glm::mat4 m;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      m[i][j] = m_data[i][j];
    }
  }

  return m;
}
CMat4x4& CMat4x4::operator=(const glm::mat4& glmMat4) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      m_data[i][j] = glmMat4[i][j];
    }
  }

  return (*this);
}

CMat4x4 operator*(const CMat4x4& left, const CMat4x4& right) {
  glm::mat4 matResult = (left.glmMat4()) * (right.glmMat4());
  return CMat4x4(matResult);
}
