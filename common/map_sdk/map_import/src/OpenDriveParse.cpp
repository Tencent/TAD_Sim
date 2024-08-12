// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "OpenDriveParse.h"
#include <assert.h>
#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>

#include <set>
#include <thread>
#include <unordered_map>

#include "common/coord_trans.h"
#include "common/log.h"
#include "common/map_util.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_object.h"

#ifdef PROJ_NEW_API
#  include <proj.h>
#else
#  ifndef ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#    define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#  endif
#  include "proj_api.h"
#endif
using namespace tinyxml2;

namespace hadmap {
void shpOut(const std::string& path, const std::vector<std::pair<double, double>>& geom) {
  std::ofstream out;
  out.open(path);
  for (size_t i = 0; i < geom.size(); ++i) {
    double _mx = geom[i].first;
    double _my = geom[i].second;

    coord_trans_api::mercator2lonlat(_mx, _my);
    out << std::setprecision(10) << _mx << " " << _my << " 50.0" << std::endl;
  }
  out.close();
}

ODParse::ODParse()
    : DataParse(DataParse::OPENDRIVE),
      target_geo_ref("+proj=longlat +datum=WGS84 +no_defs"),
      webmercator_geo(
          "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null "
          "+wktext  +no_defs") {}

ODParse::~ODParse() {}

bool ODParse::parse(const std::string& data_path) {
  _bound_pkid = 1;
  _link_pkid = 1;
  tx_roads.clear();
  tx_links.clear();
  tx_junctions.clear();
  tx_objects.clear();
  // txlog::debug("start parse map");
  tinyxml2::XMLDocument _doc;
  _doc.LoadFile(data_path.c_str());

  tinyxml2::XMLElement* _root = _doc.RootElement();
  if (_root == nullptr) return false;
  // txlog::debug("start parse header");
  tinyxml2::XMLElement* _header = _root->FirstChildElement("header");

  od_header.parse(_header);
  od_header.getBBox(_north, _south, _east, _west);

  generateHeaderPtr(od_header);
  txPoint point_ref = txPoint(_west, _south, 0);
  // txlog::debug("start parse junctions");
  // parse junction info
  tinyxml2::XMLElement* junc_elem = _root->FirstChildElement("junction");
  std::unordered_map<junctionpkid, ODJunctionPtr> junc_map;
  while (junc_elem) {
    ODJunctionPtr junc_ptr(new ODJunction);
    junc_ptr->parse(junc_elem);
    junc_map.insert(std::make_pair(junc_ptr->id(), junc_ptr));
    junc_elem = junc_elem->NextSiblingElement("junction");
  }
  tinyxml2::XMLElement* ctl_elem = _root->FirstChildElement("controller");
  std::unordered_map<controllerkid, ODControllerPtr> controller_map;
  while (ctl_elem) {
    ODControllerPtr ctl_ptr(new ODController);
    ctl_ptr->parse(ctl_elem);
    controller_map.insert(std::make_pair(ctl_ptr->id(), ctl_ptr));
    ctl_elem = ctl_elem->NextSiblingElement("controller");
  }
  // txlog::debug("start parse road");
  //  parse road info
  ODRoads roads;
  tinyxml2::XMLElement* _road = _root->FirstChildElement("road");
  while (_road) {
    ODRoadPtr _cur_road_ptr(new ODRoad);
    _cur_road_ptr->parse(_road);
    roads.push_back(_cur_road_ptr);
    _road = _road->NextSiblingElement("road");
  }
#ifdef PROJ_NEW_API

  PJ_CONTEXT* C = proj_context_create();
  PJ* P = proj_create_crs_to_crs(C, od_header.getGeoRef().c_str(), target_geo_ref.c_str(), /*  EPSG:4326 */
                                 NULL);
  if (!P) {
    txlog::printf("proj4 cannot init: %s \t", od_header.getGeoRef().c_str());
    P = proj_create_crs_to_crs(C, webmercator_geo.c_str(), target_geo_ref.c_str(), /*  EPSG:4326 */
                               NULL);
  }
  if (!P) {
    txlog::printf("proj4 pj err\t");
    return true;
  }

#else
  projPJ pj_merc = pj_init_plus(od_header.getGeoRef().c_str());
  if (!pj_merc) {
    txlog::printf("proj4 cannot init: %s \t", od_header.getGeoRef().c_str());
    pj_merc = pj_init_plus(webmercator_geo.c_str());
  }
  projPJ pj_latlong = pj_init_plus(target_geo_ref.c_str());
  if (!pj_merc || !pj_latlong) {
    txlog::printf("proj4 pj err\t");
    return true;
  }
#endif

  auto transform_pts = [&](PointVec& points) {
    if (points.empty()) {
      return;
    }
    int off = 1;
    if (points.size() > 1) {
      off = (&points[1].x) - (&points[0].x);
    }
#ifdef PROJ_NEW_API
    for (auto& p : points) {
      PJ_COORD a = proj_coord(p.x, p.y, p.z, 0);
      PJ_COORD b = proj_trans(P, PJ_FWD, a);
      p.x = b.xyz.x;
      p.y = b.xyz.y;
      p.z = b.xyz.z;
    }
#else
    int code = pj_transform(pj_merc, pj_latlong, points.size(), off, &points[0].x, &points[0].y, &points[0].z);
    // if (code != 0)
    //  txlog::printf("proj4 error: %s \t", pj_strerrno(code));
    for (auto& p : points) {
      p.x *= RAD_TO_DEG;
      p.y *= RAD_TO_DEG;
    }
#endif
  };

  auto distanceBetweenP = [](const txPoint& p0, const txPoint& p1) {
    double x1, y1, z1, x2, y2, z2;
    x1 = p0.x;
    y1 = p0.y;
    z1 = p0.z;
    x2 = p1.x;
    y2 = p1.y;
    z2 = p1.z;
    return std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2) + std::pow(z1 - z2, 2));
  };
  auto get_lane_scale_toEnu = [&](PointVec& points, hadmap::txPoint ref) {
    double r = 1.0;
    PointVec tmp = points;
    double distance_1 = distanceBetweenP(points.at(0), points.at(1));
    PointVec refs;
    refs.push_back(ref);
    transform_pts(points);
    transform_pts(refs);
    for (auto& itPt : points) {
      coord_trans_api::lonlat2enu(itPt.x, itPt.y, itPt.z, refs.at(0).x, refs.at(0).y, refs.at(0).z);
    }
    double distance_2 = distanceBetweenP(points.at(0), points.at(1));
    return distance_2 / distance_1;
  };
  // do road

  auto do_road = [&](int thn, int thid) {
    for (int ri = 0; ri < roads.size(); ri++) {
      if (ri % thn != thid) {
        continue;
      }
      DoRoad(roads.at(ri));
    }
  };
  int thnum = 1;
  std::vector<std::thread> threads;
  for (int i = 0; i < thnum; i++) {
    threads.push_back(std::thread(do_road, thnum, i));
  }
  for (int i = 0; i < thnum; i++) {
    threads[i].join();
  }
  threads.clear();
  linkofjunction.clear();
  to_erase_ids.clear();

  // do link
  auto do_link = [&](int thn, int thid) {
    for (int ri = 0; ri < roads.size(); ri++) {
      if (ri % thn != thid) {
        continue;
      }
      DoLink(roads.at(ri));
    }
  };

  for (int i = 0; i < thnum; i++) {
    threads.push_back(std::thread(do_link, thnum, i));
  }
  for (int i = 0; i < thnum; i++) {
    threads[i].join();
  }
  threads.clear();

  for (auto _cur_road_id : to_erase_ids) {
    tx_roads.erase(_cur_road_id);
  }

  std::unordered_map<objectpkid, txObjectPtr> objects;
  for (auto& objs : tx_objects) {
    for (auto& obj : objs.second) {
      if (tx_roads.find(obj->getRoadId()) != tx_roads.end()) {
        obj->setLaneLinkid(-1);
      } else {
        for (auto itLink : tx_links) {
          for (auto _link : itLink.second) {
            if (_link->getOdrRoadId() == obj->getOdData().roadid) {
              obj->setLaneLinkid(_link->getId());
              break;
            }
          }
        }
      }
      objects.insert(std::make_pair(obj->getId(), obj));
    }
  }
  for (int ri = 0; ri < roads.size(); ri++) {
    ODRoadPtr _cur_road_ptr = roads.at(ri);
    roadpkid _cur_road_id = _cur_road_ptr->id();

    for (const auto& reference : _cur_road_ptr->signalReferences) {
      auto id = std::strtoull(reference.first.c_str(), nullptr, 10);
      if (objects.find(id) != objects.end()) {
        std::vector<txLaneId> rlanes;
        objects[id]->getReliedLaneIds(rlanes);
        sectionpkid sectionid = _cur_road_ptr->getSectionIdx(reference.second.s);
        for (int lid : reference.second.laneids) {
          rlanes.push_back(txLaneId(_cur_road_id, sectionid, lid));
        }
        objects[id]->setReliedLaneIds(rlanes);
      }
    }
  }

  for (const auto& juc : junc_map) {
    txJunctionPtr txjuc(new txJunction(juc.first));
    tx_junctions[juc.first] = txjuc;
    txControllers txctrls;
    auto odjuc = juc.second->getControllers();
    for (auto oj : odjuc) {
      if (controller_map.find(oj) == controller_map.end()) {
        continue;
      }
      auto odctrl = controller_map[oj];
      if (odctrl) {
        txControllerPtr txctl_ptr(new txController(oj));
        txctl_ptr->setSignals(odctrl->getSignsals());
        txctrls.push_back(txctl_ptr);
      }
    }
    txjuc->setControllers(txctrls);
  }
  for (const auto& link : linkofjunction) {
    auto junc = tx_junctions.find(link.second);
    if (junc != tx_junctions.end()) {
      junc->second->insertLaneLink(link.first);
    }
  }

  max_bound_pkid = _bound_pkid + 1;
  max_link_pkid = _link_pkid + 1;

  for (auto& road : tx_roads) {
    txSections& sections = road.second->getSections();
    for (auto& section_ptr : sections) {
      txLanes& lanes = section_ptr->getLanes();
      for (auto& lane_ptr : lanes) {
        PointVec points;
        double lane_width_scale = 1.0;
        if (lane_ptr->getGeometry()) {
          dynamic_cast<const txLineCurve*>(lane_ptr->getGeometry())->getPoints(points);
          if (points.size() > 1) {
            PointVec tmpVec;
            tmpVec.push_back(points.front());
            tmpVec.push_back(points.back());
            hadmap::txPoint tmpRef(_west, _south, 0);
            lane_width_scale = get_lane_scale_toEnu(tmpVec, tmpRef);
          }
          // get enu scale
          for (auto& pt : points) {
            pt.w *= lane_width_scale;
          }
          transform_pts(points);
          lane_ptr->setGeometry(points, COORD_WGS84);
        }
        if (lane_ptr->getDenseGeometry()) {
          dynamic_cast<const txLineCurve*>(lane_ptr->getDenseGeometry())->getPoints(points);
          transform_pts(points);
          lane_ptr->setDenseGeometry(points, COORD_WGS84);
        }
        if (lane_ptr->getLeftBoundary() && lane_ptr->getLeftBoundary()->getGeometry() &&
            lane_ptr->getLeftBoundary()->getGeometry()->getCoordType() != COORD_WGS84) {
          PointVec points;
          dynamic_cast<const txLineCurve*>(lane_ptr->getLeftBoundary()->getGeometry())->getPoints(points);
          transform_pts(points);
          lane_ptr->getLeftBoundary()->setGeometry(points, COORD_WGS84);
        }
        if (lane_ptr->getRightBoundary() && lane_ptr->getRightBoundary()->getGeometry() &&
            lane_ptr->getRightBoundary()->getGeometry()->getCoordType() != COORD_WGS84) {
          PointVec points;
          dynamic_cast<const txLineCurve*>(lane_ptr->getRightBoundary()->getGeometry())->getPoints(points);
          transform_pts(points);
          lane_ptr->getRightBoundary()->setGeometry(points, COORD_WGS84);
        }
      }
    }
    if (road.second->getGeometry()) {
      PointVec points;
      dynamic_cast<const txLineCurve*>(road.second->getGeometry())->getPoints(points);
      transform_pts(points);
      road.second->setGeometry(points, COORD_WGS84);
    }
  }

  for (auto& ils : tx_links) {
    for (auto& link : ils.second) {
      if (link->getGeometry()) {
        PointVec points;
        dynamic_cast<const txLineCurve*>(link->getGeometry())->getPoints(points);
        transform_pts(points);
        link->setGeometry(points, COORD_WGS84);
      }
      for (auto& bd : link->getLeftBoundaries()) {
        if (bd->getGeometry() && bd->getGeometry()->getCoordType() != COORD_WGS84) {
          PointVec points;
          dynamic_cast<const txLineCurve*>(bd->getGeometry())->getPoints(points);
          transform_pts(points);
          bd->setGeometry(points, COORD_WGS84);
        }
      }
      for (auto& bd : link->getRightBoundaries()) {
        if (bd->getGeometry() && bd->getGeometry()->getCoordType() != COORD_WGS84) {
          PointVec points;
          dynamic_cast<const txLineCurve*>(bd->getGeometry())->getPoints(points);
          transform_pts(points);
          bd->setGeometry(points, COORD_WGS84);
        }
      }
    }
  }

  for (auto& objs : tx_objects) {
    for (auto& obj : objs.second) {
      size_t gs = obj->getGeomSize();
      for (size_t i = 0; i < gs; i++) {
        txObjGeomPtr objgeo = obj->getGeom(i);
        if (objgeo->getGeometry()) {
          PointVec points;
          dynamic_cast<const txLineCurve*>(objgeo->getGeometry())->getPoints(points);
          transform_pts(points);
          objgeo->setGeometry(points, COORD_WGS84);
        }
      }
      if (obj->getRepeat()) {
        PointVec points;
        obj->getRepeat()->getPoints(points);
        transform_pts(points);
        obj->getRepeat()->setCoords(points);
        obj->getRepeat()->setCoordType(COORD_WGS84);
      }
      txPoint cp = obj->getPos();
      PointVec points;
      points.push_back(cp);
      transform_pts(points);
      obj->setPos(points.front().x, points.front().y, points.front().z);
    }
  }
#ifdef PROJ_NEW_API
  proj_destroy(P);
  proj_context_destroy(C);
#endif
  return true;
}

