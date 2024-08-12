import { Group } from 'three'
import {
  getOtherConfig,
  getPoleConfig,
  getRoadSignConfig,
  getSensorConfig,
  getSignalBoardConfig,
  getTrafficLightConfig,
} from '../config'
import { promisify, queryCustomModelConfigByName } from '../utils/common'
import { createParkingSpace } from './parkingSpace'
import { createPole } from './pole'
import { createRoadSign } from './roadSign'
import { createSignalBoard } from './signalBoard'
import { createTrafficLight } from './trafficLight'
import { createSensor } from './sensor'
import { createOther } from './other'
import { createCustomRoadSign, customRoadSignList } from './customRoadSign'
import { createCustomModel } from './customModel'

// 渲染函数的入口
export function createElements (options) {
  const {
    data = [],
    types,
    ignoreAlt = false,
    mapVersion = '',
    ignoreSize = false, // 是否忽略尺寸的变化
  } = options

  // tadsim v1.0 版本，视为是旧版地图编辑器生成的地图
  const isOldMap = mapVersion === 'tadsim v1.0'

  // 地图元素有效类型
  const elementEnabled = {
    pole: false,
    roadSign: false,
    parkingSpace: false,
    other: false,
    signalBoard: false,
    trafficLight: false,
    sensor: false,
    customModel: false,
  }

  // 通过透传的地图元素类型，判断当前渲染地图元素的有效性
  for (const type of types) {
    elementEnabled[type] = true
  }

  // 所有地图元素最外层的容器
  const container = new Group()
  container.name = 'mapElementsContainer'

  const renderPromises = []

  // 遍历所有的地图元素，解析对应的数据，并创建对应要渲染的模型
  for (const _data of data) {
    const { name, strType: type, strSubType: subtype } = _data

    // 先做自定义导入模型的查询，避免自定义模型的名称跟内置模型的 name 属性重名情况
    const queryCustomModelRes = elementEnabled.customModel && queryCustomModelConfigByName(name)
    const isCustomModel = type === 'custom'
    if (queryCustomModelRes && isCustomModel) {
      // 如果是自定义导入模型，则单独解析
      renderPromises.push(
        createCustomModel({
          data: _data,
          config: queryCustomModelRes,
          parent: container,
          ignoreAlt,
        }),
      )
      continue
    }

    // 通过查找配置文件的结果，来判断当前物体的具体类型
    const queryPoleRes = elementEnabled.pole && getPoleConfig(name)
    const queryRoadSignRes = elementEnabled.roadSign && getRoadSignConfig(name)
    const queryOtherRes = elementEnabled.other && getOtherConfig(name)
    const querySignalBoardRes =
      elementEnabled.signalBoard && getSignalBoardConfig(type, subtype)
    const queryTrafficLightRes =
      elementEnabled.trafficLight && getTrafficLightConfig(type, subtype)
    const querySensorRes = elementEnabled.sensor && getSensorConfig(name)

    // 如果是停车位，则单独处理
    if (type === 'parkingSpace' && elementEnabled.parkingSpace) {
      renderPromises.push(
        createParkingSpace({
          data: _data,
          parent: container,
          ignoreAlt,
        }),
      )
      continue
    }

    // 如果是特殊的路面标线，单独处理
    if (elementEnabled.roadSign && customRoadSignList.includes(name)) {
      renderPromises.push(
        createCustomRoadSign({
          name,
          data: _data,
          parent: container,
          isOldMap,
          ignoreAlt,
        }),
      )
      continue
    }

    let poleData
    if (querySignalBoardRes || queryTrafficLightRes || querySensorRes) {
      // 解析 userdata 中的 pole_id
      const { userData } = _data
      if (!userData) continue
      let poleId
      for (const _userData of userData) {
        if (poleId !== undefined) continue
        for (const key of Object.keys(_userData)) {
          if (key === 'pole_id') {
            poleId = _userData[key]
          }
        }
      }
      // 获取对应的 pole 的数据
      poleData = poleId && getPoleById(data, poleId)
    }

    if (queryPoleRes) {
      renderPromises.push(
        promisify(
          createPole({
            data: _data,
            parent: container,
            ignoreAlt,
            isOldMap,
          }),
        ),
      )
    } else if (queryRoadSignRes) {
      renderPromises.push(
        promisify(
          createRoadSign({
            data: _data,
            config: queryRoadSignRes,
            parent: container,
            ignoreAlt,
            isOldMap,
            ignoreSize,
          }),
        ),
      )
    } else if (queryOtherRes) {
      renderPromises.push(
        promisify(
          createOther({
            data: _data,
            config: queryOtherRes,
            parent: container,
            ignoreAlt,
            isOldMap,
            ignoreSize,
          }),
        ),
      )
    } else if (querySignalBoardRes && poleData) {
      // 解析杆上的标志牌（需要依赖杆的位置和角度更新位置）
      renderPromises.push(
        promisify(
          createSignalBoard({
            data: _data,
            config: querySignalBoardRes,
            poleData,
            parent: container,
            ignoreAlt,
            isOldMap,
          }),
        ),
      )
    } else if (queryTrafficLightRes && poleData) {
      // 解析杆上的信号灯（需要依赖杆的位置和角度更新位置）
      renderPromises.push(
        promisify(
          createTrafficLight({
            data: _data,
            config: queryTrafficLightRes,
            poleData,
            parent: container,
            ignoreAlt,
            isOldMap,
          }),
        ),
      )
    } else if (querySensorRes && poleData) {
      // 解析杆上的传感器（需要依赖杆的位置和角度更新位置）
      renderPromises.push(
        promisify(
          createSensor({
            data: _data,
            config: querySensorRes,
            poleData,
            parent: container,
            ignoreAlt,
            isOldMap,
          }),
        ),
      )
    }
  }

  // 并行执行所有地图元素的创建逻辑
  Promise.all(renderPromises)

  return container
}

// 从所有的地图元素数据中，找到目标的杆数据
export function getPoleById (data, id) {
  const targetPole = data.find(_data => String(_data.id) === String(id))
  if (!targetPole) return null

  return targetPole
}
