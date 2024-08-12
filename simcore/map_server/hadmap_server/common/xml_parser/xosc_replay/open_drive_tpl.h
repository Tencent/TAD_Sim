/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "common/xml_parser/xosc_replay/tpl_macros.h"

TX_NAMESPACE_OPEN(SIM)
TX_NAMESPACE_OPEN(ODR)

struct xmlHeader {
  double north;
  double south;
  double east;
  double west;
  std::string geoReference;
  friend std::ostream& operator<<(std::ostream& os, const xmlHeader& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<header " << TX_VARS_NAME(north, std::quoted(std::to_string(v.north)))
       << TX_VARS_NAME(south, std::quoted(std::to_string(v.south)))
       << TX_VARS_NAME(east, std::quoted(std::to_string(v.east)))
       << TX_VARS_NAME(west, std::quoted(std::to_string(v.west))) << ">" << std::endl
       << "<geoReference><![CDATA[" << v.geoReference << "]]></geoReference>" << std::endl
       << "</header>";
    return os;
  }
};

struct xmlOpenDRIVE {
  xmlHeader header;
  friend std::ostream& operator<<(std::ostream& os, const xmlOpenDRIVE& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<OpenDRIVE> " << std::endl << v.header << std::endl << "</OpenDRIVE>";
    return os;
  }
};

using OpenDrivePtr = std::shared_ptr<xmlOpenDRIVE>;

// xodr file to opendrive
extern OpenDrivePtr Load(std::istream& stream);

extern OpenDrivePtr Load(const std::string& file);

TX_NAMESPACE_CLOSE(ODR)
TX_NAMESPACE_CLOSE(SIM)
