/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#ifndef SCENE_WRAPPER_EXPORTS
#  ifdef _WINDOWS
#    define SCENE_WRAPPER_API _declspec(dllimport)
#  else
#    define SCENE_WRAPPER_API
#  endif  // _WINDOWS

#else

#  ifdef _WINDOWS
#    define SCENE_WRAPPER_API _declspec(dllexport)
#  else
#    define SCENE_WRAPPER_API __attribute__((visibility("default")))
#  endif

#endif
#include <vector>

extern "C" SCENE_WRAPPER_API int init(const wchar_t* wstrResourcePath, const wchar_t* wstrAppInstallDir);
extern "C" SCENE_WRAPPER_API int deinit();
extern "C" SCENE_WRAPPER_API int testInfo(const wchar_t* wstrContent);
extern "C" SCENE_WRAPPER_API const wchar_t* getConfiguration();

extern "C" SCENE_WRAPPER_API const wchar_t* getL3StateMachine();

// hadmap
extern "C" SCENE_WRAPPER_API const wchar_t* loadHadmap(const wchar_t* strMapName);
extern "C" SCENE_WRAPPER_API const wchar_t* getRoadData(const wchar_t* strMapName);
extern "C" SCENE_WRAPPER_API const wchar_t* getLaneData(const wchar_t* strMapName);
extern "C" SCENE_WRAPPER_API const wchar_t* getLaneBoundaryData(const wchar_t* strMapName);
extern "C" SCENE_WRAPPER_API const wchar_t* getLaneLinkData(const wchar_t* strMapName);
extern "C" SCENE_WRAPPER_API const wchar_t* getMapObjectData(const wchar_t* strMapName);
extern "C" SCENE_WRAPPER_API const wchar_t* getTrafficlights(const wchar_t* strMapName);

extern "C" SCENE_WRAPPER_API const wchar_t* queryNearbyInfo(const wchar_t* strQueryString);
extern "C" SCENE_WRAPPER_API const wchar_t* queryNearbyLane(const wchar_t* strMapName, wchar_t* wstrLon,
                                                            wchar_t* wstrLat);
extern "C" SCENE_WRAPPER_API const wchar_t* queryInfobyPt(const wchar_t* strMapName, wchar_t* wstrLon, wchar_t* wstrLat,
                                                          wchar_t* wstrSearchDistance /*, wchar_t * wstrAlt*/);
extern "C" SCENE_WRAPPER_API const wchar_t* batchQueryInfobyPt(const wchar_t* strMapName, const wchar_t* param);
extern "C" SCENE_WRAPPER_API const wchar_t* queryInfobyLocalPt(const wchar_t* strMapName, wchar_t* wstrX,
                                                               wchar_t* wstrY, wchar_t* wstrZ);
extern "C" SCENE_WRAPPER_API const wchar_t* queryNextLane(const wchar_t* strMapName, wchar_t* wstrRoadID,
                                                          wchar_t* wstrSecID, wchar_t* wstrLaneID);
extern "C" SCENE_WRAPPER_API const wchar_t* queryLonLat(const wchar_t* strMapName, wchar_t* wstrRoadID,
                                                        wchar_t* wstrSecID, wchar_t* wstrLaneID, wchar_t* wstrShift,
                                                        wchar_t* wstrOffset);
extern "C" SCENE_WRAPPER_API const wchar_t* queryLonLatByPoint(const wchar_t* strMapName, wchar_t* wstrLon,
                                                               wchar_t* wstrLat /*,wchar_t * wstrAlt*/,
                                                               wchar_t* strElemType, wchar_t* wstrElemID,
                                                               wchar_t* wstrShift, wchar_t* wstrOffset);
