#ifndef _CSurface_Grids_H_
#define _CSurface_Grids_H_

#include <fstream>
#include <iostream>
#include "c_terrain_common.h"

using namespace std;

namespace terrain {
class CSurfaceGrids {
 private:
  // mesh grid pointer
  terrain::GridElement *_surGridsPtr;
  std::string _surName;
  int16_t _nRows, _nCols;
  int _nSize;
  real_t _intervalX, _intervalY;
  real_t _length, _width;
  terrain::SSurfacePt3D _bottomLeftPt, _topRightPt;

 private:
  void releaseMemoryThread(const terrain::GridElement *meshPtr);

 public:
  CSurfaceGrids() { _surGridsPtr = nullptr; }
  ~CSurfaceGrids() { releaseMeshMemory(); }

  // no copy construction
  CSurfaceGrids &operator=(const CSurfaceGrids &meshA) = delete;
  CSurfaceGrids(const CSurfaceGrids &) = delete;

  void getGridRow(const int &row, terrain::SurfaceGridPtr &gridVec);

  void getGridCol(const int &col, terrain::SurfaceGridPtr &gridVec);

  const terrain::GridElementPtr getGridElePtr(const int &row, const int &col) const;

  int getRows() const;
  int getCols() const;
  inline real_t getIntervalX() const { return _intervalX; }
  inline real_t getIntervalY() const { return _intervalY; }
  inline real_t getLength() const { return _length; }
  inline real_t getWidth() const { return _width; }
  inline terrain::SSurfacePt3D getBottomLeft() const { return _bottomLeftPt; }
  inline terrain::SSurfacePt3D getTopRight() const { return _topRightPt; }

  int getNumberOfMeshGrids() const;

  std::string getSurName();

 public:
  void setSurName(const std::string &surName);

  void setIntervals(const real_t &intX, const real_t &intY);

  void setSize(const real_t &length, const real_t &width);

  void setCorners(const terrain::SSurfacePt3D &bl, const terrain::SSurfacePt3D &tr);

  void saturationGridsPos(int16_t &row, int16_t &col);

  void setGridValue(const int &row, const int &col, const terrain::GridElement &gridValueIn);

  void setMeshMemoryPtr(terrain::GridElement *gridPtr);

  void releaseMeshMemory();

  void setRows(const int &r);

  void setCols(const int &c);

  void setNumberOfMeshGrids(const int &nSize);

 public:
  // reset surface grids
  void resetSurfaceGrids();

  // get surface grid postion of given location
  void getSurfaceGridByLoc(const terrain::SSurfacePt3D &givenENU, int16_t &row, int16_t &col);

  // get location of surface grid
  void getLocBySurfaceGrid(const int &row, const int &col, terrain::SSurfacePt3D &loc) const;

  // is inside grids
  bool isInsideEnvelope(const terrain::SSurfacePt3D &givenPtENU) const;

  // get filled grid element pointer vector
  void getFilledElePtrVec(terrain::SurfaceGridPtr &filledGridElePtrVec);

  // fill unset grid
  void completeMeshGrids(const terrain::SurfaceGridPtr &filledGridElePtrVec, const real_t &expandLength);

  // grids grow, (x-1,y),(x+1,y), (x,y+1), (x,y-1)
  void gridAutomaticGrow(const terrain::GridElementPtr elePtr, std::vector<terrain::GridElementPtr> &outerElePtrVecOut);

  void smoothGrids(const int &nConv);

  // get z by xy
  void getZInfoByXY(const terrain::SSurfacePt3D &givenENU, terrain::GridElement &gridEle);

  // save grid
  void saveGrids(const std::string &fileName);

  inline terrain::GridElement *getMeshMemoryPtr() { return _surGridsPtr; }

 public:
  // transfer mesh grid memeory pointer to another object
  void transMeshGridTo(CSurfaceGrids &meshB);
};
}  // namespace terrain

#endif
