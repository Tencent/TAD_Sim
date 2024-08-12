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
  txMapDataCache() {
    m_strSuccess = "true";
    m_strFailed = "false";
  }
  typedef std::map<std::string, ODR::CHadmap*> HadmapCaches;

 public:
  static txMapDataCache& Instance();
  // void closeHadmap();
  // load map
  const char* LoadHadMap(const char* hadmapFile);

  // create hadmap
  const char* CreateHadMap(const char* hadmapPath, const Base::txStringStream& cmd_json);

  // save hadmap
  const char* SaveHadMap(const char* hadmapPath, const char* outhadmap);

  // modity hadmap
  const char* ModifyHadMap(const char* hadmapPath, const Base::txStringStream& cmd_json);

  // close hadmap
  const char* CloseHadMap(const char* hadmapPath);

  // to json
  const char* ToJson(Base::txStringStream& cmd_json);

  // get hadmap ptr
  ODR::CHadmap* HadmapCache(const char* hadmapFile);

  // bool ClearMap();

 private:
  std::string m_strFailed;
  std::string m_strSuccess;

 protected:
  HadmapCaches m_hadmapCaches;
  std::string m_currentFilePath;
};
using txMapDataCachePtr = std::shared_ptr<txMapDataCache>;
