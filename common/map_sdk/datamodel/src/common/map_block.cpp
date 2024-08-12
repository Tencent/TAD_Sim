// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "common/map_block.h"
#include <stdio.h>
#include <iostream>

namespace map_block {
const int LON_SPAN = 6;
const int LAT_SPAN = 4;

const char SCALE_CODE = 'H';
const int BLOCK_NUMBER = 192;

static double ROUND(double x, int n) { return static_cast<double>(static_cast<int>(x / n) * n); }

static std::string XY2BlockID(double x, double y) {
  int nLong = static_cast<int>(x / LON_SPAN) + 31;
  char cLat = static_cast<int>(y / LAT_SPAN) + 'A';

  int nCow = static_cast<int>((x - ROUND(x, LON_SPAN)) * (BLOCK_NUMBER / LON_SPAN)) + 1;
  int nRow = static_cast<int>(BLOCK_NUMBER - (y - ROUND(y, LAT_SPAN)) * (BLOCK_NUMBER / LAT_SPAN)) + 1;
  if (nRow > BLOCK_NUMBER) nRow = BLOCK_NUMBER;

  char buffer[16];
  sprintf(buffer, "%c%02d%c%03d%03d", cLat, nLong, SCALE_CODE, nRow, nCow);
  return std::string(buffer);
}

void GenerateBlockIds(const hadmap::Points2d& envelope, std::set<std::string>& blockIds) {
  blockIds.clear();
  double locs[] = {envelope[1].x, envelope[1].y, envelope[1].x, envelope[0].y,
                   envelope[0].x, envelope[1].y, envelope[0].x, envelope[0].y};
  for (size_t i = 0; i < 8; i += 2) blockIds.insert(XY2BlockID(locs[i], locs[i + 1]));
}

std::string GenerateBlockStr(const std::set<std::string>& blockIds) {
  std::string blockStr = "";
  for (std::set<std::string>::const_iterator itr = blockIds.begin(); itr != blockIds.end(); ++itr)
    blockStr += "\"" + *itr + "\",";
  return blockStr.substr(0, blockStr.length() - 1);
}
}  // namespace map_block
