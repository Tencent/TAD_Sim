// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "mapengine/engine_defs.h"
#include "mapengine/search_interface.h"

namespace hadmap {
class SearchFactory {
 public:
  // Get the singleton instance of SearchFactory
  static SearchFactory* getInstance();

 public:
  // Get the search interface for the specified map data type and info
  // @param type: The map data type
  // @param info: The map data info
  // @param isSplitRoad: Whether to split road or not, default is false
  // @return The search interface pointer
  ISearchPtr getSearchInterface(MAP_DATA_TYPE type, const std::string& info, bool isSplitRoad);

  // Get the search area interface for the specified map data type, info, and map range list
  // @param type: The map data type
  // @param info: The map data info
  // @param map_range_list: The list of map ranges
  // @return The search interface pointer
  ISearchPtr getSearchAreaInterface(MAP_DATA_TYPE type, const std::string& info,
                                    std::vector<std::tuple<hadmap::txPoint, hadmap::txPoint>> map_range_list);

 private:
  // Constructor
  SearchFactory();

  // Destructor
  ~SearchFactory();

  // Delete the copy constructor
  SearchFactory(const SearchFactory&) = delete;

  // Singleton instance of SearchFactory
  static SearchFactory* ins;
};
}  // namespace hadmap
