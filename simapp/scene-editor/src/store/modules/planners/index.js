/**
 * planner模块，用于管理主车配置、传感器、动力学等数据
 */
import { cloneDeep, isEqual, pick, set } from 'lodash-es'
import {
  addSensor,
  addSensorGroup,
  delSensorGroup,
  deleteDynamic,
  deletePlanner,
  deleteSensor,
  getAllCatalogs,
  getDynamicList,
  getSensorGroups,
  getSensorList,
  modifySensorGroup,
  saveDynamic,
  updateSensor,
} from '@/api/planners'
import { changeSensorParam } from '@/store/modules/sensor/factory'
import { sensorTypeMap } from '@/store/modules/sensor/constants'
import { formatModel3d, genHashAndModelIdFromName, getModelDisplayName, setModelDisplayName } from '@/common/utils'
import { saveModel } from '@/api/models'
import { getPermission } from '@/utils/permission'
import { getModelProperty } from '@/common/utils.ts'

const sensorConfig = getPermission('action.planner-config.sensors.view.list')

/**
 * 生成传感器ID
 * @param {Array} group - 传感器组
 * @param {string} type - 传感器类型
 * @returns {number} - 生成的传感器ID
 */
function genSensorId (group, type) {
  let max = -1
  group.forEach((s) => {
    if (s.type === type) {
      max = Math.max(+max, +s.ID)
    }
  })
  return max + 1
}

/**
 * 生成传感器索引
 * @param {Array} sensors - 传感器列表
 * @returns {number} - 生成的传感器索引
 */
function genSensorIdx (sensors) {
  return sensors.reduce((max, s) => {
    return Math.max(max, s.idx)
  }, 0) + 1
}

