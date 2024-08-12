/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "./aabb.h"

class CNode;

enum ENTITY_TYPE {
  ENTITY_TYPE_Hadmap = 0,
  ENTITY_TYPE_Lane,
  ENTITY_TYPE_Lane_Boundary,
  ENTITY_TYPE_Lane_Link,
  ENTITY_TYPE_Section,
  ENTITY_TYPE_Road,
  ENTITY_TYPE_MapObject,
  ENTITY_TYPE_Mesh,
};

class CEntity {
 public:
  CEntity();
  virtual ~CEntity() {}

  ENTITY_TYPE Type() { return m_type; }
  void SetType(ENTITY_TYPE type) { m_type = type; }

  CAABB& AabbLl() { return m_aabb_ll; }
  CAABB& AabbXyz() { return m_aabb_xyz; }
  CNode* Owner() { return m_pOwner; }
  void SetOwner(CNode* pOwner);

 protected:
  ENTITY_TYPE m_type;
  CAABB m_aabb_ll;
  CAABB m_aabb_xyz;

  CNode* m_pOwner;
};
