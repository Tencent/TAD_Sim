import { CatmullRomCurve3, Group, Vector3 } from 'three'
import { fixedPrecision, transformVec3ByObject } from '../../utils/common'
import {
  CurveType,
  getPointOutOfScope,
  getVerticalVector,
  renderCustomRoadSignByGeoAttrs,
} from './index'

// 超过 176m 的长度，差值需要是 32 的整倍数
const maxFixedLength = 176
const maxStep = 32
// 横向减速标线的长度，只能为固定值
const fixedLengths = [
  1.35, // 只有一段线没有间隔，仅为线宽
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
 * 根据列数和长度来计算每一个顶点的相对偏移量
 * @param params
 */
function calcRelativeOffset (params) {
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

export function createLateralDecelerationMarking (params) {
  const { data, parent, ignoreAlt = false } = params
  const { id, geomdata, length } = data

  if ((geomdata?.points?.length || 0) < 2) return
  const originLeftPoints = geomdata.points[0].point
  const originRightPoints = geomdata.points[1].point

  // 忽略高度
  if (ignoreAlt) {
    originLeftPoints.forEach((p) => {
      p.z = 0
    })
    originRightPoints.forEach((p) => {
      p.z = 0
    })
  }

  const objectId = String(id)
  // 使用接口透传的长度
  const _length = fixedPrecision(Number(length))

  // 左右边界线的采样点数组
  const leftPoints = transformVec3ByObject(originLeftPoints)
  const rightPoints = transformVec3ByObject(originRightPoints)

  // 基于左右边界的采样点数组，计算对应的导向车道线的几何体属性
  const geoAttrs = getGeoAttrs({
    leftPoints,
    rightPoints,
    length: _length,
  })

  const group = new Group()
  group.name = 'customRoadSignGroup'
  group.objectId = objectId

  renderCustomRoadSignByGeoAttrs({
    geoAttrs,
    group,
    objectId,
  })

  parent.add(group)
}

/**
 * 计算横向减速标线的多段白线几何体顶点属性
 * @param params
 */
function calcMultiplyLineGeoAttrs (params) {
  const { curve, length: totalLength, width: laneWidth } = params

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
      const tOffset = basicTOffset + t
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

// 计算横向减速标线的几何体属性
function getGeoAttrs (params) {
  const { leftPoints, rightPoints, length: totalLength } = params

  // 左侧边界形成的几何体
  const leftPath = new CatmullRomCurve3(transformVec3ByObject(leftPoints))
  leftPath.curveType = CurveType
  // 右侧边界形成的几何体
  const rightPath = new CatmullRomCurve3(transformVec3ByObject(rightPoints))
  rightPath.curveType = CurveType

  // 左右边界线的采样点数量不一定相同，先转化成路径后再采样计算大概的宽度
  let width = 0
  let isHead = true

  // 先取路径的头部采样点和尾部采样点的间距做对比，取大处的
  const leftHeadPoint = leftPath.getPointAt(0)
  const leftTailPoint = leftPath.getPointAt(1)

  const rightHeadPoint = rightPath.getPointAt(0)
  const rightTailPoint = rightPath.getPointAt(1)

  const headDist = leftHeadPoint.distanceTo(rightHeadPoint)
  const tailDist = leftTailPoint.distanceTo(rightTailPoint)

  if (headDist < tailDist) {
    isHead = false
  }

  // 取头部侧或尾部层的几组采样点计算平均宽度
  if (isHead) {
    const pl1 = leftPath.getPointAt(0.15)
    const pr1 = rightPath.getPointAt(0.15)
    const pl2 = leftPath.getPointAt(0.3)
    const pr2 = rightPath.getPointAt(0.3)
    const dist1 = pl1.distanceTo(pr1)
    const dist2 = pl2.distanceTo(pr2)
    width = fixedPrecision((headDist + dist1 + dist2) / 3)
  } else {
    const pl1 = leftPath.getPointAt(0.85)
    const pr1 = rightPath.getPointAt(0.85)
    const pl2 = leftPath.getPointAt(0.7)
    const pr2 = rightPath.getPointAt(0.7)
    const dist1 = pl1.distanceTo(pr1)
    const dist2 = pl2.distanceTo(pr2)
    width = fixedPrecision((tailDist + dist1 + dist2) / 3)
  }

  const { vertices, indices } = calcMultiplyLineGeoAttrs({
    curve: leftPath,
    length: totalLength,
    width,
  })
  const _vertices = new Float32Array(vertices)

  const geoAttrs = [
    {
      vertices: _vertices,
      indices,
    },
  ]
  return geoAttrs
}
