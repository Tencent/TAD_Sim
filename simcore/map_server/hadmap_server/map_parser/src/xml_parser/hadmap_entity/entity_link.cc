/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "entity_link.h"

CLinkEntity::CLinkEntity() { Reset(); }

void CLinkEntity::Reset() {
  m_from_id = -1;
  m_from_end = -1;
  m_to_id = -1;
  m_to_end = -1;
}