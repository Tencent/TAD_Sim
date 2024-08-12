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
  setRotateByDirNormal,
} from '../../utils/common'
import { whiteMaterial } from './index'

/**
 * 创建停止线
 * @param {object} params - 参数对象，包含数据、父对象和是否忽略高度等信息
 * @param {object} params.data - 停止线的数据，包括长度、宽度、ID、偏航角、位置等
 * @param {object} params.parent - 父对象，用于将创建的停止线添加到场景中
 * @param {boolean} [params.ignoreAlt] - 是否忽略高度信息，默认为false
 * @returns {void} 无返回值
 */
export function createStopLine (params) {
  const { data, parent, ignoreAlt = false } = params
  const { length, width, id, yaw, x, y, z, roadid: roadId } = data

  const objectId = String(id)
  const _z = ignoreAlt ? 0 : z
  const fixedYaw = Number(yaw)

  // 停止线为一根长条的白色矩形网格
  const vertices = []
  const indices = []

  const p1 = {
    x: width / 2,
    y: length / 2,
    z: 0,
  }
  const p2 = {
    x: -width / 2,
    y: length / 2,
    z: 0,
  }
  const p3 = {
    x: -width / 2,
    y: -length / 2,
    z: 0,
  }
  const p4 = {
    x: width / 2,
    y: -length / 2,
    z: 0,
  }
  vertices.push(p1.x, p1.y, p1.z)
  vertices.push(p2.x, p2.y, p2.z)
  vertices.push(p3.x, p3.y, p3.z)
  vertices.push(p4.x, p4.y, p4.z)
  indices.push(0, 1, 2)
  indices.push(2, 3, 0)

  // 外层的容器
  const group = new Group()
  group.name = 'roadSignGroup'
  group.objectId = objectId

  const _vertices = new Float32Array(vertices)
  const geo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  geo.setIndex(indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  const mesh = new Mesh(geo, whiteMaterial)
  mesh.name = 'roadSign'
  mesh.objectId = objectId
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  group.add(mesh)

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
