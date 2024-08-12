/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: longerding@tencent.com (Emperor Penguin)
//

#include "CMapEdit.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include "../../src/server_map_cache/map_data_cache.h"
#include "../engine/util/scene_util.h"
#include "../src/engine/config.h"
#include "common/log/system_logger.h"
typedef bool (*init_logpath_fun_ptr)(const char* path);
typedef tx_session_id_t (*create_session_fun_ptr)();
typedef tx_od_error_code_t (*release_session_fun_ptr)(const tx_session_id_t sHdr);
typedef tx_od_error_code_t (*open_hadmap_fun_ptr)(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path);
typedef tx_od_error_code_t (*create_hadmap_fun_ptr)(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path,
                                                    const tx_od_param_string_t& cmd_json);
typedef tx_od_error_code_t (*create_hadmap_string_ptr)(const tx_session_id_t sHdr, _tx_od_param_string& str);
typedef tx_od_error_code_t (*save_hadmap_fun_ptr)(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path,
                                                  tx_od_lpsz_t hdmapout_path);
typedef tx_od_error_code_t (*modify_hadmap_fun_ptr)(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path,
                                                    const tx_od_param_string_t cmd_json);
typedef tx_od_error_code_t (*close_hadmap_fun_ptr)(tx_od_lpsz_t hdmap_path);
typedef void (*release_param_string_fun_ptr)(tx_od_param_string_t& _resource_allocate_in_api);

CMapEdit::CMapEdit() { hDLL = NULL; }

CMapEdit& CMapEdit::Instance() {
  static CMapEdit instance;

  return instance;
}

bool CMapEdit::Init(const char* dirPath) {
  boost::filesystem::path appInstallPath = CEngineConfig::Instance().AppInstallDir();
#ifdef _WIN32
  boost::filesystem::path xodrPluginPath =
      appInstallPath / boost::filesystem::path("service/txSimService/OpenDrivePlugin");
#else
  boost::filesystem::path xodrPluginPath =
      appInstallPath / boost::filesystem::path("service/simdeps/libOpenDrivePlugin");
#endif
  SYSTEM_LOGGER_INFO("Load plugin from %s", xodrPluginPath.string().c_str());
  // load shared
  hDLL = LoadSharedLibrary(xodrPluginPath.string().c_str());
  if (NULL == hDLL) {
    SYSTEM_LOGGER_ERROR("load dll/so failure");
    return false;
  }
  SYSTEM_LOGGER_INFO("Load plugin from %s", xodrPluginPath.string().c_str());
  // init log path

  return false;
}

bool CMapEdit::InitLog(const char* logpath) {
  init_logpath_fun_ptr pInitLog = (init_logpath_fun_ptr)GetFunction(hDLL, "init_log");
  if (NULL == pInitLog) {
    SYSTEM_LOGGER_ERROR("init log failure");
    return false;
  }
  SYSTEM_LOGGER_ERROR("init log sucess ,%s", logpath);
  return pInitLog(logpath);
}

const wchar_t* CMapEdit::CreateSession() {
  create_session_fun_ptr pCreateSession = (create_session_fun_ptr)GetFunction(hDLL, "create_session");
  if (NULL == pCreateSession) {
    SYSTEM_LOGGER_ERROR("create session failure");
    return L"{\"message\":\"error\",\"sessionId\": -1 }";
  }
  tx_session_id_t sessionId = pCreateSession();
  if (-1 == sessionId) {
    SYSTEM_LOGGER_ERROR("create session failure");
    return L"{\"message\":\"error\",\"sessionId\": -1 }";
  }
  Json::Value ret;
  ret["message"] = "ok";
  ret["sessionId"] = sessionId;
  wstrRet.clear();
  wstrRet = CEngineConfig::Instance().MBStr2WStr(formatJson(ret).c_str());
  return wstrRet.c_str();
}

