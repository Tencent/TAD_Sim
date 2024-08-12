#include "c_opencrg.h"
#include "c_terrain_core.h"

namespace tx_opencrg {
COpenCRGPtr COpenCRG::_crgPtr = nullptr;
std::mutex COpenCRG::_crgSingtonLock;

void COpenCRG::setCRGFile(const std::string &crgFileIn) { _crgFileName = crgFileIn; }

void COpenCRG::releaseCRG() {
  if (_dataSetId > 0) {
    crgDataSetRelease(_dataSetId);
    _dataSetId = -1;
  }
}

bool COpenCRG::loadCRGFile() {
  _crgFileValid = true;

  releaseCRG();

  if ((_dataSetId = crgLoaderReadFile(_crgFileName.c_str())) <= 0) {
    LOG(ERROR) << "vd | fail to load crg file. " << _crgFileName << "\n";
    _crgFileValid = false;
  } else {
    /* --- create a contact point --- */
    _cpId = crgContactPointCreate(_dataSetId);

    if (_cpId < 0) {
      LOG(ERROR) << "vd | could not create contact point.\n";
      _crgFileValid = false;
    } else {
      LOG(ERROR) << "vd | crg file loaded, " << _crgFileName << "\n";
    }
  }

  return _crgFileValid;
}
void COpenCRG::resetCRG(const std::string &crgFileIn) {
  releaseCRG();
  setCRGFile(crgFileIn);

  if (loadCRGFile()) {
    LOG(INFO) << "vd | crg file loaded.\n";
  }
}

COpenCRGPtr &COpenCRG::getSington() {
  COpenCRG::_crgSingtonLock.lock();
  if (COpenCRG::_crgPtr.use_count() == 0) {
    _crgPtr = COpenCRGPtr(new COpenCRG());
  }
  COpenCRG::_crgSingtonLock.unlock();

  return COpenCRG::_crgPtr;
}
int COpenCRG::getHeightAtWorldPosition(real_t x, real_t y, real_t z0, real_t *z, real_t *dzdy, real_t *dzdx,
                                       real_t *mu) {
  /* demo code
  const real_t _xStart=234.0,_yStart=78.0;
  const real_t _xEnd=227.0,_yEnd=83.0;
  const real_t _xStep=-0.1,_yStep=0.714;
  real_t _x=_xStart,_y=_yStart;

  COpenCRG::_crgPtr->_x+=COpenCRG::_crgPtr->_xStep;
  COpenCRG::_crgPtr->_y+=COpenCRG::_crgPtr->_yStep;

  if(COpenCRG::_crgPtr->_x<=COpenCRG::_crgPtr->_xEnd){
      COpenCRG::_crgPtr->_x=COpenCRG::_crgPtr->_xStart;
  }

  if(COpenCRG::_crgPtr->_y>=COpenCRG::_crgPtr->_yEnd){
      COpenCRG::_crgPtr->_y=COpenCRG::_crgPtr->_yStart;
  }
  */
  int zStatus = 0;
  real_t zCrg = 0.0;

  *dzdy = 0.0;
  *dzdx = 0.0;
  *mu = 0.9;

  if (COpenCRG::_crgPtr->_crgFileValid && crgEvalxy2z(COpenCRG::_crgPtr->_cpId, x, y, &zCrg)) {
    *z = zCrg;
    zStatus = 0;
  } else {
    zStatus = -1;
  }

  return zStatus;
}
}  // namespace tx_opencrg
