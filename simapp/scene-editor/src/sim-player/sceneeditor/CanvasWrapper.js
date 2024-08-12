import { Group } from 'three'
import { InterfaceCategory } from 'models-manager/src/catalogs/class.ts'
import {
  IgnoreHeight,
  MapElementType,
  OffsetConst,
  OperationMode,
  RunMode,
  SceneState,
} from '../common/Constant'
import GlobalConfig from '../common/Config'
import { LogInfo } from '../api/recordLogs'
import Profile from '../common/Profile'
import EditorGlobals from './EditorGlobals'
import { MessageBox } from './MessageBox'
import SimuScene from './SimuScene'
import i18n from '@/locales'
import { getModelProperty } from '@/common/utils'

class CanvasWrapper {
  /**
   * 初始化容器，包括创建simuScene，绑定元素节点事件，处理一些初始数据等
   * @param ele
   * @return {Promise<boolean>}
   */
  async init ({ ele }) {
    let canvasWrapper
    if (!ele) {
      canvasWrapper = document.querySelector('#WebGL-output')
    } else {
      canvasWrapper = ele
    }

    if (!canvasWrapper) {
      const msg = 'no container element!'
      EditorGlobals.signals.promptMessage.dispatch({ type: 'error', msg })
      return
    }

    GlobalConfig.runMode = RunMode.Local
    GlobalConfig.setIgnoreHeight(IgnoreHeight.No)
    GlobalConfig.setIsPlaying(false)

    // 初始化消息
    this.canvasWrapper = canvasWrapper

    if (this.simuScene) {
      this.simuScene.finialize(canvasWrapper)
    }

    const simuScene = new SimuScene({ wrapper: canvasWrapper })
    window.simuScene = simuScene
    this.simuScene = simuScene

    await simuScene.init(canvasWrapper)

    const canvasWrapperParentNode = canvasWrapper.parentNode
    canvasWrapperParentNode.addEventListener('mousedown', (event) => {
      if (event.button === 0) {
        window.simuScene.pickObject(event)
      }
    })

    if (simuScene.Config().useTopCamera) {
      simuScene.change2TopCamera(false)

      canvasWrapperParentNode.addEventListener('mousedown', this.onDocumentButtonDown.bind(this))
      canvasWrapperParentNode.addEventListener('mouseup', this.onDocumentButtonUp.bind(this))
      canvasWrapperParentNode.addEventListener('mousemove', this.onDocumentMouseMove.bind(this))
      canvasWrapperParentNode.addEventListener('contextmenu', this.onContextMenu.bind(this))
      window.addEventListener('error', this.onError.bind(this), false)
      canvasWrapperParentNode.addEventListener('dblclick', this.onDblclick.bind(this))
    }

    document.addEventListener('keydown', this.onKeyDown.bind(this))
    document.addEventListener('keyup', this.onKeyUp.bind(this))
    window.addEventListener('blur', this.onBlur.bind(this))

    this.renderSimScene()
    EditorGlobals.signals.measurementChanged.dispatch({ type: 'reset', data: [] })

    await this.simuScene.hadmap.loadHadmapinfo()

    return true
  }

  /**
   * 重新初始化容器，和init相比少了处理elem元素想关的逻辑，多了一些初始数据的处理
   * @param elem
   * @return {Promise<boolean>}
   */
  async reinit (elem) {
    let canvasWrapper
    if (!elem) {
      canvasWrapper = document.querySelector('#WebGL-output')
    } else {
      canvasWrapper = elem
    }

    if (!canvasWrapper) {
      const msg = 'no container element!'
      EditorGlobals.signals.promptMessage.dispatch({ type: 'error', msg })
      return
    }

    GlobalConfig.runMode = RunMode.Local
    GlobalConfig.setIgnoreHeight(IgnoreHeight.No)
    GlobalConfig.setIsPlaying(false)

    this.canvasWrapper = canvasWrapper
    if (this.simuScene) {
      this.simuScene.finialize(this.canvasWrapper)
      this.simuScene.dispose()
    }
    const simuScene = new SimuScene({ wrapper: canvasWrapper })
    window.simuScene = simuScene
    this.simuScene = simuScene
    try {
      await simuScene.init(canvasWrapper)
    } catch (err) {
      EditorGlobals.signals.promptMessage.dispatch({ type: 'error', msg: err.message })
      return
    }

    if (simuScene.Config().useTopCamera) {
      simuScene.change2TopCamera()
    }

    // 重置场景消息
    EditorGlobals.signals.resetScene.dispatch()

    this.simuScene.addDefaultAccData()
    this.simuScene.addDefaultMergeData()

    this.renderSimScene()
    EditorGlobals.signals.measurementChanged.dispatch({ type: 'reset', data: [] })

    await this.simuScene.hadmap.loadHadmapinfo()

    return true
  }

