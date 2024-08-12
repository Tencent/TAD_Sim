// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "conf_cmd.h"
#include "constant.h"
#include "coordinator/local_client.h"
#include "utils/constant.h"
#include "utils/json_helper.h"

namespace tx_sim {
namespace cli {

//!
//! @brief 构造函数
//! @details 构造配置命令对象
//!
//! @param[in] parent 父命令对象
//! @param[in] factory 模拟客户端工厂对象
//!
ConfCmd::ConfCmd(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory) : SimCmd(parent, factory) {
  cmd_ = std::make_shared<CLI::App>(kConfCmdDesc, kConfCmdName);
  cmd_->set_help_flag(JoinOptName(kHelpOptAlias, kHelpOptName), kConfCmdHelpOptDesc);
  cmd_->require_subcommand();

  parent_->cmd_ptr()->add_subcommand(cmd_);

  sub_cmds_.emplace_back(new ConfShow(this, sim_cli_factory_));
  sub_cmds_.emplace_back(new ConfRestore(this, sim_cli_factory_));
  sub_cmds_.emplace_back(new ConfRm(this, sim_cli_factory_));
  sub_cmds_.emplace_back(new ConfUpd(this, sim_cli_factory_));
  sub_cmds_.emplace_back(new ConfAdd(this, sim_cli_factory_));
  sub_cmds_.emplace_back(new ConfSetSys(this, sim_cli_factory_));
}

//!
//! @brief 类名：ConfShow
//! @details 类功能：显示配置命令类
//!
ConfShow::ConfShow(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory) : SimCmd(parent, factory) {
  cmd_ = std::make_shared<CLI::App>(kConfShowDesc, kConfShowName);

  auto op = std::make_shared<Opt>();
  cmd_->add_option(JoinOptName(kModuleOptAlias, kModuleOptName), op->mod, kConfShowModOptDesc)->required();
  cmd_->callback([&, op]() {
    std::shared_ptr<ISimClient> cli = CreateSimClient();
    tx_sim::impl::CmdErrorCode ec;
    try {
      std::string config_str;
      std::vector<std::string> config_json_str;
      if (op->mod == "all") {
        ec = cli->GetAllModuleConfigs(config_json_str);
      } else if (op->mod == "sys") {
        ec = cli->GetSysConfigs(config_str);
      } else {
        ec = cli->GetModuleConfig(op->mod, config_str);
      }
      if (ec == tx_sim::impl::kCmdSucceed) {
        if (op->mod == "all") {
          for (std::string config : config_json_str) { std::cout << config << "\n"; }
        } else {
          std::cout << config_str << std::endl;
        }
      } else {
        std::cerr << "Error when trying to get config, received " + tx_sim::impl::Enum2String(ec) + " from server"
                  << std::endl;
      }
    } catch (const std::exception& e) { std::cerr << "cli error: " << e.what() << std::endl; }
    SetErrorCodeForRoot(ec, this);
  });

  parent_->cmd_ptr()->add_subcommand(cmd_);
}

//!
//! @brief 构造函数
//! @details 构造恢复配置命令对象
//!
//! @param[in] parent 父命令对象
//! @param[in] factory 模拟客户端工厂对象
//!
ConfRestore::ConfRestore(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory) : SimCmd(parent, factory) {
  cmd_ = std::make_shared<CLI::App>(kConfRestoreDesc, kConfRestoreName);

  cmd_->callback([&]() {
    std::shared_ptr<ISimClient> cli = CreateSimClient();
    tx_sim::impl::CmdErrorCode ec;
    try {
      ec = cli->RestoreDefaultConfigs();
      if (ec == tx_sim::impl::kCmdSucceed) {
        std::cout << "Succeed! Configuration has been restored to default." << std::endl;
      } else {
        std::cerr << "Error when trying to restore config, received " + tx_sim::impl::Enum2String(ec) + " from server"
                  << std::endl;
      }
    } catch (const std::exception& e) { std::cerr << "cli error: " << e.what() << std::endl; }
    SetErrorCodeForRoot(ec, this);
  });

  parent_->cmd_ptr()->add_subcommand(cmd_);
}

//!
//! @brief 构造函数
//! @details 构造删除配置命令对象
//!
//! @param[in] parent 父命令对象
//! @param[in] factory 模拟客户端工厂对象
//!
ConfRm::ConfRm(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory) : SimCmd(parent, factory) {
  cmd_ = std::make_shared<CLI::App>(kConfRmDesc, kConfRmName);

  auto op = std::make_shared<Opt>();
  cmd_->add_option(JoinOptName(kModuleOptAlias, kModuleOptName), op->mod, kConfRmModOptDesc)->required();

  cmd_->callback([&, op]() {
    std::shared_ptr<ISimClient> cli = CreateSimClient();
    tx_sim::impl::CmdErrorCode ec;
    try {
      ec = cli->RemoveModuleConfig(op->mod);
      if (ec == tx_sim::impl::kCmdSucceed) {
        std::cout << "Succeed! Configuration for module " + op->mod + " has been removed." << std::endl;
      } else {
        std::cerr << "Error when trying to remove config, received " + tx_sim::impl::Enum2String(ec) + " from server"
                  << std::endl;
      }
    } catch (const std::exception& e) { std::cerr << "cli error: " << e.what() << std::endl; }
    SetErrorCodeForRoot(ec, this);
  });

  parent_->cmd_ptr()->add_subcommand(cmd_);
}

//!
//! @brief 构造函数
//! @details 构造更新配置命令对象
//!
//! @param[in] parent 父命令对象
//! @param[in] factory 模拟客户端工厂对象
//!
ConfUpd::ConfUpd(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory) : SimCmd(parent, factory) {
  cmd_ = std::make_shared<CLI::App>(kConfUpdDesc, kConfUpdName);
  op = std::make_shared<Opt>();
  cmd_->add_option(JoinOptName(kFileOptAlias, kFileOptName), op->file_name, kConfUpdFileOptDesc)
      ->check(CLI::ExistingFile);
  cmd_->add_option(JoinOptName(kConfigOptAlias, kConfigOptName), op->config_json_str, kConfShowModOptDesc);
  // exactly one of the two options is required
  cmd_->require_option(1, 1);
  cmd_->callback([&]() {
    std::shared_ptr<ISimClient> cli = CreateSimClient();
    std::string config_json_str;
    GetConfigJsonStrFromCLI(config_json_str, cmd_);
    tx_sim::impl::CmdErrorCode ec;
    try {
      ec = cli->UpdateModuleConfig(config_json_str);
      if (ec == tx_sim::impl::kCmdSucceed) {
        std::cout << "Succeed! Configuration for target module has been updated." << std::endl;
      } else {
        std::cerr << "Error when trying to update config, received " + tx_sim::impl::Enum2String(ec) + " from server"
                  << std::endl;
      }
    } catch (const std::exception& e) { std::cerr << "cli error: " << e.what() << std::endl; }
    SetErrorCodeForRoot(ec, this);
  });
  parent_->cmd_ptr()->add_subcommand(cmd_);
}

//!
//! @brief 构造函数
//! @details 构造添加配置命令对象
//!
//! @param[in] parent 父命令对象
//! @param[in] factory 模拟客户端工厂对象
//!
ConfAdd::ConfAdd(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory) : SimCmd(parent, factory) {
  cmd_ = std::make_shared<CLI::App>(kConfAddDesc, kConfAddName);
  op = std::make_shared<Opt>();
  cmd_->add_option(JoinOptName(kFileOptAlias, kFileOptName), op->file_name, kConfAddFileOptDesc)
      ->check(CLI::ExistingFile);
  cmd_->add_option(JoinOptName(kConfigOptAlias, kConfigOptName), op->config_json_str, kConfAddConfigOptDesc);
  cmd_->require_option(1, 1);
  cmd_->callback([&]() {
    std::shared_ptr<ISimClient> cli = CreateSimClient();
    std::string config_json_str;
    GetConfigJsonStrFromCLI(config_json_str, cmd_);
    tx_sim::impl::CmdErrorCode ec;
    try {
      ec = cli->AddModuleConfig(config_json_str);
      if (ec == tx_sim::impl::kCmdSucceed) {
        std::cout << "Succeed! You have added configuration for a new module." << std::endl;
      } else {
        std::cerr << "Error when trying to added config, received " + tx_sim::impl::Enum2String(ec) + " from server"
                  << std::endl;
      }
    } catch (const std::exception& e) { std::cerr << "cli error: " << e.what() << std::endl; }
    SetErrorCodeForRoot(ec, this);
  });
  parent_->cmd_ptr()->add_subcommand(cmd_);
}

//!
//! @brief 构造函数：ConfSetSys
//! @details 构造 ConfSetSys 类的实例
//!
//! @param[in] parent 指向父类 SimCmd 的指针
//! @param[in] factory 指向 ISimClientFactory 的智能指针
//!
ConfSetSys::ConfSetSys(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory) : SimCmd(parent, factory) {
  cmd_ = std::make_shared<CLI::App>(kConfSetSysDesc, kConfSetSysName);
  op = std::make_shared<Opt>();
  cmd_->add_option(JoinOptName(kFileOptAlias, kFileOptName), op->file_name, kConfSetSysFileOptDesc)
      ->check(CLI::ExistingFile);
  cmd_->add_option(JoinOptName(kConfigOptAlias, kConfigOptName), op->config_json_str, kConfSetSysConfigOptDesc);
  cmd_->require_option(1, 1);
  cmd_->callback([&]() {
    std::shared_ptr<ISimClient> cli = CreateSimClient();
    std::string config_json_str;
    GetConfigJsonStrFromCLI(config_json_str, cmd_);
    tx_sim::impl::CmdErrorCode ec;
    try {
      ec = cli->SetSysConfigs(config_json_str);
      if (ec == tx_sim::impl::kCmdSucceed) {
        std::cout << "Succeed! You have set configuration for system." << std::endl;
      } else {
        std::cerr << "Error when trying to set config for system, received " + tx_sim::impl::Enum2String(ec) +
                         " from server"
                  << std::endl;
      }
    } catch (const std::exception& e) { std::cerr << "cli error: " << e.what() << std::endl; }
    SetErrorCodeForRoot(ec, this);
  });
  parent_->cmd_ptr()->add_subcommand(cmd_);
}

void ICLIConfigJson::GetConfigJsonStrFromCLI(std::string& config_json_str, std::shared_ptr<CLI::App> cmd) {
  CLI::Option* file_opt = cmd->get_option(kFileOptAlias);
  if (*file_opt) {
    Json::Value doc;
    tx_sim::utils::ReadJsonDocFromFile(doc, op->file_name);
    config_json_str = Json::writeString(Json::StreamWriterBuilder(), doc);
  } else {
    config_json_str = op->config_json_str;
  }
}

}  // namespace cli
}  // namespace tx_sim
