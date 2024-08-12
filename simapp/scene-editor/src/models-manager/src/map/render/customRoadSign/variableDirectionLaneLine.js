import { CatmullRomCurve3, Group } from 'three'
import { fixedPrecision, transformVec3ByObject } from '../../utils/common'
import {
  CurveType,
  calcSolidBoundaryGeoAttr,
  getVerticalVector,
  renderCustomRoadSignByGeoAttrs,
} from './index'

const guideLaneLineWidth = 0.2

export function createVariableDirectionLaneLine (params) {
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

  // 基于左右边界的采样点数组，计算对应的导向车道线的几何体属性
  const geoAttrs = getGeoAttrs({
    leftPoints,
    rightPoints,
    width: guideLaneLineWidth,
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
 * 计算可变导向车道线的锯齿几何体
 */
function calcSawtoothGeoAttrs (params) {
  const { curve, isLeft, startVertexIndex = 0 } = params
  // 如果是左侧边界，需要计算靠右的锯齿；右侧边界，计算靠左的锯齿
  // 锯齿的整体偏移量
  const basicOffset = guideLaneLineWidth / 2
  // 路径的总长度
  const roadLength = curve.getLength()
  // 每一个锯齿的步长（白色实体线宽 * 1.414）
  const step = fixedPrecision(guideLaneLineWidth * 1.414)
  // 锯齿斜边的步长（步长 * 1.5）
  const sideStep = fixedPrecision(step * 1.5)
  // 锯齿的第一个坐标点 s 偏移量
  let sOffset = step
  // 三角面顶点索引
  let index = startVertexIndex

  // 顶点和三角面索引数组
  const vertices = []
  const indices = []
  /**
   * 锯齿顶点顺序以及三角面组装的简化图
   * 1————2
   *  \   | \
   *   \  |  \4
   *    \ |  /
   *     \| /
   *     3\/
   */
  // 每一组锯齿的四个顶点相对偏移量
  const relativeOffsets = [
    { s: 0, t: 0 },
    { s: step, t: 0 },
    { s: sideStep, t: sideStep },
    { s: step * 2, t: step },
  ]
  // 相邻锯齿的间隔
  const stepOffset = step * 6
  while (sOffset + stepOffset <= roadLength) {
    for (const _offset of relativeOffsets) {
      const { s, t } = _offset
      const _percent = (s + sOffset) / roadLength
      const refPoint = curve.getPointAt(_percent)
      const tangent = curve.getTangentAt(_percent)
      const vertical = getVerticalVector(tangent)
      const targetPoint = refPoint.clone()
      // 基于每一个锯齿顶点的偏移量，计算路径延伸出去的顶点坐标
      if (isLeft) {
        targetPoint.addScaledVector(vertical, basicOffset + t)
      } else {
        targetPoint.addScaledVector(vertical, -(basicOffset + t))
      }
      vertices.push(targetPoint.x, targetPoint.y, targetPoint.z)
    }
    // 一组锯齿4个顶点的索引
    const a = index
    const b = index + 1
    const c = index + 2
    const d = index + 3

    if (isLeft) {
      indices.push(a, c, b)
      indices.push(c, d, b)
    } else {
      indices.push(a, b, c)
      indices.push(c, b, d)
    }

    // 下一个锯齿的间隔
    sOffset += stepOffset
    index += 4
  }

  return {
    vertices,
    indices,
  }
}

// 计算可变导向车道线的几何体属性
function getGeoAttrs (params) {
  const { leftPoints, rightPoints, width } = params
  // 暂时将导向车道线划分为 15 段，应该也能够满足渲染效果
  const segment = 15
  // 基于车道线的路径，获取车道线网格的几何体属性（vertices + indices）
  // 左侧边界形成的几何体
  const leftPath = new CatmullRomCurve3(transformVec3ByObject(leftPoints))
  leftPath.curveType = CurveType

  const { vertices: lVertices, indices: lIndices } = calcSolidBoundaryGeoAttr({
    keyPath: leftPath,
    width,
    segment,
  })

  // 计算可变导向车道线的锯齿几何体
  const {
    vertices: lToothVertices,
    indices: lToothIndices,
  } = calcSawtoothGeoAttrs({
    curve: leftPath,
    isLeft: true,
    startVertexIndex: lVertices.length / 3,
  })

  // 左边界，组装原本边界的顶点和锯齿的顶点
  const _lVertices = new Float32Array([...lVertices, ...lToothVertices])
  const _lIndices = [...lIndices, ...lToothIndices]

  // 右侧边界形成的几何体
  const rightPath = new CatmullRomCurve3(transformVec3ByObject(rightPoints))
  rightPath.curveType = CurveType

  const { vertices: rVertices, indices: rIndices } = calcSolidBoundaryGeoAttr({
    keyPath: rightPath,
    width,
    segment,
  })

  // 计算可变导向车道线的锯齿几何体
  const {
    vertices: rToothVertices,
    indices: rToothIndices,
  } = calcSawtoothGeoAttrs({
    curve: rightPath,
    isLeft: false,
    startVertexIndex: rVertices.length / 3,
  })

  // 右边界，组装原本边界的顶点和锯齿的顶点
  const _rVertices = new Float32Array([...rVertices, ...rToothVertices])
  const _rIndices = [...rIndices, ...rToothIndices]

  //  组装所有的几何体属性
  const geoAttrs = [
    {
      vertices: _lVertices,
      indices: _lIndices,
    },
    {
      vertices: _rVertices,
      indices: _rIndices,
    },
  ]

  return geoAttrs
}
