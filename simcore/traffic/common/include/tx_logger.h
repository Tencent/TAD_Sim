// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <glog/logging.h>
#include "tx_marco.h"

TX_NAMESPACE_OPEN(Base)

class Logger {
 public:
  /**
   * @brief 初始化函数，用于设置日志记录配置选项
   *
   * @param argc 命令行参数数量
   * @param argv 命令行参数数组，每个参数对应一个选项及其参数（可选）
   *
   * @return 无返回值
   */
  static void Initialize(int argc, char* argv[]);

  /**
   * @brief 释放日志记录器
   *
   * 当不再需要使用日志记录器时，调用此函数释放相关资源。
   *
   * @return 无返回值
   */
  static void Release();
};

TX_NAMESPACE_CLOSE(Base)
