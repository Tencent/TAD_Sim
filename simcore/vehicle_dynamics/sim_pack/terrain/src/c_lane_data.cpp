#include "c_lane_data.h"

#include <algorithm>
#include <cmath>

namespace terrain {
CLaneData::CLaneData() { _skip_overpass_check = false; }

void CLaneData::filterBorder(const terrain::SSurfaceBorder& border, const terrain::CSurfaceGrids& surGrids,
                             terrain::SSurfaceBorder& borderOut) {
  borderOut.clear();

  for (auto ptIter = border.begin(); ptIter != border.end(); ++ptIter) {
    auto& pt = (*ptIter);

    if (surGrids.isInsideEnvelope(pt)) {
      borderOut.push_back(pt);
    }
  }
}

void CLaneData::setHDMapHandle(hadmap::txMapHandle* mapHandle) { _hdmapHandle = mapHandle; }
void CLaneData::setHDMapOri(const hadmap::txPoint& hdmapOri) { _hdmapOri = hdmapOri; }
void CLaneData::setEgoLocationENU(const hadmap::txPoint& egoENU) { _egoENU = egoENU; }

bool CLaneData::isAboveOverpassThresh(const SSurfacePt3D& pt) {
  return std::abs(pt.z - _egoENU.z) >= OVERPASS_HEIGHT_THRESH ? true : false;
}
bool CLaneData::isAboveOverpassThresh(const SSurfacePointVec& points, const hadmap::txLaneId& laneID) {
  bool aboveThresh = false;
  double max_absDistance = 5.0;

  if (points.size() == 0) return aboveThresh;

  for (const auto& point : points) {
    double dist = std::abs(point.x - _egoENU.x) + std::abs(point.y - _egoENU.y);
    if (dist < max_absDistance) {
      max_absDistance = dist;
      aboveThresh = std::abs(point.z - _egoENU.z) >= OVERPASS_HEIGHT_THRESH ? true : false;
    }
  }

  return aboveThresh;
}

void CLaneData::setSkipOverpassCheck(const bool& skip_overpass_check) { _skip_overpass_check = skip_overpass_check; }

// convert boundry curve pts
void CLaneData::convertPoints(const hadmap::PointVec& ptVecIn, SSurfacePointVec& ptsOut,
                              const hadmap::txCurve* lcurvePtr, const hadmap::txPoint& hdmapOri) {
  for (auto ptIter = ptVecIn.begin(); ptIter != ptVecIn.end(); ++ptIter) {
    hadmap::Point3d pt3d;

    pt3d.x = ptIter->x;
    pt3d.y = ptIter->y;
    pt3d.z = ptIter->z;

    if (lcurvePtr->getCoordType() == hadmap::COORD_WGS84) {
      coord_trans_api::lonlat2enu(pt3d.x, pt3d.y, pt3d.z, hdmapOri.x, hdmapOri.y, hdmapOri.z);
    }

    ptsOut.push_back(pt3d);
  }
}

void CLaneData::convertPoints(const hadmap::PointVec& ptVecIn, SSurfacePointVec& ptsOut) {
  for (auto ptIter = ptVecIn.begin(); ptIter != ptVecIn.end(); ++ptIter) {
    hadmap::Point3d pt3d;

    pt3d.x = ptIter->x;
    pt3d.y = ptIter->y;
    pt3d.z = ptIter->z;

    ptsOut.push_back(pt3d);
  }
}

void CLaneData::generateLaneSurface(hadmap::txLanePtr lanePtr, terrain::CSurfaceGrids* grid_ptr) {
  terrain::CLaneSurface surface = terrain::CLaneSurface();

  // left and right curve points
  terrain::SSurfaceBorder lborderIn, rborderIn;
  terrain::SSurfaceBorders borders;

  auto& surGrids = *grid_ptr;

  getLaneBoundryGeometory(lanePtr, surGrids, lborderIn, rborderIn);

  // LOG(INFO)<<"vd | number of sampled points in left and right curve: "<<lborderIn.size()<<",
  // "<<lborderIn.size()<<"\n";
  if (lborderIn.size() >= 2 && rborderIn.size() >= 2) {
    borders.push_back(lborderIn);
    borders.push_back(rborderIn);

    // create surface and its mesh
    surface.createTriangleLaneSurface(borders, surGrids);
  } else {
    LOG(ERROR) << "lborderIn.size():" << lborderIn.size() << ", rborderIn.size():" << rborderIn.size() << "\n";
  }
}

void CLaneData::generatePolyfitJunctionSurface(SSurfacePointVec& endPtsVec, terrain::CSurfaceGrids* grid_ptr) {
  // get center point of junction
  hadmap::Point3d center3d;

  // LOG(INFO) << "vd | end points number is "<<linkEndPtsVec.size()<<"\n";

  // auto& surGrids = *grid_ptr;
  auto& linkEndPtsVec = endPtsVec;

  terrain::CLaneSurface surface = terrain::CLaneSurface();
  surface.polyFitMeshCreate(linkEndPtsVec, *grid_ptr);
}

void CLaneData::generatePolyfitJunctionSurface(std::shared_ptr<SSurfacePointVec> link_end_points,
                                               terrain::CSurfaceGrids* grid_ptr) {
  // get center point of junction
  hadmap::Point3d center3d;

  // LOG(INFO) << "vd | end points number is "<<linkEndPtsVec.size()<<"\n";

  // auto& surGrids = *grid_ptr;
  auto& linkEndPtsVec = *(link_end_points.get());

  if (getCenterOfJunction(linkEndPtsVec, center3d)) {
    // sort points in clockwise order
    sortPointsInClockWiseOrder(linkEndPtsVec, center3d);

    // LOG(INFO) << "vd | sorted end points number is " << linkEndPtsVec.size() << "\n";
    // testClockWisePoints(linkEndPtsVec, center3d);

    // create mesh
    linkEndPtsVec.push_back(center3d);

    terrain::CLaneSurface surface = terrain::CLaneSurface();
    surface.polyFitMeshCreate(link_end_points, *grid_ptr);
    /*
    if (linkEndPtsVec.size() < 3) return;

    for (size_t i = 0; i < linkEndPtsVec.size(); ++i) {
        //triangle point
        std::shared_ptr<SSurfacePointVec> triangle=std::make_shared<SSurfacePointVec>();

        size_t next_i = (i + 1) % linkEndPtsVec.size();
        size_t next_next_i = (i + 2) % linkEndPtsVec.size();

        //get triangle point
        triangle->push_back(linkEndPtsVec.at(i));
        triangle->push_back(linkEndPtsVec.at(next_i));
        triangle->push_back(linkEndPtsVec.at(next_next_i));
        triangle->push_back(center3d);

        //create mesh
        terrain::CLaneSurface surface = terrain::CLaneSurface();
        surface.polyFitMeshCreate(triangle, *grid_ptr);
    }
    */
  } else {
    LOG(INFO) << "vd | fail to get center of junction \n";
  }
}

void CLaneData::generateJunctionSurface(SSurfacePointVec& endPtsVec, terrain::CSurfaceGrids* grid_ptr) {
  // get center point of junction
  hadmap::Point3d center3d;

  // LOG(INFO) << "vd | end points number is "<<linkEndPtsVec.size()<<"\n";

  auto& surGrids = *grid_ptr;
  auto& linkEndPtsVec = endPtsVec;

  if (getCenterOfJunction(linkEndPtsVec, center3d)) {
    // sort points in clockwise order
    sortPointsInClockWiseOrder(linkEndPtsVec, center3d);

    // LOG(INFO) << "vd | sorted end points number is " << linkEndPtsVec.size() << "\n";
    // testClockWisePoints(linkEndPtsVec, center3d);

    // generate triangle mesh
    for (size_t i = 0; i < linkEndPtsVec.size(); ++i) {
      // triangle point
      SSurfacePointVec triangle;

      size_t next_i = (i + 1) % linkEndPtsVec.size();

      // get triangle point
      triangle.push_back(linkEndPtsVec.at(i));
      triangle.push_back(linkEndPtsVec.at(next_i));
      triangle.push_back(center3d);

      // create mesh
      terrain::CLaneSurface surface = terrain::CLaneSurface();

      surface.triangleMeshCreate(triangle, surGrids);
    }
  } else {
    LOG(INFO) << "vd | fail to get center of junction \n";
  }
}

void CLaneData::generateJunctionSurface(std::shared_ptr<SSurfacePointVec> link_end_points,
                                        terrain::CSurfaceGrids* grid_ptr) {
  // get center point of junction
  hadmap::Point3d center3d;

  // LOG(INFO) << "vd | end points number is "<<linkEndPtsVec.size()<<"\n";

  auto& surGrids = *grid_ptr;
  auto& linkEndPtsVec = *(link_end_points.get());

  if (getCenterOfJunction(linkEndPtsVec, center3d)) {
    // sort points in clockwise order
    sortPointsInClockWiseOrder(linkEndPtsVec, center3d);

    // LOG(INFO) << "vd | sorted end points number is " << linkEndPtsVec.size() << "\n";
    // testClockWisePoints(linkEndPtsVec, center3d);

    // generate triangle mesh
    for (size_t i = 0; i < linkEndPtsVec.size(); ++i) {
      // triangle point
      SSurfacePointVec triangle;

      size_t next_i = (i + 1) % linkEndPtsVec.size();

      // get triangle point
      triangle.push_back(linkEndPtsVec.at(i));
      triangle.push_back(linkEndPtsVec.at(next_i));
      triangle.push_back(center3d);

      // create mesh
      terrain::CLaneSurface surface = terrain::CLaneSurface();

      surface.triangleMeshCreate(triangle, surGrids);
    }
  } else {
    LOG(INFO) << "vd | fail to get center of junction \n";
  }
}

void CLaneData::generateLaneLinkSurface(const hadmap::txLaneLinkPtr& lanelinkPtr, terrain::CSurfaceGrids& surGrids) {
  terrain::CLaneSurface surface = terrain::CLaneSurface();
  terrain::SSurfaceBorder cRefLine, cRefLineIn;

  terrain::SSurfaceBorders borders;

  const hadmap::txCurve* linecurvePtr = (const hadmap::txCurve*)lanelinkPtr->getGeometry();

  // left and right curve points
  hadmap::PointVec curvePoints;

  // sample center curve
  linecurvePtr->sample(_interval, curvePoints);

  convertPoints(curvePoints, cRefLine, linecurvePtr, _hdmapOri);

  filterBorder(cRefLine, surGrids, cRefLineIn);

  // create surface and its mesh
  if (cRefLineIn.size() >= 2) {
    surface.createLinearLinkSurface(cRefLineIn, surGrids);
  }
}

// get center of one junction
bool CLaneData::getCenterOfJunction(const SSurfacePointVec& endPtsVec, hadmap::Point3d& center) {
  center.x = 0.0;
  center.y = 0.0;
  center.z = 0.0;

  for (auto endPtsIter = endPtsVec.begin(); endPtsIter != endPtsVec.end(); ++endPtsIter) {
    center.x += endPtsIter->x;
    center.y += endPtsIter->y;
    center.z += endPtsIter->z;
  }

  if (endPtsVec.size() > 0) {
    center.x = center.x / endPtsVec.size();
    center.y = center.y / endPtsVec.size();
    center.z = center.z / endPtsVec.size();

    return true;
  }

  return false;
}

// get lane boundry geomtory
void CLaneData::getLaneBoundryGeometory(const hadmap::txLanePtr& lanePtr, const terrain::CSurfaceGrids& surGrids,
                                        SSurfacePointVec& lborderOut, SSurfacePointVec& rborderOut) {
  // boundries
  hadmap::txLaneBoundaryPtr lBoundryPtr, rBoundryPtr;

  // left and right curve points
  hadmap::PointVec leftCurvePoints, rightCurvePoints;
  terrain::SSurfaceBorder lborder, rborder;

  // geometroy
  const hadmap::txCurve *lCurve, *rCurve;

  // LOG(INFO)<<"vd | lane id: "<<lanePtr->getId()<<"\n";

  // get left and right boundry
  hadmap::getBoundary(_hdmapHandle, lanePtr->getLeftBoundaryId(), lBoundryPtr);
  hadmap::getBoundary(_hdmapHandle, lanePtr->getRightBoundaryId(), rBoundryPtr);

  // transfer coord from longlat to enu
  hadmap::txLaneBoundary leftBoundry = lBoundryPtr->transfer2ENU(_hdmapOri);
  hadmap::txLaneBoundary rightBoudry = rBoundryPtr->transfer2ENU(_hdmapOri);

  // get geometroy
  lCurve = lBoundryPtr->getGeometry();
  rCurve = rBoundryPtr->getGeometry();

  // sample left and right boundry curve
  lCurve->sample(_interval, leftCurvePoints);
  rCurve->sample(_interval, rightCurvePoints);

  // convert borders
  convertPoints(leftCurvePoints, lborder);
  convertPoints(rightCurvePoints, rborder);

  // filter borders
  filterBorder(lborder, surGrids, lborderOut);
  filterBorder(rborder, surGrids, rborderOut);

  // filter overpass
  if (lborderOut.size() >= 2 && rborderOut.size() >= 2 && !_skip_overpass_check) {
    /*bool l_border_check = isAboveOverpassThresh(lborderOut.front()) && isAboveOverpassThresh(lborderOut.back());
    bool r_border_check = isAboveOverpassThresh(rborderOut.front()) && isAboveOverpassThresh(rborderOut.back());*/
    const auto& laneID = lanePtr->getTxLaneId();
    bool l_border_check = isAboveOverpassThresh(lborderOut, laneID);
    bool r_border_check = isAboveOverpassThresh(rborderOut, laneID);

    if (l_border_check || r_border_check) {
      lborderOut.clear();
      rborderOut.clear();
      LOG_EVERY_N(ERROR, 100) << "terrain | lane boundry geometory discard in over pass check.\n";
    }
  }
}

void CLaneData::removePeaksFromBorder(terrain::SSurfaceBorder& border, double heightThresh) {
  if (border.size() < constPointSize) return;

  auto iterMax = std::max_element(border.begin(), border.end(),
                                  [](const hadmap::Point3d& pt1, const hadmap::Point3d& pt2) { return pt1.z < pt2.z; });
  border.erase(iterMax);

  auto iterMin = std::min_element(border.begin(), border.end(),
                                  [](const hadmap::Point3d& pt1, const hadmap::Point3d& pt2) { return pt1.z > pt2.z; });
  border.erase(iterMin);
}

// get lane start/end points
bool CLaneData::getLaneStartPoints(const hadmap::txLanePtr& lanePtr, SSurfacePointVec& points,
                                   const terrain::CSurfaceGrids& surGrids) {
  // left and right curve points
  terrain::SSurfaceBorder lborderIn, rborderIn;
  lborderIn.reserve(1000);
  rborderIn.reserve(1000);

  getLaneBoundryGeometory(lanePtr, surGrids, lborderIn, rborderIn);

  // save start points of lane
  for (auto i = 0ul; i < std::min(constPointSize, lborderIn.size()); ++i) {
    points.push_back(lborderIn.at(i));
  }
  removePeaksFromBorder(points);

  for (auto i = 0ul; i < std::min(constPointSize, rborderIn.size()); ++i) {
    points.push_back(rborderIn.at(i));
  }
  removePeaksFromBorder(points);

  return true;
}

bool CLaneData::getLaneEndPoints(const hadmap::txLanePtr& lanePtr, SSurfacePointVec& points,
                                 const terrain::CSurfaceGrids& surGrids) {
  // left and right curve points
  terrain::SSurfaceBorder lborderIn, rborderIn;

  lborderIn.reserve(1000);
  rborderIn.reserve(1000);

  getLaneBoundryGeometory(lanePtr, surGrids, lborderIn, rborderIn);

  // save end points of lane
  size_t zero = 0;
  for (auto i = std::max(lborderIn.size() - constPointSize, zero); i < lborderIn.size(); ++i) {
    points.push_back(lborderIn.at(i));
  }
  removePeaksFromBorder(points);

  for (auto i = std::max(rborderIn.size() - constPointSize, zero); i < rborderIn.size(); ++i) {
    points.push_back(rborderIn.at(i));
  }
  removePeaksFromBorder(points);

  return true;
}

void CLaneData::getLinkEndPointsFromConnectedLane(const hadmap::txLaneLinks& lanelinks,
                                                  const terrain::CSurfaceGrids& surGrids, SSurfacePointVec& endPtsVec) {
  for (auto linkIter = lanelinks.begin(); linkIter != lanelinks.end(); ++linkIter) {
    const hadmap::txLaneLinkPtr& linkPtr = *linkIter;

    // get from and to lane points
    hadmap::txLanePtr fromLanePtr, toLanePtr;
    if (hadmap::getLane(_hdmapHandle, linkPtr->fromTxLaneId(), fromLanePtr) == TX_HADMAP_DATA_OK) {
      getLaneEndPoints(fromLanePtr, endPtsVec, surGrids);
    }

    if (hadmap::getLane(_hdmapHandle, linkPtr->toTxLaneId(), toLanePtr) == TX_HADMAP_DATA_OK) {
      getLaneStartPoints(toLanePtr, endPtsVec, surGrids);
    }
  }
}

void CLaneData::getLinkRoadsEndPoints(const hadmap::txLaneLinks& lanelinks, const terrain::CSurfaceGrids& surGrids,
                                      std::vector<SSurfacePointVec>& endPointsOut) {
  // endpoints that belong to the save "m_fromRoadID" and "m_toRoadID"
  struct RoadEndPoints {
   public:
    hadmap::roadpkid m_fromRoadID, m_toRoadID;
    SSurfacePointVec m_endPoints;

    RoadEndPoints(const hadmap::roadpkid& fromID, const hadmap::roadpkid& toID, const SSurfacePointVec& endPoints) {
      m_fromRoadID = fromID;
      m_toRoadID = toID;
      m_endPoints.assign(endPoints.begin(), endPoints.end());
    }

    bool findByRoadID(const hadmap::roadpkid& fromID, const hadmap::roadpkid& toID) const {
      return (m_fromRoadID == fromID) && (m_toRoadID == toID);
    }
  };

  class RoadEndPointsVector : public std::vector<RoadEndPoints> {
   public:
    RoadEndPointsVector() { reserve(128); }

    void groupEndPointsByRoadID(const hadmap::roadpkid& fromID, const hadmap::roadpkid& toID,
                                const SSurfacePointVec& endPoints) {
      for (auto i = 0; i < size(); ++i) {
        auto& roadEndPoints = at(i);
        if (roadEndPoints.findByRoadID(fromID, toID)) {
          // save endpoints for these points belong to this "roadEndPoints"
          for (const auto& point : endPoints) {
            roadEndPoints.m_endPoints.push_back(point);
          }
          return;
        }
      }
      // new connected road pair
      emplace_back(fromID, toID, endPoints);
    }

    void debugPrint() {
      for (auto i = 0; i < size(); ++i) {
        const auto& roadEndPoints = at(i);
        LOG(INFO) << "from road id:" << roadEndPoints.m_fromRoadID << ", to road id:" << roadEndPoints.m_toRoadID
                  << ", points size:" << roadEndPoints.m_endPoints.size() << "\n";
      }
    }
  };

  // hold endpoints that belong different connected road pair
  RoadEndPointsVector roadEndPointsVector;

  for (auto linkIter = lanelinks.begin(); linkIter != lanelinks.end(); ++linkIter) {
    const hadmap::txLaneLinkPtr& linkPtr = *linkIter;
    SSurfacePointVec endPtsVec;
    endPtsVec.reserve(128);

    // get from road id and to road id
    auto fromRoadID = linkPtr->fromTxLaneId().roadId;
    auto toRoadID = linkPtr->toTxLaneId().roadId;

    // get from and to lane points
    hadmap::txLanePtr fromLanePtr, toLanePtr;

    if (hadmap::getLane(_hdmapHandle, linkPtr->fromTxLaneId(), fromLanePtr) == TX_HADMAP_DATA_OK) {
      getLaneEndPoints(fromLanePtr, endPtsVec, surGrids);
    }

    if (hadmap::getLane(_hdmapHandle, linkPtr->toTxLaneId(), toLanePtr) == TX_HADMAP_DATA_OK) {
      getLaneStartPoints(toLanePtr, endPtsVec, surGrids);
    }

    roadEndPointsVector.groupEndPointsByRoadID(fromRoadID, toRoadID, endPtsVec);
  }

  // roadEndPointsVector.debugPrint();

  for (const auto& roadEndPoints : roadEndPointsVector) {
    if (roadEndPoints.m_endPoints.size() >= 3) {
      endPointsOut.push_back(roadEndPoints.m_endPoints);
    }
  }
}

// if too close
bool CLaneData::isBelowThreshDist(const terrain::SSurfacePointVec& endPtsVec, const real_t& thresh,
                                  const SSurfacePt3D& point) {
  bool tooClose = false;

  for (auto ptIter = endPtsVec.begin(); ptIter != endPtsVec.end(); ++ptIter) {
    const SSurfacePt3D& pt = *ptIter;

    if (CTerrainMath::distSurfacePoint3D(point, pt) <= thresh) {
      tooClose = true;
      break;
    }
  }

  return tooClose;
}

// sort points in clockwise order
void CLaneData::sortPointsInClockWiseOrder(SSurfacePointVec& points, const SSurfacePt3D& center) {
  struct SortPoint {
    hadmap::Point3d point;
    double angle;

    SortPoint(const hadmap::Point3d& pt, double ang) {
      point = pt;
      angle = ang;
    }
  };

  if (points.size() > 2) {
    std::vector<SortPoint> sortPoints;
    for (const auto& point : points) {
      double angle = std::atan2(point.y - center.y, point.x - center.x);
      angle = angle < 0.0 ? angle + 2 * M_PI : angle;
      sortPoints.emplace_back(point, angle);
    }
    std::sort(sortPoints.begin(), sortPoints.end(),
              [](const SortPoint& pt1, const SortPoint& pt2) { return pt1.angle > pt2.angle; });
    SortPoint curPoint = sortPoints.front();
    const double minAngle = 1 / 180.0 * 3.14;

    points.clear();
    points.push_back(curPoint.point);

    for (auto i = 1; i < sortPoints.size(); ++i) {
      double angleDiff = std::cos(curPoint.angle) * std::cos(sortPoints.at(i).angle) +
                         std::sin(curPoint.angle) * std::sin(sortPoints.at(i).angle);
      angleDiff = std::acos(angleDiff);
      if (std::abs(angleDiff) > minAngle) {
        points.push_back(sortPoints.at(i).point);
        curPoint = sortPoints.at(i);
      }
    }
  }

  return;
}

void CLaneData::testClockWisePoints(const SSurfacePointVec& points, const SSurfacePt3D& center) {
  if (points.size() > 0) {
    std::string angleStr = "vd | sorted angle ";

    for (auto i = 0; i < points.size(); ++i) {
      size_t next_i = (i + 1) % points.size();

      const SSurfacePt3D pt = points.at(i);

      real_t angle = std::atan2(pt.y - center.y, pt.x - center.x);
      angle = angle < 0.0 ? angle + 2 * M_PI : angle;
      angleStr += ", ";
      angleStr += std::to_string(angle);
    }
    angleStr += "\n";
    LOG(INFO) << angleStr;
  }
}
}  // namespace terrain
