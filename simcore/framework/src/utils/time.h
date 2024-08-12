#pragma once

#include <stdint.h>

#include <chrono>
#include <ctime>
#include <functional>
#include <string>
#include <thread>

namespace tx_sim {
namespace utils {

// avoid confict with the GetCurrentTime macro define in WinBase.h
template <typename T, typename C>
int64_t txSimGetCurrentTime() {
  return std::chrono::duration_cast<T>(C::now().time_since_epoch()).count();
}

inline int64_t GetWallTimeInMillis() {
  return txSimGetCurrentTime<std::chrono::milliseconds, std::chrono::system_clock>();
}

inline int64_t GetCpuTimeInMillis() {
  return txSimGetCurrentTime<std::chrono::milliseconds, std::chrono::steady_clock>();
}

inline int64_t GetCpuTimeInMicros() {
  return txSimGetCurrentTime<std::chrono::microseconds, std::chrono::steady_clock>();
}

inline void SleepForMillis(uint32_t millis) { std::this_thread::sleep_for(std::chrono::milliseconds(millis)); }

// this is a high-resolution sleep control implementation(specially for Windows).
void SleepUntilMillis(uint64_t millis);

static const std::string kDateTimeLogStrFormat("%Y-%m-%d %H:%M:%S");

std::string GetDateTimeStr(std::time_t t, const std::string& format);
std::string GetCurrentDateTimeStr();

// in micro-seconds.
std::pair<uint64_t, uint64_t> GetProcessCpuTime();

}  // namespace utils
}  // namespace tx_sim