  renderSimScene () {
    this.simuScene.renderScene()
  }

  /**
   * 设置画布size
   * @param width
   * @param height
   */
  resize (width, height) {
    if (!this.simuScene) return
    const {
      simuScene: {
        curCamera,
        // cameraTop,
        // topSceneRange,
        // topSceneZoomFactor,
        renderer,
      },
      simuScene,
    } = this
    if (height === 0) {
      height = 1
    }

    simuScene.aspect = width / height
    curCamera.aspect = width / height

    curCamera.updateProjectionMatrix()
    renderer.setSize(width, height)

    this.renderSimScene()
    if (this.simuScene.sceneParser.measurements.length > 0) {
      this.simuScene.calculateMeasurementResult()
    }
  }

  /**
   * 对simuScene相关事件的包装，下同
   * @param event
   */
  onDocumentButtonDown (event) {
    this.simuScene.onTopCameraMouseButtonDown(event)
    this.simuScene.renderScene()
  }

  onError (event) {
    const str = JSON.stringify(event)
    LogInfo(str)
  }

  onDocumentButtonUp (event) {
    this.simuScene.onTopCameraMouseButtonUp(event)
    this.simuScene.renderScene()
  }

  onDocumentMouseMove (event) {
    this.simuScene.onTopCameraMouseMove(event)
    this.simuScene.renderScene()
  }

  onContextMenu (event) {
    if (!this.isUsingTopCamera()) return
    event.preventDefault()
  }

  onKeyDown (event) {
    this.simuScene.onKeyDown(event)

    // alt key
    if (event.keyCode === 18) {
      if (this.simuScene.editorControls) {
        this.simuScene.editorControls.enableRotate = true
        this.simuScene.editorControls.mouseButtons.LEFT = 0 // rotate
      }
    }
  }

  onKeyUp (event) {
    if (event.keyCode === 18) {
      if (this.simuScene.editorControls) {
        this.simuScene.editorControls.enableRotate = Profile.operationMode === OperationMode.ROTATE
        if (this.simuScene.operationMode === 2) { // pan
          this.simuScene.editorControls.mouseButtons.LEFT = 2 // pan
        }
      }
    }
  }

  onBlur () {
    if (this.simuScene.editorControls) {
      this.simuScene.editorControls.enableRotate = Profile.operationMode === OperationMode.ROTATE
      if (this.simuScene.operationMode === 2) { // pan
        this.simuScene.editorControls.mouseButtons.LEFT = 2 // pan
      }
    }
  }

  onDblclick (event) {
    if (this.simuScene) {
      this.simuScene.onDbclick(event)
    }
  }

  /**
   * 加载一个场景
   * @param sceneName
   * @param elem
   * @return {Promise<void>}
   */
  async openScene (sceneName, elem) {
    await this.reinit(elem)
    await this.simuScene.loadScene(sceneName)
  }

  /**
   * 校验一个场景是否可以保存
   * @return {{result: boolean, message: string}}
   */
  canSceneSave () {
    return this.simuScene.sceneParser.canSceneSave()
  }

  /**
   * 保存场景
   * @param id
   * @param sceneName
   * @param ext
   * @return {*}
   */
  saveScene (id, sceneName, ext) {
    return this.simuScene.saveScene(id, sceneName, ext)
  }

  /**
   * 获取L3状态机原始数据
   * @return {*}
   */
  getL3StateMachineRawData () {
    return this.simuScene.getL3StateMachineRawData()
  }

  /**
   * 设置L3状态机数据
   * @return {*}
   */
  setL3StateMachineData (states) {
    this.simuScene.setL3StateMachineData(states)
  }

  /**
   * 获取L3状态机数据
   * @return {*}
   */
  getL3StateMachineData () {
    return this.simuScene.getL3StateMachineData()
  }

  /**
   * 获取场景的批量生成信息（场景可能是被批量生成的，一些泛化参数）
   * @return {*}
   */
  getGenerateInfo () {
    return this.simuScene.getGenerateInfo()
  }

  /**
   * 设置场景的传感器数据
   * @return {*}
   */
  setSensorInfo (sensorInfo) {
    this.simuScene.setSensorInfo(sensorInfo)
  }

  /**
   * 获取场景的传感器数据
   * @return {*}
   */
  getSensorInfo () {
    return this.simuScene.getSensorInfo()
  }

  /**
   * 获取场景视图数据，如factor视角远近
   * @return {*}
   */
  getUIStateInfo () {
    return this.simuScene.getUIStateInfo()
  }

  /**
   * 获取所有道路信息
   * @return {*}
   */
  getRoadobj () {
    return this.simuScene.getRoadobj()
  }

