#ifndef _TERRAIN_HDMAP_COMMON_H_
#define _TERRAIN_HDMAP_COMMON_H_

#include <assert.h>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "c_terrain_core.h"
#include "common/coord_trans.h"
#include "eigen3/Eigen/Dense"
#include "hadmap.h"
#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"
#include "structs/map_structs.h"

#include "glog/logging.h"

using namespace std;
using namespace google;
using namespace hadmap;
using namespace coord_trans_api;

// #define _ENABLE_SAVE_TERRAIN_GRID_
#define SURFACE_GRID_INITIALIZER \
  { 0, 0, -100.0, 0.0, 0.0, false }
#define SURFACE_GRID_FLAT \
  { 0, 0, 0.0f, 0.0, 0.0, false }

#define INFINITE_POSITIVE INT32_MAX * 1.0
#define INFINITE_NEGATIVE INT32_MIN * 1.0

#define OVERPASS_HEIGHT_THRESH 3.8

namespace terrain {
// surface grid element
struct MODULE_API GridElement {
  int16_t r;    // row index
  int16_t c;    // coloumn index
  real_t z;     // height
  real_t dzdy;  // dzdy
  real_t dzdx;  // dzdx
  bool filled;  // filled or not
};

// grid width
const real_t const_grid_width = 200.0f;

// junction endpoint height thresh
const real_t const_junc_point_dist_thresh = 0.5f;

class CLaneSurface;
class CLaneData;
class CRoadData;
class CRoadDataMgr;
class CTerrainHDMap;

using CLaneSurfacePtr = CLaneSurface *;
using CLaneDataPtr = CLaneData *;
using GridElementPtr = GridElement *;
using SurfaceGridPtr = std::vector<GridElementPtr>;

class CAsyncTask;
class CThreadPool;

using CAsyncTaskPtr = std::unique_ptr<CAsyncTask>;
using CThreadPoolPtr = std::shared_ptr<CThreadPool>;

// terrain element
typedef GridElement TerrainElement;
const TerrainElement DEFAULT_TERRAIN_ELEMENT = SURFACE_GRID_INITIALIZER;

// for road surface
typedef hadmap::Point3d SSurfacePt3D;
typedef hadmap::Points3d SSurfacePointVec;
typedef hadmap::Points3d SSurfaceBorder;
typedef std::vector<SSurfaceBorder> SSurfaceBorders;

// terrain hdmap class ptr
using TLaneSurfacePtr = std::shared_ptr<CLaneSurface>;
using TRoadDataPtr = std::shared_ptr<CRoadData>;
using TRoadDataMgrPtr = std::shared_ptr<CRoadDataMgr>;
using TerrainMapPtr = CTerrainHDMap *;

// pair, first: road id, second: road data map
typedef std::map<hadmap::roadpkid, TRoadDataPtr> TRoadDataMap;
typedef std::vector<hadmap::roadpkid> TRoadIDList;

// lane data map, first: id, second: data
typedef std::map<hadmap::roadpkid, CLaneSurface> TLaneDataMap;
typedef std::pair<hadmap::roadpkid, CLaneSurface> TLaneLinkDataPair;

// time measurment
class CTimeElapsed {
 public:
  CTimeElapsed(const std::string &prefix = "");
  virtual ~CTimeElapsed();

 private:
  std::chrono::high_resolution_clock::time_point m_t_start, m_t_end;
  std::string m_prefix;
};

// split string
void stringSplit(const std::string &strIn, const std::string &spliter, std::vector<std::string> &splitedStrs);

void getFileExtension(const std::string &filePath, std::string &extension);

// rise up detection
template <typename Ty>
class CRiseUpDetection {
 public:
  CRiseUpDetection(const Ty &init_value) : m_last_value(init_value) {}
  virtual ~CRiseUpDetection() {}

  /// <summary>
  /// return true if rise-up detected
  /// </summary>
  /// <param name="value">current value</param>
  /// <param name="thresh">thresh</param>
  /// <returns></returns>
  bool detect(const Ty &value, const Ty &thresh) {
    bool rise_up = m_last_value < thresh && thresh <= value;
    m_last_value = value;
    return rise_up;
  }
  void reset(const Ty &init_value) { m_last_value = init_value; }

 protected:
  Ty m_value;
  Ty m_last_value;
};
}  // namespace terrain

#endif
