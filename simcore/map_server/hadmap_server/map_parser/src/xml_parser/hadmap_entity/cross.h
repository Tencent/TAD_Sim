/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#pragma once
#include <hadmap.h>
#include "common/engine/math/vec3.h"
#include "elements.h"
#include "entity_link2.h"
typedef std::vector<CLinkEntityV2> CrossFromToLinkLines;
typedef std::vector<CrossFromToLinkLines> CrossLinkLinesByFromRoad;
typedef std::vector<CrossLinkLinesByFromRoad> CrossAllLinkLines;
class CCross : public CMapElement {
 public:
  CCross();
  hadmap::txRoadPtr m_txRoad;
  CrossAllLinkLines m_links;
};
