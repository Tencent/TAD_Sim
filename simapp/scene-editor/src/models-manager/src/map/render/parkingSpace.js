import {
  BufferAttribute,
  BufferGeometry,
  Color,
  DoubleSide,
  DynamicDrawUsage,
  Group,
  Mesh,
  MeshBasicMaterial,
  Vector3,
} from 'three'
import { axisZ, defaultColor, transformVec3ByObject } from '../utils/common'

const basicMat = new MeshBasicMaterial({
  color: 0xFFFFFF,
  side: DoubleSide,
})

/**
 * 新版停车位的网格创建
 * 假如透传的停车位数据结构如下：
 * {
 *   id: number
 *   name: 'Parking_Space_Mark'
 *   strType: 'parkingSpace'
 *   marking: {
 *     markwidth: number
 *     markcolor: 'white' | 'yellow' | 'red' | 'green' | 'blue'
 *   }
 *   outlines: Array<{
 *     id: number
 *     outline: Array<{
 *         pointid: number
 *         x: number
 *         y: number
 *         z: number
 *     }>
 *   }>
 *   // 一组停车位的角点
 *   // 线宽
 *   // 颜色
 * }
 * @param {*} data
 */

export async function createParkingSpace (params) {
  const { data, parent, ignoreAlt = false } = params
  const { id, marking, outlines: outlinePoints } = data

  // 如果是限时停车位，则使用虚线的几何体
  const isDashed = data.name === 'Time_Limit_Parking_Space_Mark'

  // 如果存在坡度，则需要手动调整停车位的角点高度
  if (!ignoreAlt && outlinePoints.length > 1) {
    // 一组停车位中，每一个停车位的高度
    const heights = outlinePoints.map(data => data.outline[0].z)
    /**
     * 每一个停车位的 4 个角点，是按照的方式排序的
     * 3 ------- 2
     * |         |
     * 0 ------- 1
     */
    for (let i = 0; i < outlinePoints.length; i++) {
      const outlinePoint = outlinePoints[i]
      const current = heights[i]
      let preOffset = 0
      let nextOffset = 0
      if (i === 0) {
        // 第一个停车位
        const next = heights[i + 1]
        nextOffset = (next - current) / 2
        preOffset = -nextOffset
      } else if (i === outlinePoints.length - 1) {
        // 最后一个停车位
        const pre = heights[i - 1]
        preOffset = (pre - current) / 2
        nextOffset = -preOffset
      } else {
        // 中间的车位
        const pre = heights[i - 1]
        const next = heights[i + 1]
        preOffset = (pre - current) / 2
        nextOffset = (next - current) / 2
      }
      // 前侧的2个角点
      outlinePoint.outline[0].z = current + preOffset
      outlinePoint.outline[3].z = current + preOffset
      // 后侧的2个角点
      outlinePoint.outline[1].z = current + nextOffset
      outlinePoint.outline[2].z = current + nextOffset
    }
  }

  const { markwidth: lineWidth, markcolor = 'white' } = marking
  // 考虑到传入的线宽可能为0的情况
  const _lineWidth = Number(lineWidth) === 0 ? 0.2 : Number(lineWidth)
  // 兼容 markcolor 可能为 none 的情况
  const _markColor = defaultColor[markcolor] ? markcolor : 'white'

  const geoAttrs = []
  // 基于一组停车位遍历获取每一个停车位的顶点数据
  for (const outlineInfo of outlinePoints) {
    const { outline: points } = outlineInfo
    // 如果角点的数据不是4个，则视为无效
    if (points.length !== 4) continue
    if (ignoreAlt) {
      points.forEach((p) => {
        // 将距离水平面的高度统一设为 0
        p.z = 0
      })
    }
    let geoAttr
    if (isDashed) {
      geoAttr = parseDashedParkingSpace({ points, lineWidth: _lineWidth })
    } else {
      geoAttr = parseParkingSpace({ points, lineWidth: _lineWidth })
    }
    geoAttrs.push(geoAttr)
  }

  const lineColor = defaultColor[_markColor].value
  // 使用基本材质的 clone
  const mat = basicMat.clone()
  mat.color = new Color(lineColor)
  mat.needsUpdate = true

  // 一组停车位最外层的容器
  const parkingGroup = new Group()
  parkingGroup.name = 'parkingSpaceGroup'
  parkingGroup.objectId = String(id)

  // 遍历一组停车位的所有顶点数据，创建网格
  geoAttrs.forEach((geoAttr) => {
    const { vertices, indices } = geoAttr
    const _vertices = vertices instanceof Float32Array ?
      vertices :
      new Float32Array(vertices)
    const geo = new BufferGeometry()
    const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
      DynamicDrawUsage,
    )
    geo.setIndex(indices)
    geo.setAttribute('position', positionAttr)
    geo.computeVertexNormals()

    // 描边的网格
    const mesh = new Mesh(geo, mat)
    mesh.name = 'parkingSpace'
    mesh.objectId = String(id)
    mesh.matrixAutoUpdate = false
    mesh.updateMatrix()
    parkingGroup.add(mesh)
  })

  parent.add(parkingGroup)
}

