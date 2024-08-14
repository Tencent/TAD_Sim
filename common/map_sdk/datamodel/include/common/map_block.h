// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "common/macros.h"
#include "structs/map_structs.h"

#include <set>
#include <string>

namespace map_block {
// Based on the envelope, generate block IDs (J50H012076)
TXSIMMAP_API void GenerateBlockIds(const hadmap::Points2d& envelope, std::set<std::string>& blockIds);

// Generate a block string from the block IDs
TXSIMMAP_API std::string GenerateBlockStr(const std::set<std::string>& blockIds);
}  // namespace map_block
