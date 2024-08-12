// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "common/macros.h"
#include "db_operation.h"

#include <memory>

namespace hadmap {
class TXSIMMAP_API dbFactory {
 private:
  static dbFactory* insPtr;

 private:
  dbFactory();

  dbFactory(const dbFactory& fac);

  ~dbFactory();

 public:
  static dbFactory* getInstance();

 public:
  // get sqlite operation
  dbOperation* getSqliteOp(const std::string& path);

 public:
  // release db operation
  bool releaseDB(dbOperation** opPtr);
};
}  // namespace hadmap
