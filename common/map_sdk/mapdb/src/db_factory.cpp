// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "db_factory.h"
#include "sqlite_operation.h"

namespace hadmap {
dbFactory* dbFactory::insPtr = NULL;

dbFactory::dbFactory() {}

dbFactory::dbFactory(const dbFactory& fac) {}

dbFactory::~dbFactory() {}

dbFactory* dbFactory::getInstance() {
  if (NULL == insPtr) insPtr = new dbFactory;
  return insPtr;
}

dbOperation* dbFactory::getSqliteOp(const std::string& path) {
  sqliteOperation* sqlitePtr = new sqliteOperation(path);
  return sqlitePtr;
}

bool dbFactory::releaseDB(dbOperation** dbPtr) {
  if (dbPtr != NULL && (*dbPtr) != NULL) {
    delete (*dbPtr);
  } else {
    return false;
  }
  *dbPtr = NULL;
  return true;
}
}  // namespace hadmap
