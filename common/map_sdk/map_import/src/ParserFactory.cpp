// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "ParserFactory.h"
#include "OpenDriveParse.h"

namespace hadmap {
DataParsePtr ParserFactory::getParser(DataParse::MapDataType type) {
  switch (type) {
    case DataParse::OPENDRIVE:
      return DataParsePtr(new ODParse);
      break;
    default:
      return NULL;
      break;
  }
}
}  // namespace hadmap
