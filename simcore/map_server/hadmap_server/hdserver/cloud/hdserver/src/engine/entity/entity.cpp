/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "entity.h"

CEntity::CEntity() : m_pOwner(NULL) {}

void CEntity::SetOwner(CNode* pOwner) { m_pOwner = pOwner; }
