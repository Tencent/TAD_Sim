// 纵向减速标线
import { CatmullRomCurve3, type Intersection } from 'three'
import { createDefaultObjectData } from '../common'
import { calcLaneAttr, calcRoadSignLocationInRoad } from '../roadSign'
import type { IRoadSignConfig } from '@/config/roadSign'
import { genObjectId } from '@/utils/guid'
import { Constant } from '@/utils/business'
import { getObject, getRoad, setObject } from '@/utils/mapCache'
import {
  CatmullromTension,
  CurveType,
  fixedPrecision,
  getLocationByST,
  getVerticalVector,
  transformVec3ByObject,
} from '@/utils/common3d'
import { useRoadStore } from '@/stores/road'

/**
 * 计算纵向减速标记的菱形几何体属性
 * @param params
 */
function calcDiamondGeoAttrs (params: {
  curve: biz.ICurve3
  isLeft: boolean
  isForward: boolean
}) {
  const { curve, isLeft, isForward } = params

  // 顶点和三角面索引数组
  const vertices = []
  const indices = []
  // 三角面顶点索引
  let index = 0
  // 每一组第一个顶点s坐标偏移量
  let sOffset = 0
  // 整体的t方向偏移量
  const basicTOffset = 0.15
  // 路径的总长度
  const roadLength = curve.getLength()
  // 基本的步长
  // 前30米，是宽度从10cm到30cm由窄变宽的过度阶段
  // 30米后，是固定30cm宽度的稳定阶段
  const targetLength = 30
  const minStep = 0.1
  const maxStep = 0.3
  let step = minStep
  /**
   * 菱形顶点顺序及三角面组装简化图
   *     /|4
   *    / |
   *  3/  |
   *  |\  |
   *  | \ |
   *  |  \|
   *  |  /2
   *  | /
   * 1|/
   */
  // 每一组菱形的四个顶点相对间隔偏移量的系数
  const relativeOffsetRatios = [
    { s: 0, t: 0 },
    { s: 1, t: 1 },
    { s: 5, t: 0 },
    { s: 6, t: 1 },
  ]
  // 相邻菱形的间隔
  let stepOffset = step * 10
  // 是否达到30m的稳定宽度状态
  let isStable = false
  while (sOffset + stepOffset <= roadLength) {
    for (const offsetRatio of relativeOffsetRatios) {
      const { s: sRatio, t: tRatio } = offsetRatio
      const _s = sRatio * step
      const _t = tRatio * step
      const percent = (_s + sOffset) / roadLength
      const refPoint = curve.getPointAt(percent)
      const tangent = curve.getTangentAt(percent)
      const vertical = getVerticalVector(tangent)
      const targetPoint = refPoint.clone()
      // 基于每一个顶点的偏移量，计算路径延伸出去的顶点做个表
      if (isLeft) {
        targetPoint.addScaledVector(
          vertical,
          isForward ? basicTOffset + _t : -(basicTOffset + _t),
        )
      } else {
        targetPoint.addScaledVector(
          vertical,
          isForward ? -(basicTOffset + _t) : basicTOffset + _t,
        )
      }
      vertices.push(targetPoint.x, targetPoint.y, targetPoint.z)
    }
    // 4个顶点的索引
    const a = index
    const b = index + 1
    const c = index + 2
    const d = index + 3

    if (isLeft) {
      indices.push(a, b, c)
      indices.push(d, c, b)
    } else {
      indices.push(a, c, b)
      indices.push(d, b, c)
    }

    if (sOffset + step * 6 < targetLength) {
      const _percent = Math.max(0, Math.min(sOffset / targetLength, 1))
      step = minStep + (maxStep - minStep) * _percent
      stepOffset = step * 10
    } else {
      isStable = true
    }

    if (isStable) {
      // 如果是达到30m的距离，则后续菱形宽度稳定为30cm
      step = maxStep
      stepOffset = step * 10
    }

    // 下一组菱形顶点，如果在30m之后，是固定宽度和步长
    sOffset += stepOffset
    index += 4
  }
  return {
    vertices,
    indices,
  }
}

/**
 * 获取纵向减速标线的几何体属性
 * @param params
 */
