import { cloneDeep, isObject, isUndefined } from 'lodash-es'
import { ElMessage } from 'element-plus'
import {
  GPSAdvancedInfo,
  IMUAdvancedInfo,
  LidarFrequencyOptions,
  OBUAdvancedInfo,
  baseInfo,
  cameraAdvancedInfo,
  deviceOptions,
  fisheyeAdvancedInfo,
  lidarAdvancedInfo,
  radarAdvancedInfo,
  truthAdvancedInfo,
  ultrasonicAdvancedInfo,
} from './constants'
import i18n from '@/locales'

const { isArray } = Array

/**
 * 创建基础属性
 * @return {{
 * RotationY: {unit: string, range: string, step: number, type: string, value: number},
 * RotationX: {unit: string, range: string, step: number, type: string, value: number},
 * LocationY: {unit: string, range: string, step: number, type: string, value: number},
 * LocationZ: {unit: string, range: string, step: number, type: string, value: number},
 * LocationX: {unit: string, range: string, step: number, type: string, value: number},
 * RotationZ: {unit: string, range: string, step: number, type: string, value: number}
 * }}
 */
export const createBaseInfo = function () {
  return cloneDeep(baseInfo)
}

/**
 * 创建相机高级属性
 */
const createCameraAdvancedInfo = function () {
  return cloneDeep(cameraAdvancedInfo)
}

/**
 * 创建激光雷达高级属性
 */
const createLidarAdvancedInfo = function () {
  return cloneDeep(lidarAdvancedInfo)
}

/**
 * 创建毫米波雷达属性
 */
const createRadarAdvancedInfo = function () {
  return cloneDeep(radarAdvancedInfo)
}

/**
 * 创建真值传感器属性
 */
const createTruthAdvancedInfo = function () {
  return cloneDeep(truthAdvancedInfo)
}

/**
 * 创建IMU属性
 */
const createIMUAdvancedInfo = function () {
  return cloneDeep(IMUAdvancedInfo)
}

/**
 * 创建GPS属性
 */
const createGPSAdvancedInfo = function () {
  return cloneDeep(GPSAdvancedInfo)
}

/**
 * 创建超声波雷达属性
 */
const createUltrasonicAdvancedInfo = function () {
  return cloneDeep(ultrasonicAdvancedInfo)
}

/**
 * 创建鱼眼相机属性
 */
const createFisheyeAdvancedInfo = function () {
  return cloneDeep(fisheyeAdvancedInfo)
}

/**
 * 创建OBU属性
 */
const createOBUAdvancedInfo = function () {
  return cloneDeep(OBUAdvancedInfo)
}

/**
 * 创建摄像头实体
 */
const createCameraInfo = function () {
  return {
    type: 'camera',
    name: 'Camera',
    enabled: false,
    baseInfo: createBaseInfo(),
    advancedInfo: createCameraAdvancedInfo(),
    ID: 1,
    Device: deviceOptions[0].value,
    InstallSlot: 'C0',
  }
}

/**
 * 创建激光雷达实体
 */
const createLidarInfo = function () {
  return {
    type: 'lidar',
    name: 'Lidar',
    enabled: false,
    baseInfo: createBaseInfo(),
    advancedInfo: createLidarAdvancedInfo(),
    ID: 1,
    Device: deviceOptions[0].value,
    InstallSlot: 'C0',
  }
}

/**
 * 创建超声波雷达实体
 */
const createRadarInfo = function () {
  return {
    type: 'radar',
    name: 'MmWave radar',
    enabled: false,
    baseInfo: createBaseInfo(),
    advancedInfo: createRadarAdvancedInfo(),
    ID: 1,
    Device: deviceOptions[0].value,
    InstallSlot: 'C0',
  }
}

/**
 * 创建真值传感器实体
 */
const createTruthInfo = function () {
  return {
    type: 'truth',
    name: 'Truth Sensor',
    enabled: false,
    baseInfo: createBaseInfo(),
    advancedInfo: createTruthAdvancedInfo(),
    ID: 1,
    Device: deviceOptions[0].value,
    InstallSlot: 'C0',
  }
}

/**
 * 创建IMU实体
 */
