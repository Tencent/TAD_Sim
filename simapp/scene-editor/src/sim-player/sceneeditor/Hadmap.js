import axios from 'axios'
import { isObject } from 'lodash-es'
import {
  Box3,
  BufferGeometry,
  Float32BufferAttribute,
  Group,
  LineSegments,
  MathUtils,
  Ray,
  Raycaster,
  Vector3,
} from 'three'
import { mergeGeometries } from 'three/addons/utils/BufferGeometryUtils'
import { yawToVector3 } from 'sim-player/common/MeshUtils'
import { MapModelsManager } from 'models-manager/index'
import PlayerGlobals from '../simuplayer/PlayerGlobals'
import GlobalConfig from '../common/Config'
import { MapElementType, RunMode } from '../common/Constant'
import { getTextMesh } from '../render/TextMesh'
import PlayerScene from '../simuplayer/PlayerScene'
import RoadData from './RoadData'
import SectionData from './SectionData'
import { laneMat } from './LaneData'
import LaneBoundaryData from './LaneBoundaryData'
import LaneLinkData from './LaneLinkData'
import HadmapInfo from './HadmapInfo'
import Utility from './Utility'
import EditorGlobals from './EditorGlobals'
import { MessageBox } from './MessageBox'
import CustomLane from './CustomLane'
import Signlight from './Signlight'
import store from '@/store'
import i18n from '@/locales'

/**
 * 地图类，可以加载地图以及提供一些地图方面的方法
 */
class Hadmap {
  constructor(ct, simuScene) {
    this.name = `hadmap${new Date().toLocaleTimeString()}`

    this.roads = []
    this.sections = []
    this.lanes = []
    this.laneboundaries = []
    this.lanelinks = []

    this.roadsMap = new Map()
    this.sectionsMap = new Map()
    this.lanesMap = new Map()
    this.laneboundariesMap = new Map()
    this.lanelinksMap = new Map()

    this.simuScene = simuScene
    this.isPlayerScene = simuScene instanceof PlayerScene
    this.ct = ct
    this.isLoaded = false
    this.sampleDistance = 10

    this.allHadmapInfoNameMap = new Map()
    this.util = new Utility()

    this.allRoad = null
    this.allRoadGeom = null

    this.allLane = null
    this.allLaneGeom = null

    this.allLaneLink = null
    this.allLaneLinkGeom = null

    this.allMapObject = null

    this.laneGroup = new Group()
    this.highlightLanelinkMap = new Map()

    this.initData()
    simuScene.scene.add(this.laneGroup)

    this.boundingBox = new Box3()

    this.signlights = []
    this.signlightGroup = new Group()
    this.signlightGroup.renderOrder = 101

    this.visible = true

    /**
     * 通用模型加载器，用于加载地图元素
     * @type {MapModelsManager}
     */
    this.mapModelsManager = new MapModelsManager({
      modelPath: 'assets/models',
      imgPath: 'assets/imgs',
    })
  }

  /**
   * 销毁地图
   */
  dispose() {
    this.roads.forEach((r) => {
      r.dispose()
    }, this)

    this.lanes.forEach((l) => {
      l.dispose()
    }, this)

    this.laneboundaries.forEach((l) => {
      l.dispose()
    }, this)

    this.lanelinks.forEach((l) => {
      l.dispose()
    }, this)

    if (this.allRoad) {
      this.allRoadGeom.dispose()
    }

    if (this.allLane) {
      this.allLaneGeom.dispose()
    }

    if (this.allLaneLink) {
      this.allLaneLinkGeom.dispose()
    }

    // 释放空间
    if (this.laneGroup) {
      for (const mesh of this.laneGroup.children) {
        if (mesh?.geometry) {
          mesh.geometry.dispose()
        }
      }
      this.laneGroup = null
    }

    this.roads.length = 0
    this.sections.length = 0
    this.lanes.length = 0
    this.laneboundaries.length = 0
    this.lanelinks.length = 0

    this.roadsMap.clear()
    this.sectionsMap.clear()
    this.lanesMap.clear()
    this.laneboundariesMap.clear()
    this.lanelinksMap.clear()
  }

  /**
   * 根据当前运行环境初始化数据
   */
  initData() {
    if (GlobalConfig.runMode === RunMode.Cloud) {
      Hadmap.mapsdkURL = Utility.composeUrl('/simService/maps/forward/hadmapdata')
      Hadmap.getMapInfoList = Utility.composeUrl('/simService/maps/forward/hadmapinfolist')
    } else if (GlobalConfig.runMode === RunMode.Local) {
      Hadmap.mapsdkURL = 'http://localhost:9000/hadmapdata'
      Hadmap.getMapInfoList = 'http://localhost:9000/hadmapinfolist'
    } else {
      console.error('unknown client run mode!')
    }
  }

  /**
   * 清除地图数据
   */
  clearHadmap() {
    this.removeFromScene(this.simuScene.scene)
    this.dispose()
    this.isLoaded = false
  }

  loaded() {
    return this.isLoaded
  }

  /**
   * 设置采样间隔
   * @param sd
   */
  setSampleDistance(sd) {
    this.sampleDistance = sd
  }