const wchar_t* CMapEdit::ReleaseSession(const wchar_t* wstrMapInfo) {
  std::string strMapInfo = CEngineConfig::Instance().WStr2MBStr(wstrMapInfo);
  Json::Value json_value = stringToJson(strMapInfo);
  if (json_value.empty()) {
    SYSTEM_LOGGER_ERROR("Json is empty");
    return L"{\"message\":\"error\",\"info\": \"Json is empty\" }";
  }
  tx_session_id_t sessionID = json_value["sessionid"].asInt();

  release_session_fun_ptr pReleaseSession = (release_session_fun_ptr)GetFunction(hDLL, "release_session");
  tx_od_error_code_t errorCode_Release = pReleaseSession(sessionID);
  if (txErrorCode::EC_SUCCESS == errorCode_Release.err_code) {
    SYSTEM_LOGGER_INFO("success to release session!");
    return L"{\"message\":\"ok\",\"info\": \"open map success\" }";
  } else {
    SYSTEM_LOGGER_ERROR("fail to release session!");
    std::string strInfo = errorCode_Release.err_desc;
    Json::Value ret;
    ret["message"] = "error";
    ret["sessionId"] = strInfo;
    wstrRet.clear();
    wstrRet = CEngineConfig::Instance().MBStr2WStr(formatJson(ret).c_str());
    return wstrRet.c_str();
  }
}

const wchar_t* CMapEdit::OpenHadmap(const wchar_t* wstrMapInfo) {
  // string to json
  std::string strMapInfo = CEngineConfig::Instance().WStr2MBStr(wstrMapInfo);
  Json::Value json_value = stringToJson(strMapInfo);
  if (json_value.empty()) {
    SYSTEM_LOGGER_ERROR("Json is empty");
    return L"{\"message\":\"error\",\"info\": \"Json is empty\" }";
  }
  tx_session_id_t sessionID = json_value["sessionid"].asInt();

  // get hadmap path
  std::string strHadmapName = json_value["map"].asString().c_str();
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path p = strResourceDir;
  boost::filesystem::path sceneDirPath = p;
  sceneDirPath.append("hadmap");
  boost::filesystem::path xmlFilePath = CSceneUtil::HadmapFullPath(strHadmapName.c_str());
  // load hadmap
  std::string map_path = CEngineConfig::Instance().getHadmapDataDir();
  open_hadmap_fun_ptr pOpenHadmap = (open_hadmap_fun_ptr)GetFunction(hDLL, "open_hadmap");
  tx_od_error_code_t errorCode_Open = pOpenHadmap(sessionID, xmlFilePath.string().c_str());
  if (txErrorCode::EC_SUCCESS == errorCode_Open.err_code) {
    SYSTEM_LOGGER_INFO("success to open hadmap %s!", xmlFilePath.c_str());
    return L"{\"message\":\"ok\",\"info\": \"open map success\" }";
  } else {
    SYSTEM_LOGGER_ERROR("fail to open hadmap %s!", xmlFilePath.c_str());
    std::string strInfo = errorCode_Open.err_desc;
    Json::Value ret;
    ret["message"] = "error";
    ret["sessionId"] = strInfo;
    wstrRet.clear();
    wstrRet = CEngineConfig::Instance().MBStr2WStr(formatJson(ret).c_str());
    return wstrRet.c_str();
  }
}

const char* CMapEdit::CreateHadmapString(const wchar_t* wstrSessionID) {
  std::string strMapInfo = CEngineConfig::Instance().WStr2MBStr(wstrSessionID);
  Json::Value json_value = stringToJson(strMapInfo);
  if (json_value.empty()) {
    SYSTEM_LOGGER_ERROR("Json is empty");
    return "{\"message\":\"error\",\"info\": \"Json is empty\" }";
  }
  create_hadmap_string_ptr pCreateString = (create_hadmap_string_ptr)GetFunction(hDLL, "create_hadmap_string");
  _tx_od_param_string _str;
  tx_od_error_code_t errorCode_CreateString = pCreateString(
      json_value["sessionid"].asInt(), _str);  // SYSTEM_LOGGER_ERROR("start createHadmapString is empty");
  if (txErrorCode::EC_SUCCESS == errorCode_CreateString.err_code) {
    SYSTEM_LOGGER_INFO("success to get hadmap!");
    return _str.data;
  } else {
    SYSTEM_LOGGER_ERROR("fail to get hadmap!");
    std::string strInfo = errorCode_CreateString.err_desc;
    Json::Value ret;
    ret["message"] = "error";
    ret["info"] = strInfo;
    const char* data = formatJson(ret).c_str();
    return data;
    // std::wstring wstrRet =  CEngineConfig::Instance().MBStr2WStr();
    // return wstrRet.c_str();
  }
}

