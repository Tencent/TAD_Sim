// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
class txApplication {
 public:
  txApplication();
  ~txApplication() = default;

 public:
  /**
   * @brief 初始化TxApp
   *
   * @param argc 命令行参数个数
   * @param argv 命令行参数指针数组
   */
  virtual void Initialize(int argc, char* argv[]) noexcept;

  /**
   * @brief 运行程序并根据参数执行相应操作
   *
   * 使用传入的命令行参数执行相应操作，如启动程序、配置程序等。
   *
   * @param argc 命令行参数个数
   * @param argv 命令行参数指针数组
   * @return 执行结果
   *
   */
  virtual int Run(int argc, char* argv[]) noexcept = 0;

  /**
   * @brief 终止函数
   *
   */
  virtual void Terminate() noexcept;

  /**
   * @brief 输出参数值
   *
   */
  virtual void PrintParamValues() noexcept = 0;

  /**
   * @brief 获取app绝对路径
   *
   * @return std::string
   */
  static std::string AppAbstractPath() noexcept { return std::string(s_app_path); }

 protected:
  using InitFunc = std::add_pointer<void(int, char*[])>::type;
  using ReleaseFunc = std::add_pointer<void()>::type;

  std::vector<InitFunc> ms_VecInitFunc;
  std::vector<ReleaseFunc> ms_VecReleaseFunc;
  bool ms_bInitialized = false;
  // static std::string s_app_path;
  static char* s_app_path;

 protected:
  /**
   * @brief 初始化日志
   *
   * @param argc 参数个数
   * @param argv 参数指针数组
   */
  static void InitLogger(int argc, char* argv[]) noexcept;

  /**
   * @brief 释放日志
   *
   */
  static void ReleaseLogger() noexcept;

  /**
   * @brief 初始化命令行参数
   *
   * @param argc 参数个数
   * @param argv 参数指针数组
   */
  static void InitCommandLineArguments(int argc, char* argv[]) noexcept;

  /**
   * @brief 释放命令行参数
   *
   */
  static void ReleaseCommandLineArguments() noexcept;
};
using txApplicationPtr = std::shared_ptr<txApplication>;
