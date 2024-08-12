// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "ParseInterface.h"
#include "DataParse.h"
#include "ParserFactory.h"

namespace hadmap {
using AreaPointVec = std::vector<std::tuple<hadmap::txPoint, hadmap::txPoint>>;
bool parseOpenDrive(const std::string& data_path, txRoads& roads, txLaneLinks& links, txObjects& objs,
                    txJunctions& juncs, txOdHeaderPtr& header, txPoint& refPoint, bool doubleRoad) {
  DataParsePtr _parser = ParserFactory::getParser(DataParse::OPENDRIVE);
  if (_parser == nullptr) return false;
  return _parser->parse(data_path) && _parser->getData(roads, links, objs, juncs, header, refPoint, doubleRoad);
}
bool parseOpenDriveArea(const std::string& data_path, txRoads& roads, txLaneLinks& links, txObjects& objs,
                        txJunctions& juncs, AreaPointVec ptr, txOdHeaderPtr& header, txPoint& refPoint) {
  DataParsePtr _parser = ParserFactory::getParser(DataParse::OPENDRIVE);
  if (_parser == nullptr) return false;
  return _parser->parseArea(data_path, ptr) && _parser->getData(roads, links, objs, juncs, header, refPoint, true);
}

}  // namespace hadmap
