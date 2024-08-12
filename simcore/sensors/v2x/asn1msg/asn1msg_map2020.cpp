/**
 * @file asn1msg_map2020.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "asn1msg_map2020.h"
#include <glog/logging.h>
#include <google/protobuf/util/json_util.h>
#include <bitset>
#include <fstream>
#include <sstream>
#include "../hmap.h"
#include "../sensors.h"
#include "boost/filesystem.hpp"
#include "common/coord_trans.h"
#include "json/json.h"
#include "mapengine/hadmap_codes.h"
#include "merge_proto.h"
#include "read_xml.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_junction.h"
#include "structs/hadmap_lane.h"

/**
 * @brief build topo of the road network
 *
 */
void CAsn1MsgMAP2020::BuildTopo() {
  mapLinks.clear();
  mapNodes.clear();
  mapLanes.clear();
  mapConnects.clear();
  mapHdLaneToV2xLane.clear();
  mapHdRoadToV2xLink.clear();
  mapHdJunctionToV2XNode.clear();
  mapHdLinkToV2XConnect.clear();
  mapHdRid.clear();
  nodeCount.clear();

  // node: HdJuncid, HdRoadfrom, HdRoadto
  // connect: HdJuncid, HdLid
  hadmap::junctionpkid maxJcId = 0;
  std::set<hadmap::junctionpkid> temp_juncs;
  for (const auto &juc : mapJunctions) {
    temp_juncs.insert(juc->getId());
  }
  // get node map: based on junction id
  // build node id
  for (auto jid : temp_juncs) {
    auto id = mapNodes.size() + 1;
    mapNodes[id].HdJuncid = jid;
    mapHdJunctionToV2XNode[jid] = id;
    maxJcId = std::max(maxJcId, jid);
    std::cout << jid << "->" << id << ", ";
  }
  std::cout << std::endl;
  // build connect: lanelink to connect
  for (const auto &juc : mapJunctions) {
    std::vector<hadmap::lanelinkpkid> lids;
    juc->getLaneLink(lids);
    if (!lids.empty()) {
      V2xNode &node = mapNodes[mapHdJunctionToV2XNode[juc->getId()]];
      for (const auto &lid : lids) {
        hadmap::txLaneLinkPtr link;
        hadmap::getLaneLink(hMapHandle, lid, link);
        if (link.get()) {
          auto id = mapConnects.size() + 1;
          mapHdLinkToV2XConnect[link->getId()] = id;
          V2xConnect &vconn = mapConnects[id];
          vconn.HdLid = link->getId();
          vconn.HdJuncid = link->getJunctionId();
          node.HdRoadfrom.insert(link->fromRoadId());
          node.HdRoadto.insert(link->toRoadId());
        }
      }
    }
  }

  // link: roads, preN, nxtN
  // node: preLk, nxtLk
  for (auto &node : mapNodes) {
    // get previous road
    // road to link
    for (const auto &rid : node.second.HdRoadfrom) {
      std::vector<hadmap::roadpkid> rs = Topo_getFromRoad(rid);
      if (rs.empty()) {
        continue;
      }
      if (mapHdRoadToV2xLink.find(rid) == mapHdRoadToV2xLink.end()) {
        auto id = mapLinks.size() + 1;
        for (auto i : rs) {
          if (mapHdRoadToV2xLink.find(i) != mapHdRoadToV2xLink.end()) {
            std::cout << "map:BuildTopo: has error" << std::endl;
          }
          mapHdRoadToV2xLink[i] = id;
        }
        node.second.preLk.insert(id);

        V2xLink &vlink = mapLinks[id];
        vlink.roads = rs;
        vlink.nxtN = node.first;
      } else {
        node.second.preLk.insert(mapHdRoadToV2xLink.at(rid));
        mapLinks[mapHdRoadToV2xLink.at(rid)].nxtN = node.first;
      }
    }
    // get next road
    // road to link
    for (const auto &rid : node.second.HdRoadto) {
      std::vector<hadmap::roadpkid> rs = Topo_getToRoad(rid);
      if (rs.empty()) {
        continue;
      }
      if (mapHdRoadToV2xLink.find(rid) == mapHdRoadToV2xLink.end()) {
        auto id = mapLinks.size() + 1;
        for (auto i : rs) {
          if (mapHdRoadToV2xLink.find(i) != mapHdRoadToV2xLink.end()) {
            std::cout << "map:BuildTopo: has error" << std::endl;
          }
          mapHdRoadToV2xLink[i] = id;
        }
        node.second.nxtLk.insert(id);

        V2xLink &vlink = mapLinks[id];
        vlink.roads = rs;
        vlink.preN = node.first;
      } else {
        node.second.nxtLk.insert(mapHdRoadToV2xLink.at(rid));
        mapLinks[mapHdRoadToV2xLink.at(rid)].preN = node.first;
      }
    }
  }

  // lane: HdLanes, bLink
  // link: lanes
  for (auto &link : mapLinks) {
    size_t lanenum = mapLanes.size() + 1;
    for (auto rid : link.second.roads) {
      hadmap::txRoadPtr road;
      if (hadmap::getRoad(hMapHandle, rid, true, road) == TX_HADMAP_DATA_OK) {
        for (const auto &sec : road->getSections()) {
          for (const auto &l : sec->getLanes()) {
            auto &lane = mapLanes[lanenum + (-1 - l->getId())];
            lane.HdLanes.push_back(l->getTxLaneId());
            lane.bLink = link.first;
            mapHdLaneToV2xLane[l->getTxLaneId()] = lanenum + (-1 - l->getId());
          }
        }
      }
    }
    for (size_t i = lanenum; i <= mapLanes.size(); i++) {
      link.second.lanes.push_back(i);
    }
  }

  // connect: preLane, nxtLane
  for (auto &conn : mapConnects) {
    hadmap::txLaneLinkPtr link;
    hadmap::getLaneLink(hMapHandle, conn.second.HdLid, link);
    if (link && mapHdLaneToV2xLane.find(link->fromTxLaneId()) != mapHdLaneToV2xLane.end()) {
      conn.second.preLane = mapHdLaneToV2xLane.at(link->fromTxLaneId());
    } else {
      assert(0);
    }
    if (link && mapHdLaneToV2xLane.find(link->toTxLaneId()) != mapHdLaneToV2xLane.end()) {
      conn.second.nxtLane = mapHdLaneToV2xLane.at(link->toTxLaneId());
    } else {
      assert(0);
    }
  }

  // create boundray node
  for (const auto &link : mapLinks) {
    // create previous boundary nodes
    if (link.second.preN == 0) {
      auto id = mapNodes.size() + 1;
      V2xNode &node = mapNodes[id];
      node.HdJuncid = ++maxJcId;
      node.HdRoadto.insert(link.second.roads.front());
      node.nxtLk.insert(link.first);
    }
    // create next boundary nodes
    if (link.second.nxtN == 0) {
      auto id = mapNodes.size() + 1;
      V2xNode &node = mapNodes[id];
      node.HdJuncid = ++maxJcId;
      node.HdRoadfrom.insert(link.second.roads.back());
      node.preLk.insert(link.first);
    }
  }
}

