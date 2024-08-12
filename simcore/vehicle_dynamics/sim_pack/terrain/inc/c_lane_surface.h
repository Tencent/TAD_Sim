#ifndef _CROAD_SURFACE_H_
#define _CROAD_SURFACE_H_

#include <fstream>
#include <iostream>
#include "c_surface_grids.h"
#include "c_terrain_common.h"
#include "c_terrain_math.h"

using namespace std;

namespace terrain {
class CLaneSurface;
class CSurfaceSolver;
typedef std::shared_ptr<CLaneSurface> TRoadSurfacePtr;

class CSurfaceSolver {
 private:
  // square matrix
  int _nMat;
  Eigen::MatrixXd _mat;

 public:
  // create matrix, n X n
  CSurfaceSolver(int nMat);

  // set matrix
  void setMatrix(const Eigen::MatrixXd &matIn);

  // solve equation, zIn: n X 1
  bool solveEquation(const Eigen::MatrixXd &zIn, std::vector<double> &paraOut);
};

class CLaneSurface {
 private:
  // calculate bottom-left and top-right corner of surface
  void calSurfaceCorner(const terrain::SSurfaceBorder &points, terrain::SSurfacePt3D &bl, terrain::SSurfacePt3D &tr);

  // get nearest point in border, only use x and y coordinate
  bool nearestPointXY(const SSurfaceBorder &border, const SSurfacePt3D &pt, size_t &index, real_t &dist);

  // first order surface creation, f(x,y)=axy+bx+cy+d,
  bool firstOrderSurfaceSolver(const SSurfacePointVec &keyPointsIn, std::vector<double> &paraX);
  void firstOrderSurfaceMeshGridFill(const SSurfacePointVec &keyPointsIn, const std::vector<double> &paraX,
                                     terrain::CSurfaceGrids &surGrids);

  // ployfit solver
  bool polyfitSolver(const SSurfacePointVec &keyPointsIn, std::vector<double> &coeff);
  void polyfitMeshGridFill(const SSurfacePointVec &keyPointsIn, const std::vector<double> &paraX,
                           terrain::CSurfaceGrids &surGrids);

  // smooth border
  void smoothBorder(terrain::SSurfaceBorder &bordera);

  // remove close points and use grid coordinate
  void removeClosePtInBorders(const terrain::SSurfaceBorder &border1, const terrain::SSurfaceBorder &border2,
                              const real_t &thresh, terrain::SSurfaceBorder &bordera, terrain::SSurfaceBorder &borderb);

 public:
  CLaneSurface() {}

  // triangle surface creation, f(x,y)a=ax+by+c
  void createTriangleLaneSurface(const terrain::SSurfaceBorders &sBorders, terrain::CSurfaceGrids &surGrids);

  // triangle mesh generation
  void triangleMeshCreate(const SSurfacePointVec &triangle, terrain::CSurfaceGrids &surGrids);

  // ployfit mesh generation
  void polyFitMeshCreate(const std::shared_ptr<SSurfacePointVec> &points, terrain::CSurfaceGrids &surGrids);
  void polyFitMeshCreate(const SSurfacePointVec &points, terrain::CSurfaceGrids &surGrids);

  // create linear surface
  bool createLinearLinkSurface(const terrain::SSurfaceBorder &border, terrain::CSurfaceGrids &surGrids);

  // saveBorders, test function
  void saveSBorders(const terrain::SSurfaceBorders &sBorders);

  // print border for debug
  static void printPoints(const std::string &msg, const SSurfacePointVec &points);
};
}  // namespace terrain

#endif
