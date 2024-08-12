/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "group_entity.h"
#include <algorithm>
#include "common/log/system_logger.h"
#include "cross.h"
#include "curve_road.h"
#include "entity_link.h"
#include "map_scene.h"
#include "straight_road.h"

CGroupEntity::CGroupEntity() {
  m_children.clear();
  m_links.clear();
  m_fromLinksMap.clear();
  m_toLinksMap.clear();
  m_scene = nullptr;
}

void CGroupEntity::AddChild(int nID) { m_children.push_back(nID); }

void CGroupEntity::AddLink(CLinkEntity* pLink) {
  m_links.push_back(pLink);

  std::vector<CLinkEntity*>* pLinks = FindFromLinks(pLink->m_from_id);
  if (pLinks == nullptr) {
    std::vector<CLinkEntity*> links;
    links.push_back(pLink);
    m_fromLinksMap.insert(std::make_pair(pLink->m_from_id, links));
  } else {
    std::vector<CLinkEntity*>::iterator itr = std::find(pLinks->begin(), pLinks->end(), pLink);
    if (itr == pLinks->end()) {
      pLinks->push_back(pLink);
    }
  }

  pLinks = FindToLinks(pLink->m_to_id);
  if (pLinks == nullptr) {
    std::vector<CLinkEntity*> links;
    links.push_back(pLink);
    m_toLinksMap.insert(std::make_pair(pLink->m_to_id, links));
  } else {
    std::vector<CLinkEntity*>::iterator itr = std::find(pLinks->begin(), pLinks->end(), pLink);
    if (itr == pLinks->end()) {
      pLinks->push_back(pLink);
    }
  }
}

std::vector<CLinkEntity*>* CGroupEntity::FindFromLinks(int id) {
  std::map<int, std::vector<CLinkEntity*> >::iterator itr = m_fromLinksMap.find(id);
  if (itr == m_fromLinksMap.end()) {
    return nullptr;
  }

  return &(itr->second);
}

std::vector<CLinkEntity*>* CGroupEntity::FindToLinks(int id) {
  std::map<int, std::vector<CLinkEntity*> >::iterator itr = m_toLinksMap.find(id);
  if (itr == m_toLinksMap.end()) {
    return nullptr;
  }

  return &(itr->second);
}



struct sEndInfo {
  int m_endIdx;
  int m_roadID;
};