bool ODParse::parseArea(const std::string& data_path, AreaPointVec ptr) {
  _bound_pkid = 1;
  _link_pkid = 1;
  tx_roads.clear();
  tx_links.clear();
  tx_junctions.clear();
  tx_objects.clear();

  tinyxml2::XMLDocument _doc;
  _doc.LoadFile(data_path.c_str());

  tinyxml2::XMLElement* _root = _doc.RootElement();
  if (_root == nullptr) return false;
  tinyxml2::XMLElement* _header = _root->FirstChildElement("header");

  od_header.parse(_header);
  od_header.getBBox(_north, _south, _east, _west);

  // parse junction info
  tinyxml2::XMLElement* junc_elem = _root->FirstChildElement("junction");
  std::unordered_map<junctionpkid, ODJunctionPtr> junc_map;
  while (junc_elem) {
    ODJunctionPtr junc_ptr(new ODJunction);
    junc_ptr->parse(junc_elem);
    junc_map.insert(std::make_pair(junc_ptr->id(), junc_ptr));
    junc_elem = junc_elem->NextSiblingElement("junction");
  }

  // parse controller
  tinyxml2::XMLElement* ctl_elem = _root->FirstChildElement("controller");
  std::unordered_map<controllerkid, ODControllerPtr> controller_map;
  while (ctl_elem) {
    ODControllerPtr ctl_ptr(new ODController);
    ctl_ptr->parse(ctl_elem);
    controller_map.insert(std::make_pair(ctl_ptr->id(), ctl_ptr));
    ctl_elem = ctl_elem->NextSiblingElement("controller");
  }
#ifdef PROJ_NEW_API
  PJ_CONTEXT* C = proj_context_create();
  PJ* lon2mer = proj_create_crs_to_crs(C, target_geo_ref.c_str(), /*  EPSG:4326 */
                                       od_header.getGeoRef().c_str(), NULL);
  if (!lon2mer) {
    txlog::printf("proj4 cannot init: %s \t", od_header.getGeoRef().c_str());
    lon2mer = proj_create_crs_to_crs(C, target_geo_ref.c_str(), /*  EPSG:4326 */
                                     webmercator_geo.c_str(), NULL);
  }
  if (!lon2mer) {
    txlog::printf("proj4 pj err\t");
    return true;
  }
#else
  projPJ pj_merc1 = pj_init_plus(od_header.getGeoRef().c_str());
  if (!pj_merc1) {
    txlog::printf("proj4 cannot init: %s \t", od_header.getGeoRef().c_str());
    pj_merc1 = pj_init_plus(webmercator_geo.c_str());
  }
  projPJ pj_latlong1 = pj_init_plus(target_geo_ref.c_str());
  if (!pj_merc1 || !pj_latlong1) {
    txlog::printf("proj4 pj err\t");
    return true;
  }
#endif
  // transform ptr 2 enu
  std::vector<std::vector<txPoint>> tmpRecVec;
  for (auto& iter : ptr) {
    std::vector<txPoint> pointsa;
    std::vector<txPoint> points;
    points.push_back(txPoint(std::get<0>(iter).x, std::get<0>(iter).y, 0.0));
    points.push_back(txPoint(std::get<0>(iter).x, std::get<1>(iter).y, 0.0));
    points.push_back(txPoint(std::get<1>(iter).x, std::get<1>(iter).y, 0.0));
    points.push_back(txPoint(std::get<1>(iter).x, std::get<0>(iter).y, 0.0));
    for (auto& it : points) {
      double lon = it.x;
      double lat = it.y;
#ifdef PROJ_NEW_API
      PJ_COORD a = proj_coord(lon, lat, 0, 0);
      PJ_COORD b = proj_trans(lon2mer, PJ_FWD, a);
      pointsa.push_back(txPoint(b.xy.x, b.xy.y, 0));
#else
      lon *= DEG_TO_RAD;
      lat *= DEG_TO_RAD;
      pj_transform(pj_latlong1, pj_merc1, 1, 1, &lon, &lat, 0);
      pointsa.push_back(txPoint(lon, lat, 0));
#endif
    }
    tmpRecVec.push_back(pointsa);
  }
#ifdef PROJ_NEW_API
  proj_destroy(lon2mer);
#endif

  // parse road info
  ODRoads roads;
  ODRoads roadstmp;
  tinyxml2::XMLElement* _road = _root->FirstChildElement("road");
  while (_road) {
    ODRoadPtr _cur_road_ptr(new ODRoad);
    bool ret = _cur_road_ptr->parse(_road, _west, _south, tmpRecVec);
    if (ret) {
      roads.push_back(_cur_road_ptr);
    }
    _road = _road->NextSiblingElement("road");
  }

  // parse road
  std::mutex mutex;
  auto do_road = [&](int thn, int thid) {
    for (int ri = 0; ri < roads.size(); ri++) {
      if (ri % thn != thid) {
        continue;
      }
      DoRoad(roads.at(ri));
    }
  };
  int thnum = 12;
  std::vector<std::thread> threads;
  for (int i = 0; i < thnum; i++) {
    threads.push_back(std::thread(do_road, thnum, i));
  }
  for (int i = 0; i < thnum; i++) {
    threads[i].join();
  }
  threads.clear();
  std::vector<roadpkid> otherIds;
  for (int i = 0; i < roadstmp.size(); i++) {
    ODRoadPtr _cur_road_ptr = roadstmp.at(i);
    if (_cur_road_ptr->junctionId() > 0) {
      roadpkid roadIdpre = _cur_road_ptr->preRoadId();
      if (tx_roads.find(roadIdpre) == tx_roads.end() &&
          std::find(otherIds.begin(), otherIds.end(), roadIdpre) == otherIds.end()) {
        otherIds.push_back(roadIdpre);
      }
      roadpkid roadIdsucc = _cur_road_ptr->succRoadId();
      if (tx_roads.find(roadIdsucc) == tx_roads.end() &&
          std::find(otherIds.begin(), otherIds.end(), roadIdsucc) == otherIds.end()) {
        otherIds.push_back(roadIdsucc);
      }
    }
  }
  _road = _root->FirstChildElement("road");

  ODRoads otherroads;
  while (_road) {
    ODRoadPtr _cur_road_ptr(new ODRoad);
    bool ret = _cur_road_ptr->parse(_road);
    roadpkid roadId = _cur_road_ptr->id();
    if (std::find(otherIds.begin(), otherIds.end(), roadId) != otherIds.end()) {
      otherroads.push_back(_cur_road_ptr);
    }
    // _cur_road_ptr->roadRef
    _road = _road->NextSiblingElement("road");
  }
  threads.clear();
  auto do_road_other = [&](int thn, int thid) {
    for (int ri = 0; ri < otherroads.size(); ri++) {
      if (ri % thn != thid) {
        continue;
      }
      ODRoadPtr _cur_road_ptr = otherroads.at(ri);
      DoRoad(_cur_road_ptr);
    }
  };

  for (int i = 0; i < thnum; i++) {
    threads.push_back(std::thread(do_road_other, thnum, i));
  }
  for (int i = 0; i < thnum; i++) {
    threads[i].join();
  }
  threads.clear();
  roads = roadstmp;
  // adjust the pre and suc
  ODRoads tmps;
  for (int i = 0; i < roads.size(); i++) {
    if (tx_roads.find(roads.at(i)->id()) != tx_roads.end()) {
      if (roads.at(i)->junctionId() == 0) {
        int preId = roads.at(i)->preRoadId();
        int succId = roads.at(i)->succRoadId();
        if (tx_roads.find(preId) == tx_roads.end()) {
          roads.at(i)->clearPreRoadI();
          // tx_roads.find(preId)->second->clearPrev();
        }
        if (tx_roads.find(succId) == tx_roads.end()) {
          roads.at(i)->clearSuccRoadI();
          // tx_roads.find(preId)->second->clearNext();
        }
      } else {
      }
      tmps.push_back(roads.at(i));
    }
  }
  //
  linkofjunction.clear();
  to_erase_ids.clear();
  auto do_link = [&](int thn, int thid) {
    for (int ri = 0; ri < roads.size(); ri++) {
      if (ri % thn != thid) {
        continue;
      }
      ODRoadPtr _cur_road_ptr = roads.at(ri);
      DoLink(_cur_road_ptr);
    }
  };
  for (int i = 0; i < thnum; i++) {
    threads.push_back(std::thread(do_link, thnum, i));
  }
  for (int i = 0; i < thnum; i++) {
    threads[i].join();
  }
  threads.clear();

  for (auto _cur_road_id : to_erase_ids) {
    tx_roads.erase(_cur_road_id);
  }

  std::unordered_map<objectpkid, txObjectPtr> objects;
  for (auto& objs : tx_objects) {
    for (auto& obj : objs.second) {
      objects.insert(std::make_pair(obj->getId(), obj));
    }
  }
  for (int ri = 0; ri < roads.size(); ri++) {
    ODRoadPtr _cur_road_ptr = roads.at(ri);
    roadpkid _cur_road_id = _cur_road_ptr->id();

    for (const auto& reference : _cur_road_ptr->signalReferences) {
      auto id = std::strtoull(reference.first.c_str(), nullptr, 10);
      if (objects.find(id) != objects.end()) {
        std::vector<txLaneId> rlanes;
        objects[id]->getReliedLaneIds(rlanes);
        sectionpkid sectionid = _cur_road_ptr->getSectionIdx(reference.second.s);
        for (int lid : reference.second.laneids) {
          rlanes.push_back(txLaneId(_cur_road_id, sectionid, lid));
        }
        objects[id]->setReliedLaneIds(rlanes);
      }
    }
  }

  for (const auto& juc : junc_map) {
    txJunctionPtr txjuc(new txJunction(juc.first));
    tx_junctions[juc.first] = txjuc;
    txControllers txctrls;
    auto odjuc = juc.second->getControllers();
    for (auto oj : odjuc) {
      auto odctrl = controller_map[oj];
      txControllerPtr txctl_ptr(new txController(oj));
      txctl_ptr->setSignals(odctrl->getSignsals());
      txctrls.push_back(txctl_ptr);
    }
    txjuc->setControllers(txctrls);
  }
  for (const auto& link : linkofjunction) {
    auto junc = tx_junctions.find(link.second);
    if (junc != tx_junctions.end()) {
      junc->second->insertLaneLink(link.first);
    }
  }

  max_bound_pkid = _bound_pkid + 1;
  max_link_pkid = _link_pkid + 1;

#ifdef PROJ_NEW_API

  PJ* P = proj_create_crs_to_crs(C, od_header.getGeoRef().c_str(), target_geo_ref.c_str(), /*  EPSG:4326 */
                                 NULL);
  if (!P) {
    txlog::printf("proj4 cannot init: %s \t", od_header.getGeoRef().c_str());
    P = proj_create_crs_to_crs(C, webmercator_geo.c_str(), target_geo_ref.c_str(), /*  EPSG:4326 */
                               NULL);
  }
  if (!P) {
    txlog::printf("proj4 pj err\t");
    return true;
  }
#else
  projPJ pj_merc = pj_init_plus(od_header.getGeoRef().c_str());
  if (!pj_merc) {
    txlog::printf("proj4 cannot init: %s \t", od_header.getGeoRef().c_str());
    pj_merc = pj_init_plus(webmercator_geo.c_str());
  }
  projPJ pj_latlong = pj_init_plus(target_geo_ref.c_str());
  if (!pj_merc || !pj_latlong) {
    txlog::printf("proj4 pj err\t");
    return true;
  }

#endif

  auto transform_pts = [&](PointVec& points) {
    if (points.empty()) {
      return;
    }
    int off = 1;
    if (points.size() > 1) {
      off = (&points[1].x) - (&points[0].x);
    }
#ifdef PROJ_NEW_API
    for (auto& p : points) {
      PJ_COORD a = proj_coord(p.x, p.y, p.z, 0);
      PJ_COORD b = proj_trans(P, PJ_FWD, a);
      p.x = b.xyz.x;
      p.y = b.xyz.y;
      p.z = b.xyz.z;
    }
#else
    int code = pj_transform(pj_merc, pj_latlong, points.size(), off, &points[0].x, &points[0].y, &points[0].z);
    if (code != 0) txlog::printf("proj4 error: %s \t", pj_strerrno(code));
    for (auto& p : points) {
      p.x *= RAD_TO_DEG;
      p.y *= RAD_TO_DEG;
    }

#endif
  };

  for (auto& road : tx_roads) {
    txSections& sections = road.second->getSections();

    for (auto& section_ptr : sections) {
      txLanes& lanes = section_ptr->getLanes();
      for (auto& lane_ptr : lanes) {
        PointVec points;
        if (lane_ptr->getGeometry()) {
          dynamic_cast<const txLineCurve*>(lane_ptr->getGeometry())->getPoints(points);
          transform_pts(points);
          lane_ptr->setGeometry(points, COORD_WGS84);
        }
        if (lane_ptr->getDenseGeometry()) {
          dynamic_cast<const txLineCurve*>(lane_ptr->getDenseGeometry())->getPoints(points);
          transform_pts(points);
          lane_ptr->setDenseGeometry(points, COORD_WGS84);
        }
        if (lane_ptr->getLeftBoundary() && lane_ptr->getLeftBoundary()->getGeometry() &&
            lane_ptr->getLeftBoundary()->getGeometry()->getCoordType() != COORD_WGS84) {
          PointVec points;
          dynamic_cast<const txLineCurve*>(lane_ptr->getLeftBoundary()->getGeometry())->getPoints(points);
          transform_pts(points);
          lane_ptr->getLeftBoundary()->setGeometry(points, COORD_WGS84);
        }
        if (lane_ptr->getRightBoundary() && lane_ptr->getRightBoundary()->getGeometry() &&
            lane_ptr->getRightBoundary()->getGeometry()->getCoordType() != COORD_WGS84) {
          PointVec points;
          dynamic_cast<const txLineCurve*>(lane_ptr->getRightBoundary()->getGeometry())->getPoints(points);
          transform_pts(points);
          lane_ptr->getRightBoundary()->setGeometry(points, COORD_WGS84);
        }
      }
    }
    if (road.second->getGeometry()) {
      PointVec points;
      dynamic_cast<const txLineCurve*>(road.second->getGeometry())->getPoints(points);
      transform_pts(points);
      road.second->setGeometry(points, COORD_WGS84);
    }
  }

  for (auto& ils : tx_links) {
    for (auto& link : ils.second) {
      if (link->getGeometry()) {
        PointVec points;
        dynamic_cast<const txLineCurve*>(link->getGeometry())->getPoints(points);
        transform_pts(points);
        link->setGeometry(points, COORD_WGS84);
      }
      for (auto& bd : link->getLeftBoundaries()) {
        if (bd->getGeometry() && bd->getGeometry()->getCoordType() != COORD_WGS84) {
          PointVec points;
          dynamic_cast<const txLineCurve*>(bd->getGeometry())->getPoints(points);
          transform_pts(points);
          bd->setGeometry(points, COORD_WGS84);
        }
      }
      for (auto& bd : link->getRightBoundaries()) {
        if (bd->getGeometry() && bd->getGeometry()->getCoordType() != COORD_WGS84) {
          PointVec points;
          dynamic_cast<const txLineCurve*>(bd->getGeometry())->getPoints(points);
          transform_pts(points);
          bd->setGeometry(points, COORD_WGS84);
        }
      }
    }
  }

  for (auto& objs : tx_objects) {
    for (auto& obj : objs.second) {
      size_t gs = obj->getGeomSize();
      for (size_t i = 0; i < gs; i++) {
        txObjGeomPtr objgeo = obj->getGeom(i);
        if (objgeo->getGeometry()) {
          PointVec points;
          dynamic_cast<const txLineCurve*>(objgeo->getGeometry())->getPoints(points);
          transform_pts(points);
          objgeo->setGeometry(points, COORD_WGS84);
        }
      }
      if (obj->getRepeat()) {
        PointVec points;
        obj->getRepeat()->getPoints(points);
        transform_pts(points);
        obj->getRepeat()->setCoords(points);
        obj->getRepeat()->setCoordType(COORD_WGS84);
      }
      txPoint cp = obj->getPos();
      PointVec points;
      points.push_back(cp);
      transform_pts(points);
      obj->setPos(points.front().x, points.front().y, points.front().z);
    }
  }
#ifdef PROJ_NEW_API
  proj_destroy(lon2mer);
#endif

#ifdef PROJ_NEW_API
  proj_context_destroy(C);
#endif
  return true;
}

