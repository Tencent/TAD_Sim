// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/container/flat_set.hpp>
#include "tx_header.h"

TX_NAMESPACE_OPEN(Base)
template <typename T>
using txFlatSet = boost::container::flat_set<T>;
TX_NAMESPACE_CLOSE(Base)
