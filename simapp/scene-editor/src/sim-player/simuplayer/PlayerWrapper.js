import Utility from '../sceneeditor/Utility'
import GlobalConfig from '../common/Config'
import { IgnoreHeight, RunMode } from '../common/Constant'
import { MessageBox } from '../sceneeditor/MessageBox'
import PlayerGlobals from './PlayerGlobals'
import PlayerScene from './PlayerScene'
import eventBus from '@/event-bus'
import store from '@/store'
import i18n from '@/locales'

let $trafficCanvas = null

/**
 * 播放器包装类
 */
class PlayerWrapper {
  constructor () {
    this.topics = ''
    this.timeSpan = ''
    this.notifyEnd = false
    this.callback = null
    this.playerScene = null
    this.elem = null
    this.boardElem = null
    this.parentElem = null
    this.canvasTextContext = null
    this.canvasTextPayload = null
    // 主车状态，来自Grading
    this.egoStatus = null
    this.curCamera = 'third'
    this.messageFormat = 'self'
  }

  /**
   * 初始化
   * @param configObj
   * @return {Promise<void>}
   */
  async init (configObj) {
    let canvasWrapper
    let parentElem
    if (!configObj.elem) {
      // 主场景渲染Canvas，交通流文字信息渲染Canvas
      canvasWrapper = document.querySelector('#WebGL-output')
      $trafficCanvas = document.querySelector('#trafficCanvas')
      parentElem = document.querySelector('#WebGL-container')
    } else {
      canvasWrapper = configObj.elem
      $trafficCanvas = configObj.boardElem
      parentElem = configObj.elem.parentElement
    }

    if (!canvasWrapper || !parentElem) {
      MessageBox.promptEditorUIMessage('error', 'no container elements!')
      console.error('no container elements!')
      return
    }

    this.elem = canvasWrapper
    this.boardElem = $trafficCanvas
    this.parentElem = parentElem

    GlobalConfig.runMode = RunMode.Local

    GlobalConfig.setIgnoreHeight(IgnoreHeight.Yes)
    GlobalConfig.setIsPlaying(true)

    const playerScene = new PlayerScene({ wrapper: canvasWrapper })
    window.playerScene = playerScene
    this.playerScene = playerScene

    const util = new Utility()
    const jobId = 0
    let sceneId = 0
    const status = 'running'

    sceneId = util.getParameterByName('sceneId')
    playerScene.init(canvasWrapper, sceneId, jobId, status)

    playerScene.prepareRender(canvasWrapper)
    playerScene.change2TopCamera()

    try {
      await this.playerScene.hadmap.loadHadmapinfo()
    } catch (err) {
      MessageBox.promptEditorUIMessage('error', err.message)
    }

    this.playerScene.setPlayCallback(this.callback)

    const useRequestAnimation = true
    const fps = 30
    const interval = 1000 / fps
    if (useRequestAnimation) {
      let now
      let then = Date.now()
      let delta = 0

      const renderPlayerScene = () => {
        requestAnimationFrame(renderPlayerScene)

        now = Date.now()
        delta = now - then
        if (delta > interval) {
          then = now - (delta % interval)
          this.playOneFrame()
        }
        const tmp = playerScene.flyClock.getDelta()
        playerScene.flyControls.update(tmp)
      }

      renderPlayerScene()
    } else {
      const renderPlayerScene = () => {
        this.playOneFrame()
        const tmp = playerScene.flyClock.getDelta()
        playerScene.flyControls.update(tmp)
      }
      setInterval(() => {
        renderPlayerScene()
      }, interval)
    }

    this.setTrafficCanvas()
    this.bindEvents()

    PlayerGlobals.signals.trafficVPos.removeAll()
    PlayerGlobals.signals.trafficVPos.add((payload) => {
      this.canvasTextPayload = payload
    })
    // 接收grading消息，主要用于主车的加速度显示
    eventBus.$on('grading-kpi', (gradingKPIList) => {
      if (gradingKPIList?.detail.length) {
        this.egoStatus = gradingKPIList.detail[gradingKPIList.detail.length - 1]
      }
    })
  }

  /**
   * 播放一帧
   */
  playOneFrame () {
    const { playerScene } = this
    if (playerScene.localReceiver) {
      const lastFrame = playerScene.localReceiver.applyLatestData()
      lastFrame && console.debug('render', lastFrame)
    }
    if (this.canvasTextPayload) {
      this.renderCanvasText(this.canvasTextPayload)
      this.canvasTextPayload = null
    }
    playerScene.renderScene()
  }

  resetScene () {
    this.playerScene.resetScene()
  }

  finalize () {
    if (this.playerScene) {
      this.playerScene.unbindMessageReceiver()
    }
  }

  dispose () {
    this.finalize()
  }

  locateObject (payload) {
    this.playerScene.locateObject(payload)
  }

  setObjectsVisible (type) {
    this.playerScene.showObject(type)
  }

  resetAllObjectVisible () {
    this.playerScene.resetAllObjectVisible()
  }

