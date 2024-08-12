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
// 地图编辑器动态库调用接口
//
#pragma once
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <string.h>
#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#if defined(__tx_windows__)  // Microsoft compiler
#  include <windows.h>
#elif defined(__tx_linux__)  // GNU compiler
#  include <dlfcn.h>
#else
#  include "../../map_parser/src/engine/config.h"
#  include "common/xml_parser/entity/mapfile.h"

// #error define your copiler
#endif

class CMapEdit {
 public:
  CMapEdit();
  // instance
  static CMapEdit& Instance();
  // init
  bool Init(const char* logDir);
  // init log
  bool InitLog(const char* logDir);
  // create session
  const wchar_t* CreateSession();
  // release session
  const wchar_t* ReleaseSession(const wchar_t* wstrMapInfo);
  // open hadmap
  const wchar_t* OpenHadmap(const wchar_t* wstrMapInfo);
  // create hadmap string
  const char* CreateHadmapString(const wchar_t* wstrSessionID);
  // create new hadmap
  const wchar_t* CreateNewHadmap(const wchar_t* wstrMap);
  // save hadmap
  const wchar_t* SaveHadmap(const wchar_t* wstrMapInfo);
  // modify hadmap
  const wchar_t* ModifyHadmap(const wchar_t* wstrMapInfo);
  // close hadmap
  int CloseMap(const wchar_t* wstrMapInfo);

 private:
  // load library
  void* LoadSharedLibrary(const char* pcDllname, int iMode = 2);
  // get function
  void* GetFunction(void* Lib, char* Fnname);
  // release library
  bool FreeSharedLibrary(void* hDLL);
  // add hadmap to config
  int AddToConfigFile(boost::filesystem::path& p);
  // make string
  tx_od_param_string_t make_param_string(const std::string& strInfo);
  // json to string
  std::string formatJson(Json::Value& json_value);
  // string to json
  Json::Value stringToJson(std::string);

 private:
  std::wstring wstrRet;
  void* hDLL;
};
