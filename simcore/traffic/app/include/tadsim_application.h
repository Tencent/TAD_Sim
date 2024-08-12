// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_application.h"

class TADSim_Application : public txApplication {
 public:
  TADSim_Application() = default;
  ~TADSim_Application() = default;

 public:
  /**
   * @brief TADSim_Application::Run 函数实现运行 TADSim
   * @param argc 命令行参数个数
   * @param argv 命令行参数数组
   * @return int 函数返回值
   */
  virtual int Run(int argc, char* argv[]) noexcept override;

  /**
   * @brief 打印 TADSim 参数值
   *
   * 本函数用于打印 TADSim 程序的运行参数值。
   *
   * @param none
   * @return none
   */
  virtual void PrintParamValues() noexcept override;
};
using TADSim_ApplicationPtr = std::shared_ptr<TADSim_Application>;
