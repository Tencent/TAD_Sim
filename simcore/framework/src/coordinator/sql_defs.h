// Copyright 2024 Tencent Inc. All rights reserved.
//

// clang-format off

// =========================================================================
// ============================ common sqls ================================
// =========================================================================

#define TXSIM_SQL_TABLE_NAME_MODULE_CONFIG "module_config"
#define TXSIM_SQL_TABLE_NAME_MODULE_SCHEME "module_scheme"
#define TXSIM_SQL_TABLE_NAME_MULTIEGO_SCHEME "multiego_scheme"
#define TXSIM_SQL_TABLE_NAME_SYS_CONFIG    "sys_config"
#define TXSIM_SQL_TABLE_NAME_PLAY_LIST     "play_list"
#define TXSIM_SQL_TABLE_NAME_GRADING_KPI   "grading_kpi"
#define TXSIM_SQL_TABLE_NAME_KPI_GROUP     "kpi_group"

#define TXSIM_SQL_GET_CONFIG_COUNT "SELECT COUNT(*) FROM "

// =========================================================================
// ========================= module config sqls ============================
// =========================================================================

#define TXSIM_SQL_CREATE_MODULE_CONFIG_TABLE                                                                         \
  "CREATE TABLE IF NOT EXISTS " TXSIM_SQL_TABLE_NAME_MODULE_CONFIG                                                   \
  "(name TEXT PRIMARY KEY NOT NULL, exec_period INTEGER, response_time INTEGER, endpoint TEXT, init_args TEXT, "     \
  "auto_launch INTEGER DEFAULT 0 NOT NULL, dep_paths TEXT, so_path TEXT, bin_path TEXT, bin_args TEXT, cmd_timeout " \
  "INTEGER, step_timeout INTEGER, category INTEGER DEFAULT 0 NOT NULL, shadow_id INTEGER)"

#define TXSIM_SQL_MODULE_CONFIG_COLUMN_NAMES "name, exec_period, response_time, init_args, auto_launch," \
  " dep_paths, so_path, bin_path, bin_args, cmd_timeout, step_timeout, category, shadow_id"

#define TXSIM_SQL_MODULE_CONFIG_COLUMN_VALUES "%Q, %u, %u, %Q, %d, %Q, %Q, %Q, %Q, %u, %u, %d, %d"

#define TXSIM_SQL_MODULE_CONFIG_COLUMN_UPDATES "exec_period = %u, response_time = %u, init_args = %Q," \
  " auto_launch = %d, dep_paths = %Q, so_path = %Q, bin_path = %Q, bin_args = %Q, cmd_timeout = %u, step_timeout ="   \
  " %u, category = %d, shadow_id = %d"

#define TXSIM_SQL_MODULE_CONFIG_COLUMN_UPSERTS "exec_period = %u, response_time = %u, auto_launch = %d," \
  " dep_paths = %Q, so_path = %Q, bin_path = %Q, bin_args = %Q, cmd_timeout = %u, step_timeout = %u, category = %d," \
  " shadow_id = %d"

#define TXSIM_SQL_UPSERT_MODULE_CONFIG                                                        \
  "INSERT INTO " TXSIM_SQL_TABLE_NAME_MODULE_CONFIG " (" TXSIM_SQL_MODULE_CONFIG_COLUMN_NAMES ") VALUES ("\
  TXSIM_SQL_MODULE_CONFIG_COLUMN_VALUES ") ON CONFLICT(name) DO UPDATE SET " TXSIM_SQL_MODULE_CONFIG_COLUMN_UPSERTS ";"

#define TXSIM_SQL_INSERT_MODULE_CONFIG                                                        \
  "INSERT INTO " TXSIM_SQL_TABLE_NAME_MODULE_CONFIG " (" TXSIM_SQL_MODULE_CONFIG_COLUMN_NAMES \
  ") VALUES (" TXSIM_SQL_MODULE_CONFIG_COLUMN_VALUES ");"

#define TXSIM_SQL_DELETE_MODULE_CONFIG \
  "DELETE FROM " TXSIM_SQL_TABLE_NAME_MODULE_CONFIG " WHERE name = %Q;"

#define TXSIM_SQL_UPDATE_MODULE_CONFIG \
  "UPDATE " TXSIM_SQL_TABLE_NAME_MODULE_CONFIG " SET " TXSIM_SQL_MODULE_CONFIG_COLUMN_UPDATES " WHERE name = %Q;"