  /**
   * 获取地图数据
   * @return {Promise<boolean>}
   */
  async loadHadmapinfo() {
    let response = null
    try {
      response = await axios({
        method: 'get',
        url: Hadmap.getMapInfoList,
        withCredentials: true,
      })
    } catch (err) {
      const msg = 'get hadmap info list failed.'
      console.error(msg)
      EditorGlobals.signals.promptMessage.dispatch({ type: 'error', msg })
      return false
    }

    const data = response.data

    if (data.code === 0) {
      this.removeHadMaps()

      const maps = data.maps
      try {
        maps.forEach(({ id, name, reflon, reflat, refalt, unreallevelindex }) => {
          const hmi = new HadmapInfo()
          hmi.mapId = id
          hmi.mapName = name
          hmi.mapRefLon = +reflon
          hmi.mapRefLat = +reflat
          hmi.mapRefAlt = +refalt
          hmi.unrealLevelIndex = +unreallevelindex
          this.addHadmap(hmi)
        })

        return true
      } catch (e) {
        console.error('stringify mapsinfo error ', e)
      }
    } else {
      console.warn('get hadmap failed.', data.message)
    }

    return false
  }

  /**
   * 删除所有地图数据
   */
  removeHadMaps() {
    this.allHadmapInfoNameMap.clear()
  }

  /**
   * 添加地图数据
   * @param hadmap
   */
  addHadmap(hadmap) {
    const hm = this.findHadmap(hadmap.mapName)
    if (!hm) {
      this.allHadmapInfoNameMap.set(hadmap.mapName, hadmap)
    }
  }

  /**
   * 查找地图数据
   * @param name
   * @return {any}
   */
  findHadmap(name) {
    return this.allHadmapInfoNameMap.get(name)
  }

  /**
   * 将地图加载到场景
   * @param mapName
   * @param scene
   * @param sceneParser
   * @param extraParams
   * @return {Promise<{lanes: any, roads: any, links: any, mapObjects: any, trafficLights: any}>}
   */
  async loadMapToSceneLocal(mapName, scene, sceneParser, extraParams = {}) {
    const {
      sampleDistance,
      simuScene,
      isPlayerScene,
    } = this
    this.mapName = mapName

    /**
     * 通知后台开始加载地图
     */
    const { data: preloaded } = await axios({
      method: 'get',
      url: Hadmap.mapsdkURL,
      withCredentials: true,
      params: {
        cmd: 'loadhadmap',
        mapname: mapName,
        ...extraParams,
      },
    })
    if (preloaded === true) {
      // road data
      const { data: roads } = await axios({
        method: 'get',
        url: Hadmap.mapsdkURL,
        withCredentials: true,
        params: {
          cmd: 'getallroaddata',
          mapname: mapName,
          sampleDistance,
          ...extraParams,
        },
      })

      if (roads) {
        // get map range
        const res = this.getMapRange(roads)
        if (res.length > 0) {
          this.lonMin = res[0]
          this.lonMax = res[3]
          this.latMin = res[1]
          this.latMax = res[4]
          this.altMin = res[2]
          this.altMax = res[5]
        } else {
          this.lonMin = -1
          this.lonMax = -1
          this.latMin = -1
          this.latMax = -1
          this.altMin = -1
          this.altMax = -1
        }
        this.pushMapData(roads, Hadmap.HadmapElementType.ROADDATA)
      } else {
        const msg = i18n.t('tips.centerDataError')
        MessageBox.promptEditorUIMessage('error', msg)
        throw new Error(msg)
      }

      // lane data
      const { data: lanes } = await axios({
        method: 'get',
        url: Hadmap.mapsdkURL,
        withCredentials: true,
        params: {
          cmd: 'getalllanedata',
          mapname: mapName,
          sampleDistance,
          ...extraParams,
        },
      })
      if (lanes) {
        const roadObj = {}
        lanes.array.forEach(({ rid, sid, id }) => {
          if (!Object.prototype.hasOwnProperty.call(roadObj, rid)) {
            roadObj[rid] = { laneIds: [], sectionIds: [] }
          }
          if (!roadObj[rid].laneIds.includes(id)) roadObj[rid].laneIds.push(id)
          if (!roadObj[rid].sectionIds.includes(sid)) roadObj[rid].sectionIds.push(sid)
        })

        if (sceneParser) {
          sceneParser.roadObj = roadObj
        }

        const Count = this.pushMapData(lanes, Hadmap.HadmapElementType.LANEDATA)
        this.addToScene(Hadmap.HadmapElementType.LANEDATA, scene, Count)
        this.setLaneVisible()
        simuScene.renderScene()
      } else {
        const msg = i18n.t('tips.loadCenterFail')
        MessageBox.promptEditorUIMessage('error', msg)
        throw new Error(msg)
      }

      // lane boundary data
      const { data: laneBoundaries } = await axios({
        method: 'get',
        url: Hadmap.mapsdkURL,
        withCredentials: true,
        params: {
          cmd: 'getalllaneboundarydata',
          mapname: mapName,
          sampleDistance,
          ...extraParams,
        },
      })
      if (laneBoundaries) {
        const Count = this.pushMapData(laneBoundaries, Hadmap.HadmapElementType.LANEBOUNDARYDATA)
        this.addToScene(Hadmap.HadmapElementType.LANEBOUNDARYDATA, scene, Count)
        this.setLaneBoundaryVisible()
        simuScene.renderScene()
      } else {
        const msg = i18n.t('tips.loadBoundaryFailed')
        MessageBox.promptEditorUIMessage('error', msg)
        throw new Error(msg)
      }
      this.generateLaneMesh()

      // 地图元素信息
      const { data: mapObjects } = await axios({
        method: 'get',
        url: Hadmap.mapsdkURL,
        withCredentials: true,
        params: {
          cmd: 'getmapobjectdata',
          mapname: mapName,
          startLon: this.lonMin,
          startLat: this.latMin,
          endLon: this.lonMax,
          endLat: this.latMax,
          sampleDistance,
          ...extraParams,
        },
      })
      try {
        // 展示地图元素之前需要加载一下自定义地图元素定义
        const { data: modelList } = await axios.get('http://127.0.0.1:9000/mapmodel/getmodellist')
        this.allMapObject = this.mapModelsManager.loadAllMapElements(mapObjects.array, {
          ignoreAlt: isPlayerScene,
          mapVersion: mapObjects.version,
          getPositionNormal: this.getPositionNormal.bind(this),
          customModelList: modelList || [],
        })
        scene.add(this.allMapObject)
        this.setMapObjectVisible()
        simuScene.renderScene()
      } catch (e) {
        console.error('加载地图元素失败', e)
      }

      // 车道连接线信息
      const { data: links } = await axios({
        method: 'get',
        url: Hadmap.mapsdkURL,
        withCredentials: true,
        params: {
          cmd: 'getalllanelinkdata',
          mapname: mapName,
          sampleDistance,
          ...extraParams,
        },
      })

      if (links) {
        const Count = this.pushMapData(links, Hadmap.HadmapElementType.LANELINKDATA)
        this.addToScene(Hadmap.HadmapElementType.LANELINKDATA, scene, Count)
        this.setLaneLinkVisible()
        simuScene.renderScene()
      } else {
        console.warn('lane link load failed!')
      }

      // traffic lights，默认信控配置
      const { data: trafficLights } = await axios({
        method: 'get',
        url: Hadmap.mapsdkURL,
        withCredentials: true,
        params: {
          cmd: 'gettrafficlights',
          mapname: mapName,
          ...extraParams,
        },
      })
      trafficLights.activePlan = ''
      sceneParser && sceneParser.parseSignlights(sceneParser, trafficLights)

      this.rearrangeData()

      this.simuScene.renderScene()
      this.isLoaded = true
      EditorGlobals.signals.mapLoaded.dispatch({ success: true })
      PlayerGlobals.signals.mapLoaded.dispatch({ success: true })
      return {
        roads,
        lanes,
        mapObjects,
        links,
        trafficLights,
      }
    } else {
      const msg = i18n.t('tips.mapLoadFailed')
      MessageBox.promptEditorUIMessage('error', msg)
      throw new Error(msg)
    }
  }

