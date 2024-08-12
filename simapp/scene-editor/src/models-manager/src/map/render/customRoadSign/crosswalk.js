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
  axisZ,
  getPositionNormal,
  halfPI,
  setRotateByDirNormal,
} from '../../utils/common'
import { whiteMaterial } from './index'

// 创建人行横道线
export function createCrosswalkLine (params) {
  const { data, parent, isOldMap = false, ignoreAlt = false } = params
  const { id, yaw, x, y, z, roadid: roadId } = data
  let { length, width } = data

  if (isOldMap) {
    // 兼容旧版地图的人行横道，长度和宽度调换
    [length, width] = [width, length]
  }
  const _yaw = isOldMap ? Number(yaw) + halfPI : Number(yaw)

  const objectId = String(id)
  const _z = ignoreAlt ? 0 : z
  const fixedYaw = Number(_yaw)

  const geo = getCrosswalkGeo({
    length,
    width,
  })

  const group = new Group()
  group.name = 'roadSignGroup'
  group.objectId = objectId

  const mesh = new Mesh(geo, whiteMaterial)
  mesh.name = 'roadSign'
  mesh.objectId = objectId
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  group.add(mesh)

  // 人行横道外层容器角度和位置调整
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

// 计算人行横道线的几何体属性
function getCrosswalkGeo (params) {
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
