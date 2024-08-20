import {
  AmbientLight,
  AxesHelper,
  Box3,
  BufferAttribute,
  BufferGeometry,
  Clock,
  Euler,
  GridHelper,
  Group,
  Line,
  LineBasicMaterial,
  MathUtils,
  OrthographicCamera,
  PerspectiveCamera,
  Scene,
  Vector3,
  WebGLRenderer,
} from 'three'
import { ScenarioModelsManager } from 'models-manager/index'
import GlobalConfig from '../common/Config'
import '../libs/inflate.min'
import OrbitControls from '../libs/OrbitControls'
import CoordConverter from '../sceneeditor/CoordConverter'
import Hadmap from '../sceneeditor/Hadmap'
import MapFile from '../sceneeditor/MapFile'
import { MessageBox } from '../sceneeditor/MessageBox'
import PlayerSceneParser from '../sceneeditor/SimuSceneParser'
import LocalMessagePumper from './local_data/LocalMessagePumper'
import PlayerGlobals from './PlayerGlobals'
import SelfV from './SelfV'
import TrafficDO from './TrafficDO'
import TrafficLight from './TrafficLight'
import TrafficO from './TrafficO'
import TrafficV from './TrafficV'
import Trajectory from './Trajectory'
import VehicleUIInfo from './VehicleUIInfo'
import i18n from '@/locales'
import store from '@/store'

const { isNaN } = Number

class PlayerScene {
  constructor ({ wrapper }) {
    this.curPlayTime = 0
    this.topSceneRange = 80
    this.topSceneZoomFactor = 1.0
    this.aspect = wrapper.clientWidth / wrapper.clientHeight
    this.scene = new Scene()
    this.scene.name = `playerScene${new Date().toLocaleTimeString()}`
    // 通用加载器问题，暂时关闭
    // this.scene.autoUpdate = false
    // 自由视角相机
    this.freeCamera = new PerspectiveCamera(45, this.aspect, 0.1, 4000)

    this.cameraPosBackup = new Vector3()
    this.cameraRotBackup = new Euler()
    this.cameraUpBackup = new Vector3()
    // 跟踪视角相机
    this.cameraFollow = new PerspectiveCamera(45, this.aspect, 0.1, 4000)
    // 顶视角相机
    this.cameraTop = new OrthographicCamera(
      -1 * this.topSceneRange * this.aspect,
      this.topSceneRange * this.aspect,
      this.topSceneRange,
      -1 * this.topSceneRange,
    )
    // 司机视角相机
    this.cameraFPS = new PerspectiveCamera(45, this.aspect, 0.1, 4000)
    const canvas = document.createElement('canvas')
    let context = canvas.getContext('webgl2')
    if (context == null) {
      console.log('use webgl 1')
      context = canvas.getContext('webgl')
    } else {
      console.log('use webgl 2')
    }

    // 初始化渲染器
    this.renderer = new WebGLRenderer({ canvas, context, antialias: true })
    this.renderer.setPixelRatio(window.devicePixelRatio)
    this.renderer.debug.checkShaderErrors = false

    this.ambientLight = new AmbientLight(0xFFFFFF, 3)
    // 坐标转换器
    this.ct = new CoordConverter()
    this.hadmap = new Hadmap(this.ct, this)
    // 坐标轴
    this.axis = new AxesHelper(20)
    // 参考网格
    this.grid = new GridHelper(10000, 500, 0x101010, 0x101010)
    this.grid.rotation.x = Math.PI / 2
    this.grid.material.depthTest = false
    this.grid.renderOrder = -999
    this.freeCamera.position.set(0, 0, 193)
    this.freeCamera.up.set(0, 0, 1)
    // 自由视角控制器
    this.flyControls = new OrbitControls(this.freeCamera, wrapper.parentNode)
    this.flyControls.rotateSpeed = 1.0
    this.flyControls.zoomSpeed = 1.0
    this.flyControls.panSpeed = 1.0
    this.flyControls.maxPolarAngle = Math.PI * 0.5
    this.flyControls.minDistance = 5
    this.flyControls.maxDistance = 2000
    this.flyControls.addEventListener('change', () => {
      this.dispatchFreeCameraZoom()
      this.dispatchCameraChanged(this.flyControls.object, this.flyControls.getAzimuthalAngle())
    })
    this.flyClock = new Clock()

    this.sceneParser = new PlayerSceneParser(this)

    // 初始化各种元素的map
    this.currentPlanner = null
    this.egoMap = new Map()
    this.egoTransparentMap = new Map()
    this.trailerMap = new Map()
    this.trailerTransparentMap = new Map()
    this.trjMap = new Map()
    this.trjTransparentMap = new Map()
    this.trafficVMap = new Map()
    this.trafficVReplayMap = new Map()
    this.trafficOMap = new Map()
    this.trafficPMap = new Map()
    this.trafficLightMap = new Map()
    this.trafficVUIInfoMap = new Map()

    this.curRenderer = this.renderer
    this.curCamera = this.cameraFollow

    this.status = PlayerScene.StatusRunning
    this.ModelHeiht = 0.0

    // configuration
    this.cameraTargetOriginalDistance = this.freeCamera.position.distanceTo(this.flyControls.target)
    this.ZOOM_MIN = 0.1
    this.ZOOM_MAX = 1000
    this.flyControls.minDistance = this.cameraTargetOriginalDistance * this.ZOOM_MIN
    this.flyControls.maxDistance = this.cameraTargetOriginalDistance * this.ZOOM_MAX
    this.cameraFollowPlanner = true
    this.initialized = false

    this.modelsManager = new ScenarioModelsManager({
      type: 'player',
      catalogs: () => this.catalogs,
      prefix: 'assets/models',
    })
  }

  get catalogs () {
    return {
      ...store.state.catalogs,
      plannerList: store.state.planners.plannerList,
    }
  }

  /**
   * 障碍物类型map
   * @return {{}}
   */
  get obstacleTypesMap () {
    const map = {}
    this.catalogs.obstacleList.forEach((o) => {
      map[o.catalogParams.properties.modelId] = o.variable
    })
    return map
  }

  /**
   * 各个相机变化的回调
   * @param camera
   * @param radian
   */
  dispatchCameraChanged (camera, radian = 0) {
    if (this.curCamera === camera) {
      const degree = MathUtils.radToDeg(radian)
      PlayerGlobals.signals.cameraChanged.dispatch({ degree })
    }
  }

  dispatchTopCameraZoom () {
    if (this.curCamera === this.cameraTop) {
      PlayerGlobals.signals.zoomView.dispatch({ factor: this.topSceneZoomFactor })
      return this.topSceneZoomFactor
    }
  }

