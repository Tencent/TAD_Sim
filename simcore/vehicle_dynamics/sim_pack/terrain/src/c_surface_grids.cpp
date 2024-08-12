#include "c_surface_grids.h"

namespace terrain {

void CSurfaceGrids::getGridRow(const int &row, terrain::SurfaceGridPtr &gridVec) {
  for (auto i = 0; i < _nCols; ++i) {
    gridVec.push_back(getGridElePtr(row, i));
  }
}

void CSurfaceGrids::getGridCol(const int &col, terrain::SurfaceGridPtr &gridVec) {
  for (auto i = 0; i < _nRows; ++i) {
    gridVec.push_back(getGridElePtr(i, col));
  }
}

const terrain::GridElementPtr CSurfaceGrids::getGridElePtr(const int &row, const int &col) const {
  // LOG(INFO)<<"vd | mesh grids element size "<<_surGrids.size()<<"\n";
  return (const terrain::GridElementPtr)(&_surGridsPtr[row * _nCols + col]);
}

void CSurfaceGrids::setGridValue(const int &row, const int &col, const terrain::GridElement &gridValueIn) {
  _surGridsPtr[row * _nCols + col] = gridValueIn;
}

int CSurfaceGrids::getRows() const { return _nRows; }

int CSurfaceGrids::getCols() const { return _nCols; }

std::string CSurfaceGrids::getSurName() { return _surName; }

void CSurfaceGrids::setSurName(const std::string &surName) { _surName = surName; }

void CSurfaceGrids::setSize(const real_t &length, const real_t &width) {
  _length = length;
  _width = width;

  _nRows = static_cast<int16_t>(std::round(_width / _intervalY));
  _nCols = static_cast<int16_t>(std::round(_length / _intervalX));

  _nSize = _nRows * _nCols;
}

void CSurfaceGrids::setIntervals(const real_t &intX, const real_t &intY) {
  _intervalX = intX;
  _intervalY = intY;
}

// is inside grids
bool CSurfaceGrids::isInsideEnvelope(const terrain::SSurfacePt3D &givenPtENU) const {
  return givenPtENU.x >= _bottomLeftPt.x && givenPtENU.y >= _bottomLeftPt.y && givenPtENU.x <= _topRightPt.x &&
         givenPtENU.y <= _topRightPt.y;
}

void CSurfaceGrids::saturationGridsPos(int16_t &row, int16_t &col) {
  col = col <= 0 ? 0 : col;
  col = col >= _nCols ? _nCols - 1 : col;
  row = row <= 0 ? 0 : row;
  row = row >= _nRows ? _nRows - 1 : row;
}

// calculate surface grids
void CSurfaceGrids::resetSurfaceGrids() {
  terrain::GridElement grid = SURFACE_GRID_INITIALIZER;

  // new grid memory if needed
  if (_surGridsPtr == nullptr) {
    _surGridsPtr = new terrain::GridElement[_nSize];
    LOG(INFO) << "terrain | new mesh grid:" << _surGridsPtr << "\n";
  }

  for (auto r = 0; r < _nRows; ++r) {
    for (auto c = 0; c < _nCols; ++c) {
      grid.r = r;
      grid.c = c;
      setGridValue(r, c, grid);
    }
  }

  // LOG(INFO)<<"vd | mesh grids created with rows: "<<_nRows<<", cols: "<<_nCols<<"\n";
}

void CSurfaceGrids::setCorners(const terrain::SSurfacePt3D &bl, const terrain::SSurfacePt3D &tr) {
  _bottomLeftPt = bl;
  _topRightPt = tr;
}

// get surface grid postion of given location
void CSurfaceGrids::getSurfaceGridByLoc(const terrain::SSurfacePt3D &givenLocENU, int16_t &row, int16_t &col) {
  // get n row and col of grid
  col = static_cast<int16_t>(std::round((givenLocENU.x - _bottomLeftPt.x) / _intervalX));
  row = static_cast<int16_t>(std::round((givenLocENU.y - _bottomLeftPt.y) / _intervalY));

  saturationGridsPos(row, col);
}

// get location of surface grid
void CSurfaceGrids::getLocBySurfaceGrid(const int &row, const int &col, terrain::SSurfacePt3D &loc) const {
  loc.x = _bottomLeftPt.x + col * _intervalX;
  loc.y = _bottomLeftPt.y + row * _intervalY;
}

// grids grow, 3 x 3 connv
void CSurfaceGrids::gridAutomaticGrow(const terrain::GridElementPtr elePtr,
                                      std::vector<terrain::GridElementPtr> &outerElePtrVecOut) {
  for (int16_t i = -1; i <= 1; ++i) {
    for (int16_t j = -1; j <= 1; ++j) {
      int16_t row = elePtr->r + i, col = elePtr->c + j;

      saturationGridsPos(row, col);

      const terrain::GridElementPtr suroundElePtr = getGridElePtr(row, col);

      if (suroundElePtr != elePtr && !suroundElePtr->filled) {
        suroundElePtr->filled = true;
        suroundElePtr->z = elePtr->z;
        outerElePtrVecOut.push_back(suroundElePtr);
      }
    }
  }
}

// get filled grid element pointer vector
void CSurfaceGrids::getFilledElePtrVec(terrain::SurfaceGridPtr &filledGridElePtrVec) {
  // get all filled grid ele
  for (auto i = 0; i < _nSize; ++i) {
    auto &gridEle = _surGridsPtr[i];
    if (gridEle.filled) {
      filledGridElePtrVec.push_back(&gridEle);
    }
  }
}

// fill unset grid
void CSurfaceGrids::completeMeshGrids(const terrain::SurfaceGridPtr &filledGridElePtrVec, const real_t &expandLength) {
  std::vector<terrain::GridElementPtr> outerElePtrVec, nextOuterElePreVec;

  int maxGrowSteps = static_cast<int>(std::round(expandLength / _intervalX));
  int growStepCounter = 0;

  // get outer grid element pointer
  for (auto eleIter = filledGridElePtrVec.begin(); eleIter != filledGridElePtrVec.end(); ++eleIter) {
    const terrain::GridElementPtr &elePtr = (*eleIter);
    gridAutomaticGrow(elePtr, outerElePtrVec);
  }

  // grow grid with outer element automaticly
  while (outerElePtrVec.size() > 0 && growStepCounter < maxGrowSteps) {
    nextOuterElePreVec.clear();

    for (auto eleIter = outerElePtrVec.begin(); eleIter != outerElePtrVec.end(); ++eleIter) {
      const terrain::GridElementPtr &elePtr = (*eleIter);
      gridAutomaticGrow(elePtr, nextOuterElePreVec);
    }

    outerElePtrVec.clear();
    outerElePtrVec.assign(nextOuterElePreVec.begin(), nextOuterElePreVec.end());

    growStepCounter++;
  }
}

void CSurfaceGrids::smoothGrids(const int &nConv) {
  int16_t halfn = static_cast<int16_t>(std::round((nConv - 1) / 2.0));
  int16_t startR = halfn, startC = halfn, endR = _nRows - halfn, endC = _nCols - halfn;

  for (int16_t i = startR; i < endR; ++i) {
    for (int16_t j = startC; j < endC; ++j) {
      real_t convZ = 0.0;
      int counter = 0;

      for (int16_t ii = -halfn; ii <= halfn; ++ii) {
        for (int16_t jj = -halfn; jj <= halfn; ++jj) {
          int16_t row = i + ii;
          int16_t col = j + jj;

          saturationGridsPos(row, col);

          const terrain::GridElementPtr elePtr = getGridElePtr(row, col);
          if (elePtr->filled) {
            convZ += elePtr->z;
            counter++;
          }
        }
      }

      if (counter > 0) {
        const terrain::GridElementPtr elePtr = getGridElePtr(i, j);
        elePtr->z = convZ / counter;
      }
    }
  }
}

// get terrain info by xy
void CSurfaceGrids::getZInfoByXY(const terrain::SSurfacePt3D &givenENU, terrain::GridElement &gridEle) {
  int16_t r = 0, c = 0;

  getSurfaceGridByLoc(givenENU, r, c);

  gridEle = *getGridElePtr(r, c);

  assert(gridEle.r == r && gridEle.c == c);
}

// save grid,debug function
void CSurfaceGrids::saveGrids(const std::string &fileName) {
  std::ofstream binStream;

  std::string defaultPath;

#ifdef WIN32
  defaultPath = "E:\\tmp\\";
#else
  defaultPath = "/home/dhu/Documents/";
#endif

  binStream.open((defaultPath + fileName + ".pcd").c_str(), std::ios::out);

  size_t counter = 0;
  for (auto i = 0; i < _nRows; ++i) {
    for (auto j = 0; j < _nCols; ++j) {
      const terrain::GridElementPtr elePtr = getGridElePtr(i, j);
      if (elePtr->z > -50.0f) counter++;
    }
  }
  /* save pcd v.7 ascii data*/
  std::string point_size = std::to_string(counter);
  std::string pcd_header =
      "# .PCD v.7 - Point Cloud Data file format\n"
      "VERSION .7\n"
      "FIELDS x y z\n"
      "SIZE 4 4 4\n"
      "TYPE F F F\n"
      "COUNT 1 1 1\n";

  pcd_header = pcd_header + "WIDTH " + point_size.c_str() + "\n";
  pcd_header = pcd_header + "HEIGHT 1\n";
  pcd_header = pcd_header + "VIEWPOINT 0 0 0 1 0 0 0\n";
  pcd_header = pcd_header + "POINTS " + point_size.c_str() + "\n";
  pcd_header = pcd_header + "DATA ascii\n";

  binStream.write(pcd_header.c_str(), pcd_header.size());

  for (auto i = 0; i < _nRows; ++i) {
    for (auto j = 0; j < _nCols; ++j) {
      const terrain::GridElementPtr elePtr = getGridElePtr(i, j);
      if (elePtr->z > -20.0f) {
        real_t z = elePtr->z;
        real_t x = (double)j / 10.0f;
        real_t y = (double)i / 10.0f;
        std::string point = std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) + "\n";
        binStream.write(point.c_str(), point.size());
      }
    }
  }
}