#define TXSIM_SQL_GET_MODULE_CONFIG \
  "SELECT " TXSIM_SQL_MODULE_CONFIG_COLUMN_NAMES " FROM " TXSIM_SQL_TABLE_NAME_MODULE_CONFIG " WHERE name = %Q;"

#define TXSIM_SQL_GET_ALL_MODULE_CONFIGS \
  "SELECT " TXSIM_SQL_MODULE_CONFIG_COLUMN_NAMES " FROM " TXSIM_SQL_TABLE_NAME_MODULE_CONFIG " ORDER BY name ASC;"

#define TXSIM_SQL_GET_ACTIVE_MODULES \
  "SELECT " TXSIM_SQL_MODULE_CONFIG_COLUMN_NAMES " FROM " TXSIM_SQL_TABLE_NAME_MODULE_CONFIG " WHERE name IN "

// =========================================================================
// ========================= module scheme sqls ============================
// =========================================================================

#define TXSIM_SQL_CREATE_MODULE_SCHEME_TABLE                       \
  "CREATE TABLE IF NOT EXISTS " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME \
  "(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL UNIQUE, modules TEXT," \
  "active INTEGER DEFAULT 0 NOT NULL, category TEXT)"

#define TXSIM_SQL_UPSERT_MODULE_SCHEME                                                                            \
  "INSERT INTO " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " (id, name, modules, category)" \
  " VALUES (%d, %Q, %Q, %Q) ON CONFLICT(id) DO" \
  " UPDATE SET name = %Q, modules = %Q;"

#define TXSIM_SQL_ADD_MODULE_SCHEME \
  "INSERT INTO " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " (name, modules, category) VALUES (%Q, %Q, %Q);"

#define TXSIM_SQL_REMOVE_MODULE_SCHEME \
  "DELETE FROM " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " WHERE id = %u;"

#define TXSIM_SQL_UPDATE_MODULE_SCHEME \
  "UPDATE " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " SET name = %Q, modules = %Q, category = %Q WHERE id = %u;"

#define TXSIM_SQL_CLEAR_ALL_MODULE_SCHEME_ACTIVES \
  "UPDATE " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " SET active = 0;"

#define TXSIM_SQL_SET_ACTIVE_MODULE_SCHEME \
  "UPDATE " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " SET active = 1 WHERE id = %d;"

#define TXSIM_SQL_GET_MODULE_SCHEME \
  "SELECT id, modules, category FROM " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " WHERE name = %Q;"

#define TXSIM_SQL_GET_MODULE_SCHEME_BY_ID \
  "SELECT id, modules FROM " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " WHERE id = %d;"

#define TXSIM_SQL_GET_ALL_MODULE_SCHEMES \
  "SELECT id, name, modules, active, category FROM " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " ORDER BY id ASC;"

#define TXSIM_SQL_GET_ACTIVE_MODULE_LIST \
  "SELECT modules FROM " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " WHERE active = 1;"

#define TXSIM_SQL_GET_ACTIVE_MODULE_CATEGORY \
  "SELECT category FROM " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " WHERE active = 1;"


#define TXSIM_SQL_GET_ACTIVE_SCHEME_ID \
  "SELECT id FROM " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " WHERE active = 1;"

#define TXSIM_SQL_GET_LAST_SCHEME_ID \
  "SELECT id FROM " TXSIM_SQL_TABLE_NAME_MODULE_SCHEME " WHERE id < %d order by id desc limit 1;"


#define TXSIM_SQL_CREATE_MULTIEGO_SCHEME_TABLE \
  "CREATE TABLE IF NOT EXISTS " TXSIM_SQL_TABLE_NAME_MULTIEGO_SCHEME \
  "(name TEXT PRIMARY KEY NOT NULL, scheme_id INTEGER, type INTEGER)"

#define TXSIM_SQL_UPSERT_MULTIEGO_SCHEME \
  "INSERT INTO " TXSIM_SQL_TABLE_NAME_MULTIEGO_SCHEME " (name, scheme_id, type) VALUES" \
  " (%Q, %d, %d) ON CONFLICT(name) DO" \
  " UPDATE SET scheme_id = %d, type = %d;"

#define TXSIM_SQL_RESET_MULTIEGO_SCHEME \
  "DELETE FROM " TXSIM_SQL_TABLE_NAME_MULTIEGO_SCHEME ";"

#define TXSIM_SQL_GET_ALL_MULTIEGO_SCHEME \
  "SELECT name, scheme_id, type FROM " TXSIM_SQL_TABLE_NAME_MULTIEGO_SCHEME " ORDER BY name ASC;"
