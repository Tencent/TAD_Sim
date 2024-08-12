import _ from 'lodash'
import axios from './axios'
import {
  GPSAdvancedInfo,
  IMUAdvancedInfo,
  OBUAdvancedInfo,
  cameraAdvancedInfo,
  fisheyeAdvancedInfo,
  lidarAdvancedInfo,
  radarAdvancedInfo,
  semanticAndDepthAdvancedInfo,
  truthAdvancedInfo,
  ultrasonicAdvancedInfo,
} from '@/store/modules/sensor/constants'

const advancedInfoObj = {
  Camera: cameraAdvancedInfo,
  Fisheye: fisheyeAdvancedInfo,
  Semantic: semanticAndDepthAdvancedInfo,
  Depth: semanticAndDepthAdvancedInfo,
  Radar: radarAdvancedInfo,
  TraditionalLidar: lidarAdvancedInfo,
  Truth: truthAdvancedInfo,
  IMU: IMUAdvancedInfo,
  GPS: GPSAdvancedInfo,
  Ultrasonic: ultrasonicAdvancedInfo,
  OBU: OBUAdvancedInfo,
}

/**
 * 获取所有种类的 catalog
 * @returns {Promise<any>}
 */
export function getAllCatalogs () {
  return axios.get('http://127.0.0.1:9000/catalogs/getcatalogslist')
    .then((data) => {
      const {
        driver_catalog: driverCatalog,
        miscobject_catalog: miscobjectCatalog,
        pedestrian_catalog: pedestrianCatalog,
        vehicle_catalog: vehicleCatalog,
      } = data.catalog_list

      if (!driverCatalog?.length) {
        throw new Error('主车配置数据为空，请联系系统管理员。')
      }
      // ensure planner.sensor.group
      driverCatalog.forEach((c) => {
        c.sensor = {
          groupName: c.catalogParams[0].properties.sensorGroup,
          group: [],
        }
      })
      driverCatalog.sort((a, b) => {
        if (a.preset && !b.preset) {
          return -1
        }
        if (!a.preset && b.preset) {
          return 1
        }
        return 0
      })
      return {
        driverCatalog: driverCatalog || [],
        vehicleCatalog: vehicleCatalog || [],
        pedestrianCatalog: pedestrianCatalog || [],
        miscobjectCatalog: miscobjectCatalog || [],
      }
    })
}

/**
 * 处理响应传感器参数
 * @param {object} sensor - 传感器对象
 * @returns {Promise<void>}
 */
async function handleRespSensorParam (sensor) {
  const { type, param, idx, name } = sensor
  if (param.Noise_Intensity && ['Low', 'Middle', 'High', 'Off'].includes(param.Noise_Intensity)) {
    param.Noise_Intensity = 0
  }
  const advancedInfo = advancedInfoObj[type]
  if (param.DisplayMode !== undefined) {
    // 提前处理一下各种相机的DisplayMode，从字符串改成数字
    const value = param.DisplayMode
    if (value === 'Color') {
      param.DisplayMode = '0'
    } else if (value === 'Gray') {
      param.DisplayMode = '1'
    } else if ([0, 1].includes(+value)) {
      param.DisplayMode = `${+value}`
    } else {
      // 特殊情况兜底
      param.DisplayMode = '0'
    }
  }

  if (type === 'OBU' && 'Frequency' in param) {
    // 老式OBU数据转换新版OBU数据，生成一套默认的
    Object.keys(param).forEach(key => delete param[key])
    Object.keys(OBUAdvancedInfo).forEach((key) => {
      param[key] = OBUAdvancedInfo[key].value
    })
    updateSensor({
      sensor: {
        idx,
        name,
        type,
        param,
      },
    })
  }
  Object.keys(advancedInfo).forEach((key) => {
    const { type, value } = advancedInfo[key]
    if (param[key]) {
      if (type === 'Number') {
        param[key] = param[key] * 1
      }
      if (type === 'Array') {
        param[key] = param[key].split(',')
      }
      if (type === 'Boolean' && typeof param[key] !== 'boolean') {
        param[key] = param[key] === 'true'
      }
    }
    if (key === 'InsideParamGroup') {
      if (param.IntrinsicParamType === undefined) {
        param.IntrinsicParamType = value
      } else {
        param.IntrinsicParamType = param.IntrinsicParamType * 1
      }
    } else if (param[key] === undefined) {
      param[key] = value
    }
  })
}

/**
 * 处理请求传感器参数
 * @param {string} type - 传感器类型
 * @param {object} param - 传感器参数
 * @returns {object} - 处理后的传感器参数
 */