  /**
   * 获取经纬度附近车道信息
   * @param lon
   * @param lat
   * @param alt
   * @param searchDistance
   * @return {Promise<unknown>}
   */
  async getNearbyLaneInfo(lon, lat, alt, searchDistance) {
    return axios({
      method: 'get',
      url: Hadmap.mapsdkURL,
      withCredentials: true,
      params: {
        cmd: 'queryinfobypt',
        mapname: this.mapName,
        startLon: lon,
        startLat: lat,
        // startAlt: alt || 0,
        sampleDistance: this.sampleDistance,
        searchDistance,
      },
    }).then(({ data }) => data)
  }

  /**
   * 批量查laneInfo
   * @param {{startLon: string, startLat: string}[]} points
   * @param {number} [searchDistance]
   * @returns {Promise<Array>}
   */
  async batchGetNearbyLaneInfo(points, searchDistance) {
    try {
      const { data } = await axios({
        method: 'post',
        url: Hadmap.mapsdkURL,
        withCredentials: true,
        data: {
          cmd: 'batchqueryinfobypt',
          mapname: this.mapName,
          sampleDistance: this.sampleDistance,
          searchDistance,
          points,
        },
      })
      return data
    } catch (e) {
      console.error(e)
    }
  }

  /**
   * 根据道路坐标获取经纬度信息
   * @param rid
   * @param sid
   * @param lid
   * @param shift
   * @param offset
   * @return {Promise<{err: number}|any>}
   */
  async getLonLat(rid, sid, lid, shift, offset) {
    const { data } = await axios({
      method: 'get',
      url: Hadmap.mapsdkURL,
      withCredentials: true,
      params: {
        cmd: 'querylonlat',
        mapname: this.mapName,
        roadid: rid,
        secid: sid,
        laneid: lid,
        shift,
        offset,
      },
    })
    if (isObject(data) && data.err === 0) {
      return data
    }
    const msg = `getLonLat fail. (${rid}, ${sid}, ${lid})-(${shift}, ${offset})`
    console.error(msg)
    EditorGlobals.signals.promptMessage.dispatch({ type: 'error', msg })
    return { err: 1 }
  }

  /**
   * 根据车道坐标获取经纬度信息
   * @param lon
   * @param lat
   * @param type
   * @param lid
   * @param shift
   * @param offset
   * @return {Promise<{err: number}|any>}
   */
  async getLonLatByPoint(lon, lat, type, lid, shift, offset) {
    const { data } = await axios({
      method: 'get',
      url: Hadmap.mapsdkURL,
      withCredentials: true,
      params: {
        cmd: 'querylonlatbypoint',
        mapname: this.mapName,
        startLon: lon,
        startLat: lat,
        type,
        id: lid,
        shift,
        offset,
      },
    })
    if (isObject(data) && data.err === 0) {
      return data
    } else {
      const msg = `getLonLatByPoint error (${lon}, ${lat})-${type}-(${lid})-(${shift}, ${offset})`
      console.error(msg)
      return { err: 1 }
    }
  }

