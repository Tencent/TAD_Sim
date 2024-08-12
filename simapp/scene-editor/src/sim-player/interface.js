import CanvasWrapper from './sceneeditor/CanvasWrapper'
import PlayerWrapper from './simuplayer/PlayerWrapper'

class SimScenario {
  constructor () {
    this.wrapper = new CanvasWrapper()
    this.parentElem = null
    this.signals = this.wrapper.allSignals()
  }

  /**
   * 注册容器
   * @param {HTMLElement} payload.el 容器element
   * @return {Promise<boolean>}
   */
  async register (payload) {
    this.parentElem = payload.el
    return this.wrapper.init({
      ele: this.parentElem,
    })
  }

  /**
   * 创建场景
   * @param {string} mapId map名称
   * @param {string} name 场景名称
   * @param {string} desc 场景工况
   * @return {Promise<boolean>}
   */
  async create (mapId, name, desc) {
    await this.wrapper.reinit(this.parentElem)
    this.wrapper.setSceneName(name)
    this.wrapper.setSceneDesc(desc)
    return this.wrapper.loadHadmapOnNewScene(mapId)
  }

  /**
   * 读取场景
   * @param {string} id 场景名称
   * @return {Promise<[]>} 场景元素列表
   */
  async load (id) {
    await this.wrapper.openScene(id, this.parentElem)
    return this.wrapper.getAllObjects()
  }

  async unload () {
    return this.wrapper.reinit(this.parentElem)
  }

  /**
   * 通过类型获取物体所有信息
   * @param {string} type 物体类型
   * @return {[]} 类型为type的场景元素列表
   */
  getObjectsByType (type) {
    return this.wrapper.getObjectsInfoByType(type)
  }

  /**
   * 校验当前场景是否可以保存
   * @return {{result: boolean, message: string}}
   result：校验是否通过，message：错误信息
   */
  canSave () {
    return this.wrapper.canSceneSave()
  }

  /**
   * 设置工况说明
   * @param {string} desc 工况说明
   */
  setSceneDesc (desc) {
    this.wrapper.setSceneDesc(desc)
  }

  /**
   * 保存场景
   * @param {string} id 场景编号
   * @param {string} name 场景名称
   */
  save (id, name, ext) {
    return this.wrapper.saveScene(id, name, ext)
  }

  /**
   * 另存为场景
   * @param {string} name 场景名称
   */
  saveAs (name, ext) {
    return this.wrapper.saveScene(-1, name, ext)
  }

  /**
   * 获取生成信息
   * @returns {object} 生成信息
   */
  getGenerateInfo () {
    return this.wrapper.getGenerateInfo()
  }

  /**
   * 保存传感器配置
   * @param {object} config 传感器配置
   */
  applySensor (config) {
    this.wrapper.setSensorInfo(config)
  }

  /**
   * 获取传感器配置
   * @returns {object} 传感器配置
   */
  getSensorInfo () {
    return this.wrapper.getSensorInfo()
  }

  /**
   * 获取界面部分UI信息
   * @returns
   */
  getUIStateInfo () {
    return this.wrapper.getUIStateInfo()
  }

  /**
   * 获取界面所有道路id及其下属section、laneId
   * @returns
   */
  getRoadobj () {
    return this.wrapper.getRoadobj()
  }

  /**
   * 获取界面所有所有触发事件
   * @returns
   */
  getSceneevents () {
    return this.wrapper.getSceneevents()
  }

  /**
   * 保存grading配置
   * @param {string} config grading配置
   */
  applyGrading (config) {
    this.wrapper.setGradingInfo(config)
  }

  /**
   * 获取grading配置
   * @returns {string} grading配置
   */
  getGradingInfo () {
    return this.wrapper.getGradingInfo()
  }

  /**
   * 保存环境配置
   * @param {object} config 环境配置
   */
  applyEnvironment (config) {
    this.wrapper.setEnvironmentInfo(config)
  }

  /**
   * 获取环境配置
   * @returns {object} 环境配置
   */
  getEnvironmentInfo () {
    return this.wrapper.getEnvironmentInfo()
  }

  /**
   * 保存交通流配置
   * @param {object} config
   */
  applyTrafficAI (config) {
    return this.wrapper.applyTrafficAI(config)
  }

