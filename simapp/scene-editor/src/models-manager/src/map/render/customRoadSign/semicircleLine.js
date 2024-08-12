import { CatmullRomCurve3, Group, Vector3 } from 'three'
import { PI, fixedPrecision, transformVec3ByObject } from '../../utils/common'
import {
  CurveType,
  getPointOutOfScope,
  getVerticalVector,
  renderCustomRoadSignByGeoAttrs,
} from './index'

// 每 50 米设置一个半圆状确认线
const step = 50
// 半径 30cm
const radius = 0.3
const relativeOffsets = []
function initRelativeOffsets (params) {
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

export function createSemicircleLine (params) {
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

  // 基于左右边界的采样点数组，计算对应几何体属性
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
 * 计算半圆状线的几何体属性
 * @param params
 */
function calcSemicircleGeoAttrs (params) {
  const { curve, isLeft, length: totalLength } = params

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
      const tOffset = isLeft ? -(basicTOffset + t) : basicTOffset + t
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

// 计算几何体属性
function getGeoAttrs (params) {
  const { leftPoints, rightPoints, length: totalLength } = params

  // 左侧边界形成的几何体
  const leftPath = new CatmullRomCurve3(transformVec3ByObject(leftPoints))
  leftPath.curveType = CurveType

  // 右侧边界形成的几何体
  const rightPath = new CatmullRomCurve3(transformVec3ByObject(rightPoints))
  rightPath.curveType = CurveType

  const { vertices: lVertices, indices: lIndices } = calcSemicircleGeoAttrs({
    curve: leftPath,
    isLeft: true,
    length: totalLength,
  })
  const _lVertices = new Float32Array(lVertices)

  const { vertices: rVertices, indices: rIndices } = calcSemicircleGeoAttrs({
    curve: rightPath,
    isLeft: false,
    length: totalLength,
  })
  const _rVertices = new Float32Array(rVertices)

  // 半圆状确认线的几何体属性
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
