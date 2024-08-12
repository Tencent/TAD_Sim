// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>

namespace tx_sim {
namespace cli {
//!
//! @brief 枚举名称：CLIErrorCode
//! @details 枚举功能：定义命令行接口错误代码
//!
enum CLIErrorCode {
  //!
  //! @brief 常量名称：kCmdAccepted
  //! @details 常量功能：枚举变量,命令接受
  //!
  kCmdAccepted = 0,
  //!
  //! @brief 常量名称：kCmdRejected
  //! @details 常量功能：枚举变量,命令拒绝
  //!
  kCmdRejected = 10,
  //!
  //! @brief 常量名称：kCmdServerBusy
  //! @details 常量功能：枚举变量,服务器繁忙
  //!
  kCmdServerBusy = 20,
  //!
  //! @brief 常量名称：kCmdSucceed
  //! @details 常量功能：枚举变量,命令成功
  //!
  kCmdSucceed = 100,
  //!
  //! @brief 常量名称：kCmdScenarioTimeout
  //! @details 常量功能：枚举变量,场景超时
  //!
  kCmdScenarioTimeout = 500,
  //!
  //! @brief 常量名称：kCmdScenarioStopped
  //! @details 常量功能：枚举变量,场景停止
  //!
  kCmdScenarioStopped = 900,
  //!
  //! @brief 常量名称：kCmdInvalidTopicPubSub
  //! @details 常量功能：枚举变量,无效的主题发布订阅
  //!
  kCmdInvalidTopicPubSub = 1500,
  //!
  //! @brief 常量名称：kCmdFailed
  //! @details 常量功能：枚举变量,命令失败
  //!
  kCmdFailed = 3000,
  //!
  //! @brief 常量名称：kCmdScenarioParsingError
  //! @details 常量功能：枚举变量,场景解析错误
  //!
  kCmdScenarioParsingError = 20000,
  //!
  //! @brief 常量名称：kCmdShmemCreationError
  //! @details 常量功能：枚举变量,共享内存创建错误
  //!
  kCmdShmemCreationError = 29000,
  //!
  //! @brief 常量名称：kCmdSystemError
  //! @details 常量功能：枚举变量,系统错误
  //!
  kCmdSystemError = 30000,
  //!
  //! @brief 常量名称：kCLIInCorrectConstruction
  //! @details 常量功能：枚举变量,命令行参数错误
  //!
  kCLIInCorrectConstruction = 40000,
  //!
  //! @brief 常量名称：kCLIBadNameString
  //! @details 常量功能：枚举变量,命令行参数名称错误
  //!
  kCLIBadNameString,
  //!
  //! @brief 常量名称：kCLIOptionAlreadyAdded
  //! @details 常量功能：枚举变量,命令行参数已添加
  //!
  kCLIOptionAlreadyAdded,
  //!
  //! @brief 常量名称：kCLIFileError
  //! @details 常量功能：枚举变量,命令行参数文件错误
  //!
  kCLIFileError,
  //!
  //! @brief 常量名称：kCLIConversionError
  //! @details 常量功能：枚举变量,命令行参数转换错误
  //!
  kCLIConversionError,
  //!
  //! @brief 常量名称：kCLIValidationError
  //! @details 常量功能：枚举变量,命令行参数验证错误
  //!
  kCLIValidationError,
  //!
  //! @brief 常量名称：kCLIRequiredError
  //! @details 常量功能：枚举变量,命令行参数缺失
  //!
  kCLIRequiredError,
  //!
  //! @brief 常量名称：kCLIRequiresError
  //! @details 常量功能：枚举变量,命令行参数依赖错误
  //!
  kCLIRequiresError,
  //!
  //! @brief 常量名称：kCLIExcludesError
  //! @details 常量功能：枚举变量,命令行参数排除错误
  //!
  kCLIExcludesError,
  //!
  //! @brief 常量名称：kCLIExtrasError
  //! @details 常量功能：枚举变量,命令行参数额外错误
  //!
  kCLIExtrasError,
  //!
  //! @brief 常量名称：kCLIConfigError
  //! @details 常量功能：枚举变量,命令行参数配置错误
  //!
  kCLIConfigError,
  //!
  //! @brief 常量名称：kCLIInvalidError
  //! @details 常量功能：枚举变量,命令行参数无效错误
  //!
  kCLIInvalidError,
  //!
  //! @brief 常量名称：kCLIHorribleError
  //! @details 常量功能：枚举变量,命令行参数严重错误
  //!
  kCLIHorribleError,
  //!
  //! @brief 常量名称：kCLIOptionNotFound
  //! @details 常量功能：枚举变量,命令行参数未找到
  //!
  kCLIOptionNotFound,
  //!
  //! @brief 常量名称：kCLIArgumentMismatch
  //! @details 常量功能：枚举变量,命令行参数参数不匹配
  //!
  kCLIArgumentMismatch,
  //!
  //! @brief 常量名称：kCLIUnknownError
  //! @details 常量功能：枚举变量,命令行参数未知错误
  //!
  kCLIUnknownError,
};

//!
//! @brief 函数名：ExitCode2CLIErrorCode
//! @details 函数功能：将 CLI::ExitCodes 转换为 CLIErrorCode
//!
//! @param[in] ec CLI::ExitCodes 类型的错误代码
//! @return CLIErrorCode 类型的错误代码
//!
inline CLIErrorCode ExitCode2CLIErrorCode(CLI::ExitCodes ec) {
  switch (ec) {
    //!
    //! @brief 条件类型：构造不合理
    //! @details 返回：返回客户端构造不合理
    //!
    case CLI::ExitCodes::IncorrectConstruction: return kCLIInCorrectConstruction;
    //!
    //! @brief 条件类型：名称字符串错误
    //! @details 返回：返回客户端名称字符串错误
    //!
    case CLI::ExitCodes::BadNameString: return kCLIBadNameString;
    //!
    //! @brief 条件类型：选项已添加
    //! @details 返回：返回客户端选项已添加
    //!
    case CLI::ExitCodes::OptionAlreadyAdded: return kCLIOptionAlreadyAdded;
    //!
    //! @brief 条件类型：文件错误
    //! @details 返回：返回客户端文件错误
    //!
    case CLI::ExitCodes::FileError: return kCLIFileError;
    //!
    //! @brief 条件类型：转换错误
    //! @details 返回：返回客户端转换错误
    //!
    case CLI::ExitCodes::ConversionError: return kCLIConversionError;
    //!
    //! @brief 条件类型：验证错误
    //! @details 返回：返回客户端验证错误
    //!
    case CLI::ExitCodes::ValidationError: return kCLIValidationError;
    //!
    //! @brief 条件类型：缺少必需的错误
    //! @details 返回：返回客户端缺少必需的错误
    //!
    case CLI::ExitCodes::RequiredError: return kCLIRequiredError;
    //!
    //! @brief 条件类型：依赖错误
    //! @details 返回：返回客户端依赖错误
    //!
    case CLI::ExitCodes::RequiresError: return kCLIRequiresError;
    //!
    //! @brief 条件类型：排除错误
    //! @details 返回：返回客户端排除错误
    //!
    case CLI::ExitCodes::ExcludesError: return kCLIExcludesError;
    //!
    //! @brief 条件类型：额外错误
    //! @details 返回：返回客户端额外错误
    //!
    case CLI::ExitCodes::ExtrasError: return kCLIExtrasError;
    //!
    //! @brief 条件类型：配置错误
    //! @details 返回：返回客户端配置错误
    //!
    case CLI::ExitCodes::ConfigError: return kCLIConfigError;
    //!
    //! @brief 条件类型：无效错误
    //! @details 返回：返回客户端无效错误
    //!
    case CLI::ExitCodes::InvalidError: return kCLIInvalidError;
    //!
    //! @brief 条件类型：严重错误
    //! @details 返回：返回客户端严重错误
    //!
    case CLI::ExitCodes::HorribleError: return kCLIHorribleError;
    //!
    //! @brief 条件类型：选项未找到
    //! @details 返回：返回客户端选项未找到
    //!
    case CLI::ExitCodes::OptionNotFound: return kCLIOptionNotFound;
    //!
    //! @brief 条件类型：参数不匹配
    //! @details 返回：返回客户端参数不匹配
    //!
    case CLI::ExitCodes::ArgumentMismatch: return kCLIArgumentMismatch;
    //!
    //! @brief 条件类型：未知错误
    //! @details 返回：返回客户端未知错误
    //!
    default: return kCLIUnknownError;
  }
}

// Command Names
//!
//! @brief 常量名称：kRootCmdName
//! @details 常量功能：定义根命令名称
//!
const std::string kRootCmdName = "txsim";

//!
//! @brief 常量名称：kConfCmdName
//! @details 常量功能：定义配置命令名称
//!
const std::string kConfCmdName = "conf";

//!
//! @brief 常量名称：kConfShowName
//! @details 常量功能：定义配置显示命令名称
//!
const std::string kConfShowName = "show";

//!
//! @brief 常量名称：kConfRestoreName
//! @details 常量功能：定义配置恢复命令名称
//!
const std::string kConfRestoreName = "restore";

//!
//! @brief 常量名称：kConfRmName
//! @details 常量功能：定义配置删除命令名称
//!
const std::string kConfRmName = "rm";

//!
//! @brief 常量名称：kConfUpdName
//! @details 常量功能：定义配置更新命令名称
//!
const std::string kConfUpdName = "upd";

//!
//! @brief 常量名称：kConfAddName
//! @details 常量功能：定义配置添加命令名称
//!
const std::string kConfAddName = "add";

//!
//! @brief 常量名称：kConfSetSysName
//! @details 常量功能：定义配置系统设置命令名称
//!
const std::string kConfSetSysName = "set-sys";

// Command descriptions
//!
//! @brief 常量名称：kRootCmdDesc
//! @details 常量功能：定义根命令描述
//!
const std::string kRootCmdDesc = "Tadsim, a command line tool to interact with server";

//!
//! @brief 常量名称：kConfCmdDesc
//! @details 常量功能：定义配置命令描述
//!
const std::string kConfCmdDesc = "Get, set or change configurations of system or target module";

//!
//! @brief 常量名称：kConfShowDesc
//! @details 常量功能：定义配置显示命令描述
//!
const std::string kConfShowDesc = "Show configuration of system or target module";

//!
//! @brief 常量名称：kConfRestoreDesc
//! @details 常量功能：定义配置恢复命令描述
//!
const std::string kConfRestoreDesc = "Restore configuration to default";

//!
//! @brief 常量名称：kConfRmDesc
//! @details 常量功能：定义配置删除命令描述
//!
const std::string kConfRmDesc = "Remove the configuration of target module";

//!
//! @brief 常量名称：kConfUpdDesc
//! @details 常量功能：定义配置更新命令描述
//!
const std::string kConfUpdDesc = "Update configuration for an existing module";

//!
//! @brief 常量名称：kConfAddDesc
//! @details 常量功能：定义配置添加命令描述
//!
const std::string kConfAddDesc = "Add configuration for a currently not existed module";

//!
//! @brief 常量名称：kConfSetSysDesc
//! @details 常量功能：定义配置系统设置命令描述
//!
const std::string kConfSetSysDesc = "Set configuration for system";

// Command flags and options
//!
//! @brief 常量名称：kHelpOptAlias
//! @details 常量功能：定义帮助选项别名
//!
const std::string kHelpOptAlias = "-h";

//!
//! @brief 常量名称：kHelpOptName
//! @details 常量功能：定义帮助选项名称
//!
const std::string kHelpOptName = "--help";

//!
//! @brief 常量名称：kEndpointOptAlias
//! @details 常量功能：定义端点选项别名
//!
const std::string kEndpointOptAlias = "-p";

//!
//! @brief 常量名称：kEndpointOptName
//! @details 常量功能：定义端点选项名称
//!
const std::string kEndpointOptName = "--endpoint";

//!
//! @brief 常量名称：kModuleOptAlias
//! @details 常量功能：定义模块选项别名
//!
const std::string kModuleOptAlias = "-m";

//!
//! @brief 常量名称：kModuleOptName
//! @details 常量功能：定义模块选项名称
//!
const std::string kModuleOptName = "--mod";

//!
//! @brief 常量名称：kPathOptAlias
//! @details 常量功能：定义路径选项别名
//!
const std::string kPathOptAlias = "-p";

//!
//! @brief 常量名称：kPathOptName
//! @details 常量功能：定义路径选项名称
//!
const std::string kPathOptName = "--path";

//!
//! @brief 常量名称：kFileOptAlias
//! @details 常量功能：定义文件选项别名
//!
const std::string kFileOptAlias = "-f";

//!
//! @brief 常量名称：kFileOptName
//! @details 常量功能：定义文件选项名称
//!
const std::string kFileOptName = "--file";

//!
//! @brief 常量名称：kConfigOptAlias
//! @details 常量功能：定义配置选项别名
//!
const std::string kConfigOptAlias = "-c";

//!
//! @brief 常量名称：kConfigOptName
//! @details 常量功能：定义配置选项名称
//!
const std::string kConfigOptName = "--config";

// Options and flags descriptions
//!
//! @brief 常量名称：kEndpointOptDesc
//! @details 常量功能：定义端点选项描述
//!
const std::string kEndpointOptDesc = "The endpoint of server";

//!
//! @brief 常量名称：kRootCmdHelpOptDesc
//! @details 常量功能：定义根命令帮助选项描述
//!
const std::string kRootCmdHelpOptDesc = "Print option help";

//!
//! @brief 常量名称：kConfCmdHelpOptDesc
//! @details 常量功能：定义配置命令帮助选项描述
//!
const std::string kConfCmdHelpOptDesc = "Print conf options";

//!
//! @brief 常量名称：kConfShowModOptDesc
//! @details 常量功能：定义配置显示模块选项描述
//!
const std::string kConfShowModOptDesc = "Specified cofiguration wanted, can be [sys], [MODULE NAME], [all]";

//!
//! @brief 常量名称：kConfRmModOptDesc
//! @details 常量功能：定义配置删除模块选项描述
//!
const std::string kConfRmModOptDesc = "[MODULE] for which configuration to be removed";

//!
//! @brief 常量名称：kConfUpdModOptDesc
//! @details 常量功能：定义配置更新模块选项描述
//!
const std::string kConfUpdModOptDesc = "[MODULE] for which configuration to be updated";

//!
//! @brief 常量名称：kConfUpdFileOptDesc
//! @details 常量功能：定义配置更新文件选项描述
//!
const std::string kConfUpdFileOptDesc = "Configuration file for the updated module";

//!
//! @brief 常量名称：kConfUpdConfigOptDesc
//! @details 常量功能：定义配置更新配置选项描述
//!
const std::string kConfUpdConfigOptDesc = "The configuration used for update, should be a json string";

//!
//! @brief 常量名称：kConfAddModOptDesc
//! @details 常量功能：定义配置添加模块选项描述
//!
const std::string kConfAddModOptDesc = "The configuration used for adding, should be a json string";

//!
//! @brief 常量名称：kConfAddFileOptDesc
//! @details 常量功能：定义配置添加文件选项描述
//!
const std::string kConfAddFileOptDesc = "Configuration file for newly added module";

//!
//! @brief 常量名称：kConfAddConfigOptDesc
//! @details 常量功能：定义配置添加配置选项描述
//!
const std::string kConfAddConfigOptDesc = "The configuration to be added, should be a json string";

//!
//! @brief 常量名称：kConfSetSysFileOptDesc
//! @details 常量功能：定义配置系统设置文件选项描述
//!
const std::string kConfSetSysFileOptDesc = "Configuration file for system";

//!
//! @brief 常量名称：kConfSetSysConfigOptDesc
//! @details 常量功能：定义配置系统设置配置选项描述
//!
const std::string kConfSetSysConfigOptDesc = "The configuration for system, should be a json string";

inline const std::string JoinOptName(const std::string& alias, const std::string& name) {
  return alias + "," + name;
}

}  // namespace cli
}  // namespace tx_sim