  /**
   * 获取交通流配置
   * @returns {object}
   */
  getTrafficAIConfig () {
    return this.wrapper.getTrafficAIConfig()
  }

  applyTrafficNormal (config) {
    return this.wrapper.applyTrafficNormal()
  }

  getL3StateMachineRawData () {
    return this.wrapper.getL3StateMachineRawData()
  }

  setL3StateMachineData (config) {
    return this.wrapper.setL3StateMachineData(config)
  }

  getL3StateMachineData () {
    return this.wrapper.getL3StateMachineData()
  }

  /**
   * 获取地图信息,返回地图的名称、参考点经纬度及高度
   * @returns {{mapName: string, lon: string, lat: string, alt: string}}
   */
  getMapInfo () {
    return this.wrapper.getMapInfo()
  }

  /**
   * 获取车道宽度
   * @param {object} payload
   * @param {number} payload.routeID 道路id
   * @param {number} payload.laneID 车道线id
   * @returns {number}
   */
  getLaneWidth ({ routeId, laneId }) {
    return this.wrapper.getLaneWidth(routeId, laneId)
  }

  /**
   * 获取最大偏移值
   * @param {object} payload
   * @param {number} payload.type 元素类型
   * @param {number} payload.routeID 道路id
   * @param {number} payload.laneID 车道线id
   * @returns {number}
   */
  getMaxOffset (payload) {
    return this.wrapper.getMaxOffset(payload)
  }

  /**
   * 获取车道长度
   * @param {object} payload
   * @param {number} payload.routeID 道路id
   * @param {number} payload.laneID 车道线id
   * @returns {number}
   */
  getLaneLength ({ routeId, laneId }) {
    return this.wrapper.getLaneLength(routeId, laneId)
  }

  /**
   * 获取车道列表
   * @param {object} payload
   * @param {number} payload.routeID 道路id
   * @returns {[]}
   */
  getLaneList ({ routeId }) {
    return this.wrapper.getLaneListByRoute(routeId)
  }

  /**
   * 获取场景中某一辆车所在lane的最大长度
   * @param vehicleId 车辆Id
   * @returns Number 长度数值
   */
  getVehicleMaxShift (vehicleId) {
    return this.wrapper.getVehicleMaxShift(vehicleId)
  }

  /**
   * 获取场景中物体所在lane的最大偏移
   * @param type 物体类型
   * @param id 物体id
   * @returns Number 长度数值
   */
  getObjectMaxOffset (type, id) {
    return this.wrapper.getObjectMaxOffset(type, id)
  }

  /**
   * 获取场景中所有车辆的ID
   * @returns Array 车辆ID数组
   */
  getVehicleIDs () {
    return this.wrapper.getVehicleIDs()
  }

  /**
   * 获取场景中跟随vehicleId的车辆IDs
   * @param vehicleId 车辆Id
   * @returns Array 车辆ID数组
   */
  getFollowedVehicleIDs (vehicleId) {
    return this.wrapper.getFollowedVehicleIDs(vehicleId)
  }

  /**
   * 获取场景中可以被vehicleId跟随的交通流车辆ID
   * @param vehicleId 车辆Id
   * @returns Array 车辆ID数组
   */
  getCanBeFollowedVehicleIDs (vehicleId) {
    return this.wrapper.getCanBeFollowedVehicleIDs(vehicleId)
  }

  /**
   * 添加场景元素
   * @param {object} payload 场景元素
   * @returns {Promise<undefined|null|any|string|{id: *, accs: *}>}
   */
  addObject (payload) {
    return this.wrapper.addObject(payload)
  }

  syncPlannerVisual (id) {
    return this.wrapper.syncPlannerVisual(id)
  }

  /**
   * 复制场景元素
   * @param {object} payload 场景元素
   * @returns {Promise<undefined|null|any|string|{id: *, accs: *}>}
   */
  copyObject (payload) {
    return this.wrapper.copyObject(payload)
  }

  /**
   * 移除场景元素
   * @param {object} payload 场景元素
   */
  removeObject (payload) {
    return this.wrapper.removeObject(payload)
  }