/**
 * @brief build node
 *
 */
void CAsn1MsgMAP2020::BuildNode() {
  for (auto &node : mapNodes) {
    BuildNode(node.second);
  }
}

/**
 * @brief build link
 *
 */
void CAsn1MsgMAP2020::BuildLink() {
  for (auto &link : mapLinks) {
    BuildLink(link.second);
  }
}

/**
 * @brief build lane
 *
 */
void CAsn1MsgMAP2020::BuildLane() {
  for (auto &lane : mapLanes) {
    BuildLane(lane.second);
  }
}

/**
 * @brief build connect.
 *
 * @param scene_file
 */
void CAsn1MsgMAP2020::BuildConnect(const std::string &scene_file) {
  // build connect
  for (auto &conn : mapConnects) {
    BuildConnect(conn.second);
  }
  // load traffic light infomation
  namespace fs = boost::filesystem;
  fs::path scenario_file(scene_file);
  fs::path parent_path = scenario_file.parent_path();
  fs::path file_name = scenario_file.filename();
  fs::path stem = file_name.stem();
  stem += "_traffic.xml";
  fs::path traffic_file = parent_path / stem;
  std::string traffic_xml = traffic_file.lexically_normal().string();
  tinyxml2::XMLDocument doc;
  const tinyxml2::XMLElement *pRoot = GetXMLRoot(traffic_xml, doc);
  const tinyxml2::XMLElement *pRoutes = TryGetChildNode(pRoot, "routes");
  if (!pRoutes) return;
  std::map<std::string, hadmap::txPoint> route_lonlat;
  const tinyxml2::XMLElement *itemRoute = pRoutes->FirstChildElement();
  // get traffic light position
  while (itemRoute) {
    const char *pId = itemRoute->Attribute("id");
    const char *pStart = itemRoute->Attribute("start");
    if (pId && pStart) {
      std::stringstream input(pStart);
      std::string tmp;
      std::vector<double> data;
      while (std::getline(input, tmp, ',')) {
        data.push_back(std::atof(tmp.c_str()));
      }
      if (data.size() > 1) {
        route_lonlat[std::string(pId)] = hadmap::txPoint(data[0], data[1], data.size() > 2 ? data[2] : 0);
      }
    }
    itemRoute = itemRoute->NextSiblingElement();
  }
  const tinyxml2::XMLElement *pSignlights = TryGetChildNode(pRoot, "signlights");
  if (!pSignlights) return;
  std::string activatePlan = pSignlights->Attribute("activePlan");
  const tinyxml2::XMLElement *itemLight = pSignlights->FirstChildElement();
  // handle all light item, there are some invalid data
  while (itemLight) {
    std::string plan = itemLight->Attribute("plan");
    // only valid data will process here
    if (plan == activatePlan && std::string(itemLight->Attribute("status")) == "Activated") {
      auto fd = route_lonlat.find(std::string(itemLight->Attribute("routeID")));
      if (fd != route_lonlat.end()) {
        hadmap::txLanePtr lane;
        hadmap::getLane(hMapHandle, fd->second, lane);
        hadmap::txJunctionPtr juc;
        hadmap::getJunction(hMapHandle, std::atoll(itemLight->Attribute("junction")), juc);
        if (lane && juc) {
          int pid = std::atoi(itemLight->Attribute("phaseNumber"));
          std::stringstream input(itemLight->Attribute("lane"));
          std::string tmp;
          std::set<int> data;
          while (std::getline(input, tmp, ';')) {
            data.insert(std::atoi(tmp.c_str()));
          }
          std::set<std::string> phase;
          input = std::stringstream(itemLight->Attribute("phase"));
          while (std::getline(input, tmp, ';')) {
            phase.insert(tmp);
          }
          std::vector<hadmap::lanelinkpkid> lids;
          juc->getLaneLink(lids);
          for (const auto &lid : lids) {
            hadmap::txLaneLinkPtr link;
            hadmap::getLaneLink(hMapHandle, lid, link);
            if (link && link->fromRoadId() == lane->getRoadId() && data.find(link->fromLaneId()) != data.end()) {
              V2xConnect &vconn = mapConnects[mapHdLinkToV2XConnect[link->getId()]];
              int mane = vconn.connect.connectinglane().allowedmaneuvers();
              // set turn type
              // maneuverStraightAllowed=128
              // maneuverLeftAllowed=64
              // maneuverRightAllowed=32
              // maneuverUTurnAllowed=16
              // maneuverLeftTurnOnRedAllowed=8
              // maneuverRightTurnOnRedAllowed=4
              // maneuverLaneChangeAllowed=2
              // maneuverNoStoppingAllowed=1,
              // yieldAllwaysRequired=32768
              // goWithHalt=16384
              // caution=8192
              // reserved1=4096
              if (mane & 128) {
                // straight
                if (phase.find("T") != phase.end()) vconn.connect.set_phaseid(pid);
              } else if (mane & 64) {
                // left
                if (phase.find("L") != phase.end()) {
                  vconn.connect.set_phaseid(pid);
                  mane &= ~8;
                }
              } else if (mane & 32) {
                // right
                if (phase.find("R") != phase.end()) {
                  vconn.connect.set_phaseid(pid);
                  mane &= ~4;
                }
              } else if (mane & 16) {
                // uturn
                if (phase.find("L0") != phase.end()) vconn.connect.set_phaseid(pid);
              }
              vconn.connect.mutable_connectinglane()->set_allowedmaneuvers(mane);
            }
          }
        }
      }
    }
    itemLight = itemLight->NextSiblingElement();
  }
}

