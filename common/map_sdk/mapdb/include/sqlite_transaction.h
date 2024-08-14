// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

struct sqlite3;
struct sqlite3_stmt;

#include <string>

namespace hadmap {
class sqlitePackage {
 public:
  // check table exists
  static bool isTableExists(sqlite3* dbPtr, const std::string& tableName);

  // execute sql
  static bool execSql(sqlite3* dbPtr, const std::string& sql);

  // drop table
  static bool dropTable(sqlite3* dbPtr, const std::string& tableName);

  // clear table
  static bool clearTable(sqlite3* dbPtr, const std::string& tableName);

  // initialize spatial metadata
  static bool initSpatialMeta(sqlite3* dbPtr);

  // add geometry column
  static bool addGeomColumn(sqlite3* dbPtr, const std::string& tableName, const std::string& columnName,
                            const int& srsId, const std::string& geomType, const std::string& dimension);

  // this function must follow an insert statement
  static int getLastInsertedRowid(sqlite3* dbPtr);
};

class sqliteStmtPtr {
 private:
  bool stat;
  sqlite3_stmt* ptr;
  const char* tail;

 public:
  sqliteStmtPtr();

  sqliteStmtPtr(sqlite3* dbPtr, const std::string& sql);

  ~sqliteStmtPtr();

 public:
  bool prepare(sqlite3* dbPtr, const std::string& sql);

  void finalize();

  bool status() { return stat; }

  operator sqlite3_stmt**() { return &ptr; }

  operator sqlite3_stmt*() { return ptr; }
};
}  // namespace hadmap