  /**
   * 获取场景事件信息
   * @return {*}
   */
  getSceneevents () {
    return this.simuScene.getSceneevents()
  }

  /**
   * 设置场景的环境信息
   * @return {*}
   */
  setEnvironmentInfo (environmentInfo) {
    this.simuScene.setEnvironmentInfo(environmentInfo)
  }

  /**
   * 获取场景的环境信息
   * @return {*}
   */
  getEnvironmentInfo () {
    return this.simuScene.getEnvironmentInfo()
  }

  /**
   * 设置场景的grading信息
   * @return {*}
   */
  setGradingInfo (gradingInfo) {
    this.simuScene.setGradingInfo(gradingInfo)
  }

  /**
   * 获取场景的grading数据
   * @return {*}
   */
  getGradingInfo () {
    return this.simuScene.getGradingInfo()
  }

  /**
   * 设置场景的摘要
   * @return {*}
   */
  setSceneDesc (info) {
    try {
      this.simuScene.sceneParser.setSceneDesc(info)
    } catch (error) {
      console.log('setSceneDesc failed: ', error)
    }
  }

  /**
   * 设置场景交通流配置方案
   * @return {*}
   */
  applyTrafficAI (config) {
    this.simuScene.sceneParser.applyTrafficAI(config)
  }

  /**
   * 读取场景交通流配置方案
   * @return {*}
   */
  getTrafficAIConfig () {
    return this.simuScene.sceneParser.getTrafficAIConfig()
  }

  /**
   * 设置场景交通流激进程度
   * @return {*}
   */
  applyTrafficNormal () {
    this.simuScene.sceneParser.applyAggress(0, -1)
  }

  /**
   * 新建场景时加载地图
   * @return {*}
   */
  async loadHadmapOnNewScene (mapName) {
    await this.simuScene.loadHadmap(mapName, 'desc')
    this.simuScene.locateCameraOverMap()
  }

  /**
   * 设置场景名
   * @return {*}
   */
  setSceneName (sceneName) {
    this.simuScene.setSceneName(sceneName)
  }

  /**
   * 根据ID和type获取场景元素
   * @return {*}
   */
  getObjectInfoByID (type, id) {
    let data = {}
    // 对象保存的字段名跟前端定义的不一样，需要将值对应过去
    switch (type) {
      case 'acc': {
        const acc = this.simuScene.sceneParser.findAccInMap(id)
        data = {
          id: acc.id,
          accs: acc.nodes.slice(0),
        }
        break
      }
      case 'merge': {
        const merge = this.simuScene.sceneParser.findMergeInMap(id)
        data = {
          id: merge.id,
          merges: merge.merges.slice(0),
        }
        break
      }
      case 'route': {
        const route = this.simuScene.sceneParser.findRouteInMap(id)
        data = {
          id: route.id,
          remarks: route.remarks,
          startLng: route.startLon,
          startLat: route.startLat,
          endLng: route.endLon,
          endLat: route.endLat,
          routeId: route.roadId,
          sectionID: route.sectionId,
        }
        data.defineType = route.type === 'start_end' ? '1' : '2'
        if (data.endLat < -90 || data.endLat > 90) {
          data.endLat = ''
        }
        if (data.endLon < -180 || data.endLon > 180) {
          data.endLon = ''
        }
        break
      }
      case 'car': {
        data = this.simuScene.sceneParser.findVehicleInMap(id).carFormData()
        break
      }
      case 'pedestrian':
      case 'animal':
      case 'bike':
      case 'moto':
      case 'machine':
        data = this.simuScene.sceneParser.findPedestrianInMap(id).pedestrianFormData()
        break
      case 'obstacle':
        data = this.simuScene.sceneParser.findObstacleInMap(id).obstacleFormData()
        break
      case 'signlight':
        data = this.simuScene.sceneParser.findSignlightInMap(id).signlightFormData()
        break
      case 'planner':
        data = this.simuScene.sceneParser.findPlannerInArr(id).plannerFormData()
        break
      case 'rosbag':
        if (this.simuScene.sceneParser.rosbag) {
          data.path = this.simuScene.sceneParser.rosbag.path
        } else {
          return undefined
        }
        break
    }

    data.type = type
    data.label = type + id

    return data
  }

  /**
   * 根据ID和type获取场景元素
   * @return {*}
   */
  getObjectsInfoByIDs (type, IDs) {
    if (IDs?.length) {
      return IDs.map(id => this.getObjectInfoByID(type, id)).filter(obj => !!obj)
    }
    return []
  }

