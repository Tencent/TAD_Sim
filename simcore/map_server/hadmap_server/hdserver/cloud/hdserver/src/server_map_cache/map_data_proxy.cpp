/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "map_data_proxy.h"
#include <stdlib.h>
#include <chrono>

#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/date.hpp>
#include <boost/date_time/date_facet.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/time_duration.hpp>
#include <boost/date_time/time_facet.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding.hpp>
#include <boost/uuid/detail/md5.hpp>
#include "./map_data_cache.h"
#include "common/log/system_logger.h"
#include "common/map_data/map_data_predef.h"
#include "var/flags.h"

#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

#include "../engine/config.h"
#include "../engine/error_code.h"
#include "../engine/util/scene_util.h"
// #include <util/auth_tool.h>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

#include "common/utils/misc.h"
#include "src/MD5.h"

using namespace boost;
using namespace boost::date_time;
using namespace std::chrono;
time_input_facet<posix_time::ptime, char>* tFacet(
    new time_input_facet<posix_time::ptime, char>("%a, %d %b %Y %H:%M:%S GMT"));

inline unsigned int ToHex(unsigned char x) { return x > 9 ? x + 55 : x + 48; }

inline unsigned char FromHex(unsigned char x) {
  unsigned char y = 0;
  if (x >= 'A' && x <= 'Z')
    y = x - 'A' + 10;
  else if (x >= 'a' && x <= 'z')
    y = x - 'a' + 10;
  else if (x >= '0' && x <= '9')
    y = x - '0';
  else
    assert(0);
  return y;
}

void sTagServiceHadmapInfo::Reset() {
  m_strHadmapName = "";
  m_strCosBucket = "";
  m_strCosKey = "";
  m_strLastModifiedTime = "";
  m_llTimestamp = 0;
}

void sTagServiceHadmapInfo::ToTimeStamp() {
  if (m_strLastModifiedTime.size() < 1) {
    return;
  }

  thread_local std::stringstream s_SS;
  s_SS.imbue(std::locale(s_SS.getloc(), tFacet));
  s_SS.clear();
  s_SS.str("");
  s_SS << m_strLastModifiedTime;

  boost::posix_time::ptime pt;
  s_SS >> pt;
  // std::string strT = boost::posix_time::to_simple_string(pt);

  boost::posix_time::ptime end(boost::gregorian::date(1970, 1, 1));
  boost::posix_time::time_duration dur = pt - end;
  m_llTimestamp = dur.total_seconds();
}

void sTagServiceHadmapInfo::ToGMTTime() {}

CMapDataProxy::CMapDataProxy() {}

CMapDataProxy& CMapDataProxy::Instance() {
  static CMapDataProxy instance;

  return instance;
}

void CMapDataProxy::Init() {
  Aws::SDKOptions options;
  options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Off;
  Aws::InitAPI(options);

  auto create_s3client = [&](const CEnvVar& cos_env, Aws::S3::S3Client** client) {
    Aws::Client::ClientConfiguration cfg;
    cfg.endpointOverride = cos_env.m_strEndPoint;
    cfg.verifySSL = cos_env.m_VerifySSL;
    Aws::Auth::AWSCredentials cred(cos_env.m_strSecretID, cos_env.m_strSecretKey);  // 认证的Key
    if (cos_env.m_strScheme == "http") {
      cfg.scheme = Aws::Http::Scheme::HTTP;
    }
    *client = new Aws::S3::S3Client(cred, cfg, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never,
                                    cos_env.m_VirtualAddressing);
  };
  create_s3client(CEngineConfig::Instance().EnvVar(), &m_Client);

  Json::Value root = StringToJson(getenv("TXSIM_COS_CONFIG_OTHERS"));
  for (auto& one : root) {
    // ["TXSIM_COS_CONFIG1", "TXSIM_COS_CONFIG2"]
    CEnvVar cos_env;
    cos_env.ParseFromEnvInner(FLAGS_cos_config);
    cos_env.ParseFromEnvInner(JsonToString(one));
    if (cos_env.m_strDefaultBucket.empty()) {
      continue;
    }
    create_s3client(cos_env, &(s3client_others_[cos_env.m_strDefaultBucket]));
  }
}

