/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <boost/filesystem.hpp>
#include <string>
#include <vector>

class CSimulation;

class CSceneData {
 public:
  static CSceneData& Instance();

  const wchar_t* GetSceneData(const wchar_t* wstrSceneName);
  const wchar_t* GetTemplateData(const wchar_t* wstrTemplateName);

  const wchar_t* SaveSceneData(const wchar_t* wstrSceneName, const wchar_t* wstrSceneContent);

  const wchar_t* ParamScene(const wchar_t* wstrSceneName, const wchar_t* wstrParam);
  const wchar_t* ParamSceneCount(const wchar_t* wstrSceneName, const wchar_t* wstrParam);

  const wchar_t* GetSceneDataByParam(const wchar_t* wstrParam);
  const wchar_t* SaveSceneDataByParam(const wchar_t* wstrParam);

  const std::wstring OpenScenarioImpl(const wchar_t* ws_param);
  const std::wstring SaveScenarioImpl(const wchar_t* ws_param);
  const std::wstring Xosc2SimImpl(const wchar_t* ws_param);
  const std::wstring Xosc2SimrecImpl(const wchar_t* ws_param);
  const std::wstring GetCatalogImpl(const char* param);
  const std::wstring SetCatalogImpl(const char* param);

  const wchar_t* SyncScenarioDBFromDisk();
  const wchar_t* GetSceneList(int offset, int limit);
  const wchar_t* GetSceneListByParam(const wchar_t* wstrParam);
  const wchar_t* GetGISImageList(int offset, int limit);
  const wchar_t* GetSceneInfo(const wchar_t* strSceneName);
  const wchar_t* GetSceneInfoByParam(const wchar_t* wstrParam);
  const wchar_t* DeleteScene(const wchar_t* strSceneName);

  const wchar_t* SearchScene(const wchar_t* strSceneName, int nOffset, int nLimit);
  const wchar_t* SearchSceneByParam(const wchar_t* wstrParam);

  const wchar_t* GetScenesInfo(const wchar_t* strSceneName);
  const wchar_t* GetScenesInfoByParam(const wchar_t* wstrParam);

  const wchar_t* DeleteScenes(const wchar_t* strParams);
  const wchar_t* DownloadScenes(const wchar_t* strParams);
  const wchar_t* DownloadScenesV2(const wchar_t* strParams);
  const wchar_t* UploadScenes(const wchar_t* strParams);
  const wchar_t* CopyScenes(const wchar_t* strParams);
  const wchar_t* RenameScenes(const wchar_t* strParams);

  // sensor file
  const wchar_t* LoadGlobalSensorConfig();
  const wchar_t* SaveGlobalSensorConfig(const wchar_t* strConfigContent);
  const wchar_t* LoadGlobalEnvironmentConfig();
  const wchar_t* SaveGlobalEnvironmentConfig(const wchar_t* strConfigContent);

  // hadmap editor
  const wchar_t* GetHadmapSceneList(int offset, int limit);
  const wchar_t* SearchHadmapScene(const wchar_t* strSceneName, int nOffset, int nLimit);

  int ReleaseStr(const wchar_t* strContent);

  int Convert2OpenScenario(const char* strSrcDir, const char* strDstDir);

 protected:
  int DeleteSingleScene(const char* strFileName);
  int CopySingleScene(const char* strFileName, const char* strSrcDir, const char* strDestDir, std::string& strDestPath);

  int ExportInOneScene(boost::filesystem::path oneSceneFile, std::string strScenarioSetID);

  int ExportInBatchGeneratedScenes(const char* strSrcDir, std::string& strExistFiles, std::string& strFailedFiles);

  const wchar_t* GetFileList(std::vector<std::string> relativePaths, std::string strExt, int offset, int limit);
  const wchar_t* SearchFile(std::string relativePath, std::string strExt, std::string strSearchName, int offset,
                            int limit);
  const wchar_t* GetParamSceneCount(uint64_t ullCount, uint64_t ullSize, uint64_t ullTime);

  std::string GetTrafficFile(const char* strSimFile);
  int GetRelatedFiles(const char* strSimFile, CSimulation& sim);

 private:
  std::wstring m_wstrSceneContent;
  std::wstring m_wstrGlobalSensorConfig;
  std::wstring m_wstrGlobalEnvironmentConfig;
  std::wstring m_wstrSceneList;
  std::wstring m_wstrParamSceneCount;
  std::wstring m_wstrParamSceneResp;
  std::wstring m_wstrSceneInfo;
  std::wstring m_wstrSaveSceneResp;
  std::wstring m_wstrDownloadSceneResp;
  std::wstring m_wstrUploadSceneResp;
};
