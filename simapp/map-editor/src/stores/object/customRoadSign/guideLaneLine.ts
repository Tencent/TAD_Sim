import { CatmullRomCurve3, type Intersection } from 'three'
import { createDefaultObjectData } from '../common'
import { calcLaneAttr, calcRoadSignLocationInRoad } from '../roadSign'
import type { IRoadSignConfig } from '@/config/roadSign'
import { genObjectId } from '@/utils/guid'
import { getObject, getRoad, setObject } from '@/utils/mapCache'
import { Constant, calcSolidBoundaryGeoAttr } from '@/utils/business'
import { useRoadStore } from '@/stores/road'
import {
  CatmullromTension,
  CurveType,
  fixedPrecision,
  getLocationByST,
  transformVec3ByObject,
} from '@/utils/common3d'

/**
 * 获取对应的几何体属性
 * @param params
 */
function getGeoAttrs (params: {
  width: number // 导向车到线的宽度
  leftPoints: Array<common.vec3>
  rightPoints: Array<common.vec3>
}) {
  const { width, leftPoints, rightPoints } = params
  // 基于车道线的路径，获取车道线网格的几何体属性（vertices + indices）
  const offset = 0
  // 暂时将导向车道线划分为 15 段，应该也能够满足渲染效果
  const segment = 15
  // 基于左侧边界线采样点创建路径，按照实线边界线几何体的创建方法获取对应的属性
  const leftPath = new CatmullRomCurve3(transformVec3ByObject(leftPoints))
  leftPath.curveType = CurveType
  if (CurveType === 'catmullrom') {
    leftPath.tension = CatmullromTension
  }
  const { vertices: lVertices, indices: lIndices } = calcSolidBoundaryGeoAttr({
    width,
    keyPath: leftPath,
    offset,
    isSingle: true,
    segment,
  })
  const _lVertices = new Float32Array(lVertices)
  // 右侧
  const rightPath = new CatmullRomCurve3(transformVec3ByObject(rightPoints))
  rightPath.curveType = CurveType
  if (CurveType === 'catmullrom') {
    rightPath.tension = CatmullromTension
  }
  const { vertices: rVertices, indices: rIndices } = calcSolidBoundaryGeoAttr({
    width,
    keyPath: rightPath,
    offset,
    isSingle: true,
    segment,
  })
  const _rVertices = new Float32Array(rVertices)

  // 导向车道线左右的几何体属性
  const geoAttrs: Array<biz.IGeoAttr> = [
    {
      vertices: _lVertices,
      indices: lIndices,
    },
    {
      vertices: _rVertices,
      indices: rIndices,
    },
  ]

  return geoAttrs
}

/**
 * 基于 st 坐标计算导向车道线的位置属性
 */
export function getGuideLaneLineAttrByST (params: {
  roadId: string
  sectionId: string
  laneId: string
  s: number
  length: number // 车道线的长度
}) {
  const { roadId, sectionId, laneId, s, length } = params
  const roadStore = useRoadStore()
  const queryLaneRes = roadStore.getLaneById(roadId, sectionId, laneId)
  if (!queryLaneRes) return null

  const { road } = queryLaneRes
  // 在[0,1]区间内的比例
  const percent = Math.min(1, Math.max(0, s / road.length))

  // 跟车道相关的属性
  const laneRes = calcLaneAttr({
    roadId,
    sectionId,
    laneId,
    length,
    percent,
  })

  if (!laneRes) return null

  const {
    leftPoints,
    rightPoints,
    s: newS,
    t: newT,
    length: newLength,
    width: newWidth,
    percent: newPercent,
  } = laneRes

  //
  const location = getLocationByST({
    s: newS,
    t: newT,
    useElevationTangent: true,
    elevationPath: road.elevationPath,
    curvePath: road.keyPath,
  })
  if (!location) return null

  const geoAttrs = getGeoAttrs({
    width: Constant.guideLaneLineSize.width,
    leftPoints,
    rightPoints,
  })

  // 由于导向车道线的长度或s坐标可能存在更新，位置信息返回 length、percent、s 属性用于覆盖原有的数据
  return {
    geoAttrs,
    length: newLength,
    width: newWidth,
    percent: newPercent,
    s: newS,
    t: newT,
    position: location.targetPoint,
  }
}