/**
 * @brief write map to geojson for debug
 *
 */
void CAsn1MsgMAP2020::WriteGeojson() {
  // write node
  {
    Json::Value geojson;
    geojson["type"] = "FeatureCollection";
    for (const auto &node : mapNodes) {
      Json::Value feature;
      feature["type"] = "Feature";
      feature["geometry"]["type"] = "Point";
      Json::Value jp;
      jp.append(node.second.node.position3d().longitude() * 1e-7);
      jp.append(node.second.node.position3d().latitude() * 1e-7);
      jp.append(node.second.node.position3d().elevation());
      feature["geometry"]["coordinates"] = jp;
      feature["properties"]["id"] = node.second.HdJuncid;
      for (auto p : node.second.preLk) {
        feature["properties"]["preLk"].append(p);
      }
      for (auto p : node.second.nxtLk) {
        feature["properties"]["nxtLk"].append(p);
      }
      feature["properties"]["msg"] = node.second.node.DebugString();
      geojson["features"].append(feature);
    }
    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream jf("e:/nodes.geojson");
    writer->write(geojson, &jf);
    jf.close();
  }
  // write link
  {
    Json::Value geojson;
    geojson["type"] = "FeatureCollection";
    for (const auto &lk : mapLinks) {
      Json::Value feature;
      feature["type"] = "Feature";
      feature["geometry"]["type"] = "LineString";
      for (const auto &p : lk.second.link.pointlist()) {
        Json::Value jp;
        jp.append(p.longitude() * 1e-7);
        jp.append(p.latitude() * 1e-7);
        jp.append(p.elevation());
        feature["geometry"]["coordinates"].append(jp);
      }
      feature["properties"]["id"] = lk.second.roads.front();
      feature["properties"]["preN"] = lk.second.preN;
      feature["properties"]["nxtN"] = lk.second.nxtN;
      for (auto p : lk.second.lanes) {
        feature["properties"]["lanes"].append(p);
      }
      feature["properties"]["msg"] = lk.second.link.DebugString();

      geojson["features"].append(feature);
    }
    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream jf("e:/links.geojson");
    writer->write(geojson, &jf);
    jf.close();
  }
  // write lane
  {
    Json::Value geojson;
    geojson["type"] = "FeatureCollection";
    for (const auto &ln : mapLanes) {
      Json::Value feature;
      feature["type"] = "Feature";
      feature["geometry"]["type"] = "LineString";
      for (const auto &p : ln.second.lane.pointlist()) {
        Json::Value jp;
        jp.append(p.longitude() * 1e-7);
        jp.append(p.latitude() * 1e-7);
        jp.append(p.elevation());
        feature["geometry"]["coordinates"].append(jp);
      }

      feature["properties"]["id"] = ln.second.HdLanes.front().laneId;
      feature["properties"]["msg"] = ln.second.lane.DebugString();
      geojson["features"].append(feature);
    }
    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream jf("e:/lanes.geojson");
    writer->write(geojson, &jf);
    jf.close();
  }
  // write junction
  {
    Json::Value geojson;
    geojson["type"] = "FeatureCollection";
    for (const auto &conn : mapConnects) {
      Json::Value feature;
      feature["type"] = "Feature";
      feature["geometry"]["type"] = "LineString";
      {
        const auto &lane = mapLanes.at(conn.second.preLane);
        if (lane.lane.pointlist().size() > 0) {
          Json::Value jp;
          jp.append(lane.lane.pointlist().rbegin()->longitude() * 1e-7);
          jp.append(lane.lane.pointlist().rbegin()->latitude() * 1e-7);
          jp.append(lane.lane.pointlist().rbegin()->elevation());
          feature["geometry"]["coordinates"].append(jp);
        }
      }
      {
        const auto &lane = mapLanes.at(conn.second.nxtLane);
        if (lane.lane.pointlist().size() > 0) {
          Json::Value jp;
          jp.append(lane.lane.pointlist().begin()->longitude() * 1e-7);
          jp.append(lane.lane.pointlist().begin()->latitude() * 1e-7);
          jp.append(lane.lane.pointlist().begin()->elevation());
          feature["geometry"]["coordinates"].append(jp);
        }
      }

      feature["properties"]["id"] = conn.second.HdLid;
      feature["properties"]["preLane"] = conn.second.preLane;
      feature["properties"]["nxtLane"] = conn.second.nxtLane;
      feature["properties"]["msg"] = conn.second.connect.DebugString();
      geojson["features"].append(feature);
    }
    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream jf("e:/connects.geojson");
    writer->write(geojson, &jf);
    jf.close();
  }
}