  dispatchFreeCameraZoom () {
    if (this.curCamera === this.freeCamera) {
      const distance = this.freeCamera.position.distanceTo(this.flyControls.target)
      const factor = distance / this.cameraTargetOriginalDistance
      PlayerGlobals.signals.zoomView.dispatch({ factor })
      return factor
    }
  }

  async init (wrapper, sceneId, jobId, status) {
    // 画布
    this.wrapper = wrapper
    this.halfWidth = this.wrapper.clientWidth / 2
    this.halfHeight = this.wrapper.clientHeight / 2
    this.status = status

    // 设置renderer背景色
    this.renderer.setClearColor(0x1F1F27, 1.0)
    this.renderer.setSize(wrapper.clientWidth, wrapper.clientHeight)

    // 保存相机初始位置
    this.cameraPosBackup.copy(this.freeCamera.position)
    this.cameraUpBackup.copy(this.freeCamera.up)
    this.cameraRotBackup.copy(this.freeCamera.rotation)

    this.resetCamera()

    // setup scene
    this.scene.add(this.ambientLight)
    this.scene.add(this.axis)
    this.scene.add(this.grid)
    this.renderScene()

    this.localReceiver = new LocalMessagePumper(this)
  }

  // 重置相机位置
  resetCamera () {
    this.cameraTop.position.x = 0
    this.cameraTop.position.y = 0
    this.cameraTop.position.z = 50
    this.cameraTop.up.x = 0
    this.cameraTop.up.y = 0
    this.cameraTop.up.z = 1

    this.cameraFollow.position.x = -114
    this.cameraFollow.position.y = 20
    this.cameraFollow.position.z = 67
    this.cameraFollow.rotation.x = -0.46
    this.cameraFollow.rotation.y = -1.0
    this.cameraFollow.rotation.z = -2.14
    this.cameraFollow.up.x = 0
    this.cameraFollow.up.y = 0
    this.cameraFollow.up.z = 1

    this.cameraFPS.position.x = -30
    this.cameraFPS.position.y = 40
    this.cameraFPS.position.z = 30
    this.cameraFPS.up.x = 0
    this.cameraFPS.up.y = 0
    this.cameraFPS.up.z = 1
  }

  setPlayCallback (cb) {
    this.playCallback = cb
  }

  resetSceneSetting () {
    this.topSceneZoomFactor = 1
  }

  resetStaticSceneData () {
    this.emptyTrafficLight()
  }

  // 清除动态场景数据
  clearDynamicSceneData () {
    this.emptyTrafficV()
    this.emptyTrafficO()
    this.emptyTrafficP()
    this.emptyTrafficVUIInfo()
  }

  /**
   * 设置不同元素的显示隐藏
   * @param type
   * @param show
   */
  showObject (type, show) {
    if (type === 'route') {
      this.sceneParser.routes.forEach((rs) => {
        rs.setVisible(show)
      })
    } else if (type === 'car') {
      this.sceneParser.vehicles.forEach((v) => {
        v.setVisible(show)
      })
    } else if (type === 'planner') {
      this.sceneParser.planners.forEach((p) => {
        p.setVisible(show)
      })
    } else if (type === 'signlight') {
      this.sceneParser.signlights.forEach((sl) => {
        sl.setVisible(show)
      })
    } else if (type === 'pedestrian') {
      this.sceneParser.pedestrians.forEach((pt) => {
        pt.setVisible(show)
      })
    } else if (type === 'obstacle') {
      this.sceneParser.obstacles.forEach((ob) => {
        ob.setVisible(show)
      })
    } else if (type === 'road') {
      this.hadmap.setRoadVisible(show)
    } else if (type === 'lane') {
      this.hadmap.setLaneVisible(show)
    } else if (type === 'laneboundary') {
      this.hadmap.setLaneBoundaryVisible(show)
    } else if (type === 'lanelink') {
      this.hadmap.setLaneLinkVisible(show)
    } else if (type === 'mapobject') {
      this.hadmap.setMapObjectVisible(show)
    } else if (type === 'roadmesh') {
      this.hadmap.setLaneMeshVisible(show)
    } else if (type === 'grid') {
      this.grid.visible = !!show
    } else {
      console.error('unknown show type', type)
    }
  }

  prepareRender (elem) {
    elem.append(this.curRenderer.domElement)
  }

  /**
   * 将视角定位到指定元素上，暂时只支持主车
   * @param payload
   */
  locateObject (payload) {
    const { type, id } = payload

    if (type === 'car') {
    } else if (type === 'planner') {
      const planner = this.egoMap.get(`${id}`)
      if (!planner) {
        console.warn(`没找到主车：${id}`)
        return
      }
      // 顶相机
      const { x, y, z } = planner.model.position
      this.cameraTop.position.x = x
      this.cameraTop.position.y = y
      if ((z + 50) < 0) {
        this.cameraTop.position.z = 50
      } else {
        this.cameraTop.position.z = z + 50
      }
      this.cameraTop.updateMatrixWorld(true)

      if (this.curCamera === this.freeCamera) {
        // 自由相机
        this.attachFreeCamera(planner)
      }

      planner.model.add(this.cameraFPS)

      this.currentPlanner = planner
      // dispatch locateObject
      PlayerGlobals.signals.locateObject.dispatch('planner', id)
      this.updateAllVUIInfo()
    }
  }

  /**
   * 缩放顶相机
   * @param factor
   */
  zoomCameraTop (factor) {
    this.cameraTop.left = -1 * this.topSceneRange * factor * this.aspect
    this.cameraTop.right = this.topSceneRange * factor * this.aspect
    this.cameraTop.top = this.topSceneRange * factor
    this.cameraTop.bottom = -1 * this.topSceneRange * factor

    this.cameraTop.updateProjectionMatrix()
    this.updateAllVUIInfo()
  }

