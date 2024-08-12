/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

// class dbOperation;
#include <db_factory.h>
#include <db_operation.h>
// #include <map_import/ParseInterface.h>
#include <ParseInterface.h>
#include <structs/hadmap_curve.h>
#include <structs/hadmap_section.h>

#include <common/mapdata_trans.h>
#include <structs/hadmap_map.h>

class CMapOperator {
 public:
  void insertIntoSqlite(const char* strName, hadmap::txRoads& roads, hadmap::txLaneLinks& links);

 protected:
  int openDB(hadmap::dbOperation** db_ptr, const char* db_file);
  int closeDB(hadmap::dbOperation** db_ptr);
};
