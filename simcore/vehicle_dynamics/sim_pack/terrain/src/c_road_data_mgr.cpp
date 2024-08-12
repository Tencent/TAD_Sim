#include "c_road_data_mgr.h"

// #define _ENABLE_SAVE_TERRAIN_GRID_

namespace terrain {
CRoadDataMgr::CRoadDataMgr() {
  hadmap::txPoint pt;

  _envelope.push_back(pt);
  _envelope.push_back(pt);

  _curTerrainEle = terrain::DEFAULT_TERRAIN_ELEMENT;

  // define mesh grid
  _meshGrid.setIntervals(_intervalX, _intervalY);
  _meshGrid.setSize(_envelopeLF - _envelopeLR, _envelopeWR - _envelopeWL);
  _meshGrid.resetSurfaceGrids();

  _meshGridLookUp.setIntervals(_meshGrid.getIntervalX(), _meshGrid.getIntervalY());
  _meshGridLookUp.setSize(_meshGrid.getLength(), _meshGrid.getWidth());
  _meshGridLookUp.resetSurfaceGrids();

  m_stopUpdate = false;
}

CRoadDataMgr::~CRoadDataMgr() { releaseData(); }

void CRoadDataMgr::setHDMaphandle(hadmap::txMapHandle *mapHandle) { _hdmapHandle = mapHandle; }
void CRoadDataMgr::setHDMapOrigin(const hadmap::txPoint &hdmapOri) { _hdmapOri = hdmapOri; }
void CRoadDataMgr::setEgoLocENU(const hadmap::txPoint &egoLocENU) {
  _egoLocGNSSLock.lock();
  _curEgoLocENU = egoLocENU;
  _egoLocGNSSLock.unlock();
}
hadmap::txPoint CRoadDataMgr::getEgoLocENU() {
  std::lock_guard<std::mutex> guard(_egoLocGNSSLock);
  return _curEgoLocENU;
}
void CRoadDataMgr::setEgoLocGNSS(const hadmap::txPoint &egoLocGNS) {
  _egoLocGNSSLock.lock();
  _curEgoLocGNS = egoLocGNS;
  _egoLocGNSSLock.unlock();
}

void CRoadDataMgr::releaseData() { stopThread_UpdateRoad(); }

// check if terrain is fresh
bool CRoadDataMgr::isTerrainFresh() {
  bool fresh = true;
  terrain::SSurfacePt3D pt_enu;

  {
    auto egoENU = getEgoLocENU();
    pt_enu.x = egoENU.x;
    pt_enu.y = egoENU.y;
    pt_enu.z = egoENU.z;
  }

  {
    std::lock_guard<std::mutex> gurad(_meshGridLock);
    fresh = _meshGridLookUp.isInsideEnvelope(pt_enu);
  }

  return fresh;
}

// update ego loc, both enu and gnss
void CRoadDataMgr::updateEnvelope() {
  hadmap::txLanePtr lanePtr;
  terrain::SSurfacePt3D bl, tr;

  _egoLocGNSSLock.lock();

  // update envelope, envelope -> [0] leftbottom, [1] righttop
  _envelope[0].x = _curEgoLocENU.x + _envelopeWL;
  _envelope[0].y = _curEgoLocENU.y + _envelopeLR;
  _envelope[0].z = _curEgoLocENU.z;

  _envelope[1].x = _curEgoLocENU.x + _envelopeWR;
  _envelope[1].y = _curEgoLocENU.y + _envelopeLF;
  _envelope[1].z = _curEgoLocENU.z;

  // update envelope ENU and bottom-left coordinate of surface
  bl.x = _envelope[0].x;
  bl.y = _envelope[0].y;
  bl.z = _envelope[0].z;

  tr.x = _envelope[1].x;
  tr.y = _envelope[1].y;
  tr.z = _envelope[1].z;

  _meshGrid.setCorners(bl, tr);

  _egoLocGNSSLock.unlock();

  // gps envelope
  coord_trans_api::enu2lonlat(_envelope[0].x, _envelope[0].y, _envelope[0].z, _hdmapOri.x, _hdmapOri.y, _hdmapOri.z);
  coord_trans_api::enu2lonlat(_envelope[1].x, _envelope[1].y, _envelope[1].z, _hdmapOri.x, _hdmapOri.y, _hdmapOri.z);
}

// regroup lanelinks that belongs to the same junction
void CRoadDataMgr::groupLaneLinkByJunctionID(hadmap::txLaneLinks &lanelinks, JunctionsType &juncs) {
  // pack data
  for (auto iter = lanelinks.begin(); iter != lanelinks.end(); ++iter) {
    const hadmap::txLaneLinkPtr lanelink_ptr = *iter;
    juncs[lanelink_ptr->getJunctionId()].insert(lanelink_ptr->getId());
  }
}

void CRoadDataMgr::updateRoadsSync(const bool &skip_overpass_check) {
  // CTimeElapsed timer("one step of updateRoadsSync");

  // update ego location
  updateEnvelope();

  // to create surface mesh grid
  hadmap::txLanes lane_list;
  hadmap::txLaneLinks lanelink_list;

  // get lanes and links within envelope
  try {
    // get lanes and lane link within an envelope
    if (hadmap::getLanes(_hdmapHandle, _envelope, lane_list) != TX_HADMAP_DATA_OK) {
      LOG_EVERY_N(ERROR, 100) << "terrain | fail to  get lanes from envelope.\n";
    }

    if (hadmap::getLaneLinks(_hdmapHandle, _envelope, lanelink_list) != TX_HADMAP_DATA_OK) {
      LOG_EVERY_N(ERROR, 100) << "terrain | fail to  get lane links from envelope.\n";
    }

  } catch (exception &e) {
    LOG_EVERY_N(ERROR, 100) << "terrain | fail to get lanes or lane links from envelope. " << e.what() << "\n";
  }

  // reset mesh grid
  _meshGrid.resetSurfaceGrids();

  // generate lane surface
  try {
    constexpr size_t const_max_lane_num = 24;
    std::vector<std::shared_ptr<CLaneData>> lane_data_list;
    lane_data_list.reserve(const_max_lane_num);

    // generate all lane surface mesh
    for (auto laneIter = lane_list.begin(); laneIter != lane_list.end(); ++laneIter) {
      // new lane data
      std::shared_ptr<CLaneData> lane_data = std::make_shared<CLaneData>();

      // set parameters for lane data
      lane_data->setHDMapHandle(_hdmapHandle);
      lane_data->setHDMapOri(_hdmapOri);
      lane_data->setSkipOverpassCheck(skip_overpass_check);

      // update ego location
      _egoLocGNSSLock.lock();
      lane_data->setEgoLocationENU(_curEgoLocENU);
      _egoLocGNSSLock.unlock();

      // hadmap laneptr
      hadmap::txLanePtr &lanePtr = (*laneIter);

      // save lane data ptr
      lane_data_list.push_back(lane_data);
      lane_data->generateLaneSurface(lanePtr, &_meshGrid);
    }
  } catch (exception &e) {
    LOG_EVERY_N(ERROR, 100) << "terrain | fail to create lane surface. " << e.what() << "\n";
  }

  // generate junction surface
  try {
    JunctionsType junctions;

    // group junction links
    groupLaneLinkByJunctionID(lanelink_list, junctions);

    // multi-thread to create junction surface
    constexpr size_t const_max_junc_num = 8;
    std::vector<std::shared_ptr<CLaneData>> junc_data_list;
    junc_data_list.reserve(const_max_junc_num);

    // generate junction surface mesh
    for (auto iter = junctions.begin(); iter != junctions.end(); ++iter) {
      // get lane links from one junction
      hadmap::txLaneLinks junc;
      for (auto iter1 = iter->second.begin(); iter1 != iter->second.end(); ++iter1) {
        hadmap::txLaneLinkPtr link_ptr;
        hadmap::getLaneLink(_hdmapHandle, *iter1, link_ptr);
        if (link_ptr.get()) junc.push_back(link_ptr);
      }

      // CLaneData is used to create junction surface
      std::shared_ptr<CLaneData> junc_data = std::make_shared<CLaneData>();

      junc_data->setHDMapHandle(_hdmapHandle);
      junc_data->setHDMapOri(_hdmapOri);
      junc_data->setSkipOverpassCheck(skip_overpass_check);

      _egoLocGNSSLock.lock();
      junc_data->setEgoLocationENU(_curEgoLocENU);
      _egoLocGNSSLock.unlock();

      // get connected road endpoints using trianle mesh
      std::vector<SSurfacePointVec> endPointsOut;
      junc_data->getLinkRoadsEndPoints(junc, _meshGrid, endPointsOut);

      for (auto &endPoints : endPointsOut) {
        // LOG(INFO) << "points size:" << endPoints.size() << "\n";
        junc_data_list.push_back(junc_data);
        junc_data->generateJunctionSurface(endPoints, &_meshGrid);
      }

      // get connected road endpoints using axy+bx+cy+d mesh
      /*
      for (auto& endPoints : endPointsOut) {
          //LOG(INFO) << "points size:" << endPoints.size() << "\n";
          if (endPoints.size() >= 4) {
              junc_data_list.push_back(junc_data);
              junc_data->generatePolyfitJunctionSurface(endPoints, &_meshGrid);
          }
      }
      */

      // get start and end point of each lane link
      /*
      for (const auto& link : junc) {
          junc_data->generateLaneLinkSurface(link, _meshGrid);
      }
      */
    }
  } catch (exception &e) {
    LOG_EVERY_N(ERROR, 100) << "terrain | fail to create lane link surface. " << e.what() << "\n";
  }

  // complete surface mesh, fill holes
  try {
    terrain::SurfaceGridPtr filledGridElePtrVec;

    // complete grid
    _meshGrid.getFilledElePtrVec(filledGridElePtrVec);
    _meshGrid.completeMeshGrids(filledGridElePtrVec, 2.0);

    // smooth
    _meshGrid.smoothGrids(3);
  } catch (exception &e) {
    LOG_EVERY_N(ERROR, 100) << "terrain | fail to post-process surface. " << e.what() << "\n";
  }

  // save for debug
#ifdef _ENABLE_SAVE_TERRAIN_GRID_
  std::string meshName = "debug_" + std::to_string(debugCounter++);
  _meshGrid.saveGrids(meshName);
#endif

  _meshGridLock.lock();
  _meshGrid.transMeshGridTo(_meshGridLookUp);
  _meshGridLock.unlock();

  // update lastest ego location enu
  _lastEgoLocENU = getEgoLocENU();
}

void CRoadDataMgr::startThread_UpdateRoad() {
  m_threadUpdate = std::thread(&CRoadDataMgr::updateRoad_ThreadFunc, this, false);

  if (!m_threadUpdate.joinable()) {
    LOG(ERROR) << "fail to start thread to update road.";
    return;
  }
}
void CRoadDataMgr::updateRoad_ThreadFunc(bool skipOverPass) {
  bool stop = false;

  // check if stop this thread
  {
    std::lock_guard<std::mutex> guard(m_stopLock);
    stop = m_stopUpdate;
  }

  while (!stop) {
    auto curEgoENU = getEgoLocENU();
    double dist = std::hypot(curEgoENU.x - _lastEgoLocENU.x, curEgoENU.y - _lastEgoLocENU.y);

    if (dist > const_grid_width * 0.4) {
      try {
        updateRoadsSync(skipOverPass);
        _lastEgoLocENU = curEgoENU;
      } catch (const std::exception &e) {
        _lastEgoLocENU = curEgoENU;
        LOG_EVERY_N(ERROR, 100) << "error while update road terrain, " << e.what() << "\n";
      }
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    // check if stop this thread
    {
      std::lock_guard<std::mutex> guard(m_stopLock);
      stop = m_stopUpdate;
    }
  }
}
void CRoadDataMgr::stopThread_UpdateRoad() {
  {
    std::lock_guard<std::mutex> guard(m_stopLock);
    m_stopUpdate = true;
  }

  if (m_threadUpdate.joinable()) {
    m_threadUpdate.join();
    LOG(INFO) << "thread of updating road quit.";
  }
}

// get terrain info
bool CRoadDataMgr::getTerrainInfo(hadmap::txMapHandle *mapHandle, const terrain::Vector3d &givenLocENU,
                                  terrain::TerrainElement &ele) {
  bool meshLookupSucc = false;

  terrain::SSurfacePt3D givenPt;

  givenPt.x = givenLocENU.x;
  givenPt.y = givenLocENU.y;
  givenPt.z = givenLocENU.z;

  _meshGridLock.lock();
  if (_meshGridLookUp.isInsideEnvelope(givenPt)) {
    _meshGridLookUp.getZInfoByXY(givenPt, _gridEle);
    meshLookupSucc = _gridEle.filled;
    if (_gridEle.filled) {
      _curTerrainEle = _gridEle;
    }
  }
  _meshGridLock.unlock();

  ele = _curTerrainEle;

  return meshLookupSucc;
}
}  // namespace terrain