// 解析单个停车位的数据
function parseParkingSpace (data) {
  const { points, lineWidth } = data
  // 一个停车位有 4 个角点
  const cornerCount = 4
  // 基于四个角点的顶点坐标，计算带有宽度的描边定位顶点坐标
  const _points = transformVec3ByObject(points)
  const outlinePoints = []
  for (let i = 0; i < cornerCount; i++) {
    // 每一个基准点，都要基于前、后的基准点共同计算三角面的顶点
    const prePoint = _points[(i - 1 + cornerCount) % cornerCount]
    const curPoint = _points[i]
    const nextPoint = _points[(i + 1) % cornerCount]

    // 两条边的方向
    const preDirection = prePoint
      .clone()
      .sub(curPoint)
      .normalize()
    const nextDirection = nextPoint
      .clone()
      .sub(curPoint)
      .normalize()

    // 两条边形成的夹角
    const angle = preDirection.angleTo(nextDirection) / 2
    const cornerLength = lineWidth / Math.sin(angle)

    // 两条边的角平分线方向【场景编辑器返回的停车位角点数据为逆时针】
    const cornerDirection = preDirection.clone().applyAxisAngle(axisZ, -angle)

    // 一个角对应的内、外两个定位点
    const innerPoint = curPoint
      .clone()
      .addScaledVector(cornerDirection, cornerLength / 2)
    const outerPoint = curPoint
      .clone()
      .addScaledVector(cornerDirection, -cornerLength / 2)

    outlinePoints.push(innerPoint, outerPoint)
  }

  // 将停车位四个角内、外侧的定位点，按照三角面的组装顺序拼接几何体顶点属性
  // 将停车位四个角内侧和外侧的顶点，按照三角面的组装顺序，拼接顶点
  const vertices = []
  const indices = []
  const vLength = outlinePoints.length
  for (let i = 0; i < vLength; i += 2) {
    // 顶点的索引
    const index1 = i
    const index2 = (i + 1 + vLength) % vLength
    const index3 = (i + 2 + vLength) % vLength
    const index4 = (i + 3 + vLength) % vLength

    const innerPoint = outlinePoints[index1]
    const outerPoint = outlinePoints[index2]

    // 顶点的坐标（顶点只添加一遍）
    vertices.push(innerPoint.x, innerPoint.y, innerPoint.z)
    vertices.push(outerPoint.x, outerPoint.y, outerPoint.z)

    // 通过定义三角面的顶点索引，重复利用顶点，即索引三角面
    indices.push(index1, index2, index3) // face1
    indices.push(index3, index2, index4) // face2
  }

  const _vertices = new Float32Array(vertices)

  return {
    vertices: _vertices,
    indices,
  }
}

