/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

class CLinkEntity {
 public:
  CLinkEntity();

  void Reset();

  int m_from_id;
  int m_from_end;
  int m_to_id;
  int m_to_end;
};
