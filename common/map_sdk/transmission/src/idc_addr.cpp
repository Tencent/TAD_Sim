// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "transmission/idc_addr.h"

#include <iostream>

#ifdef IDC_NET
#  include "transmission/qos_client.h"
#endif

#define MODID 64629441
#define CMD 65536

namespace hadmap {
std::string idcLocation() {
#ifdef IDC_NET
#  pragma message("IDC_NET ON")
  cl5::QOSREQUESTtag req;
  req._modid = MODID;
  req._cmd = CMD;

  float tm_out = 0.2;
  std::string err_msg;

  int ret = cl5::ApiGetRoute(req, tm_out, err_msg);
  if (ret < 0) {
    txlog::printf("ApiGetRoute failed, ret: %d, err: %s\n", ret, err_msg.c_str());
    return "";
  }

  struct timeval start, end;
  gettimeofday(&start, NULL);

  std::cout << "ip:" << req._host_ip << std::endl;
  std::cout << "port:" << req._host_port << std::endl;

  gettimeofday(&end, NULL);
  int use_time = static_cast<int>((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000);

  ret = ApiRouteResultUpdate(req, 0, use_time, err_msg);
  if (ret != 0) {
    txlog::printf("ApiRouteResultUpdate failed, ret: %d, err: %s\n", ret, err_msg.c_str());
  }

  std::string location = "http://" + req._host_ip + ":" + std::to_string(req._host_port) + "/hadmap_mapserver_test";
  return location;

#else
#  pragma message("IDC_NET OFF")
  return std::string("http://cyberan.sparta.html5.qq.com/hadmap_mapserver_test");
#endif
}
}  // namespace hadmap
