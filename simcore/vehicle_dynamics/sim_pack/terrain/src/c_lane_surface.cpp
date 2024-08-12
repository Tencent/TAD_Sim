#include "c_lane_surface.h"

namespace terrain {
// create matrix, n X n
CSurfaceSolver::CSurfaceSolver(int nMat) {
  assert(nMat > 0);
  _nMat = nMat;
  _mat = Eigen::MatrixXd::Zero(_nMat, _nMat);
}

// set matrix
void CSurfaceSolver::setMatrix(const Eigen::MatrixXd &matIn) {
  // non-empty square matrix
  assert(matIn.rows() > 0 && matIn.rows() == matIn.cols() && _mat.rows() == matIn.rows());
  _mat = matIn;
}

// solve equation, zIn: n X 1
bool CSurfaceSolver::solveEquation(const Eigen::MatrixXd &zIn, std::vector<double> &paraOut) {
  bool notSingular = false;

  assert(zIn.rows() == _nMat && zIn.cols() == 1);

  // clear output
  paraOut.clear();

  try {
    // inverse of square matrix
    Eigen::MatrixXd matInv = _mat.inverse();

    // solve equation
    Eigen::MatrixXd resMat = matInv * zIn;

    // return solved parameter
    for (auto i = 0; i < _nMat; ++i) {
      paraOut.push_back(resMat(i, 0));
    }

    notSingular = true;

  } catch (exception &e) {
    paraOut.clear();
    for (auto i = 0; i < _nMat; ++i) {
      paraOut.push_back(0.0);
    }

    //_mat.print("matrix inverse error :\n");
    LOG(ERROR) << "vd | fail to calculate inverse of square matrix. " << e.what() << "\n";
  }

  return notSingular;
}

void CLaneSurface::calSurfaceCorner(const terrain::SSurfaceBorder &points, terrain::SSurfacePt3D &blOut,
                                    terrain::SSurfacePt3D &trOut) {
  auto lambdaCalCorners = [](const terrain::SSurfacePt3D &pt, terrain::SSurfacePt3D &bl, terrain::SSurfacePt3D &tr) {
    if (pt.x <= bl.x) {
      bl.x = pt.x;
    }

    if (pt.y <= bl.y) {
      bl.y = pt.y;
    }

    if (pt.x >= tr.x) {
      tr.x = pt.x;
    }

    if (pt.y >= tr.y) {
      tr.y = pt.y;
    }
  };

  for (auto iter = points.begin(); iter != points.end(); ++iter) {
    const terrain::SSurfacePt3D &pt = (*iter);
    lambdaCalCorners(pt, blOut, trOut);
  }
}

// smooth grid with n x n conv
void CLaneSurface::smoothBorder(terrain::SSurfaceBorder &bordera) {
  auto n = bordera.size() - 1;

  for (auto i = 0; i < n; ++i) {
    bordera.at(i).z = (bordera.at(i).z + bordera.at(i + 1).z) / 2.0;
  }
}

// remove close points
void CLaneSurface::removeClosePtInBorders(const terrain::SSurfaceBorder &border1,
                                          const terrain::SSurfaceBorder &border2, const real_t &thresh,
                                          terrain::SSurfaceBorder &bordera, terrain::SSurfaceBorder &borderb) {
  // generate first order suface
  size_t nB1 = border1.size(), nB2 = border2.size();
  size_t nPt = nB1 <= nB2 ? nB1 : nB2;

  for (auto i = 0; i < nPt; ++i) {
    // points
    auto &pt1 = border1.at(i);
    auto &pt2 = border2.at(i);

    auto dist = std::sqrt(std::pow(pt2.x - pt1.x, 2.0) + std::pow(pt2.y - pt1.y, 2.0));

    if (dist > thresh) {
      bordera.push_back(pt1);
      borderb.push_back(pt2);
    }
  }
}

// first order surface creation, f(x,y)=axy+bx+cy+d,
bool CLaneSurface::firstOrderSurfaceSolver(const SSurfacePointVec &keyPointsIn, std::vector<double> &paraX) {
  int nPt = (int)keyPointsIn.size();

  // f(x,y)=ax+by+c, to solve surface
  CSurfaceSolver foss = CSurfaceSolver(nPt);

  //[x y 1.0]
  Eigen::MatrixXd matP = Eigen::MatrixXd(nPt, nPt);

  // z
  Eigen::MatrixXd matZ = Eigen::MatrixXd(nPt, 1);

  // set para matrix
  for (auto i = 0; i < nPt; ++i) {
    const hadmap::Point3d &pt = keyPointsIn.at(i);

    matP(i, 0) = pt.x;
    matP(i, 1) = pt.y;
    matP(i, 2) = 1.0;

    matZ(i, 0) = pt.z;
  }

  // solve equation
  foss.setMatrix(matP);
  return foss.solveEquation(matZ, paraX);
}

// file mesh grid
void CLaneSurface::firstOrderSurfaceMeshGridFill(const SSurfacePointVec &keyPointsIn, const std::vector<double> &paraX,
                                                 terrain::CSurfaceGrids &surGrids) {
  terrain::SSurfacePt3D bl, tr;

  bl.x = INFINITE_POSITIVE;
  bl.y = INFINITE_POSITIVE;
  tr.x = INFINITE_NEGATIVE;
  tr.y = INFINITE_NEGATIVE;

  calSurfaceCorner(keyPointsIn, bl, tr);

  int16_t r0, c0, r1, c1;
  real_t a = 0.0, b = 0.0, c = 0.0;

  a = paraX.at(0);
  b = paraX.at(1);
  c = paraX.at(2);

  surGrids.getSurfaceGridByLoc(bl, r0, c0);
  surGrids.getSurfaceGridByLoc(tr, r1, c1);

  // polygon of control points
  terrain::SGridPolygon polygon;
  terrain::CGridPt2d pPt;
  SSurfacePt3D pt3d;

  for (auto i = 0; i < keyPointsIn.size(); ++i) {
    pPt.x = keyPointsIn[i].x;
    pPt.y = keyPointsIn[i].y;
    polygon.push_back(pPt);
  }

  for (auto i = r0; i <= r1; ++i) {
    for (auto j = c0; j <= c1; ++j) {
      surGrids.getLocBySurfaceGrid(i, j, pt3d);
      pPt.x = pt3d.x;
      pPt.y = pt3d.y;

      // if grid position is inside polygon
      if (terrain::CTerrainMath::isInside(polygon, pPt)) {
        const terrain::GridElementPtr tarGridElePtr = surGrids.getGridElePtr(i, j);

        real_t z = a * pt3d.x + b * pt3d.y + c;

        if (!tarGridElePtr->filled) {
          tarGridElePtr->z = z;
          tarGridElePtr->dzdx = a;
          tarGridElePtr->dzdy = b;
        } else {
          tarGridElePtr->z = (tarGridElePtr->z + z) / 2.0;
        }

        tarGridElePtr->filled = true;
      }
    }
  }
}

// triangle mesh generation
void CLaneSurface::triangleMeshCreate(const SSurfacePointVec &triangle, terrain::CSurfaceGrids &surGrids) {
  assert(triangle.size() == 3);

  std::vector<double> paraXP;

  if (firstOrderSurfaceSolver(triangle, paraXP)) {
    firstOrderSurfaceMeshGridFill(triangle, paraXP, surGrids);
  } else {
    LOG(INFO) << "vd | singluar caught, skip one piece of triangle mesh generation.\n";
  }
}

// ployfit mesh generation
void CLaneSurface::polyFitMeshCreate(const std::shared_ptr<SSurfacePointVec> &points,
                                     terrain::CSurfaceGrids &surGrids) {
  if (points.get() == nullptr || points->size() < 4) {
    return;
  }

  // y=a1*xy+a2*x+a3*y+a4
  std::vector<double> coeff;
  coeff.reserve(4);

  // y=a1*xy+a2*x+a3*y+a4
  auto state = polyfitSolver(*(points.get()), coeff);

  if (!state) {
    return;
  }

  polyfitMeshGridFill(*(points.get()), coeff, surGrids);
}

void CLaneSurface::polyFitMeshCreate(const SSurfacePointVec &points, terrain::CSurfaceGrids &surGrids) {
  if (points.size() < 4) {
    return;
  }

  // y=a1*xy+a2*x+a3*y+a4
  std::vector<double> coeff;
  coeff.reserve(4);

  // y=a1*xy+a2*x+a3*y+a4
  auto state = polyfitSolver(points, coeff);

  if (!state) {
    return;
  }

  polyfitMeshGridFill(points, coeff, surGrids);
}

// ployfit solver
bool CLaneSurface::polyfitSolver(const SSurfacePointVec &points, std::vector<double> &coeff) {
  // y=a1*xy+a2*x+a3*y+a4
  Eigen::MatrixXd A(points.size(), 4);
  Eigen::VectorXd b(points.size());
  Eigen::VectorXd a(points.size());

  // assign matrix and vector
  for (auto row = 0ul; row < points.size(); ++row) {
    const auto &point = points.at(row);
    A(row, 0) = point.x * point.y;
    A(row, 1) = point.x;
    A(row, 2) = point.y;
    A(row, 3) = 1.0;
    b[row] = point.z;
    a[row] = 0.0;
  }

  Eigen::MatrixXd A_Transpose = A.transpose();
  try {
    a = (A_Transpose * A).inverse() * A_Transpose * b;

    for (auto i = 0; i < 4; ++i) {
      coeff.push_back(a[i]);
    }
  } catch (const char *msg) {
    LOG(ERROR) << "fail to polyfit junction surface. " << msg << "\n";
    return false;
  }

  return true;
}
void CLaneSurface::polyfitMeshGridFill(const SSurfacePointVec &keyPointsIn, const std::vector<double> &coeff,
                                       terrain::CSurfaceGrids &surGrids) {
  terrain::SSurfacePt3D bl, tr;

  bl.x = INFINITE_POSITIVE;
  bl.y = INFINITE_POSITIVE;
  tr.x = INFINITE_NEGATIVE;
  tr.y = INFINITE_NEGATIVE;

  calSurfaceCorner(keyPointsIn, bl, tr);

  int16_t r0, c0, r1, c1;
  real_t a = 0.0, b = 0.0, c = 0.0, d = 0.0;

  a = coeff.at(0);
  b = coeff.at(1);
  c = coeff.at(2);
  d = coeff.at(3);

  surGrids.getSurfaceGridByLoc(bl, r0, c0);
  surGrids.getSurfaceGridByLoc(tr, r1, c1);

  // polygon of control points
  terrain::SGridPolygon polygon;
  terrain::CGridPt2d pPt;
  SSurfacePt3D pt3d;

  for (auto i = 0; i < keyPointsIn.size(); ++i) {
    pPt.x = keyPointsIn[i].x;
    pPt.y = keyPointsIn[i].y;
    polygon.push_back(pPt);
  }

  for (auto i = r0; i <= r1; ++i) {
    for (auto j = c0; j <= c1; ++j) {
      surGrids.getLocBySurfaceGrid(i, j, pt3d);
      pPt.x = pt3d.x;
      pPt.y = pt3d.y;

      // if grid position is inside polygon
      // if (terrain::CTerrainMath::isInside(polygon, pPt)) {
      {
        const terrain::GridElementPtr tarGridElePtr = surGrids.getGridElePtr(i, j);

        real_t z = a * pt3d.x * pt3d.y + b * pt3d.x + c * pt3d.y + d;

        if (!tarGridElePtr->filled) {
          tarGridElePtr->z = z;
          tarGridElePtr->dzdx = a * pt3d.y + b;
          tarGridElePtr->dzdy = a * pt3d.x + c;
        } else {
          tarGridElePtr->z = (z + tarGridElePtr->z) / 2.0;
        }

        tarGridElePtr->filled = true;
      }
    }
  }
}

// first order surface creation, f(x,y)=axy+bx+cy+d,
void CLaneSurface::createTriangleLaneSurface(const terrain::SSurfaceBorders &sBorders,
                                             terrain::CSurfaceGrids &surGrids) {
  assert(sBorders.size() == 2);

  // std::chrono::high_resolution_clock::time_point t1=std::chrono::high_resolution_clock::now();
  const terrain::SSurfaceBorder &lborder = sBorders.at(0), &rborder = sBorders.at(1);
  terrain::SSurfaceBorder border1, border2;

  // remove close points in borders
  removeClosePtInBorders(lborder, rborder, 0.2, border1, border2);

  // generate first order suface
  size_t nB1 = border1.size(), nB2 = border2.size();
  size_t nPt = nB1 <= nB2 ? nB1 : nB2;

  // LOG(INFO)<<"vd | number of useful points in border "<<nPt<<"\n";

  if (nPt >= 2) {
    // generate surafce ax+by+c
    for (auto i = 0; i < nPt - 1; ++i) {
      // fill whole mesh with 4 control point of borders
      SSurfacePointVec keyPointsP;

      // order is important
      keyPointsP.push_back(border1.at(i));
      keyPointsP.push_back(border2.at(i));
      keyPointsP.push_back(border2.at(i + 1));

      triangleMeshCreate(keyPointsP, surGrids);

      keyPointsP.clear();

      // order is important
      keyPointsP.push_back(border1.at(i));
      keyPointsP.push_back(border1.at(i + 1));
      keyPointsP.push_back(border2.at(i + 1));

      triangleMeshCreate(keyPointsP, surGrids);
    }
  }
}

// get nearest point in border, only use x and y coordinate
bool CLaneSurface::nearestPointXY(const SSurfaceBorder &border, const SSurfacePt3D &pt, size_t &index, real_t &dist) {
  auto nPt = border.size();

  if (nPt <= 0) {
    return false;
  }

  dist = INFINITE_POSITIVE;

  for (auto i = 0; i < nPt; ++i) {
    auto &pt1 = border.at(i);
    auto tmpDist = std::sqrt(std::pow(pt.x - pt1.x, 2.0) + std::pow(pt.y - pt1.y, 2.0));

    if (tmpDist <= dist) {
      dist = tmpDist;
      index = i;
    }
  }

  return true;
}

// create lane link surface
bool CLaneSurface::createLinearLinkSurface(const terrain::SSurfaceBorder &sBorder, terrain::CSurfaceGrids &surGrids) {
  int16_t r = 0, c = 0;

  // CLaneSurface::printPoints("vd | link points: ",sBorder);

  // get info from lanelink
  for (auto iterPt = sBorder.begin(); iterPt != sBorder.end(); ++iterPt) {
    // get center curve points
    auto &pt = (*iterPt);

    if (surGrids.isInsideEnvelope(pt)) {
      surGrids.getSurfaceGridByLoc(pt, r, c);
      auto gridElePtr = surGrids.getGridElePtr(r, c);

      if (!gridElePtr->filled) {
        gridElePtr->z = pt.z;
        gridElePtr->filled = true;
      } else {
        gridElePtr->z = (pt.z + gridElePtr->z) / 2.0;
      }
    }
  }

  return true;
}

// saveBorders, test function, 2 borders
void CLaneSurface::saveSBorders(const terrain::SSurfaceBorders &sBorders) {
  std::string coordFileL = "/home/hdy/Documents/lane_left.bin";
  std::string coordFileR = "/home/hdy/Documents/lane_right.bin";

  std::ofstream binStream, binStream1;
  binStream.open(coordFileL.c_str(), std::ios::out | std::ios::binary);
  binStream1.open(coordFileR.c_str(), std::ios::out | std::ios::binary);

  const SSurfaceBorder &leftCvPts = sBorders.at(0);
  const SSurfaceBorder &rightCvPts = sBorders.at(1);

  // save points
  for (auto iter = leftCvPts.begin(); iter != leftCvPts.end(); ++iter) {
    const terrain::SSurfacePt3D &pt = (*iter);
    binStream.write((char *)&pt.x, sizeof(double));
    binStream.write((char *)&pt.y, sizeof(double));
    binStream.write((char *)&pt.z, sizeof(double));
  }

  // save points
  for (auto iter = rightCvPts.begin(); iter != rightCvPts.end(); ++iter) {
    const terrain::SSurfacePt3D &pt = (*iter);
    binStream1.write((char *)&pt.x, sizeof(double));
    binStream1.write((char *)&pt.y, sizeof(double));
    binStream1.write((char *)&pt.z, sizeof(double));
  }
  // close stream
  binStream.close();
  binStream1.close();
}

// print border for debug
void CLaneSurface::printPoints(const std::string &msg, const SSurfacePointVec &points) {
  std::string printMsg = msg;
  for (auto i = 0; i < points.size(); ++i) {
    printMsg += ", (" + std::to_string(points[i].x) + "," + std::to_string(points[i].y) + "," +
                std::to_string(points[i].z) + ")";
  }
  LOG(INFO) << printMsg + "\n";
}
}  // namespace terrain
