// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_header.h"
namespace hadmap {
struct txOdHeader::HeaderData {
 public:
  std::string m_date;
  std::string m_name;
  double m_east;
  double m_north;
  double m_south;
  double m_west;
  std::string m_revMajor;
  std::string m_revMinor;
  std::string georeference;
  std::string m_vendor;
  HeaderData() {}
  ~HeaderData() {}
};
txOdHeader::txOdHeader() { instancePtr.reset(new HeaderData); }

txOdHeader::~txOdHeader() {}

std::string txOdHeader::getHeaderDate() { return instancePtr->m_date; }

txOdHeader& txOdHeader::setHeaderDate(const std::string& date) {
  instancePtr->m_date = date;
  return *this;
}
std::string txOdHeader::getName() { return instancePtr->m_name; }
txOdHeader& txOdHeader::setName(const std::string& name) {
  instancePtr->m_name = name;
  return *this;
}
std::string txOdHeader::getVendor() { return instancePtr->m_vendor; }
txOdHeader& txOdHeader::setVendor(const std::string& vendor) {
  instancePtr->m_vendor = vendor;
  return *this;
}
uint8_t txOdHeader::getRevMajor() { return std::atoi(instancePtr->m_revMajor.c_str()); }
txOdHeader& txOdHeader::setRevMajor(const uint8_t& revMajor) {
  instancePtr->m_revMajor = std::to_string(revMajor);
  return *this;
}

uint8_t txOdHeader::getRevMinor() { return uint8_t(std::atoi(instancePtr->m_revMinor.c_str())); }

txOdHeader& txOdHeader::setRevMinor(const uint8_t& reminor) {
  instancePtr->m_revMinor = std::to_string(reminor);
  return *this;
}

double txOdHeader::getEast() { return instancePtr->m_east; }

txOdHeader& txOdHeader::setEast(const double& east) {
  instancePtr->m_east = east;
  return *this;
}

double txOdHeader::getNorth() { return instancePtr->m_north; }

txOdHeader& txOdHeader::setNorth(const double& north) {
  instancePtr->m_north = north;
  return *this;
}

double txOdHeader::getSouth() { return instancePtr->m_south; }

txOdHeader& txOdHeader::setSouth(const double& sourth) {
  instancePtr->m_south = sourth;
  return *this;
}

double txOdHeader::getWest() { return instancePtr->m_west; }

txOdHeader& txOdHeader::setWest(const double& west) {
  instancePtr->m_west = west;
  return *this;
}

std::string txOdHeader::getGeoReference() { return instancePtr->georeference; }

txOdHeader& txOdHeader::setGeoreference(const std::string& reference) {
  instancePtr->georeference = reference;
  return *this;
}

// txPoint txOdHeader::getOrigin(CoordType cotype)
// {
//  if (cotype == CoordType::COORD_XYZ)
//  {
//    return txPoint(instancePtr->m_west, instancePtr->m_south, 0);
//  }  else {
//    return txPoint(instancePtr->m_west, instancePtr->m_south, 0);
//  }
// }

}  // namespace hadmap
