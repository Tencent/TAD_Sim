/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <boost/filesystem.hpp>
#include <string>

class CHadmapSceneData {
 public:
  CHadmapSceneData();

  static CHadmapSceneData& Instance();

  const wchar_t* GetHadmapSceneData(const wchar_t* wstrHadmapSceneName);
  const wchar_t* GetHadmapSceneDataV2(const wchar_t* wstrHadmapSceneName);
  const wchar_t* GetHadmapSceneDataV3(const wchar_t* wstrHadmapSceneName);

  int DeleteHadmapSceneData(const wchar_t* wstrHadmapSceneName);

  const wchar_t* SaveHadmapSceneData(const wchar_t* wstrHadmapSceneName, const wchar_t* wstrHadmapSceneContent);

  const wchar_t* SaveHadmapSceneDataV2(const wchar_t* wstrHadmapSceneContent);
  const wchar_t* SaveHadmapSceneDataV3(const wchar_t* wstrHadmapSceneContent);

  const wchar_t* uploadGISImage(const wchar_t* wstrParams);

  const wchar_t* deleteGISImage(const wchar_t* wstrParams);

  const wchar_t* renameGISImage(const wchar_t* wstrParams);

  const wchar_t* uploadGISModel(const wchar_t* wstrParams);

  const wchar_t* deleteGISModel(const wchar_t* wstrParams);

  const wchar_t* renameGISModel(const wchar_t* wstrParams);

 protected:
  void FormResponseStr(std::vector<boost::filesystem::path>& vPaths);
  int DeleteSingleGISImage(const char* strFileName);
  int CopySingleGISImage(const char* strFileName, const char* strSrcDir, const char* strDestDir);
  int RenameSingleGISImage(const char* strFileName, const char* strNewFileName);

  int DeleteSingleGISModel(const char* strFileName);
  int CopySingleGISModel(const char* strFileName, const char* strSrcDir, const char* strDestDir);
  int RenameSingleGISModel(const char* strFileName, const char* strNewFileName);

 private:
  std::wstring m_wstrSceneContent;
  std::wstring m_wstrSceneList;
  std::wstring m_wstrSceneInfo;
  std::wstring m_wstrSucceed;
  std::wstring m_wstrSucceedUploadImages;
};
