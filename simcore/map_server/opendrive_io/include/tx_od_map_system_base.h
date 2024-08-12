// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_od_error_code.h"
#include "tx_od_header.h"
TX_NAMESPACE_OPEN(Server)

class txMapSystemBase {
 public:
  // Type aliases for convenience
  using txString = Base::txString;
  using txStringStream = Base::txStringStream;
  using MapType = Base::Enums::MapType;
  using MapSourceType = Base::Enums::MapSourceType;

 public:
  // Destructor
  virtual ~txMapSystemBase() { clear(); }

  // Clear the map system data
  virtual void clear() TX_NOEXCEPT {
    mValid = true;
    mMapType = MapType::eUnKnown;
    mMapStandardVersion = "";
    mMapSouceLocation = "";
    mMapVersion = "";
    mMapSourceType = MapSourceType::eUnKnown;
  }

  // Load a map from the specified path
  virtual tx_od_error_info load(const txString& strMapPath) TX_NOEXCEPT = 0;

  // Create a new map with the specified output path and command JSON
  virtual tx_od_error_info create(const txString& outMapPath, const txStringStream&& cmd_json) TX_NOEXCEPT = 0;

  // Save the map to the specified output path
  virtual tx_od_error_info save(const txString& strMapPath, const txString& outMapPath) TX_NOEXCEPT = 0;

  // Convert the map data to a JSON string
  virtual tx_od_error_info makestring(txStringStream& json_ret) TX_NOEXCEPT = 0;

  // Modify the map with the specified command JSON
  virtual tx_od_error_info modify(const txString& strMapPath, txStringStream&& cmd_json) TX_NOEXCEPT = 0;

  // Close the map with the specified path
  virtual tx_od_error_info close(const txString& strMapPath) TX_NOEXCEPT = 0;

 public:
  // Check if the map system is valid
  Base::txBool isValid() const TX_NOEXCEPT { return mValid; }

  // Get the map type
  MapType map_type() const TX_NOEXCEPT { return mMapType; }

  // Get the map standard version
  txString map_standard_version() const TX_NOEXCEPT { return mMapStandardVersion; }

  // Get the map source location
  txString map_source_location() const TX_NOEXCEPT { return mMapSouceLocation; }

  // Get the map version
  txString map_version() const TX_NOEXCEPT { return mMapVersion; }

  // Get the map source type
  MapSourceType map_source_type() const TX_NOEXCEPT { return mMapSourceType; }

 protected:
  // Set the map system as invalid
  void setInValid() TX_NOEXCEPT { mValid = false; }

  // Get a reference to the map type
  MapType& map_type() TX_NOEXCEPT { return mMapType; }

  // Get a reference to the map standard version
  txString& map_standard_version() TX_NOEXCEPT { return mMapStandardVersion; }

  // Get a reference to the map source location
  txString& map_source_location() TX_NOEXCEPT { return mMapSouceLocation; }

  // Get a reference to the map version
  txString& map_version() TX_NOEXCEPT { return mMapVersion; }

  // Get a reference to the map source type
  MapSourceType& map_source_type() TX_NOEXCEPT { return mMapSourceType; }

 protected:
  // Member variables
  Base::txBool mValid = true;
  MapType mMapType = MapType::eUnKnown;
  txString mMapStandardVersion;
  txString mMapSouceLocation;
  txString mMapVersion;
  MapSourceType mMapSourceType = MapSourceType::eUnKnown;
};

using txMapSystemBasePtr = std::shared_ptr<txMapSystemBase>;
TX_NAMESPACE_CLOSE(Server)
