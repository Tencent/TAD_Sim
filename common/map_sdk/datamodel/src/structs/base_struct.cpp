// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/base_struct.h"

#include <string>

namespace hadmap {
//
// txLinkObject
//
txLinkObject::txLinkObject() {}

void txLinkObject::addPrev(const std::string& preId) {
  // check if linkObjPtr is already in prev

  prev.push_back(preId);
}

void txLinkObject::addNext(const std::string& nextId) {
  // check if linkobjptr is already in next

  next.push_back(nextId);
}
bool txLinkObject::getPrev(std::vector<std::string>& ids) const {
  ids.assign(prev.begin(), prev.end());
  return !ids.empty();
}

bool txLinkObject::getNext(std::vector<std::string>& ids) const {
  ids.assign(next.begin(), next.end());
  return !ids.empty();
}

std::string txLinkObject::getUniqueId() const { return std::string("base_class"); }
}  // namespace hadmap
