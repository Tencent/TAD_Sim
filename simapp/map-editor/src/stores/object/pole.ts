import { type Intersection, Vector3 } from 'three'
import { createDefaultObjectData, getLocationInRoad } from './common'
import { type IPoleConfig, getPoleConfig } from '@/config/pole'
import {
  fixedPrecision,
  getLocationByST,
  getLookAtPointByAngle,
  getLookAtPointByYaw,
  getVerticalVector,
} from '@/utils/common3d'
import { genObjectId } from '@/utils/guid'
import { getObject, getRoad, setObject } from '@/utils/mapCache'
import root3d from '@/main3d/index'

// 由于横杆在靠近道路参考线时，横杆看向投影点的角度偏差可能会偏离 90° 直角
// 基于杆在参考线的位置，计算跟切线方向垂直的看向目标点
export function getValidLookAtPoint (params: {
  tangent: common.vec3
  position: common.vec3
  t: number
}) {
  const { tangent, position, t } = params
  // 刚垂直方向为切线方向顺时针旋转后的方向
  const vertical = getVerticalVector(tangent)
  const _position = new Vector3(position.x, position.y, position.z)
  const offset = t <= 0 ? -1 : 1
  const targetPoint = _position.clone().addScaledVector(vertical, offset)
  return targetPoint
}

/**
 * 基于配置项，创建一个杆的数据结构
 * @param params
 */
export function createPole (params: {
  option: IPoleConfig
  intersection: Intersection
}) {
  const { option, intersection } = params
  const { name, showName, type, subtype } = option
  const poleData = createDefaultObjectData()
  poleData.id = genObjectId()
  poleData.mainType = 'pole'
  poleData.name = name
  poleData.showName = showName
  poleData.type = type
  poleData.subtype = subtype

  // 基于鼠标交互结果，更新物体的【空间属性】
  const { point, object } = intersection
  // @ts-expect-error
  const { roadId = '', junctionId = '' } = object

  const poleConfig = getPoleConfig(name)
  if (!poleConfig) return null

  if (name === 'Cross_Pole' || name === 'Cantilever_Pole') {
    // 横杆
    const { style } = poleConfig
    poleData.radius = style.vRadius
    poleData.height = fixedPrecision(style.height)
  } else {
    // 竖杆
    // 半径
    const { style } = poleConfig
    poleData.radius = style.vRadius
    // 高度
    poleData.height = fixedPrecision(style.height)
  }

  // 将位置同步到鼠标射线检测到的位置
  poleData.position = {
    x: point.x,
    y: point.y,
    z: point.z,
  }

  // 基于交互的配置项，更新物体的一些属性
  if (roadId) {
    poleData.roadId = roadId
    const locationData = getLocationInRoad({
      point,
      roadId,
      useElevation: true,
    })
    if (!locationData) return null
    const { s, t, percent, tangent, closestPointOnRefLine } = locationData
    poleData.s = s
    poleData.t = t
    poleData.closestPointTangent = tangent
    poleData.closestPointPercent = percent
    poleData.closestPoint = closestPointOnRefLine

    // 为了保证杆始终看向跟 s 轴投影点切线方向的垂直方向，在形成的角度偏离直角过多时，手动计算新的 lookAtPoint
    const lookAtPoint = getValidLookAtPoint({
      tangent,
      position: point,
      t,
    })

    poleData.lookAtPoint = lookAtPoint
    // 道路上的杆吗，默认都是看向参考线的，偏航角始终为 90°
    // 区分正反向车道
    poleData.yaw = t > 0 ? -90 : 90
  }
  if (junctionId) {
    // 在路口中放置，只有保存时，才会针对最近的 laneLink 计算参考线坐标
    // 交互时不需要计算参考线坐标，可以直接使用交互点的坐标，路口中的物体角度通过属性面板调整
    poleData.junctionId = junctionId
    // 世界坐标系下的角度
    poleData.angle = 0
  }

  // 将新增的物体缓存到本地
  setObject(poleData.id, poleData)

  return poleData
}

// 移动杆
export function movePole (params: {
  objectId: string
  point: common.vec3
  roadId?: string
  refLineLocation?: biz.IRefLineLocation // refLineLocation 跟 roadId 同时存在
}) {
  const {
    objectId,
    point: targetPoint,
    refLineLocation = null,
    roadId = '',
  } = params
  const poleData = getObject(objectId)
  if (!poleData) return null

  // 更新道路上的杆时对应的定制化属性更新
  if (roadId) {
    // 如果传入了已经计算好的参考线坐标系数据，则可直接用，无须重复计算
    const locationData =
      refLineLocation ||
      getLocationInRoad({
        point: targetPoint,
        roadId,
        useElevation: true,
      })
    if (!locationData) return null
    const { s, t, percent, tangent, closestPointOnRefLine } = locationData

    poleData.s = s
    poleData.t = t
    poleData.closestPointTangent = tangent
    poleData.closestPointPercent = percent
    poleData.closestPoint = closestPointOnRefLine

    // 为了保证杆始终看向跟 s 轴投影点切线方向的垂直方向，在形成的角度偏离直角过多时，手动计算新的 lookAtPoint
    const lookAtPoint = getValidLookAtPoint({
      tangent,
      position: targetPoint,
      t,
    })

    poleData.lookAtPoint = lookAtPoint
    // 道路上的杆吗，默认都是看向参考线的，偏航角始终为 90°
    // 区分正反向车道
    poleData.yaw = t > 0 ? -90 : 90
  } else {
    // 针对路口中的物体，基于全局的角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: targetPoint,
      angle: poleData.angle,
    })
    poleData.lookAtPoint = lookAtPoint
  }

  // 道路和路口中的杆，都需要更新位置
  poleData.position = {
    x: targetPoint.x,
    y: targetPoint.y,
    z: targetPoint.z,
  }

  setObject(poleData.id, poleData)

  return poleData
}

