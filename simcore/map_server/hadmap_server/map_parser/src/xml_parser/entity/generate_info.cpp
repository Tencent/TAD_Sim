/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "generate_info.h"

CGenerateInfo::CGenerateInfo() { Reset(); }

void CGenerateInfo::Reset() {
  m_strGenerateInfo = "";
  m_strOriginFile = "";
}
