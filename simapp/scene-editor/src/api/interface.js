import SimViZ from 'sim-player/interface'
import { copyScenario } from '@/api/scenario'

// 在 interface 中添加中间层完成与 simViZ 的 scenario 、player 的交互
const simViz = new SimViZ()

/**
 * 转换事件名称
 * @type {object}
 */

const conversion = {
  editor: {
    'object.selectedChanged': 'pickedObject',
    'object.deselected': 'cancelSelectObject2UI',
    'object.selectedPropertyChanged': 'dropModifyObject',
    'positionChanged': 'mousePosition2UI',
    'distanceMeasurement.changed': 'measurementChanged',
  },
  player: {
    locateObject: 'locateObject',
  },
}

/**
 * 编辑器相关功能。
 * @type {object}
 */

const editor = {
  /**
   * 注册事件监听器。
   * @param {object} payload - 载荷。
   * @returns {object} 返回注册信息。
   */
  register (payload) {
    return simViz.scenario.register(payload)
  },
  /**
   * 添加事件监听器。
   * @param {string} type - 事件类型。
   * @param {Function} listener - 监听器函数。
   */
  addEventListener (type, listener) {
    simViz.scenario.signals[conversion.editor[type] || type].add(listener)
  },
  /**
   * 移除事件监听器。
   * @param {string} type - 事件类型。
   * @param {Function} listener - 监听器函数。
   */
  removeEventListener (type, listener) {
    simViz.scenario.signals[conversion.editor[type] || type].remove(listener)
  },

  getSignals (type) {
    return simViz.scenario.signals[type]
  },

  /**
   * 改变场景相机
   * @param {string} type - 相机类型。
   */
  setCamera (type) {
    simViz.scenario.setCamera(type)
  },

  /**
   * 获取是否紧贴道路中线
   * @return {*}
   */
  getSnap () {
    return simViz.scenario.getSnap()
  },

  /**
   * 设置是否紧贴道路中线
   * @return {*}
   */
  setSnap (val) {
    return simViz.scenario.setSnap(val)
  },

  scenario: {
    /**
     * 创建场景
     * @param {string} mapId map名称
     * @param {string} name 场景名称
     * @param {string} desc 场景工况
     * @return {Promise<boolean>}
     */
    add ({ mapId, name, info }) {
      return simViz.scenario.create(mapId, name, info)
    },

    /**
     * 读取场景
     * @param {string} id 场景名称
     * @return {Promise<[]>} 场景元素列表
     */
    open (id) {
      return simViz.scenario.load(id)
    },
    unload () {
      return simViz.scenario.unload()
    },

    /**
     * 获取信控方案
     * @return {*}
     */
    getSignlightPlans () {
      return simViz.scenario.getSignlightPlans()
    },

    /**
     * 设置信控方案
     * @return {*}
     */
    setSignlightPlans (data) {
      simViz.scenario.setSignlightPlans(data)
    },

    /**
     * 复制场景
     * @param {{name: string, newName: string}[]}payload
     */
    copy (payload) {
      return copyScenario(payload)
    },

    /**
     * 保存场景
     * @param {string} id 场景编号
     * @param {string} name 场景名称
     */
    save (id, name, ext) {
      return simViz.scenario.save(id, name, ext)
    },

    /**
     * 另存为场景
     * @param {string} name 场景名称
     */
    saveAs (name, ext) {
      return simViz.scenario.saveAs(name, ext)
    },

    /**
     * 获取最大偏移值
     * @param {object} payload
     * @param {number} payload.type 元素类型
     * @param {number} payload.routeID 道路id
     * @param {number} payload.laneID 车道线id
     * @returns {number}
     */
    getMaxOffset (payload) {
      return simViz.scenario.getMaxOffset(payload)
    },

    /**
     * 获取车道宽度
     * @param {object} payload
     * @param {number} payload.routeID 道路id
     * @param {number} payload.laneID 车道线id
     * @returns {number}
     */
    getLaneWidth (payload) {
      return simViz.scenario.getLaneWidth(payload)
    },

    /**
     * 获取车道长度
     * @param {object} payload
     * @param {number} payload.routeID 道路id
     * @param {number} payload.laneID 车道线id
     * @returns {number}
     */
    getLaneLength (payload) {
      return simViz.scenario.getLaneLength(payload)
    },

    /**
     * 获取车道列表
     * @param {object} payload
     * @param {number} payload.routeID 道路id
     * @returns {[]}
     */
    getLaneList (payload) {
      return simViz.scenario.getLaneList(payload)
    },

    /**
     * 保存交通流配置
     * @param {object} config
     */
    applyTrafficAIConfig (payload) {
      return simViz.scenario.applyTrafficAI(payload)
    },

    /**
     * 获取交通流配置
     * @returns {object}
     */
    getTrafficAIConfig () {
      return simViz.scenario.getTrafficAIConfig()
    },

    /**
     * 获取传感器配置
     * @returns {object} 传感器配置
     */
    getCurrentSensor () {
      return simViz.scenario.getSensorInfo()
    },

    /**
     * 保存传感器配置
     * @param {object} config 传感器配置
     */
    setCurrentSensor (payload) {
      return simViz.scenario.applySensor(payload)
    },

    /**
     * 获取grading配置
     * @returns {string} grading配置
     */
    getCurrentKPIConfig () {
      return simViz.scenario.getGradingInfo()
    },

    /**
     * 保存grading配置
     * @param {string} config grading配置
     */
    setCurrentKPIConfig (payload) {
      return simViz.scenario.applyGrading(payload)
    },

    /**
     * 获取环境配置
     * @returns {object} 环境配置
     */
    getCurrentEnvironment () {
      return simViz.scenario.getEnvironmentInfo()
    },

    /**
     * 保存环境配置
     * @param {object} config 环境配置
     */
    setCurrentEnvironment (payload) {
      return simViz.scenario.applyEnvironment(payload)
    },
    getCurrentFSM () {
      return simViz.scenario.getFSM()
    },
    setCurrentFSM (payload) {
      return simViz.scenario.applyFSM(payload)
    },

    /**
     * 设置界面缩放
     * @param {number} factor 相对缩放比
     * @returns {undefined}
     */
    zoom (level) {
      return simViz.scenario.zoom(level)
    },

    /**
     * 获取地图信息,返回地图的名称、参考点经纬度及高度
     * @returns {{mapName: string, lon: string, lat: string, alt: string}}
     */
    getMapInfo () {
      return simViz.scenario.getMapInfo()
    },

    /**
     * @desc 修改容器大小
     * @param {number} width
     * @param {number} height
     */
    resize (width, height) {
      simViz.scenario.resize(width, height)
    },

    /**
     * 校验当前场景是否可以保存
     * @return {{result: boolean, message: string}}
     result：校验是否通过，message：错误信息
     */
    validate () {
      return simViz.scenario.canSave()
    },
    setEnd (type, id) {
      return simViz.scenario.setEnd(type, id)
    },
    removePoint (type, payload) {
      return simViz.scenario.removePoint(type, payload)
    },

    /**
     * 开始测距功能
     */
    setDistanceMeasurement () {
      return simViz.scenario.setMeasurement()
    },

    /**
     * 删除一个测距点
     */
    removeDistanceMeasurementPoint (payload) {
      return simViz.scenario.removeMeasurementPoint(payload)
    },

    /**
     * 删除一条测距线
     */
    removeDistanceMeasurementLine (payload) {
      return simViz.scenario.removeMeasurement(payload)
    },
    getL3StateMachineRawData () {
      return simViz.scenario.getL3StateMachineRawData()
    },
    getL3StateMachineData () {
      return simViz.scenario.getL3StateMachineData()
    },
    setL3StateMachineData (payload) {
      return simViz.scenario.setL3StateMachineData(payload)
    },

    /**
     * 获取场景中物体所在lane的最大偏移
     * @param type 物体类型
     * @param id 物体id
     * @returns Number 长度数值
     */
    getObjectMaxOffset (type, id) {
      return simViz.scenario.getObjectMaxOffset(type, id)
    },

    /**
     * 获取场景中某一辆车所在lane的最大长度
     * @param vehicleId 车辆Id
     * @returns Number 长度数值
     */
    getVehicleMaxShift (id) {
      return simViz.scenario.getVehicleMaxShift(id)
    },

    /**
     * 获取场景中所有车辆的ID
     * @returns Array 车辆ID数组
     */
    getVehicleIDs () {
      return simViz.scenario.getVehicleIDs()
    },

    /**
     * 获取场景中可以被vehicleId跟随的交通流车辆ID
     * @param vehicleId 车辆Id
     * @returns Array 车辆ID数组
     */
    getCanBeFollowedVehicleIDs (vehicleId) {
      return simViz.scenario.getCanBeFollowedVehicleIDs(vehicleId)
    },

    /**
     * 通过类型获取物体所有信息
     * @param {string} type 物体类型
     * @return {[]} 类型为type的场景元素列表
     */
    getObjectsByType (type) {
      return simViz.scenario.getObjectsByType(type)
    },

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
    paramScene (id, name, payload) {
      return simViz.scenario.paramScene(id, name, payload)
    },

    /**
     * 获取生成信息
     * @returns {object} 生成信息
     */
    getGenerateInfo () {
      return simViz.scenario.getGenerateInfo()
    },

    /**
     * 获取界面部分UI信息
     * @returns
     */
    getUIStateInfo () {
      return simViz.scenario.getUIStateInfo()
    },

    /**
     * 获取界面所有道路id及其下属section、laneId
     * @returns
     */
    getRoadobj () {
      return simViz.scenario.getRoadobj()
    },

    /**
     * 获取界面所有所有触发事件
     * @returns
     */
    getSceneevents () {
      return simViz.scenario.getSceneevents()
    },

    canOpenScenario (scenario) {
      return simViz.scenario.canOpenScenario(scenario)
    },

    updateHadmapInfo () {
      return simViz.scenario.updateHadmapInfo()
    },

    /**
     * 设置工况说明
     * @param {string} desc 工况说明
     */
    setSceneDesc (info) {
      return simViz.scenario.setSceneDesc(info)
    },

    getState () {
      return simViz.scenario.getState()
    },

    syncPlannerVisual (id) {
      return simViz.scenario.syncPlannerVisual(id)
    },
    // 获取地图所有的mesh
    getMapGroup () {
      return simViz.scenario.getMapGroup()
    },

    updateModel (data) {
      return simViz.scenario.updateModel(data)
    },
  },
  object: {
    /**
     * 添加场景元素
     * @param {object} payload 场景元素
     * @returns {Promise<undefined|null|any|string|{id: *, accs: *}>}
     */
    add (payload) {
      return simViz.scenario.addObject(payload)
    },

    /**
     * 移除场景元素
     * @param {object} payload 场景元素
     */
    del (payload) {
      return simViz.scenario.removeObject(payload)
    },

    /**
     * 修改场景元素
     * @param {object} payload 场景元素（位置相关属性）
     */
    update (payload) {
      return simViz.scenario.modifyObject(payload)
    },

    /**
     * 修改场景元素
     * @param {object} payload 场景元素（非位置相关属性）
     */
    modifyObjectParams (payload) {
      return simViz.scenario.modifyObjectParams(payload)
    },

    /**
     * 相机聚焦到某个场景元素
     * @param {object} payload 场景元素
     */
    select (payload) {
      return simViz.scenario.locateObject(payload)
    },

    /**
     * 复制场景元素
     * @param {object} payload 场景元素
     * @returns {Promise<undefined|null|any|string|{id: *, accs: *}>}
     */
    copy (payload) {
      return simViz.scenario.copyObject(payload)
    },

    /**
     * 设置元素初始角度
     * @param {object} payload 场景元素
     */
    setAngle (payload) {
      return simViz.scenario.setAngle(payload)
    },
  },
}