const wchar_t* CMapEdit::CreateNewHadmap(const wchar_t* wstrMapInfo) {
  std::string strMapInfo = CEngineConfig::Instance().WStr2MBStr(wstrMapInfo);
  Json::Value json_value = stringToJson(strMapInfo);
  if (json_value.empty()) {
    SYSTEM_LOGGER_ERROR("Json is empty");
    return L"{\"message\":\"error\",\"info\": \"Json is empty\" }";
  }
  create_hadmap_fun_ptr pCreateHadmap = (create_hadmap_fun_ptr)GetFunction(hDLL, "create_hadmap");
  tx_od_param_string_t tx_str = make_param_string(strMapInfo);
  // get save path
  std::string hadmapDir = CEngineConfig::Instance().getHadmapDataDir();
  boost::filesystem::path p = hadmapDir;
  p.append(json_value["xodr"].asString().c_str());
  // xodrpath
  boost::filesystem::path xodrpath = p;
  xodrpath /= json_value["map"].asString();

  // start create new map
  tx_od_error_code_t errorCode_createnewHadmap =
      pCreateHadmap(json_value["sessionid"].asInt(), xodrpath.string().c_str(), tx_str);
  if (txErrorCode::EC_SUCCESS == errorCode_createnewHadmap.err_code) {
    this->AddToConfigFile(xodrpath);
    SYSTEM_LOGGER_INFO("finished to create new hadmap!");
    return L"{\"message\":\"ok\",\"info\": \"success to create hadmap\" }";
  } else {
    SYSTEM_LOGGER_ERROR("fail to get hadmap!");
    std::string strInfo = errorCode_createnewHadmap.err_desc;
    Json::Value ret;
    ret["message"] = "error";
    ret["info"] = strInfo;
    wstrRet.clear();
    wstrRet = CEngineConfig::Instance().MBStr2WStr(formatJson(ret).c_str());
    return wstrRet.c_str();
  }
}

const wchar_t* CMapEdit::SaveHadmap(const wchar_t* wstrMapInfo) {
  std::string strMapInfo = CEngineConfig::Instance().WStr2MBStr(wstrMapInfo);
  Json::Value json_value = stringToJson(strMapInfo);
  if (json_value.empty()) {
    SYSTEM_LOGGER_ERROR("Json is empty");
    return L"{\"message\":\"error\",\"info\": \"Json is empty\" }";
  }
  tx_session_id_t sessionID = json_value["sessionid"].asInt();
  // get src hadmap path
  std::string strHadmapName = json_value["srcmap"].asString().c_str();
  boost::filesystem::path mapFilePath = CSceneUtil::HadmapFullPath(strHadmapName.c_str());

  // get dst hadmap path
  std::string hadmapDir = CEngineConfig::Instance().getHadmapDataDir();
  boost::filesystem::path p = hadmapDir;
  p.append(json_value["xodr"].asString().c_str());
  // xodrpath
  boost::filesystem::path dstpath = p;
  dstpath /= json_value["dstmap"].asString();

  // 获取保存地图函数
  save_hadmap_fun_ptr pSaveHadmap = (save_hadmap_fun_ptr)GetFunction(hDLL, "save_hadmap");
  tx_od_error_code_t errorCode_Save = pSaveHadmap(sessionID, mapFilePath.string().c_str(), dstpath.string().c_str());
  if (txErrorCode::EC_SUCCESS == errorCode_Save.err_code) {
    if (mapFilePath.string() != dstpath.string()) {
      this->AddToConfigFile(dstpath);
      SYSTEM_LOGGER_INFO("AddToConfigFile!");
    } else {
      std::string strName = dstpath.filename().string();
      std::wstring wstrName = CEngineConfig::Instance().MBStr2WStr(strName.c_str());
      SYSTEM_LOGGER_INFO("Remove hadmap cache %s!", strName.c_str());
      CMapDataCache::Instance().RemoveHadmapCacheAll(wstrName.c_str());
    }
    return L"{\"message\":\"ok\",\"info\": \"success to save hadmap\" }";
  } else {
    SYSTEM_LOGGER_ERROR("fail to save hadmap!");
    std::string strInfo = errorCode_Save.err_desc;
    Json::Value ret;
    ret["message"] = "error";
    ret["info"] = strInfo;
    wstrRet.clear();
    wstrRet = CEngineConfig::Instance().MBStr2WStr(formatJson(ret).c_str());
    return wstrRet.c_str();
  }
}

