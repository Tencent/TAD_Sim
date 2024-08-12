import {
  BufferAttribute,
  BufferGeometry,
  DoubleSide,
  DynamicDrawUsage,
  Mesh,
  MeshBasicMaterial,
  Vector3,
} from 'three'
import { createCrosswalkLine } from './crosswalk'
import { createCrosswalkWithSide } from './crosswalkWithSide'
import { createSemicircleLine } from './semicircleLine'
import { createGuideLaneLine } from './guideLaneLine'
import { createVariableDirectionLaneLine } from './variableDirectionLaneLine'
import { createIntersectionGuideLine } from './intersectionGuideLine'
import { createStopLine } from './stopLine'
import { createNonMotorVehicleArea } from './nonMotorVehicleArea'
import { createLonDecelerationMarking } from './longitudinalDecelerationMarking'
import { createLateralDecelerationMarking } from './lateralDecelerationMarking'

export const whiteMaterial = new MeshBasicMaterial({
  color: 0xFFFFFF,
  side: DoubleSide,
})

export const yellowMaterial = new MeshBasicMaterial({
  color: 0xE8E800,
  side: DoubleSide,
})

// 三维样条曲线的默认类型
export const CurveType = 'centripetal'

// 特殊路面标线的列表
export const customRoadSignList = [
  'White_Semicircle_Line_Vehicle_Distance_Confirmation', // 白色半圆状车距确认线
  'Crosswalk_Line', // 人行横道线
  'Crosswalk_with_Left_and_Right_Side', // 行人左右分道的人行横道线
  'Road_Guide_Lane_Line', // 导向车道线
  'Variable_Direction_Lane_Line', // 可变导向车道线
  'Intersection_Guide_Line', // 路口导向线
  'Longitudinal_Deceleration_Marking', // 车行道纵向减速标线
  'Lateral_Deceleration_Marking', // 车行道横向减速标线
  'Stop_Line', // 停止线
  'Non_Motor_Vehicle_Area', // 非机动车禁驶区标记
]

// 渲染特殊的路面标线的入口函数
export function createCustomRoadSign (params) {
  const { name, data, parent, isOldMap = false, ignoreAlt = false } = params
  const _params = {
    data,
    parent,
    isOldMap,
    ignoreAlt,
  }
  if (name === 'Crosswalk_Line') {
    createCrosswalkLine(_params)
  } else if (name === 'Crosswalk_with_Left_and_Right_Side') {
    createCrosswalkWithSide(_params)
  } else if (name === 'White_Semicircle_Line_Vehicle_Distance_Confirmation') {
    createSemicircleLine(_params)
  } else if (name === 'Road_Guide_Lane_Line') {
    createGuideLaneLine(_params)
  } else if (name === 'Variable_Direction_Lane_Line') {
    createVariableDirectionLaneLine(_params)
  } else if (name === 'Intersection_Guide_Line') {
    createIntersectionGuideLine(_params)
  } else if (name === 'Stop_Line') {
    createStopLine(_params)
  } else if (name === 'Non_Motor_Vehicle_Area') {
    createNonMotorVehicleArea(_params)
  } else if (name === 'Longitudinal_Deceleration_Marking') {
    createLonDecelerationMarking(_params)
  } else if (name === 'Lateral_Deceleration_Marking') {
    createLateralDecelerationMarking(_params)
  }
}

/**
 * 获取绕 Z 轴顺时针旋转 90 度的单位向量
 * @param vec3
 * @returns
 */
export function getVerticalVector (vec3) {
  return new Vector3(vec3.y, -vec3.x, 0).normalize()
}

// 渲染通过 geoAttrs 属性组装的自定义路面标线
export function renderCustomRoadSignByGeoAttrs (params) {
  const { geoAttrs, group, color = 'white', objectId } = params
  let mat = whiteMaterial
  if (color === 'yellow') {
    mat = yellowMaterial
  }

  geoAttrs.forEach((geoAttr) => {
    const { vertices, indices } = geoAttr
    const geo = new BufferGeometry()
    const positionAttr = new BufferAttribute(vertices, 3).setUsage(
      DynamicDrawUsage,
    )
    geo.setIndex(indices)
    geo.setAttribute('position', positionAttr)
    geo.computeVertexNormals()

    const mesh = new Mesh(geo, mat)
    mesh.name = 'customRoadSign'
    mesh.objectId = objectId
    mesh.matrixAutoUpdate = false
    mesh.updateMatrix()
    group.add(mesh)
  })
}