std::string CMapDataProxy::UrlEncode(const std::string& inputString) {
  std::string strTemp = "";
  size_t length = inputString.length();
  for (size_t i = 0; i < length; i++) {
    if (isalnum((unsigned char)inputString[i]) || (inputString[i] == '.') || (inputString[i] == '-') ||
        (inputString[i] == '_')) {
      strTemp += inputString[i];
    } else {
      strTemp += '%';
      strTemp += ToHex((unsigned char)inputString[i] >> 4);
      strTemp += ToHex((unsigned char)inputString[i] % 16);
    }
  }
  return strTemp;
}

std::string CMapDataProxy::UrlDecode(const std::string& inputString) {
  std::string strTemp = "";
  size_t length = inputString.length();
  for (size_t i = 0; i < length; i++) {
    if (inputString[i] == '%') {
      assert(i + 2 < length);
      unsigned char high = FromHex((unsigned char)inputString[++i]);
      unsigned char low = FromHex((unsigned char)inputString[++i]);
      strTemp += high * 16 + low;
    } else {
      strTemp += inputString[i];
    }
  }
  return strTemp;
}

int CMapDataProxy::HttpGet(std::string url, std::string& res) {
  /* example req parameter:
  std::string host = "simdev.noscan.sparta.html5.qq.com";
  std::string port = "80";
  std::string strSalt = "tlasmai";
  */

  auto const host = CEngineConfig::Instance().EnvVar().m_strHost;
  auto const port = CEngineConfig::Instance().EnvVar().m_strPort;
  // std::string strSalt = CEngineConfig::Instance().EnvVar().m_strSalt;

  // url = UrlEncode(url);

  std::string strBody;
  SYSTEM_LOGGER_INFO("request to url: %s:%s, target: %s", host.c_str(), port.c_str(), url.c_str());
  int nRet = doHttpGet(host, port, url, strBody);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("Error do http get: %s ", url.c_str());
    return nRet;
  } else {
    res = strBody;
  }
  return 0;
}

int CMapDataProxy::GetAllHadmapInfoFromService(std::vector<sTagServiceHadmapInfo>& hadmapInfo) {
  std::string strSalt = CEngineConfig::Instance().EnvVar().m_strSalt;
  std::string strTimeStamp = GetCurTimeInMilliSeconds();
  std::string strSign = GenMD5(strTimeStamp + strSalt);

  boost::to_lower(strSign);

  std::string strPath = CEngineConfig::Instance().EnvVar().m_strAllMapURL;
  std::string target = strPath + "ts=" + strTimeStamp + "&sign=" + strSign;

  std::string strBody;
  int nRet = HttpGet(target, strBody);
  if (nRet != 0) {
    return nRet;
  }

  SYSTEM_LOGGER_INFO("return body: %s", strBody.c_str());

  SYSTEM_LOGGER_INFO("start parse return body");
  std::cout << "yyyyyyyyyyyyyyy" << std::endl;

  nRet = ParserAllMapCosInfo(strBody.c_str(), hadmapInfo);
  if (nRet != HSEC_OK) {
    return nRet;
  }

  return HSEC_OK;
}