const createIMUInfo = function () {
  const baseInfo = createBaseInfo()

  baseInfo.LocationX.value = -154.1
  baseInfo.LocationY.value = 0
  baseInfo.LocationZ.value = 58
  baseInfo.RotationX.value = 1.099277
  baseInfo.RotationY.value = 2.130317
  baseInfo.RotationZ.value = -90.6149035

  return {
    type: 'imu',
    name: 'IMU',
    enabled: false,
    baseInfo,
    advancedInfo: createIMUAdvancedInfo(),
    ID: 1,
    InstallSlot: 'C0',
  }
}

/**
 * 创建GPS实体
 */
const createGPSInfo = function () {
  const baseInfo = createBaseInfo()

  baseInfo.LocationX.value = 0
  baseInfo.LocationY.value = 15.8
  baseInfo.LocationZ.value = 112.6
  baseInfo.RotationX.value = 0
  baseInfo.RotationY.value = 0
  baseInfo.RotationZ.value = 0

  return {
    type: 'gps',
    name: 'GPS',
    enabled: false,
    baseInfo,
    advancedInfo: createGPSAdvancedInfo(),
    ID: 1,
    InstallSlot: 'C0',
  }
}

/**
 * 创建成声波实体
 */
const createUltrasonicInfo = function () {
  const baseInfo = createBaseInfo()

  baseInfo.LocationX.value = -37
  baseInfo.LocationY.value = 25
  baseInfo.LocationZ.value = 171
  baseInfo.RotationX.value = 0
  baseInfo.RotationY.value = 0
  baseInfo.RotationZ.value = 0

  return {
    type: 'ultrasonic',
    name: 'Ultrasonic Radar',
    enabled: false,
    baseInfo,
    advancedInfo: createUltrasonicAdvancedInfo(),
    ID: 1,
    Device: deviceOptions[0].value,
    InstallSlot: 'C0',
  }
}

/**
 * 创建鱼眼相机实体
 */
const createFisheyeInfo = function () {
  const baseInfo = createBaseInfo()

  baseInfo.LocationX.value = -37
  baseInfo.LocationY.value = 25
  baseInfo.LocationZ.value = 171
  baseInfo.RotationX.value = 0
  baseInfo.RotationY.value = 0
  baseInfo.RotationZ.value = 0

  return {
    type: 'fisheye',
    name: 'Fisheye Camera',
    enabled: false,
    baseInfo,
    advancedInfo: createFisheyeAdvancedInfo(),
    ID: 1,
    Device: deviceOptions[0].value,
    InstallSlot: 'C0',
  }
}

/**
 * 创建语义相机实体
 */
const createSemanticInfo = function () {
  const info = createCameraInfo()
  info.type = 'semantic'
  info.name = 'Semantic Camera'
  return info
}

/**
 * 创建深度相机实体
 */
const createDepthInfo = function () {
  const info = createCameraInfo()
  info.type = 'depth'
  info.name = 'Depth Camera'
  return info
}

/**
 * 创建OBU实体
 */
const createOBUInfo = function () {
  return {
    type: 'obu',
    name: 'OBU',
    enabled: false,
    baseInfo: createBaseInfo(),
    advancedInfo: createOBUAdvancedInfo(),
    ID: 1,
    Device: deviceOptions[0].value,
    InstallSlot: 'C0',
  }
}

/**
 * 创建传感器实体
 */
export function createSensor (type) {
  switch (type) {
    case 'camera':
      return createCameraInfo()
    case 'lidar':
      return createLidarInfo()
    case 'radar':
      return createRadarInfo()
    case 'truth':
      return createTruthInfo()
    case 'imu':
      return createIMUInfo()
    case 'gps':
      return createGPSInfo()
    case 'ultrasonic':
      return createUltrasonicInfo()
    case 'fisheye':
      return createFisheyeInfo()
    case 'semantic':
      return createSemanticInfo()
    case 'depth':
      return createDepthInfo()
    case 'obu':
      return createOBUInfo()
    default:
      console.error('unsupported type')
  }
}

/**
 * 创建默认传感器组
 */
export function createDefaultSensorGroup () {
  return [
    createCameraInfo(),
    createFisheyeInfo(),
    createSemanticInfo(),
    createDepthInfo(),
    createLidarInfo(),
    createRadarInfo(),
    createTruthInfo(),
    createIMUInfo(),
    createGPSInfo(),
    createUltrasonicInfo(),
    createOBUInfo(),
  ]
}

/**
 * 创建默认传感器组
 */
export function createDefaultSensorGroups () {
  return [
    createDefaultSensorGroup(),
    createDefaultSensorGroup(),
    createDefaultSensorGroup(),
    createDefaultSensorGroup(),
  ]
}