  /**
   * 根据类型获取场景元素
   * @param type
   * @return {*|*[]}
   */
  getObjectsInfoByType (type) {
    let ids
    switch (type) {
      case 'planner':
        ids = this.simuScene.getPlannerIDs()
        break
      case 'car':
        ids = this.simuScene.getVehicleIDs()
        break
      case 'pedestrian':
      case 'animal':
      case 'bike':
      case 'moto':
      case 'machine':
        ids = this.simuScene.getPedestrianIDsBySubType(type)
        break
      case 'obstacle':
        ids = this.simuScene.getObstacleIDs()
        break
      case 'signlight':
        ids = this.simuScene.getSignlightIDs()
        break
      default:
        return []
    }

    return this.getObjectsInfoByIDs(type, ids)
  }

  /**
   * 获取所有场景元素
   * @return {*[]}
   */
  getAllObjects () {
    let alls = []
    const cars = this.getObjectsInfoByIDs('car', this.simuScene.getVehicleIDs())
    const pedestrians = this.getObjectsInfoByIDs('pedestrian', this.simuScene.getPedestrianIDsBySubType('pedestrian'))
    const animals = this.getObjectsInfoByIDs('animal', this.simuScene.getPedestrianIDsBySubType('animal'))
    const bicycles = this.getObjectsInfoByIDs('bike', this.simuScene.getPedestrianIDsBySubType('bike'))
    const motos = this.getObjectsInfoByIDs('moto', this.simuScene.getPedestrianIDsBySubType('moto'))
    const machines = this.getObjectsInfoByIDs('machine', this.simuScene.getPedestrianIDsBySubType('machine'))
    const obstacles = this.getObjectsInfoByIDs('obstacle', this.simuScene.getObstacleIDs())
    const planner = this.getObjectsInfoByIDs('planner', this.simuScene.getPlannerIDs())
    const rosbag = this.getObjectsInfoByIDs('rosbag', [0])

    if (cars?.length) {
      cars.sort((a, b) => a.id - b.id)
      alls = alls.concat(cars)
    }
    if (pedestrians?.length) {
      pedestrians.sort((a, b) => a.id - b.id)
      alls = alls.concat(pedestrians)
    }

    if (animals?.length) {
      animals.sort((a, b) => a.id - b.id)
      alls = alls.concat(animals)
    }

    if (bicycles?.length) {
      bicycles.sort((a, b) => a.id - b.id)
      alls = alls.concat(bicycles)
    }

    if (motos?.length) {
      motos.sort((a, b) => a.id - b.id)
      alls = alls.concat(motos)
    }

    if (machines?.length) {
      machines.sort((a, b) => a.id - b.id)
      alls = alls.concat(machines)
    }

    if (obstacles?.length) {
      obstacles.sort((a, b) => a.id - b.id)
      alls = alls.concat(obstacles)
    }
    // if (signlights?.length) {
    //   signlights.sort((a, b) => a.id - b.id)
    //   alls = alls.concat(signlights)
    // }
    if (planner?.length) {
      planner.sort((a, b) => a.id - b.id)
      alls = alls.concat(planner)
    }
    if (rosbag?.length) {
      rosbag.sort((a, b) => a.id - b.id)
      alls = alls.concat(rosbag)
    }
    return alls
  }

