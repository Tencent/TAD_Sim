// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_geometry_element_base.h"
#include <sstream>
#include "tx_string_utils.h"
TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(Element)

txGeomElementBase::txString txGeomElementBase::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{" << TX_COND_NAME(valid, IsValid()) << TX_VARS_NAME(type, ElemTypeStr()) << TX_VARS_NAME(len, RealLength())
      << TX_VARS_NAME(start, Utils::ToString(StartVtx())) << TX_VARS_NAME(end, Utils::ToString(EndVtx())) << "}";
  return oss.str();
}

TX_NAMESPACE_CLOSE(Element)
TX_NAMESPACE_CLOSE(Geometry)