int CMapDataProxy::GetHadmapInfoFromService(const char* strMapNameWhole, sTagServiceHadmapInfo& hadmapInfo) {
  if (hadmapInfo.m_strCosBucket.empty()) {
    hadmapInfo.m_strCosBucket = CEngineConfig::Instance().EnvVar().m_strDefaultBucket;
  }
  if (!hadmapInfo.m_strCosBucket.empty() && !hadmapInfo.m_strCosKey.empty()) {
    SYSTEM_LOGGER_INFO("has cos {bucket, key} in param: %s, %s", hadmapInfo.m_strCosBucket.c_str(),
                       hadmapInfo.m_strCosKey.c_str());
    return HSEC_OK;
  }
  // std::string strPath = "/simService/maps/queryByMapName?mapName=";--
  // std::string strPath = "/simService/third/maps/query?";

  std::string strMapName;
  std::string version;

  std::string tmp(strMapNameWhole);
  int pos = tmp.find_first_of(VERSION_NAME_DELIM);
  if (pos == std::string::npos) {
    SYSTEM_LOGGER_ERROR("raw map name provided (not version).");
    strMapName = tmp;
  } else {
    // std::cout << "pos is: " << pos << std::endl;
    version = tmp.substr(0, pos);
    strMapName = tmp.substr(pos + 1, tmp.size() - pos - 1);

    SYSTEM_LOGGER_INFO("get hadmap info for: %s, version: %s", strMapName.c_str(), version.c_str());
    // std::cout << "version: " << version << std::endl;
    // std::cout << "map: " << strMapName << std::endl;
  }

  std::string strSalt = CEngineConfig::Instance().EnvVar().m_strSalt;
  std::string strTimeStamp = GetCurTimeInMilliSeconds();
  std::string strSign = GenMD5(strMapName + strTimeStamp + strSalt);
  boost::to_lower(strSign);

  std::string strPath = CEngineConfig::Instance().EnvVar().m_strURL;
  std::string target;
  // target = strPath + "name=" + strMapName + "&version=" + "&ts=" + strTimeStamp + "&sign=" + strSign;
  target = strPath + strMapName + "&version=" + version + "&ts=" + strTimeStamp + "&sign=" + strSign;

  SYSTEM_LOGGER_INFO("get hadmap info from service: with req url:  %s", target.c_str());
  std::string strBody;
  int nRet = HttpGet(target, strBody);
  if (nRet != 0) {
    return nRet;
  }

  SYSTEM_LOGGER_INFO("return body: %s", strBody.c_str());

  SYSTEM_LOGGER_INFO("start parse return body");
  nRet = ParseResMapCosInfo(strBody.c_str(), hadmapInfo);
  // 适配交通仿真新架构，通过版本号生成COS路径
  if (hadmapInfo.m_strCosKey.empty()) {
    hadmapInfo.m_strHadmapName = strMapName;
    hadmapInfo.m_version = version;
    hadmapInfo.m_strCosBucket = CEngineConfig::Instance().EnvVar().m_strDefaultBucket;
    hadmapInfo.m_strCosKey = GenMapEditorCosKeyByVersion(hadmapInfo.m_strHadmapName, hadmapInfo.m_version);
    hadmapInfo.m_strJsonKey = GenMapEditorJsonKeyByVersion(hadmapInfo.m_strHadmapName, hadmapInfo.m_version);
    SYSTEM_LOGGER_INFO("reset cos key: %s %s %s %s", hadmapInfo.m_strHadmapName.c_str(), hadmapInfo.m_version.c_str(),
                       hadmapInfo.m_strCosKey.c_str(), hadmapInfo.m_strJsonKey.c_str());
  }
  /////////////////////////////////////////
  return nRet;
}

int CMapDataProxy::GetHadmapInfoFromCos(const char* strMapName, sTagServiceHadmapInfo& hadmapInfo) {
  if (hadmapInfo.m_strCosBucket.size() < 1 || hadmapInfo.m_strCosKey.size() < 1 || !strMapName ||
      strlen(strMapName) < 1) {
    SYSTEM_LOGGER_ERROR("cosbucket, coskey is null! map %s not exist", strMapName);
    return HSEC_COS_BUCKET_OR_KEY_STRING_NULL_ERROR;
  }

  boost::filesystem::path p = strMapName;
  std::string strExt = p.extension().string();
  if (kHadmapTypeDict.count(strExt) <= 0) {
    SYSTEM_LOGGER_ERROR("map file extension is wrong: %s !", strMapName);
    return HSEC_MAP_FILE_FORMAT_ERROR;
  }

  Aws::S3::Model::HeadObjectRequest headReq;
  headReq.WithBucket(hadmapInfo.m_strCosBucket).WithKey(hadmapInfo.m_strCosKey);
  Aws::S3::Model::HeadObjectOutcome headRes = m_Client->HeadObject(headReq);
  if (headRes.IsSuccess()) {
    hadmapInfo.m_strLastModifiedTime =
        headRes.GetResult().GetLastModified().ToGmtString(Aws::Utils::DateFormat::RFC822);
  } else if (headRes.GetError().GetResponseCode() == Aws::Http::HttpResponseCode::NOT_FOUND) {
    return HSEC_COS_NOT_FOUND;
  } else {
    std::stringstream ss;
    ss << "error while headobject " << headRes.GetError().GetExceptionName() << " " << headRes.GetError().GetMessage();
    SYSTEM_LOGGER_ERROR("%s", ss.str().c_str());
    return HSEC_DOWNLOAD_HADMAP_FAILED;
  }

  return HSEC_OK;
}