function handleReqSensorParam (type, param) {
  const advancedInfo = advancedInfoObj[type]
  if (param.DisplayMode !== undefined) {
    // 单独处理一下各种相机的DisplayMode，从字符串改成数字
    const value = param.DisplayMode
    if (value === 'Color') {
      param.DisplayMode = 0
    } else if (value === 'Gray') {
      param.DisplayMode = 1
    } else if ([0, 1].includes(+value)) {
      // 已经是数字格式，do nothing
      param.DisplayMode = +value
    } else {
      // 特殊情况兜底
      param.DisplayMode = 0
    }
  }
  Object.keys(advancedInfo).forEach((key) => {
    const { type, value } = advancedInfo[key]
    if (key === 'InsideParamGroup') {
      if (param.IntrinsicParamType === undefined) {
        param.IntrinsicParamType = value.toString()
      } else {
        param.IntrinsicParamType = param.IntrinsicParamType.toString()
      }
    } else if (param[key] === undefined) {
      param[key] = value
    }
    if (['Number', 'Array', 'Boolean'].includes(type) && key !== 'InsideParamGroup') {
      param[key] = param[key].toString()
    }
  })
  return param
}

/**
 * 获取传感器列表
 * @returns {Promise<any[]>}
 */
export async function getSensorList () {
  const { sensors } = await axios.get('http://127.0.0.1:9000/sensors/load')
  for (let i = 0; i < sensors.length; i++) {
    await handleRespSensorParam(sensors[i])
  }
  return sensors
}

/**
 * 更新传感器
 * @param {object} sensor - 传感器对象
 * @returns {Promise<void>}
 */
export function updateSensor ({ sensor }) {
  const param = handleReqSensorParam(sensor.type, _.cloneDeep(sensor.param))
  return axios.post('http://127.0.0.1:9000/sensors/update', {
    cmd: 'update',
    sensor: {
      ...sensor,
      param,
    },
  }).then(({ err, message }) => {
    if (err !== 0) {
      return Promise.reject(new Error(message))
    }
  })
}

/**
 * 添加传感器
 * @param {object} sensor - 传感器对象
 * @returns {Promise<void>}
 */
export function addSensor ({ sensor }) {
  const param = handleReqSensorParam(sensor.type, _.cloneDeep(sensor.param))
  return axios.post('http://127.0.0.1:9000/sensors/add', {
    cmd: 'add',
    sensor: {
      ...sensor,
      param,
    },
  }).then(({ err, message }) => {
    if (err !== 0) {
      return Promise.reject(new Error(message))
    }
  })
}

/**
 * 删除传感器
 * @param {number} idx - 传感器索引
 * @returns {Promise<void>}
 */
export function deleteSensor ({ idx }) {
  return axios.post('http://127.0.0.1:9000/sensors/delete', {
    cmd: 'delete',
    sensor_idx: idx,
  }).then(({ err, message }) => {
    if (err !== 0) {
      return Promise.reject(new Error(message))
    }
  })
}

/**
 * 更新主车
 * @param {string} oldname - 旧的主车名称
 * @param {object} catalog - 规划对象
 * @returns {Promise<void>}
 */
export function updatePlanner ({ oldname, catalog }) {
  return axios.post('http://127.0.0.1:9000/catalogs/update', {
    cmd: 'update',
    oldname,
    data: catalog,
  }).then(({ err, message }) => {
    if (err !== 0) {
      return Promise.reject(new Error(message))
    }
  })
}

export function addPlanner ({ catalog }) {
  return axios.post('http://127.0.0.1:9000/catalogs/add', {
    cmd: 'add',
    data: catalog,
  }).then(({ err, message }) => {
    if (err !== 0) {
      return Promise.reject(new Error(message))
    }
  })
}

/**
 * 删除主车
 * @param {string} catalogName - 主车名称
 * @returns {Promise<void>}
 */
export function deletePlanner (catalogName) {
  return axios.post('http://127.0.0.1:9000/catalogs/delete', {
    cmd: 'delete',
    variable: catalogName,
  }).then(({ err, message }) => {
    if (err !== 0) {
      return Promise.reject(new Error(message))
    }
  })
}

export function getDynamicList () {
  return axios.get('http://127.0.0.1:9000/dynamics/load')
    .then((data) => {
      return data.dynamics
    })
}

export function saveDynamic (data) {
  return axios.post('http://127.0.0.1:9000/dynamics/save', data)
    .then(({ err, message }) => {
      if (err !== 0) {
        return Promise.reject(new Error(message))
      }
    })
}

export function deleteDynamic (id) {
  return axios.post('http://127.0.0.1:9000/dynamics/delete', { id })
    .then(({ err, message }) => {
      if (err !== 0) {
        return Promise.reject(new Error(message))
      }
    })
}

/**
 * 获取传感器组列表
 * @returns {Promise<any[]>}
 */
export function getSensorGroups () {
  return axios.get('http://127.0.0.1:9000/sensorGroup/load').then(res => res.sensor_groups || [])
}

/**
 * 添加传感器组
 * @param {object} data - 传感器组数据对象
 * @returns {Promise<void>}
 */
export function addSensorGroup (data) {
  return axios.post('http://127.0.0.1:9000/sensorGroup/save', data)
}

/**
 * 修改传感器组
 * @param {object} data - 传感器组数据对象
 * @returns {Promise<void>}
 */
export function modifySensorGroup (data) {
  return axios.post('http://127.0.0.1:9000/sensorGroup/save', data)
}

/**
 * 删除传感器组
 * @param {object} data - 传感器组数据对象
 * @returns {Promise<void>}
 */
export function delSensorGroup (data) {
  return axios.post('http://127.0.0.1:9000/sensorGroup/delete', data)
}
