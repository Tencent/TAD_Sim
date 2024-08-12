import { CatmullRomCurve3, Group } from 'three'
import { transformVec3ByObject } from '../../utils/common'
import {
  CurveType,
  calcSolidBoundaryGeoAttr,
  renderCustomRoadSignByGeoAttrs,
} from './index'

const guideLaneLineWidth = 0.2

export function createGuideLaneLine (params) {
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

// 计算导向车道线的几何体属性
function getGeoAttrs (params) {
  const { leftPoints, rightPoints, width } = params

  // 导向车道线采样点划分为 15 段，应该能够满足渲染效果
  const segment = 15

  // 左侧边界形成的几何体
  const leftPath = new CatmullRomCurve3(transformVec3ByObject(leftPoints))
  leftPath.curveType = CurveType

  const { vertices: lVertices, indices: lIndices } = calcSolidBoundaryGeoAttr({
    keyPath: leftPath,
    width,
    segment,
  })

  // 右侧边界形成的几何体
  const rightPath = new CatmullRomCurve3(transformVec3ByObject(rightPoints))
  rightPath.curveType = CurveType

  const { vertices: rVertices, indices: rIndices } = calcSolidBoundaryGeoAttr({
    keyPath: rightPath,
    width,
    segment,
  })

  // 左右边界形成的几何体属性
  const geoAttrs = [
    {
      vertices: lVertices,
      indices: lIndices,
    },
    {
      vertices: rVertices,
      indices: rIndices,
    },
  ]

  return geoAttrs
}
