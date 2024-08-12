/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once
#include <string>
#include <vector>

class CParseTools {
 public:
  static void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);
  static void SplitString2(const std::string& s, std::vector<std::string>& v, const std::string& c);
};