  /**
   * 批量查询经纬度信息
   * @param {{startLon: number, startLat: number, type: string, id: number, shift: number, offset: number}[]} points
   * @returns {Promise<any>}
   */
  async batchGetLonLatByPoint(points) {
    // startLon, startLat, type, id, shift, offset
    try {
      const { data } = await axios({
        method: 'post',
        url: Hadmap.mapsdkURL,
        withCredentials: true,
        data: {
          cmd: 'batchquerylonlatbypoint',
          mapname: this.mapName,
          sampleDistance: this.sampleDistance,
          points,
        },
      })
      return data
    } catch (e) {
      console.error(e)
    }
  }

  /**
   * 批量计算车道偏移信息
   * @param {{lon: number, lat: number, type: string, id: number, shift: number, offset: number}[]} points
   * @returns {Promise<any>}
   */
  async batchGetLaneShiftPos4(points) {
    const lonlatInfos = await this.batchGetLonLatByPoint(points)
    const laneInfos = await this.batchGetNearbyLaneInfo(lonlatInfos.map(ll => ({ startLon: ll.lon, startLat: ll.lat })))
    return lonlatInfos.map((info, i) => {
      if (info.err !== 0) {
        console.error('getLaneShiftPos4 error!')
        return []
      }
      const yaw = laneInfos[i].err === 0 ? laneInfos[i].yaw * MathUtils.RAD2DEG : 0
      const pos = this.ct.lonlat2xyz(info.lon, info.lat, info.alt)
      return [pos[0], pos[1], pos[2], info.finalshift, info.alt, yaw]
    })
  }

  /**
   * 将地图数据压入数组
   * @param data
   * @param dataType
   * @return {{}}
   */
  pushMapData(data, dataType) {
    const Count = {}
    Count.totalCount = 0

    if (data?.array?.length) {
      let nElementCount = 0

      const nObj = data.array.length
      console.log('data count', nObj)
      for (let i = 0; i < nObj; ++i) {
        const mapElement = this.createMapElement(dataType)
        if (data.array[i].array) {
          nElementCount += data.array[i].array.length
        }

        mapElement.pushData(data.array[i], this.ct)
        this.pushElement(mapElement, data.array[i], dataType)

        if (dataType === Hadmap.HadmapElementType.LANEBOUNDARYDATA) {
          mapElement.composeSceneData()
        } else {
          mapElement.composeSceneData(this.getMapElementMaterial(dataType))
        }
      }

      Count.totalCount = nElementCount
    }

    return Count
  }

  /**
   * 获取地图范围
   * @param data
   * @return {number[]|*[]}
   */
  getMapRange(data) {
    if (data?.array) {
      const min = [Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY]
      const max = [Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY]
      const nObj = data.array.length
      for (let i = 0; i < nObj; ++i) {
        if (data.array[i].array) {
          const len = data.array[i].array.length
          for (let j = 0; j < len; ++j) {
            if (+data.array[i].array[j].x > max[0]) {
              max[0] = +data.array[i].array[j].x
            }
            if (+data.array[i].array[j].x < min[0]) {
              min[0] = +data.array[i].array[j].x
            }

            if (+data.array[i].array[j].y > max[1]) {
              max[1] = +data.array[i].array[j].y
            }
            if (+data.array[i].array[j].y < min[1]) {
              min[1] = +data.array[i].array[j].y
            }

            if (+data.array[i].array[j].z > max[2]) {
              max[2] = +data.array[i].array[j].z
            }
            if (+data.array[i].array[j].z < min[2]) {
              min[2] = +data.array[i].array[j].z
            }
          }
        }
      }
      return [min[0], min[1], min[2], max[0], max[1], max[2]]
    }

    return []
  }

