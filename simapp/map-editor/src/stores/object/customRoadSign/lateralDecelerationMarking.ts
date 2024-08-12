// 横向减速标线
import { CatmullRomCurve3, type Intersection, Vector3 } from 'three'
import { createDefaultObjectData } from '../common'
import { calcLaneAttr, calcRoadSignLocationInRoad } from '../roadSign'
import { getPointOutOfScope } from './index'
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

// 超过 176m 的长度，差值需要是 32 的整倍数
const maxFixedLength = 176
const maxStep = 32
// 横向减速标线的长度，只能为固定值
const fixedLengths = [
  Constant.lateralDecelerationSize.length.min, // 只有一段线没有间隔，仅为线宽
  17,
  37, // 间隔 20m
  60, // 间隔 23m
  86, // 间隔 26m
  114, // 间隔 28m
  144, // 间隔 30m
  maxFixedLength, // 间隔 32m
]
// 步长间隔
const stepRules = [0, 17, 20, 23, 26, 28, 30, 32]

/**
 * 由于横向减速标线长度只能是固定值，需要对于自定义的长度做解析得到目标长度
 * @param length
 */
export function parseLateralDecelerationLength (length: number) {
  if (length >= maxFixedLength) {
    const delta = length - maxFixedLength
    // 向下取整
    const count = Math.floor(delta / maxStep)
    return maxFixedLength + maxStep * count
  } else {
    for (let i = 0; i < fixedLengths.length - 1; i++) {
      const currentLength = fixedLengths[i]
      const nextLength = fixedLengths[i + 1]
      if (length >= currentLength && length < nextLength) {
        return currentLength
      }
    }
    // 兜底返回一个最小值
    return fixedLengths[0]
  }
}

/**
 * 根据列数和长度来计算每一个顶点的相对偏移量
 * @param params
 */
function calcRelativeOffset (params: { column: 2 | 3, length: number }) {
  const { column, length: l } = params

  const offsets = []
  const span = 0.05 // 白线之间的间隔 5cm
  const w = 0.45 // 45cm 的线宽
  let sOffset = -1.5 * w
  if (column === 3) {
    sOffset = -2.5 * w
  }
  for (let i = 0; i < column; i++) {
    // 由三条中间存在接缝的长条组成
    const s1 = sOffset + i * w * 2
    const s2 = sOffset + w + i * w * 2
    offsets.push(
      // part1
      { s: s1, t: 0 },
      { s: s2, t: 0 },
      { s: s2, t: l },
      { s: s1, t: l },
      // part2
      { s: s1, t: l + span },
      { s: s2, t: l + span },
      { s: s2, t: l * 2 + span },
      { s: s1, t: l * 2 + span },
      // part3
      { s: s1, t: l * 2 + span * 2 },
      { s: s2, t: l * 2 + span * 2 },
      { s: s2, t: l * 3 + span * 2 },
      { s: s1, t: l * 3 + span * 2 },
    )
  }
  return offsets
}

/**
 * 计算横向减速标线的多段白线几何体顶点属性
 * @param params
 */
function calcMultiplyLineGeoAttrs (params: {
  curve: biz.ICurve3
  length: number // 横向减速标线的总长度
  width: number // 车道的宽度
  isForward: boolean
}) {
  const { curve, length: totalLength, width: laneWidth, isForward } = params

  // 顶点和三角面索引
  const vertices = []
  const indices = []

  // 三角面顶点索引
  let index = 0
  let sOffset = 0

  // 步长的索引
  let stepIndex = 0
  const span = 0.05 // 白线之间的间隔
  const basicTOffset = 0.15
  const lineLength = fixedPrecision(
    (laneWidth - basicTOffset * 2 - span * 2) / 3,
  )
  const twoLineRelativeOffsets = calcRelativeOffset({
    column: 2,
    length: lineLength,
  })

  const threeLineRelativeOffsets = calcRelativeOffset({
    column: 3,
    length: lineLength,
  })
  let relativeOffsets = twoLineRelativeOffsets

  let targetPoint = new Vector3()
  while (sOffset <= totalLength) {
    for (const _offset of relativeOffsets) {
      const { s, t } = _offset
      const percent = (s + sOffset) / totalLength
      const tOffset = isForward ? basicTOffset + t : -(basicTOffset + t)
      if (percent >= 0 && percent <= 1) {
        const refPoint = curve.getPointAt(percent)
        const tangent = curve.getTangentAt(percent)
        const vertical = getVerticalVector(tangent)
        targetPoint = refPoint.clone()
        targetPoint.addScaledVector(vertical, tOffset)
      } else {
        // 范围外
        targetPoint = getPointOutOfScope({
          curve,
          percent,
          offset: tOffset,
        })
      }
      vertices.push(targetPoint.x, targetPoint.y, targetPoint.z)
    }
    // 每四个点为一组，基于段数来调节三角面的顶点索引
    const count = relativeOffsets.length / 4
    for (let i = 0; i < count; i++) {
      const a = index + i * 4
      const b = index + i * 4 + 1
      const c = index + i * 4 + 2
      const d = index + i * 4 + 3
      indices.push(a, c, b)
      indices.push(c, a, d)
    }

    index += relativeOffsets.length
    // 步长随着距离递增
    if (stepIndex < stepRules.length - 1) {
      stepIndex++
      if (stepIndex >= stepRules.length - 2) {
        // 更换为三列
        relativeOffsets = threeLineRelativeOffsets
      }
    }

    sOffset += stepRules[stepIndex]
  }

  return {
    vertices,
    indices,
  }
}

