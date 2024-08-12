// Copyright 2024 Tencent Inc. All rights reserved.
//
// copy and modified from: https://en.cppreference.com/w/cpp/atomic/atomic_flag
//

#pragma once
#include <atomic>
#include <iostream>

class SpinLock {
 private:
  std::atomic_flag atomic_flag = ATOMIC_FLAG_INIT;

 public:
  ~SpinLock() {}
  void lock() {
    for (;;) {
      if (!atomic_flag.test_and_set(std::memory_order_acquire)) {
        break;
      }

#if defined(__cpp_lib_atomic_flag_test)  // since c++20
      while (atomic_flag.test(std::memory_order_relaxed)) {
      }  // test lock
#endif
    }
  }
  void unlock() { atomic_flag.clear(std::memory_order_release); }
};