bool CMapDataProxy::IsExistsInCos(const std::string& strBucket, const std::string& strCosKey) {
  Aws::S3::Model::HeadObjectRequest headReq;
  headReq.WithBucket(strBucket).WithKey(strCosKey);
  Aws::S3::Model::HeadObjectOutcome headRes = m_Client->HeadObject(headReq);
  return headRes.IsSuccess();
}

boost::filesystem::path CMapDataProxy::composeFilePath(std::string mapName) {
  boost::filesystem::path initPath = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path hadmapPath = initPath;
  hadmapPath.append("hadmap");

  boost::filesystem::path filePath = hadmapPath.append(mapName);
  return filePath;
}

int CMapDataProxy::FetchHadmap(const char* strHadmapFile, sTagServiceHadmapInfo& info) {
  // 文件名为空检查
  if (!strHadmapFile) {
    SYSTEM_LOGGER_ERROR("query hadmap file name is null!");
    return -1;
  }

  int nRet = HSEC_OK;

  std::string fpath = composeFilePath(strHadmapFile).string();

  nRet = GetHadmapInfoFromService(strHadmapFile, info);
  if (nRet != HSEC_OK) {
    SYSTEM_LOGGER_ERROR("from service get hadmap info error!");
    return nRet;
  }

  SYSTEM_LOGGER_INFO("start to download map file bucket: %s, key: %s , to : %s", info.m_strCosBucket.c_str(),
                     info.m_strCosKey.c_str(), fpath.c_str());
  for (int i = 0; i < 3; ++i) {
    nRet = DownloadMapFile(info, fpath);
    if (nRet == HSEC_OK) {
      break;
    }
  }

  if (nRet == HSEC_OK) {
    SYSTEM_LOGGER_INFO("start to update hadmap info, (in cloud version skip it.)");
    // nRet = UpdateHadmapInfo(strMapName.c_str());
  } else {
    SYSTEM_LOGGER_ERROR("download hadmap file %s failed!", fpath.c_str());
  }

  return nRet;
}

int CMapDataProxy::doHttpGet(std::string strHost, std::string strPort, std::string strURL, std::string& strBody) {
  SYSTEM_LOGGER_INFO("start http get action");

  strBody = "";
  using tcp = boost::asio::ip::tcp;
  namespace http = boost::beast::http;

  int version = 11;

  try {
    boost::asio::io_context ioc;
    tcp::resolver resolver{ioc};
    tcp::socket socket{ioc};
    auto const results = resolver.resolve(strHost, strPort);

    boost::asio::connect(socket, results.begin(), results.end());

    // set up an http get request message
    http::request<http::string_body> req{http::verb::get, strURL, version};
    req.set(http::field::host, strHost);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    boost::beast::http::write(socket, req);

    boost::beast::flat_buffer buffer;

    http::response<http::dynamic_body> res;

    http::read(socket, buffer, res);

    auto itr = res.base().begin();
    for (; itr != res.base().end(); ++itr) {
      boost::beast::string_view n = itr->name_string();
      std::string strN = n.to_string();
      boost::beast::string_view v = itr->value();
      std::string strV = v.to_string();
      std::cout << strN << " : " << strV << std::endl;
    }

    strBody = boost::beast::buffers_to_string(res.body().data());
    // std::string strRes = boost::locale::conv::to_utf<char>(strBody, "utf8");
    std::cout << strBody << std::endl;

    // std::cout << strRes << std::endl;
    // std::cout << res << std::endl;

    // gracefully close the socket
    boost::system::error_code ec;
    socket.shutdown(tcp::socket::shutdown_both, ec);

    if (ec && ec != boost::system::errc::not_connected) throw boost::system::system_error{ec};
  } catch (std::exception const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    SYSTEM_LOGGER_ERROR("Error: %s", e.what());
    return HSEC_HTTP_GET_ERROR;
  }

  return HSEC_OK;
}

