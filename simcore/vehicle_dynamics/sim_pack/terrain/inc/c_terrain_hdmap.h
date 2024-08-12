#ifndef _TERRAIN_HDMAP_H_
#define _TERRAIN_HDMAP_H_

#include "c_opencrg.h"
#include "c_road_data_mgr.h"
#include "c_terrain_common.h"

#include <chrono>
#include <condition_variable>

using namespace std;
using namespace tx_opencrg;

namespace terrain {

class CTerrainHDMap {
 public:
  CTerrainHDMap();

  ~CTerrainHDMap();

  // update road one
  bool updateRoadsOnce(const bool &skip_overpass_check = false);

  // update ego location
  void setEgoLocation(const terrain::Vector3d &egoLocENU);

  // reset
  bool resetTerrainHDMap(const std::string &hdmapPath, const terrain::Vector3d &hdmapOri, const bool &useOpenCRG,
                         const std::string &crgFile);

  // release
  void releaseTerrain();

  // check if is terrain data valid
  bool isTerrainDataValid();

  // get terrain info by given point
  bool getTerrainElement(const terrain::Vector3d &given_pt_enu, GridElement &out);

  // is terrain fresh, ego inside terrain block
  bool isTerrainFresh();

  // start thread to update terrain
  void startUpdateTerrain_Thread();

 private:
  // disconnect hdmap
  int disconnectHDMap();

  // connect hdmap
  int connectHDMap(const std::string &hdmapPath, const terrain::Vector3d &hdmapOri);

  // release road data manager
  void releaseRoadDataMgr();

 private:
  std::string _hdmapPath;
  hadmap::txPoint _hdmapOri;
  terrain::Vector3d _hdmapOriENU;
  hadmap::MAP_DATA_TYPE _constMapType = hadmap::OPENDRIVE;
  hadmap::txMapHandle *_hdmapHandle;
  int _hadMapConnectStatus;
  TerrainElement _lastTerrainEle;
  bool _async;

 private:
  terrain::CRoadDataMgr *_roadDataMgrPtr;
  tx_opencrg::COpenCRGPtr _crgPtr;
  bool _useOpenCRG;
};
}  // namespace terrain

#endif
