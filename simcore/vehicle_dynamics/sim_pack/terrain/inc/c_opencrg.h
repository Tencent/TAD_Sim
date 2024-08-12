#ifndef _COPEN_CRG_H_
#define _COPEN_CRG_H_

#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "crg/crgBaseLib.h"
#include "glog/logging.h"

using namespace std;
using namespace google;

namespace tx_opencrg {
class COpenCRG;

typedef std::shared_ptr<COpenCRG> COpenCRGPtr;
typedef double TerrainElement;

class COpenCRG {
 private:
  std::string _crgFileName;
  bool _crgFileValid;
  tx_opencrg::TerrainElement _lastTerrainEle;

  int _dataSetId;
  int _cpId;

  // for test now, @dhu
  const double _xStart = 234.0, _yStart = 78.0;
  const double _xEnd = 227.0, _yEnd = 83.0;
  const double _xStep = -0.1, _yStep = 0.0714;
  double _x = _xStart, _y = _yStart;

 private:
  void setCRGFile(const std::string &crgFileIn);
  bool loadCRGFile();
  void releaseCRG();

 public:
  void resetCRG(const std::string &crgFileIn);

  ~COpenCRG() { releaseCRG(); }

 private:
  static COpenCRGPtr _crgPtr;
  static std::mutex _crgSingtonLock;

  COpenCRG() {
    _lastTerrainEle = 0.0;
    _dataSetId = -1;
    _cpId = -1;
  }

 public:
  static COpenCRGPtr &getSington();
  static int getHeightAtWorldPosition(double x, double y, double z0, double *z, double *dzdy, double *dzdx, double *mu);
};
}  // namespace tx_opencrg

#endif
