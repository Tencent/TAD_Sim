/**
 * @file asn1msg_map2020.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include "asn1msg_map.h"
#include "structs/hadmap_junction.h"
#include "structs/hadmap_lane.h"
#include "v2x_asn1_2020.pb.h"  // for serialization/deserialization of ASN.1 definitions

class CAsn1MsgMAP2020 : public CAsn1MsgMAP {
 public:
  void WritePreJsonExample(const std::string &fpath);

 protected:
  virtual void Reset(tx_sim::ResetHelper &helper);
  virtual bool Convert(google::protobuf::Message *msg);

  void CoverMap(const std::string &preRSU, v2x_asn1_2020::V2XMAPMsg *pMapMsg);

 private:
  v2x_asn1_2020::V2XMAPMsg pMapMsg;

  struct V2xNode;
  struct V2xLink;
  struct V2xLane;
  struct V2xConnect;

  struct V2xLink {
    size_t preN = 0, nxtN = 0;
    std::vector<hadmap::roadpkid> roads;
    std::vector<size_t> lanes;
    v2x_asn1_2020::Link link;
  };
  struct V2xNode {
    hadmap::junctionpkid HdJuncid = 0;
    std::set<hadmap::roadpkid> HdRoadfrom;
    std::set<hadmap::roadpkid> HdRoadto;
    std::set<size_t> preLk, nxtLk;
    v2x_asn1_2020::Node node;
  };

  struct V2xLane {
    std::vector<hadmap::txLaneId> HdLanes;
    size_t bLink;
    double lwidth;
    v2x_asn1_2020::Lane lane;
  };

  struct V2xConnect {
    size_t preLane = 0, nxtLane = 0;
    hadmap::junctionpkid HdJuncid = 0;
    hadmap::lanelinkpkid HdLid = 0;
    v2x_asn1_2020::Connection connect;
  };

  std::map<size_t, V2xLink> mapLinks;
  std::map<size_t, V2xNode> mapNodes;
  std::map<size_t, V2xLane> mapLanes;
  std::map<size_t, V2xConnect> mapConnects;

  std::map<hadmap::junctionpkid, size_t> mapHdJunctionToV2XNode;
  std::map<hadmap::lanelinkpkid, size_t> mapHdLinkToV2XConnect;
  std::map<hadmap::roadpkid, size_t> mapHdRoadToV2xLink;
  std::map<hadmap::txLaneId, size_t> mapHdLaneToV2xLane;

  // build in order
  void BuildTopo();
  void BuildLane();
  void BuildNode();
  void BuildLink();
  void BuildConnect(const std::string &);
  void WriteGeojson();
  int calLinkTurn(const hadmap::txLaneLinkPtr &link);

  // function

  // only use for calculate NodeReferenceID
  std::map<hadmap::roadpkid, std::uint32_t> mapHdRid;
  std::map<hadmap::roadpkid, std::uint32_t> nodeCount;
  bool BuildNode(V2xNode &v2xNode);
  bool BuildLink(V2xLink &v2xLink);
  bool BuildLane(V2xLane &v2xLane);
  bool BuildConnect(V2xConnect &v2xConnect);
  void SetLaneType(v2x_asn1_2020::Lane *pLane, hadmap::LANE_TYPE type);
  std::vector<hadmap::roadpkid> Topo_getFromRoad(hadmap::roadpkid rid);
  std::vector<hadmap::roadpkid> Topo_getToRoad(hadmap::roadpkid rid);

  friend class CAsn1MsgSPAT2020;
};