std::string CMapDataProxy::GetCurTimeInMilliSeconds() {
  /*
  boost::posix_time::ptime time_t_epoch(boost::gregorian::date(1970, 1, 1));
  boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
  now -= boost::posix_time::hours(8);
  boost::posix_time::time_duration diff = now - time_t_epoch;
  int64_t ms = diff.total_milliseconds();
  std::string strTimeStamp = std::to_string(diff.total_milliseconds());
  */

  milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  return std::to_string(ms.count());
}

int CMapDataProxy::ParserAllMapCosInfo(const char* strJson, std::vector<sTagServiceHadmapInfo>& mapVec) {
  Json::Value root;
  int ret = misc::ParseJson(strJson, &root, "parser all hadmap info");
  if (ret != HSEC_OK) {
    SYSTEM_LOGGER_ERROR("parse all map cos info failed");
    return ret;
  }

  // response
  Json::Value& jBody = root;

  if (!jBody.isNull()) {
    int nCode = jBody["code"].asInt();
    if (nCode != 0) {
      std::string strMsg = jBody["message"].asString();
      SYSTEM_LOGGER_ERROR("queryByMapName result error . message is : ", strMsg);
      return HSEC_PARSE_COS_ADDR_ERROR;
    }

    Json::Value& jData = jBody["data"];
    if (!jData.isNull()) {
      // SYSTEM_LOGGER_INFO("")
      // std::cout << "size of data is: " << jData.size() << std::endl;
      for (Json::Value& da : jData) {
        sTagServiceHadmapInfo info;
        json2HadmapInfo(da, info);
        mapVec.push_back(info);
      }
      std::cout << "after fill" << mapVec.size() << std::endl;

    } else {
      SYSTEM_LOGGER_ERROR("empty data field (getAllMapInfo) rest call resp.");
      return HSEC_HTTP_GET_ERROR;
    }
  } else {
    SYSTEM_LOGGER_INFO("empty resp from getAllMapInfo rest call(simService).");
    return HSEC_HTTP_GET_ERROR;
  }

  return HSEC_OK;
}
// int CMapDataProxy::ParseResMapCosAddr(const char* strJson, std::string& strCosBucket, std::string& strCosKey) {
int CMapDataProxy::ParseResMapCosInfo(const char* strJson, sTagServiceHadmapInfo& hadmapInfo) {
  Json::Value root;
  int ret = misc::ParseJson(strJson, &root, "parse Response map cos info");
  if (ret != HSEC_OK) {
    SYSTEM_LOGGER_ERROR("parse Response map cos info failed: %s", strJson);
    return ret;
  }

  // response
  Json::Value& jBody = root;

  if (!jBody.isNull()) {
    int nCode = jBody["code"].asInt();
    if (nCode != 0) {
      std::string strMsg = jBody["message"].asString();
      SYSTEM_LOGGER_ERROR("queryByMapName result error . message is : ", strMsg);
      return HSEC_PARSE_COS_ADDR_ERROR;
    }

    Json::Value& jData = jBody["data"];
    if (!jData.isNull()) {
      json2HadmapInfo(jData, hadmapInfo);
    }
  }

  SYSTEM_LOGGER_INFO("bucket: %s, key: %s !", hadmapInfo.m_strCosBucket.c_str(), hadmapInfo.m_strCosKey.c_str());
  SYSTEM_LOGGER_INFO("queryByMapName response parse finish!");

  return HSEC_OK;
}

