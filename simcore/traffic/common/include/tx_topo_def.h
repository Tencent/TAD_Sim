// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <vector>
#include "tx_header.h"
TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(Topology)

using txEdge = std::pair<Base::txInt, Base::txInt>;
using txEdgeVec = std::vector<txEdge>;

TX_NAMESPACE_CLOSE(Topology)
TX_NAMESPACE_CLOSE(Geometry)
