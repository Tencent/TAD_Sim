#include "c_terrain_hdmap.h"
#include "c_terrain_lookup.h"

namespace terrain {
TerrainMapPtr CTerrainLookup::s_terrain_ptr = nullptr;
GridElement CTerrainLookup::s_last_element = SURFACE_GRID_FLAT;

void CTerrainLookup::updateTerrainMapPtr(TerrainMapPtr terrain_ptr) { s_terrain_ptr = terrain_ptr; }

int CTerrainLookup::getHeightAtWorldPosition(double x, double y, double z0, double* z, double* dzdy, double* dzdx,
                                             double* mu) {
  terrain::Vector3d givenLocENU;

  givenLocENU.x = x;
  givenLocENU.y = y;
  givenLocENU.z = z0;

  GridElement terrainEle;

  if (s_terrain_ptr && s_terrain_ptr->getTerrainElement(givenLocENU, terrainEle)) {
    *z = terrainEle.z;
    *dzdy = terrainEle.dzdy;
    *dzdx = terrainEle.dzdx;
    s_last_element = terrainEle;
  } else {
    *z = s_last_element.z;
    *dzdy = s_last_element.dzdy;
    *dzdx = s_last_element.dzdx;
  }
  *mu = 0.9;

  return 0;
}
}  // namespace terrain