  /**
   * 缩放视角
   * @param factor
   * @return {number}
   */
  zoom (factor) {
    const list = [this.cameraTop, this.freeCamera]

    if (!list.includes(this.curCamera)) {
      MessageBox.promptEditorUIMessage('info', i18n.t('tips.zoomOnlyTopFree'))
      return this.topSceneZoomFactor
    }
    if (this.curCamera === this.cameraTop) {
      let update = false
      const tmp = this.topSceneZoomFactor * factor
      if (tmp < this.ZOOM_MIN) {
        if (this.topSceneZoomFactor < this.ZOOM_MIN + 0.0000001) {
        } else {
          this.topSceneZoomFactor = this.ZOOM_MIN
          update = true
        }
      } else if (tmp > this.ZOOM_MAX) {
        if (this.topSceneZoomFactor > this.ZOOM_MAX + 0.0000001) {
        } else {
          this.topSceneZoomFactor = this.ZOOM_MAX
          update = true
        }
      } else {
        this.topSceneZoomFactor = tmp
        update = true
      }

      if (update) {
        this.zoomCameraTop(this.topSceneZoomFactor)
      }
      return this.dispatchTopCameraZoom()
    } else if (this.curCamera === this.freeCamera) {
      // 自由视角下缩放焦点在鼠标位置上
      const position = new Vector3()
      position.subVectors(this.freeCamera.position, this.flyControls.target)
      const length = position.length()
      if (length * factor < this.flyControls.minDistance) {
        factor = this.flyControls.minDistance / length
      } else if (length * factor > this.flyControls.maxDistance) {
        factor = this.flyControls.maxDistance / length
      }
      position.multiplyScalar(factor)
      position.add(this.flyControls.target)
      this.freeCamera.position.copy(position)
      return this.dispatchFreeCameraZoom()
    }
  }

  // 备份自由相机状态
  backupFreeCamera () {
    if (this.curCamera === this.freeCamera) {
      this.cameraPosBackup.copy(this.freeCamera.position)
      this.cameraUpBackup.copy(this.freeCamera.up)
      this.cameraRotBackup.copy(this.freeCamera.rotation)
    }
  }

  // 重置自由相机状态
  restoreFreeCamera () {
    this.freeCamera.position.copy(this.cameraPosBackup)
    this.freeCamera.rotation.copy(this.cameraRotBackup)
    this.freeCamera.up.copy(this.cameraUpBackup)
  }

  // 切换到顶相机
  change2TopCamera () {
    this.backupFreeCamera()
    this.curCamera = this.cameraTop
    this.updateAllVUIInfo()
    GlobalConfig.setUseTopCameraEditor(true)
    this.dispatchTopCameraZoom()
    this.dispatchCameraChanged(this.curCamera, this.curCamera.rotation.z)
  }

  // 切换到跟随相机
  change2FollowCamera () {
    this.backupFreeCamera()
    this.curCamera = this.cameraFollow
    this.updateAllVUIInfo()
    GlobalConfig.setUseTopCameraEditor(false)
    this.dispatchCameraChanged(this.curCamera, this.curCamera.rotation.z)
  }

  // 切换到驾驶员相机
  change2FPSCamera () {
    this.backupFreeCamera()
    this.curCamera = this.cameraFPS
    this.updateAllVUIInfo()
    GlobalConfig.setUseTopCameraEditor(false)
    if (this.currentPlanner) {
      this.dispatchCameraChanged(this.curCamera, this.currentPlanner.model.rotation.z - Math.PI / 2)
    } else {
      this.dispatchCameraChanged(this.curCamera, 0)
    }
  }

  // 切换到自由相机
  change2FreeCamera () {
    this.curCamera = this.freeCamera
    if (!this.initialized) {
      this.restoreFreeCamera()
    }
    setTimeout(() => {
      this.updateAllVUIInfo()
      this.freeCamera.updateMatrixWorld(true)
    }, 20)
    GlobalConfig.setUseTopCameraEditor(false)
    this.dispatchFreeCameraZoom()
    this.dispatchCameraChanged(this.curCamera, this.curCamera.rotation.z)
  }

  renderScene () {
    this.curRenderer.render(this.scene, this.curCamera)
  }

  onMouseButtonDown (event, status) {
    // left button
    if (event.button === 0) {
      this.lButtonDown = true
      // middle button
    } else if (event.button === 1) {
      // right button
    } else if (event.button === 2) {
      this.rButtonDown = true
    }
  }

  onMouseButtonUp (event) {
    // left button
    if (event.button === 0) {
      this.lButtonDown = false
      // middle button
    } else if (event.button === 1) {
      // right button
    } else if (event.button === 2) {
      event.preventDefault()
      this.rButtonDown = false
    }
    this.updateAllVUIInfo()
  }

  // 如果视角在移动或者缩放，则需要更新UI层
  onMouseMove (event) {
    const curX = event.clientX
    const curY = event.clientY
    if (this.rButtonDown) {
      const deltaX = curX - this.lastPX
      const deltaY = curY - this.lastPY

      const width = this.wrapper.clientWidth
      const height = this.wrapper.clientHeight
      const camWidth = this.cameraTop.right - this.cameraTop.left
      const camHeight = this.cameraTop.bottom - this.cameraTop.top

      this.cameraTop.position.x -= camWidth * (deltaX / width)
      this.cameraTop.position.y -= camHeight * (deltaY / height)

      this.updateAllVUIInfo()
    }

    if (this.lButtonDown) {
      if (store.state.scenario.playingStatus === 'ready') {
        this.updateAllVUIInfo()
      }
    } else {
      const x = event.offsetX
      const y = event.offsetY
      const ux = (x / this.wrapper.clientWidth) * 2 - 1
      const uy = ((this.wrapper.clientHeight - y) / this.wrapper.clientHeight) * 2 - 1
      this.hadmap.showLaneId(ux, uy, this.curCamera)
    }

    this.lastPX = curX
    this.lastPY = curY
  }

  onMouseWheel (wheelEvent) {
    if (this.curCamera === this.cameraTop) {
      const factor = 1 + wheelEvent.deltaY * 0.001
      this.zoom(factor)
    } else if (this.curCamera === this.freeCamera) {
      this.updateAllVUIInfo()
    }
  }

  /**
   * 将自由相机重置到主车附近
   * @param planner
   */
  attachFreeCamera (planner) {
    const { position } = planner.model
    this.freeCamera.position.copy(position).add(new Vector3(-50, 0, 10))
    this.freeCamera.lookAt(position)
    this.flyControls.target.copy(position)
    this.dispatchCameraChanged(this.flyControls.object, this.flyControls.getAzimuthalAngle())
  }

