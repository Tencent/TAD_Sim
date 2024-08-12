// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "mapengine/search_factory.h"
#include "mapengine/local_search.h"
#include "mapengine/mem_search.h"
#include "mapengine/mem_search_ex.h"
#include "mapengine/remote_search.h"

namespace hadmap {
SearchFactory* SearchFactory::ins = NULL;

SearchFactory::SearchFactory() {}

SearchFactory::~SearchFactory() {}

SearchFactory* SearchFactory::getInstance() {
  if (ins == NULL) ins = new SearchFactory;
  return ins;
}

ISearchPtr SearchFactory::getSearchInterface(MAP_DATA_TYPE type, const std::string& info, bool doubleRoad) {
  if (type == REMOTE) {
    return ISearchPtr(new RemoteSearch(type, info));
  } else if (type == MEMORY) {
    return ISearchPtr(new CloudSearch(type, info));
  } else if (type == OPENDRIVE) {
    return ISearchPtr(new OpenDriveSearch(type, info, doubleRoad));
  } else {
    return ISearchPtr(new LocalSearch(type, info));
  }
}

ISearchPtr SearchFactory::getSearchAreaInterface(
    MAP_DATA_TYPE type, const std::string& info,
    std::vector<std::tuple<hadmap::txPoint /*left_bottom*/, hadmap::txPoint /*right_top*/> > map_range_list) {
  if (type == REMOTE) {
    return nullptr;
  } else if (type == MEMORY) {
    return ISearchPtr(new CloudSearch(type, info));
  } else if (type == OPENDRIVE) {
    return ISearchPtr(new OpenDriveSearch(type, info, map_range_list));
  } else {
    return ISearchPtr(new LocalSearch(type, info));
  }
}
}  // namespace hadmap
