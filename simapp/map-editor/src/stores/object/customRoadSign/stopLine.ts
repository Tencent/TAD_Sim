import type { Intersection } from 'three'
import {
  createDefaultObjectData,
  getAlongRoadFrontPoint,
  getLocationInRoad,
  getRoadWidth,
} from '../common'
import type { IRoadSignConfig } from '@/config/roadSign'
import { genObjectId } from '@/utils/guid'
import { getObject, getRoad, setObject } from '@/utils/mapCache'
import { Constant } from '@/utils/business'
import { fixedPrecision, getLocationByST } from '@/utils/common3d'

/**
 * 移动停止线
 * @param params
 */
export function moveStopLine (params: {
  objectId: string
  point?: common.vec3
  s?: number
}) {
  const { objectId, point, s } = params
  const roadSignData = getObject(objectId)
  if (!roadSignData) return null

  const { roadId = '' } = roadSignData

  let stopLineAttr
  if (point) {
    stopLineAttr = getStopLineAttrOnRoad({
      roadId,
      point,
    })
  } else if (s !== undefined) {
    stopLineAttr = getStopLineAttrOnRoadByST({
      roadId,
      s,
      isForward: roadSignData.t <= 0,
    })
  }
  if (!stopLineAttr) return null
  // 移动停止线
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
  } = stopLineAttr

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
  // 停止线的宽度始终固定
  // 长度跟随当前道路的宽度动态调整
  roadSignData.length = fixedPrecision(length || Constant.stopLineSize.length)

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

// 通过 st 坐标获取停止线在当前道路上的属性
export function getStopLineAttrOnRoadByST (params: {
  roadId: string
  s: number
  isForward: boolean
}) {
  const { roadId, s, isForward } = params
  const road = getRoad(roadId)
  if (!road) return null

  // 由于新的 s 坐标会计算出一个新的 t 坐标，因此旧的 t 坐标无效，需要基于新的 t 坐标来计算
  const percent = s / road.length
  // 判断之前 t 坐标的正负，来判断使用正向车道还是反向车道的宽度
  const { forwardWidth, reverseWidth } = getRoadWidth({
    road,
    percent,
  })
  // 由于停止线需要放置在道路中央，覆盖所有的车道，需要重新调整 t 坐标
  const newT = isForward ? -forwardWidth / 2 : reverseWidth / 2

  const { targetPoint, refLinePoint, tangent, normal } = getLocationByST({
    s,
    t: newT,
    useElevationTangent: true,
    elevationPath: road.elevationPath,
    curvePath: road.keyPath,
  })

  const frontPoint = getAlongRoadFrontPoint({
    tangent,
    point: targetPoint,
  })
  return {
    length: isForward ? forwardWidth : reverseWidth,
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

// 获取停止线在当前道路上的属性
export function getStopLineAttrOnRoad (params: {
  roadId: string
  point: common.vec3
}) {
  const { roadId, point } = params
  const road = getRoad(roadId)
  if (!road) return null

  // 在目标道路上的坐标数据
  const locationData = getLocationInRoad({
    point,
    roadId,
    useElevation: true,
  })
  if (!locationData) return null
  // 基于位置的百分比，获取道路对应的section
  const { s, percent } = locationData
  const { forwardWidth, reverseWidth } = getRoadWidth({
    road,
    percent,
  })
  // 由于停止线需要放置在道路中央，覆盖所有的车道，需要重新调整 t 坐标
  const isForward = locationData.t <= 0
  const newT = isForward ? -forwardWidth / 2 : reverseWidth / 2
  const { targetPoint, refLinePoint, tangent, normal } = getLocationByST({
    s,
    t: newT,
    useElevationTangent: true,
    elevationPath: road.elevationPath,
    curvePath: road.keyPath,
  })
  const frontPoint = getAlongRoadFrontPoint({
    tangent,
    point: targetPoint,
  })

  return {
    length: isForward ? forwardWidth : reverseWidth,
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

// 创建停止线
export function createStopLine (params: {
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

  const stopLineAttr = getStopLineAttrOnRoad({
    roadId,
    point: intersectionPoint,
  })
  if (!stopLineAttr) return null

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
  } = stopLineAttr

  // 更新路面标识源数据的属性
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
  // 停止线的宽度始终固定
  // 长度跟随当前道路的宽度动态调整
  roadSignData.length = fixedPrecision(length || Constant.stopLineSize.length)
  roadSignData.width = Constant.stopLineSize.width

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}
