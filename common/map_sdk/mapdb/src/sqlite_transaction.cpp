// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "sqlite_transaction.h"
#include "common/log.h"

#include <sqlite3.h>

#include <iostream>

namespace hadmap {
bool sqlitePackage::isTableExists(sqlite3* dbPtr, const std::string& tableName) {
  char sql[512];
  sprintf(sql, "SELECT MAX(ROWID) FROM sqlite_master WHERE name='%s'", tableName.c_str());

  sqliteStmtPtr stmtPtr(dbPtr, std::string(sql));
  if (!stmtPtr.status()) {
    std::cout << "Error " << std::string(sql) << std::endl;
    return false;
  }

  sqlite3_step(stmtPtr);
  int num = sqlite3_column_int(stmtPtr, 0);

  return num != 0;
}

bool sqlitePackage::execSql(sqlite3* dbPtr, const std::string& sql) {
  char* err = NULL;
  if (SQLITE_OK != sqlite3_exec(dbPtr, sql.c_str(), 0, 0, &err)) {
    txlog::printf("#Error: %s\n", sql.c_str());
    txlog::printf("#Reason: %s\n", err);
    sqlite3_free(err);
    return false;
  }
  return true;
}

bool sqlitePackage::dropTable(sqlite3* dbPtr, const std::string& tableName) {
  char sql[512];
  sprintf(sql, "DROP TABLE %s", tableName.c_str());
  return sqlitePackage::execSql(dbPtr, sql);
}

bool sqlitePackage::clearTable(sqlite3* dbPtr, const std::string& tableName) {
  char sql[512];
  sprintf(sql, "DELETE FROM %s", tableName.c_str());
  return sqlitePackage::execSql(dbPtr, sql);
}

bool sqlitePackage::initSpatialMeta(sqlite3* dbPtr) {
  if (sqlitePackage::isTableExists(dbPtr, "spatial_ref_sys")) return true;
  char sql[512];
  sprintf(sql, "SELECT InitSpatialMetadata(1)");
  return sqlitePackage::execSql(dbPtr, sql);
}

bool sqlitePackage::addGeomColumn(sqlite3* dbPtr, const std::string& tableName, const std::string& columnName,
                                  const int& srsId, const std::string& geomType, const std::string& dimension) {
  char sql[512];
  sprintf(sql, "SELECT AddGeometryColumn('%s', '%s', %d, '%s', '%s')", tableName.c_str(), columnName.c_str(), srsId,
          geomType.c_str(), dimension.c_str());
  if (!sqlitePackage::execSql(dbPtr, sql)) return false;

  sprintf(sql, "SELECT CreateSpatialIndex('%s', '%s')", tableName.c_str(), columnName.c_str());
  return sqlitePackage::execSql(dbPtr, sql);
}

int sqlitePackage::getLastInsertedRowid(sqlite3* dbPtr) {
  sqlite3_stmt* stmt = 0;
  const char* tail = 0;

  char sql[512];
  sprintf(sql, "SELECT last_insert_rowid()");
  if (SQLITE_OK != sqlite3_prepare_v2(dbPtr, sql, -1, &stmt, &tail)) {
    sqlite3_finalize(stmt);
    return -1;
  }

  sqlite3_step(stmt);
  int pkid = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);

  return pkid;
}

sqliteStmtPtr::sqliteStmtPtr() : stat(false), ptr(NULL), tail(NULL) {}

sqliteStmtPtr::sqliteStmtPtr(sqlite3* dbPtr, const std::string& sql) : stat(false), ptr(NULL), tail(NULL) {
  prepare(dbPtr, sql);
}

sqliteStmtPtr::~sqliteStmtPtr() { finalize(); }

void sqliteStmtPtr::finalize() {
  if (ptr != NULL) {
    sqlite3_finalize(ptr);
    ptr = NULL;
  }
}

bool sqliteStmtPtr::prepare(sqlite3* dbPtr, const std::string& sql) {
  int code = sqlite3_prepare_v2(dbPtr, sql.c_str(), -1, &ptr, &tail);
  if (SQLITE_OK != code) {
    txlog::printf("#Error: %s\n", sql.c_str());
    // txlog::printf( "%s\n", tail );
    stat = false;
  } else {
    stat = true;
  }
  return stat;
}
}  // namespace hadmap