  /**
   * 加载地图
   * @param hdmapName
   * @param hdmapDesc
   * @return {Promise<boolean>}
   */
  async loadHadmap (hdmapName, hdmapDesc) {
    if (this.hadmap.loaded() === true) {
      const msg = i18n.t('tips.sceneCannotLoadedAgain')
      MessageBox.promptEditorUIMessage('error', msg)
      return false
    }

    const hm = this.hadmap.findHadmap(hdmapName)

    if (!hm) {
      const msg = i18n.t('tips.mapConfigInfoNotExists', { map: hdmapName })
      console.error(msg)
      MessageBox.promptEditorUIMessage('error', msg)
      return false
    }

    this.sceneParser.mapName = hdmapName
    this.sceneParser.mapDesc = hdmapDesc
    this.ct.setRef(hm.mapRefLon, hm.mapRefLat, hm.mapRefAlt)
    this.hadmap.setSampleDistance(20)

    if (!this.sceneParser.mapfile) {
      this.sceneParser.mapfile = new MapFile()
    }
    const {
      refLon,
      refLat,
      refAlt,
    } = this.ct
    this.sceneParser.mapfile.setData({
      lon: refLon,
      lat: refLat,
      alt: refAlt,
      mapName: hdmapName,
    })

    let result
    try {
      result = await this.hadmap.loadMapToSceneLocal(hdmapName, this.scene)
    } catch (err) {
      console.error(err)
    }

    return !!result
  }

  /**
   * 清除场景数据
   */
  clearScene () {
    this.localReceiver.clear()
    this.resetSceneSetting()

    if (this.localReceiver) {
      this.localReceiver.reset()
    }

    this.clearSelf()
    this.clearCtlTrj()
    this.resetStaticSceneData()
    this.clearDynamicSceneData()
    this.hadmap.clearHadmap()
    this.sceneParser.clearScene()
  }

  /**
   * 播放完毕之后重置一些模型位置
   */
  resetScene () {
    if (this.localReceiver) {
      this.localReceiver.reset()
    }

    this.resetSelf()
    this.clearDynamicSceneData()
  }

  /**
   * 加载场景
   * @param sceneId
   * @return {Promise<void>}
   */
  async loadScene (sceneId) {
    console.log('begin load scene ', sceneId)
    try {
      this.sceneId = sceneId
      console.log('get scene content')
      const sceneData = await this.sceneParser.getScene(sceneId)
      console.log('get scene content finished, begin parse scene')
      this.sceneParser.parseScene(this.sceneParser, sceneData)
      console.log('scene parsed, start to load hadmap and planner')
      await Promise.all([this.createSelf(), this.loadHadmap(this.sceneParser.mapfile.mapName)])
      console.log('load hadmap succeed, load scene finished!')
    } catch (e) {
      console.error(e)
    }
  }

  unbindMessageReceiver () {
    this.localReceiver.unbind()
  }

  /**
   * 创建主车数据，加载主车模型
   * @return {Promise<void>}
   */
  async createSelf () {
    const ps = this.sceneParser.planners.map(async (p) => {
      const self = new SelfV()
      const selfTransparent = new SelfV()
      selfTransparent.replay = true
      self.id = p.id
      selfTransparent.id = p.id
      // 多挂车
      const trailers = []
      const trailersTransparent = []
      self.model = new Group()
      selfTransparent.model = new Group()

      self.selfModelSize = new Vector3()

      const {
        planner: plannerModel,
        trailers: trailersModel,
      } = await this.modelsManager.loadPlannerModel(p.name)

      self.model.add(plannerModel)
      this.scene.add(self.model)
      self.model.position.set(0, 0, 0)

      const {
        planner: plannerTransparentModel,
        trailers: trailersTransparentModel,
      } = await this.modelsManager.loadPlannerModel(p.name, true)
      selfTransparent.model.add(plannerTransparentModel)
      this.scene.add(selfTransparent.model)
      // 别跟眼前晃
      selfTransparent.model.position.set(-10000, -10000, -10000)
      selfTransparent.model.updateMatrixWorld(true)

      // todo: 调整司机视角用
      new Box3().setFromObject(self.model).getSize(self.selfModelSize)

      if (trailersModel.length) {
        trailersModel.forEach((t) => {
          const trailer = new SelfV()
          trailer.model = t
          trailer.model.position.set(-10000, -10000, -10000)
          trailer.model.updateMatrixWorld(true)
          trailers.push(trailer)
          this.scene.add(t)
        })
      }

      if (trailersTransparentModel.length) {
        trailersTransparentModel.forEach((t) => {
          const trailerT = new SelfV()
          trailerT.model = t.clone()
          trailerT.model.position.set(-10000, -10000, -10000)
          trailerT.model.updateMatrixWorld(true)
          trailersTransparent.push(trailerT)
          this.scene.add(trailerT.model)
        })
      }
      this.egoMap.set(`${p.id}`, self)
      this.egoTransparentMap.set(`${p.id}`, selfTransparent)
      this.trailerMap.set(`${p.id}`, trailers)
      this.trailerTransparentMap.set(`${p.id}`, trailersTransparent)

      const trj = new Trajectory(this.scene, this.hadmap.ct)
      const trjTransparent = new Trajectory(this.scene, this.hadmap.ct)
      this.trjMap.set(`${p.id}`, trj)
      this.trjTransparentMap.set(`${p.id}`, trjTransparent)
    })
    await Promise.all(ps)
    if (this.egoMap.size) {
      this.locateObject({ type: 'planner', id: this.sceneParser.planners[0].id })
    }
    if (!this.initialized) {
      this.freeCamera.position.set(-50, 0, 10)
    }
    this.initialized = true
  }

  /**
   * 将相机重置到主车附近
   * @param tmp
   */
  centerCameras (tmp) {
    // 第三人称相机
    this.cameraFollow.position.x = tmp[0] - 10
    this.cameraFollow.position.y = tmp[1] - 10
    this.cameraFollow.position.z = 50
    this.cameraFollow.lookAt(this.currentPlanner.model.position)
    this.dispatchCameraChanged(this.cameraFollow, this.cameraFollow.rotation.z)

    // 顶相机
    this.cameraTop.position.x = tmp[0]
    this.cameraTop.position.y = tmp[1]
    if ((tmp[2] + 50) < 0) {
      this.cameraTop.position.z = 50
    } else {
      this.cameraTop.position.z = tmp[2] + 50
    }

    this.cameraFPS.position.x = 0
    this.cameraFPS.position.y = 0
    this.cameraFPS.position.z = this.currentPlanner.selfModelSize.z + 0.6
    this.cameraFPS.rotation.z = -Math.PI / 2
    this.cameraFPS.rotation.y = -Math.PI / 2
    this.dispatchCameraChanged(this.cameraFPS, this.currentPlanner.model.rotation.z - Math.PI / 2)
  }

  processCamerasFollow (tmp) {
    if (this.cameraFollowPlanner) {
      this.centerCameras(tmp)
    }
  }

