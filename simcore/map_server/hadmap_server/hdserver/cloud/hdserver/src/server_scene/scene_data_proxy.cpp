/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "scene_data_proxy.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include "S3ClientWrap.h"
#include "engine/config.h"
#include "server_map_cache/map_data_proxy.h"

namespace {

bool IsNeeded(const std::string& suffix) { return suffix == ".sim" || suffix == ".xosc"; }

}  // namespace

CSceneDataProxy& CSceneDataProxy::Instance() {
  static CSceneDataProxy instance;

  return instance;
}

int CSceneDataProxy::UploadScenario(const sTagEntityScenario& scenario) {
  std::vector<std::string> s1{".xosc"};
  std::vector<std::string> s2{".sim", ".simrec", "_traffic.xml", "_sensor.xml", "_environment.xml"};
  std::map<std::string, std::vector<std::string>> suffix_map{{".xosc", s1}, {".sim", s2}};

  std::string prefix = boost::filesystem::path(scenario.m_strName).stem().string();
  std::string suffix = boost::filesystem::path(scenario.m_strName).extension().string();

  for (auto& s : suffix_map[suffix]) {
    boost::filesystem::path filePath = CEngineConfig::Instance().ResourceDir();
    filePath.append("scene");
    filePath.append(prefix + s);
    if (!boost::filesystem::exists(filePath)) {
      if (IsNeeded(s)) {
        return -1;
      } else {
        continue;
      }
    }

    int nRet = Upload(scenario.m_strCosBucket, scenario.m_strCosKey + "/" + prefix + s, filePath.string());
    if (nRet != HSEC_OK) {
      return nRet;
    }
  }

  // btrees
  if (suffix == ".sim") {
    boost::filesystem::path filePath = CEngineConfig::Instance().ResourceDir();
    filePath.append("scene");
    auto files = misc::TravelDir(filePath.string(), false, prefix + "_btree_[0-9]{1,}.xml");
    for (auto& file : files) {
      int nRet = Upload(scenario.m_strCosBucket,
                        scenario.m_strCosKey + "/" + boost::filesystem::path(file).filename().string(), file);
      if (nRet != HSEC_OK) {
        return nRet;
      }
    }
  }

  return HSEC_OK;
}

int CSceneDataProxy::DownloadScenario(const sTagEntityScenario& scenario, bool all_dir) {
  std::vector<std::string> s1{".xosc"};
  std::vector<std::string> s2{".sim", "_traffic.xml", "_sensor.xml", "_environment.xml"};
  std::vector<std::string> s3{".json"};
  std::vector<std::string> s4{".frames"};
  std::map<std::string, std::vector<std::string>> suffix_map{
      {".xosc", s1}, {".sim", s2}, {".json", s3}, {".frames", s4}};

  std::string prefix = boost::filesystem::path(scenario.m_strName).stem().string();
  std::string suffix = boost::filesystem::path(scenario.m_strName).extension().string();

  for (auto& s : suffix_map[suffix]) {
    boost::filesystem::path filePath = CEngineConfig::Instance().ResourceDir();
    filePath.append("scene");
    filePath.append(prefix + s);
    int nRet = Download(scenario.m_strCosBucket, scenario.m_strCosKey + "/" + prefix + s, filePath.string());
    if (nRet != HSEC_OK && IsNeeded(s)) {
      return nRet;
    }
  }
  if (all_dir) {
    utils::S3ClientWrap::Instance().DownloadDir(scenario.m_strCosBucket, scenario.m_strCosKey,
                                                CEngineConfig::Instance().SceneDir());
  }

  return HSEC_OK;
}

int CSceneDataProxy::Upload(const std::string& bucket, const std::string& key, const std::string& file) {
  SYSTEM_LOGGER_INFO("upload file, bucket: %s, key: %s, file: %s", bucket.c_str(), key.c_str(), file.c_str());

  return utils::S3ClientWrap::Instance().Upload(bucket, key, file) ? HSEC_OK : HSEC_COS_OP_ERROR;
}

int CSceneDataProxy::Download(const std::string& bucket, const std::string& key, const std::string& file,
                              bool download_always, bool need_lock) {
  SYSTEM_LOGGER_INFO("download file, bucket: %s, key: %s, file: %s, download_always %d", bucket.c_str(), key.c_str(),
                     file.c_str(), download_always);
  if (need_lock) {
    std::lock_guard<std::mutex> _(download_mtx_);
    return utils::S3ClientWrap::Instance().Download(bucket, key, const_cast<std::string&>(file),
                                                    utils::S3ClientWrap::kFile, download_always)
               ? HSEC_OK
               : HSEC_COS_OP_ERROR;
  }
  return utils::S3ClientWrap::Instance().Download(bucket, key, const_cast<std::string&>(file),
                                                  utils::S3ClientWrap::kFile, download_always)
             ? HSEC_OK
             : HSEC_COS_OP_ERROR;
}

int CSceneDataProxy::Delete(const std::string& bucket, const std::string& key, const std::string& file) {
  SYSTEM_LOGGER_INFO("delete file, bucket: %s, key: %s, file: %s", bucket.c_str(), key.c_str(), file.c_str());
  Aws::S3::Model::DeleteObjectRequest req;
  req.WithBucket(bucket).WithKey(key);

  Aws::S3::Model::DeleteObjectOutcome rsp;
  rsp = CMapDataProxy::Instance().S3Client(bucket)->DeleteObject(req);

  if (!file.empty() && boost::filesystem::exists(file)) {
    boost::filesystem::remove(file);
  }
  return 0;
}