/**
 * 格式化各种高级信息
 */
function formatCameraAdvanceInfo (camera, index, type = 'camera') {
  let obj
  switch (type) {
    case 'camera':
      obj = createCameraInfo()
      break
    case 'fisheye':
      obj = createFisheyeInfo()
      break
    case 'semantic':
      obj = createSemanticInfo()
      break
    case 'depth':
      obj = createDepthInfo()
      break
    default:
      throw new Error('Unsupported camera sub type.')
  }
  obj.enabled = camera.Enabled
  if (isUndefined(camera.ID)) {
    obj.ID = index + 1
  } else {
    obj.ID = +camera.ID
  }
  if (isUndefined(camera.Device)) {
    obj.Device = deviceOptions[0].value
  } else {
    obj.Device = camera.Device
  }
  obj.InstallSlot = camera.InstallSlot || 'C0'
  formatInfo(obj.baseInfo, camera)
  Object.keys(obj.advancedInfo).forEach((key) => {
    switch (key) {
      case 'Distortion_Parameters':
      case 'Intrinsic_Matrix':
        if (obj.advancedInfo[key].range) {
          obj.advancedInfo[key].value = formatValue(obj.advancedInfo[key], camera[key].split(','))
        } else {
          obj.advancedInfo[key].value = camera[key].split(',')
        }
        break
      default:
        if (obj.advancedInfo[key].range) {
          obj.advancedInfo[key].value = formatValue(obj.advancedInfo[key], camera[key])
        } else {
          try {
            obj.advancedInfo[key].value = JSON.parse(camera[key])
          } catch (e) {
            obj.advancedInfo[key].value = camera[key]
          }
        }
    }
  })
  return obj
}

/**
 * 格式化传感器实体
 */
function formatInfo (info, sensor) {
  Object.keys(info).forEach((key) => {
    if (isUndefined(sensor[key])) sensor[key] = info[key].value
    if (info[key].range) {
      info[key].value = formatValue(info[key], sensor[key])
    } else {
      try {
        info[key].value = JSON.parse(sensor[key])
      } catch (e) {
        info[key].value = sensor[key]
      }
    }
  })
}

/**
 * 从后台数据创建传感器组
 */
