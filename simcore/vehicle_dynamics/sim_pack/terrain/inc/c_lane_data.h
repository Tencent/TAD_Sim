#ifndef _CLANE_DATA_H_
#define _CLANE_DATA_H_

#include "c_lane_surface.h"
#include "c_surface_grids.h"
#include "c_terrain_common.h"

namespace terrain {
class CLaneData {
 public:
  CLaneData();

  // generate surface of lane and junction
  void generateLaneSurface(hadmap::txLanePtr lanePtr, terrain::CSurfaceGrids* surGrids);
  void generateJunctionSurface(std::shared_ptr<SSurfacePointVec> endPtsVec, terrain::CSurfaceGrids* surGrids);
  void generateJunctionSurface(SSurfacePointVec& endPtsVec, terrain::CSurfaceGrids* surGrids);
  void generatePolyfitJunctionSurface(std::shared_ptr<SSurfacePointVec> endPtsVec, terrain::CSurfaceGrids* surGrids);
  void generatePolyfitJunctionSurface(SSurfacePointVec& endPtsVec, terrain::CSurfaceGrids* surGrids);
  void generateLaneLinkSurface(const hadmap::txLaneLinkPtr& lanelinkPtr, terrain::CSurfaceGrids& surGrids);

  // get endpoints of lane link, true if there is too close points
  // bool getLaneLinkEndPoints(const hadmap::txLaneLinks &lanelinks, const terrain::CSurfaceGrids &surGrids,
  // SSurfacePointVec &endPtsVec);
  void getLinkEndPointsFromConnectedLane(const hadmap::txLaneLinks& lanelinks, const terrain::CSurfaceGrids& surGrids,
                                         SSurfacePointVec& endPtsVec);

  // ignore endpoints which size is less than 3
  void getLinkRoadsEndPoints(const hadmap::txLaneLinks& lanelinks, const terrain::CSurfaceGrids& surGrids,
                             std::vector<SSurfacePointVec>& endPoints);

  void setHDMapHandle(hadmap::txMapHandle* mapHandle);
  void setHDMapOri(const hadmap::txPoint& hdmapOri);

  // convert boundry/lanelink curve pts
  void convertPoints(const hadmap::PointVec& ptVecIn, SSurfacePointVec& ptsOut, const hadmap::txCurve* lcurvePtr,
                     const hadmap::txPoint& hdmapOri);
  void convertPoints(const hadmap::PointVec& ptVecIn, SSurfacePointVec& ptsOut);

  /*
  void generateLaneSurfaceASync(const hadmap::txLanePtr &lanePtr, terrain::CSurfaceGrids &surGrids);
  void waitUntilTaskFinished();

  void generateLaneLinkSurfaceASync(const hadmap::txLaneLinkPtr &lanelinkPtr, terrain::CSurfaceGrids &surGrids);
  void generateJunctionSurfaceASync(SSurfacePointVec &endPtsVec,terrain::CSurfaceGrids &surGrids);
          void waitUntilJuncTaskFinished();
          void waitUntilJuncLinkTaskFinished();
  */

  void filterBorder(const terrain::SSurfaceBorder& border, const terrain::CSurfaceGrids& surGrids,
                    terrain::SSurfaceBorder& borderOut);
  void setEgoLocationENU(const hadmap::txPoint& egoENU);

  bool isAboveOverpassThresh(const SSurfacePt3D& pt);
  bool isAboveOverpassThresh(const SSurfacePointVec& points, const hadmap::txLaneId& laneID);
  void setSkipOverpassCheck(const bool& skip_overpass_check);

 private:
  // get center of one junction
  bool getCenterOfJunction(const SSurfacePointVec& endPtsVec, hadmap::Point3d& center);

  // if too close
  bool isBelowThreshDist(const terrain::SSurfacePointVec& endPtsVec, const real_t& thresh, const SSurfacePt3D& point);

  // sort points in clockwise order
  void sortPointsInClockWiseOrder(SSurfacePointVec& points, const SSurfacePt3D& center);
  void testClockWisePoints(const SSurfacePointVec& points, const SSurfacePt3D& center);

  // get lane start/end points
  bool getLaneStartPoints(const hadmap::txLanePtr& lanePtr, SSurfacePointVec& points,
                          const terrain::CSurfaceGrids& surGrids);
  bool getLaneEndPoints(const hadmap::txLanePtr& lanePtr, SSurfacePointVec& points,
                        const terrain::CSurfaceGrids& surGrids);
  void removePeaksFromBorder(terrain::SSurfaceBorder& border, double heightThresh = 0.1);

  // get lane boundry geomtory
  void getLaneBoundryGeometory(const hadmap::txLanePtr& lanePtr, const terrain::CSurfaceGrids& surGrids,
                               SSurfacePointVec& lborder, SSurfacePointVec& rborder);

 private:
  hadmap::txPoint _hdmapOri;
  hadmap::txMapHandle* _hdmapHandle;
  const real_t _interval = 0.1;
  hadmap::txPoint _egoENU;
  bool _skip_overpass_check;
  const size_t constPointSize = 10;
  /*
  std::thread _threadGen;
  std::thread _threadJunc;
  std::thread _threadLinkJunc;
  */
};
}  // namespace terrain

#endif
