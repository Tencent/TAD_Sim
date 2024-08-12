// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_od_map_system_sqlite.h"
#include "tx_od_api_assist.h"
#include "tx_od_path_utils.h"

TX_NAMESPACE_OPEN(Server)
void txMapSystemSqlite::clear() TX_NOEXCEPT {
  if (NonNull_Pointer(m_data)) {
    // m_data.;
  }
  m_data = nullptr;
  ParentClass::clear();
}

tx_od_error_info txMapSystemSqlite::load(const Base::txString& strMapPath) TX_NOEXCEPT {
  using namespace boost::filesystem;
  using FilePath = boost::filesystem::path;
  FilePath p(strMapPath);
  if (exists(p)) {
    if (Null_Pointer(m_data)) {
      m_data = std::make_shared<txMapDataCache>();
    }
    if (m_data->LoadHadMap(strMapPath.c_str()) == "false") {
      return Utils::make_error_code(txErrorCode::EC_FILE_LOAD_FAIL, txString("map file load failure."));
    } else {
      return Utils::make_error_code(txErrorCode::EC_SUCCESS, txString("map file load success."));
    }
  } else {
    return Utils::make_error_code(txErrorCode::EC_FILE_NOT_FOUND, txString("map file not find."));
  }
}

tx_od_error_info txMapSystemSqlite::create(const Base::txString& outMapPath,
                                           const txStringStream&& cmd_json) TX_NOEXCEPT {
  LogInfo << "start create hadmap!";
  using namespace boost::filesystem;
  using FilePath = boost::filesystem::path;
  if (Null_Pointer(m_data)) {
    m_data = std::make_shared<txMapDataCache>();
  }
  if (m_data->CreateHadMap(outMapPath.c_str(), cmd_json) == std::string("false")) {
    return Utils::make_error_code(txErrorCode::EC_FILE_CREATE_FAIL, txString("map file create failure."));
  } else {
    return Utils::make_error_code(txErrorCode::EC_SUCCESS, txString("map file create success."));
  }
}

tx_od_error_info txMapSystemSqlite::save(const Base::txString& strMapPath,
                                         const Base::txString& outMapPath) TX_NOEXCEPT {
  if (NonNull_Pointer(m_data)) {
    if (m_data->SaveHadMap(strMapPath.c_str(), outMapPath.c_str()) == std::string("false")) {
      return Utils::make_error_code(txErrorCode::EC_FILE_CREATE_FAIL, txString("map file save failure."));
    } else {
      return Utils::make_error_code(txErrorCode::EC_SUCCESS, txString(""));
    }
  } else {
    return Utils::make_error_code(txErrorCode::EC_SUCCESS, txString(""));
  }
}

tx_od_error_info txMapSystemSqlite::modify(const Base::txString& strMapPath,
                                           Base::txStringStream&& cmd_json) TX_NOEXCEPT {
  if (NonNull_Pointer(m_data)) {
    const char* data = m_data->ModifyHadMap(strMapPath.c_str(), cmd_json);
    if (std::string(data) == std::string("false")) {
      return Utils::make_error_code(txErrorCode::EC_INVALID_INPUT, txString(""));
    }
    return Utils::make_error_code(txErrorCode::EC_SUCCESS, txString(""));
  }
  return Utils::make_error_code(txErrorCode::EC_DATA_IS_EMPTY, txString("data is empty"));
}

tx_od_error_info txMapSystemSqlite::makestring(Base::txStringStream& cmd_json) TX_NOEXCEPT {
  if (NonNull_Pointer(m_data)) {
    const char* retChar = m_data->ToJson(cmd_json);
    if (std::string(retChar) == std::string("true")) {
      return Utils::make_error_code(txErrorCode::EC_SUCCESS, txString("success"));
    } else {
      return Utils::make_error_code(txErrorCode::EC_DATA_IS_ERROR, txString(retChar));
    }
  }
  return Utils::make_error_code(txErrorCode::EC_DATA_IS_EMPTY, txString("Data is null!"));
}

tx_od_error_info txMapSystemSqlite::close(const Base::txString& strMapPath) TX_NOEXCEPT {
  if (NonNull_Pointer(m_data)) {
    const char* data = m_data->CloseHadMap(strMapPath.c_str());
    if (std::string(data) == std::string("true")) {
      return Utils::make_error_code(txErrorCode::EC_SUCCESS, txString("success"));
    } else {
      return Utils::make_error_code(txErrorCode::EC_DATA_IS_ERROR, txString(data));
    }
  }
  return Utils::make_error_code(txErrorCode::EC_DATA_IS_EMPTY, txString("Data is null!"));
}

TX_NAMESPACE_CLOSE(Server)
