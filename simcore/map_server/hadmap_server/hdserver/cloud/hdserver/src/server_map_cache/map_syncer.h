/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <engine/error_code.h>
#include <chrono>
#include <memory>
#include <thread>
#include "common/log/log.h"
#include "common/third/gutil/macros.h"
#include "common/utils/misc.h"
#include "engine/config.h"
#include "map_data_cache.h"
#include "map_data_proxy.h"
#include "var/flags.h"

class MapSyncer {
 public:
  static std::shared_ptr<MapSyncer> Create() {
    static std::shared_ptr<MapSyncer> ins_ptr(new MapSyncer());
    return ins_ptr;
  }

  ~MapSyncer() {
    SYSTEM_LOGGER_INFO("Destructing map syncer.");
    Stop();
  }
  void Start() {
    if (!FLAGS_enable_map_syncer) {
      return;
    }
    SYSTEM_LOGGER_INFO("Starting map syncer.");
    sync_thread = std::thread(&MapSyncer::doSync, this);
    running_.store(true);
  }

  void Stop() {
    SYSTEM_LOGGER_INFO("Stopping map syncer.");
    if (sync_thread.joinable()) sync_thread.join();

    running_.store(false);
  }

  bool IsRunning() const { return running_.load(); }

 private:
  void doSync() {
    SYSTEM_LOGGER_INFO("do map sync in dedicate thread.");
    while (running_.load()) {
      std::vector<sTagServiceHadmapInfo> hadmaps;
      int ret = CMapDataProxy::Instance().GetAllHadmapInfoFromService(hadmaps);

      if (ret != HSEC_OK) {
        SYSTEM_LOGGER_INFO("Error sync hadmap infos(all) from simService.");
      } else {
        for (auto& m : hadmaps) {
          SYSTEM_LOGGER_INFO("load hadmap %s", m.m_strHadmapName.c_str());

          std::string whole_name = computeMapCompleteName(m.m_strHadmapName, m.m_version);
          std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(whole_name.c_str());
          std::wstring wstrRes = CMapDataCache::Instance().LoadHadMap(wstrMapName.c_str());
          if (wstrRes != CMapDataCache::m_wstrSuccess) {
            SYSTEM_LOGGER_ERROR("Error load hadmap %s (in map syncer)", m.m_strHadmapName.c_str());
          }
        }
      }
      // make this sleep interval configurable.
      std::this_thread::sleep_for(std::chrono::seconds(FLAGS_map_syncer_interval_seconds));
    }
  }
  MapSyncer() : running_(false) {}
  DISALLOW_COPY_AND_ASSIGN(MapSyncer);

  std::thread sync_thread;
  std::atomic<bool> running_;
};