void ODParse::mergeSections(const txSections& sections, txSectionPtr& sec_ptr) {
  sec_ptr = txSectionPtr(new txSection(*sections[0]));
  if (sections.size() == 1) {
    return;
  } else {
    txLanes& lanes = sec_ptr->getLanes();
    for (size_t i = 1; i < sections.size(); ++i) {
      for (auto& lane_ptr : lanes) {
        if (!lane_ptr->getGeometry()) continue;
        txPoint end = lane_ptr->getGeometry()->getEnd();
        txLanes& cur_lanes = sections[i]->getLanes();
        for (auto& cur_lane_ptr : cur_lanes) {
          if (!cur_lane_ptr->getGeometry()) continue;
          txPoint start = cur_lane_ptr->getGeometry()->getStart();
          if (fabs(start.x - end.x) < 0.1 && fabs(start.y - end.y) < 0.1) {
            PointVec ori_pts, cur_pts;
            dynamic_cast<const txLineCurve*>(lane_ptr->getGeometry())->getPoints(ori_pts);
            dynamic_cast<const txLineCurve*>(cur_lane_ptr->getGeometry())->getPoints(cur_pts);

            ori_pts.insert(ori_pts.end(), cur_pts.begin() + 1, cur_pts.end());
            lane_ptr->setGeometry(ori_pts, cur_lane_ptr->getGeometry()->getCoordType());
            break;
          }
        }
      }
    }
  }
}

OBJECT_TYPE ODParse::parseObjectType(const ODRoad::Object& obj) {
  if (obj.type == "sensor") {
    return (OBJECT_TYPE_V2X_Sensor);
  } else if (obj.st == ODRoad::Object::SIGNAL) {
    return (OBJECT_TYPE_TrafficSign);
  } else if (obj.st == ODRoad::Object::PARKING) {
    return (OBJECT_TYPE_ParkingSpace);
  } else if (obj.st == ODRoad::Object::SIGNAL_DYNAMIC) {
    return (OBJECT_TYPE_TrafficLights);
  } else if (obj.type == "parkingSpace") {
    return (OBJECT_TYPE_ParkingSpace);
  } else if (obj.type == "TrafficLights" || obj.type == "trafficLight") {
    return (OBJECT_TYPE_TrafficLights);
  } else if (obj.type == "roadmark") {
    if (obj.name.find("Arrow") != std::string::npos) {
      return (OBJECT_TYPE_Arrow);
    } else {
      return OBJECT_TYPE_RoadSign;
    }
  } else if (obj.type == "Curb" || obj.type == "curb") {
    return (OBJECT_TYPE_Curb);
  } else if (obj.type == "railling") {
    return (OBJECT_TYPE_TrafficBarrier);
  } else if (obj.type == "arrow") {
    return (OBJECT_TYPE_Arrow);
  } else if (obj.type == "LeftArrow" || obj.type == "RightArrow" || obj.type == "StraightArrow" ||
             obj.type == "StraightLeftArrow" || obj.type == "StraightRightArrow") {
    return (OBJECT_TYPE_Arrow);
  } else if (obj.type == "StraightLeftArrow") {
    return (OBJECT_TYPE_Arrow);
  } else if (obj.type == "StraightRightArrow") {
    return (OBJECT_TYPE_Arrow);
  } else if (obj.type == "StopLine") {
    return (OBJECT_TYPE_Stop);
  } else if (obj.type == "crosswalk" || obj.type == "ZebraCrossing") {
    return (OBJECT_TYPE_CrossWalk);
  } else if (obj.type == "Poles" || obj.type == "pole") {
    return (OBJECT_TYPE_Pole);
  } else if (obj.type == "streetlamp" || obj.type == "streetLamp" || obj.type == "RoadSignPole" ||
             obj.type == "gantry") {  // jingzong
    return (OBJECT_TYPE_Pole);
  } else if (obj.type == "TrafficSign" || obj.type == "trafficSign" || obj.type == "SpeedBump" ||
             obj.type == "PedsstrianWarning" || obj.type == "HoriSlowMarking") {
    return (OBJECT_TYPE_TrafficSign);
  } else if (obj.type == "Toll") {
    return (OBJECT_TYPE_TollBooth);
  } else if (obj.type == "Belt") {
    return (OBJECT_TYPE_TrafficBarrier);
  } else if (obj.type == "Text") {
    return (OBJECT_TYPE_Text);
  } else if (obj.type == "ConcreteSlab") {
    return (OBJECT_TYPE_Building);
  } else if (obj.type == "parking") {
    return (OBJECT_TYPE_RR_Parking);
  } else if (obj.type == "2" && obj.name == "J_014") {
    return (OBJECT_TYPE_ChargingPile);
  }
  return OBJECT_TYPE_None;
}

void ODParse::parseObjectGeomType(const ODRoad::Object& obj, OBJECT_GEOMETRY_TYPE& type, OBJECT_STYLE& style) {
  type = OBJECT_GEOMETRY_TYPE_None;
  style = OBJECT_STYLE_None;
  if (obj.type == "parkingSpace") {
    type = OBJECT_GEOMETRY_TYPE_Polygon;
    style = OBJECT_STYLE_Polyline;
  } else if (obj.type == "LeftArrow") {
    type = (OBJECT_GEOMETRY_TYPE_Polygon);
    style = ((OBJECT_STYLE)LANE_ARROW_Left);
  } else if (obj.type == "RightArrow") {
    type = (OBJECT_GEOMETRY_TYPE_Polygon);
    style = ((OBJECT_STYLE)LANE_ARROW_Right);
  } else if (obj.type == "StraightArrow") {
    type = (OBJECT_GEOMETRY_TYPE_Polygon);
    style = ((OBJECT_STYLE)LANE_ARROW_Straight);
  } else if (obj.type == "StraightLeftArrow") {
    type = (OBJECT_GEOMETRY_TYPE_Polygon);
    style = ((OBJECT_STYLE)LANE_ARROW_StraightLeft);
  } else if (obj.type == "StraightRightArrow") {
    type = (OBJECT_GEOMETRY_TYPE_Polygon);
    style = ((OBJECT_STYLE)LANE_ARROW_StraightRight);
  } else if (obj.type == "roadmark" || obj.type == "arrow") {
    type = (OBJECT_GEOMETRY_TYPE_Polygon);
    if (obj.name == "ArrowForwardLeft") {
      style = ((OBJECT_STYLE)LANE_ARROW_StraightLeft);
    } else if (obj.name == "ArrowForwardRight") {
      style = ((OBJECT_STYLE)LANE_ARROW_StraightRight);
    } else if (obj.name == "ArrowForward") {
      style = ((OBJECT_STYLE)LANE_ARROW_Straight);
    } else if (obj.name == "ArrowLeft") {
      style = ((OBJECT_STYLE)LANE_ARROW_Left);
    } else if (obj.name == "ArrowRight") {
      style = ((OBJECT_STYLE)LANE_ARROW_Right);
    }
  } else if (obj.type == "StopLine") {
    type = (OBJECT_GEOMETRY_TYPE_Polyline);
  } else if (obj.type == "Poles") {
    type = (OBJECT_GEOMETRY_TYPE_Polygon);
    style = (OBJECT_STYLE_Circle);
  } else if (obj.type == "2" && obj.name == "J_014") {
    type = (OBJECT_GEOMETRY_TYPE_Polygon);
    style = (OBJECT_STYLE_Polygon);
  } else if (obj.name == "Intersection_Guide_Line") {
    style = OBJECT_STYLE_Polyline;
  }
}

void ODParse::parseObject(const ODRoad::Object& obj, txObjectPtr& tx_ptr, double west, double south) {
  auto obtype = parseObjectType(obj);
  if (obtype == OBJECT_TYPE_None) {
    // return;
  }
  if (obj.st == ODRoad::Object::PARKING) {
    tx_ptr.reset(new txParkingSpace);
    dynamic_cast<txParkingSpace*>(tx_ptr.get())->getAccess() = obj.access;
    dynamic_cast<txParkingSpace*>(tx_ptr.get())->getRestrictions() = obj.restrictions;
    for (const auto& mark : obj.parkingmark) {
      ParkingMarking* pmark = nullptr;
      if (mark.side == "front") {
        pmark = &(dynamic_cast<txParkingSpace*>(tx_ptr.get())->getFrontMarking());
      } else if (mark.side == "rear") {
        pmark = &(dynamic_cast<txParkingSpace*>(tx_ptr.get())->getRearMarking());
      } else if (mark.side == "left") {
        pmark = &(dynamic_cast<txParkingSpace*>(tx_ptr.get())->getLeftMarking());
      } else if (mark.side == "right") {
        pmark = &(dynamic_cast<txParkingSpace*>(tx_ptr.get())->getRightMarking());
      }
      if (pmark) {
        pmark->valid = true;
        pmark->type = mark.type;
        pmark->width = mark.width;
        if (mark.color == "standard" || mark.color == "white") {
          pmark->color = OBJECT_COLOR_White;
        } else if (mark.color == "blue") {
          pmark->color = OBJECT_COLOR_Blue;
        } else if (mark.color == "green") {
          pmark->color = OBJECT_COLOR_Green;
        } else if (mark.color == "red") {
          pmark->color = OBJECT_COLOR_Red;
        } else if (mark.color == "yellow") {
          pmark->color = OBJECT_COLOR_Yellow;
        }
      }
    }
  } else {
    tx_ptr.reset(new txObject);
  }
  //
  // tx_ptr->setObjectType(obtype);
  tx_ptr->setId(std::strtoull(obj.id.c_str(), nullptr, 10));
  tx_ptr->setName(obj.name);
  tx_ptr->setLWH(obj.len, obj.wid, obj.hei);
  tx_ptr->setRPY(obj.roll, obj.pitch, obj.yaw);
  tx_ptr->setRawTypeString(obj.type, obj.subtype);
  tx_ptr->setObjectType(obj.name, obj.type, obj.subtype, obj.st);
  tx_od_object_t od_data = tx_ptr->getOdData();
  od_data.length = obj.len;
  od_data.width = obj.wid;
  od_data.height = obj.hei;
  od_data.roll = obj.roll;
  od_data.pitch = obj.pitch;
  od_data.hdg = obj.hdg;
  od_data.zoffset = obj.zoffset;
  od_data.s = obj.s;
  od_data.t = obj.t;
  od_data.orientation = 1;
  od_data.markWidth = obj.markwidth;
  std::string repetadata = obj.repetaData;
  strcpy(od_data.repeatdata, repetadata.c_str());
  txPoint txp;
  txp.x = obj.x + west;
  txp.y = obj.y + south;
  txp.z = obj.z;
  tx_ptr->setST(obj.s, obj.t);
  tx_ptr->setTxOdData(od_data);
  tx_ptr->setUserData(obj.userData);
  tx_ptr->setPos(txp.x, txp.y, txp.z);
  tx_ptr->transform2OdData();

  // //bool forceGeo = false;
  // if (obj.name == "Road_Guide_Lane_Line"){
  //  OBJECT_GEOMETRY_TYPE type(OBJECT_GEOMETRY_TYPE_Polyline);
  //  OBJECT_STYLE type(OBJECT_STYLE_Polyline);
  // }
  // if (!forceGeo && !obj.boundary.empty() && (obj.geom.empty() || obj.geom.size() == 1 &&
  // obj.geom.front().second.size() < 2))//obtype == OBJECT_TYPE_RR_Parking
  // {
  //  PointVec curve;
  //  curve.reserve(obj.boundary.size());
  //  for (const auto& p : obj.boundary)
  //  {
  //    txPoint txp;
  //    txp.x = p.x + west;
  //    txp.y = p.y + south;
  //    txp.z = p.z;
  //    if (pj_merc)
  //    {
  //      int code = pj_transform(pj_merc, pj_latlong, 1, 1, &txp.x, &txp.y, &txp.z);
  //      if (code != 0){
  //        printf("proj4 error: %s \t", pj_strerrno(code));}
  //      txp.x *= RAD_TO_DEG;
  //      txp.y *= RAD_TO_DEG;
  //    } else {
  //      //coord_trans_api::mercator2lonlat(txp.x, txp.y);
  //    }
  //    curve.push_back(txp);
  //  }
  //  OBJECT_GEOMETRY_TYPE type(OBJECT_GEOMETRY_TYPE_Polyline);
  //  OBJECT_STYLE style(OBJECT_STYLE_Rectangle);
  //  parseObjectGeomType(obj, type, style);
  //  txObjGeomPtr geomPtr(new txObjectGeom);
  //  geomPtr->setId(0).setType(type).setStyle(style);
  //  geomPtr->setGeometry(curve, pj_merc ? COORD_WGS84 : COORD_ENU);
  //  tx_ptr->addGeom(geomPtr);
  // }

  {
    for (size_t i = 0, is = obj.geom.size(); i < is; i++) {
      bool closed = obj.geom[i].first;
      const auto& pts = obj.geom[i].second;
      PointVec curve;
      curve.reserve(pts.size());
      for (const auto& p : pts) {
        txPoint txp;
        txp.x = p.x + west;
        txp.y = p.y + south;
        txp.z = p.z;
        curve.push_back(txp);
      }
      if (closed) {
        curve.push_back(curve.front());
      }
      OBJECT_GEOMETRY_TYPE type(OBJECT_GEOMETRY_TYPE_None);
      OBJECT_STYLE style(OBJECT_STYLE_None);
      if (curve.size() == 0) {
      } else if (curve.size() == 1) {
        type = OBJECT_GEOMETRY_TYPE_Point;
      } else {
        parseObjectGeomType(obj, type, style);
      }
      OBJECT_COLOR color;
      if (obj.markcolor == "white") {
        color = OBJECT_COLOR_White;
      } else if (obj.markcolor == "red") {
        color = OBJECT_COLOR_Red;
      } else if (obj.markcolor == "yellow") {
        color = OBJECT_COLOR_Yellow;
      } else if (obj.markcolor == "blue") {
        color = OBJECT_COLOR_Blue;
      } else if (obj.markcolor == "green") {
        color = OBJECT_COLOR_Green;
      } else if (obj.markcolor == "black") {
        color = OBJECT_COLOR_Black;
      } else {
        color = OBJECT_COLOR_Unknown;
      }
      txObjGeomPtr geomPtr(new txObjectGeom);
      geomPtr->setId(max_geom_pkid++).setType(type).setStyle(style);
      geomPtr->setGeometry(curve, COORD_ENU);
      geomPtr->setColor(color);
      tx_ptr->addGeom(geomPtr);
    }
  }

  if (!obj.repeat.empty()) {
    const auto& pts = obj.repeat;
    PointVec curve;
    curve.reserve(pts.size());
    for (const auto& p : pts) {
      txPoint txp;
      txp.x = p.x + west;
      txp.y = p.y + south;
      txp.z = p.z;
      txp.w = p.w;
      txp.h = p.h;

      curve.push_back(txp);
    }
    txLineCurvePtr rep = std::make_shared<txLineCurve>(COORD_ENU);
    rep->setCoords(curve);
    tx_ptr->setRepeat(rep);
  }
}

void ODParse::calcRefLineLen(const std::vector<std::pair<double, double>>& ref_line,
                             std::vector<double>& dis_from_begin) {
  dis_from_begin.resize(ref_line.size(), 0.0);
  for (size_t _i = 1; _i < ref_line.size(); ++_i) {
    auto& _front_p = ref_line[_i - 1];
    auto& _cur_p = ref_line[_i];

    double _x = _cur_p.first - _front_p.first;
    double _y = _cur_p.second - _front_p.second;
    double _dis = sqrt(_x * _x + _y * _y);
    dis_from_begin[_i] = dis_from_begin[_i - 1] + _dis;
  }
}

void ODParse::generateHeaderPtr(ODHeader odheader) {
  double north = 0, south = 0, east = 0, west = 0;
  odheader.getBBox(north, south, east, west);
  tx_header.reset(new txOdHeader);
  tx_header->setGeoreference(odheader.getGeoRef())
      .setEast(east)
      .setWest(west)
      .setNorth(north)
      .setSouth(south)
      .setVendor(odheader.getVendor())
      .setRevMinor(odheader.getReminor())
      .setRevMajor(odheader.getRemajor())
      .setHeaderDate(odheader.getDate());
}

