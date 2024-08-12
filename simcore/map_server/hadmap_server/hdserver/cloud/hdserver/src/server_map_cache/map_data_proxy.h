/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WINDOWS
#  include <Windows.h>
#  undef GetObject
#  undef GetMessage
#endif
#include <aws/s3/S3Client.h>

#include "server_scene/scene_data_proxy.h"

#include "common/utils/misc.h"
#include "common/utils/oid_generator.h"

struct sTagServiceHadmapInfo {
  sTagServiceHadmapInfo() { Reset(); }

  void Reset();

  void ToTimeStamp();

  void ToGMTTime();

  std::string ToString() {
    return "Name: " + m_strHadmapName + ";" + "cos bucket: " + m_strCosBucket + ";" + "cos key: " + m_strCosKey + ";";
  }

  std::string m_strHadmapName;
  int64_t m_llTimestamp;
  std::string m_strLastModifiedTime;
  std::string m_strCosBucket;
  std::string m_strCosKey;
  std::string m_strJsonKey;
  std::string m_version;
  std::string m_strEtag;
};

class CMapDataProxy {
  friend class CSceneDataProxy;

 public:
  static CMapDataProxy& Instance();

  void Init();

  int Mode() { return m_nMode; }

  void SetMode(int nMode) { m_nMode = nMode; }

  int GetCosEnvironments();

  int FetchHadmap(const char* strMapName, sTagServiceHadmapInfo& info);

  int GetHadmapInfoFromService(const char* strMapName, sTagServiceHadmapInfo& hadmapInfo);

  int GetAllHadmapInfoFromService(std::vector<sTagServiceHadmapInfo>& hadmapInfo);

  int GetHadmapInfoFromCos(const char* strMapName, sTagServiceHadmapInfo& hadmapInfo);

  bool IsExistsInCos(const std::string& strBucket, const std::string& strCosKey);

  // there is no such api in simservice, implement it later.
  int GethadmapInfoList();

  int getObjectToStr(const std::string& cosBucket, const std::string& cosKey, std::string& data);

  std::string GetCurTimeInMilliSeconds();

  int uploadStr(const std::string& cosBucket, std::string cosKey, const std::string& data);

  int uploadFile(const std::string& cosBucket, const std::string& cosKey, const std::string& fpath);

  int DownloadMapFile(std::string strCosBucket, std::string strCosKey, std::string strMapName);
  int DownloadMapFile(sTagServiceHadmapInfo& info, const std::string& fpath);

  static std::string UrlEncode(const std::string& inputString);

  static std::string UrlDecode(const std::string& inputString);

  Aws::S3::S3Client* S3Client(const std::string& bucket = "") {
    auto iter = s3client_others_.find(bucket);
    return (iter != s3client_others_.end() && iter->second != nullptr) ? iter->second : m_Client;
  }

 protected:
  void json2HadmapInfo(const Json::Value& da, sTagServiceHadmapInfo& info);

  int UpdateHadmapInfo(const char* strMapName);

  int cosCheck(const std::string& cosBucket, const std::string& cosKey);

  boost::filesystem::path composeFilePath(std::string mapName);

  // int ParseResMapCosAddr(const char* strJson, std::string& strCosBucket, std::string& strCosKey);
  int ParseResMapCosInfo(const char* strJson, sTagServiceHadmapInfo&);

  int ParserAllMapCosInfo(const char* strJson, std::vector<sTagServiceHadmapInfo>& mapVec);

  int HttpGet(std::string url, std::string& res);

  int doHttpGet(std::string strHost, std::string strPort, std::string strURL, std::string& strBody);
  // std::string GenMD5(std::string strMapName, std::string strTimeStamp, std::string strSalt);

  CMapDataProxy();

 protected:
  int m_nMode;
  std::string m_strURL;

  tad::sim::ObjectIdGenerator id_generator_;

  Aws::S3::S3Client* m_Client;
  std::map<std::string, Aws::S3::S3Client*> s3client_others_;
};
