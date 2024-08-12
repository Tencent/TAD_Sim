/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "var/flags.h"

DEFINE_string(cos_config,
              R"({
                  "SecretId":"",
                  "SecretKey":"",
                  "Region":"",
                  "EndPoint":"",
                  "DefaultBucket":"",
                  "SignExpiredTime":360,
                  "ConnectTimeoutInms":6000,
                  "ReceiveTimeoutInms":5000,
                  "AsynThreadPoolSize":2,
                  "UploadPartSize":10485760,
                  "UploadCopyPartSize":10485760,
                  "UploadThreadPoolSize":5,
                  "LogoutType":1,
                  "LogLevel":3,
                  "DownloadThreadPoolSize":5,
                  "DownloadSliceSize":4194304
              })",
              "default cos config");

DEFINE_bool(enable_map_syncer, true, "default flag whether start the map syncer.");
DEFINE_int32(map_syncer_interval_seconds, 60, "default of map syncer run interval (in sec)");

DEFINE_string(sim_host, "127.0.0.1", "default sim-service host");
DEFINE_string(sim_port, "80", "default sim service port");
DEFINE_string(sim_salt, "tlasmai", "default sim service salt");
DEFINE_string(sim_url, "/simService/third/maps/query?name=", "default query hadmap url");
DEFINE_string(sim_all_map_url, "/simService/third/maps/allVersions?", "default qurey all hadmaps url");
