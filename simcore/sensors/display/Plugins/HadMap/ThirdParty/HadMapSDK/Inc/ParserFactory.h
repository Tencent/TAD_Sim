// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "DataParse.h"

#include <unordered_map>

namespace hadmap {
class ParserFactory {
 public:
  static DataParsePtr getParser(DataParse::MapDataType type);
};
}  // namespace hadmap
