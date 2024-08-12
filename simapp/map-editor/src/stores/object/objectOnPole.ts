// 杆上物体（信号灯、标志牌、传感器）共用的原子化方法
import { type Intersection, MathUtils, Vector3 } from 'three'
import { createDefaultObjectData } from './common'
import {
  calcPositionOnHorizontalPole,
  calcPositionOnVerticalPole,
} from './pole'
import { genObjectId } from '@/utils/guid'
import { getPoleConfig } from '@/config/pole'
import { getObject, setObject } from '@/utils/mapCache'
import type { ISignalBoardConfig } from '@/config/signalBoard'
import type { ITrafficLightConfig } from '@/config/trafficLight'
import {
  type ISensorConfig,
  getCameraParams,
  getLidarParams,
  getRadarParams,
  getRsuParams,
} from '@/config/sensor'

// 创建一个杆上物体
export function createObjectOnPole (params: {
  option: ITrafficLightConfig | ISignalBoardConfig | ISensorConfig
  intersection: Intersection
  type: 'trafficLight' | 'signalBoard' | 'sensor'
}) {
  const { option, intersection, type: objectType } = params
  const { name, showName, type, subtype } = option
  let objectData: biz.ITrafficLight | biz.ISignalBoard | biz.ISensor | null =
    null
  if (objectType === 'trafficLight') {
    objectData = {
      ...createDefaultObjectData(),
      poleId: '',
      controlRoadId: '',
      controlJunctionId: '',
      onVerticalPole: true, // 默认放置在竖直杆上
    } as biz.ITrafficLight
  } else if (objectType === 'signalBoard') {
    objectData = {
      ...createDefaultObjectData(),
      poleId: '',
      onVerticalPole: true,
    } as biz.ISignalBoard
  } else if (objectType === 'sensor') {
    const _objectData: biz.ISensor = {
      ...createDefaultObjectData(),
      poleId: '',
      onVerticalPole: true,
      deviceParams: null,
    }

    // 根据传感器的类型，挂载默认的设备参数配置
    switch (name) {
      case 'RSU':
        // 通信单元
        _objectData.deviceParams = getRsuParams()
        break
      case 'Camera':
        // 摄像头
        _objectData.deviceParams = getCameraParams()
        break
      case 'Lidar':
        // 激光雷达
        _objectData.deviceParams = getLidarParams()
        break
      case 'Millimeter_Wave_Radar':
        // 毫米波雷达
        _objectData.deviceParams = getRadarParams()
        break
      default:
        break
    }

    objectData = _objectData
  }

  if (!objectData) return null

  objectData.id = genObjectId()
  objectData.mainType = objectType
  objectData.name = name
  objectData.showName = showName
  objectData.type = type
  objectData.subtype = subtype

  // 基于鼠标交互结果，更新物体的【空间属性】
  const { object, point: intersectionPoint } = intersection
  // @ts-expect-error
  const { objectId, name: meshName } = object

  // 当前检测到的 object 应该是横杆或竖杆
  const poleData = getObject(objectId)
  if (!poleData) return null

  // 绑定父级 pole 的 id
  objectData.poleId = objectId
  // 让杆上的物体也绑定关联的道路或路口 id，便于查找
  if (poleData.roadId) {
    objectData.roadId = poleData.roadId
  } else if (poleData.junctionId) {
    objectData.junctionId = poleData.junctionId
  }

  // 基于射线检测到交点，计算标识牌放置的位置
  if (meshName === 'verticalPole' || meshName === 'verticalPolePlacementArea') {
    // 竖杆（包括横杆的竖直部分也走这个逻辑）
    const _point = calcPositionOnVerticalPole({
      poleId: objectId,
      point: intersectionPoint,
    })
    if (!_point) return null
    // 将计算后的局部坐标赋值
    // 默认将物体添加到杆的正方向上，即 x > 0
    objectData.position.x = _point.x >= 0 ? _point.x : -_point.x
    objectData.position.y = _point.y
    objectData.position.z = _point.z
    // 竖杆上初始放置的角度，默认为 0
    objectData.angle = 0
    objectData.onVerticalPole = true
  } else if (
    meshName === 'horizontalPole' ||
    meshName === 'horizontalPolePlacementArea'
  ) {
    // 横杆
    const _point = calcPositionOnHorizontalPole({
      poleId: objectId,
      point: intersectionPoint,
    })
    if (!_point) return null
    // 默认将物体添加到杆的正方向上，即 x > 0
    objectData.position.x = _point.x >= 0 ? _point.x : -_point.x
    objectData.position.y = _point.y
    objectData.position.z = _point.z
    // 调整成放置在横杆上
    objectData.onVerticalPole = false
  }

  setObject(objectData.id, objectData)

  return objectData
}

