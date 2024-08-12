/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

class TiXmlElement;

class CGrading {
 public:
  int Parse(const char* strGradingFile);
  int Save(const char* strGradingFile);
  int Delete(const char* strGradingFile);
};
