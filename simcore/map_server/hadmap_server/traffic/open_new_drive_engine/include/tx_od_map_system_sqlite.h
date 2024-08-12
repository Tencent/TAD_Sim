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
  virtual ~txMapSystemSqlite() { clear(); }
  virtual void clear() TX_NOEXCEPT TX_OVERRIDE;
  virtual tx_od_error_info load(const Base::txString& strMapPath) TX_NOEXCEPT TX_OVERRIDE;
  virtual tx_od_error_info create(const txString& outMapPath, const txStringStream&& cmd_json) TX_NOEXCEPT TX_OVERRIDE;
  virtual tx_od_error_info save(const Base::txString& strMapPath,
                                const Base::txString& outMapPath) TX_NOEXCEPT TX_OVERRIDE;
  virtual tx_od_error_info modify(const txString& strMapPath, Base::txStringStream&& cmd_json) TX_NOEXCEPT TX_OVERRIDE;
  virtual tx_od_error_info makestring(Base::txStringStream& cmd_json) TX_NOEXCEPT TX_OVERRIDE;
  virtual tx_od_error_info close(const txString& strMapPath) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  txMapDataCachePtr m_data;
};

// using txMapSystemOpenDrivePtr = std::shared_ptr<txMapSystemSqlite>;
TX_NAMESPACE_CLOSE(Server)
