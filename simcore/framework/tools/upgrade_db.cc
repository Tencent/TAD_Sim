#include <iostream>
#include <string>

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/join.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/filesystem.hpp"
#include "sqlite3.h"

// !!! this upgrade scirpt should not include(depends on) any socure file definition in the framework_v2/src
// since definitions in source files would change with version updates, but definitions in the script
// should respect to those ones in each previous versions !!!
// #include "coordinator/sql_defs.h"


void UpgradeFromV0toV1(sqlite3* db) {
  std::cout << "start upgrading from v0 to v1 ..." << std::endl;

  char* err = nullptr;
  int ec = sqlite3_exec(db,
                        "alter table sys_config add auto_reset integer default 1 not null;"
                        "update config_version set version = 1 where rowid = 1;",
                        nullptr, nullptr, &err);
  if (ec != SQLITE_OK) {
    std::cerr << "upgrading from v0 to v1 failed: " << err << std::endl;
    exit(6);
  }

  std::cout << "finished upgrading from v0 to v1." << std::endl;
}


void UpgradeFromV1toV2(sqlite3* db) {
  std::cout << "start upgrading from v1 to v2 ..." << std::endl;

  char* err = nullptr;
  int ec = sqlite3_exec(db,
                        "alter table sys_config add initial_location integer default 1 not null;"
                        "update config_version set version = 2 where rowid = 1;",
                        nullptr, nullptr, &err);
  if (ec != SQLITE_OK) {
    std::cerr << "upgrading from v1 to v2 failed: " << err << std::endl;
    exit(6);
  }

  std::cout << "finished upgrading from v1 to v2." << std::endl;
}


void UpgradeFromV2toV3(sqlite3* db) {
  std::cout << "start upgrading from v2 to v3 ..." << std::endl;

  char* err = nullptr;
  int ec = sqlite3_exec(db,
                        "alter table module_config add shadow_id integer;"
                        "update config_version set version = 3 where rowid = 1;",
                        nullptr, nullptr, &err);
  if (ec != SQLITE_OK) {
    std::cerr << "upgrading from v2 to v3 failed: " << err << std::endl;
    exit(6);
  }

  std::cout << "finished upgrading from v2 to v3." << std::endl;
}


void UpgradeFromV3toV4(sqlite3* db) {
  std::cout << "start upgrading from v3 to v4 ..." << std::endl;

  char* err = nullptr;
  int ec = sqlite3_exec(db,
                        "alter table sys_config add override_user_log integer default 1 not null; "
                        "alter table sys_config add grading_feedback_process text;"
                        "update config_version set version = 4 where rowid = 1;",
                        nullptr, nullptr, &err);
  if (ec != SQLITE_OK) {
    std::cerr << "upgrading from v3 to v4 failed: " << err << std::endl;
    exit(6);
  }

  std::cout << "finished upgrading from v3 to v4." << std::endl;
}


void UpgradeFromV4toV5(sqlite3* db) {
  std::cout << "start upgrading from v4 to v5 ..." << std::endl;

  auto func_update_grading_init_args = [db](const std::string& module_name) {
    char* err = nullptr;
    std::string init_args_str;
    char* sql_str = sqlite3_mprintf("select init_args from module_config where name = %Q", module_name.c_str());
    if (sql_str == nullptr) {
      std::cerr << "upgrading from v4 to v5 failed: sqlite3 printf failed on select." << std::endl;
      exit(6);
    }
    int ec = sqlite3_exec(
        db, sql_str,
        [](void* arg_str, int column_count, char** columns, char** column_names) -> int {
          std::string* s = static_cast<std::string*>(arg_str);
          *s = columns[0];
          return 0;
        },
        &init_args_str, &err);
    sqlite3_free(sql_str);
    if (ec != SQLITE_OK) {
      std::cerr << "get module " << module_name << " init args failed: ";
      if (err != nullptr) {
        std::cerr << err;
        sqlite3_free(err);
      }
      std::cerr << std::endl;
      return;
    }
    std::vector<std::string> init_args;
    boost::split(init_args, init_args_str, boost::is_any_of("\n"));
    bool altered = false;
    for (std::string& args_kv : init_args) {
      const auto idx = args_kv.find_first_of("=");
      const auto key = args_kv.substr(0, idx), value = (idx == std::string::npos ? "" : args_kv.substr(idx + 1));
      if (key == "log-folder" && !value.empty()) {
        boost::filesystem::path p(value);
        p = p.parent_path().parent_path().parent_path();
        if (p.stem() == "tadsim") {
          p /= "data/service_data/sim_data/pblog";
          const std::string altered_value = p.lexically_normal().make_preferred().string();
          args_kv = key + "=" + altered_value;
          altered = true;
        }
      }
      if (key == "post-script" && !value.empty()) {
        boost::filesystem::path p(value);
        p = p.parent_path().parent_path().parent_path();
        if (p.stem() == "tadsim") {
          p /= "sys/service_data/script/post_process";
          std::string altered_value = p.lexically_normal().make_preferred().string();
          args_kv = key + "=" + altered_value;
          altered = true;
        }
      }
    }
    if (altered) {
      init_args_str = boost::join(init_args, "\n");
      sql_str = sqlite3_mprintf("update module_config set init_args = %Q where name = %Q", init_args_str.c_str(),
                                module_name.c_str());
      if (sql_str == nullptr) {
        std::cerr << "upgrading from v4 to v5 failed: sqlite3 printf failed on update." << std::endl;
        exit(6);
      }
      ec = sqlite3_exec(db, sql_str, nullptr, nullptr, &err);
      sqlite3_free(sql_str);
      if (ec == SQLITE_OK) {
        std::cout << "update module " << module_name << " init args succeed." << std::endl;
      } else {
        std::cerr << "update module " << module_name << " init args failed: ";
        if (err != nullptr) {
          std::cerr << err;
          sqlite3_free(err);
        }
        std::cerr << std::endl;
        exit(6);
      }
    }
  };

  func_update_grading_init_args("Grading");
  func_update_grading_init_args("Protobuf_Logger");

  char* err = nullptr;
  int ec = sqlite3_exec(db, "update config_version set version = 5 where rowid = 1;", nullptr, nullptr, &err);
  if (ec != SQLITE_OK) {
    std::cerr << "upgrading from v4 to v5 failed: " << err << std::endl;
    exit(6);
  }
  std::cout << "finished upgrading from v4 to v5." << std::endl;
}