export default {
  namespaced: true,
  state: {
    plannerList: [], // 主车配置列表
    currentPlannerIndex: -1, // 当前主车配置索引
    sensors: [], // 传感器列表
    dynamics: [], // 动力学列表
    sensorGroups: [], // 传感器组列表
    // 选的是主车元素里的 index
    selectedSensorIDs: [], // 选中的传感器ID列表
    // 主车元素除了传感器的 name | planner | controller | dynamics
    selected: null, // 选中的主车元素
    currentPlanner: null, // 当前主车配置
    isDirty: false, // 是否已修改
    isNew: false, // 是否为新建
    selectedDynamicsIDs: [], // 选中的动力学ID列表
    currentDynamicData: {}, // 当前动力学数据
    paramsWidth: 240, // 参数宽度
  },
  getters: {
    sensorGroups (state) {
      // 获取按类型分类的传感器组
      const groups = {
        Camera: [],
        Fisheye: [],
        Semantic: [],
        Depth: [],
        TraditionalLidar: [],
        Radar: [],
        Truth: [],
        IMU: [],
        GPS: [],
        Ultrasonic: [],
        OBU: [],
      }
      Object.keys(sensorTypeMap).forEach((key) => {
        const name = sensorTypeMap[key]
        if (!sensorConfig.includes(key)) {
          delete groups[name]
        }
      })
      state.sensors.forEach((sensor) => {
        if (sensor.type in groups) {
          groups[sensor.type].push(sensor)
        }
      })
      return groups
    },
    currentSensor (state) {
      // 获取当前选中的传感器
      const {
        selectedSensorIDs,
        currentPlanner,
      } = state
      if (selectedSensorIDs.length !== 1) {
        return null
      }
      return currentPlanner.sensor.group[selectedSensorIDs[0]]
    },
    currentDynamics (state) {
      // 获取当前选中的动力学
      const {
        selectedDynamicsIDs,
      } = state
      if (selectedDynamicsIDs.length !== 1) {
        return null
      }
      return selectedDynamicsIDs[0]
    },
    obuIds (state) {
      // 获取OBU传感器的索引列表
      return state.sensors.filter(s => s.type === 'OBU').map(s => s.idx)
    },
    obuBoundingSensorGroups (state, getters) {
      // 获取与OBU传感器关联的传感器组
      const {
        sensorGroups,
        plannerList,
      } = state
      const { obuIds } = getters
      return sensorGroups.filter((sg) => {
        return !plannerList
          .find(p => p.sensor.groupName === sg.groupName) && sg.group?.length === 1 && obuIds.includes(sg.group[0]?.idx)
      })
    },
  },
  mutations: {
    // 更新传感器的基础信息
    changeBaseInfo (state, payload) {
      const { paramName, value } = payload
      const { currentPlanner, selectedSensorIDs } = state
      const sensor = currentPlanner.sensor.group[selectedSensorIDs[0]]
      sensor[paramName] = value
      state.isDirty = true
    },
    // 更新传感器的高级信息
    changeAdvancedInfo (state, payload) {
      const { selectedSensorIDs, currentPlanner } = state
      const sensor = currentPlanner.sensor.group[selectedSensorIDs[0]]
      const { type, param } = sensor
      // 重新计算数据
      const newParam = changeSensorParam(type, param, payload)
      sensor.param = { ...param, ...newParam }
      state.isDirty = true
    },
    // 选择主车配置
    selectPlanner (state, index) {
      // 选择一个主车的时候，组装一个主车对象出来
      state.selectedSensorIDs = []
      state.selectedDynamicsIDs = []
      state.currentPlannerIndex = index
      const planner = cloneDeep(state.plannerList[index])
      planner.sensor.group.forEach((s) => {
        const sensor = state.sensors.find(_s => _s.idx === s.idx)
        Object.assign(s, cloneDeep(sensor))
      })

      const dynamic = state.dynamics.find(dynamic => dynamic.id === `${planner.catalogParams[0].properties.dynamic}`)
      if (dynamic) {
        state.currentDynamicData = cloneDeep(dynamic.dynamicData)
      } else {
        state.currentDynamicData = {}
      }
      state.currentPlanner = planner
      state.selected = 'planner'
    },
    select (state, key) {
      state.selectedSensorIDs = []
      state.selectedDynamicsIDs = []
      state.selected = key
    },
    // 选择传感器
    selectSensor (state, payload) {
      state.selected = null
      state.selectedSensorIDs = []
      state.selectedDynamicsIDs = []
      state.selectedSensorIDs.push(...payload)
    },
    // 选择动力学
    selectDynamics (state, payload) {
      state.selected = null
      state.selectedSensorIDs = []
      state.selectedDynamicsIDs = []
      state.selectedDynamicsIDs.push(...payload)
    },
    // 更新主车配置
    updatePlanner (state, payload) {
      const {
        currentPlanner: planner,
      } = state
      Object.entries(payload).forEach(([key, value]) => {
        set(planner, key, value)
      })
      state.isDirty = true
    },
    // 更新主车配置参数
    updatePlannerParam (state, payload) {
      const {
        currentPlanner: planner,
      } = state
      const { index = 0, ...params } = payload
      Object.entries(params).forEach(([key, value]) => {
        set(planner.catalogParams[index], key, value)
      })
      state.isDirty = true
    },
    // 删除传感器
    deletePlannerSensor (state, ids) {
      const { currentPlanner: planner } = state
      planner.sensor.group = planner.sensor.group.filter((s, i) => !ids.includes(i))
      state.selectedSensorIDs = []
      state.isDirty = true
    },
    // 设置是否已修改状态
    setIsDirty (state, bool) {
      state.isDirty = bool
    },
    // 设置是否为新建状态
    setIsNew (state, bool) {
      state.isNew = bool
    },
    // 设置当前动力学数据
    setCurrentDynamicData (state, data) {
      state.currentDynamicData = data
      state.isDirty = true
    },
    // 设置参数宽度
    setParamsWidth (state, width) {
      state.paramsWidth = width
    },
  },
  actions: {
    // 主车和其他catalogs通过一个接口全部获取
    async getAllCatalogs ({ state, commit, dispatch }) {
      const catalogs = await getAllCatalogs()
      const {
        driverCatalog,
        vehicleCatalog,
        pedestrianCatalog,
        miscobjectCatalog,
      } = catalogs
      const canCustom = getPermission('action.importCustomModels.view.enable')
      const isPreset = planner => getModelProperty(planner, 'preset')
      const plannerList = driverCatalog.filter(planner => canCustom ? true : isPreset(planner))
      await dispatch('getAllSensorGroups')
      // 组装一下planner
      plannerList.forEach((planner) => {
        const g = state.sensorGroups.find(g => +g.groupName === +getModelProperty(planner, 'sensorGroup'))
        if (g?.group?.slice) {
          planner.sensor.group = g.group.slice()
        }
      })
      state.plannerList = plannerList
      commit('catalogs/updateList', {
        vehicleList: vehicleCatalog,
        pedestrianList: pedestrianCatalog,
        obstacleList: miscobjectCatalog,
      }, { root: true })
      return {
        plannerList,
        vehicleList: vehicleCatalog,
        pedestrianList: pedestrianCatalog,
        obstacleList: miscobjectCatalog,
      }
    },
    // 获取传感器列表
    async getAllSensors ({ state, dispatch }) {
      state.sensors = await getSensorList()
      await dispatch('getAllSensorGroups')
      await dispatch('fixObuBoundingSensorGroups')
    },
    // 获取动力学列表
    async getAllDynamics ({ state }) {
      state.dynamics = await getDynamicList()
    },
    // 获取传感器组列表
    async getAllSensorGroups ({ state }) {
      state.sensorGroups = await getSensorGroups()
    },
    async fixObuBoundingSensorGroups ({ state, dispatch }) {
      const obus = state.sensors.filter(s => s.type === 'OBU')
      const { sensorGroups, plannerList } = state
      // 先排除主车已经绑定过的group
      const notBoundingWithPlannerSensorGroups = sensorGroups
        .filter(sg => !plannerList.find(p => p.sensor.groupName === sg.groupName))
      for (let i = 0; i < obus.length; i++) {
        // 用forEach会并发接口，可能会导致后台处理文件出错
        const obu = obus[i]
        const bound = notBoundingWithPlannerSensorGroups
          .find(sg => sg.group?.length === 1 && sg.group[0].idx === obu.idx)
        if (!bound) {
          // 没有绑定的，则生成一个
          await dispatch('createBoundingObuSensorGroup', obu.idx)
        }
      }
    },
    async createBoundingObuSensorGroup ({ state }, idx) {
      const { sensorGroups } = state
      const maxGroupIndex = sensorGroups.reduce((max, current) => Math.max(max, +current.groupName), 0)
      const sensorGroup = {
        group: [{
          idx,
          Device: '.0',
          ID: '0',
          InstallSlot: 'C0',
          LocationX: '0',
          LocationY: '0',
          LocationZ: '0',
          RotationX: '0',
          RotationY: '0',
          RotationZ: '0',
        }],
        groupName: `${maxGroupIndex + 1}`,
      }
      await addSensorGroup(sensorGroup)
      state.sensorGroups.push(sensorGroup)
    },
    async copyPlanner ({ state, commit }, index) {
      const {
        dynamics,
        plannerList,
        currentDynamicData,
      } = state
      const target = plannerList[index]
      if (target) {
        const newPlanner = cloneDeep(target)
        const newName = `${getModelDisplayName(newPlanner)}副本`
        setModelDisplayName(newPlanner, newName)

        // 处理动力学文件
        if (target.catalogParams[0].properties.dynamic) {
          const newDynamicLen = dynamics.length + 1
          let newDynamicId = '1'
          for (let i = 1; i <= newDynamicLen; i++) {
            newDynamicId = `${i}`
            const index = dynamics.findIndex(dynamic => dynamic.id === newDynamicId)
            if (index === -1) {
              break
            }
          }
          newPlanner.catalogParams[0].properties.dynamic = newDynamicId
          dynamics.push({
            id: newDynamicId,
            dynamicData: cloneDeep(currentDynamicData),
          })
        }

        // 处理传感器引用
        const plannerLength = plannerList.length + 2
        let newSensorGroupName = 1
        for (let i = 1; i < plannerLength; i++) {
          newSensorGroupName = i
          const index = plannerList.findIndex(planner => planner.sensor?.groupName === `${newSensorGroupName}`)
          if (index === -1) {
            break
          }
        }
        newPlanner.sensor.groupName = `${newSensorGroupName}`

        const {
          hash: hashValue,
        } = genHashAndModelIdFromName(newName)
        newPlanner.variable = `user_ego_${hashValue}`
        newPlanner.catalogParams[0].properties.preset = false
        plannerList.push(newPlanner)
        commit('selectPlanner', plannerList.length - 1)
        state.isDirty = true
        state.isNew = true
      }
    },
    async removePlanner ({ state, commit }, index) {
      const {
        currentPlannerIndex,
        plannerList,
        dynamics,
        isNew,
      } = state
      if (index === currentPlannerIndex) {
        commit('setIsDirty', false)
        commit('setIsNew', false)
      }
      if (index <= currentPlannerIndex) {
        commit('selectPlanner', currentPlannerIndex - 1)
      }
      if (plannerList[index].catalogParams[0].properties.dynamic) {
        await deleteDynamic(plannerList[index].catalogParams[0].properties.dynamic)
        const dynamicIndex = dynamics.findIndex(dynamic => dynamic.id === `${plannerList[index].catalogParams[0].properties.dynamic}`)
        if (dynamicIndex !== -1) {
          dynamics.splice(dynamicIndex, 1)
        }
      }
      if (!isNew && index !== currentPlannerIndex) {
        await deletePlanner(plannerList[index].variable)
      }
      plannerList.splice(index, 1)
    },
    async savePlanner ({ state }) {
      const {
        dynamics,
        sensors,
        plannerList,
        currentPlanner,
        currentPlannerIndex,
        currentPlanner: {
          catalogParams: [{
            properties: {
              dynamic: dynamicId,
            },
          }],
          sensor: {
            groupName,
            group,
          },
        },
        currentDynamicData,
      } = state
      for (const s of group) {
        // 因为name编辑在别的地方存了，所以这里只判断param的改变
        const os = sensors.find(_s => _s.idx === s.idx)
        if (!isEqual(os.param, s.param)) {
          const data = pick(s, ['type', 'name', 'idx', 'param'])
          await updateSensor({ sensor: data })
        }
      }
      await saveDynamic({
        id: dynamicId,
        dynamicData: currentDynamicData,
      })
      const sensorGroup = {
        groupName,
        group: group.map((s) => {
          return pick(s, [
            'idx',
            'ID',
            'Device',
            'InstallSlot',
            'LocationX',
            'LocationY',
            'LocationZ',
            'RotationX',
            'RotationY',
            'RotationZ',
          ])
        }),
      }
      // 格式化一下model3d，这里不会上传新模型，所以不使用模型variable计算模型路径
      const data = formatModel3d(currentPlanner, false)
      if (state.isNew) {
        await addSensorGroup(sensorGroup)
        await saveModel('-1', { ...data, sensor: undefined })
      } else {
        await modifySensorGroup(sensorGroup)
        await saveModel('-1', { ...data, sensor: undefined })
      }
      state.isDirty = false
      state.isNew = false
      const index = dynamics.findIndex(dynamic => dynamic.id === dynamicId)
      if (index !== -1) {
        dynamics.splice(index, 1, {
          id: dynamicId,
          dynamicData: cloneDeep(currentDynamicData),
        })
      }
      const planner = {
        ...currentPlanner,
        sensor: sensorGroup,
      }
      // 反写传感器信息
      group.forEach((s) => {
        const index = sensors.findIndex(_s => _s.idx === s.idx)
        sensors.splice(index, 1, pick(s, ['type', 'name', 'idx', 'param']))
      })
      // 反写主车信息
      Object.assign(plannerList[currentPlannerIndex], planner)
    },
    async validatePlanner ({ state }) {
      // 暂时只有vehicle_alias需要校验
      const {
        currentPlanner: planner,
        plannerList,
      } = state
      if (!getModelDisplayName(planner).trim()) {
        return Promise.reject(new Error('请填写主车配置名称'))
      }
      if (getModelDisplayName(planner).trim().length > 20) {
        return Promise.reject(new Error('主车配置名称不能超过20个字符'))
      }
      const isRepeat = plannerList.some((p) => {
        if (p.variable !== planner.variable) {
          return getModelDisplayName(p) === getModelDisplayName(planner)
        }
        return false
      })
      if (isRepeat) {
        return Promise.reject(new Error(`已经存在名称为 ${getModelDisplayName(planner)} 的主车配置`))
      }
      return true
    },
    async validateSensorName ({ state }, { idx, name }) {
      if (!name) {
        return Promise.reject(new Error('请填写传感器配置名称'))
      }
      if (name.length > 20) {
        return Promise.reject(new Error('传感器配置名称不能超过20个字符'))
      }
      const isRepeated = state.sensors.some((s) => {
        if (s.idx === idx) {
          return false
        }
        return s.name === name
      })
      if (isRepeated) {
        return Promise.reject(new Error('这个配置名称已经存在了'))
      }
      return true
    },
    addSensorToPlanner ({ state }, idx) {
      const {
        currentPlanner,
        sensors,
      } = state
      const sensor = sensors.find(s => s.idx === idx)
      currentPlanner.sensor.group.push({
        ...cloneDeep(sensor),
        Device: '.0',
        ID: `${genSensorId(currentPlanner.sensor.group, sensor.type)}`,
        InstallSlot: 'C0',
        LocationX: 0,
        LocationY: 0,
        LocationZ: 0,
        RotationX: 0,
        RotationY: 0,
        RotationZ: 0,
      })
      state.isDirty = true
    },
    copySensor ({ state }, idx) {
      const source = state.sensors.find(s => s.idx === idx)
      const newIdx = genSensorIdx(state.sensors)
      let newName = `${source.name}副本`
      while (state.sensors.some(s => s.name === newName)) {
        newName += '副本'
      }
      state.sensors.push({
        ...cloneDeep(source),
        idx: newIdx,
        name: newName,
      })
      return newIdx
    },
    async createSensor ({ state, dispatch }, { idx, name }) {
      const target = state.sensors.find(s => s.idx === idx)
      const newName = name || target.name
      await addSensor({ sensor: { ...target, name: newName } })
      // 如果是obu，则新建一个sensorGroup
      if (target.type === 'OBU') {
        await dispatch('createBoundingObuSensorGroup', idx)
      }
      target.name = newName
    },
    async modifySensor ({ state }, { idx, name }) {
      const target = state.sensors.find(s => s.idx === idx)
      const newName = name || target.name
      await updateSensor({ sensor: { ...target, name: newName } })
      target.name = newName
      state.currentPlanner.sensor.group.forEach((s) => {
        if (s.idx === idx) {
          s.name = newName
        }
      })
    },
    async removeSensor ({ state }, { idx, isNewSensor = false }) {
      const {
        sensors,
        plannerList,
        currentPlanner,
        sensorGroups,
      } = state
      // 每个类型的最后一个传感器不允许删除
      const types = {}
      let sensor
      let index
      sensors.some((s) => {
        if (s.idx === idx) {
          sensor = s
          if (s.type in types) {
            index = types[s.type] + 1
          }
          return true
        }
        if (s.type in types) {
          types[s.type] += 1
        } else {
          types[s.type] = 0
        }
        return false
      })
      if (index === 0) {
        return Promise.reject(new Error('无法删除预设传感器'))
      }

      for (let i = 0; i < plannerList.length; i++) {
        const p = plannerList[i]
        // 将引用了这个传感器的主车删除传感器然后保存
        if (p.sensor.group.find(s => s.idx === idx)) {
          p.sensor.group = p.sensor.group.filter(s => s.idx !== idx)
          // 当前车辆不直接保存，设置为dirty状态
          if (p.variable === currentPlanner.variable) {
            state.isDirty = true
          } else {
            await modifySensorGroup(p.sensor)
          }
        }
      }
      // 当前车辆引用了这个传感器时需要删除一下
      currentPlanner.sensor.group = currentPlanner.sensor.group.filter(s => s.idx !== idx)

      // 如果是obu，且绑定了它的sensorGroup没有被任何主车引用，则删除这个group
      if (sensor.type === 'OBU') {
        const groups = sensorGroups.filter(
          sg => sg.group?.length === 1 && sg.group[0].idx === idx,
        ).filter(
          sg => !plannerList.find(p => p.sensor.groupName === sg.groupName),
        )
        if (groups.length) {
          // 有可能因为各种原因，查出来多个，这里全部删除了。
          for (let i = 0; i < groups.length; i++) {
            await delSensorGroup({
              groupName: groups[i].groupName,
            })
            state.sensorGroups = sensorGroups.filter(sg => sg.groupName !== groups[i].groupName)
          }
        }
      }

      if (!isNewSensor) {
        await deleteSensor({ idx })
      }
      state.sensors = sensors.filter(s => s.idx !== idx)
    },
    async updateAllSensors ({ state }) {
      // 把所有 sensor 读取再保存就会自动添加所有传感器新加的字段的默认值
      for (let i = 0; i < state.sensors.length; i++) {
        await updateSensor({ sensor: { ...state.sensors[i] } })
      }
    },
    async saveSensor ({ state }, payload) {
      const target = state.sensors.find(s => s.idx === payload.idx)
      target.name = payload.name
      target.param = cloneDeep(payload.param)
      await updateSensor({ sensor: target })
      state.currentPlanner.sensor.group.forEach((s) => {
        if (s.idx === payload.idx) {
          s.name = payload.name
          s.param = cloneDeep(payload.param)
        }
      })
    },
  },
}
