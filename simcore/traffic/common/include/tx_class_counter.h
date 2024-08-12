// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <atomic>
#include "tx_header.h"
TX_NAMESPACE_OPEN(Utils)
#if 0
template <typename T>
class Counter {
 public:
  static Base::txSize howMany() { return 999; }
};
#else
template <typename T>
class Counter {
 public:
  Counter() { ++count; }

  /**
   * @brief 每次构造，计数器加1
   *
   */
  Counter(const Counter&) { ++count; }
  ~Counter() { --count; }

  static Base::txSize howMany() { return count.load(); }

 private:
  static std::atomic<Base::txSize> count;
};

template <typename T>
std::atomic<Base::txSize> Counter<T>::count = {0};  // this may now go in a header file
#endif
TX_NAMESPACE_CLOSE(Utils)