/**
 * 计算一条带有宽度的路径的几何体顶点相关属性
 * @param {*} params
 */
export function calcSolidBoundaryGeoAttr (params) {
  const { width, keyPath, segment } = params
  let space = Number((1 / segment).toFixed(8))
  if (space > 1 / segment) {
    space = space - 1 / 10 ** 8
  }
  const innerPoints = []
  const outerPoints = []

  for (let i = 0; i <= segment; i++) {
    const percent = Math.max(0, Math.min(1, i * space))
    const refPoint = keyPath.getPointAt(percent)

    const tangent = keyPath.getTangentAt(percent)
    const vertical = getVerticalVector(tangent)

    const innerPoint = refPoint.clone()
    innerPoint.addScaledVector(vertical, -(width / 2))
    innerPoints.push(innerPoint)

    const outerPoint = refPoint.clone()
    outerPoint.addScaledVector(vertical, width / 2)
    outerPoints.push(outerPoint)
  }

  // 基于路边内外侧的两组采样点，计算对应几何体的属性
  const { vertices, indices } = getGeometryAttrBySamplePoints(
    innerPoints,
    outerPoints,
  )

  return {
    vertices,
    indices,
  }
}

/**
 * 通过两条边界的采样点，获取几何体属性
 * @param points1
 * @param points2
 * @returns
 */
export function getGeometryAttrBySamplePoints (points1, points2) {
  // 由于第三方地图部分车道边界线存在左右边界采样点数量不一致的情况，取车道数量的小值
  const minLength = Math.min(points1.length, points2.length)
  const vertices = []
  const indices = []
  const uvs = []
  const deltaU = 1 / (minLength - 1)
  const deltaV = 1

  /**
   * a ---- b
   * |      |
   * c ---- d
   * 车道几何体，由车道两侧边界线的相同数量的采样点组成
   * 每四个点可以组成一对三角面，按照三角面的顶点顺时针来排列组合，三角面的顶点为 abd 和 adc
   * 为了保证后续纹理能够应用到车道的网格中，需要手动指定每个顶点的 uv 坐标
   */
  // 添加顶点
  for (let i = 0; i < minLength; i++) {
    const p1 = points1[i]
    const p2 = points2[i]
    const u = deltaU * i
    const v1 = 0
    const v2 = deltaV
    // 顶点坐标
    vertices.push(p1.x, p1.y, p1.z)
    vertices.push(p2.x, p2.y, p2.z)
    // 每个顶点对应的 uv 坐标
    uvs.push(u, v1)
    uvs.push(u, v2)

    if (i < minLength - 1) {
      // 在多个三角形中重复使用顶点，即为索引三角形
      const a = i * 2
      const b = i * 2 + 1
      const c = (i + 1) * 2
      const d = (i + 1) * 2 + 1
      indices.push(a, b, d) // face1
      indices.push(a, d, c) // face2
    }
  }

  const _vertices = new Float32Array(vertices)
  const _uvs = new Float32Array(uvs)

  return {
    vertices: _vertices,
    uvs: _uvs,
    indices,
  }
}

/**
 * 获取一条曲线上某个超过范围对应百分比的偏移点
 * @param percent
 */
export function getPointOutOfScope (params) {
  const { curve, percent, offset: tOffset } = params
  const roadLength = curve.getLength()
  // 取首尾顶点的切线方向
  let _percent = 0
  // 计算s方向的偏移量
  let sOffset = 0
  if (percent < 0) {
    sOffset = roadLength * (percent - 0)
  } else {
    _percent = 1
    sOffset = roadLength * (percent - 1)
  }

  const tangent = curve.getTangentAt(_percent)
  const endPoint = curve.getPointAt(_percent)
  // 切线延长线方向上的延伸点
  const refLinePoint = endPoint.clone().addScaledVector(tangent, sOffset)
  const vertical = getVerticalVector(tangent).normalize()
  const targetPoint = refLinePoint.clone().addScaledVector(vertical, tOffset)

  return targetPoint
}