void ODParse::generateSecRefInfo(const std::vector<std::pair<double, double>>& ref_line,
                                 const std::vector<std::pair<double, double>>& ref_normal,
                                 const std::vector<double>& road_ele, const std::vector<double>& dis_from_begin,
                                 const double& s, const double& e, std::vector<std::pair<double, double>>& sec_ref_line,
                                 std::vector<std::pair<double, double>>& sec_ref_normal, std::vector<double>& sec_ele,
                                 std::vector<double>& dis_from_sec) {
  if (ref_line.size() != ref_normal.size() || ref_normal.size() != dis_from_begin.size()) {
    throw std::runtime_error("generate lane geom error, refference info size error");
  }

  std::vector<std::pair<double, double>>& _lane_ref_line = sec_ref_line;
  std::vector<std::pair<double, double>>& _lane_ref_normal = sec_ref_normal;
  std::vector<double>& _lane_dis_from_section = dis_from_sec;
  std::vector<double>& _lane_ele = sec_ele;

  std::vector<size_t> _range;
  for (size_t _i = 0; _i < dis_from_begin.size(); ++_i) {
    if (fabs(dis_from_begin[_i] - s) < 0.1) {
      _range.push_back(_i);
    } else if (fabs(dis_from_begin[_i] - e) < 0.1) {
      _range.push_back(_i);
      break;
    } else if (s < dis_from_begin[_i] && dis_from_begin[_i] < e) {
      _range.push_back(_i);
    }
  }

  for (size_t _i = 0; _i < _range.size(); ++_i) {
    size_t _index = _range[_i];
    if (_index == 0 || _index == ref_line.size() - 1 || (0 < _i && _i < _range.size() - 1) ||
        fabs(dis_from_begin[_index] - s) < 0.1 || fabs(dis_from_begin[_index] - e) < 0.1) {
      _lane_ref_line.push_back(ref_line[_index]);
      _lane_ref_normal.push_back(ref_normal[_index]);
      _lane_ele.push_back(road_ele[_index]);
      _lane_dis_from_section.push_back(dis_from_begin[_index] - s);
    } else {
      size_t _front_index;
      size_t _back_index;
      double _offset;
      if (_i == 0) {
        _front_index = _index - 1;
        _back_index = _index;
        _offset = s - dis_from_begin[_front_index];
      } else {
        _front_index = _index;
        _back_index = _index + 1;
        _offset = e - dis_from_begin[_front_index];

        // insert cur info
        _lane_ref_line.push_back(ref_line[_index]);
        _lane_ref_normal.push_back(ref_normal[_index]);
        _lane_dis_from_section.push_back(dis_from_begin[_index] - s);
      }

      double _x_dir = ref_line[_back_index].first - ref_line[_front_index].first;
      double _y_dir = ref_line[_back_index].second - ref_line[_front_index].second;
      double _dir_len = sqrt(_x_dir * _x_dir + _y_dir * _y_dir);
      _x_dir /= _dir_len;
      _y_dir /= _dir_len;
      double _x = ref_line[_front_index].first + _x_dir * _offset;
      double _y = ref_line[_front_index].second + _y_dir * _offset;
      _lane_ref_line.push_back(std::make_pair(_x, _y));

      double _x_normal = ref_normal[_front_index].first + ref_normal[_back_index].first;
      double _y_normal = ref_normal[_front_index].second + ref_normal[_back_index].second;
      double _normal_len = sqrt(_x_normal * _x_normal + _y_normal * _y_normal);
      _x_normal /= _normal_len;
      _y_normal /= _normal_len;
      _lane_ref_normal.push_back(std::make_pair(_x_normal, _y_normal));

      _lane_ele.push_back((road_ele[_front_index] + road_ele[_back_index]) / 2.0);

      if (_i == 0) {
        _lane_dis_from_section.push_back(0.0);
      } else {
        _lane_dis_from_section.push_back(e - s);
      }

      if (_i == 0) {
        // insert front info
        _lane_ref_line.push_back(ref_line[_index]);
        _lane_ref_normal.push_back(ref_normal[_index]);
        _lane_dis_from_section.push_back(dis_from_begin[_index] - s);
      }
    }
  }
  // special case: only one point in range, should extend geom both in front and back direction
  if (_range.size() == 1) {
    size_t _index = _range[0];
    size_t _front_index;
    size_t _back_index;
    double _offset;

    if (_index > 0 && fabs(dis_from_begin[_index] - e) < 0.1) {
      _front_index = _index - 1;
      _back_index = _index;
      _offset = s - dis_from_begin[_front_index];
    } else {
      _front_index = _index;
      _back_index = _index + 1;
      _offset = e - dis_from_begin[_front_index];
    }

    double _x_dir = ref_line[_back_index].first - ref_line[_front_index].first;
    double _y_dir = ref_line[_back_index].second - ref_line[_front_index].second;
    double _dir_len = sqrt(_x_dir * _x_dir + _y_dir * _y_dir);
    _x_dir /= _dir_len;
    _y_dir /= _dir_len;
    double _x = ref_line[_front_index].first + _x_dir * _offset;
    double _y = ref_line[_front_index].second + _y_dir * _offset;

    double _x_normal = ref_normal[_front_index].first + ref_normal[_back_index].first;
    double _y_normal = ref_normal[_front_index].second + ref_normal[_back_index].second;
    double _normal_len = sqrt(_x_normal * _x_normal + _y_normal * _y_normal);
    _x_normal /= _normal_len;
    _y_normal /= _normal_len;

    if (_index == _back_index) {
      _lane_ref_line.insert(_lane_ref_line.begin(), std::make_pair(_x, _y));
      _lane_ref_normal.insert(_lane_ref_normal.begin(), std::make_pair(_x_normal, _y_normal));
      _lane_ele.insert(_lane_ele.begin(), (road_ele[_front_index] + road_ele[_back_index]) / 2.0);

      _lane_dis_from_section.insert(_lane_dis_from_section.begin(), 0.0);
    } else {
      _lane_ref_line.push_back(std::make_pair(_x, _y));
      _lane_ref_normal.push_back(std::make_pair(_x_normal, _y_normal));
      _lane_ele.push_back((road_ele[_front_index] + road_ele[_back_index]) / 2.0);

      _lane_dis_from_section.push_back(e - s);
    }

  } else if (_range.size() ==
             0) {  // special case: no point in range, should extend geom both in front and back direction
    for (size_t _i = 1; _i < dis_from_begin.size(); ++_i) {
      if (s < dis_from_begin[_i]) {
        _range.push_back(_i - 1);
        break;
      }
    }
    if (_range.size() == 1) {
      size_t _index = _range[0];
      size_t _front_index;
      size_t _back_index;
      double _offset_front, _offset_back;

      _front_index = _index;
      _back_index = _index + 1;
      _offset_front = s - dis_from_begin[_front_index];
      _offset_back = e - dis_from_begin[_front_index];

      double _x_dir = ref_line[_back_index].first - ref_line[_front_index].first;
      double _y_dir = ref_line[_back_index].second - ref_line[_front_index].second;
      double _dir_len = sqrt(_x_dir * _x_dir + _y_dir * _y_dir);
      _x_dir /= _dir_len;
      _y_dir /= _dir_len;
      double _x_front = ref_line[_front_index].first + _x_dir * _offset_front;
      double _y_front = ref_line[_front_index].second + _y_dir * _offset_front;
      double _x_back = ref_line[_front_index].first + _x_dir * _offset_back;
      double _y_back = ref_line[_front_index].second + _y_dir * _offset_back;

      double _x_normal = ref_normal[_front_index].first + ref_normal[_back_index].first;
      double _y_normal = ref_normal[_front_index].second + ref_normal[_back_index].second;
      double _normal_len = sqrt(_x_normal * _x_normal + _y_normal * _y_normal);

      _x_normal /= _normal_len;
      _y_normal /= _normal_len;

      _lane_ref_line.push_back(std::make_pair(_x_front, _y_front));
      _lane_ref_normal.push_back(std::make_pair(_x_normal, _y_normal));
      _lane_ele.push_back(road_ele[_front_index] +
                          (road_ele[_back_index] - road_ele[_front_index]) * _offset_front / _dir_len);
      _lane_dis_from_section.push_back(0.0);

      _lane_ref_line.push_back(std::make_pair(_x_back, _y_back));
      _lane_ref_normal.push_back(std::make_pair(_x_normal, _y_normal));
      _lane_ele.push_back(road_ele[_front_index] +
                          (road_ele[_back_index] - road_ele[_front_index]) * _offset_back / _dir_len);

      _lane_dis_from_section.push_back(e - s);
    }
  }

  while (sec_ele.size() < sec_ref_line.size()) sec_ele.push_back(sec_ele.back());
}

void ODParse::generateRoadPtr(const roadpkid& id, const ROAD_TYPE& type, int speed_limit,
                              const std::vector<std::pair<double, double>>& road_geom,
                              const std::vector<double>& road_ele, txRoadPtr& road_ptr) {
  if (road_geom.size() != road_ele.size()) return;
  road_ptr.reset(new txRoad);
  road_ptr->setId(id).setRoadType(type).setSpeedLimit(speed_limit);

  PointVec _points(road_geom.size(), txPoint());
  for (size_t i = 0; i < road_geom.size(); ++i) {
    double _x = road_geom[i].first;
    double _y = road_geom[i].second;
    double _z = 0;
    if (i < road_ele.size()) _z = road_ele[i];
    // coord_trans_api::mercator2lonlat( _x, _y );
    // coord_trans_api::mercator2lonlat( _x, _y );
    _points[i].x = _x;
    _points[i].y = _y;
    _points[i].z = _z;
  }
  road_ptr->setGeometry(_points, COORD_ENU);
}

void ODParse::generateBoundaryPtr(const laneboundarypkid& id, const LANE_MARK& mark, double lanewidth,
                                  double lanedinterval, double lanedspace, double lanedoffset, double lanedlen,
                                  const std::vector<std::pair<double, double>>& bound_geom,
                                  const std::vector<double>& bound_ele, txLaneBoundaryPtr& bound_ptr) {
  bound_ptr.reset(new txLaneBoundary);

  bound_ptr->setId(id)
      .setLaneMark(mark)
      .setLaneWidth(lanewidth)
      .setDoubleInterval(lanedinterval)
      .setDotOffset(lanedoffset)
      .setDotLen(lanedlen)
      .setDotSpace(lanedspace);

  PointVec _points(bound_geom.size(), txPoint());
  for (size_t i = 0; i < bound_geom.size(); ++i) {
    double _x = bound_geom[i].first;
    double _y = bound_geom[i].second;
    double _z = 0;
    if (i < bound_ele.size()) _z = bound_ele[i];
    // coord_trans_api::mercator2lonlat( _x, _y );

    // coord_trans_api::mercator2lonlat( _x, _y );
    _points[i].x = _x;
    _points[i].y = _y;
    _points[i].z = _z;
  }

  bound_ptr->setGeometry(_points, COORD_ENU);
}

void ODParse::generateLanePtr(const roadpkid& r_id, const sectionpkid& s_id, const lanepkid& l_id,
                              const LANE_TYPE& type, const int& speed_limit, double lanewidth, const int& surface,
                              const double& friction, const double& materialOffset,
                              const std::vector<std::pair<double, double>>& lane_geom,
                              const std::vector<double>& lane_ele, const std::vector<double>& lane_width,
                              txLanePtr& lane_ptr) {
  lane_ptr.reset(new txLane);

  (*lane_ptr)
      .setRoadId(r_id)
      .setSectionId(s_id)
      .setId(l_id)
      .setLaneType(type)
      .setSpeedLimit(speed_limit)
      .setLaneWidth(lanewidth)
      .setLaneSurface(surface)
      .setLaneFriction(friction)
      .setMaterialOffset(materialOffset);

  PointVec _points(lane_geom.size());
  for (size_t i = 0; i < lane_geom.size(); ++i) {
    double _x = lane_geom[i].first;
    double _y = lane_geom[i].second;
    double _z = 0;
    if (i < lane_ele.size()) _z = lane_ele[i];
    _points[i].x = _x;
    _points[i].y = _y;
    _points[i].z = _z;
    if (i < lane_width.size()) _points[i].w = std::abs(lane_width[i]);
  }

  if (_points.size() > 1) {
    std::tuple<hadmap::txPoint, hadmap::txPoint> my_tuple(_points.front(), _points.back());
    m_laneidToStartAndEndPoint[lane_ptr->getTxLaneId()] = my_tuple;
  }
  lane_ptr->setGeometry(_points, COORD_ENU);
}

void ODParse::generateRoadLinks(const ODRoadPtr od_road_ptr, txLaneLinks& links) {
  typedef std::pair<lanepkid, lanepkid> LinkPair;
  std::vector<LinkPair> _pre_links;
  std::vector<LinkPair> _succ_links;

  od_road_ptr->predecessor(_pre_links);
  od_road_ptr->successor(_succ_links);

  if (_pre_links.empty() && _succ_links.empty()) return;

  roadpkid from_rid = od_road_ptr->preRoadId();
  roadpkid cur_rid = od_road_ptr->id();
  roadpkid to_rid = od_road_ptr->succRoadId();

  ODLink::ODLinkType from_rt = od_road_ptr->preRoadType();
  ODLink::ODLinkType to_rt = od_road_ptr->succRoadType();

  ODLink::ODConType preContactPoint = od_road_ptr->preRoadCont();
  ODLink::ODConType succContactPoint = od_road_ptr->succRoadCont();

  if (from_rid != ROAD_PKID_INVALID && from_rt == ODLink::ROAD && !_pre_links.empty() &&
      tx_roads.find(from_rid) != tx_roads.end()) {
    sort(_pre_links.begin(), _pre_links.end(), [](const LinkPair& a, const LinkPair& b) {
      if (a.first == b.first) {
        return a.second > b.second;
      } else {
        return a.first > b.first;
      }
    });

    sectionpkid from_sid =
        od_road_ptr->preRoadCont() == ODLink::START ? 0 : (tx_roads[from_rid]->getSections().size() - 1);
    for (auto _p : _pre_links) {
      roadpkid _from_rid = from_rid;
      sectionpkid _from_sid = from_sid;
      lanepkid _from_lid = _p.first;
      roadpkid _to_rid = cur_rid;
      sectionpkid _to_sid = 0;
      lanepkid _to_lid = _p.second;
      bool revs = _to_lid > 0;
      if (revs) {
        std::swap(_from_rid, _to_rid);
        std::swap(_from_sid, _to_sid);
        std::swap(_from_lid, _to_lid);
      }

      txLaneLinkPtr link_ptr(new txLaneLink);
      (*link_ptr)
          .setId(0)
          .setFromRoadId(_from_rid)
          .setFromSectionId(_from_sid)
          .setFromLaneId(_from_lid)
          .setToRoadId(_to_rid)
          .setToSectionId(_to_sid)
          .setToLaneId(_to_lid);

      link_ptr->setPreContact((txContactType)preContactPoint);
      link_ptr->setSuccContact((txContactType)succContactPoint);
      links.push_back(link_ptr);
    }
  }

  if (to_rid != ROAD_PKID_INVALID && to_rt == ODLink::ROAD && !_succ_links.empty() &&
      tx_roads.find(to_rid) != tx_roads.end()) {
    sort(_succ_links.begin(), _succ_links.end(), [](const LinkPair& a, const LinkPair& b) {
      if (a.first == b.first) {
        return a.second > b.second;
      } else {
        return a.first > b.first;
      }
    });

    sectionpkid from_sid = od_road_ptr->sectionSize() - 1;
    sectionpkid to_sid =
        od_road_ptr->succRoadCont() == ODLink::START ? 0 : (tx_roads[to_rid]->getSections().size() - 1);
    for (auto _p : _succ_links) {
      roadpkid _from_rid = cur_rid;
      sectionpkid _from_sid = from_sid;
      lanepkid _from_lid = _p.first;
      roadpkid _to_rid = to_rid;
      sectionpkid _to_sid = to_sid;
      lanepkid _to_lid = _p.second;
      bool revs = _from_lid > 0;
      if (revs) {
        std::swap(_from_rid, _to_rid);
        std::swap(_from_sid, _to_sid);
        std::swap(_from_lid, _to_lid);
      }
      txLaneLinkPtr link_ptr(new txLaneLink);
      (*link_ptr)
          .setId(0)
          .setFromRoadId(_from_rid)
          .setFromSectionId(_from_sid)
          .setFromLaneId(_from_lid)
          .setToRoadId(_to_rid)
          .setToSectionId(_to_sid)
          .setToLaneId(_to_lid);
      link_ptr->setPreContact((txContactType)preContactPoint);
      link_ptr->setSuccContact((txContactType)succContactPoint);
      // std::cout << "preContactPoint:::" << preContactPoint;
      // std::cout << "succContactPoint:::" << succContactPoint;
      links.push_back(link_ptr);
    }
  }

  // 将两条收尾相连车道连接
  if (from_rid == ROAD_PKID_INVALID) {
    roadpkid _cur_roadid = od_road_ptr->id();
    hadmap::txSectionPtr _end_sections = tx_roads.find(_cur_roadid)->second->getSections().back();
    for (auto itLane : _end_sections->getLanes()) {
      hadmap::txLaneId laneid = itLane->getTxLaneId();
      hadmap::txPoint _startPoint = std::get<0>(m_laneidToStartAndEndPoint.find(laneid)->second);
      for (auto it : m_laneidToStartAndEndPoint) {
        hadmap::txPoint _endPoint = std::get<1>(it.second);
        double distance = std::pow(_startPoint.x - _endPoint.x, 2) + std::pow(_startPoint.y - _endPoint.y, 2);
        if (distance < 1.0) {
          txLaneLinkPtr link_ptr(new txLaneLink);
          (*link_ptr)
              .setId(0)
              .setFromRoadId(it.first.roadId)
              .setFromSectionId(it.first.sectionId)
              .setFromLaneId(it.first.laneId)
              .setToRoadId(laneid.roadId)
              .setToSectionId(laneid.sectionId)
              .setToLaneId(laneid.laneId);
          link_ptr->setPreContact(txContactType::END);
          link_ptr->setSuccContact(txContactType::START);
          links.push_back(link_ptr);
        }
      }
    }
    // m_laneidToStartAndEndPoint
  }
}