  /**
   * 添加一个场景元素
   * @param payload
   * @return {Promise<*|boolean>}
   */
  async addObject (payload) {
    const vec = this.simuScene.getPointerPosInScene(payload.x, payload.y)

    if (!vec) {
      const msg = i18n.t('tips.objOutOfRoad')
      MessageBox.promptEditorUIMessage('error', msg)
      return
    }

    const lonlat = this.simuScene.ct.xyz2lonlat(vec.x, vec.y, vec.z)
    const types = payload.type.split('.')
    let isBigObsticle = false
    if (types[0] === 'obstacle' && types[1].includes('Port_Crane')) {
      isBigObsticle = true
    }
    const laneInfo = await this.simuScene.hadmap.getNearbyLaneInfo(
      lonlat[0],
      lonlat[1],
      lonlat[2],
      isBigObsticle ? 11 : undefined,
    )

    if (laneInfo.err !== 0) {
      const msg = i18n.t('tips.objOutOfRoad')
      MessageBox.promptEditorUIMessage('error', msg)
      console.warn(`${msg}经纬度: ${lonlat[0]}, ${lonlat[1]}, ${lonlat[2]}`)
      return
    }

    // 一下类型可以放置在lanelink上
    const laneLinkTypes = ['car', 'pedestrian', 'animal', 'obstacle', 'machine']
    if (!laneLinkTypes.includes(types[0]) && laneInfo.type === MapElementType.LANELINK) {
      const info = i18n.t('tips.objOutOfRoad')
      MessageBox.promptEditorUIMessage('error', info)
      console.warn(`${info}经纬度: ${lonlat[0]}, ${lonlat[1]}, ${lonlat[2]}`)
      return false
    }

    let offsetRange = laneInfo.width / 2
    let info = i18n.t('tips.objOutOfRoad')
    if (isBigObsticle) {
      offsetRange = 11
    }
    // 行人和交通灯放置在5米范围
    if (['pedestrian', 'signlight', 'animal', 'bike', 'moto', 'machine'].includes(types[0])) {
      if (offsetRange < OffsetConst.Pedestrian) {
        offsetRange = OffsetConst.Pedestrian
      }
      info = i18n.t('tips.outOfRoadCenter', { n: OffsetConst.Pedestrian })
      // 其他放置在中心线两边2.5米范围内
    } else {
      if (offsetRange < OffsetConst.Other) {
        offsetRange = OffsetConst.Other
        info = i18n.t('tips.outOfRoadCenter', { n: '2.5' })
      }
    }

    if (Math.abs(laneInfo.offset) > offsetRange) {
      MessageBox.promptEditorUIMessage('error', info)
      console.warn(`${info}: 经纬度： ${lonlat[0]}, ${lonlat[1]}, ${lonlat[2]}`)
      return
    }

    let curve
    if (laneInfo.type === 'lane') {
      curve = this.simuScene.hadmap.getLane(laneInfo.rid, laneInfo.sid, laneInfo.lid)
    } else if (laneInfo.type === 'lanelink') {
      curve = this.simuScene.hadmap.getLanelink(laneInfo.llid)
    } else {
      console.error(`type error!`)
      return
    }

    let laneStartPT
    if (curve.data?.length) {
      const p = curve.getPoint(this.simuScene.LaneStartShift)
      if (p.length > 0) {
        laneStartPT = this.simuScene.ct.xyz2lonlat(p[0], p[1], p[2])
      } else {
        let msg
        if (laneInfo.type === 'lane') {
          msg = `lane ${laneInfo.rid}, ${laneInfo.sid}, ${laneInfo.lid} length to short`
        } else if (laneInfo.type === 'lanelink') {
          msg = `lanelink ${laneInfo.llid}, length to short`
        } else {
          msg = `map element type error!`
        }

        console.error(msg)
        MessageBox.promptEditorUIMessage('error', msg)
        return
      }
    } else {
      const msg = i18n.t('tips.noDataForRoad')
      MessageBox.promptEditorUIMessage('error', msg)
      return
    }

    let id
    switch (types[0]) {
      case 'planner':
        console.log('drop self')
        id = await this.simuScene.addPlannerByDrop(types[1], lonlat[0], lonlat[1], laneInfo)
        break
      case 'car':
        console.log('drop car')
        id = await this.simuScene.addCarByDrop(types[1], laneStartPT[0], laneStartPT[1], lonlat[0], lonlat[1], laneInfo)
        break
      case 'pedestrian':
      case 'animal':
      case 'bike':
      case 'moto':
      case 'machine':
        console.log(`drop ${types[0]}`)
        id = this.simuScene.addPedestrianByDrop(
          types[1],
          laneStartPT[0],
          laneStartPT[1],
          lonlat[0],
          lonlat[1],
          laneInfo,
        )
        break
      case 'obstacle':
        console.log('drop obstacle', types)
        id = this.simuScene.addObstacleByDrop(types[1], laneStartPT[0], laneStartPT[1], lonlat[0], lonlat[1], laneInfo)
        break
      // case 'signlight':
      //   console.log('drop traffic light')
      //   id = this.simuScene.addSignlightDataByDrop(laneStartPT[0], laneStartPT[1], laneInfo)
      //   break
    }

    this.simuScene.renderScene()

    const type = types[0]
    return this.getObjectInfoByID(type, id)
  }

  /**
   * 变更主车类型时同步更改主车模型显示
   * @param id
   * @return {Promise<void>}
   */
  syncPlannerVisual (id) {
    return this.simuScene.syncPlannerVisual(id)
  }

  /**
   * 复制一个场景元素
   * @param payload
   * @return {*}
   */
  copyObject (payload) {
    const types = payload.type.split('.')
    let id
    switch (types[0]) {
      case 'planner':
        break
      case 'car':
        id = this.simuScene.copyCar(payload)
        break
      case 'pedestrian':
      case 'animal':
      case 'bike':
      case 'moto':
      case 'machine':
        id = this.simuScene.copyPedestrianDataV2(payload)
        break
      case 'obstacle':
      case 'signlight':
        break
      default:
    }
    this.simuScene.renderScene()
    return this.getObjectInfoByID(types[0], id)
  }