function getGeoAttrs (params: {
  leftPoints: Array<common.vec3>
  rightPoints: Array<common.vec3>
  isForward: boolean
}) {
  const { leftPoints, rightPoints, isForward } = params
  // 基于左侧边界线采样点创建路径，按照实线边界线几何体的创建方法获取对应的属性
  const _leftPoints = transformVec3ByObject(leftPoints)
  if (!isForward) {
    _leftPoints.reverse()
  }
  const leftPath = new CatmullRomCurve3(_leftPoints)
  leftPath.curveType = CurveType
  if (CurveType === 'catmullrom') {
    leftPath.tension = CatmullromTension
  }

  const { vertices: lVertices, indices: lIndices } = calcDiamondGeoAttrs({
    curve: leftPath,
    isLeft: true,
    isForward,
  })
  const _lVertices = new Float32Array(lVertices)

  // 右侧
  const _rightPoints = transformVec3ByObject(rightPoints)
  if (!isForward) {
    _rightPoints.reverse()
  }
  const rightPath = new CatmullRomCurve3(_rightPoints)
  rightPath.curveType = CurveType
  if (CurveType === 'catmullrom') {
    rightPath.tension = CatmullromTension
  }

  const { vertices: rVertices, indices: rIndices } = calcDiamondGeoAttrs({
    curve: rightPath,
    isLeft: false,
    isForward,
  })
  const _rVertices = new Float32Array(rVertices)

  // 左右纵向减速标线的几何体属性
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
 * 基于 st 坐标计算纵向减速标线的位置属性
 * @param params
 */
export function getLongitudinalDecelerationAttrByST (params: {
  roadId: string
  sectionId: string
  laneId: string
  s: number
  length: number
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
    leftPoints,
    rightPoints,
    isForward: Number(laneId) <= 0,
  })

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
 * 基于放置车道的信息，计算纵向减速标记在车道两侧的几何体属性
 * @param params
 */
export function getLongitudinalDecelerationAttr (params: {
  roadId: string
  sectionId: string
  laneId: string
  point: common.vec3
  length: number
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

  // 计算跟车道相关的属性
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
    leftPoints,
    rightPoints,
    isForward: Number(laneId) <= 0,
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
 * 创建纵向减速标记的数据结构
 * @param params
 */
export function createLongitudinalDeceleration (params: {
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

  roadSignData.length = Constant.longitudinalDecelerationSize.length.default
  // 纵向减速标记，成对出现。宽度应该为两条线之间的距离
  // 先默认赋值为 0
  roadSignData.width = 0

  // 基于鼠标交互结果，更新物体的空间属性
  const { object } = intersection
  // @ts-expect-error
  const { roadId, sectionId, laneId } = object
  if (!roadId || !sectionId || !laneId) return null

  // 在逻辑层计算导向车道线几何体的属性
  const guideLaneLineAttr = getLongitudinalDecelerationAttr({
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
 * 移动纵向减速标线
 * @param params
 */
export function moveLongitudinalDeceleration (params: {
  objectId: string
  point?: common.vec3
  s?: number
  laneId?: string
}) {
  const { objectId, point, s, laneId: newLaneId } = params
  const roadSignData = getObject(objectId) as biz.ICustomRoadSign
  if (!roadSignData) return null

  if (newLaneId) {
    roadSignData.laneId = newLaneId
  }
  const { roadId, sectionId, laneId } = roadSignData

  let attr
  if (point) {
    // 在逻辑层计算导向车道线几何体的属性
    attr = getLongitudinalDecelerationAttr({
      roadId,
      sectionId,
      laneId,
      point,
      length: roadSignData.length,
    })
  } else if (s !== undefined) {
    attr = getLongitudinalDecelerationAttrByST({
      roadId,
      sectionId,
      laneId,
      s,
      length: roadSignData.length,
    })
  }
  if (!attr) return null
  const {
    geoAttrs,
    length: newLength,
    width: newWidth,
    percent,
    s: newS,
    t: newT,
    position,
  } = attr
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
 * 更新纵向减速标线的尺寸（长度）
 */
export function updateLongitudinalDecelerationSize (params: {
  objectId: string
  length: number
}) {
  const { objectId, length } = params
  const roadSignData = getObject(objectId) as biz.ICustomRoadSign
  if (!roadSignData) return null

  const { roadId, sectionId, laneId, s } = roadSignData

  // 基于传入的新的车道线长度，重新计算对应几何体的属性
  const attr = getLongitudinalDecelerationAttrByST({
    roadId,
    sectionId,
    laneId,
    s,
    length,
  })
  if (!attr) return null
  const {
    geoAttrs,
    length: newLength,
    width: newWidth,
    percent,
    s: newS,
    t: newT,
    position,
  } = attr
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
export function parseLongitudinalDeceleration (params: {
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

  // 基于 st 坐标，判断当前标线关联的 section 和车道
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

  // 获取几何体相关属性
  const attr = getLongitudinalDecelerationAttrByST({
    roadId,
    sectionId,
    laneId,
    s: _s,
    length: fixedPrecision(Number(length)),
  })
  if (!attr) return null

  const {
    geoAttrs,
    length: newLength,
    width: newWidth,
    percent: newPercent,
    s: newS,
    t: newT,
    position,
  } = attr
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