  /**
   * 设置元素初始角度
   * @param {object} payload 场景元素
   */
  setAngle (payload) {
    return this.wrapper.setAngle(payload)
  }

  /**
   * 场景元素是否可以被移除
   * @param {object} payload 场景元素
   */
  canObjectRemoved (payload) {
    return this.wrapper.canObjectRemoved(payload)
  }

  /**
   * 修改场景元素
   * @param {object} payload 场景元素（位置相关属性）
   */
  modifyObject (payload) {
    return this.wrapper.modifyObject(payload)
  }

  /**
   * 修改场景元素
   * @param {object} payload 场景元素（非位置相关属性）
   */
  modifyObjectParams (payload) {
    return this.wrapper.modifyObjectParams(payload)
  }

  /**
   * 相机聚焦到某个场景元素
   * @param {object} payload 场景元素
   */
  locateObject (payload) {
    this.wrapper.locateObject(payload)
  }

  /**
   * 开始测距功能
   */
  setMeasurement () {
    this.wrapper.setMeasurement()
  }

  /**
   * 删除一个测距点
   */
  removeMeasurementPoint (payload) {
    this.wrapper.removeMeasurementPoint(payload)
  }

  /**
   * 删除一条测距线
   */
  removeMeasurement (payload) {
    this.wrapper.removeMeasurement(payload)
  }

  setEnd (type, id) {
    switch (type) {
      case 'planner':
        return this.wrapper.setEgoCarEnd(id)
      case 'car':
        return this.wrapper.setVehicleEnd(id)
      case 'moto':
      case 'bike':
      case 'pedestrian':
      case 'animal':
      case 'machine':
        return this.wrapper.setPedestrianEnd(id)
      default:
        console.error('unsupported type')
    }
  }

  removePoint (type, payload) {
    const { id, index } = payload
    switch (type) {
      case 'planner':
        return this.wrapper.removePlannerEnd(id, index)
      case 'car':
        return this.wrapper.removeVehicleEnd(id, index)
      case 'moto':
      case 'bike':
      case 'pedestrian':
      case 'animal':
      case 'machine':
        return this.wrapper.removePedestrianEnd(id, index)
      default:
        console.error('unsupported type')
    }
  }

  /**
   * 批量生成场景
   * @param {string} name 生成的场景前缀
   * @param {object} payload 配置参数
   *
   {
   type: "single", // single, all
   vehicle:{
   id: 1,
   pos:{
   start: 0,
   end: 20,
   sep: 3
   },
   velocity:{
   start: 0,
   end: 12,
   sep: 2
   }
   },
   ego:{
   velocity:{
   start: 0,
   end: 10,
   sep: 3
   }
   }
   }
   */
  paramScene (id, prefix, payload) {
    return this.wrapper.paramScene(id, prefix, payload)
  }

  /**
   * 设置界面缩放
   * @param {number} factor 相对缩放比
   * @returns {undefined}
   */
  zoom (factor) {
    console.log('editor zoom ', factor)
    return this.wrapper.zoom(factor)
  }

  /**
   * @desc 修改容器大小
   * @param {number} width
   * @param {number} height
   */
  resize (width, height) {
    this.wrapper.resize(width, height)
  }

  /**
   * @desc
   * @param {} type
   */
  setCamera (type) {
    this.wrapper.setCamera(type)
  }

  canOpenScenario (scenario) {
    return this.wrapper.canOpenScenario(scenario)
  }

  updateHadmapInfo () {
    this.wrapper.updateHadmapInfo()
  }

  getState () {
    return this.wrapper.getState()
  }

  // 获取地图所有mesh
  getMapGroup () {
    return this.wrapper.getMapGroup()
  }

  getSignlightPlans () {
    return this.wrapper.getSignlightPlans()
  }

  setSignlightPlans (data) {
    this.wrapper.setSignlightPlans(data)
  }

  getSnap () {
    return this.wrapper.getSnap()
  }

  setSnap (val) {
    return this.wrapper.setSnap(val)
  }

  updateModel (data) {
    return this.wrapper.updateModel(data)
  }
}

class SimPlayer {
  constructor () {
    this.wrapper = new PlayerWrapper()
    this.parentElem = null
    this.boardElem = null
    this.signals = this.wrapper.allSignals()
  }

