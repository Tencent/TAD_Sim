#include "c_terrain_interface.h"
#include "c_async_task.h"
#include "c_terrain_hdmap.h"
#include "c_terrain_lookup.h"
#include "glog/logging.h"

namespace terrain {

real_t g_friction_coef = 0.9;  // global predefine road friction coeff
std::string g_crg_file;
CRiseUpDetection<double> g_terrain_update_detector(0.0);

CTerrainInterface::CTerrainInterface() {
  m_terrain_map_ptr = nullptr;
  g_friction_coef = 0.9;
}
CTerrainInterface::~CTerrainInterface() { releaseRoad(); }

// init
void CTerrainInterface::initVDRoad(const bool &enableTerrain, const bool &enableOpencrg, real_t friction_coef) {
  m_use_map_terrain = enableTerrain;
  m_use_crg = false;
  g_friction_coef = friction_coef;

  if (enableOpencrg) {
    LOG(ERROR) << "terrain | additional work is needed to use opencrg. please do not use it now.\n";
  }

  if (m_use_map_terrain) {
    LOG(INFO) << "terrain | using hdmap terrain.\n";
  }
}

bool CTerrainInterface::isUsingTerrain() { return m_use_crg || m_use_map_terrain; }

bool CTerrainInterface::resetRoad(const char *map_path, const terrain::Vector3d &hdmapOri,
                                  const terrain::Vector3d &egoInitENU) {
  LOG(INFO) << "terrain | resetting road.\n";

  std::string hdmapPath(map_path);

  if (m_use_map_terrain) {
    // release terrain map
    releaseRoad();

    // async task
    // m_async_task = std::unique_ptr<CAsyncTask>(new CAsyncTask());

    // reset dist detector
    g_terrain_update_detector.reset(0.0);

    // init last enu
    m_ego_last_enu = egoInitENU;
    LOG(INFO) << "ego init enu:" << m_ego_last_enu.x << ", " << m_ego_last_enu.y << ", " << m_ego_last_enu.z << "\n";

    // create terrain map
    makeRoad();

    // init road
    if (m_terrain_map_ptr != nullptr &&
        m_terrain_map_ptr->resetTerrainHDMap(hdmapPath, hdmapOri, m_use_crg, g_crg_file)) {
      m_terrain_map_ptr->setEgoLocation(egoInitENU);
      m_use_map_terrain = m_terrain_map_ptr->updateRoadsOnce(true);
      if (m_use_map_terrain) {
        m_terrain_map_ptr->startUpdateTerrain_Thread();
      }

      LOG(INFO) << "terrain | load hdmap done: " << hdmapPath << "\n";
      return true;
    } else {
      releaseRoad();
      m_use_map_terrain = false;
      LOG(ERROR) << "terrain | fail to load hdmap.\n";

      return false;
    }
  }

  return false;
}

void CTerrainInterface::updateRoad(const double &xo, const double &yo, const double &zo, bool update_asap) {
  if (m_use_map_terrain && m_terrain_map_ptr != nullptr) {
    // ego current enu
    terrain::Vector3d ego_cur_enu{xo, yo, zo};

    // update ego location
    m_terrain_map_ptr->setEgoLocation(ego_cur_enu);

    // wait until terrain is fresh
    while (!m_terrain_map_ptr->isTerrainFresh()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }
}

// get road height information
int CTerrainInterface::getHeightAtWorldPosition(double x, double y, double z0, double *z, double *dzdy, double *dzdx,
                                                double *mu) {
  auto state = CTerrainLookup::getHeightAtWorldPosition(x, y, z0, z, dzdy, dzdx, mu);
  *mu = g_friction_coef;
  return state;
}

bool CTerrainInterface::makeRoad() {
  m_terrain_map_ptr = new CTerrainHDMap();

  if (m_terrain_map_ptr != nullptr) {
    CTerrainLookup::updateTerrainMapPtr(m_terrain_map_ptr);
  }

  return m_terrain_map_ptr != nullptr;
}

void CTerrainInterface::releaseRoad() {
  // m_async_task.release();

  if (m_terrain_map_ptr != nullptr) {
    m_terrain_map_ptr->releaseTerrain();
    delete m_terrain_map_ptr;
    m_terrain_map_ptr = nullptr;
    LOG(INFO) << "terrain | hadmap pointer destoryed.\n";
  }
  CTerrainLookup::updateTerrainMapPtr(nullptr);
}
}  // namespace terrain