/**
 * @brief map message reset, callback by simulator
 *
 * @param helper helper of txsim
 */
void CAsn1MsgMAP2020::Reset(tx_sim::ResetHelper &helper) {
  mapLinks.clear();
  mapNodes.clear();
  mapLanes.clear();
  mapConnects.clear();
  mapHdJunctionToV2XNode.clear();
  mapHdLinkToV2XConnect.clear();
  mapHdRoadToV2xLink.clear();
  mapHdLaneToV2xLane.clear();
  mapHdRid.clear();
  nodeCount.clear();
  if (rsu_sensors.empty()) {
    LOG(INFO) << "skip map topo build.";
    return;
  }
  std::string scene_file = helper.scenario_file_path();
  // BUILD ALL MAP
  BuildTopo();               // topo, gen node id, lane id
  BuildConnect(scene_file);  // first, depend by link, lane
  BuildLane();               // second, depend by link
  BuildLink();               // third, depend by node
  BuildNode();
#ifdef _DEBUG
  WriteGeojson();
#endif
}

/**
 * @brief convert message for current timestamp
 *
 * @param msg message to be returned
 * @return true on success
 * @return false
 */
bool CAsn1MsgMAP2020::Convert(google::protobuf::Message *msg) {
  v2x_asn1_2020::V2X_ASN1 *v2x_asn1_2020 = (v2x_asn1_2020::V2X_ASN1 *)msg;

  ////////
  // FILTER BY DATA
  ///////
  for (const auto &rsu : data.rsuData) {
    v2x_asn1_2020::V2XMAPMsg *pMapMsg = v2x_asn1_2020->mutable_map()->Add();
    // recover message from privous string
    CoverMap(rsu.PreRsu, pMapMsg);
    // has no privous message, using map
    if (pMapMsg->nodelist_size() == 0) {
      pMapMsg->set_msgcount(rsu.frameid);
      pMapMsg->set_minuteoftheyear(std::round(rsu.timestampe / 60000));
      // filter by junction id
      for (const auto &jun : rsu.junctions) {
        if (mapNodes.find(mapHdJunctionToV2XNode[jun]) != mapNodes.end()) {
          *pMapMsg->add_nodelist() = mapNodes.at(mapHdJunctionToV2XNode[jun]).node;
        }
      }
    }
  }
  return true;
}