export function createSensorGroupFromValue (value) {
  if (!isObject(value)) {
    throw new TypeError('invalid sensor value')
  }

  let {
    cameras = [],
    fisheyes = [],
    semantics = [],
    depths = [],
    lidars = [],
    radars = [],
    truths = [],
    imus = [],
    gpses = [],
    ultrasonics = [],
    obus = [],
  } = value

  if (cameras.length) {
    cameras = cameras.map((camera, index) => formatCameraAdvanceInfo(camera, index))
  } else {
    cameras = [createCameraInfo()]
  }
  if (fisheyes.length) {
    fisheyes = fisheyes.map((fisheye, index) => formatCameraAdvanceInfo(fisheye, index, 'fisheye'))
  } else {
    fisheyes = [createFisheyeInfo()]
  }
  if (semantics.length) {
    semantics = semantics.map((semantic, index) => formatCameraAdvanceInfo(semantic, index, 'semantic'))
  } else {
    semantics = [createSemanticInfo()]
  }
  if (depths.length) {
    depths = depths.map((depth, index) => formatCameraAdvanceInfo(depth, index, 'depth'))
  } else {
    depths = [createDepthInfo()]
  }
  if (lidars.length) {
    lidars = lidars.map((lidar, index) => {
      const obj = createLidarInfo()
      obj.enabled = lidar.Enabled
      if (isUndefined(lidar.ID)) {
        obj.ID = index + 1
      } else {
        obj.ID = +lidar.ID
      }
      if (isUndefined(lidar.Device)) {
        obj.Device = deviceOptions[0].value
      } else {
        obj.Device = lidar.Device
      }
      obj.InstallSlot = lidar.InstallSlot || 'C0'
      formatInfo(obj.baseInfo, lidar)
      formatInfo(obj.advancedInfo, lidar)
      return obj
    })
  } else {
    lidars = [createLidarInfo()]
  }
  if (radars.length) {
    radars = radars.map((radar, index) => {
      const obj = createRadarInfo()
      obj.enabled = radar.Enabled
      if (isUndefined(radar.ID)) {
        obj.ID = index + 1
      } else {
        obj.ID = +radar.ID
      }
      if (isUndefined(radar.Device)) {
        obj.Device = deviceOptions[0].value
      } else {
        obj.Device = radar.Device
      }
      obj.InstallSlot = radar.InstallSlot || 'C0'
      formatInfo(obj.baseInfo, radar)
      formatInfo(obj.advancedInfo, radar)
      return obj
    })
  } else {
    radars = [createRadarInfo()]
  }
  if (truths.length) {
    truths = truths.map((truth, index) => {
      const obj = createTruthInfo()
      obj.enabled = truth.Enabled
      if (isUndefined(truth.ID)) {
        obj.ID = index + 1
      } else {
        obj.ID = +truth.ID
      }
      if (isUndefined(truth.Device)) {
        obj.Device = deviceOptions[0].value
      } else {
        obj.Device = truth.Device
      }
      obj.InstallSlot = truth.InstallSlot || 'C0'
      formatInfo(obj.baseInfo, truth)
      formatInfo(obj.advancedInfo, { ...truth })
      return obj
    })
  } else {
    truths = [createTruthInfo()]
  }
  if (imus.length) {
    imus = imus.map((imu) => {
      const obj = createIMUInfo()
      obj.enabled = imu.Enabled
      obj.ID = 1
      obj.InstallSlot = imu.InstallSlot || 'C0'
      formatInfo(obj.baseInfo, imu)
      obj.advancedInfo.Quaternion.value = imu.Quaternion.split(',')
      return obj
    })
  } else {
    imus = [createIMUInfo()]
  }
  if (gpses.length) {
    gpses = gpses.map((gps) => {
      const obj = createGPSInfo()
      obj.enabled = gps.Enabled
      obj.ID = 1
      obj.InstallSlot = gps.InstallSlot || 'C0'
      formatInfo(obj.baseInfo, gps)
      obj.advancedInfo.Quaternion.value = gps.Quaternion.split(',')
      return obj
    })
  } else {
    gpses = [createGPSInfo()]
  }
  if (ultrasonics.length) {
    ultrasonics = ultrasonics.map((ultrasonic, index) => {
      const obj = createUltrasonicInfo()
      obj.enabled = ultrasonic.Enabled
      if (isUndefined(ultrasonic.ID)) {
        obj.ID = index + 1
      } else {
        obj.ID = +ultrasonic.ID
      }
      if (isUndefined(ultrasonic.Device)) {
        obj.Device = deviceOptions[0].value
      } else {
        obj.Device = ultrasonic.Device
      }
      obj.InstallSlot = ultrasonic.InstallSlot || 'C0'
      formatInfo(obj.baseInfo, ultrasonic)
      formatInfo(obj.advancedInfo, ultrasonic)
      return obj
    })
  } else {
    ultrasonics = [createUltrasonicInfo()]
  }

  if (obus.length) {
    obus = obus.map((obu, index) => {
      const obj = createOBUInfo()
      obj.enabled = obu.Enabled
      if (isUndefined(obu.ID)) {
        obj.ID = index + 1
      } else {
        obj.ID = +obu.ID
      }
      if (isUndefined(obu.Device)) {
        obj.Device = deviceOptions[0].value
      } else {
        obj.Device = obu.Device
      }
      obj.InstallSlot = obu.InstallSlot || 'C0'
      formatInfo(obj.baseInfo, obu)
      formatInfo(obj.advancedInfo, obu)
      return obj
    })
  } else {
    obus = [createOBUInfo()]
  }
  return cameras
    .concat(fisheyes)
    .concat(semantics)
    .concat(depths)
    .concat(lidars)
    .concat(radars)
    .concat(truths)
    .concat(imus)
    .concat(gpses)
    .concat(ultrasonics)
    .concat(obus)
}

/**
 * 从后台数据创建传感器组
 */
export function createDefaultSensorGroupFromValue (value) {
  const arr = value.map((sensor) => {
    const obj = { ...sensor }
    if (isUndefined(obj.Device)) {
      obj.Device = deviceOptions[0].value
    }
    obj.InstallSlot = obj.InstallSlot || 'C0'
    return obj
  })
  const imuLength = arr.filter(sensor => sensor.type === 'imu').length
  const gpsLength = arr.filter(sensor => sensor.type === 'gps').length
  const obuLength = arr.filter(sensor => sensor.type === 'obu').length
  if (!imuLength) {
    arr.push(createIMUInfo())
  }
  if (!gpsLength) {
    arr.push(createGPSInfo())
  }
  if (!obuLength) {
    arr.push(createOBUInfo())
  }
  return arr
}

