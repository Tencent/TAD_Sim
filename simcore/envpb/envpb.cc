/**
 * @file envpb.cc
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include <iostream>
#include <memory>

#include "envpb.h"
#include "txsim_module_service.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "usage: " << argv[0] << " name tcp_address(optional)" << std::endl;
    return 1;
  }

  std::shared_ptr<tx_sim::SimModule> module = std::make_shared<envpb>();
  std::string name(argv[1]), address;
  if (argc >= 3) {
    address = argv[2];
  }
  try {
    tx_sim::SimModuleService service;
    service.Serve(name, module, address);
    service.Wait();
  } catch (const std::exception& e) {
    std::cerr << "Serving module error:\n" << e.what() << std::endl;
    return 2;
  }

  return 0;
}