/**
 * @brief build node to asn.1 msg
 *
 * @param v2xNode node tobe builded
 * @return true on success
 * @return false on failure
 */
bool CAsn1MsgMAP2020::BuildNode(V2xNode &v2xNode) {
  v2x_asn1_2020::Node *pNode = &v2xNode.node;
  pNode->set_descriptivename("Node" + std::to_string(v2xNode.HdJuncid));
  pNode->mutable_nodereferenceid()->set_nodeid(mapHdJunctionToV2XNode[v2xNode.HdJuncid]);
  double x = 0, y = 0, z = 0;
  int ccc = 0;
  // add end of outcome road
  for (auto rid : v2xNode.HdRoadfrom) {
    hadmap::txRoadPtr roadPtr;
    hadmap::getRoad(hMapHandle, rid, false, roadPtr);
    if (roadPtr) {
      const hadmap::txCurve *pGeo = roadPtr->getGeometry();
      x += pGeo->getEnd().x;
      y += pGeo->getEnd().y;
      z += pGeo->getEnd().z;
      ccc += 1;
    }
  }
  // add front of income road
  for (auto rid : v2xNode.HdRoadto) {
    hadmap::txRoadPtr roadPtr;
    hadmap::getRoad(hMapHandle, rid, false, roadPtr);
    if (roadPtr) {
      const hadmap::txCurve *pGeo = roadPtr->getGeometry();
      x += pGeo->getStart().x;
      y += pGeo->getStart().y;
      z += pGeo->getStart().z;
      ccc += 1;
    }
  }
  // calculate the center of the node
  if (ccc > 0) {
    pNode->mutable_position3d()->set_longitude(x * 1E7 / ccc);
    pNode->mutable_position3d()->set_latitude(y * 1E7 / ccc);
    pNode->mutable_position3d()->set_elevation(z * 10 / ccc);
  }
  for (auto lid : v2xNode.preLk) {
    pNode->set_linklistflag(true);
    *pNode->add_linklist() = mapLinks.at(lid).link;
  }
  return true;
}

/**
 * @brief Build link to asn1 msg
 *
 * @param v2xLink the link to be built
 * @return true on success
 * @return false
 */
bool CAsn1MsgMAP2020::BuildLink(V2xLink &v2xLink) {
  v2x_asn1_2020::Link *pLink = &v2xLink.link;
  pLink->set_descriptivename("Link");
  // set id
  if (v2xLink.preN > 0) {
    pLink->mutable_nodereferenceid()->set_nodeid(mapHdJunctionToV2XNode[mapNodes.at(v2xLink.preN).HdJuncid]);
  }
  // set width
  if (!v2xLink.lanes.empty()) {
    auto &fstLane = mapLanes.at(v2xLink.lanes.front());
    pLink->set_speedlimitflag(fstLane.lane.speedlimitlistflag());
    *pLink->mutable_speedlimitlist() = fstLane.lane.speedlimitlist();
    double widths = 0;
    for (auto lane : v2xLink.lanes) {
      auto &Lane = mapLanes.at(lane);
      widths += Lane.lwidth;
    }
    pLink->set_lanewidth(widths * 100);  // cm
  }
  // add lane
  for (auto lid : v2xLink.lanes) {
    auto &vLane = mapLanes.at(lid);

    *pLink->add_lanelist() = vLane.lane;
  }
  // add movement list
  if (v2xLink.nxtN > 0) {
    auto mov = pLink->add_movementlist();
    mov->mutable_remoteintersection()->set_nodeid(mapHdJunctionToV2XNode[mapNodes.at(v2xLink.nxtN).HdJuncid]);
    for (auto lid : v2xLink.lanes) {
      for (const auto &conn : mapLanes.at(lid).lane.connectstolist()) {
        int pid = conn.phaseid();
        if (pid > 0) {
          // set phase id
          mov->set_phaseid(pid);
          break;
        }
      }
      if (mov->phaseid() > 0) break;
    }
  }
  // set point list of link
  for (const auto &rid : v2xLink.roads) {
    hadmap::txRoadPtr roadPtr;
    hadmap::getRoad(hMapHandle, rid, false, roadPtr);
    if (roadPtr) {
      // set road geometry to link
      const hadmap::txCurve *pCurve = roadPtr->getGeometry();
      if (pCurve) {
        pLink->set_pointlistflag(true);
        hadmap::txLineCurve *curve = (hadmap::txLineCurve *)pCurve;
        hadmap::PointVec points;
        curve->sample(30, points);
        for (auto item : points) {
          v2x_asn1_2020::Position3D *pPos3d = pLink->mutable_pointlist()->Add();
          pPos3d->set_longitude(item.x * 1E7);
          pPos3d->set_latitude(item.y * 1E7);
          pPos3d->set_elevation(item.z * 10);
        }
      }
    }
  }
  // set direction of link
  if (pLink->pointlist_size() > 1) {
    v2x_asn1_2020::Position3D p1 = pLink->pointlist().at(pLink->pointlist_size() - 1);
    v2x_asn1_2020::Position3D p2 = pLink->pointlist().at(pLink->pointlist_size() - 2);
    // set to int
    double x = p1.longitude() * 1E-7;
    double y = p1.latitude() * 1E-7;
    double z = p1.elevation() * 0.1;
    coord_trans_api::lonlat2enu(x, y, z, p2.longitude() * 1E-7, p2.latitude() * 1E-7, p2.elevation() * 0.1);
    double yaw = std::atan2(y, x);
    if (std::abs(yaw) < 0.7853981633974483096) {
      pLink->set_descriptivename("WestLink");
    } else if (std::abs(yaw) > 2.3561944901923449) {
      pLink->set_descriptivename("EastLink");
    } else if (yaw > 0) {
      pLink->set_descriptivename("SouthLink");
    } else {
      pLink->set_descriptivename("NorthLink");
    }
  }

  return true;
}

