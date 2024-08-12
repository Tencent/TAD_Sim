package SW

import (
	"encoding/json"
	"fmt"
	"syscall"
	"unicode/utf16"
	"unsafe"
)

type SceneWrapper struct {
	dll                     *syscall.LazyDLL
	initWrapper             *syscall.LazyProc
	getConfiguration        *syscall.LazyProc
	getL3StateMachine       *syscall.LazyProc
	loadHadmap              *syscall.LazyProc
	getTrafficlights        *syscall.LazyProc
	getRoad                 *syscall.LazyProc
	getLane                 *syscall.LazyProc
	getLaneBoundary         *syscall.LazyProc
	getLaneLink             *syscall.LazyProc
	getMapObject            *syscall.LazyProc
	queryNearbyLane         *syscall.LazyProc
	queryNearbyInfo         *syscall.LazyProc
	queryInfobyPt           *syscall.LazyProc
	batchQueryInfobyPt      *syscall.LazyProc
	queryInfobyLocalPt      *syscall.LazyProc
	queryNextLane           *syscall.LazyProc
	queryLonLat             *syscall.LazyProc
	queryLonLatByPoint      *syscall.LazyProc
	batchQueryLonLatByPoint *syscall.LazyProc
	getMapList              *syscall.LazyProc
	getMapinfoList          *syscall.LazyProc
	importSceneTemplate     *syscall.LazyProc
	getScene                *syscall.LazyProc
	getSceneV2              *syscall.LazyProc
	saveScene               *syscall.LazyProc
	saveSceneV2             *syscall.LazyProc
	paramScene              *syscall.LazyProc
	paramSceneCount         *syscall.LazyProc
	getSceneList            *syscall.LazyProc
	getSceneListV2          *syscall.LazyProc
	getSceneInfo            *syscall.LazyProc
	getSceneInfoV2          *syscall.LazyProc
	deleteScene             *syscall.LazyProc
	searchScene             *syscall.LazyProc
	searchSceneV2           *syscall.LazyProc
	getScenesInfo           *syscall.LazyProc
	getScenesInfoV2         *syscall.LazyProc
	syncScenarioDBFromDisk  *syscall.LazyProc
	saveHadmapScene         *syscall.LazyProc
	saveHadmapScene2        *syscall.LazyProc
	saveHadmapScene3        *syscall.LazyProc
	getHadmapSceneList      *syscall.LazyProc
	getHadmapScene          *syscall.LazyProc
	getHadmapScene3         *syscall.LazyProc
	searchHadmapScene       *syscall.LazyProc
	deleteHadmaps           *syscall.LazyProc
	downloadHadmaps         *syscall.LazyProc
	uploadHadmaps           *syscall.LazyProc
	deleteScenes            *syscall.LazyProc
	downloadScenes          *syscall.LazyProc
	uploadScenes            *syscall.LazyProc
	renameScene             *syscall.LazyProc
	copyScenes              *syscall.LazyProc
	getGISImageList         *syscall.LazyProc
	deleteGISImages         *syscall.LazyProc
	uploadGISImages         *syscall.LazyProc
	renameGISImages         *syscall.LazyProc
	loadGlobalSensor        *syscall.LazyProc
	saveGlobalSensor        *syscall.LazyProc
	loadGlobalEnvironment   *syscall.LazyProc
	saveGlobalEnvironment   *syscall.LazyProc
	getScenarioSetList      *syscall.LazyProc
	createScenarioSet       *syscall.LazyProc
	updateScenarioSet       *syscall.LazyProc
	deleteScenarioSet       *syscall.LazyProc
	releaseStr              *syscall.LazyProc
	recordLog               *syscall.LazyProc

	getScenesProgressval  *syscall.LazyProc
	stopScenesProgress    *syscall.LazyProc
	getHadmapsProgressval *syscall.LazyProc
	stopHadmapsProgress   *syscall.LazyProc

	getGenScenesProgressval *syscall.LazyProc
	stopGenScenesProgress   *syscall.LazyProc
	//new map edit
	mapEditCreateSession  *syscall.LazyProc
	mapEditReleaseSession *syscall.LazyProc
	mapEditOpenHadmap     *syscall.LazyProc
	mapEditCreateString   *syscall.LazyProc
	mapEditCreateMap      *syscall.LazyProc
	mapEditModifyMap      *syscall.LazyProc
	mapEditSaveMap        *syscall.LazyProc

	getCatalogList *syscall.LazyProc
	addCatalog     *syscall.LazyProc
	updateCatalog  *syscall.LazyProc
	deleteCatalog  *syscall.LazyProc

	sensorLoad   *syscall.LazyProc
	sensorSave   *syscall.LazyProc
	sensorAdd    *syscall.LazyProc
	sensorUpdate *syscall.LazyProc
	sensorDelete *syscall.LazyProc

	sensorGroupLoad   *syscall.LazyProc
	sensorGroupAdd    *syscall.LazyProc
	sensorGroupDelete *syscall.LazyProc

	dynamicLoad   *syscall.LazyProc
	dynamicSave   *syscall.LazyProc
	dynamicDelete *syscall.LazyProc

	modelUpload      *syscall.LazyProc
	mapModelSave     *syscall.LazyProc
	mapModelDelete   *syscall.LazyProc
	getMapModelList  *syscall.LazyProc
	sceneModelSave   *syscall.LazyProc
	sceneModelDelete *syscall.LazyProc

	startGenerateSemantic *syscall.LazyProc
	insertMap             *syscall.LazyProc
	generateSceneSet      *syscall.LazyProc
	insertScene           *syscall.LazyProc
}