/**
 * 基于放置到车道的信息，计算导向车道线在车道两侧的几何体属性
 */
export function getGuideLaneLineAttr (params: {
  roadId: string
  sectionId: string
  laneId: string
  point: common.vec3
  length: number // 车道线的长度
}) {
  const { roadId, sectionId, laneId, point, length } = params

  // 放置在车道上的位置
  const locationData = calcRoadSignLocationInRoad({
    point,
    roadId,
    sectionId,
    laneId,
  })
  if (!locationData) return null
  const { t, percent, closestPointInLane } = locationData

  // 跟车道相关的属性
  const laneRes = calcLaneAttr({
    roadId,
    sectionId,
    laneId,
    length,
    percent,
  })
  if (!laneRes) return null
  const {
    leftPoints,
    rightPoints,
    s: newS,
    length: newLength,
    width: newWidth,
    percent: newPercent,
  } = laneRes

  const geoAttrs = getGeoAttrs({
    width: Constant.guideLaneLineSize.width,
    leftPoints,
    rightPoints,
  })

  // 由于导向车道线的长度或s坐标可能存在更新，位置信息返回 length、percent、s 属性用于覆盖原有的数据
  return {
    geoAttrs,
    length: newLength,
    width: newWidth,
    percent: newPercent,
    s: newS,
    t,
    position: closestPointInLane,
  }
}

/**
 * 创建导向车道线
 * @param params
 */
