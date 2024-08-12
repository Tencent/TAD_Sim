// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_od_c_interface.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include "tx_od_error_code.h"
#include "tx_od_mapdata.h"
#include "tx_od_session_manager.h"

#if __TX_Mark__("plain c interface utility")

tx_od_error_code_t make_error_code(const txErrorCode err_code, const std::string& refErrInfo) {
  tx_od_error_code_t ret;
  ret.err_code = err_code;
  memset(&ret.err_desc[0], '\0', sizeof(ret.err_desc));
  memcpy(&ret.err_desc[0], refErrInfo.c_str(),
         std::min((size_t)(tx_od_error_code::ERR_DESC_SIZE - 1), (refErrInfo.size() + 1)));
  return std::move(ret);
}

tx_od_error_code_t make_error_code(const tx_od_error_info& errInfo) {
  tx_od_error_code_t ret;
  ret.err_code = errInfo.err_code;
  memset(&ret.err_desc[0], '\0', sizeof(ret.err_desc));
  memcpy(&ret.err_desc[0], &errInfo.err_desc[0],
         std::min(tx_od_error_code::ERR_DESC_SIZE - 1, (tx_od_error_info::ERR_DESC_SIZE - 1)));
  return std::move(ret);
}

bool is_param_string_valid(const tx_od_param_string_t& paraString) {
  return (paraString.length > 1) && ((paraString.data));
}

std::string make_string(const tx_od_param_string_t paraString) {
  if (is_param_string_valid(paraString)) {
    return std::move(std::string(paraString.data));
  } else {
    return std::move(std::string(""));
  }
}

std::stringstream make_string_stream(const tx_od_param_string_t paraString) {
  return std::move(std::stringstream(make_string(paraString)));
}

void make_string(const tx_od_param_string_t paraString, std::string& retStr) {
  if (is_param_string_valid(paraString)) {
    retStr = std::move(std::string(paraString.data));
  } else {
    retStr = std::move(std::string(""));
  }
}

tx_od_param_string_t make_param_string(const std::string& strInfo) {
  tx_od_param_string_t ret;
  ret.length = strInfo.size() + 1;
  ret.data = new char[ret.length];
  if ((ret.data)) {
    memset(ret.data, '\0', ret.length * sizeof(std::string::value_type));
    memcpy(ret.data, strInfo.c_str(), strInfo.size());
  } else {
    ret.length = 0;
    ret.data = nullptr;
  }
  return ret;
}

tx_od_param_string_t make_param_string(const std::stringstream& streamInfo) {
  return make_param_string(streamInfo.str());
}
#endif /*__TX_Mark__("plain c interface utility")*/

bool init_log(const char* path) { return Server::txSessionManager::getInstance().initlog(path); }

tx_session_id_t create_session() { return Server::txSessionManager::getInstance().create_session(); }

tx_od_error_code_t release_session(const tx_session_id_t sHdr) {
  tx_od_error_info ret;
  Server::txSessionManager::getInstance().release_session(sHdr, ret);
  return make_error_code(ret);
}

tx_od_error_code_t open_hadmap(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path) {
  LogInfo << "start open hadmap";
  tx_od_error_info ret;
  auto mapPtr = Server::txSessionManager::getInstance().query_session(sHdr, ret);
  if (NonNull_Pointer(mapPtr)) {
    return make_error_code(mapPtr->load(hdmap_path));
  } else {
    if (txErrorCode::EC_SUCCESS == ret.err_code) {
      return make_error_code(txErrorCode::EC_SESSION_NULL, "query_session return nullptr.");
    } else {
      return make_error_code(ret);
    }
  }
}

tx_od_error_code_t create_hadmap(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path,
                                 const tx_od_param_string_t& cmd_json) {
  tx_od_error_info ret;
  auto mapPtr = Server::txSessionManager::getInstance().query_session(sHdr, ret);
  if (NonNull_Pointer(mapPtr)) {
    return make_error_code(mapPtr->create(hdmap_path, make_string_stream(cmd_json)));
  } else {
    if (txErrorCode::EC_SUCCESS == ret.err_code) {
      return make_error_code(txErrorCode::EC_SESSION_NULL, "query_session return nullptr.");
    } else {
      return make_error_code(ret);
    }
  }
}

tx_od_error_code_t save_hadmap(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path, tx_od_lpsz_t outhdmap_path) {
  tx_od_error_info ret;
  auto mapPtr = Server::txSessionManager::getInstance().query_session(sHdr, ret);
  if (NonNull_Pointer(mapPtr)) {
    return make_error_code(mapPtr->save(hdmap_path, outhdmap_path));
  } else {
    if (txErrorCode::EC_SUCCESS == ret.err_code) {
      return make_error_code(txErrorCode::EC_SESSION_NULL, "query_session return nullptr.");
    } else {
      return make_error_code(ret);
    }
  }
}

tx_od_error_code_t modify_hadmap(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path,
                                 const tx_od_param_string_t cmd_json) {
  tx_od_error_info ret;
  auto mapPtr = Server::txSessionManager::getInstance().query_session(sHdr, ret);
  if (NonNull_Pointer(mapPtr)) {
    return make_error_code(mapPtr->modify(hdmap_path, make_string_stream(cmd_json)));
  } else {
    if (txErrorCode::EC_SUCCESS == ret.err_code) {
      return make_error_code(txErrorCode::EC_SESSION_NULL, "query_session return nullptr.");
    } else {
      return make_error_code(ret);
    }
  }
}

tx_od_error_code_t create_hadmap_string(const tx_session_id_t sHdr, tx_od_param_string_t& ret) {
  tx_od_error_info errorinfo;
  auto mapPtr = Server::txSessionManager::getInstance().query_session(sHdr, errorinfo);
  if (NonNull_Pointer(mapPtr)) {
    std::string tmp = "";
    Base::txStringStream json_ret(tmp);
    errorinfo = mapPtr->makestring(json_ret);
    ret = make_param_string(json_ret);
    // std::cout<<std::endl << ret.data;
    return make_error_code(errorinfo);
  } else {
    LogInfo << "Map is not load success!";
    return make_error_code(errorinfo);
  }
}

void release_param_string(tx_od_param_string_t& _resource_allocate_in_api) {
  if (is_param_string_valid(_resource_allocate_in_api)) {
    delete[] _resource_allocate_in_api.data;
    _resource_allocate_in_api.data = nullptr;
    _resource_allocate_in_api.length = 0;
  }
}

tx_od_error_code_t close_hadmap(tx_od_lpsz_t hdmap_path) {
  tx_od_error_info errorinfo;
  auto mapPtrVec = Server::txSessionManager::getInstance().get_allsession();
  for (auto it = mapPtrVec.begin(); it != mapPtrVec.end(); it++) {
    it->second->close(hdmap_path);
  }
  // if (std::string(data) == "true")
  //{
  errorinfo.err_code = EC_SUCCESS;
  return make_error_code(errorinfo);
  //}
  // else {
  //  errorinfo.err_code = EC_CLOSE_MAP_ERROR;
  //  return make_error_code(errorinfo);
  //}
}
