// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "transmission/json_parser.h"

#include <algorithm>
#include <unordered_map>

#include "transmission/CJsonObject.hpp"

namespace hadmap {
bool JsonParser::parseRoad(const std::string& jsonStr, txRoads& roads) {
  neb::CJsonObject jP;
  jP.Parse(jsonStr);

  neb::CJsonObject& jRoads = jP["road"];
  size_t roadSize = jRoads.GetArraySize();
  for (size_t i = 0; i < roadSize; ++i) {
    uint32_t roadType;
    jRoads[i].Get("type", roadType);

    std::string ridStr;
    jRoads[i].Get("roadid", ridStr);
    roadpkid rid = (roadpkid)std::stoll(ridStr);

    neb::CJsonObject& geoms = jRoads[i]["geom"];
    PointVec curve;
    size_t geomSize = geoms.GetArraySize();
    for (size_t j = 0; j < geomSize; ++j) {
      txPoint p;
      geoms[j].Get("x", p.x);
      geoms[j].Get("y", p.y);
      geoms[j].Get("z", p.z);
      curve.push_back(p);
    }

    txRoadPtr roadPtr(new txRoad);
    roadPtr->setId(rid).setRoadType(ROAD_TYPE(roadType)).setGeometry(curve, COORD_WGS84);

    roads.push_back(roadPtr);
  }
  return true;
}

bool JsonParser::parseSection(const std::string& jsonStr, txSections& sections) {
  txLanes lanes;
  txLaneBoundaries boundaries;
  if (JsonParser::parseLane(jsonStr, lanes) && JsonParser::parseBoundary(jsonStr, boundaries)) {
    std::unordered_map<laneboundarypkid, size_t> boundaryMap;
    for (size_t i = 0; i < boundaries.size(); ++i) boundaryMap.insert(std::make_pair(boundaries[i]->getId(), i));

    std::map<roadpkid, std::map<sectionpkid, txLanes> > laneMap;
    for (auto& curLanePtr : lanes) {
      roadpkid rid = curLanePtr->getRoadId();
      sectionpkid sid = curLanePtr->getSectionId();
      if (laneMap.find(rid) == laneMap.end()) {
        laneMap.insert(std::make_pair(rid, std::map<sectionpkid, txLanes>()));
        if (laneMap[rid].find(sid) == laneMap[rid].end()) laneMap[rid].insert(std::make_pair(sid, txLanes()));
      }
      curLanePtr->setLeftBoundary(boundaries[boundaryMap[curLanePtr->getLeftBoundaryId()]]);
      curLanePtr->setRightBoundary(boundaries[boundaryMap[curLanePtr->getRightBoundaryId()]]);
      laneMap[rid][sid].push_back(curLanePtr);
    }

    for (auto itr = laneMap.begin(); itr != laneMap.end(); ++itr) {
      for (auto sitr = itr->second.begin(); sitr != itr->second.end(); ++sitr) {
        std::sort(sitr->second.begin(), sitr->second.end(),
                  [](txLanePtr a, txLanePtr b) -> bool { return a->getId() > b->getId(); });

        txSectionPtr secPtr(new txSection);
        secPtr->setId(sitr->first).setRoadId(itr->first).setLanes(sitr->second);
        sections.push_back(secPtr);
      }
    }

    return !sections.empty();
  } else {
    return false;
  }
}

bool JsonParser::parseLane(const std::string& jsonStr, txLanes& lanes) {
  neb::CJsonObject jP;
  jP.Parse(jsonStr);

  neb::CJsonObject& jLanes = jP["lane"];
  size_t laneSize = jLanes.GetArraySize();
  for (size_t i = 0; i < laneSize; ++i) {
    uint32_t arrow;
    jLanes[i].Get("arr", arrow);

    std::string ridStr, sidStr, lidStr, lbidStr, rbidStr;
    jLanes[i].Get("rid", ridStr);
    jLanes[i].Get("sid", sidStr);
    jLanes[i].Get("id", lidStr);
    jLanes[i].Get("lbid", lbidStr);
    jLanes[i].Get("rbid", rbidStr);
    roadpkid rid = roadpkid(std::stoll(ridStr));
    sectionpkid sid = sectionpkid(std::stoll(sidStr));
    lanepkid lid = lanepkid(std::stoll(lidStr));
    laneboundarypkid lbid = laneboundarypkid(std::stoll(lbidStr));
    laneboundarypkid rbid = laneboundarypkid(std::stoll(rbidStr));

    uint32_t splimit;
    jLanes[i].Get("sl", splimit);

    neb::CJsonObject& geoms = jLanes[i]["geom"];
    PointVec curve;
    size_t geomSize = geoms.GetArraySize();
    for (size_t j = 0; j < geomSize; ++j) {
      txPoint p;
      geoms[j].Get("x", p.x);
      geoms[j].Get("y", p.y);
      geoms[j].Get("z", p.z);
      curve.push_back(p);
    }

    txLanePtr lanePtr(new txLane);
    lanePtr->setRoadId(rid)
        .setSectionId(sid)
        .setId(lid)
        .setLeftBoundaryId(lbid)
        .setRightBoundaryId(rbid)
        .setSpeedLimit(splimit)
        .setLaneArrow(LANE_ARROW(arrow))
        .setGeometry(curve, COORD_WGS84);

    lanes.push_back(lanePtr);
  }
  return true;
}

bool JsonParser::parseBoundary(const std::string& jsonStr, txLaneBoundaries& boundaries) {
  neb::CJsonObject jP;
  jP.Parse(jsonStr);

  neb::CJsonObject& jBoundaries = jP["boundary"];
  size_t boundarySize = jBoundaries.GetArraySize();
  for (size_t i = 0; i < boundarySize; ++i) {
    std::string id;
    jBoundaries[i].Get("id", id);
    laneboundarypkid boundaryId = laneboundarypkid(std::stoll(id));

    uint32_t mrk;
    jBoundaries[i].Get("mark", mrk);

    neb::CJsonObject& geoms = jBoundaries[i]["geom"];
    PointVec curve;
    size_t geomSize = geoms.GetArraySize();
    for (size_t j = 0; j < geomSize; ++j) {
      txPoint p;
      geoms[j].Get("x", p.x);
      geoms[j].Get("y", p.y);
      geoms[j].Get("z", p.z);
      curve.push_back(p);
    }

    txLaneBoundaryPtr boundaryPtr(new txLaneBoundary);
    boundaryPtr->setId(boundaryId).setLaneMark(LANE_MARK(mrk)).setGeometry(curve, COORD_WGS84);

    boundaries.push_back(boundaryPtr);
  }
  return true;
}

bool JsonParser::parseLink(const std::string& jsonStr, txLaneLinks& links) {
  neb::CJsonObject jP;
  jP.Parse(jsonStr);

  neb::CJsonObject& jLinks = jP["link"];
  size_t linkSize = jLinks.GetArraySize();
  for (size_t i = 0; i < linkSize; ++i) {
    std::string id;
    jLinks[i].Get("id", id);
    lanelinkpkid linkId = lanelinkpkid(std::stoll(id));

    std::string fridStr, fsidStr, flidStr, tridStr, tsidStr, tlidStr;
    jLinks[i].Get("frid", fridStr);
    jLinks[i].Get("fsid", fsidStr);
    jLinks[i].Get("fid", flidStr);
    jLinks[i].Get("trid", tridStr);
    jLinks[i].Get("tsid", tsidStr);
    jLinks[i].Get("tid", tlidStr);
    roadpkid frid = roadpkid(std::stoll(fridStr));
    sectionpkid fsid = sectionpkid(std::stoll(fsidStr));
    lanepkid flid = lanepkid(std::stoll(flidStr));
    roadpkid trid = roadpkid(std::stoll(tridStr));
    sectionpkid tsid = sectionpkid(std::stoll(tsidStr));
    lanepkid tlid = lanepkid(std::stoll(tlidStr));

    neb::CJsonObject& geoms = jLinks[i]["geom"];
    PointVec curve;
    size_t geomSize = geoms.GetArraySize();
    for (size_t j = 0; j < geomSize; ++j) {
      txPoint p;
      geoms[j].Get("x", p.x);
      geoms[j].Get("y", p.y);
      geoms[j].Get("z", p.z);
      curve.push_back(p);
    }

    txLaneLinkPtr linkPtr(new txLaneLink);
    linkPtr->setId(linkId)
        .setFromRoadId(frid)
        .setFromSectionId(fsid)
        .setFromLaneId(flid)
        .setToRoadId(trid)
        .setToSectionId(tsid)
        .setToLaneId(tlid)
        .setGeometry(curve, COORD_WGS84);

    links.push_back(linkPtr);
  }
  return true;
}

bool JsonParser::parseObj(const std::string& jsonStr, txObjects& objects) {
  neb::CJsonObject jP;
  jP.Parse(jsonStr);

  neb::CJsonObject& jObjects = jP["object"];
  size_t objectSize = jObjects.GetArraySize();
  for (size_t i = 0; i < objectSize; ++i) {
    std::string id;
    jObjects[i].Get("id", id);
    objectpkid objId = objectpkid(std::stoll(id));

    uint32_t type;
    jObjects[i].Get("type", type);

    neb::CJsonObject& relIds = jObjects[i]["relids"];
    neb::CJsonObject& geoms = jObjects[i]["geom"];

    PointVec curve;
    size_t geomSize = geoms.GetArraySize();
    for (size_t i = 0; i < geomSize; ++i) {
      txPoint p;
      geoms[i].Get("x", p.x);
      geoms[i].Get("y", p.y);
      geoms[i].Get("z", p.z);
      curve.push_back(p);
    }

    std::vector<txLaneId> laneIds;
    size_t relIdSize = relIds.GetArraySize();
    for (size_t i = 0; i < relIdSize; ++i) {
      std::string ridStr, sidStr, lidStr;
      relIds[i].Get("rid", ridStr);
      relIds[i].Get("sid", sidStr);
      relIds[i].Get("lid", lidStr);
      roadpkid rid = roadpkid(std::stoll(ridStr));
      sectionpkid sid = sectionpkid(std::stoll(sidStr));
      lanepkid lid = lanepkid(std::stoll(lidStr));
      laneIds.push_back(txLaneId(rid, sid, lid));
    }

    txObjGeomPtr geomPtr(new txObjectGeom);
    geomPtr->setId(1)
        .setType(OBJECT_GEOMETRY_TYPE_Polyline)
        .setStyle(OBJECT_STYLE_Polyline)
        .setGeometry(curve, COORD_WGS84);

    txObjectPtr objPtr(new txObject);
    objPtr->setId(objId).setObjectType(OBJECT_TYPE(type)).setReliedLaneIds(laneIds).addGeom(geomPtr);

    objects.push_back(objPtr);
  }
  return true;
}

bool JsonParser::parseRoute(const std::string& jsonStr, txRoute& route) {
  neb::CJsonObject jP;
  jP.Parse(jsonStr);

  neb::CJsonObject jRoute = jP["route"];
  size_t routeSize = jRoute.GetArraySize();
  for (size_t i = 0; i < routeSize; ++i) {
    uint32 type;
    jRoute[i].Get("type", type);

    std::string idStr, preidStr, nexidStr;
    jRoute[i].Get("id", idStr);
    jRoute[i].Get("previd", preidStr);
    jRoute[i].Get("nextid", nexidStr);
    roadpkid id = roadpkid(std::stoull(idStr));
    roadpkid preid = roadpkid(std::stoull(preidStr));
    roadpkid nexid = roadpkid(std::stoull(nexidStr));

    double start, end;
    jRoute[i].Get("start", start);
    jRoute[i].Get("end", end);

    std::string attrStr;
    jRoute[i].Get("attr", attrStr);
    uint64_t attr = uint64_t(std::stoull(attrStr));

    bool reverse;
    jRoute[i].Get("reverse", reverse);

    txRouteNode node;
    node.setRouteType(txRouteNode::RouteType(type))
        .setId(id)
        .setPreId(preid)
        .setNextId(nexid)
        .setStartRange(start)
        .setEndRange(end)
        .setAttr(attr)
        .setReverse(reverse);

    route.push_back(node);
  }
  return true;
}
}  // namespace hadmap