  /**
   * 将地图元素添加到场景
   * @param type
   * @param scene
   * @param Count
   */
  addToScene(type, scene, Count) {
    // 内部标志线 count 有可能是 0
    if (Count.totalCount < 1 && type !== Hadmap.HadmapElementType.MAPOBJECTDATA) {
      return
    }

    if (GlobalConfig.MergeGeom()) {
      const nPointCount = Count.totalCount
      let mat = laneMat
      let dataArr
      if (type === Hadmap.HadmapElementType.ROADDATA) {
        dataArr = this.roads
        mat = RoadData.mat

        const positions = Array.from({ length: nPointCount * 3 })
        // this.allRoadGeom = new BufferGeometry()
        // this.allRoadGeom.setAttribute('position', new Float32BufferAttribute(positions, 3))

        // const nCount = dataArr.length
        // let offset = 0
        // for (let i = 0; i < nCount; ++i) {
        //   if (dataArr[i].geom) {
        //     this.allRoadGeom.merge(dataArr[i].geom, offset)
        //     offset += dataArr[i].data.length
        //   }
        // }
        this.allRoadGeom = mergeGeometries(dataArr.filter(d => !!d.geom).map(d => d.geom))
        this.allRoad = new LineSegments(this.allRoadGeom, mat)
        scene.add(this.allRoad)
        this.allRoadGeom.userData._type = 'roadData'
      } else if (type === Hadmap.HadmapElementType.LANEDATA) {
        dataArr = this.lanes
        mat = laneMat

        const positions = Array.from({ length: nPointCount * 3 })
        // this.allLaneGeom = new BufferGeometry()
        // this.allLaneGeom.setAttribute('position', new Float32BufferAttribute(positions, 3))

        // const nCount = dataArr.length
        // let offset = 0

        // for (let i = 0; i < nCount; ++i) {
        //   if (dataArr[i].geom) {
        //     this.allLaneGeom.merge(dataArr[i].geom, offset)
        //     offset += dataArr[i].data.length
        //   }
        // }

        this.allLaneGeom = mergeGeometries(dataArr.filter(d => !!d.geom).map(d => d.geom))

        this.allLane = new LineSegments(this.allLaneGeom, mat)
        scene.add(this.allLane)
        this.allLaneGeom.userData._type = 'laneData'
      } else if (type === Hadmap.HadmapElementType.LANEBOUNDARYDATA) { // 因为要区分颜色和样式，所以不在merge了
        dataArr = this.laneboundaries
        dataArr.forEach((lbd) => {
          lbd.addToScene(scene)
        })
      } else if (type === Hadmap.HadmapElementType.LANELINKDATA) {
        dataArr = this.lanelinks
        mat = LaneLinkData.mat.clone()
        mat.depthWrite = false

        const positions = Array.from({ length: nPointCount * 3 })
        // this.allLaneLinkGeom = new BufferGeometry()
        // this.allLaneLinkGeom.setAttribute('position', new Float32BufferAttribute(positions, 3))

        // const nCount = dataArr.length
        // let offset = 0
        // for (let i = 0; i < nCount; ++i) {
        //   if (dataArr[i].geom) {
        //     this.allLaneLinkGeom.merge(dataArr[i].geom, offset)
        //     offset += dataArr[i].data.length
        //   }
        // }
        this.allLaneLinkGeom = mergeGeometries(dataArr.filter(d => !!d.geom).map(d => d.geom))

        this.allLaneLink = new LineSegments(this.allLaneLinkGeom, mat)
        // this.allLaneLink.renderOrder = RenderOrder.LANE
        scene.add(this.allLaneLink)
        this.allLaneLinkGeom.userData._type = 'lanelinkData'
      } else if (type === Hadmap.HadmapElementType.MAPOBJECTDATA) {
      } else {
        console.error('unknown hadmap data type')
      }
    } else {
      let dataArr
      if (type === Hadmap.HadmapElementType.ROADDATA) {
        dataArr = this.roads
      } else if (type === Hadmap.HadmapElementType.LANEDATA) {
        dataArr = this.lanes
      } else if (type === Hadmap.HadmapElementType.LANEBOUNDARYDATA) {
        dataArr = this.laneboundaries
      } else if (type === Hadmap.HadmapElementType.LANELINKDATA) {
        dataArr = this.lanelinks
      } else if (type === Hadmap.HadmapElementType.MAPOBJECTDATA) {
        dataArr = []
      } else {
        console.error('unknown hadmap data type')
      }

      dataArr.forEach((data) => {
        data.addToScene(scene)
      })
    }
  }

  /**
   * 将地图从场景中移除
   * @param scene
   */
  removeFromScene(scene) {
    if (this.allRoad) {
      scene.remove(this.allRoad)
    }
    if (this.allLane) {
      scene.remove(this.allLane)
    }
    if (this.allLaneLink) {
      scene.remove(this.allLaneLink)
    }
    if (this.allMapObject) {
      scene.remove(this.allMapObject)
    }
    if (this.laneGroup) {
      scene.remove(this.laneGroup)
    }
    this.laneboundaries.forEach(lb => lb.removeFromScene(scene))
  }

  /**
   * 创建地图元素
   * @param type
   * @return {undefined|LaneLinkData|LaneBoundaryData|RoadData|CustomLane}
   */
  createMapElement(type) {
    if (type === Hadmap.HadmapElementType.ROADDATA) {
      return new RoadData()
    } else if (type === Hadmap.HadmapElementType.LANEDATA) {
      return new CustomLane()
    } else if (type === Hadmap.HadmapElementType.LANEBOUNDARYDATA) {
      return new LaneBoundaryData()
    } else if (type === Hadmap.HadmapElementType.LANELINKDATA) {
      return new LaneLinkData()
    }

    return undefined
  }

  /**
   * 获取地图元素材质
   * @param type
   * @return {undefined|LineDashedMaterial|LineBasicMaterial}
   */
  getMapElementMaterial(type) {
    if (type === Hadmap.HadmapElementType.ROADDATA) {
      return RoadData.mat
    } else if (type === Hadmap.HadmapElementType.LANEDATA) {
      return laneMat
    } else if (type === Hadmap.HadmapElementType.LANELINKDATA) {
      return LaneLinkData.mat
    }

    return undefined
  }