// =========================================================================
// ========================= system config sqls ============================
// =========================================================================

#define TXSIM_SQL_CREATE_SYS_CONFIG_TABLE                                                                  \
  "CREATE TABLE IF NOT EXISTS " TXSIM_SQL_TABLE_NAME_SYS_CONFIG                                            \
  "(play_rate INTEGER, scenario_limit INTEGER, coord_mode INTEGER, auto_reset INTEGER DEFAULT 1 NOT NULL," \
  " initial_location INTEGER DEFAULT 1 NOT NULL, override_user_log INTEGER DEFAULT 1 NOT NULL,"            \
  " grading_feedback_process TEXT)"

#define TXSIM_SQL_SYS_CONFIG_COLUMN_NAMES "play_rate, scenario_limit, coord_mode, auto_reset, initial_location," \
  " override_user_log, grading_feedback_process"

#define TXSIM_SQL_SYS_CONFIG_COLUMN_VALUES "%u, %u, %u, %u, %u, %u, %Q"

#define TXSIM_SQL_INSERT_SYS_CONFIG                                                                  \
  "INSERT INTO " TXSIM_SQL_TABLE_NAME_SYS_CONFIG " (" TXSIM_SQL_SYS_CONFIG_COLUMN_NAMES ") VALUES (" \
  TXSIM_SQL_SYS_CONFIG_COLUMN_VALUES ");"

#define TXSIM_SQL_UPDATE_SYS_CONFIG                                                                                   \
  "UPDATE " TXSIM_SQL_TABLE_NAME_SYS_CONFIG " SET play_rate = %u, scenario_limit = %u, coord_mode = %u, auto_reset =" \
  " %u, initial_location = %u, override_user_log = %u, grading_feedback_process = %Q WHERE rowid = 1;"

#define TXSIM_SQL_SELECT_SYS_CONFIG \
  "SELECT " TXSIM_SQL_SYS_CONFIG_COLUMN_NAMES " FROM " TXSIM_SQL_TABLE_NAME_SYS_CONFIG " WHERE rowid = 1;"

// =========================================================================
// =========================== play list sqls ==============================
// =========================================================================

#define TXSIM_SQL_CREATE_PLAY_LIST_TABLE                                              \
  "CREATE TABLE IF NOT EXISTS " TXSIM_SQL_TABLE_NAME_PLAY_LIST "(scenario_ids TEXT);" \
  "INSERT OR IGNORE INTO " TXSIM_SQL_TABLE_NAME_PLAY_LIST " (rowid, scenario_ids) VALUES (1, '{\"scenarios\": []}')"

#define TXSIM_SQL_UPDATE_PLAY_LIST \
  "UPDATE " TXSIM_SQL_TABLE_NAME_PLAY_LIST " SET scenario_ids = %Q WHERE rowid = 1;"

#define TXSIM_SQL_GET_PLAY_LIST \
  "SELECT scenario_ids FROM " TXSIM_SQL_TABLE_NAME_PLAY_LIST " WHERE rowid = 1;"

// =========================================================================
// ========================== grading kpi sqls =============================
// =========================================================================

#define TXSIM_SQL_CREATE_GRADING_KPI_TABLE                                                                        \
  "CREATE TABLE IF NOT EXISTS " TXSIM_SQL_TABLE_NAME_GRADING_KPI                                                  \
  " (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, parameters TEXT, pass_condition TEXT," \
  " finish_condition TEXT, group_id INTEGER NOT NULL);"                                               \
  "CREATE INDEX IF NOT EXISTS group_id_idx ON " TXSIM_SQL_TABLE_NAME_GRADING_KPI "(group_id, name);"

#define TXSIM_SQL_CREATE_KPI_GROUP_TABLE                                                                              \
  "CREATE TABLE IF NOT EXISTS " TXSIM_SQL_TABLE_NAME_KPI_GROUP                                                        \
  " (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, active INTEGER DEFAULT 0 NOT NULL, file_path TEXT NOT" \
  " NULL, dirty INTEGER DEFAULT 1 NOT NULL)"

#define TXSIM_SQL_ADD_KPI                                                                                           \
  "UPDATE " TXSIM_SQL_TABLE_NAME_KPI_GROUP " SET dirty = 1 WHERE id = %u;"                                          \
  "INSERT INTO " TXSIM_SQL_TABLE_NAME_GRADING_KPI " (name, parameters, pass_condition, finish_condition, group_id)" \
  " VALUES (%Q, %Q, %Q, %Q, %u);"

