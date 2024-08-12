/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "route_data.h"
#include <cassert>
#include "./lane_data.h"
#include "./section_data.h"
#include "common/engine/entity/hadmap.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/route.h"
#ifndef __HADMAP_SERVER__
#  include "../engine/primitives/cube.h"
#  include "../engine/renderer/billboard/billboard.h"
#endif

CRouteData::CRouteData(CHadmap* pHadmap) {
  m_fBB2CubeDist = 2.0f;

  m_pBBStart = nullptr;
  m_pBBMid = nullptr;
  m_pBBEnd = nullptr;

  m_pCubeStart = nullptr;
  m_pCubeMid = nullptr;
  m_pCubeEnd = nullptr;

  m_pRoute = nullptr;
  m_pHadmap = pHadmap;
}

CRouteData::~CRouteData() { Clear(); }
/*
void CRouteData::SetBBStart(CBillboard* pBBStart)
{
        m_pBBStart = pBBStart;
}

void CRouteData::SetBBEnd(CBillboard* pBBEnd)
{
        m_pBBEnd = pBBEnd;
}
*/

void CRouteData::Clear() {
  if (m_pBBStart) {
    delete m_pBBStart;
    m_pBBStart = nullptr;
  }

  if (m_pBBMid) {
    delete m_pBBMid;
    m_pBBMid = nullptr;
  }

  if (m_pBBEnd) {
    delete m_pBBEnd;
    m_pBBEnd = nullptr;
  }

  if (m_pCubeStart) {
    delete m_pCubeStart;
    m_pCubeStart = nullptr;
  }

  if (m_pCubeMid) {
    delete m_pCubeMid;
    m_pCubeMid = nullptr;
  }

  if (m_pCubeEnd) {
    delete m_pCubeEnd;
    m_pCubeEnd = nullptr;
  }

  m_pRoute = nullptr;
}

