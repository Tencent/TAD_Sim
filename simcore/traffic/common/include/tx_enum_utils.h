// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <type_traits>
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_marco.h"

TX_NAMESPACE_OPEN(Utils)

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
  return static_cast<typename std::underlying_type<E>::type>(e);
}
#if 0
template <typename T>
constexpr T String2ElementType(const Base::txString& strElemType, const T _defaultType) noexcept {
  return __lpsz2enum__(T, strElemType.c_str());
    /*return T::_from_string(strElemType.c_str());*/
    /*T retValue = _defaultType;
    auto opValue = typename T::_from_string(strElemType.c_str());
    if (opValue) {
        retValue = *opValue;
    }
    return retValue;*/
}
#endif
TX_NAMESPACE_CLOSE(Utils)