  /**
   * 将地图元素添加到数组
   * @param data
   * @param rawData
   * @param dataType
   */
  pushElement(data, rawData, dataType) {
    if (dataType === Hadmap.HadmapElementType.ROADDATA) {
      const tmp = this.roadsMap.get(`${rawData.roadid}`)
      if (tmp === undefined) {
        const nLen = this.roads.length
        data.parse(rawData)
        this.roads[nLen] = data
        this.roadsMap.set(`${data.id}`, data)
      } else {
        console.debug('road repeat: ', data.id)
      }
    } else if (dataType === Hadmap.HadmapElementType.LANEDATA) {
      const key = { rid: `${rawData.rid}`, sid: `${rawData.sid}`, lid: `${rawData.id}` }
      const ids = JSON.stringify(key)
      const tmp = this.lanesMap.get(ids)
      if (tmp === undefined) {
        const nLen = this.lanes.length
        data.parse(rawData)
        this.lanes[nLen] = data
        this.lanesMap.set(ids, data)
      } else {
        console.debug('lane repeat: ', ids)
      }
    } else if (dataType === Hadmap.HadmapElementType.LANEBOUNDARYDATA) {
      const tmp = this.laneboundariesMap.get(`${rawData.id}`)
      if (tmp === undefined) {
        data.parse(rawData)
        const nLen = this.laneboundaries.length
        this.laneboundaries[nLen] = data
        this.laneboundariesMap.set(`${data.id}`, data)
      } else {
        console.debug('laneboundaries repeat: ', data.id)
      }
    } else if (dataType === Hadmap.HadmapElementType.LANELINKDATA) {
      const tmp = this.lanelinksMap.get(`${rawData.id}`)
      if (tmp === undefined) {
        data.parse(rawData)
        this.lanelinks.push(data)
        this.lanelinksMap.set(`${data.id}`, data)
      } else {
        console.debug('lanelink repeat: ', data.id)
      }
    } else if (dataType === Hadmap.HadmapElementType.MAPOBJECTDATA) {
      // do nothing
    } else {
      console.debug('error map element')
    }
  }

  /**
   * 重新排列数据
   */
  rearrangeData() {
    // section lane relationship
    let len = this.lanes.length
    for (let i = 0; i < len; ++i) {
      const rid = `${this.lanes[i].roadId}`
      const sid = `${this.lanes[i].sectionId}`

      const slObj = { rid, sid }
      const slKey = JSON.stringify(slObj)
      if (!this.sectionsMap.get(slKey)) {
        const sec = new SectionData()
        sec.roadId = rid
        sec.sectionId = sid
        this.sections.push(sec)
        this.sectionsMap.set(slKey, sec)
      }

      this.sectionsMap.get(slKey).lanes.push(this.lanes[i])
    }

    // road section relationship
    len = this.sections.length
    for (let i = 0; i < len; ++i) {
      const rid = `${this.sections[i].roadId}`
      const rd = this.roadsMap.get(rid)
      if (!rd) {
        console.assert(false, 'unknown road id')
      } else {
        rd.sections.push(this.sections[i])
      }
    }
  }

  /**
   * 生成车道线Mesh
   */
  generateLaneMesh() {
    const len = this.lanes.length
    for (let i = 0; i < len; ++i) {
      const mesh = this.lanes[i].generateMesh(this, this.simuScene.scene)
      if (!this.laneGroup) {
        this.laneGroup = new Group()
        this.simuScene.scene.add(this.laneGroup)
      }
      if (mesh) {
        this.laneGroup.add(mesh)
      }
    }
    this.setLaneMeshVisible()
    this.laneGroup.updateMatrixWorld()
  }

  /**
   * 获取section
   * @param roadId
   * @param sectionId
   * @return {any}
   */
  getSection(roadId, sectionId) {
    roadId = `${roadId}`
    sectionId = `${sectionId}`
    const rsObj = { rid: roadId, sid: sectionId }
    const rsKey = JSON.stringify(rsObj)
    return this.sectionsMap.get(rsKey)
  }

  /**
   * 获取lane
   * @param roadId
   * @param sectionId
   * @param laneId
   * @return {any}
   */
  getLane(roadId, sectionId, laneId) {
    roadId = `${roadId}`
    sectionId = `${sectionId}`
    laneId = `${laneId}`
    const key = { rid: roadId, sid: sectionId, lid: laneId }
    const ids = JSON.stringify(key)
    return this.lanesMap.get(ids)
  }

  /**
   * 获取road
   * @param roadId
   * @return {any}
   */
  getRoad(roadId) {
    roadId = `${roadId}`
    return this.roadsMap.get(roadId)
  }

  /**
   * 获取lanelink
   * @param lanelinkId
   * @return {any}
   */
  getLanelink(lanelinkId) {
    return this.lanelinksMap.get(`${lanelinkId}`)
  }

  /**
   * 获取车道线边线
   * @param bid
   * @return {any}
   */
  getLaneBoundary(bid) {
    return this.laneboundariesMap.get(`${bid}`)
  }

  /**
   * 设置road的显示状态
   */
  setRoadVisible() {
    // 只有播放的时候才受store变量的控制
    const show = this.isPlayerScene ? store.state.playerViewConfig.mapViewConfig.road : true
    if (this.allRoad) {
      this.allRoad.visible = show
    }
  }

  /**
   * 设置lane的显示状态
   */
  setLaneVisible() {
    const show = this.isPlayerScene ? store.state.playerViewConfig.mapViewConfig.lane : true
    if (this.allLane) {
      this.allLane.visible = show
    }
  }

