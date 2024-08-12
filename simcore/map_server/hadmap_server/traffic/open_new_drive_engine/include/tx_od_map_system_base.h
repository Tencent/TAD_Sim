// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_od_error_code.h"
#include "tx_od_header.h"
TX_NAMESPACE_OPEN(Server)

class txMapSystemBase {
 public:
  using txString = Base::txString;
  using txStringStream = Base::txStringStream;
  using MapType = Base::Enums::MapType;
  using MapSourceType = Base::Enums::MapSourceType;

 public:
  virtual ~txMapSystemBase() { clear(); }
  virtual void clear() TX_NOEXCEPT {
    mValid = true;
    mMapType = MapType::eUnKnown;
    mMapStandardVersion = "";
    mMapSouceLocation = "";
    mMapVersion = "";
    mMapSourceType = MapSourceType::eUnKnown;
  }
  virtual tx_od_error_info load(const txString& strMapPath) TX_NOEXCEPT = 0;
  virtual tx_od_error_info create(const txString& outMapPath, const txStringStream&& cmd_json) TX_NOEXCEPT = 0;
  virtual tx_od_error_info save(const txString& strMapPath, const txString& outMapPath) TX_NOEXCEPT = 0;
  virtual tx_od_error_info makestring(txStringStream& json_ret) TX_NOEXCEPT = 0;
  virtual tx_od_error_info modify(const txString& strMapPath, txStringStream&& cmd_json) TX_NOEXCEPT = 0;
  virtual tx_od_error_info close(const txString& strMapPath) TX_NOEXCEPT = 0;

 public:
  Base::txBool isValid() const TX_NOEXCEPT { return mValid; }
  MapType map_type() const TX_NOEXCEPT { return mMapType; }
  txString map_standard_version() const TX_NOEXCEPT { return mMapStandardVersion; }
  txString map_source_location() const TX_NOEXCEPT { return mMapSouceLocation; }
  txString map_version() const TX_NOEXCEPT { return mMapVersion; }
  MapSourceType map_source_type() const TX_NOEXCEPT { return mMapSourceType; }

 protected:
  void setInValid() TX_NOEXCEPT { mValid = false; }
  MapType& map_type() TX_NOEXCEPT { return mMapType; }
  txString& map_standard_version() TX_NOEXCEPT { return mMapStandardVersion; }
  txString& map_source_location() TX_NOEXCEPT { return mMapSouceLocation; }
  txString& map_version() TX_NOEXCEPT { return mMapVersion; }
  MapSourceType& map_source_type() TX_NOEXCEPT { return mMapSourceType; }

 protected:
  Base::txBool mValid = true;
  MapType mMapType = MapType::eUnKnown;
  txString mMapStandardVersion;
  txString mMapSouceLocation;
  txString mMapVersion;
  MapSourceType mMapSourceType = MapSourceType::eUnKnown;
};

using txMapSystemBasePtr = std::shared_ptr<txMapSystemBase>;
TX_NAMESPACE_CLOSE(Server)
