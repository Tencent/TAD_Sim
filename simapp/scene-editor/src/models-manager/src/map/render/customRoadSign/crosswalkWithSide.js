import {
  BufferAttribute,
  BufferGeometry,
  DynamicDrawUsage,
  Group,
  Mesh,
  Vector2,
  Vector3,
} from 'three'
import {
  PI,
  axisZ,
  getPositionNormal,
  setRotateByDirNormal,
} from '../../utils/common'
import { whiteMaterial } from './index'

// 创建行人左右分道的人行横道线
export function createCrosswalkWithSide (params) {
  const { data, parent, ignoreAlt = false } = params
  const {
    length: totalLength,
    width: totalWidth,
    id,
    yaw,
    x,
    y,
    z,
    roadid: roadId,
  } = data

  const objectId = String(id)
  const _z = ignoreAlt ? 0 : z
  const fixedYaw = Number(yaw)

  // 三角形长度为1米，同时需要保留一定的间隔区域
  const arrowLength = 1 // 三角箭头的长度
  const gap = 0.4 // 两条人行横道线的中间间隔
  const arrowGap = 0.3 // 三角形箭头左右间隔
  const lineLength = totalLength - arrowLength - arrowGap * 2
  const lineWidth = (totalWidth - gap) / 2

  // 创建人行横道的白色实线
  const lineGeo = createCrosswalkGeo({
    length: lineLength,
    width: lineWidth,
  })

  // 创建三角形箭头
  const arrowGeo = createArrowGeo()

  const group = new Group()
  group.name = 'roadSignGroup'
  group.objectId = objectId

  // 将模型组装到容器中
  const lineMesh = new Mesh(lineGeo, whiteMaterial)
  // 白线区域的偏移量
  const lOffsetX = (gap + lineWidth) / 2
  const lOffsetY = (totalLength - lineLength) / 2
  lineMesh.position.set(lOffsetX, lOffsetY, 0)
  lineMesh.matrixAutoUpdate = false
  lineMesh.updateMatrix()
  group.add(lineMesh)

  // 上侧的三角箭头
  const arrowTopMesh = new Mesh(arrowGeo, whiteMaterial)
  // 三角箭头的偏移量
  const aOffsetY = -(lineLength - totalLength / 2 + gap + arrowLength / 2)
  const aTopOffsetX = gap / 2 + (lineWidth / 4) * 3
  arrowTopMesh.position.set(aTopOffsetX, aOffsetY, 0)
  arrowTopMesh.matrixAutoUpdate = false
  arrowTopMesh.updateMatrix()
  group.add(arrowTopMesh)

  // 下侧的三角箭头
  const arrowBottomMesh = new Mesh(arrowGeo, whiteMaterial)
  const aBottomOffsetX = gap / 2 + lineWidth / 4
  arrowBottomMesh.position.set(aBottomOffsetX, aOffsetY, 0)
  arrowBottomMesh.matrixAutoUpdate = false
  arrowBottomMesh.updateMatrix()
  group.add(arrowBottomMesh)

  // 将模型进行克隆，绕中心旋转180°得到另一侧的人行横道
  const _lineMesh = lineMesh.clone()
  _lineMesh.rotateOnAxis(axisZ, PI)
  _lineMesh.position.set(-lOffsetX, -lOffsetY, 0)
  _lineMesh.updateMatrix()
  group.add(_lineMesh)

  const _arrowTopMesh = arrowTopMesh.clone()
  _arrowTopMesh.rotateOnAxis(axisZ, PI)
  _arrowTopMesh.position.set(-aTopOffsetX, -aOffsetY, 0)
  _arrowTopMesh.updateMatrix()
  group.add(_arrowTopMesh)

  const _arrowBottomMesh = arrowBottomMesh.clone()
  _arrowBottomMesh.rotateOnAxis(axisZ, PI)
  _arrowBottomMesh.position.set(-aBottomOffsetX, -aOffsetY, 0)
  _arrowBottomMesh.updateMatrix()
  group.add(_arrowBottomMesh)

  // 外层容器的位置和角度调整
  if (
    !ignoreAlt &&
    getPositionNormal &&
    typeof getPositionNormal === 'function'
  ) {
    // 如果是常规带有坡道的道路情况，则将路面标线沿道路坡道调整角度
    const _p = new Vector3(x, y, z)
    let normal
    if (roadId) {
      // 如果是放置在道路上的元素，需要在请求当前位置法向量时，透传物体对应的 roadId
      normal = getPositionNormal(_p, roadId)
    } else {
      normal = getPositionNormal(_p)
    }
    const directionOnXY = new Vector2(
      Math.cos(fixedYaw),
      Math.sin(fixedYaw),
    ).normalize()
    setRotateByDirNormal(group, directionOnXY, normal)
  } else {
    // 路面标识角度调整
    group.rotateOnAxis(axisZ, fixedYaw)
  }
  group.position.set(x, y, _z)

  parent.add(group)
}