void CAsn1MsgMAP2020::SetLaneType(v2x_asn1_2020::Lane *pLane, hadmap::LANE_TYPE type) {
  v2x_asn1_2020::LaneAttributes *pLaneAttr = pLane->mutable_laneattributes();
  // todo
  pLaneAttr->set_lanesharing(0);
  // handle lane type
  switch (type) {
    case hadmap::LANE_TYPE_Others:
      break;
    case hadmap::LANE_TYPE_None:
      break;
    case hadmap::LANE_TYPE_Driving: {
      std::bitset<8> bValue;
      for (int i = 0; i < 8; i++) bValue.set(i);
      pLaneAttr->mutable_lanetypeattributes()->set_vehicle(bValue.to_ulong());
      pLane->set_laneattributesflag(true);
      break;
    }
    case hadmap::LANE_TYPE_Stop:
      break;
    case hadmap::LANE_TYPE_Shoulder:
      break;
    case hadmap::LANE_TYPE_Biking: {
      std::bitset<16> bValue;
      for (int i = 0; i < 16; i++) bValue.set(i);
      pLaneAttr->mutable_lanetypeattributes()->set_bike(bValue.to_ulong());
      pLane->set_laneattributesflag(true);
      break;
    }
    case hadmap::LANE_TYPE_Sidewalk: {
      std::bitset<16> bValue;
      for (int i = 0; i < 16; i++) bValue.set(i);
      pLaneAttr->mutable_lanetypeattributes()->set_crosswalk(bValue.to_ulong());
      pLane->set_laneattributesflag(true);
      break;
    }
    case hadmap::LANE_TYPE_Border:
      break;
    case hadmap::LANE_TYPE_Restricted:
      break;
    case hadmap::LANE_TYPE_Parking: {
      std::bitset<16> bValue;
      for (int i = 0; i < 16; i++) bValue.set(i);
      pLaneAttr->mutable_lanetypeattributes()->set_parking(bValue.to_ulong());
      pLane->set_laneattributesflag(true);

      break;
    }
    // not supported
    case hadmap::LANE_TYPE_MwyEntry:
      break;
    case hadmap::LANE_TYPE_MwyExit:
      break;
    case hadmap::LANE_TYPE_Acceleration:
      break;
    case hadmap::LANE_TYPE_Deceleration:
      break;
    case hadmap::LANE_TYPE_Compound:
      break;
    case hadmap::LANE_TYPE_HOV:
      break;
    case hadmap::LANE_TYPE_Slow:
      break;
    case hadmap::LANE_TYPE_Reversible:
      break;
    case hadmap::LANE_TYPE_EmergencyParking:
      break;
    case hadmap::LANE_TYPE_Emergency:
      break;
    case hadmap::LANE_TYPE_TurnWaiting:
      break;
    case hadmap::LANE_TYPE_TollBooth:
      break;
    case hadmap::LANE_TYPE_ETC:
      break;
    case hadmap::LANE_TYPE_CheckStation:
      break;
    default:
      break;
  }
}

/**
 * @brief Build lane to asn1
 *
 * @param v2xLane
 * @return true
 * @return false
 */