void CSurfaceGrids::setRows(const int &r) { _nRows = r; }

void CSurfaceGrids::setCols(const int &c) { _nCols = c; }

// transfer mesh grid memeory pointer to another object
void CSurfaceGrids::transMeshGridTo(CSurfaceGrids &meshB) {
  // swap mesh memory
  auto mesh_b_grid_ptr = meshB.getMeshMemoryPtr();
  meshB.setMeshMemoryPtr(_surGridsPtr);
  setMeshMemoryPtr(mesh_b_grid_ptr);

  // mesh attribute
  meshB.setIntervals(getIntervalX(), getIntervalY());
  meshB.setSize(getLength(), getWidth());
  meshB.setCorners(getBottomLeft(), getTopRight());

  // LOG(INFO)<<"terrain mesh memory swapped.("<<_surGridsPtr<<", "<<mesh_b_grid_ptr<<")\n";
}

void CSurfaceGrids::releaseMemoryThread(const terrain::GridElement *meshPtr) {
  try {
    delete[] meshPtr;
  } catch (const exception &e) {
    LOG(ERROR) << "terrain | delete memeory exception, " << e.what() << "\n";
  }
}

void CSurfaceGrids::releaseMeshMemory() {
  // std::chrono::high_resolution_clock::time_point t1=std::chrono::high_resolution_clock::now();
  if (_surGridsPtr != nullptr) {
    // delete []_surGridsPtr;
    LOG(INFO) << "terrain | release mesh grid:" << _surGridsPtr << "\n";
    terrain::GridElement *meshPtr = _surGridsPtr;
    _surGridsPtr = nullptr;
    std::thread th = std::thread(std::bind(&CSurfaceGrids::releaseMemoryThread, this, meshPtr));
    th.detach();
  }

  // std::chrono::high_resolution_clock::time_point t2=std::chrono::high_resolution_clock::now();
  // LOG(INFO)<<"terrain | mesh memory destory time elapsed
  // "<<std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()<<"\n";
}

void CSurfaceGrids::setMeshMemoryPtr(terrain::GridElement *gridPtr) { _surGridsPtr = gridPtr; }

void CSurfaceGrids::setNumberOfMeshGrids(const int &nSize) { _nSize = nSize; }

int CSurfaceGrids::getNumberOfMeshGrids() const { return _nSize; }
}  // namespace terrain
