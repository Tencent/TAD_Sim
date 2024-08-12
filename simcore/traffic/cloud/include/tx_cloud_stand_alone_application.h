// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_application.h"

class CloudStandAlone_Application : public txApplication {
 public:
  CloudStandAlone_Application() = default;
  ~CloudStandAlone_Application() = default;

 public:
  /**
   * @brief 执行程序并返回结果
   * @param argc 命令行参数的数量
   * @param argv 命令行参数的字符串数组
   * @return 返回0表示成功，其他值表示失败
   */
  virtual int Run(int argc, char* argv[]) noexcept override;

  /**
   * @brief 打印参数值
   * @details 打印Application的参数值
   */
  virtual void PrintParamValues() noexcept override;
};
using CloudStandAlone_ApplicationPtr = std::shared_ptr<CloudStandAlone_Application>;
