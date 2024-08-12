// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "cli.hpp"
#include "constant.h"
#include "sim_client.h"

namespace tx_sim {
namespace cli {

//!
//! @brief 类名：SimCmd
//! @details 类功能：用于处理模拟命令
//!
class SimCmd {
 public:
  //!
  //! @brief 构造函数：SimCmd
  //! @details 构造 SimCmd 类的实例
  //!
  //! @param[in] parent 指向父类 SimCmd 的指针
  //! @param[in] factory 指向 ISimClientFactory 的智能指针
  //!
  SimCmd(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory);

  //!
  //! @brief 析构函数：~SimCmd
  //! @details 析构 SimCmd 类的实例
  //!
  virtual ~SimCmd();
  SimCmd(const SimCmd&) = delete;
  SimCmd& operator=(const SimCmd&) = delete;
  //! @brief 函数名：RetrieveToplevelCmdOpt
  //! @details 函数功能：检索顶级命令选项
  //!
  //! @param[in] cmd_name 命令名称
  //! @param[in] opt_name 选项名称
  //! @param[in] cmd 指向 CLI::App 的指针
  //! @return 指向 CLI::Option 的指针
  //!
  CLI::Option* RetrieveToplevelCmdOpt(const std::string& cmd_name, const std::string& opt_name, CLI::App* cmd);

  //!
  //! @brief 函数名：CreateSimClient
  //! @details 函数功能：创建 ISimClient 实例
  //!
  //! @return 指向 ISimClient 的智能指针
  //!
  std::shared_ptr<ISimClient> CreateSimClient();

  //!
  //! @brief 函数名：SetErrorCodeForRoot
  //! @details 函数功能：设置根命令的错误代码
  //!
  //! @param[in] ec 错误代码
  //! @param[in] cur_cmd 指向当前 SimCmd 的指针
  //!
  void SetErrorCodeForRoot(tx_sim::impl::CmdErrorCode ec, SimCmd* cur_cmd);

  //!
  //! @brief 函数名：cmd_ptr
  //! @details 函数功能：获取 CLI::App 的智能指针
  //!
  //! @return 指向 CLI::App 的智能指针
  //!
  std::shared_ptr<CLI::App> cmd_ptr() const { return cmd_; }

 protected:
  //! 指向父类 SimCmd 的指针
  SimCmd* parent_;

  //! 指向 CLI::App 的智能指针
  std::shared_ptr<CLI::App> cmd_;

  //! 指向 ISimClientFactory 的智能指针
  std::shared_ptr<ISimClientFactory> sim_cli_factory_;

  //! 存储子命令的向量
  std::vector<std::unique_ptr<SimCmd>> sub_cmds_;
};

//!
//! @brief 类名：RootCmd
//! @details 类功能：继承自 SimCmd，用于处理根命令
//!
class RootCmd : public SimCmd {
 public:
  //!
  //! @brief 构造函数：RootCmd
  //! @details 构造 RootCmd 类的实例
  //!
  //! @param[in] parent 指向父类 SimCmd 的指针
  //! @param[in] factory 指向 ISimClientFactory 的智能指针
  //!
  RootCmd(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory);

  //!
  //! @brief 函数名：root_ptr
  //! @details 函数功能：获取 CLI::App 的智能指针
  //!
  //! @return 指向 CLI::App 的智能指针
  //!
  std::shared_ptr<CLI::App> root_ptr() const { return cmd_; }

  //!
  //! @brief 函数名：error_code
  //! @details 函数功能：获取错误代码
  //!
  //! @return 错误代码
  //!
  CLIErrorCode error_code() const { return error_code_; }

  //!
  //! @brief 函数名：set_error_code
  //! @details 函数功能：设置错误代码
  //!
  //! @param[in] error_code 错误代码
  //!
  void set_error_code(tx_sim::impl::CmdErrorCode error_code) { error_code_ = CLIErrorCode(error_code); }

  //!
  //! @brief 函数名：set_error_code
  //! @details 函数功能：设置错误代码
  //!
  //! @param[in] error_code 错误代码
  //!
  void set_error_code(CLI::ExitCodes error_code);

  //!
  //! @brief 函数名：ExecuteCmd
  //! @details 函数功能：执行命令
  //!
  //! @param[in] argc 命令行参数的个数
  //! @param[in] argv 指向命令行参数的指针数组
  //!
  void ExecuteCmd(int argc, const char* const* argv);

 private:
  //! 错误代码
  CLIErrorCode error_code_;

  //! 存储选项的结构体
  struct Opt {
    std::string endpoint;
  };
};

}  // namespace cli
}  // namespace tx_sim
