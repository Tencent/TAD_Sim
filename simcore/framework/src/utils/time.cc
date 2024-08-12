#include "time.h"

#ifdef __linux__
#  include <sys/resource.h>
#  include <sys/time.h>
#  include <unistd.h>
#elif defined _WIN32
#  include <Windows.h>
#  pragma comment(lib, "winmm")
#endif  // __linux__
#include <iomanip>
#include <sstream>


namespace tx_sim {
namespace utils {

void SleepUntilMillis(uint64_t millis) {
#ifdef _WIN32
  timeBeginPeriod(1);
#endif  // _WIN32
  std::this_thread::sleep_until(std::chrono::time_point<std::chrono::steady_clock>(std::chrono::milliseconds(millis)));
#ifdef _WIN32
  timeEndPeriod(1);
#endif  // _WIN32
}

std::string GetDateTimeStr(std::time_t t, const std::string& format) {
  std::tm tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, format.c_str());
  return oss.str();
}


std::string GetCurrentDateTimeStr() {
  return GetDateTimeStr(std::time(nullptr), "%Y%m%d%H%M%S");
}


#ifdef _WIN32
std::pair<uint64_t, uint64_t> GetProcessCpuTime() {
  static std::pair<uint64_t, uint64_t> empty_pair(0, 0);
  FILETIME creation_time, exit_time, kernel_time, user_time;
  if (GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time) != 0) {
    return std::make_pair<uint64_t, uint64_t>(
        (user_time.dwLowDateTime | ((unsigned long long)user_time.dwHighDateTime << 32)) / 10,
        (kernel_time.dwLowDateTime | ((unsigned long long)kernel_time.dwHighDateTime << 32)) / 10);
  } else {
    return empty_pair;
  }
}
#elif defined __linux__
std::pair<uint64_t, uint64_t> GetProcessCpuTime() {
  static std::pair<uint64_t, uint64_t> empty_pair(0, 0);
  // static int clocks_per_microsec = CLOCKS_PER_SEC / 1000000;
  // clock_t t = std::clock();
  // return t < 0 ? 0 : (t / clocks_per_microsec);
  struct rusage r;
  int ret = getrusage(RUSAGE_SELF, &r);
  return ret == 0 ? std::make_pair<uint64_t, uint64_t>(r.ru_utime.tv_sec * 1000000 + r.ru_utime.tv_usec,
                                                       r.ru_stime.tv_sec * 1000000 + r.ru_stime.tv_usec)
                  : empty_pair;
}
#endif  // _WIN32

}  // namespace utils
}  // namespace tx_sim