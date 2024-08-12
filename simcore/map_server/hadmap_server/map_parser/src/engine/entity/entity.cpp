/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "entity.h"

// #include "../scene/node.h"

CEntity::CEntity() : m_pOwner(NULL) {}

void CEntity::SetOwner(CNode* pOwner) { m_pOwner = pOwner; }
