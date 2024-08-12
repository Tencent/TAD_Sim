// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routeplan/a_star.h"
#include <string.h>
#include <algorithm>
#include <map>
#include <set>

#include "common/log.h"
#include "routeplan/node_opt.h"

namespace planner {
void AStar::resetQueryParam() {
  queryParam.srcNode->clear();
  queryParam.dstNode->clear();
  queryParam.bakDstNode->clear();
}

bool AStar::setQueryParam(const hadmap::txPoint& srcPt, const hadmap::txPoint& dstPt) {
  resetQueryParam();
  if (!NodeOpt::getInstance()->getNodeByLoc(srcPt, queryParam.srcNode) ||
      !NodeOpt::getInstance()->getNodeByLoc(dstPt, queryParam.dstNode)) {
    txlog::error("setQueryParam Error, cant find road by loc");
    return false;
  }

  if (strategy == FUZZY_LOCATE) NodeOpt::getInstance()->getOppositeNodeByLoc(dstPt, queryParam.bakDstNode, tagP);

  queryParam.srcNode->gCost = 0.0;
  queryParam.srcNode->hCost = NodeOpt::getInstance()->euclideanDis(queryParam.srcNode, queryParam.dstNode);
  queryParam.dstNode->hCost = 0.0;

  char info[128];
  sprintf(info, "From %s, To %s", std::to_string(queryParam.srcNode->nodeId.roadId).c_str(),
          std::to_string(queryParam.dstNode->nodeId.roadId).c_str());
  txlog::info(info);
  return true;
}

SearchState AStar::searchPath(PathList& path) {
  SearchState state = INVALID;
  bool findDst = queryParam.srcNode->nodeId == queryParam.dstNode->nodeId;

  PathList openList;
  std::set<NodeId> openSet;
  std::set<NodeId> closeSet;
  std::map<NodeId, NodePtr> accessedSet;

  openList.push_back(queryParam.srcNode);
  openSet.insert(queryParam.srcNode->nodeId);

  chTag = false;

  while (!openList.empty() && !findDst) {
    std::make_heap(openList.begin(), openList.end(), Greater());
    NodePtr curNodePtr = openList.front();
    closeSet.insert(curNodePtr->nodeId);
    std::pop_heap(openList.begin(), openList.end(), Greater());
    openList.pop_back();
    closeSet.insert(curNodePtr->nodeId);

    NodeOpt::getInstance()->getAdjNodes(curNodePtr);
    NodePtrList::iterator itr = curNodePtr->adjNodes.begin();
    // std::cout << curNodePtr->nodeId.roadId << "." << int( curNodePtr->nodeId.laneId ) << " -> ";
    for (; itr != curNodePtr->adjNodes.end(); ++itr) {
      NodePtr curAdjNodePtr = *itr;
      // uint64_t tmpRoadId = curAdjNodePtr->nodeId.roadId;
      // std::cout << curAdjNodePtr->nodeId.roadId << "." << int( curAdjNodePtr->nodeId.laneId ) << " ";

      if (closeSet.find(curAdjNodePtr->nodeId) != closeSet.end()) {
        continue;
      } else {
        if (accessedSet.find(curAdjNodePtr->nodeId) != accessedSet.end()) {
          if (curAdjNodePtr->gCost < accessedSet[curAdjNodePtr->nodeId]->gCost) {
            accessedSet[curAdjNodePtr->nodeId]->gCost = curAdjNodePtr->gCost;
            accessedSet[curAdjNodePtr->nodeId]->parrent = curAdjNodePtr->parrent;
          }
        } else {
          accessedSet.insert(std::make_pair(curAdjNodePtr->nodeId, curAdjNodePtr));
        }

        if (openSet.find(curAdjNodePtr->nodeId) == openSet.end()) {
          openSet.insert(curAdjNodePtr->nodeId);
          openList.push_back(accessedSet[curAdjNodePtr->nodeId]);
        }

        if ((*curAdjNodePtr) == (*(queryParam.dstNode))) {
          queryParam.dstNode->parrent = accessedSet[curAdjNodePtr->nodeId]->parrent;
          findDst = true;
          break;
        }
        if (strategy == FUZZY_LOCATE && (*curAdjNodePtr) == (*(queryParam.bakDstNode))) {
          txlog::info("Find Bak Path");
          queryParam.bakDstNode->parrent = accessedSet[curAdjNodePtr->nodeId]->parrent;
          findDst = true;
          chTag = true;
          break;
        }
      }
    }
    // std::cout << std::endl;
  }
  if (findDst) {
    txlog::info("Find Path");
    state = SUCC;
    if (!chTag) {
      if (!genPathList(queryParam.srcNode, queryParam.dstNode, path)) return INVALID;
    } else {
      if (!genPathList(queryParam.srcNode, queryParam.bakDstNode, path)) return INVALID;
    }
    printPathList(path);
  } else {
    state = UN_MATCHED;
  }
  return state;
}

bool AStar::genPathList(const NodePtr& beginPtr, const NodePtr& endPtr, PathList& path) {
  if (!beginPtr || !endPtr) {
    txlog::error("genPathList Error, NodePtr Empty");
    return false;
  }
  PathList rPath;
  NodePtr curNodePtr = endPtr;
  while (*beginPtr != *curNodePtr) {
    rPath.push_back(curNodePtr);
    curNodePtr = curNodePtr->parrent.lock();
  }
  rPath.push_back(beginPtr);
  path.assign(rPath.rbegin(), rPath.rend());
  return true;
}

void AStar::printPathList(const PathList& path) const {
  const int minPathSize = (path.size() > 64 ? path.size() + 1 : 64);
  char* pathStr = new char[minPathSize * 32];
  sprintf(pathStr, "Path ");
  for (size_t i = 0; i < path.size(); ++i) {
    char nodeInfo[32];
    sprintf(nodeInfo, "%s - ", std::to_string(path[i]->nodeId.roadId).c_str());
    strcat(pathStr, nodeInfo);
  }
  txlog::info(pathStr);
  delete[] pathStr;
  pathStr = NULL;
}

bool AStar::getTagP(hadmap::txPoint& p) const {
  if (chTag) p = tagP;
  return chTag;
}
}  // namespace planner
