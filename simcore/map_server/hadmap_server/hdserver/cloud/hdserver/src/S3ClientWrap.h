// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <map>

#ifdef _WINDOWS
#  include <Windows.h>
#  undef GetObject
#  undef GetMessage
#endif
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

#include "MD5.h"
#include "common/utils/misc.h"
#include "common/utils/singleton.h"
#include "var/env.h"
#include "var/flags.h"

namespace utils {

class S3ClientWrap : public utils::Singleton<S3ClientWrap> {
 public:
  using EnvVarPtr = std::shared_ptr<CEnvVar>;

  using S3Ptr = std::shared_ptr<Aws::S3::S3Client>;

  enum TransType {
    kFile,
    kString,
  };

  virtual ~S3ClientWrap() {}
  void Initialize() {
    Aws::SDKOptions options;
    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Off;
    Aws::InitAPI(options);

    CreateClient(CreateCosEnv(getenv("TXSIM_COS_CONFIG")), s3client_);

    Json::Value root = StringToJson(getenv("TXSIM_COS_CONFIG_OTHERS"));
    for (auto& one : root) {
      // ["TXSIM_COS_CONFIG1", "TXSIM_COS_CONFIG2"]
      EnvVarPtr cos_env = CreateCosEnv(JsonToString(one));
      if (cos_env->m_strDefaultBucket.empty()) {
        continue;
      }
      CreateClient(cos_env, s3client_others_[cos_env->m_strDefaultBucket]);
    }
  }

  bool Upload(const std::string& bucket, const std::string& key, const std::string& input,
              TransType type = TransType::kFile) {
    SYSTEM_LOGGER_INFO("upload file|str, bucket: %s, key: %s, type: %d, file: %s, size: %lu", bucket.c_str(),
                       key.c_str(), type, input.c_str(), input.size());

    // req
    Aws::S3::Model::PutObjectRequest req;
    req.WithBucket(bucket).WithKey(key);
    if (type == TransType::kFile) {
      auto body = Aws::MakeShared<Aws::FStream>("PutObjectInputStream", input, std::ios::in | std::ios::binary);
      req.SetBody(body);
    } else if (type == TransType::kString) {
      auto body = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream");
      (*body) << input;
      req.SetBody(body);
    }
    // rsp
    Aws::S3::Model::PutObjectOutcome rsp;
    for (int i = 0; i < kRetryTimes && !rsp.IsSuccess(); i++) {
      rsp = GetS3Ptr(bucket)->PutObject(req);
    }

    if (!rsp.IsSuccess()) {
      SYSTEM_LOGGER_ERROR("upload file|str, error: %s", rsp.GetError().GetMessage().c_str());
      return false;
    }

    return true;
  }

  bool UploadDir(const std::string& bucket, const std::string& prefix, const std::string& path) {
    std::vector<std::string> files = misc::TravelDir(path);
    for (auto& file : files) {
      Upload(bucket, prefix + "/" + file.substr(path.size()), file);
    }
    return true;
  }

  bool Download(const std::string& bucket, const std::string& key, std::string& output,
                TransType type = TransType::kFile, bool download_always = true) {
    SYSTEM_LOGGER_INFO("download file|str, bucket: %s, key: %s, type: %d, file: %s, download_always %d", bucket.c_str(),
                       key.c_str(), type, output.c_str(), download_always);
    if (!download_always && boost::filesystem::exists(output) &&
        ETag(bucket, key).find(GenMD5Sum(output)) != std::string::npos) {
      return true;
    }

    // req
    Aws::S3::Model::GetObjectRequest req;
    req.WithBucket(bucket).WithKey(key);
    // rsp
    Aws::S3::Model::GetObjectOutcome rsp;
    for (int i = 0; i < kRetryTimes && !rsp.IsSuccess(); i++) {
      rsp = GetS3Ptr(bucket)->GetObject(req);
    }
    if (!rsp.IsSuccess()) {
      SYSTEM_LOGGER_ERROR("download file|str, error: %s", rsp.GetError().GetMessage().c_str());
      return false;
    }

    if (type == TransType::kString) {
      std::ostringstream oss;
      oss << rsp.GetResult().GetBody().rdbuf();
      output = oss.str();
      return true;
    }

    boost::filesystem::create_directories(boost::filesystem::path(output).parent_path());
    Aws::OFStream ofs;
    ofs.open(output, std::ios::out | std::ios::binary);
    ofs << rsp.GetResult().GetBody().rdbuf();
    ofs.close();
    return true;
  }

