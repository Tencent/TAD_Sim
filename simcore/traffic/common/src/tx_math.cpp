// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_math.h"
#include <boost/math/constants/constants.hpp>
#include <limits>

TX_NAMESPACE_OPEN(Math)

Base::txBool isZero(const Base::txFloat val, const Base::txFloat _eps) TX_NOEXCEPT {
  if (val > (0.0 - _eps) && val < _eps) {
    return true;
  } else {
    return false;
  }
}

Base::txBool isNotZero(const Base::txFloat val) TX_NOEXCEPT { return !isZero(val); }

Base::txFloat Power(const Base::txFloat _value, const Base::txInt count) TX_NOEXCEPT {
  Base::txFloat value = _value;
  for (unsigned int i = 0; i < count - 1; i++) value *= _value;

  return value;
}

Base::txFloat ProjectPoint2Line(const Base::txVec2& p, const Base::txVec2& start, const Base::txVec2& end,
                                Base::txVec2& projectP) TX_NOEXCEPT {
  Base::txVec2 se = end - start;
  Base::txVec2 v = p - start;
  const Base::txFloat se_norm2 = se.squaredNorm();
  if (se_norm2 < EPSILON) {
    return 0.0;
  } else {
    Base::txFloat u = (v.dot(se)) / se_norm2;
    if (u > 1.0) {
      u = 1.0;
    } else if (u < 0.0) {
      u = 0.0;
    }
    projectP = (start + se * u);
    return (projectP - start).norm();
  }
}

TX_NAMESPACE_CLOSE(Math)
