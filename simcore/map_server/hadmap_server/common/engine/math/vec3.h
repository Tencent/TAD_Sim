/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <hadmap.h>

#include <sstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <glm/glm.hpp>
#include "common/engine/common.h"

#ifndef INFINITY
#  define INFINITY (1e+9f)
#else
// using the default INFINITY definition linux(__builtin_inff in inf.h)
#endif

template <typename T>
class vector3 {
 public:
  vector3();
  vector3(T x, T y, T z);
  explicit vector3(hadmap::txPoint& txPt);
  explicit vector3(const std::string& xyz, const std::string& delimiter = ",");

  void Set(T x, T y, T z);

  void Reset();

  glm::vec3 glmVec3();

  vector3 operator*(const T fX) const;
  vector3 operator+(const vector3& vec3) const;
  vector3& operator=(const vector3& vec3);
  vector3& operator=(const glm::vec3& vec3);
  vector3& operator=(const hadmap::txPoint& txPt);
  vector3& operator+=(const vector3& vec3);
  vector3& operator-=(const vector3& vec3);
  vector3 operator-(const vector3& vec3);
  vector3 cross(const vector3& vec3);
  T dot(const vector3& vec3);
  void norm();
  double len();
  double distance(const vector3& other);

  T X() const { return m_fX; }
  T Y() const { return m_fY; }
  T Z() const { return m_fZ; }

  T& X() { return m_fX; }
  T& Y() { return m_fY; }
  T& Z() { return m_fZ; }

  std::string ToString(bool ignore_z = false) const {
    std::string ret = to_string_with_precision(m_fX, 12);
    ret += ",";
    ret += to_string_with_precision(m_fY, 12);
    if (!ignore_z) {
      ret += ",";
      ret += to_string_with_precision(m_fZ, 12);
    }
    return std::move(ret);
  }

 protected:
  T m_fX;
  T m_fY;
  T m_fZ;
};

template <typename T>
vector3<T>::vector3() {
  Reset();
}

template <typename T>
vector3<T>::vector3(T x, T y, T z) : m_fX(x), m_fY(y), m_fZ(z) {}

template <typename T>
vector3<T>::vector3(hadmap::txPoint& txPt) : m_fX(txPt.x), m_fY(txPt.y), m_fZ(txPt.z) {}

template <typename T>
vector3<T>::vector3(const std::string& xyz, const std::string& delimiter) {
  Reset();
  std::vector<std::string> slices;
  boost::algorithm::split(slices, xyz, boost::algorithm::is_any_of(delimiter));
  if (slices.size() > 0) {
    std::istringstream iss(slices[0]);
    iss >> m_fX;
  }
  if (slices.size() > 1) {
    std::istringstream iss(slices[1]);
    iss >> m_fY;
  }
  if (slices.size() > 2) {
    std::istringstream iss(slices[2]);
    iss >> m_fZ;
  }
}

template <typename T>
void vector3<T>::Set(T x, T y, T z) {
  m_fX = x;
  m_fY = y;
  m_fZ = z;
}

template <typename T>
void vector3<T>::Reset() {
  m_fX = 0.0f;
  m_fY = 0.0f;
  m_fZ = 0.0f;
}

template <typename T>
glm::vec3 vector3<T>::glmVec3() {
  return glm::vec3(m_fX, m_fY, m_fZ);
}

template <typename T>
vector3<T> vector3<T>::operator*(const T fScale) const {
  T fX = m_fX * fScale;
  T fY = m_fY * fScale;
  T fZ = m_fZ * fScale;

  return vector3<T>(fX, fY, fZ);
}

template <typename T>
vector3<T> vector3<T>::operator+(const vector3<T>& vec3) const {
  T fX = m_fX + vec3.m_fX;
  T fY = m_fY + vec3.m_fY;
  T fZ = m_fZ + vec3.m_fZ;

  return vector3<T>(fX, fY, fZ);
}

template <typename T>
vector3<T>& vector3<T>::operator=(const vector3<T>& vec3) {
  m_fX = vec3.m_fX;
  m_fY = vec3.m_fY;
  m_fZ = vec3.m_fZ;

  return (*this);
}
template <typename T>
vector3<T>& vector3<T>::operator=(const glm::vec3& vec3) {
  m_fX = vec3.x;
  m_fY = vec3.y;
  m_fZ = vec3.z;

  return (*this);
}
template <typename T>
vector3<T>& vector3<T>::operator=(const hadmap::txPoint& txPt) {
  m_fX = txPt.x;
  m_fY = txPt.y;
  m_fZ = txPt.z;
  return (*this);
}
template <typename T>
vector3<T>& vector3<T>::operator+=(const vector3<T>& vec3) {
  m_fX += vec3.m_fX;
  m_fY += vec3.m_fY;
  m_fZ += vec3.m_fZ;

  return (*this);
}
template <typename T>
vector3<T>& vector3<T>::operator-=(const vector3<T>& vec3) {
  m_fX -= vec3.m_fX;
  m_fY -= vec3.m_fY;
  m_fZ -= vec3.m_fZ;

  return (*this);
}

template <typename T>
vector3<T> vector3<T>::operator-(const vector3<T>& vec3) {
  vector3<T> t;
  t.m_fX = m_fX - vec3.m_fX;
  t.m_fY = m_fY - vec3.m_fY;
  t.m_fZ = m_fZ - vec3.m_fZ;
  return t;
}

template <typename T>
vector3<T> vector3<T>::cross(const vector3<T>& vec3) {
  vector3<T> t;
  t.m_fX = m_fY * vec3.m_fZ - m_fZ * vec3.m_fY;
  t.m_fY = m_fZ * vec3.m_fX - m_fX * vec3.m_fZ;
  t.m_fZ = m_fX * vec3.m_fY - m_fY * vec3.m_fX;
  return t;
}

template <typename T>
T vector3<T>::dot(const vector3<T>& vec3) {
  T t;
  t = m_fX * vec3.m_fX + m_fY * vec3.m_fY + m_fZ * vec3.m_fZ;
  return t;
}

template <typename T>
void vector3<T>::norm() {
  T t;
  t = m_fX * m_fX + m_fY * m_fY + m_fZ * m_fZ;

  T tmp = sqrt(t);
  m_fX /= tmp;
  m_fY /= tmp;
  m_fZ /= tmp;
}

template <typename T>
double vector3<T>::len() {
  double t = m_fX * m_fX + m_fY * m_fY + m_fZ * m_fZ;
  return sqrt(t);
}

template <typename T>
double vector3<T>::distance(const vector3<T>& other) {
  double x, y, z;
  x = m_fX - other.m_fX;
  y = m_fY - other.m_fY;
  z = m_fZ - other.m_fZ;

  double t = x * x + y * y + z * z;

  return sqrt(t);
}

typedef vector3<float> CVector3;

typedef vector3<double> CPoint3D;

typedef std::vector<CPoint3D> PointsVec;