void CRouteData::SetRouteData(CRoute* pRoute) {
  if (!pRoute) {
    SYSTEM_LOGGER_ERROR("SetRouteData is null");
    assert(false);
    return;
  }

  if (!m_pHadmap) {
    SYSTEM_LOGGER_ERROR("routedata hadmap is null");
    assert(false);
    return;
  }

  Clear();

  m_pRoute = pRoute;

#ifndef __HADMAP_SERVER__
  if (pRoute->m_nType == CRoute::ROUTE_TYPE_START_END) {
    double dLon, dLat, dAlt;
    std::string strName = "";
    if (pRoute->m_dStartLon >= 0 && pRoute->m_dStartLat >= 0) {
      // 起始点标识
      CBillboard* pBBStart = new CBillboard();
      dLon = pRoute->m_dStartLon;
      dLat = pRoute->m_dStartLat;
      dAlt = 0;
      m_pHadmap->LonlatToLocal(dLon, dLat, dAlt);
      pBBStart->Pos().X() = dLon;
      pBBStart->Pos().Y() = dLat;
      pBBStart->Pos().Z() = m_fBB2CubeDist;
      pBBStart->SetType(BillBoard_Text);
      strName = pRoute->m_strID;
      strName.append("_SE_Start");
      pBBStart->SetContent(strName.c_str());
      pBBStart->Prepare();

      CCube* pCubeStart = new CCube();
      pCubeStart->SetColor(0, 1, 0);
      pCubeStart->SetSize(0.5, 0.5, 0.5);
      pCubeStart->SetPosition(dLon, dLat, 0);
      pCubeStart->PrepareDraw();

      m_pBBStart = pBBStart;
      m_pCubeStart = pCubeStart;
    }

    if (pRoute->m_dMidLon >= 0 && pRoute->m_dMidLat >= 0) {
      // 结束点标识
      CBillboard* pBBMid = new CBillboard();
      dLon = pRoute->m_dMidLon;
      dLat = pRoute->m_dMidLat;
      dAlt = 0;
      m_pHadmap->LonlatToLocal(dLon, dLat, dAlt);
      pBBMid->Pos().X() = dLon;
      pBBMid->Pos().Y() = dLat;
      pBBMid->Pos().Z() = m_fBB2CubeDist;
      pBBMid->SetType(BillBoard_Text);
      strName = pRoute->m_strID;
      strName.append("_SE_Mid");
      pBBMid->SetContent(strName.c_str());
      pBBMid->Prepare();

      CCube* pCubeMid = new CCube();
      pCubeMid->SetColor(0, 1, 0);
      pCubeMid->SetSize(0.5, 0.5, 0.5);
      pCubeMid->SetPosition(dLon, dLat, 0);
      pCubeMid->PrepareDraw();

      m_pBBMid = pBBMid;
      m_pCubeMid = pCubeMid;
    }

    if (pRoute->m_dEndLon >= 0 && pRoute->m_dEndLat >= 0) {
      // 结束点标识
      CBillboard* pBBEnd = new CBillboard();
      dLon = pRoute->m_dEndLon;
      dLat = pRoute->m_dEndLat;
      dAlt = 0;
      m_pHadmap->LonlatToLocal(dLon, dLat, dAlt);
      pBBEnd->Pos().X() = dLon;
      pBBEnd->Pos().Y() = dLat;
      pBBEnd->Pos().Z() = m_fBB2CubeDist;
      pBBEnd->SetType(BillBoard_Text);
      strName = pRoute->m_strID;
      strName.append("_SE_End");
      pBBEnd->SetContent(strName.c_str());
      pBBEnd->Prepare();

      CCube* pCubeEnd = new CCube();
      pCubeEnd->SetColor(0, 1, 0);
      pCubeEnd->SetSize(0.5, 0.5, 0.5);
      pCubeEnd->SetPosition(dLon, dLat, 0);
      pCubeEnd->PrepareDraw();

      m_pBBEnd = pBBEnd;
      m_pCubeEnd = pCubeEnd;
    }
  } else if (pRoute->m_nType == CRoute::ROUTE_TYPE_ROAD_ID) {
    CSectionData* pSecData = m_pHadmap->FindSection(m_pRoute->m_roadID, m_pRoute->m_sectionID);

    if (!pSecData) {
      SYSTEM_LOGGER_ERROR("section not exist!");
      assert(false);
    }

    Lanes& lanes = pSecData->Data();
    if (lanes.size() == 0) {
      SYSTEM_LOGGER_ERROR("section data is null!");
      assert(false);
    }

    CLaneData* pLane = lanes[0];
    if (!pLane->IsEmpty()) {
      // 起始点标识
      CBillboard* pBBStart = new CBillboard();
      pBBStart->Pos().X() = pLane->Data()[0].X();
      pBBStart->Pos().Y() = pLane->Data()[0].Y();
      pBBStart->Pos().Z() = pLane->Data()[0].Z() + m_fBB2CubeDist;
      pBBStart->SetType(BillBoard_Text);
      std::string strName = pRoute->m_strID;
      strName.append("_R_Start");
      pBBStart->SetContent(strName.c_str());
      pBBStart->Prepare();

      CCube* pCubeStart = new CCube();
      pCubeStart->SetColor(0, 1, 0);
      pCubeStart->SetSize(0.5, 0.5, 0.5);
      pCubeStart->SetPosition(pLane->Data()[0].X(), pLane->Data()[0].Y(), pLane->Data()[0].Z());
      pCubeStart->PrepareDraw();

      m_pBBStart = pBBStart;
      m_pCubeStart = pCubeStart;
    }
  } else {
    SYSTEM_LOGGER_ERROR("route type error!");
    assert(false);
  }
#endif
}

int CRouteData::Type() {
  if (!m_pRoute) {
    return CRoute::ROUTE_TYPE_INVALID;
  }

  return m_pRoute->m_nType;
}

void CRouteData::Draw() {
#ifndef __HADMAP_SERVER__

  if (m_pBBStart) {
    m_pBBStart->Draw();
  }

  if (m_pBBMid) {
    m_pBBMid->Draw();
  }
  if (m_pBBEnd) {
    m_pBBEnd->Draw();
  }

  if (m_pCubeStart) {
    m_pCubeStart->Draw();
  }

  if (m_pCubeMid) {
    m_pCubeMid->Draw();
  }

  if (m_pCubeEnd) {
    m_pCubeEnd->Draw();
  }
#endif
}
