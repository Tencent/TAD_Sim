import _ from 'lodash'
import {
  changeCameraAdvancedInfo,
  changeLidarAdvancedInfo,
  changeObuAdvancedInfo,
  changeRadarAdvancedInfo,
  formatValue,
} from '@/store/modules/sensor/factory'

/**
 * 更新模块状态
 * @param {Object} state - Vuex状态对象
 * @param {Object} payload - 需要更新的状态数据
 */
export function updateState (state, payload) {
  Object.assign(state, payload)
}

/**
 * 更新传感器选中状态
 * @param {Object} state - Vuex状态对象
 * @param {Object} payload - 包含索引和选中状态的参数
 */
export function updateSensorChecked (state, payload) {
  state.sensorGroups[state.currentGroupIndex][payload.index].enabled = payload.enabled
}

/**
 * 更新传感器设备
 * @param {Object} state - Vuex状态对象
 * @param {Object} payload - 包含索引和设备信息的参数
 */
export function updateSensorDevice (state, payload) {
  state.sensorGroups[state.currentGroupIndex][payload.index].Device = payload.Device
}

/**
 * 更新传感器安装位置
 * @param {Object} state - Vuex状态对象
 * @param {Object} payload - 包含索引和安装位置信息的参数
 */
export function updateSensorInstallSlot (state, payload) {
  state.sensorGroups[state.currentGroupIndex][payload.index].InstallSlot = payload.InstallSlot
}

/**
 * 复制传感器
 * @param {Object} state - Vuex状态对象
 * @param {number} index - 传感器索引
 */
export function copySensor (state, index) {
  const currentGroup = state.sensorGroups[state.currentGroupIndex]
  const targetSensor = _.cloneDeep(currentGroup[index])
  let targetID = 1
  while (currentGroup.find(sensor => sensor.type === targetSensor.type && sensor.ID === targetID)) {
    targetID++
  }
  targetSensor.ID = targetID

  currentGroup.splice(index + 1, 0, targetSensor)
  state.currentSensorIndex = index + 1
}

/**
 * 删除传感器
 * @param {Object} state - Vuex状态对象
 * @param {number} index - 传感器索引
 */
export function deleteSensor (state, index) {
  const currentGroup = state.sensorGroups[state.currentGroupIndex]
  currentGroup.splice(index, 1)

  if (state.currentSensorIndex >= index) {
    state.currentSensorIndex -= 1
  }
}

/**
 * 更新传感器基础信息
 * @param {Object} state - Vuex状态对象
 * @param {Object} payload - 需要更新的基础信息数据
 */
export function updateBaseInfo (state, payload) {
  const { baseInfo } = state.sensorGroups[state.currentGroupIndex][state.currentSensorIndex]
  Object.keys(payload).forEach((key) => {
    baseInfo[key].value = payload[key]
  })
}

/**
 * 改变传感器基础信息
 * @param {Object} state - Vuex状态对象
 * @param {Object} payload - 包含参数名和参数值的参数
 */
export function changeBaseInfo (state, { paramName, value }) {
  const { baseInfo } = state.sensorGroups[state.currentGroupIndex][state.currentSensorIndex]

  const param = baseInfo[paramName]
  if (param.range) {
    param.value = formatValue(param, value)
  } else {
    param.value = value
  }
}

export function updateAdvancedInfo (state, payload) {
  const { advancedInfo } = state.sensorGroups[state.currentGroupIndex][state.currentSensorIndex]
  Object.keys(payload).forEach((key) => {
    advancedInfo[key].value = payload[key]
  })
}

// with some logic
export function changeAdvancedInfo (state, payload) {
  const { advancedInfo, type } = state.sensorGroups[state.currentGroupIndex][state.currentSensorIndex]
  switch (type) {
    case 'camera':
    case 'fisheye':
    case 'semantic':
    case 'depth':
    case 'truth':
    case 'gps':
    case 'imu':
    case 'ultrasonic':
      changeCameraAdvancedInfo(advancedInfo, payload)
      break
    case 'obu':
      changeObuAdvancedInfo(advancedInfo, payload)
      break
    case 'lidar':
      changeLidarAdvancedInfo(advancedInfo, payload)
      break
    case 'radar':
      changeRadarAdvancedInfo(advancedInfo, payload)
      break
    default:
  }
}

/**
 * @desc 应用配置
 * target: 'scenario' | 'global'
 */
export function useSensorGroup (state, { index, target }) {
  const sourceGroup = _.cloneDeep(state.sensorGroups[index])
  let targetIndex = 0
  if (target === 'global') {
    targetIndex = 5
  }
  state.sensorGroups.splice(targetIndex, 1, sourceGroup)
}

export function updateSensorGroup (state, { index, group }) {
  state.sensorGroups.splice(index, 1, group)
}

export function acceptDefaultGroups (state, groups) {
  state.sensorGroups.splice(1, groups.length, ...groups)
}

export function acceptGlobalGroups (state, group) {
  state.sensorGroups.splice(5, 1, group)
}
