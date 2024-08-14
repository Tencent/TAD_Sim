/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  SimMapsdk
 * @author   longerding@tencent
 * Modify history:
 ******************************************************************************/
#pragma once
#include <unordered_map>

#include <mutex>
#include "DataParse.h"
#include "OpenDriveStruct.h"
#include "structs/hadmap_header.h"
#include "structs/hadmap_junction.h"
#include "tinyxml2.h"
#include "types/map_types.h"

namespace hadmap {
struct Wid_Data {
  ODLaneSection _sec;
  std::vector<double> _sec_ele;
  std::vector<double> _dis_from_sec;
  std::vector<std::pair<double, double>> _sec_ref_line, _sec_ref_normal;
  std::vector<std::vector<double>> _bdy_width;
  std::vector<std::vector<double>> _lane_width;
  std::vector<LANE_MARK> _bdy_mark;
  std::vector<double> _bdy_lanewidth;
  std::vector<double> _bdy_lanedinterval;
  std::vector<double> _bdy_lanedspace;
  std::vector<double> _bdy_lanedsoffset;
  std::vector<double> _bdy_lanedlen;
};

using AreaPointVec = std::vector<std::tuple<hadmap::txPoint, hadmap::txPoint>>;

class TXSIMMAP_API ODParse : public DataParse {
 public:
  ODParse();

  ~ODParse();

 public:
  // Parse the OpenDRIVE data file
  bool parse(const std::string& dataPath);
  // Parse the OpenDRIVE data file with area points
  bool parseArea(const std::string& dataPath, AreaPointVec ptr);
  // Get the parsed data
  bool getData(txRoads& roads, txLaneLinks& links, txObjects& objs, txJunctions& juncs, txOdHeaderPtr& header,
               txPoint& refPoint, bool doubleRoad);

 private:
  // Calculate the reference line length
  void calcRefLineLen(const std::vector<std::pair<double, double>>& ref_line, std::vector<double>& dis_from_begin);

  // Generate the header pointer
  void generateHeaderPtr(ODHeader odheader);

  // Generate the section reference information
  void generateSecRefInfo(const std::vector<std::pair<double, double>>& ref_line,
                          const std::vector<std::pair<double, double>>& ref_normal, const std::vector<double>& road_ele,
                          const std::vector<double>& dis_from_begin, const double& s, const double& e,
                          std::vector<std::pair<double, double>>& sec_ref_line,
                          std::vector<std::pair<double, double>>& sec_ref_normal, std::vector<double>& sec_ele,
                          std::vector<double>& dis_from_sec);

  // Generate the road pointer without section info
  void generateRoadPtr(const roadpkid& id, const ROAD_TYPE& type, int speed_limit,
                       const std::vector<std::pair<double, double>>& road_geom, const std::vector<double>& road_ele,
                       txRoadPtr& road_ptr);

  // Generate the lane boundary pointer
  void generateBoundaryPtr(const laneboundarypkid& id, const LANE_MARK& mark, double lanewidth, double lanedinterval,
                           double lanedspace, double lanedsoffset, double lanedlen,
                           const std::vector<std::pair<double, double>>& bound_geom,
                           const std::vector<double>& bound_ele, txLaneBoundaryPtr& bound_ptr);

  // Generate the lane pointer
  void generateLanePtr(const roadpkid& r_id, const sectionpkid& s_id, const lanepkid& l_id, const LANE_TYPE& type,
                       const int& speed_limit, double lanewidth, const int& surface, const double& friction,
                       const double& materialOffset, const std::vector<std::pair<double, double>>& lane_geom,
                       const std::vector<double>& lane_ele, const std::vector<double>& lane_width, txLanePtr& lane_ptr);

  // Generate road related links
  void generateRoadLinks(const ODRoadPtr od_road_ptr, txLaneLinks& links);

  // Merge section data
  void mergeSections(const txSections& sections, txSectionPtr& sec_ptr);

  // Parse the object type
  OBJECT_TYPE parseObjectType(const ODRoad::Object& obj);

  // Parse the object geometry type
  void parseObjectGeomType(const ODRoad::Object& obj, OBJECT_GEOMETRY_TYPE& type, OBJECT_STYLE& style);

  // Parse the object
  void parseObject(const ODRoad::Object& obj, txObjectPtr& tx_ptr, double west, double south);

  // Split bilateral data
  void splitBilateral();

  // Check if a curve is within a rectangle area
  bool isInRecVecArea(const txCurve* curveptr, std::vector<std::vector<txPoint>> ptr);

  // Check if sections are within a rectangle area
  bool isInRecVecArea(txSections _sections, std::vector<std::vector<hadmap::txPoint>> ptr);

  // Genrate Road
  bool DoRoad(hadmap::ODRoadPtr ptr);

  bool DoLink(hadmap::ODRoadPtr ptr);

  double GetMeanCurvature(const txRoadPtr _tx_road_ptr, const double startf, const double length);

  double GetMeanSlope(const txRoadPtr _tx_road_ptr, const double startf, const double length);

  txOpenCrgVec GetOpenCrg(std::vector<ODCrg> odcrg);

  ROAD_TYPE GetRoadType(std::string strType);

 private:
  double _north = 0, _south = 0, _east = 0, _west = 0;
  laneboundarypkid _bound_pkid = 1;
  ODHeader od_header;
  ODRoads od_roads;

  std::string target_geo_ref;
  std::string webmercator_geo;
  std::unordered_map<roadpkid, txRoadPtr> tx_roads;
  std::unordered_map<std::string, txLaneLinks> tx_links;
  std::unordered_map<roadpkid, txObjects> tx_objects;
  std::unordered_map<junctionpkid, txJunctionPtr> tx_junctions;
  std::map<hadmap::txLaneId, std::tuple<hadmap::txPoint, hadmap::txPoint>> m_laneidToStartAndEndPoint;
  // Header
  txOdHeaderPtr tx_header;
  // RefPoint
  hadmap::txPoint tx_refpoint;
  laneboundarypkid max_bound_pkid = 1;
  lanelinkpkid max_link_pkid = 1;
  objectgeompkid max_geom_pkid = 1;
  lanelinkpkid _link_pkid = 1;
  std::unordered_map<lanelinkpkid, junctionpkid> linkofjunction;
  std::vector<roadpkid> to_erase_ids;
  std::mutex mutex;
};
}  // namespace hadmap
