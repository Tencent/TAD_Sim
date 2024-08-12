// Copyright (c) 2022, Tencent Inc.
// All rights reserved.

#include <fstream>
#include <functional>
#include <iostream>
#include <map>

#include "aws/core/Aws.h"
#include "aws/core/auth/AWSCredentialsProvider.h"
#include "aws/s3/S3Client.h"
#include "aws/s3/model/GetObjectRequest.h"
#include "aws/s3/model/HeadObjectRequest.h"
#include "aws/s3/model/ListObjectsRequest.h"
#include "aws/s3/model/PutObjectRequest.h"
#include "gflags/gflags.h"

DEFINE_string(ak, "", "aws cos secret id");
DEFINE_string(sk, "", "aws cos secret key");
DEFINE_string(endpoint, "cos-internal.ap-guangzhou.tencentcos.cn", "aws cos endpoint");
DEFINE_string(bucket, "", "cos bucket");

DEFINE_string(key, "", "cos key");
DEFINE_string(scheme, "http", "");
DEFINE_string(input, "", "");
DEFINE_string(output, "", "");
DEFINE_string(get_op, "get", "");
DEFINE_string(put_op, "put", "");
DEFINE_string(head_op, "head", "");
DEFINE_string(list_op, "list", "");
DEFINE_bool(virtualaddr, false, "");

DEFINE_string(usage, R"(

  Usage: AwsCosTool COMMAND [--ak=<access key>]
                            [--sk=<secret key>]
                            [--endpoint=<endpoint>]
                            --bucket=<bucket> --key=<key> [--input=<file>] [--output=<file>]

  Options:
    --ak=<id>             access key []
    --sk=<key>            secret key []
    --endpoint=<endpoint> endpoint [default: cos-internal.ap-guangzhou.tencentcos.cn]
    --bucket=<bucket>     cos bucket
    --key=<key>           cos key
    --input=<file>        input file
    --output=<file>       output file
    --virtualaddr=<bool>  is virtual address [default: false]

  Commands:
    get                   get object
    put                   put object
    head                  head object
    list                  list bucket

)",
              "");

#define CHECK_ARG(arg)                                                    \
  {                                                                       \
    google::CommandLineFlagInfo info;                                     \
    if (google::GetCommandLineFlagInfo(#arg, &info) && info.is_default) { \
      std::cerr << FLAGS_usage << std::endl;                              \
      exit(-1);                                                           \
    }                                                                     \
  }

Aws::S3::S3Client* CreateClient() {
  Aws::SDKOptions options;
  options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Off;
  Aws::InitAPI(options);

  Aws::Client::ClientConfiguration cfg;
  cfg.endpointOverride = FLAGS_endpoint;
  cfg.scheme = FLAGS_scheme == "http" ? Aws::Http::Scheme::HTTP : Aws::Http::Scheme::HTTPS;

  Aws::Auth::AWSCredentials cred(FLAGS_ak, FLAGS_sk);

  return new Aws::S3::S3Client(cred, cfg, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, FLAGS_virtualaddr);
}

void HeadHandler() {
  Aws::S3::Model::HeadObjectRequest req;
  req.WithBucket(FLAGS_bucket).WithKey(FLAGS_key);
  Aws::S3::Model::HeadObjectOutcome rsp = CreateClient()->HeadObject(req);
  if (!rsp.IsSuccess()) {
    std::cerr << "HEAD failed: " << rsp.GetError().GetExceptionName() << rsp.GetError().GetMessage() << std::endl;
    return;
  }

  std::cout << "last-modified: " << rsp.GetResult().GetLastModified().ToGmtString(Aws::Utils::DateFormat::RFC822)
            << std::endl;
  std::cout << "etag: " << rsp.GetResult().GetETag() << std::endl;
  std::cout << "content-encoding: " << rsp.GetResult().GetContentEncoding() << std::endl;
  std::cout << "content-type: " << rsp.GetResult().GetContentType() << std::endl;
  std::cout << "content-length: " << rsp.GetResult().GetContentLength() << std::endl;
  for (auto&& kv : rsp.GetResult().GetMetadata()) {
    std::cout << kv.first << " : " << kv.second << std::endl;
  }
}

void GetHandler() {
  Aws::S3::Model::GetObjectRequest req;
  req.WithBucket(FLAGS_bucket).WithKey(FLAGS_key);
  Aws::S3::Model::GetObjectOutcome rsp = CreateClient()->GetObject(req);
  if (!rsp.IsSuccess()) {
    std::cerr << "GET failed: " << rsp.GetError().GetExceptionName() << rsp.GetError().GetMessage() << std::endl;
    return;
  }

  if (FLAGS_output.empty()) {
    std::cout << rsp.GetResult().GetBody().rdbuf() << std::endl;
    return;
  }
  std::fstream output(FLAGS_output, std::ios::out | std::ios::binary);
  output << rsp.GetResult().GetBody().rdbuf();
}

void ListHandler() {
  Aws::S3::Model::ListObjectsRequest req;
  req.WithBucket(FLAGS_bucket).WithPrefix(FLAGS_key);
  Aws::S3::Model::ListObjectsOutcome rsp = CreateClient()->ListObjects(req);
  if (!rsp.IsSuccess()) {
    std::cerr << "LIST failed: " << rsp.GetError().GetExceptionName() << rsp.GetError().GetMessage() << std::endl;
    return;
  }

  for (auto&& object : rsp.GetResult().GetContents()) {
    std::cout << object.GetKey() << std::endl;
  }
}

void PutHandler() { CHECK_ARG(input); }

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    std::cerr << FLAGS_usage << std::endl;
    exit(-1);
  }

  std::map<std::string, std::function<void()>> handlers = {
      {FLAGS_head_op, HeadHandler},
      {FLAGS_list_op, ListHandler},
      {FLAGS_get_op, GetHandler},
      {FLAGS_put_op, PutHandler},
  };

  std::string op = argv[1];
  if (handlers.count(op) <= 0) {
    std::cerr << FLAGS_usage << std::endl;
    exit(-1);
  }

  google::ParseCommandLineFlags(&argc, &argv, false);
  CHECK_ARG(bucket);
  CHECK_ARG(key);

  handlers.at(op)();

  return 0;
}