func strPtr(s string) uintptr {
	return uintptr(unsafe.Pointer(syscall.StringToUTF16Ptr(s)))
}

// UTF16PtrToString converts a pointer to a null-terminated UTF-16 wide character
// array into a Go string.
func UTF16PtrToString(cstr *uint16) string {
	// Check if the pointer is not nil
	if cstr != nil {
		// Create a slice to hold the UTF-16 wide characters
		us := make([]uint16, 0, 256)

		// Iterate through the wide characters, incrementing the pointer by 2 bytes each time
		for p := uintptr(unsafe.Pointer(cstr)); ; p += 2 {
			// Dereference the pointer and get the current wide character
			u := *(*uint16)(unsafe.Pointer(p))

			// If the wide character is a null terminator (0), return the decoded string
			if u == 0 {
				return string(utf16.Decode(us))
			}

			// Append the wide character to the slice
			us = append(us, u)
		}
	}

	// Return an empty string if the pointer is nil
	return ""
}

func (_this *SceneWrapper) Initlize(path string, appInstallDir string) {

	_this.dll = syscall.NewLazyDLL("scene_wrapper.dll")
	_this.initWrapper = _this.dll.NewProc("init")

	_this.initWrapper.Call(strPtr(path), strPtr(appInstallDir))

	_this.loadHadmap = _this.dll.NewProc("loadHadmap")
	_this.getTrafficlights = _this.dll.NewProc("getTrafficlights")
	_this.getConfiguration = _this.dll.NewProc("getConfiguration")
	_this.getL3StateMachine = _this.dll.NewProc("getL3StateMachine")
	_this.getRoad = _this.dll.NewProc("getRoadData")
	_this.getLane = _this.dll.NewProc("getLaneData")
	_this.getLaneBoundary = _this.dll.NewProc("getLaneBoundaryData")
	_this.getLaneLink = _this.dll.NewProc("getLaneLinkData")
	_this.getMapObject = _this.dll.NewProc("getMapObjectData")
	_this.queryNearbyLane = _this.dll.NewProc("queryNearbyLane")
	_this.queryNearbyInfo = _this.dll.NewProc("queryNearbyInfo")
	_this.queryInfobyPt = _this.dll.NewProc("queryInfobyPt")
	_this.batchQueryInfobyPt = _this.dll.NewProc("batchQueryInfobyPt")
	_this.queryInfobyLocalPt = _this.dll.NewProc("queryInfobyLocalPt")
	_this.queryNextLane = _this.dll.NewProc("queryNextLane")
	_this.queryLonLat = _this.dll.NewProc("queryLonLat")
	_this.queryLonLatByPoint = _this.dll.NewProc("queryLonLatByPoint")
	_this.batchQueryLonLatByPoint = _this.dll.NewProc("batchQueryLonLatByPoint")
	_this.getMapList = _this.dll.NewProc("getMapList")
	_this.getMapinfoList = _this.dll.NewProc("getMapinfoList")
	_this.importSceneTemplate = _this.dll.NewProc("importSceneTemplate")
	_this.getScene = _this.dll.NewProc("getSceneData")
	_this.getSceneV2 = _this.dll.NewProc("getSceneDataV2")
	_this.saveScene = _this.dll.NewProc("saveSceneData")
	_this.saveSceneV2 = _this.dll.NewProc("saveSceneDataV2")
	_this.paramScene = _this.dll.NewProc("paramScene")
	_this.paramSceneCount = _this.dll.NewProc("paramSceneCount")
	_this.getSceneList = _this.dll.NewProc("getSceneList")
	_this.getSceneListV2 = _this.dll.NewProc("getSceneListV2")
	_this.syncScenarioDBFromDisk = _this.dll.NewProc("syncScenarioDBFromDisk")
	_this.getSceneInfo = _this.dll.NewProc("getSceneInfo")
	_this.getSceneInfoV2 = _this.dll.NewProc("getSceneInfoV2")
	_this.deleteScene = _this.dll.NewProc("deleteScene")
	_this.searchScene = _this.dll.NewProc("searchScene")
	_this.searchSceneV2 = _this.dll.NewProc("searchSceneV2")
	_this.getScenesInfo = _this.dll.NewProc("getScenesInfo")
	_this.getScenesInfoV2 = _this.dll.NewProc("getScenesInfoV2")
	_this.releaseStr = _this.dll.NewProc("releaseStr")
	_this.saveHadmapScene = _this.dll.NewProc("saveHadmapSceneData")
	_this.saveHadmapScene2 = _this.dll.NewProc("saveHadmapSceneData2")
	_this.saveHadmapScene3 = _this.dll.NewProc("saveHadmapSceneData3")
	_this.getHadmapSceneList = _this.dll.NewProc("getHadmapSceneList")
	_this.searchHadmapScene = _this.dll.NewProc("searchHadmapScene")
	_this.getHadmapScene = _this.dll.NewProc("getHadmapSceneData")
	_this.getHadmapScene3 = _this.dll.NewProc("getHadmapSceneData3")
	_this.deleteHadmaps = _this.dll.NewProc("deleteHadmaps")
	_this.downloadHadmaps = _this.dll.NewProc("downloadHadmaps")
	_this.uploadHadmaps = _this.dll.NewProc("uploadHadmaps")
	_this.deleteScenes = _this.dll.NewProc("deleteScenes")
	_this.downloadScenes = _this.dll.NewProc("downloadScenes")
	_this.uploadScenes = _this.dll.NewProc("uploadScenes")
	_this.copyScenes = _this.dll.NewProc("copyScenes")
	_this.renameScene = _this.dll.NewProc("renameScene")
	_this.getGISImageList = _this.dll.NewProc("getGISImageList")
	_this.deleteGISImages = _this.dll.NewProc("deleteGISImage")
	_this.uploadGISImages = _this.dll.NewProc("uploadGISImage")
	_this.renameGISImages = _this.dll.NewProc("renameGISImage")
	_this.loadGlobalSensor = _this.dll.NewProc("loadGlobalSensor")
	_this.saveGlobalSensor = _this.dll.NewProc("saveGlobalSensor")
	_this.loadGlobalEnvironment = _this.dll.NewProc("loadGlobalEnvironment")
	_this.saveGlobalEnvironment = _this.dll.NewProc("saveGlobalEnvironment")
	_this.getScenarioSetList = _this.dll.NewProc("getScenarioSetList")
	_this.createScenarioSet = _this.dll.NewProc("createScenarioSet")
	_this.updateScenarioSet = _this.dll.NewProc("updateScenarioSet")
	_this.deleteScenarioSet = _this.dll.NewProc("deleteScenarioSet")
	_this.recordLog = _this.dll.NewProc("recordLog")

	_this.getScenesProgressval = _this.dll.NewProc("getScenesProgress")
	_this.stopScenesProgress = _this.dll.NewProc("stopScenesProgress")
	_this.getHadmapsProgressval = _this.dll.NewProc("getHadmapsProgress")
	_this.stopHadmapsProgress = _this.dll.NewProc("stopHadmapsProgress")

	_this.getGenScenesProgressval = _this.dll.NewProc("getGenScenesProgress")
	_this.stopGenScenesProgress = _this.dll.NewProc("stopGenScenesProgress")

	_this.getCatalogList = _this.dll.NewProc("getCatalogList")
	_this.addCatalog = _this.dll.NewProc("addCatalog")
	_this.updateCatalog = _this.dll.NewProc("updateCatalog")
	_this.deleteCatalog = _this.dll.NewProc("deleteCatalog")

	_this.sensorLoad = _this.dll.NewProc("sensorLoad")
	_this.sensorSave = _this.dll.NewProc("sensorSave")
	_this.sensorAdd = _this.dll.NewProc("sensorAdd")
	_this.sensorUpdate = _this.dll.NewProc("sensorUpdate")
	_this.sensorDelete = _this.dll.NewProc("sensorDelete")

	_this.sensorGroupLoad = _this.dll.NewProc("sensorGroupLoad")
	_this.sensorGroupAdd = _this.dll.NewProc("sensorGroupAdd")
	_this.sensorGroupDelete = _this.dll.NewProc("sensorGroupDelete")

	_this.dynamicLoad = _this.dll.NewProc("dynamicLoad")
	_this.dynamicSave = _this.dll.NewProc("dynamicSave")
	_this.dynamicDelete = _this.dll.NewProc("dynamicDelete")

	_this.mapEditCreateSession = _this.dll.NewProc("mapEditCreateSession")
	_this.mapEditReleaseSession = _this.dll.NewProc("mapEditReleaseSession")
	_this.mapEditOpenHadmap = _this.dll.NewProc("mapEditOpenHadmap")
	_this.mapEditCreateString = _this.dll.NewProc("mapEditCreateString")
	_this.mapEditCreateMap = _this.dll.NewProc("mapEditCreateMap")
	_this.mapEditModifyMap = _this.dll.NewProc("mapEditModifyMap")
	_this.mapEditSaveMap = _this.dll.NewProc("mapEditSaveMap")
	_this.modelUpload = _this.dll.NewProc("modelUpload")
	_this.mapModelSave = _this.dll.NewProc("mapModelSave")
	_this.mapModelDelete = _this.dll.NewProc("mapModelDelete")
	_this.getMapModelList = _this.dll.NewProc("getMapModelList")
	_this.sceneModelSave = _this.dll.NewProc("sceneModelSave")
	_this.sceneModelDelete = _this.dll.NewProc("sceneModelDelete")
	_this.generateSceneSet = _this.dll.NewProc("generateSceneSet")
	_this.startGenerateSemantic = _this.dll.NewProc("startGenerateSemantic")
	_this.insertMap = _this.dll.NewProc("insertMap")
	_this.insertScene = _this.dll.NewProc("insertScene")
}

