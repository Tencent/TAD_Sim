/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/parser_tools.h"
#include <boost/algorithm/string.hpp>

void CParseTools::SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c) {
  if (s.empty()) return;
  std::string::size_type pos1, pos2;
  pos2 = s.find(c);
  pos1 = 0;
  while (std::string::npos != pos2) {
    v.push_back(s.substr(pos1, pos2 - pos1));

    pos1 = pos2 + c.size();
    pos2 = s.find(c, pos1);
  }
  if (pos1 != s.length()) v.push_back(s.substr(pos1));
}

void CParseTools::SplitString2(const std::string& s, std::vector<std::string>& v, const std::string& c) {
  boost::algorithm::split(v, s, boost::algorithm::is_any_of(c));
}
