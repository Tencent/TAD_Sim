/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "point3d.h"

tagPoint3d::tagPoint3d() { Reset(); }

void tagPoint3d::Reset() {
  x = 0.0f;
  y = 0.0f;
  z = 0.0f;
}
