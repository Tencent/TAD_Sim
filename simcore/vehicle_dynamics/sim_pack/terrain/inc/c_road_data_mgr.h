#ifndef _Cterrain_DATA_MANAGER_H_
#define _Cterrain_DATA_MANAGER_H_

#include "c_lane_data.h"
#include "c_terrain_common.h"

#include <chrono>
#include <mutex>
#include <thread>

typedef std::pair<hadmap::junctionpkid, std::unordered_set<hadmap::lanelinkpkid>> JunctionType;
typedef std::map<hadmap::junctionpkid, std::unordered_set<hadmap::lanelinkpkid>> JunctionsType;

using namespace std;

namespace terrain {
/*
 * do not support reset, user should manage object by himself
 */
class CRoadDataMgr {
 public:
  CRoadDataMgr();
  ~CRoadDataMgr();

  // set resource
  void setHDMaphandle(hadmap::txMapHandle *mapHandle);
  void setHDMapOrigin(const hadmap::txPoint &hdmapOri);

  // update cyclely
  void setEgoLocENU(const hadmap::txPoint &egoLocENU);
  void setEgoLocGNSS(const hadmap::txPoint &egoLocGNS);

  // update roads
  void updateRoadsSync(const bool &skip_overpass_check = false);

  // get terrain info
  bool getTerrainInfo(hadmap::txMapHandle *mapHandle, const terrain::Vector3d &givenLocENU,
                      terrain::TerrainElement &ele);

  // check if terrain is fresh
  bool isTerrainFresh();

  // start and stop thread to update road
  void startThread_UpdateRoad();
  void stopThread_UpdateRoad();

 private:
  // update ego loc, both enu and gnss
  void updateEnvelope();

  // update road thread func
  void updateRoad_ThreadFunc(bool skipOverPass);

  // get lanelinks that belongs to the same junction
  void groupLaneLinkByJunctionID(hadmap::txLaneLinks &lanelinks, JunctionsType &juncs);

  void releaseData();

  // get ego location ENU
  hadmap::txPoint getEgoLocENU();

 private:
  hadmap::txMapHandle *_hdmapHandle;
  hadmap::txPoint _curEgoLocGNS;
  hadmap::txPoint _curEgoLocENU, _lastEgoLocENU;
  hadmap::txPoint _hdmapOri;
  hadmap::roadpkid _curEgoRoadID;
  terrain::TerrainElement _curTerrainEle;
  terrain::GridElement _gridEle;

  // use envelope to get roads, 200x200 m
  const real_t _envelopeWL = -const_grid_width / 2.0f, _envelopeWR = const_grid_width / 2.0f;
  const real_t _envelopeLR = -const_grid_width / 2.0f, _envelopeLF = const_grid_width / 2.0f;
  hadmap::PointVec _envelope;

  // surface of current roads
  const real_t _intervalX = 0.1, _intervalY = 0.1;

  // surface grid, _meshGrid is used to generate surfacem, _meshGridLookUp is used to look up terrain
  terrain::CSurfaceGrids _meshGrid, _meshGridLookUp;
  std::mutex _meshGridLock;

  // if load road async
  bool _async;

  // data
  std::mutex _egoLocGNSSLock;

  // for debug
  int debugCounter = 0;

  // update road handle
  std::thread m_threadUpdate;
  bool m_stopUpdate;
  std::mutex m_stopLock;
};
}  // namespace terrain

#endif
