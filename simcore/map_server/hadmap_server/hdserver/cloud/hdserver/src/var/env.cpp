/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "env.h"

#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <memory>

#include "common/log/system_logger.h"
#include "json/json.h"
#include "var/flags.h"

void CEnvVar::ParseFromEnv() {
  char* p = getenv("TXSIM_SERVICE_URL");
  if (p) {
    m_strURL = p;
  } else {
    SYSTEM_LOGGER_INFO("using gflag default sim url: %s", FLAGS_sim_url.c_str());
    m_strURL = FLAGS_sim_url;
  }

  p = getenv("TXSIM_SERVICE_ALL_MAP_URL");
  if (p) {
    m_strAllMapURL = p;
  } else {
    SYSTEM_LOGGER_INFO("using gflag default sim host: %s", FLAGS_sim_all_map_url.c_str());
    m_strAllMapURL = FLAGS_sim_all_map_url;
  }

  p = getenv("TXSIM_SERVICE_HOST");
  if (p) {
    m_strHost = p;
  } else {
    SYSTEM_LOGGER_INFO("using gflag default sim host: %s", FLAGS_sim_host.c_str());
    m_strHost = FLAGS_sim_host;
  }
  p = getenv("TXSIM_SERVICE_PORT");
  if (p) {
    m_strPort = p;
  } else {
    SYSTEM_LOGGER_INFO("using gflag default sim port: %s", FLAGS_sim_port.c_str());
    m_strPort = FLAGS_sim_port;
  }
  p = getenv("TXSIM_SERVICE_SALT");
  if (p) {
    m_strSalt = p;
  } else {
    SYSTEM_LOGGER_INFO("using gflag default sim salt: %s", FLAGS_sim_salt.c_str());
    m_strSalt = FLAGS_sim_salt;
  }
  p = getenv("TXSIM_PRIVATE_BUCKET");
  if (p) {
    m_strPrivateDefaultBucket = p;
  }
  std::string strCosConfig;
  p = getenv("TXSIM_COS_CONFIG");
  if (p) {
    strCosConfig = p;
  } else {
    SYSTEM_LOGGER_INFO("using gflag default cos config: %s", FLAGS_cos_config.c_str());
  }

  ParseFromEnvInner(FLAGS_cos_config);
  ParseFromEnvInner(strCosConfig);
}

void CEnvVar::ParseFromEnvInner(const std::string& config) {
  if (config.empty()) {
    return;
  }
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  JSONCPP_STRING errs;
  if (!reader->parse(config.c_str(), config.c_str() + config.length(), &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse from env variable error: %s, %s", errs.c_str(), config.c_str());
    return;
  }
  if (root.isNull()) {
    SYSTEM_LOGGER_ERROR("cos config json is null");
    return;
  }

#define SET_STRING_MEMBER(m, k) \
  if (root.isMember(#k) && root[#k].isString()) m = root[#k].asString();

  SET_STRING_MEMBER(m_strSecretID, SecretId);
  SET_STRING_MEMBER(m_strSecretKey, SecretKey);
  SET_STRING_MEMBER(m_strRegion, Region);
  SET_STRING_MEMBER(m_strEndPoint, EndPoint);
  SET_STRING_MEMBER(m_strDefaultBucket, DefaultBucket);
  SET_STRING_MEMBER(m_strSignExpiredTime, SignExpiredTime);
  SET_STRING_MEMBER(m_strConnectTimeoutInms, ConnectTimeoutInms);
  SET_STRING_MEMBER(m_strReceiveTimeoutInms, ReceiveTimeoutInms);
  SET_STRING_MEMBER(m_strAsynThreadPoolSize, AsynThreadPoolSize);
  SET_STRING_MEMBER(m_strUploadPartSize, UploadPartSize);
  SET_STRING_MEMBER(m_strUploadCopyPartSize, UploadCopyPartSize);
  SET_STRING_MEMBER(m_strUploadThreadPoolSize, UploadThreadPoolSize);
  SET_STRING_MEMBER(m_strLogoutType, LogoutType);
  SET_STRING_MEMBER(m_strLogLevel, LogLevel);
  SET_STRING_MEMBER(m_strDownloadThreadPoolSize, DownloadThreadPoolSize);
  SET_STRING_MEMBER(m_strDownloadSliceSize, DownloadSliceSize);
  SET_STRING_MEMBER(m_strScheme, Scheme);
  if (root.isMember("VirtualAddress") && root["VirtualAddress"].isBool()) {
    m_VirtualAddressing = root["VirtualAddress"].asBool();
  }
  if (root.isMember("VerifySSL") && root["VerifySSL"].isBool()) {
    m_VerifySSL = root["VerifySSL"].asBool();
  }
}
