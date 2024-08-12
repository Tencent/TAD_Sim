import {
  PI,
  axisZ,
  getPositionNormal,
  setRotateByDirNormal,
} from '../../utils/common'
import { yellowMaterial } from './index'
import {
  BufferAttribute,
  BufferGeometry,
  DynamicDrawUsage,
  Group,
  Mesh,
  Vector2,
  Vector3,
} from 'three'

/**
 * 创建非机动车禁驶区标记【路口中】
 * @param {object} params - 参数对象，包含数据、父对象和是否忽略高度等信息
 * @param {object} params.data - 包含长度、宽度、ID、偏航角、x、y、z和roadId的数据对象
 * @param {object} params.parent - 父对象，用于添加创建的组
 * @param {boolean} [params.ignoreAlt] - 是否忽略高度，默认为false
 * @returns {void} 无返回值
 */
export function createNonMotorVehicleArea (params) {
  const { data, parent, ignoreAlt = false } = params
  const { length, width, id, yaw, x, y, z, roadid: roadId } = data

  const objectId = String(id)
  const _z = ignoreAlt ? 0 : z
  const fixedYaw = Number(yaw)

  // 向外延伸的距离 3 米
  const extendLength = 3
  // 矩形区域的范围
  const rectLength = length - extendLength * 2
  const rectWidth = width - extendLength * 2
  const halfRectLength = rectLength / 2
  const halfRectWidth = rectWidth / 2
  // 每一段黄色实体的长度 1 米
  const perLength = 1
  // 每一段黄色实体的宽度 0.2 米
  const perWidth = 0.2
  const halfPerWidth = perWidth / 2

  // 长度和宽度方向开始循环计算黄色实体角点的偏移量
  let lOffset = halfRectLength + extendLength
  let wOffset = -halfRectWidth - extendLength
  // 长度和宽度方向，顶点和索引的数组
  const lVertices = []
  const lIndices = []
  const wVertices = []
  const wIndices = []

  // 黄色实体的材质
  const mat = yellowMaterial

  // 只需要计算一次长度方向和宽度方向的定位点数据，通过clone和rotate的形式创建其他的
  // 长度方向的计算
  let lBasicIndex = 0
  let wBasicIndex = 0
  while (lOffset > -halfRectLength) {
    let step = perLength
    // 如果索引是 0，则步长是延长的长度
    if (lBasicIndex === 0) {
      step = extendLength
    } else {
      if (lOffset + halfRectLength < perLength) {
        step = lOffset + halfRectLength
      }
    }
    const p1 = {
      x: lOffset,
      y: -halfRectWidth + halfPerWidth,
      z: 0,
    }
    const p2 = {
      x: lOffset - step,
      y: -halfRectWidth + halfPerWidth,
      z: 0,
    }
    const p3 = {
      x: lOffset - step,
      y: -halfRectWidth - halfPerWidth,
      z: 0,
    }
    const p4 = {
      x: lOffset,
      y: -halfRectWidth - halfPerWidth,
      z: 0,
    }

    lVertices.push(p1.x, p1.y, p1.z)
    lVertices.push(p2.x, p2.y, p2.z)
    lVertices.push(p3.x, p3.y, p3.z)
    lVertices.push(p4.x, p4.y, p4.z)

    const index1 = lBasicIndex * 4 + 0
    const index2 = lBasicIndex * 4 + 1
    const index3 = lBasicIndex * 4 + 2
    const index4 = lBasicIndex * 4 + 3

    lIndices.push(index1, index2, index3)
    lIndices.push(index3, index4, index1)

    if (lBasicIndex === 0) {
      lOffset -= extendLength
    } else {
      lOffset -= perLength * 2
    }
    lBasicIndex++
  }

  // 宽度方向的循环计算
  while (wOffset < halfRectWidth) {
    let step = perLength
    if (wBasicIndex === 0) {
      step = extendLength
    } else {
      if (halfRectWidth - wOffset < perLength) {
        step = halfRectWidth - wOffset
      }
    }
    const p1 = {
      x: -halfRectLength + halfPerWidth,
      y: wOffset,
      z: 0,
    }
    const p2 = {
      x: -halfRectLength + halfPerWidth,
      y: wOffset + step,
      z: 0,
    }
    const p3 = {
      x: -halfRectLength - halfPerWidth,
      y: wOffset + step,
      z: 0,
    }
    const p4 = {
      x: -halfRectLength - halfPerWidth,
      y: wOffset,
      z: 0,
    }

    wVertices.push(p1.x, p1.y, p1.z)
    wVertices.push(p2.x, p2.y, p2.z)
    wVertices.push(p3.x, p3.y, p3.z)
    wVertices.push(p4.x, p4.y, p4.z)

    const index1 = wBasicIndex * 4 + 0
    const index2 = wBasicIndex * 4 + 1
    const index3 = wBasicIndex * 4 + 2
    const index4 = wBasicIndex * 4 + 3

    wIndices.push(index1, index2, index3)
    wIndices.push(index3, index4, index1)

    if (wBasicIndex === 0) {
      wOffset += extendLength
    } else {
      wOffset += perLength * 2
    }
    wBasicIndex++
  }

  // 外层的容器
  const group = new Group()
  group.name = 'roadSignGroup'
  group.objectId = objectId

  // 长度方向的网格
  const _lVertices = new Float32Array(lVertices)
  const lGeo = new BufferGeometry()
  const lPositionAttr = new BufferAttribute(_lVertices, 3).setUsage(
    DynamicDrawUsage,
  )
  lGeo.setIndex(lIndices)
  lGeo.setAttribute('position', lPositionAttr)
  lGeo.computeVertexNormals()

  const lMesh = new Mesh(lGeo, mat)
  lMesh.matrixAutoUpdate = false
  lMesh.updateMatrix()
  group.add(lMesh)

  const lMeshClone = lMesh.clone()
  lMeshClone.rotateOnAxis(axisZ, PI)
  lMeshClone.matrixAutoUpdate = false
  lMeshClone.updateMatrix()
  group.add(lMeshClone)

  // 宽度方向的网格
  const _wVertices = new Float32Array(wVertices)
  const wGeo = new BufferGeometry()
  const wPositionAttr = new BufferAttribute(_wVertices, 3).setUsage(
    DynamicDrawUsage,
  )
  wGeo.setIndex(wIndices)
  wGeo.setAttribute('position', wPositionAttr)
  wGeo.computeVertexNormals()

  const wMesh = new Mesh(wGeo, mat)
  wMesh.matrixAutoUpdate = false
  wMesh.updateMatrix()
  group.add(wMesh)

  const wMeshClone = wMesh.clone()
  wMeshClone.rotateOnAxis(axisZ, PI)
  wMeshClone.matrixAutoUpdate = false
  wMeshClone.updateMatrix()
  group.add(wMeshClone)
  group.traverse((child) => {
    if (child instanceof Mesh) {
      child.name = 'roadSign'
      child.objectId = objectId
    }
  })
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
