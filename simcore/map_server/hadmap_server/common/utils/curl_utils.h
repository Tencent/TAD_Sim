// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "common/third/gutil/macros.h"
// #include "util/monotime.h"
#include "util/status.h"

typedef void CURL;

namespace hdserver {

class faststring;

// Simple wrapper around curl's "easy" interface, allowing the user to
// fetch web pages into memory using a blocking API.
//
// This is not thread-safe.
class EasyCurl {
 public:
  EasyCurl();
  ~EasyCurl();

  // Fetch the given URL into the provided buffer.
  // Any existing data in the buffer is replaced.
  // The optional param 'headers' holds additional headers.
  // e.g. {"Accept-Encoding: gzip"}
  Status FetchURL(const std::string& url, faststring* dst, const std::vector<std::string>& headers = {});

  // Issue an HTTP POST to the given URL with the given data.
  // Returns results in 'dst' as above.
  Status PostToURL(const std::string& url, const std::string& post_data, faststring* dst);

  // Set whether to verify the server's SSL certificate in the case of an HTTPS
  // connection.
  void set_verify_peer(bool verify) { verify_peer_ = verify; }

  void set_return_headers(bool v) { return_headers_ = v; }

  void set_timeout(MonoDelta t) { timeout_ = t; }

  void set_use_spnego(bool use_spnego) { use_spnego_ = use_spnego; }

  // Enable verbose mode for curl. This dumps debugging output to stderr, so
  // is only really useful in the context of tests.
  void set_verbose(bool v) { verbose_ = v; }

  // Overrides curl's HTTP method handling with a custom method string.
  void set_custom_method(std::string m) { custom_method_ = std::move(m); }

  // Returns the number of new connections created to achieve the previous transfer.
  int num_connects() const { return num_connects_; }

 private:
  // Do a request. If 'post_data' is non-NULL, does a POST.
  // Otherwise, does a GET.
  Status DoRequest(const std::string& url, const std::string* post_data, faststring* dst,
                   const std::vector<std::string>& headers = {});
  CURL* curl_;

  std::string custom_method_;

  // Whether to verify the server certificate.
  bool verify_peer_ = true;

  // Whether to return the HTTP headers with the response.
  bool return_headers_ = false;

  bool use_spnego_ = false;

  bool verbose_ = false;

  std::chrono::seconds timeout_;
  // MonoDelta timeout_;

  int num_connects_ = 0;

  DISALLOW_COPY_AND_ASSIGN(EasyCurl);
};

}  // namespace hdserver