  /**
   * 校验一个元素能否被删除
   * @param payload
   * @return {{result: boolean, message: string}}
   */
  canObjectRemoved (payload) {
    const { type, id } = payload
    const result = { result: true, message: '' }
    switch (type) {
      case 'car': {
        const follows = this.simuScene.getFollowedVehicleIDs(id)
        if (follows) {
          result.result = false
          result.message = `vehicle ${follows} follow this vehicle`
        }
      }
        break
      case 'acc':
      case 'merge':
      case 'route':
      case 'pedestrian':
      case 'animal':
      case 'bike':
      case 'moto':
      case 'machine':
      case 'obstacle':
      case 'signlight':
      case 'planner':
      case 'rosbag':
    }

    return result
  }

  /**
   * 删除一个场景元素
   * @param payload
   * @return {*}
   */
  removeObject (payload) {
    const { type, id } = payload
    const simuScene = this.simuScene
    let result
    switch (type) {
      case 'car':
        result = simuScene.removeVehicle(id)
        break
      case 'acc':
        result = simuScene.removeAcc(id)
        break
      case 'merge':
        result = simuScene.removeMerge(id)
        break
      case 'route':
        result = simuScene.removeRoute(id)
        break
      case 'pedestrian':
      case 'animal':
      case 'bike':
      case 'moto':
      case 'machine':
        result = simuScene.removePedestrian(id)
        break
      case 'obstacle':
        result = simuScene.removeObstacle(id)
        break
      case 'signlight':
        result = simuScene.removeSignlight(id)
        break
      case 'planner':
        result = simuScene.removePlanner(id)
        break
      case 'rosbag':
        result = simuScene.removeRosbag(id)
        break
    }
    if (simuScene.pickBox && simuScene.pickBox.isPicked()) {
      simuScene.pickBox.hide()
      simuScene.transformControl.detach()
    }
    return result.result
  }

  /**
   * 将视角定位到一个场景元素
   * @param payload
   */
  locateObject (payload) {
    if (payload) {
      const { type, id, index } = payload

      let notSend = false
      if ([
        'egopath_end',
        'vehiclepath_end',
        'pedestrianpath_end',
        'animalpath_end',
        'bikepath_end',
        'motopath_end',
        'machinepath_end',
      ].includes(type)) {
        notSend = true
      }

      this.simuScene.cancelPickedObject(notSend)
      if (this.simuScene.canSelectObject()) {
        this.simuScene.selectObject(type, id, index)
      }
      this.simuScene.locateCameraView2Obj(type, id, index)
    } else {
      this.simuScene.cancelPickedObject()
    }

    this.simuScene.renderScene()
    this.simuScene.calculateMeasurementResult(null, 'noedit')
  }

  /**
   * 更新一个场景元素（会导致元素位置、旋转等外参改变的）
   * @param payload
   * @return {Promise<*>}
   */
  modifyObject (payload) {
    return this.simuScene.modifyObject(payload)
  }

  /**
   * 更新一个场景元素（不会导致元素重渲染的内参）
   * @param payload
   */
  modifyObjectParams (payload) {
    // 添加物体或者改变物体位置的逻辑还是走上面的modifyObject，本方法只适用于改变物体与位置无关的属性，如初始速度，控制模式，等等
    const { type, data } = payload
    const target = this.simuScene.sceneParser.fineAnyById(type, data.id)
    if (target) {
      Object.assign(target, data)
      switch (type) {
        case 'car':
          this.simuScene.sceneParser.modifyVehicle(target)
          break
        case 'planner':
          this.simuScene.sceneParser.modifyPlanner(target)
      }
    }
  }

  /**
   * 设置一个元素的朝向
   * @param payload
   */
  setAngle (payload) {
    const { type, id, angle } = payload
    let target
    switch (type) {
      case 'planner':
        target = this.simuScene.sceneParser.findPlannerInArr(id)
        break
      case 'car':
        target = this.simuScene.sceneParser.findVehicleInMap(id)
        break
      case 'pedestrian':
      case 'animal':
      case 'bike':
      case 'moto':
      case 'machine':
        target = this.simuScene.sceneParser.findPedestrianInMap(id)
        break
      case 'obstacle':
        target = this.simuScene.sceneParser.findObstacleInMap(id)
        break
    }
    if (target) {
      // 直接设置角度值
      target.start_angle = angle
      this.simuScene.alignObject(target, target.model.position, null, angle)
      target.model.updateMatrixWorld(true)
      if (this.simuScene.transformControl.object && this.simuScene.transformControl.object.uuid === target.model.uuid) {
        this.simuScene.pickBox.pick(target.model)
      }
      this.renderSimScene()
    }
  }

  /**
   * 开始测量状态
   */
  setMeasurement () {
    this.simuScene.SetState(SceneState.StatePickMeasurementEnd)
  }