// 通过参考线坐标系的 st 值更新杆的位置
export function movePoleByST (params: {
  objectId: string
  s: number
  t: number
}) {
  const { objectId, s, t } = params
  const poleData = getObject(objectId)
  if (!poleData) return

  // 通过 st 修改物体的位置，说明 roadId 一定存在
  const { roadId } = poleData
  const roadData = getRoad(roadId)
  if (!roadData) return

  // 返回的 s 是校正后的值
  const {
    s: newS,
    refLinePoint,
    targetPoint,
    tangent,
    percent,
  } = getLocationByST({
    s,
    t,
    elevationPath: roadData.elevationPath,
    curvePath: roadData.keyPath,
  })

  // 更新源数据属性
  poleData.s = newS
  poleData.t = t
  poleData.position = {
    x: targetPoint.x,
    y: targetPoint.y,
    z: targetPoint.z,
  }
  poleData.closestPoint = refLinePoint
  poleData.closestPointPercent = percent
  poleData.closestPointTangent = tangent

  // 为了保证杆始终看向跟 s 轴投影点切线方向的垂直方向，在形成的角度偏离直角过多时，手动计算新的 lookAtPoint
  const lookAtPoint = getValidLookAtPoint({
    tangent,
    position: targetPoint,
    t,
  })

  poleData.lookAtPoint = lookAtPoint
  // 道路上的杆吗，默认都是看向参考线的，偏航角始终为 90°
  // 区分正反向车道
  poleData.yaw = t > 0 ? -90 : 90

  setObject(poleData.id, poleData)

  return poleData
}

// 更新杆的角度
export function rotatePole (params: {
  objectId: string
  yaw?: number
  angle?: number
}) {
  const { objectId, yaw, angle } = params
  const poleData = getObject(objectId)
  if (!poleData) return null

  const { position } = poleData
  if (yaw !== undefined) {
    // 针对道路上的物体，基于偏航角来更新物体的角度
    // 基于变更的偏航角，调整杆看向的目标点
    const { closestPointTangent } = poleData
    if (!closestPointTangent) return null
    const lookAtPoint = getLookAtPointByYaw({
      tangent: closestPointTangent,
      point: position,
      yaw,
    })
    poleData.yaw = yaw
    poleData.lookAtPoint = lookAtPoint
  }

  if (angle !== undefined) {
    // 针对路口中的物体，基于全局的角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: position,
      angle,
    })
    poleData.angle = angle
    poleData.lookAtPoint = lookAtPoint
  }

  setObject(poleData.id, poleData)

  return poleData
}

// 通过射线交互的点，计算竖杆上准确的【局部】位置坐标
export function calcPositionOnVerticalPole (params: {
  poleId: string
  point: common.vec3
}): common.vec3 | null {
  const { poleId, point } = params
  const pole = getObject(poleId)
  if (!pole) return { x: 0, y: 0, z: 0 }

  const _point = new Vector3(point.x, point.y, point.z)
  const poleContainer = root3d.mapElementsContainer.getObjectByProperty(
    'objectId',
    poleId,
  )
  if (!poleContainer) return null

  // 将世界坐标系的绝对坐标，转换成 pole 容器中的局部坐标
  // matrix在反转之前，一定要 clone! 避免修改原矩阵
  _point.applyMatrix4(poleContainer.matrix.clone().invert())

  const poleConfig = getPoleConfig(pole.name)
  if (!poleConfig) return null

  const { style } = poleConfig
  let _x: number
  // 控制竖杆上物体的高度在区间内
  const _y = fixedPrecision(Math.max(0, Math.min(_point.y, style.height)))
  const _z = 0
  if (_point.x >= 0) {
    _x = style.vRadius
  } else {
    _x = -style.vRadius
  }
  return {
    x: _x,
    y: _y,
    z: _z,
  }
}
// 通过射线交互的点，计算横杆上准确的【局部】位置坐标
export function calcPositionOnHorizontalPole (params: {
  poleId: string
  point: common.vec3
}): common.vec3 | null {
  const { poleId, point } = params

  const _point = new Vector3(point.x, point.y, point.z)
  const poleContainer = root3d.mapElementsContainer.getObjectByProperty(
    'objectId',
    poleId,
  )
  if (!poleContainer) return null

  // 将世界坐标系的绝对坐标，转换成 pole 容器中的局部坐标
  // matrix在反转之前，一定要 clone! 避免修改原矩阵
  _point.applyMatrix4(poleContainer.matrix.clone().invert())

  const pole = getObject(poleId)
  if (!pole) return null
  const poleConfig = getPoleConfig(pole.name)
  if (!poleConfig) return null

  const { style } = poleConfig
  if (!style.hRadius || !style.length) return null
  let _x: number
  if (_point.x >= 0) {
    _x = style.hRadius
  } else {
    _x = -style.hRadius
  }

  const _y = style.height - style.hRadius

  // 横杆的半径 < 竖杆的半径，如果距离过小会导致物体模型跟竖杆穿模
  // 目前杆上物体最宽的模型是横排信号灯，暂定一个距离避免穿模
  const objectOffset = 0.8
  const minDist = style.vRadius + objectOffset
  const maxDist = style.length + style.vRadius
  // 控制横杆上物体到竖杆的距离在 [min, max] 区间内
  const _z = fixedPrecision(Math.max(minDist, Math.min(maxDist, _point.z)))

  return {
    x: _x,
    y: _y,
    z: _z,
  }
}