export function createGuideLaneLine (params: {
  option: IRoadSignConfig
  intersection: Intersection
}) {
  const { option, intersection } = params
  const { name, showName, type, subtype } = option
  const roadSignData: biz.ICustomRoadSign = {
    ...createDefaultObjectData(),
    geoAttrs: [],
  }
  roadSignData.id = genObjectId()
  roadSignData.mainType = 'roadSign'
  roadSignData.name = name
  roadSignData.showName = showName
  roadSignData.type = type
  roadSignData.subtype = subtype

  // 单条导向车道线的长度和宽度（因为成对出现的车道线不好计算宽度，是跟随车道宽度展示的）
  roadSignData.length = Constant.guideLaneLineSize.length
  roadSignData.width = Constant.guideLaneLineSize.width

  // 基于鼠标交互结果，更新物体的【空间属性】
  const { object } = intersection
  // @ts-expect-error
  const { roadId, sectionId, laneId } = object
  // 判断是放置在那条车道上
  if (!roadId || !sectionId || !laneId) return null

  // 在逻辑层计算导向车道线几何体的属性
  const guideLaneLineAttr = getGuideLaneLineAttr({
    roadId,
    sectionId,
    laneId,
    point: intersection.point,
    length: roadSignData.length,
  })
  if (!guideLaneLineAttr) return null
  const {
    geoAttrs,
    length: newLength,
    width: newWidth,
    percent,
    s,
    t,
    position,
  } = guideLaneLineAttr
  roadSignData.position = {
    x: position.x,
    y: position.y,
    z: position.z,
  }
  roadSignData.length = newLength
  roadSignData.width = newWidth
  roadSignData.closestPointPercent = percent
  roadSignData.geoAttrs = geoAttrs
  roadSignData.roadId = roadId
  roadSignData.sectionId = sectionId
  roadSignData.laneId = laneId
  roadSignData.s = s
  roadSignData.t = t

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

/**
 * 移动导向车道线
 * @param params
 */
export function moveGuideLaneLine (params: {
  objectId: string
  point?: common.vec3
  s?: number
  laneId?: string
}) {
  const { objectId, point, s, laneId: newLaneId } = params
  const roadSignData = getObject(objectId) as biz.ICustomRoadSign
  if (!roadSignData) return null

  // 如果更新了绑定的车道 id
  if (newLaneId) {
    roadSignData.laneId = newLaneId
  }
  const { roadId, sectionId, laneId } = roadSignData

  let guideLaneLineAttr
  if (point) {
    // 在逻辑层计算导向车道线几何体的属性
    guideLaneLineAttr = getGuideLaneLineAttr({
      roadId,
      sectionId,
      laneId,
      point,
      length: roadSignData.length,
    })
  } else if (s !== undefined) {
    guideLaneLineAttr = getGuideLaneLineAttrByST({
      roadId,
      sectionId,
      laneId,
      s,
      length: roadSignData.length,
    })
  }
  if (!guideLaneLineAttr) return null
  const {
    geoAttrs,
    length: newLength,
    width: newWidth,
    percent,
    s: newS,
    t: newT,
    position,
  } = guideLaneLineAttr
  roadSignData.position = {
    x: position.x,
    y: position.y,
    z: position.z,
  }
  roadSignData.length = newLength
  roadSignData.width = newWidth
  roadSignData.closestPointPercent = percent
  roadSignData.geoAttrs = geoAttrs
  roadSignData.s = newS
  roadSignData.t = newT

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

/**
 * 更新导向车道线的尺寸（长度）
 */
export function updateGuideLaneLineSize (params: {
  objectId: string
  length: number
}) {
  const { objectId, length } = params
  const roadSignData = getObject(objectId) as biz.ICustomRoadSign
  if (!roadSignData) return null

  const { roadId, sectionId, laneId, s } = roadSignData

  // 基于传入的新的车道线长度，重新计算对应几何体的属性
  const guideLaneLineAttr = getGuideLaneLineAttrByST({
    roadId,
    sectionId,
    laneId,
    s,
    length,
  })
  if (!guideLaneLineAttr) return null
  const {
    geoAttrs,
    length: newLength,
    width: newWidth,
    percent,
    s: newS,
    t: newT,
    position,
  } = guideLaneLineAttr
  roadSignData.position = {
    x: position.x,
    y: position.y,
    z: position.z,
  }
  roadSignData.length = newLength
  roadSignData.width = newWidth
  roadSignData.closestPointPercent = percent
  roadSignData.geoAttrs = geoAttrs
  roadSignData.s = newS
  roadSignData.t = newT

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

// 解析特殊的路面标线
export function parseGuideLaneLine (params: {
  config: IRoadSignConfig
  data: biz.ICommonObject
}) {
  const { config, data: commonData } = params
  const { id: objectId, roadid: roadId, s, t, length } = commonData
  const { name, type, subtype, showName } = config
  const roadSignData: biz.ICustomRoadSign = {
    ...createDefaultObjectData(),
    geoAttrs: [],
  }

  roadSignData.id = objectId
  roadSignData.mainType = 'roadSign'
  roadSignData.name = name
  roadSignData.showName = showName
  roadSignData.type = type
  roadSignData.subtype = subtype
  const _s = fixedPrecision(Number(s))
  const _t = fixedPrecision(Number(t))
  const absT = Math.abs(_t)

  // 基于 st 坐标，判断当前导向车道线关联的 section 和车道
  const road = getRoad(roadId)
  if (!road) return null
  // 当前 s 坐标在整条道路的比例
  const percent = _s / road.length
  let targetSection
  let targetLane
  let tOffset = 0
  for (const section of road.sections) {
    if (targetSection) continue
    const { pStart, pEnd } = section
    if (pStart <= percent && pEnd >= percent) {
      targetSection = section
    }
  }
  if (!targetSection) return null
  for (const lane of targetSection.lanes) {
    if (targetLane) continue
    const { normalWidth } = lane
    // 如果 t 坐标在车道宽度范围区间内，则为目标车道
    if (absT >= tOffset && absT <= tOffset + normalWidth) {
      targetLane = lane
      continue
    }
    // 否则 t 值的偏移量递增
    tOffset += normalWidth
  }
  if (!targetLane) return null
  const { id: sectionId } = targetSection
  const { id: laneId } = targetLane

  // 获取导向车道线的几何体相关属性
  const guideLaneLineAttr = getGuideLaneLineAttrByST({
    roadId,
    sectionId,
    laneId,
    s: _s,
    length: fixedPrecision(Number(length)),
  })
  if (!guideLaneLineAttr) return null

  const {
    geoAttrs,
    length: newLength,
    width: newWidth,
    percent: newPercent,
    s: newS,
    t: newT,
    position,
  } = guideLaneLineAttr
  roadSignData.position = {
    x: position.x,
    y: position.y,
    z: position.z,
  }
  roadSignData.length = newLength
  roadSignData.width = newWidth
  roadSignData.closestPointPercent = newPercent
  roadSignData.geoAttrs = geoAttrs
  roadSignData.roadId = roadId
  roadSignData.sectionId = sectionId
  roadSignData.laneId = laneId
  roadSignData.s = newS
  roadSignData.t = newT

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}
