/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <iomanip>

#ifndef TX_VARS_NAME
#  define TX_VARS_NAME(_NAME_, _VALUE_) " " #_NAME_ " = " << (_VALUE_) << ", "
#endif

#ifndef TX_NOEXCEPT
#  define TX_NOEXCEPT noexcept
#endif

#ifndef TX_NAMESPACE_OPEN
#  define TX_NAMESPACE_OPEN(_NAME_) namespace _NAME_ {
#endif

#ifndef TX_NAMESPACE_CLOSE
#  define TX_NAMESPACE_CLOSE(_NAME_) }
#endif

#ifndef FloatPrecision
#  define FloatPrecision (14)
#endif

#ifndef _StreamPrecision_
#  define _StreamPrecision_ std::setprecision(FloatPrecision) << std::fixed
#endif
