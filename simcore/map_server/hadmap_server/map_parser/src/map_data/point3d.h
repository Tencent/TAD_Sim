/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

struct tagPoint3d {
 public:
  tagPoint3d();

  float x;
  float y;
  float z;

  void Reset();
};
