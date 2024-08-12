// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "mapengine/hadmap_engine.h"
#include "tx_od_map_system_base.h"
#include "tx_od_mapdata.h"
TX_NAMESPACE_OPEN(Server)

class txMapSystemSqlite : public txMapSystemBase {
  using ParentClass = txMapSystemBase;

 public:
  // Destructor
  virtual ~txMapSystemSqlite() { clear(); }

  // Clear the map system data
  virtual void clear() TX_NOEXCEPT TX_OVERRIDE;

  // Load a map from the specified path
  virtual tx_od_error_info load(const Base::txString& strMapPath) TX_NOEXCEPT TX_OVERRIDE;

  // Create a new map with the specified output path and command JSON
  virtual tx_od_error_info create(const txString& outMapPath, const txStringStream&& cmd_json) TX_NOEXCEPT TX_OVERRIDE;

  // Save the map to the specified output path
  virtual tx_od_error_info save(const Base::txString& strMapPath,
                                const Base::txString& outMapPath) TX_NOEXCEPT TX_OVERRIDE;

  // Modify the map with the specified command JSON
  virtual tx_od_error_info modify(const txString& strMapPath, Base::txStringStream&& cmd_json) TX_NOEXCEPT TX_OVERRIDE;

  // Convert the map data to a JSON string
  virtual tx_od_error_info makestring(Base::txStringStream& cmd_json) TX_NOEXCEPT TX_OVERRIDE;

  // Close the map with the specified path
  virtual tx_od_error_info close(const txString& strMapPath) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  // Pointer to the map data cache
  txMapDataCachePtr m_data;
};

// using txMapSystemOpenDrivePtr = std::shared_ptr<txMapSystemSqlite>;
TX_NAMESPACE_CLOSE(Server)