  // 处理挂车位置数据
  processTrailer (trailerLocs) {
    trailerLocs.forEach((trailerLoc) => {
      const { id, locs } = trailerLoc
      locs.forEach((loc) => {
        // 暂时只有1条挂车数据
        const trailer = this.trailerMap.get(`${id}`)
        trailer.lastPos.copy(trailer.pos)
        trailer.copyFromNet(trailerLoc)

        const tmp = this.hadmap.ct.lonlat2xyz(trailerLoc.pos.x, trailerLoc.pos.y, trailerLoc.pos.z)
        trailer.model.position.set(tmp[0], tmp[1], 0)

        // rotation
        trailer.model.rotation.z = trailerLoc.rpy.z
        trailer.model.updateMatrixWorld(true)
        trailer.model.visible = store.state.playerViewConfig.sceneViewConfig.ego
      })
    })
    this.updateAllVUIInfo()
  }

  // 处理本车位置数据
  async processSelfLoc (selfLocs) {
    selfLocs.forEach((selfLoc) => {
      const { pos, vel, rpy, id, isTransparent = false, replay = false } = selfLoc
      const ego = this.egoMap.get(`${id}`)
      const egoT = this.egoTransparentMap.get(`${id}`)

      // 处理Logsim主车逻辑
      const self = isTransparent ? egoT : ego
      if (self?.model) {
        self.replay = replay
        // 修正另一个Self
        if (self === ego) {
          egoT.replay = !replay
        } else {
          ego.replay = !replay
        }
        if (this.playCallback) {
          const msg = {}
          const selfV = Math.sqrt(vel.x * vel.x + vel.y * vel.y)
          msg.velocity = +selfV.toFixed(2)
          msg.wheel = 0
          msg.acceleration = 0
          this.playCallback(msg)
        }

        self.lastPos.copy(self.pos)
        self.copyFromNet(selfLoc)

        const tmp = this.hadmap.ct.lonlat2xyz(pos.x, pos.y, 0) // fix 6265
        tmp[2] = 0
        self.model.position.set(tmp[0], tmp[1], tmp[2])

        // rotation
        self.model.rotation.z = rpy.z
        self.model.updateMatrixWorld(true)
        // todo: from release_0605
        // if (!replay) {
        //   Self.model.visible = store.state.playerViewConfig.sceneViewConfig.ego
        // }
        if (ego === this.currentPlanner) {
          this.processCamerasFollow(tmp)
        }
      }
    })
    this.updateAllVUIInfo()
  }

  // 处理本车路径数据
  processSelfTrj (trjLocs) {
    trjLocs.forEach((trjLoc) => {
      const { points, id, isTransparent = false } = trjLoc
      const trj = isTransparent ? this.trjTransparentMap.get(`${id}`) : this.trjMap.get(`${id}`)
      trj.render(points)
      // todo: from release_0605 trj.setVisible(store.state.playerViewConfig.sceneViewConfig.ego)
    })
  }

  /**
   * 处理Grading数据
   * @param message
   */
  processGrading (message = {}) {
    if (!this.Grading) {
      this.Grading = {}
    }
    const { distHeadway } = message
    if (distHeadway) {
      this.Grading.distToFellow = distHeadway.distToFellow
    }
  }

  /**
   * 清除主车、拖车、以及Logsim对应的数据
   */
  clearSelf () {
    const removeModel = (obj) => {
      this.scene.remove(obj.model)
      obj.model = null
    }
    this.egoMap.forEach(removeModel)
    this.egoTransparentMap.forEach(removeModel)
    this.trailerMap.forEach((trailers) => {
      trailers.forEach(removeModel)
    })
    this.trailerTransparentMap.forEach((trailers) => {
      trailers.forEach(removeModel)
    })
    this.egoMap.clear()
    this.egoTransparentMap.clear()
    this.trailerMap.clear()
    this.trailerTransparentMap.clear()
  }

  /**
   * 重置主车、拖车、以及Logsim对应的数据
   */
  resetSelf () {
    const resetModel = (obj) => {
      obj.model.position.set(-10000, -10000, -10000)
      obj.model.updateMatrixWorld(true)
    }
    this.egoMap.forEach(resetModel)
    this.egoTransparentMap.forEach(resetModel)
    this.trailerMap.forEach((trailers) => {
      trailers.forEach(resetModel)
    })
    this.trailerTransparentMap.forEach((trailers) => {
      trailers.forEach(resetModel)
    })
    this.trjMap.forEach((trj) => {
      trj.reset()
    })
    this.trjTransparentMap.forEach((trj) => {
      trj.reset()
    })
  }

  /**
   * 清除本车控制模块发出的路径数据
   */
  clearCtlTrj () {
    // ---- line ---------------
    if (this.selfLineCtlTrj) {
      this.scene.remove(this.selfLineCtlTrj)
      const mat = this.selfLineCtlTrj.material
      const geom = this.selfLineCtlTrj.geometry
      mat.dispose()
      geom.dispose()
      this.selfLineCtlTrj = null
    }

    this.trjMap.forEach((trj) => {
      trj.dispose()
    })
    this.trjTransparentMap.forEach((trj) => {
      trj.dispose()
    })
    this.trjMap.clear()
    this.trjTransparentMap.clear()
  }

  // 处理本车控制模块发出的路径数据
  processSelfCtlTrj (selfCtlTrj) {
    const len = selfCtlTrj.points.length
    const height = 0.8

    if (this.selfLineCtlTrj) {
      this.selfLineCtlTrj.visible = len > 1
    }

    if (len > 1) {
      const lineGeometry = new BufferGeometry()
      const linePositions = new Float32Array(len * 3) // 3 float per point

      let index = 0
      for (let i = 0; i < len; ++i) {
        const { x, y } = selfCtlTrj.points[i]
        const tmp = this.hadmap.ct.lonlat2xyz(x, y, 0)
        linePositions[index++] = tmp[0]
        linePositions[index++] = tmp[1]
        linePositions[index++] = height
      }
      lineGeometry.setAttribute('position', new BufferAttribute(linePositions, 3))

      const lineMaterial = new LineBasicMaterial({ color: 0xFF0000 })
      this.selfLineCtlTrj = new Line(lineGeometry, lineMaterial)
      this.scene.add(this.selfLineCtlTrj)
    }
  }

