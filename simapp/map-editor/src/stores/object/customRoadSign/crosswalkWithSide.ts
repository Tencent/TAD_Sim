import type { Intersection } from 'three'
import {
  createDefaultObjectData,
  getAlongRoadFrontPoint,
  getLocationInRoad,
  getRoadWidth,
} from '../common'
import type { IRoadSignConfig } from '@/config/roadSign'
import { fixedPrecision, getLocationByST } from '@/utils/common3d'
import { genObjectId } from '@/utils/guid'
import { getObject, getRoad, setObject } from '@/utils/mapCache'
import { Constant } from '@/utils/business'

/**
 * 移动人行横道
 * @param params
 */
export function moveCrosswalkWithSide (params: {
  objectId: string
  point?: common.vec3
  s?: number
}) {
  const { objectId, point, s } = params
  const roadSignData = getObject(objectId)
  if (!roadSignData) return null

  const { roadId = '' } = roadSignData
  let crosswalkAttr
  if (point) {
    crosswalkAttr = getCrosswalkWithSideAttrOnRoad({
      objectId,
      roadId,
      point,
      crossBorder: true,
    })
  } else if (s !== undefined) {
    crosswalkAttr = getCrosswalkWithSideAttrOnRoadByST({
      objectId,
      roadId,
      s,
    })
  }

  if (!crosswalkAttr) return null

  // 移动人行横道
  const {
    length,
    s: newS,
    t: newT,
    closestPoint,
    closestPointTangent,
    closestPointPercent,
    position,
    projectNormal,
    lookAtPoint,
  } = crosswalkAttr

  // 更新路面标识源数据的属性
  roadSignData.s = newS
  roadSignData.t = newT
  roadSignData.closestPoint = closestPoint
  roadSignData.closestPointTangent = closestPointTangent
  roadSignData.closestPointPercent = closestPointPercent
  roadSignData.position = {
    x: position.x,
    y: position.y,
    z: position.z,
  }
  roadSignData.projectNormal = projectNormal
  roadSignData.lookAtPoint = lookAtPoint
  // 人行横道的宽度始终固定
  // 长度跟随当前道路的宽度动态调整
  roadSignData.length = fixedPrecision(
    length || Constant.crosswalkWithSideSize.length.default,
  )

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

/**
 * 获取人行横道在道路（单向或双向道路）中的宽度和道路参考线坐标系的 t 坐标
 * @param params
 */
function getCrosswalkLengthAndTCoordinate (params: {
  roadId: string
  percent: number
}) {
  const { roadId, percent } = params
  const road = getRoad(roadId)
  if (!road) return null

  // 如果是双向道路，则需要基于双向道路的长度来定义人行横道的长度
  const { forwardWidth, reverseWidth } = getRoadWidth({
    road,
    percent,
  })

  // 人行横道的长度
  const length = forwardWidth + reverseWidth
  // st 坐标系的 t 方向偏移量
  const newT = (reverseWidth - forwardWidth) / 2

  return {
    t: newT,
    length,
  }
}

/**
 * 通过 st 坐标，获取人行横道在某一处的位置属性
 * @param params
 */
export function getCrosswalkWithSideAttrOnRoadByST (params: {
  objectId: string
  roadId: string
  s: number
}) {
  const { objectId, roadId, s } = params
  const road = getRoad(roadId)
  if (!road) return null
  const objectData = getObject(objectId)
  if (!objectData) return null

  // 由于新的 s 坐标会计算出一个新的 t 坐标，因此旧的 t 坐标无效，需要基于新的 t 坐标来计算
  const percent = s / road.length

  const getRes = getCrosswalkLengthAndTCoordinate({
    roadId,
    percent,
  })
  if (!getRes) return null
  const { length, t: newT } = getRes

  const { targetPoint, refLinePoint, tangent, normal } = getLocationByST({
    s,
    t: newT,
    useElevationTangent: true,
    elevationPath: road.elevationPath,
    curvePath: road.keyPath,
    crossBorder: true, // 允许超出道路参考线坐标系的范围
  })

  const frontPoint = getAlongRoadFrontPoint({
    tangent,
    point: targetPoint,
  })

  return {
    length,
    s,
    t: newT,
    closestPoint: refLinePoint,
    closestPointTangent: tangent,
    closestPointPercent: percent,
    position: targetPoint,
    projectNormal: normal,
    lookAtPoint: frontPoint,
  }
}
// 获取人行横道在道路某一处位置的属性
export function getCrosswalkWithSideAttrOnRoad (params: {
  objectId?: string
  roadId: string
  point: common.vec3
  crossBorder?: boolean // 是否跨越原有的道路参考线范围
}) {
  const { objectId = '', roadId, point, crossBorder = false } = params
  const road = getRoad(roadId)
  if (!road) return null

  // 在目标道路上的坐标数据
  let locationData
  if (crossBorder) {
    const objectData = getObject(objectId)
    if (!objectData) return null
    // 允许位置在超过道路参考线 s 轴的一定范围中移动
    const range = Constant.crosswalkWithSideSize.offset + objectData.width / 2

    locationData = getLocationInRoad({
      point,
      roadId,
      useElevation: true,
      crossBorder: true,
      crossBorderRange: range,
    })
  } else {
    locationData = getLocationInRoad({
      point,
      roadId,
      useElevation: true,
    })
  }
  if (!locationData) return null
  const { s, percent } = locationData

  const getRes = getCrosswalkLengthAndTCoordinate({
    roadId,
    percent,
  })
  if (!getRes) return null
  const { length, t: newT } = getRes

  // 基于新的 st 坐标计算实际的位置
  const { targetPoint, refLinePoint, tangent, normal } = getLocationByST({
    s,
    t: newT,
    useElevationTangent: true,
    elevationPath: road.elevationPath,
    curvePath: road.keyPath,
    crossBorder: true, // 允许超出道路参考线坐标系的范围
  })
  const frontPoint = getAlongRoadFrontPoint({
    tangent,
    point: targetPoint,
  })

  return {
    length,
    s,
    t: newT,
    closestPoint: refLinePoint,
    closestPointTangent: tangent,
    closestPointPercent: percent,
    position: targetPoint,
    projectNormal: normal,
    lookAtPoint: frontPoint,
  }
}

// 创建人行横道的数据结构
export function createCrosswalkWithSide (params: {
  option: IRoadSignConfig
  intersection: Intersection
}) {
  const { option, intersection } = params
  const { name, showName, type, subtype } = option
  const roadSignData: biz.IRoadSign = {
    ...createDefaultObjectData(),
    size: [0, 0],
    basicWidth: 0,
    basicLength: 0,
    basicHeight: 0,
  }
  roadSignData.id = genObjectId()
  roadSignData.mainType = 'roadSign'
  roadSignData.name = name
  roadSignData.showName = showName
  roadSignData.type = type
  roadSignData.subtype = subtype

  // 基于鼠标交互结果，更新物体的【空间属性】
  const { object, point: intersectionPoint } = intersection
  // @ts-expect-error
  const { roadId } = object
  if (!roadId) return null

  // 计算人行横道在当前道路上的属性
  const crosswalkAttr = getCrosswalkWithSideAttrOnRoad({
    roadId,
    point: intersectionPoint,
    crossBorder: false,
  })
  if (!crosswalkAttr) return null

  const {
    length,
    s,
    t,
    closestPoint,
    closestPointTangent,
    closestPointPercent,
    position,
    projectNormal,
    lookAtPoint,
  } = crosswalkAttr

  roadSignData.roadId = roadId
  roadSignData.s = s
  roadSignData.t = t
  roadSignData.closestPoint = closestPoint
  roadSignData.closestPointTangent = closestPointTangent
  roadSignData.closestPointPercent = closestPointPercent
  roadSignData.position = {
    x: position.x,
    y: position.y,
    z: position.z,
  }
  roadSignData.projectNormal = projectNormal
  roadSignData.lookAtPoint = lookAtPoint
  // 默认的偏航角为0
  const yaw = 0
  roadSignData.yaw = yaw
  // 人行横道新建时的长和宽
  roadSignData.length = fixedPrecision(
    length || Constant.crosswalkWithSideSize.length.default,
  )
  roadSignData.width = Constant.crosswalkWithSideSize.width.default

  // 将人行横道数据添加到缓存中
  setObject(roadSignData.id, roadSignData)

  return roadSignData
}
