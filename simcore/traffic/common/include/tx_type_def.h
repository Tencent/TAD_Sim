// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <string>
#include "structs/hadmap_predef.h"
#include "tx_marco.h"
#include "types/map_defs.h"
#if USE_TBB
#  include <tbb/tbb.h>
#endif

TX_NAMESPACE_OPEN(Base)

#if UseDouble
using txFloat = double;
#else
using txFloat = float;
#endif
using txString = std::string;
using txInt = int;
using txLaneUId = hadmap::txLaneId;
using txLaneID = hadmap::lanepkid;
using txLaneLinkID = hadmap::lanelinkpkid;
using txSectionID = hadmap::sectionpkid;
using txRoadID = hadmap::roadpkid;
using txBoundaryID = hadmap::laneboundarypkid;
using txSignedRoadID = int64_t;
using txSectionUId = std::pair<txRoadID, txSectionID>;
using txDiff_t = int64_t;
using txSize = std::size_t;
using txUInt = unsigned int;
using txSysId = int64_t;
using txUShort = uint16_t;
using txUByte = unsigned char;
using txUChar = unsigned char;
using txULong = uint64_t;
using txLpsz = char const *;
using txBool = bool;
TX_NAMESPACE_CLOSE(Base)