  /**
   * 设置lane mesh的显示状态
   */
  setLaneMeshVisible() {
    const show = this.isPlayerScene ? store.state.playerViewConfig.mapViewConfig.roadmesh : true
    if (this.laneGroup) {
      this.laneGroup.visible = show
    }
  }

  /**
   * 设置车道分界线的显示状态
   */
  setLaneBoundaryVisible() {
    const show = this.isPlayerScene ? store.state.playerViewConfig.mapViewConfig.laneboundary : true
    this.laneboundaries.forEach((lb) => {
      if (lb.mesh) {
        lb.mesh.visible = show
      }
    })
  }

  /**
   * 设置lanelink显示状态
   */
  setLaneLinkVisible() {
    const show = this.isPlayerScene ? store.state.playerViewConfig.mapViewConfig.lanelink : true
    if (this.allLaneLink) {
      this.allLaneLink.visible = show
    }
  }

  /**
   * 设置mapobject显示状态
   */
  setMapObjectVisible() {
    const show = this.isPlayerScene ? store.state.playerViewConfig.mapViewConfig.mapobject : true
    if (this.allMapObject) {
      this.allMapObject.visible = show
    }
  }

  /**
   * 设置所有显示状态
   */
  setVisible() {
    this.setRoadVisible()
    this.setLaneVisible()
    this.setLaneMeshVisible()
    this.setLaneBoundaryVisible()
    this.setLaneLinkVisible()
    this.setMapObjectVisible()
  }

  /**
   * 获取地图的包围盒
   * @return {Box3}
   */
  getBoundingBox() {
    const arr = [this.lonMin, this.latMin, this.altMin, this.lonMax, this.latMax, this.altMax]
    this.boundingBox.setFromArray(arr)
    return this.boundingBox
  }

  /**
   * 显示车道的ID
   * @param xNor
   * @param yNor
   * @param camera
   * @return {Promise<void>}
   */
  async showLaneId(xNor, yNor, camera) {
    try {
      const scene = this.simuScene.scene
      const pickedInfo = this.getPointerPosInLaneLine(xNor, yNor, camera)
      if (!pickedInfo) {
        if (currentLaneText) {
          scene.remove(currentLaneText)
          currentLaneText.geometry.dispose()
          currentLaneText = null
        }
        lastPickedLaneLine = null
        return
      }
      const { point, lane } = pickedInfo
      if (lane !== lastPickedLaneLine) {
        if (currentLaneText) {
          scene.remove(currentLaneText)
          currentLaneText.geometry.dispose()
        }
        currentLaneText = await getTextMesh(lane.getDescInfo())
        scene.add(currentLaneText)
        currentLaneText.position.copy(point)
      } else {
        if (currentLaneText) {
          currentLaneText.position.copy(point)
        }
      }

      if (currentLaneText) {
        currentLaneText.updateMatrixWorld(true)
      }

      lastPickedLaneLine = lane
    } catch (error) {
      console.warn(error)
    }
  }

  /**
   * 获取鼠标点击位置在车道线上的信息
   * @param xNor
   * @param yNor
   * @param camera
   * @return {{point: Vector3, lane: *}|null}
   */
  getPointerPosInLaneLine(xNor, yNor, camera) {
    const lanes = this.lanes
    if (!lanes) {
      return null
    }
    raycaster.setFromCamera({ x: xNor, y: yNor }, camera)

    for (let i = 0; i < lanes.length; i++) {
      meshLineInter.geometry = lanes[i].geom
      if (!meshLineInter.geometry) {
        continue
      }
      const intersects = raycaster.intersectObject(meshLineInter, true)

      if (intersects.length > 0) {
        return {
          point: intersects[0].point,
          lane: lanes[i],
        }
      }
    }

    const links = this.lanelinks
    if (!links) {
      return null
    }

    for (let i = 0; i < links.length; i++) {
      meshLineInter.geometry = links[i].geom
      if (!meshLineInter.geometry) {
        continue
      }
      const intersects = raycaster.intersectObject(meshLineInter, true)

      if (intersects.length > 0) {
        return {
          point: intersects[0].point,
          lane: links[i],
        }
      }
    }
    return null
  }

