// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "cli.hpp"

#include "root_cmd.h"
#include "sim_client.h"

int main(int argc, char** argv) {
  std::unique_ptr<tx_sim::cli::RootCmd> root_cmd(
      new tx_sim::cli::RootCmd(nullptr, std::make_shared<tx_sim::cli::SimClientFactory>()));
  root_cmd->ExecuteCmd(argc, argv);
  return (root_cmd->error_code() == tx_sim::cli::kCmdSucceed) ? 0 : root_cmd->error_code();
}
