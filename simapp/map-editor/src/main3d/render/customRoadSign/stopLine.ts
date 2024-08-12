import {
  BufferAttribute,
  BufferGeometry,
  DynamicDrawUsage,
  Group,
  Mesh,
  Vector3,
} from 'three'
import { type IRenderRoadSign, customRoadSignMaterial } from '../roadSign'
import { RenderOrder } from '@/utils/business'

/**
 * 创建停止线
 */
export function createStopLine (params: IRenderRoadSign) {
  const { data: roadSignData, parent } = params
  const {
    id: objectId,
    length,
    width,
    roadId,
    position,
    lookAtPoint,
    projectNormal,
  } = roadSignData

  // 停止线为一根长条的白色矩形网格
  const vertices = []
  const indices = []

  const p1 = {
    x: length / 2,
    y: 0,
    z: width / 2,
  }
  const p2 = {
    x: length / 2,
    y: 0,
    z: -width / 2,
  }
  const p3 = {
    x: -length / 2,
    y: 0,
    z: -width / 2,
  }
  const p4 = {
    x: -length / 2,
    y: 0,
    z: width / 2,
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
  group.roadId = roadId

  const _vertices = new Float32Array(vertices)
  const geo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  geo.setIndex(indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  const mat = customRoadSignMaterial.clone()
  const mesh = new Mesh(geo, mat)
  mesh.name = 'roadSign'
  mesh.objectId = objectId
  mesh.roadId = roadId
  mesh.renderOrder = RenderOrder.roadSign
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  group.add(mesh)

  group.position.set(position.x, position.y, position.z)
  if (projectNormal) {
    group.up.set(projectNormal.x, projectNormal.y, projectNormal.z)
  }
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    group.lookAt(_lookAtPoint)
  }

  parent.add(group)

  return {
    length,
    width,
    height: 0,
  }
}
