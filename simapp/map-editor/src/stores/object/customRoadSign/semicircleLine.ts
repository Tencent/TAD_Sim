// 白色半圆状车距确认线
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
  PI,
  fixedPrecision,
  getLocationByST,
  getVerticalVector,
  transformVec3ByObject,
} from '@/utils/common3d'
import { useRoadStore } from '@/stores/road'

// 每 50 米设置一个半圆状确认线
const step = 50
// 半径 30cm
const radius = 0.3
const relativeOffsets = []
function initRelativeOffsets (params: { radius: number, count: number }) {
  const { radius, count } = params
  // 先清空
  relativeOffsets.length = 0

  // 坐标系的原点，也作为一个顶点
  relativeOffsets.push({
    s: 0,
    t: 0,
  })
  // 遍历导入半径上的点
  for (let i = 0; i <= count; i++) {
    const angle = (PI / count) * i
    relativeOffsets.push({
      s: radius * Math.cos(angle),
      t: radius * Math.sin(angle),
    })
  }
}
initRelativeOffsets({
  radius,
  count: 10, // 一个半圆均分为三角面的个数
})

/**
 * 由于半圆状确认线长度只能是固定值，需要对于自定义的长度做解析得到目标长度
 * @param length
 */
export function parseSemicircleLineLength (length: number) {
  // 只能是 50 的整倍数
  if (length >= step) {
    const delta = length - step
    // 向下取整
    const count = Math.floor(delta / step)
    return step * (count + 1)
  } else {
    // 如果长度小于 50，说明只重复了一段，取半圆状线的直径
    return radius * 2
  }
}

/**
 * 计算半圆状线的几何体属性
 * @param params
 */
function calcSemicircleGeoAttrs (params: {
  curve: biz.ICurve3
  isLeft: boolean
  length: number
  isForward: boolean
}) {
  const { curve, isLeft, length: totalLength, isForward } = params

  // 顶点和三角面索引
  const vertices = []
  const indices = []
  // 三角面顶点索引
  let index = 0
  let sOffset = 0
  const basicTOffset = 0.15
  let targetPoint = new Vector3()
  while (sOffset <= totalLength) {
    for (const _offset of relativeOffsets) {
      const { s, t } = _offset
      const percent = (s + sOffset) / totalLength
      let tOffset = isLeft ? -(basicTOffset + t) : basicTOffset + t
      if (!isForward) {
        tOffset = -tOffset
      }
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

    // 三角面顶点索引（始终跟半圆中间形成三角面）
    for (let i = 1; i < relativeOffsets.length - 1; i++) {
      indices.push(index, index + i, index + i + 1)
    }

    index += relativeOffsets.length
    // 步长递增
    sOffset += step
  }

  return {
    vertices,
    indices,
  }
}

/**
 * 获取半圆状确认线的几何体属性
 * @param params
 */
function getGeoAttrs (params: {
  leftPoints: Array<common.vec3>
  rightPoints: Array<common.vec3>
  length: number
  isForward: boolean
}) {
  const { leftPoints, rightPoints, length, isForward } = params
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

  const { vertices: lVertices, indices: lIndices } = calcSemicircleGeoAttrs({
    curve: leftPath,
    isLeft: true,
    length,
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

  const { vertices: rVertices, indices: rIndices } = calcSemicircleGeoAttrs({
    curve: rightPath,
    isLeft: false,
    length,
    isForward,
  })
  const _rVertices = new Float32Array(rVertices)

  // 半圆状确认线的几何体属性
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
 * 基于 st 坐标计算半圆状确认线的位置属性
 * @param params
 */
export function getSemicircleLineAttrByST (params: {
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

  // 半圆状确认线的长度只能为固定值
  const _fixedLength = parseSemicircleLineLength(length)

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
 * 基于放置车道的信息，计算半圆状确认线在车道两侧的几何体属性
 * @param params
 */
export function getSemicircleLineAttr (params: {
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

  // 半圆状确认线的长度只能为固定值
  const _fixedLength = parseSemicircleLineLength(length)

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
 * 创建半圆状确认线的数据结构
 * @param params
 */
export function createSemicircleLine (params: {
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

  roadSignData.length = Constant.semicircleLineSize.length.default
  // 先默认赋值为 0
  roadSignData.width = 0

  // 基于鼠标交互结果，更新物体的空间属性
  const { object } = intersection
  // @ts-expect-error
  const { roadId, sectionId, laneId } = object
  if (!roadId || !sectionId || !laneId) return null

  // 在逻辑层计算导向车道线几何体的属性
  const guideLaneLineAttr = getSemicircleLineAttr({
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
 * 移动半圆状确认线
 * @param params
 */
export function moveSemicircleLine (params: {
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
    attr = getSemicircleLineAttr({
      roadId,
      sectionId,
      laneId,
      point,
      length: roadSignData.length,
    })
  } else if (s !== undefined) {
    attr = getSemicircleLineAttrByST({
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
 * 更新半圆状确认线的尺寸（长度）
 */
export function updateSemicircleLineSize (params: {
  objectId: string
  length: number
}) {
  const { objectId, length } = params
  const roadSignData = getObject(objectId) as biz.ICustomRoadSign
  if (!roadSignData) return null

  const { roadId, sectionId, laneId, s } = roadSignData

  // 基于传入的新的车道线长度，重新计算对应几何体的属性
  const attr = getSemicircleLineAttrByST({
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
export function parseSemicircleLine (params: {
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
  const attr = getSemicircleLineAttrByST({
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