/**
 * 创建分道箭头
 */
function createArrowGeo () {
  /**
   *     /
   *    / \
   *   /   \
   *  /     \
   * /_______\
   *    | |
   *    |_|
   */
  const rectA = { x: 0.15, z: 0, y: 0 }
  const rectB = { x: 0.15, z: 0, y: -0.4 }
  const rectC = { x: -0.15, z: 0, y: -0.4 }
  const rectD = { x: -0.15, z: 0, y: 0 }
  const triA = { x: 0.3, z: 0, y: 0 }
  const triB = { x: -0.3, z: 0, y: 0 }
  const triC = { x: 0, z: 0, y: 0.6 }
  const vertices = []
  const indices = []
  vertices.push(
    rectA.x,
    rectA.y,
    rectA.z,
    rectB.x,
    rectB.y,
    rectB.z,
    rectC.x,
    rectC.y,
    rectC.z,
    rectD.x,
    rectD.y,
    rectD.z,
  )
  indices.push(0, 2, 1)
  indices.push(2, 0, 3)
  vertices.push(
    triA.x,
    triA.y,
    triA.z,
    triB.x,
    triB.y,
    triB.z,
    triC.x,
    triC.y,
    triC.z,
  )
  indices.push(4, 6, 5)

  const _vertices = new Float32Array(vertices)
  const geo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  geo.setIndex(indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  return geo
}

/**
 * 获取人行横道的几何体
 */
function createCrosswalkGeo (params) {
  const { length, width } = params
  /**
   * 人行横道，每一个实线长度为人行横道的 width 属性
   * 每一个长条白色实线区域，宽度为 40-45cm，间隔为60cm
   */
  const perLength = width // 每一个白线区域的长度
  const perWidth = 0.45 // 每一个白线区域的宽度
  const gap = 0.6 // 间隔

  // 顶点的坐标
  const vertices = []
  // 三角面的索引
  const indices = []
  // 最左和最右边界的角点
  const edgePoints = []
  // 第一个白色实体绘制的偏移距离
  const firstOffset = 0.1 // 10cm
  // 绘制白色实体的偏移量
  let offset = firstOffset
  // 循环的索引
  let basicIndex = 0
  const halfLength = perLength / 2

  // 用 while 代替 for 循环，
  // 如果从偏移处开始，到终点距离有1/3条白线宽度的距离，都可以绘制白色实体区域(只不过最后的白色实体可能会不完整)
  while (offset + perWidth / 3 <= length) {
    // 由于最后的白色区域不一定能够绘制一条完整的，所以最后一条需要动态调整绘制的宽度
    let areaWidth = perWidth
    if (length - offset < perWidth) {
      areaWidth = length - offset
    }
    const tl = {
      x: halfLength,
      y: offset - length / 2,
      z: 0,
    }
    const tr = {
      x: halfLength,
      y: offset + areaWidth - length / 2,
      z: 0,
    }
    const br = {
      x: -halfLength,
      y: offset + areaWidth - length / 2,
      z: 0,
    }
    const bl = {
      x: -halfLength,
      y: offset - length / 2,
      z: 0,
    }
    if (basicIndex === 0) {
      // 第一组实体边界
      edgePoints.push(tl.x, tl.y, tl.z)
      edgePoints.push(bl.x, bl.y, bl.z)
    } else if (offset + (perWidth / 3) * 4 + gap > length) {
      // 最后一组实体边界
      edgePoints.push(br.x, br.y, br.z)
      edgePoints.push(tr.x, tr.y, tr.z)
    }

    // 一组4个角点的坐标
    vertices.push(tl.x, tl.y, tl.z)
    vertices.push(tr.x, tr.y, tr.z)
    vertices.push(br.x, br.y, br.z)
    vertices.push(bl.x, bl.y, bl.z)
    // 索引
    const indexLt = basicIndex * 4 + 0
    const indexLr = basicIndex * 4 + 1
    const indexBr = basicIndex * 4 + 2
    const indexBt = basicIndex * 4 + 3
    // 一组2个三角面的顶点索引顺序
    indices.push(indexLt, indexLr, indexBr)
    indices.push(indexBr, indexBt, indexLt)

    offset += perWidth + gap
    basicIndex++
  }

  // 人行横道实线实体
  // 基于顶点和索引，创建人行横道的geometry
  const _vertices = new Float32Array(vertices)
  const geo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  geo.setIndex(indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  return geo
}