// 移动杆上物体
export function moveObjectOnPole (params: {
  objectId: string
  point: common.vec3
}) {
  const { objectId, point } = params
  const objectData = getObject(objectId) as
    | biz.ITrafficLight
    | biz.ISignalBoard
    | biz.ISensor
  if (!objectData) return null
  const { onVerticalPole, poleId } = objectData
  if (onVerticalPole) {
    // 竖杆上
    const _point = calcPositionOnVerticalPole({
      poleId,
      point,
    })
    if (!_point) return null
    objectData.position.y = _point.y
  } else {
    // 横杆上
    const _point = calcPositionOnHorizontalPole({
      poleId,
      point,
    })
    if (!_point) return null
    objectData.position.z = _point.z
  }

  setObject(objectData.id, objectData)

  return objectData
}

// 通过属性面板的高度和水平距离，更新杆上物体的位置
export function moveObjectOnPoleByHeightAndDist (params: {
  objectId: string
  height?: number
  distance?: number
}) {
  const { objectId, height, distance } = params
  const objectData = getObject(objectId) as
    | biz.ITrafficLight
    | biz.ISignalBoard
    | biz.ISensor
  if (!objectData) return null
  const { onVerticalPole } = objectData
  if (onVerticalPole && height !== undefined) {
    // 在竖杆上才能调整高度
    objectData.position.y = height
  } else if (!onVerticalPole && distance !== undefined) {
    // 在横杆上才能调整到竖杆的距离
    objectData.position.z = distance
  }
  setObject(objectData.id, objectData)

  return objectData
}

// 旋转竖杆上的物体
export function rotateObjectOnPole (params: {
  objectId: string
  angle: number
}) {
  const { objectId, angle } = params
  const objectData = getObject(objectId) as
    | biz.ITrafficLight
    | biz.ISignalBoard
    | biz.ISensor
  if (!objectData) return null
  const { onVerticalPole, position } = objectData
  // 只有竖杆才能响应
  if (!onVerticalPole) return null
  const poleData = getObject(objectData.poleId) as biz.IPole
  if (!poleData) return null
  const poleConfig = getPoleConfig(poleData.name)
  if (!poleConfig) return null

  // 将角度制转换成弧度制
  const rad = MathUtils.degToRad(angle)
  // 竖杆的半径
  const radius = poleConfig.style.vRadius
  // 绕竖杆圆心旋转 angle 角度后的新坐标
  const newPosition = new Vector3(
    radius * Math.cos(-rad),
    position.y,
    radius * Math.sin(-rad),
  )

  // 更新源数据的属性
  objectData.angle = angle
  objectData.position = newPosition

  setObject(objectData.id, objectData)
  return objectData
}

// 快速调整横杆上物体的朝向
// 正向：物体朝向来车方向；反向：物体背对来车方向
export function toggleDirection (params: {
  objectId: string
  isForward: boolean
}) {
  const { objectId, isForward } = params
  const objectData = getObject(objectId) as
    | biz.ITrafficLight
    | biz.ISignalBoard
    | biz.ISensor
  if (!objectData) return null
  const { onVerticalPole, position, angle } = objectData
  // 只有横杆上的物体才能响应
  if (onVerticalPole) return null

  // 正向，物体的局部坐标 x 为正数；反向，物体的局部坐标 x 为负数
  const { x: localX } = position

  // 如果要调整的朝向，跟当前朝向一致，则无须调整
  if ((localX > 0 && isForward) || (localX < 0 && !isForward)) return null

  // 调整朝向，位置也需要更新
  objectData.position = {
    x: -localX,
    y: position.y,
    z: position.z,
  }

  // 角度制
  objectData.angle = isForward ? 0 : 180

  setObject(objectData.id, objectData)

  return objectData
}

// 更新传感器的设备参数配置
export function updateSensorDeviceParams (params: {
  objectId: string
  deviceParams: any
}) {
  const { objectId, deviceParams } = params
  const objectData = getObject(objectId) as biz.ISensor
  if (!objectData) return null

  // 直接全量覆盖原有的属性
  objectData.deviceParams = deviceParams
  return objectData
}