/**
 * 获取横向减速标线的几何体属性
 * @param params
 */
function getGeoAttrs (params: {
  leftPoints: Array<common.vec3>
  rightPoints: Array<common.vec3>
  length: number
  width: number
  isForward: boolean
}) {
  const { leftPoints, rightPoints, length, width, isForward } = params
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

  const { vertices, indices } = calcMultiplyLineGeoAttrs({
    curve: leftPath,
    length,
    width,
    isForward,
  })

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

  // 横向减速标线的几何体属性
  const geoAttrs: Array<biz.IGeoAttr> = [
    {
      vertices,
      indices,
    },
  ]
  return geoAttrs
}

/**
 * 基于 st 坐标计算横向减速标线的位置属性
 * @param params
 */
export function getLateralDecelerationAttrByST (params: {
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

  // 横向减速标线的长度只能为固定值
  const _fixedLength = parseLateralDecelerationLength(length)

  // 跟车道相关的属性
  const laneRes = calcLaneAttr({
    roadId,
    sectionId,
    laneId,
    length: _fixedLength,
    percent,
  })

  if (!laneRes) return null

  const {
    leftPoints,
    rightPoints,
    s: newS,
    t: newT,
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
    length: _fixedLength,
    width: newWidth,
    isForward: Number(laneId) <= 0,
  })

  return {
    geoAttrs,
    length: _fixedLength,
    width: newWidth,
    percent: newPercent,
    s: newS,
    t: newT,
    position: location.targetPoint,
  }
}

/**
 * 基于放置车道的信息，计算横向减速标记在车道两侧的几何体属性
 * @param params
 */
export function getLateralDecelerationAttr (params: {
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

  // 横向减速标线的长度只能为固定值
  const _fixedLength = parseLateralDecelerationLength(length)

  // 计算跟车道相关的属性
  const laneRes = calcLaneAttr({
    roadId,
    sectionId,
    laneId,
    length: _fixedLength,
    percent,
  })
  if (!laneRes) return null
  const {
    leftPoints,
    rightPoints,
    s: newS,
    width: newWidth,
    percent: newPercent,
  } = laneRes

  const geoAttrs = getGeoAttrs({
    leftPoints,
    rightPoints,
    length: _fixedLength,
    width: newWidth,
    isForward: Number(laneId) <= 0,
  })

  // 由于导向车道线的长度或s坐标可能存在更新，位置信息返回 length、percent、s 属性用于覆盖原有的数据
  return {
    geoAttrs,
    length: _fixedLength,
    width: newWidth,
    percent: newPercent,
    s: newS,
    t,
    position: closestPointInLane,
  }
}

/**
 * 创建横向减速标记的数据结构
 * @param params
 */
export function createLateralDeceleration (params: {
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

  roadSignData.length = Constant.lateralDecelerationSize.length.default
  // 横向减速标记，成对出现。宽度应该为两条线之间的距离
  // 先默认赋值为 0
  roadSignData.width = 0

  // 基于鼠标交互结果，更新物体的空间属性
  const { object } = intersection
  // @ts-expect-error
  const { roadId, sectionId, laneId } = object
  if (!roadId || !sectionId || !laneId) return null

  // 在逻辑层计算导向车道线几何体的属性
  const guideLaneLineAttr = getLateralDecelerationAttr({
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
 * 移动横向减速标线
 * @param params
 */
export function moveLateralDeceleration (params: {
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
    attr = getLateralDecelerationAttr({
      roadId,
      sectionId,
      laneId,
      point,
      length: roadSignData.length,
    })
  } else if (s !== undefined) {
    attr = getLateralDecelerationAttrByST({
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
 * 更新横向减速标线的尺寸（长度）
 */
export function updateLateralDecelerationSize (params: {
  objectId: string
  length: number
}) {
  const { objectId, length } = params
  const roadSignData = getObject(objectId) as biz.ICustomRoadSign
  if (!roadSignData) return null

  const { roadId, sectionId, laneId, s } = roadSignData

  // 基于传入的新的车道线长度，重新计算对应几何体的属性
  const attr = getLateralDecelerationAttrByST({
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
export function parseLateralDeceleration (params: {
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
  const attr = getLateralDecelerationAttrByST({
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
