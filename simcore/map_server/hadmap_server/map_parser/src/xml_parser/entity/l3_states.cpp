/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "l3_states.h"

bool CL3States::IsEmpty() { return m_states.size() == 0; }

void CL3States::AddState(tagStateItem& item) { m_states.push_back(item); }