void CMapDataProxy::json2HadmapInfo(const Json::Value& obj, sTagServiceHadmapInfo& info) {
  if (!obj.isObject()) {
    return;
  }
  info.m_strCosBucket = obj.get("cosBucket", "").asString();
  info.m_strCosKey = obj.get("cosKey", "").asString();
  info.m_version = obj.get("version", "").asString();
  info.m_strJsonKey = obj.get("jsonKey", "").asString();
  info.m_strHadmapName = boost::filesystem::path{info.m_strCosKey}.filename().string();
  return;
}

// using namespace qcloud_cos;

int CMapDataProxy::cosCheck(const std::string& cosBucket, const std::string& cosKey) {
  if (cosBucket.size() < 1 || cosKey.size() < 1) {
    SYSTEM_LOGGER_ERROR("cosbucket or coskey is null! Failed to update file.");
    return HSEC_COS_BUCKET_OR_KEY_STRING_NULL_ERROR;
  }
  return HSEC_OK;
}

int CMapDataProxy::getObjectToStr(const std::string& cosBucket, const std::string& cosKey, std::string& data) {
  if (cosCheck(cosBucket, cosKey) != HSEC_OK) {
    return HSEC_COS_BUCKET_OR_KEY_STRING_NULL_ERROR;
  }

  std::ostringstream os;

  Aws::S3::Model::GetObjectRequest req;
  req.WithBucket(cosBucket).WithKey(cosKey);
  auto getobjres = m_Client->GetObject(req);
  if (getobjres.IsSuccess()) {
    os << getobjres.GetResult().GetBody().rdbuf();
  } else {
    os << "Error while GetObject " << getobjres.GetError().GetExceptionName() << " "
       << getobjres.GetError().GetMessage();
    SYSTEM_LOGGER_ERROR("GetObject bucket:%s key:%s error:%s", cosBucket.c_str(), cosKey.c_str(), os.str().c_str());
    return HSEC_COS_OP_ERROR;
  }

  SYSTEM_LOGGER_INFO("get cosKey: %s against %s succ! ", cosKey.c_str(), cosBucket.c_str());
  data = os.str();

  return HSEC_OK;
}

int CMapDataProxy::uploadFile(const std::string& cosBucket, const std::string& cosKey, const std::string& fpath) {
  if (cosCheck(cosBucket, cosKey) != HSEC_OK) {
    return HSEC_COS_BUCKET_OR_KEY_STRING_NULL_ERROR;
  }

  Aws::S3::Model::PutObjectRequest putreq;
  putreq.WithBucket(cosBucket).WithKey(cosKey);
  auto inputData =
      Aws::MakeShared<Aws::FStream>("PutObjectInputStream", fpath, std::ios_base::in | std::ios_base::binary);
  putreq.SetBody(inputData);

  Aws::S3::Model::PutObjectOutcome putres = m_Client->PutObject(putreq);
  if (putres.IsSuccess()) {
    SYSTEM_LOGGER_INFO("upload file cosKey: %s against %s succ!", cosKey.c_str(), cosBucket.c_str());
  } else {
    std::ostringstream os;
    os << "Error while PutObject " << putres.GetError().GetExceptionName() << " " << putres.GetError().GetMessage();
    SYSTEM_LOGGER_ERROR("PutObject bucket:%s key:%s file:%s error:%s", cosBucket.c_str(), cosKey.c_str(), fpath.c_str(),
                        os.str().c_str());
    return HSEC_COS_OP_ERROR;
  }

  return HSEC_OK;
}

int CMapDataProxy::uploadStr(const std::string& cosBucket, std::string cosKey, const std::string& data) {
  if (cosCheck(cosBucket, cosKey) != HSEC_OK) {
    return HSEC_COS_BUCKET_OR_KEY_STRING_NULL_ERROR;
  }

  Aws::S3::Model::PutObjectRequest putreq;
  putreq.WithBucket(cosBucket).WithKey(cosKey);
  auto inputData = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream");
  (*inputData) << data;
  putreq.SetBody(inputData);

  Aws::S3::Model::PutObjectOutcome putres = m_Client->PutObject(putreq);
  if (putres.IsSuccess()) {
    SYSTEM_LOGGER_INFO("upload cosKey: %s against %s succ!", cosKey.c_str(), cosBucket.c_str());
  } else {
    std::ostringstream os;
    os << "Error while PutObject " << putres.GetError().GetExceptionName() << " " << putres.GetError().GetMessage();
    SYSTEM_LOGGER_ERROR("PutObject bucket:%s key:%s error:%s", cosBucket.c_str(), cosKey.c_str(), os.str().c_str());
    return HSEC_COS_OP_ERROR;
  }

  return HSEC_OK;
}

