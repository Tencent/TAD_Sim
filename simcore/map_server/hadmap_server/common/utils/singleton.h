// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <assert.h>

#include <iostream>
#include <mutex>

namespace utils {

namespace detail {
template <typename T>
class CreateUsingNew {
 public:
  static T* Create() { return new T; }
};
}  // namespace detail

template <typename T, typename CreatePolicy = detail::CreateUsingNew<T>>
class Singleton {
 public:
  virtual ~Singleton() {}

  static T& Instance() {
    std::call_once(once_flag_, [&] { instance_ = CreatePolicy::Create(); });
    assert(instance_ != nullptr);
    return *instance_;
  }

 protected:
  Singleton() {}
  Singleton(const Singleton&) = delete;
  Singleton(const Singleton&&) = delete;
  Singleton& operator=(const Singleton&) = delete;
  Singleton& operator=(const Singleton&&) = delete;

 private:
  static void Initialize() { instance_ = new T(); }

 private:
  static T* instance_;
  static std::once_flag once_flag_;
};

template <typename T, typename CreatePolicy>
T* Singleton<T, CreatePolicy>::instance_ = nullptr;

template <typename T, typename CreatePolicy>
std::once_flag Singleton<T, CreatePolicy>::once_flag_;

}  // namespace utils