#define TXSIM_SQL_REMOVE_KPI                                                                 \
  "UPDATE " TXSIM_SQL_TABLE_NAME_KPI_GROUP " SET dirty = 1 WHERE id = (SELECT group_id FROM" \
  " " TXSIM_SQL_TABLE_NAME_GRADING_KPI " WHERE id = %d);"                                    \
  "DELETE FROM " TXSIM_SQL_TABLE_NAME_GRADING_KPI " WHERE id = %d;"

#define TXSIM_SQL_UPDATE_KPI                                                                                    \
  "UPDATE " TXSIM_SQL_TABLE_NAME_KPI_GROUP " SET dirty = 1 WHERE id = (SELECT group_id FROM"                    \
  " " TXSIM_SQL_TABLE_NAME_GRADING_KPI " WHERE id = %d);"                                                       \
  "UPDATE " TXSIM_SQL_TABLE_NAME_GRADING_KPI " SET parameters = %Q, pass_condition = %Q, finish_condition = %Q" \
  " WHERE id = %d;"

#define TXSIM_SQL_GET_ALL_KPI_IN_GROUP                                                                            \
  "SELECT id, name, parameters, pass_condition, finish_condition FROM " TXSIM_SQL_TABLE_NAME_GRADING_KPI " WHERE" \
  " group_id = %d ORDER BY name ASC;"

#define TXSIM_SQL_ADD_KPI_GROUP \
  "INSERT INTO " TXSIM_SQL_TABLE_NAME_KPI_GROUP " (name, file_path) VALUES (%Q, '');"

#define TXSIM_SQL_REMOVE_KPI_GROUP                                                                              \
  "DELETE FROM " TXSIM_SQL_TABLE_NAME_KPI_GROUP " WHERE id = %d; DELETE FROM " TXSIM_SQL_TABLE_NAME_GRADING_KPI \
  " WHERE group_id = %d;"

#define TXSIM_SQL_UPDATE_KPI_GROUP \
  "UPDATE " TXSIM_SQL_TABLE_NAME_KPI_GROUP " SET name = %Q WHERE id = %d;"

#define TXSIM_SQL_GET_ALL_KPI_GROUP \
  "SELECT id, name, active, file_path FROM " TXSIM_SQL_TABLE_NAME_KPI_GROUP " ORDER BY id ASC;"

#define TXSIM_SQL_SET_ACTIVE_KPI_GROUP                                                               \
  "UPDATE " TXSIM_SQL_TABLE_NAME_KPI_GROUP " SET active = 0; UPDATE " TXSIM_SQL_TABLE_NAME_KPI_GROUP \
  " SET active = 1 WHERE id = %d;"

#define TXSIM_SQL_GET_DIRTY_KPI_GROUP_IDS \
  "SELECT id FROM " TXSIM_SQL_TABLE_NAME_KPI_GROUP " WHERE dirty = 1;"

#define TXSIM_SQL_CLEAR_DIRTY_KPI_GROUPS \
  "UPDATE " TXSIM_SQL_TABLE_NAME_KPI_GROUP " SET dirty = 0;"

#define TXSIM_SQL_UPDATE_KPI_FILE_NAME \
  "UPDATE " TXSIM_SQL_TABLE_NAME_KPI_GROUP " SET file_path = %Q WHERE id = %d;"

// clang-format on

namespace tx_sim {
namespace impl {

static const std::string kSeparatorInitArgKV = "=";
static const std::string kSeparatorInitArgList = "\n";
static const std::string kSeparatorDepPathList = ";";
static const std::string kSeparatorModuleList = "\n";

static const std::string kJsonKeyModuleSchemes = "scheme";
static const std::string kJsonKeyExportedSchemeName = "name";
static const std::string kJsonKeyExportedSchemeCategory = "category";
static const std::string kJsonKeyExportedSchemeModules = "modules";
static const std::string kJsonKeyScenarioList = "scenarios";
static const int64_t kDefaultActiveModuleSchemeId = 0;
static const int64_t kInvalidId = std::numeric_limits<int64_t>::min();
static const int64_t kMockGroupIdWithAllDefaultGradingKpi = -1;
static const int64_t kDefaultGradingKpiGroupId = 0;

enum SqlStatFormat : uint8_t {
  kSqlStateInsert = 1,
  kSqlStateUpdate = 2,
  kSqlStateUpsert = 3,
};

}  // namespace impl
}  // namespace tx_sim