const player = {
  register (payload) {
    return simViz.player.register(payload)
  },
  addEventListener (type, listener) {
    return simViz.player.signals[conversion.player[type] || type].add(listener)
  },
  removeEventListener (type, listener) {
    return simViz.player.signals[conversion.player[type] || type].remove(listener)
  },
  dispatchListener (type, payload) {
    return simViz.player.signals[conversion.player[type] || type].dispatch(payload)
  },

  /**
   * 设置界面缩放
   * @param {number} factor 相对缩放比
   * @returns {undefined}
   */
  zoom (level) {
    return simViz.player.zoom(level)
  },

  /**
   * 切换视角
   * @param {string} id 视角
   */
  getCameras (view) {
    return simViz.player.setCamera(view)
  },

  /**
   * 切换视角
   * @param {string} id 视角
   */
  setCamera (view) {
    return simViz.player.setCamera(view)
  },

  /**
   * 读取场景
   * @param {string} scenarioId 场景名称
   * @returns {Promise<void>}
   */
  setScenario (id) {
    return simViz.player.setScenario(id)
  },

  /**
   * 相机聚焦到某个场景元素
   * @param {object} payload 场景元素
   */
  locateObject (payload) {
    simViz.player.locateObject(payload)
  },
  setObjectsVisible (type, visible) {
    simViz.player.setObjectsVisible(type, visible)
  },
  resetAllObjectVisible () {
    simViz.player.resetAllObjectVisible()
  },
  scenario: {
    /**
     * @desc 修改容器大小
     * @param {number} width
     * @param {number} height
     */
    resize (...args) {
      simViz.player.resize(...args)
    },
    render (message) {
      simViz.player.processMessage(message)
    },

    /**
     * 获取地图信息,返回地图的名称、参考点经纬度及高度
     * @returns {{mapName: string, lon: string, lat: string, alt: string}}
     */
    getMapInfo () {
      return simViz.player.getMapInfo()
    },

    /**
     * 更新本地地图文件信息
     */
    updateHadmapInfo () {
      return simViz.player.updateHadmapInfo()
    },
    resetScene () {
      simViz.player.resetScene()
    },
    // 播放可能存在的已缓存但未播放的帧
    playerLastFrame () {
      simViz.player.playerLastFrame()
    },
  },
}

/**
 * 通用功能。
 * @type {object}
 */
const common = {
  /**
   * 判断某个map是否存在
   * @param {*} scenario - 场景对象。
   * @returns {boolean} 如果地图存在则返回 true，否则返回 false。
   */
  hasMap (scenario) {
    return simViz.hasMap(scenario)
  },
}

export {
  editor,
  player,
  common,
}
