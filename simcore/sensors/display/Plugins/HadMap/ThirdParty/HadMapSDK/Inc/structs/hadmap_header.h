// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "structs/base_struct.h"
namespace hadmap {

class TXSIMMAP_API txOdHeader {
 private:
  struct HeaderData;

  std::unique_ptr<HeaderData> instancePtr;

 public:
  txOdHeader();

  ~txOdHeader();
  // Get the date of the map header
  std::string getHeaderDate();
  // Set the date of the map header
  txOdHeader& setHeaderDate(const std::string& date);

  // Get the vendor information of the map header
  std::string getVendor();

  // Set the vendor information of the map header
  txOdHeader& setVendor(const std::string& name);

  // Get the name of the map header
  std::string getName();

  // Set the name of the map header
  txOdHeader& setName(const std::string& name);

  // Get the major revision number of the map header
  uint8_t getRevMajor();

  // Set the major revision number of the map header
  txOdHeader& setRevMajor(const uint8_t& revMajor);

  // Get the minor revision number of the map header
  uint8_t getRevMinor();

  // Set the minor revision number of the map header
  txOdHeader& setRevMinor(const uint8_t& reminor);

  // Get the east boundary of the map header
  double getEast();

  // Set the east boundary of the map header
  txOdHeader& setEast(const double& east);

  // Get the north boundary of the map header
  double getNorth();

  // Set the north boundary of the map header
  txOdHeader& setNorth(const double& north);

  // Get the south boundary of the map header
  double getSouth();

  // Set the south boundary of the map header
  txOdHeader& setSouth(const double& sourth);

  // Get the west boundary of the map header
  double getWest();

  // Set the west boundary of the map header
  txOdHeader& setWest(const double& west);

  // Get the geographic reference information of the map header
  std::string getGeoReference();

  // Set the geographic reference information of the map header
  txOdHeader& setGeoreference(const std::string& reference);

  // typedef std::shared_ptr< txOdHeader > txOdHeaderPtr;
};
typedef std::shared_ptr<txOdHeader> txOdHeaderPtr;
}  // namespace hadmap