// 解析单个停车位的数据
function parseDashedParkingSpace (data) {
  const { points, lineWidth } = data
  // 一个停车位有 4 个角点
  const cornerCount = 4
  const _points = transformVec3ByObject(points)
  // 基于四个角的顶点坐标，计算带有宽度的描边定位顶点坐标
  const outlinePoints = [[], [], [], []]
  // 虚线间隔0.6米
  const span = 0.6
  // 三角面顶点索引的基准值
  const vertexBasicIndex = []
  for (let i = 0; i < cornerCount; i++) {
    // 每一个基准点，都要基于前、后的基准点共同计算三角面的顶点
    const prePoint = _points[(i - 1 + cornerCount) % cornerCount]
    const curPoint = _points[i]
    const nextPoint = _points[(i + 1) % cornerCount]

    // 两条边的方向
    const preDirection = prePoint
      .clone()
      .sub(curPoint)
      .normalize()
    const nextDirection = nextPoint
      .clone()
      .sub(curPoint)
      .normalize()

    // 两条边形成的夹角
    const angle = preDirection.angleTo(nextDirection) / 2
    const cornerLength = lineWidth / Math.sin(angle)

    // 两条边的角平分线方向【场景编辑器返回的停车位角点数据为逆时针】
    const cornerDirection = preDirection.clone().applyAxisAngle(axisZ, -angle)

    // 一个角对应的内、外两个定位点
    const innerPoint = curPoint
      .clone()
      .addScaledVector(cornerDirection, cornerLength / 2)
    const outerPoint = curPoint
      .clone()
      .addScaledVector(cornerDirection, -cornerLength / 2)

    outlinePoints[i].push(innerPoint, outerPoint)

    // 基于当前定位点，和下一组定位点，计算当前边的长度
    // 保证两端的虚线实体都存在，即[实体,间隔,实体,...,实体,间隔,实体]，所以两端的实体可以更长一些
    const dist = nextPoint.distanceTo(curPoint)
    // 分段数，理应大于2
    const segment = Math.floor(dist / span)
    // 中间等分跨度数量，要么为 0，要么一定是奇数
    let middleSpanCount = 0
    if (segment % 2 === 1) {
      // 如果是奇数
      middleSpanCount = Math.max(0, segment - 2)
    } else {
      // 如果是偶数
      middleSpanCount = Math.max(0, segment - 3)
    }
    // 虚线实体的数量
    const entityCount = Math.max(0, (middleSpanCount - 1) / 2)

    // 左右两端实体的长度
    let sideSpan = (dist - middleSpanCount * span) / 2
    if (entityCount === 0) {
      // 如果中间没有虚线的实体部分，则限定两端的实体长度
      sideSpan = span
    }
    // 从当前定位点到下一个定位点方向的垂直向量
    const vertical = new Vector3(-nextDirection.y, nextDirection.x, 0)

    // 第一组实体边界顶点
    const firstBasicPoint = curPoint
      .clone()
      .addScaledVector(nextDirection, sideSpan)
    const firstInnerPoint = firstBasicPoint
      .clone()
      .addScaledVector(vertical, lineWidth / 2)
    const firstOuterPoint = firstBasicPoint
      .clone()
      .addScaledVector(vertical, -lineWidth / 2)
    outlinePoints[i].push(firstInnerPoint, firstOuterPoint)

    // 中间的实体个数，每一个实体对应4个顶点（2个三角面）
    let offset = sideSpan + span
    let count = 0
    while (count < entityCount * 2) {
      const basicPoint = curPoint.clone().addScaledVector(nextDirection, offset)
      const iPoint = basicPoint.clone().addScaledVector(vertical, lineWidth / 2)
      const oPoint = basicPoint
        .clone()
        .addScaledVector(vertical, -lineWidth / 2)
      outlinePoints[i].push(iPoint, oPoint)

      count++
      offset += span
    }

    // 最后一组实体边界顶点
    const lastBasicPoint = curPoint
      .clone()
      .addScaledVector(nextDirection, dist - sideSpan)
    const lastInnerPoint = lastBasicPoint
      .clone()
      .addScaledVector(vertical, lineWidth / 2)
    const lastOuterPoint = lastBasicPoint
      .clone()
      .addScaledVector(vertical, -lineWidth / 2)
    outlinePoints[i].push(lastInnerPoint, lastOuterPoint)

    if (i === 0) {
      vertexBasicIndex[i] = 0
    } else {
      // 顶点索引的基准值依赖上一组顶点的数据
      vertexBasicIndex[i] = vertexBasicIndex[i - 1] + outlinePoints[i - 1].length
    }
  }

  const vertices = []
  const indices = []

  // 针对停车位的每一条边界，拼装对应的虚线几何体
  for (let i = 0; i < cornerCount; i++) {
    const curOutlinePoints = outlinePoints[i]
    const basicIndex = vertexBasicIndex[i]
    for (let j = 0; j < curOutlinePoints.length; j += 4) {
      let p1, p2, p3, p4
      if (j + 4 > curOutlinePoints.length) {
        // 最后一组实体边界顶点，跟下一条边的2个角点，组成另一段实体
        p1 = curOutlinePoints[j]
        p2 = curOutlinePoints[j + 1]
      } else {
        // 2个角点，跟第一组实体边界顶点，组成第一段实体
        p1 = curOutlinePoints[j]
        p2 = curOutlinePoints[j + 1]
        p3 = curOutlinePoints[j + 2]
        p4 = curOutlinePoints[j + 3]
      }

      // 保存顶点坐标
      vertices.push(p1.x, p1.y, p1.z)
      vertices.push(p2.x, p2.y, p2.z)
      p3 && vertices.push(p3.x, p3.y, p3.z)
      p4 && vertices.push(p4.x, p4.y, p4.z)

      // 形成三角面的顶点索引
      const index1 = basicIndex + j
      const index2 = basicIndex + j + 1
      let index3 = basicIndex + j + 2
      let index4 = basicIndex + j + 3
      if (!p3 || !p4) {
        // 索引为下一组的角点
        const nextBasicIndex = (i + 1) % cornerCount
        index3 = vertexBasicIndex[nextBasicIndex]
        index4 = vertexBasicIndex[nextBasicIndex] + 1
      }

      indices.push(index1, index3, index2)
      indices.push(index4, index2, index3)
    }
  }

  // 主动释放内存
  outlinePoints.length = 0
  vertexBasicIndex.length = 0

  const _vertices = new Float32Array(vertices)

  return {
    vertices: _vertices,
    indices,
  }
}
