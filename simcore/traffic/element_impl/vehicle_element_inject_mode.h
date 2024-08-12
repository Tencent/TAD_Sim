// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <map>
#include <string>

namespace hadmap {
// database operation handle
typedef struct txMapHandle txMapHandle;
}  // namespace hadmap

class InjectImpl {
 public:
  using ParamKVMap = std::map<std::string, std::string>;

 public:
  double DrivingFollow(hadmap::txMapHandle* hdmap_handler, const ParamKVMap& in_param_kvmap) noexcept;
  double DrivingChangeLane(hadmap::txMapHandle* hdmap_handler, const ParamKVMap& in_param_kvmap) noexcept;
};
