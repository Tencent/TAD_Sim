#pragma once

#include <string>


namespace tx_sim {
namespace utils {

void* LoadSharedLibrary(const std::string& lib_path, bool deep_bind = false);
void UnLoadLibrary(void* lib);
void* GetLibraryFunc(const char* func_name, void* lib);

} // namespace utils
} // namespace tx_sim