  /**
   * 显示所有交通灯
   * @param signlights
   * @param routes
   * @return {Promise<void>}
   */
  async showSignlights({ signlights, routes }) {
    if (!signlights.length) return
    const points = routes.map((r) => {
      [r.startLon, r.startLat] = r.start.split(',')
      r.endPosArr = [{ lon: r.startLon, lat: r.startLat }]
      return { startLon: r.startLon, startLat: r.startLat }
    })
    const infos = await this.batchGetNearbyLaneInfo(points)
    routes.forEach((r, i) => {
      const data = infos[i]
      if (data?.err === 0) {
        r.yaw = data.yaw
        if (data.type === 'lane') {
          r.roadId = data.rid
          r.sectionId = data.sid
          r.laneId = data.lid
          r.elemType = MapElementType.LANE
          r.startAlt = data.alt
        } else if (data.type === 'lanelink') {
          r.lanelinkId = data.llid
          r.elemType = MapElementType.LANELINK
          r.startAlt = data.alt
          const lanelink = this.getLanelink(r.lanelinkId)
          if (lanelink) {
            ({ fromRoadId: r.roadId, fromSectionId: r.sectionId, fromLaneId: r.laneId } = lanelink)
          } else {
            console.warn(`语义灯关联的route没有正确查询到lane, routeId: ${r.id}`)
          }
        }
      } else {
        const msg = `route: ${r.id} (lon: ${r.startLon} lat: ${r.startLat}) no lane`
        console.error('queryinfobypt error:', msg)
        MessageBox.promptEditorUIMessage('error', msg)
      }
    })
    this.signlights.length = 0
    await Promise.all(signlights.map(async (s) => {
      const signlight = new Signlight()
      signlight.copyNoModel(s)
      const arrowObjects = await this.simuScene.modelsManager.loadSignArrowModels()
      signlight.createModel(arrowObjects)
      signlight.addToScene(this.signlightGroup)
      let startOffset = ({ L: -1.75, R: 1.75, T: 0, L0: -3.5, R0: 3.5 })[signlight.phase]
      const route = routes.find(r => `${r.id}` === `${signlight.routeId}`)
      if (route) {
        this.signlights.push(signlight)
        const { laneId, roadId, sectionId } = route
        signlight.roadId = roadId
        signlight.sectionId = sectionId
        signlight.laneId = laneId
        // 支持双向道路之后，对向车道（id>0的）要取shift=0
        const shift = laneId > 0 ? 0 : Number.MAX_SAFE_INTEGER
        if (laneId > 0) {
          startOffset *= -1
        }
        const pos = this.simuScene.getLaneShiftPos(
          route.roadId,
          route.sectionId,
          route.laneId,
          shift,
          startOffset,
        )
        signlight.model.position.set(pos[0], pos[1], pos[2])
        signlight.startShift = pos[4] || 0
        if (signlight.startShift < 0) {
          signlight.startShift = 0
        }
        // const dir = this.simuScene.getLaneShiftDir(roadId, sectionId, laneId, shift)
        const dir = yawToVector3(route.yaw)
        signlight.dir.copy(dir)
        signlight.updateDisplay()
      } else {
        console.error(`未找到 signlight：${signlight.id} 的route`)
      }
    }))
    this.simuScene.scene.add(this.signlightGroup)
    this.simuScene.renderScene()
  }

  /**
   * 隐藏所有信号灯
   */
  hideAllSignlights() {
    this.signlights.forEach((signlight) => {
      signlight.removeFromScene(this.signlightGroup)
    })
    this.signlights.length = 0
  }

  /**
   * 更改灯颜色
   * @param {{id:string, color: 'red'|'yellow'|'green'|'gray', disabled: false}[]} options
   */
  changeSignlightColors(options) {
    options.forEach((opt) => {
      const signlight = this.signlights.find(s => s.id === opt.id)
      if (signlight) {
        signlight.setStatus({ color: opt.color, disabled: opt.disabled })
      } else {
        console.warn(`未找到 signlight，id: ${opt.id}`)
      }
    })
  }

  /**
   * 获取位置的法向量
   * @param position
   * @param roadId
   * @return {Vector3}
   */
  getPositionNormal(position, roadId) {
    let normal = new Vector3(0, 0, 1)
    const rayDir = new Vector3(0, 0, -1)
    const farPoint = new Vector3()
    farPoint.copy(position).setZ(10000)
    const rayCaster = new Raycaster(farPoint, rayDir)
    // 从road找点可能刚好不在路面mesh上，这里采用lane的
    let roadMeshes
    let lanes
    if (roadId !== undefined) {
      lanes = this.lanes.filter(l => l.roadId === roadId)
      roadMeshes = lanes.map(lane => lane.laneMesh)
    } else {
      roadMeshes = this.laneGroup.children
    }
    const intersections = rayCaster.intersectObjects(roadMeshes || [], true)
    if (intersections.length) {
      [{ face: { normal } }] = intersections
    } else {
      // 不知为何rayCaster的ray数据全不一样了，这里重新构建一个ray
      const ray = new Ray(farPoint, rayDir)
      if (lanes?.length) {
        const points = []
        lanes.forEach((l) => {
          const data = l.data || []
          points.push(...data.map(p => new Vector3(p.x, p.y, p.z)))
        })
        // 找到这条道路离射线最近的点
        const { point } = points.reduce((prev, vec) => {
          const { distance } = prev
          const vector = new Vector3(vec.x, vec.y, vec.z)
          // 其实是距离的平方，为了节省性能
          const currentDistance = ray.distanceSqToPoint(vector)
          if (currentDistance < distance) {
            return {
              distance: currentDistance,
              point: vector,
            }
          } else {
            return prev
          }
        }, { distance: Number.POSITIVE_INFINITY, point: null })
        if (!point) return normal
        const normalOnRoad = this.getPositionNormal(point)
        normal = normalOnRoad
      } else {
        normal = new Vector3(0, 0, 1)
      }
    }
    return normal
  }
}

/**
 * show roadId usage
 */
const raycaster = new Raycaster()
const meshLineInter = new LineSegments()
let lastPickedLaneLine = null
let currentLaneText = null

Hadmap.HadmapElementType = {
  ROADDATA: 0,
  LANEDATA: 1,
  LANEBOUNDARYDATA: 2,
  LANELINKDATA: 3,
  MAPOBJECTDATA: 4,
  LANEBOUNDARYDATA_SOLID: 5,
  LANEBOUNDARYDATA_BROKEN: 6,
}

export default Hadmap