  /**
   * 删除一个测量点
   * @param payload
   */
  removeMeasurementPoint (payload) {
    const { id, idx } = payload
    this.simuScene.removeMeasurementEnd(id, idx)
    this.simuScene.renderScene()
    this.simuScene.calculateMeasurementResult()
  }

  /**
   * 删除一整段测量线
   * @param payload
   */
  removeMeasurement (payload) {
    const { id } = payload
    this.simuScene.removeMeasurement(id)
    this.simuScene.renderScene()
    this.simuScene.calculateMeasurementResult()
  }

  /**
   * 设置主车设置路径状态
   * @param id
   * @return {boolean}
   */
  setEgoCarEnd (id) {
    const ret = this.simuScene.setCurEditPlanner(id)
    if (ret && id !== undefined) {
      this.simuScene.SetState(SceneState.StatePickEgoCarEnd)
    }
    return ret
  }

  /**
   * 设置交通车设置路径状态
   * @param id
   * @return {boolean}
   */
  setVehicleEnd (id) {
    const ret = this.simuScene.setCurEditVehicle(id)
    if (ret && id !== undefined) {
      this.simuScene.SetState(SceneState.StatePickVehicleEnd)
    }

    return ret
  }

  /**
   * 设置行人设置路径状态
   * @param id
   * @return {boolean}
   */
  setPedestrianEnd (id) {
    const ret = this.simuScene.setCurEditPedestrian(id)
    if (ret && id !== undefined) {
      this.simuScene.SetState(SceneState.StatePickPedestrianEnd)
    }

    return ret
  }

  /**
   * 删除一个交通车轨迹点
   * @param id
   * @param idx
   */
  removeVehicleEnd (id, idx) {
    this.simuScene.removeVehicleEnd(id, idx)
  }

  /**
   * 删除一个行人轨迹点
   * @param {*} id
   * @param {*} idx idx 为 -1时代表了全部删除，只保留第一个路径点
   */
  removePedestrianEnd (id, idx) {
    this.simuScene.removePedestrianEnd(id, idx)
  }

  isStateNormal () {
    return this.simuScene.State() === SceneState.StateEdit
  }

  /**
   * 删除一个主车轨迹点
   * @param id
   * @param idx
   */
  removePlannerEnd (id, idx) {
    this.simuScene.removePlannerEnd(id, idx)
  }

  /**
   * 批量生成场景
   * @param id
   * @param name
   * @param payload
   * @return {*}
   */
  paramScene (id, name, payload) {
    return this.simuScene.paramScene(id, name, payload)
  }

  /**
   * 缩放视角
   * @param factor
   * @return {*}
   */
  zoom (factor) {
    return this.simuScene.zoom(factor)
  }

  /**
   * 获取地图信息
   * @return {any}
   */
  getMapInfo () {
    return Object.assign({}, this.simuScene.sceneParser.mapfile)
  }

  /**
   * 根据类型获取相对车道中心线最大横向偏移
   * @param payload
   * @return {number|number}
   */
  getMaxOffset (payload) {
    const { type, routeId, laneId } = payload
    let maxOffset = this.simuScene.getLaneWidth(routeId, laneId) / 2
    maxOffset = maxOffset < OffsetConst.Other ? OffsetConst.Other : maxOffset
    if (['pedestrian', 'signlight', 'animal', 'bike', 'moto', 'machine'].includes(type)) {
      return maxOffset < OffsetConst.Pedestrian ? OffsetConst.Pedestrian : maxOffset
    } else if (type === 'obstacle' && payload.obstacleType.startsWith('Port_Crane')) {
      // 吊机暂时允许设置成11
      return 11
    } else {
      return maxOffset
    }
  }

  /**
   * 获取某个车道的宽度
   * @param routeId
   * @param laneId
   * @return {number|*}
   */
  getLaneWidth (routeId, laneId) {
    return this.simuScene.getLaneWidth(routeId, laneId)
  }

  /**
   * 获取某个车道的长度
   * @param routeId
   * @param laneId
   * @return {number|*}
   */
  getLaneLength (routeId, laneId) {
    return this.simuScene.getLaneLength(routeId, laneId)
  }

  /**
   * 根据route id获取所有车道
   * @param routeId
   * @return {[]}
   */
  getLaneListByRoute (routeId) {
    return this.simuScene.getLaneIDsByRoute(routeId)
  }

  /**
   * 获取某交通车所在车道的最大纵向偏移
   * @param vehicleId
   * @return {number|*}
   */
  getVehicleMaxShift (vehicleId) {
    return this.simuScene.getVehicleMaxShift(vehicleId)
  }