  /**
   * 更新所有交通对象的VUI信息
   * @param type
   * @param o
   */
  generateTrafficVUIInfo (type, o) {
    if (!o) {
      return
    }
    let tid = 'v_'
    // 修复元素的tid
    if (['self', 'trailer', 'self_logsim', 'trailer_logsim'].includes(type)) {
      // 主车\拖车及Logsim对应元素
      tid = `${type}_${`${o.id}`.padStart(0, '3')}`
    } else {
      if (type === 'car' || type === 'car_logsim') {
        // 交通车
        tid = 'v_'
      } else if (type === 'staticObj') {
        // 静态障碍物
        tid = 'so_'
      } else if (type === 'dynamicObj') {
        // 行人、动物、自行车、摩托车、电动车、机械等
        tid = 'do_'
      } else if (type === 'signlight') {
        // 信号灯
        tid = 'l_'
      } else {
        console.error('unknown traffic object type')
      }
      tid += o.id
      if (type === 'car_logsim') {
        // logsim的交通车
        tid += '_logsim'
      }
    }

    let vui = this.trafficVUIInfoMap.get(tid)
    if (['self', 'trailer', 'self_logsim', 'trailer_logsim'].includes(type)) {
      // 生成主车模型VUI信息
      // 主车模型没有加载完
      if (!o.model) return
      let selfV = o.vel.x * o.vel.x + o.vel.y * o.vel.y
      selfV = Math.sqrt(selfV)
      if (!vui) {
        vui = new VehicleUIInfo(tid, type)
        this.trafficVUIInfoMap.set(vui.id, vui)
      }
      o.model.getWorldPosition(vui.pos)
      vui.pos.project(this.curCamera)
      vui.v = selfV
      vui.vx = o.vel.x.toFixed(2)
      vui.vy = o.vel.y.toFixed(2)
      vui.lon = o.pos.x.toFixed(7)
      vui.lat = o.pos.y.toFixed(7)
      vui.alt = o.pos.z.toFixed(2)
      if (this.Grading && this.Grading.distToFellow !== undefined) {
        vui.dist = this.Grading.distToFellow
      }
    } else {
      if (!vui) {
        vui = new VehicleUIInfo(tid, type)
        vui.id = tid
        this.trafficVUIInfoMap.set(vui.id, vui)
      }

      vui.pos.set(o.x, o.y, this.ModelHeiht)
      vui.pos.project(this.curCamera)
      if (o.v) {
        vui.v = o.v
      } else {
        vui.v = 0
      }
      const { x = 0, y = 0, z = 0 } = o
      vui.lon = x === 0 ? 0 : x.toFixed(7)
      vui.lat = y === 0 ? 0 : y.toFixed(7)
      vui.alt = z === 0 ? 0 : z.toFixed(2)

      if (['car', 'car_logsim', 'dynamicObj'].includes(type)) {
        // 交通车、动态障碍物VUI
        vui.showAbsVelocity = o.showAbsVelocity.toFixed(2)
        vui.showAbsAcc = o.showAbsAcc.toFixed(2)
        vui.showRelativeAcc = o.showRelativeAcc.toFixed(2)
        vui.showRelativeVelocity = o.showRelativeVelocity.toFixed(2)
        vui.showRelativeVelocityHorizontal = o.showRelativeVelocityHorizontal.toFixed(2)
        vui.showRelativeDistVertical = o.showRelativeDistVertical.toFixed(2)
        vui.showRelativeDistHorizontal = o.showRelativeDistHorizontal.toFixed(2)
      }
    }
    vui.visible = o.model ? o.model.visible : false
    vui.pos.x = Math.round(this.halfWidth * vui.pos.x + this.halfWidth)
    vui.pos.y = Math.round(this.halfHeight * (-1 * vui.pos.y) + this.halfHeight)
    vui.v = vui.v.toFixed(2)
  }

  // 删除不再使用的VUI信息
  removeRedundantVUIInfo () {
    const toBeRemoved = []
    this.trafficVUIInfoMap.forEach((vui, id) => {
      if (['self', 'trailer', 'self_logsim', 'trailer_logsim'].find(s => vui.id.startsWith(s))) {
      } else {
        const ids = vui.id.split('_')
        const [type, idStr, logsim] = ids
        const id = +idStr
        let map
        switch (type) {
          case 'v':
            map = logsim === 'logsim' ? this.trafficVReplayMap : this.trafficVMap
            break
          case 'so':
            map = this.trafficOMap
            break
          case 'do':
            map = this.trafficPMap
            break
          case 'l':
            map = this.trafficLightMap
            break
        }
        if (!map.has(id) && !map.has(`${id}`)) {
          toBeRemoved.push(vui.id)
        }
      }
    })

    for (let i = 0; i < toBeRemoved.length; ++i) {
      this.trafficVUIInfoMap.delete(toBeRemoved[i])
    }

    toBeRemoved.splice(0, toBeRemoved.length)
  }

  // 更新UIInfo
  updateAllVUIInfo () {
    if (this.curCamera) {
      this.curCamera.updateMatrixWorld()
    }

    this.egoMap.forEach((t) => {
      const topic = t.replay ? 'self_logsim' : 'self'
      this.generateTrafficVUIInfo(topic, t)
    })

    this.egoTransparentMap.forEach((t) => {
      const topic = t.replay ? 'self_logsim' : 'self'
      this.generateTrafficVUIInfo(topic, t)
    })

    this.trafficVMap.forEach((t) => {
      this.generateTrafficVUIInfo('car', t)
    })

    this.trafficVReplayMap.forEach((t) => {
      this.generateTrafficVUIInfo('car_logsim', t)
    })

    this.trafficOMap.forEach((t) => {
      this.generateTrafficVUIInfo('staticObj', t)
    })

    this.trafficPMap.forEach((t) => {
      this.generateTrafficVUIInfo('dynamicObj', t)
    })

    this.trafficLightMap.forEach((t) => {
      this.generateTrafficVUIInfo('signlight', t)
    })

    this.removeRedundantVUIInfo()
    this.dispatchVUIInfo()
  }

  /**
   * 删除不再使用的交通对象
   * @param trafficLoc
   * @param replay
   */
  removeRedundantTraffic (trafficLoc, replay = false) {
    // 根据replay状态决定删除sim交通车还是log交通车
    // cars
    const trafficVMap = replay ? this.trafficVReplayMap : this.trafficVMap
    const toBeRemoved = []
    trafficVMap.forEach((car) => {
      if (!trafficLoc.carMap.has(car.id)) {
        toBeRemoved.push(car)
      }
    })

    for (let i = 0; i < toBeRemoved.length; ++i) {
      const ele = toBeRemoved[i]
      trafficVMap.delete(ele.id)
      this.scene.remove(ele.model)
      ele.model = null

      if (ele.selfTrj) {
        ele.selfTrj.dispose()
        ele.selfTrj = null
      }
    }

    toBeRemoved.splice(0, toBeRemoved.length)

    // obstacle
    this.trafficOMap.forEach((obstacle) => {
      if (!trafficLoc.obstacleMap.has(obstacle.id)) {
        toBeRemoved.push(obstacle)
      }
    })

    for (let i = 0; i < toBeRemoved.length; ++i) {
      const ele = toBeRemoved[i]
      this.trafficOMap.delete(ele.id)
      this.scene.remove(ele.model)
      ele.model = null
    }

    // dynamic obstacle
    this.trafficPMap.forEach((dynamicObstacle) => {
      if (!trafficLoc.dynamicObstacleMap.has(dynamicObstacle.id)) {
        toBeRemoved.push(dynamicObstacle)
      }
    })

    for (let i = 0; i < toBeRemoved.length; ++i) {
      const ele = toBeRemoved[i]
      this.trafficPMap.delete(ele.id)
      this.scene.remove(ele.model)
      ele.model = null
    }
  }