int CMapDataProxy::DownloadMapFile(std::string strCosBucket, std::string strCosKey, std::string strMapName) {
  sTagServiceHadmapInfo info;
  info.m_strCosBucket = strCosBucket;
  info.m_strCosKey = strCosKey;
  return DownloadMapFile(info, strMapName);
}

int CMapDataProxy::DownloadMapFile(sTagServiceHadmapInfo& info, const std::string& fpath) {
  if (info.m_strCosBucket.size() < 1 || info.m_strCosKey.size() < 1 || fpath.size() < 1) {
    SYSTEM_LOGGER_ERROR("cosbucket, coskey is null! eithere bucket: %s;key:%s ;mapName %s empty",
                        info.m_strCosBucket.c_str(), info.m_strCosKey.c_str(), fpath.c_str());
    return HSEC_COS_BUCKET_OR_KEY_STRING_NULL_ERROR;
  }

  boost::filesystem::path p = fpath;
  std::string strExt = p.extension().string();
  if (kHadmapTypeDict.count(strExt) <= 0) {
    SYSTEM_LOGGER_ERROR("map file extension is wrong: %s !", fpath);
    return HSEC_MAP_FILE_FORMAT_ERROR;
  }

  SYSTEM_LOGGER_INFO("Down load hadmap to: %s", fpath.c_str());

  Aws::S3::Model::GetObjectRequest req;
  req.WithBucket(info.m_strCosBucket).WithKey(info.m_strCosKey);
  auto getobjres = m_Client->GetObject(req);
  if (getobjres.IsSuccess()) {
    Aws::OFStream local_file;
    local_file.open(fpath, std::ios::out | std::ios::binary);
    local_file << getobjres.GetResult().GetBody().rdbuf();
    local_file.close();
    info.m_strEtag = getobjres.GetResult().GetETag();
  } else {
    std::stringstream ss;
    ss << "Error while ListBuckets " << getobjres.GetError().GetExceptionName() << " "
       << getobjres.GetError().GetMessage();
    SYSTEM_LOGGER_ERROR("%s", ss.str().c_str());
    return HSEC_DOWNLOAD_HADMAP_FAILED;
  }

  return HSEC_OK;
}

// deprecate this method(hadmap config related code should be deprecated)
int CMapDataProxy::UpdateHadmapInfo(const char* strMapName) {
  SYSTEM_LOGGER_INFO("CMapDataProxy UpdateHadmapInfo started!");

  boost::filesystem::path curPath = boost::filesystem::current_path();
  // curPath.append(strMapName);
  boost::filesystem::path destPath = CEngineConfig::Instance().HadmapDir();

  SYSTEM_LOGGER_INFO("source path is %s, destination path is %s", curPath.string().c_str(), destPath.string().c_str());

  int nRet =
      CMapDataCache::Instance().CopySingleHadmap(strMapName, curPath.string().c_str(), destPath.string().c_str());
  if (nRet != HSEC_OK) {
    SYSTEM_LOGGER_ERROR("CMapDataProxy UpdateHadmapInfo copy hadmap failed!");
    return nRet;
  }

  destPath.append(strMapName);

  SYSTEM_LOGGER_INFO("CMapDataProxy UpdateHadmapInfo update hadmap config file!");

  CMapDataCache::Instance().UpdateHadmapConfig(destPath.string().c_str(), strMapName);

  SYSTEM_LOGGER_INFO("CMapDataProxy UpdateHadmapInfo finished!");

  return HSEC_OK;
}

int CMapDataProxy::GetCosEnvironments() { return HSEC_OK; }
