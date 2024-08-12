/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

template <typename T>
class vector2 {
 public:
  vector2();
  vector2(T x, T y);

 protected:
  T mX;
  T mY;
};

template <typename T>
vector2<T>::vector2() {
  mX = (T)0;
  mY = (T)0;
}

template <typename T>
vector<T>::vector2(T x, T y) {
  mX = x;
  mY = y;
}

typedef vector2<float> CVector2;
