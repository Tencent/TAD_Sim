// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_sys_info.h"
#if __tx_windows__
#  include "TCHAR.h"
#  include "pdh.h"
#  include "psapi.h"
#  include "windows.h"
TX_NAMESPACE_OPEN(Utils)
TX_NAMESPACE_OPEN(Sys)

void ShowMemoryInfo() TX_NOEXCEPT {
  if (FLAGS_EnableSysInfo) {
    std::ostringstream oss;
    oss << "\n################  System Informaion Start ####################" << std::endl;
#  if __TX_Mark__("Total Virtual Memory")
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    const DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
    oss << TX_VARS_NAME("Total Virtual Memory", totalVirtualMem) << std::endl;

    const DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
    oss << TX_VARS_NAME("Virtual Memory currently used", virtualMemUsed) << std::endl;

    const DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    oss << TX_VARS_NAME("Total Physical Memory (RAM)", totalPhysMem) << std::endl;

    const DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    oss << TX_VARS_NAME("Total Physical Memory (RAM) currently used", physMemUsed) << std::endl;
#  endif /*Total Virtual Memory*/

#  if __TX_Mark__("Total Virtual Memory current process")
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc));
    const SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
    oss << TX_VARS_NAME("Virtual Memory currently used by current process", virtualMemUsedByMe) << std::endl;

    const SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
    oss << TX_VARS_NAME("Physical Memory currently used by current process", physMemUsedByMe) << std::endl;
#  endif /*Total Virtual Memory current process*/
    oss << "################  System Informaion End    ####################" << std::endl;
    LOG(WARNING) << oss.str();
  }
}

TX_NAMESPACE_CLOSE(Sys)
TX_NAMESPACE_CLOSE(Utils)
#else /*__tx_windows__*/
#  include "stdio.h"
#  include "stdlib.h"
#  include "string.h"
#  include "sys/sysinfo.h"
#  include "sys/types.h"
TX_NAMESPACE_OPEN(Utils)
TX_NAMESPACE_OPEN(Sys)

int parseLine(char* line) {
  // This assumes that a digit will be found and the line ends in " Kb".
  int i = strlen(line);
  const char* p = line;
  // 跳过非数字字符
  while (*p < '0' || *p > '9') p++;
  // 将行末尾的" Kb"替换为'\0'，以便在转换为整数时不包含这部分字符串
  line[i - 3] = '\0';
  i = atoi(p);
  return i;
}

int getVirtualMemoryUsedByCurrentProcess(
    /*Virtual Memory currently used by current process*/) {  // Note: this value is in KB!
  // 打开文件"/proc/self/status"
  FILE* file = fopen("/proc/self/status", "r");
  // 初始化结果为-1
  int result = -1;
  char line[128];

  // 逐行读取文件
  while (fgets(line, 128, file) != NULL) {
    if (strncmp(line, "VmSize:", 7) == 0) {
      // 解析该行并将结果存储在result中
      result = parseLine(line);
      break;
    }
  }
  fclose(file);
  return result;
}

// 打开文件"/proc/self/status"
int getPhysicalMemoryUsedByCurrentProcess(
    /*Physical Memory currently used by current process*/) {  // Note: this value is in KB!
  // 打开文件"/proc/self/status"
  FILE* file = fopen("/proc/self/status", "r");
  int result = -1;
  char line[128];

  // 逐行读取文件
  while (fgets(line, 128, file) != NULL) {
    if (strncmp(line, "VmRSS:", 6) == 0) {
      // 解析该行并将结果存储在result中
      result = parseLine(line);
      break;
    }
  }
  fclose(file);
  return result;
}

void ShowMemoryInfo() TX_NOEXCEPT {
  if (FLAGS_EnableSysInfo) {
    std::ostringstream oss;
    oss << "\n################  System Informaion Start ####################" << std::endl;
#  if __TX_Mark__("Total Virtual Memory")
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    int64_t totalVirtualMem = memInfo.totalram;
    // Add other values in next statement to avoid int overflow on right hand side...
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;
    oss << TX_VARS_NAME("Total Virtual Memory", totalVirtualMem) << std::endl;

    int64_t virtualMemUsed = memInfo.totalram - memInfo.freeram;
    // Add other values in next statement to avoid int overflow on right hand side...
    virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
    virtualMemUsed *= memInfo.mem_unit;
    oss << TX_VARS_NAME("Virtual Memory currently used", virtualMemUsed) << std::endl;

    int64_t totalPhysMem = memInfo.totalram;
    // Multiply in next statement to avoid int overflow on right hand side...
    totalPhysMem *= memInfo.mem_unit;
    oss << TX_VARS_NAME("Total Physical Memory (RAM)", totalPhysMem) << std::endl;

    int64_t physMemUsed = memInfo.totalram - memInfo.freeram;
    // Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= memInfo.mem_unit;
    oss << TX_VARS_NAME("Total Physical Memory (RAM) currently used", physMemUsed) << std::endl;
#  endif /*Total Virtual Memory*/

#  if __TX_Mark__("Total Virtual Memory current process")

    oss << TX_VARS_NAME("Virtual Memory currently used by current process", getVirtualMemoryUsedByCurrentProcess())
        << std::endl;

    oss << TX_VARS_NAME("Physical Memory currently used by current process", getPhysicalMemoryUsedByCurrentProcess())
        << std::endl;
#  endif /*Total Virtual Memory current process*/
    oss << "################  System Informaion End    ####################" << std::endl;
    LOG(WARNING) << oss.str();
  }
}

TX_NAMESPACE_CLOSE(Sys)
TX_NAMESPACE_CLOSE(Utils)
#endif   /*__tx_windows__*/
