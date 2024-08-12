#ifndef _VD_ROAD_H_
#define _VD_ROAD_H_

#include "c_terrain_core.h"

namespace terrain {
class CTerrainInterface final {
 public:
  MODULE_API CTerrainInterface();
  MODULE_API ~CTerrainInterface();

  CTerrainInterface(const CTerrainInterface &) = delete;
  CTerrainInterface &operator=(const CTerrainInterface &) = delete;

 public:
  // init road configuration
  MODULE_API void initVDRoad(const bool &enableTerrain, const bool &enableOpencrg = false, real_t friction_coef = 0.9);

  // reset road
  MODULE_API bool resetRoad(const char *hdmapPath, const terrain::Vector3d &hdmapOri,
                            const terrain::Vector3d &egoInitENU);

  // ENU coordinate
  MODULE_API void updateRoad(const double &xo, const double &yo, const double &zo, bool update_asap = false);

  // release road
  MODULE_API void releaseRoad();

  // if using terrain
  MODULE_API bool isUsingTerrain();

 public:
  // get road height information
  MODULE_API static int getHeightAtWorldPosition(double x, double y, double z0, double *z, double *dzdy, double *dzdx,
                                                 double *mu);

 private:
  // make road
  bool makeRoad();

 private:
  // terrain based on hdmap
  terrain::TerrainMapPtr m_terrain_map_ptr;

  bool m_use_map_terrain;
  bool m_use_crg;

  // last terrain ego location
  terrain::Vector3d m_ego_last_enu;

  // an async to update terrain
  // CAsyncTaskPtr m_async_task;
};
}  // namespace terrain

#endif