// 匹配 (1,2) 或 [1,2]
const rangeReg = /(\(|\[)([^,)\]]*),\s?(.*)(\)|\])/

/**
 * @desc 根据参数计算 value
 * @param {'('|'['} containMin
 * @param {number} min
 * @param {number} max
 * @param {')'|']'} containMax
 * @param {number}[step]
 * @param {number|string}value
 * @return {number}
 */
function constraintValue (containMin, min, max, containMax, step, value) {
  let val = Number.parseFloat(value) || 0
  if (containMin === '[' && val < min) {
    val = min
  } else if (containMin === '(' && val <= min) {
    val = min + step
  }
  if (containMax === ']' && val > max) {
    val = max
  } else if (containMax === ')' && val >= max) {
    val = max - step
  }
  return val
}

/**
 * @desc 计算param的value，如果value是数组则计算每一个值
 * @param range
 * @param step
 * @param value
 * @return {number}
 */
export function formatValue ({ range, step = 0 }, value) {
  // 解析范围表达式
  const [, containMin, _min, _max, containMax] = rangeReg.exec(range)
  const min = +_min
  const max = +_max
  let val
  if (isArray(value)) {
    val = value.map(v => v === '' ? '' : constraintValue(containMin, min, max, containMax, step, v))
  } else {
    val = constraintValue(containMin, min, max, containMax, step, value)
  }
  return val
}

/**
 * 对比范围数据
 * @param info
 * @param {string[]} paramNames
 * @param {string} inputParamName
 */
function compareRangeValues (info, paramNames, inputParamName) {
  const [minParamName, maxParamName] = paramNames
  const max = info[maxParamName]
  const min = info[minParamName]
  if (inputParamName === minParamName && min.value >= max.value) {
    ElMessage.info(i18n.t('tips.minMastLtMax', {
      min: i18n.t(min.name),
      max: i18n.t(max.name),
    }))
    min.value = max.value - (min.step || 1)
  } else if (inputParamName === maxParamName && max.value <= min.value) {
    ElMessage.info(i18n.t('tips.maxMastGtMin', {
      min: i18n.t(min.name),
      max: i18n.t(max.name),
    }))
    max.value = min.value + (max.step || 1)
  }
}

/**
 * 以下三个fn处理各自传感器数据联动逻辑
 * @param advancedInfo
 * @param paramName
 * @param value
 */
export function changeCameraAdvancedInfo (advancedInfo, { paramName, value }) {
  const targetParam = advancedInfo[paramName]
  if (targetParam.range) {
    const val = formatValue(targetParam, value)
    if (isArray(val) && isArray(value)) {
      if (val.toString() !== value.toString()) {
        ElMessage.info(i18n.t('tips.enteredValueInvalid'))
      }
    } else if (+val !== +value) {
      ElMessage.info(i18n.t('tips.enteredValueInvalid'))
    }
    targetParam.value = val
  } else {
    targetParam.value = value
  }
}

/**
 * 更新激光雷达高级属性
 * @param advancedInfo
 * @param paramName
 * @param value
 */
export function changeLidarAdvancedInfo (advancedInfo, { paramName, value }) {
  const targetParam = advancedInfo[paramName]
  if (paramName === 'Model') {
    targetParam.value = value
    if ((value.startsWith('RS') || value.startsWith('HS')) && !LidarFrequencyOptions.map(o => o.value).includes(+advancedInfo.Frequency.value)) {
      advancedInfo.Frequency.value = 5
    }
  } else if (targetParam.range) {
    const val = formatValue(targetParam, value)
    if (+val !== +value) {
      ElMessage.info(i18n.t('tips.enteredValueInvalid'))
    }
    targetParam.value = val
  } else {
    targetParam.value = value
  }
  if (paramName === 'Frequency' && (advancedInfo.Model.value.startsWith('RS') || advancedInfo.Model.value.startsWith('HS')) && ![5, 10, 20].includes(+value)) {
    setTimeout(() => {
      // 因为连续调用 ElMessage 会造成弹窗重叠
      ElMessage.warning(i18n.t('tips.frequencyNotBeSupported', { frequency: advancedInfo.Model.value }))
    }, 0)
  }
}

/**
 * 更新毫米波雷达高级属性
 * @param advancedInfo
 * @param paramName
 * @param value
 */
