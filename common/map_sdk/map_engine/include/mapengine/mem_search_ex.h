// Copyright 2024 Tencent Inc. All rights reserved.
//

// Header file for MemSearchEx classes
#pragma once

#include "mapengine/mem_search.h"

namespace hadmap {
// Class for cloud search
class CloudSearch : public MemSearch {
 public:
  CloudSearch(MAP_DATA_TYPE type, const std::string& envelope);

  ~CloudSearch();

 public:
  void init();
};

// Class for OpenDrive search
class OpenDriveSearch : public MemSearch {
 public:
  OpenDriveSearch(MAP_DATA_TYPE type, const std::string& path, bool doubleRoad = true);

  OpenDriveSearch(MAP_DATA_TYPE type, const std::string& path,
                  std::vector<std::tuple<hadmap::txPoint, hadmap::txPoint>> map_range_list);

  ~OpenDriveSearch();

 public:
  void init();
};

}  // namespace hadmap