  // 处理交通流数据
  processTrafficLoc (trafficLoc, isTransparent = false, replay = false) {
    // traffic cars
    // 根据replay状态选择更新哪个Map
    const trafficVMap = replay ? this.trafficVReplayMap : this.trafficVMap
    let len = trafficLoc.cars.length
    for (let i = 0; i < len; ++i) {
      const car = trafficLoc.cars[i]
      if (car.id == undefined) {
        continue
      }

      let v = trafficVMap.get(car.id)
      if (!v) {
        v = new TrafficV()
        v.copyFromNet(car)
        // 暂时修复车头angle朝向问题，后期联系交通流同学一起修复
        const sv = this.sceneParser.findVehicleInMap(v.id)
        if (sv) {
          // 去parser查找boundingBox
          v.boundingBox = sv.boundingBox
        } else {
          const catalog = this.modelsManager.findCatalogByModelId('car', v.id)
          if (catalog) {
            v.boundingBox = catalog.catalogParams[0].boundingBox
          }
        }
        v.angle = +sv?.angle || 0
        v.isTransparent = isTransparent
        v.model = this.modelsManager.loadCarModelSync(v, isTransparent)
        this.scene.add(v.model)
        trafficVMap.set(v.id, v)
        v.model.visible = store.state.playerViewConfig.sceneViewConfig.trafficDynamic
      } else {
        v.copyFromNet(car)
        // 如果新的透明设定和以前的不同，则删掉以前的，重新生成一个
        if (isTransparent !== v.isTransparent) {
          this.scene.remove(v.model)
          v.model = this.modelsManager.loadCarModelSync(v, isTransparent)
          this.scene.add(v.model)
          v.isTransparent = isTransparent
        }
      }

      if (((v.lon == undefined) || (v.lon < -180) || (v.lon > 180)) &&
        ((v.lat == undefined) || (v.lat < -90) || (v.lat > 90))) {
        // 坐标非法，隐藏车
        if (v.model.visible) {
          v.visible = false
          v.model.visible = false
        }
      } else {
        const tmp = this.hadmap.ct.lonlat2xyz(v.lon, v.lat, 0)
        v.x = tmp[0]
        v.y = tmp[1]

        v.model.position.set(v.x, v.y, this.ModelHeiht)
        v.visible = true
        v.model.visible = !!(v.visible && store.state.playerViewConfig.sceneViewConfig.trafficDynamic)
      }

      if (!isNaN(v.heading)) {
        const absoluteHeading = 0
        const angle = v.angle || 0
        v.model.rotation.z = v.heading + absoluteHeading + angle
      }

      if (isNaN(v.length) || v.length <= 0 || !v.length) {
        v.length = 1
      }

      if (isNaN(v.width) || v.width <= 0 || !v.width) {
        v.width = 1
      }

      if (v.length != v.model.scale.x || v.width != v.model.scale.z) {
        if (!v.model.ignoreScale) {
          v.model.scale.set(v.length, v.width, 1)
        }
      }

      v.model.updateMatrixWorld(true)

      if (v.trajectory) {
        if (!v.selfTrj) {
          v.selfTrj = new Trajectory(this.scene, this.hadmap.ct)
        }

        v.selfTrj.render(v.trajectory.points)
      } else {
        if (v.selfTrj) {
          v.selfTrj.dispose()
          v.selfTrj = null
        }
      }
    }

    // 收集未登记的静态障碍物类型
    const unknownStaticObstacleTypeSet = new Set()
    // static obstacles
    len = trafficLoc.obstacles.length
    for (let i = 0; i < len; ++i) {
      if (trafficLoc.obstacles[i].id == undefined) {
        continue
      }
      // 类型为已知类型
      if (trafficLoc.obstacles[i].type in this.obstacleTypesMap) {
        // 障碍物
        let o = this.trafficOMap.get(trafficLoc.obstacles[i].id)
        if (o == undefined) {
          o = new TrafficO()
          o.copyFromNet(trafficLoc.obstacles[i])
          const so = this.sceneParser.findObstacleInMap(`${o.id}`)
          if (so) {
            o.boundingBox = so.boundingBox
          } else {
            console.log(`场景文件中未找到静态障碍物定义：${o.id}`)
            const catalog = this.modelsManager.findCatalogByModelId('obstacle', `${o.id}`)
            if (catalog) {
              o.boundingBox = catalog.catalogParams.boundingBox
            } else {
              o.boundingBox = {
                center: { x: 0, y: 0, z: 0.5 },
                dimensions: {
                  width: 1,
                  height: 1,
                  length: 1,
                },
              }
            }
          }
          o.model = this.modelsManager.loadObstacleModelSync(o)
          this.scene.add(o.model)
          this.trafficOMap.set(o.id, o)
          o.model.visible = store.state.playerViewConfig.sceneViewConfig.obstacle
        } else {
          o.copyFromNet(trafficLoc.obstacles[i])
        }

        if (((o.lon == undefined) || (o.lon < -180) || (o.lon > 180)) &&
          ((o.lat == undefined) || (o.lat < -90) || (o.lat > 90))) {
          if (o.model.visible == true) {
            o.visible = false
            o.model.visible = false
          }
        } else {
          const tmp = this.hadmap.ct.lonlat2xyz(o.lon, o.lat, 0)
          o.x = tmp[0]
          o.y = tmp[1]
          o.model.position.set(o.x, o.y, this.ModelHeiht)
          o.visible = true
          if (o.visible && store.state.playerViewConfig.sceneViewConfig.obstacle) {
            o.model.visible = true
          }
        }

        if (!isNaN(o.heading)) {
          o.model.rotation.z = o.heading
        }
        o.model.updateMatrixWorld(true)
      } else {
        if (trafficLoc.obstacles[i].type >= 700 && trafficLoc.obstacles[i].type < 800) {
          // 地图已知元素，TRAFFIC 发布给别的算法使用
        } else {
          unknownStaticObstacleTypeSet.add(trafficLoc.obstacles[i].type)
        }
      }
    }

    if (unknownStaticObstacleTypeSet.size) {
      console.warn(`unknown static obstacle types: ${[...unknownStaticObstacleTypeSet].join(',')}`)
    }

    // 收集未登记的动态障碍物类型
    const unknownDynamicObstacleTypeSet = new Set()
    // dynamic obstacles
    len = trafficLoc.dynamicObstacles.length
    for (let i = 0; i < len; ++i) {
      if (trafficLoc.dynamicObstacles[i].id == undefined) {
        continue
      }

      // 行人
      if (trafficLoc.dynamicObstacles[i].type >= 0) {
        let p = this.trafficPMap.get(trafficLoc.dynamicObstacles[i].id)
        if (p == undefined) {
          p = new TrafficDO()
          p.copyFromNet(trafficLoc.dynamicObstacles[i])
          const sp = this.sceneParser.findDynamicObstacleInMap(`${-p.id}`)
          p.angle = +sp?.angle || 0
          p.boundingBox = sp.boundingBox
          p.model = this.modelsManager.loadPedestrianModelSync(p)
          this.scene.add(p.model)
          this.trafficPMap.set(p.id, p)
          p.model.visible = store.state.playerViewConfig.sceneViewConfig.trafficDynamic
        } else {
          p.copyFromNet(trafficLoc.dynamicObstacles[i])
        }

        if (((p.lon == undefined) || (p.lon < -180) || (p.lon > 180)) &&
          ((p.lat == undefined) || (p.lat < -90) || (p.lat > 90))) {
          if (p.model.visible) {
            p.visible = false
            p.model.visible = false
          }
        } else {
          const tmp = this.hadmap.ct.lonlat2xyz(p.lon, p.lat, 0)
          p.x = tmp[0]
          p.y = tmp[1]
          p.model.position.set(p.x, p.y, this.ModelHeiht)
          p.visible = true
          if (p.visible && store.state.playerViewConfig.sceneViewConfig.trafficDynamic) {
            p.model.visible = true
          } else {
            p.model.visle = false
          }
        }

        if (!isNaN(p.heading)) {
          const angle = p.angle || 0
          p.model.rotation.z = p.heading + angle
        }

        p.model.updateMatrixWorld(true)
      } else {
        unknownDynamicObstacleTypeSet.add(trafficLoc.dynamicObstacles[i].type)
      }
    }

    if (unknownDynamicObstacleTypeSet.size) {
      console.warn(`unknown dynamic obstacle types: ${[...unknownDynamicObstacleTypeSet].join(',')}`)
    }

    // traffic lights
    len = trafficLoc.lights.length
    for (let i = 0; i < len; ++i) {
      if (trafficLoc.lights[i].id == undefined) {
        continue
      }

      let l = this.trafficLightMap.get(trafficLoc.lights[i].id)
      if (!l) {
        l = new TrafficLight()
        l.copyFromNet(trafficLoc.lights[i])
        this.modelsManager.loadSignArrowModels()
          .then((arrowObjects) => {
            l.createModels(arrowObjects)
          })
        this.scene.add(l.model)
        this.trafficLightMap.set(l.id, l)
        l.model.visible = store.state.playerViewConfig.sceneViewConfig.signlight
      } else {
        l.copyFromNet(trafficLoc.lights[i])
      }

      if (((l.lon == undefined) || (l.lon < -180) || (l.lon > 180)) &&
        ((l.lat == undefined) || (l.lat < -90) || (l.lat > 90))) {
        if (l.model.visible) {
          l.visible = false
          l.model.visible = false
        }
      } else {
        const tmp = this.hadmap.ct.lonlat2xyz(l.lon, l.lat, 0)
        l.x = tmp[0]
        l.y = tmp[1]
        l.model.position.set(l.x, l.y, 1)
        l.visible = true
        const rad = MathUtils.degToRad(l.heading - 90)
        l.model.rotation.set(0, 0, rad)
        l.model.visible = !!(l.visible && store.state.playerViewConfig.sceneViewConfig.signlight)
      }

      l.setColor(l.color)
      l.model.updateMatrixWorld(true)
    }

    this.removeRedundantTraffic(trafficLoc, replay)
    this.updateAllVUIInfo()
  }