func (hadmap *SceneWrapper) LoadHadmap(hmName string) string {
	// Call the loadHadmap method with the given name and get the result
	s, _, _ := hadmap.loadHadmap.Call(strPtr(hmName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetTrafficlights retrieves the traffic light information
// for the specified hadmap name and returns it as a string.
func (hadmap *SceneWrapper) GetTrafficlights(hmName string) string {
	// Call the getTrafficlights method with the given name and get the result
	s, _, _ := hadmap.getTrafficlights.Call(strPtr(hmName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetConfiguration retrieves the current configuration of the SceneWrapper and returns it as a string.
func (hadmap *SceneWrapper) GetConfiguration() string {
	// Call the getConfiguration method and get the result
	s, _, _ := hadmap.getConfiguration.Call()

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetL3StateMachine retrieves the L3 state machine information
// for the SceneWrapper and returns it as a string.
func (hadmap *SceneWrapper) GetL3StateMachine() string {
	// Call the getL3StateMachine method and get the result
	s, _, _ := hadmap.getL3StateMachine.Call()

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetRoad retrieves the road information
// for the specified hadmap name and returns it as a string.
func (_this *SceneWrapper) GetRoad(hmName string) string {
	// Call the getRoad method with the given name and get the result
	s, _, _ := _this.getRoad.Call(strPtr(hmName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Release the memory allocated for the string using defer
	defer _this.releaseStr.Call(s)

	return str
}

// GetLane retrieves the lane information
// for the specified hadmap name and returns it as a string.
func (_this *SceneWrapper) GetLane(hmName string) string {
	// Call the getLane method with the given name and get the result
	s, _, _ := _this.getLane.Call(strPtr(hmName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Release the memory allocated for the string using defer
	defer _this.releaseStr.Call(s)

	return str
}

// GetLaneBoundary retrieves the lane boundary information for
// the specified hadmap name and returns it as a string.
func (_this *SceneWrapper) GetLaneBoundary(hmName string) string {
	// Call the getLaneBoundary method with the given name and get the result
	s, _, _ := _this.getLaneBoundary.Call(strPtr(hmName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Release the memory allocated for the string using defer
	defer _this.releaseStr.Call(s)

	return str
}

// GetLaneLink retrieves the lane link information
// for the specified hadmap name and returns it as a string.
func (_this *SceneWrapper) GetLaneLink(hmName string) string {
	// Call the getLaneLink method with the given name and get the result
	s, _, _ := _this.getLaneLink.Call(strPtr(hmName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Release the memory allocated for the string using defer
	defer _this.releaseStr.Call(s)

	return str
}

// GetMapObject retrieves the map object information
// for the specified hadmap name and returns it as a string.
func (_this *SceneWrapper) GetMapObject(hmName string) string {
	// Call the getMapObject method with the given name and get the result
	s, _, _ := _this.getMapObject.Call(strPtr(hmName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Release the memory allocated for the string using defer
	defer _this.releaseStr.Call(s)

	return str
}

// QueryNearbyLane queries the nearby lanes based on the given hadmap
// name, longitude, and latitude, and returns the result as a string.
func (_this *SceneWrapper) QueryNearbyLane(hmName string, lon string, lat string) string {
	// Call the queryNearbyLane method with the given parameters and get the result
	s, _, _ := _this.queryNearbyLane.Call(strPtr(hmName), strPtr(lon), strPtr(lat))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Unmarshal the JSON response into a map
	var r map[string]interface{}
	json.Unmarshal([]byte(str), &r)

	// If there is no error (err == 0), release the memory allocated for the string
	if r["err"] == 0 {
		_this.releaseStr.Call(s)
	}

	return str
}

// QueryNearbyInfo queries the nearby information based
// on the given query string and returns the result as a string.
func (_this *SceneWrapper) QueryNearbyInfo(qs string) string {
	// Call the queryNearbyInfo method with the given parameter and get the result
	s, _, _ := _this.queryNearbyInfo.Call(strPtr(qs))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Unmarshal the JSON response into a map
	var r map[string]interface{}
	json.Unmarshal([]byte(str), &r)

	// If there is no error (err == 0), release the memory allocated for the string
	if r["err"] == 0 {
		_this.releaseStr.Call(s)
	}

	return str
}

// QueryInfobyPt queries the information by point based on the given
// hadmap name, longitude, latitude, and search distance, and returns the result as a string.
func (_this *SceneWrapper) QueryInfobyPt(hmName string, lon string, lat string, searchDistance string) string {
	// Call the queryInfobyPt method with the given parameters and get the result
	s, _, _ := _this.queryInfobyPt.Call(strPtr(hmName), strPtr(lon), strPtr(lat), strPtr(searchDistance))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Unmarshal the JSON response into a map
	var r map[string]interface{}
	json.Unmarshal([]byte(str), &r)

	// If there is no error (err == 0), release the memory allocated for the string
	if r["err"] == 0 {
		_this.releaseStr.Call(s)
	}

	return str
}

// BatchQueryInfobyPt performs a batch query for information by point based
// on the given hadmap name and parameters, and returns the result as a string.
func (_this *SceneWrapper) BatchQueryInfobyPt(hmName string, param string) string {
	// Call the batchQueryInfobyPt method with the given parameters and get the result
	s, _, _ := _this.batchQueryInfobyPt.Call(strPtr(hmName), strPtr(param))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Release the memory allocated for the string using defer
	defer _this.releaseStr.Call(s)

	return str
}

// QueryInfobyLocalPt queries the information by local point based
// on the given hadmap name and coordinates, and returns the result as a string.
func (_this *SceneWrapper) QueryInfobyLocalPt(hmName string, x string, y string, z string) string {
	// Call the queryInfobyLocalPt method with the given parameters and get the result
	s, _, _ := _this.queryInfobyLocalPt.Call(strPtr(hmName), strPtr(x), strPtr(y), strPtr(z))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Unmarshal the JSON response into a map
	var r map[string]interface{}
	json.Unmarshal([]byte(str), &r)

	// If there is no error (err == 0), release the memory allocated for the string
	if r["err"] == 0 {
		_this.releaseStr.Call(s)
	}

	return str
}

// QueryNextLane queries the next lane based on the given
// hadmap name, road ID, section ID, and lane ID, and returns the result as a string.
func (_this *SceneWrapper) QueryNextLane(hmName string, roadid string, secid string, laneid string) string {
	// Call the queryNextLane method with the given parameters and get the result
	s, _, _ := _this.queryNextLane.Call(strPtr(hmName), strPtr(roadid), strPtr(secid), strPtr(laneid))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Unmarshal the JSON response into a map
	var r map[string]interface{}
	json.Unmarshal([]byte(str), &r)

	// If there is no error (err == 0), release the memory allocated for the string
	if r["err"] == 0 {
		_this.releaseStr.Call(s)
	}

	return str
}

// QueryLonLat queries the longitude and latitude based on the given
// hadmap name, road ID, section ID, lane ID, shift, and offset, and returns the result as a string.
func (_this *SceneWrapper) QueryLonLat(
	hmName string,
	roadid string,
	secid string,
	laneid string,
	shift string,
	offset string) string {

	// Call the queryLonLat method with the given parameters and get the result
	s, _, _ := _this.queryLonLat.Call(
		strPtr(hmName),
		strPtr(roadid),
		strPtr(secid),
		strPtr(laneid),
		strPtr(shift),
		strPtr(offset))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Unmarshal the JSON response into a map
	var r map[string]interface{}
	json.Unmarshal([]byte(str), &r)

	// If there is no error (err == 0), release the memory allocated for the string
	if r["err"] == 0 {
		_this.releaseStr.Call(s)
	}

	return str
}

// QueryLonLatByPoint queries the longitude and latitude by point based on the given
// hadmap name, longitude, latitude, element type, element ID, shift, and offset, and returns the result as a string.
func (_this *SceneWrapper) QueryLonLatByPoint(
	hmName string,
	lon string,
	lat string,
	elemType string,
	elemId string,
	shift string,
	offset string) string {

	// Call the queryLonLatByPoint method with the given parameters and get the result
	s, _, _ := _this.queryLonLatByPoint.Call(
		strPtr(hmName),
		strPtr(lon),
		strPtr(lat),
		strPtr(elemType),
		strPtr(elemId),
		strPtr(shift),
		strPtr(offset))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Unmarshal the JSON response into a map
	var r map[string]interface{}
	json.Unmarshal([]byte(str), &r)

	// If there is no error (err == 0), release the memory allocated for the string
	if r["err"] == 0 {
		_this.releaseStr.Call(s)
	}

	return str
}

// BatchQueryLonLatByPoint performs a batch query for longitude and latitude by point based
// on the given hadmap name and parameters, and returns the result as a string.
func (_this *SceneWrapper) BatchQueryLonLatByPoint(hmName string, param string) string {
	// Call the batchQueryLonLatByPoint method with the given parameters and get the result
	s, _, _ := _this.batchQueryLonLatByPoint.Call(strPtr(hmName), strPtr(param))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	// Release the memory allocated for the string using defer
	defer _this.releaseStr.Call(s)

	return str
}

// GetMapList retrieves the list of maps and returns the result as a string.
func (_this *SceneWrapper) GetMapList() string {
	// Call the getMapList method and get the result
	s, _, _ := _this.getMapList.Call()

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetMapinfoList retrieves the list of map information and returns the result as a string.
func (_this *SceneWrapper) GetMapinfoList() string {
	// Call the getMapinfoList method and get the result
	s, _, _ := _this.getMapinfoList.Call()

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// ImportSceneTemplate imports a scene template
// with the given template name and returns the result as a string.
func (_this *SceneWrapper) ImportSceneTemplate(tplName string) string {
	// Call the importSceneTemplate method with the given template name and get the result
	s, _, _ := _this.importSceneTemplate.Call(strPtr(tplName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetScene retrieves a scene with the given scene name and returns the result as a string.
func (_this *SceneWrapper) GetScene(sceneName string) string {
	// Call the getScene method with the given scene name and get the result
	s, _, _ := _this.getScene.Call(strPtr(sceneName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetSceneV2 retrieves a scene with the given parameters using the V2 method and returns the result as a string.
func (_this *SceneWrapper) GetSceneV2(param string) string {
	// Call the getSceneV2 method with the given parameters and get the result
	s, _, _ := _this.getSceneV2.Call(strPtr(param))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SaveScene saves a scene with the given scene name and content and returns the result as a string.
func (_this *SceneWrapper) SaveScene(sceneName string, sceneContent string) string {
	// Call the saveScene method with the given scene name and content and get the result
	s, _, _ := _this.saveScene.Call(strPtr(sceneName), strPtr(sceneContent))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SaveSceneV2 saves a scene with the given parameters using the V2 method and returns the result as a string.
func (_this *SceneWrapper) SaveSceneV2(param string) string {
	// Call the saveSceneV2 method with the given parameters and get the result
	s, _, _ := _this.saveSceneV2.Call(strPtr(param))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// ParamScene sets the parameters for a scene
// with the given scene name and parameters and returns the result as a string.
func (_this *SceneWrapper) ParamScene(sceneName string, param string) string {
	// Call the paramScene method with the given scene name and parameters and get the result
	s, _, _ := _this.paramScene.Call(strPtr(sceneName), strPtr(param))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// ParamSceneCount returns the count of scenes
// with the given scene name and parameters and returns the result as a string.
func (_this *SceneWrapper) ParamSceneCount(sceneName string, param string) string {
	// Call the paramSceneCount method with the given scene name and parameters and get the result
	s, _, _ := _this.paramSceneCount.Call(strPtr(sceneName), strPtr(param))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetSceneList retrieves the list of scenes with the given offset and limit and returns the result as a string.
func (_this *SceneWrapper) GetSceneList(offset int, limit int) string {
	// Call the getSceneList method with the given offset and limit and get the result
	s, _, _ := _this.getSceneList.Call(uintptr(offset), uintptr(limit))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetSceneListV2 retrieves the list of scenes
// with the given parameters using the V2 method and returns the result as a string.
func (_this *SceneWrapper) GetSceneListV2(param string) string {
	// Call the getSceneListV2 method with the given parameters and get the result
	s, _, _ := _this.getSceneListV2.Call(strPtr(param))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SyncScenarioDBFromDisk synchronizes the scenario database from disk and returns the result as a string.
func (_this *SceneWrapper) SyncScenarioDBFromDisk() string {
	// Call the syncScenarioDBFromDisk method and get the result
	s, _, _ := _this.syncScenarioDBFromDisk.Call()

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetSceneInfo retrieves the information of a scene
// with the given scene name and returns the result as a string.
func (_this *SceneWrapper) GetSceneInfo(sceneName string) string {
	// Call the getSceneInfo method with the given scene name and get the result
	s, _, _ := _this.getSceneInfo.Call(strPtr(sceneName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetSceneInfoV2 retrieves the information of a scene
// with the given ID using the V2 method and returns the result as a string.
func (_this *SceneWrapper) GetSceneInfoV2(id string) string {
	// Call the getSceneInfoV2 method with the given ID and get the result
	s, _, _ := _this.getSceneInfoV2.Call(strPtr(id))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// DeleteScene deletes a scene with the given scene name and returns the result as a string.
func (_this *SceneWrapper) DeleteScene(sceneName string) string {
	// Call the deleteScene method with the given scene name and get the result
	s, _, _ := _this.deleteScene.Call(strPtr(sceneName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SearchScene searches for scenes with the given scene
// name, offset, and limit and returns the result as a string.
func (_this *SceneWrapper) SearchScene(sceneName string, offset int, limit int) string {
	// Print the search parameters
	fmt.Print("SearchScene ", offset, " ", limit)

	// Call the searchScene method with the given parameters and get the result
	s, _, _ := _this.searchScene.Call(strPtr(sceneName), uintptr(offset), uintptr(limit))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SearchSceneV2 searches for scenes with the given parameters
// using the V2 method and returns the result as a string.
func (_this *SceneWrapper) SearchSceneV2(param string) string {
	// Print the search parameter
	fmt.Print("SearchScene V2 ", param)

	// Call the searchSceneV2 method with the given parameters and get the result
	s, _, _ := _this.searchSceneV2.Call(strPtr(param))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetScenesInfo retrieves the information of multiple scenes
// with the given scene names and returns the result as a string.
func (_this *SceneWrapper) GetScenesInfo(scenesName string) string {
	// Call the getScenesInfo method with the given scene names and get the result
	s, _, _ := _this.getScenesInfo.Call(strPtr(scenesName))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetScenesInfoV2 retrieves the information of multiple scenes
// with the given parameters using the V2 method and returns the result as a string.
func (_this *SceneWrapper) GetScenesInfoV2(param string) string {
	// Call the getScenesInfoV2 method with the given parameters and get the result
	s, _, _ := _this.getScenesInfoV2.Call(strPtr(param))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SaveHadmapScene saves a hadmap scene with the given scene name and content and returns the result as a string.
func (_this *SceneWrapper) SaveHadmapScene(sceneName string, sceneContent string) string {
	// Call the saveHadmapScene method with the given scene name and content and get the result
	s, _, _ := _this.saveHadmapScene.Call(strPtr(sceneName), strPtr(sceneContent))

	// Convert the result to a Go string using UTF16PtrToString
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SaveHadmapScene2 saves a hadmap scene with the given content and returns the result as a string.
func (_this *SceneWrapper) SaveHadmapScene2(sceneContent string) string {
	s, _, _ := _this.saveHadmapScene2.Call(strPtr(sceneContent))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// SaveHadmapScene3 saves a hadmap scene with the given content and returns the result as a string.
func (_this *SceneWrapper) SaveHadmapScene3(sceneContent string) string {
	s, _, _ := _this.saveHadmapScene3.Call(strPtr(sceneContent))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// GetHadmapSceneList retrieves a list of hadmap scenes
// with the given offset and limit and returns the result as a string.
func (_this *SceneWrapper) GetHadmapSceneList(offset int, limit int) string {
	s, _, _ := _this.getHadmapSceneList.Call(uintptr(offset), uintptr(limit))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// SearchHadmapScene searches for hadmap scenes
// with the given name, offset, and limit, and returns the result as a string.
func (_this *SceneWrapper) SearchHadmapScene(sceneName string, offset int, limit int) string {
	fmt.Print("SearchHadmapScene ", offset, " ", limit)
	s, _, _ := _this.searchHadmapScene.Call(strPtr(sceneName), uintptr(offset), uintptr(limit))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// GetHadmapScene retrieves a hadmap scene with the given name and returns the result as a string.
func (_this *SceneWrapper) GetHadmapScene(sceneName string) string {
	s, _, _ := _this.getHadmapScene.Call(strPtr(sceneName))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// GetHadmapScene3 retrieves a hadmap scene with the given name and returns the result as a string.
func (_this *SceneWrapper) GetHadmapScene3(sceneName string) string {
	s, _, _ := _this.getHadmapScene3.Call(strPtr(sceneName))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// DeleteHadmaps deletes hadmaps with the given parameters and returns the result as a string.
func (_this *SceneWrapper) DeleteHadmaps(params string) string {
	s, _, _ := _this.deleteHadmaps.Call(strPtr(params))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// DownloadHadmaps downloads hadmaps with the given parameters and returns the result as a string.
func (_this *SceneWrapper) DownloadHadmaps(params string) string {
	s, _, _ := _this.downloadHadmaps.Call(strPtr(params))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// UploadHadmaps uploads hadmaps with the given parameters and returns the result as a string.
func (_this *SceneWrapper) UploadHadmaps(params string) string {
	s, _, _ := _this.uploadHadmaps.Call(strPtr(params))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// DeleteScenes deletes scenes with the given parameters and returns the result as a string.
func (_this *SceneWrapper) DeleteScenes(params string) string {
	s, _, _ := _this.deleteScenes.Call(strPtr(params))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// DownloadScenes downloads scenes with the given parameters and returns the result as a string.
func (_this *SceneWrapper) DownloadScenes(params string) string {
	s, _, _ := _this.downloadScenes.Call(strPtr(params))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// UploadScenes uploads scenes with the given parameters and returns the result as a string.
func (_this *SceneWrapper) UploadScenes(params string) string {
	s, _, _ := _this.uploadScenes.Call(strPtr(params))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// CopyScenes copies scenes with the given parameters and returns the result as a string.
func (_this *SceneWrapper) CopyScenes(params string) string {
	s, _, _ := _this.copyScenes.Call(strPtr(params))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// RenameScene renames a scene with the given parameters and returns the result as a string.
func (_this *SceneWrapper) RenameScene(params string) string {
	s, _, _ := _this.renameScene.Call(strPtr(params))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// GetGISImageList retrieves a list of GIS images
// with the given offset and limit and returns the result as a string.
func (_this *SceneWrapper) GetGISImageList(offset int, limit int) string {
	s, _, _ := _this.getGISImageList.Call(uintptr(offset), uintptr(limit))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// DeleteGISImages deletes GIS images with the given parameters and returns the result as a string.
func (_this *SceneWrapper) DeleteGISImages(params string) string {
	s, _, _ := _this.deleteGISImages.Call(strPtr(params))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))
	return str
}

// UploadGISImages uploads GIS images with the given parameters.
func (_this *SceneWrapper) UploadGISImages(params string) string {
	s, _, _ := _this.uploadGISImages.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// RenameGISImages renames GIS images with the given parameters.
func (_this *SceneWrapper) RenameGISImages(params string) string {
	s, _, _ := _this.renameGISImages.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// LoadGlobalSensor loads the global sensor data.
func (_this *SceneWrapper) LoadGlobalSensor() string {
	s, _, _ := _this.loadGlobalSensor.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SaveGlobalSensor saves the global sensor data with the given config content.
func (_this *SceneWrapper) SaveGlobalSensor(configContent string) string {
	s, _, _ := _this.saveGlobalSensor.Call(strPtr(configContent))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// LoadGlobalEnvironment loads the global environment data.
func (_this *SceneWrapper) LoadGlobalEnvironment() string {
	s, _, _ := _this.loadGlobalEnvironment.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SaveGlobalEnvironment saves the global environment data with the given config content.
func (_this *SceneWrapper) SaveGlobalEnvironment(configContent string) string {
	s, _, _ := _this.saveGlobalEnvironment.Call(strPtr(configContent))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetScenarioSetList retrieves the list of scenario sets with the given parameters.
func (_this *SceneWrapper) GetScenarioSetList(params string) string {
	s, _, _ := _this.getScenarioSetList.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// CreateScenarioSet creates a new scenario set with the given parameters.
func (_this *SceneWrapper) CreateScenarioSet(params string) string {
	s, _, _ := _this.createScenarioSet.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// UpdateScenarioSet updates an existing scenario set with the given parameters.
func (_this *SceneWrapper) UpdateScenarioSet(params string) string {
	s, _, _ := _this.updateScenarioSet.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// DeleteScenarioSet deletes a scenario set with the given parameters.
func (_this *SceneWrapper) DeleteScenarioSet(params string) string {
	s, _, _ := _this.deleteScenarioSet.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// RecordLog records a log message with the given content.
func (_this *SceneWrapper) RecordLog(content string) {
	_this.recordLog.Call(strPtr(content))
}

// GetScenesProgress retrieves the progress of loading scenes.
func (_this *SceneWrapper) GetScenesProgress() string {
	s, _, _ := _this.getScenesProgressval.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// StopScenesProgress stops the progress of loading scenes.
func (_this *SceneWrapper) StopScenesProgress() string {
	s, _, _ := _this.stopScenesProgress.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetHadmapsProgress retrieves the progress of loading hadmaps.
func (_this *SceneWrapper) GetHadmapsProgress() string {
	s, _, _ := _this.getHadmapsProgressval.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// StopHadmapsProgress stops the progress of loading hadmaps.
func (_this *SceneWrapper) StopHadmapsProgress() string {
	s, _, _ := _this.stopHadmapsProgress.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetGenScenesProgress retrieves the progress of generating scenes.
func (_this *SceneWrapper) GetGenScenesProgress() string {
	s, _, _ := _this.getGenScenesProgressval.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// StopGenScenesProgress stops the progress of generating scenes.
func (_this *SceneWrapper) StopGenScenesProgress() string {
	s, _, _ := _this.stopGenScenesProgress.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetCatalogList retrieves the list of catalogs.
func (_this *SceneWrapper) GetCatalogList() string {
	s, _, _ := _this.getCatalogList.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// AddCatalog adds a new catalog with the given parameters.
func (_this *SceneWrapper) AddCatalog(params string) string {
	s, _, _ := _this.addCatalog.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// UpdateCatalog updates an existing catalog with the given parameters.
func (_this *SceneWrapper) UpdateCatalog(params string) string {
	s, _, _ := _this.updateCatalog.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// DeleteCatalog deletes a catalog with the given parameters.
func (_this *SceneWrapper) DeleteCatalog(params string) string {
	s, _, _ := _this.deleteCatalog.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SensorLoad loads sensor data.
func (_this *SceneWrapper) SensorLoad() string {
	s, _, _ := _this.sensorLoad.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SensorSave saves sensor data with the given parameters.
func (_this *SceneWrapper) SensorSave(params string) string {
	s, _, _ := _this.sensorSave.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SensorAdd adds a new sensor with the given parameters.
func (_this *SceneWrapper) SensorAdd(params string) string {
	s, _, _ := _this.sensorAdd.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SensorUpdate updates an existing sensor with the given parameters.
func (_this *SceneWrapper) SensorUpdate(params string) string {
	s, _, _ := _this.sensorUpdate.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SensorDelete deletes a sensor with the given parameters.
func (_this *SceneWrapper) SensorDelete(params string) string {
	s, _, _ := _this.sensorDelete.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SensorGroupLoad loads sensor group data.
func (_this *SceneWrapper) SensorGroupLoad() string {
	s, _, _ := _this.sensorGroupLoad.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SensorGroupAdd adds a new sensor group with the given parameters.
func (_this *SceneWrapper) SensorGroupAdd(params string) string {
	s, _, _ := _this.sensorGroupAdd.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SensorGroupDelete deletes a sensor group with the given parameters.
func (_this *SceneWrapper) SensorGroupDelete(params string) string {
	s, _, _ := _this.sensorGroupDelete.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// DynamicLoad loads dynamic data.
func (_this *SceneWrapper) DynamicLoad() string {
	s, _, _ := _this.dynamicLoad.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// DynamicSave saves dynamic data with the given parameters.
func (_this *SceneWrapper) DynamicSave(params string) string {
	s, _, _ := _this.dynamicSave.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// DynamicDelete deletes dynamic data with the given parameters.
func (_this *SceneWrapper) DynamicDelete(params string) string {
	s, _, _ := _this.dynamicDelete.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// MapEditCreateSession creates a new map editing session.
func (_this *SceneWrapper) MapEditCreateSession() string {
	s, _, _ := _this.mapEditCreateSession.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// MapEditReleaseSession releases an existing map editing session with the given parameters.
func (_this *SceneWrapper) MapEditReleaseSession(params string) string {
	s, _, _ := _this.mapEditReleaseSession.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// MapEditOpenHadmap opens an existing hadmap with the given parameters.
func (_this *SceneWrapper) MapEditOpenHadmap(params string) string {
	s, _, _ := _this.mapEditOpenHadmap.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// MapEditCreateString creates a new string in the map editor with the given parameters.
func (_this *SceneWrapper) MapEditCreateString(params string) string {
	s, _, _ := _this.mapEditCreateString.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// MapEditCreateMap creates a new map in the map editor with the given parameters.
func (_this *SceneWrapper) MapEditCreateMap(params string) string {
	s, _, _ := _this.mapEditCreateMap.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// MapEditModifyMap modifies an existing map in the map editor with the given parameters.
func (_this *SceneWrapper) MapEditModifyMap(params string) string {
	s, _, _ := _this.mapEditModifyMap.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// MapEditSaveMap saves the current map in the map editor with the given parameters.
func (_this *SceneWrapper) MapEditSaveMap(params string) string {
	s, _, _ := _this.mapEditSaveMap.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// ModelUpload uploads a model with the given zip file and ID.
func (_this *SceneWrapper) ModelUpload(strzip string, strid string) string {
	s, _, _ := _this.modelUpload.Call(strPtr(strzip), strPtr(strid))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// MapModelSave saves the current map model with the given parameters.
func (_this *SceneWrapper) MapModelSave(params string) string {
	s, _, _ := _this.mapModelSave.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// MapModelDelete deletes a map model with the given parameters.
func (_this *SceneWrapper) MapModelDelete(params string) string {
	s, _, _ := _this.mapModelDelete.Call(strPtr(params))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// GetMapModelList retrieves the list of map models.
func (_this *SceneWrapper) GetMapModelList() string {
	s, _, _ := _this.getMapModelList.Call()
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SceneModelSave saves the current scene model with the given parameters.
func (_this *SceneWrapper) SceneModelSave(params string) string {
	s, _, _ := _this.sceneModelSave.Call((strPtr(params)))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

// SceneModelDelete deletes a scene model with the given parameters.
func (_this *SceneWrapper) SceneModelDelete(params string) string {
	s, _, _ := _this.sceneModelDelete.Call((strPtr(params)))
	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

func (_this *SceneWrapper) InsertMap(params string) string {
	s, _, _ := _this.insertMap.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

func (_this *SceneWrapper) StartGenerateSemantic(params string) string {
	s, _, _ := _this.startGenerateSemantic.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

func (_this *SceneWrapper) GenerateSceneSet() string {
	s, _, _ := _this.generateSceneSet.Call()

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}

func (_this *SceneWrapper) InsertScene(params string) string {
	s, _, _ := _this.insertScene.Call(strPtr(params))

	str := UTF16PtrToString((*uint16)(unsafe.Pointer(s)))

	return str
}
