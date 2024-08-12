// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <mapengine/hadmap_codes.h>
#include <mapengine/hadmap_engine.h>
#include <map>
#include <string>
#include "tx_hadmap_cache.h"
#include "tx_od_header.h"
struct txMapDataCache {
  // Constructor
  txMapDataCache() {
    m_strSuccess = "true";
    m_strFailed = "false";
  }

  // Type alias for a map of HAD maps
  typedef std::map<std::string, ODR::CHadmap*> HadmapCaches;

 public:
  // Get the singleton instance of txMapDataCache
  static txMapDataCache& Instance();

  // Load a HAD map from the specified file path
  const char* LoadHadMap(const char* hadmapFile);

  // Create a HAD map from the specified file path and JSON
  const char* CreateHadMap(const char* hadmapPath, const Base::txStringStream& cmd_json);

  // Save a HAD map from the specified file path
  const char* SaveHadMap(const char* hadmapPath, const char* outhadmap);

  // Modify a HAD map from the specified file path
  const char* ModifyHadMap(const char* hadmapPath, const Base::txStringStream& cmd_json);

  // Close a HAD map from the specified file path
  const char* CloseHadMap(const char* hadmapPath);

  // Convert map data to JSON
  const char* ToJson(Base::txStringStream& cmd_json);

  // Get a pointer to the HAD map with the specified file path
  ODR::CHadmap* HadmapCache(const char* hadmapFile);

 private:
  // Member variables for success and failure strings
  std::string m_strFailed;
  std::string m_strSuccess;

 protected:
  // Member variable for the map of HAD maps
  HadmapCaches m_hadmapCaches;

  // Member variable for the current file path
  std::string m_currentFilePath;
};
using txMapDataCachePtr = std::shared_ptr<txMapDataCache>;
