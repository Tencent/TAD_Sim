#include "c_terrain_hdmap.h"

#include <chrono>
#include <thread>

namespace terrain {
CTerrainHDMap::CTerrainHDMap() {
  _roadDataMgrPtr = nullptr;
  _async = false;
  _useOpenCRG = false;
  _hadMapConnectStatus = TX_HADMAP_DATA_ERROR;
  _hdmapHandle = nullptr;
}

CTerrainHDMap::~CTerrainHDMap() { releaseTerrain(); }

// check if is terrain data valid
bool CTerrainHDMap::isTerrainDataValid() { return _roadDataMgrPtr != nullptr; }

// get terrain info by given point
bool CTerrainHDMap::getTerrainElement(const terrain::Vector3d &given_pt_enu, GridElement &out) {
  if (isTerrainDataValid()) {
    _roadDataMgrPtr->getTerrainInfo(_hdmapHandle, given_pt_enu, out);
    return true;
  }
  return false;
}

bool CTerrainHDMap::resetTerrainHDMap(const std::string &hdmapPath, const terrain::Vector3d &hdmapOri,
                                      const bool &useOpenCRG, const std::string &crgFile) {
  _lastTerrainEle = DEFAULT_TERRAIN_ELEMENT;

  releaseRoadDataMgr();
  disconnectHDMap();

  // connect hdmap and create CRoadDataMgr
  if (connectHDMap(hdmapPath, hdmapOri) == TX_HADMAP_HANDLE_OK) {
    _roadDataMgrPtr = new terrain::CRoadDataMgr();

    // set hdmap handle and hdmap origin to CRoadDataMgr
    _roadDataMgrPtr->setHDMaphandle(_hdmapHandle);
    _roadDataMgrPtr->setHDMapOrigin(_hdmapOri);

    // opencrg
    _useOpenCRG = useOpenCRG;

    if (_useOpenCRG) {
      _crgPtr = tx_opencrg::COpenCRG::getSington();
      _crgPtr->resetCRG(crgFile);
    }

    return true;
  }

  return false;
}

int CTerrainHDMap::connectHDMap(const std::string &hdmapPath, const terrain::Vector3d &hdmapOri) {
  _hadMapConnectStatus = TX_HADMAP_HANDLE_ERROR;

  _hdmapOri.x = hdmapOri.x;
  _hdmapOri.y = hdmapOri.y;
  _hdmapOri.z = hdmapOri.z;

  _hdmapOriENU.x = hdmapOri.x;
  _hdmapOriENU.y = hdmapOri.y;
  _hdmapOriENU.z = hdmapOri.z;

  coord_trans_api::enu2lonlat(_hdmapOriENU.x, _hdmapOriENU.y, _hdmapOriENU.z, _hdmapOri.x, _hdmapOri.y, _hdmapOri.z);

  LOG(INFO) << "terrain| connecting hdmap " << hdmapPath << " with origin long: " << _hdmapOri.x
            << ", lat: " << _hdmapOri.y << ", alt: " << _hdmapOri.z << ".\n";

  if (_hadMapConnectStatus != TX_HADMAP_HANDLE_OK) {
    // get hdmap type
    try {
      std::string maptype;
      terrain::getFileExtension(hdmapPath, maptype);

      if (maptype == ".xodr") {
        LOG(INFO) << "terrain| hdmap type is xodr.\n";
        _hadMapConnectStatus = hadmap::hadmapConnect(hdmapPath.c_str(), hadmap::OPENDRIVE, &_hdmapHandle);
      } else if (maptype == ".sqlite") {
        LOG(INFO) << "terrain| hdmap type is sqlite which is not supported.\n";
        // _hadMapConnectStatus = hadmap::hadmapConnect(hdmapPath.c_str(), hadmap::SQLITE, &_hdmapHandle);
      } else {
        LOG(ERROR) << "terrain| unknown hdmap file type " << maptype << "\n";
      }
    } catch (exception &e) {
      LOG(ERROR) << "terrain | error when connecting hadmap, " << e.what() << "\n";
    }
  }

  LOG(INFO) << "terrain| hdmap connection state is " << _hadMapConnectStatus << "\n";

  return _hadMapConnectStatus;
}

int CTerrainHDMap::disconnectHDMap() {
  if (_hadMapConnectStatus == TX_HADMAP_HANDLE_OK) {
    hadmap::hadmapClose(&_hdmapHandle);
    _hadMapConnectStatus = TX_HADMAP_HANDLE_ERROR;
    _hdmapHandle = nullptr;
    LOG(INFO) << "terrain | hadmap connection released.\n";
  }

  return _hadMapConnectStatus;
}

void CTerrainHDMap::setEgoLocation(const terrain::Vector3d &egoLocENU) {
  if (_hadMapConnectStatus == TX_HADMAP_HANDLE_OK) {
    hadmap::txPoint egoPtGNS, egoPtENU;

    egoPtGNS.x = egoLocENU.x;
    egoPtENU.x = egoLocENU.x;
    egoPtGNS.y = egoLocENU.y;
    egoPtENU.y = egoLocENU.y;
    egoPtGNS.z = egoLocENU.z;
    egoPtENU.z = egoLocENU.z;

    // get ego loc, gnss
    coord_trans_api::enu2lonlat(egoPtGNS.x, egoPtGNS.y, egoPtGNS.z, _hdmapOri.x, _hdmapOri.y, _hdmapOri.z);

    egoPtGNS.z = _hdmapOri.z;

    _roadDataMgrPtr->setEgoLocENU(egoPtENU);
    _roadDataMgrPtr->setEgoLocGNSS(egoPtGNS);
  }
}

bool CTerrainHDMap::isTerrainFresh() {
  if (_hadMapConnectStatus == TX_HADMAP_HANDLE_OK) {
    return _roadDataMgrPtr->isTerrainFresh();
  }
  return true;
}

bool CTerrainHDMap::updateRoadsOnce(const bool &skip_overpass_check) {
  if (_hadMapConnectStatus == TX_HADMAP_HANDLE_OK) {
    _roadDataMgrPtr->updateRoadsSync(skip_overpass_check);
    return true;
  }
  return false;
}

void CTerrainHDMap::startUpdateTerrain_Thread() { _roadDataMgrPtr->startThread_UpdateRoad(); }

void CTerrainHDMap::releaseRoadDataMgr() {
  if (_roadDataMgrPtr != nullptr) {
    _roadDataMgrPtr->stopThread_UpdateRoad();
    delete _roadDataMgrPtr;
    _roadDataMgrPtr = nullptr;
    LOG(INFO) << "terrain | road data manager released.\n";
  }
}

void CTerrainHDMap::releaseTerrain() {
  releaseRoadDataMgr();
  disconnectHDMap();
}
}  // namespace terrain