bool ODParse::getData(txRoads& roads, txLaneLinks& links, txObjects& objs, txJunctions& juncs, txOdHeaderPtr& header,
                      txPoint& refPoint, bool doubleRoad) {
  if (doubleRoad) {
    splitBilateral();
  }
  roads.clear();
  for (auto _itr = tx_roads.begin(); _itr != tx_roads.end(); ++_itr) roads.push_back(_itr->second);

  links.clear();
  for (auto _itr = tx_links.begin(); _itr != tx_links.end(); ++_itr)
    links.insert(links.end(), _itr->second.begin(), _itr->second.end());

  objs.clear();
  for (auto _itr = tx_objects.begin(); _itr != tx_objects.end(); ++_itr)
    objs.insert(objs.end(), _itr->second.begin(), _itr->second.end());

  juncs.clear();
  for (auto _itr = tx_junctions.begin(); _itr != tx_junctions.end(); ++_itr) juncs.push_back(_itr->second);

  header = this->tx_header;
  refPoint = this->tx_refpoint;
  return true;
}

void ODParse::splitBilateral() {
  roadpkid dummyId = 100000000;
  for (const auto& _itr : tx_roads) {
    dummyId = std::max(dummyId, _itr.first);
  }
  dummyId = std::pow(10, 1 + std::ceil(std::log10(dummyId)));

  auto ReservePoints = [](const txCurve* curve) -> PointVec {
    PointVec points;
    if (curve) dynamic_cast<const txLineCurve*>(curve)->getPoints(points);
    std::reverse(points.begin(), points.end());
    return points;
  };
  std::unordered_map<roadpkid, txRoadPtr> new_roads;
  std::map<txLaneId, txLaneId> laneIdReplace;
  for (auto _itr = tx_roads.begin(); _itr != tx_roads.end();) {
    bool bil = false;
    for (const auto& sec : _itr->second->getSections()) {
      if (!sec->getLanes().empty() && sec->getLanes().front()->getId() > 0) {
        bil = true;
        break;
      }
    }
    if (bil) {
      txRoadPtr new_road_ptr(new txRoad);
      new_road_ptr->setId(_itr->second->getId() + dummyId).setRoadType(_itr->second->getRoadType());
      new_road_ptr->setGeometry(ReservePoints(_itr->second->getGeometry()),
                                _itr->second->getGeometry()->getCoordType());
      // set curvature
      txCurvatureVec vecOld = _itr->second->getCurvature();
      txCurvatureVec vecNew;
      double start_s = 0;
      for (int i = vecOld.size() - 1; i >= 0; i--) {
        txCurvature tmp = vecOld.at(i);
        tmp.m_starts = start_s;
        vecNew.push_back(tmp);
        start_s += tmp.m_length;
      }
      new_road_ptr->setCurvature(vecNew);
      //
      txSlopeVec vecSlopeOld = _itr->second->getSlope();
      txSlopeVec vecSlopeNew;
      start_s = 0;
      for (int i = vecSlopeOld.size() - 1; i >= 0; i--) {
        txSlope tmp = vecSlopeOld.at(i);
        tmp.m_starts = start_s;
        vecSlopeNew.push_back(tmp);
        start_s += tmp.m_length;
      }
      new_road_ptr->setSlope(vecSlopeOld);

      size_t remain_lane_num = 0;
      for (auto& sec : _itr->second->getSections()) {
        if (sec->getLanes().empty()) {
          txlog::warning(std::string("empty sec, rid:") + std::to_string(_itr->first) +
                         ", sid:" + std::to_string(sec->getId()));
          continue;
        }
        txSectionPtr new_sec_ptr(new txSection);
        new_sec_ptr->setId(_itr->second->getSections().size() - 1 - sec->getId());
        auto& lanes = sec->getLanes();
        assert(lanes.size() + 1 == sec->getBoundaries().size());
        size_t nxLaneNum = std::count_if(lanes.begin(), lanes.end(), [](txLanePtr lane) { return lane->getId() > 0; });
        if (nxLaneNum == 0) continue;
        std::vector<txLaneBoundaryPtr> new_bound_ptrs(nxLaneNum + 1);
        for (size_t i = 0; i < new_bound_ptrs.size(); ++i) {
          txLaneBoundaryPtr bound = sec->getBoundaries().at(i);
          txLaneBoundaryPtr new_bound_ptr(new txLaneBoundary);
          new_bound_ptr->setId(max_bound_pkid++).setLaneMark(bound->getLaneMark());
          new_bound_ptr->setGeometry(ReservePoints(bound->getGeometry()), bound->getGeometry()->getCoordType());
          new_bound_ptrs[new_bound_ptrs.size() - 1 - i] = new_bound_ptr;
        }
        for (size_t i = 0; i < nxLaneNum; i++) {
          txLanePtr lane = lanes[nxLaneNum - 1 - i];
          txLanePtr new_lane(new txLane);

          (*new_lane)
              .setRoadId(new_road_ptr->getId())
              .setSectionId(new_sec_ptr->getId())
              .setId(-lane->getId())
              .setLaneType(lane->getLaneType())
              .setLaneWidth(lane->getLaneWidth())
              .setSpeedLimit(lane->getSpeedLimit())
              .setLaneArrow(lane->getLaneArrow());
          new_lane->setGeometry(ReservePoints(lane->getGeometry()), lane->getGeometry()->getCoordType());

          new_lane->setLeftBoundary(new_bound_ptrs[i]);
          new_lane->setRightBoundary(new_bound_ptrs[i + 1]);

          new_sec_ptr->add(new_lane);

          laneIdReplace[lane->getTxLaneId()] = new_lane->getTxLaneId();
        }

        new_sec_ptr->bindSectionPtr();
        new_sec_ptr->setMeanCurvature(sec->getMeanCurvature());
        new_sec_ptr->setMeanSlope(sec->getMeanSlope() * -1.0);
        new_road_ptr->addSection(new_sec_ptr);
        // remove old
        for (size_t i = 0; i < nxLaneNum; i++) {
          sec->getLanes().erase(sec->getLanes().begin());
        }
        remain_lane_num += sec->getLanes().size();
      }
      auto& section = new_road_ptr->getSections();
      std::reverse(section.begin(), section.end());
      new_road_ptr->bindRoadPtr();
      for (int i = 0; i < section.size(); i++) {
        section.at(i);
      }
      new_roads.insert(std::make_pair(new_road_ptr->getId(), new_road_ptr));
      if (remain_lane_num == 0) {
        _itr = tx_roads.erase(_itr);
        continue;
      }
    }
    ++_itr;
  }
  if (new_roads.empty()) return;
  tx_roads.insert(new_roads.begin(), new_roads.end());
  std::map<txLaneId, txLanePtr> lanesAll;
  for (const auto& road : tx_roads) {
    for (const auto& sec : road.second->getSections()) {
      for (txLanePtr lane : sec->getLanes()) {
        lanesAll[lane->getTxLaneId()] = lane;
      }
    }
  }
  auto Distance = [](const txPoint& p1, const txPoint& p2, bool wgs = true) -> double {
    return map_util::distanceBetweenPoints(Point3d(p1.x, p1.y, p1.z), Point3d(p2.x, p2.y, p2.z), wgs);
  };
  for (auto _itr = tx_links.begin(); _itr != tx_links.end(); ++_itr) {
    txLaneLinks& links = _itr->second;
    for (auto& link : links) {
      txLanePtr from = lanesAll[link->fromTxLaneId()];
      txLanePtr to = lanesAll[link->toTxLaneId()];
      // reverze
      txContactType fromType = link->getPreContact();
      txContactType toType = link->getSuccContact();

      if (link->fromLaneId() > 0 && laneIdReplace.find(link->fromTxLaneId()) != laneIdReplace.end()) {
        txLaneId newid = laneIdReplace[link->fromTxLaneId()];
        assert(newid.roadId > 0);
        link->setFromRoadId(newid.roadId).setFromSectionId(newid.sectionId).setFromLaneId(newid.laneId);
        if (fromType == txContactType::START) {
          link->setPreContact(txContactType::END);
        } else {
          link->setPreContact(txContactType::START);
        }
      }
      if (link->toLaneId() > 0 && laneIdReplace.find(link->toTxLaneId()) != laneIdReplace.end()) {
        txLaneId newid = laneIdReplace[link->toTxLaneId()];
        assert(newid.roadId > 0);
        link->setToRoadId(newid.roadId).setToSectionId(newid.sectionId).setToLaneId(newid.laneId);
        if (toType == txContactType::START) {
          link->setSuccContact(txContactType::END);
        } else {
          link->setSuccContact(txContactType::START);
        }
      }

      if (!link->getGeometry() || link->getGeometry()->empty()) continue;
      if (!from.get() || !to.get()) continue;
      if (!from->getGeometry() || !to->getGeometry() || from->getGeometry()->empty() || to->getGeometry()->empty()) {
        continue;
      }

      double disF = Distance(link->getGeometry()->getStart(), from->getGeometry()->getEnd(),
                             link->getGeometry()->getCoordType() == COORD_WGS84);
      double disB = Distance(link->getGeometry()->getEnd(), to->getGeometry()->getStart(),
                             link->getGeometry()->getCoordType() == COORD_WGS84);
      double test_disF = Distance(link->getGeometry()->getStart(), from->getGeometry()->getStart(),
                                  link->getGeometry()->getCoordType() == COORD_WGS84);
      double test_disB = Distance(link->getGeometry()->getEnd(), to->getGeometry()->getEnd(),
                                  link->getGeometry()->getCoordType() == COORD_WGS84);
      if ((disF + disB) > (test_disB + test_disF)) {
        link->setFromRoadId(to->getRoadId()).setFromSectionId(to->getSectionId()).setFromLaneId(to->getId());
        if (link->getPreContact() == txContactType::START) {
          link->setPreContact(txContactType::END);
        } else {
          link->setPreContact(txContactType::START);
        }
        link->setToRoadId(from->getRoadId()).setToSectionId(from->getSectionId()).setToLaneId(from->getId());
        if (link->getSuccContact() == txContactType::START) {
          link->setSuccContact(txContactType::END);
        } else {
          link->setSuccContact(txContactType::START);
        }
        link->setGeometry(ReservePoints(link->getGeometry()), link->getGeometry()->getCoordType());

        auto leftBds = link->getLeftBoundaries();
        auto rightBds = link->getRightBoundaries();
        for (auto bd : leftBds) {
          if (bd) bd->setGeometry(ReservePoints(bd->getGeometry()), bd->getGeometry()->getCoordType());
        }
        for (auto bd : rightBds) {
          if (bd) bd->setGeometry(ReservePoints(bd->getGeometry()), bd->getGeometry()->getCoordType());
        }
        std::reverse(leftBds.begin(), leftBds.end());
        std::reverse(rightBds.begin(), rightBds.end());
        link->setLeftBoundaries(rightBds);
        link->setRightBoundaries(leftBds);
      }
    }
  }
  for (auto& objs : tx_objects) {
    auto& txobjects = objs.second;
    for (auto& obj : txobjects) {
      std::vector<txLaneId> rlanes;
      obj->getReliedLaneIds(rlanes);
      for (auto& rl : rlanes) {
        if (laneIdReplace.find(rl) != laneIdReplace.end()) {
          rl = laneIdReplace[rl];
        }
      }
      obj->setReliedLaneIds(rlanes);
      // tunnel
      if (obj->getObjectType() == OBJECT_TYPE_Tunnel) {
        if (tx_roads.find(objs.first + dummyId) != tx_roads.end()) {
          std::map<std::string, std::string> userData;
          userData["relate_road"] = std::to_string(objs.first + dummyId);
          obj->setUserData(userData);
          // add object
          txObjectPtr _cur_object = std::make_shared<txObject>();
          tx_object_t _cur_object_data = obj->getTxData();
          double _s =
              tx_roads.find(objs.first + dummyId)->second->getLength() - (_cur_object_data.s + _cur_object_data.length);
          _cur_object_data.s = (_s >= 0) ? _s : 0;
          _cur_object->setData(_cur_object_data);
          tx_od_object_t _cur_object_od_data = obj->getOdData();
          _cur_object_od_data.s = (_s >= 0) ? _s : 0;
          userData.clear();
          userData["relate_road"] = std::to_string(objs.first);
          _cur_object->setTxOdData(_cur_object_od_data);
          _cur_object->setOdRoadId(objs.first + dummyId);
          _cur_object->setUserData(userData);
          txObjGeomPtr geomPtr(new txObjectGeom);
          _cur_object->addGeom(geomPtr);
          _cur_object->setId(dummyId + obj->getId());
          PointVec curve;
          curve.reserve(1);
          curve.push_back(tx_roads.find(objs.first + dummyId)->second->getGeometry()->getPoint(_cur_object_od_data.s));
          geomPtr->setGeometry(curve, COORD_WGS84);
          if (tx_objects.find(objs.first + dummyId) == tx_objects.end()) {
            txObjects _objects;
            _objects.push_back(_cur_object);
            tx_objects[objs.first + dummyId] = _objects;
          } else {
            tx_objects.find(objs.first + dummyId)->second.push_back(_cur_object);
          }
        }
      }
    }
  }
}

