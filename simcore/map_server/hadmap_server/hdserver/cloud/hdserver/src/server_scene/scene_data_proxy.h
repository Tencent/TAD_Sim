/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "database/entity/scenario.h"

class CSceneDataProxy {
 public:
  static CSceneDataProxy& Instance();

  int UploadScenario(const sTagEntityScenario& scenario);
  int DownloadScenario(const sTagEntityScenario& scenario, bool all_dir = true);

  int Upload(const std::string& bucket, const std::string& key, const std::string& file);
  int Download(const std::string& bucket, const std::string& key, const std::string& file, bool download_always = true,
               bool need_lock = false);
  int Delete(const std::string& bucket, const std::string& key, const std::string& file);
  static constexpr int kRetryTimes = 3;

 private:
  std::mutex download_mtx_;
};