  /**
   * 获取某交通车所在车道最大横向偏移
   * @param type
   * @param id
   * @return {number|*}
   */
  getObjectMaxOffset (type, id) {
    return this.simuScene.getObjectMaxOffset(type, id)
  }

  /**
   * 获取所有交通车id
   * @return {*}
   */
  getVehicleIDs () {
    return this.simuScene.getVehicleIDs()
  }

  /**
   * 获取跟踪的交通车id
   * @return {*}
   */
  getFollowedVehicleIDs (vehicleId) {
    return this.simuScene.getFollowedVehicleIDs(vehicleId)
  }

  /**
   * 获取可以被跟踪的交通车id
   * @return {*}
   */
  getCanBeFollowedVehicleIDs (vehicleId) {
    return this.simuScene.getCanBeFollowedVehicleIDs(vehicleId)
  }

  /**
   * 获取所有消息通道
   * @return {*}
   */
  allSignals () {
    return EditorGlobals.signals
  }

  /**
   * 设置相机
   * @return {*}
   */
  setCamera (type) {
    if (type === 'top') {
      this.simuScene.change2TopCamera()
    } else if (type === 'debugging') {
      this.simuScene.change2PerspectiveCamera()
    }
  }

  /**
   * 是否是顶部视角的初始状态
   * @returns
   */
  isUsingTopCamera () {
    return this.simuScene.Config().useTopCamera
  }

  /**
   * 判断是否可以打开场景
   * @return {*}
   */
  canOpenScenario (scenario) {
    try {
      return this.simuScene.canOpenScenario(scenario)
    } catch (error) {
      console.warn('canOpenScenrio failed: ', error)
      return false
    }
  }

  /**
   * 更新地图信息
   * @return {*}
   */
  updateHadmapInfo () {
    return this.simuScene.hadmap.loadHadmapinfo()
  }

  /**
   * 获取状态
   * @return {*}
   */
  getState () {
    return this.simuScene.State()
  }

  /**
   * 获取地图所有元素
   * @return {*}
   */
  getMapGroup () {
    const {
      allDashedLaneBoundary,
      allLane,
      allLaneBoundary,
      allLaneLink,
      laneGroup,
      mapObjectGroup,
      laneboundaries,
    } = this.simuScene.hadmap
    const group = new Group()
    Object.values({
      allDashedLaneBoundary,
      allLane,
      allLaneBoundary,
      allLaneLink,
      laneGroup,
      mapObjectGroup,
    }).forEach((v) => {
      if (v) {
        group.add(v.clone())
      }
    })
    laneboundaries.forEach((lane) => {
      group.add(lane.mesh.clone())
    })
    return group
  }

  /**
   * 获取信控方案
   * @return {*}
   */
  getSignlightPlans () {
    return this.simuScene.getSignlightPlans()
  }

  /**
   * 设置信控方案
   * @return {*}
   */
  setSignlightPlans (data) {
    this.simuScene.setSignlightPlans(data)
  }

  /**
   * 获取是否紧贴道路中线
   * @return {*}
   */
  getSnap () {
    return this.simuScene.Snap()
  }

  /**
   * 设置是否紧贴道路中线
   * @return {*}
   */
  setSnap (val) {
    return this.simuScene.setSnap(val)
  }

  /**
   * 更新元素的模型
   * @return {*}
   */
  updateModel (data) {
    let targets
    let type
    const interfaceCategory = getModelProperty(data, 'interfaceCategory')
    switch (interfaceCategory) {
      case InterfaceCategory.EGO:
        type = 'planner'
        break
      case InterfaceCategory.CAR:
        type = 'car'
        break
      case InterfaceCategory.BIKE:
      case InterfaceCategory.ANIMAL:
      case InterfaceCategory.MACHINE:
      case InterfaceCategory.MOTO_BIKE:
      case InterfaceCategory.PEDESTRIAN:
        type = 'pedestrian'
        break
      case InterfaceCategory.OBSTACLE:
        type = 'obstacle'
        break
    }
    switch (type) {
      case 'planner':
        targets = this.simuScene.sceneParser.planners.filter(p => p.name === data.variable)
        break
      case 'car':
        targets = this.simuScene.sceneParser.vehicles.filter(c => c.type === data.variable)
        break
      case 'pedestrian':
        targets = this.simuScene.sceneParser.pedestrians.filter(p => p.subType === data.variable)
        break
      case 'obstacle':
        targets = this.simuScene.sceneParser.obstacles.filter(p => p.type === data.variable)
        break
    }
    if (targets?.length) {
      this.simuScene.modelsManager.clearModelCache(type, data.variable)
      targets.forEach((p) => {
        this.simuScene.syncVisual(type, p.id)
      })
    }
  }
}

export default CanvasWrapper
