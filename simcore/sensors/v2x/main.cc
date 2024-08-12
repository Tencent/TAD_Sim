/**
 * @file main.cc
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include <glog/logging.h>
#include <iostream>
#include <memory>
#include "txsim_messenger.h"
#include "txsim_module_service.h"
#include "v2x.h"

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "usage: " << argv[0] << " name tcp_address(optional)" << std::endl;
    return 1;
  }
  google::InitGoogleLogging(argv[0]);

  std::shared_ptr<tx_sim::SimModule> module = std::make_shared<SimV2X>();
  std::string name(argv[1]), address;
  if (argc >= 3) {
    address = argv[2];
  }
  try {
    std::cout << "name:" << name << std::endl;
    std::cout << "module:" << module << std::endl;
    std::cout << "address:" << address << std::endl;

    tx_sim::SimModuleService service;
    service.Serve(name, module, address);
    service.Wait();
    exit(0);
  } catch (const std::exception &e) {
    std::cerr << "Serving module error:\n" << e.what() << std::endl;
    return 2;
  }
  return 0;
}