void UpgradeFromV5toV6(sqlite3* db) {
  std::cout << "start upgrading from v5 to v6 ..." << std::endl;

  char* err = nullptr;
  int ec = sqlite3_exec(db,
                        "DROP TABLE grading_kpi;"
                        "CREATE TABLE IF NOT EXISTS grading_kpi (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT "
                        "NULL, parameters TEXT, pass_condition TEXT, finish_condition TEXT, group_id INTEGER NOT NULL);"
                        "CREATE INDEX IF NOT EXISTS group_id_idx ON grading_kpi (group_id, name);",
                        nullptr, nullptr, &err);
  if (ec == SQLITE_OK) {
    std::cout << "upgrading grading_kpi pass_condition and finish_condition from int to TEXT." << std::endl;
  }

  ec = sqlite3_exec(db, "update config_version set version = 6 where rowid = 1;", nullptr, nullptr, &err);
  if (ec != SQLITE_OK) {
    std::cerr << "upgrading from v5 to v6 failed: " << err << std::endl;
    exit(6);
  }

  std::cout << "finished upgrading from v5 to v6." << std::endl;
}

void UpgradeFromV6toV7(sqlite3* db) {
  std::cout << "start upgrading from v6 to v7 ..." << std::endl;

  char* err = nullptr;
  int ec = sqlite3_exec(db, "alter table module_scheme add category TEXT;", nullptr, nullptr, &err);
  if (ec == SQLITE_OK) { std::cout << "upgrading module_scheme add category from to TEXT." << std::endl; }

  ec = sqlite3_exec(db, "update config_version set version = 7 where rowid = 1;", nullptr, nullptr, &err);
  if (ec != SQLITE_OK) {
    std::cerr << "upgrading from v6 to v7 failed: " << err << std::endl;
    exit(6);
  }

  std::cout << "finished upgrading from v6 to v7." << std::endl;
}


int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "usage: upgrade-db path-to-install-dir path-to-user-dir" << std::endl;
    return 1;
  }

  boost::filesystem::path db_path(argv[2]);
  db_path /= "data/service.sqlite";
  if (!boost::filesystem::exists(db_path)) {
    std::cerr << "config db (" << db_path << ") not exists. skip upgrading." << std::endl;
    return 0;
  }
  std::string db_path_str(db_path.lexically_normal().string());

  sqlite3* db = nullptr;
  int db_ec = 0;
  db_ec = sqlite3_open(db_path_str.c_str(), &db);
  if (db_ec != SQLITE_OK || db == nullptr) {
    std::string err = sqlite3_errmsg(db);
    if (db) sqlite3_close_v2(db);
    std::cerr << "open db(" << db_path_str << ") failed: " << err << std::endl;
    return 3;
  }

  char* db_err = nullptr;
  db_ec = sqlite3_exec(db,
                       "create table if not exists config_version(version integer default 0 not null);"
                       "insert or ignore into config_version (rowid, version) values (1, 5);",
                       nullptr, nullptr, &db_err);
  if (db_ec != SQLITE_OK) {
    std::cerr << "create version table failed(" << db_ec << "): " << db_err << std::endl;
    return 4;
  }

  int db_version = 0;
  db_ec = sqlite3_exec(
      db, "select version from config_version where rowid = 1;",
      [](void* data_ptr, int column_count, char** columns, char** column_names) -> int {
        auto v = static_cast<int*>(data_ptr);
        try {
          *v = std::stoi(columns[0]);
          return 0;
        } catch (const std::exception& e) {
          std::cerr << "error getting version column value from config_version table: " << e.what() << std::endl;
          return -1;
        }
      },
      &db_version, &db_err);
  if (db_ec != SQLITE_OK) {
    std::cerr << "retrieving config verson failed(" << db_ec << "): " << db_err << std::endl;
    return 5;
  }

  // clang-format off
  switch (db_version) {
    case 0: UpgradeFromV0toV1(db);
    case 1: UpgradeFromV1toV2(db);
    case 2: UpgradeFromV2toV3(db);
    case 3: UpgradeFromV3toV4(db);
    case 4: UpgradeFromV4toV5(db);
    case 5: UpgradeFromV5toV6(db);
    case 6: UpgradeFromV6toV7(db);
          break;
    default: std::cout << "already newest config db version: " << db_version << std::endl; return 0;
  }
  // clang-format on

  std::cout << "config db upgrading finished successfully." << std::endl;
  return 0;
}