  bool DownloadDir(const std::string& bucket, const std::string& raw_prefix, const std::string& path,
                   bool download_always = true) {
    // note: prefix can`t start with a slash('/').
    // note: prefix must be end with a slash('/').
    std::string prefix = raw_prefix;
    if (prefix.find_first_of(kStringPathDelimiter) == 0) {
      prefix.erase(0, 1);
    }
    if (prefix.find_last_of(kStringPathDelimiter) != prefix.size() - 1) {
      prefix.append(kStringPathDelimiter);
    }
    if (bucket.empty() || prefix.empty() || path.empty()) {
      SYSTEM_LOGGER_ERROR("download dir, invalid param: %s, %s, %s", bucket.c_str(), prefix.c_str(), path.c_str());
      return false;
    }
    SYSTEM_LOGGER_INFO("download dir, param: %s, %s, %s", bucket.c_str(), prefix.c_str(), path.c_str());
    Aws::S3::Model::ListObjectsRequest req;
    req.WithBucket(bucket).WithPrefix(prefix).WithDelimiter(kStringPathDelimiter);
    Aws::S3::Model::ListObjectsOutcome rsp = GetS3Ptr(bucket)->ListObjects(req);

    for (auto& one : rsp.GetResult().GetContents()) {
      if (one.GetKey().size() <= prefix.size()) {
        continue;
      }
      std::string key = one.GetKey();
      std::string dst = path + kStringPathDelimiter + key.substr(prefix.size());
      Download(bucket, key, dst);
    }
    return true;
  }

  bool Delete(const std::string& bucket, const std::string& key, const std::string& file = "") {
    SYSTEM_LOGGER_INFO("delete file, bucket: %s, key: %s, file: %s", bucket.c_str(), key.c_str(), file.c_str());
    Aws::S3::Model::DeleteObjectRequest req;
    req.WithBucket(bucket).WithKey(key);
    Aws::S3::Model::DeleteObjectOutcome rsp = GetS3Ptr(bucket)->DeleteObject(req);

    if (!rsp.IsSuccess()) {
      SYSTEM_LOGGER_ERROR("delete file error: %s", rsp.GetError().GetMessage().c_str());
      return false;
    }
    if (!file.empty() && boost::filesystem::exists(file)) {
      boost::filesystem::remove(file);
    }
    return true;
  }

  bool Exists(const std::string& bucket, const std::string& key) {
    Aws::S3::Model::HeadObjectRequest req;
    req.WithBucket(bucket).WithKey(key);
    Aws::S3::Model::HeadObjectOutcome rsp = GetS3Ptr(bucket)->HeadObject(req);
    if (!rsp.IsSuccess()) {
      SYSTEM_LOGGER_ERROR("head file error: %s", rsp.GetError().GetMessage().c_str());
      return false;
    }
    return true;
  }

  std::string ETag(const std::string& bucket, const std::string& key) {
    Aws::S3::Model::HeadObjectRequest req;
    req.WithBucket(bucket).WithKey(key);
    Aws::S3::Model::HeadObjectOutcome rsp = GetS3Ptr(bucket)->HeadObject(req);
    if (!rsp.IsSuccess()) {
      SYSTEM_LOGGER_ERROR("head file error: %s", rsp.GetError().GetMessage().c_str());
      return "";
    }
    return rsp.GetResult().GetETag();
  }

 private:
  EnvVarPtr CreateCosEnv(const char* config) {
    EnvVarPtr cos_env = std::make_shared<CEnvVar>();
    cos_env->ParseFromEnvInner(FLAGS_cos_config);
    cos_env->ParseFromEnvInner(config ? config : "");
    return cos_env;
  }
  EnvVarPtr CreateCosEnv(const std::string& config) { return CreateCosEnv(config.c_str()); }

  void CreateClient(EnvVarPtr cos_env, S3Ptr& client) {
    if (!cos_env) {
      return;
    }

    Aws::Client::ClientConfiguration cfg;
    cfg.endpointOverride = cos_env->m_strEndPoint;
    cfg.verifySSL = cos_env->m_VerifySSL;
    Aws::Auth::AWSCredentials cred(cos_env->m_strSecretID, cos_env->m_strSecretKey);
    if (cos_env->m_strScheme == kStringHttp) {
      cfg.scheme = Aws::Http::Scheme::HTTP;
    }
    client = std::make_shared<Aws::S3::S3Client>(cred, cfg, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never,
                                                 cos_env->m_VirtualAddressing);
  }

  S3Ptr GetS3Ptr(const std::string& bucket) {
    auto iter = s3client_others_.find(bucket);
    return (iter != s3client_others_.end() && iter->second != nullptr) ? iter->second : s3client_;
  }

 private:
  static constexpr int kRetryTimes = 3;
  static constexpr const char* kStringHttp = "http";
  static constexpr const char* kStringPathDelimiter = "/";

  S3Ptr s3client_;
  std::map<std::string, S3Ptr> s3client_others_;
};

}  // namespace utils
