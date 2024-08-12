// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "transmission/trans_interface.h"
#include "transmission/idc_addr.h"

#define CURL_ON
#ifdef CURL_ON
#  include <curl/curl.h>
#endif

namespace hadmap {
struct JsonData {
  std::string data;
};

size_t curlCallBack(char* ptr, size_t size, size_t nmemb, void* userdata) {
  std::string curJsD(ptr, nmemb);
  (reinterpret_cast<JsonData*>(userdata))->data += curJsD;
  return nmemb;
}

TransInterface::TransInterface() {
  // baseUrl = "http://cyberan.sparta.html5.qq.com/hadmap_mapserver_test";
}

TransInterface::TransInterface(const std::string& cfgPath) {
  // baseUrl = "http://cyberan.sparta.html5.qq.com/hadmap_mapserver_test";
}

TransInterface::~TransInterface() {}

std::string TransInterface::location() const { return idcLocation(); }

bool TransInterface::get(const std::string& url, std::string& data) {
#ifdef CURL_ON
  CURL* curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  JsonData jsd;

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallBack);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &jsd);
    // curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");

    // curl_slist* plist = curl_slist_append(NULL, "Accept-Encoding:gzip");
    // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist );

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();

  data = jsd.data;
  return res;
#else
  return false;
#endif
}

bool TransInterface::post(const std::string& url, const std::string& postJson, std::string& data) {
#ifdef CURL_ON
  CURL* curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  JsonData jsd;

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallBack);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &jsd);

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postJson.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postJson.length());
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  data = jsd.data;
  return res;
#else
  return false;
#endif
}
}  // namespace hadmap
