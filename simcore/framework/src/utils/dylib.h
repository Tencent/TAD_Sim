// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>

namespace tx_sim {
namespace utils {

void* LoadSharedLibrary(const std::string& lib_path, bool deep_bind = false);
void UnLoadLibrary(void* lib);
void* GetLibraryFunc(const std::string& func_name, void* lib);

}  // namespace utils
}  // namespace tx_sim