  /**
   * 注册容器
   * @param {object} conf
   * @param {HTMLElement} conf.el 容器
   * @param {HTMLCanvasElement} conf.boardEl 交通流容器
   * @returns {Promise<void>}
   */
  async register (conf) {
    const { el, boardEl } = conf
    this.parentElem = el
    this.boardElem = boardEl

    const configObj = {
      boardElem: this.boardElem,
      elem: this.parentElem,
    }

    await this.wrapper.init(configObj)
  }

  /**
   * 更新本地地图文件信息
   */
  updateHadmapInfo () {
    return this.wrapper.updateHadmapInfo()
  }

  /**
   * 读取场景
   * @param {string} scenarioId 场景名称
   * @returns {Promise<void>}
   */
  setScenario (scenarioId) {
    return this.wrapper.loadScene(scenarioId)
  }

  /**
   * 不读取场景，直接设置地图名称
   * @param {string} hadmapName 地图名称
   * @returns {Promise<void>}
   */
  loadHadmap (hadmapName) {
    return this.wrapper.loadHadmap(hadmapName)
  }

  /**
   * 连接到服务器
   * @param {string} addr 服务器地址
   * @returns { void }
   */
  // connectServer (addr) {
  //   this.wrapper.connectServer(addr)
  // }

  // /**
  //  * 播放
  //  * @returns { void }
  //  */
  // play () {
  //   this.wrapper.play()
  // }
  //
  // /**
  //  * 暂停
  //  * @returns { void }
  //  */
  // pause () {
  //   this.wrapper.pause()
  // }
  //
  // /**
  //  * 停止
  //  * @returns { void }
  //  */
  // stop () {
  //   this.wrapper.stop()
  // }
  //
  // /**
  //  * 单步播放
  //  * @returns { void }
  //  */
  // step () {
  //   this.wrapper.step()
  // }
  //
  // /**
  //  * 恢复播放
  //  * @returns { void }
  //  */
  // resume () {
  //   this.wrapper.resume()
  // }

  /**
   * 获取地图信息,返回地图的名称、参考点经纬度及高度
   * @returns {{mapName: string, lon: string, lat: string, alt: string}}
   */
  getMapInfo () {
    return this.wrapper.getMapInfo()
  }

  /**
   * 获取所有视角
   * @returns {}
   * {
   *  curCamera: string,
   *  cameras: [{name: string, id: string}]
   * }
   */
  getCameras () {
    return this.wrapper.getCameras()
  }

  /**
   * 切换视角
   * @param {string} id 视角
   */
  setCamera (id) {
    this.wrapper.changeCamera(id)
  }

  /**
   * 相机聚焦到某个场景元素
   * @param {object} payload 场景元素
   */
  locateObject (payload) {
    this.wrapper.locateObject(payload)
  }

  processMessage (msg) {
    this.wrapper.processMessage(msg)
  }

  setObjectsVisible (type, visible) {
    this.wrapper.setObjectsVisible(type, visible)
  }

  resetAllObjectVisible () {
    this.wrapper.resetAllObjectVisible()
  }

  /**
   * 设置界面缩放
   * @param {number} factor 相对缩放比
   * @returns {undefined}
   */
  zoom (factor) {
    console.log('editor zoom ', factor)
    return this.wrapper.zoom(factor)
  }

  /**
   * @desc 修改容器大小
   * @param {number} width
   * @param {number} height
   */
  resize (width, height) {
    this.wrapper.resize(width, height)
  }

  resetScene () {
    this.wrapper.resetScene()
  }

  // 播放可能存在的已缓存但未播放的帧
  playerLastFrame () {
    this.wrapper.playerLastFrame()
  }

  /**
   * @desc 释放资源
   */
  dispose () {
    this.wrapper.dispose()
  }
}

class SimViz {
  constructor () {
    this.scenario = new SimScenario()
    this.player = new SimPlayer()
  }

  /**
   * 判断地图是否存在
   * @param {*} scenario
   * @returns
   */
  hasMap (scenario) {
    return this.scenario.canOpenScenario(scenario)
  }
}

export default SimViz
