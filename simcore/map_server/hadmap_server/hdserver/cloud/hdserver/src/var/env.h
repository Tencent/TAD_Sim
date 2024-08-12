/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>

class CEnvVar {
 public:
  void ParseFromEnv();
  void ParseFromEnvInner(const std::string& config);

  std::string m_strURL;
  std::string m_strAllMapURL;
  std::string m_strHost;
  std::string m_strPort;
  std::string m_strSalt;
  std::string m_strSecretID;
  std::string m_strSecretKey;
  std::string m_strRegion;
  std::string m_strEndPoint;
  std::string m_strDefaultBucket;
  std::string m_strScheme = "https";
  std::string m_strSignExpiredTime;
  std::string m_strConnectTimeoutInms;
  std::string m_strReceiveTimeoutInms;
  std::string m_strAsynThreadPoolSize;
  std::string m_strUploadPartSize;
  std::string m_strUploadCopyPartSize;
  std::string m_strUploadThreadPoolSize;
  std::string m_strLogoutType;
  std::string m_strLogLevel;
  std::string m_strDownloadThreadPoolSize;
  std::string m_strDownloadSliceSize;
  std::string m_strPrivateDefaultBucket;
  bool m_VirtualAddressing = true;
  bool m_VerifySSL = true;
};