void CGroupEntity::GenerateCrossLink() {
  std::vector<int>::iterator itr = m_children.begin();
  for (; itr != m_children.end(); ++itr) {
    CCross* pCross = m_scene->FindCross((*itr));
    if (pCross) {
      std::vector<CLinkEntity*>* pToLinks = FindToLinks(pCross->m_id);
      std::vector<CLinkEntity*>* pFromLinks = FindFromLinks(pCross->m_id);
      if (pToLinks && pFromLinks) {
        std::vector<CLinkEntity*>::iterator fromItr = pFromLinks->begin();
        std::vector<CLinkEntity*>::iterator toItr = pToLinks->begin();
        std::map<int, sEndInfo> endSetsStart;
        std::set<int> endSetsFrom;
        std::set<int> endSetsTo;

        for (; toItr != pToLinks->end(); ++toItr) {
          sEndInfo info;
          info.m_endIdx = (*toItr)->m_to_end;
          info.m_roadID = (*toItr)->m_from_id;
          endSetsStart.insert(std::make_pair(info.m_endIdx, info));
          endSetsFrom.insert(info.m_endIdx);
        }
        for (; fromItr != pFromLinks->end(); ++fromItr) {
          sEndInfo info;
          info.m_endIdx = (*fromItr)->m_from_end;
          info.m_roadID = (*fromItr)->m_to_id;
          endSetsStart.insert(std::make_pair(info.m_endIdx, info));
          endSetsTo.insert(info.m_endIdx);
        }
        std::map<int, sEndInfo> endSetsEnd(endSetsStart);

        std::map<int, sEndInfo>::iterator fromSetItr = endSetsStart.begin();
        std::map<int, sEndInfo>::iterator toSetItr = endSetsEnd.begin();
        for (; fromSetItr != endSetsStart.end(); ++fromSetItr) {
          toSetItr = endSetsEnd.begin();
          for (; toSetItr != endSetsEnd.end(); ++toSetItr) {
            if (fromSetItr->first == toSetItr->first) {
              continue;
            }
            int nTerminalStart = fromSetItr->first;
            int nTerminalEnd = toSetItr->first;

            CMapElement* pFromElement = m_scene->FindElement(fromSetItr->second.m_roadID);
            CMapElement* pToElement = m_scene->FindElement(toSetItr->second.m_roadID);
            hadmap::txRoadPtr pTXRoadFrom = nullptr;
            hadmap::txRoadPtr pTXRoadTo = nullptr;
            if (pFromElement->m_type == HET_Straight) {
              CStraightRoad* pStraightFrom = static_cast<CStraightRoad*>(pFromElement);
              pTXRoadFrom = pStraightFrom->m_txRoad;
            } else if (pFromElement->m_type == HET_Curve) {
              CCurveRoad* pCurveFrom = static_cast<CCurveRoad*>(pFromElement);
              pTXRoadFrom = pCurveFrom->m_txRoad;
            }

            if (pToElement->m_type == HET_Straight) {
              CStraightRoad* pStraightFrom = static_cast<CStraightRoad*>(pToElement);
              pTXRoadTo = pStraightFrom->m_txRoad;
            } else if (pToElement->m_type == HET_Curve) {
              CCurveRoad* pCurveFrom = static_cast<CCurveRoad*>(pToElement);
              pTXRoadTo = pCurveFrom->m_txRoad;
            }

            hadmap::txSections secsFrom = pTXRoadFrom->getSections();
            hadmap::txSections secsTo = pTXRoadTo->getSections();

            for (int i = 0; i < 2; ++i) {
              for (int j = 0; j < 2; ++j) {
                if (pCross->m_links[nTerminalStart][nTerminalEnd][2 * i + j].m_points.size() > 0) {
                  hadmap::txLaneLinkPtr linkPtr(new hadmap::txLaneLink());
                  linkPtr->setFromRoadId(pTXRoadFrom->getId());
                  linkPtr->setFromSectionId(secsFrom[0]->getId());
                  linkPtr->setToRoadId(pTXRoadTo->getId());
                  linkPtr->setToSectionId(secsTo[0]->getId());

                  std::set<int>::iterator itrF = endSetsFrom.find(nTerminalStart);
                  std::set<int>::iterator itrT = endSetsTo.find(nTerminalEnd);
                  if (itrF != endSetsFrom.end() && itrT != endSetsTo.end()) {
                    linkPtr->setFromLaneId(i - 2);
                    linkPtr->setToLaneId(j - 2);
                  } else if (itrF != endSetsFrom.end() && itrT == endSetsTo.end()) {
                    linkPtr->setFromLaneId(i - 2);
                    linkPtr->setToLaneId(j + 1);
                  } else if (itrF == endSetsFrom.end() && itrT != endSetsTo.end()) {
                    linkPtr->setFromLaneId(i + 1);
                    linkPtr->setToLaneId(j - 2);
                  } else if (itrF == endSetsFrom.end() && itrT == endSetsTo.end()) {
                    linkPtr->setFromLaneId(i + 1);
                    linkPtr->setToLaneId(j + 1);
                  } else {
                    assert(false);
                  }

                  hadmap::txLineCurve curve;
                  std::vector<CPoint3D>& pts = pCross->m_links[nTerminalStart][nTerminalEnd][2 * i + j].m_points;
                  std::vector<CPoint3D>::iterator p3dItr = pts.begin();
                  for (; p3dItr != pts.end(); ++p3dItr) {
                    hadmap::txPoint pt;
                    pt.x = p3dItr->X();
                    pt.y = p3dItr->Y();
                    pt.z = p3dItr->Z();
                    curve.addPoint(pt);
                  }
                  linkPtr->setGeometry(curve);

                  SYSTEM_LOGGER_INFO("from:(%d, %d, %d) to(%d, %d, %d)", linkPtr->fromRoadId(),
                                     linkPtr->fromSectionId(), linkPtr->fromLaneId(), linkPtr->toRoadId(),
                                     linkPtr->toSectionId(), linkPtr->toLaneId());
                  m_scene->TXLaneLinks().push_back(linkPtr);
                }
              }
            }
          }
        }
      }
    }
  }
}