export function changeRadarAdvancedInfo (advancedInfo, { paramName, value }) {
  const targetParam = advancedInfo[paramName]
  if (paramName === 'F0_GHz') {
    targetParam.value = value
    if (value === 24) {
      ElMessage.info(i18n.t('tips.modifyRadarCarriedFrequency', { frequency: 24, range: '[15, 30]' }))
      advancedInfo.L0_dB.range = '[15,30]'
      advancedInfo.L0_dB.value = 30
    } else {
      const { value: L0Value } = advancedInfo.L0_dB
      advancedInfo.L0_dB.range = '[5,10]'
      if (L0Value > 10 || L0Value < 5) {
        ElMessage.info(i18n.t('tips.modifyRadarCarriedFrequency', { frequency: 77, range: '[5, 10]' }))
        advancedInfo.L0_dB.value = 10
      }
    }
  } else if (targetParam.range) {
    const val = formatValue(targetParam, value)
    if (+val !== +value) {
      ElMessage.info(i18n.t('tips.enteredValueInvalid'))
    }
    targetParam.value = val
  } else {
    targetParam.value = value
  }
}

/**
 * 更新OBU高级属性
 * @param advancedInfo
 * @param paramName
 * @param value
 */
export function changeObuAdvancedInfo (advancedInfo, { paramName, value }) {
  changeCameraAdvancedInfo(advancedInfo, { paramName, value })
  const lossRangeParamNames = ['v2x_loss_burs_min', 'v2x_loss_burs_max']
  if (lossRangeParamNames.includes(paramName)) {
    compareRangeValues(advancedInfo, lossRangeParamNames, paramName)
  }

  const delayRangeParamNames = ['v2x_delay_uniform_min', 'v2x_delay_uniform_max']
  if (delayRangeParamNames.includes(paramName)) {
    compareRangeValues(advancedInfo, delayRangeParamNames, paramName)
  }
}

/**
 * 以下三个fn处理各自传感器数据联动逻辑
 * @param options
 * @param param
 * @param paramName
 * @param value
 * @return {*}
 */
function changeCameraParam (options, param, { paramName, value }) {
  let targetParam = options[paramName]
  if (paramName === 'IntrinsicParamType') {
    targetParam = options.InsideParamGroup
  }
  if (targetParam.range) {
    const val = formatValue(targetParam, value)
    if (isArray(val) && isArray(value)) {
      if (val.toString() !== value.toString()) {
        ElMessage.info(i18n.t('tips.enteredValueInvalid'))
      }
    } else if (+val !== +value) {
      ElMessage.info(i18n.t('tips.enteredValueInvalid'))
    }
    param[paramName] = val
  } else {
    param[paramName] = value
  }
  return param
}

/**
 * 更新激光雷达传感器参数
 * @param options
 * @param param
 * @param paramName
 * @param value
 * @return {*}
 */
function changeLidarParam (options, param, { paramName, value }) {
  const targetParam = options[paramName]
  if (paramName === 'Model') {
    param[paramName] = value
    if ((value.startsWith('RS') || value.startsWith('HS')) && !LidarFrequencyOptions.map(o => o.value).includes(+param.Frequency)) {
      param.Frequency = 5
    }
  } else if (targetParam.range) {
    const val = formatValue(targetParam, value)
    if (isArray(val) && isArray(value)) {
      if (val.toString() !== value.toString()) {
        ElMessage.info(i18n.t('tips.enteredValueInvalid'))
      }
    } else if (+val !== +value) {
      ElMessage.info(i18n.t('tips.enteredValueInvalid'))
    }
    param[paramName] = val
  } else {
    param[paramName] = value
  }
  if (paramName === 'Frequency' && (param.Model.startsWith('RS') || param.Model.startsWith('HS')) && ![5, 10, 20].includes(+value)) {
    setTimeout(() => {
      // 因为连续调用 ElMessage 会造成弹窗重叠
      ElMessage.warning(i18n.t('tips.frequencyNotBeSupported', { frequency: param.Model }))
    }, 0)
  }
  return param
}

/**
 * 更新毫米波雷达传感器参数
 * @param options
 * @param param
 * @param paramName
 * @param value
 * @return {*}
 */
