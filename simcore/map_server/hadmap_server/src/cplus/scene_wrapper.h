// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#ifndef SCENE_WRAPPER_EXPORTS
#  ifdef _WINDOWS
#    define SCENE_WRAPPER_API _declspec(dllimport)
#  else
#    define SCENE_WRAPPER_API
#  endif
#else
#  ifdef _WINDOWS
#    define SCENE_WRAPPER_API _declspec(dllexport)
#  else
#    define SCENE_WRAPPER_API __attribute__((visibility("default")))
#  endif
#endif
#ifdef __cplusplus
extern "C" {
#endif
SCENE_WRAPPER_API int init(const char* strPath, const char* strAppInstallPath);
SCENE_WRAPPER_API int deinit();
SCENE_WRAPPER_API int testInfo(const char* strContent);
SCENE_WRAPPER_API const char* getConfiguration();

SCENE_WRAPPER_API const char* getL3StateMachine();

// hadmap
SCENE_WRAPPER_API const char* loadHadmap(const char* strMapName);
SCENE_WRAPPER_API const char* getRoadData(const char* strMapName);
SCENE_WRAPPER_API const char* getLaneData(const char* strMapName);
SCENE_WRAPPER_API const char* getLaneBoundaryData(const char* strMapName);
SCENE_WRAPPER_API const char* getLaneLinkData(const char* strMapName);
SCENE_WRAPPER_API const char* getMapObjectData(const char* strMapName);
SCENE_WRAPPER_API const char* getTrafficlights(const char* strMapName);
SCENE_WRAPPER_API const char* queryNearbyInfo(const char* strQueryString);
SCENE_WRAPPER_API const char* queryNearbyLane(const char* strMapName, char* strLon, char* strLat);
SCENE_WRAPPER_API const char* queryInfobyPt(const char* strMapName, char* strLon, char* strLat, char* searchDistance);
SCENE_WRAPPER_API const char* batchQueryInfobyPt(const char* strMapName, const char* param);
SCENE_WRAPPER_API const char* queryInfobyLocalPt(const char* strMapName, char* strX, char* strY, char* strZ);
SCENE_WRAPPER_API const char* queryNextLane(const char* strMapName, char* strRoadID, char* strSecID, char* strLaneID);
SCENE_WRAPPER_API const char* queryLonLat(const char* strMapName, char* strRoadID, char* strSecID, char* strLaneID,
                                          char* strShift, char* strOffset);
SCENE_WRAPPER_API const char* queryLonLatByPoint(const char* strMapName, char* strLon, char* strLat, char* strElemType,
                                                 char* strElemID, char* strShift, char* strOffset);
SCENE_WRAPPER_API const char* batchQueryLonLatByPoint(const char* strMapName, const char* param);
// hadmap list
SCENE_WRAPPER_API const char* getMapList();
SCENE_WRAPPER_API const char* getMapinfoList();
SCENE_WRAPPER_API const char* deleteHadmaps(const char* strParams);
SCENE_WRAPPER_API const char* downloadHadmaps(const char* strParams);
SCENE_WRAPPER_API const char* uploadHadmaps(const char* strParams);

SCENE_WRAPPER_API const char* getHadmapsProgress();   // 导入导出进度
SCENE_WRAPPER_API const char* stopHadmapsProgress();  // 终止进度

// hadmap editor
SCENE_WRAPPER_API const char* getHadmapSceneList(const int offset, const int limit);
SCENE_WRAPPER_API const char* searchHadmapScene(const char* strHadmapSceneName, const int offset, const int limit);
SCENE_WRAPPER_API const char* getHadmapSceneData(const char* strSceneName);
SCENE_WRAPPER_API const char* getHadmapSceneData3(const char* strSceneName);
SCENE_WRAPPER_API const char* saveHadmapSceneData(const char* strSceneName, const char* strSceneContent);
SCENE_WRAPPER_API const char* saveHadmapSceneData2(const char* strSceneContent);
SCENE_WRAPPER_API const char* saveHadmapSceneData3(const char* strSceneContent);

SCENE_WRAPPER_API const char* getGISImageList(const int offset, const int limit);
SCENE_WRAPPER_API const char* uploadGISImage(const char* strParams);
SCENE_WRAPPER_API const char* deleteGISImage(const char* strParams);
SCENE_WRAPPER_API const char* renameGISImage(const char* strParams);

// new map edit
/// create session
SCENE_WRAPPER_API const char* mapEditCreateSession();
/// create session
SCENE_WRAPPER_API const char* mapEditReleaseSession(const char* strmapinfo);
/// open map
SCENE_WRAPPER_API const char* mapEditOpenHadmap(const char* strmapinfo);
/// create string
SCENE_WRAPPER_API const char* mapEditCreateString(const char* strmapinfo);
/// create map
SCENE_WRAPPER_API const char* mapEditCreateMap(const char* strmapinfo);
/// modify map
SCENE_WRAPPER_API const char* mapEditModifyMap(const char* strmapinfo);
/// save map
SCENE_WRAPPER_API const char* mapEditSaveMap(const char* strmapinfo);

/// model upload
SCENE_WRAPPER_API const char* modelUpload(const char* zippath, const char* id);
/// map model save
SCENE_WRAPPER_API const char* mapModelSave(const char* strModelInfo);
/// map model delete
SCENE_WRAPPER_API const char* mapModelDelete(const char* strModelInfo);
/// get map model
SCENE_WRAPPER_API const char* getMapModelList();
// scene model
/// save
SCENE_WRAPPER_API const char* sceneModelSave(const char* strModelInfo);
/// delete
SCENE_WRAPPER_API const char* sceneModelDelete(const char* strModelInfo);

// scene file
SCENE_WRAPPER_API const char* importSceneTemplate(const char* strTemplateName);
SCENE_WRAPPER_API const char* getSceneData(const char* strSceneName);
SCENE_WRAPPER_API const char* saveSceneData(const char* strSceneName, const char* strSceneContent);
SCENE_WRAPPER_API const char* paramScene(const char* strSceneName, const char* params);
SCENE_WRAPPER_API const char* paramSceneCount(const char* strSceneName, const char* params);

// scene file from db
SCENE_WRAPPER_API const char* getSceneDataV2(const char* strParam);
SCENE_WRAPPER_API const char* saveSceneDataV2(const char* strParam);

// sensor file
SCENE_WRAPPER_API const char* loadGlobalSensor();
SCENE_WRAPPER_API const char* saveGlobalSensor(const char* strSensorContent);
SCENE_WRAPPER_API const char* loadGlobalEnvironment();
SCENE_WRAPPER_API const char* saveGlobalEnvironment(const char* strEnvironmentContent);

// scene files
SCENE_WRAPPER_API const char* getSceneList(const int offset, const int limit);
SCENE_WRAPPER_API const char* getSceneInfo(const char* strSceneName);
SCENE_WRAPPER_API const char* searchScene(const char* strSceneName, const int offset, const int limit);
SCENE_WRAPPER_API const char* deleteScene(const char* strSceneName);
SCENE_WRAPPER_API const char* getScenesInfo(const char* strSceneName);
SCENE_WRAPPER_API const char* deleteScenes(const char* strParams);
SCENE_WRAPPER_API const char* downloadScenes(const char* strParams);
SCENE_WRAPPER_API const char* uploadScenes(const char* strParams);
SCENE_WRAPPER_API const char* copyScenes(const char* strParams);
SCENE_WRAPPER_API const char* renameScene(const char* strParams);

SCENE_WRAPPER_API const char* getScenesProgress();   // 导入导出进度
SCENE_WRAPPER_API const char* stopScenesProgress();  // 终止进度

SCENE_WRAPPER_API const char* getGenScenesProgress();   // 导入导出进度
SCENE_WRAPPER_API const char* stopGenScenesProgress();  // 终止进度

// scene files from db
SCENE_WRAPPER_API const char* syncScenarioDBFromDisk();
SCENE_WRAPPER_API const char* getSceneListV2(const char* strParam);
SCENE_WRAPPER_API const char* searchSceneV2(const char* strParam);
SCENE_WRAPPER_API const char* getSceneInfoV2(const char* strID);
SCENE_WRAPPER_API const char* getScenesInfoV2(const char* strParam);

// scene set
SCENE_WRAPPER_API const char* getScenarioSetList(const char* strParams);
SCENE_WRAPPER_API const char* createScenarioSet(const char* strParams);
SCENE_WRAPPER_API const char* updateScenarioSet(const char* strParams);
SCENE_WRAPPER_API const char* deleteScenarioSet(const char* strParams);

SCENE_WRAPPER_API int releaseStr(char* strContent);
SCENE_WRAPPER_API int recordLog(const char* strContent);
// catalog
SCENE_WRAPPER_API const char* getCatalogList();
SCENE_WRAPPER_API const char* addCatalog(const char* strCatalogInfo);
SCENE_WRAPPER_API const char* updateCatalog(const char* strCatalogInfo);
SCENE_WRAPPER_API const char* deleteCatalog(const char* strCatalogInfo);

// sensor
SCENE_WRAPPER_API const char* sensorLoad();
SCENE_WRAPPER_API const char* sensorAdd(const char* strCatalogInfo);
SCENE_WRAPPER_API const char* sensorUpdate(const char* strCatalogInfo);
SCENE_WRAPPER_API const char* sensorDelete(const char* strCatalogInfo);
SCENE_WRAPPER_API const char* sensorSave(const char* strSensorInfo);

// sensorGroup
SCENE_WRAPPER_API const char* sensorGroupLoad();
SCENE_WRAPPER_API const char* sensorGroupAdd(const char* strAddInfo);
SCENE_WRAPPER_API const char* sensorGroupDelete(const char* strDeleteInfo);

// dynamic
SCENE_WRAPPER_API const char* dynamicLoad();
SCENE_WRAPPER_API const char* dynamicSave(const char* strDynamicInfo);
SCENE_WRAPPER_API const char* dynamicDelete(const char* strDynamicInfo);
// 语义生成
SCENE_WRAPPER_API const char* startGenerateSemantic(const char* wstrParam);
// 开始生成
SCENE_WRAPPER_API const char* generateSceneSet();

SCENE_WRAPPER_API const char* insertMap(const char* wstrParam);

SCENE_WRAPPER_API const char* insertScene(const char* wstrParam);

#ifdef __cplusplus
}
#endif
