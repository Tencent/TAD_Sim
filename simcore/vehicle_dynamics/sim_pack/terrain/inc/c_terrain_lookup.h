#pragma once

#include "c_terrain_common.h"

namespace terrain {
class CTerrainLookup {
 public:
  static void updateTerrainMapPtr(TerrainMapPtr terrain_ptr);
  static int getHeightAtWorldPosition(double x, double y, double z0, double* z, double* dzdy, double* dzdx, double* mu);

 private:
  static TerrainMapPtr s_terrain_ptr;
  static GridElement s_last_element;
};
}  // namespace terrain
