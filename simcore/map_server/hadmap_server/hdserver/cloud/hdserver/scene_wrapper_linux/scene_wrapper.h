/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/


#pragma once

#include <stddef.h>

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

typedef struct StringSlice {
  char* data;
  size_t size;
} StringSlice;

SCENE_WRAPPER_API StringSlice* NewStringSlice(const char* data, size_t size);
SCENE_WRAPPER_API void DeleteStringSlice(StringSlice* p);
SCENE_WRAPPER_API const char* StringSliceData(StringSlice* p);
SCENE_WRAPPER_API size_t StringSliceSize(StringSlice* p);

SCENE_WRAPPER_API int init(const char* strPath, const char* strAppInstallPath, int argc, char* argv[]);
SCENE_WRAPPER_API int deinit();
SCENE_WRAPPER_API int testInfo(const char* strContent);
SCENE_WRAPPER_API const char* getConfiguration();

SCENE_WRAPPER_API const char* getL3StateMachine();

// newly added.
SCENE_WRAPPER_API const char* cloudHadmapUploadCallback(const char* strParams);
SCENE_WRAPPER_API const char* getForwardPoints(const char* strParams);
SCENE_WRAPPER_API const char* getLaneRel(const char* strParams);

SCENE_WRAPPER_API const char* getRoadRel(const char* strParams);
// hadmap
SCENE_WRAPPER_API const char* loadHadmap(const char* strMapName, const char* version, const char* fineRsp,
                                         const char* cosBucket, const char* cosKey);
SCENE_WRAPPER_API const char* getEtag(const char* strMapName, const char* version);
SCENE_WRAPPER_API const char* getRoadData(const char* strMapName, const char* version);
SCENE_WRAPPER_API const char* getLaneData(const char* strMapName, const char* version);
SCENE_WRAPPER_API const char* getLaneBoundaryData(const char* strMapName, const char* version);
SCENE_WRAPPER_API const char* getLaneLinkData(const char* strMapName, const char* version);
SCENE_WRAPPER_API const char* getMapObjectData(const char* strMapName, const char* version);
SCENE_WRAPPER_API const char* getTrafficLightData(const char* strMapName, const char* version);

SCENE_WRAPPER_API const char* queryNearbyInfo(const char* strQueryString);
SCENE_WRAPPER_API const char* queryNearbyLane(const char* strMapName, char* strLon, char* strLat, const char* version);
SCENE_WRAPPER_API const char* queryInfobyPt(const char* strMapName, char* strLon, char* strLat, const char* version);
SCENE_WRAPPER_API const char* batchQueryInfobyPt(const char* strMapName, const char* version, const char* param);
SCENE_WRAPPER_API const char* queryInfobyLocalPt(const char* strMapName, char* strX, char* strY, char* strZ);
SCENE_WRAPPER_API const char* queryNextLane(const char* strMapName, char* strRoadID, char* strSecID, char* strLaneID,
                                            const char* version);
SCENE_WRAPPER_API const char* queryLonLat(const char* strMapName, char* strRoadID, char* strSecID, char* strLaneID,
                                          char* strShift, char* strOffset, const char* version);
SCENE_WRAPPER_API const char* queryLonLatByPoint(const char* strMapName, char* strLon, char* strLat, char* strElemType,
                                                 char* strElemID, char* strShift, char* strOffset, const char* version);
SCENE_WRAPPER_API const char* batchQueryLonLatByPoint(const char* strMapName, const char* version, const char* param);

// hadmap list
SCENE_WRAPPER_API const char* getMapList();
SCENE_WRAPPER_API const char* getMapinfoList();
SCENE_WRAPPER_API const char* deleteHadmaps(const char* strParams);
SCENE_WRAPPER_API const char* downloadHadmaps(const char* strParams);
SCENE_WRAPPER_API const char* uploadHadmaps(const char* strParams);

// hadmap editor
SCENE_WRAPPER_API const char* getHadmapSceneList(const int offset, const int limit);
SCENE_WRAPPER_API const char* searchHadmapScene(const char* strHadmapSceneName, const int offset, const int limit);
SCENE_WRAPPER_API const char* getHadmapSceneData(const char* strSceneName);
SCENE_WRAPPER_API const char* getHadmapSceneData3(const char* strSceneName, const char* version, const char* bucket,
                                                  const char* mapKey, const char* jsonKey);
SCENE_WRAPPER_API const char* saveHadmapSceneData(const char* strSceneName, const char* strSceneContent);
SCENE_WRAPPER_API const char* saveHadmapSceneData2(const char* strSceneContent);
SCENE_WRAPPER_API const char* saveHadmapSceneData3(const char* strSceneContent);

SCENE_WRAPPER_API const char* getGISImageList(const int offset, const int limit);
SCENE_WRAPPER_API const char* uploadGISImage(const char* strParams);
SCENE_WRAPPER_API const char* deleteGISImage(const char* strParams);
SCENE_WRAPPER_API const char* renameGISImage(const char* strParams);

// scene file
SCENE_WRAPPER_API const char* importSceneTemplate(const char* strTemplateName, const char* strMapName);
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

SCENE_WRAPPER_API const char* OpenScenario(const char* param);
SCENE_WRAPPER_API const char* SaveScenario(const char* param);
SCENE_WRAPPER_API const char* Xosc2Sim(const char* param);
SCENE_WRAPPER_API const char* Xosc2Simrec(const char* param);
SCENE_WRAPPER_API const char* GetCatalog(const char* param);
SCENE_WRAPPER_API const char* SetCatalog(const char* param);
SCENE_WRAPPER_API const char* SearchPath(const char* param);

SCENE_WRAPPER_API const char* BuildNavMesh(const char* obj, const char* settings, const char* events,
                                           const char* coskey);
SCENE_WRAPPER_API const StringSlice* TransDroneToSimrec(const char* drone, const char* capsule, int interval);

#ifdef __cplusplus
}
#endif