bool CAsn1MsgMAP2020::BuildLane(V2xLane &v2xLane) {
  if (v2xLane.HdLanes.empty()) return false;
  hadmap::txLanePtr lanePtr;
  hadmap::getLane(hMapHandle, v2xLane.HdLanes.at(v2xLane.HdLanes.size() / 2), lanePtr);
  if (!lanePtr) {
    return false;
  }
  v2x_asn1_2020::Lane *pLane = &v2xLane.lane;
  // set id
  pLane->set_laneid(-lanePtr->getId());
  // set width
  v2xLane.lwidth = lanePtr->getLaneWidth();

  SetLaneType(pLane, lanePtr->getLaneType());
  // calculate connects to list
  hadmap::txLaneLinks links;
  hadmap::getNextLaneLinks(hMapHandle, v2xLane.HdLanes.back(), links);
  if (!links.empty()) {
    pLane->set_connectstolistflag(true);
    int turnType = 0;
    for (auto link : links) {
      if (mapHdLinkToV2XConnect.find(link->getId()) == mapHdLinkToV2XConnect.end()) {
        continue;
      }
      auto &vConn = mapConnects.at(mapHdLinkToV2XConnect.at(link->getId()));
      turnType |= vConn.connect.connectinglane().allowedmaneuvers();
      *pLane->add_connectstolist() = vConn.connect;
    }
    // copy maneuvers
    pLane->set_allowedmaneuvers(turnType);
  }
  // set speed limit
  if (lanePtr->getSpeedLimit() != 0) {
    pLane->set_speedlimitlistflag(true);
    auto speed = pLane->mutable_speedlimitlist()->Add();
    speed->set_speedlimittype(5);
    speed->set_speed(std::round(3.6 * lanePtr->getSpeedLimit() * 50));  // 0.02m/s
  }
  // set lane points
  for (const auto &lid : v2xLane.HdLanes) {
    hadmap::txLanePtr lanePtr;
    hadmap::getLane(hMapHandle, lid, lanePtr);
    if (!lanePtr || !lanePtr->getGeometry()) {
      continue;
    }
    const hadmap::txCurve *pCurve = lanePtr->getGeometry();

    pLane->set_pointlistflag(true);
    hadmap::txLineCurve *curve = (hadmap::txLineCurve *)pCurve;
    hadmap::PointVec points;
    curve->sample(30, points);
    if (!pLane->mutable_pointlist()->empty()) {
      pLane->mutable_pointlist()->RemoveLast();
    }
    for (const auto &item : points) {
      v2x_asn1_2020::Position3D *pPos3d = pLane->mutable_pointlist()->Add();
      pPos3d->set_longitude(item.x * 1E7);
      pPos3d->set_latitude(item.y * 1E7);
      pPos3d->set_elevation(item.z * 10);
    }
  }
  // set maneuver using the last lane
  {
    hadmap::txLanePtr lanePtr;
    hadmap::getLane(hMapHandle, v2xLane.HdLanes.back(), lanePtr);
    if (lanePtr) {
      int btype = 0;
      hadmap::txLaneBoundaryPtr lb, rb;
      hadmap::getBoundary(hMapHandle, lanePtr->getLeftBoundaryId(), lb);
      hadmap::getBoundary(hMapHandle, lanePtr->getRightBoundaryId(), rb);
      if (lb) {
        btype |= lb->getLaneMark();
      }
      if (rb) {
        btype |= rb->getLaneMark();
      }
      if (btype & hadmap::LANE_MARK_Broken) {
        int turnType = pLane->allowedmaneuvers();
        turnType |= 2;
        pLane->set_allowedmaneuvers(turnType);
      }
    }
  }
  return true;
}

/**
 * @brief build connect to ans1 message
 *
 * @param v2xConnect connect message, in and out
 * @return true on success
 * @return false
 */
bool CAsn1MsgMAP2020::BuildConnect(V2xConnect &v2xConnect) {
  if (mapHdJunctionToV2XNode.find(v2xConnect.HdJuncid) == mapHdJunctionToV2XNode.end()) {
    return false;
  }
  v2x_asn1_2020::Connection *pConn = &v2xConnect.connect;
  if (v2xConnect.nxtLane > 0 && mapLanes[v2xConnect.nxtLane].bLink > 0) {
    pConn->mutable_nodereferenceid()->set_nodeid(mapLinks[mapLanes[v2xConnect.nxtLane].bLink].nxtN);
  }
  // handle lane link
  hadmap::txLaneLinkPtr link;
  hadmap::getLaneLink(hMapHandle, v2xConnect.HdLid, link);
  if (link) {
    pConn->mutable_connectinglane()->set_laneid(-link->toLaneId());
    pConn->mutable_connectinglane()->set_allowedmaneuvers(calLinkTurn(link));
  }
  pConn->set_connectinglaneflag(true);
  return true;
}

/**
 * @brief write previous message to json
 *
 * @param fpath path to write
 */
