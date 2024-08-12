// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <memory>
#include <mutex>

namespace eval {
// none-copyable
class NoneCopyable {
 public:
  NoneCopyable() {}
  NoneCopyable(const NoneCopyable &) = delete;
  NoneCopyable &operator=(const NoneCopyable &) = delete;
  virtual ~NoneCopyable() {}
};

// singleton
template <class T>
class CSingleton : public NoneCopyable {
 private:
  static std::mutex _lock;
  static std::shared_ptr<T> _inst_ptr;
  CSingleton() {}

 public:
  virtual ~CSingleton() {}

  template <class... Vars>
  static std::shared_ptr<T> GetInstance(Vars &&...vars) {
    std::lock_guard<std::mutex> guard(_lock);
    if (_inst_ptr.get() == nullptr) {
      T *ptr = new T(vars...);
      if (ptr) _inst_ptr.reset(ptr);
    }
    return _inst_ptr;
  }
};

// singleton static member
template <class T>
std::mutex CSingleton<T>::_lock;

template <class T>
std::shared_ptr<T> CSingleton<T>::_inst_ptr;
}  // namespace eval