bool ODParse::isInRecVecArea(const txCurve* curveptr, std::vector<std::vector<txPoint>> ptr) {
  if (curveptr) {
    std::vector<hadmap::txPoint> points;
    for (int k = 0; k < points.size(); k++) {
      txPoint point = points.at(k);
      for (int m = 0; m < ptr.size(); m++) {
        if (map_util::isPointInRect(Point2d(point.x, point.y), Point2d(ptr.at(m).at(0).x, ptr.at(m).at(0).y),
                                    Point2d(ptr.at(m).at(2).x, ptr.at(m).at(2).y))) {
          return true;
        }
      }
    }
  }
  return false;
}

bool ODParse::isInRecVecArea(txSections _sections, std::vector<std::vector<txPoint>> ptr) {
  for (txSectionPtr iter : _sections) {
    txLaneBoundaries txBoundaries = iter->getBoundaries();
    for (auto boundary : txBoundaries) {
      const txCurve* curveptr = boundary->getGeometry();
      if (curveptr) {
        std::vector<hadmap::txPoint> points;
        curveptr->sample(1, points);
        for (auto point : points) {
          for (int i = 0; i < ptr.size(); i++) {
            if (map_util::isPointInRect(Point2d(point.x, point.y), Point2d(ptr.at(i).at(0).x, ptr.at(i).at(0).y),
                                        Point2d(ptr.at(i).at(2).x, ptr.at(i).at(2).y))) {
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

bool ODParse::DoRoad(const hadmap::ODRoadPtr ptr) {
  ODRoadPtr _cur_road_ptr = ptr;
  roadpkid _cur_road_id = _cur_road_ptr->id();
  std::vector<std::pair<double, double>> _road_geom;
  std::vector<double> _road_ele;
  std::vector<double> _road_interval;
  _cur_road_ptr->roadInterval(_road_interval);
  _cur_road_ptr->roadGeom(_road_geom);
  _cur_road_ptr->roadEle(_road_ele);
  std::vector<double> _all;

  for (auto& _point : _road_geom) {
    _point.first += _west;
    _point.second += _south;
  }
  if (_road_ele.size() < _road_geom.size()) {
    size_t s = _road_ele.size();
    double h = _road_ele.empty() ? 0 : _road_ele.back();
    _road_ele.resize(_road_geom.size(), h);
  }
  // generate tx road ptr
  txRoadPtr _tx_road_ptr;
  ROAD_TYPE eType = GetRoadType(_cur_road_ptr->getRoadType());
  generateRoadPtr(_cur_road_ptr->id(), eType, _cur_road_ptr->speedlimit(), _road_geom, _road_ele, _tx_road_ptr);
  if (_tx_road_ptr) {
    std::vector<double> startx, length, radios;
    _cur_road_ptr->roadCurvature(startx, length, radios);
    txCurvatureVec tmpVec;
    for (size_t i = 0; i < startx.size(); i++) {
      txCurvature tmp;
      tmp.m_starts = startx.at(i);
      tmp.m_length = length.at(i);
      tmp.m_curvature = radios.at(i);
      tmpVec.push_back(tmp);
    }
    _tx_road_ptr->setCurvature(tmpVec);
    // calculate slope
    std::vector<double> _startx, _length, _slopes;
    _cur_road_ptr->roadSlope(_startx, _length, _slopes);
    txSlopeVec tmpSlopeVec;
    for (size_t i = 0; i < _startx.size(); i++) {
      txSlope tmp;
      tmp.m_starts = _startx.at(i);
      tmp.m_length = _length.at(i);
      tmp.m_slope = _slopes.at(i);
      tmpSlopeVec.push_back(tmp);
    }
    _tx_road_ptr->setSlope(tmpSlopeVec);
    // set controlPoint
    if (this->od_header.getVendor() == "tadsim v2.0") {
      txControlPoint point;
      std::vector<std::vector<double>> _controlPoint;
      _cur_road_ptr->roadControlPoint(point.m_type, _controlPoint);
      for (auto it : _controlPoint) {
        txPoint tmp(it.at(0), it.at(1), 0);
        tmp.hdg = it.at(2);
        point.m_points.push_back(tmp);
      }
      _tx_road_ptr->setControlPoint(point);
    }
    // set crg
    std::vector<ODCrg> _crgs = _cur_road_ptr->getCrgs();
    _tx_road_ptr->setCrgs(GetOpenCrg(_crgs));
    // set ele control
    std::vector<std::vector<double>> points;
    _cur_road_ptr->roadEleControlPoint(points, _cur_road_ptr->length(), this->od_header.getVendor());
    txControlPoint cPoint;
    for (auto it : points) {
      hadmap::txPoint ptA;
      ptA.x = it.at(0);
      ptA.y = it.at(1);
      ptA.h = it.at(2);
      cPoint.m_points.push_back(ptA);
    }
    if (cPoint.m_points.size() > 0) {
      cPoint.m_type = "catmullrom";
      _tx_road_ptr->setElePoint(cPoint);
    }

    // generate sections
    txSections _sections;
    std::vector<Wid_Data> wid_datas(_cur_road_ptr->sectionSize());

    for (size_t _sec_i = 0; _sec_i < _cur_road_ptr->sectionSize(); ++_sec_i) {
      ODLaneSection& _sec = wid_datas[_sec_i]._sec;
      _cur_road_ptr->getSection(_sec_i, _sec);

      auto& _bdy_width = wid_datas[_sec_i]._bdy_width;
      auto& _lane_width = wid_datas[_sec_i]._lane_width;
      auto& _bdy_mark = wid_datas[_sec_i]._bdy_mark;
      auto& _bdy_lanewidth = wid_datas[_sec_i]._bdy_lanewidth;
      auto& _bdy_lanedinterval = wid_datas[_sec_i]._bdy_lanedinterval;
      auto& _bdy_lanedspace = wid_datas[_sec_i]._bdy_lanedspace;
      auto& _bdy_lanedsoffset = wid_datas[_sec_i]._bdy_lanedsoffset;
      auto& _bdy_lanedlen = wid_datas[_sec_i]._bdy_lanedlen;
      auto& _dis_from_sec = wid_datas[_sec_i]._dis_from_sec;

      _bdy_width.resize(_sec.size());
      _lane_width.resize(_sec.size());
      _bdy_mark.resize(_sec.size(), LANE_MARK_None);
      _bdy_lanewidth.resize(_sec.size(), 0);
      _bdy_lanedinterval.resize(_sec.size(), 0);
      _bdy_lanedspace.resize(_sec.size(), 0);
      _bdy_lanedsoffset.resize(_sec.size(), 0);
      _bdy_lanedlen.resize(_sec.size(), 0);

      double _sec_s, _sec_e;
      _cur_road_ptr->getSectionRange(_sec_i, _sec_s, _sec_e);

      _sec_s += 0.001;
      _sec_e -= 0.001;
      if (!_road_interval.empty() && _sec_s < _sec_e) {
        _dis_from_sec.push_back(_sec_s);
        for (auto d : _road_interval) {
          if (d > _sec_s && d < _sec_e) {
            _dis_from_sec.push_back(d);
          }
        }
        _dis_from_sec.push_back(_sec_e);
      }
      if (_dis_from_sec.empty()) {
        continue;
      }
      _all.insert(_all.end(), _dis_from_sec.begin(), _dis_from_sec.end());
      auto& _sec_ele = wid_datas[_sec_i]._sec_ele;
      auto& _sec_ref_line = wid_datas[_sec_i]._sec_ref_line;
      auto& _sec_ref_normal = wid_datas[_sec_i]._sec_ref_normal;

      _cur_road_ptr->roadRef(_sec_ref_line, &_dis_from_sec);
      _cur_road_ptr->roadNormal(_sec_ref_normal, &_dis_from_sec);
      _cur_road_ptr->roadEle(_sec_ele, &_dis_from_sec);

      for (auto& _point : _sec_ref_line) {
        _point.first += _west;
        _point.second += _south;
      }
      for (auto& s : _dis_from_sec) {
        s -= _sec_s - 0.001;
      }
      for (auto& lw : _lane_width) {
        lw = std::vector<double>(_sec_ref_line.size(), 0.0);
      }
      for (auto& lw : _bdy_width) {
        lw = std::vector<double>(_sec_ref_line.size(), 0.0);
      }
      std::vector<double> _width0(_sec_ref_line.size(), 0.0);
      for (size_t i = 0; i < _sec.size(); ++i) {
        if (_sec[i]->id() >= 0) {
          continue;
        }
        auto& lw = _lane_width[i];
        auto& bw = _bdy_width[i];
        _bdy_mark[i] = _sec[i]->mark();
        _bdy_lanewidth[i] = _sec[i]->lanewidth();
        _bdy_lanedinterval[i] = _sec[i]->lanedinterval();
        _bdy_lanedspace[i] = _sec[i]->laned_space;
        _bdy_lanedsoffset[i] = _sec[i]->laned_soffset;
        _bdy_lanedlen[i] = _sec[i]->laned_len;
        std::vector<double> _cur_width;
        _sec[i]->width(_dis_from_sec, _cur_width);
        for (size_t j = 0; j < _sec_ref_line.size(); ++j) {
          lw[j] = _width0[j] + _cur_width[j] * 0.5;
          _width0[j] += _cur_width[j];
          bw[j] = _width0[j];
        }
      }
      _width0 = std::vector<double>(_sec_ref_line.size(), 0.0);
      for (int i = _sec.size() - 1; i >= 0; --i) {
        if (_sec[i]->id() <= 0) {
          continue;
        }
        auto& lw = _lane_width[i];
        auto& bw = _bdy_width[i];
        _bdy_mark[i] = _sec[i]->mark();
        _bdy_lanewidth[i] = _sec[i]->lanewidth();
        _bdy_lanedinterval[i] = _sec[i]->lanedinterval();
        _bdy_lanedspace[i] = _sec[i]->laned_space;
        _bdy_lanedsoffset[i] = _sec[i]->laned_soffset;
        _bdy_lanedlen[i] = _sec[i]->laned_len;
        std::vector<double> _cur_width;
        _sec[i]->width(_dis_from_sec, _cur_width);
        for (size_t j = 0; j < _sec_ref_line.size(); ++j) {
          lw[j] = _width0[j] - _cur_width[j] * 0.5;
          _width0[j] -= _cur_width[j];
          bw[j] = _width0[j];
        }
      }
      for (size_t i = 0; i < _sec.size(); ++i) {
        if (_sec[i]->id() == 0) {
          _bdy_mark[i] = _sec[i]->mark();
          _bdy_lanewidth[i] = _sec[i]->lanewidth();
          _bdy_lanedinterval[i] = _sec[i]->lanedinterval();
          _bdy_lanedspace[i] = _sec[i]->laned_space;
          _bdy_lanedsoffset[i] = _sec[i]->laned_soffset;
          _bdy_lanedlen[i] = _sec[i]->laned_len;
          auto& bw = _bdy_width[i];
          _sec[i]->width(_dis_from_sec, bw);
          break;
        }
      }
    }
    for (size_t _sec_i = 0; _sec_i < _cur_road_ptr->sectionSize(); ++_sec_i) {
      ODLaneSection& _sec = wid_datas[_sec_i]._sec;

      auto& _dis_from_sec = wid_datas[_sec_i]._dis_from_sec;
      auto& _sec_ele = wid_datas[_sec_i]._sec_ele;
      auto& _sec_ref_line = wid_datas[_sec_i]._sec_ref_line;
      auto& _sec_ref_normal = wid_datas[_sec_i]._sec_ref_normal;
      auto& _bdy_width = wid_datas[_sec_i]._bdy_width;
      auto& _lane_width = wid_datas[_sec_i]._lane_width;
      auto& _bdy_mark = wid_datas[_sec_i]._bdy_mark;
      auto& _bdy_lanewidth = wid_datas[_sec_i]._bdy_lanewidth;
      auto& _bdy_lanedinterval = wid_datas[_sec_i]._bdy_lanedinterval;
      auto& _bdy_lanedspace = wid_datas[_sec_i]._bdy_lanedspace;
      auto& _bdy_lanedsoffset = wid_datas[_sec_i]._bdy_lanedsoffset;
      auto& _bdy_lanedlen = wid_datas[_sec_i]._bdy_lanedlen;

      // generate cur section info
      txSectionPtr _cur_sec_ptr(new txSection);
      (*_cur_sec_ptr).setRoadId(_cur_road_ptr->id()).setId(sectionpkid(_sec_i));

      std::vector<txLaneBoundaryPtr> _bound_ptrs(_bdy_width.size());
      for (size_t i = 0; i < _bdy_width.size(); ++i) {
        auto& bw = _bdy_width[i];
        std::vector<std::pair<double, double>> _bound_geom;
        for (size_t j = 0; j < _sec_ref_line.size(); ++j) {
          double wid = bw[j];
          double _b_x = _sec_ref_line[j].first - _sec_ref_normal[j].first * wid;
          double _b_y = _sec_ref_line[j].second - _sec_ref_normal[j].second * wid;
          _bound_geom.push_back(std::make_pair(_b_x, _b_y));
        }

        // generate boundary
        txLaneBoundaryPtr _bound_ptr;
        generateBoundaryPtr(0, _bdy_mark[i], _bdy_lanewidth[i], _bdy_lanedinterval[i], _bdy_lanedspace[i],
                            _bdy_lanedsoffset[i], _bdy_lanedlen[i], _bound_geom, _sec_ele, _bound_ptr);

        mutex.lock();
        _bound_ptr->setId(_bound_pkid++);
        mutex.unlock();
        _bound_ptrs[i] = _bound_ptr;
      }
      // added lane hook up

      for (size_t i = 0; i < _sec.size(); ++i) {
        if (_sec[i]->id() == 0) {
          continue;
        }
        const auto& lw = _lane_width[i];

        if (!_dis_from_sec.empty() && _dis_from_sec.back() > 1e-4) {
          const double fix_dis = std::min(10.0, _dis_from_sec.back());
          const double equal_dis = 1.0;
          if (_sec_i > 0) {
            std::vector<bool> valid_link_pre(_sec.size(), false);
            for (size_t k = 0; k < _sec.size(); ++k) {
              if (_sec[k]->id() == 0) {
                continue;
              }
              const auto& lw = _lane_width[k];
              if (_sec[k]->fromLId() != LANE_PKID_INVALID) {
                bool flag = false;
                for (int i = 0; i < wid_datas[_sec_i - 1]._sec.size(); i++) {
                  if (_sec[k]->fromLId() == wid_datas[_sec_i - 1]._sec[i]->id()) {
                    auto& frontLaneWidth = wid_datas[_sec_i - 1]._lane_width[i];
                    if (!frontLaneWidth.empty() && std::abs(frontLaneWidth.back() - lw.front()) > equal_dis) {
                      valid_link_pre[k] = false;
                      flag = true;
                      break;
                    }
                  }
                }
                if (flag) continue;
                valid_link_pre[k] = true;
                continue;
              }

              auto& pre_lane_width = wid_datas[_sec_i - 1]._lane_width;
              for (auto pre_wid : pre_lane_width) {
                if (!pre_wid.empty() && std::abs(pre_wid.back() - lw.front()) < equal_dis) {
                  valid_link_pre[k] = true;
                  break;
                }
              }
            }
            for (size_t i = 0; i < _sec.size(); ++i) {
              if (_sec[i]->id() == 0) {
                continue;
              }
              if (valid_link_pre[i]) {
                continue;
              }
              auto id = _sec[i]->id();

              for (int j = 0; j < _sec.size(); j++) {
                int neast_i = 0;
                if (_sec[i]->fromLId() != LANE_PKID_INVALID) {
                  ODLaneSection& _secLast = wid_datas[_sec_i - 1]._sec;
                  lanepkid idd = _sec[i]->fromLId();
                  for (int k = 0; k < _secLast.size(); k++) {
                    if (_secLast[k]->id() == idd) {
                      for (int m = 0; m < _sec.size(); m++) {
                        if (_sec.at(m)->id() == _secLast[k]->toLId()) {
                          neast_i = m;
                          break;
                        }
                      }
                      break;
                    }
                  }
                }
                if (neast_i == 0) {
                  int d = j / 2 + 1;
                  d *= j % 2 == 0 ? 1 : -1;
                  d *= id > 0 ? 1 : -1;
                  double flag = 1.0;
                  neast_i = i + flag * j;
                }

                if (neast_i >= 0 && neast_i < _sec.size() && valid_link_pre[neast_i] && id * _sec[neast_i]->id() > 0) {
                  auto& lw = _lane_width[i];
                  const auto& nlw = _lane_width[neast_i];
                  for (size_t k = 0; k < lw.size(); k++) {
                    if (_dis_from_sec[k] >= fix_dis) {
                      break;
                    }
                    const double pi2 = 1.5707963267948966192313216916398;
                    const double e = 2.7182818284590452353602874713527;
                    double d = _dis_from_sec[k] * 10 / fix_dis - 5;
                    double alpha = 1 / (1 + std::pow(e, -d));
                    lw[k] = lw[k] * alpha + nlw[k] * (1 - alpha);
                  }
                  break;
                }
              }
            }
          }
          if (_sec_i < _cur_road_ptr->sectionSize() - 1) {
            std::vector<bool> valid_link_suc(_sec.size(), false);
            for (size_t k = 0; k < _sec.size(); ++k) {
              if (_sec[k]->id() == 0) {
                continue;
              }
              const auto& lw = _lane_width[k];
              if (_sec[k]->toLId() != LANE_PKID_INVALID) {
                bool flag = false;
                for (int i = 0; i < wid_datas[_sec_i + 1]._lane_width.size(); i++) {
                  if (_sec[k]->toLId() == wid_datas[_sec_i + 1]._sec[i]->id()) {
                    auto& frontLaneWidth = wid_datas[_sec_i + 1]._lane_width[i];
                    if (!frontLaneWidth.empty() && std::abs(frontLaneWidth.front() - lw.back()) > equal_dis) {
                      valid_link_suc[k] = false;
                      flag = true;
                      break;
                    }
                  }
                }
                if (flag) continue;
                valid_link_suc[k] = true;
                continue;
              }

              auto& post_lane_width = wid_datas[_sec_i + 1]._lane_width;
              for (auto post_wid : post_lane_width) {
                if (!post_wid.empty() && std::abs(post_wid.front() - lw.back()) < equal_dis) {
                  valid_link_suc[k] = true;
                  break;
                }
              }
            }
            for (size_t i = 0; i < _sec.size(); ++i) {
              if (_sec[i]->id() == 0) {
                continue;
              }
              if (valid_link_suc[i]) {
                continue;
              }
              auto id = _sec[i]->id();

              for (int j = 0; j < _sec.size(); j++) {
                int neast_i = 0;
                if (_sec[i]->toLId() != LANE_PKID_INVALID) {
                  ODLaneSection& _secLast = wid_datas[_sec_i + 1]._sec;
                  lanepkid idd = _sec[i]->toLId();
                  for (int k = 0; k < _secLast.size(); k++) {
                    if (_secLast[k]->id() == idd) {
                      for (int m = 0; m < _sec.size(); m++) {
                        if (_sec.at(m)->id() == _secLast[k]->fromLId()) {
                          neast_i = m;
                          break;
                        }
                      }
                      break;
                    }
                  }
                }
                if (neast_i == 0) {
                  int d = j / 2 + 1;
                  d *= j % 2 == 0 ? 1 : -1;
                  d *= id > 0 ? 1 : -1;
                  double flag = 1.0;
                  neast_i = i + flag * j;
                }

                if (neast_i >= 0 && neast_i < _sec.size() && valid_link_suc[neast_i] && id * _sec[neast_i]->id() > 0) {
                  auto& lw = _lane_width[i];
                  const auto& nlw = _lane_width[neast_i];
                  for (int k = lw.size() - 1; k >= 0; k--) {
                    double d = _dis_from_sec.back() - _dis_from_sec[k];
                    if (d >= fix_dis) {
                      break;
                    }
                    d = d * 10 / fix_dis - 5;
                    const double pi2 = 1.5707963267948966192313216916398;
                    const double e = 2.7182818284590452353602874713527;
                    double alpha = 1 / (1 + std::pow(e, -d));
                    lw[k] = lw[k] * alpha + nlw[k] * (1 - alpha);
                  }
                  break;
                }
              }
            }
          }
        }

        // added lane hook up
        if (!_dis_from_sec.empty()) {
          double fix_dis = std::min(10.0, _dis_from_sec.back());
          const double equal_dis = 0.5;
          if (_sec_i > 0 && _sec[i]->fromLId() == LANE_PKID_INVALID) {
            bool fd = false;
            auto& pre_lane_width = wid_datas[_sec_i - 1]._lane_width;
            for (auto pre_wid : pre_lane_width) {
              if (!pre_wid.empty() && std::abs(pre_wid.back() - lw.front()) < equal_dis) {
                fd = true;
              }
            }
          }
        }

        std::vector<std::pair<double, double>> _lane_geom;
        for (size_t j = 0; j < _sec_ref_line.size(); ++j) {
          double wid = lw[j];
          double _l_x = _sec_ref_line[j].first - _sec_ref_normal[j].first * wid;
          double _l_y = _sec_ref_line[j].second - _sec_ref_normal[j].second * wid;
          _lane_geom.push_back(std::make_pair(_l_x, _l_y));
        }

        // generate lane ptr
        double lanewidth = -1;
        if (i > 0 && !_bdy_width[i].empty()) {
          lanewidth = 0.5 * (_bdy_width[i].front() - _bdy_width[i - 1].front() + _bdy_width[i].back() -
                             _bdy_width[i - 1].back());
        }
        // get lane width
        auto get_lane_width = [](const std::vector<double>& lane_width_front,
                                 const std::vector<double>& lane_width_back) {
          std::vector<double> ret;
          for (int i = 0; i < lane_width_front.size(); ++i) {
            ret.push_back(lane_width_back.at(i) - lane_width_front.at(i));
          }
          return ret;
        };

        int cur_bounary_index = i;
        if (_sec[i]->id() > 0) {
          cur_bounary_index += 1;
        }

        txLanePtr _lane_ptr;
        generateLanePtr(_cur_road_ptr->id(), sectionpkid(_sec_i), _sec[i]->id(), _sec[i]->type(), _sec[i]->speedlimit(),
                        lanewidth, _sec[i]->surface(), _sec[i]->friction(), _sec[i]->lane_material_sOffset, _lane_geom,
                        _sec_ele, get_lane_width(_bdy_width[cur_bounary_index], _bdy_width[cur_bounary_index - 1]),
                        _lane_ptr);

        if (!_bound_ptrs.empty()) {
          if (_sec[i]->id() < 0) {
            _lane_ptr->setLeftBoundary(_bound_ptrs[i - 1]);
            _lane_ptr->setRightBoundary(_bound_ptrs[i]);
          } else {
            _lane_ptr->setLeftBoundary(_bound_ptrs[i]);
            _lane_ptr->setRightBoundary(_bound_ptrs[i + 1]);
          }
        }

        if (_lane_ptr->getSpeedLimit() == 0) {
          _lane_ptr->setSpeedLimit(_cur_road_ptr->speedlimit());
        }

        _cur_sec_ptr->add(_lane_ptr);
        _cur_sec_ptr->bindSectionPtr();
      }

      double startf = 0.0, length = 0.0;
      _cur_road_ptr->getSectionRange(_sec_i, startf, length);
      length = length - startf;
      // set mean slope
      _cur_sec_ptr->setMeanSlope(GetMeanSlope(_tx_road_ptr, startf, length));
      // set mean curvature;
      _cur_sec_ptr->setMeanCurvature(GetMeanCurvature(_tx_road_ptr, startf, length));
      _sections.push_back(_cur_sec_ptr);
    }
    _tx_road_ptr->setSections(_sections);
    _tx_road_ptr->bindRoadPtr();
    mutex.lock();
    tx_roads.insert(std::make_pair(_tx_road_ptr->getId(), _tx_road_ptr));
    mutex.unlock();
  } else {
    assert(0);
  }

  // generate object
  std::vector<ODRoad::Object> objs;
  _cur_road_ptr->getObjects(objs);
  txObjects objects;
  for (size_t i = 0; i < objs.size(); i++) {
    txObjectPtr objPtr;
    parseObject(objs[i], objPtr, _west, _south);
    objPtr->setOdRoadId(_cur_road_ptr->id());
    if (objPtr) {
      std::vector<txLaneId> rlanes;
      objPtr->getReliedLaneIds(rlanes);
      for (int lid : objs[i].validityLane) {
        sectionpkid sectionid = SECT_PKID_INVALID;
        if (_tx_road_ptr) {
          sectionid = _cur_road_ptr->getSectionIdx(objs[i].s);
        }
        rlanes.push_back(txLaneId(_cur_road_id, sectionid, lid));
      }
      objPtr->setReliedLaneIds(rlanes);
      objects.push_back(objPtr);
    }
  }

  mutex.lock();
  tx_objects.insert(std::make_pair(_cur_road_ptr->id(), objects));
  mutex.unlock();
  return true;
}

bool ODParse::DoLink(const hadmap::ODRoadPtr ptr) {
  ODRoadPtr _cur_road_ptr = ptr;
  roadpkid _cur_road_id = _cur_road_ptr->id();
  double invalid_double = std::numeric_limits<double>::max();

  auto distanceBetweenP = [](const txPoint& p0, const txPoint& p1) {
    double x1, y1, z1, x2, y2, z2;
    x1 = p0.x;
    y1 = p0.y;
    z1 = p0.z;
    x2 = p1.x;
    y2 = p1.y;
    z2 = p1.z;
    return std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2) + std::pow(z1 - z2, 2));
  };
  auto hook_up_lines = [&](const PointVec& line, const PointVec& base, double fix_len) {
    assert(line.size() == base.size());
    PointVec nline = line;
    const double equal_dis = 0.5;
    double pass_d = 0;
    const double pi2 = 1.5707963267948966192313216916398;
    const double e = 2.7182818284590452353602874713527;
    for (size_t k = 0; k < line.size(); k++) {
      if (k > 0) {
        pass_d += distanceBetweenP(line[k], line[k - 1]);
      }
      if (pass_d >= fix_len) {
        break;
      }
      const auto& p1 = line[k];
      const auto& p0 = base[k];
      double d = pass_d * 10 / fix_len - 5;
      double alpha = 1 / (1 + std::pow(e, -d));
      double x = p1.x - p0.x;
      double y = p1.y - p0.y;
      double z = p1.z - p0.z;
      x *= alpha;
      y *= alpha;
      z *= alpha;
      nline[k].x = p0.x + x;
      nline[k].y = p0.y + y;
      nline[k].z = p0.z + z;
    }
    return nline;
  };
  auto get_lane_point = [&](const txLaneId& laneid, bool front) {
    txPoint p(invalid_double, invalid_double, invalid_double);
    if (tx_roads.find(laneid.roadId) != tx_roads.end()) {
      txRoadPtr road_ptr = tx_roads[laneid.roadId];
      const auto& lanes = road_ptr->getSections().at(laneid.sectionId)->getLanes();
      auto lane_it =
          std::find_if(lanes.begin(), lanes.end(), [&](const txLanePtr& l) { return l->getId() == laneid.laneId; });
      if (lane_it != lanes.end()) {
        txLanePtr lane = *lane_it;
        if (lane->getGeometry() && !lane->getGeometry()->empty()) {
          if (front) {
            p = lane->getGeometry()->getStart();
          } else {
            p = lane->getGeometry()->getEnd();
          }
        }
      }
    }
    return p;
  };
  if (tx_roads.find(_cur_road_id) != tx_roads.end()) {
    if (_cur_road_ptr->junctionId() > 0) {
      if (_cur_road_ptr->preRoadId() == ROAD_PKID_INVALID || _cur_road_ptr->succRoadId() == ROAD_PKID_INVALID) {
        mutex.lock();
        to_erase_ids.push_back(_cur_road_id);
        mutex.unlock();
      } else if (tx_roads.find(_cur_road_id) != tx_roads.end()) {
        txRoadPtr road_ptr = tx_roads[_cur_road_id];

        roadpkid from_rid = _cur_road_ptr->preRoadId();
        roadpkid to_rid = _cur_road_ptr->succRoadId();

        if (tx_roads.find(from_rid) != tx_roads.end() && tx_roads.find(to_rid) != tx_roads.end() &&
            !tx_roads[from_rid]->getSections().empty() && !tx_roads[to_rid]->getSections().empty()) {
          sectionpkid from_sid =
              _cur_road_ptr->preRoadCont() == ODLink::START ? 0 : tx_roads[from_rid]->getSections().size() - 1;
          sectionpkid to_sid =
              _cur_road_ptr->succRoadCont() == ODLink::START ? 0 : tx_roads[to_rid]->getSections().size() - 1;

          std::vector<std::vector<lanepkid>> lanelinks;
          _cur_road_ptr->links(lanelinks);

          std::map<laneboundarypkid, txLaneBoundaryPtr> linkBds;

          txCurvatureVec vec = road_ptr->getCurvature();
          txSlopeVec slope = road_ptr->getSlope();

          for (auto& lanelink : lanelinks) {
            roadpkid _from_rid = from_rid;
            roadpkid _to_rid = to_rid;
            sectionpkid _from_sid = from_sid;
            sectionpkid _to_sid = to_sid;
            lanepkid _from_lid = lanelink.front();
            lanepkid _to_lid = lanelink.back();
            txLaneLinkPtr link_ptr(new txLaneLink);
            assert(road_ptr->getSections().size() + 2 == lanelink.size());
            PointVec Apoints;
            CoordType coordtype(COORD_ENU);
            txLaneBoundaries leftBds;
            txLaneBoundaries rightBds;
            txContactType _preCon = (txContactType)_cur_road_ptr->preRoadCont();
            txContactType _toCon = (txContactType)_cur_road_ptr->succRoadCont();
            for (size_t i = 1, is = lanelink.size() - 1; i < is; i++) {
              txLanes lanes = road_ptr->getSections().at(i - 1)->getLanes();
              for (auto& lane_ptr : lanes) {
                if (lane_ptr->getId() != lanelink[i]) continue;
                PointVec points;
                dynamic_cast<const txLineCurve*>(lane_ptr->getGeometry())->getPoints(points);
                Apoints.insert(Apoints.end(), points.begin(), points.end());
                coordtype = lane_ptr->getGeometry()->getCoordType();

                txLaneBoundaryPtr l_bdy;
                if (lane_ptr->getLeftBoundary()) {
                  auto id = lane_ptr->getLeftBoundary()->getId();
                  if (linkBds.find(id) == linkBds.end()) {
                    linkBds.insert(std::make_pair(id, std::make_shared<txLaneBoundary>(*lane_ptr->getLeftBoundary())));
                  }
                  l_bdy = linkBds[id];
                }
                txLaneBoundaryPtr r_bdy;
                if (lane_ptr->getRightBoundary()) {
                  auto id = lane_ptr->getRightBoundary()->getId();
                  if (linkBds.find(id) == linkBds.end()) {
                    linkBds.insert(std::make_pair(id, std::make_shared<txLaneBoundary>(*lane_ptr->getRightBoundary())));
                  }
                  r_bdy = linkBds[id];
                }

                leftBds.push_back(l_bdy);
                rightBds.push_back(r_bdy);
                break;
              }
            }
            bool revs = lanelink.size() > 2 && lanelink[1] > 0;
            if (revs) {
              std::swap(_from_rid, _to_rid);
              std::swap(_from_sid, _to_sid);
              std::swap(_from_lid, _to_lid);
              std::reverse(Apoints.begin(), Apoints.end());
              auto ReservePoints = [](const txCurve* curve) -> PointVec {
                PointVec points;
                if (curve) dynamic_cast<const txLineCurve*>(curve)->getPoints(points);
                std::reverse(points.begin(), points.end());
                return points;
              };
              for (auto bd : leftBds) {
                if (bd) bd->setGeometry(ReservePoints(bd->getGeometry()), bd->getGeometry()->getCoordType());
              }
              for (auto bd : rightBds) {
                if (bd) bd->setGeometry(ReservePoints(bd->getGeometry()), bd->getGeometry()->getCoordType());
              }
              std::swap(_preCon, _toCon);
              std::reverse(leftBds.begin(), leftBds.end());
              std::reverse(rightBds.begin(), rightBds.end());
              std::swap(leftBds, rightBds);
            }

            (*link_ptr)
                .setId(0)
                .setFromRoadId(_from_rid)
                .setFromSectionId(_from_sid)
                .setFromLaneId(_from_lid)
                .setToRoadId(_to_rid)
                .setToSectionId(_to_sid)
                .setToLaneId(_to_lid)
                .setJunctionId(_cur_road_ptr->junctionId())
                .setLeftBoundaries(leftBds)
                .setRightBoundaries(rightBds)
                .setOdrRoadId(_cur_road_id)
                .setPreContact(_preCon)
                .setSuccContact(_toCon);
            if (this->od_header.getVendor() == "tadsim v2.0") {
              txControlPoint point;
              std::vector<std::vector<double>> _controlPoint;
              _cur_road_ptr->roadControlPoint(point.m_type, _controlPoint);
              for (auto it : _controlPoint) {
                txPoint tmp(it.at(0), it.at(1), 0);
                tmp.hdg = it.at(2);
                point.m_points.push_back(tmp);
              }
              link_ptr->setControlPoint(point);
            }
            // txlog::debug("generate link id = " + link_ptr->getId());

            // set ele control
            std::vector<std::vector<double>> points;
            _cur_road_ptr->roadEleControlPoint(points, _cur_road_ptr->length(), this->od_header.getVendor());
            txControlPoint cPoint;
            for (auto it : points) {
              hadmap::txPoint ptA;
              ptA.x = it.at(0);
              ptA.y = it.at(1);
              ptA.h = it.at(2);
              cPoint.m_points.push_back(ptA);
            }
            if (cPoint.m_points.size() > 0) {
              cPoint.m_type = "catmullrom";
              link_ptr->setEleControlPoint(cPoint);
            }
            link_ptr->setGeometry(Apoints, coordtype);
            link_ptr->setSlope(slope);
            link_ptr->setCurvature(vec);
            char _index[64];
            sprintf(_index, "%s_%s", std::to_string(_from_rid).c_str(), std::to_string(_to_rid).c_str());
            std::string _link_index(_index);
            mutex.lock();
            auto& links = tx_links[_link_index];
            bool exist = false;
            for (const auto& link : links) {
              if (link->fromRoadId() == _from_rid && link->toRoadId() == _to_rid &&
                  link->fromSectionId() == _from_sid && link->toSectionId() == _to_sid &&
                  link->fromLaneId() == _from_lid && link->toLaneId() == _to_lid) {
                exist = true;
                break;
              }
            }
            if (!exist) {
              link_ptr->setId(_link_pkid++);
              links.push_back(link_ptr);
              linkofjunction.insert(std::make_pair(link_ptr->getId(), _cur_road_ptr->junctionId()));
            }
            mutex.unlock();
          }
        }

        mutex.lock();
        to_erase_ids.push_back(_cur_road_id);
        mutex.unlock();
      }
    } else {
      // zero len road
      txLaneLinks links;
      generateRoadLinks(_cur_road_ptr, links);
      if (!links.empty()) {
        mutex.lock();

        for (auto link_ptr : links) {
          char _index[64];
          sprintf(_index, "%s_%s", std::to_string(link_ptr->fromRoadId()).c_str(),
                  std::to_string(link_ptr->toRoadId()).c_str());
          std::string _link_index(_index);
          auto& links = tx_links[_link_index];
          bool exist = false;
          for (const auto& link : links) {
            if (*link == *link_ptr) {
              exist = true;
              break;
            }
          }
          if (!exist) {
            if (tx_roads.find(_cur_road_id) != tx_roads.end()) {
              txRoadPtr road_ptr = tx_roads[_cur_road_id];
              link_ptr->setSlope(road_ptr->getSlope());
              link_ptr->setCurvature(road_ptr->getCurvature());
            }
            link_ptr->setId(_link_pkid++);
            links.push_back(link_ptr);
          }
        }
        mutex.unlock();
        const double fix_len = 20;
        std::map<txLaneId, std::set<txLaneId>> prevLanelinksbuf;
        std::map<txLaneId, std::set<txLaneId>> nextLanelinksbuf;
        for (const auto& link_ptr : links) {
          if (link_ptr->fromRoadId() != _cur_road_id) {
            prevLanelinksbuf[link_ptr->fromTxLaneId()].insert(link_ptr->toTxLaneId());
          } else if (link_ptr->toRoadId() != _cur_road_id) {
            nextLanelinksbuf[link_ptr->toTxLaneId()].insert(link_ptr->fromTxLaneId());
          }
        }
        for (const auto& prev : prevLanelinksbuf) {
          if (prev.second.size() <= 1) {
            continue;
          }
          auto endP = get_lane_point(prev.first, false);
          if (endP.x == invalid_double) {
            continue;
          }
          txLaneId nestest(ROAD_PKID_INVALID, SECT_PKID_INVALID, LANE_PKID_INVALID);
          double neatestDis = invalid_double;
          for (const auto& l : prev.second) {
            auto startP = get_lane_point(l, true);
            if (startP.x == invalid_double) {
              continue;
            }
            double d = distanceBetweenP(startP, endP);
            if (d < neatestDis) {
              neatestDis = d;
              nestest = l;
            }
          }
          if (nestest.roadId != ROAD_PKID_INVALID && neatestDis < 0.5) {
            txRoadPtr road_ptr = tx_roads[nestest.roadId];
            const auto& lanes = road_ptr->getSections().at(nestest.sectionId)->getLanes();
            const txLineCurve* geom =
                dynamic_cast<const txLineCurve*>((*std::find_if(lanes.begin(), lanes.end(), [&](const txLanePtr& l) {
                                                   return l->getId() == nestest.laneId;
                                                 }))->getGeometry());
            PointVec pts;
            geom->getPoints(pts);
            for (const auto& cl : prev.second) {
              if (cl.roadId != nestest.roadId || cl.sectionId != nestest.sectionId || cl.laneId == nestest.laneId) {
                continue;
              }
              auto lane_it =
                  std::find_if(lanes.begin(), lanes.end(), [&](const txLanePtr& l) { return l->getId() == cl.laneId; });
              auto leftcl = cl;
              leftcl.laneId += 1;
              auto lane_it_left = std::find_if(lanes.begin(), lanes.end(),
                                               [&](const txLanePtr& l) { return l->getId() == leftcl.laneId; });
              if (lane_it_left != lanes.end() && (*lane_it_left)->getLaneType() == LANE_TYPE_None) continue;
              auto rightcl = cl;
              rightcl.laneId -= 1;
              auto lane_it_right = std::find_if(lanes.begin(), lanes.end(),
                                                [&](const txLanePtr& l) { return l->getId() == rightcl.laneId; });
              if (lane_it_right != lanes.end() && (*lane_it_right)->getLaneType() == LANE_TYPE_None) continue;
              if (lane_it != lanes.end()) {
                if ((*lane_it)->getLaneType() == LANE_TYPE_None) continue;
                const txLineCurve* cg = dynamic_cast<const txLineCurve*>((*lane_it)->getGeometry());
                if (cg) {
                  PointVec cpts;
                  cg->getPoints(cpts);
                  assert(cpts.size() == pts.size());
                  const double fix_dis = std::min(fix_len, cg->getLength());
                  auto nline = hook_up_lines(cpts, pts, fix_dis);
                  (*lane_it)->setGeometry(nline, cg->getCoordType());
                }
              }
            }
          }
        }
        for (const auto& next : nextLanelinksbuf) {
          if (next.second.size() <= 1) {
            continue;
          }
          auto startP = get_lane_point(next.first, true);
          if (startP.x == invalid_double) {
            continue;
          }
          txLaneId nestest(ROAD_PKID_INVALID, SECT_PKID_INVALID, LANE_PKID_INVALID);
          double neatestDis = invalid_double;
          for (const auto& l : next.second) {
            auto endP = get_lane_point(l, false);
            if (endP.x == invalid_double) {
              continue;
            }
            double d = distanceBetweenP(startP, endP);
            if (d < neatestDis) {
              neatestDis = d;
              nestest = l;
            }
          }
          if (nestest.roadId != ROAD_PKID_INVALID && neatestDis < 0.5) {
            txRoadPtr road_ptr = tx_roads[nestest.roadId];
            const auto& lanes = road_ptr->getSections().at(nestest.sectionId)->getLanes();
            const txLineCurve* geom =
                dynamic_cast<const txLineCurve*>((*std::find_if(lanes.begin(), lanes.end(), [&](const txLanePtr& l) {
                                                   return l->getId() == nestest.laneId;
                                                 }))->getGeometry());
            PointVec pts;
            geom->getPoints(pts);
            std::reverse(pts.begin(), pts.end());
            for (const auto& cl : next.second) {
              if (cl.roadId != nestest.roadId || cl.sectionId != nestest.sectionId || cl.laneId == nestest.laneId) {
                continue;
              }
              auto lane_it =
                  std::find_if(lanes.begin(), lanes.end(), [&](const txLanePtr& l) { return l->getId() == cl.laneId; });
              auto leftcl = cl;
              leftcl.laneId += 1;
              auto lane_it_left = std::find_if(lanes.begin(), lanes.end(),
                                               [&](const txLanePtr& l) { return l->getId() == leftcl.laneId; });
              if (lane_it_left != lanes.end() && (*lane_it_left)->getLaneType() == LANE_TYPE_None) continue;
              auto rightcl = cl;
              rightcl.laneId -= 1;
              auto lane_it_right = std::find_if(lanes.begin(), lanes.end(),
                                                [&](const txLanePtr& l) { return l->getId() == rightcl.laneId; });
              if (lane_it_right != lanes.end() && (*lane_it_right)->getLaneType() == LANE_TYPE_None) continue;

              if (lane_it != lanes.end()) {
                if ((*lane_it)->getLaneType() == LANE_TYPE_None) continue;
                const txLineCurve* cg = dynamic_cast<const txLineCurve*>((*lane_it)->getGeometry());
                if (cg) {
                  PointVec cpts;
                  cg->getPoints(cpts);
                  std::reverse(cpts.begin(), cpts.end());
                  assert(cpts.size() == pts.size());
                  const double fix_dis = std::min(fix_len, cg->getLength());
                  auto nline = hook_up_lines(cpts, pts, fix_dis);
                  std::reverse(nline.begin(), nline.end());
                  (*lane_it)->setGeometry(nline, cg->getCoordType());
                }
              }
            }
          }
        }
      }
    }
  }

  return true;
}

double ODParse::GetMeanCurvature(const txRoadPtr _tx_road_ptr, const double startf, const double length) {
  double curvatureSum = 0;
  double lengthSum2 = 0;
  for (size_t m = 0; m < _tx_road_ptr->getCurvature().size(); m++) {
    double tmpLength = 0.0;
    txCurvature curvature = _tx_road_ptr->getCurvature().at(m);
    if (curvature.m_starts < startf && (curvature.m_starts + curvature.m_length < startf + length)) {
      continue;
    }
    if (curvature.m_starts > startf && (curvature.m_starts + curvature.m_length > startf + length)) {
      continue;
    }
    tmpLength = curvature.m_length;
    if (curvature.m_starts < startf) {
      tmpLength = curvature.m_length - (startf - curvature.m_starts);
    }
    if ((curvature.m_starts + curvature.m_length > startf + length)) {
      tmpLength = curvature.m_length - (curvature.m_starts + curvature.m_length - startf - length);
    }
    curvatureSum += (curvature.m_curvature * tmpLength);
    lengthSum2 += tmpLength;
  }
  double meanCurvature = 0.0;
  if (lengthSum2 != 0) meanCurvature = curvatureSum / lengthSum2;
  return meanCurvature;
}

double ODParse::GetMeanSlope(const txRoadPtr _tx_road_ptr, const double startf, const double length) {
  double slopeSum = 0;
  double lengthSum = 0;
  for (size_t m = 0; m < _tx_road_ptr->getSlope().size(); m++) {
    double tmpLength = 0.0;
    txSlope slope = _tx_road_ptr->getSlope().at(m);
    if (slope.m_starts < startf && (slope.m_starts + slope.m_length < startf + length)) {
      continue;
    }
    if (slope.m_starts > startf && (slope.m_starts + slope.m_length > startf + length)) {
      continue;
    }
    tmpLength = slope.m_length;
    if (slope.m_starts < startf) {
      tmpLength = slope.m_length - (startf - slope.m_starts);
    }
    if ((slope.m_starts + slope.m_length > startf + length)) {
      tmpLength = slope.m_length - (slope.m_starts + slope.m_length - startf - length);
    }
    slopeSum += (slope.m_slope * tmpLength);
    lengthSum += tmpLength;
  }
  double meanSlope = 0.0;
  if (lengthSum != 0) meanSlope = slopeSum / lengthSum;
  return meanSlope;
}

txOpenCrgVec ODParse::GetOpenCrg(std::vector<ODCrg> odcrg) {
  txOpenCrgVec crgvec;
  for (auto itCrg : odcrg) {
    txOpenCrg _tx_crg;
    _tx_crg.m_file = itCrg.getFile();
    _tx_crg.m_orientation = itCrg.getOrientation();
    _tx_crg.m_mode = itCrg.getMode();
    _tx_crg.m_purpose = itCrg.getPurpose();
    _tx_crg.m_soffset = itCrg.getSOffset();
    _tx_crg.m_toffset = itCrg.getTOffset();
    _tx_crg.m_zoffset = itCrg.getZOffset();
    _tx_crg.m_zscale = itCrg.getZScale();
    _tx_crg.m_hoffset = itCrg.getHOffset();
    crgvec.push_back(_tx_crg);
  }
  return crgvec;
}

ROAD_TYPE ODParse::GetRoadType(std::string roadType) {
  ROAD_TYPE eType;
  if (roadType == "rural") {
    eType = ROAD_TYPE_ODR_Rural;
  } else if (roadType == "motorway") {
    eType = ROAD_TYPE_ODR_Motorway;
  } else if (roadType == "town") {
    eType = ROAD_TYPE_ODR_Town;
  } else if (roadType == "lowspeed") {
    eType = ROAD_TYPE_ODR_LowSpeed;
  } else if (roadType == "pedestrian") {
    eType = ROAD_TYPE_ODR_Pedestrian;
  } else {
    eType = ROAD_TYPE_Normal;
  }
  return eType;
}

}  // namespace hadmap