void CAsn1MsgMAP2020::WritePreJsonExample(const std::string &fpath) {
  v2x_asn1_2020::V2XMAPMsg msg;
  // fillup proto
  FillupProto(&msg);
  std::string buffer;
  google::protobuf::util::MessageToJsonString(msg, &buffer);
  Json::Value tree;
  Json::CharReaderBuilder reader;
  std::string err;
  std::unique_ptr<Json::CharReader> const json_read(reader.newCharReader());
  if (!json_read->parse(buffer.c_str(), buffer.c_str() + buffer.length(), &tree, &err)) {
    LOG(WARNING) << "Read pre MAP faild: " << err;
    return;
  }
  Json::Value prejson;
  prejson["MAP"] = tree;
  Json::StreamWriterBuilder builder;
  const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::ofstream emp(fpath);
  writer->write(prejson, &emp);
  emp.close();
}

/**
 * @brief recover map message by previous data
 *
 * @param preRSU previous rsu data
 * @param pMapMsg message to be reconverred
 */
void CAsn1MsgMAP2020::CoverMap(const std::string &preRSU, v2x_asn1_2020::V2XMAPMsg *pMapMsg) {
  if (preRSU.empty()) return;
  // json to proto
  Json::Value tree;
  Json::CharReaderBuilder reader;
  std::string err;
  std::unique_ptr<Json::CharReader> const json_read(reader.newCharReader());
  if (!json_read->parse(preRSU.c_str(), preRSU.c_str() + preRSU.length(), &tree, &err)) {
    LOG(WARNING) << "Read pre RSU faild: " << err;
    return;
  }
  Json::Value jsonv = tree["MAP"];
  if (!jsonv.isObject()) {
    return;
  }
  if (!google::protobuf::util::JsonStringToMessage(jsonv.toStyledString(), pMapMsg).ok()) {
    LOG(WARNING) << "CANNOT read Map node from preRSU";
    return;
  }
}

/**
 * @brief calculate link turn
 *
 * @param link ptr to lane link
 * @return turn type
 */
int CAsn1MsgMAP2020::calLinkTurn(const hadmap::txLaneLinkPtr &link) {
  if (link->getGeometry()) {
    hadmap::txLineCurve *curve = (hadmap::txLineCurve *)(link->getGeometry());
    double a = curve->yaw(curve->size() - 1) - curve->yaw(0);
    a *= EIGEN_PI / 180.;
    a = std::atan2(std::sin(a), std::cos(a));
    // maneuverStraightAllowed=128
    // maneuverLeftAllowed=64
    // maneuverRightAllowed=32
    // maneuverUTurnAllowed=16
    // maneuverLeftTurnOnRedAllowed=8
    // maneuverRightTurnOnRedAllowed=4
    // maneuverLaneChangeAllowed=2
    // maneuverNoStoppingAllowed=1,
    // yieldAllwaysRequired=32768
    // goWithHalt=16384
    // caution=8192
    // reserved1=4096
    if (std::abs(a) < EIGEN_PI * 0.25) {
      return 128;  // straight
    } else if (std::abs(a) > EIGEN_PI * 0.75) {
      return 16;  // uturn
    } else if (a > 0) {
      return 64 + 8;  // left
    } else {
      return 32 + 4;  // right
    }
  }
  return 0;
}
/**
 * @brief get previous road
 *
 * @param rid id of road
 */
std::vector<hadmap::roadpkid> CAsn1MsgMAP2020::Topo_getFromRoad(hadmap::roadpkid rid) {
  std::vector<hadmap::roadpkid> roads;
  hadmap::txRoadPtr road;
  while (hadmap::getRoad(hMapHandle, rid, true, road) == TX_HADMAP_DATA_OK) {
    roads.push_back(rid);
    rid = ROAD_PKID_INVALID;
    if (!road->getSections().empty()) {
      for (const auto &lane : road->getSections().front()->getLanes()) {
        hadmap::txLanes lanes;
        hadmap::getPrevLanes(hMapHandle, lane, lanes);
        if (!lanes.empty()) {
          rid = lanes.front()->getRoadId();
        }
      }
    }
  }
  std::reverse(roads.begin(), roads.end());
  return roads;
}

/**
 * @brief get next road
 *
 * @param rid id of road
 */
std::vector<hadmap::roadpkid> CAsn1MsgMAP2020::Topo_getToRoad(hadmap::roadpkid rid) {
  std::vector<hadmap::roadpkid> roads;
  hadmap::txRoadPtr road;
  while (hadmap::getRoad(hMapHandle, rid, true, road) == TX_HADMAP_DATA_OK) {
    roads.push_back(rid);
    rid = ROAD_PKID_INVALID;
    if (!road->getSections().empty()) {
      for (const auto &lane : road->getSections().back()->getLanes()) {
        hadmap::txLanes lanes;
        hadmap::getNextLanes(hMapHandle, lane, lanes);
        if (!lanes.empty()) {
          rid = lanes.front()->getRoadId();
        }
      }
    }
  }
  return roads;
}
