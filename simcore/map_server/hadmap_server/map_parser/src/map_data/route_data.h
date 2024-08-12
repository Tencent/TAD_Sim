/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

class CBillboard;
class CRoute;
class CHadmap;
class CCube;

class CRouteData {
 public:
  explicit CRouteData(CHadmap* pHadmap);
  ~CRouteData();

  void Clear();
  void SetRouteData(CRoute* pRoute);
  CBillboard* BBStart() { return m_pBBStart; }
  CBillboard* BBMid() { return m_pBBMid; }
  CBillboard* BBEnd() { return m_pBBEnd; }
  CRoute* RouteData() { return m_pRoute; }
  int Type();

  void Draw();

 protected:
  float m_fBB2CubeDist;

  CBillboard* m_pBBStart;
  CBillboard* m_pBBMid;
  CBillboard* m_pBBEnd;

  CCube* m_pCubeStart;
  CCube* m_pCubeMid;
  CCube* m_pCubeEnd;

  CRoute* m_pRoute;

  CHadmap* m_pHadmap;
};
