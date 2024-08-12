// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "root_cmd.h"
#include "conf_cmd.h"
#include "constant.h"
#include "coordinator/local_client.h"
#include "utils/constant.h"
#include "utils/os.h"

namespace tx_sim {
namespace cli {

//!
//! @brief 构造函数：SimCmd
//! @details 构造 SimCmd 类的实例
//!
//! @param[in] parent 指向父类 SimCmd 的指针
//! @param[in] factory 指向 ISimClientFactory 的智能指针
//!
SimCmd::SimCmd(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory)
    : parent_(parent), sim_cli_factory_(factory) {}

//!
//! @brief 析构函数：~SimCmd
//! @details 析构 SimCmd 类的实例
//!
SimCmd::~SimCmd() {}

//!
//! @brief 函数名：SetErrorCodeForRoot
//! @details 函数功能：设置根命令的错误代码
//!
//! @param[in] ec 错误代码
//! @param[in] cur_cmd 指向当前 SimCmd 的指针
//!
void SimCmd::SetErrorCodeForRoot(tx_sim::impl::CmdErrorCode ec, SimCmd* cur_cmd) {
  if (cur_cmd == nullptr) return;
  while (cur_cmd->parent_ != nullptr) { cur_cmd = cur_cmd->parent_; }
  RootCmd* root_cmd = dynamic_cast<RootCmd*>(cur_cmd);
  if (root_cmd) { root_cmd->set_error_code(ec); }
}

//!
//! @brief 函数名：RetrieveToplevelCmdOpt
//! @details 函数功能：检索顶级命令选项
//!
//! @param[in] cmd_name 命令名称
//! @param[in] opt_name 选项名称
//! @param[in] cmd 指向 CLI::App 的指针
//! @return 指向 CLI::Option 的指针
//!
CLI::Option* SimCmd::RetrieveToplevelCmdOpt(const std::string& cmd_name, const std::string& opt_name, CLI::App* cmd) {
  while (cmd->get_parent()) {
    cmd = cmd->get_parent();
    if (cmd->get_name() == cmd_name) { return cmd->get_option(opt_name); }
  }
  return nullptr;
}

//!
//! @brief 函数名：CreateSimClient
//! @details 函数功能：创建 ISimClient 实例
//!
//! @return 指向 ISimClient 的智能指针
//!
std::shared_ptr<ISimClient> SimCmd::CreateSimClient() {
  CLI::Option* root_opt = RetrieveToplevelCmdOpt(kRootCmdName, kEndpointOptAlias, cmd_.get());
  std::string endpoint = root_opt->as<std::string>();
  return sim_cli_factory_->CreateSimClient(endpoint);
}

//!
//! @brief 构造函数：RootCmd
//! @details 构造 RootCmd 类的实例
//!
//! @param[in] parent 指向父类 SimCmd 的指针
//! @param[in] factory 指向 ISimClientFactory 的智能指针
//!
RootCmd::RootCmd(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory) : SimCmd(parent, factory) {
  cmd_ = std::make_shared<CLI::App>(kRootCmdDesc, kRootCmdName);
  cmd_->require_subcommand();
  cmd_->set_help_flag(JoinOptName(kHelpOptAlias, kHelpOptName), kRootCmdHelpOptDesc);

  auto opt = std::make_shared<Opt>();
  opt->endpoint = tx_sim::utils::GetLocalServiceBindAddress();

  cmd_->callback([opt]() {});

  CLI::Option* option =
      cmd_->add_option(JoinOptName(kEndpointOptAlias, kEndpointOptName), opt->endpoint, kEndpointOptDesc);
  option->capture_default_str();

  sub_cmds_.emplace_back(new ConfCmd(this, sim_cli_factory_));
}

//!
//! @brief 函数名：ExecuteCmd
//! @details 函数功能：执行命令
//!
//! @param[in] argc 命令行参数的个数
//! @param[in] argv 指向命令行参数的指针数组
//!
void RootCmd::ExecuteCmd(int argc, const char* const* argv) {
  try {
    cmd_->parse(argc, argv);
  } catch (const CLI::ParseError& e) { set_error_code(CLI::ExitCodes(cmd_->exit(e))); }
}

//!
//! @brief 函数名：set_error_code
//! @details 函数功能：设置错误代码
//!
//! @param[in] error_code 错误代码
//!
void RootCmd::set_error_code(CLI::ExitCodes error_code) {
  error_code_ = ExitCode2CLIErrorCode(error_code);
}

}  // namespace cli
}  // namespace tx_sim