const wchar_t* CMapEdit::ModifyHadmap(const wchar_t* wstrMapInfo) {
  std::string strMapInfo = CEngineConfig::Instance().WStr2MBStr(wstrMapInfo);
  Json::Value json_value = stringToJson(strMapInfo);
  if (json_value.empty()) {
    SYSTEM_LOGGER_ERROR("Json is empty");
    return L"{\"message\":\"error\",\"info\": \"Json is empty\" }";
  }
  tx_session_id_t sessionID = json_value["sessionid"].asInt();
  // get src hadmap path
  std::string strHadmapName = json_value["map"].asString().c_str();
  boost::filesystem::path mapFilePath = CSceneUtil::HadmapFullPath(strHadmapName.c_str());
  // start modify
  modify_hadmap_fun_ptr pModifyHadmap = (modify_hadmap_fun_ptr)GetFunction(hDLL, "modify_hadmap");
  if (!pModifyHadmap) {
    SYSTEM_LOGGER_ERROR("Opendrive Dll error");
    return L"{\"message\":\"error\",\"info\": \"Opendrive Dll error\" }";
  }
  tx_od_param_string_t allocateString = make_param_string(strMapInfo);
  //
  tx_od_error_code_t errorCode_Modify = pModifyHadmap(sessionID, mapFilePath.string().c_str(), allocateString);  //
  if (txErrorCode::EC_SUCCESS == errorCode_Modify.err_code) {
    SYSTEM_LOGGER_INFO("Success to modify map!");
    return L"{\"message\":\"ok\",\"info\": \"success to create hadmap\" }";
  } else {
    SYSTEM_LOGGER_ERROR("fail to get hadmap!");
    std::string strInfo = errorCode_Modify.err_desc;
    Json::Value ret;
    ret["message"] = "error";
    ret["info"] = strInfo;
    wstrRet.clear();
    wstrRet = CEngineConfig::Instance().MBStr2WStr(formatJson(ret).c_str());
    return wstrRet.c_str();
  }
}

int CMapEdit::CloseMap(const wchar_t* wstrMapInfo) {
  std::string strMapInfo = CEngineConfig::Instance().WStr2MBStr(wstrMapInfo);
  std::string hadmapDir = CEngineConfig::Instance().getHadmapDataDir();
  boost::filesystem::path p = hadmapDir;
  boost::filesystem::path dstpath = p;
  dstpath /= strMapInfo;
  SystemLogger::Debug("dstpath:%s", dstpath.c_str());
  close_hadmap_fun_ptr pCloseHadmap = (close_hadmap_fun_ptr)GetFunction(hDLL, "close_hadmap");
  if (!pCloseHadmap) {
    SYSTEM_LOGGER_ERROR("Opendrive Dll error");
    return -1;
  }
  tx_od_error_code_t errorCode_close = pCloseHadmap(dstpath.string().c_str());
  if (txErrorCode::EC_SUCCESS == errorCode_close.err_code) {
    return 0;
  } else {
    return -1;
  }
}