function changeRadarParam (options, param, { paramName, value }) {
  const targetParam = options[paramName]
  if (paramName === 'F0_GHz') {
    param[paramName] = value
    if (value === 24) {
      ElMessage.info(i18n.t('tips.modifyRadarCarriedFrequency', { frequency: 24, range: '[15, 30]' }))
      options.L0_dB.range = '[15,30]'
      param.L0_dB = 30
    } else {
      const L0Value = param.L0_dB
      options.L0_dB.range = '[5,10]'
      if (L0Value > 10 || L0Value < 5) {
        ElMessage.info(i18n.t('tips.modifyRadarCarriedFrequency', { frequency: 77, range: '[5, 10]' }))
        param.L0_dB = 10
      }
    }
  } else if (targetParam.range) {
    const val = formatValue(targetParam, value)
    if (+val !== +value) {
      ElMessage.info(i18n.t('tips.enteredValueInvalid'))
    }
    param[paramName] = val
  } else {
    param[paramName] = value
  }
  return param
}

/**
 * 更新OBU传感器参数
 * @param options
 * @param param
 * @param paramName
 * @param value
 * @return {*}
 */
function changeObuParam (options, param, { paramName, value }) {
  changeCameraParam(options, param, { paramName, value })
  const lossRangeParamNames = ['v2x_loss_burs_min', 'v2x_loss_burs_max']
  if (lossRangeParamNames.includes(paramName)) {
    compareRangeValuesForSensor(options, param, lossRangeParamNames, paramName)
  }

  const delayRangeParamNames = ['v2x_delay_uniform_min', 'v2x_delay_uniform_max']
  if (delayRangeParamNames.includes(paramName)) {
    compareRangeValuesForSensor(options, param, delayRangeParamNames, paramName)
  }
  return param
}

/**
 * 更新超声波雷达参数
 * @param options
 * @param param
 * @param paramName
 * @param value
 * @return {*}
 */
function changeUltrasonicParam (options, param, { paramName, value }) {
  changeCameraParam(options, param, { paramName, value })
  if (paramName === 'PollTurn') {
    param.PollTurn = value.toUpperCase().replace(/[^01SR]/g, '')
    if (param.PollTurn.length > 20) {
      param.PollTurn = param.PollTurn.substring(0, 20)
    }
  }
  return param
}

/**
 * 对比范围数据
 * @param options
 * @param param
 * @param paramNames
 * @param inputParamName
 */
function compareRangeValuesForSensor (options, param, paramNames, inputParamName) {
  const [minParamName, maxParamName] = paramNames
  const max = options[maxParamName]
  const min = options[minParamName]
  if (inputParamName === minParamName && param[minParamName] >= param[maxParamName]) {
    ElMessage.info(i18n.t('tips.minMastLtMax', {
      min: i18n.t(min.name),
      max: i18n.t(max.name),
    }))
    param[minParamName] = param[maxParamName] - (min.step || 1)
  } else if (inputParamName === maxParamName && param[maxParamName] <= param[minParamName]) {
    ElMessage.info(i18n.t('tips.maxMastGtMin', {
      min: i18n.t(min.name),
      max: i18n.t(max.name),
    }))
    param[maxParamName] = param[minParamName] + (max.step || 1)
  }
}

/**
 * 更新雷达参数
 * @param type
 * @param param
 * @param payload
 * @return {*}
 */
export function changeSensorParam (type, param, payload) {
  let options
  switch (type) {
    case 'Camera':
      options = cameraAdvancedInfo
      return changeCameraParam(options, param, payload)
    case 'Fisheye':
      options = fisheyeAdvancedInfo
      return changeCameraParam(options, param, payload)
    case 'Semantic':
      options = cameraAdvancedInfo
      return changeCameraParam(options, param, payload)
    case 'Depth':
      options = cameraAdvancedInfo
      return changeCameraParam(options, param, payload)
    case 'Truth':
      options = truthAdvancedInfo
      return changeCameraParam(options, param, payload)
    case 'GPS':
      options = GPSAdvancedInfo
      return changeCameraParam(options, param, payload)
    case 'IMU':
      options = IMUAdvancedInfo
      return changeCameraParam(options, param, payload)
    case 'Ultrasonic':
      options = ultrasonicAdvancedInfo
      return changeUltrasonicParam(options, param, payload)
    case 'OBU':
      options = OBUAdvancedInfo
      return changeObuParam(options, param, payload)
    case 'TraditionalLidar':
      options = lidarAdvancedInfo
      return changeLidarParam(options, param, payload)
    case 'Radar':
      options = radarAdvancedInfo
      return changeRadarParam(options, param, payload)
    default:
  }
}