  resize (width, height) {
    if (height === 0) {
      height = 1
    }
    const { playerScene } = this
    playerScene.aspect = width / height
    playerScene.freeCamera.aspect = playerScene.aspect
    playerScene.freeCamera.updateProjectionMatrix()
    playerScene.renderer.setSize(width, height)

    playerScene.cameraFPS.aspect = playerScene.aspect
    playerScene.cameraFPS.updateProjectionMatrix()

    playerScene.cameraFollow.aspect = playerScene.aspect
    playerScene.cameraFollow.updateProjectionMatrix()

    // playerScene.cameraTop.left = -1 * playerScene.aspect * playerScene.topSceneRange
    // playerScene.cameraTop.right = playerScene.aspect * playerScene.topSceneRange
    // playerScene.cameraTop.top = playerScene.topSceneRange
    // playerScene.cameraTop.bottom = -1 * playerScene.topSceneRange
    // playerScene.cameraTop.updateProjectionMatrix()

    playerScene.zoomCameraTop(playerScene.topSceneZoomFactor)

    playerScene.halfWidth = width / 2
    playerScene.halfHeight = height / 2

    this.setTrafficCanvas()
    this.playerScene.updateAllVUIInfo()
    playerScene.renderScene()
  }

  /**
   * 设置交通流文字信息渲染Canvas
   */
  setTrafficCanvas () {
    const clientRect = this.parentElem.getBoundingClientRect()

    this.boardElem.width = clientRect.width
    this.boardElem.height = clientRect.height
    this.boardElem.style.width = `${clientRect.width}px`
    this.boardElem.style.height = `${clientRect.height}px`

    this.canvasTextContext = this.boardElem.getContext('2d')
    this.canvasTextContext.font = '12px sans-serif'
    this.canvasTextContext.fillStyle = '#ffffe6'
    this.canvasTextContext.shadowBlur = 2
    this.canvasTextContext.shadowColor = 'black'
  }

  /**
   * 绑定事件
   */
  bindEvents () {
    const self = this

    function onDocumentButtonDown (event) {
      self.playerScene.onMouseButtonDown(event)
    }

    function onDocumentButtonUp (event) {
      self.playerScene.onMouseButtonUp(event)
    }

    function onDocumentMouseMove (event) {
      self.playerScene.onMouseMove(event)
    }

    function onWheel (wheelEvent) {
      self.playerScene.onMouseWheel(wheelEvent)
    }

    function onContextMenu (event) {
      event.preventDefault()
    }

    self.boardElem.removeEventListener('mousedown', onDocumentButtonDown)
    self.boardElem.removeEventListener('mouseup', onDocumentButtonUp)
    self.boardElem.removeEventListener('mousemove', onDocumentMouseMove)
    self.boardElem.removeEventListener('wheel', onWheel)
    self.boardElem.removeEventListener('contextmenu', onContextMenu)

    self.boardElem.addEventListener('mousedown', onDocumentButtonDown)
    self.boardElem.addEventListener('mouseup', onDocumentButtonUp)
    self.boardElem.addEventListener('mousemove', onDocumentMouseMove)
    self.boardElem.addEventListener('wheel', onWheel)
    self.boardElem.addEventListener('contextmenu', onContextMenu)
  }

  /**
   * 渲染Canvas文字信息
   * @param payload
   */
  renderCanvasText (payload) {
    this.canvasTextContext.clearRect(0, 0, this.boardElem.width, this.boardElem.height)
    const {
      simMainCar,
      simTraffic,
      others,
    } = store.state.playerViewConfig.cityMonitor

    payload.forEach((value) => {
      const { id, v, pos: { x, y } } = value
      let text = ''
      if (['self', 'trailer', 'self_logsim', 'trailer_logsim'].includes(id) || id.startsWith('self_')) {
        // 主车相关
        const { dist } = value
        let idStr = ''
        let velocity = ''
        let acceleration = ''
        let lon = ''
        let lat = ''
        let alt = ''
        let distStr = ''
        if (dist !== undefined && dist !== (2 ** 31 - 1)) { // INT32_MAX
          distStr = `distHead:${dist.toFixed(2)}m`
        }
        if (simMainCar.id) {
          idStr = `${id}`
        }
        if (simMainCar.v) {
          velocity = `v:${v}m/s`
        }
        if (simMainCar.lonlat) {
          lon = `lon:${value.lon}°`
          lat = `lat:${value.lat}°`
          alt = `alt:${value.alt}m`
        }
        if ((id === 'self' || id.startsWith('self_')) && this.egoStatus && simMainCar.acc) {
          const acc = this.egoStatus?.acceleration || 0
          acceleration = `acc:${acc.toFixed(3)}m/s²`
        }
        text = [idStr, velocity, acceleration, lon, lat, alt, distStr].filter(s => !!s).join(' ')
      } else if (id.startsWith('l_') || id.startsWith('so_')) {
        // 交通灯，静态障碍物
        let idStr = ''
        if (others.id) {
          idStr = `${id}`
        }
        text = `${idStr}`
      } else if (id.startsWith('v_') || id.startsWith('do_')) {
        // 交通车
        let idStr = ''
        let velocity = ''
        let acceleration = ''
        let lon = ''
        let lat = ''
        let alt = ''
        let vR = ''
        let aR = ''
        if (simTraffic.id) {
          idStr = `${id}`
        }
        if (simTraffic.v) {
          velocity = `velocity:${value.showAbsVelocity || 0}m/s`
        }
        if (simTraffic.acc) {
          acceleration = `acc:${value.showAbsAcc || 0}m/s²`
        }
        if (simTraffic.vr) {
          vR = `relative-v:${value.showRelativeVelocity || 0}m/s`
        }
        if (simTraffic.ar) {
          aR = `relative-acc:${value.showRelativeAcc || 0}m/s²`
        }
        if (simTraffic.lonlat) {
          lon = `lon:${value.lon}°`
          lat = `lat:${value.lat}°`
          alt = `alt:${value.alt}m`
        }
        text = [idStr, velocity, acceleration, vR, aR, lon, lat, alt].filter(s => !!s).join(' ')
      } else if (id.startsWith('do_')) {
        // 动态障碍物，只有速度
        let idStr = ''
        let velocity = ''
        let lon = ''
        let lat = ''
        let alt = ''
        if (simTraffic.id) {
          idStr = `${id}`
        }
        if (simTraffic.v) {
          velocity = `abs:${value.showAbsVelocity || 0}m/s`
        }
        if (simTraffic.lonlat) {
          lon = `lon:${value.lon}°`
          lat = `lat:${value.lat}°`
          alt = `alt:${value.alt}m`
        }
        text = [idStr, velocity, lon, lat, alt].filter(s => !!s).join(' ')
      } else {
        // 静态障碍物
        let idStr = ''
        let velocity = ''
        if (simTraffic.id) {
          idStr = `${id}`
        }
        if (simTraffic.v) {
          velocity = `abs:${value.showAbsVelocity}m/s`
        }
        text = [idStr, velocity].filter(s => !!s).join(' ')
      }
      this.canvasTextContext.fillText(text, x, y)
    })
  }