  /**
   * 清除多余的交通车信息
   */
  emptyTrafficV () {
    this.trafficVMap.forEach((value) => {
      this.scene.remove(value.model)
      value.model = undefined

      if (value.selfTrj) {
        value.selfTrj.dispose()
        value.selfTrj = null
      }
    })
    this.trafficVMap.clear()

    this.trafficVReplayMap.forEach((value) => {
      this.scene.remove(value.model)
      value.model = undefined

      if (value.selfTrj) {
        value.selfTrj.dispose()
        value.selfTrj = null
      }
    })
    this.trafficVReplayMap.clear()
  }

  /**
   * 触发一次 VUI 信息更新
   */
  dispatchVUIInfo () {
    const trafficUIInfo = []

    const {
      wrapper: {
        clientWidth,
        clientHeight,
      },
    } = this
    this.trafficVUIInfoMap.forEach((vui, id) => {
      if (vui.pos.x >= 0 && vui.pos.x <= clientWidth &&
        vui.pos.y >= 0 && vui.pos.y <= clientHeight &&
        vui.pos.z < 1 && vui.visible === true) {
        trafficUIInfo.push(vui)
      }
    })

    PlayerGlobals.signals.trafficVPos.dispatch(trafficUIInfo)
  }

  /**
   * 清除 VUI 信息
   */
  emptyTrafficVUIInfo () {
    this.trafficVUIInfoMap.clear()
    this.dispatchVUIInfo()
  }

  /**
   * 清除交通灯信息
   */
  emptyTrafficLight () {
    this.trafficLightMap.forEach((value) => {
      this.scene.remove(value.model)
      value.model = undefined
    })

    this.trafficLightMap.clear()
  }

  /**
   * 清除行人信息
   */
  emptyTrafficP () {
    this.trafficPMap.forEach((value) => {
      this.scene.remove(value.model)
      value.model = undefined
    })

    this.trafficPMap.clear()
  }

  /**
   * 清除障碍物信息
   */
  emptyTrafficO () {
    this.trafficOMap.forEach((value) => {
      this.scene.remove(value.model)
      value.model = undefined
    })
    this.trafficOMap.clear()
  }
}

PlayerScene.StatusRunning = 'running'
PlayerScene.StatusSuccess = 'success'

export default PlayerScene