void* CMapEdit::LoadSharedLibrary(const char* pcDllname, int iMode) {
  std::string sDllName = pcDllname;
#if defined(_MSC_VER)  // Microsoft compiler
  sDllName += ".dll";
  return reinterpret_cast<void*>(LoadLibraryA(pcDllname));
#elif defined(__GNUC__)  // GNU compiler
  sDllName += ".so";
  return dlopen(sDllName.c_str(), iMode);
#endif
}

void* CMapEdit::GetFunction(void* Lib, char* Fnname) {
#if defined(_MSC_VER)  // Microsoft compiler
  return reinterpret_cast<void*>(GetProcAddress((HINSTANCE)Lib, Fnname));
#elif defined(__GNUC__)  // GNU compiler
  return dlsym(Lib, Fnname);
#endif
}

bool CMapEdit::FreeSharedLibrary(void* hDLL) {
  {
#if defined(_MSC_VER)  // Microsoft compiler
    return FreeLibrary((HINSTANCE)hDLL);
#elif defined(__GNUC__)  // GNU compiler
    return dlclose(hDLL);
#endif
  }
}

int CMapEdit::AddToConfigFile(boost::filesystem::path& p) {
  // get hadmap parameter
  CMapFile mapfile;
  int nRet = mapfile.ParseMapV2(p.string().c_str(), false);
  if (nRet != 0) {
    SYSTEM_LOGGER_ERROR("open hadmap %s error", p.string().c_str());
  } else {
    std::string strName = p.filename().string();
    int nIndex = CEngineConfig::Instance().HadmapIndex(strName.c_str());
    // chanage attr 1
    tagHadmapAttr attr;
    attr.m_name = strName;
    attr.m_unRealLevelIndex = nIndex;
    attr.m_strUnrealLevelIndex = std::to_string(nIndex);
    attr.m_nPreset = 0;
    attr.m_strPreset = "0";
    attr.m_dRefLon = mapfile.m_center[0];
    attr.m_dRefLat = mapfile.m_center[1];
    attr.m_dRefAlt = mapfile.m_center[2];
    std::string str;
    std::stringstream ss1, ss2, ss3;
    ss1 << std::fixed << std::setprecision(8) << attr.m_dRefAlt;
    attr.m_strRefAlt = ss1.str();
    ss2 << std::fixed << std::setprecision(8) << attr.m_dRefLon;
    attr.m_strRefLon = ss2.str();
    ss3 << std::fixed << std::setprecision(8) << attr.m_dRefLat;
    attr.m_strRefLat = ss3.str();
    CEngineConfig::Instance().UpdateHadmapAttr(strName, attr);
    std::wstring wstrName = CEngineConfig::Instance().MBStr2WStr(strName.c_str());
    CMapDataCache::Instance().RemoveHadmapCacheAll(wstrName.c_str());
  }
  mapfile.Disconect();
  return 0;
}

tx_od_param_string_t CMapEdit::make_param_string(const std::string& strInfo) {
  {
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
}

std::string CMapEdit::formatJson(Json::Value& json_value) {
  Json::StreamWriterBuilder swbuilder;
  std::string strResult = Json::writeString(swbuilder, json_value);
  Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "";
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream ss;
  writer->write(json_value, &ss);
  std::string strUnstyledResult = ss.str();
  return strUnstyledResult;
}

Json::Value CMapEdit::stringToJson(std::string strJson) {
  Json::Value root;
  if (strJson.empty()) {
    SYSTEM_LOGGER_ERROR("Parse Json error!");
    return root;
  }
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  Json::CharReader* reader = builder.newCharReader();
  int nLen = strlen(strJson.c_str());
  if (!reader->parse(strJson.c_str(), strJson.c_str() + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parser map json error!");
    return root;
  }
  return root;
}
