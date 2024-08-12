import { CatmullRomCurve3, Group } from 'three'
import { transformVec3ByObject } from '../../utils/common'
import {
  CurveType,
  getVerticalVector,
  renderCustomRoadSignByGeoAttrs,
} from './index'

export function createLonDecelerationMarking (params) {
  const { data, parent, ignoreAlt = false } = params
  const { id, geomdata } = data

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

  // 左右边界线的采样点数组
  const leftPoints = transformVec3ByObject(originLeftPoints)
  const rightPoints = transformVec3ByObject(originRightPoints)

  // 基于左右边界的采样点数组，计算对应的减速标线的几何体
  const geoAttrs = getGeoAttrs({
    leftPoints,
    rightPoints,
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
 * 计算纵向减速标记的菱形几何体属性
 * @param params
 */
function calcDiamondGeoAttrs (params) {
  const { curve, isLeft } = params

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
        targetPoint.addScaledVector(vertical, basicTOffset + _t)
      } else {
        targetPoint.addScaledVector(vertical, -(basicTOffset + _t))
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

// 计算纵向减速标线的几何体属性
function getGeoAttrs (params) {
  const { leftPoints, rightPoints } = params

  // 左侧边界形成的几何体
  const leftPath = new CatmullRomCurve3(transformVec3ByObject(leftPoints))
  leftPath.curveType = CurveType

  const { vertices: lVertices, indices: lIndices } = calcDiamondGeoAttrs({
    curve: leftPath,
    isLeft: true,
  })
  const _lVertices = new Float32Array(lVertices)

  // 右侧边界形成的几何体
  const rightPath = new CatmullRomCurve3(transformVec3ByObject(rightPoints))
  rightPath.curveType = CurveType

  const { vertices: rVertices, indices: rIndices } = calcDiamondGeoAttrs({
    curve: rightPath,
    isLeft: false,
  })
  const _rVertices = new Float32Array(rVertices)

  // 左右纵向减速标线的几何体属性
  const geoAttrs = [
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
