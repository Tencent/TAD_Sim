#pragma once

#include <string>
#include <vector>


namespace tx_sim {
namespace utils {

const std::string GetHomeDir();
const std::string GetFileDirectory(const std::string& file_abs_path);

const std::string GetModuleRegistryBindAddress(uint16_t port_offset);
const std::string GetLocalServiceBindAddress();

#ifdef _WIN32
const std::string GetLastWinErrorString();
#endif  // _WIN32

void ConfigureSignalHandling(void (*handler)(int));
void BlockSignals();
void UnBlockSignals();

const std::string GetEnvVar(const std::string& env_key);
void SetEnvVars(const std::vector<std::pair<std::string, std::string>> &envs);
void RemoveEnvVars(const std::vector<std::pair<std::string, std::string>> &envs);

void SetThreadName(const std::string& name);

}  // namespace utils
}  // namespace tx_sim