  /**
   * 获取所有信号
   * @return {{
   * sceneLoaded: *,
   * locateObject: *,
   * trafficVPos: *,
   * mapLoaded: *,
   * playControllerStatus: *,
   * promptMessage: *,
   * log2worldChanged: *,
   * cameraChanged: *,
   * zoomView: *,
   * monitorUIInfo: *
   * }}
   */
  allSignals () {
    return PlayerGlobals.signals
  }

  updateHadmapInfo () {
    return this.playerScene.hadmap.loadHadmapinfo()
  }

  /**
   * 加载场景
   * @param id
   * @return {Promise<*[]>}
   */
  async loadScene (id) {
    if (!this.playerScene) {
      const msg = 'Error: start load scene before init.'
      console.error(msg)
      throw msg
    }

    GlobalConfig.setIgnoreHeight(IgnoreHeight.Yes)
    GlobalConfig.setIsPlaying(true)

    this.playerScene.clearScene()
    await this.playerScene.loadScene(id)
    return this.getAllObjects()
  }

  /**
   * 加载地图
   * @param hadmapName
   * @return {*}
   */
  loadHadmap (hadmapName) {
    this.playerScene.clearScene()
    return this.playerScene.loadHadmap(hadmapName)
  }

  /**
   * 获取所有对象
   * @return {*[]}
   */
  getAllObjects () {
    let all = []
    const planners = this.playerScene.sceneParser.planners.map(planner => planner.plannerFormData())
    all = all.concat(planners)
    // 暂时只用到主车，后面需要再加
    return all
  }

  /**
   * 获取地图信息
   * @return {any}
   */
  getMapInfo () {
    return Object.assign({}, this.playerScene.sceneParser.mapfile)
  }

  /**
   * 获取所有相机
   * @return {{
   * curCamera: string,
   * cameras: [
   * {name: TranslateResult, id: string},
   * {name: TranslateResult, id: string},
   * {name: TranslateResult, id: string},
   * {name: TranslateResult, id: string}
   * ]
   * }}
   */
  getCameras () {
    return {
      curCamera: this.curCamera,
      cameras: [
        { id: 'third', name: i18n.t('view.third') },
        { id: 'fps', name: i18n.t('view.fps') },
        { id: 'top', name: i18n.t('view.top') },
        { id: 'debugging', name: i18n.t('view.car') },
      ],
    }
  }

  /**
   * 切换相机
   * @param id
   */
  changeCamera (id) {
    switch (id) {
      case 'third':
      // 跟车视角
        this.playerScene.change2FollowCamera()
        break
      case 'fps':
      // 司机视角
        this.playerScene.change2FPSCamera()
        break
      case 'top':
      // 顶视角
        this.playerScene.change2TopCamera()
        break
      case 'debugging':
      // 自由视角
        this.playerScene.change2FreeCamera()
        break
    }
    this.playerScene.renderScene()
  }

  zoom (factor) {
    return this.playerScene.zoom(factor)
  }

  processMessage (msg) {
    this.playerScene.localReceiver.processMsg(msg)
  }

  playerLastFrame () {
    this.playOneFrame()
  }
}

window.PlayerWrapper = PlayerWrapper

export default PlayerWrapper
