// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <iostream>
#include <memory>

#include "./src/txcar_interface.h"
#include "txsim_module_service.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "usage: " << argv[0] << " name tcp_address(optional)" << std::endl;
    return 1;
  }

  std::shared_ptr<tx_sim::SimModule> module = std::make_shared<tx_car::TxCarInterface>();
  std::string name(argv[2]), address;
  if (argc >= 4) {
    address = argv[4];
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