extern "C" SCENE_WRAPPER_API const wchar_t* batchQueryLonLatByPoint(const wchar_t* strMapName, const wchar_t* param);
// hadmap list
extern "C" SCENE_WRAPPER_API const wchar_t* getMapList();
extern "C" SCENE_WRAPPER_API const wchar_t* getMapinfoList();
extern "C" SCENE_WRAPPER_API const wchar_t* deleteHadmaps(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* downloadHadmaps(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* uploadHadmaps(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* getHadmapsProgress();   // 导入导出进度
extern "C" SCENE_WRAPPER_API const wchar_t* stopHadmapsProgress();  // 终止进度

// hadmap editor
extern "C" SCENE_WRAPPER_API const wchar_t* getHadmapSceneList(const int offset, const int limit);

extern "C" SCENE_WRAPPER_API const wchar_t* searchHadmapScene(const wchar_t* strHadmapSceneName, const int offset,
                                                              const int limit);
extern "C" SCENE_WRAPPER_API const wchar_t* getHadmapSceneData(const wchar_t* strSceneName);
extern "C" SCENE_WRAPPER_API const wchar_t* getHadmapSceneData3(const wchar_t* strSceneName);
extern "C" SCENE_WRAPPER_API const wchar_t* saveHadmapSceneData(const wchar_t* strSceneName,
                                                                const wchar_t* strSceneContent);
extern "C" SCENE_WRAPPER_API const wchar_t* saveHadmapSceneData2(const wchar_t* strSceneContent);
extern "C" SCENE_WRAPPER_API const wchar_t* saveHadmapSceneData3(const wchar_t* strSceneContent);

extern "C" SCENE_WRAPPER_API const wchar_t* getGISImageList(const int offset, const int limit);
extern "C" SCENE_WRAPPER_API const wchar_t* uploadGISImage(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* deleteGISImage(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* renameGISImage(const wchar_t* wstrParams);

extern "C" SCENE_WRAPPER_API const wchar_t* getGISModelList(const int offset, const int limit);
extern "C" SCENE_WRAPPER_API const wchar_t* uploadGISModel(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* deleteGISModel(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* renameGISModel(const wchar_t* wstrParams);

// new map edit
/// create session
extern "C" SCENE_WRAPPER_API const wchar_t* mapEditCreateSession();
/// create session
extern "C" SCENE_WRAPPER_API const wchar_t* mapEditReleaseSession(const wchar_t* strmapinfo);
/// open map
extern "C" SCENE_WRAPPER_API const wchar_t* mapEditOpenHadmap(const wchar_t* strmapinfo);
/// create string
extern "C" SCENE_WRAPPER_API const wchar_t* mapEditCreateString(const wchar_t* strmapinfo);
/// create map
extern "C" SCENE_WRAPPER_API const wchar_t* mapEditCreateMap(const wchar_t* strmapinfo);
/// modify map
extern "C" SCENE_WRAPPER_API const wchar_t* mapEditModifyMap(const wchar_t* strmapinfo);
/// save map
extern "C" SCENE_WRAPPER_API const wchar_t* mapEditSaveMap(const wchar_t* strmapinfo);
/// model upload
extern "C" SCENE_WRAPPER_API const wchar_t* modelUpload(const wchar_t* zippath, const wchar_t* id);
/// map model save
extern "C" SCENE_WRAPPER_API const wchar_t* mapModelSave(const wchar_t* strModelInfo);
/// map model delete
extern "C" SCENE_WRAPPER_API const wchar_t* mapModelDelete(const wchar_t* strModelInfo);
/// get map model
extern "C" SCENE_WRAPPER_API const wchar_t* getMapModelList();
// scene model
/// save
extern "C" SCENE_WRAPPER_API const wchar_t* sceneModelSave(const wchar_t* strModelInfo);
/// delete
extern "C" SCENE_WRAPPER_API const wchar_t* sceneModelDelete(const wchar_t* strModelInfo);
// scene file
extern "C" SCENE_WRAPPER_API const wchar_t* importSceneTemplate(const wchar_t* strTemplateName);
extern "C" SCENE_WRAPPER_API const wchar_t* getSceneData(const wchar_t* strSceneName);
extern "C" SCENE_WRAPPER_API const wchar_t* saveSceneData(const wchar_t* strSceneName, const wchar_t* strSceneContent);
extern "C" SCENE_WRAPPER_API const wchar_t* paramScene(const wchar_t* strSceneName, const wchar_t* params);
extern "C" SCENE_WRAPPER_API const wchar_t* paramSceneCount(const wchar_t* strSceneName, const wchar_t* params);

// scene file from
extern "C" SCENE_WRAPPER_API const wchar_t* getSceneDataV2(const wchar_t* strParam);
extern "C" SCENE_WRAPPER_API const wchar_t* saveSceneDataV2(const wchar_t* strParam);
extern "C" SCENE_WRAPPER_API const wchar_t* saveSceneDataV3(const char* strParam);

// sensor file
extern "C" SCENE_WRAPPER_API const wchar_t* loadGlobalSensor();
extern "C" SCENE_WRAPPER_API const wchar_t* saveGlobalSensor(const wchar_t* strSensorContent);
extern "C" SCENE_WRAPPER_API const wchar_t* loadGlobalEnvironment();
extern "C" SCENE_WRAPPER_API const wchar_t* saveGlobalEnvironment(const wchar_t* strEnvironmentContent);

// catalog
extern "C" SCENE_WRAPPER_API const wchar_t* getCatalogList();
extern "C" SCENE_WRAPPER_API const wchar_t* addCatalog(const wchar_t* strCatalogInfo);
extern "C" SCENE_WRAPPER_API const wchar_t* updateCatalog(const wchar_t* strCatalogInfo);
extern "C" SCENE_WRAPPER_API const wchar_t* deleteCatalog(const wchar_t* strCatalogInfo);

// sensor
extern "C" SCENE_WRAPPER_API const wchar_t* sensorLoad();
extern "C" SCENE_WRAPPER_API const wchar_t* sensorAdd(const wchar_t* strCatalogInfo);
extern "C" SCENE_WRAPPER_API const wchar_t* sensorUpdate(const wchar_t* strCatalogInfo);
extern "C" SCENE_WRAPPER_API const wchar_t* sensorDelete(const wchar_t* strCatalogInfo);
extern "C" SCENE_WRAPPER_API const wchar_t* sensorSave(const wchar_t* strSensorInfo);

// sensorgroup
extern "C" SCENE_WRAPPER_API const wchar_t* sensorGroupLoad();
extern "C" SCENE_WRAPPER_API const wchar_t* sensorGroupAdd(const wchar_t* strAddInfo);
extern "C" SCENE_WRAPPER_API const wchar_t* sensorGroupDelete(const wchar_t* strDeleteInfo);

// dynamic
extern "C" SCENE_WRAPPER_API const wchar_t* dynamicLoad();
extern "C" SCENE_WRAPPER_API const wchar_t* dynamicSave(const wchar_t* strDynamicInfo);
extern "C" SCENE_WRAPPER_API const wchar_t* dynamicDelete(const wchar_t* strDynamicInfo);

// scene files
extern "C" SCENE_WRAPPER_API const wchar_t* getSceneList(const int offset, const int limit);
extern "C" SCENE_WRAPPER_API const wchar_t* getSceneInfo(const wchar_t* strSceneName);
extern "C" SCENE_WRAPPER_API const wchar_t* searchScene(const wchar_t* strSceneName, const int offset, const int limit);
extern "C" SCENE_WRAPPER_API const wchar_t* deleteScene(const wchar_t* strSceneName);
extern "C" SCENE_WRAPPER_API const wchar_t* getScenesInfo(const wchar_t* strSceneName);
extern "C" SCENE_WRAPPER_API const wchar_t* deleteScenes(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* downloadScenes(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* uploadScenes(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* copyScenes(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* renameScene(const wchar_t* wstrParams);

extern "C" SCENE_WRAPPER_API const wchar_t* getScenesProgress();      // 导入导出进度
extern "C" SCENE_WRAPPER_API const wchar_t* stopScenesProgress();     // 终止进度
extern "C" SCENE_WRAPPER_API const wchar_t* getGenScenesProgress();   // 导入导出进度
extern "C" SCENE_WRAPPER_API const wchar_t* stopGenScenesProgress();  // 终止进度

// scene files from db
extern "C" SCENE_WRAPPER_API const wchar_t* syncScenarioDBFromDisk();
extern "C" SCENE_WRAPPER_API const wchar_t* getSceneListV2(const wchar_t* wstrParam);
extern "C" SCENE_WRAPPER_API const wchar_t* searchSceneV2(const wchar_t* wstrParam);
extern "C" SCENE_WRAPPER_API const wchar_t* getSceneInfoV2(const wchar_t* wstrID);
extern "C" SCENE_WRAPPER_API const wchar_t* getScenesInfoV2(const wchar_t* wstrParam);

// scene set
extern "C" SCENE_WRAPPER_API const wchar_t* getScenarioSetList(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* createScenarioSet(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* updateScenarioSet(const wchar_t* wstrParams);
extern "C" SCENE_WRAPPER_API const wchar_t* deleteScenarioSet(const wchar_t* wstrParams);

//
extern "C" SCENE_WRAPPER_API const wchar_t* startGenerateSemantic(const wchar_t* wstrParam);
//
extern "C" SCENE_WRAPPER_API const wchar_t* generateSceneSet();

extern "C" SCENE_WRAPPER_API const wchar_t* insertMap(const wchar_t* wstrParam);

extern "C" SCENE_WRAPPER_API const wchar_t* insertScene(const wchar_t* wstrParam);

// tools
extern "C" SCENE_WRAPPER_API int releaseStr(const wchar_t* strContent);

extern "C" SCENE_WRAPPER_API int recordLog(const wchar_t* strContent);

extern "C" SCENE_WRAPPER_API const bool json2xml(const char* xmlfilePath, const char* sceneName,
                                                 const char* jsoncontent, int xmlType);
extern "C" SCENE_WRAPPER_API const char* xml2json(const char* xmlfilePath, const char* sceneName, int xmlType);
