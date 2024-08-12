// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "root_cmd.h"

namespace tx_sim {
namespace cli {

//!
//! @brief 类名：ConfCmd
//! @details 类功能：配置命令类
//!
class ConfCmd : public SimCmd {
 public:
  //!
  //! @brief 构造函数
  //! @details 构造配置命令对象
  //!
  //! @param[in] parent 父命令对象
  //! @param[in] factory 模拟客户端工厂对象
  //!
  ConfCmd(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory);
};

//!
//! @brief 类名：ConfShow
//! @details 类功能：显示配置命令类
//!
class ConfShow : public SimCmd {
 public:
  //!
  //! @brief 构造函数
  //! @details 构造显示配置命令对象
  //!
  //! @param[in] parent 父命令对象
  //! @param[in] factory 模拟客户端工厂对象
  //!
  ConfShow(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory);

 private:
  //!
  //! @brief 结构体名：Opt
  //! @details 结构体功能：存储显示配置命令选项
  //!
  struct Opt {
    //!
    //! @brief 成员变量：mod
    //! @details 成员变量功能：存储模块名称
    //!
    std::string mod;
  };
};

//!
//! @brief 类名：ConfRestore
//! @details 类功能：恢复配置命令类
//!
class ConfRestore : public SimCmd {
 public:
  //!
  //! @brief 构造函数
  //! @details 构造恢复配置命令对象
  //!
  //! @param[in] parent 父命令对象
  //! @param[in] factory 模拟客户端工厂对象
  //!
  ConfRestore(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory);
};

//!
//! @brief 类名：ConfRm
//! @details 类功能：删除配置命令类
//!
class ConfRm : public SimCmd {
 public:
  //!
  //! @brief 构造函数
  //! @details 构造删除配置命令对象
  //!
  //! @param[in] parent 父命令对象
  //! @param[in] factory 模拟客户端工厂对象
  //!
  ConfRm(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory);

 private:
  //!
  //! @brief 结构体名：Opt
  //! @details 结构体功能：存储删除配置命令选项
  //!
  struct Opt {
    //!
    //! @brief 成员变量：mod
    //! @details 成员变量功能：存储模块名称
    //!
    std::string mod;
  };
};

//!
//! @brief 类名：ICLIConfigJson
//! @details 类功能：配置 JSON 接口类
//!
class ICLIConfigJson {
 protected:
  //!
  //! @brief 结构体名：Opt
  //! @details 结构体功能：存储配置 JSON 选项
  //!
  struct Opt {
    //!
    //! @brief 成员变量：file_name
    //! @details 成员变量功能：存储文件名称
    //!
    std::string file_name;

    //!
    //! @brief 成员变量：config_json_str
    //! @details 成员变量功能：存储配置 JSON 字符串
    //!
    std::string config_json_str;
  };

  //!
  //! @brief 成员变量：op
  //! @details 成员变量功能：存储配置 JSON 选项对象
  //!
  std::shared_ptr<Opt> op;

  //!
  //! @brief 函数名：GetConfigJsonStrFromCLI
  //! @details 函数功能：从 CLI 获取配置 JSON 字符串
  //!
  //! @param[out] config_json_str 输出的配置 JSON 字符串
  //! @param[in] cmd CLI 命令对象
  //!
  void GetConfigJsonStrFromCLI(std::string& config_json_str, std::shared_ptr<CLI::App> cmd);
};

//!
//! @brief 类名：ConfUpd
//! @details 类功能：更新配置命令类
//!
class ConfUpd : public SimCmd, public ICLIConfigJson {
 public:
  //!
  //! @brief 构造函数
  //! @details 构造更新配置命令对象
  //!
  //! @param[in] parent 父命令对象
  //! @param[in] factory 模拟客户端工厂对象
  //!
  ConfUpd(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory);
};

//!
//! @brief 类名：ConfAdd
//! @details 类功能：添加配置命令类
//!
class ConfAdd : public SimCmd, public ICLIConfigJson {
 public:
  //!
  //! @brief 构造函数
  //! @details 构造添加配置命令对象
  //!
  //! @param[in] parent 父命令对象
  //! @param[in] factory 模拟客户端工厂对象
  //!
  ConfAdd(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory);
};

//!
//! @brief 类名：ConfSetSys
//! @details 类功能：继承自 SimCmd 和 ICLIConfigJson，用于设置系统配置
//!
class ConfSetSys : public SimCmd, public ICLIConfigJson {
 public:
  //!
  //! @brief 构造函数：ConfSetSys
  //! @details 构造 ConfSetSys 类的实例
  //!
  //! @param[in] parent 指向父类 SimCmd 的指针
  //! @param[in] factory 指向 ISimClientFactory 的智能指针
  //!
  ConfSetSys(SimCmd* parent, std::shared_ptr<ISimClientFactory> factory);
};

}  // namespace cli
}  // namespace tx_sim
