// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "transmission/post_param.h"

#include <set>

namespace hadmap {
void generatePostJson(const std::vector<PostParamPtr>& params, std::string& jsonStr) {
  neb::CJsonObject paramJsObj;
  std::set<PostParam::ParamType> typeSets;
  for (auto& paramPtr : params) {
    if (typeSets.find(paramPtr->getType()) == typeSets.end()) {
      typeSets.insert(paramPtr->getType());
      neb::CJsonObject pAry;
      paramPtr->addToJson(pAry);
      paramJsObj.Add(paramPtr->typeStr(), pAry);
    } else {
      paramPtr->addToJson(paramJsObj[paramPtr->typeStr()]);
    }
  }
  jsonStr = paramJsObj.ToString();
}
}  // namespace hadmap
