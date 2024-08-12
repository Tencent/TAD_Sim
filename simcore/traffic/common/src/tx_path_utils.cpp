// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_path_utils.h"
#if __tx_windows__
#  include <windows.h>
#endif /*__tx_windows__*/
#if __tx_linux__
#  include <boost/dll.hpp>
#endif /*__tx_linux__*/

TX_NAMESPACE_OPEN(Utils)

Base::txBool GetAppPath(Base::txString str_argv, const char* refAppPath) TX_NOEXCEPT {
#if __tx_windows__
  char ownPth[MAX_PATH];
  // When NULL is passed to GetModuleHandle, the handle of the exe itself is returned
  HMODULE hModule = GetModuleHandle(NULL);
  if (hModule != NULL) {
    // Use GetModuleFileName() with module handle to get the path
    GetModuleFileName(hModule, ownPth, (sizeof(ownPth)));
    using namespace boost::filesystem;
    refAppPath = FilePath(ownPth).parent_path().string().c_str();
    // LOG(INFO) << "Get App Path Success. " << refAppPath;
    return true;
  } else {
    LOG(WARNING) << "Get App Path Failure. " << str_argv;
    return false;
  }
#endif /*__tx_windows__*/
#if __tx_linux__
  refAppPath = boost::dll::program_location().parent_path().string().c_str();
#endif /*__tx_linux__*/
  return true;
}

Base::txBool GetAppPath(Base::txString str_argv, Base::txString& refAppPath) TX_NOEXCEPT {
#if __tx_windows__
  char ownPth[MAX_PATH];
  // When NULL is passed to GetModuleHandle, the handle of the exe itself is returned
  HMODULE hModule = GetModuleHandle(NULL);
  if (hModule != NULL) {
    // Use GetModuleFileName() with module handle to get the path
    GetModuleFileName(hModule, ownPth, (sizeof(ownPth)));
    using namespace boost::filesystem;
    refAppPath = FilePath(ownPth).parent_path().string();
    // LOG(INFO) << "Get App Path Success. " << refAppPath;
    return true;
  } else {
    LOG(WARNING) << "Get App Path Failure. " << str_argv;
    return false;
  }
#endif /*__tx_windows__*/
#if __tx_linux__
  refAppPath = boost::dll::program_location().parent_path().string();
#endif /*__tx_linux__*/
  return true;
}

Base::txBool CreateDirectory(Base::txString const& strDirPath) TX_NOEXCEPT {
  try {
    using namespace boost::filesystem;
    FilePath p(strDirPath);
    if (exists(p)) {
      return true;
    } else {
      create_directory(p);
      LOG(INFO) << "Create Directory Success. " << strDirPath;
      return true;
    }
  } catch (const boost::filesystem::filesystem_error& e) {
    if (boost::system::errc::permission_denied == e.code()) {
      LOG(WARNING) << "Permission is denied for creating directory : " << strDirPath;
    } else {
      LOG(WARNING) << "Create Directory \"" << strDirPath << "\" Failed With : " << e.code().message();
    }
  }
  return false;
}

Base::txBool GetFileExtension(Base::txString const& strFilePath, Base::txString& refStrExternsion) TX_NOEXCEPT {
  refStrExternsion = Base::txString("");
  try {
    using namespace boost::filesystem;
    FilePath p(strFilePath);
    if (exists(p)) {
      if (p.has_extension()) {
        refStrExternsion = p.extension().string();
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  } catch (const boost::filesystem::filesystem_error& e) {
    if (boost::system::errc::permission_denied == e.code()) {
      LOG(WARNING) << "Permission is denied for reading file : " << strFilePath;
    } else {
      LOG(WARNING) << "Reading file \"" << strFilePath << "\" Failed With : " << e.code().message();
    }
  }
  return false;
}

TX_NAMESPACE_CLOSE(Utils)
