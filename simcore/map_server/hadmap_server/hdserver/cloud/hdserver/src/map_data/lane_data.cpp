/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "lane_data.h"
#include "common/log/system_logger.h"
#include "lane_boundary_data.h"
#include "map_curve.h"

#ifndef __HADMAP_SERVER__
#  include "../engine/engine.h"
#  include "../engine/renderer/vao/vao_manager.h"
#  include "../engine/renderer/vao/vertex_array_object.h"

#endif
CLaneData::CLaneData() {
  Reset();
  SetType(ENTITY_TYPE_Lane);
}
CLaneData::CLaneData(const hadmap::txLanePtr& pLane) {
  CopyFrom(pLane);
  SetType(ENTITY_TYPE_Lane);
}

CLaneData& CLaneData::operator=(hadmap::txLanePtr& pLane) {
  CopyFrom(pLane);
  return *this;
}

CLaneData::~CLaneData() {
  // Reset();
}

void CLaneData::Reset() {
  m_speedLimit = -1;
  m_leftBoundaryID = -1;
  m_rightBoundaryID = -1;
  m_laneType = HADMAP_LANE_TYPE_None;
  m_laneArrow = HADMAP_LANE_ARROW_None;
  m_roadID = -1;
  m_sectionID = -1;
  m_laneID = -1;
  m_laneWidth = -1;
  m_length = -1;
  m_distanceToEnd = -1;
  m_name = "";
  m_enabled = false;

  // 这块内存回收以后要改，要不然会崩溃
  if (m_generatedMesh.m_pData) {
    delete[] m_generatedMesh.m_pData;
    m_generatedMesh.m_uDataCount = 0;
    m_generatedMesh.m_uDataSize = 0;
  }
  SetType(ENTITY_TYPE_Lane);
}

void CLaneData::CopyFrom(const hadmap::txLanePtr& pLane) {
  // info data
  m_speedLimit = pLane->getSpeedLimit();
  m_leftBoundaryID = pLane->getLeftBoundaryId();
  m_rightBoundaryID = pLane->getRightBoundaryId();
  m_laneType = static_cast<HADMAP_LANE_TYPE>(pLane->getLaneType());
  m_laneArrow = static_cast<HADMAP_LANE_ARROW>(pLane->getLaneArrow());
  m_roadID = pLane->getRoadId();
  m_sectionID = pLane->getSectionId();
  m_laneID = pLane->getId();
  m_laneWidth = pLane->getLaneWidth();
  if (m_laneWidth < 0) {
    // Ä¬ÈÏµÀÂ·¿í¶ÈÎª3.25
    // m_laneWidth = 3.25;
    // ÎªÁËÉÏÆû´óÖÚ300¸öcase½»¸¶°æ£¬½«Ä¬ÈÏÖµ¸ÄÎª3.5
    m_laneWidth = LANE_WIDTH;
  }
  m_length = pLane->getLength();
  m_distanceToEnd = pLane->getDistanceToEnd();
  m_name = pLane->getName();
  m_enabled = pLane->isEnable();

  // geom data
  const hadmap::txCurve* pCurve = pLane->getGeometry();
  if (pCurve) {
    CMapCurve::FromTxCurve(m_points, (*pCurve));
    CMapCurve::FormAabb(m_points, m_aabb_ll);
  } else {
    SYSTEM_LOGGER_DEBUG("lane %s curve is null", m_name.c_str());
  }

  /*PointsVec::iterator itr = m_points.begin();
  for (; itr != m_points.end(); ++itr)*/
  /*if (m_points.size() > 0)
  {
          SYSTEM_LOGGER_DEBUG("%4d %4d (%f, %f, %f), (%f, %f, %f)", m_roadID, m_laneID, m_points[0].X()
                  , m_points[0].Y(), m_points[0].Z(), m_points[1].X(), m_points[1].Y(), m_points[1].Z());
  }*/

  // boundary data
  m_boundarys[0] = new CLaneBoundaryData();
  m_boundarys[1] = new CLaneBoundaryData();
  hadmap::txLaneBoundaryPtr pLeft = pLane->getLeftBoundary();
  hadmap::txLaneBoundaryPtr pRight = pLane->getRightBoundary();
  m_boundarys[0]->FromTxLaneBoundary((*pLeft));
  m_boundarys[1]->FromTxLaneBoundary((*pRight));
}

void CLaneData::Lonlat2Local(double refLon, double refLat, double refAlt) {
  /*if (m_points.size() > 0)
  {
          SYSTEM_LOGGER_DEBUG("before %4d %4d (%f, %f, %f), (%f, %f, %f)", m_roadID, m_laneID, m_points[0].X()
                  , m_points[0].Y(), m_points[0].Z(), m_points[1].X(), m_points[1].Y(), m_points[1].Z());
  }*/

  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2Local(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);
  /*if (m_points.size() > 0)
  {
          SYSTEM_LOGGER_DEBUG("after  %4d %4d (%f, %f, %f), (%f, %f, %f)", m_roadID, m_laneID, m_points[0].X()
                  , m_points[0].Y(), m_points[0].Z(), m_points[1].X(), m_points[1].Y(), m_points[1].Z());
  }*/

  for (int i = 0; i < 2; ++i) {
    if (m_boundarys[i]) {
      m_boundarys[i]->Lonlat2Local(refLon, refLat, refAlt);
    }
  }
}

