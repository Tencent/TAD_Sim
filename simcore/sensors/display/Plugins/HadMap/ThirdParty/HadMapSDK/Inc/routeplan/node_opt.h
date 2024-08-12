// Copyright (c) 2016 Tencent, Inc. All Rights Reserved
// @author elkejiang@tencent.com
// @date 2018/04/02
// @brief A Star Node Operation

#pragma once

#include "mapengine/hadmap_engine.h"
#include "routeplan/route_struct.h"

namespace planner {
class NodeOpt {
 private:
  hadmap::txMapHandle* pHandle;

  static NodeOpt* insPtr;

 private:
  NodeOpt() : pHandle(NULL) {}
  ~NodeOpt() {}

 public:
  static NodeOpt* getInstance();

 public:
  void setMapHandle(hadmap::txMapHandle* pH);

  bool getNodeByLoc(const hadmap::txPoint& loc, NodePtr& nodePtr);

  // get bidirection node info
  // if lane(id=1) nearest to loc, then return lane(id=-1)
  // if lane(id=-1) nearest to loc, then return lane(id=1)
  bool getOppositeNodeByLoc(const hadmap::txPoint& loc, NodePtr& nodePtr, hadmap::txPoint& tagP);

  // get all adj nodes from cur node
  // set nodes data to nodePtr->adjNodes
  bool getAdjNodes(NodePtr& nodePtr);

  // init node info
  bool initNode(NodePtr& nodePtr);

  // calc directly distance
  double euclideanDis(const NodePtr& srcPtr, const NodePtr& dstPtr);

 public:
  // get lanelinks between nodes
  bool getLaneLinks(const NodeId& fromId, const NodeId& toId, hadmap::txLaneLinks& links);

  // get lanlinks between nodes
  bool getLaneLinks(hadmap::roadpkid fromId, hadmap::roadpkid toId, hadmap::txLaneLinks& links);

  // get lanelinks by envelope
  // center point lon lat
  // radius meter
  bool getLaneLinks(const hadmap::txPoint& center, double radidus, hadmap::txLaneLinks& links);

  // get node info
  bool getRoad(const NodeId& id, const bool& wholeData, hadmap::txRoadPtr& roadPtr);

  // get node info
  bool getRoad(hadmap::roadpkid roadId, const bool& wholeData, hadmap::txRoadPtr& roadPtr);

  // get section info
  // laneId < 0, only keep current side info
  // laneId > 0, only keep opposite side info and reverse data
  // laneId == 0, keep both side info
  bool getSection(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& secId, const hadmap::lanepkid& laneId,
                  hadmap::txSectionPtr& secPtr);

  // get sections
  // limited by minLen
  bool getSections(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& secId, const hadmap::lanepkid& laneId,
                   double minLen, hadmap::txSections& sections);

  // get lane info
  bool getLane(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& sectionId, const hadmap::lanepkid& laneId,
               hadmap::txLanePtr& lanePtr);

  // get lane width
  double getLaneWidth(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& sectionId,
                      const hadmap::lanepkid& laneId);

  // get traffic lights
  bool getTrafficLights(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& sectionId,
                        const hadmap::lanepkid& laneId, hadmap::txObjects& objects);

  // get objects by relation
  bool getObjects(const hadmap::roadpkid& roadId, const hadmap::sectionpkid& sectionId, const hadmap::lanepkid& laneId,
                  hadmap::OBJECT_TYPE objType, hadmap::txObjects& objects);
};
}  // namespace planner
