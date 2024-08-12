/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <json/value.h>
#include <boost/filesystem.hpp>
#include <map>
#include <string>
#include <vector>

class CSimulation;

static bool isStopScenesProgess = false;
static double dScenesProgessVal = 0.0;
class CSceneData {
 public:
  static CSceneData& Instance();

  const wchar_t* GetSceneData(const wchar_t* wstrSceneName);
  const wchar_t* GetTemplateData(const wchar_t* wstrTemplateName);

  const wchar_t* SaveSceneData(const wchar_t* wstrSceneName, const wchar_t* wstrSceneContent);

  const wchar_t* ParamScene(const wchar_t* wstrSceneName, const wchar_t* wstrParam);
  const wchar_t* ParamSceneCount(const wchar_t* wstrSceneName, const wchar_t* wstrParam);

  const wchar_t* ParamSceneV2(const wchar_t* wstrSceneName, const wchar_t* wstrParam);
  const wchar_t* ParamSceneCountV2(const wchar_t* wstrSceneName, const wchar_t* wstrParam);

  const wchar_t* GetSceneDataByParam(const wchar_t* wstrParam);
  const wchar_t* SaveSceneDataByParam(const wchar_t* wstrParam);
  const wchar_t* SaveSceneDataByParam(const char* wstrParam);

  const wchar_t* SyncScenarioDBFromDisk();
  const wchar_t* GetSceneList(int offset, int limit);
  const wchar_t* GetSceneListByParam(const wchar_t* wstrParam);
  const wchar_t* GetGISImageList(int offset, int limit);
  const wchar_t* GetGISModelList(int offset, int limit);
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

  // catalog
  const wchar_t* GetCatalogList();

  const wchar_t* AddCatalogList(const wchar_t* strVechileCatalog);

  const wchar_t* UpdateCatalog(const wchar_t* strVechileCatalog);

  const wchar_t* DeleteCatalog(const wchar_t* strVechileCatalog);

  // sensor
  const wchar_t* SensorLoad();

  const wchar_t* SensorSave(const wchar_t* strSensor);

  const wchar_t* SensorAdd(const wchar_t* strSensor);

  const wchar_t* SensorUpdate(const wchar_t* strSensor);

  const wchar_t* SensorDelete(const wchar_t* strSensor);
  // sensor group
  const wchar_t* SensorGroupLoad();

  const wchar_t* SensorGroupSave(const wchar_t* strSensor);

  const wchar_t* SensorGroupDelete(const wchar_t* strSensor);

  // dynamic
  const wchar_t* DynamicLoad();

  const wchar_t* DynamicSave(const wchar_t* strDynamic);

  const wchar_t* DynamicDelete(const wchar_t* strDynamic);

  // model
  const wchar_t* ModelUpload(const wchar_t* zipPath, const wchar_t* strId);

  const wchar_t* MapModelSave(const wchar_t* strMapModelInfo);

  const wchar_t* MapModelDelete(const wchar_t* wstrMapModelInfo);

  const wchar_t* GetMapModelList();

  const wchar_t* SceneModelSave(const wchar_t* strMapModelInfo);

  const wchar_t* SceneModelDelete(const wchar_t* strMapModelInfo);
  // 语义生成
  const wchar_t* StartExe(const wchar_t* strParam);

  const wchar_t* CreateCurrentSceneSet();

  const wchar_t* InsertScenes(const wchar_t* strParam);

  const wchar_t* InsertOneMap(const wchar_t* strParam);

  // hadmap editor
  const wchar_t* GetHadmapSceneList(int offset, int limit);
  const wchar_t* SearchHadmapScene(const wchar_t* strSceneName, int nOffset, int nLimit);

  int ReleaseStr(const wchar_t* strContent);

  int Convert2OpenScenario(const char* strSrcDir, const char* strDstDir);

  const wchar_t* StopScenesProgess();
  const wchar_t* GetScenesProgessVal();

  const wchar_t* StopGenScenesProgess();
  const wchar_t* GetGenScenesProgessVal();


 protected:
  int DeleteSingleScene(const char* strFileName, const char* strExt);
  int CopySingleScene(const char* strFileName, const char* strSrcDir, const char* strDestDir, std::string& strDestPath,
                      std::string strExportype, std::string strExt = "");

  // 奇怪的导入方式用到
  int CopySingleScene(const char* strFileName, const char* strSrcDir, const char* strDestDir, std::string& strDestPath,
                      std::string strFlag, std::string strExt, std::string xoscType, std::string& newname);

  int ExportInOneScene(boost::filesystem::path oneSceneFile, std::string strScenarioSetID);

  int ExportInBatchGeneratedScenes(const char* strSrcDir, std::string& strExistFiles, std::string& strFailedFiles);

  const wchar_t* GetFileList(std::vector<std::string> relativePaths, std::string strExt, int offset, int limit);
  const wchar_t* SearchFile(std::string relativePath, std::string strExt, std::string strSearchName, int offset,
                            int limit);
  const wchar_t* GetParamSceneCount(uint64_t ullCount, uint64_t ullSize, uint64_t ullTime);

  std::string GetTrafficFile(const char* strSimFile);
  int GetRelatedFiles(const char* strSimFile, CSimulation& sim);

  std::string GetMapName() { return strMapName; }

 private:
  bool Wstr2Json(const wchar_t* wstr, Json::Value& json);
  bool IncludeChinese(const char* str);

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
  std::wstring m_wstrCataloglist;
  std::wstring m_wstrSensorLoad;
  std::wstring m_wstrSensorGroup;
  std::wstring m_wstrDynamicLoad;
  std::wstring m_wstrUploadModel;
  std::wstring m_wstrMapModelList;
  std::string strMapName;
  std::wstring m_wstrProgressVal;
  std::wstring m_wstrStopProgress;
  std::wstring strWMessageProgess;
  std::wstring strGenWMessageProgess;
  std::string strJsonContent;
  std::string m_SemanticSetName;
  std::wstring m_wstrStartExe;
};