void CLaneData::Lonlat2ENU(double refLon, double refLat, double refAlt) {
  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2ENU(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);

  for (int i = 0; i < 2; ++i) {
    if (m_boundarys[i]) {
      m_boundarys[i]->Lonlat2ENU(refLon, refLat, refAlt);
    }
  }
}

void CLaneData::GenerateMesh() {
  // 判断边界数据是否为空
  bool bEmpty = false;
  for (int i = 0; i < 2; ++i) {
    if (!m_boundarys[i]) {
      bEmpty = true;
    }
  }

  if (bEmpty) {
    SYSTEM_LOGGER_ERROR("lane boundary data is empty");
    return;
  }

  // 获取边界数据
  PointsVec& leftData = m_boundarys[0]->Data();
  PointsVec& rightData = m_boundarys[1]->Data();

  // 获取左右边界数据个数差值
  int nLeftSize = leftData.size();
  int nRightSize = rightData.size();
  int nSize = nLeftSize > nRightSize ? nLeftSize : nRightSize;

  if (nSize <= 1) {
    SYSTEM_LOGGER_INFO("lane boundary data count is small");
    return;
  }

  // 补齐数据差值
  PointsVec points1;
  int nMinBoundaryDataCount = 0;
  int nDifCount = nLeftSize - nRightSize;
  bool bPoint1IsLeft = true;
  if (nDifCount < 0) {
    points1 = leftData;
    nDifCount = -1 * nDifCount;
    nMinBoundaryDataCount = leftData.size();
    bPoint1IsLeft = true;
  } else if (nDifCount > 0) {
    points1 = rightData;
    nMinBoundaryDataCount = rightData.size();
    bPoint1IsLeft = false;
  }

  // 分配三角面buffer
  float* pBuf = new float[(nSize - 1) * 6 * 3];

  PointsVec* pLeft = NULL;
  PointsVec* pRight = NULL;

  // 设置左右边界
  if (nDifCount > 0) {
    // 补齐左右边界相差的数据
    for (int i = 0; i < nDifCount; ++i) {
      points1.push_back(points1[nMinBoundaryDataCount - 1]);
    }
    //
    if (bPoint1IsLeft) {
      pLeft = &points1;
      pRight = &rightData;
    } else {
      pLeft = &leftData;
      pRight = &points1;
    }
  } else {
    pLeft = &leftData;
    pRight = &rightData;
  }

  // 保证面法线朝向z轴正向
  CPoint3D pvec1 = (*pLeft)[0] - (*pLeft)[1];
  CPoint3D pvec2 = (*pRight)[0] - (*pLeft)[1];
  CPoint3D pvec = pvec2.cross(pvec1);
  double tan = pvec.dot(CPoint3D(0, 0, 1));
  if (tan < 0) {
    PointsVec* pTmp = pLeft;
    pLeft = pRight;
    pRight = pTmp;
  }

  // 生成面
  for (int i = 0; i < nSize - 1; ++i) {
    // 第一个面
    pBuf[i * 18 + 0] = (*pLeft)[i].X();
    pBuf[i * 18 + 1] = (*pLeft)[i].Y();
    pBuf[i * 18 + 2] = (*pLeft)[i].Z();

    pBuf[i * 18 + 3] = (*pLeft)[i + 1].X();
    pBuf[i * 18 + 4] = (*pLeft)[i + 1].Y();
    pBuf[i * 18 + 5] = (*pLeft)[i + 1].Z();

    pBuf[i * 18 + 6] = (*pRight)[i].X();
    pBuf[i * 18 + 7] = (*pRight)[i].Y();
    pBuf[i * 18 + 8] = (*pRight)[i].Z();

    // 第二个面
    pBuf[i * 18 + 9] = (*pRight)[i].X();
    pBuf[i * 18 + 10] = (*pRight)[i].Y();
    pBuf[i * 18 + 11] = (*pRight)[i].Z();

    pBuf[i * 18 + 12] = (*pLeft)[i + 1].X();
    pBuf[i * 18 + 13] = (*pLeft)[i + 1].Y();
    pBuf[i * 18 + 14] = (*pLeft)[i + 1].Z();

    pBuf[i * 18 + 15] = (*pRight)[i + 1].X();
    pBuf[i * 18 + 16] = (*pRight)[i + 1].Y();
    pBuf[i * 18 + 17] = (*pRight)[i + 1].Z();
  }

  m_generatedMesh.m_pData = pBuf;
  m_generatedMesh.m_uDataCount = (nSize - 1) * 6 * 3;
  m_generatedMesh.m_uDataSize = m_generatedMesh.m_uDataCount * sizeof(float);
}
