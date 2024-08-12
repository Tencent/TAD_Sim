// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#define TXSIM_DISABLE_COPY_MOVE(cls)   \
  cls(const cls&) = delete;            \
  cls(cls&&) = delete;                 \
  cls& operator=(const cls&) = delete; \
  cls& operator=(cls&&) = delete;

#define TXSIM_DISABLE_COPY(cls) \
  cls(const cls&) = delete;     \
  cls& operator=(const cls&) = delete;

#if defined __GNUC__
#  define TXSIM_LIKELY(x)   __builtin_expect((x), 1)
#  define TXSIM_UNLIKELY(x) __builtin_expect((x), 0)
#else
#  define TXSIM_LIKELY(x)   (x)
#  define TXSIM_UNLIKELY(x) (x)
#endif

#ifdef __linux__
#  include <sys/types.h>
#  define txsim_pid_t         pid_t
#  define txsim_invalid_pid_t -1
#endif  // __linux__
#ifdef _WIN32
// #include <Windows.h>
#  define txsim_pid_t         void*  // HANDLE
#  define txsim_invalid_pid_t nullptr
#endif  // _WIN32

namespace tx_sim {
namespace impl {
typedef std::pair<std::string, std::string> StringPair;
}  // namespace impl
}  // namespace tx_sim
